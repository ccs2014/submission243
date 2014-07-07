#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <math.h>
#include <search.h>
#include <libgen.h>
/*************************************/
#include "min_opgroups.c"
#include "constants.c"
#include "pump.c"
/*************************************/
#include "ud_hashtable.c"
/*************************************/
#include "LM_MemSafety.c"
/*************************************/

unsigned int Ncycles;

struct IRF {
	unsigned int tags[32];
};

struct FRF {
	unsigned int tags[32];
};

struct mem_segment
{
	unsigned int base;
	unsigned int length;
	unsigned int dealloced;
};

unsigned int pctag;
unsigned int pctag_bypassed;

struct IRF irf;		//tags on the Integer RF
struct FRF frf;		//tags on the Float RF
ud_hashtable *mem;	//tags on the memory

struct mem_segment *mem_segments;
unsigned int num_mem_segments;

unsigned int pc_malloc_start;
unsigned int pc_malloc_end;
unsigned int pc_free_start;
unsigned int skip_alloc=2;
unsigned int num_allocs=0;


unsigned int strToLInt(char* in)
{
	int len = strlen(in);
	int i;
	unsigned int hex = 1;
	unsigned int val = 0;

	i = 0;

	while(i<len)
	{
		if(in[len-1-i]>96)
		{
			val += ((in[len-1-i]-87))*hex;
		}
		else
		{
			val += ((in[len-1-i]-48))*hex;
		}
		hex *= 16;
		i++;
	}

	return val;
}

unsigned int mvec_out(unsigned int *m, FILE *f)
{
	fprintf(f, "%d %d %d %d %d %d\n", m[M_PC], m[M_CI], m[M_OP], m[M_OP1], m[M_OP2], m[M_MR]);

	return 0;
}


//initialize the Registers and memory with initial tags
void init_tags()
{
	int i;

	for (i=0; i<32; i++)
	{
		irf.tags[i] = BOTTOM;
		frf.tags[i] = BOTTOM;
	}

	mem = ht_create(1<<20);
	pctag = BOTTOM;


	mem_segments = (struct mem_segment *) malloc(1024 * sizeof(struct mem_segment));
	num_mem_segments = 0;
}

unsigned int create_new_segment(unsigned int alloc_size, unsigned int alloc_ptr)
{
	mem_segments[num_mem_segments].base = alloc_ptr;
	mem_segments[num_mem_segments].length = alloc_size;
	mem_segments[num_mem_segments].dealloced = 0;

	num_mem_segments++;

	if(num_mem_segments%1024 == 0)
	{
		mem_segments = (struct mem_segment *) realloc(mem_segments, (num_mem_segments+1024) * sizeof(struct mem_segment));
	}

	return 0;
}

unsigned int free_mem_segment(unsigned int *ptr)
{
	int i;
	int freed = 0;

	for(i=num_mem_segments-1; i>=0; i--)
	{
		if((*ptr >= mem_segments[i].base) && (*ptr <= mem_segments[i].base+mem_segments[i].length))
		{
			if(mem_segments[i].dealloced == 0)
			{
				//found the mem segment to be freed...
				*ptr = mem_segments[i].base;
				freed = mem_segments[i].length;
				mem_segments[i].dealloced = 1;
			}
		}
	}

	return freed;
}


unsigned int digested_rule(unsigned int* m, gzFile f)
{
	int i;
	uint32_t digest = 0;
	uint32_t key;
	for(i=0;i<6;i++)
	{
		key = m[i];
		digest += crc32_hash(key);
	}

	gzprintf(f, "%08x\n", digest);

	return 0;
}


//main simulator for the PUMP analysis
void simulate(  gzFile* inputFile
		, gzFile* allocFile 
		, FILE *result_file
		, gzFile dfile
		, FILE *mvec_file
		, FILE *l2misses_file
		, unsigned int size_l1
		, unsigned int size_l2
		)
{ 
	int cycle = 0;

//	int key_size = 
//	int val_size = 

	//initialize the pump structures...
	init_pumps(
			  size_l1
			, size_l2
	);
	
	char instring[1024];
	char* ptr;
	char* sstr;

	char astring[1024];
	char* asstr;

	unsigned int pc, opgroup;
	unsigned int if1 = 0, if2 = 0, if3 = 0;
	unsigned int r1, r2, r3;
	unsigned int op1, op2, op3, mem_addr;
	
	unsigned int alloc_ptr, alloc_size, alloc_tag=0;
	unsigned int dealloc_ptr, dealloc_size;

	unsigned int m[6];
	unsigned int r[2];

	struct PUMP_Output pump_out;


	unsigned int memcare, care, rescare;
	unsigned int tmpval;


	unsigned int inc = 1e3;
	unsigned int checkpoint = inc;

	//Processing trace...
  	while ((ptr=gzgets(*inputFile, instring, 1000))!= NULL) {
		//printf("%d - %s", cycle, instring);

		cycle++;

		instring[strlen(instring)] = '\0';	
		sstr = strtok(instring, " \n");

		pc = strToLInt(sstr);

		sstr = strtok(NULL, " ");

		if(opcodeToInt(sstr, &opgroup, &care, &rescare) == 0)
		{
			printf("%s\n", sstr);
			exit(0);
		}

		if((strcmp(sstr, "call_pal") != 0) && (strcmp(sstr,"wh64") != 0))
		{
			sstr = strtok(NULL, " ");
			if(*sstr != '-') {
				r1 = atoi(sstr+1);
				if(*sstr == 'r')
					if1 = 0;
				else
					if1 = 1;
			}
			else {
				r1 = 0;
				if1 = 0;
			}

			sstr = strtok(NULL, " ");
			if(*sstr != '-') {
				r2 = atoi(sstr+1);
				if(*sstr == 'r')
					if2 = 0;
				else
					if2 = 1;
			}
			else {
				r2 = 0;
				if2 = 0;
			}

			sstr = strtok(NULL, " ");
			if(*sstr != '-') {
				r3 = atoi(sstr+1);
				if(*sstr == 'r')
					if3 = 0;
				else
					if3 = 1;
			}
			else {
				r3 = 0;
				if3 = 0;
			}

			if((opgroup==arith2s1d) && (*sstr == '-'))
				opgroup = arith1s1d;


			op1=r1;
			op2=r2;
			op3=r3;

			int t;

			if((opgroup==arith2s1d) || (opgroup==arith1s1d) || (opgroup==move))
			{
				op3 = r1;
				op1 = r2;
				op2 = r3;

				t = if3;
				if3 = if1;
				if1 = if2;
				if2 = t;
			}
			else if(opgroup==other)
			{
				op1 = r2;

				if1 = if2;
			}
			else if((opgroup==bload) || (opgroup==wload))
			{
				op3 = r1;
				op1 = r2;

				if3 = if1;
				if1 = if2;
			}
			else if((opgroup==bstore) || (opgroup==wstore))
			{ 
				op1 = r1;
				op2 = r2;
			}
			else if((opgroup==ret) || (opgroup==icall) || (opgroup==dcall) || (opgroup==ijump))
			{
				op1 = r2;
				if1 = if2;
			}

			sstr = strtok(NULL, " ");
			if(*sstr == 'm')
			{
				sstr++;
				sstr[strlen(sstr)-1] = '\0';
				mem_addr = strToLInt(sstr);
			}
			else 
				mem_addr = 0;

			if(pc == pc_malloc_start)
			{
				//new allocation
				if(skip_alloc==0)
				{
					gzgets(*allocFile, astring, 128);
					//printf("%s", astring);
					astring[strlen(astring)-1] = '\0';
					asstr = strtok(astring, " ");
					asstr++;
					alloc_size = (unsigned int) strToLInt(asstr);
					gzgets(*allocFile, astring, 128);
					astring[strlen(astring)-1] = '\0';
					asstr = strtok(astring, " ");
					asstr++;
					alloc_ptr = (unsigned int) strToLInt(asstr);
					alloc_tag = create_new_tag_for_allocation();//create_new_segment(alloc_size, alloc_ptr);
					create_new_segment(alloc_size, alloc_ptr);
					//printf("alloc_size: %d\n", alloc_size);

					num_allocs++;
					while (alloc_size>0)
					{
						ht_insert(mem, alloc_ptr, alloc_tag);
						alloc_ptr+=4;
						alloc_size--;
					}
				}
				else
				{
					gzgets(*allocFile, astring, 128);
					gzgets(*allocFile, astring, 128);

					skip_alloc--;
				}	
			}
			else if(pc == pc_malloc_end)
			{
				if(num_allocs >= 1)
				{
					irf.tags[0] = alloc_tag;
					//printf("alloc_tag: %d (%d)\n", alloc_tag, irf.tags[0]);
				}
			} 
			else if(pc == pc_free_start)
			{
				//new deallocation
				gzgets(*allocFile, astring, 1000);
				astring[strlen(astring)-1] = '\0';
				asstr = strtok(astring, " ");
				asstr++;
				dealloc_ptr = (unsigned int) strToLInt(asstr);
				dealloc_size = free_mem_segment(&dealloc_ptr);

				if(dealloc_size == 0)
				{
//					printf("%x double free...exiting()\n", dealloc_ptr);
//					exit(0);
				}
				while(dealloc_size > 0)
				{
					ht_insert(mem, dealloc_ptr, BOTTOM);
					dealloc_ptr+=4;
					dealloc_size--;
				}
			}


			m[M_PC] = BOTTOM;//pctag;

//			if (ht_search(mem, pc, &tmpval) == -1)
//			{
//				//ht_insert(mem, pc, INSN_TAG);
//				//tmpval = INSN_TAG;
//				printf("%08x not found\n", pc);
//				exit(0);
//			}
			m[M_CI] = BOTTOM;//tmpval;

			m[M_OP] = opgroup;	

			if(if1 == 0)
				if(op1<32)
					m[M_OP1] = irf.tags[op1];
				else 
					m[M_OP1] = 0;
			else
				if(op1<32)
					m[M_OP1] = frf.tags[op1];
				else
					m[M_OP1] = 0;

			if(if2 == 0)
				if(op2 < 32)
					m[M_OP2] = irf.tags[op2];
				else
					m[M_OP2] = 0;
			else
				if(op2<32)
					m[M_OP2] = frf.tags[op2];
				else
					m[M_OP2] = 0;

			memcare = care & 0x1;
			if(memcare == 1)
			{
				if (ht_search(mem, mem_addr, &tmpval) == -1)
				{
					ht_insert(mem, mem_addr, BOTTOM);
					tmpval = BOTTOM;
				}
				m[M_MR] = tmpval;
			}
			else {
				m[M_MR] = BOTTOM;
			}


			unsigned int pump_input[6] = {m[M_PC], m[M_CI], m[M_OP], m[M_OP1], m[M_OP2], m[M_MR]};	//these are long tags
			//unsigned int i;
			mask_dc(pump_input, care);
			//printf("\t"); 
			//for(i=0;i<6;i++)
			//{
			//	printf("%d ", pump_input[i]);
			//}
			//printf("\n");

			digested_rule(pump_input, dfile);

			pump_out = pump_lookup(pump_input, care);	//pump_out is in long tags

			if(pump_out.found == 1)
			{
				if((rescare&1) == 1)
				{
					ht_insert(mem, mem_addr, pump_out.r[R_RES]);

				}
				if(((rescare&2)>>1) == 1)
				{
					if(if3 == 0)
					{
						irf.tags[op3] = pump_out.r[R_RES];
					}
					else
					{
						frf.tags[op3] = pump_out.r[R_RES];
					}
				}
				if(((rescare&4)>>2) == 1)
				{
					pctag = pump_out.r[R_PC];
				}
			}
			else {
				fprintf(l2misses_file, "%d\n", cycle);

				mvec_out(m, mvec_file);


				lm_memsafety(	  m
						, r
						);

				unsigned int pump_m[6] = {m[M_PC], m[M_CI], m[M_OP], m[M_OP1], m[M_OP2], m[M_MR]};
				unsigned int pump_r[2] = {r[R_PC], r[R_RES]};

				install_new_mr(pump_m, pump_r, care);

				//re-execute the instruction, this time with a hit!
				if((rescare&1) == 1)
				{
					ht_insert(mem, mem_addr, r[R_RES]);
				}
				if(((rescare&2)>>1) == 1)
				{
					if(if3 == 0)
						irf.tags[op3] = r[R_RES];
					else
					{
						frf.tags[op3] = r[R_RES];
					}
				}
				if(((rescare&4)>>2) == 1)
				{
					pctag = r[R_PC];
				}
			}
		}
		if(cycle==checkpoint)
		{
			fprintf(result_file, "%d %d %d %d %d\n", cycle, get_l1_misses(), get_l1_hits(), 
						    		        get_l2_misses(), get_l2_hits());		
			checkpoint += inc;	//+inc
		}
		if(cycle==Ncycles)
			break;
  	}

	//compute the DRAM footprint
//	int dram_tags[8] = {0,0,0,0,0,0,0,0};
//	i=0;
//	while (i < ht->depth)
//	{
//	}
}

int main(int argc, char *argv[]) 
{
	gzFile inputFile;
	gzFile allocFile;


	FILE* fp;
	char in[10];
	char fname[128];

	sprintf(fname, "%s/%s/malloc.start", argv[1], argv[2]);
	fp = fopen(fname, "r"); 
	fgets(in, 10, fp);
	pc_malloc_start = strToLInt(in);
	fclose(fp);

	sprintf(fname, "%s/%s/malloc.end", argv[1], argv[2]);
	fp = fopen(fname, "r"); 
	fgets(in, 10, fp);
	pc_malloc_end = strToLInt(in);
	fclose(fp);         

	sprintf(fname, "%s/%s/free.start", argv[1], argv[2]);
	fp = fopen(fname, "r"); 
	fgets(in, 10, fp);
	pc_free_start = strToLInt(in);
	fclose(fp);         

	// count the number of opcodes in the dictionary
	init_dict_size();
  
	sprintf(fname, "%s/%s/m5out/trace.trc.gz", argv[1], argv[2]);
	inputFile = gzopen(fname, "r");
    	assert(inputFile != NULL);

	sprintf(fname, "%s/%s/alloc.out.gz", argv[1], argv[2]);
	allocFile = gzopen(fname, "r");
    	assert(allocFile != NULL);


	Ncycles = atoi(argv[3]);
	unsigned int ncolors = atoi(argv[4]);
	unsigned int size_l1 = atoi(argv[5]);
	unsigned int size_l2 = atoi(argv[6]);

	//initialize the shadow space for tags...
	init_tags();
	initialize_LM_MemSafety(ncolors);


	char result_filename[128];
	FILE *result_file;
	sprintf(result_filename, "./results/%s.%d.%d.out", argv[2], size_l1, size_l2);
	result_file = fopen(result_filename, "w");

	char tag_filename[128];
	FILE *tag_file;
	sprintf(tag_filename, "./tags/%s.%d.%d.out", argv[2], size_l1, size_l2);

	tag_file = fopen(tag_filename, "w");

	char digested_rule_filename[128];
	gzFile digested_rule_file;
	sprintf(digested_rule_filename, "./rules/%s.%d.%d.rules.gz", argv[2], size_l1, size_l2);
	digested_rule_file = gzopen(digested_rule_filename, "w");


	char mvectors_filename[128];
	FILE* mvectors_file;
	sprintf(mvectors_filename, "./m-vectors/%s.%d.%d.mvec", argv[2], size_l1, size_l2);
	mvectors_file = fopen(mvectors_filename, "w");

	char l2misses_filename[128];
	FILE* l2misses_file;
	sprintf(l2misses_filename, "./l2misses/%s.%d.%d.mvec", argv[2], size_l1, size_l2);
	l2misses_file = fopen(l2misses_filename, "w");


	//initial type tags, memsafety tags, cfi tags, taint tags, composite_tags
	fprintf(tag_file, "1 %d\n", get_num_memsafety_tags());

	simulate(	  &inputFile
			, &allocFile
			, result_file
			, digested_rule_file
			, mvectors_file
			, l2misses_file
			, size_l1
			, size_l2
			);

	//type tags, memsafety tags, cfi tags, taint tags, composite_tags
	fprintf(tag_file, "1 %d\n", get_num_memsafety_tags());
	fclose(tag_file);

	free_pumps();
	ht_destroy(mem);
	gzclose(inputFile);
	gzclose(allocFile);
	fclose(result_file);
	gzclose(digested_rule_file);
	fclose(mvectors_file);
	fclose(l2misses_file);

	return 0;
}

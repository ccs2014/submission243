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
#include "LMTypes.h"
#include "LM_CCFI.c"
/*************************************/

unsigned int Ncycles;

struct IRF {
	unsigned int tags[32];
};

struct FRF {
	unsigned int tags[32];
};

struct CallInstr {
	unsigned int call_pc;
	unsigned int call_target;
};

struct ReturnInstr {
	unsigned int return_pc;
	unsigned int return_target;
};


unsigned int pctag;
unsigned int pctag_bypassed;

struct IRF irf;		//tags on the Integer RF
struct FRF frf;		//tags on the Float RF
ud_hashtable *mem;	//tags on the memory

unsigned int pc_malloc_start;
unsigned int pc_malloc_end;
unsigned int pc_free_start;
unsigned int skip_alloc=2;
unsigned int num_allocs=0;

struct reg{
	int is_ptr;
	int is_loaded;
	int load_addr;
	int is_alive;
};

struct reg regs[32];

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



int extract_cfg(gzFile* inputFile, gzFile* callFile, 
		FILE *cfg_file, 
		struct CallInstr *call_instrs, gzFile* retFile, struct ReturnInstr *return_instrs)
{
	int i;

	char instring[1024];
	char* ptr;
	char* sstr;

	char rstring[1024];
	char* rstr;

	unsigned int pc;
	unsigned int call_target;
	unsigned int return_target;

	unsigned int cfi_tag=1;
	unsigned int tmpval;

	unsigned int found, opgroup, care, rescare;
	int num_call_instrs = 0;
	int num_return_instrs = 0;
	int cycle = 0;

	//poduces the set of unique (return-instr-pc -> return-target-pc) pairs
	while ((ptr=gzgets(*inputFile, instring, 1000))!= NULL) {
		cycle++;

		instring[strlen(instring)] = '\0';	
		sstr = strtok(instring, " \n");

		pc = strToLInt(sstr);

		sstr = strtok(NULL, " ");

		if((strcmp(sstr, "call_pal") != 0) && (strcmp(sstr,"wh64") != 0))
		{
			opcodeToInt(sstr, &opgroup, &care, &rescare);

			if((strcmp(sstr, "jsr") == 0) || (strcmp(sstr, "bsr") == 0))
			{
				if(strcmp(sstr, "jsr") == 0)
				{
					gzgets(*callFile, rstring, 1000);
					rstring[strlen(rstring)] = '\0';
					rstr = strtok(rstring, " \n");
					rstr = strtok(NULL, " \n");

					call_target = strToLInt(rstr);

					found = 0;
					for(i=0; i<num_call_instrs; i++)
					{
						if((call_instrs[i].call_pc == pc) && (call_instrs[i].call_target == call_target))
						{
							found = 1;
						}
					}

					if(found == 0)
					{
						call_instrs = (struct CallInstr *) realloc(call_instrs, (num_call_instrs+1) * sizeof(struct CallInstr));
						call_instrs[num_call_instrs].call_pc = pc;
						call_instrs[num_call_instrs].call_target = call_target;
						num_call_instrs++;

						if(ht_search(mem, pc, &tmpval) == -1)
						{
							ht_insert(mem, pc, cfi_tag);
							cfi_tag++;
						}

						if(ht_search(mem, call_target, &tmpval) == -1)
						{
							ht_insert(mem, call_target, cfi_tag);
							cfi_tag++;
						}
					}
				}
			}
			else if(strcmp(sstr, "jmp") == 0)
			{
				sstr = strtok(NULL, " ");
				sstr = strtok(NULL, " ");
				sstr = strtok(NULL, " ");
				sstr = strtok(NULL, " ");

				sstr++;
				sstr[strlen(sstr)-1] = '\0';
				call_target = strToLInt(sstr);

				found = 0;
				for(i=0; i<num_call_instrs; i++)
				{
					if((call_instrs[i].call_pc == pc) && (call_instrs[i].call_target == call_target))
					{
						found = 1;
					}
				}

				if(found == 0)
				{
					call_instrs = (struct CallInstr *) realloc(call_instrs, (num_call_instrs+1) * sizeof(struct CallInstr));
					call_instrs[num_call_instrs].call_pc = pc;
					call_instrs[num_call_instrs].call_target = call_target;
					num_call_instrs++;

					if(ht_search(mem, pc, &tmpval) == -1)
					{
						ht_insert(mem, pc, cfi_tag);
						cfi_tag++;
					}

					if(ht_search(mem, call_target, &tmpval) == -1)
					{
						ht_insert(mem, call_target, cfi_tag);
						cfi_tag++;
					}
				}
			}
			else if((strcmp(sstr, "ret") == 0))
			{
				gzgets(*retFile, rstring, 1000);
				rstring[strlen(rstring)] = '\0';
				rstr = strtok(rstring, " \n");

				rstr = strtok(NULL, " \n");
				return_target = strToLInt(rstr);

				found = 0;
				for(i=0; i<num_return_instrs; i++)
				{
					if((return_instrs[i].return_pc == pc) && (return_instrs[i].return_target == return_target))
					{
						found = 1;
					}
				}

				if(found == 0)
				{
					return_instrs = (struct ReturnInstr *) realloc(return_instrs, (num_return_instrs+1) * sizeof(struct ReturnInstr));
					return_instrs[num_return_instrs].return_pc = pc;
					return_instrs[num_return_instrs].return_target = return_target;
					num_return_instrs++;

					if(ht_search(mem, pc, &tmpval) == -1)
					{
						ht_insert(mem, pc, cfi_tag);
						cfi_tag++;
					}

					if(ht_search(mem, return_target, &tmpval) == -1)
					{
						ht_insert(mem, return_target, cfi_tag);
						cfi_tag++;
					}
				}
			}
		}
		if(cycle==Ncycles)
			break;
	}

	initialize_cfg(12);

	//print out the CFG, build the cfg for the miss handler
	unsigned int a, b;
	for(i=0; i<num_return_instrs; i++)
	{
		ht_search(mem, return_instrs[i].return_pc, &a);
		ht_search(mem, return_instrs[i].return_target, &b);

		fprintf(cfg_file, "%d %d\n", a, b);
		add_edge_to_cfg(a, b);
	}
	for(i=0; i<num_call_instrs; i++)
	{
		ht_search(mem, call_instrs[i].call_pc, &a);
		ht_search(mem, call_instrs[i].call_target, &b);

		fprintf(cfg_file, "%d %d\n", a, b);
		add_edge_to_cfg(a, b);
	}

	return cfi_tag;
}



//main simulator for the PUMP analysis
void simulate(  gzFile* inputFile
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


	unsigned int pc, opgroup;
	unsigned int if1 = 0, if2 = 0, if3 = 0;
	unsigned int r1, r2, r3;
	unsigned int op1, op2, op3, mem_addr;
	

	unsigned int m[6];
	unsigned int r[2];

	struct PUMP_Output pump_out;


	unsigned int memcare, care, rescare;
	unsigned int tmpval;


	unsigned int inc = 1e3;
	unsigned int checkpoint = inc;

	//Processing trace...
  	while ((ptr=gzgets(*inputFile, instring, 1000))!= NULL) {
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


			m[M_PC] = pctag;

			if (ht_search(mem, pc, &tmpval) == -1)
			{
				//ht_insert(mem, pc, INSN_TAG);
				//tmpval = INSN_TAG;
				//printf("%08x not found\n", pc);
				//exit(0);
				m[M_CI] = BOTTOM;
			}
			else
			{
				m[M_CI] = tmpval;
			}

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
			mask_dc(pump_input, care);

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
						irf.tags[op3] = pump_out.r[R_RES];

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


				lm_ccfi(	  m
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
}

int main(int argc, char *argv[]) 
{
	gzFile inputFile;
	gzFile callFile;
	gzFile retFile;


	// count the number of opcodes in the dictionary
	init_dict_size();

	char fname[128];

  
	sprintf(fname, "%s/%s/m5out/trace.trc.gz", argv[1], argv[2]);
	inputFile = gzopen(fname, "r");
    	assert(inputFile != NULL);

	sprintf(fname, "%s/%s/calls.out.gz", argv[1], argv[2]);
	callFile = gzopen(fname, "r");
    	assert(callFile != NULL);

	sprintf(fname, "%s/%s/ret.out.gz", argv[1], argv[2]);
	retFile = gzopen(fname, "r");
    	assert(retFile != NULL);

	Ncycles = atoi(argv[3]);
	unsigned int size_l1 = atoi(argv[4]);
	unsigned int size_l2 = atoi(argv[5]);


	//initialize the shadow space for tags...
	init_tags();

	//extract the cfg for the program
	struct ReturnInstr *return_instrs = NULL;
	struct CallInstr *call_instrs = NULL;
	
	char cfg_filename[128];
	FILE* cfg_file;
	sprintf(cfg_filename, "./cfg/%s.cfg", argv[2]);
	cfg_file = fopen(cfg_filename, "w");

	unsigned int num_cfi_tags = extract_cfg(&inputFile, &callFile, 
			cfg_file, 
			call_instrs, &retFile, return_instrs);
	free(call_instrs);
	free(return_instrs);
	gzclose(callFile);
	gzclose(retFile);
	fclose(cfg_file);

	gzrewind(inputFile);


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
	sprintf(l2misses_filename, "./l2misses/%s.%d.%d.at", argv[2], size_l1, size_l2);
	l2misses_file = fopen(l2misses_filename, "w");


	//initial type tags, memsafety tags, cfi tags, taint tags, composite_tags
	fprintf(tag_file, "%d\n", num_cfi_tags);


	simulate(	  &inputFile
			, result_file
			, digested_rule_file
			, mvectors_file
			, l2misses_file
			, size_l1
			, size_l2
			);

	//type tags, memsafety tags, cfi tags, taint tags, composite_tags
	fprintf(tag_file, "%d\n", num_cfi_tags);
	fclose(tag_file);


	destroy_cfg();
	free_pumps();
	ht_destroy(mem);
	gzclose(inputFile);
	fclose(result_file);
	gzclose(digested_rule_file);
	fclose(mvectors_file);
	fclose(l2misses_file);

	return 0;
}

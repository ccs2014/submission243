#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAXTAINTS 37	//for libtaint

#define TAINT_BOTTOM 0x00


struct Taint
{
	unsigned int taint;
	unsigned int tag_value;
	struct Taint *children[MAXTAINTS];
	unsigned int num_children;
};
struct Taint *root_taint;

struct Taint_Tag
{
	unsigned int tag_value;
	unsigned int length;
	unsigned int tagset[MAXTAINTS];
};
struct Taint_Tag *taint_tags;

unsigned int num_taint_tags;
//unsigned int tag_value;


void mergesort(unsigned int *arr1, unsigned int n1, unsigned int *arr2, unsigned int n2, unsigned int *tunion, unsigned int *lunion)
{
	unsigned int i, i1, i2, t1, t2;

	tunion[0] = 0;

	i  = 1;
	i1 = 1;
	i2 = 1;

	while ((i1 < n1) || (i2 < n2))
	{
		t1 = 1<<24;
		if (i1 < n1)
		{
			t1 = arr1[i1];
		}

		t2 = 1<<24;
		if (i2 < n2)
		{
			t2 = arr2[i2];
		}

		if(t1 < t2)
		{
			if(tunion[i-1] != t1)
			{
				tunion[i] = t1;
				i++;
			}
			i1++;
		}
		else if(t2 < t1)
		{
			if(tunion[i-1] != t2)
			{
				tunion[i] = t2;
				i++; 
			}
			i2++;
		}
		else
		{
			if(tunion[i-1] != t1)
			{
				tunion[i] = t1;
				i++; 
			}
			i1++; i2++;
		}
	}

	*lunion = i;
}


struct Taint* new_taint_tag(struct Taint *parent, unsigned int taint, unsigned int *tagset, unsigned int length)
{
	struct Taint *new_taint = malloc(sizeof(struct Taint));
	new_taint->taint = taint;
	new_taint->tag_value = num_taint_tags;
	//printf("\t taint: %d @ %08x\n", taint, new_taint);
	//new_taint->children = (struct Taint *) malloc((MAXTAINTS-taint) * sizeof(struct Taint *));
	new_taint->num_children = 0;

	parent->children[parent->num_children] = new_taint;
	parent->num_children++;


	if(num_taint_tags%1024 == 0)
	{
		taint_tags = (struct Taint_Tag *) realloc(taint_tags, (num_taint_tags+1024)* sizeof(struct Taint_Tag));
	}

	taint_tags[num_taint_tags].tag_value = num_taint_tags;
	taint_tags[num_taint_tags].length = length;

	unsigned int i;
	for(i=0; i<length; i++)
	{
		taint_tags[num_taint_tags].tagset[i] = tagset[i];
	}

	//printf("%d tags exist (%d)\n", num_taint_tags, 0);

	num_taint_tags++;

	return new_taint;
}

int initialize_taint_tags()
{
	root_taint = (struct Taint *) malloc(sizeof(struct Taint));
	root_taint->taint = TAINT_BOTTOM;
	root_taint->tag_value = TAINT_BOTTOM;
	//root_taint->children = (struct Taint *) malloc(MAXTAINTS*sizeof(struct Taint *));
	root_taint->num_children = 0;

	taint_tags =  (struct Taint_Tag *) malloc(1024*sizeof(struct Taint_Tag));
	taint_tags[0].tag_value = TAINT_BOTTOM;
	taint_tags[0].length = 1;
	//taint_tags[0].tagset = (unsigned int *) malloc(sizeof(unsigned int));
	taint_tags[0].tagset[0] = TAINT_BOTTOM;


	num_taint_tags = 1;

	return 0;
}

/*** code to initialize the initial taints on the memory ***/ 
int initialize_taints()
{
	unsigned int taint;
	unsigned int set[2] = {TAINT_BOTTOM, TAINT_BOTTOM};

	taint = 2;
	while(taint < MAXTAINTS)
	{
		set[1] = taint;
		new_taint_tag(root_taint, taint, set, 2);
		taint++;
	}	

	return 0;
}

int initialize_LM_TaintTracking()
{
	initialize_taint_tags();
	initialize_taints();

	return 0;
}

unsigned int match_taint_tag(unsigned int *tunion, unsigned int lunion)
{
	//does tunion exist?
	struct Taint *cur_taint;
	cur_taint = root_taint;
	unsigned int i=1;
	unsigned int j;

//	for (j=0;j<lunion;j++)
//	{
//		printf("\t%d ", tunion[j]);
//	}
//printf("\n");
	unsigned int found=0, taint;

	while (i < lunion)
	{
//		printf("matching tunion[%d] = %d\n", i, tunion[i]);
		taint = tunion[i];
		found = 0;
		for (j=0; j<cur_taint->num_children; j++)
		{
//			printf("\t against (%d/%d) %d @ %08x\n", j, cur_taint->num_children, (cur_taint->children[j])->taint, (cur_taint->children[j]));
			if ((cur_taint->children[j])->taint == taint)
			{
				cur_taint = cur_taint->children[j];
				found = 1;
				break;
			}
		}
		if(found == 0)
		{
			cur_taint = new_taint_tag(cur_taint, taint, tunion, i+1);
		}

		i++;
	}

	return cur_taint->tag_value;
}

unsigned int tag_union(unsigned int *tagset, unsigned int length)
{
	unsigned int i;
	unsigned int tunion[MAXTAINTS]; //maximum size of the union...
	unsigned int lunion = 0;

	mergesort(taint_tags[tagset[0]].tagset, taint_tags[tagset[0]].length,
		  taint_tags[tagset[1]].tagset, taint_tags[tagset[1]].length,
		  tunion, &lunion);

	i=2;
	while(i<length)
	{
		mergesort(tunion, lunion, taint_tags[tagset[i]].tagset, taint_tags[tagset[i]].length,
			  tunion, &lunion);

		i++;
	}

	return match_taint_tag(tunion, lunion);
}

unsigned int lm_taint (unsigned int *m, unsigned int *r)
{
	unsigned int input_tags[5];
	unsigned  int result_tag;

	r[R_PC] = TAINT_BOTTOM;

	if(m[M_OP] == move)
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];

		result_tag = tag_union(input_tags, 2);
		r[R_RES] = result_tag;
	}
	else if(m[M_OP] == arith2s1d)
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];
		input_tags[2] = m[M_OP2];

		result_tag = tag_union(input_tags, 3);
		r[R_RES] = result_tag;
	}
	else if(m[M_OP] == arith1s1d)
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];

		result_tag = tag_union(input_tags, 2);
		r[R_RES] = result_tag;
	}

	else if(m[M_OP] == arith1d)
	{
		result_tag = m[M_CI];
		r[R_RES] = result_tag;
	}
	else if((m[M_OP] == wload) || (m[M_OP] == bload))
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];
		input_tags[2] = m[M_MR];

		result_tag = tag_union(input_tags, 3);
		r[R_RES] = result_tag;
	}
	else if(m[M_OP] == wstore)
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];
		input_tags[2] = m[M_OP2];
		//input_tags[3] = m[M_MR];

		result_tag = tag_union(input_tags, 3);
		r[R_RES] = result_tag;
	}
	else if(m[M_OP] == bstore)
	{
		input_tags[0] = m[M_CI];
		input_tags[1] = m[M_OP1];
		input_tags[2] = m[M_OP2];
		input_tags[3] = m[M_MR];

		result_tag = tag_union(input_tags, 4);
		r[R_RES] = result_tag;
	}


	return 1;
}

unsigned int get_num_taint_tags()
{
	return num_taint_tags;
}

void destroy_taint_tags()
{
	free(taint_tags);
	free(root_taint->children);
	free(root_taint);
}


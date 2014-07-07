#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <stddef.h>
//#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <string.h>

#define MEMSAFETY_BOTTOM 0x00

/*** encoding for memory safety tags ***/
// 32-bit tag = <24b primary color>:<8b secondary color index>
// use the primary color to index in to the tag-space, then the secondary color for indexing in to the buckets per se
struct SecondaryColor
{
	unsigned int valid;
	unsigned int color;
	unsigned int tag_value;
};
struct PrimaryColor
{
	unsigned int tag_value;
	struct SecondaryColor children[256]; //for now assuming constant
	unsigned int num_children;
};

struct PrimaryColor *memsafety_tags;

unsigned int num_primary_colors;
unsigned int num_memsafety_tags;
unsigned int next_tag_value;

unsigned int ncolors;
unsigned int next_color;

//max number of colors
int set_ncolors(unsigned int n)
{
	ncolors = n;
	next_color = 0;

	return 0;
}

//initialize the tag space...
int initialize_LM_MemSafety(unsigned int ncolors)
{
	set_ncolors(ncolors);

	memsafety_tags = (struct PrimaryColor *) malloc(1024*sizeof(struct PrimaryColor));	//pre-allocate 1024 tags

	memsafety_tags[0].tag_value = MEMSAFETY_BOTTOM;
	//memsafety_tags[0].num_children = 0;

	unsigned int i;
	for (i=0;i<1024;i++)
	{
		memsafety_tags[i].num_children = 1;
	}

	num_memsafety_tags = 1;

	num_primary_colors = 1;
	next_tag_value = 1;

	return 0;
}

//check if the tag exists...
int match_tag(unsigned int *tagset)
{
	unsigned int i,j;
	unsigned int primary_color, secondary_color;

	//primary_color = tag >> 8; //shift out the secondary color bits
	//secondary_color = tag & 0xff; //mask out primary colors
	primary_color = tagset[0];
	secondary_color = tagset[1];

	struct PrimaryColor root_tag = memsafety_tags[primary_color];

	if(secondary_color == 0)
	{
		return root_tag.tag_value;
	}

	j = root_tag.num_children;

	//OPTIMIZE_NOTE: replace the following linear search with a binary search -- need to keep the list always sorted...
	for (i=1; i<j; i++)
	{
		if(root_tag.children[i].color == secondary_color)
		{
			return root_tag.children[i].tag_value;
		}
	}
	
	return -1;
}

unsigned int get_memsafety_tag(unsigned int *tagset)
{
	int tag_exists = -1;
	unsigned int primary_color, secondary_color;
	unsigned int tag_value;

	tag_exists = match_tag(tagset);

	if(tag_exists != -1)
	{
		return tag_exists;
	}

	primary_color = tagset[0];
	secondary_color = tagset[1];

	struct PrimaryColor root_tag = memsafety_tags[primary_color];

	if (root_tag.num_children == 256)
	{
		printf("out of secondary colors\n");
		exit(0);
	}

	tag_value = (primary_color<<8) + root_tag.num_children;

	root_tag.children[root_tag.num_children].valid = 1;
	root_tag.children[root_tag.num_children].color = secondary_color;
	root_tag.children[root_tag.num_children].tag_value = tag_value;
	root_tag.num_children++;

	num_memsafety_tags++;

	return tag_value;
}

unsigned int create_new_tag_for_allocation()
{
	//unsigned int set[2] = {1+next_color, MEMSAFETY_BOTTOM};
	unsigned int primary_color = 1+next_color;

	unsigned int tag_value = primary_color << 8;

	if(primary_color >= num_primary_colors)
	{
		if(num_primary_colors%1024 == 0)
		{
			memsafety_tags = (struct PrimaryColor *) realloc(memsafety_tags, (num_primary_colors+1024) * sizeof(struct PrimaryColor));
		}

		memsafety_tags[primary_color].tag_value = tag_value;
		memsafety_tags[primary_color].num_children = 1;

		num_primary_colors++;
		num_memsafety_tags++;

		next_color++;
		if(next_color == ncolors)
		{
			next_color = 0;
		}
	}

	return tag_value;
}

unsigned int lm_memsafety (unsigned int *m, unsigned int *r)
{
	r[R_PC] = MEMSAFETY_BOTTOM;
	r[R_RES] = MEMSAFETY_BOTTOM;

	unsigned int set[2] = {MEMSAFETY_BOTTOM, MEMSAFETY_BOTTOM};
	unsigned int tag;

	unsigned int allow = 0;

	unsigned int temp1, temp2;

	if(m[M_OP] == other)
	{
		return 1;
	}
	else if(m[M_OP] == arith2s1d)
	{
		//if((m[OP1] == MEMSAFETY_BOTTOM) && (m[OP2] == MEMSAFETY_BOTTOM))
		//{
		//	r[RES] = MEMSAFETY_BOTTOM;
		//}
		//else 
		if((m[M_OP1]==MEMSAFETY_BOTTOM))
		{
			set[0] = m[M_OP2] >> 8;
			r[R_RES] = get_memsafety_tag(set);
			allow = 1;
		}
		else if(m[M_OP2]==MEMSAFETY_BOTTOM)
		{
			set[0] = m[M_OP1] >> 8;
			r[R_RES] = get_memsafety_tag(set);
			allow = 1;
		}
		//else {
		//	r[RES] = MEMSAFETY_BOTTOM;
		//}
	}
	else if (m[M_OP] == arith1s1d)
	{
		r[R_RES] = m[M_OP1];
		allow = 1;
	}
	else if (m[M_OP] == arith1d)
	{
		r[R_RES] = MEMSAFETY_BOTTOM;
		allow = 1;
	}
	else if (m[M_OP] == wload)
	{
		if(m[M_MR] > MEMSAFETY_BOTTOM)
		{
			//printf("\twload: [%d][%d]->(%d)\n", m[M_OP1], m[M_OP2], m[M_MR]);

			temp1 = m[M_MR] >> 8;
			temp2 = m[M_MR] & 0xff;

			//printf("\twload: (%d, %d)\n", temp1, temp2);
			if(temp2 == 0)
			{
				r[R_RES] = MEMSAFETY_BOTTOM;
			}
			else
			{
				set[0] = memsafety_tags[temp1].children[temp2].color;
				r[R_RES] = get_memsafety_tag(set);

			}

			allow = 1;
		}
		//else
		//{
		//	r[RES] = MEMSAFETY_BOTTOM;
		//}
	}
	else if (m[M_OP] == bload)
	{
		if(m[M_MR] > MEMSAFETY_BOTTOM)
		{
			r[R_RES] = MEMSAFETY_BOTTOM;
			allow = 1;
		}
	}
	else if (m[M_OP] == wstore)
	{
		//printf("\twstore: (%d, %d)\n", m[M_OP1], m[M_OP2]);

		if(m[M_OP1] > MEMSAFETY_BOTTOM)
		{
			set[0] = m[M_OP2] >> 8;
			set[1] = m[M_OP1] >> 8;

			//printf("\twstore: (%d, %d)\n", set[0], set[1]);

			tag = get_memsafety_tag(set);
			r[R_RES] = tag;
			allow = 1;
		}
		//else
		//{
		//	r[R_RES] = m[M_OP2];
		//}
	}
	else if (m[M_OP] == bstore)
	{
		if(m[M_OP1] > MEMSAFETY_BOTTOM)
		{
			set[0] = m[M_OP2] >> 8;
			set[1] = MEMSAFETY_BOTTOM;

			tag = get_memsafety_tag(set);
			r[R_RES] = tag;
			allow = 1;
		}
		//else
		//{
		//	r[R_RES] = m[M_OP2];
		//}
	}
	else if (m[M_OP] == move)
	{
		r[R_RES] = m[M_OP1];
		allow = 1;
	}

	return allow;
}

unsigned int get_num_memsafety_tags()
{
	return num_memsafety_tags;
}

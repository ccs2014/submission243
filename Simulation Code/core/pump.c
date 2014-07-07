// PUMP Simulator
// Fully associative with LRU replacement
//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>


/*** PUMP Data Structures ***/
struct MR_Slot
{
	int valid;
	unsigned int key[6];	//original keys stored in the mr_table
	unsigned int val[2]; 	//original values stored in the mr_table
	unsigned int age;
	unsigned int hits;	//number of hits in this lap
};

struct PUMP_Output{
	int found;
	unsigned int r[2];
};

struct PUMP
{
	/************************************/
	int num_entries;
	struct MR_Slot* mr_slots;
	/*****************************/

	int hits;
	int misses;
};

struct PUMP l1;	//stores short tags
struct PUMP l2;	//stores long tags

#ifdef MH_CACHE
int *policy_id;
struct PUMP mh_cache;
#endif

#ifdef RESTAG_CACHE
struct CompositeTagEntry
{
	unsigned int components[4];
	unsigned int valid;
	unsigned int composite_tag_value;
	unsigned int age;
};

struct CompositeTagCache
{
	struct CompositeTagEntry *composite_tags;

	unsigned int num_entries;

	unsigned int hits;
	unsigned int misses;
};

struct CompositeTagCache composite_tag_cache;
#endif

static unsigned int instrs = 0;
unsigned int curr_key[6];

#ifdef SHORT_TAGS
struct L2S
{
	unsigned int valid;
	unsigned int long_tag;
	unsigned int has_short_tag;	//only for l2->l1 tags
};

struct L2S *long2short_l1;
unsigned int num_l1_tags;
unsigned int next_l1_tag = 0;
unsigned int l1_flushes;
unsigned int *l1_flush_instrs;

struct L2S *long2short_l2;
unsigned int num_l2_tags;
unsigned int next_l2_tag = 0;
unsigned int l2_flushes;
unsigned int *l2_flush_instrs;
#endif

int init_pumps(
		  int n_l1
		, int n_l2
	#ifdef SHORT_TAGS
		, int l1_tag_bits
		, int l2_tag_bits
	#endif
	#ifdef MH_CACHE
		, int n_mh_cache
	#endif
	#ifdef RESTAG_CACHE
		, int n_restag_cache
	#endif
		)
{
	int i, j;

	//initialize the unified l1 pump
	l1.mr_slots = (struct MR_Slot *) malloc(n_l1 * sizeof(struct MR_Slot));
	for(i=0; i<n_l1; i++)
	{
		l1.mr_slots[i].valid = 0;
		for (j=0; j<6; j++)
		{
			l1.mr_slots[i].key[j]   = 0;
		}
		for (j=0; j<2; j++)
		{
			l1.mr_slots[i].val[j]   = 0;
		}
		l1.mr_slots[i].hits = 0;
		l1.mr_slots[i].age = 0;
	}
	//set the dmhc config variables...
	l1.num_entries   = n_l1;
	l1.hits = 0;
	l1.misses = 0;

	
	//initialize the unified l2 pump
	l2.mr_slots = (struct MR_Slot *) malloc(n_l2 * sizeof(struct MR_Slot));
	for(i=0; i<n_l2; i++)
	{
		l2.mr_slots[i].valid = 0;
		for (j=0; j<6; j++)
		{
			l2.mr_slots[i].key[j]   = 0;
		}
		for (j=0; j<2; j++)
		{
			l2.mr_slots[i].val[j]   = 0;
		}
		l2.mr_slots[i].hits = 0;
		l2.mr_slots[i].age = 0;
	}
	//set the dmhc config variables...
	l2.num_entries   = n_l2;
	l2.hits = 0;
	l2.misses = 0;

#ifdef SHORT_TAGS
	num_l1_tags = 1 << l1_tag_bits;
	long2short_l1 = (struct L2S *) malloc(num_l1_tags * sizeof(struct L2S));
	long2short_l1[BOTTOM].valid = 1;
	long2short_l1[BOTTOM].long_tag = BOTTOM;
	for(i=1;i<num_l1_tags;i++)
	{
		long2short_l1[i].valid = 0;
	}
	next_l1_tag = 1;	
	l1_flushes = 0;
	l1_flush_instrs = (unsigned int *) malloc(128 * sizeof(unsigned int));

	num_l2_tags = 1 << l2_tag_bits;
	long2short_l2 = (struct L2S *) malloc(num_l2_tags * sizeof(struct L2S));
	long2short_l2[BOTTOM].valid = 1;
	long2short_l2[BOTTOM].long_tag = BOTTOM;
	long2short_l2[BOTTOM].has_short_tag = 1;
	for(i=1;i<num_l2_tags;i++)
	{
		long2short_l2[i].valid = 0;
		long2short_l2[i].has_short_tag = 0;
	}	
	next_l2_tag = 1;
	l2_flushes = 0;
	l2_flush_instrs = (unsigned int *) malloc(32 * sizeof(unsigned int));
#endif

#ifdef MH_CACHE
	//initialize the per policy cache for Miss Handler software...
	mh_cache.mr_slots = (struct MR_Slot *) malloc(n_mh_cache * sizeof(struct MR_Slot));
	policy_id = (int *) malloc(n_mh_cache * sizeof(int));
	for(i=0; i<n_mh_cache; i++)
	{
		mh_cache.mr_slots[i].valid = 0;
		for(j=0; j<6; j++)
		{
			mh_cache.mr_slots[i].key[j] = 0;
		}
		for(j=0; j<2; j++)
		{
			mh_cache.mr_slots[i].val[j] = 0;
		}
		mh_cache.mr_slots[i].hits = 0;
		mh_cache.mr_slots[i].age = 0;

		policy_id[i] = -1;
	}
	mh_cache.num_entries = n_mh_cache;
	mh_cache.hits = 0;
	mh_cache.misses = 0;
#endif

#ifdef RESTAG_CACHE
	printf("composite tag cache of size %d entries\n", n_restag_cache);
	composite_tag_cache.composite_tags = (struct CompositeTagEntry *) malloc(n_restag_cache * sizeof(struct CompositeTagEntry));

	for(i=0; i<n_restag_cache; i++)
	{
		composite_tag_cache.composite_tags[i].valid = 0;
	}

	composite_tag_cache.hits = 0;
	composite_tag_cache.misses = 0;
	composite_tag_cache.num_entries = n_restag_cache;
#endif

	instrs = 0;

	return 0;
}


void mask_dc(unsigned int *m, unsigned int care)
{
	if((care&1) == 0)
		m[5] = BOTTOM;
	if((care&2) == 0)
		m[4] = BOTTOM;
	if((care&4) == 0)
		m[3] = BOTTOM;
	if((care&8) == 0)
		m[2] = BOTTOM;
	if((care&16) == 0)
		m[1] = BOTTOM;
	if((care&32) == 0)
		m[0] = BOTTOM;
}


#ifdef SHORT_TAGS
void flush_l1_tags()
{
	//flush the L1 pumps...
	if((l1_flushes>0) && (l1_flushes%128 == 0))
	{
		l1_flush_instrs = (unsigned int *) realloc(l1_flush_instrs, (l1_flushes+128) * sizeof(unsigned int));
	}
	l1_flush_instrs[l1_flushes] = instrs; 
	l1_flushes++;
	
	unsigned int i;

	//flush L1 PUMP
	for(i=0;i<l1.num_entries;i++)
	{
		l1.mr_slots[i].valid = 0;
		l1.mr_slots[i].hits = 0;
	}

	long2short_l1[BOTTOM].valid = 1;
	long2short_l1[BOTTOM].long_tag = BOTTOM;
	
	next_l1_tag = 1;

	for(i=next_l1_tag;i<num_l1_tags;i++)
	{
		long2short_l1[i].valid = 0;
		long2short_l2[long2short_l1[i].long_tag].has_short_tag = 0;
		long2short_l1[i].long_tag = 0;
	}

}

unsigned int tag_l2_l1(unsigned int long_tag, unsigned int *flush)
{
	unsigned int short_tag = 0;
	int i;
	int found;

	found = -1;

	if(long_tag==BOTTOM)
	{
		short_tag = long_tag;
		found = 1;
	}

	if(long2short_l2[long_tag].has_short_tag == 0)
	{
		goto new_short_tag;
	}

	if(found == -1)
	{
		for(i=1;i<next_l1_tag;i++)
		{
			if(long2short_l1[i].valid == 1)
			{
				if(long2short_l1[i].long_tag == long_tag)
				{
					short_tag = i;
					found = 1;
				}
			}
			if (found == 1)
			{
				break;
			}
		}
	}

new_short_tag:
	if (found == -1)	//did not find a valid long2short, assign new
	{
		if(next_l1_tag != num_l1_tags)	//not out of short tags
		{
			long2short_l1[next_l1_tag].valid = 1;
			long2short_l1[next_l1_tag].long_tag = long_tag;
			long2short_l2[long_tag].has_short_tag = 1;
			short_tag = next_l1_tag;
			next_l1_tag++;
		}
		else	//out of short tags [:(]
		{
			*flush = 1;
			//flush the L1 pumps...
			flush_l1_tags();

			//update the short2long data structure...
			long2short_l1[1].valid = 1;
			long2short_l1[1].long_tag = long_tag;

			next_l1_tag = 2;
			short_tag = 1;
		}
	}

	//printf ("\t(l2) %d  -> (l1) %d\n", long_tag, short_tag);

	return short_tag;
}

unsigned int tag_long_l2(unsigned int long_tag, unsigned int *flush)
{
	unsigned int short_tag = 0;
	int i;
	int found;

	found = -1;

	if(long_tag==BOTTOM)
	{
		short_tag = long_tag;
		found = 1;
	}

	if(found == -1)
	{
		for(i=1;i<next_l2_tag;i++)
		{
			if(long2short_l2[i].valid == 1)
			{
				if(long2short_l2[i].long_tag == long_tag)
				{
					short_tag = i;
					found = 1;
				}
			}
			if (found == 1)
			{
				break;
			}
		}
	}

	if (found == -1)	//did not find a valid long2short, assign new
	{
		if(next_l2_tag != num_l2_tags)	//not out of short tags
		{
			long2short_l2[next_l2_tag].valid = 1;
			long2short_l2[next_l2_tag].long_tag = long_tag;
			long2short_l2[next_l2_tag].has_short_tag = 0;
			short_tag = next_l2_tag;
			next_l2_tag++;
		}
		else	//out of L2 short tags [:(]
		{
			*flush = 1;
			//flush the L2 pumps...
			if((l2_flushes>0) && (l2_flushes%32 == 0))
			{
				l2_flush_instrs = (unsigned int *) realloc(l2_flush_instrs, (l2_flushes+32) * sizeof(unsigned int));
			}
			l2_flush_instrs[l2_flushes] = instrs; 
			l2_flushes++;

			//flush L2 PUMP
			for(i=0;i<l2.num_entries;i++)
			{
				l2.mr_slots[i].valid = 0;
				l2.mr_slots[i].hits = 0;
			}

			//update the short2long data structure...
			long2short_l2[BOTTOM].valid = 1;
			long2short_l2[BOTTOM].long_tag = BOTTOM;
			long2short_l2[BOTTOM].has_short_tag = 1;
			long2short_l2[1].valid = 1;
			long2short_l2[1].long_tag = long_tag;
			long2short_l2[1].has_short_tag = 0;

			next_l2_tag = 2;
			short_tag = 1;

			for(i=next_l2_tag;i<num_l2_tags;i++)
			{
				long2short_l2[i].valid = 0;
				long2short_l2[i].long_tag = 0;
				long2short_l2[i].has_short_tag = 0;
			}

			flush_l1_tags();
		}
	}

	//printf ("\t(long) %d  -> (l2) %d\n", long_tag, short_tag);

	return short_tag;
}
#endif


struct PUMP_Output pump_search(struct PUMP *pump, unsigned int *key)
{
	int i, j;
	struct PUMP_Output pump_out;
	int found = 0;

	for (i=0; i<pump->num_entries; i++)
	{
		found = 0;
		if(pump->mr_slots[i].valid == 1)
		{
			found = 1;
			for(j=0; j<6; j++)
			{
				if (key[j] != pump->mr_slots[i].key[j])
				{
					found = 0;
					break;
				}
			}

		}
		if(found==1)
		{
			pump->mr_slots[i].hits++;
			break;
		}
	}

	if(found==1)
	{
		pump->mr_slots[i].age = instrs;
		pump_out.found = 1;
		for(j=0; j<2; j++)
			pump_out.r[j] = pump->mr_slots[i].val[j];
	}
	else
	{
		pump_out.found = 0;
	}

	return pump_out;
}

//using full-scale LRU strategy for the FA cache...
int find_evictee(struct PUMP *pump)
{
	int slot=-1;
	unsigned int age = 0x7fffffff;
	int i;

	for(i=0;i<pump->num_entries;i++)
	{
		if(pump->mr_slots[i].valid == 0)
		{
			age = 0;
			slot = i;
			break;
		}
		else
		{
			if(pump->mr_slots[i].age < age)
			{
				age = pump->mr_slots[i].age;
				slot = i;
			}
		}
	}

	return slot;
}


int pump_new_mr(struct PUMP *pump, unsigned int *m, unsigned int *r)
{
	//we come here only if it is a miss...
	int i;
	int mr_slot = find_evictee(pump);

	for(i=0; i<6; i++)
		pump->mr_slots[mr_slot].key[i] = m[i];
	for(i=0; i<2; i++)
		pump->mr_slots[mr_slot].val[i] = r[i];

	pump->mr_slots[mr_slot].hits = 0;
	pump->mr_slots[mr_slot].valid = 1;
	pump->mr_slots[mr_slot].age = instrs;

	return 0;
}


struct PUMP_Output pump_lookup(unsigned int *key, unsigned int care)
{
	int i;
	struct PUMP_Output l1_out;
	struct PUMP_Output l2_out;
	instrs++;

	l1_out.found = 0;
	for(i=0; i<2; i++)
	{
		l1_out.r[i] = BOTTOM;
	}

	unsigned int l1_key[6];
	unsigned int l2_key[6];

#ifdef SHORT_TAGS
	unsigned int flush;
#endif

	l1_key[2] = key[2];
	for(int j=0; j<6; j++)
	{
		if(j!=2)
		{
#ifdef SHORT_TAGS
			//convert these to short tags before sending them to the L1 PUMPs.
			l1_key[j] = tag_l2_l1(tag_long_l2((key[j]), &flush), &flush);
#else
			l1_key[j] = key[j];
#endif
		}
	}

	l1_out = pump_search(&l1, l1_key);

	if(l1_out.found==1)
	{
		l1.hits++;
	}
	else
	{
		l1.misses++;
	}

	if(l1_out.found == 1)
	{
#ifdef SHORT_TAGS
		l1_out.r[0] = long2short_l2[long2short_l1[l1_out.r[0]].long_tag].long_tag;
		l1_out.r[1] = long2short_l2[long2short_l1[l1_out.r[1]].long_tag].long_tag;
#endif
		return	l1_out;
	}
	else
	{
		//convert short tags to long tags first...
		l2_key[2] = l1_key[2];
		for(i=0; i<6; i++)
		{
			if(i!=2)
			{
#ifdef SHORT_TAGS
				l2_key[i] = long2short_l1[l1_key[i]].long_tag;
#else
				l2_key[i] = l1_key[i];
#endif
			}
		}

		l2_out = pump_search(&l2, l2_key);

		if(l2_out.found==1)
		{
			l2.hits++;

			//install the rule from l2 to l1
			struct PUMP_Output new_l1;
#ifdef SHORT_TAGS
			new_l1.r[0] = tag_l2_l1(l2_out.r[0], &flush);
			new_l1.r[1] = tag_l2_l1(l2_out.r[1], &flush);
#else
			new_l1.r[0] = l2_out.r[0];
			new_l1.r[1] = l2_out.r[1];
#endif
			pump_new_mr(&l1, l1_key, new_l1.r);

#ifdef SHORT_TAGS
			l2_out.r[0] = long2short_l2[l2_out.r[0]].long_tag;
			l2_out.r[1] = long2short_l2[l2_out.r[1]].long_tag;
#endif
			return l2_out;
		}
		else
		{
			//need to install the rule in l2, l1
			l2.misses++;
//#ifdef SHORT_TAGS
//			l2_out.r[0] = long2short_l2[l2_out.r[0]].long_tag;
//			l2_out.r[1] = long2short_l2[l2_out.r[1]].long_tag;
//#endif
			return l2_out;
		}
	}
	//return l1_out;
}

int install_new_mr(unsigned int *m, unsigned int *r, unsigned int care)
{
	int i;

	mask_dc(m, care);

	unsigned int l1_key[6];
	unsigned int l2_key[6];
	unsigned int l1_val[2];
	unsigned int l2_val[2];

	l2_key[2] = m[2];
	for(i=0;i<6;i++)
	{
		if(i!=2)
		{
			l2_key[i] = m[i];
		}
	}
	for(i=0;i<2;i++)
	{
		l2_val[i] = r[i];
	}

	//install new rule into l2 cache
	pump_new_mr(&l2, l2_key, l2_val);

	l1_key[2] = m[2];
	for(i=0;i<6;i++)
	{
		if(i!=2)
		{
			l1_key[i] = l2_key[i];
		}
	}
	for(i=0;i<2;i++)
	{
		l1_val[i] = l2_val[i];
	}

	//install new rule into l1 cache
	pump_new_mr(&l1, l1_key, l1_val);

	return 0;
}


int get_l1_misses()
{
	int x = l1.misses;
	l1.misses = 0;
	return x;
}
int get_l1_hits()
{
	int x = l1.hits;
	l1.hits = 0;
	return x;
}

int get_l2_misses()
{
	int x = l2.misses;
	l2.misses = 0;
	return x;
}
int get_l2_hits()
{
	int x = l2.hits;
	l2.hits = 0;
	return x;
}

int free_pumps()
{
	free(l1.mr_slots);
	free(l2.mr_slots);

	return 0;
}

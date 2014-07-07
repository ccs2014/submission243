#include "cfg_hashtable.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>


struct CFG * cfg_create(unsigned int depth)
{
	struct CFG *ht = (struct CFG *) calloc(1, sizeof(struct CFG));

	unsigned int i;
	ht->depth = depth;

	ht->buckets = (struct CFG_Bucket *) calloc(depth, sizeof(struct CFG_Bucket));
	//assert(ht->buckets != NULL);

	for(i=0; i<depth; i++)
	{
		ht->buckets[i].length = 0;
		ht->buckets[i].nodes  = (struct CFG_BucketNode *) malloc(128*sizeof(struct CFG_BucketNode));
	}
	
	return ht;
}

int cfg_search(struct CFG *ht, unsigned int src, unsigned int tgt)
{
	unsigned int i, j, hval;
	//int index;

	hval = (unsigned int) (crc32_hash((uint32_t) src) % ht->depth);
	//hval = src % ht->depth;


	//index = -1;

	for (i=0; i<ht->buckets[hval].length; i++)
	{
		if(src == ht->buckets[hval].nodes[i].source)
		{
			for(j=0; j<ht->buckets[hval].nodes[i].num_targets; j++)
			{
				if(ht->buckets[hval].nodes[i].targets[j] == tgt)
				{
					//index = j;
					return j;
				}
			}
		}
	}

	return -1;
}

unsigned int cfg_insert(struct CFG *ht, unsigned int src, unsigned int tgt)
{
	unsigned int i, j, hval;

	hval = (unsigned int) (crc32_hash((uint32_t) src) % ht->depth);
	//hval = src % ht->depth;


	//int index;

	unsigned int found = 0;

	//first check if the src exists in the table
	for (i=0; i<ht->buckets[hval].length; i++)
	{
		if(src == ht->buckets[hval].nodes[i].source)
		{
			for(j=0; j<ht->buckets[hval].nodes[i].num_targets; j++)
			{
				if(ht->buckets[hval].nodes[i].targets[j] == tgt)
				{
					//index = j;
					found = 1;
				}
			}
			if (found == 0)
			{
				ht->buckets[hval].nodes[i].targets = (unsigned int *) realloc(ht->buckets[hval].nodes[i].targets, (ht->buckets[hval].nodes[i].num_targets+1)*sizeof(unsigned int));
				ht->buckets[hval].nodes[i].targets[ht->buckets[hval].nodes[i].num_targets] = tgt;
				ht->buckets[hval].nodes[i].num_targets++;
				found = 1;
				break;
			}

		}
		if(found == 1)
			break;
	}

	if (found == 0)
	{
		if((ht->buckets[hval].length > 0) && (ht->buckets[hval].length%128==0))
		{
			ht->buckets[hval].nodes = (struct CFG_BucketNode *) realloc(ht->buckets[hval].nodes, (ht->buckets[hval].length+128)*sizeof(struct CFG_BucketNode));
		}
		ht->buckets[hval].nodes[ht->buckets[hval].length].source = src;
		ht->buckets[hval].nodes[ht->buckets[hval].length].targets = (unsigned int *) malloc(sizeof(unsigned int));
		ht->buckets[hval].nodes[ht->buckets[hval].length].targets[0] = tgt;
		ht->buckets[hval].nodes[ht->buckets[hval].length].num_targets = 1;
		ht->buckets[hval].length++;
	}

	return 0;
}

unsigned int cfg_destroy(struct CFG *ht)
{
	unsigned int i;
	//unsigned int max_length = 0;

	for (i=0; i<ht->depth; i++)
	{
		free(ht->buckets[i].nodes);
	}

	free(ht->buckets);
	free(ht);

	return 0;
}

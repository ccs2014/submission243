//direct mapped cache

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct FACache{
	unsigned int *valid;
	unsigned int *tag;
	//unsigned int *block;

	unsigned int next_slot;
	unsigned int depth;

	unsigned int hits;
	unsigned int misses;
};

struct FACache * create_facache (unsigned int depth)
{
	depth = depth/4;
	struct FACache *facache = (struct FACache *) calloc(1, sizeof(struct FACache));
	facache->valid = (unsigned int *) calloc(depth, sizeof(unsigned int));
	facache->tag   = (unsigned int *) calloc(depth, sizeof(unsigned int));

	facache->next_slot = 0;
	facache->depth     = depth;

	facache->hits = 0;
	facache->misses = 0;

	return facache;
}

unsigned int insert_entry (struct FACache *facache, unsigned int address)
{
	facache->tag[facache->next_slot]   = address>>2;
	facache->valid[facache->next_slot] = 1;

	facache->next_slot = (facache->next_slot + 1) % (facache->depth);

	return 0;
}


unsigned int lookup_entry (struct FACache *facache, unsigned int address)
{
	unsigned int valid;
	unsigned int addr = address >> 2;

	unsigned int i;
	unsigned int found = 0;

	for(i=0; i<facache->depth; i++)
	{
		if(facache->tag[i] == addr)
		{
			found = 1;
			valid = facache->valid[i];
			break;
		}
	}


	if((found == 1) && (valid == 1))
	{
		facache->hits ++;
		return 1;
	}
	else
	{
		facache->misses ++;
		return 0;
	}
}

unsigned int facache_hits(struct FACache *facache)
{
	return facache->hits;
}

unsigned int facache_misses(struct FACache *facache)
{
	return facache->misses;
}


unsigned int destroy_facache(struct FACache *facache)
{
	free(facache);

	return 0;
}

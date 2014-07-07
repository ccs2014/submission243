//direct mapped cache

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct DMCache{
	unsigned int *valid;
	unsigned int *tag;
	//unsigned int *block;

	unsigned int next_slot;
	unsigned int addr_mask;
	unsigned int index_bits;

	unsigned int hits;
	unsigned int misses;
};

struct DMCache * create_dmcache (unsigned int depth)
{
	struct DMCache *dmcache = (struct DMCache *) calloc(1, sizeof(struct DMCache));
	dmcache->valid = (unsigned int *) calloc(depth, sizeof(unsigned int));
	dmcache->tag   = (unsigned int *) calloc(depth, sizeof(unsigned int));

	dmcache->next_slot = 0;

	dmcache->addr_mask =  depth - 1;
	dmcache->index_bits = (unsigned int ) (log(depth)/log(2));

	dmcache->hits = 0;
	dmcache->misses = 0;

	return dmcache;
}

unsigned int insert_entry (struct DMCache *dmcache, unsigned int tag)
{
	dmcache->tag[dmcache->next_slot]   = tag;
	dmcache->valid[dmcache->next_slot] = 1;

	dmcache->next_slot = (dmcache->next_slot + 1) % (dmcache->addr_mask + 1); 

	return 0;
}


unsigned int lookup_entry (struct DMCache *dmcache, unsigned int address)
{
	unsigned int tag, dmcache_index, valid, in_tag;
	unsigned int addr = address >> 2;

//	printf("address = %d, %d\n", address, (*dmcache).hits);

	dmcache_index = addr & dmcache->addr_mask;
//	printf("dcache_index = %d\n", dmcache_index);

	tag = dmcache->tag[dmcache_index];
//	printf("tag = %d\n", tag);

	valid = dmcache->valid[dmcache_index];
	
	in_tag = addr >> dmcache->index_bits;

	if((tag == in_tag) && (valid == 1))
	{
		dmcache->hits ++;
		return 1;
	}
	else
	{
		dmcache->misses ++;
		return 0;
	}
}

unsigned int destroy_dmcache(struct DMCache *dmcache)
{
	free(dmcache);

	return 0;
}

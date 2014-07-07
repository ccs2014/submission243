#include <inttypes.h>

struct CFG_BucketNode {
	unsigned int  source;
	unsigned int* targets;
	unsigned int  num_targets;
};

struct CFG_Bucket {
	unsigned int length;
	struct CFG_BucketNode *nodes;
};

struct CFG {
	unsigned int depth;
	struct CFG_Bucket *buckets;
};

typedef struct CFG cfg_hashtable;

struct CFG* cfg_create(unsigned int );
int cfg_search(struct CFG *, unsigned int , unsigned int );
unsigned int cfg_insert(struct CFG *, unsigned int , unsigned int );
unsigned int cfg_destroy(struct CFG *);

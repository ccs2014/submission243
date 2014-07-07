#include <inttypes.h>

struct BucketNode {
	unsigned int key;
	unsigned int value;
};

struct Bucket {
	unsigned int length;
	struct BucketNode *nodes;
};

struct HashTable {
	unsigned int depth;
	struct Bucket *buckets;
};

typedef struct HashTable ud_hashtable;

uint32_t crc32_hash(uint32_t );
struct HashTable* ht_create(unsigned int );
int ht_search(struct HashTable *, unsigned int , unsigned int *);
unsigned int ht_insert(struct HashTable *, unsigned int , unsigned int );
unsigned int ht_destroy(struct HashTable *);

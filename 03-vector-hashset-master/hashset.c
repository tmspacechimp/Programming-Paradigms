#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const int initialAllocation = 8;
void HashSetNew(hashset *h, int elem_Size, int num_Buckets,
				HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elem_Size > 0);
	assert(num_Buckets > 0);
	assert(hashfn != NULL);
	assert(comparefn != NULL);
	h->numBuckets = num_Buckets;
	h->elemSize = elem_Size;
	h->hashFn = hashfn;
	h->freeFn = freefn;
	h->cmpFn = comparefn;
	h->buckets = (vector *)malloc(sizeof(vector) * h->numBuckets);
	assert(h->buckets != NULL);
	for (int i = 0; i < h->numBuckets; i++)
	{
		VectorNew(&(h->buckets[i]), elem_Size, freefn, initialAllocation);
	}
}

void HashSetDispose(hashset *h)
{
	for (int i = 0; i < h->numBuckets; i++)
	{
		VectorDispose(&(h->buckets[i]));
	}
	free(h->buckets);
}

int HashSetCount(const hashset *h)
{
	int ans = 0;
	for (int i = 0; i < h->numBuckets; i++)
	{
		ans += VectorLength(&(h->buckets[i]));
	}
	return ans;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn!=NULL);
	for (int i = 0; i < h->numBuckets; i++)
		VectorMap(&(h->buckets[i]), mapfn, auxData);
	
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr!=NULL);
	int bucketIn = h->hashFn(elemAddr,h->numBuckets);
	assert(bucketIn>-1);
	assert(bucketIn<h->numBuckets);
	vector* bucket = &(h->buckets[bucketIn]);
	int pos = VectorSearch(bucket,elemAddr,h->cmpFn,0,false);
	if (pos>-1)
		VectorReplace(bucket,elemAddr,pos);
	else VectorAppend(bucket,elemAddr);

}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
	assert(elemAddr!=NULL);
	int bucketIn = h->hashFn(elemAddr,h->numBuckets);
	assert(bucketIn>-1);
	assert(bucketIn<h->numBuckets);
	vector* bucket = &(h->buckets[bucketIn]);
	int pos = VectorSearch(bucket,elemAddr,h->cmpFn,0,false);
	if (pos< 0) return NULL;
	return VectorNth(bucket,pos);
}

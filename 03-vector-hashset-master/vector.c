#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorExpand(vector *v)
{
    int newLen = v->allocLen *2;
    void *expandedVector = realloc(v->elems, newLen * v->elemSize);
    assert(expandedVector != NULL);
    v->elems = expandedVector;
    v->allocLen = newLen;
}

void VectorNew(vector *v, int elem_Size, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elem_Size > 0);
    assert(initialAllocation > -1);
    v->elemSize = elem_Size;
    if (initialAllocation == 0)
    {
        v->allocLen = 4;
        v->initAllocLen = 4;
    }
    else
    {
        v->allocLen = initialAllocation;
        v->initAllocLen = initialAllocation;
    }
    v->logLen = 0;
    v->vectorFreeFunction = freeFn;
    v->elems = malloc(elem_Size * v->allocLen);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    for (int i = 0; i < v->logLen; i++)
    {
        if (v->vectorFreeFunction != NULL)
            v->vectorFreeFunction((char *)v->elems + i * v->elemSize);
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{
    return v->logLen;
}

void *VectorNth(const vector *v, int position)
{
    assert(position > -1);
    assert(position < v->logLen);
    void *ans = (void *)((char *)v->elems + position * v->elemSize);
    return ans;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position > -1);
    assert(position < v->logLen);
    assert(elemAddr != NULL);
    void *dst = (void *)((char *)v->elems + position * v->elemSize);
    if (v->vectorFreeFunction != NULL)
        v->vectorFreeFunction(dst);
    memcpy(dst, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position > -1);
    assert(position <= v->logLen);
    assert(elemAddr != NULL);
    if (v->logLen == v->allocLen)
        VectorExpand(v);
    void *src = (void *)((char *)v->elems + position * v->elemSize);
    void *dst = (void *)((char *)src + v->elemSize);
    memmove(dst, src, (v->logLen - position) * v->elemSize);
    memcpy(src, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    assert(elemAddr != NULL);
    if (v->logLen == v->allocLen)
        VectorExpand(v);
    void *dst = (void *)((char *)v->elems + (v->logLen) * v->elemSize);
    memcpy(dst, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorDelete(vector *v, int position)
{
    assert(position > -1);
    assert(position < v->logLen);
    void *dst = (void *)((char *)v->elems + v->elemSize * position);
    if (v->vectorFreeFunction != NULL)
        v->vectorFreeFunction(dst);
    if (position != v->logLen - 1)
        memmove(dst, (void *)((char *)dst + v->elemSize), (v->logLen - 1 - position) * v->elemSize);
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    for (char *it = (char *)v->elems; it < (char *)v->elems + v->elemSize * v->logLen; it += v->elemSize)
    {
        mapFn(it, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert(startIndex > -1);
    if (!v->logLen == 0 && startIndex == 0)
        assert(startIndex < v->logLen);
    assert(searchFn != NULL);
    assert(key != NULL);
    if (!isSorted)
    {
        for (char *it = (char *)v->elems + startIndex * v->elemSize; it < (char *)v->elems + v->elemSize * v->logLen; it += v->elemSize)
        {
            if (searchFn(it, key) == 0)
                return (it - (char *)v->elems) / v->elemSize;
        }
        return kNotFound;
    }
    char *src = (char *)bsearch(key, v->elems, v->logLen, v->elemSize, searchFn);
    if (src == NULL)
        return kNotFound;
    return (src - (char *)v->elems) / v->elemSize;
}

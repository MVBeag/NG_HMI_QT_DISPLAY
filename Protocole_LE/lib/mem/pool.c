/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: pool.c
 * Date			: 14.12.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:
 * 
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lib/mem/pool.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
/* The header defining an allocated memory slot. A memory slot consists out of
 * one or more buckets, where a header is found at the beginning of the memory
 * slot. */
#define BUCKET_HEADER_SIZE		4

/* Header added to the very beginning of the allocated memory. */
struct bucket{
	uint8_t size;
	uint8_t refCnt;
	uint8_t idx;
	void *data;
};

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/* Since this is an object, it can't have local variables, otherwise it
 * wouldn't be possible to use it multiple times! */

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */
#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Initialize the pool object.
 *
 * Argument:	self	Pointer to pool object.
 * 				mem		Pointer to memory, which will be used for pool_alloc().
 * 				nBuckets  Number of buckets. Maximal 32.
 * 				bucketSizeShift  Bucket size in bytes as power of 2.
 * Return:		 0		success.
 * 				-1		self must not be a NULL pointer.
 * 				-2		mem must not be a NULL pointer.
 * 				-3		nBuckets must not exceed 32.
 */
int32_t pool_init(struct pool *self, void *mem,
					uint8_t nBuckets,
					uint8_t bucketSizeShift)
{
	if(self == NULL)
		return -1;
	if(mem == NULL)
		return -2;
	if(nBuckets > 32)
		return -3;
	self->mem = (uint8_t *) mem;
	self->nBuckets = nBuckets;
	self->bucketSizeShift = bucketSizeShift;
	self->activeBuckets = 0;
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Allocate memory.
 *
 * Argument:	self	Pointer to pool object.
 * 				size	Number of bytes to allocate.
 * Return:		Pointer to allocated memory.
 */
void *pool_alloc(struct pool *self, uint32_t size)
{
	int32_t i;
	uint32_t mask;
	uint32_t nBucketsNeeded;
	uint32_t bucketSize;
	struct bucket *bucket;
	void *ret = NULL;

	nBucketsNeeded = 1;
	bucketSize = 1 << self->bucketSizeShift;
	if(size > (bucketSize - BUCKET_HEADER_SIZE)){
		size -= (bucketSize - BUCKET_HEADER_SIZE);
		nBucketsNeeded += (size >> self->bucketSizeShift);
		if(size & (bucketSize-1))
			nBucketsNeeded++;
	}
	mask = (1 << nBucketsNeeded) - 1;
	for(i=0; i<(self->nBuckets - nBucketsNeeded); i++){
		if(!(self->activeBuckets & mask)){
			self->activeBuckets |= mask;
			bucket = (struct bucket *) (self->mem + i * bucketSize);
			bucket->size = nBucketsNeeded;
			bucket->idx = i;
			bucket->refCnt = 1;
			ret = &bucket->data;
			break;
		}
		mask <<= 1;
	}
	return ret;
}
/*---------------------------------------------------------------------------*/

/*
 * Release allocated memory.
 *
 * Argument:	self	Pointer to pool object.
 * 				mem		Pointer to allocated memory.
 */
void pool_free(struct pool *self, void *mem)
{
	uint32_t mask;
	uint8_t *d;
	struct bucket *bucket;

	if(mem != NULL){
		d = (uint8_t *) mem;
		bucket = (struct bucket *) (d - BUCKET_HEADER_SIZE);
		bucket->refCnt--;
		if(bucket->refCnt == 0){
			mask = ((1 << bucket->size) - 1) << bucket->idx;
			self->activeBuckets &= ~mask;
		}
	}
}
/*---------------------------------------------------------------------------*/

/*
 * Can be used to increase the internal reference counter.
 *
 * Argument:	mem		Pointer to allocated memory.
 */
void pool_get(void *mem)
{
	uint8_t *d;
	struct bucket *bucket;

	if(mem != NULL){
		d = (uint8_t *) mem;
		bucket = (struct bucket *) (d - BUCKET_HEADER_SIZE);
		bucket->refCnt++;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * Sets all buckets to empty state.
 *
 * Argument:	self	Pointer to pool object.
 */
void pool_clear(struct pool *self)
{
	self->activeBuckets = 0;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

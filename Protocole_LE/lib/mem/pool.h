/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: pool.h
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
 *****************************************************************************/

#ifndef SOURCE_LIB_MEM_POOL_H_
#define SOURCE_LIB_MEM_POOL_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/


/******************************************************************************
 * DEFINES
 *****************************************************************************/

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* The pool object.
 * Memory allocation for the mem pointer should be done like this:
 * 		uint8_t poolMemory[nBuckets * (1 << bucketSizeShift)];
 * Use pool_init() to initialize the fields of the pool object.
 */
struct pool{
	uint8_t *mem;
	uint32_t activeBuckets;  /* A bit field telling whether bucket is used. */
	uint8_t nBuckets;  /* Number of buckets. */
	uint8_t bucketSizeShift;  /* The bucket size in bytes as power of 2. */
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t pool_init(struct pool *, void *, uint8_t, uint8_t);
extern void *pool_alloc(struct pool *, uint32_t);
extern void pool_free(struct pool *, void *);
extern void pool_get(void *);
extern void pool_clear(struct pool *);


#endif /* SOURCE_LIB_MEM_POOL_H_ */

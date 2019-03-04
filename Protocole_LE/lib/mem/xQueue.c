/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: xQueue.c
 * Date			: 27.07.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO): Add a watermark to track usage? Uses quit some
 *                      memory since the current structure perfectly
 *                      matches 32-bit with (2 x 32-bit).
 *
 ******************************************************************************
 * Description:
 *  	The queue type works as a ring-buffer, where elements are added/pushed
 * 		to the head of the queue and returned/poped from the tail.
 * 		There are several versions of the queue, varying/optimized for a
 * 		certain memory size of the elements. The versions are found by the
 * 		file and function name prefix:
 * 		 - uc	for storing 8-bit unsigned character
 * 		 - us	for storing 16-bit unsigned short
 * 		 - ul	for storing 32-bit unsigned long
 * 		 - x	for storing any type, hence it is preferably used with struct
 * 		 		type. Note that this type has an additional field (bSize)
 * 		 		where the byte size of the elements is remembered.
 *
 * 	    There are two basic ways two implement whether a queue is full or not.
 * 	    1. Waste a slot in the buffer
 * 	    2. Use a flag to show that the buffer is full
 * 	    I decided to use the second approach. The flag is encoded in the
 * 	    head counter in the most significant bit. Encoding into the head
 * 	    counter has the advantage to save memory and further, it allows to
 * 	    very quickly check whether the queue is empty (head != tail). The
 * 	    disadvantage is that the head counter and consequently the tail
 * 	    counter and maxLen value must not be greater than half its possible
 * 	    value (since the first bit is reserved!). This is checked when
 * 	    calling xQueue_init().
 *
 * Example xQueue:
 * 		static x_t myBuf[10];
 * 		static volatile struct xQueue q = {.maxLen=10, .bSize=sizeOf(x_t)}
 *
 * 		void init(void){
 * 			q.buffer = (const void*) myBuf;
 * 		}
 *
 * 		void foo(x_t arg){
 * 			int32_t err = 0;
 * 			err = xQueue_push(&q, (const void*) &arg)
 * 		}
 * 
 *
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <string.h>

#include "lib/mem/xQueue.h"
#include "config/projConfig.h"

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/

/*
 * Initialize the queue.
 *
 * Argument:    c       pointer to xQueue object
 *              buf     pointer to buffer memory
 *              len     number of slots the buffer memory is reserved for
 *              slotSize  number of bytes for a single slot
 * Return:      err      0 success
 *                      -1 the number of slots must not be greater 0xff>>1
 *                         as the most significant bit is reserved.
 */
int32_t xQueue_init(struct xQueue *c, void *buf, uint8_t len, uint8_t slotSize)
{
    c->buffer = buf;
    c->maxLen = len;
    c->bSize = slotSize;
    c->head = 0;
    c->tail = 0;
    if(len < 0x80)
        return 0;
    else
        return -1;
}
/*---------------------------------------------------------------------------*/

/*
 * Add element to the head of the queue.
 * If the queue is full, the queue content remains unaltered.
 *
 * Argument:	c		pointer to xQueue object.
 * 				src 	pointer to data which will be copied into queue. Number
 * 						of bytes to copy is held in c.bSize
 * Return:		err		 0 success
 * 						-1 buffer full
 */
int32_t xQueue_push(struct xQueue *c, const void *src)
{
	void *dest;

    if(XQUEUE_FULL(c))
        return -1;
    INT_GLOB_MASK_SET;
    dest = (void *)(((uint32_t) c->buffer) + (c->head * c->bSize));
    memcpy(dest, src, c->bSize);
    c->head = (c->head + 1 >= c->maxLen) ? 0 : c->head + 1;
    if(c->head == c->tail)
        c->head |= 0x80;
    INT_GLOB_MASK_CLEAR;
    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Add element to the head of the queue uniquely.
 * The element is only added if not already in the queue. The number
 * of buffer entries is computed and it's then run over all these entries.
 * If any compares true, nothing is added to the queue and -2 is returned.
 *
 * Argument:	c		pointer to xQueue object.
 * 				src 	pointer to data which will be copied into queue. Number
 * 						of bytes to copy is held in c.bSize
 * Return:		err		 0 success
 * 						-1 buffer full
 * 						-2 not unique
 */
int32_t xQueue_push_unique(struct xQueue *c, const void *src)
{
	int32_t err;
	int32_t i, i_x;
	uint32_t flipOver;
	uint32_t nRiBufEntr;
	uint32_t bufAddr;

	if(XQUEUE_FULL(c))
        return -1;
	nRiBufEntr = (c->tail <= c->head) ?
            (c->head - c->tail) : (c->maxLen + c->head - c->tail);
	flipOver = c->maxLen - c->tail;
	for(i=0; i<nRiBufEntr; i++){
		i_x = (i >= flipOver) ? (i - c->maxLen) : i;
		bufAddr = ((uint32_t) c->buffer) + ((c->tail + i_x) * c->bSize);
		if(memcmp(src, (void*) bufAddr, c->bSize))
			return -2;
	}
	err = xQueue_push(c, src);
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Get and remove an element from the tail of the queue.
 * The element is copied to dest, make sure that at least c.bSize memory
 * is reserved. The slot is marked as free afterwards.
 *
 * Argument:	c		pointer to xQueue object.
 * 				dest	address where queue content is copied to.
 * Return:		err		 0 success
 * 						-1 queue empty
 */
int32_t xQueue_pop(struct xQueue *c, void *dest)
{
	int32_t err = 0;
	void *src;

    if(c->head != c->tail){
    	INT_GLOB_MASK_SET;
    	src = (void*)(((uint32_t) c->buffer) + (c->tail * c->bSize));
        memcpy(dest, src, c->bSize);
		c->tail = (c->tail+1 >= c->maxLen) ? 0 : c->tail + 1;
    	c->head &= 0x7f;
    	INT_GLOB_MASK_CLEAR;
    }else
    	err = -1;
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Get an element from the tail of the queue but leave it on the queue.
 * The element is not copied it is only a reference. The slot is still
 * marked as used.
 *
 * Argument:	c		pointer to xQueue object.
 * 				pData	address of the pointer which will be set to the
 * 						beginning of the element in the queue.
 * Return:		err		 0 success
 * 						-1 queue empty
 */
int32_t xQueue_get(struct xQueue *c, void **pData)
{
	int32_t err = 0;

    if(c->head != c->tail)
    	*pData = (void*)(((uint32_t) c->buffer) + (c->tail * c->bSize));
    else
    	err = -1;
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Remove an element from the tail of the queue.
 * This function is typically used together with xQueue_get.
 *
 * Argument:	c		pointer to xQueue object.
 * Return:		err		 0 success
 * 						-1 queue empty
 */
int32_t xQueue_consume(struct xQueue *c)
{
	int32_t err = 0;

    if(c->head != c->tail){
    	INT_GLOB_MASK_SET;
		c->tail = (c->tail+1 >= c->maxLen) ? 0 : c->tail + 1;
        c->head &= 0x7f;
    	INT_GLOB_MASK_CLEAR;
    }else{
    	err = -1;
    }
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Reset the queue to its original empty state.
 *
 * Argument:	c		pointer to xQueue object.
 */
void xQueue_reset(struct xQueue *c)
{
	c->head = 0;
	c->tail = 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Returns number of used slots in the queue.
 *
 * Argument:	c		pointer to xQueue object.
 * Return:		tmp		number of elements on the queue.
 */
uint32_t xQueue_load(struct xQueue *c)
{
	uint32_t tmp;

	if(XQUEUE_FULL(c))
	    return c->maxLen;
	tmp = (c->tail <= c->head) ?
			(c->head - c->tail) : (c->maxLen + c->head - c->tail);
	return tmp;
}
/*---------------------------------------------------------------------------*/

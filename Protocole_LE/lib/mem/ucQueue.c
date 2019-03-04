/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: ucQueue.c
 * Date			: 27.07.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
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
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#include "ucQueue.h"

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/

/*
 * Add element to the head of the queue.
 *
 * Return:	err		 0 success
 * 					-1 buffer full
 */
int32_t ucQueue_push(volatile struct ucQueue *c, const uint8_t data)
{
	int32_t err = 0;
	uint32_t next = 0;

	next = (c->head + 1 >= c->maxLen) ? 0 : c->head + 1;
    if (next != c->tail){
    	c->buffer[c->head] = data;
    	c->head = next;
    }else
    	err = -1;
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Only add to head of queue if element is not already in queue, hence unique.
 *
 * Return:	err		 0 success
 * 					-1 buffer full
 * 					-2 not unique
 */
int32_t ucQueue_push_unique(volatile struct ucQueue *c, const uint8_t data)
{
	int32_t err = 0;
	uint32_t i = 0;
	int32_t i_x = 0;
	uint32_t flipOver = 0;
	uint32_t nRiBufEntr = 0;

	/* calc the number of queue entries and where to flip the index over */
	nRiBufEntr = (c->tail <= c->head) ?
			(c->head - c->tail) : ((c->head - c->tail) + c->maxLen);
	flipOver = c->maxLen - c->tail;
	/* Run over all elements */
	for(i = 0; i < nRiBufEntr; i++){
		/* Prevent the index from over running the buffer */
		i_x = (i >= flipOver) ? (i - c->maxLen) : i;
		if(data == c->buffer[c->tail + i_x])
			return -2;
	}
	err = ucQueue_push(c, data);
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Get an element from the tail of the queue and remove it from the queue.
 *
 * Return:	err		 0 success
 * 					-1 queue empty
 */
int32_t ucQueue_pop(volatile struct ucQueue *c, uint8_t *pData)
{
	int32_t err = 0;

    if(c->head != c->tail){
    	*pData = c->buffer[ c->tail ];
		c->tail = (c->tail+1 >= c->maxLen) ? 0 : c->tail + 1;
    }else
    	err = -1;
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Reset the queue to its original empty state.
 */
void ucQueue_reset(volatile struct ucQueue *c)
{
	c->head = 0;
	c->tail = 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Returns number of messages in the queue.
 */
uint32_t ucQueue_messages_waiting(volatile struct ucQueue *c)
{
	uint32_t tmp;

	tmp = (c->tail <= c->head) ?
			(c->head - c->tail) : ((c->head - c->tail) + c->maxLen);
	return tmp;
}
/*---------------------------------------------------------------------------*/

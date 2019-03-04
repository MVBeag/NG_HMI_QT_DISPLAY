/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: xQueue.h
 * Date			: 27.07.2017
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

#ifndef SOURCE_LIB_MEM_XQUEUE_H_
#define SOURCE_LIB_MEM_XQUEUE_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * MACROS
 *****************************************************************************/
/*
 *
 */
#define XQUEUE_EMPTY(queue)		((queue)->head == (queue)->tail)
#define XQUEUE_FULL(queue)      ((queue)->head & 0x80)

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
struct xQueue{
    void * buffer;  /* pointer to the buffer */
    volatile uint8_t tail;  /* tail counter */
    volatile uint8_t head;  /* head counter */
    uint8_t maxLen;  /* maximal number of elements */
    uint8_t bSize;  /* byte size. Size of each element in bytes */
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t xQueue_init(struct xQueue *, void *, uint8_t, uint8_t);
extern int32_t xQueue_push(struct xQueue *, const void *);
extern int32_t xQueue_push_unique(struct xQueue *, const void *);
extern int32_t xQueue_pop(struct xQueue *, void *);
extern int32_t xQueue_get(struct xQueue *, void **);
extern int32_t xQueue_consume(struct xQueue *);
extern void xQueue_reset(struct xQueue *);
extern uint32_t xQueue_load(struct xQueue *);


#endif /* SOURCE_LIB_MEM_XQUEUE_H_ */

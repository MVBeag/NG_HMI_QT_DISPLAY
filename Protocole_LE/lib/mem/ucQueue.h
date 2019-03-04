/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: ucQueue.h
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

#ifndef SOURCE_LIB_MEM_UCQUEUE_H_
#define SOURCE_LIB_MEM_UCQUEUE_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
struct ucQueue{
    uint8_t * const buffer;
    uint8_t tail;
    uint8_t head;
    const uint8_t maxLen;
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t ucQueue_push(volatile struct ucQueue *, const uint8_t);
extern int32_t ucQueue_push_unique(volatile struct ucQueue *, const uint8_t);
extern int32_t ucQueue_pop(volatile struct ucQueue *, uint8_t *);
extern void ucQueue_reset(volatile struct ucQueue *);
extern uint32_t ucQueue_messages_waiting(volatile struct ucQueue *);

#endif /* SOURCE_LIB_MEM_UCQUEUE_H_ */

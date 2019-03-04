/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: usQueue.h
 * Date			: 27.09.2017
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

#ifndef SOURCE_LIB_MEM_USQUEUE_H_
#define SOURCE_LIB_MEM_USQUEUE_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
struct usQueue{
    uint16_t * const buffer;
    uint8_t tail;
    uint8_t head;
    const uint8_t maxLen;
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t usQueue_push(volatile struct usQueue *, const uint16_t);
extern int32_t usQueue_push_unique(volatile struct usQueue *, const uint16_t);
extern int32_t usQueue_pop(volatile struct usQueue *, uint16_t *);
extern void usQueue_reset(volatile struct usQueue *);
extern uint32_t usQueue_messages_waiting(volatile struct usQueue *);

#endif /* SOURCE_LIB_MEM_USQUEUE_H_ */

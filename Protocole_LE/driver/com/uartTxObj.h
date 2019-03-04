/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: uartTxObj.h
 * Date			: 11.08.2017
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

#ifndef UARTTXOBJ_H_
#define UARTTXOBJ_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/prot/protocol.h"
#include "lib/stm/aok.h"
#include "lib/stm/event.h"
#include "lib/mem/xQueue.h"
#include "lib/mem/ucBuffer.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* signals to object */
enum{
	TX_GO_SIG,  /* start transmitting */
	TX_STOP_SIG,  /* stops further transmission, remaining characters in FIFO
	 	 	 	 	 are still sent. */
	TX_SIG,  /* TX FIFO content transmitted */
};

/* signals from object */
enum{
	TX_DONE_SIG = TXOBJ_PUBLIC_SIG,
	TX_ERR_SIG,
};

/* TX_ERR_SIG event data */
enum{
	TX_ERR_BUFFEREMPTY,
	TX_ERR_UARTBUSY,
    TX_ERR_STOP,
};

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef void (*txCb_t)(void *, struct event *);

struct uartTxObj{
	struct aoStm super;
	struct{
		txCb_t func;  /* callback function */
		void *handle;  /* handle passed to the callback function */
	}cb;
	struct ucBuffer txBuf;  /* the receive buffer */
	uint32_t uartBase;  /* base address of the UART port */
	uint16_t crc;  /* storage for 16bit CRC */
	uint8_t flags;  /* internal flags */
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern uint32_t txObj_init_buffer(struct uartTxObj *, uint8_t *, uint16_t);
extern void txObj_set_buffered_unsent(struct uartTxObj *);

/* state machine */
extern int32_t tx_init(struct uartTxObj *, uint8_t, uint32_t,
						txCb_t, void *, uint32_t);


#endif /* UARTTXOBJ_H_ */

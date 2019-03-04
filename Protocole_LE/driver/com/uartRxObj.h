/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: uartRxObj.h
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

#ifndef UARTRXOBJ_H_
#define UARTRXOBJ_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/stm/aok.h"
#include "lib/stm/event.h"
#include "lib/mem/xQueue.h"
#include "lib/mem/ucBuffer.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* signals to object */
enum{
	RX_SIG,
	RX_ERROR_SIG,
	RX_TIMEOUT_SIG,
};

/* signals from object */
enum{
	RX_DONE_SIG = RXOBJ_PUBLIC_SIG,
	RX_ERR_SIG,
	RX_RECEIVING_SIG,
};

/* RX_ERR_SIG event data */
enum{
	RX_ERR_MSGTOOSHORT = 1,  /* RX timeout --> incomplete message */
	RX_ERR_MSGTOOLONG,  /* Message exceeds buffer size */
	RX_ERR_PREAMBLEMISMATCH,  /* Wrong preamble */
	RX_ERR_BUFFERUSED,  /* Buffer not cleared (len != 0) */
	RX_ERR_CRCMISMATCH,  /* Wrong CRC */
	RX_INTERR_FRAMING = 0x0010,
	RX_INTERR_PARITY = 0x0020,
	RX_INTERR_BREAK = 0x0040,
	RX_INTERR_OVERRUN = 0x0080,
};

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* Function prototype for the callback function */
typedef void (*rxCb_t)(void *, struct event *);

/* The UART RX object */
struct uartRxObj{
	struct aoStm super;
	struct{
		rxCb_t func;  /* callback function */
		void *handle;  /* handle passed to the callback function */
	}cb;
	struct ucBuffer rxBuf;
	uint32_t uartBase;  /* base address of the UART port */
	struct{
		uint16_t len;  /* data link layer field 'length' */
		uint16_t crc;  /* data link layer field 'CRC' */
	}dlink;
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
/**/
extern uint32_t rxObj_init_buffer(struct uartRxObj *, uint8_t *, uint16_t);

/* state machine */
extern int32_t rx_init(struct uartRxObj *, uint8_t, uint32_t,
						rxCb_t, void *, uint32_t);


#endif /* UARTRXOBJ_H_ */

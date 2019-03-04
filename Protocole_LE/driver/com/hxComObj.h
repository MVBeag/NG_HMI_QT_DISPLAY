/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: hxComObj.h
 * Date			: 06.09.2017
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

#ifndef SOURCE_USER_COM_HXCOMOBJ_H_
#define SOURCE_USER_COM_HXCOMOBJ_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "driver/com/uartRxObj.h"
#include "driver/com/uartTxObj.h"
#include "lib/stm/aok.h"
#include "lib/stm/event.h"
#include "lib/mem/xQueue.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/**/
#define HX_CONF_LB_EN           0x01  /* Tell if there is a loop back */
#define HX_CONF_TX_EN_NOT       0x02  /* Tell if TX enable is logic low */
#define HX_CONF_RX_EN_NOT       0x04  /* Tell if RX enable is logic low */

/* signals to object */
enum{
	HX_ON_SIG,  /* turn driver on */
	HX_OFF_SIG,  /* turn driver off */
	HX_GO_SIG, /* start the process of sending followed by receiving */
	HX_TIMEOUT_SIG,  /* internal timer signal */
	HX_TX_DELAY_SIG,  /* internal timer signal */
};

/* signals from object */
enum{
	HX_NO_RESPONSE_ERR_SIG = HXCOMOBJ_PUBLIC_SIG,  /* No response */
	HX_ERR_SIG,  /* Either a TX or an RX error occurred */
	HX_RX_SIG,  /* Valid message received (a wrapper for RX_DONE_SIG) */
	HX_INV_RX_SIG,  /* Received data while no data should be received */
};

/* HX_NO_RESPONSE_ERR_SIG event data */
enum{
	HX_NO_RESPONSE_ERR_RECEIVE,  /* no response by slave */
	HX_NO_RESPONSE_ERR_LOOPBACK,  /* no response by hardware */
};

/* HX_ERR_SIG event data. Note that in this special case, the event data
 * from TX_ERR and RX_ERR is preserved in the lower byte! */
#define HX_ERR_EVENT_DATA_M		0xff00
enum{
	HX_ERR_TXERR = 0x0000,  /* transmit error, added to TX_ERR event data */
	HX_ERR_RWTERR = 0x0100,  /* receive while transmit error, added to RX_ERR event data */
	HX_ERR_LBERR = 0x0200,  /* loop back error, added to RX_ERR event data */
	HX_ERR_RXERR = 0x0300,  /* receive error, added to RX_ERR event data */
};

/* HX_INV_RX_SIG event data. Note that RX_ERR event data is preserved in
 * the lower byte. */
enum{
    HX_INV_RX_WERR = 0x0000,  /* RX_ERR while in waiting state, added to RX_ERR event data */
    HX_INV_RX_WRX = 0x0100,  /* RX while in waiting state */
    HX_INV_RX_SERR = 0x0200,  /* RX_ERR while in send state, added to RX_ERR event data */
    HX_INV_RX_SRX = 0x0300,  /* RX while in send state */
};

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef void (*hxComCb_t)(void *, struct event *);

#define HX_STATE_NESTING			2

/* -- object of this file -- */
struct hxComObj{
	struct aoHsm super;
	struct uartRxObj rxObj;
	struct uartTxObj txObj;
	struct{
		hxComCb_t func;  /* callback function */
		void *handle;  /* handle passed to the callback function */
	}cb;
	struct{
	    uint16_t txDelay;  /* delay [us] before send to let hardware toggles settle */
	    uint16_t timeout;  /* timeout [ms] for not responding slave */
	}timing;
	struct{
	    uint32_t txEnBaseNPin;  /**/
        uint32_t rxEnBaseNPin;  /**/
	}hw;
	int16_t timerId;  /* remember the timer ID to start/stop the timer */
	uint8_t config;  /* configuration, see CONF_X defines above */
	uint8_t flags;  /* internally used flags, see FLAG_X defines in .c file */
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern void hxcom_set_callback(struct hxComObj *, hxComCb_t, void *);

/* state machine */
extern int32_t hxcom_init(struct hxComObj *, uint8_t, uint8_t,
                          uint32_t, uint32_t, uint32_t,
                          uint16_t, uint16_t,
                          hxComCb_t, void *,
                          uint8_t, uint8_t, uint8_t);

#endif /* SOURCE_USER_COM_HXCOMOBJ_H_ */

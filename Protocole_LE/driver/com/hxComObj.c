/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: hxComObj.c
 * Date			: 06.09.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description: This active object is a half dublex (HX) driver.
 * 				The receive and transmit functionality is called through
 * 				the uartRxObj and uartTxObj. Therefore, this driver is
 * 				specific to UART and also to the protocol described in
 * 				BronCom2017.xlsx.
 *
 * 				The half dublex driver is supposed to be used by a
 * 				communication master device. It can be turned on and off
 * 				through the HX_ON_SIG and HX_OFF_SIG. Once switched on, the
 * 				driver can be moved to 'send' state through the HX_GO_SIG,
 * 				where transmission is started. On a successful transmit, the
 * 				HX driver moves into 'receive' state, where a response by the
 * 				slave device is expected. As soon as the slave response is
 * 				received, the HX_RX_SIG is thrown and the state machine goes
 * 				into 'waiting' state until it's time to send the next message.
 * 				If loop back is enabled, it is expected that two messages are
 * 				received. The first one being the HW loop back is checked by
 * 				the RX driver and cleared before moving into receive state.
 * 				The second message is the slave response.
 * 				If no response is given by the slave (or loop back), a
 * 				NO_RESPONSE_ERROR_SIG is thrown at the respond timeout and
 * 				the state machine moves back to 'waiting'. Respond timeout
 * 				is given with hxcom_init(). Note that the RX buffer is set
 * 				to 8, hence you should add (8 * 10bits * 1/baudrate) to the
 * 				desired response timeout!
 *
 *				Errors and message reception are reported over a callback
 * 				function. The prototype of the callback function looks as
 * 				follows: int32_t (*cb)(void *, struct event *).
 * 				This prototype matches the ao_post and ao_dispatch function,
 * 				so one of these two is typically assigned as the callback
 * 				function. Callback function and the void * argument is
 * 				assigned with the objects initializing function (hxcom_init).
 * 				The outgoing events are (e.sig): HX_RX_SIG, HX_ERR_SIG and
 * 				HX_NO_RESPONSE_ERR_SIG. The event.data field holds more
 * 				accurate information about errors.
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "config/projConfig.h"
#include "user/debug/debugTask.h"
#include "lib/timer/timerDeamon.h"
#include "driver/com/hxComObj.h"
#include "driver/com/uartRxObj.h"
#include "driver/com/uartTxObj.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
/**/
#define FLAG_LB_RX_EARLY        0x01

/**/
#define TX_EN(self)     HWREG(self->hw.txEnBaseNPin + GPIO_O_DATA) = \
                            (self->config & HX_CONF_TX_EN_NOT) ? 0 : 0xff
#define TX_DIS(self)    HWREG(self->hw.txEnBaseNPin + GPIO_O_DATA) = \
                            (self->config & HX_CONF_TX_EN_NOT) ? 0xff : 0
#define RX_EN(self)     HWREG(self->hw.rxEnBaseNPin + GPIO_O_DATA) = \
                            (self->config & HX_CONF_RX_EN_NOT) ? 0 : 0xff
#define RX_DIS(self)    HWREG(self->hw.rxEnBaseNPin + GPIO_O_DATA) = \
                            (self->config & HX_CONF_RX_EN_NOT) ? 0xff : 0

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/* Since this is an object, it can't have local variables, otherwise it
 * wouldn't be possible to used it multiple times! */

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
/* -- state machine -- */
static void off(struct hxComObj *, struct event *);
static void on(struct hxComObj *, struct event *);
    static void waiting(struct hxComObj *, struct event *);
    static void send(struct hxComObj *, struct event *);
    static void receive_lb(struct hxComObj *, struct event *);
    static void receive(struct hxComObj *, struct event *);

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
 * Assign callback function and the handle given with the callback function.
 * Initial assignment is done with the hxcom_init() function. This function
 * is used in the seldom case where a re-assignment is necessary.
 *
 * Argument:	self		Reference to hxComObj.
 *  			cb			The callback function.
 * 				cbHandle	Handle passed with the callback function.
 */
void hxcom_set_callback(struct hxComObj *self, hxComCb_t cb, void *cbHandle)
{
	self->cb.func = cb;
	self->cb.handle = cbHandle;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * STATE MACHINE
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * The init function is typically called once before the main loop. It
 * can be crowded with initialization functions and must be used to
 * initialize and enter the first state.
 *
 * Argument:	self		Reference to hxComObj.
 * 				prio		Active object priority.
 * 				config      Configuration like loop back. See CONF_X
 * 				            defines in header file.
 * 				uartBase	The base memory address of the UART module.
 * 				txEnBaseNPin (GPIO_BASE | (GPIO_PIN<<2)) to switch
 * 				            transmitter on/off. Needed for RS485.
 * 				rxEnBaseNPin (GPIO_BASE | (GPIO_PIN<<2)) to switch
 *                          receiver on/off. Optional.
 * 				timeout		The time [ms] waited for an answer before a
 * 							HX_NO_RESPONSE_ERR is thrown. Note that RX buffer
 * 							is set to 8, hence you should add 8*10bits*1/baudrate
 * 							to the desired response timeout!
 * 				txDelay     Optional delay [us] before sending the message,
 * 				            typically used if HW needs to be switched on.
 * 				cb			The callback function.
 * 				cbHandle	Handle passed with the callback function.
 * 				eQueueLenHx Length of HX event queue.
 *              eQueueLenRx Length of RX event queue.
 *              eQueueLenTx Length of TX event queue.
 * Return:		 0			success
 * 				-1			event queue memory not assigned/allocated
 */
int32_t hxcom_init(struct hxComObj *self, uint8_t prio, uint8_t config,
                   uint32_t uartBase, uint32_t txEnBaseNPin, uint32_t rxEnBaseNPin,
                   uint16_t timeout, uint16_t txDelay,
                   hxComCb_t cb, void *cbHandle,
                   uint8_t eQueueLenHx, uint8_t eQueueLenRx, uint8_t eQueueLenTx)
{
	int32_t err;
	void *eQueueMem;
	void *stateMem;

	/* Top action */
	self->cb.func = cb;
	self->cb.handle = cbHandle;
	self->config = config;
	self->hw.txEnBaseNPin = txEnBaseNPin;
	self->hw.rxEnBaseNPin = rxEnBaseNPin;
	self->timing.timeout = timeout;
	self->timing.txDelay = txDelay;
	timerD_create_timer(1, 0, &self->timerId, NULL, NULL, NULL);
	err = rx_init(&self->rxObj, prio, uartBase,
					(rxCb_t) &ao_post, self, eQueueLenRx);
	err = tx_init(&self->txObj, prio, uartBase,
					(txCb_t) &ao_post, self, eQueueLenTx);

	/* Initial state and register to scheduler (does initial transition) */
	self->super.super.objType = OBJTYPE_HX_OBJ;

	eQueueMem = malloc(sizeof(struct event) * eQueueLenHx);
    ao_init_event_queue((struct ao *) self, eQueueMem, eQueueLenHx);
	stateMem = malloc(sizeof(struct hsmState) * HX_STATE_NESTING);
	memset(stateMem, 0, sizeof(struct hsmState) * HX_STATE_NESTING);
    ao_init_hsm_state_memory((struct aoHsm *) self, stateMem, HX_STATE_NESTING);
	HSM_SET_STATE(self, &off, LVL0);
	ao_register((struct ao *) self, prio, true);
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * The off state. The timer is not running, no messages are sent or received.
 */
void off(struct hxComObj *self, struct event *e)
{
	switch(e->sig){
	case STATE_ENTRY_SIG:
        if(self->hw.txEnBaseNPin != NULL)
            TX_DIS(self);
        if(self->hw.rxEnBaseNPin != NULL)
            RX_DIS(self);
		break;
	case STATE_EXIT_SIG:
		break;
	case HX_ON_SIG:
		ucBuffer_clear(&self->rxObj.rxBuf);
		HSM_SET_STATE(self, &on, LVL0);
		HSM_STATE_TRAN(e, 1);
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The on state. This is the super-state if the HX driver is running. The
 * four contained sub-states are found below.
 */
void on(struct hxComObj *self, struct event *e)
{
	switch(e->sig){
	case STATE_ENTRY_SIG:
		break;
	case STATE_INIT_SIG:
		HSM_SET_STATE(self, &waiting, LVL1);
		break;
	case STATE_EXIT_SIG:
		break;
	case HX_OFF_SIG:
		timerD_stop_timer(self->timerId);
		HSM_SET_STATE(self, &off, LVL0);
		HSM_STATE_TRAN(e, 1);
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The waiting state, waiting for the timer to move the state machine to send.
 * While in this state, neither is anything sent nor should anything be
 * received. If there is still something received, it is cleared.
 */
void waiting(struct hxComObj *self, struct event *e)
{
	switch(e->sig){
	case STATE_ENTRY_SIG:
		timerD_stop_timer(self->timerId);
		break;
	case STATE_EXIT_SIG:
		break;
	case HX_GO_SIG:
		HSM_SET_STATE(self, &send, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	case RX_DONE_SIG:
	case RX_ERR_SIG:
		HSM_EVENT_HANDLED(e);
		ucBuffer_clear(&self->rxObj.rxBuf);
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The send state. This state triggers the TX object to transmit the buffered
 * message. No RX signal should occur here, if still happening, it is
 * considered an error.
 */
void send(struct hxComObj *self, struct event *e)
{
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
	    if(self->hw.txEnBaseNPin != NULL)
	        TX_EN(self);
	    if(self->timing.txDelay){
	        tmpE.sig = HX_TX_DELAY_SIG;
	        timerD_set_timer(self->timerId, self->timing.txDelay,
	                         TD_SINGLE_SHOT,
	                         (timerCb_t) &ao_post, self, &tmpE);
	        timerD_start_timer(self->timerId);
	    }else{
	        tmpE.sig = TX_GO_SIG;
	        ao_post((struct ao *) &self->txObj, &tmpE);
	    }
		break;
	case STATE_EXIT_SIG:
        self->flags &= ~FLAG_LB_RX_EARLY;
        if(self->hw.txEnBaseNPin != NULL)
            TX_DIS(self);
		break;
	case HX_TX_DELAY_SIG:
        HSM_EVENT_HANDLED(e);
	    tmpE.sig = TX_GO_SIG;
        ao_post((struct ao *) &self->txObj, &tmpE);
	    break;
	case TX_DONE_SIG:
	    if(self->timing.timeout > 0){
            tmpE.sig = HX_TIMEOUT_SIG;
            timerD_set_timer(self->timerId,
                             self->timing.timeout * 1000,
                             TD_SINGLE_SHOT,
                             (timerCb_t) &ao_post, self, &tmpE);
            timerD_start_timer(self->timerId);
	    }
		if(self->config & HX_CONF_LB_EN){
		    if(self->flags & FLAG_LB_RX_EARLY){
	            HSM_SET_STATE(self, &receive, LVL1);
	            HSM_STATE_TRAN(e, 1);
		    }else{
                HSM_SET_STATE(self, &receive_lb, LVL1);
                HSM_STATE_TRAN(e, 1);
		    }
		}else{
			HSM_SET_STATE(self, &receive, LVL1);
			HSM_STATE_TRAN(e, 1);
		}
		break;
	case TX_ERR_SIG:
		if(self->cb.func != NULL){
			tmpE.data = HX_ERR_TXERR | e->data;
			tmpE.sig = HX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		HSM_SET_STATE(self, &waiting, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	case RX_DONE_SIG:
        HSM_EVENT_HANDLED(e);
		ucBuffer_clear(&self->rxObj.rxBuf);
        if(self->config & HX_CONF_LB_EN){  /* the lb RX accidentally arrived before TX */
            self->flags |= FLAG_LB_RX_EARLY;
        }else{
            tmpE.data = HX_INV_RX_SRX;
            tmpE.sig = HX_INV_RX_SIG;
            self->cb.func(self->cb.handle, &tmpE);
        }
        break;
	case RX_ERR_SIG:
		if(self->config & HX_CONF_LB_EN){
			tmpE.data = HX_ERR_LBERR | e->data;
			tmpE.sig = HX_ERR_SIG;
            if(self->cb.func != NULL)
                self->cb.func(self->cb.handle, &tmpE);
            tmpE.sig = TX_STOP_SIG;
            ao_post((struct ao *) &self->txObj, &tmpE);
            HSM_SET_STATE(self, &waiting, LVL1);
            HSM_STATE_TRAN(e, 1);
		}else{
			tmpE.data = HX_INV_RX_SERR | e->data;
            tmpE.sig = HX_INV_RX_SIG;
            if(self->cb.func != NULL)
                self->cb.func(self->cb.handle, &tmpE);
		}
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The receive loop backed state. This state is basically the same as the
 * receive state, but on message reception, the message is cleared and it
 * is moved on to receive state. This state is only used if the loop back
 * flag is set, to receive a looped back message, e.g. if you have any
 * hardware transmitter that implements a loop back.
 */
void receive_lb(struct hxComObj *self, struct event *e)
{
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		break;
	case STATE_EXIT_SIG:
		break;
	case HX_TIMEOUT_SIG:
		if(self->cb.func != NULL){
			tmpE.data = HX_NO_RESPONSE_ERR_LOOPBACK;
			tmpE.sig = HX_NO_RESPONSE_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		HSM_SET_STATE(self, &waiting, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	case RX_ERR_SIG:
		if(self->cb.func != NULL){
			tmpE.data = HX_ERR_LBERR | e->data;
			tmpE.sig = HX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		HSM_SET_STATE(self, &waiting, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	case RX_DONE_SIG:
		ucBuffer_clear(&self->rxObj.rxBuf);
		HSM_SET_STATE(self, &receive, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The receive state, receiving the slave response.
 */
void receive(struct hxComObj *self, struct event *e)
{
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
        if(self->hw.rxEnBaseNPin != NULL)
            RX_EN(self);
		break;
	case STATE_EXIT_SIG:
        if(self->hw.rxEnBaseNPin != NULL)
            RX_DIS(self);
		break;
	case HX_TIMEOUT_SIG:
		if(self->cb.func != NULL){
			tmpE.data = HX_NO_RESPONSE_ERR_RECEIVE;
			tmpE.sig = HX_NO_RESPONSE_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		goto trans_waiting;
	case RX_ERR_SIG:
		if(self->cb.func != NULL){
			tmpE.data = HX_ERR_RXERR | e->data;
			tmpE.sig = HX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		goto trans_waiting;
	case RX_RECEIVING_SIG:
		timerD_stop_timer(self->timerId);
		break;
	case RX_DONE_SIG:
		if(self->cb.func != NULL){
			tmpE.data = e->data;
			tmpE.sig = HX_RX_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
trans_waiting:
		HSM_SET_STATE(self, &waiting, LVL1);
		HSM_STATE_TRAN(e, 1);
		break;
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

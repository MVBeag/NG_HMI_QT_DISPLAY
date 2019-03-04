/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: uartTxObj.c
 * Date			: 11.08.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:	This active object can be used to transmit a message on an
 * 				UART port. The message is specific to our protocol as the
 * 				first layer is added in the progress of transmission.
 * 				The first layer is constructed as follows:
 * 				[PREAMBLE(1), PAYLOAD LENGTH(1), PAYLOAD(x), CRC(2)].
 *
 *              Callback
 *              --------
 *				Errors and message reception are reported over a callback
 * 				function. The prototype of the callback function looks as
 * 				follows: int32_t (*cb)(void *, struct event *).
 * 				This prototype matches the ao_post and ao_dispatch function,
 * 				so one of these two is typically assigned as the callback
 * 				function. Callback function and the void * argument is
 * 				assigned with the objects initializing function (tx_init).
 * 				The event given with the callback is either TX_DONE_SIG
 * 				or TX_ERR_SIG.
 *
 * 				Transmission
 * 				------------
 * 				Transmission of a message can be started with the TX_GO_SIG.
 * 				The message on the buffer from .pos to .len is then moved
 * 				into the TX FIFO if the state machine is in idle state.
 * 				Since the TX FIFO is only 16 bytes deep, it is likely that
 * 				not the whole message on the buffer fits into the FIFO in
 * 				one go. Therefore, there are two states in the send process,
 * 				one if there are still bytes to read from the buffer (busy)
 * 				and one if there only remain bytes in the FIFO (eot).
 * 				If the TX_GO_SIG is given while the object is transmitting,
 * 				namely in busy or eot state, an error will be thrown once
 * 				the transmission is finished.
 *
 * 				With the TX_STOP_SIG, the transmission can be stopped. The
 * 				state machine moves into eot state sending the remaining
 * 				characters on the FIFO. An error will be thrown once the
 * 				transmission is finished.
 *
 * 				Message transmission complete is signaled with the TX_DONE_SIG
 * 				and also with the TX_ERR_SIG. In the first case, transmission
 * 				has been successful. The two signals indicate that the last
 * 				bit has left the serializer and hence the UART is no more busy.
 *
 * 				One major drawback of the UART module is, that the transmit
 * 				interrupt is either EOT, hence when the last bit leaves the
 * 				serializer, or a FIFO level, but not both. Because it has to
 * 				be known when the message is truly sent (e.g. to switch the
 * 				transmitter off) the EOT interrupt is chosen. However, this
 * 				means that time gaps between the bytes can occur if the
 * 				interrupt is not handled immediately. It cannot bet swaped
 * 				between EOT and FIFO interrupt since the module has to be
 * 				disabled to do so.
 *
 *              Error
 *              -----
 * 				Errors are signaled with the TX_ERR_SIG. Possible errors are
 * 				buffer empty if len <= pos, UART busy, if TX_GO_SIG while
 * 				in busy or eot state and stop, if the TX_STOP_SIG has been
 * 				given while in busy or eot state. The error cause can be
 * 				found in the data field of the event. When an error is
 * 				thrown, the state will be idle in any case, meaning that
 * 				the transmitter is ready again.
 *
 *              Buffer
 *              ------
 *              The buffer must be filled with the PAYLOAD, preferably by
 *              using the encode function found in protocol.h. Typically
 *              the .len field is set to the number of bytes to transmit and
 *              the .pos field to 0. The number of characters to transmit
 *              is determined by the difference len-pos, where pos is the
 *              index to start from.
 * 				When transmitting a message, the only buffer field that is
 * 				adjusted is the pos field. Once a transmission is complete,
 * 				the pos field equals the len field. Since nothing else is
 * 				adjusted, re-transmission of a message is as simple as setting
 * 				pos=0.
 *
 * Usage:
 * 				 -	Make a static uint8_t array to allocate memory for the
 * 				 	transmit buffer (any size).
 * 				 -	Make a static struct uartTxObj variable and assign the
 * 				 	memory and size of the buffer.
 * 				 -	Write the interrupt handler (copy it from somewhere).
 * 				 -	Configure and Enable the UART module. Most important:
 * 				 	set the FEN bit in the LCRH register to enable the FIFO
 * 				 	and set the EOT bit in the CTL register.
 * 				 	Enable interrupts in the NVIC and also configure the I/O's.
 * 				 -	Call the tx_init function. TX interrupt becomes unmasked.
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config/projConfig.h"
#include "driver/com/uartTxObj.h"
#include "lib/prot/protocol.h"
#include "lib/stm/event.h"
#include "lib/crc/crc16Lookup.h"

#include "inc/hw_types.h"  /* HWREG macro */
#include "inc/hw_uart.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
/* defines for the .flags field */
#define FLAGS_UARTBUSY_ERR        0x01
#define FLAGS_TX_STOP             0x02

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/* Since this is an object, it can't have local variables, otherwise it
 * wouldn't be possible to use it multiple times! */

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
/* other subroutines */
static void uart_tx_init(struct uartTxObj *);
static void write_payload(struct uartTxObj *, uint32_t);

/* state machine */
static void tx_idle(struct uartTxObj *, struct event *);
static void tx_busy(struct uartTxObj *, struct event *);
static void tx_eot(struct uartTxObj *, struct event *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Configures TX interrupt and TX DMA. FIFO level is not used since the EOT
 * bit in the CTL register must be set. Hence an interrupt is thrown on FIFO
 * empty. DMA operation is disabled.
 * Note that the main UART configuration must be done by the object holder
 * (e.g. the baudrate, eot, ...). Also make sure not to overwrite this
 * configuration and that the UART module is clocked before calling the
 * tx_init function.
 *
 * Argument:	self	Reference to TX object.
 */
void uart_tx_init(struct uartTxObj *self)
{
	HWREG(self->uartBase + UART_O_ICR) = UART_ICR_TXIC;
	HWREG(self->uartBase + UART_O_IM) |= UART_IM_TXIM;
	HWREG(self->uartBase + UART_O_DMACTL) &= ~UART_DMACTL_TXDMAE;
}
/*---------------------------------------------------------------------------*/

/*
 * Reads the payload from the buffer and moves it into the TX FIFO. Further
 * the CRC is calculated on the go.
 * Note that no safety measures are done, neither to protect from buffer
 * overrun nor to protect from FIFO overrun. Both is expected to be ensured
 * by the caller.
 *
 * Argument:	self	Reference to TX object.
 * 				len		Number of characters to send from the buffer.
 */
void write_payload(struct uartTxObj *self, uint32_t len)
{
	int32_t i;
	uint8_t tmp;

	for(i=0; i<len; i++){
		tmp = self->txBuf.buf[self->txBuf.pos++];
		crc16_ccitt_byte_calc(&self->crc, tmp);
		HWREG(self->uartBase + UART_O_DR) = tmp;
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Initialize the transmit buffer.
 *
 * Argument:	self	Reference to TX object.
 * 				buf		Pointer to allocated buffer memory.
 * 				len		Length of the buffer in bytes.
 */
uint32_t txObj_init_buffer(struct uartTxObj *self, uint8_t *buf, uint16_t len)
{
	self->txBuf.buf = buf;
	self->txBuf.size = len;
	self->txBuf.len = 0;
	self->txBuf.pos = 0;
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Sets the .pos field of the TX buffer to 0, indicating that the buffered
 * message has not yet been sent. This is an easy way to re-send a message,
 * without filling the buffer again. The buffer content is not altered
 * during the send process.
 *
 * Argument:	self	Reference to TX object.
 */
void txObj_set_buffered_unsent(struct uartTxObj *self)
{
	self->txBuf.pos = 0;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * STATE MACHINE
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * This function is typically only called once by the object holder.
 * It can be crowded with initialization functions and must be used to
 * initialize and enter the first state.
 *
 * Argument:	self		Reference to TX object.
 * 				prio		Active object priority.
 * 				uartBase	The base memory address of the UART module.
 * 				cb			The callback function.
 * 				cbHandle	Handle passed with the callback function.
 * 				eQueueLen	Desired length of event queue.
 * Return:		 0			success
 * 				-1			event queue memory not assigned/allocated
 */
int32_t tx_init(struct uartTxObj *self, uint8_t prio, uint32_t uartBase,
				txCb_t cb, void *cbHandle, uint32_t eQueueLen)
{
	void *eQueueMem;

	/* Top transition and action */
	self->uartBase = uartBase;
	self->cb.func = cb;
	self->cb.handle = cbHandle;
	self->txBuf.len = 0;
	self->flags = 0;
	uart_tx_init(self);

	/* Initial state and register to scheduler (does initial transition) */
    self->super.super.objType = OBJTYPE_TX_OBJ;
	eQueueMem = malloc(sizeof(struct event) * eQueueLen);
	ao_init_event_queue((struct ao *) self, eQueueMem, eQueueLen);
	STM_SET_STATE(self, &tx_idle);
	ao_register((struct ao *) self, prio, false);
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * The transmit idle state waiting for the trigger TX_GO_SIG to start
 * transmitting a buffered message. Once the trigger is received, it is
 * checked whether the buffer contains any elements (len > pos). If yes,
 * the message is transmitted by starting with [preamble, payload length].
 * If the hole message fits into the FIFO (16 characters where 4x header +
 * up to 12x payload) a transition to tx_eot is done. Otherwise, 15 characters
 * are pushed onto the FIFO (2x header + 13x payload) and a transition to
 * tx_busy is done. It's 15 characters because the CRC must not be split.
 */
void tx_idle(struct uartTxObj *self, struct event *e)
{
	int32_t nChr;
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		break;
	case STATE_EXIT_SIG:
		break;
	case TX_GO_SIG:
		self->crc = CRC16_CCITT_INIT_0000;
		nChr = self->txBuf.len - self->txBuf.pos;
		if(nChr <= 0){  /* buffer empty? */
			if(self->cb.func != NULL){
				tmpE.data = TX_ERR_BUFFEREMPTY;
				tmpE.sig = TX_ERR_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
		}else{  /* Okey, start transmission */
			HWREG(self->uartBase + UART_O_DR) = PREAMBLE;
			HWREG(self->uartBase + UART_O_DR) = nChr;
			if(nChr <= (16-DLINK_H_LEN)){
				write_payload(self, nChr);
				HWREG(self->uartBase + UART_O_DR) = self->crc >> 8;
				HWREG(self->uartBase + UART_O_DR) = self->crc;
				STM_STATE_TRAN(self, &tx_eot, e);
			}else{
				write_payload(self, 15-2);
				STM_STATE_TRAN(self, &tx_busy, e);
			}
		}
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The transmit busy state. In this state, further data is in the buffer,
 * waiting to be sent as soon as the FIFO becomes empty. Once the FIFO is
 * empty either the remaining message is pushed onto the FIFO if it fits
 * or 15 characters otherwise. In case that the remaining message fits, a
 * transition to tx_eot is done.
 * If the TX_GO_SIG or the TX_STOP_SIG is received, an error is thrown
 * and a transition to either eot or idle is done, depending on whether
 * the UART is busy or not.
 */
void tx_busy(struct uartTxObj *self, struct event *e)
{
	uint32_t nChr;
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		break;
	case STATE_EXIT_SIG:
		break;
	case TX_SIG:
		nChr = self->txBuf.len-self->txBuf.pos;
		if(nChr <= (16-CRC_LEN)){
			write_payload(self, nChr);
			HWREG(self->uartBase + UART_O_DR) = self->crc >> 8;
			HWREG(self->uartBase + UART_O_DR) = self->crc;
			STM_STATE_TRAN(self, &tx_eot, e);
		}else{
			write_payload(self, 15);
		}
		break;
	case TX_GO_SIG:
	    self->flags |= FLAGS_UARTBUSY_ERR;
		goto check_busy;
	case TX_STOP_SIG:
        self->flags |= FLAGS_TX_STOP;
    check_busy:
        if(HWREGBITW(self->uartBase + UART_O_FR, 3)){  /* UART busy */
            STM_STATE_TRAN(self, &tx_eot, e);
        }else{
            STM_STATE_TRAN(self, &tx_idle, e);
            if(self->cb.func != NULL){
                if(self->flags & FLAGS_UARTBUSY_ERR){
                    tmpE.data = TX_ERR_UARTBUSY;
                }else{
                    tmpE.data = TX_ERR_STOP;
                }
                tmpE.sig = TX_ERR_SIG;
                self->cb.func(self->cb.handle, &tmpE);
            }
            self->flags &= ~FLAGS_UARTBUSY_ERR;
            self->flags &= ~FLAGS_TX_STOP;
        }
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The end of transmission state. Once the FIFO becomes empty, message
 * transmission has succeeded and it is returned to idle state. When
 * entering this state, the buffer is already empty (pos = len). Hence
 * a new message can be stored on the buffer.
 * To increase reliability, the TX_GO_SIG and the TX_STOP_SIG can move
 * the state machine back to idle state if the UART is no more busy
 * and hence the state machine is somehow stuck here.
 * In any case, TX_STOP_SIG will lead to TX_DONE_SIG callback,
 * TX_GO_SIG will lead to TX_ERR_SIG callback, both thrown when
 * returning to idle state.
 */
void tx_eot(struct uartTxObj *self, struct event *e)
{
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		break;
	case STATE_EXIT_SIG:
		break;
	case TX_SIG:
		STM_STATE_TRAN(self, &tx_idle, e);
		if(self->cb.func != NULL){
		    if(self->flags & FLAGS_UARTBUSY_ERR){
	            tmpE.data = TX_ERR_UARTBUSY;
	            tmpE.sig = TX_ERR_SIG;
		    }else if(self->flags & FLAGS_TX_STOP){
                tmpE.data = TX_ERR_STOP;
                tmpE.sig = TX_ERR_SIG;
		    }else{
                tmpE.sig = TX_DONE_SIG;
		    }
			self->cb.func(self->cb.handle, &tmpE);
		}
        self->flags &= ~FLAGS_UARTBUSY_ERR;
        self->flags &= ~FLAGS_TX_STOP;
		break;
	case TX_GO_SIG:
	    self->flags |= FLAGS_UARTBUSY_ERR;
        goto check_busy;
    case TX_STOP_SIG:
        self->flags |= FLAGS_TX_STOP;
    check_busy:
        if(HWREGBITW(self->uartBase + UART_O_FR, 3))  /* UART busy */
            break;
        STM_STATE_TRAN(self, &tx_idle, e);
        if(self->cb.func != NULL){
            if(self->flags & FLAGS_UARTBUSY_ERR){
                tmpE.data = TX_ERR_UARTBUSY;
            }else{
                tmpE.data = TX_ERR_STOP;
            }
            tmpE.sig = TX_ERR_SIG;
            self->cb.func(self->cb.handle, &tmpE);
        }
        self->flags &= ~FLAGS_UARTBUSY_ERR;
        self->flags &= ~FLAGS_TX_STOP;
        break;
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: uartRxObj.c
 * Date			: 11.08.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description: This active object can be used to receive a message, specific
 *				to our protocol, on an UART port. Basically, it checks and
 *				removes the first layer and stores the PAYLOAD on a buffer.
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
 * 				assigned with the objects initializing function (rx_init).
 *
 *              Receive
 *              -------
 * 				Once a message is received with no error, the RX_DONE_SIG is
 * 				thrown through the callback function. The complete message
 * 				(without the first layer, which has been checked already)
 * 				can	be found in the RX buffer. It should be further
 * 				processed with the decode function found in protocol.h.
 * 				Note that the buffer.len field is written once a message is
 * 				successfully received, telling that the receive buffer is
 * 				occupied. Hence in case of an error, buffer.len is not
 * 				written and the buffer remains 'empty'.
 *
 * 				Error
 * 				-----
 * 				If any error occurs, the RX_ERR_SIG is thrown through the
 * 				callback function. Possible errors are wrong PREAMBLE,
 * 				buffer occupied (len != 0), message to long (if it won't
 *              fit into the receive buffer), message to short (if a
 * 				timeout occurs), CRC mismatch or any HW error. The error
 * 				cause can be found in the data field of the event.
 *
 * 				Buffer
 * 				------
 * 				The RX buffer holds a pointer to the buffer memory, the
 * 				number of allocated bytes (size), a .pos field to reference
 * 				the write position of the buffer and a message length field
 * 				(.len). The .len field is filled with the PAYLOAD length
 * 				once a message has been successfully received. The processing
 * 				functions are responsible for setting the .len field back to 0,
 * 				otherwise the object will not start fetching incoming data
 * 				again, as the buffer is marked as 'occupied'.
 *
 * Usage:
 * 				 -	Make a static uint8_t array to allocate memory for the
 * 				 	receive buffer (any size).
 * 				 -	Make a static struct uartRxObj variable and assign the
 * 				 	memory and size of the receive buffer.
 * 				 -	Write the interrupt handler (copy it from somewhere).
 * 				 -	Configure and Enable the UART module. Most important:
 * 				 	set the FEN bit in the LCRH register to enable the FIFO.
 * 				 	Enable interrupts in the NVIC and also configure the I/O's.
 * 				 -	Call the rx_init function. RX, timeout and error
 * 				 	interrupts are unmasked and the FIFO level is set.
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config/projConfig.h"
#include "user/debug/debugTask.h"
#include "driver/com/uartRxObj.h"
#include "lib/prot/protocol.h"
#include "lib/stm/event.h"
#include "lib/crc/crc16Lookup.h"

#include "inc/hw_types.h"  /* HWREG macro */
#include "inc/hw_uart.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
#define MAX_RX_BUF_DEPTH		14
#define FIFO_REMAINDER			4

#define DISABLE_RX_INT			HWREG(self->uartBase + UART_O_IM) &= \
																~UART_IM_RXIM
#define ENABLE_RX_INT			HWREG(self->uartBase + UART_O_IM) |= \
																UART_IM_RXIM
#define ENABLE_TIMEOUT_INT		HWREG(self->uartBase + UART_O_IM) |= \
																UART_IM_RTIM

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/* Since this is an object, it can't have local variables, otherwise it
 * wouldn't be possible to use it multiple times! */

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
/* other subroutines */
static void uart_rx_init(struct uartRxObj *);
static void clear_uart_rx_fifo(struct uartRxObj *);
static void read_payload(struct uartRxObj *, int32_t);

/* state machine */
static void rx_idle(struct uartRxObj *, struct event *);
static void rx_busy(struct uartRxObj *, struct event *);
static void rx_eor(struct uartRxObj *, struct event *);
static void rx_error(struct uartRxObj *, struct event *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Configures RX interrupts and RX DMA. RX FIFO is setup when entering the
 * first state. DMA is disabled and interrupts are thrown on RX FIFO level
 * match, RX timeout and several RX errors.
 * Note that the main UART configuration must be done by the object holder
 * (e.g. the baudrate). Also make sure not to overwrite this configuration
 * and that the UART module is clocked before calling the rx_obj init.
 *
 * Argument:	self	Reference to RX object.
 */
void uart_rx_init(struct uartRxObj * self)
{
	HWREG(self->uartBase + UART_O_ICR) = 0
				| UART_ICR_OEIC  /* UART Overrun Error */
				| UART_ICR_BEIC  /* UART Break Error */
				| UART_ICR_PEIC  /* UART Parity Error */
				| UART_ICR_FEIC  /* UART Framing Error */
				| UART_ICR_RTIC  /* UART Receive Time-Out */
				| UART_ICR_RXIC  /* UART Receive */
				| 0;
	HWREG(self->uartBase + UART_O_IM) |= 0
				| UART_IM_OEIM  /* UART Overrun Error */
				| UART_IM_BEIM  /* UART Break Error */
				| UART_IM_PEIM  /* UART Parity Error */
				| UART_IM_FEIM  /* UART Framing Error */
				| UART_IM_RTIM  /* UART Receive Time-Out */
				| UART_IM_RXIM  /* UART Receive */
				| 0;
	HWREG(self->uartBase + UART_O_DMACTL) &= ~(UART_DMACTL_DMAERR
			| UART_DMACTL_RXDMAE);
}
/*---------------------------------------------------------------------------*/

/*
 * Reads the UART RX FIFO until it is empty.
 *
 * Argument:	self	Reference to RX object.
 */
void clear_uart_rx_fifo(struct uartRxObj * self)
{
	volatile uint8_t rxChr;

	while(!HWREGBITW(self->uartBase + UART_O_FR, 4)){
		rxChr = HWREG(self->uartBase + UART_O_DR);
	}
}
/*---------------------------------------------------------------------------*/

/*
 * Reads the UART RX FIFO, calculates the CRC on the characters and stores
 * them on the buffer. Number of characters to read can be given with the
 * second argument. Negative numbers can be used to readout all characters
 * on the FIFO.
 *
 * Argument:	self	Reference to RX object.
 * 				len		Number of characters to read from RX FIFO. Negative
 * 						number means reading buffer till empty.
 */
void read_payload(struct uartRxObj * self, int32_t len)
{
	int32_t i;
	volatile uint8_t rxChr;

	if(len < 0){
		while(!HWREGBITW(self->uartBase + UART_O_FR, 4)){
			rxChr = HWREG(self->uartBase + UART_O_DR);
			crc16_ccitt_byte_calc(&self->dlink.crc, rxChr);
			self->rxBuf.buf[self->rxBuf.pos++] = rxChr;
		}
	}else{
		for(i=0; i<len; i++){
			rxChr = HWREG(self->uartBase + UART_O_DR);
			crc16_ccitt_byte_calc(&self->dlink.crc, rxChr);
			self->rxBuf.buf[self->rxBuf.pos++] = rxChr;
		}
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 * Argument:	self	Reference to RX object.
 * 				buf		Pointer to allocated buffer memory.
 * 				len		Length of the buffer in bytes.
 */
uint32_t rxObj_init_buffer(struct uartRxObj *self, uint8_t *buf, uint16_t len)
{
	self->rxBuf.buf = buf;
	self->rxBuf.size = len;
	self->rxBuf.len = 0;
	self->rxBuf.pos = 0;
	return 0;
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
 * Argument:	self		Reference to RX object.
 * 				prio		Active object priority.
 * 				uartBase	The base memory address of the UART module.
 * 				cb			The callback function.
 * 				cbHandle	Handle passed with the callback function.
 * 				eQueueLen	Desired length of event queue.
 * Return:		 0			success
 * 				-1			event queue memory not assigned/allocated
 */
int32_t rx_init(struct uartRxObj *self, uint8_t prio, uint32_t uartBase,
				rxCb_t cb, void *cbHandle, uint32_t eQueueLen)
{
	void *eQueueMem;

	/* Top transition and action */
	self->uartBase = uartBase;
	self->cb.func = cb;
	self->cb.handle = cbHandle;
	self->rxBuf.len = 0;
	uart_rx_init(self);

	/* Initial state and register to scheduler (does initial transition) */
    self->super.super.objType = OBJTYPE_RX_OBJ;
	eQueueMem = malloc(sizeof(struct event) * eQueueLen);
	ao_init_event_queue((struct ao *) self, eQueueMem, eQueueLen);
	STM_SET_STATE(self, &rx_idle);
	ao_register((struct ao *) self, prio, false);
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * The idle state waiting on incoming data. The idle state sets the RX FIFO
 * depth to 8 characters. Once the trigger level is reached, preamble and
 * expected message length are read (2 characters). It is then checked whether
 * the preamble is correct and if the receive buffer is not in use. If these
 * conditions are met, the first two characters of the payload are read, so
 * that 4 remain in the RX FIFO (this is the requirement to transition to
 * rx_eor). If more than 14 characters are still expected, we transition to
 * rx_busy and to rx_eor otherwise. Since there is the requirement of having
 * at least four remaining bytes, the trigger level must be set to 8
 * characters here, meaning that no message can possibly be shorter than 8
 * characters. However, this is the case with our protocol.
 */
void rx_idle(struct uartRxObj *self, struct event *e)
{
	uint8_t preamble;
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		clear_uart_rx_fifo(self);
		HWREG(self->uartBase + UART_O_IFLS) &= ~UART_IFLS_RX_M;
		HWREG(self->uartBase + UART_O_IFLS) |= UART_IFLS_RX4_8;
		break;
	case STATE_EXIT_SIG:
		break;
	case RX_SIG:
		preamble = HWREG(self->uartBase + UART_O_DR);
		self->dlink.len = HWREG(self->uartBase + UART_O_DR);
		tmpE.data = 0;
		if(preamble != PREAMBLE)
			tmpE.data = RX_ERR_PREAMBLEMISMATCH;
		else if(self->rxBuf.len != 0)
			tmpE.data = RX_ERR_BUFFERUSED;
		else if(self->dlink.len+CRC_LEN > self->rxBuf.size)
			tmpE.data = RX_ERR_MSGTOOLONG;
	    if(tmpE.data){  /* if error */
			if(self->cb.func != NULL){
				tmpE.sig = RX_ERR_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
			STM_STATE_TRAN(self, &rx_error, e);
		}else{
			if(self->cb.func != NULL){
				tmpE.sig = RX_RECEIVING_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
			self->dlink.crc = CRC16_CCITT_INIT_0000;
			self->rxBuf.pos = 0;
			read_payload(self, 2);  /* 8-2-2 = 4 remain */
			if((self->dlink.len + CRC_LEN - self->rxBuf.pos)
					<= MAX_RX_BUF_DEPTH){
				STM_STATE_TRAN(self, &rx_eor, e);
			}else{
				STM_STATE_TRAN(self, &rx_busy, e);
			}
		}
		ENABLE_RX_INT;
		break;
	case RX_TIMEOUT_SIG:
		clear_uart_rx_fifo(self);
		if(self->cb.func != NULL){
			tmpE.data = RX_ERR_MSGTOOSHORT;
			tmpE.sig = RX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		ENABLE_TIMEOUT_INT;
		break;
	case RX_ERROR_SIG:
		/* RX FIFO empty? This is a strange case, because a faulty
		   received character should also be stored on the FIFO! */
		if(!HWREGBITW(self->uartBase + UART_O_FR, 4)){
			if(self->cb.func != NULL){
				tmpE.data = e->data;
				tmpE.sig = RX_ERR_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
			STM_STATE_TRAN(self, &rx_error, e);
		}
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The receive busy state. This state is only entered in case of a message
 * that is greater than 18 bytes. The RX FIFO is set to its maximal interrupt
 * depth (14 characters). Once the trigger level is reached, 10 characters are
 * read so that 4 remain in the RX FIFO (this is the requirement to transition
 * to rx_eor). If more than 14 characters are still expected we remain in this
 * state, otherwise a transition to rx_eor is done.
 */
void rx_busy(struct uartRxObj *self, struct event *e)
{
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		DISABLE_RX_INT;
		HWREG(self->uartBase + UART_O_IFLS) &= ~UART_IFLS_RX_M;
		HWREG(self->uartBase + UART_O_IFLS) |= UART_IFLS_RX7_8;
		ENABLE_RX_INT;
		break;
	case STATE_EXIT_SIG:
		break;
	case RX_SIG:
		read_payload(self, 10);
		if((self->dlink.len + CRC_LEN - self->rxBuf.pos)
				<= MAX_RX_BUF_DEPTH){
			STM_STATE_TRAN(self, &rx_eor, e);
		}
		ENABLE_RX_INT;
		break;
	case RX_TIMEOUT_SIG:
		if(self->cb.func != NULL){
			tmpE.data = RX_ERR_MSGTOOSHORT;
			tmpE.sig = RX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		ENABLE_TIMEOUT_INT;
		STM_STATE_TRAN(self, &rx_idle, e);
		break;
	case RX_ERROR_SIG:
		if(self->cb.func != NULL){
			tmpE.data = e->data;
			tmpE.sig = RX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		STM_STATE_TRAN(self, &rx_error, e);
		break;
	}
}
/*---------------------------------------------------------------------------*/


/*
 * The end of reception state. Whenever this state is entered, it must be
 * guaranteed that the RX FIFO holds at least four bytes. Further it must be
 * guaranteed that the expected amount of bytes doesn't exceed the FIFOs depth.
 * These rules allow to setup the RX FIFO interrupt level in a way that only
 * one more interrupt is needed to complete message reception.
 */
void rx_eor(struct uartRxObj *self, struct event *e)
{
	uint32_t nRemaining;
	struct event tmpE;

	switch(e->sig){
	case STATE_ENTRY_SIG:
		nRemaining = self->dlink.len + CRC_LEN - self->rxBuf.pos;
		if(nRemaining == 4){
			tmpE.sig = RX_SIG;
			ao_post((struct ao *) self, &tmpE);
		}else{
			read_payload(self, (nRemaining & 0x3));
			DISABLE_RX_INT;
			HWREG(self->uartBase + UART_O_IFLS) &= ~UART_IFLS_RX_M;
			HWREG(self->uartBase + UART_O_IFLS) |= nRemaining << 1;
			ENABLE_RX_INT;
		}
		break;
	case STATE_EXIT_SIG:
		break;
	case RX_SIG:
		read_payload(self, -1);
		if(self->dlink.crc == 0){
			self->rxBuf.len = self->dlink.len;
			if(self->cb.func != NULL){
				tmpE.sig = RX_DONE_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
		}else{
			if(self->cb.func != NULL){
				tmpE.data = RX_ERR_CRCMISMATCH;
				tmpE.sig = RX_ERR_SIG;
				self->cb.func(self->cb.handle, &tmpE);
			}
		}
		ENABLE_RX_INT;
		STM_STATE_TRAN(self, &rx_idle, e);
		break;
	case RX_TIMEOUT_SIG:
		if(self->cb.func != NULL){
			tmpE.data = RX_ERR_MSGTOOSHORT;
			tmpE.sig = RX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		ENABLE_TIMEOUT_INT;
		STM_STATE_TRAN(self, &rx_idle, e);
		break;
	case RX_ERROR_SIG:
		if(self->cb.func != NULL){
			tmpE.data = e->data;
			tmpE.sig = RX_ERR_SIG;
			self->cb.func(self->cb.handle, &tmpE);
		}
		STM_STATE_TRAN(self, &rx_error, e);
		break;
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The receive error state. Entering this state means that either a HW receive
 * error has been detected, a preamble mismatch occurred or the receive buffer
 * is in use (len!=0). The state is left once a receive timeout occurs.
 * Receive and HW error interrupts are masked since they would only cause
 * unnecessary data processing.
 */
void rx_error(struct uartRxObj *self, struct event *e)
{
	switch(e->sig){
	case STATE_ENTRY_SIG:
		HWREG(self->uartBase + UART_O_IM) &= ~(0
				| UART_IM_OEIM  /* UART Overrun Error */
				| UART_IM_BEIM  /* UART Break Error */
				| UART_IM_PEIM  /* UART Parity Error */
				| UART_IM_FEIM  /* UART Framing Error */
				| UART_IM_RXIM);  /* UART Receive */
		break;
	case STATE_EXIT_SIG:
		break;
	case RX_TIMEOUT_SIG:
		HWREG(self->uartBase + UART_O_ICR) = (0
				| UART_IM_OEIM  /* UART Overrun Error */
				| UART_IM_BEIM  /* UART Break Error */
				| UART_IM_PEIM  /* UART Parity Error */
				| UART_IM_FEIM  /* UART Framing Error */
				| UART_IM_RXIM);  /* UART Receive */
		HWREG(self->uartBase + UART_O_IM) |= (0
				| UART_IM_OEIM  /* UART Overrun Error */
				| UART_IM_BEIM  /* UART Break Error */
				| UART_IM_PEIM  /* UART Parity Error */
				| UART_IM_FEIM  /* UART Framing Error */
				| UART_IM_RXIM);  /* UART Receive */
		ENABLE_TIMEOUT_INT;
		STM_STATE_TRAN(self, &rx_idle, e);
		break;
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

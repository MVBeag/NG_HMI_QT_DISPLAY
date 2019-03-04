/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: protocol.c
 * Date			: 15.08.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:
 * 
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "protocol.h"
#include "prot/services/generator.h"
#include "mem/ucBuffer.h"
//#include "user/routing/route.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
const uint8_t netLayerLen[] = {1, 3, 5, 6};
const uint8_t tranLayerLen[] = {1, 4};

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
int32_t enc_network_layer(struct protocol *, struct ucBuffer *);
int32_t enc_transport_layer(struct protocol *, struct ucBuffer *);
int32_t enc_process_layer(struct protocol *, struct ucBuffer *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
int32_t enc_network_layer(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = 0;

	dest->pos = 0;
	switch(src->netLayer.nid){
	case 0:
		dest->buf[dest->pos++] = 0;
		break;
	case 1:
		dest->buf[dest->pos++] = 1;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid1.destAddr;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid1.srcAddr;
		break;
	case 2:
		dest->buf[dest->pos++] = 2;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid2.destDev;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid2.destChn;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid2.srcDev;
		dest->buf[dest->pos++] = (uint8_t) src->netLayer.opt.nid2.srcChn;
		break;
	case 3:
		dest->buf[dest->pos++] = 3;
		dest->buf[dest->pos++] = (uint8_t)(src->netLayer.opt.nid3.destAddr & 0xff00) >> 8;
		dest->buf[dest->pos++] = (uint8_t)(src->netLayer.opt.nid3.destAddr & 0xff);
		dest->buf[dest->pos++] = (uint8_t)(src->netLayer.opt.nid3.srcAddr & 0xff00) >> 8;
		dest->buf[dest->pos++] = (uint8_t)(src->netLayer.opt.nid3.srcAddr & 0xff);
		dest->buf[dest->pos++] = src->netLayer.opt.nid3.flags;
		break;
	default:
		return PROT_ERR_INVALID_NID;
	}
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t enc_transport_layer(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = 0;

	dest->pos = netLayerLen[src->netLayer.nid];
	switch(src->tranLayer.tid){
	case 0:
		dest->buf[dest->pos++] = 0;
		break;
	case 1:
		dest->buf[dest->pos++] = 1;
		dest->buf[dest->pos++] = src->tranLayer.opt.tid1.seqNr;
		dest->buf[dest->pos++] = src->tranLayer.opt.tid1.winSize;
		dest->buf[dest->pos++] = src->tranLayer.opt.tid1.flags;
		break;
	default:
		return PROT_ERR_INVALID_TID;
	}
	err = enc_network_layer(src, dest);
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Note that the pos field is used to reference the buffer instead of the
 * len field and the len is assigned at the very end, where pos is set
 * back to 0. This way, the buffer is marked as empty during the encoding
 * process.
 */
int32_t enc_process_layer(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = 0;
	uint16_t tmp;

	dest->pos = netLayerLen[src->netLayer.nid] + tranLayerLen[src->tranLayer.tid];
	dest->buf[dest->pos++] = (uint8_t)((src->procLayer.sid & 0xff00) >> 8);
	dest->buf[dest->pos++] = (uint8_t)(src->procLayer.sid & 0xff);

    /* Commented from MV */
	/* answers do have a service status field */
    /*
	if(src->procLayer.sid & SID_ANS_M)
		dest->buf[dest->pos++] = src->procLayer.sst;
    */

    /* call the service packer/encrypter */
	switch((src->procLayer.sid & SID_DEV_M) >> SID_DEV_S){
	case SID_DEV_GEN:
		err = generator_service_pack(src, dest);
		break;
	default:
		err = PROT_ERR_INVALID_SID;
		break;
	}
	tmp = dest->pos;
	err = enc_transport_layer(src, dest);
	dest->len = tmp;
	dest->pos = 0;
	return err;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Decodes the network layer of a message according to the next gen protocol.
 * The network layer is the outmost layer after the data link layer. However,
 * the data link layer is decoded directly within the receiver object.
 *
 * Note: The argument, which is a protocol structure, has to hold a reference
 * to the messages network header within its pData field and further the
 * remaining length of the message within its dLen field. After decoding,
 * pData is updated to point to the transport header and dLen is also updated
 * accordingly.
 *
 * Argument:	dec		Pointer to protocol structure.
 * Return:		PROT_SUCCESS
 * 				PROT_ERR_INVALID_NID
 *			 	PROT_ERR_INVALID_LEN
 */
int32_t prot_dec_network_layer(struct protocol *dec)
{
	uint8_t nid;
	uint8_t *src = (uint8_t *) dec->data.pData;

	nid = src[0];
	switch(nid){
	case 0:
		dec->netLayer.nid = nid;
		dec->data.pData = (void *) &src[1];
		dec->data.dLen -= 1;
		break;
	case 1:
		dec->netLayer.nid = nid;
		dec->netLayer.opt.nid1.destAddr = src[1];
		dec->netLayer.opt.nid1.srcAddr = src[2];
		dec->data.pData = (void *) &src[3];
		dec->data.dLen -= 3;
		break;
	case 2:
		dec->netLayer.nid = nid;
		dec->netLayer.opt.nid2.destDev = src[1];
		dec->netLayer.opt.nid2.destChn = src[2];
		dec->netLayer.opt.nid2.srcDev = src[3];
		dec->netLayer.opt.nid2.srcChn = src[4];
		dec->data.pData = (void *) &src[5];
		dec->data.dLen -= 5;
		break;
	case 3:
		dec->netLayer.nid = nid;
		dec->netLayer.opt.nid3.destAddr = (src[1] << 8) + src[2];
		dec->netLayer.opt.nid3.srcAddr = (src[3] << 8) + src[4];
		dec->netLayer.opt.nid3.flags = src[5];
		dec->data.pData = (void *) &src[6];
		dec->data.dLen -= 6;
		break;
	default:
		return PROT_ERR_INVALID_NID;
	}
	if(dec->data.dLen < 0)
		return PROT_ERR_INVALID_LEN;
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Decodes the transport layer of a message according to the next gen protocol.
 *
 * Note: The argument, which is a protocol structure, has to hold a reference
 * to the messages transport header within its pData field and further the
 * remaining length of the message within its dLen field. This is automatically
 * the case if prot_dec_network_layer() has been used in advance. After
 * decoding, pData is updated to point to the process header and dLen is also
 * updated accordingly.
 *
 * Argument:	dec		Pointer to protocol structure.
 * Return:		PROT_SUCCESS
 * 				PROT_ERR_INVALID_TID
 *			 	PROT_ERR_INVALID_LEN
 */
int32_t prot_dec_transport_layer(struct protocol *dec)
{
	uint8_t tid;
	uint8_t *src = (uint8_t *) dec->data.pData;

	tid = src[0];
	switch(tid){
	case 0:
		dec->tranLayer.tid = tid;
		dec->data.pData = (void *) &src[1];
		dec->data.dLen -= 1;
		break;
	case 1:
		dec->tranLayer.tid = tid;
		dec->tranLayer.opt.tid1.seqNr = src[1];
		dec->tranLayer.opt.tid1.winSize = src[2];
		dec->tranLayer.opt.tid1.flags = src[3];
		dec->data.pData = (void *) &src[4];
		dec->data.dLen -= 4;
		break;
	default:
		return PROT_ERR_INVALID_TID;
	}
	if(dec->data.dLen < 0)
		return PROT_ERR_INVALID_LEN;
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Decodes the process layer of a message according to the next gen protocol.
 *
 * Note: The argument, which is a protocol structure, has to hold a reference
 * to the messages process header within its pData field and further the
 * remaining length of the message within its dLen field. This is automatically
 * the case if prot_dec_transport_layer() has been used in advance. After
 * decoding, pData is updated to point to the service data and dLen is also
 * updated accordingly.
 *
 * Argument:	dec		Pointer to protocol structure.
 * Return:		PROT_SUCCESS
 *			 	PROT_ERR_INVALID_LEN
 */
int32_t prot_dec_process_layer(struct protocol *dec)
{
	uint8_t *src = (uint8_t *) dec->data.pData;

	dec->procLayer.sid = (src[0] << 8) + src[1];
	/* answers do have a service status field */
	if(dec->procLayer.sid & 0x01){
		dec->procLayer.sst = src[2];
		dec->data.pData = (void *) &src[3];
		dec->data.dLen -= 3;
	}else{
		dec->data.pData = (void *) &src[2];
		dec->data.dLen -= 2;
	}
	if(dec->data.dLen < 0)
		return PROT_ERR_INVALID_LEN;
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Decodes a message.
 * Decoding is done layer by layer starting with the outmost layer. After
 * each layer, the error variable is checked and the function is left if
 * an error occurred cleaning the source buffer first.
 * Decoding layer by layer also allows to do routing. If routing is necessary
 * is decided after decoding the network layer by evaluating the NID (NID = 2).
 *
 * After calling this function, the caller has three major results to
 * distinguish:
 * 		- Message to call service handler (err == 0)
 * 		- Message that shall be routed (err == 0xff)
 * 		- Erroneous message, reply with error message (err != 0 & err != 0xff)
 *
 * Argument:	src		Pointer to ucBuffer, typically from RX object.
 * 				dest	Pointer to protocol structure.
 * Return:		0		success
 * 				0xff	routed message
 * 				...		see the protocol.h error enumeration.
 */
int32_t prot_decode(struct ucBuffer *src, struct protocol *dest)
{
	int32_t err;

	dest->data.dLen = src->len;
	dest->data.pData = (void *) src->buf;
	err = prot_dec_network_layer(dest);
	if(err)
		goto err_free_src_buffer;
	if(dest->netLayer.nid == 2){
		if(dest->netLayer.opt.nid2.destDev != DEV_ADDR_GEN)
			return 0xff;
	}
	err = prot_dec_transport_layer(dest);
	if(err)
		goto err_free_src_buffer;
	err = prot_dec_process_layer(dest);
	if(err)
		goto err_free_src_buffer;
	return 0;
err_free_src_buffer:
	ucBuffer_clear(src);
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Function that calls the device service handle function based on the SID.
 * This function is usually called straight after decode, where src is the
 * result (dest) from decode. The dest argument of this function is typically
 * an object for which the message is supposed. The f argument is a filter,
 * protecting the void *dest from being misused. E.g. if a lamp sends a battery
 * service, the battery service may not be called, since dest is most likely
 * a lampObj (crash...).
 * Note that the device_service_handle function can be called directly without
 * using this wrapper.
 *
 * Argument:	src		The source data. Use decode() in advance to move the
 * 						data from the buffer into the protocol struct.
 * 				dest	The destination object the message is directed at.
 * 						Depending on the service it is interpreted differently,
 * 						hence it is a rather dangerous argument, being not type
 * 						safe!
 * 				f		A filter to avoid misuse of dest.
 * Return:		err		 0	success
 * 						... see the protocol.h error enum.
 */
int32_t prot_service_handle(struct protocol *src, void *dest,
								struct devServList *f)
{
	int32_t err = 0;
	int32_t i;
	uint16_t device;

	device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;
	if(device != SID_DEV_GENERAL && f != NULL){
		err = PROT_ERR_SERV_ACCESS_DENIED;
		for(i=0; i<f->nDevice; i++){
			if(device == f->device[i]){
				err = 0;
				break;
			}
		}
		if(err)
			return err;
	}
	switch(device){
	case SID_DEV_GEN:
		err = generator_service_handle(src, dest);
		break;
    case SID_DEV_GENERAL:
	case SID_DEV_BAT:
	case SID_DEV_ACDC:
	case SID_DEV_CC:
    case SID_DEV_LAMP:
		break;
	default:
		err = PROT_ERR_INVALID_SID;
		break;
	}
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * This function encodes a message according to the next gen protocol.
 * The source is a protocol struct. The destination is the buffer of a TX
 * object.
 * Note that the very first layer, namely the data link layer,
 * is not included in the struct and therefore also not in the encoded
 * message on the buffer. This layer is added when sending the message
 * with the uartTxObj.
 * For maximal efficiency, each encoding step only adds what it
 *
 * Argument:	src		The source data. A protocol structure.
 * 				dest	TX buffer, the destination of the source data.
 * Return:		err		 0	success
 * 						... see the protocol.h error enum.
 */
int32_t prot_encode(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = 0;

	ucBuffer_clear(dest);
	err = enc_process_layer(src, dest);
	if(err != PROT_SUCCESS)
		ucBuffer_clear(dest);
	return err;
}
/*---------------------------------------------------------------------------*/
/*Commented from MV*/
/*
 * This function fills the default error message into a protocol structure.
 * The default error message shall be used to reply with, whenever an error
 * during message decoding occurs. In this case it is not possible to increment
 * the SID and reply with the according error in the SST field as the SID is
 * invalid too!
 *
 * Argument:	src		Pointer to protocol structure
 * 				err		Error that is filled into SST field.
 * Return:		 0		success
 * 				-1 		Protocol structure must not be a NULL pointer.
 */
/*
int32_t prot_fillin_error_message(struct protocol *src, uint8_t err)
{
	if(src == NULL)
		return -1;
	src->netLayer.nid = 0;
	src->tranLayer.tid = 0;
	src->procLayer.sid = PROT_SID(SID_DEV_GENERAL,
			SID_SERV_GENERAL_ERROR,
			SID_ANS);
	src->procLayer.sst = err;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/
/*Commented from MV*/
/*
 * This function fills the routing approve message into a protocol structure.
 *
 * Argument:	src		Pointer to protocol structure
 * 				err		Error that is filled into SST field.
 * Return:		 0		success
 * 				-1 		Protocol structure must not be a NULL pointer.
 */
/*
int32_t prot_fillin_route_approve_message(struct protocol *src, uint8_t err)
{
	if(src == NULL)
		return -1;
	src->netLayer.nid = 0;
	src->tranLayer.tid = 0;
	src->procLayer.sid = PROT_SID(SID_DEV_GENERAL,
			SID_SERV_GENERAL_ROUTE_APPROVE,
			SID_ANS);
	src->procLayer.sst = err;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

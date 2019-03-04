/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: general.c
 * Date			: 30.11.2017
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

#include "lib/prot/services/general.h"
#include "lib/stm/aok.h"
#include "user/ps/psObj.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static int32_t handle_reply_get_device(struct protocol *, void *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Argument:	src		pointer to protocol structure, where received, decoded
 * 						data is stored.
 * 				dest	assumed to be pointer to any AO.
 */
int32_t handle_reply_get_device(struct protocol *src, void *dest)
{
	uint8_t *pData;
	uint16_t device;

	if(src->data.dLen < 1)  // 2
		return SERVICE_ERR_INVALID_DATA_LEN;
	pData = (uint8_t *) src->data.pData;
	device = ((pData[0] << 8 | pData[1]) & SID_DEV_M) >> SID_DEV_S;
	switch(((struct ao *) dest)->objType){
	case OBJTYPE_PS_OBJ:
		switch(device){
		case SID_DEV_BAT:
			((struct psObj *) dest)->psType = PS_BATTERY;
			break;
		case SID_DEV_ACDC:
			((struct psObj *) dest)->psType = PS_ACDC;
			break;
		default:
			((struct psObj *) dest)->psType = PS_INVALID_TYPE;
		}
		break;
	default:
		return -20;//todo
	}
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * Argument:	src		Protocol structure holding all information to encrypt
 * 						a message onto the destination TX buffer.
 * 				dest	The destination TX buffer.
 * Return:		err		 0	success
 * 						... see the protocol.h error enum.
 */
int32_t general_service_pack(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_GENERAL)
		return 30;  /* TODO assert() */
	if(reply){
		switch(serv){
		case SID_SERV_GENERAL_GET_DEVICE:
			break;
		case SID_SERV_GENERAL_ERROR:
			break;
		case SID_SERV_GENERAL_ROUTE_APPROVE:
			break;
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
		case SID_SERV_GENERAL_GET_DEVICE:
			break;
		case SID_SERV_GENERAL_ERROR:
			break;
		case SID_SERV_GENERAL_ROUTE_APPROVE:
			break;
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Argument:	src		The source data. Use decode() in advance to move the
 * 						data from the buffer into the protocol struct.
 * 				dest	The destination object the message is directed at.
 * 						Depending on the service it is interpreted differently,
 * 						hence it is a rather dangerous argument, being not type
 * 						safe!
 * Return:		err		 0	success
 * 						... see the protocol.h error enum.
 */
int32_t general_service_handle(struct protocol *src, void *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_GENERAL)
		return 30;  /* TODO assert() */
	if(reply){
		switch(serv){
		case SID_SERV_GENERAL_GET_DEVICE:
			err = handle_reply_get_device(src, dest);
			break;
		case SID_SERV_GENERAL_ERROR:
			break;
		case SID_SERV_GENERAL_ROUTE_APPROVE:
			break;
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
		case SID_SERV_GENERAL_GET_DEVICE:
			break;
		case SID_SERV_GENERAL_ERROR:
			break;
		case SID_SERV_GENERAL_ROUTE_APPROVE:
			break;
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}
	return err;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

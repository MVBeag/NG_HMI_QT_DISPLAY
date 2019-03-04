/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: capCharger.c
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

#include "lib/prot/services/capCharger.h"
#include "user/cc/ccTask.h"
#include "user/cc/ccObj.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static int32_t handle_reply_get_device_info(struct protocol *, struct cc *);
static int32_t handle_reply_get_specific_value(struct protocol *, struct cc *);
static int32_t handle_reply_get_live_data(struct protocol *, struct cc *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
int32_t handle_reply_get_device_info(struct protocol *src, struct cc *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 0)
        return SERVICE_ERR_INVALID_DATA_LEN;
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_specific_value(struct protocol *src, struct cc *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 0)
        return SERVICE_ERR_INVALID_DATA_LEN;
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_live_data(struct protocol *src, struct cc *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 0)
        return SERVICE_ERR_INVALID_DATA_LEN;
//    dest->data.liveData.volt = RETRIEVE16(&data[0]);
    ccObj_calib_ub(dest);
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
int32_t cc_service_pack(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t ans = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_CC)
		return 30;  /* TODO assert() */
	if(ans){
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
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
int32_t cc_service_handle(struct protocol *src, void *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t ans = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_CC)
		return 30;  /* TODO assert() */
	if(ans){
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}
	return err;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

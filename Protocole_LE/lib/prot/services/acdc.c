/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: acdcServices.c
 * Date			: 07.09.2017
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

#include "lib/prot/services/acdc.h"
#include "lib/prot/protocol.h"
#include "lib/stm/aok.h"
#include "user/ps/psObj.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static int32_t handle_reply_get_device_info(struct protocol *, struct psObj *);
static int32_t handle_reply_get_specific_value(struct protocol *, struct psObj *);
static int32_t handle_reply_get_live_data(struct protocol *, struct psObj *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/***********/
/*
 *
 */
int32_t handle_reply_get_device_info(struct protocol *src, struct psObj *dest)
{
//    uint8_t *data = (uint8_t *) src->data.pData;
//
//    if(src->data.dLen != 15)
//        return SERVICE_ERR_INVALID_DATA_LEN;
//    dest->ps.bat.info.serNr = RETRIEVE32(&data[0]);
//    dest->ps.bat.info.fwVersion = RETRIEVE32(&data[4]);
//    dest->ps.bat.info.hwVersion = RETRIEVE32(&data[8]);
//    dest->ps.bat.info.year = RETRIEVE32(&data[12]);
//    dest->ps.bat.info.month = RETRIEVE32(&data[13]);
//    dest->ps.bat.info.day = RETRIEVE32(&data[14]);
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_specific_value(struct protocol *src, struct psObj *dest)
{
//    uint8_t *data = (uint8_t *) src->data.pData;

//    if(src->data.dLen != 18)
//        return SERVICE_ERR_INVALID_DATA_LEN;
//    dest->ps.bat.specVal.designCap = RETRIEVE16(&data[0]);
//    dest->ps.bat.specVal.fullCap = RETRIEVE16(&data[2]);
//    dest->ps.bat.specVal.vNom = RETRIEVE16(&data[4]);
//    dest->ps.bat.specVal.eNom = RETRIEVE16(&data[6]);
//    dest->ps.bat.specVal.iDiscNom = RETRIEVE16(&data[8]);
//    dest->ps.bat.specVal.iDiscMax = RETRIEVE16(&data[10]);
//    dest->ps.bat.specVal.iChgNom = RETRIEVE16(&data[12]);
//    dest->ps.bat.specVal.tempDiscTrhL = RETRIEVE8(&data[14]);
//    dest->ps.bat.specVal.tempDiscTrhH = RETRIEVE8(&data[15]);
//    dest->ps.bat.specVal.tempChgTrhL = RETRIEVE8(&data[16]);
//    dest->ps.bat.specVal.tempChgTrhH = RETRIEVE8(&data[17]);
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_live_data(struct protocol *src, struct psObj *dest)
{
//    uint8_t *data = (uint8_t *) src->data.pData;
    struct acdc *acdc = &dest->data.acdc;
//
//    if(src->data.dLen != 15)
//        return SERVICE_ERR_INVALID_DATA_LEN;
//    chgs = bat->liveData.chgs;
//    bat->liveData.relRemainCap = RETRIEVE8(&data[0]);
//    bat->liveData.vBatPwr = RETRIEVE16(&data[1]);
//    bat->liveData.vPack = RETRIEVE16(&data[3]);
//    bat->liveData.vCharge = RETRIEVE16(&data[5]);
//    bat->liveData.iBat = RETRIEVE16(&data[7]);
//    bat->liveData.tCell = RETRIEVE16(&data[9]);
//    bat->liveData.tFet = RETRIEVE16(&data[11]);
//    bat->liveData.outs = RETRIEVE8(&data[13]);
//    bat->liveData.chgs = RETRIEVE8(&data[14]);
    /* lower the power according to temperature. Above 65°C power is
     * lowered by: 100*10/512 = 2 %/°C */
    if(acdc->liveData.temp > ACDC_DEGRADE_MAX_TEMP){
        acdc->calcVal.wNom = acdc->specVal.wNom
                - (((int32_t) (acdc->liveData.temp - ACDC_DEGRADE_MAX_TEMP)
                        * acdc->specVal.wNom) * ACDC_DEGRADE_PER_DEGREE) >> 10;
    }else{
        acdc->calcVal.wNom = acdc->specVal.wNom;
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
int32_t acdc_service_pack(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t ans = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_ACDC)
		return 30;  /* TODO assert() */
	if(ans){
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
		case SID_SERV_ACDC_GET_DEVICE_INFO:
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
int32_t acdc_service_handle(struct protocol *src, void *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_ACDC)
		return 30;  /* TODO assert() */
	if(reply){
        switch(serv){
        case SID_SERV_ACDC_GET_DEVICE_INFO:
            err = handle_reply_get_device_info(src, dest);
            break;
        case SID_SERV_ACDC_GET_SPECIFIC_VALUE:
            err = handle_reply_get_specific_value(src, dest);
            break;
        case SID_SERV_ACDC_GET_LIVE_DATA:
            err = handle_reply_get_live_data(src, dest);
            break;
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

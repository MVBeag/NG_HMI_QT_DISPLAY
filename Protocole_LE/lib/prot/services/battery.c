/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: batteryServices.c
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

#include "lib/prot/services/battery.h"
#include "lib/prot/protocol.h"
#include "lib/stm/aok.h"
#include "user/ps/psTask.h"
#include "user/ps/psObj.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static int32_t pack_req_clear_error_history(struct protocol *, struct ucBuffer *);
static int32_t pack_req_set_charging(struct protocol *, struct ucBuffer *);
static int32_t pack_req_set_user_register(struct protocol *, struct ucBuffer *);
static int32_t pack_req_set_bat_capacity(struct protocol *, struct ucBuffer *);

static int32_t handle_reply_get_device_info(struct protocol *, struct psObj *);
static int32_t handle_reply_get_specific_value(struct protocol *, struct psObj *);
static int32_t handle_reply_get_live_data(struct protocol *, struct psObj *);
static int32_t handle_reply_get_usage_info(struct protocol *, struct psObj *);
static int32_t handle_reply_get_bat_registers(struct protocol *, struct psObj *);
static int32_t handle_reply_clear_error_history(struct protocol *, struct psObj *);
static int32_t handle_reply_set_charging(struct protocol *, struct psObj *);
static int32_t handle_reply_set_user_register(struct protocol *, struct psObj *);
static int32_t handle_reply_set_bat_capacity(struct protocol *, struct psObj *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * not implemented
 */
static int32_t pack_req_clear_error_history(struct protocol *src, struct ucBuffer *dest)
{
//    STORE8(dest, src->data.arr[0]);
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
static int32_t pack_req_set_charging(struct protocol *src, struct ucBuffer *dest)
{
    struct psObj *ps = src->data.obj;

    STORE8(dest, ps->data.bat.setter.chg);  /* charge start(1)/stop(0) */
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * not implemented (an example on how to; just add setter.usrReg.idx and
 * setter.usrReg.data to battery structure!)
 */
static int32_t pack_req_set_user_register(struct protocol *src, struct ucBuffer *dest)
{
//    struct psObj *ps = src->data.obj;

//    if(src->data.set){
//        STORE8(dest, ps->data.bat.setter.usrReg.idx);  /* index */
//        STORE32(dest, ps->data.bat.setter.usrReg.data);  /* data */
//    }
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * not implemented
 */
static int32_t pack_req_set_bat_capacity(struct protocol *src, struct ucBuffer *dest)
{
//    if(src->data.arr[0]){
//        STORE8(dest, src->data.arr[1]);  /* index */
//        STORE16(dest, src->data.arr[4]);  /* capacity */
//    }
    return 0;
}
/*---------------------------------------------------------------------------*/

/***********/
/*
 *
 */
int32_t handle_reply_get_device_info(struct protocol *src, struct psObj *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

	if(src->data.dLen != 15)
		return SERVICE_ERR_INVALID_DATA_LEN;
	dest->data.bat.info.serNr = RETRIEVE32(&data[0]);
    dest->data.bat.info.fwVersion = RETRIEVE32(&data[4]);
    dest->data.bat.info.hwVersion = RETRIEVE32(&data[8]);
    dest->data.bat.info.year = RETRIEVE32(&data[12]);
    dest->data.bat.info.month = RETRIEVE32(&data[13]);
    dest->data.bat.info.day = RETRIEVE32(&data[14]);
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_specific_value(struct protocol *src, struct psObj *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

	if(src->data.dLen != 18)
		return SERVICE_ERR_INVALID_DATA_LEN;
    dest->data.bat.specVal.designCap = RETRIEVE16(&data[0]);
    dest->data.bat.specVal.fullCap = RETRIEVE16(&data[2]);
    dest->data.bat.specVal.vNom = RETRIEVE16(&data[4]);
    dest->data.bat.specVal.eNom = RETRIEVE16(&data[6]);
    dest->data.bat.specVal.iDiscNom = RETRIEVE16(&data[8]);
    dest->data.bat.specVal.iDiscMax = RETRIEVE16(&data[10]);
    dest->data.bat.specVal.iChgNom = RETRIEVE16(&data[12]);
    dest->data.bat.specVal.tempDiscTrhL = RETRIEVE8(&data[14]);
    dest->data.bat.specVal.tempDiscTrhH = RETRIEVE8(&data[15]);
    dest->data.bat.specVal.tempChgTrhL = RETRIEVE8(&data[16]);
    dest->data.bat.specVal.tempChgTrhH = RETRIEVE8(&data[17]);
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_get_live_data(struct protocol *src, struct psObj *dest)
{
    uint8_t chgs;
    struct event eTmp;
    struct battery *bat = &dest->data.bat;
    uint8_t *data = (uint8_t *) src->data.pData;

	if(src->data.dLen != 19)
		return SERVICE_ERR_INVALID_DATA_LEN;
	chgs = bat->liveData.chgs;
    bat->liveData.relRemainCap = RETRIEVE8(&data[0]);
    bat->liveData.vBatPwr = RETRIEVE16(&data[1]);
    bat->liveData.vPack = RETRIEVE16(&data[3]);
    bat->liveData.vCharge = RETRIEVE16(&data[5]);
    bat->liveData.iBat = RETRIEVE16(&data[7]);
    bat->liveData.tCell = RETRIEVE16(&data[9]);
    bat->liveData.tFet = RETRIEVE16(&data[11]);
    bat->liveData.outs = RETRIEVE8(&data[13]);
    bat->liveData.outErr = RETRIEVE16(&data[14]);
    bat->liveData.chgs = RETRIEVE8(&data[16]);
    bat->liveData.chgErr = RETRIEVE16(&data[17]);
    /* tell when the charger stopped/started */
    if((chgs & CHGS_CHARGE_ENABLE_FLAG) !=
            (bat->liveData.chgs & CHGS_CHARGE_ENABLE_FLAG)){
        if(bat->liveData.chgs & CHGS_CHARGE_ENABLE_FLAG)
            eTmp.data = 1;
        else
            eTmp.data = 0;
        eTmp.sig = PS_CHARGER_PWR_CHANGED_SIG;
        ao_post(psTaskAo, &eTmp);
    }
    /* calculate unloaded battery pack voltage */
    bat->calcVal.vPackNotLoaded = bat->liveData.vPack
            + (((int32_t) abs(bat->liveData.iBat)*R_BAT)/100);
    /* lower the maximal current depending on temperature */
    if(bat->liveData.tCell > BAT_DEGRADE_MAX_TEMP){
        bat->calcVal.iDiscMax = bat->specVal.iDiscMax
                - (((int32_t) (bat->liveData.tCell - BAT_DEGRADE_MAX_TEMP)
                        * bat->specVal.iDiscMax) * BAT_DEGRADE_PER_DEGREE) >> 10;
    }else{
        bat->calcVal.iDiscMax = bat->specVal.iDiscMax;
    }
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Not implemented! What for...
 */
int32_t handle_reply_get_usage_info(struct protocol *src, struct psObj *dest)
{
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Not implemented! What for...
 */
int32_t handle_reply_get_bat_registers(struct protocol *src, struct psObj *dest)
{
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Not implemented! What for...
 */
int32_t handle_reply_clear_error_history(struct protocol *src, struct psObj *dest)
{
    return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_set_charging(struct protocol *src, struct psObj *dest)
{
    uint8_t chgs;
    struct event eTmp;
    struct battery *bat = &dest->data.bat;
    uint8_t *data = (uint8_t *) src->data.pData;

	if(src->data.dLen != 3)
		return SERVICE_ERR_INVALID_DATA_LEN;
    chgs = bat->liveData.chgs;
	bat->liveData.chgs = RETRIEVE8(&data[0]);
    bat->liveData.chgErr = RETRIEVE16(&data[1]);
    /* tell when the charger stopped/started */
    if((chgs & CHGS_CHARGE_ENABLE_FLAG) !=
            (bat->liveData.chgs & CHGS_CHARGE_ENABLE_FLAG)){
        if(bat->liveData.chgs & CHGS_CHARGE_ENABLE_FLAG)
            eTmp.data = 1;
        else
            eTmp.data = 0;
        eTmp.sig = PS_CHARGER_PWR_CHANGED_SIG;
        ao_post(psTaskAo, &eTmp);
    }
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * User registers are currently not used!
 */
int32_t handle_reply_set_user_register(struct protocol *src, struct psObj *dest)
{
	return PROT_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_reply_set_bat_capacity(struct protocol *src, struct psObj *dest)
{
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 6)
        return SERVICE_ERR_INVALID_DATA_LEN;
    dest->data.bat.specVal.designCap = RETRIEVE16(&data[0]);
    dest->data.bat.specVal.fullCap = RETRIEVE16(&data[2]);
    dest->data.bat.liveData.relRemainCap = RETRIEVE16(&data[4]);
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
int32_t battery_service_pack(struct protocol *src, struct ucBuffer *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_BAT)
		return 30;  /* TODO assert() */
	if(reply){  /* pack reply */
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{  /* pack request */
		switch(serv){
		case SID_SERV_BAT_GET_DEVICE_INFO:
		case SID_SERV_BAT_GET_SPECIFIC_VALUE:
		case SID_SERV_BAT_GET_LIVE_DATA:
		case SID_SERV_BAT_GET_USAGE_INFO:
		case SID_SERV_BAT_GET_BAT_REGISTERS:
		case SID_SERV_BAT_UPDATE:
			break;
		case SID_SERV_BAT_CLEAR_ERROR_HISTORY:
			err = pack_req_clear_error_history(src, dest);
			break;
		case SID_SERV_BAT_SET_CHARGING:
			err = pack_req_set_charging(src, dest);
			break;
		case SID_SERV_BAT_SET_USER_REGISTER:
			err = pack_req_set_user_register(src, dest);
			break;
		case SID_SERV_BAT_SET_BAT_CAPACITY:
		    err = pack_req_set_bat_capacity(src, dest);
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
int32_t battery_service_handle(struct protocol *src, void *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_BAT)
		return 30;  /* TODO assert() */

	/* TODO test purpose only */
	if(src->procLayer.sst != 0)
		while(1);
	/* -- end test -- */

	if(reply){
		switch(serv){
		case SID_SERV_BAT_GET_DEVICE_INFO:
			err = handle_reply_get_device_info(src, dest);
			break;
		case SID_SERV_BAT_GET_SPECIFIC_VALUE:
			err = handle_reply_get_specific_value(src, dest);
			break;
		case SID_SERV_BAT_GET_LIVE_DATA:
			err = handle_reply_get_live_data(src, dest);
			break;
		case SID_SERV_BAT_GET_USAGE_INFO:
			err = handle_reply_get_usage_info(src, dest);
			break;
		case SID_SERV_BAT_GET_BAT_REGISTERS:
			err = handle_reply_get_bat_registers(src, dest);
			break;
		case SID_SERV_BAT_CLEAR_ERROR_HISTORY:
			err = handle_reply_clear_error_history(src, dest);
			break;
		case SID_SERV_BAT_SET_CHARGING:
			err = handle_reply_set_charging(src, dest);
			break;
		case SID_SERV_BAT_SET_USER_REGISTER:
			err = handle_reply_set_user_register(src, dest);
			break;
		case SID_SERV_BAT_SET_BAT_CAPACITY:
			err = handle_reply_set_bat_capacity(src, dest);
			break;
		case SID_SERV_BAT_UPDATE:
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

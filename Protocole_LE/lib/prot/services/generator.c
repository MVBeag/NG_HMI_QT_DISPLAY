/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: generator.c
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
#include <string.h>

#include "Protocole_LE/lib/prot/services/generator.h"
/*Commented from MV*/
/*
#include "user/co/coTask.h"
#include "user/co/coObj.h"
#include "user/sys/systemTask.h"
#include "lib/stm/aok.h"
#include "lib/stm/eventPool.h"

#include "user/math/interpolation.h"
*/
/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/

/* Commented from MV */
/*
static int32_t pack_reply_get_device_info(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_get_specific_value(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_get_live_data(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_get_usage_info(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_flash_channel_state(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_flash_sequencer_step(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_flash_sequencer_step_nr(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_intercept_flash(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_charger_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_ps_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_flash_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_lamp_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_trigger_source(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_trigger_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_rst_trigger_settings(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_sw_update(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_sleep(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_shut_down(struct protocol *, struct ucBuffer *);
*/
static int32_t handle_req_set_flash_channel_state(struct protocol *, void *);
static int32_t handle_req_set_flash_sequencer_step(struct protocol *, void *);
static int32_t handle_req_set_flash_sequencer_step_nr(struct protocol *, void *);
static int32_t handle_req_set_charger_settings(struct protocol *, void *);
static int32_t handle_req_set_ps_settings(struct protocol *, void *);
static int32_t handle_req_set_flash_settings(struct protocol *, void *);
static int32_t handle_req_set_lamp_settings(struct protocol *, void *);
static int32_t handle_req_set_trigger_source(struct protocol *, void *);
static int32_t handle_req_set_trigger_settings(struct protocol *, void *);
static int32_t handle_req_rst_trigger_settings(struct protocol *, void *);
static int32_t handle_req_sw_update(struct protocol *, void *);
static int32_t handle_req_sleep(struct protocol *, void *);
static int32_t handle_req_shut_down(struct protocol *, void *);

/* debug */
static int32_t pack_reply_set_final_flash_mode(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_set_model_param(struct protocol *, struct ucBuffer *);
static int32_t pack_reply_func_test(struct protocol *, struct ucBuffer *);

static int32_t handle_req_set_final_flash_mode(struct protocol *, void *);
static int32_t handle_req_set_model_param(struct protocol *, void *);
static int32_t handle_req_func_test(struct protocol *, void *);


/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_get_device_info(struct protocol *src, struct ucBuffer *dest)
{
    struct deviceInfo *info;

    //sysTask_get_device_info(info);
	dest->buf[dest->pos++] = info->serNr;
	dest->buf[dest->pos++] = info->prodDate;
    dest->buf[dest->pos++] = info->servDate;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_get_specific_value(struct protocol *src, struct ucBuffer *dest)
{
	struct hwInfo hwinfo;

	sysTask_get_hw_config(&hwinfo);
	coTask_get_number_of_channels();
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_get_live_data(struct protocol *src, struct ucBuffer *dest)
{
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_get_usage_info(struct protocol *src, struct ucBuffer *dest)
{
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_flash_channel_state(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    int32_t i;
    uint8_t nChn;
    uint8_t chn;
    uint8_t state;

    chn = src->data.arr[0];
    STORE8(dest, chn);
    if(chn == 0xff){
        nChn = coTask_get_number_of_channels();
        for(i=0; i<nChn; i++){
            err = coTask_get_flash_channel_state(i, &state);
            STORE8(dest, state);
        }
    }else{
        err = coTask_get_flash_channel_state(chn, &state);
        STORE8(dest, state);
    }
    return err;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_flash_sequencer_step(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    uint8_t chn;
    uint8_t step;
    uint32_t pool[14];

    chn = src->data.arr[0];
    step = src->data.arr[1];
    err = coTask_get_flash_sequencer_step(chn, step, pool);
    STORE8(dest, chn);
    STORE8(dest, step);
    STORE8(dest, pool[0]);
    STORE8(dest, pool[1]);
    STORE8(dest, pool[2]);
    STORE8(dest, pool[3]);
    STORE8(dest, pool[4]);
    STORE8(dest, pool[5]);
    STORE16(dest, pool[6]);
    STORE16(dest, pool[7]);
    STORE32(dest, pool[8]);
    STORE32(dest, pool[9]);
    STORE8(dest, pool[10]);
    STORE8(dest, pool[11]);
    STORE8(dest, pool[12]);
    STORE8(dest, pool[13]);
    return err;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_flash_sequencer_step_nr(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    int32_t i;
    uint8_t nChn;
    uint8_t chn;
    uint8_t step;

    chn = src->data.arr[0];
    STORE8(dest, chn);
    if(chn == 0xff){
        nChn = coTask_get_number_of_channels();
        for(i=0; i<nChn; i++){
            err = coTask_get_flash_sequencer_step_nr(i, &step);
            STORE8(dest, step);
        }
    }else{
        err = coTask_get_flash_sequencer_step_nr(chn, &step);
        STORE8(dest, step);
    }
    return err;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_intercept_flash(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    int32_t i;
    uint8_t nChn;
    uint8_t chn;
    uint8_t state;
    uint32_t delay;

    chn = src->data.arr[0];
    err = coTask_get_flash_state(chn, &state);
    STORE8(dest, chn);
    STORE8(dest, state);
    if(chn == 0xff){
        nChn = coTask_get_number_of_channels();
        for(i=0; i<nChn; i++){
            err = coTask_get_active_delay(i, &delay);
            STORE32(dest, delay);
        }
    }else{
        err = coTask_get_active_delay(chn, &delay);
        STORE32(dest, delay);
    }
    return err;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_charger_settings(struct protocol *src, struct ucBuffer *dest)
{
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_ps_settings(struct protocol *src, struct ucBuffer *dest)
{
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_flash_settings(struct protocol *src, struct ucBuffer *dest)
{
	dest->buf[dest->pos++] = 0;
	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_lamp_settings(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_trigger_source(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_set_trigger_settings(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_rst_trigger_settings(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_sw_update(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_sleep(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t pack_reply_shut_down(struct protocol *src, struct ucBuffer *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/******/
/*
 *
 */
/* Commented from MV */
/*
int32_t handle_req_set_flash_channel_state(struct protocol *src, void *dest)
{
    int32_t err;
    int32_t i;
    uint8_t nChn;
    uint8_t chn;
    uint8_t state;
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen == 1){
        chn = data[0];
        src->data.arr[0] = chn;
        nChn = coTask_get_number_of_channels();
        if(chn >= nChn && chn != 0xff)
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    }else if(src->data.dLen == 2){
        chn = data[0];
        state = data[1];
        src->data.arr[0] = chn;
        if(chn == 0xff){
            nChn = coTask_get_number_of_channels();
            for(i=0; i<nChn; i++){
                err = coTask_set_flash_channel_state(i, state);
                if(err)
                    return SERVICE_ERR_PARAM_OUT_OF_RANGE;
            }
        }else{
            err = coTask_set_flash_channel_state(chn, state);
            if(err)
                return SERVICE_ERR_PARAM_OUT_OF_RANGE;
        }
    }else{
        return SERVICE_ERR_INVALID_DATA_LEN;
    }
    return 0;
}
*/
/*---------------------------------------------------------------------------*/

 /*
  *
  */
/* Commented from MV */
#ifdef MV
int32_t handle_req_set_flash_sequencer_step(struct protocol *src, void *dest)
{
    uint8_t chn, step;
    int32_t err;
    uint32_t pool[11];
    int32_t i = 0;
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen == 2){
        chn = data[i++];
        step = data[i++];
        if(step == 0xff)
            coTask_get_flash_sequencer_step_nr(chn, &step);
        src->data.arr[0] = chn;
        src->data.arr[1] = step;
        if(chn >= coTask_get_number_of_channels())
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
        if(step >= coTask_get_sequencer_size())
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    }else if(src->data.dLen == 20){
        chn = data[i++];
        step = data[i++];  /* step number */
        if(step == 0xff)
            coTask_get_flash_sequencer_step_nr(chn, &step);
        pool[0] = data[i++];  /* step option */
        pool[1] = data[i++];  /* step remain */
        pool[2] = data[i++];  /* trigger source */
        pool[3] = data[i++];  /* flash mode */
        pool[4] = data[i++];  /* aperture */
        pool[5] = RETRIEVE16(&data[i]);  /* cct */
        i += 2;
        pool[6] = RETRIEVE32(&data[i]);  /* delay */
        i += 4;
        pool[7] = RETRIEVE32(&data[i]);  /* interval */
        i += 4;
        pool[8] = data[i++];  /* repetition */
        pool[9] = data[i++];  /* wait_state */
        pool[10] = data[i++];  /* wait_state_init */
        src->data.arr[0] = chn;
        src->data.arr[1] = step;
        err = coTask_set_flash_sequencer_step(chn, step, pool);
        if(err)
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    }else{
        return SERVICE_ERR_INVALID_DATA_LEN;
    }
    return 0;
}
#endif
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
#ifdef MV
int32_t handle_req_set_flash_sequencer_step_nr(struct protocol *src, void *dest)
{
    int32_t err;
    int32_t i;
    uint8_t nChn;
   uint8_t chn;
   uint8_t step;
   uint8_t *data = (uint8_t *) src->data.pData;

   if(src->data.dLen == 1){
       chn = data[0];
       src->data.arr[0] = chn;
       nChn = coTask_get_number_of_channels();
       if(chn >= nChn && chn != 0xff)
           return SERVICE_ERR_PARAM_OUT_OF_RANGE;
   }else if(src->data.dLen == 2){
       chn = data[0];
       step = data[1];
       src->data.arr[0] = chn;
       if(chn == 0xff){
           nChn = coTask_get_number_of_channels();
           for(i=0; i<nChn; i++){
               err = coTask_set_flash_sequencer_step_nr(i, step);
               if(err)
                   return SERVICE_ERR_PARAM_OUT_OF_RANGE;
           }
       }else{
           err = coTask_set_flash_sequencer_step_nr(chn, step);
           if(err)
               return SERVICE_ERR_PARAM_OUT_OF_RANGE;
       }
   }else{
       return SERVICE_ERR_INVALID_DATA_LEN;
   }
   return 0;
}
#endif
/*---------------------------------------------------------------------------*/

 /*
  *
  */
/* Commented from MV */
/*
int32_t handle_req_intercept_flash(struct protocol *src, void *dest)
{
    uint8_t chn;
    uint8_t code;
    int32_t err;
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 2)
        return SERVICE_ERR_INVALID_DATA_LEN;
    chn = data[0];
    code = data[1];
    src->data.arr[0] = chn;
    err = coTask_intercept_flash(chn, code);
    if(err)
        return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_test_flash(struct protocol *src, void *dest)
{
   /* MV: potention answer: flash done */
   uint8_t chn;
   int32_t err;
   uint8_t *data = (uint8_t *) src->data.pData;

   if(src->data.dLen != 1)
       return SERVICE_ERR_INVALID_DATA_LEN;
   chn = data[0];
   src->data.arr[0] = chn;
   err = coTask_test_flash(chn);
   if(err)
       return SERVICE_ERR_PARAM_OUT_OF_RANGE;
   return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t handle_req_set_charger_settings(struct protocol *src, void *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t handle_req_set_ps_settings(struct protocol *src, void *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
/* Commented from MV */
/*
int32_t handle_req_set_flash_settings(struct protocol *src, void *dest)
{

	return 0;
}
*/
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_set_lamp_settings(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_set_trigger_source(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_set_trigger_settings(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_rst_trigger_settings(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_sw_update(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_sleep(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_shut_down(struct protocol *src, void *dest)
{

	return 0;
}
/*---------------------------------------------------------------------------*/

/******** DEBUG ********/
/*
 *
 */
int32_t pack_reply_set_final_flash_mode(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    uint8_t chn;
    uint32_t pool[4];

    chn = src->data.arr[0];
    err = coTask_get_final_flash_mode(chn, pool);
    STORE8(dest, chn);
    STORE16(dest, pool[0]);
    STORE16(dest, pool[1]);
    STORE32(dest, pool[2]);
    STORE16(dest, pool[3]);
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t pack_reply_set_model_param(struct protocol *src, struct ucBuffer *dest)
{
    int32_t err;
    uint8_t chn;
    uint32_t pool[8];

    chn = src->data.arr[0];
    err = coTask_get_model_param(chn, pool);
    STORE8(dest, chn);
    STORE16(dest, pool[0]);
    STORE16(dest, pool[1]);
    STORE16(dest, pool[2]);
    STORE16(dest, pool[3]);
    STORE16(dest, pool[4]);
    STORE16(dest, pool[5]);
    STORE16(dest, pool[6]);
    STORE16(dest, pool[7]);
    return err;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t pack_reply_func_test(struct protocol *src, struct ucBuffer *dest)
{
    int32_t y;

    if(((uint32_t *) src->data.arr)[0] == 1)
        y = i2uk_linear_interpolation(((uint32_t *) src->data.arr)[1]);
    if(((uint32_t *) src->data.arr)[0] == 2)
        y = ubdc2i_cubic_interpolation(((uint32_t *) src->data.arr)[1]);
    if(((uint32_t *) src->data.arr)[0] == 3)
        y = i2isqrt_linear_interpolation(((uint32_t *) src->data.arr)[1]);
    STORE32(dest, y);
    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_set_final_flash_mode(struct protocol *src, void *dest)
{
    uint8_t chn;
    uint8_t nChn;
    uint32_t pool[4];
    int32_t err;
    int32_t i = 0;
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen == 1){
        chn = data[i++];
        src->data.arr[0] = chn;
        nChn = coTask_get_number_of_channels();
        if(chn >= nChn)
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    }else if(src->data.dLen == 11){
        chn = data[i++];
        pool[0] = RETRIEVE16(&data[i]);  /* tpre */
        i += 2;
        pool[1] = RETRIEVE16(&data[i]);  /* tramp */
        i += 2;
        pool[2] = RETRIEVE32(&data[i]);  /* tmain */
        i += 4;
        pool[3] = RETRIEVE16(&data[i]);  /* duty cycle */
        src->data.arr[0] = chn;
        err = coTask_set_final_flash_mode(chn, pool);
        if(err)
            return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    }else{
        return SERVICE_ERR_INVALID_DATA_LEN;
    }


    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
int32_t handle_req_set_model_param(struct protocol *src, void *dest)
{
    uint32_t chn;
    uint32_t pool[8];
    int32_t err;
    int32_t i = 0;
    uint8_t *data = (uint8_t *) src->data.pData;

    if(src->data.dLen != 17)
        return SERVICE_ERR_INVALID_DATA_LEN;
    chn = data[i++];
    pool[0] = RETRIEVE16(&data[i]);
    i += 2;
    pool[1] = RETRIEVE16(&data[i]);
    i += 2;
    pool[2] = RETRIEVE16(&data[i]);
    i += 2;
    pool[3] = RETRIEVE16(&data[i]);
    i += 2;
    pool[4] = RETRIEVE16(&data[i]);
    i += 2;
    pool[5] = RETRIEVE16(&data[i]);
    i += 2;
    pool[6] = RETRIEVE16(&data[i]);
    i += 2;
    pool[7] = RETRIEVE16(&data[i]);
    src->data.arr[0] = chn;
    err = coTask_set_model_param(chn, pool);
    if(err)
        return SERVICE_ERR_PARAM_OUT_OF_RANGE;
    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Two int32_t values are expected which are then stored in the protocol
 * structure.
 */
int32_t handle_req_func_test(struct protocol *src, void *dest)
{
    int32_t i = 0;
    uint8_t *data = (uint8_t *) src->data.pData;
    uint32_t *arr = (uint32_t *) src->data.arr;

    arr[0] = RETRIEVE32(&data[i]);
    i += 4;
    arr[1] = RETRIEVE32(&data[i]);
    return 0;
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
int32_t generator_service_pack(struct protocol *src, struct ucBuffer *dest)
{
    int32_t i;
    uint32_t tmp;
	struct usBuffer *buf;  /* TODO flash volt debug */
	int32_t err = PROT_SUCCESS;
    uint16_t reply = src->procLayer.sid & SID_ANS_M; /* Commented from MV: request from our side */
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_GEN)
		return 30;  /* TODO assert() */
    if(!reply){
		switch(serv){
		case SID_SERV_GEN_GET_DEVICE_INFO:
            /* Commented from MV: not implamented now: err = pack_request_get_device_info(src, dest); */
			break;
		case SID_SERV_GEN_GET_SPECIFIC_VALUE:
            /* Commented from MV: not implamented now: err = pack_reply_get_specific_value(src, dest); */
			break;
		case SID_SERV_GEN_GET_LIVE_DATA:
            /* Commented from MV: not implamented now: err = pack_reply_get_live_data(src, dest); */
			break;
		case SID_SERV_GEN_GET_USAGE_INFO:
            /* Commented from MV: not implamented now: err = pack_reply_get_usage_info(src, dest); */
			break;
        /* cutoff commands */
		case SID_SERV_GEN_SET_FLASH_CHANNEL_STATE:
            /* Commented from MV: not implamented now: err = pack_reply_set_flash_channel_state(src, dest); */
		    break;
		case SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP:
            /* Commented from MV: not implamented now: err = pack_reply_set_flash_sequencer_step(src, dest); */
            break;
		case SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP_NR:
            /* Commented from MV: not implamented now: err = pack_reply_set_flash_sequencer_step_nr(src, dest); */
            break;
		case SID_SERV_GEN_INTERCEPT_FLASH:
            /* Commented from MV: not implamented now: err = pack_reply_intercept_flash(src, dest); */
            break;
		case SID_SERV_GEN_TEST_FLASH:
		    dest->buf[dest->pos++] = src->data.arr[0];
            break;
        /* --- */
		case SID_SERV_GEN_SET_CHARGER_SETTINGS:
             /* Commented from MV: not implamented now: err = pack_reply_set_charger_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_PS_SETTINGS:
             /* Commented from MV: not implamented now: err = pack_reply_set_ps_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_FLASH_SETTINGS:
            /* Commented from MV: err = pack_reply_set_flash_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_LAMP_SETTINGS:
            /* Commented from MV: err = pack_reply_set_lamp_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_TRIGGER_SOURCE:
            /* Commented from MV: err = pack_reply_set_trigger_source(src, dest); */
			break;
		case SID_SERV_GEN_SET_TRIGGER_SETTINGS:
            /* Commented from MV: err = pack_reply_set_trigger_settings(src, dest); */
			break;
		case SID_SERV_GEN_RST_TRIGGER_SETTINGS:
            /* Commented from MV: err = pack_reply_rst_trigger_settings(src, dest); */
			break;
		case SID_SERV_GEN_SW_UPDATE:
            /* Commented from MV: err = pack_reply_sw_update(src, dest); */
			break;
		case SID_SERV_GEN_SLEEP:
            /* Commented from MV: err = pack_reply_sleep(src, dest); */
			break;
		case SID_SERV_GEN_SHUT_DOWN:
            /* Commented from MV: err = pack_reply_shut_down(src, dest); */
			break;
		/* debug & test */
		case SID_SERV_GEN_SET_HSS_MODE:
		case SID_SERV_GEN_SET_SINGLE_SHOT_MODE:
		case SID_SERV_GEN_SET_LITTLE_FLASH_MODE:
		case SID_SERV_GEN_SET_DUTY_CYCLE_TABLE:
		case SID_SERV_GEN_FLASH_DONE:
		case SID_SERV_GEN_SET_IGNITION_TIMING:
		case SID_SERV_GEN_SET_RAMP_FLASH_MODE:
		    break;
		case SID_SERV_GEN_SET_FINAL_FLASH_MODE:
		    err = pack_reply_set_final_flash_mode(src, dest);
			break;
        case SID_SERV_GEN_SET_MODEL_PARAM:
            err = pack_reply_set_model_param(src, dest);
            break;
        case SID_SERV_GEN_GET_FVOLT_DATA:
            /* Commented from MV:
            buf = get_fvolt_data_buffer();
            if(src->tranLayer.tid == 0){
                while((dest->pos < dest->size-4) && (buf->pos < buf->len)){
                    dest->buf[dest->pos++] = buf->buf[buf->pos] >> 8;
                    dest->buf[dest->pos++] = buf->buf[buf->pos];
                    buf->pos++;
                }
            }else if(src->tranLayer.tid == 1){
                src->tranLayer.opt.tid1.flags = 0x1;
                for(i=2; i<src->tranLayer.opt.tid1.winSize/2; i++){
                    if(buf->pos >= buf->len){
                        src->tranLayer.opt.tid1.flags = 0x0;
                        break;
                    }
                    if(dest->pos >= dest->size-4)
                        break;
                    dest->buf[dest->pos++] = buf->buf[buf->pos] >> 8;
                    dest->buf[dest->pos++] = buf->buf[buf->pos];
                    buf->pos++;
                }
            }
            break;
            */
        case SID_SERV_GEN_GET_FLASH_INFO:
            /* Commented from MV:
            tmp = get_flash_info();  // trigger counter
            STORE32(dest, tmp);
            */
            break;
        case SID_SERV_GEN_FUNC_TEST:
            err = pack_reply_func_test(src, dest);
            break;

		/* TODO bat_test */
		case SID_SERV_GEN_GET_BAT_TEST:
			break;
		case SID_SERV_GEN_SET_BAT_CHARGING:
			break;
		/* -- */

		default:
			err = PROT_ERR_INVALID_SID;
		}
    }else{  /* MV: request for you Dudy :-) */
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
int32_t generator_service_handle(struct protocol *src, void *dest)
{
	int32_t err = PROT_SUCCESS;
	uint16_t reply = src->procLayer.sid & SID_ANS_M;
	uint16_t serv = (src->procLayer.sid & SID_SERV_M) >> SID_SERV_S;
	uint16_t device = (src->procLayer.sid & SID_DEV_M) >> SID_DEV_S;

	if(device != SID_DEV_GEN)
		return PROT_ERR_SERV_ACCESS_DENIED;
	if(reply){
		switch(serv){
		default:
			err = PROT_ERR_INVALID_SID;
		}
	}else{
		switch(serv){
		case SID_SERV_GEN_GET_DEVICE_INFO:
		case SID_SERV_GEN_GET_SPECIFIC_VALUE:
		case SID_SERV_GEN_GET_LIVE_DATA:
		case SID_SERV_GEN_GET_USAGE_INFO:
			break;
		/* cutoff commands */
		case SID_SERV_GEN_SET_FLASH_CHANNEL_STATE:
            /* Commented from MV:err = handle_reply_set_flash_channel_state(src, dest); */
            break;
		case SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP:
            /* Commented from MV:err = handle_req_set_flash_sequencer_step(src, dest); */
            break;
		case SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP_NR:
            /* Commented from MV:err = handle_req_set_flash_sequencer_step_nr(src, dest); */
            break;
		case SID_SERV_GEN_INTERCEPT_FLASH:
            /* Commented from MV:err = handle_req_intercept_flash(src, dest); */
            break;
		case SID_SERV_GEN_TEST_FLASH:
            err = handle_req_test_flash(src, dest);
		    break;
		/* --- */
		case SID_SERV_GEN_SET_CHARGER_SETTINGS:
            /* Commented from MV:err = handle_req_set_charger_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_PS_SETTINGS:
            /* Commented from MV:err = handle_req_set_ps_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_FLASH_SETTINGS:
            /* Commented from MV:err = handle_req_set_flash_settings(src, dest); */
			break;
		case SID_SERV_GEN_SET_LAMP_SETTINGS:
			err = handle_req_set_lamp_settings(src, dest);
			break;
		case SID_SERV_GEN_SET_TRIGGER_SOURCE:
			err = handle_req_set_trigger_source(src, dest);
			break;
		case SID_SERV_GEN_SET_TRIGGER_SETTINGS:
			err = handle_req_set_trigger_settings(src, dest);
			break;
		case SID_SERV_GEN_RST_TRIGGER_SETTINGS:
			err = handle_req_rst_trigger_settings(src, dest);
			break;
		case SID_SERV_GEN_SW_UPDATE:
			err = handle_req_sw_update(src, dest);
			break;
		case SID_SERV_GEN_SLEEP:
			err = handle_req_sleep(src, dest);
			break;
		case SID_SERV_GEN_SHUT_DOWN:
			err = handle_req_shut_down(src, dest);
			break;
		/* debug */
		case SID_SERV_GEN_SET_HSS_MODE:
		case SID_SERV_GEN_SET_SINGLE_SHOT_MODE:
		case SID_SERV_GEN_SET_LITTLE_FLASH_MODE:
		case SID_SERV_GEN_SET_DUTY_CYCLE_TABLE:
		case SID_SERV_GEN_GET_FVOLT_DATA:
		case SID_SERV_GEN_SET_IGNITION_TIMING:
        case SID_SERV_GEN_GET_FLASH_INFO:
        case SID_SERV_GEN_SET_RAMP_FLASH_MODE:
            break;
        case SID_SERV_GEN_SET_FINAL_FLASH_MODE:
            //err = handle_req_set_final_flash_mode(src, dest);
            break;
        case SID_SERV_GEN_SET_MODEL_PARAM:
            //err = handle_req_set_model_param(src, dest);
            break;
        case SID_SERV_GEN_FUNC_TEST:
            //err = handle_req_func_test(src, dest);
            break;

		/* TODO bat_test */
		case SID_SERV_GEN_GET_BAT_TEST:
			src->data.arr[0] = ((uint8_t *) src->data.pData)[0];
			break;
		case SID_SERV_GEN_SET_BAT_CHARGING:
			// TODO
			break;
		/* -- */

		default:
			err = PROT_ERR_INVALID_SID;
		}
	}
	return err;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

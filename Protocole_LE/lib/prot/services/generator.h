/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: generator.h
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
 *****************************************************************************/

#ifndef SOURCE_LIB_PROT_SERVICES_GENERATOR_H_
#define SOURCE_LIB_PROT_SERVICES_GENERATOR_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "prot/protocol.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
enum generatorServSid{
	SID_SERV_GEN_GET_DEVICE_INFO,
	SID_SERV_GEN_GET_SPECIFIC_VALUE,
	SID_SERV_GEN_GET_LIVE_DATA,
	SID_SERV_GEN_GET_USAGE_INFO,
	/**/
	SID_SERV_GEN_SET_FLASH_CHANNEL_STATE = 0x08,
	SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP,
	SID_SERV_GEN_SET_FLASH_SEQUENCER_STEP_NR,
    SID_SERV_GEN_INTERCEPT_FLASH,
    SID_SERV_GEN_TEST_FLASH,

	SID_SERV_GEN_NONE,
	SID_SERV_GEN_SET_CHARGER_SETTINGS,
	SID_SERV_GEN_SET_PS_SETTINGS,
	SID_SERV_GEN_SET_FLASH_SETTINGS,
	SID_SERV_GEN_SET_LAMP_SETTINGS,
	SID_SERV_GEN_SET_TRIGGER_SOURCE,
	SID_SERV_GEN_SET_TRIGGER_SETTINGS,
	SID_SERV_GEN_RST_TRIGGER_SETTINGS,
	SID_SERV_GEN_SW_UPDATE,
	SID_SERV_GEN_SLEEP,
	SID_SERV_GEN_SHUT_DOWN,
	/**/
	SID_SERV_GEN_SET_HSS_MODE = 0x50,
	SID_SERV_GEN_SET_SINGLE_SHOT_MODE,
	SID_SERV_GEN_SET_LITTLE_FLASH_MODE,
	SID_SERV_GEN_SET_DUTY_CYCLE_TABLE,
	SID_SERV_GEN_GET_FVOLT_DATA,
    SID_SERV_GEN_FLASH_DONE,  /* push message over debug interface! */
    SID_SERV_GEN_SET_IGNITION_TIMING,  /* used for ignition tests */
    SID_SERV_GEN_GET_FLASH_INFO,  /* get number of flashes (volatile) */
    SID_SERV_GEN_SET_RAMP_FLASH_MODE,
    SID_SERV_GEN_SET_FINAL_FLASH_MODE,
    SID_SERV_GEN_SET_MODEL_PARAM,

	/* TODO bat_test */
	SID_SERV_GEN_GET_BAT_TEST = 0x60,
	SID_SERV_GEN_SET_BAT_CHARGING,

    /**/
    SID_SERV_GEN_FUNC_TEST = 0x70,
	/* -- */
};

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* The data struct to push onto the message queue. If other data than id is
 * needed, simply add your variable to the union. This way, there is
 * automatically enough memory allocated within the queue and the data can
 * be easily accessed. */
struct genServData{
	uint16_t sid;
	union{
		void *obj;  /* not used */
	}data;
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t generator_service_pack(struct protocol *, struct ucBuffer *);
extern int32_t generator_service_handle(struct protocol *, void *);


#endif /* SOURCE_LIB_PROT_SERVICES_GENERATOR_H_ */

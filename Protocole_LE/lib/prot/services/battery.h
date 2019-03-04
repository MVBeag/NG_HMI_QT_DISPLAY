/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: batteryServices.h
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
 *****************************************************************************/

#ifndef BATTERYSERVICES_H_
#define BATTERYSERVICES_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/prot/protocol.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
enum batServSid{
	SID_SERV_BAT_GET_DEVICE_INFO,
	SID_SERV_BAT_GET_SPECIFIC_VALUE,
	SID_SERV_BAT_GET_LIVE_DATA,
	SID_SERV_BAT_GET_USAGE_INFO,
	SID_SERV_BAT_GET_BAT_REGISTERS,
	SID_SERV_BAT_CLEAR_ERROR_HISTORY,
	SID_SERV_BAT_SET_CHARGING,
	SID_SERV_BAT_SET_USER_REGISTER,
	SID_SERV_BAT_SET_BAT_CAPACITY,
	SID_SERV_BAT_UPDATE,
};

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t battery_service_pack(struct protocol *, struct ucBuffer *);
extern int32_t battery_service_handle(struct protocol *, void *);

#endif /* BATTERYSERVICES_H_ */

/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: acdcServices.h
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

#ifndef SOURCE_USER_PS_ACDCSERVICES_H_
#define SOURCE_USER_PS_ACDCSERVICES_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "user/ps/psTask.h"
#include "lib/prot/protocol.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
enum acdcServSid{
	SID_SERV_ACDC_GET_DEVICE_INFO,
	SID_SERV_ACDC_GET_SPECIFIC_VALUE,
	SID_SERV_ACDC_GET_LIVE_DATA,
};

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t acdc_service_pack(struct protocol *, struct ucBuffer *);
extern int32_t acdc_service_handle(struct protocol *, void *);


#endif /* SOURCE_USER_PS_ACDCSERVICES_H_ */

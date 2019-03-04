/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: capCharger.h
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

#ifndef SOURCE_LIB_PROT_SERVICES_CAPCHARGER_H_
#define SOURCE_LIB_PROT_SERVICES_CAPCHARGER_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/prot/protocol.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
enum ccSercSid{
    SID_SERV_CC_GET_DEVICE_INFO,
    SID_SERV_CC_GET_SPECIFIC_VALUE,
    SID_SERV_CC_GET_LIVE_DATA,
};

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t cc_service_pack(struct protocol *, struct ucBuffer *);
extern int32_t cc_service_handle(struct protocol *, void *);


#endif /* SOURCE_LIB_PROT_SERVICES_CAPCHARGER_H_ */

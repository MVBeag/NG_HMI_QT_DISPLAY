/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: general.h
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

#ifndef SOURCE_LIB_PROT_SERVICES_GENERAL_H_
#define SOURCE_LIB_PROT_SERVICES_GENERAL_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/prot/protocol.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
enum generalServSid{
	SID_SERV_GENERAL_GET_DEVICE,
	SID_SERV_GENERAL_ERROR,
	SID_SERV_GENERAL_ROUTE_APPROVE,
};

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* The data struct to push onto the message queue. If other data than id is
 * needed, simply add your variable to the union. This way, there is
 * automatically enough memory allocated within the queue and the data can
 * be easily accessed. */
struct generalServData{
	uint16_t sid;
	/*union{
		bool charging;  // example
	}data;*/
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern int32_t general_service_pack(struct protocol *, struct ucBuffer *);
extern int32_t general_service_handle(struct protocol *, void *);


#endif /* SOURCE_LIB_PROT_SERVICES_GENERAL_H_ */

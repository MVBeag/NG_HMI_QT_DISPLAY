/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: protocol.h
 * Date			: 11.08.2017
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

#ifndef SOURCE_USER_COM_PROTOCOL_H_
#define SOURCE_USER_COM_PROTOCOL_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "mem/ucBuffer.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* Defines for the data link layer. */
#define CRC_LEN					2
#define DLINK_H_LEN				4
#define PREAMBLE				0xa5

/* Error codes used by the encoder and decoder. Should also be used by the
 * service handlers. */
#define SERVICE_ERRORS			0x20
enum{
	PROT_SUCCESS,
	PROT_ERR_INVALID_NID,
	PROT_ERR_INVALID_TID,
	PROT_ERR_SERV_ACCESS_DENIED,  /* SID/device not allowed on this com port */
	PROT_ERR_INVALID_SID,
	PROT_ERR_INVALID_LEN,  /* msg length not even long enough for headers */

	SERVICE_SUCCESS = SERVICE_ERRORS,
	SERVICE_ERR_INVALID_DATA_LEN,
	SERVICE_ERR_PARAM_OUT_OF_RANGE,
	SERVICE_ERR_SPECIFIC_1,
	SERVICE_ERR_SPECIFIC_2,
};

/* Defines for the SID field in the process layer. */
#define SID_REQ_M				0x0000
#define SID_ANS_M				0x0001
#define SID_SERV_M				0x0ffe
#define SID_SERV_S				1
#define SID_DEV_M				0xf000
#define SID_DEV_S				12

/* Enumerations for the SID fields. Note that the services are enumerated in
 * the corresponding service handler header file. */
enum deviceSid{
	SID_DEV_GENERAL,
	SID_DEV_GEN,  /* generator */
	SID_DEV_BAT,  /* battery */
	SID_DEV_ACDC,
	SID_DEV_CC,
	SID_DEV_LAMP,
	SID_ROUTING = 0xf,  /* for internal usage */
};
/**/
enum answerSid{
	SID_REQ,
	SID_ANS,
};

/*Added to arrange*/
#define DEV_ADDR_GEN    1


/******************************************************************************
 * MACROS
 *****************************************************************************/
/* Macro to set SID. The SID is composed of the three parts device[12:15],
 * service number[1:11], answer/request[0]. */
#define PROT_SID(dev, serv, ans)	((dev << SID_DEV_S) | \
									(serv << SID_SERV_S) | \
									ans)

/* Macros to write data onto the ucBuffer */
#define STORE8(dest, value) \
            dest->buf[dest->pos++] = value

#define STORE16(dest, value) \
        do{ \
            dest->buf[dest->pos++] = value >> 8; \
            dest->buf[dest->pos++] = value; \
        }while(0)

#define STORE32(dest, value) \
        do{ \
            dest->buf[dest->pos++] = value >> 24; \
            dest->buf[dest->pos++] = value >> 16; \
            dest->buf[dest->pos++] = value >> 8; \
            dest->buf[dest->pos++] = value; \
        }while(0)

/* Macros to read from the ucBuffer doing big to little endian conversion */
#define RETRIEVE8(x)            (x)[0]
#define RETRIEVE16(x)           ((x)[0]<<8 | (x)[1])
#define RETRIEVE32(x)           ((x)[0]<<24 | (x)[1]<<16 | (x)[2]<<8 | (x)[3])


/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* Note that the data link layer is handled within the RX and TX object. */
struct protocol{
	struct{
		uint8_t nid;
		union{
			struct{
				uint8_t destAddr;
				uint8_t srcAddr;
			}nid1;
			struct{
				uint8_t destDev;
				uint8_t destChn;
				uint8_t srcDev;
				uint8_t srcChn;
			}nid2;
			struct{
				uint16_t destAddr;
				uint16_t srcAddr;
				uint8_t flags;
			}nid3;
		}opt;
	}netLayer;
	struct{
		uint8_t tid;
		union{
			struct{
				uint8_t seqNr;  /* sequence number */
				uint8_t winSize;  /* window size */
				uint8_t flags;
			}tid1;
		}opt;
	}tranLayer;
	struct{
		uint16_t sid;
		uint8_t sst;  /* service status. Attached with every answer */
	}procLayer;
	union{
		struct{
			void *pData;
			int16_t dLen;
		};
		struct{
		    void *obj;
		    bool set;
		};
		uint8_t arr[8];
	}data;
};

/* This type can be used as the filter for the service_handle() function. */
struct devServList{
	 uint16_t nDevice;  /* Number of devices appearing in the device field */
	 enum deviceSid device[5];  /* List of devices that are allowed */
};

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
/**/
extern int32_t prot_dec_network_layer(struct protocol *);
extern int32_t prot_dec_transport_layer(struct protocol *);
extern int32_t prot_dec_process_layer(struct protocol *);
extern int32_t prot_decode(struct ucBuffer *, struct protocol *);
extern int32_t prot_service_handle(struct protocol *, void *, struct devServList *);
extern int32_t prot_encode(struct protocol *, struct ucBuffer *);
extern int32_t prot_fillin_error_message(struct protocol *, uint8_t);
extern int32_t prot_fillin_route_approve_message(struct protocol *, uint8_t);


#endif /* SOURCE_USER_COM_PROTOCOL_H_ */

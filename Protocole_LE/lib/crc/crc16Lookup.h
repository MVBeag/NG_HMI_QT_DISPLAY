/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: hss_tm4c123
 * File			: crc16Lookup.h
 * Date			: 14.08.2017
 * Author		: leichelberger - original A. Siebenhaar
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:
 * 
 *****************************************************************************/

#ifndef CRC16LOOKUP_H_
#define CRC16LOOKUP_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* crc init values */
#define CRC16_CCITT_INIT_FFFF	0xFFFF
#define CRC16_CCITT_INIT_0000	0x0000
#define CRC16_CCITT_INIT_1D0F 	0x1D0F

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern const uint16_t CRC16_CCITT_TABLE[256];

/******************************************************************************
 * INLINE
 *****************************************************************************/

/*
 * Calculates the CRC for one byte.
 * Argument:	pCrc	Pointer onto previous CRC value (also the result).
 * 				data	The value to build the CRC from.
 */
static inline void crc16_ccitt_byte_calc( uint16_t* pCrc, uint8_t data )
{
	uint32_t index;

	index = ( (*pCrc >> 8) ^ data );
	*pCrc = ( (*pCrc << 8) & 0xFF00 ) ^ CRC16_CCITT_TABLE[index & 0x00FF];
}
/*---------------------------------------------------------------------------*/

/*
 * Calculates the CRC for two bytes.
 * Argument:	pCrc	Pointer onto previous CRC value (also the result).
 * 				data	The value to build the CRC from.
 */
static inline void crc16_ccitt_calc( uint16_t* pCrc, uint16_t data )
{
	uint32_t index;

	index = ( (*pCrc >> 8) ^ (data & 0x00FF) );
	*pCrc = ( (*pCrc << 8) & 0xFF00 ) ^ CRC16_CCITT_TABLE[index & 0x00FF];

	data = data >> 8;

	index = ( (*pCrc >> 8) ^ (data & 0x00FF) );
	*pCrc = ( (*pCrc << 8) & 0xFF00 ) ^ CRC16_CCITT_TABLE[index & 0x00FF];
}
/*---------------------------------------------------------------------------*/

#endif /* CRC16LOOKUP_H_ */

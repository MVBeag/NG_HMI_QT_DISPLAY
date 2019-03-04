/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: eventPool.c
 * Date			: 22.01.2018
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:	This is a wrapper class for the pool object, that shall be
 * 				used to store event data.
 * 
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "lib/stm/eventPool.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
#define EPOOL_NBUCKETS			32
#define EPOOL_BUCKET_SIZE_S		4

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/**/
struct pool ePoolObj;

/**/
static uint8_t poolMemory[EPOOL_NBUCKETS * (1 << EPOOL_BUCKET_SIZE_S)];

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */
#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
void epool_init(void)
{
	int32_t err;

	err = pool_init(&ePoolObj, (void *) poolMemory,
			EPOOL_NBUCKETS, EPOOL_BUCKET_SIZE_S);
	//assert(err)
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */
/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: timerDeamon.h
 * Date			: 29.08.2017
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

#ifndef SOURCE_USER_TIME_TIMERDEAMON_H_
#define SOURCE_USER_TIME_TIMERDEAMON_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/stm/event.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* defines for the bit fields in the create_timer config argument */
#define TD_SINGLE_SHOT			0x00000000
#define TD_PERIODIC				0x00000001
#define TD_KILL					0x00000002

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef void (*timerCb_t)(void *, struct event *);

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern void timerD_init(void);
extern int16_t timerD_create_timer(uint32_t, uint8_t, int16_t *,
                                   timerCb_t, void *, struct event *);
extern int32_t timerD_set_timer(int16_t, uint32_t, uint8_t,
                                timerCb_t, void *, struct event *);
extern int32_t timerD_start_timer(int16_t);
extern int32_t timerD_restart_timer(int16_t);
extern int32_t timerD_resume_timer(int16_t);
extern int32_t timerD_stop_timer(int16_t);
extern int32_t timerD_kill_timer(int16_t);


#endif /* SOURCE_USER_TIME_TIMERDEAMON_H_ */

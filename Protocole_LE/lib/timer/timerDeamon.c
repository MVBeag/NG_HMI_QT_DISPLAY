/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: timerDeamon.c
 * Date			: 29.08.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 * 			-	Check whether using bitfields is a good idea.
 *
 ******************************************************************************
 * Description:	The timer deamon uses one TIVA wide timer, which is split
 * 				into two 32-bit timers, A & B. You can find the number of
 * 				hardware timers in the NR_HW_TIMERS define.
 * 				The two 32-bit timers are configured to run at 1MHz = 1us
 * 				per tick and to run in single shot mode. Each timer has
 * 				it's own interrupt handler, which is called after the count
 * 				value reaches 0 (down count mode).
 *
 * 				To one hardware timer, an unlimited amount of actual soft-
 * 				ware timers can be assigned, only limited through memory
 * 				usage. The number of software timers per hardware timer is
 * 				defined through the NR_TIMER_SLOTS define for a total of
 * 				NR_HW_TIMERS * NR_TIMER_SLOTS software timers.
 * 				Increasing the number of software timers is as easy as
 * 				increasing the NR_TIMER_SLOTS define. Of course, it can
 * 				also be done through adding further hardware timers, but
 * 				these have to be initialized, interrupt handler has to be
 * 				added and so on. Memory usage is the same with both methods,
 * 				but the performance is certainly better with more hardware
 * 				timers, since it is looped over a smaller software timer
 * 				array.
 *
 * 				The data of a software timer is hold in a structure using
 * 				~32 Bytes. The structure members can be grouped to 3 groups:
 * 				timing, configuration and callback.
 * 				Timing: The time period and the remaining time is stored,
 * 				both in 32-bit variables. As the time value is in microseconds,
 * 				a maximal time period of 4'295 seconds can be assigned.
 * 				Configuration: The configuration variable holds bit fields
 * 				with different meaning, namely with the state (empty, stopped,
 *				running), the type (single shot, periodic) and a kill option,
 *				allowing to automatically remove/empty a single shot timer
 *				once it has expired.
 *				Callback: Once the timer has expired, the callback function
 *				is called, which does have two arguments, the first named
 *				'handle' and the second named 'event'. Further there is the
 *				idCb, which is used to altered the id to -1 when the timer
 *				is killed.
 *
 * 				Here is the basic implementation approach of the timer deamon:
 * 				When creating a software timer, an empty timer slot is
 * 				searched (alternately between HW timer A & B) and assigned.
 * 				When starting the timer, the remaining time of running
 * 				timers is updated and the shortest remaining time is
 * 				searched in the timer array, which is written to the timer
 * 				interval HW register. Once the timer has expired (interrupt),
 * 				the remaining time of running timers is again updated. If the
 * 				remaining time becomes 0, the callback function is called and
 * 				the timer is updated according to the configuration. It is
 * 				important to note that it is always only looped over the
 * 				timers assigned to the same HW timer, hence more hardware
 * 				timer equals less loops for updating timers.
 * 				What has to be further mentioned is the interrupt safety of
 * 				the timer deamon. There are guards to make sure that ever-
 * 				thing works properly, even if new timers are created, started
 * 				or stopped from an ISR or the call back (which is also from an
 * 				ISR, namely the timer ISR itself).
 *
 * Example:
 * 		int32_t cb(void *handle, struct event *e){
 * 			...
 *			return 0;
 *		}
 * 
 * 		int16_t id;
 * 		id = timerD_create_timer(1000, TD_PERIODIC, &id, &cb, self, e);
 *		timerD_start_timer(id);
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "lib/timer/timerDeamon.h"
#include "lib/stm/event.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_types.h"  /* HWREG macro */
#include "inc/hw_memmap.h"  /* module base addresses */
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
#define NR_HW_TIMERS			2
#define NR_TIMER_SLOTS			10

/* Timer ID defines */
#define HW_ID_S					8
#define T_ID_M					0x000000ff

/**/
#define TD_CONFIG_KILL_S		1

/* Enum for the cState timer variable */
enum timerState{
	TD_TIMER_EMPTY,
	TD_TIMER_STOPPED,
	TD_TIMER_RUNNING,
	TD_TIMER_WAITING,
};

/* Enum for the timer start function */
enum{
	 TIMER_START_NORMAL,
	 TIMER_START_RESTART,
	 TIMER_START_RESUME,
};

struct timer{
	uint32_t period;  /* the period of the timer [us] */
	uint32_t rt;  /* remaining time */
	struct{
		timerCb_t func;  /* callback function */
		void *handle;  /* handle passed to the callback function */
		struct event e;  /* event passed to the callback function */
	}cb;
	int16_t *idCb;  /* pointer to the timer id */
	uint8_t cState;  /* config state 0:empty, 1:stopped, 2:running */
	uint8_t cPeriodic: 1;  /* config type 0:single shot, 1:periodic */
	uint8_t cKill: 1;  /* config kill 1:timer slot set empty after usage */
};

struct timerDeamon{
	struct timer timers[NR_HW_TIMERS][NR_TIMER_SLOTS];
};

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
volatile static struct timerDeamon self;

/* Guards for interrupt safety */
volatile static bool timerHandlerActive[NR_HW_TIMERS] = {false, false};
volatile static bool timerHandlerAccessed[NR_HW_TIMERS] = {false, false};

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static void enable_modules(void);
static void init_wtimer(void);

static int32_t handle_timers_hw(uint32_t);
static int32_t handle_timers(volatile struct timer *, uint32_t);
static uint32_t trigger_n_timeout(volatile struct timer *);
static int32_t start_timer(int16_t, uint32_t);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
void enable_modules(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
	SysCtlDelay(3);
}
/*---------------------------------------------------------------------------*/

/*
 * Initializes wide timer 0 A&B.
 */
void init_wtimer(void)
{
	/* enable interrupt & set priority (7=lowest) in the NVIC */
	IntEnable(INT_WTIMER0A_TM4C123);
	IntPrioritySet(INT_WTIMER0A_TM4C123, 7<<5);
	IntEnable(INT_WTIMER0B_TM4C123);
	IntPrioritySet(INT_WTIMER0B_TM4C123, 7<<5);
	/* configure timer */
	HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 0) = 0;  /* Timer A disable */
	HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 8) = 0;  /* Timer B disable */
	HWREG(WTIMER0_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;  /* 2xtimer */
	HWREG(WTIMER0_BASE + TIMER_O_TAMR) = 0
			//| TIMER_TAMR_TAPLO  /* PWM Legacy Operation */
			//| TIMER_TAMR_TAMRSU  /* Match Register Update */
			//| TIMER_TAMR_TAPWMIE  /* PWM Interrupt Enable */
			//| TIMER_TAMR_TAILD  /* Interval Load Write (0=immediate) */
			//| TIMER_TAMR_TASNAPS  /* Snap-Shot Mode */
			//| TIMER_TAMR_TAWOT  /* Wait-on-Trigger */
			//| TIMER_TAMR_TAMIE  /* Match Interrupt Enable */
			//| TIMER_TAMR_TACDIR  /* Count Direction (0=down) */
			//| TIMER_TAMR_TAAMS  /* Alternate Mode Select */
			//| TIMER_TAMR_TACMR  /* Capture Mode (see Mode) */
			| TIMER_TAMR_TAMR_1_SHOT  /* Mode */
			| 0;
	HWREG(WTIMER0_BASE + TIMER_O_TBMR) = 0
			| TIMER_TBMR_TBMR_1_SHOT  /* Mode */
			| 0;
	HWREG(WTIMER0_BASE + TIMER_O_CTL) = 0
			//| TIMER_CTL_TBPWML  /* PWM Output Level */
			//| TIMER_CTL_TBOTE  /* Output Trigger Enable */
			//| TIMER_CTL_TBEVENT_POS  /* Event Mode */
			| TIMER_CTL_TBSTALL  /* Stall Enable */
			//| TIMER_CTL_TBEN  /* Enable */
			//| TIMER_CTL_TAPWML  /* PWM Output Level */
			//| TIMER_CTL_TAOTE  /* Output Trigger Enable */
			//| TIMER_CTL_TAEVENT_POS  /* Event Mode */
			| TIMER_CTL_TASTALL  /* Stall Enable */
			//| TIMER_CTL_TAEN  /* Enable */
			| 0;
	HWREG(WTIMER0_BASE + TIMER_O_IMR) = 0
			| TIMER_IMR_WUEIM  /* Write Update Error */
			//| TIMER_IMR_DMABIM  /* DMA Done */
			//| TIMER_IMR_TBMIM  /* Match */
			//| TIMER_IMR_CBEIM  /* Capture Mode Event */
			//| TIMER_IMR_CBMIM  /* Capture Mode Match */
			| TIMER_IMR_TBTOIM  /* Time-Out */
			//| TIMER_IMR_DMAAIM  /* DMA Done */
			//| TIMER_IMR_TAMIM  /* Match */
			//| TIMER_IMR_RTCIM  /* RTC */
			//| TIMER_IMR_CAEIM  /* Capture Mode Event */
			//| TIMER_IMR_CAMIM  /* Capture Mode Match */
			| TIMER_IMR_TATOIM  /* Time-Out */
			| 0;
	HWREG(WTIMER0_BASE + TIMER_O_TAPR) = 79;  /* Timer A prescale (80MHz/80) */
	HWREG(WTIMER0_BASE + TIMER_O_TBPR) = 79;  /* Timer B prescale (80MHz/80) */
}
/*---------------------------------------------------------------------------*/

/*
 * Chooses the hardware module according to the ID argument, calculates the
 * time delta to the last timer interval update and calls the software timer
 * handler (handle_timers() function). Ones the software timers are updated,
 * the shortest remaining time is calculated and filled into the HW interval
 * register. The timer is finally started if the interval is >0.
 *
 * Argument:	hwId	The hardware ID, distinguishing the timer module
 * Return:		 0		success
 */
int32_t handle_timers_hw(uint32_t hwId)
{
	uint32_t dt;  /* time delta to the last timer interval update */
	uint32_t t;  /* shortest remaining time --> HW interval setup */

	/* hardware timer 0 */
	if(hwId == 0){
		HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 0) = 0;  /* TimerA disable */
		dt = HWREG(WTIMER0_BASE + TIMER_O_TAILR)
				- HWREG(WTIMER0_BASE + TIMER_O_TAV);
		handle_timers(self.timers[hwId], dt);
		do{
			timerHandlerAccessed[hwId] = false;
			t = trigger_n_timeout(self.timers[hwId]);
		}while(timerHandlerAccessed[hwId]);
		if(t > 0){
			HWREG(WTIMER0_BASE + TIMER_O_TAILR) = t;
			HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 0) = 1;  /* TimerA enable */
		}
	/* hardware timer 1 */
	}else if(hwId == 1){
		timerHandlerActive[1] = true;
		HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 8) = 0;  /* TimerB disable */
		dt = HWREG(WTIMER0_BASE + TIMER_O_TBILR)
				- HWREG(WTIMER0_BASE + TIMER_O_TBV);
		handle_timers(self.timers[hwId], dt);
		do{
			timerHandlerAccessed[hwId] = false;
			t = trigger_n_timeout(self.timers[hwId]);
		}while(timerHandlerAccessed[hwId]);
		if(t > 0){
			HWREG(WTIMER0_BASE + TIMER_O_TBILR) = t;
			HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 8) = 1;  /* TimerB enable */
		}
	}
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Updates the remaining time of the running timers with a common HW timer.
 * If the remaining time is 0 the owner is called back and the timer is
 * either killed/removed, stopped or refreshed (if periodic).
 *
 * Argument:	timers	Pointer to timer array of one HW timer.
 * 				dt		The time delta to the last timer interval update
 * Return:		 0		success
 */
int32_t handle_timers(volatile struct timer *timers, uint32_t dt)
{
	int32_t i;
	uint32_t nExpired = 0;  /* number of expired timers */
	uint8_t iExpired[NR_TIMER_SLOTS];  /* index of expired timer */
	struct event eTmp;
	volatile struct timer *timer;

	for(i=0; i<NR_TIMER_SLOTS; i++){
		timer = &timers[i];
		if(timer->cState == TD_TIMER_RUNNING){
			timer->rt -= dt;
			/* timer expired */
			if(timer->rt == 0){
				iExpired[nExpired++] = i;
				if(timer->cPeriodic == 1){
					timer->rt = timer->period;
				}else if(timer->cKill == 1){
					timer->cState = TD_TIMER_EMPTY;
			        if(timer->idCb != NULL)
			            *timer->idCb = -1;
				}else{
					timer->cState = TD_TIMER_STOPPED;
				}
			}
		}
	}
	/* callback expired timers */
	for(i=0; i<nExpired; i++){
		timer = &timers[iExpired[i]];
		if(timer->cb.func != NULL){
			eTmp = timer->cb.e;  /* send a copy since it might be rendered */
			timer->cb.func(timer->cb.handle, &eTmp);
		}
	}
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Sets waiting timers active/running and returns the shortest remaining time.
 * Timers that are started with any start function are not set to running, but
 * waiting, because they could be rendered incorrectly by an already running
 * call to handle_timers() otherwise.
 *
 * Argument:	timers	Pointer to timer array of one HW timer.
 * Return:		t		The shortest remaining time. 0 if no timer remains
 * 						active.
 */
uint32_t trigger_n_timeout(volatile struct timer *timers)
{
	int32_t i;
	uint32_t t = 0xffffffff;

	for(i=0; i<NR_TIMER_SLOTS; i++){
		if(timers[i].cState == TD_TIMER_RUNNING ||
				timers[i].cState == TD_TIMER_WAITING){
			timers[i].cState = TD_TIMER_RUNNING;
			if(timers[i].rt < t)
				t = timers[i].rt;
		}
	}
	return (t == 0xffffffff) ? 0 : t;
}
/*---------------------------------------------------------------------------*/

/*
 * Internal function to start a timer. Whether a timer is started, resumed
 * or restarted is basically the same. This behavior is collected here.
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * 				type	TIMER_START_NORMAL	If already running, do nothing.
 * 						TIMER_START_RESTART	If already running, reload period.
 * 						TIMER_START_RESUME	If stopped resumes at remaining time.
 * Return:		err		 0	success
 * 						-1	invalid id
 * 						-2	timer empty
 * 						-3	timer already running
 * 						-4	timer can not be resumed if remaining time is 0
 */
int32_t start_timer(int16_t id, uint32_t type)
{
	uint32_t hwId;  /* hardware id, first index of the 2D array */
	uint32_t tId;  /* timer id, second index of the 2D array */
	uint32_t period;  /* the period of the timer to start */

	hwId = id >> HW_ID_S;
	tId = id & T_ID_M;
	/**/
	if(hwId > NR_HW_TIMERS || tId > NR_TIMER_SLOTS || id < 0)
		return -1;
	if(self.timers[hwId][tId].cState == TD_TIMER_EMPTY)
		return -2;
	switch(type){
	case TIMER_START_NORMAL:
		if(self.timers[hwId][tId].cState == TD_TIMER_RUNNING)
			return -3;
		period = self.timers[hwId][tId].period;
		break;
	case TIMER_START_RESTART:
		period = self.timers[hwId][tId].period;
		break;
	case TIMER_START_RESUME:
		if(self.timers[hwId][tId].cState == TD_TIMER_RUNNING)
			return -3;
		if(self.timers[hwId][tId].rt == 0)
			return -4;
		period = self.timers[hwId][tId].rt;
		break;
	}
	self.timers[hwId][tId].rt = period;
	self.timers[hwId][tId].cState = TD_TIMER_WAITING;
	/* guarded call to the timer handler */
	if(!timerHandlerActive[hwId]){
		timerHandlerActive[hwId] = true;
		handle_timers_hw(hwId);
		timerHandlerActive[hwId] = false;
	}else{
		timerHandlerAccessed[hwId] = true;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
void timerD_init(void)
{
	enable_modules();
	init_wtimer();
}
/*---------------------------------------------------------------------------*/

/*
 * This function creates a new timer if there is a empty slot. The ID that is
 * returned can than be used to manipulate the timer, e.g. start it.
 *
 * Argument:	period	time period in microseconds.
 * 				config	timer configuration, possible options are TD_PERIODIC,
 * 						TD_SINGLE_SHOT, TD_KILL.
 * 				idCb    Timer ID callback. The pointer is stored within the
 * 				        timer. It allows to directly fill the timer ID into
 * 				        the variable pointed to. This makes it possible to
 * 				        render the ID invalid (-1) once the timer has been
 * 				        killed.
 * 				        Be careful with this argument, only statically
 * 				        allocated variables must be assigned otherwise the
 * 				        timer eventually writes to a place where he should
 * 				        not! So if you have a local variable, pass NULL and
 * 				        use the return value of this function. If you assign
 * 				        the variable, you don't need the return value.
 * 				cb		callback function.
 * 				handle	Handle passed to callback function (first argument).
 * 				        Can be anything but it's typically used for the task
 * 				        reference.
 * 				e		Event passed to the callback function (second argument).
 * Return:		id		positive numbers are valid id's,
 * 						negative are error codes:
 * 						-1 no free timer slot
 * 						-2 period must not be 0
 */
int16_t timerD_create_timer(uint32_t period, uint8_t config, int16_t *idCb,
							timerCb_t cb, void *handle, struct event *e)
{
	int32_t i;
	int32_t j;

    if(idCb != NULL)
        *idCb = -1;
	if(period == 0)
		return -2;
	/* find unused timer slot */
	for(i=0; i<NR_TIMER_SLOTS; i++){
		for(j=0; j<NR_HW_TIMERS; j++){
			if(self.timers[j][i].cState == TD_TIMER_EMPTY){
				self.timers[j][i].period = period;
				self.timers[j][i].rt = period;
				self.timers[j][i].cb.func = cb;
				self.timers[j][i].cb.handle = handle;
				self.timers[j][i].cb.e = *e;
				self.timers[j][i].cPeriodic = config & 0x1;
				self.timers[j][i].cKill = (config >> TD_CONFIG_KILL_S) & 0x1;
				self.timers[j][i].cState = TD_TIMER_STOPPED;
				self.timers[j][i].idCb = idCb;
				if(idCb != NULL)
				    *idCb = j<<HW_ID_S | i;
				return j<<HW_ID_S | i;
			}
		}
	}
	while(1){}  /* loop forever if not enough timers! */
	return -1;
}
/*---------------------------------------------------------------------------*/

/*
 * Allows to set the timing and configuration of an already allocated
 * timer. The timer is stopped when calling this function.
 *
 * Argument:    id      The id returned by the timerD_create_timer() function.
 *              period  time period in microseconds.
 *              config  timer configuration, possible options are TD_PERIODIC,
 *                      TD_SINGLE_SHOT, TD_KILL.
 *              cb      callback function.
 *              handle  Handle passed to callback function (first argument).
 *              e       Event passed to the callback function (second argument).
 * Return:      err      0  success
 *                      -1  invalid id
 *                      -2  timer empty
 *                      -3  period must not be 0
 */
int32_t timerD_set_timer(int16_t id, uint32_t period, uint8_t config,
                         timerCb_t cb, void *handle, struct event *e)
{
    int32_t err;
    uint32_t hwId;  /* hardware id, first index of the 2D array */
    uint32_t tId;  /* timer id, second index of the 2D array */

    if(period == 0)
        return -3;
    err = timerD_stop_timer(id);
    if(err)
        return err;
    hwId = id >> HW_ID_S;
    tId = id & T_ID_M;
    self.timers[hwId][tId].period = period;
    self.timers[hwId][tId].rt = period;
    self.timers[hwId][tId].cb.func = cb;
    self.timers[hwId][tId].cb.handle = handle;
    self.timers[hwId][tId].cb.e = *e;
    self.timers[hwId][tId].cPeriodic = config & 0x1;
    self.timers[hwId][tId].cKill = (config >> TD_CONFIG_KILL_S) & 0x1;
    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Starts a previously created timer. When a timer is started, the counter is
 * set to the value given in period. If you don't want this behavior, use
 * timerD_resume_timer(). If the timer is already running, the timer continues
 * running without being rendered. If you want the timer to reload the period,
 * use timerD_restart_timer().
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * Return:		err		 0	success
 * 						-1	invalid id
 * 						-2	timer empty
 * 						-3	timer already running
 */
int32_t timerD_start_timer(int16_t id)
{
	int32_t err;

	err = start_timer(id, TIMER_START_NORMAL);
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Restarts a previously created timer. In any case, the timer is set to its
 * period and is then started.
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * Return:		err		 0	success
 * 						-1	invalid id
 * 						-2	timer empty
 */
int32_t timerD_restart_timer(int16_t id)
{
	int32_t err;

	err = start_timer(id, TIMER_START_RESTART);
	return err;
}
/*---------------------------------------------------------------------------*/

/*
 * Resumes a previously created timer. If the timer has been stopped, the
 * timer resumes from where it has been stopped. If the timer is already
 * running, the timer continues running without being rendered. If the timer
 * has just been created, the remaining time equals the period, hence start
 * and resume has the same effect.
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * Return:		err		 0	success
 * 						-1	invalid id
 * 						-2	timer empty
 * 						-3	timer already running
 * 						-4	timer can not be resumed if remaining time is 0
 */
int32_t timerD_resume_timer(int16_t id)
{
	int32_t err;

	err = start_timer(id, TIMER_START_RESUME);
	return err;
}
/*---------------------------------------------------------------------------*/


/*
 * Stops a timer. Stopping a timer changes the state attribute to stopped,
 * independent from the kill argument. A stopped timer can be either resumed
 * or started/restarted (which is the same in this case).
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * Return:		err		 0	success
 * 						-1	invalid id
 * 						-2	timer empty
 */
int32_t timerD_stop_timer(int16_t id)
{
	uint32_t hwId;  /* hardware id, first index of the 2D array */
	uint32_t tId;  /* timer id, second index of the 2D array */

	hwId = id >> HW_ID_S;
	tId = id & T_ID_M;
	if(hwId > NR_HW_TIMERS || tId > NR_TIMER_SLOTS || id < 0)
		return -1;
	if(self.timers[hwId][tId].cState == TD_TIMER_EMPTY)
		return -2;
	if(self.timers[hwId][tId].cState == TD_TIMER_STOPPED)
		return 0;
	self.timers[hwId][tId].cState = TD_TIMER_STOPPED;
	/* guarded call to the timer handler */
	if(!timerHandlerActive[hwId]){
		timerHandlerActive[hwId] = true;
		handle_timers_hw(hwId);
		timerHandlerActive[hwId] = false;
	}else{
		timerHandlerAccessed[hwId] = true;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Kills a timer. Killing a timer stops the timer and changes the state
 * attribute to empty. Hence a new timer can be assigned to the slot.
 *
 * Argument:	id		The id returned by the timerD_create_timer() function.
 * Return:		err		 0	success
 * 						-1	invalid id
 */
int32_t timerD_kill_timer(int16_t id)
{
	int32_t err;
	uint32_t hwId;  /* hardware id, first index of the 2D array */
	uint32_t tId;  /* timer id, second index of the 2D array */

	hwId = id >> HW_ID_S;
	tId = id & T_ID_M;
	err = timerD_stop_timer(id);
	if(err != -1){
		self.timers[hwId][tId].cState = TD_TIMER_EMPTY;
        if(self.timers[hwId][tId].idCb != NULL)
            *self.timers[hwId][tId].idCb = -1;
	}else{
		return -1;
    }
	return 0;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * INTERRUPTS
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 *
 */
void ISR_WTIMER0A(void)
{
	uint32_t dt;
	uint32_t t;

	timerHandlerActive[0] = true;  /* interrupt guard */
	HWREGBITW(WTIMER0_BASE + TIMER_O_ICR, 0) = 1;  /* clear time-out int */
	dt = HWREG(WTIMER0_BASE + TIMER_O_TAILR);  /* interval time */
	handle_timers(self.timers[0], dt);
	do{
		timerHandlerAccessed[0] = false;
		t = trigger_n_timeout(self.timers[0]);
	}while(timerHandlerAccessed[0]);
	if(t > 0){
		HWREG(WTIMER0_BASE + TIMER_O_TAILR) = t;
		HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 0) = 1;  /* TimerA enable */
	}
	timerHandlerActive[0] = false;
}
/*---------------------------------------------------------------------------*/

/*
 *
 */
void ISR_WTIMER0B(void)
{
	uint32_t dt;
	uint32_t t;

	timerHandlerActive[1] = true;
	HWREGBITW(WTIMER0_BASE + TIMER_O_ICR, 8) = 1;  /* clear time-out int */
	dt = HWREG(WTIMER0_BASE + TIMER_O_TBILR);  /* interval time */
	handle_timers(self.timers[1], dt);
	do{
		timerHandlerAccessed[1] = false;
		t = trigger_n_timeout(self.timers[1]);
	}while(timerHandlerAccessed[1]);
	if(t > 0){
		HWREG(WTIMER0_BASE + TIMER_O_TBILR) = t;
		HWREGBITW(WTIMER0_BASE + TIMER_O_CTL, 8) = 1;  /* TimerB enable */
	}
	timerHandlerActive[1] = false;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

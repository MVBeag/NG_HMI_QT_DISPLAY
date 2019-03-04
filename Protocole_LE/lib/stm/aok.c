/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: aok.c
 * Date			: 07.12.2017
 * Author		: leichelberger
 ******************************************************************************
 * Known Bugs (_FIXME):
 *
 * Enhancement (_TODO):
 *
 ******************************************************************************
 * Description:
 * 
 * 
 *****************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "lib/stm/aok.h"
#include "lib/mem/set56.h"

/******************************************************************************
 * DEFINES & MACROS & TYPEDEFS
 *****************************************************************************/
/**/
struct scheduler{
	struct ao *aos[MAX_NR_AOS];  /* Reference to registered AOs */
	uint8_t nAoOnPrio[NR_PRIO_LVL];  /* Number of registered AOs on priority [x] */
	uint8_t aoPrioOffset[NR_PRIO_LVL];  /* Offset in the AO list for priority [x] */
	volatile struct set56 waitingAoSet[NR_PRIO_LVL];  /**/
	volatile uint8_t waitingPrio;  /* Bit x signals non empty queue in AO having prio x */
	volatile uint8_t prioMask;  /* Mask holding the currently handled priority */
	uint8_t nAos;  /* Number of registered AOs */
};

/**/
typedef void (*dispatchAo_t)(struct ao *, struct event *);

/******************************************************************************
 * FILE SCOPE VARIABLES
 *****************************************************************************/
/**/
static struct scheduler self;

/**/
static struct event entryEvt = {.sig = STATE_ENTRY_SIG};
static struct event initEvt = {.sig = STATE_INIT_SIG};
static struct event exitEvt = {.sig = STATE_EXIT_SIG};

/******************************************************************************
 * PROTOTYPES (LOCAL)
 *****************************************************************************/
static void state_entry_hsm(struct aoHsm *, uint32_t);
static void dispatch_hsm(struct aoHsm *, struct event *);
static void dispatch_stm(struct aoStm *, struct event *);

/******************************************************************************
 * SUBROUTINES (LOCAL)
 *****************************************************************************/
#if(1)	/* code folding trick */

 /*
  * Enter and initialize states.
  * This function enters and initializes states in a HSM, starting with
  * the state given with the argument lvl. The function than continues to
  * call itself, entering and initializing till the leave state is reached.
  *
  * Argument:	ao		Pointer to active object.
  * 			lvl		The nesting level from which states will be entered,
  * 					including himself.
  */
void state_entry_hsm(struct aoHsm *ao, uint32_t lvl)
{
	if(ao->state[lvl].nextFunc != NULL){
		ao->nesting = lvl;
		ao->state[lvl].func = ao->state[lvl].nextFunc;
		ao->state[lvl].nextFunc = NULL;
		(*ao->state[lvl].func)(ao, &entryEvt);
		if(lvl < ao->maxNesting-1){
			if(ao->state[lvl+1].nextFunc == NULL)
				(*ao->state[lvl].func)(ao, &initEvt);
			state_entry_hsm(ao, ++lvl);
		}
	}
}
/*---------------------------------------------------------------------------*/

/*
 * The dispatch function calls the active state.
 *
 * Argument:	ao		Pointer to active object.
 *	 			e		Pointer to event.
 */
void dispatch_hsm(struct aoHsm *ao, struct event *e)
{
	int32_t i, k;
	int32_t lvl;

 	for(i=ao->nesting; i>=0; i--){
		//assert(ao->state[i].func == NULL);
		(*ao->state[i].func)(ao, e);
		if(e->sig < FIRST_USER_SIG){
			if(e->sig == EVENT_HANDLED_SIG){
				break;
			}else if(e->sig == STATE_TRAN_SIG){
				lvl = i - (e->data & ~TRAN_LOCAL_M);
				for(k=ao->nesting; k>lvl; k--){
					//assert(k < 0);
					//assert(ao->state[k].func == NULL);
					(*ao->state[k].func)(ao, &exitEvt);
				}
				if(e->data & TRAN_LOCAL_M)
					(*ao->state[lvl].func)(ao, &initEvt);
				ao->nesting = lvl;
				state_entry_hsm(ao, ++lvl);
				break;
			}else{
				//assert(1);
				break;
			}
		}
	}
}
 /*---------------------------------------------------------------------------*/

/*
 * The dispatch function calls the active state.
 *
 * Argument:	ao		Pointer to active object.
 *	 			e		Pointer to event.
 */
static void dispatch_stm(struct aoStm *ao, struct event *e)
{
	//assert(ao->state.func == NULL);
	(*ao->state.func)(ao, e);
	if(e->sig == STATE_TRAN_SIG){
		(*ao->state.func)(ao, &exitEvt);
		while(entryEvt.sig != STATE_ENTRY_SIG){}
		ao->state.func = ao->state.nextFunc;
		//assert(ao->state.func == NULL);
		(*ao->state.func)(ao, &entryEvt);
	}
}
/*---------------------------------------------------------------------------*/

/*
 * Scheduler sub-function.
 * This function gets events from non-empty queues having the same priority.
 * Because most of the time, only a single queue is non-empty, this case is
 * handled separately in a first part. If multiple queues on this priority are
 * non-empty, the while loop is entered, where a copy of the set is used to
 * dispatch events in round robin fashion. Note that the first queue is
 * eventually accessed twice in a row.
 * After an AO is done, the waitingPrio bit-field is checked if a higher
 * priority AO is waiting. If yes, the function returns.
 *
 * Argument:	prio	priority to handle.
 */
static inline void handle_prio(uint32_t prio)
{
	int32_t err;
	uint32_t idx;
	uint8_t k;
	struct set56 tmpSet;
	struct event *e;
	struct ao *ao;

	idx = self.aoPrioOffset[prio];
	/* handle event of first queue */
	SET56_FIND(self.waitingAoSet[prio], k);
	ao = self.aos[idx+k];
	err = xQueue_get(&ao->eventQueue, (void **) &e);
	if(!err){
		(*ao->dispatch)(ao, e);
		xQueue_consume(&ao->eventQueue);
		if(XQUEUE_EMPTY(&ao->eventQueue)){
			SET56_REMOVE(self.waitingAoSet[prio], k);
		}
	}else{
		while(1);  // TODO err
		//SET56_REMOVE(self.waitingAoSet[prio], k);
	}
	/* handle events of all the other queues if any */
	while(!SET56_EMPTY(self.waitingAoSet[prio])){
		tmpSet = self.waitingAoSet[prio];
		do{
			/* higher priority AO waiting */
			if(self.waitingPrio >= (self.prioMask<<1))
				return;
			SET56_FIND(tmpSet, k);
			SET56_REMOVE(tmpSet, k);
			ao = self.aos[idx+k];
			err = xQueue_get(&ao->eventQueue, (void **) &e);
			if(!err){
				(*ao->dispatch)(ao, e);
				xQueue_consume(&ao->eventQueue);
				if(XQUEUE_EMPTY(&ao->eventQueue)){
					SET56_REMOVE(self.waitingAoSet[prio], k);
				}
			}else{
				while(1);  // TODO err
				//SET56_REMOVE(self.waitingAoSet[prio], k);
			}
		}while(!SET56_EMPTY(tmpSet));
	}
	self.waitingPrio &= ~self.prioMask;
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

/******************************************************************************
 * SUBROUTINES (EXPORT)
 *****************************************************************************/
#if(1)	/* code folding trick */

/*
 * The scheduler or dispatcher is a very simple non preemptive kernel.
 * He searches a bit field telling on which priority there are waiting AOs.
 * Once the highest waiting priority is found, all AOs on this priority
 * level are checked whether they have a non empty queue and the events are
 * dispatched eventually. AOs on the same level are handled in a round-robin
 * fashion, meaning that if multiple events are awaiting in each queue, the
 * events are dispatched alternating.
 * Whenever a higher priority AO is waiting with a non empty queue, the
 * higher priority AO is searched just after the current AO has finished.
 * Note that high priority task must not necessarily be waiting, this solely
 * depends on whether ao_post() or ao_dispatch() is issued.
 * Note the TASK pragma below. It tells the compiler that this function is
 * entered only once and never left, helping the compiler to save RAM and Flash.
 */
#pragma TASK(ao_scheduler)
void ao_scheduler(void)
{
	uint32_t n;
	uint32_t prio;

	while(1){
		/* Sleep/idle till something happens */
		while(!self.waitingPrio){}
		/* something happened */
		do{
			n = log2lookup[self.waitingPrio];
			prio = NR_PRIO_LVL - n;
			self.prioMask = 1 << (n-1);
			handle_prio(prio);
		}while(self.waitingPrio);
		self.prioMask = 0;
		/* Nothing to do at the moment. Check error then sleep. */
		//assert(entryEvt.sig != STATE_ENTRY_SIG);
		//assert(initEvt.sig != STATE_INIT_SIG);
		//assert(exitEvt.sig != STATE_EXIT_SIG);
		//assert(self.waitingAo); //set[]
	}
}
/*---------------------------------------------------------------------------*/

/*
 * Initialize the event queue.
 * Each active object has an event queue of xQueue type where the events
 * are stored. This function is a wrapper for the xQueue_init() function.
 *
 * Argument:    ao      pointer to active object
 *              mem     pointer to event queue memory
 *              len     number of events the event queue memory can hold
 * Return:       0      success
 *              -1      length to big, must be smaller than 0xff>>1
 */
int32_t ao_init_event_queue(struct ao *ao, struct event *mem, uint8_t len)
{
    return xQueue_init(&ao->eventQueue, (void *) mem,
                       len, sizeof(struct event));
}
/*---------------------------------------------------------------------------*/

/*
 * Assign and initialize the memory where the current & next state is
 * kept (use only if aoHsm).
 * Why not use constant amount of memory and directly allocate it
 * statically when creating an aoHsm object?
 * Because in case of a HSM AO the level of nesting is variable. For every
 * nesting level two function pointers are needed, pointing to the active
 * state (and the next state in case of a state transition). Hence the
 * memory for these function pointers has to be assigned. This is not the
 * case with flat STMs, there the memory for the state pointer is always
 * the same and is in the aoStm structure.
 * Note that this function must be called before using the macros to
 * set the state (HSM_SET_STATE), otherwise you will probably write
 * to the NULL pointer.
 *
 * Argument:    ao      pointer to HSM active object
 *              mem     pointer to HSM state memory
 *              len     number of maximal nesting levels
 * Return:       0      success
 */
int32_t ao_init_hsm_state_memory(struct aoHsm *ao, struct hsmState *mem,
                                 uint8_t len)
{
    ao->state = mem;
    ao->maxNesting = len;
    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Register an active object (AO) to be recognized by the scheduler.
 * The scheduler keeps track of all registered AOs in an ordered list, where
 * the highest priority AOs are written to the very start of this list. When
 * registering a AO, the initial entry signal is automatically posted.
 *
 * Argument:	ao		Pointer to the active object
 * 				prio	Priority of active object. 0 = highest.
 * 				hsm		Both HSM and STM may be used, tell what it is.
 * Return:		 0		success
 * 				-1		active object is a NULL pointer
 * 				-2		event queue memory is a NULL pointer
 * 				-3		Priority exceeded
 * 				-4		AO list full
 * 				-5		AO inheriting from aoHsm needs state memory
 */
int32_t ao_register(struct ao *ao, uint32_t prio, bool hsm)
{
	int32_t i;
	uint32_t nAosLow;

	if(ao == NULL)
		return -1;
	if(ao->eventQueue.buffer == NULL)
		return -2;
	if(prio >= NR_PRIO_LVL)
		return -3;
	if(self.nAos >= MAX_NR_AOS)
		return -4;
	/**/
	for(i=NR_PRIO_LVL-1; i>prio; i--){
		self.aoPrioOffset[i]++;
	}
	nAosLow = self.aoPrioOffset[prio] + self.nAoOnPrio[prio];
	for(i=self.nAos; i>nAosLow; i--){
		self.aos[i] = self.aos[i-1];
		(self.aos[i]->handle)++;
	}
	self.aos[nAosLow] = ao;
	ao->handle = nAosLow;
	ao->prio = prio;
	ao->prioMask = 1 << (NR_PRIO_LVL - prio - 1);
	xQueue_reset(&ao->eventQueue);  /* make sure event queue empty */
	/* Assign dispatch function (STM or HSM) and enter initial state */
	if(hsm){
		ao->dispatch = (dispatchAo_t) &dispatch_hsm;
		if(((struct aoHsm *) ao)->state == NULL)
			return -5;
		state_entry_hsm((struct aoHsm *)ao, 0);
	}else{
		ao->dispatch = (dispatchAo_t) &dispatch_stm;
		//assert(((struct aoStm *) ao)->state.nextFunc == NULL);
		((struct aoStm *) ao)->state.func = ((struct aoStm *) ao)->state.nextFunc;
		(*((struct aoStm *) ao)->state.func)((struct aoStm *) ao, &entryEvt);
	}
	/* add the AO to be recognized by the scheduler */
	self.nAoOnPrio[prio]++;
	self.nAos++;
	return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * Post an event to the event queue of an active object.
 * By using ao_post(), behavior is completely asynchronous, meaning that the
 * currently running AO is finished even if the event given with ao_post wakes
 * a higher priority AO. The higher priority AO will then start as soon as
 * the running AO has finished.
 * If you only use ao_post for event communication, you are absolutely save
 * from any data access concurrency problems. However, delay of event dispatch
 * is dependent on the granularity of the event handling.
 *
 * Argument:	ao		pointer to the AO
 * 				event	pointer to the event that will be queued
 */
void ao_post(struct ao *ao, struct event *e)
{
    int32_t err;
	uint8_t k;

	//assert(ao->handle >= self.nAos);
	//assert(ao != self.aos[ao->handle]);
	err = xQueue_push(&ao->eventQueue, e);
	if(err) while(1){}
	k = ao->handle - self.aoPrioOffset[ao->prio];
	SET56_INSERT(self.waitingAoSet[ao->prio], k);
	self.waitingPrio |= ao->prioMask;
}
/*---------------------------------------------------------------------------*/

/*
 * Dispatch or post an event.
 * By using ao_dispatch(), preemptive behavior can be achieved, meaning that
 * the currently running AO can be preempted if he is of lower priority than
 * the AO that is waked through the event given with ao_dispatch.
 * When using ao_dispatch, be aware of data access concurrency problems. To
 * be save, one should only communicate through events in this case!
 *
 * The function makes sure that the AO is of higher priority than the currently
 * highest waiting or running AO. If this is the case, the event is directly
 * dispatched, preempting the currently running AO. If the requirement is not
 * fulfilled, the event is posted to the event queue and asynchronously managed
 * by the scheduler as usual.
 *
 * Argument:	ao		pointer to the AO
 * 				event	pointer to the event that will be dispatched
 */
void ao_dispatch(struct ao *ao, struct event *e)
{
	uint8_t tmp;

	//assert(ao->handle >= self.nAos);
	//assert(ao != self.aos[ao->handle]);
	if(ao->prioMask > self.prioMask){
		tmp = self.prioMask;  /* backup */
		self.prioMask = ao->prioMask;
		(*ao->dispatch)(ao, e);
		self.prioMask = tmp;  /* restore */
	}else{
		ao_post(ao, e);
	}
}
/*---------------------------------------------------------------------------*/

#endif	/* end code folding */

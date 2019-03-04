/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		: generator2018
 * File			: aok.h
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
 *****************************************************************************/

#ifndef SOURCE_LIB_STM_AOK_H_
#define SOURCE_LIB_STM_AOK_H_


/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "lib/stm/event.h"
#include "lib/mem/xQueue.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/
/* Number of priority levels. 0 is always the highest level while NR_PRIO_LVL-1
 * is the lowest. */
#define NR_PRIO_LVL			8

/* Maximal number of active objects (AO). Using set56, there is a maximum of
 * NR_PRIO_LVL x 56 allowed. */
#define MAX_NR_AOS			64

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/* Basic structure of an active object (AO). */
struct ao{
	struct xQueue eventQueue;
	void (*dispatch)(struct ao *, struct event *);
	uint8_t handle;  /* holds the position of the AO in the scheduler list */
	uint8_t prio;  /* AO priority */
	uint8_t prioMask;  /* AO priority. Redundant priority as shift. */
	uint8_t objType;  /* This field allows to identify the structure type. */
};

/* Structure of an active object (AO), being a HSM or a STM.
 * Every AO that shall be actively managed by the scheduler must inherit from
 * one of the two below structure.
 * There is one structure for HSM (hierarchical) and one for STM (flat). Which
 * one is used must be told when registering the active object. */
struct aoHsm{
	struct ao super;
	struct hsmState{
		void (*func)(struct aoHsm *, struct event *);
		void (*nextFunc)(struct aoHsm *, struct event *);
	}*state;
	uint8_t maxNesting;
	uint8_t nesting;
};

/**/
struct aoStm{
	struct ao super;
	struct stmState{
		void (*func)(struct aoStm *, struct event *);
		void (*nextFunc)(struct aoStm *, struct event *);
	}state;
};

/* Typedefs for type conversion of state functions. */
typedef void (*hsmState_t)(struct aoHsm *, struct event *);
typedef void (*stmState_t)(struct aoStm *, struct event *);

/******************************************************************************
 * MACROS
 *****************************************************************************/

/*--- STM ---*/

/* Macro to set the next state. Only needed for state initialization.
 * Argument:	obj		pointer to AO
 * 				st		pointer to state function
 */
#define STM_SET_STATE(obj, st) \
		do{ \
			((struct aoStm *) obj)->state.nextFunc = (stmState_t) st; \
		}while(0)

/* Macro to do a state transition.
 * Argument:	obj		pointer to AO
 * 				st		pointer to state function
 * 				e		event pointer found in the state function arguments
 */
#define STM_STATE_TRAN(obj, st, e)	\
		do{ \
			((struct aoStm *) obj)->state.nextFunc = (stmState_t) st; \
			e->sig = STATE_TRAN_SIG; \
		}while(0)

/*--- HSM ---*/

/* Macro to set the next state to enter. This macro is used for initialization
 * and also for state transition. For state transition HSM_STATE_TRAN has to be
 * used too.
 * Argument:	obj		pointer to AO
 * 				st		pointer to state function
 * 				lvl		nesting level (0 is the topmost state)
 */
#define HSM_SET_STATE(obj, st, lvl) \
		do{ \
			((struct aoHsm *) obj)->state[lvl].nextFunc = (hsmState_t) st; \
		}while(0)

/* Macro to initiate a state transition.
 * Argument:	e		event pointer found in the state function arguments
 *				lvl		how many nesting levels to exit starting at the current
 *						level. Hence with lvl=1 the current state and its sub-
 *						states are left.
 */
#define HSM_STATE_TRAN(e, lvl) \
		do{ \
			e->sig = STATE_TRAN_SIG; \
			e->data = lvl; \
		}while(0)

/* Macro to initiate a local state transition. In case of a local transition,
 * the INIT_SIG is thrown to the first state that is neither entered nor exited.
 * Argument:	e		event pointer found in the state function arguments
 *				lvl		how many nesting levels to exit starting at the current
 *						level. Hence with lvl=1 the current state and its sub-
 *						states are left.
 */
#define HSM_STATE_TRAN_LOCAL(e, lvl) \
		do{ \
			e->sig = STATE_TRAN_SIG; \
			e->data = lvl | TRAN_LOCAL_M; \
		}while(0)

/* Macro to tell that the event has been handled. It should always be used,
 * even if you know that the event doesn't appear in any super-state, because
 * it allows the dispatcher to quit.
 * Argument:	e		event pointer found in the state function arguments
 */
#define HSM_EVENT_HANDLED(e) \
		do{ \
			e->sig = EVENT_HANDLED_SIG; \
		}while(0)

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
extern void ao_scheduler(void);
extern int32_t ao_init_event_queue(struct ao *, struct event *, uint8_t);
extern int32_t ao_init_hsm_state_memory(struct aoHsm *, struct hsmState *,
                                        uint8_t);
extern int32_t ao_register(struct ao *, uint32_t, bool);
extern void ao_post(struct ao *, struct event *);
extern void ao_dispatch(struct ao *, struct event *);


#endif /* SOURCE_LIB_STM_AOK_H_ */

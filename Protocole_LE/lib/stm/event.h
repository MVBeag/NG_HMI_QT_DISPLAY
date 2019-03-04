/******************************************************************************
 * Copyright	: (c) Bron Elektronik AG
 * Project		:
 * File			: event.h
 * Date			: 25.07.2017
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
 *  
 *****************************************************************************/

#ifndef EVENT_H_
#define EVENT_H_


/******************************************************************************
 * DEFINES
 *****************************************************************************/
#ifndef NULL
#define NULL 0
#endif

/* I prefer enum over defines because they are highlighted by the editor. */
enum{
	NO_SIG = -6,
	STATE_TRAN_SIG,
	STATE_ENTRY_SIG,
	STATE_INIT_SIG,
	EVENT_HANDLED_SIG,
	STATE_EXIT_SIG,
	FIRST_USER_SIG,  /* 0 */
};

enum{
	LVL0,  /* level 0 indicates a top level state */
	LVL1,
	LVL2,
	LVL3,
	LVL4,
};

/**/
#define TRAN_LOCAL_M		0x8000

/******************************************************************************
 * MACROS
 *****************************************************************************/

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/* -- -- */
/* Here is the pattern one can use for creating an active object (replace x):

	// for a simple state machine
	struct xObj{
		struct{
			int32_t (*func)(struct xObj*, event_t*);
		}state;
		x_t x;
		...
	};

	// for a hierarchical state machine
	#define OBJNAME_STATE_NESTING	2  // for 2 levels
	struct xObj{
		struct{
			int32_t (*func)(struct xObj*, event_t*);
			int32_t (*nextFunc)(struct xObj*, event_t*);
			uint32_t flags;
		}state[OBJNAME_STATE_NESTING];
		x_t x;
		...
	};

	static struct xObj self;  // create file scope variable that is the object
	*self.state.func(self, event);  // call state function
*/

/* -- event -- */
/* The event type. Originally I only had a 'sig' and a 'data' field to keep
 * memory footprint small and get the best performance from assignments, since
 * the type became 4 bytes or one word. As the project moved on, it became more
 * and more troublesome that I wasn't able to store a reference to anything
 * within the event. A typical example is an object that gets events from other
 * objects over a callback. The called back function doesn't know the object then!
 * (Unless it's a singleton object, what I call a Task). For that very reason, I
 * added a field called obj of type 'void pointer', where you can add a reference
 * to anything you like.
 * Concluding, we do have an event type that is now 8 bytes wide, hence doubled
 * memory usage. Considering the performance, we do have a very slightly slower
 * memcpy performance (maybe 2 steps added out of ~30). */
struct event{
	int16_t sig;  /* typically an enumeration. Negative # reserved! */
	uint16_t data;  /* variable for data or an offset to a memory structure */
	void *obj;  /* void pointer to store a reference to anything */
};

#endif /* EVENT_H_ */

#include "common.h"

_Context *schedule(_Context *prev);
_Context *do_syscall(_Context *c);

static _Context *do_event(_Event e, _Context *c) {
	switch (e.event) {
		//case _EVENT_YIELD: printf("This is a yield event.\n"); break;
	case _EVENT_YIELD:
		return schedule(c);
		break;
	case _EVENT_SYSCALL:
		do_syscall(c);
		break;
	default:
		panic("Unhandled event ID = %d", e.event);
	}

	return NULL;
	return NULL;
}

void init_irq(void) {
	Log("Initializing interrupt/exception handler...");
	_cte_init(do_event);
}

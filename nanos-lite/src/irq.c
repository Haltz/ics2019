#include "common.h"

static _Context *do_event(_Event e, _Context *c) {
	switch (e.event) {
	//case _EVENT_YIELD: printf("This is a yield event.\n"); break;
	case _EVENT_YIELD:
		printf("This is a yield event.\n");
		break;
	case _EVENT_SYSCALL:
		printf("This is a syscall event.\n");
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

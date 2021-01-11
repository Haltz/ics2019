#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
	rtl_push(&cpu.eflags.val);
	t0 = cpu.cs;
	rtl_push(&t0);
	rtl_push(&ret_addr);
	cpu.eflags.IF = 0;
}

bool isa_query_intr(void) {
	return false;
}

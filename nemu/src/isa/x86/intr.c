#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
	rtl_push(&cpu.eflags.val);
	t0 = cpu.cs;
	rtl_push(&t0);
	rtl_push(&ret_addr);
	cpu.eflags.IF = 0;

	GateDesc gatedesc;
	gatedesc.offset_15_0 = (vaddr_read(cpu.idtr.base + NO * 8, 4) & 0x0000ffff);
	gatedesc.offset_31_16 = ((vaddr_read(cpu.idtr.base + NO * 8 + 4, 4) & 0xffff0000) >> 16);

	vaddr_t goalAddr = gatedesc.offset_15_0 | (gatedesc.offset_31_16 & 0x0000ffff) << 16;
	rtl_j(goalAddr);
}

bool isa_query_intr(void) {
	return false;
}

#include "cpu/exec.h"

make_EHelper(lidt) {
	cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
	if (decinfo.isa.is_operand_size_16) {
		cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4) && 0x00ffffff;
	} else {
		cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);
	}

	print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
	if (id_dest->reg == 0)
		cpu.cr0.val = id_src->val;
	else if (id_dest->reg == 3)
		cpu.cr3.val = id_src->val;
	else
		assert("Control register doesn't exist!\n");

	print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
	if (id_src->reg == 0)
		rtl_li(&s0, cpu.cr0.val);
	else if (id_src->reg == 3)
		rtl_li(&s0, cpu.cr3.val);
	else
		assert("Control register doesn't exist!\n");
	operand_write(id_dest, &s0);

	print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

	difftest_skip_ref();
}

make_EHelper(int) {
	switch (decinfo.opcode) {
	case 0xcc:
		raise_intr((uint32_t)0x3, decinfo.seq_pc);
		break;
	case 0xcd:
		raise_intr(id_dest->val, decinfo.seq_pc);
		break;
	case 0xce:
		raise_intr((uint32_t)0x4, decinfo.seq_pc);
		break;
	default:
		raise_intr(id_dest->val, decinfo.seq_pc);
		break;
	}

	print_asm("int %s", id_dest->str);

	difftest_skip_dut(1, 2);
}

make_EHelper(iret) {
	rtl_pop(&decinfo.jmp_pc);
	rtl_j(decinfo.jmp_pc);
	rtl_pop(&cpu.cs);
	rtl_pop(&cpu.eflags.val);

	print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void	 pio_write_l(ioaddr_t, uint32_t);
void	 pio_write_w(ioaddr_t, uint32_t);
void	 pio_write_b(ioaddr_t, uint32_t);

make_EHelper(in) {
	switch (id_src->width) {
	case 1:
		s0 = pio_read_b(id_src->val);
		break;
	case 2:
		s0 = pio_read_w(id_src->val);
		break;
	case 4:
		s0 = pio_read_l(id_src->val);
		break;
	default:
		break;
	}
	operand_write(id_dest, &s0);

	print_asm_template2(in);
}

make_EHelper(out) {
	switch (id_src->width) {
	case 1:
		pio_write_b(id_dest->val, id_src->val);
		break;
	case 2:
		pio_write_w(id_dest->val, id_src->val);
		break;
	case 4:
		pio_write_l(id_dest->val, id_src->val);
		break;
	default:
		break;
	}

	print_asm_template2(out);
}

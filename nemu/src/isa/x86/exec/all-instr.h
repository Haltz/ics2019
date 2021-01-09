#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);
make_EHelper(ret);

make_EHelper(push);
make_EHelper(pop);

make_EHelper(sub);
make_EHelper(add); // un

make_EHelper(xor);
make_EHelper(or); // un
make_EHelper(adc); //un
make_EHelper(sbb); // un
make_EHelper(and);
make_EHelper(cmp); // un

make_EHelper(lea);
make_EHelper(nop);

make_EHelper(setcc);
make_EHelper(mov);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(mov_r2cr);
make_EHelper(mov_cr2r);

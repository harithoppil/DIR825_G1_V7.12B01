#include <linux/module.h>
#include <asm/tc3162/tc3162.h>
#include <linux/netdevice.h>
#include <linux/mii.h>

#include "fe_api.h"

#ifdef CONFIG_MT751X
#define FE_BASE    0xBFB50000
#else
#error "Pls define the FE_BASE address!"
#endif

#define read_reg_word(reg)    regRead32(reg)
#define write_reg_word(reg, wdata)     regWrite32(reg, wdata)

unsigned int fe_reg_read(unsigned int reg_offset){
	return read_reg_word(FE_BASE + reg_offset);

}
void fe_reg_write(unsigned int reg_offset, unsigned int value){
	write_reg_word((FE_BASE + reg_offset), value);
	return;
}
void fe_reg_modify_bits(unsigned int reg_offset, unsigned int Data, unsigned int Offset, unsigned int Len)
{
	unsigned int Mask = 0;
	unsigned int Value;
	unsigned int i;

	for (i = 0; i < Len; i++) {
		Mask |= 1 << (Offset + i);
	}

	Value = fe_reg_read(reg_offset);
	Value &= ~Mask;
	Value |= (Data << Offset) & Mask;;

	fe_reg_write(reg_offset, Value);
}

EXPORT_SYMBOL(fe_reg_read);
EXPORT_SYMBOL(fe_reg_write);
EXPORT_SYMBOL(fe_reg_modify_bits);

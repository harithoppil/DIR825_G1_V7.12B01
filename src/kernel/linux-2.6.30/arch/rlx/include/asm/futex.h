/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2006  Ralf Baechle (ralf@linux-mips.org)
 */
#ifndef _ASM_FUTEX_H
#define _ASM_FUTEX_H

#ifdef __KERNEL__

#include <linux/futex.h>
#include <linux/uaccess.h>
#include <asm/barrier.h>
#include <asm/errno.h>
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181) || defined(CONFIG_CPU_RLX5281)
#define LL_DELAYSLOT	"nop\n"
#else
#define LL_DELAYSLOT	"\n"
#endif

#ifdef CONFIG_CPU_HAS_LLSC
#define __futex_atomic_op(insn, ret, oldval, uaddr, oparg)		\
{									\
		__asm__ __volatile__(					\
		"	.set	push				\n"	\
		"	.set	noat				\n"	\
		"1:	ll	    %1, %4	# __futex_atomic_op	\n"	\
		LL_DELAYSLOT						\
		"	" insn	"				\n"	\
		"2:	sc	    $1, %2				\n"	\
		"	beqz	$1, 1b				\n"	\
		__WEAK_LLSC_MB						\
		"3:						\n"	\
		"	.set	pop				\n"	\
		"	.section .fixup,\"ax\"			\n"	\
		"4:	li	    %0, %6				\n"	\
		"	j	    3b				\n"	\
		"	.previous				\n"	\
		"	.section __ex_table,\"a\"		\n"	\
		"	"__UA_ADDR "\t1b, 4b			\n"	\
		"	"__UA_ADDR "\t2b, 4b			\n"	\
		"	.previous				\n"	\
		: "=r" (ret), "=&r" (oldval), "=R" (*uaddr)		\
		: "0" (0), "R" (*uaddr), "Jr" (oparg), "i" (-EFAULT)	\
		: "memory");						\
}
#else
#define __futex_atomic_op(insn, ret, oldval, uaddr, oparg)		\
{									\
	ret = -ENOSYS;						\
}
#endif

static inline int
futex_atomic_op_inuser(int encoded_op, int __user *uaddr)
{
	int op = (encoded_op >> 28) & 7;
	int cmp = (encoded_op >> 24) & 15;
	int oparg = (encoded_op << 8) >> 20;
	int cmparg = (encoded_op << 20) >> 20;
	int oldval = 0, ret;
	if (encoded_op & (FUTEX_OP_OPARG_SHIFT << 28))
		oparg = 1 << oparg;

	if (! access_ok (VERIFY_WRITE, uaddr, sizeof(int)))
		return -EFAULT;

	pagefault_disable();

	switch (op) {
	case FUTEX_OP_SET:
		__futex_atomic_op("move	$1, %z5", ret, oldval, uaddr, oparg);
		break;

	case FUTEX_OP_ADD:
		__futex_atomic_op("addu	$1, %1, %z5",
		                  ret, oldval, uaddr, oparg);
		break;
	case FUTEX_OP_OR:
		__futex_atomic_op("or	$1, %1, %z5",
		                  ret, oldval, uaddr, oparg);
		break;
	case FUTEX_OP_ANDN:
		__futex_atomic_op("and	$1, %1, %z5",
		                  ret, oldval, uaddr, ~oparg);
		break;
	case FUTEX_OP_XOR:
		__futex_atomic_op("xor	$1, %1, %z5",
		                  ret, oldval, uaddr, oparg);
		break;
	default:
		ret = -ENOSYS;
	}

	pagefault_enable();

	if (!ret) {
		switch (cmp) {
		case FUTEX_OP_CMP_EQ: ret = (oldval == cmparg); break;
		case FUTEX_OP_CMP_NE: ret = (oldval != cmparg); break;
		case FUTEX_OP_CMP_LT: ret = (oldval < cmparg); break;
		case FUTEX_OP_CMP_GE: ret = (oldval >= cmparg); break;
		case FUTEX_OP_CMP_LE: ret = (oldval <= cmparg); break;
		case FUTEX_OP_CMP_GT: ret = (oldval > cmparg); break;
		default: ret = -ENOSYS;
		}
	}
	return ret;
}

static inline int
futex_atomic_cmpxchg_inatomic(int __user *uaddr, int oldval, int newval)
{
	int retval;

	if (!access_ok(VERIFY_WRITE, uaddr, sizeof(int)))
		return -EFAULT;

#ifdef CONFIG_CPU_HAS_LLSC
		__asm__ __volatile__(
		"# futex_atomic_cmpxchg_inatomic			\n"
		"	.set	push					\n"
		"	.set	noat					\n"
		"1:	ll	%0, %2					\n"
		LL_DELAYSLOT
		"	bne	%0, %z3, 3f				\n"
		"	move	$1, %z4					\n"
		"2:	sc	$1, %1					\n"
		"	beqz	$1, 1b					\n"
		__WEAK_LLSC_MB
		"3:							\n"
		"	.set	pop					\n"
		"	.section .fixup,\"ax\"				\n"
		"4:	li	%0, %5					\n"
		"	j	3b					\n"
		"	.previous					\n"
		"	.section __ex_table,\"a\"			\n"
		"	"__UA_ADDR "\t1b, 4b				\n"
		"	"__UA_ADDR "\t2b, 4b				\n"
		"	.previous					\n"
		: "=&r" (retval), "=R" (*uaddr)
		: "R" (*uaddr), "Jr" (oldval), "Jr" (newval), "i" (-EFAULT)
		: "memory");
#else
		return -ENOSYS;
#endif

	return retval;
}

#endif
#endif /* _ASM_FUTEX_H */

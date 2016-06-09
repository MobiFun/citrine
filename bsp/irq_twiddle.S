/*
 * TI's L1 code (and possibly other parts of the software suite) calls
 * functions named INT_DisableIRQ() and INT_EnableIRQ(); in the Leonardo
 * fw they are implemented (in a very ugly manner, as usual) in the
 * int.s entry point assembly module.  Here we implement the same
 * functions in a cleaner way.
 */

#include "../nucleus/asm_defs.h"

	.code 32
	.text

	.globl	INT_DisableIRQ
INT_DisableIRQ:
	mrs	r0, CPSR
	orr	r0, r0, #LOCKOUT
	msr	CPSR, r0
	bx	lr

	.globl	INT_EnableIRQ
INT_EnableIRQ:
	mrs	r0, CPSR
	bic	r0, r0, #LOCKOUT
	msr	CPSR, r0
	bx	lr

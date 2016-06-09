#include "../include/config.h"
#include "ns16550.h"

/* this module implements the elementary serial I/O operations */

	.text
	.code	32
	.globl	lldbg_serial_out
lldbg_serial_out:
	ldr	r2, =CONFIG_LLDBG_UART_BASE
1:	ldrb	r3, [r2, #NS16550_LSR]
	tst	r3, #NS16550_LSR_THRE
	beq	1b
	strb	r0, [r2, #NS16550_THR]
	bx	lr

	.globl	lldbg_serial_in_poll
lldbg_serial_in_poll:
	ldr	r2, =CONFIG_LLDBG_UART_BASE
	ldrb	r3, [r2, #NS16550_LSR]
	tst	r3, #NS16550_LSR_DR
	ldrneb	r0, [r2, #NS16550_RBR]
	mvneq	r0, #0
	bx	lr

#if 0	// not needed currently
	.globl	lldbg_serial_flush
lldbg_serial_flush:
	ldr	r2, =CONFIG_LLDBG_UART_BASE
1:	ldrb	r3, [r2, #NS16550_LSR]
	tst	r3, #NS16550_LSR_TEMP
	beq	1b
	bx	lr
#endif

/*
 * This ARM implementation of bzero() has been derived from:
 *
 *  linux/arch/arm/lib/memzero.S
 *
 *  Copyright (C) 1995-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

	.text
	.code	32
	.globl	bzero

/*
 * Align the pointer in r0.  r3 contains the number of bytes that we are
 * mis-aligned by, and r1 is the number of bytes.  If r1 < 4, then we
 * don't bother; we use byte stores instead.
 */
1:	subs	r1, r1, #4		@ 1 do we have enough
	blt	5f			@ 1 bytes to align with?
	cmp	r3, #2			@ 1
	strltb	r2, [r0], #1		@ 1
	strleb	r2, [r0], #1		@ 1
	strb	r2, [r0], #1		@ 1
	add	r1, r1, r3		@ 1 (r1 = r1 - (4 - r3))
/*
 * The pointer is now aligned and the length is adjusted.  Try doing the
 * bzero again.
 */

bzero:
	mov	r2, #0			@ 1
	ands	r3, r0, #3		@ 1 unaligned?
	bne	1b			@ 1
/*
 * r3 = 0, and we know that the pointer in r0 is aligned to a word boundary.
 */
3:	subs	r1, r1, #4
	strge	r2, [r0], #4
	bgt	3b			@ 1
	bxeq	lr			@ 1/2 quick exit
/*
 * No need to correct the count; we're only testing bits from now on
 *
 * When we get here, we've got less than 4 bytes to zero.  We
 * may have an unaligned pointer as well.
 */
5:	tst	r1, #2			@ 1 2 bytes or more?
	strneb	r2, [r0], #1		@ 1
	strneb	r2, [r0], #1		@ 1
	tst	r1, #1			@ 1 a byte left over
	strneb	r2, [r0], #1		@ 1
	bx	lr			@ 1

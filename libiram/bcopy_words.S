/*
 * bcopy_aligned_words() takes the same arguments as BSD bcopy(),
 * but requires both addresses and the count to be 32-bit word-aligned,
 * and assumes that the source and destination memory regions
 * do not overlap.  Furthermore, the count is expected to be non-zero.
 *
 * Arguments:
 *
 * R0: source address (word-aligned)
 * R1: destination address (ditto)
 * R2: byte count (must be a multiple of 4)
 */

	.text
	.code	32

	.globl	bcopy_aligned_words
bcopy_aligned_words:
1:	ldr	r3, [r0], #4
	str	r3, [r1], #4
	subs	r2, r2, #4
	bhi	1b
	bx	lr

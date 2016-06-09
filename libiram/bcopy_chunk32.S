/*
 * bcopy_32byte_chunks() takes the same arguments as BSD bcopy(),
 * but works on large chunks, specifically:
 *
 * - both addresses must be 32-bit word-aligned;
 * - the count must be a non-zero multiple of 32 bytes;
 * - the source and destination memory regions must not overlap.
 *
 * Arguments:
 *
 * R0: source address (word-aligned)
 * R1: destination address (ditto)
 * R2: byte count (must be a multiple of 32)
 */

	.text
	.code	32

	.globl	bcopy_32byte_chunks
bcopy_32byte_chunks:
	stmfd	sp!, {r4-r9}
1:	ldmia	r0!, {r3-r9,r12}
	stmia	r1!, {r3-r9,r12}
	subs	r2, r2, #0x20
	bhi	1b
	ldmfd	sp!, {r4-r9}
	bx	lr

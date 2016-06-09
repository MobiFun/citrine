/*
 * These 7 branch instructions, corresponding to ARM exception and interrupt
 * vectors, will be placed in different sections depending on the flashImage
 * vs. ramImage configuration and which target we build for; this little
 * snippet file will be #included where it is needed.
 */

	b	_arm_undefined
	b	_arm_swi
	b	_arm_abort_prefetch
	b	_arm_abort_data
	b	_arm_reserved
	b	_INT_IRQ
	b	_INT_FIQ

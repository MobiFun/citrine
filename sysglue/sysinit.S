/*
 * This assembly module, which puts bits of code into several different
 * sections, contains those essential entry point etc bits which are common
 * to both the regular flashable build and the fc-xram development build.
 */

	.section	iram.vectors,"ax",%progbits
	.code	32
#include "vectors.S"

	.section	xip.text,"ax",%progbits
	.code	32
	.globl	_FlashorXram_entry
_FlashorXram_entry:
/* enable Calypso boot ROM for the ARM exception and IRQ/FIQ vectors */
	ldr	r1, =0xFFFFFB10
	mov	r2, #0x0100
	strh	r2, [r1]
/* now we can call any function w/o worry as to where it lives */
/* clear int.bss and ext.bss, using bzero() from libc */
	ldr	r0, =__intbss_start
	ldr	r1, =__intbss_size
	bl	bzero
	ldr	r0, =__extbss_start
	ldr	r1, =__extbss_size
	bl	bzero
/* we can start Nucleus now! */
	b	INT_Initialize

/*
 * riviera/support/exception.c references exit()
 * The following kludge will allow us to pass the link for the time being
 * until we can find a better solution, such as hooking into DAR maybe.
 */

	.globl	exit
exit:
	b	.

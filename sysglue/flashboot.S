/*
 * This assembly module contains those bits which are specific to flashable
 * image builds only. It puts bits of code into several different sections.
 */

#include "../include/config.h"

#if FLASH_BOOT_VIA_BOOTROM
/*
 * Put something sensible in the boot ROM overlay area, just for the
 * heck of it, or for extra robustness.
 */
	.section	bootrom.overlay,"ax",%progbits
	.code	32
	.org	0
	b	BootROM_disabled_entry
#include "vectors.S"
BootROM_disabled_entry:
/* copy the boot ROM switch code to IRAM and jump to it */
	ldr	r4, =__romswitch_flash_addr
	ldr	r5, =__romswitch_ram_addr
	ldr	r2, =__romswitch_size
1:	ldr	r0, [r4], #4
	str	r0, [r5], #4
	subs	r2, r2, #4
	bhi	1b
	ldr	pc, =__romswitch_ram_addr

	.section	bootrom.switch,"ax",%progbits
	.code	32
	.org	0
@ enable the Calypso boot ROM
	ldr	r1, =0xFFFFFB10
	mov	r2, #0x0100
	strh	r2, [r1]
@ jump to it!
	mov	pc, #0
#endif

	.section	flashboot.text,"ax",%progbits
	.code	32
	.org	0
#if FLASH_BOOT_VIA_BOOTROM
/* sane targets with Calypso boot ROM enabled by the PCB wiring */
/* provide the necessary magic words for the boot ROM */
	.word	0
	.word	_Flash_boot_entry
#elif CONFIG_TARGET_C139 || CONFIG_TARGET_C11X
/*
 * On this target we'll put a patched version of Compal's boot code in
 * flash sector 0 (the brickable one); the main fw images will then be
 * flashed starting at 0x10000, which is where our modified boot code
 * expects them to be.  The interface between our hacked boot code and
 * the main fw has been made to mimic TI's TCS211 reference fw.
 */
#include "vectors.S"
	.org	0x58	/* put _Flash_boot_entry at 0x10058 */
#elif CONFIG_TARGET_C155
/*
 * On this target the hand-off point between the bootloader and the main
 * fw image coincides with a flash erase block boundary, thus we can reuse
 * the original bootloader without having to reflash the brickable sector
 * at all.  The following bits will appear at 0x20000.
 */
	.asciz	"FreeCalypso firmware for C155/156 target"
	.org	0xE0
/* C155/156 bootloader jumps here */
	b	_Flash_boot_entry
#include "vectors.S"
#else
#error "Unsupported flash boot configuration"
#endif

	.globl	_Flash_boot_entry
_Flash_boot_entry:
/* first order of business: configure flash and XRAM access */
	ldr	r2, =0xFFFFFB00
#if CONFIG_TARGET_PIRELLI
	mov	r0, #0x00A4
	strh	r0, [r2, #0]
	strh	r0, [r2, #2]
	strh	r0, [r2, #6]
#elif CONFIG_TARGET_FCFAM
	mov	r0, #0x00A4
	strh	r0, [r2, #0]
	strh	r0, [r2, #2]
	strh	r0, [r2, #4]
#else
	mov	r0, #0x00A3
	strh	r0, [r2, #0]
	strh	r0, [r2, #2]
#endif
/*
 * On targets with 8 MiB flash (or XRAM) banks we need to switch the
 * CS4/ADD22 pin to the ADD22 function.  We could do it for all targets
 * (not aware of any that need the pin to be CS4), but we strongly
 * prefer to have a consistent execution environment between flashImage
 * and ramImage, so let's only do this setting on those targets
 * on which we do it in ../../loadtools/scripts/*.init (for fc-xram).
 */
#if CONFIG_TARGET_PIRELLI || CONFIG_TARGET_C155 || CONFIG_TARGET_FCFAM
	ldr	r1, =0xFFFEF006
	mov	r2, #0x0008
	strh	r2, [r1]
#endif
/* we now have full access to all flash and RAM on the device */
/* copy iram.text to where it's supposed to be */
	ldr	r8, =__iramtext_flash_addr
	ldr	r9, =__iramtext_ram_addr
	ldr	r10, =__iramtext_size
1:	ldmia	r8!, {r0-r7}
	stmia	r9!, {r0-r7}
	subs	r10, r10, #0x20
	bhi	1b
/* likewise copy .data from flash to XRAM */
	ldr	r8, =__initdata_flash_addr
	ldr	r9, =__initdata_ram_addr
	ldr	r10, =__initdata_size
1:	ldmia	r8!, {r0-r7}
	stmia	r9!, {r0-r7}
	subs	r10, r10, #0x20
	bhi	1b
/* boot path joins with fc-xram loadable builds */
	b	_FlashorXram_entry

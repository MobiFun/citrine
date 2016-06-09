/*
 * TI's TCS211 firmware is designed to autodetect the flash chip type for
 * its FFS, selecting different FFS configurations depending on what flash
 * chip is found.  In our FreeCalypso fw we fix our FFS configuration at
 * compile time instead (for us it's more a matter of either mimicking or
 * working around what the previous firmwares on our target devices do),
 * so we have no "proper" place for flash chip type autodetection logic in
 * our fw architecture.
 *
 * However, the following function (to be executed out of IRAM) mimics the
 * sequence of flash chip accesses performed by TCS211's flash chip type
 * autodetection function.  It was added in a stray hope that it might fix
 * the perplexing crash behavior we see when booting on a GTA02 modem that
 * is "hot", i.e., powered off and then back on without giving it a long
 * time to "cool".
 */

	.code 32
	.text

	.globl	flash_chip_init
flash_chip_init:
	ldr	r1, =0x0300AAAA
	ldr	r2, =0x03005555
	ldr	r3, =0x03000000
/* read ID mode sequence */
	mov	r0, #0xAA
	strh	r0, [r1]
	mov	r0, #0x55
	strh	r0, [r2]
	mov	r0, #0x90
	strh	r0, [r1]
/* dummy reads */
	ldrh	r0, [r3, #0]
	ldrh	r0, [r3, #2]
/* Intel reset command */
	mov	r0, #0xFF
	strh	r0, [r3]
/* AMD reset sequence */
	mov	r0, #0xAA
	strh	r0, [r1]
	mov	r0, #0x55
	strh	r0, [r2]
	mov	r0, #0xF0
	strh	r0, [r1]
	bx	lr

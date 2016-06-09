/*
 * Assembly code extracted out of TI's l1_small.c
 *
 * This code is correct ONLY for CHIPSET 10 or 11 as currently used
 * by FreeCalypso; see TI's original code for what changes would be
 * needed to support other CHIPSETs.
 */

	.text
	.code 32

/*-------------------------------------------------------*/ 
/* _GSM_Small_Sleep                                      */
/* (formerly INT_Small_Sleep)                            */
/*-------------------------------------------------------*/
/*                                                       */
/* Description: small sleep                              */
/* ------------                                          */
/* Called by TCT_Schedule main loop of Nucleus           */
/*-------------------------------------------------------*/

#define	SMALL_SLEEP	0x01
#define	ALL_SLEEP	0x04
#define	PWR_MNGT	0x01

	.globl	_GSM_Small_Sleep
_GSM_Small_Sleep:

	ldr	r0,Switch
	ldr	r0,[r0]
	ldrb	r1,[r0]
	cmp	r1,#PWR_MNGT
	bne	TCT_Schedule_Loop

	ldr	r0,Mode
	ldr	r0,[r0]
	ldrb	r1,[r0]
	cmp	r1,#SMALL_SLEEP
	beq	Small_sleep_ok
	cmp	r1,#ALL_SLEEP
	bne	TCT_Schedule_Loop

Small_sleep_ok:

// *****************************************************
//reset the DEEP_SLEEP bit 12 of CNTL_ARM_CLK register
// (Cf BUG_1278)

	ldr r0,addrCLKM			@ pick up CNTL_ARM_CLK register address
	ldrh r1,[r0]			@ take the current value of the register
	orr  r1,r1,#0x1000		@ reset the bit
	strh r1,[r0]			@ store the result

	ldr	r0,addrCLKM		@ pick up CLKM clock register address
	ldrh	r1,[r0]			@ take the current value of the register
	bic	r1,r1,#1		@ disable ARM clock
	strh	r1,[r0]

	B	TCT_Schedule_Loop	@ Return to TCT_Schedule main loop

addrCLKM:   	.word	0xfffffd00	@ CLKM clock register address

Mode:           .word   mode_authorized
Switch:         .word   switch_PWR_MNGT

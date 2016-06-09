/*
 * Definitions for Calypso general-purpose timer registers
 * Added to the FreeNucleus Calypso port by Spacefalcon the Outlaw.
 *
 * This header is usable from both .c and .S source files.
 */

#ifndef _CALYPSO_TIMER_H
#define _CALYPSO_TIMER_H

#define	TIMER1_BASE_ADDR	0xFFFE3800
#define	TIMER2_BASE_ADDR	0xFFFE6800

#ifdef __ASSEMBLER__

/*
 * Assembly source with cpp
 *
 * The most convenient way to access registers like these from ARM
 * assembly is to load the base address of the register block in some
 * ARM register, using only one ldr rN, =xxx instruction and only one
 * literal pool entry, and then access various registers in the block
 * from the same base using the immediate offset addressing mode.
 *
 * Here we define the offsets for the usage scenario above.
 */

#define	CNTL_TIM	0x00
#define	LOAD_TIM	0x02
#define	READ_TIM	0x04

#else

/*
 * C source
 *
 * For access from C, we define the layout of each timer register block
 * as a struct, and then define a pleudo-global-var for easy "volatile"
 * access to each of the 2 timers.
 */

struct timer_regs {
	unsigned char	cntl;
	unsigned char	pad;
	unsigned short	load;
	unsigned short	read;
};

#define	TIMER1_REGS	(*(volatile struct timer_regs *) TIMER1_BASE_ADDR)
#define	TIMER2_REGS	(*(volatile struct timer_regs *) TIMER2_BASE_ADDR)

#endif

/* CNTL register bit definitions */
#define	CNTL_START		0x01
#define	CNTL_AUTO_RELOAD	0x02
#define	CNTL_CLOCK_ENABLE	0x20

#endif /* include guard */

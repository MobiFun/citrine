/*
 * FreeNucleus port by Spacefalcon the Outlaw
 *
 * This module implements the INT_Timer_Initialize() function
 * for the proof-of-concept Calypso port.
 *
 * TIMER2 configuration is based on that used by OsmocomBB.
 */

#include "calirq.h"
#include "caltimer.h"

void
INT_Timer_Initialize()
{
	/* program the timer */
	TIMER2_REGS.cntl = CNTL_CLOCK_ENABLE;
	TIMER2_REGS.load = 4062;
	TIMER2_REGS.cntl = CNTL_CLOCK_ENABLE | CNTL_AUTO_RELOAD | CNTL_START;
	/* now let it interrupt */
	INTH_REGS.ilr_irq[IRQ_TIMER2] = 0x7E;
	INTH_REGS.mask_it_reg1 &= ~(1 << IRQ_TIMER2);
}

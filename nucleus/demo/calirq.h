/*
 * Definitions for Calypso IRQ numbers and the related registers
 * Added to the FreeNucleus Calypso port by Spacefalcon the Outlaw.
 *
 * This header is usable from both .c and .S source files.
 */

#ifndef _CALYPSO_IRQ_H
#define _CALYPSO_IRQ_H

#define	IRQ_WATCHDOG		0
#define	IRQ_TIMER1		1
#define	IRQ_TIMER2		2
#define	IRQ_TSP_RX		3
#define	IRQ_TPU_FRAME		4
#define	IRQ_TPU_PAGE		5
#define	IRQ_SIMCARD		6
#define	IRQ_UART_MODEM		7
#define	IRQ_KEYPAD_GPIO		8
#define	IRQ_RTC_TIMER		9
#define	IRQ_RTC_ALARM_I2C	10
#define	IRQ_ULPD_GAUGING	11
#define	IRQ_EXTERNAL		12
#define	IRQ_SPI			13
#define	IRQ_DMA			14
#define	IRQ_API			15
#define	IRQ_SIM_DETECT		16
#define	IRQ_EXTERNAL_FIQ	17
#define	IRQ_UART_IRDA		18
#define	IRQ_ULPD_GSM_TIMER	19
#define	IRQ_GEA			20

#define	MAX_IRQ_NUM		20

#define	INTH_BASE_ADDR		0xFFFFFA00

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

#define	IT_REG1		0x00
#define	IT_REG2		0x02
#define	MASK_IT_REG1	0x08
#define	MASK_IT_REG2	0x0A
#define	IRQ_NUM		0x10
#define	FIQ_NUM		0x12
#define	IRQ_CTRL	0x14
#define	ILR_OFFSET	0x20

#else

/*
 * C source
 *
 * For access from C, we define the layout of the INTH register block
 * as a struct, and then define a pleudo-global-var for easy "volatile"
 * access.
 */

struct inth_regs {
	unsigned short	it_reg1;
	unsigned short	it_reg2;
	unsigned short	pad1[2];
	unsigned short	mask_it_reg1;
	unsigned short	mask_it_reg2;
	unsigned short	pad2[2];
	unsigned short	irq_num;
	unsigned short	fiq_num;
	unsigned short	irq_ctrl;
	unsigned short	pad3[5];
	unsigned short	ilr_irq[MAX_IRQ_NUM+1];
};

#define	INTH_REGS	(*(volatile struct inth_regs *) INTH_BASE_ADDR)

/*
 * C code can now access INTH registers like this:
 *
 * old_mask = INTH_REGS.mask_it_reg1;
 * INTH_REGS.mask_it_reg1 = new_mask;
 */

#endif

#endif /* _CALYPSO_IRQ_H */

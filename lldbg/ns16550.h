#ifndef __NS16550_H
#define	__NS16550_H

/* NS16550 registers */
#define	NS16550_RBR	0
#define	NS16550_THR	0
#define	NS16550_IER	1
#define	NS16550_IIR	2
#define	NS16550_FCR	2
#define	NS16550_LCR	3
#define	NS16550_MCR	4
#define	NS16550_LSR	5
#define	NS16550_MSR	6
#define	NS16550_SCR	7
#define	NS16550_DLL	0
#define	NS16550_DLM	1

#ifndef __ASSEMBLER__
#include "types.h"

struct ns16550_regs {
	u8	datareg;
	u8	ier;
	u8	iir_fcr;
	u8	lcr;
	u8	mcr;
	u8	lsr;
	u8	msr;
	u8	scr;
};
#endif

/* IER bits */
#define	NS16550_IER_EDSSI	0x08
#define	NS16550_IER_ELSI	0x04
#define	NS16550_IER_ETBEI	0x02
#define	NS16550_IER_ERBFI	0x01

/* IIR bits */
#define	NS16550_IIR_FIFOEN	0xC0
#define	NS16550_IIR_INTID	0x0E
#define	NS16550_IIR_INT_RLS	0x06
#define	NS16550_IIR_INT_RDA	0x04
#define	NS16550_IIR_INT_CTO	0x0C
#define	NS16550_IIR_INT_THRE	0x02
#define	NS16550_IIR_INT_MODEM	0x00
#define	NS16550_IIR_INTPEND	0x01

/* FCR bits */

#define	NS16550_FCR_RXTR	0xC0
#define	NS16550_FCR_RXTR_1	0x00
#define	NS16550_FCR_RXTR_4	0x40
#define	NS16550_FCR_RXTR_8	0x80
#define	NS16550_FCR_RXTR_14	0xC0
#define	NS16550_FCR_DMAMODE	0x08
#define	NS16550_FCR_TXRST	0x04
#define	NS16550_FCR_RXRST	0x02
#define	NS16550_FCR_FIFOEN	0x01

/* LCR bits */
#define	NS16550_LCR_DLAB	0x80
#define	NS16550_LCR_BREAK	0x40
#define	NS16550_LCR_STICK	0x20
#define	NS16550_LCR_EPS		0x10
#define	NS16550_LCR_PEN		0x08
#define	NS16550_LCR_STB		0x04
#define	NS16550_LCR_WLS		0x03
#define	NS16550_LCR_WLS_5	0x00
#define	NS16550_LCR_WLS_6	0x01
#define	NS16550_LCR_WLS_7	0x02
#define	NS16550_LCR_WLS_8	0x03

/* MCR bits */
#define	NS16550_MCR_LOOP	0x10
#define	NS16550_MCR_OUT2	0x08
#define	NS16550_MCR_OUT1	0x04
#define	NS16550_MCR_RTS		0x02
#define	NS16550_MCR_DTR		0x01

/* LSR bits */
#define	NS16550_LSR_ERR		0x80
#define	NS16550_LSR_TEMP	0x40
#define	NS16550_LSR_THRE	0x20
#define	NS16550_LSR_BI		0x10
#define	NS16550_LSR_FE		0x08
#define	NS16550_LSR_PE		0x04
#define	NS16550_LSR_OE		0x02
#define	NS16550_LSR_DR		0x01

/* MSR bits */
#define	NS16550_MSR_DCD		0x80
#define	NS16550_MSR_RI		0x40
#define	NS16550_MSR_DSR		0x20
#define	NS16550_MSR_CTS		0x10
#define	NS16550_MSR_DDCD	0x08
#define	NS16550_MSR_TERI	0x04
#define	NS16550_MSR_DDSR	0x02
#define	NS16550_MSR_DCTS	0x01

#endif	/* __NS16550_H */

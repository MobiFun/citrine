CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc -DDTILIB

OBJS=	uart_drxf.o uart_drxp.o uart_drxs.o \
	uart_dtxf.o uart_dtxp.o uart_dtxs.o \
	uart_kerf.o uart_kerp.o uart_kers.o uart_pei.o \
	uart_rtf.o uart_rtp.o uart_rts.o \
	uart_rxf.o uart_rxp.o uart_rxs.o \
	uart_txf.o uart_txp.o uart_txs.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

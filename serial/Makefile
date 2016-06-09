CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

sinclude ../include/config.mk

OBJS=	debug.o init.o serialswitch.o uart.o
ifeq (${CONFIG_FDMODEM},1)
OBJS+=	uartfax.o
endif

HDRS=	faxdata.h serialswitch.h traceswitch.h uart.h uartfax.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
ASFLAGS=-mthumb-interwork
LD=	arm-elf-ld

sinclude ../include/config.mk

IOBJS=	flashchipinit.o irq_twiddle.o niq32.o
XTOBJS=	armio.o clkm.o init_target.o inth.o niq.o timer.o timer1.o timer2.o

ifeq (${CONFIG_INCLUDE_SIM},1)
XTOBJS+=sim.o
IOBJS+=	sim32.o
endif

XOBJS=	${XTOBJS} oldint.o
AOBJS=	${IOBJS} oldint.o

HDRS=	armio.h clkm.h dma.h inth.h iq.h mem.h rhea_arm.h sim.h timer.h \
	timer1.h timer2.h ulpd.h

SUBDIR=	abb+spi rtc
SUBDIR_XIPCODE=	abb+spi/xipcode.o rtc/xipcode.o

all:	${IOBJS} ${XOBJS} ${SUBDIR} iramcode.o xipcode.o

${AOBJS}:	${HDRS}

${XTOBJS}:	%.o : %.c ${HDRS}
	${CC} ${CFLAGS} -mthumb -c $<

iramcode.o:	${IOBJS}
	${LD} -r -o $@ ${IOBJS}

xipcode.o:	${XOBJS} ${SUBDIR}
	${LD} -r -o $@ ${XOBJS} ${SUBDIR_XIPCODE}

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

clean:
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

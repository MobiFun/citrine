CC=	arm-elf-gcc
CPPFLAGS=-I../../include -I../include -I../audio_include -I../audio_cust0 \
	-I../tm_include -I../tm_cust0 -I../tpudrv
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
LD=	arm-elf-ld

sinclude ../../include/config.mk

IOBJS=	ind_os.o l1_cust_iram.o
XOBJS=	l1_cust_xip.o l1_rf12.o

ifeq (${CONFIG_TARGET_PIRELLI},1)
XOBJS+=	pirelli.o
endif

all:	iramcode.o xipcode.o

l1_cust_iram.o:	l1_cust.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DRUN_INT_RAM -c -o $@ $<

l1_cust_xip.o:	l1_cust.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DRUN_FLASH -mthumb -c -o $@ $<

ifeq (${CONFIG_TARGET_PIRELLI},1)
pirelli.o:	pirelli.c
	${CC} ${CFLAGS} ${CPPFLAGS} -mthumb -c $<
endif

iramcode.o:	${IOBJS}
	${LD} -r -o $@ ${IOBJS}

xipcode.o:	${XOBJS}
	${LD} -r -o $@ ${XOBJS}

clean:
	rm -f *.[oa] *.out *errs

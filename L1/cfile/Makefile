CC=	arm-elf-gcc
CPPFLAGS=-I../../include -I../include -I../audio_include -I../audio_cust0 \
	-I../dyn_dwl_include -I../tm_include -I../tm_cust0 -I../cust0 \
	-I../tpudrv -I../../bsp -I../../bsp/abb+spi -DMOVE_IN_INTERNAL_RAM
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
ASFLAGS=-mthumb-interwork
LD=	arm-elf-ld
INTSED=	../intram.sed

sinclude ../../include/config.mk

IOBJS=	l1_api_hisr.o l1_cmplx_intram.o l1_ctl.o l1_drive.o l1_func.o \
	l1_mfmgr.o l1_small_asm.o l1_sync_intram.o l1_isr_glue.o
XOBJS=	l1_afunc.o l1_async.o l1_cmplx.o l1_init.o l1_pwmgr.o l1_small_defs.o \
	l1_sync.o l1_trace.o dummy_tch_data.o

ifeq (${FEATURE_TCH_REROUTE},1)
XOBJS+=	tch_feature.o
endif

all:	iramcode.o xipcode.o

${XOBJS}:	%.o : %.c
	${CC} ${CFLAGS} ${CPPFLAGS} -mthumb -c $<

l1_cmplx_intram.c:	l1_cmplx.c ${INTSED}
	sed -n -f ${INTSED} l1_cmplx.c > $@

l1_sync_intram.c:	l1_sync.c ${INTSED}
	sed -n -f ${INTSED} l1_sync.c > $@

iramcode.o:	${IOBJS}
	${LD} -r -o $@ ${IOBJS}

xipcode.o:	${XOBJS}
	${LD} -r -o $@ ${XOBJS}

clean:
	rm -f *.[oa] l1_*_intram.c *.out *errs

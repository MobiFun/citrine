CC=	arm-elf-gcc
CPPFLAGS=-I../../include -I../include -I../audio_include -I../audio_cust0 \
	-I../dyn_dwl_include -I../tm_include -I../tm_cust0 -I../cust0 \
	-I../tpudrv -DMOVE_IN_INTERNAL_RAM
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
LD=	arm-elf-ld

XOBJS=	l1_dyn_dwl_afunc.o l1_dyn_dwl_apihisr.o l1_dyn_dwl_async.o \
	l1_dyn_dwl_func.o l1_dyn_dwl_init.o l1_dyn_dwl_sync.o

all:	xipcode.o

${XOBJS}:	%.o : %.c
	${CC} ${CFLAGS} ${CPPFLAGS} -mthumb -c $<

xipcode.o:	${XOBJS}
	${LD} -r -o $@ ${XOBJS}

clean:
	rm -f *.[oa] *.out *errs

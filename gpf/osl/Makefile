CC=	arm-elf-gcc
COPTS=	-O2 -fno-builtin -mthumb-interwork -I../inc
IOPTS=	-DRUN_INT_RAM
XOPTS=	-DRUN_FLASH -mthumb

IOBJS=	os_com_ir.o os_mem_ir.o os_mis_ir.o os_pro_ir.o \
	os_sem_ir.o os_tim_ir.o
XOBJS=	os_com_fl.o os_drv.o os_isr.o os_mem_fl.o os_mis_fl.o os_pro_fl.o \
	os_sem_fl.o os_tim_fl.o

all:	${IOBJS} ${XOBJS}

${IOBJS}:	%.o : %.c
	${CC} ${COPTS} ${IOPTS} -c $<

${XOBJS}:	%.o : %.c
	${CC} ${COPTS} ${XOPTS} -c $<

clean:
	rm -f *.[oa] *errs

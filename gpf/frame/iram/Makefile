CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
CPPFLAGS=-I../../inc -DRUN_INT_RAM
VPATH=	..

OBJS=	frame.o prf_func.o route.o \
	vsi_com.o vsi_drv.o vsi_mem.o vsi_mis.o vsi_ppm.o \
	vsi_pro.o vsi_sem.o vsi_tim.o vsi_trc.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

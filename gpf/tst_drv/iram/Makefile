CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
CPPFLAGS=-I../../inc -DRUN_INT_RAM
VPATH=	..

OBJS=	ser_tr.o tif2.o titrc.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

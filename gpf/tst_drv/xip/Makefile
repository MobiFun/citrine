CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../../inc -DRUN_FLASH
VPATH=	..

OBJS=	ser_tr.o tif2.o titrc.o tr2.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
CPPFLAGS=-I../../inc -DRUN_INT_RAM
VPATH=	..

OBJS=	tst_pei.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

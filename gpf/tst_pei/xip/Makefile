CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../../inc -DRUN_FLASH
VPATH=	..

OBJS=	tif_version.o tst_pei.o tstdriver.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

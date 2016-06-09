CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../../inc -DRUN_FLASH
VPATH=	..

OBJS=	misc_version.o tok.o tools.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

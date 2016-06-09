CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../nucleus

OBJS=	demo.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

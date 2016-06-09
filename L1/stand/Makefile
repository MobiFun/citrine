CC=	arm-elf-gcc
CPPFLAGS=-I../../gpf/inc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

OBJS=	l1_pei.o forwarder.o dll_stubs.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

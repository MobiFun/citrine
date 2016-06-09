CC=	arm-elf-gcc
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	audio.o ffs_coat.o power.o rtc.o rx.o

all:	libgdi.a

libgdi.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *.out *errs

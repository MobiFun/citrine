CC=	arm-elf-gcc
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	dti_int.o dti_kerf.o dti_kerp.o

all:	libdti.a

libdti.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *.out *errs

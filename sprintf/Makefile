CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

OBJS=	float.o integer.o sprintf.o strtok.o vspcore.o

all:	libsprintf.a

libsprintf.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *errs

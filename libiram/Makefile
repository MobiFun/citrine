CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
ASFLAGS=-mthumb-interwork
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

OBJS=	bcopy_chunk32.o bcopy_words.o bzero.o

all:	libiram.a

libiram.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *errs

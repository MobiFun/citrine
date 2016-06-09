CC=	arm-elf-gcc
CFLAGS=	-Os -fno-builtin -mthumb-interwork -mthumb
ASFLAGS=-mthumb-interwork
LD=	arm-elf-ld

OBJS=	appinit.o exceptions.o irqfiq.o sysinit.o

all:	xipcode.o flashboot.o

flashboot.o:	flashboot.S vectors.S
sysinit.o:	sysinit.S vectors.S

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

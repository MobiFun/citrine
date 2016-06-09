CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

OBJS=	pcmcode.o pcmdata.o

HDRS=	pcm.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

CC=	arm-elf-gcc
CPPFLAGS=-I../../include -I../include -I../audio_include -I../audio_cust0 \
	-I../tm_include -I../tm_cust0 -I../cust0
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
LD=	arm-elf-ld

OBJS=	tpudrv.o tpudrv12.o

all:	iramcode.o

iramcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

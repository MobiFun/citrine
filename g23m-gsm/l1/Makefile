CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

PEI_OBJ=l1_pei.o

all:	${PEI_OBJ}

clean:
	rm -f *.[oa] *.out *errs

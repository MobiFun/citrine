CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc -I../aci

ONE_OBJ=dti_conn_mng.o

all:	${ONE_OBJ}

clean:
	rm -f *.[oa] *.out *errs

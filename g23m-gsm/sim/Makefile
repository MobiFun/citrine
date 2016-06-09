CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	sim_app.o sim_em.o sim_fkt.o sim_pei.o sim_stk.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	alr_cbch.o alr_cs.o alr_csf.o alr_dedi.o alr_em.o alr_main.o alr_nc.o \
	alr_pch.o alr_pei.o alr_rach.o alr_tim.o alr_trc.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

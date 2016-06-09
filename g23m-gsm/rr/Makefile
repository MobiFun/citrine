CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	rr_attf.o rr_attp.o rr_atts.o rr_cs.o rr_csf.o rr_datf.o rr_datp.o \
	rr_dats.o rr_em.o rr_forf.o rr_forp.o rr_fors.o rr_pei.o rr_srv.o \
	rr_tim.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

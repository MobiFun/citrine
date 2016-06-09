CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc

OBJS=	mm_csf.o mm_em.o mm_forf.o mm_forp.o mm_fors.o mm_mmf.o mm_mmp.o \
	mm_mms.o mm_pei.o mm_regf.o mm_regp.o mm_regs.o mm_tim.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

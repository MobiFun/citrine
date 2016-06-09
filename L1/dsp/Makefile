CC=	arm-elf-gcc
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb

OBJS=	amr_mms_patch_file36_10.o amr_sch_patch_file36_10.o \
	e2_patch_file36_10.o gprs_patch_file36_10.o leadapi.o leadboot.o \
	patch_file36_10_dyn_dwl.o tty_patch_file36_10.o

all:	libdsp.a

libdsp.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *.out *errs

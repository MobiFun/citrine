CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../inc
LD=	arm-elf-ld

sinclude ../../include/config.mk

ifeq (${CONFIG_GSM},1)
CONFIG_MODULE=	gsmcomp
else
CONFIG_MODULE=	barecomp
endif

OBJS=	gpf_misc_init.o gpf_tst_drv.o ${CONFIG_MODULE}.o

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

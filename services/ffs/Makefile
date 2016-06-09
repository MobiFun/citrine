CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
LD=	arm-elf-ld

sinclude ../../include/config.mk

OBJS=	cfgffs.o core.o drv.o ffs_env.o ffs_target.o ffstrace.o fsck.o rand.o \
	reclaim.o task.o tmffs.o

HDRS=	core.h drv.h ffs.h ffs_api.h ffs_env.h ffs_pool_size.h ffstrace.h \
	intctl.h ramffs.h task.h tmffs.h

TARGETS=xipcode.o
ifeq (${FLASH_IS_INTEL_ONEBANK},1)
TARGETS+=intelsbdrv.o
endif

all:	${TARGETS}

${OBJS}:	%.o : %.c ${HDRS}
	${CC} ${CFLAGS} -mthumb -c $<

ffs_target.c:	ffs.c mktarget.pl
	./mktarget.pl

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

intelsbdrv.o:	intelsbdrv.c ${HDRS}
	${CC} ${CFLAGS} -c intelsbdrv.c

clean:
	rm -f *.[oa] *errs ffs_target.c

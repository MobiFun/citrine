CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

sinclude ../../include/config.mk

OBJS=	abb.o spi_api.o spi_drv.o spi_env.o spi_process.o spi_task.o

ifeq (${CONFIG_INCLUDE_L1},1)
OBJS+=	abb_core_inth.o abb_inth.o
endif

HDRS=	abb.h abb_core_inth.h abb_inline.h abb_inth.h spi_api.h spi_drv.h \
	spi_env.h spi_pool_size.h spi_process.h spi_task.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

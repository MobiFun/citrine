CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

OBJS=	etm_api.o etm_env.o etm_task.o etm_tmcore.o etm_trace.o

HDRS=	etm.h etm_api.h etm_at.h etm_audio_err.h etm_config.h etm_env.h \
	etm_messages_i.h etm_misc.h etm_pool_size.h etm_trace.h etm_version.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

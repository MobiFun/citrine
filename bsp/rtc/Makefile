CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

OBJS=	rtc_env.o rtc_functions.o rtc_handle_message.o rtc_process.o rtc_task.o

HDRS=	rtc_api.h rtc_config.h rtc_env.h rtc_i.h rtc_messages_i.h \
	rtc_pool_size.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

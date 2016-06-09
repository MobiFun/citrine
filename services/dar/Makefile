CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
LD=	arm-elf-ld

OBJS=	dar_api.o dar_diagnose.o dar_emergency.o dar_env.o dar_error_hdlr.o \
	dar_gbl_var.o dar_handle_message.o dar_msg_ft.o dar_task.o \
	dar_watchdog.o

HDRS=	dar_api.h dar_const_i.h dar_diagnose_i.h dar_emergency.h dar_env.h \
	dar_error_hdlr_i.h dar_gen.h dar_handle_message.h dar_macro_i.h \
	dar_messages_i.h dar_msg_ft.h dar_pool_size.h dar_structs_i.h \
	dar_watchdog.h

all:	xipcode.o

${OBJS}:	${HDRS}

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *errs

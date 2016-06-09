CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb

OBJS=	rvm_api.o rvm_group_swe.o rvm_mem.o rvm_swe.o rvm_swe_db.o \
	rvm_swe_hdlr.o rvm_task.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

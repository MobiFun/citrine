CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb

OBJS=	rvf_buffer.o rvf_mem_pool.o rvf_msg.o rvf_task.o rvf_time.o \
	rvf_trace_adapt.o

all:	${OBJS}

clean:
	rm -f *.[oa] *errs

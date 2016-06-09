CC=	arm-elf-gcc
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I../include -I../include/condat -I../gpf/inc -I../cdginc

OBJS=	cl_des.o cl_imei.o cl_list.o cl_md5.o cl_ribu.o cl_rlcmac.o cl_shrd.o

all:	comlib.a

comlib.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *errs

AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

SUBDIR=	init rvf rvm rvt support

INIT_OBJS=	init/create_RVtasks.o
RVF_OBJS=	rvf/rvf_buffer.o rvf/rvf_mem_pool.o rvf/rvf_msg.o \
		rvf/rvf_task.o rvf/rvf_time.o rvf/rvf_trace_adapt.o
RVM_OBJS=	rvm/rvm_api.o rvm/rvm_group_swe.o rvm/rvm_mem.o rvm/rvm_swe.o \
		rvm/rvm_swe_db.o rvm/rvm_swe_hdlr.o rvm/rvm_task.o
RVT_OBJS=	rvt/rvt_api.o rvt/rvt_env.o rvt/rvt_task.o
SUP_OBJS=	support/exception.o

OBJS=	${INIT_OBJS} ${RVF_OBJS} ${RVM_OBJS} ${RVT_OBJS} ${SUP_OBJS}

all:	librv.a

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

${INIT_OBJS}:	init
	@true

${RVF_OBJS}:	rvf
	@true

${RVM_OBJS}:	rvm
	@true

${RVT_OBJS}:	rvt
	@true

${SUP_OBJS}:	support
	@true

librv.a:	${OBJS}
	${AR} cru $@ ${OBJS}
	${RANLIB} $@

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

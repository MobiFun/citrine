sinclude ../include/config.mk

ALL_SUBDIR=	dar etm ffs pcm

BUILD_SUBDIR=	dar ffs
ifeq (${CONFIG_INCLUDE_PCM},1)
BUILD_SUBDIR+=	pcm
endif
ifeq (${RVM_ETM_SWE},1)
BUILD_SUBDIR+=	etm
endif

all:	${BUILD_SUBDIR}

${BUILD_SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${ALL_SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

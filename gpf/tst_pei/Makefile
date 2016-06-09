SUBDIR=	iram xip

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

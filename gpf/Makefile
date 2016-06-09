LIBCOMP=frame misc osl osx tst_drv tst_pei
SUBDIR=	${LIBCOMP} libs conf

all:	${SUBDIR}

libs:	${LIBCOMP}

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

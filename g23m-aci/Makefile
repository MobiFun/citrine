LD=	arm-elf-ld

SUBDIR=	aci aci_dti_mng aci_ext ati_ext bat dti gdd_dio ksd l2p uart

XIP_OBJS=	aci/xipcode.o aci_dti_mng/dti_conn_mng.o \
		aci_ext/aci_ext_pers.o ati_ext/ati_ext_mech.o ksd/xipcode.o \
		uart/xipcode.o

all:	xipcode.o dti

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

aci/xipcode.o:	aci
	@true

aci_dti_mng/dti_conn_mng.o:	aci_dti_mng
	@true

aci_ext/aci_ext_pers.o:	aci_ext
	@true

ati_ext/ati_ext_mech.o:	ati_ext
	@true

ksd/xipcode.o:	ksd
	@true

uart/xipcode.o:	uart
	@true

xipcode.o:	${XIP_OBJS}
	${LD} -r -o $@ ${XIP_OBJS}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

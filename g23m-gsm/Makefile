LD=	arm-elf-ld

SUBDIR=	alr cc dl l1 mm rr sim sms ss

XIP_OBJS=	alr/xipcode.o cc/xipcode.o dl/xipcode.o l1/l1_pei.o \
		mm/xipcode.o rr/xipcode.o sim/xipcode.o sms/xipcode.o \
		ss/xipcode.o

all:	xipcode.o

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

alr/xipcode.o:	alr
	@true

cc/xipcode.o:	cc
	@true

dl/xipcode.o:	dl
	@true

l1/l1_pei.o:	l1
	@true

mm/xipcode.o:	mm
	@true

rr/xipcode.o:	rr
	@true

sim/xipcode.o:	sim
	@true

sms/xipcode.o:	sms
	@true

ss/xipcode.o:	ss
	@true

xipcode.o:	${XIP_OBJS}
	${LD} -r -o $@ ${XIP_OBJS}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

SUBDIR=	L1 bsp ccd comlib finlink g23m-aci g23m-glue g23m-gsm gpf include \
	libiram lldbg nucleus riviera serial services sprintf sysglue

default:	config.stamp
	${MAKE} ${MFLAGS} -f Makefile.build $@

ramImage flashImage:	config.stamp
	${MAKE} ${MFLAGS} -f Makefile.build $@

config.stamp:	build.conf
	cfgmagic/processconf.sh
	touch $@

build.conf:
	@echo 'Configuration is required before the build.'
	@echo 'Please copy one of the configuration files under configs/'
	@echo 'to build.conf, optionally edit it to taste, and then run make.'
	@false

clean: FRC
	rm -f a.out core errs *.stamp
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

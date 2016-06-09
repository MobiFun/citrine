LD=	arm-elf-ld

SUBDIR=	audio_cfile cfile cust0 dsp dyn_dwl_cfile stand tpudrv

sinclude ../include/config.mk

INT_PIECES=	cfile/iramcode.o cust0/iramcode.o tpudrv/iramcode.o
EXT_PIECES=	audio_cfile/xipcode.o cfile/xipcode.o cust0/xipcode.o

ifeq (${L1_DYN_DSP_DWNLD},1)
EXT_PIECES+=	dyn_dwl_cfile/xipcode.o
endif

ifeq (${CONFIG_L1_STANDALONE},1)
EXT_PIECES+=	stand/xipcode.o
endif

all:	iramcode.o xipcode.o dsp

${SUBDIR}: FRC
	cd $@; ${MAKE} ${MFLAGS}

audio_cfile/xipcode.o:	audio_cfile
	@true

cfile/iramcode.o cfile/xipcode.o:	cfile
	@true

cust0/iramcode.o cust0/xipcode.o:	cust0
	@true

dyn_dwl_cfile/xipcode.o:	dyn_dwl_cfile
	@true

stand/xipcode.o:	stand
	@true

tpudrv/iramcode.o:	tpudrv
	@true

iramcode.o:	${INT_PIECES}
	${LD} -r -o $@ ${INT_PIECES}

xipcode.o:	${EXT_PIECES}
	${LD} -r -o $@ ${EXT_PIECES}

clean: FRC
	rm -f *.[oa] *.out *errs
	for i in ${SUBDIR}; do (cd $$i; ${MAKE} ${MFLAGS} clean); done

FRC:

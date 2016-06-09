CC=	arm-elf-gcc
LD=	arm-elf-ld
OBJCOPY=arm-elf-objcopy

FLASH_TARGET=	flashImage.bin
RAM_TARGET=	ramImage.srec

LIBC_A=		`${CC} -print-file-name=libc.a -mthumb-interwork`
LIBC_T=		`${CC} -print-file-name=libc.a -mthumb-interwork -mthumb`
LIBGCC_A=	`${CC} -print-file-name=libgcc.a -mthumb-interwork`
LIBGCC_T=	`${CC} -print-file-name=libgcc.a -mthumb-interwork -mthumb`

sinclude ../include/config.mk

BASE_LIBS=	../riviera/librv.a ../nucleus/libplus.xip.a \
		../nucleus/libplus.iram.a ../sprintf/libsprintf.a \
		../libiram/libiram.a
LIB_DEPEND=	${BASE_LIBS}

ifeq (${CONFIG_INCLUDE_PS},1)
PS_LIBS=	../g23m-aci/dti/libdti.a ../g23m-glue/gdi/libgdi.a \
		../comlib/comlib.a ../ccd/libccd.a
LIB_LINK_ORDER+=${PS_LIBS}
LIB_DEPEND+=	${PS_LIBS}
endif

ifeq (${CONFIG_INCLUDE_GPF},1)
GPF_LIBS=	../gpf/libs/libgpf.xip.a ../gpf/libs/libgpf.iram.a
LIB_LINK_ORDER+=	--start-group ${GPF_LIBS} --end-group
LIB_DEPEND+=	${GPF_LIBS}
endif

ifeq (${CONFIG_INCLUDE_L1},1)
LIB_LINK_ORDER+=../L1/dsp/libdsp.a
LIB_DEPEND+=	../L1/dsp/libdsp.a
endif

LIB_LINK_ORDER+=	${BASE_LIBS}
LIB_LINK_ORDER+=	--start-group ${LIBC_A} ${LIBGCC_A} --end-group

INT_PIECES=	../bsp/iramcode.o
EXT_PIECES=	../bsp/xipcode.o ../serial/xipcode.o ../sysglue/xipcode.o \
		../services/ffs/xipcode.o ../services/dar/xipcode.o
ifeq (${FLASH_IS_INTEL_ONEBANK},1)
INT_PIECES+=	../services/ffs/intelsbdrv.o
endif
ifeq (${CONFIG_INCLUDE_GPF},1)
EXT_PIECES+=	../gpf/conf/xipcode.o
endif
ifeq (${CONFIG_INCLUDE_L1},1)
INT_PIECES+=	../L1/iramcode.o
EXT_PIECES+=	../L1/xipcode.o
endif
ifeq (${CONFIG_INCLUDE_PS},1)
EXT_PIECES+=	../g23m-aci/xipcode.o ../g23m-gsm/xipcode.o
EXT_PIECES+=	../g23m-glue/cst/xipcode.o
endif
ifeq (${CONFIG_INCLUDE_PCM},1)
EXT_PIECES+=	../services/pcm/xipcode.o
endif
ifeq (${RVM_ETM_SWE},1)
EXT_PIECES+=	../services/etm/xipcode.o
endif
ifeq (${CONFIG_LLDBG},1)
EXT_PIECES+=	../lldbg/xipcode.o
endif

flashImage:	${FLASH_TARGET}
ramImage:	${RAM_TARGET}

iramcode.o:	${INT_PIECES}
	${LD} -r -o $@ ${INT_PIECES}

xipcode.o:	${EXT_PIECES}
	${LD} -r -o $@ ${EXT_PIECES}

flashImage.ld:	ld-script.src flash.m4 ../include/config.m4
	m4 flash.m4 ld-script.src > $@

flashImage.elf:	iramcode.o xipcode.o ../sysglue/flashboot.o ${LIB_DEPEND} \
		flashImage.ld
	${LD} -N -T flashImage.ld -o $@ -Map flashImage.map \
		iramcode.o xipcode.o ../sysglue/flashboot.o \
		${LIB_LINK_ORDER}

flashImage.bin:	flashImage.elf
	${OBJCOPY} -O binary $< $@

ramImage.ld:	ld-script.src xram.m4 ../include/config.m4
	m4 xram.m4 ld-script.src > $@

ramImage.elf:	iramcode.o xipcode.o ${LIB_DEPEND} ramImage.ld
	${LD} -N -T ramImage.ld -o $@ -Map ramImage.map iramcode.o xipcode.o \
		${LIB_LINK_ORDER}

ramImage.srec:	ramImage.elf
	${OBJCOPY} -O srec --srec-forceS3 --srec-len=30 $< $@

clean:
	rm -f *.o *errs *core *.elf *.bin *.srec *.ld *.map

FRC:

CC=	arm-elf-gcc
CFLAGS=	-O2 -fno-builtin -mthumb-interwork
ASFLAGS=-mthumb-interwork
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

IOBJS=	csc.o erc.o pmc.o pmce.o pmd.o smc.o smce.o smd.o tcc.o tcce.o tcd.o \
	tct.o tmc.o tmd.o tms.o tmse.o tmt.o

XTOBJS=	dmc.o dmce.o dmd.o dmf.o dmi.o dms.o erd.o eri.o evc.o evce.o evd.o \
	evf.o evi.o hic.o hid.o hii.o inc.o ioc.o ioce.o iod.o iof.o ioi.o \
	lic.o lid.o mbc.o mbce.o mbd.o mbf.o mbi.o mbs.o mbse.o pic.o pice.o \
	pid.o pif.o pii.o pis.o pise.o pmf.o pmi.o quc.o quce.o qud.o quf.o \
	qui.o qus.o quse.o rlc.o rld.o smf.o smi.o sms.o smse.o tcf.o tcfe.o \
	tci.o tcs.o tcse.o tmf.o tmi.o

XOBJS=	${XTOBJS} init.o
AOBJS=	${IOBJS} init.o

HDRS=	asm_defs.h cs_defs.h cs_extr.h dm_defs.h dm_extr.h er_defs.h er_extr.h \
	ev_defs.h ev_extr.h hi_defs.h hi_extr.h in_defs.h in_extr.h io_defs.h \
	io_extr.h mb_defs.h mb_extr.h nu_sd.h nucleus.h pi_defs.h pi_extr.h \
	pm_defs.h pm_extr.h profiler.h qu_defs.h qu_extr.h sm_defs.h sm_extr.h \
	tc_defs.h tc_extr.h tm_defs.h tm_extr.h

all:	libplus.iram.a libplus.xip.a

${AOBJS}:	${HDRS}

${XTOBJS}:	%.o : %.c ${HDRS}
	${CC} ${CFLAGS} -mthumb -c $<

libplus.iram.a:	${IOBJS}
	${AR} cru $@ ${IOBJS}
	${RANLIB} $@

libplus.xip.a:	${XOBJS}
	${AR} cru $@ ${XOBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *errs

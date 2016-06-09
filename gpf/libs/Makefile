AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

IOBJS=	../frame/iram/frame.o ../frame/iram/prf_func.o ../frame/iram/route.o \
	../frame/iram/vsi_com.o ../frame/iram/vsi_drv.o ../frame/iram/vsi_mem.o\
	../frame/iram/vsi_mis.o ../frame/iram/vsi_ppm.o ../frame/iram/vsi_pro.o\
	../frame/iram/vsi_sem.o ../frame/iram/vsi_tim.o ../frame/iram/vsi_trc.o\
	../osl/os_com_ir.o ../osl/os_mem_ir.o ../osl/os_mis_ir.o \
	../osl/os_pro_ir.o ../osl/os_sem_ir.o ../osl/os_tim_ir.o \
	../misc/iram/tools.o \
	../tst_drv/iram/ser_tr.o ../tst_drv/iram/tif2.o ../tst_drv/iram/titrc.o\
	../tst_pei/iram/tst_pei.o \
	../osx/osx.o

XOBJS=	../frame/xip/frame.o ../frame/xip/frame_version.o \
	../frame/xip/frm_ext.o ../frame/xip/prf_func.o ../frame/xip/route.o \
	../frame/xip/vsi_com.o ../frame/xip/vsi_drv.o ../frame/xip/vsi_mem.o \
	../frame/xip/vsi_mis.o ../frame/xip/vsi_ppm.o ../frame/xip/vsi_pro.o \
	../frame/xip/vsi_sem.o ../frame/xip/vsi_tim.o ../frame/xip/vsi_trc.o \
	../osl/os_com_fl.o ../osl/os_drv.o ../osl/os_isr.o ../osl/os_mem_fl.o \
	../osl/os_mis_fl.o ../osl/os_pro_fl.o ../osl/os_sem_fl.o \
	../osl/os_tim_fl.o \
	../misc/xip/misc_version.o ../misc/xip/tok.o ../misc/xip/tools.o \
	../tst_drv/xip/ser_tr.o ../tst_drv/xip/tif2.o ../tst_drv/xip/titrc.o \
	../tst_drv/xip/tr2.o \
	../tst_pei/xip/tif_version.o ../tst_pei/xip/tst_pei.o \
	../tst_pei/xip/tstdriver.o

ILIB=	libgpf.iram.a
XLIB=	libgpf.xip.a

all:	${ILIB} ${XLIB}

${ILIB}:	${IOBJS}
	${AR} cru $@ ${IOBJS}
	${RANLIB} $@

${XLIB}:	${XOBJS}
	${AR} cru $@ ${XOBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *.out *errs

CC=	arm-elf-gcc
LD=	arm-elf-ld

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
CPPFLAGS=-I. -I../../include -I../../include/condat -I../../gpf/inc \
	-I../../cdginc -I../aci_dti_mng -I../aci_ext -I../ati_ext -I../ksd

sinclude ../../include/config.mk

OBJS=	aci_aci.o aci_csf.o aci_em.o aci_lst.o aci_mem.o aci_mfw.o aci_pei.o \
	aci_tim.o aci_util.o aoc.o \
	ati_bas.o ati_cc.o ati_cmd.o ati_cphs.o ati_csc.o ati_dti.o ati_em.o \
	ati_em_ret.o ati_err.o ati_io.o ati_mm.o ati_mmi.o ati_pdu.o ati_phb.o \
	ati_prs.o ati_ret.o ati_sat.o ati_sim.o ati_sms.o ati_src_sat.o \
	ati_src_uart.o ati_src_uart_io.o ati_ss.o \
	cmh_ccf.o cmh_ccq.o cmh_ccr.o cmh_ccs.o cmh_cphs.o cmh_dmy.o cmh_dti.o \
	cmh_emq.o cmh_emr.o cmh_f.o cmh_mmf.o cmh_mmiq.o cmh_mmir.o cmh_mmis.o \
	cmh_mmit.o cmh_mmq.o cmh_mmr.o cmh_mms.o cmh_mmt.o cmh_phbf.o \
	cmh_phbq.o cmh_phbr.o cmh_phbs.o cmh_phbt.o cmh_satf.o cmh_satq.o \
	cmh_satr.o cmh_sats.o cmh_simf.o cmh_simq.o cmh_simr.o cmh_sims.o \
	cmh_smsf.o cmh_smsq.o cmh_smsr.o cmh_smss.o cmh_ssf.o cmh_ssq.o \
	cmh_ssr.o cmh_sss.o cmh_tsts.o cmh_uartf.o cmh_uartr.o cmh_uarts.o \
	cphs.o cphs_aci.o dti_cntrl_mng.o hl_audio_drv.o \
	line_edit.o line_split.o phb.o \
	psa_ccf.o psa_ccp.o psa_ccs.o psa_f.o psa_mmf.o psa_mmif.o psa_mmip.o \
	psa_mmis.o psa_mmp.o psa_mms.o psa_satf.o psa_satp.o psa_sats.o \
	psa_simf.o psa_simp.o psa_sims.o psa_smsf.o psa_smsp.o psa_smss.o \
	psa_ssf.o psa_ssp.o psa_sss.o psa_uartp.o psa_uarts.o psa_util.o \
	sap_dti.o sap_em.o version_dummy.o

ifeq (${CONFIG_AT_RVTMUX},1)
OBJS+=	ati_src_rvt.o
endif

all:	xipcode.o

xipcode.o:	${OBJS}
	${LD} -r -o $@ ${OBJS}

clean:
	rm -f *.[oa] *.out *errs

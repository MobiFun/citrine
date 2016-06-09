CC=	arm-elf-gcc
AR=	arm-elf-ar
RANLIB=	arm-elf-ranlib

CFLAGS=	-O2 -fno-builtin -mthumb-interwork -mthumb
INCPATH=-I../gpf/inc -I../cdginc
DEFS=	-DM_INTEL -DSHARED_VSI -DSHARED_CCD -DERR_TRACE_CCD
CPPFLAGS=${INCPATH} ${DEFS}

CCD_OBJS=	ccd.o bitfun.o cdc_std.o cdc_com.o ccd_err.o ccd_store.o \
		gsm1_tv.o gsm1_v.o gsm2_t.o gsm3_tv.o gsm3_v.o gsm4_lv.o \
		gsm4_tlv.o gsm5_tlv.o gsm5_tv.o gsm5_v.o gsm6_tlv.o gsm7_lv.o \
		bcd_mnc.o bcd_nofill.o bcdeven.o bcdodd.o \
		gsm1_asn.o \
		csn1_s1.o csn1_sh.o s_padding.o t30_ident.o \
		asn1_bitstr.o asn1_octet.o asn1_integ.o \
		asn1_seq.o asn1_choice.o no_code.o \
		asn1_choice_ext.o asn1_seq_ext.o asn1_integ_ext.o \
		asn1_opentype.o asn1_objid.o \
		noncritical_ext.o critical_ext.o \
		ccd_elem.o s_padding_0.o csn1_s0.o hl_flag.o \
		freq_list.o fdd_ci.o tdd_ci.o freq_list_com.o \
		csn1_concat.o break_cond.o csn1_sx.o \
		csn1_choice_1.o csn1_choice_2.o csn1_choice_x.o \
		csn1_sh_opt.o csn1_s1_opt.o csn1_s0_opt.o

CCDDATA_OBJS=	ccd_config.o ccddata_ccd.o ccddata_ccdent.o ccddata_ccdmtab.o \
		ccddata_cdemval.o ccddata_mconst.o

ALL_OBJS=	${CCD_OBJS} ${CCDDATA_OBJS}

all:	libccd.a

libccd.a:	${ALL_OBJS}
	${AR} cru $@ ${ALL_OBJS}
	${RANLIB} $@

clean:
	rm -f *.[oa] *errs

/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  ccd_config.c
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Deutschland GmbH
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Deutschland GmbH 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments  Deutschland GmbH. 
+----------------------------------------------------------------------------- 
|  Purpose :  Definition of codec for CCD based on Ccddata
+----------------------------------------------------------------------------- 
*/ 

#define CCD_CONFIG_C

/*
 * standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"

/* MAX_ERRORS and T_CCD_ERR_ENTRY are defined in ccd_globs.h */ 
#include "ccd_globs.h"
#include "ccdtable.h"
#include "ccddata.h"
/* prototypes of the exported functions are given in ccd.h */
#include "ccd.h"

/*
 * all coding types with their constant enumeration are given in ccd_codingtypes.h
 */
#include "ccd_codingtypes.h"

#ifndef CCD_MK
#include "mconst.cdg"
#endif

/*
 * ccdError and ccdWarning are defined in ccdapi.h 
 */
#include "ccdapi.h"

#if defined CCD_MK || !defined _TOOLS_
UBYTE cdc_init (T_FUNC_POINTER codec[MAX_CODEC_ID+1][2])
{
  int i, j;

  for (i = 0; i <= MAX_CODEC_ID; i++)
  {
    codec[i][0] = cdc_std_encode;
    codec[i][1] = cdc_std_decode;
  }

  i = 0; 
  j = 1;       
#if defined GSM1_V || defined _TOOLS_
  codec[CCDTYPE_GSM1_V][0] = cdc_gsm1v_encode;
  codec[CCDTYPE_GSM1_V][1] = cdc_gsm1v_decode;
#endif

  j++;
#if defined GSM1_TV || defined _TOOLS_
  codec[CCDTYPE_GSM1_TV][0] = cdc_gsm1tv_encode;
  codec[CCDTYPE_GSM1_TV][1] = cdc_gsm1tv_decode;
#endif

  j++;
#if defined GSM2_T || defined _TOOLS_
  codec[CCDTYPE_GSM2_T][0] = cdc_gsm2t_encode;
  codec[CCDTYPE_GSM2_T][1] = cdc_gsm2t_decode;
#endif

  j++;
#if defined GSM3_V || defined _TOOLS_
  codec[CCDTYPE_GSM3_V][0] = cdc_gsm3v_encode;
  codec[CCDTYPE_GSM3_V][1] = cdc_gsm3v_decode;
#endif

  j++;
#if defined GSM3_TV || defined _TOOLS_
  codec[CCDTYPE_GSM3_TV][0] = cdc_gsm3tv_encode;
  codec[CCDTYPE_GSM3_TV][1] = cdc_gsm3tv_decode;
#endif
      
  j++;   
#if defined GSM4_LV || defined _TOOLS_
  codec[CCDTYPE_GSM4_LV][0] = cdc_gsm4lv_encode;
  codec[CCDTYPE_GSM4_LV][1] = cdc_gsm4lv_decode;
#endif

  j++;
#if defined GSM4_TLV || defined _TOOLS_
  codec[CCDTYPE_GSM4_TLV][0] = cdc_gsm4tlv_encode;
  codec[CCDTYPE_GSM4_TLV][1] = cdc_gsm4tlv_decode;
#endif

  j++;
#if defined GSM5_V || defined _TOOLS_
  codec[CCDTYPE_GSM5_V][0] = cdc_gsm5v_encode;
  codec[CCDTYPE_GSM5_V][1] = cdc_gsm5v_decode;
#endif

  j++;
#if defined GSM5_TLV || defined _TOOLS_
  codec[CCDTYPE_GSM5_TLV][0] = cdc_gsm5tlv_encode;
  codec[CCDTYPE_GSM5_TLV][1] = cdc_gsm5tlv_decode;
#endif

  j++;
#if defined GSM6_TLV || defined _TOOLS_
  codec[CCDTYPE_GSM6_TLV][0] = cdc_gsm6tlv_encode;
  codec[CCDTYPE_GSM6_TLV][1] = cdc_gsm6tlv_decode;
#endif

  j++;
#if defined GSM7_LV || defined _TOOLS_
  codec[CCDTYPE_GSM7_LV][0] = cdc_gsm7lv_encode;
  codec[CCDTYPE_GSM7_LV][1] = cdc_gsm7lv_decode;
#endif

  j++;
#if defined GSM1_ASN || defined _TOOLS_
  codec[CCDTYPE_GSM1_ASN][0] = cdc_gsm1asn_encode;
  codec[CCDTYPE_GSM1_ASN][1] = cdc_gsm1asn_decode;
#endif         

  j++;
#if defined BCDODD || defined _TOOLS_
  codec[CCDTYPE_BCDODD][0] = cdc_bcdodd_encode;
  codec[CCDTYPE_BCDODD][1] = cdc_bcdodd_decode;
#endif

  j++;
#if defined BCDEVEN || defined _TOOLS_
  codec[CCDTYPE_BCDEVEN][0] = cdc_bcdeven_encode;
  codec[CCDTYPE_BCDEVEN][1] = cdc_bcdeven_decode;
#endif

  j++;
#if defined BCD_NOFILL || defined _TOOLS_
  codec[CCDTYPE_BCD_NOFILL][0] = cdc_bcd_nofill_encode;
  codec[CCDTYPE_BCD_NOFILL][1] = cdc_bcd_nofill_decode;
#endif

  j++;
#if defined BCD_MNC || defined _TOOLS_
  codec[CCDTYPE_BCD_MNC][0] = cdc_bcd_mnc_encode;
  codec[CCDTYPE_BCD_MNC][1] = cdc_bcd_mnc_decode;
#endif

  j++;
#if defined CSN1_S1 || defined _TOOLS_
  codec[CCDTYPE_CSN1_S1][0] = cdc_csn1_s1_encode;
  codec[CCDTYPE_CSN1_S1][1] = cdc_csn1_s1_decode;
#endif        
      
  j++;   
#if defined CSN1_SHL || defined _TOOLS_
  codec[CCDTYPE_CSN1_SHL][0] = cdc_csn1_shl_encode;
  codec[CCDTYPE_CSN1_SHL][1] = cdc_csn1_shl_decode;
#endif        

  j++;  
#if defined S_PADDING || defined _TOOLS_
  codec[CCDTYPE_S_PADDING][0] = cdc_padd_encode;
  codec[CCDTYPE_S_PADDING][1] = cdc_padd_decode;
#endif

  j++;
#if defined T30_IDENT || defined _TOOLS_
  codec[CCDTYPE_T30_IDENT][0] = cdc_t30_ident_encode;
  codec[CCDTYPE_T30_IDENT][1] = cdc_t30_ident_decode;
#endif

  j++;
#if defined BITSTRING || defined _TOOLS_
  codec[CCDTYPE_BITSTRING][0] = cdc_bitstring_encode;
  codec[CCDTYPE_BITSTRING][1] = cdc_bitstring_decode;
#endif

  j++;
#if defined ASN1_INTEGER || defined _TOOLS_
  codec[CCDTYPE_ASN1_INTEGER][0] = cdc_asn1_integ_encode;
  codec[CCDTYPE_ASN1_INTEGER][1] = cdc_asn1_integ_decode;
#endif

  j++;
#if defined ASN1_SEQUENCE || defined _TOOLS_
  codec[CCDTYPE_ASN1_SEQUENCE][0] = cdc_asn1_seq_encode;
  codec[CCDTYPE_ASN1_SEQUENCE][1] = cdc_asn1_seq_decode;
#endif

  j++;
#if defined ASN1_CHOICE || defined _TOOLS_
  codec[CCDTYPE_ASN1_CHOICE][0] = cdc_asn1_choice_encode;
  codec[CCDTYPE_ASN1_CHOICE][1] = cdc_asn1_choice_decode;
#endif

  j++;
#if defined ASN1_OCTET || defined _TOOLS_
  codec[CCDTYPE_ASN1_OCTET][0] = cdc_asn1_octet_encode;
  codec[CCDTYPE_ASN1_OCTET][1] = cdc_asn1_octet_decode;
#endif

  j++;
#if defined NO_CODE || defined _TOOLS_
  codec[CCDTYPE_NO_CODE][0] = cdc_no_encode;
  codec[CCDTYPE_NO_CODE][1] = cdc_no_decode;
#endif

  j++;
#if defined ASN1_INTEGER_EXTENSIBLE || defined _TOOLS_
  codec[CCDTYPE_ASN1_INTEGER_EXTENSIBLE][0] = cdc_asn1_integ_ext_encode;
  codec[CCDTYPE_ASN1_INTEGER_EXTENSIBLE][1] = cdc_asn1_integ_ext_decode;
#endif

  j++;
#if defined ASN1_SEQUENCE_EXTENSIBLE || defined _TOOLS_
  codec[CCDTYPE_ASN1_SEQUENCE_EXTENSIBLE][0] = cdc_asn1_seq_ext_encode;
  codec[CCDTYPE_ASN1_SEQUENCE_EXTENSIBLE][1] = cdc_asn1_seq_ext_decode;
#endif

  j++;
#if defined ASN1_CHOICE_EXTENSIBLE || defined _TOOLS_
  codec[CCDTYPE_ASN1_CHOICE_EXTENSIBLE][0] = cdc_asn1_choice_ext_encode;
  codec[CCDTYPE_ASN1_CHOICE_EXTENSIBLE][1] = cdc_asn1_choice_ext_decode;
#endif

  j++;
#if defined ASN1_OBJ_ID || defined _TOOLS_
  codec[CCDTYPE_ASN1_OBJ_ID][0] = cdc_asn1_obj_id_encode;
  codec[CCDTYPE_ASN1_OBJ_ID][1] = cdc_asn1_obj_id_decode;
#endif

  j++;
#if defined ASN1_OPEN_TYPE || defined _TOOLS_
  codec[CCDTYPE_ASN1_OPEN_TYPE][0] = cdc_asn1_open_type_encode;
  codec[CCDTYPE_ASN1_OPEN_TYPE][1] = cdc_asn1_open_type_decode;
#endif

  j++;
#if defined NONCRITICAL_EXT || defined _TOOLS_
  codec[CCDTYPE_NONCRITICAL_EXT][0] = cdc_noncritical_ext_encode;
  codec[CCDTYPE_NONCRITICAL_EXT][1] = cdc_noncritical_ext_decode;
#endif

  j++;
#if defined CRITICAL_EXT || defined _TOOLS_
  codec[CCDTYPE_CRITICAL_EXT][0] = cdc_critical_ext_encode;
  codec[CCDTYPE_CRITICAL_EXT][1] = cdc_critical_ext_decode;
#endif

  j++;
#if defined S_PADDING_0 || defined _TOOLS_
  codec[CCDTYPE_S_PADDING_0][0] = cdc_padd_0_encode;
  codec[CCDTYPE_S_PADDING_0][1] = cdc_padd_0_decode;
#endif

  j++;
#if defined CSN1_S0 || defined _TOOLS_
  codec[CCDTYPE_CSN1_S0][0] = cdc_csn1_s0_encode;
  codec[CCDTYPE_CSN1_S0][1] = cdc_csn1_s0_decode;
#endif


  j++;
#if defined HL_FLAG || defined _TOOLS_
  codec[CCDTYPE_HL_FLAG][0] = cdc_hl_flag_encode;
  codec[CCDTYPE_HL_FLAG][1] = cdc_hl_flag_decode;
#endif

  j++;

#if defined FDD_CI || defined _TOOLS_
  codec[CCDTYPE_FDD_CI][0] = cdc_fdd_ci_encode;
  codec[CCDTYPE_FDD_CI][1] = cdc_fdd_ci_decode;
#endif

  j++;

#if defined TDD_CI || defined _TOOLS_
  codec[CCDTYPE_TDD_CI][0] = cdc_tdd_ci_encode;
  codec[CCDTYPE_TDD_CI][1] = cdc_tdd_ci_decode;
#endif

  j++;

#if defined FREQ_LIST || defined _TOOLS_
  codec[CCDTYPE_FREQ_LIST][0] = cdc_freq_list_encode;
  codec[CCDTYPE_FREQ_LIST][1] = cdc_freq_list_decode;
#endif

  j++;
#if defined CSN1_CONCAT || defined _TOOLS_
  codec[CCDTYPE_CSN1_CONCAT][0] = cdc_csn1_concat_encode;
  codec[CCDTYPE_CSN1_CONCAT][1] = cdc_csn1_concat_decode;
#endif

    j++;
#if defined CCDTYPE_BREAK_COND || defined _TOOLS_
  codec[CCDTYPE_BREAK_COND][0] = cdc_break_cond_encode;
  codec[CCDTYPE_BREAK_COND][1] = cdc_break_cond_decode;
#endif

  j++;
#if defined GSM5_TV || defined _TOOLS_
  codec[CCDTYPE_GSM5_TV][0] = cdc_gsm5tv_encode;
  codec[CCDTYPE_GSM5_TV][1] = cdc_gsm5tv_decode;
#endif

  j++;
#if defined CSN1_CHOICE1 || defined _TOOLS_
  codec[CCDTYPE_CSN1_CHOICE1][0] = cdc_csn1_choice1_encode;
  codec[CCDTYPE_CSN1_CHOICE1][1] = cdc_csn1_choice1_decode;
#endif

  j++;
#if defined CSN1_CHOICE2 || defined _TOOLS_
  codec[CCDTYPE_CSN1_CHOICE2][0] = cdc_csn1_choice2_encode;
  codec[CCDTYPE_CSN1_CHOICE2][1] = cdc_csn1_choice2_decode;
#endif

  j++;   
#if defined CSN1_SHL_OPT || defined _TOOLS_
  codec[CCDTYPE_CSN1_SHL_OPT][0] = cdc_csn1_shl_opt_encode;
  codec[CCDTYPE_CSN1_SHL_OPT][1] = cdc_csn1_shl_opt_decode;
#endif        

  j++;   
#if defined CSN1_S1_OPT || defined _TOOLS_
  codec[CCDTYPE_CSN1_S1_OPT][0] = cdc_csn1_s1_opt_encode;
  codec[CCDTYPE_CSN1_S1_OPT][1] = cdc_csn1_s1_opt_decode;
#endif        

  j++;   
#if defined CSN1_S0_OPT || defined _TOOLS_
  codec[CCDTYPE_CSN1_S0_OPT][0] = cdc_csn1_s0_opt_encode;
  codec[CCDTYPE_CSN1_S0_OPT][1] = cdc_csn1_s0_opt_decode;
#endif        

  /*
   * CCD has more co/dec functions than used in the table above.
   * Look in ccd_codingtypes.h.
   */
  /*lint -e774
   * The follwing instruction checks if it was forgotten to initialize
   * the codec when the number of coding types was increased; if everything
   * was OK, the if evaluates for sure to false, causing the lint error
   */
  if (j < MAX_CODEC_ID)
    return ccdWarning;
  /*lint +e774*/

  return ccdOK;
}
#endif /* CCD_MK || !_TOOLS_ */

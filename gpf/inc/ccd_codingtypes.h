/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_codingtypes.h
+----------------------------------------------------------------------------- 
|  Copyright 2004 Texas Instruments Deutschland GmbH
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Deutschland GmbH 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Deutschland GmbH. 
+----------------------------------------------------------------------------- 
|  Purpose :  Definition of constant numbers for coding types used by CCD and 
|             CCDDATA
+----------------------------------------------------------------------------- 
*/

#define MAX_CODEC_ID    0x2f

/*
 * These numbers refer to the place of each used en/decoding function 
 * in the codec table. The first entry is reserved for cdc_STD_encode and
 * cdc_STD_decode.
 */
#define CCDTYPE_GSM1_V                 0x1
#define CCDTYPE_GSM1_TV                0x2
#define CCDTYPE_GSM2_T                 0x3
#define CCDTYPE_GSM3_V                 0x4
#define CCDTYPE_GSM3_TV                0x5
#define CCDTYPE_GSM4_LV                0x6
#define CCDTYPE_GSM4_TLV               0x7
#define CCDTYPE_GSM5_V                 0x8
#define CCDTYPE_GSM5_TLV               0x9
#define CCDTYPE_GSM6_TLV               0xa
#define CCDTYPE_GSM7_LV                0xb
#define CCDTYPE_GSM1_ASN               0xc
#define CCDTYPE_BCDODD                 0xd
#define CCDTYPE_BCDEVEN                0xe
#define CCDTYPE_BCD_NOFILL             0xf
#define CCDTYPE_BCD_MNC                0x10
#define CCDTYPE_CSN1_S1                0x11
#define CCDTYPE_CSN1_SHL               0x12
#define CCDTYPE_S_PADDING              0x13
#define CCDTYPE_T30_IDENT              0x14
#define CCDTYPE_BITSTRING              0x15
#define CCDTYPE_ASN1_INTEGER           0x16
#define CCDTYPE_ASN1_SEQUENCE          0x17
#define CCDTYPE_ASN1_CHOICE            0x18
#define CCDTYPE_ASN1_OCTET             0x19
#define CCDTYPE_NO_CODE                0x1a
#define CCDTYPE_ASN1_INTEGER_EXTENSIBLE   0x1b
#define CCDTYPE_ASN1_SEQUENCE_EXTENSIBLE  0x1c
#define CCDTYPE_ASN1_CHOICE_EXTENSIBLE 0x1d
#define CCDTYPE_ASN1_OBJ_ID            0x1e
#define CCDTYPE_ASN1_OPEN_TYPE         0x1f
#define CCDTYPE_NONCRITICAL_EXT        0x20
#define CCDTYPE_CRITICAL_EXT           0x21
#define CCDTYPE_S_PADDING_0            0x22
#define CCDTYPE_CSN1_S0                0x23
#define CCDTYPE_HL_FLAG                0x24
#define CCDTYPE_FDD_CI                 0x25
#define CCDTYPE_TDD_CI                 0x26
#define CCDTYPE_FREQ_LIST              0x27
#define CCDTYPE_CSN1_CONCAT            0x28
#define CCDTYPE_BREAK_COND             0x29
#define CCDTYPE_GSM5_TV                0x2a
#define CCDTYPE_CSN1_CHOICE1           0x2b
#define CCDTYPE_CSN1_CHOICE2           0x2c
#define CCDTYPE_CSN1_SHL_OPT           0x2d
#define CCDTYPE_CSN1_S1_OPT            0x2e
#define CCDTYPE_CSN1_S0_OPT            0x2f

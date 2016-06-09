/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd.h
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Condat Coder Decoder
|             Global function prototypes
+----------------------------------------------------------------------------- 
*/ 


#ifndef CCD_H
#define CCD_H

/* Start addresses of ccddata tables */
#ifndef CCD_C
extern const T_CCD_VarTabEntry*   mvar;
extern const T_CCD_SpareTabEntry* spare;
extern const T_CCD_CalcTabEntry*  calc;
extern const T_CCD_CompTabEntry*  mcomp;
extern const T_CCD_ElemTabEntry*  melem;
extern const T_CCD_CalcIndex*     calcidx;
extern const T_CCD_ValTabEntry*   mval;
#endif /* !CCD_C */

/*
 * Chain of error information.
 */
typedef struct ccd_error_list
{
  T_CCD_ERR_ENTRY entry;
  struct ccd_error_list *next;
} T_CCD_ERR_LIST;

/*
 * Head of the error information chain.
 */
typedef struct ccd_err_list_head
{
  T_CCD_ERR_LIST *first_error;
  T_CCD_ERR_LIST **act_error;
  int            num_errors;
} T_CCD_ERR_LIST_HEAD;

typedef struct ccd_store_list
{
  ULONG store[3];
  /* for future use to extend register capacity  */
  /* T_CCD_STORE_LIST *next; */
} T_CCD_STORE_LIST;

/*
 * CCD list of all tasks
 */
typedef struct ccd_task_table
{
  T_CCD_Globs* ccd_globs;
  U8* decmsgbuf;
  T_CCD_ERR_LIST_HEAD* ccd_err_list;
  T_CCD_STORE_LIST* ccd_store;
} T_CCD_TASK_TABLE;

/*
 * Macros for forcing n-byte aligned addresses to byte arrays
 */
#ifndef BUFFER_ALIGNMENT
  #define BUFFER_ALIGNMENT 3
#endif

#define ALIGN_BUF(uabuf) (UBYTE*) ((((U32)uabuf) + BUFFER_ALIGNMENT) & \
                                   (~BUFFER_ALIGNMENT))

/*
 * Error behaviour to use with ccd_setError()
 */
#ifdef BREAK
#undef BREAK
#endif

#define BREAK                0
#define CONTINUE             1

#define END_OF_COMPOSITION   0x7e

/*
 * Functions defined in ccd.c
 */
EXTERN BOOL   ccd_conditionOK       (const ULONG  e_ref,
                                     T_CCD_Globs *globs);

EXTERN BOOL   ccd_calculateRep      (const ULONG  e_ref,
                                     ULONG       *repeat,
                                     ULONG       *max_repeat,
                                     T_CCD_Globs *globs);

EXTERN void   ccd_performOperations (ULONG        num_of_ops,
                                     ULONG        op_def_ref,
                                     T_CCD_Globs *globs);

EXTERN void   ccd_encodeComposition (const ULONG  c_ref,
                                     T_CCD_Globs *globs);

EXTERN void   ccd_decodeComposition (const ULONG  c_ref,
                                     T_CCD_Globs *globs);
extern int    ccd_check_pointer     (U8* ptr);

/* from ccd_err.c */
extern void   ccd_setError          (T_CCD_Globs *globs,
                                     UBYTE      ErrCode,
                                     UBYTE      Action,
                                     USHORT     first_par, ...);
extern void   ccd_recordFault       (T_CCD_Globs *globs,
                                     UBYTE      ErrCode,
                                     UBYTE      Action,
                                     T_ERR_INFO error_info,
                                     U8        *err_IEaddr);
extern void   ccd_err_reset         (T_CCD_ERR_LIST_HEAD* eentry);
extern void   ccd_err_free          (T_CCD_ERR_LIST_HEAD* eentry);
extern int    ccd_err_init          (T_CCD_ERR_LIST_HEAD** eentry);
extern void   ccd_err_exit          (void);
extern T_CCD_Globs* ccd_GetGlobVars (T_CCD_ERR_LIST_HEAD** eentry, 
                                     T_CCD_STORE_LIST** stoentry);
extern void ccd_FreeGlobVars        (T_CCD_Globs* globs);
extern void ccd_common_decode_init  (U16 l_buf,
                                     U16 o_buf,
                                     U8 *buf,
                                     T_CCD_Globs *globs);
extern void ccd_dump_msg            (U16 l_buf,
                                     U16 o_buf,
                                     U8 *buf,
                                     T_CCD_Globs *globs);
/* from ccd_store.c */
extern void ccd_store_exit          (void);
extern UBYTE ccd_writeStore         (T_CCD_Globs *globs, ULONG regNo, ULONG value);
extern UBYTE ccd_getStore           (T_CCD_Globs *globs, ULONG regNo, ULONG *value);
extern int   ccd_store_init         (T_CCD_STORE_LIST** stoentry);

/* from ccd_patch.c */
extern int ccd_patch (T_CCD_Globs* globs, int validflag);
                                     /*
 *
 * Functions defined in cdc_std.c
 */
/*
 * STANDARD codecs
 */
EXTERN SHORT  cdc_std_decode        (const ULONG  c_ref,
                                     const ULONG  e_ref,
                                     T_CCD_Globs *globs);

EXTERN SHORT  cdc_std_encode        (const ULONG  c_ref,
                                     const ULONG  e_ref,
                                     T_CCD_Globs *globs);

EXTERN void   cdc_decodeElemvalue   (ULONG        e_ref,
                                     ULONG       *repeat, 
                                     T_CCD_Globs *globs);

EXTERN void   cdc_encodeElemvalue   (ULONG        e_ref,
                                     ULONG        repeat, 
                                     T_CCD_Globs *globs);

#ifdef DEBUG_CCD
EXTERN void   TRACE_CCD             (T_CCD_Globs  *globs,
                                     char         *format, ...);
#endif


typedef struct
{
  BOOL gotTag;
  BOOL gotLen; 
} T_TLV_SORT;

/* 
 * Functions defined in cdc_com.c
 */
EXTERN void   cdc_BCD_decode        (const ULONG e_ref, 
                                     UBYTE       startDigit, 
                                     T_CCD_Globs *globs);

EXTERN void   cdc_BCD_encode        (const ULONG e_ref, 
                                     UBYTE       startDigit, 
                                     T_CCD_Globs *globs);
                                     
EXTERN SHORT  cdc_tlv_decode        (const ULONG  c_ref,
                                     const ULONG  e_ref,
                                     const T_TLV_SORT  *tlv_inf,
                                     T_CCD_Globs *globs);

EXTERN void   cdc_tlv_encode        (const ULONG  e_ref,
                                     UBYTE        lenT,
                                     UBYTE        lenL, 
                                     T_CCD_Globs *globs);

EXTERN void   cdc_GSM_start         (T_CCD_Globs *globs);

EXTERN U16    cdc_isPresent         (const ULONG  e_ref,
                                     T_CCD_Globs *globs);

EXTERN BOOL   is_pointer_type       (const ULONG  e_ref);
EXTERN BOOL   is_variable_type      (const ULONG  e_ref);

EXTERN SHORT  PER_CommonBegin       (const ULONG  e_ref,
                                     ULONG       *maxRep, 
                                     T_CCD_Globs *globs);

EXTERN U8     *PER_allocmem         (const ULONG  e_ref,
                                     ULONG        repeat,
                                     T_CCD_Globs *globs);

EXTERN USHORT PER_allocmem_and_update (const ULONG  e_ref,
                                       ULONG        repeat, 
                                       T_CCD_Globs *globs);

/* 
 * Rest of the CCD coding functions e.g. GSM4_TLV, CSN1_S1 
 */

/* 
 * Functions used for standard IEs of GSM
 */
EXTERN SHORT cdc_gsm1v_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm1v_encode   (const ULONG c_Ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm1tv_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm1tv_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm2t_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm2t_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm3v_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm3v_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm3tv_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm3tv_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm4lv_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm4lv_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm4tlv_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm4tlv_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5v_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5v_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5tv_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5tv_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5tlv_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm5tlv_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm6tlv_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm6tlv_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm7lv_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm7lv_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for BCD digits 
 */
EXTERN SHORT cdc_bcdodd_decode     (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcdodd_encode     (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcdeven_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcdeven_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcd_nofill_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcd_nofill_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcd_mnc_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bcd_mnc_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for ASN1 BER
 */
EXTERN SHORT cdc_gsm1asn_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_gsm1asn_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for rest octets
 */
EXTERN const UBYTE padding_bits[];
EXTERN const UBYTE padding_bits_prev[];
#define GET_HL(bit)     (padding_bits[globs->bitpos%8] ^ bit)
#define GET_HL_PREV(bit) (padding_bits_prev[globs->bitpos%8] ^ bit)
EXTERN SHORT cdc_csn1_s1_decode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s1_encode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s0_decode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s0_encode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_shl_decode     (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_shl_encode     (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_padd_decode         (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_padd_encode         (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_padd_0_decode       (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_padd_0_encode       (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_hl_flag_decode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_hl_flag_encode      (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_concat_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_concat_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_break_cond_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_break_cond_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_choice1_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_choice1_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_choice2_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_choice2_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_shl_opt_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_shl_opt_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s1_opt_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s1_opt_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s0_opt_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_csn1_s0_opt_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for fax and data
 */
EXTERN SHORT cdc_t30_ident_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_t30_ident_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
                                 
/* 
 * Functions and types used for decoding frequency lists, fdd_cell_information 
 * and tdd_cell_information
 */
#if defined CCD_TEST
#include <stdlib.h>
#define  MALLOC(P,S) P = (void*)malloc(S)
#define  MFREE(P)    free(P)
#endif

typedef struct
{
  U8 length;
  U8 count;
} T_W_PARAM;

EXTERN const T_W_PARAM param_1024[];
EXTERN const T_W_PARAM param_512[];

#define BITOFFSET_LIST          1024
#define T_LIST_MAX_SIZE 128 /* 1024/8 = 128 */

typedef struct
{
  U8 channels [T_LIST_MAX_SIZE];
} T_LIST;

#define FDD_CI_LIST     1
#define TDD_CI_LIST     2
#define FREQUENCY_LIST  3
EXTERN U8 ByteBitMask[];
EXTERN long for_modulo (long a, long b);
EXTERN void cdc_decode_param       (const T_W_PARAM *param,short *w, U16 ListLength, T_CCD_Globs *globs);
EXTERN void cdc_decode_frequencies (short original_range, short *w, short offset, U8 callerID, T_CCD_Globs *globs);
EXTERN SHORT cdc_freq_list_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_freq_list_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_fdd_ci_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_fdd_ci_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_tdd_ci_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_tdd_ci_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for ASN1 PER
 */
EXTERN void  PER_Decode_ASN1_CHOICE_alterative (const ULONG e_ref, T_ENUM UnionTag, T_CCD_Globs *globs);
EXTERN void  PER_Encode_ASN1_CHOICE_alterative (const ULONG e_ref, T_ENUM UnionTag, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_choice_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_choice_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_integ_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_integ_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_octet_encode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_octet_decode  (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_seq_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_seq_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bitstring_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_bitstring_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for ASN1 PER extensible
 */
EXTERN const UBYTE bitSize[];
EXTERN U32   Read_NormallySmallNonNegativeWholeNr (T_CCD_Globs *globs);
EXTERN void  Write_NormallySmallNonNegativeWholeNr (U32 Value, T_CCD_Globs *globs);
EXTERN U32   Read_OpenTpye_Length (T_CCD_Globs *globs);
EXTERN void  Write_OpenTpye_Length (U32 Value, T_CCD_Globs *globs);

EXTERN SHORT cdc_asn1_choice_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_choice_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

EXTERN void  Read_unique_Integer (const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_integ_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_integ_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

EXTERN void  Read_SEQ_BitMap  (const ULONG first_elem, const ULONG last_elem, T_CCD_Globs *globs);
EXTERN void  Write_SEQ_BitMap (const ULONG first_elem, const ULONG last_elem, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_seq_ext_decode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_seq_ext_encode   (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
                                       
EXTERN SHORT cdc_asn1_obj_id_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_obj_id_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

EXTERN SHORT cdc_asn1_open_type_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_asn1_open_type_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
/* 
 * Extra functions
 */
EXTERN SHORT cdc_no_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_no_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);

/* 
 * Functions used for umts message extensions
 */
EXTERN SHORT cdc_noncritical_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_noncritical_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_critical_ext_decode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
EXTERN SHORT cdc_critical_ext_encode    (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs);
#endif

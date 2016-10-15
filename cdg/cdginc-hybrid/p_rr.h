/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_rr.h                                                         |
| SOURCE  : "sap\rr.pdf"                                                   |
| LastModified : "2003-02-04"                                              |
| IdAndVersion : "6147.107.97.108"                                         |
| SrcFileTime  : "Tue Jun 3 11:14:30 2008"                                 |
| Generated by CCDGEN_2.5.5A on Fri Oct 14 21:41:52 2016                   |
|           !!DO NOT MODIFY!!DO NOT MODIFY!!DO NOT MODIFY!!                |
+--------------------------------------------------------------------------+
*/

/* PRAGMAS
 * PREFIX                 : NONE
 * COMPATIBILITY_DEFINES  : NO (require PREFIX)
 * ALWAYS_ENUM_IN_VAL_FILE: NO
 * ENABLE_GROUP: NO
 * CAPITALIZE_TYPENAME: NO
 */


#ifndef P_RR_H
#define P_RR_H


#define CDG_ENTER__P_RR_H

#define CDG_ENTER__FILENAME _P_RR_H
#define CDG_ENTER__P_RR_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_RR_H__LAST_MODIFIED _2003_02_04
#define CDG_ENTER__P_RR_H__ID_AND_VERSION _6147_107_97_108

#define CDG_ENTER__P_RR_H__SRC_FILE_TIME _Tue_Jun_3_11_14_30_2008

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_RR_H

#undef CDG_ENTER__FILENAME


#include "p_rr.val"

#ifndef __T_kcv__
#define __T_kcv__
/*
 * kc - Value
 * CCDGEN:WriteStruct_Count==2883
 */
typedef struct
{
  U8                        v_kc;                     /*<  0:  1> valid flag                                         */
  U8                        kc[KC_STRING_SIZE];       /*<  1: 16> Kc value                                           */
  U8                        _align0;                  /*< 17:  1> alignment                                          */
  U8                        _align1;                  /*< 18:  1> alignment                                          */
  U8                        _align2;                  /*< 19:  1> alignment                                          */
} T_kcv;
#endif

#ifndef __T_bcch_info__
#define __T_bcch_info__
/*
 * BCCH information
 * CCDGEN:WriteStruct_Count==2884
 */
typedef struct
{
  U8                        v_bcch;                   /*<  0:  1> valid flag                                         */
  U8                        bcch[BA_BITMAP_SIZE];     /*<  1: 16> BCCH carrier list                                  */
  U8                        _align0;                  /*< 17:  1> alignment                                          */
  U8                        _align1;                  /*< 18:  1> alignment                                          */
  U8                        _align2;                  /*< 19:  1> alignment                                          */
} T_bcch_info;
#endif

#ifndef __T_chm__
#define __T_chm__
/*
 * Channel using mode
 * CCDGEN:WriteStruct_Count==2885
 */
typedef struct
{
  U8                        ch_type;                  /*<  0:  1> Channel Type                                       */
  U8                        ch_mode;                  /*<  1:  1> Channel Mode                                       */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_chm;
#endif

#ifndef __T_mm_info__
#define __T_mm_info__
/*
 * MM information
 * CCDGEN:WriteStruct_Count==2886
 */
typedef struct
{
  U8                        valid;                    /*<  0:  1> valid information                                  */
  U8                        la;                       /*<  1:  1> Location area indication                           */
  U8                        att;                      /*<  2:  1> attach / detach flag                               */
  U8                        re;                       /*<  3:  1> re-establishment flag                              */
  U8                        band;                     /*<  4:  1> band (dummy)                                       */
  U8                        ncc;                      /*<  5:  1> national colour code                               */
  U8                        bcc;                      /*<  6:  1> base station colour code                           */
  U8                        t3212;                    /*<  7:  1> periodic updating timer period                     */
} T_mm_info;
#endif

#ifndef __T_imsi_struct__
#define __T_imsi_struct__
/*
 * mobile identity
 * CCDGEN:WriteStruct_Count==2887
 */
typedef struct
{
  U8                        v_mid;                    /*<  0:  1> valid flag                                         */
  U8                        id_type;                  /*<  1:  1> type of identity                                   */
  U8                        id[MAX_DIGITS];           /*<  2: 16> IMSI digits                                        */
  U8                        _align0;                  /*< 18:  1> alignment                                          */
  U8                        _align1;                  /*< 19:  1> alignment                                          */
  U32                       tmsi_dig;                 /*< 20:  4> TMSI digits                                        */
} T_imsi_struct;
#endif

/*
 * mobile identity
 * CCDGEN:WriteStruct_Count==2888
 */
#ifndef __T_tmsi_struct__
#define __T_tmsi_struct__
typedef T_imsi_struct T_tmsi_struct;
#endif
#ifndef __T_op__
#define __T_op__
/*
 * operation mode
 * CCDGEN:WriteStruct_Count==2889
 */
typedef struct
{
  U8                        v_op;                     /*<  0:  1> valid flag                                         */
  U8                        ts;                       /*<  1:  1> test SIM card                                      */
  U8                        m;                        /*<  2:  1> registration mode                                  */
  U8                        sim_ins;                  /*<  3:  1> SIM card                                           */
  U8                        func;                     /*<  4:  1> Operation Mode                                     */
  U8                        service;                  /*<  5:  1> RR Service                                         */
  U8                        _align0;                  /*<  6:  1> alignment                                          */
  U8                        _align1;                  /*<  7:  1> alignment                                          */
} T_op;
#endif

#ifndef __T_plmn__
#define __T_plmn__
/*
 * PLMN identification
 * CCDGEN:WriteStruct_Count==2890
 */
typedef struct
{
  U8                        v_plmn;                   /*<  0:  1> valid flag                                         */
  U8                        mcc[SIZE_MCC];            /*<  1:  3> Mobile country code.                               */
  U8                        mnc[SIZE_MNC];            /*<  4:  3> Mobile network code.                               */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_plmn;
#endif

#ifndef __T_eq_plmn_list__
#define __T_eq_plmn_list__
/*
 * Equivalent plmn List
 * CCDGEN:WriteStruct_Count==2892
 */
typedef struct
{
  U8                        eq_plmn[SIZE_EPLMN];      /*<  0: 18> Basic Element                                      */
  U8                        v_eq_plmn;                /*< 18:  1> Validity of equivalent plmn list                   */
  U8                        _align0;                  /*< 19:  1> alignment                                          */
} T_eq_plmn_list;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_RR_ABORT_REQ__
#define __T_RR_ABORT_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2896
 */
typedef struct
{
  U8                        abcs;                     /*<  0:  1> abort cause                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RR_ABORT_REQ;
#endif

#ifndef __T_RR_ABORT_IND__
#define __T_RR_ABORT_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2897
 */
typedef struct
{
  T_op                      op;                       /*<  0:  8> operation mode                                     */
  U16                       cause;                    /*<  8:  2> RR cause                                           */
  U8                        plmn_avail;               /*< 10:  1> PLMN available                                     */
  U8                        _align0;                  /*< 11:  1> alignment                                          */
  T_plmn                    plmn[MAX_PLMN];           /*< 12: 96> PLMN identification                                */
  U16                       lac_list[MAX_PLMN];       /*<108: 24> LACs of found PLMNs                                */
  U8                        rxlevel[MAX_PLMN];        /*<132: 12> Fieldstrength                                      */
  U8                        power;                    /*<144:  1> Power class                                        */
  U8                        _align1;                  /*<145:  1> alignment                                          */
  U8                        _align2;                  /*<146:  1> alignment                                          */
  U8                        _align3;                  /*<147:  1> alignment                                          */
} T_RR_ABORT_IND;
#endif

#ifndef __T_RR_ACTIVATE_REQ__
#define __T_RR_ACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2898
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> PLMN identification                                */
  T_op                      op;                       /*<  8:  8> operation mode                                     */
  U8                        cksn;                     /*< 16:  1> ciphering key sequence number                      */
  U8                        _align0;                  /*< 17:  1> alignment                                          */
  U8                        _align1;                  /*< 18:  1> alignment                                          */
  U8                        _align2;                  /*< 19:  1> alignment                                          */
  T_kcv                     kcv;                      /*< 20: 20> kc - Value                                         */
  U16                       accc;                     /*< 40:  2> access control classes                             */
  U8                        _align3;                  /*< 42:  1> alignment                                          */
  U8                        _align4;                  /*< 43:  1> alignment                                          */
  T_imsi_struct             imsi_struct;              /*< 44: 24> mobile identity                                    */
  T_tmsi_struct             tmsi_struct;              /*< 68: 24> mobile identity                                    */
  U8                        thplmn;                   /*< 92:  1> HPLN time                                          */
  U8                        _align5;                  /*< 93:  1> alignment                                          */
  U8                        _align6;                  /*< 94:  1> alignment                                          */
  U8                        _align7;                  /*< 95:  1> alignment                                          */
  T_bcch_info               bcch_info;                /*< 96: 20> BCCH information                                   */
  U8                        cell_test;                /*<116:  1> cell test operation                                */
  U8                        gprs_indication;          /*<117:  1> GPRS indicator                                     */
  U8                        _align8;                  /*<118:  1> alignment                                          */
  U8                        _align9;                  /*<119:  1> alignment                                          */
  T_eq_plmn_list            eq_plmn_list;             /*<120: 20> Equivalent plmn List                               */
  U8                        check_hplmn;              /*<140:  1> Flag for HPLMN                                     */
  U8                        _align10;                 /*<141:  1> alignment                                          */
  U8                        _align11;                 /*<142:  1> alignment                                          */
  U8                        _align12;                 /*<143:  1> alignment                                          */
} T_RR_ACTIVATE_REQ;
#endif

#ifndef __T_RR_ACTIVATE_CNF__
#define __T_RR_ACTIVATE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==2899
 */
typedef struct
{
  T_op                      op;                       /*<  0:  8> operation mode                                     */
  T_mm_info                 mm_info;                  /*<  8:  8> MM information                                     */
  U16                       cid;                      /*< 16:  2> cell identity                                      */
  U8                        _align0;                  /*< 18:  1> alignment                                          */
  U8                        _align1;                  /*< 19:  1> alignment                                          */
  T_plmn                    plmn;                     /*< 20:  8> PLMN identification                                */
  U16                       lac;                      /*< 28:  2> location area code                                 */
  U8                        power;                    /*< 30:  1> Power class                                        */
  U8                        gprs_indication;          /*< 31:  1> GPRS indicator                                     */
} T_RR_ACTIVATE_CNF;
#endif

#ifndef __T_RR_ACTIVATE_IND__
#define __T_RR_ACTIVATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2900
 */
typedef struct
{
  T_op                      op;                       /*<  0:  8> operation mode                                     */
  T_mm_info                 mm_info;                  /*<  8:  8> MM information                                     */
  U16                       cid;                      /*< 16:  2> cell identity                                      */
  U8                        _align0;                  /*< 18:  1> alignment                                          */
  U8                        _align1;                  /*< 19:  1> alignment                                          */
  T_plmn                    plmn;                     /*< 20:  8> PLMN identification                                */
  U16                       lac;                      /*< 28:  2> location area code                                 */
  U8                        power;                    /*< 30:  1> Power class                                        */
  U8                        gprs_indication;          /*< 31:  1> GPRS indicator                                     */
} T_RR_ACTIVATE_IND;
#endif

#ifndef __T_RR_DATA_REQ__
#define __T_RR_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2901
 */
typedef struct
{
  U8                        d1;                       /*<  0:  1> dummy, not used                                    */
  U8                        d2;                       /*<  1:  1> dummy, not used                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_RR_DATA_REQ;
#endif

#ifndef __T_RR_DATA_IND__
#define __T_RR_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2902
 */
typedef struct
{
  U8                        d1;                       /*<  0:  1> dummy, not used                                    */
  U8                        d2;                       /*<  1:  1> dummy, not used                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_RR_DATA_IND;
#endif

#ifndef __T_RR_DEACTIVATE_REQ__
#define __T_RR_DEACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2903
 */
typedef struct
{
  U8                        param;                    /*<  0:  1> dummy parameter                                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RR_DEACTIVATE_REQ;
#endif

#ifndef __T_RR_ESTABLISH_REQ__
#define __T_RR_ESTABLISH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2904
 */
typedef struct
{
  U16                       estcs;                    /*<  0:  2> establishment cause                                */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_RR_ESTABLISH_REQ;
#endif

#ifndef __T_RR_ESTABLISH_CNF__
#define __T_RR_ESTABLISH_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==2905
 */
typedef struct
{
  U8                        param;                    /*<  0:  1> dummy parameter                                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RR_ESTABLISH_CNF;
#endif

#ifndef __T_RR_ESTABLISH_IND__
#define __T_RR_ESTABLISH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2906
 */
typedef struct
{
  U8                        param;                    /*<  0:  1> dummy parameter                                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RR_ESTABLISH_IND;
#endif

#ifndef __T_RR_RELEASE_IND__
#define __T_RR_RELEASE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2907
 */
typedef struct
{
  U16                       cause;                    /*<  0:  2> RR cause                                           */
  U8                        sapi;                     /*<  2:  1> service access point identifier                    */
  U8                        gprs_resumption;          /*<  3:  1> GPRS resumption information                        */
} T_RR_RELEASE_IND;
#endif

#ifndef __T_RR_SYNC_REQ__
#define __T_RR_SYNC_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2908
 */
typedef struct
{
  T_op                      op;                       /*<  0:  8> operation mode                                     */
  U8                        cksn;                     /*<  8:  1> ciphering key sequence number                      */
  U8                        _align0;                  /*<  9:  1> alignment                                          */
  U8                        _align1;                  /*< 10:  1> alignment                                          */
  U8                        _align2;                  /*< 11:  1> alignment                                          */
  T_kcv                     kcv;                      /*< 12: 20> kc - Value                                         */
  T_tmsi_struct             tmsi_struct;              /*< 32: 24> mobile identity                                    */
  T_plmn                    plmn;                     /*< 56:  8> PLMN identification                                */
  U16                       lac;                      /*< 64:  2> location area code                                 */
  U16                       synccs;                   /*< 66:  2> synchronisation cause                              */
  U16                       accc;                     /*< 68:  2> access control classes                             */
  U8                        thplmn;                   /*< 70:  1> HPLN time                                          */
  U8                        _align3;                  /*< 71:  1> alignment                                          */
  T_eq_plmn_list            eq_plmn_list;             /*< 72: 20> Equivalent plmn List                               */
} T_RR_SYNC_REQ;
#endif

#ifndef __T_RR_SYNC_IND__
#define __T_RR_SYNC_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2909
 */
typedef struct
{
  U8                        ciph;                     /*<  0:  1> cipher mode                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_mm_info                 mm_info;                  /*<  4:  8> MM information                                     */
  T_bcch_info               bcch_info;                /*< 12: 20> BCCH information                                   */
  U16                       synccs;                   /*< 32:  2> synchronisation cause                              */
  U8                        _align3;                  /*< 34:  1> alignment                                          */
  U8                        _align4;                  /*< 35:  1> alignment                                          */
  T_chm                     chm;                      /*< 36:  4> Channel using mode                                 */
} T_RR_SYNC_IND;
#endif


// PRIM-FF: REL99 
// #ifndef __T_RR_RRLP_START_IND__
// #define __T_RR_RRLP_START_IND__
// 
//  
// CCDGEN:WriteStruct_Count==2910
// 
// typedef struct
// {
//   U8                        param;                    /*<  0:  1> dummy parameter                                    */
//   U8                        _align0;                  /*<  1:  1> alignment                                          */
//   U8                        _align1;                  /*<  2:  1> alignment                                          */
//   U8                        _align2;                  /*<  3:  1> alignment                                          */
// } T_RR_RRLP_START_IND;
// #endif
// 

// PRIM-FF: REL99 
// #ifndef __T_RR_RRLP_STOP_IND__
// #define __T_RR_RRLP_STOP_IND__
// 
//  
// CCDGEN:WriteStruct_Count==2911
// 
// typedef struct
// {
//   U8                        param;                    /*<  0:  1> dummy parameter                                    */
//   U8                        _align0;                  /*<  1:  1> alignment                                          */
//   U8                        _align1;                  /*<  2:  1> alignment                                          */
//   U8                        _align2;                  /*<  3:  1> alignment                                          */
// } T_RR_RRLP_STOP_IND;
// #endif
// 
#ifndef __T_RR_SYNC_HPLMN_REQ__
#define __T_RR_SYNC_HPLMN_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2912
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> PLMN identification                                */
} T_RR_SYNC_HPLMN_REQ;
#endif


#include "CDG_LEAVE.h"


#endif

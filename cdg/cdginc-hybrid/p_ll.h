/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_ll.h                                                         |
| SOURCE  : "sap\ll.pdf"                                                   |
| LastModified : "2003-02-04"                                              |
| IdAndVersion : "8441.104.99.017"                                         |
| SrcFileTime  : "Thu Nov 29 09:44:38 2007"                                |
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


#ifndef P_LL_H
#define P_LL_H


#define CDG_ENTER__P_LL_H

#define CDG_ENTER__FILENAME _P_LL_H
#define CDG_ENTER__P_LL_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_LL_H__LAST_MODIFIED _2003_02_04
#define CDG_ENTER__P_LL_H__ID_AND_VERSION _8441_104_99_017

#define CDG_ENTER__P_LL_H__SRC_FILE_TIME _Thu_Nov_29_09_44_38_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_LL_H

#undef CDG_ENTER__FILENAME


#include "p_ll.val"

#ifndef __T_reference1__
#define __T_reference1__
/*
 * to the segment to be confirmed
 * CCDGEN:WriteStruct_Count==1727
 */
typedef struct
{
  U8                        ref_nsapi;                /*<  0:  1> ref_nsapi                                          */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       ref_npdu_num;             /*<  2:  2> ref_npdu_num                                       */
  U8                        ref_seg_num;              /*<  4:  1> ref_seg_num                                        */
  U8                        _align1;                  /*<  5:  1> alignment                                          */
  U8                        _align2;                  /*<  6:  1> alignment                                          */
  U8                        _align3;                  /*<  7:  1> alignment                                          */
} T_reference1;
#endif

/*
 * reserved octets
 * CCDGEN:WriteStruct_Count==1728
 */
#ifndef __T_reserved_unitdata_req1__
#define __T_reserved_unitdata_req1__
typedef T_reference1 T_reserved_unitdata_req1;
#endif
/*
 * reserved octets
 * CCDGEN:WriteStruct_Count==1729
 */
#ifndef __T_reserved_data_ind2__
#define __T_reserved_data_ind2__
typedef T_reference1 T_reserved_data_ind2;
#endif
/*
 * reserved octets
 * CCDGEN:WriteStruct_Count==1730
 */
#ifndef __T_reserved_unitdata_ind2__
#define __T_reserved_unitdata_ind2__
typedef T_reference1 T_reserved_unitdata_ind2;
#endif
#ifndef __T_ll_qos__
#define __T_ll_qos__
/*
 * quality of service
 * CCDGEN:WriteStruct_Count==1731
 */
typedef struct
{
  U8                        delay;                    /*<  0:  1> delay class                                        */
  U8                        relclass;                 /*<  1:  1> reliability class                                  */
  U8                        peak;                     /*<  2:  1> peak throughput                                    */
  U8                        preced;                   /*<  3:  1> precedence class                                   */
  U8                        mean;                     /*<  4:  1> main throughput                                    */
  U8                        reserved_1;               /*<  5:  1> reserved                                           */
  U8                        reserved_2;               /*<  6:  1> reserved                                           */
  U8                        reserved_3;               /*<  7:  1> reserved                                           */
} T_ll_qos;
#endif

#ifndef __T_desc_list3__
#define __T_desc_list3__
/*
 * List of generic data descriptors
 * CCDGEN:WriteStruct_Count==1732
 */
typedef struct
{
  U16                       list_len;                 /*<  0:  2> length in octets of whole data                     */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  U32                       first;                    /*<  4:  4> pointer to first generic data descriptor           */
} T_desc_list3;
#endif

#ifndef __T_desc3__
#define __T_desc3__
/*
 * generic data descriptor
 * CCDGEN:WriteStruct_Count==1733
 */
typedef struct
{
  U32                       next;                     /*<  0:  4> next generic data descriptor                       */
  U16                       offset;                   /*<  4:  2> offset in octets                                   */
  U16                       len;                      /*<  6:  2> length of content in octets                        */
  U32                       buffer;                   /*<  8:  4> pointer to buffer                                  */
} T_desc3;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_LL_RESET_IND__
#define __T_LL_RESET_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1746
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LL_RESET_IND;
#endif

#ifndef __T_LL_ESTABLISH_REQ__
#define __T_LL_ESTABLISH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1747
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > service data unit                                  */
} T_LL_ESTABLISH_REQ;
#endif

#ifndef __T_LL_ESTABLISH_CNF__
#define __T_LL_ESTABLISH_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1748
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       n201_u;                   /*<  2:  2> maximum number of octets in an U or UI frame information field */
  U16                       n201_i;                   /*<  4:  2> maximum number of octets in an I frame information field */
  U8                        xid_valid;                /*<  6:  1> layer-3 XID parameters valid or not                */
  U8                        _align1;                  /*<  7:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  8: ? > service data unit                                  */
} T_LL_ESTABLISH_CNF;
#endif

#ifndef __T_LL_ESTABLISH_IND__
#define __T_LL_ESTABLISH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1749
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       n201_u;                   /*<  2:  2> maximum number of octets in an U or UI frame information field */
  U16                       n201_i;                   /*<  4:  2> maximum number of octets in an I frame information field */
  U8                        xid_valid;                /*<  6:  1> layer-3 XID parameters valid or not                */
  U8                        _align1;                  /*<  7:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  8: ? > service data unit                                  */
} T_LL_ESTABLISH_IND;
#endif

#ifndef __T_LL_ESTABLISH_RES__
#define __T_LL_ESTABLISH_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1750
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        xid_valid;                /*<  1:  1> layer-3 XID parameters valid or not                */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > service data unit                                  */
} T_LL_ESTABLISH_RES;
#endif

#ifndef __T_LL_RELEASE_REQ__
#define __T_LL_RELEASE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1751
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        local;                    /*<  1:  1> local release or release both sides                */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_LL_RELEASE_REQ;
#endif

#ifndef __T_LL_RELEASE_CNF__
#define __T_LL_RELEASE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1752
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        reserved_release_cnf;     /*<  1:  1> reserved octets for release confirm                */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_LL_RELEASE_CNF;
#endif

#ifndef __T_LL_RELEASE_IND__
#define __T_LL_RELEASE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1753
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       cause;                    /*<  2:  2> cause of ABM termination                           */
} T_LL_RELEASE_IND;
#endif

#ifndef __T_LL_XID_REQ__
#define __T_LL_XID_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1754
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > service data unit                                  */
} T_LL_XID_REQ;
#endif

#ifndef __T_LL_XID_CNF__
#define __T_LL_XID_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1755
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       n201_u;                   /*<  2:  2> maximum number of octets in an U or UI frame information field */
  U16                       n201_i;                   /*<  4:  2> maximum number of octets in an I frame information field */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  8: ? > service data unit                                  */
} T_LL_XID_CNF;
#endif

#ifndef __T_LL_XID_IND__
#define __T_LL_XID_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1756
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       n201_u;                   /*<  2:  2> maximum number of octets in an U or UI frame information field */
  U16                       n201_i;                   /*<  4:  2> maximum number of octets in an I frame information field */
  U8                        xid_valid;                /*<  6:  1> layer-3 XID parameters valid or not                */
  U8                        _align1;                  /*<  7:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  8: ? > service data unit                                  */
} T_LL_XID_IND;
#endif

#ifndef __T_LL_XID_RES__
#define __T_LL_XID_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1757
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > service data unit                                  */
} T_LL_XID_RES;
#endif

#ifndef __T_LL_READY_IND__
#define __T_LL_READY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1758
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LL_READY_IND;
#endif

#ifndef __T_LL_UNITREADY_IND__
#define __T_LL_UNITREADY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1759
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LL_UNITREADY_IND;
#endif

#ifndef __T_LL_GETDATA_REQ__
#define __T_LL_GETDATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1760
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LL_GETDATA_REQ;
#endif

#ifndef __T_LL_GETUNITDATA_REQ__
#define __T_LL_GETUNITDATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1761
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
} T_LL_GETUNITDATA_REQ;
#endif

#ifndef __T_LL_DATA_REQ__
#define __T_LL_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1762
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  T_ll_qos                  ll_qos;                   /*<  8:  8> quality of service                                 */
  U8                        radio_prio;               /*< 16:  1> Radio Priority                                     */
  U8                        reserved_data_req1;       /*< 17:  1> reserved octets                                    */
  U8                        _align3;                  /*< 18:  1> alignment                                          */
  U8                        _align4;                  /*< 19:  1> alignment                                          */
  T_reference1              reference1;               /*< 20:  8> to the segment to be confirmed                     */
  U8                        seg_pos;                  /*< 28:  1> First and/or last segment of N-PDU?                */
  U8                        attached_counter;         /*< 29:  1> attached to primitive data counter                 */
  U8                        _align5;                  /*< 30:  1> alignment                                          */
  U8                        _align6;                  /*< 31:  1> alignment                                          */
  U32                       reserved_data_req4;       /*< 32:  4> reserved octets for data request                   */
// ELEM-FF: REL99 
//   U16                       pkt_flow_id;              /*<  0:  0> packet flow identifier                             */
// ELEM-FF: !REL99 
  U16                       reserved_data_req5;       /*< 36:  2> reserved octets                                    */
  U8                        _align7;                  /*< 38:  1> alignment                                          */
  U8                        _align8;                  /*< 39:  1> alignment                                          */
  T_sdu                     sdu;                      /*< 40: ? > service data unit                                  */
} T_LL_DATA_REQ;
#endif

#ifndef __T_LL_DATA_CNF__
#define __T_LL_DATA_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1763
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        c_reference1;             /*<  3:  1> counter                                            */
  T_reference1              reference1[LLC_MAX_CNF];  /*<  4:256> to the segment to be confirmed                     */
} T_LL_DATA_CNF;
#endif

#ifndef __T_LL_DATA_IND__
#define __T_LL_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1764
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  U32                       reserved_data_ind1;       /*<  8:  4> reserved octets for data indication                */
  T_reserved_data_ind2      reserved_data_ind2;       /*< 12:  8> reserved octets                                    */
  U8                        reserved_data_ind3;       /*< 20:  1> reserved octets for data indication                */
  U8                        reserved_data_ind4;       /*< 21:  1> reserved octets for data indication                */
  U8                        reserved_data_ind5;       /*< 22:  1> reserved octets for data indication                */
  U8                        reserved_data_ind6;       /*< 23:  1> reserved octets for data indication                */
  T_sdu                     sdu;                      /*< 24: ? > service data unit                                  */
} T_LL_DATA_IND;
#endif

#ifndef __T_LL_UNITDATA_REQ__
#define __T_LL_UNITDATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1765
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  T_ll_qos                  ll_qos;                   /*<  8:  8> quality of service                                 */
  U8                        radio_prio;               /*< 16:  1> Radio Priority                                     */
  U8                        cipher;                   /*< 17:  1> ciphering indicator                                */
  U8                        _align3;                  /*< 18:  1> alignment                                          */
  U8                        _align4;                  /*< 19:  1> alignment                                          */
  T_reserved_unitdata_req1  reserved_unitdata_req1;   /*< 20:  8> reserved octets                                    */
  U8                        seg_pos;                  /*< 28:  1> First and/or last segment of N-PDU?                */
  U8                        attached_counter;         /*< 29:  1> attached to primitive data counter                 */
  U8                        _align5;                  /*< 30:  1> alignment                                          */
  U8                        _align6;                  /*< 31:  1> alignment                                          */
  U32                       reserved_unitdata_req4;   /*< 32:  4> reserved octets for unitdata request               */
// ELEM-FF: REL99 
//   U16                       pkt_flow_id;              /*<  0:  0> packet flow identifier                             */
// ELEM-FF: !REL99 
  U16                       reserved_unitdata_req5;   /*< 36:  2> reserved octets                                    */
  U8                        _align7;                  /*< 38:  1> alignment                                          */
  U8                        _align8;                  /*< 39:  1> alignment                                          */
  T_sdu                     sdu;                      /*< 40: ? > service data unit                                  */
} T_LL_UNITDATA_REQ;
#endif

#ifndef __T_LL_UNITDATA_IND__
#define __T_LL_UNITDATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1766
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  U32                       reserved_unitdata_ind1;   /*<  8:  4> reserved octets                                    */
  T_reserved_unitdata_ind2  reserved_unitdata_ind2;   /*< 12:  8> reserved octets                                    */
  U8                        reserved_unitdata_ind3;   /*< 20:  1> reserved octets                                    */
  U8                        reserved_unitdata_ind4;   /*< 21:  1> reserved octets                                    */
  U8                        reserved_unitdata_ind5;   /*< 22:  1> reserved octets                                    */
  U8                        cipher;                   /*< 23:  1> ciphering indicator                                */
  T_sdu                     sdu;                      /*< 24: ? > service data unit                                  */
} T_LL_UNITDATA_IND;
#endif

#ifndef __T_LL_STATUS_IND__
#define __T_LL_STATUS_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1767
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       error_cause;              /*<  2:  2> error cause                                        */
} T_LL_STATUS_IND;
#endif

#ifndef __T_LL_DESC_REQ__
#define __T_LL_DESC_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1768
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  T_ll_qos                  ll_qos;                   /*<  8:  8> quality of service                                 */
  U8                        radio_prio;               /*< 16:  1> Radio Priority                                     */
  U8                        reserved_data_req1;       /*< 17:  1> reserved octets                                    */
  U8                        _align3;                  /*< 18:  1> alignment                                          */
  U8                        _align4;                  /*< 19:  1> alignment                                          */
  T_reference1              reference1;               /*< 20:  8> to the segment to be confirmed                     */
  U8                        seg_pos;                  /*< 28:  1> First and/or last segment of N-PDU?                */
  U8                        attached_counter;         /*< 29:  1> attached to primitive data counter                 */
  U8                        _align5;                  /*< 30:  1> alignment                                          */
  U8                        _align6;                  /*< 31:  1> alignment                                          */
  U32                       reserved_data_req4;       /*< 32:  4> reserved octets for data request                   */
// ELEM-FF: REL99 
//   U16                       pkt_flow_id;              /*<  0:  0> packet flow identifier                             */
// ELEM-FF: !REL99 
  U16                       reserved_data_req5;       /*< 36:  2> reserved octets                                    */
  U8                        _align7;                  /*< 38:  1> alignment                                          */
  U8                        _align8;                  /*< 39:  1> alignment                                          */
  T_desc_list3              desc_list3;               /*< 40:  8> List of generic data descriptors                   */
} T_LL_DESC_REQ;
#endif

#ifndef __T_LL_UNITDESC_REQ__
#define __T_LL_UNITDESC_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1769
 */
typedef struct
{
  U8                        sapi;                     /*<  0:  1> service access point identifier                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  U32                       tlli;                     /*<  4:  4> temporary logical link identifier                  */
  T_ll_qos                  ll_qos;                   /*<  8:  8> quality of service                                 */
  U8                        radio_prio;               /*< 16:  1> Radio Priority                                     */
  U8                        cipher;                   /*< 17:  1> ciphering indicator                                */
  U8                        _align3;                  /*< 18:  1> alignment                                          */
  U8                        _align4;                  /*< 19:  1> alignment                                          */
  T_reserved_unitdata_req1  reserved_unitdata_req1;   /*< 20:  8> reserved octets                                    */
  U8                        seg_pos;                  /*< 28:  1> First and/or last segment of N-PDU?                */
  U8                        attached_counter;         /*< 29:  1> attached to primitive data counter                 */
  U8                        _align5;                  /*< 30:  1> alignment                                          */
  U8                        _align6;                  /*< 31:  1> alignment                                          */
  U32                       reserved_unitdata_req4;   /*< 32:  4> reserved octets for unitdata request               */
// ELEM-FF: REL99 
//   U16                       pkt_flow_id;              /*<  0:  0> packet flow identifier                             */
// ELEM-FF: !REL99 
  U16                       reserved_unitdata_req5;   /*< 36:  2> reserved octets                                    */
  U8                        _align7;                  /*< 38:  1> alignment                                          */
  U8                        _align8;                  /*< 39:  1> alignment                                          */
  T_desc_list3              desc_list3;               /*< 40:  8> List of generic data descriptors                   */
} T_LL_UNITDESC_REQ;
#endif

#ifndef __T_LLC_DUMMY_REQ__
#define __T_LLC_DUMMY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1770
 */
typedef struct
{
  T_desc3                   desc3;                    /*<  0: 12> generic data descriptor                            */
} T_LLC_DUMMY_REQ;
#endif


#include "CDG_LEAVE.h"


#endif

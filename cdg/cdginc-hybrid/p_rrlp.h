/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_rrlp.h                                                       |
| SOURCE  : "sap\rrlp.pdf"                                                 |
| LastModified : "2002-11-13"                                              |
| IdAndVersion : "8443.102.02.100"                                         |
| SrcFileTime  : "Thu Nov 29 09:53:06 2007"                                |
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


#ifndef P_RRLP_H
#define P_RRLP_H


#define CDG_ENTER__P_RRLP_H

#define CDG_ENTER__FILENAME _P_RRLP_H
#define CDG_ENTER__P_RRLP_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_RRLP_H__LAST_MODIFIED _2002_11_13
#define CDG_ENTER__P_RRLP_H__ID_AND_VERSION _8443_102_02_100

#define CDG_ENTER__P_RRLP_H__SRC_FILE_TIME _Thu_Nov_29_09_53_06_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_RRLP_H

#undef CDG_ENTER__FILENAME


#include "p_rrlp.val"

#ifndef __T_assist_data__
#define __T_assist_data__
/*
 * Assistance Data
 * CCDGEN:WriteStruct_Count==2994
 */
typedef struct
{
  U16                       arfcn;                    /*<  0:  2> Channel number                                     */
  U8                        bsic;                     /*<  2:  1> Base station ID code                               */
  U8                        mfrm_offset;              /*<  3:  1> multiframe offset                                  */
  U8                        otd_type;                 /*<  4:  1> OTD type                                           */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U16                       exp_otd;                  /*<  6:  2> expectedOTD                                        */
  U8                        uncertainty;              /*<  8:  1> Uncertainty of expected OTD                        */
  U8                        _align1;                  /*<  9:  1> alignment                                          */
  U16                       rough_rtd;                /*< 10:  2> rough RTD                                          */
} T_assist_data;
#endif

#ifndef __T_bts_data__
#define __T_bts_data__
/*
 * BTS data
 * CCDGEN:WriteStruct_Count==2995
 */
typedef struct
{
  U8                        bsic;                     /*<  0:  1> Base station ID code                               */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       arfcn;                    /*<  2:  2> Channel number                                     */
  U8                        quality;                  /*<  4:  1> 5 bit encoding: ranges                             */
  U8                        num_measurements;         /*<  5:  1> Number of measurements                             */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
  S32                       timing_offset;            /*<  8:  4> Timing offset                                      */
  S32                       timing_advance;           /*< 12:  4> Timing advance                                     */
} T_bts_data;
#endif

#ifndef __T_ref_bts_data__
#define __T_ref_bts_data__
/*
 * Reference BTS data
 * CCDGEN:WriteStruct_Count==2996
 */
typedef struct
{
  U16                       cell_id;                  /*<  0:  2> Cell ID                                            */
  U16                       lac;                      /*<  2:  2> Location area code                                 */
  U8                        quality;                  /*<  4:  1> 5 bit encoding: ranges                             */
  U8                        num_measurements;         /*<  5:  1> Number of measurements                             */
  U8                        _align0;                  /*<  6:  1> alignment                                          */
  U8                        _align1;                  /*<  7:  1> alignment                                          */
  S32                       timing_offset;            /*<  8:  4> Timing offset                                      */
  U8                        total_neigh_bts;          /*< 12:  1> Total number of neighbor BTS                       */
  U8                        _align2;                  /*< 13:  1> alignment                                          */
  U8                        _align3;                  /*< 14:  1> alignment                                          */
  U8                        _align4;                  /*< 15:  1> alignment                                          */
  T_bts_data                bts_data[R_MAX_REF_NEIGH_BTS]; /*< 16:240> BTS data                                           */
  U16                       frame_number;             /*<256:  2> Frame number                                       */
  U8                        _align5;                  /*<258:  1> alignment                                          */
  U8                        _align6;                  /*<259:  1> alignment                                          */
  S32                       timing_advance;           /*<260:  4> Timing advance                                     */
} T_ref_bts_data;
#endif

#ifndef __T_timing_data__
#define __T_timing_data__
/*
 * Output of the position algorithm
 * CCDGEN:WriteStruct_Count==2997
 */
typedef struct
{
  U16                       mcc;                      /*<  0:  2> Mobile country code                                */
  U16                       mnc;                      /*<  2:  2> Mobile country code                                */
  U8                        reference_relation;       /*<  4:  1> 0, 1 or 2, only used when there are 2 ref cells and 3 measurement sets */
  U8                        time_slot;                /*<  5:  1> Time slot number                                   */
  U8                        std_resolution;           /*<  6:  1> Standard resolution in meters                      */
  U8                        num_measurement_sets;     /*<  7:  1> Number of measurement sets                         */
  U8                        num_reference_cells;      /*<  8:  1> Number of reference cells                          */
  U8                        ta_correction_present;    /*<  9:  1>                                                    */
  U16                       ta_correction;            /*< 10:  2> TA correction                                      */
  T_ref_bts_data            ref_bts_data[R_NUM_MEAS_SETS]; /*< 12:792> Reference BTS data                                 */
} T_timing_data;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_RRLP_POS_IND__
#define __T_RRLP_POS_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2998
 */
typedef struct
{
  U8                        _align0;                  /*<  0:  1> alignment                                          */
  U8                        _align1;                  /*<  1:  1> alignment                                          */
  U8                        v_assist_data;            /*<  2:  1> valid-flag                                         */
  U8                        c_assist_data;            /*<  3:  1> counter                                            */
  T_assist_data             assist_data[MAX_NCELL_EOTD]; /*<  4:180> Assistance Data                                    */
  U8                        loc_method;               /*<184:  1> Location method                                    */
  U8                        pos_method;               /*<185:  1> Position method                                    */
  U8                        _align2;                  /*<186:  1> alignment                                          */
  U8                        v_arfcn;                  /*<187:  1> valid-flag                                         */
  U16                       arfcn;                    /*<188:  2> Channel number                                     */
  U8                        v_bsic;                   /*<190:  1> valid-flag                                         */
  U8                        bsic;                     /*<191:  1> Base station ID code                               */
} T_RRLP_POS_IND;
#endif

#ifndef __T_RRLP_POS_RES__
#define __T_RRLP_POS_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==2999
 */
typedef struct
{
  T_timing_data             timing_data;              /*<  0:804> Output of the position algorithm                   */
} T_RRLP_POS_RES;
#endif

#ifndef __T_RRLP_ERROR_REQ__
#define __T_RRLP_ERROR_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==3000
 */
typedef struct
{
  U8                        cause;                    /*<  0:  1> Cause Id Error                                     */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RRLP_ERROR_REQ;
#endif


#include "CDG_LEAVE.h"


#endif

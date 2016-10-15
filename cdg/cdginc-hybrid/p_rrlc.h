/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_rrlc.h                                                       |
| SOURCE  : "sap\rrlc.pdf"                                                 |
| LastModified : "2002-10-11"                                              |
| IdAndVersion : "8443.101.02.008"                                         |
| SrcFileTime  : "Thu Nov 29 09:52:52 2007"                                |
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


#ifndef P_RRLC_H
#define P_RRLC_H


#define CDG_ENTER__P_RRLC_H

#define CDG_ENTER__FILENAME _P_RRLC_H
#define CDG_ENTER__P_RRLC_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_RRLC_H__LAST_MODIFIED _2002_10_11
#define CDG_ENTER__P_RRLC_H__ID_AND_VERSION _8443_101_02_008

#define CDG_ENTER__P_RRLC_H__SRC_FILE_TIME _Thu_Nov_29_09_52_52_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_RRLC_H

#undef CDG_ENTER__FILENAME


#include "p_rrlc.val"

#ifndef __T_assist_data__
#define __T_assist_data__
/*
 * Assistance Data
 * CCDGEN:WriteStruct_Count==2987
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

#ifndef __T_eotd_sc_res__
#define __T_eotd_sc_res__
/*
 * EOTD SC Results at start of measurement
 * CCDGEN:WriteStruct_Count==2988
 */
typedef struct
{
  U8                        sb_flag;                  /*<  0:  1> data on SCH could be decoded, 'sb' stems from Sch Burst */
  U8                        bsic;                     /*<  1:  1> Base station ID code                               */
  U16                       arfcn;                    /*<  2:  2> Channel number                                     */
  S16                       eotd_crosscor[XCOR_NO];   /*<  4: 36> Cross-correlation data                             */
  U32                       d_eotd_nrj;               /*< 40:  4> Signal level                                       */
  U32                       time_tag;                 /*< 44:  4> Nominal Position                                   */
} T_eotd_sc_res;
#endif

/*
 * EOTD SC Results at stop of measurement
 * CCDGEN:WriteStruct_Count==2989
 */
#ifndef __T_eotd_sc_res1__
#define __T_eotd_sc_res1__
typedef T_eotd_sc_res T_eotd_sc_res1;
#endif
/*
 * EOTD NC Results
 * CCDGEN:WriteStruct_Count==2990
 */
#ifndef __T_eotd_nc_res__
#define __T_eotd_nc_res__
typedef T_eotd_sc_res T_eotd_nc_res;
#endif

/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_RRLC_MEAS_REQ__
#define __T_RRLC_MEAS_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2991
 */
typedef struct
{
  U16                       req_id;                   /*<  0:  2> Request Identifier                                 */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        v_arfcn;                  /*<  3:  1> valid-flag                                         */
  U16                       arfcn;                    /*<  4:  2> Channel number                                     */
  U8                        v_bsic;                   /*<  6:  1> valid-flag                                         */
  U8                        bsic;                     /*<  7:  1> Base station ID code                               */
  U8                        _align1;                  /*<  8:  1> alignment                                          */
  U8                        _align2;                  /*<  9:  1> alignment                                          */
  U8                        v_assist_data;            /*< 10:  1> valid-flag                                         */
  U8                        c_assist_data;            /*< 11:  1> counter                                            */
  T_assist_data             assist_data[MAX_NCELL_EOTD]; /*< 12:180> Assistance Data                                    */
} T_RRLC_MEAS_REQ;
#endif

#ifndef __T_RRLC_MEAS_IND__
#define __T_RRLC_MEAS_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2992
 */
typedef struct
{
  U8                        mcc[3];                   /*<  0:  3> Mobile country code                                */
  U8                        mnc[3];                   /*<  3:  3> Mobile country code                                */
  U16                       lac;                      /*<  6:  2> Location area code                                 */
  U16                       cell_id;                  /*<  8:  2> Cell ID                                            */
  U8                        eotd_mode;                /*< 10:  1> Operation mode                                     */
  U8                        _align0;                  /*< 11:  1> alignment                                          */
  U16                       req_id;                   /*< 12:  2> Request Identifier                                 */
  U8                        tav;                      /*< 14:  1> Timing advance                                     */
  U8                        _align1;                  /*< 15:  1> alignment                                          */
  U32                       fn;                       /*< 16:  4> full frame number                                  */
  T_eotd_sc_res             eotd_sc_res;              /*< 20: 48> EOTD SC Results at start of measurement            */
  T_eotd_sc_res1            eotd_sc_res1;             /*< 68: 48> EOTD SC Results at stop of measurement             */
  U8                        _align2;                  /*<116:  1> alignment                                          */
  U8                        _align3;                  /*<117:  1> alignment                                          */
  U8                        _align4;                  /*<118:  1> alignment                                          */
  U8                        c_eotd_nc_res;            /*<119:  1> counter                                            */
  T_eotd_nc_res             eotd_nc_res[MAX_NCELL_EOTD_L1]; /*<120:576> EOTD NC Results                                    */
} T_RRLC_MEAS_IND;
#endif

#ifndef __T_RRLC_ERROR_IND__
#define __T_RRLC_ERROR_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2993
 */
typedef struct
{
  U8                        cause;                    /*<  0:  1> Cause Identifier                                   */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RRLC_ERROR_IND;
#endif


#include "CDG_LEAVE.h"


#endif

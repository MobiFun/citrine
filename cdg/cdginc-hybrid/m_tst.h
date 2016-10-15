/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : m_tst.h                                                        |
| SOURCE  : "msg\tst.mdf"                                                  |
| LastModified : "2000-10-23"                                              |
| IdAndVersion : "8441.603.99.005"                                         |
| SrcFileTime  : "Wed Nov 28 10:21:30 2007"                                |
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


#ifndef M_TST_H
#define M_TST_H


#define CDG_ENTER__M_TST_H

#define CDG_ENTER__FILENAME _M_TST_H
#define CDG_ENTER__M_TST_H__FILE_TYPE CDGINC
#define CDG_ENTER__M_TST_H__LAST_MODIFIED _2000_10_23
#define CDG_ENTER__M_TST_H__ID_AND_VERSION _8441_603_99_005

#define CDG_ENTER__M_TST_H__SRC_FILE_TIME _Wed_Nov_28_10_21_30_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__M_TST_H

#undef CDG_ENTER__FILENAME


#include "m_tst.val"

#ifndef __T_pdu_description__
#define __T_pdu_description__
/*
 * PDU Description
 * CCDGEN:WriteStruct_Count==772
 */
typedef struct
{
  U16                       no_of_pdus;               /*<  0:  2> Number of PDUs                                     */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_pdu_description;
#endif

#ifndef __T_pdu_description_ie__
#define __T_pdu_description_ie__
/*
 * PDU Description IE
 * CCDGEN:WriteStruct_Count==773
 */
typedef struct
{
  U8                        _align0;                  /*<  0:  1> alignment                                          */
  U8                        _align1;                  /*<  1:  1> alignment                                          */
  U8                        _align2;                  /*<  2:  1> alignment                                          */
  U8                        v_pdu_description;        /*<  3:  1> valid-flag                                         */
  T_pdu_description         pdu_description;          /*<  4:  4> PDU Description                                    */
} T_pdu_description_ie;
#endif

#ifndef __T_mode_flag__
#define __T_mode_flag__
/*
 * Mode Flag
 * CCDGEN:WriteStruct_Count==774
 */
typedef struct
{
  U8                        dl_timeslot_offset;       /*<  0:  1> Downlink Timeslot Offset                           */
  U8                        mode_flag_val;            /*<  1:  1> Mode Flag Value                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_mode_flag;
#endif


/*
 * End of substructure section, begin of message definition section
 */

#ifndef __T_GPRS_TEST_MODE_CMD__
#define __T_GPRS_TEST_MODE_CMD__
/*
 * 
 * CCDGEN:WriteStruct_Count==775
 */
typedef struct
{
  U8                        msg_type;                 /*<  0:  1> Message Type                                       */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_pdu_description_ie      pdu_description_ie;       /*<  4:  8> PDU Description IE                                 */
  T_mode_flag               mode_flag;                /*< 12:  4> Mode Flag                                          */
} T_GPRS_TEST_MODE_CMD;
#endif


#include "CDG_LEAVE.h"


#endif

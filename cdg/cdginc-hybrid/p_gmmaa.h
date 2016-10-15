/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_gmmaa.h                                                      |
| SOURCE  : "sap\gmmaa.pdf"                                                |
| LastModified : "1999-06-22"                                              |
| IdAndVersion : "8441.107.99.001"                                         |
| SrcFileTime  : "Thu Nov 29 09:41:58 2007"                                |
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


#ifndef P_GMMAA_H
#define P_GMMAA_H


#define CDG_ENTER__P_GMMAA_H

#define CDG_ENTER__FILENAME _P_GMMAA_H
#define CDG_ENTER__P_GMMAA_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_GMMAA_H__LAST_MODIFIED _1999_06_22
#define CDG_ENTER__P_GMMAA_H__ID_AND_VERSION _8441_107_99_001

#define CDG_ENTER__P_GMMAA_H__SRC_FILE_TIME _Thu_Nov_29_09_41_58_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_GMMAA_H

#undef CDG_ENTER__FILENAME


#include "p_gmmaa.val"


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_GMMAA_ESTABLISH_REQ__
#define __T_GMMAA_ESTABLISH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1566
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_GMMAA_ESTABLISH_REQ;
#endif

#ifndef __T_GMMAA_RELEASE_IND__
#define __T_GMMAA_RELEASE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1567
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_GMMAA_RELEASE_IND;
#endif

#ifndef __T_GMMAA_ESTABLISH_REJ__
#define __T_GMMAA_ESTABLISH_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1568
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_GMMAA_ESTABLISH_REJ;
#endif

#ifndef __T_GMMAA_TIMER_REQ__
#define __T_GMMAA_TIMER_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1569
 */
typedef struct
{
  U8                        aa_timer;                 /*<  0:  1> GPRS READY timer                                   */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_GMMAA_TIMER_REQ;
#endif


#include "CDG_LEAVE.h"


#endif

/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_cl.h                                                         |
| SOURCE  : "sap\cl.pdf"                                                   |
| LastModified : "2004-06-08"                                              |
| IdAndVersion : "8010.149.04.012"                                         |
| SrcFileTime  : "Thu Nov 29 09:38:38 2007"                                |
| Generated by CCDGEN_2.5.5A on Fri Oct 14 21:41:52 2016                   |
|           !!DO NOT MODIFY!!DO NOT MODIFY!!DO NOT MODIFY!!                |
+--------------------------------------------------------------------------+
*/

/* PRAGMAS
 * PREFIX                 : CL
 * COMPATIBILITY_DEFINES  : NO
 * ALWAYS_ENUM_IN_VAL_FILE: NO
 * ENABLE_GROUP: NO
 * CAPITALIZE_TYPENAME: NO
 */


#ifndef P_CL_H
#define P_CL_H


#define CDG_ENTER__P_CL_H

#define CDG_ENTER__FILENAME _P_CL_H
#define CDG_ENTER__P_CL_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_CL_H__LAST_MODIFIED _2004_06_08
#define CDG_ENTER__P_CL_H__ID_AND_VERSION _8010_149_04_012

#define CDG_ENTER__P_CL_H__SRC_FILE_TIME _Thu_Nov_29_09_38_38_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_CL_H

#undef CDG_ENTER__FILENAME


#include "p_cl.val"

#include "p_8010_152_ps_include.h"


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_CL_NWRL_SET_SGSN_RELEASE_REQ__
#define __T_CL_NWRL_SET_SGSN_RELEASE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1675
 */
typedef struct
{
  U8                        sgsn_rel;                 /*<  0:  1> sgsn release version                               */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CL_NWRL_SET_SGSN_RELEASE_REQ;
#endif

#ifndef __T_CL_NWRL_SET_SGSN_RELEASE_CNF__
#define __T_CL_NWRL_SET_SGSN_RELEASE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1676
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CL_NWRL_SET_SGSN_RELEASE_CNF;
#endif

#ifndef __T_CL_NWRL_GET_SGSN_RELEASE_REQ__
#define __T_CL_NWRL_GET_SGSN_RELEASE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1677
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_CL_NWRL_GET_SGSN_RELEASE_REQ;
#endif

#ifndef __T_CL_NWRL_GET_SGSN_RELEASE_CNF__
#define __T_CL_NWRL_GET_SGSN_RELEASE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1678
 */
typedef struct
{
  U8                        sgsn_rel;                 /*<  0:  1> sgsn release version                               */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_CL_NWRL_GET_SGSN_RELEASE_CNF;
#endif


#include "CDG_LEAVE.h"


#endif
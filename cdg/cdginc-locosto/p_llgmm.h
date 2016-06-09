/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_llgmm.h                                                      |
| SOURCE  : "sap\llgmm.pdf"                                                |
| LastModified : "2003-03-21"                                              |
| IdAndVersion : "8441.103.99.014"                                         |
| SrcFileTime  : "Thu Nov 29 09:44:54 2007"                                |
| Generated by CCDGEN_2.5.5A on Thu Sep 25 09:18:53 2014                   |
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


#ifndef P_LLGMM_H
#define P_LLGMM_H


#define CDG_ENTER__P_LLGMM_H

#define CDG_ENTER__FILENAME _P_LLGMM_H
#define CDG_ENTER__P_LLGMM_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_LLGMM_H__LAST_MODIFIED _2003_03_21
#define CDG_ENTER__P_LLGMM_H__ID_AND_VERSION _8441_103_99_014

#define CDG_ENTER__P_LLGMM_H__SRC_FILE_TIME _Thu_Nov_29_09_44_54_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_LLGMM_H

#undef CDG_ENTER__FILENAME


#include "p_llgmm.val"

#ifndef __T_llgmm_kc__
#define __T_llgmm_kc__
/*
 * GPRS ciphering key
 * CCDGEN:WriteStruct_Count==1771
 */
typedef struct
{
  U8                        key[8];                   /*<  0:  8> ciphering key content                              */
} T_llgmm_kc;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_LLGMM_ASSIGN_REQ__
#define __T_LLGMM_ASSIGN_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1772
 */
typedef struct
{
  U32                       old_tlli;                 /*<  0:  4> old temporary logical link identifier              */
  U32                       new_tlli;                 /*<  4:  4> new temporary logical link identifier              */
  T_llgmm_kc                llgmm_kc;                 /*<  8:  8> GPRS ciphering key                                 */
  U8                        ciphering_algorithm;      /*< 16:  1> ciphering algorithm                                */
  U8                        _align0;                  /*< 17:  1> alignment                                          */
  U8                        _align1;                  /*< 18:  1> alignment                                          */
  U8                        _align2;                  /*< 19:  1> alignment                                          */
} T_LLGMM_ASSIGN_REQ;
#endif

#ifndef __T_LLGMM_TRIGGER_REQ__
#define __T_LLGMM_TRIGGER_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1773
 */
typedef struct
{
  U8                        trigger_cause;            /*<  0:  1> cause of the trigger primitive                     */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LLGMM_TRIGGER_REQ;
#endif

#ifndef __T_LLGMM_SUSPEND_REQ__
#define __T_LLGMM_SUSPEND_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1774
 */
typedef struct
{
  U8                        susp_cause;               /*<  0:  1> suspension cause                                   */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_LLGMM_SUSPEND_REQ;
#endif

#ifndef __T_LLGMM_RESUME_REQ__
#define __T_LLGMM_RESUME_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1775
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_LLGMM_RESUME_REQ;
#endif

#ifndef __T_LLGMM_STATUS_IND__
#define __T_LLGMM_STATUS_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1776
 */
typedef struct
{
  U16                       error_cause;              /*<  0:  2> error cause                                        */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_LLGMM_STATUS_IND;
#endif

#ifndef __T_LLGMM_TLLI_IND__
#define __T_LLGMM_TLLI_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1777
 */
typedef struct
{
  U32                       new_tlli;                 /*<  0:  4> new temporary logical link identifier              */
} T_LLGMM_TLLI_IND;
#endif


#include "CDG_LEAVE.h"


#endif

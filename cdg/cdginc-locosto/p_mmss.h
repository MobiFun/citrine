/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_mmss.h                                                       |
| SOURCE  : "sap\mmss.pdf"                                                 |
| LastModified : "2002-07-19"                                              |
| IdAndVersion : "6147.105.97.102"                                         |
| SrcFileTime  : "Thu Nov 29 09:47:24 2007"                                |
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


#ifndef P_MMSS_H
#define P_MMSS_H


#define CDG_ENTER__P_MMSS_H

#define CDG_ENTER__FILENAME _P_MMSS_H
#define CDG_ENTER__P_MMSS_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_MMSS_H__LAST_MODIFIED _2002_07_19
#define CDG_ENTER__P_MMSS_H__ID_AND_VERSION _6147_105_97_102

#define CDG_ENTER__P_MMSS_H__SRC_FILE_TIME _Thu_Nov_29_09_47_24_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_MMSS_H

#undef CDG_ENTER__FILENAME


#include "p_mmss.val"


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_MMSS_ESTABLISH_REQ__
#define __T_MMSS_ESTABLISH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1912
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> transaction identifier                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMSS_ESTABLISH_REQ;
#endif

#ifndef __T_MMSS_RELEASE_REQ__
#define __T_MMSS_RELEASE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1913
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> transaction identifier                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMSS_RELEASE_REQ;
#endif

#ifndef __T_MMSS_DATA_REQ__
#define __T_MMSS_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1914
 */
typedef struct
{
  U8                        d1;                       /*<  0:  1> dummy, not used                                    */
  U8                        d2;                       /*<  1:  1> dummy, not used                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_MMSS_DATA_REQ;
#endif

#ifndef __T_MMSS_DATA_IND__
#define __T_MMSS_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1915
 */
typedef struct
{
  U8                        d1;                       /*<  0:  1> dummy, not used                                    */
  U8                        d2;                       /*<  1:  1> dummy, not used                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_MMSS_DATA_IND;
#endif

#ifndef __T_MMSS_ERROR_IND__
#define __T_MMSS_ERROR_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1916
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> transaction identifier                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       cause;                    /*<  2:  2> MM cause                                           */
} T_MMSS_ERROR_IND;
#endif

#ifndef __T_MMSS_ESTABLISH_CNF__
#define __T_MMSS_ESTABLISH_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1917
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> transaction identifier                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMSS_ESTABLISH_CNF;
#endif

#ifndef __T_MMSS_ESTABLISH_IND__
#define __T_MMSS_ESTABLISH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1918
 */
typedef struct
{
  U8                        d1;                       /*<  0:  1> dummy, not used                                    */
  U8                        d2;                       /*<  1:  1> dummy, not used                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_MMSS_ESTABLISH_IND;
#endif

#ifndef __T_MMSS_RELEASE_IND__
#define __T_MMSS_RELEASE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1919
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> transaction identifier                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       cause;                    /*<  2:  2> MM cause                                           */
} T_MMSS_RELEASE_IND;
#endif


#include "CDG_LEAVE.h"


#endif

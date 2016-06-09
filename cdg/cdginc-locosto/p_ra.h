/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_ra.h                                                         |
| SOURCE  : "sap\ra.pdf"                                                   |
| LastModified : "2002-04-26"                                              |
| IdAndVersion : "8411.102.98.204"                                         |
| SrcFileTime  : "Thu Nov 29 09:51:34 2007"                                |
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


#ifndef P_RA_H
#define P_RA_H


#define CDG_ENTER__P_RA_H

#define CDG_ENTER__FILENAME _P_RA_H
#define CDG_ENTER__P_RA_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_RA_H__LAST_MODIFIED _2002_04_26
#define CDG_ENTER__P_RA_H__ID_AND_VERSION _8411_102_98_204

#define CDG_ENTER__P_RA_H__SRC_FILE_TIME _Thu_Nov_29_09_51_34_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_RA_H

#undef CDG_ENTER__FILENAME


#include "p_ra.val"


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_RA_ACTIVATE_REQ__
#define __T_RA_ACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2399
 */
typedef struct
{
  U8                        model;                    /*<  0:  1> transfer protocol model                            */
  U8                        tra_rate;                 /*<  1:  1> transmission rate                                  */
  U8                        user_rate;                /*<  2:  1> user rate                                          */
  U8                        ndb;                      /*<  3:  1> number data bits                                   */
  U8                        nsb;                      /*<  4:  1> number stop bits                                   */
  U8                        _align0;                  /*<  5:  1> alignment                                          */
  U8                        _align1;                  /*<  6:  1> alignment                                          */
  U8                        _align2;                  /*<  7:  1> alignment                                          */
} T_RA_ACTIVATE_REQ;
#endif

#ifndef __T_RA_DEACTIVATE_REQ__
#define __T_RA_DEACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2400
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_RA_DEACTIVATE_REQ;
#endif

#ifndef __T_RA_READY_IND__
#define __T_RA_READY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2401
 */
typedef struct
{
  U8                        req_frames;               /*<  0:  1> requested frames                                   */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RA_READY_IND;
#endif

#ifndef __T_RA_DATA_REQ__
#define __T_RA_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2402
 */
typedef struct
{
  U8                        fr_type;                  /*<  0:  1> frame type                                         */
  U8                        dtx_flg;                  /*<  1:  1> discontinuous transmission flag                    */
  U8                        status;                   /*<  2:  1> V24 Status                                         */
  U8                        reserved;                 /*<  3:  1> Reserved                                           */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_RA_DATA_REQ;
#endif

#ifndef __T_RA_BREAK_REQ__
#define __T_RA_BREAK_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2403
 */
typedef struct
{
  U16                       break_len;                /*<  0:  2> Break Length                                       */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_RA_BREAK_REQ;
#endif

#ifndef __T_RA_DATA_IND__
#define __T_RA_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2404
 */
typedef struct
{
  U8                        fr_type;                  /*<  0:  1> frame type                                         */
  U8                        status;                   /*<  1:  1> V24 Status                                         */
  U8                        reserved;                 /*<  2:  1> Reserved                                           */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_RA_DATA_IND;
#endif

#ifndef __T_RA_ACTIVATE_CNF__
#define __T_RA_ACTIVATE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==2405
 */
typedef struct
{
  U8                        ack_flg;                  /*<  0:  1> acknowledge flag                                   */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RA_ACTIVATE_CNF;
#endif

#ifndef __T_RA_DEACTIVATE_CNF__
#define __T_RA_DEACTIVATE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==2406
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_RA_DEACTIVATE_CNF;
#endif

#ifndef __T_RA_BREAK_IND__
#define __T_RA_BREAK_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2407
 */
typedef struct
{
  U16                       break_len;                /*<  0:  2> Break Length                                       */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_RA_BREAK_IND;
#endif

#ifndef __T_RA_DETECT_REQ__
#define __T_RA_DETECT_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2408
 */
typedef struct
{
  U8                        detect;                   /*<  0:  1> Detect mode                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_RA_DETECT_REQ;
#endif

#ifndef __T_RA_MODIFY_REQ__
#define __T_RA_MODIFY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2409
 */
typedef struct
{
  U8                        tra_rate;                 /*<  0:  1> transmission rate                                  */
  U8                        user_rate;                /*<  1:  1> user rate                                          */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_RA_MODIFY_REQ;
#endif

#ifndef __T_RA_MODIFY_CNF__
#define __T_RA_MODIFY_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==2410
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_RA_MODIFY_CNF;
#endif

#ifndef __T_RA_DATATRANS_REQ__
#define __T_RA_DATATRANS_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2411
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_RA_DATATRANS_REQ;
#endif

#ifndef __T_SHM_TICK_REQ__
#define __T_SHM_TICK_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2412
 */
typedef struct
{
  U8                        tick_dir;                 /*<  0:  1> tick direction                                     */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_SHM_TICK_REQ;
#endif

#ifndef __T_SHM_DATA_REQ__
#define __T_SHM_DATA_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2413
 */
typedef struct
{
  U8                        fr_type;                  /*<  0:  1> frame type                                         */
  U8                        dtx_flg;                  /*<  1:  1> discontinuous transmission flag                    */
  U8                        status;                   /*<  2:  1> V24 Status                                         */
  U8                        reserved;                 /*<  3:  1> Reserved                                           */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_SHM_DATA_REQ;
#endif

#ifndef __T_SHM_READ_REQ__
#define __T_SHM_READ_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2414
 */
typedef struct
{
  U8                        model;                    /*<  0:  1> transfer protocol model                            */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_SHM_READ_REQ;
#endif

#ifndef __T_SHM_DATA_IND__
#define __T_SHM_DATA_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2415
 */
typedef struct
{
  U8                        fr_type;                  /*<  0:  1> frame type                                         */
  U8                        status;                   /*<  1:  1> V24 Status                                         */
  U8                        reserved;                 /*<  2:  1> Reserved                                           */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
  T_sdu                     sdu;                      /*<  4: ? > Service Data Unit                                  */
} T_SHM_DATA_IND;
#endif

#ifndef __T_SHM_BITSET_REQ__
#define __T_SHM_BITSET_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2416
 */
typedef struct
{
  U8                        id;                       /*<  0:  1> Bit Identity                                       */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       value;                    /*<  2:  2> Bits value                                         */
} T_SHM_BITSET_REQ;
#endif

#ifndef __T_SHM_BITTEST_REQ__
#define __T_SHM_BITTEST_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==2417
 */
typedef struct
{
  U8                        offset;                   /*<  0:  1> Address offset                                     */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       mask;                     /*<  2:  2> Bit mask                                           */
} T_SHM_BITTEST_REQ;
#endif

#ifndef __T_SHM_BITTEST_IND__
#define __T_SHM_BITTEST_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==2418
 */
typedef struct
{
  U16                       value;                    /*<  0:  2> Bits value                                         */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_SHM_BITTEST_IND;
#endif


#include "CDG_LEAVE.h"


#endif

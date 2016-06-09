/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UTIL
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Definitions for the PSA utility functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_UTIL_H
#define PSA_UTIL_H


/*==== CONSTANTS ==================================================*/

 
/*==== TYPES ======================================================*/


/*==== PROTOTYPES =================================================*/
EXTERN void   utl_BCD2String  (char *bcd_string, 
                               const UBYTE *bcd, 
                               USHORT len);
EXTERN USHORT utl_dialStr2BCD (const char *pDialStr,
                               UBYTE *pBCDBuf,
                               UBYTE  maxDigits);
EXTERN void   utl_BCD2DialStr (const UBYTE *pBCD,
                               char *pDSBuf, 
                               UBYTE numDigits);
/* PATCH Add bit_offset parameter to function cvt7To8 */
/*EXTERN UBYTE  utl_cvt7To8     ( UBYTE* source,
                                UBYTE  source_len,
                                UBYTE* dest );*/
EXTERN UBYTE  utl_cvt7To8     ( const UBYTE* source,
                                UBYTE  source_len,
                                UBYTE* dest,
                                UBYTE  bit_offset);
/* PATCH END */
EXTERN UBYTE utl_cvtPnn7To8 (const UBYTE* source, UBYTE source_len, UBYTE dcs, UBYTE* dest );

EXTERN UBYTE  utl_cvt8To7     ( const UBYTE* source,
                                UBYTE  source_len,
                                UBYTE* dest,
                                UBYTE  bit_offset);
/* Implements Measure 25 */

#ifdef FF_SAT_E
EXTERN T_ACI_CMD_SRC psa_search_SATSrcId (void);
#endif /* FF_SAT_E */

/*==== EXPORT =====================================================*/

 

#endif /* PSA_UTIL_H */

/*==== EOF =======================================================*/

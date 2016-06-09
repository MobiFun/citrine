/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  KSD_UTL
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
|  Purpose :  This module defines utility functions for the KSD
|             component of the protocol stack.
+----------------------------------------------------------------------------- 
*/ 

#ifndef KSD_UTL_H
#define KSD_UTL_H

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/

GLOBAL void utl_splitDialnumber (CHAR     *dial,
                                 CHAR    **main,
                                 BOOL     *international,
                                 CHAR    **sub,
                                 USHORT   *numCharsSub);

GLOBAL BOOL utl_string2UByte    (CHAR     *inSeq,
                                 USHORT    inLen,
                                 UBYTE    *outValue);

GLOBAL BOOL utl_string2Byte     (CHAR     *inSeq,
                                 USHORT    inLen,
                                 BYTE     *outValue);

GLOBAL BOOL utl_string2Short    (CHAR     *inSeq,
                                 USHORT    inLen,
                                 SHORT    *outValue);

GLOBAL BOOL utl_string2Long     (CHAR     *inSeq,
                                 USHORT    inLen,
                                 LONG     *outValue);

/*==== EXPORT =====================================================*/

#endif


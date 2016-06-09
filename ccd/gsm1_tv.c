/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : gsm1_tv.c
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
|  Purpose :  Definition of encoding and decoding functions for GSM1_TV elements 
+----------------------------------------------------------------------------- 
*/ 


/*
 * standard definitions like GLOBAL, UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"

/*
 * Prototypes of ccd (USE_DRIVER EQ undef) for prototypes only
 * look at ccdapi.h
 */
#undef USE_DRIVER

#include "ccdapi.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "ccd_globs.h"
#include "bitfun.h"

/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm1tv_decode   |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type TV element. This element
            consists of a 4 Bit T component and a V component with
            max. 4 Bit length.
*/

SHORT cdc_gsm1tv_decode (const ULONG c_ref, const ULONG  e_ref, T_CCD_Globs *globs)
{
  T_TLV_SORT tlv_inf;
  
  tlv_inf.gotTag = TRUE;
  tlv_inf.gotLen = FALSE;
  
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm1tv_decode()");
  #else
  TRACE_CCD (globs, "cdc_gsm1tv_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  return cdc_tlv_decode (c_ref, e_ref, &tlv_inf, globs);
}
#endif /* !RUN_INT_RAM */


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm1tv_encode   |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type TV element. This element
            consists of a 4 Bit T component and a V component with
            max. 4 Bit length.
*/

SHORT cdc_gsm1tv_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm1tv_encode()");
  #else
  TRACE_CCD (globs, "cdc_gsm1tv_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif
  
  cdc_tlv_encode (e_ref, 4, 0, globs);  

  return 1;
}
#endif /* !RUN_INT_RAM */

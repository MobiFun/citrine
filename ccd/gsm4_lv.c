/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : gsm4_lv.c
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
|  Purpose :  Definition of encoding and decoding functions for GSM4_LV elements 
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
| STATE   : code                       ROUTINE : cdc_gsm4lv_decode   |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type 4LV element. This element
            consists of a 8 Bit L component and a V component wich
            length depends on the value of the L component.

*/

SHORT cdc_gsm4lv_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  T_TLV_SORT tlv_inf;
  
  tlv_inf.gotTag = FALSE;
  tlv_inf.gotLen = TRUE;
  
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm4lv_decode()");
  #else
  TRACE_CCD (globs, "cdc_gsm4lv_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  return cdc_tlv_decode (c_ref, e_ref, &tlv_inf, globs);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm4lv_encode   |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type 4LV element. This element
            consists of a 8 Bit L component and a V component wich
            length depends on the value of the L component.

*/

SHORT cdc_gsm4lv_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm4lv_encode()");
  #else
  TRACE_CCD (globs, "cdc_gsm4lv_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cdc_tlv_encode (e_ref, 0, 8, globs);

  return 1;
}
#endif /* !RUN_INT_RAM */

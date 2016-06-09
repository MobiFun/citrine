/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : csn1_sh.c
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
|  Purpose :  Definition of encoding and decoding functions for CSN1_SH elements 
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


extern SHORT cdc_csn1_sx_decode (int flag, const ULONG e_ref, T_CCD_Globs *globs);
extern SHORT cdc_csn1_sx_encode (int flag, const ULONG e_ref, T_CCD_Globs *globs);

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CDC_GSM               |
| STATE   : code                     ROUTINE : cdc_csn1_shl_decode   |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type CSN1 SHL element. This element
            consists of a 1 bit valid flag and a value part.
            Only if the valid bit is equal H the value part follows.

*/

SHORT cdc_csn1_shl_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_shl_decode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_shl_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));  
	#endif
#endif

  return cdc_csn1_sx_decode (0xFF, e_ref, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_csn1_shl_encode    |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type CSN1 SHL element. This element
            consists of a 1 bit valid flag and a value part. If the
            element is valid (the v_xxx components is TRUE in the
            decoded message) a H bit will be coded followed by the
            coding of the value part. Otherwise a L bit will be
            coded.

*/

SHORT cdc_csn1_shl_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_shl_encode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_shl_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));  
	#endif
#endif

  return cdc_csn1_sx_encode (0xFF, e_ref, globs);
}
#endif /* !RUN_FLASH */

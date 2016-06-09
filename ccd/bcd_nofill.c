/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : bcd_nofill.c
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
|  Purpose :  Definition of encoding and decoding functions for BCD_NOFILL elements 
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
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_bcd_nofill_decode |
+--------------------------------------------------------------------+

  PURPOSE :

*/


SHORT cdc_bcd_nofill_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_bcd_nofill_decode()");
  #else
  TRACE_CCD (globs, "cdc_bcd_nofill_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif
  cdc_BCD_decode (e_ref, 2, globs); 
  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_bcd_nofill_encode |
+--------------------------------------------------------------------+

  PURPOSE : encoding a Bytearray, that contain a BCD Number, into
            the bitstream. without the filling nibble 
*/


SHORT cdc_bcd_nofill_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_bcd_nofill_encode()");
  #else
  TRACE_CCD (globs, "cdc_bcd_nofill_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif
  cdc_BCD_encode (e_ref, 2, globs); 
  return 1;
}
#endif /* !RUN_INT_RAM */

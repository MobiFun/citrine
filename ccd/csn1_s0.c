/* 
+----------------------------------------------------------------------------- 
|  Project : CCD
|  Modul   : csn1_s0.c
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
|  Purpose :  Definition of encoding and decoding functions for CSN1_S0 elements
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
| PROJECT : CCD (6144)               MODULE  : CCD                   |
| STATE   : code                     ROUTINE : cdc_csn1_s0_decode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the non-standard type CSN1 S0 element.
            The encoding rule for this type is very similar to the 
            CSN.1 type. Except for the presence flag which is 0 in 
            this case. 
            The encoded IE consists of one bit for presence flag and 
            a value part. Only if the flag bit is equal 0 the value 
            part will follow it.
*/

SHORT cdc_csn1_s0_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_s0_decode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_s0_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  return cdc_csn1_sx_decode (0, e_ref, globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_csn1_s0_encode     |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the non-standard type CSN1 S0 element.
            The encoding rule for this type is very similar to CSN.1
            type. Except for the presence flag which is 0 in this
            case. The encoded bit pattern consists of a 0 bit as 
            valid flag and a value part. If the element is present
            (the v_xxx component is TRUE in the decoded message)
            a 0 bit will be coded followed by the encoded value part.
            Otherwise a 1 bit will be encoded.
            Possible array types are given by melem[e_ref].repType:
            0) ' ' =>  {0| IE}        (no array)
            1) 'i' =>  {0 IE} ** 1    (many repeations up to a 1 bit)
            2) 'v' =>  {0|1 IE}* val  (val is another IE)
            3) 'c' =>  {0|1 IE}* 3    (3 is  an example)
*/

SHORT cdc_csn1_s0_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_s0_encode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_s0_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  return cdc_csn1_sx_encode (0, e_ref, globs);
}
#endif /* !RUN_FLASH */

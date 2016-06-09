/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  asn1_opentype.c     
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
|  Purpose :  Encoding and decoding functions for ASN1_OPEN_TYPE type
+----------------------------------------------------------------------------- 
*/ 

/*
 * Standard definitions like UCHAR, ERROR etc.
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
 * Prototypes and constants in the common part of ccd
 */
#include "ccd.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_opentype              |
| STATE   : code                    ROUTINE : cdc_asn1_open_type_decode  |
+------------------------------------------------------------------------+

  PURPOSE : Decode PER OBJECT IDENTIFIER type 

            The PER encoding of an open type is preceded by a length 
            determinant (1 or 2 bytes as long as no data fragmentation is
            necessary). The bit field of the encoded open type itself is  
            octet aligned octet string.
            If the result of encoding the outermost value is an empty bit 
            string, the bit string must be replaced with a single octet 
            with all bits set to 0.
*/
SHORT cdc_asn1_open_type_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8    BER_Len;

#ifdef DEBUG_CCD
#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_open_type_decode()");
#else
  TRACE_CCD (globs, "cdc_asn1_open_type_decode() %s", mcomp[melem[e_ref].elemRef].name);
#endif
#endif

  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the 
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) ) {
    return 1;
  }

  /*
   * For unknown extension read the length and skip over the octets.
   */
  BER_Len = (U8) Read_OpenTpye_Length (globs);
  bf_incBitpos (8 * BER_Len, globs);

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_opentype              |
| STATE   : code                    ROUTINE : cdc_asn1_open_type_encode  |
+------------------------------------------------------------------------+

  PURPOSE : PER encoding of open types 

            The PER encoding of an open type is preceded by a length 
            determinant (1 or 2 bytes as long as no data fragmentation is
            necessary). The bit field of the encoded open type itself is  
            octet aligned octet string.
            If the result of encoding the outermost value is an empty bit 
            string, the bit string must be replaced with a single octet 
            with all bits set to 0.            
*/
SHORT cdc_asn1_open_type_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_open_type_encode()");
#else
  TRACE_CCD (globs, "cdc_asn1_open_type_encode() %s", mcomp[melem[e_ref].elemRef].name);
#endif
#endif

  /* For optional elements we have already set the valid flag in the
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) )
    return 1;
#ifdef DEBUG_CCD
  else
  {
    TRACE_CCD (globs, "Please set valid flag to 0 for open type element.");
  }
#endif
  return 1;
}
#endif /* !RUN_INT_RAM */

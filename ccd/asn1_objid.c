
/*
+------------------------------------------------------------------------------
|  File:       asn1_objid.c
+------------------------------------------------------------------------------
|                 Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+------------------------------------------------------------------------------
| Purpose:    Encoding and decoding functions for ASN1_OBJ_ID type 
|
| $Identity:$
+------------------------------------------------------------------------------
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
| PROJECT : CCD (6144)              MODULE  : asn1_objid                 |
| STATE   : code                    ROUTINE : cdc_asn1_obj_id_decode     |
+------------------------------------------------------------------------+

  PURPOSE : Decode PER OBJECT IDENTIFIER type 

            The PER encoding of an object identifier type uses the 
            content octets of BER preceded by a length determinant
            that will in practice be a single octet.
*/
SHORT cdc_asn1_obj_id_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8    BER_Len;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
    TRACE_CCD (globs, "cdc_asn1_obj_id_decode()");
  #else
    TRACE_CCD (globs, "cdc_asn1_obj_id_decode() %s",
		mcomp[melem[e_ref].elemRef].name);
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
   * Skip over PER length octet and BER tag octet.
   */
  bf_incBitpos (16, globs);

  /*
   * For unknown extension read the length and skip over the octets.
   */
  BER_Len = (U8) bf_getBits (8, globs);
  *(U8 *)(globs->pstruct + globs->pstructOffs++) = BER_Len;

  /*
   * Copy the content of object identifier bytes into the C-Structure.
   */
#if 1 //#ifdef _TMS470
  bf_incBitpos (8 * BER_Len, globs);
#else
  {
    int i;
    for (i=0; i < BER_Len; i++)
      *(U8 *)(globs->pstruct + globs->pstructOffs++) = (U8) bf_getBits (8, globs);
  }
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_objid                 |
| STATE   : code                    ROUTINE : cdc_asn1_obj_id_encode     |
+------------------------------------------------------------------------+

  PURPOSE : Encode PER OBJECT IDENTIFIER type

            The PER encoding of an object identifier type uses the 
            content octets of BER preceded by a length determinant
            that will in practice be a single octet.            
*/
SHORT cdc_asn1_obj_id_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
    TRACE_CCD (globs, "cdc_asn1_obj_id_encode()");
  #else
    TRACE_CCD (globs, "cdc_asn1_obj_id_encode() %s",
		mcomp[melem[e_ref].elemRef].name);
  #endif
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

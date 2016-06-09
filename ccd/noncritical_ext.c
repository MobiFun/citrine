
/*
+------------------------------------------------------------------------------
|  File:       noncritical_ext.c
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
| Purpose:    Encoding and decoding functions for nonCriticalExtensions elements 
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
| PROJECT : CCD (6144)              MODULE  : noncritical_ext            |
| STATE   : code                    ROUTINE : cdc_noncritical_ext_decode |
+------------------------------------------------------------------------+

  PURPOSE : Decode elements of type nonCriticalExtensions

            An element of this type should never be encoded or decoded.
            If the coder faces an IE of this type, it should report a
            warning and continue decoding.
            Whenever a nonCriticalExtensions-IE is extended from NULL or 
            SEQUENCE {} to another type, its CCD coding type will change
            and the functions in this file will not be called for that IE
            any more.

            According to 3GPP TS25.331 V3.6.0, 10.1.1.1:
            Information elements applicable to choices reserved for future
            releases of the protocol shall be added to the end of the message.
            In future releases non critical informaion elements ... shall 
            be appended at the end of the message.
            See also 3GPP TS25.921 V4.3.0, 10.4.

*/
SHORT cdc_noncritical_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_noncritical_ext_decode()");
#else
  TRACE_CCD (globs, "cdc_noncritical_ext_decode() %s", mcomp[melem[e_ref].elemRef].name);
#endif
#endif

  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the 
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) ) {
    return 1;
  }

  if (melem[e_ref].elemType EQ 'V' AND mvar[melem[e_ref].elemRef].bSize EQ 0)
  {
    return 0;
  }
  /* 
   * Currently CCD tool chain does not support extensions of this type.
   * This else-implementation is just an outlook.
   * 
  else
  {
    U16 compRef = melem[e_ref].elemRef;
    U16 elemRef = mcomp[compRef].componentRef;
    (void) codec[melem[Elem].codingType][DECODE_FUN]
                                            (compRef, elemRef, globs);
  }*/
  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : noncritical_ext            |
| STATE   : code                    ROUTINE : cdc_noncritical_ext_encode |
+------------------------------------------------------------------------+

  PURPOSE : Encode elements of type nonCriticalExtensions

            An element of this type should never be encoded or decoded.
            If the coder faces an IE of this type, it should report an
            error report and abort.             
*/
SHORT cdc_noncritical_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_noncritical_ext_encode()");
#else
  TRACE_CCD (globs, "cdc_noncritical_ext_encode() %s", mcomp[melem[e_ref].elemRef].name);
#endif
#endif

  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the 
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) ) {
    return 1;
  }

  if (melem[e_ref].elemType EQ 'V' AND mvar[melem[e_ref].elemRef].bSize EQ 0)
  {
    return 0;
  }
  /* 
   * Currently CCD tool chain does not support extensions of this type.
   * This else-implementation is just an outlook.
   * 
  else
  {
    U16 compRef = melem[e_ref].elemRef;
    U16 elemRef = mcomp[compRef].componentRef;
    (void) codec[melem[Elem].codingType][ENCODE_FUN]
                                            (compRef, elemRef, globs);
  }*/
  return 1;
}
#endif /* !RUN_INT_RAM */

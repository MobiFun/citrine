/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_octet.c
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
|  Purpose :  Definition of encoding and decoding functions for ASN1_OCTET
|             elements 
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
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_asn1_octet_decode  |
+--------------------------------------------------------------------+

  PURPOSE : UNALIGNED PER decoding for the octet string type (UMTS)
            The coded octets are preceded by a length indicator, if
            they are not of fixed length. The length indicator is 
            decoded as an ASN1_INTEGER type. 
*/
SHORT cdc_asn1_octet_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat;
  U8     *old_pstruct = NULL;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_octet_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_octet_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  /* 
   * Set pstrcutOffs and maxRep. 
   * Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &repeat, globs) NEQ ccdOK)
    return 1;

#ifdef DYNAMIC_ARRAYS
  /*
   * Check for pointer types, and allocate memory if necessary.
   * May overwrite globs->pstruct (and initialize globs->pstructOffs to 0).
   */
  if ( is_pointer_type(e_ref) ) 
  {
    old_pstruct = globs->pstruct;
    if ( PER_allocmem_and_update(e_ref, repeat, globs) NEQ ccdOK)
      /* No memory - Return.  Error already set in function call above. */
      return 1;
  }
#endif

  bf_readBitStr_PER ((USHORT)(repeat << 3), globs);

#ifdef DYNAMIC_ARRAYS
  if (old_pstruct NEQ NULL)
    globs->pstruct = old_pstruct;
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_asn1_octet_encode  |
+--------------------------------------------------------------------+

  PURPOSE : UNALIGNED PER encoding for the octet string type (UMTS)
            The coded octets are preceded by a length indicator, if
            they are not of fixed length. The length indicator is 
            encoded as an ASN1_INTEGER type. 
*/
SHORT cdc_asn1_octet_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat;
  U8     *old_pstruct = NULL;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_octet_encode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_octet_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  /* 
   * Set pstrcutOffs and maxRep. Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &repeat, globs) NEQ ccdOK)
    return 1;

#ifdef DYNAMIC_ARRAYS
  if ( is_pointer_type(e_ref) ) {
    old_pstruct = globs->pstruct;
    globs->pstruct = *(U8 **)(globs->pstruct + globs->pstructOffs);

    if (ccd_check_pointer(globs->pstruct) == ccdOK)
    {
      globs->pstructOffs = 0;
    }
    else
    {
      ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                       &globs->pstruct[globs->pstructOffs]);
      return 1;
    }
  }
#endif

  bf_writeBitStr_PER ((USHORT)(repeat << 3), globs);

#ifdef DYNAMIC_ARRAYS
  if ( old_pstruct NEQ NULL )
    globs->pstruct = old_pstruct;
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

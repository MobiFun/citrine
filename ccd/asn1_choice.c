/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_choice.c
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
|  Purpose :  Definition of encoding and decoding functions for ASN1_CHOICE
|             elements 
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
#include "ccd_codingtypes.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"

EXTERN T_FUNC_POINTER codec[MAX_CODEC_ID+1][2];

#ifndef RUN_INT_RAM
const UBYTE bitSize[] = {0, 1, 1, 2, 2, 3, 3, 3, 3,
            4, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)  MODULE  : asn1_choice                        |
| STATE   : code        ROUTINE : PER_Decode_ASN1_CHOICE_altervative |
+--------------------------------------------------------------------+

  PURPOSE : Decode a chosen alternative of an ASN.1 CHOICE type
            Use the parameter union tag bit-size to read the CHOICE
            index. Because of the ascending enumeration of union tags
            it is easy to calculate the e_ref for the chosen element 
            which is to be decoded. Then the appropriate decoding
            function for the chosen element is called.

            No actions for pointer types (dynamic arrays), as CHOICE
            clauses contain no information in themselves. Memory is
            allocated in the containing SEQUENCE instead.
*/
void PER_Decode_ASN1_CHOICE_alterative (const ULONG e_ref, T_ENUM UnionTag, T_CCD_Globs *globs)
{
  UBYTE *old_pstruct;
  ULONG elem_ref, c_ref;
  
  c_ref = (ULONG) melem[e_ref].elemRef;

  /*
   * Calculate the elem_ref for the chosen element.
   * Write the CHOICE tag value in the C-structure.
   */
  elem_ref =  mcomp[c_ref].componentRef + (ULONG) UnionTag;
  *(T_ENUM *) (globs->pstruct+globs->pstructOffs) = (T_ENUM) UnionTag;

  /*
   * Prepare for decoding next element.
   * Store the current value of the C-structure pointer. After decoding the CHOICE component 
   * we will set the pointer to this stored value. Then we will use pstructOffs for pointing
   * to the next element.
   */
  old_pstruct = globs->pstruct;
  globs->pstruct += (globs->pstructOffs + sizeof(T_ENUM));

#ifdef DYNAMIC_ARRAYS
  /*
   * Allocate memory for this whole composition, if elemType is
   * one of the pointer types.
   */
  if ( is_pointer_type(e_ref) AND
       PER_allocmem_and_update(e_ref, 1, globs) NEQ ccdOK ) {
    return;
  } else
#endif

  /* 
   * Call the decode function for the chosen element.
   */
  (void) codec[melem[elem_ref].codingType][DECODE_FUN]
                                        (c_ref, elem_ref, globs);
  /*
   * Prepare for decoding the next element.
   */
  globs->pstruct = old_pstruct;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_choice            |
| STATE   : code                    ROUTINE : cdc_asn1_choice_decode |
+--------------------------------------------------------------------+

  PURPOSE : Decode ASN.1 CHOICE type according to PER
            CHOICE index is read from the message bit string. Its  
            bit-size can be extracted from the number of CHOICE
            alternatives. 
            For decoding the CHOICE alternative itself, call the same 
            function as for extensible CHOICE types.
*/
SHORT cdc_asn1_choice_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  T_ENUM UnionTag=0; /* default: only one alternative */
  ULONG  num_of_comps, mcomp_ref;
  
  mcomp_ref = (ULONG) melem[e_ref].elemRef;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_choice_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_choice_decode() %s", mcomp[mcomp_ref].name);
	#endif
#endif

  /* Don't do anything for empty choices */
  if (mcomp[mcomp_ref].numOfComponents == 0)
  {
    return 1;
  }

  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the 
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) ) {
    return 1;
  }

  /* 
   * Get the bit size of the CHOICE index.
   * Then read the bits representing the index in the air message.
   */
  num_of_comps = (ULONG) mcomp[mcomp_ref].numOfComponents;
  /* CHOICE index is encoded only if there are more than one alternatives. */
  if (num_of_comps NEQ 1)
  {
    UnionTag = bf_getBits ((U32)bitSize[num_of_comps], globs);
    /*
     * Check correctness of the read CHOICE index.
     */ 
    if ((ULONG)UnionTag >= num_of_comps)
    {
      ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK, (USHORT) e_ref, globs->pstruct+globs->pstructOffs);
    }
  }
  PER_Decode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);
  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)  MODULE  : asn1_choice                        |
| STATE   : code        ROUTINE : PER_Encode_ASN1_CHOICE_altervative |
+--------------------------------------------------------------------+

  PURPOSE : Encode a chosen alternative of an ASN.1 CHOICE type.

            Because of ascending enumeration of union tags it is
            easy to calculate the e_ref for the chosen element to be
						encoded. Then the appropriate encoding function for the 
            chosen element is called.

            No actions for pointer types (dynamic arrays), as CHOICE
            clauses contain no information in themselves. Memory is
            allocated in the containing SEQUENCE instead.
*/
void  PER_Encode_ASN1_CHOICE_alterative (const ULONG e_ref, T_ENUM UnionTag, T_CCD_Globs *globs)
{
  UBYTE  *old_pstruct;
  ULONG   elem_ref, c_ref;
  
  c_ref = (ULONG) melem[e_ref].elemRef;

  /*
   * Prepare for encoding next element.
   * Store the current value of the C-structure pointer. After encoding
   * the CHOICE component we will set the pointer to this stored value.
   * Then we will use pstructOffs for pointing to the next element.
   */
  old_pstruct = globs->pstruct;
  globs->pstruct += (globs->pstructOffs + sizeof(T_ENUM));

#ifdef DYNAMIC_ARRAYS
  /*
   * Dereference pointer if this is a pointer types (dynamic arrays)
   * globs->pstruct was saved above, so no need to do it again.
   */
  if ( is_pointer_type(e_ref) ) {
    if (ccd_check_pointer(*(U8 **)(globs->pstruct)) == ccdOK)
      globs->pstruct = *(U8 **)(globs->pstruct);
    else {
      ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                         &globs->pstruct[globs->pstructOffs]);
      return;
    }
  }
#endif

  elem_ref = mcomp[c_ref].componentRef + (USHORT) UnionTag;

  /* 
   * Call the decode function for the chosen element.
   */
  (void) codec[melem[elem_ref].codingType][ENCODE_FUN]
                                        (c_ref, elem_ref, globs);
  /*
   * Prepare for encoding the next element.
   */
  globs->pstruct = old_pstruct;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_choice            |
| STATE   : code                    ROUTINE : cdc_asn1_choice_encode |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of CHOICE type for UMTS
            The size of the union tag must be calculated. Then its value
            is written in the air message. 
						For encoding the CHOICE alternative itself, call the same 
            function as for extensible CHOICE types. 
*/
SHORT cdc_asn1_choice_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  T_ENUM UnionTag=0; /* default: only one alternative */
	ULONG  num_of_comps, mcomp_ref;
	
	mcomp_ref = (ULONG) melem[e_ref].elemRef;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_choice_encode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_choice_encode() %s", mcomp[mcomp_ref].name);
	#endif
#endif

  /* Don't do anything for empty choices */
  if (mcomp[mcomp_ref].numOfComponents == 0)
  {
    return 1;
  }

  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the
   * C-structure. We have done it while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) )
    return 1;

  /*
   * Get the value of CHOICE index (= union controller) from the C-structure.
   * Check its correctness. Write it in the air message.
	 * Afterwards encode the chosen CHOICE altervative.
   */
	num_of_comps = mcomp[mcomp_ref].numOfComponents;
  /* CHOICE index is encoded only if there are more than one alternatives. */
  if (num_of_comps NEQ 1)
  {
    UnionTag = *(T_ENUM *) (globs->pstruct+globs->pstructOffs);
    if (UnionTag >= (T_ENUM)num_of_comps)
    {
      ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK, (USHORT) e_ref, 
                       globs->pstruct+globs->pstructOffs);
    }
    bf_writeVal ((ULONG) UnionTag, (ULONG) bitSize[num_of_comps], globs);
  }
  PER_Encode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);

  return 1;
}
#endif /* !RUN_INT_RAM */

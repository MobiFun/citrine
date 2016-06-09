/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_integ.c
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
|  Purpose :  Definition of encoding and decoding functions for ASN1_INTEGER
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
| PROJECT : CCD (6144)              MODULE  : asn1_integ             |
| STATE   : code                    ROUTINE : Read_unique_Integer    |
+--------------------------------------------------------------------+

  PURPOSE : Decode integer with only one possible value.
            Such a value is never encoded.
*/
void Read_unique_Integer (const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  varRef, valRef;
  U8    *value;

  varRef = (ULONG) melem[e_ref].elemRef;
  valRef = (ULONG) mvar[varRef].valueDefs;
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "Read_unique_Integer()");
	#else
	TRACE_CCD (globs, "Read_unique_Integer() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  if (mval[valRef].startValue EQ 0)
  {
    /* 
     * Do not do anything for empty sequences and NULL elements.
     * (Hint: Integers with only one possible value equal to 0 are  
     * automatically processed through memory reset of the C-structure 
     * at the beginning of decode activities or after each dynamic
     * memory allocation.)
     */
    return;
  }
  else 
  {
    /* 
     * For optional elements we have already set the valid flag in the 
     * C-structure while processing ASN1_SEQ.
     */
    if (melem[e_ref].optional)
    {
      if (globs->pstruct[globs->pstructOffs++] EQ FALSE)
        return;
    }

#ifdef DYNAMIC_ARRAYS
    if ( is_pointer_type(e_ref) )
    {
	    value = PER_allocmem(e_ref, 1, globs);

	    if (value EQ (U8 *)ccdError)
	      return;

	    /* 
       * Store pointer to allocated memory in c structure.
       */
	    *(U8 **)(globs->pstruct + globs->pstructOffs) = value;
    }
    else
#endif
	    value = globs->pstruct + globs->pstructOffs;

    switch (mvar[varRef].cType)
    {
      case 'B':
                *(U8*)  value = (U8)   mval[valRef].startValue;
                break;
      case 'C':
                *(S8*)  value = (S8)   mval[valRef].startValue;
                break;
      case 'S':
                *(U16*) value = (U16)  mval[valRef].startValue;
                break;
      case 'T':
                *(S16*) value = (S16)  mval[valRef].startValue;
                break;
      case 'L':
                *(U32*) value = (U32)  mval[valRef].startValue;
                break;
      case 'M':
                *(S32*) value = (S32)  mval[valRef].startValue;
                break;
      default:
                ccd_recordFault (globs,ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, value);
                break;
    }
    return;
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_integ             |
| STATE   : code                    ROUTINE : cdc_asn1_integ_decode  |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the PER integer type for UMTS
            PER-visible constraints restrict the integer value to be a
            constrained whole number. This gives a lower and an upper 
            bound for the integer. The lb is also called offset. The 
            encoded value is the difference between the actual and the
            offset value.
            A possible and meant default value is never encoded.
*/
SHORT cdc_asn1_integ_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat=1, maxRep=1, varRef, valRef;
  BOOL    DefaultFound= FALSE;
  S32     IfnotPresent;
  UBYTE   *value, *old_pstruct = NULL;

  varRef = (ULONG) melem[e_ref].elemRef;
 
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_integ_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_integ_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  /*
   * Set the offset in the C-structure on the value for this element.
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  /* 
   * Decode an empty sequence, a NULL element or an integer of constant value.
   */
  if (mvar[varRef].bSize EQ 0)
  {
    Read_unique_Integer (e_ref, globs);
    return 1;
  }
  valRef = (ULONG) mvar[varRef].valueDefs;

  /* 
   * Set pstrcutOffs and maxRep. Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &maxRep, globs) NEQ ccdOK)
    return 1;

#ifdef DYNAMIC_ARRAYS
  /*
   * Allocate memory if this is a pointer type (dynamic array)
   */
  if ( is_pointer_type(e_ref) ) {
    old_pstruct = globs->pstruct;
    if ( PER_allocmem_and_update(e_ref, maxRep, globs) NEQ ccdOK)
      /* No memory - Return.  Error already set in function call above. */
      return 1;
  }
#endif

  /* 
   * Check if there is a default value for the element. 
   * If yes, just set it aside for a later comparision.  
   */
  if (mval[valRef+1].isDefault EQ 2)
  {
    IfnotPresent = mval[valRef+1].startValue;
    DefaultFound = TRUE;
  }

  /*
   * Decode all elements of the array.
   */
  while ( repeat <= maxRep)
  {

    value = globs->pstruct + globs->pstructOffs;

    /* 
     * There is a default value for this integer elment. 
     * While decoding of the ASN1-SEQUENCE contiaing this integer
     * we have used a particular byte of C-structure to signalize 
     * the decoding of a default value (byte set to 0).
     */
    if (DefaultFound AND !globs->pstruct[melem[e_ref].structOffs])
    {
      switch (mvar[varRef].cType)
      {
        case 'B':
                  *(U8*)  value = (U8)   IfnotPresent;
                  break;
        case 'C':
                  *(S8*)  value = (S8)   IfnotPresent;
                  break;
        case 'S':
                  *(U16*) value = (U16)  IfnotPresent;
                  break;
        case 'T':
                  *(S16*) value = (S16)  IfnotPresent;
                  break;
        case 'L':
                  *(U32*) value = (U32)  IfnotPresent;
                  break;
        case 'M':
                  *(S32*) value = (S32)  IfnotPresent;
                  break;
        default:
                  ccd_recordFault (globs,ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, value);
                  return 1;
      }
    }
    /* 
     * There is no default value defined for this integer elment.
     * Read the value from the bit buffer.
     */
    else
    {
      U32 ub, lb;
      ULONG readBits;
      U32 DecodedValue;

      lb = mval[valRef].startValue;
      ub = mval[valRef].endValue;

      /*
       * Read the non-negative value from the air message.
       */
      readBits = bf_getBits (mvar[varRef].bSize, globs);
      

      if (readBits <= (U32)(ub - lb))
      {
        DecodedValue = lb + readBits;
        /* 
         * Add the offset to the read value to get the actual one.

         */
        switch (mvar[varRef].cType)
        {
          case 'B':
                    *(U8*)  value = (U8) DecodedValue;
                    break;
          case 'C':
                    *(S8*)  value = (S8) DecodedValue;
                    break;
          case 'S':
                    *(U16*) value = (U16) DecodedValue;
                    break;
          case 'T':
                    *(S16*) value = (S16) DecodedValue;
                    break;
          case 'L':
                    *(U32*) value = (U32) DecodedValue;
                    break;
          case 'M':
                    *(S32*) value = (S32) DecodedValue;
                    break;
          default:
                    ccd_recordFault (globs,ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, value);
                    break;
        } 
      }
      else
      {
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "integer out of range! %ld require: %ld .. %ld ", DecodedValue, lb, ub);
#endif
        if (melem[e_ref].optional)
          ccd_recordFault (globs, ERR_ASN1_OPT_IE, CONTINUE, (USHORT) e_ref, value);
        else
          ccd_recordFault (globs, ERR_ASN1_MAND_IE, CONTINUE, (USHORT) e_ref, value);
      }
    }
    repeat ++;
    globs->pstructOffs += mvar[varRef].cSize; 
  }/*while*/

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
| PROJECT : CCD (6144)              MODULE  : asn1_integ             |
| STATE   : code                    ROUTINE : cdc_asn1_integ_encode  |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the PER integer type for UMTS
            PER-visible constraints restrict the integer value to be a
            constrained whole number. This gives a lower and an upper 
            bound for the integer. The lb is also called offset. The 
            encoded value is the difference between the actual and the
            offset value. Hence encoded values are non-negative.
            A possible and meant default value is never encoded. 
*/
SHORT cdc_asn1_integ_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat=1, maxRep=1, varRef, valRef;
  BOOL    DefaultFound= FALSE;
  S32     IfnotPresent;
  U8     *base_pstruct;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_integ_encode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_integ_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  varRef = (ULONG) melem[e_ref].elemRef;
  valRef = (ULONG) mvar[varRef].valueDefs;

  /* 
   * Don't do anything for empty sequences, NULL elements or integers of constant value.
   */
  if (mvar[varRef].bSize EQ 0)
    return 1;

  /* 
   * Set pstrcutOffs and maxRep. Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &maxRep, globs) NEQ ccdOK)
    return 1;

  /* 
   * Check if there is a default value for the element. 
   * If yes, just set it aside for a later comparision.  
   */
  if (mval[valRef+1].isDefault EQ 2)
  {
    IfnotPresent = mval[valRef+1].startValue;
    DefaultFound = TRUE;
  }

#ifdef DYNAMIC_ARRAYS
  if ( is_pointer_type(e_ref) )
  {
    base_pstruct = *(U8 **)(globs->pstruct + globs->pstructOffs);
    if (ccd_check_pointer(base_pstruct) == ccdOK)
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
  else
#endif
    base_pstruct = globs->pstruct;

  /*
   * Encode all elements of the array.
   */
  while ( repeat <= maxRep)
  {
    S32 ub, lb, value;
    UBYTE   *p;

    /*
     * setup the read pointer to the element in the C-structure
     */
    p = base_pstruct + globs->pstructOffs;

    switch (mvar[varRef].cType)
    {
    case 'B':
      value = (S32)*(UBYTE *) p;
      break;
    case 'C':
      value = (S32)*(S8 *) p;
      break;
    case 'S':
      value = (S32)*(USHORT *) p;
      break;
    case 'T':
      value = (S32)*(S16 *) p;
      break;
    case 'L':
      /* 
       * This type casting can be critical.
       * Thus the case of bSize=32 will be handled separately.
       */
      if (mvar[varRef].bSize < 32)
      {
        value = (S32)*(U32 *) p;
      }
      break;
    case 'M':
      value = *(S32 *) p;
      break;
    default:
      ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, p);
      return 1;
    }

    if (mvar[varRef].cType EQ 'L' AND  
           (mvar[varRef].bSize EQ 32))
    {
        ULONG CriticalValue;
        U32 lb, ub;
        CriticalValue = *(U32 *) p;
        if (!DefaultFound OR (U32)IfnotPresent NEQ CriticalValue)
        {
          lb = (U32) mval[valRef].startValue;
          ub = (U32) mval[valRef].endValue;
          if (lb <= CriticalValue && CriticalValue <= ub)
          {
            bf_writeVal (CriticalValue - lb, mvar[varRef].bSize, globs);
          }
        }
        else
        {
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "integer out of range! %ld require: %ld .. %ld ",
                             value, lb, ub);
#endif
          ccd_recordFault (globs, ERR_INT_VALUE, CONTINUE, (USHORT) e_ref, p);
        }
    }
    else 
    {
      /* 
       * Encode only non-default values.
       */
      if (!DefaultFound OR IfnotPresent NEQ value)
      {
        /* 
         * A non-negative-binary-integer will be encoded since the offset must
         * be subtracted from the value read from the C-structure.
         */
        lb = mval[valRef].startValue;
        ub = mval[valRef].endValue;
        if (lb <= value AND value <= ub)
        {
          bf_writeVal ((ULONG)(value - lb), mvar[varRef].bSize, globs);
        }
        else
        {
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "integer out of range! %ld require: %ld .. %ld ", value, lb, ub);
#endif
          ccd_recordFault (globs, ERR_INT_VALUE, CONTINUE, (USHORT) e_ref, p);
        }
      }
    }
    repeat ++; 
    globs->pstructOffs += mvar[varRef].cSize;
  }/* while-loop */

  return 1;
}
#endif /* !RUN_INT_RAM */

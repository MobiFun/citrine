/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_integ_ext.c
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
|  Purpose :  Encoding and decoding functions for ASN1_INTEGER_EXTENSIBLE type
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
| PROJECT : CCD (6144)              MODULE  : asn1_integ_ext             |
| STATE   : code                    ROUTINE : cdc_asn1_integ_ext_decode  |
+------------------------------------------------------------------------+

  PURPOSE : Decode UNALIGNED PER extensible ENUMERATED and INTEGER type 
            PER-visible constraints restrict the integer value to be a
            constrained whole number. This gives a lower and an upper 
            bound for the integer. The lb is also called offset. The 
            encoded value is the difference between the actual and the
            offset value.
            A possible and meant default value is never encoded.

            If the value is in the extension root, it will be encoded as
            a normally small non-negative whole number. Otherwise it will 
            be encoded in the smalles number of bits needed to express
            every enumeration.
*/
SHORT cdc_asn1_integ_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat=1, max_rep=1, var_ref, val_ref;
  BOOL    DefaultFound= FALSE;
  S32     IfnotPresent;
  UBYTE   *value, *old_pstruct = NULL;

  var_ref = (ULONG) melem[e_ref].elemRef;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_integ_ext_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_integ_ext_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  /* 
   * For integer with only one possible value in the extension root:
   * If the extensinon bit is 0, the encoded value is in the extension root.
   * In this case no arrays are expected.
   * For other types:
   * The extension bit and the value will be read in the while-loop below.
   */
  if (mvar[var_ref].bSize EQ 0)
  {
    if (bf_readBit (globs) EQ FALSE)
    {
      Read_unique_Integer (e_ref, globs);
      return 1;
    }
    else
    {
      bf_incBitpos (-1, globs);
    }
  }

  val_ref = (ULONG) mvar[var_ref].valueDefs;
  /* 
   * Set pstrcutOffs and max_rep. Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &max_rep, globs) NEQ ccdOK)
    return 1;

#ifdef DYNAMIC_ARRAYS
  /*
   * Allocate memory if this is a pointer type (dynamic array)
   */
  if ( is_pointer_type(e_ref) ) {
    old_pstruct = globs->pstruct;
    if ( PER_allocmem_and_update(e_ref, max_rep, globs) NEQ ccdOK)
      /* No memory - Return.  Error already set in function call above. */
      return 1;
  }
#endif

  /* 
   * Check if there is a default value for the element. 
   * If yes, just set it aside for a later comparision.  
   */
  if (mval[val_ref+1].isDefault EQ 2)
  {
    IfnotPresent = mval[val_ref+1].startValue;
    DefaultFound = TRUE;
  }

  /*
   * Decode all elements of the array.
   */
  while ( repeat <= max_rep)
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
      switch (mvar[var_ref].cType)
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
                  break;
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

      lb = mval[val_ref].startValue;
      ub = mval[val_ref].endValue;
      /*
       * Read first the extensinon bit.
       * Then the non-negative value from the air message.
       */
      if (bf_readBit (globs) EQ FALSE)
      { 
        readBits = bf_getBits (mvar[var_ref].bSize, globs);
      }
      /* Value out of the extension root. */ 
      else
      {
      	U16 calcRef = calcidx[melem[e_ref].calcIdxRef].condCalcRef;
        /*
         * Get max value of integer within the extension root.
         */
        if (calcRef EQ NO_REF 
            OR 
            calc[calcRef].operation NEQ 'P')
        {
          ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT)   
                           e_ref, globs->pstruct+globs->pstructOffs);
        }
        else
        {
          lb = calc[calcRef].operand;
        }
        readBits = Read_NormallySmallNonNegativeWholeNr (globs);
      }      


      if (readBits <= (U32)(ub - lb))
      {
        DecodedValue = lb + readBits;
        /* 
         * Add the offset to the read value to get the actual one.
         */
        switch (mvar[var_ref].cType)
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
    globs->pstructOffs += mvar[var_ref].cSize; 
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
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_integ_ext             |
| STATE   : code                    ROUTINE : cdc_asn1_integ_ext_encode  |
+------------------------------------------------------------------------+

  PURPOSE : UNALIGNED PER extensible ENUMERATED and INTEGER type 
            PER-visible constraints restrict the integer value to be a
            constrained whole number. This gives a lower and an upper 
            bound for the integer. The lb is also called offset. The 
            encoded value is the difference between the actual and the
            offset value.
            A possible and meant default value is never encoded.

            If the value is in the extension root, it will be encoded as
            a normally small non-negative whole number. Otherwise it will 
            be encoded in the smalles number of bits needed to express
            every enumeration.

			       -----------------------------------------------
			      | extension  | value encoded as a normally      |
			      | bit = 1    | non-negtive integer whole number |
			       -----------------------------------------------

			       -----------------------------------------------
			      | extension  | value encoded in the bit size    |
			      | bit = 0    | needed to express ub - lb        |
			       -----------------------------------------------
*/
SHORT cdc_asn1_integ_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG   repeat=1, max_rep=1, var_ref, val_ref;
  BOOL    DefaultFound= FALSE;
  S32     IfnotPresent;
  U8     *base_pstruct;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_integ_ext_encode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_integ_ext_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  var_ref = (ULONG) melem[e_ref].elemRef;
  val_ref = (ULONG) mvar[var_ref].valueDefs;

  /* 
   * Set pstrcutOffs and max_rep. Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &max_rep, globs) NEQ ccdOK)
    return 1;
  /* 
   * Check if there is a default value for the element. 
   * If yes, just set it aside for a later comparision.  
   */
  if (mval[val_ref+1].isDefault EQ 2)
  {
    IfnotPresent = mval[val_ref+1].startValue;
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
  while ( repeat <= max_rep)
  {
    S32     ub, lb, value;
    U32     extension_lb;
    UBYTE   *p;
    U16     calcRef = calcidx[melem[e_ref].calcIdxRef].condCalcRef;
    /*
     * Get offset value of integer in the extension addition.
     */
    if (calcRef EQ NO_REF 
        OR 
        calc[calcRef].operation NEQ 'P')
    {
      ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK,(USHORT) e_ref,
                       globs->pstruct+globs->pstructOffs);
      return 1;
    }
    else
    {
      extension_lb = calc[calcRef].operand;
    }
    /*
     * setup the read pointer to the element in the C-structure
     */
    p = base_pstruct + globs->pstructOffs;

    switch (mvar[var_ref].cType)
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
      if (mvar[var_ref].bSize < 32)
      {
        value = (S32)*(U32 *) p;
      }
      break;
    case 'M':
      value = *(S32 *) p;
      break;
    default:
      ccd_recordFault (globs,ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, p);
      return 1;
    }

    if (mvar[var_ref].cType EQ 'L' AND  
           (mvar[var_ref].bSize EQ 32))
    {
      U32 CriticalValue;
      CriticalValue = *(U32 *) p;
      if (CriticalValue >= extension_lb)
      {
        bf_writeBit (1, globs);
        if (!DefaultFound OR (U32)IfnotPresent NEQ CriticalValue)
          Write_NormallySmallNonNegativeWholeNr (CriticalValue - extension_lb, globs);
      }
      else
      {
        bf_writeBit (0, globs);          
        if (!DefaultFound OR (U32)IfnotPresent NEQ CriticalValue)
        {
          U32 lb, ub;
          lb = (U32) mval[val_ref].startValue;
          ub = (U32) mval[val_ref].endValue;
          if (lb <= CriticalValue && CriticalValue <= ub)
          {
            bf_writeVal (CriticalValue - lb, mvar[var_ref].bSize, globs);
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
         * Set the extension bit to 0 if the value belongs to the extension root.
         * Otherwise set it to 1.
         * A non-negative-binary-integer will be encoded since the offset must
         * be subtracted from the value read from the C-structure.
         */
        lb = mval[val_ref].startValue;
        ub = mval[val_ref].endValue;

        if (value >= (S32)extension_lb AND value <= ub)
        {
          bf_writeBit (1, globs);
          Write_NormallySmallNonNegativeWholeNr ((U32)value - extension_lb, globs);
        }
        else if (lb <= value AND value <= ub)
        {
          bf_writeBit (0, globs);
          /* 
           * Do not encode single valued extension roots.
           */
          if (mvar[var_ref].bSize NEQ 0) 
            bf_writeVal ((ULONG)(value-lb), mvar[var_ref].bSize, globs);
        }
        else
        {
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "integer out of range! %ld require: %ld .. %ld ", value, lb, ub);
#endif
          ccd_recordFault (globs, ERR_INT_VALUE, CONTINUE, (USHORT) e_ref, p);
        }
      }
    } /* value not critical*/
    repeat ++; 
    globs->pstructOffs += mvar[var_ref].cSize;
  }/* while-loop */

  return 1;
}
#endif /* !RUN_INT_RAM */

/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_choice_ext.c
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
|  Purpose :  Encoding and decoding functions for ASN1_CHOICE_EXTENSIBLE type
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
| PROJECT : CCD (6144)              MODULE  : asn1_choice_ext            |
| STATE   : code                    ROUTINE : cdc_asn1_choice_ext_decode |
+------------------------------------------------------------------------+

  PURPOSE : Decode PER extensible CHOICE type 

            The value of the union tag must be calculated from the encoded
            CHOICE index in the air message.
            For CHOICE alternatives within the extension root: the index 
            bit-size relates to max index value in the extension root.
            For CHOICE alternatives within the extension part: the index
            is encoded as a normally small non-negative whole number.

            For decoding the CHOICE alternative itself, call the same 
            function as for non-extensible CHOICE types.
*/
SHORT cdc_asn1_choice_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U32    extensionBegin;
  T_ENUM UnionTag; /* default: only one alternative */
  ULONG  calc_ref = calcidx[melem[e_ref].calcIdxRef].condCalcRef;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_choice_ext_decode()");
  #else
  TRACE_CCD (globs, "cdc_asn1_choice_ext_decode() %s", mcomp[melem[e_ref].elemRef].name);
  #endif
#endif

  /* Don't do anything for empty choices */
  if (mcomp[melem[e_ref].elemRef].numOfComponents == 0)
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
   * Get max value of CHOICE index within the extension root.
   */
  if (calc_ref EQ NO_REF 
      OR 
      calc[calc_ref].operation NEQ 'P')
  {
    ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref, 
                     globs->pstruct+globs->pstructOffs);
    return 1;
  }
  else
  {
    extensionBegin = calc[calc_ref].operand;
  }

  /* 
   * Read the extensinon bit.
   * If set to 1, read the CHOICE index as a normally samll
   * non-negative whole number. Otherwise read it according
   * to the bitSize of index in the extension root.
   */
  if (bf_readBit (globs) EQ 0)
  {
    /* CHOICE index is encoded only if there are more than one alternatives. */
    if (mcomp[melem[e_ref].elemRef].numOfComponents > 1)
    {
      UnionTag = (T_ENUM)bf_getBits (bitSize[extensionBegin], globs);
    }
    else
      UnionTag = 0;

    if (UnionTag >= (T_ENUM)extensionBegin)
    {
      ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK, (USHORT) e_ref, 
                       globs->pstruct+globs->pstructOffs);
    }
    PER_Decode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);
  }
  else
  {
    U32 finalBP=0; 
    UnionTag = (T_ENUM)Read_NormallySmallNonNegativeWholeNr (globs);
    UnionTag += extensionBegin;
    finalBP = Read_OpenTpye_Length (globs)*8;
    finalBP += globs->bitpos;

    /*
     * For unknown extension skip over the octets.
     */
    if (UnionTag <= (T_ENUM)mcomp[melem[e_ref].elemRef].numOfComponents)
    {
      PER_Decode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);
    }
    else
    {
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "Unknown extension with CHOICE index = %ld...skipped to the bit %ld", UnionTag, finalBP);
#endif
    }
    bf_setBitpos (finalBP, globs);
  }

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_choice_ext            |
| STATE   : code                    ROUTINE : cdc_asn1_choice_ext_encode |
+------------------------------------------------------------------------+

  PURPOSE : Encode PER extensible CHOICE type
            
      Evaluate the union controller.
      1) It refers to a CHOICE alternative within the extension root:
         Set the extension bit to 0. 
         Write the CHOICE index. 
         Then encode the chosen alternative.
       ------------------------------------
      | extension  | CHOICE  | encoded     |
      | bit        | index   | alternative |
       ------------------------------------

      2) It refers to a CHOICE alternative within the extension list:
        Set the extension bit to 1. 
         Encode the CHOICE index as a normally small non-negative whole nr. 
         Skip over 6 bits dedicated to length determinant.
         Encode the chosen alternative. 
         Calculate the bit size of the encoded alternative.
         Encode the calculated bit size into the field of length 
         determinant. Right shift the encoded alternative, if 6 bits 
         are not enough for the length determinant.
       ------------------------------------------------
      | extension | CHOICE | length      | encoded     |
      | bit       | index  | determinant | alternative |
       ------------------------------------------------

            For encoding the CHOICE alternative itself, call the same 
            function as for non-extensible CHOICE types.
*/
SHORT cdc_asn1_choice_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U16    startBitpos, Len, EndBitPos;
  T_ENUM UnionTag; /* default: only one alternative */
  U32    extensionBegin;
  ULONG  calc_ref= calcidx[melem[e_ref].calcIdxRef].condCalcRef;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_choice_ext_encode()");
  #else
  TRACE_CCD (globs, "cdc_asn1_choice_ext_encode() %s", mcomp[melem[e_ref].elemRef].name);
  #endif
#endif

  /* Don't do anything for empty choices */
  if (mcomp[melem[e_ref].elemRef].numOfComponents == 0)
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
   * Get max value of CHOICE index within the extension root.
   */
  if (calc_ref EQ NO_REF 
      OR 
      calc[calc_ref].operation NEQ 'P')
  {
    ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK,(USHORT) e_ref, 
                     globs->pstruct+globs->pstructOffs);
    return 1;
  }
  else
  {
    extensionBegin = calc[calc_ref].operand;
  }

  /*
   * Get the value of chosen index (= union controller).
   */
  UnionTag = *(T_ENUM *) (globs->pstruct+globs->pstructOffs);

  /*
   * The chosen alternative belongs to the extension list.
   */
  if ((U32)UnionTag >= extensionBegin)
  {
    /* Encode the extension bit first */
    bf_writeBit (1, globs);
    /* Encode the CHOICE index. */
    Write_NormallySmallNonNegativeWholeNr (((U32)UnionTag - extensionBegin), globs);

    /* Skip over the bits estimated for the length determinant. */
    bf_incBitpos (8, globs);
    startBitpos = globs->bitpos;
    PER_Encode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);

    /* 
     * Check if zero padding bits are necessary. If encoding 
     * consumed no bits, insert a zero-octet in the bit string.
     * Then calculate length of the encoded open type in octets.
     */
    if ((Len = globs->bitpos - startBitpos) EQ 0)
    {
      bf_writeVal (0, 8, globs);
      Len = 1;
    }
    else 
    {
      if ((Len&7) NEQ 0)
      bf_incBitpos (8 - (Len&7), globs);		  
      Len = (U16)(globs->bitpos - startBitpos) >> 3;
    }
    EndBitPos = globs->bitpos;

    /* 
     * Encode the length determinant.
     */
    if (Len < 128)
    {
      bf_setBitpos (startBitpos-8, globs);
      Write_OpenTpye_Length ((U32)Len, globs);
    }
    /*
     * This case does not seem to happen very often.
     */
    else
    {
      ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK,(USHORT) e_ref, 
                       globs->pstruct+globs->pstructOffs);
    }
    /* 
     * Encoding for the extensible choice is finished.
     * Set the bit position pointer to the end of encoded open type.
     */
    bf_setBitpos (EndBitPos, globs);
  }
  /*
   * The chosen alternative belongs to the extension root.
   */
  else
  {    
    bf_writeBit (0, globs);
    /* CHOICE index is encoded only if there are more than one alternatives. */
    if (mcomp[melem[e_ref].elemRef].numOfComponents > 1)
    {
      bf_writeVal ((ULONG) UnionTag, (ULONG) bitSize[extensionBegin], globs);
    }
    PER_Encode_ASN1_CHOICE_alterative (e_ref, UnionTag, globs);
  }

  return 1;
}
#endif /* !RUN_INT_RAM */

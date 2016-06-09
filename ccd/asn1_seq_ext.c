/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_seq_ext.c
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
|  Purpose :  Encoding and decoding functions for ASN1_SEQ_EXTENSIBLE type
|             This module has some issues in common with asn1_seq.c.
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
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_seq_ext               |
| STATE   : code                    ROUTINE : cdc_asn1_seq_ext_decode    |
+------------------------------------------------------------------------+

  PURPOSE : Decode basic UNALIGNED PER for extensible SEQUENCE type and
            SEQUENCE OF. 

            The element can be a field of fixed or variable length.
            It can contain OPTIONAL elements or integer elements with 
            a DEFAULT value. The so called nonCriticalExtensions or empty
            sequences belong to the category of bit strings of fixed length
            which is set to 0.

            First the extension bit is decoded, then the elements of the 
            extension root. Finally the elements of the extension addition.
            See also comments to the encoding function.
*/

SHORT cdc_asn1_seq_ext_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat=1, max_rep=1, first_elem, last_elem, elem, ext_begin; 
  ULONG  c_size;
  UBYTE *old_pstruct;
  BOOL   extPresent=FALSE;
	ULONG  calc_ref = calcidx[melem[e_ref].calcIdxRef].condCalcRef;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_seq_ext_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_seq_ext_decode() %s", mcomp[melem[e_ref].elemRef].name);
	#endif
#endif

  /* 
   * This function is called as an equivalent to ccd_decodeComposition.  
   * Hence the increment on globs->ccd_recurs_level.
   */
  globs->ccd_recurs_level ++;

  /* 
   * Set pstrcutOffs and max_rep. 
   * Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &max_rep, globs) NEQ ccdOK)
  {
    globs->ccd_recurs_level --;
    return 1;
  }

  /*
   * Prepare for decoding of the same sequence type up to max_rep times.
   * Set the upper and lower bound of elemRef for processing of each repeatition.
   * Read the C-size to go ahead in the C-structure after each repeatition.
   */
  switch (melem[e_ref].elemType)
  {
    case 'C':
    case 'D':
    case 'E':
    case 'P':
    case 'Q':
              elem       = (ULONG) melem[e_ref].elemRef;
              first_elem = (ULONG) mcomp[elem].componentRef;
              last_elem  = first_elem + mcomp[elem].numOfComponents;
              c_size      = (ULONG) mcomp[elem].cSize;
              break;
    case 'F':
    case 'R':
              first_elem = e_ref;
              last_elem  = e_ref + 1;
              c_size     = mvar[e_ref].cSize;
              break;
    default:
              ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                        (USHORT) (globs->bitpos), (USHORT) -1);
              return 1;
         
  };

#ifdef WIN32
  if (calc_ref EQ NO_REF OR calc[calc_ref].operation NEQ 'P')
  {
    ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT)  e_ref, 
                     globs->pstruct+globs->pstructOffs); 
    return 0;
  }
#endif

  /*
   * Get max index for elements within the extension root.
   */
   ext_begin = (ULONG) calc[calc_ref].operand;

  /*
   * Store the current value of the C-structure pointer.  
   * After decoding the SEQUENCE component we will set the pointer 
   * to this stored value. Then we will use pstructOffs for pointing   
   * to the next element.
   */
  old_pstruct = globs->pstruct;
#ifdef DYNAMIC_ARRAYS
  /*
   * Allocate memory for this whole composition, if elemType is
   * one of the pointer types.
   */
  if ( is_pointer_type(e_ref))
  {
    if ( PER_allocmem_and_update(e_ref, max_rep, globs) NEQ ccdOK )
    {
    /* No memory - Return.  Error already set in function call above. */
    globs->ccd_recurs_level --;
    return 1;
    }

  }
#endif
  globs->pstruct += globs->pstructOffs;

  /*
   * Decode all elements of the field for SEQUENCE (SIZE) OF .
   */
  while (repeat <= max_rep)
  {
    /* 
     * Read the extensinon bit for each array SEQUENCE.
     */
    if (bf_readBit (globs) EQ 1)
    {
      extPresent = TRUE;
    }

    /* 
     * Decode the bit-map preamble for the extension root.
     */
    Read_SEQ_BitMap (first_elem, ext_begin, globs);
     

    /*
     * Decode elements in the extension root for each array SEQUENCE.
     */
    for (elem = first_elem; elem < ext_begin; elem++)
    {
#ifdef ERR_TRC_STK_CCD
      /* 
       * Save the value for tracing in error case.
       */
      globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
		TRACE_CCD (globs, "decoding level %d element %d", globs->ccd_recurs_level, elem - first_elem);
	#else
    TRACE_CCD (globs, "decoding level %d element %d '%s'",globs->ccd_recurs_level, elem - first_elem, ccddata_get_alias((USHORT) elem, 1));
	#endif
#endif
      /*
       * Use the jump-table for selecting the decode function.
       * Possible types are 0, BITSTRING, ASN1_OCTET, ASN1_INTEGER(_EXTENSIBLE), 
       * ASN1_CHOICE(_EXTENSIBLE) andASN1_SEQUENCE(_EXTENSIBLE).
       * In case of 0 function cdc_STD_decode will be called.
       */
			(void) codec[melem[elem].codingType][DECODE_FUN]
                                            (c_ref, elem, globs);      
    }
/* ############################################### */
    /*
     * Decode extension elements if anyone present
     */
    if (extPresent)
    {
      int   unknownExt_nr=0;
      ULONG bmp_len=0;

      /* 
       * Read length of bit-map for the encoded extension additions.
       */
      bmp_len = Read_NormallySmallNonNegativeWholeNr (globs) + 1;

      /* 
       * Decode the bit-map for the encoded extension additions.
       */
      for (elem = ext_begin; elem < last_elem AND unknownExt_nr < (int)bmp_len; elem++)
      {
        if (melem[elem].optional)
        {
          unknownExt_nr++;
          if (melem[elem].elemType < 'P' OR melem[elem].elemType > 'R')
          {
            globs->pstruct[melem[elem].structOffs] = (UBYTE) bf_readBit(globs);
          }
          else
          {
            if(bf_readBit(globs)) /*elemType P, Q or R*/
            {
              *(void**) &globs->pstruct[melem[elem].structOffs] = (void *) 0xFFFF;
            }
            else /*Not present set the pointer to NULL*/
              *(void**) &globs->pstruct[melem[elem].structOffs] = NULL;
          }
        }
        else if (melem[elem].codingType EQ CCDTYPE_ASN1_INTEGER)
        {
          if (mval[mvar[melem[elem].elemRef].valueDefs+1].isDefault EQ 2)
          {
            unknownExt_nr++;
            globs->pstruct[melem[elem].structOffs] = (UBYTE) bf_readBit(globs);
          }           
        }
      }

      /* 
       * Decode the bit-map part for unknown extension additions.
       */
      if ((unknownExt_nr = (int) bmp_len - (last_elem - ext_begin)) > 0)
      {
        UBYTE tmp_bmp_len=0;
        while (unknownExt_nr--)
        {
          if (bf_readBit(globs))
            tmp_bmp_len++;
        }
        unknownExt_nr = (int)tmp_bmp_len;
      }

      /*
       * Decode elements in the extension root for each array SEQUENCE
       */
      for (elem = ext_begin; elem < last_elem; elem ++)
      {
        U32 finalBP; 
#ifdef ERR_TRC_STK_CCD
        /* 
         * Save the value for tracing in error case.
         */
        globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
		    TRACE_CCD (globs, "decoding level %d element %d", globs->ccd_recurs_level, elem - first_elem);
	#else
        TRACE_CCD (globs, "decoding level %d element %d '%s'",globs->ccd_recurs_level, elem - first_elem, ccddata_get_alias((USHORT) elem, 1));
	#endif
#endif

        /* Decode only present extensions. */
        globs->pstructOffs = melem[elem].structOffs;
        if ( cdc_isPresent(elem, globs) )
        {
          finalBP = 8*Read_OpenTpye_Length (globs);
          finalBP += globs->bitpos;

          /*
           * Use the jump-table for selecting the decode function.
           * Possible types are 0, BITSTRING, ASN1_OCTET, ASN1_INTEGER(_EXTENSIBLE), 
           * ASN1_CHOICE(_EXTENSIBLE) andASN1_SEQUENCE(_EXTENSIBLE).
           * In case of 0 function cdc_STD_decode will be called.
           */
			    (void) codec[melem[elem].codingType][DECODE_FUN]
                                              (c_ref, elem, globs);
          if (globs->bitpos > finalBP)
          {
            /*  ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK, (USHORT) e_ref, 
                                 globs->pstruct+melem[e_ref].structOffs);*/
          } 
          bf_setBitpos (finalBP, globs);
        }       
      } /* for: SEQUENCE extensions */

      /* For unknown extensions skip over open type and its length determinant */
      while (unknownExt_nr--)
      {
        U32 opLen = Read_OpenTpye_Length (globs);
        bf_incBitpos (8 * opLen, globs);
#ifdef DEBUG_CCD
TRACE_CCD (globs, "Unknown extension for SEQUENCE type...skipped over %ld bytes", opLen);
#endif
      } /*if: unknown extension */
    } /*if: extPresent*/

    /*
     * Set the pointer of the C-structure on the next element.
     */
    globs->pstruct += c_size; 

    extPresent = FALSE;
    repeat ++;
  } /* while: SEQUENCE field*/

  /*
   * Prepare for decoding the next element.
   */
  globs->pstruct = old_pstruct;
  globs->ccd_recurs_level--;

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_seq_ext               |
| STATE   : code                    ROUTINE : cdc_asn1_seq_ext_encode    |
+------------------------------------------------------------------------+

  PURPOSE : Encode basic UNALIGNED PER for extensible SEQUENCE type and
            SEQUENCE OF. 

            The element can be a field of fixed or variable length.
            It can contain OPTIONAL elements or integer elements with 
            a DEFAULT value. The so called nonCriticalExtensions or empty
            sequences belong to the category of bit strings of fixed length
            which is set to 0.

            First the extension bit is encoded, then the elements of the 
            extension root, finally the elements of the extension addition.
            One of the following scenarios is possible:

			 ----------------------------------------
			| extension  | encoded extension root of |
			| bit = 0    | including bitmap preamble |
			 ----------------------------------------
 -----------------------------------------------------------------------------
| extension |           | encoded   | encoded elment  | encoded elment  |     |
| bit = 1   |           | bitmap    | x of the        | y of the        |     |
|           |           | including | extension part  | extension part  |     |
|           |           | its       | as an open type | as an open type | ... |
|           |           | length    | including       | including       |     | 
|           |           |           | its length      | its length      |     |
 -----------------------------------------------------------------------------
 -----------------------------------------------------------------------------
| extension | encoded   | encoded   | encoded elment  | encoded elment  |     |
| bit = 1   | extension | bitmap    | x of the        | y of the        |     |
|           | root      | including | extension part  | extension part  |     |
|           | including | its       | as an open type | as an open type | ... |
|           | bitmap    | length    | including       | including       |     | 
|           | preamble  |           | its length      | its length      |     |
 -----------------------------------------------------------------------------
*/
SHORT cdc_asn1_seq_ext_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat=1, max_rep=1, first_elem, last_elem, elem, ext_begin;
  U16    ext_BitPos, ext_bmpLenBP, ext_bmpEndBP; 
  ULONG  c_size, Len;
  UBYTE *old_pstruct;
	ULONG  calc_ref = calcidx[melem[e_ref].calcIdxRef].condCalcRef;

#ifdef DEBUG_CCD
#ifndef CCD_SYMBOLS
TRACE_CCD (globs, "cdc_asn1_seq_ext_encode()");
#else
TRACE_CCD (globs, "cdc_asn1_seq_ext_encode() %s", mcomp[melem[e_ref].elemRef].name);
#endif
#endif

  /* 
   * SEQUENCE contains nested elements.
   */
  globs->ccd_recurs_level ++;

  /* 
   * Set pstrcutOffs and max_rep. 
   * Check the valid flag in case of optional elements.
   */
  if (PER_CommonBegin (e_ref, &max_rep, globs) NEQ ccdOK)
  {
    globs->ccd_recurs_level --;
    return 1;
  }

  /*
   * Prepare for encoding of the same sequence type up to max_rep times.
   * Set the upper and lower bound of elemRef for processing of each repeatition.
   * Read the C-size to go ahead in the C-structure after each repeatition.
   */ 
  switch (melem[e_ref].elemType)
  {
    case 'C':
    case 'D':
    case 'E':
    case 'P':
    case 'Q':
      {
        elem       = (ULONG) melem[e_ref].elemRef;
        first_elem = (ULONG) mcomp[elem].componentRef;
        last_elem  = first_elem + mcomp[elem].numOfComponents;
        c_size     = (ULONG) mcomp[elem].cSize;
        break;
      }
    case 'F':
    case 'R':
      {
        first_elem = e_ref;
        last_elem  = e_ref + 1;           
        c_size     = mvar[e_ref].cSize;
        break;
      }
    default:
        ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                        (USHORT) (globs->bitpos), (USHORT) -1);
        return 1;         
  }
  /*
   * Store the current value of the C-structure pointer.  
   * After encoding the SEQUENCE component we will set the pointer 
   * to this stored value. Then we will use pstructOffs for pointing
   * to the next element.  
   */  
  old_pstruct = globs->pstruct;
  globs->pstruct += globs->pstructOffs;

#ifdef DYNAMIC_ARRAYS
  if ( is_pointer_type(e_ref) ) {
    if ( ccd_check_pointer(*(U8 **)globs->pstruct) == ccdOK )
      globs->pstruct = *(U8 **) globs->pstruct;
    else
    {
       ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                        &globs->pstruct[globs->pstructOffs]);
       return 1;
    }
  }
#endif

#ifdef WIN32
  if (calc_ref EQ NO_REF OR calc[calc_ref].operation NEQ 'P')
  {
    ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref,  
                     globs->pstruct+globs->pstructOffs);
    return 0;
  }
#endif

  /*
   * Get max index for elements within the extension root.
   */
  ext_begin = (ULONG) calc[calc_ref].operand;

  /*
   * Decode all elements of the field for SEQUENCE (SIZE) OF.
   */
  while (repeat <= max_rep)
  {
    /* 
     * Prepare for a later encoding of the extension bit.
     */
    ext_BitPos = globs->bitpos;
    bf_incBitpos (1, globs);

    /* 
     * Encode the bit-map preamble for elements with the extension root.
     */
    Write_SEQ_BitMap (first_elem, ext_begin, globs);

    /* 
     * Encode present elements of the extension root.
     */
    for (elem = first_elem; elem < ext_begin; elem++)
    {
#ifdef ERR_TRC_STK_CCD
      /* 
       * Save the value for tracing in error case.
       */
      globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */    
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "encoding level %d element %d", globs->ccd_recurs_level, elem - first_elem);
	#else
    TRACE_CCD (globs, "encoding level %d element %d '%s'", globs->ccd_recurs_level, elem - first_elem ,ccddata_get_alias((USHORT) elem, 1));
	#endif
#endif

#if defined _TOOLS_
      if (!ccd_patch (globs, 0))
#endif /* _TOOLS_ */
      /*
       * Use the jump-table for selecting the code function.
       * Possible types are 0, ASN1_INTEGER, BITSTRING, ASN1_CHOICE and 
       * ASN1_SEQUENCE. In case of 0 function cdc_STD_encode will be called.
       */
      (void) codec[melem[elem].codingType][ENCODE_FUN]
                                            (c_ref, elem, globs);
    }

    ext_bmpLenBP = globs->bitpos;
    /* 
     * Set the extension bit to 0, if extension part is empty.
     */
    if (ext_begin EQ last_elem)
    {
      bf_setBitpos (ext_BitPos, globs);
      bf_writeBit  (0, globs);
      bf_setBitpos (ext_bmpLenBP, globs);
    }
    else
    {
      /* 
       * Prepare for a later encoding of the bitmap length.
       */
      
      bf_incBitpos (7, globs);

      /*
       * Write the bitmap preamble for the extension part.
       */
      Write_SEQ_BitMap (ext_begin, last_elem, globs);
      ext_bmpEndBP = globs->bitpos;
      Len = last_elem - ext_begin; // equivalent to: globs->bitpos - ext_bmpLenBP -7;

      /* Check any of the extension elements is present. */
      bf_setBitpos (ext_bmpLenBP+7, globs);
      if (bf_getBits (Len, globs) EQ 0)
      { /* Hint: the following two lines can be deleted because of ccd general memset. */
        bf_setBitpos (ext_BitPos, globs);
        bf_writeBit  (0, globs);
        bf_setBitpos (ext_bmpLenBP, globs);
      }
      else
      {
        /*
         * Write the extension bit.
         */
        bf_setBitpos (ext_BitPos, globs);
        bf_writeBit (1, globs);

        /* 
         * Write the bitmap length assuming small bitmaps as common case.
         * Lower bound of the length is 1.
         /  
        if (Len < 64) 
        { */
        bf_setBitpos (ext_bmpLenBP, globs);
        Write_NormallySmallNonNegativeWholeNr (Len-1, globs);
        bf_setBitpos (ext_bmpEndBP, globs);
        /*
        } else
        {
          encode_bmp_len_as_NormallySmallNonNegativeWholeNr (...);
          shift_the_bitStr (...);
        }*/

        /*
         * Encode present elements of the extension part.
         */         
        for (elem  = ext_begin; elem < last_elem; elem++)
        {
          U16 startBitPos, endBitPos; 
#ifdef ERR_TRC_STK_CCD
          /* 
           * Save the value for tracing in error case.
           */
          globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */    
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
TRACE_CCD (globs, "encoding level %d element %d", globs->ccd_recurs_level, elem - first_elem);
	#else
TRACE_CCD (globs, "encoding level %d element %d '%s'", globs->ccd_recurs_level, elem - first_elem ,ccddata_get_alias((USHORT) elem, 1));
  #endif
#endif
          /* Decode only present extensions. */
          globs->pstructOffs = melem[elem].structOffs;
          if ( cdc_isPresent(elem, globs) )
          {
            /*
             * Prepare for a later encoding of the length for each open type
             */
            bf_incBitpos (8, globs);
            startBitPos = globs->bitpos;

#if defined _TOOLS_
      if (!ccd_patch (globs, 0))
#endif /* _TOOLS_ */

            /*
             * Use the jump-table for selecting the code function.
             * Possible types are 0, ASN1_INTEGER, BITSTRING, ASN1_CHOICE and 
             * ASN1_SEQUENCE. In case of 0 function cdc_STD_encode will be called.
             */
            (void) codec[melem[elem].codingType][ENCODE_FUN]
                                                (c_ref, elem, globs);

            /* 
             * Complete open type encoding. Check if zero padding bits
             * are necessary. If encoding consumed no bits, insert a 
             * zero-octet in the bit string.
             * Then calculate length of the encoded open type in octets.
             */
            if ((Len = globs->bitpos - startBitPos) EQ 0)
            {
              bf_writeVal (0, 8, globs);
              Len = 1;
            }
		        else 
            {
              if ((Len&7) NEQ 0)
			          bf_incBitpos (8 - (Len&7), globs);		  
  		        Len = (U32)(globs->bitpos - startBitPos) >> 3;
            }

            endBitPos = globs->bitpos;

            /* 
             * Encode the length determinant.
             */
            if (Len < 128)
            {
              bf_setBitpos (startBitPos-8, globs);
              Write_OpenTpye_Length ((U32)Len, globs);
            }
            /*
             * This case does not seem to happen very often.
             * We could let an error report lead us to the need for total implementation.
             */
            else
            {
            /*ccd_recordFault (globs, ERR_ASN1_ENCODING, BREAK,(USHORT) e_ref,   
                               globs->pstruct+globs->pstructOffs);*/
            }
            /* 
             * Encoding for the sequence sub element is finished. For further encoding
             * set the bit position pointer to the end of the encoded open type.
             */
            bf_setBitpos (endBitPos, globs);
          }
        }
      }/* if ext present or not */
    } /* if any extension defined or not*/

    /*
     * Set the pointer of the C-structure on the next element.
     */ 
    globs->pstruct += c_size;

    repeat ++;
  } /* while: SEQUENCE field*/

  /*
   * Prepare for encoding the next element.
   */
  globs->pstruct = old_pstruct;
  globs->ccd_recurs_level--;
  return 1;
}
#endif /* !RUN_INT_RAM */

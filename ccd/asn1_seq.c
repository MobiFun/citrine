/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : asn1_seq.c
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
|  Purpose :  Definition of encoding and decoding functions for ASN1_SEQUENCE
|             elements 
+----------------------------------------------------------------------------- 
*/ 

#define ASN1_SEQ_C

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
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_seq               |
| STATE   : code                    ROUTINE : Read_SEQ_BitMap        |
+--------------------------------------------------------------------+

  PURPOSE : Decode the bit-map preamble for OPTIONAL elements 
            or those with DEFAULT value.
*/
void Read_SEQ_BitMap (const ULONG first_elem, const ULONG last_elem, T_CCD_Globs *globs)
{
  ULONG elem = first_elem;
  while (elem < last_elem)
  {
    if (melem[elem].optional)
    {
      /* 
       * For optional elements read the corresponding bit in the preamble
       * and set the valid flag in the C-Structure to it.
       */             
      if (melem[elem].elemType < 'P' OR melem[elem].elemType > 'R')
      {
        globs->pstruct[melem[elem].structOffs] = (UBYTE) bf_readBit(globs);
      }
      else
      {
        if(bf_readBit(globs)) /*elemType P, Q or R*/
        {
          /*If present set the pointer to -1 (0xFFFF) - anything else than NULL, because the 
          element is present*/
          *(void**) &globs->pstruct[melem[elem].structOffs] = (void *) 0xFFFF;
        }
        else /*Not present set the pointer to NULL*/
          *(void**) &globs->pstruct[melem[elem].structOffs] = NULL;
      
      }
    }
    /* 
     * For optional elements read the corresponding bit in the preamble
     * and set the valid flag in the C-structure to it.
     */
    else if (melem[elem].codingType EQ CCDTYPE_ASN1_INTEGER)
    {       
      /* 
       * Check if this variable has a default value.
       * As long as the DEFAULT values are given with ranges it is right to
       * look for isDefault in the second entry in mval.cdg.
       * There is no valid flag for elements with default value. But we use
       * the first byte of this element within the C-structure for giving a
       * signal to this function. The only simple type which has DEFAULT is INTEGER.
       */
      if (mval[mvar[melem[elem].elemRef].valueDefs+1].isDefault EQ 2)
      {
        globs->pstruct[melem[elem].structOffs] = (UBYTE) bf_readBit(globs);
      }           
    }
    elem++;
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_asn1_seq_decode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the SEQUENCE and SEQUENCE OF type for UMTS
            The element can be a field of fixed or variable length.
            It can contain OPTIONAL elements or integer elements with 
            a DEFAULT value. 
            A special case is when the element is a so called msg_data. 
            According to CCDDATA a message is made of a msg_type and 
            a msg_data part. If the msg_data has a coding type of 
            ASN1_SEQUENCE this function is called.
            In this case CCD needs to pass over the msg_type by 
            incrementing globs->pstruct. A msg_data sequence can not 
            be optional. Nor it can be an array of msg_data. 
            If the sequence is not a msg_data this function is called 
            as an equivalent to ccd_decodeComposition. Hence the 
            increment on globs->ccd_recurs_level. A non-msg_data 
            sequence can be optional or an array.    
*/ 
SHORT cdc_asn1_seq_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat=1, max_rep=1; 
  ULONG  cSize, first_elem, last_elem, elem;
  UBYTE  *old_pstruct;
#ifdef DEBUG_CCD
	static S8 trace_nesting_level = -1;
#endif

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_seq_decode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_seq_decode() %s", mcomp[melem[e_ref].elemRef].name);
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
              cSize      = (ULONG) mcomp[elem].cSize;
              break;
    case 'F':
    case 'R':
              first_elem = e_ref;
              last_elem  = e_ref + 1;
              cSize      = (ULONG) mvar[e_ref].cSize;
              break;
    default:
              ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, 
                        (USHORT) (globs->bitpos), (USHORT) -1);
              return 1;
         
  };
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
   * Decode all elements of the field.
   */
  while (repeat <= max_rep)
  {
    Read_SEQ_BitMap (first_elem, last_elem, globs);
    elem = first_elem;
    
    /*
     * Decode all elements of the array
     */
#ifdef DEBUG_CCD
		trace_nesting_level++;
#endif
    while (elem < last_elem)
    {
#ifdef ERR_TRC_STK_CCD
      /* 
       * Save the value for tracing in error case.
       */
      globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
		TRACE_CCD (globs, "decoding level %d element %d", trace_nesting_level, elem - first_elem);
	#else
    TRACE_CCD (globs, "decoding level %d element %d '%s'",trace_nesting_level, elem - first_elem, ccddata_get_alias((USHORT) elem, 1));
	#endif
#endif

      /*
       * Use the jump-table for selecting the decode function.
       * Possible types are 0, ASN1_INTEGER, BITSTRING, ASN1_CHOICE and 
       * ASN1_SEQUENCE. In case of 0 function cdc_STD_decode will be called.
       */
			(void) codec[melem[elem].codingType][DECODE_FUN]
                                            (c_ref, elem, globs);
      elem ++;
    }
#ifdef DEBUG_CCD
		trace_nesting_level--;
#endif

    /*
     * Set the pointer of the C-structure on the next element.
     */
    globs->pstruct += cSize; 

    repeat ++;
  }

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
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : asn1_seq               |
| STATE   : code                    ROUTINE : Write_SEQ_BitMap       |
+--------------------------------------------------------------------+

  PURPOSE : Encode the bit-map preamble for OPTIONAL elements 
            or those with DEFAULT value.
*/
void Write_SEQ_BitMap (const ULONG first_elem, const ULONG last_elem, T_CCD_Globs *globs)
{
  ULONG elem = first_elem;
    while (elem < last_elem)
    {
      if (melem[elem].optional)
      {
        /* 
         * For optional elements read the valid flag in the C-Structure  
         * and overwrite the corresponding bit in the preamble.
         */             

#if defined _TOOLS_
        int patch;
        U16 stelem;
        stelem = globs->error_stack[globs->ccd_recurs_level];
        globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
        patch = ccd_patch (globs, 1);
        globs->error_stack[globs->ccd_recurs_level] = stelem;
        if (patch)
        {
          bf_writeBit(1, globs);
          elem++;
          continue;
        }
#endif /* _TOOLS_ */

#ifdef DYNAMIC_ARRAYS
	    if ( is_pointer_type(elem) )
      {
	      BOOL   present;
	      /*
	       * Check for NULL pointer (== notPresent/not valid) for this element.
	       */
	      present = (*(U8 **)(globs->pstruct + melem[elem].structOffs) NEQ NULL);
	      /*
	       * Double check for 'D' to 'F' types. Both NULL pointer and
	       * valid flag. (Strictly not necessary, but may catch uninitialized
	       * flags/pointers).
	       */
	      if (present AND (melem[elem].elemType >= 'D' AND
			       melem[elem].elemType <= 'F')) 
        {
	        present = (BOOL)globs->pstruct[melem[elem].structOffs];
	      }
	      bf_writeBit(present, globs);
	    } 
      else
#endif
	      bf_writeBit((BOOL)globs->pstruct[melem[elem].structOffs], globs);
      }
      else if (melem[elem].codingType EQ CCDTYPE_ASN1_INTEGER)
      {       
        /* 
         * Check if this variable has a default value.
         * As long as the DEFAULT values are given with ranges it is right to
         * look for isDefault in the second entry in mval.cdg.
         * There is no valid flag for elements with default value. So we need
         * to read the value from C-structure and compare it with the DEFAULT 
         * value given in the mval table. The only simple type which has 
         * DEFAULT is INTEGER. 
         */
        if (mval[mvar[melem[elem].elemRef].valueDefs+1].isDefault EQ 2)
        {
          U32 value=0;
          UBYTE *p;
          /*
           * setup the read pointer to the element in the C-structure
           */
#ifdef DYNAMIC_ARRAYS
          if ( is_pointer_type(elem) )
          {
            /*
             * NULL pointers should be caught as part of the optionality
             * check in PER_CommonBegin()
             */
            p = *(U8 **)(globs->pstruct + globs->pstructOffs);
            if (ccd_check_pointer(p) != ccdOK)
            {
               ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) elem, 
                                &globs->pstruct[globs->pstructOffs]);
               return;
            }
          }
          else
#endif
          p = globs->pstruct + melem[elem].structOffs;
          /*
           * Default values are all positive and small values (< 64K)
           * in the current version of umts. The cases 'L' and 'C' are added
           * to be ready for a future change where we need also a check of
           * possible errors through type casting.
           */
          switch (mvar[melem[elem].elemRef].cType)
          {
            case 'B':
                      value = (U32)*(UBYTE *) p;
                      break;
            case 'C':
                      value = (U32)(S32)*(S8 *) p;
                      break;
            case 'S':
                      value = (U32)(*(USHORT *) p);
                      break;
            case 'T':
                      value = (U32)(S32)*(S16 *) p;
                      break;
            case 'L':
                      value = (U32)*(S32 *) p;
                      break;
            case 'M':
                      value = (U32)*(U32 *) p;
                      break;
            default:
                      ccd_setError (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT) -1);
                      break;
          }
          /* 
           * Compare the value to be encoded with the default value. 
           * Write the presence flag into the preamble.
           */
          if (value EQ (U32)mval[mvar[melem[elem].elemRef].valueDefs+1].startValue)
          {
            bf_writeBit(0, globs);
          }
          else
          {
            bf_writeBit(1, globs);
          }
        }           
      }
      elem++;
    }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_asn1_seq_encode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the SEQUENCE and SEQUENCE OF type for UMTS
            The element can be a field of fixed or variable length.
            It can contain OPTIONAL elements or integer elements with 
            a DEFAULT value. 
            A special case is when the element is a so called msg_data. 
            According to CCDDATA a message is made of a msg_type and 
            a msg_data part. If the msg_data has a coding type of 
            ASN1_SEQUENCE this function is called.
            In this case CCD needs to pass over the msg_type by 
            incrementing globs->pstruct. A msg_data sequence can not 
            be optional. Nor it can be an array of msg_data. 
            If the sequence is not a msg_data this function is called 
            as an equivalent to ccd_encodeComposition. Hence the 
            increment on globs->ccd_recurs_level. A non-msg_data 
            sequence can be optional or an array.
*/
SHORT cdc_asn1_seq_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat=1, max_rep=1; 
  ULONG  cSize, first_elem, last_elem, elem;
  UBYTE  *old_pstruct;
#ifdef DEBUG_CCD
	static S8 trace_nesting_level = -1;
#endif

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_asn1_seq_encode()");
	#else
	TRACE_CCD (globs, "cdc_asn1_seq_encode() %s", mcomp[melem[e_ref].elemRef].name);
	#endif
#endif

  /* 
   * This function is called as an equivalent to ccd_encodeComposition.
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
              cSize      = (ULONG) mcomp[elem].cSize;
              break;
    }
    case 'F':
    case 'R':
    {
              first_elem = e_ref;
              last_elem  = e_ref + 1;           
              cSize      = (ULONG) mvar[e_ref].cSize;
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
    if (ccd_check_pointer(*(U8 **)globs->pstruct) == ccdOK)
    {
      globs->pstruct = *(U8 **) globs->pstruct;
    }
    else
    {
       ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                        &globs->pstruct[globs->pstructOffs]);
       return 1;
    }
  }
#endif


  /*
   * Encode all elements of the field.
   */
  while (repeat <= max_rep)
  { 
    Write_SEQ_BitMap (first_elem, last_elem, globs); 

    /*
     * Encode all elements
     */
#ifdef DEBUG_CCD
		trace_nesting_level++;
#endif

    elem = first_elem;
    while (elem < last_elem)
    {
#ifdef ERR_TRC_STK_CCD
      /* 
       * Save the value for tracing in error case.
       */
      globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem;
#endif /* ERR_TRC_STK_CCD */    
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "encoding level %d element %d", trace_nesting_level, elem - first_elem);
	#else
    TRACE_CCD (globs, "encoding level %d element %d '%s'", trace_nesting_level, elem - first_elem ,ccddata_get_alias((USHORT) elem, 1));
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
      /*
       * Set the elemRef to the next element.
       */
      elem ++;
    }
#ifdef DEBUG_CCD
		trace_nesting_level--;
#endif
    /*
     * Set the pointer of the C-structure on the next element.
     */ 
    globs->pstruct += cSize;

    repeat ++;
  }

  /*
   * Prepare for encoding the next element.
   */
  globs->pstruct = old_pstruct;
  globs->ccd_recurs_level--;
  return 1;
}
#endif /* !RUN_INT_RAM */

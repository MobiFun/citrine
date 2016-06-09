/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : csn1_concat.c
+----------------------------------------------------------------------------- 
|  Copyright 2004 Texas Instruments Deutschland GmbH 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Deutschland GmbH
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Deutschland GmbH. 
+----------------------------------------------------------------------------- 
|  Purpose :  Condat Conder Decoder - 
|             Definition of encoding and decoding functions of 
|             CSN1 truncated concatenation elements
+----------------------------------------------------------------------------- 
*/ 


/*
 * Standard definitions like GLOBAL, UCHAR, ERROR etc.
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
#include "ccd_codingtypes.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
#include "ccddata.h"

EXTERN T_FUNC_POINTER  codec[MAX_CODEC_ID+1][2];

#ifndef RUN_FLASH
/*
+---------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                    |
| STATE   : code                     ROUTINE : cdc_csn1_concat_decode |
+---------------------------------------------------------------------+

  PURPOSE : decodes the bitstream to a C-Structure.The decoding
            rules contains the element definitions for the
            elements of this message.
            This function may called recursivly because of a
            substructured element definition.
*/

SHORT cdc_csn1_concat_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  /*
   * index in table melem
   */
  ULONG   elem_ref, last_elem, start_elem;
  SHORT   codecRet;
  U8     *actStructpos;
  U8      actErrLabel;
  U16     actMaxBitpos, finalBitPos;
  U8      *pnumConcatElem = NULL;
  ULONG   i, num_concat_elem;
  BOOL    SetPosExpected = FALSE;
  ULONG   cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_concat_decode()");
	#else
	TRACE_CCD (globs, "cdc_csn1_concat_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif
  
  actErrLabel = globs->errLabel;

  /* Set ref number for calcidx table. */
  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * If this element is conditional, check the condition.
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * if this element have a defined Prolog
   * we have to process it before decoding the bitstream
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }

  globs->ccd_recurs_level++;

	if (globs->bitpos < globs->maxBitpos)
	{
    if (melem[e_ref].repType == 's')
    {
      BOOL  is_variable;
      ULONG max_rep, repeat;
      
      is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
      if (repeat > (ULONG) (globs->maxBitpos-globs->bitpos))
      {
        ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                         (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        repeat = MINIMUM (repeat, (ULONG) (globs->maxBitpos-globs->bitpos));
      }
      
      finalBitPos = (USHORT) (globs->bitpos + repeat);
  
  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
        TRACE_CCD (globs, "decoding of concatenation %s as a bit array",
                         mcomp[melem[e_ref].elemRef].name);
  #else
        TRACE_CCD (globs, "decoding of concatenation %d as a bit array", melem[e_ref].elemRef);
  #endif
  #endif
      /* Store the limit. The truncated concatenation may contain 
         other compositions as bitstring. */
      actMaxBitpos = globs->maxBitpos;
      globs->maxBitpos = finalBitPos;
    }
    else 
    {
  #ifdef DEBUG_CCD
  #ifdef CCD_SYMBOLS
  	TRACE_CCD (globs, "decoding concatenation %s",
  				 mcomp[melem[e_ref].elemRef].name);
  #else
  	TRACE_CCD (globs, "decoding concatenation %d", melem[e_ref].elemRef);
  #endif
  #endif
    }

    /*
     * Store the actual structure position.
     */
    actStructpos = globs->pstruct;
    globs->pstructOffs = melem[e_ref].structOffs;
    globs->pstruct += globs->pstructOffs;
    /*
     * setup the index in the melem table for this composition.
     */
    elem_ref  = (ULONG) mcomp[melem[e_ref].elemRef].componentRef;
    last_elem = elem_ref + mcomp[melem[e_ref].elemRef].numOfComponents;
    /*
     * It is recommended to use a leading element of coding type NO_CODE
     * in the message description which is used to count the existing 
     * elements of the truncated concatenation. If this element is missing
     * the decoding process will proceed but the CCD user is forced to 
     * evaluate all of the valid flags.
     */

    if (melem[elem_ref].codingType == CCDTYPE_NO_CODE)
    {
      pnumConcatElem = globs->pstruct;
      elem_ref++;

      num_concat_elem = (ULONG) (mcomp[melem[e_ref].elemRef].numOfComponents - 1);
    }

    start_elem = elem_ref;
    /*
     * decode all elements
     */
    while (elem_ref < last_elem)

    {
  #ifdef ERR_TRC_STK_CCD
      /* save the value for tracing in error case */
      globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem_ref;
  #endif /* ERR_TRC_STK_CCD */

      /*
       * check if the bitstream has ended
       */
      if (bf_endOfBitstream(globs) AND !globs->TagPending)
      {
        /* End of the bit stream is not reached if a call to bf_setBitpos()
         * is expected for the next element of the current substructure. 
         * An instructive example is an empty "mob_id"
         */
        cix_ref = melem[elem_ref].calcIdxRef;
        num_prolog_steps = calcidx[cix_ref].numPrologSteps;
        prolog_step_ref  = calcidx[cix_ref].prologStepRef;

        if (num_prolog_steps)
        {
          i = prolog_step_ref + num_prolog_steps;
          
          while (i >= prolog_step_ref)
          {
            if (calc[i].operation == 'S')
            {
              SetPosExpected = TRUE;
              break;
            }
            i--;
          }
        }

        if (SetPosExpected EQ FALSE)
        {
          num_concat_elem = elem_ref - start_elem;
          /* after the while loop the recursion level will be decremented. */
          break;
        }

      }//if end of bit string

      /*
       * use the jump-table for selecting the decode function
       */
      codecRet =
        codec[melem[elem_ref].codingType][DECODE_FUN](melem[e_ref].elemRef,
                                                      elem_ref, globs);
      if (codecRet NEQ 0x7f)
      {
        /*
         * set the elem_ref to the next or the same element
         */
        elem_ref += codecRet;
      }
    }

    if (pnumConcatElem != NULL)
    {
      *pnumConcatElem = (UBYTE) num_concat_elem;
    }

  
  if (melem[e_ref].repType == 's')
  {
    if (globs->bitpos > finalBitPos)
    {
      ccd_recordFault (globs, ERR_CONCAT_LEN, CONTINUE, (USHORT) elem_ref, 
                       globs->pstruct + globs->pstructOffs);
    }
    bf_setBitpos (finalBitPos, globs);
    /* Update maxBitpos to avoid an early end of decoding. */
    globs->maxBitpos = actMaxBitpos;
  }

  /*
   * restore the write pointer
   */
  globs->pstruct   = actStructpos;
  }

  globs->errLabel  = actErrLabel;
  /* Reset indicator of exhaustion in the IEI table*/
  for (i = 0; globs->iei_ctx[globs->ccd_recurs_level].iei_table[i].valid == TRUE; i++)
  {
    globs->iei_ctx[globs->ccd_recurs_level].iei_table[i].exhausted = FALSE;
  }
  globs->ccd_recurs_level--;

  return 1;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+---------------------------------------------------------------------+
| PROJECT : CCD (6144)               MODULE  : CCD                    |
| STATE   : code                     ROUTINE : cdc_csn1_concat_encode |
+---------------------------------------------------------------------+

  PURPOSE : codes the content of a C-Structure into a bitstream.
            This function may be called recursivly if an IE in the
            structure is itself a structured IE.
*/

SHORT cdc_csn1_concat_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)

{
  ULONG cix_ref, elem_ref, last_elem;
  U8  codecRet;
  U16 actBitpos;
  U8  actByteOffs;
  U8     *actStructpos;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_csn1_concat_encode()");
  #else
  TRACE_CCD (globs, "cdc_csn1_concat_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;

  /*
   * If this element is conditional, check the condition.
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  globs->ccd_recurs_level++;

  actStructpos = globs->pstruct;
  globs->pstructOffs = melem[e_ref].structOffs;
  globs->pstruct += globs->pstructOffs;

  elem_ref  = (ULONG) mcomp[melem[e_ref].elemRef].componentRef;
  last_elem = elem_ref + mcomp[melem[e_ref].elemRef].numOfComponents;

  /*
   * It is recommended to use a leading element of coding type NO_CODE
   * in the message description which is used to count the existing 
   * elements of the truncated concatenation in case of decoding. 
   * In case of encoding this element must be skipped.
   */

  if (melem[elem_ref].codingType == CCDTYPE_NO_CODE)
  {
    elem_ref++;
  /*  last_elem = elem_ref + *globs->pstruct;  
      
   * Encoding act on the assumption that all elements of the truncated 
   * concatenation should be encoded. CCD will skip tagged elements
   * but in case of CSN1 coding CCD will write the flag indicating absent
   * elements. Values of mandatory elements without valid flags are coded
   * according to their assignments in the C-structure. 
   * If more bits are written than the component l_buf of the message buffer
   * suggested CCD generates a warning (error code ERR_BUFFER_OF). It is up
   * to the user to analyse the consequences of this warning and to choose 
   * adequate procedures.
   */
  }

  /*
   * code all elements
   */
  while ((elem_ref < last_elem) && (globs->bitpos < globs->msgLen))
  {
#ifdef ERR_TRC_STK_CCD
    /* 
     * Save the value for tracing in error case.
     */
    globs->error_stack[globs->ccd_recurs_level] = (USHORT) elem_ref;
#endif /* ERR_TRC_STK_CCD */

#if defined _TOOLS_
    if (ccd_patch (globs, 0))
      codecRet = 1;
    else
#endif /* _TOOLS_ */

    actBitpos = globs->bitpos;
    actByteOffs  = globs->byteoffs;

    /* Use the jump-table for selecting encode function. */
    codecRet = (UBYTE)
      codec[melem[elem_ref].codingType][ENCODE_FUN](melem[e_ref].elemRef, 
                                                    elem_ref, globs);

    if (globs->bitpos < globs->msgLen)
    {
      if (codecRet NEQ 0x7f)
      {
        /* Set the elem_ref to the next or the same element. */
        elem_ref += codecRet;
      }
    }
    else
    {
      if (globs->bitpos > globs->msgLen)
      {
        globs->bitpos = actBitpos;
        globs->byteoffs  = actByteOffs;
        ccd_recordFault (globs, ERR_CONCAT_LEN, CONTINUE, (USHORT) elem_ref, 
                         globs->pstruct + globs->pstructOffs);
      }
      break;
    }
  }

  globs->pstruct += mcomp[melem[e_ref].elemRef].cSize;
  /*
   * restore the read pointer
   */
  globs->pstruct = actStructpos;
  globs->ccd_recurs_level--;
  return 1;
}
#endif /* !RUN_FLASH */

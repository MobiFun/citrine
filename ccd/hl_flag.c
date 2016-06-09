/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : hl_flag.c
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Inc. 
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
|  Purpose :  Definition of encoding and decoding functions for HL_FLAG elements 
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
| PROJECT : CCD (6144)               MODULE  : CDC_GSM               |
| STATE   : code                     ROUTINE : cdc_hl_flag_decode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type HL_FLAG element. This element
            consists of a single bit only. The decoded value will be 0
            if the encoded value is L respectively 1 if the encoded
            value is H.
            
*/

SHORT cdc_hl_flag_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG repeat, max_rep;
  BOOL  is_variable = FALSE;
  ULONG cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_hl_flag_decode()");
  #else
  TRACE_CCD (globs, "cdc_hl_flag_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  globs->SeekTLVExt = FALSE;
  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;
  repeat     = 1;
  
  if (cix_ref != 0)
  {
    /*
     * if this element is conditional, check the condition
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
    if (melem[e_ref].repType NEQ ' ')
    {
      is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
    }
  }
    /*
     * Element is not a SPARE. Setup the struct pointer
     */
    globs->pstructOffs = melem[e_ref].structOffs;

      
      if (melem[e_ref].optional)
      {
        /*
         * for optional elements we must set the valid-flag
         * ??.
         * Therefore we store the address of the valid flag.
         */
        *(globs->pstruct + globs->pstructOffs++) = TRUE;

      }

  globs->pstruct[globs->pstructOffs++] = (UBYTE) (bf_readBit(globs) == GET_HL_PREV(1));
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "decoding var %s",ccddata_get_alias((USHORT) e_ref, 1));
#else
  TRACE_CCD (globs, "decoding var %d", melem[e_ref].elemRef);
#endif
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_hl_flag_encode     |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type HL_FLAG element. This element
            consists of a single bit only. If the element is set to 1
            a H bit will be coded. Otherwise a L bit will be coded if 
            the element value to encode is set to 0.

*/

SHORT cdc_hl_flag_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  repeat=1, amount=1;
  USHORT cSize = 0, startOffset;
  ULONG  i;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_hl_flag_encode()");
  #else
  TRACE_CCD (globs, "cdc_hl_flag_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  if (cix_ref != 0)
  {
    /*
     * if this element is conditional, check the condition
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
  }

  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE.
     * Setup the offset into the C-structure for this element
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
    {
      /*
       * for optional elements check the valid-flag in the C-struct.
       * Spare elements does not have a corresponding valid flag.
       */
      if (globs->pstruct[globs->pstructOffs++] == FALSE)
      {
        /*
         * element is invalid so we must code a 0 bit
         */
        bf_writeBit (GET_HL(0), globs);

        return 1;
      }
      
      else
      {
#ifdef DEBUG_CCD
        if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
        {
          TRACE_CCD (globs, "Ambiguous value for valid flag!\n...assumed 1 for ccdID=%d",
                     e_ref);
        }
#endif
        /*
         * element is valid so we must code a 1 bit
         */
        bf_writeBit (GET_HL(1), globs);
      }
    }

    if (melem[e_ref].repType NEQ ' ')
    {
      /* As a default amount =1 due to initialization. */
      if (melem[e_ref].repType EQ 'i')
      {
        /*
         * for variable repeatable elements read the amount
         * of repeats out of the C-Structure (c_xxx).
         * If the number of repeats given by the C-Structure 
         * exceeds the allowed value CCD gives a warning!
         */
        if (melem[e_ref].maxRepeat > 255)
        {
          ULONG count = (ULONG) (* (USHORT *)(globs->pstruct + globs->pstructOffs++));
          repeat = MINIMUM (count, (ULONG) melem[e_ref].maxRepeat);
          if (repeat < count) 
            ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                             (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        else
        {
          repeat = (ULONG) MINIMUM (globs->pstruct[globs->pstructOffs], 
                                    melem[e_ref].maxRepeat);
          if ( repeat < (ULONG)(globs->pstruct[globs->pstructOffs]) ) 
            ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                             (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        globs->pstructOffs++;
      }
      else
      if (melem[e_ref].repType EQ 'v')
      {
        /*
         * for variable repeatable elements read the amount
         * of repeats out of the C-Structure (c_xxx).
         * If the number of repetitions given by the C-Structure 
         * exceeds the allowed value (maxRepeat) CCD gives a warning!
         */
        if (melem[e_ref].maxRepeat > 255)
        {
          ULONG count = (ULONG) (* (USHORT *)(globs->pstruct + globs->pstructOffs++));
          amount = MINIMUM (count, (ULONG) melem[e_ref].maxRepeat);
          if (amount < count) 
            ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                             (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        else
        {
          amount = (ULONG) MINIMUM (globs->pstruct[globs->pstructOffs], 
                                    melem[e_ref].maxRepeat);
          if ( amount < (ULONG) (globs->pstruct[globs->pstructOffs]) )
            ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                             (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        globs->pstructOffs++;
      }
      else
      if (melem[e_ref].repType EQ 'c')
      {
        amount = (ULONG) melem[e_ref].maxRepeat;
      }

      if (melem[e_ref].repType EQ 'v' OR melem[e_ref].repType EQ 'i')
      {
        cSize = (USHORT)(((melem[e_ref].elemType EQ 'V')
                         ? mvar[melem[e_ref].elemRef].cSize
                         : mcomp[melem[e_ref].elemRef].cSize
                        ));
        startOffset = (USHORT) globs->pstructOffs;
      }
    }
  
    for (i=0; i < repeat; i++)
    {
      if (cSize)
      {
        /*
         * calculate the offset if it is an array
         */
        globs->pstructOffs = (USHORT)(startOffset + (i * cSize));
      }
      /*
       * encode the value
       */
      if (globs->pstruct[globs->pstructOffs++] EQ FALSE)
      {
        /*
         * element is 0 so we must signalize L
         */
        bf_writeBit (GET_HL(0), globs);
      }
      else
      {
        /*
         * element is 1 so we must signalize H
         */
        bf_writeBit (GET_HL(1), globs);
      }
      
      globs->pstructOffs += mvar[melem[e_ref].elemRef].cSize;
    }

    if (melem[e_ref].repType EQ 'i')
    {
      /*
       * for variable CNS1 fields we code a 0 flag to mark the end of the
       * arrays
       */
      bf_writeBit (GET_HL(0), globs);
    }
  }

  return 1;
}
#endif /* !RUN_INT_RAM */

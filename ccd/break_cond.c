/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : break_cond.c
+----------------------------------------------------------------------------- 
|  Copyright 2004 Texas Instruments Deutschland GmbH
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
|  Purpose :  Definition of encoding and decoding functions for BREAK_COND
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
+-----------------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM                      |
| STATE   : code                       ROUTINE : cdc_break_cond_decode        |
+-----------------------------------------------------------------------------+

  PURPOSE : Decoding of the BREAK_COND element. This element consists of a V 
            component with a variable bit length and must be connected with a 
            special condition. This condition has to be a simple value, which 
            matches to the value range of BREAK_COND element itself.
            This function performs a standard decoding for a given elem table
            entry. This means for non structured elements that 1-n bits are
            read from the bitstream and write to a C-Variable in a machine
            dependent format.
            After decoding of the requested number of bits the resulting value
            will be compared with the constant given by the condition. In case
            of equality the global variable globs->continue_array is set to 
            FALSE. This breaks decoding of the current superior composition 
            and finishes the array. 
*/
SHORT cdc_break_cond_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8     break_ind = FALSE;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_break_cond_decode()");
  #else
  TRACE_CCD (globs, "cdc_break_cond_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  /*
   * if this element has a defined prologue
   * we have to process it before decoding the bitstream
   * If there are some epilogue expressions to be processed for this element
   * (rare cases) the result here will be a reading of 0 to an internal
   * register. The valid processing of expression takes place after the 
   * decoding of the element. 
   */
  if (num_prolog_steps)
  {
    if (calc[prolog_step_ref].operation EQ 'P')
    {
      break_ind = TRUE;
    }
    
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }

  globs->pstructOffs = melem[e_ref].structOffs;


  if (globs->bitpos < globs->maxBitpos)
  {
 
    if (mvar[melem[e_ref].elemRef].cType EQ 'X')
      bf_readBitChunk (mvar[melem[e_ref].elemRef].bSize, globs);
    else
      bf_readBits (mvar[melem[e_ref].elemRef].bSize, globs);
    
    if ( ( break_ind == TRUE ) && (num_prolog_steps > 0))
    {
      if ( calc[prolog_step_ref].operand == 
           (U16) *(globs->pstruct + globs->pstructOffs) )
      {
        globs->continue_array = FALSE;
      }
    }

    globs->pstructOffs += mvar[melem[e_ref].elemRef].cSize;
  }
  else
    ccd_recordFault ( globs, 
                      ERR_ELEM_LEN, 
                      BREAK, 
                      (USHORT) e_ref, 
                      globs->pstruct + globs->pstructOffs);

  /*
   * process the epilogue expression for this element if there is any 
   */
  if (num_prolog_steps)
  {
    if (  (calc[prolog_step_ref+1].operation EQ 'K')
       || (calc[prolog_step_ref+1].operation EQ 'C')
       || (calc[prolog_step_ref+1].operation EQ 's'))
    {
      ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
    }
  }

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+-----------------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM                      |
| STATE   : code                       ROUTINE : cdc_break_cond_encode        |
+-----------------------------------------------------------------------------+

  PURPOSE : encoding of the BREAK_COND element. This element consists of a V 
            component with a variable bit length and must be connected with a 
            special condition. This condition has to be a simple value, which 
            matches to the value range of BREAK_COND element itself.

*/

SHORT cdc_break_cond_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8     break_ind = FALSE;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_break_cond_encode()");
  #else
  TRACE_CCD (globs, "cdc_break_cond_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

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
    if (calc[prolog_step_ref].operation EQ 'P')
    {
      break_ind = TRUE;
    }
    
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }

  /*
   * Element is not a SPARE.
   * Setup the readpointer into the C-structure for this element
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  if (mvar[melem[e_ref].elemRef].cType EQ 'X')
    bf_writeBitChunk (mvar[melem[e_ref].elemRef].bSize, globs);
  else
    bf_writeBits (mvar[melem[e_ref].elemRef].bSize, globs);

  if ( ( break_ind == TRUE ) && (num_prolog_steps > 0))
  {
    if ( calc[prolog_step_ref].operand == 
         (U16) *(globs->pstruct + globs->pstructOffs) )
    {
      globs->continue_array = FALSE;
    }
  }

  globs->pstructOffs += mvar[melem[e_ref].elemRef].cSize;

  return 1;
}
#endif /* !RUN_INT_RAM */

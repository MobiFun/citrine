/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : gsm5_v.c
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
|  Purpose :  Definition of encoding and decoding functions for GSM5_V elements 
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
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm5v_decode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type 5V element. This element
            consists of the not decoded bits of the bitstream.
            In the target C-structure this element is a bitbuffer
            (T_xxx_BUF) in wich the bitstream content are written.

*/

SHORT cdc_gsm5v_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm5v_decode()");
	#else
	TRACE_CCD (globs, "cdc_gsm5v_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif

  if (globs->bitpos >= globs->buflen)
  {
    return 1;
  }
  else
  {
    ULONG  cix_ref;
    
    cix_ref = melem[e_ref].calcIdxRef;

    /*
     * if this element is conditional, check the condition
     */
    if (calcidx[cix_ref].numCondCalcs NEQ 0
        AND ! ccd_conditionOK (e_ref, globs))
    {
      return 1;
    }
    else
    {
      ULONG  bits_to_read;
      ULONG  num_prolog_steps, prolog_step_ref;
      
      num_prolog_steps = calcidx[cix_ref].numPrologSteps;
      prolog_step_ref  = calcidx[cix_ref].prologStepRef;

      /* If there is a prologue given for this element, process it. */
      if (num_prolog_steps)
      {
        ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
      }
      
      /*
       * Setup the offset into the C-structure for this element
       */
      globs->pstructOffs = melem[e_ref].structOffs;

      if (melem[e_ref].optional)
      {  
        /*
         * for optional elements set the valid-flag in the C-struct.
         */
        globs->pstruct[globs->pstructOffs++] = TRUE;
      }

      if (globs->maxBitpos < globs->buflen - 16*globs->numEOCPending)
      {
        ccd_recordFault (globs, ERR_LEN_MISMATCH, CONTINUE, (USHORT) e_ref, 
                         globs->pstruct + globs->pstructOffs);
      }

      bits_to_read = (ULONG)(globs->buflen - globs->bitpos - 16*globs->numEOCPending);  
      bf_readBitChunk (bits_to_read, globs);
    }
  }

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm5v_encode    |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type 5V element. This element
            consists of the not decoded bits of the bitstream.
            In the target C-structure this element is a bitbuffer
            (T_xxx_BUF) from wich the bitstream content is read.

*/

SHORT cdc_gsm5v_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm5v_encode()");
	#else
	TRACE_CCD (globs, "cdc_gsm5v_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  /*
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
      return 1;
    }
#ifdef DEBUG_CCD
    else if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
    {
      TRACE_CCD (globs, "Ambiguous value for valid flag!\n...assumed 1 for ccdID=%d",
                 e_ref);
    }
#endif
  }
 
  bf_writeBitChunk (mvar[melem[e_ref].elemRef].bSize, globs);

  return 1;
}
#endif /* !RUN_INT_RAM */

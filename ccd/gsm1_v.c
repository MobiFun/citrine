/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : gsm1_v.c
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
|  Purpose :  Definition of encoding and decoding functions for GSM1_V elements 
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

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm1v_decode    |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type 1V element. This element
            consists of a V component with max. 4 Bit length.
*/

SHORT cdc_gsm1v_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  SHORT ret; 
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm1v_decode()");
  #else
  TRACE_CCD (globs, "cdc_gsm1v_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * if this element have a defined Prolog
   * we have to process it before decoding the bitstream
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (!globs->Swap1V_inProgress)
  {
    /*
     * check if the next element is a GSM1V too
     */
    if ((ULONG)(mcomp[c_ref].componentRef
        +mcomp[c_ref].numOfComponents) > e_ref
      AND (melem[e_ref].codingType EQ melem[e_ref+1].codingType
           OR melem[e_ref+1].elemType EQ 'S'))
    { 
	    if (melem[e_ref+1].elemType EQ 'S') 
	    {
		    /*
		     * if the next element is a spare then skip the next 4 bits
		     * do not decode the spare bits.
		     */
	      bf_setBitpos ((globs->bitpos+4), globs);

          ret = cdc_std_decode (c_ref, e_ref, globs);

          if (ret EQ 1)
            ret++;

          return ret;
	    }
	    else
	    {
		    /*
		     * another 1V-element follow. We have to swap the nibbles.
		     */
		    globs->Swap1V_inProgress = TRUE;
		    /*
		     * store the akt position
		     */
		    globs->akt1VPos  = (USHORT)(globs->bitpos+4);
		    globs->next1VPos = globs->bitpos;

		    bf_setBitpos (globs->akt1VPos, globs);
        ret = cdc_std_decode (c_ref, e_ref, globs);
        /*
         * increment the globs->maxBitpos by 1 so the bf_endOfBitstream
         * will return FALSE
         */
        globs->maxBitpos++;

        return ret;
      }
    }
    ret = cdc_std_decode (c_ref, e_ref, globs);

  }
  else
  {
    globs->akt1VPos = globs->next1VPos;
    globs->next1VPos = globs->bitpos;

    bf_setBitpos (globs->akt1VPos, globs);
    
    /*
     * decrement the globs->maxBitpos by 1 so the bf_endOfBitstream
     * will return TRUE if the bitstream ended
     */
    globs->maxBitpos--;

    ret = cdc_std_decode (c_ref, e_ref, globs);

    bf_setBitpos (globs->next1VPos, globs);

    globs->Swap1V_inProgress = FALSE;

  }
  return ret;
}
#endif /* !RUN_FLASH */
 
#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_GSM             |
| STATE   : code                       ROUTINE : cdc_gsm1v_encode    |
+--------------------------------------------------------------------+

  PURPOSE : encoding of the GSM Type 1V element. This element
            consists of a V component with max. 4 Bit length.
*/

SHORT cdc_gsm1v_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_gsm1v_encode()");
  #else
  TRACE_CCD (globs, "cdc_gsm1v_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
  #endif
#endif

  if (!globs->Swap1V_inProgress)
  {
    /*
     * check if the next element is a GSM1V too
     */
    if ((ULONG)(mcomp[c_ref].componentRef
        +mcomp[c_ref].numOfComponents) > e_ref
      AND (melem[e_ref].codingType EQ melem[e_ref+1].codingType
           OR melem[e_ref+1].elemType EQ 'S'))
    { 
	    if (melem[e_ref+1].elemType EQ 'S') 
	    {
        SHORT ret; 
		    /*
		     * if the next element is a spare then skip the next 4 bits
		     * do not code the spare bits because the bitstream is cleared.
		     */
	      bf_setBitpos (globs->bitpos+4, globs);

        ret = cdc_std_encode (c_ref, e_ref, globs);

        if (ret EQ 1)
          ret++;

        return ret;
	    }
	    else
      {
        /*
         * another 1V-element follow. We have to swap the nibbles.
         */
        globs->Swap1V_inProgress = TRUE;
        /*
         * store the akt position
         */
        globs->akt1VPos  = (USHORT)(globs->bitpos+4);
        globs->next1VPos = globs->bitpos;

        bf_setBitpos (globs->akt1VPos, globs);
      }
    }
    return cdc_std_encode (c_ref, e_ref, globs);
  }
  else
  {
    SHORT ret;

    globs->akt1VPos = globs->next1VPos;
    globs->next1VPos = globs->bitpos;

    bf_setBitpos (globs->akt1VPos, globs);
    
    ret = cdc_std_encode (c_ref, e_ref, globs);

    bf_setBitpos (globs->next1VPos, globs);

    globs->Swap1V_inProgress = FALSE;

    return ret;
  }
}
#endif /* !RUN_FLASH */

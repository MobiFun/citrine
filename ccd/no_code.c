/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : no_code.c
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
|  Purpose :  Definition of encoding and decoding functions for NO_CODE elements 
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
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_no_decode          |
+--------------------------------------------------------------------+

  PURPOSE : Either reading a value from the stack and writing it into
            the C structure or processing the error branch in case of
            a message escape error label.
            An IE of this type does not occur in the air message.

            EXAMPLE (reading stack value):
            The first usage of this type is the IE "tlv_len" in a 
            Multi Rate Configuration. In this case "tlv_len" is to be
            used for evaluating conditions which decide for the content
            of Multi Rate Configuration IE.

            EXMAPLE (message escape):
            A part of a message, which depends on a certain protocol
            status, is marked by the 'Message escape' error label. It
            is preceeded by an amount of bits given by the specification.
            Some of these bit combinations are concatenated with a
            well-defined message structure. All the rest of combinations
            are expected to provide an escape
            -> use coding type 'NO_CODE'.
*/

SHORT cdc_no_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U16  cixRef = melem[e_ref].calcIdxRef;

#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_no_decode()");
  #endif
#endif

  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cixRef].numCondCalcs != 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  if (calcidx[cixRef].numPrologSteps)
  {
    switch (calc[calcidx[cixRef].prologStepRef].operation)
    {
        case 'Z': /* address information part error */
            globs->errLabel = ERR_ADDR_INFO_PART;
          break;
          
        case 'D': /* distribution part error */
            globs->errLabel = ERR_DISTRIB_PART;
          break;
          
        case 'N': /* non distribution part error */
            globs->errLabel = ERR_NON_DISTRIB_PART;
          break;
          
        case 'M': /* message escape */
            globs->errLabel = ERR_MESSAGE_ESCAPE;
          break;
          
        default:
          break;
    }
  }

  if (globs->errLabel)
  {
#ifdef DEBUG_CCD
	#ifdef CCD_SYMBOLS
	  TRACE_CCD (globs, "cdc_no_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif
    ccd_setError (globs, globs->errLabel, BREAK, globs->bitpos, (USHORT) -1);
  }
  
  else
  {
#ifdef DEBUG_CCD
	#ifdef CCD_SYMBOLS
  if (calcidx[cixRef].numPrologSteps == 0) 
    TRACE_CCD (globs, "writing 2 bytes (%ld) to struct", globs->KeepReg[0]);
  else
	  TRACE_CCD (globs, "cdc_no_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));

	#endif
#endif

    globs->pstructOffs = melem[e_ref].structOffs;
    *(U16*) (globs->pstruct + globs->pstructOffs) = (U16) globs->KeepReg[0];
  }      

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_GSM                |
| STATE   : code                    ROUTINE : cdc_no_encode          |
+--------------------------------------------------------------------+

  PURPOSE : An IE of this type does not occure in the air message.
            Nevertheless the variable in the C structure must be 
            written by the caller entity.
            EXAMPLE:
            The first usage of this type is the IE "tlv_len" in a 
            Multi Rate Configuration. In this case "tlv_len" is to be used
            for evaluating conditions which decide for the content of
            Multi Rate Configuration IE.

*/

SHORT cdc_no_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
#ifdef DEBUG_CCD
	#ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_no_encode()");
	#else
	TRACE_CCD (globs, "cdc_no_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
	#endif
#endif
  return 1;
}
#endif /* !RUN_INT_RAM */

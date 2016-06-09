/* 
+----------------------------------------------------------------------------- 
|  Project : CCD  
|  Modul   : fdd_ci.c
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
|  Purpose :  Definition of encoding and decoding functions for FDD_CI type
+----------------------------------------------------------------------------- 
*/ 


/*
 * standard definitions like GLOBAL, UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "ccd_globs.h"
/*
 * Function prototypes of CCD-CCDDATA interface 
 */
#include "ccddata.h"


/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"
#include "bitfun.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"

#if !(defined (CCD_TEST))
#include "vsi.h"
#endif

#ifndef RUN_INT_RAM
/* Attention: static data, only used in cdc_fdd_ci_decode */
static const U8 params_bSize[17] =
{
  0,
  10,
  19, 28,
  36, 44, 52, 60,
  67, 74, 81, 88, 95, 102, 109, 116,
  122
};
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                    MODULE  : cdc_fdd_ci_decode       |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the FDD_CELL_INFORMATION Field reusing 
            RANGE 1024 format of frequency list information.
            The IE is preceded by FDD_Indic0(1 bit) and made of the
            following two IEs:
            1) NR_OF_FDD_CELLS(5 bit field),
            2) FDD_CELL_INFORMATION information parameters
            
            FDD_Indic0 indicates if the parameter value '0000000000' 
            is a member of the set.

            FDD_CELL_INFORMATION or "Scrambling Codes and Diversity 
            Field" is a bit filed of length
                   p(Number_of_Scrambling_Codes_and_Diversity),
            whereas the function p(x) is defined by the table below
            with n = Number_of_Scrambling_Codes_and_Diversity. 
            n p   n p    n p    n    p
            0 0   5 44  10 81  15    116
            1 10  6 52  11 88  16    122
            2 19  7 60  12 95  17-31 0
            3 28  8 67  13 102
            4 36  9 74  14 109

            The message is sent from net to MS and a MS supporting 
            enhanced measurements has to understand it.
            
            The space this IE takes in the C-structure is made of a
            counter for the number of decoded parameter and an array
            of them.
*/

SHORT cdc_fdd_ci_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8   ListSize   = 0;
  U16  ListBitLen = 0;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  short  *w;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_fdd_ci_decode()");
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "decoding list %s with range 1024 format",
                       ccddata_get_alias((USHORT) e_ref, 1));
#else
  TRACE_CCD (globs, "decoding elem %d with range 1024 format", melem[e_ref].elemRef);
#endif
#endif

  globs->SeekTLVExt  = FALSE;

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
  
  /*
   * First read NR_OF_FDD_CELLS (5 bits).
   */
  globs->pstructOffs = melem[e_ref].structOffs;;
  bf_readBits (5, globs);
  ListSize = globs->pstruct[globs->pstructOffs++];

  /* If n=0 there is nothing to do for this IE. */
  if (!ListSize)
  {
    return 1;
  }

  /* Read the corresponding bit number or suppose the maximum length. */
  if (ListSize <= 16)
  {
    ListBitLen = params_bSize [ListSize];
  }
  else
  {
    /* If n>17 there is nothing to do for this IE. */
    return 1;
  }
  /*
   * Bit size for params is bigger than the size of unread bits in the 
   * message buffer. Danger: buffer overwriting!
   */
  if ( ListBitLen > globs->maxBitpos - globs->bitpos)
  {
    ccd_recordFault (globs, ERR_ELEM_LEN, BREAK, (USHORT) e_ref, 
                     globs->pstruct + globs->pstructOffs);
    ListBitLen = (U16)(globs->maxBitpos - globs->bitpos);
  }

  /*
   * Use dynamic memory for calculation instead of global memory or stack.
   */
  MALLOC (w, 257 * sizeof (U16));

  /* Decode the W-parameter. */
  cdc_decode_param (param_1024, w, ListBitLen, globs);

  /*
   * Decode and set the remaining channel number according the
   * algorithm described in GSM 4.08.
   */
  cdc_decode_frequencies (1023, &w[0], 0, FDD_CI_LIST, globs);

  /* Free the dynamic allocated memory. */
  MFREE (w);

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                   MODULE  : cdc_fdd_ci_encode        |
+--------------------------------------------------------------------+

  PURPOSE : Encoding function is not needed, since this message is 
            sent only from net to MS.
            It could be only useful for testing procedure if there
            were an encoder function at this place. 
            This will be a future work.  
*/

SHORT cdc_fdd_ci_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_fdd_ci_encode()");
#endif
#ifdef TARGET_WIN32
  /* TBD */
#endif
  return 1;
}
#endif /* !RUN_INT_RAM */

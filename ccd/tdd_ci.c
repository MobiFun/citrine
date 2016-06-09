/* 
+----------------------------------------------------------------------------- 
|  Project : CCD  
|  Modul   : tdd_ci.c
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
|  Purpose :  Definition of encoding and decoding functions for TDD_CI type
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
 * Prototypes of ccd internal functions
 */
#include "ccd.h"
#include "bitfun.h"

/*
 * Declaration of coder/decoder tables
 */
#include "ccdtable.h"
/*
 * Function prototypes of CCD-CCDDATA interface 
 */
#include "ccddata.h"


#if !(defined (CCD_TEST))
#include "vsi.h"
#endif

#ifndef RUN_INT_RAM
/* Attention: static data, only used in cdc_tdd_ci_decode */
static const U8 params_bSize[21] =
{
  0,
  9,
  17, 25,
  32, 39, 46, 53,
  59, 65, 71, 77, 83, 89, 95, 101,
  106, 111, 116, 121, 126
};
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                    MODULE  : cdc_tdd_ci_decode       |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the TDD_CELL_INFORMATION Field reusing 
            RANGE 511 format of frequency lists (with w0=0.).
            This field allows to compute a set of 9-bit-long 
            TDD_CELL_INFORMATION aprameters.
            The IE is preceded by TDD_Indic0(1 bit) and made of the
            following two IEs:
            1) NR_OF_TDD_CELLS(5 bit field),
            2) TDD_CELL_INFORMATION information parameters
            
            TDD_Indic0 indicates if the parameter value '0000000000' 
            is a member of the set.
            The total number of bits q of this field depends on the 
            value of the parameter NR_OF_TDD_CELLS = m 
            as follows (with q=0 if m=0):
            m q   m q    m q    m q     m    q
            0 0   5 39  10 71  15 101  20    126
            1 9   6 46  11 77  16 106  21-31 0
            2 17  7 53  12 83  17 111
            3 25  8 59  13 89  18 116
            4 32  9 65  14 95  19 121

            The message is sent from net to MS and a MS supporting 
            enhanced measurements has to understand it.
            
            The space this IE takes in the C-structure is made of a
            counter for the number of decoded parameter and an array
            of them.
*/

SHORT cdc_tdd_ci_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8   ListSize   = 0;
  U16  ListBitLen = 0;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  short *w;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_tdd_ci_decode()");
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "decoding list %s with range 512 format",
                       ccddata_get_alias((USHORT) e_ref, 1));
#else
  TRACE_CCD (globs, "decoding list %d of range 512 format", melem[e_ref].elemRef);
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
   * First read NR_OF_TDD_CELLS (5 bits).
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
  if (ListSize <= 20)
  {
    ListBitLen = params_bSize [ListSize];
  }
  else
  {
    /* If n>20 there is nothing to do for this IE. */
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


  /*
   * Decode the W-parameter.
   * As a rule for this type w(0) must be 0. 
   */
  w[0] = 0;
  cdc_decode_param (param_512+1, &w[1], ListBitLen, globs);

  /*
   * Decode and set the remaining channel number according the
   * algorithm described in GSM 4.08.
   */
  cdc_decode_frequencies (511, &w[1], 0, TDD_CI_LIST,  globs);
  
  /* Free the dynamic allocated memory. */
  MFREE (w);

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                   MODULE  : cdc_tdd_ci_encode        |
+--------------------------------------------------------------------+

  PURPOSE : Encoding function is not needed, since this message is 
            sent from net to MS.
            It could be only useful for testing procedure if there
            were an encoder function at this place. 
            This will be a future work.

*/

SHORT cdc_tdd_ci_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_tdd_ci_encode()");
#endif
#ifdef TARGET_WIN32
  /* TBD */
#endif
  return 1;
}
#endif /* !RUN_INT_RAM */

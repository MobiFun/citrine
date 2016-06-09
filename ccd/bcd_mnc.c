/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : bcd_mnc.c
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
|  Purpose :  Definition of encoding and decoding functions for BCD_MNC elements 
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
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : cdc_bcd_mnc_decode  |
+--------------------------------------------------------------------+

  PURPOSE : decoding a byte array, that contains a Mobile Network Code,
            from the bitstream:
            
            MSBit     LSBit   
            7 8 6 5 4 3 2 1
            DIGIT_3 XXXXXXX    Octett n-1
            DIGIT_2 DIGIT_1    Octett n

            The current decoding position is expected after Octett n-1
            The byte array should be of dimension [2..3] (preferred)
            or [3] or [2] (also supported)

*/


SHORT cdc_bcd_mnc_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               repeat, max_rep;
  BOOL                is_variable;
  UBYTE               digBuffer[3];
  UBYTE              *addr_c_xxx;
  ULONG               i;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;
  register UBYTE     *digits;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_bcd_mnc_decode()");
  #else
  TRACE_CCD (globs, "cdc_bcd_mnc_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
   * if this element is repeatable, and the number of
   * repeats depends on another element, calculate the repeater
   */
  if (melem[e_ref].repType NEQ ' ')
  {
    is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
  }
  else
  {
    repeat = 1;
    is_variable = FALSE;
  }

  /*
   * setup the offset into the C-structure for this element
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  if (melem[e_ref].optional)
  {
    /*
     * for optional elements set the valid-flag
     */
    globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
  }

  if (is_variable)
  {
    if (max_rep < 2 OR max_rep > 3)
    {
      ccd_setError (globs, ERR_INVALID_TYPE, BREAK, (USHORT) (globs->bitpos),
		    (USHORT) -1);
    }
    /*
     * for variable sized elements store the min-value
     * as counter into the C-Structure (c_xxx).
     */
    addr_c_xxx = (UBYTE *) (globs->pstruct + globs->pstructOffs++);
    if (max_rep > 255)
      globs->pstructOffs++;
  }
  else
    addr_c_xxx = NULL;

  digits = (UBYTE *) (globs->pstruct + globs->pstructOffs);
  
  bf_setBitpos ((globs->bitpos - 8), globs);
  
  /*
   * read the BCD digits out of the bitstream.
   * The read order is 3,X,2,1
   */
  digBuffer[2] = bf_decodeByteNumber (4, globs);
  bf_incBitpos (4, globs);

  digBuffer[1] = bf_decodeByteNumber (4, globs);
  digBuffer[0] = bf_decodeByteNumber (4, globs);

  if (addr_c_xxx NEQ NULL)
  {
    /*
     * store the number of digits into the 
     * c_xxx variable if there is one.
     */
    repeat = (ULONG) ((digBuffer[2] EQ 0x0f) ? 2 : 3);
    if (max_rep > 65535)
    {
      ULONG *addr_c_xxx_u32;
      addr_c_xxx_u32 = (ULONG *)addr_c_xxx;
      *addr_c_xxx_u32 = repeat;
    }
    else if (max_rep > 255)
    {
      USHORT *addr_c_xxx_u16;
      addr_c_xxx_u16 = (USHORT *)addr_c_xxx;
      *addr_c_xxx_u16 = (USHORT) repeat;
    }
    else
      *addr_c_xxx = (UBYTE) repeat;
  }
  else
  {
    if (max_rep EQ 2 AND digBuffer[2] NEQ 0xf)
      ccd_setError (globs, ERR_PATTERN_MISMATCH,
              CONTINUE,
              (USHORT) (globs->bitpos-16),
              (USHORT) -1);

    repeat = max_rep;
  }
  /*
   * store the digits into the C-Structure variable
   */
  for (i=0; i<repeat; i++)
    digits[i] = digBuffer[i];

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : cdc_bcd_mnc_encode  |
+--------------------------------------------------------------------+

  PURPOSE : encoding a byte array, that contains a Mobile Network Code,
            into the bitstream:
            
            MSBit     LSBit   
            7 8 6 5 4 3 2 1
            DIGIT_3 XXXXXXX    Octett n-1
            DIGIT_2 DIGIT_1    Octett n

            The current coding position is expected after Octett n-1 
*/


SHORT cdc_bcd_mnc_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               repeat;
  UBYTE               dig3;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;
  register UBYTE *digits;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_bcd_mnc_encode()");
  #else
  TRACE_CCD (globs, "cdc_bcd_mnc_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
   * setup the offset into the C-structure for this element
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  if (melem[e_ref].optional)
  {
    /*
     * for optional elements check the valid-flag
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

  /* 
   * if this element is repeatable, and the number of
   * repeats depends on another element, calculate the repeater
   */
  if (melem[e_ref].repType EQ 'v' OR melem[e_ref].repType EQ 'i')
  {
    /*
     * for variable sized elements read the amount
     * of repeats out of the C-Structure (c_xxx).
     * If the number of repeats given by the C-Structure 
     * exceeds the allowed value (max_repeat) CCD gives a warning!
     */
    if (melem[e_ref].maxRepeat > 255)
    {
      ULONG count = (ULONG) (* (USHORT *)(globs->pstruct + globs->pstructOffs++));
      repeat = MINIMUM (count, (ULONG)melem[e_ref].maxRepeat);
      if (repeat < count) 
        ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                         (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
    }
    else
    {
      repeat = (ULONG) MINIMUM (globs->pstruct[globs->pstructOffs], 
                                melem[e_ref].maxRepeat);
      if ( repeat < (ULONG) (globs->pstruct[globs->pstructOffs]) )
        ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                         (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
    }

    globs->pstructOffs++;
  }
  else
    if (melem[e_ref].repType EQ 'c')
      repeat = (ULONG) melem[e_ref].maxRepeat;
    else
      repeat = 1; 

  /* 
   * setup the read pointer to the byte array that contain
   * the BCD number.
   */
  digits = (UBYTE *) (globs->pstruct + globs->pstructOffs);

  if (repeat EQ 2)
    dig3 = 0x0f;
  else if (repeat EQ 3)
    dig3 = digits[2];
  else
  {
    ccd_setError (globs, ERR_INVALID_TYPE,
              BREAK,
              (USHORT) (globs->bitpos),
              (USHORT) -1);
    return 1;
  }

  bf_setBitpos ((globs->bitpos-8), globs);
  bf_codeByteNumber (4, dig3, globs);
  bf_incBitpos (4, globs);
  bf_codeByteNumber (4, digits[1], globs);
  bf_codeByteNumber (4, digits[0], globs);
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "skipping back 8 bits");
  TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) dig3);
  TRACE_CCD (globs, "skipping 4 bits");
  TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) digits[1]);
  TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) digits[0]);
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

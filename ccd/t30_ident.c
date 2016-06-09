/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : t30_ident.c
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
|  Purpose :  Definition of encoding and decoding functions for T30_IDENT elements 
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
| PROJECT : CCD (6144)                MODULE  : CCD                  |
| STATE   : code                      ROUTINE : cdc_t30_ident_decode |
+--------------------------------------------------------------------+

  PURPOSE :

*/

SHORT cdc_t30_ident_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               max_rep;
  BOOL                is_variable;
  UBYTE               digBuffer[30], bit, digT30, digASCII;
  UBYTE               *addr_c_xxx;
  int                 i, repeat;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;
  register UBYTE      *ident;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_t30_ident_decode()");
  #else
  TRACE_CCD (globs, "cdc_t30_ident_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
    ULONG rep; 
    is_variable = ccd_calculateRep (e_ref, &rep, &max_rep, globs);
    repeat = rep;
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

  /*
   * calculate the address of the Most Significant Digit 
   * of the T30_IDENT element in the C-struct
   */
  ident = (UBYTE *) (globs->pstruct + globs->pstructOffs);

  
  /*
   * now read 'repeat' T30_IDENT digits and convert them
   * into ASCII.
   */
  i=0;

  while (!bf_endOfBitstream(globs) AND i<repeat)
  {
    digT30 = bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "T30 digit (%X) read", digT30);
#endif
    /*
     * conversion T30->ASCII
     * reverse the bitorder of each byte
     */
    digASCII = 0;

    for (bit = 0; bit < 8; bit++)
    {
      digASCII <<= 1;
      digASCII |= (digT30 & 0x01);
      digT30   >>= 1;
    }

    digBuffer[i] = digASCII;
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "  converted to %X = %c", digBuffer[i], digBuffer[i]);
#endif
    i++;
  }

  /*
   * eleminate leading spaces
   */
  while (i > 0 AND digBuffer[i-1] EQ ' ')
  {
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "eliminating leading space");
#endif
    i--;
  }

  repeat = i;


  if (addr_c_xxx NEQ NULL)
  {
    /*
     * store the number of digits into the 
     * c_xxx variable if there is one.
     */
    if (max_rep > 65535)
    {
      ULONG *addr_c_xxx_u32;
      addr_c_xxx_u32 = (ULONG *)addr_c_xxx;
      *addr_c_xxx_u32 = (ULONG) repeat;
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "storing %d into counter var at (%lx)",
               repeat, addr_c_xxx_u32);
#endif
    }
    else if (max_rep > 255)
    {
      USHORT *addr_c_xxx_u16;
      addr_c_xxx_u16 = (USHORT *)addr_c_xxx;
      *addr_c_xxx_u16 = (USHORT) repeat;
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "storing %d into counter var at (%lx)",
               repeat, addr_c_xxx_u16);
#endif
    }
    else
    {
      *addr_c_xxx = (UBYTE) repeat;
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "storing %d into counter var at (%lx)",
               repeat,
               addr_c_xxx);
#endif
    }
  }

  /*
   * store the digits in reverse order
   * into the C-Structure variable
   */
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "storing %d digits into cstruct at (%lx)",
             repeat,
             ident);
#endif

  for (i=0; i<repeat; i++)
    ident[i] = digBuffer[(repeat-1)-i];

  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                MODULE  : CCD                  |
| STATE   : code                      ROUTINE : cdc_t30_ident_encode |
+--------------------------------------------------------------------+

  PURPOSE : 

*/


SHORT cdc_t30_ident_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  ULONG               repeat, max_rep;
  UBYTE               digBuffer[30], bit, digT30, digASCII;
  ULONG               i;
  ULONG               cix_ref, num_prolog_steps, prolog_step_ref;
  register UBYTE     *ident;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_t30_ident_encode()");
  #else
  TRACE_CCD (globs, "cdc_t30_ident_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
   * we have to process it before encoding
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
     * exceeds the allowed value (maxRepeat) CCD gives a warning!
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
      if ( repeat < (ULONG) (globs->pstruct[globs->pstructOffs]) )
        ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, 
                         (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
    }
    globs->pstructOffs++;
    max_rep = (ULONG) melem[e_ref].maxRepeat;
  }
  else
    if (melem[e_ref].repType EQ 'c')
    {
      repeat = (ULONG) melem[e_ref].maxRepeat;
      max_rep = (ULONG) melem[e_ref].maxRepeat;
    }
    else
    {
      repeat = 1; 
      max_rep = 1;
    }

  /* 
   * setup the read pointer to the byte array that contain
   * the ident number.
   */
  ident = (UBYTE *) (globs->pstruct + globs->pstructOffs);

  /*
   * read the digits in reverse order out of the C-Structure variable.
   * (filled up with blanks to the maxRepeat)
   */

  i=0;

  while (i < max_rep)
  {
    if (i < repeat)
      digBuffer[i] = ident[(repeat-1)-i];
    else
    {
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "appending space char");
#endif
      digBuffer[i] = ' ';
    }
    i++;
  }

  /*
   * now read 'repeat' T30_IDENT digits and convert them
   * into ASCII.
   */
  i=0;

  while (i < max_rep)
  {
    digASCII = digBuffer[i];

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "ASCII digit (%X) = %c ", (USHORT) digASCII, digASCII);
#endif
    /*
     * conversion ASCII->T30
     * reverse the bitorder of each byte
     */
    digT30 = 0;

    for (bit = 0; bit < 8; bit++)
    {
      digT30   <<= 1;
      digT30   |= (digASCII & 0x01);
      digASCII >>= 1;
    }

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "  converted to %X", (USHORT) digT30);
#endif
    bf_codeByteNumber (8, digT30, globs);

    i++;
  }
  
  return 1;
}
#endif /* !RUN_INT_RAM */

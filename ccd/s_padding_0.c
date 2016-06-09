/* 
+----------------------------------------------------------------------------- 
|  Project : CCD
|  Modul   : s_padding_0.c
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
|  Purpose :  Definition of encoding and decoding functions for S_PADDING_0 
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
| PROJECT : CCD                      MODULE  : s_padding_0           |
| STATE   : code                     ROUTINE : cdc_padd_0_decode     |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the GSM Type CSN1 spare padding which is
            preceded by a 0 bit.
            If that bit is read as 1 then a protocol extension is
            assumed by CCD.
            This function does not evaluate the encoded bits, since
            their content is irrelevant.
*/

SHORT cdc_padd_0_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U16  cixRef;
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_padd_0_decode()");
#endif

  if (bf_readBit(globs) == 1)
  {
    ccd_recordFault (globs, ERR_PROTOCOL_EXTENSION, CONTINUE, (USHORT) e_ref, NULL);
  }
   
  /*
   * Do not decode padding bits. They are not relevant.
   * Just adjust the position pointer in the bit stream buffer.
   * Either to the next octet boundary or to the message end, if necessary.
   */
  bf_incBitpos (8-(globs->bitpos & 7), globs);

  /* First assume padding bits up to an octet boundary. In this case
   * message extension could be made of T, TV or TLV types.
   */
  globs->SeekTLVExt = TRUE;

  cixRef = melem[e_ref].calcIdxRef;
  if (calcidx[cixRef].numPrologSteps > 0)
  {
    U16 msgEnd = (USHORT) calc[calcidx[cixRef].prologStepRef].operand * 8;
    if (msgEnd)
    {
      msgEnd += globs->bitoffs;
      msgEnd = (USHORT)MINIMUM(globs->maxBitpos, msgEnd);
      bf_setBitpos (msgEnd, globs);
      /*
       * Padding bytes exclude the presence of message extension.
       */
      globs->SeekTLVExt = FALSE;
    }
  }
  return 1;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                     MODULE  : s_padding_0.c          |
| STATE   : code                    ROUTINE : cdc_padd_0_encode      |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of the GSM Type CSN1 spare padding which is
            preceded by a 0 bit.
            Supported padding values are 0x00 and 0x2B.
            If the first prologue step is a value msg_len, padding 
            is done until globs->bitpos is msg_len*8.
            Otherwise until the octet boundary.
*/

SHORT cdc_padd_0_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8   padd_bit = 0;
  U16  cixRef;
  U16  msgLen = globs->buflen;
  BOOL paddingOctets = FALSE;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_padd_0_encode()");
#endif

  cixRef = melem[e_ref].calcIdxRef;

  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cixRef].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  if (calcidx[cixRef].numPrologSteps > 0)
  {
    if (calc[calcidx[cixRef].prologStepRef].operation NEQ 'P')
      ccd_setError (globs, ERR_INTERNAL_ERROR, BREAK, (USHORT) -1);
    else
    {
      msgLen = (USHORT)(calc[calcidx[cixRef].prologStepRef].operand * 8);
      paddingOctets = TRUE;
    }
  }

  /*
   * If there is some space left for spare padding, we will code them.
   * If the bit pos pointer goes beyond the message border, ccd will detect
   * this later and bring a warning. But not here. Reason: saving room.
   */
  if (globs->bitpos - globs->bitoffs < msgLen )
  {
    /*
     * The IE should be present in the message so we code 0 bit.
     */
    bf_writeBit (0, globs);

    if (melem[e_ref].elemType EQ 'S' AND spare[melem[e_ref].elemRef].bSize EQ 8)
    {
      ULONG spareVal = spare[melem[e_ref].elemRef].value;
      /*
       * Element is a SPARE of length 8.
       */
      while (globs->bitpos % 8 NEQ 0)
      {
        switch(spareVal)
        {
          case 0:
            break;
          case 0x2B:
            padd_bit = (UBYTE)GET_HL(0);
            break;
          default:
            ccd_setError (globs, ERR_INTERNAL_ERROR,
                          BREAK,
                          (USHORT) (globs->bitpos),
                          (USHORT) -1);
        }
        bf_writeBit (padd_bit, globs);
      }

      /* Write spare padding octets. */
      if (paddingOctets)
      {
        while (globs->bitpos - globs->bitoffs < msgLen )
        {
          bf_codeLongNumber (8, spareVal, globs);
        }
      }
    }

    else
    {
      ccd_setError (globs, ERR_INVALID_TYPE,
                    BREAK,
                    (USHORT) (globs->bitpos),
                    (USHORT) -1);
    }
  } 

  return 1;
}
#endif /* !RUN_FLASH */

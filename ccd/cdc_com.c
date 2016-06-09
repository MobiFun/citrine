/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : cdc_com.c
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
|  Purpose :  Condat Conder Decoder
|             Definitions of common functions for encoding and decoding of
|             GSM, GPRS or UMTS air interface messages
+----------------------------------------------------------------------------- 
*/ 

#ifdef _MSDOS
#include <dos.h>
#include <conio.h>
#endif

/*
 * standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"
#include <string.h>
#include <stdlib.h>

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
 * Declaration of coder/decoder-tables
 */
#include "ccdtable.h"
#include "ccddata.h"

/*
 * Prototypes and constants in the common part of ccd
 */
#include "ccd.h"
#include "ccd_codingtypes.h"

/*
 * Need memory allocation functions for dynamic arrays (pointers)
 */
#ifdef DYNAMIC_ARRAYS
#include "vsi.h"
#endif

#ifndef RUN_FLASH
const UBYTE padding_bits[8]      =    {0, 0, 1, 0, 1, 0, 1, 1};
const UBYTE padding_bits_prev[8] =    {1, 0, 0, 1, 0, 1, 0, 1};
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static data (used in cdc_skipElem) */
static UBYTE  dummy[256];
#endif /* !RUN_FLASH */

typedef struct unknownTag
{
 U8  errCode;
 U8  tag;
 U16 bitpos;
 struct unknownTag *next;
}T_UNKNOWN_TAG;


#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_init_ctx_table  |
+--------------------------------------------------------------------+

  PURPOSE : init the iei_ctx table. This must be done before decoding
            a message.

*/

static void cdc_init_ctx_table (T_CCD_Globs *globs)
{
  int i;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "CONTEXT table init");
#endif

  for (i=0; i<MAX_RECURSIONS_PER_MSG; i++)
  {
    globs->iei_ctx[i].valid = FALSE;
  }
  globs->numEOCPending = 0;
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : cdc_BCD_decode      |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of an bitstream containing a BCD string
            with k digits. If the first digit in the bitstream
            is DIGIT_2, the digits are ordered
            in the Bitstream as follow:

            MSBit     LSBit
            8 7 6 5 4 3 2 1
            DIGIT_2 DIGIT_1    Octett n
            DIGIT_4 DIGIT_3    Octett n+1
            : : : : : : : :
            DIGIT_Z DIGIT_X    Octett n+m

            if the number of the digits is odd, the last
            Octett contains the bit pattern 1111 in the
            most significant nibble.

            : : : : : : : :
            1 1 1 1 DIGIT_X    Octett n+m

  NOTE:     If the first digit in the bitstream is DIGIT_1,
            the digits are ordered in a different way:

            MSBit     LSBit
            8 7 6 5 4 3 2 1
            DIGIT_1 XXXXXXX    Octett n
            DIGIT_3 DIGIT_2    Octett n+1
            DIGIT_5 DIGIT_4    Octett n+2
            : : : : : : : :
            DIGIT_Z DIGIT_X    Octett n+m

            In this case, if the number of the digits
            is even, the last octett contains the bit
            pattern 1111 in the most significant nibble.

            : : : : : : : :
            1 1 1 1 DIGIT_X    Octett n+m

            The amount of digits may be constant or variable.

  NOTE:     A special case (type BCD_NOFILL) is the encoding and
            decoding of a BCD string starting with DIGIT_2 but
            without setting/checking the most significant nibble
            of Octet n+m. This nibble belongs to the next IE 
            (usual coded by type BCD_MNC). 
            Type BCD_NOFILL is coded by startDigit EQ 2.  
*/

void cdc_BCD_decode (const ULONG e_ref, UBYTE startDigit, T_CCD_Globs *globs)
{
  BOOL    is_variable;
  UBYTE  *digits;
  UBYTE  *addr_c_xxx;
  ULONG   i, max_rep, nibbles_to_read, repeat;
  ULONG   cix_ref, num_prolog_steps, prolog_step_ref;
  int     k;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_BCD_decode()");
	#else
	TRACE_CCD (globs, "cdc_BCD_decode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
    return;

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


  if (startDigit EQ 1)
  {
    /*
     * read the BCD digits out of the bitstream.
     * The read order is 1,X,3,2,5,4 ...
     */
    /*
     * if the first digit is digit_1 read it and skip
     * the next 4 bits, because they do not belong to
     * the BCD stream.
     */
    digits[0] = bf_decodeByteNumber (4, globs);
    bf_incBitpos (4, globs);
    /*
     * make a correction on the repeatvalue
     */
    if (melem[e_ref].repType NEQ ' ')
    {
      is_variable = ccd_calculateRep (e_ref, &repeat, &max_rep, globs);
    }

    k = 2;

    for (i=0; i<repeat; i++)
    {
      digits[k] = bf_decodeByteNumber (4, globs);
  #ifdef DEBUG_CCD
      TRACE_CCD (globs, "BCD digit (%X) read", (USHORT) digits[k]);
  #endif
      k = ((k&1) EQ 0) ? (k-1) : (k+3);
    }
  }
  else
  {
    /*
     * read the BCD digits out of the bitstream.
     * The read order is 2,1,4,3,6,5 ...
     */
    k = 1;

    nibbles_to_read = repeat;

    if (repeat & 1)
      nibbles_to_read++;

    for (i=0; i<nibbles_to_read; i++)
    {
      digits[k] = bf_decodeByteNumber (4, globs);
  #ifdef DEBUG_CCD
      TRACE_CCD (globs, "BCD digit (%X) read", (USHORT) digits[k]);
  #endif
      k = ((k&1) EQ 1) ? (k-1) : (k+3);
    }
  }

  /*
   * check the 1111 pattern and the even odd criteria
   */

  if (startDigit EQ 1)
  {
    if ((repeat & 1) EQ 0) 
    {
      /* even number of digits */
      if (digits[repeat] NEQ 0xf)
        repeat++;
    }
  }
  else
  {
    if ((repeat & 1) EQ 1) 
    {
      /* odd number of digits */
      if (digits[repeat] NEQ 0xf AND startDigit EQ 0)
      {
        /*
         * if there is no 1111 pattern generate an error
         */
        ccd_setError (globs, ERR_PATTERN_MISMATCH,
                      CONTINUE,
                      (USHORT) (globs->bitpos-8), (USHORT) -1);
      }
    }

    else
    {
      /* even number of digits - the last may be 0xf */
      if (digits[repeat-1] EQ 0xf)
        repeat--;   /* 0x0f dosn't belong to the coded digit string */
    }
  }

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
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : cdc_BCD_encode      |
+--------------------------------------------------------------------+

  PURPOSE : encoding a Bytearray, that contain a BCD Number, into
            bitstream.
            The digits coded in the following order
            into the Bitstream:

            MSBit     LSBit   
            7 8 6 5 4 3 2 1
            DIGIT_2 DIGIT_1    Octett n
            DIGIT_4 DIGIT_3    Octett n+1
            : : : : : : : :
            DIGIT_Z DIGIT_X    Octett n+m
            
            if the number of the digits are odd, the bit pattern 1111
            will be coded in the most significant nibble of
            the last Octett.
            
            : : : : : : : :
            1 1 1 1 DIGIT_X    Octett n+m

            The amount of digits may be constant or variable.
*/
void cdc_BCD_encode (const ULONG e_ref, UBYTE startDigit, T_CCD_Globs *globs)
{
  ULONG           repeat;
  int             k;
  register UBYTE *digits;
  BOOL            fullBitsNeeded=FALSE;
  ULONG           cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  #ifndef CCD_SYMBOLS
  TRACE_CCD (globs, "cdc_BCD_encode()");
	#else
	TRACE_CCD (globs, "cdc_BCD_encode() %s", ccddata_get_alias((USHORT) e_ref, 1));
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
    return;

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
      return;
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
                          e_ref, globs->pstruct + globs->pstructOffs);
    }
    else
    {
      repeat = (ULONG)MINIMUM (globs->pstruct[globs->pstructOffs], 
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

  /* There seems to exist cases where address contains no digits. */
  if (repeat EQ 0)
    return;

  /* 
   * setup the read pointer to the byte array that contain
   * the BCD number.
   */
  digits = (UBYTE *) (globs->pstruct + globs->pstructOffs);

  if (startDigit EQ 1)
  {
    /*
     * write the BCD digits into the bitstream.
     * The write order is 1,X,3,2,5,4 ...
     */
    if ((repeat & 1) EQ 0)
    {
      /*
       * for even digits store the 1111 pattern at last digit
       */
      fullBitsNeeded = TRUE;
    }
    /*
     * if the first digit is digit_1 write it and skip
     * the next 4 bits, because they does not belong to
     * the BCD stream.
     */
    bf_codeByteNumber (4, digits[0], globs);

  #ifdef DEBUG_CCD
    TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) digits[0]);
    TRACE_CCD (globs, "skipping 4 bits");
  #endif

    bf_incBitpos (4, globs);
    k = 2;

    while (--repeat>1)
    {
      bf_codeByteNumber (4, digits[k], globs);
  #ifdef DEBUG_CCD
      TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) digits[k]);
  #endif
      k = ((k&1) EQ 0) ? (k-1) : (k+3);
    }
    if (repeat)
    {
    if (fullBitsNeeded)
    {
      bf_codeByteNumber (4, 0xf, globs);
      k = ((k&1) EQ 0) ? (k-1) : (k+3);
      }
      bf_codeByteNumber (4, digits[k], globs);
   }
    
  }
  else
  {
    /*
     * store the BCD digits into the bitstream.
     * The write order is 2,1,4,3,6,5 ...
     */
    if (repeat & 1)
    {
      /*
       * for odd digits store the 1111 pattern at last digit
       * in case of type BCD_NOFILL use 0 instead
       */
        fullBitsNeeded = TRUE;
    }

    k = 1;

    while ( repeat-- > 1)
    {
      bf_codeByteNumber (4, digits[k], globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "BCD digit (%X) written", (USHORT) digits[k]);
#endif
      k = ((k&1) EQ 1) ? (k-1) : (k+3);
    }
    if (fullBitsNeeded)
    {
      bf_codeByteNumber (4, (UBYTE)((startDigit NEQ 2) ? 0xf : 0), globs);
      k = ((k&1) EQ 1) ? (k-1) : (k+3);
    }
    bf_codeByteNumber (4, digits[k], globs);
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_init_table      |
+--------------------------------------------------------------------+

  PURPOSE : init the iei_table for each new msg that is to be decoded.
            The c_ref references a composition (msg). The function
            initialises the table entrys only for the used iei for
            this message.

*/

static void cdc_init_table (const ULONG c_ref, T_CCD_Globs *globs)

{
  ULONG  look_up;
  ULONG  num_elems;
  ULONG  ie_table_idx;
  ULONG  rlevel = globs->ccd_recurs_level;

  if (globs->iei_ctx[rlevel].valid AND rlevel < (ULONG) globs->last_level)
  {
    int i;
    /*
     * this iei context has been initialized before, so
     * no action for this. All deeper levels must be set
     * to invalid;
     */
    for (i=globs->last_level; i<MAX_RECURSIONS_PER_MSG; i++)
       globs->iei_ctx[i].valid = FALSE;
 
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "TAG table init for old level %d", rlevel);
#endif
  }
  else
  {
    /*
     * this iei context has not been initialized before, so
     * initialize the iei_table for this.
     */
#ifdef DEBUG_CCD
  TRACE_CCD (globs, "TAG table init for new level %d", rlevel);
#endif
    look_up   = (ULONG) mcomp[c_ref].componentRef;
    num_elems = (ULONG) mcomp[c_ref].numOfComponents;

    /*
     * store the startposition of the corresponding melem table and
     * the number of elements in the IEtable
     */
    globs->iei_ctx[rlevel].melemStart   = (USHORT) look_up;
    globs->iei_ctx[rlevel].ieTableLen   = (USHORT) num_elems;
    globs->iei_ctx[rlevel].EOCPending   = FALSE;
    globs->iei_ctx[rlevel].countSkipped = 0;

    /* 
     * for each element with an iei (ident) setup the 
     * the iei_table-entry.
     */
    ie_table_idx = 0;

    /*
     * if the number of IE in this message is greater than 
     * the allocated IEItable, generate an error.
     */
    if (num_elems > MAX_IE_PER_MSG)
      ccd_setError (globs, ERR_INTERNAL_ERROR, BREAK, (USHORT) -1);

    while (num_elems--)
    {
      if (melem[look_up].ident NEQ 0xffff)
      {  
        globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident = (UBYTE) melem[look_up].ident;
        
        /*
         * GSM1TV elements have only a 4 bit Tag (T). For this 
         * elements we have to shift the ident into the upper nibble
         * and set the lower nibble to zero. For GSM2T elements and
         * GSM1TV elements set the MSBit (Bit7). 
         */
        if (melem[look_up].codingType EQ CCDTYPE_GSM1_TV)
        {
          /*
           * shift into the upper nibble, clear the lower nibble
           * and set the MSBit.
           */
          globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident <<= 4;
          globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident |= 0x80;
          globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident &= 0xf0;
        }
        else
        {
          if (melem[look_up].codingType EQ CCDTYPE_GSM2_T)
          {
            /*
             * Set the MSBit.
             */
            globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident |= 0x80;
          }
        }
        globs->iei_ctx[rlevel].iei_table[ie_table_idx].act_amount = 0;
        globs->iei_ctx[rlevel].iei_table[ie_table_idx].exhausted = FALSE;

        switch (melem[look_up].codingType)
        {      
          case CCDTYPE_GSM1_TV:

          case CCDTYPE_GSM2_T:

          case CCDTYPE_GSM3_TV:

          case CCDTYPE_GSM4_TLV:

          case CCDTYPE_GSM5_TV:

          case CCDTYPE_GSM5_TLV:

          case CCDTYPE_GSM1_ASN:

          case CCDTYPE_GSM6_TLV:

            globs->iei_ctx[rlevel].iei_table[ie_table_idx].valid = TRUE;
            break;

          default:
            globs->iei_ctx[rlevel].iei_table[ie_table_idx].valid = FALSE;
            break;
        }
      }
      else
        globs->iei_ctx[rlevel].iei_table[ie_table_idx].valid = FALSE;

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "iei_table[%d] v=%d ident=%x level=%d",
                ie_table_idx,
                globs->iei_ctx[rlevel].iei_table[ie_table_idx].valid,
                globs->iei_ctx[rlevel].iei_table[ie_table_idx].ident,
                rlevel);
#endif

      look_up++;
      ie_table_idx++;
    }
    globs->iei_ctx[rlevel].valid = TRUE;
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_search_table    |
+--------------------------------------------------------------------+

  PURPOSE : search on the iei_table for the given TAG (T).
            if the TAG can be found (and - in case of CCDTYPE_GSM1_ASN - 
            if the information element isn't exhausted), the table 
            index was returned as a difference between the found index
            and the aktIndex, -127 otherwise.
  
*/

static int cdc_search_table 
            ( 
              int   akt_index,
              ULONG t,
              BOOL  limited,
              BOOL  *nonTaggedFound,
              T_CCD_Globs *globs
            )
{
  int   tab_idx;
  ULONG iei;
  int   ret = -127;
  ULONG rec_level = globs->ccd_recurs_level;
  
  /*
   * search from the akt position to the end of the table.
   * This is faster, because in correct messages the found Tag
   * is at a later position in the table. 
   */
  tab_idx = akt_index;
  
  *nonTaggedFound = FALSE;

  while (tab_idx < (int) globs->iei_ctx[rec_level].ieTableLen)
  {
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "looking for Tag(%x) iei_table[%d] v=%d ident=%x level=%d",
                t, tab_idx,
                globs->iei_ctx[rec_level].iei_table[tab_idx].valid,
                globs->iei_ctx[rec_level].iei_table[tab_idx].ident,
                rec_level);
#endif

    if (globs->iei_ctx[rec_level].iei_table[tab_idx].valid)
    {
      if (limited)
      {
        iei= (ULONG)(globs->iei_ctx[rec_level].iei_table[tab_idx].ident & 0x7f);
      }
      else
      {
        iei= (ULONG)(globs->iei_ctx[rec_level].iei_table[tab_idx].ident);
      }
      if ( iei EQ t )
      {
        if ( globs->iei_ctx[rec_level].iei_table[tab_idx].exhausted EQ FALSE )
        {
          return (tab_idx-akt_index);
        }
        else if ( (globs->iei_ctx[rec_level].melemStart + akt_index) 
            EQ  (int) globs->iei_ctx[rec_level].melemLast)
        {
         /*
          * allows multiple appearance of the repeated element is coded as 
          * TLV0 TLV1 TLV2 ....
          */
          return (tab_idx-akt_index);
        }
        else 
        {
          ret = (tab_idx-akt_index);
        }
      }
    }
    else
      *nonTaggedFound = TRUE;        
    tab_idx++;
  }

  tab_idx = 0;

  while (tab_idx < akt_index)
  {
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "looking for Tag(%x) iei_table[%d] v=%d ident=%x level=%d",
                t, tab_idx,
                globs->iei_ctx[rec_level].iei_table[tab_idx].valid,
                globs->iei_ctx[rec_level].iei_table[tab_idx].ident,
                rec_level);
#endif
    if (limited)
    {
      iei= (ULONG)(globs->iei_ctx[rec_level].iei_table[tab_idx].ident & 0x7f);
    }
    else
    {
      iei= (ULONG) globs->iei_ctx[rec_level].iei_table[tab_idx].ident;
    }
    if (globs->iei_ctx[rec_level].iei_table[tab_idx].valid
    AND (iei EQ t) )
    {
      if ( globs->iei_ctx[rec_level].iei_table[tab_idx].exhausted EQ FALSE )
      {
        return (tab_idx-akt_index);
      }
      else 
      {
        ret = (tab_idx-akt_index);
      }
    }
    tab_idx++;
  }
 
  if (ret != -127)
  {
    globs->SequenceError = TRUE;
  }
    
  return ret;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_decode_L        |
+--------------------------------------------------------------------+

  PURPOSE : Decode the length element of TLV and LV values
*/

static ULONG cdc_decode_L (const ULONG  e_ref, const ULONG len_l, T_CCD_Globs *globs)
{
  ULONG l;
  
  switch (melem[e_ref].codingType)
  {
    case CCDTYPE_GSM1_ASN:
      l = (ULONG) bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "decoding 8 bits, l = (%x)", l);
#endif
      if (l EQ 0x80)
        l = 0xFFFF;
      else if (l EQ 0x81)
      {
        l = (ULONG) bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "decoding 8 bits after 0x81, l = (%x)", l);
#endif
      }
      else if (l EQ 0x82)
      {
        l = bf_decodeShortNumber (16, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "decoding 16 bits after 0x82, l = (%x)", l);
#endif
      }
      break;
  
    case CCDTYPE_GSM5_TLV:
      l = (ULONG) bf_decodeByteNumber (8, globs);

      if (l EQ 0x81)
      {
        l = (ULONG) bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "decoding 8 bits after 0x81, l = (%x)", l);
      }
      else
      {
        TRACE_CCD (globs, "decoding 8 bits, l = (%x)", l);
#endif
      }
      break;

   case CCDTYPE_GSM6_TLV:
      l = bf_decodeShortNumber (16, globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "decoding 16 bits, l = (%x)", l);
#endif
      break;  

    default:
      l = (ULONG) bf_decodeByteNumber (len_l, globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "decoding %d bits, l = (%x)", len_l, l);
#endif
      break;
  }

  /*
   * Write the value of l at the end of UPN Stack.
   * this could be read by an IE of the coding type NO_CODE.
   */  
  globs->KeepReg[0] = l ; 
  return l;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_tagged_LV_decode|
+--------------------------------------------------------------------+

  PURPOSE : If the parameter lenL is set to a positive value,
            this function decodes the L-component.
            After this it decodes the element referenced
            by eRef out of the bitstream into the C-Structure
            (globs->pstruct) at position globs->pstructOffs.
            If a repeat value is defined for this element,
            this function decodes only one appeareance
            of the element because it is a tagged 
            element. In this case the decoded element is stored in
            an array wich is indexed by eIndex;
*/

static BOOL cdc_tagged_LV_decode (const ULONG e_ref,
                                  ULONG e_index,
                                  const ULONG len_l,
                                  T_CCD_Globs *globs)
{
  ULONG  amount, l;
  USHORT act_maxBP, tmp_maxBP; 
  BOOL   endOfComposition = FALSE;
  BOOL   asn1=FALSE;
  U32    offset=0;
#ifdef DYNAMIC_ARRAYS
  U8    *old_pstruct = NULL;
#endif
      
  if (melem[e_ref].codingType EQ CCDTYPE_GSM1_ASN)
    asn1 = TRUE;
 
  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * set the offset into the C-structure for this element. 
     */
    if (melem[e_ref].optional)
    {
      globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
    }

    if (melem[e_ref].repType EQ 'i')
    {
      /*
       * The number of appearance of all repeatable IEs may
       * differ in a message. So we have to store the number
       * in a c_xxx counter into the C-Structure.
       */
      if (melem[e_ref].maxRepeat > 65535)
        *(ULONG  *) (globs->pstruct + globs->pstructOffs++) = e_index;
      else if (melem[e_ref].maxRepeat > 255)
        *(USHORT *) (globs->pstruct + globs->pstructOffs++) = (USHORT) e_index;
      else
        globs->pstruct[globs->pstructOffs] = (UBYTE) e_index;

      globs->pstructOffs++;

      /*
       * Recalculate the struct offset for repeatable IEs.
       * New pointer types 'R' and 'F' are equivalent to 'V'.
       */
#ifdef DYNAMIC_ARRAYS
      offset = (e_index-1) * ((melem[e_ref].elemType EQ 'V'
                              OR melem[e_ref].elemType EQ 'R'
                              OR melem[e_ref].elemType EQ 'F'
                              ) ? mvar[melem[e_ref].elemRef].cSize
			     : mcomp[melem[e_ref].elemRef].cSize
                             );
#else
      offset = (e_index-1) * ((melem[e_ref].elemType EQ 'V')
				      ? mvar[melem[e_ref].elemRef].cSize
				      : mcomp[melem[e_ref].elemRef].cSize
				      );
#endif
    }
  }
  /*
   * If len_l > 0 read the l-Component out of the bistream.
   */
  if (len_l)
  {
    if( len_l <= (ULONG) (globs->maxBitpos - globs->bitpos) )
    {
      act_maxBP = globs->maxBitpos;
      l = cdc_decode_L (e_ref, len_l, globs);

      if (l EQ 0xFFFF)
      {
        /*
         * For ASN1-BER encoding we must look for the special
         * length 0x80 because it indicates the indefinite
         * length. This needs a special handling with later EOC tags.
         *
         */
        globs->iei_ctx[globs->ccd_recurs_level].EOCPending = TRUE;
        globs->numEOCPending++;

  #ifdef DEBUG_CCD
        TRACE_CCD (globs, "implicit ASN1 length - EOC is pending");
  #endif
      }
      else
      {
        /*
         * Calculate the max bitpos for this element
         */
        tmp_maxBP = (USHORT) (globs->bitpos + (l*8));
        if (globs->buflen < tmp_maxBP)
        {
          ccd_recordFault (globs, ERR_MSG_LEN, CONTINUE, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        else if (globs->maxBitpos < tmp_maxBP)
        {
          ccd_recordFault (globs, ERR_ELEM_LEN, BREAK, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        globs->maxBitpos = (USHORT)MINIMUM (globs->buflen, tmp_maxBP);
        tmp_maxBP = globs->maxBitpos;
      }
    }
    else 
    {
      ccd_recordFault (globs, ERR_ELEM_LEN, BREAK, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
    }
  }
#ifdef DYNAMIC_ARRAYS
  /*
   * Check for pointer types; allocate memory if necessary.
   */
  if ((melem[e_ref].elemType >= 'P' AND melem[e_ref].elemType <= 'R') OR
      (melem[e_ref].elemType >= 'D' AND melem[e_ref].elemType <= 'F')) 
  {
    U32     cSize;
    U8      *addr;

    /*
     * Find size to allocate;
     * - Read from mcomp or mvar according to type
     */
    cSize = (ULONG)((melem[e_ref].elemType EQ 'V' OR
		      melem[e_ref].elemType EQ 'R')
                     ? mvar[melem[e_ref].elemRef].cSize
                     : mcomp[melem[e_ref].elemRef].cSize
		     );

    /*
     * Allocate additional memory
     */
    addr = (U8 *)DP_ALLOC( cSize, globs->alloc_head, DP_NO_FRAME_GUESS);

    /* If no memory, log error and return immediately */
    if (addr EQ NULL) {
      ccd_setError (globs, ERR_NO_MEM,
                    BREAK,
                    (USHORT) -1);
      return endOfComposition;
    }
    else
      memset (addr, 0, (size_t)(cSize));

    /*
     * Memory allocated;
     * 1. Save old "globs->pstruct" variables
     * 2. Store pointer to freshly allocated memory area in structure
     * 3. Initialize pstruct to point to the freshly allocated memory area.
     * 4. Initialize pstructOffs to 0 to start decoding at offset 0
     *    in the new memory area.
     */
    old_pstruct        = globs->pstruct;
    *(U8 **)(globs->pstruct + globs->pstructOffs) = addr;
    globs->pstruct     = addr;
    globs->pstructOffs = 0;
  }
  else
  {
    globs->pstructOffs += offset;
  }
#else /* DYNAMIC_ARRAYS */
  globs->pstructOffs += offset;
#endif /* DYNAMIC_ARRAYS */


  /*
   * Decode the value. Keep caution with BER encoding of ASN1 integers.
   * All other types can be decoded by a generic function.
   */
  if (asn1 AND melem[e_ref].elemType EQ 'V'
      AND
      melem[e_ref].repType EQ ' '
      AND
      l NEQ 0xFFFF
     )
  {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "BER decoding of ASN.1 integer %s",
               ccddata_get_alias((USHORT) e_ref, 1));
#else
    TRACE_CCD (globs, "BER decoding of ASN.1 integer; e_ref = %d", melem[e_ref].elemRef);
#endif
#endif

    if (mvar[melem[e_ref].elemRef].cType EQ 'X')
      bf_readBitChunk (l*8, globs);
    else
      bf_readBits (l*8, globs);
  }
  else
  {
    amount = 1;
    if (len_l)
    {
      if (l > 0)
      {
        cdc_decodeElemvalue (e_ref, &amount, globs);
      }
      else
      {
        amount = 0;
      }
    }
    else
    {
      if (melem[e_ref].codingType != CCDTYPE_GSM2_T)
        cdc_decodeElemvalue (e_ref, &amount, globs);
    }
  }
#ifdef DYNAMIC_ARRAYS
  /*
   * Restore globs->pstruct for possible use below
   */
  if (old_pstruct NEQ NULL)
    globs->pstruct     = old_pstruct;
#endif

  if (asn1 AND globs->numEOCPending AND !bf_endOfBitstream(globs))
  {
    UBYTE T = bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "looking for EOC decoding 8 bits T = (%x)", T);
#endif

    if (T EQ 0)
    {
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "First EOC octet found");
#endif

      if (globs->iei_ctx[globs->ccd_recurs_level].EOCPending) 
      {
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "End of ASN1 TLV");
#endif
        /*
         * Skip the second EOC octet.
         */
        bf_incBitpos (8, globs);
        globs->iei_ctx[globs->ccd_recurs_level].EOCPending = FALSE;
        globs->numEOCPending--;
      }
      else
      {
        /*
         * The read first EOC octet belongs to an ASN1 TLV of a 
         * higher recursion level. Let it be read and evalauted later 
         * again for that IE.
         */
        bf_setBitpos (globs->bitpos-8, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "End of higer level ASN1 TLV");
        TRACE_CCD (globs, "Decrementing bitpos by 8 to %d", globs->bitpos);
#endif
        endOfComposition = TRUE;
      }
    }
    else
    {
      if (globs->iei_ctx[globs->ccd_recurs_level].EOCPending)
      {
        /*
         * EOC element is pending but not found.
         */
        ccd_setError (globs, ERR_EOC_TAG_MISSING,
                      BREAK,
                      (USHORT) T,
                      globs->bitpos-8,
                      (USHORT) -1);

        bf_setBitpos (globs->bitpos-8, globs);
      }
      else
      {
        /*
         * normal TAG leave it in the bitstream
         */
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "Normal TAG - Decrementing bitpos by 8 to %d", globs->bitpos);
#endif
        bf_setBitpos (globs->bitpos-8, globs);
      }
    }
  }

  if (len_l)
  {
    if (!asn1)
    {
      if (globs->bitpos > tmp_maxBP)
      {
        ccd_recordFault (globs, ERR_ELEM_LEN, CONTINUE, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      else
      {
        /*
         * set the bitpos to the end of the LV or TLV element
         */
        bf_setBitpos (tmp_maxBP, globs);
      }
    }
    /*
     * set the maxBitpos to the next octet boundary if the 
     * last non-spare IE does not end at an octet boundary.
     * This is necessary for avoiding an early end of decoding.
     */
/*  
    globs->maxBitpos = globs->buflen;
*/
    globs->maxBitpos = act_maxBP;
  }

  return endOfComposition;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_normal_LV_decode|
+--------------------------------------------------------------------+

  PURPOSE : If the parameter lenL is set, this function
            decodes the L-component. After this it decodes
            the element referenced by eRef from the
            bitstream into the C-Structure (globs->pstruct)
            at position globs->pstructOffs. 
            If a repeat value is defined for this element
            this function decodes the V-component multiple and stores
            the values into an array.
*/

static BOOL cdc_normal_LV_decode (const ULONG  e_ref,
                                  const ULONG  len_l,
                                  T_CCD_Globs *globs)
{
  ULONG  l, repeat, amount, max_rep;
  USHORT act_maxBP, tmp_maxBP; 
  BOOL   is_variable;
  BOOL   endOfComposition = FALSE;
  BOOL   asn1;
  BOOL   length_in_bits;
#ifdef DYNAMIC_ARRAYS
  U8    *old_pstruct = NULL;
#endif  

  switch (melem[e_ref].codingType)
  {
    case CCDTYPE_GSM1_ASN:
      asn1 = TRUE;
      length_in_bits = FALSE;
      break;

    case CCDTYPE_GSM7_LV:
      asn1 = FALSE;
      length_in_bits = TRUE;
      break;

    default:
      asn1 = FALSE;
      length_in_bits = FALSE;
      break;
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

  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE.
     * Setup the offset into the C-structure for this element
     */
    if (melem[e_ref].optional)
    {
      globs->pstruct[globs->pstructOffs++] = (UBYTE) TRUE;
    }

    if (is_variable)
    {
      /*
       * for variable sized elements store the min-value
       * as counter into the C-Structure (c_xxx).
       */
      if (max_rep > 65535)
        *(ULONG *) (globs->pstruct + globs->pstructOffs++) = repeat;
      else if (max_rep > 255)
        *(USHORT *) (globs->pstruct + globs->pstructOffs++) = (USHORT) repeat;
      else
        globs->pstruct[globs->pstructOffs] = (UBYTE) repeat;

      globs->pstructOffs++;
    }
  }

  /*
   * if len_l > 0 read the l-Component out of the bistream.
   */
  if (len_l)
  {
    if( len_l <= (ULONG)(globs->maxBitpos - globs->bitpos) )
    {
      act_maxBP = globs->maxBitpos;

      l = cdc_decode_L (e_ref, len_l, globs);

      if (l EQ 0xFFFF)
      {
        /*
         * for ASN1 element coding we must look for the special
         * length 0x80 because it indicates the indefinite
         * length. This needs a special handling with later EOC tags.
         */
        globs->iei_ctx[globs->ccd_recurs_level].EOCPending = TRUE;

        globs->numEOCPending++;

  #ifdef DEBUG_CCD
        TRACE_CCD (globs, "implicit ASN1 length - EOC is pending");
  #endif
      }
      else
      {
        /*
         * calculate the max bitpos for this element
         */
        if (!length_in_bits)
          l *= 8;

        tmp_maxBP = (USHORT) (globs->bitpos + l);

        if (globs->buflen < tmp_maxBP)
        {
          ccd_recordFault (globs, ERR_MSG_LEN, CONTINUE, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        else if (globs->maxBitpos < tmp_maxBP)
        {
          ccd_recordFault (globs, ERR_ELEM_LEN, BREAK, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
        }
        globs->maxBitpos = (USHORT)MINIMUM (globs->buflen, tmp_maxBP); 
        tmp_maxBP = globs->maxBitpos;
       
        /*
         * for bitfields which appear in TLV or LV elements
         * we must calculate the length (repeat) from the l values
         */
         if (melem[e_ref].repType EQ 'b')
           repeat = l;
      }
    }
    else
    {
      ccd_recordFault (globs, ERR_ELEM_LEN, BREAK, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
    }
  }
#ifdef DYNAMIC_ARRAYS
  /*
   * MVJ: Dynamic array addition.
   * Check for pointer types; allocate memory if necessary.
   */
  if ((melem[e_ref].elemType >= 'P' AND melem[e_ref].elemType <= 'R') OR
      (melem[e_ref].elemType >= 'D' AND melem[e_ref].elemType <= 'F')) {
    ULONG    cSize, rep;
    U8      *addr;

    /*
     * Find size to allocate;
     * - Read from mcomp or mvar according to type
     * - Unbounded (0-terminated) ASN1-types are allocated with MAX repeat
     */
    if (globs->iei_ctx[globs->ccd_recurs_level].EOCPending) {
      rep = (ULONG) melem[e_ref].maxRepeat;
    } else {
      rep = repeat;
    }
    cSize = (ULONG)((melem[e_ref].elemType EQ 'V' OR
		      melem[e_ref].elemType EQ 'R')
                     ? mvar[melem[e_ref].elemRef].cSize
                     : mcomp[melem[e_ref].elemRef].cSize
		     ) * rep;

    /*
     * Allocate additional memory
     */
    addr = (U8 *)DP_ALLOC( cSize, globs->alloc_head, DP_NO_FRAME_GUESS);

    /* If no memory, log error and return immediately */
    if (addr EQ NULL) {
      ccd_setError (globs, ERR_NO_MEM,
                    BREAK,
                    (USHORT) -1);
      return endOfComposition;
    }
    else
      memset (addr, 0, (size_t)cSize);

    /*
     * Memory allocated;
     * 1. Save old "globs->pstruct" variables
     * 2. Store pointer to freshly allocated memory area in structure
     * 3. Initialize pstruct to point to the freshly allocated memory area.
     * 4. Initialize pstructOffs to 0 to start decoding at offset 0
     *    in the new memory area.
     */
    old_pstruct        = globs->pstruct;
    *(U8 **)(globs->pstruct + globs->pstructOffs) = addr;
    globs->pstruct     = addr;
    globs->pstructOffs = 0;
  }
#endif

  /*
   * Decode the value. Keep caution with BER encoding of ASN1 integers.
   * All other types can be decoded by a generic function.
   */
  if (asn1 AND melem[e_ref].elemType EQ 'V' 
      AND 
      melem[e_ref].repType EQ ' ' 
      AND 
      l NEQ 0xFFFF
     )
  {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
    TRACE_CCD (globs, "BER decoding of ASN.1 integer %s",
               ccddata_get_alias((USHORT) e_ref, 1));
#else
    TRACE_CCD (globs, "BER decoding of ASN.1 integer; e_ref = %d", melem[e_ref].elemRef);
#endif
#endif
    amount = l;
    if (mvar[melem[e_ref].elemRef].cType EQ 'X')
      bf_readBitChunk (l, globs);
    else
      bf_readBits (l, globs);
  }
  else
  {
    amount = repeat;
    if (len_l)
    {
      if (l > 0)
      {
        cdc_decodeElemvalue (e_ref, &amount, globs);
      }
      else
      {
        amount = 0;
      }
    }
    else
    {
      if (melem[e_ref].codingType != CCDTYPE_GSM2_T)
        cdc_decodeElemvalue (e_ref, &amount, globs);
    }
  }

#ifdef DYNAMIC_ARRAYS
  /*
   * Restore globs->pstruct for possible use below
   */
  if (old_pstruct NEQ NULL) {
    globs->pstruct     = old_pstruct;
  }
#endif

  if (amount NEQ repeat AND is_variable)
  {
    /*
     * If the number of decoded elements is not equal to the given
     * repeat value, because the bitstream or the IE ended,
     * store the new c_xxx value.
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if (melem[e_ref].optional)
      globs->pstructOffs++;

    globs->pstruct[globs->pstructOffs] = (UBYTE) amount;

    if (melem[e_ref].repType NEQ 'i')
    {
      /*
       * if this element is not of the repeat style 'interval'
       * ([X..Y] where X and Y are constants) we have to generate
       * an ccd error because some outstanding repeats are missing.
       */
      ccd_setError (globs, ERR_MAND_ELEM_MISS,
                    CONTINUE,
                    (USHORT) -1);
    }
  }

  if (asn1 AND globs->numEOCPending AND !bf_endOfBitstream(globs))
  {
    UBYTE T = bf_decodeByteNumber (8, globs);
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "looking for EOC decoding 8 bits T = (%x)", T);
#endif

    if (T EQ 0)
    {
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "First EOC octet found");
#endif

      if (globs->iei_ctx[globs->ccd_recurs_level].EOCPending) 
      {
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "End of ASN1 TLV");
#endif
        /*
         * Skip the second EOC octet.
         */
        bf_incBitpos (8, globs);
        globs->iei_ctx[globs->ccd_recurs_level].EOCPending = FALSE;
        globs->numEOCPending--;
      }
      else
      {
        /*
         * The read first EOC octet belongs to an ASN1 TLV of a 
         * higher recursion level. Let it be read and evalauted later 
         * again for that IE.
         */
        bf_setBitpos (globs->bitpos-8, globs);
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "End of higer level ASN1 TLV");
        TRACE_CCD (globs, "Decrementing bitpos by 8 to %d", globs->bitpos);
#endif
        endOfComposition = TRUE;
      }
    }
    else
    {
      if (globs->iei_ctx[globs->ccd_recurs_level].EOCPending)
      {
        /*
         * EOC element is pending but not found.
         */
        ccd_setError (globs, ERR_EOC_TAG_MISSING,
                      BREAK,
                      (USHORT) T,
                      globs->bitpos-8,
                      (USHORT) -1);

        bf_setBitpos (globs->bitpos-8, globs);
      }
      else
      {
        /*
         * normal TAG leave it in the bitstream
         */
#ifdef DEBUG_CCD
        TRACE_CCD (globs, "Normal TAG - Decrementing bitpos by 8 to %d", globs->bitpos);
#endif
        bf_setBitpos (globs->bitpos-8, globs);
      }
    }
  }

  if (len_l)
  {
    if (!asn1)
    {
      if (globs->bitpos > tmp_maxBP)
      {
        ccd_recordFault (globs, ERR_ELEM_LEN, CONTINUE, (USHORT) e_ref, globs->pstruct + globs->pstructOffs);
      }
      else
      {
        /*
         * set the bitpos to the end of the LV or TLV element
         */
         bf_setBitpos (tmp_maxBP, globs);
      }
    }

    /*
     * set the maxBitpos to the next octet boundary if the 
     * last non-spare IE does not end at an octet boundary.
     * This is necessary for avoiding an early end of decoding.
     */
/*
    globs->maxBitpos = globs->buflen;
*/
    globs->maxBitpos = act_maxBP;
  }

  return endOfComposition;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_skipElem        |
+--------------------------------------------------------------------+

  PURPOSE : Skip an element referenced by eRef. This function
            perform a decoding of this element, but not into
            the target C-Structure. A dummy C-Structure is used
            instead.
            The complete decoding is necesary, because there is
            no information about the length of this element.
            B.t.w. for mandatory elements with fixed length, we can
            calculate the length, for optional elements or for
            variable sized arrays or bitbuffers it is impossible
            without decoding the entire element.
*/

static void cdc_skipElem (const ULONG e_ref, const ULONG len_l, T_CCD_Globs *globs)
{
  UBYTE  *ActStructAddr;
  U32    ActStructOffs;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "skipping element %d",
             melem[e_ref].elemRef);
#endif

  ActStructAddr = globs->pstruct; 
  ActStructOffs = globs->pstructOffs; 

  globs->pstruct     = dummy; 
  globs->pstructOffs = 0;
  
  cdc_tagged_LV_decode (e_ref, 1, len_l, globs);

  globs->pstruct     = ActStructAddr; 
  globs->pstructOffs = ActStructOffs; 
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_tlv_decode      |
+--------------------------------------------------------------------+

  PURPOSE : Decoding of the T-components of T TV and TLV typed
            information elements. The len determines the
            length of the T component. This function returns the
            index (reference) of the rigth element. If the
            iei is not known in this composition (msg or submsg)
            an error handling is done and NO_REF is returned. 

*/

SHORT cdc_tlv_decode (const ULONG c_ref,
                      const ULONG e_ref,
                      const T_TLV_SORT  *tlv_inf,
                            T_CCD_Globs *globs)
{
  ULONG  repeat, max_rep;
  ULONG  ie_amount, l, len_l, len_t, t;
  BOOL   is_variable, nonTagged, limitSearch=FALSE;
  UBYTE  CR=FALSE;
  SHORT  IdxOffset = 0;
  int    ieTableIdx;
  BOOL   asn1, non_std_tag;
  SHORT  ret;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;
  /* .../src/linux/include/asm/current.h defines a macro 'current' */
  T_UNKNOWN_TAG *first, *currentTag;


  /*
   * Set the flag for the type of extension which is to expect
   * at the end of the message.
   */
  globs->SeekTLVExt = TRUE;

  /* Set ref number for calcidx table. */
  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * if this element is conditional, check the condition
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return 1;

  len_t = 8; 
  switch (melem[e_ref].codingType)
  {
    case CCDTYPE_GSM1_ASN:
      asn1        = TRUE;
      non_std_tag = FALSE;
      len_l        = 8;
      break;

    case CCDTYPE_GSM5_TV:
    case CCDTYPE_GSM5_TLV:
      non_std_tag = TRUE;
      asn1        = FALSE;
      len_l        = 8;
      break;

    case CCDTYPE_GSM6_TLV:
      non_std_tag = FALSE;
      asn1        = FALSE;
      len_l        = 16;
      break;

    case CCDTYPE_GSM7_LV:
      non_std_tag = FALSE;
      asn1        = FALSE;
      len_l        = 7;
      break;

    default:
      asn1        = FALSE;
      non_std_tag = FALSE;
      len_l        = 8;
      break;
  }

  /*
   * if this element have a defined Prolog
   * we have to process it before decoding the bitstream
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (tlv_inf->gotTag)
  {
    /* 
     * tagged element
     */
    len_t = 8;
    /*
     * initialize the iei_table for each new message
     */
    if (globs->ccd_recurs_level NEQ globs->last_level)
    {
      cdc_init_table (c_ref, globs);
      globs->TagPending    = FALSE;
      globs->SequenceError = FALSE;
      globs->last_level    = globs->ccd_recurs_level;
    }

    /*
     * calculate the index into the ieTable for this element
     */
    ieTableIdx = (int)(e_ref - globs->iei_ctx[globs->ccd_recurs_level].melemStart);

    if (globs->TagPending)
    {
      /*
       * if we previously read a t value and does not processed it
       * get this pending tag.
       */
      t = (ULONG) globs->PendingTag;
      globs->TagPending = FALSE;
    }
    else
    {
      /*
       * read the information element identifier out of the bitstream.
       * If the first bit (MSBit) of the t-component is set, it is
       * a Tag of a TYPE1 or TYPE2 element.
       */
      
      t = (ULONG) bf_decodeByteNumber (8, globs);



      if (!asn1 AND !non_std_tag AND (t & 0x80) EQ 0x80 AND (t & 0xA0) NEQ 0xA0)
      {
        ULONG Tag4 = t & 0xf0;        
        /*
         * MSBit is set. We have to check if the Tag value can
         * be found as a 4 bit or 8 bit value in the IEI-table.
         */
        if (cdc_search_table (ieTableIdx, Tag4, limitSearch, &nonTagged, globs) NEQ -127) 
        {
          /*
           * Tag found as a 4 bit value. Decrement the readpointer
           * of the bitstream by 4, because we have read out 4 bits
           * to much.
           */
          bf_setBitpos (globs->bitpos-4, globs);
          t = Tag4;
          len_t =4;
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "4 bit Tag decrementing bitpos by 4 to %d", globs->bitpos);
#endif
        }
      }
    }

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "reading t = 0x%X", t);
#endif

    if (melem[e_ref].codingType EQ CCDTYPE_GSM5_TLV)
    {
      limitSearch = TRUE;  
      CR = (UBYTE) (((t & 0x80) EQ 0x80) ? TRUE : FALSE);
      t &= 0x7f;
    }


    if (asn1 AND t EQ 0x00)
    {
      /*
       * This is for ASN1 element coding the special
       * End Of Component Tag (EOC). The following length must be zero.
       */
      bf_setBitpos (globs->bitpos-8, globs);

#ifdef DEBUG_CCD
      TRACE_CCD (globs, "ASN1 End of Component found belongs to higher TLV");
      TRACE_CCD (globs, "leaving this level and decrementing bitpos by 8 to %d", globs->bitpos);
#endif

      return END_OF_COMPOSITION; /* skip the remaining elements in this level */
    } /* asn1 and EOC */
    else
    {
      if ((IdxOffset = (SHORT) cdc_search_table (ieTableIdx, t, limitSearch, &nonTagged, globs)) == -127)
      {
        /*
         * t (iei) not defined in this composition (msg or submsg)
         */
        if (asn1)
        {
          if (melem[mcomp[c_ref].componentRef + mcomp[c_ref].numOfComponents -1].codingType == CCDTYPE_GSM5_V)
          {
            /* Restore the old bitposition (before the 'TAG') and return
             * IdxOffset to jump to last element of the composition.
             * The coding type of this elements is CCDTYPE_GSM5_V
             */
            bf_setBitpos (globs->bitpos-8, globs);
            IdxOffset = (SHORT)(mcomp[c_ref].numOfComponents - ieTableIdx - 1);
            return (IdxOffset);
          }

          /*
           * for recursive ASN.1 structs it is possible that the foreign
           * tag belongs to a upper level composition of element.
           *
           * 
           * Restore the old bitposition (before the TAG) and return
           * END_OF_COMPOSITION to leave this composition level
           */
          bf_setBitpos (globs->bitpos-8, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Unknown Tag. It may belong to upper ASN.1 comp -> dec. bitpos by 8 to %d", globs->bitpos);
#endif

          return END_OF_COMPOSITION; /* skip the remaining elements in this level */
        }
        else if (nonTagged)
        {
          U16 actBitpos;
          actBitpos = globs->bitpos-8;

          if (melem[mcomp[c_ref].componentRef + mcomp[c_ref].numOfComponents -1].codingType == CCDTYPE_GSM5_V &&
              melem[e_ref].codingType EQ CCDTYPE_GSM5_TLV)
          {
#if defined (CCD_TEST)
            currentTag = (T_UNKNOWN_TAG *) malloc(sizeof(T_UNKNOWN_TAG));
#else
            currentTag = (T_UNKNOWN_TAG *) D_ALLOC(sizeof(T_UNKNOWN_TAG));
#endif
            first = currentTag;
            currentTag->bitpos = globs->bitpos-8;
            currentTag->errCode = ERR_NO_MORE_ERROR;
            currentTag->next = NULL;

            /* unknown GSM Type TLV -> skip 'l' bytes */
            /* at least 8 bits must remain for following expeceted tagged element */
            while (globs->maxBitpos - 8 - globs->bitpos >= 8)
            {
              currentTag->bitpos = globs->bitpos-8;
              currentTag->tag    = (UBYTE) t;

              /* 
               * Expecting a CCDTYPE_GSM5_TLV type we get an unknown tag with MSB set.
               * Store bitpos and t for the application (SAT) for the handling of 
               * comprehension required elements.
               */

              if (CR)
              { // save (ERR_COMPREH_REQUIRED; globs->bitpos-len_t) 
                currentTag->errCode =  ERR_COMPREH_REQUIRED;
              }
              else 
              { // save (ERR_IE_NOT_EXPECTED; globs->bitpos-len_t) 
                currentTag->errCode =  ERR_IE_NOT_EXPECTED;
              }

              l = (ULONG) bf_decodeByteNumber (8, globs);
              bf_incBitpos ((l << 3) , globs);
              
              t = (ULONG) bf_decodeByteNumber (8, globs);

              limitSearch = TRUE;  
              CR = (UBYTE) (((t & 0x80) EQ 0x80) ? TRUE : FALSE);
              t &= 0x7f;

              if (cdc_search_table (ieTableIdx, t, limitSearch, &nonTagged, globs) != -127)
              {
                bf_setBitpos (globs->bitpos-8, globs);
                // set all ccd Errors
                do 
                {
                  currentTag = first;
                  ccd_setError (globs, currentTag->errCode,
                                CONTINUE,
                                (USHORT) currentTag->tag,
                                currentTag->bitpos,
                                (USHORT) -1);
                  first = currentTag->next;
#if defined (CCD_TEST)
                  free(currentTag);
#else                
                  D_FREE(currentTag);
#endif               
                }
                while (first != NULL );

                return 0;
              }
              else
              {                
#if defined (CCD_TEST)
                currentTag->next = (T_UNKNOWN_TAG *) malloc(sizeof(T_UNKNOWN_TAG));
#else                
                currentTag->next = (T_UNKNOWN_TAG *) D_ALLOC(sizeof(T_UNKNOWN_TAG));
#endif               
                currentTag = currentTag->next;
                currentTag->next = NULL;
              }
            }

            do
            {
              currentTag = first;
              first = currentTag->next;
#if defined (CCD_TEST)
              free(currentTag);
#else                
              D_FREE(currentTag);
#endif               
            }
            while (first != NULL );
          }

            /*
             * a non tagged element is possible in the message. If the tag 
             * can not be found, the tag may be the beginning of the non  tagged
             * element. E.g. rest octetts in sysinfo 4
             * 
             * Restore the old bitposition (before the TAG) and return 1 to 
             * go to the next element.
             */
          
          bf_setBitpos (actBitpos, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Unknown Tag but mand. IE possible -> dec. bitpos by 8 to %d", globs->bitpos);
#endif

          return 1;
        }


        /*
         * Otherwise look if it is a type 1,2 or 4 Element
         */
        if ((t & 0x80) EQ 0x80)
        {
          /* MSBit set -> GSM Type 1 or Type2 -> skip 1 byte */
          /* position already incremented by decoding the TAG value */
        }
        /* Just another reason for getting IdxOffset equal to 0. */
        else if (globs->ccd_recurs_level >= MAX_RECURSIONS_PER_MSG)
        {
          ccd_setError (globs, ERR_INTERNAL_ERROR, BREAK, (USHORT) -1);
        }
        else
        {
          /* 
           * Expecting a CCDTYPE_GSM5_TLV type we get an unknown tag with MSB set.
           * Store bitpos and t for the application (SAT) for the handling of 
           * comprehension required elements.
           */
          if (CR)
          {
            ccd_setError (globs, ERR_COMPREH_REQUIRED,
                          CONTINUE,
                          (USHORT) t,
                          (USHORT) globs->bitpos-len_t,
                          (USHORT) -1);
          }
          /* 
           * Expecting other types than CCDTYPE_GSM5_TLV we get an unknown tag with
           * comprehension required bits (5, 6, 7 and 8 of IEI according to GSM0407)
           * are set to zero.
           * Store bitpos and t for the application for the handling of comprehension
           * required elements.
           */
          else if ((t & 0x70) EQ 0 AND
                    melem[e_ref].codingType NEQ CCDTYPE_GSM5_TLV)
          {
            ccd_setError (globs, ERR_COMPREH_REQUIRED,
                          CONTINUE,
                          (USHORT) t,
                          (USHORT) globs->bitpos-len_t,
                          (USHORT) -1);
          }
          /* 
           * We get an unknown tag and any sort of comprehension required flag is set.
           * Store bitpos and t for the application
           */
          else 
          {
            ccd_setError (globs, ERR_IE_NOT_EXPECTED,
                          CONTINUE,
                          (USHORT) t,
                          (USHORT) globs->bitpos-len_t,
                          (USHORT) -1);
          }
          
          /* MSBit cleared -> GSM Type TLV -> skip 'l' bytes */
          if (globs->maxBitpos - globs->bitpos >= 8)
          {
            l = (ULONG) bf_decodeByteNumber (8, globs);
            bf_incBitpos ((l << 3) , globs);
          }
          else
          {
            ccd_recordFault (globs,
                             ERR_ELEM_LEN,
                             BREAK,
                             (USHORT) e_ref,
                             globs->pstruct + globs->pstructOffs);
          }
        }

        /*
         * return 0 -> that means try it again with this actual element
         * referenced by e_ref
         */
        return 0;
      } /* tag not found */
      else
      {
        T_IEI_TABLE *iei_tbl = &globs->iei_ctx[globs->ccd_recurs_level].iei_table[ieTableIdx];
        /*
         * element definition for this iei found
         */
        if (IdxOffset NEQ 0)
        {
          /*
           * found index differs from the actual index
           */
          globs->TagPending    = TRUE;
          globs->PendingTag    = (UBYTE) t;

          if (!asn1 AND IdxOffset < 0)
          {
            /*
             * found an element in wrong sequence
             * (for ASN1 elements the sequence order is not relevant)
             */
            ccd_setError (globs, ERR_IE_SEQUENCE,
                          (UBYTE) ((asn1) ? BREAK : CONTINUE),
                          (USHORT) t,
                          (USHORT) globs->bitpos-len_t,
                          (USHORT) -1);

            globs->SequenceError = TRUE;
          }
          if (globs->SequenceError)
          {
            globs->RefBeforeError = (USHORT) e_ref;
          }
          if (asn1) 
          {
            globs->iei_ctx[globs->ccd_recurs_level].countSkipped += IdxOffset;
          }
          /*
           * skip to the found element
           */
          return (IdxOffset);
        }
        else
        {
          globs->iei_ctx[globs->ccd_recurs_level].melemLast = (USHORT) e_ref;

          if (iei_tbl->act_amount == 0)
          {
            /*
             * first apearance of this iei
             * calculate the upper and lower boundaries and the
             * facility of multiple appearance of this tagged element
             * in the bitstream.
             */

            /*
             * The element is repeatable. There are three kinds of 
             * repeat definitions valid for standard elements:
             *  [5]    - The element is repeated 5 times.
             *  [0..5] - The element is repeated 0 to 5 times.
             *  [a..5] - The element is repeated "the value of a" times.
             *
             * For tagged elements the following processing is defined:
             *
             *  [5]    - The t-Component is decoded
             *           (maybe the l-Component too (if defined one).
             *           After this the V-component of the element
             *           is decoded 5 times.
             *
             *  [0..5] - The t- and maybe the l-Component are decoded.
             *           After this one V-Component is decoded and it
             *           is stored as an array entry into
             *           the target C-Structure. In this case the
             *           parameter ieIndex gives the index into
             *           this array, where the element has to
             *           be written into.
             *
             *  [a..5] - The t- and maybe the l-Component are decoded.            
             *           After this one V-Component is decoded
             *           "a" times and is stored into the C-Structure
             *           as an array.
             *           
             */
            switch (melem[e_ref+IdxOffset].repType)
            {
              case 'i':
                /* 
                 * multiapearance of this element. The V-component is 
                 * repeated once
                 */
                is_variable = ccd_calculateRep (e_ref+IdxOffset,
                                  &repeat,
                                  &max_rep,
                                  globs);

                iei_tbl->max_amount = (UBYTE) max_rep;
                iei_tbl->multiple   = TRUE;
                break;

              case 'v':
              case 'b':
              default:
                /* 
                 * if this element is repeatable, and the number of
                 * repeats depends on another element, the valid amount
                 * of this element is 1 and the V-component will be
                 * repeated.
                 */
                iei_tbl->max_amount = 1;
                iei_tbl->multiple   = FALSE;
                break;
            }
            iei_tbl->act_amount = 1;
          }

          if (iei_tbl->act_amount <= iei_tbl->max_amount)
          {
            /*
             * process only the max_amount appearances of each element.
             * All additional IEs are ignored.
             */
            ie_amount = (ULONG)(iei_tbl->act_amount)++;
          }
          else
          {
            if (asn1)
            {
              /* For ASN1 elements the sequence order is not relevant.
               * It is possible that the tag belongs to an upper level 
               * composition of elements. 
               * Restore the old bitposition (before the TAG) and return
               * END_OF_COMPOSITION to leave this composition level
               */
              bf_setBitpos (globs->bitpos-8, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Tag may belong to upper ASN.1 comp -> dec. bitpos by 8 to %d", globs->bitpos);
#endif

              return END_OF_COMPOSITION; /* skip the remaining elements in this level */
            }
            else
            {
              ie_amount = 0;
              ccd_setError (globs, ERR_MAX_IE_EXCEED,
                            CONTINUE,
                            (USHORT) t,
                            (USHORT) globs->bitpos-len_t,
                            (USHORT) -1);
            }
          }

          /*
           * The t-component matches with the defined identifier for
           * the actual element definition (e_ref).
           */

          if (globs->SequenceError)
          {
            globs->SequenceError = FALSE;

            if (asn1)
            {
              /* For ASN1 elements the sequence order is not relevant.
               * It is possible that the tag belongs to an upper level 
               * composition of elements. 
               * Restore the old bitposition (before the TAG) and return
               * END_OF_COMPOSITION to leave this composition level
               */
              bf_setBitpos (globs->bitpos-8, globs);
#ifdef DEBUG_CCD
          TRACE_CCD (globs, "Tag may belong to upper ASN.1 comp -> dec. bitpos by 8 to %d", globs->bitpos);
#endif

              return END_OF_COMPOSITION; /* skip the remaining elements in this level */
            }
            else
            {
              /* found an element in wrong sequence */
              
              cdc_skipElem (e_ref, (tlv_inf->gotLen ? len_l:0), globs);
              return (SHORT)(globs->RefBeforeError - e_ref);
            }
          }

          /* 
           * if this element is conditional, check the condition
           */
          if (calcidx[cix_ref].numCondCalcs NEQ 0
            AND ! ccd_conditionOK (e_ref, globs))
          {
            /*
             * if the condition for this element is not valid
             * but this element appears in the message, generate 
             * an error and skip the element
             */
            ccd_setError (globs, ERR_IE_NOT_EXPECTED,
                          CONTINUE,
                          (USHORT) t,
                          (USHORT) globs->bitpos-len_t,
                          (USHORT) -1);

            cdc_skipElem (e_ref, (tlv_inf->gotLen ? len_l:0), globs);

            return 0;
          }

          /*
           * check for a valid index
           */
          if (ie_amount EQ 0)
          {
            /*
             * The max number of repeats are reached
             * In this case we must skip this element.
             */
            cdc_skipElem (e_ref, (tlv_inf->gotLen ? len_l:0), globs);

            return 0;
          }

     
          if (iei_tbl->multiple)
          {
            if (melem[e_ref].elemType NEQ 'S')
            {
              /*
               * Element is not a SPARE 
               * Setup the offset into the C-structure for this element
               */
              globs->pstructOffs = melem[e_ref].structOffs;
            }
            ret = cdc_tagged_LV_decode (e_ref, ie_amount,
                                       (tlv_inf->gotLen ? len_l:0), globs);
          }
          else
          {
            if (melem[e_ref].elemType NEQ 'S')
            { /*
               * Element is not a SPARE 
               * Setup the offset into the C-structure for this element
               */
              globs->pstructOffs = melem[e_ref].structOffs;
            }
            ret = cdc_normal_LV_decode (e_ref, 
                                       (tlv_inf->gotLen ? len_l:0), globs);
          }
          globs->SeekTLVExt = TRUE;
          iei_tbl->exhausted  = TRUE;

          if (ret)
            return END_OF_COMPOSITION;

          /*
           * if more then one IE of this type are allowed, a ret of 0
           * indicates the calling function (ccd_decodeComposition())
           * to leave the pointer to the actual element definition on
           * this element. If the value of ret is greater then 0 the
           * calling function will increment the pointer by the value
           * of ret.
           * cdc_T_decode() has found the expected element definition.
           * Go to the next definition or stay at this definition,
           * if the occurance of this element is more than one. 
           */
          if (iei_tbl->act_amount > iei_tbl->max_amount)
          {
            iei_tbl->act_amount = 0;
          }
          if (iei_tbl->max_amount > 1)
          {
            return (0);
          }
          else 
          {
            if (globs->iei_ctx[globs->ccd_recurs_level].countSkipped)
            {
              ret = (-1) * (globs->iei_ctx[globs->ccd_recurs_level].countSkipped);
              (globs->iei_ctx[globs->ccd_recurs_level].countSkipped) = 0;
              return (ret);
            }
            else 
            {
              return (1);
            }
          }
        } /* IdxOffset == 0 */
      } /* tag found */
    }  /* no asn1, no EOC */
  } /* got tag */
  else
  {
    /* 
     * element has no t-component, process the l- and V-components
     */
    if (melem[e_ref].elemType NEQ 'S')
    { /*
       * Element is not a SPARE 
       * Setup the offset into the C-structure for this element
       */
      globs->pstructOffs = melem[e_ref].structOffs;
    }
    ret = cdc_normal_LV_decode (e_ref, len_l, globs) ? END_OF_COMPOSITION : 1;
    globs->SeekTLVExt = TRUE;
    return ret;
  }
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_tlv_encode      |
+--------------------------------------------------------------------+

  PURPOSE : if T_len > 0 this function encodes the T-Component of
            this element. If L_len > 0 it encodes the number
            of bytes uses for this element. After all the function
            encodes the V-component referenced by eRef from the
            C-Structure (globs->pstruct) at position globs->pstructOffs
            into the bitstream.

*/

void cdc_tlv_encode  (const ULONG  e_ref,
                            UBYTE  lenT,
                            UBYTE  lenL,
                            T_CCD_Globs *globs)
{
  ULONG  posL=0, t_repeat, v_repeat, repeat;
  ULONG  cSize, startOffset=0;
  BOOL   multAppear;
  U8     *old_pstruct = NULL;
  ULONG  i;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

  cix_ref = melem[e_ref].calcIdxRef;
  num_prolog_steps = calcidx[cix_ref].numPrologSteps;
  prolog_step_ref  = calcidx[cix_ref].prologStepRef;

  /*
   * If this element is conditional, check the condition.
   */
  if (calcidx[cix_ref].numCondCalcs NEQ 0
  AND ! ccd_conditionOK (e_ref, globs))
    return;

  /*
   * If this element have a defined Prolog,
   * we have to process it before decoding the bit stream.
   */
  if (num_prolog_steps)
  {
    ccd_performOperations (num_prolog_steps, prolog_step_ref, globs);
  }
  
  if (melem[e_ref].elemType NEQ 'S')
  {
    /*
     * Element is not a SPARE.
     * Setup the offset into the C-structure for this element.
     * In case of pointer types, the pstructOffs must be
     * the offset into the memory area pointed to. CCDGEN must
     * ensure this holds true.
     */
    globs->pstructOffs = melem[e_ref].structOffs;

    if ( ! cdc_isPresent(e_ref, globs) )
      return;

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
        repeat     = (ULONG) MINIMUM (globs->pstruct[globs->pstructOffs],
                                      melem[e_ref].maxRepeat);
        if ( repeat < (ULONG) (globs->pstruct[globs->pstructOffs]) )
          ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, (USHORT) e_ref, 
                           globs->pstruct + globs->pstructOffs);
      }
                         
      globs->pstructOffs++;

      multAppear = (melem[e_ref].repType EQ 'i');
    }
    else
    {
      /*
       * Field of constant length: repType EQ 'c'  
       * or bit-field allocated with 
       * given maximum length: repType EQ 'b' (often cType='X')
       */
      repeat = (ULONG)((melem[e_ref].repType EQ 'c' 
                         OR melem[e_ref].repType EQ 'b')
                        ? melem[e_ref].maxRepeat 
                        : 1 );
      multAppear = FALSE;
    }

    /*
     * Perform pointer dereference for pointer types.
     * Also, check optionality for these types.
     */
#ifdef DYNAMIC_ARRAYS
    if ((melem[e_ref].elemType >= 'P' AND melem[e_ref].elemType <= 'R') OR
	(melem[e_ref].elemType >= 'D' AND melem[e_ref].elemType <= 'F')) 
    {
      U8 *deref_pstruct;

      /* Get pointer value */
      deref_pstruct = *(U8 **)(globs->pstruct + globs->pstructOffs);

      /*
       * Strictly speaking the 'D' to 'F' types should not need this
       * check (should have returned after the optionality check above),
       * but it will catch stray NULL pointers (or uninitialized
       * valid flags)
      */
      if (ccd_check_pointer(deref_pstruct) != ccdOK)
      {
        ccd_recordFault (globs, ERR_INVALID_PTR, BREAK, (USHORT) e_ref, 
                         &globs->pstruct[globs->pstructOffs]);
        return;
      }

      /*
       * Pointer not NULL;
       * 1. Save old globs->pstruct and assign pointer to globs->pstruct
       *    as new base.
       * 2. Set pstructOffs to 0 (zero) as the next offset will start
       *    in the new memory area.
       */
      old_pstruct        = globs->pstruct;
      globs->pstruct     = deref_pstruct;
      globs->pstructOffs = 0;
    }
#endif

    /*
     * 20010621 MVJ: Dynamic array addition.
     * Types 'R' and 'F' point to base types (just as type 'V') and
     * read sizes from the same table.
     */
    cSize = (ULONG)((melem[e_ref].elemType EQ 'V'
#ifdef DYNAMIC_ARRAYS
                      OR melem[e_ref].elemType EQ 'R'
                      OR melem[e_ref].elemType EQ 'F'
#endif
                     ) ? mvar[melem[e_ref].elemRef].cSize
                     : mcomp[melem[e_ref].elemRef].cSize
                     );

    startOffset = globs->pstructOffs;
  }
  else
  {
    repeat = (ULONG)((melem[e_ref].repType EQ 'c') 
                      ? melem[e_ref].maxRepeat
                      : 1);

    multAppear = FALSE;

    cSize = 0;
  }

  if (multAppear AND lenT)
  {
    /*
     * multiple appearance of the repeated element is coded as 
     * TLV0 TLV1 TLV2 ....
     */
    t_repeat = repeat;
    v_repeat = 1;
  }
  else
  {
    t_repeat = 1;
    v_repeat = repeat;
  }

  /*
   * single appearance of the repeated element is coded as 
   * TLV0V1V2V3 ....
   */

  for (i=0; i < t_repeat; i++)
  {
    if (lenT)
    {
      /*
       * encode the T-component
       */
      bf_codeByteNumber (lenT, (UBYTE) melem[e_ref].ident, globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "encoding %d bits T-value (%x)", lenT, melem[e_ref].ident);
#endif
    }

    /*
     * if lenL > 0 remember the position of the L-component, because
     * we know it after encoding the entire element. for GSM5TLV elements
     * it could be necessary to use 2 bytes for the length information.
     */
    if (lenL)
    {
      posL = (ULONG) globs->bitpos;
      bf_incBitpos (lenL, globs);
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "skipping %d bits for L-value at byte %d.%d", lenL, globs->bytepos, globs->byteoffs);
#endif
    }

    if (cSize)
    {
      /*
       * calculate the offset if it is not a spare
       */
      globs->pstructOffs = (ULONG)(startOffset + (i * cSize));
    }

    /*
     * Encode the value. Keep caution with BER encoding of ASN1 integers.
     * All other types can be encoded by a generic function.
     */
    if (melem[e_ref].codingType EQ CCDTYPE_GSM1_ASN
        AND 
        melem[e_ref].elemType EQ 'V'
        AND melem[e_ref].repType EQ ' '
       )
    {
#ifdef DEBUG_CCD
#ifdef CCD_SYMBOLS
      TRACE_CCD (globs, "BER encoding of ASN.1 integer %s",
               ccddata_get_alias((USHORT) e_ref, 1));
#else
      TRACE_CCD (globs, "BER encoding of ASN.1 integer; e_ref= %d", melem[e_ref].elemRef);
#endif
#endif

      switch (mvar[melem[e_ref].elemRef].cType)
      {
        case 'B': bf_writeBits (8, globs);
          break;
        case 'S':
        {
          if (*(U16 *) (globs->pstruct+globs->pstructOffs) <= (U16)0xFF)
            bf_writeBits (8, globs);
          else
            bf_writeBits (16, globs);
        }
          break;
        case 'L': 
        {          
          U32 tmpVal= *(U32 *) (globs->pstruct+globs->pstructOffs);
        
          if ( tmpVal <= (U32)0xFF)
            bf_writeBits (8, globs);
          else if ( tmpVal <= (U32)0xFFFF)
            bf_writeBits (16, globs);
          else if ( tmpVal <= (U32)0xFFFFFF)
            bf_writeBits (24, globs);
          else
            bf_writeBits (32, globs);
        }
          break;
        case 'X': 
        {   
          U16 ValLen= *(U16 *) (globs->pstruct+globs->pstructOffs);
          
          if ( mvar[melem[e_ref].elemRef].bSize >= ValLen)
            bf_writeBitChunk (ValLen, globs);
          else
          {
#ifdef DEBUG_CCD
            TRACE_CCD (globs, "value length (%d) exceeds defined bSize!", ValLen);
#endif
          }

        }
          break;
      }
    }
    else
    {
      cdc_encodeElemvalue (e_ref, v_repeat, globs);
    }

    /*
     * calculate the bitlen if it is an TLV element and write the
     * L-value.
     */

    if (lenL)
    {
      switch (melem[e_ref].codingType)
      {
        case CCDTYPE_GSM5_TLV:
        {
          USHORT L = (((USHORT)((globs->bitpos - posL)-lenL)+7) >> 3);


#ifdef DEBUG_CCD
          TRACE_CCD (globs, "recoding the 8 bit L-value (%d)", L);
#endif

          if (L > 127)
          {
            /*
             * if the length is > 127 we code the first byte to
             * 0x81, shift the whole stuff rightwise by 8 and 
             * encode the length in the next byte (16 bits for L)
             */
            bf_rShift8Bit ((USHORT) (posL+8), (USHORT) (L<<3), globs);
            bf_incBitpos (8, globs);
            bf_recodeByteNumber ((USHORT) posL, lenL, (UBYTE) 0x81, globs);
            bf_recodeByteNumber ((USHORT) (posL+8), lenL, (UBYTE) L, globs);
            /*
             * set the bitpos to a 8 bit aligned position
             * corresponding the L value
             */
            bf_setBitpos (posL+(L*8)+16, globs);
          }
          else
          {
            bf_recodeByteNumber ((USHORT) posL, lenL, (UBYTE) L, globs);
            /*
             * set the bitpos to a 8 bit aligned position
             * corresponding the L value
             */
            bf_setBitpos (posL+(L*8)+8, globs);
          }
          break;
        }

        case CCDTYPE_GSM6_TLV:
        {
          USHORT L = ((USHORT)(((globs->bitpos - posL)-lenL)+7) >> 3);

#ifdef DEBUG_CCD
          TRACE_CCD (globs, "recoding the 16 bit L-value (%d)", L);
#endif
          bf_recodeShortNumber ((USHORT)posL, lenL, L, globs);
          /*
           * set the bitpos to a 8 bit aligned position
           * corresponding the L value
           */
          bf_setBitpos (posL+(L*8)+16, globs);
          break;
        }

        case CCDTYPE_GSM7_LV:
        {
          USHORT L = (USHORT) ((globs->bitpos - posL)-lenL);

#ifdef DEBUG_CCD
          TRACE_CCD (globs, "recoding the 7 bit L-value (bitlength) (%d)", L);
#endif
          bf_recodeShortNumber ((USHORT)posL, lenL, L, globs);

          bf_setBitpos (posL+L+7, globs);
          break;
        }

        default:
        {
          USHORT L = ((USHORT)(((globs->bitpos - posL)-lenL)+7) >> 3);

#ifdef DEBUG_CCD
          TRACE_CCD (globs, "recoding the 8 bit L-value (%d)", L);
#endif
          bf_recodeByteNumber ((USHORT)posL, lenL, (UBYTE) L, globs);
          /*
           * Set the bitpos to a 8 bit aligned position
           * corresponding the L value
           */
          bf_setBitpos (posL+(L*8)+8, globs);
          break;
        }
      }
    }
  }

  /*
   * Restore globs->pstruct if overwritten by pointer dereference.
   */
  if (old_pstruct)
    globs->pstruct = old_pstruct;
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CDC_COM             |
| STATE   : code                       ROUTINE : cdc_GSM_start       |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the GSM specific codec part for each msg. 

*/

void cdc_GSM_start (T_CCD_Globs *globs)
{
  globs->Swap1V_inProgress = FALSE;
  globs->last_level       = 255;
  cdc_init_ctx_table (globs);
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : cdc_isPresent          |
+--------------------------------------------------------------------+

  PURPOSE : For optional elements check the valid-flag in the C-struct.
            Spare elements in PER do not have a corresponding valid flag.
            In case of Dynamic Arrays:
            Postpone optional check for non-code transparent pointer
            types ('P', 'Q', 'R').
            For these types, the optional flag is the pointer itself.
            These types cannot be checked yet, as the pointer may be
            preceeded by a counter octet, a union tag id octet etc.
*/
U16 cdc_isPresent (const ULONG e_ref, T_CCD_Globs *globs)
{
  if (melem[e_ref].optional)
  {
#ifdef DYNAMIC_ARRAYS
    if (melem[e_ref].elemType < 'P' OR melem[e_ref].elemType > 'R')
    {
      if(globs->pstruct[globs->pstructOffs++] == FALSE)
        return FALSE;
#ifdef DEBUG_CCD
      else if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
      {
        TRACE_CCD (globs, "Ambiguous value for valid flag!\n...assumed 1 for ccdID=%d",
                   e_ref);
      }
#endif
    }
    else
    { /*If elemType is P, Q or R - check the pointer value*/
      if(*(void**) &globs->pstruct[globs->pstructOffs] == NULL)
        return FALSE;
    }
#else
    if (globs->pstruct[globs->pstructOffs++] == FALSE)
      return FALSE;
#ifdef DEBUG_CCD
    else if (globs->pstruct [melem[e_ref].structOffs] != TRUE)
    {
      TRACE_CCD (globs, "Ambiguous value for valid flag!\n...assumed 1 for ccdID=%d",
                 e_ref);
    }
#endif
#endif
  }
  return TRUE;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : is_pointer_type        |
+--------------------------------------------------------------------+

  PURPOSE : Return TRUE for pointer elements.

*/
BOOL is_pointer_type (const ULONG e_ref)
{
  return ((melem[e_ref].elemType >= 'P' AND melem[e_ref].elemType <= 'R') OR
	  (melem[e_ref].elemType >= 'D' AND melem[e_ref].elemType <= 'F'));
}
#endif /* !RUN_FLASH */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : is_variable_type       |
+--------------------------------------------------------------------+

  PURPOSE : Return TRUE for elements with variable character.

*/
BOOL is_variable_type (const ULONG e_ref)
{
  return ((melem[e_ref].elemType == 'F') || ( melem[e_ref].elemType == 'R') ||
	  (melem[e_ref].elemType == 'V'));
}
#endif /* !RUN_FLASH */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : PER_CommonBegin        |
+--------------------------------------------------------------------+

  PURPOSE : Common settings done by most of the encoding or decoding 
            functions for UNALIGNED PER (UMTS).
            It handles position of pointer to the C-structure, 
            valid flag for optional elements and length determinant 
            for array of elements.
*/
SHORT PER_CommonBegin (const ULONG e_ref, ULONG *max_rep, T_CCD_Globs *globs)
{
  /*
   * Set the offset in the C-structure on the value for this element
   */
  globs->pstructOffs = melem[e_ref].structOffs;

  /* For optional elements we have already set the valid flag in the
   * C-structure while processing ASN1_SEQ.
   */
  if ( ! cdc_isPresent(e_ref, globs) )
    return (SHORT)ccdError;
   
  switch (melem[e_ref].repType)
  {
    case ' ':
      /*
       * Element is not an array.
       */
      *max_rep = 1;
      break;
    case 'c':
    case 'C':
      /*
       * Read the size for an array of fixed length.
       */
      *max_rep = (ULONG) melem[e_ref].maxRepeat; 
      break;                
    case 'j':
    case 'J':
    {
      /*
       * Read the size for an array of variable length.
       * Read the value of the last encoded element. It is the length
       * indicator.
       * Hint 1: globs->pstruct[melem[e_ref-1].structOffs is 0, since
       *   fields of variable length are projected on a COMP made of an
       *   ASN1_INTEGER for the lenght indicator and the field elements
       *   (sequences, integers, octets or bits). 
       * Hint 2: The current version of UMTS does not use length
       *   indicators larger than 64K. Hence the use of USHORT for repeat.
       */
      switch (mvar[melem[e_ref-1].elemRef].cType)
      {
      case 'B':  *max_rep = (ULONG)  globs->pstruct[melem[e_ref-1].structOffs];
      	         break;
      case 'S':  *max_rep = (ULONG) *(USHORT *) (globs->pstruct+melem[e_ref-1].structOffs);
	               break;
      default:   *max_rep = 0;
	               break;
      }
      break;
    }
    default:
      ccd_recordFault (globs, ERR_DEFECT_CCDDATA, BREAK, (USHORT) e_ref,
                     globs->pstruct + globs->pstructOffs);
      break;
  }

  /*
   * There is nothing to be encoded. 
   */
  if (*max_rep EQ 0)
  {
    return (SHORT)ccdError;
  }
  /* 
   * Check the validity of the lenght information.
   */
  else if (melem[e_ref].maxRepeat AND *max_rep > melem[e_ref].maxRepeat)
  {
    ccd_recordFault (globs, ERR_MAX_REPEAT, CONTINUE, (USHORT) e_ref,
                     globs->pstruct + globs->pstructOffs);
  }

  return (SHORT)ccdOK;
}
#endif /* !RUN_INT_RAM */


#ifdef DYNAMIC_ARRAYS
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : PER_allocmem           |
+--------------------------------------------------------------------+

  PURPOSE : Allocate memory for pointer types (dynamic array addition)
            Returns address of freshly allocated memory
            or ccdError in case no memory is available.
*/
U8 *PER_allocmem(const ULONG e_ref, ULONG repeat, T_CCD_Globs *globs)
{
  /*
   * Check for pointer types; allocate memory if necessary.
   */
  if ( is_pointer_type(e_ref) ) {
    ULONG   cSize;
    U8     *addr;

    /*
     * Find size to allocate.
     * Read from mcomp or mvar according to type.
     */
    cSize = (ULONG)((melem[e_ref].elemType EQ 'V' OR
		     melem[e_ref].elemType EQ 'R'
		     OR melem[e_ref].elemType EQ 'F')
		    ? mvar[melem[e_ref].elemRef].cSize
		    : mcomp[melem[e_ref].elemRef].cSize
		    );

#ifdef DEBUG_CCD
    TRACE_CCD (globs, "PER_allocmem(): alloc%5d x%5d bytes (type '%c'); "
	       "elem%5d ('%s')",
	       repeat, cSize, melem[e_ref].elemType, e_ref,
#ifdef CCD_SYMBOLS
	       mcomp[melem[e_ref].elemRef].name
#else
	       ""
#endif
	       );
#endif
    /*
     * Allocate additional memory - append to existing mem chain
     */

    cSize *= repeat;
    addr = (U8 *)DP_ALLOC( cSize, globs->alloc_head, DP_NO_FRAME_GUESS);

    /* If no memory, log error and return immediately */
    if (addr EQ NULL) {
      ccd_setError (globs, ERR_NO_MEM,
                    BREAK,
                    (USHORT) -1);
      return (U8 *)ccdError;
    }
    else
      memset (addr, 0, (size_t)cSize);
    return addr;
  }
  return (U8 *)ccdError;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : PER_allocmem_and_update|
+--------------------------------------------------------------------+

  PURPOSE : Allocate memory for pointer types (dynamic array addition)
            Updates global variables after allocation
            (globs->pstruct and globs->pstructOffs).
            Assumes that these global variables are saved by the
            calling function.
            Returns ccdOK or ccdError in case no memory is available.
*/
USHORT PER_allocmem_and_update(const ULONG e_ref, ULONG repeat, T_CCD_Globs *globs)
{
  U8    *addr;

  /* Allocate memory */
  addr = PER_allocmem(e_ref, repeat, globs);

  /* No memory ? */
  if ( addr != (U8 *)ccdError ) {
    /*
     * Memory allocated;
     * 1. Store pointer to freshly allocated memory area in structure
     * 2. Initialize pstruct to point to the freshly allocated memory area.
     * 3. Initialize pstructOffs to 0 to start decoding at offset 0
     *    in the new memory area.
     * Assumes that globs->pstruct is saved in the calling function.
     */
    *(U8 **)(globs->pstruct + globs->pstructOffs) = addr;
    globs->pstruct     = addr;
    globs->pstructOffs = 0;
    return ccdOK;
  } else {
    /* No memory - Return error */
    return ccdError;
  }
}
#endif /* !RUN_INT_RAM */
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code       ROUTINE : Read_NormallySmallNonNegativeWholeNr|
+--------------------------------------------------------------------+

  PURPOSE : Read a normally small non-negative whole number as defined
            by ASN.1 PER. Function is used to read elements such as:
			a) bit-map field of SEQUENCE extensions,
			b) index of CHOICE extension or
			c) extension value of extensible INTEGER or ENUMERATED.
*/
U32 Read_NormallySmallNonNegativeWholeNr (T_CCD_Globs *globs)
{
  U32 value_length=0;

  /* Read the first bit. If set to 0 it means the value is encoded
   * in the following five bits. Else read a normally small ...nr.
   */
  if (bf_readBit (globs) EQ 0)
  {
    return ((U32) bf_getBits (6, globs));
  }
  else 
  {
    /*
     * Do not handle the theoretical case that value length 
     * needs more than 63 bits.
     */
    bf_incBitpos (1, globs);

    /* 
     * Read the value length first. 
     * Then use the length to read the value.
     */
    value_length = (U32) bf_getBits (6, globs);
    return ((U32) bf_getBits (value_length, globs));
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code      ROUTINE : Write_NormallySmallNonNegativeWholeNr|
+--------------------------------------------------------------------+

  PURPOSE : Write a normally small non-negative whole number as defined
            by ASN.1 PER. Function is used to encode elements such as:
			a) bit-map field of SEQUENCE extensions,
			b) index of CHOICE extension or
			c) extension value of extensible INTEGER or ENUMERATED.
*/
void Write_NormallySmallNonNegativeWholeNr (U32 Value, T_CCD_Globs *globs)
{
  /* For small numbers write 0 in the first bit. 
   * Then encode that number in the succeeding five bits.
   */
  if (Value < 64)
  {
  	bf_writeBit (0, globs);
	  bf_writeVal (Value, 6, globs);
  }
  /*
   * Encode the number under the assumption that its length is 
   * given by less than 63 bits. Hence encode also the length as a 
   * normally small...
   */
  else 
  {
	  /* Set flag bits:
	   * 1 means "length determinant encoded before the value itself"
	   * 0 means "length determinant encoded only in five bits"
	   */
	  bf_writeVal (2, 2, globs);
	  bf_writeVal (bitSize[Value], 5, globs);

	  /* Encode the number itself */
	  bf_writeVal (Value, bitSize[Value], globs);
  }

  return;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : Read_OpenTpye_Length   |
+--------------------------------------------------------------------+

  PURPOSE : Read length of an ASN.1 open type.
            Open types are normally found in encoding of 
            parametrized information objects and extension types.
*/
U32 Read_OpenTpye_Length (T_CCD_Globs *globs)
{
  U32 Value;

  /* 
   * Flag bit is 0 for "Value < 128" which means
   * "encoding fits in the current octet"
   */
  if (bf_readBit (globs) EQ 0)
  {
	  Value = bf_getBits (7, globs);
  }
  /* 
   * Flag bits are 10 for 128 "< Value < 16K".
   * 1 means "encoding does not fit in the current octet".
   * 0 means "encoding needs only one further octet".
   */
  else if (bf_readBit (globs) EQ 0)
  {
  	Value = bf_getBits (14, globs);
  } 
  /* Currently no support for bigger values is required. */
  else 
  {
    /* force error detection */
    Value = 0;
  }

  return Value;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)              MODULE  : CDC_COM                |
| STATE   : code                    ROUTINE : Write_OpenTpye_Length  |
+--------------------------------------------------------------------+

  PURPOSE : Write length of an ASN.1 open type.
            Open types are normally found in encoding of 
            parametrized information objects and extension types.
*/
void Write_OpenTpye_Length (U32 Value, T_CCD_Globs *globs)
{

  if (Value < 128)
  {
	  bf_writeVal (Value, 8, globs);
  }
  else if (Value < 0x8000)
  {
	  /* Set flag bits:
	   * 1 means "encoding does not fit in the current octet"
	   * 0 means "encoding needs only one further octet"
	   */
	  bf_writeVal (2, 2, globs);
	  bf_writeVal (Value, 14, globs);
  } 
  /* Currently no support for bigger values is required. */
  else 
  {}

  return;
}
#endif /* !RUN_INT_RAM */

/*
+-----------------------------------------------------------------------------
|  Project : CCD
|  Modul   : freq_list_com.c
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
|  Purpose :  Definitions of common functions for decoding of types FDD_CI,
|             TDD_CI and FREQ_LIST.
+-----------------------------------------------------------------------------
*/

#define CDC_FREQ_LIST_COM_C

/*
 * standard definitions like GLOBAL, UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"

/*
 * Types and functions for bit access and manipulation
 */
#include "ccd_globs.h"
#include "bitfun.h"

/*
 * Error codes and prototypes of exported functions by CCD
 */
#include "ccdapi.h"

/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"

#ifndef RUN_INT_RAM
U8 ByteBitMask[]= {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x1};
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
 * The following table indicates the number of W-parameters and their
 * length in bits. A length of zero indicates the end of the table.
 * For frequency lists the W-parameter in the 1024 range starts from 
 * bit 6 of the information element.
 */
const T_W_PARAM param_1024[9] =
{
 /*
  * length       count
  */
      10,          1,
       9,          2,
       8,          4,
       7,          8,
       6,          16,
       5,          32,
       4,          64,
       3,          128,
       0,          0
};
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
 * The following table indicates the number of W-parameters and their
 * length in bits. A length of zero indicates the end of the table.
 * For frequency lists the W-parameter in the 512 range starts from 
 * bit 7 of the information element.
 */
const T_W_PARAM param_512[10] =
{
 /*
  *  length     count
  */
       10,        1,
        9,        1,
        8,        2,
        7,        4,
        6,        8,
        5,       16,
        4,       32,
        3,       64,
        2,      128,
        0,        0
};
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD            FUNCTION : for_modulo                     |
+--------------------------------------------------------------------+

  PURPOSE : A modulo calculation function. The standard C-Operator
            fails for negative values! (e.g. -4 mod 6 is 2 and not 4).

*/

/* static */ long for_modulo (long a, long b)
{
  long result;

  /* Use standard C-Operator for calculation. */
  result = a % b;

  /* Correct the result for negative values. */
  if (result < 0)
  {
    result += b;
  }

  return result;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD            FUNCTION : for_smodulo                    |
+--------------------------------------------------------------------+

  PURPOSE : Similar to the modulo operator, but 0 smod n is n and
            not 0. Same problem for negative values with the standard
            C-Operator.

*/
static long for_smodulo (long a, long b)
{
  long result;

  /* Use standard C-Operator for calculation. */
  result = a % b;

  /* Correct the result for negative values. */
  if (result < 0)
  {
    result += b;
  }

  /* Special handling for result equal 0 */
  if (result EQ 0)
  {
    result = b;
  }

  return result;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+--------------------------------------------------------------------+
| PROJECT : CCD            FUNCTION : for_get_generation             |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the greatest power of 2 of the given
            value. The algorithm simply looks to the position of the
            highest bit.

*/

static U16 for_get_generation (U16 value)
{
  int result = 0;
  int i;


  /* Check all 16 bit positions. */
  for (i = 0; i < 16; i++)
  {
    /* If bit is set, store the position. */
    if (value & 1)
    {
      result = i + 1;
    }

    /* Shift value to have the next bit for comparision. */
    value = value >> 1;
  }

  /* Return the highest position. */
  return result;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD            FUNCTION : cdc_decode_frequencies         |
+--------------------------------------------------------------------+

  PURPOSE : The algorithm is according GSM 4.08 Annex J. It calculates
            a frequency hopping list from the W-parameter.

*/
void cdc_decode_frequencies (short        original_range,
                             short       *w,
                             short        offset,
                             U8           callerID,
                             T_CCD_Globs *globs)
{
  short g;
  short k;
  short j;
  short index;
  short n;
  short range;
  U16   channel;
  U16   bitposition;
  U8   *BitmapInStruct = globs->pstruct + globs->pstructOffs;
  U16 first = 0;
  U16 last  = BITOFFSET_LIST;
  U16 num   = 0;
  BOOL ReadW = TRUE;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_decode_frequencies()");
#endif

  if (callerID != TDD_CI_LIST && w[0] == 0)
    ReadW = FALSE;
  for (k = 1; ReadW; k++)
  {
    ReadW = (w[k-1] != 0) ? 1 : 0;
    /*
     * The next loop follows the tree from child to parent,
     * from the node of index K to the root (index 1). For each iteration the
     * node of index INDEX is tackled. The corresponding range is RANGE, and N
     * is the value of the element in the range defined by the node.
     *
     * The data are set to their initial values
     */
    index = k;
    n = w[index-1];
    g = for_get_generation (index);
    j = (1 << (g-1));
    range = original_range / j;

    while (index > 1)
    {
      /*
       * Due to the assumption that the original range is a power of two minus one,
       * the range for the parent node can be easily computed, and does not depend
       * upon whether the current node is a left or right child
       */
      g     = for_get_generation (index);
      j     = (1 << (g-1));
      range = 2 * range + 1;

      /*
       * Let us note J := 2 g-1 , g being the generation of node INDEX. We have J =
       * GREATEST_POWER_OF_2_LESSER_OR_EQUAL_TO(INDEX). The numbering used in the tree
       * is such that the nodes of index J to J + J/2 - 1 are left children, and the nodes
       * of index J/2 to J+J-1 are right children. Hence an easy test to
       * distinguish left and right children:
       */
      if (2 * index < 3 * j)
      {
        /*
         * The next computation gives the index of the parent node of the node of index
         * INDEX, for a left child :
         */
        index = index - j / 2;

        /*
         * The next formula is the inverse of the renumbering appearing in the encoding
         * for a left child. It gives the value of the parent node in the range defined
         * by the grand-parent node:
         */
        n = (short)for_smodulo (n + w[index-1] + (range-1) / 2, range);
      }
      else
      {
        /*
         * The next computation gives the index of the parent node of the node of index
         * INDEX, for a right child :
         */
        index = index - j;

        /*
         * The next formula is the inverse of the renumbering appearing in the encoding
         * for a right child:
         */
        n = (short)for_smodulo (n + w[index-1], range);
      }
    }

    /*
     * Write the calculated number for non-frequency types.
     * For TDD_CI and TDD_CI: offset = 0 and original_range = 1023.
     */
    channel = (U16)for_modulo (n+offset, 1024);
    if (callerID == FDD_CI_LIST || callerID == TDD_CI_LIST)
    {
      *(U16*)(globs->pstruct + globs->pstructOffs) = (U16)channel;
      globs->pstructOffs += 2;
    }
    /* Set the calculated channel number for frequency channel list.*/
    else
    {
      if (channel == 0)
      {
        bitposition = 0;
      }
      else
      {
        bitposition = (U16)(BITOFFSET_LIST - channel);
      }
      if (first > bitposition)
        first = bitposition;
      if (last < bitposition)
        last = bitposition;
      num++;
      BitmapInStruct[bitposition >> 3] |= ByteBitMask[bitposition & 7];
    }
  }

  /* For the bitmap type print the helpful information into the structure. */
  if (callerID == FREQUENCY_LIST)
  {
    *(U16*) (BitmapInStruct - 6) = first;
    *(U16*) (BitmapInStruct - 6) = last;
    *(U16*) (BitmapInStruct - 2) = num;
  }
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD            FUNCTION : cdc_decode_param               |
+--------------------------------------------------------------------+

  PURPOSE : The information element contains a list of W-parameter.
            The table param indicates how many W-parameter from each
            length shall be inside. The function converts the bitstream
            of the W-parameter to an array of W-parameter 16 bit values.

*/

void cdc_decode_param (const T_W_PARAM *param,
                       short           *w,
                       U16              ListLength,
                       T_CCD_Globs     *globs)
{
  U8  end_detected = FALSE;
  U16 w_index      = 0;
  U16 length       = ListLength;
  U16 act_length   = param->length;
  U16 act_counter  = param->count;
  U32 lvalue;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_decode_param()");
#endif

  /*
   * Decode values in the list until the end of the IE is detected.
   */
  while (!end_detected)
  {
    /*
     * If the length of the next W-parameter is greater than eight bits,
     * use ccd_decodeLong function. For smaller length use the 
     * ccd_decodeByte function to extract the W-parameter from the bitstream.
     */
    lvalue = bf_getBits (act_length, globs);
    w[w_index++] = (short)lvalue;

    /*
     * w = 0 is equal to end of list if it is not the w(0) !!!
     * (The case w(0)=0 possible for frequency list, but maybe not for other
     * cases this algorithm is invoked.
     */
    if (w_index != 1 && w[w_index-1] == 0)
    {
      end_detected = TRUE;
    }

    /* End of buffer is equal to end of list. */
    if (length > act_length)
    {
      length -= act_length;
    }
    else
    {
      end_detected = TRUE;
    }

    /* Check if all w parameters of one size are read. */
    if (--act_counter == 0)
    {
      param++;
      act_length   = param->length;
      act_counter  = param->count;
    }
    /* End of parameter table */
    if ((act_length == 0) || (length < act_length))
    {
      end_detected = TRUE;
    }
  }

  /* Add an end identifier. */
  w[w_index++] = 0;
}
#endif /* !RUN_INT_RAM */

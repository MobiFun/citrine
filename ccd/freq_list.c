/*
+-----------------------------------------------------------------------------
|  Project : CCD
|  Modul   : freq_list.c
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
|  Purpose :  Definition of decoding function for FREQ_LIST type
+-----------------------------------------------------------------------------
*/

#define FREQ_LIST_C

/*
 * standard definitions like GLOBAL, UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"

#include "string.h"

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
/*
 * Function prototypes of CCD-CCDDATA interface 
 */
#include "ccddata.h"


#if !(defined (CCD_TEST))
#include "vsi.h"
#endif

#ifndef RUN_INT_RAM
/*
 * Attention: in this file all static and global functions as well as
 * the static data are controlled by one RUN_.. clause.
 * The static data and functions are currently only used in the decode
 * function, whereas encoding is not yet available.
 */
static U16 first;
static U16 last;
static U16 num;


/*
 * The W-parameter in the 256 range start from bit 7 of the
 * information element. The following table indicate the number
 * of W-parameters and their length in bits. A length of zero
 * indicated the end of the table.
 */
static const T_W_PARAM param_256[10] =
{
 /*
  *  length     count
  */
       10,        1,
        8,        1,
        7,        2,
        6,        4,
        5,        8,
        4,       16,
        3,       32,
        2,       64,
        1,      128,
        0,        0
};

/*
 * The W-parameter in the 128 range start from bit 7 of the
 * information element. The following table indicate the number
 * of W-parameters and their length in bits. A length of zero
 * indicated the end of the table.
*/

static const T_W_PARAM param_128[9] =
{
 /*
  *  length     count
  */
       10,        1,
        7,        1,
        6,        2,
        5,        4,
        4,        8,
        3,       16,
        2,       32,
        1,       64,
        0,        0
};

/*
 * Compare results of the first channel with those of the others
 * to find the dimensions.
 */
static void completeAddInfo (U8 *BitmapInStruct)
{
  *(U16*) (BitmapInStruct - 2) += num;
  if (*(U16*) (BitmapInStruct - 6) > first)
    *(U16*) (BitmapInStruct - 6) = first;
  if (*(U16*) (BitmapInStruct - 4) < last)
    *(U16*) (BitmapInStruct - 6) = last;
}

static void setFirstChanNr (U8 *BitmapInStruct, short w0)
{
  U16 bitposition;
  bitposition = (w0 == 0) ? 0 : (U16)(BITOFFSET_LIST - w0);
  BitmapInStruct[bitposition >> 3] |= ByteBitMask[bitposition & 7];
  first = bitposition;
  last = bitposition;
  num = 1;
}
/*
+--------------------------------------------------------------------+
| PROJECT : CCD                    MODULE  : cdc_freq_list_decode    |
+--------------------------------------------------------------------+
  PURPOSE : The function creates a frequency hopping list from one of
            the following information elements according GSM 4.08:

            cell channel description
            frequency list
            frequency short list
            neighbour cell description

            The format identifier of the information element is defined as:

            FORMAT-ID, Format Identifier

            Bit Bit Bit Bit Bit    format notation
             8   7   4   3   2

             0   0   X   X   X     bit map 0
             1   0   0   X   X     1024 range
             1   0   1   0   0     512 range
             1   0   1   0   1     256 range
             1   0   1   1   0     128 range
             1   0   1   1   1     variable bit map

            The space this IE takes in the C-structure is depicted in the
            example below:
            typedef struct
            {
              U16 first;
              U16 last;
              U16 num;
              U8  bitmap[128];
            } BMP_arfcn_list;
            
            The member bitmap stores a list of frequencies in a range of 
            0 - 1023 (GSM), where some of the frequencies are not yet used.
*/

SHORT cdc_freq_list_decode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{
  U8 *FirstByte = globs->bitbuf + globs->bytepos;
  ULONG  cix_ref, num_prolog_steps, prolog_step_ref;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_freq_list_decode()");
#ifdef CCD_SYMBOLS
  TRACE_CCD (globs, "decoding list %s with range 1024 format",
                       ccddata_get_alias((USHORT) e_ref, 1));
#else
  TRACE_CCD (globs, "decoding list %d of range 1024 format", melem[e_ref].elemRef);
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
  
  globs->SeekTLVExt = FALSE;
  globs->pstructOffs = melem[e_ref].structOffs;

  /*
   * Bitmap 0 format
   * only for GSM 900 or GSM 850 bands !!!!
   * std = STD_900, STD_EGSM, STD_DUAL, STD_DUAL_EGSM, STD_850, STD_DUAL_US
   * No check of std for being PCS 1900 or DCS 1800 is implemented.
   */
  if ((*FirstByte & 0x80) == 0)
  {

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "bitmap 0 format");
#endif
    /*
     * Copy only the first 16 bytes. According to section 10.5.2.1b.2 of 
     * GSM04.08, 124 bits are dedicated to ARFCN and two bits to the Format-ID.
     * Two bits are spare.
     */
    FirstByte = globs->pstruct + globs->pstructOffs;
    /* first bit */
    *(U16 *) FirstByte = 896;//T_LIST_MAX_SIZE - 16;
    /* last bit */
    *(U16 *) (FirstByte+2) = T_LIST_MAX_SIZE;
    /* number of entries */
    *(U16 *) (FirstByte+4) = 124;
    memcpy (FirstByte + T_LIST_MAX_SIZE - 10, //FirstByte + 6 + T_LIST_MAX_SIZE - 16,
            globs->bitbuf + globs->bytepos, 16);
  }
  else
  {
    U16 ListLength = mvar[melem[e_ref].elemRef].bSize;
    U8  *BitmapInStruct = globs->pstruct + globs->pstructOffs;
    first = 0;
    last = 0;
    num = 0;
    /*
     * RANGE 128,
     */
    if ((*FirstByte & 0x8E) == 0x8C)
    {
      /*
       * Use dynamic memory for calculation instead of global memory or stack.
       */
      short *w;
      MALLOC (w, 129 * sizeof (U16));

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "range 128 format");
#endif

      /*
       * The algorithm for several ranges is the same with different
       * tables. The W-parameter start with bit 7. Skip over offset.
       */
      bf_incBitpos (7, globs);
      cdc_decode_param (param_128, w, ListLength, globs);

      /*
       * W[0] contains the first channel number
       */
      setFirstChanNr (BitmapInStruct, w[0]);

      /*
       * Decode and set the remaining channel number according the
       * algorithm described in GSM 4.08.
       */
      cdc_decode_frequencies (127, &w[1], w[0], FREQUENCY_LIST, globs);
      completeAddInfo (BitmapInStruct);

      /*
       * free the dynamic allocated memory.
       */
      MFREE (w);
    }
    /*
     * RANGE 256
     */
    if ((*FirstByte & 0x8E) == 0x8A)
    {
      /*
       * Use dynamic memory for calculation instead of global memory or stack.
       */
      short *w;
      MALLOC (w, 257 * sizeof (U16));

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "range 256 format");
#endif

      /*
       * Decode the W-parameter. The W-parameter start with bit 7.
       */
      bf_incBitpos (7, globs);
      cdc_decode_param (param_256, w, ListLength, globs);

      /*
       * W[0] contains the first channel number
       */
      setFirstChanNr (BitmapInStruct, w[0]);

      /*
       * decode and set the remaining channel number according the
       * algorithm described in GSM 4.08.
       */
      cdc_decode_frequencies (255, &w[1], w[0], FREQUENCY_LIST, globs);
      completeAddInfo (BitmapInStruct);

      /*
       * free the dynamic allocated memory.
       */
      MFREE (w);
    }
    /*
     * RANGE 512
     */
    if ((*FirstByte & 0x8E) == 0x88)
    {
      /*
       * Use dynamic memory for calculation instead of global memory or stack.
       */
      short *w;
      MALLOC (w, 257 * sizeof (U16));

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "range 512 format");
#endif

      /*
       * the algorithm for the several ranges is the same with different
       * tables. The W-parameter start with bit 7. Skip over offset.
       */
      bf_incBitpos (7, globs);
      cdc_decode_param (param_512, w, ListLength, globs);

      /*
       * W[0] contains the first channel number
       */
      setFirstChanNr (BitmapInStruct, w[0]);

      /*
       * decode and set the remaining channel number according the
       * algorithm described in GSM 4.08.
       */
      cdc_decode_frequencies (511, &w[1], w[0], FREQUENCY_LIST, globs);
      completeAddInfo (BitmapInStruct);

      /*
       * free the dynamic allocated memory.
       */
      MFREE (w);
    }

    if ((*FirstByte & 0x88) == 0x80)
    {
      /*
       * RANGE 1024
       *
       * Use dynamic memory for calculation instead of global memory or stack.
       */
      U8 f0;
      U8 offset;

      short *w;
      MALLOC (w, 257 * sizeof (U16));

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "range 1024 format");
#endif

      /*
       * Get the f0 indicator. It indicates whether channel 0 is part
       * of the frequency hopping list or not.
       */
      /* The following lines are equal to:
       * ccd_decodeByte (FirstByte, (U16)(globs->byteoffs+5), 1, &f0);
       */
      offset = globs->byteoffs+5;
      if (offset < 8)
      {
        f0 = FirstByte[0] << offset;
        f0 &= 0x80;
      }
      else
      {
        U16 tmpvar;
        tmpvar = *(U16*) FirstByte;
        tmpvar <<= offset;
        f0 = tmpvar & 0x8000;
      }

      /*
       * The algorithm for the several ranges is the same with different
       * tables. The W-parameter start with bit 6. Skip over offset.
       */
      bf_incBitpos (6, globs);
      cdc_decode_param (param_1024, w, ListLength, globs);

      /*
       * If indicated add channel 0 to the list
       */
      if (f0)
      {
        /* The following is equal to setFirstChanNr(0); */
        BitmapInStruct[0] |= 0x80;
        num = 1;
      }

      /*
       * decode and set the remaining channel number according the
       * algorithm described in GSM 4.08.
       */
      cdc_decode_frequencies (1023, &w[0], 0, FREQUENCY_LIST, globs);
      completeAddInfo (BitmapInStruct);

      /*
       * free the dynamic allocated memory.
       */
      MFREE (w);
    }
    /*
     * RANGE variable
     */
    if ((*FirstByte & 0x8E) == 0x8E)
    {
      /*
       * The format is similar to the bitmap 0 format. The
       * calculation starts from a base channel number svalue
       * instead of channel number 1.
       */
      U32  lvalue;
      U16 svalue;
      U32 i;
      U16 bitposition;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "range variable format");
#endif

      /* Get the first channel number. */
      bf_incBitpos (7, globs);
      lvalue = bf_getBits (10, globs);

      /* Copy lvalue to svalue to set the correct channel. */
      svalue = (U16)lvalue;
      setFirstChanNr (BitmapInStruct, svalue);
      first = svalue;
      num = 1;
      for (i = 1; i < 112; i++)
      {
        /*
         * Get the value of the next bit.
         * If the bit is set, set channel i+svalue
         */
        if (bf_readBit (globs))
        {
          U16   channel = (U16)for_modulo(i+svalue, 1024);

          bitposition = (channel == 0) ? 0 : (U16)(BITOFFSET_LIST - channel);
          BitmapInStruct[bitposition >> 3] |= ByteBitMask[bitposition & 7];
          last = bitposition;
          num++;
        }
      }
      completeAddInfo (BitmapInStruct);
    }
  }

  return 1;
}

/*
+--------------------------------------------------------------------+
| PROJECT : CCD                   MODULE  : cdc_freq_list_encode     |
+--------------------------------------------------------------------+

  PURPOSE : Encoding function is not needed, since this message is 
            sent from net to MS.
            It could be only useful for testing procedure if there
            were an encoder function at this place. 
            This will be a future work.
*/

SHORT cdc_freq_list_encode (const ULONG c_ref, const ULONG e_ref, T_CCD_Globs *globs)
{

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "cdc_freq_list_encode()");
#endif
#ifdef TARGET_WIN32
  /* TBD */
#endif

  return 1;
}
#endif /* !RUN_INT_RAM */

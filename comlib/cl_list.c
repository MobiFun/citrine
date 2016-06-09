/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This module defines the functions for the List
|             processing functions used in components RR/PL of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef CL_LIST_C
#define CL_LIST_C

#include "config.h"
#include "fixedconf.h"

#define ENTITY_RR
#define ENTITY_PL

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>   
#include "typedefs.h"
#include "message.h"
#include "vsi.h"
#include "gsm.h"
#include "prim.h"
#include "cl_list.h"

/*==== CONST ======================================================*/
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
#define INRANGE(min, x, max)  ((unsigned)(x-min) <= (max-min))
#endif

/*==== VARIABLES ==================================================*/
LOCAL const BYTE ByteBitMask[]= {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x1};

/*==== EXPORT =====================================================*/

/*==== PRIVATE FUNCTIONS ==========================================*/
LOCAL UBYTE srv_is_not_in_list (USHORT * channels,
                                USHORT   new_channel,
                                USHORT   size);


/*==== PUBLIC FUNCTIONS ===========================================*/
/*
 * List processing Functions
 *
 * RR uses a lot of channel lists. They are organized internally as bitmaps.
 * In the following a set of functions is defined for access to this lists:
 *
 * srv_set_channel
 * srv_unset_channel
 * srv_get_channel
 * srv_create_list
 * srv_clear_list
 * srv_copy_list
 * srv_compare_list
 * srv_merge_list
 * srv_is_empty_list
 * srv_create_chan_mob_alloc
 *
 * The size of the internal channel lists depends on the supported frequency
 * band:
 *
 * STD=1 (STD_900)         GSM 900
 * STD=2 (STD_EGSM)        E-GSM
 * STD=3 (STD_1900)        PCS 1900
 * STD=4 (STD_1800)        DCS 1800
 * STD=5 (STD_DUAL)        GSM 900 / DCS 1800 DUALBAND
 * STD=6 (STD_DUAL_EGSM)   GSM 900 / E-GSM / DCS 1800 DUALBAND
 * STD=7 (STD_850)         GSM 850
 * STD=8 (STD_DUAL_US)     GSM 850 / PCS 1900 DUALBAND
 *
 * We use a compressed bit array to store the list of channels.
 * Dependent on the configured or found frequency bands the bit array
 * needs several numbers of bytes. For the representation of the individual
 * bits in the array we need the function scr_channel_bit(), setBit(),
 * resetBit() and getBit().
 *
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : setBit					 |
+--------------------------------------------------------------------+

  PURPOSE : sets bit.

*/

LOCAL void setBit (UBYTE* bitstream, unsigned bitindex)
{
  bitstream[bitindex >> 3] |= ByteBitMask[bitindex & 7];
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : resetBit					 |
+--------------------------------------------------------------------+

  PURPOSE : Resets bit.

*/

LOCAL void resetBit (UBYTE* bitstream, unsigned bitindex)
{
  bitstream[bitindex >> 3] &= ~ByteBitMask[bitindex & 7];
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : getBit					 |
+--------------------------------------------------------------------+

  PURPOSE : Gets bit.

*/


LOCAL BYTE getBit (UBYTE* bitstream, unsigned bitindex)
{
  unsigned ByteIdx = bitindex >> 3;

  if (bitstream[ByteIdx])
    return (bitstream[ByteIdx] & ByteBitMask[bitindex & 7]) ? 1 : 0;
  else
    return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : scr_channel_bit            |
+--------------------------------------------------------------------+

  PURPOSE : calculate bit position in the bitstream for a given
            channel and execute function dependent on mode.

*/

GLOBAL UBYTE scr_channel_bit(T_LIST * list, int channel, int mode)
{
  int     bitposition = -1;
  UBYTE   ret = 0;

  /*
   * a more efficient way of range checking for ARM
   * (according to application note 34, ARM DAI 0034A, January 1998)
   *
   * For the following code:
   * if (channel >= low_channel AND channel <= high_channel)
   *   bitposition = ...;
   *
   * exist the faster way to implemented this:
   * if ((unsigned)(channel - low_channel) <= (high_channel - low_channel)
   *   bitposition = ...;
   *
   * Future versions of the compiler will perform this optimization
   * automatically.
   *
   * We use the follwing macro:
   * #define  INRANGE(min, x, max)  ((unsigned)(x-min) <= (max-min))
   */
  if(channel EQ CHANNEL_0)
    channel = CHANNEL_0_INTERNAL;

  bitposition = BITOFFSET_LIST - channel;
      /*if (channel EQ CHANNEL_0)
        bitposition = (USHORT)(BITOFFSET_DUAL_EGSM_B - CHANNEL_0_INTERNAL);
      else if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
        bitposition = (USHORT)(BITOFFSET_DUAL_EGSM_A - channel);
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
        bitposition = (USHORT)(BITOFFSET_DUAL_EGSM_B - channel);
      else if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
        bitposition = (USHORT)(BITOFFSET_DUAL_EGSM_C - channel);
      if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
        bitposition = (USHORT)(BITOFFSET_DUAL_US_A - channel);
      else if (INRANGE(LOW_CHANNEL_1900,channel,HIGH_CHANNEL_1900))
        bitposition = (USHORT)(BITOFFSET_DUAL_US_B - channel);
*/
  if (bitposition >=0)
  {
    switch (mode)
    {
      case SET_CHANNEL_BIT:
        setBit (list->channels, bitposition);
        break;
      case RESET_CHANNEL_BIT:
        resetBit (list->channels, bitposition);
        break;
      case GET_CHANNEL_BIT:
        ret = getBit (list->channels, bitposition);
        break;
      case CHECK_CHANNEL:
        ret = 1;
        break;
    }
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_create_list            |
+--------------------------------------------------------------------+

  PURPOSE : Creates a frequency list in USHORT format from the
            bit list. USHORT format means an array of USHORTs
            followed by NOT_PRESENT_16BIT (0xFFFF), except for the
            case when all elements of the array are used, i.e. the
            array's space is fully occupied.
            In this function the channels are just converted between
            the formats, any semantic errors are not checked here, ie.
            if the MS is 1800-only and the list contains a 900 channel
            this will be converted even if this leads to an error.
            This has to be handled by the caller.

            Parameters:
                list          - [in]  is the input list.
                channel_array - [out] converted list.
                size          - [in]  defines the maximum number of channels
                                      in the list.
                zero_at_start - [in]  specifies where the CHANNEL_0 should be
                                      put:
                                       TRUE  - at the start of the list
                                       FALSE - at the end of the list
                start_index   - [in] specifies a index into the channel_array.
                                     new channels are added at the positions
                                     following and including the index:
                                      channel_array[start_index] to
                                       channel_array[size-1]

                                     If the start_index is not equal zero it is
                                     also checked if the new channel is already
                                     in the channel_array list (from
                                      channel_array[0] to
                                       channel_array[start_index-1])

                                     If the start_index is equal zero the
                                     above check is not performed.

            Return Value:
                number of elements added + start_index

*/

GLOBAL int srv_create_list (T_LIST * list, USHORT * channel_array, USHORT size,
                            UBYTE zero_at_start, USHORT start_index)
{
  int           BitOffset, Idx;
  unsigned int  ByteValue, BitMask, LeftMask;
  int           i = start_index;
  UBYTE         *pch;
  USHORT        *parray = &channel_array[start_index];

  pch = &list->channels[T_LIST_MAX_SIZE-1];
  for(Idx = T_LIST_MAX_SIZE-1; Idx >= 0 AND i < size; Idx--, pch--)
  {
    /*
     * check and add all channels
     */
    if ((ByteValue = *pch) NEQ 0)
    {
      /* byte contains set bits */

      /* check single bits */
      for (BitOffset=7, BitMask=0x01, LeftMask=0xfe;
           BitOffset>=0;
           BitOffset--, BitMask<<=1, LeftMask<<=1)
      {
        if (ByteValue & BitMask)
        {
          *parray = BITOFFSET_LIST - (BitOffset+(Idx<<3));
          if(!start_index OR
             srv_is_not_in_list (channel_array, *parray, start_index))
          {
            /* if the check is ok, ie:
             * always add channel, or the channel has not yet existed
             * in the list, then advance the pointer and add the
             * next channel on next position
             * if the check fails the pointer is not advanced and
             * the channel will not be added and the next channel
             * will overwrite the current channel.
             */
            parray++;

            /* check if list is full */
            if (++i >= size)
              break;
          }
          /* check if any bits are left */
          if ((ByteValue & LeftMask) EQ 0)
            break;
        }
      } /* for all bits in byte */
    } /* if Byte NEQ 0 */
  } /* for all Bytes in List */


  /*
   * If CHANNEL_0 is included in the list
   * it has to be changed from CHANNEL_0_INTERNAL to CHANNEL_0
   * and then the zero_at_start flag is handled.
   *
   * If CHANNEL_0 is in the list it is always
   * at the end of the list.
   */
  if(i NEQ start_index AND
     *(parray-1) EQ CHANNEL_0_INTERNAL)
  {
    *(parray-1) = CHANNEL_0;

    if(zero_at_start AND (i > 1))
    {
      memmove(&channel_array[1], &channel_array[0], (int)sizeof(channel_array[0])*(i-1));
      channel_array[0] = CHANNEL_0;
    }
  }

  /*
   * add the end identifier to the output list
   */
  if (i<size)
  {
    *parray = NOT_PRESENT_16BIT;
  }

  return i;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_is_not_in_list         |
+--------------------------------------------------------------------+

  PURPOSE : Checks if the given channel number is a member of the given
            list.

            Parameters:
            channels    - contains the existing output list.
            new_channel - is the channel number which shall be checked.
            size        - indicates the length of the list.

*/

static UBYTE srv_is_not_in_list (USHORT * channels,
                                 USHORT   new_channel,
                                 USHORT   size)
{
  USHORT   i;

  /*
   * for all members of the list
   */
  for (i=0;i<size;i++)
  {
    /*
     * The end of the list is reached
     * that means the new channel is not inside.
     */
    if (channels[i] EQ NOT_PRESENT_16BIT)
      return TRUE;

    /*
     * the channel is inside
     */
    if (channels[i] EQ new_channel)
      return FALSE;
  }

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_clear_list             |
+--------------------------------------------------------------------+

  PURPOSE : Clears a list by clearing all bits.

*/

GLOBAL void srv_clear_list (T_LIST * list)
{
  /*
   * simple algorithm: clear the whole list.
   */
  memset (list, 0, sizeof (T_LIST));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_copy_list              |
+--------------------------------------------------------------------+

  PURPOSE : Copies a list.

*/

GLOBAL void srv_copy_list (T_LIST * target_list, T_LIST * source_list,
                           UBYTE    size)
{
  memcpy (target_list, source_list, size);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_compare_list           |
+--------------------------------------------------------------------+

  PURPOSE : Compares two lists.

*/

GLOBAL UBYTE srv_compare_list (T_LIST * list1, T_LIST * list2)
{
  return (memcmp (list1, list2, sizeof (T_LIST)) EQ 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_merge_list             |
+--------------------------------------------------------------------+

  PURPOSE : Merges two lists. Both lists are bitmaps. So the merge
            is done by a bitwise OR.

*/

GLOBAL void srv_merge_list (T_LIST * target_list, T_LIST * list)
{
  USHORT i;

  /*
   * The maximum list size is T_LIST_MAX_SIZE Bytes for the dualband extended
   * frequency standard.
   */

  for (i=0;i<T_LIST_MAX_SIZE;i++)
    target_list->channels[i] |= list->channels[i];
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_unmask_list            |
+--------------------------------------------------------------------+

  PURPOSE : This routine resets those bits in destination list that are set in
            the source list.
            Refer Cell Selection Improvements-LLD section:4.1.1.3.11    
*/

GLOBAL void srv_unmask_list(T_LIST *target,T_LIST *source)
{  
   UBYTE count=0;
   for (count=0;count<T_LIST_MAX_SIZE; count++)
   {
      target->channels[count] &= ~source->channels[count];
   }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_trace_freq_in_list     |
+--------------------------------------------------------------------+

  PURPOSE : This routine traces the frequencies in the list
            CSI-LLD section:4.1.1.3.11    
*/

GLOBAL void srv_trace_freq_in_list(T_LIST *list)
{ 
  U16 i;
 
  for(i=CHANNEL_0;i<CHANNEL_0_INTERNAL;i++)
  {
    if(srv_get_channel (list, i))
    {
      TRACE_EVENT_P1("arfcn=%u",i);
    }
  } 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_count_list             |
+--------------------------------------------------------------------+

  PURPOSE : This routine returns the count of the number of channels 
            set in the List
            CSI-LLD section:4.1.1.3.11    
*/

GLOBAL U16 srv_count_list(T_LIST *list)
{ 
  U16 i;
  U16 sum = 0;
 
  for(i=CHANNEL_0;i<CHANNEL_0_INTERNAL;i++)
  {
    if(srv_get_channel (list, i))
    {
      sum++;
    }
  }

  return sum; 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_is_list_set            |
+--------------------------------------------------------------------+

  PURPOSE : This routine checks if any channel in the list is set
            CSI-LLD section:4.1.1.3.11    
*/

GLOBAL BOOL srv_is_list_set(T_LIST *list)
{ 
  U8 i;
 
  for(i=0;i<T_LIST_MAX_SIZE;i++)
  {
    if(list->channels[i])
    {
       return TRUE;
    }
  }

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_get_region_from_std    |
+--------------------------------------------------------------------+

  PURPOSE : This routine derived "region" from "std" 
            CSI-LLD section:4.1.1.3.11    
*/
GLOBAL U8 srv_get_region_from_std(U8 std)
{
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  U8 region = BOTH_REGIONS;
#else
  U8 region = EUROPEAN_REGION;
#endif

  switch(std)
  {
    case STD_850:
    case STD_1900:
    case STD_DUAL_US:
      region = AMERICAN_REGION;
      break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_900:
    case STD_1800:
    case STD_DUAL:
    case STD_DUAL_EGSM:
      region = EUROPEAN_REGION;
      break;
    default:
      TRACE_EVENT_P1 ("srv_get_region_from_std: wrong std %x", std);
      TRACE_ERROR ("srv_get_region_from_std: wrong std");
      break;
#endif
  }

  return region;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)     MODULE  : RR_SRV                       |
| STATE   : code              ROUTINE : srv_get_region_from_std_arfcn|
+--------------------------------------------------------------------+

  PURPOSE : This routine derived "region" from "std" and "ARFCN"
*/
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
GLOBAL U8 srv_get_region_from_std_arfcn(U8 std, U16 arfcn)
{
  U8 region = EUROPEAN_REGION;

  switch(std)
  {
    case STD_850_1800:
    case STD_850_900_1800:
      if (INRANGE(LOW_CHANNEL_850,arfcn,HIGH_CHANNEL_850))
        region = AMERICAN_REGION;
      else
        region = EUROPEAN_REGION;
      break;

    case STD_900_1900:
      if (INRANGE(LOW_CHANNEL_1900,arfcn,HIGH_CHANNEL_1900))
        region = AMERICAN_REGION;
      else
        region = EUROPEAN_REGION;
      break;

    case STD_850_900_1900:
      if (INRANGE(LOW_CHANNEL_1900,arfcn,HIGH_CHANNEL_1900) OR 
          INRANGE(LOW_CHANNEL_850,arfcn,HIGH_CHANNEL_850))
        region = AMERICAN_REGION;
      else
        region = EUROPEAN_REGION;
      break;
    default:
      TRACE_EVENT_P1 ("srv_get_region_from_std_arfcn: wrong std %x", std);
      TRACE_ERROR ("srv_get_region_from_std_arfcn: wrong std");
      break;
  }

  return region;
}
#endif

#endif /* !CL_LIST_C */


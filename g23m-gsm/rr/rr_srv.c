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
|  Purpose :  This module defines the functions for the common
|             services of the component RR of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef RR_SRV_C
#define RR_SRV_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"


/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
LOCAL UBYTE srv_is_not_in_list (USHORT * channels,
                                USHORT   new_channel,
                                USHORT   size);
/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_store_prim             |
+--------------------------------------------------------------------+

  PURPOSE : stores a primitive into a ring buffer. The buffer contains
            up to MAX_RR_STORED_PRIM primitives. The ring buffer is
            controlled by the variables stored_prim_in, which is the
            index of the next free place in the buffer. The variable
            stored_prim_read defines the number of stored primitives,
            which are already indicated as to be read again (by function
            srv_use_stored_prim. The variable stored_prim_write indicates
            the number of stored primitives, which are not indicated as
            to be read again (by function srv_use_stored_prim).
            The return value of the function indicates whether the storing
            was successful or not.

*/

BOOL srv_store_prim (T_PRIM * prim)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("srv_store_prim()");

  /*
   * check the limits. If buffer is full, return false.
   */
  if (rr_data->stored_prim_read + rr_data->stored_prim_write >=
      MAX_RR_STORED_PRIM)
  {
    TRACE_EVENT ("Buffer overrun");
    return FALSE;
  }

  /*
   * Else store the primitive
   */
  TRACE_EVENT ("save prim");

  /*
   * store primitive address and increment stored_prim_in
   * to next free location.
   */
  rr_data->stored_prim [rr_data->stored_prim_in++] = prim;

  /*
   * set stored_prim_in to 0 if at the boarder of the ring buffer
   */
  if (rr_data->stored_prim_in EQ MAX_RR_STORED_PRIM)
  {
    rr_data->stored_prim_in = 0;
  }

  /*
   * increment Number of the stored primitives which are not
   * indicated as to be read again.
   */
  rr_data->stored_prim_write++;

  /*
   * return the success of storing of the primitive.
   */
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_use_stored_prim        |
+--------------------------------------------------------------------+

  PURPOSE : The function re-send all stored primitives to RR. This
            function is called by RR whenever a stored primitive must
            be taken in account, e.g. after cell reselection to handle
            a stored paging.

*/

GLOBAL void srv_use_stored_prim (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("srv_use_stored_prim()");

  ENTITY_DATA->use_stored_entries = FALSE;

  /*
   * Only if anything new has been stored
   */
  if (rr_data->stored_prim_write > 0)
  {
    /*
     * increment the number of primitives which
     * must be re-send to RR by the number of
     * newly stored primitives and reset this variable
     * to zero.
     */
    rr_data->stored_prim_read +=
      rr_data->stored_prim_write;
    rr_data->stored_prim_write = 0;

    /*
     * For all stored primitives
     */
    while (rr_data->stored_prim_read NEQ 0)
    {
      T_PRIM * prim;

      rr_data->stored_prim_read--;

      /*
       * get the primitive address
       */
      prim = rr_data->stored_prim[rr_data->stored_prim_out];
      rr_data->stored_prim[rr_data->stored_prim_out  ] = NULL;
      rr_data->stored_prim_out++;
      /*
       * set the stored_prim_out pointer to the start of the
       * ring buffer, if it is at the end of the buffer
       */
      if (rr_data->stored_prim_out EQ MAX_RR_STORED_PRIM)
        rr_data->stored_prim_out = 0;

      /*
       * If the stored primitive is a paging_ind clear the stored paging flag
       * right away. If not the paging will be thrown away if the flag was
       * not reset propperly on the outside
       */
      if(prim NEQ NULL AND prim->custom.opc EQ MPH_PAGING_IND)
      {
        rr_data->pag_rec = FALSE;
      }

      /*
       * call the protocol entity interface function to handle
       * the stored primitive.
       */
      rr_pei_primitive (prim);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_clear_stored_prim      |
+--------------------------------------------------------------------+

  PURPOSE : The function clears all primitives with the given opcode
            from the ring buffer. If the opcode matches, it releases
            the partition and stores instead a NULL pointer.

*/

GLOBAL void srv_clear_stored_prim (ULONG opc)
{
  GET_INSTANCE_DATA;
  USHORT i, out;

  TRACE_FUNCTION ("srv_clear_stored_prim()");

  /*
   * Only if anything is stored
   */
  if (rr_data->stored_prim_read + rr_data->stored_prim_write > 0)
  {
    /*
     * Calculate the number of stored primitives and the beginning
     */
    i   = rr_data->stored_prim_read + rr_data->stored_prim_write;
    out = rr_data->stored_prim_out;

    while (i NEQ 0)
    {
      /*
       * for each stored primitive
       */
      T_PRIM * prim;

      i--;

      /*
       * Get the partition
       */
      prim = rr_data->stored_prim[out];
      if (prim NEQ NULL AND prim->custom.opc EQ opc)
      {
        /*
         * If a primitive is stored and it has the
         * exspected opcode, release the primitive
         * and store instead a NULL pointer.
         */
        PFREE (P2D(prim));
        rr_data->stored_prim[out] = NULL;
      }
      /*
       * increment out index and set it to 0 if
       * it is on the right boarder of the ring buffer
       */
      out++;
      if (out EQ MAX_RR_STORED_PRIM)
        out = 0;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_check_stored_prim      |
+--------------------------------------------------------------------+

  PURPOSE : Check specific stored primitives. Indicates whether
            a primitive with opcode opc is already stored.

*/

GLOBAL UBYTE srv_check_stored_prim (ULONG opc)
{
  GET_INSTANCE_DATA;
  USHORT i, out;

  TRACE_FUNCTION ("srv_check_stored_prim()");

  /*
   * only if a primitve is stored
   */
  if (rr_data->stored_prim_read + rr_data->stored_prim_write > 0)
  {
    /*
     * calculate number of stored primitives and the beginning
     */
    i   = rr_data->stored_prim_read + rr_data->stored_prim_write;
    out = rr_data->stored_prim_out;

    while (i NEQ 0)
    {
      /*
       * for all stored primitives
       */
      T_PRIM * prim;

      i--;
      /*
       * get the primitive address
       */
      prim = rr_data->stored_prim[out];
      if (prim NEQ NULL AND prim->custom.opc EQ opc)
      {
        /*
         * return TRUE if a primitive is stored and the opcode
         * is as expected.
         */
        return TRUE;
      }
      /*
       * increase out index to the next entry
       */
      out++;
      if (out EQ MAX_RR_STORED_PRIM)
        out = 0;
    }
  }

  /*
   * nothing found
   */
  return FALSE;
}

/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                            |
| STATE   : code                ROUTINE : srv_compare_used_frequency_bands  |
+---------------------------------------------------------------------------+

  PURPOSE : Looks at the frequency bands which were used    and compares 
	    them with the supported bands. Checks if the MA spans more 
	    then one band  (exception is the EGSM part).    

*/

LOCAL int srv_compare_used_frequency_bands (int frequency_bands_used)
{
  int nr_of_frequency_bands_used;
  switch(frequency_bands_used)
  {
    case LOW_BAND_USED:
      if(std EQ STD_900 OR std EQ STD_EGSM OR std EQ STD_850 OR
         std EQ STD_DUAL OR std EQ STD_DUAL_EGSM OR std EQ STD_DUAL_US
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
         OR std EQ STD_850_1800 OR std EQ STD_900_1900 OR std EQ STD_850_900_1800 OR
         std EQ STD_850_900_1900
#endif
         )
        nr_of_frequency_bands_used = 1;
      else
        nr_of_frequency_bands_used = 0;
      break;
    case HIGH_BAND_USED:
      if(std EQ STD_1800 OR std EQ STD_1900 OR
         std EQ STD_DUAL OR std EQ STD_DUAL_EGSM OR std EQ STD_DUAL_US
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
         OR std EQ STD_850_1800 OR std EQ STD_900_1900 OR std EQ STD_850_900_1800 OR
         std EQ STD_850_900_1900
#endif
         )
        nr_of_frequency_bands_used = 1;
      else
        nr_of_frequency_bands_used = 0;
      break;
    case EXT_BAND_USED:
    case LOW_BAND_USED|EXT_BAND_USED:
      if(std EQ STD_EGSM OR std EQ STD_DUAL_EGSM 
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        OR std EQ STD_900_1900 OR std EQ STD_850_900_1800 OR std EQ STD_850_900_1900
#endif
        )
        nr_of_frequency_bands_used = 1;
      else
        nr_of_frequency_bands_used = 0;
      break;
    default:
      nr_of_frequency_bands_used = 0;
    break;
  }

  return nr_of_frequency_bands_used;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_create_chan_mob_alloc  |
+--------------------------------------------------------------------+

  PURPOSE : create a frequency hopping list from channel list and
            mobile allocation. The mobile allocation is a bitmap.
            if the n-th bit of the mobile allocation is set to zero,
            the n-th channel in the channel list is not member of the
            frequency hopping list. If the n-th bit of the mobile
            allocation is set to one, the n-th channel in the channel list
            is member of the frequency hopping list.

            e.g:    channel list has the members 10,20,30,40,50,60,70 and
                    in the mobile allocation the bits 1,3 and 4 are set:

            channel list:        10  20  30  40  50  60  70
            mobile allocation:    0   1   0   1   1   0   0

            that means the hopping list is 20, 40, 50.

            The format of the mobile allocation is an UBYTE array with the
            end identifier NOT_PRESENT_8BIT (0xFF).

            The function returns TRUE if the Mobile Allocation is in one
            frequency band. If the Mobile Allocation is spread over multiple
            frequency bands FALSE is returned and the values returned are
            invalid.

*/

GLOBAL int srv_create_chan_mob_alloc (T_LIST     *clist,
                                        T_LIST     *list,
                                        UBYTE      *mobile_alloc)
{
  /*
   * index in the mobile allocation list
   */
  USHORT  ma_index = 0;
  /*
   * get value of the first bit which is set in the mobile allocation
   */
  UBYTE   ma_value = mobile_alloc [ma_index++];
  UBYTE   x = 0;
  UBYTE   result;
  USHORT  arfcn; 
  UBYTE   band_used;
  UBYTE   freq_band_used = 0;

  TRACE_FUNCTION ("srv_create_chan_mob_alloc()");

  /*
   * clear the result list
   */
  srv_clear_list (list);

  /*
   * Check all Frequencies.
   */
  if(ma_value NEQ NOT_PRESENT_8BIT) 
  {
    for (arfcn=LOW_CHANNEL_900; arfcn<HIGH_CHANNEL_EGSM; arfcn++)
    {
      result = srv_get_channel (clist, arfcn);            
      if (result)
      {
        x++;
      }
      if (x EQ ma_value)
      {
        band_used = for_check_frequency(arfcn);
        if(band_used <= INVALID_BAND_USED)
          return FALSE;

        freq_band_used = freq_band_used | band_used;
        
        srv_set_channel (list, arfcn);
        ma_value = mobile_alloc [ma_index++];
        if (ma_value EQ NOT_PRESENT_8BIT)
          break;
      }
    }
  }
  else
  {
    TRACE_EVENT("Mobile Allocation list empty");
    return FALSE;
  }

  /*
   * at last add channel 0 if needed
   */

  if (ma_value NEQ NOT_PRESENT_8BIT)
  {
    result = srv_get_channel (clist, CHANNEL_0);
    if (result)
    {
      x++;
    }
    if (x EQ ma_value)
    {
      srv_set_channel (list, CHANNEL_0);
      band_used = for_check_frequency(CHANNEL_0);
      if(band_used <= INVALID_BAND_USED)
        return FALSE;
      freq_band_used = freq_band_used | EXT_BAND_USED;
    }
  }

  return srv_compare_used_frequency_bands(freq_band_used);
}


/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                       |
| STATE   : code                ROUTINE : srv_check_frequencies_in_list|
+----------------------------------------------------------------------+

  PURPOSE : Checks used frequencies and returns. Returns number 
	    of frequencies used. 

*/

GLOBAL int srv_check_frequencies_in_list(T_LIST *clist)
{
  int frequency_bands_used = 0;
  int i;

  TRACE_FUNCTION ("srv_check_frequencies_in_list()");

  /*
   * Depending on the frequency standard
   */
  switch (std)
  {
    case STD_900:
    case STD_EGSM:
    case STD_1800:
    case STD_DUAL:
    case STD_DUAL_EGSM:
        /*
         * First check GSM 900
         */
      for (i=LOW_CHANNEL_900; i<=HIGH_CHANNEL_900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }

      for (i=LOW_CHANNEL_1800; i<=HIGH_CHANNEL_1800; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      for (i=LOW_CHANNEL_EGSM; i<HIGH_CHANNEL_EGSM; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= EXT_BAND_USED;
      }
      if (srv_get_channel (clist, CHANNEL_0))
        frequency_bands_used |= EXT_BAND_USED;
      break;
    case STD_850:
    case STD_1900:
    case STD_DUAL_US:
      for (i=LOW_CHANNEL_850; i<=HIGH_CHANNEL_850; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }

      for (i=LOW_CHANNEL_1900; i<=HIGH_CHANNEL_1900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      for (i=LOW_CHANNEL_850; i<=HIGH_CHANNEL_850; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }

      for (i=LOW_CHANNEL_1800; i<=HIGH_CHANNEL_1800; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      break;
    case STD_900_1900:
      for (i=LOW_CHANNEL_900; i<=HIGH_CHANNEL_900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }

      for (i=LOW_CHANNEL_1900; i<=HIGH_CHANNEL_1900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      for (i=LOW_CHANNEL_EGSM; i<HIGH_CHANNEL_EGSM; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= EXT_BAND_USED;
      }
      if (srv_get_channel (clist, CHANNEL_0))
        frequency_bands_used |= EXT_BAND_USED;
      break;
    case STD_850_900_1800:
      for (i=LOW_CHANNEL_900; i<=HIGH_CHANNEL_900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }

      for (i=LOW_CHANNEL_1800; i<=HIGH_CHANNEL_1800; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      for (i=LOW_CHANNEL_EGSM; i<HIGH_CHANNEL_EGSM; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= EXT_BAND_USED;
      }
      for (i=LOW_CHANNEL_850; i<=HIGH_CHANNEL_850; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }
      if (srv_get_channel (clist, CHANNEL_0))
        frequency_bands_used |= EXT_BAND_USED;
      break;
    case STD_850_900_1900:
      for (i=LOW_CHANNEL_900; i<=HIGH_CHANNEL_900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }
      for (i=LOW_CHANNEL_EGSM; i<HIGH_CHANNEL_EGSM; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= EXT_BAND_USED;
      }
      for (i=LOW_CHANNEL_850; i<=HIGH_CHANNEL_850; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= LOW_BAND_USED;
      }
      for (i=LOW_CHANNEL_1900; i<=HIGH_CHANNEL_1900; i++)
      {
        if (srv_get_channel (clist, i))
          frequency_bands_used |= HIGH_BAND_USED;
      }
      if (srv_get_channel (clist, CHANNEL_0))
        frequency_bands_used |= EXT_BAND_USED;
      break;
#endif
    default:
      /*
       * just for LINT
       */
      break;
  }

  return srv_compare_used_frequency_bands (frequency_bands_used);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_SRV                      |
| STATE   : code               ROUTINE : srv_remove_frequencies_in_array_gen |
+--------------------------------------------------------------------+

  PURPOSE : The functions manipulates the neighbourcell list send to
            layer 1. It filters out not supported frequencies or if
            enabled allows to configure a band restriction, that means that
            the mobile camps inside GSM 900 or DCS 1800. The band
            restriction can be configured by a dynamic configuration
            command.

*/

GLOBAL UBYTE srv_remove_frequencies_in_array_gen (USHORT* arfcn_list, UBYTE c_arfcn_list)
{
  GET_INSTANCE_DATA;
  int  in,out, ranges;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  USHORT low[4], high[4];
#else
  USHORT low[3], high[3];
#endif
  UBYTE local_std = std;

  TRACE_FUNCTION ("att_check_band_restrictions()");

  in  = out = ranges = 0;

  memset (low, NOT_PRESENT_8BIT, sizeof (low));
  memset (high, NOT_PRESENT_8BIT, sizeof (high));

  if(rr_data->dyn_config.set_band)
  {
   /* set band is activated */
   if      (rr_data->dyn_config.set_band EQ 1) local_std = STD_900;
   else if (rr_data->dyn_config.set_band EQ 2) local_std = STD_1800;
  }

  switch (local_std)
  {
    case STD_DUAL:
      ranges=2;
      low[0]=LOW_CHANNEL_1800;
      low[1]=LOW_CHANNEL_900;
      high[0]=HIGH_CHANNEL_1800;
      high[1]=HIGH_CHANNEL_900;
      break;
    case STD_DUAL_EGSM:
      /* shift the ranges to allow CHANNEL_0 */
      ranges=3;
      low[0]=LOW_CHANNEL_900-1;/*lint !e778 */
      low[1]=LOW_CHANNEL_EGSM;
      low[2]=LOW_CHANNEL_1800;
      high[0]=HIGH_CHANNEL_900;
      high[1]=HIGH_CHANNEL_EGSM-1;
      high[2]=HIGH_CHANNEL_1800;
      break;
    case STD_DUAL_US:
      ranges=2;
      low[0]=LOW_CHANNEL_1900;
      low[1]=LOW_CHANNEL_850;
      high[0]=HIGH_CHANNEL_1900;
      high[1]=HIGH_CHANNEL_850;
      break;
    case STD_EGSM:
      low[0]  = LOW_CHANNEL_EGSM;
      low[1]  = LOW_CHANNEL_900-1;/*lint !e778 */
      high[0] = HIGH_CHANNEL_EGSM-1;
      high[1] = HIGH_CHANNEL_900;
      ranges=2;
      break;
    case STD_900:
      low[0]  = LOW_CHANNEL_900;
      high[0] = HIGH_CHANNEL_900;
      ranges=1;
      break;
    case STD_1800:
      low[0]  = LOW_CHANNEL_1800;
      high[0] = HIGH_CHANNEL_1800;
      ranges=1;
      break;
    case STD_1900:
      low[0]  = LOW_CHANNEL_1900;
      high[0] = HIGH_CHANNEL_1900;
      ranges=1;
      break;
    case STD_850:
      low[0]  = LOW_CHANNEL_850;
      high[0] = HIGH_CHANNEL_850;
      ranges=1;
      break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      ranges=2;
      low[0]=LOW_CHANNEL_1800;
      low[1]=LOW_CHANNEL_850;
      high[0]=HIGH_CHANNEL_1800;
      high[1]=HIGH_CHANNEL_850;
      break;
    case STD_900_1900:
      /* shift the ranges to allow CHANNEL_0 */
      ranges=3;
      low[0]=LOW_CHANNEL_900-1;/*lint !e778 */
      low[1]=LOW_CHANNEL_EGSM;
      low[2]=LOW_CHANNEL_1900;
      high[0]=HIGH_CHANNEL_900;
      high[1]=HIGH_CHANNEL_EGSM-1;
      high[2]=HIGH_CHANNEL_1900;
      break;
    case STD_850_900_1800:
      ranges=4;
      low[0]=LOW_CHANNEL_900-1;/*lint !e778 */
      low[1]=LOW_CHANNEL_EGSM;
      low[2]=LOW_CHANNEL_1800;
      low[3]=LOW_CHANNEL_850;
      high[0]=HIGH_CHANNEL_900;
      high[1]=HIGH_CHANNEL_EGSM-1;
      high[2]=HIGH_CHANNEL_1800;
      high[3]=HIGH_CHANNEL_850;
      break;
    case STD_850_900_1900:
      ranges=4;
      low[0]=LOW_CHANNEL_900-1;/*lint !e778 */
      low[1]=LOW_CHANNEL_EGSM;
      low[2]=LOW_CHANNEL_1900;
      low[3]=LOW_CHANNEL_850;
      high[0]=HIGH_CHANNEL_900;
      high[1]=HIGH_CHANNEL_EGSM-1;
      high[2]=HIGH_CHANNEL_1900;
      high[3]=HIGH_CHANNEL_850;
      break;
#endif
    default:
      /* default */
      break;
  }

  if (ranges)
  {
    while (arfcn_list[in] NEQ NOT_PRESENT_16BIT AND
           in < c_arfcn_list)
    {
      UBYTE r = ranges;
      while(r)
      {
        if (INRANGE(low[r-1],arfcn_list[in],high[r-1]))
        {
          arfcn_list[out] = arfcn_list[in];
          out++;
        }
        r--;
      }
      in++;
    }
    arfcn_list[out] = NOT_PRESENT_16BIT;
  }

  TRACE_EVENT_WIN_P2 ("%d of %d arfcn's removed", in - out, in);

  return out;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_SRV                      |
| STATE   : code               ROUTINE : srv_remove_frequencies_in_array |
+--------------------------------------------------------------------+

  PURPOSE : The functions manipulates the neighbourcell list send to
            layer 1. It filters out not supported frequencies or if
            enabled allows to configure a band restriction, that means that
            the mobile camps inside GSM 900 or DCS 1800. The band
            restriction can be configured by a dynamic configuration
            command.

*/

GLOBAL void srv_remove_frequencies_in_array (USHORT* arfcn_list)
{
  srv_remove_frequencies_in_array_gen (arfcn_list, MAX_NEIGHBOURCELLS);
}

/*
+-------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV              | 
| STATE   : code                ROUTINE : srv_trace_black_list|
+-------------------------------------------------------------+

  PURPOSE : Traces Black list channels
*/

GLOBAL void srv_trace_black_list()
{
  GET_INSTANCE_DATA;
  U8 i;
  T_LIST *list;

  /* Trace Black List Channels */    
  for(i=0;i<MAX_REGIONS;i++)
  {
    list = &rr_data->cs_data.black_list.list[i];

    TRACE_EVENT_P1("Black List:[%d]Region",i);
     
    srv_trace_freq_in_list(list);
  
  }
}

/*
+--------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV               |  
| STATE   : code                ROUTINE : srv_trace_white_list |
+--------------------------------------------------------------+

  PURPOSE : Traces White list channels
*/

GLOBAL void srv_trace_white_list()
{
  GET_INSTANCE_DATA;
  T_LIST *list;

  /* Trace White List Channels */
  if(rr_data->cs_data.white_list.region NEQ NOT_PRESENT_8BIT)
  {
    list = &rr_data->cs_data.white_list.list;

    TRACE_EVENT_P9 ( "[%d]Reg [%d]SC MCC/MNC r=%x%x%x/%x%x%x/%d",
      rr_data->cs_data.white_list.region,
      rr_data->cs_data.white_list.last_sc_arfcn,
      rr_data->cs_data.white_list.last_sc_lac.mcc[0],
      rr_data->cs_data.white_list.last_sc_lac.mcc[1],
      rr_data->cs_data.white_list.last_sc_lac.mcc[2],
      rr_data->cs_data.white_list.last_sc_lac.mnc[0],
      rr_data->cs_data.white_list.last_sc_lac.mnc[1],
      rr_data->cs_data.white_list.last_sc_lac.mnc[2],
      rr_data->cs_data.white_list.last_sc_lac.lac);
 
    srv_trace_freq_in_list(list);
  }
  else
  {
    TRACE_EVENT("White List absent");
  }

}

#if !defined(NTRACE)
#if 0

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_ATT                      |
| STATE   : code               ROUTINE : srv_list_channels           |
+--------------------------------------------------------------------+

  PURPOSE : print list of channels to the trace interface.
*/
#define SRV_LIST_CHANNELS_TMPBUFSIZE 80 /* sprintf is limited to 80 characters */
static char src_list_channels_tmpbuf[SRV_LIST_CHANNELS_TMPBUFSIZE];
#if defined(_SIMULATION_)
int sprintf( char *buffer, const char *format, ... );
#endif

GLOBAL void srv_list_channels (USHORT* channel_list, USHORT size, char* titel)
{
  UBYTE i,o;

  o = sprintf(src_list_channels_tmpbuf, "%s ", titel);
  for (i = 0; i < size; i++)
    if (channel_list[i] NEQ NOT_PRESENT_16BIT)
    {
      if (o>SRV_LIST_CHANNELS_TMPBUFSIZE-5)
      {
        TRACE_EVENT (src_list_channels_tmpbuf);
        o = 0;
      }
      o += sprintf(src_list_channels_tmpbuf+o, "%u,", channel_list[i]);
    }
  if (o)
  {
    src_list_channels_tmpbuf[o-1] = 0;
    TRACE_EVENT (src_list_channels_tmpbuf);
  }
}
#endif  /* !NTRACE */
#endif

#ifdef GPRS

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_SRV                     |
| STATE   : code                ROUTINE : srv_get_cell_alloc_list    |
+--------------------------------------------------------------------+

  PURPOSE : Access function in RR to get cell alloc list. Needed by GRR to set
            up hopping config.

*/
/* called by GRR */
/*lint -esym(714,srv_get_cell_alloc_list) | Symbol not referenced */
/*lint -esym(765,srv_get_cell_alloc_list) | external could be made static */
GLOBAL void srv_get_cell_alloc_list (T_LIST * target_list)
{
  GET_INSTANCE_DATA;
  srv_merge_list ( target_list, &rr_data->sc_data.cd.cell_chan_desc );
}

#endif  /* GPRS */

#if defined(ARRAY_TRACE)
GLOBAL void  array_trace (UBYTE*array, int size, char *titel)
{
#define MAXCOL  80
  static char buf[MAXCOL+1];
  int col=0, n=0;
  UBYTE b, nh, nl;

  memset (buf, 0, MAXCOL+1);
  TRACE_EVENT_P2 ("%s size=%u bytes", titel, size);
  while (n < size)
  {
    if (col >= MAXCOL-3)
    {
      TRACE_EVENT (buf);
      col = 0;
      memset (buf, 0, MAXCOL+1);
    }
    b = array[n++];
    nh = b>>4;
    nl = b&0x0f;
    buf[col++] = nh > 9 ? nh + 'A' - 10 : nh + '0';
    buf[col++] = nl > 9 ? nl + 'A' - 10 : nl + '0';
    buf[col++] = ' ';
  }
  if (col)
  {
    TRACE_EVENT (buf);
  }
}
#endif  /* ARRAY_TRACE */

#if defined (REL99) AND defined (TI_PS_FF_EMR) AND defined (GPRS)
#ifdef GPRS

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)      MODULE  : RR_SRV                      |
| STATE   : code               ROUTINE : rr_get_support_for_emr      |
+--------------------------------------------------------------------+

  PURPOSE : PS-EMR flag is enabled or not.
*/
GLOBAL BOOL rr_get_support_for_emr(void)
{
  GET_INSTANCE_DATA;
  if(rr_data->ms_data.enable_ps_emr EQ TRUE)   /*for grr*/
  {
    return TRUE;
  }
  
  return FALSE;
}

#endif   /*GPRS*/ 
#endif

#endif /* !RR_SRV_C */

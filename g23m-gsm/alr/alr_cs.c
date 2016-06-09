/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_CS
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
|  Purpose :  This Modul defines the SDL process Cell Selection.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_CS_C
#define ALR_CS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "typedefs.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"

#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif

#include "alr.h"
#include "alr_em.h"
#include "cl_list.h"

#if defined (_SIMULATION_)
#define TRACING
#endif

#if defined (TRACING)
#define ALR_TRACE_CS(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_CS(a)
#endif

#if defined (TRACING)

#define ALR_TRACE_CS_STD(a1)        TRACE_EVENT_P1 ("std %d",a1)
#define ALR_TRACE_CS_BSIC_REQ(a,s)   TRACE_EVENT_P2 ("BSIC REQ [%u] %d", a, s)
#define ALR_TRACE_CS_SYNC_VALID(a,f,t) TRACE_EVENT_P3 ("set new SC[%u],valid block fno:%d,time_al%d",a,f,t)

#else

#define ALR_TRACE_CS_STD(std)
#define ALR_TRACE_CS_BSIC_REQ(a,s)
#define ALR_TRACE_CS_SYNC_VALID(a,f,t)

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/* Power scan attempts for different search modes */
LOCAL const U8 power_scan_attempts[] = { 
  FULL_SEARCH_MODE_ATTEMPTS, 
  NORMAL_SEARCH_MODE_ATTEMPTS,
  FAST_SEARCH_MODE_ATTEMPTS, 
  BLACK_LIST_SEARCH_MODE_ATTEMPTS
};

/* Power measurements spreading time for different search modes */
#if defined(_SIMULATION_)
LOCAL const U16 tim_powermeas_value[] = {
  500,
  500,
  100,
  100
};
#else
LOCAL const U16 tim_powermeas_value[] = { 
  TIM_FULL_SEARCH_POWERMEAS_VAL, 
  TIM_NORMAL_SEARCH_POWERMEAS_VAL, 
  TIM_FAST_SEARCH_POWERMEAS_VAL, 
  TIM_BLACK_LIST_SEARCH_POWERMEAS_VAL
};
#endif

LOCAL const int array_band_index[] = { 
  B_GSM_900,
  B_E_GSM, 
  B_PCS_1900, 
  B_DCS_1800, 
  MAX_NUM_BANDS,
  MAX_NUM_BANDS,
  B_GSM_850 
};

/*==== FUNCTIONS ==================================================*/

LOCAL void cs_add_and_sort_channels           (void);
LOCAL void cs_find_inactive_carriers          (T_POWER_MEAS **p_results,
                                               U16 p_results_size[2],U8 *std,
                                               U8 no_of_attempts, SHORT *min_rxlev);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL U8   cs_add_whitelist_carriers          (U16 p_results_size[2], U8 *std,
                                               U8 attempts,
                                               SHORT *min_rxlev,
                                               T_POWER_MEAS **p_results,
                                               U8 no_of_carriers_per_band[4]);
#else
LOCAL U8   cs_add_whitelist_carriers          (U16 p_results_size[2], 
                                               U8 std, U8 attempts,
                                               SHORT *min_rxlev,
                                               T_POWER_MEAS *presults,
                                               U8 no_of_carriers_per_band[4]);
#endif
LOCAL BOOL cs_is_in_black_list                (U8 region,U16 arfcn);
LOCAL U8   cs_restrict_max_carriers_per_band  (U16 arfcn, U8 std, 
                                               U8 no_of_carriers_per_band[4],
                                               U16 p_results_size[2], U8 min_rxlev);
LOCAL void cs_move_extra_carriers              (U8 i_cnf, U8 extra_cnf);
LOCAL void cs_reorder_the_extra_carriers       (U8 extra_cnf);
LOCAL void cs_power_array_swap_arfcn           (T_POWER_ARRAY *from,
                                                T_POWER_ARRAY *to);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Cell Selection Process.
            Set state and dynamic allocated RAM area to NULL.

*/

GLOBAL void cs_init (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cs_init()");
  alr_data->state[STATE_CS] = CS_NULL;
  memset (&alr_data->cs_data, 0, sizeof (T_CS_DATA));
  alr_data->cs_data.p_results1 = (T_POWER_MEAS*)&alr_power_meas_result1;
  alr_data->cs_data.p_results2 = (T_POWER_MEAS*)&alr_power_meas_result2;
  alr_data->cs_data.search_mode = SM_WIDE_MODE;
  alr_data->cs_data.sync_fail_count = CS_SYNC_FAIL_COUNT_MAX;
  alr_data->cs_data.bcch_fail_count = CS_BCCH_FAIL_COUNT_MAX;

  if ( IS_EXT_MEAS_RUNNING ) /*alr_data->cs_data.mph_ext_meas_req NEQ NULL*/
  {
    PFREE ( alr_data->cs_data.mph_ext_meas_req );
            alr_data->cs_data.mph_ext_meas_req = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_get_next_area           |
+--------------------------------------------------------------------+

  PURPOSE : This returns the next area to scan ('next_area').
            Also it sets 'std' and 'freq_area' - members of 'cs_data' -
            depend on given 'freq_bands'.
*/

LOCAL UBYTE cs_get_next_area (void)
{
  GET_INSTANCE_DATA;
  UBYTE next_area;

  if ((alr_data->cs_data.freq_area&ALL_FREQ_AREA) EQ ALL_FREQ_AREA)
    next_area =
      (alr_data->cs_data.freq_area&NEXT_AMERICAN_AREA) ? AMERICAN_FREQ_AREA : EUROPEAN_FREQ_AREA;
  else
    next_area = alr_data->cs_data.freq_area;

  /*
   * In the case the members and value aren't already initialized,
   * next_area is set to 0
   */
  switch (next_area)
  {
    default:
    case EUROPEAN_FREQ_AREA:
      if ((alr_data->cs_data.std12 & 0x0f) EQ 0) /* not initialized */
        next_area = 0;
      break;
    case AMERICAN_FREQ_AREA:
      if ((alr_data->cs_data.std12 >> 4) EQ 0) /* not initialized */
        next_area = 0;
      break;
    case 0:
      break;
  }

/*
  if (next_area)
  {
    TRACE_EVENT_P9 ("cs_get_next_area: fb=%02x => cs_std=%u(%u) std12=%02x area=%02x'%c%c' next=%u'%c'",
      alr_data->cs_data.freq_bands, alr_data->cs_data.std, std,
      alr_data->cs_data.std12, alr_data->cs_data.freq_area,
      (alr_data->cs_data.freq_area & EUROPEAN_FREQ_AREA) ? 'E':' ',
      (alr_data->cs_data.freq_area & AMERICAN_FREQ_AREA) ? 'A':' ',
      next_area, next_area ? ((next_area EQ AMERICAN_FREQ_AREA) ? 'A':' E') : '?');
  }
*/
  return next_area;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_get_std_area            |
+--------------------------------------------------------------------+

  PURPOSE : This returns the frequency band 'std' depend on given
            'freq_bands'. It also sets the suitable frequency area
            to the value of a possible given area pointer.

            This function knows only the single and dual frequency
            band combinations. Triple or quad frequency band
            combinations leads to return values set to 0.
*/

LOCAL UBYTE cs_get_std_area (UBYTE freq_bands, UBYTE *p_area)
{
  UBYTE cs_freq_area;
  UBYTE cs_std;

  switch (freq_bands)
  {
    case BAND_GSM_900:/* single band */
      cs_freq_area = EUROPEAN_FREQ_AREA;
      cs_std = STD_900;
      break;
    case BAND_DCS_1800:/* single band */
      cs_freq_area = EUROPEAN_FREQ_AREA;
      cs_std = STD_1800;
      break;
    case BAND_PCS_1900:/* single band */
      cs_freq_area = AMERICAN_FREQ_AREA;
      cs_std = STD_1900;
      break;
    case BAND_E_GSM:/* extended single band */
    case BAND_GSM_900|BAND_E_GSM: /* extended single band */
      cs_freq_area = EUROPEAN_FREQ_AREA;
      cs_std = STD_EGSM;
      break;
    case BAND_GSM_850:/* single band */
      cs_freq_area = AMERICAN_FREQ_AREA;
      cs_std = STD_850;
      break;
    case BAND_DUAL:/* dual band */
      cs_freq_area = EUROPEAN_FREQ_AREA;
      cs_std = STD_DUAL;
      break;
    case BAND_DUAL_EXT:/* dual band */
      cs_freq_area = EUROPEAN_FREQ_AREA;
      cs_std = STD_DUAL_EGSM;
      break;
    case BAND_DUAL_US:/* dual band */
      cs_freq_area = AMERICAN_FREQ_AREA;
      cs_std = STD_DUAL_US;
      break;
    default:
      cs_freq_area = 0;
      cs_std = 0;
      break;
  }

  if (p_area)
    *p_area = cs_freq_area;
  return cs_std;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_set_std_area            |
+--------------------------------------------------------------------+

  PURPOSE : This returns the next area to scan ('next_area').
            Also it sets 'std' and 'freq_area' - members of 'cs_data' -
            depend on given 'freq_bands'.
            This function will be executed at least once (single band) and
            not more than twice (multiband) per power request.
*/

LOCAL UBYTE cs_set_std_area (void)
{
  GET_INSTANCE_DATA;
  UBYTE next_area;
  UBYTE cs_freq_bands = alr_data->cs_data.freq_bands;
  UBYTE cs_std;
  UBYTE cs_freq_area = alr_data->cs_data.freq_area;

/*
  TRACE_EVENT_P1 ("cs_set_std_area(): %s call", cs_freq_area?"second":"first");
*/
  if (cs_freq_area & NEXT_AMERICAN_AREA)
    /*
     * Initializing before second measurement
     */
    next_area = AMERICAN_FREQ_AREA;
  else
    next_area = 0;

  if (cs_freq_bands EQ 0)
  {
    TRACE_ERROR ("alr_data->cs_data.freq_bands=0 (Invalid value)!");
    TRACE_ASSERT (cs_freq_bands EQ 0);
  }

  cs_std = cs_get_std_area(cs_freq_bands, &cs_freq_area);
  if (cs_std EQ 0)
  {
    cs_freq_area = 0;
    if (cs_freq_bands & BAND_DUAL_EXT)
    { /* european frequency bands */
      cs_freq_area |= EUROPEAN_FREQ_AREA;
      if (next_area EQ 0)
        next_area = EUROPEAN_FREQ_AREA;
    }
    if (cs_freq_bands & BAND_DUAL_US)
    { /* american frequency bands */
      cs_freq_area |= AMERICAN_FREQ_AREA;
      if (next_area EQ 0)
        next_area = AMERICAN_FREQ_AREA;
    }

    if (next_area EQ EUROPEAN_FREQ_AREA)
    {
      cs_freq_bands &= BAND_DUAL_EXT;
    }
    else
    {
      cs_freq_bands &= BAND_DUAL_US;
      cs_freq_area |= NEXT_AMERICAN_AREA;
    }
    /*
     * get the next 'std' depend on the value of 'next_area'
     */
    cs_std = cs_get_std_area (cs_freq_bands, NULL);
  }

  if (next_area EQ 0)
    next_area = cs_freq_area;

  alr_data->cs_data.freq_area = cs_freq_area;
  alr_data->cs_data.std = cs_std;
  TRACE_EVENT_P9 ("cs_set_std_area: fb=%02x => cs_std=%u(%u) std12=%02x area=%02x'%c%c' next=%u'%c'",
    alr_data->cs_data.freq_bands, alr_data->cs_data.std, std,
    alr_data->cs_data.std12, alr_data->cs_data.freq_area,
    (alr_data->cs_data.freq_area & EUROPEAN_FREQ_AREA) ? 'E':' ',
    (alr_data->cs_data.freq_area & AMERICAN_FREQ_AREA) ? 'A':' ',
    next_area, next_area ? ((next_area EQ AMERICAN_FREQ_AREA) ? 'A':'E') : '?');

  return next_area;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_power_array_swap_arfcn|
+--------------------------------------------------------------------+

  PURPOSE : This function swaps ARFCN and its RxLev between the 
            locations passed.
*/
LOCAL void cs_power_array_swap_arfcn(T_POWER_ARRAY *ptr1,T_POWER_ARRAY *ptr2)
{
  U16   temp_arfcn;
  SHORT temp_rxlev;  

  temp_arfcn               = ptr1->radio_freq;
  temp_rxlev               = ptr1->accum_power_result;

  ptr1->radio_freq         = ptr2->radio_freq;
  ptr1->accum_power_result = ptr2->accum_power_result;

  ptr2->radio_freq         = temp_arfcn;
  ptr2->accum_power_result = temp_rxlev;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_prepare_power_req       |
+--------------------------------------------------------------------+

  PURPOSE : This prepares the inputs for the power request depending
            on the frequency areas.

*/

GLOBAL T_POWER_MEAS* cs_prepare_power_req (void)
{
  GET_INSTANCE_DATA;
  UBYTE         next_area,region;
  T_POWER_MEAS  *power_meas;
  T_LIST        *black_list;

  TRACE_FUNCTION ("cs_prepare_power_req");

  /*
   * depending on the set frequency area
   */
  next_area = cs_get_next_area ();

  if (next_area)
  { /* members and values are already initialized */
    switch (next_area)
    {
      default:
      case EUROPEAN_FREQ_AREA:
        power_meas = alr_data->cs_data.p_results1;
        alr_data->cs_data.std = alr_data->cs_data.std12&0x0f;
        break;
      case AMERICAN_FREQ_AREA:
        power_meas = alr_data->cs_data.p_results2;
        alr_data->cs_data.std = alr_data->cs_data.std12>>4;
        break;
    }

    TRACE_EVENT_WIN_P4 ("cs_prepare_power_req: cs_std=%u(%02x) next=%u'%c'",
      alr_data->cs_data.std, alr_data->cs_data.std12,
      next_area, (next_area EQ AMERICAN_FREQ_AREA) ? 'A':'E');

  }
  else
  { /* must be initialize first */
    int   i;
    int   power_array_size;
    int   radio_freq_offset = 1;

    /*
     * depending on the given frequency bands
     */
    next_area = cs_set_std_area ();
    /*
     * depending on the just set frequency standard
     */
    switch(alr_data->cs_data.std)
    {
      case STD_900:
        power_array_size = MAX_CARRIERS_GSM900;
        break;

      case STD_EGSM:
        power_array_size = MAX_CARRIERS_EGSM900;
        break;

      case STD_1900:
        power_array_size = MAX_CARRIERS_PCS1900;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        radio_freq_offset = 1024;
#else
        radio_freq_offset = 512;
#endif
        break;

      case STD_1800:
        power_array_size = MAX_CARRIERS_DCS1800;
        radio_freq_offset = 512;
        break;

      case STD_DUAL:
        power_array_size = MAX_CARRIERS_DUAL;
        break;

      case STD_DUAL_EGSM:
        power_array_size = MAX_CARRIERS_DUAL_EGSM;
        break;

      case STD_850:
        power_array_size = MAX_CARRIERS_GSM850;
        radio_freq_offset = 128;
        break;

      case STD_DUAL_US:
        power_array_size = MAX_CARRIERS_DUAL_US;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        radio_freq_offset = 128;
#endif
        break;

      default:
          power_array_size = 0;
        break;
    }

    if (power_array_size)
    {
      /*
       * fill all carriers which shall be measured into the structure
       * to layer 1.
       */
      T_POWER_ARRAY *power_array;
      USHORT size;

      if ( IS_EXT_MEAS_RUNNING )
      {
        power_array_size = alr_data->cs_data.mph_ext_meas_req->num_of_chan;
      }

      /* depend on the next area get the right T_POWER_MEAS instance */
      switch (next_area)
      {
        default:
        case EUROPEAN_FREQ_AREA:
            size = sizeof (T_POWER_MEAS1);
            power_meas = alr_data->cs_data.p_results1;
            /*
            TRACE_EVENT_P1 ("static T_POWER_MEAS(EU): size=%u", sizeof (T_POWER_MEAS1));
            */
          memset (alr_data->cs_data.p_results1, 0, sizeof (T_POWER_MEAS));

          /* save value of 'std' for cs_increment_c_channels() */
          alr_data->cs_data.std12 |= alr_data->cs_data.std & 0x0f;
          break;

        case AMERICAN_FREQ_AREA:
            size = sizeof (T_POWER_MEAS2);
            power_meas = alr_data->cs_data.p_results2;
            /*
            TRACE_EVENT_P1 ("static T_POWER_MEAS(USA): size=%u", sizeof (T_POWER_MEAS2));
            */
          /* save value of 'std' for cs_increment_c_channels() */
          alr_data->cs_data.std12 |= (alr_data->cs_data.std << 4);
          break;
      }

      memset (power_meas, 0, size);

      TRACE_EVENT_WIN_P8 ("cs_prepare_power_req: cs_std=%u(%u) std12=%02x rf=%u..%u (%u) next=%u'%c'",
        alr_data->cs_data.std, std, alr_data->cs_data.std12,
        radio_freq_offset, power_array_size+radio_freq_offset-1,
        power_array_size,
        next_area, (next_area EQ AMERICAN_FREQ_AREA) ? 'A':'E');

      power_array = &(power_meas->power_array[0]);
      memset (power_array, 0, sizeof (T_POWER_ARRAY) * power_array_size);
      power_meas->power_array_size = power_array_size;

      if ( IS_EXT_MEAS_RUNNING )
      {
        USHORT *arfcn = &alr_data->cs_data.mph_ext_meas_req->arfcn[0];
        for ( i = 0; i < power_array_size; i++, power_array++, arfcn++ )
        {
          power_array->radio_freq = ARFCN_TO_L1 ( *arfcn );
        }
      }
      else
      {
        if(alr_data->cs_data.p_power_req->search_mode NEQ BLACK_LIST_SEARCH_MODE)
        {
          power_array_size += radio_freq_offset;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          if (alr_data->cs_data.std EQ STD_EGSM)
          {
            for (i=radio_freq_offset; i <= MAX_CARRIERS_GSM900; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            for (i=LOW_CHANNEL_EGSM; i < HIGH_CHANNEL_EGSM; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            power_array->radio_freq = 0x00;
            power_array++;
          }
          else if (alr_data->cs_data.std EQ STD_DUAL)
          {
            for (i=radio_freq_offset; i <= MAX_CARRIERS_GSM900; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            for (i=LOW_CHANNEL_1800; i <= HIGH_CHANNEL_1800; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
          }
          else if (alr_data->cs_data.std EQ STD_DUAL_EGSM)
          {
            for (i=radio_freq_offset; i <= MAX_CARRIERS_GSM900; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            for (i=LOW_CHANNEL_EGSM; i < HIGH_CHANNEL_EGSM; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            for (i=LOW_CHANNEL_1800; i <= HIGH_CHANNEL_1800; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            power_array->radio_freq = 0x00;
            power_array++;
          }
          else if (alr_data->cs_data.std EQ STD_DUAL_US)
          {
            for (i=radio_freq_offset; i <= HIGH_CHANNEL_850; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
            for (i=1024; i <= 1322; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
          }
          else
          {
            for(i=radio_freq_offset; i < power_array_size; i++, power_array++)
            {
              power_array->radio_freq = (U16)i;
            }
          }
#else
          for(i=radio_freq_offset; i < power_array_size; i++, power_array++)
          {
            power_array->radio_freq = (U16)i;
          }
#endif
        }
        else
        { 
          /* Blacklist search. Fill all Black Listed and "Grey" carriers 
           * sent by RR for L1 measurement
           */
          region     = srv_get_region_from_std(std);
          power_meas->power_array_size = 0;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          if (region EQ BOTH_REGIONS)
          {
            int j;
            for (j=0; j<MAX_REGIONS; j++)
            {
              black_list = (T_LIST*)&alr_data->cs_data.p_power_req->black_list.list[j];
          
          for(i=CHANNEL_0;i<CHANNEL_0_INTERNAL;i++)
          {
            if(srv_get_channel(black_list, i))
            {
              power_array->radio_freq = ARFCN_TO_L1 ( i );

              power_meas->power_array_size++;
              power_array++;
            }
          }
            }
          }
          else
          {
#endif
            black_list = (T_LIST*)&alr_data->cs_data.p_power_req->black_list.list[region];
          
            for(i=CHANNEL_0;i<CHANNEL_0_INTERNAL;i++)
            {
              if(srv_get_channel(black_list, i))
              {
                power_array->radio_freq = ARFCN_TO_L1 ( i );
  
                power_meas->power_array_size++;
                power_array++;
              }
            }
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
          }
#endif
        } /* Black list search */
      } /* !EXT_MEAS */
    } /* power_array_size != 0 */
    else
    {
      TRACE_EVENT_P6 ("cs_prepare_power_req: invalid: fb=%02x cs_std=%u area=%02x'%c%c' next='%c'",
        alr_data->cs_data.freq_bands,
        alr_data->cs_data.std, alr_data->cs_data.freq_area,
        (alr_data->cs_data.freq_area & EUROPEAN_FREQ_AREA) ? 'E':' ',
        (alr_data->cs_data.freq_area & AMERICAN_FREQ_AREA) ? 'A':' ',
        (next_area EQ AMERICAN_FREQ_AREA) ? 'A':'E');
      power_meas = NULL;/* invalid values */
    }
  }
  return power_meas;/* do it */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_power_req               |
+--------------------------------------------------------------------+

  PURPOSE : This starts measurement of the fieldstrength of all channels.

*/

GLOBAL void cs_power_req (UBYTE pch_interrupt)
{
  GET_INSTANCE_DATA;
  /* When Power On alr_data->cs_data.std will be set to 0, which wil be changed later. This is
     used to find out if it is a first power scan after boot up. MPHC_INIT_L1_REQ will be sent
     only during first powerv scan. It should not be sent on subsequent power scan */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  U8 initial_req = alr_data->cs_data.std;
#endif

  TRACE_EVENT_P1 (" p_results1=%08x", alr_data->cs_data.p_results1);
  
  /* Reset CS data */
  alr_data->cs_data.std12 = 0;
  alr_data->cs_data.freq_area = 0;
  alr_data->cs_data.c_meas = 0;
  alr_data->cs_data.p_results1->power_array_size = 0;
  alr_data->cs_data.p_results2->power_array_size = 0;

  cs_prepare_power_req();

  if(IS_EXT_MEAS_RUNNING)
  {
    alr_data->cs_data.c_max_meas = power_scan_attempts[FULL_SEARCH_MODE];

    alr_data->cs_data.c_tim_meas = (tim_powermeas_value[FULL_SEARCH_MODE]/
                                    power_scan_attempts[FULL_SEARCH_MODE]);

  }
  else
  {
    T_MPH_POWER_REQ* mph_power_req = alr_data->cs_data.p_power_req;
    
    /* CSI-LLD Section: 4.1.3.4.2.3 
     * Set the number of RF scan attempts and TIM_POWER_MEAS timer value 
     * based on the search mode
     */

    /* Set the number of RF scan attempts */
    alr_data->cs_data.c_max_meas = 
      power_scan_attempts[mph_power_req->search_mode];

    /* Set TIM_POWER_MEAS timer value */
    alr_data->cs_data.c_tim_meas = 
      (tim_powermeas_value[mph_power_req->search_mode] /  
       power_scan_attempts[mph_power_req->search_mode]);


    if (((alr_data->cs_data.freq_area & ALL_FREQ_AREA) EQ ALL_FREQ_AREA) AND 
        (mph_power_req->search_mode NEQ BLACK_LIST_SEARCH_MODE))
    {
      /* Multiply the number of field strength measurements by 2 */
      alr_data->cs_data.c_max_meas <<= 1;

      /* Reduce the TIM_POWER_MEAS value by half */
      alr_data->cs_data.c_tim_meas >>= 1;
    }

    TRACE_EVENT_P2(" Search Type:[%u] Search Mode:[%u]",
      mph_power_req->pch_interrupt,
      mph_power_req->search_mode);
  }

  /* new state is dependent on necessary of the L1 configuration */
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
  if (!initial_req)
#else
  if ((pch_interrupt EQ PCH_INTERRUPT) OR (alr_data->cs_data.std NEQ std))
#endif
  {
    SET_STATE (STATE_CS, CS_INIT_L1);
  }
  else
  {
    SET_STATE (STATE_CS, CS_START_MEASURE);
  }

  TRACE_EVENT_P2 ("cs_power_req: c_max_meas=%u c_tim_meas=%u",
    alr_data->cs_data.c_max_meas, alr_data->cs_data.c_tim_meas);

  ma_cs_rxlev_req ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_bsic_req                |
+--------------------------------------------------------------------+

  PURPOSE : Request of RR to search for frequency correction
            burst and synchron burst.

*/

GLOBAL void cs_bsic_req (T_MPH_BSIC_REQ * mph_bsic_req)
{
  GET_INSTANCE_DATA;
  UBYTE cs_std;

  if (mph_bsic_req)
  {
    ALR_TRACE_CS_BSIC_REQ(mph_bsic_req->arfcn&ARFCN_MASK, GET_STATE (STATE_CS));

    ALR_EM_BSIC_REQUEST;

  }
  /*
   * cs_bsic_req is called from several places with mph_bsic_req EQ NULL
   */

  switch (GET_STATE (STATE_CS))
  {
    case CS_ACTIVE_BCCH:
      ma_stop_scell_bcch_req ();
      /*
       * first stop BCCH reading,
       * then start like in idle mode if
       * it is a valid channel number
       */
    /*lint -fallthrough*/
    default:
      if (mph_bsic_req)
      {
        alr_data->cs_data.arfcn = mph_bsic_req->arfcn;
        cs_std = STD_GET_FROM_ARFCN (mph_bsic_req->arfcn);
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        alr_data->cs_data.std  = cs_std;
#else
        if (cs_std AND (cs_std NEQ alr_data->cs_data.std))
        { /*
           * When RR supplies a value of 'std' then init radio band before sync
           */
          TRACE_EVENT_WIN_P2("cs_bsic_req: cs_std=%u->%u",
            alr_data->cs_data.std, cs_std);
          alr_data->cs_data.std  = cs_std;
          ma_cs_init_l1_req(alr_data->cs_data.std);
          SET_STATE(STATE_CS, CS_INIT_SYNC);
          return; /* wait of MPHC_INIT_L1_CON */
        }
#endif
      }
      /* If no value of 'std' are supplied, then start sync immediately */
    /*lint -fallthrough*/
    case CS_INIT_DONE:
#if defined(STOP_SYNC_TASK)
      if (alr_data->cs_data.sync_active)
      {
        /*
         * stop any synchronisation task
         */
        SET_STATE(STATE_CS, CS_STOP_SYNC);
        ma_cs_stop_network_sync_req();
        return;/* wait for MPHC_STOP_NETWORK_SYNC_CON */
      }
    /*lint -fallthrough*/
    case CS_STOP_SYNC_DONE:
#endif /* STOP_SYNC */
      /*
       * start synchronisation
       * to the frequency correction burst and synchron burst
       * in layer 1.
       */
      TRACE_EVENT_P1 ("NETWORK_SYNC_REQ[%u]", alr_data->cs_data.arfcn&ARFCN_MASK);
      ma_cs_network_sync_req (ARFCN_STD_TO_L1(alr_data->cs_data.arfcn, alr_data->cs_data.std));
      SET_STATE(STATE_CS, CS_ACTIVE_SYNC);
      break;
#if defined(STOP_SYNC_TASK)
    case CS_STOP_SYNC:
      /* do nothing, wait for MPHC_STOP_NETWORK_SYNC_CON */
      return;
#endif /* STOP_SYNC */
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_rxlev_ind               |
+--------------------------------------------------------------------+

  PURPOSE : This is the response from layer 1 for
            a measurement sample request over all channels.

*/

GLOBAL void cs_rxlev_ind (T_MPHC_RXLEV_IND* rxlev_ind)
{
  GET_INSTANCE_DATA;
#if defined(_SIMULATION_)
  {
    /*
     * special handling for windows simulation. In this case predefined
     * values are used, because the primitives are too big to be forwarded
     * via the test interface.
     */
    int index = rxlev_ind->shared_ptr;

    memcpy(alr_data->cs_data.p_results1,&tap_rxlev_response_european[index],
           sizeof(T_POWER_MEAS));

    memcpy(alr_data->cs_data.p_results2,&tap_rxlev_response_american[index],
           sizeof(T_POWER_MEAS));
  }
#endif  /* _SIMULATION_ */

  TRACE_EVENT_P3 ("cs_rxlev_ind(): cs_std=%u c_meas=%u/%u",
    alr_data->cs_data.std, alr_data->cs_data.c_meas+1, alr_data->cs_data.c_max_meas);

  if ( IS_EXT_MEAS_RUNNING AND alr_data->cs_data.ext_meas_state_pend NEQ CS_NULL )
  {
    TIMERSTOP(TIM_POWERMEAS);
    SET_STATE (STATE_CS, CS_NULL);
    nc_stop_ext_meas_ind();
    return;
  }

  switch (GET_STATE (STATE_CS))
  {
    case CS_MEASURED:

      /* increment the number of measurement samples */
      alr_data->cs_data.c_meas++;

      if (alr_data->cs_data.c_meas EQ alr_data->cs_data.c_max_meas)
      { 
        /* Allocate memory for MPH_POWER_CNF */
        PALLOC (mph_power_cnf, MPH_POWER_CNF);
        
        if (alr_data->cs_data.p_power_cnf)
        {
          PFREE (alr_data->cs_data.p_power_cnf);
        }

        alr_data->cs_data.p_power_cnf = mph_power_cnf;

        memset (alr_data->cs_data.p_power_cnf, 0, sizeof (T_MPH_POWER_CNF));

        TIMERSTOP(TIM_POWERMEAS);

        /* Sort the channels based on their RxLev */
        cs_add_and_sort_channels ();

        if(!IS_EXT_MEAS_RUNNING)
        {
          /* Free MPH_POWER_REQ buffer */
          PFREE(alr_data->cs_data.p_power_req);

          alr_data->cs_data.p_power_req = NULL;
        }

        /* Send fieldstrength list to RR */
        ma_cs_power_cnf (mph_power_cnf);

        /* we are done with power measurements, next comes the BCCH detection */
        SET_STATE (STATE_CS, CS_NULL);

        alr_data->cs_data.p_power_cnf = NULL;

        ALR_EM_POWER_MEASUREMENT_CONFIRM;
      }
      else
      if (alr_data->cs_data.c_meas < alr_data->cs_data.c_max_meas)
      {
       /*
        * start next sample
        */
        if ((alr_data->cs_data.freq_area & ALL_FREQ_AREA) EQ ALL_FREQ_AREA)
        { /*
           * toggle radio_band and configure new before the next measurement
           */
          TRACE_EVENT_P2 ("cs_rxlev_ind(): area=%02x->%02x",
            alr_data->cs_data.freq_area,
            alr_data->cs_data.freq_area^NEXT_AMERICAN_AREA);
          alr_data->cs_data.freq_area ^= NEXT_AMERICAN_AREA;
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
          SET_STATE (STATE_CS, CS_INIT_L1);
#endif
        }
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
        else
        { /*
           * start new measurement without new configuration of radio_band
           */
#endif
        SET_STATE (STATE_CS, CS_START_MEASURE);
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
        }
#endif
        ma_cs_rxlev_req ();
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_network_sync_ind        |
+--------------------------------------------------------------------+

  PURPOSE : The function handles the result of a search for FCB or SCB.

*/

GLOBAL void cs_network_sync_ind (T_MPHC_NETWORK_SYNC_IND* sync_ind)
{
  GET_INSTANCE_DATA;
  PALLOC (mph_bsic_cnf, MPH_BSIC_CNF);

  alr_data->cs_data.sync_active = FALSE;
  /*
   * The BCCH fail counter has to be reinitialized for every new cell.
   */
  alr_data->cs_data.bcch_fail_count = CS_BCCH_FAIL_COUNT_MAX;
  /*
   * copy arfcn, rxlev and bsic
   */
  mph_bsic_cnf->arfcn  = ARFCN_STD_TO_G23(sync_ind->radio_freq, alr_data->cs_data.std);
  mph_bsic_cnf->arfcn  = STD_ADD_TO_ARFCN(mph_bsic_cnf->arfcn, alr_data->cs_data.std);
  /* US_BIT should be used to differentiate an US frequency channel. */
  switch (alr_data->cs_data.std)
  {
    case STD_1900:
    case STD_850:
    case STD_DUAL_US:
      mph_bsic_cnf->arfcn |= US_BIT;
      break;
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
    case STD_850_900_1800:
      if ((mph_bsic_cnf->arfcn >= LOW_CHANNEL_850) && (mph_bsic_cnf->arfcn <= HIGH_CHANNEL_850))
        mph_bsic_cnf->arfcn |= US_BIT;
      break;
    case STD_900_1900:
      if ((mph_bsic_cnf->arfcn >= LOW_CHANNEL_1900) && (mph_bsic_cnf->arfcn <= HIGH_CHANNEL_1900))
        mph_bsic_cnf->arfcn |= US_BIT;
      break;
    case STD_850_900_1900:
      if (mph_bsic_cnf->arfcn >= HIGH_CHANNEL_900)
        mph_bsic_cnf->arfcn |= US_BIT;
      break;
#endif
    default:
      break;
  }

  /*
   * set bsic and result code
   */
  mph_bsic_cnf->bsic   = (UBYTE)(sync_ind->bsic & 63);
  mph_bsic_cnf->cs = sync_ind->sb_flag ? CS_NO_ERROR : CS_NO_BCCH_AVAIL;
  
/* Implements Measure#32: Row 21 and 22 */
  if (sync_ind->sb_flag)
  {
    TRACE_EVENT_P3 ("network_sync_ind:[%u] rf=%u cs_std=%u OK",
      mph_bsic_cnf->arfcn&ARFCN_MASK,
      sync_ind->radio_freq, alr_data->cs_data.std);
  }
  else
  {
    TRACE_EVENT_P3 ("network_sync_ind:[%u] rf=%u cs_std=%u no BCCH avail.",
      mph_bsic_cnf->arfcn&ARFCN_MASK,
      sync_ind->radio_freq, alr_data->cs_data.std);
  }

  switch (GET_STATE(STATE_CS))
  {
    /*
     * workarounds for crossing MPHC_STOP_NETWORK_SYNC_REQ and
     * MPHC_NETWORK_SYNC_IND:
     * It is possible to receive a MPHC_NETWORK_SYNC_IND from layer 1 at the
     * same time as sending a MPHC_STOP_NETWORK_SYNC_REQ. The
     * MPHC_STOP_NETWORK_SYNC_REQ will be ignored by the layer 1 and no
     * STOP_NW_SYNC_CON will be send.
     */
  case CS_STOP_SYNC:
    /* The state CS_STOP_SYNC was set and the MPHC_NETWORK_SYNC_REQ was
     * interrupted to clean the way for a new MPHC_NETWORK_SYNC_REQ. A possible
     * MPHC_NETWORK_SYNC_IND will be treated as MPHC_STOP_NETWORK_SYNC_CON
     * because the result does not matter.
     */
    SET_STATE (STATE_CS, CS_STOP_SYNC_DONE);
    cs_bsic_req(NULL);
    /*lint -fallthrough */
  default:
    /*
     * the synchronisation has been broken
     */
    ALR_TRACE_CS ("MPHC_NETWORK_SYNC_IND ignored");
    PFREE (mph_bsic_cnf);
    return;
    /* break; */

  case CS_NW_SYNC_TIMEOUT:
    /*
     * The state CS_NW_SYNC_TIMEOUT was set and the MPHC_NETWORK_SYNC_REQ was
     * interrupted to limit the time for reading the BCCH. A possible
     * MPHC_NETWORK_SYNC_IND will be treated normal. Otherwise, no BSIC_CNF
     * will be sent to RR, and RR waits forever.
     */
    SET_STATE(STATE_CS, CS_ACTIVE_SYNC);
    /*lint -fallthrough */
  case CS_ACTIVE_SYNC:
    ma_bsic_cnf (mph_bsic_cnf);
    break;
  }

#ifdef GPRS
  if(alr_data->gprs_data.pcco_active)
  {
    USHORT index;
    /* store data in nc_data.cr_cell */
    alr_data->nc_data.cr_cell.ba_arfcn     =
      ARFCN_TO_G23(sync_ind->radio_freq)&ARFCN_MASK;
    alr_data->nc_data.cr_cell.bsic         = (UBYTE)(sync_ind->bsic & 63);
    alr_data->nc_data.cr_cell.frame_offset = sync_ind->fn_offset;
    alr_data->nc_data.cr_cell.time_align   = sync_ind->time_alignment;

    index = nc_get_index(alr_data->nc_data.cr_cell.ba_arfcn);
    if ((index NEQ NOT_PRESENT_16BIT) AND (index NEQ LAST_BSIC_REQ))
    {
      /*
       * update in nc_data also because this data is not valid
       * anymore after a network_sync_req and in some special cases
       * nc_start_reselect will use the data in nc_data.cell[index]
       */
      alr_data->nc_data.cell[index].bsic         = (UBYTE)(sync_ind->bsic & 63);
      alr_data->nc_data.cell[index].frame_offset = sync_ind->fn_offset;
      alr_data->nc_data.cell[index].time_align   = sync_ind->time_alignment;
    }
    SET_STATE(STATE_CS, CS_NULL);
    return;
  }
#endif
  if (sync_ind->sb_flag)
  {
    if (GET_STATE (STATE_CS) EQ CS_ACTIVE_SYNC)
    {
      USHORT arfcn;
      PALLOC(new_scell, MPHC_NEW_SCELL_REQ);

      arfcn = ARFCN_STD_TO_G23(sync_ind->radio_freq, alr_data->cs_data.std);
      ALR_TRACE_CS_SYNC_VALID(arfcn, sync_ind->fn_offset,
                              sync_ind->time_alignment);

      /* store data in nc_data.cr_cell */
      alr_data->nc_data.cr_cell.ba_arfcn     =
        ARFCN_TO_G23(sync_ind->radio_freq)&ARFCN_MASK;
      alr_data->nc_data.cr_cell.bsic         = sync_ind->bsic;
      alr_data->nc_data.cr_cell.frame_offset = 0;
      alr_data->nc_data.cr_cell.time_align   = 0;

      new_scell->radio_freq     = sync_ind->radio_freq;
      new_scell->fn_offset      = sync_ind->fn_offset;
      new_scell->time_alignment = sync_ind->time_alignment;
      new_scell->tsc            = sync_ind->bsic;

      ALR_EM_BSIC_CONFIRM(EM_AVAIL);

      /* after successful sync we can use narrow band search mode for 
       * subsequent syncs.
       */
      alr_data->cs_data.search_mode = SM_NARROW_MODE;
      alr_data->cs_data.sync_fail_count = CS_SYNC_FAIL_COUNT_MAX;

      alr_data->sc_band = get_band (arfcn);
      ma_new_scell_req(new_scell);
    }
  }
  else
  {
    ALR_TRACE_CS ("INVALID BLOCK");

    ALR_EM_BSIC_CONFIRM(EM_NOT_AVAIL);
    /* If there are too many failed sync attempts in a row the AFC value
     * in L1 might be screwed up somehow.
     */
    if(alr_data->cs_data.sync_fail_count EQ 0)
    {
      alr_data->cs_data.sync_fail_count = CS_SYNC_FAIL_COUNT_MAX;
      alr_data->cs_data.search_mode = SM_WIDE_MODE;
    }
    else
    {
      alr_data->cs_data.sync_fail_count -= 1;
    }

  }
  SET_STATE(STATE_CS, CS_NULL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_stop                    |
+--------------------------------------------------------------------+

  PURPOSE : Process signal cs_stop from SDL process
            Main_Control. This function stops all cell selection
         activities of ALR.

*/

GLOBAL void cs_stop (void)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_CS))
  {
    case CS_ACTIVE_MEASURE:
    {
      PALLOC (stop_req, MPHC_STOP_RXLEV_REQ);
      PSENDX (L1, stop_req);
      if ( IS_EXT_MEAS_RUNNING )
      {
        /* wait for MPHC_RXLEV_IND */
        alr_data->cs_data.ext_meas_state_pend = CS_ACTIVE_MEASURE;
      }
      break;
    }
    case CS_ACTIVE_SYNC:
    {
      PALLOC (stop_req, MPHC_STOP_NETWORK_SYNC_REQ);
      TIMERSTOP(TIM_NW_SYNC_GUARD);
      PSENDX (L1, stop_req);
      break;
    }
    case CS_ACTIVE_BCCH:
      /*
       * Stop BCCH reading
       */
      ma_stop_scell_bcch_req ();
      break;
    default:
      break;
  }
  SET_STATE (STATE_CS, CS_NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_read_scell_bcch         |
+--------------------------------------------------------------------+

  PURPOSE : Process signal cs_read_scell_bcch from SDL process
            Main_Control. This funtion requests reading of the full
            serving cell BCCH.

*/
GLOBAL void cs_read_scell_bcch (void)
{
  GET_INSTANCE_DATA;
    /*
     * send bcch req
     * do full normal BCCH reading(modulus=1,position=0)
     */
    SET_STATE(STATE_CS, CS_ACTIVE_BCCH);
    ma_scell_full_nbcch();
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_add_and_sort_channels   |
+--------------------------------------------------------------------+

  PURPOSE : Accumulate power measurements for all found channels
            sorted by highest fieldstrength.
*/

LOCAL void cs_add_and_sort_channels (void)
{
  GET_INSTANCE_DATA;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;
  T_MPH_POWER_REQ* mph_power_req = alr_data->cs_data.p_power_req;
  T_POWER_MEAS *p_results[MAX_REGIONS];
  T_POWER_ARRAY *parray, *pbig, *last;
  SHORT    rxlev, min_rxlev[MAX_NUM_BANDS+1];
  /*lint -e644 (Warning -- Variable 'band_index'  may not have been initialized) */
  UBYTE x, band_index = 0;
  /*lint +e644 (Warning -- Variable 'band_index'  may not have been initialized) */
  U16      i, j, p_results_size[MAX_REGIONS], arfcn;
  U8       radio_band_config, std[MAX_REGIONS];
  U8       no_of_attempts, region, where_to_add;
  U8       i_cnf, extra_cnf, extra_space;
  U8       no_of_carriers_per_band[4] = {0, 0, 0, 0}; /* Counter for Multible frequency band in a Region
                                                       * Index 0 for GSM_900,
                                                       * Index 1 for DCS_1800,
                                                       * Index 2 for GSM_850,
                                                       * Index 3 for PCS_1900
                                                       */
  /* Obtain data for European region */
  p_results[EUROPEAN_REGION]      = alr_data->cs_data.p_results1;
  p_results_size[EUROPEAN_REGION] = alr_data->cs_data.p_results1 ? 
    (alr_data->cs_data.p_results1->power_array_size) : 0;

  /* Obtain data for American region */
  p_results[AMERICAN_REGION]      = alr_data->cs_data.p_results2;
  p_results_size[AMERICAN_REGION] = alr_data->cs_data.p_results2 ? 
    alr_data->cs_data.p_results2->power_array_size : 0;

  TRACE_FUNCTION ( "cs_add_and_sort_channels()" );

  if (!mph_power_cnf)
  {
    TRACE_EVENT ("mph_power_cnf EQ NULL");
    SET_STATE (STATE_CS, CS_NULL);
    return;
  }

  i_cnf = 0;
  extra_cnf = MAX_CHANNELS - 1;
  mph_power_cnf->num_of_chan = 0;


  /* Set the minimum signal level */
  if ( IS_EXT_MEAS_RUNNING  )
  {
    no_of_attempts = power_scan_attempts[FULL_SEARCH_MODE];
    for(x=0 ; x<=MAX_NUM_BANDS ; x++ )
        min_rxlev[x] = SHRT_MIN + 1;
  }
  else
  {
    if (!mph_power_req)
    {
      TRACE_EVENT ("mph_power_req EQ NULL");
      SET_STATE (STATE_CS, CS_NULL);
      return;
    }

    no_of_attempts = power_scan_attempts[mph_power_req->search_mode];
    for(x=0 ; x<MAX_NUM_BANDS ; x++ )
        min_rxlev[x]      = mph_power_req->lower_rxlevel_threshold[x] * no_of_attempts;
    /* If the std value cannot be retreived use the 
      * default lower_rxlev_threshold value of 4
      */
    min_rxlev[MAX_NUM_BANDS] = LOWER_RXLEV_THRESHOLD; 
  }

  TRACE_EVENT_P5 ("cs_add_and_sort_channels: std12=%02x max=%u/%u a='%c%c'",
    alr_data->cs_data.std12,
    p_results_size[EUROPEAN_REGION],
    p_results_size[AMERICAN_REGION],
    (alr_data->cs_data.freq_area & EUROPEAN_FREQ_AREA) ? 'E' : ' ',
    (alr_data->cs_data.freq_area & AMERICAN_FREQ_AREA) ? 'A' : ' ');

  if ((alr_data->cs_data.freq_area&ALL_FREQ_AREA) EQ ALL_FREQ_AREA)
  {
    /* use 'std' values saved by cs_prepare_power_req() */
    std[EUROPEAN_REGION] = alr_data->cs_data.std12&0x0f;
    std[AMERICAN_REGION] = alr_data->cs_data.std12>>4;
  }
  else
  {
    /* only one area */
    std[EUROPEAN_REGION] = std[AMERICAN_REGION] = alr_data->cs_data.std;
    if (alr_data->cs_data.freq_area & EUROPEAN_FREQ_AREA)
      p_results_size[AMERICAN_REGION] = 0;
    if (alr_data->cs_data.freq_area & AMERICAN_FREQ_AREA)
      p_results_size[EUROPEAN_REGION] = 0;
  }

  if( IS_EXT_MEAS_RUNNING )
  {
    /* 
     * According to 3GPP 05 08
     * Section "Range of parameter RxLev"
     *
     * The measured signal level shall be mapped to an RXLEV value between 0 and 63, as follows:
     * RXLEV 0 = less than    -110 dBm + SCALE.
     * RXLEV 1 =  -110 dBm + SCALE  to  -109 dBm + SCALE.
     * RXLEV 2 =  -109 dBm + SCALE  to  -108 dBm + SCALE.
     * :
     * :
     * RXLEV 62 =  -49 dBm + SCALE  to  -48 dBm + SCALE.
     * RXLEV 63 = greater than   -48 dBm + SCALE.
     * where SCALE is an offset that is used only in the ENHANCED MEASUREMENT REPORT message, 
     * otherwise it is set to 0. 
     */

    for(i=0; i < p_results_size[EUROPEAN_REGION]; i++)
      if( p_results[EUROPEAN_REGION]->power_array->accum_power_result < 0 )
        p_results[EUROPEAN_REGION]->power_array->accum_power_result = 0;

    for(i=0; i < p_results_size[AMERICAN_REGION]; i++)
      if( p_results[AMERICAN_REGION]->power_array->accum_power_result < 0 )
        p_results[AMERICAN_REGION]->power_array->accum_power_result = 0;
  }
  else
  {
    /* LLD Section : 4.1.3.4.2
     * Find all inactive carriers and add them to MPH_POER_CNF
     * Also set the Rxlev of all Black Listed carriers to less than
     * Lower_Rxlev_Threshold
     */

    TRACE_EVENT_P2("BIC->PWR array size, E:%d, A:%d", 
                    p_results_size[0], p_results_size[1]);

    cs_find_inactive_carriers(p_results, p_results_size,
                              std, no_of_attempts,
                              min_rxlev);

    TRACE_EVENT_P2("AIC->PWR array size, E:%d, A:%d",
                     p_results_size[0], p_results_size[1]);

    /*
     * If the search mode is BLACK_LIST_SEARCH_MODE no need for sorting 
     * (based on RXLEV) the carriers (RR will look only for
     * inactive carrier list)
     */
    if(mph_power_req->search_mode EQ BLACK_LIST_SEARCH_MODE)
    {
      /* Allow measurement indications posting to RR */
      SET_STATE(STATE_NC,NC_IDLE);
      return;
    }

    /* Put whitelist carriers at the top of power cnf list */
    if(mph_power_req->white_list.white_list_valid)
    {
      region = mph_power_req->white_list.region;

      switch(mph_power_req->white_list.region)
      {
        case EUROPEAN_REGION :
        case AMERICAN_REGION :
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
        case BOTH_REGIONS:
          i_cnf = cs_add_whitelist_carriers(p_results_size, std,
                                            no_of_attempts,
                                            min_rxlev, p_results,
                                            no_of_carriers_per_band);
#else
          i_cnf = cs_add_whitelist_carriers(p_results_size,
                                            std[region], no_of_attempts,
                                            min_rxlev, p_results[region],
                                            no_of_carriers_per_band);
#endif
          TRACE_EVENT_P5(
            "[%c]White list Area (B_GSM_EGSM:%d, B_1800:%d, B_850:%d, B_1900:%d)",
            (mph_power_req->white_list.region ? 'A' : 'E'),
            no_of_carriers_per_band[0], no_of_carriers_per_band[1],
            no_of_carriers_per_band[2], no_of_carriers_per_band[3]);          
          break;
        default :
          TRACE_EVENT_P1("Invalid whitelist region:%d",region);
          break;
      }
    } /* white list valid */
    else
    {
      TRACE_EVENT("WL is absent");
    }
  } /* !ext_meas */

  TRACE_EVENT_P3("AWL->PWR array size, E:%d A:%d i_cnf:%d",
                  p_results_size[0], p_results_size[1],i_cnf);

  while (i_cnf < MAX_CHANNELS)
  {
    pbig=NULL;
    rxlev = 0;
    radio_band_config = where_to_add =0x00;

    /* Loop through both regions */
    for (i=0;i<MAX_REGIONS;i++)
    {
      if((p_results_size[i]) AND (p_results[i] NEQ NULL))
      {
        parray = p_results[i]->power_array;

        for (j=0; j<p_results_size[i]; j++, parray++)
        {
          arfcn = ARFCN_STD_TO_G23(parray->radio_freq, std[i]);
          /*lint -e661 (Warning -- Possible access of out-of-bounds) */
          get_band_index_from_arfcn(arfcn, x, std[i]); 
          if (parray->accum_power_result >= min_rxlev[x] AND parray->accum_power_result > rxlev)
          {
            pbig = parray;
            rxlev = parray->accum_power_result;
            radio_band_config = std[i];
            region = (U8) i;
            band_index = x;
          }
          /*lint +e661 (Warning -- Possible access of out-of-bounds) */
        }
      }
    }

    if( pbig NEQ NULL )
    {
      arfcn = ARFCN_STD_TO_G23(pbig->radio_freq, radio_band_config);
      if (rxlev > (min_rxlev[band_index] -1))
      {
        /* fill mph_power_cnf */
        arfcn = STD_ADD_TO_ARFCN(arfcn, radio_band_config);

        /* US_BIT should be used to differentiate an US frequency channel. */
        switch (radio_band_config)
        {
          case STD_1900:
          case STD_850:
          case STD_DUAL_US:
            arfcn |= US_BIT;
            break;
          default:
            break;
        }

        where_to_add = cs_restrict_max_carriers_per_band(
                         arfcn&ARFCN_MASK, 
                         radio_band_config,
                         no_of_carriers_per_band,
                         p_results_size, min_rxlev[band_index]);

        if(where_to_add EQ ADD_AT_THE_TOP)
        {  
          /* First 40 Strongest Cariiers */
          mph_power_cnf->arfcn[i_cnf]  = arfcn;
          mph_power_cnf->rx_lev[i_cnf] = (U8)(rxlev/no_of_attempts);
  
          i_cnf++;
        }
        else if(where_to_add EQ ADD_AT_THE_BOTTOM)
        {
          /* Strongest Carriers which fall between 41 to 60 */
          if(extra_cnf >= i_cnf)
          {
            mph_power_cnf->arfcn[extra_cnf]  = arfcn;
            mph_power_cnf->rx_lev[extra_cnf] = (U8)(rxlev/no_of_attempts);

            extra_cnf--;
          }
          else
          {
            TRACE_EVENT_P2("MPH_POWER_CNF crossover, i_cnf: %d extra_cnf: %d",
              i_cnf, extra_cnf);
          }
        }

       /* After adding a carrier to MPH_POWER_CNF, the particular carrier will be 
        * replaced by the Last carrier of that region. So that we could avoid
        * searching the already added (MPH_POWER_CNF) carrier.
        */
        if(where_to_add NEQ REACHED_THE_MAXIMUM)
        {
          last = p_results[region]->power_array + (p_results_size[region]-1);/*lint !e644 region may not have been initialized */
  
          pbig->accum_power_result = min_rxlev[band_index] - 1;
        
          cs_power_array_swap_arfcn(pbig, last);
  
          p_results_size[region]--;
        }
        else
        {
          TRACE_EVENT_P4("RTM->PWR array size, E:%d A:%d i_cnf:%d extra_cnf:%d",
            p_results_size[0], p_results_size[1],i_cnf, 
            (MAX_CHANNELS-(extra_cnf+1)));
        }
      }
    }
    else
      break; /* no level found higher or equal than min_rxlev -> break sort/fill */

  }/* while (i_cnf < MAX_CHANNELS) */

  mph_power_cnf->num_of_chan = i_cnf;

  /* Obtain the number of extra channels(41 to 60) added to 
   * Power cnf array 
   */
  extra_cnf   = MAX_CHANNELS - (extra_cnf+1);

  /* Obtain the amount of space available for extra channels
   * in power_cnf array
   */
  extra_space = MAX_CHANNELS - i_cnf;

  TRACE_EVENT_P3("After Sorting, i_cnf:%d extra_cnf:%d extra_space:%d",
                 i_cnf,extra_cnf,(extra_space-extra_cnf));

  if(extra_cnf AND extra_space)
  {
    /* Extra channels are present and space to fit them is also available */
    if(extra_space < extra_cnf)
    {
      /* Some of the extra channels(41 to 60) are overwritten
       * Ignore them
       */
      extra_cnf = extra_space;
    }

     /* Reordering the carriers(41 to 60) from Strongest to Weakest */
     cs_reorder_the_extra_carriers(extra_cnf);

     /* Move the extra carriers up in power_cnf array, in case there
      * is empty gap between i_cnf and extra_cnf channels
      */
     if(extra_space > extra_cnf)
       cs_move_extra_carriers(i_cnf, extra_cnf);

     mph_power_cnf->num_of_chan += extra_cnf;
  }

  TRACE_EVENT_P5(
    "No. of carriers in POWER_CNF:%d (B_GSM_EGSM:%d, B_1800:%d, B_850:%d, B_1900:%d)",
    mph_power_cnf->num_of_chan,
    no_of_carriers_per_band[0], no_of_carriers_per_band[1],
    no_of_carriers_per_band[2], no_of_carriers_per_band[3]);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : get_band                   |
+--------------------------------------------------------------------+

  PURPOSE : The function extracts the frequency band from the given
            'arfcn' parameter.
*/

GLOBAL UBYTE get_band (USHORT arfcn)
{
  UBYTE local_std = STD_GET_FROM_ARFCN(arfcn);
  UBYTE sc_band;

  if (local_std EQ 0)
    local_std = std;

  switch (local_std)
  {
    case STD_900:
      sc_band = BAND_GSM_900;
      break;

    case STD_EGSM:
      sc_band = BAND_E_GSM;
      break;

    case STD_1800:
      sc_band = BAND_DCS_1800;
      break;

    case STD_1900:
      sc_band = BAND_PCS_1900;
      break;

    case STD_850:
      sc_band = BAND_GSM_850;
      break;

    case STD_DUAL:
      if (arfcn >= LOW_CHANNEL_1800)
        sc_band = BAND_DCS_1800;
      else
        sc_band = BAND_GSM_900;
      break;

    case STD_DUAL_EGSM:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1800)
        sc_band = BAND_DCS_1800;
      else if (arfcn EQ CHANNEL_0)
        sc_band = BAND_E_GSM;
      else
        sc_band = BAND_GSM_900;
      break;

    case STD_DUAL_US:
      if (arfcn >= LOW_CHANNEL_1900)
        sc_band = BAND_PCS_1900;
      else
        sc_band = BAND_GSM_850;
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      if (arfcn >= LOW_CHANNEL_1800)
        sc_band = BAND_DCS_1800;
      else
        sc_band = BAND_GSM_850;
      break;

    case STD_900_1900:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1900)
        sc_band = BAND_PCS_1900;
      else if (arfcn EQ CHANNEL_0)
        sc_band = BAND_E_GSM;
      else
        sc_band = BAND_GSM_900;
      break;

    case STD_850_900_1800:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1800)
        sc_band = BAND_DCS_1800;
      else if (arfcn EQ CHANNEL_0)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_850)
        sc_band = BAND_GSM_850;
      else
        sc_band = BAND_GSM_900;
      break;

    case STD_850_900_1900:
      if (arfcn >= LOW_CHANNEL_EGSM)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_1900)
        sc_band = BAND_PCS_1900;
      else if (arfcn EQ CHANNEL_0)
        sc_band = BAND_E_GSM;
      else if (arfcn >= LOW_CHANNEL_850)
        sc_band = BAND_GSM_850;
      else
        sc_band = BAND_GSM_900;
      break;
#endif

    default:
      sc_band = 0;
      break;
  }
  /* this trace causes a lot of trace load; switch on only if needed
  TRACE_EVENT_P2 ("[%u] sc_band=%02x", arfcn&ARFCN_MASK, sc_band);
  */
  return sc_band;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_increment_bfc           |
+--------------------------------------------------------------------+

  PURPOSE : The function increments the BCCH fail counter. This 
            counter is decremented on every invalid BCCH block read 
            during CS. It is incremented on evry valid block read on
            BCCH during CS. Valid means in this case that there was a
            SYS Info decodable in the block on the BCCH. There is a 
            maximum value to start with and when the counter reaches 
            0 we switch back to wide band search mode for further
            MPHC_NETWORK_SYNC_REQs (if any).
*/

GLOBAL void cs_increment_bfc (void)
{
  GET_INSTANCE_DATA;
  alr_data->cs_data.bcch_fail_count += 1;
  if(alr_data->cs_data.bcch_fail_count > CS_BCCH_FAIL_COUNT_MAX)
    alr_data->cs_data.bcch_fail_count = CS_BCCH_FAIL_COUNT_MAX;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_decrement_bfc           |
+--------------------------------------------------------------------+

  PURPOSE : The function decrements the BCCH fail counter. This 
            counter is decremented on every invalid BCCH block read 
            during CS. It is incremented on evry valid block read on
            BCCH during CS. Valid means in this case that there was a
            SYS Info decodable in the block on the BCCH. There is a 
            maximum value to start with and when the counter reaches 
            0 we switch back to wide band search mode for further
            MPHC_NETWORK_SYNC_REQs (if any).
*/

GLOBAL void cs_decrement_bfc (void)
{
  GET_INSTANCE_DATA;
  if(alr_data->cs_data.bcch_fail_count < 3)
  {
    alr_data->cs_data.bcch_fail_count = 0;
    alr_data->cs_data.search_mode = SM_WIDE_MODE;
  }
  else
    alr_data->cs_data.bcch_fail_count -= 2;

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_set_wideband_sync       |
+--------------------------------------------------------------------+

  PURPOSE : The function sets wide band search mode for further
            MPHC_NETWORK_SYNC_REQs (if any).
*/

GLOBAL void cs_set_wideband_sync (void)
{
  GET_INSTANCE_DATA;
  alr_data->cs_data.search_mode = SM_WIDE_MODE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_is_in_black_list        |
+--------------------------------------------------------------------+

  PURPOSE : The function checks whether the given carrier is in the
            Blacklist or not
            CSI-LLD section: 4.1.3.4.2.1
*/

LOCAL BOOL cs_is_in_black_list(U8 region,U16 arfcn)
{
  GET_INSTANCE_DATA;
  BOOL ret = FALSE;

  if((region EQ EUROPEAN_REGION) OR (region EQ AMERICAN_REGION))
  {
    if(alr_data->cs_data.p_power_req->search_mode EQ FULL_SEARCH_MODE)
    {
      return ret;
    }

    ret = srv_get_channel((T_LIST*)&alr_data->cs_data.p_power_req->black_list.list[region],
                            arfcn&ARFCN_MASK);

    if(ret)
    {
      if(srv_get_region_from_std(alr_data->cs_data.std) == region
         AND nc_is_in_ba(arfcn&ARFCN_MASK))
      {
        TRACE_EVENT_P1 ("ARFCN %d found in BA and Black list", arfcn & ARFCN_MASK);
        ret = FALSE;
      }
    }    

  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_find_inactive_carriers  |
+--------------------------------------------------------------------+

  PURPOSE : 1. Finds all inactive carriers
            2. Sets the RxLev of all Blacklisted carriers to less than
               LOWER_RXLEV_THRESHOLD
            CSI-LLD Section: 4.1.3.8.2.2.1
            CSI-LLD Section: 4.2
*/

LOCAL void cs_find_inactive_carriers (T_POWER_MEAS **p_results,
                                      U16 p_results_size[2],U8 *std,
                                      U8 no_of_attempts, SHORT *min_rxlev)
{
  GET_INSTANCE_DATA;
  T_POWER_ARRAY* parray, *last;
  U16 j, arfcn;
  U8 i, x=0;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;

  TRACE_FUNCTION("cs_find_inactive_carriers ()");

  /* Loop through both regions */
  for (i=0;i<MAX_REGIONS;i++)
  {
    if((p_results_size[i]) AND (p_results[i] NEQ NULL))
    {
      parray = p_results[i]->power_array;

      for (j=0; j<p_results_size[i];)
      {
        arfcn = ARFCN_STD_TO_G23(parray->radio_freq, std[i]);
        /* Pointer to the Last Power array for a particular region */
        last = (p_results[i]->power_array + (p_results_size[i]-1));
        /*lint -e661 (Warning -- Possible access of out-of-bounds) */
        get_band_index_from_arfcn(arfcn, x, std[i]);
        if (parray->accum_power_result < min_rxlev[x])
        {
        /*lint +e661 (Warning -- Possible access of out-of-bounds) */
          /* Inactive carrier */
          srv_set_channel((T_LIST*)&mph_power_cnf->inactive_carrier_list.list[i],
                          arfcn&ARFCN_MASK);
          
          /* Replace inactive carrier with the last active carrier */
          cs_power_array_swap_arfcn(parray,last);
 
          /* Decrement the power array size to exclude this carrier */
          p_results_size[i]--;
        }
        else if(cs_is_in_black_list(i, (U16)(arfcn&ARFCN_MASK)))
        {
          /* Carrier is black listed. No need to consider this */
          /*lint -e661 (Warning -- Possible access of out-of-bounds) */
          parray->accum_power_result = min_rxlev[x]-1;
          /*lint +e661 (Warning -- Possible access of out-of-bounds) */
          /* Replace inactive carrier with the last active carrier */
          cs_power_array_swap_arfcn(parray,last);
 
          /* Decrement the power array size to exclude this carrier */
          p_results_size[i]--;

        }
        else
        {
          j++, parray++;
        }
      } /* for size */
    } /* if size */
  }  /* MAX_REGIONS */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE : cs_whitelist_handle        |
+--------------------------------------------------------------------+

  PURPOSE : This function puts the White carriers at the top of the 
            MPH_POWER_CNF list
            CSI-LLD 4.1.3.4.2.7

  FreeCalypso note: when TI reworked this function for their
  TI_PS_FF_QUAD_BAND_SUPPORT, they broke the configuration without this
  feature.  If one takes the version of ALR from the LoCosto source
  and builds it without TI_PS_FF_QUAD_BAND_SUPPORT, after passing through
  the C preprocessor this function will be reduced to an equivalent of
  return 0; followed by unreachable code.

  Our solution: let's put the #ifdef TI_PS_FF_QUAD_BAND_SUPPORT on the
  outside of the function, and have two entirely separate versions.
  If one builds with the new feature, the version from LoCosto will be
  built, made more readable by removing further internal ifdefs on the
  same condition.  The version to be built *without* TI_PS_FF_QUAD_BAND_SUPPORT
  has been taken from the ALR source that miraculously survived in the
  otherwise sanitized Leonardo semi-src.

*/

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
LOCAL U8 cs_add_whitelist_carriers(U16 p_results_size[2], U8 *r_std, U8 no_of_attempts, 
                                   SHORT *min_rxlev, T_POWER_MEAS **presults,
                                   U8 no_of_carriers_per_band[4])
{
  GET_INSTANCE_DATA;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;
  T_MPH_POWER_REQ* mph_power_req = alr_data->cs_data.p_power_req;
  U8  i_cnf,j, where_to_add = DO_NOT_ADD;
  U8  region = mph_power_req->white_list.region;
  U16 i,arfcn, temp_arfcn;
  U8  temp_rxlev;
  UBYTE x=0;
  T_POWER_ARRAY *parray, *last;
  U8 std, start_region, no_of_regions, k;

  TRACE_FUNCTION ("cs_add_whitelist_carriers()");

  i_cnf = 0;

  if (region EQ BOTH_REGIONS)
  {
    if((!(p_results_size[EUROPEAN_REGION]) OR (presults[EUROPEAN_REGION] EQ NULL)) AND 
       (!(p_results_size[AMERICAN_REGION]) OR (presults[AMERICAN_REGION] EQ NULL)))
       return i_cnf;
  }
  else if(!(p_results_size[region]) OR (presults EQ NULL))
  {
    return i_cnf;
  }

  /* Move the white list carriers to MPH_POWER_CNF array first */
  if (region EQ BOTH_REGIONS)
  {
    start_region = EUROPEAN_REGION;
    no_of_regions = MAX_REGIONS;
  }
  else if (region EQ EUROPEAN_REGION)
  {
    start_region = region;
    no_of_regions = MAX_REGIONS - 1;
  }
  else
  {
    start_region = region;
    no_of_regions = MAX_REGIONS;
  }
  
  for (k=start_region; k< no_of_regions; k++)
  {
    std = r_std[k];
    parray = presults[k]->power_array;

    for (i=0; (i<p_results_size[k] AND i_cnf < 32); )
    {
      /* Convert to GSM standard format from L1 format*/
      arfcn = ARFCN_STD_TO_G23(parray->radio_freq,std);
      /*lint -e661 (Warning -- Possible access of out-of-bounds) */
      get_band_index_from_arfcn(arfcn, x, std);
      if (parray->accum_power_result > (min_rxlev[x] - 1))
      {
        /*lint +e661 (Warning -- Possible access of out-of-bounds) */
        if(srv_get_channel((T_LIST*)&mph_power_req->white_list.list, arfcn&ARFCN_MASK))
        {

          /* Channel is present in white list. Add this to top of MPH_POWER_CNF */
          arfcn = STD_ADD_TO_ARFCN(arfcn, std);

          /* US_BIT should be used to differentiate an US frequency channel. */
          switch (std)
          {
            case STD_1900:
            case STD_850:
            case STD_DUAL_US:
              arfcn |= US_BIT;
              break;
            default:
              break;
          }
          /*lint -e661 (Warning -- Possible access of out-of-bounds) */
          where_to_add = cs_restrict_max_carriers_per_band(arfcn&ARFCN_MASK, std,
                                       no_of_carriers_per_band, p_results_size, min_rxlev[x]);
          /*lint +e661 (Warning -- Possible access of out-of-bounds) */
          if(where_to_add NEQ DO_NOT_ADD)
          {
            /* White list carriers are always added at the top */
            mph_power_cnf->arfcn[i_cnf]  = arfcn;
            mph_power_cnf->rx_lev[i_cnf] = (U8)(parray->accum_power_result/no_of_attempts);
            i_cnf++;
          }
          else
          {
            TRACE_EVENT_P2("[WL] [DO_NOT_ADD] [%d] : [%c]", arfcn&ARFCN_MASK,
                            (k ? 'A' :'E'));
          }

          /* Exclude this carrier */
          parray->accum_power_result = min_rxlev[x]-1;

          last = presults[k]->power_array + (p_results_size[k] - 1);

          /* Swapping the current carrier with the last carrier */
          cs_power_array_swap_arfcn(parray, last);

          /* Decrement the power array counter to exclude the above carrier */
          p_results_size[k]--;

        } /* Present in White List */
        else
        {
          i++; parray++;
        }
      } /* Active Carrier */
      else 
      {
        TRACE_EVENT_P2("[WL] [IA] [%d] : [%c]", parray->radio_freq,
                        (k ? 'A' :'E'));
        i++; parray++;
      }
    } /* i < max */
  }

  /* Assign the total Number of white list channels */
  mph_power_cnf->num_of_white_list_chan = i_cnf;

  TRACE_EVENT_P1("[WL] no. of channels : %d ",mph_power_cnf->num_of_white_list_chan);

  /*
   * Sort the white list carriers added to power_cnf array on the 
   * basis of their field strength
   */
  for(i=0; i < i_cnf; i++)
  {
    for(j=i+1; j<i_cnf; j++)
    {
      if(mph_power_cnf->rx_lev[i] < mph_power_cnf->rx_lev[j])
      {
        temp_rxlev = mph_power_cnf->rx_lev[i];
        temp_arfcn = mph_power_cnf->arfcn[i];

        mph_power_cnf->rx_lev[i] = mph_power_cnf->rx_lev[j];
        mph_power_cnf->arfcn[i]  = mph_power_cnf->arfcn[j];

        mph_power_cnf->rx_lev[j] = temp_rxlev;
        mph_power_cnf->arfcn[j]  = temp_arfcn;
      }
    }
  }

  return (i_cnf);
}
#else	/* no TI_PS_FF_QUAD_BAND_SUPPORT */

/*
 * Version taken from the TCS211 ALR source, modified in a few places
 * to work with the rest of ALR which is the version from LoCosto.
 */

LOCAL U8 cs_add_whitelist_carriers(U16 p_results_size[2], U8 std, U8 no_of_attempts, 
                                   SHORT *min_rxlev,
                                   T_POWER_MEAS *presults,
                                   U8 no_of_carriers_per_band[4])
{
  GET_INSTANCE_DATA;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;
  T_MPH_POWER_REQ* mph_power_req = alr_data->cs_data.p_power_req;
  U8  i_cnf,j, where_to_add = DO_NOT_ADD;
  U8  region = mph_power_req->white_list.region;
  U16 i,arfcn, temp_arfcn;
  U8  temp_rxlev;
  UBYTE x;
  T_POWER_ARRAY *parray, *last;

  TRACE_FUNCTION ("cs_add_whitelist_carriers()");

  i_cnf = 0;

  if((p_results_size[region]) AND (presults NEQ NULL))
  {
    parray = presults->power_array;
  }
  else
    return i_cnf;

  /* Move the white list carriers to MPH_POWER_CNF array first */
  for (i=0; (i<p_results_size[region] AND i_cnf < 32); )
  {

    /* Convert to GSM standard format from L1 format*/
    arfcn = ARFCN_STD_TO_G23(parray->radio_freq,std);
    get_band_index_from_arfcn(arfcn, x, std);
    if (parray->accum_power_result > (min_rxlev[x] - 1))
    {
      if(srv_get_channel((T_LIST*)&mph_power_req->white_list.list, arfcn&ARFCN_MASK))
      {

        /* Channel is present in white list. Add this to top of MPH_POWER_CNF */
        arfcn = STD_ADD_TO_ARFCN(arfcn, std);

        /* US_BIT should be used to differentiate an US frequency channel. */
        switch (std)
        {
          case STD_1900:
          case STD_850:
          case STD_DUAL_US:
            arfcn |= US_BIT;
            break;
          default:
            break;
        }

        where_to_add = cs_restrict_max_carriers_per_band(arfcn&ARFCN_MASK, std,
                                       no_of_carriers_per_band, p_results_size,
				       min_rxlev[x]);

        if(where_to_add NEQ DO_NOT_ADD)
        {
          /* White list carriers are always added at the top */
          mph_power_cnf->arfcn[i_cnf]  = arfcn;
          mph_power_cnf->rx_lev[i_cnf] = (U8)(parray->accum_power_result/no_of_attempts);
          i_cnf++;
        }
        else
        {
          TRACE_EVENT_P2("[WL] [DO_NOT_ADD] [%d] : [%c]", arfcn&ARFCN_MASK,
                          (region ? 'A' :'E'));
        }

        /* Exclude this carrier */
        parray->accum_power_result = min_rxlev[x]-1;

        last = presults->power_array + (p_results_size[region] - 1);

        /* Swapping the current carrier with the last carrier */
        cs_power_array_swap_arfcn(parray, last);

        /* Decrement the power array counter to exclude the above carrier */
        p_results_size[region]--;

      } /* Present in White List */
      else
      {
        i++; parray++;
      }
    } /* Active Carrier */
    else 
    {
      TRACE_EVENT_P2("[WL] [IA] [%d] : [%c]", parray->radio_freq,
                      (region ? 'A' :'E'));
      i++; parray++;
    }
  } /* i < max */

  /* Assign the total Number of white list channels */
  mph_power_cnf->num_of_white_list_chan = i_cnf;

  TRACE_EVENT_P1("[WL] no. of channels : %d ",mph_power_cnf->num_of_white_list_chan);

  /*
   * Sort the white list carriers added to power_cnf array on the 
   * basis of their field strength
   */
  for(i=0; i < i_cnf; i++)
  {
    for(j=i+1; j<i_cnf; j++)
    {
      if(mph_power_cnf->rx_lev[i] < mph_power_cnf->rx_lev[j])
      {
        temp_rxlev = mph_power_cnf->rx_lev[i];
        temp_arfcn = mph_power_cnf->arfcn[i];

        mph_power_cnf->rx_lev[i] = mph_power_cnf->rx_lev[j];
        mph_power_cnf->arfcn[i]  = mph_power_cnf->arfcn[j];

        mph_power_cnf->rx_lev[j] = temp_rxlev;
        mph_power_cnf->arfcn[j]  = temp_arfcn;
      }
    }
  }

  return (i_cnf);
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : ALR_CS                     |
| STATE   : code                ROUTINE :                            |
|                                  cs_restrict_max_carriers_per_band |
+--------------------------------------------------------------------+
  PURPOSE : This function is to handle the Multiple Frequency Bands 
            in a Region. Will help to add the minimum(40) carriers per
            band on top of the power cnf and the remaining(40 to 60) 
            carriers will be added in at the end (Below the Normal (40)
            carriers of all band).
            CSI-LLD : 
*/

U8  cs_restrict_max_carriers_per_band (U16 arfcn, U8 std, 
                U8 no_of_carriers_per_band[4], U16 p_results_size[2], UBYTE min_rxlev)
{
  GET_INSTANCE_DATA;

  T_POWER_MEAS    *presults;
  T_POWER_ARRAY   *parray, *last;
  U16              i;
  U8              index = 0xff,just_reached_the_maximum=FALSE;
  U8              region, where_to_add = DO_NOT_ADD; 
  
  switch(std)
  {
    case STD_900       :
    case STD_DUAL      :
    case STD_EGSM      :
    case STD_DUAL_EGSM :
    case STD_1800      :
      if(INRANGE (CHANNEL_0, arfcn, HIGH_CHANNEL_900) OR 
         INRANGE(LOW_CHANNEL_EGSM, arfcn, HIGH_CHANNEL_EGSM))
      {
        index = 0;
      }
      else if(INRANGE (LOW_CHANNEL_1800, arfcn, HIGH_CHANNEL_1800))
      {
        index = 1;
      }
      else
      {
        TRACE_ERROR("[European]Invalid Carrier");
        return DO_NOT_ADD;
      }
      region = EUROPEAN_REGION;
      break; 
    case STD_DUAL_US :
    case STD_850     :
    case STD_1900    :
      if(INRANGE (LOW_CHANNEL_850, arfcn, HIGH_CHANNEL_850))
      {
        index = 2;
      }
      else if(INRANGE (LOW_CHANNEL_1900, arfcn, HIGH_CHANNEL_1900))
      {
        index = 3;
      }
      else
      {
        TRACE_ERROR("[American]Invalid Carrier");
        return DO_NOT_ADD;
      }
      region = AMERICAN_REGION;
      break;
  } /* end switch */

  if(index < 4)
  {
    if(no_of_carriers_per_band[index] < MIN_CHANNELS_PER_BAND)
    {
      /* Increment counter for corresponding band */
      no_of_carriers_per_band[index]++;

      if(no_of_carriers_per_band[index] EQ MIN_CHANNELS_PER_BAND)
      {      
        TRACE_EVENT_P5(
          "[%d]Band, 40 channels added (B_GSM_EGSM%d, B_1800:%d, B_850:%d, B_1900:%d)",
          index,
          no_of_carriers_per_band[0], no_of_carriers_per_band[1],
          no_of_carriers_per_band[2], no_of_carriers_per_band[3]);
      }

      /* Add at the top of MPH_POWER_CNF list */
      where_to_add = ADD_AT_THE_TOP;
    }
    else if(no_of_carriers_per_band[index] <= MAX_CHANNELS_PER_BAND)
    {
      /* Increment counter for corresponding band */
      no_of_carriers_per_band[index]++;

      /* Add at the bottom of MPH_POWER_CNF list */
      where_to_add = ADD_AT_THE_BOTTOM;

      if(no_of_carriers_per_band[index] EQ (MAX_CHANNELS_PER_BAND + 1))
      {
        TRACE_EVENT_P5(
          "[%d]Band, 60 channels added (B_GSM_EGSM%d, B_1800:%d, B_850:%d, B_1900:%d)",
          index,
          no_of_carriers_per_band[0], no_of_carriers_per_band[1],
          no_of_carriers_per_band[2], no_of_carriers_per_band[3]);

        just_reached_the_maximum = TRUE;

        where_to_add = REACHED_THE_MAXIMUM;
      }
    }
    else
    {
      where_to_add = DO_NOT_ADD;
    }
  }
  else
  {
    TRACE_ERROR("Unknown Band Index");
    return DO_NOT_ADD;
  } /* if index < 4 */

  /* When Max number of channels(60) are added for a particular band,
   * all the remaining carriers belonging to that band are excluded from
   * further sorting
   */
  if(just_reached_the_maximum)
  {
    BOOL exclude;
    U16  l3_arfcn;

    if (region EQ AMERICAN_REGION)/*lint !e644 region may not have been initialized */
    { /* American band */
      presults =  alr_data->cs_data.p_results2;
    }
    else
    { /* European band */
      presults =  alr_data->cs_data.p_results1;
    }

    TRACE_EVENT_P3("[%d]Index, E:%d A:%d",
      index, p_results_size[0], p_results_size[1]);
   
    if((presults NEQ NULL) AND (p_results_size[region]))/*lint !e644 region may not have been initialized */
    {
      for(i=0, parray = presults->power_array; i < p_results_size[region];)
      {
        exclude  = FALSE;
        l3_arfcn = ARFCN_STD_TO_G23(parray->radio_freq, std);
      
        switch(index)
        {
          case 0 :
            if(INRANGE (LOW_CHANNEL_900, l3_arfcn, HIGH_CHANNEL_900) OR 
               INRANGE(LOW_CHANNEL_EGSM, l3_arfcn, HIGH_CHANNEL_EGSM))
            {
              exclude = TRUE;
            }
            break;
          case 1:
            if(INRANGE (LOW_CHANNEL_1800, l3_arfcn, HIGH_CHANNEL_1800))
            {
              exclude = TRUE;
            }
            break;
          case 2 :
            if(INRANGE (LOW_CHANNEL_850, l3_arfcn, HIGH_CHANNEL_850))
            {
              exclude = TRUE;
            }
            break;
          case 3:
            if(INRANGE (LOW_CHANNEL_1900, l3_arfcn, HIGH_CHANNEL_1900))
            {
              exclude = TRUE;
            }
            break;
          default :
            break;
        } /* end switch */

        if(exclude)
        {
          last = presults->power_array + (p_results_size[region] - 1); /*lint !e644 region may not have been initialized */

          /* Exclude this carrier */
          parray->accum_power_result = min_rxlev-1;  

          /* Swaping the Current carrier with the last carrier */
          cs_power_array_swap_arfcn(parray, last);
  
          /* Decrement the power array counter to exclude the above carrier */
          p_results_size[region]--;
        }
        else
        {
          parray++; i++;
        }
      } /* power array size */
    } /* end if(just_reached_the_maximum */
  } /* presults NEQ NULL */

  return where_to_add;
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)    MODULE  : ALR_CS                        |
| STATE   : code             ROUTINE : cs_reorder_the_extra_carriers |
+--------------------------------------------------------------------+
  PURPOSE : Extra Carriers (More than 40 and below 60) are stored at
            the bottom of the MPH_POWER_CNF. But these are stored in
            ascending order (if you see from the TOP). This needs to 
            be reordered (means Strongest carrier should go to top).
*/

LOCAL void cs_reorder_the_extra_carriers(U8 extra_cnf)
{
  GET_INSTANCE_DATA;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;
  U8     i,j,count,max_count,temp_rxlevel;
  U16    temp_arfcn;

  TRACE_FUNCTION("cs_reorder_the_extra_carriers");

  i         = MAX_CHANNELS - 1;
  j         = MAX_CHANNELS - extra_cnf;
  max_count = extra_cnf/2;


    for(count=0; count < max_count; i--, j++, count++)
    {
      temp_arfcn               = mph_power_cnf->arfcn[i];
      temp_rxlevel             = mph_power_cnf->rx_lev[i];

      mph_power_cnf->arfcn[i]  = mph_power_cnf->arfcn[j];
      mph_power_cnf->rx_lev[i] = mph_power_cnf->rx_lev[j];

      mph_power_cnf->arfcn[j]  = temp_arfcn;
      mph_power_cnf->rx_lev[j] = temp_rxlevel;

    }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)     MODULE  : ALR_CS                       |
| STATE   : code              ROUTINE :                              |
|                              cs_move_extra_carriers                |
+--------------------------------------------------------------------+
  PURPOSE : Extra Carriers (More than 40 and below 60) are stored at
              the bottom of the MPH_POWER_CNF. These carriers needs to
              be rearranged below the normal (Strangest 40 Carriers)
              carriers
*/

LOCAL void cs_move_extra_carriers(U8 i_cnf, U8 extra_cnf)
{
  GET_INSTANCE_DATA;
  T_MPH_POWER_CNF* mph_power_cnf = alr_data->cs_data.p_power_cnf;

  /*
   *  Add the extra carriers below Normal carrier
   */
  TRACE_FUNCTION("cs_move_extra_carriers");

  /* Move the extra carriers below the Normal carriers */
  memmove (&mph_power_cnf->arfcn[i_cnf], 
           &mph_power_cnf->arfcn[MAX_CHANNELS - extra_cnf],
           sizeof (mph_power_cnf->arfcn[0]) * (extra_cnf));

  memmove (&mph_power_cnf->rx_lev[i_cnf],
           &mph_power_cnf->rx_lev[MAX_CHANNELS - extra_cnf],
           sizeof (mph_power_cnf->rx_lev[0]) * (extra_cnf));

}

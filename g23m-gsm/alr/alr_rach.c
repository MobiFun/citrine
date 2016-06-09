/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_RACH
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
|  Purpose :  This Modul defines the SDL process RACH_Control.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_RACH_C
#define ALR_RACH_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"
#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif

#include "alr.h"
#include "alr_em.h"

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
LOCAL void rach_send_mphc_ra_req (T_MPH_RANDOM_ACCESS_REQ  *random_req);
/*==== VARIABLES ==================================================*/
#if defined (WIN32)
#define TRACING
#endif

#if defined (TRACING)
#define ALR_TRACE_RACH(a)  ALR_TRACE(a)
#else
#define ALR_TRACE_RACH(a)
#endif

#if defined (TRACING)

#define ALR_TRACE_RACH_CLASS(x)   TRACE_EVENT_P1 ("class %d",x)
#define ALR_TRACE_RACH_POWER(p)   TRACE_EVENT_P1 ("configured power = %d", p)

#else

#define ALR_TRACE_RACH_CLASS(x)
#define ALR_TRACE_RACH_POWER(p)

#endif

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_init                  |
+--------------------------------------------------------------------+

  PURPOSE : Initialize RACH Control Process.

*/

GLOBAL void rach_init (void)
{
  GET_INSTANCE_DATA;
  alr_data->state[STATE_RA] = RA_NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_configure_class       |
+--------------------------------------------------------------------+

  PURPOSE : Process signal rach_configure_class from SDL process
            Main_Control.

*/
GLOBAL void rach_configure_class (UBYTE ms_class,
                                  UBYTE dcs_class)
{
  GET_INSTANCE_DATA;
  /*
   * +1 because of interface difference between RR and TI
   */
  alr_data->rach_data.ms_class     = (UBYTE)(ms_class  + 1);
  alr_data->rach_data.dcs_class    = (UBYTE)(dcs_class + 1);

  ALR_TRACE_RACH_CLASS (alr_data->rach_data.ms_class);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_configure_power       |
+--------------------------------------------------------------------+

  PURPOSE : Process signal rach_configure_power from SDL process
            Main_Control.

*/

GLOBAL void rach_configure_power (UBYTE power)
{
  GET_INSTANCE_DATA;
  ALR_TRACE_RACH_POWER(power);

  alr_data->rach_data.max_tx_pwr_ccch     = power;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_stop                  |
+--------------------------------------------------------------------+

  PURPOSE : Process signal rach_stop from SDL process
            Main_Control.
*/

GLOBAL void rach_stop (void)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_RA))
  {
    case RA_ACTIVE:
      ma_rach_stop_ra_req ();
      /*lint -fallthrough*/
    case RA_LISTEN_TO_CCCH:
      SET_STATE (STATE_RA, RA_NULL);
      break;

    default:
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_random_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process signal rach_random_req from SDL process
            Main_Control.

*/

GLOBAL void rach_random_req (T_MPH_RANDOM_ACCESS_REQ  *random_req)
{
  GET_INSTANCE_DATA;
  switch (GET_STATE (STATE_RA))
  {
    case RA_NULL:
      if (random_req->send_mode.no NEQ 0)
      {
        rach_send_mphc_ra_req (random_req);
      }
      break;

    case RA_ACTIVE:
      if (random_req->send_mode.no EQ 0)
      {

        ALR_EM_STOP_CONNECTION_ESTABLISHMENT;

        SET_STATE (STATE_RA, RA_LISTEN_TO_CCCH);
        ma_rach_stop_ra_req ();
      }
      else
      {
        /*stop to be safe */
        ma_rach_stop_ra_req ();
        {
          rach_send_mphc_ra_req (random_req);
        } /*PALLOC*/
      } /* if */
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : rach_ra_cnf                |
+--------------------------------------------------------------------+

  PURPOSE : Process signal rach_ra_cnf from SDL process
            Main_Control.

*/

GLOBAL void rach_ra_cnf (T_MPHC_RA_CON  *ra_cnf)
{
  GET_INSTANCE_DATA;
  UBYTE c_burst = alr_data->rach_data.c_burst-1; /* confirmed index */

  switch (GET_STATE (STATE_RA))
  {
    case RA_ACTIVE:
    {
      PALLOC (random_cnf, MPH_RANDOM_ACCESS_CNF);

      ra_cnf->fn %= 42432L;
#ifdef GPRS
      alr_data->rach_data.t1[c_burst] =
        random_cnf->frame_no.t1=
        (UBYTE)((ra_cnf->fn / 1326) % 32);
      alr_data->rach_data.t2[c_burst] =
        random_cnf->frame_no.t2=
        (UBYTE)(ra_cnf->fn % 26);
      alr_data->rach_data.t3[c_burst] =
        random_cnf->frame_no.t3=
        (UBYTE)(ra_cnf->fn % 51);
#else
      random_cnf->frame_no.t1= (UBYTE)((ra_cnf->fn / 1326) % 32);
      random_cnf->frame_no.t2= (UBYTE)(ra_cnf->fn % 26);
      random_cnf->frame_no.t3= (UBYTE)(ra_cnf->fn % 51);
#endif
      ma_rach_random_cnf (random_cnf);

      c_burst++; /* next index */
      if (c_burst >= alr_data->rach_data.max_bursts)
      {/* last index ? */
        SET_STATE (STATE_RA, RA_LISTEN_TO_CCCH);
        ma_rach_stop_ra_req ();
 #if !defined(TNNN_WITH_TIMER)
        ma_fake_rr_report();
 #endif /* !TNNN_WITH_TIMER */
      }
      else
      {
        PALLOC (ra_req, MPHC_RA_REQ);

        /*
         * power limitation
         */
        ra_req->txpwr          = cut_power_value (alr_data->rach_data.max_tx_pwr_ccch, NULL);
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
        ra_req->powerclass_gsm = alr_data->rach_data.ms_class;
        ra_req->powerclass_dcs = alr_data->rach_data.dcs_class;
#endif

        /*
         * Modification: Add 1 to the time of RR !!!!
         */
        ra_req->rand = (UBYTE)(alr_data->rach_data.delta[c_burst]+1);
        ra_req->channel_request = alr_data->rach_data.channel_request[c_burst];
        ma_rach_ra_req (ra_req);
        alr_data->rach_data.c_burst++; /* the next index */
      }
      break;
    }
    default:
      break;
  }
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_RACH                   |
| STATE   : code                ROUTINE : cut_power_value            |
+--------------------------------------------------------------------+

  PURPOSE : Cut the nominal maximum output power to the
            associated power class.
            (Reference for the tables with the associated
             power classes, control levels and nominal output power
             is 3GPP TS 05.05, 4.1)

*/

GLOBAL UBYTE  cut_power_value (UBYTE power, T_MPH_DEDICATED_REQ *mph_dedicated_req)
{
  GET_INSTANCE_DATA;
  UBYTE new_power = power;
  UBYTE sc_band;

  if(mph_dedicated_req == NULL)
  {
    sc_band = alr_data->sc_band;
  }
  else
  {
    if(mph_dedicated_req->ch_type.h)
      sc_band = get_band(mph_dedicated_req->ch_type.ma[0]);
    else
      sc_band = get_band(mph_dedicated_req->ch_type.arfcn);
  }

  switch (sc_band)
  {
    case BAND_GSM_900:
    case BAND_E_GSM:
    case BAND_GSM_850:
      switch (alr_data->rach_data.ms_class)
      {
        case POW_CLASS_1:    /* doesn't exist */
        case POW_CLASS_2:    /* power class 2, 8W, 39 dBm, level 0..2 */
          /* highest level allowed => no cut necessary */
          break;
        case POW_CLASS_3:    /* power class 3, 5W, 37 dBm, level 3 */
          if (new_power < 3)
            new_power = 3;
          break;
        case POW_CLASS_4:    /* power class 4, 2W, 33 dBm, level 5 */
          if (new_power < 5)
            new_power = 5;
         break;
        case POW_CLASS_5:    /* power class 5, 0.8W, 29 dBm, level 7 */
          if (new_power < 7)
            new_power = 7;
          break;
      }
      break;
    case BAND_DCS_1800:
      switch (alr_data->rach_data.dcs_class)
      {
        case POW_CLASS_1:    /* power class 1, 1W, 30 dBm, level 0 */
        if (new_power > 28)
          new_power = 0;
          break;
        case POW_CLASS_2:    /* power class 2, 0.25W, 24 dBm, level 3 */
          if (!INRANGE (3, new_power, 28))
            new_power = 3;
          break;
        case POW_CLASS_3:    /* power class 3, 4W, 36 dBm , level 29 */
          /* highest level allowed => no cut necessary */
          break;
      }
      break;
    case BAND_PCS_1900:
      switch (alr_data->rach_data.dcs_class)
      {
        case POW_CLASS_1:    /* power class 1, 1W, 30 dBm, level 0 */
          if (!INRANGE (0, new_power, 15))
            new_power = 0;
          break;
        case POW_CLASS_2:    /* power class 2, 0.25W, 24 dBm, level 3 */
          if (!INRANGE (3, new_power, 15))
            new_power = 3;
          break;
        default:
        case POW_CLASS_3:    /* power class 3, 2W, 33 dBm , level 30 */
          /* highest level allowed => no cut necessary */
          break;
      }
      break;
  }
  return new_power;
}

/*
+--------------------------------------------------------------------+
| PROJECT :                          MODULE  : ALR_RACH              |
| STATE   : code                     ROUTINE : rach_send_mphc_ra_req |
+--------------------------------------------------------------------+

PURPOSE : Function to send the MPHC_RA_REQ primitive.
          Called from function 'rach_random_req' 
          This function handles:
          1) Starts Random Burst Sending.
          2) Power limitation.
          3) Adds 1 to the time of RR.
*/
LOCAL void rach_send_mphc_ra_req (T_MPH_RANDOM_ACCESS_REQ  *random_req)
{
  GET_INSTANCE_DATA;
  PALLOC (ra_req, MPHC_RA_REQ);
  ALR_TRACE_RACH("rach_send_mphc_ra_req");
  ALR_EM_START_CONNECTION_ESTABLISHMENT;

  /*
   * Start Random Burst Sending
   */
  memcpy (alr_data->rach_data.delta, random_req->send_mode.delta,
                                     sizeof (alr_data->rach_data.delta));
  memcpy (alr_data->rach_data.channel_request, random_req->send_mode.rach,
                                     sizeof (alr_data->rach_data.channel_request));
  alr_data->rach_data.max_bursts = random_req->send_mode.no;
  alr_data->rach_data.c_burst = 0;
  /*
   * power limitation
   */
  ra_req->txpwr          = cut_power_value (alr_data->rach_data.max_tx_pwr_ccch, NULL);
#ifndef TI_PS_FF_QUAD_BAND_SUPPORT
  ra_req->powerclass_gsm = alr_data->rach_data.ms_class;
  ra_req->powerclass_dcs = alr_data->rach_data.dcs_class;
#endif

  /*
   * Modification: Add 1 to the time of RR !!!!
   */
  ra_req->rand = (UBYTE)(alr_data->rach_data.delta[alr_data->rach_data.c_burst]+1);
  ra_req->channel_request = alr_data->rach_data.channel_request[alr_data->rach_data.c_burst++];
  SET_STATE (STATE_RA, RA_ACTIVE);
  ma_rach_ra_req (ra_req);
}

#endif

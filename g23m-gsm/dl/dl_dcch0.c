/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_DCCH0
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
|  Purpose :  This Modul defines the functions for the stand alone
|             dedicated control channel (SDCCH) and the fast dedicated
|             control channel (FACCH) each with SAPI 0 of the component DL
+-----------------------------------------------------------------------------
*/

#ifndef DL_DCCH0_C
#define DL_DCCH0_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include "string.h"
#include "vsi.h"
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"
#include "dl_trc.h"

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
LOCAL void dcch0_resume_reconnect_req (T_PRIM                 * prim,
                                       U8                       ch_type);
/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_init_dl_data        |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of DCCH data (SAPI 0) for an instance.

*/

GLOBAL void dcch0_init_dl_data (void)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[C_DCCH0];

  TRACE_FUNCTION ("dcch0_init_dl_data()");

  memset (pcch, 0, sizeof (T_CCH));

  dcch3_init_dl_data (); /*Disabling SAPI 3 connections */

  com_clear_queue (PS_SAPI_0);
  dl_data->rr_short_pd_buffer.l_buf = 0;

  if (dl_data->dcch0_in_msg)
  {
      COM_FREE_POINTER (dl_data->dcch0_in_msg);
      dl_data->dcch0_in_msg = NULL;
  }

  dl_data->RR_dedicated = FALSE;
  dl_data->dcch0_ch_type = 0;
  dl_data->dcch0_disc_request  = FALSE;

#if defined(DELAYED_SABM)
  dl_data->dcch0_sabm_delay = 0;
  dl_data->dcch0_sabm_flag = NOT_PRESENT_8BIT;
#endif  /* DELAYED_SABM */
#if defined(DELAYED_RELEASE_IND)
  dl_data->release_ind_delay = 0;
  dl_data->release_ind_ch_type = NOT_PRESENT_8BIT;
#endif  /* DELAYED_RELEASE_IND */
  set_channel_state(C_DCCH0, STATE_IDLE_DL);

  sacch0_init_dl_data ();
}

#if defined(DELAYED_SABM)
GLOBAL void dcch0_delay_sabm (T_CCH *pcch)
{
  GET_INSTANCE_DATA;
  BOOL send_sabm = FALSE;
  
  if (dl_data->dcch0_sabm_flag EQ NOT_PRESENT_8BIT) 
  { 
    if ((pcch->ch_type EQ L2_CHANNEL_FACCH_F) OR (pcch->ch_type EQ L2_CHANNEL_FACCH_H))
    { /* start delay */
      pcch->vtx = EMPTY_CMD; /* send dummy only */
      dl_data->dcch0_sabm_flag  = SABM_CMD;
      dl_data->dcch0_sabm_delay = DL_FACCH_SABM_DELAY_VALUE;
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, pcch->ch_type, "delay SABM on FACCH");
    }
    else
    { /* but for FACCH only */
      send_sabm = TRUE;
    }
  }
  else /* this part is called also called during uplink opportunity */
  { /* pending delay */
    if (dl_data->dcch0_sabm_delay > 0)
    {
      dl_data->dcch0_sabm_delay--;
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, pcch->ch_type, "pending delayed SABM");
    }
    else
    {/* finishing delay, send SABM with the current (next) uplink opportunity */
      #if !defined(DL_TRACE_ENABLED)
        TRACE_EVENT ("delayed SABM");
      #endif  /* ! DL_TRACE_ENABLED*/
      dl_data->dcch0_sabm_flag = NOT_PRESENT_8BIT;
      send_sabm = TRUE;
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, pcch->ch_type, "send delayed SABM");
    }
  }

  if (send_sabm)
  {
    pcch->vtx = SABM_CMD;
    pcch->time_flag  = TRUE;
    pcch->rc         = 0;
    set_channel_state (C_DCCH0, STATE_CONTENTION_RESOLUTION);
  }
}


#endif  /* DELAYED_SABM */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_establish_req       |
+--------------------------------------------------------------------+

  PURPOSE : Establish Request for SDCCH SAPI 0.

*/

GLOBAL void dcch0_establish_req (T_DL_ESTABLISH_REQ * est_req)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[C_DCCH0];

  TRACE_FUNCTION ("sdcch0_establish_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, est_req->ch_type, "est req");

  dl_data->dcch0_ch_type = pcch->ch_type = est_req->ch_type;
  TRACE_EVENT_WIN_P1 ("set dcch0_ch_type=%s", CH_TYPE_NAME[dl_data->dcch0_ch_type]);

  dl_data->dcch0_unserved = com_queue_awaiting_transmission (PS_SAPI_0) ?
                            DL_UNSERVED : DL_ALL_DONE;
  switch (dl_data->state[C_DCCH0])
  {
    case STATE_IDLE_DL:
      /*
       * initialize sacch buffer
       */
      dl_data->sacch_act_buffer = meas_report_no_nc;
      dl_data->sacch_last_buffer= meas_report_no_nc;

      com_clear_queue (PS_SAPI_0);
      pcch->contention_resolution = TRUE;
      dl_data->dcch0_queue.switch_buffer = (T_DL_DATA_REQ*)est_req;
      dl_data->dcch0_disc_request  = FALSE;
#if defined(DELAYED_SABM)
      dcch0_delay_sabm(pcch);
#else  /* DELAYED_SABM */
      pcch->vtx        = SABM_CMD;
      pcch->time_flag  = TRUE;
      pcch->rc         = 0;
      set_channel_state (C_DCCH0, STATE_CONTENTION_RESOLUTION);
#endif  /* DELAYED_SABM */
      TRACE_EVENT_WIN_P5 ("EST_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                          est_req->sapi, VTX_NAME[pcch->vtx],
                                          __FILE10__, __LINE__);
#if defined(DELAYED_SABM)
      dcch0_delay_sabm(pcch);
#endif  /* DELAYED_SABM */
      break;

    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
      if (est_req->sdu.l_buf EQ 0)
      {
        dcch3_init_dl_data (); /*Disabling SAPI 3 connections */
        com_clear_queue (PS_SAPI_0);
        pcch->T200_counter           = 0;
        pcch->contention_resolution  = FALSE;
        pcch->vtx                    = SABM_CMD;
        pcch->time_flag              = TRUE;
        pcch->rc                     = 0;
        set_channel_state (C_DCCH0, STATE_CONTENTION_RESOLUTION);
        TRACE_EVENT_WIN_P5 ("EST_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                            est_req->sapi, VTX_NAME[pcch->vtx],
                                            __FILE10__, __LINE__);
      }
      else
      {
        dcch0_mdl_release_req ();
      }
      COM_FREE_POINTER (est_req);
      break;

    default:
      COM_FREE_POINTER (est_req);
      dcch0_mdl_release_req ();
     break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_resume_req          |
+--------------------------------------------------------------------+

  PURPOSE : Resume Request for SDCCH (SAPI 0).

*/

GLOBAL void dcch0_resume_req (T_DL_RESUME_REQ * resume_req)
{
  
  TRACE_FUNCTION ("dcch0_resume_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, resume_req->ch_type, "resume req");

  dcch0_resume_reconnect_req( (T_PRIM *)D2P(resume_req), resume_req->ch_type);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_reconnect_req       |
+--------------------------------------------------------------------+

  PURPOSE : Reconnect Request for SDCCH (SAPI 0).

*/

GLOBAL void dcch0_reconnect_req (T_DL_RECONNECT_REQ * reconnect_req)
{
  
  TRACE_FUNCTION ("dcch0_reconnect_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, reconnect_req->ch_type, "reconn req");

  dcch0_resume_reconnect_req( (T_PRIM *)D2P (reconnect_req),reconnect_req->ch_type);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : dcch0_res_recon_req        |
+--------------------------------------------------------------------+

  PURPOSE : This function processes Resume Request and Reconnect Request 
            for SDCCH (SAPI 0).

*/
LOCAL void dcch0_resume_reconnect_req (T_PRIM                 * prim,
                                        U8                       ch_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch0_resume_reconnect_req()");

  switch (dl_data->state[C_DCCH0])
  {
    case STATE_SUSPENDED:
    case STATE_IDLE_DL:
    {
      T_CCH *pcch = &dl_data->cch[C_DCCH0];

      dl_data->dcch0_ch_type = pcch->ch_type = ch_type;

      TRACE_EVENT_WIN_P1 ("set dcch0_ch_type=%s", CH_TYPE_NAME[dl_data->dcch0_ch_type]);

      com_restore_queue(PS_SAPI_0, (T_DL_DATA_REQ*)P2D(prim));

      pcch->contention_resolution = FALSE;

#if defined(DELAYED_SABM)
      dcch0_delay_sabm(dl_data, pcch);
#else  /* DELAYED_SABM */
      pcch->vtx        = SABM_CMD;
      pcch->time_flag  = TRUE;
      pcch->rc         = 0;
      set_channel_state (C_DCCH0, STATE_CONTENTION_RESOLUTION);
#endif  /* DELAYED_SABM */

      if(prim->custom.opc EQ DL_RECONNECT_REQ)
      {
        TRACE_EVENT_WIN_P5 ("RECONN_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                            PS_SAPI_0, VTX_NAME[pcch->vtx],
                                            __FILE10__, __LINE__);
      }
      else
      {
        TRACE_EVENT_WIN_P5 ("RESUME_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                            PS_SAPI_0, VTX_NAME[pcch->vtx],
                                            __FILE10__, __LINE__);

        /* invalid measurement report first */
        sacch0_reset_meas ();
      }
      break;
    }
    default:
      {
        COM_FREE_POINTER (P2D(prim));
      }
      break;
  }/* endswitch */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_release_req         |
+--------------------------------------------------------------------+

  PURPOSE : Release Request for SDCCH (SAPI 0).

*/

GLOBAL BOOL dcch0_release_req (T_DL_RELEASE_REQ * release_req)
{
  GET_INSTANCE_DATA;
  BOOL bFree = TRUE;

  TRACE_FUNCTION ("dcch0_release_req()");

  if (release_req->mode EQ DL_LOCAL_END_RELEASE)
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, release_req->ch_type, "local end release req");
    dcch0_init_dl_data ();
    bFree = FALSE;
  }
  else
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, release_req->ch_type, "normal release req");
    dl_data->cch[C_DCCH0].ch_type = release_req->ch_type;
    switch (dl_data->state[C_DCCH0])
    {
      default:
        bFree = FALSE;
        break;

      case STATE_CONTENTION_RESOLUTION:
      case STATE_MULTIPLE_FRAME_ESTABLISHED:
      case STATE_TIMER_RECOVERY:
        if (!dl_data->dcch0_disc_request AND (dl_data->cch[C_DCCH0].vtx EQ RR_CMD))
        {
          dl_data->dcch0_disc_request = TRUE;
          dcch3_init_dl_data (); /*Disabling SAPI 3 connections */
        }
        else
        {
          dcch3_init_dl_data (); /*Disabling SAPI 3 connections */
          com_prepare_DISC (C_DCCH0, release_req->sapi);
        }
        break;

      case STATE_AWAITING_RELEASE:
        dl_data->cch[C_DCCH0].T200_counter = 0;
        break;
    }
  }

  if (bFree EQ FALSE)
  {
    PREUSE (release_req, release_cnf, DL_RELEASE_CNF);
    release_cnf->indication =
      com_queue_awaiting_transmission (PS_SAPI_0) ? DL_UNSERVED : DL_ALL_DONE;
    PSENDX (RR, release_cnf);
    dcch0_init_dl_data ();
  }

  return bFree;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_mdl_release_req     |
+--------------------------------------------------------------------+

  PURPOSE : Release Request from Mobility Management for SDCCH (SAPI 0).

*/

GLOBAL void dcch0_mdl_release_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch0_mdl_release_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, dl_data->cch[C_DCCH0].ch_type, "mdl rel req");

  switch (dl_data->state[C_DCCH0])
  {
    case STATE_CONTENTION_RESOLUTION:
    case STATE_AWAITING_RELEASE:
      drr_dl_release_cnf (dl_data->cch[C_DCCH0].ch_type, PS_SAPI_0, TRUE);
      break;
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
      drr_dl_release_ind (dl_data->cch[C_DCCH0].ch_type, PS_SAPI_0, NOT_PRESENT_8BIT, TRUE);
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch0_suspend_req         |
+--------------------------------------------------------------------+

  PURPOSE : Suspend Request for SDCCH (SAPI 0).

*/

GLOBAL void dcch0_suspend_req (T_DL_SUSPEND_REQ * suspend_req)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[C_DCCH0];

  TRACE_FUNCTION ("dcch0_suspend_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, suspend_req->ch_type, "suspend req");

  pcch->ch_type = suspend_req->ch_type;

  TRACE_EVENT_WIN_P1 ("reset dcch0_ch_type=%s ->0", CH_TYPE_NAME[dl_data->dcch0_ch_type]);
  dl_data->dcch0_ch_type = 0;

  switch (dl_data->state[C_DCCH0])
  {
    case STATE_CONTENTION_RESOLUTION:
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
    case STATE_AWAITING_RELEASE:
      pcch->T200_counter = 0;
      pcch->time_flag             = FALSE;
      pcch->contention_resolution = FALSE;
      set_channel_state (C_DCCH0, STATE_SUSPENDED);
      dcch3_init_dl_data (); /* Disabling SAPI 3 connections */

#if 0 /* yes, from now DL sends measurement reports also in suspended state */
      /*
       * no further measurement reports
       */
      sacch0_reset_meas (dl_data);
#endif  /* 0|1 */
      break;

    default:
      TRACE_EVENT ("INVALID SUSPEND STATE");
      break;
  }
  COM_FREE_POINTER (suspend_req);

  TRACE_EVENT_WIN_P5 ("SUSPEND_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                      PS_SAPI_0, VTX_NAME[pcch->vtx],
                                      __FILE10__, __LINE__);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : dcch0_check_disc           |
+--------------------------------------------------------------------+

  PURPOSE : DISC command frame on DCCH0.

*/

GLOBAL int dcch0_check_disc (int send)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch0_check_disc()");

  if (dl_data->dcch0_disc_request AND !(dl_data->cch[C_DCCH0].vtx EQ RR_CMD))
  {
    dcch3_init_dl_data (); /*Disabling SAPI 3 connections */
    com_prepare_DISC (C_DCCH0, PS_SAPI_0);
    dl_data->dcch0_disc_request = FALSE;
#if defined(DELAYED_SABM)
    dl_data->dcch0_sabm_flag = NOT_PRESENT_8BIT; /* delete pending delayed SABM */
#endif /* DELAYED_SABM */
    return uplink_awaiting_release  (C_DCCH0, PS_SAPI_0);
  }
  else
    return send;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : dcch0_data_req             |
+--------------------------------------------------------------------+

  PURPOSE : Data Request on SDCCH (SAPI 0).

*/

GLOBAL void dcch0_data_req (T_DL_DATA_REQ * data_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("sdcch0_data_req()");

  dl_data->cch[C_DCCH0].ch_type = data_req->ch_type;

  switch (dl_data->state[C_DCCH0])
  {
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
    case STATE_SUSPENDED:
    case STATE_CONTENTION_RESOLUTION:
      com_store_queue (PS_SAPI_0, data_req);
      break;

    default:
      COM_FREE_POINTER (data_req);
      break;
  }
}

#endif /* !DL_DCCH0_C */

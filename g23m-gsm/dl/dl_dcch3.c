/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_DCCH3
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
|  Purpose :  This Modul defines the functions for the slow
|             dedicated control channel (SAPI 3) of the component DL
+-----------------------------------------------------------------------------
*/

#ifndef DL_DCCH3_C
#define DL_DCCH3_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include <string.h>
#include "pconst.cdg"
#include "vsi.h"
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

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DCCH3                   |
| STATE   : code                ROUTINE : dcch3_init_dl_data         |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of SDCCH data (SAPI 3) for an instance.

*/

GLOBAL void dcch3_init_dl_data (void)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[C_DCCH3];

  TRACE_FUNCTION ("dcch3_init_dl_data()");

  memset (pcch, 0, sizeof (T_CCH));
  com_clear_queue (PS_SAPI_3);

  if (dl_data->dcch3_in_msg)
  {
    COM_FREE_POINTER (dl_data->dcch3_in_msg);
    dl_data->dcch3_in_msg = NULL;
  }

  dl_data->dcch3_disc_request  = FALSE;
  set_channel_state(C_DCCH3, STATE_DISABLED);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DCCH3                   |
| STATE   : code                ROUTINE : dcch3_enable               |
+--------------------------------------------------------------------+

  PURPOSE : Enabling SAPI 3 connections.

*/

GLOBAL void dcch3_enable (UBYTE ch_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch3_enable()");

  switch (dl_data->state[C_DCCH3])
  {
    case STATE_DISABLED:
      set_channel_state (C_DCCH3, STATE_IDLE_DL);
      dl_data->cch[C_DCCH3].ch_type = ch_type;
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DCCH3                   |
| STATE   : code                ROUTINE : dcch3_establish_req       |
+--------------------------------------------------------------------+

  PURPOSE : Establish Request for SDCCH (SAPI 3).

*/

GLOBAL void dcch3_establish_req (T_DL_ESTABLISH_REQ * est_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch3_establish_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH3, est_req->ch_type, "est req");


  switch (dl_data->state[C_DCCH3])
  {
    case STATE_IDLE_DL:
    case STATE_DISABLED:
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
      {
        T_CCH *pcch = &dl_data->cch[C_DCCH3];

        pcch->ch_type = est_req->ch_type;

        dl_data->dcch3_disc_request  = FALSE;
        dl_data->dcch3_unserved = com_queue_awaiting_transmission (PS_SAPI_3) ?
                                  DL_UNSERVED : DL_ALL_DONE;

        com_clear_queue (PS_SAPI_3);
        pcch->vtx       = SABM_CMD;
        pcch->time_flag = TRUE;
        pcch->rc        = 0;
        set_channel_state (C_DCCH3, STATE_AWAITING_ESTABLISHMENT);
        TRACE_EVENT_WIN_P5 ("EST_REQ: %s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[pcch->ch_type],
                                            est_req->sapi, VTX_NAME[pcch->vtx],
                                            __FILE10__, __LINE__);
      }

      break;
    default:
      break;
  }
  COM_FREE_POINTER (est_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : sdcch3_release_req         |
+--------------------------------------------------------------------+

  PURPOSE : Release Request for SDCCH (SAPI 3).

*/

GLOBAL BOOL dcch3_release_req (T_DL_RELEASE_REQ * release_req)
{
  GET_INSTANCE_DATA;
  BOOL bFree = TRUE;

  TRACE_FUNCTION ("dcch3_release_req()");

  if (release_req->mode EQ DL_LOCAL_END_RELEASE)
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH3, release_req->ch_type, "local end release req");
    bFree = FALSE;
  }
  else
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH3, release_req->ch_type, "normal release req");
    dl_data->cch[C_DCCH3].ch_type = release_req->ch_type;
    switch (dl_data->state[C_DCCH3])
    {
      default:
        bFree = FALSE;
        break;

      case STATE_AWAITING_ESTABLISHMENT:
      case STATE_MULTIPLE_FRAME_ESTABLISHED:
      case STATE_TIMER_RECOVERY:
        if (!dl_data->dcch3_disc_request AND (dl_data->cch[C_DCCH3].vtx EQ RR_CMD))
        {
          dl_data->dcch3_disc_request = TRUE;
        }
        else
        {
          com_prepare_DISC (C_DCCH3, release_req->sapi);
        }
        break;

      case STATE_AWAITING_RELEASE:
        dl_data->cch[C_DCCH3].T200_counter = 0;
        break;
    }
  }

  if (bFree EQ FALSE)
  {
    PREUSE (release_req, release_cnf, DL_RELEASE_CNF);
    release_cnf->indication =
      com_queue_awaiting_transmission (PS_SAPI_3) ? DL_UNSERVED : DL_ALL_DONE;
    PSENDX (RR, release_cnf);
    dcch3_init_dl_data ();
  }

  return bFree;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DCCH3                   |
| STATE   : code                ROUTINE : dcch3_mdl_release_req     |
+--------------------------------------------------------------------+

  PURPOSE : Release Request from Mobility Management for SDCCH (SAPI 3).

*/

GLOBAL void dcch3_mdl_release_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch3_mdl_release_req()");

  DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH3, dl_data->cch[C_DCCH3].ch_type, "mdl rel req");

  switch (dl_data->state[C_DCCH3])
  {
    case STATE_AWAITING_ESTABLISHMENT:
    case STATE_TIMER_RECOVERY:
    case STATE_AWAITING_RELEASE:
      drr_dl_release_cnf (dl_data->cch[C_DCCH3].ch_type, PS_SAPI_3, TRUE);
      break;

    case STATE_MULTIPLE_FRAME_ESTABLISHED:
      drr_dl_release_ind (dl_data->cch[C_DCCH3].ch_type, PS_SAPI_3, NOT_PRESENT_8BIT, TRUE);
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DCCH3                   |
| STATE   : code                ROUTINE : dcch3_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : Data Request on SDCCH (SAPI 3).

*/

GLOBAL void dcch3_data_req (T_DL_DATA_REQ * data_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch3_data_req()");

  switch (dl_data->state[C_DCCH3])
  {
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
    case STATE_TIMER_RECOVERY:
      com_store_queue (PS_SAPI_3, data_req);
      break;

    default:
      COM_FREE_POINTER (data_req);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SDCCH                   |
| STATE   : code                ROUTINE : dcch3_check_disc           |
+--------------------------------------------------------------------+

  PURPOSE : DISC command frame on DCCH3.

*/

GLOBAL int dcch3_check_disc (int send)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dcch3_check_disc()");

  if (dl_data->dcch3_disc_request AND !(dl_data->cch[C_DCCH3].vtx EQ RR_CMD))
  {
    com_prepare_DISC (C_DCCH3, PS_SAPI_3);
    return uplink_awaiting_release (C_DCCH3, PS_SAPI_3);
  }
  else
    return send;
}
#endif /* DL_DCCH3_C */

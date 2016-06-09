/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_SACCH0
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
|  Purpose :  This Modul defines the functions for the slow associated
|             control channel SACCH (only SAPI=0) of the component DL
+-----------------------------------------------------------------------------
*/

#ifndef DL_SACCH0_C
#define DL_SACCH0_C

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
GLOBAL const T_FRAME  meas_report_no_nc =
{
  N201_SACCH*8,
  24,
  {
    0x00, 0x00, 0x00,
    0x06, 0x15, 0x1E, 0x1E, 0x7E,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
  }
};
LOCAL const T_FRAME  meas_report_invalid =
{
  N201_SACCH*8,
  24,
  {
    0x00, 0x00, 0x00,
    0x06, 0x15, 0x00, 0x40, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
  }
};


/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SACCH0                  |
| STATE   : code                ROUTINE : sacch_init_dl_data         |
+--------------------------------------------------------------------+

  PURPOSE : Initialisation of SACCH data for an instance.

*/

GLOBAL void sacch0_init_dl_data (void)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[C_SACCH0];
  TRACE_FUNCTION ("sacch_init_dl_data()");

  memset (pcch, 0, sizeof (T_CCH));
  pcch->ch_type        = L2_CHANNEL_SACCH;

  if (dl_data->dcch3_in_msg)
  {
    COM_FREE_POINTER (dl_data->dcch3_in_msg);
    dl_data->dcch3_in_msg = NULL;
  }

  sacch0_reset_meas ();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SACCH0                  |
| STATE   : code                ROUTINE : sacch0_reset_meas            |
+--------------------------------------------------------------------+

  PURPOSE : Reset measurement reports and sacch_mode.

*/

GLOBAL void sacch0_reset_meas (void)
{
  GET_INSTANCE_DATA;
  dl_data->sacch_act_buffer = meas_report_no_nc;
  dl_data->sacch_last_buffer= meas_report_no_nc;
  dl_data->sacch_mode  = 2;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_SACCH0                  |
| STATE   : code                ROUTINE : sacch_send_data            |
+--------------------------------------------------------------------+

  PURPOSE : Send unacknowledged data on SACCH, SAPI=0.

*/

GLOBAL void sacch0_send_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("sacch_send_data()");

  TRACE_EVENT_WIN_P2 ("SACCH0 MODE = %d, %s", dl_data->sacch_mode,
    (dl_data->rr_short_pd_buffer.l_buf AND
      dl_data->rr_short_pd_ch_type EQ L2_CHANNEL_SACCH) ? "RR short PD" : "normal");

  if (dl_data->rr_short_pd_buffer.l_buf AND
      dl_data->rr_short_pd_ch_type EQ L2_CHANNEL_SACCH)
  { /* RR message with short PD, short L2 header type 1, format Bter */
    com_build_UI_Bter (L2_CHANNEL_SACCH);
  }
  else
  { /* format A or B */
    if (dl_data->sacch_mode EQ 2)
    {
      com_build_UI_command (L2_CHANNEL_SACCH, PS_SAPI_0, &meas_report_invalid);
    }
    else
    {
      if (dl_data->dl_active EQ FALSE)
        dl_data->sacch_last_buffer = dl_data->sacch_act_buffer;

      com_build_UI_command (L2_CHANNEL_SACCH, PS_SAPI_0, &dl_data->sacch_last_buffer);
    }
    if (dl_data->sacch_mode)
      dl_data->sacch_mode--;
  }
}

#endif

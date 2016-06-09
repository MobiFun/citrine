/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_DRR
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
|  Purpose :  This Modul defines the functions for distributing
|             all primitives of the upper layers.
+-----------------------------------------------------------------------------
*/

#ifndef DL_DRR_C
#define DL_DRR_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include <string.h>
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
#include "dl_em.h"
#include "dl_trc.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_establish_req       |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_ESTABLISH_REQ primitive.

*/

GLOBAL void drr_dl_establish_req (T_DL_ESTABLISH_REQ * est_req)
{
  BOOL fRelease;
  BOOL fInfoAvailable;
  GET_INSTANCE_DATA; 

  TRACE_FUNCTION ("drr_dl_establish_req()");

  if (est_req)
  {
    fRelease  = FALSE;
    fInfoAvailable  = (est_req->sdu.l_buf NEQ 0);

    TRACE_EVENT_WIN_P3 ("DL_ESTABLISH_REQ ch=%u SAPI=%u fInfoAvailable=%d",
      est_req->ch_type, est_req->sapi, fInfoAvailable);

    switch (est_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
      if ((est_req->sapi EQ 0)/* AND (fInfoAvailable)*/)
      {
        dcch0_establish_req (est_req);
      }
      else
        fRelease = TRUE;
      break;
    case L2_CHANNEL_SACCH:
      if ((dl_data->RR_dedicated) AND (est_req->sapi EQ 3) AND (!fInfoAvailable))
      {
        dcch3_establish_req (est_req);
      }
      else
        fRelease = TRUE;
      break;
    case L2_CHANNEL_SDCCH:
      if ((est_req->sapi EQ 0)/* AND (fInfoAvailable)*/)
      {
        dcch0_establish_req (est_req);
      }
      else if ((est_req->sapi EQ 3) AND (!fInfoAvailable))
      {
        dcch3_establish_req (est_req);
      }
      else
        fRelease = TRUE;
      break;
    default:
      fRelease = TRUE;
      break;
    }/* endswitch */

    if (fRelease)
    {
      PREUSE (est_req, rel_ind, DL_RELEASE_IND);
#ifndef DL_2TO1
      rel_ind->cs = NOT_PRESENT_8BIT;
#else
      rel_ind->ps_cause.ctrl_value = CAUSE_is_from_dl;
      rel_ind->ps_cause.value.dl_cause = NOT_PRESENT_8BIT;
#endif /*DL_2TO1*/
      PSENDX (RR, rel_ind);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_establish_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_ESTABLISH_IND primitive.

*/

GLOBAL void drr_dl_establish_ind (UBYTE ch_type,
                             UBYTE sapi, UBYTE indication)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_establish_ind (ch_type, sapi, indication);
    return;
  }
  else
#endif  /* INVOKE_SIGNAL */
  {
    PALLOC (est_ind, DL_ESTABLISH_IND);

    TRACE_FUNCTION ("drr_dl_establish_ind()");

    est_ind->ch_type   = ch_type;
    est_ind->sapi      = sapi;
    est_ind->indication = indication;
    PSENDX (RR, est_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_resume_req          |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_RESUME_REQ primitive.

*/

GLOBAL void drr_dl_resume_req (T_DL_RESUME_REQ * resume_req)
{
  TRACE_FUNCTION ("drr_dl_resume_req()");

  if (resume_req)
  {
    switch (resume_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
      dcch0_resume_req (resume_req);
      break;
    default:
      COM_FREE_POINTER (resume_req);
      break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_reconnect_req       |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_RECONNECT_REQ primitive.

*/

GLOBAL void drr_dl_reconnect_req (T_DL_RECONNECT_REQ * reconnect_req)
{
  TRACE_FUNCTION ("drr_dl_reconnect_req()");

  if (reconnect_req)
  {
    switch (reconnect_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
      dcch0_reconnect_req (reconnect_req);
      break;
    default:
      COM_FREE_POINTER (reconnect_req);
      break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_release_req         |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_RELEASE_REQ primitive.

*/

GLOBAL void drr_dl_release_req (T_DL_RELEASE_REQ * release_req)
{
  TRACE_FUNCTION ("drr_dl_release_req()");


  if (release_req)
  {
    BOOL  bFree = TRUE;

#if !defined(LATE_LEAVING_DEDICATED)
    com_leave_dedicated (release_req->ch_type);
#endif  /* LATE_LEAVING_DEDICATED */

    switch (release_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
    case L2_CHANNEL_SACCH:

       DL_OFFLINE_TRACE (TRACE_DL_EVENT,
         release_req->sapi EQ PS_SAPI_0 ? C_DCCH0 : C_DCCH3,
         release_req->ch_type, "Release Req");

      if (release_req->sapi EQ PS_SAPI_0)
      {
        bFree = dcch0_release_req (release_req);
      }
      else if (release_req->sapi EQ PS_SAPI_3)
      {
        bFree = dcch3_release_req (release_req);
      }
      break;
    default:
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, release_req->ch_type, "Release Req");
      break;
    }

    if (bFree)
    {
      COM_FREE_POINTER (release_req);
    }
  }
}
#ifndef DL_2TO1
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_mdl_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a MDL_RELEASE_REQ primitive.

*/

GLOBAL void drr_mdl_release_req (T_MDL_RELEASE_REQ * release_req)
{
  TRACE_FUNCTION ("drr_mdl_release_req()");

  if (release_req)
  {
    switch (release_req->sapi)
    {
    case DL_SAPI_0:
      dcch0_mdl_release_req ();
      break;
    case DL_SAPI_3:
      dcch3_mdl_release_req ();
      break;

    default:
      break;
    }
  }

  COM_FREE_POINTER (release_req);
}
#endif /* DL_2TO1*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_suspend_req         |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_SUSPEND_REQ primitive.

*/

GLOBAL void drr_dl_suspend_req (T_DL_SUSPEND_REQ * suspend_req)
{
  TRACE_FUNCTION ("drr_dl_suspend_req()");

  if (suspend_req)
  {

    switch (suspend_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
      dcch0_suspend_req (suspend_req);
      break;
    default:
      COM_FREE_POINTER (suspend_req);
      break;
    }


    DL_EM_LINK_SUSPENDED;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_DATA_REQ primitive.

*/

GLOBAL void drr_dl_data_req (T_DL_DATA_REQ * data_req)
{
  TRACE_FUNCTION ("drr_dl_data_req()");

  if (data_req)
  {
    switch (data_req->ch_type)
    {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
    case L2_CHANNEL_SACCH:
      if (data_req->sapi EQ 3)
      {
        dcch3_data_req (data_req);
      }
      else
      {
        dcch0_data_req (data_req);
      }
      break;

    default:
      COM_FREE_POINTER (data_req);
      break;
    }
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_DATA_IND primitive.

*/

GLOBAL void drr_dl_data_ind (UBYTE sapi, ULONG fn)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT_WIN_P2 ("drr_dl_data_ind(,SAPI=%u): in_msg=%08x",
    sapi, (sapi EQ PS_SAPI_0) ? dl_data->dcch0_in_msg : dl_data->dcch3_in_msg);

  com_l3trace (TRACE_DOWNLINK, sapi,
    (UBYTE *)((sapi EQ PS_SAPI_0) ? dl_data->dcch0_in_msg : dl_data->dcch3_in_msg));

  TRACE_FUNCTION ("drr_dl_data_ind()");

  if (fn EQ NOT_PRESENT_32BIT)
    fn = dl_data->fn;

  if (sapi EQ PS_SAPI_0)
  {
    if (dl_data->dcch0_in_msg)
    {
      dl_data->dcch0_in_msg->fn  = fn;
      PSENDX (RR, dl_data->dcch0_in_msg);
      dl_data->dcch0_in_msg = NULL;
    }
    else
    {
      TRACE_EVENT_WIN ("dcch0_in_msg = NULL");
    }
  }
  else
  {
    if (dl_data->dcch3_in_msg)
    {
      dl_data->dcch3_in_msg->fn  = fn;
      PSENDX (RR, dl_data->dcch3_in_msg);
      dl_data->dcch3_in_msg = NULL;
    }
    else
    {
      TRACE_EVENT_WIN ("dcch3_in_msg = NULL");
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Indicates the acknowledgement of an uplinked message by
            a DL_DATA_CNF primitive (only if it have been requested by L3).

*/

GLOBAL void drr_dl_data_cnf (UBYTE sapi)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_data_cnf (sapi);
    return;
  }
  else
#endif  /* INVOKE_SIGNAL */
  {
    PALLOC (dl_data_cnf, DL_DATA_CNF);
    T_QUEUE * queue;

    TRACE_FUNCTION ("drr_dl_data_cnf()");

    if (sapi EQ PS_SAPI_0)
    {
      queue = &dl_data->dcch0_queue;
    }
    else
    {
      queue = &dl_data->dcch3_queue;
    }

    dl_data_cnf->ch_type = queue->sending_buffer->ch_type;
    dl_data_cnf->sapi = queue->sending_buffer->sapi;
    dl_data_cnf->cnf = queue->sending_buffer->cnf;
    PSENDX (RR, dl_data_cnf);

    COM_FREE_QUEUE_BUFFER (queue, INDEX_SENDING_BUFFER);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_unitdata_req        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_UNITDATA_REQ primitive.

*/

GLOBAL void drr_dl_unitdata_req (T_DL_UNITDATA_REQ * unitdata_req)
{
  TRACE_FUNCTION ("drr_dl_unitdata_req()");

  if (unitdata_req)
  {
    GET_INSTANCE_DATA; 
    /*lint -e420 (Warning -- Apparent access beyond array) */
    memcpy (&dl_data->sacch_act_buffer, &unitdata_req->sdu, sizeof (T_FRAME));
    /*lint +e420 (Warning -- Apparent access beyond array) */
    dl_data->sacch_mode = 0;

    COM_FREE_POINTER (unitdata_req);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_unitdata_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_UNITDATA_IND primitive.

*/

GLOBAL void drr_dl_unitdata_ind (UBYTE error_flag, UBYTE * layer1head,
                                 UBYTE * layer3msg, UBYTE length, ULONG fn)
{
  GET_INSTANCE_DATA; 
#define UNITDATA_OFFSET 1
#if 0 /* happens in primitive context only */
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_unitdata_ind (dl_data, error_flag, layer1head, layer3msg, length, fn);
    return;
  }
#endif  /* INVOKE_SIGNAL */
#endif  /* 0 */

  TRACE_FUNCTION ("drr_dl_unitdata_ind()");

  /* limit length of UI frame, because they could be invalid */
  if (length > DL_N201_SACCH_A_B)
    length = DL_N201_SACCH_A_B;

  if ((layer3msg AND length) OR (error_flag NEQ VALID_BLOCK))
  {
    PALLOC_SDU (unitdata_ind, DL_UNITDATA_IND,
      (USHORT)(((USHORT)length + UNITDATA_OFFSET) * BITS_PER_BYTE));

    unitdata_ind->error_flag = error_flag;
    unitdata_ind->pwr_lev = layer1head[0];
    unitdata_ind->ta = layer1head[1];
    unitdata_ind->sdu.o_buf = UNITDATA_OFFSET * BITS_PER_BYTE;
    unitdata_ind->sdu.l_buf = length * BITS_PER_BYTE;
    memset (&unitdata_ind->sdu.buf[0], 0, UNITDATA_OFFSET);
#ifdef DL_2TO1
    /* build a pseudo length octet (for check in RR rr_msg_accept_sacch_normal) */
    unitdata_ind->sdu.buf[UNITDATA_OFFSET-1] = (length << 2) | 0x01;
#endif /* DL_2TO1 */
    if (error_flag EQ VALID_BLOCK)
      /*lint -e668 (Warning -- Possibly passing a null pointer) */
      /*lint -e669 (Warning -- Possible data overrun) */
      memcpy (&unitdata_ind->sdu.buf[UNITDATA_OFFSET], layer3msg, length);
      /*lint +e669 (Warning -- Possible data overrun) */
      /*lint +e668 (Warning -- Possibly passing a null pointer) */


    if (fn EQ NOT_PRESENT_32BIT)
      fn = dl_data->fn;
    unitdata_ind->fn  = fn;

    if (error_flag NEQ VALID_BLOCK)
    {
      DL_OFFLINE_TRACE(TRACE_DL_EVENT, C_DCCH0, L2_CHANNEL_SACCH, "inv UNITDATA_IND");
    }
    else
    {
      com_l3trace (TRACE_UACK_DN, L2_CHANNEL_SACCH, layer3msg);
    }

    PSENDX (RR, unitdata_ind);
  }

#undef UNITDATA_OFFSET
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_short_unitdata_req  |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a DL_SHORT_UNITDATA_REQ primitive.

*/

GLOBAL void drr_dl_short_unitdata_req (T_DL_SHORT_UNITDATA_REQ * short_unitdata_req)
{
  TRACE_FUNCTION ("drr_dl_short_unitdata_req()");

  if (short_unitdata_req)
  {
    GET_INSTANCE_DATA; 
    /*lint -e420 (Warning -- Apparent access beyond array) */
    /*lint -e420 (Warning -- Apparent access beyond array) */
    memcpy (&dl_data->rr_short_pd_buffer, &short_unitdata_req->sdu, sizeof (T_FRAME));
    /*lint +e420 (Warning -- Apparent access beyond array) */
#ifndef DL_2TO1
    dl_data->rr_short_pd_ch_type = short_unitdata_req->ch_type;
#else
    dl_data->rr_short_pd_ch_type = short_unitdata_req->l2_channel;
#endif

    COM_FREE_POINTER (short_unitdata_req);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_short_unitdata_ind  |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_SHORT_UNITDATA_IND primitive.

*/

GLOBAL void drr_dl_short_unitdata_ind (UBYTE ch_type,
                                UBYTE error_flag, UBYTE * layer1head,
                                UBYTE * layer3msg, UBYTE length, ULONG fn)
{
  GET_INSTANCE_DATA;
#define SHORT_UNITDATA_OFFSET 1
#if defined(INVOKE_SIGNAL)
    if (dl_data->interrupt_context)
    {
      sig_invoke_drr_dl_short_unitdata_ind (ch_type, error_flag,
        layer1head, layer3msg, length, fn);
      return;
    }
#endif  /* INVOKE_SIGNAL */

  TRACE_FUNCTION ("drr_dl_short_unitdata_ind()");

  if (layer3msg)
  {
    PALLOC_SDU (short_unitdata_ind, DL_SHORT_UNITDATA_IND,
      (USHORT)((length + SHORT_UNITDATA_OFFSET) * BITS_PER_BYTE));

    short_unitdata_ind->ch_type = ch_type;
    short_unitdata_ind->error_flag = error_flag;
    if (ch_type EQ L2_CHANNEL_SACCH)
    {
      short_unitdata_ind->pwr_lev = layer1head[0];
      short_unitdata_ind->ta = layer1head[1];
    }
    else
    {
      short_unitdata_ind->pwr_lev = 0;
      short_unitdata_ind->ta = 0;
    }
    short_unitdata_ind->sdu.o_buf = SHORT_UNITDATA_OFFSET * BITS_PER_BYTE;
    short_unitdata_ind->sdu.l_buf = length * BITS_PER_BYTE;
    memset (&short_unitdata_ind->sdu.buf[0], 0, SHORT_UNITDATA_OFFSET);
    memcpy (&short_unitdata_ind->sdu.buf[SHORT_UNITDATA_OFFSET], layer3msg, length);

    if (fn EQ NOT_PRESENT_32BIT)
      fn = dl_data->fn;
    short_unitdata_ind->fn  = fn;
    if (error_flag NEQ VALID_BLOCK)
    {
      DL_OFFLINE_TRACE(TRACE_DL_EVENT, C_DCCH0, ch_type, "inv SHORT_UNITDATA_IND");
    }
    else
    {
      com_l3trace (TRACE_UACK_DN, ch_type, layer3msg);
    }
    PSENDX (RR, short_unitdata_ind);
  }

#undef SHORT_UNITDATA_OFFSET
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_establish_cnf       |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_ESTABLISH_CNF primitive.

*/

GLOBAL void drr_dl_establish_cnf (UBYTE ch_type, UBYTE sapi)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_establish_cnf (ch_type, sapi);
    return;
  }
  else
#endif  /* INVOKE_SIGNAL */
  {
    PALLOC (dl_establish_cnf, DL_ESTABLISH_CNF);

    TRACE_FUNCTION ("drr_dl_establish_cnf()");
    dl_establish_cnf->ch_type = ch_type;
    dl_establish_cnf->sapi    = sapi;
    dl_establish_cnf->indication = sapi EQ PS_SAPI_0 ?
                                   dl_data->dcch0_unserved : dl_data->dcch3_unserved;
    PSENDX (RR, dl_establish_cnf);

    DL_EM_CHANNEL_ESTABLISHMENT_PASSED;

    if (!dl_data->RR_dedicated)
    {
      /* first uplink on SACCH after the first establishment is an empty frame */
      if (sapi EQ PS_SAPI_0)
      {
        /* the SAPI value of the next SACCH frame depends on the next awaiting frame */
        TRACE_EVENT_WIN_P1 ("sacch_last_uplink_sapi:=%u->3", dl_data->sacch_last_uplink_sapi);
        dl_data->sacch_last_uplink_sapi = NOT_PRESENT_8BIT;
      }

      /* RR is entering the dedicated mode */
      dl_data->RR_dedicated = TRUE;
      DL_OFFLINE_TRACE(TRACE_DL_EVENT, sapi EQ PS_SAPI_0 ? C_DCCH0 : C_DCCH3,
                       ch_type, "RR_dedicated:=TRUE");
    }
  }
}/* endfunc drr_dl_establish_cnf */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_release_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_RELEASE_IND primitive.

*/

GLOBAL void drr_dl_release_ind (UBYTE ch_type,
                            UBYTE sapi, UBYTE cs, BOOL init)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_release_ind (ch_type, sapi, cs, init);
    return;
  }
  else
#endif  /* INVOKE_SIGNAL */
  {
    PALLOC (dl_release_ind, DL_RELEASE_IND);/* T_DL_RELEASE_IND */

    TRACE_FUNCTION ("drr_dl_release_ind()");

    dl_release_ind->ch_type  = ch_type;
    dl_release_ind->sapi     = sapi;
#ifndef DL_2TO1
    dl_release_ind->cs = cs;
#else
    dl_release_ind->ps_cause.ctrl_value = CAUSE_is_from_dl;
    dl_release_ind->ps_cause.value.dl_cause = cs;
#endif /*DL_2TO1*/

#if defined (DL_TRACE_ENABLED)
    switch (ch_type)
    {
      case L2_CHANNEL_FACCH_F:
      case L2_CHANNEL_FACCH_H:
      case L2_CHANNEL_SDCCH:
        DL_OFFLINE_TRACE (TRACE_DL_EVENT,
          sapi EQ PS_SAPI_0 ? C_DCCH0 : C_DCCH3, ch_type, "Release Ind");
        break;
      case L2_CHANNEL_SACCH:
        DL_OFFLINE_TRACE (TRACE_DL_EVENT, sapi EQ PS_SAPI_0 ? C_DCCH0 : C_DCCH3, ch_type,
          sapi EQ PS_SAPI_0 ? "Release Ind" : "Release Ind SAPI=3");
        break;
    }
#endif /* DL_TRACE_ENABLED */

#if defined(DELAYED_RELEASE_IND)
    {
      GET_INSTANCE_DATA; 
      dl_data->release_ind_ch_type    = NOT_PRESENT_8BIT;
    }
#endif  /* DELAYED_RELEASE_IND */
    PSENDX (RR, dl_release_ind);

    if (init)
    {
      if (sapi EQ DL_SAPI_0)
      {
        dcch0_init_dl_data ();/* reset data link layer for SAPI=0 and 3 */
      }
      else if  (sapi EQ DL_SAPI_3)
      {
        dcch3_init_dl_data ();/* reset data link layer for SAPI=3 only */
      }
    }
  }

}/* endfunc drr_dl_release_ind */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
| STATE   : code                ROUTINE : drr_dl_release_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Send of a DL_RELEASE_CNF primitive.

*/

GLOBAL void drr_dl_release_cnf (UBYTE ch_type,
                            UBYTE sapi, BOOL init)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_dl_release_cnf (ch_type, sapi, init);
    return;
  }
  else
#endif  /* INVOKE_SIGNAL */
  {
    PALLOC (dl_release_cnf, DL_RELEASE_CNF);

    TRACE_FUNCTION ("drr_dl_release_cnf()");
    dl_release_cnf->ch_type = ch_type;
    dl_release_cnf->sapi    = sapi;

    DL_EM_LINK_RELEASE;

    PSENDX (RR, dl_release_cnf);

    if (init)
    {
      if (sapi EQ DL_SAPI_0)
      {
        dcch0_init_dl_data ();/* reset data link layer for SAPI=0 and 3 */
      }
      else if  (sapi EQ DL_SAPI_3)
      {
        dcch3_init_dl_data ();/* reset data link layer for SAPI=3 only */
      }
    }
  }
}/* endfunc drr_dl_release_cnf */

/*
 +--------------------------------------------------------------------+
 | PROJECT : GSM-PS (6147)       MODULE  : DL_DRR                     |
 | STATE   : code                ROUTINE : drr_error_ind              |
 +--------------------------------------------------------------------+

   PURPOSE : Release after error.

*/

GLOBAL void drr_error_ind (UBYTE ch_type, UBYTE sapi)
{
  GET_INSTANCE_DATA;
#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_drr_error_ind (ch_type, sapi);
    return;
  }
#endif  /* INVOKE_SIGNAL */

  TRACE_FUNCTION ("drr_error_ind()");
  DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, ch_type, "error_ind ");

  /*
   * not a real error indication, instead a release ind to RR and release req for myself
   */
  drr_dl_release_ind (ch_type, sapi, NOT_PRESENT_8BIT, FALSE);
  if (((sapi EQ DL_SAPI_0) AND (dl_data->state[C_DCCH0] > STATE_IDLE_DL))
      OR
      ((sapi EQ DL_SAPI_3) AND (dl_data->state[C_DCCH3] > STATE_IDLE_DL)))
  {
    PALLOC (release_req, DL_RELEASE_REQ);
    release_req->ch_type = ch_type;
    release_req->sapi = sapi;
    release_req->mode = DL_NORMAL_RELEASE;
    drr_dl_release_req (release_req);
  }
}

#endif /* DL_DRR_C */

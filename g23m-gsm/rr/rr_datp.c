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
|  Purpose :  This Modul defines the functions for the data transfer
|             capability of the module Radio Resource.
+-----------------------------------------------------------------------------
*/

#ifndef RR_DATP_C
#define RR_DATP_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include <stdio.h>      /* sprintf */
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
#include "rr_em.h"

/*==== EXPORT =====================================================*/
#if defined (REL99) && defined (TI_PS_FF_EMR)
#define MAX_MSTRUCT_CCD_RR MAXIMUM(MAX_MSTRUCT_LEN_RR_SHORT, MAX_MSTRUCT_LEN_RR)
GLOBAL UBYTE _decodedMsg [MAX_MSTRUCT_CCD_RR];
#else
GLOBAL UBYTE _decodedMsg [MAX_MSTRUCT_LEN_RR];
#endif


/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_init_rr_data           |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the RR data for the module data transfer.

*/

GLOBAL void dat_init_rr_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_init_rr_data()");

  SET_STATE (STATE_DAT, DAT_NULL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_dl_establish_cnf       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive DL_ESTABLISH_CNF received from DL.

*/
GLOBAL void dat_dl_establish_cnf (T_DL_ESTABLISH_CNF *dl_establish_cnf_orig)
{
  GET_INSTANCE_DATA;
  /*
   * sys infos during l2 establishment are
   * lost, inform layer 1 to send them again
   */
  PPASS(dl_establish_cnf_orig, dl_establish_cnf, DL_ESTABLISH_CNF);

  TRACE_FUNCTION ("dat_dl_establish_cnf()");
  rr_data->net_lost = FALSE;
  TRACE_EVENT_P1("rr_data->net_lost = %u", rr_data->net_lost);
  
  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_ASS_1:
    case DAT_CHAN_ASS_2:
    case DAT_IMM_ASS_1:
      break;

    default :
      if (dl_establish_cnf->sapi EQ SAPI_0)
      {
        PALLOC (mph_sync_req, MPH_SYNC_REQ);
        mph_sync_req->cs = CS_CLEAN_SYS_INFO;
        PSENDX (PL, mph_sync_req);
      }
      break;
  }

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_ASS_1:
      /*
       * successfull establishment of new layer 2 connection
       * after channel assignment
       */

      EM_DL_ESTABLISH_CNF;

      PFREE (dl_establish_cnf);

      if (rr_data->sc_data.ch_mode NEQ
          rr_data->cr_data.ch_mode)
      {
        /*
         * a changed channel mode has been detected.
         * update channel description and channel mode
         * for the serving cell and inform MM about the
         * new channel mode.
         */
        memcpy (&rr_data->sc_data.chan_desc,
                &rr_data->cr_data.chan_desc,
                sizeof (T_chan_desc));
        rr_data->sc_data.ch_mode = rr_data->cr_data.ch_mode;
        dat_code_channel_mode_to_mm ();
      }

      if (rr_data->cr_data.ciph_on NEQ
          rr_data->sc_data.ciph_on)
      {
        /*
         * the ciphering has changed. Inform MM about the new
         * ciphering.
         */
        dat_code_ciphering_to_mm (rr_data->cr_data.ciph_on);
      }

      /*
       * copy cr data to sc data and go back to normal dedicated mode.
       */
      rr_data->sc_data.ciph_on  = rr_data->cr_data.ciph_on;
      rr_data->sc_data.algo     = rr_data->cr_data.algo;
      rr_data->sc_data.ch_mode  = rr_data->cr_data.ch_mode;
      rr_data->sc_data.cd.v_cell_chan_desc =
               rr_data->cr_data.cd.v_cell_chan_desc;
      memcpy (&rr_data->sc_data.cd.cell_chan_desc,
              &rr_data->cr_data.cd.cell_chan_desc,
              sizeof (T_LIST));
      memcpy (&rr_data->sc_data.chan_desc,
              &rr_data->cr_data.chan_desc,
              sizeof (T_chan_desc));
      memcpy (&rr_data->sc_data.amr_conf,
              &rr_data->cr_data.amr_conf,
              sizeof (T_multirate_conf));

      rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
      att_remove_multiple_channels ();
      SET_STATE (STATE_SAPI_3, SMS_IDLE);
      srv_use_stored_prim ();
      dat_set_last_used_channel (&rr_data->sc_data.chan_desc);
      att_dat_dedicated ();
      dat_emo_stop ( TRUE );
      SET_STATE (STATE_DAT, DAT_DEDICATED);
      rr_data->mode_after_dedi = MODE_CELL_RESELECTION;
      break;

    case DAT_CHAN_ASS_2:
      /*
       * successfull reconnection on old channel after failed
       * channel assignment
       *
       * update some parameters to go back to normal dedicated mode.
       */
      PFREE (dl_establish_cnf);
      srv_use_stored_prim ();
      SET_STATE (STATE_SAPI_3, SMS_IDLE);
      att_dat_dedicated ();
      rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
      SET_STATE (STATE_DAT, DAT_DEDICATED);

      rr_data->mode_after_dedi = MODE_CELL_RESELECTION;

      EM_ASS_FAILURE_RECONNECT_SUCCESS;

      break;

    case DAT_DEDICATED:
      /*
       * An establish confirm in normal dedicated mode is only
       * expected for short messages (SAPI 3)
       */
      if (dl_establish_cnf->sapi EQ SAPI_3)
      {
        SET_STATE (STATE_SAPI_3, SMS_ESTABLISHED);
        srv_use_stored_prim ();
      }
      PFREE (dl_establish_cnf);
      break;

    case DAT_HANDOVER_4:
      TRACE_EVENT ("DL_ESTABLISH_CNF");

      EM_L2_CONNECTION_ESTABLISHED;

      PFREE (dl_establish_cnf);

      /*
       * successfull establishment of new layer 2 connection
       * after handover
       */
      if (rr_data->sc_data.ch_mode NEQ
          rr_data->cr_data.ch_mode)
      {
        /*
         * a changed channel mode has been detected.
         * update channel description and channel mode
         * for the serving cell and inform MM about the
         * new channel mode.
         */
        memcpy (&rr_data->sc_data.chan_desc,
                &rr_data->cr_data.chan_desc,
                sizeof (T_chan_desc));
        rr_data->sc_data.ch_mode = rr_data->cr_data.ch_mode;
        dat_code_channel_mode_to_mm ();
      }

      if (rr_data->cr_data.ciph_on NEQ rr_data->sc_data.ciph_on)
      {
        /*
         * the ciphering has changed. Inform MM about the new
         * ciphering.
         */
        dat_code_ciphering_to_mm (rr_data->cr_data.ciph_on);
      }

      /*
       * copy cr data to sc data and go back to normal dedicated mode.
       */
      rr_data->sc_data.ciph_on  = rr_data->cr_data.ciph_on;
      rr_data->sc_data.algo     = rr_data->cr_data.algo;
      rr_data->sc_data.ch_mode  = rr_data->cr_data.ch_mode;
      rr_data->sc_data.cd.v_cell_chan_desc =
               rr_data->cr_data.cd.v_cell_chan_desc;
      memcpy (&rr_data->sc_data.cd.cell_chan_desc,
              &rr_data->cr_data.cd.cell_chan_desc,
              sizeof (T_LIST));
      memcpy (&rr_data->sc_data.chan_desc,
              &rr_data->cr_data.chan_desc,
              sizeof (T_chan_desc));
      memcpy (&rr_data->sc_data.amr_conf,
              &rr_data->cr_data.amr_conf,
              sizeof (T_multirate_conf));

      rr_data->nc_data[SC_INDEX].bsic = rr_data->nc_data[CR_INDEX].bsic;
      rr_data->nc_data[SC_INDEX].arfcn = rr_data->nc_data[CR_INDEX].arfcn;
      rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
      /*
       * after a handover we cannot come back directly to the
       * cell instead we have to do a normal cell reselection
       */
      rr_data->mode_after_dedi = MODE_CELL_RESELECTION;
      att_remove_multiple_channels ();
      SET_STATE (STATE_SAPI_3, SMS_IDLE);
      srv_use_stored_prim ();
      dat_set_last_used_channel (&rr_data->sc_data.chan_desc);
      att_dat_dedicated ();
      dat_emo_stop ( TRUE );
      SET_STATE (STATE_DAT, DAT_DEDICATED);
      break;

    case DAT_HANDOVER_5:
      TRACE_EVENT ("DL_ESTABLISH_CNF");
      /*
       * successfull reconnection on old channel after failed
       * handover
       *
       * update some parameters to go back to normal dedicated mode.
       */
      PFREE (dl_establish_cnf);
      srv_use_stored_prim ();
      SET_STATE (STATE_SAPI_3, SMS_IDLE);
      att_dat_dedicated ();
      rr_data->nc_data[CR_INDEX].arfcn = NOT_PRESENT_16BIT;
      SET_STATE (STATE_DAT, DAT_DEDICATED);

      rr_data->mode_after_dedi = MODE_CELL_RESELECTION;

      EM_HO_FAILURE_RECONNECT_SUCCESS;
      break;

    case DAT_IMM_ASS_1:
      /*
       * successfull establishment of a dedicated connection.
       * Depending on the originator of the connection MM
       * is informed.
       */
      EM_L2_CONNECTION_ESTABLISHED;

#ifdef GPRS
      if(! dat_check_packet_access())
      {
#endif
        if (rr_data->ms_data.establish_cause EQ ESTCS_PAGING)
        {
          PREUSE (dl_establish_cnf, rr_establish_ind, RR_ESTABLISH_IND);
          PSENDX (MM, rr_establish_ind);
        }
        else
        {
          PREUSE (dl_establish_cnf, rr_establish_cnf, RR_ESTABLISH_CNF);
          PSENDX (MM, rr_establish_cnf);
        }
#ifdef GPRS
      }
      else
      { /* packet access */
        PFREE (dl_establish_cnf);
      }
#endif

      /*
       * initialize some variables for the dedicated mode.
       */
      rr_data->dyn_config.fho                       = 0;
      rr_data->dyn_config.fca                       = 0;
      rr_data->tch_loop_subch            = NOT_PRESENT_8BIT;
      rr_data->sc_data.ciph_received     = FALSE;
      rr_data->rel_cause                 = RRCS_INT_NOT_PRESENT;
      rr_data->sc_data.ciph_on           = CIPH_OFF;
      att_copy_old_lai_rac(SC_INDEX);
#if 0
      memcpy (&rr_data->old_lai,
              &rr_data->nc_data[SC_INDEX].lai,
              sizeof (rr_data->old_lai));
      rr_data->old_cell_id               = rr_data->nc_data[SC_INDEX].cell_id;
#endif
      SET_STATE (STATE_SAPI_3, SMS_IDLE);
      att_dat_dedicated ();
      SET_STATE (STATE_DAT, DAT_DEDICATED);

      /*
       * Early classmark Sending is performed if the mobile supports it
       * and the network requests it.
       */

      if (rr_data->ms_data.classmark2.es_ind AND
          rr_data->nc_data[SC_INDEX].c2_par.ecsc)
      {
        /*
         * building of the Early Classmark Sending message
         */
        /* Implements RR Clone findings #15 */
        dat_class_chng_data_req();
      }

#ifdef GPRS
      dat_gprs_suspend_req ();
#endif

      EM_EARLY_CLASSMARK_SENDING;
      break;

#ifdef GPRS
    case DAT_PDCH_ASS_4:
      dat_rrgrr_reconnect_dcch_cnf (RECONN_OK);
      SET_STATE (STATE_DAT, DAT_DEDICATED);
      PFREE (dl_establish_cnf);
      break;

    case DAT_CCO_4:
      if ( rr_data->gprs_data.cco_need_reconnect_cnf )
      {
        dat_rrgrr_reconnect_dcch_cnf (RECONN_OK);
      }
      else
      {
        /*
         * GRR has received d_change_order message,
         * but the BCCH reading in the new cell failed.
         * The connection is resumed on the old channel
         * in dedicated mode.
         */

        PALLOC (rrgrr_sync_ind, RRGRR_SYNC_IND);
        rrgrr_sync_ind->sync_res  = SYNC_FAILED;
        PSENDX (GRR, rrgrr_sync_ind);
      }
      SET_STATE (STATE_DAT, DAT_DEDICATED);
      PFREE (dl_establish_cnf);
      break;
#endif

    default:
      PFREE (dl_establish_cnf);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_dl_establish_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive DL_ESTABLISH_IND received from DL.
            This is only expected for short messages on SAPI 3 during
            a connection.

*/

GLOBAL void dat_dl_establish_ind (T_DL_ESTABLISH_IND *dl_establish_ind)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("dat_dl_establish_ind()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_DEDICATED:
      /*
       * if it indicates a mobile terminated connection establishment
       * for SAPI 3 in the layer 2, change the state in RR for SMS.
       * Answers from upper layer for SMS can be forwarded then
       * immediately.
       */
      if (dl_establish_ind->sapi EQ SAPI_3)
      {
        /*
         * set state for SMS if it is on SAPI 3
         */
        SET_STATE (STATE_SAPI_3, SMS_ESTABLISHED);
        srv_use_stored_prim ();
      }
      break;

    default:
      break;
  }
  PFREE (dl_establish_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_mph_random_access_cnf  |
+--------------------------------------------------------------------+

  PURPOSE : Layer 1 confirms with the primitive MPH_RANDOM_ACCESS_CNF
            the sending of a random burst during connection establishment.

*/

GLOBAL void dat_mph_random_access_cnf (T_MPH_RANDOM_ACCESS_CNF *mph_random_access_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_mph_random_access_cnf()");

  EM_CHANNEL_REQUEST_SENT;

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_IMM_ASS:
      if (rr_data->ms_data.access_counter < rr_data->ms_data.max_attempt)
      {
        /*
         * if it the last or less then last random burst
         * copy the frame number of the random burst for later
         * comparision with the request reference of an immediate
         * assignment message.
         */
        memcpy (&rr_data->used_frame_no[rr_data->ms_data.access_counter],
                &mph_random_access_cnf->frame_no,
                sizeof (T_frame_no));

        /*
        TRACE_EVENT_P5 ("RA %u CNF: %d %d %d 0x%02x",
          rr_data->ms_data.access_counter,
          mph_random_access_cnf->frame_no.t1,
          mph_random_access_cnf->frame_no.t2,
          mph_random_access_cnf->frame_no.t3,
          rr_data->used_channel_ref[rr_data->ms_data.access_counter]);
        */

        /*
         * increment the number of already sent messages
         */
        rr_data->ms_data.access_counter++;

        if (rr_data->ms_data.access_counter EQ rr_data->ms_data.max_attempt)
        {
          /*
           * T3126 and T3146 (GPRS Packet Access on CCCH) are
           * the same so use them for both purposes
           *
           * if it is the last random burst, start T3126 if the timer is not
           * running yet (can be started after reception of an immediate
           * assignment reject message).
           * The timer controls reception of an immediate assignment message
           * as response to the random bursts.
           */
/* Implements Measure#32: Row 196,197 */
          (IS_TIMER_ACTIVE(T3126)) ? 
            TRACE_TIMER ( "T3126 re-start") : TRACE_TIMER ( "T3126 start");
          
          if (! IS_TIMER_ACTIVE(T3126))
          {
            TIMERSTART (T3126, T3126_VALUE);
          }

          /*
           * set a flag that all random bursts are send and confirmed.
           */
          rr_data->ms_data.all_conf_received = TRUE;
        }
      }
      break;

    default:
      break;
  }
  PFREE (mph_random_access_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_dl_release_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Layer 2 confirms with DL_RELEASE_CNF a disconnection on
            layer 2 which has been initiated by RR.

*/

GLOBAL void dat_dl_release_cnf (T_DL_RELEASE_CNF *dl_release_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_dl_release_cnf()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_REL:
      /*
       * after reception of a channel release message RR has
       * started disconnection to come from dedicated to idle
       * mode. Stop T3110, which controls the disconnection in
       * layer 2.
       */
      TIMERSTOP (T3110);
      /*
       * Send STOP_DEDICATED_REQUEST to L1 and wait for confirmation 
       */
      att_stop_dedicated();
      break;

#ifdef GPRS
    case DAT_PDCH_ASS_2:
      dat_rrgrr_resumed_tbf_cnf();
      rr_data->gprs_data.tbf_est = TBF_EST_NONE;  /* the initial state */
      break;
#endif

    case DAT_HANDOVER_4:
      TRACE_EVENT ("Event: DAT_HANDOVER_4");
      /*
       * the layer 2 resumption on the new channel during handover
       * has failed and RR switches back to the old channel.
       */
      dat_code_mph_old_chan_req ();
      SET_STATE (STATE_DAT, DAT_HANDOVER_5);
      break;

    case DAT_CHAN_ASS_1:
      /*
       * resumption of layer 2 has failed after channel assignment
       * go back to the old channel.
       */
      dat_code_mph_old_chan_req ();
      SET_STATE (STATE_DAT, DAT_CHAN_ASS_2);
      break;

    case DAT_DEDICATED:
    case DAT_CHAN_ASS:
    case DAT_CHAN_ASS_2:
    case DAT_HANDOVER_5:
      TRACE_EVENT ("Event: DAT_DEDICATED, DAT_CHAN_ASS, DAT_CHAN_ASS_2 or DAT_HANDOVER_5");

      if (dl_release_cnf->sapi EQ SAPI_3)
      {
        /*
         * the connection for SAPI 3 is disconnected by the
         * network.
         */
        SET_STATE (STATE_SAPI_3, SMS_IDLE);
        dat_rr_release_ind(RRCS_DATA_LINK_FAIL, SAPI_3);

        /*
         * clear any stored SAPI 3 message
         */
        srv_clear_stored_prim (RR_DATA_REQ);
      }
      else
      {
        /*
         * the reconnection to the old channel has failed
         * or a lower layer failure had happen.
         * Indicate the abort to MM and start cell reselection.
         */
        switch (GET_STATE (STATE_DAT))
        {
          case DAT_CHAN_ASS:
          case DAT_CHAN_ASS_2:
            TRACE_EVENT("Assignment failed: reconnect failed");
            EM_ASS_FAILURE_RECONNECT_FAILED;
            break;
          case DAT_HANDOVER_5:
            TRACE_EVENT("Handover failed: reconnect failed");
            EM_HO_FAILURE_RECONNECT_FAILED;
            break;
          default:
            break;
        }

        att_code_rr_abort_ind (RRCS_DATA_LINK_FAIL);

        rr_data->net_lost = TRUE;
        att_stop_dedicated();
      }
      break;

    case DAT_IMM_ASS_1:
      /*
       * Layer 2 establishment has failed for immediate assignment
       * Stop dedicated and go back to idle mode. MM will be informed about
       * release after receiving MPH_STOP_DEDICATED_CNF from L1.
       */
      if (rr_data->ms_data.establish_cause NEQ ESTCS_PAGING)
      {
        rr_data->rel_cause = RRCS_DL_EST_FAIL;
      }
      att_stop_dedicated();
      break;

    default:
      break;
  }
  PFREE (dl_release_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_dl_release_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Layer 2 indicates disconnection due to layer 2 problems.

*/

GLOBAL void dat_dl_release_ind (T_DL_RELEASE_IND *dl_release_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_dl_release_ind()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_ASS:
    case DAT_CHAN_ASS_2:
    case DAT_DEDICATED:
    case DAT_HANDOVER:
    case DAT_HANDOVER_5:
      TRACE_EVENT ("DL_RELEASE_IND");
      if (dl_release_ind->sapi EQ SAPI_3)
      {
        /*
         * indicate release for SAPI 3 (SMS)
         * to MM and reset RR state back to SMS_IDLE.
         */
        dat_rr_release_ind(RRCS_DATA_LINK_FAIL, SAPI_3);
        SET_STATE (STATE_SAPI_3, SMS_IDLE);
        /*
         * clear any stored SAPI 3 message
         */
        srv_clear_stored_prim (RR_DATA_REQ);
      }
      else
      {
        /*
         * Delay 4 frames to allow sending UA response
         * in RF for FTA testcase 25.2.3
         * else the idle mode is configured too early.
         * vsi_t_sleep (VSI_CALLER FOUR_FRAMES);
         *
         * msb: DL delays the release indication itself for completion of the
         *      UA response transmission.
         */
        EM_DL_RELEASE_IND;

        /*
         * Inform MM about the release and start cell reselection.
         */
        att_code_rr_abort_ind (RRCS_DATA_LINK_FAIL);

        rr_data->net_lost = TRUE;
        att_stop_dedicated();
      }
      break;

    case DAT_CHAN_ASS_1:
      /*
       * resumption of layer 2 has failed after channel assignment
       * go back to the old channel.
       */
      dat_code_mph_old_chan_req ();
      SET_STATE (STATE_DAT, DAT_CHAN_ASS_2);

      EM_ASS_FAILURE_RECONNECT_FAILED2
      break;

    case DAT_CHAN_REL:
      /*
       * after reception of a channel release message RR has
       * started disconnection to come from dedicated to idle
       * mode. Stop T3110, which controls the disconnection in
       * layer 2.
       */
      TIMERSTOP (T3110);

      /*
       * Send STOP_DEDICATED_REQUEST to L1 and wait for confirmation 
       */
      att_stop_dedicated();

      EM_L2_CONNECTION_LOST;
      break;

    case DAT_HANDOVER_4:
      TRACE_EVENT ("DL_RELEASE_IND");
      /*
       * resumption of layer 2 has failed handover
       * go back to the old channel.
       */
      dat_code_mph_old_chan_req ();
      SET_STATE (STATE_DAT, DAT_HANDOVER_5);

      EM_HO_FAILURE_RECONNECT_FAILED2;

      break;

    case DAT_IMM_ASS_1:

      /*
       * layer 2 establishment has failed during immediate assignment
       */
      if (dl_release_ind->cs EQ DL_INFO_FIELD_MISMATCH AND
          rr_data->sc_data.first_attempt)
      {
        /*
         * if the reason is a mismatch in the layer 3 messages in SABM
         * and the response UA, a second attempt of establishment is started.
         */
        rr_data->sc_data.first_attempt = FALSE;
        rr_data->repeat_est    = TRUE;
#ifdef GPRS
        if (rr_data->ms_data.establish_cause EQ ESTCS_GPRS_PAGING)
          rr_data->dcch_stop_cause = CONTENTION_RESOLUTION_FAIL;
#endif
      }
      else
      {
        switch (rr_data->ms_data.establish_cause)
        {
#ifdef GPRS
          case ESTCS_GPRS_PAGING:
            rr_data->dcch_stop_cause = DL_ESTABLISHMENT_FAIL;
            break;
          case ESTCS_PAGING:
            rr_data->rel_cause = RRCS_INT_NOT_PRESENT;
#else
          case ESTCS_PAGING:
#endif
            break;
          default:
            rr_data->rel_cause = RRCS_DL_EST_FAIL;
            break;
        }
      }

      /*
       * Stop dedicated and go back to idle mode.
       */
      att_stop_dedicated();
      break;

#ifdef GPRS
    case DAT_PDCH_ASS_4:
    case DAT_CCO_4:
      if ( GET_STATE (STATE_DAT) EQ DAT_PDCH_ASS_4 OR
           rr_data->gprs_data.cco_need_reconnect_cnf )
      {
        dat_rrgrr_reconnect_dcch_cnf (RECONN_LOW_FAIL);
      }
      att_build_idle_req (SC_INDEX, MODE_CELL_RESELECTION);
      SET_STATE (STATE_DAT, DAT_IDLE);
      break;
#endif

    default:
      break;
  }
  PFREE (dl_release_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_mph_dedicated_cnf      |
+--------------------------------------------------------------------+

  PURPOSE : Layer 1 confirms the configuration of a dedicated channel.

*/

GLOBAL void dat_mph_dedicated_cnf (T_MPH_DEDICATED_CNF *mph_dedicated_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_mph_dedicated_cnf()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_IMM_ASS_1:
      /*
       * A dedicated channel has been configured after
       * reception of the immediate assignment message.
       *
       * clear neighbourcell list and start layer 2 establishment
       */
      srv_clear_list (&rr_data->sc_data.cd.ncell_list);
      srv_clear_list (&rr_data->sc_data.five_ter_list);
      att_clean_buf (IND_ALL_DEDI_SI);
      rr_data->sc_data.cd.sys_info_read &= ~ALL_DEDI_SYS_INFOS;
      /*
       * from now on we have a channel assigned
       * even if the contention resolution is not
       * done yet. If the contention resolution fails
       * we use the same procedure for selecting a cell
       * as in dedicated mode
       */

      rr_data->mode_after_dedi = MODE_CELL_SELECTION; /* default mode */
      TIMERSTART (T_DEDICATED_MODE, THIRTY_SEC);
#ifdef GPRS
      if(! dat_gprs_start_sabm())
#endif
        dat_start_sabm ();
      break;

    case DAT_CHAN_ASS:

      if (rr_data->dyn_config.fca)
      {
        /*
         * special testfeature to force a failed layer 2 establishment
         * during  channel assignment. Reset flag and switch back to old
         * channel.
         */
        rr_data->dyn_config.fca = 0;
        dat_code_mph_old_chan_req ();
        SET_STATE (STATE_DAT, DAT_CHAN_ASS_2);
      }
      else
      {
        /*
         * Resume layer 2 connection on the new channel with
         * an assignment complete message.
         */
        MCAST (assign_com, U_ASSIGN_COMP);
        PALLOC_MSG ( dl_resume_req, DL_RESUME_REQ, U_ASSIGN_COMP);

        SET_STATE (STATE_DAT, DAT_CHAN_ASS_1);
        assign_com->msg_type  = U_ASSIGN_COMP;
        assign_com->rr_cause  = RRC_NORMAL_EVENT;

        /*
         * set channel and sapi for the new channel description
         */
        dat_code_prr_channel (&dl_resume_req->ch_type,
                              &dl_resume_req->sapi,
                              rr_data->cr_data.chan_desc.chan_type);

        /*
         * start layer 2 resumption.
         */
        for_dat_resume_req (dl_resume_req);
        EM_ASSIGNMENT_COMPLETE;
      }
      break;

#ifdef GPRS
    case DAT_PDCH_ASS:
      /*
       * The current channel has been stopped at the Physical Layer.
       */
      dat_rrgrr_suspend_dcch_cnf();
      SET_STATE (STATE_DAT, DAT_PDCH_ASS_1);
      SET_STATE (STATE_GPRS, GPRS_PIM_BCCH); /* force MPH_IDLE_REQ with RRGRR_STOP_MON_CCCH_REQ */
      rr_data->gprs_data.tbf_est = TBF_EST_PDCH;
      break;
#endif

    case DAT_HANDOVER:
    {
      /*
       * clearing of neighbourcells is already done
       * after reception of the handover command.
       *
       * following behaviour depends on the result of
       * handover execution in layer 1.
       */
      switch (mph_dedicated_cnf->dedi_res)
      {
        case DEDI_RES_OK:
          /*
           * handover is successfull.
           */
          if (rr_data->dyn_config.fca)
          {
            /*
             * special testfeature to simulate
             * failed handover, reset flag
             * and start reconnection.
             */
            rr_data->dyn_config.fca = 0;
            dat_code_mph_old_chan_req ();
            SET_STATE (STATE_DAT, DAT_HANDOVER_5);
          }
          else
          {
            /*
             * build a handover complete message.
             */
            MCAST (handov_comp, U_HANDOV_COMP);
            PALLOC_MSG (dl_resume, DL_RESUME_REQ, U_HANDOV_COMP);

            if (rr_data->ms_data.ho_type.rot EQ TIME_DIFF_YES)
            {
              /*
               * handover command has requested the observed time difference.
               */
              handov_comp->v_mob_time_diff = TRUE;
              handov_comp->mob_time_diff.diff   = rr_data->sc_data.observed_ta;
            }
            else
            {
              handov_comp->v_mob_time_diff = FALSE;
            }

            handov_comp->msg_type = U_HANDOV_COMP;
            handov_comp->rr_cause = RRC_NORMAL_EVENT;

            /*
             * set channel type and sapi for the new channel
             */
            dat_code_prr_channel (&dl_resume->ch_type,
                                  &dl_resume->sapi,
                                  rr_data->cr_data.chan_desc.chan_type);

            EM_HANDOVER_COMPLETE;

            /*
             * start layer 2 resumption.
             */
            for_dat_resume_req (dl_resume);
            SET_STATE (STATE_DAT, DAT_HANDOVER_4);
          }
          break;

        case DEDI_RES_TIMEOUT:
          /*
           * the timer T3124 during an asynchronous handover
           * has timed out, start reconnection.
           */
          dat_code_mph_old_chan_req ();
          SET_STATE (STATE_DAT, DAT_HANDOVER_5);
          break;

        case DEDI_RES_CELL_NOT_SYNC :
           /*
            * Timing Info for this cell is not present in ALR.
            * This could be due to :
            * a) Cell not present in BA list,
            * b) Synchronization to this cell failed or not attempted.
            */
        case DEDI_RES_TA_OUT_OF_RANGE:
        {
          /*
           * Layer 1 has detected that the timing advance is out of range.
           * In fact the new channel has not been configured and RR
           * can start reconnection to the old layer 2 connection immediately.
           * It sends a handover failure message with the expected cause.
           */
          MCAST (handov_fail, U_HANDOV_FAIL);
          PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_HANDOV_FAIL);

          /*
           * set channel type and sapi for the old channel
           */
          dat_code_prr_channel (&dl_reconnect_req->ch_type,
                                &dl_reconnect_req->sapi,
                                rr_data->sc_data.chan_desc.chan_type);
          handov_fail->msg_type = U_HANDOV_FAIL;

          if( mph_dedicated_cnf->dedi_res EQ DEDI_RES_TA_OUT_OF_RANGE)
            handov_fail->rr_cause = RRC_TIME_ADVANCE;
          else if ( mph_dedicated_cnf->dedi_res EQ DEDI_RES_CELL_NOT_SYNC)
#if defined (REL99) && defined (FF_BHO)
            handov_fail->rr_cause = RRC_LOWER_LAYER_FAIL;
#else
            handov_fail->rr_cause = RRC_CHAN_UNACCEPT;
#endif
          RR_EM_SET_HANDOVER_FAIL_CAUSE(handov_fail->rr_cause);

          /*
           * start reconnection of the layer 2 link on the old channel.
           */
          for_dat_reconnect_req (dl_reconnect_req);
          SET_STATE (STATE_DAT, DAT_HANDOVER_5);

          break;
        }
      }
      break;
    }

    default:
      break;
  }
  PFREE (mph_dedicated_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_mph_dedicated_fail_cnf |
+--------------------------------------------------------------------+

  PURPOSE : Layer 1 confirms the re-configuration of the old channel
            after a failed handover or channel assignment with the
            primitive MPH_DEDICATED_FAIL_CNF.

*/

GLOBAL void dat_mph_dedicated_fail_cnf (T_MPH_DEDICATED_FAIL_CNF *mph_dedicated_fail_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_mph_dedicated_fail_cnf()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_ASS_2:
    {
      /*
       * reconnection is done in layer 2. An assignment failure
       * message is send to the network.
       */
    
      MCAST (assign_fail, U_ASSIGN_FAIL);
      PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_ASSIGN_FAIL);

      assign_fail->msg_type = U_ASSIGN_FAIL;
      assign_fail->rr_cause = RRC_PROT_UNSPECIFIED;

      RR_EM_SET_ASSIGN_FAIL_CAUSE(assign_fail->rr_cause);

      /*
       * set channel type and SAPI for layer 2
       */
      dat_code_prr_channel (&dl_reconnect_req->ch_type,
                            &dl_reconnect_req->sapi,
                            rr_data->sc_data.chan_desc.chan_type);

      /*
       * start layer 2 reconnection
       */
      for_dat_reconnect_req (dl_reconnect_req);

      break;
    }

    case DAT_HANDOVER_5:
    {
      /*
       * reconnection is done in layer 2. A handover failure
       * message is send to the network.
       */
      MCAST (handov_fail, U_HANDOV_FAIL);
      PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_HANDOV_FAIL);

      /*
       * clear neighbourcell list
       */
      srv_clear_list (&rr_data->sc_data.cd.ncell_list);
      srv_clear_list (&rr_data->sc_data.five_ter_list);
      att_clean_buf (IND_ALL_DEDI_SI);
      rr_data->sc_data.cd.sys_info_read &= ~ALL_DEDI_SYS_INFOS;
      handov_fail->msg_type = U_HANDOV_FAIL;
      handov_fail->rr_cause = RRC_UNSPECIFIED;

      RR_EM_SET_HANDOVER_FAIL_CAUSE(handov_fail->rr_cause);

      /*
       * set channel type and SAPI for layer 2
       */
      dat_code_prr_channel (&dl_reconnect_req->ch_type,
                            &dl_reconnect_req->sapi,
                            rr_data->sc_data.chan_desc.chan_type);

      /*
       * start layer 2 reconnection
       */
      for_dat_reconnect_req (dl_reconnect_req);
      break;
    }

#ifdef GPRS
    case DAT_PDCH_ASS_3:
    {
      MCAST (u_assign_fail, U_ASSIGN_FAIL);
      PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_ASSIGN_FAIL);

      /*
       * set channel type and SAPI
       */
      dat_code_prr_channel (&dl_reconnect_req->ch_type,
                            &dl_reconnect_req->sapi,
                            rr_data->sc_data.chan_desc.chan_type);

      u_assign_fail->msg_type = U_ASSIGN_FAIL;
      u_assign_fail->rr_cause = rr_data->gprs_data.reconn_cause;

      for_dat_reconnect_req (dl_reconnect_req);
  
      SET_STATE (STATE_DAT, DAT_PDCH_ASS_4);
      break;
    }

    case DAT_CCO_3:
    {
      MCAST (u_handov_fail, U_HANDOV_FAIL);
      PALLOC_MSG (dl_reconnect_req, DL_RECONNECT_REQ, U_HANDOV_FAIL);

      /*
       * set channel type and SAPI
       */
      dat_code_prr_channel (&dl_reconnect_req->ch_type,
                            &dl_reconnect_req->sapi,
                            rr_data->sc_data.chan_desc.chan_type);

      u_handov_fail->msg_type = U_HANDOV_FAIL;
      u_handov_fail->rr_cause = rr_data->gprs_data.reconn_cause;

      for_dat_reconnect_req (dl_reconnect_req);
      SET_STATE (STATE_DAT, DAT_CCO_4);
      break;
    }
#endif

    default:
      break;
  }
  PFREE (mph_dedicated_fail_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_rr_abort_req           |
+--------------------------------------------------------------------+

  PURPOSE : MM aborts a connection due to several reasons (SIM remove,
            timeout, power off etc.).

*/

GLOBAL void dat_rr_abort_req (T_RR_ABORT_REQ *rr_abort_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_rr_abort_req()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_CHAN_ASS:
    case DAT_CHAN_ASS_1:
    case DAT_CHAN_ASS_2:
    case DAT_DEDICATED:
    case DAT_HANDOVER:
    case DAT_HANDOVER_4:
    case DAT_HANDOVER_5:
      if (rr_abort_req->abcs EQ ABCS_SIM_REM)
      {
        /*
         * in case of SIM remove the registration data
         * especially the mobile identities are cleared
         */
        att_clear_registration_data ();
      }

      /*
       * the disconnection of the layer 2 link is started.
       */
      dat_disconnect_link (RRCS_MM_ABORTED);
      break;

    case DAT_CHAN_REL:
      /*
       * RR has already started the layer 2 link disconnection.
       * Wait for T3110 timeout or DL disconnection and process
       * normal cell reselection.
       */
      if (rr_abort_req->abcs EQ ABCS_SIM_REM)
      {
        /*
         * in case of SIM remove the registration data
         * especially the mobile identities are cleared
         */
        att_clear_registration_data ();
      }
      break;

    case DAT_IDLE:
      if (rr_abort_req->abcs EQ ABCS_SIM_REM)
      {
        /*
         * in case of SIM remove the registration data
         * especially the mobile identities are cleared
         * The idle mode is configured again to set a
         * faked BS_PA_MFRMS of 9 to slow down layer 1
         * and save power. A correct paging group is not
         * longer needed, because paging is not possible
         * in limited service.
         */
        att_clear_registration_data ();
        att_build_idle_req (SC_INDEX,
                            MODE_SYS_INFO_CHANGE);
#ifdef REL99
        att_config_cbch ();
#else
        att_build_cbch ();
#endif
      }
      break;

    case DAT_IMM_ASS:
    case DAT_IMM_ASS_1:
      if (rr_abort_req->abcs EQ ABCS_SIM_REM)
      {
        /*
         * in case of SIM remove the registration data
         * especially the mobile identities are cleared
         */
        att_clear_registration_data ();
      }

      /*
       * stop any timer related to connection establishment
       */
      TIMERSTOP (T3122);
      TIMERSTOP (T3126);

      /*
       * set the release establishment cause. This will be used in 
       * dat_release_connection() function
       */
      rr_data->rel_cause = RRCS_MM_ABORTED;

      if(GET_STATE (STATE_DAT) EQ DAT_IMM_ASS)
      {
        dat_rr_release_ind(rr_data->rel_cause, SAPI_0);
        att_leave_dat_imm_ass();
      }
      else
      {
        /*
         * go back to idle mode.
         * inform GRR, and don't wait for CR_RSP 
         */
        att_stop_dedicated();
      }
      break;

    default:
      if (rr_abort_req->abcs EQ ABCS_SIM_REM)
      {
        /*
         * in case of SIM remove the registration data
         * especially the mobile identities are cleared
         */
        att_clear_registration_data ();
      }
      /*
       * clear any store establish requests if available
       * due to a timeout abort of the upper layer.
       */
      if (srv_check_stored_prim (RR_ESTABLISH_REQ))
      {
        /*
         * release previous establish request
         */
        dat_rr_release_ind(RRCS_MM_ABORTED, SAPI_0);
        srv_clear_stored_prim (RR_ESTABLISH_REQ);
      }
      break;
  }

  PFREE (rr_abort_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_rr_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : A layer 3 message of an upper layer shall be send.

*/

GLOBAL void dat_rr_data_req (T_RR_DATA_REQ *rr_data_req)
{
  GET_INSTANCE_DATA;
  UBYTE pd;

  TRACE_FUNCTION ("dat_rr_data_req()");

  /*
   * get the protocol discriminator of the message.
   */
  GET_PD (rr_data_req->sdu, pd);

  if (pd EQ PD_SMS)
  {
    TRACE_EVENT ("SMS Message");

    /*
     * A SMS must be send with SAPI 3. Therefore the
     * SAPI 3 connection must be established on layer 2.
     * The STATE_SAPI_3 variable in RR indicates whether
     * the connection is established or not. If not the
     * message is stored and the connection will be established.
     * A SAPI 3 connection is automatically released after
     * channel assignment, handover or SAPI 0 release.
     */
    switch (GET_STATE (STATE_SAPI_3))
    {
      case SMS_ESTABLISHED:
      {
        /*
         * SAPI 3 connection is available, then send the message
         * directly.
         */       
        PPASS (rr_data_req, dl_data_req, DL_DATA_REQ);

        /*
         * set channel type and SAPI for SMS message.
         */
        dat_code_prr_channel_sms (dl_data_req,
                                  rr_data->sc_data.chan_desc.chan_type);
        for_dat_l3_data_req (dl_data_req);  
        break;
      }

      case SMS_PENDING:
        /*
         * the establishment of SAPI 3 connection is still ongoing.
         * store the message until connection is ready.
         */
        if (!srv_store_prim ((T_PRIM *)D2P(rr_data_req)))
        {
          PFREE (rr_data_req);
        }
        break;

      case SMS_IDLE:
        /*
         * the establishment of SAPI 3 connection is not available
         * store the message until connection is ready.
         */
        if (!srv_store_prim ((T_PRIM *)D2P(rr_data_req)))
        {
          PFREE (rr_data_req);
        }
        else
        {
          /*
           * Maximum size of the initial message is one frame (= 23 Bytes).
           */
          PALLOC_SDU (dl_establish_req, DL_ESTABLISH_REQ, MAX_L2_FRAME_SIZE * BITS_PER_BYTE);
          dat_code_prr_channel_sms ((T_DL_DATA_REQ *)dl_establish_req,
                                    rr_data->sc_data.chan_desc.chan_type);

          SET_STATE (STATE_SAPI_3, SMS_PENDING);
          for_dat_est_req (dl_establish_req);
        }
        break;
    }
  }
  else
  {
    /*
     * it is a SAPI 0 message
     */
    PPASS (rr_data_req, dl_data_req, DL_DATA_REQ);

    /*
     * set channel type and SAPI according the channel type.
     */
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);

    /*
     * set N(S) for upper layer message and forward it to Layer 2.
     */
    dat_vsd_bit_set ((T_L3_SDU *)&dl_data_req->sdu, SET_ONLY);
    for_dat_l3_data_req (dl_data_req);   
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_rr_establish_req       |
+--------------------------------------------------------------------+

  PURPOSE : A mobile originated connection is started.

*/

GLOBAL void dat_rr_establish_req (T_RR_ESTABLISH_REQ *rr_establish_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_rr_establish_req()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_NULL:
      /*
       * Data transfer process is in null state, that means
       * the MS has no service
       */
      if (!srv_store_prim ((T_PRIM *)D2P(rr_establish_req)))
      {
        /*
         * storing is not possible due to an overflow of the storage area
         * then reject the connection attempt.
         */
        dat_rr_release_ind(RRCS_QUEUE_FULL, SAPI_0);
        PFREE (rr_establish_req);
      }
      break;

    case DAT_IDLE:
      if (IS_TIMER_ACTIVE(T3122) AND (rr_establish_req->estcs NEQ ESTCS_EMRG_CAL))
      {
        /*
         * all non-emergency calls are rejected if T3122 is running.
         * The timer has been started after reception of an immediate
         * assignment reject message.
         */
        dat_rr_release_ind(RRCS_T3122_RUNNING, SAPI_0);
        PFREE (rr_establish_req);

        TRACE_EVENT ("imm ass delayed");
      }
      else
      {
        /*
         * If T3122 is not running the access to the network is checked.
         */
        if (dat_access_allowed (rr_establish_req->estcs))
        {
          if (rr_establish_req->estcs EQ ESTCS_SERV_REQ_BY_MM)
          {
            /*
             * if location updating is started, store fieldstrength
             * for an optimisation. RR indicates to MM fieldstrength jumps
             * over 6 dBm in the field, that means better chance to
             * perform a successfull location updating.
             */
            rr_data->lup_rxlev = rr_data->nc_data[SC_INDEX].rxlev;
          }

          /*
           * store the piggy-backed layer 3 message for later use.
           */
          memcpy (&rr_data->ms_data.l3msg, &rr_establish_req->sdu,
                  sizeof (T_L3_SDU));/*lint !e420 Apparent access beyond array for function*/

          /*
           * Initialize the N(S) for upper layer messages.
           */
          dat_vsd_bit_set (&rr_data->ms_data.l3msg, SET_AND_RESET);

          /*
           * Initialize some parameters and start immediate assignment.
           */
          rr_data->sc_data.first_attempt = TRUE;
          rr_data->repeat_est    = FALSE;
#ifdef GPRS
          dat_gprs_set_suspended();
#endif
          dat_start_immediate_assign (rr_establish_req->estcs);
          PFREE (rr_establish_req);
        }
        else
        {
          /*
           * Access is not allowed and the rejection is signalled to MM.
           */
          dat_rr_release_ind(RRCS_ACCESS_BARRED, SAPI_0);
          PFREE (rr_establish_req);

          TRACE_EVENT ("access barred");
        }
      }
      break;

    case DAT_IMM_ASS:
    case DAT_IMM_ASS_1:
    case DAT_DEDICATED:
      {
        /*
         * collision of MO and MT calls. MT has higher priority than
         * MO calls.
         */
        dat_rr_release_ind(RRCS_MO_MT_COLL, SAPI_0);
        PFREE (rr_establish_req);
        break;
      }

    default:
      PFREE (rr_establish_req);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_mph_paging_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Layer 1 has detected a paging for the mobile.
            RR starts the immediate assignment procedure.

*/

GLOBAL void dat_mph_paging_ind (T_MPH_PAGING_IND *mph_paging_ind)
{
  GET_INSTANCE_DATA;
#ifdef GPRS
  UBYTE ret;
#endif
  TRACE_FUNCTION ("dat_mph_paging_ind()");

  EM_PAGING_IND;

  /* check access control class before processing for paging */
  if (dat_access_allowed(ESTCS_PAGING) EQ FALSE)
  {
    PFREE (mph_paging_ind);
    TRACE_EVENT("Access ctrl class not allowed for responding to the page msg");
    return;
  }

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_NULL:
      /*
       * MS is not in idle mode, searching for cell
       */
      if (! IS_TIMER_ACTIVE(T3122) AND GET_STATE (STATE_ATT) NEQ ATT_NULL)
      {
        /*
         * paging is only excepted if T3122 is not running and there isn't
         * performed a RR_DEACTIVATE_REQ just before
         */
        if (rr_data->pag_rec EQ FALSE)
        {
          /*
           * store paging if no paging is stored until now.
           */
          if (!srv_store_prim ((T_PRIM *)D2P(mph_paging_ind)))
          {
            /*
             * storage is full
             */
            PFREE (mph_paging_ind);
          }
          else
          {
            /*
             * marker that paging has been received.
             */
            rr_data->pag_rec = TRUE;
          }
          return;
        }
      }
      break;

    case DAT_IDLE:
      if (! IS_TIMER_ACTIVE(T3122))
      {
        if (rr_data->first_meas_received EQ FALSE)
        {
          /*
           *  we are actually still in cell reselection
           *  because we have not yet received a measurement
           *  report. But we have to be in idle mode to make
           *  measurements in the first place.
           */

          if (srv_check_stored_prim (MPH_PAGING_IND))
          {
            /*
             * if already a paging is stored, ignore subsequent pagings
             */
            PFREE (mph_paging_ind);
          }
          else
          {
            /*
             * store paging until measurement receives
             */
            rr_data->pag_rec = TRUE;
            if (!srv_store_prim ((T_PRIM *)D2P(mph_paging_ind)))
            {
              /*
               * storage buffer is full
               */
              rr_data->pag_rec = FALSE;
              PFREE (mph_paging_ind);
            }
          }
          return;
        }

#ifdef GPRS
        ret = dat_check_packet_paging_ind(mph_paging_ind);
        if(ret EQ FALSE)
        {
            /*
             * we are suspended and it is not a packet paging
             */
            dat_begin_start_immediate_assign (mph_paging_ind->identity_type,
                                              mph_paging_ind->channel_needed);
        }
        else if(ret EQ TRUE)
        {
          /*
           * we are not suspended and it is not a packet paging
           * and GPRS is activate
           */
          dat_ask_paging_ind(mph_paging_ind);
        }
        /* ret EQ 2-> packet paging for GPRS, do nothing */
#else  /* GPRS */
        dat_begin_start_immediate_assign (mph_paging_ind->identity_type,
                                          mph_paging_ind->channel_needed);
#endif  /* GPRS */
      }

      break;
#ifdef GPRS
    case DAT_IMM_ASS:
      /* only PS pagings are received in this state */
      dat_ask_paging_ind_pa_only(mph_paging_ind);
      break;
#endif
    default:
      break;
  }
  PFREE (mph_paging_ind);
}

#if defined FF_EOTD
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_rrlc_meas_req          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RRLC_MEAS_REQ received from LC.
            Start the LCS/EOTD postion measurement procedure.

*/

GLOBAL void dat_rrlc_meas_req (T_RRLC_MEAS_REQ *rrlc_meas_req)
{
  GET_INSTANCE_DATA;
  T_NC_DATA   *sc;
  int         i,n;

  TRACE_FUNCTION ("dat_rrlc_meas_req()");

  switch ( GET_STATE (STATE_DAT) )
  {
    case DAT_HANDOVER  :
    case DAT_HANDOVER_4:
    case DAT_HANDOVER_5:
      {
        PALLOC (rrlc_error_ind, RRLC_ERROR_IND);
        rrlc_error_ind->cause = LCS_HANDOVER;

        rr_data->eotd_req_id = NOT_PRESENT_16BIT;

        PSENDX (LC, rrlc_error_ind);

        PFREE(rrlc_meas_req);
        return;
      }
    default:
      break;
  }

  /*
   *  Check if the requested cell matches the serving cell.
   */

  sc = &rr_data->nc_data[SC_INDEX];

  if ( rrlc_meas_req->v_arfcn              AND
       rrlc_meas_req->  arfcn NEQ sc->arfcn AND
       rrlc_meas_req->v_bsic               AND
       rrlc_meas_req->  bsic  NEQ sc->bsic     )
  {
    PALLOC (rrlc_error_ind, RRLC_ERROR_IND);
    rrlc_error_ind->cause = LCS_WRONG_BTS;
    PSENDX (LC, rrlc_error_ind);

    PFREE(rrlc_meas_req);
    return;
  }

  /*
   *  The requested cell matches the serving cell or
   *  the requested cell was not specified.
   */

  {
    PALLOC ( mph_ncell_pos_req, MPH_NCELL_POS_REQ );

    rr_data->eotd_req_id = rrlc_meas_req->req_id;

    n = rrlc_meas_req->v_assist_data ? rrlc_meas_req->c_assist_data : 0;
    mph_ncell_pos_req->c_ncell_eotd = n;
    mph_ncell_pos_req->req_id       = rrlc_meas_req->req_id;
    for ( i = 0; i < n; ++i )
      mph_ncell_pos_req->ncell_eotd[i] = *(T_ncell_eotd*)&rrlc_meas_req->assist_data[i];

    PSENDX ( PL, mph_ncell_pos_req );
  }
  PFREE(rrlc_meas_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_rrrrlp_data_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RRRRLP_DATA_REQ received from LC.
            Transmit a APDU according 3GPP TS 04.18, section 3.4.21.

*/

GLOBAL void dat_rrrrlp_data_req (T_RRRRLP_DATA_REQ *rrrrlp_data_req_orig)
{
  GET_INSTANCE_DATA;

  UBYTE  cr      =  rrrrlp_data_req_orig->cr;
  T_sdu *sdu     = &rrrrlp_data_req_orig->sdu;
  UBYTE *buf     = &sdu->buf[sdu->o_buf/BITS_PER_BYTE];
  USHORT len     = BYTELEN(sdu->l_buf);
  USHORT num_seg = len / APDU_FULL_L2_FRAME;      /* number of full segments */
  USHORT rem_seg = len % APDU_FULL_L2_FRAME;      /* size of last segment    */
  USHORT n_seg;                                   /* total number of segments*/
  USHORT i;
  UBYTE  flag;

  PPASS(rrrrlp_data_req_orig, rrrrlp_data_req, RRRRLP_DATA_REQ);

  TRACE_FUNCTION ("dat_rrrrlp_data_req()");

  /* sdu->o_buf must be a multiple of BITS_PER_BYTE */
  TRACE_ASSERT ( sdu->o_buf % BITS_PER_BYTE EQ 0 );

  /* compute the total number of segments (n_seg) */

  if ( len EQ 0 )
  {
    PFREE(rrrrlp_data_req);
    return;
  }

  n_seg = num_seg + ((rem_seg EQ 0) ? 0 : 1);

  /* in a loop with DL_DATA_REQ send all APDU segments */

  for ( i = 0; i < num_seg; ++i )
  {
    PALLOC_MSG ( dl_data_req, DL_DATA_REQ, B_APPLIC_INFO );

    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type );

    /* compute the APDU control flags */

    if ( n_seg EQ 1 )
    {
      flag = (UBYTE)(FIRST_SEG | LAST_SEG | (cr << 2));
    }
    else
    {
      if ( i EQ 0 )
      {
        flag = FIRST_SEG | NOT_LAST_SEG;
      }
      else
      {
        if ( i EQ num_seg-1 AND rem_seg EQ 0 )
          flag = NOT_FIRST_SEG | LAST_SEG | (cr << 2);
        else
          flag = NOT_FIRST_SEG | NOT_LAST_SEG;
      }
    }

    /* compile the APDU message and send it to DL */

    dl_data_req->sdu.buf[0] = PD_RR_TI_0;
    dl_data_req->sdu.buf[1] = B_APPLIC_INFO;
    dl_data_req->sdu.buf[2] = (flag << 4 ) | RRLP_LCS;
    dl_data_req->sdu.buf[3] = APDU_FULL_L2_FRAME;
    memcpy ( &dl_data_req->sdu.buf[4], buf, APDU_FULL_L2_FRAME );

    buf += APDU_FULL_L2_FRAME;
    dl_data_req->sdu.l_buf = (4 + APDU_FULL_L2_FRAME) * BITS_PER_BYTE;
    dl_data_req->sdu.o_buf = 0;

    PSENDX ( DL, dl_data_req );
  }

  /* send remaining APDU segment if its length > 0 */

  if ( rem_seg > 0 )
  {
    PALLOC_MSG ( dl_data_req, DL_DATA_REQ, B_APPLIC_INFO );

    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type );

    /* compute the APDU control flags */

    if ( n_seg EQ 1 )
      flag = FIRST_SEG | LAST_SEG | (cr << 2);
    else
      flag = NOT_FIRST_SEG | LAST_SEG | (cr << 2);

    /* compile the APDU message and send it to DL */

    dl_data_req->sdu.buf[0] = PD_RR_TI_0;
    dl_data_req->sdu.buf[1] = B_APPLIC_INFO;
    dl_data_req->sdu.buf[2] = (flag << 4 ) | RRLP_LCS;
    dl_data_req->sdu.buf[3] = (UBYTE)rem_seg;
    memcpy ( &dl_data_req->sdu.buf[4], buf, rem_seg );

    dl_data_req->sdu.l_buf = (4 + rem_seg) * BITS_PER_BYTE;
    dl_data_req->sdu.o_buf = 0;

    PSENDX ( DL, dl_data_req );
  }

  /* release RRRRLP_DATA_REQ */
  PFREE(rrrrlp_data_req);
#ifdef REL99
  /* Send RRLP procedure stop indication to MM*/
  {
    PALLOC (rr_rrlp_stop_ind, RR_RRLP_STOP_IND);
    PSENDX (MM, rr_rrlp_stop_ind);
  }
#endif
}
#endif /* FF_EOTD */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : att_mph_stop_dedicated_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MPH_STOP_DEDICATED_CNF received from L1
*/
GLOBAL void att_mph_stop_dedicated_cnf (T_MPH_STOP_DEDICATED_CNF * stop_cnf)
{
  GET_INSTANCE_DATA;
  PFREE(stop_cnf);
  TRACE_FUNCTION ("att_mph_stop_dedicated_cnf()");
  /*
   * Dedicated mode activity can be stopped in 3 cases
   *
   * - normal case :- Dedicated mode actvity has been stopped cleanly, without any errors.
   *                  Either a cell selection or a cell reselection (to the same cell) will take
   *                  place, depending on the timer T_DEDICATED_MODE.
   *
   * - failure :- Dedicated mode actvity has been stopped because of a radio link failure or a 
   *              data link failure. In this case cell re-selection must be started immediately.
   *
   * - CCO :- This is Applicable after RR has received a network cell change order
   */
  switch(GET_STATE (STATE_ATT))
  {
    case ATT_NULL:
    case ATT_CS1:
    case ATT_CS2:
      break;

    default:
#ifdef GPRS
      if( rr_data->mode_after_dedi EQ MODE_CELL_CHANGE_ORDER)
      {
        PALLOC (mph_bsic_req, MPH_BSIC_REQ);
        mph_bsic_req->arfcn = rr_data->gprs_data.arfcn;
        SET_STATE (STATE_ATT, ATT_IDLE);
        SET_STATE (STATE_CELL_SEL, CS_CCO);
        rr_data->mode_after_dedi = NOT_PRESENT_8BIT;
        PSENDX( PL, mph_bsic_req);
        return;
      } 
#endif
      if(! rr_data->net_lost )
      {
        /* Send Release indication to MM and go back to IDLE state */
        dat_release_connection();
      }
      else
      {
#ifdef GPRS
        att_start_cell_reselection_gprs (BACK_FROM_DEDICATED_RLF);
#else
        att_start_cell_reselection (BACK_FROM_DEDICATED_RLF);
#endif
      }
      break;
   } 
}
#endif

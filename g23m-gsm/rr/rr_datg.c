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
|  Purpose :  This module holds the functions for handling primitives
|             sent from entity GRR to entity RR and vice versa.
+-----------------------------------------------------------------------------
*/

#ifndef RR_DATG_C
#define RR_DATG_C

#ifdef GPRS

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stddef.h>
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
#include "pei.h"
#include "tok.h"
#include "rr_gprs.h"
#include "rr.h"

static UBYTE get_r_bit (void);
static void  handle_non_gprs_param(T_non_gprs* non_gprs);
static void  dat_rrgrr_rr_est_ind (void);
static void  dat_build_rr_initialisation_request  (void);

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_channel_req               |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_CHANNEL_REQ received from GRR.
            This primitive indicates to the RR to send a channel request

*/

void dat_rrgrr_channel_req (T_RRGRR_CHANNEL_REQ *chan_req)
{
  GET_INSTANCE_DATA;
  USHORT req = (USHORT) (chan_req->req_data + 0x0400);
  TRACE_FUNCTION ("gprs_rrgrr_channel_req()");

  /*
   * check availability of GPRS
   */
  switch (GET_STATE(STATE_GPRS)) /* rr_data->gprs_data.gprs_avail) */
  {
    case GPRS_PIM_BCCH:
    case GPRS_PAM_BCCH:
    case GPRS_PTM_BCCH:
      if (GET_STATE(STATE_DAT) NEQ DAT_NULL)
      {
        switch (req)
        {
          case ESTCS_GPRS_1P:
          case ESTCS_GPRS_SB:
          case ESTCS_PAGING:
            /*
             * reset values used in the Packet Access
             * Procedure
             */
            if (IS_TIMER_ACTIVE(T3126))
            {
              TIMERSTOP(T3126);
            }
            memset(rr_data->gprs_data.rr_sdu, NOT_PRESENT_8BIT,
                   sizeof(rr_data->gprs_data.rr_sdu));
            rr_data->gprs_data.tma_in_progress = FALSE;
            rr_data->gprs_data.fn              = NOT_PRESENT_32BIT;
            rr_data->gprs_data.req_ref_idx     = NOT_PRESENT_8BIT;
            rr_data->sc_data.first_attempt     = TRUE;
            rr_data->repeat_est                = FALSE;
            /* start procedure */
            dat_start_immediate_assign((USHORT)(req EQ
                                       ESTCS_PAGING ? ESTCS_GPRS_PAGING : req));
            SET_STATE(STATE_GPRS, GPRS_PAM_BCCH);
            break;
          default:
            TRACE_EVENT("channel request with wrong value");
            break;
        }
      }
      else
      {
        TRACE_EVENT ("store channel req");
        if(! srv_store_prim((T_PRIM *)D2P(chan_req)))
        {
          TRACE_EVENT("prim store full");
          PFREE(chan_req);
        }
        return;
      }
      break;
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
      TRACE_EVENT ("chan_req w/ PBCCH!");
      break;
    case GPRS_ACTIVATED:
      TRACE_EVENT ("no cell selected yet, or cell supports no GPRS");
      break;
    case GPRS_SUSPENDED_BCCH:
    case GPRS_SUSPENDED_PBCCH:
      TRACE_EVENT ("GPRS is suspended!");
      break;
    default:
      TRACE_EVENT ("GPRS not supported");
      break;
  }
  PFREE (chan_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_check_gprs_imm_ass              |
+-----------------------------------------------------------------------------+

  PURPOSE : checks an incoming immediate assignment message and forwards
            it to GRR if necessary.

*/

GLOBAL BOOL dat_check_gprs_imm_ass (T_MPH_UNITDATA_IND * unitdata,
                                     T_D_IMM_ASSIGN     * imm_assign,
                                     UBYTE index)
{
  GET_INSTANCE_DATA;
  BOOL ret = FALSE;
  int  send_indication = 0;

  TRACE_FUNCTION ("dat_check_gprs_imm_ass()");

  if (GET_STATE(STATE_GPRS) NEQ GPRS_PAM_BCCH)
    return FALSE; /* normal circuit switched handling */

  TRACE_EVENT_P7 ("fn=%u tma=%u ri=%x d_t=%u (tmaip=%u fn=%d ri=%d)",
      unitdata->fn, imm_assign->tma, index, imm_assign->d_t,
      rr_data->gprs_data.tma_in_progress, rr_data->gprs_data.fn,
      rr_data->gprs_data.req_ref_idx);

  if (imm_assign->tma EQ TMA_1)
  {
    /* Message indicates to be the first message of two in a
     * two-message assignment of a downlink TBF.
     */
    if (rr_data->gprs_data.tma_in_progress)
    {
      TRACE_ERROR ("more than one TMA after another!");
      TRACE_EVENT_P4 ("ri=%x?%x(old) fn=%u?%u(old)",
        index, rr_data->gprs_data.req_ref_idx,
        unitdata->fn, rr_data->gprs_data.fn);
    }

    rr_data->gprs_data.tma_in_progress = TRUE;

    /*
     * store message, framenumber and burst index
     */
    gprs_rrgrr_store_sdu(rr_data->gprs_data.rr_sdu, &unitdata->sdu);
    rr_data->gprs_data.fn = unitdata->fn;
    rr_data->gprs_data.req_ref_idx = index;
    ret = TRUE; /* further treatment by GRR */
  }
  else
  {
    if (rr_data->gprs_data.tma_in_progress)
    { /*
       * MS awaits the second message of two in a two-message assignment
       * of a downlink TBF.
       * The message must belong to the same channel request
       * and the time difference must be less than two multiframes.
       */
      BOOL in_time, same_reference_index;
      U32  delta_fn;

      if (rr_data->gprs_data.fn > unitdata->fn)
        delta_fn = unitdata->fn + HYPERFRAME - rr_data->gprs_data.fn;
      else
        delta_fn = unitdata->fn - rr_data->gprs_data.fn;

      in_time = (delta_fn/51) < 2;
      same_reference_index = rr_data->gprs_data.req_ref_idx EQ index;

      TRACE_EVENT_P6 ("TMA:delta_fn=%u %s time, %sequal ref idx(%x?%x): %s",
        delta_fn, in_time?"in":"out of", same_reference_index?"":"un",
        rr_data->gprs_data.req_ref_idx, index,
        (in_time AND same_reference_index)?"OK":"not ok");

      if (in_time AND same_reference_index)
      { /* This is the matching second message of a tma.
         * -> Send both immediate assignments
         */
        send_indication = 2;
        ret = TRUE; /* further treatment by GRR */
      }
      else
      {
        TRACE_ERROR ("MS awaits second message of a tma, this is not the right one!");

        if (!in_time)
        {
          /*
           * If the MS does not received the second IMMEDIATE ASSIGNMENT message
           * in a two-message assignment within two multiframe periods following
           * the first message, the MS shall discard the IA message received.
           */
          rr_data->gprs_data.tma_in_progress = FALSE;
          rr_data->gprs_data.fn = NOT_PRESENT_32BIT;
          rr_data->gprs_data.req_ref_idx = NOT_PRESENT_8BIT;
        }

        if(imm_assign->d_t EQ D_T_TBF)
        {
          /*
           * stop timer and set RR state
           * the handling of PDCH is done by GPRS
           */
          ret = TRUE; /* further treatment by GPRS */
        }
        else
        {
          /*
           * allocation on SDCCH for GPRS
           * normal RR establishment
           */
          ret = FALSE;
        }
      }
    }
    else
    { /*
       * MS don´t await the second message of two in a two-message assignment
       * of a downlink TBF and the message isn´t the first message of a tma.
       *
       * -> Send one indication to GRR
       */
      send_indication = 1;

      if(imm_assign->d_t EQ D_T_TBF)
      {
        /*
         * stop timer and set RR state
         * the handling of PDCH is done by GPRS
         */
        ret = TRUE; /* further treatment by GRR */
      }
      else
        /*
         * allocation on SDCCH for GPRS
         * normal RR establishment
         */
        ret = FALSE;
    }
  }

  if (send_indication)
  {
    if (send_indication EQ 2)
    {
      PALLOC_SDU ( rrgrr_ia_ind, RRGRR_IA_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE); /* T_RRGRR_IA_IND */
      /*
       * indicate first immediate assignment
       */
      gprs_rrgrr_fill_from_stored_sdu(&rrgrr_ia_ind->sdu, rr_data->gprs_data.rr_sdu);
      rrgrr_ia_ind->fn    = 0;      /* not valid */
      rrgrr_ia_ind->r_bit = 0;      /* not valid */
      PSENDX (GRR, rrgrr_ia_ind);
    }
    {
      PALLOC_SDU ( rrgrr_ia_ind2, RRGRR_IA_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE); /* T_RRGRR_IA_IND */
      /*
       * indicate second or sole immediate assignment
       */
      if (unitdata->sdu.o_buf >= 8)
      {
        unitdata->sdu.o_buf -= 8;
        unitdata->sdu.l_buf += 8;
      }
      else
      {
        TRACE_ERROR("dat_check_gprs_imm_ass(), not able to move offset o_buf");
      }
      gprs_rrgrr_fill_from_stored_sdu(&rrgrr_ia_ind2->sdu,
        &unitdata->sdu.buf[unitdata->sdu.o_buf/8]);
      rrgrr_ia_ind2->fn = unitdata->fn;
      rrgrr_ia_ind2->r_bit = get_r_bit ();
      PSENDX (GRR, rrgrr_ia_ind2);
    }
  }

  TRACE_EVENT_P1 ("dat_check_gprs_imm_ass() returns %u", ret);
  return ret;
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_check_immediate_assignment_ext |
+-----------------------------------------------------------------------------+

  PURPOSE : checks an incoming immediate assignment extended message
            and forwards it to GRR if necessary.

*/

GLOBAL void dat_check_imm_ass_ext (T_MPH_UNITDATA_IND * unitdata,
                                                 UBYTE index)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("grps_check_immediate_assignment_ext()");

  /*switch (rr_data->ms_data.establish_cause)
  {
    case ESTCS_GPRS_1P:
    case ESTCS_GPRS_SB:
    case ESTCS_GPRS_PAGING:*/
  switch (GET_STATE(STATE_GPRS))
  {
    case GPRS_PAM_BCCH:
      /*
       * Immediate Assignment Extended received on AGCH during connection
       * establishment
       */
      {
        PALLOC_SDU (rrgrr_iaext_ind, RRGRR_IAEXT_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE);
        gprs_rrgrr_store_sdu(rr_data->gprs_data.rr_sdu, &unitdata->sdu);
        gprs_rrgrr_fill_from_stored_sdu(&rrgrr_iaext_ind->sdu, rr_data->gprs_data.rr_sdu);

        rrgrr_iaext_ind->ia_index = index;
        rrgrr_iaext_ind->fn       = unitdata->fn;
        rrgrr_iaext_ind->r_bit    = get_r_bit();

        PSENDX (GRR, rrgrr_iaext_ind);
      }
      break;
    default:
      break;
  }
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : att_check_imm_ass_rej               |
+-----------------------------------------------------------------------------+

  PURPOSE : checks an incoming immediate assignment reject message and forwards
            it to GRR if necessary.

*/

GLOBAL UBYTE dat_check_imm_ass_rej (UBYTE wait_ind)
{
  GET_INSTANCE_DATA;
  UBYTE ret = FALSE;

  TRACE_FUNCTION ("att_check_imm_ass_rej()");

/*
  switch (rr_data->ms_data.establish_cause)
  {
    case ESTCS_GPRS_1P:
    case ESTCS_GPRS_SB:
    case ESTCS_GPRS_PAGING:
    */
  switch (GET_STATE(STATE_GPRS))
  {
    case GPRS_PAM_BCCH:
      /*
       * Immediate Assignment Reject received on AGCH during connection
       * establishment
       */
      {
        PALLOC( rrgrr_assignment_rej_ind, RRGRR_ASSIGNMENT_REJ_IND);

        rrgrr_assignment_rej_ind->wait_ind = wait_ind;
        rrgrr_assignment_rej_ind->r_bit    = get_r_bit ();

        PSENDX (GRR, rrgrr_assignment_rej_ind);
      }
      ret = TRUE;
      break;
      /* return TRUE; */
    default:
      break;
  }
  return ret;
  /* return FALSE; */
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_check_imm_assign_pch            |
+-----------------------------------------------------------------------------+

  PURPOSE : checks an incoming immediate assignment message in idle mode on
            PCH

*/

GLOBAL UBYTE dat_check_imm_assign_pch (T_MPH_UNITDATA_IND * unitdata,
                                       T_D_IMM_ASSIGN     * imm_assign)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_check_imm_assign_pch()");
  TRACE_EVENT("check dl ass");
  if(imm_assign->d_t EQ D_T_TBF AND
     imm_assign->dl  EQ DL_1)
  {
    PALLOC_SDU ( rrgrr_ia_ind, RRGRR_IA_DOWNLINK_IND, MAX_L2_FRAME_SIZE * BITS_PER_BYTE);
    switch(GET_STATE(STATE_GPRS))
    {
      case GPRS_NULL:
      case GPRS_ACTIVATED:
        /*
         * here it would be possible to store it if we are in
         * cell reselection
         * but i think this does not make sense:
         * -> this is not page (ia_dl is only cell wide),
         * -> we dont know if its actually for us.
         * -> first we have to do a Cell Update.
         */
      case GPRS_SUSPENDED_BCCH:
      case GPRS_SUSPENDED_PBCCH:
        TRACE_EVENT_P1("dl ass in state %d", GET_STATE(STATE_GPRS) );
        PFREE(rrgrr_ia_ind);
        return TRUE;
      case GPRS_PIM_BCCH:
      case GPRS_PAM_BCCH:
        /*
         * Immediate Assignment received in idle mode and it is a PDCH
         */
        TRACE_EVENT("dl ass");
        rr_data->start_cell_reselection = FALSE;
        TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);
        gprs_rrgrr_store_sdu(rr_data->gprs_data.rr_sdu, &unitdata->sdu);
        gprs_rrgrr_fill_from_stored_sdu(&rrgrr_ia_ind->sdu, rr_data->gprs_data.rr_sdu);
        rrgrr_ia_ind->fn    = unitdata->fn;
        rrgrr_ia_ind->r_bit = NOT_PRESENT_8BIT;

        PSENDX (GRR, rrgrr_ia_ind);
        return TRUE;
      default:
		PFREE(rrgrr_ia_ind);
        return FALSE;
    }
  }
  return FALSE;
}



/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_check_packet_paging_ind         |
+-----------------------------------------------------------------------------+

  PURPOSE : checks an incoming paging whether it is for GPRS.

*/

UBYTE dat_check_packet_paging_ind (T_MPH_PAGING_IND * pag_ind)
{
  GET_INSTANCE_DATA;
  UBYTE ret=2;
  TRACE_FUNCTION ("dat_check_packet_paging_ind ()");

  if (pag_ind->channel_needed EQ CN_PACKET)
  {
    switch(GET_STATE(STATE_GPRS))
    {
      case GPRS_PIM_BCCH:
        {
          /*
           * Inform GRR about packet paging
           */
          PALLOC ( rrgrr_packet_paging_ind, RRGRR_PACKET_PAGING_IND );
          switch (pag_ind->identity_type)
          {
            case ID_IMSI:
              rrgrr_packet_paging_ind->pg_type = RRGRR_IMSI;
              break;
            case ID_PTMSI:
              rrgrr_packet_paging_ind->pg_type = RRGRR_PTMSI;
              break;
            default:
              TRACE_EVENT("packet paging with TMSI");
              break;
          }
          PSENDX (GRR, rrgrr_packet_paging_ind);
        }
        ret = 2;
        break;
      case GPRS_NULL:
      case GPRS_ACTIVATED:
        /*strange cases, ignore it */
        break;
      case GPRS_SUSPENDED_BCCH:
      case GPRS_SUSPENDED_PBCCH:
        /*is is a packet paging but we are suspended so ignore it*/
        ret = 2;
        break;
      default:
        break;
    }
  }
  else
  {
    /* normal CS paging */
    switch(GET_STATE(STATE_GPRS))
    {
      case GPRS_SUSPENDED_BCCH:
      case GPRS_SUSPENDED_PBCCH:
      case GPRS_NULL:
      case GPRS_ACTIVATED:
        /* we are already suspended so just react to the paging */
        /* or GPRS is not activated */
        ret = 0;
        break;
      default:
        /* ask GRR */
        ret = 1;
        break;
    }
  }
  return ret;
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_start_sabm                     |
+-----------------------------------------------------------------------------+

  PURPOSE : starts connection establishment if a packet service is requested.

*/

BOOL dat_gprs_start_sabm(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("gprs_start_sabm()");

  switch (GET_STATE(STATE_GPRS))
  {
    /*
     * Mobile originated packet connection
     */
    case GPRS_PAM_BCCH:
      dat_build_rr_initialisation_request ();
      return TRUE;
    default:
      return FALSE;
  }
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_build_rr_initialisation_request|
+-----------------------------------------------------------------------------+

  PURPOSE : starts connection establishment if a mobile originated packet
            service is requested and it builds

*/

static void dat_build_rr_initialisation_request (void)
{
  GET_INSTANCE_DATA;
  MCAST (rr_init_req, D_RR_INIT_REQ);
  PALLOC_MSG (establish_req, DL_ESTABLISH_REQ, D_RR_INIT_REQ);

  TRACE_FUNCTION ("grps_build_rr_initialisation_request()");
  TRACE_EVENT ("POWER CLASS: grps_build_rr_initialisation_request()");

  dat_code_prr_channel (&establish_req->ch_type,
                        &establish_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  /*
   * fill message parameter
   */
  rr_init_req->msg_type               = D_RR_INIT_REQ;
  rr_init_req->ciph_key_num.key_seq   = rr_data->ms_data.cksn;
  memcpy(&rr_init_req->mob_class_2, &rr_data->ms_data.classmark2,
         sizeof(rr_init_req->mob_class_2));

  /*
   * set power classes
   */
  rr_init_req->mob_class_2.rf_pow_cap = att_get_power ();

  /*
   * set all other parameter
   */
  rr_init_req->chan_coding.mac_mode   = rr_data->gprs_data.mac_req;
  rr_init_req->chan_coding.cod_scheme = rr_data->gprs_data.cs_req;

  /*
   * store tlli independent from the used binary format
   */
  rr_init_req->ded_tlli.l_ded_tlli = 32;
  rr_init_req->ded_tlli.o_ded_tlli = 0;
  ccd_codeByte (rr_init_req->ded_tlli.b_ded_tlli, 0,  8, (UBYTE)(rr_data->gprs_data.tlli >> 24));
  ccd_codeByte (rr_init_req->ded_tlli.b_ded_tlli, 8,  8, (UBYTE)(rr_data->gprs_data.tlli >> 16));
  ccd_codeByte (rr_init_req->ded_tlli.b_ded_tlli, 16, 8, (UBYTE)(rr_data->gprs_data.tlli >> 8));
  ccd_codeByte (rr_init_req->ded_tlli.b_ded_tlli, 24, 8, (UBYTE)rr_data->gprs_data.tlli);

  rr_init_req->chan_req_desc.or_ty = rr_data->gprs_data.p_chan_req_des.mo_mt;

  if (rr_init_req->chan_req_desc.or_ty)
  {
    rr_init_req->chan_req_desc.v_crd_prio    = 1;
    rr_init_req->chan_req_desc.crd_prio      = rr_data->gprs_data.p_chan_req_des.prio;
    rr_init_req->chan_req_desc.v_rlc_mode    = 1;
    rr_init_req->chan_req_desc.rlc_mode      = rr_data->gprs_data.p_chan_req_des.rlc_mode_req;
    rr_init_req->chan_req_desc.v_llc_fr_type = 1;
    rr_init_req->chan_req_desc.llc_fr_type   = rr_data->gprs_data.p_chan_req_des.llc_type;
    rr_init_req->chan_req_desc.v_rbw         = 1;
    rr_init_req->chan_req_desc.rbw           = rr_data->gprs_data.p_chan_req_des.req_bwd;
    rr_init_req->chan_req_desc.v_rlc_c_oct   = 1;
    rr_init_req->chan_req_desc.rlc_c_oct     = rr_data->gprs_data.p_chan_req_des.rlc_octets;
  }

  rr_init_req->gprs_meas_res.c_val    = rr_data->gprs_data.gprs_meas_results.c_value;
  rr_init_req->gprs_meas_res.rxqual   = rr_data->gprs_data.gprs_meas_results.rxqual;
  rr_init_req->gprs_meas_res.sign_var = rr_data->gprs_data.gprs_meas_results.sign_var;

  for_dat_est_req_content  (establish_req);
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_stop_dcch_ind                  |
+-----------------------------------------------------------------------------+

  PURPOSE : inform GRR about failed layer 2 establishment.

*/
void dat_stop_dcch_ind (UBYTE stop_cause)
{
  GET_INSTANCE_DATA;
  PALLOC(stop, RRGRR_STOP_DCCH_IND);
  TRACE_FUNCTION ("gprs_stop_dcch_ind()");
  stop->stop_cause = stop_cause;
  /* we will do a CR anyway */
  SET_STATE(STATE_GPRS, GPRS_ACTIVATED);
  PSENDX(GRR, stop);
}



/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_rrgrr_gprs_data_req            |
+-----------------------------------------------------------------------------+

  PURPOSE : The primitive RRGRR_GPRS_DATA_REQ contains various parameters from
            GRR. Layer 1 is configured and some data are stored.
*/

void dat_rrgrr_gprs_data_req (T_RRGRR_GPRS_DATA_REQ *data_req)
{
  GET_INSTANCE_DATA;
  /*
   * store parameter
   */

  rr_data->gprs_data.tlli              = data_req->tlli;
  rr_data->gprs_data.p_chan_req_des    = data_req->p_chan_req_des;
  rr_data->gprs_data.gprs_meas_results = data_req->gprs_meas_results;
  rr_data->gprs_data.mac_req           = data_req->mac_req;
  rr_data->gprs_data.cs_req            = data_req->cs_req;
  rr_data->gprs_data.current_rai       = data_req->rai;

  if(rr_data->gprs_data.ptmsi  NEQ data_req->old_ptmsi OR
     rr_data->gprs_data.ptmsi2 NEQ data_req->new_ptmsi)
  {
    rr_data->gprs_data.ptmsi  = data_req->old_ptmsi;
    rr_data->gprs_data.ptmsi2 = data_req->new_ptmsi;
    att_mph_identity_req ();
  }

  PFREE (data_req);
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_data_ind                  |
+-----------------------------------------------------------------------------+

  PURPOSE : Send the primitive rrgrr_data_ind to GRR.
            This primitive indicates to the GRR that a
            CTRL Message on DCCH for RR was received.
  IN      : frame_number
  OUT     : rrgrr_data_ind

*/

void dat_rrgrr_data_ind (T_DL_DATA_IND* dl_data_ind)
{
  USHORT soff,doff;
  PALLOC_SDU ( rrgrr_data_ind, RRGRR_DATA_IND, dl_data_ind->sdu.l_buf );

  TRACE_EVENT ("dat_rrgrr_data_ind ()");

  /*
   * compute byte offsets (soff,doff) into message streams,
   * assume that o_buf is multiple of 8
   */

  doff = (USHORT)(rrgrr_data_ind->sdu.o_buf / BITS_PER_BYTE);
  soff = (USHORT)(   dl_data_ind->sdu.o_buf / BITS_PER_BYTE);

  memcpy ( &rrgrr_data_ind->sdu.buf[doff],
           &   dl_data_ind->sdu.buf[soff], BYTELEN ( dl_data_ind->sdu.l_buf ) );

  rrgrr_data_ind->fn = NOT_PRESENT_32BIT;

  PFREE ( dl_data_ind );
  PSENDX (GRR, rrgrr_data_ind);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_rrgrr_data_req                 |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_DATA_REQ received from GRR.
            This primitive indicates to the RR to send a block on DCCH.
  IN      :
  OUT     :

*/

void dat_rrgrr_data_req (T_RRGRR_DATA_REQ *rrgrr_data_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_data_req ()");

  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_DEDI_SDCCH:
      {
        PPASS (rrgrr_data_req, dl_data_req, DL_DATA_REQ);

        dat_code_prr_channel (&dl_data_req->ch_type,
                              &dl_data_req->sapi,
                              rr_data->sc_data.chan_desc.chan_type);
        dat_vsd_bit_set ((T_L3_SDU *)&dl_data_req->sdu, SET_ONLY);
        for_dat_l3_data_req (dl_data_req);
      }
      break;
    default:
      break;
  }

  PFREE (rrgrr_data_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rgrr_suspend_dcch_req           |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_SUSPEND_DCCH_REQ received from GRR.
            This primitive is used by GRR to order RR to stop the
            dedicated channel. It is only a trigger.
  IN      : RRGRR-SUSPEND-DCCH-REQ
  OUT     : DL-SUSPEND-REQ and MPH-DEDICATED-REQ

*/
void dat_rrgrr_suspend_dcch_req( T_RRGRR_SUSPEND_DCCH_REQ  *suspend_dcch_req)
{
  GET_INSTANCE_DATA;
  PALLOC (dedicated_req, MPH_DEDICATED_REQ);

  TRACE_EVENT ("dat_rrgrr_suspend_dcch_req ()");

  for_suspend_layer_2();

  memset (dedicated_req, 0, sizeof (T_MPH_DEDICATED_REQ));
  dedicated_req->mod = MODE_PDCH_ASSIGN;
  PSENDX (PL, dedicated_req);

  SET_STATE (STATE_DAT, DAT_PDCH_ASS);

  PFREE (suspend_dcch_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_suspend_dcch_cnf          |
+-----------------------------------------------------------------------------+

  PURPOSE : Create the primitive RRGRR_SUSPEND_DCCH_CNF and send it to GRR.
            As a response GRR will start the establishment of a TBF.
  IN      : nothing
  OUT     : RRGRR_SUSPEND_DCCH_CNF

*/
void dat_rrgrr_suspend_dcch_cnf( void )
{
  PALLOC (suspend_dcch_cnf, RRGRR_SUSPEND_DCCH_CNF);
  PSENDX (GRR, suspend_dcch_cnf);
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_reconnect_dcch_req        |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR-RECONNECT-DCCH-REQ received from GRR.
            This primitive is used by GRR during a PDCH Assignment procedure
            and RR Network Controlled Cell Change Order to order RR to
            continue with the dedicated channel.
  IN      : T-RRGRR-RECONNECT-DCCH-REQ
  OUT     : DL-DATA-REQ(u-assign-fail) or MPH_DEDICATED_FAIL_REQ

*/
void dat_rrgrr_reconnect_dcch_req (T_RRGRR_RECONNECT_DCCH_REQ  *reconnect_dcch_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_reconnect_dcch_req()");

  if ( rr_data->gprs_data.tbf_est EQ TBF_EST_PDCH OR
       rr_data->gprs_data.tbf_est EQ TBF_EST_CCO     )
  {
    UBYTE next_state;

    if ( rr_data->gprs_data.tbf_est EQ TBF_EST_PDCH )
      next_state = DAT_PDCH_ASS_3;
    else
      next_state = DAT_CCO_3;

    /*
     * The dedicated channel has been suspended at PL and DL.
     * However RR could not established the TBF.
     */

    rr_data->gprs_data.tbf_est = TBF_EST_NONE;
    dat_code_mph_old_chan_req();
    rr_data->gprs_data.reconn_cause = reconnect_dcch_req->reconn_cause;
    rr_data->gprs_data.cco_need_reconnect_cnf = TRUE;
    SET_STATE (STATE_DAT, next_state);
  }
  else /* Any other state assuming D_PDCH_ASS_CMD has been sent to GRR.
          GRR cannot evaluate the message. */
    {
    MCAST (u_assign_fail, U_ASSIGN_FAIL);
    PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_ASSIGN_FAIL);

    /*
     * set channel type and SAPI
     */
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);

    u_assign_fail->msg_type = U_ASSIGN_FAIL;
    u_assign_fail->rr_cause = reconnect_dcch_req->reconn_cause;
    for_dat_data_req (dl_data_req);

    dat_rrgrr_reconnect_dcch_cnf (RECONN_OK);
  }
  PFREE (reconnect_dcch_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_reconnect_dcch_cnf        |
+-----------------------------------------------------------------------------+

  PURPOSE : Create the primitive RRGRR_RECONNECT_DCCH_CNF and send it to GRR.
            This indicates the outcome of the Reconnection procedure to GRR.
  IN      : nothing
  OUT     : RRGRR-RECONNECT-DCCH-CNF

*/
void dat_rrgrr_reconnect_dcch_cnf( UBYTE reconn_state )
{
  PALLOC (reconnect_dcch_cnf, RRGRR_RECONNECT_DCCH_CNF);
  TRACE_EVENT ("dat_rrgrr_reconnect_dcch_cnf()");
  reconnect_dcch_cnf->reconn_state = reconn_state;
  PSENDX (GRR, reconnect_dcch_cnf);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rgrr_resumed_tbf_req            |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR-RESUMED-TBF-REQ received from GRR.
            This primitive is used by GRR to order RR to indicate the
            successful outcome of a TBF establishment at GRR.
            This is applicable during PDCH Assignment and RR Network Contolled
            Cell Change Order.
  IN      : RRGRR-RESUMED-TBF-REQ
  OUT     : MPH-IDLE-REQ and DL-RELEASE-REQ

*/
void dat_rrgrr_resumed_tbf_req( T_RRGRR_RESUMED_TBF_REQ *resumed_tbf_req )
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_resumed_tbf_req ()");

  if ( rr_data->gprs_data.tbf_est EQ TBF_EST_PDCH OR
       rr_data->gprs_data.tbf_est EQ TBF_EST_CCO     )
  {
    PALLOC (dl_release_req, DL_RELEASE_REQ);

    rr_data->gprs_data.tbf_est = TBF_EST_NONE;

    att_build_idle_req(SC_INDEX, MODE_PACKET_TRANSFER);

    /*
     * Note that also during a Cell Change Order the states DAT_PDCH* are used.
     */

    SET_STATE (STATE_GPRS, GPRS_PTM_BCCH);
    SET_STATE (STATE_DAT, DAT_PDCH_ASS_2);

    dat_code_prr_channel (&dl_release_req->ch_type,
                          &dl_release_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);

    dl_release_req->mode = DL_NORMAL_RELEASE;
    PSENDX (DL, dl_release_req);
  }
  PFREE (resumed_tbf_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_resumed_tbf_cnf           |
+-----------------------------------------------------------------------------+

  PURPOSE : Create a primitive RGRR_RESUMED_TBF_CNF and send it to GRR.
            This indicates the successful outcome of the release of the DCCH
            which was in use before the TBF was established.
  IN      : nothing
  OUT     : RGRR-RESUMED-TBF-CNF

*/
void dat_rrgrr_resumed_tbf_cnf( void )
{
  GET_INSTANCE_DATA;
  PALLOC (rrgrr_resumed_tbf_cnf, RRGRR_RESUMED_TBF_CNF);

  TRACE_EVENT ("dat_rrgrr_resumed_tbf_cnf ()");

  SET_STATE(STATE_GPRS, GPRS_PTM_BCCH);
  SET_STATE(STATE_DAT,  DAT_IDLE);
  SET_STATE(STATE_ATT,  ATT_IDLE);

  PSENDX (GRR, rrgrr_resumed_tbf_cnf);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_change_order              |
+-----------------------------------------------------------------------------+

  PURPOSE : Handle the message RR Network Controlled Cell Change Order.
  IN      : DL-DATA-IND
  OUT     : RRGRR-DATA-IND

*/
void dat_rrgrr_change_order (T_DL_DATA_IND *dl_data_ind, T_D_CHANGE_ORDER *d_change_order)
{
  GET_INSTANCE_DATA;
  for_check_cell_descr (&d_change_order->cell_desc);   /* check if the BCCH is ok */

  if ( rr_data->ms_data.error.cs EQ 0 )   /* '0' indicates successful message decoding */
  {
    T_cell_desc *cell_desc;
    
    for_suspend_layer_2();

    if ( rr_data->gprs_data.dl_data_ind NEQ NULL )
    {
      PFREE ( rr_data->gprs_data.dl_data_ind );
    }
    rr_data->gprs_data.dl_data_ind = dl_data_ind;

    dat_att_null();
    cs_set_all();

    cell_desc = &d_change_order->cell_desc;
    rr_data->gprs_data.bsic  = (UBYTE)(( cell_desc->ncc << 3 ) + cell_desc->bcc);
    rr_data->gprs_data.arfcn = (USHORT)(( cell_desc->bcch_arfcn_hi << 8 ) +
                               cell_desc->bcch_arfcn_lo);
    TIMERSTOP(T_DEDICATED_MODE);
    rr_data->mode_after_dedi = MODE_CELL_CHANGE_ORDER;
    att_stop_dedicated();
  }
  else
  {
    MCAST (handov_fail, U_HANDOV_FAIL);
    PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_HANDOV_FAIL);

    handov_fail->msg_type = U_HANDOV_FAIL;
    handov_fail->rr_cause = RRC_PROT_UNSPECIFIED;
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);
    for_dat_data_req (dl_data_req);
    PFREE ( dl_data_ind );
  }
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_check_packet_access             |
+-----------------------------------------------------------------------------+

  PURPOSE : change state from conn. establishment to dedicated
  IN      :
  OUT     :

*/

BOOL dat_check_packet_access(void)
{
  GET_INSTANCE_DATA;
  if(GET_STATE(STATE_GPRS) EQ GPRS_PAM_BCCH)
  {
    SET_STATE(STATE_GPRS, GPRS_DEDI_SDCCH);
    return TRUE;
  }
  else return FALSE;
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_ask_paging_ind                 |
+-----------------------------------------------------------------------------+

  PURPOSE : Ask GRR if paging should be processed. Answer will come in
            RRGRR_RR_EST_RSP primitive

*/

void dat_ask_paging_ind (T_MPH_PAGING_IND *pag_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_ask_paging_ind ()");

  rr_data->gprs_data.pag_dat.id_type   = pag_ind->identity_type;
  rr_data->gprs_data.pag_dat.chan_need = pag_ind->channel_needed;

  rr_data->start_cell_reselection = FALSE;
  TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);
  /* ask GRR if paging should be processed */
  dat_rrgrr_rr_est_ind ();
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_ask_paging_ind_pa_only          |
+-----------------------------------------------------------------------------+

  PURPOSE : Ask GRR if paging should be processed. Answer will come in
            RRGRR_RR_EST_RSP primitive. Only applicable in state GPRS_PA.

*/

void dat_ask_paging_ind_pa_only (T_MPH_PAGING_IND *pag_ind)
{
  GET_INSTANCE_DATA;
  if ( GET_STATE(STATE_GPRS) EQ GPRS_PAM_BCCH AND pag_ind->channel_needed NEQ CN_PACKET )
      dat_ask_paging_ind ( pag_ind );
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_rr_est_ind                |
+-----------------------------------------------------------------------------+

  PURPOSE : Send the primitive rrgrr_est_ind to GRR.
            This primitive indicates to the GRR that an RR connection
            establishment was received from the network via paging.
            It is only a trigger.
  IN      :
  OUT     : rrgrr_rr_est_ind

*/

static void dat_rrgrr_rr_est_ind (void)
{
  PALLOC ( rrgrr_est_ind, RRGRR_RR_EST_IND );
  TRACE_EVENT ("dat_rrgrr_est_ind ()");
  PSENDX (GRR, rrgrr_est_ind);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_rr_est_req                |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_RR_EST_REQ received from GRR.
            This primitive indicates to the RR a RR connection establishment
            was received on paging channel (PCCCH or PACCH). RR has to start
            a RR connection as if it has received a connection establishment
            on CCCH.
  IN      :
  OUT     :

*/

void dat_rrgrr_rr_est_req (T_RRGRR_RR_EST_REQ *est_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_rr_est_req ()");

  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PTM_BCCH:
    case GPRS_PAM_BCCH:
    case GPRS_PTM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PIM_PBCCH:
      rr_data->gprs_data.gprs_suspend = est_req->susp_req;
      rr_data->gprs_data.gprs_resump  = GPRS_RESUMPTION_ACK;

      if(est_req->non_gprs.v_non_gprs)
      {
        handle_non_gprs_param(&est_req->non_gprs);
      }

      switch(GET_STATE(STATE_GPRS))
      {
        case GPRS_PIM_BCCH:
        case GPRS_PAM_BCCH:
        case GPRS_PTM_BCCH:
          SET_STATE(STATE_GPRS, GPRS_SUSPENDED_BCCH);
          break;
        case GPRS_PIM_PBCCH:
        case GPRS_PAM_PBCCH:
        case GPRS_PTM_PBCCH:
          SET_STATE(STATE_GPRS, GPRS_SUSPENDED_PBCCH);
          break;
        default:
          break;
      }

      dat_begin_start_immediate_assign (est_req->ident_type,
                                        est_req->ch_needed);
      break;
    default:
      break;
  }
  PFREE (est_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : handle_non_gprs_param				  |
+-----------------------------------------------------------------------------+

  PURPOSE :  Processes RGRR_RR_EST_REQ  parameters   received from GRR.
  IN      :  Non GPRS parameter structure of the RRGRR_RR_EST_REQ primitive.
  OUT     :

*/

static void handle_non_gprs_param(T_non_gprs* non_gprs)
{
  GET_INSTANCE_DATA;
  rr_data->nc_data[SC_INDEX].control_descr.att            = non_gprs->att;
  rr_data->nc_data[SC_INDEX].control_descr.bs_ag_blks_res = non_gprs->bs_ag_blks_res;
  rr_data->nc_data[SC_INDEX].control_descr.ccch_conf      = non_gprs->ccch_conf;
  rr_data->nc_data[SC_INDEX].control_descr.bs_pa_mfrms    = non_gprs->bs_pa_mfrms;
  rr_data->nc_data[SC_INDEX].rach.max_retrans             = non_gprs->max_retrans;
  rr_data->nc_data[SC_INDEX].rach.tx_integer              = non_gprs->tx_integer;
  rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch = non_gprs->gprs_ms_txpwr_max_cch;
  rr_data->nc_data[SC_INDEX].select_para.neci             = non_gprs->neci;
  rr_data->sc_data.cd.cell_options.pow_ctrl               = non_gprs->pwrc;

  if(non_gprs->dtx EQ 2)
    rr_data->sc_data.cd.dtx = DTX_NOT_USED;
  else
    rr_data->sc_data.cd.dtx = DTX_USED;

  rr_data->sc_data.cd.dtx_half = rr_data->sc_data.cd.dtx_full =
    rr_data->sc_data.cd.dtx;

  rr_data->sc_data.cd.cell_options.rlt                    = non_gprs->radio_link_timeout;

  if(non_gprs->ec)
    rr_data->nc_data[SC_INDEX].rach.ac &= 0x0400;
  if(non_gprs->v_T3212)
    rr_data->nc_data[SC_INDEX].control_descr.t3212        = non_gprs->T3212;
  
#ifdef REL99
  /*Copy Early Classmark Sending Control flag received in PSI2 by GRR*/
  rr_data->nc_data[SC_INDEX].c2_par.ecsc = non_gprs->ecsc;
#endif
}
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_rr_est_rsp                |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_RR_EST_RSP received from GRR.
            This primitive indicates to the RR whether an establishment of
            RR connection is allowed or not. This primitive is an answer
            to the RRGRR_RR_EST_IND primitive.
  IN      :
  OUT     :

*/

void dat_rrgrr_rr_est_rsp (T_RRGRR_RR_EST_RSP *est_rsp)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_rr_est_rsp ()");

  rr_data->start_cell_reselection = TRUE;
  TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);

  /* establishment allowed */
  if (est_rsp->rr_est)
  {
    rr_data->gprs_data.gprs_suspend = est_rsp->susp_req;
    rr_data->gprs_data.gprs_resump  = GPRS_RESUMPTION_ACK;
    switch(GET_STATE(STATE_GPRS))
    {
      case GPRS_PIM_BCCH:
      case GPRS_PAM_BCCH:
      case GPRS_PTM_BCCH:
        SET_STATE(STATE_GPRS, GPRS_SUSPENDED_BCCH);
        break;
      case GPRS_PIM_PBCCH:
      case GPRS_PAM_PBCCH:
      case GPRS_PTM_PBCCH:
        SET_STATE(STATE_GPRS, GPRS_SUSPENDED_PBCCH);
        break;
      default:
        break;
    }
    dat_begin_start_immediate_assign (rr_data->gprs_data.pag_dat.id_type,
                                      rr_data->gprs_data.pag_dat.chan_need);
  }

  rr_data->gprs_data.pag_dat.id_type   = 0;
  rr_data->gprs_data.pag_dat.chan_need = 0;

  PFREE (est_rsp);
}


/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_gprs_suspend_req				  |
+-----------------------------------------------------------------------------+

  PURPOSE : Builds  GPRS suspension request message.
  IN      :
  OUT     :

*/

void dat_gprs_suspend_req (void)
{
  GET_INSTANCE_DATA;
  if(rr_data->gprs_data.gprs_suspend)
  {
    /*
     * building of the GPRS Suspension Request message
     */
    MCAST (susp_req, U_GPRS_SUSP_REQ);
    PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_GPRS_SUSP_REQ);

    /*
     * set channel type and SAPI for layer 2
     */
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);
    susp_req->msg_type = U_GPRS_SUSP_REQ;

    /*
     * store tlli independent from the used binary format
     */
    susp_req->ded_tlli.l_ded_tlli = 32;
    susp_req->ded_tlli.o_ded_tlli = 0;
    ccd_codeByte (susp_req->ded_tlli.b_ded_tlli, 0,  8,
                  (UBYTE)(rr_data->gprs_data.tlli >> 24));
    ccd_codeByte (susp_req->ded_tlli.b_ded_tlli, 8,  8,
                  (UBYTE)(rr_data->gprs_data.tlli >> 16));
    ccd_codeByte (susp_req->ded_tlli.b_ded_tlli, 16, 8,
                  (UBYTE)(rr_data->gprs_data.tlli >> 8));
    ccd_codeByte (susp_req->ded_tlli.b_ded_tlli, 24, 8,
                  (UBYTE) rr_data->gprs_data.tlli);


    switch(rr_data->ms_data.establish_cause)
    {
      case ESTCS_SERV_REQ_BY_MM:
        susp_req->susp_cause = SUSP_C_LU;
        break;
      case ESTCS_EMRG_CAL:
      case ESTCS_CAL_REEST:
      case ESTCS_MOB_ORIG_SPCH_CAL_BY_CC:
      case ESTCS_MOB_ORIG_DATA_CAL_BY_CC:
      case ESTCS_MOB_ORIG_DATA_CAL_BY_CC_HR_SUFF:
      case ESTCS_PAGING:
        susp_req->susp_cause = SUSP_C_CALL;
        break;
      case ESTCS_MOB_ORIG_CAL_BY_SS_SMS:
        susp_req->susp_cause = SUSP_C_SMS; /* SUSP_C_SS */
        break;
      default:
        break;
    }

    memcpy(susp_req->rout_area_id.mcc,rr_data->gprs_data.current_rai.plmn.mcc, SIZE_MCC);
    memcpy(susp_req->rout_area_id.mnc,rr_data->gprs_data.current_rai.plmn.mnc, SIZE_MNC);
    if (susp_req->rout_area_id.mnc[2] EQ 0x0f)
      susp_req->rout_area_id.c_mnc = 2;
    else
      susp_req->rout_area_id.c_mnc = SIZE_MNC;

    susp_req->rout_area_id.rac    = rr_data->gprs_data.current_rai.rac;
    susp_req->rout_area_id.lac    = rr_data->gprs_data.current_rai.lac;

    rr_data->gprs_data.gprs_resump  = GPRS_RESUMPTION_NOT_ACK;
    /*
     * send to layer 2
     */
    for_dat_data_req (dl_data_req);
  }

}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : gprs_rrgrr_stop_task                |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_STOP_TASK_REQ received from GRR.
            This primitive indicates to the RR that a specific task on RR side
            should be stopped.
  IN      :
  OUT     :

*/

void gprs_rrgrr_stop_task( T_RRGRR_STOP_TASK_REQ *stop_task)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("gprs_rrgrr_stop_task");

  switch(stop_task->ctrl_task)
  {
    case RR_TASK_1:
      {
        switch(GET_STATE(STATE_GPRS))
        {
          case GPRS_PAM_BCCH:
            rr_data->start_cell_reselection = TRUE;
            TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);
            if(stop_task->task.v_stop_ccch NEQ INVALID_MSG)
            {
              att_build_idle_req(SC_INDEX, MODE_PACKET_TRANSFER);
              TIMERSTOP(T3126);
              SET_STATE(STATE_GPRS, GPRS_PTM_BCCH);
              SET_STATE(STATE_DAT,  DAT_IDLE);
              SET_STATE(STATE_ATT,  ATT_IDLE);
              {
               PALLOC(stop_cnf, RRGRR_STOP_TASK_CNF);
               PSENDX(GRR, stop_cnf);
              }
            }
            break;
          case GPRS_PIM_BCCH:
            if(stop_task->task.v_stop_ccch NEQ INVALID_MSG)
            {
              /*
                * XXX we still have a problem if the search terminates
                * and we want to select to the HPLMN cell just in the moment
                * after we have sent the IA_DOWNLINK_IND but before receiving
                * the STOP_TASK_REQ. But we should not stop the search
                * before, because maybe the IA_DOWNLINK was not actually
                * addressing us.
                */

              if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
              {
                /*inform MM and stop everything*/
                /*
                 * send Please Retry to the MMI
                 */
                rr_data->sc_data.found_entries = 0;
                att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
              }
              att_build_idle_req(SC_INDEX, MODE_PACKET_TRANSFER);
              SET_STATE(STATE_GPRS, GPRS_PTM_BCCH);
              SET_STATE(STATE_DAT,  DAT_IDLE);
              SET_STATE(STATE_ATT,  ATT_IDLE);
              {
                PALLOC(stop_cnf, RRGRR_STOP_TASK_CNF);
                PSENDX(GRR, stop_cnf);
              }
            }
            /*
             * If a search is active in idle this is set to false
             * it is also set to false on sending the IA_DL to GRR
             * If the IA_DL is for us we abort the search and enter TBF
             * that means that the start_cr flag is always set to true
             * If the IA_DL is not for us then the start_cr flag should only be
             * reset if no search is ongoing.
             */
            if (rr_data->ms_data.req_mm_service NEQ FUNC_NET_SRCH_BY_MMI)
           {
              rr_data->start_cell_reselection = TRUE;
              TRACE_EVENT_P1("start_cell_reselection %d", rr_data->start_cell_reselection);
            }
            break;
          default:
            break;
        }
      }
      break;
    case LEAVE_PIM_PBCCH:
      {
        TRACE_EVENT("LEAVE PIM");
        /* stop CCCH reading if running */
        /* SET_STATE(STATE_DAT, DAT_NULL);  avoid reacting to crossing prim */
        /* stop CBCH reading if running */
        /* stop active ncell procedures */
        if (rr_data->ms_data.req_mm_service EQ FUNC_NET_SRCH_BY_MMI)
        {
          /*inform MM and stop everything*/
          /*
           * send Please Retry to the MMI
           */
          rr_data->sc_data.found_entries = 0;
          att_code_rr_abort_ind (RRCS_ABORT_CEL_SEL_FAIL);
        }
        {
          PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
          mph_mon_ctrl_req->action = LEAVING_PIM_PBCCH;
          PSENDX (PL, mph_mon_ctrl_req);
        }
        {
          PALLOC(stop_cnf, RRGRR_STOP_TASK_CNF);
          PSENDX(GRR, stop_cnf);
        }
        /* XXX stop PLMN scan ?         */
      }
    break;
    case LEAVE_PAM_PBCCH:
      {
        PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
        TRACE_EVENT("LEAVE PAM");
        mph_mon_ctrl_req->action = LEAVING_PAM_PBCCH;
        PSENDX (PL, mph_mon_ctrl_req);
        {
          PALLOC(stop_cnf, RRGRR_STOP_TASK_CNF);
          PSENDX(GRR, stop_cnf);
        }
      }
    break;
    case LEAVE_PTM_PBCCH:

    {
      PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
      TRACE_EVENT("LEAVE PTM");
      mph_mon_ctrl_req->action = LEAVING_PTM_PBCCH;
      PSENDX (PL, mph_mon_ctrl_req);

      {
        PALLOC(stop_cnf, RRGRR_STOP_TASK_CNF);
        PSENDX(GRR, stop_cnf);
      }
    }
    break;
    default:
      break;

  }
  PFREE(stop_task);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_rrgrr_activate_req              |
+-----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RRGRR_ACTIVATE_REQ received from GRR.
            This primitive activates RR. RR acts as if the MS is only
            GSM service mobile. Monitor CCCH and BCCH.
  IN      :
  OUT     :

*/

void dat_rrgrr_activate_req (T_RRGRR_ACTIVATE_REQ *act_req)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("dat_rrgrr_activate_req ()");

  rr_data->gprs_data.gprs_suspend = act_req->susp_req;
  rr_data->gprs_data.rac          = act_req->rac;
  rr_data->gprs_data.gprs_resump  = GPRS_RESUMPTION_ACK;

  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:

       /* we need the parameters from GRR to make
        * a MO call
        */
      if(act_req->non_gprs.v_non_gprs)
      {
        handle_non_gprs_param(&act_req->non_gprs);
      }
      break;
  case GPRS_PIM_BCCH:
    /* nothing to do, just wait for MM to start with RR_ESTABLISH_REQ */
    /*SET_STATE(STATE_GPRS, GPRS_SUSPENDED_BCCH); do this on
      RR_ESTABLISH_REQ */
    break;
  case GPRS_PAM_BCCH:
  case GPRS_PTM_BCCH:
    /*
     * abort procedures we are doing for GPRS
     * and go back to idle mode to wait for the
     * RR_ESTABLISH_REQ
     */
    rr_data->gprs_data.page_mode = PAG_MODE_DEFAULT;
#ifdef REL99
#else
    att_return_to_idle();
#endif
    SET_STATE(STATE_GPRS, GPRS_PIM_BCCH);
#ifdef REL99
    att_return_to_idle();
#endif
    break;
  default:
    break;
  }

  PFREE (act_req);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_set_gprs_resump				  |
+-----------------------------------------------------------------------------+

  PURPOSE : Sets stored GPRS resumption value to the RR_RELEASE_IND primitive
			to send.
  IN      : Pointer to RR_RELEASE_IND primitive.
  OUT     :

*/

void dat_set_gprs_resump (T_RR_RELEASE_IND* rr_release_ind)
{
  GET_INSTANCE_DATA;
  rr_release_ind->gprs_resumption = rr_data->gprs_data.gprs_resump;
}



/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_gprs_set_suspended              |
+-----------------------------------------------------------------------------+

  PURPOSE : Sets state to GPRS_SUSPENDED_PBCCH or GPRS_SUSPENDED_BCCH
  IN      :
  OUT     :

*/


void dat_gprs_set_suspended(void)
{
  GET_INSTANCE_DATA;
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PIM_PBCCH:
    case GPRS_PAM_PBCCH:
    case GPRS_PTM_PBCCH:
      SET_STATE(STATE_GPRS, GPRS_SUSPENDED_PBCCH);
      break;
    case GPRS_PIM_BCCH:
    case GPRS_PAM_BCCH:
    case GPRS_PTM_BCCH:
      SET_STATE(STATE_GPRS, GPRS_SUSPENDED_BCCH);
      break;
    default:
      break;
  }
}

#ifdef REL99
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (6147)     MODULE  : RR_GPRS                             |
| STATE   : code                ROUTINE : dat_gprs_set_suspended              |
+-----------------------------------------------------------------------------+

  PURPOSE : Sets state to GPRS_SUSPENDED_PBCCH or GPRS_SUSPENDED_BCCH
  IN      :
  OUT     :

*/
BOOL dat_gprs_cell_in_ptm(void)
{
  GET_INSTANCE_DATA;
  switch(GET_STATE(STATE_GPRS))
  {
    case GPRS_PTM_PBCCH:
    case GPRS_PTM_BCCH:
      return TRUE;
    default:
      return FALSE;  
  }
}
#endif

/*===========================================================================*/
/*
 *                                  L O C A L S
 */
/*===========================================================================*/
/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-GPRS (??6147)     MODULE  : RR_GPRS                           |
| STATE   : code                ROUTINE : get_r_bit                           |
+-----------------------------------------------------------------------------+

  PURPOSE : R-bit for the IA-prims
  IN      : access counter
  OUT     : r_bit

*/

static UBYTE get_r_bit (void)
{
  GET_INSTANCE_DATA;
  UBYTE r_bit;

  switch (rr_data->ms_data.access_counter)
  {
    case 0:
      r_bit = NOT_PRESENT_8BIT;
      break;
    case 1:
      r_bit = CHAN_REQ_SENT_ONCE;
      break;
    default:
      r_bit = CHAN_REQ_SENT_MORE;
      break;
  }
  return r_bit;
}

#endif /* GPRS */
#endif /* RR_DATG_C */

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

#ifndef RR_DATS_C
#define RR_DATS_C

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
#include "em.h"
#include "rr_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

#define TDMA_FRAMES_PER_HYPERFRAME  2715648
#define QUARTER_BITS_PER_FRAME 5000

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

LOCAL BOOL dat_for_handover_mob_alloc(UBYTE  *mob_alloc,
                                      T_LIST  *hop_list_handover, 
                                      T_VOID_STRUCT  *mob_alloc_handover,
                                      T_DL_DATA_IND    *dl_data_ind);

LOCAL void dat_cr_data_multirate_conf(U8 v_multirate_conf, T_multirate_conf  *multirate_conf);

LOCAL void dat_dedicated_req_ch_type2(T_ch_type2 *ch_type2,  T_chan_desc_before  *chan_desc_before,
                                      T_LIST    *hop_list_before);
/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_att_cell_selected      |
+--------------------------------------------------------------------+

  PURPOSE : Attachment process indicates that it has camped on a cell.
            Data transfer process changes the state and connections
            are possible.

*/

GLOBAL void dat_att_cell_selected (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_att_cell_selected()");

  SET_STATE (STATE_DAT, DAT_IDLE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_att_null               |
+--------------------------------------------------------------------+

  PURPOSE : Attachment process indicates loss of coverage and connections
            are not longer possible.

*/

GLOBAL void dat_att_null (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("dat_att_null()");

  SET_STATE (STATE_DAT, DAT_NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_assign_cmd         |
+--------------------------------------------------------------------+

  PURPOSE : The function handles the reception of a channel
            assignment message.

*/

GLOBAL void dat_for_assign_cmd (T_DL_DATA_IND  *dl_data_ind,
                                T_D_ASSIGN_CMD *assign_cmd,
                                T_LIST         *hop_list_after,
                                T_LIST         *hop_list_before,
                                T_LIST         *cell_chan_desc)
{
  GET_INSTANCE_DATA;
  UBYTE mob_alloc[65];

  PALLOC (dedicated_req, MPH_DEDICATED_REQ);

  TRACE_FUNCTION ("dat_for_assign_cmd()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_DEDICATED:
      if (rr_data->ms_data.error.cs)
      {
        TRACE_EVENT_P1 ("RRC cause = %02x", rr_data->ms_data.error.cs);
      }
      /* 
       * dynamic configuration command : IHO
       * Lock the DUT to the cell it is already camping -
       * Ignore the Channel Assignment command message and send an
       * Assignment failure message to the network.
       */

      if(rr_data->dyn_config.iho AND (rr_data->sc_data.ch_mode EQ CM_AMR))
      {
        TRACE_EVENT("D_ASSIGN_CMD : IHO");
        for_set_content_error (RRC_CHANNEL_MODE);
      }
       
      switch (rr_data->ms_data.error.cs)
      {
        /* case RRC_INVALID_MAN_INFO: this value is currently never set */
        case RRC_COND_IE_ERROR: /* 0x64 */
        {
          /*
           * If an mandatory info element error or a
           * conditional info element error has been detected,
           * a RR STATUS message is returned on the existing
           * connection before l2 is suspended
           */
          dat_send_rr_status_msg(rr_data->ms_data.error.cs);
          PFREE (dedicated_req);
          PFREE (dl_data_ind);
          break ;
        }

        case RRC_INCORRECT_MSG: /* 0x5f */
        {
          /*
           * If a structurally correct message has been detected,
           * containing erroneous data, an Assignment Failure message
           * is sent back.
           */

          /*
           * If the ASSIGNMENT COMMAND is erroneous, then the
           * ASSIGNMENT FAILURE command is sent via a priority
           * DL_RECONNECT_REQ. This ensures DL will halt processing
           * anything in its buffer until it has sent this message
           * onto the nw.
           *
           */
          for_suspend_layer_2 ();
          dat_send_assign_fail_msg(rr_data->ms_data.error.val);

          RR_EM_SET_ASSIGN_FAIL_CAUSE(rr_data->ms_data.error.val);

          PFREE (dedicated_req);
          PFREE (dl_data_ind);
          break;
        }

        default:
          /*
           * the initial check was successful and the
           * message is processed.
           *
           */
          for_suspend_layer_2 ();

          /*
           * use data of the old cell if no new data
           * are inserted in the message
           */
          rr_data->cr_data.ch_mode   = rr_data->sc_data.ch_mode;
          rr_data->cr_data.ciph_on   = rr_data->sc_data.ciph_on;
          rr_data->cr_data.algo      = rr_data->sc_data.algo;
          rr_data->cr_data.cd.v_cell_chan_desc =
                   rr_data->sc_data.cd.v_cell_chan_desc;
          memcpy (&rr_data->cr_data.cd.cell_chan_desc,
                  &rr_data->sc_data.cd.cell_chan_desc,
                  sizeof (T_LIST));

          /*
           * if AMR is supported set the default values
           * to the current serving cell values.
           */
          if(rr_data->sc_data.ch_mode EQ CM_AMR)
          {
            memcpy(&rr_data->cr_data.amr_conf,
                   &rr_data->sc_data.amr_conf,
                   sizeof (T_multirate_conf));
          }
          else {
            /*
            *  AMR is not supported, therefore set some dummy values. This is necessary because
            *  the later Layer1 configuration must include an AMR configuration!!
            */
            memset(&rr_data->cr_data.amr_conf, 0, sizeof (T_multirate_conf));
          }

          if (assign_cmd->v_cell_chan_desc)
          {
            /*
             * If the message contains a cell channel description
             * use the new one.
             */
            memcpy (&rr_data->cr_data.cd.cell_chan_desc,
                    cell_chan_desc,
                    sizeof (T_LIST));
            rr_data->cr_data.cd.v_cell_chan_desc = WITH_CHANGED_CONTENT;
          }

          if (assign_cmd->v_chan_mode)
            /*
             * if the message contains a channel mode, use the new one.
             */
            rr_data->cr_data.ch_mode = assign_cmd->chan_mode;

          /*
          *  If AMR is signalled check if new multi-rate speech codec is part of the assignment cmd
          *  otherwise use default values set earlier.
          */

/* Implements RR Clone findings #9 */
              dat_cr_data_multirate_conf(assign_cmd->v_multirate_conf,&assign_cmd->multirate_conf);
         
         
          if (assign_cmd->v_ciph_mode_set)
          {
            /*
             * If ciphering is defined in the message, handle it.
             */
            rr_data->cr_data.ciph_on = assign_cmd->ciph_mode_set.sc;
            rr_data->cr_data.algo    = assign_cmd->ciph_mode_set.algo_ident;

            if (rr_data->cr_data.ciph_on EQ CIPH_ON AND
                rr_data->sc_data.ciph_received EQ FALSE)
            {
              /*
               * if ciphering is not active, but set in the message
               * this is a failure and the configuration is aborted.
               * Instead the reconnection on the old channel is started.
               */
              dat_send_assign_fail_msg(RRC_PROT_UNSPECIFIED);

              RR_EM_SET_ASSIGN_FAIL_CAUSE(RRC_PROT_UNSPECIFIED);

              PFREE (dedicated_req);
              PFREE (dl_data_ind);

              return;
            }
          }

          if (assign_cmd->chan_desc.hop EQ 1 AND
              assign_cmd->v_mob_alloc_after)
          {
            if (rr_data->cr_data.cd.v_cell_chan_desc EQ NO_CONTENT)
            {
              /*
              * If the new channel needs hopping, but there is no
              * cell channel description available, the configuration
              * is aborted due to a conditional error.
              * Instead the reconnection on the old channel is started.
              */
              dat_send_assign_fail_msg(RRC_NO_CELL_ALLOC);

              RR_EM_SET_ASSIGN_FAIL_CAUSE(RRC_NO_CELL_ALLOC);

              PFREE (dedicated_req);
              PFREE (dl_data_ind);
              return;
            }

            /*
             * if the message contains a mobile allocation,
             * build a list of 1-bits from the bitmap.
             */
            att_bits_to_byte (mob_alloc,
                              assign_cmd->mob_alloc_after.c_mac,
                              assign_cmd->mob_alloc_after.mac);

            /*
             * create a hopping list from mobile allocation and cell channel
             * description
             */
            if(!srv_create_chan_mob_alloc (&rr_data->cr_data.cd.cell_chan_desc,
                                           hop_list_after,
                                           mob_alloc))
            {
              dat_send_assign_fail_msg(RRC_FREQ_NOT_IMPL);

              RR_EM_SET_ASSIGN_FAIL_CAUSE(RRC_FREQ_NOT_IMPL);

              PFREE (dedicated_req);
              PFREE (dl_data_ind);
              return;
            }
          }

          /*
           * clean primitive to layer 1
           */
          memset (dedicated_req, 0, sizeof (T_MPH_DEDICATED_REQ));

          dedicated_req->mod         = MODE_CHAN_ASSIGN;

          rr_data->cr_data.chan_desc = assign_cmd->chan_desc;

          /*
           * Set Channel Type
           */
          dedicated_req->ch_type.ch    = assign_cmd->chan_desc.chan_type;
          dedicated_req->ch_type.tn    = assign_cmd->chan_desc.tn;
          dedicated_req->ch_type.tsc   = assign_cmd->chan_desc.tsc;
          dedicated_req->ch_type.h     = assign_cmd->chan_desc.hop;
          if (assign_cmd->chan_desc.hop EQ H_NO)
          {
            dedicated_req->ch_type.arfcn = assign_cmd->chan_desc.arfcn;
          }
          else
          {
            dedicated_req->ch_type.maio  = assign_cmd->chan_desc.maio;
            dedicated_req->ch_type.hsn   = assign_cmd->chan_desc.hsn;

            /* CSI-LLD section:4.1.1.11
             * This function Updates the black list with the MA list received
             * inthe assignment command  
             */   
            cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,hop_list_after);

            srv_create_list (hop_list_after, dedicated_req->ch_type.ma,
                             MAX_MA_CHANNELS, TRUE, 0);
          }

          /*
           * set initial power
           */
          dedicated_req->tr_para.power = assign_cmd->pow_cmd.pow;

          /*
           * set starting time if available.
           */
          if (assign_cmd->v_start_time)
          {
            dedicated_req->start.v_start = TRUE;
            dedicated_req->start.t1      = assign_cmd->start_time.t1;
            dedicated_req->start.t2      = assign_cmd->start_time.t2;
            dedicated_req->start.t3      = assign_cmd->start_time.t3;
          }

          /*
           * Setting of before starting time elements !
           */
          if (assign_cmd->v_chan_desc_before EQ FALSE)
            dedicated_req->ch_type2.ch = NOT_PRESENT_8BIT;
          else
          {
            if (assign_cmd->v_mob_alloc_before)
            {
              att_bits_to_byte (mob_alloc,
                                assign_cmd->mob_alloc_before.c_mac,
                                assign_cmd->mob_alloc_before.mac);
              if(!srv_create_chan_mob_alloc (&rr_data->cr_data.cd.cell_chan_desc,
                                             hop_list_before,
                                             mob_alloc))
              {
                dat_send_assign_fail_msg(RRC_FREQ_NOT_IMPL);

                RR_EM_SET_ASSIGN_FAIL_CAUSE( RRC_FREQ_NOT_IMPL);

                PFREE (dedicated_req);
                PFREE (dl_data_ind);
                return;
              }
            }
        
            dat_dedicated_req_ch_type2(&dedicated_req->ch_type2, &assign_cmd->chan_desc_before,
                                      hop_list_before );

          }


          /*
           * set dtx depending on the channel type (halfrate or  fullrate)
           */

          if (dedicated_req->ch_type.ch EQ 2 OR
              dedicated_req->ch_type.ch EQ 3)
            dedicated_req->tr_para.dtx =
              rr_data->sc_data.cd.dtx = rr_data->sc_data.cd.dtx_half;
          else
            dedicated_req->tr_para.dtx =
              rr_data->sc_data.cd.dtx = rr_data->sc_data.cd.dtx_full;

          dedicated_req->arfcn        = rr_data->nc_data[SC_INDEX].arfcn;
          dedicated_req->tr_para.rlt  = rr_data->sc_data.cd.cell_options.rlt;
          dedicated_req->tr_para.pwrc = rr_data->sc_data.cd.cell_options.pow_ctrl;
          dedicated_req->tr_para.mode = rr_data->cr_data.ch_mode;

            /*
             * Set multi-rate speech codec
             */
            dedicated_req->amr_conf.nscb     = rr_data->cr_data.amr_conf.nscb;
            dedicated_req->amr_conf.icmi     = rr_data->cr_data.amr_conf.icmi;
            dedicated_req->amr_conf.st_mode  = rr_data->cr_data.amr_conf.st_mode;
            dedicated_req->amr_conf.acs      = rr_data->cr_data.amr_conf.set_amr;

            /*
             *  valid flag for the threshold and hystersis values. amr_conf.c_cod_prop
             *  defines the number of threshold and hystersis values.
             */
            dedicated_req->amr_conf.v_cod_prop  = rr_data->cr_data.amr_conf.v_cod_prop;

            if(dedicated_req->amr_conf.v_cod_prop)
            {
              int i;
              dedicated_req->amr_conf.c_cod_prop = rr_data->cr_data.amr_conf.c_cod_prop;
              for(i=0; i<dedicated_req->amr_conf.c_cod_prop; i++)
                memcpy(&dedicated_req->amr_conf.cod_prop[i],
                       &rr_data->cr_data.amr_conf.cod_prop[i],
                       sizeof(T_cod_prop));
            }

          if (rr_data->cr_data.ciph_on)
          {
            /*
             * set cipher parameter if available.
             */
            dedicated_req->ciph.stat     = rr_data->cr_data.ciph_on;
            dedicated_req->ciph.algo     = rr_data->cr_data.algo;
            memcpy (dedicated_req->ciph.kc, rr_data->ms_data.kc, KC_STRING_SIZE);
          }

          RR_EM_GET_HOPPING_CHANNEL (dedicated_req->ch_type.ma, dedicated_req->ch_type2.ma, 
              dedicated_req->start.v_start,dedicated_req->ch_type2.maio);

          EM_ASSIGNMENT_RECEIVED;

          /*
           * configure layer 1
           */
          SET_STATE (STATE_DAT, DAT_CHAN_ASS);
          PSENDX (PL, dedicated_req);
          PFREE (dl_data_ind);
          return;
      }
      break;

    default:
      PFREE (dedicated_req);
      PFREE (dl_data_ind);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_chan_mod           |
+--------------------------------------------------------------------+

  PURPOSE : The function handles a received channel mode modify message.

*/

GLOBAL void dat_for_chan_mod (T_DL_DATA_IND *dl_data_ind,
                              T_D_CHAN_MOD *chan_mod)
{
  GET_INSTANCE_DATA;
  MCAST (chan_mod_ack, U_CHAN_MOD_ACK);

  TRACE_FUNCTION ("dat_for_chan_mod()");

  if (rr_data->ms_data.error.cs EQ 0)
  {
    /*
     * the check in the formatter indicates no problems
     * store new channel mode.
     */
    rr_data->sc_data.ch_mode = chan_mod->chan_mode;

    /*
     * the channel mode modify message contains a multi-rate configuration IEI
     */
    if( chan_mod->v_multirate_conf AND (chan_mod->chan_mode EQ CM_AMR) )
    {
      int i;
      rr_data->sc_data.amr_conf.mr_vers = chan_mod->multirate_conf.mr_vers;
      rr_data->sc_data.amr_conf.nscb    = chan_mod->multirate_conf.nscb;
      rr_data->sc_data.amr_conf.icmi    = chan_mod->multirate_conf.icmi;
      rr_data->sc_data.amr_conf.st_mode = chan_mod->multirate_conf.st_mode;
      rr_data->sc_data.amr_conf.set_amr = chan_mod->multirate_conf.set_amr;

      rr_data->sc_data.amr_conf.v_cod_prop = chan_mod->multirate_conf.v_cod_prop;
      if(rr_data->sc_data.amr_conf.v_cod_prop)
      {
        rr_data->sc_data.amr_conf.c_cod_prop   = chan_mod->multirate_conf.c_cod_prop;
        for (i=0; i< rr_data->sc_data.amr_conf.c_cod_prop; i++)
          memcpy(&rr_data->sc_data.amr_conf.cod_prop[i], &chan_mod->multirate_conf.cod_prop[i], sizeof(T_cod_prop));
      }
    }

    /*
     * configure layer 1
     */
    dat_code_mph_chan_mode_req (chan_mod);

    EM_CHANNEL_MODE_MODIFY;

    /*
     * indicate new channel mode to MM
     */
    dat_code_channel_mode_to_mm ();
  }

  {
    /*
     * build the answer to the network
     * (channel mode modify acknowledge message)
     */
    PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_CHAN_MOD_ACK);

    /*
     * set channel type and SAPI
     */
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi,
                          rr_data->sc_data.chan_desc.chan_type);

    chan_mod_ack->msg_type = U_CHAN_MOD_ACK;
    memcpy (&chan_mod_ack->chan_desc,
            &rr_data->sc_data.chan_desc,
            sizeof (T_chan_desc));

    /*
     * set the current channel mode. if the new
     * channel mode is supported by the MS, the new
     * one is returned, else it is the previous one
     * and layer 1 was not re-configured.
     */
    chan_mod_ack->chan_mode = rr_data->sc_data.ch_mode;

    for_dat_data_req (dl_data_req);

    EM_CHANNEL_MODE_MODIFY_ACK;
  }

  PFREE(dl_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_chan_rel           |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a channel release message.

*/

GLOBAL void dat_for_chan_rel (T_DL_DATA_IND *dl_data_ind,
                              T_D_CHAN_REL *chan_rel)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_for_chan_rel()");

  if (GET_STATE (STATE_DAT) NEQ DAT_NULL)
  {
    /*
     * disconnect layer 2 link
     */
    dat_disconnect_link (CAUSE_MAKE (DEFBY_STD,
                         ORIGSIDE_NET,
                         RR_ORIGINATING_ENTITY,
                         chan_rel->rr_cause));

#ifdef GPRS
    if (chan_rel->v_gprs_resum)
    {
      rr_data->gprs_data.gprs_resump  = chan_rel->gprs_resum.res_ack;
    }
    /*
      o if the element is not available but we have send a susp_req
        a resumption failure has occured (gprs_resump was already set
        on tx of the suspension request)
      o if the element is not present and we have not send a suspension
        request there is no resumption failure.
      o For Ericsson we have to do a RAU after every CS call even if the
       call started on a GSM-only cell and we did not send a suspension request     */
    else
      if(att_gprs_is_avail())
        rr_data->gprs_data.gprs_resump = GPRS_RESUMPTION_NOT_ACK;
#endif

    if (chan_rel->v_ba_range)
    {
      /*
       *  convert RR_BA_RANGE to BCCH-LIST and
       *  send it with RR SYNC IND to MM
       */
      dat_code_prr_bcch_info (chan_rel->v_ba_range,
                              &chan_rel->ba_range);
    }

    EM_CHANNEL_RELEASE;
  }
  PFREE (dl_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_class_enq          |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a classmark enquiry message.

*/

#ifdef REL99
GLOBAL void dat_for_class_enq (T_DL_DATA_IND *dl_data_ind, 
                               T_D_CLASS_ENQ *class_enq)
#else
GLOBAL void dat_for_class_enq (T_DL_DATA_IND *dl_data_ind)
#endif
{
  TRACE_FUNCTION ("dat_for_class_enq()");

  if (dat_check_error_flag (SEND_RR_STATUS))
  {
    /*
     * The syntax check indicates no problems, then
     * process the message.
     *
     * The MS returns a classmark change message.
     */
/* Implements RR Clone findings #15 */
#ifdef REL99
    /*Perform checks on classmark enquiry mask IE, if present*/
    if ((class_enq->v_class_enq_mask EQ FALSE) OR 
        ((class_enq->v_class_enq_mask EQ TRUE) AND
         (class_enq->class_enq_mask.class_req EQ CLASS_CHANGE_REQ) )  )
#endif
      dat_class_chng_data_req();
  }

  EM_CLASSMARK_ENQUIRY;

  PFREE (dl_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : send_mph_tch_loop_req      |
+--------------------------------------------------------------------+

  PURPOSE : Send the L1 primitive for close TCH loop.

*/

static void send_mph_tch_loop_req(T_DL_DATA_IND * dl_data_ind,
                                  UBYTE           loop_command)
{
  /*
   * configure layer 1
   */
  PREUSE (dl_data_ind, loop_req, MPH_TCH_LOOP_REQ);/* T_MPH_TCH_LOOP_REQ */
  loop_req->tch_loop = loop_command;
  PSENDX (PL, loop_req);
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                        |
| STATE   : code                ROUTINE : send_close_tch_loop_ack_to_nw |
+-----------------------------------------------------------------------+

  PURPOSE : Send the CLOSE TCH LOOP ACK message to the network.

*/
static void send_close_tch_loop_ack_to_nw(void)
{
  GET_INSTANCE_DATA;
  /*
   * if the TCH loop is open and a TCH is assigned
   */

  PALLOC_SDU (data_req, DL_DATA_REQ, 2*BITS_PER_BYTE);
  /*
   * set channel type and sapi for the response to the network
   */
  dat_code_prr_channel (&data_req->ch_type,
                        &data_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  /*
   * code the message without CCD
   */
  data_req->sdu.l_buf = 16;
  data_req->sdu.o_buf = ENCODE_OFFSET;
  data_req->sdu.buf [0] = 0;
  /*lint -e415 -e416 Likely access of out-of-bounds pointer*/
  data_req->sdu.buf [1] = 0;
  data_req->sdu.buf [2] = 0;
  data_req->sdu.buf [3] = 0x0F;   /* TI=0, PD = TST     */
  data_req->sdu.buf [4] = 0x01;   /* MT = Close TCH Ack */
  /*lint +e415 +e416 Likely access of out-of-bounds pointer*/
  TRACE_EVENT ("DL_DATA_REQ (RR message)");

  EM_TCH_LOOP_CLOSED;

  PSENDX (DL, data_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_close_loop_cmd     |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a TCH Close Loop Command message.

*/

static const UBYTE LOOP_TYPE [32] =
  {              /* C B A Z Y               */
    0x00,        /* 0 0 0 0 0  -> Type A    */
    0x01,        /* 0 0 0 0 1  -> Type B    */
    0x02,        /* 0 0 0 1 0  -> Type C    */
    0x02,        /* 0 0 0 1 1  -> Type C    */
    0x03,        /* 0 0 1 0 0  -> Type D    */
    0x03,        /* 0 0 1 0 1  -> Type D    */
    0x03,        /* 0 0 1 1 0  -> Type D    */
    0x03,        /* 0 0 1 1 1  -> Type D    */
    0x04,        /* 0 1 0 0 0  -> Type E    */
    0x04,        /* 0 1 0 0 1  -> Type E    */
    0x04,        /* 0 1 0 1 0  -> Type E    */
    0x04,        /* 0 1 0 1 1  -> Type E    */
    0x05,        /* 0 1 1 0 0  -> Type F    */
    0x05,        /* 0 1 1 0 1  -> Type F    */
    0x05,        /* 0 1 1 1 0  -> Type F    */
    0x05,        /* 0 1 1 1 1  -> Type F    */
    0xFF,        /* 1 0 0 0 0  -> Not valid */
    0xFF,        /* 1 0 0 0 1  -> Not valid */
    0xFF,        /* 1 0 0 1 0  -> Not valid */
    0xFF,        /* 1 0 0 1 1  -> Not valid */
    0xFF,        /* 1 0 1 0 0  -> Not valid */
    0xFF,        /* 1 0 1 0 1  -> Not valid */
    0xFF,        /* 1 0 1 1 0  -> Not valid */
    0xFF,        /* 1 0 1 1 1  -> Not valid */
    0xFF,        /* 1 1 0 0 0  -> Not valid */
    0xFF,        /* 1 1 0 0 1  -> Not valid */
    0xFF,        /* 1 1 0 1 0  -> Not valid */
    0xFF,        /* 1 1 0 1 1  -> Not valid */
    0x06,        /* 1 1 1 0 0  -> Type I    */
    0x06,        /* 1 1 1 0 1  -> Type I    */
    0x06,        /* 1 1 1 1 0  -> Type I    */
    0x06         /* 1 1 1 1 1  -> Type I    */
  };


GLOBAL void dat_for_close_loop_cmd (T_DL_DATA_IND * dl_data_ind,
                                    UBYTE           subchannel)
{
  GET_INSTANCE_DATA;
  UBYTE loop_command = NOT_PRESENT_8BIT;

  TRACE_FUNCTION ("dat_for_close_loop_cmd()");

  if (dat_test_sim_available () OR !dat_check_sim_available () )
  {
    /*
     * only if a test SIM card is inserted
     */
    if ((rr_data->tch_loop_subch EQ NOT_PRESENT_8BIT) AND
          rr_data->sc_data.chan_desc.chan_type < CH_SDCCH_4_0)
    {
      switch ((loop_command = LOOP_TYPE [(subchannel>>1) & 0x1F])) 
      {
        case TCH_LOOP_C:         /* Loop C */
          /* first send ACK msg, then activate L1  */
          send_close_tch_loop_ack_to_nw();
          /*
           * Delay to allow L1/HW to switch
           */
          vsi_t_sleep (VSI_CALLER DELAY_CLOSE_TCH_LOOP_ACK);
          send_mph_tch_loop_req(dl_data_ind, loop_command);
          /* will be needed when TCH Open Loop Command will be received */
          rr_data->tch_loop_subch = loop_command;
          break;
        case TCH_LOOP_I:         /* Loop I */
          if (rr_data->sc_data.ch_mode NEQ CM_AMR) 
          {
            PFREE (dl_data_ind);
            break;
          }
        case TCH_LOOP_A:
        case TCH_LOOP_B:
        case TCH_LOOP_D:
        case TCH_LOOP_E:
        case TCH_LOOP_F:
          /* Loop A, B, D, E, F, I */
          send_mph_tch_loop_req(dl_data_ind, loop_command);
          /*
           * Delay to allow L1/HW to switch
           */
          vsi_t_sleep (VSI_CALLER DELAY_CLOSE_TCH_LOOP_ACK);
          send_close_tch_loop_ack_to_nw();
          /* will be needed when TCH Open Loop Command will be received */
          rr_data->tch_loop_subch = loop_command;
          break;
        default :
          TRACE_EVENT_P1("TCH_LOOP_CLOSE_CMD : wrong subchannel (%x)", subchannel);
          PFREE (dl_data_ind);
          break;
      }
    }
    else
    {
      PFREE (dl_data_ind);
    }
  }
  else
  {
    PFREE (dl_data_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_test_interface     |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a Test-Interface message.

*/

GLOBAL void dat_for_test_interface (T_DL_DATA_IND * dl_data_ind,
                                    UBYTE           tested_device)
{
  TRACE_FUNCTION ("dat_for_test_interface()");

  if (dat_test_sim_available ())
  {
    /*
     * Only if a test SIM card is inserted
     *
     * then configure layer 1
     */
    PREUSE (dl_data_ind, dai_req, MPH_DAI_REQ); /* T_MPH_DAI_REQ */

    dai_req->device = tested_device;

    EM_TEST_INTERFACE;

    PSENDX (PL, dai_req);
  }
  else
  {
    /*
     * else ignore the message
     */
    PFREE (dl_data_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_ciph_cmd           |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a cipher mode command message.

*/

GLOBAL void dat_for_ciph_cmd (T_DL_DATA_IND *dl_data_ind,
                              T_D_CIPH_CMD *ciph_cmd)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_for_cyph_cmd()");

  if (dat_check_error_flag (SEND_RR_STATUS))
  {
    /*
     * the check in the formatter was successful
     */
    if (
         ((rr_data->sc_data.ciph_on EQ CIPH_ON) AND
           (ciph_cmd->ciph_mode_set.sc EQ START_CIPH_YES))
         OR
         ((ciph_cmd->ciph_mode_set.sc EQ START_CIPH_YES) AND
          (rr_data->dyn_config.nkc EQ 0 AND rr_data->ms_data.cksn > 6))
       )
    {
      /*
       * Respond with RR Status in 2 cases
       *
       * 1: if NW re-enables ciphering 
       * 2: if network has enabled ciphering "and" no valid ciphering key
       *     is available (and user specific handling of cksn is 
       *     disabled (nck==0)).
       *     If network has not enabled ciphering, then ciphering key 
       *     value is not checked
       */
      dat_send_rr_status_msg(RRC_PROT_UNSPECIFIED);
    }
    else
    {
      MCAST (ciph_comp, U_CIPH_COMP);
      PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_CIPH_COMP);

      /*
       * set channel type and SAPI for response to the network
       */
      dat_code_prr_channel (&dl_data_req->ch_type,
                            &dl_data_req->sapi,
                            rr_data->sc_data.chan_desc.chan_type);

      /*
       * store cipher parameter
       */
      rr_data->sc_data.ciph_on = ciph_cmd->ciph_mode_set.sc;

      rr_data->sc_data.algo    = ciph_cmd->ciph_mode_set.algo_ident;
      rr_data->sc_data.ciph_received = TRUE;
      memcpy (rr_data->ms_data.kc, rr_data->ms_data.new_kc, KC_STRING_SIZE);

      /*
       * configure layer 1
       */

      if ( rr_data->ms_data.cksn <= 6 )
      {
        dat_code_mph_ciphering_req (rr_data->sc_data.ciph_on,
                                    rr_data->sc_data.algo,
                                    rr_data->ms_data.kc);
      }
      else
      {
        dat_code_mph_ciphering_req (CIPH_OFF, 0, NULL);
      }

      if (ciph_cmd->ciph_res.cr EQ INC_IMEISV_YES)
      {
        /*
         * if the response shall contain the IMEI, fill it in.
         */
        ciph_comp->v_mob_ident    = TRUE;
        memcpy (&ciph_comp->mob_ident, &rr_data->ms_data.imei,
                sizeof (T_mob_ident));
      }
      else
      {
        ciph_comp->v_mob_ident = FALSE;
      }

      ciph_comp->msg_type = U_CIPH_COMP;

      /*
       * send response to the network
       */
      for_dat_data_req (dl_data_req);

      /*
       * Indicate changed ciphering mode to MM.
       * Any supression of ciphering information to MMI/ACI will
       * be done by the upper layers.
       */
      dat_code_ciphering_to_mm (rr_data->sc_data.ciph_on);

      EM_CIPHERING_CMD;
    }
  }

  PFREE (dl_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_freq_redef         |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a frequency redefinition message.

*/

GLOBAL void dat_for_freq_redef (T_DL_DATA_IND  *dl_data_ind,
                                T_D_FREQ_REDEF *freq_redef,
                                T_LIST         *cell_chan_desc)
{
  GET_INSTANCE_DATA;
  T_start  start;
  T_LIST   hop_list;
  UBYTE    mob_alloc[65];

  TRACE_FUNCTION ("dat_for_freq_redef()");

  if (dat_check_error_flag (SEND_RR_STATUS))
  {
    /*
     * the check in the formatter has passed
     */
    memcpy (&rr_data->sc_data.chan_desc,
            &freq_redef->chan_desc,
            sizeof (T_chan_desc));

    /*
     * convert the mobile allocation from the message format
     * to a list of 1-bit positions to build the hopping list.
     */
    att_bits_to_byte (mob_alloc,
                      freq_redef->mob_alloc.c_mac,
                      freq_redef->mob_alloc.mac);

    dat_set_last_used_channel (&rr_data->sc_data.chan_desc);

    if (freq_redef->v_cell_chan_desc)
    {
      /*
       * if the message contains a new cell channel description
       * copy the new one, else use the old one.
       */
      srv_copy_list (&rr_data->sc_data.cd.cell_chan_desc,
                     cell_chan_desc,
                     sizeof (T_LIST));
      rr_data->sc_data.cd.v_cell_chan_desc = WITH_CONTENT;
    }

    if (rr_data->sc_data.cd.v_cell_chan_desc NEQ NO_CONTENT)
    {
      /*
       * create the hopping list from cell channel description and
       * mobile allocation.
       */
      if(! srv_create_chan_mob_alloc (&rr_data->sc_data.cd.cell_chan_desc,
                                      &hop_list,
                                      mob_alloc))
      {

        U32 st = 51*((26 + freq_redef->start_time.t3 - freq_redef->start_time.t2 )%26)
                 + freq_redef->start_time.t3 + 1326*freq_redef->start_time.t1;
        U32 ct = dl_data_ind->fn%STARTING_TIME_INTERVAL;

#if defined(_SIMULATION_)
        TRACE_EVENT_WIN_P5 ("D_FREQ_REDEF: t1=%u t2=%u t3=%u, st=%u, ct=%u",
          freq_redef->start_time.t1, freq_redef->start_time.t2,
          freq_redef->start_time.t3, st, ct);
        TRACE_EVENT_WIN_P2 ("D_FREQ_REDEF: (st-ct) %u <= %u ?",
          ((STARTING_TIME_INTERVAL + st - ct)%STARTING_TIME_INTERVAL), STARTING_TIME_INTERVAL1);
#endif

        if(((STARTING_TIME_INTERVAL + st - ct)%STARTING_TIME_INTERVAL) <= STARTING_TIME_INTERVAL1)
        {
          /*XXX this should only be done if the starting time has not yet expired */
          dat_send_rr_status_msg(RRC_FREQ_NOT_IMPL);
        }
        else
        {
          /*
           * 3GPP TS 04.18, section 3.4.5.1
           * Frequency redefinition procedure, abnormal cases:
           * If the mobile station receives a FREQUENCY REDEFINITION message
           * with a Mobile Allocation IE indexing frequencies that are not all
           * in one band and a Starting Time IE indicating a time that has
           * elapsed, then the mobile station shall locally abort the radio
           * connection and, if permitted, attempt Call Re-establishment.
           *
           * Inform MM about a radio link failure and start cell reselection.
           * It would be possible to create a new cause but RLF does exactly
           * what is needed and this is really 'some kind of' RLF.
           */
          rr_data->net_lost = TRUE;
          att_code_rr_abort_ind (RRCS_ABORT_RAD_LNK_FAIL);
          att_stop_dedicated();
        }
      }
      else
      {
        /*
         * copy start time for the new hopping list
         */
        start.v_start = TRUE;
        start.t1      = freq_redef->start_time.t1;
        start.t2      = freq_redef->start_time.t2;
        start.t3      = freq_redef->start_time.t3;

        /*
         * configure layer 1 with the new hopping list
         */
        dat_code_mph_freq_redef_req (&start,
                                     &hop_list);
      }
    }
  }

  PFREE (dl_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_handov_cmd         |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a handover command message.

*/

GLOBAL void dat_for_handov_cmd (T_DL_DATA_IND  *dl_data_ind,
                                T_D_HANDOV_CMD *handov_cmd,
                                T_LIST         *cell_chan_desc,
                                T_LIST         *hop_list_after,
                                T_LIST         *hop_list_before)
{
  GET_INSTANCE_DATA;
  UBYTE                  mob_alloc [65];

  TRACE_FUNCTION ("dat_for_handov_cmd()");

  rr_data->dyn_config.fho               = 0;

  /*
   * dynamic configuration command : IHO 
   * Lock the DUT to the cell it is already camping -
   * Ignore the Handover command message and send an
   * Handover failure message to the network.
   */

  if(rr_data->dyn_config.iho AND (rr_data->sc_data.ch_mode EQ CM_AMR))
  {
    TRACE_EVENT("D_HANDOV_CMD : IHO");
    for_set_content_error (RRC_CHANNEL_MODE);
  }

  switch (rr_data->ms_data.error.cs)
  {
    /*
     * in case of mandatory info element error,
     * the message is returned immediately.
     */
    /* case RRC_INVALID_MAN_INFO: this value is currently never set */
    case RRC_COND_IE_ERROR:
    {
      /*
       * build a RR status message.
       */
      dat_send_rr_status_msg(rr_data->ms_data.error.cs);
      PFREE (dl_data_ind);
      break;
    }

    case RRC_INCORRECT_MSG:
    {
      /*
       * If a structurally correct message has been detected,
       * containing erroneous data, an Assignment Failure message
       * is sent back.
       */

      /*
       * Even though it's not possible to go onto the new channel
       * we still need to suspend the current link and send the
       * HANDOVER FAILURE command via a priority DL_RECONNECT_REQ.
       * This ensures DL will halt processing anything in its
       * buffer until it has sent this message onto the nw
       */
      for_suspend_layer_2 ();
      dat_send_handov_fail_msg(rr_data->ms_data.error.val);

      RR_EM_SET_HANDOVER_FAIL_CAUSE(rr_data->ms_data.error.val);

      PFREE (dl_data_ind);
      break;
    }

    default:
      /*
       * the message check has passed.
       * first of all suspend current layer 2 link
       */

      TRACE_EVENT_P1 ("HO:default (%02x)", rr_data->ms_data.error.cs);

      for_suspend_layer_2 ();

      /*
       * set for the optional information elements
       * of the handover message the default value
       * to the current serving cell value.
       */
      rr_data->cr_data.ch_mode   = rr_data->sc_data.ch_mode;
      rr_data->cr_data.ciph_on   = rr_data->sc_data.ciph_on;
      rr_data->cr_data.algo      = rr_data->sc_data.algo;
      rr_data->cr_data.cd.v_cell_chan_desc =
          rr_data->sc_data.cd.v_cell_chan_desc;
      memcpy (&rr_data->cr_data.cd.cell_chan_desc,
              &rr_data->sc_data.cd.cell_chan_desc,
              sizeof (T_LIST));

      /*
       * if AMR is supported set the default values
       * to the current serving cell values.
       */
      if(rr_data->sc_data.ch_mode EQ CM_AMR)
      {
        memcpy(&rr_data->cr_data.amr_conf,
               &rr_data->sc_data.amr_conf,
               sizeof (T_multirate_conf));
      }
      else {
      /*
      *  AMR is not supported, therefore set some dummy values. This is necessary because
      *  the later Layer1 configuration must include an AMR configuration!!
      */
        memset(&rr_data->cr_data.amr_conf, 0, sizeof (T_multirate_conf));
      }

      /*
       * set BSIC, BCCH channel number and channel description from
       * the handover command.
       */
      rr_data->nc_data[CR_INDEX].bsic    = (handov_cmd->cell_desc.ncc << 3) +
                                            handov_cmd->cell_desc.bcc;
      rr_data->nc_data[CR_INDEX].arfcn   =  handov_cmd->cell_desc.bcch_arfcn_lo +
                                            (handov_cmd->cell_desc.bcch_arfcn_hi << 8);
      memcpy (&rr_data->cr_data.chan_desc,
              &handov_cmd->chan_desc_after,
              sizeof (T_chan_desc));

      if (handov_cmd->v_synch_ind)
      {
        /*
         * store the Handover synchronisation information if available.
         */
        memcpy (&rr_data->ms_data.ho_type, &handov_cmd->synch_ind,
                sizeof (T_synch_ind));
      }
      else
      {
        /*
         * else set the values to the default values.
         */
        rr_data->ms_data.ho_type.rot = TIME_DIFF_NO;
        rr_data->ms_data.ho_type.nci = TRUE;
        rr_data->ms_data.ho_type.si  = SYI_NON_SYNCH;
      }

      if (rr_data->ms_data.ho_type.si EQ SYI_PSEUDO_SYNCH AND
          ! rr_data->ms_data.classmark2.ps)
      {
        /*
         * if the handover requests a pseudo synchronized handover
         * and the mobile does not support this, a handover failure
         * message is send and the procedure is aborted with
         * reconnection to the old channel.
         */
        dat_send_handov_fail_msg(RRC_INCORRECT_MSG);

        RR_EM_SET_HANDOVER_FAIL_CAUSE(RRC_INCORRECT_MSG);

        PFREE (dl_data_ind);
        return;
      }
      else
      {
        if (handov_cmd->v_cell_chan_desc)
        {
          /*
           * if the handover command contains a new cell channel description
           * copy the new list.
           */
          srv_copy_list (&rr_data->cr_data.cd.cell_chan_desc,
                         cell_chan_desc,
                         sizeof (T_LIST));
          rr_data->cr_data.cd.v_cell_chan_desc    = WITH_CHANGED_CONTENT;
        }

        if (handov_cmd->v_chan_mode)
        {
          /*
           * store a new channel mode if available.
           */
          rr_data->cr_data.ch_mode = handov_cmd->chan_mode;
        }


        /*
         *  If AMR is signalled check if new multi-rate speech codec is part of the handover cmd
         *  otherwise use default values set earlier. If AMR is not signalled use the dummy values
         *  instead either set earlier.
         */

/* Implements RR Clone findings #9 */
        dat_cr_data_multirate_conf(handov_cmd->v_multirate_conf, &handov_cmd->multirate_conf);

   
        if (handov_cmd->v_ciph_mode_set)
        {
          /*
           * if the message contains cipher mode parameter
           * copy the parameters
           */
          rr_data->cr_data.ciph_on = handov_cmd->ciph_mode_set.sc;
          rr_data->cr_data.algo    = handov_cmd->ciph_mode_set.algo_ident;

          /*
           * if ciphering is already enabled and the handover command
           * requests ciphering again, the procedure is aborted with
           * a handover failure message.
           */
          if (rr_data->cr_data.ciph_on EQ CIPH_ON AND
              rr_data->sc_data.ciph_received EQ FALSE)
          {
            dat_send_handov_fail_msg(RRC_PROT_UNSPECIFIED);

            RR_EM_SET_HANDOVER_FAIL_CAUSE(RRC_PROT_UNSPECIFIED);

            PFREE (dl_data_ind);
            return;
          }
        }

        if(handov_cmd->v_mob_alloc_after)
        {
          if(dat_for_handover_mob_alloc(mob_alloc, hop_list_after, (T_VOID_STRUCT *) &handov_cmd->mob_alloc_after,dl_data_ind))
            return;
        }

        if(handov_cmd->v_mob_alloc_before)
        {
          if(dat_for_handover_mob_alloc(mob_alloc, hop_list_before, (T_VOID_STRUCT *) &handov_cmd->mob_alloc_before,dl_data_ind))
            return;
        }

        /*
         * Handover resets a SAPI 3 connection for SMS
         */
        SET_STATE (STATE_SAPI_3, SMS_IDLE);
        PFREE (dl_data_ind);
        {
          /*
           * All Parameters are checked
           * Now the handover is started
           */
          PALLOC (dedicated_req, MPH_DEDICATED_REQ);

          memset (dedicated_req, 0, sizeof (T_MPH_DEDICATED_REQ));

          if (handov_cmd->v_start_time)
          {
            /*
             * copy starting time if available.
             */
            dedicated_req->start.v_start = TRUE;
            dedicated_req->start.t1      = handov_cmd->start_time.t1;
            dedicated_req->start.t2      = handov_cmd->start_time.t2;
            dedicated_req->start.t3      = handov_cmd->start_time.t3;
          }

          /*
           * Calculate observed time difference
           */
          {
            UBYTE i1;
            ULONG fn_offset;

            rr_data->sc_data.observed_ta = 0;
            for (i1 = 0; i1< rr_data->ms_data.measurement_report.ncells.no_of_ncells; i1++)
            {
              /*
               * find the handover cell inn the neighbourcells of the last measurement report
               */
              if (rr_data->nc_data[CR_INDEX].arfcn EQ
                  rr_data->ms_data.measurement_report.ncells.arfcn[i1])
              {
                /*
                 * According to 05.10 OTD is defined as the timing difference
                 * between BTS0 and BTS1 ( system time in BTS 0 minus that of
                 * BTS 1..), with BTS1 as the new cell (neighbour cell, HO
                 * Target cell) and BTS0 and the current cell (serving cell)
                 */
                fn_offset = (HYPERFRAME -
                             rr_data->ms_data.measurement_report.ncells.frame_offset[i1])
                             % HYPERFRAME;

                /*
                 * calculate the observed time difference from the relative
                 * time difference of neighbourcell and serving cell
                 * (given by time_alignment and frame offset) and the observed
                 * time difference of the serving cell (coming from timing
		 * advance in layer 1 header of the downlink SACCH messages).
                 */
                 /*
                  * A.1.3 of   3GPP TS 05.10
                  * after successful handover, either synchronized,
                  * non-synchronized or pseudo-synchronized, the MS shall
		  * provide to BTS 1 the value of OTD + t0 in the
		  * "HANDOVER COMPLETE" message.
                  *
                  * NOTE : measurement_report.otd is the TA sent by the
                  * network in downlink SACCH.  TA is roundtrip propogation
		  * delay in bit periods.
                  * t0 denotes the "one way" line of sight propagation delay
		  * between the MS and BTS 0, in "half bits".
                  * t0 = measurement_report.otd * 2 / 2.
                  */
                rr_data->sc_data.observed_ta =
                  ( (rr_data->ms_data.measurement_report.ncells.time_alignmt[i1]
                    + fn_offset* QUARTER_BITS_PER_FRAME)/2
                    +  rr_data->ms_data.measurement_report.otd ) % 2097152;
              }
            }
          }

          dedicated_req->ho_param.ho_nci = rr_data->ms_data.ho_type.nci;

          /*
           * Set the handover mode
           */
          switch (rr_data->ms_data.ho_type.si)
          {
            case SYI_NON_SYNCH:
              /*
               * asynchronous handover
               */
              dedicated_req->mod = MODE_ASYNC_HANDOVER;
              break;

            case SYI_NORM_SYNCH:
              /*
               * synchronous handover
               */
              rr_data->sc_data.new_ta    = rr_data->ms_data.measurement_report.otd/2;
              att_set_tim_advance_info();
              dedicated_req->mod = MODE_SYNC_HANDOVER;
              break;

            case SYI_PRE_SYNCH:
              /*
               * pre-synchronized handover
               */
              dedicated_req->mod = MODE_PRE_SYNC_HANDOVER;

              if (handov_cmd->v_time_advance)
                /*
                 * if the handover command contains a new timing advance
                 */
                dedicated_req->tr_para.tav     = handov_cmd->time_advance.ta;
              else
                /*
                 * else set the default value 1
                 */
                dedicated_req->tr_para.tav     = 1;

              rr_data->sc_data.new_ta = dedicated_req->tr_para.tav * 2;
              att_set_tim_advance_info();
              break;

            case SYI_PSEUDO_SYNCH:
              /*
               * pseudo-synchronized handover
               */
              dedicated_req->mod = MODE_PSEUDO_SYNC_HANDOVER;
              dedicated_req->tr_para.tav     = handov_cmd->time_diff;
              rr_data->sc_data.new_ta        = dedicated_req->tr_para.tav;
              att_set_tim_advance_info();
              break;
          }

          /*
           * Set Channel Type
           */
          dedicated_req->ch_type.ch    = handov_cmd->chan_desc_after.chan_type;
          dedicated_req->ch_type.tn    = handov_cmd->chan_desc_after.tn;
          dedicated_req->ch_type.tsc   = handov_cmd->chan_desc_after.tsc;
          dedicated_req->ch_type.h     = handov_cmd->chan_desc_after.hop;

          if (handov_cmd->chan_desc_after.hop EQ H_NO)
            dedicated_req->ch_type.arfcn = handov_cmd->chan_desc_after.arfcn;
          else
          {
            dedicated_req->ch_type.maio  = handov_cmd->chan_desc_after.maio;
            dedicated_req->ch_type.hsn   = handov_cmd->chan_desc_after.hsn;

            /* CSI-LLD section:4.1.1.11
             * This function Updates the black list with the MA list received
             * in the handover command  
             */   
            cs_remove_BA_MA_from_black_list(rr_data->cs_data.region , hop_list_after);

            srv_create_list (hop_list_after, dedicated_req->ch_type.ma,
                             MAX_MA_CHANNELS, TRUE, 0);
          }

          dedicated_req->bsic                 = rr_data->nc_data[CR_INDEX].bsic & 0x3F;
          dedicated_req->arfcn                = rr_data->nc_data[CR_INDEX].arfcn;
          dedicated_req->ho_param.ho_ref      = handov_cmd->handov_ref;
          dedicated_req->ho_param.ho_pow      = handov_cmd->pow_cmd_access.pow;
          dedicated_req->ho_param.ho_acc_type = handov_cmd->pow_cmd_access.atc;
          dedicated_req->tr_para.mode         = rr_data->cr_data.ch_mode;

          /*
           * Set multi-rate speech codec
           */
          dedicated_req->amr_conf.nscb     = rr_data->cr_data.amr_conf.nscb;
          dedicated_req->amr_conf.icmi     = rr_data->cr_data.amr_conf.icmi;
          dedicated_req->amr_conf.st_mode  = rr_data->cr_data.amr_conf.st_mode;
          dedicated_req->amr_conf.acs      = rr_data->cr_data.amr_conf.set_amr;

          /*
           *  valid flag for the threshold and hystersis values. amr_conf.c_cod_prop
           *  defines the number of threshold and hystersis values.
           */
          dedicated_req->amr_conf.v_cod_prop  = rr_data->cr_data.amr_conf.v_cod_prop;
          if(dedicated_req->amr_conf.v_cod_prop)
          {
            int i;
            dedicated_req->amr_conf.c_cod_prop = rr_data->cr_data.amr_conf.c_cod_prop;
            for(i=0; i<dedicated_req->amr_conf.c_cod_prop; i++)
              memcpy(&dedicated_req->amr_conf.cod_prop[i], &rr_data->cr_data.amr_conf.cod_prop[i], sizeof(T_cod_prop));
          }

          /*
           * Set Channel Type before starting time
           */

          if (handov_cmd->v_chan_desc_before EQ FALSE)
            dedicated_req->ch_type2.ch   = NOT_PRESENT_8BIT;
          else
          {

/* Implements RR Clone findings #22 */
            dat_dedicated_req_ch_type2(&dedicated_req->ch_type2, &handov_cmd->chan_desc_before,
                                       hop_list_before);
          }

          if (rr_data->cr_data.ciph_on)
          {
            /*
             * set cipher parameter
             */
            dedicated_req->ciph.stat     = rr_data->cr_data.ciph_on;
            dedicated_req->ciph.algo     = rr_data->cr_data.algo;
            memcpy (dedicated_req->ciph.kc, rr_data->ms_data.kc, KC_STRING_SIZE);
          }

          /*
           * clear neighbourcell lists for the new cell.
           */
          srv_clear_list (&rr_data->sc_data.cd.ncell_list);
          srv_clear_list (&rr_data->sc_data.five_ter_list);

          att_clean_buf (IND_ALL_DEDI_SI);

          rr_data->sc_data.cd.sys_info_read &= ~ALL_DEDI_SYS_INFOS;

          SET_STATE (STATE_DAT, DAT_HANDOVER);

#if defined (REL99) && defined (TI_PS_FF_EMR)
          /*clear EMR parameters also, if present*/
          if (rr_data->sc_data.enh_para_status EQ ENH_PARA_DEDICATED )
          {
            /*discard the enhanced para that were related to BA(SACCH) before HO*/
            rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
            memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
            for_set_default_emr_data(&rr_data->sc_data.emr_data_current);
            /*Indicate to ALR that enhanced para are invalid*/
            attf_send_enh_para_to_alr(rr_data->sc_data.emr_data_current.rep_type,
              &rr_data->sc_data.emr_data_current.enh_para); 
          }
#endif

          RR_EM_GET_HOPPING_CHANNEL (dedicated_req->ch_type.ma, dedicated_req->ch_type2.ma,
                           dedicated_req->start.v_start,dedicated_req->ch_type2.maio);

          EM_HANDOVER_CMD;

#if defined FF_EOTD
          if ( rr_data->eotd_req_id NEQ NOT_PRESENT_16BIT )
          {
            PALLOC (rrlc_error_ind, RRLC_ERROR_IND);
            rrlc_error_ind->cause = LCS_HANDOVER;

            rr_data->eotd_req_id = NOT_PRESENT_16BIT;

            PSENDX (LC, rrlc_error_ind);
          }
#endif /* FF_EOTD */

          /*
           * configure layer 1.
           */
          PSENDX (PL, dedicated_req);
        }
      } /* else */
  } /* switch */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : stop_rach_and_enter_idle   |
+--------------------------------------------------------------------+

  PURPOSE : Invalid frequency list received during Immediate Assignment
            procedure. The sending of Channel Request messages is
            stopped and Idle Mode entered.
*/

LOCAL void stop_rach_and_enter_idle(void)
{
  PALLOC (mph_random_access_req, MPH_RANDOM_ACCESS_REQ);

  TRACE_ERROR ("invalid frequencies (Frequency Hopping)");

  TIMERSTOP (T3122);
  TIMERSTOP (T3126);

  /*
   *  Stop sending Random Burst
   */
  memset (&mph_random_access_req->send_mode, 0, sizeof (T_send_mode));
  PSENDX (PL, mph_random_access_req);

  dat_send_release_ind (RRCS_INVALID_HOP_FREQ);

/*SET_STATE (STATE_DAT, DAT_IDLE);
  att_build_idle_req (SC_INDEX, MODE_CELL_SELECTION);*/

#ifdef GPRS
  att_start_cell_reselection_gprs (CELL_RESELECTION_RACH);
#else
  att_start_cell_reselection (CELL_RESELECTION_RACH);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_imm_assign         |
+--------------------------------------------------------------------+

  PURPOSE : Reception of an immediate assignment message.

*/

GLOBAL void dat_for_imm_assign (T_MPH_UNITDATA_IND *mph_unitdata_ind,
                                T_D_IMM_ASSIGN     *imm_assign)
{
  GET_INSTANCE_DATA;
  T_SC_DATA *rrd = &rr_data->sc_data;
  T_start   start;
  UBYTE     mob_alloc [65];
  T_LIST    hop_list_bef;
  T_LIST    hop_list_after;
  UBYTE     maio;
  T_IA_REST ia_rest;
  UBYTE     index = 0;

  TRACE_FUNCTION ("dat_for_imm_assign()");

  switch (GET_STATE (STATE_DAT))
  {
#ifdef GPRS
    case DAT_IDLE:
      TRACE_EVENT("check dl idle");
      dat_check_imm_assign_pch (mph_unitdata_ind, imm_assign);
      break;
#endif

    case DAT_IMM_ASS:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
#ifdef GPRS
        TRACE_EVENT("check dl pa");
        if(GET_STATE(STATE_GPRS) EQ GPRS_PAM_BCCH AND
           dat_check_imm_assign_pch(mph_unitdata_ind, imm_assign))
          return;
#endif
         if (dat_compare_request_ref (&imm_assign->req_ref, &index))
         {
           /*
            * the request reference in the immediate assignment
            * message matches to one of the last three channel
            * request messages.
            */
           TRACE_EVENT("matched");
           /*
            * check the channel description
            */
           if(imm_assign->v_chan_desc)
             for_check_channel_descr (&imm_assign->chan_desc);

           /* was channel description ok? */
           if(!dat_check_error_flag (SEND_NO_RR_STATUS))
             return;
#ifdef GPRS
           if(dat_check_gprs_imm_ass (mph_unitdata_ind,
                                       imm_assign,
                                       index))
             return;
#endif
           if (imm_assign->v_chan_desc)
           {
             if (imm_assign->chan_desc.hop AND
                 imm_assign->mob_alloc.c_mac)
             {
               TRACE_EVENT ("create mob alloc (after st)");
               /*
                * if the message contains a mobile allocation
                * build a hopping list together with the cell
                * channel description of system information
                * type 1 message.
                */
               att_bits_to_byte (mob_alloc,
                                 imm_assign->mob_alloc.c_mac,
                                 imm_assign->mob_alloc.mac);
               if( rrd->cd.v_cell_chan_desc EQ NO_CONTENT OR
                   !srv_create_chan_mob_alloc (&rrd->cd.cell_chan_desc,
                                              &hop_list_after,
                                              mob_alloc))
               {
                 stop_rach_and_enter_idle();
                 return;
               }
             }
             else
             {
               /*
                * else clear the hopping list
                */
               srv_clear_list (&hop_list_after);
             }
           }
           else
           {
             TRACE_EVENT("IMM ASS discarded: neither sent to GRR nor channel description found");
             return;  /* for non-packet access we need a channel description */
           }

           TRACE_EVENT("now get started");
           if (imm_assign->v_start_time)
           {
             /*
              * if the message contains a starting time,
              * store the starting time.
              */
             start.v_start = TRUE;
             start.t1 = imm_assign->start_time.t1;
             start.t2 = imm_assign->start_time.t2;
             start.t3 = imm_assign->start_time.t3;
           }
           else
           {
             /*
              * clear the starting time.
              */
             memset (&start, 0, sizeof (T_start));
           }

          /*
           * decode IA Rest Octet
           */
          memset (&ia_rest, 0, sizeof (T_IA_REST));

          ia_rest.ia_p     =  imm_assign->ia_rest_oct.flag_2bit;
          ia_rest.ia_maio  =  imm_assign->ia_rest_oct.ia_freq_par.maio;
          ia_rest.c_ia_mac =  imm_assign->ia_rest_oct.ia_freq_par.c_mac;
          if (ia_rest.c_ia_mac > 9)
            ia_rest.c_ia_mac = 9;

          memcpy (ia_rest.ia_mac,
                  &imm_assign->ia_rest_oct.ia_freq_par.mac,
                  ia_rest.c_ia_mac);

          if (imm_assign->v_start_time AND
              imm_assign->chan_desc.hop AND
              ia_rest.ia_p EQ 2)
          {
            /*
             * calculate frequency list before starting time
             */
            TRACE_EVENT("create mob alloc (before st)");
            maio = ia_rest.ia_maio;

            att_bits_to_byte (mob_alloc,
                              ia_rest.c_ia_mac,
                              ia_rest.ia_mac);
            if(rrd->cd.v_cell_chan_desc EQ NO_CONTENT OR
               !srv_create_chan_mob_alloc (&rrd->cd.cell_chan_desc,
                                           &hop_list_bef,
                                           mob_alloc))
            {
              stop_rach_and_enter_idle();
              return;
            }
          }
          else
          {
            maio = 0;
            srv_clear_list (&hop_list_bef);
          }


          /*
           * stop T3122 and T3126 if they are running.
           */
          TIMERSTOP (T3122);
          TIMERSTOP (T3126);
          SET_STATE (STATE_DAT, DAT_IMM_ASS_1);

          /*
           * store channel description
           */
          memcpy (&rrd->chan_desc, &imm_assign->chan_desc,
                  sizeof (T_chan_desc));

          /*
           * the initial channel mode is always signalling only
           */
          rrd->ch_mode   = MODE_SIG_ONLY;

          /*
           * set the timing advance
           */
          rrd->new_ta    = imm_assign->time_advance.ta;
          att_set_tim_advance_info();
          dat_set_last_used_channel (&rrd->chan_desc);


          /*
           * configure layer 1
           */
          dat_code_mph_imm_assign_req (&start,
                                       rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch,
                                       maio,
                                       &hop_list_after,
                                       &hop_list_bef);
         }
         EM_IMMEDIATE_ASSIGNMENT;
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_imm_assign_ext     |
+--------------------------------------------------------------------+

  PURPOSE : Reception of the immediate assignment extended message.

*/


GLOBAL void dat_for_imm_assign_ext (T_MPH_UNITDATA_IND *mph_unitdata_ind,
                                    T_D_IMM_ASSIGN_EXT *imm_assign_ext)
{
  GET_INSTANCE_DATA;
  USHORT     i;
  T_start    start;
  UBYTE     mob_alloc [65];
  T_SC_DATA *rrd = &rr_data->sc_data;
  T_LIST     hop_list_after;
  T_LIST     hop_list_bef;
  UBYTE      index;
  T_chan_desc *p_chan_desc;

  TRACE_FUNCTION ("dat_for_imm_assign_ext()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_IMM_ASS:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
        /*
         * the message check in the formatter has passed
         */
        for (i=0; i<2; i++)
        {
          /*
           * the immediate assignment extended message contains
           * two request references.
           */
          if (dat_compare_request_ref ((i EQ 0)
                               ? &imm_assign_ext->req_ref
                               : (T_req_ref *)&imm_assign_ext->req_ref_2,
                               &index))
          {
#ifdef GPRS
            dat_check_imm_ass_ext (mph_unitdata_ind,(UBYTE)(i+1));
#endif
            /*
             * check channel description
             */
            if(i EQ 0)
              p_chan_desc = &imm_assign_ext->chan_desc;
            else
              p_chan_desc = (T_chan_desc *)&imm_assign_ext->chan_desc_2;
            for_check_channel_descr (p_chan_desc);

            if (!dat_check_error_flag (SEND_NO_RR_STATUS))
              return;
            /*
             * the request reference in the immediate assignment
             * extended message matches to one of the last three
             * channel request messages.
             */
            if (imm_assign_ext->mob_alloc.c_mac AND p_chan_desc->hop)
            {
              /*
               * if the message contains a mobile allocation and
               * the mobile shall hop
               * build a frequency hopping list together with
               * the cell channel description of system information
               * type 1 message.
               */
              att_bits_to_byte (mob_alloc,
                                imm_assign_ext->mob_alloc.c_mac,
                                imm_assign_ext->mob_alloc.mac);
              if(rrd->cd.v_cell_chan_desc EQ NO_CONTENT OR
                 ! srv_create_chan_mob_alloc (&rr_data->sc_data.cd.cell_chan_desc,
                                              &hop_list_after,
                                              mob_alloc))
              {
                stop_rach_and_enter_idle();
                return;
              }
            }
            else
            {
              /*
               * else clear frequency hopping list
               */
              srv_clear_list (&hop_list_after);
            }

            /*
             * stop T3122 and T3126 if they are running.
             */
            TIMERSTOP (T3122);
            TIMERSTOP (T3126);
            /*
             * store channel description
             */
            memcpy (&rrd->chan_desc,
                    p_chan_desc,
                    sizeof (T_chan_desc));

            /*
             * the initial channel mode is ever signalling only
             */
            rrd->ch_mode      = MODE_SIG_ONLY;

            /*
             * store the new timing advance
             */
            rrd->new_ta       = (i EQ 0)
                                   ? imm_assign_ext->time_advance.ta
                                   : imm_assign_ext->time_advance_2.ta;
            att_set_tim_advance_info();
            dat_set_last_used_channel (&rrd->chan_desc);


            if (imm_assign_ext->v_start_time)
            {
              /*
               * copy starting time if available
               */
              start.v_start = TRUE;
              start.t1 = imm_assign_ext->start_time.t1;
              start.t2 = imm_assign_ext->start_time.t2;
              start.t3 = imm_assign_ext->start_time.t3;
            }
            else
              memset (&start, 0, sizeof (T_start));

            srv_clear_list (&hop_list_bef);
            SET_STATE (STATE_DAT, DAT_IMM_ASS_1);

            /*
             * configure layer 1.
             */
            dat_code_mph_imm_assign_req (&start,
                                         rr_data->nc_data[SC_INDEX].select_para.ms_txpwr_max_cch,
                                         0,
                                         &hop_list_after,
                                         &hop_list_bef);

            EM_IMMEDIATE_ASSIGNMENT_EXT;
            return;
          }
        }
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_imm_assign_rej     |
+--------------------------------------------------------------------+

  PURPOSE : Reception of an immediate assignment reject message.

*/

GLOBAL void dat_for_imm_assign_rej (T_D_IMM_ASSIGN_REJ *imm_assign_rej)
{
  GET_INSTANCE_DATA;
  UBYTE index;

  TRACE_FUNCTION ("dat_for_imm_assign_rej()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_IMM_ASS:
      if (dat_check_error_flag (SEND_NO_RR_STATUS))
      {
        /*
         * the message has passed the checks in the formatter.
         */

        if (! IS_TIMER_ACTIVE(T3122))
        {
          /*
           * Only if T3122 is not running, that means there is no
           * immediate assignment reject message taken in account
           * before.
           */
          BOOL  result = FALSE;
          UBYTE t3122 = 0;

          if (dat_compare_request_ref (&imm_assign_rej->req_ref, &index))
          {
            /*
             * if the request reference matches to one of the last
             * three channel requests, set the result of TRUE and
             * store the timer value. This is checked for the up to
             * four request references in the message.
             */
            result = TRUE;
            t3122  = imm_assign_rej->t3122;
          }

          else if (dat_compare_request_ref ((T_req_ref *)&imm_assign_rej->req_ref_2, &index))
          {
            result = TRUE;
            t3122  = imm_assign_rej->t3122_2;
          }

          else if (dat_compare_request_ref ((T_req_ref *)&imm_assign_rej->req_ref_3, &index))
          {
            result = TRUE;
            t3122  = imm_assign_rej->t3122_3;
          }

          else if (dat_compare_request_ref ((T_req_ref *)&imm_assign_rej->req_ref_4, &index))
          {
            result = TRUE;
            t3122  = imm_assign_rej->t3122_4;
          }

          if (result)
          {
            /*
             * a request reference has matched
             */
#ifdef GPRS
            if (dat_check_imm_ass_rej (t3122) EQ FALSE)
#endif
            if (t3122 NEQ 0)
            {
              /*
               * start T3122 if a value is defined
               */
              TIMERSTART (T3122, T3122_VALUE(t3122));
            }
            TRACE_EVENT("set rej_rec");
            rr_data->imm_ass_rej_rec = TRUE;
            rr_data->ms_data.all_conf_received = TRUE;

            /*
             * Start T3126 if the timer is not running yet.
             */
/* Implements Measure#32: Row 217,218 */
            (IS_TIMER_ACTIVE(T3126)) ? 
              TRACE_TIMER ( "T3126 re-start") : TRACE_TIMER ( "T3126 start");
          

            if (! IS_TIMER_ACTIVE(T3126))
            {
              TIMERSTART (T3126, T3126_VALUE);
              /*
               *  Stop sending Random Burst
               */
              {
                PALLOC (mph_random_access_req, MPH_RANDOM_ACCESS_REQ);

                memset (&mph_random_access_req->send_mode, 0, sizeof (T_send_mode));
                PSENDX (PL, mph_random_access_req);
              }
            }
          }
          EM_IMMEDIATE_ASSIGNMENT_REJECT;
        }
      }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_ext_meas_order     |
+--------------------------------------------------------------------+

  PURPOSE : Reception of an extended measurement order message.

*/

GLOBAL void dat_for_ext_meas_order (T_D_EXT_MEAS_ORDER *ext_meas_order)
{
  GET_INSTANCE_DATA;
  UBYTE       new_seq;
  T_LIST     *chan_list;

  PALLOC (mph_emo_req, MPH_EMO_REQ);

  TRACE_FUNCTION ("dat_for_ext_meas_order()");

  /*
   * Decode EMO: new_seq
   */

  ccd_decodeByte (ext_meas_order->ext_meas_freq.b_ext_meas_freq,
                  (USHORT)(ext_meas_order->ext_meas_freq.o_ext_meas_freq+3),
                  1, &new_seq);

  /*
   * Ignore EMO if EMO proc already running and new SEQ EQ current SEQ.
   */

  if ( rr_data->emo_arfcn NEQ NULL AND rr_data->emo_seq EQ new_seq )
  {
    PFREE ( mph_emo_req );
    return;
  }

  /*
   * Decode and store EMO frequency list
   */

  if ( rr_data->emo_arfcn EQ NULL )
  {
    MALLOC ( rr_data->emo_arfcn, MAX_EMO_CHANNELS * sizeof (rr_data->emo_arfcn[0]) );
  }
  rr_data->emo_seq = new_seq;
  MALLOC ( chan_list, sizeof ( T_LIST ) );
  for_create_channel_list ( (T_f_range*) &ext_meas_order->ext_meas_freq, chan_list);

  /*
   * Function srv_create_list_dedicated ensures that the frequencies are sorted
   * and the number of frequencies are limited to 21 frequencies
   */

  rr_data->c_emo_arfcn = srv_create_list (chan_list,
                                          rr_data->emo_arfcn,
                                          MAX_EMO_CHANNELS,
                                          FALSE,
                                          0);
  MFREE ( chan_list );

  memcpy ( &mph_emo_req->arfcn[0],
           &rr_data->emo_arfcn[0],
            rr_data->c_emo_arfcn * sizeof (rr_data->emo_arfcn[0]) );

  mph_emo_req->c_arfcn =
  srv_remove_frequencies_in_array_gen ( &mph_emo_req->arfcn[0], rr_data->c_emo_arfcn );

  /*
   * Create newBA_ID, save as currentBA_ID.
   */

  rr_data->ba_id = RR_ALLOCATE_NEW_BA ( rr_data->ba_id );
  mph_emo_req->ba_id = rr_data->ba_id;

  /*
   * EMOtime = 10 seconds and request PL to perform extended measurement.
   */

  TIMERSTART (TIM_EXT_MEAS, 10000);

  PSENDX (PL, mph_emo_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_mph_emo_meas_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Measurement report for the Extended Measurment procedure
            has been received.

*/

GLOBAL void dat_mph_emo_meas_ind (T_MPH_EMO_MEAS_IND *mph_emo_meas_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("att_mph_emo_meas_ind()");

  switch (GET_STATE (STATE_DAT))
  {
    case DAT_DEDICATED:
      if ( rr_data->emo_arfcn NEQ NULL AND mph_emo_meas_ind->ba_id EQ rr_data->ba_id )
      {
        dat_code_ext_meas_report (mph_emo_meas_ind);
        dat_emo_stop ( TRUE );
      }
      else
      {
         /*
          * Build an invalid measurement reports
          */
        MCAST (meas, U_MEAS_REP);
        PALLOC_MSG (dl_unitdata_req, DL_UNITDATA_REQ, U_MEAS_REP);
        memset (&dl_unitdata_req->sdu.buf[0], 0, dl_unitdata_req->sdu.o_buf / BITS_PER_BYTE);

        memset (meas, 0, sizeof (T_U_MEAS_REP));
        meas->msg_type                = U_MEAS_REP;
        meas->meas_result.meas_valid = 1;
        for_dat_unitdata_req (dl_unitdata_req);
      }
      break;

    default:
      break;
  }
  PFREE (mph_emo_meas_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_emo_stop               |
+--------------------------------------------------------------------+

  PURPOSE : Stop the Extended Measurement Order procedure.

*/

GLOBAL void dat_emo_stop (BOOL send_ncell_req )
{
  GET_INSTANCE_DATA;
  if ( rr_data->emo_arfcn NEQ NULL )
  {
    MFREE (rr_data->emo_arfcn);
           rr_data->emo_arfcn = NULL;

 /* restore the neighbour cell description which was used prior EMO */
      if ( send_ncell_req AND
         (rr_data->sc_data.cd.sys_info_read & (SYS_INFO_5_READ | SYS_INFO_5BIS_READ | SYS_INFO_5TER_READ) ) )

      att_code_mph_ncell_req_dedicated();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_l3_data_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a layer 3 message for upper layers.

*/

GLOBAL void dat_for_l3_data_ind (T_DL_DATA_IND *dl_data_ind)
{
  GET_INSTANCE_DATA;
  /* RR_DATA_IND is not the same as DL_DATA_IND anymore because of the new
   * member fn (frame number) of T_DL_DATA_IND which is not contained in
   * T_RR_DATA_IND.
   */
  PALLOC_SDU(rr_data_ind, RR_DATA_IND, (USHORT)(dl_data_ind->sdu.l_buf+dl_data_ind->sdu.o_buf));

  TRACE_FUNCTION ("dat_for_l3_data_ind()");

  if (dl_data_ind->sapi EQ SAPI_3)
  {
    /*
     * if it is a SMS message, this is implicitly an
     * indication for an established SAPI 3 link
     */
    SET_STATE (STATE_SAPI_3, SMS_ESTABLISHED);
  }

  rr_data_ind->sdu.l_buf = dl_data_ind->sdu.l_buf;
  rr_data_ind->sdu.o_buf = dl_data_ind->sdu.o_buf;
  memcpy(rr_data_ind->sdu.buf, dl_data_ind->sdu.buf,
    (dl_data_ind->sdu.l_buf+dl_data_ind->sdu.o_buf)>>3);
  PFREE (dl_data_ind);
  /*
   * forward the message to MM for distribution
   */
  PSENDX (MM, rr_data_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : dat_for_open_loop_cmd      |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a TCH OPEN LOOP COMMAND message.

*/

GLOBAL void dat_for_open_loop_cmd (T_DL_DATA_IND *dl_data_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("dat_for_open_loop_cmd()");

  if (dat_test_sim_available () OR !dat_check_sim_available () )
  {
    /*
     * only if a test SIM card is inserted
     */
    if (rr_data->tch_loop_subch NEQ NOT_PRESENT_8BIT)
    {
      /*
       * A TCH Loop must be closed before, then
       * open in layer 1.
       */
      PREUSE (dl_data_ind, loop_req, MPH_TCH_LOOP_REQ);
      loop_req->tch_loop = NOT_PRESENT_8BIT;
      PSENDX (PL, loop_req);

      if(rr_data->tch_loop_subch EQ TCH_LOOP_C)
      {
        /* only TCH_LOOP_C is acknowledged */
        PALLOC_SDU (data_req, DL_DATA_REQ, 3*BITS_PER_BYTE);

        /*
         * set channel type and SAPI for answer
         */
        dat_code_prr_channel (&data_req->ch_type,
                              &data_req->sapi,
                              rr_data->sc_data.chan_desc.chan_type);

	TRACE_EVENT_P1 ( "Value of tch_loop_subch %x", rr_data->tch_loop_subch);

        /*
         * do not use CCD for the response
         */
        data_req->sdu.l_buf = 24;
        data_req->sdu.o_buf = ENCODE_OFFSET;
        /*lint -e415 -e416 Likely access of out-of-bounds pointer*/		
        data_req->sdu.buf [0] = 0;
        data_req->sdu.buf [1] = 0;
        data_req->sdu.buf [2] = 0;
        data_req->sdu.buf [3] = 0x0F;   /* TI=0, PD = TST     */
        data_req->sdu.buf [4] = 0x06;   /* MT = Open Loop Cmd */
        data_req->sdu.buf [5] = 0x81;   /* IE acknowledge     */
        /*lint +e415 +e416 Likely access of out-of-bounds pointer*/
        TRACE_EVENT ("DL_DATA_REQ (RR message)");

        EM_TCH_LOOP_OPEN;

        PSENDX (DL, data_req);
      } 
      /* tch loop "open" */
      rr_data->tch_loop_subch = NOT_PRESENT_8BIT;
    } 
    else
    {
      PFREE (dl_data_ind);
    }
  }
  else
  {
    PFREE (dl_data_ind);
  }
}

#if defined FF_EOTD
/*
+------------------------------------------------------------------------------
| Function    : rr_applic_rx_init
+------------------------------------------------------------------------------
| Description : Initialize the data structures related to
|               Application Information Transfer
|               Reference: 3GPP TS 04.18, 3.4.21.3
|
| Parameters  : The downlink (RX) part of the APDU structure.
|
+------------------------------------------------------------------------------
*/
GLOBAL void rr_applic_rx_init ( T_APPLIC_RX *applic_rx)
{
  applic_rx->state = SAI_NULL;
  if ( applic_rx->rrrrlp_data_ind NEQ NULL )
  {
    PFREE ( applic_rx->rrrrlp_data_ind )
  }
  applic_rx->rrrrlp_data_ind = NULL;
#ifdef REL99
  /* Send RRLP procedure stop indication to MM*/
  {
    PALLOC (rr_rrlp_stop_ind, RR_RRLP_STOP_IND);
    PSENDX (MM, rr_rrlp_stop_ind);
  }
#endif

  TIMERSTOP ( TAPDU );
}

/*
+------------------------------------------------------------------------------
| Function    : rr_applic_rx_msg_store
+------------------------------------------------------------------------------
| Description : Store the first segment of an APDU.
|               Reference: 3GPP TS 04.18, 3.4.21.3.2
|
| Parameters  : The first part of the APDU.
|
+------------------------------------------------------------------------------
*/
LOCAL void rr_applic_rx_msg_store ( T_B_APPLIC_INFO *b_applic_info )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX *applic_rx = &rr_data->applic_rx;
  T_apdu_data *apdu_data = &b_applic_info->apdu_data;
  UBYTE        size      = apdu_data->c_apdu_info;
  T_sdu       *sdu;
  PALLOC_SDU ( rrrrlp_data_ind, RRRRLP_DATA_IND, (USHORT)(size * BITS_PER_BYTE) );

  if ( applic_rx->rrrrlp_data_ind NEQ NULL )
  {
    TRACE_EVENT_P1 ( "APPLIC: non empty store message found", 0 );
    PFREE ( applic_rx->rrrrlp_data_ind );
  }

  applic_rx->rrrrlp_data_ind = rrrrlp_data_ind;
  sdu = &applic_rx->rrrrlp_data_ind->sdu;

  memcpy ( &sdu->buf[0], apdu_data->apdu_info, size );
  sdu->l_buf = size * BITS_PER_BYTE;
  sdu->o_buf = 0;
}

/*
+------------------------------------------------------------------------------
| Function    : rr_applic_rx_msg_append
+------------------------------------------------------------------------------
| Description : Append segments to the APDU.
|               Reference: 3GPP TS 04.18, 3.4.21.3.2
|
| Parameters  : APDU segment to be appended.
|
+------------------------------------------------------------------------------
*/

LOCAL int rr_applic_rx_msg_append ( T_B_APPLIC_INFO *b_applic_info )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX *applic_rx = &rr_data->applic_rx;
  T_apdu_data *apdu_data = &b_applic_info->apdu_data;
  T_sdu       *sdu       = &applic_rx->rrrrlp_data_ind->sdu;    /* current APDU */
  T_sdu       *sdu2;                                            /* new APDU */
  USHORT       size_cur  = (USHORT)(sdu->l_buf/BITS_PER_BYTE);  /* Current size of stored APDU */
  USHORT       size_inf  = (USHORT)apdu_data->c_apdu_info;      /* size of new APDU INFOrmation */
  USHORT       size_tot  = (USHORT)(size_cur + size_inf);       /* total APDU size after append */

  if ( size_tot <= MAX_APDU_SIZE*BITS_PER_BYTE ) /*lint !e648 !e650/ Overflow caused by alternative defines, not applicable to target*/
  {
    PALLOC_SDU ( rrrrlp_data_ind, RRRRLP_DATA_IND, (USHORT)(size_tot * BITS_PER_BYTE) );

    sdu2 = &rrrrlp_data_ind->sdu;

    memcpy ( &sdu2->buf[   0    ], &sdu->buf            [0], size_cur );
    memcpy ( &sdu2->buf[size_cur], &apdu_data->apdu_info[0], size_inf );

    sdu2->l_buf = (USHORT)(size_tot * BITS_PER_BYTE);
    sdu2->o_buf = 0;

    PFREE ( applic_rx->rrrrlp_data_ind );
    applic_rx->rrrrlp_data_ind = rrrrlp_data_ind;

    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*
+------------------------------------------------------------------------------
| Function    : rr_applic_rx_msg_send
+------------------------------------------------------------------------------
| Description : Send the re-segmented APDU to RRRRLP.
|               Reference: 3GPP TS 04.18, 3.4.21.3.2
|
| Parameters  : The C/R bit of the last APDU segment received.
|
+------------------------------------------------------------------------------
*/
LOCAL void rr_applic_rx_msg_send ( UBYTE cr )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX       *applic_rx       = &rr_data->applic_rx;
  T_RRRRLP_DATA_IND *rrrrlp_data_ind = applic_rx->rrrrlp_data_ind;

  rrrrlp_data_ind->cr = cr;
  PSENDX ( RRLP, rrrrlp_data_ind );
  applic_rx->rrrrlp_data_ind = NULL;
  applic_rx->state = SAI_NULL;
  TIMERSTOP ( TAPDU );
}

/*
+------------------------------------------------------------------------------
| Function    : dat_for_applic_info_rrlp_rx_null
+------------------------------------------------------------------------------
| Description : Received the first segment of an APDU.
|               Reference: 3GPP TS 04.18, 3.4.21.3.2
|
| Parameters  : b_applic_info: The first segment of an APDU.
|               seg: the combinbed APDU control flags
|
+------------------------------------------------------------------------------
*/
LOCAL void dat_for_applic_info_rrlp_rx_null ( T_B_APPLIC_INFO * b_applic_info, UBYTE seg )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX *applic_rx = &rr_data->applic_rx;

  TRACE_ASSERT ( applic_rx->rrrrlp_data_ind EQ NULL );

  switch ( seg )
  {
    case         FIRST_SEG | LAST_SEG    :
      /* Allowed, simple case. Forward segment and stay in state SAI_NULL */
      rr_applic_rx_msg_store ( b_applic_info );
#ifdef REL99
      /* Send RRLP procedure start indication to MM*/
      {
        PALLOC (rr_rrlp_start_ind, RR_RRLP_START_IND);
        PSENDX (MM, rr_rrlp_start_ind);
      }
#endif
      rr_applic_rx_msg_send ( b_applic_info->apdu_flags.c_r );
      break;

    case         FIRST_SEG | NOT_LAST_SEG:
      /* Allowed, standard case of APDU segmentation. */
      /* Check length of this segment -> L2 frame must be 251 bytes,
         otherwise protocol error as described in 3GPP 04.18, section 3.4.21.3.3 a) */

      if ( b_applic_info->apdu_data.c_apdu_info EQ APDU_FULL_L2_FRAME )
      {
        /* store this segment, start de-segmentation */
        rr_applic_rx_msg_store ( b_applic_info );
        TIMERSTART ( TAPDU, 2500 /* milli seconds */ ); /* 3GPP TS 04.18, 3.4.21.3.2 */
        applic_rx->state = SAI_SEGM;
#ifdef REL99
        /* Send RRLP procedure start indication to MM*/
        {
          PALLOC (rr_rrlp_start_ind, RR_RRLP_START_IND);
          PSENDX (MM, rr_rrlp_start_ind);
        }
#endif
      }
      else
      {
        /* Protocol error occured, remain in state SAI_NULL,
           discard segment (cf 3.4.21.3.3, last clause). */
      }
      break;

    case     NOT_FIRST_SEG | LAST_SEG    :
    case     NOT_FIRST_SEG | NOT_LAST_SEG:
      /* Not allowed. Protocol error as described in 3GPP 04.18, section 3.4.21.3.3 c),
        discard segment as described in last sentence of 3.4.21.3.3 */
      break;

    default:
      TRACE_EVENT_P1 ("unexpected 'default:' seg=%d", seg );
      break;
  }
}

/*
+------------------------------------------------------------------------------
| Function    : dat_for_applic_info_rrlp_rx_segm
+------------------------------------------------------------------------------
| Description : Received second and subsequent segments of an APDU.
|               Reference: 3GPP TS 04.18, 3.4.21.3.2
|
| Parameters  : b_applic_info: A segment of an APDU.
|               seg: the combinbed APDU control flags
|
+------------------------------------------------------------------------------
*/
LOCAL void dat_for_applic_info_rrlp_rx_segm ( T_B_APPLIC_INFO * b_applic_info, UBYTE seg )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX *applic_rx = &rr_data->applic_rx;

  TRACE_ASSERT ( applic_rx->rrrrlp_data_ind NEQ NULL );

  switch ( seg )
  {
    case         FIRST_SEG | LAST_SEG    :
    case         FIRST_SEG | NOT_LAST_SEG:

      /* Abnormal case, refer to 3GPP TS 04.18, 3.4.21.3.3 b), clause 2 */
      /* Discard any partially reassembed APDU, enter state SAI_NULL */

      rr_applic_rx_init ( applic_rx );

      /* Now (re-)use the current segment, refer to 3GPP TS 04.18, 3.4.21.3.3:
         "...reprocess any received APDU or APDU segment that caused the error... " */

      dat_for_applic_info_rrlp_rx_null ( b_applic_info, seg );
      break;

    case     NOT_FIRST_SEG | LAST_SEG    :

      /* Normal case, end of re-segmentation, TAPDU stop,
         send the message to the application entity.
         Enter state SAI_NULL. */

      if ( rr_applic_rx_msg_append ( b_applic_info ) EQ FALSE )
        rr_applic_rx_init ( applic_rx );
      else
        rr_applic_rx_msg_send ( b_applic_info->apdu_flags.c_r );
      break;

    case     NOT_FIRST_SEG | NOT_LAST_SEG:

      /* Normal case, re-segmetation is still ongoing.
         If 'append' operation fails, then return to state SAI_NULL. */

      if ( rr_applic_rx_msg_append ( b_applic_info ) EQ FALSE )
        rr_applic_rx_init ( applic_rx );
      break;

    default:
      break;
  }
}


/*
+------------------------------------------------------------------------------
| Function    : dat_for_applic_info_rrlp
+------------------------------------------------------------------------------
| Description : Application Information Transfer (RX) for RRRLP
|               Reference: 3GPP TS 04.18, 3.4.21.3
|
| Parameters  : b_applic_info: Segment of an APDU.
|
+------------------------------------------------------------------------------
*/
LOCAL void dat_for_applic_info_rrlp ( T_B_APPLIC_INFO * b_applic_info )
{
  GET_INSTANCE_DATA;
  T_APPLIC_RX *applic_rx = &rr_data->applic_rx;

  UBYTE seg = ((b_applic_info->apdu_flags.f_seg << 1) |
               (b_applic_info->apdu_flags.l_seg     )   ) & 0x03;

  switch ( applic_rx->state )
  {
    case SAI_NULL :
      dat_for_applic_info_rrlp_rx_null  ( b_applic_info, seg );
      break;

    case SAI_SEGM :
      dat_for_applic_info_rrlp_rx_segm  ( b_applic_info, seg );
      break;

    default:
      break;
  }
}

/*
+------------------------------------------------------------------------------
| Function    : dat_for_applic_info
+------------------------------------------------------------------------------
| Description : Main entry point for Application Information Transfer (RX)
|               Reference: 3GPP TS 04.18, 3.4.21.3
|
| Parameters  : b_applic_info: Segment of an APDU.
|
+------------------------------------------------------------------------------
*/
GLOBAL void dat_for_applic_info ( T_B_APPLIC_INFO * b_applic_info )
{
  /*
   * handle RRLP, all other protocols are not supported
   */
  if ( b_applic_info->apdu_id.protoc_ident EQ RRLP_LCS )
  {
    dat_for_applic_info_rrlp ( b_applic_info );
  }
  else
  {
    TRACE_EVENT_P1 ( "unsupported protocol %x", b_applic_info->apdu_id.protoc_ident );
  }
}
#endif /* FF_EOTD */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_for_handover_mob_alloc        |
+--------------------------------------------------------------------+

  PURPOSE : This function generates a frequency hopping list for handover

*/

LOCAL BOOL dat_for_handover_mob_alloc(UBYTE  *mob_alloc,
                                      T_LIST  *hop_list_handover, 
                                      T_VOID_STRUCT  *mob_alloc_handover,
                                      T_DL_DATA_IND  *dl_data_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("dat_for_handover_mob_alloc()");
   /*
    * the handover command contains a mobile allocation.
    * Convert the bitmap to a list of the 1-bits in the
    * bitmap for generating a frequency hopping list.
    */
  att_bits_to_byte (mob_alloc,
                   ((T_mob_alloc *)mob_alloc_handover)->c_mac,
                   ((T_mob_alloc *)mob_alloc_handover)->mac);

   /*
    * Now create the frequency hopping list
    */
  if(!srv_create_chan_mob_alloc (&rr_data->cr_data.cd.cell_chan_desc,
                                 hop_list_handover,
                                 mob_alloc))
  {
    dat_send_handov_fail_msg(RRC_FREQ_NOT_IMPL);

    RR_EM_SET_HANDOVER_FAIL_CAUSE(RRC_FREQ_NOT_IMPL);

    PFREE (dl_data_ind);
    return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_cr_data_multirate_conf |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the multirate configuration and stores 
  it in the rr_data.

*/
LOCAL void dat_cr_data_multirate_conf(U8 v_multirate_conf, T_multirate_conf  *multirate_conf)

{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("dat_cr_data_multirate_conf()");

  if(rr_data->cr_data.ch_mode EQ CM_AMR)
  {
    if (v_multirate_conf)
    {
      /*
       *  store a new multi-rate speech codec if available.
       */
      UBYTE i;

      rr_data->cr_data.amr_conf.mr_vers = multirate_conf->mr_vers;
      rr_data->cr_data.amr_conf.nscb    = multirate_conf->nscb;
      rr_data->cr_data.amr_conf.icmi    = multirate_conf->icmi;
      rr_data->cr_data.amr_conf.st_mode = multirate_conf->st_mode;
      rr_data->cr_data.amr_conf.set_amr = multirate_conf->set_amr;

      /*
       * valid flag for the threshold and hystersis values.
       * multirate_conf.c_cod_prop
       * defines the number of threshold and hystersis values.
       */
      rr_data->cr_data.amr_conf.v_cod_prop = multirate_conf->v_cod_prop;

      if(rr_data->cr_data.amr_conf.v_cod_prop)
      {
        rr_data->cr_data.amr_conf.c_cod_prop   = multirate_conf->c_cod_prop;
        
        for (i=0; i< multirate_conf->c_cod_prop; i++)
             memcpy(&rr_data->cr_data.amr_conf.cod_prop[i], &multirate_conf->cod_prop[i], sizeof(T_cod_prop));
      } /* if(rr_data->cr_data.amr_conf.v_cod_prop) */
    } /* if (assign_cmd->v_multirate_conf) */
  } /* if (rr_data->cr_data.ch_mode EQ CM_AMR) */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_class_chng_data_req    |
+--------------------------------------------------------------------+

  PURPOSE : This function forms a peer CLASSMARK CHANGE request 

*/
GLOBAL void dat_class_chng_data_req(void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION("dat_class_chng_data_req()");
  /*
   * The syntax check indicates no problems, then
   * process the message.
   *
   * The MS returns a classmark change message.
   */
  {
    MCAST (class_chng, U_CLASS_CHNG);/* T_U_CLASS_CHNG */
    PALLOC_MSG (dl_data_req, DL_DATA_REQ, U_CLASS_CHNG);

    /*
     * set channel type and sapi
     */
    dat_code_prr_channel (&dl_data_req->ch_type,
                          &dl_data_req->sapi, rr_data->sc_data.chan_desc.chan_type);

    class_chng->msg_type                = U_CLASS_CHNG;
    class_chng->mob_class_2             = rr_data->ms_data.classmark2;
    class_chng->mob_class_2.rf_pow_cap  = att_get_power ();
    class_chng->mob_class_3             = rr_data->ms_data.classmark3;
    class_chng->v_mob_class_3           = rr_data->ms_data.classmark2.class3;

    for_dat_data_req (dl_data_req);
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_DAT                     |
| STATE   : code                ROUTINE : dat_dedicated_req_ch_type2 |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the channle type from the channel 
  description IE .

*/
LOCAL void dat_dedicated_req_ch_type2(T_ch_type2 *ch_type2,  T_chan_desc_before  *chan_desc_before,
                                      T_LIST  *hop_list_before)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION("dat_dedicated_req_ch_type2()");

  ch_type2->ch   = chan_desc_before->chan_type;
  ch_type2->tn   = chan_desc_before->tn;
  ch_type2->tsc  = chan_desc_before->tsc;
  ch_type2->h    = chan_desc_before->hop;
  if(ch_type2->h EQ H_NO)
    ch_type2->arfcn = chan_desc_before->arfcn;
  else
  {
    ch_type2->maio  = chan_desc_before->maio;
    ch_type2->hsn   = chan_desc_before->hsn;

    /* CSI-LLD section:4.1.1.11
     * This function Updates the black list with the MA list received
     * in the assignment command  
     */   
    cs_remove_BA_MA_from_black_list(rr_data->cs_data.region,hop_list_before);

    srv_create_list (hop_list_before, ch_type2->ma,
                     MAX_MA_CHANNELS, TRUE, 0);
  }
}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+------------------------------------------------------------------------------
| Function    : dat_for_meas_inf
+------------------------------------------------------------------------------
| Description : Processing of measurement information message is done in this function.
|               All possible errors, if present, are detected and an error free MI-message
|               instance is decoded and data base updated with the enhanced measurement parameters.
| Parameters  : MI-message pointer
|
+------------------------------------------------------------------------------
*/
GLOBAL BOOL dat_for_meas_inf (T_D_MEAS_INF *p_mi)
{
  GET_INSTANCE_DATA;
  T_rr_enh_para     *p_cur = &rr_data->sc_data.emr_data_current;
  T_rr_enh_para     *p_temp = &rr_data->sc_data.emr_data_temp;    
  BOOL              send_enh_para = FALSE;  
  T_gprs_rep_prio   *p_rep = NULL;
  T_gprs_bsic       *p_bl = NULL;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  UBYTE             i,j;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  /* Step 1: Check if we received right BA_IND */
  if( (rr_data->sc_data.ba_list_ded EQ TRUE) AND
      (p_mi->ba_ind NEQ rr_data->sc_data.ba_index ) )
  {
    rr_data->sc_data.ba_list_ded = FALSE;
    rr_data->sc_data.ba_index = p_mi->ba_ind ;
    srv_clear_list (&rr_data->sc_data.cd.ncell_list);
#ifdef TI_PS_FF_REL4 
    srv_clear_list (&rr_data->sc_data.cd.multiband_ncell_list);
#else
    srv_clear_list (&rr_data->sc_data.five_ter_list);
#endif    
    att_clean_buf (IND_ALL_DEDI_SI);
    rr_data->sc_data.cd.sys_info_read &= ~ALL_DEDI_SYS_INFOS;
    TRACE_EVENT("Flushed off the entire dedicated mode BA-LIST as the BA-IND got changed");
  }  
  
  /* Step 2: Check report type. 
     IMPORTANT ASSUMPTION: We will not process the other parameters if report type is Normal*/
  if( p_mi->report_type NEQ ENHANCED_MEAS ) 
  {
    /*check whether there are enhanced parameters and BA list, already.
      If present then it means that report type is changing from
      Enhanced to Normal*/
    if ( p_cur->is_data_valid EQ TRUE  )
    {
      for_set_default_emr_data(p_cur);
      return TRUE; /*send enh para update to indicate change in report type*/
    }
    else
      return send_enh_para;
  }     
    
  /* Step 3: Check if we already have enh_para in current or temp
     and if there is change in parameters  or continuation of reception*/
  if(p_temp->is_data_valid EQ FALSE )  
  {
    /*This means we were not in the process of receiving. Check whether there
      is already information in current and if so, is there change in mp_change_mark*/
    if( (p_cur->is_data_valid EQ TRUE ) AND
        (p_cur->mp_change_mark EQ p_mi->mp_cm ) )
    {
      TRACE_EVENT("No change in Enhanced measurement parameters -ignore ");
      return send_enh_para;      
    }   
    /* This means there's either a change in MP change mark or receiving EMP for first time */    
    /* Decode rest of the parameters*/      
    p_temp->is_data_valid = TRUE;  
    rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;    
    p_temp->enh_para.ncc_permitted = rr_data->sc_data.cd.ncc_permitted;
  }
  
  /*Note :If different values occur for the same parameter in different instances of a message, 
          the instance with the highest index shall be used (sec.3.4.1.2.1, 4.18)*/      
  if ( (p_mi->mi_idx > rr_data->sc_data.prev_highest_index ) OR
        (rr_data->sc_data.prev_highest_index EQ NOT_PRESENT_8BIT) )
  {
    p_temp->enh_para.rep_rate = p_mi->rep_rate;
    p_temp->enh_para.inv_bsic_enabled = p_mi->inv_bsic_rep; 
    p_temp->mp_change_mark = p_mi->mp_cm;
    p_temp->msg_count = p_mi->mi_c;   
    p_temp->rep_type = p_mi->report_type;
    if (p_mi->v_emp EQ TRUE ) /* This is updation of parameters other than BSIC list*/ 
    {
      dat_update_emr_rep_para(&p_mi->emp,&p_temp->enh_para);    
    }    
    rr_data->sc_data.prev_highest_index = p_mi->mi_idx;
  }
  
#if defined (TI_PS_FF_RTD) AND defined (REL99)
    if(p_mi->v_rtdd)
      dat_update_rtd_data(p_mi,p_temp);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */


  /*Get relevant parameters to pass to BSIC and report priority list handler*/
  if(p_mi->v_gprs_rep_prio EQ TRUE)
    p_rep = &p_mi->gprs_rep_prio;
  
  if( p_mi->v_gprs_bsic EQ TRUE)
    p_bl = &p_mi->gprs_bsic;

  if (for_dat_process_common_emr_data(p_rep,p_bl,p_mi->mi_idx,
    rr_data->sc_data.ba_list_ded) )
  {
    rr_data->sc_data.enh_para_status = ENH_PARA_DEDICATED;

    if ( rr_data->sc_data.ba_list_ded EQ TRUE)
      send_enh_para = TRUE;
  }    

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  if(p_mi->v_rtdd)
  {
    /* reset the temporary storage to RTD value not available */
    for(j = 0;j < MAX_NR_OF_NCELL; j++ )
    { 
      p_temp->enh_para.enh_cell_list[j].v_rtd = FALSE;
      for(i = 0;i < MAX_NUM_OF_RTD_VALUES; i++)
        p_temp->enh_para.enh_cell_list[j].rtd[i]= RTD_NOT_AVAILABLE;
    }/*for*/
  }/*if*/
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  return send_enh_para;
}
#endif

#endif

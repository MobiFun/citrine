/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_FORP
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
|  Purpose :  This Modul defines the functions for the mob. management
|             capability of the module Mobility Managemant.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_MMS_C
#define MM_MMS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_MM

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include <stdlib.h>
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
#include "cnf_mm.h"
#include "mon_mm.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "mm_em.h"

#else

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "stddefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_mm.h"
#include "mon_mm.h"
#include "vsi.h"
#include "pei.h"
#include "tok.h"
#include "mm.h"
#include "mm_em.h"

#endif

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== TEST =====================================================*/

/*==== VARIABLES ==================================================*/
/* added by TISH 0418 to write simloci to FFS */
extern T_loc_info                loc_info_ffs;
extern T_imsi_struct      imsi_in_ffs;
/* added by TISH 0418 to write simloci to FFS */

/*==== FUNCTIONS ==================================================*/

LOCAL void mm_send_rr_data_ind          (T_RR_DATA_IND        *rr_data_ind,
                                         UBYTE                 comp,
                                         T_PRIM_TYPE           snd_prim_type);
LOCAL void mm_cpy_net_name              (T_full_net_name      *net_name,
                                         T_full_name          *name,
                                         UBYTE                 v_net_name);

/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_store_tmsi              |
+--------------------------------------------------------------------+

  PURPOSE : Convert the mobile identity to the internal TMSI
            representation.

*/

LOCAL void mm_store_tmsi (const T_mob_id *moi)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_store_tmsi()");

  mm_data->reg.tmsi =
    (((ULONG)moi->tmsi.b_tmsi[0]) << 24) +
    (((ULONG)moi->tmsi.b_tmsi[1]) << 16) +
    (((ULONG)moi->tmsi.b_tmsi[2]) <<  8) +
      (ULONG)moi->tmsi.b_tmsi[3];
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_abort                   |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal ABORT.
            The abort procedure may be invoked by the network to abort any
            on-going MM connection establishment or already established MM 
            connection. The mobile station shall treat ABORT message as 
            compatible with current protocol state only if it is received 
            when at least one MM connection exists or an MM connection is 
            being established. [GSM 04.08 clause 4.3.5]

*/

GLOBAL void mm_abort (T_D_ABORT *d_abort)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_abort()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
      TIMERSTOP (T3230);
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_NW_CMD:

      mm_data->rej_cause = CAUSE_MAKE (DEFBY_STD, 
                                       ORIGSIDE_NET,
                                       MM_CAUSE,
                                       d_abort->rej_cause);

      if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
      {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
        TIMERSTOP (T3212); 
        mm_data->t3212_timeout = FALSE; 
      }

      switch (mm_data->rej_cause)
      {
        case MMCS_NETWORK_FAILURE:
          mm_mmxx_rel_ind (RC_NETWORK_FAILURE, CM_NOT_IDLE);
          break;
        case MMCS_ILLEGAL_ME:
          /*
           * "At the receipt of the ABORT message the mobile station shall
           * abort any MM connection establishment or call re-establishment
           * procedure and release all MM connections (if any).
           * If cause value #6 is received the mobile station shall
           * delete any TMSI, LAI and ciphering key sequence number stored
           * in the SIM, set the update status to ROAMING NOT ALLOWED (and
           * store it in the SIM according to section 4.1.2.2) and consider
           * the SIM invalid until switch off or the SIM is removed.
           * As a consequence the mobile station enters state MM IDLE,
           * substate NO IMSI after the release of the RR connection.
           * The mobile station shall then wait for the network to release
           * the RR connection - see section 4.5.3.1." [GSM 04.08 4.3.5]
           */
          /* Release all connections with appropriate cause */
          mm_mmxx_rel_ind (mm_data->rej_cause, CM_NOT_IDLE);
           /* Inform RR about loss of registration until power cycle */
          mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);
           /* Delete registration data and update SIM */
#ifdef REL99
          reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
          reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif
           /* Delete IMSI - consider SIM as invalid */
          mm_clear_mob_ident (&mm_data->reg.imsi_struct);
          mm_clear_reg_data ();
           /* Remember limited service cause for MMI information */
          mm_data->limited_cause = mm_data->rej_cause;
           // Debug patch >>>
          if (mm_data->mm_idle_no_imsi_marker EQ 0)
            mm_data->mm_idle_no_imsi_marker = 23;
          // Debug patch <<<

          break;
        default:
          break;
      }
      TIMERSTART (T3240, T_3240_VALUE);
      SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
      break;

    case MM_LUP_REJECTED:
    case MM_LUP_INITIATED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      mm_for_set_error (RC_MESSAGE_INCOMPAT);
      break;

    default:    
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_auth_rej                |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal AUTH_REJ.

*/

GLOBAL void mm_auth_rej (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_auth_rej()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
        /* 
         * Upon receipt of an AUTHENTICATION REJECT message, 
         * the mobile station shall set the update status in the SIM to
         * U2 ROAMING NOT ALLOWED, delete from the SIM the stored TMSI, 
         * LAI and ciphering key sequence number.
         * The SIM shall be considered as invalid until switching off 
         * or the SIM is removed. 
         * If the AUTHENTICATION REJECT message is received in the state
         * IMSI DETACH INITIATED the mobile station shall follow
         * section 4.3.4.3. [Means: Ignore it].
         * If the AUTHENTICATION REJECT message is received in any other
         * state the mobile station shall abort any MM specific, MM connection
         * establishment or call re-establishment procedure, stop any of the 
         * timers T3210 or T3230 (if running), release all MM connections 
         * (if any), start timer T3240 and enter the state WAIT FOR NETWORK
         * COMMAND, expecting the release of the RR connection. If the RR 
         * connection is not released within a given time controlled by the
         * timer T3240, the mobile station shall abort the RR connection. 
         * In both cases, either after a RR connection release triggered from
         * the network side or after a RR connection abort requested by the
         * MS-side, the MS enters state MM IDLE, substate NO IMSI. 
         * [GSM 04.08 subclause 4.3.2.5]
         */

        mm_mmxx_rel_ind (MMCS_AUTHENTICATION_REJECTED, CM_NOT_IDLE);

        /* 
         * T3212 is stopped if an AUTHENTICATION REJECT message 
         * is received [GSM 04.08 subclause 4.4.2]
         */
        TIMERSTOP (T3212); 
        mm_data->t3212_timeout = FALSE;

        TIMERSTOP (T3210);
        TIMERSTOP (T3211);
        TIMERSTOP (T3230);

        // As this will set later MM_IDLE_NO_IMSI_STATE, we will remember
        mm_data->mm_idle_no_imsi_marker = 129;

        /* 
         * Upon receipt of an AUTHENTICATION REJECT message, the mobile station 
         * shall set the update status in the SIM to U2 ROAMING NOT ALLOWED, 
         * delete from the SIM the stored TMSI, LAI and ciphering key sequence 
         * number. [GSM 04.08 subclause 4.3.2.5]
         */

        /* Inform RR about the invalidation of the SIM */
        mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);

        /* Invalidate the SIM in MM until switch off and inform the SIM */
#ifdef REL99
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
        reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif

        /* Invalidate SIM data after indirect call to reg_build_sim_update() */
        mm_clear_mob_ident (&mm_data->reg.imsi_struct);
        mm_clear_reg_data ();

        /* Remember limited service reason for MMI information */
        mm_data->limited_cause = MMCS_AUTHENTICATION_REJECTED;

        /* Delete EPLMN list */
        if (reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE))
          mm_build_rr_sync_req_cause (SYNCCS_EPLMN_LIST);

#ifdef GPRS
        /* 
         * Notify GMM about the AUTHENTICATION REJECT, not regarding whether 
         * it is active or not. For details, see GSM 04.08 subclause 4.1.1.2.
         */
        mm_mmgmm_auth_rej_ind ();
#endif
#ifdef REL99
       /*Stop timer t3241 if it is ruuning.
        *As per the spec 24.008, Timer T3241 is stopped and reset (but not started)
        *when the MM state RR CONNECTION RELEASE NOT ALLOWED is left.
        */
        TIMERSTOP(T3241);
#endif
        /* Enter state MM_WAIT_FOR_NW_CMD */
        TIMERSTART (T3240, T_3240_VALUE);
        SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);

        EM_AUTHENTICATION(EM_REJECT);
 
      break;

    case MM_LUP_REJECTED:
      {
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      }
      break;

    default:                                              
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_auth_req                |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal AUTH_REQ.

*/

GLOBAL void mm_auth_req (T_D_AUTH_REQ *auth_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_auth_req()");

  switch (GET_STATE (STATE_MM))
  {
    
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_WAIT_FOR_NW_CMD:
      {
        if ( mm_data->idle_substate EQ MM_IDLE_NO_IMSI )
        {
          mm_for_set_error(  RC_MESSAGE_TYPE_INCOMPAT );
          break;
        }

      }
      /*Fall Through*/

    case MM_LUP_INITIATED:
    case MM_PROCESS_PROMPT:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      {
        PALLOC (sim_auth_req, SIM_AUTHENTICATION_REQ);
        TIMERSTOP (T3211);
        
        if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
        {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
        }

        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;
        mm_data->reg.cksn = auth_req->ciph_key_num.key_seq;
        memcpy (sim_auth_req, auth_req,
                sizeof (T_SIM_AUTHENTICATION_REQ));
        reg_mmr_auth_ind (sim_auth_req);
        if (TIMERACTIVE(T3240))
        {
          TIMERSTOP (T3240);
          TIMERSTART (T3240, T_3240_VALUE);
        }

        EM_AUTHENTICATION(EM_REQUEST);

      }
      break;
    
    case MM_LUP_REJECTED:
    {
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;
    }
    default:                                              
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_cm_message              |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal CM_MESSAGE. Normally, this means
            that a L3 message which is for one of the CM entities 
            is forwarded to the appropriate CM entity, either as 
            MMXX_ESTABLISH_IND or MMXX_DATA_IND.
            The following can be assumed to be true:
            PD is either PD_CC, PD_SS or PD_SMS.
            The ti has already been transformed by the formatter into 
            the range 0..6 for mobile originated transactions and 
            the range 8..14 for mobile terminated transactions.

*/

GLOBAL void mm_cm_message (UBYTE     pd,
                           UBYTE     ti,
                           T_RR_DATA_IND *rr_data_ind)
{
  GET_INSTANCE_DATA;
  UBYTE        comp;

  TRACE_FUNCTION ("mm_cm_message()");

  switch (pd)
  {
    case PD_CC:
      comp = CC_COMP;
      break;
    case PD_SS:
      comp = SS_COMP;
      break;
    case PD_SMS:
      comp = SMS_COMP;
      break;
    default:
      PFREE (rr_data_ind);
      return;
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      switch (CM_GET_STATE (comp, ti))
      {
        case CM_IDLE:
          CM_SET_STATE (comp, ti, CM_ACTIVE);
          mm_send_rr_data_ind(rr_data_ind, comp, PRIM_EST_IND);
          
          TIMERSTOP (T3211);
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;
          TIMERSTOP (T3240);
#ifdef REL99
         /*Stop timer t3241 if it is ruuning.
          *As per the spec 24.008, Timer T3241 is stopped and reset (but not started)
          *when the MM state RR CONNECTION RELEASE NOT ALLOWED is left.
          */
          TIMERSTOP(T3241);
#endif

          /* 
           * T3212 is stopped if the mobile station has responded to paging
           * and thereafter has received the first correct layer 3 message 
           * except RR message. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 

          SET_STATE (STATE_MM, MM_CONN_ACTIVE);
          USE_STORED_ENTRIES();
          break;

        case CM_PENDING:
          /*
           * The connection is pending, and instead of MMXX_ESTABLISH_CNF
           * the CM entity receives MMXX_ESTABLISH_IND. No state change 
           * in the connection table is performed.
           * The special problem which was intended to solve here 
           * was to pass multilayer testcase MCC 100.
           * Problem: From which GSM 11.10 testcase is MCC 100 derived?
           * A transaction which is mobile originated has assigned ti=0..6 
           * here, a transactions which is mobile terminated has assigned
           * ti=8..14 here. 
           *  => It is impossible to have a MMXX_ESTABLISH_IND for a pending
           *     connection with a correctly working network.
           */ 
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;
          TIMERSTOP (T3240);
          mm_send_rr_data_ind(rr_data_ind, comp, PRIM_EST_IND);

          /* 
           * T3212 is stopped if the mobile station has responded to paging
           * and thereafter has received the first correct layer 3 message 
           * except RR message. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
          break; /* case CM_PENDING */

        case CM_ACTIVE:
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;
          TIMERSTOP (T3240);
          mm_send_rr_data_ind(rr_data_ind, comp, PRIM_DATA_IND);
          break; /* case CM_ACTIVE */

        default:
          {
/* Implements Measure 29 and streamline encoding */
            mm_send_status(RC_IDENTIFIY);
          }
          PFREE (rr_data_ind);
          break;
      }
      break;
    
    case MM_LUP_REJECTED: 
    case MM_LUP_INITIATED:
      {
        mm_for_set_error(RC_MESSAGE_INCOMPAT);
      }
      PFREE (rr_data_ind);
      break;

    default:
      PFREE (rr_data_ind);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_cm_serv_accept          |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal CM_SERV_ACCEPT.

*/

GLOBAL void mm_cm_serv_accept (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_cm_serv_accept()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_REESTABLISH:
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_WAIT_FOR_NW_CMD:
      if (mm_data->wait_for_accept)
      {
        mm_mmxx_est_cnf ();

        /* 
         * Ensure we are not fooled by a forgotten reset of the location 
         * updating type, so check also the service state. Should not 
         * be necessary for a perfect world.
         */
        if (mm_data->idle_substate NEQ MM_IDLE_LIMITED_SERVICE AND
            mm_data->idle_substate NEQ MM_IDLE_NO_IMSI AND
            (mm_data->loc_upd_type.lut EQ PERIODIC_LUP OR 
             mm_data->loc_upd_type.lut EQ IMSI_ATTACH_LUP))
        {        
          /* 
           * Implicit location updating accept for periodic or 
           * IMSI attach location updating request.
           * This cannot be found in GSM 04.08, 
           * but otherwise GSM 11.10 26.7.4.3.4 would fail.
           * For details, see GSM 11.10 subclause 26.7.4.3.4.1
           * 1.) and 2.)
           */
          mm_data->loc_upd_type.lut = NOT_RUNNING;

          if (mm_data->first_attach)
          {
            mm_data->first_attach_mem = mm_data->first_attach;
            mm_data->first_attach = FALSE;
          }
#ifdef GPRS
          if (!mm_gsm_alone())
            mm_data->gprs.reg_cnf_on_idle_entry = TRUE;
#endif /* GPRS */
          reg_mm_success (FULL_SERVICE);
        }
        
        if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
        {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
        }

        TIMERSTOP (T3211);
        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;
        TIMERSTOP (T3230);
        mm_data->wait_for_accept = FALSE;
        SET_STATE (STATE_MM, MM_CONN_ACTIVE);
        
        EM_CM_SERVICE_ACCEPTED(EM_CIPHERING); 

        USE_STORED_ENTRIES();
      }
      else
      {
        /* CM_SERV_ACCEPT not expected, send MM_STATUS */
        mm_for_set_error(RC_MESSAGE_INCOMPAT);
      }
      break;

    case MM_PROCESS_PROMPT:
    case MM_LUP_REJECTED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      /* CM_SERV_ACCEPT not expected, send MM_STATUS */
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;

    default:                                              
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_cm_serv_rej             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal CM_SERV_REJ.

*/

GLOBAL void mm_cm_serv_rej (T_D_CM_SERV_REJ *cm_serv_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_cm_serv_rej()");

  /* Semantical checks and preprocessing */
  cm_serv_rej->rej_cause = for_check_reject_cause (cm_serv_rej->rej_cause);

  TRACE_EVENT_P1 ("CM_SERV_REJ cause = %d", cm_serv_rej->rej_cause);

  /* 
   * The behaviour is described in GSM 04.08 subclause 4.5.1.1
   * For further details, see this recommendation.
   */
  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_REESTABLISH:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
        if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
        {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
        }

        TIMERSTOP (T3211);
        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;

        /* T3230 stopped on reception of CM_SERV_REJ or CM_SERV_ACC */
        TIMERSTOP (T3230);

        mm_data->rej_cause = CAUSE_MAKE (DEFBY_STD,
                                         ORIGSIDE_NET,
                                         MM_CAUSE,
                                         cm_serv_rej->rej_cause);
        switch (mm_data->rej_cause)
        {
          case MMCS_IMSI_IN_VLR:
            /*
             * CM SERVICE REJECT by the network, cause #4: For some
             * reason the IMSI is not attached in the VLR. This may happen 
             * if there was a failure in the VLR's database.
             * This is expected to happen seldom.
             * GSM 11.10 subclause 26.7.5.5 doesn't expect that a call is saved 
             * and established is performed later in this condition.
	     * Choosing this
             * implementation option complicates the protocol here too much.
             * (danger of introducing bugs, especially if GPRS is also present)
             * The next thing to do is a NORMAL UPDATE.
             */

            /* 
             * If cause value #4 is received, the mobile station aborts any 
             * MM connection, deletes any TMSI, LAI and ciphering key 
             * sequence number in the SIM, changes the update status to 
             * NOT UPDATED (and stores it in the SIM according to section
             * 4.1.2.2), and enters the MM sublayer state WAIT FOR NETWORK 
             * COMMAND. If subsequently the RR connection is released or 
             * aborted, this will force the mobile station to initiate a 
             * normal location updating). Whether the CM request shall be 
             * memorized during the location updating procedure, is a
             * choice of implementation.
             * [GSM 04.08 subclause 4.5.1.1]
             */

            /* Indicate connection release to CM */
            mm_mmxx_rel_ind (mm_data->rej_cause, CM_NOT_IDLE);

            /* Invalidate update state and synchronize SIM */
#ifdef REL99
            reg_invalidate_upd_state (MS_NOT_UPDATED, FALSE);
#else
            reg_invalidate_upd_state (MS_NOT_UPDATED);
#endif

            /* Invalidate TMSI in RR and lower layers */
            mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL);

            /* 
             * Ensure that the conditions are set in a way that after release
             * of the RR connection a normal update will be performed.
             */
            mm_data->attempt_cnt = 0;
            mm_data->loc_upd_type.lut = NORMAL_LUP;

            /* Await network release in state MM_WAIT_FOR_NW_CMD */
            TIMERSTART (T3240, T_3240_VALUE);
            SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
            break;

          case MMCS_ILLEGAL_ME:
            /* 
             * If cause value #6 is received, the mobile station aborts any 
             * MM connection, deletes any TMSI, LAI and ciphering key sequence
             * number in the SIM, changes the update status to ROAMING NOT 
             * ALLOWED (and stores it in the SIM according to section 
             * 4.1.2.2), and enters the MM sublayer state WAIT FOR NETWORK
             * COMMAND. The mobile station shall consider the SIM as invalid
             * until switch-off or the SIM is removed.
             * [GSM 04.08 subclause 4.5.1.1]
             */

            /* Indicate connection release to CM */
            mm_mmxx_rel_ind (mm_data->rej_cause, CM_PENDING);

            /* Invalidate update state and synchronize SIM */
#ifdef REL99
            reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
            reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif

            /* Delete IMSI - consider SIM as invalid */
            mm_clear_mob_ident (&mm_data->reg.imsi_struct);
            mm_clear_reg_data ();

            /* Inform RR about loss of registration until power cycle */
            mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);

            /* Remember limited service cause for MMI information */
            mm_data->limited_cause = mm_data->rej_cause;

            // Debug patch >>>
            if (mm_data->mm_idle_no_imsi_marker EQ 0)
              mm_data->mm_idle_no_imsi_marker = 6;
            // Debug patch <<<

            TIMERSTART (T3240, T_3240_VALUE);
            SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
            break;

          default:
            /* 
             * Pending connections are only expected 
             * in state MM_CONN_ACTIVE if mm_data->wait_for_accept.
             */
            mm_data->wait_for_accept = FALSE;
            if ((GET_STATE (STATE_MM) EQ MM_CONN_ACTIVE) AND
                (mm_count_connections (CM_ACTIVE) EQ 0))
            {
              /*
               * Special clash case.
               * Seen in field for the SMS protocol when test engineers are
               * sending SMS like crazy to themselfes, using scripts.
               * For the CC protocol the following scenario is probably to 
               * happen, for SMS this must be something analogous:
               * The MS sent the RELEASE message via the air interface to 
               * the network. The MSC receives this RELEASE message and 
               * sends the RELEASE_COMPLETE to the MS via the BSS and the 
               * CLEAR COMMAND message to the BSC in one single transaction.
               * Now we have a pending clearance of the assigned radio 
               * resources. For details see "GSM Signalisierung", page 246.
               * If there is in this situation a CM SERVICE REQUEST is 
               * received at the MSC this cannot be accepted as the
               * clearance of the assigned radio resources are underway,
               * so the MSC answers with a CM_SERVICE_REJECT.
               * The only thing which can be done here is to repeat the 
               * CM_SERVICE_REQ besides the specification after entering
               * IDLE stateto make the protocol bullet proof.
               */
              TRACE_EVENT ("CM_SERV_REQ clashed with CLR_CMD");
              mm_write_entry (mm_data->pend_conn.comp,
                              mm_data->pend_conn.ti,
                              mm_data->pend_conn.cause,
                              EVENT_ENTRY,
                              NULL, 
                              UNSPEC);
#if defined (FF_EOTD) AND defined (REL99)
              /*
               * If there is no MM connection & rrlp is started, start the
	       * timer3241 and move MM to state MM_RR_CONN_RELEASE_NOT_ALLOWED
               */
              if(mm_data->rrlp_lcs_started EQ TRUE)
              {
                TIMERSTART(T3241,T_3241_VALUE);
                SET_STATE(STATE_MM, MM_RR_CONN_RELEASE_NOT_ALLOWED);
              }
              else
#endif /* (FF_EOTD) AND defined (REL99) */
              {
                TIMERSTART (T3240, T_3240_VALUE);
                SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
              }
            }
            else
            {
              /* 
               * Handle the CM_SERVICE_REJECT as specified in 
               * [GSM 04.08 subclause 4.5.1.1]
               */
              mm_mmxx_rel_ind (mm_data->rej_cause, CM_PENDING);
              if (mm_count_connections (CM_ACTIVE) EQ 0)
              {
#if defined (FF_EOTD) AND defined (REL99)
                /*
                 * If there is no MM connection & rrlp is started, start the
		 * timer3241 and move MM to state MM_RR_CONN_RELEASE_NOT_ALLOWED
                 */
                if(mm_data->rrlp_lcs_started EQ TRUE)
                {
                  TIMERSTART(T3241,T_3241_VALUE);
                  SET_STATE(STATE_MM, MM_RR_CONN_RELEASE_NOT_ALLOWED);
                }
                else
#endif /* (FF_EOTD) AND defined (REL99) */
                {
                  TIMERSTART (T3240, T_3240_VALUE);
                  SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
                }
              }
              USE_STORED_ENTRIES();
            }
            break;
        }

      EM_CM_SERVICE_REJECT;

      break;

    case MM_LUP_REJECTED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_INITIATED:
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;

    default: 
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_ident_req               |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal IDENT_REQ.

*/

GLOBAL void mm_ident_req (T_D_IDENT_REQ *ident_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_ident_req()");

  /* Message processing starts with semantical checks now */
  if (!for_check_identity_type (MSG(D_IDENT_REQ)->ident.ident_type))
  {
     mm_for_set_error (RC_INVALID_MAND_MESSAGE);
     return;
  }


  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      {
        MCAST (ident_res, U_IDENT_RES); /* T_U_IDENT_RES */
        if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
        {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
        }

        TIMERSTOP (T3211);
        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;
        mm_build_ident_res (ident_req->ident.ident_type, ident_res);
        for_data_req (BSIZE_U_IDENT_RES);

        EM_IDENTITY_REQUEST_RESPONSE;

        if (TIMERACTIVE(T3240))
        {
          TIMERSTOP (T3240);
          TIMERSTART (T3240, T_3240_VALUE);
        }
      }
      break;

    case MM_LUP_REJECTED:
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;
    
    default:

      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_loc_upd_acc             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal LOC_UPD_ACC.

*/

GLOBAL void mm_loc_upd_acc (T_D_LOC_UPD_ACCEPT *loc_upd_accept)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_loc_upd_acc()");

  /* Semantical checks and preprocessing */
  if (loc_upd_accept->loc_area_ident.c_mnc EQ 2)
  {
    loc_upd_accept->loc_area_ident.mnc[2] = 0xf;
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
        TRACE_EVENT ("*** LUP ACCEPTED ***");

#ifdef WIN32

        /* 
         * Check whether the simulation testcases send the 
         * LOCATION UPDATING ACCEPT message for the location area 
         * of the cell where the mobile currently is camped on.
         * Receiving a non appropritate location area in the LOCATION 
         * UPDATING ACCEPT message in the simulation environment 
         * is probably a bug in the simulation testcase.
         */
        vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                      "  ACC: MCC=%x%x%x MNC=%x%x%x LAC=%04x",
                      loc_upd_accept->loc_area_ident.mcc[0],
                      loc_upd_accept->loc_area_ident.mcc[1],
                      loc_upd_accept->loc_area_ident.mcc[2],
                      loc_upd_accept->loc_area_ident.mnc[0],
                      loc_upd_accept->loc_area_ident.mnc[1],
                      loc_upd_accept->loc_area_ident.mnc[2],
                      loc_upd_accept->loc_area_ident.lac);
        
        assert (mm_check_lai (&loc_upd_accept->loc_area_ident, 
                              &mm_data->mm.lai));

#endif /* #ifdef WIN32 */

        /* 
         * T3212 is stopped if a LOCATION UPDATING ACCEPT or 
         * LOCATION UPDATING REJECT message is received.
         * [GSM 04.08 subclause 4.4.2]
         */
        TIMERSTOP (T3212); 
        mm_data->t3212_timeout = FALSE; 

        TIMERSTOP (T3210);
        mm_data->reg.update_stat = MS_UPDATED;
        mm_data->rej_cause = 0;
        mm_data->attempt_cnt = 0;
        if (loc_upd_accept->v_mob_id NEQ 0)
        {
          switch (loc_upd_accept->mob_id.ident_type)
          {
            case TYPE_TMSI:
            {
              MCAST (tmsi_realloc_comp, U_TMSI_REALLOC_COMP);

              /* No IMSI ATTACH neccessary anymore */
              if (mm_data->first_attach)
              {
                mm_data->first_attach_mem = mm_data->first_attach;
                mm_data->first_attach = FALSE;
              }

              /* No running location updating procedure anymore */
              mm_data->loc_upd_type.lut = NOT_RUNNING;
              
              /* Save TMSI in MM registration data */
              mm_store_tmsi (&loc_upd_accept->mob_id);

              /* Send RR_DATA_REQ (TMSI_REALLOC_COMPLETE) */
              tmsi_realloc_comp->msg_type = U_TMSI_REALLOC_COMP;
              for_data_req (BSIZE_U_TMSI_REALLOC_COMP);

              EM_LOCATION_UPDATING;

              /* Structure copy */
              mm_data->reg.lai = loc_upd_accept->loc_area_ident; 

              /* 
               * We assume the network has updated us for the currently 
               * selected cell, otherwise we will run into trouble. 
               */
              assert (mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai));
                
              /* Send RR_SYNC_REQ */
              mm_build_rr_sync_req_tmsi ();

              /* Send RR_SYNC_REQ (Location Area allowed) */
              mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);

              EM_TMSI_REALLOCATION_COMPLETE;

              break;
            }
            case TYPE_IMSI:
            {
              /* No IMSI ATTACH neccessary anymore */
              if (mm_data->first_attach)
              {
                mm_data->first_attach_mem = mm_data->first_attach;
                mm_data->first_attach = FALSE;
              }

              /* No running location updating procedure anymore */
              mm_data->loc_upd_type.lut = NOT_RUNNING;

              mm_data->reg.tmsi = TMSI_INVALID_VALUE;
              mm_build_rr_sync_req_cause (SYNCCS_TMSI_INVAL);

              /* Structure copy */
              mm_data->reg.lai = loc_upd_accept->loc_area_ident; 

              /* 
               * We assume the network has updated us for the currently 
               * selected cell, otherwise we will run into trouble. 
               */
              assert (mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai));

              mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);

              EM_LOCATION_UPDATING;
   
              break;
            }
            case 2: /*TYPE_IMEI:*/
            {
	      /* Implements Measure 29 and streamline encoding */
              mm_send_status(RC_INCORRECT_MESSAGE);
              /* Implementation problem: This should be handled like 
                 LOCATION UPDATING REJECT received with cause NETWORK FAILURE.
                 The same may be true for all negative asserts in the field
                 in the whole function */
              break;
            }
            default:
              /* No IMSI ATTACH neccessary anymore */
              if (mm_data->first_attach)
              {
                mm_data->first_attach_mem = mm_data->first_attach;
                mm_data->first_attach = FALSE;
              }

              /* No running location updating procedure anymore */
              mm_data->loc_upd_type.lut = NOT_RUNNING;
              
              /* Structure copy */
              mm_data->reg.lai = loc_upd_accept->loc_area_ident; 

              /* 
               * We assume the network has updated us for the currently 
               * selected cell, otherwise we will run into trouble. 
               */
              assert (mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai));

              mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);
              break;
          }
        }
        else
        {
          /* 
           * Location updating accept without mobile ID, keep old TMSI
           * stored in MM data structures.
           */
          
          /* No IMSI ATTACH neccessary anymore */
          if (mm_data->first_attach)
          {
            mm_data->first_attach_mem = mm_data->first_attach;
            mm_data->first_attach = FALSE;
          }

          /* No running location updating procedure anymore */
          mm_data->loc_upd_type.lut = NOT_RUNNING;

          /* Structure copy */
          mm_data->reg.lai = loc_upd_accept->loc_area_ident; 

          /* 
           * We assume the network has updated us for the currently 
           * selected cell, otherwise we will run into trouble. 
           */
          assert (mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai));
          
          mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);
        }
        
        /* remove from forbidden PLMN list if stored */
        reg_plmn_bad_del (mm_data->reg.forb_plmn, 
                          MAX_FORB_PLMN_ID,
                          &mm_data->reg.actual_plmn);

        
        /* Store equivalent PLMNs (if received) */
        if (loc_upd_accept->v_eqv_plmn_list NEQ 0)
        {
          if(reg_store_eqv_plmns(&loc_upd_accept->eqv_plmn_list, &mm_data->reg.actual_plmn))
            mm_build_rr_sync_req_cause (SYNCCS_EPLMN_LIST);
        }

        /* Copy actual BCCH data before SIM update */
        memcpy (mm_data->reg.bcch, mm_data->mm.bcch, SIZE_BCCH);

        /* Indicate successfull end of registration to GMM/MMI */
#ifdef  GPRS
        if (!mm_gsm_alone())
          mm_data->gprs.reg_cnf_on_idle_entry = TRUE;
#endif
        /* Inform MMI about full service condition (with side effects) */ 
        reg_mm_success (FULL_SERVICE);

        /* Update all EFs on SIM */
        mm_data->ef_indicator = 0xFF;
        /* Update SIM */
        reg_build_sim_update ();
/* added by TISH 0418 to write simloci to FFS */
        mm_write_simloci_to_ffs();
        mm_write_imsi_to_ffs();
/* added by TISH 0418 to write simloci to FFS */

#if defined (WIN32)
        {
        TRACE_EVENT_P1 ("Follow On decoded = %d", loc_upd_accept->v_follow_proceed);
        }
#endif /* #if defined (WIN32) */


        if (loc_upd_accept->v_follow_proceed NEQ 0 AND
            mm_set_follow_on_request())
        {
          SET_STATE (STATE_MM, MM_CONN_ACTIVE);
          mm_data->loc_upd_type.follow = FOR_PENDING_NO;
          mm_data->wait_for_accept     = FALSE;
          TIMERSTART (T3230, T_3230_VALUE);
          TRACE_FUNCTION ("mm_loc_upd_acc () follow on - use_entry");
          USE_STORED_ENTRIES();
        }
        else
        {
          mm_data->loc_upd_type.follow = FOR_PENDING_NO;
          /* PATCH LE 02.12.99
           *
           * Don't stop connection if not follow on proceed
           *
           * mm_mmxx_rel_ind (RELCS_UNSPECIFIED, CM_PENDING);
           * mm_mmxx_rel_ind (RELCS_UNSPECIFIED, CM_NOT_IDLE);
           *
           * END PATCH LE 02.12.99
           */
          TIMERSTART (T3240, T_3240_VALUE);
          SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
        }
      break;
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
    case MM_LUP_REJECTED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    {
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;
    }
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_lup_rej                 |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal LUP_REJ. Here the reject cause is a well 
            defined MM cause and no garbage. Even if the network sent
            garbage, here this has already been corrected.

*/

GLOBAL void mm_lup_rej (T_D_LOC_UPD_REJ *loc_upd_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_lup_rej()");

  /* Semantical checks and preprocessing */
  loc_upd_rej->rej_cause = for_check_reject_cause (loc_upd_rej->rej_cause);

  // Variable solely used for debugging purposes
  mm_data->debug_last_rej_cause = loc_upd_rej->rej_cause;

  /* Remember cause value for MMI information */
  mm_data->limited_cause = CAUSE_MAKE (DEFBY_STD,
                                       ORIGSIDE_NET,
                                       MM_CAUSE,
                                       loc_upd_rej->rej_cause);

  EM_LOCATION_UPDATING_REJECT;
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
        /* 
         * T3212 is stopped if a LOCATION UPDATING ACCEPT or 
         * LOCATION UPDATING REJECT message is received.
         * [GSM 04.08 subclause 4.4.2]
         */
        TIMERSTOP (T3212); 
        mm_data->t3212_timeout = FALSE; 

        TIMERSTOP (T3210);

        mm_data->rej_cause = CAUSE_MAKE (DEFBY_STD,
                                         ORIGSIDE_NET,
                                         MM_CAUSE,
                                         loc_upd_rej->rej_cause);
        
        TIMERSTART (T3240, T_3240_VALUE);
        SET_STATE (STATE_MM, MM_LUP_REJECTED);
      break;

    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
    case MM_LUP_REJECTED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    {
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;
    }
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mm_status               |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MM_STATUS.

*/

GLOBAL void mm_mm_status (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mm_status()");
  /* Semantical checks and preprocessing */
  /* MSG(B_MM_STATUS)->rej_cause = for_check_reject_cause (MSG(B_MM_STATUS)->rej_cause); nobody cares */

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
    case MM_LUP_REJECTED:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
        if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
        {
          /* 
           * T3212 is stopped if the first MM message is received, or 
           * ciphering mode setting is completed in the case of MM 
           * connection establishment, except when the most recent service 
           * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
           */
          TIMERSTOP (T3212); 
          mm_data->t3212_timeout = FALSE; 
        }

        if (TIMERACTIVE(T3240))
        {
          TIMERSTOP (T3240);
          TIMERSTART (T3240, T_3240_VALUE);
        }
      break;

    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmr_auth_cnf            |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MMR_AUTH_CNF.

*/

GLOBAL void mm_mmr_auth_cnf (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmr_auth_cnf()");

  /* Handle the response only if it is for the last sent SIM_AUTHENTICATION_REQ */
  if (mm_data->last_auth_req_id EQ sim_auth_cnf->req_id)
  {
    switch (GET_STATE (STATE_MM))
    {
      case MM_LUP_INITIATED:
      case MM_WAIT_FOR_OUTG_MM_CONN:
      case MM_CONN_ACTIVE:
      case MM_PROCESS_PROMPT:
      case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
      case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
        {
          MCAST (auth_res, U_AUTH_RES);
          if(memcmp(mm_data->reg.kc, sim_auth_cnf->kc, MAX_KC))
          {
            /* Set bit 4 in ef_indicator to indicate kc change to SIM for next SIM_MM_UPDATE_REQ */
            mm_data->ef_indicator|=(0x01 << 3);
          }
          memcpy (mm_data->reg.kc, sim_auth_cnf->kc, MAX_KC);
          mm_build_auth_res (sim_auth_cnf, auth_res);
          for_data_req (BSIZE_U_AUTH_RES);
          mm_build_rr_sync_req(MSG_MM_CIPH);
          if (TIMERACTIVE(T3240))
          {
            TIMERSTOP (T3240);
            TIMERSTART (T3240, T_3240_VALUE);
          }
      
          EM_AUTHENTICATION(EM_RESPONSE);
   
        }
        break;

      default:
        break;
    }
  /* Reset the variable since there are no more auth_rsp expected from SIM */ 
    mm_data->last_auth_req_id = NOT_PRESENT_8BIT;
  } 
  PFREE (sim_auth_cnf);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmr_nreg_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MMR_NREG_REQ.
            The cause for the deregistration attempt may be
            CS_SIM_REM, CS_POW_OFF or CS_SOFT_OFF.

*/

GLOBAL void mm_mmr_nreg_req (UBYTE nreg_cause,
                             UBYTE detach_done)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmr_nreg_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
      /* No DL is active */
      switch (nreg_cause)
      {
        case CS_POW_OFF:
        case CS_SOFT_OFF:
          mm_write_eplmn_to_ffs();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;

        case CS_SIM_REM:
          mm_write_eplmn_to_ffs();
          mm_clear_reg_data ();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;
        
#ifdef GPRS
        case CS_DISABLE: /* Remote controlled IMSI DETACH */
          mm_mmgmm_nreg_cnf (nreg_cause);
          break;
#endif /* GPRS */
        
        default:
          TRACE_ERROR (UNEXPECTED_PARAMETER);
          break;
      }
      break;

    case MM_LUP_INITIATED:
    case MM_LUP_REJECTED:
      /*
       * We do not know the answer to the running location updating procedure.
       * Therefore the update state is set to MS_NOT_UPDATED here, 
       * so MM is on the safe side if assuming it is not updated anymore.
       * This also leads to the situation that in this
       * states an IMSI DETACH will not be performed, but this is ok 
       * according to GSM 04.08 subclause 4.3.4.1: 
       * "The IMSI detach procedure may not be started if a MM specific
       * procedure is active. If possible, the IMSI detach procedure is 
       * then delayed until the MM specific procedure is finished, else 
       * the IMSI detach is omitted."
       */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_INVAL);
#ifdef REL99
      reg_invalidate_upd_state (MS_NOT_UPDATED, FALSE);
#else
      reg_invalidate_upd_state (MS_NOT_UPDATED);
#endif
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      if (mm_data->mm.mm_info.att EQ ATT_ALLOW AND
          mm_data->reg.update_stat EQ MS_UPDATED AND
          detach_done EQ MMGMM_PERFORM_DETACH)
      { 
        mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);

        TIMERSTOP (T3210);
        TIMERSTOP (T3211);
      
        /* 
         * The timer T3212 is stopped if the mobile station is deactivated
         * (i.e. equipment powered down or SIM removed.
         */
        TIMERSTOP (T3212);
        mm_data->t3212_timeout = FALSE;
      
        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;
        TIMERSTOP (T3230);
        TIMERSTOP (T3240);
#ifdef REL99
        TIMERSTOP (T3241);
#endif
        
        /* 
         * Start IMSI Detach procedure 
         */
        if (mm_normal_upd_needed() OR 
            mm_data->idle_substate EQ MM_IDLE_LIMITED_SERVICE)
        {
          TRACE_EVENT ("IMSI DETACH questionable");
        }
        mm_data->nreg_cause = nreg_cause;
        mm_create_imsi_detach_message ();
        for_data_req (BSIZE_U_IMSI_DETACH_IND);
        TIMERSTART (T3220, T_3220_VALUE);
        SET_STATE (STATE_MM, MM_IMSI_DETACH_INIT);
        break;
      }
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_REESTABLISH:
      /* 
       * No IMSI Detach procedure. 
       * See also comment for state MM_WAIT_FOR_RR_ACTIVE.
       */
      mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);
      
      TIMERSTOP (T3210);
      TIMERSTOP (T3211);
      
      /* 
       * The timer T3212 is stopped if the mobile station is deactivated
       * (i.e. equipment powered down or SIM removed.
       */
      TIMERSTOP (T3212);
      mm_data->t3212_timeout = FALSE;
      
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;
      TIMERSTOP (T3230);
      TIMERSTOP (T3240);

      switch (nreg_cause)
      {
        case CS_POW_OFF:
        case CS_SOFT_OFF:
#ifdef GPRS
        case CS_DISABLE: /* Remote controlled IMSI DETACH */
#endif /* GPRS */
          mm_data->nreg_cause = nreg_cause;
          mm_abort_connection (ABCS_NORM);

          /* 
           * Entering state MM_IMSI_DETACH_INIT is a trick.
           * We know RR will confirm the RR_ABORT_REQ by RR_RELEASE_IND, 
           * and in this state the appropriate actions will be taken then.
           */
          SET_STATE (STATE_MM, MM_IMSI_DETACH_INIT);
          break;
        
        case CS_SIM_REM:
          mm_data->nreg_cause = nreg_cause;
          mm_abort_connection (ABCS_SIM_REM);
          SET_STATE (STATE_MM, MM_IMSI_DETACH_INIT);
          break;

        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      }
      break;

    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      mm_data->nreg_cause = nreg_cause;
      break;

    case MM_WAIT_FOR_RR_ACTIVE: /* RR is searching for a cell */
      /* 
       * GSM 04.08 subclause 4.3.4.1 requires the following: 
       * "If no RR connection exists, the MM sublayer within the mobile
       * station will request the RR sublayer to establish a RR
       * connection. If establishment of the RR connection is not possible
       * because a suitable cell is not (or not yet) available then, the 
       * mobile station shall try for a period of at least 5 seconds and for
       * not more than a period of 20 seconds to find a suitable cell. If a 
       * suitable cell is found during this time then, the mobile station shall 
       * request the RR sublayer to establish an RR connection, otherwise the
       * IMSI detach is aborted.
       * [Here the situation is that RR is searching for a cell and no cell is 
       * yet available, but one may be found in a period lesser than 20 seconds
       * and according to the standard an IMSI DETACH shall be performed.
       * This MM implementation is more simple here, if still searching for a
       * cell. the IMSI detach is not done. This may not cause any harm to the
       * mobile user, however, it is a minor violation of GSM 04.08.]
       */
      switch (nreg_cause)
      {
        case CS_POW_OFF:                    /* switch off mobile */
        case CS_SOFT_OFF:
          mm_mdl_rel_req ();
          mm_power_off ();                  /* deactivate lower layer */
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;

        case CS_SIM_REM:
          mm_abort_connection (ABCS_SIM_REM);
          mm_clear_reg_data ();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;

#ifdef GPRS 
        case CS_DISABLE:
          /* Remember MM may have to IMSI ATTACH if reactivated */
          mm_data->first_attach = TRUE;
 
          /* Confirm the GMM requested deregistration */
          mm_mmgmm_nreg_cnf (nreg_cause);

          /* No state change */
          break;
#endif /* GPRS */

        default: 
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      }
      break;

#ifdef GPRS
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
    case MM_IDLE_LUP_NEEDED:
#endif /* GPRS */
    case MM_IDLE_NORMAL_SERVICE:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
      TIMERSTOP (T3211);

      /* 
       * The timer T3212 is stopped if the mobile station is deactivated
       * (i.e. equipment powered down or SIM removed.
       */
      TIMERSTOP (T3212);
      mm_data->t3212_timeout = FALSE;
      
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;

      if (mm_data->mm.mm_info.att EQ ATT_ALLOW AND
          mm_data->reg.update_stat EQ MS_UPDATED AND
          detach_done EQ MMGMM_PERFORM_DETACH)
      {
        /* Start IMSI Detach procedure */

        if (mm_normal_upd_needed())
        {
          TRACE_EVENT ("IMSI DETACH questionable");
        }
        mm_data->nreg_cause = nreg_cause;
        mm_create_imsi_detach_message ();
        for_est_req (ESTCS_MOB_ORIG_CAL_BY_SS_SMS, BSIZE_U_IMSI_DETACH_IND);
        SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_DETACH);
        break;
      }
      /* 
       * No IMSI DETACH procedure
       */
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_IMSI:
      TIMERSTOP (T3211);

      /* 
       * The timer T3212 is stopped if the mobile station is deactivated
       * (i.e. equipment powered down or SIM removed.
       */
      TIMERSTOP (T3212);
      mm_data->t3212_timeout = FALSE;
      
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;
      switch (nreg_cause)
      {
        case CS_POW_OFF:
        case CS_SOFT_OFF:
          mm_mdl_rel_req ();
          mm_power_off ();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;

        case CS_SIM_REM:
          mm_mdl_rel_req();
          mm_abort_connection (ABCS_SIM_REM);
          mm_clear_reg_data ();
          // Debugging patch >>>
          if (mm_data->mm_idle_no_imsi_marker EQ 0)
            mm_data->mm_idle_no_imsi_marker = 17;
          // Debugging patch <<<
          SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
          reg_end_of_deregistration (nreg_cause, LIMITED_SERVICE);
          break;

#ifdef GPRS
        case CS_DISABLE:
          /* Remember MM may have to IMSI ATTACH if reactivated */
          mm_data->first_attach = TRUE;
 
          /* Confirm the GMM requested deregistration */
          mm_mmgmm_nreg_cnf (nreg_cause);

          /* No state change */
          break;
#endif /* GPRS */

        default: /* Not expected */
          TRACE_ERROR (UNEXPECTED_PARAMETER);
          break;
      }
      break;
    
    case MM_IDLE_NO_CELL_AVAILABLE:
      switch (nreg_cause)
      {
        case CS_POW_OFF:
        case CS_SOFT_OFF:
          mm_mdl_rel_req ();
          mm_power_off ();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          break;

        case CS_SIM_REM:
          mm_mdl_rel_req();
          mm_abort_connection (ABCS_SIM_REM);
          mm_clear_reg_data ();
          reg_end_of_deregistration (nreg_cause, NO_SERVICE);
          /* No state transition to MM_IDLE_NO_IMSI here, 
           * as state MM_IDLE_NO_CELL_AVAILABLE has precedence. */
          break;

#ifdef GPRS
        case CS_DISABLE:
          /* Remember MM may have to IMSI ATTACH if reactivated */
          mm_data->first_attach = TRUE;
 
          /* Confirm the GMM requested deregistration */
          mm_mmgmm_nreg_cnf (nreg_cause);

          /* No state transition to MM_IDLE_NO_IMSI here, 
           * as state MM_IDLE_NO_CELL_AVAILABLE has precedence. */
          break;
#endif /* GPRS */

        default: /* Not expected */
          TRACE_ERROR (UNEXPECTED_PARAMETER);
          break;
      }
      break;

    case MM_IDLE_PLMN_SEARCH:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      /* Back to IDLE state before network search was started, 
       * as deregistration will stop network search in RR. */
      SET_STATE (STATE_MM, mm_data->idle_substate);

      /* Repeat the deregistration attempt in new MM state */
      mm_mmr_nreg_req (nreg_cause, detach_done);
      return;

    default: /* Not expected as all states are handled implicitely */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmr_reg_req             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MMR_REG_REQ.

*/

GLOBAL void mm_mmr_reg_req (UBYTE func)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT ("mm_mmr_reg_req()");

  /* Check input parameter */
  assert (func EQ FUNC_LIM_SERV_ST_SRCH OR
          func EQ FUNC_PLMN_SRCH OR
          func EQ FUNC_NET_SRCH_BY_MMI);
  
  mm_data->reg.op.v_op = V_OP_PRES;
  mm_data->reg.op.func = func;

  /* (Re)Start MM's watchdog timer, next state is different from MM_NULL. */
  TIMERSTART (T_REGISTRATION, T_REG_VALUE);
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
    case MM_IDLE_NORMAL_SERVICE:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_IMSI:
    case MM_IDLE_NO_CELL_AVAILABLE:
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
#endif /* GPRS */
      if (func EQ FUNC_NET_SRCH_BY_MMI)
        mm_start_net_req ();
      else
      {
        mm_rr_act_req ();
        SET_STATE (STATE_MM, MM_WAIT_FOR_RR_ACTIVE);
      }
      break;

    case MM_IDLE_PLMN_SEARCH:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      if (func NEQ FUNC_NET_SRCH_BY_MMI)
      {
        /* Network search aborted by GMM. Inform MMI */
        mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);

        mm_rr_act_req ();
        SET_STATE (STATE_MM, MM_WAIT_FOR_RR_ACTIVE);
      }
      break;

    case MM_LUP_INITIATED:
    case MM_IMSI_DETACH_INIT:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      if (func EQ FUNC_NET_SRCH_BY_MMI)
      {
        /*  
         * If the MM state is not IDLE and this is caused by an MM specific
         * procedure, we store MMI's net request until MM becomes IDLE again.
         */
        mm_write_entry (REG_COMP, 0, 0, EVENT_ENTRY, NULL, UNSPEC);
        break;
      }
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_REESTABLISH:
      if (func EQ FUNC_NET_SRCH_BY_MMI)
      {
        mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
      }
      else
      {
        /* func is either FUNC_LIM_SERV_ST_SRCH or FUNC_PLMN_SRCH */
        mm_mmxx_rel_ind (MMCS_INT_NOT_PRESENT, CM_NOT_IDLE);
        mm_abort_connection (ABCS_NORM);
        if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
        {
          mm_rr_act_req ();
          SET_STATE (STATE_MM, MM_WAIT_FOR_RR_ACTIVE);
        }
      }
      break;

    case MM_WAIT_FOR_RR_ACTIVE:
      if (func EQ FUNC_NET_SRCH_BY_MMI)
        mm_start_net_req ();
      else
      {
        mm_rr_act_req ();
        /* State remains */
      }
      break;

    default:
      /* As all states are already handled by case, this must not happen */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_tmsi_realloc_cmd        |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TMSI_REALLOC_CMD.

*/

GLOBAL void mm_tmsi_realloc_cmd (T_D_TMSI_REALLOC_CMD *tmsi_realloc_cmd)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_tmsi_realloc_cmd()");

  /* semantical checks and preprocessing*/
  if (!for_check_mobile_identity (&MSG(D_TMSI_REALLOC_CMD)->mob_id))
  {
    mm_for_set_error (RC_INVALID_MAND_MESSAGE);
    return;
  }

  if (tmsi_realloc_cmd->loc_area_ident.c_mnc EQ 2)
  {
    tmsi_realloc_cmd->loc_area_ident.mnc[2] = 0xf;
  }
  
  {
    MCAST (tmsi_realloc_comp, U_TMSI_REALLOC_COMP);
    switch (tmsi_realloc_cmd->mob_id.ident_type)
    {
      case TYPE_TMSI:
        mm_store_tmsi (&tmsi_realloc_cmd->mob_id);
        mm_build_rr_sync_req_tmsi ();
        /* EF LOCI value has changed, hence write it on SIM */
        /* EF Indicator for EF LOCI - bit 1 - changed value from Attach Accept msg*/
        mm_data->ef_indicator|=0x01;
        break;
      case TYPE_IMSI:
        mm_data->reg.tmsi = TMSI_INVALID_VALUE;
        mm_build_rr_sync_req_cause (SYNCCS_TMSI_INVAL);
        /* EF LOCI value has changed, hence write it on SIM */
        /* EF Indicator for EF LOCI - bit 1 - changed value from Attach Accept msg*/
        mm_data->ef_indicator|=0x01;
        break;
      default:
        TRACE_EVENT ("Unexpected mobile id");
        break;
    }

      /* 
       * The mobile station shall consider the new TMSI and new LAI, 
       * if any, as valid and the old TMSI and old LAI as deleted as
       * soon as a TMSI REALLOCATION COMMAND or another message 
       * containing a new TMSI (e.g. LOCATION UPDATING ACCEPT) is 
       * correctly received." [GSM 04.08 clause 4.3.1.4].
       * It can *not* be assumed the new update state is U1 "UPDATED",
       * even if the description of the update state in GSM 04.08 4.1.2.2
       * says a TMSI can only exists in "UPDATED". GSM 04.08 4.1.2.2 says
       * that normally a TMSI etc. only exists in updated, but first the 
       * presence of other values shall not be considered as an error and
       * second this subclause states clearly that the update status shall 
       * only be changed by LUP ACCEPT and some other explicitly mentioned 
       * procedures, TMSI REALLOCATION COMMAND not beeing one of them.
       */
    if (memcmp(mm_data->reg.lai.mnc, tmsi_realloc_cmd->loc_area_ident.mnc, SIZE_MNC) 
        OR memcmp (mm_data->reg.lai.mcc, tmsi_realloc_cmd->loc_area_ident.mcc, SIZE_MCC) 
        OR (mm_data->reg.lai.lac NEQ tmsi_realloc_cmd->loc_area_ident.lac))
    {
    /* EF LOCI value has changed, hence write it on SIM */
    /* EF Indicator for EF LOCI - bit 1 */
    mm_data->ef_indicator|=0x01;
    }
    mm_data->reg.lai = tmsi_realloc_cmd->loc_area_ident; /* Struct copy */
    tmsi_realloc_comp->msg_type = U_TMSI_REALLOC_COMP;
    for_data_req (BSIZE_U_TMSI_REALLOC_COMP);

    EM_TMSI_REALLOCATION_COMPLETE;

    TIMERSTOP (T3212);
    mm_data->t3212_timeout = FALSE;

    if (TIMERACTIVE(T3240))
    {
      TIMERSTOP (T3240);
      TIMERSTART (T3240, T_3240_VALUE);
    }
    reg_build_sim_update ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_cm_service_prompt       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal CM_SERVICE_PROMPT. Only called from 
            for_rr_data_ind if cmsp in classmark 2 is set.

*/

GLOBAL void mm_cm_service_prompt (T_D_CM_SERVICE_PROMPT *cm_service_prompt)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_cm_service_prompt()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED: /* MM specific procedure, reject */
    case MM_LUP_REJECTED:  /* MM specific procedure, reject */
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_OUTG_MM_CONN:
      mm_for_set_error(RC_MESSAGE_INCOMPAT);
      break;

    case MM_CONN_ACTIVE:
      if (mm_data->wait_for_accept)
      {
        /* This is state WAIT_FOR_ADD_OUTGOING_MM_CONN */
        mm_for_set_error(RC_MESSAGE_INCOMPAT);
      }
      else
      {
        /* This is really MM_CONN_ACTIVE */
        if ((cm_service_prompt->pd_and_sapi.pd   EQ PD_CC) AND
            (cm_service_prompt->pd_and_sapi.sapi EQ SAPI_0))
        {
          /* Send MMCM_PROMPT_IND to CC */
          PALLOC (prompt_ind, MMCM_PROMPT_IND);
          PSENDX (CC, prompt_ind);
          SET_STATE (STATE_MM, MM_PROCESS_PROMPT);
        }
        else
        {
          /* Send MM_STATUS until CCBS fully supported by ACI and MMI */
          mm_for_set_error(RC_SERVICE_NOT_SUPPORTED);
        }
      }
      break;

    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
      {
        /* 
         * T3212 is stopped if the first MM message is received, or 
         * ciphering mode setting is completed in the case of MM 
         * connection establishment, except when the most recent service 
         * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
         */
        TIMERSTOP (T3212); 
        mm_data->t3212_timeout = FALSE; 
      }
      
      if ((cm_service_prompt->pd_and_sapi.pd   EQ PD_CC) AND
          (cm_service_prompt->pd_and_sapi.sapi EQ SAPI_0))
      {
#ifdef REL99
        /* if timer T3240 is active, Restart T3240 */
        if(TIMERACTIVE(T3240))
#endif
        {
          TIMERSTART (T3240, T_3240_VALUE);
        }
        /* Send MMCM_PROMPT_IND to CC */
        {
          PALLOC (prompt_ind, MMCM_PROMPT_IND); /* T_MMCM_PROMPT_IND */
          PSENDX (CC, prompt_ind);
        }
#ifdef REL99
        /*
         * Stop timer t3241 if it is running. *As per the spec 24.008, Timer
         * T3241 is stopped and reset (but not started) when the MM state
         * RR CONNECTION RELEASE NOT ALLOWED is left.
         */
        TIMERSTOP(T3241);
#endif

        SET_STATE (STATE_MM, MM_PROCESS_PROMPT);
      }
      else
      {
        /* Send MM_STATUS message, only CC and SAPI=0 supported */
        mm_for_set_error(RC_SERVICE_NOT_SUPPORTED);
      }
      break;

    default: /* States without RR connection */
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mm_information          |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MM_INFORMATION. This signal may be
            received any time a RR connection to the network exists
            and is always been considered as compatible with the
            protocol state.
*/

GLOBAL void mm_mm_information (T_D_MM_INFORMATION *mm_information)
{
  GET_INSTANCE_DATA;

#ifdef GPRS
  PALLOC (mmr_info_ind,MMGMM_INFO_IND);
#else
  PALLOC (mmr_info_ind,MMR_INFO_IND);
#endif /* GPRS */

  TRACE_FUNCTION ("mm_mm_information()");

  if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
  {
    /* 
     * T3212 is stopped if the first MM message is received, or 
     * ciphering mode setting is completed in the case of MM 
     * connection establishment, except when the most recent service 
     * state is LIMITED SERVICE. [GSM 04.08 subclause 4.4.2]
     */
    TIMERSTOP (T3212); 
    mm_data->t3212_timeout = FALSE; 
  }
  
  /* Set PLMN, this will be used if network name is given */
  mmr_info_ind->plmn.v_plmn = TRUE;
  memcpy (mmr_info_ind->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
  memcpy (mmr_info_ind->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);

  /* Set full network name, if present */
  mm_cpy_net_name(&mm_information->full_net_name, &mmr_info_ind->full_name, 
                   mm_information->v_full_net_name);

  /* Set short network name, if present */
  mm_cpy_net_name(&mm_information->short_net_name, &mmr_info_ind->short_name, 
                   mm_information->v_short_net_name);

  /* Set network time zone, if present */
  if (mm_information->v_net_tz NEQ 0)
  {
    mmr_info_ind->ntz.v_tz = TRUE;
    mmr_info_ind->ntz.tz = mm_information->net_tz.tz;
  }
  else
    mmr_info_ind->ntz.v_tz = FALSE;

  /* Set network time zone and time, if present */
  if (mm_information->v_net_tz_and_time NEQ 0)
  {
    mmr_info_ind->ntz.v_tz = TRUE;
    mmr_info_ind->ntz.tz   = mm_information->net_tz_and_time.tz;
    mmr_info_ind->time.v_time = TRUE;
    mmr_info_ind->time.year   = 
      10 * mm_information->net_tz_and_time.year[0] + 
           mm_information->net_tz_and_time.year[1];
    mmr_info_ind->time.month  = 
      10 * mm_information->net_tz_and_time.month[0] + 
           mm_information->net_tz_and_time.month[1];
    mmr_info_ind->time.day    = 
      10 * mm_information->net_tz_and_time.day[0] +
           mm_information->net_tz_and_time.day[1];
    mmr_info_ind->time.hour   =
      10 * mm_information->net_tz_and_time.hour[0] +
           mm_information->net_tz_and_time.hour[1];
    mmr_info_ind->time.minute =
      10 * mm_information->net_tz_and_time.minute[0] + 
           mm_information->net_tz_and_time.minute[1];
    mmr_info_ind->time.second =
      10 * mm_information->net_tz_and_time.second[0] + 
           mm_information->net_tz_and_time.second[1];
  }
  else
  {
    mmr_info_ind->time.v_time = FALSE;
  }
#ifdef REL99
  if (mm_information->v_daylight_save_time)
  {
    mmr_info_ind->daylight_save_time =
      mm_information->daylight_save_time.save_time_value;
  }
  else
  {
    mmr_info_ind->daylight_save_time = MMR_ADJ_NO;
  }
#endif
#ifdef GPRS
  PSENDX (GMM, mmr_info_ind);
#else
  PSENDX (MMI, mmr_info_ind);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_rr_data_ind        |
+--------------------------------------------------------------------+

  PURPOSE : The function passes the RR_DATA_IND depending on the parameters
            'comp' and 'snd_prim_type' to the respective entity.
            'comp'          gives either CC or SS or SMS entity
            'snd_prim_type' gives either ESTABLISH_IND or DATA_IND
*/

LOCAL void mm_send_rr_data_ind       (T_RR_DATA_IND        *rr_data_ind,
                                      UBYTE                 comp,
                                      T_PRIM_TYPE           snd_prim_type)
{
  TRACE_FUNCTION ("mm_send_rr_data_ind()");

  switch (comp)
  {
    case CC_COMP:
    {
      if(snd_prim_type EQ PRIM_EST_IND)
      {
        PPASS (rr_data_ind, est, MMCM_ESTABLISH_IND);
        PSENDX (CC, est);
      }
      else
      {
        PPASS (rr_data_ind, data, MMCM_DATA_IND);
        PSENDX (CC, data);
      }
    }
    break;

    case SS_COMP:
    {
      if(snd_prim_type EQ PRIM_EST_IND)
      {
        PPASS (rr_data_ind, est, MMSS_ESTABLISH_IND);
        PSENDX (SS, est);
      }
      else
      {
        PPASS (rr_data_ind, data, MMSS_DATA_IND);
        PSENDX (SS, data);
      }
    }
    break;

    case SMS_COMP:
    {
      if(snd_prim_type EQ PRIM_EST_IND)
      {
        PPASS (rr_data_ind, est, MMSMS_ESTABLISH_IND);
        PSENDX (SMS, est);
      }
      else
      {
        PPASS (rr_data_ind, data, MMSMS_DATA_IND);
        PSENDX (SMS, data);
      }
    }
    break;

    default:
      PFREE (rr_data_ind);
      return;
  } /* switch (comp) */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_cpy_net_name            |
+--------------------------------------------------------------------+

  PURPOSE : The function sets the network name.
*/

LOCAL void mm_cpy_net_name       (T_full_net_name      *net_name,
                                  T_full_name          *name,
                                  UBYTE                 v_net_name)
{
  TRACE_FUNCTION ("mm_cpy_net_name()");
  if (v_net_name NEQ 0)
  {
    name->v_name     = TRUE;
    name->dcs        = net_name->cs;
    name->add_ci     = net_name->add_ci;
    name->num_spare  = net_name->num_spare;
    memset(name->text, 0, MMR_MAX_TEXT_LEN);
    name->c_text     = MINIMUM (MMR_MAX_TEXT_LEN, net_name->c_text);
    memcpy (name->text, net_name->text,name->c_text);
  }
  else
    name->v_name = FALSE;
}

#endif

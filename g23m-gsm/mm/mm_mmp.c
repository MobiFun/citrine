/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (8410)
|  Modul   :  MM_MMP
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
|             capability of the module Mobility Management.
+-----------------------------------------------------------------------------
*/

#ifndef MM_MMP_C
#define MM_MMP_C

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

/*==== FUNCTIONS ==================================================*/


LOCAL void mm_rr_abort_cell_sel_fail (T_RR_ABORT_IND  *rr_abort_ind);
LOCAL void mm_mmcm_ss_sms_data_req   (T_VOID_STRUCT   *mm_data_req);
LOCAL void mm_sim_insert_state       (void);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_est_rr_for_cm           |
+--------------------------------------------------------------------+

  PURPOSE : Establish a RR connection for any CM entity. If this is
            possible, e.g. GPRS is not active/delivered or
            CM establishment is allowed by GPRS,
            the RR_ESTABLISH_REQ is sent and state MM_WAIT_FOR_RR_CONN_MM
            is entered. Otherwise GMM is informed about the CM connection
            request and the primitive is saved for later usage, but no state
            change is done.

*/

LOCAL void mm_est_rr_for_cm (UBYTE comp, UBYTE ti, USHORT estcs)
{
  GET_INSTANCE_DATA;
  UBYTE service;

  TRACE_FUNCTION ("mm_est_rr_for_cm()");

#ifdef WIN32
    TRACE_EVENT_P3 ("  comp = %d, ti = %d, estcs = %x",
                    comp,
                    ti,
                    estcs);
#endif /* #ifdef WIN32 */

  switch (comp)
  {
    case CC_COMP:
      if (estcs EQ MMCM_ESTCS_EMERGE)
        service = EMERGENCY_SERVICE;
      else
        service = CALL_SERVICE;
      break;

    case SS_COMP:
      estcs   = ESTCS_MOB_ORIG_CAL_BY_SS_SMS; /* Override estcs */
      service = SS_SERVICE;
      break;

    case SMS_COMP:
      estcs   = ESTCS_MOB_ORIG_CAL_BY_SS_SMS; /* Override estcs */
      service = SMS_SERVICE;
      break;

    default:
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      return;
  }

  mm_data->act_retrans = INTERNAL_REDIAL;

  if (estcs EQ MMCM_ESTCS_EMERGE)
  {
    /*
     * Establish RR connection for emergency call
     */
#ifdef  GPRS
    if (mm_cm_est_allowed())
    {
#endif
      mm_data->idle_substate = GET_STATE (STATE_MM);
      mm_rr_est_req (estcs, service, ti);
      SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_MM);
#ifdef GPRS
    }
    else
    {
      mm_mmgmm_cm_emergency_ind ();
      mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
    }
#endif
  }
  else
  {
    /*
     * Establish RR connection for normal call, SS or SMS
     */
#ifdef GPRS
    if (mm_cm_est_allowed())
    {
#endif
      mm_data->idle_substate = GET_STATE (STATE_MM);
      mm_rr_est_req (estcs, service, ti);
      SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_MM);
#ifdef GPRS
    }
    else
    {
      mm_mmgmm_cm_establish_ind ();
      mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
    }
#endif
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitives
            MMCM_RELEASE_REQ, MMSS_RELEASE_REQ and MMSMS_RELEASE_REQ.

*/

LOCAL void mm_mmxx_release_req (UBYTE comp, UBYTE ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmxx_release_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_NW_CMD:
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      /*
       * RR connection requested/established. Only delete the request from CM,
       * but do not inform GMM now until connection released.
       */
      mm_delete_entry (comp, ti);
      break;

    case MM_WAIT_FOR_RR_ACTIVE:
    case MM_IDLE_NORMAL_SERVICE:        /* 19.1 */
    case MM_IDLE_ATTEMPT_TO_UPDATE:     /* 19.2 */
    case MM_IDLE_LIMITED_SERVICE:       /* 19.3 */
    case MM_IDLE_NO_IMSI:               /* 19.4 */
    case MM_IDLE_NO_CELL_AVAILABLE:     /* 19.5 */
    case MM_IDLE_PLMN_SEARCH:           /* 19.7 */
    case MM_PLMN_SEARCH_NORMAL_SERVICE: /* 19.8 */
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:            /* 19.6 */
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
#endif /* GPRS */
      mm_delete_entry (comp, ti);
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_OK);
      break;

    case MM_WAIT_FOR_OUTG_MM_CONN:
      if ((mm_data->pend_conn.comp EQ comp) AND
          (mm_data->pend_conn.ti EQ ti))
      {
        MCAST (cm_serv_abort, U_CM_SERV_ABORT);
        CM_SET_STATE (comp, mm_data->pend_conn.ti, CM_IDLE);
        cm_serv_abort->msg_type = U_CM_SERV_ABORT;
        for_data_req (BSIZE_U_CM_SERV_ABORT);

        EM_SERVICE_ABORTED;
        TIMERSTOP (T3230);

#if defined (FF_EOTD) AND defined (REL99)
        /*
         * check the flag rrlp_lcs_started value. True if rrlp is running, False
         * if no rrlp is runnig.
         */
        if(mm_data->rrlp_lcs_started EQ TRUE)
        {
          /*
           * Enter state MM_RR_CONN_RELEASE_NOT_ALLOWED only if the last
           * active MM connection has been released by the CM layer
           * and there is rrlp service on going.
           */
          TIMERSTART (T3241, T_3241_VALUE);
          SET_STATE (STATE_MM, MM_RR_CONN_RELEASE_NOT_ALLOWED);
        }
        else
#endif /* (FF_EOTD) AND defined (REL99) */
        {
          TIMERSTART (T3240, T_3240_VALUE);
          mm_data->wait_for_accept = FALSE;
          SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
        }
      }
      else
      {
        mm_delete_entry (comp, ti);
      }
      break;

    case MM_CONN_ACTIVE:
      switch (CM_GET_STATE (comp, ti))
      {
        case CM_STORE:
          mm_delete_entry (comp, ti);
          break;

        case CM_PENDING:
          mm_data->wait_for_accept = FALSE;
          CM_SET_STATE (comp, ti, CM_IDLE);
          break;

        case CM_ACTIVE:
          CM_SET_STATE (comp, ti, CM_IDLE);
          if ((mm_count_connections (CM_ACTIVE) EQ 0) AND
              !TIMERACTIVE (T3230))
          {
#if defined (FF_EOTD) AND defined (REL99)
            /*check the flag rrlp_lcs_started value. True if rrlp is running, False if no rrlp is runnig.*/
            if(mm_data->rrlp_lcs_started EQ TRUE)
            {
              /*
               * Enter state MM_RR_CONN_RELEASE_NOT_ALLOWED only if the last
               * active MM connection has been released by the CM layer
               * and there is rrlp service on going.
               */
              TIMERSTART (T3241, T_3241_VALUE);
              SET_STATE (STATE_MM, MM_RR_CONN_RELEASE_NOT_ALLOWED);
            }
            else
#endif /* (FF_EOTD) AND defined (REL99) */
            {
              /*
               * Enter state MM_WAIT_FOR_NW_CMD only if the last
               * active MM connection has been released by the CM layer
               * and there is no pending connection. Otherwise keep state.
               */
              TIMERSTART (T3240, T_3240_VALUE);
              SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
            }
          }
          break;

        default:
          break;
      }
      break;

    case MM_PROCESS_PROMPT:
      switch (CM_GET_STATE (comp, ti))
      {
        case CM_STORE:
          mm_delete_entry (comp, ti);
          break;

        case CM_PENDING:
          /*
           * In state MM_PROCESS_PROMPT there are never pending connections,
           * so these need not be handled here
           */
          TRACE_ERROR ("CM_PENDING?");
          break;

        case CM_ACTIVE:
          CM_SET_STATE (comp, ti, CM_IDLE);
          if (mm_count_connections (CM_ACTIVE) EQ 0)
          {
            /*
             * Last active connection released, but
             * PROMPT remains present. Only START T3240
             */
            TIMERSTART (T3240, T_3240_VALUE);
          }
          break;

        default:
          break;
      }
      break;

    case MM_WAIT_FOR_RR_CONN_MM:
      if (mm_data->pend_conn.comp EQ comp AND
          mm_data->pend_conn.ti EQ ti)
      {
        CM_SET_STATE (comp, mm_data->pend_conn.ti, CM_IDLE);
        mm_abort_connection (ABCS_NORM);
        TIMERSTOP (T3230);
        /* After RR_ABORT_REQ here, RR_RELEASE_IND is guaranteed by RR */
      }
      else
      {
        mm_delete_entry (comp, ti);
      }
      break;

    case MM_WAIT_FOR_REESTABLISH:
      switch (CM_GET_STATE (comp, ti))
      {
        case CM_IDLE:
        case CM_PENDING:
          break;
        case CM_STORE:
          mm_delete_entry (comp, ti);
          break;
        case CM_ACTIVE:
          /* CC will not start call reestablishment,
           * then clear connection status
           */
          CM_SET_STATE (comp, ti, CM_IDLE);
          /* this was the last answer from MM */
          if (mm_count_connections (CM_ACTIVE) EQ 0)
          {
            /* there was no connection requesting call reestablishment */
            if ( mm_count_connections (CM_REEST_PENDING) EQ 0)
            {
              /* Find IDLE state after MM connection */
              mm_release_rr_connection (MMGMM_RESUMPTION_FAILURE);
            }
            else
            {
              /*
               * RR has already signalled a cell which is
               * suitable for call reestablishment
               * This could be explained to me. Why is in the release
               * routine reestablishment performed. I never understood
               * the problem which was solved here. Implementation problem? ...
               */
              if (mm_data->reest_cell_avail)
                mm_reest (mm_data->reest_ti);
            }
          }
          break;
      }
      break;

    default:
      break;
  }
}

/*==== VARIABLES ==================================================*/
GLOBAL UBYTE _decodedMsg [MAX_MSTRUCT_LEN_MM];

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_init_mm_data            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the MM data for the module mob. management.

*/

GLOBAL void mm_init_mm_data (void)
{
  GET_INSTANCE_DATA;
  USHORT i;

  TRACE_FUNCTION ("mm_init_mm_data()");

#if defined (NEW_FRAME)
  for (i = 0; i < NUM_OF_MM_TIMERS; i++)
    mm_data->t_running[i] = FALSE;
#else
  for (i = 0; i < NUM_OF_MM_TIMERS; i++)
    mm_data->t_handle[i] = VSI_ERROR;
#endif

  mm_init ();
  reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE);
  mm_data->state[STATE_MM] = MM_NULL;
#ifdef GPRS
  mm_data->state[STATE_REG_TYPE]    = REG_GPRS_INACTIVE;
  mm_data->state[STATE_GPRS_CM_EST] = CM_GPRS_EST_OK;
#endif /* GPRS */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mdl_error_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MDL_ERROR_IND.

*/

GLOBAL void mm_mdl_error_ind (T_MDL_ERROR_IND *mdl_error_ind)
{
  TRACE_FUNCTION ("mm_mdl_error_ind()");

#if 0
  /*
   * No MDL_RELEASE_REQ is needed in DL for fatal reasons,
   * as DL already automatically releases the DL connection if such
   * a fatal reason occurs. The opposite is true: This function
   * will disturb DL operation if it sends a MDL_RELEASE_REQ
   * after receiving MDL_ERROR_IND with cause CS_T200_EXP in case
   * of a switch back to the old channel on handover failure.
   *
   * The communication path between DL and MM may have been a
   * general misdecision in design, this has still to be discussed.
   */
  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
    case MM_IDLE_NO_CELL_AVAILABLE:
      /*
       * no DL is active
       */
      break;

    default:
      /*
       * Handling depending on the error cause
       */
      switch (mdl_error_ind->cs)
      {
        case CS_T200_EXP:
        case CS_UNSOL_DM_RESP:
        case CS_UNSOL_DM_RESP_MULT_FRM:
        case CS_NR_SEQ_ERR:
          switch (mdl_error_ind->sapi)
          {
            case SAPI_0:
              mm_mdl_rel_req ();
              break;
            case SAPI_3:
              mm_mdl_rel_req_sapi_3 ();
              break;
          }
          break;
      }
      break;
  }
#endif

  PFREE (mdl_error_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_data_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_DATA_REQ.

*/

GLOBAL void mm_mmcm_data_req (T_MMCM_DATA_REQ *mmcm_data_req)
{
  TRACE_FUNCTION ("mm_mmcm_data_req()");

  mm_mmcm_ss_sms_data_req ( (T_VOID_STRUCT*)mmcm_data_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmxx_establish_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process MMCM_ESTABLISH_REQ, MMSS_ESTABLISH_REQ and
            MMSMS_ESTABLISH_REQ finally. This is a generic function
            which handles the establishment for all CM entities.

*/

GLOBAL void mm_mmxx_establish_req (UBYTE  comp,
                                   UBYTE  ti,
                                   USHORT estcs,
                                   U8     info)

{
  GET_INSTANCE_DATA;
  UBYTE service;

restart_function:
  TRACE_FUNCTION ("mm_mmxx_establish_req()");

  switch (comp)
  {
    case CC_COMP:
      if (estcs EQ MMCM_ESTCS_EMERGE)
        service = EMERGENCY_SERVICE;
      else
        service = CALL_SERVICE;
      break;

    case SS_COMP:
      service = SS_SERVICE;
      break;

    case SMS_COMP:
      service = SMS_SERVICE;
      break;

    default:
      TRACE_ERROR (UNEXPECTED_DEFAULT); /* Cannot happen */
      return; /* No action which makes sense possible here */
  }

#ifdef GPRS
  if ((mm_data->gprs.mobile_class EQ MMGMM_CLASS_CG) OR
      (mm_data->gprs.sim_physically_removed AND
      (service NEQ EMERGENCY_SERVICE)))
  {
    /*
     * 1.) No CS services with a PS only mobile,
     *     for MMGMM_CLASS_BG MM has to ask
     * 2.) SIM removal, GMM detaching and call request requiring SIM
     *     => release the call
     */
    mm_mmxx_release_ind (comp, ti, MMCS_NO_REGISTRATION);
    return;
  }
#endif /* #ifdef GPRS */

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
    case MM_IDLE_NO_CELL_AVAILABLE:
      /*
       * without coverage no calls !
       */
      mm_mmxx_release_ind (comp, ti, MMCS_NO_REGISTRATION);
      break;

    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_REESTABLISH:
    case MM_WAIT_FOR_RR_ACTIVE:
#ifdef GPRS
    case MM_LOCATION_UPDATING_PENDING:
#endif /* GPRS */
      mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
      break;
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
        /*Upon reception of CM establishment request, MM will stop the timer 3241 &
         *shall send mm_rr_data_request. Upon reception of cm service accept from
         *network MM enter in MM_CONN_ACTIVE state.
         */
        TIMERSTOP (T3241);
        mm_rr_data_req (estcs, service, ti);
        TIMERSTART (T3230, T_3230_VALUE);
        SET_STATE (STATE_MM, MM_WAIT_FOR_OUTG_MM_CONN);
        break;
#endif

    case MM_CONN_ACTIVE:
      if (mm_data->wait_for_accept)
      {
        mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
      }
      else
      {
        /*
         * There is a rare, unhandled case:
         * MM_CONN_ACTIVE and the service state is not full service,
         * we have an ongoing emergency call. In this case MM should
         * reject the establish request, but this is not critical here.
         */
        mm_rr_data_req (estcs, service, ti);
        TIMERSTART (T3230, T_3230_VALUE);
      }
      break;

    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_RR_CONN_DETACH:
#ifdef GPRS
    case MM_IMSI_DETACH_PENDING:
#endif /* GPRS */
      if (mm_data->nreg_cause EQ CS_SIM_REM AND
          estcs EQ MMCM_ESTCS_EMERGE)
      {
        mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
      }
      else
      {
        mm_mmxx_release_ind (comp, ti, MMCS_NO_REGISTRATION);
      }
      break;

    case MM_IDLE_NORMAL_SERVICE:
      mm_est_rr_for_cm (comp, ti, estcs);
      break;

#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:
      /*
       * Inform GMM about the wish to establish a RR connection
       * for CM. Either GMM will accept this and bring MM into
       * IDLE updated state or GMM will refuse this.
       * The final decision is left to GMM here.
       */
      if (estcs EQ MMCM_ESTCS_EMERGE)
      {
        /*
         * Emergency call
         */
        mm_mmxx_rel_ind (MMCS_INT_PREEM, CM_NOT_IDLE);
        mm_est_rr_for_cm (comp, ti, estcs);
      }
      else
      {
        /*
         * Normal call
         */
        if (mm_cm_est_allowed())
        {
          mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, CM_LUP_TRIGGER);
          if (info EQ UNSPEC)
          {
            SET_STATE (STATE_REG_TYPE, REG_REMOTE_CONTROLLED);
            mm_normal_loc_upd ();
          }
        }
        else
        {
          mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
          mm_mmgmm_cm_establish_ind ();
        }
      }
      break;
#endif /* GPRS */

    case MM_IDLE_ATTEMPT_TO_UPDATE:
      if (info NEQ CM_LUP_TRIGGER)
        mm_data->attempt_cnt = 0;
      if (estcs EQ MMCM_ESTCS_EMERGE)
      {
        /*
         * Emergency call
         */
        mm_mmxx_rel_ind (MMCS_INT_PREEM, CM_NOT_IDLE);
        mm_est_rr_for_cm (comp, ti, estcs);
      }
      else
      {
        /*
         * 'info' is used to check to see where this function was called. 
         * If this function was called from 'mm_mmcm_establish_req()' then
         * this is the first pass for this function and 'mm_normal_loc_upd()' 
         * must be called according to the Recs.
         * If the function was called from 'mm_use_entry()', then it means 
         * an existing CM request is stored in the MM local stack and the current
         * MM procedure is running for that entry. Subsequent calls of 'mm_use_entry()'
         * must be ignored i.e. mm_normal_loc_upd()' must *not* be called.
         */
        /*
         * Normal call
         */
        if (mm_cm_est_allowed())
        {
          mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, CM_LUP_TRIGGER);
          if (info EQ UNSPEC)
          {
#ifdef GPRS
            if (!mm_lup_allowed_by_gmm())
            {
              SET_STATE (STATE_REG_TYPE, REG_REMOTE_CONTROLLED);
            }
#endif
            mm_normal_loc_upd ();
          }
        }
        else
        {
          mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
          mm_mmgmm_cm_establish_ind ();
        }
      }
      break;

    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_IMSI:
      if (estcs EQ MMCM_ESTCS_EMERGE)
      {
        mm_est_rr_for_cm (comp, ti, estcs);
      }
      else
      {
        mm_mmxx_release_ind (comp, ti, MMCS_NO_REGISTRATION);
      }
      break;

    case MM_IDLE_PLMN_SEARCH:
      mm_write_entry (comp, ti, estcs, EVENT_ENTRY, NULL, UNSPEC);
      if (estcs EQ MMCM_ESTCS_EMERGE)
      {
        mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
        mm_rr_act_req ();
        SET_STATE (STATE_MM, MM_WAIT_FOR_RR_ACTIVE);
      }
      break;

    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      /* Indicate stop of search to the MMI */
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);

      /* Back to IDLE state before network search was started,
       * as establishment will stop network search in RR */
      SET_STATE (STATE_MM, mm_data->idle_substate);

      /* Repeat the establish attempt in new MM state */
      /* -- mm_mmxx_establish_req (comp, ti, estcs,info); -- */
      /* -- return; -- */
      /* avoid recursion, stack usage, therefore the goto. */
      goto restart_function; /*lint !e801 goto*/

    default:
      TRACE_ERROR (UNEXPECTED_DEFAULT); /* All states caught */
      break;
  }
} /* mm_mmxx_establish_req() */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_establish_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_ESTABLISH_REQ.

*/

GLOBAL void mm_mmcm_establish_req (T_MMCM_ESTABLISH_REQ *mmcm_establish_req)
{
  TRACE_FUNCTION ("mm_mmcm_establish_req()");

  /*
   * And from this point MM is not interested in prio anymore.
   * To distinguish emergency calls from normal calls estcs is used.
   */

  switch (mmcm_establish_req->org_entity)
  {
    case MMCM_ORG_ENTITY_CC:

      mm_mmxx_establish_req (CC_COMP,
                             mmcm_establish_req->ti,
                             mmcm_establish_req->estcs,
                             UNSPEC);
      break;

    case MMCM_ORG_ENTITY_SS:
      mm_mmxx_establish_req (SS_COMP, mmcm_establish_req->ti, 0,UNSPEC);
      break;

    case MMCM_ORG_ENTITY_SMS:
      mm_mmxx_establish_req (SMS_COMP, mmcm_establish_req->ti, 0,UNSPEC);
      break;

    default:
      TRACE_ERROR ("org_entity trashed");
      break;
  }

  PFREE (mmcm_establish_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mmcm_reestablish_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_REESTABLISH_REQ.

*/

GLOBAL void mm_mmcm_reestablish_req (T_MMCM_REESTABLISH_REQ *mmcm_reestablish_req)
{
  
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmcm_reestablish_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_REESTABLISH:
      /* set re-establishment transaction identifier if not already set */
      if (mm_data->reest_ti EQ NOT_PRESENT_8BIT)
        mm_data->reest_ti = mmcm_reestablish_req->ti;

      /* set connection type to reestablishment pending */
      CM_SET_STATE (CC_COMP, mmcm_reestablish_req->ti, CM_REEST_PENDING);

      /* all CM connection have answered and
       * RR has signalled a suitable cell for call reestablishment */
      if (mm_count_connections (CM_ACTIVE) EQ 0 AND mm_data->reest_cell_avail)
        mm_reest (mm_data->reest_ti);
      break;

    default:
      break;
  }
  PFREE (mmcm_reestablish_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_RELEASE_REQ.

*/

GLOBAL void mm_mmcm_release_req (T_MMCM_RELEASE_REQ *mmcm_release_req)
{
  TRACE_FUNCTION ("mm_mmcm_release_req()");

  mm_mmxx_release_req (CC_COMP, mmcm_release_req->ti);

  PFREE (mmcm_release_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmss_data_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_DATA_REQ.

*/

GLOBAL void mm_mmss_data_req (T_MMSS_DATA_REQ *mmss_data_req)
{
  TRACE_FUNCTION ("mm_mmss_data_req()");

  mm_mmcm_ss_sms_data_req ( (T_VOID_STRUCT*)mmss_data_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmss_establish_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_ESTABLISH_REQ.

*/

GLOBAL void mm_mmss_establish_req (T_MMSS_ESTABLISH_REQ *mmss_establish_req)
{
  TRACE_FUNCTION ("mm_mmss_establish_req()");

  mm_mmxx_establish_req (SS_COMP, mmss_establish_req->ti, 0, UNSPEC);

  PFREE (mmss_establish_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmss_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSS_RELEASE_REQ.

*/

GLOBAL void mm_mmss_release_req (T_MMSS_RELEASE_REQ *mmss_release_req)
{
  TRACE_FUNCTION ("mm_mmss_release_req()");

  mm_mmxx_release_req (SS_COMP, mmss_release_req->ti);

  PFREE (mmss_release_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmsms_data_req          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSMS_DATA_REQ.

*/

GLOBAL void mm_mmsms_data_req (T_MMSMS_DATA_REQ *mmsms_data_req)
{
  TRACE_FUNCTION ("mm_mmsms_data_req()");

  mm_mmcm_ss_sms_data_req ( (T_VOID_STRUCT*)mmsms_data_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmsms_establish_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSMS_ESTABLISH_REQ.

*/

GLOBAL void mm_mmsms_establish_req (T_MMSMS_ESTABLISH_REQ *mmsms_establish_req)
{
  TRACE_FUNCTION ("mm_mmsms_establish_req()");

  mm_mmxx_establish_req (SMS_COMP, mmsms_establish_req->ti, 0, UNSPEC);

  PFREE (mmsms_establish_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmsms_release_req       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMSMS_RELEASE_REQ.

*/

GLOBAL void mm_mmsms_release_req (T_MMSMS_RELEASE_REQ *mmsms_release_req)
{
  TRACE_FUNCTION ("mm_mmsms_release_req()");

  mm_mmxx_release_req (SMS_COMP, mmsms_release_req->ti);

  PFREE (mmsms_release_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_abort_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ABORT_IND.

*/

GLOBAL void mm_rr_abort_ind (T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_rr_abort_ind()");
  mm_data->t3213_restart = MAX_REST_T3213;
  
  /*
   * If MM is reading sim and waiting for the SIM_READ_CNF for the EF PLMNsel
   * /EF EFPLMNwAcT/EFOPLMNwAcT which are used to prepare pref_plmn list.
   * rr_abort_ind will be stored if plmn_avail is indicated and will be used
   * once sim_read_cnf is received.
   */
  if (mm_data->reg.sim_read_in_progress AND rr_abort_ind->plmn_avail >0)
  {
    mm_write_entry(NO_ENTRY,NO_ENTRY,NO_ENTRY,PRIMITIVE_ENTRY,rr_abort_ind, UNSPEC);
    return;
  }

#ifdef WIN32
  /* Check for correct cause value */
  switch (rr_abort_ind->cause)
  {
    case RRCS_ABORT_CEL_SEL_FAIL:
    case RRCS_ABORT_RAD_LNK_FAIL:
    case RRCS_DATA_LINK_FAIL:
    case RRCS_ABORT_PTM:
      break; 

    default:
      TRACE_ERROR("Unexpected cause value");
      assert (0); /* Stop the simulation */
      break;
  }
#endif /* #ifdef WIN32 */

  EM_RESULT_PLMN_LIST;

  TRACE_EVENT_P1 ("SERVICE = %d", rr_abort_ind->op.service);

  /* This switch statement is used to catch the case where the RR_ABORT_IND sent in state        */
  /* MM_LUP_INITIATED triggers a second RR_ABORT_IND received in state MM_IDLE_NORMAL_SERVICE    */
  /* caused by a TABORT timeout. The second RR_ABORT_IND (containing zero plmns) is subsequently */
  /* ignored. This situation occurs when running TC 26.7.4.3.4 */

  switch(GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
      mm_data->rr_abort_prior_to_tabort = TRUE;
      break;

    case MM_IDLE_NORMAL_SERVICE:
      break;

    default:
      mm_data->rr_abort_prior_to_tabort = FALSE;
      break;
  }


  if (rr_abort_ind->op.service EQ LIMITED_SERVICE)
    mm_data->rf_power = rr_abort_ind->power;

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:
      break;

    case MM_LUP_INITIATED:
      TIMERSTOP (T3210);
      mm_mdl_rel_req ();
      mm_lup_restart ();
      break;

    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      mm_mdl_rel_req ();
      TIMERSTOP (T3230);
      TIMERSTOP (T3240);
#ifdef REL99
      TIMERSTOP (T3241);
#endif
      mm_mmxx_rel_ind (rr_abort_ind->cause, CM_PENDING);

      if (mm_mmxx_err_ind (rr_abort_ind->cause) NEQ 0)
      {
        /*
         * Active connections exist (CM_ACTIVE), remember that
         * CC has not yet signalled a valid ti for call re-establishment and
         * RR has not yet indicated a cell which is able to process call
         * re-establishment. Enter MM state MM_WAIT_FOR_REESTABLISH.
         */
        mm_data->reest_ti = NOT_PRESENT_8BIT;
        mm_data->reest_cell_avail = FALSE;
        SET_STATE (STATE_MM, MM_WAIT_FOR_REESTABLISH);
      }
      else
      {
        /* No active CM connection. Find IDLE state after MM connection. */
        mm_release_rr_connection (MMGMM_RESUMPTION_FAILURE);
      }
      break;

    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      mm_mdl_rel_req ();
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
      mm_end_of_detach ();
      break;

    case MM_LUP_REJECTED:
      mm_mdl_rel_req ();
      TIMERSTOP (T3240);
      mm_loc_upd_rej ();
      break;

    case MM_WAIT_FOR_RR_CONN_LUP:
      if (rr_abort_ind->op.func EQ FUNC_NET_SRCH_BY_MMI)
      {
        /*
         * A running parallel search has been interrupted by a 
         * location updating procedure. The sequence is 
         * RR_ACTIVATE_REQ (MMI SEARCH) / RR_ACTIVATE_IND (new LA)
         * RR_ESTABLISH_REQ (LUP request) / RR_ABORT_IND (MMI SEARCH)
         * In this case we abort the pending search to the upper layers
         * and continue with our location updating procedure.
         */
        mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
      }
      else
      {
        /*
         * The RR_ABORT_IND is not indicating the end of a search for 
         * available networks.
         */
        TIMERSTOP (T3210);
        mm_mdl_rel_req ();
        if (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL)
        {
          TIMERSTOP (T3211);
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;
          mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
          mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
          switch (rr_abort_ind->op.service)
          {
            case NO_SERVICE:
              SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
              break;

            case LIMITED_SERVICE:
              SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
              break;

            default: /* eg. FULL_SERVICE and reg_rr_failure => nonsense */
              TRACE_ERROR (UNEXPECTED_DEFAULT);
              break;
          }
          reg_rr_failure (rr_abort_ind);
        }
        else
        {
          mm_lup_restart ();
        }
      }
      break;

    case MM_WAIT_FOR_RR_CONN_MM:
      if (rr_abort_ind->op.func EQ FUNC_NET_SRCH_BY_MMI)
      {
        /*
         * Handle MMXXX_NET_REQ followed by MMGXXX_PLMN_RES during network
         * running search. MMXXX_NET_REQ is leading to establishment for MM's
         * location updating, this aborts the network search in RR.
         * Don't send probably partial or empty list to the MMI and avoid
         * MMXXX_NREG_IND in this situation, as this could be interpreted
         * by GMM as end of the procedure (which it is not).
         * Instead, send simply MMXXX_PLMN_IND (MMCS_PLMN_NOT_IDLE_MODE).
         */
        mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
      }
      else
      {
        mm_mdl_rel_req ();
        if (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL)
        {
          TIMERSTOP (T3211);
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;
          if (rr_abort_ind->op.service EQ NO_SERVICE)
          {
            mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
            mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
            SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
            reg_rr_failure (rr_abort_ind);
          }
          else
          {
            if (mm_data->pend_conn.cause EQ MMCM_ESTCS_EMERGE)
            {
               /*
                * Clash RR_ESTABLISH_REQ (emergency call) /
                * RR_ABORT_IND (limited service).
                * RR dropped the RR_ESTABLISH_REQ.
                * Repeat the emergency call attempt.
                */
               mm_rr_est_req (mm_data->pend_conn.cause,
                              mm_data->pend_conn.service,
                              mm_data->pend_conn.ti);
               /* Remain in state MM_WAIT_FOR_RR_CONN_MM */
            }
            else
            {
              mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
              mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
              SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
              reg_rr_failure (rr_abort_ind);
            }
          }
        }
        else
        {
          /*
           * Is data link failure possible here? Release pending connection.
           */
          mm_mmxx_rel_ind (rr_abort_ind->cause, CM_PENDING);
          mm_release_rr_connection (MMGMM_RESUMPTION_FAILURE);
        }
      }
      break;

    case MM_WAIT_FOR_REESTABLISH:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
      mm_mdl_rel_req ();
      mm_rr_abort_cell_sel_fail( rr_abort_ind);
      break;

    case MM_IDLE_NO_IMSI:
      /*
       * During limited service without SIM card
       * the no service condition is signalled by RR.
       */
      mm_mdl_rel_req ();
      if (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL AND
          rr_abort_ind->op.service EQ NO_SERVICE)
      {
        TIMERSTOP (T3211);
        TIMERSTOP (T3213);
        mm_data->t3213_restart = 0;
        mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
        mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
        SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
        reg_rr_failure (rr_abort_ind);
      }
      break;

    case MM_IDLE_NORMAL_SERVICE:
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;
      /*Check to see if this RR_ABORT_IND is caused by a RR TABORT timeout...*/
      if((mm_data->rr_abort_prior_to_tabort EQ TRUE) && (rr_abort_ind->plmn_avail EQ 0))
      {
        mm_data->rr_abort_prior_to_tabort = FALSE;
        PFREE (rr_abort_ind);
        return;
      }

      /*lint -fallthrough */
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:
#endif /* GPRS */

      mm_mdl_rel_req ();
      mm_rr_abort_cell_sel_fail( rr_abort_ind);
      break;

    case MM_IDLE_NO_CELL_AVAILABLE:
      if (rr_abort_ind->op.service NEQ NO_SERVICE)
      {
        mm_data->reg.bcch_encode = FALSE;
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
        reg_rr_failure (rr_abort_ind);
      }
      break;

    case MM_WAIT_FOR_RR_ACTIVE:
      if (mm_data->reg.op.func EQ rr_abort_ind->op.func)
      {
        /*
         * answer to the request of MM
         */
        mm_mdl_rel_req ();
        mm_data->reg.bcch_encode = FALSE;
        if (rr_abort_ind->op.service EQ NO_SERVICE)
        {
          SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
        }
        else
        {
          SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
        }
        reg_rr_failure (rr_abort_ind);
        USE_STORED_ENTRIES();
      }
#ifndef NTRACE
      else
      {
        /* This RR_ABORT_IND is to be ignored here, trace the func */
        switch (rr_abort_ind->op.func)
        {
          case FUNC_LIM_SERV_ST_SRCH:
            TRACE_EVENT ("RR_ABORT_IND (FUNC_LIM_SERV_ST_SRCH)");
            break;

          case FUNC_PLMN_SRCH:
            TRACE_EVENT ("RR_ABORT_IND (FUNC_PLMN_SRCH)");
            break;

          case FUNC_NET_SRCH_BY_MMI:
            TRACE_EVENT ("RR_ABORT_IND (FUNC_NET_SRCH_BY_MMI)");
            break;

          default:
            TRACE_ERROR (UNEXPECTED_PARAMETER);
            break;
        }
      }
#endif /* of #ifndef NTRACE */
      break;

    case MM_IDLE_PLMN_SEARCH:
      /* Find new MM IDLE state, not searching */
      switch (rr_abort_ind->op.service)
      {
        case NO_SERVICE:
          SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
          break;

        case LIMITED_SERVICE:
          mm_sim_insert_state();
          break;
        case FULL_SERVICE:
          /* MM has to wait with full service in MM state machine
           * until it is known which cell was selected by RR. */
          mm_sim_insert_state();
          break;

        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT); /* Something has been garbled */
          break;
      }
      /* Build and send the list */  
      reg_net_list (rr_abort_ind);               
      
      /* State change has occurred => use stored primitives */
      USE_STORED_ENTRIES();
      break;

    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      /* This state implies that a SIM is present */

      /* Find new MM IDLE state, not searching */
      switch (rr_abort_ind->op.service)
      {
        case NO_SERVICE:
          SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
          reg_net_list (rr_abort_ind); /* Build and send the list */
          USE_STORED_ENTRIES();
          break;

        case LIMITED_SERVICE:
          SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
          reg_net_list (rr_abort_ind); /* Build and send the list */
          USE_STORED_ENTRIES();
          break;

        case FULL_SERVICE:
          /*
           * Back to old full service IDLE state, maybe either
           * MM_IDLE_NORMAL_SERVICE, MM_IDLE_ATTEMPT_TO_UPDATE,
           * or MM_IDLE_LUP_NEEDED.
           */

          SET_STATE(STATE_MM,mm_data->idle_substate);

          if (rr_abort_ind->cause EQ RRCS_ABORT_PTM)
          {
            if(mm_data->net_search_count < 2)
            {
              /*
               * Since GPRS is in PTM there seems to be no need to inform
               * ACI of the Network list
               * Also,MM  should start with 10 secs timer and on the expiry of 
               * this timer should initiate the plmn scan search again
               */
              TIMERSTART(T_HPLMN,10000);
              mm_data->net_search_count++;
            }
            else
            {
	      /*
               * The user initiated network search should be repeated only twice
               * and empty PLMN list should be returned. But if the search was 
               * other than user initiated network scan allow repetions till
	       * PLMN search is a success. This should be tried every 2 mins.
               */ 
              if ( mm_data->plmn_scan_mmi )
                reg_net_list (rr_abort_ind);
              else
            TIMERSTART(T_HPLMN,120000);
          }
          }
          else
          {
            mm_data->net_search_count = 0;/*Network Scan successful,reset couter*/
            reg_net_list (rr_abort_ind); /* Build and send the list */
          }

          if (GET_STATE (STATE_MM) NEQ MM_WAIT_FOR_RR_ACTIVE)
          {
            /*
             * reg_net_list () didn't select a different PLMN.
             * Check whether MM needs a location update procedure.
             */
            if (mm_data->t3212_timeout AND
                mm_data->loc_upd_type.lut EQ NOT_RUNNING)
            {
              /*
               * T3212 expired during MM_PLMN_SEARCH_NORMAL_SERVICE.
               * Repeat the timeout event now as we are back to IDLE.
               */
              mm_data->t3212_timeout = FALSE;
              tim_t3212 ();
            }
            else
            {
              /*
               * Check whether T3211 and T3213 are inactive, if so,
               * continue an already running update now (if any).
               */
              if (!TIMERACTIVE (T3211) AND !TIMERACTIVE (T3213))
              {
                mm_continue_running_update ();
              }
            }
          }
          break;

        default:
          TRACE_ERROR (UNEXPECTED_DEFAULT); /* Something has been garbled */
          break;
      }
      break;

#ifdef GPRS
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      /*
       * This state transitions here should be discussed with ANS,
       * maybe MM is doing the wrong thing here.
       */
      if (rr_abort_ind->op.func NEQ FUNC_NET_SRCH_BY_MMI)
      {
        mm_mdl_rel_req ();

        /* Throw out all pending connections */
        mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
        mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);

        TRACE_EVENT ("State transition may be subject of discussion");

        /* Enter the appropriate IDLE state for the offered service */
        if (rr_abort_ind->op.service EQ NO_SERVICE)
        {
          SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
        }
        else
        {
          mm_sim_insert_state();
        }
        reg_rr_failure (rr_abort_ind);
      }
      break;
#endif /* GPRS */

    default:
      /* All states caught by case statements, this is impossible */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
  PFREE (rr_abort_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_limited_from_rr         |
+--------------------------------------------------------------------+

  PURPOSE : This function takes the appropriate actions if a limited
            service cell is offered by RR.

*/

LOCAL void mm_limited_from_rr (T_RR_ACTIVATE_CNF *rr_activate_cnf)
{
  TRACE_FUNCTION ("mm_limited_from_rr()");

  mm_copy_rr_act_cnf_data (rr_activate_cnf);

  reg_mm_success (LIMITED_SERVICE);

  /* Inform GPRS about selected cell */
  mm_mmgmm_activate_ind (MMGMM_LIMITED_SERVICE);

  mm_sim_insert_state();
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_rr_activate_cnf                 |
+----------------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ACTIVATE_CNF.

*/

GLOBAL void mm_rr_activate_cnf (T_RR_ACTIVATE_CNF *rr_activate_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_rr_activate_cnf()");

#ifdef WIN32
  vsi_o_ttrace (VSI_CALLER TC_FUNC,
                "  MCC=%x%x%x MNC=%x%x%x LAC=%04X CID=%04X POW=%d",
                rr_activate_cnf->plmn.mcc[0],
                rr_activate_cnf->plmn.mcc[1],
                rr_activate_cnf->plmn.mcc[2],
                rr_activate_cnf->plmn.mnc[0],
                rr_activate_cnf->plmn.mnc[1],
                rr_activate_cnf->plmn.mnc[2],
                rr_activate_cnf->lac,
                rr_activate_cnf->cid,
                rr_activate_cnf->power);
#endif /* #ifdef WIN32 */
  /* Changes for Boot Time Speedup. MM will get RR_ACTIVATE_CNF with op.func = FUNC_ST_PWR_SCAN.
   * No need to process it as this is response to dummy request. 
   * MM need to send REG_CNF indicating PWR_SCAN_START 
   */
  if (mm_data->reg.op.func EQ FUNC_ST_PWR_SCAN)
  {
    if (rr_activate_cnf->op.func EQ FUNC_ST_PWR_SCAN)
    {
      mm_send_mmgmm_reg_cnf (PWR_SCAN_START);
    }
    else
    {
      TRACE_EVENT_P1 ("Expected function FUNC_ST_PWR_SCAN but received %x", rr_activate_cnf->op.func);
    }
    return ;
  }
  mm_data->rf_power = rr_activate_cnf->power;
  mm_data->reg.new_cell_ind = TRUE;

  if (reg_plmn_equal_eqv (&rr_activate_cnf->plmn, &mm_data->reg.actual_plmn))
  {
      mm_data->reg.actual_plmn = rr_activate_cnf->plmn; /* Struct copy */
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_RR_ACTIVE:
      if (mm_data->reg.op.func EQ rr_activate_cnf->op.func)
      {
        /*
         * this is the answer to the request of MM
         */
        switch (rr_activate_cnf->op.service)
        {
          case LIMITED_SERVICE:
            mm_limited_from_rr (rr_activate_cnf);
            USE_STORED_ENTRIES();
            break;

          case FULL_SERVICE:
            /*
             * full service is indicated by RR.
             */
            mm_copy_rr_act_cnf_data (rr_activate_cnf);

            if (!mm_normal_upd_needed() AND
                !mm_attach_upd_needed() AND
                !mm_periodic_upd_needed())
            {
              /*
               * No location updating needed
               */
              TIMERSTOP (T3213);
              mm_data->t3213_restart = 0;

              /* Track possible change of T3212 */
              mm_change_t3212 ();

              reg_mm_success (FULL_SERVICE);
              reg_build_sim_update (); /* Update cell id */
              mm_mmgmm_activate_ind (MMGMM_FULL_SERVICE);

              /* Back to MM_IDLE_NORMAL_SERVICE */
              mm_data->idle_entry = RRCS_INT_NOT_PRESENT;
              /* Remember MM doesn't need any IMSI ATTACH anymore */
              if (mm_lup_allowed_by_gmm() AND mm_data->first_attach )
              {
                mm_data->first_attach_mem = mm_data->first_attach;
                mm_data->first_attach = FALSE;
              }
              
              mm_data->t3212_timeout = FALSE;
              mm_data->loc_upd_type.lut = NOT_RUNNING;
              SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
              /* Check HPLMN timer state */
              reg_check_hplmn_tim (mm_data->reg.thplmn);
              USE_STORED_ENTRIES();
            }
            else
            {
              /*
               * Location updating needed
               */
              reg_mm_cell_selected ();

              mm_data->attempt_cnt = 0;

              if (mm_normal_upd_needed())
              {
                /*
                 * If updating is allowed by GMM, start procedure,
                 * otherwise enter state MM_IDLE_LUP_NEEDED.
                 */
                mm_normal_loc_upd ();
              }
              else if (mm_attach_upd_needed())
              {
                /*
                 * If updating is allowed by GMM, start procedure,
                 * otherwise enter state MM_IDLE_LUP_NEEDED.
                 */
                mm_attach_loc_upd ();
              }
              else
              {
                /*
                 * It must be a periodic location updating procedure.
                 * If updating is allowed, start procedure,
                 * otherwise enter state MM_IDLE_NORMAL_SERVICE.
                 * Compare this with GSM 04.08 subclause 4.2.3
                 */
                if (mm_lup_allowed_by_gmm()) /*lint !e774*/
                {
                  mm_periodic_loc_upd ();
                }
                else
                {
                  SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
                }
              }

              /*
               * If GPRS is active, GMM will be informed about the
               * cell selection. In this case, MM has not tried
               * to establish a RR connection for location updating
               * and the state MM_IDLE_LUP_NEEDED has already been entered.
               */
              mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
            }
            break; /* case FULL_SERVICE */

          default: /* NO_SERVICE or other garbage */
            TRACE_ERROR (UNEXPECTED_DEFAULT);
            break;
        } /* switch() */
      }
      break;

    default:
      /*
       * Normally MM should not receive an unsolicited RR_ACTIVATE_CNF,
       * but if it happens, we keep MM up to date using considering the
       * RR_ACTIVATE_CNF as RR_ACTIVATE_IND.
       * It is theoretically possible that a clash 
       *   RR_ACTIVATE_REQ -> RR_ABORT_IND -> RR_ACTIVATE_CNF 
       * happens, but in most of the cases the reception of RR_ACTIVATE_CNF
       * in another state than MM_WAIT_FOR_RR_ACTIVE is a strong hint that 
       * something went wrong in RR which should be observed carefully.
       */
      TRACE_ERROR ("RR_ACTIVATE_CNF => RR_ACTIVATE_IND");
      mm_rr_activate_ind ((T_RR_ACTIVATE_IND*)rr_activate_cnf);
      return; /* Don't free primitive twice */
  }
  PFREE (rr_activate_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_handled_forb_plmn_cell  |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether RR selected a full service
            cell which is part of the forbidden PLMN list.
            If the cell is a member of a forbidden list, the
            appropriate actions are taken, but no change
            of the MM main state is performed.

*/

LOCAL BOOL mm_handled_forb_plmn_cell (T_RR_ACTIVATE_IND *rr_activate_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_handled_forb_plmn_cell()");

  if (reg_plmn_in_list (mm_data->reg.forb_plmn,
                        MAX_FORB_PLMN_ID,
                        &rr_activate_ind->plmn))
  {
    TRACE_EVENT ("RR selected forbidden PLMN");

    mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF *)rr_activate_ind);

    reg_mm_success (LIMITED_SERVICE);

    mm_build_rr_sync_req_cause (SYNCCS_LIMITED_SERVICE);

    /*
     * GSM 04.08 subclause 4.4.4.7 doesn't say that a forbidden PLMN
     * for GSM shall also be considered as a forbidden PLMN for GPRS.
     * This means, we could have the situation that GSM has limited
     * service only, but GPRS has full network access.
     */
    mm_mmgmm_activate_ind (MMGMM_LIMITED_SERVICE);

    return TRUE; /* Cell is in forbidden list */
  }

  if (rr_activate_ind->mm_info.la EQ LA_IN_FRBD_LST_INCL)
  {
    mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF *)rr_activate_ind);

    reg_mm_success (LIMITED_SERVICE);

    mm_mmgmm_activate_ind (MMGMM_LIMITED_SERVICE);

    return TRUE; /* Cell is in forbidden list */
  }
  return FALSE; /* Cell is not in forbidden list */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_full_from_rr            |
+--------------------------------------------------------------------+

  PURPOSE : This function takes the appropriate actions if a full
            service cell is offered by RR in some MM states.

*/

LOCAL void mm_full_from_rr (T_RR_ACTIVATE_IND *rr_activate_ind)
{
  GET_INSTANCE_DATA;
  BOOL rr_changed_lai;

  TRACE_FUNCTION ("mm_full_from_rr()");

  rr_changed_lai = !mm_check_lai_from_RR (&mm_data->mm.lai,
                                          &rr_activate_ind->plmn,
                                          rr_activate_ind->lac);

  mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF *)rr_activate_ind);

  mm_data->idle_entry = RRCS_INT_NOT_PRESENT;

  if (mm_normal_upd_needed())
    mm_data->loc_upd_type.lut = NORMAL_LUP;
  else if (mm_attach_upd_needed())
    mm_data->loc_upd_type.lut = IMSI_ATTACH_LUP;
  else if (mm_periodic_upd_needed())
    mm_data->loc_upd_type.lut = PERIODIC_LUP;
  else
  {
    if (memcmp(mm_data->reg.lai.mcc, mm_data->mm.lai.mcc, SIZE_MCC) 
      OR memcmp(mm_data->reg.lai.mnc, mm_data->mm.lai.mnc, SIZE_MNC) 
      OR (mm_data->reg.lai.lac NEQ mm_data->mm.lai.lac))
    {
      /* EF LOCI value has changed, hence write it on SIM */
      /* EF Indicator for EF LOCI - bit 1*/
      mm_data->ef_indicator|=0x01;
    }
    mm_data->loc_upd_type.lut = NOT_RUNNING;
  }

#ifdef GPRS
  if (rr_changed_lai AND
      (mm_data->loc_upd_type.lut NEQ NOT_RUNNING) AND
      (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED))
  {
    /*
     * MM is in remote controlled operation with GPRS present and
     * the location area identifier has changed, MM has to set
     * its reg_type auxiliary state variable. The network mode
     * may have changed to network mode I.
     * Beware not to suppress an outstanding MMGMM_REG_CNF if
     * no update needed anymore for whatever reason.
     */
    SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);
  }
#endif /* GPRS */

  switch (mm_data->loc_upd_type.lut)
  {
    case NOT_RUNNING:
      /*
       * No location updating needed
       */

      /* Track possible change of T3212 */
      mm_change_t3212 ();

      TIMERSTOP (T3211);
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;

      mm_mmgmm_activate_ind (MMGMM_FULL_SERVICE);
      reg_mm_success (FULL_SERVICE);
      reg_build_sim_update ();

      /* Remember MM doesn't need any IMSI ATTACH anymore */
      if (mm_lup_allowed_by_gmm() AND mm_data->first_attach)
      {
        mm_data->first_attach_mem = mm_data->first_attach;
        mm_data->first_attach = FALSE;
      }
      mm_data->t3212_timeout = FALSE;

      SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
      USE_STORED_ENTRIES();
      break;

    case NORMAL_LUP:
      /*
       * If updating is allowed by GMM, start procedure,
       * otherwise enter appropriate IDLE state.
       */
      if (mm_lup_allowed_by_gmm())  /*lint !e774*/
      {
        mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);
        if (rr_changed_lai OR (!TIMERACTIVE (T3211) AND !TIMERACTIVE (T3213)))
        {
          mm_normal_loc_upd ();
        }
        else
        {
          /* Await timer expiry in appropriate IDLE state */
          if (mm_data->reg.update_stat EQ MS_UPDATED)
          {
            SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
          }
          else
          {
            SET_STATE (STATE_MM, MM_IDLE_ATTEMPT_TO_UPDATE);
          }
        }
      }
      else
      {
        mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
        SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
      }
      break;

    case IMSI_ATTACH_LUP:
      /*
       * If updating is allowed, start procedure,
       * otherwise enter state MM_IDLE_LUP_NEEDED.
       */
      if (mm_lup_allowed_by_gmm()) /*lint !e774*/
      {
        mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);

        if (mm_gsm_alone ())
        {
          mm_mmgmm_reg_cnf (); /* Early indication of full service */
        }

        if (!TIMERACTIVE (T3211) AND !TIMERACTIVE (T3213))
        {
          mm_attach_loc_upd ();
        }
        else
        {
          SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
        }
      }
      else
      {
        mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
        SET_STATE (STATE_MM, MM_IDLE_LUP_NEEDED);
      }
      break;

    case PERIODIC_LUP:
      /*
       * It is a periodic location updating procedure.
       * If updating is allowed, start procedure,
       * otherwise enter state MM_IDLE_NORMAL_SERVICE.
       * Compare this with GSM 04.08 subclause 4.2.3
       */
      if (mm_lup_allowed_by_gmm()) /*lint !e774*/
      {
        mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);

        if (mm_gsm_alone ())
        {
          mm_mmgmm_reg_cnf (); /* Early indication of full service */
        }

        if (!TIMERACTIVE (T3211) AND !TIMERACTIVE (T3213))
        {
          mm_periodic_loc_upd ();
        }
        else
        {
          SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
        }
      }
      else
      {
        mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
        SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
      }
      break;

    default: /* Cannot happen */
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_activate_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ACTIVATE_IND. MM is informed
            by RR about a cell selection without prior request
            by RR_ACTIVATE_REQ.

*/

GLOBAL void mm_rr_activate_ind (T_RR_ACTIVATE_IND *rr_activate_ind)
{
  GET_INSTANCE_DATA;

  mm_data->t3213_restart = MAX_REST_T3213;
restart_function:
  TRACE_FUNCTION ("mm_rr_activate_ind()");

#ifdef WIN32
  vsi_o_ttrace (VSI_CALLER TC_FUNC,
                "  MCC=%x%x%x MNC=%x%x%x LAC=%04X CID=%04X POW=%d",
                rr_activate_ind->plmn.mcc[0],
                rr_activate_ind->plmn.mcc[1],
                rr_activate_ind->plmn.mcc[2],
                rr_activate_ind->plmn.mnc[0],
                rr_activate_ind->plmn.mnc[1],
                rr_activate_ind->plmn.mnc[2],
                rr_activate_ind->lac,
                rr_activate_ind->cid,
                rr_activate_ind->power);
#endif /* #ifdef WIN32 */

  mm_data->rf_power = rr_activate_ind->power;
  mm_data->reg.new_cell_ind = TRUE;

  if(reg_plmn_equal_eqv (&rr_activate_ind->plmn, &mm_data->reg.actual_plmn))
  {
    mm_data->reg.actual_plmn = rr_activate_ind->plmn; /* Struct copy */
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_RR_CONN_MM:
      /*
       * If the unexpected incoming RR_ACTIVATE_IND in this state indicates
       * a new location area the rr_activation_indication is stored and a
       * rr_abort_req is sent. This should be answered by RR with a
       * RR_RELEASE_IND message triggering the MM to change the state to
       * MM_IDLE and to handle the outstanding RR_ACTIVATE_IND there.
       */
      {
        T_loc_area_ident  local_lai;

        memcpy (local_lai.mcc, rr_activate_ind->plmn.mcc, SIZE_MCC);
        memcpy (local_lai.mnc, rr_activate_ind->plmn.mnc, SIZE_MNC);
        local_lai.lac =   rr_activate_ind->lac;

        if (!mm_check_lai (&mm_data->mm.lai, &local_lai))
        {
          mm_write_entry(NO_ENTRY, NO_ENTRY, NO_ENTRY, PRIMITIVE_ENTRY, rr_activate_ind, UNSPEC);
          if (mm_data->pend_conn.cause NEQ ESTCS_EMRG_CAL)
          {
            TRACE_EVENT_P1 ("pend_conn.cause= %x", mm_data->pend_conn.cause);
            mm_abort_connection(ABCS_NORM);
          }
          return;
        }
      }
      /* FALLTHROUGH */
      /*lint -fallthrough */
    case MM_WAIT_FOR_RR_CONN_LUP:
    /*
     * case MM_WAIT_FOR_RR_CONN_MM:
     */
    case MM_WAIT_FOR_RR_CONN_DETACH:
      /*
       * A dedicated connection to the network
       * has been requested, but has not yet been established.
       * As we are storing the cell data here only,
       * it may become necessary to perform a location
       * updating procedure if coming back to IDLE.
       */

      mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF*) rr_activate_ind);

      /* Inform GPRS about selected cell */
      mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);

      if (mm_check_lai (&mm_data->mm.lai, &mm_data->reg.lai))
        reg_build_sim_update (); /* Update cell id */
      break;

    case MM_WAIT_FOR_REESTABLISH:
      /* RR indicates a suitable cell for call reestablishment */
      if (rr_activate_ind->mm_info.re EQ 0)
      {
        /* What if call reestablishment and after call release of
         reestablished call in other LA LUP is necessary?

         The following line was obviously missing here (HM, 01.02.01)
         mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF *) rr_activate_ind);

         Inform GPRS about selected cell 
         mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);
        */
        mm_data->reest_cell_avail = TRUE;
        /*at least one connection has requested call reestablishment */
        if (mm_data->reest_ti NEQ NOT_PRESENT_8BIT)
          mm_reest (mm_data->reest_ti);
        /*if (mm_normal_upd_needed())
         { */
          mm_write_entry(NO_ENTRY, NO_ENTRY, NO_ENTRY, PRIMITIVE_ENTRY, rr_activate_ind, UNSPEC);
          return;
        /*} */
      }
      else
      {
        /*
         * No support of call reestablishment
         */

        mm_mmxx_rel_ind (MMCS_NO_REESTABLISH, CM_NOT_IDLE);

        /* Find IDLE state after MM connection */
        mm_release_rr_connection (MMGMM_RESUMPTION_FAILURE);

        /* Restart the function in new state to perform location updating
         * if needed, avoid recursion for stack usage, therefore the goto */
        goto restart_function; /*lint !e801 goto*/
      }
     /* break is removed ,as case is returning before break so it is not needed */ 
    case MM_WAIT_FOR_RR_ACTIVE:
      /*
       * Clash case. While MM required to perform a network selection in RR,
       * RR performed a cell selection. This maybe ignored, as RR stored the
       * RR_ACTIVATE_REQ primitive in this case and a RR_ACTIVATE_CNF or
       * RR_ABORT_IND primitive will follow within short time.
       */
      break;

#ifdef GPRS
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      /*
       * What to do here?...
       */
      assert (GET_STATE (STATE_REG_TYPE) EQ REG_CELL_SEARCH_ONLY);
      TRACE_EVENT ("This needs still discussion");
      /*FALLTHROUGH*/
      /* lint -fallthrough */
#endif /* GPRS */
    case MM_IDLE_NORMAL_SERVICE: /* 19.1 */
      switch (rr_activate_ind->op.service)
      {
        case LIMITED_SERVICE:
          mm_limited_from_rr ((T_RR_ACTIVATE_CNF*)rr_activate_ind);
          break;

        case FULL_SERVICE:
          if (mm_handled_forb_plmn_cell (rr_activate_ind))
          {
            /*
             * The cell is a member of a forbidden list.
             */
            SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
            USE_STORED_ENTRIES();
          }
          else
          {
            mm_full_from_rr (rr_activate_ind);
          }
          break;

        default: /* Either NO_SERVICE or other garbage */
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      }
      break;

    case MM_IDLE_ATTEMPT_TO_UPDATE: /* 19.2 */
      if (mm_handled_forb_plmn_cell (rr_activate_ind))
      {
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      }
      else
      {
        /*
         * RR selected a cell which may serve for full service. Behaviour
         * here is described in GSM 04.08 subclause 4.2.2.2, "Service State,
         * ATTEMPTING TO UPDATE". The idea behind this subclause for the
         * case "cell in old location area selected" seems to be very simple:
         * If the location updating problem has been caused by the BSS or the
         * cell itself, perform an updating attempt as soon as a new cell
         * has been selected by RR and don't consider the timers T3211 and
         * T3213. In case the location updating problem has been caused by the
         * NSS (core network), e.g. there was a "network failure", updating
         * if a new cell is entered makes no sense as the problem was under
         * no circumstances related to the previously selected cell.
         */
        if (mm_check_lai_from_RR (&mm_data->mm.lai,
                                  &rr_activate_ind->plmn,
                                  rr_activate_ind->lac))
        {
          /*
           * RR selected a cell which belongs to a location
           * area identical with the previously selected cell.
           * Don't reset the attempt counter.
           * Compare this with GSM 04.08 subclause 4.4.4.5.
           */
          BOOL perform_lup;

          mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF*) rr_activate_ind);

          /* Track possible change of T3212 */
          mm_change_t3212 ();

          /*
           * Check reject cause category according to GSM 04.08 s
           * subclause 4.2.2.2 and decide whether a location updating
           * procedure shall be performed.
           */
          switch (mm_data->rej_cause)
          {
            case RRCS_MM_ABORTED:         /* GSM 04.08 4.4.4.9 e), T3210 */
            case RRCS_ABNORM_UNSPEC:      /* GSM 04.08 4.4.4.9 f), ABNORM */
              /*
               * don't start normal location updating if the state
               * has been entered after T3210 timeout or the network
               * released the RR connection with RR cause RRCS_ABNORM_UNSPEC.
               */
              perform_lup = FALSE;
              break;

            case MMCS_RETRY_IN_NEW_CELL:  /* GSM 04.08 4.4.4.8 g), RETRY */
              perform_lup = TRUE;
              break;

            case RRCS_RND_ACC_FAIL:       /* GSM 04.08 4.4.4.9 c) */
            case RRCS_DL_EST_FAIL:        /* GSM 04.08 4.4.4.9 d) */
              perform_lup = (mm_data->attempt_cnt < 4);
              break;

            default:
              /*
               * Treated here: GSM 04.08 4.4.4.9 f) with causes different
               * from "abnormal release, unspecified and g) with causes
               * different from "retry upon entry into a new cell".
               */
              perform_lup =
                GET_CAUSE_ORIGIN_ENTITY (mm_data->rej_cause) EQ RR_ORIGINATING_ENTITY;
              break;
          } /* switch (mm_data->rej_cause) */

          if (perform_lup)
          {
            /*
             * Normal location update is necessary
             */
            mm_normal_loc_upd ();

            /* Inform GPRS about selected cell */
            if (mm_lup_allowed_by_gmm()) /*lint !e774*/
            {
              mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);
            }
            else
            {
              mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
            }
          }
          else
          {
            /*
             * RR performed a cell selection which doesn't lead to a
             * location updating procedure in this state
             */
            mm_mmgmm_activate_ind (MMGMM_CELL_SELECTED);
          }
        }
        else
        {
          /*
           * RR selected a cell which belongs to a location
           * area not identical with the previously selected cell.
           * See GSM 04.08, subclause 4.2.2.2, "Service State,
           * ATTEMPTING TO UPDATE".
           */
          mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF*) rr_activate_ind);

          /* Track possible change of T3212 */
          mm_change_t3212 ();

          mm_data->attempt_cnt = 0; /* GSM 04.08 subclause 4.4.4.5 */

#ifdef GPRS
          TIMERSTOP (T3211);
          TIMERSTOP (T3213);
          mm_data->t3213_restart = 0;

          if (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED)
          {
            SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);
          }
#endif /* GPRS */

          mm_normal_loc_upd ();
          mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);
        }
      } /* cell which may offer full service */
      break;

    case MM_IDLE_LIMITED_SERVICE:   /* 19.3 */
    case MM_IDLE_NO_CELL_AVAILABLE: /* 19.5 */
      switch (rr_activate_ind->op.service)
      {
        case LIMITED_SERVICE:
          mm_limited_from_rr ((T_RR_ACTIVATE_CNF*)rr_activate_ind);
          break;

        case FULL_SERVICE:
          if (mm_handled_forb_plmn_cell (rr_activate_ind))
          {
            /*
             * The cell is a member of a forbidden list.
             */
            SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
            mm_use_entry ();
          }
          else
          {
            mm_full_from_rr (rr_activate_ind);
          }
          break;

        default: /* Either NO_SERVICE or other garbage */
          TRACE_ERROR (UNEXPECTED_DEFAULT);
          break;
      }
      break;

    case MM_IDLE_PLMN_SEARCH: /* 19.7 */
      if (!mm_handled_forb_plmn_cell (rr_activate_ind))
      {
        /*
         * Cell is not in forbidden list, offering full service.
         */
        mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF*) rr_activate_ind);

        // Is it really for sure that an ACTIVATE IND in this state
        // cannot serve for more than limited service? Why?...

        /* Inform GPRS about selected cell */
        mm_mmgmm_activate_ind (MMGMM_LIMITED_SERVICE);

        if (mm_check_lai (&mm_data->mm.lai, &mm_data->reg.lai))
          reg_build_sim_update ();

        switch (mm_data->reg.op.func)
        {
          case FUNC_LIM_SERV_ST_SRCH:
            reg_mm_success (LIMITED_SERVICE);
            mm_sim_set_imsi_marker( MSG_RR_ACT);
            break;
        }
      }
      break;

#ifdef GPRS
    case MM_IDLE_LUP_NEEDED: /* 19.6 */
      if (mm_handled_forb_plmn_cell (rr_activate_ind))
      {
        SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
      }
      else
      {
        mm_copy_rr_act_cnf_data ((T_RR_ACTIVATE_CNF*) rr_activate_ind);
        /*
         * Cell maybe ok for full service, not forbidden PLMN
         */
        if (!mm_normal_upd_needed() AND !mm_attach_upd_needed())
        {
          /*
           * Back to old updated area, no IMSI ATTACH needed
           */
          mm_mmgmm_activate_ind (MMGMM_FULL_SERVICE);
          SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
        }
        else
        {
          /*
           * Location updating procedure needed
           */
          mm_mmgmm_activate_ind (MMGMM_WAIT_FOR_UPDATE);

          /* Remain in MM state MM_IDLE_LUP_NEEDED */
        }
      }
      break;
#endif /* GPRS */

    case MM_IDLE_NO_IMSI: /* 19.4 */
      /*
       * The mobile has no SIM and a cell change is indicated.
       * Service cannot be better than LIMITED_SERVICE without IMSI (SIM).
       */
      mm_limited_from_rr ((T_RR_ACTIVATE_CNF*)rr_activate_ind);
      break;

    case MM_PLMN_SEARCH_NORMAL_SERVICE: /* 19.8 */
      /*
       * RR_ACTIVATE_REQ -> RR_ACTIVATE_IND -> RR_ABORT_IND (search result)
       * Best thing (which is not perfect anyway) here is to abort the
       * search requested by the MMI and to handle this in the previous state.
       * The user may get an empty list, but the RR_ACTIVATE_IND maybe
       * more important.
       */

      /* Abort the search for the MMI */
      mm_data->reg.plmn_cnt = 0;
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);

      /* Back to previous IDLE state */
      SET_STATE (STATE_MM, mm_data->idle_substate);

      /* Restart the function in new state, avoid recursion, stack usage,
       * therefore the goto. */
      goto restart_function; /*lint !e801 goto*/

    /*
     * Rare case: rr_activate_ind was received during call establishment for
     * emergency call and stored.
     * Stored rr_activate_ind shall remain stored until end of emergency call.
     */
    case MM_CONN_ACTIVE:
    /*
     * Rare case: 16868 rr_activate indication follows to RR_ABORT during a call.
     * Reestablishment was rejected because of unknown TI. 
     * Should trigger an LUP after RR connection release if a new LAI is contained.
     * Stored rr_activate_ind shall remain stored.
     */
    case MM_WAIT_FOR_NW_CMD:
      mm_write_entry(NO_ENTRY, NO_ENTRY, NO_ENTRY, PRIMITIVE_ENTRY, rr_activate_ind, UNSPEC);
      return;

    default:
      /*
       * MM_LUP_INITIATED, MM_WAIT_FOR_OUTG_MM_CONN,
       * MM_IMSI_DETACH_INIT, MM_PROCESS_PROMPT,
       * MM_LUP_REJECTED => Not expected cell selection in dedicated mode
       */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
  PFREE (rr_activate_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_establish_cnf        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ESTABLISH_CNF.

*/

GLOBAL void mm_rr_establish_cnf (T_RR_ESTABLISH_CNF *rr_establish_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mmrr_establish_cnf()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_RR_CONN_LUP:
      TIMERSTART (T3210, T_3210_VALUE);
      mm_data->ciphering_on = FALSE;
      SET_STATE (STATE_MM, MM_LUP_INITIATED);
      break;

    case MM_WAIT_FOR_RR_CONN_MM:
      mm_data->ciphering_on    = FALSE;
      mm_data->wait_for_accept = TRUE;
      SET_STATE (STATE_MM, MM_WAIT_FOR_OUTG_MM_CONN);
      TIMERSTART (T3230, T_3230_VALUE);
      break;

    case MM_WAIT_FOR_REESTABLISH:
      mm_data->ciphering_on    = FALSE;
      mm_data->wait_for_accept = TRUE;
      TIMERSTART (T3230, T_3230_VALUE);
      break;

    case MM_WAIT_FOR_RR_CONN_DETACH:
      /*
       * RR connection for IMSI Detach has been established
       */
      TIMERSTART (T3220, T_3220_VALUE);
      /*
       * Wait for release by the infrastructure or timeout T3220
       * if the SIM card is removed.
       */
      SET_STATE (STATE_MM, MM_IMSI_DETACH_INIT);
      break;

    default:
      /*
       * A RR_ESTABLISH_CNF is somewhat unexpected here, but we can try to
       * handle it by aborting the RR connection. But it is at least also
       * worth a TRACE.
       */
      mm_abort_connection (ABCS_NORM);
      TRACE_EVENT (UNEXPECTED_IN_STATE);
      break;
  }
  EM_RR_CONECTION_ESTABLISHED;

  PFREE (rr_establish_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_establish_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_ESTABLISH_IND.

*/

GLOBAL void mm_rr_establish_ind (T_RR_ESTABLISH_IND *rr_establish_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_rr_establish_ind()");

  TIMERSTOP (T3240);

  switch (GET_STATE (STATE_MM))
  {
    case MM_IDLE_PLMN_SEARCH:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_IDLE_NORMAL_SERVICE:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:
    case MM_LOCATION_UPDATING_PENDING:
      mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);
#endif /* GPRS */

      mm_data->idle_substate = mm_get_service_state ();
      mm_data->ciphering_on = FALSE;
      mm_data->rej_cause    = 0;
      SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
      break;

    case MM_WAIT_FOR_RR_CONN_MM:
      /*
       * Clash. RR_ESTABLISH_IND was underway, in the
       * same moment RR_ESTABLISH_REQ was sent.
       * The RR_ESTABLISH_REQ is cancelled, the MT
       * establishment has the right of way.
       */
#ifdef GPRS
      mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);
#endif /* GPRS */

      if (mm_data->pend_conn.comp EQ SMS_COMP)
      {
        /*
         * In the clash a pending MO SMS is involved. Do not release the SMS
         * but store it until it can established again.
         * Note: This special treatment makes only sense for SMS.
         */
        TRACE_EVENT ("MO SMS clashed with MT");
        mm_write_entry (mm_data->pend_conn.comp,
                        mm_data->pend_conn.ti,
                        mm_data->pend_conn.cause,
                        EVENT_ENTRY,
                        NULL,
                        UNSPEC);
      }
      else
      {
        /* Release all pending connections */
        mm_mmxx_rel_ind (MMCS_INT_PREEM, CM_PENDING);
      }
      mm_data->idle_substate = mm_get_service_state ();
      mm_data->ciphering_on = FALSE;
      mm_data->rej_cause    = 0;
      SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
      break;

      /*
    case MM_WAIT_FOR_RR_CONN_MM:
      mm_abort_connection (ABCS_NORM);
      switch (mm_data->pend_conn.comp)
      {
        case CC_COMP:
          if (mm_data->pend_conn.prio EQ PRIO_NORM_CALL)
          {
            switch (mm_data->pend_conn.cause)
            {
              case ESTCS_MOB_ORIG_SPCH:
              case ESTCS_MOB_ORIG_DATA:
              case ESTCS_MOB_ORIG_DATA_HR_SUFF:
                mm_rr_est_req (mm_data->pend_conn.cause,
                               CALL_SERVICE,
                               mm_data->pend_conn.ti);
                break;
            }
          }
          else
            mm_rr_est_req (ESTCS_EMERGE, CALL_SERVICE,
                           mm_data->pend_conn.ti);
          break;
        case SS_COMP:
          mm_rr_est_req (ESTCS_MOB_ORIG_CAL_BY_SS_SMS, SS_SERVICE,
                         mm_data->pend_conn.ti);
          break;
        case SMS_COMP:
          mm_rr_est_req (ESTCS_MOB_ORIG_CAL_BY_SS_SMS, SMS_SERVICE,
                         mm_data->pend_conn.ti);
          break;
      }
      break;
    */

    case MM_WAIT_FOR_REESTABLISH:
      /*
       * Lost RR connection by a radio link failure and next thing which
       * happens is MT call establishment, just before the internal
       * communication after the radio link failure was completed.
       * This is not expected to happen, but if so, the MT call
       * has to be aborted. Maybe the incoming call ti is identical to a ti
       * for a call which has to be reestablished, this would lead to failure.
       */
      mm_abort_connection (ABCS_NORM);
      break;

    default:
      break;
  }
  EM_RR_CONECTION_ESTABLISHED;

  PFREE (rr_establish_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_release_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_RELEASE_IND.

*/

GLOBAL void mm_rr_release_ind (T_RR_RELEASE_IND *rr_release_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_rr_release_ind()");

  /* Check for correct cause value */
  assert (GET_CAUSE_ORIGIN_ENTITY (rr_release_ind->cause) EQ
          RR_ORIGINATING_ENTITY);

#ifdef GPRS
  mm_data->gprs.resumption = rr_release_ind->gprs_resumption;
#endif /* #ifdef GPRS */

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
      if (mm_data->rej_cause EQ 0)
        mm_data->rej_cause = rr_release_ind->cause;
      TIMERSTOP (T3210);
      mm_mdl_rel_req ();
      mm_lup_restart ();
      break;

    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_WAIT_FOR_RR_CONN_MM:

      EM_RR_CONNECTION_ESTABLISHED_2;

      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_REESTABLISH:
      if (rr_release_ind->cause NEQ RRCS_MO_MT_COLL)
        mm_mdl_rel_req ();

      TIMERSTOP (T3230);
      TIMERSTOP (T3240);
#ifdef REL99
        /*Stop timer t3241 if it is ruuning.
         *As per the spec 24.008, Timer T3241 is stopped and reset (but not started)
         *when the MM state RR CONNECTION RELEASE NOT ALLOWED is left.
         */
        TIMERSTOP(T3241);
#endif

      if (rr_release_ind->cause EQ RRCS_RND_ACC_FAIL AND
          mm_data->reg.op.sim_ins EQ SIM_INSRT AND
          mm_data->reg.op.ts EQ TS_NO_AVAIL AND           
          mm_data->act_retrans NEQ 0 AND
          (mm_count_connections (CM_PENDING) NEQ 0 OR
           mm_count_connections (CM_REEST_PENDING) NEQ 0))
      {
        /*
         * start internal redial, if
         * - no TEST SIM
         * - SIM
         * - Cause <> random access failure
         * - retransmission counter <> 0
         * - at least one CM connection is pending
         */
        mm_data->act_retrans--;
        mm_rr_est_req (mm_data->pend_conn.cause,
                       mm_data->pend_conn.service,
                       mm_data->pend_conn.ti);
        SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_MM);
      }
      else
      {
        /*
         * other cases: no internal redial
         */
        if (rr_release_ind->cause EQ RRCS_MO_MT_COLL)
        {
          if (mm_data->pend_conn.comp EQ SMS_COMP)
          {
            /*
             * Clash MO/MT, a pending MO SMS is involved. Do not release
             * the SMS but store it until it can be established again.
             * Note: This special treatment makes only sense for SMS.
             */
            TRACE_EVENT ("MO SMS clashed with MT");
            mm_write_entry (mm_data->pend_conn.comp,
                            mm_data->pend_conn.ti,
                            mm_data->pend_conn.cause,
                            EVENT_ENTRY,
                            NULL,
                            UNSPEC);
          }
          else
          {
            /*
             * Clash MO/MT, no pending MO SMS is involved. Inform CM
             * about the release of the pending connection
             */
            mm_mmxx_rel_ind (rr_release_ind->cause, CM_PENDING);
            mm_mmxx_rel_ind (rr_release_ind->cause, CM_REEST_PENDING);
          }

          /* Back to old Idle state without informing GMM about CM release */
          SET_STATE (STATE_MM, mm_data->idle_substate);
        }
        else /* if release_ind->cause */
        {

        /* Commenting the OMAPS00048777 changes as it was a incomplete workaround.
        Refer the analysis section of the defect 71208 for details */

/*#ifdef GPRS 
          if (mm_data->gprs.sim_physically_removed)
          {
            mm_data->nreg_cause = CS_SIM_REM;
            mm_create_imsi_detach_message ();
            for_est_req (ESTCS_MOB_ORIG_CAL_BY_SS_SMS, BSIZE_U_IMSI_DETACH_IND);
            SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_DETACH);
          }
          else
#endif
          {*/
              /*
               * No MO/MT clash. Inform CM and also GMM about release.
               * Release all connections except the stored connections.
               */
              mm_mmxx_rel_ind (rr_release_ind->cause, CM_PENDING);
              mm_mmxx_rel_ind (rr_release_ind->cause, CM_ACTIVE);
              mm_mmxx_rel_ind (rr_release_ind->cause, CM_REEST_PENDING);

              /* Find IDLE state */
              mm_release_rr_connection (rr_release_ind->gprs_resumption);
/*          }*/
        } /* release cause <> collision */
      }
      break;

    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    {
      if (rr_release_ind->sapi NEQ SAPI_3)
      {
        /*
         * Release of main signalling link, release all.
         */

        /* Manager DL release, kill layer 2 */
        mm_mdl_rel_req ();

        TIMERSTOP (T3230);
        TIMERSTOP (T3240);

        if ((rr_release_ind->cause EQ RRCS_NORM) AND
            (mm_count_connections (CM_PENDING) NEQ 0))
        {
          /*
           * This is state MM WAIT FOR ADD OUTG MM CONN.
           * MM_PROCESS_PROMPT is incompatible with the requestion of a new
           * MM connection, so we are not in this state here.
           * The RR connection was released by the network normally.
           * Assume a clash case and repeat the CM_SERVICE_REQUEST message
           * for the pending connection.
           */
          mm_mmxx_rel_ind (rr_release_ind->cause, CM_ACTIVE);
          mm_mmxx_rel_ind (rr_release_ind->cause, CM_REEST_PENDING);

          mm_rr_est_req (mm_data->pend_conn.cause,
                         mm_data->pend_conn.service,
                         mm_data->pend_conn.ti);
          SET_STATE (STATE_MM, MM_WAIT_FOR_RR_CONN_MM);
        }
        else
        {
          /*
           * Inform CM about release.
           */
          mm_mmxx_rel_ind (rr_release_ind->cause, CM_PENDING);
          mm_mmxx_rel_ind (rr_release_ind->cause, CM_ACTIVE);
          mm_mmxx_rel_ind (rr_release_ind->cause, CM_REEST_PENDING);

          /* Find IDLE state after MM connection */
          mm_release_rr_connection (rr_release_ind->gprs_resumption);
        }
      }
      else
      {
        /*
         * Only release of RR connection for SAPI = 3,
         * main signalling link not released.
         */

        /*
         * Inform CM entity SMS about release. All SAPI 3 connections
         * are to be released to SMS (except the stored ones).
         */
        mm_mmsms_rel_ind (rr_release_ind->cause, CM_PENDING);
        mm_mmsms_rel_ind (rr_release_ind->cause, CM_ACTIVE);
        mm_mmsms_rel_ind (rr_release_ind->cause, CM_REEST_PENDING);

        if (mm_count_connections (CM_ACTIVE) NEQ 0 OR
            mm_count_connections (CM_PENDING) NEQ 0)
        {
          /*
           * Some active or pending connections remaining for
           * SAPI NEQ 3, kill layer 2 only for SAPI = 3
           */
          mm_mdl_rel_req_sapi_3 ();
        }
        else
        {
          /*
           * No active or pending connections
           * remaining, manager release of layer 2.
           */
          mm_mdl_rel_req ();

          TIMERSTOP (T3230);
          TIMERSTOP (T3240);

          /* Find IDLE state after MM connection */
          mm_release_rr_connection (rr_release_ind->gprs_resumption);
        }
      }
      break;
    }

    case MM_IMSI_DETACH_INIT:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      mm_mdl_rel_req ();
      mm_mmgmm_cm_release_ind (rr_release_ind->gprs_resumption);
      mm_end_of_detach ();
      break;

    case MM_LUP_REJECTED:
      mm_mdl_rel_req ();
      TIMERSTOP (T3240);
      mm_loc_upd_rej ();
      break;

    case MM_WAIT_FOR_RR_CONN_LUP:
      mm_data->rej_cause = rr_release_ind->cause;
      mm_mdl_rel_req ();
      TIMERSTOP (T3210);
      switch (rr_release_ind->cause)
      {
        case RRCS_DL_EST_FAIL:
          /*
           * GSM 04.08 subclause 4.4.4.9 case d)
           *   RR connection failure.
           */
          mm_data->rej_cause = RRCS_DL_EST_FAIL;
          mm_lup_restart ();
          break;

        case RRCS_RND_ACC_FAIL:
          /*
           * GSM 04.08 subclause 4.4.4.9 case c)
           *   Random access failure.
           */
          mm_data->rej_cause = RRCS_RND_ACC_FAIL;
          mm_data->idle_entry = RRCS_INT_NOT_PRESENT;

#ifdef WIN32
          TRACE_EVENT_P1 ("Last Rej Cause = %x", mm_data->last_rej_cause);
#endif /* #ifdef WIN32 */

          if (mm_data->last_rej_cause EQ RRCS_RND_ACC_FAIL)
          {
            mm_lup_restart ();
          }
          else
          {
            mm_data->last_rej_cause = RRCS_RND_ACC_FAIL;
            TIMERSTART (T3213, T_3213_VALUE);
            mm_data->t3213_restart = 0;

            /*
             * It can be safely assumed that idle_substate here is either
             * MM_IDLE_NORMAL_SERVICE or MM_IDLE_ATTEMPT_TO_UPDATE
             */
            SET_STATE (STATE_MM, mm_data->idle_substate);
          }
          break;

        case RRCS_ACCESS_BARRED:
        case RRCS_RND_ACC_DELAY:
          /*
           * GSM 04.08 subclause 4.4.4.9 case a)
           *   Access barred because of access class control.
           * GSM 04.08 subclause 4.4.4.9 case b)
           *   The answer to random access is an
           *   IMMEDIATE ASSIGNMENT REJECT message.
           */
          mm_data->idle_entry = rr_release_ind->cause;

          /*
           * It can be safely assumed that idle_substate here is either
           * MM_IDLE_NORMAL_SERVICE or MM_IDLE_ATTEMPT_TO_UPDATE
           */
          SET_STATE (STATE_MM, mm_data->idle_substate);
          break;

        default: /* eg. RRCS_ABNORM_UNSPEC, RRCS_INT_NOT_PRESENT */
          mm_lup_restart ();
          break;
      }
      break;

    default:
      /*
       * 19.x, MM_LOCATION_UPDATING_PENDING, MM_IMSI_DETACH_PENDING,
       * and all remaining MM states.
       */

      /* Local end release of layer 2 */
      mm_mdl_rel_req ();

#ifdef GPRS
      /* Assume GMM sent GMMRR_CS_PAGE_RES (GMMRR_CS_PAGE_CNF).
       * This means CS services are (were) allowed. */
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);

      /* Give CM control back to GMM */
      mm_mmgmm_cm_release_ind (rr_release_ind->gprs_resumption);
#endif /* #ifdef GPRS */

      USE_STORED_ENTRIES();
      break;
  }

#ifdef GPRS
  mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;
#endif /* #ifdef GPRS */

  PFREE (rr_release_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_sync_ind             |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive RR_SYNC_IND.

*/

GLOBAL void mm_rr_sync_ind (T_RR_SYNC_IND *rr_sync_ind)
{
  GET_INSTANCE_DATA;
  BOOL t3212_changed;

  TRACE_FUNCTION ("mm_rr_sync_ind()");

  /* prevent the T3213 from restarting when it runs first time, but don't forget, that it is restarted if so */
  if (mm_data->t3213_restart EQ 0)
  {
    mm_data->t3213_restart = MAX_REST_T3213;
  }

  /* Remember wheter a change in broadcasted value for T3212 was detected */
  t3212_changed = (rr_sync_ind->mm_info.valid EQ MM_INFO_PRES AND
                   mm_data->mm.mm_info.t3212 NEQ rr_sync_ind->mm_info.t3212);

  /*
   * Forward new BCCH information to the SIM application
   */
  if (rr_sync_ind->bcch_info.v_bcch EQ V_BCCH_PRES)
  {
    // Patch HM 14.03.01 >>>
    // memcpy (&mm_data->mm.bcch, &rr_sync_ind->bcch_info, SIZE_BCCH);
    memcpy (mm_data->mm.bcch, rr_sync_ind->bcch_info.bcch, SIZE_BCCH);
    // Patch HM 14.03.01 <<<
    if (memcmp(rr_sync_ind->bcch_info.bcch,mm_data->reg.bcch,SIZE_BCCH))
    {
      /* Set bit 2 in ef_indicator to indicate bcch_info change to SIM */
      mm_data->ef_indicator|=(0x01 << 1);
    }
    reg_build_sim_update ();
    PFREE (rr_sync_ind); //
    return;              //
  }

  /*
   * forwarding of ciphering indicator
   */
  if (rr_sync_ind->ciph NEQ CIPH_NOT_PRES)
  {
      if (rr_sync_ind->ciph NEQ mm_data->ciphering_on)
      {
#ifdef GPRS /* GPRS supported, forward ciphering info for indicator to GMM */
         PALLOC (ciphering_ind,MMGMM_CIPHERING_IND);
         ciphering_ind->ciph = rr_sync_ind->ciph;
         PSENDX (GMM, ciphering_ind);
#else /* GSM only case, forward ciphering info for indicator to ACI directly */
         PALLOC (ciphering_ind,MMR_CIPHERING_IND);
         ciphering_ind->ciph = rr_sync_ind->ciph;
         PSENDX (MMI, ciphering_ind);
#endif /* GPRS */
      }
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_LUP_INITIATED:
      if (rr_sync_ind->ciph NEQ CIPH_NOT_PRES)
      {
        mm_data->ciphering_on = rr_sync_ind->ciph;
      }
      break;

    case MM_WAIT_FOR_OUTG_MM_CONN:
      if (rr_sync_ind->ciph NEQ CIPH_NOT_PRES)
      {
        mm_data->ciphering_on = rr_sync_ind->ciph;
        mm_data->error = FALSE;
        mm_cm_serv_accept ();
      }

      if (rr_sync_ind->chm.ch_mode NEQ NOT_PRESENT_8BIT)
      {
        PALLOC (mmcm_sync_ind, MMCM_SYNC_IND); /* T_MMCM_SYNC_IND */
        mmcm_sync_ind->ti  = 0;
        mmcm_sync_ind->sync_info.ch_info.ch_type = rr_sync_ind->chm.ch_type;
        mmcm_sync_ind->sync_info.ch_info.ch_mode = rr_sync_ind->chm.ch_mode;
        PSENDX (CC, mmcm_sync_ind);

      }
      break;

   case MM_CONN_ACTIVE:
      if (rr_sync_ind->chm.ch_mode NEQ NOT_PRESENT_8BIT)
      {
        PALLOC (mmcm_sync_ind, MMCM_SYNC_IND); /* T_MMCM_SYNC_IND */
        mmcm_sync_ind->ti  = 0;
        mmcm_sync_ind->sync_info.ch_info.ch_type = rr_sync_ind->chm.ch_type;
        mmcm_sync_ind->sync_info.ch_info.ch_mode = rr_sync_ind->chm.ch_mode;
        PSENDX (CC, mmcm_sync_ind);
      }
      if (rr_sync_ind->ciph NEQ NOT_PRESENT_8BIT)
      {
        mm_data->ciphering_on = rr_sync_ind->ciph;
        if (mm_data->wait_for_accept)
        {
          mm_mmxx_est_cnf ();
          TIMERSTOP (T3230);
          mm_data->wait_for_accept = FALSE;

          EM_CM_SERVICE_ACCEPTED(EM_COMMAND);

          USE_STORED_ENTRIES();
        }
      }
      break;

    case MM_PROCESS_PROMPT:
      if (rr_sync_ind->chm.ch_mode NEQ NOT_PRESENT_8BIT)
      {
        /* Channel mode modification, MMCM_SYNC_IND to CC */
        PALLOC (mmcm_sync_ind, MMCM_SYNC_IND); /* T_MMCM_SYNC_IND */
        mmcm_sync_ind->ti  = 0;
        mmcm_sync_ind->sync_info.ch_info.ch_type = rr_sync_ind->chm.ch_type;
        mmcm_sync_ind->sync_info.ch_info.ch_mode = rr_sync_ind->chm.ch_mode;
        PSENDX (CC, mmcm_sync_ind);

      }

      if (rr_sync_ind->ciph NEQ NOT_PRESENT_8BIT)
      {
        /* Ciphering changed, remember this is MM data */
        mm_data->ciphering_on = rr_sync_ind->ciph;
        if (mm_count_connections (CM_ACTIVE) NEQ 0)
        {
          /*
           * In state MM_PROCESS PROMPT we cannot have
           * pending connections which are waiting
           * for CM SERVICE ACCEPT. This means, do nothing here.
           */
        }
        else
        {
          /*
           * No connection exists, behaviour like in state
           * of MM_WAIT_FOR_NW_CMD, restart T3240
           */
          TIMERSTART (T3240, T_3240_VALUE);
        }
      }
      break;

    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
      if (rr_sync_ind->chm.ch_mode NEQ NOT_PRESENT_8BIT)
      {
        PALLOC (mmcm_sync_ind, MMCM_SYNC_IND); /* T_MMCM_SYNC_IND */
        mmcm_sync_ind->ti  = 0;
        mmcm_sync_ind->sync_info.ch_info.ch_type = rr_sync_ind->chm.ch_type;
        mmcm_sync_ind->sync_info.ch_info.ch_mode = rr_sync_ind->chm.ch_mode;
        PSENDX (CC, mmcm_sync_ind);

      }

      if (rr_sync_ind->ciph NEQ NOT_PRESENT_8BIT)
      {
        mm_data->ciphering_on = rr_sync_ind->ciph;

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
#ifdef REL99
if(TIMERACTIVE(T3241))
        {
          /*Do nothing*/
        }
        else
#endif
        {
          /*restart timer T3240*/
          TIMERSTART (T3240, T_3240_VALUE);
        }

      }
      break;

    case MM_IDLE_NO_IMSI:
      /*
       * Add traces to see last reject cause for location updating reject and
       * the place where MM entered the MM_IDLE_NO_IMSI state.
       */
      TRACE_EVENT_P1 ("Last lup rej cause: %04x",
                      mm_data->debug_last_rej_cause);
      TRACE_EVENT_P1 ("Entered state at %d",
                      mm_data->mm_idle_no_imsi_marker);
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_CONN_DETACH:
    case MM_IDLE_LIMITED_SERVICE:
      if ((rr_sync_ind->mm_info.valid EQ MM_INFO_PRES) AND
          (mm_data->reg.lai.lac NEQ LAC_INVALID_VALUE))
      {
        mm_data->mm.mm_info = rr_sync_ind->mm_info; /* Structure copy */

        if (t3212_changed)
        {
          // Maybe GMM is not interested either in T3212 if service state
          // is LIMITED SERVICE only, this should be checked...
          mm_mmgmm_t3212_val_ind ();
        }
      }
      break;

    case MM_IDLE_NORMAL_SERVICE: /* 19.1 */
      if (rr_sync_ind->mm_info.valid EQ MM_INFO_PRES)
      {
        mm_data->mm.mm_info = rr_sync_ind->mm_info; /* Structure copy */

        if (t3212_changed)
        {
          mm_mmgmm_t3212_val_ind ();
          mm_change_t3212 ();
        }

      }

      if ((rr_sync_ind->synccs EQ SYNCCS_ACC_CLS_CHA AND
           mm_data->idle_entry EQ RRCS_ACCESS_BARRED) OR
          (rr_sync_ind->synccs EQ SYNCCS_T3122_TIM_OUT AND
           mm_data->idle_entry EQ RRCS_RND_ACC_DELAY) OR
          (mm_data->t3213_restart > 0 AND
           mm_data->rej_cause EQ RRCS_RND_ACC_FAIL))

      {
        mm_continue_running_update ();
      }
      break;

    case MM_IDLE_ATTEMPT_TO_UPDATE:
      if (rr_sync_ind->mm_info.valid EQ MM_INFO_PRES)
      {
        mm_data->mm.mm_info = rr_sync_ind->mm_info; /* Structure copy */

        if (t3212_changed)
        {
          mm_mmgmm_t3212_val_ind ();
          mm_change_t3212 ();
        }

      }

      if ((rr_sync_ind->synccs EQ SYNCCS_ACC_CLS_CHA AND
           mm_data->idle_entry EQ RRCS_ACCESS_BARRED) OR
          (rr_sync_ind->synccs EQ SYNCCS_T3122_TIM_OUT AND
           mm_data->idle_entry EQ RRCS_RND_ACC_DELAY) OR
          (mm_data->t3213_restart > 0 AND
           mm_data->rej_cause EQ RRCS_RND_ACC_FAIL))
      {
        mm_continue_running_update ();
        break;
      }

#if 0 /* This code causes failure on ALCATEL test cases */
      /* This registration attempt does not check the attempt counter*/
      /* and so can cause the MS to attempt more than 4 LUs to the network */
      if (rr_sync_ind->synccs EQ SYNCCS_LUP_RETRY)
      {
        if (mm_data->reg.op.sim_ins EQ SIM_INSRT AND
            mm_data->reg.op.ts EQ TS_NO_AVAIL)
        {
          /*
           * A SIM is inserted and it is no test SIM
           */
          if (mm_lup_allowed_by_gmm())  /*lint !e774*/
          {
            mm_normal_loc_upd ();
          }
          else
          {
            mm_mmgmm_lup_needed_ind (MMGMM_RXLEV_JUMP);
            /* No state change, MM remains in MM_IDLE_ATTEMPT_TO_UPDATE */
          }
        }
      }
#endif
      break;

    case MM_WAIT_FOR_REESTABLISH:
      if (rr_sync_ind->mm_info.re EQ RE_ALLOW)
      {
        /*
         * RR indicates a suitable cell for call reestablishment
         */

        mm_data->reest_cell_avail = TRUE;
        // at least one connection has requested call reestablishment
        if (mm_data->reest_ti NEQ NOT_PRESENT_8BIT)
          mm_reest (mm_data->reest_ti);
      }
      else
      {
        /*
         * No support of call reestablishment
         */

        mm_mmxx_rel_ind (MMCS_NO_REESTABLISH, CM_NOT_IDLE);

        /* Find IDLE state after MM connection */
        mm_release_rr_connection(MMGMM_RESUMPTION_FAILURE);
      }
      break;

#ifdef GPRS
    case MM_IDLE_LUP_NEEDED:           /* 19.6 */
    case MM_LOCATION_UPDATING_PENDING: /* 23 */
    case MM_IMSI_DETACH_PENDING:       /* 24 */
      if (rr_sync_ind->mm_info.valid EQ MM_INFO_PRES)
      {
        mm_data->mm.mm_info = rr_sync_ind->mm_info; /* Structure copy */

        if (t3212_changed)
        {
          mm_mmgmm_t3212_val_ind ();
          if (mm_get_service_state () NEQ MM_IDLE_LIMITED_SERVICE)
            mm_change_t3212 ();
        }

      }
      break;
#endif /* GPRS */

    default:
      TRACE_EVENT (PRIMITIVE_IGNORED);
      break;
  }
  PFREE (rr_sync_ind);
}

#if defined (FF_EOTD) AND defined (REL99)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_rrlp_start_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive mm_rr_rrlp_start_ind.

*/

GLOBAL void mm_rr_rrlp_start_ind (T_RR_RRLP_START_IND *rr_rrlp_start_ind)
{
  GET_INSTANCE_DATA;
    TRACE_FUNCTION ("mm_rr_rrlp_start_ind()");
    /*
     *set rrlp_lcs_started flag to true
    */
    mm_data->rrlp_lcs_started = TRUE;
    switch (GET_STATE (STATE_MM))
    {
      case MM_WAIT_FOR_NW_CMD:
        TIMERSTOP(T3240);
        TIMERSTART(T3241, T_3241_VALUE);
        SET_STATE (STATE_MM, MM_RR_CONN_RELEASE_NOT_ALLOWED);
        break;

      case MM_RR_CONN_RELEASE_NOT_ALLOWED:
        TIMERSTOP(T3241);
        TIMERSTART(T3241, T_3241_VALUE);
        break;

      default :
        break;
    }
    PFREE (rr_rrlp_start_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_rrlp_stop_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive mm_rr_rrlp_stop_ind.

*/

GLOBAL void mm_rr_rrlp_stop_ind (T_RR_RRLP_STOP_IND *rr_rrlp_stop_ind)
{
  GET_INSTANCE_DATA;
    TRACE_FUNCTION ("mm_rr_rrlp_stop_ind()");
    /*
     *set rrlp_lcs_started flag to false
    */
    mm_data->rrlp_lcs_started = FALSE;
    switch (GET_STATE (STATE_MM))
    {
      case MM_RR_CONN_RELEASE_NOT_ALLOWED:
        TIMERSTOP(T3241);
        TIMERSTART(T3240, T_3240_VALUE);
        SET_STATE (STATE_MM, MM_WAIT_FOR_NW_CMD);
        break;

      default :
        break;
    }
    PFREE (rr_rrlp_stop_ind);
}

#endif /* (FF_EOTD) AND defined (REL99) */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_prompt_rej         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_PROMPT_REJ.

*/

GLOBAL void mm_mmcm_prompt_rej (T_MMCM_PROMPT_REJ *prompt_rej)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmcm_prompt_rej()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_PROCESS_PROMPT:
      /* Send MM STATUS with cause #34 */
      {
/* Implements Measure 29 and streamline encoding */
        mm_send_status(RC_SERVICE_ORDER);
      }

      if ((mm_count_connections (CM_ACTIVE) NEQ 0) OR
          (mm_count_connections (CM_PENDING) NEQ 0))
      {
        /* This is not standardized in GSM 4.08, but
           without returning to state MM_CONN_ACTIVE
           some MSCs in GSM 04.93 don't make sense. */
        SET_STATE (STATE_MM, MM_CONN_ACTIVE);
      }
      else
      {
#if defined (FF_EOTD) AND defined (REL99)
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
      break;

    default:
      break;
  }
  PFREE (prompt_rej);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_prompt_res         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMCM_PROMPT_RES.

*/

GLOBAL void mm_mmcm_prompt_res (T_MMCM_PROMPT_RES *prompt_res)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmcm_prompt_res()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_PROCESS_PROMPT:
      TIMERSTOP (T3240);
      CM_SET_STATE (CC_COMP, prompt_res->ti, CM_ACTIVE);
      SET_STATE (STATE_MM, MM_CONN_ACTIVE);
      break;
    default:
      /* MM cannot do anything (anymore) with the ti, send MMCM_RELEASE_IND */
      mm_mmxx_release_ind (CC_COMP, prompt_res->ti, MMCS_INT_NOT_PRESENT);
      break;
  }
  PFREE (prompt_res);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmcm_ss_sms_data_req    |
+--------------------------------------------------------------------+

  PURPOSE : This function unifies mm_mmcm_data_req(), mm_mmss_data_req()
            and mm_mmsms_data_req().
*/

LOCAL void mm_mmcm_ss_sms_data_req   (T_VOID_STRUCT         *mm_data_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmcm_ss_sms_data_req()");
  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
    {
      PPASS (mm_data_req, rr_data_req, RR_DATA_REQ);
      for_cm_message (rr_data_req);
      break;
    }
    default:
      PFREE (mm_data_req);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_sim_insrt_state         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the parameter mm_idle_no_imsi_marker
            depending on the selector imsi_marker.

*/
GLOBAL void mm_sim_set_imsi_marker  (T_MSG_TYPE imsi_marker)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_sim_set_imsi_marker()");
  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /* Valid SIM inserted */
    SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
  }
  else
  {
    /* Find original place where MM entered MM_IDLE_NO_IMSI state >>> */
    if (mm_data->mm_idle_no_imsi_marker EQ 0)
    {
      if ( imsi_marker EQ MSG_RR_ACT)
        mm_data->mm_idle_no_imsi_marker = 13;
      else
        mm_data->mm_idle_no_imsi_marker = 3;
    }
    /* End of debugging patch <<< */
    /* Invalid SIM inserted */
    SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_sim_insrt_state         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the MM state depending on the SIM INSERT 
            status.

*/
LOCAL void mm_sim_insert_state  (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_sim_insert_state()");
  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /* SIM present */
    SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
  }
  else
  {
    /* SIM not present */
    SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_rr_abort_cell_sel_fail  |
+--------------------------------------------------------------------+

  PURPOSE : This function processes RR_ABORT_IND

*/
LOCAL void mm_rr_abort_cell_sel_fail  (T_RR_ABORT_IND   *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_rr_abort_cell_sel_fail()");
  if (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL)
  {
    TIMERSTOP (T3211);
    TIMERSTOP (T3213);
    mm_data->t3213_restart = 0;
    mm_mmxx_rel_ind (rr_abort_ind->cause, CM_NOT_IDLE);
    mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
    if (rr_abort_ind->op.service EQ NO_SERVICE)
    {
      SET_STATE (STATE_MM, MM_IDLE_NO_CELL_AVAILABLE);
    }
    else
    {
      SET_STATE (STATE_MM, MM_IDLE_LIMITED_SERVICE);
    }
    reg_rr_failure (rr_abort_ind);
  }
}

#endif

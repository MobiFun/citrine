/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_GPRS.C
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
|  Purpose :  This module defines the functions which are necessary 
|             to process requests from GMM in MM.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_GMM_C
#define MM_GMM_C

#define ENTITY_MM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
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

#ifdef GPRS

/*==== EXPORT ==============================================================*/

/*==== TEST ================================================================*/

/*==== PRIVAT ==============================================================*/

/*==== VARIABLES ===========================================================*/

/*==== FUNCTIONS ===========================================================*/

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_gprs_update_req                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function performs an remote controlled, by GMM initiatiated
            location update
  
*/

GLOBAL void mm_gprs_update_req (void)
{
  GET_INSTANCE_DATA;
restart_function:
  
  TRACE_FUNCTION ("mm_gprs_update_req()");

  mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_ACTIVE:
      mm_data->attempt_cnt = 0; /* Just to be safe */
      /* Save the event as outstanding periodic update */
      mm_data->t3212_timeout = TRUE;
      break;

    case MM_IDLE_NORMAL_SERVICE: /* 19.1 */
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      /* 
       * As MM had no knowledge that a LOCATION UPDATING is necessary, 
       * it can only be a periodic location updating or an unnecessary 
       * trigger for an IMSI attach location updating.
       */
      if (mm_data->first_attach)
      {
        assert (mm_data->mm.mm_info.att EQ ATT_NOT_ALLOW);

        /* 
         * Start BCCH with broadcasted value. It could be discussed 
         * it the better time would have been after RR_ACTIVATE_IND.
         */
        mm_start_t3212_bcch();

        mm_data->first_attach_mem = mm_data->first_attach;
        mm_data->first_attach = FALSE;

        mm_mmgmm_reg_cnf ();
      }
      else
      {
        /* 
         * GMM should not trigger a periodic location updating 
         * if this is not announced by the cell, but anyway, it does not
         * do any harm to check this condition here.
         */
        if (mm_data->mm.mm_info.t3212 NEQ T3212_NO_PRD_UPDAT)
        {
          mm_data->attempt_cnt = 0; /* Just to be safe */
          mm_periodic_loc_upd ();
        }
        else
        {
          mm_mmgmm_reg_cnf ();
        }
      }
      break;
    
    case MM_IDLE_ATTEMPT_TO_UPDATE: /* 19.2 */
      mm_data->attempt_cnt = 0; // Patch HM 14-Feb-02, GSM 11.10 44.2.1.2.8      
      mm_normal_loc_upd ();
      break;

    case MM_IDLE_LIMITED_SERVICE:   /* 19.3 */
      if (mm_data->reg.op.m EQ M_MAN)
      {
        mm_data->attempt_cnt = 0;
        mm_normal_loc_upd ();
      }
      else
      {
        TRACE_EVENT ("GMM triggered invalid update, not done.");
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                           SEARCH_NOT_RUNNING, 
                           FORB_PLMN_NOT_INCLUDED);
      }
      break;
    
    case MM_IDLE_NO_IMSI:           /* 19.4 */
      mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                         SEARCH_NOT_RUNNING, 
                         FORB_PLMN_NOT_INCLUDED);
      break;
    
    case MM_IDLE_NO_CELL_AVAILABLE: /* 19.5 */
      mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                         SEARCH_NOT_RUNNING, 
                         FORB_PLMN_NOT_INCLUDED);
      break;
    
    case MM_IDLE_LUP_NEEDED:        /* 19.6 */
      /* 
       * Start location update procedure in remote controlled form,
       * the type of location update may be NORMAL_LUP or IMSI_ATTACH_LUP.
       * MM already knows from the state that it needs a remote controlled
       * location updating.
       */
      if (!mm_normal_upd_needed ())
      {
        /* Send fake MMGMM_LUP_ACCEPT_IND to indicate the full service 
         * condition already (which is a fact if only IMSI ATTACH needed). */
        mm_mmgmm_lup_accept_ind ();
        
        mm_data->attempt_cnt = 0; /* Power on, SIM inserted */
        mm_attach_loc_upd ();
      }
      else
      {
        mm_data->attempt_cnt = 0; /* Change of LA detected */
        mm_normal_loc_upd ();
      }
      break;

    case MM_IDLE_PLMN_SEARCH:            /* 19.7 */
      if (mm_data->idle_substate EQ MM_IDLE_NO_CELL_AVAILABLE)
      {
        mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                           SEARCH_NOT_RUNNING, 
                           FORB_PLMN_NOT_INCLUDED);
        break;
      }
      /*FALLTHROUGH*/
      //lint -fallthrough 
    case MM_PLMN_SEARCH_NORMAL_SERVICE:  /* 19.8 */
      /* Scan aborted, notify MMI */
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);

      /* Back to previous, non-searching IDLE substate */
      SET_STATE (STATE_MM, mm_data->idle_substate);
      
      /* Restart the function in new state, avoid recursion, stack usage */
      goto restart_function;
    
    default: 
      TRACE_EVENT (UNEXPECTED_IN_STATE);
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_auth_rej_req         |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_AUTH_REJ_REQ primitive.
            MMGMM_AUTH_REJ_REQ indicates MM that the authentication procedure
            performed by GMM is rejected by the network. For further details, 
            see GSM 04.08 subclause 4.3.2.5.
  
*/

GLOBAL void mm_func_mmgmm_auth_rej_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_auth_rej_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_IDLE_NORMAL_SERVICE:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_IMSI:
    case MM_IDLE_NO_CELL_AVAILABLE:
    case MM_IDLE_LUP_NEEDED:
    case MM_IDLE_PLMN_SEARCH:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      /* Release all calls, can only be stored calls */
      mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);

      /* 
       * Upon receipt of an AUTHENTICATION REJECT message, the mobile station 
       * shall set the update status in the SIM to U2 ROAMING NOT ALLOWED, 
       * delete from the SIM the stored TMSI, LAI and ciphering key sequence 
       * number. [GSM 04.08 subclause 4.3.2.5]
       */
#ifdef REL99
      reg_invalidate_upd_state (MS_LA_NOT_ALLOWED, FALSE);
#else
      reg_invalidate_upd_state (MS_LA_NOT_ALLOWED);
#endif

      /* Delete IMSI - consider SIM as invalid */
      mm_clear_mob_ident (&mm_data->reg.imsi_struct);
      mm_clear_reg_data ();

      /* Inform RR about invalidation */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_CKSN_KC_INVAL_NO_PAG);

      /* Set new MM main state */
      SET_STATE (STATE_MM, MM_IDLE_NO_IMSI);
      break;

    default: /* No IDLE state */
      /* 
       * This means: Either this is an internal protocol error or we are 
       * performing circuit switched and packed switched operations in 
       * parallel. MM does not handle this. As it seems so that 
       * the networks will never support this it makes no sense to handle it.
       */
      TRACE_ERROR (UNEXPECTED_IN_STATE);
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_cm_establish_res     |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_CM_ESTABLISH_RES primitive.
            MMGMM_CM_ESTABLISH_RES confirms the establish request.
  
*/

GLOBAL void mm_func_mmgmm_cm_establish_res (UBYTE cm_establish_res)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_cm_establish_res()");

  switch (GET_STATE (STATE_GPRS_CM_EST))
  {
    case CM_GPRS_EST_PEND:    /* Normal case */
      break;

    case CM_GPRS_EST_OK:      /* Clash MO/MT, special case */
      TRACE_EVENT ("MO/MT clash");
      return;

    default:
      TRACE_ERROR ("Unexpected STATE_GPRS_CM_EST");
      break;
  }

  if (cm_establish_res EQ MMGMM_ESTABLISH_OK)
  {
    mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;

    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);

    if (mm_count_connections (CM_NOT_IDLE) EQ 0)
    {
     /* 
      * In the meantime, all the stored connections have been cleared
      * Worst case assumption: MMGMM_RESUMPTION_FAILURE.
      */
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
    }
    else
    {
      /* 
       * Now all the stored establish requests may be performed
       */
      USE_STORED_ENTRIES();
    }
  } 
  else /* cm_establish_res EQ MMGMM_ESTABLISH_REJECT */ 
  {
    /* Clear outstanding MMGMM_CM_ESTABLISH_IND */
    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
        
    /* Release all connections, there can only be stored connections */ 
    mm_mmxx_rel_ind (MMCS_INT_NOT_PRESENT, CM_NOT_IDLE);
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_cm_emergency_res     |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_CM_EMERGENCY_RES primitive.
            MMGMM_CM_EMERGENCY_RES confirms the establish request.
  
*/

GLOBAL void mm_func_mmgmm_cm_emergency_res (UBYTE cm_establish_res)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_cm_emergency_res()");

  switch (GET_STATE (STATE_GPRS_CM_EST))
  {
    case CM_GPRS_EST_PEND:    /* Normal case */
    case CM_GPRS_EMERGE_PEND:
      break;

    case CM_GPRS_EST_OK:      /* Clash MO/MT, special case */
      TRACE_EVENT ("MO/MT clash");
      return;

    default:
      TRACE_ERROR ("Unexpected STATE_GPRS_CM_EST");
      break;
  }

  if (cm_establish_res EQ MMGMM_ESTABLISH_OK)
  {
    mm_data->gprs.resumption = MMGMM_RESUMPTION_FAILURE;

    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);

    if (mm_count_connections (CM_NOT_IDLE) EQ 0)
    {
     /* 
      * In the meantime, all the stored connections have been cleared
      * Worst case assumption: MMGMM_RESUMPTION_FAILURE.
      */
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);
    }
    else
    {
      /* 
       * Now all the stored establish requests can be performed
       */
      USE_STORED_ENTRIES();
    }
  } 
  else /* cm_establish_res EQ MMGMM_EMERGENCY_REJECT */ 
  {
    /* Clear outstanding MMGMM_CM_ESTABLISH_IND */
    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
        
    /* Release all connections, there can only be stored connections */ 
    mm_mmxx_rel_ind (MMCS_INT_NOT_PRESENT, CM_NOT_IDLE);
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_attach_started_req         |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_ATTACH_STARTED_REQ primitive.
            GMM indicates MM that GMM starts the combined attach procedure.
            This implies also network mode I.
  
*/

GLOBAL void mm_func_mmgmm_attach_started_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_attach_started_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_IDLE_LUP_NEEDED:
      assert (GET_STATE (STATE_REG_TYPE) EQ REG_CELL_SEARCH_ONLY);
      /*FALLTHROUGH*/
      //lint -fallthrough
    case MM_IDLE_NORMAL_SERVICE:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      
      /* IMSI present physically */
      assert (mm_data->reg.imsi_struct.v_mid EQ V_MID_PRES AND 
              !mm_data->gprs.sim_physically_removed);
  
      /* Revalidate SIM data */
      mm_data->reg.op.sim_ins = SIM_INSRT;

      /* Prevent MM specific procedures (location updating/detach) */
      SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);

      /* 
       * A GPRS MS operating in mode A or B in a network that operates in 
       * mode I should not use any MM timers relating to MM specific 
       * procedures, (e.g T3210, T3211, T3212, T3213) except in some error 
       * and abnormal cases. If the MM timers are already running, the MS 
       * should not react on the expiration of the timers.
       *    
       * NOTE 2: Whenever GMM performs a combined GMM procedure, 
       * a GPRS MS enters the MM state MM LOCATION UPDATING PENDING in order 
       * to prevent the MM to perform a location update procedure.
       *
       * [GSM 04.08 subclause 4.1.1.2.1, 
       *  GPRS MS operating in mode A or B in a network that operates in mode I]
       */
      
      /* 
       * As GMM sends the primitive MMGMM_ATTACH_STARTED_REQ if it begins a combined
       * attach procedure, MM is cautious not to stop any timer here already. 
       * This is left up to the reception of MMGMM_ATTACH_ACC_REQ.
       */

      /* 
       * LOCATION UPDATING PENDING
       * 
       * A location updating has been started using the combined 
       * GPRS routing area updating procedure.
       * 
       * [GSM 04.08 subclause 4.1.2.1.1, Main states]
       */

      /* Remember service state */
      mm_data->idle_substate = mm_get_service_state ();

      //
      // MM_LOCATION_UPDATING_PENDING is a superflous state. 
      // Don't enter it.
      //
      //SET_STATE (STATE_MM, MM_LOCATION_UPDATING_PENDING);
      break;

    default: 
      TRACE_ERROR (UNEXPECTED_IN_STATE);
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_attach_acc                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_ATTACH_ACC_REQ primitive.
            GMM informs MM about the successfull combined attach procedure
            and sends the new TMSI, if available, to MM.
            This implies also network mode I.
  
*/

LOCAL void mm_attach_acc (const T_plmn *plmn,
                          USHORT lac, 
                          UBYTE v_mobile_identity,
                          ULONG mobile_identity,
                          UBYTE v_equ_plmn_list,
                          const T_equ_plmn_list *equ_plmn_list)
{
  GET_INSTANCE_DATA;
  BOOL eplmn_changed;

  TRACE_FUNCTION ("mm_attach_acc()");

  /* All other conditions are an internal protocol failure here */
  assert (GET_STATE (STATE_REG_TYPE) EQ REG_CELL_SEARCH_ONLY);

  /* It is not specified that MM has to confirm a MMGMM_ATTACH_ACC_REQ, 
   * GMM is informed about full service condition */
  mm_data->reg.full_service_indicated = TRUE;

  /* Remember IMSI ATTACH has been done. */
  if (mm_data->first_attach)
  {
    mm_data->first_attach_mem = mm_data->first_attach;
    mm_data->first_attach = FALSE;
  }

  /* Remember MM was attached by combined attach/detach procedures */
  mm_data->gprs.combined_procedures = TRUE;
  mm_data->reg.update_stat = MS_UPDATED;
  mm_data->rej_cause = 0;
  mm_data->attempt_cnt = 0;
  mm_data->loc_upd_type.lut = NOT_RUNNING;

  if (memcmp(mm_data->reg.lai.mnc, plmn->mnc, SIZE_MNC) OR memcmp (mm_data->reg.lai.mcc, plmn->mcc, SIZE_MCC) OR (mm_data->reg.lai.lac NEQ lac))
  {
  /* EF LOCI value has changed, hence write it on SIM */
  /* EF Indicator for EF LOCI - bit 1 - changed value from Attach Accept msg*/
  mm_data->ef_indicator|=0x01;
  }
  /* Copy PLMN and LAC into registration data */
  memcpy (mm_data->reg.lai.mnc, plmn->mnc, SIZE_MNC);
  memcpy (mm_data->reg.lai.mcc, plmn->mcc, SIZE_MCC);
  mm_data->reg.lai.lac = lac;

  if (v_mobile_identity EQ MMGMM_TMSI_USED)
  {
    /* Remember GMM's TMSI to avoid unnecessary synchronization */
    mm_data->reg.indicated_tmsi = mobile_identity;

    if (mobile_identity NEQ mm_data->reg.tmsi)
    {
      /* EF LOCI value has changed, hence write it on SIM */
      /* EF Indicator for EF LOCI - bit 1 - changed value from Attach Accept msg*/
      mm_data->ef_indicator|=0x01;
    }
    if (mobile_identity NEQ MMGMM_TMSI_INVALID)
    {
      mm_data->reg.tmsi = mobile_identity;

      /* Inform RR about changed TMSI */
      mm_build_rr_sync_req_tmsi ();
    }
    else
    {
      mm_data->reg.tmsi = TMSI_INVALID_VALUE;

      /* Inform RR about invalidation of TMSI */
      mm_build_rr_sync_req_cause (SYNCCS_TMSI_INVAL);
    }
  }

  /* Remove the PLMN from the "forbidden PLMN" list */
  reg_plmn_bad_del (mm_data->reg.forb_plmn, MAX_FORB_PLMN_ID, plmn);

  /* Remove the PLMN from the "forbidden PLMNs for GPRS service" list */
  reg_plmn_bad_del (mm_data->reg.gprs_forb_plmn, MAX_GPRS_FORB_PLMN_ID, plmn);

  /* Send RR_SYNC_REQ (Location Area allowed) */
  mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);
  
  /* This is a nasty hack to process 'equ_plmn_list'. The function 'reg_store_eqv_plmns'
   * can only take a 'T_eqv_plmn_list' Currently 'T_equ_plmn_list' is identical to 'T_eqv_plmn_list' 
   * in all but name hence making the casting OK, **for now**. This problem needs to be adressed
   * in the air interface document.
   */
  if (v_equ_plmn_list)
    eplmn_changed = reg_store_eqv_plmns((T_eqv_plmn_list *)equ_plmn_list, (T_plmn *)plmn);
  else
    eplmn_changed = reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE);

  /* Send RR_SYNC_REQ (equivalent plmn list changed) */
  if(eplmn_changed)
    mm_build_rr_sync_req_cause (SYNCCS_EPLMN_LIST);

  /* Inform SIM */
  reg_build_sim_update ();

  /* 
   * We assume the network has updated us for the currently selected cell,
   * otherwise we will run into trouble. 
   */
  assert (mm_check_lai (&mm_data->reg.lai, &mm_data->mm.lai));

  /* Stop all timers used for MM specific procedures */
  TIMERSTOP (T3210); 
  TIMERSTOP (T3211);
  TIMERSTOP (T3212); 
  mm_data->t3212_timeout = FALSE;
  
  TIMERSTOP (T3213);
  mm_data->t3213_restart = 0;
}



/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_attach_acc_req             |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_ATTACH_ACC_REQ primitive.
            GMM informs MM about the successfull combined attach procedure
            and sends the new TMSI, if available, to MM.
            This implies also network mode I.
  
*/

GLOBAL void mm_func_mmgmm_attach_acc_req (const T_plmn *plmn,
                                          USHORT lac, 
                                          UBYTE v_mobile_identity,
                                          ULONG mobile_identity,
                                          UBYTE v_equ_plmn_list,
                                          const T_equ_plmn_list *equ_plmn_list)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_attach_acc_req()");

  switch (GET_STATE (STATE_MM))
  {
    case MM_IDLE_NORMAL_SERVICE:        /* 19.1 */
    case MM_IDLE_ATTEMPT_TO_UPDATE:     /* 19.2 */
    case MM_IDLE_LIMITED_SERVICE:       /* 19.3 */
    case MM_IDLE_LUP_NEEDED:            /* 19.6 */
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
      mm_attach_acc (plmn, lac, v_mobile_identity, mobile_identity, v_equ_plmn_list, equ_plmn_list);
       
      SET_STATE (STATE_MM, MM_IDLE_NORMAL_SERVICE);
      USE_STORED_ENTRIES();
      break;

    case MM_IDLE_PLMN_SEARCH:           /* 19.7 */
      mm_attach_acc (plmn, lac, v_mobile_identity, mobile_identity, v_equ_plmn_list, equ_plmn_list);
       
      SET_STATE (STATE_MM, MM_PLMN_SEARCH_NORMAL_SERVICE);
      USE_STORED_ENTRIES();
      break;
      
    case MM_PLMN_SEARCH_NORMAL_SERVICE: /* 19.8 */
      mm_attach_acc (plmn, lac, v_mobile_identity, mobile_identity, v_equ_plmn_list, equ_plmn_list);
      break;

    default:
      TRACE_ERROR (UNEXPECTED_IN_STATE);
      break;
  }

  /* Check HPLMN timer state */
  reg_check_hplmn_tim (mm_data->reg.thplmn);

}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_allowed_req          |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_ALLOWED_REQ primitive.
            GMM informs MM that the attach procedure was accepted by the 
            network, but for GPRS services only. MM doesn't perform a state 
            change (especially GSM does not become idle updated on the cell
            by receiving this event), but the location area is removed from 
            all forbidden lists.

*/

GLOBAL void mm_func_mmgmm_allowed_req (UBYTE v_equ_plmn_list, const T_equ_plmn_list *equ_plmn_list)
{
  GET_INSTANCE_DATA;
  T_plmn plmn;
  BOOL eplmn_changed;

  TRACE_FUNCTION ("mm_mmgmm_allowed_req()");

  /* Fill up the PLMN structure from the camped cell data */
  plmn.v_plmn = V_PLMN_PRES;
  memcpy (plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
  memcpy (plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);

  /* Remove the PLMN from the "forbidden PLMN" list */
  reg_plmn_bad_del (mm_data->reg.forb_plmn, MAX_FORB_PLMN_ID, &plmn);

  /* Remove the PLMN from the "forbidden PLMNs for GPRS service" list */
  reg_plmn_bad_del (mm_data->reg.gprs_forb_plmn, MAX_GPRS_FORB_PLMN_ID, &plmn);

    /* Send RR_SYNC_REQ (Location Area allowed) */
  mm_build_rr_sync_req_cause (SYNCCS_LAI_ALLOW);

  /* This is a nasty hack to process 'equ_plmn_list'. The function 'reg_store_eqv_plmns'
   * can only take a 'T_eqv_plmn_list' Currently 'T_equ_plmn_list' is identical to 'T_eqv_plmn_list' 
   * in all but name hence making the casting OK, for now. This problem needs to be adressed
   * in the air interface document.
   */
  if (v_equ_plmn_list)
    eplmn_changed = reg_store_eqv_plmns((T_eqv_plmn_list *)equ_plmn_list, (T_plmn *)&plmn);
  else
    eplmn_changed = reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list, EPLMNLIST_SIZE);

  /* Send RR_SYNC_REQ (equivalent plmn list changed) */
  if(eplmn_changed)
    mm_build_rr_sync_req_cause (SYNCCS_EPLMN_LIST);

  /* Update SIM */
  reg_build_sim_update ();

  /* Check HPLMN timer state */
  reg_check_hplmn_tim (mm_data->reg.thplmn);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_attach_rej_req       |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_ATTACH_REJ_REQ primitive.
            GMM informs MM that the attach procedure was rejected by the 
            network with the given reject cause. According to the given 
            cause action in MM is taken. For further details, see 
            GSM 04.08 subclause 4.7.3.1.4

            See also the GSM functions mm_lup_rej() and mm_loc_upd_rej().

            This does *not* imply network mode I here, also other network 
            modes are possible here. MM need not have received the 
            MMGMM_ATTACH_STARTED_REQ message.
  
*/

GLOBAL void mm_func_mmgmm_attach_rej_req (USHORT cs) /* T_MMGMM_ATTACH_REJ_REQ */
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_attach_rej_req()");

  TRACE_EVENT_P1 ("  cs = %d", cs);

  /* All other conditions is an internal protocol failure here */
  assert (GET_STATE (STATE_REG_TYPE) EQ REG_CELL_SEARCH_ONLY);

  /* Check for correct originating entity */
  assert (GET_CAUSE_ORIGIN_ENTITY (cs) EQ 
          GMM_ORIGINATING_ENTITY);

  if (cs EQ GMMCS_GPRS_NOT_ALLOWED)
  {
    /* List of forbidden PLMNs for GPRS services has no meaning anymore */
    reg_clear_plmn_list (mm_data->reg.gprs_forb_plmn, MAX_GPRS_FORB_PLMN_ID);

    return; /* No further impact on GSM side */
  }

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:                /* MM is off, do nothing */
      break;

    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_IMSI_DETACH_INIT:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_CONN_DETACH:
    case MM_WAIT_FOR_REESTABLISH:
      /* Abort to RR */
      mm_abort_connection (ABCS_NORM);
      /*FALLTHROUGH*/
      //lint -fallthrough
    default: /* These states have no existing or requested RR connection */
      /* Release all connections to CM */
      mm_mmxx_rel_ind (cs, CM_NOT_IDLE);

      /* CM control back to GMM */
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_OK);

      mm_data->rej_cause = cs;

      /* 
       * Set the attempt counter to the maximal possible value. This will 
       * ensure no own location updating attempt will be scheduled and MM 
       * will enter the state MM_IDLE_ATTEMPT_TO_UPDATE if an unspecified 
       * cause has been received by the network.
       * An exception is GMMCS_GPRS_NOT_ALLOWED_IN_PLMN which shall
       * not have any influence for GSM (except that the PLMN in question
       * has to be added to the forbidden list for GPRS services).
       */
      if (mm_data->rej_cause NEQ GMMCS_GPRS_NOT_ALLOWED_IN_PLMN)
        mm_data->attempt_cnt = 4;

      mm_loc_upd_rej ();
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_detach_started_req   |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_DETACH_STARTED_REQ primitive.
            GMM indicates MM that GMM starts the detach procedure for GSM.
            MM has to enter state MM_IMSI_DETACH_PENDING.
            See GSM 04.08 subclause 4.7.4.1.
            The reception of this primitive is only possible if network mode I, 
            this means, combined procedures are performed.
  
*/

GLOBAL void mm_func_mmgmm_detach_started_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_detach_started_req()");

  /* Reception of this primitive is a protocol failure if GSM is alone. */
  assert (!mm_gsm_alone());

  /* Remember combinded detach */
  mm_data->gprs.combined_procedures = TRUE;

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:                /* MM is off, do nothing */
    case MM_IMSI_DETACH_PENDING: /* State already entered, do nothing */
      break;

    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_IMSI_DETACH_INIT:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_CONN_DETACH:
    case MM_WAIT_FOR_REESTABLISH:
      /* Abort to RR */
      mm_abort_connection (ABCS_NORM);
      /*FALLTHROUGH*/
      //lint -fallthrough
    default: /* These states have no existing or requested RR connection */
      /* Release all connections to CM */
      mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);

      /* Prevent MM specific procedures (location updating/detach) */
      SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);

      /* Stop all possibly running MM timers */
      TIMERSTOP (T3210);
      TIMERSTOP (T3211);
      TIMERSTOP (T3212);  
      mm_data->t3212_timeout = FALSE;
      TIMERSTOP (T3213);
      mm_data->t3213_restart = 0;
      TIMERSTOP (T3220);
      TIMERSTOP (T3230);
      TIMERSTOP (T3240);

      /* Remember service state */
      mm_data->idle_substate = mm_get_service_state ();
      
      /* Enter state MM_IMSI_DETACH_PENDING */
      // 
      // MM_IMSI_DETACH_PENDING is a superflous state. Don't enter it.
      //      
      //SET_STATE (STATE_MM, MM_IMSI_DETACH_PENDING);
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_start_t3212_req      |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_START_T3212_REQ primitive.
            MMGMM_START_T3212_REQ indicates that GPRS is now detached after 
            combined attach state in network mode I. 
            MM has to start its own timer T3212 with the broadcasted value.

*/

GLOBAL void mm_func_mmgmm_start_t3212_req (void)
{
  GET_INSTANCE_DATA;
  T_TIME t3212_time;

  TRACE_FUNCTION ("mm_func_mmgmm_start_t3212_req()");

  /* Reception of this primitive is a protocol failure if GSM is alone. */
  assert (!mm_gsm_alone());

  /* 
   * If the detach type information element value indicates "GPRS detach
   * without switching off" and the MS is attached for GPRS and non-GPRS
   * services and the network operates in network operation mode I, then 
   * if in the MS the timer T3212 is not already running, the timer T3212 
   * shall be set to its initial value and restarted after the 
   * DETACH REQUEST message has been sent.
   * [GSM 04.08 subclause 4.7.4.1.1, 
   *  MS initiated detach procedure initiation]
   */

  /*
   * If the detach type information element value indicates "re-attach
   * required" or "re-attach not required" and the MS is attached for GPRS
   * and non-GPRS services and the network operates in network operation 
   * mode I, then if in the MS the timer T3212 is not already running, 
   * the timer T3212 shall be set to its initial value and restarted.
   * [GSM 04.08 subclause 4.7.4.2.2, 
   *  Network initiated GPRS detach procedure completion by the MS]
   */
  
  /* 
   * One thing is for sure: MM is not (anymore) attached by combined
   * procedures if its own T3212 is running. On the other hand, GSM 
   * is not alone, but still performs its operation with GMM.
   */
  mm_data->gprs.combined_procedures = FALSE;

  /* 
   * Start timer T3212 with broadcasted value.
   */
  if (mm_data->t3212_cfg_counter NEQ 0 AND 
      mm_data->mm.mm_info.t3212 NEQ 0)
    t3212_time = mm_data->t3212_cfg_counter;
  else
    t3212_time = mm_data->mm.mm_info.t3212 * 36;

  if (t3212_time NEQ 0)
  {
    TIMERSTART (T3212, t3212_time * T_T3212_TIC_VALUE);
  }
  else
  {
    TIMERSTOP (T3212); /* No periodic update timer */
  }
}

#if 0
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_trigger_req          |
+----------------------------------------------------------------------------+

  PURPOSE : Goal of this function is to ensure that the remaining 
            time before expiracy of the T_HPLMN timer is long enough.
            Used to prevent from aborting a data transfer because of 
            a PLMN reselection.

*/

GLOBAL void mm_func_mmgmm_trigger_req (void)
{
  TRACE_FUNCTION ("mm_func_mmgmm_trigger_req()");

  /* Check if running first */
  if (TIMERACTIVE(T_HPLMN))
  {
    T_TIME remaining_time;

    vsi_t_status (VSI_CALLER T_HPLMN, &remaining_time);

    /* Check if THPLMN is close from expiracy */
    if (remaining_time <= TICS_PER_DECIHOURS)
    {
      TRACE_EVENT("Kick HPLMN timer");
      reg_stop_hplmn_tim(); /* Not restarted if already running */
/*      if (mm_full_service_pplmn_scan())
*/
        reg_check_hplmn_tim (1); /* Expiry not before 6 minutes */
    }
  }
}
#endif

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_network_initiated_detach        |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the network initiated detach received
            by GMM and signalled to MM by using the MMGMM_NREG_REQ primitive
            with cs EQ CS_DISABLE.

*/

void mm_network_initiated_detach (USHORT cs)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_network_initiated_detach()");
  
  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL:  
      /* No further action required */
      mm_mmgmm_nreg_cnf (mm_data->nreg_cause);
      break;
    
    case MM_LUP_INITIATED:
    case MM_CONN_ACTIVE:
    case MM_IMSI_DETACH_INIT:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_REJECTED:
      /* 
       * States where the mobile has a layer 2 connection 
       */

      /* 
       * The problem with that all is the following:
       * If we read GSM 04.08, subclause 4.7.4.2, it is not described 
       * what to do with a dedicated layer 2 connection.
       * Entering state MM_WAIT_FOR_NW_CMD and wait for either CHANNEL RELEASE
       * message in RR or for T3240 expiry in MM is not possible as it violates
       * the standard, entering MM_IDLE state is requested.
       * So the only way to get rid of an MM connection is a local release
       * before entering MM_IDLE state.
       * This is not described in GSM 04.08, subclause 4.7.4.2. 
       * Also is not described what to do with (pending) CM connections, compare
       * this with the stardadization of the abort procedure (GSM 04.08, 4.3.5).
       * The conclusion of this all is: It is either not foreseen or 
       * not yet standardized. So it still cannot be implemented in a 
       * standard conformant way. Class A is for further study in MM, really.
       */
  
      /*FALLTHROUGH*/
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_CONN_DETACH:
    case MM_WAIT_FOR_REESTABLISH:
    case MM_WAIT_FOR_RR_ACTIVE:
      /* 
       * States where the mobile has a layer 2 connection requested
       */
      TRACE_ERROR (UNEXPECTED_IN_STATE);
      break; 

    case MM_IDLE_NORMAL_SERVICE:
    case MM_LOCATION_UPDATING_PENDING:
    case MM_IMSI_DETACH_PENDING:
    case MM_IDLE_ATTEMPT_TO_UPDATE:
    case MM_IDLE_LIMITED_SERVICE:
    case MM_IDLE_NO_IMSI:
    case MM_IDLE_NO_CELL_AVAILABLE:
    case MM_IDLE_LUP_NEEDED:
    case MM_IDLE_PLMN_SEARCH:
    case MM_PLMN_SEARCH_NORMAL_SERVICE:
      /* Release all connections with appropriate cause to CM */
      mm_mmxx_rel_ind (cs, CM_NOT_IDLE);

      /* Inform GMM */
      mm_mmgmm_cm_release_ind (MMGMM_RESUMPTION_FAILURE);

      /* Handle the request of GMM like an location updating reject */
      mm_data->rej_cause = cs;
      mm_loc_upd_rej ();
      break;

    default:
      /* All states caught, this means trouble */
      TRACE_ERROR (UNEXPECTED_DEFAULT); 
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_sim_removed_gprs_active         |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the case that the SIM is removed when GPRS 
            is active. An IMSI DETACH cannot be done immediately as MM has no
            knowledge about the network mode in the current design (which is
            regarded as a disadvantage for this case). All calls are thrown
            out and all update activities are to be stopped.

*/

GLOBAL void mm_sim_removed_gprs_active (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_sim_removed_gprs_active()");

  /* Remember the cause for limited service */
  mm_data->limited_cause = MMCS_SIM_REMOVED;

  /* Release all CM connections to upper layers */
  mm_mmxx_rel_ind (MMCS_NO_REGISTRATION, CM_NOT_IDLE);

  /* Stop all running update timers and running attempts */
  TIMERSTOP (T_REGISTRATION);
  TIMERSTOP (T3210);
  TIMERSTOP (T3211);
  TIMERSTOP (T3212);
  TIMERSTOP (T3213);
  mm_data->t3213_restart = 0;
  mm_data->loc_upd_type.lut = NOT_RUNNING;
  mm_data->idle_entry = RRCS_INT_NOT_PRESENT;

  switch (GET_STATE (STATE_MM))
  {
    /* Dedicated or dedicated requested states */
    case MM_LUP_INITIATED:
    case MM_WAIT_FOR_OUTG_MM_CONN:
    case MM_CONN_ACTIVE:
    case MM_IMSI_DETACH_INIT:
    case MM_PROCESS_PROMPT:
    case MM_WAIT_FOR_NW_CMD:
#ifdef REL99
    case MM_RR_CONN_RELEASE_NOT_ALLOWED:
#endif
    case MM_LUP_REJECTED:
    case MM_WAIT_FOR_RR_CONN_LUP:
    case MM_WAIT_FOR_RR_CONN_MM:
    case MM_WAIT_FOR_RR_CONN_DETACH:
      /* Abort RR connection */
      mm_abort_connection (ABCS_NORM);

      /* No state change, wait for RR_RELEASE_IND in old state */
      break;

    default: /* All other states, especially IDLE states */
      if (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED)
      {
        /* Indicate end of RC update to GMM */
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
      }
      break;
  }
}

#endif /* GPRS */

#endif /* MM_GPRS_C */

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_REGP
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
|  Purpose :  This Modul defines the functions for the registration
|             capability of the module Mobility Management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_REGP_C
#define MM_REGP_C

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

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*==== PRIVAT =====================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : mm_auto_net_reg            |
+--------------------------------------------------------------------+

  PURPOSE : Register in automatic mode.

*/

GLOBAL void mm_auto_net_reg(void)
{
  GET_INSTANCE_DATA;
  T_plmn last_plmn;

  TRACE_FUNCTION("mm_auto_net_reg");

  last_plmn.v_plmn = V_PLMN_PRES;
  memcpy (last_plmn.mcc, mm_data->reg.lai.mcc, SIZE_MCC);
  memcpy (last_plmn.mnc, mm_data->reg.lai.mnc, SIZE_MNC);

  /*
   * start full PLMN search in automatic mode
   */
  mm_data->attempt_cnt = 0;
   
  mm_data->reg.bcch_encode =  
    (mm_data->reg.update_stat EQ MS_UPDATED AND !reg_plmn_empty (&last_plmn));
  /*
   * It exists an updated last PLMN if bcch_encode is TRUE here
   */

  mm_data->reg.plmn_cnt = 0; /* Delete list of available PLMNs */
  if (mm_data->reg.bcch_encode AND !reg_plmn_equal_hplmn (&last_plmn))
  {
    TRACE_EVENT ("Start with LPLMN");

    /* LPLMN available and not equal HPLMN => start with LPLMN */
    mm_data->reg.actual_plmn = last_plmn; /* Struct copy */
  }
  else
  {
    TRACE_EVENT ("Start with HPLMN");

    /* LPLMN not available or not updated => start with HPLMN */
    reg_extract_hplmn (&mm_data->reg.actual_plmn);
  }

  EM_START_REGISTRATION_AUTO_MODE;

  mm_mmr_reg_req (FUNC_PLMN_SRCH);
}


#ifdef GPRS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_set_gprs_reg_type      |
+--------------------------------------------------------------------+

  PURPOSE : Set the new registration type as delivered by GMM. 
            This may also affect the CM_GPRS_EST state machine.

*/


LOCAL void reg_set_gprs_reg_type (UBYTE reg_type)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_set_gprs_reg_type()");

  switch (reg_type)
  {
    case REG_GPRS_INACTIVE: 
      /* CM establishment always allowed if GPRS is not present */
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);
      break;

    case REG_REMOTE_CONTROLLED:
      /* Remote controlled location updating not possible with GSM only */
      assert (GET_STATE (STATE_REG_TYPE) NEQ REG_GPRS_INACTIVE);

      /* During RC for non GPRS only mobile CM services are allowed */
      if (mm_data->gprs.mobile_class NEQ MMGMM_CLASS_CG)
      {
        SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_OK);
      }
      break;

    case REG_CELL_SEARCH_ONLY:
      if (mm_gsm_alone())
      {
        /* CM establishment not allowed anymore if GPRS was switched on */
        SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
      }
      break;

    default:
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      return;
  }

  /* Remember new MM working type */
  SET_STATE (STATE_REG_TYPE, reg_type);
}
#endif /* GPRS */

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_reg_gsm_only_req                |
+----------------------------------------------------------------------------+

  PURPOSE : This function perform registration in GSM mode only.
            (What about cell selection only?) ###
  
*/

LOCAL void mm_reg_gsm_only_req (UBYTE service_mode)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_reg_gsm_only_req()");

  mm_data->reg.full_service_indicated = FALSE;

  /* 
   * If SIM is inserted, make it valid for MM if 
   * IMSI is present and full service required,
   * otherwise invalidate SIM for MM
   */
  switch (service_mode)
  {
    case SERVICE_MODE_FULL:
      if (mm_data->reg.imsi_struct.v_mid EQ V_MID_PRES)
      {
        mm_data->reg.op.sim_ins = SIM_INSRT;
        mm_data->limited_cause = MMCS_INT_NOT_PRESENT;
      }
      break;

    case SERVICE_MODE_LIMITED:
      if (mm_data->reg.op.sim_ins NEQ SIM_NO_INSRT)
      {
        mm_data->reg.op.sim_ins = SIM_NO_INSRT;
        mm_data->limited_cause = MMCS_SIM_REMOVED; /* MMCS_SIM_INVAL_MMIREQ */
      }
      break;

    default:
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      break;
  }

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /*
     * No valid SIM inserted
     */
    mm_data->reg.bcch_encode = FALSE;
    mm_data->reg.op.v_op     = V_OP_PRES;
    mm_data->reg.op.m        = M_AUTO;
    mm_mmr_reg_req (FUNC_LIM_SERV_ST_SRCH);
  }
  else
  {
    /* 
     * Valid SIM inserted
     */
    if (mm_data->reg.op.m EQ M_AUTO)
    {
      mm_auto_net_reg ();
    } 
    else
    {
      /*
       * PLMN search in manual mode: Request PLMN list from RR
       */
      // Patch HM >>>
      mm_data->plmn_scan_mmi = TRUE;
      // Patch HM <<<
      mm_mmr_reg_req (FUNC_NET_SRCH_BY_MMI);

      EM_START_REGISTRATION_MANUAL_MODE;

    }
  }
}

/*==== TEST =====================================================*/

/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_net_req              |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_NET_REQ primitive.
            MMGMM_NET_REQ is always used to start a network search.
  
*/

GLOBAL void mm_func_mmgmm_net_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_net_req()");

  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /* 
     * SIM present 
     */
    switch (GET_STATE (STATE_MM))
    {
      case MM_NULL:
        /* prepare environement for further processing by RR */
        
        mm_data->reg.op.m = MODE_MAN;
        reg_clear_plmn (&mm_data->reg.actual_plmn);
        
        mm_mmr_reg_req (FUNC_PLMN_SRCH);
        break;

      default:
        mm_mmr_reg_req (FUNC_NET_SRCH_BY_MMI);
        break;
    }
    
    EM_START_PLMN_LIST_REQUEST;   
   
  }
  else
  {
    /* 
     * SIM not present
     */
    /*If Dual SIM is supported by MS then initially before SIM activation
    Network search should be done.Also, anytime network search is asked for
    MM should start with PLMN scan and return proper PLMN list*/
#ifdef FF_DUAL_SIM
    TRACE_EVENT("Dual sim is supported and no sim");
    mm_mmr_reg_req (FUNC_NET_SRCH_BY_MMI);
#else
    mm_mmgmm_plmn_ind (MMCS_SIM_REMOVED, NULL);
#endif
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_nreg_req             |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_NREG_REQ primitive.
  
*/

GLOBAL void mm_func_mmgmm_nreg_req (UBYTE detach_cause, 
                                    UBYTE detach_done,
                                    USHORT cs)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_nreg_req()");

  /* 
   * Parameter check from ACI/GMM
   */
#ifdef GPRS

#ifdef WIN32
  TRACE_EVENT_P1 ("detach_cause = %02X", detach_cause);
  TRACE_EVENT_P1 ("detach_done  = %02X", detach_done);
  TRACE_EVENT_P1 ("cs           = %04X", cs);
#endif /* #ifdef WIN32 */

  /* Check for correct originating entity */
  assert (GET_CAUSE_ORIGIN_ENTITY (cs) EQ 
          GMM_ORIGINATING_ENTITY);

  /* Check for correct detach_done parameter */  
  assert ((detach_done EQ MMGMM_PERFORM_DETACH) OR 
          (detach_done EQ MMGMM_DETACH_DONE));
#else
  assert (detach_done EQ MMGMM_PERFORM_DETACH);
#endif /* GPRS */

  /* Remember that the deregistration was requested by MMI */
  mm_data->nreg_request = TRUE;

  /* Remember the cause of the deregistration */
  mm_data->nreg_cause = detach_cause; 
  
  /* 
   * Timer T3212 is stopped if the mobile station is deactivated
   * (i.e. equipment powered down or SIM removed.
   */
  TIMERSTOP (T3212);
  mm_data->t3212_timeout = FALSE;

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /*
     * No valid SIM inserted
     */
    switch (detach_cause) 
    {
      case CS_POW_OFF:
      case CS_SOFT_OFF:
        mm_mmr_nreg_req (detach_cause, detach_done);
        break;

      case CS_SIM_REM:
        mm_mmgmm_nreg_cnf (detach_cause); /* SIM already removed */
        break;

#ifdef GPRS
      case CS_DISABLE:
        mm_mmgmm_nreg_cnf (detach_cause); /* Already detached */
        break;
#endif /* GPRS */

      default:
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
  }
  else
  {
    /*
     * Valid SIM inserted
     */
    switch (detach_cause)
    {
      case CS_SIM_REM:
        mm_data->limited_cause = MMCS_SIM_REMOVED; /* MMCS_SIM_INVAL_MMIREQ */
        //lint -fallthrough
      case CS_POW_OFF:
      case CS_SOFT_OFF:
        mm_mmr_nreg_req (detach_cause, detach_done);
        break;

#ifdef GPRS
      case CS_DISABLE:
        /* 
         * Check deregistration cause from GMM 
         */
        switch (cs)
        {
          case GMMCS_IMSI_UNKNOWN:          /* #2  */
          case GMMCS_ILLEGAL_MS:            /* #3  */
          case GMMCS_ILLEGAL_ME:            /* #6  */
          case GMMCS_GSM_GPRS_NOT_ALLOWED:  /* #8  */
          case GMMCS_PLMN_NOT_ALLOWED:      /* #11 */
          case GMMCS_LA_NOT_ALLOWED:        /* #12 */
          case GMMCS_ROAMING_NOT_ALLOWED:   /* #13 */
#ifdef REL99
          case GMMCS_ROAMING_NOT_ALLOWED_WITH_RAU_REJ:  /* #13 GPRS RAU rejected*/
          case GMMCS_NO_SUITABLE_CELL_IN_LA:  /* #15 GPRS attach rejected*/
          case GMMCS_NO_SUITABLE_CELL_IN_LA_WITH_RAU_REJ:  /* #15 GPRS RAU rejected*/
#endif
            mm_network_initiated_detach (cs);
            break;
          
          case GMMCS_GPRS_NOT_ALLOWED:      /* #7 */
            /*
             * # 7 (GPRS services not allowed)
             * The MS shall set the GPRS update status to GU3 
             * ROAMING NOT ALLOWED (and shall store it according to 
             * section 4.1.3.2) and shall delete any P-TMSI, P-TMSI signature,
             * RAI and GPRS ciphering key sequence number. The SIM shall be 
             * considered as invalid for GPRS services until switching off or 
             * the SIM is removed. The new state is GMM-DEREGISTERED.
             * [GSM 04.08 subclause 4.7.3.1.4].
             *
             * This is a GPRS only cause. MM starts automatically T3212 
             * without the need to receive MMGMM_START_T3212_REQ.
             * It is expected that GMM will send a MMGMM_REG_REQ 
             * (REG_GPRS_INACTIVE) immediately after this to trigger a 
             * registration attempt.
             */

            /* As this is a GPRS only reason, this should not have any
             * impact on the update status of GSM. But T3212 has to be
             * started now.
             */
            mm_func_mmgmm_start_t3212_req ();
            TRACE_EVENT ("No impact on update status");
            mm_mmgmm_nreg_cnf (detach_cause);
            break;
          
          case GMMCS_INT_NOT_PRESENT:
            /* 
             * Start remote controlled IMSI DETACH operation 
             */
            mm_mmr_nreg_req (detach_cause, detach_done);
            break;

          default:
            TRACE_EVENT ("No impact on update status and nothing done!");
            mm_mmgmm_nreg_cnf (detach_cause);
            break;
        }
        break; /* CS_DISABLE */
#endif /* GPRS */

      default:
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
  }

  /* Check HPLMN timer state */
  reg_check_hplmn_tim (mm_data->reg.thplmn);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_plmn_mode_req        |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_PLMN_MODE_REQ primitive.
            MMGMM_PLMN_MODE_REQ is used to switch between automatic and
            manual mode of registration.
  
*/

GLOBAL void mm_func_mmgmm_plmn_mode_req (UBYTE mode)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_plmn_mode_req()");

#ifndef NTRACE
  switch (mode)
  {
    case MODE_AUTO: 
      TRACE_EVENT ("automatic mode");
      break;

    case MODE_MAN:
      TRACE_EVENT ("manual mode");
      break;

    default:
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      break;
  }
#endif /* NTRACE */

  /* 
   * If SIM is inserted, make it valid for MM if IMSI is present
   */
  if (mm_data->reg.imsi_struct.v_mid EQ V_MID_PRES)
  {
    mm_data->reg.op.sim_ins = SIM_INSRT;
    mm_data->limited_cause = MMCS_INT_NOT_PRESENT;
  }

  mm_data->reg.op.m = mode;

  switch (GET_STATE (STATE_MM))
  {
    case MM_NULL: /* Mobile is off, do nothing */
      break;

    // This is just for issue 15605 should be asap removed by an clearly implemented concept for RR_SYNC interface
    case MM_IDLE_NO_IMSI: /* Mobile has no SIM or received e.g. cause MMCS_IMSI_IN_HLR" ... */
      if (mode EQ M_MAN)
        mm_data->reg.op.func = FUNC_LIM_SERV_ST_SRCH;
    //lint -fallthrough
    default:
      mm_build_rr_sync_req(MSG_MM_MODE);
      break;
  }

  /* Check HPLMN timer state */
  reg_check_hplmn_tim (mm_data->reg.thplmn);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_plmn_res             |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_PLMN_RES primitive.
            MMGMM_PLMN_RES is used to select a PLMN manually after 
            network search or to select RPLMN at switch on in manual mode
  
*/

GLOBAL void mm_func_mmgmm_plmn_res (const T_plmn *plmn, 
                                          UBYTE reg_type,
                                          UBYTE mobile_class)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_plmn_res ()");

  mm_data->reg.full_service_indicated = FALSE;

#ifdef GPRS
  mm_data->gprs.mobile_class = mobile_class;

  if ((reg_type EQ REG_REMOTE_CONTROLLED) AND
      (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED))
  {
    /* 
     * GMM tries to retrigger a remote controlled RC procedure while 
     * this is already running. MM ignores this and continues updating.
     */
    TRACE_EVENT ("No retrigger of RC update");
    return;
  }

  reg_set_gprs_reg_type (reg_type);
#endif /* GPRS */

  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /*
     * A valid SIM is inserted
     */
    mm_data->reg.plmn_cnt = 0; /* Delete list of available PLMNs */

#ifdef GPRS
    switch (reg_type)
    {
      case REG_GPRS_INACTIVE:
      case REG_CELL_SEARCH_ONLY:
        reg_select_network(plmn);
        break;

      case REG_REMOTE_CONTROLLED:
        {
          /* 
           * First, we check whether GMM sent a correct PLMN. 
           * All other cases are a protocol failure and MM cannot do much.
           */ 
          T_plmn camped_plmn;
    
          camped_plmn.v_plmn = V_PLMN_PRES;
          memcpy (camped_plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
          memcpy (camped_plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);

          assert (reg_plmn_equal_sim (&camped_plmn, plmn));
        }
        
        /* Perform a remote controlled location updating procedure */
        mm_gprs_update_req ();
        break;

      default:
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
#else
    reg_select_network(plmn);
#endif /* GPRS */
  } 
  else
  {
    /*
     * No valid SIM inserted, selection a network manually makes no sense.
     */
    mm_func_mmgmm_reg_req (SERVICE_MODE_FULL, reg_type, MMGMM_CLASS_CC, NORMAL_REG);
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                              |
| STATE   : code                ROUTINE : mm_func_mmgmm_reg_req              |
+----------------------------------------------------------------------------+

  PURPOSE : This function handles the MMGMM_REG_REQ primitive,
            but it is the functional interface.
  
*/

GLOBAL void mm_func_mmgmm_reg_req (UBYTE service_mode,
                                   UBYTE reg_type,
                                   UBYTE mobile_class,
                                   UBYTE bootup_act)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_func_mmgmm_reg_req()");

#ifdef TRACE_FUNC
/* Implements Measure#32: Row 158 & 159 */
  TRACE_EVENT_P1 ("  service_mode = %d", service_mode);
  TRACE_EVENT_P1 ("  reg_type = %d", reg_type);
#endif

  /* Changes for Boot Time Speedup. MM will get dummy REG_REQ indicating QUICK_REG. In this case
   * MM has to send RR_ACTIVATE_REQ with op.func = FUNC_ST_PWR_SCAN. No need to process this as 
   * it is dummy request and MM will get other REG_REQ indicating NORMAL_REG
   */
  if (bootup_act EQ QUICK_REG)
  {
    mm_data->reg.op.v_op = V_OP_PRES;
    mm_data->reg.op.func = FUNC_ST_PWR_SCAN;
    mm_rr_act_req ();
    return ;
  }
  /* 
   * Check incoming parameters from GMM
   */
  assert (service_mode EQ SERVICE_MODE_LIMITED OR
          service_mode EQ SERVICE_MODE_FULL);

  mm_data->reg.full_service_indicated = FALSE;

#ifdef GPRS

  mm_data->gprs.mobile_class = mobile_class;

  if ((reg_type EQ REG_REMOTE_CONTROLLED) AND
      (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED))
  {
    /* 
     * GMM tries to retrigger a remote controlled RC procedure while 
     * this is already running. MM ignores this and continues updating.
     */
    TRACE_EVENT ("No retrigger of RC update");
    return;
  }

  reg_set_gprs_reg_type (reg_type);

  switch (reg_type)
  {
    case REG_CELL_SEARCH_ONLY:
    case REG_GPRS_INACTIVE:
      mm_reg_gsm_only_req (service_mode);
      break;

    case REG_REMOTE_CONTROLLED:
      /* 
       * This is the request not to start a cell selection but to perform a 
       * remote controlled location update procedure for GSM. 
       */

      if (mm_data->gprs.sim_physically_removed)
      {
        mm_sim_removed_gprs_active ();
        return;
      }

      /* Actually start the location updating procedure */
      mm_gprs_update_req();
      break;

    default: 
      TRACE_ERROR (UNEXPECTED_PARAMETER);
      break;
  }
#else
  mm_reg_gsm_only_req (service_mode);
#endif /* GPRS */
}


#ifndef GPRS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_net_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMR_NET_REQ.

*/

GLOBAL void reg_mmr_net_req (T_MMR_NET_REQ * mmr_net_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_mmr_net_req()");

  /* Mark the network search as beeing for the MMI */
  mm_data->plmn_scan_mmi = TRUE;

  /* Start scanning for available PLMNs */
  mm_func_mmgmm_net_req ();

  PFREE (mmr_net_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_nreg_req           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMR_NREG_REQ.

*/

GLOBAL void reg_mmr_nreg_req (T_MMR_NREG_REQ *mmr_nreg_req)
{
  TRACE_FUNCTION ("reg_mmr_nreg_req()");

  /* Use internal functional interface, set GSM only default parameters */
  mm_func_mmgmm_nreg_req (mmr_nreg_req->detach_cause, 
                          MMGMM_PERFORM_DETACH,
                          MMGMM_NO_ERROR);

  PFREE (mmr_nreg_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_plmn_mode_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMR_PLMN_MODE_REQ

*/

GLOBAL void reg_mmr_plmn_mode_req (T_MMR_PLMN_MODE_REQ *plmn_mode_req)
{
  TRACE_FUNCTION ("reg_mmr_plmn_mode_req()");

  /* Use internal functional interface */
  mm_func_mmgmm_plmn_mode_req (plmn_mode_req->mode);

  EM_SET_PLMN_SEARCH_MODE;  
  PFREE (plmn_mode_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_plmn_res           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMR_PLMN_RES.

*/

GLOBAL void reg_mmr_plmn_res (T_MMR_PLMN_RES *mmr_plmn_res)
{
  TRACE_FUNCTION ("reg_mmr_plmn_res()");

  /* Use internal functional interface */
  mm_func_mmgmm_plmn_res (&mmr_plmn_res->plmn, 
                          REG_GPRS_INACTIVE,
                          MMGMM_CLASS_CC); 

  PFREE (mmr_plmn_res);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_reg_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive MMR_REG_REQ.

*/

GLOBAL void reg_mmr_reg_req (T_MMR_REG_REQ *mmr_reg_req)
{
  TRACE_FUNCTION ("reg_mmr_reg_req()");

  /* Use internal functional interface */
  mm_func_mmgmm_reg_req (mmr_reg_req->service_mode, 
                         REG_GPRS_INACTIVE,
                         MMGMM_CLASS_CC,
                         mmr_reg_req->bootup_act);

  PFREE (mmr_reg_req);
}
#endif /* GPRS */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_auth_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_AUTHENTICATION_CNF.

*/

GLOBAL void reg_sim_auth_cnf (T_SIM_AUTHENTICATION_CNF *sim_auth_cnf)
{
  TRACE_FUNCTION ("reg_sim_auth_cnf()");

  mm_mmr_auth_cnf (sim_auth_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_mm_insert_ind      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_MM_INSERT.

*/

GLOBAL void reg_sim_mm_insert_ind (T_SIM_MM_INSERT_IND *sim_mm_insert_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_sim_mm_insert()");

#ifdef GPRS
  mm_data->gprs.sim_physically_removed = FALSE;
#endif /* #ifdef GPRS */

  /*SIM_SYNC_REQ to be sent after reading EFs*/
  mm_data->reg.sim_sync_req_pending = TRUE;

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /*
     * No SIM card was inserted, now it is in and the coresponding handling should happen.
     */
    reg_copy_sim_data (sim_mm_insert_ind);

    EM_SIM_INSERT;

    PFREE (sim_mm_insert_ind);
  }
  else
  {
    /* 
     * This happens by SAT activity while SIM is inserted
     */
    T_imsi_struct new_imsi_struct;

    reg_read_imsi(&new_imsi_struct, &sim_mm_insert_ind->imsi_field);

    if (! reg_imsi_equal(&new_imsi_struct, &mm_data->reg.imsi_struct))
    {
      /*
       * IMSI changed. MM Restart procedure, GSM 03.22 subclause 4.10:
       * "To perform the procedure the MS shall behave as if 
       * the SIM is removed and afterwards a new SIM is inserted"
       */

      /* 
       * Remember the primitive data. Value sim_mm_insert_info NEQ NULL means 
       * the MM Restart procedure is active. All actions normally performed
       * if SIM inserted will be done after RR_ABORT_IND, RR_RELEASE_IND or 
       * T3220 timeout received or immediately in mm_mmr_nreg_req().
       */
      if (mm_data->reg.sim_insert_info NEQ NULL)
        PFREE (mm_data->reg.sim_insert_info); /* Not expected to happen */
      mm_data->reg.sim_insert_info = sim_mm_insert_ind; 

      mm_mmr_nreg_req (CS_SIM_REM, MMGMM_PERFORM_DETACH);
      if (mm_data->reg.sim_insert_info NEQ NULL)
      {
        /* 
         * SIM insert not already performed. e.g. IMSI detach running.
         * Clear the registration data, but remember the pointer to the
         * new SIM data, the registration mode and the actual plmn.
         * sim_sync_req_pending which gets reset in reg_init() is set 
         * back to TRUE 
         */
        T_plmn old_plmn;
        UBYTE old_mode;
        
        old_mode = mm_data->reg.op.m;
        old_plmn = mm_data->reg.actual_plmn; /* Structure copy */
        reg_init ();
        mm_data->reg.sim_insert_info = sim_mm_insert_ind;
        mm_data->reg.op.m = old_mode;
        mm_data->reg.actual_plmn = old_plmn;
        mm_data->reg.sim_sync_req_pending = TRUE;

      }

      EM_SIM_INSERT;

    }
    else
    {
      /* 
       * IMSI not changed
       */
      USHORT old_acc_class;
      UBYTE  old_thplmn;

      old_acc_class = mm_data->reg.acc_class;
      old_thplmn    = mm_data->reg.thplmn;

      reg_copy_sim_data (sim_mm_insert_ind);

      if (old_acc_class NEQ mm_data->reg.acc_class)
      {
        mm_build_rr_sync_req_cause (SYNCCS_ACCC);
      }

      if (old_thplmn NEQ mm_data->reg.thplmn)
      {
        reg_stop_hplmn_tim ();
        reg_check_hplmn_tim (mm_data->reg.thplmn);
      }

      PFREE (sim_mm_insert_ind);
    }
  }
  check_if_cingular_sim();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_mm_info_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_MM_INFO_IND.

*/

GLOBAL void reg_sim_mm_info_ind (T_SIM_MM_INFO_IND *sim_mm_info_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_sim_mm_info_ind()");

  switch(sim_mm_info_ind->datafield)
  {
#ifdef REL99
    case SIM_UCPS_ACTEC:
      mm_data->reg.upd_sim_ucps_at = SAT_READ_FILE;
      mm_data->reg.sim_ucps_at_len = NOT_PRESENT_16BIT;
      /*
       * If EFPLMNwAcT is changed, MM must have to re-read EFOPLMNwAcT to 
       * make a complete list of pref_plmn.
       */
      mm_data->reg.upd_sim_ocps_at = SAT_READ_FILE;
      mm_data->reg.sim_ocps_at_len = NOT_PRESENT_16BIT;
      /*
       * Read indicated EF in SIM_MM_INFO_IND from SIM.
       */
      /*Set indicatort sim reading is in progress to true*/
      mm_data->reg.sim_read_in_progress = TRUE;
      reg_read_next_sim_file();
      break;
#endif

    case SIM_PLMNSEL:
#ifdef REL99
      if (mm_data->reg.sim_uocps_at_used EQ FALSE)
#endif
      {
        mm_data->reg.upd_sim_plmnsel = SAT_READ_FILE;
        mm_data->reg.sim_plmnsel_len = NOT_PRESENT_16BIT;

        /*Set indicatort sim reading is in progress to true*/
        mm_data->reg.sim_read_in_progress = TRUE;

        /*
         * Read indicated EF in SIM_MM_INFO_IND from SIM.
         */
        reg_read_next_sim_file();
      }  
      break;

    default:
      break;
  }

  if(GET_STATE (STATE_MM) == MM_IDLE_NORMAL_SERVICE)
  {
    reg_check_hplmn_tim(mm_data->reg.thplmn);
  }
  PFREE (sim_mm_info_ind);
}

 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_remove_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_REMOVE_IND.

*/

GLOBAL void reg_sim_remove_ind (T_SIM_REMOVE_IND *sim_remove_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_sim_remove_ind()");

  mm_data->limited_cause = MMCS_SIM_REMOVED; /* MMCS_SIM_INVAL_REMOVED */

  // Better now instead after cause concept implementation in SIM also:
  // mm_data->limited_cause = sim_remove_ind->cause;
  //
  // Problem: None. But this affects not only the MM testcases, but 
  // also ACI code. And as of the time of this writing it's a too
  // big risk to incorporate this in both entities. By the way, 
  // also GMM should be enhanced in the future and at least a comment
  // incorporated in the respective SAPs that also SIM causes are transported.

  if (mm_data->reg.sim_insert_info NEQ NULL) 
  {
    /* 
     * Pending MM RESTART procedure and now the SIM is physically removed. 
     * Forget about the MM RESTART procedure.
     */
    PFREE (mm_data->reg.sim_insert_info);
    mm_data->reg.sim_insert_info = NULL;
  }

  if (mm_gsm_alone())
  {
    /*
     * Clear SIM data for lower layer, start IMSI Detach, 
     * indicate limited service etc.
     */
    if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
      mm_mmr_nreg_req (CS_SIM_REM, MMGMM_PERFORM_DETACH);
    reg_init ();
  }
#ifdef GPRS
  else
  {
    /* Remember there was a physical SIM removal */
    mm_data->gprs.sim_physically_removed = TRUE;

    /* Commenting the OMAPS00048777 changes as it was a incomplete workaround.
        Refer the analysis section of the defect 71208 for details */

/*    if(GET_STATE (STATE_MM) NEQ MM_CONN_ACTIVE)
    {*/
      mm_sim_removed_gprs_active ();
/*    }*/

    /*
     * Possible IMSI DETACH and entering of MM_IDLE_NO_IMSI state will happen
     * after GMM sends
     * MMGMM_NREG_REQ (CS_SIM_REM, MMGMM_PERFORM_DETACH/MMGMM_DETACH_DONE).
     * Here no IMSI DETACH and invalidation of SIM data can be done as a
     * combined attach maybe performed in network mode I.
     */
  }
#endif /* GPRS */

  EM_SIM_REMOVE;

  /* Debug hack. If they have a SIM driver problem, we hopefully will see it */
  mm_data->mm_idle_no_imsi_marker = 128;
  PFREE (sim_remove_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_sync_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_SYNC_CNF.

*/

GLOBAL void reg_sim_sync_cnf (T_SIM_SYNC_CNF *sim_sync_cnf)
{
  TRACE_FUNCTION ("reg_sim_sync_cnf()");

  /*Nothing to be done. Just free the primitive to avoid memory leaks*/
  PFREE(sim_sync_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_file_update_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_FILE_UPDATE_IND.
            The worst case which may by caused by this primitive
            is a RR_ACTIVATE_REQ to inform RR about a changed
            HPLMN search period or a changed access class.
            It is guaranteed by GSM 11.14 subclause 6.4.7.1
            that here no field is changed which causes MM Restart procedure.


*/
GLOBAL void reg_sim_file_upd_ind (T_SIM_FILE_UPDATE_IND *file_upd)
{
  GET_INSTANCE_DATA;
  USHORT i;

  TRACE_FUNCTION ("reg_sim_file_upd_ind()");
  /* 
   * This flag is required to prevent MM to send Acknowledge completion of file update
   * if file reading was because of SIM_MM_INSERT_IND/SIM_MM_INFO_IND.
   */
  mm_data->reg.sim_file_upd_ind_rec = TRUE;

  for (i = 0; i < file_upd->val_nr; i++)
  {
     if( file_upd->file_info[i].v_path_info EQ TRUE       AND
         file_upd->file_info[i].path_info.df_level1   EQ SIM_DF_GSM AND
         file_upd->file_info[i].path_info.v_df_level2 EQ FALSE )
     {
       switch (file_upd->file_info[i].datafield)
       {
          case SIM_ACC:  /* Access control class */
            mm_data->reg.upd_sim_acc = SAT_READ_FILE;
            break;

          case SIM_PLMNSEL: /* Preferred PLMN list */
            mm_data->reg.upd_sim_plmnsel = SAT_READ_FILE;
            mm_data->reg.sim_plmnsel_len = NOT_PRESENT_16BIT;
            /*Set indicator sim reading is in progress to true*/
            mm_data->reg.sim_read_in_progress = TRUE;
            break;

          case SIM_HPLMN: /* Home PLMN search period */
            mm_data->reg.upd_sim_hplmn = SAT_READ_FILE;
            break;

          case SIM_FPLMN: /* Forbidden PLMN list */
            mm_data->reg.upd_sim_fplmn = SAT_READ_FILE;
            break;

#ifdef REL99
          case SIM_UCPS_ACTEC: /* User controlled PLMN selector with access technology list */
            mm_data->reg.upd_sim_ucps_at = SAT_READ_FILE;
            mm_data->reg.sim_ucps_at_len = NOT_PRESENT_16BIT;
            /*
             * If EFPLMNwAcT is changed, MM must have to re-read EFOPLMNwAcT to 
             * make acomplete list of pref_plmn. Actual length which came in 
             * SIM_MM_INSERT_IND can be used for EFOPLMNwAcT since file is unchanged.
             */
            mm_data->reg.upd_sim_ocps_at = SAT_READ_FILE;
            /*Set indicator sim reading is in progress to true*/
            mm_data->reg.sim_read_in_progress = TRUE;
          break;
      
          case SIM_OCPS_ACTEC: /* Operator controlled PLMN selector with access technology list*/
            /*If only this file is modified MM need not to re-read EFPLMNwAcT*/
            mm_data->reg.upd_sim_ocps_at = SAT_READ_FILE;
            mm_data->reg.sim_ocps_at_len = NOT_PRESENT_16BIT;
            /*Set indicator sim reading is in progress to true*/
            mm_data->reg.sim_read_in_progress = TRUE;
            break;
#endif

          case SIM_IMSI:
          case SIM_LOCI: /* Does not happen this way, this is SIM_MM_INSERT_IND */
            /* FALLTHROUGH */ 
          case SIM_BCCH:
          case SIM_KC: /* Guaranteed to not happen */
            TRACE_ERROR ("File change not handled");
            break;
      
          default: /* MM is not interested in this elementary file */
            break;
       }
     }
     else if((file_upd->file_info[i].path_info.df_level1 EQ SIM_DF_CING) AND 
            (file_upd->file_info[i].path_info.v_df_level2 EQ TRUE) AND 
            (file_upd->file_info[i].path_info.df_level2 EQ SIM_DF2_CING) AND
            (file_upd->file_info[i].datafield EQ SIM_CING_AHPLMN))
     {
        mm_data->reg.upd_sim_act_hplmn = SAT_READ_FILE;
     }
  }

  PFREE (file_upd);
  
  /* Start reading SIM elementary files */
  if (reg_read_next_sim_file() EQ FALSE)
  {
    /* 
     * Nothing interesting for MM. Acknowledge completion of file update
     */
    PALLOC (update_res, SIM_FILE_UPDATE_RES); /* T_SIM_FILE_UPDATE_RES */
    update_res->source = SRC_MM;
    update_res->fu_rsc = SIM_FU_SUCCESS;
    PSENDX (SIM, update_res);
    mm_data->reg.sim_file_upd_ind_rec = FALSE;

  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_send_sim_sync_req      |
+--------------------------------------------------------------------+

  PURPOSE : Sends the primitive SIM_SYNC_REQ with cause.

*/

GLOBAL void reg_send_sim_sync_req (void)
{
  GET_INSTANCE_DATA;
  PALLOC (sync_req, SIM_SYNC_REQ); 
  sync_req->synccs = (U8)SYNC_MM_FINISHED_READING;
  PSENDX (SIM, sync_req);
  mm_data->reg.sim_sync_req_pending = FALSE; 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_sim_read_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_READ_CNF.

*/

GLOBAL void reg_sim_read_cnf (T_SIM_READ_CNF *sim_read_cnf)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_sim_read_cnf()");

  switch (mm_data->sim_read_req_data_field)
  {
    case SIM_ACC:  /* Access control class */
      assert ((sim_read_cnf->cause EQ SIM_NO_ERROR) AND
              (sim_read_cnf->length >= 2));
      
      mm_data->reg.upd_sim_acc = SAT_PEND_ACT; /* RR_ACTIVATE_REQ later */
      mm_data->reg.acc_class = sim_read_cnf->trans_data[0] * 256 +
                               sim_read_cnf->trans_data[1];
      break;

    case SIM_PLMNSEL: /* Preferred PLMN list, no further action */
      mm_data->reg.upd_sim_plmnsel = SAT_UNCHANGED;

      if (sim_read_cnf->cause EQ SIM_NO_ERROR)
      {
        reg_read_pref_plmn(sim_read_cnf->trans_data, sim_read_cnf->length); 

#ifdef REL99
        /* Set flage user/operator plmn selector with access tech. used to FALSE */
        mm_data->reg.sim_uocps_at_used = FALSE;
#endif
        
      }
      else
      {

    TRACE_EVENT_P2 ("SIM read cnf error file:%x, cause:%x",
                         SIM_PLMNSEL, sim_read_cnf->cause);

      }
      if (mm_data->reg.sim_sync_req_pending EQ TRUE)
      {
        /* Allocate and send SIM_SYNC_REQ */
        reg_send_sim_sync_req();
      }

      break;

#ifdef REL99
    case SIM_UCPS_ACTEC:
      /*user controlled PLMN selector with access technology list, No Further action???*/
      mm_data->reg.upd_sim_ucps_at = SAT_UNCHANGED;

      if (sim_read_cnf->cause EQ SIM_NO_ERROR)
      {
        /*
         * EF PLMNsel will not be used. PLMNsel only be used if EFs EFPLMNwAcT or EFOPLMNwAcT 
         * are not used. MM should not read EF SIM_PLMNSEL.
         */
        mm_data->reg.upd_sim_plmnsel = SAT_UNCHANGED;

      if ( (mm_data->reg.sim_sync_req_pending == TRUE) &&
             (mm_data->reg.upd_sim_ocps_at != SAT_READ_FILE) ) /*EFOPLMNwAcT not read*/
        {
          /*EFOPLMNwAcT already read. Allocate and send SIM_SYNC_REQ*/
          reg_send_sim_sync_req();
        }

        mm_data->reg.sim_uocps_at_used = TRUE;
        reg_read_ucps_acctec(sim_read_cnf->trans_data, sim_read_cnf->length);
      }
      else
      {
        TRACE_EVENT_P2 ("SIM read cnf error file:%x, cause:%x",
                         SIM_UCPS_ACTEC, sim_read_cnf->cause);

        mm_data->reg.sim_ucps_at_len = 0;
        if (mm_data->reg.upd_sim_ocps_at EQ SAT_UNCHANGED)
        {
          /*
           * EF PLMNsel can be used if indicated because EF EFPLMNwAcT reading
	   * error and EFOPLMNwAcT is not indicated.
           */
          mm_data->reg.sim_uocps_at_used = FALSE;
        }
      }
      break;
    case SIM_OCPS_ACTEC:
      /*
       * Read Operator controlled PLMN selector with access technology list.
       * This can happens only after SIM insert indication indicates to read
       * file from SIM.
       */
      mm_data->reg.upd_sim_ocps_at  = SAT_UNCHANGED;
      if (sim_read_cnf->cause EQ SIM_NO_ERROR)
      {
        /*
         * EF PLMNsel will not be used. PLMNsel only be used if EFs EFPLMNwAcT
	 * or EFOPLMNwAcT are not used. MM should not read EF SIM_PLMNSEL.
         */
        mm_data->reg.upd_sim_plmnsel = SAT_UNCHANGED;

      if ( mm_data->reg.sim_sync_req_pending == TRUE )
        {
          /*EFPLMNwAcT already read. Allocate and send SIM_SYNC_REQ*/
          reg_send_sim_sync_req();
        }

        mm_data->reg.sim_uocps_at_used = TRUE;
        reg_read_ocps_acctec(sim_read_cnf->trans_data, sim_read_cnf->length);
      }
      else
      {
       /*
        * EF PLMNsel can be used if indicated and if there was any error
	* during reading of EF EFPLMNwAcT.
        */
       TRACE_EVENT_P2 ("SIM read cnf error file:%x, cause:%x",
                         SIM_OCPS_ACTEC, sim_read_cnf->cause);
      }
      break;
#endif

    case SIM_HPLMN: /* Home PLMN search period */
      assert ((sim_read_cnf->cause EQ SIM_NO_ERROR) AND
              (sim_read_cnf->length >= 1));

      mm_data->reg.upd_sim_hplmn = SAT_PEND_ACT; /* RR_SYNC_REQ later */
      mm_data->reg.thplmn = sim_read_cnf->trans_data[0];
      if (mm_data->reg.thplmn > HPLMN_MAX_SEARCH_PERIOD)
      {
        /* 3GPP 22.011 subclause 3.2.2.5 requires to use the default value
         * if the value delivered by the SIM exceeds the allowed limit. */
        mm_data->reg.thplmn = HPLMN_DEF_SEARCH_PERIOD;
      }
      break;

    case SIM_FPLMN: /* Forbidden PLMN list, no further action */
      {
        T_forb_plmn forb_plmn; /* Maybe avoidable by unclean cast */
        
        assert(sim_read_cnf->cause EQ SIM_NO_ERROR);
      
        /* Store new forbidden PLMN list, does this happen? */
        mm_data->reg.upd_sim_fplmn = SAT_UNCHANGED;
        forb_plmn.c_forb = sim_read_cnf->length;
        memcpy (forb_plmn.forb,
                sim_read_cnf->trans_data,
                sizeof (forb_plmn.forb));
        reg_read_forb_plmn (&forb_plmn);
      }
      break;

    case SIM_CING_AHPLMN:  /* AHPLMN Value read, RR_SYNC_REQ later */
      {
        U8 ahplmn[3];

        assert(sim_read_cnf->cause EQ SIM_NO_ERROR);
        
        mm_data->reg.upd_sim_act_hplmn = SAT_PEND_ACT;
        mm_data->reg.reg_plmn_equal_ahplmn = FALSE; 

        /* If the current RPLMN is AHPLMN then set the flag to true */
        if ( mm_data->reg.acting_hplmn.v_plmn AND
             reg_plmn_equal_rplmn(&mm_data->reg.acting_hplmn))
        {
          mm_data->reg.reg_plmn_equal_ahplmn = TRUE;
        }

        memcpy(ahplmn,
               sim_read_cnf->trans_data,
               sizeof(ahplmn));         
        reg_read_acting_hplmn(ahplmn);
      }
      break;

    case SIM_IMSI: /* IMSI */
    case SIM_LOCI: /* Location information, not expected without IMSI */
      break; /* This will not happen this way */

    case SIM_KC:
    case SIM_BCCH: /* Guaranteed not to be changed by SAT */
      break;
    
    default:
      break;
  } /* switch */

  mm_data->sim_read_req_data_field = NOT_PRESENT_16BIT;

  PFREE (sim_read_cnf);

  /* Read next changed elementary file */
  if (reg_read_next_sim_file() EQ FALSE)
  {
    /* All changed SIM files read */
    /* Acknowledge completion of file update */
    /* SIM_FILE_UPDATE_RES should be sent immediately after response 
      from SIM : Issue 23561*/
    if(mm_data->reg.sim_file_upd_ind_rec)
    {
      PALLOC (update_res, SIM_FILE_UPDATE_RES); /* T_SIM_FILE_UPDATE_RES */
      update_res->source = SRC_MM;
      update_res->fu_rsc = SIM_FU_SUCCESS;
      PSENDX (SIM, update_res);
      mm_data->reg.sim_file_upd_ind_rec = FALSE;
    }
    /*
     * Set sim mm read in progress to False since MM has completed reading of
     * EFs(PLMNsel/EF EFPLMNwAcT/EFOPLMNwAcT) indicated in primitive SIM_MM_INSERT_IND
     * /SIM_MM_INFO_IND/SIM_FILE_UPDATE_IND. Also check if there was any rr_abort_ind 
     * was received and stored becuaseof SIM reading was in progress.
     */
    if(mm_data->reg.sim_read_in_progress)
    {
      mm_data->reg.sim_read_in_progress = FALSE;
      USE_STORED_ENTRIES();
    }
    if (mm_data->reg.upd_sim_acc NEQ SAT_UNCHANGED)
    {
      /* 
       * MM has to send a RR_SYNC_REQ
       * The actual changes of these fields are not tested by GSM 11.11
       */
      mm_data->reg.upd_sim_acc = SAT_UNCHANGED;

      /* Inform RR about changes by SAT */
       mm_build_rr_sync_req_cause (SYNCCS_ACCC);
    }

    if (mm_data->reg.upd_sim_hplmn NEQ SAT_UNCHANGED)
    {
      if (mm_data->first_attach_mem AND
          mm_data->reg.thplmn NEQ 0)
      {
        /* do nothing : MM is in initial search period. So, the timer 
          should not be restarted. The new value is stored and after initial 
          search the timer should be restarted with this new value*/
      }
      else
      {
        /* Start HPLMN search timer with new value */
        reg_stop_hplmn_tim ();
        reg_check_hplmn_tim(mm_data->reg.thplmn);
      }
    }/*end of mm_data->reg.upd_sim_hplmn*/

    if (mm_data->reg.upd_sim_act_hplmn NEQ SAT_UNCHANGED )
    {         
      BOOL valid_ahplmn= FALSE;
      valid_ahplmn = valid_acting_hplmn(&mm_data->reg.acting_hplmn);
      mm_build_rr_sync_hplmn_req();
      if (valid_ahplmn)
      {
        /* Delete the AHPLMN entry from the Forbidden list, if present*/
        if ( mm_data->reg.acting_hplmn.v_plmn)
        {
          /* Remove from forbidden PLMN list if stored */
          TRACE_FUNCTION(" AHPLMN is being deleted from the Forbidden List");
          reg_plmn_bad_del (mm_data->reg.forb_plmn, 
                          MAX_FORB_PLMN_ID,
                          &mm_data->reg.acting_hplmn);
        }

        /*Inform ACI of the new AHPLMN value*/
        mm_mmgmm_ahplmn_ind(&mm_data->reg.acting_hplmn);

        /* Delete any Equivalent PLMN list */
        reg_clear_plmn_list (mm_data->reg.eqv_plmns.eqv_plmn_list,
				EPLMNLIST_SIZE);

        /* If AHPLMN value read from SIM is same as the RPLMN then inform
         * RR about it. No need of registering again. Set the flag 
         * to true so that hplmn will now be taken from this
         */
        if (reg_plmn_equal_rplmn(&mm_data->reg.acting_hplmn))  
        {
          reg_stop_hplmn_tim ();
        }
        else
        {
          mm_data->plmn_scan_mm = TRUE;
          /* Start with AHPLMN registration */
          mm_func_mmgmm_net_req();
        }
      }
      else
      {
        T_plmn   hplmn;

        /*Inform ACI of the new AHPLMN value*/
        mm_mmgmm_ahplmn_ind(&mm_data->reg.acting_hplmn);

        /* Since mm_data->reg.acting_hplmn.v_plmn is FALSE it will take HPLMN from IMSI */
        reg_extract_hplmn(&hplmn);

        if (reg_plmn_equal_rplmn(&hplmn))
        {
          /* Do Nothing. MS is already registered to True-HPLMN */
        }
        else
        {
          /* If AHPLMN is FFFFFF OR we are on already on an AHPLMN MM should
           * immediately start a network search for HPLMN
           */
          if (mm_data->reg.acting_hplmn_invalid OR
              mm_data->reg.reg_plmn_equal_ahplmn)
          {
            mm_data->plmn_scan_mm = TRUE;
            mm_func_mmgmm_net_req();
          }
          else
          {
            /*Do Nothing. Since MS is registered to any other PLMN HPPLMN
              Timer will be running. Network scan will start after the
              Timer expires
            */
          }
        }/*reg_plmn_equal_rplmn*/
      }/*valid_acting_hplmn*/
      check_if_cingular_sim(); /*for cingular only 31179*/       
    }/*end of mm_data->reg.upd_sim_act_hplmn*/
  }/*end of reg_read_next_sim_file*/
}/*end of reg_sim_read_cnf*/

#endif

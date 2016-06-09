/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_GMM.C
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
|  Purpose :  This module defines the functions for the communication
|             of MM with GMM. All communications from MM to GMM will 
|             be through this file.
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

/*==== EXPORT =====================================================*/

/*==== TEST =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

#if defined(GPRS)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_mmgmm_reg_cnf     |
+--------------------------------------------------------------------+

  PURPOSE : sends the MMGMM_REG_CNF primitive

*/

GLOBAL void mm_send_mmgmm_reg_cnf (UBYTE bootup_cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_send_mmgmm_reg_cnf()");

  {
    PALLOC (mmgmm_reg_cnf, MMGMM_REG_CNF); /* T_MMGMM_REG_CNF */
    if (bootup_cause NEQ PWR_SCAN_START)
    {
      mmgmm_reg_cnf->plmn.v_plmn = V_PLMN_PRES;
      memcpy (mmgmm_reg_cnf->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
      memcpy (mmgmm_reg_cnf->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
    }
    else
    {
      mmgmm_reg_cnf->plmn.v_plmn = V_PLMN_NOT_PRES;
    }
    mmgmm_reg_cnf->lac = mm_data->mm.lai.lac;
    mmgmm_reg_cnf->cid = mm_data->mm.cid;
    mmgmm_reg_cnf->resumption = mm_data->gprs.resumption;
    mmgmm_reg_cnf->gprs_indicator = mm_data->mm.gprs_indication;  
    mmgmm_reg_cnf->bootup_cause = bootup_cause;
    PSENDX (GMM, mmgmm_reg_cnf);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_reg_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : This function indicates change in service or change in PLMN 
            to the GMREG SAP. This is suppressed if the 
            registration type is REG_CELL_SEARCH_ONLY, in this
            case GMM is not interested in the MM service information.

*/

GLOBAL void mm_mmgmm_reg_cnf (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_reg_cnf()");

  if (GET_STATE (STATE_REG_TYPE) NEQ REG_CELL_SEARCH_ONLY)
  {
    /* Issue 20792 : Added condition mm_data->plmn_scan_mmi. If network search requested by user
     is pending then after RR release MM should initiate network search and after getting
     proper PLMN list it should inform GMM followed by successful Registration. Else there will be
     collision in RR between network search and GPRS Attach resulting in abortion of network
     search and hence empty list by RR will be returned.
    */


    if ( mm_data->plmn_scan_mmi ) 
    {
      mm_data->gprs.reg_cnf_on_idle_entry = TRUE;
    }
    else
    {
      if ((mm_data->reg.full_service_indicated EQ FALSE) OR
          (mm_data->reg.new_cell_ind EQ TRUE))
      {
        /* 
         * Either no full service was indicated to the MMI, 
         * or the PLMN has changed from that what was indicated before.
         */
        mm_send_mmgmm_reg_cnf (REG_END);

        mm_data->reg.full_service_indicated = TRUE;
        mm_data->reg.new_cell_ind = FALSE;	  
      }

      mm_data->gprs.reg_cnf_on_idle_entry = FALSE;

      /* Disallow establishment for own location updating and CM services */
      if (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED)
      {
        SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);
        SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
      }
    } /* else */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_mmgmm_nreg_ind     |
+--------------------------------------------------------------------+

  PURPOSE : This procedure builds ans sends the MMGMM_NREG_IND
            primitive without having any side effects.

*/

LOCAL void mm_send_mmgmm_nreg_ind (UBYTE service, 
                                   UBYTE search_running,
                                   UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  PALLOC (mmgmm_nreg_ind, MMGMM_NREG_IND); /* T_MMGMM_NREG_IND */

  TRACE_FUNCTION ("mm_send_mmgmm_nreg_ind()");

  mmgmm_nreg_ind->service        = service; 
  mmgmm_nreg_ind->search_running = search_running;

  if (forb_ind EQ FORB_PLMN_INCLUDED)
  {
    mmgmm_nreg_ind->new_forb_plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmgmm_nreg_ind->new_forb_plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmgmm_nreg_ind->new_forb_plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
  }
  else
  {
    mmgmm_nreg_ind->new_forb_plmn.v_plmn = V_PLMN_NOT_PRES;
    memset (mmgmm_nreg_ind->new_forb_plmn.mcc, 0x0f, SIZE_MCC);
    memset (mmgmm_nreg_ind->new_forb_plmn.mnc, 0x0f, SIZE_MNC);
  }

  mmgmm_nreg_ind->cause = mm_data->limited_cause;
  /* No GPRS resumption field in MMGMM_NREG_IND as in MMGMM_REG_REJ */
  PSENDX (GMM, mmgmm_nreg_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_send_mmgmm_reg_rej      |
+--------------------------------------------------------------------+

  PURPOSE : This procedure builds ans sends the MMGMM_REG_REJ
            primitive without having any side effects.

*/

LOCAL void mm_send_mmgmm_reg_rej (UBYTE service,
                                  UBYTE search_running,
                                  UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  PALLOC (mmgmm_reg_rej, MMGMM_REG_REJ); /* T_MMGMM_REG_REJ */

  TRACE_FUNCTION ("mm_send_mmgmm_reg_rej()");

  mmgmm_reg_rej->service        = service; 
  mmgmm_reg_rej->search_running = search_running;

  if (forb_ind EQ FORB_PLMN_INCLUDED)
  {
    mmgmm_reg_rej->new_forb_plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmgmm_reg_rej->new_forb_plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmgmm_reg_rej->new_forb_plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
  }
  else
  {
    mmgmm_reg_rej->new_forb_plmn.v_plmn = V_PLMN_NOT_PRES;
    memset (mmgmm_reg_rej->new_forb_plmn.mcc, 0x0f, SIZE_MCC);
    memset (mmgmm_reg_rej->new_forb_plmn.mnc, 0x0f, SIZE_MNC);
  }

  mmgmm_reg_rej->cause      = mm_data->limited_cause;
  mmgmm_reg_rej->resumption = mm_data->gprs.resumption;
  PSENDX (GMM, mmgmm_reg_rej);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_nreg_ind          |
+--------------------------------------------------------------------+

  PURPOSE : This is the common procedure for MMGMM_NREG_IND and
            MMGMM_REG_REJ. MMGMM_REG_REJ is sent if the procedure 
            war a remote controlled location updating procedure, 
            otherwise MMGMM_NREG_IND is sent.

*/

GLOBAL void mm_mmgmm_nreg_ind (UBYTE service,
                               UBYTE search_running,
                               UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_nreg_ind()");

  if (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED)
  {
    mm_send_mmgmm_reg_rej  (service, search_running, forb_ind);
  }
  else
  {
    mm_send_mmgmm_nreg_ind (service, search_running, forb_ind);
  }

#ifdef WIN32
  TRACE_EVENT_P1 ("  service=%d",        service);
  TRACE_EVENT_P1 ("  search_running=%d", search_running);
  TRACE_EVENT_P1 ("  limited_cause=%d",  mm_data->limited_cause);
  TRACE_EVENT_P1 ("  resumption=%d",     mm_data->gprs.resumption);
#endif /* #ifdef WIN32 */

  /* 
   * Delete the limited cause if it was not fatal, as 
   * on the next cell selection the reason may be another than 
   * the one now indicated.
   */
  if (mm_data->reg.op.sim_ins EQ SIM_INSRT)
    mm_data->limited_cause = MMCS_INT_NOT_PRESENT;

  mm_data->reg.full_service_indicated = FALSE;
  mm_data->gprs.reg_cnf_on_idle_entry = FALSE;

  /* Disallow establishment for own location updating and CM services */
  if (GET_STATE (STATE_REG_TYPE) EQ REG_REMOTE_CONTROLLED)
  {
    SET_STATE (STATE_REG_TYPE, REG_CELL_SEARCH_ONLY);
    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_nreg_cnf          |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_NREG_CNF. 
            Otherwise send MMR_NREG_CNF.

*/

GLOBAL void mm_mmgmm_nreg_cnf (UBYTE detach_cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_nreg_cnf()");

  if (mm_data->nreg_request)
  {
    PALLOC (mmgmm_nreg_cnf, MMGMM_NREG_CNF); /* T_MMGMM_NREG_CNF */
    mmgmm_nreg_cnf->detach_cause = detach_cause;
    
#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  detach_cause=%d", mmgmm_nreg_cnf->detach_cause);
#endif /* WIN32 */
  
    PSENDX (GMM, mmgmm_nreg_cnf);

    mm_data->nreg_request = FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_plmn_ind          |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_PLMN_IND. 
            Otherwise send MMR_PLMN_IND.

*/

GLOBAL void mm_mmgmm_plmn_ind (USHORT cause,
                         const T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_plmn_ind()");

  if (mm_data->plmn_scan_mmi)
  {
    USHORT i;
    PALLOC (mmgmm_plmn_ind, MMGMM_PLMN_IND); /* T_MMGMM_PLMN_IND */

    /* Issue 20792  : If there was any network search request pending during specific
         prorcedure then on getting RR_RELEASE_IND that would be processed first and 
         MMGMM_REG_CNF will be held till that is done. Once PLMN list is receieved by RR
         MM should send MMGMM_PLMN_IND to GMM followed by MMGMM_REG_CNF .
    */

    if ( mm_data->gprs.reg_cnf_on_idle_entry )
    {
      mm_data->plmn_scan_mmi = FALSE;
      mm_data->reg.full_service_indicated = FALSE;/* Force MMGMM_REG_CNF */
      mm_mmgmm_reg_cnf();
    }

    /* Net Search Result */
    mmgmm_plmn_ind->cause = cause;
    
    // See comment for MMR_PLMN_IND!!!
    memset (mmgmm_plmn_ind->plmn, NOT_PRESENT_8BIT,
            sizeof (mmgmm_plmn_ind->plmn));

    /* Make testcases happy, initialize all unused data */
    memset (mmgmm_plmn_ind->forb_ind, FORB_PLMN_NOT_INCLUDED, 
            sizeof (mmgmm_plmn_ind->forb_ind));    
    memset (mmgmm_plmn_ind->rxlevel, 0x00, 
            sizeof (mmgmm_plmn_ind->rxlevel));    
    memset (mmgmm_plmn_ind->gprs_status, MMGMM_GPRS_GSM, 
            sizeof (mmgmm_plmn_ind->gprs_status));    

    if (cause EQ MMCS_SUCCESS)
    {  
      /* Create the PLMN list for the MMI and sent it */
      reg_create_plmn_list (rr_abort_ind, WITH_ALL_PLMNS);

      for (i = 0; i < mm_data->reg.plmn_cnt; i++) 
      {
        /* Copy the PLMN information itself from registration data 
         * This may be too complicated, one memcpy() may be sufficient */

        reg_unpack_plmn (&mmgmm_plmn_ind->plmn[i], mm_data->reg.plmn, i);

        /* Process forbidden PLMN information */
        if (reg_plmn_in_list (mm_data->reg.forb_plmn,
                              MAX_FORB_PLMN_ID,
                              &mmgmm_plmn_ind->plmn[i]))
        {
          mmgmm_plmn_ind->forb_ind[i] = FORB_PLMN_INCLUDED;
        }
        else
        {
          /* 
           * PLMN not member of forbidden list (cause #11), if GPRS is 
           * active, consider GPRS forbidden list also (cause #14)
           */ 
          if (!mm_gsm_alone() AND 
              (mm_data->gprs.mobile_class EQ MMGMM_CLASS_CG) AND 
              reg_plmn_in_list (mm_data->reg.gprs_forb_plmn,
                                MAX_GPRS_FORB_PLMN_ID,
                                &mmgmm_plmn_ind->plmn[i]))
          {
            mmgmm_plmn_ind->forb_ind[i] = FORB_PLMN_INCLUDED;
          }
          else
          {
            mmgmm_plmn_ind->forb_ind[i] = FORB_PLMN_NOT_INCLUDED;
          }
        }

        /* Copy the rx level from registration data */
        mmgmm_plmn_ind->rxlevel[i] = mm_data->reg.plmn_rx [i];

        mmgmm_plmn_ind->lac_list[i] = mm_data->reg.plmn_lac [i]; //LOL 02.01.2003: added for EONS support

        /* Copy the GPRS capabilities from registration data */
        mmgmm_plmn_ind->gprs_status[i] = mm_data->reg.gprs_status[i];
      } /* for */

      /* Do not consider the forbidden PLMNs for MM's internal operation */
      reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);
    } /* if */

    PSENDX (GMM, mmgmm_plmn_ind);

  }

  reg_check_plmn_search (cause, rr_abort_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_auth_rej_ind      |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_AUTH_REJ_IND. 
            Otherwise this is compiled to a dummy function and will 
            return without further action.
*/

GLOBAL void mm_mmgmm_auth_rej_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_auth_rej_ind()");
  
  {
    PALLOC (mmgmm_auth_rej_ind, MMGMM_AUTH_REJ_IND);
    PSENDX (GMM, mmgmm_auth_rej_ind);
  }

  mm_data->reg.full_service_indicated = FALSE;
  mm_data->gprs.reg_cnf_on_idle_entry = FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_cm_establish_ind  |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_CM_ESTABLISH_IND if 
            not already done. A GSM only protocol stack doesn't 
            need this function.
            By the primitive MMGMM_CM_ESTABLISH_IND GMM is informed 
            that a circuit switched call has to be started.

*/

GLOBAL void mm_mmgmm_cm_establish_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_establish_ind()");

  switch (GET_STATE (STATE_GPRS_CM_EST))
  {
    case CM_GPRS_EST_IDLE:
      {
        PALLOC (mmgmm_establish_ind, MMGMM_CM_ESTABLISH_IND);
        PSENDX (GMM, mmgmm_establish_ind);
      }
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_PEND);
      break;

    default: /* Maybe CM establishment is already allowed etc. */
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_cm_emergency_ind  |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_CM_EMERGENCY_IND. 
            Otherwise this is compiled to a dummy function and will 
            return without further action.
            By this primitive GMM is informed that a circuit switched 
            emergency call has to be started.

*/

GLOBAL void mm_mmgmm_cm_emergency_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_emergency_ind()");

  switch (GET_STATE (STATE_GPRS_CM_EST))
  {
    case CM_GPRS_EST_IDLE:
    case CM_GPRS_EST_PEND:
      {
        PALLOC (mmgmm_emergency_ind, MMGMM_CM_EMERGENCY_IND);
        PSENDX (GMM, mmgmm_emergency_ind);
      }
      SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EMERGE_PEND);
      break;

    default: /* MMGMM_CM_EMERGENCY_IND has already been sent */
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_cm_release_ind    |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_CM_RELEASE_IND if the 
            conditions are met.
            Otherwise this is compiled to a dummy function and will 
            return without further action.
*/

GLOBAL void mm_mmgmm_cm_release_ind (UBYTE resumption)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_cm_release_ind()");

  if (!mm_gsm_alone() AND 
      (GET_STATE (STATE_GPRS_CM_EST) NEQ CM_GPRS_EST_IDLE) AND
      (mm_count_connections (CM_NOT_IDLE) EQ 0))
  {
    PALLOC (mmgmm_cm_release_ind, MMGMM_CM_RELEASE_IND);
    mmgmm_cm_release_ind->resumption = resumption;

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  resumption=%d", mmgmm_cm_release_ind->resumption);
#endif /* WIN32 */

    PSENDX (GMM, mmgmm_cm_release_ind);

    SET_STATE (STATE_GPRS_CM_EST, CM_GPRS_EST_IDLE);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_activate_ind      |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_ACTIVATE_IND. 
            Otherwise this is compiled to a dummy function and will 
            return without further action.
            The t3212_val here is to be given in the same form as it 
            is to be received from the network, this means in steps of
            6 minutes, it will be scaled to milliseconds for GMM here.
*/

GLOBAL void mm_mmgmm_activate_ind (UBYTE status)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_activate_ind()");

  if (!mm_gsm_alone())
  {
    PALLOC (mmgmm_activate_ind, MMGMM_ACTIVATE_IND); /* T_MMGMM_ACTIVATE_IND */
    mmgmm_activate_ind->plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmgmm_activate_ind->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmgmm_activate_ind->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
    mmgmm_activate_ind->lac = mm_data->mm.lai.lac;
    mmgmm_activate_ind->cid = mm_data->mm.cid;
    if (mm_data->t3212_cfg_counter NEQ 0 AND 
        mm_data->mm.mm_info.t3212 NEQ 0)
      mmgmm_activate_ind->t3212_val = mm_data->t3212_cfg_counter * 10000;
    else
      mmgmm_activate_ind->t3212_val = mm_data->mm.mm_info.t3212 * 360000;
    mmgmm_activate_ind->status = status;
    mmgmm_activate_ind->gprs_indicator = mm_data->mm.gprs_indication;

    if (status NEQ MMGMM_LIMITED_SERVICE)
    {
      mm_data->reg.actual_plmn = mmgmm_activate_ind->plmn; /* Struct copy */
    }

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  MCC=%x%x%x MNC=%x%x%x LAC=%04x",
                  mmgmm_activate_ind->plmn.mcc[0],
                  mmgmm_activate_ind->plmn.mcc[1],
                  mmgmm_activate_ind->plmn.mcc[2],
                  mmgmm_activate_ind->plmn.mnc[0],
                  mmgmm_activate_ind->plmn.mnc[1],
                  mmgmm_activate_ind->plmn.mnc[2],
                  mmgmm_activate_ind->lac);
     
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  t3212_val=%d", 
                  mmgmm_activate_ind->t3212_val);

    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  status=%d", 
                  mmgmm_activate_ind->status);

    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  gprs_indicator=%d", 
                  mmgmm_activate_ind->gprs_indicator);
#endif /* #ifdef WIN32 */

    PSENDX (GMM, mmgmm_activate_ind);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_t3212_val_ind     |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_T3212_VAL_IND always T3212 
            changes, but no cell selection by RR was performed.
            For a GSM only protocol stack, this function doesn't exist.
            The time to be sent to GMM is scaled in units of milliseconds.

*/

GLOBAL void mm_mmgmm_t3212_val_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_t3212_val_ind()");

  {
    /* T_MMGMM_T3212_VAL_IND */
    PALLOC (mmgmm_t3212_val_ind, MMGMM_T3212_VAL_IND);

    if (mm_data->t3212_cfg_counter NEQ 0 AND 
        mm_data->mm.mm_info.t3212 NEQ 0)
    {
      mmgmm_t3212_val_ind->t3212_val = mm_data->t3212_cfg_counter * 10000;
    }
    else
    {
      mmgmm_t3212_val_ind->t3212_val = mm_data->mm.mm_info.t3212 * 360000;
    }

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  t3212_val=%d", mmgmm_t3212_val_ind->t3212_val);
#endif /* WIN32 */

    PSENDX (GMM, mmgmm_t3212_val_ind);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_lup_accept_ind    |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_LUP_ACCEPT_IND.
            For a GSM only protocol stack, this function doesn't exist.
            It is assumed that the networks sends a location updating 
            accept message for the location area of the current serving
            cell here, this assumption should be true always.

*/

GLOBAL void mm_mmgmm_lup_accept_ind (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_lup_accept_ind()");

  if ((mm_data->reg.full_service_indicated EQ FALSE) OR
      (mm_data->reg.new_cell_ind EQ TRUE))
  {
    /* T_MMGMM_LUP_ACCEPT_IND */
    PALLOC (mmgmm_lup_accept_ind, MMGMM_LUP_ACCEPT_IND);
    mmgmm_lup_accept_ind->plmn.v_plmn = V_PLMN_PRES;
    memcpy (mmgmm_lup_accept_ind->plmn.mcc, mm_data->mm.lai.mcc, SIZE_MCC);
    memcpy (mmgmm_lup_accept_ind->plmn.mnc, mm_data->mm.lai.mnc, SIZE_MNC);
    mmgmm_lup_accept_ind->lac = mm_data->mm.lai.lac;
    mmgmm_lup_accept_ind->cid = mm_data->mm.cid;

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  MCC=%x%x%x MNC=%x%x%x LAC=%04x",
                  mmgmm_lup_accept_ind->plmn.mcc[0],
                  mmgmm_lup_accept_ind->plmn.mcc[1],
                  mmgmm_lup_accept_ind->plmn.mcc[2],
                  mmgmm_lup_accept_ind->plmn.mnc[0],
                  mmgmm_lup_accept_ind->plmn.mnc[1],
                  mmgmm_lup_accept_ind->plmn.mnc[2],
                  mmgmm_lup_accept_ind->lac);
#endif /* #ifdef WIN32 */

    PSENDX (GMM, mmgmm_lup_accept_ind);

    mm_data->reg.full_service_indicated = TRUE;
    mm_data->reg.new_cell_ind = FALSE;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_lup_needed_ind    |
+--------------------------------------------------------------------+

  PURPOSE : If GPRS is enabled, send MMGMM_LUP_NEEDED_IND.
            For a GSM only protocol stack, this function doesn't exist.
            This function is used if GMM has to be informed about the
            fact that a location updating procedure is needed by MM, 
            eg. a CM SERVICE REJECT message with the cause "IMSI unknown
            in VLR" has been received or simply T3212 has expired. 
            A cell selection was not the cause for the need to update.
            If the location updating is neccessary due to an incoming 
            establish request by the CM entities in state 
            MM_IDLE_ATTEMPT_TO_UPDATE, this is indicated by 
            MMGMM_CM_ESTABLISH_IND and should cause a registration by GMM
            as GMM should know that MM needs an update in this situation.
 
*/

GLOBAL void mm_mmgmm_lup_needed_ind (UBYTE reason)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_lup_needed_ind()");

  if (!mm_lup_allowed_by_gmm ())
  {
    /* T_MMGMM_LUP_NEEDED_IND */
    PALLOC (mmgmm_lup_needed_ind, MMGMM_LUP_NEEDED_IND);
    mmgmm_lup_needed_ind->reason = reason;

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  reason=%d", mmgmm_lup_needed_ind->reason);
#endif /* WIN32 */

    PSENDX (GMM, mmgmm_lup_needed_ind);
  }
  else
  {
    /* This is an internal MM error, harmless, but should be fixed if seen */
    TRACE_ERROR ("MMGMM_LUP_NEEDED_IND suppressed");
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_tmsi_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Send MMGMM_TMSI_IND with the new TMSI
            whenever the TMSI changes after SIM insertion.

*/

GLOBAL void mm_mmgmm_tmsi_ind (ULONG tmsi)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("mm_mmgmm_tmsi_ind()");

  if (tmsi NEQ mm_data->reg.indicated_tmsi)
  {
    PALLOC (mmgmm_tmsi_ind, MMGMM_TMSI_IND);  /* T_MMGMM_TMSI_IND */
    mmgmm_tmsi_ind->tmsi = tmsi;
    PSENDX (GMM, mmgmm_tmsi_ind);

    mm_data->reg.indicated_tmsi = tmsi;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_MM                      |
| STATE   : code                ROUTINE : mm_mmgmm_ahplmn_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Send MMGMM_AHPLMN_IND with the new AHPLMN
            at poweron if valid AHPLMN and whenever the
            AHPLMN changes.            
*/

GLOBAL void mm_mmgmm_ahplmn_ind (T_plmn  *acting_hplmn)
{

  TRACE_FUNCTION ("mm_mmgmm_ahplmn_ind()");
  {
    PALLOC(mmgmm_ahplmn_ind, MMGMM_AHPLMN_IND);  /*T_MMGMM_AHPLMN_IND*/

    mmgmm_ahplmn_ind->ahplmn.v_plmn = acting_hplmn->v_plmn;
    memcpy(mmgmm_ahplmn_ind->ahplmn.mcc, acting_hplmn->mcc, SIZE_MCC);
    memcpy(mmgmm_ahplmn_ind->ahplmn.mnc, acting_hplmn->mnc, SIZE_MNC);

     

#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "  MCC=%x%x%x MNC=%x%x%x v_plmn=%x",
                  mmgmm_ahplmn_ind->ahplmn.mcc[0],
                  mmgmm_ahplmn_ind->ahplmn.mcc[1],
                  mmgmm_ahplmn_ind->ahplmn.mcc[2],
                  mmgmm_ahplmn_ind->ahplmn.mnc[0],
                  mmgmm_ahplmn_ind->ahplmn.mnc[1],
                  mmgmm_ahplmn_ind->ahplmn.mnc[2],
                  mmgmm_ahplmn_ind->ahplmn.v_plmn);
                  
#endif /* #ifdef WIN32 */

    PSENDX(GMM, mmgmm_ahplmn_ind);
  }   

}

#endif /* defined(GPRS) */
#endif /* MM_GMM_C */

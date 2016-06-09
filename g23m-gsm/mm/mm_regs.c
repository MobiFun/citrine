/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  MM_REGS
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
|             capability of the module Mobility Managemant.
+----------------------------------------------------------------------------- 
*/ 

#ifndef MM_REGS_C
#define MM_REGS_C

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

#endif
/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*==== TEST =====================================================*/

/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mmr_auth_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal MMR_AUTH_IND.

*/

GLOBAL void reg_mmr_auth_ind (T_SIM_AUTHENTICATION_REQ *sim_auth_req)
{
  GET_INSTANCE_DATA;
  MCAST (auth_req, D_AUTH_REQ);
  TRACE_FUNCTION ("reg_mmr_auth_ind()");

  if(mm_data->last_auth_req_id != NOT_PRESENT_8BIT)
  {
    mm_data->last_auth_req_id++;
  }
  else
  {
    mm_data->last_auth_req_id = 0;
  }

  sim_auth_req->source = SRC_MM;
  sim_auth_req->req_id = mm_data->last_auth_req_id;
  sim_auth_req->cksn   = auth_req->ciph_key_num.key_seq;
  memcpy (sim_auth_req->rand, auth_req->auth_rand.rand,
          sizeof (sim_auth_req->rand));
  PSENDX (SIM, sim_auth_req);
  if (mm_data->last_auth_req_id != 0)
  {
     /* problem occurred!!!! */
#ifdef WIN32
    vsi_o_ttrace (VSI_CALLER TC_FUNC, 
                  "Authentication problem occurred %d", mm_data->last_auth_req_id);
#endif /* WIN32 */
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_net_list               |
+--------------------------------------------------------------------+

  PURPOSE : Build a list of available PLMNs for the MMI. 
            Use the information received from RR to update MM's 
            internal PLMN list used for MM's own operation.

*/

#if 0 // The old code
GLOBAL void reg_net_list (const T_RR_ABORT_IND *rr_abort_ind)
{
  TRACE_FUNCTION ("reg_net_list()");

  /* Create the PLMN list for the MMI and send it */
  reg_create_plmn_list (rr_abort_ind, WITH_ALL_PLMNS);

  if (mm_data->reg.plmn_cnt EQ 0)
  {
    if ((rr_abort_ind->op.service NEQ NO_SERVICE) AND
        (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL))
    {
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE);
    }
    else
    {
      mm_mmgmm_plmn_ind (MMCS_SUCCESS);
    }
  }
  else
  {
    mm_mmgmm_plmn_ind (MMCS_SUCCESS);
  }

  /* Do not consider the forbidden PLMN's for MM's internal operation */
  reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);

  switch (rr_abort_ind->op.service)
  {
    case NO_SERVICE:
      mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
      break;

    case LIMITED_SERVICE:
      mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
      break;

    case FULL_SERVICE:
      break;

    default: /* All possible values caught, this is garbage */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
}
#else // The new code
GLOBAL void reg_net_list (const T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_net_list()");

  if (rr_abort_ind->plmn_avail EQ 0)
  {
    /*
     * Distinguish between "no PLMN found" and search aborted 
     * due to eg. paging in RR. In the future, we may introduce
     * a proper cause in the RR_ABORT_IND to distinguish this, 
     * this here should work but is ugly.
     */   
    if ((rr_abort_ind->op.service NEQ NO_SERVICE) AND
        (rr_abort_ind->cause EQ RRCS_ABORT_CEL_SEL_FAIL))
    { 
      mm_mmgmm_plmn_ind (MMCS_PLMN_NOT_IDLE_MODE, NULL);
    }
    else
    {
      mm_mmgmm_plmn_ind (MMCS_SUCCESS, rr_abort_ind);
    }
    mm_data->reg.plmn_cnt = 0;
  }
  else
  {
    mm_mmgmm_plmn_ind (MMCS_SUCCESS, rr_abort_ind);
  }

  switch (rr_abort_ind->op.service)
  {
    case NO_SERVICE:
      mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
      break;

    case LIMITED_SERVICE:
      mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
      break;

    case FULL_SERVICE:
      break;

    default: /* All possible values caught, this is garbage */
      TRACE_ERROR (UNEXPECTED_DEFAULT);
      break;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_rr_failure             |
+--------------------------------------------------------------------+

  PURPOSE : Indication from MM/RR about unsuccessfull cell selection.
            The new service and the found PLMN indications are for-
            warded to MM.

*/

GLOBAL void reg_rr_failure (T_RR_ABORT_IND *rr_abort_ind)
{
  GET_INSTANCE_DATA;
  unsigned i;
  BOOL actual_plmn_found;

  TRACE_FUNCTION ("reg_rr_failure()");

#if defined (WIN32)
  {
    TRACE_EVENT_P1 ("PLMN_AVAIL = %d", rr_abort_ind->plmn_avail);

    for (i = 0 ;i < rr_abort_ind->plmn_avail; i++)
    {
      TRACE_EVENT_P6 ("MCC=%x%x%x MNC=%x%x%x",
                      rr_abort_ind->plmn[i].mcc[0],
                      rr_abort_ind->plmn[i].mcc[1],
                      rr_abort_ind->plmn[i].mcc[2],
                      rr_abort_ind->plmn[i].mnc[0],
                      rr_abort_ind->plmn[i].mnc[1],
                      rr_abort_ind->plmn[i].mnc[2]);
    }
  }
#endif /* #if defined (WIN32) */

  if (rr_abort_ind->plmn_avail EQ 0 OR 
      mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /* 
     * No PLMN delivered by RR to build a PLMN list or no SIM present
     */
    switch (rr_abort_ind->op.service)
    {
      case NO_SERVICE:
        mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
        break;

      case LIMITED_SERVICE:
        mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE,
                           SEARCH_NOT_RUNNING,
                           FORB_PLMN_NOT_INCLUDED);
        break;

      default: /* eg. FULL_SERVICE and reg_rr_failure => nonsense */
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
    
    /* Delete the PLMN list, currently no PLMN available */
    mm_data->reg.plmn_cnt = 0;
    
/*
    // First make sure to understand completely the underlying problem...
    // Patch HM 11.06.01, clear PLMN list in ACI and MM >>>
    mm_mmgmm_plmn_ind (MMCS_SUCCESS, rr_abort_ind);
    // Patch HM 11.06.01, clear PLMN list in ACI and MM <<<
*/

  }
  else
  {
    /*
     * At least one PLMN found and SIM present, create PLMN list.
     * Depending on the selected mode (automatic or manual), 
     * either select the next available PLMN or present the
     * list of PLMNs possibly providing full service to the user.
     */
    switch (mm_data->reg.op.m)
    {
      case MODE_AUTO:
        // Note: There are better algorithms to decide whether an update
        // of the PLMN list is necessary or not, but for these we need the
        // information from RR whether a found PLMN/location area is
        // forbidden. This becomes especially TRUE for an exhausted PLMN
        // list as we waste a lot of battery here if there is no location
        // area suitable for full service available, but a lot of location
        // areas on VPLMNs which are forbidden for roaming. In this case we
        // run in a loop senseless activating VPLMN which don't provide 
        // full service at all.
        /*
         * Don't update the PLMN list if the actual PLMN is a member
         * of the PLMN list delivered by RR and the current list is
         * non-exhaused.
         */
        actual_plmn_found = FALSE;

        if (mm_data->reg.plmn_cnt > mm_data->reg.plmn_index)
        {
          /* 
           * There is a non-exhausted list present in MM.
           * Check whether the actual PLMN is a member.
           */
          for (i = 0; i < rr_abort_ind->plmn_avail; i++)
          {
            if (reg_plmn_equal_sim (&rr_abort_ind->plmn[i],
                                    &mm_data->reg.actual_plmn))
            {
              actual_plmn_found = TRUE;
              break;
            }
          }
          if (!actual_plmn_found)
          {
            /* The requested PLMN was not found. Update outdated list. */
            reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);
          }
          else
          {
            TRACE_EVENT ("PLMN list ignored");
          }
        }
        else
        {
          /* No non-exhaused list present in MM. Take the new list from RR */
          reg_create_plmn_list (rr_abort_ind, WITH_OTHER_PLMNS);
        }

        /*
         * Select the next PLMN if available
         * else indicate LIMITED SERVICE to MMI
         */
        reg_plmn_select (FORB_PLMN_NOT_INCLUDED);
        break;

      case MODE_MAN:
        reg_create_plmn_list (rr_abort_ind, WITH_ALL_PLMNS);

        /*
         * Forward the PLMN list to MMI for selection
         */
        switch (rr_abort_ind->op.service)
        {
          case NO_SERVICE:
            mm_mmgmm_nreg_ind (NREG_NO_SERVICE, 
                               SEARCH_NOT_RUNNING,
                               FORB_PLMN_NOT_INCLUDED);
            break;

          case LIMITED_SERVICE:
            mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE,
                               SEARCH_NOT_RUNNING,
                               FORB_PLMN_NOT_INCLUDED);
            break;

          default: /* eg. FULL_SERVICE and reg_rr_failure => nonsense */
            TRACE_ERROR (UNEXPECTED_DEFAULT);
            break;
        }
        mm_data->plmn_scan_mmi = TRUE;
        mm_mmgmm_plmn_ind (MMCS_SUCCESS, rr_abort_ind);
        break;
      
      default:  /* No mode left */
        TRACE_ERROR (UNEXPECTED_DEFAULT);
        break;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mm_success             |
+--------------------------------------------------------------------+

  PURPOSE : MM indicates successfull registration.

*/

GLOBAL void reg_mm_success (UBYTE service)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_mm_success()");

  if (mm_data->reg.op.sim_ins EQ SIM_NO_INSRT)
  {
    /*
     * No SIM is inserted
     */
    mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                       SEARCH_NOT_RUNNING,
                       FORB_PLMN_NOT_INCLUDED);
  }
  else
  {
    /*
     * SIM is inserted
     */
    if (service EQ LIMITED_SERVICE)
    {
      mm_mmgmm_nreg_ind (NREG_LIMITED_SERVICE, 
                         SEARCH_NOT_RUNNING,
                         FORB_PLMN_NOT_INCLUDED);
    }
    else
    {
      /*
       * Send new PLMN identification to MMI or GMM. 
       * In case we compile for a GPRS protocol stack, 
       * we will not indicate end of procedure (entering IDLE state)
       * by MMGMM_REG_CNF, but sending an intermediate result (full service)
       * by MMGMM_LUP_ACCEPT_IND.
       */
#ifdef GPRS
      if (mm_data->gprs.reg_cnf_on_idle_entry)
      {
        mm_mmgmm_lup_accept_ind ();
      }
      else
#endif
      {
        mm_mmgmm_reg_cnf ();
      }

      /* Check HPLMN timer state */
      reg_check_hplmn_tim (mm_data->reg.thplmn);

    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mm_cell_selected       |
+--------------------------------------------------------------------+

  PURPOSE : MM indicates successfull cell selection.

*/

GLOBAL void reg_mm_cell_selected (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_mm_cell_selected()");

  if (mm_gsm_alone() AND 
      mm_data->reg.op.sim_ins EQ SIM_INSRT)
  {
    /*
     * GPRS is inactive (GSM only mobile or GPRS deactivated) AND
     * a valid SIM is inserted. These are the prerequisites to indicate
     * full service to the MMI after cell selection.
     */
    if (!mm_normal_upd_needed())
    {
      /* 
       * If no normal update is needed, the next service state 
       * is "normal service" regardless of the need of an IMSI 
       * ATTACH. Normal calls are possible, so we have to indicate 
       * full service to the MMI.
       */
      mm_mmgmm_reg_cnf ();
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : MM_REG                     |
| STATE   : code                ROUTINE : reg_mm_failure             |
+--------------------------------------------------------------------+

  PURPOSE : MM indicates an internal failure during location updating.
  
*/

GLOBAL void reg_mm_failure (UBYTE forb_ind)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("reg_mm_failure()");

  if (!mm_lup_allowed_by_gmm())
  {
    /* 
     * Call of function caused by MMGMM_ATTACH_REJ_REQ 
     * and not by own failed LOCATION UPDATING ATTEMPT - or -
     * by MMGMM_NREG_REQ with (cs EQ CS_DISABLE).
     */

    // This means, in automatic mode we wait for a 
    // GMM decision to try a location update in automatic 
    // mode if there are further networks present. 
    // We have to discuss this item for network mode I!

    /* 
     * Check whether the MM failure was caused by MMGMM_NREG_REQ.
     */
    if (mm_data->nreg_request) 
      mm_mmgmm_nreg_cnf (mm_data->nreg_cause);
    return;
  }

  /* 
   * The MM failure was caused by MM's own location updating procedure, 
   * LOCATION UPDATING REJECT message has been received.
   */

  /*
   * In automatic mode with valid SIM, select the next PLMN if available
   */
  reg_plmn_select (forb_ind);
}

#endif

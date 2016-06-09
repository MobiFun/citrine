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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by the
|             registrations part of GPRS mobility management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_GMMP_C
#define PSA_GMMP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gmm.h"
#include "psa_mm.h"
#include "cmh.h"
#include "cmh_gmm.h"

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif
#include "dcm_f.h"
#include "rvm/rvm_api.h"



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_attach_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_ATTACH_CNF primitive send by GMM.
            this confirms a successful attach. 
*/
GLOBAL void psa_gmmreg_attach_cnf ( T_GMMREG_ATTACH_CNF *gmmreg_attach_cnf )
{

  TRACE_FUNCTION ("psa_gmmreg_attach_cnf()");

  GMM_PRIM_TRACE_2("ATT_CNF",dbg_attachType(gmmreg_attach_cnf->attach_type),
                   dbg_searchRunning(gmmreg_attach_cnf->search_running));

  
  /* Attach Cnf will be sent when RR starts the power scanning 
     and hence ignore it */
  if (gmmreg_attach_cnf->bootup_cause EQ PWR_SCAN_START)
  {
    TRACE_EVENT("Dummy cnf from the lower layer");
    PFREE(gmmreg_attach_cnf);
    return;
  }

/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  cmhGMM_setPLMN(&gmmreg_attach_cnf->plmn);
  gmmShrdPrm.lac = gmmreg_attach_cnf->lac;
  gmmShrdPrm.cid = gmmreg_attach_cnf->cid;

  /* ACI-SPR-17218: Map gprs_indicator to ACI state */
  switch( gmmreg_attach_cnf->gprs_indicator )
  {
    case GMM_GPRS_SUPP_NO:
      gmmShrdPrm.gprs_indicator = P_CREG_GPRS_Not_Supported;
      break;
    case GMM_GPRS_SUPP_LIMITED:
      gmmShrdPrm.gprs_indicator = P_CREG_GPRS_Supported_Limited_Serv;
      break;
    case GMM_GPRS_SUPP_YES:
      gmmShrdPrm.gprs_indicator = P_CREG_GPRS_Supported;
      break;
    default:
      gmmShrdPrm.gprs_indicator = P_CREG_GPRS_Support_Unknown;      
      break;
  }
  gmmShrdPrm.rt  = gmmreg_attach_cnf->rt;
  
  TRACE_EVENT_P2("NEW ! gmmShrdPrm.lac: %04X, gmmShrdPrm.cid: %04X", gmmShrdPrm.lac, gmmShrdPrm.cid);

  psaGMM_NetworkRegistrationStatus(GMMREG_ATTACH_CNF, gmmreg_attach_cnf);
  
  cmhGMM_Attached( gmmreg_attach_cnf->attach_type, &gmmreg_attach_cnf->plmn, gmmreg_attach_cnf->search_running );

  switch ( gmmreg_attach_cnf->attach_type )
  {
    case GMMREG_AT_GPRS:
    case GMMREG_AT_COMB:
    case GMMREG_AT_IMSI:
      {
        PALLOC (mmr_reg_cnf, MMR_REG_CNF);
        memcpy(&mmr_reg_cnf->plmn, &gmmreg_attach_cnf->plmn, sizeof(T_plmn));
        mmr_reg_cnf->lac = gmmreg_attach_cnf->lac;
        mmr_reg_cnf->cid = gmmreg_attach_cnf->cid;
        mmr_reg_cnf->bootup_cause = gmmreg_attach_cnf->bootup_cause;
        psa_mmr_reg_cnf(mmr_reg_cnf);
      }
      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_attach_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_attach_rej   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_ATTACH_REJ primitive send by GMM.
            this confirms a failed attach. 
*/
EXTERN T_DCM_ENV_CTRL_BLK *dcm_env_ctrl_blk_p;
GLOBAL void psa_gmmreg_attach_rej ( T_GMMREG_ATTACH_REJ *gmmreg_attach_rej )
{

  TRACE_FUNCTION ("psa_gmmreg_attach_rej()");

  GMM_PRIM_TRACE_2("ATT_REJ",dbg_detachType(gmmreg_attach_rej->detach_type),
                   dbg_searchRunning(gmmreg_attach_rej->search_running));
/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */
//pinghua add to avoid no net signal case 
#ifdef FF_GPF_TCPIP
  if(is_gpf_tcpip_call())
  {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG; 
    dcm_send_message(msg, dcm_env_ctrl_blk_p->substate[0]);
    		
  }
#endif /* FF_GPF_TCPIP */

//end 

  psaGMM_NetworkRegistrationStatus(GMMREG_ATTACH_REJ, gmmreg_attach_rej);

  cmhGMM_NoAttach (gmmreg_attach_rej->detach_type,gmmreg_attach_rej->service, 
                   gmmreg_attach_rej->search_running );

  switch ( gmmreg_attach_rej->detach_type )
  {
    case GMMREG_DT_COMB:
    case GMMREG_DT_IMSI:
      /*
       *    No MM information, if
       *    IMSI wasn' t registered and wasn' t requested
       */
      if (  ( gmmShrdPrm.last_attach_type       EQ  ATTACH_TYPE_DETACHED OR  /* wasn' t registered */
              gmmShrdPrm.last_attach_type       EQ  GMMREG_AT_GPRS          )  AND
            ( gmmShrdPrm.requested_attach_type  EQ  ATTACH_TYPE_DETACHED OR  /* wasn' t requested */
              gmmShrdPrm.requested_attach_type  EQ  GMMREG_AT_GPRS          )       )
      break;

        /* else walk through */
        /*lint -fallthrough */
    case GMMREG_DT_LIMITED_SERVICE:
    case GMMREG_DT_SIM_REMOVED:
      {
        PALLOC (mmr_nreg_ind, MMR_NREG_IND); /* T_MMR_NREG_IND */
        mmr_nreg_ind->cause   = gmmreg_attach_rej->cause;
        mmr_nreg_ind->service = 
          cmhGMM_translate_gmm_cause_to_nreg_cs(gmmreg_attach_rej->service);
        mmr_nreg_ind->search_running = gmmreg_attach_rej->search_running;
        psa_mmr_nreg_ind(mmr_nreg_ind);
      }
      break;
    case GMMREG_DT_POWER_OFF:
    case GMMREG_DT_GPRS:
    case GMMREG_DT_DISABLE_GPRS:
    default:
      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_attach_rej);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_detach_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_DETACH_CNF primitive send by GMM.
            this confirms a successful detach. 
*/
GLOBAL void psa_gmmreg_detach_cnf ( T_GMMREG_DETACH_CNF *gmmreg_detach_cnf )
{
  TRACE_FUNCTION ("psa_gmmreg_detach_cnf()");

  GMM_PRIM_TRACE_1("DET_CNF",
                   dbg_detachType(gmmreg_detach_cnf->detach_type));

/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  psaGMM_NetworkRegistrationStatus(GMMREG_DETACH_CNF, gmmreg_detach_cnf);

  cmhGMM_Detached( gmmreg_detach_cnf->detach_type );

  switch ( gmmreg_detach_cnf->detach_type )
  {
    case GMMREG_DT_SOFT_OFF:
      {
        PALLOC (mmr_nreg_cnf, MMR_NREG_CNF);
        mmr_nreg_cnf->detach_cause = CS_SOFT_OFF;
        psa_mmr_nreg_cnf(mmr_nreg_cnf);
      }
      break;

    case GMMREG_DT_POWER_OFF:
      {
        PALLOC (mmr_nreg_cnf, MMR_NREG_CNF);
        mmr_nreg_cnf->detach_cause = CS_POW_OFF;
        psa_mmr_nreg_cnf(mmr_nreg_cnf);
      }
      break;

    case GMMREG_DT_COMB:  /* is set for e.g. AT+COPS=2 or AT%NRG=0,2 */
    case GMMREG_DT_SIM_REMOVED:
      {
        PALLOC (mmr_nreg_cnf, MMR_NREG_CNF);
        mmr_nreg_cnf->detach_cause = CS_SIM_REM;
        psa_mmr_nreg_cnf(mmr_nreg_cnf);
      }
      break;

    case GMMREG_DT_GPRS:
    case GMMREG_DT_IMSI:
    case GMMREG_DT_DISABLE_GPRS:
    case GMMREG_DT_LIMITED_SERVICE:
      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_detach_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_detach_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_DETACH_IND primitive send by GMM.
            the network initiated detach. 
*/
EXTERN T_DCM_ENV_CTRL_BLK *dcm_env_ctrl_blk_p;

GLOBAL void psa_gmmreg_detach_ind ( T_GMMREG_DETACH_IND *gmmreg_detach_ind )
{

  TRACE_FUNCTION ("psa_gmmreg_detach_ind()");

  GMM_PRIM_TRACE_2("DET_IND",dbg_detachType(gmmreg_detach_ind->detach_type),
                   dbg_searchRunning(gmmreg_detach_ind->search_running));



//pinghua add to avoid net drop off due to no signal 
#ifdef FF_GPF_TCPIP
  if(is_gpf_tcpip_call())
  {

   if(dcm_env_ctrl_blk_p->state[0] == DCM_CONN_ACTIVATED AND 
       dcm_env_ctrl_blk_p->substate[0] == DCM_SUB_NO_ACTION )  
   	{
 T_DCM_CLOSE_CONN_REQ_MSG dcm_close_conn_req_msg;
  U8 current_row = dcm_env_ctrl_blk_p->current_row ;	  

  memset(&dcm_close_conn_req_msg, 0x00, sizeof(T_DCM_CLOSE_CONN_REQ_MSG));

  dcm_close_conn_req_msg.hdr.msg_id = DCM_CLOSE_CONN_REQ_MSG;
  dcm_close_conn_req_msg.close_req.api_instance  = dcm_env_ctrl_blk_p->ipu_list[current_row].api_instance;
  dcm_close_conn_req_msg.close_req.bearer_handle = dcm_env_ctrl_blk_p->ipu_list[current_row].bearer_handle;
  dcm_process_network_drop(&dcm_close_conn_req_msg);

		  
	}
  }
#endif /* FF_GPF_TCPIP */

//end 


/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  psaGMM_NetworkRegistrationStatus(GMMREG_DETACH_IND, gmmreg_detach_ind);
  
  cmhGMM_NetDetach( gmmreg_detach_ind->detach_type, 
                     gmmreg_detach_ind->service, 
                    gmmreg_detach_ind->search_running );
  switch ( gmmreg_detach_ind->detach_type )
  {
    case GMMREG_DT_COMB:
    case GMMREG_DT_IMSI:
    case GMMREG_DT_LIMITED_SERVICE:
      {
        PALLOC (mmr_nreg_ind, MMR_NREG_IND);
        mmr_nreg_ind->cause   = gmmreg_detach_ind->cause;
        mmr_nreg_ind->service = 
          cmhGMM_translate_gmm_cause_to_nreg_cs(gmmreg_detach_ind->service);
        mmr_nreg_ind->search_running = gmmreg_detach_ind->search_running;
        psa_mmr_nreg_ind(mmr_nreg_ind);
      }
      break;
    case GMMREG_DT_POWER_OFF:
    case GMMREG_DT_SIM_REMOVED:
    case GMMREG_DT_GPRS:
    case GMMREG_DT_DISABLE_GPRS:
      cmhGMM_send_NetworkRegistrationStatus( CGREG_STAT_NOT_REG, P_CGREG_STAT_NOT_REG );
    /*lint -fallthrough */
    default:
      break;
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_detach_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_plmn_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_PLMN_IND primitive send by GMM.
            this indicate a PLMN list to the MMI. 
*/

GLOBAL void psa_gmmreg_plmn_ind       ( T_GMMREG_PLMN_IND *gmmreg_plmn_ind )
{
  short i;
  TRACE_FUNCTION ("psa_gmmreg_plmn_ind()");

/*
 *-------------------------------------------------------------------
 * copy primitive for MM and update shared parameter from GMM
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mmr_plmn_ind, MMR_PLMN_IND); /* T_MMR_PLMN_IND */

    mmr_plmn_ind->cause = gmmreg_plmn_ind->cause;

    for (i = 0; i < MAX_PLMN_ID; i++)
    {
      mmr_plmn_ind->plmn[i]     = gmmreg_plmn_ind->plmn[i];
      mmr_plmn_ind->forb_ind[i] = gmmreg_plmn_ind->forb_ind[i];
      mmr_plmn_ind->rxlevel[i]  = gmmreg_plmn_ind->rxlevel[i];
      mmr_plmn_ind->lac_list[i]  = gmmreg_plmn_ind->lac_list[i];
      /* shared parameter */
      gmmShrdPrm.gprs_status[i]  = gmmreg_plmn_ind->gprs_status[i];

      if ( mmr_plmn_ind->plmn[i].v_plmn EQ INVLD_PLMN )
        break;
    }
  
/*
 *-------------------------------------------------------------------
 * call MM primitive handler for update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */  
    psa_mmr_plmn_ind(mmr_plmn_ind);
  }
/*
 *-------------------------------------------------------------------
 * notify ACI
 *-------------------------------------------------------------------
 */  
  cmhGMM_Plmn();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_plmn_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_suspend_ind  |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_SUSPEND_IND primitive send by GMM.
            this inform MMI if whether full service or limited service is available. 
*/
GLOBAL void psa_gmmreg_suspend_ind( T_GMMREG_SUSPEND_IND *gmmreg_suspend_ind )
{

  TRACE_FUNCTION ("psa_gmmreg_suspend_ind()");

  GMM_PRIM_TRACE_1("SUS_IND",dbg_cellState(gmmreg_suspend_ind->cell_state));

/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  psaGMM_NetworkRegistrationStatus(GMMREG_SUSPEND_IND, gmmreg_suspend_ind);

  cmhGMM_Suspend( gmmreg_suspend_ind->cell_state );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_suspend_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_resume_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_RESUME_IND primitive send by GMM.
            this inform MMI that GPRS full service is now available. 
*/
GLOBAL void psa_gmmreg_resume_ind( T_GMMREG_RESUME_IND *gmmreg_resume_ind )
{

  TRACE_FUNCTION ("psa_gmmreg_resume_ind()");

/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  psaGMM_NetworkRegistrationStatus(GMMREG_RESUME_IND, gmmreg_resume_ind);

  cmhGMM_Resume();

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_resume_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_info_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_INFO_IND primitive send by GMM.
            this transmit the information provided by the GMM INFORMATION message to the MMI. 
*/
GLOBAL void psa_gmmreg_info_ind( T_GMMREG_INFO_IND *gmmreg_info_ind )
{

  TRACE_FUNCTION ("psa_gmmreg_info_ind()");

/*
 *-------------------------------------------------------------------
 *  notify ACI
 *-------------------------------------------------------------------
 */  
  {
    PALLOC (mmr_info_ind, MMR_INFO_IND);
    memcpy (&(mmr_info_ind->plmn), &(gmmreg_info_ind->plmn), sizeof (T_plmn));
    memcpy (&(mmr_info_ind->full_name), &(gmmreg_info_ind->full_net_name_gmm), sizeof (T_full_net_name_gmm));
    memcpy (&(mmr_info_ind->short_name), &(gmmreg_info_ind->short_net_name_gmm), sizeof (T_short_net_name_gmm));
    memcpy (&(mmr_info_ind->ntz), &(gmmreg_info_ind->net_time_zone), sizeof (T_net_time_zone));
    memcpy (&(mmr_info_ind->time), &(gmmreg_info_ind->net_time), sizeof (T_net_time));

    psa_mmr_info_ind(mmr_info_ind);
  }
  
  /*cmhGMM_Info();*/

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_info_ind);

}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_ciphering_ind|
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_CIPHERING_IND primitive send by GMM.
*/
GLOBAL void psa_gmmreg_ciphering_ind(T_GMMREG_CIPHERING_IND *gmmreg_ciphering_ind)
{

  TRACE_FUNCTION ("psa_gmmreg_ciphering_ind()");

  cmhGMM_CipheringInd (gmmreg_ciphering_ind->gsm_ciph,gmmreg_ciphering_ind->gprs_ciph);


/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (gmmreg_ciphering_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMP                |
|                                 ROUTINE : psa_gmmreg_ahplmn_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the GMMREG_AHPLMN_IND primitive send by GMM.
*/
GLOBAL void psa_gmmreg_ahplmn_ind(T_GMMREG_AHPLMN_IND *gmmreg_ahplmn_ind)
{
  TRACE_FUNCTION("psa_gmmreg_ahplmn_ind()");

  {
    PALLOC(mmr_ahplmn_ind, MMR_AHPLMN_IND);
    mmr_ahplmn_ind->ahplmn = gmmreg_ahplmn_ind->ahplmn;
    psa_mmr_ahplmn_ind(mmr_ahplmn_ind);
  }

  psaGMM_NetworkRegistrationStatus(GMMREG_AHPLMN_IND, gmmreg_ahplmn_ind);

  PFREE(gmmreg_ahplmn_ind);
}


#endif  /* GPRS */
/*==== EOF =========================================================*/

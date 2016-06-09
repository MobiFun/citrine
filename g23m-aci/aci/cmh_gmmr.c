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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for 
|             GPRS mobility management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI) 

#ifndef CMH_GMMR_C
#define CMH_GMMR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti_conn_mng.h"

#include "aci.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_mm.h"
#include "psa_gmm.h"
#include "psa_sim.h"

#include "cmh.h"
#include "cmh_mm.h"
#include "cmh_gmm.h"
#include "cmh_sim.h"

#if defined (FF_WAP) OR defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_sm.h"
#endif /* WAP */

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
#include "psa_cc.h"
#include "psa_sat.h"
#include "cmh_sat.h"
#endif  /* SIM_TOOLKIT AND FF_SAT_E */

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif

#include "dcm_f.h"
#include "cmh_sm.h"
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Attached              |
+-------------------------------------------------------------------+

  PURPOSE : ME is attached

*/
GLOBAL void cmhGMM_Attached ( UBYTE attach_type, T_plmn *plmn, UBYTE search_running )
{
  T_CGEREP_EVENT_REP_PARAM  event;
  SHORT i;
  
  TRACE_FUNCTION ("cmhGMM_Attached()");

 /*
  *   set GPRS attach state
  */
  gmmShrdPrm.current_attach_type = attach_type;  
  
  switch ( attach_type )
  {
    case GMMREG_AT_GPRS:
    case GMMREG_AT_COMB:
      cmhGMM_Set_state(AS_ATTACHED);
      break;
    case GMMREG_AT_IMSI:
      cmhGMM_Set_state(AS_DETACHED);
      break;
  }

  if (search_running EQ GMMREG_SEARCH_NOT_RUNNING)
  {
    gmmShrdPrm.mobile_class = gmmShrdPrm.requested_mobile_class;
    gaciMobileClass.current = gaciMobileClass.requested;

    if(AT_CMD_CGCLASS EQ gmmEntStat.curCmd)
    {
    /*
     *  brz: 22.10.02
     *  special behaviour for NMO III
     *
     *  In the case of NMO III a COMB reject can not be succesful.
     *
     *  case BG requested: attach type GPRS is also OK
     *  case BC requested: attach type IMSI is also OK
     */
      if( GMMREG_CLASS_BG EQ gmmShrdPrm.mobile_class AND GMMREG_AT_GPRS EQ attach_type  OR
          GMMREG_CLASS_BC EQ gmmShrdPrm.mobile_class AND GMMREG_AT_IMSI EQ attach_type     )
      {
       /*
        *   GPRS event reporting>
        */
        event.mobile_class = (T_CGCLASS_CLASS)gmmShrdPrm.mobile_class;
        for( i = 0 ; i < CMD_SRC_MAX; i++ )
        {
          R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_CLASS, &event ); 
          R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_CLASS, &event );
        }
        R_AT( RAT_OK, gmmEntStat.entOwn ) ( gmmEntStat.curCmd );

        /* log result */
        cmh_logRslt ( gmmEntStat.entOwn, RAT_OK, gmmEntStat.curCmd, 
                               -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );

        gmmEntStat.curCmd = AT_CMD_NONE;
        return;
      }
    }
  }
    
  if ( attach_type EQ GMMREG_AT_IMSI AND 
       gmmShrdPrm.requested_mobile_class NEQ GMMREG_CLASS_CC)
  {
  /*
   *  brz: 03.05.02
   *  patch for wrong behaviour of GMM: sending attach_cnf->IMSI instead of attach_rej->GPRS
   */
    if (search_running EQ GMMREG_SEARCH_NOT_RUNNING)
    {
      cmhGMM_NoAttach(GMMREG_DT_GPRS ,GMMCS_NO_SERVICE , GMMREG_SEARCH_NOT_RUNNING);
    }

    return;
  }
  else  /* end of +CGATT or +CGCLASS */
  {
   /*
   *  Inform CMH SM about new attach state.
    */

    cmhSM_GprsAttached( GPRS_ATTACH );
    
    
    switch ( gmmEntStat.curCmd )
    {
      case AT_CMD_CGCLASS:
       /*
        *   GPRS event reporting
        */
        event.mobile_class = (T_CGCLASS_CLASS)gmmShrdPrm.requested_mobile_class;
        for( i = 0 ; i < CMD_SRC_MAX; i++ )
        {
          R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_CLASS, &event ); 
          R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_CLASS, &event );
        }
        /*lint -fallthrough */
      case AT_CMD_CGATT:
        R_AT( RAT_OK, gmmEntStat.entOwn ) ( gmmEntStat.curCmd );

        /* log result */
        cmh_logRslt ( gmmEntStat.entOwn, RAT_OK, gmmEntStat.curCmd, -1,
                               BS_SPEED_NotPresent,CME_ERR_NotPresent );

#ifdef FF_GPF_TCPIP
        if(is_gpf_tcpip_call())
        {
          T_DCM_STATUS_IND_MSG msg;
          msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
          dcm_send_message(msg, DCM_SUB_WAIT_CGATT_CNF);
        }
#endif /* FF_GPF_TCPIP */
        break;
    }

    gmmEntStat.curCmd = AT_CMD_NONE;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_NoAttach              |
+-------------------------------------------------------------------+

  PURPOSE : the attach fail

*/
GLOBAL void cmhGMM_NoAttach ( UBYTE detach_type, USHORT cause, UBYTE search_running )
{
  T_ACI_CME_ERR cme_err;          /* error number */
  
  TRACE_FUNCTION ("cmhGMM_NoAttach()");

 /*
  *   map error cause
  */
  switch(cause)
  {
    case GMMCS_ILLEGAL_MS:
    case MMCS_ILLEGAL_MS:
      cme_err = CME_ERR_GPRSBadMs;
      break;
    case GMMCS_ILLEGAL_ME:
    case MMCS_ILLEGAL_ME:
      cme_err = CME_ERR_GPRSBadMe;
      break;
    case GMMCS_GPRS_NOT_ALLOWED:
    /* No corrosponding MM cause */
      cme_err = CME_ERR_GPRSNoService;
      break;
    case GMMCS_PLMN_NOT_ALLOWED:
    case MMCS_PLMN_NOT_ALLOWED:
      cme_err = CME_ERR_GPRSBadPlmn;
      break;
    case GMMCS_LA_NOT_ALLOWED:
    case MMCS_LA_NOT_ALLOWED:
      cme_err = CME_ERR_GPRSBadLoc;
      break;
    case GMMCS_ROAMING_NOT_ALLOWED:
    case MMCS_ROAMING_NOT_ALLOWED:
      cme_err = CME_ERR_GPRSNoRoam;
      break;
    case GMMCS_IMPLICIT_DETACHED:
    /* No corrosponding MM cause */
      cme_err = CME_ERR_GPRSUnspec;
      break;
    case GMMCS_IMSI_UNKNOWN:
    case MMCS_IMSI_IN_HLR:
    case MMCS_IMEI_NOT_ACCEPTED:
    /* No corrosponding GMM cause */
    case GMMCS_NO_MS_ID:
    /* No corrosponding MM cause */
      cme_err = CME_ERR_SimFail;
      break;
    case GMMCS_GSM_GPRS_NOT_ALLOWED:
    /* No corrosponding MM cause */
    case GMMCS_NET_FAIL:
    case MMCS_NETWORK_FAILURE:
    case GMMCS_MSC_TEMP_NOT_REACHABLE:
    /* No corrosponding MM cause */
    case GMMCS_CONGESTION:
    case MMCS_CONGESTION:
      cme_err = CME_ERR_NoServ;
      break;
    /* case GMMCS_CELL_SELECTION_FAILED: */
    /* No corrosponding MM cause */
    /* case GMMCS_NO_SERVICE: */
    /* No corrosponding MM cause */
    /* case GMMCS_LIMITED_SERVICE: */
    /* No corrosponding MM cause */
/*  case GMMCS_SEMANTIC_INCORRECT:
    case MMCS_INCORRECT_MESSAGE:
    case GMMCS_INVALID_M_INFO:
    case MMCS_INVALID_MAND_MESSAGE:
    case GMMCS_TYPE_INVALID:
    case MMCS_MESSAGE_TYPE_NOT_IMPLEM:
    case GMMCS_TYPE_INCOMPATIBLE:
    case MMCS_MESSAGE_TYPE_INCOMPAT:
    case GMMCS_IE_INVALID:
    case MMCS_IE_NOT_IMPLEM:
    case GMMCS_COND_IE_ERROR:
    case MMCS_CONDITIONAL_IE:
    case GMMCS_MESSAGE_INVALID:
    case MMCS_MESSAGE_INCOMPAT:
    case GMMCS_INT_PROTOCOL_ERROR: */
    /* No corrosponding MM cause */
    default:
      cme_err = CME_ERR_Unknown;
  }
        
  if ( search_running EQ GMMREG_SEARCH_NOT_RUNNING ) 
  {
   /*
    *  Inform CMH SM about new attach state 
    */
    cmhSM_GprsAttached( GPRS_DETACH );

    switch ( gmmEntStat.curCmd )
    {
      case AT_CMD_CGCLASS:
      case AT_CMD_CGATT:
          R_AT( RAT_CME, gmmEntStat.entOwn ) ( gmmEntStat.curCmd, cme_err );

          /* log result */
          cmh_logRslt ( gmmEntStat.entOwn, RAT_CME, gmmEntStat.curCmd, -1, 
                                 BS_SPEED_NotPresent, cme_err );
          break;
    }

    gmmEntStat.curCmd = AT_CMD_NONE;
  }

 /*
  *   set GPRS attach state
  */
  switch ( detach_type )
  {
    case GMMREG_DT_LIMITED_SERVICE:
    case GMMREG_DT_SIM_REMOVED:
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_DISABLE_GPRS:
      cmhGMM_Set_state(AS_SUSPENTED);
      break;
    case GMMREG_DT_POWER_OFF:
      break;
    case GMMREG_DT_GPRS:
    case GMMREG_DT_COMB:
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_IMSI:
      break;
  }

  switch ( detach_type )
  {
    case GMMREG_DT_GPRS:
      gmmShrdPrm.current_attach_type = GMMREG_AT_IMSI;
      break;
    case GMMREG_DT_IMSI:
      gmmShrdPrm.current_attach_type = GMMREG_AT_GPRS;
      break;
    case GMMREG_DT_COMB:
    case GMMREG_DT_POWER_OFF:
    case GMMREG_DT_LIMITED_SERVICE:
    case GMMREG_DT_SIM_REMOVED:
      gmmShrdPrm.current_attach_type = ATTACH_TYPE_DETACHED;
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Detached              |
+-------------------------------------------------------------------+

  PURPOSE : ME is detached

*/
GLOBAL void cmhGMM_Detached ( UBYTE detach_type )
{
  SHORT i;

  TRACE_FUNCTION ("cmhGMM_Detached()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch ( detach_type )
  {
    case GMMREG_DT_GPRS:
    case GMMREG_DT_COMB:
      for( i = 0 ; i < CMD_SRC_MAX; i++ )
      {
        R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DETACH, NULL ); 
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DETACH, NULL );
      }
      
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_POWER_OFF:
      for( i = 0 ; i < CMD_SRC_MAX; i++ )
      {
        R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DETACH, NULL ); 
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DETACH, NULL );
      }
      
      cmhGMM_Set_state(AS_MOBILE_OFF);
      break;
    case GMMREG_DT_LIMITED_SERVICE:
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_SIM_REMOVED:
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_IMSI:
      cmhGMM_Set_state(AS_ATTACHED);
      break;
    case GMMREG_DT_DISABLE_GPRS:
      cmhGMM_Set_state(AS_SUSPENTED);
      break;
    default:
      break;
  }

  switch ( detach_type )
  {
    case GMMREG_DT_IMSI:
      gmmShrdPrm.current_attach_type = GMMREG_AT_GPRS;
      break;
    case GMMREG_DT_GPRS:
      gmmShrdPrm.current_attach_type = GMMREG_AT_IMSI;
      break;
    case GMMREG_DT_COMB:
    case GMMREG_DT_LIMITED_SERVICE:
    case GMMREG_DT_SIM_REMOVED:
    case GMMREG_DT_POWER_OFF:
      gmmShrdPrm.current_attach_type = ATTACH_TYPE_DETACHED;
      break;
  }

 /*
  *  Inform CMH SM about new attach state 
  */
  cmhSM_GprsAttached( GPRS_DETACH );


  switch( gmmEntStat.curCmd )
  {
    case( AT_CMD_CGCLASS ):
    case( AT_CMD_CGATT ):
      
      R_AT( RAT_OK, gmmEntStat.entOwn ) ( gmmEntStat.curCmd );

      /* log result */
      cmh_logRslt ( gmmEntStat.entOwn, RAT_OK, gmmEntStat.curCmd, 
                             -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );
      break;
  }
  gmmEntStat.curCmd = AT_CMD_NONE;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_NetDetach             |
+-------------------------------------------------------------------+

  PURPOSE : ME is detached by the network

*/
GLOBAL void cmhGMM_NetDetach ( UBYTE detach_type, USHORT cause, UBYTE search_running )
{
  UBYTE cme_err;          /* error number */
  SHORT i;


  TRACE_FUNCTION ("cmhGMM_NetDetach()");

  switch (detach_type)
  {
    case GMMREG_DT_GPRS:
    case GMMREG_DT_COMB:
      for( i = 0 ; i < CMD_SRC_MAX; i++ )
      {
        R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_DETACH, NULL ); 
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_DETACH, NULL );
      }
    /*lint -fallthrough */
    case GMMREG_DT_SIM_REMOVED:
    case GMMREG_DT_LIMITED_SERVICE:
      cmhGMM_Set_state(AS_DETACHED);
      break;
    case GMMREG_DT_IMSI:
      cmhGMM_Set_state(AS_ATTACHED);
      break;
    case GMMREG_DT_DISABLE_GPRS:
      cmhGMM_Set_state(AS_SUSPENTED);
      break;
    case GMMREG_DT_POWER_OFF:
    default:
      break;
  }

  switch (detach_type)
  {
    case GMMREG_DT_GPRS:
      gmmShrdPrm.current_attach_type = GMMREG_AT_IMSI;
      break;
    case GMMREG_DT_IMSI:
      gmmShrdPrm.current_attach_type = GMMREG_AT_GPRS;
      break;
    case GMMREG_DT_COMB:
    case GMMREG_DT_SIM_REMOVED:
    case GMMREG_DT_LIMITED_SERVICE:
    case GMMREG_DT_POWER_OFF:
      gmmShrdPrm.current_attach_type = ATTACH_TYPE_DETACHED;
      break;
  }

  if( search_running EQ GMMREG_SEARCH_NOT_RUNNING)
  {
    if (work_cids[cid_pointer] NEQ PDP_CONTEXT_CID_INVALID)
    {
      /*
       * Inform CMH SM about the new attach state.
       */
      cmhSM_GprsAttached( GPRS_DETACH );
    }
    else
    {
      /*
       * map error cause
       */
      switch(cause)
      {
        case GMMCS_ILLEGAL_MS:
        case MMCS_ILLEGAL_MS:
          cme_err = CME_ERR_GPRSBadMs;
          break;
        case GMMCS_ILLEGAL_ME:
        case MMCS_ILLEGAL_ME:
          cme_err = CME_ERR_GPRSBadMe;
          break;
        case GMMCS_GPRS_NOT_ALLOWED:
        /* No corrosponding MM cause */
          cme_err = CME_ERR_GPRSNoService;
          break;
        case GMMCS_PLMN_NOT_ALLOWED:
        case MMCS_PLMN_NOT_ALLOWED:
          cme_err = CME_ERR_GPRSBadPlmn;
          break;
        case GMMCS_LA_NOT_ALLOWED:
        case MMCS_LA_NOT_ALLOWED:
          cme_err = CME_ERR_GPRSBadLoc;
          break;
        case GMMCS_ROAMING_NOT_ALLOWED:
        case MMCS_ROAMING_NOT_ALLOWED:
          cme_err = CME_ERR_GPRSNoRoam;
          break;
        case GMMCS_IMPLICIT_DETACHED:
        /* No corrosponding MM cause */
          cme_err = CME_ERR_GPRSUnspec;
          break;
        case GMMCS_IMSI_UNKNOWN:
        case MMCS_IMSI_IN_HLR:
        case MMCS_IMEI_NOT_ACCEPTED:
        /* No corrosponding GMM cause */
        case GMMCS_NO_MS_ID:
        /* No corrosponding MM cause */
          cme_err = CME_ERR_SimFail;
          break;
        case GMMCS_GSM_GPRS_NOT_ALLOWED:
        /* No corrosponding MM cause */
        case GMMCS_NET_FAIL:
        case MMCS_NETWORK_FAILURE:
        case GMMCS_MSC_TEMP_NOT_REACHABLE:
        /* No corrosponding MM cause */
        case GMMCS_CONGESTION:
        case MMCS_CONGESTION:
          cme_err = CME_ERR_NoServ;
          break;
        default:
          cme_err = CME_ERR_Unknown;
      }

      /* If we get detach indication with search_running as false, then we should 
         send error response to user. This is in the normal attach / cgclass case
       */

      switch ( gmmEntStat.curCmd )
      {
        case AT_CMD_CGCLASS:
        case AT_CMD_CGATT:
          R_AT( RAT_CME, gmmEntStat.entOwn ) ( gmmEntStat.curCmd, cme_err );

          /* log result */
          cmh_logRslt (gmmEntStat.entOwn, RAT_CME, gmmEntStat.curCmd,(SHORT)-1, (T_ACI_BS_SPEED)-1, (T_ACI_CME_ERR)cme_err );
          break;
      }
      gmmEntStat.curCmd = AT_CMD_NONE;
    }
  }

}
/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Plmn                  |
+-------------------------------------------------------------------+

  PURPOSE : reseive a PLMN list

*/
GLOBAL SHORT cmhGMM_Plmn ( void )
{
  TRACE_FUNCTION ("cmhGMM_Plmn()");


  return GMMH_CMD_OK;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Suspend               |
+-------------------------------------------------------------------+

  PURPOSE : full service or limited service is available

*/
GLOBAL SHORT cmhGMM_Suspend ( UBYTE cell_state )
{

  TRACE_FUNCTION ("cmhGMM_Suspend()");

  cmhGMM_Set_state( AS_SUSPENTED );

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)

  cmhSAT_OpChnGPRSStat(SAT_GPRS_SUSPEND, SAT_GPRS_INV_CAUSE);

#endif  /* SIM_TOOLKIT AND FF_SAT_E */

  return GMMH_CMD_OK;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Resume                |
+-------------------------------------------------------------------+

  PURPOSE : GPRS full service is now available

*/
GLOBAL SHORT cmhGMM_Resume ( void )
{

  TRACE_FUNCTION ("cmhGMM_Resume()");

  cmhGMM_Set_state( AS_ATTACHED );

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)

  cmhSAT_OpChnGPRSStat(SAT_GPRS_RESUME, SAT_GPRS_INV_CAUSE);

#endif  /* SIM_TOOLKIT AND FF_SAT_E */

  return GMMH_CMD_OK;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_GMMR                     |
|                            ROUTINE : cmhGMM_Info                  |
+-------------------------------------------------------------------+

  PURPOSE : reseive the information provided by the GMM INFORMATION message.

*/
GLOBAL SHORT cmhGMM_Info ( void )
{
  TRACE_FUNCTION ("cmhGMM_Info()");


  return GMMH_CMD_OK;
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_GMMR                      |
|                            ROUTINE : cmhGMM_CipheringInd           |
+-------------------------------------------------------------------+

  PURPOSE : ciphering indication received

*/

GLOBAL SHORT cmhGMM_CipheringInd ( UBYTE gsm_ciph, UBYTE gprs_ciph )
{
  SHORT idx;                  /* holds index counter */

  TRACE_FUNCTION ("cmhGMM_CipheringInd()");

  if (simShrdPrm.ciSIMEnabled NEQ TRUE)
  {
    return 1;
  }

  for (idx = 0; idx < CMD_SRC_MAX; idx++)
  {
     /*
     *-----------------------------------------------------------------
     * new message indication
     *-----------------------------------------------------------------
     */
     R_AT(RAT_P_CPRI,(T_ACI_CMD_SRC)idx)(gsm_ciph, gprs_ciph);
  }
  return 0;
}


#endif /* GPRS */
/*==== EOF ========================================================*/
 

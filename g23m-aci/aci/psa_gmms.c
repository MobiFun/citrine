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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for the registration part of
|             GPRS mobility management ( GMM ).
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef PSA_GMMS_C
#define PSA_GMMS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "gaci_cmh.h"
#include "aci.h"
#include "psa.h"
#include "psa_gmm.h"

#include "psa_sim.h"
#include "cmh.h"
#include "dti_conn_mng.h"
#include "cmh_sim.h"


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/
/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Attach           |
+-------------------------------------------------------------------+

  PURPOSE : start an attach

*/
GLOBAL void psaGMM_Attach ( UBYTE mobile_class, UBYTE attach_type, UBYTE service_mode )
{

  TRACE_FUNCTION ("psaGMM_Attach()");

  GMM_PRIM_TRACE_2("ATT_REQ",
                   dbg_mobileClass(mobile_class),
                   dbg_attachType(attach_type));
/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (gmmreg_attach_req, GMMREG_ATTACH_REQ);

    /* fill in primitive parameter: registration mode */
    gmmreg_attach_req -> mobile_class           = mobile_class;
    gmmreg_attach_req -> attach_type            = attach_type;
    gmmreg_attach_req -> service_mode           = service_mode;
    
    /* Fill the bootup action to NORMAL_REG to take
       appropriate actions by the lower layers */
    gmmreg_attach_req->bootup_act = NORMAL_REG;
    psaGMM_NetworkRegistrationStatus(GMMREG_ATTACH_REQ, gmmreg_attach_req);

    PSEND (hCommGMM, gmmreg_attach_req);
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Detach           |
+-------------------------------------------------------------------+

  PURPOSE : start an detach

*/
GLOBAL void psaGMM_Detach   ( UBYTE detach_type )
{

  TRACE_FUNCTION ("psaGMM_Detach()");

  GMM_PRIM_TRACE_1("DET_REQ",
                   dbg_detachType(detach_type));
/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (gmmreg_detach_req, GMMREG_DETACH_REQ);

    /* fill in primitive parameter: registration mode */
    gmmreg_detach_req -> detach_type = detach_type;
      
    PSEND (hCommGMM, gmmreg_detach_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Net_Req          |
+-------------------------------------------------------------------+

  PURPOSE : start a network search

*/
GLOBAL void psaGMM_Net_Req ( void )
{

  TRACE_FUNCTION ("psaGMM_Net_Req()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (gmmreg_net_req, GMMREG_NET_REQ);
    PSEND (hCommGMM, gmmreg_net_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Plmn_res         |
+-------------------------------------------------------------------+

  PURPOSE : select a PLMN after network search

*/
GLOBAL void psaGMM_Plmn_res ( UBYTE mobile_class, UBYTE attach_type, T_plmn *plmn )
{

  TRACE_FUNCTION ("psaGMM_Plmn_res()");

  GMM_PRIM_TRACE_2("PLMN_RES",
                   dbg_mobileClass(mobile_class),
                   dbg_attachType(attach_type));
/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (gmmreg_plmn_res, GMMREG_PLMN_RES);

    /* fill in primitive parameter: registration mode */
    gmmreg_plmn_res -> plmn             = *plmn;
    gmmreg_plmn_res -> attach_type      = attach_type;
    gmmreg_plmn_res -> mobile_class     = mobile_class;
      
    PSEND (hCommGMM, gmmreg_plmn_res);
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Plmn_mode_req    |
+-------------------------------------------------------------------+

  PURPOSE : switch between automatic and manual mode of registration

*/
GLOBAL void psaGMM_Plmn_mode_req   ( UBYTE net_selection_mode )
{

  TRACE_FUNCTION ("psaGMM_Plmn_mode_req()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (gmmreg_plmn_mode_req, GMMREG_PLMN_MODE_REQ);

    /* fill in primitive parameter: registration mode */
    gmmreg_plmn_mode_req -> net_selection_mode = net_selection_mode;
      
    PSEND (hCommGMM, gmmreg_plmn_mode_req);
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_GMMS                |
|                                 ROUTINE : psaGMM_Config_req       |
+-------------------------------------------------------------------+

  PURPOSE : send a config request to GMM.

*/
GLOBAL void psaGMM_Config_req( UBYTE cipher_on, UBYTE tlli_handling )
{
  TRACE_FUNCTION ("psaGMM_Config_req()");

  {
    PALLOC (gmmreg_config_req, GMMREG_CONFIG_REQ);

    gmmreg_config_req -> cipher_on  = cipher_on;
    gmmreg_config_req -> tlli_handling = tlli_handling;

    PSEND (hCommGMM, gmmreg_config_req);
  }

}

#endif  /* GPRS */
/*==== EOF ========================================================*/
 

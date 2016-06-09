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
|             primitives send to the protocol stack adapter for GPRS 
|             Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_GPPPP_C
#define PSA_GPPPP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "psa.h"
#include "psa_gppp.h"
#include "cmh.h"
#include "cmh_gppp.h"


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPP                |
| STATE   : finished              ROUTINE : psa_ppp_establish_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : Processes the PPP_ESTABLISH_REQ primitive send by PPP.
            This starts PPP in the correct mode. 
*/
GLOBAL void psa_gppp_establish_cnf ( T_PPP_ESTABLISH_CNF *ppp_establish_cnf )
{

  TRACE_FUNCTION ("psa_gppp_establish_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */  
  memcpy(&gpppShrdPrm.est, ppp_establish_cnf, sizeof(T_PPP_ESTABLISH_CNF));

  if ( cmhGPPP_Established() < 0)
  {
      TRACE_EVENT("cmhGPPP_Established() gives not supported return value.");
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (ppp_establish_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPP                |
| STATE   : finished              ROUTINE : psa_ppp_terminate_ind   |
+-------------------------------------------------------------------+

  PURPOSE : Processes the PPP_TERMINATE_IND primitive send by PPP.
            This indicates that the PPP connection is closed. 
*/
GLOBAL void psa_gppp_terminate_ind ( T_PPP_TERMINATE_IND *ppp_terminate_ind )
{

  TRACE_FUNCTION ("psa_gppp_terminate_ind()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */  
  gpppShrdPrm.ppp_cause  = ppp_terminate_ind->ppp_cause;
  
  gpppShrdPrm.tui      = DTI_ENTITY_PPPS;   /*ppp_terminate_ind->tui;*/

  if ( DTI_ENTITY_PPPS EQ gpppShrdPrm.tui)
  {
    if ( cmhGPPP_Terminated() < 0)
    {
        TRACE_EVENT("cmhGPPP_Terminated() gives not supported return value.");
    }
  }
  else
  {
    /* PPP Client with L2R - circuit switched */
  }
  
/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (ppp_terminate_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPP                 |
| STATE   : finished              ROUTINE : psa_ppp_pdp_activate_ind |
+--------------------------------------------------------------------+

  PURPOSE : Processes the PPP_PDP_ACTIVATE_IND primitive send by PPP.
            PPP is ready for PDP context activation (only in server mode).
*/
GLOBAL void psa_gppp_pdp_activate_ind ( T_PPP_PDP_ACTIVATE_IND *ppp_pdp_activate_ind )
{

  TRACE_FUNCTION ("psa_gppp_pdp_activate_ind()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */  
  gpppShrdPrm.pdp = ppp_pdp_activate_ind;

  if ( cmhGPPP_Activated() < 0)
  {
    TRACE_EVENT("cmhGPPP_Activated() give not supported return value.");
  }

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (ppp_pdp_activate_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPP                 |
| STATE   : finished              ROUTINE : psa_ppp_modification_cnf |
+--------------------------------------------------------------------+

  PURPOSE : Processes the PPP_MODIFICATION_CNF primitive send by PPP.
            this indicate the negotiated header compression (only in server mode). 
*/
GLOBAL void psa_gppp_modification_cnf ( T_PPP_MODIFICATION_CNF *ppp_modification_cnf )
{

  TRACE_FUNCTION ("psa_gppp_modification_cnf()");

/*
 *-------------------------------------------------------------------
 * update shared parameter and notify ACI
 *-------------------------------------------------------------------
 */ 
  /*
  gpppShrdPrm.ppp_hc = ppp_modification_cnf->ppp_hc;
  gpppShrdPrm.msid   = ppp_modification_cnf->msid;

  if ( cmhGPPP_Modified() < 0)
  {
      TRACE_EVENT("cmhGPPP_Modified() give not supported return value.");
  }
  */

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (ppp_modification_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPP                 |
| STATE   : finished              ROUTINE : psa_ppp_dti_connected_ind|
+--------------------------------------------------------------------+

  PURPOSE : Processes the PPP_MODIFICATION_CNF primitive send by PPP.
            this indicate the negotiated header compression (only in server mode). 
*/
GLOBAL void psa_gppp_dti_connected_ind ( T_PPP_DTI_CONNECTED_IND *ppp_dti_connected_ind )
{

  TRACE_FUNCTION ("psa_gppp_dti_connected_ind()");

/*
 *-------------------------------------------------------------------
 * inform DTI Manager
 *-------------------------------------------------------------------
 */  
  cmhGPPPS_DTIconnected( ppp_dti_connected_ind->connected_direction );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (ppp_dti_connected_ind);
}

#endif  /* GPRS */
/*==== EOF =========================================================*/

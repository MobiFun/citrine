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
|             protocol stack adapter for GPRS Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_GPPPS_C
#define PSA_GPPPS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "psa_gppp.h"

/*==== CONSTANTS ==================================================*/

#define TOK_STR     " =\t"          /* string of token delimiter */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : psaGPPP_Establish       |
+-------------------------------------------------------------------+

  PURPOSE : request establishment of PPP connection

*/
GLOBAL void psaGPPP_Establish ( T_PPP_ESTABLISH_REQ *est_req )
{

  TRACE_FUNCTION ("psaGPPP_Establish()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for establich PPP
 *-------------------------------------------------------------------
 */
  {
    PALLOC (ppp_establish_req, PPP_ESTABLISH_REQ);

    *ppp_establish_req = *est_req;

    PSEND (hCommPPP, ppp_establish_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : psaGPPP_Terminate       |
+-------------------------------------------------------------------+

  PURPOSE : close the PPP connection

*/
GLOBAL void psaGPPP_Terminate ( UBYTE lower_layer )
{

  TRACE_FUNCTION ("psaGPPP_Terminate()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
  {
    PALLOC (ppp_terminate_req, PPP_TERMINATE_REQ);

    /* fill in primitive parameter: registration mode */
    ppp_terminate_req -> lower_layer = lower_layer;
      
    PSEND (hCommPPP, ppp_terminate_req);
  }
}  

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : psaGPPP_PDP_Activate    |
+-------------------------------------------------------------------+

  PURPOSE : the PDP context is activated (only in server mode)

*/
GLOBAL void psaGPPP_PDP_Activate ( T_NAS_ip * ip, UBYTE *pco_buf, UBYTE pco_length, U8 nsapi)
{

  TRACE_FUNCTION ("psaGPPP_PDP_Activate()");

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */

  {
    PALLOC_SDU (ppp_pdp_activate_res, PPP_PDP_ACTIVATE_RES, (USHORT)(pco_length << 3));

    /* Issue OMAPS00047332: SM, SNDCP and UPM are not supporting ppp_hc and msid.
     * these two parameter should only be removed after 
     * PPP SAP modification. till then reset it to 0.
     */
    ppp_pdp_activate_res->ppp_hc    = 0; 
    ppp_pdp_activate_res->msid      = 0; 

#ifdef _SIMULATION_
#pragma message("A check of the IP version is needed before starting a PPP connection.")
#endif
    ppp_pdp_activate_res->ip  = ip->ip_address.ipv4_addr.a4[0] << 24;
    ppp_pdp_activate_res->ip += ip->ip_address.ipv4_addr.a4[1] << 16;
    ppp_pdp_activate_res->ip += ip->ip_address.ipv4_addr.a4[2] <<  8;
    ppp_pdp_activate_res->ip += ip->ip_address.ipv4_addr.a4[3];
    ppp_pdp_activate_res->sdu.l_buf = pco_length << 3;
    ppp_pdp_activate_res->sdu.o_buf = 0;
    if ( pco_length )
    {
      memcpy(&ppp_pdp_activate_res->sdu.buf, pco_buf, pco_length);
    }

    PSEND (hCommPPP, ppp_pdp_activate_res);
  }
}  

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : psaGPPP_PDP_Reject      |
+-------------------------------------------------------------------+

  PURPOSE : the PDP context can't activated (only in server mode)

*/
GLOBAL SHORT psaGPPP_PDP_Reject ( void )
{

  TRACE_FUNCTION ("psaGPPP_PDP_Reject()");


/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
  if(!psa_IsVldOwnId((T_OWN)gpppShrdPrm.owner)) 
    
    return( -1 );

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
      
  PSEND (hCommPPP, gpppShrdPrm.setPrm[gpppShrdPrm.owner].pdp_rej);

  return 0;
}  

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPS                |
| STATE   : finished              ROUTINE : psaGPPP_Modification    |
+-------------------------------------------------------------------+

  PURPOSE : negotiate header compression again (only in server mode)

*/
/* NEEDED ???*/


/* LOCAL SHORT psaGPPP_Modification ( void )*/
/* {*/
 /* T_PPP_SET_PRM * pPrmSet;       *//* points to used parameter set */
 /* TRACE_FUNCTION ("psaGPPP_Modification()");*/

/*
 *-------------------------------------------------------------------
 * check owner id
 *-------------------------------------------------------------------
 */  
/*   if(!psa_IsVldOwnId(gpppShrdPrm.owner)) 
    
    return( -1 );

  pPrmSet = &gpppShrdPrm.setPrm[gpppShrdPrm.owner]; */

/*
 *-------------------------------------------------------------------
 * create and send primitive for network attach
 *-------------------------------------------------------------------
 */
/*  {
    PALLOC (ppp_modification_req, PPP_MODIFICATION_REQ);*/

    /* fill in primitive parameter: registration mode */
/*    ppp_modification_req -> ppp_hc    = pPrmSet -> ppp_hc;
    ppp_modification_req -> msid      = pPrmSet -> msid;
      
    PSEND (hCommPPP, ppp_modification_req);
  }

  return 0;
}*/

#endif  /* GPRS */
/*==== EOF ========================================================*/
 

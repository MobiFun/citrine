/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_PPPP
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
|             primitives sent to the protocol stack adapter by the PPP
|             module.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)

#ifndef PSA_PPPP_C
#define PSA_PPPP_C
#endif /* of PSA_PPPP_C */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "psa.h"
#include "psa_l2r.h"
#include "psa_cc.h" /* for getting the NO_ENTRY definition */
#include "wap_aci.h"
#include "psa_ppp_w.h"
#include "cmh.h"
#include "cmh_ppp.h"


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPP                 |
|                                 ROUTINE : psa_wppp_establish_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the PPP_ESTABLISH_CNF primitive sent by PPP.

*/

GLOBAL void psa_wppp_establish_cnf(T_PPP_ESTABLISH_CNF *ppp_establish_cnf)
{
	ULONG ip_address;
	USHORT max_receive_unit;
  ULONG dns1, dns2 ;            /* Name server addresses. */
  TRACE_FUNCTION ("psa_wppp_establish_cnf()");
		
  if(pppShrdPrm.state EQ PPP_ESTABLISH) 
	{
    TRACE_EVENT("PPP_ESTABLISH_CNF received: PPP activated");
		pppShrdPrm.state = PPP_ESTABLISHED;
    
		ip_address = ppp_establish_cnf -> ip;
    TRACE_EVENT_P4 ("IP Address: %03u.%03u.%03u.%03u", (ip_address & 0xff000000) >> 24,
                                                       (ip_address & 0x00ff0000) >> 16,
                                                       (ip_address & 0x0000ff00) >>  8,
                                                       (ip_address & 0x000000ff)       );
    
	  max_receive_unit =	ppp_establish_cnf -> mru;
    TRACE_EVENT_P1 ("MRU       : %u", max_receive_unit);
    dns1 = ppp_establish_cnf->dns1 ;
    dns2 = ppp_establish_cnf->dns2 ;
    
		cmhPPP_Established(ip_address, max_receive_unit, dns1, dns2);
	}
	else
	{
		TRACE_EVENT("unexpected PPP_ESTABLISH_CNF primitive received");
	}
	
	PFREE(ppp_establish_cnf);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPP                  |
|                                 ROUTINE : psa_ppp_dti_connected_ind|
+--------------------------------------------------------------------+

  PURPOSE : processes the PPP_DTI_CONNECTED_IND primitive sent by PPP.

*/

GLOBAL void psa_wppp_dti_connected_ind(T_PPP_DTI_CONNECTED_IND 
                                            *ppp_dti_connected_ind)
{
  TRACE_FUNCTION("psa_wppp_dti_connected_ind()");
	
  if (ppp_dti_connected_ind->connected_direction EQ PPP_DTI_CONN_PEER)
  {
    dti_cntrl_entity_connected( peer_link_id, DTI_ENTITY_PPPC, DTI_OK );
  }
  else if (ppp_dti_connected_ind->connected_direction EQ PPP_DTI_CONN_PROT)
  {
    dti_cntrl_entity_connected( prot_link_id, DTI_ENTITY_PPPC, DTI_OK );
  }

	PFREE(ppp_dti_connected_ind);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPP                 |
|                                 ROUTINE : psa_wppp_terminate_ind  |
+-------------------------------------------------------------------+

  PURPOSE : processes the PPP_TERMINATE_IND primitive sent by PPP.

*/

GLOBAL void psa_wppp_terminate_ind(T_PPP_TERMINATE_IND *ppp_terminate_ind)
{   
 
 TRACE_FUNCTION("psa_wppp_terminate_ind()");

 if (Wap_Call OR pppShrdPrm.is_PPP_CALL )
  {                                                    
    
     /*
      *  PPP is terminated before - second termination indication received
      */

     if(pppShrdPrm.state EQ PPP_TERMINATED)
     {
       TRACE_EVENT("Unexpected: PPP_TERMINATE_IND received twice");
       PFREE(ppp_terminate_ind);
       return;
     }

     /*  
      *  Termination after established and by terminate
      */
     
     if((pppShrdPrm.state EQ PPP_ESTABLISHED) OR (pppShrdPrm.state EQ PPP_TERMINATE))
     {
       TRACE_EVENT("PPP_TERMINATE_IND, PPP terminate as requested");
       pppShrdPrm.state = PPP_TERMINATED;
       cmhPPP_Terminated();
       PFREE(ppp_terminate_ind);
       return;
     }
    
    /*
     *  PPP send a terminate indication by establish request
     */
    
     if(pppShrdPrm.state EQ PPP_ESTABLISH) 
     {
       TRACE_EVENT("PPP_TERMINATE_IND, PPP terminate by establish");
       pppShrdPrm.state = PPP_TERMINATED;
//       cmhIPA_Deconfigurated(); 
      cmhPPP_Terminated();
      PFREE(ppp_terminate_ind);
      return;
	 }
	 
	 else
	 {
	    pppShrdPrm.state = PPP_TERMINATED;
		TRACE_EVENT("Unexpected: PPP_TERMINATE_IND, PPP in wrong ACI PPP state");
	 }	
  }
  else
    TRACE_EVENT("Unexpected: Not a WAP Call");
 
 PFREE(ppp_terminate_ind);
  
}

#endif /* of FF_WAP or FF_PPP OR FF_GPF_TCPIP or FF_SAT_E */

#endif /* DTI */

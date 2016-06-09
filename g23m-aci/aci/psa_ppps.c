/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_PPPS
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)

#ifndef PSA_PPPS_C
#define PSA_PPPS_C
#endif  /* PSA_PPPS_C */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"

#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"

#include "wap_aci.h"
#include "psa_ppp_w.h"

#include "dcm_f.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPP                 |
|                                 ROUTINE : psaPPP_Establish        |
+-------------------------------------------------------------------+

  PURPOSE : This function sends an PPP_ESTABLISH_REQ.

*/

GLOBAL void psaPPP_Establish(T_DTI_CONN_LINK_ID link_id, UBYTE peer)
{
  T_login pLogin;

  TRACE_FUNCTION("psaPPP_Establish()");

  if( (is_gpf_tcpip_call() GPF_TCPIP_STATEMENT(AND (peer EQ DTI_ENTITY_TCPIP))) OR
       peer EQ DTI_ENTITY_IP OR peer EQ DTI_ENTITY_AAA OR peer EQ DTI_ENTITY_PSI)
  {
    prot_link_id  = link_id;
    prot_entity_id = peer;
  }
  else if (peer EQ DTI_ENTITY_L2R)
  {
    peer_link_id  = link_id;
  }
  else
  {
    TRACE_EVENT("wrong peer entity");
  }

#ifdef FF_WAP
  rAT_WAP_start_login();
#endif /* FF_WAP */

  if (pppShrdPrm.state NEQ PPP_ESTABLISH)
  {
    /* wait for the second call of psaPPP_Establish() */
    pppShrdPrm.state = PPP_ESTABLISH;
    return;
  }
  /* Set the unused bytes of login and password to zero */
  memset(&pLogin, 0x00, sizeof( T_login ));
  pLogin.name_len = strlen(pppShrdPrm.ppp_login);
  memcpy((char*)(pLogin.name),pppShrdPrm.ppp_login, pLogin.name_len);
  pLogin.password_len = strlen(pppShrdPrm.ppp_password);
  memcpy((char*)(pLogin.password),
         pppShrdPrm.ppp_password,
         pLogin.password_len);

  {
    PALLOC(ppp_establish_req, PPP_ESTABLISH_REQ);

    ppp_establish_req -> mode   = PPP_CLIENT;
    ppp_establish_req -> mru    = PPP_MRU_DEFAULT;
    ppp_establish_req -> ap     = pppShrdPrm.auth_prot;

    /* Copy user and password */
    memcpy( &(ppp_establish_req->login),&pLogin, sizeof( T_login ));

    ppp_establish_req -> accm   = PPP_ACCM_DEFAULT;
    ppp_establish_req -> rt     = PPP_RT_DEFAULT;
    ppp_establish_req -> mc     = PPP_MC_DEFAULT;
    ppp_establish_req -> mt     = PPP_MT_DEFAULT;
    ppp_establish_req -> mf     = PPP_MF_DEFAULT;
    ppp_establish_req -> ppp_hc = PPP_HC_OFF;
    ppp_establish_req -> ip     = PPP_IP_DYNAMIC;
    ppp_establish_req -> dns1   = PPP_DNS1_DYNAMIC;
    ppp_establish_req -> dns2   = PPP_DNS2_DYNAMIC;

    strcpy((char*)ppp_establish_req->peer_channel.peer_entity, 
            dti_entity_name[DTI_ENTITY_L2R].name);

    strcpy((char*)ppp_establish_req->protocol_channel.protocol_entity, 
            dti_entity_name[prot_entity_id].name);

    ppp_establish_req -> peer_link_id = peer_link_id;
    ppp_establish_req -> prot_link_id = prot_link_id;

    /* Direction for the Protocol Layer */
    ppp_establish_req -> peer_direction = DTI_CHANNEL_TO_LOWER_LAYER;

    /* Direction for the Peer Layer     */
    ppp_establish_req -> prot_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
    
    PSENDX(PPP,ppp_establish_req);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPP                 |
|                                 ROUTINE : psaPPP_Terminate        |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaPPP_Terminate(T_ACI_PPP_LOWER_LAYER ppp_lower_layer)
{

  TRACE_FUNCTION("psaPPP_Terminate()");

  if ((pppShrdPrm.state EQ PPP_TERMINATE) OR (pppShrdPrm.state EQ PPP_TERMINATED))
  {
    return;
  }

  pppShrdPrm.state = PPP_TERMINATE;
  {
    PALLOC(ppp_terminate_req, PPP_TERMINATE_REQ);
    switch (ppp_lower_layer)
    {
    case (UP):
      ppp_terminate_req -> lower_layer = PPP_LOWER_LAYER_UP;
      break;

    case (DWN):
      ppp_terminate_req -> lower_layer = PPP_LOWER_LAYER_DOWN;
      break;

    default:
      TRACE_EVENT("error: status of lower_layer undefined");
    }

    PSENDX(PPP,ppp_terminate_req);
  }
}
#endif /* of FF_WAP or FF_PPP OR FF_GPF_TCPIP or FF_SAT_E */
#endif /* DTI */

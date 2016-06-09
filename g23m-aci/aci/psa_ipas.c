/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_IPAS
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
#if defined (FF_WAP) || defined (FF_SAT_E)

#ifndef PSA_IPAS_C
#define PSA_IPAS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "psa.h"
//#include "psa_l2r.h"
#include "psa_cc.h"
#include "aci_fd.h"
#include "cmh.h"
#include "dti_conn_mng.h"
//#include "cmh_l2r.h"

#include "wap_aci.h"
#include "psa_tcpip.h"

#ifdef GPRS
#include "gaci.h"
#include "cmh_sm.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psaIPA_Config                |
+-------------------------------------------------------------------+

  PURPOSE : Configurate or disconfigurate IPA.

*/

GLOBAL void psaIPA_Config(ULONG ip, USHORT max_trans_unit, T_ACI_IPA_DIRC dirc
                      /*T_ACI_AT_CMD at_cmd */)
{
                  /* dirc = IPA_CONN if IPA is to be activated
                     dirc = IPA_DSC if IPA i to be deactivated */

  TRACE_FUNCTION("psaIPA_Config()") ;

  switch( dirc )
  {

  case ( IPA_CONN ):
    wap_state = IPA_Configuration;
    {
      PALLOC(ipa_config_req,IPA_CONFIG_REQ);
      ipa_config_req -> ip = ip;
      ipa_config_req->peer_ip = 0;
      ipa_config_req->cmd = IPA_CONFIG_UP;
      ipa_config_req -> mtu = max_trans_unit;

      PSENDX(IP,ipa_config_req);
    }
    break;

  case ( IPA_DSC ):
    wap_state = IPA_Deconfiguration;
    {
      PALLOC(ipa_config_req,IPA_CONFIG_REQ);
      ipa_config_req -> ip = 0;
      ipa_config_req->peer_ip = 0;
      ipa_config_req->cmd = IPA_CONFIG_DOWN;
      ipa_config_req -> mtu = 0;

      PSENDX(IP,ipa_config_req);
    }
    break;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPAS                     |
|                            ROUTINE : psaIPA_Dti_Req               |
+-------------------------------------------------------------------+

  PURPOSE : connect IP

*/

GLOBAL void psaIPA_Dti_Req(ULONG link_id, UBYTE peer_to_connect_to, UBYTE dti_conn)
{

  PALLOC( ipa_dti_req, IPA_DTI_REQ);

  

#ifdef GPRS
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
    
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer]);

  if( p_pdp_context_node )
  {
    p_pdp_context_node->internal_data.link_id = link_id;
  }

  /* This is probably not the right place to populate the link id since
     IPA dti req might be sent for CSD call as well ! Need to investigate
     as separate issue.
  */
#endif /* GPRS */

  TRACE_FUNCTION("psaIPA_Dti_Req");

  ipa_dti_req -> dti_conn  = dti_conn;
  ipa_dti_req -> link_id   = link_id;

  if (dti_conn EQ IPA_CONNECT_DTI)
  {
    switch( peer_to_connect_to )
    {
    case( DTI_ENTITY_UDP ):
      ipa_dti_req -> dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      strcpy((char*)ipa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_UDP].name);
      break;
#ifdef GPRS
    case( DTI_ENTITY_SNDCP ):
      ipa_dti_req -> dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;
      strcpy((char*)ipa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_SNDCP].name);
      break;
#endif
    case( DTI_ENTITY_PPPC ):
      ipa_dti_req -> dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;
      strcpy((char*)ipa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_PPPC].name);
      break;
    default:
      TRACE_EVENT("unknown conn_peer_Id: IPA_DTI_REQ not sent");
      PFREE(ipa_dti_req);
      return;
    }
  }

  PSENDX( IP, ipa_dti_req );
}


#endif /* WAP || SAT E */

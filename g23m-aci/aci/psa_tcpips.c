/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_TCPS
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
#ifndef PSA_TCPS_C
#define PSA_TCPS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_ss.h"
#include "aoc.h"

#if !defined (MFW)
#include "aci_io.h"
#endif

#include "wap_aci.h"

#include "psa_tcpip.h"
#include "psa_sim.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_TCPS                    |
|                            ROUTINE : psaTCPIP_Config               |
+-------------------------------------------------------------------+

  PURPOSE : setup TCPIP

*/

GLOBAL void psaTCPIP_Config (ULONG ipaddr, ULONG dns1, ULONG dns2, UBYTE dirc)
{
  TRACE_FUNCTION("psaTCPIP_Config()");
  switch( dirc )
  {

  case ( TCPIP_IFCONFIG_UP ):
    wap_state = TCPIP_Configuration;
    {
      PALLOC(tcpip_config_req, TCPIP_IFCONFIG_REQ);
      tcpip_config_req->if_up = TCPIP_IFCONFIG_UP;
      tcpip_config_req->mtu_size = 1500; 
      tcpip_config_req->ipaddr = ipaddr;
      tcpip_config_req->dnsaddr1 = dns1; 
      tcpip_config_req->dnsaddr2 = dns2; 
      PSENDX(TCPIP, tcpip_config_req);
    }
    break;

  case ( TCPIP_IFCONFIG_DOWN ):
    wap_state = TCPIP_Deconfiguration;
    {
      PALLOC(tcpip_config_req, TCPIP_IFCONFIG_REQ);
      tcpip_config_req->if_up = TCPIP_IFCONFIG_DOWN;
      tcpip_config_req->mtu_size = 0;
      tcpip_config_req->ipaddr = 0;
      tcpip_config_req->dnsaddr1 = 0; 
      tcpip_config_req->dnsaddr2 = 0; 
      PSENDX(TCPIP, tcpip_config_req);
    }
    break;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_TCPS                    |
|                            ROUTINE : psaTCPIP_Dti_Req              |
+-------------------------------------------------------------------+

  PURPOSE : connect TCPIP 

*/

GLOBAL void psaTCPIP_Dti_Req(T_DTI_CONN_LINK_ID dti_id, UBYTE peer_to_connect_to, UBYTE dti_conn)
{
  PALLOC( tcpip_dti_req, TCPIP_DTI_REQ);
  
  TRACE_FUNCTION("psaTCPIP_Dti_Req");

  tcpip_dti_req -> dti_conn  = dti_conn;
  tcpip_dti_req -> link_id   = dti_id;

  if (dti_conn EQ TCPIP_CONNECT_DTI)
  {
    if (dti_cntrl_set_conn_parms(dti_id, DTI_ENTITY_TCPIP, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
    {
        return;
    }
  }
  switch( peer_to_connect_to )
  {
    case( DTI_ENTITY_PPPC ):
      tcpip_dti_req -> dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;
      tcpip_dti_req->entity_name = (U32) dti_entity_name[DTI_ENTITY_PPPC].name ;
      TRACE_EVENT_P2("tcpip_dti_req->entity_name = %s <- %s",
                     (U8 *) tcpip_dti_req->entity_name,
                     dti_entity_name[DTI_ENTITY_PPPC].name) ;
      break;
#ifdef GPRS
    case( DTI_ENTITY_SNDCP ):
      tcpip_dti_req -> dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;
      tcpip_dti_req->entity_name = (U32) dti_entity_name[DTI_ENTITY_SNDCP].name ;
      TRACE_EVENT_P2("tcpip_dti_req->entity_name = %s <- %s",
                     (U8 *) tcpip_dti_req->entity_name,
                     dti_entity_name[DTI_ENTITY_SNDCP].name) ;
      break;
#endif
    case( DTI_ENTITY_WAP ):
      tcpip_dti_req -> dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      strcpy((char*)tcpip_dti_req->entity_name, dti_entity_name[DTI_ENTITY_WAP].name);
      break;

    case( DTI_ENTITY_SIM ):
      tcpip_dti_req -> dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      strcpy((char*)tcpip_dti_req->entity_name, dti_entity_name[DTI_ENTITY_SIM].name);
      break;
      
    default:
      TRACE_EVENT("Error: Unknown conn_peer_Id: TCPIP_DTI_REQ not sent");
      PFREE(tcpip_dti_req);
      return;
    }

  PSENDX( TCPIP, tcpip_dti_req );
}


GLOBAL void psaTCPIP_Initialize_Req(void)
{
  TRACE_FUNCTION("psaTCPIP_Initialize_Req()");
  {
    PALLOC(tcpip_initialize_req,TCPIP_INITIALIZE_REQ);
    PSEND(hCommTCPIP,tcpip_initialize_req);
  }
  
}


GLOBAL void psaTCPIP_Shutdown_Req(void)
{
  TRACE_FUNCTION("psaTCPIP_Shutdown_Req()");
  {
    PALLOC(tcpip_shutdown_req ,TCPIP_SHUTDOWN_REQ);
    PSEND(hCommTCPIP,tcpip_shutdown_req);
  } 
}

/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UDPAS
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

#ifndef PSA_UDPAS_C
#define PSA_UDPAS_C
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
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UDPAS                    |
|                            ROUTINE : psaUDPA_Config               |
+-------------------------------------------------------------------+

  PURPOSE : setup UDPA

*/

GLOBAL void psaUDPA_Config ( UBYTE dirc )
{

  switch( dirc )
  {

  case ( UDPA_CONFIG_UP ):
    wap_state = UDPA_Configuration;
    {
      PALLOC(udpa_config_req,UDPA_CONFIG_REQ);
      udpa_config_req->cmd = UDPA_CONFIG_UP;
      PSENDX(UDP,udpa_config_req);
    }
    break;

  case ( UDPA_CONFIG_DOWN ):
    wap_state = UDPA_Deconfiguration;
    {
      PALLOC(udpa_config_req,UDPA_CONFIG_REQ);
      udpa_config_req->cmd = UDPA_CONFIG_DOWN;
      PSENDX(UDP,udpa_config_req);
    }
    break;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UDPAS                    |
|                            ROUTINE : psaUDPA_Dti_Req              |
+-------------------------------------------------------------------+

  PURPOSE : connect UDP

*/

GLOBAL void psaUDPA_Dti_Req(T_DTI_CONN_LINK_ID dti_id, UBYTE peer_to_connect_to, UBYTE dti_conn)
{

  PALLOC( udpa_dti_req, UDPA_DTI_REQ);


  TRACE_FUNCTION("psaUDPA_Dti_Req");


  udpa_dti_req -> dti_conn  = dti_conn;
  udpa_dti_req -> link_id   = dti_id;

  if (dti_conn EQ UDPA_CONNECT_DTI)
  {

    switch( peer_to_connect_to )
    {
    case( DTI_ENTITY_IP ):
      udpa_dti_req -> dti_direction = DTI_CHANNEL_TO_LOWER_LAYER;
      strcpy((char*)udpa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_IP].name);
      break;
    case( DTI_ENTITY_WAP ):
      udpa_dti_req -> dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      strcpy((char*)udpa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_WAP].name);

      if (dti_cntrl_set_conn_parms(dti_id, DTI_ENTITY_UDP, DTI_INSTANCE_NOTPRESENT, DTI_SUB_NO_NOTPRESENT) EQ FALSE)
      {
        return;
      }

      break;
#ifdef FF_SAT_E      
    case( DTI_ENTITY_SIM ):
      udpa_dti_req -> dti_direction = DTI_CHANNEL_TO_HIGHER_LAYER;
      strcpy((char*)udpa_dti_req->entity_name, dti_entity_name[DTI_ENTITY_SIM].name);
      break;
#endif /* FF_SAT_E */      
    default:
      TRACE_EVENT("unknown conn_peer_Id: UDPA_DTI_REQ not sent");
      PFREE(udpa_dti_req);
      return;
    }
  }

  PSENDX( UDP, udpa_dti_req );

}



#endif /* of FF_WAP or SAT E */

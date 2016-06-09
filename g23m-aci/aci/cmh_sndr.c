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
|             for the responses of the protocol stack adapter for SNDCP.
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_SNDR_C
#define CMH_SNDR_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/

#include "aci_cmh.h"

#include "psa.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci_cmh.h"
#include "gaci.h"

#include "cmh.h"
#include "cmh_snd.h"
#include "cmh_sm.h"

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : UMTS             MODULE  : CMH_SNDR                     |
| STATE   : finished         ROUTINE : cmhUPM_Counted               |
+-------------------------------------------------------------------+

  PURPOSE : 

*/
GLOBAL void cmhUPM_Counted( UBYTE nsapi,
                              ULONG octets_uplink,
                              ULONG octets_downlink,
                              ULONG packets_uplink,
                              ULONG packets_downlink )
{
  TRACE_FUNCTION ("cmhUPM_Counted()");

  R_AT( RAT_SNCNT, (T_ACI_CMD_SRC)sndcpShrdPrm.srcId ) ( NSAPI_TO_CID(nsapi),
                                          octets_uplink, 
                                          octets_downlink,
                                          packets_uplink,
                                          packets_downlink );

  R_AT( RAT_OK, (T_ACI_CMD_SRC)sndcpShrdPrm.srcId ) ( AT_CMD_SNCNT );
}


#if defined(FF_PKTIO) OR defined(FF_TCP_IP) OR defined (FF_PSI)
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)                             MODULE  : CMH_SNDR                             |
| STATE   : code                                                 ROUTINE : PKTIO_SNDCP_connect_dti_cb |
+---------------------------------------------------------------------------+

  PURPOSE : Callback for connection between PKTIO and SNDCP.

*/
#ifdef GPRS

GLOBAL BOOL PKTIO_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  U8 cid;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("PKTIO_SNDCP_connect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      cid = (U8)gaci_get_cid_over_dti_id(dti_id);
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( ! p_pdp_context_node )
      {
        TRACE_ERROR( "ERROR: PDP context not found, in function psa_smreg_pdp_activate_cnf" );
        return FALSE;
      }

      if( INVALID_CID   NEQ cid                                AND
          PDP_CONTEXT_STATE_ACTIVATING  EQ get_state_over_cid(cid)            AND
          ( DTI_ENTITY_PKTIO EQ p_pdp_context_node->internal_data.entity_id OR
            DTI_ENTITY_PSI   EQ p_pdp_context_node->internal_data.entity_id OR
            DTI_ENTITY_AAA   EQ p_pdp_context_node->internal_data.entity_id     ) )
      {
        cmhSM_deactivateAContext(CMD_SRC_NONE, cid);
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEACTIVATE_NORMAL);
      }
      break;
    case DTI_CONN_STATE_DISCONNECTED:
    case DTI_CONN_STATE_CONNECTING:
    case DTI_CONN_STATE_CONNECTED:
      break ;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect SNDCP */
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("PKTIO_SNDCP_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* FF_PKTIO OR FF_TCP_IP  OR FF_PSI*/

#endif /* GPRS */

#ifdef CO_UDP_IP
/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_SNDR                 |
| STATE   : code                        ROUTINE : IP_SNDCP_connect_dti_cb  |
+--------------------------------------------------------------------------+

  PURPOSE : Callback for connection between IP Server and SNDCP.

*/

GLOBAL BOOL IP_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("IP_SNDCP_connect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
    case DTI_CONN_STATE_CONNECTED:
    case DTI_CONN_STATE_CONNECTING:
      break;
    case DTI_CONN_STATE_DISCONNECTED:
      /* Deactivate the context if necessary. */
      /*cmhUPM_Disable( dti_id );*/
      break;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect SNDCP */
      TRACE_EVENT("IP_SNDCP_connect_dti_cb connection not possible: disconnect SNDCP");
      dti_cntrl_close_dpath_from_dti_id( dti_id);
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("IP_SNDCP_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* CO_UDP_IP */


#ifdef FF_GPF_TCPIP
GLOBAL BOOL TCPIP_SNDCP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("TCPIP_SNDCP_connect_dti_cb");
  TRACE_EVENT_P1("TCPIP_SNDCP_connect_dti_cb() result = %d",result_type);

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb in DTI_CONN_STATE_DISCONNECTING") ;
      break ;

    case DTI_CONN_STATE_CONNECTING:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb in DTI_CONN_STATE_CONNECTING") ;
      break;
         
    case DTI_CONN_STATE_DISCONNECTED:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb in DTI_CONN_STATE_DISCONNECTED") ;
      dti_cntrl_erase_entry(dti_id);
      {
        T_DCM_STATUS_IND_MSG msg;
        msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
        dcm_send_message(msg, DCM_SUB_WAIT_CGACT_CNF);
      }
      break;
         
    case DTI_CONN_STATE_CONNECTED:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb in DTI_CONN_STATE_CONNECTED");
      {
        T_DCM_STATUS_IND_MSG msg;
        msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
        dcm_send_message(msg, DCM_SUB_WAIT_CGACT_CNF);
      }
      break;
         
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect SNDCP */
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb connection not possible: disconnect SNDCP");
      dti_cntrl_close_dpath_from_dti_id( dti_id);
      break;

    case DTI_CONN_STATE_UNKNOWN:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb in DTI_CONN_STATE_UNKNOWN") ;
      break;
         
    default:
      TRACE_EVENT("TCPIP_SNDCP_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}
#endif /* FF_GPF_TCPIP */

#endif /* GPRS && DTI*/

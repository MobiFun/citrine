/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_TCPP
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
|             primitives sent to the protocol stack adapter by the TCPIP
|             module.
+-----------------------------------------------------------------------------
*/

#ifndef PSA_TCPP_C
#define PSA_TCPP_C
#endif /* of PSA_TCPP_C */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_fd.h"
#include "aci.h"
#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"
#include "psa_tcpip.h"
#include "psa_cc.h"

#include "wap_aci.h"
#include "psa_sat.h"


#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#include "dcm_f.h"


/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TCP                |
|                                 ROUTINE : psa_tcpip_dti_cnf        |
+-------------------------------------------------------------------+

  PURPOSE : processes the TCPIP_DTI_CNF primitive sent by TCPIP.
*/
GLOBAL void psa_tcpip_dti_cnf(T_TCPIP_DTI_CNF *tcpip_dti_cnf)
{
  TRACE_FUNCTION ("psa_tcpip_dti_cnf()");

  switch( tcpip_dti_cnf->dti_conn )
  {
    case(TCPIP_CONNECT_DTI):
      dti_cntrl_entity_connected( tcpip_dti_cnf->link_id, DTI_ENTITY_TCPIP, DTI_OK );
      break;

    case(TCPIP_DISCONNECT_DTI):
      dti_cntrl_entity_disconnected( tcpip_dti_cnf->link_id, DTI_ENTITY_TCPIP );
      break;
  }
  PFREE(tcpip_dti_cnf);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_TCP                 |
|                                 ROUTINE : psa_tcpip_ifconfig_cnf  |
+-------------------------------------------------------------------+

  PURPOSE : processes the TCPIP_CONFIG_CNF primitive sent by TCPIP.

*/
GLOBAL void psa_tcpip_ifconfig_cnf(T_TCPIP_IFCONFIG_CNF *tcpip_ifconfig_cnf)
{
  TRACE_FUNCTION ("psa_tcpip_ifconfig_cnf()");

  if(tcpip_ifconfig_cnf->result == TCPIP_RESULT_OK)
  {
    if (wap_state EQ TCPIP_Configuration)
    {
      wap_state = TCPIP_Configurated;
      cmhTCPIP_Configurated();
    }
    else if (wap_state EQ TCPIP_Deconfiguration)
    {
      wap_state = TCPIP_Deconfigurated;
      /* For TCPIP call also we need to shutdown the TCP before clearing the 
         DTI connection */
      if( (tcpipShrdPrm.connection_type EQ TCPIP_CONNECTION_TYPE_CSD_WAP) OR 
          ((is_gpf_tcpip_call()) AND 
          (tcpipShrdPrm.connection_type EQ TCPIP_CONNECTION_TYPE_GPRS_WAP)))

      {
        wap_state = TCPIP_Deactivation;
        psaTCPIP_config_dispatch();
      }
      else
        cmhTCPIP_Deconfigurated();
    }
  }
  else
  {
    TRACE_EVENT("unexpected TCPIP_IFCONFIG_CNF primitive received");
  }

  PFREE(tcpip_ifconfig_cnf);
}



GLOBAL void psa_tcpip_shutdown_cnf(T_TCPIP_SHUTDOWN_CNF *tcpip_shutdown_cnf)
{
  TRACE_FUNCTION ("psa_tcpip_shutdown_cnf()");

  if(tcpip_shutdown_cnf->result != TCPIP_RESULT_OK)
  {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    dcm_send_message(msg, DCM_SUB_WAIT_CGDEACT_CNF);

    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    dcm_send_message(msg, DCM_SUB_WAIT_SATH_CNF);
  }
  else
  {
    wap_state = TCPIP_Deactivated;
    psaTCPIP_config_dispatch();
  }
  reset_gpf_tcpip_call();
  PFREE(tcpip_shutdown_cnf);
}



GLOBAL void psa_tcpip_initialize_cnf(T_TCPIP_INITIALIZE_CNF *tcpip_initialize_cnf)
{
  TRACE_FUNCTION ("psa_tcpip_initialize_cnf()");

  if(tcpip_initialize_cnf->result != TCPIP_RESULT_OK)
  {
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    dcm_send_message(msg, DCM_SUB_WAIT_CGACT_CNF);

    msg.hdr.msg_id = DCM_NEXT_CMD_STOP_MSG;
    dcm_send_message(msg, DCM_SUB_WAIT_SATDN_CNF);
  }
  else
  {
    wap_state = TCPIP_Activation;
    psaTCPIP_config_dispatch();
  }
  PFREE(tcpip_initialize_cnf);
}

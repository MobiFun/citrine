/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_TCPR
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

#ifndef CMH_TCPR_C
#define CMH_TCPR_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/

#include "aci_cmh.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"

#include "psa.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "dti_conn_mng.h"
#include "Dti_cntrl_mng.h"

#include "wap_aci.h"

#include "psa_tcpip.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN ULONG ipAddress;
/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TCPR               |
|                                 ROUTINE : cmhTCPIP_Configurated    |
+-------------------------------------------------------------------+

  PURPOSE : TCPIP configurated
*/

GLOBAL T_ACI_RETURN cmhTCPIP_Configurated ( void )
{
  TRACE_FUNCTION ("cmhTCPIP_Configurated()");

#ifdef FF_WAP
  if (Wap_Call)
  {
    if ( tcpipShrdPrm.connection_type EQ TCPIP_CONNECTION_TYPE_CSD_WAP )
    {
      /* This is only BMI specific code */
      rAT_WAP_PPP_connected(wapId,ipAddress);
    }
    else
    {
      TRACE_EVENT_P1 ("IP: %s", tcpipShrdPrm.ipaddr);
      /* This is only BMI specific code and only evaluates the ipaddr but has nothing to do
         with PPP connection since this is the GPRS path of the above if(TCPIP_CONNECTION_TYPE_CSD_WAP)*/
      rAT_WAP_PPP_connected(wapId, psaTCPIP_bytes2ipv4addr(tcpipShrdPrm.ipaddr));
    }
  }
#endif
  psaTCPIP_config_dispatch ();
  return AT_CMPL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TCPR               |
|                                 ROUTINE : cmhTCPIP_Configurated    |
+-------------------------------------------------------------------+
  PURPOSE : TCPIP deconfigurated
*/
GLOBAL T_ACI_RETURN cmhTCPIP_Deconfigurated ( void )
{
  TRACE_FUNCTION ("cmhTCPIP_Deconfigurated()");

  cmhTCPIP_Deactivate(tcpipShrdPrm.src_id);
  return AT_CMPL;
}


/******************************************************************************/
/* This function returns the DTI connection state as a string; it is only 
 * relevant for tracing
 */
LOCAL char* dti_conn_state_string(T_DTI_CONN_STATE state)
{
  switch (state)
  {
    case DTI_CONN_STATE_UNKNOWN:       return "DTI_CONN_STATE_UNKNOWN";
    case DTI_CONN_STATE_CONNECTING:    return "DTI_CONN_STATE_CONNECTING";
    case DTI_CONN_STATE_CONNECTED:     return "DTI_CONN_STATE_CONNECTED";
    case DTI_CONN_STATE_DISCONNECTING: return "DTI_CONN_STATE_DISCONNECTING";
    case DTI_CONN_STATE_DISCONNECTED:  return "DTI_CONN_STATE_DISCONNECTED";
    case DTI_CONN_STATE_ERROR:         return "DTI_CONN_STATE_ERROR";
    default: return "NOT_DEFINED_DTI_CONN_STATE";

  }
}


/******************************************************************************/
GLOBAL BOOL TCPIP_connect_dti_cb (UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("TCPIP_connect_dti_cb()");
  TRACE_EVENT_P1("Result: %s", dti_conn_state_string(result_type));

  switch( result_type)
  {
    case(DTI_CONN_STATE_CONNECTING):
    case(DTI_CONN_STATE_CONNECTED):
    case(DTI_CONN_STATE_DISCONNECTING):
      break;

    case(DTI_CONN_STATE_DISCONNECTED):
      dti_cntrl_clear_conn_parms(dti_id);
      break;

    case(DTI_CONN_STATE_ERROR):
      /* connection not possible: disconnect */
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      TRACE_EVENT("TCPIP_connect_dti_cb(): call with not awaited value");
      break;
  }
  return TRUE;
}


/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_IPAR
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

#ifndef CMH_IPAR_C
#define CMH_IPAR_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
  #include "aci_fd.h"
#endif

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci_io.h"

#include "psa.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"

#include "wap_aci.h"

#include "psa_tcpip.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

EXTERN SHORT cmhCC_IPA_Configurated (void);
EXTERN SHORT cmhCC_IPA_Deconfigurated (void);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_IPAR                |
|                                 ROUTINE : cmhIPA_Configurated     |
+-------------------------------------------------------------------+

  PURPOSE : IPA configurated 
*/

GLOBAL T_ACI_RETURN cmhIPA_Configurated ( void )
{

  TRACE_FUNCTION ("cmhIPA_Configurated()");

  cmhCC_IPA_Configurated();
  
  return (AT_CMPL);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_IPAR                |
|                                 ROUTINE : cmhIPA_Deconfigurated   |
+-------------------------------------------------------------------+

  PURPOSE : IPA deconfigurated 
*/

GLOBAL T_ACI_RETURN cmhIPA_Deconfigurated ( void )
{

  TRACE_FUNCTION ("cmhIPA_Deconfigurated()");

  cmhCC_IPA_Deconfigurated();
  
  return (AT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : CMH_IPAR              |
| STATE   : code                     ROUTINE : IP_UDP_connect_dti_cb |
+--------------------------------------------------------------------+

  PURPOSE : Callback for connection between IP and UDP.

*/

GLOBAL BOOL IP_UDP_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("IP_UDP_connect_dti_cb()");

  switch( result_type)
  {
    case(DTI_CONN_STATE_DISCONNECTING):
      break;

    case(DTI_CONN_STATE_CONNECTED):
      if (wap_state EQ UDPA_Activation)
      {
        wap_state = IPA_Activated;
        psaUDPIP_config_dispatch();
      }
      else
      {
        TRACE_EVENT_P1("IP_UDP_connect_dti_cb(): DTI_CONN_STATE_CONNECTED, but wrong WAP STATE: %d ",wap_state );
      }
      break;

    case(DTI_CONN_STATE_DISCONNECTED):
      if (wap_state EQ UDPA_Deactivation)
      {
        /* wap_state is set to IPA_Deactivated in psaUDPIP_config_dispatch() */
        wap_state = IPA_Deactivation;
        psaUDPIP_config_dispatch();
        dti_cntrl_erase_entry (dti_id);
        dti_cntrl_clear_conn_parms(dti_id);
        if (wap_dti_id EQ dti_id)
        {
          wap_dti_id = DTI_DTI_ID_NOTPRESENT;         
          TRACE_EVENT("IP_UDP_connect_dti_cb(): DTI_CONN_STATE_DISCONNECTED and wap_dti_id resetted");        
        }
      }
      else
      {
        TRACE_EVENT_P1("IP_UDP_connect_dti_cb(): DTI_CONN_STATE_DISCONNECTED, but wrong WAP STATE: %d ",wap_state );
      }
      break;

    case(DTI_CONN_STATE_ERROR):
      /* connection not possible: disconnect */
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;

    default:
      TRACE_EVENT("IP_UDP_connect_dti_cb(): call with not awaited value");
      break;
  }
  return TRUE;
}

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_TCPS
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
|  Purpose :  This module provides the set functions related to the 
|             protocol stack adapter for TCPIP.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_TCPS_C
#define CMH_TCPS_C
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

#include "wap_aci.h"

#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "cmh_ipa.h"

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "psa_sim.h"
#endif

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif /* FF_GPF_TCPIP */

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TCPS               |
|                                 ROUTINE : cmhTCPIP_Activate        |
+-------------------------------------------------------------------+

  PURPOSE : activates DTI connection TCPIP-IP

*/

#if 0
T_ACI_RETURN cmhTCPIP_Activate (T_ACI_CMD_SRC srcId, SHORT cId)
{
  T_BEARER_TYPE current_bear_type = NO_BEARER_TYPE;
  TRACE_FUNCTION ("cmhTCPIP_Activate()");

  if (Wap_Call EQ TRUE)
  {
    wap_state = TCPIP_Activation;

#if defined (SIM_TOOLKIT) AND defined(FF_SAT_E)
    /* SAT Class E */
    if (satShrdPrm.opchStat EQ OPCH_EST_REQ)
    {
      T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TCPIP, DTI_ENTITY_IP};

      /* create a SAT class E DTI ID if not present */
      if (simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT)
      {
        simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti (DTI_DTI_ID_NOTPRESENT);
        TRACE_EVENT_P1 ("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
      }

      dti_cntrl_est_dpath (
        simShrdPrm.sat_class_e_dti_id,
        entity_list,
        2,
        SPLIT,
        TCPIP_connect_dti_cb
      );
    }
    else
#endif
    {
      /* normal WAP Call */
      // TODO: check this function: get_current_bearer_type()
      //current_bear_type = get_current_bearer_type();

      if(current_bear_type == GPRS_BEARER_TYPE)
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TCPIP, DTI_ENTITY_SNDCP};
      /* create a WAP DTI if not present */
      if (wap_dti_id EQ DTI_DTI_ID_NOTPRESENT)
      {
        wap_dti_id = dti_cntrl_new_dti (DTI_DTI_ID_NOTPRESENT);
        TRACE_EVENT_P1 ("wap_dti_id = %d", wap_dti_id);
      }
      dti_cntrl_est_dpath(wap_dti_id,
                          entity_list,
                          2,
                          SPLIT,
                          TCPIP_connect_dti_cb);
      }
      else if(current_bear_type == GSM_BEARER_TYPE)
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TCPIP, DTI_ENTITY_PPPC , DTI_ENTITY_L2R };
        
        /* create a WAP DTI if not present */
        if (wap_dti_id EQ DTI_DTI_ID_NOTPRESENT)
        {
          wap_dti_id = dti_cntrl_new_dti (DTI_DTI_ID_NOTPRESENT);
          TRACE_EVENT_P1 ("wap_dti_id = %d", wap_dti_id);
        }
        dti_cntrl_est_dpath(wap_dti_id,
                            entity_list,
                            3,
                            SPLIT,
                            TCPIP_connect_dti_cb);
      }
      else
      {
        TRACE_EVENT ("Ivalid Bearer Type is not set,so don't establish a data path");
        return AT_FAIL;    
      }
    }
  }
  else
  {
    TRACE_EVENT ("Wap_Call is not set so don't establish a data path");
    return AT_FAIL;
  }

  return AT_EXCT;
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_TCPS               |
|                                 ROUTINE : cmhTCPIP_Deactivate      |
+-------------------------------------------------------------------+

  PURPOSE : deactivates DTI connection TCPIP-IP

*/

T_ACI_RETURN cmhTCPIP_Deactivate (UBYTE src_id)
{
  TRACE_FUNCTION ("cmhTCPIP_Deactivate()");

  wap_state = TCPIP_Deactivation;

  if ( wap_dti_id NEQ DTI_DTI_ID_NOTPRESENT )
  {
    dti_cntrl_close_dpath_from_dti_id (wap_dti_id);
  }
  else if (simShrdPrm.sat_class_e_dti_id NEQ DTI_DTI_ID_NOTPRESENT)
  {
    dti_cntrl_close_dpath_from_dti_id (simShrdPrm.sat_class_e_dti_id);
  }
  else
  {
    TRACE_EVENT ("No dti id available");
    return AT_FAIL;
  } 

  return AT_EXCT;
}


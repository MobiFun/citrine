/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_UDPAS
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
|             protocol stack adapter for UDPA.
+----------------------------------------------------------------------------- 
*/ 

#ifdef DTI

#ifndef CMH_UDPAS_C
#define CMH_UDPAS_C
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
#include "cmh_sat.h" 
#endif

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UDPAS               |
|                                 ROUTINE : cmhUDPA_Activate        |
+-------------------------------------------------------------------+

  PURPOSE : activates DTI connection UDP-IP

*/

GLOBAL T_ACI_RETURN cmhUDPA_Activate ( T_ACI_CMD_SRC srcId, 
                                       SHORT cId )
{
  TRACE_FUNCTION ("cmhUDPA_Activate()");

  if( Wap_Call EQ TRUE )
  {
    wap_state = UDPA_Activation;
#ifdef SIM_TOOLKIT 
#ifdef FF_SAT_E
    /* SAT Class E */
    if ( satShrdPrm.opchStat EQ OPCH_EST_REQ )
    {
      /* establish connections: SIM-UDP and UDP-IP */ 
      T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_SIM, DTI_ENTITY_UDP, DTI_ENTITY_IP};
      
      /* prepare SAT environment for SIM - UDP connection */
      cmhSAT_OpChnSIMCnctReq( (UBYTE) DTI_ENTITY_UDP );


      /* create a SAT class E DTI ID if not present */      
      if ( simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT )
      {
        simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
        TRACE_EVENT_P1("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
      }
      
      dti_cntrl_est_dpath ( simShrdPrm.sat_class_e_dti_id,
                            entity_list,
                            3,
                            SPLIT,
                            IP_UDP_connect_dti_cb);      
    }
    else
#endif /* FF_SAT_E */
#endif /* SIM_TOOLKIT */
    {
      /* normal WAP Call */
      T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_WAP, DTI_ENTITY_UDP, DTI_ENTITY_IP};

      /* create a WAP DTI if not present */
      if ( wap_dti_id EQ DTI_DTI_ID_NOTPRESENT )
      {
        wap_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
        TRACE_EVENT_P1("wap_dti_id = %d", wap_dti_id);
      }

      dti_cntrl_est_dpath ( wap_dti_id,
                            entity_list,
                            3,
                            SPLIT,
                            IP_UDP_connect_dti_cb);
    }
  }
  else
  {
    TRACE_EVENT("Wap_Call is not set so don't establish a data path");
    return ( AT_FAIL );
  }

  return( AT_EXCT );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_UDPAS               |
|                                 ROUTINE : cmhUDPA_Deactivate      |
+-------------------------------------------------------------------+

  PURPOSE : deactivates DTI connection UDP-IP

*/

GLOBAL T_ACI_RETURN cmhUDPA_Deactivate ( UBYTE src_id)
{
  TRACE_FUNCTION ("cmhUDPA_Deactivate()");

  wap_state = UDPA_Deactivation;

  if ( wap_dti_id NEQ DTI_DTI_ID_NOTPRESENT )
  {
    dti_cntrl_close_dpath_from_dti_id(wap_dti_id);
  }  
#ifdef FF_SAT_E   
  else if (simShrdPrm.sat_class_e_dti_id NEQ DTI_DTI_ID_NOTPRESENT)
  {
    dti_cntrl_close_dpath_from_dti_id(simShrdPrm.sat_class_e_dti_id);
  }
#endif 
  else
  {
    TRACE_EVENT("No dti id available");
    return( AT_FAIL );
  } 

  return( AT_EXCT );
}

#endif /* DTI */

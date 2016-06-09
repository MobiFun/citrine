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
|  Purpose :  This module defines the functions used by the command
|             handler for GPRS Point-to-Point Protocol ( PPP ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_PPPF_C
#define CMH_PPPF_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "pcm.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "aci.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gppp.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_gppp.h"
#include "cmh_sm.h"
/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/



GLOBAL void cmhGPPPS_DTIconnected( UBYTE dti_direction )
{
  T_DTI_CONN_LINK_ID     link_id             = DTI_LINK_ID_NOTPRESENT;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhGPPPS_DTIconnected()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[0] );
  if( p_pdp_context_node )
  {

/*
 *-------------------------------------------------------------------
 * inform DTI Manager
 *-------------------------------------------------------------------
 */  
    switch ( dti_direction )
    {
      case NAS_HOME:
        link_id = p_pdp_context_node->internal_data.link_id_uart;
        break;
      case NAS_NEIGHBOR:
        link_id = p_pdp_context_node->internal_data.link_id;
        break;
      default:
        return;
    }
   
    dti_cntrl_entity_connected( link_id, DTI_ENTITY_PPPS, DTI_OK );
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in fundtion cmhGPPPS_DTIconnected");
  }

}


#endif  /* GPRS */
/*==== EOF ========================================================*/

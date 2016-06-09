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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by SNDCP.
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_SNDP_C
#define PSA_SNDP_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "cmh.h"
#include "cmh_sm.h"
#include "cmh_gppp.h"

#if defined (FF_WAP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "wap_aci.h"
#include "psa_tcpip.h"
#endif /* defined (FF_WAP) || defined (FF_GPF_TCPIP) */

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#endif
#include "dcm_f.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SND           |
| STATE   : finished              ROUTINE : psa_sn_dti_cnf    |
+-------------------------------------------------------------+

  PURPOSE : processes the SN_DTI_CNF primitive send by SNDCP.
*/
GLOBAL  void psa_sn_dti_cnf ( T_SN_DTI_CNF *sn_dti_cnf )
{
#if defined(FF_PKTIO) OR defined(FF_TCP_IP)
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
#endif

  TRACE_FUNCTION ("psa_sn_dti_cnf()");

  switch( sn_dti_cnf->dti_conn )
  {
  case(NAS_CONNECT_DTI):
    dti_cntrl_entity_connected( sn_dti_cnf->dti_linkid, DTI_ENTITY_SNDCP, DTI_OK );
    break;

  case(NAS_DISCONNECT_DTI):
    dti_cntrl_entity_disconnected( sn_dti_cnf->dti_linkid, DTI_ENTITY_SNDCP );
    break;
  }
/* Issue 34035 - Not possible to perform a context activation with user plane 
   for PKTIO (TCS.4.x) without +CGACT before +CGDATA. */

#if defined(FF_PKTIO) OR defined(FF_TCP_IP)

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  
  if ( p_pdp_context_node )
  {
    if ((p_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_PKTIO) AND 
       (p_pdp_context_node->internal_data.state NEQ PDP_CONTEXT_STATE_ACTIVATING))
    {
      if( cmhSM_next_work_cid( AT_CMD_CGDATA ) EQ FALSE )
      {
        TRACE_EVENT("psa_sn_dti_cnf(): No more context in the work_cid");
      }
      else
      {
        TRACE_EVENT("psa_sn_dti_cnf(): More context in work_cid are getting handled");
      }
    }
  }
  else
  {
    TRACE_EVENT("psa_sn_dti_cnf(): NULL p_pdp_context_node");
  }
 
#endif /* FF_PKTIO OR FF_TCP_IP */

  PFREE (sn_dti_cnf);
}
#endif /* GPRS */

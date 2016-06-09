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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for the registration part of
|             the SNDCP entity.
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef PSA_SNDS_C
#define PSA_SNDS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci.h"

#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"

#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#if defined(FF_WAP)  || defined(FF_GPF_TCPIP)
#include "wap_aci.h"
#include "psa_tcpip.h"
#endif



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_SMS                 |
| STATE   : finished              ROUTINE : psaSNDCP_Dti_Req        |
+-------------------------------------------------------------------+

  PURPOSE : Request of setting or removing the DTI user plane..

*/
GLOBAL void psa_sn_dti_req( T_DTI_CONN_LINK_ID  link_id, T_DTI_ENTITY_ID peer, U8 dti_conn )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = pdp_context_find_node_from_cid(work_cids[cid_pointer]);  /* Issue 27457 */
#ifdef _SIMULATION_
  U8 *dti_neighbor = NULL;
#endif

#if defined (FF_WAP) || defined (FF_GPF_TCPIP)
  U8 cid = PDP_CONTEXT_CID_OMITTED;
  U8 nsapi;
#endif

  U8 dti_direction = NAS_HOME;

  PALLOC (sn_dti_req, SN_DTI_REQ);

  TRACE_FUNCTION("psa_sn_dti_req()");

#ifdef _SIMULATION_
  ACI_MALLOC(dti_neighbor, NAS_SIZE_ENTITY_NAME);
#endif

  if (dti_conn EQ NAS_CONNECT_DTI)
  {

    switch ( peer )
    {
      case DTI_ENTITY_PPPS:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), PPP_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)PPP_NAME;
#endif        
        dti_direction = NAS_HOME;
        p_pdp_context_node->internal_data.entity_id = peer;
        p_pdp_context_node->internal_data.link_id   = link_id;
        break;

#ifdef TI_DUAL_MODE
      case DTI_ENTITY_SNDCP:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), SNDCP_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)SNDCP_NAME;
#endif
        dti_direction = NAS_HOME;
        break;
#endif /* TI_DUAL_MODE */

#ifdef FF_TCP_IP
      case DTI_ENTITY_AAA:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), RIV_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)RIV_NAME;
#endif        
        dti_direction = NAS_NEIGHBOR;
        p_pdp_context_node->internal_data.entity_id = peer;
        p_pdp_context_node->internal_data.link_id   = link_id;
        break;
#endif /* FF_TCP_IP */

      case DTI_ENTITY_IP:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), IP_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)IP_NAME;
#endif
        dti_direction = NAS_NEIGHBOR;
        p_pdp_context_node->internal_data.link_id = link_id;
        break;
      case DTI_ENTITY_SIM:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), SIM_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)SIM_NAME;
#endif
        dti_direction = NAS_NEIGHBOR;
        p_pdp_context_node->internal_data.link_id = link_id;
        break;
      case DTI_ENTITY_PKTIO:

#ifdef _SIMULATION_
        strcpy((char*)(dti_neighbor), PKTIO_NAME);
#else
        sn_dti_req->dti_neighbor = (ULONG)PKTIO_NAME;
#endif
        dti_direction = NAS_HOME;
        p_pdp_context_node->internal_data.entity_id = peer;
        p_pdp_context_node->internal_data.link_id   = link_id;
        break;
#ifdef FF_GPF_TCPIP
      case DTI_ENTITY_TCPIP:
        sn_dti_req->dti_neighbor = (ULONG)TCPIP_NAME;
        dti_direction = NAS_NEIGHBOR;
        break;
#endif /* FF_GPF_TCPIP */

    }

    /* Copy the neighbor name (incl. the zero termination). */
#ifdef _SIMULATION_
    memcpy( &sn_dti_req->dti_neighbor, dti_neighbor, strlen((const char *)dti_neighbor) + 1 );
    ACI_MFREE( dti_neighbor );
#endif
    sn_dti_req->dti_direction = dti_direction;
    sn_dti_req->dti_conn      = NAS_CONNECT_DTI;
  }
  else /* if (dti_conn == SNDCP_CONNECT_DTI) */
  {
    sn_dti_req->dti_conn   = NAS_DISCONNECT_DTI;
  }

  sn_dti_req->dti_linkid    = link_id;
  sn_dti_req->nsapi         = (U8)CID_TO_NSAPI( gaci_get_cid_over_link_id( link_id ) );
#if defined (FF_WAP) || defined (FF_GPF_TCPIP)
  nsapi = sn_dti_req->nsapi;
#endif
  PSEND(hCommSNDCP, sn_dti_req);
#if defined (FF_WAP) || defined (FF_GPF_TCPIP)
  if ( (Wap_Call)                    AND
       (dti_conn EQ NAS_CONNECT_DTI) )   /* TC ACISAT 530 */
  {
    cid = (U8)NSAPI_TO_CID( nsapi );
    if ( (get_state_over_cid( cid ) EQ PDP_CONTEXT_STATE_DEFINED ) OR
         (get_state_over_cid( cid ) EQ PDP_CONTEXT_STATE_ATTACHING) OR 
         (get_state_over_cid( cid ) EQ PDP_CONTEXT_STATE_DEACTIVATE_NORMAL) )  //// pinghua DCM_OPEN_CLOSE patch 20080429 start
    {
      cmhSM_activate_context();
    }
    else if ( (get_state_over_cid( cid ) EQ PDP_CONTEXT_STATE_ACTIVATED))
    {
      char c_ip_address[16];
      UBYTE* ip_address;

      TRACE_EVENT("No PDP Context Found to Activate");

      ip_address = p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4;
      sprintf (c_ip_address, "%03u.%03u.%03u.%03u", ip_address[0],
                                                    ip_address[1],
                                                    ip_address[2],
                                                    ip_address[3] );


      psaTCPIP_Configure( NULL,
                          (UBYTE*) c_ip_address,
  	    	                NULL, 
                          NULL,
                          NULL,
                          1500, 
                          cmhSM_IP_activate_cb );     
    }
  }
#endif /* FF_WAP */
}
#endif /* GPRS */

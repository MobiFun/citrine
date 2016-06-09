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
|  Purpose :  This module implements the source control management for GACI.
+----------------------------------------------------------------------------- 
*/ 


#ifdef GPRS

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "psa_gmm.h"


#include "cmh.h"
#include "cmh_sm.h"
#include "cmh_gppp.h"
#include "cmh_gmm.h"
#include "gaci_srcc.h"


/*===== PRIVATE VARIABLES ==========================================*/
static UBYTE entity_counter[SRRC_MAX_ENTITY];

/*===== FUNCTIONS DECLARATIONS =====================================*/

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : ACI_DTI                  |
| STATE   : finished              ROUTINE : srcc_init                |
+--------------------------------------------------------------------+

  PURPOSE : init of source control management
*/
GLOBAL void srcc_init (void)
{
  SHORT i = 0;

  for (i = 0; i < SRRC_MAX_ENTITY; i++)
    entity_counter[i] = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : ACI_DTI                  |
| STATE   : finished              ROUTINE : srcc_new_count           |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void srcc_new_count ( SRCC_LINK_NO link_no )
{
  
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );

  TRACE_FUNCTION("srcc_new_count()");

  if ( !p_pdp_context_node )
  {
    TRACE_ERROR("PDP Context Not Found");
    return;
  }

  if ( srcc_reserve_sources( link_no, 1) )
  {
    switch( link_no )
    {

      case SRCC_PPPS_SNDCP_LINK:
        entity_counter[SRRC_PPPS_ENTITY]++;
        entity_counter[SRRC_SNDCP_ENTITY]++;
        TRACE_EVENT_P3("link_no = %d, new PPP counter = %d and new SNDCP counter = %d",
                        link_no, entity_counter[SRRC_PPPS_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
        break;

#if defined (FF_PKTIO) OR defined (FF_TCP_IP) OR defined (FF_PSI)
      case SRCC_PKTIO_SNDCP_LINK:
        entity_counter[SRRC_SNDCP_ENTITY]++;
        TRACE_EVENT_P2("link_no = %d, new SNDCP counter = %d",
                        link_no, entity_counter[SRRC_SNDCP_ENTITY]);
        break;
#endif /* FF_PKTIO OR FF_TCP_IP OR FF_PSI */

#ifdef FF_SAT_E        
      case SRCC_SIM_SNDCP_LINK:
        entity_counter[SRRC_SIM_ENTITY]++;
        entity_counter[SRRC_SNDCP_ENTITY]++;
        TRACE_EVENT_P3("link_no = %d, new SIM counter = %d and new SNDCP counter = %d",
                        link_no, entity_counter[SRRC_SIM_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
        break;
#endif
        
      case SRCC_IP_SNDCP_LINK:
#ifdef CO_UDP_IP
        entity_counter[SRRC_IP_ENTITY]++;
        TRACE_EVENT_P2("link_no = %d, new IP counter = %d",
                      link_no, entity_counter[SRRC_IP_ENTITY]);
#endif
        entity_counter[SRRC_SNDCP_ENTITY]++;
        TRACE_EVENT_P1("new SNDCP counter = %d", entity_counter[SRRC_SNDCP_ENTITY]);
        break;
 
#ifdef FF_GPF_TCPIP
      case SRCC_TCPIP_SNDCP_LINK:
        entity_counter[SRRC_TCPIP_ENTITY]++;
        entity_counter[SRRC_SNDCP_ENTITY]++;
        TRACE_EVENT_P3("link_no = %d, new TCPIP counter = %d and new SNDCP counter = %d",
                        link_no, entity_counter[SRRC_TCPIP_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
        break;
#endif
       
      default:
        TRACE_EVENT_P1("ERROR: srcc_new_count: link_no unknown: %d",link_no);
        return;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : ACI_DTI                  |
| STATE   : finished              ROUTINE : srcc_delete_count        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void srcc_delete_count ( SRCC_LINK_NO link_no )
{

  TRACE_FUNCTION("srcc_delete_count()");

  switch( link_no )
  {
    case SRCC_PPPS_SNDCP_LINK:
      entity_counter[SRRC_PPPS_ENTITY]--;
      entity_counter[SRRC_SNDCP_ENTITY]--;
      TRACE_EVENT_P3("link_no = %d, new PPP counter = %d and new SNDCP counter = %d",
                      link_no, entity_counter[SRRC_PPPS_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
      return;
#if defined (FF_PKTIO) OR defined (FF_TCP_IP) OR defined (FF_PSI)
    case SRCC_PKTIO_SNDCP_LINK:
      entity_counter[SRRC_SNDCP_ENTITY]--;
      TRACE_EVENT_P1("new SNDCP counter = %d", entity_counter[SRRC_SNDCP_ENTITY]);
      return;
#endif /* FF_PKTIO OR FF_TCP_IP OR FF_PSI */
#ifdef FF_SAT_E      
    case SRCC_SIM_SNDCP_LINK:
      entity_counter[SRRC_SIM_ENTITY]--;
      entity_counter[SRRC_SNDCP_ENTITY]--;
      TRACE_EVENT_P3("link_no = %d, new SIM counter = %d and new SNDCP counter = %d",
                      link_no, entity_counter[SRRC_SIM_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
      break;
#endif 
    case SRCC_IP_SNDCP_LINK:
#ifdef CO_UDP_IP
      entity_counter[SRRC_IP_ENTITY]--;
      TRACE_EVENT_P2("link_no = %d, new IP counter = %d",
                      link_no, entity_counter[SRRC_IP_ENTITY]);
#endif
      entity_counter[SRRC_SNDCP_ENTITY]--;
      TRACE_EVENT_P1("new SNDCP counter = %d", entity_counter[SRRC_SNDCP_ENTITY]);
      break;

#ifdef FF_GPF_TCPIP
    case SRCC_TCPIP_SNDCP_LINK:
      entity_counter[SRRC_TCPIP_ENTITY]--;  
      entity_counter[SRRC_SNDCP_ENTITY]--;
      TRACE_EVENT_P3("link_no = %d, new TCPIP counter = %d and new SNDCP counter = %d",
                      link_no, entity_counter[SRRC_TCPIP_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
      return;
#endif

    default:
      return;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : ACI_DTI                  |
| STATE   : finished              ROUTINE : srcc_reserve_sources     |
+--------------------------------------------------------------------+

  PURPOSE : check the possibility of creation a number of dti connections
*/
GLOBAL BOOL srcc_reserve_sources( SRCC_LINK_NO link_no, SHORT no )
{

  TRACE_FUNCTION("srcc_reserve_sources()");

  switch( link_no )
  {

    case SRCC_PPPS_SNDCP_LINK:
      if ( entity_counter[SRRC_PPPS_ENTITY]  >= MAX_PPPS_INSTANCES OR
           entity_counter[SRRC_SNDCP_ENTITY] >= MAX_SNDCP_INSTANCES
         )
      {
        TRACE_EVENT_P3("FAILED to reserve resources for link_no = %d, PPP counter = %d and SNDCP counter = %d",
                        link_no, entity_counter[SRRC_PPPS_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
        return FALSE;
      }
      break;

#if defined (FF_PKTIO) OR defined (FF_TCP_IP) OR defined (FF_PSI)
    case SRCC_PKTIO_SNDCP_LINK:
      if ( entity_counter[SRRC_SNDCP_ENTITY] >= MAX_SNDCP_INSTANCES )
      {
        TRACE_EVENT_P2("FAILED to reserve resources for link_no = %d, SNDCP counter = %d",
                        link_no, entity_counter[SRRC_SNDCP_ENTITY]);
        return FALSE;
      }
      break;
#endif /* FF_PKTIO OR FF_TCP_IP  OR FF_PSI */

#ifdef FF_SAT_E      
    case SRCC_SIM_SNDCP_LINK:
      if ( entity_counter[SRRC_SIM_ENTITY] >= MAX_SIM_INSTANCES OR
           entity_counter[SRRC_SNDCP_ENTITY] >= MAX_SNDCP_INSTANCES  )
      {
        TRACE_EVENT_P3("FAILED to reserve resources for link_no = %d, SIM counter = %d and SNDCP counter = %d",
                        link_no, entity_counter[SRRC_SIM_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
        return FALSE;
      }
      break;
#endif /* FF_SAT_E */      

    case SRCC_IP_SNDCP_LINK:
      if ( entity_counter[SRRC_SNDCP_ENTITY] >= MAX_SNDCP_INSTANCES
#ifdef CO_UDP_IP
           OR
           entity_counter[SRRC_IP_ENTITY] >= MAX_IP_INSTANCES
#endif
          )
      {
        TRACE_EVENT_P2("FAILED to reserve resources for link_no = %d, SNDCP counter = %d",
                        link_no, entity_counter[SRRC_SNDCP_ENTITY]);
#ifdef CO_UDP_IP
        TRACE_EVENT_P1(" IP counter = %d", entity_counter[SRRC_IP_ENTITY]);
#endif
        return FALSE;
      }
      break;

    #ifdef FF_GPF_TCPIP
    case SRCC_TCPIP_SNDCP_LINK:
         if ( entity_counter[SRRC_SNDCP_ENTITY] >= MAX_SNDCP_INSTANCES
            OR entity_counter[SRRC_TCPIP_ENTITY] >= MAX_TCPIP_INSTANCES  )
         {
           TRACE_EVENT_P3("FAILED to reserve resources for link_no = %d, TCPIP counter = %d and SNDCP counter = %d",
                        link_no, entity_counter[SRRC_TCPIP_ENTITY], entity_counter[SRRC_SNDCP_ENTITY]);
           return FALSE;
         }
       break;

    #endif /* FF_GPF_TCPIP */

    default:
      return FALSE;
  }

  return TRUE;
}
#endif

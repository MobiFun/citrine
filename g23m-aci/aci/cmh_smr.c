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
|             for the responses of the protocol stack adapter for
|             GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_SMR_C
#define CMH_SMR_C
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
#include "psa_gppp.h"
#include "psa_gmm.h"
#include "psa_sim.h"

#include "cmh.h"
#include "cmh_sm.h"
#include "cmh_gmm.h"
#include "cmh_gppp.h"
#include "gaci_srcc.h"
#include "dti.h"      /* functionality of the dti library */

#include "psa_uart.h"

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
#include "wap_aci.h"
#include "psa_tcpip.h"
#include "psa_cc.h"
#include "cmh_cc.h"
#include "psa_sat.h"
#include "cmh_sat.h"

#include "dcm_f.h"
#endif /* (CO_UDP_IP) OR defined (FF_GPF_TCPIP) */
#include "aci_lst.h"
#include "ati_int.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;

/*==== FUNCTIONS ==================================================*/

static void dumpContextInfo(U8 cid)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  if( ! p_pdp_context_node )
    return;

  TRACE_EVENT_P2("ERROR in context state: cid %d, state %d", 
                cid, p_pdp_context_node->internal_data.state);
  TRACE_EVENT_P3("dump: nsapi %d, srcID %d, connected entity %d", 
                  CID_TO_NSAPI( cid ), p_pdp_context_node->internal_data.owner,
                  p_pdp_context_node->internal_data.entity_id);
  if(smEntStat.curCmd NEQ AT_CMD_NONE)
  {
    TRACE_EVENT_P1("dump running command: %d", smEntStat.curCmd); 
  }
  TRACE_EVENT_P3("dump link_ids: new %d sn %d uart %d", 
      p_pdp_context_node->internal_data.link_id_new, p_pdp_context_node->internal_data.link_id,
      p_pdp_context_node->internal_data.link_id_uart); 
}

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
static BOOL is_ip_dti_id(T_DTI_ENTITY_ID dti_id)
{
  if(is_gpf_tcpip_call()) {
    GPF_TCPIP_STATEMENT(return dti_id EQ DTI_ENTITY_TCPIP);
  }
  else {
    return dti_id EQ DTI_ENTITY_IP;
  }
}
#endif

static void get_dns_address(char* dns1, char *dns2, UBYTE *pco, UBYTE len)
{
  ULONG tmp_dns1, tmp_dns2, gateway;
  utl_analyze_pco(pco,len, &tmp_dns1, &tmp_dns2, &gateway);

  sprintf(dns1, "%03u.%03u.%03u.%03u", (tmp_dns1 & 0xff000000) >> 24,
                                       (tmp_dns1 & 0x00ff0000) >> 16,
                                       (tmp_dns1 & 0x0000ff00) >> 8 ,
                                       (tmp_dns1 & 0x000000ff) );
  sprintf(dns2, "%03u.%03u.%03u.%03u", (tmp_dns2 & 0xff000000) >> 24,
                                       (tmp_dns2 & 0x00ff0000) >> 16,
                                       (tmp_dns2 & 0x0000ff00) >> 8 ,
                                       (tmp_dns2 & 0x000000ff) );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finished         ROUTINE : cmhSM_Activated              |
+-------------------------------------------------------------------+

  PURPOSE : confirms a successful context activation

*/
GLOBAL SHORT cmhSM_Activated ( T_SMREG_PDP_ACTIVATE_CNF *pdp_cnf )
{
  char dns1[16], dns2[16];
  U8                     cid                = work_cids[cid_pointer];
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_ACI_AT_CMD curCmd = smEntStat.curCmd;
  T_CGEREP_EVENT_REP_PARAM  event;
  int i = 0;

  TRACE_FUNCTION ("cmhSM_Activated()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Activated" );
    return -1;
  }

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( get_state_working_cid() )
  {
    case( PDP_CONTEXT_STATE_ACTIVATING ):

      R_AT( RAT_CGACT, smEntStat.entOwn )( 0 );

      set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED );

      /* Storing of Dynamic address sent in SMREG_PDP_ACTIVATE_CNF message is missing in
         Alborg code.
       */
      p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = pdp_cnf->ctrl_ip_address;

      if ( p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address )
      {
        if ( p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address EQ NAS_is_ipv4 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4, &pdp_cnf->ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );          
    		  TRACE_EVENT_P1("PDP Address: %s", p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4);
        }
        else
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6, &pdp_cnf->ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
          TRACE_EVENT_P1("PDP Address: %s", p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6);
        }
      }
      else
      {
        if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4, &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );
          p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address;
          TRACE_EVENT_P1("PDP Address: %s", p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4);
        }
        else if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv6 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6, &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
          p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address;
          TRACE_EVENT_P1("PDP Address: %s", p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6);
        }
        else
        {
          TRACE_ERROR( "ERROR: NO IP Address Assigned, In PDP_CONTEXT_STATE_ACTIVATING State" );
        }
      }

      cmhSM_set_PCO( cid, 
                     PCO_NETWORK,
                     &smShrdPrm.pdp_cnf->sdu.buf[smShrdPrm.pdp_cnf->sdu.o_buf >> 3],
                     (UBYTE) (smShrdPrm.pdp_cnf->sdu.l_buf >> 3));

      get_dns_address(dns1, dns2,
                      p_pdp_context_node->internal_data.network_pco.pco,
                      p_pdp_context_node->internal_data.network_pco.len);

      TRACE_EVENT_P2("DNS1: %s, DNS2: %s", dns1, dns2);

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
      /* if WAP/TCPIP over GPRS is in progress, request WAP configuration */
      if ( is_ip_dti_id(p_pdp_context_node->internal_data.entity_id) )
      {
        char c_ip_address[16];
        UBYTE* ip_address;

        ip_address = p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4;
        sprintf (c_ip_address, "%03u.%03u.%03u.%03u", ip_address[0],
                                                      ip_address[1],
                                                      ip_address[2],
                                                      ip_address[3] );

        psaTCPIP_Configure(NULL, (UBYTE*) c_ip_address,
                           NULL, (UBYTE*)dns1, (UBYTE*)dns2, 1500, 
                           cmhSM_IP_activate_cb );
      }
#endif /* (CO_UDP_IP) OR defined (FF_GPF_TCPIP) */

      /*
       *  do we need one more context activation
       */
      if( FALSE EQ cmhSM_next_work_cid( curCmd ) )
      {
        if( AT_CMD_CGACT EQ curCmd)
        {
          gaci_RAT_caller ( RAT_OK, cid, (UBYTE) curCmd, 0 );

          /* log result */
          cmh_logRslt ( p_pdp_context_node->internal_data.owner, RAT_OK, curCmd, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent);
#if defined (FF_WAP) OR defined (FF_SAT_E)
          if (Wap_Call)
          {
            gpppEntStat.curCmd = AT_CMD_NONE;
          }
#endif /* WAP OR SAT E */
        }
      }
      
      break;

    case( PDP_CONTEXT_STATE_ESTABLISH_2 ):
    /*
     *---------------------------------------------------------------
     * inform PPP    
     *---------------------------------------------------------------
     */
      cmhSM_set_PCO( cid ,
                     PCO_NETWORK,
                     &smShrdPrm.pdp_cnf->sdu.buf[smShrdPrm.pdp_cnf->sdu.o_buf >> 3],
                     (UBYTE) (smShrdPrm.pdp_cnf->sdu.l_buf >> 3));

      /* Storing of Dynamic address sent in SMREG_PDP_ACTIVATE_CNF message is missing in
          Alborg code.
       */
      p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = smShrdPrm.pdp_cnf->ctrl_ip_address;

      if ( p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address )
      {
        if ( p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address EQ NAS_is_ipv4 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4, &smShrdPrm.pdp_cnf->ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );
        }
        else
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6, &smShrdPrm.pdp_cnf->ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
        }
      }
      else
      {
        if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv4_addr.a4, &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );
          p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address;
        }
        else if ( p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv6 )
        {
          memcpy( p_pdp_context_node->internal_data.pdp_address_allocated.ip_address.ipv6_addr.a6, &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
          p_pdp_context_node->internal_data.pdp_address_allocated.ctrl_ip_address = p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address;
        }
        else
        {
          TRACE_ERROR( "ERROR: NO IP Address Assigned, In PDP_CONTEXT_STATE_ESTABLISH_2" );
        }
      }

      psaGPPP_PDP_Activate( &p_pdp_context_node->internal_data.pdp_address_allocated,
                            &smShrdPrm.pdp_cnf->sdu.buf[smShrdPrm.pdp_cnf->sdu.o_buf >> 3],
                            (UBYTE) (smShrdPrm.pdp_cnf->sdu.l_buf >> 3),
                            (UBYTE) CID_TO_NSAPI( cid ) );

      set_state_working_cid( PDP_CONTEXT_STATE_ESTABLISH_3 );

      break;
      
    default:
      return -1;
  }
  
  /*
  *   %CGEV - GPRS event reporting
  */

  strcpy( event.act.pdp_type, p_pdp_context_node->attributes.pdp_type );
  memcpy(&(event.act.pdp_addr), &(p_pdp_context_node->internal_data.pdp_address_allocated), sizeof(T_NAS_ip));
  
  event.act.cid      = cid;
  
  if( smShrdPrm.direc EQ CGEREP_EVENT_ME_ACT )
  {
    for( i = 0; i < CMD_SRC_MAX; i++ )
    {
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_ACT, &event );
    }
  }
  else
  {
    for( i = 0; i < CMD_SRC_MAX; i++ )
    {
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_ACT, &event );
    }
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finished         ROUTINE :                              |
+-------------------------------------------------------------------+

  PURPOSE : 

*/

LOCAL void cp_pdp_rej_prim( T_SMREG_PDP_ACTIVATE_REJ * pdp_activate_rej,
                            T_PPP_PDP_ACTIVATE_REJ   *activate_result )
{

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finished         ROUTINE : cmhSM_NoActivate             |
+-------------------------------------------------------------------+

  PURPOSE : indicates a context activation failed

*/
GLOBAL SHORT cmhSM_NoActivate ( void )
{
  T_CGEREP_EVENT_REP_PARAM  event;
  T_PDP_CONTEXT_STATE state;
  T_DTI_CONN_LINK_ID link_id;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  int i = 0;
 
  TRACE_FUNCTION ("cmhSM_NoActivate()");
/*
 *-------------------------------------------------------------------
 * Set error cause for SIM owner
 *-------------------------------------------------------------------
 */
#ifdef FF_SAT_E
  gaci_SAT_err((UBYTE)smShrdPrm.pdp_rej->ps_cause.value.nwsm_cause);
#endif /* FF_SAT_E */
/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  
  p_pdp_context_node  = pdp_context_find_node_from_cid( NSAPI_TO_CID( smShrdPrm.pdp_rej->nsapi ) );
  if (!p_pdp_context_node ) 
  {
    return -1;
  }

  state = get_state_over_cid(p_pdp_context_node->cid);

  switch( state )
  {
    case PDP_CONTEXT_STATE_INVALID:
    case PDP_CONTEXT_STATE_DEFINED:
    case PDP_CONTEXT_STATE_ATTACHING:    	
    case PDP_CONTEXT_STATE_ESTABLISH_1:
    case PDP_CONTEXT_STATE_ESTABLISH_3:
    case PDP_CONTEXT_STATE_ACTIVATED:
    case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
    case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
    case PDP_CONTEXT_STATE_DATA_LINK:
    case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
    case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
    case PDP_CONTEXT_STATE_REACTIVATION_1:
    case PDP_CONTEXT_STATE_REACTIVATION_2:
    case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
      dumpContextInfo(work_cids[cid_pointer]);
      TRACE_ERROR("cmhSM_NoActivate(): State/Event error!!!");
      return 0;

    case PDP_CONTEXT_STATE_ACTIVATING:
      /*Instead of: link_id = cmhSM_get_link_id_SNDCP_peer(work_cids[cid_pointer], SNDCP_PEER_NORMAL);*/
      link_id = p_pdp_context_node->internal_data.link_id;

      switch(p_pdp_context_node->internal_data.entity_id)
      {
        case DTI_ENTITY_PKTIO:
        case DTI_ENTITY_PSI:          
          set_state_working_cid( PDP_CONTEXT_STATE_DEACTIVATE_NORMAL );
          dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(link_id));
//          gaci_RAT_caller(RAT_NO_CARRIER, work_cids[cid_pointer], AT_CMD_CGDATA, 0);
          break;

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
        case DTI_ENTITY_IP:
        GPF_TCPIP_STATEMENT(case DTI_ENTITY_TCPIP:)
          set_state_working_cid( PDP_CONTEXT_STATE_DEACTIVATE_NORMAL ); 
          dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(link_id));
          /* tell WAP ACI that contextactivation was rejected */
          psaTCPIP_Deactivate(cmhSM_IP_activate_cb);
          dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_SNDCP );
          break;
#endif /* WAP OR FF_SAT_E OR FF_GPF_TCPIP */          
        default:
          /* in this case is SMREG_PDP_ACTIVATE_REJ the same as SMREG_DEACTIVATE_CNF */

          /* set parameter for SMREG_DEACTIVATE_CNF */
          smShrdPrm.nsapi_set = 1 << smShrdPrm.pdp_rej->nsapi;
          cmhSM_Deactivated(); /* SMREG_DEACTIVATE_CNF */
     
          /* the last expected primitive from SM */
          smEntStat.curCmd = AT_CMD_NONE;
          return 0;
      }

     /*
      *   GPRS event reporting
      */
      strcpy( event.act.pdp_type, p_pdp_context_node->attributes.pdp_type );
      memcpy(&(event.act.pdp_addr), &(p_pdp_context_node->attributes.pdp_addr), sizeof(T_NAS_ip));
      event.act.cid = work_cids[cid_pointer];
      for( i = 0; i < CMD_SRC_MAX; i++ )
      {
        R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DEACT, &event );
        R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DEACT, &event );
      }

      cmhSM_contextDeactivated();  
      break;

    case PDP_CONTEXT_STATE_ESTABLISH_2:
    case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
      /* in this case is SMREG_PDP_ACTIVATE_REJ the same as SMREG_DEACTIVATE_CNF */

      /* set parameter for SMREG_DEACTIVATE_CNF */
      smShrdPrm.nsapi_set = 1 << CID_TO_NSAPI( p_pdp_context_node->cid );

      cmhSM_Deactivated(); /* SMREG_DEACTIVATE_CNF */
     
      /* the last expected primitive from SM */
      smEntStat.curCmd = AT_CMD_NONE;

      if( state NEQ  PDP_CONTEXT_STATE_ESTABLISH_2 )
      {
        return 0;
      }

     /*
     *---------------------------------------------------------------
     * inform PPP
     *---------------------------------------------------------------
     */
      {
        PALLOC( act_rej_temp, PPP_PDP_ACTIVATE_REJ ); /* ppass */
        gpppShrdPrm.setPrm[gpppEntStat.entOwn].pdp_rej = act_rej_temp;
        act_rej_temp->ppp_cause = smShrdPrm.pdp_rej->ps_cause.value.ppp_cause;
      }

      if( psaGPPP_PDP_Reject() < 0 )
      {
        TRACE_EVENT( "FATAL RETURN psaPPP_PDP_Reject in +CGDATA" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        return -1 ;
      }

      set_state_over_cid( p_pdp_context_node->cid, PDP_CONTEXT_STATE_ABORT_ESTABLISH );
      break;
      
    default:
      return -1;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finished         ROUTINE : cmhSM_NetActivate            |
+-------------------------------------------------------------------+

  PURPOSE : indicates a network asked for a PDP context activation

*/
GLOBAL void cmhSM_NetActivate ( void )
{
  T_CGEREP_EVENT_REP_PARAM  event;
  U8 i = 0;                            /* holds index  counter */
  U8 context_reactivation = 0;
  U8 cid;

  TRACE_FUNCTION ("cmhSM_NetActivate()");

  /*
   *  in first only one requested context activation is provided
   *  and the request will be rejected if an other context will be
   *  activated
   */

  /*
   *  it's no check to CC neccesary, because this is no class A mobile
   */

  if( cmhSM_is_smreg_ti_used( smShrdPrm.act_ind.ti, &cid ) EQ TRUE )
    context_reactivation = 1;

  /*
   *  no PDP context is during the setup
   */
  if ( work_cids[cid_pointer] EQ PDP_CONTEXT_CID_INVALID          AND
       gprs_ct_index < MAX_GPRS_CALL_TABLE_ENTRIES                
       AND
       ( TRUE EQ srcc_reserve_sources( SRCC_PPPS_SNDCP_LINK, 1 )  OR
         context_reactivation EQ 1)        
         )
         

  {
/*
 *-------------------------------------------------------------------
 * ring for call, if no call is in use
 *-------------------------------------------------------------------
 */

#ifdef AT_INTERPRETER
    /* V.24 Ring Indicator Line */
    /* io_setRngInd ( IO_RS_ON, CRING_TYP_NotPresent, CRING_TYP_NotPresent dummy parameters here, need real ones when used); */
#endif

    /* fill gprs call table */
    memcpy(&gprs_call_table[gprs_ct_index].sm_ind, &smShrdPrm.act_ind, sizeof(T_SMREG_PDP_ACTIVATE_IND));

    /* no context reactivation */
    if ( context_reactivation NEQ 1 )
    {
      gprs_call_table[gprs_ct_index].reactivation = GCTT_NORMAL;
      *gprs_call_table[gprs_ct_index].L2P = 0;
      gprs_call_table[gprs_ct_index].cid = PDP_CONTEXT_CID_INVALID;
      gprs_ct_index++;

      for( i = 0 ; i < CMD_SRC_MAX; i++ )
      {
        R_AT( RAT_CRING, (T_ACI_CMD_SRC)i )  ( CRING_MOD_Gprs, CRING_SERV_TYP_GPRS, CRING_SERV_TYP_NotPresent );
      }
    }
    else
    { /* context reactivation */
      gprs_call_table[gprs_ct_index].reactivation = GCTT_REACTIVATION ;
      strcpy(gprs_call_table[gprs_ct_index].L2P, "PPP");
      gprs_call_table[gprs_ct_index].cid    = cid;
      gprs_ct_index++;
    }

  }
  else
  /*
   *  one or more PDP contexts are during the setup
   */
  {
    psaSM_PDP_No_activate(smShrdPrm.act_ind.ti, CAUSE_NWSM_INSUFFICIENT_RESOURCES);

   /*
    *   GPRS event reporting
    */
    cmhSM_pdp_typ_to_string(smShrdPrm.act_ind.pdp_type, event.reject.pdp_type);
   
   event.reject.pdp_addr.ctrl_ip_address = smShrdPrm.act_ind.ctrl_ip_address;

   if (event.reject.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4)
   {
     memcpy( &event.reject.pdp_addr.ip_address.ipv4_addr, &smShrdPrm.act_ind.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );
   }
   else
   {
     memcpy( &event.reject.pdp_addr.ip_address.ipv6_addr, &smShrdPrm.act_ind.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
   }

    for( i = 0; i < CMD_SRC_MAX; i++ )
    {
      R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_REJECT, &event );
      R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_REJECT, &event );
    }

  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_Deactivated            |
+-------------------------------------------------------------------+

  PURPOSE : confirms a successful PDP context deactivation

*/

GLOBAL void cmhSM_Deactivated ( void )
{
  T_CGEREP_EVENT_REP_PARAM  event;
  T_PDP_CONTEXT_INTERNAL    *p_pdp_context_node = NULL;
  T_PDP_CONTEXT_STATE       state;

  U8                        cid;
  SHORT                     i, 
                            reactivation = 0,
                            rat_id = RAT_MAX;
  UBYTE                     cme_err = CME_ERR_GPRSUnspec, /* error number */
                            cmdBuf = smEntStat.curCmd;    /* buffers current command */
  U16                       nsapi_set_to_free = smShrdPrm.nsapi_set;
  T_DTI_CONN_LINK_ID        dti_link_id;
  UBYTE srcId = srcId_cb;
  
  TRACE_FUNCTION ("cmhSM_Deactivated()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */

  /* 28592 cmhSM_context_deactivated(smShrdPrm.nsapi_set); */

  while((state = get_state_over_nsapi_set( &smShrdPrm.nsapi_set, &cid )) AND state NEQ  PDP_CONTEXT_STATE_INVALID )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( !p_pdp_context_node )
    {
      TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Deactivated" );
      return;
    }

    dti_link_id = p_pdp_context_node->internal_data.link_id;

    switch ( state )
    {
      case PDP_CONTEXT_STATE_INVALID:
      case PDP_CONTEXT_STATE_DEFINED:
      case PDP_CONTEXT_STATE_ATTACHING:
      case PDP_CONTEXT_STATE_ESTABLISH_1:
      case PDP_CONTEXT_STATE_ESTABLISH_3:
      case PDP_CONTEXT_STATE_ACTIVATED:
      case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
      case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
      case PDP_CONTEXT_STATE_DATA_LINK:
      case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
      default:
        dumpContextInfo(cid);
        continue;

      case PDP_CONTEXT_STATE_ESTABLISH_2:

        set_state_over_cid ( cid, PDP_CONTEXT_STATE_ABORT_ESTABLISH );
        //asdf        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));

        break;

      case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
        TRACE_EVENT("state: PDP_CONTEXT_STATE_ABORT_ESTABLISH");
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);

        if(ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Disable)
          rat_id = RAT_NO_CARRIER;
        else
          rat_id = RAT_CME;
        break;

      case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
        TRACE_EVENT("state: CS_BREAKDOWN_LINK_NORMAL");
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);
        rat_id = RAT_NO_CARRIER;
        break;

      case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
        TRACE_EVENT("state: PDP_CONTEXT_DEACTIVATE_NORMAL") ;
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );

#if defined(FF_PKTIO) OR defined(FF_TCP_IP)
        if( p_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_PKTIO )
        {
          /* Issue OMAPS00072119: For PKTIO, delete the SNDCP counter */
          cmhSM_disconnect_cid(cid, GC_TYPE_NULL);
        }
#endif

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
        if ( is_ip_dti_id(p_pdp_context_node->internal_data.entity_id) )
        if( Wap_Call )
        {
          /* tell WAP ACI that contextactivation was rejected */
          psaTCPIP_Deactivate(cmhSM_IP_activate_cb);
        } 
#else  /* (CO_UDP_IP) OR defined (FF_GPF_TCPIP) */
        rat_id = RAT_NO_CARRIER;
#endif
        break;

      case PDP_CONTEXT_STATE_ACTIVATING:
        TRACE_EVENT("state: PDP_CONTEXT_STATE_ACTIVATING");
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );

        /* 18th April 2005. As till this date we are not handiling the correct 
           cause value between ACI and SM. As we are not mapping the cause values 
           sent by SM enity to ACI. This is creating problem to display wrong ERROR
           values .*/

        cme_err = cmhSM_mapSM2ACI_Cause(smShrdPrm.pdp_rej->ps_cause.value.nwsm_cause);
        rat_id = RAT_CME;
        break;

      case PDP_CONTEXT_STATE_REACTIVATION_1:
        TRACE_EVENT("state: PDP_CONTEXT_STATE_REACTIVATION_1");
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_REACTIVATION_2 );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        smEntStat.curCmd = AT_CMD_NONE;
        nsapi_set_to_free &= ~( 1U << CID_TO_NSAPI(cid) ); /* 28592 */
        continue;
        
      case PDP_CONTEXT_STATE_REACTIVATION_2:
        TRACE_EVENT("state: PDP_CONTEXT_STATE_REACTIVATION_2");
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);
        smEntStat.curCmd = AT_CMD_NONE;
        rat_id = RAT_NO_CARRIER;
        reactivation = 1;
        break;
    }

    if ( reactivation EQ 0 )
    {
        switch( smEntStat.curCmd )
        {
          case( AT_CMD_CGDATA ):
          case( AT_CMD_CGACT ):
          TRACE_EVENT_P2("cmhSM_Deactivated: smEntStat.entOwn: Source = %d and GPRS related AT Command is smEntStat.curCmd: = %d", smEntStat.entOwn, smEntStat.curCmd );
          cmhSM_clear_work_cids(cid);
          break;
        default:
          TRACE_EVENT_P1("cmhSM_Deactivated: Current AT Command is smEntStat.curCmd: = %d", smEntStat.curCmd);
          break;
      }      
    }

    strcpy( event.act.pdp_type, p_pdp_context_node->attributes.pdp_type );
    memcpy(&(event.act.pdp_addr), &(p_pdp_context_node->internal_data.pdp_address_allocated), sizeof(T_NAS_ip));

    event.act.cid      = cid;
    for( i = 0; i < CMD_SRC_MAX; i++ )
    {
      R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DEACT, &event );
      R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_ME_DEACT, &event );
    }

    if ( rat_id NEQ RAT_MAX )
    {
      gaci_RAT_caller ( rat_id, cid, cmdBuf, cme_err );

      rat_id = RAT_MAX;

      cmhSM_clear_work_cids(cid);
      cmhSM_context_reactivation();
    }
  }

  cmhSM_context_deactivated(nsapi_set_to_free); /* 28592 */
  cmhSM_contextDeactivated();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_NetDeactivate          |
+-------------------------------------------------------------------+

  PURPOSE : indicates a PDP context deactivation

*/
GLOBAL void cmhSM_NetDeactivate ( void )
{
  T_PDP_CONTEXT_INTERNAL    *p_pdp_context_node = NULL;
  T_PDP_CONTEXT_INTERNAL    *p_prev_pdp_context_node = NULL;
  T_CGEREP_EVENT_REP_PARAM  event;
  T_PDP_CONTEXT_STATE       state;
  U8                        cid;
  SHORT                     i = 0, 
                            inform_ppp = 1, 
                            rat_id = RAT_MAX;
  UBYTE                     cme_err = CME_ERR_Unknown, /* error number */
                            cmdBuf = AT_CMD_NONE;      /* buffers current command */
  T_DTI_CONN_LINK_ID        dti_link_id;

  USHORT                    temp_nsapiSet = smShrdPrm.nsapi_set;
  UBYTE srcId = srcId_cb;
  
  
  TRACE_FUNCTION ("cmhSM_NetDeactivate()");

  /* 28592 cmhSM_context_deactivated(smShrdPrm.nsapi_set); */

  while( (smShrdPrm.nsapi_set) AND (PDP_CONTEXT_STATE_INVALID NEQ (state = get_state_over_nsapi_set( &smShrdPrm.nsapi_set, &cid )) ))
  {

    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( !p_pdp_context_node )
    {
      TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_NetDeactivate" );
      continue; /* Try the next... */
    }

    dti_link_id = p_pdp_context_node->internal_data.link_id;
    
    switch ( state )
    {
      case PDP_CONTEXT_STATE_INVALID:
      case PDP_CONTEXT_STATE_DEFINED:
      case PDP_CONTEXT_STATE_ESTABLISH_1:
      case PDP_CONTEXT_STATE_ATTACHING:
        TRACE_EVENT("PDP context state ignored, continue");
        continue;

      case PDP_CONTEXT_STATE_ESTABLISH_2:
      case PDP_CONTEXT_STATE_ESTABLISH_3:
      case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_ABORT_ESTABLISH );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        break;

      case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);
        inform_ppp = 0;
        if(ati_user_output_cfg[srcId].CMEE_stat EQ CMEE_MOD_Disable)
          rat_id = RAT_NO_CARRIER;
        else
          rat_id = RAT_CME;
        break;

      case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        cmhSM_disconnect_cid(cid, GC_TYPE_DATA_LINK);
        rat_id = RAT_NO_CARRIER;
        inform_ppp = 0;
        break;

#ifdef REL99
      case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
        /* Reject +CGCMOD command if executing. Network context deactivation has higher priority. */
        if (smEntStat.curCmd EQ AT_CMD_CGCMOD)
        {
           /* Answer the executing source not the data source for cid */
           R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_GPRSUnspec );
           /* Remove command */
           smEntStat.curCmd = AT_CMD_NONE;
           cid_pointer  = 0;
           work_cids[0] = PDP_CONTEXT_CID_OMITTED;
        }        
#endif
        /* Fall through */
      case PDP_CONTEXT_STATE_DATA_LINK:
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL );
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        break;
#ifdef REL99
      case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
        /* Reject +CGCMOD command if executing. Network context deactivation has higher priority. */
        if (smEntStat.curCmd EQ AT_CMD_CGCMOD)
        {
           /* Answer the executing source not the data source for cid */
           R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_GPRSUnspec );
           /* Remove command */
           smEntStat.curCmd = AT_CMD_NONE;
           cid_pointer  = 0;
           work_cids[0] = PDP_CONTEXT_CID_INVALID;
        }        
        /* Fall through */
#endif
      case PDP_CONTEXT_STATE_ACTIVATED:
        p_prev_pdp_context_node = pdp_context_find_node_from_cid( cid );
        inform_ppp = 0;
        
        /* Issue OMAPS00062126: new case of connection failed, AT+CGDATA return ERROR */
        if( (state EQ PDP_CONTEXT_STATE_ACTIVATED) AND
            (p_prev_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_PKTIO) )
        {
          TRACE_EVENT("Free the DTI Links for PKTIO Entity after AT+CFUN=0");
          dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(p_prev_pdp_context_node->internal_data.link_id));
        }
        
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
        if ( is_ip_dti_id(p_pdp_context_node->internal_data.entity_id) )
        {
          if ( p_prev_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_IP )
          {
            /* tell WAP ACI that contextactivation was rejected */
            psaTCPIP_Deactivate(cmhSM_IP_activate_cb);
            dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
          }
        }
        else
#endif /* WAP OR SAT E */
        {
          cmhSM_disconnect_cid(cid, GC_TYPE_NULL);
        }
        rat_id = RAT_NO_CARRIER;
        
        break;

      case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
        p_prev_pdp_context_node = pdp_context_find_node_from_cid( cid );        
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );

#if defined (FF_WAP) OR defined (FF_SAT_E)
        inform_ppp = 0;
        if ( p_prev_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_IP )
        {
          /* tell WAP ACI that contextactivation was rejected */
          dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
          psaTCPIP_Deactivate(cmhSM_IP_activate_cb);         
        }
        else
#endif /* WAP OR SAT E */
        {
          cmhSM_disconnect_cid(cid, GC_TYPE_NULL);
        }
        rat_id = RAT_NO_CARRIER; /* it required ?!! */
        break;

      case PDP_CONTEXT_STATE_ACTIVATING:
        p_prev_pdp_context_node = pdp_context_find_node_from_cid( cid );
        inform_ppp = 0;
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );

#if defined (FF_WAP) OR defined (FF_SAT_E)
        if ( p_prev_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_IP )
        {
          /* tell WAP ACI that contextactivation was rejected */
          psaTCPIP_Deactivate(cmhSM_IP_activate_cb);
          dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(dti_link_id));
        }
#endif /* WAP OR SAT E */
        rat_id = RAT_CME;
        break;

      case PDP_CONTEXT_STATE_REACTIVATION_1:
        set_state_over_cid(cid, PDP_CONTEXT_STATE_REACTIVATION_2);
        cmhSM_stop_context_reactivation();
        temp_nsapiSet &= ~( 1U << CID_TO_NSAPI(cid) ); /* 28592 */
        continue;
        
      case PDP_CONTEXT_STATE_REACTIVATION_2:
        set_state_over_cid ( cid, PDP_CONTEXT_STATE_DEFINED );
        cmhSM_stop_context_reactivation();
        rat_id = RAT_NO_CARRIER;
        inform_ppp = 0;
        break;
    }

    if ( inform_ppp )
    {
    /*
     *---------------------------------------------------------------
     * inform PPP
     *---------------------------------------------------------------
     */  
      psaGPPP_Terminate( PPP_LOWER_LAYER_UP );
    }

      switch( smEntStat.curCmd )
      {
        case( AT_CMD_CGDATA ):
        case( AT_CMD_CGACT ):
        TRACE_EVENT_P2("cmhSM_NetDeactivate: smEntStat.entOwn: Source = %d and GPRS related AT Command is smEntStat.curCmd: = %d", smEntStat.entOwn, smEntStat.curCmd );
        cmhSM_clear_work_cids(cid);
        break;
      default:
        TRACE_EVENT_P1("cmhSM_NetDeactivate: Current AT Command is smEntStat.curCmd: = %d", smEntStat.curCmd);
        break;    
    }

    /*
     *   GPRS event reporting
     */
    strcpy( event.act.pdp_type, p_pdp_context_node->attributes.pdp_type );
    memcpy(&(event.act.pdp_addr), &(p_pdp_context_node->internal_data.pdp_address_allocated), sizeof(T_NAS_ip));
    event.act.cid      = cid;
    for( i = 0; i < CMD_SRC_MAX; i++ )
    {
      R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_DEACT, &event ); 
      R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_DEACT, &event );
    }
    
    if ( rat_id NEQ RAT_MAX )
    {

      /* Below code will decide whether to send NO CARRIER OR NOT
         depeding on the context flags and source's
       */

      if ( !( gmmShrdPrm.gprs_call_killer EQ p_pdp_context_node->internal_data.owner ) )                
      {
        gaci_RAT_caller ( rat_id, cid, cmdBuf, cme_err );
      }
      
      gmmShrdPrm.gprs_call_killer = (UBYTE)CMD_SRC_NONE;

      cmhSM_clear_work_cids(cid);    
      rat_id = RAT_MAX;
    }
  }

  cmhSM_context_deactivated(temp_nsapiSet);
  cmhSM_contextDeactivated();
  /* inform SAT if needed */
#if defined (FF_SAT_E)
  cmhSAT_OpChnGPRSDeact();
#endif 
}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_NetModify              |
+-------------------------------------------------------------------+

  PURPOSE : indicates a network initiated PDP context modification

*/
GLOBAL void cmhSM_NetModify( T_SMREG_PDP_MODIFY_IND *smreg_pdp_modify_ind )
{
  U8    cid = 0; 
  U8    i   = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhSM_NetModify()");

  if( get_state_over_cid( NSAPI_TO_CID( smreg_pdp_modify_ind->nsapi ) ) NEQ  PDP_CONTEXT_STATE_INVALID )
    {
    p_pdp_context_node = pdp_context_find_node_from_cid( NSAPI_TO_CID( smreg_pdp_modify_ind->nsapi ) );
    cid = p_pdp_context_node->cid;
    if( p_pdp_context_node )
    {
      /* Store the QoS indicated by the network */
      p_pdp_context_node->ctrl_neg_qos = smreg_pdp_modify_ind->ctrl_qos;  
      switch( smreg_pdp_modify_ind->ctrl_qos )
      {
        case PS_is_R97:
          memcpy( &p_pdp_context_node->neg_qos.qos_r97, &smreg_pdp_modify_ind->qos.qos_r97, sizeof(T_PS_qos_r97) );
          break;
        case PS_is_R99:
          memcpy( &p_pdp_context_node->neg_qos.qos_r99, &smreg_pdp_modify_ind->qos.qos_r99, sizeof(T_PS_qos_r99) );
          break;
        default:
          TRACE_ERROR( "QoS type indicated by NW not supported" );
      } 

      /* inform all sources */
      for( i = 0 ; i < CMD_SRC_MAX; i++ )
      {
        switch( p_pdp_context_node->ctrl_neg_qos )
        {
          case PS_is_R97:
            R_AT( RAT_QOS_MOD,(T_ACI_CMD_SRC) i ) ( cid, p_pdp_context_node->neg_qos.qos_r97 ); /* the macro must be updated, QoS type changed !!! */
            break;
          case PS_is_R99:
            R_AT( RAT_QOS_MOD,(T_ACI_CMD_SRC) i ) ( cid, p_pdp_context_node->neg_qos.qos_r99 ); /* the macro must be updated, QoS type changed !!! */
            break;
        }
      }

    }
    else
    {
      TRACE_ERROR( "PDP context not found, in function cmhSM_NetModify" );
    }
  }
  else
  {
    TRACE_ERROR( "PDP context for the given nsapi is in the wrong state" );
  }

}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_Modified               |
+-------------------------------------------------------------------+

  PURPOSE : indicates that the MT initiated PDP context modification
            was successful. Continue to modify the rest of the contexts
            until the work_cids list is empty.

*/
#ifdef REL99
GLOBAL SHORT cmhSM_Modified( T_SMREG_PDP_MODIFY_CNF *smreg_pdp_modify_cnf )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhSM_Modified()");

  if ( NSAPI_TO_CID( smreg_pdp_modify_cnf->nsapi ) NEQ  work_cids[cid_pointer])
  {
    /* Wrong nsapi received from SM */
    TRACE_ERROR("WRONG NSAPI. Ignored.");
    return -1;
  }

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );

  if( p_pdp_context_node EQ NULL )
  {
    TRACE_ERROR ("PDP context not found");
    return -1;
  }

  /* This context was successful modified: Change state back. */
  switch( get_state_working_cid() )
  {
    case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
      set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED );
      break;

    case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
      set_state_working_cid( PDP_CONTEXT_STATE_DATA_LINK );
      break;

    default:
      /* Error handling */
      TRACE_ERROR("WRONG STATE for context.");
      /* Reject command if possible */
      if (smEntStat.curCmd EQ AT_CMD_CGCMOD)
      {
        R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_GPRSUnspec );
        smEntStat.curCmd = AT_CMD_NONE;
        cid_pointer  = 0;
        work_cids[0] = PDP_CONTEXT_CID_INVALID;
      }
      else
      {
        TRACE_ERROR("SMREG_PDP_MODIFY_CNF received but +CGCMOD not executed");
      }
      return -1;
  }



  cid_pointer ++; // This might be wrong


  /* Investigate if more contexts to modify */
  if( (cid_pointer EQ PDP_CONTEXT_CID_MAX) OR (work_cids[cid_pointer] EQ PDP_CONTEXT_CID_INVALID) )
  {
    /* No more contexts to modify. All context modification were successful. */

    switch( smEntStat.curCmd )
    {
      case AT_CMD_CGACT:
      {
        R_AT( RAT_CGACT, smEntStat.entOwn )(0);
        break;
      }
      case AT_CMD_CGCMOD:
      {
        R_AT( RAT_CGCMOD, smEntStat.entOwn )();
        break;
      }
    }

    R_AT( RAT_OK, smEntStat.entOwn ) ( smEntStat.curCmd );
    smEntStat.curCmd = AT_CMD_NONE;
    cid_pointer  = 0;
    work_cids[0] = PDP_CONTEXT_CID_INVALID;
  
  }
  else
  {
    /* More contexts to modify */
    switch (get_state_over_cid(work_cids[cid_pointer]))
    {
      case PDP_CONTEXT_STATE_ACTIVATED:
        set_state_over_cid(work_cids[cid_pointer], PDP_CONTEXT_STATE_ACTIVATED_MODIFYING);
        /* Send next modify request */
        psaSM_PDP_Modify();
        break;

      case PDP_CONTEXT_STATE_DATA_LINK:
        set_state_over_cid(work_cids[cid_pointer], PDP_CONTEXT_STATE_DATA_LINK_MODIFYING);
        /* Send next modify request */
        psaSM_PDP_Modify();
        break;

      default:
      {
        /* The state has changed probably due to network deactivation. The command fails. */
        R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_GPRSUnspec );
        smEntStat.curCmd = AT_CMD_NONE;
        cid_pointer  = 0;
        work_cids[0] = PDP_CONTEXT_CID_INVALID;
      }
    }
  }
  return 0;
}
#endif



/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_NoModify               |
+-------------------------------------------------------------------+

  PURPOSE : indicates that the MT initiated PDP context modification
            failed.
  RETURN  : Callback with CME ERROR or just ERROR.
*/
#ifdef REL99
GLOBAL SHORT cmhSM_NoModify( T_SMREG_PDP_MODIFY_REJ *smreg_pdp_modify_rej )
{
  SHORT result = 0;

  
  TRACE_FUNCTION ("cmhSM_NoModify()");

  if (smreg_pdp_modify_rej EQ NULL)
    return -1;

  switch( smEntStat.curCmd )
  {
    case AT_CMD_CGCMOD:
    {
      /* Reset state to the previous. */
      switch( get_state_working_cid() )
      {
        case PDP_CONTEXT_STATE_ACTIVATED_MODIFYING:
          set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED );
          break;
          
        case PDP_CONTEXT_STATE_DATA_LINK_MODIFYING:
          set_state_working_cid( PDP_CONTEXT_STATE_DATA_LINK );
          break;

        default:
          TRACE_FUNCTION("State for modified context has been changed (Network deactivation?).");
      }

      /* Reject the +CGCMOD command */
      R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_GPRSUnspec );
      smEntStat.curCmd = AT_CMD_NONE;
      cid_pointer  = 0;
      work_cids[0] = PDP_CONTEXT_CID_INVALID;
      break;
    }
    
    default:
    {
      result = -1;
      break;
    }
  } /* End switch( smEntStat.curCm ) */
  
  return result;
  
}

#endif

/*
+-------------------------------------------------------------------+
| PROJECT : UMTS             MODULE  : CMH_SMR                      |
| STATE   :                  ROUTINE : cmhSM_ActivatedSecondary     |
+-------------------------------------------------------------------+

  PURPOSE : confirms a successful secondary context activation

*/
#ifdef REL99
GLOBAL SHORT cmhSM_ActivatedSecondary( T_SMREG_PDP_ACTIVATE_SEC_CNF *p_smreg_pdp_activate_sec_cnf )
{
  U8 cid = work_cids[cid_pointer];
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_sec_node  = NULL;

  TRACE_FUNCTION ("cmhSM_ActivatedSecondary()");

 /*
  *  Find the secondary PDP context
  */
  p_pdp_context_sec_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_sec_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_ActivatedSecondary" );
    return -1;
  }

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  switch( smEntStat.curCmd )
  {
    case( AT_CMD_CGACT ):
      TRACE_EVENT("CGACT is current command");
      if( get_state_working_cid() NEQ  PDP_CONTEXT_STATE_ACTIVATING )
      {
       /*
        *   no action
        */

        return 0;
      }
      R_AT( RAT_CGACT, smEntStat.entOwn )(p_pdp_context_sec_node->internal_data.link_id);
      
      set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED );

      /*
       *   do we need one more context activation
       */
       
      if( cmhSM_next_work_cid( AT_CMD_CGACT ) EQ FALSE )
      {
        R_AT( RAT_OK, smEntStat.entOwn ) ( smEntStat.curCmd );

        /* log result */
        cmh_logRslt ( smEntStat.entOwn, RAT_OK, smEntStat.curCmd, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent);
        
        smEntStat.curCmd = AT_CMD_NONE;
      }


      break;
      
    case( AT_CMD_CGDATA ):
      TRACE_EVENT("CGDATA is current command");
      if( get_state_working_cid() NEQ  PDP_CONTEXT_STATE_ESTABLISH_2 )
      {
       /*
        *    no action
        */
        return 0;
      }

    /*
     *---------------------------------------------------------------
     * inform PPP
     *---------------------------------------------------------------
     */
      psaGPPP_PDP_Activate( &p_pdp_context_sec_node->internal_data.pdp_address_allocated,
                            p_pdp_context_sec_node->internal_data.user_pco.pco,
                            p_pdp_context_sec_node->internal_data.user_pco.len, 
                            (U8)CID_TO_NSAPI( p_pdp_context_sec_node->cid ) );

      set_state_working_cid( PDP_CONTEXT_STATE_ESTABLISH_3 );

      break;
      
    default:
      TRACE_EVENT("current command not detected!");
      return -1;
  }

  TRACE_EVENT_P2( "cmhSM_Activated, dti_entity_connected, dti_it = %d, cid_ptr = %d", 
                  p_pdp_context_sec_node->internal_data.link_id, cid_pointer );
  dti_cntrl_entity_connected(p_pdp_context_sec_node->internal_data.link_id, DTI_ENTITY_SNDCP, DTI_OK);
  
  return 0;
}
#endif /* REL99 */


#ifdef REL99
GLOBAL SHORT cmhSM_NoActivateSecondary( T_SMREG_PDP_ACTIVATE_SEC_REJ *p_smreg_pdp_activate_sec_rej )
{
  short res = 0;

  TRACE_FUNCTION( "cmhSM_NoActivateSecondary" );
  TRACE_EVENT( "This funciton is not implemented!" );

  return res;
}
#endif /* REL99 */
GLOBAL BOOL PKTIO_UPM_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  U8 cid;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("PKTIO_UPM_connect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      cid = (U8)gaci_get_cid_over_dti_id(dti_id);
      p_pdp_context_node = pdp_context_find_node_from_cid(cid);

      if( (INVALID_CID   NEQ cid )                                          AND
          ( PDP_CONTEXT_STATE_ACTIVATING  EQ get_state_over_cid(cid))       AND
          (( DTI_ENTITY_PKTIO EQ p_pdp_context_node->internal_data.entity_id) OR
            (DTI_ENTITY_AAA   EQ p_pdp_context_node->internal_data.entity_id)))
      {
        cmhSM_deactivateAContext(CMD_SRC_NONE, cid);
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEACTIVATE_NORMAL);
      }
      break;
    case DTI_CONN_STATE_DISCONNECTED:
    case DTI_CONN_STATE_CONNECTING:
    case DTI_CONN_STATE_CONNECTED:
      break;
    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect SNDCP */
      dti_cntrl_close_dpath_from_dti_id( dti_id );
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("PKTIO_UPM_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}


#endif /* GPRS */
/*==== EOF ========================================================*/

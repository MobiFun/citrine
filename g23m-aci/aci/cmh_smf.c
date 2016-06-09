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
|             handler for GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_SMF_C
#define CMH_SMF_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "gprs.h"

#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_lst.h"
#include "aci_mem.h"
#include "aci.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#include "psa_gppp.h"

#include "phb.h"
#include "cmh.h"
#include "cmh_gmm.h"

#include "cmh_sm.h"
#include "cmh_gppp.h"
#include "gaci_srcc.h"
#include "psa_cc.h"

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
#include "wap_aci.h"
#include "psa_tcpip.h"
#include "cmh_ipa.h"
#endif /* WAP  OR FF_GPF_TCPIP OR SAT E */

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#endif

#include "psa_sim.h"
#include "cmh_sim.h"

#include "cmh_sm.h"
/* temp needed because of use of ATI functions here: should eventually disappear */
#include "ati_int.h"

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif
#include "dcm_f.h"
#include "psa_uart.h"


/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/
typedef struct
{
  T_CGEREP_EVENT            event;
  T_CGEREP_EVENT_REP_PARAM  parm;

} T_CGERP_EVENT_BUFFER;

typedef enum
{
  T_CDS_IDLE,
  T_CDS_RUNNING

} T_CONTEXTS_DEACTIVATION_STATUS;

typedef struct
{
  T_CONTEXTS_DEACTIVATION_STATUS state;
  USHORT         nsapi_set;
  T_ACI_CMD_SRC  srcId;
  U8             cid_set;

} T_CONTEXTS_DEACTIVATION_INFORMATION;

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
static T_CGSMS_SERVICE      m_service;
static T_CGERP_EVENT_BUFFER gprs_event_buffer[GPRS_EVENT_REPORTING_BUFFER_SIZE];
static SHORT                gprs_eb_current_p, gprs_eb_oldest_p;
static BOOL                 m_mt_te_link;
static BOOL                 call_waits_in_table;
static T_CONTEXTS_DEACTIVATION_INFORMATION  working_cgact_actions;
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;
/*==== FUNCTIONS ==================================================*/

/*
 *  functions for ATA and ATH
 */
static BOOL cmhSM_sAT_A_H_intern( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret, SHORT mode );


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Init               |
+--------------------------------------------------------------------+

  PURPOSE : Fill variables for the own use with default values.
*/
GLOBAL void cmhSM_Init (void)
{

  UBYTE default_pco[] = {
                                0x80,0x80,0x21,0x10,0x01,0x01,0x00,0x10,0x81,0x06,
                                0x00,0x00,0x00,0x00,0x83,0x06,0x00,0x00,0x00,0x00
                             };

  T_PDP_CONTEXT default_pdp_context = { "",
                                        "",
                                        NAS_is_ip_not_present,0,
                                        PDP_CONTEXT_D_COMP_OMITTED, 
                                        PDP_CONTEXT_H_COMP_OMITTED,
                                        PDP_CONTEXT_CID_OMITTED };


  /* SM CMH global parameter */
  smEntStat.curCmd = AT_CMD_NONE;
  smEntStat.entOwn = CMD_SRC_NONE;

  memset( &pdp_context_default, 0, sizeof(T_PDP_CONTEXT_INTERNAL) );

  memset( work_cids, PDP_CONTEXT_CID_INVALID, sizeof(work_cids) );
    
  pdp_context_default.ctrl_qos     = PS_is_R97;
  pdp_context_default.ctrl_min_qos = PS_is_min_qos_not_present;
  
  
  //set default context values initial !
  memcpy( &pdp_context_default.attributes, &default_pdp_context , sizeof(T_PDP_CONTEXT) );
  strcpy(  pdp_context_default.attributes.pdp_type, "IP" );

  memcpy( &pdp_context_default.internal_data.user_pco.pco, &default_pco, sizeof( default_pco ) );
           pdp_context_default.internal_data.user_pco.len = sizeof( default_pco );
  
  /* GPRS event reporting */
  memset( gprs_event_buffer, 0, sizeof(T_CGERP_EVENT_BUFFER) * GPRS_EVENT_REPORTING_BUFFER_SIZE );

  gprs_eb_current_p = 0;
  gprs_eb_oldest_p  = 0;

  cmhSM_empty_call_table();

  /* used for network requested context reactivation */
  call_waits_in_table = FALSE;

  working_cgact_actions.state     = T_CDS_IDLE;
  working_cgact_actions.nsapi_set = 0;
  working_cgact_actions.srcId     = CMD_SRC_NONE;
  working_cgact_actions.cid_set   = 0;

  m_mt_te_link = FALSE;
}

/*
+-------------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                       |
| STATE   :                       ROUTINE : cmhSM_ResetNonWorkingContexts |
+-------------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void cmhSM_ResetNonWorkingContexts( void )
{

  UBYTE cid = 0;
  T_PDP_CONTEXT_STATE    pdp_context_state;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  UBYTE                   k = 0;

  UBYTE gprs_default_pco[] = {
                                0x80,0x80,0x21,0x10,0x01,0x01,0x00,0x10,0x81,0x06,
                                0x00,0x00,0x00,0x00,0x83,0x06,0x00,0x00,0x00,0x00
                             };

  T_PDP_CONTEXT default_pdp_context = { "",
                                        "",
                                        NAS_is_ip_not_present,0,
                                        PDP_CONTEXT_D_COMP_OMITTED, 
                                        PDP_CONTEXT_H_COMP_OMITTED,
                                        PDP_CONTEXT_CID_OMITTED };

  TRACE_FUNCTION("cmhSM_ResetNonWorkingContexts()");

 
  /****************************************************************************
     The reset of PDP contexts to factory setting of causes:
     - Any defined context to be deactivated.
     - The default context and QoS is set to initial values.
   ***************************************************************************/

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node AND k < PDP_CONTEXT_CID_MAX )
  {
    cid = p_pdp_context_node->cid;
    pdp_context_state = pdp_context_get_state_for_cid( cid );

    if( pdp_context_state EQ PDP_CONTEXT_STATE_DEFINED )
    {
      if( !pdp_context_cid_used_by_other( cid ) )
      {
        pdp_context_remove_node( cid );
        TRACE_EVENT("ATZ: UDEFINED the defined PDP Context");
      }
      else
      {
        TRACE_ERROR("PDP Context Not Found");
      }
    }
    k++;
    p_pdp_context_node = p_pdp_context_node->p_next;

  }

  TRACE_EVENT("ATZ: Command Initiated Restting all the Factory Defined Values");

  /* set default context parameter */
  memset( &pdp_context_default,  0, sizeof(T_PDP_CONTEXT_INTERNAL) );

  memset( work_cids, PDP_CONTEXT_CID_INVALID, sizeof(work_cids) );
  cid_pointer = 0;

  //set default context values initial !
  memcpy( &pdp_context_default.attributes, &default_pdp_context , sizeof(T_PDP_CONTEXT) );
  strcpy(  pdp_context_default.attributes.pdp_type, "IP" );

  pdp_context_default.ctrl_qos     = PS_is_R97;
  pdp_context_default.ctrl_min_qos = (T_PS_ctrl_min_qos)PS_is_R97;

  memcpy( &pdp_context_default.internal_data.user_pco.pco, &gprs_default_pco, sizeof( gprs_default_pco ) );
           pdp_context_default.internal_data.user_pco.len = sizeof( gprs_default_pco );

  /* mode of CGAUTO*/
  automatic_response_mode = 3;

  m_service = CGSMS_SERVICE_CS_PREFERRED;

  /* GPRS event reporting */
  sm_cgerep_mode    = CGEREP_MODE_BUFFER;
  sm_cgerep_bfr     = CGEREP_BFR_CLEAR;
  sm_cgsms_service  = CGSMS_SERVICE_CS_PREFERRED;    
}

GLOBAL void cmhSM_Reset( void )
{


  /* SMF CMH local parameter */

  memset( work_cids, PDP_CONTEXT_CID_INVALID, sizeof(work_cids) );

  cid_pointer = 0;

  /* set default context parameter */

  /* mode of CGAUTO*/
  automatic_response_mode = 3;

  m_service = CGSMS_SERVICE_CS_PREFERRED;

  /* GPRS event reporting */
  sm_cgerep_mode   = CGEREP_MODE_BUFFER;
  sm_cgerep_bfr    = CGEREP_BFR_CLEAR;
  sm_cgsms_service = CGSMS_SERVICE_CS_PREFERRED;
  sm_cgerep_srcId   = (T_ACI_CMD_SRC)CMD_SRC_ATI;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_empty_call_table   |
+--------------------------------------------------------------------+

  PURPOSE : Fill variables for the own use with default values.
*/
GLOBAL void cmhSM_empty_call_table (void)
{
  memset( gprs_call_table, 0, sizeof(T_GPRS_CALL_TABLE) *MAX_GPRS_CALL_TABLE_ENTRIES );
  current_gprs_ct_index = 0;
  gprs_ct_index = 0;
  gprs_call_table[0].sm_ind.ti = UNDEFINED_TI;
}


/*
+-------------------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                                  |
| STATE   : code             ROUTINE : cmhSM_getSrcIdOfRunningCGACTDeactivation |
+-------------------------------------------------------------------------------+

  PURPOSE : Returns a source ID if the given cid is requested to deactivate
            by +CGACT. The source ID indicates where the +CGACT was started.
*/
GLOBAL T_ACI_CMD_SRC cmhSM_getSrcIdOfRunningCGACTDeactivation(U8 cid)
{
  if( ( 1 << cid ) & working_cgact_actions.cid_set )
  {
    return working_cgact_actions.srcId;
  }
  return CMD_SRC_NONE;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : code             ROUTINE : cmhSM_connection_down           |
+----------------------------------------------------------------------+

  PURPOSE : Control the answer of context deactivations 
            started by an AT command.
*/
GLOBAL void cmhSM_connection_down( UBYTE dti_id ) 
{
  SHORT cid = gaci_get_cid_over_dti_id( dti_id );

  TRACE_FUNCTION("cmhSM_connection_down");

  switch( working_cgact_actions.state )
  {
    case T_CDS_RUNNING:
      TRACE_EVENT_P1("T_CDS_RUNNING, nsapi:%d", CID_TO_NSAPI(cid));
      if  ( ( 1 << CID_TO_NSAPI(cid) ) & working_cgact_actions.nsapi_set )
      { /* nsapi deactivation is requested */

        working_cgact_actions.nsapi_set &= (USHORT) ~(1U << CID_TO_NSAPI(cid));
        if ( ! working_cgact_actions.nsapi_set )
        { 
            R_AT( RAT_OK, working_cgact_actions.srcId ) ( AT_CMD_CGACT );
            working_cgact_actions.state = T_CDS_IDLE;
        }
        else
        {
          TRACE_EVENT_P1("NO OK: nsapi_set:%d",working_cgact_actions.nsapi_set);
        }
      }
      else
      {      
        TRACE_EVENT_P1("meets not the nsapi_set: %d",working_cgact_actions.nsapi_set);
      }
      break;
    case T_CDS_IDLE:
      TRACE_EVENT("T_CDS_IDLE");
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_contextDeactivated |
+--------------------------------------------------------------------+

  PURPOSE : Detach mobile if necessary? Only possible if all context are disconnected.
  cmhSM_automatic_detach is called after SM is deactivated (either MS initiated or
  network initiated)
*/
GLOBAL void cmhSM_contextDeactivated ( void )
{

  if( ! cmhSM_isContextActive() )
  {
    cmhGMM_allContextsDeactivated();
  }
}

/*
+-------------------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                                  |
| STATE   : code             ROUTINE : isContextDeactivationRequestedByCGACT    |
+-------------------------------------------------------------------------------+

  PURPOSE : Returns TRUE if +CGACT is running on the given cid -> NO_CARRIER
                    FALSE if no CGACT was running on this cid  -> CME_ERROR
*/
GLOBAL BOOL isContextDeactivationRequestedByCGACT(SHORT cid)
{
  TRACE_FUNCTION("***isContextDeactivationRequestedByCGACT");

  switch( working_cgact_actions.state )
  {
    case T_CDS_RUNNING:
      if  ( (1 << (cid - 1)) & working_cgact_actions.cid_set )
      {
        return TRUE;
      }
  }
  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Get_pdp_type       |
+--------------------------------------------------------------------+

  PURPOSE : Give the PDP type of the current PDP context that will build.
*/
GLOBAL UBYTE cmhSM_Get_pdp_type( void )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  
  if( p_pdp_context_node )
  {
    if( !strcmp( p_pdp_context_node->attributes.pdp_type, "PPP" ) )
      return SMREG_PDP_PPP;
    if( !strcmp( p_pdp_context_node->attributes.pdp_type, "IP") )
      return SMREG_PDP_IPV4;
    if( !strcmp( p_pdp_context_node->attributes.pdp_type, "IPV6") )
      return SMREG_PDP_IPV6;

    /* Otherwise return an empty pdp_type. */
    return SMREG_PDP_EMPTY;
  }

  return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Get_pdp_address    |
+--------------------------------------------------------------------+

  PURPOSE : Give the PDP address of the current PDP context that will build.
*/
GLOBAL void cmhSM_Get_pdp_address ( T_NAS_ip_address *pdp_address, T_NAS_ctrl_ip_address * ctrl_ip_address )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    * ctrl_ip_address = p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address;
    if (p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv4)
    {
      memcpy( &(pdp_address->ipv4_addr.a4), &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR);
    }
    else if(p_pdp_context_node->attributes.pdp_addr.ctrl_ip_address EQ NAS_is_ipv6)
    {
      memcpy( &(pdp_address->ipv6_addr.a6), &p_pdp_context_node->attributes.pdp_addr.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR);
    }
    else
    {
      TRACE_EVENT("Dynamic IP Address");
    }
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Get_pdp_address" );
  }
  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_pdp_address_to_ip  |
+--------------------------------------------------------------------+

  PURPOSE : Transform a PDP address to 4 BYTE IP form.
*/
GLOBAL UBYTE cmhSM_pdp_address_to_ip( T_PDP_TYPE pdp_type, T_NAS_ip *pdp_addr_str, U8 *pdp_addr )
{

  UBYTE addr_len    = 0;

  switch( cmhSM_transform_pdp_type(pdp_type))
  {
    case PDP_T_IP:
      addr_len = 4;
      break;
    case PDP_T_IPV6:
      addr_len = 16;
      break;
    default:
      addr_len = 0;
      break;
  }
  if (addr_len)
  {
    memcpy(pdp_addr, pdp_addr_str, addr_len);
  }
  else
  {
    *pdp_addr = 0;
  }

  return addr_len;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                 |
| STATE   : finished              ROUTINE : convert_netaddr_to_apn  |
+-------------------------------------------------------------------+

  PURPOSE : converts a domain name into an APN

  Technical Information:

            The APN contains out of labels separated by dots.
            (e.g. xxx.yyy.zzz)

            This string representation must be translated to a network
            representation.

            The APN network representation contains out of a sequence of
            a length field (not ASCII) followed by a ASCII string.

            xxx.yyy.zzz => 3xxx3yyy3zzz
*/

LOCAL void convert_netaddr_to_apn ( T_SMREG_apn *apn)
{
  UBYTE counter  = 0, 
        buffer   = apn->apn_buf[0],
        *pdest   = apn->apn_buf + apn->c_apn_buf,
        *psource = pdest - 1;

  if(apn->c_apn_buf EQ 0)
  {
    return;
  }

  if(apn->c_apn_buf >= sizeof apn->apn_buf)
  {
    apn->c_apn_buf = 0;
    TRACE_EVENT_P2 ("convert_netaddr_to_apn: array out of bounds exeption (%d >= %d)", apn->c_apn_buf, sizeof apn->apn_buf);
    return;
  }

  /* The network representation is 1 byte longer. */
  apn->c_apn_buf++;

  /* A sentinel */
  apn->apn_buf[0] = '.';
  
  /* Algorithm: copy from back to front! */
  while(pdest > apn->apn_buf )
  {
    counter = 0;
    while(*psource NEQ '.')
    {
      *(pdest--) = *(psource--);
      counter++;
    }
    *(pdest--) = counter;
    psource--;
  }

  /* Correction according to the sentinel */

  apn->apn_buf[1] = buffer;
  apn->apn_buf[0] = ++counter;

  /* Modify special empty APN to the need of SMREG_SAP */
  if ((apn->c_apn_buf EQ 2) AND (apn->apn_buf[0] EQ 1) AND (apn->apn_buf[1] EQ 255))
  {
    apn->c_apn_buf = 1; /* Special SMREG_SAP indicating that there is an APN present but empty */
    apn->apn_buf[0]= 0;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Get_smreg_apn      |
+--------------------------------------------------------------------+

  PURPOSE : Give the APN of the current PDP context that will build.
*/
GLOBAL void cmhSM_Get_smreg_apn( T_SMREG_apn *smreg_apn )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    smreg_apn->c_apn_buf = strlen(p_pdp_context_node->attributes.pdp_apn);
    strncpy((char *)smreg_apn->apn_buf, (const char *)p_pdp_context_node->attributes.pdp_apn, smreg_apn->c_apn_buf);
    convert_netaddr_to_apn(smreg_apn);
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Get_smreg_apn" );
  }
  
#ifdef _SIMULATION_
  memset( smreg_apn->apn_buf + smreg_apn->c_apn_buf, 0, sizeof(smreg_apn->apn_buf) - smreg_apn->c_apn_buf );
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Get_h_comp         |
+--------------------------------------------------------------------+

  PURPOSE : Give the h_comp of the current PDP context that will build.
*/
GLOBAL  UBYTE cmhSM_Get_h_comp ( void )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    return (UBYTE) p_pdp_context_node->attributes.h_comp;
  }

  TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Get_h_comp" );
  return 0;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Get_d_comp         |
+--------------------------------------------------------------------+

  PURPOSE : Give the d_comp of the current PDP context that will build.
*/
GLOBAL UBYTE cmhSM_Get_d_comp( void )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    return (UBYTE) p_pdp_context_node->attributes.d_comp;
  }

  TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Get_d_comp" );
  return 0;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_change_def_QOS     |
+--------------------------------------------------------------------+

  PURPOSE : Set the quality of service (requested) of default context.

*/
GLOBAL void cmhSM_change_def_QOS( T_PS_qos *qos, T_PS_ctrl_qos ctrl_qos )
{

  pdp_context_default.ctrl_qos = ctrl_qos;

  if(ctrl_qos EQ PS_is_R97) {
    memcpy( &pdp_context_default.qos.qos_r97, &(qos->qos_r97), sizeof(T_PS_qos_r97) );
  }
  else {
    memcpy( &pdp_context_default.qos.qos_r99, &(qos->qos_r99), sizeof(T_PS_qos_r99) );
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_change_def_QOS_min |
+--------------------------------------------------------------------+

  PURPOSE : Set the quality of service (min.) of default context.

*/
GLOBAL void cmhSM_change_def_QOS_min( T_PS_min_qos *qos, T_PS_ctrl_min_qos ctrl_min_qos )
{

  pdp_context_default.ctrl_min_qos = ctrl_min_qos;

  if(ctrl_min_qos EQ PS_is_min_R97) {
    memcpy( &pdp_context_default.min_qos.qos_r97, &(qos->qos_r97), sizeof(T_PS_qos_r97) );
  }
  else {
    memcpy( &pdp_context_default.min_qos.qos_r99, &(qos->qos_r99), sizeof(T_PS_qos_r99) );
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Set_default_QOS    |
+--------------------------------------------------------------------+

  PURPOSE : Set the quality of service of the spezified PDP context
            to default.
*/
GLOBAL void cmhSM_Set_default_QOS( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    p_pdp_context_node->ctrl_qos = pdp_context_default.ctrl_qos;
    memcpy( &p_pdp_context_node->qos, &pdp_context_default.qos, sizeof(T_PS_qos) );
  }
  else
  {
    TRACE_ERROR(" ERROR: PDP context not found, in function cmhSM_Set_default_QOS ");
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_Set_default_QOS    |
+--------------------------------------------------------------------+

  PURPOSE : Set the quality of service of the spezified PDP context
            to default.
*/
GLOBAL void cmhSM_Set_default_QOS_min ( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    p_pdp_context_node->ctrl_min_qos = pdp_context_default.ctrl_min_qos;
    memcpy( &p_pdp_context_node->min_qos, &pdp_context_default.min_qos, sizeof(T_PS_min_qos) );
  }
  else
  {
    TRACE_ERROR(" ERROR: PDP context not found, in function cmhSM_Set_default_QOS_min");
  }
 
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : changed, smn          ROUTINE : cmhSM_Get_QOS            |
+--------------------------------------------------------------------+

  PURPOSE : Give the requested quality of service of the current
            PDP context that will build.
*/
GLOBAL void cmhSM_Get_QOS( T_PS_qos *dest_qos )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
    memcpy( dest_qos, &p_pdp_context_node->qos, sizeof(T_PS_qos) );
  else
    TRACE_ERROR( "ERROR: PDP context not found, in funciton cmhSM_Get_QOS" );

} /* End: cmhSM_Get_QOS */


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : changed, smn          ROUTINE : cmhSM_Get_QOS_min        |
+--------------------------------------------------------------------+

  PURPOSE : Give the minimum acceptable quality of service of the
            current PDP context that will build.
*/
GLOBAL void cmhSM_Get_QOS_min ( T_PS_min_qos *dest_qos_min )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
    memcpy( dest_qos_min, &p_pdp_context_node->min_qos, sizeof(T_PS_min_qos) );
  else
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Get_QOS_min" );

} /* End: cmhSM_Get_QOS_min */



GLOBAL  USHORT cmhSM_pdp_typ_to_string ( UBYTE pdp_typ_no, char* string )
{
  switch ( pdp_typ_no )
  {
    case 0:
      strcpy (string, "X_121");
      return 5;
    case 33:
      strcpy (string, "IP_V_4");
      return 6;
    case 87:
      strcpy (string, "IP_V_6");
      return 6;
    default:
      strcpy (string, "");
      return 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                  |
| STATE   :                       ROUTINE : cmhSM_pdp_type_to_string |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL SHORT cmhSM_pdp_type_to_string ( UBYTE pdp_type_no, char* string )
{
  SHORT i;

  switch ( pdp_type_no )
  {
  case 0:
    strcpy (string, "X_121");
    i = 5;
    break;
  case 33:
    strcpy (string, "IP_V_4");
    i = 6;
    break;
  case 87:
    strcpy (string, "IP_V_6");
    i = 6;
    break;
  default:
    strcpy (string, "");
    i = 0;
  }

  return i;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_ring_gprs_par      |
+--------------------------------------------------------------------+

  PURPOSE : Return the information for CRING.
*/
GLOBAL  char* cmhSM_ring_gprs_par ( void )
{
  static char string[MAX_CRING_INFORMATION_LENGTH] = "\"";
  unsigned int i = 1;

  i += cmhSM_pdp_typ_to_string(gprs_call_table[current_gprs_ct_index].sm_ind.pdp_type, string + i);
  string[i++] = '\"';

  string[i++] = ',';

  string[i++] = '\"';
  if (gprs_call_table[current_gprs_ct_index].sm_ind.ctrl_ip_address EQ NAS_is_ipv4)
  {
    memcpy (string + i, gprs_call_table[current_gprs_ct_index].sm_ind.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR);
    i += NAS_SIZE_IPv4_ADDR + 1;
  }
  else
  {
    memcpy (string + i, gprs_call_table[current_gprs_ct_index].sm_ind.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR);
    i += NAS_SIZE_IPv6_ADDR + 1;
  }


  string[i++] = '\"';

  if ( *gprs_call_table[current_gprs_ct_index].L2P )
  {
  string[i++] = ',';

  string[i++] = '\"';
  strcpy (string + i, gprs_call_table[current_gprs_ct_index].L2P);
  i += strlen (gprs_call_table[current_gprs_ct_index].L2P);
  string[i++] = '\"';
  }

  string[i] = 0;

  return string;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_call_answer        |
+--------------------------------------------------------------------+

  PURPOSE : Return TRUE in this case, if an auto answer is needed.
*/
GLOBAL  BOOL  cmhSM_call_answer ( UBYTE ring_counter, T_ACI_CRING_MOD mode )
{

  switch(automatic_response_mode)
  {
    case 0: /* GPRS off, GSM controlled by S0 */
      if( mode NEQ CRING_MOD_Gprs AND
          at.S[0]                 AND
          at.S[0] <= ring_counter )
        return TRUE;
      break;

    case 1: /* GPRS on, GSM controlled by S0 */
      if( mode EQ CRING_MOD_Gprs )
        return TRUE;
      
      if( at.S[0] AND 
          at.S[0] <= ring_counter )
        return TRUE;
      break;

    case 2: /* modem copatibility mode, GPRS on, GSM off */
      if ( mode NEQ CRING_MOD_Gprs )
        break;

      /*lint -fallthrough*/
      /*lint -fallthrough*/
    case 3: /* modem copatibility mode, GPRS on, GSM on */
      if (at.S[0] AND ring_counter >= at.S[0])
        return TRUE;
  }

  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_call_reject        |
+--------------------------------------------------------------------+

  PURPOSE : Return TRUE in this case, if an auto reject is needed.
*/
GLOBAL  BOOL  cmhSM_call_reject ( UBYTE ring_counter, T_ACI_CRING_MOD mode )
{
  switch(automatic_response_mode)
  {
    case 0: /* GPRS off, GSM controlled by S0 */
      return FALSE;
    case 1: /* GPRS on, GSM controlled by S0 */
      if (at.S99 AND mode EQ CRING_MOD_Gprs )
        return TRUE;
      break;
    case 2: /* modem copatibility mode, GPRS on, GSM off */
    case 3: /* modem copatibility mode, GPRS on, GSM on */
      if ( mode NEQ CRING_MOD_Gprs )
        break;
      if (at.S99 AND ring_counter >= at.S99)
        return TRUE;
  }

  return FALSE;
}




/*
+--------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                  |
| STATE   :                       ROUTINE : is_GSM_call_active       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/

LOCAL BOOL is_GSM_call_active (void)
{
  SHORT ctbIdx;               /* holds call table index */

  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL)
    {
      return (TRUE);
    }
  }

  return (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_sAT_H              |
+--------------------------------------------------------------------+

  PURPOSE : handle GPRS calls and return FALSE if a circuit switched
            call need a handle.
*/
GLOBAL BOOL cmhSM_sAT_H( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret )
{

  SHORT cid_array[1] = { PDP_CONTEXT_CID_INVALID };
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, (UBYTE)srcId, search_ati_src_id);

  if ( ( at.rngPrms.isRng EQ TRUE ) AND ( at.rngPrms.mode EQ CRING_MOD_Gprs) )
  {
   /*
    *   brz patch: In the case of context reactivation over SMREG_PDP_ACTIVATE_IND with an used ti
    *              the GPRS ATH command doesn't do anything!
    *
    *   Why?       Because the Windows Dial-Up Networking client send every time an ATH after termination
    *              of the connection and with this a context reactivation was impossible. 
    */
    if ( gprs_call_table[current_gprs_ct_index].reactivation EQ GCTT_NORMAL )
    {
      return cmhSM_sAT_A_H_intern(srcId, aci_ret, 0);
    }
    return TRUE;
  }
  else
  {
    if (is_GSM_call_active())
    {
      return (FALSE);
    }
   /* if AT_H has been called and no RING is active, then disconnect the active
       context */
#ifdef FF_GPF_TCPIP
    if(is_gpf_tcpip_call())
    {
      T_DCM_STATUS_IND_MSG err_ind_msg;
      err_ind_msg.hdr.msg_id = DCM_ERROR_IND_MSG;
      err_ind_msg.result = DCM_PS_CONN_BROKEN;
      dcm_send_message(err_ind_msg, DCM_SUB_NO_ACTION);
    }
#endif
    *aci_ret = sAT_PlusCGACT ( srcId, CGACT_STATE_DEACTIVATED, cid_array );

    switch (*aci_ret)
    {
      case (AT_CMPL):                         /*operation completed*/
        return FALSE; /* return false, so that GSM calls will be canceled */

      case (AT_EXCT):
        src_params->curAtCmd    = AT_CMD_CGACT;
        return TRUE;

      default:
        cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
        return FALSE;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_sAT_A              |
+--------------------------------------------------------------------+

  PURPOSE : handle GPRS calls and return FALSE if a circuit switched
            call need a handle.
*/
GLOBAL BOOL cmhSM_sAT_A( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret )
{
  BOOL b;

  b = cmhSM_sAT_A_H_intern(srcId, aci_ret, 1);

  if ( *aci_ret EQ AT_EXCT )
    cmdErrStr   = NULL;

  return b;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_sAT_A_H_intern     |
+--------------------------------------------------------------------+

  PURPOSE : handle sAT_A and cAT_H for GPRS
*/
static BOOL cmhSM_sAT_A_H_intern ( T_ACI_CMD_SRC srcId, T_ACI_RETURN *aci_ret, SHORT mode)
{
  if ( at.rngPrms.isRng EQ TRUE )
  {
    if ( at.rngPrms.mode EQ CRING_MOD_Gprs)
    {
      *aci_ret =  automatic_response_mode > 1 ? /* modem copatibility mode possible */
                  sAT_PlusCGANS(srcId, mode, NULL, PDP_CONTEXT_CID_OMITTED): AT_FAIL;
    }
    else  /* circuit switched call */
    {
      return FALSE;
    }
  }
  else
  {
     return FALSE;
  }
  return TRUE;
}



/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                    |
| STATE   :                       ROUTINE : pdp_context_type_omitted   |
+----------------------------------------------------------------------+

  PURPOSE : check if the pdp context type is omitted.
*/
GLOBAL BOOL pdp_context_type_omitted( char *p_type )
{
  BOOL omitted = TRUE;
  char  i;

  for( i = 0; i < MAX_PDP_CONTEXT_TYPE_LEN; i++ )
  {
    if( *(p_type + i) NEQ  0 )
      omitted = FALSE;
  }

  return omitted;
}


/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                    |
| STATE   :                       ROUTINE : pdp_context_addr_omitted   |
+----------------------------------------------------------------------+

  PURPOSE : check if the pdp context apn is omitted.
*/
GLOBAL BOOL pdp_context_apn_omitted( char *p_apn )
{
  BOOL omitted = TRUE;
  int  i;

  for( i = 0; i < MAX_PDP_CONTEXT_APN_LEN; i++ )
  {
    if( *(p_apn + i) NEQ  0 )
      omitted = FALSE;
  }

  return omitted;
}


/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                    |
| STATE   :                       ROUTINE : pdp_context_addr_omitted   |
+----------------------------------------------------------------------+

  PURPOSE : check if the pdp context addr is omitted.
*/
GLOBAL BOOL pdp_context_addr_omitted( T_NAS_ip *p_addr )
{
  return  (p_addr->ctrl_ip_address EQ NAS_is_ip_not_present);
}


/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                    |
| STATE   :                       ROUTINE : pdp_context_type_valid     |
+----------------------------------------------------------------------+

  PURPOSE : check pdp context type, return false if not valid.
*/
GLOBAL BOOL pdp_context_type_valid( char *p_type )
{
  BOOL valid = FALSE;
  
  if( !strcmp( p_type, "IP") )
    valid = TRUE;

  if( !strcmp( p_type, "IPV6") )
    valid = TRUE;

  if( !strcmp( p_type, "PPP") )
  {
    /* For EDGE we are not supporting PDP_TYPE "PPP" */
    valid = FALSE;
  }

  return valid;
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                    |
| STATE   : finished              ROUTINE : pdp_context_apn_vaild      |
+----------------------------------------------------------------------+

  PURPOSE : check APN of well formed
*/
GLOBAL BOOL pdp_context_apn_valid( char *p_apn )
{
  SHORT  i      = 0;
  SHORT  length = strlen( p_apn );
  char *no_start_labels[] = {"rac","lac","sgsn"}, *start, *end, *mark;
  T_PDP_CONTEXT_APN  apn;

  strcpy( apn, p_apn );
  start = apn;

  /* change charcter of string to lower cases */
  for( i = 0; i < length; i++ )
    apn[i] = tolower( apn[i] );

  /* The Network Identifier shall not start with this labels */
  for( i = 0; i < 3; i++ )
  {
    if( apn EQ strstr(apn, no_start_labels[i]) )
      return FALSE;
  }

  /* the Wild Card APN */
  if ( length EQ 1 AND *start EQ '*' )
    return TRUE;

  /* the APN with no name */
  if ( length EQ 1 AND *start EQ /*lint -e(743)*/ '\x0ff' )
    return TRUE;

  /* Oporater Identifer is optional and the Network Identifer */
  mark = strrchr( apn, '.' );
  if( mark )
    if( strstr(mark + 1, "gprs") )
    {
      /* APN Operator Identifier contained (optional) */
      if( length < 18 )
        return FALSE;
      
      mark = start + length - 18; /* start of the APN Operator Identifier */
      /* check APN Operator Identifier: "mncXXX.mccXXX.gprs" */
      if( mark NEQ strstr(mark, "mnc") )
        return FALSE;
      
      if ( mark + 6 NEQ strstr(mark, ".mcc") )
        return FALSE;
      strtol(mark + 3, &end, 10);
      
      if ( end NEQ mark + 6 )
        return FALSE;
      strtol(mark + 10, &end, 10);
      
      if ( end NEQ mark + 13 )
        return FALSE;
      /* check character between APN Network Identifier and the Operator Identifer */
      mark--;
      
      if ( *mark NEQ '.' )
        return FALSE;
      /* set stop mark */
      *mark = 0;
    }
    else
      mark = 0;

  /* check APN Network Identifier */

  /* shall not end in ".gprs" */
  end = strrchr(apn, '.');
  if ( end )
    if ( strstr(end + 1, "gprs") )
      return FALSE;

  /* parse all labels */
  while ( *start )
  {
    /* in first at least one Label */
    while ( (*start >= 'a' AND *start <= 'z') OR (*start >= '0' AND *start <= '9') OR *start EQ '-' )
      start ++;

    /* next Label or nothing */
    if ( *start EQ '.' )
      start ++;
    else
      if ( *start NEQ 0)
        return FALSE;
  }

  /* The APN Network Identifier shall have a maximum length of 63 octets. */
  if ( start - apn > 63 )
    return FALSE;

  /* clear stop mark */
  if ( mark )
    *mark = '.';

  return TRUE;
}



/*
+----------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                    |
| STATE   :                       ROUTINE : pdp_context_addr_valid     |
+----------------------------------------------------------------------+

  PURPOSE : check pdp context address, return false if not valid.
*/
GLOBAL BOOL pdp_context_addr_valid( T_NAS_ip * p_addr )
{
  BOOL valid = TRUE;
  int  i     = 0;

  if (p_addr->ctrl_ip_address EQ NAS_is_ip_not_present)
    return FALSE;
  
  if (p_addr->ctrl_ip_address EQ NAS_is_ipv4)
  {
    if (( p_addr->ip_address.ipv4_addr.a4[0] EQ 0 )   OR   //According to RFC3330 first digit must not be ZERO
        ( p_addr->ip_address.ipv4_addr.a4[0] EQ 127 ) OR   //According to RFC3330 first digit 127 means LOOPBACK
        ((p_addr->ip_address.ipv4_addr.a4[0] >= 224 ) AND (p_addr->ip_address.ipv4_addr.a4[0] < 240)) )  //According to RFC3330 224.0.0.0/4 is reserved for multicast
    {
      valid = FALSE;
    }
  }
  else
  {
    switch( p_addr->ip_address.ipv6_addr.a6[0] )
    {
      case 0:
        if( p_addr->ip_address.ipv6_addr.a6[11] EQ 0 AND p_addr->ip_address.ipv6_addr.a6[12] NEQ  0 
          AND p_addr->ip_address.ipv6_addr.a6[12] NEQ  127 AND p_addr->ip_address.ipv6_addr.a6[12] NEQ  255 )
        {
          /* this could be an ip v4 address embedded in ip v6 */
          break;
        }
            
        for(i = 1; i < PDP_CONTEXT_ADDR_LEN_MAX; i++)
        {
          if( p_addr->ip_address.ipv6_addr.a6[i] EQ 0 )
            valid = FALSE;
        }
        break;

      case 127: /* local host address           */
        valid = FALSE;
        break;
      
      case 254: /* invalid for an ip v6 address, the rest of the address must be '0' */
        for(i = 4; i < PDP_CONTEXT_ADDR_LEN_MAX; i++)
        {
          if( p_addr->ip_address.ipv6_addr.a6[i] NEQ  0 )
            valid = FALSE;
        }
        break;

//      case 255: /* omitted 255.255...255.255 */
//        for( i = 1; i < PDP_CONTEXT_ADDR_LEN_MAX; i++ )
//        {
//          if( p_addr->ip_address.ipv6_addr.a6[i] NEQ  255 )
//          {
//            valid = FALSE;
//          }
//        }
//        break;

      default:
        /* the address is valid */
        break;

    }
  }
  return valid;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_transform_pdp_type |
+--------------------------------------------------------------------+

  PURPOSE : transform pdp_type
*/
GLOBAL USHORT cmhSM_transform_pdp_type( char *pdp_type_str )
{
  T_GACI_PDP_TYPE pdp_type = PDP_T_NONE;

  if( !strcmp(pdp_type_str, "IP") )
    pdp_type = PDP_T_IP;

  if( !strcmp(pdp_type_str, "IPV6") )
    pdp_type = PDP_T_IPV6;

  if( !strcmp(pdp_type_str, "PPP") )
    pdp_type = PDP_T_PPP;

  return pdp_type;
}


/*
+-------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                       |
| STATE   :                       ROUTINE : pdp_context_get_state_for_cid |
+-------------------------------------------------------------------------+

  PURPOSE : return the PDP context state for the given <cid>.
*/
GLOBAL T_PDP_CONTEXT_STATE pdp_context_get_state_for_cid( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_PDP_CONTEXT_STATE     pdp_context_state  = PDP_CONTEXT_STATE_INVALID;
  
  TRACE_FUNCTION("pdp_context_get_state_from_cid()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  	pdp_context_state = p_pdp_context_node->internal_data.state;

  return pdp_context_state;
}


/*
+-------------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMF                       |
| STATE   :                       ROUTINE : pdp_context_cid_used_by_other |
+-------------------------------------------------------------------------+

  PURPOSE : return true if the context id is used 
*/
GLOBAL BOOL pdp_context_cid_used_by_other( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION("pdp_context_cid_used_by_other()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  while( p_pdp_context_node )
  {
    if( p_pdp_context_node->attributes.p_cid < PDP_CONTEXT_CID_MIN AND 
        p_pdp_context_node->attributes.p_cid > PDP_CONTEXT_CID_MAX )
    {
      return TRUE;
    }
    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  return FALSE;  
}

GLOBAL void set_state_over_cid ( U8 cid, T_PDP_CONTEXT_STATE state )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
  	p_pdp_context_node->internal_data.state = state;
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : get_state_over_cid        |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL T_PDP_CONTEXT_STATE get_state_over_cid( U8 cid )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION("get_state_over_cid()");

  if ( (cid >= 1) AND (cid <= PDP_CONTEXT_CID_MAX) )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( p_pdp_context_node )
    {
      return p_pdp_context_node->internal_data.state;
    }
  }

  TRACE_EVENT("invalid cid detected!");
  return PDP_CONTEXT_STATE_INVALID;

}


/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : get_state_working_cid     |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL T_PDP_CONTEXT_STATE get_state_working_cid( void )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    return p_pdp_context_node->internal_data.state;
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function get_state_working_cid" );
    return PDP_CONTEXT_STATE_INVALID;
  }
 
}


/*
+------------------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                            |
| STATE   :                       ROUTINE : set_conn_param_on_working_cids |
+------------------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL  void  set_conn_param_on_working_cid ( UBYTE owner, T_DTI_ENTITY_ID entity_id )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  TRACE_FUNCTION("set_conn_param_on_working_cid()");
  
  p_pdp_context_node = pdp_context_find_node_from_cid(work_cids[cid_pointer]);
  
  if( p_pdp_context_node )
  {
    p_pdp_context_node->internal_data.owner = (T_ACI_CMD_SRC)owner;
    p_pdp_context_node->internal_data.entity_id = entity_id;
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function set_conn_param_on_working_cid  " );
    return;
  }
}



/*
+------------------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                            |
| STATE   :                       ROUTINE : set_conn_param_on_all_working_cids |
+------------------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL  void  set_conn_param_on_all_working_cids ( UBYTE owner, T_DTI_ENTITY_ID entity_id )
{
  U8 *pcid = &(work_cids[cid_pointer]);
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("set_conn_param_on_all_working_cids()");

  while(INVALID_CID NEQ *pcid) {

    p_pdp_context_node = pdp_context_find_node_from_cid((U8)*pcid);
    
    if( p_pdp_context_node )
    {
      p_pdp_context_node->internal_data.owner = (T_ACI_CMD_SRC)owner;
      p_pdp_context_node->internal_data.entity_id = entity_id;
    }
    pcid ++;
  }
}

/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : set_state_working_cid     |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL void set_state_working_cid( T_PDP_CONTEXT_STATE c_state )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION("set_state_working_cid()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
    p_pdp_context_node->internal_data.state = c_state;
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, invalid cid, in function set_state_working_cid" );
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : get_state_over_nsapi_set  |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL T_PDP_CONTEXT_STATE get_state_over_nsapi_set ( USHORT *nsapi_set, U8 *cid )
{
  USHORT nsapi = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("get_state_over_nsapi_set()");
  while( nsapi < SMH_LAST_FREE_NSAPIS AND !((*nsapi_set >> nsapi) & 1) )
    nsapi++;

  if( !(*nsapi_set & ( 1 << nsapi )) )
  {
    return PDP_CONTEXT_STATE_INVALID;
  }

  TRACE_EVENT_P1("NSAPI: %d", nsapi);

  TRACE_EVENT_P1("NSAPI: %4d", nsapi);

  *nsapi_set &= ~( 1U << nsapi );

  p_pdp_context_node = pdp_context_find_node_from_cid( NSAPI_TO_CID( nsapi ) );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "ERROR: PDP context not found, invalid cid, in function get_state_over_nsapi_set" );
    return PDP_CONTEXT_STATE_INVALID;
  }

  *cid = p_pdp_context_node->cid;

  return get_state_over_cid( *cid );

}


/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : cmhSM_Give_nsapi_set      |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL USHORT cmhSM_Give_nsapi_set ( U8 cid )
{
  return (1 << CID_TO_NSAPI(cid) );
}


/*
+---------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                   |
| STATE   :                       ROUTINE : get_owner_over_cid        |
+---------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL T_ACI_CAL_OWN get_owner_over_cid( U8 cid )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    return (T_ACI_CAL_OWN)p_pdp_context_node->internal_data.owner;
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function get_owner_over_cid" );
    return CAL_OWN_NONE;
  }
}


/*
+--------------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                        |
| STATE   :                       ROUTINE : cmhGPPP_send_establish_request |
+--------------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL void cmhGPPP_send_establish_request ( UBYTE peer, UBYTE prot )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
//  T_PPP_ESTABLISH_REQ     est_req;

  PALLOC (ppp_establish_req, PPP_ESTABLISH_REQ);

  TRACE_FUNCTION( "cmhGPPP_send_establish_request" );
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );  
  if( p_pdp_context_node )
  {
  
    if(!strcmp(p_pdp_context_node->attributes.pdp_type, "PPP"))
      ppp_establish_req->mode = PPP_TRANSPARENT;
    else
      ppp_establish_req->mode = PPP_SERVER;

    ppp_establish_req->mru  = PPP_MRU_DEFAULT;
    ppp_establish_req->ap   = gpppShrdPrm.ppp_authentication_protocol;
    ppp_establish_req->accm = gpppShrdPrm.accm;
    ppp_establish_req->rt   = gpppShrdPrm.restart_timer;
    ppp_establish_req->mc   = gpppShrdPrm.max_configure;
    ppp_establish_req->mt   = gpppShrdPrm.max_terminate;
    ppp_establish_req->mf   = gpppShrdPrm.max_failure;
    ppp_establish_req->ip  = p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4[0] << 24;
    ppp_establish_req->ip += p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4[1] << 16;
    ppp_establish_req->ip += p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4[2] <<  8;
    ppp_establish_req->ip += p_pdp_context_node->attributes.pdp_addr.ip_address.ipv4_addr.a4[3];

    ppp_establish_req->peer_direction = DTI_CHANNEL_TO_LOWER_LAYER;
    ppp_establish_req->prot_direction = DTI_CHANNEL_TO_HIGHER_LAYER;

    ppp_establish_req->peer_link_id = p_pdp_context_node->internal_data.link_id_uart;
    ppp_establish_req->prot_link_id = p_pdp_context_node->internal_data.link_id;

    
#ifdef _SIMULATION_
    memset (ppp_establish_req->peer_channel.peer_entity,         0, CHANNEL_NAME_LENGTH);
    memset (ppp_establish_req->protocol_channel.protocol_entity, 0, CHANNEL_NAME_LENGTH);
#endif  /* _SIMULATION_ */

  strcpy ( (char *) ppp_establish_req->protocol_channel.protocol_entity, SNDCP_NAME);

    switch (peer)
    {
#ifdef BT_ADAPTER
      case DTI_ENTITY_BLUETOOTH:
        strcpy ( (char *)(&(ppp_establish_req->peer_channel)), BTI_NAME);
        break;
#endif
      case DTI_ENTITY_UART:
        strcpy ( (char *)(&(ppp_establish_req->peer_channel)), UART_NAME);
        break;

#ifdef FF_PSI      
      case DTI_ENTITY_PSI:
        strcpy ( (char *)(&(ppp_establish_req->peer_channel)), PSI_NAME);		  
        break;
#endif /*FF_PSI*/

      case DTI_ENTITY_AAA:
        strcpy ( (char *)(&(ppp_establish_req->peer_channel)), RIV_NAME);
        break;

    default:
      TRACE_ERROR ("[cmhGPPP_send_establish_request()] Unexpected peer!");
      return;
  }

    psaGPPP_Establish ( ppp_establish_req );

    switch (get_state_working_cid())
    {
      case PDP_CONTEXT_STATE_ACTIVATED:
        set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1 );
        break;
      default:
        set_state_working_cid( PDP_CONTEXT_STATE_ESTABLISH_1 );
        break;
    }

  }
  else
  {
    TRACE_ERROR( "PDP context not found, in function cmhGPPP_send_establish_request" );
  }
}


/*
+-----------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF               |
| STATE   :                       ROUTINE : cmhSM_cgerep_buffer   |
+-----------------------------------------------------------------+
  PURPOSE : 
*/

GLOBAL void cmhSM_cgerep_buffer ( void )
{

  switch (sm_cgerep_bfr)
  {
    case CGEREP_BFR_CLEAR:
      memset(gprs_event_buffer, 0, sizeof(T_CGERP_EVENT_BUFFER) * GPRS_EVENT_REPORTING_BUFFER_SIZE);
     break;
    case CGEREP_BFR_FLUSH:
      if ( uart_is_mt_te_link EQ FALSE)
      {
        while ( gprs_eb_oldest_p NEQ gprs_eb_current_p )
        {
              R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)sm_cgerep_srcId )
              ( gprs_event_buffer[gprs_eb_oldest_p].event, gprs_event_buffer[gprs_eb_oldest_p].parm );

          gprs_eb_oldest_p++;
        }
      }
      break;
    case CGEREP_BFR_OMITTED:
    case CGEREP_BFR_INVALID:
    default:
      break;
  }
}


/*
+-----------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF               |
| STATE   :                       ROUTINE : cmhSM_save_event      |
+-----------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL void cmhSM_save_event( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param )
{

  /* save event */
  gprs_event_buffer[gprs_eb_current_p].event = event;
  if (param)
    memcpy (&gprs_event_buffer[gprs_eb_current_p].parm, param, sizeof(T_CGEREP_EVENT_REP_PARAM));

  /* is buffer full */
  if ( gprs_eb_oldest_p EQ gprs_eb_current_p )
    gprs_eb_oldest_p = -1;

  /* new current pointer */
  gprs_eb_current_p++;
  if ( gprs_eb_current_p EQ GPRS_EVENT_REPORTING_BUFFER_SIZE )
    gprs_eb_current_p = 0;

  /* if buffer full correct pointer to oldest event */
  if ( gprs_eb_oldest_p EQ -1 )
    gprs_eb_oldest_p = gprs_eb_current_p;

}


/*
+-----------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF               |
| STATE   :                       ROUTINE : cmhSM_set_sms_service |
+-----------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL void cmhSM_set_sms_service( T_CGSMS_SERVICE service )
{

  {
    PALLOC (mnsms_mo_serv_req, MNSMS_MO_SERV_REQ);

    /* fill in primitive parameter: command request */
    mnsms_mo_serv_req -> mo_sms_serv = (UBYTE) service;

    PSENDX (SMS, mnsms_mo_serv_req);
  }

  m_service = service;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                      |
| STATE   : finnished        ROUTINE : cmhSM_sms_service_changed    |
+-------------------------------------------------------------------+

  PURPOSE : indicates a network initiated PDP context modification

*/
GLOBAL void cmhSM_sms_service_changed ( UBYTE service )
{

  TRACE_FUNCTION ("cmhSM_sms_service_changed()");

/*
 *-------------------------------------------------------------------
 * check for command context
 *-------------------------------------------------------------------
 */
  if ( smEntStat.curCmd EQ AT_CMD_CGSMS )
  {
    if ( m_service EQ service )
    {
      sm_cgsms_service = m_service;
      R_AT( RAT_OK, smEntStat.entOwn ) ( smEntStat.curCmd );
    }
    else
    {
      R_AT( RAT_CME, smEntStat.entOwn ) ( smEntStat.curCmd, CME_ERR_Unknown );
      /* log result */
      cmh_logRslt ( smEntStat.entOwn, RAT_CME, smEntStat.curCmd,
                             -1, BS_SPEED_NotPresent, CME_ERR_Unknown );
    }

    smEntStat.curCmd = AT_CMD_NONE;
  }
}


/*
+-----------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF               |
| STATE   :                       ROUTINE : cmhSM_GprsAttached    |
+-----------------------------------------------------------------+

  PURPOSE    : Handling of changed GMM attach state.
  PARAMETERS : state - TRUE is GPRS attached.
                       FALSE is GPRS detached.
*/

GLOBAL void cmhSM_GprsAttached( T_GPRS_ATTACH_STATE state )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  TRACE_FUNCTION ("cmhSM_GprsAttached()");

  /*
   * Do nothing if no context action is in progress (no cmd is running).
   */
  if (work_cids[cid_pointer] EQ PDP_CONTEXT_CID_INVALID)
    return;


  if ( state EQ GPRS_ATTACH )
  { /* mobile is attached */
    switch ( get_state_working_cid( ) )
    {
      case PDP_CONTEXT_STATE_ATTACHING:
        /* The first cid in work_cids should continue activation after the attach */
        if (gpppEntStat.curCmd EQ AT_CMD_CGDATA)
        {
          /* Continue establishing the data link context */
          cmhSM_data_link_context();
          break;
        }
        else if (smEntStat.curCmd EQ AT_CMD_CGACT)
        {
          /* Continue activating the context */
          cmhSM_activate_context();
          break;
        }
      default:
        /* Do nothing since no context is forcing the attach. */
        break;
    }
  }
  else
  {
    /* attach failed or the network has indicated detach. */

    p_pdp_context_node = pdp_context_find_node_from_cid((U8)(work_cids[cid_pointer]));
    if( p_pdp_context_node NEQ  NULL )
    {
      switch ( get_state_working_cid() )
      {
        case PDP_CONTEXT_STATE_ATTACHING:
          /* The attach was rejected. Set state back to defined and return no carrier */

          if (smEntStat.curCmd EQ AT_CMD_CGACT)
          {
            p_pdp_context_node->internal_data.owner = smEntStat.entOwn;
          }
          else
          {
            p_pdp_context_node->internal_data.owner = gpppEntStat.entOwn;
          }

          if (gpppEntStat.curCmd EQ AT_CMD_CGDATA)
          {
            gaci_RAT_caller ( RAT_NO_CARRIER, work_cids[cid_pointer], AT_CMD_DATA, 0 );
          }
          else if (smEntStat.curCmd EQ AT_CMD_CGACT)
          {
            gaci_RAT_caller ( RAT_CME, work_cids[cid_pointer], AT_CMD_CGACT, CME_ERR_GPRSUnspec );
          }

          set_state_working_cid( PDP_CONTEXT_STATE_DEFINED );

          gpppEntStat.curCmd = AT_CMD_NONE;
          smEntStat.curCmd = AT_CMD_NONE;
          work_cids[0] = PDP_CONTEXT_CID_INVALID;
          cid_pointer  = 0;

          break;
        case PDP_CONTEXT_STATE_ESTABLISH_1:
         /*
          * Context not activated towards SM, but PPP has to be terminated.
          */
          cmhSM_deactivateAContext( smEntStat.entOwn, work_cids[cid_pointer] );
        
          gpppEntStat.curCmd = AT_CMD_NONE;
          gpppEntStat.entOwn = CMD_SRC_NONE;
          break;
        default:
          /* Do nothing. Context is deactivated from SM */
          break;
      }
    }
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT :                       MODULE  : CMH_SMF                 |
| STATE   :                       ROUTINE : cmhSM_activate_context  |
+-------------------------------------------------------------------+

  PURPOSE : Activates a context without user plane.
*/

GLOBAL void cmhSM_activate_context(void)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8 hcomp;
  U8 dcomp;

  TRACE_FUNCTION ("cmhSM_activate_context()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( !p_pdp_context_node )
  {
    TRACE_ERROR( "PDP context not found, in function cmhSM_activate_context" );
    return;
  }


  /* hcomp value being changed according to spec 27.007 */
  hcomp = cmhSM_Get_h_comp() ;
  dcomp = cmhSM_Get_d_comp() ;

  /*
   * Activate the primary or secondary context 
   */
  if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_PRIMARY )
  {
    psaSM_smreg_pdp_activate_req( work_cids[cid_pointer], 
                                  hcomp,
                                  dcomp);
  }
#ifdef REL99
  else if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_SECONDARY )
  {
    psaSM_smreg_pdp_activate_sec_req( work_cids[cid_pointer] );
  }
#endif

  set_state_working_cid( PDP_CONTEXT_STATE_ACTIVATING );    
}


#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : cmh_smf              |
| STATE   : initial                     ROUTINE : cmhSM_IP_activate_cb |
+----------------------------------------------------------------------+

  PURPOSE : callback function for WAP over GPRS.
            While handling the command ATD*98# in function atGD()
            the function psaTCPIP_Activate() is called for WAP handling.
            psaTCPIP_Activate() has this function as parameter for
            call backs.
*/
void cmhSM_IP_activate_cb(T_ACI_RETURN result)
{
  U8 cid = gaci_get_cid_over_dti_id(wap_dti_id);
  TRACE_FUNCTION("cmhSM_IP_activate_cb");
  TRACE_EVENT_P1("wap_state: %s",wap_state_to_string(wap_state));
#ifndef FF_SAT_E
  ACI_ASSERT(DTI_DTI_ID_NOTPRESENT NEQ wap_dti_id);
  ACI_ASSERT(cid >= GPRS_CID_1 AND cid < GPRS_CID_INVALID);
  ACI_ASSERT(result NEQ AT_FAIL);
#endif /* not FF_SAT_E */

  if ( result EQ AT_FAIL )
  {
    /* IP activation failed. */
    TRACE_EVENT("UDP/TCP/IP activation/configuration returned AT_FAIL");
    
    if (DTI_DTI_ID_NOTPRESENT NEQ wap_dti_id)
    {
      dti_cntrl_close_dpath_from_dti_id (wap_dti_id);
      cmhSM_disconnect_cid(cid, GC_TYPE_WAP );
    }

    /* reset work_cids */
    set_state_over_cid(cid, PDP_CONTEXT_STATE_DEFINED);

    sAT_PercentWAP((T_ACI_CMD_SRC)smShrdPrm.owner, 0);
    smEntStat.curCmd = AT_CMD_NONE;
    
    dti_cntrl_erase_entry(wap_dti_id);
    cmdCmeError(CME_ERR_Unknown);

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
    if( cmhSAT_OpChnGPRSPend( PDP_CONTEXT_CID_INVALID, OPCH_EST_REQ ))
    {
      cmhSAT_OpChnUDPDeactGprs();
    }
#endif  /* SIM_TOOLKIT */
    return;
  }

  switch(wap_state)
  {
    // in case CO_UDP_IP and FF_GPF_TCPIP is defined this is a fall through case
    UDPIP_STATEMENT(case IPA_Activated:)
    GPF_TCPIP_STATEMENT(case TCPIP_Activation:)

      if(is_gpf_tcpip_call()) {
        GPF_TCPIP_STATEMENT(set_conn_param_on_working_cid( 
                            (UBYTE)smEntStat.entOwn, DTI_ENTITY_TCPIP));
      }
      else {
        UDPIP_STATEMENT(set_conn_param_on_working_cid( 
                        (UBYTE)smEntStat.entOwn, DTI_ENTITY_IP));
      }
      cmhSM_activate_context_For_WAP();
      return;

    UDPIP_STATEMENT(case UDPA_Configurated:)
    GPF_TCPIP_STATEMENT(case TCPIP_Configurated:)

      //smEntStat.curCmd = AT_CMD_NONE;
#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
      if( cmhSAT_OpChnGPRSPend( PDP_CONTEXT_CID_INVALID, OPCH_EST_REQ ))
      {
        cmhSAT_OpChnUDPConfGprs();
      }
      else
#endif  /* SIM_TOOLKIT */
      {
        R_AT ( RAT_CONNECT, smEntStat.entOwn )( AT_CMD_CGACT, -1, wapId, FALSE );
      }
      smEntStat.curCmd = AT_CMD_NONE;
      smEntStat.entOwn = CMD_SRC_NONE;
#ifdef FF_GPF_TCPIP
      if(is_gpf_tcpip_call())
      {
        T_DCM_STATUS_IND_MSG msg;
        msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
        dcm_send_message(msg, DCM_SUB_WAIT_CGACT_CNF);
      }
#endif
      break;

    UDPIP_STATEMENT(case IPA_Deactivated:)
    GPF_TCPIP_STATEMENT(case TCPIP_Deactivated:)

      TRACE_EVENT_P1("cmhSM_IP_activate_cb, no connection, dti_id = %d", wap_dti_id);

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
      if( cmhSAT_OpChnGPRSPend( PDP_CONTEXT_CID_INVALID, OPCH_NONE ))
      {
        cmhSAT_OpChnUDPDeactGprs();
      }
      else
#endif  /* SIM_TOOLKIT */
      {
#ifdef FF_GPF_TCPIP 
        /* Don't send NO CARRIER if it is TCPIP call */
        if (!is_gpf_tcpip_call())
#endif

        /* TC GACIWAP232 */
        gaci_RAT_caller ( RAT_NO_CARRIER, cid, AT_CMD_CGACT, 0 );
      }
      dti_cntrl_close_dpath_from_dti_id (wap_dti_id);
      cmhSM_connection_down(wap_dti_id);
      /*Decrease the count for the corresponding link if it is a TCPIP call */
#ifdef FF_GPF_TCPIP
      if (is_gpf_tcpip_call())
      {
         GPF_TCPIP_STATEMENT(srcc_delete_count(SRCC_TCPIP_SNDCP_LINK ));
      }
      else
#endif
      {

      cmhSM_disconnect_cid(cid, GC_TYPE_WAP );
    }
      sAT_PercentWAP((T_ACI_CMD_SRC)smShrdPrm.owner, 0);
      if(work_cids[cid_pointer] EQ cid)
      {
        smEntStat.curCmd = AT_CMD_NONE;
        *work_cids = 0;
        cid_pointer = 0;
      }
#ifdef FF_GPF_TCPIP
      if(is_gpf_tcpip_call())
      {
        T_DCM_STATUS_IND_MSG msg;
        msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
        dcm_send_message(msg, DCM_SUB_WAIT_CGDEACT_CNF);
      }
#endif
      break;

    default:
      TRACE_EVENT("Unexpected wap state in cmhSM_IP_activate_cb()");
      if(is_gpf_tcpip_call()) {
        GPF_TCPIP_STATEMENT(srcc_delete_count(SRCC_TCPIP_SNDCP_LINK ));
      }
      else {
        UDPIP_STATEMENT(srcc_delete_count(SRCC_IP_SNDCP_LINK ));
      }
      dti_cntrl_erase_entry(wap_dti_id);
      sAT_PercentWAP((T_ACI_CMD_SRC)smShrdPrm.owner, 0);
      smEntStat.curCmd = AT_CMD_NONE;

#if defined (SIM_TOOLKIT) AND defined (FF_SAT_E)
      if( cmhSAT_OpChnGPRSPend( PDP_CONTEXT_CID_INVALID, OPCH_NONE ))
      {
        cmhSAT_OpChnUDPDeactGprs();
      }
      else
#endif  /* SIM_TOOLKIT */
      {
        cmdCmeError(CME_ERR_Unknown);
      }
      break;
  }
  return;
}

GLOBAL T_ACI_RETURN cmhSM_activate_context_For_WAP(void)
{
  TRACE_FUNCTION ("cmhSM_activate_context_For_WAP()");

  if( AT_CMPL EQ cmhGMM_attach_if_necessary( smEntStat.entOwn, AT_CMD_CGDATA ) )
  {
    cmhSM_connect_working_cid();    
  }
  else  /* AT_EXCT -> class BX class change requested (NOMIII) */
  {
    /* For TC ACTSAT 510 
       Activating the context for WAP 
    */
    gpppEntStat.curCmd = AT_CMD_CGDATA;
    set_state_working_cid( PDP_CONTEXT_STATE_ATTACHING );  
  }
  return (AT_EXCT);
}
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : cmh_smf              |
| STATE   : initial                     ROUTINE : cmhSM_IP_Enable      |
+----------------------------------------------------------------------+

  PURPOSE : enables IP dti connection.
*/
GLOBAL void cmhSM_IP_Enable ( T_DTI_CONN_LINK_ID link_id)
{
  TRACE_FUNCTION("cmhSM_IP_Enable");

#ifdef _SIMULATION_
  cmhSM_connect_context ( gaci_get_cid_over_link_id( link_id ),
                          DTI_ENTITY_IP );
#else  /* _SIMULATION_ */
  cmhSM_connect_context ( gaci_get_cid_over_link_id( link_id ),
                          DTI_ENTITY_IP );
#endif /* _SIMULATION_ */
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : cmh_smf              |
| STATE   : initial                     ROUTINE : cmhSM_IP_Disable     |
+----------------------------------------------------------------------+

  PURPOSE : disables IP dti connection.
*/
GLOBAL void cmhSM_IP_Disable ()
{
  TRACE_FUNCTION("cmhSM_IP_Disable");
}

#endif /* WAP OR FF_GPF_TCPIP OR SAT E */

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                      |
| STATE   : finnished        ROUTINE : cmhSM_data_link_context      |
+-------------------------------------------------------------------+
*/

GLOBAL void cmhSM_data_link_context(void)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("cmhSM_data_link_context()");

  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );
  if( p_pdp_context_node )
  {
#if defined FF_WAP
    if(!Wap_Call)
    {
      R_AT( RAT_CONNECT, p_pdp_context_node->internal_data.owner )
        ( AT_CMD_CGDATA, 0, 0, FALSE );
    }
#else
    R_AT( RAT_CONNECT, p_pdp_context_node->internal_data.owner )
      ( AT_CMD_CGDATA, 0, 0, FALSE );
#endif /* WAP OR SAT E */
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_data_link_context" ); 
  }

    

  /* log result */
  if( p_pdp_context_node )
    cmh_logRslt ( p_pdp_context_node->internal_data.owner,
                RAT_CONNECT, AT_CMD_CGDATA, -1, BS_SPEED_NotPresent,CME_ERR_NotPresent );

  cmhSM_connect_working_cid();

}

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                      |
| STATE   : finnished        ROUTINE : cmhSM_next_work_cid          |
+-------------------------------------------------------------------+

  PURPOSE : start the next context activation if requested

*/
GLOBAL  BOOL    cmhSM_next_work_cid ( T_ACI_AT_CMD curCmd )
{

  TRACE_EVENT_P1("cmhSM_next_work_cid, cid_pointer: %d", cid_pointer);

  cid_pointer ++;

  if ( work_cids[cid_pointer] EQ PDP_CONTEXT_CID_INVALID )
  {
    smEntStat.curCmd = AT_CMD_NONE;
    gpppEntStat.curCmd = AT_CMD_NONE;

    cid_pointer  = 0;
    memset( work_cids, PDP_CONTEXT_CID_INVALID, sizeof(work_cids) ); // *work_cids = 0;

    return FALSE;
  }

  switch ( curCmd )
  {
    case AT_CMD_CGDATA:
      cmhSM_data_link_context();
      break;
    case AT_CMD_CGACT:
      cmhSM_activate_context();
      break;
    default:
      cid_pointer  = 0;
      *work_cids = 0;
      return FALSE;
  }
  return TRUE;
}



/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                      |
| STATE   : finnished        ROUTINE : cmhSM_make_active_cid_list   |
+-------------------------------------------------------------------+

  PURPOSE : moves cids into work_cids list, verifies cid range,
            checks for duplicated cids and checks that the context
            is active. If no cids are specified, all active cids are
            put into the work_cids list.
            Used by the sAT_PlusCGCMOD command.
  RETURN  : Number of cids in work_cids list.

*/
GLOBAL SHORT cmhSM_make_active_cid_list ( T_ACI_CMD_SRC srcId, U8 *cids )
{
  U8 i, j;

  cid_pointer = 0;
  if( cids[0] NEQ  PDP_CONTEXT_CID_INVALID )
  {
    /* A number of cids are listed in *cids */
    i = 0;
    while( (i<PDP_CONTEXT_CID_MAX) AND (cids[i] NEQ  PDP_CONTEXT_CID_INVALID) )
    {
      /* Check for duplicated cids (no check the first time). */
      for (j=0; j<i; j++)
        if (work_cids[j] EQ cids[i]) return 0;

      /* Check for valid cid range */
      if( (cids[i] < PDP_CONTEXT_CID_MIN) OR (cids[i] > PDP_CONTEXT_CID_MAX) )
        return 0;

      /* Check that context state for cid is Active */
      if( (get_state_over_cid( (U8)(i+1) ) NEQ  PDP_CONTEXT_STATE_ACTIVATED) AND 
          (get_state_over_cid( (U8)(i+1) ) NEQ  PDP_CONTEXT_STATE_DATA_LINK) ) 
        return 0;

      /* This cid is OK: store in works_cids list */
      work_cids[i] = cids[i];
      i++;
    }
    work_cids[i] = PDP_CONTEXT_CID_INVALID;
    /* Success!!! */
    return i;
  }
  else
  {
    /* No cids are listed in *cids. Apply cids for all Active context (check not necessary). */
    j = 0;
      i = 1;
    while (i<=PDP_CONTEXT_CID_MAX)
    {
      if( (get_state_over_cid(i) EQ PDP_CONTEXT_STATE_ACTIVATED) OR 
          (get_state_over_cid(i) EQ PDP_CONTEXT_STATE_DATA_LINK) )
      {
        work_cids[j] = i;
        j++;
      }
      i++;
    }
    work_cids[j] = PDP_CONTEXT_CID_INVALID;
    return j;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : UMTS             MODULE  : CMH_SMR                      |
| STATE   : finnished        ROUTINE : cmhSM_Set_pdp_type           |
+-------------------------------------------------------------------+

  PURPOSE : Set the pdp_type for a given cid.
  RETURN  : -
*/
GLOBAL void cmhSM_Set_pdp_type( U8 cid,  char *pdp_type )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  if( p_pdp_context_node )
  {
    memcpy( p_pdp_context_node->attributes.pdp_type, pdp_type, sizeof(T_PDP_TYPE) );
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_Set_pdp_type");
  }

}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_get_pdp_addr_for_CGPADDR  |
+----------------------------------------------------------------------+

  PURPOSE : return the PDP_address to one cid for the GPRS CGPADDR AT command

*/
GLOBAL U8 cmhSM_get_pdp_addr_for_CGPADDR( U8 cid, T_NAS_ip * pdp_adress )
{
  T_PDP_CONTEXT_STATE     c_state;      /* state of context */
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );

  if( p_pdp_context_node )
  {

    c_state = get_state_over_cid( cid );

    if ( c_state EQ PDP_CONTEXT_STATE_INVALID )
    {
      //*pdp_adress = 0;
      pdp_adress->ctrl_ip_address = NAS_is_ip_not_present;
      return PDP_CONTEXT_CID_INVALID;
    }

    if ( c_state EQ PDP_CONTEXT_STATE_ACTIVATED OR c_state EQ PDP_CONTEXT_STATE_DATA_LINK )
    {
      memcpy(pdp_adress, &(p_pdp_context_node->internal_data.pdp_address_allocated), sizeof(T_NAS_ip));
    }
    else
    {
      memcpy(pdp_adress, &(p_pdp_context_node->attributes.pdp_addr), sizeof(T_NAS_ip));
    }

  return cid;
  }
  else
  {
    pdp_adress->ctrl_ip_address = NAS_is_ip_not_present;
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_get_pdp_addr_for_CGPADDR" );
    return PDP_CONTEXT_CID_INVALID;
  }
  
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_is_smreg_ti_used          |
+----------------------------------------------------------------------+

  PURPOSE : handle for used ti

*/
GLOBAL BOOL cmhSM_is_smreg_ti_used ( U8 ti, U8 *cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_smreg_ti( ti );
  if( p_pdp_context_node )
  {
    psaSM_PDP_Deactivate( (USHORT) (1 << CID_TO_NSAPI(p_pdp_context_node->cid) ), PS_REL_IND_YES );
    psaGPPP_Terminate( PPP_LOWER_LAYER_UP );

    call_waits_in_table = TRUE;
    set_state_over_cid( p_pdp_context_node->cid, PDP_CONTEXT_STATE_REACTIVATION_1 );
    *cid = p_pdp_context_node->cid;
    return TRUE;
  }

  return FALSE;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_context_reactivation      |
+----------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void cmhSM_context_reactivation ( void )
{
  T_CGEREP_EVENT_REP_PARAM  event;
  T_SMREG_PDP_ACTIVATE_IND *sm_ind;
  SHORT i = 0;

  if ( call_waits_in_table EQ TRUE )
  {
    call_waits_in_table = FALSE;
   /*
    *   GPRS event reporting
    */
    sm_ind = &gprs_call_table[current_gprs_ct_index].sm_ind;

    cmhSM_pdp_typ_to_string(sm_ind->pdp_type, (char*) &event.act.pdp_type);
    memcpy(&event.act.pdp_addr, &sm_ind->ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR);
    event.act.cid      = gprs_call_table[current_gprs_ct_index].cid;
    for( i = 0 ; i < CMD_SRC_MAX; i++ )
    {
      R_AT( RAT_CRING,(T_ACI_CMD_SRC)i )  ( CRING_MOD_Gprs, CRING_SERV_TYP_GPRS, CRING_SERV_TYP_NotPresent );
      R_AT( RAT_CGEREP, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_REACT, &event );
      R_AT( RAT_P_CGEV, (T_ACI_CMD_SRC)i ) ( CGEREP_EVENT_NW_REACT, &event );
    }
  }
  else
  {
    cmhSM_next_call_table_entry();
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_stop_context_reactivation |
+----------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void cmhSM_stop_context_reactivation ( void )
{

  call_waits_in_table = FALSE;
}

GLOBAL void cmhSM_next_call_table_entry( void )
{
  current_gprs_ct_index++;

  if ( current_gprs_ct_index >= gprs_ct_index )
  {
    cmhSM_empty_call_table();
  }
  else
  {
    cmhSM_context_reactivation();
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_init_GPRS_DTI_list        |
+----------------------------------------------------------------------+

  PURPOSE : Init all DTI identifier for GPRS.
*/
GLOBAL SHORT cmhSM_connect_working_cid ( void )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  UBYTE                  dti_id;
  
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[cid_pointer] );

  if( p_pdp_context_node )
  {
    switch ( p_pdp_context_node->internal_data.entity_id )
    {
      case DTI_ENTITY_PPPS:
      {
        
        srcc_new_count( SRCC_PPPS_SNDCP_LINK );

        if( IS_SRC_BT(p_pdp_context_node->internal_data.owner) )
        {
          #if defined TI_GPRS OR TI_DUAL_MODE
            T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_BLUETOOTH, DTI_ENTITY_PPPS, DTI_ENTITY_UPM, DTI_ENTITY_SNDCP};
          #else /* TI_GPRS OR TI_DUAL_MODE */
            T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_BLUETOOTH, DTI_ENTITY_PPPS, DTI_ENTITY_SNDCP};
          #endif /* TI_GPRS OR TI_DUAL_MODE */


          dti_id = dti_cntrl_new_dti( DTI_DTI_ID_NOTPRESENT );
          dti_cntrl_est_dpath ( dti_id,
                                entity_list,
                                GET_NUM_OF_DTI_ENTITIES(entity_list),
                                SPLIT,
                                PPP_UART_connect_dti_cb);
      }
      else
      {
          #if defined TI_GPRS OR TI_DUAL_MODE
             T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_PPPS, DTI_ENTITY_UPM, DTI_ENTITY_SNDCP};
          #else /* TI_GPRS OR TI_DUAL_MODE */
             T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_PPPS, DTI_ENTITY_SNDCP};
          #endif /* TI_GPRS OR TI_DUAL_MODE */

          dti_cntrl_est_dpath_indirect( (UBYTE)p_pdp_context_node->internal_data.owner, 
                                         entity_list, 
                                         GET_NUM_OF_DTI_ENTITIES(entity_list), 
                                         SPLIT, 
                                         PPP_UART_connect_dti_cb, 
                                         DTI_CPBLTY_SER, 
                                         (UBYTE)work_cids[cid_pointer] );

      }

      m_mt_te_link = TRUE;
      break;
      }
      
    case DTI_ENTITY_IP:
#if defined CO_UDP_IP
      {
        #if defined TI_GPRS OR TI_DUAL_MODE
          T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_IP, DTI_ENTITY_UPM, DTI_ENTITY_SNDCP};
        #else /* TI_GPRS OR TI_DUAL_MODE */
          T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_IP, DTI_ENTITY_SNDCP};
        #endif /* TI_GPRS OR TI_DUAL_MODE */
#ifdef FF_SAT_E
        if ( satShrdPrm.opchStat EQ OPCH_EST_REQ )
          dti_id = simShrdPrm.sat_class_e_dti_id;
        else
#endif /* FF_SAT_E */          
          dti_id = wap_dti_id;

        /* link_id should be created in atGD already, so just connect: */
          if( !dti_cntrl_est_dpath( dti_id,
                                    entity_list,
                                    GET_NUM_OF_DTI_ENTITIES(entity_list),
                                    APPEND,
                                    IP_UDP_connect_dti_cb) )
        {
          TRACE_EVENT("cmhSM_connect_working_cid: dti_cntrl_est_dpath returned FALSE");
          return 0;
        }
      }
#endif /* CO_UDP_IP */
      break;
#ifdef FF_SAT_E
      case DTI_ENTITY_SIM:
      {
        #if defined TI_GPRS OR TI_DUAL_MODE
          T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_SIM, DTI_ENTITY_UPM, DTI_ENTITY_SNDCP};
        #else /* TI_GPRS OR TI_DUAL_MODE */
      T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_SIM, DTI_ENTITY_SNDCP};
        #endif /* TI_GPRS OR TI_DUAL_MODE */

      if ( simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT )
      {
        simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
        TRACE_EVENT_P1("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
      }

        srcc_new_count( SRCC_SIM_SNDCP_LINK );

        p_pdp_context_node->internal_data.link_id = dti_conn_compose_link_id( 0, 0, simShrdPrm.sat_class_e_dti_id, DTI_TUPLE_NO_NOTPRESENT );
       dti_cntrl_est_dpath( simShrdPrm.sat_class_e_dti_id,
                            entity_list,
                            GET_NUM_OF_DTI_ENTITIES(entity_list),
                            SPLIT,
                            SIM_SNDCP_connect_dti_cb );

        break;
      }
#endif /* FF_SAT_E */

#if defined (FF_GPF_TCPIP)
    case DTI_ENTITY_TCPIP:
    {
      T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_TCPIP, DTI_ENTITY_SNDCP};
      UBYTE dti_id;
      dti_id = wap_dti_id;
      /* link_id should be created in atGD already, so just connect: */
      if (!dti_cntrl_est_dpath( dti_id,
                                entity_list,
                                2,
                                SPLIT,
                                TCPIP_connect_dti_cb))
      {
        TRACE_EVENT("cmhSM_connect_working_cid: dti_cntrl_est_dpath returned FALSE");
        return 0;
      }
    }
    break;
#endif // FF_GPF_TCPIP


#if defined(FF_PKTIO) OR defined(FF_TCP_IP) OR defined (FF_PSI)
      case DTI_ENTITY_INVALID:
      {
        #if defined TI_GPRS OR TI_DUAL_MODE
          T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_UPM, DTI_ENTITY_SNDCP};
        #else /* TI_GPRS OR TI_DUAL_MODE */
          T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_SNDCP};
        #endif /* TI_GPRS OR TI_DUAL_MODE */
              
        srcc_new_count( SRCC_PKTIO_SNDCP_LINK );
              
        dti_cntrl_est_dpath_indirect( (UBYTE)p_pdp_context_node->internal_data.owner,
                                       entity_list,
                                       GET_NUM_OF_DTI_ENTITIES(entity_list),
                                       SPLIT,
                                       PKTIO_SNDCP_connect_dti_cb,
                                       DTI_CPBLTY_PKT,
                                       (UBYTE)work_cids[cid_pointer] );

        /*
           Issue 31781 - If the PDP context is not activated then activate the context.
         */
        switch ( get_state_working_cid( ) )
        {
          case PDP_CONTEXT_STATE_DEFINED:  
          case PDP_CONTEXT_STATE_ATTACHING:
            {
              cmhSM_activate_context();
            }
            break;
        }
        break;
      }
#else /* FF_PKTIO OR FF_TCP_IP  OR FF_PSI */
     case DTI_ENTITY_INVALID:
     {
        TRACE_ERROR("cmhSM_connect_working_cid(): DTI_ENTITY_INVALID is illegal for this product!!!");
        break;
     }
#endif /* FF_PKTIO OR FF_TCP_IP */

    default:
      return 0;

    }
  }

  return 1;
}


/*
+----------------------------------------------------------------------+
| PROJECT :                  MODULE  : CMH_SMF                         |
| STATE   :                  ROUTINE : cmhSM_disconnect_cid            |
+----------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void cmhSM_disconnect_cid ( SHORT cid, T_GPRS_CONNECT_TYPE type )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("cmhSM_disconnect_cid");

  p_pdp_context_node = pdp_context_find_node_from_cid((U8) cid );

  if( p_pdp_context_node )
  {
  
    if ( DTI_LINK_ID_NOTPRESENT NEQ p_pdp_context_node->internal_data.link_id)
  {
    switch ( type )
    {
      case GC_TYPE_DATA_LINK:
          srcc_delete_count( SRCC_PPPS_SNDCP_LINK );
          p_pdp_context_node->internal_data.link_id      = DTI_LINK_ID_NOTPRESENT;
          p_pdp_context_node->internal_data.link_id_uart = DTI_LINK_ID_NOTPRESENT;
          m_mt_te_link = FALSE;
          break;

#if defined (FF_WAP) OR defined (FF_SAT_E)
      case GC_TYPE_WAP:
          srcc_delete_count( SRCC_IP_SNDCP_LINK );
          p_pdp_context_node->internal_data.link_id     = DTI_LINK_ID_NOTPRESENT;
          break;
#endif /* FF_WAP OR SAT E */

      case GC_TYPE_NULL:
      case GC_TYPE_EMAIL:
          switch ( p_pdp_context_node->internal_data.entity_id )
          {
            case DTI_ENTITY_IP:
              srcc_delete_count( SRCC_IP_SNDCP_LINK);
              p_pdp_context_node->internal_data.link_id     = DTI_LINK_ID_NOTPRESENT;
              break;            
#if defined(FF_PKTIO) OR defined(FF_TCP_IP) OR defined(FF_GPF_TCPIP) OR defined (FF_PSI)
            case DTI_ENTITY_PKTIO:
            case DTI_ENTITY_PSI:
            case DTI_ENTITY_AAA:
              srcc_delete_count( SRCC_PKTIO_SNDCP_LINK );
              p_pdp_context_node->internal_data.link_id     = DTI_LINK_ID_NOTPRESENT;
              break;
#endif /* FF_PKTIO OR FF_TCP_IP OR FF_GPF_TCPIP OR FF_PSI */
          }
          break;
      }
    }
  }
  else
  {
    TRACE_ERROR( "PDP context not found" );
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT :                  MODULE  : CMH_SMF                         |
| STATE   :                  ROUTINE : uart_is_mt_te_link              |
+----------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL BOOL uart_is_mt_te_link( void )
{

  return m_mt_te_link;
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_connect_context           |
+----------------------------------------------------------------------+

  PURPOSE : Activate the context or if the context is already activated
            then activate the user plane.
            (PPP_ACTIVATE_IND is just received).
*/
GLOBAL SHORT cmhSM_connect_context ( U8 cid, T_DTI_ENTITY_ID peer )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8  hcomp;
  U8  dcomp;

  TRACE_FUNCTION( "cmhSM_connect_context()" );
  
  /* hcomp value being changed according to spec 27.007 */
  hcomp = cmhSM_Get_h_comp() ;
  dcomp = cmhSM_Get_d_comp() ;
 

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  /*
   * Activate the context if not already activated.
   */
  if (p_pdp_context_node)
  {
    switch (get_state_over_cid(cid))
    {
      case PDP_CONTEXT_STATE_ESTABLISH_1:
        if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_PRIMARY )
        {
          psaSM_smreg_pdp_activate_req( cid,                                     
                                        hcomp,
                                        dcomp);
                                        /*,                                     
                                        PS_RAT_UMTS_FDD);*/
        }
#ifdef REL99
        else if( p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_SECONDARY )
        {
          psaSM_smreg_pdp_activate_sec_req( cid );
        }
#endif
        return 1;
      default:
        TRACE_ERROR("cmhSM_connect_context: Wrong state!!!");
      return 0;
    }
  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, in function cmhSM_connect_context" );
    return 0;
  }
}

   

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_set_PCO                   |
+----------------------------------------------------------------------+

  PURPOSE : Set a PCO in the context of the cid.
*/
GLOBAL void cmhSM_set_PCO( U8 cid, T_PCO_TYPE pco_type, UBYTE* buf_addr, UBYTE length )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    switch( pco_type )
  {
    case PCO_USER:
      {
  /*lint -e(644) */ /* all possible types defined */
        memcpy( &p_pdp_context_node->internal_data.user_pco.pco, buf_addr, length );
        p_pdp_context_node->internal_data.user_pco.len = length;
        break;
      }

      case PCO_NETWORK:
      {
        memcpy( &p_pdp_context_node->internal_data.network_pco.pco, buf_addr, length );
        p_pdp_context_node->internal_data.network_pco.len = length;
        break;
      }
      
    }

  }
  else
  {
    TRACE_ERROR( "ERROR: PDP context not found, invalid cid" );
  }

}
 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMF                  |
| STATE   : finished              ROUTINE : cmhSM_CGPCO_HEX     |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the %CGPCO= AT
          command to set protocol configuration options for the
          PDP context activation.
*/

GLOBAL T_ACI_RETURN cmhSM_CGPCO_HEX (U8     cid,
                                          UBYTE  *pco_array,
                                          UBYTE  pco_len )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_ACI_RETURN ret = AT_CMPL;
  U8 i;

  TRACE_FUNCTION("cmhSM_CGPCO_HEX");

  if (pco_len EQ 0)
  {
    if( cid EQ PDP_CONTEXT_CID_OMITTED )
    {
      /* For later contexts */
      pdp_context_default.internal_data.user_pco.len = 0;
      memset( &pdp_context_default.internal_data.user_pco, 0, sizeof(T_PDP_CONTEXT_PCO) );

      /* For defined contexts */
      for( i = PDP_CONTEXT_CID_MIN; i <= PDP_CONTEXT_CID_MAX; i++)
      {
        p_pdp_context_node = pdp_context_find_node_from_cid( i );

        if( p_pdp_context_node )
        {
          p_pdp_context_node->internal_data.user_pco.len = 0;
          memset( &p_pdp_context_node->internal_data.user_pco, 0, sizeof(T_PDP_CONTEXT_PCO) );
        }
      }
    }
    else
    {
      /* For specified context */
      p_pdp_context_node = pdp_context_find_node_from_cid( (U8)cid );
      
      if( p_pdp_context_node )
      {
        p_pdp_context_node->internal_data.user_pco.len = 0;
        memset( &p_pdp_context_node->internal_data.user_pco, 0, sizeof(T_PDP_CONTEXT_PCO) );
      }
      else
      {
        TRACE_EVENT_P1("PDP context for cid = %d not found.", cid );
        ret = AT_FAIL;
      }
    }
  }
  else  
  {
    if( cid EQ PDP_CONTEXT_CID_OMITTED )
    {
      /* For later contexts */
      pdp_context_default.internal_data.user_pco.len = pco_len;
      memcpy( &pdp_context_default.internal_data.user_pco, pco_array, pco_len );

      /* For defined contexts */
      for( i = PDP_CONTEXT_CID_MIN; i <= PDP_CONTEXT_CID_MAX; i++)
      {
        p_pdp_context_node = pdp_context_find_node_from_cid( i );

        if( p_pdp_context_node )
        {
          p_pdp_context_node->internal_data.user_pco.len = pco_len;
          memcpy( &p_pdp_context_node->internal_data.user_pco.pco, pco_array, pco_len );
        }
      }
    }
    else
    {
      /* For specified context */
      p_pdp_context_node = pdp_context_find_node_from_cid( (U8)cid );
  
      if( p_pdp_context_node )
      {
        p_pdp_context_node->internal_data.user_pco.len = pco_len;
        memcpy( &p_pdp_context_node->internal_data.user_pco.pco, pco_array, pco_len );
      }
      else
      {
        TRACE_EVENT_P1("PDP context for cid = %d not found.", cid );
        ret = AT_FAIL;
      }
    }
  }
  return (ret);
}

 
/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_get_dti_UPM_peer        |
+----------------------------------------------------------------------+

  PURPOSE : Give back the link_id for the UPM peer.
*/
GLOBAL ULONG cmhSM_get_link_id_UPM_peer( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    return p_pdp_context_node->internal_data.link_id;
  }

  return DTI_LINK_ID_NOTPRESENT;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : code             ROUTINE : cmhSM_context_deactivated       |
+----------------------------------------------------------------------+

  PURPOSE : Control the answer of context deactivations 
            started by an AT command. 
*/
GLOBAL void cmhSM_context_deactivated( USHORT nsapi_set )
{

  switch( working_cgact_actions.state  )
  {
    case T_CDS_RUNNING:
      /* For TC ACISAT531  
         We do not need OK for SAT invoked context !
      */
#if defined (FF_SAT_E)
      if( ( satShrdPrm.chnTb.chnUsdFlg )                  AND
          ( satShrdPrm.chnTb.chnType EQ B_GPRS ) )
      {
        if( nsapi_set EQ (1U << CID_TO_NSAPI(satShrdPrm.chnTb.chnRefId) ) )
        {
          working_cgact_actions.nsapi_set &= ~nsapi_set;
          if ( ! working_cgact_actions.nsapi_set  )
          { 
            working_cgact_actions.state = T_CDS_IDLE;
            return;
          }
        }
      }
#endif /* SAT Class E */

      if  ( nsapi_set & working_cgact_actions.nsapi_set )
      { /* nsapi deactivation is requested */
        working_cgact_actions.nsapi_set &= ~nsapi_set;
        if ( ! working_cgact_actions.nsapi_set  )
        { 
            R_AT( RAT_OK, working_cgact_actions.srcId ) ( AT_CMD_CGACT );
            working_cgact_actions.state = T_CDS_IDLE;
        }
      }
      break;
    case T_CDS_IDLE:
      break;
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                    |
| STATE   : finnished             ROUTINE : deactivateAContextForCGACT |
+----------------------------------------------------------------------+

PURPOSE       : deactivate a context for CGACT

RETURN VALUE  : TRUE  - AT_EXCT have to be returned for this context
                FALSE - AT_CMPL have to be reported for this context

*/
T_ACI_RETURN deactivateAContextForCGACT(U8 cid)
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION("deactivateAContextForCGACT()");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    TRACE_EVENT("ERROR: PDP context not defined");
    return( AT_FAIL );
  }

#if defined (CO_UDP_IP) OR defined (FF_GPF_TCPIP)
  /*
   *  additinal behaviour for WAP
   *  abort ATD*98# before context activation requested
   *  2003.05.06 brz
   */
   if( TRUE NEQ srcc_reserve_sources( SRCC_IP_SNDCP_LINK, 1 )                   AND
        DTI_LINK_ID_NOTPRESENT NEQ p_pdp_context_node->internal_data.link_id_new AND
      cid  EQ work_cids[cid_pointer]                              OR
        p_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_IP             )
  {
      /* tell WAP ACI that contextactivation was rejected */
      TRACE_EVENT("Tell WAP ACI that contextactivation was rejected");
      psaTCPIP_Deactivate(cmhSM_IP_activate_cb);

      if ( PDP_CONTEXT_STATE_DEFINED EQ p_pdp_context_node->internal_data.state )
      {
        if(is_gpf_tcpip_call()) {
          GPF_TCPIP_STATEMENT(wap_state = TCPIP_Deactivation);
        }
        else {
          wap_state = UDPA_Deactivation;
        }
        dti_cntrl_close_dpath_from_dti_id(
        EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
      }

      /* srcc_delete_count(SRCC_IP_SNDCP_LINK); */
      return cmhSM_deactivateAContext(CMD_SRC_NONE, cid);
  }
  else
#endif  /* CO_UDP_IP OR FF_GPF_TCPIP */
  if( PDP_CONTEXT_STATE_DEFINED NEQ p_pdp_context_node->internal_data.state AND
      PDP_CONTEXT_STATE_INVALID NEQ p_pdp_context_node->internal_data.state )
    {
    return cmhSM_deactivateAContext(CMD_SRC_NONE, cid);
  }
  return AT_CMPL;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_deactivateAContext        |
+----------------------------------------------------------------------+

  PURPOSE : deactivate some contexts (able for serial multiplexer mode).
*/
GLOBAL T_ACI_RETURN cmhSM_deactivateContexts( T_ACI_CMD_SRC srcId, SHORT *cids)
{
  T_ACI_RETURN ret_value = AT_CMPL;
  USHORT nsapi_set = 0;
  U8     cid_set = 0;
  SHORT i = 0; 
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("cmhSM_deactivateContexts");

  if( T_CDS_RUNNING EQ working_cgact_actions.state) {
    TRACE_EVENT("cgact_ation is running: bussy returned");
    return AT_FAIL; /* AT_BUSY */
  }

  cid_pointer = 0;
  if( *cids EQ PDP_CONTEXT_CID_INVALID ) 
  { /* all available contexts */

    p_pdp_context_node = p_pdp_context_list;
    while( p_pdp_context_node )
    {
      work_cids[i] = p_pdp_context_node->cid;
      i++;
      p_pdp_context_node = p_pdp_context_node->p_next;
    }

    work_cids[i] = PDP_CONTEXT_CID_INVALID;

    if( work_cids[0] EQ PDP_CONTEXT_CID_INVALID )
    {
      return AT_CMPL;
    }
  } 
  else 
  { /* copy cid list */

    for( i = 0; cids[i] NEQ  PDP_CONTEXT_CID_INVALID; i++ )
    {
      work_cids[i] = (U8)cids[i];
    }
  }

  for(i = 0; work_cids[i] NEQ PDP_CONTEXT_CID_INVALID; i++)
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[i] );
    
    if( !p_pdp_context_node )
        continue;

    if(AT_EXCT EQ deactivateAContextForCGACT( work_cids[i] ))
    {
      ret_value = AT_EXCT;
      nsapi_set |= ( 1 << CID_TO_NSAPI (work_cids[i]) );
      cid_set   |= ( 1 << work_cids[i] );
    }
  }

  if( nsapi_set ) {
    working_cgact_actions.state     = T_CDS_RUNNING;
    working_cgact_actions.nsapi_set = nsapi_set;
    working_cgact_actions.srcId     = srcId;
    working_cgact_actions.cid_set   = cid_set;
  }

  cmhSM_stop_context_reactivation();

  return ret_value;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_CGACT_start               |
+----------------------------------------------------------------------+

  PURPOSE : Set specific +CGACT command parameter.
*/
GLOBAL void cmhSM_CGACT_start( T_CGACT_STATE state, USHORT nsapi_set )
{

  working_cgact_actions.state     = (T_CONTEXTS_DEACTIVATION_STATUS)state;
  working_cgact_actions.nsapi_set = nsapi_set;

}



/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_getCurQOS                 |
+----------------------------------------------------------------------+

  PURPOSE : This function returns the current QOS settings.
*/
#ifdef FF_SAT_E
GLOBAL T_PS_qos* cmhSM_getCurQOS( U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if (p_pdp_context_node)
    return &p_pdp_context_node->qos;
  return NULL;
}
#endif /* FF_SAT_E */

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_deactivateAContext        |
+----------------------------------------------------------------------+

  PURPOSE : deactivate a contexts (able for serial multiplexer mode).
*/
GLOBAL T_ACI_RETURN cmhSM_deactivateAContext( T_ACI_CMD_SRC srcId, U8 cid )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION("cmhSM_deactivateAContext()") ;

  switch( get_state_over_cid( cid ) )
  {
    case PDP_CONTEXT_STATE_INVALID:
    case PDP_CONTEXT_STATE_DEFINED:
      /* 
       *    context ís deactivated -> not action necessary
       */
      break;

    case PDP_CONTEXT_STATE_ATTACHING:
      /* 
       *    +CGDATA or +CGACT has started the attach procedure and which is stopped.
       */
      set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
      psaGMM_Detach( GMMREG_DT_GPRS );
      gpppEntStat.curCmd = AT_CMD_NONE;
      smEntStat.curCmd = AT_CMD_NONE;
      work_cids[0] = PDP_CONTEXT_CID_INVALID;
      cid_pointer  = 0;
      if( CMD_SRC_NONE EQ srcId )
      {
        p_pdp_context_node = pdp_context_find_node_from_cid( cid );
        p_pdp_context_node->internal_data.owner = gpppEntStat.entOwn;
        gaci_RAT_caller ( RAT_NO_CARRIER, cid, AT_CMD_CGDATA, 0 );
      }
      return AT_CMPL;
    
    case PDP_CONTEXT_STATE_ESTABLISH_1:
      /*
       *   context not activated, but PPP has to be terminated
       */
      set_state_over_cid( cid, PDP_CONTEXT_STATE_ABORT_ESTABLISH );
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      dti_cntrl_entity_disconnected( p_pdp_context_node->internal_data.link_id_new, DTI_ENTITY_SNDCP );
      if (uartShrdPrm.escape_seq EQ UART_DETECT_ESC)
      {
        psaGPPP_Terminate( PPP_LOWER_LAYER_DOWN );
      }
      else
      {
        psaGPPP_Terminate( PPP_LOWER_LAYER_UP );
      }
      uartShrdPrm.escape_seq = UART_DETECT_DTR;
      /* The user plane will be closed when the PPP_TERMINATE_IND is received */
      return AT_EXCT;
    
    case PDP_CONTEXT_STATE_DATA_LINK:
    case PDP_CONTEXT_STATE_ESTABLISH_2:
    case PDP_CONTEXT_STATE_ESTABLISH_3:
    case PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1:
      /* 
       *    context has to be deactivated and PPP has to be terminated
       */
      set_state_over_cid( cid, PDP_CONTEXT_STATE_REACTIVATION_1 );
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      psaSM_PDP_Deactivate( (USHORT) (1 << CID_TO_NSAPI( cid )), PS_REL_IND_NO);
      /* The terminate request to PPP entitiy will be done in DTI for the 
         disconnection when PSI is enabled */
#ifndef FF_PSI                
      psaGPPP_Terminate( PPP_LOWER_LAYER_UP );
#endif /* FF_PSI */
      return AT_EXCT;

    case PDP_CONTEXT_STATE_ACTIVATED:
    case PDP_CONTEXT_STATE_ACTIVATING:
      /*
       *    +CGACT aborted
       */
      set_state_over_cid( cid, PDP_CONTEXT_STATE_DEACTIVATE_NORMAL );
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      psaSM_PDP_Deactivate( (USHORT) (1 << CID_TO_NSAPI( cid )), PS_REL_IND_NO);
      
      if( p_pdp_context_node->internal_data.entity_id EQ DTI_ENTITY_PKTIO )
      {
        /* For PKTIO we are not maintaining any other states other than
           PDP_CONTEXT_STATE_ACTIVATED and PDP_CONTEXT_STATE_ACTIVATING
           and in this case after entering data mode if we deactivate the 
           PDP context, we are not taking care of freeing the DTI links 
           between PKTIO and other Neighboring entity
           This is done if the entity is PKTIO. 
           As for PPP the DTI link free is taken care by PPP 
           primitives between ACI and PPP
        */
        TRACE_EVENT("cmhSM_deactivateAContext: Free the DTI Links fpr PKTIO Entity after CGACT=0,1");
        TRACE_EVENT("States: PDP_CONTEXT_STATE_ACTIVATED OR PDP_CONTEXT_STATE_ACTIVATING");
        dti_cntrl_close_dpath_from_dti_id (EXTRACT_DTI_ID(p_pdp_context_node->internal_data.link_id));
      }
      return AT_EXCT;

    case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
    case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
    case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
      /*
       *    context is during deactivation procedure -> not action necessary
       */
      return AT_EXCT;

    case PDP_CONTEXT_STATE_REACTIVATION_1:
    case PDP_CONTEXT_STATE_REACTIVATION_2:
      /* 
       *  context is during deactivation procedure 
       *  -> not action for context deactivation or PPP termination necessary
       */
      return AT_EXCT;
  }

  return AT_FAIL;
}


GLOBAL BOOL cmhSM_isContextActive( void )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED OR 
        p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED_ESTABLISH_1 OR
        p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_ACTIVATED_MODIFYING OR
        p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK OR
        p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DATA_LINK_MODIFYING )
        
      return TRUE;
    else
      p_pdp_context_node = p_pdp_context_node->p_next;
  }

  return FALSE;

}


LOCAL  void cmhSM_free_pdpcontext_tft(T_TFT_INTERNAL * pdp_tft)
{
  TRACE_FUNCTION("cmhSM_free_pdpcontext_tft");
  if(pdp_tft->p_next)
  {
    cmhSM_free_pdpcontext_tft(pdp_tft->p_next);
  }
  ACI_MFREE(pdp_tft);

}


LOCAL  void cmhSM_free_pdpcontext( T_PDP_CONTEXT_INTERNAL * p_pdp_context)
{
  //recursive call so that the contexts will be deallocated from last to first.
  //when p_next pointer is null, it will proceed to deallocate the PDP context itself
  TRACE_FUNCTION("cmhSM_free_pdpcontext");
  if (p_pdp_context->p_next)
  {
    cmhSM_free_pdpcontext(p_pdp_context->p_next);
  }
  //If one exist, it shall be deallocated together with the PDP context 
  if (p_pdp_context->internal_data.p_pdp_activate_cnf)
  {
    ACI_MFREE(p_pdp_context->internal_data.p_pdp_activate_cnf);
  }
  
  if (p_pdp_context->p_tft_pf) 
  {
    cmhSM_free_pdpcontext_tft(p_pdp_context->p_tft_pf);
  }
  
  ACI_MFREE(p_pdp_context);
}


GLOBAL void cmhSM_free_pdpcontext_list(void)
{
  TRACE_FUNCTION("cmhSM_free_pdpcontext_list");
  if (p_pdp_context_list)
    cmhSM_free_pdpcontext(p_pdp_context_list);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_mapSM2ACI_Cause()         |
+----------------------------------------------------------------------+

  PURPOSE : The below function is used to MAP the SMREG cause values
            sent by N/W to the ACI specific ERROR's. Refer 27.007 
            Section 9.2. 
*/

GLOBAL UBYTE cmhSM_mapSM2ACI_Cause(U16 cause_value)
{
  TRACE_FUNCTION("cmhSM_mapSM2ACI_Cause");

  switch(cause_value)
  {
    case CAUSE_NWSM_USER_AUTH_FAILED :
      return CME_ERR_GPRSPdpAuth;
  
    case CAUSE_NWSM_SERVICE_NOT_SUPPORTED :
      return CME_ERR_GPRSSerOptNsup;

    case CAUSE_NWSM_SERVICE_NOT_SUBSCRIBED :
      return CME_ERR_GPRSSerOptNsub;

    case CAUSE_NWSM_SERVICE_TEMP_OUT_OF_ORDER :
      return CME_ERR_GPRSSerOptOOO;

    default:
      return CME_ERR_GPRSUnspec;
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT : GPRS (8441)      MODULE  : CMH_SMF                         |
| STATE   : finnished        ROUTINE : cmhSM_clear_work_cids           |
+----------------------------------------------------------------------+

  PURPOSE : The below function will decide whether to clear work_cids 
            and cid_pointer variables. 
*/

GLOBAL void cmhSM_clear_work_cids(U8 cid)
{
  U8 index = 0;

  TRACE_FUNCTION("cmhSM_clear_work_cids()");

  while( index < PDP_CONTEXT_CID_MAX )
  {
    if(cid NEQ work_cids[index])
    {
      index++;
    }
    else if( pdp_context_get_state_for_cid(cid) EQ PDP_CONTEXT_STATE_DEFINED )
    {
      /*
        If the state of the cid is other than PDP_CONTEXT_STATE_DEFINED then
        don't clear the work_cids[].
       */
      TRACE_EVENT_P2("cmhSM_clear_work_cids(): Clear the work_cid value for the cid and for the location work_cids[index]= %d, %d", cid, index);
      work_cids[index++] = PDP_CONTEXT_CID_INVALID;
    }
    else
    {
      index++;
    }
  }

  /* 
     Check whether the work_cids[] contains any valid cids which are in the
     state, other than PDP_CONTEXT_CID_INVALID.
   */
  while ( ( index < PDP_CONTEXT_CID_MAX ) AND ( work_cids[index] EQ PDP_CONTEXT_CID_INVALID ) )
  {
    index++;
  }

  /*
    After scnaing whole work_cids[] if we didn't find any valid cids in the
    work_cids[] list, then clear the work_cids[].
  */
  if (index >= PDP_CONTEXT_CID_MAX)
  {
    TRACE_EVENT("cmhSM_clear_work_cids(): Clear the whole work_cid[] list and cid_pointer");
    *work_cids = PDP_CONTEXT_CID_INVALID;
    cid_pointer = 0;
    smEntStat.curCmd = AT_CMD_NONE;
  }
}

#endif  /* GPRS */
/*==== EOF ========================================================*/

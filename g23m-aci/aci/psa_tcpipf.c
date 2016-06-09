/* 
+----------------------------------------------------------------------------- 
|  Project :  WAPoverGPRS
|  Modul   :  PSA_TCPIPF
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
|  Purpose :  Functions for interfacing to TCP/IP-related entities. At
|             the moment, these are IP(v4) and UDP. In the future, TCP and IPv6
|             and perhaps others will follow, so we try to be as general as
|             possible.
|             
|             Main exports:
|             psaTCPIP_Activate()
|             psaTCPIP_Configure()
|             psaTCPIP_Deactivate()
|             
|             Declarations and definitions are in psa_tcpip.h.
+----------------------------------------------------------------------------- 
*/ 
#if defined (CO_UDP_IP) || defined (FF_GPF_TCPIP)

#ifndef PSA_TCPIPF_C
#define PSA_TCPIPF_C
#endif /* !PSA_TCPIPF_C */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_fd.h"
#include "psa.h"
#include "psa_tcpip.h"
#include "psa_cc.h"
#include "cmh.h"
#include "psa_ra.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"

#include "ksd_utl.h"

#if defined (FF_SAT_E) 
#include "cmh_cc.h"
#include "psa_sat.h"
#endif /* SAT E */ 

#ifdef FF_GPF_TCPIP
#include "dcm_utils.h"
#include "dcm_state.h"
#include "dcm_env.h"
#endif
#include "dcm_f.h"



/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/
GLOBAL void psaTCPIP_Initialize_Req(void);
GLOBAL void psaTCPIP_Shutdown_Req(void);
GLOBAL void psaTCPIP_Config (ULONG ipaddr, ULONG dns1, ULONG dns2, UBYTE dirc);
GLOBAL ULONG bytes2ipv4addr(UBYTE *host);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/* Convert an IP address in an array of 4 bytes (network order
 * assumed) into an unsigned long value representing the same IP
 * address, also in network order. This is purely a matter of
 * alignment, so a simple copy will do.
 */
ULONG psaTCPIP_bytes2ipv4addr(UBYTE *ip_address)
{
  ULONG result;
  UBYTE i;
  BYTE buf[4];

  for (i=0; i<4; i++)
  {
    utl_string2Byte ((CHAR*)(ip_address+(i*4)), 3, &buf[3-i]);
  }
  memcpy(&result, buf, 4);
  return result;
}

GLOBAL char* wap_state_to_string(T_ACI_WAP_STATES wap_state)
{
  switch(wap_state)
  {
    case Wap_Not_Init: return "wap-not_init";
#ifdef FF_GPF_TCPIP
    case TCPIP_Initialization:  return "TCPIP_Initialization";
    case TCPIP_Initialized:     return "TCPIP_Initialized";
    case TCPIP_Activation:      return "TCPIP_Activation";
    case TCPIP_Activated:       return "TCPIP_Activated";
#endif /*FF_GPF_TCPIP*/
  	case UDPA_Activation:       return "UDPA_Activation";
  	case UDPA_Activated:        return "UDPA_Activated";
    case IPA_Activation:        return "IPA_Activation";
  	case IPA_Activated:         return "IPA_Activated";
  	case IPA_Configuration:     return "IPA_Configuration";
  	case IPA_Configurated:      return "IPA_Configurated";
   	case UDPA_Configuration:    return "UDPA_Configuration";
  	case UDPA_Configurated:     return "UDPA_Configurated";
#ifdef FF_GPF_TCPIP	
  	case TCPIP_Configuration:   return "TCPIP_Configuration";
    case TCPIP_Configurated:    return "TCPIP_Configurated";
#endif /*FF_GPF_TCPIP*/
  	case IPA_Deconfiguration:   return "IPA_Deconfiguration";
   	case IPA_Deconfigurated:    return "IPA_Deconfigurated";
   	case UDPA_Deconfiguration:  return "UDPA_Deconfiguration";
   	case UPDA_Deconfigurated:   return "UPDA_Deconfigurated";
#ifdef FF_GPF_TCPIP	
   	case TCPIP_Deconfiguration: return "TCPIP_Deconfiguration";
   	case TCPIP_Deconfigurated:  return "TCPIP_Deconfigurated";
    case TCPIP_Deactivation:    return "TCPIP_Deactivation";
   	case TCPIP_Deactivated:     return "TCPIP_Deactivated";
#endif /*FF_GPF_TCPIP*/
    case UDPA_Deactivation:     return "UDPA_Deactivation";
  	case UDPA_Deactivated:      return "UDPA_Deactivated";
  	case IPA_Deactivation:      return "IPA_Deactivation";
   	case IPA_Deactivated:       return "IPA_Deactivated";
    default:                    return "<Unknown wap_state>";
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : WAPoverGPRS           MODULE  : PSA_TCPIP               |
|                                 ROUTINE : psaTCPIP_Init           |
+-------------------------------------------------------------------+

  PURPOSE : Initialize the protocol stack adapter for TCP/IP.

*/
GLOBAL void psaTCPIP_Init(void)
{
  memset (&tcpipShrdPrm, 0, sizeof (T_TCPIP_SHRD_PRM));
  wap_state = Wap_Not_Init ;    /* This is redundant at the moment,
                                 * but will be set only here
                                 * eventually. */
  tcpipShrdPrm.connection_type    = TCPIP_CONNECTION_TYPE_UNKNOWN;
  tcpipShrdPrm.connection_buildup = TCPIP_CONNECTION_BUILDUP_UNKNOWN;
}


/* Activate TCP/IP-related entities. See psa_tcpip.h for a detailed
 * description.
 */
void psaTCPIP_Activate(UBYTE src_id,
                       UBYTE dti_id,
                       SHORT wap_call_id,
                       UBYTE options,
                       UBYTE connection_type,
                       void (*callback_function)(T_ACI_RETURN result))
{
  /* The `options' parameter is not yet used, since we have only UDP
   * and IPv4. This will change in the future, including related
   * changes to other parts of the ACI. (At the moment the completion
   * of UDP activation calls IP activation directly, but this will
   * have to be torn apart. [ni 2001-10-02]
   */

  TRACE_FUNCTION ("psaTCPIP_Activate()") ;
  
  tcpipShrdPrm.src_id             = src_id ;
  tcpipShrdPrm.connection_type    = connection_type;
  tcpipShrdPrm.connection_buildup = TCPIP_CONNECTION_BUILDUP_UP;
  tcpipShrdPrm.wap_call_id        = wap_call_id ;
  tcpipShrdPrm.options            = options ;
  tcpipShrdPrm.callback_function  = callback_function ;

  if(is_gpf_tcpip_call()) {
    GPF_TCPIP_STATEMENT(wap_state = TCPIP_Initialization);
    psaTCPIP_config_dispatch();
  }
  else {
    wap_state = Wap_Not_Init ;
    psaUDPIP_config_dispatch();
  }
}


/* Configure TCP/IP-related entities.
 */
void psaTCPIP_Configure(UBYTE *ip_address,
                        void *pdp_addrp,
                        UBYTE *peer_address,
                        UBYTE *dns1,
                        UBYTE *dns2,
                        short  mtu,
                        void (*callback_function)(T_ACI_RETURN result))
{
  TRACE_FUNCTION("psaTCPIP_Configure()") ;

  if (ip_address AND pdp_addrp) {
    TRACE_ERROR("psaTCPIP_Configure(): both ip_address and pdp_addrp non-null!") ;
    return ;
  }

  if (!ip_address AND !pdp_addrp) {
    TRACE_ERROR("psaTCPIP_Configure(): both ip_address and pdp_addrp null!") ;
    return ;
  }

  if (ip_address)               /* From PPP, IP over CSD. */
  {
    memcpy(tcpipShrdPrm.ipaddr, ip_address, 16) ;
  }
#ifdef GPRS
  else if (pdp_addrp)           /* From SNDCP, IP over GPRS. */
  {
    memcpy(tcpipShrdPrm.ipaddr, pdp_addrp, 16/*sizeof(tcpipShrdPrm.ipaddr)*/);
  }
#endif

  if (peer_address) {
    memcpy(tcpipShrdPrm.peer_addr, peer_address, 16) ;
  }
  if(dns1) {
    memcpy(tcpipShrdPrm.dns1,dns1,16);
  }

  if(dns2) {
    memcpy(tcpipShrdPrm.dns2,dns2,16);
  }
  tcpipShrdPrm.mtu = mtu ;
  tcpipShrdPrm.callback_function = callback_function ;

  if(is_gpf_tcpip_call()) {
    GPF_TCPIP_STATEMENT(wap_state = TCPIP_Configuration);
    psaTCPIP_config_dispatch();
  }
  else {
  wap_state = IPA_Configuration ;
    psaUDPIP_config_dispatch();
  }
}


/* Deactivate TCP/IP-related entities.
 */
void psaTCPIP_Deactivate(void (*callback_function)(T_ACI_RETURN result))
{
  TRACE_FUNCTION("psaTCPIP_Deactivate()") ;
  TRACE_EVENT_P1("wap_state: %d", wap_state) ;

  tcpipShrdPrm.connection_buildup = TCPIP_CONNECTION_BUILDUP_DOWN;
  tcpipShrdPrm.callback_function = callback_function ;

  if(is_gpf_tcpip_call()) {
    #ifdef FF_GPF_TCPIP
    if ( wap_state EQ TCPIP_Configurated ) {
      wap_state = TCPIP_Deconfiguration ;
    }
    #endif
    psaTCPIP_config_dispatch() ;
  }
  else {
    if ( wap_state EQ UDPA_Configurated ) {
    wap_state = IPA_Deconfiguration ;
  }
    psaUDPIP_config_dispatch();
  }
}


/********************** Dispatcher functions **********************/

/* State machine for UDP/TCP/IP activation, configuration, and
 * deactivation. At the moment some of this is still handled elsewhere
 * in the ACI (see comments to psaTCPIP_Configure() and
 * psaTCPIP_Activate() above), but I plan to tear this apart and keep
 * all control over these activities in this place, including status
 * checks and transitions. [ni 2001-10-02]
 */

void psaUDPIP_config_dispatch(void)
{
#ifdef CO_UDP_IP // to avoid linker errors in simulation

  TRACE_FUNCTION("psaUDPIP_config_dispatch()") ;
  TRACE_EVENT_P1("wap_state: %s", wap_state_to_string(wap_state)) ;
  switch (wap_state)
  {
    /* Entry point for activation. */
    case Wap_Not_Init:
      cmhUDPA_Activate((T_ACI_CMD_SRC)tcpipShrdPrm.src_id, tcpipShrdPrm.wap_call_id) ;
      break ;
    case UDPA_Activation:       /* Unused at the moment; handled elsewhere. */
      break ;
    case UDPA_Activated:        /* Unused at the moment; handled elsewhere. */
      break ;
    case IPA_Activation:        /* Unused at the moment; handled elsewhere. */
      break ;
    case IPA_Activated:
      switch (tcpipShrdPrm.connection_buildup)
      {
        case TCPIP_CONNECTION_BUILDUP_UP:
          if (tcpipShrdPrm.callback_function)
          {
            tcpipShrdPrm.callback_function(AT_CMPL) ;
          }
          break ;
        case TCPIP_CONNECTION_BUILDUP_DOWN:
          cmhUDPA_Deactivate(tcpipShrdPrm.src_id);
          break;
        default:
          TRACE_ERROR("Unknown build up state in psaTCPIP_config_dispatch()");
          return;
      }
      break;
      /* Entry point for configuration. */
    case IPA_Configuration:
      switch (tcpipShrdPrm.connection_buildup)
      {
        case TCPIP_CONNECTION_BUILDUP_UP:
          psaIPA_Config(psaTCPIP_bytes2ipv4addr(tcpipShrdPrm.ipaddr),
                        tcpipShrdPrm.mtu, IPA_CONN) ;
          break;
        case TCPIP_CONNECTION_BUILDUP_DOWN:
          psaIPA_Config(psaTCPIP_bytes2ipv4addr(tcpipShrdPrm.ipaddr),
                        tcpipShrdPrm.mtu, IPA_DSC) ;
          break;
        default:
          TRACE_ERROR("Unknown build up state in psaTCPIP_config_dispatch()");
          return;
      }
      break;
    case IPA_Configurated:      /* Unused at the moment; handled elsewhere. */
      break ;
    case UDPA_Configuration:    /* Unused at the moment; handled elsewhere. */
      break ;
    case UDPA_Configurated:
      if (tcpipShrdPrm.callback_function)
      {
        tcpipShrdPrm.callback_function(AT_CMPL) ;
      }
      break ;

      /* Entry point for deactivation. */
    case IPA_Deconfiguration:
      psaIPA_Config(0, 0, IPA_DSC) ;
      break ;
    case IPA_Deconfigurated:    /* Unused at the moment; handled elsewhere. */
      break ;
    case UDPA_Deactivation:     /* Unused at the moment; handled elsewhere. */
#if defined (FF_SAT_E) 
        /* If transport layer is UDP, reset wap_call flag 
           UDP is not busy anymore */
        if( satShrdPrm.chnTb.chnTPL EQ UDP )
        {
          sAT_PercentWAP ( CMD_SRC_NONE, 0 );
        }
#endif /* SAT E */ 
      break ;
    case UDPA_Deactivated:      /* Unused at the moment; handled elsewhere. */
      break ;
    case IPA_Deactivation:
      wap_state = IPA_Deactivated ;
      tcpipShrdPrm.connection_type = TCPIP_CONNECTION_TYPE_UNKNOWN;

      if (tcpipShrdPrm.callback_function)
      {
        tcpipShrdPrm.callback_function(AT_CMPL) ;
      }
      /*lint -fallthrough*/
    case IPA_Deactivated:
      wap_state = Wap_Not_Init ;
      
      if(ccShrdPrm.wapStat EQ CC_WAP_STACK_DOWN)
      {
        /* WAP-dedicated variables shall be reinitialized */
        wapId     = NO_ENTRY;
        Wap_Call  = FALSE;
      
        TRACE_EVENT ("WAP parameter reseted");
      }
      break ;

    default:
      TRACE_ERROR("Unknown wap state in psaTCPIP_config_dispatch()") ;
  }
#else
  ACI_ASSERT(FALSE);
#endif  
}


/******************************************************************************/
void psaTCPIP_config_dispatch(void)
{
#ifdef FF_GPF_TCPIP // to avoid linker errors in simulation

  TRACE_FUNCTION("psaTCPIP_config_dispatch()") ;
  TRACE_EVENT_P1("wap_state: %s", wap_state_to_string(wap_state)) ;
  switch (wap_state)
  {
    case TCPIP_Initialization :
      switch(tcpipShrdPrm.connection_buildup)
      {
        case TCPIP_CONNECTION_BUILDUP_UP:
          psaTCPIP_Initialize_Req();      
          break;

        case TCPIP_CONNECTION_BUILDUP_DOWN:
          psaTCPIP_Shutdown_Req();
          break;

        default:
          TRACE_ERROR("Error: Unknown build_up state in psaTCPIP_config_dispatch()");
          return;   
      }

    case TCPIP_Initialized :
       break;

    case TCPIP_Activation :        
      switch(tcpipShrdPrm.connection_buildup)
      {
        case TCPIP_CONNECTION_BUILDUP_UP:
          if(tcpipShrdPrm.callback_function)
          {
            tcpipShrdPrm.callback_function(AT_CMPL);
          }
          break ;

        case TCPIP_CONNECTION_BUILDUP_DOWN:
          wap_state = TCPIP_Deactivation;
          psaTCPIP_Shutdown_Req();
          break;
          
        default:
          TRACE_ERROR("Error: Unknown build up state in psaTCPIP_config_dispatch()");
          return;
      }
      break;

    case TCPIP_Configuration:
      switch(tcpipShrdPrm.connection_buildup)
      {
        case TCPIP_CONNECTION_BUILDUP_UP:
          psaTCPIP_Config(bytes2ipv4addr(tcpipShrdPrm.ipaddr),
                          bytes2ipv4addr(tcpipShrdPrm.dns1),
                          bytes2ipv4addr(tcpipShrdPrm.dns2),
                          TCPIP_IFCONFIG_UP);
          break;
        case TCPIP_CONNECTION_BUILDUP_DOWN:
          psaTCPIP_Config(0,0,0,TCPIP_IFCONFIG_DOWN);
          break;
                
        default:
          TRACE_ERROR("Error: Unknown build up state in psaTCPIP_config_dispatch()");
          return;  
      }
      break;

    case TCPIP_Configurated:
      if(tcpipShrdPrm.callback_function)
      {
        tcpipShrdPrm.callback_function(AT_CMPL) ;
      }
      break;
	     
    case TCPIP_Deconfiguration :
      psaTCPIP_Config(0,0,0,TCPIP_IFCONFIG_DOWN);
      break;
         
  	case TCPIP_Deconfigurated :
	    break;
	     
    case TCPIP_Deactivation :
      psaTCPIP_Shutdown_Req();         
      break;
         
    case TCPIP_Deactivated:    
      tcpipShrdPrm.connection_type = TCPIP_CONNECTION_TYPE_UNKNOWN;
      if(tcpipShrdPrm.callback_function)
      {
        tcpipShrdPrm.callback_function(AT_CMPL) ;
      }
      wap_state = Wap_Not_Init ;
      break ;

    default:
      TRACE_ERROR("Error: Unknown wap_state in psaTCPIP_config_dispatch()") ;
  }
#else
  ACI_ASSERT(FALSE);
#endif  
}


/********************** Callbacks, several ***********************/

/* TCP/IP activation callback for circuit-switched data, to be called
 * when activation is completed.
 */
void psaTCPIP_act_csd_callback(T_ACI_RETURN result)
{
  TRACE_FUNCTION("psaTCPIP_act_csd_callback()");

  if (!psaCC_ctbIsValid (tcpipShrdPrm.wap_call_id))
  {
    /* Avoid to dereference NULL */
    TRACE_ERROR ("Call table entry disappeared");
    return;
  }

  /*
   * activate RA connection: in case of failure clear call !
   */
  ccShrdPrm.datStat = DS_ACT_REQ;

  if(cmhRA_Activate((T_ACI_CMD_SRC)psaCC_ctb(tcpipShrdPrm.wap_call_id)->curSrc,
                    (T_ACI_AT_CMD)psaCC_ctb(tcpipShrdPrm.wap_call_id)->curCmd,
                    tcpipShrdPrm.wap_call_id)
      NEQ AT_EXCT)
  {
    TRACE_EVENT("RA ACTIVATION FAILURE -> DISC CALL");
    ccShrdPrm.datStat = DS_IDL ;
    psaCC_ctb(tcpipShrdPrm.wap_call_id)->nrmCs = MNCC_CAUSE_CALL_CLEAR ;
    psaCC_ClearCall (tcpipShrdPrm.wap_call_id);
  }
}


/* TCP/IP configuration callback for circuit-switched data, to be called
 * when configuration is completed.
 */
void psaTCPIP_conf_csd_callback(T_ACI_RETURN result)
{
  TRACE_FUNCTION("psaTCPIP_conf_csd_callback()") ;

  R_AT ( RAT_CONNECT, (T_ACI_CMD_SRC)tcpipShrdPrm.src_id )
    ( AT_CMD_NONE, -1, tcpipShrdPrm.wap_call_id, FALSE );
  if(is_gpf_tcpip_call()) {
    #ifdef FF_GPF_TCPIP
    T_DCM_STATUS_IND_MSG msg;
    msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
	  dcm_send_message(msg, DCM_SUB_WAIT_SATDN_CNF);
    #endif // #ifdef FF_GPF_TCPIP
  }
}


/* TCP/IP deactivation callback for circuit-switched data, to be called
 * when deactivation is completed.
 */
void psaTCPIP_deact_csd_callback(T_ACI_RETURN result)
{
  TRACE_FUNCTION("psaTCPIP_deact_csd_callback()") ;

  /*
   * deactivate L2R connection: 
   */
  if (cmhL2R_Deactivate() NEQ AT_EXCT)
  {
    TRACE_EVENT("L2R DEACTIVATION FAILURE ");
    if (psaCC_ctbIsValid (tcpipShrdPrm.wap_call_id)) /* To be sure */
      psaCC_ctb(tcpipShrdPrm.wap_call_id)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
    psaCC_ClearCall (tcpipShrdPrm.wap_call_id);     /* Changed in OMAPS00049111 */
  }

  if(is_gpf_tcpip_call())
  {
#ifdef FF_GPF_TCPIP
    {
      T_DCM_STATUS_IND_MSG msg;
      msg.hdr.msg_id = DCM_NEXT_CMD_READY_MSG;
      dcm_send_message(msg, DCM_SUB_WAIT_SATH_CNF);
    }
#endif /* #ifdef FF_GPF_TCPIP */
  }
}

#endif /*defined(FF_WAP) OR defined(FF_GPF_TCPIP) */

/* EOF */


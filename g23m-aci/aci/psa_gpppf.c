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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for GPRS Point-to-Point Protocol ( PPP ).
+-----------------------------------------------------------------------------
*/

#if defined (GPRS) && defined (DTI)

#ifndef PSA_GPPPF_C
#define PSA_GPPPF_C
#endif

#include "aci_all.h"

#undef TRACING
/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "psa.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "cmh.h"
#include "cmh_sm.h"

#include "psa_gppp.h"
#include "aci_io.h"

#include "psa_uart.h"
#include "sap_dti.h"
#ifdef FF_PSI
#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#endif /*FF_PSI*/

/*==== CONSTANTS ==================================================*/
/*#define ITM_WDT         (14)*/    /* item width in chars */
/*#define HDR_WDT         (10)*/    /* header width in chars */

/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
EXTERN GLOBAL void cmhGPPP_send_establish_request ( UBYTE peer, UBYTE prot );

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : PSA_PPPF                |
| STATE   : code                  ROUTINE : psaGPPP_Init            |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for PPP.

*/

/* MACRO: initializer for set parameter */
#ifdef  INIT_SET_PARM
  #undef  INIT_SET_PARM
#endif

#define INIT_SET_PARM( dest, def )\
  for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )\
    gpppShrdPrm.setPrm[LpCnt].dest = def

GLOBAL void psaGPPP_Init ( UBYTE accm, UBYTE restart_timer,
                           UBYTE max_configure, UBYTE max_terminate, UBYTE max_failure )
{

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */
  gpppShrdPrm.owner                       = (UBYTE) CMD_SRC_NONE;
  gpppShrdPrm.ppp_authentication_protocol = PPP_AP_AUTO;

  gpppShrdPrm.accm          = accm;
  gpppShrdPrm.restart_timer = restart_timer;
  gpppShrdPrm.max_configure = max_configure;
  gpppShrdPrm.max_terminate = max_terminate;
  gpppShrdPrm.max_failure   = max_failure;

}

UBYTE GPPS_First_Stored_Peer = 0; // HHV: Bad hack due to two DTI links in one prim!
GLOBAL void psaGPPPS_Dti_Req( T_DTI_CONN_LINK_ID link_id, UBYTE peer )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  TRACE_FUNCTION("psaGPPPS_Dti_Req()");
  
  p_pdp_context_node = pdp_context_find_node_from_cid( work_cids[0]);
  
  switch ( peer )
  {
#ifdef BT_ADAPTER
    case DTI_ENTITY_BLUETOOTH:
      p_pdp_context_node->internal_data.link_id_uart = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_BLUETOOTH, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);
      if (GPPS_First_Stored_Peer)
      {
        io_setDCD (p_pdp_context_node->internal_data.owner, IO_DCD_ON);
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_BLUETOOTH);
        GPPS_First_Stored_Peer = 0;
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_BLUETOOTH;
      }
      break;

#endif  /* BT_ADAPTER */
    case DTI_ENTITY_UART:
      p_pdp_context_node->internal_data.link_id_uart = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_UART, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);
      if (GPPS_First_Stored_Peer)
      {
        io_setDCD (p_pdp_context_node->internal_data.owner, IO_DCD_ON);
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_UART);
        GPPS_First_Stored_Peer = 0;
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_UART;
      }
      break;
#if 0
    /* For the 3G protocol stack PPP can not have UPM as peer as all data goes through SNDCP! */
    case DTI_ENTITY_UPM:
      p_pdp_context_node->internal_data.link_id = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_UPM, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);
      if (GPPS_First_Stored_Peer)
      {
        T_DTI_ENTITY_ID other_peer = dti_cntrl_get_peer(DTI_ENTITY_PPPS, 0, 0);
        if(DTI_ENTITY_UART EQ other_peer) 
        {
          io_setDCD (p_pdp_context_node->internal_data.owner, IO_DCD_ON);
        }
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_UPM);
        GPPS_First_Stored_Peer = 0;
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_UPM;
      }
      break;
#endif /* 0 */
#ifdef FF_PSI
    case DTI_ENTITY_PSI:
      p_pdp_context_node->internal_data.link_id_uart = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PSI, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);
      if (GPPS_First_Stored_Peer)
      {
        io_setDCD (p_pdp_context_node->internal_data.owner, IO_DCD_ON);
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_PSI);
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_PSI;
      }
      break;
#endif /*FF_PSI*/
    case DTI_ENTITY_AAA:
      p_pdp_context_node->internal_data.link_id_uart = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_AAA, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);
      if (GPPS_First_Stored_Peer)
      {
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_AAA);
        GPPS_First_Stored_Peer = 0;
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_AAA;
      }
      break;

    case DTI_ENTITY_SNDCP:
      p_pdp_context_node->internal_data.link_id = link_id;
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_SNDCP, 0, 0);
      dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_PPPS, 0, 0);

      if (GPPS_First_Stored_Peer)
      {
        T_DTI_ENTITY_ID other_peer = dti_cntrl_get_peer(DTI_ENTITY_PPPS, 0, 0);
        if(DTI_ENTITY_UART EQ other_peer) 
        {
          io_setDCD (p_pdp_context_node->internal_data.owner, IO_DCD_ON);
        }
        cmhGPPP_send_establish_request(GPPS_First_Stored_Peer, DTI_ENTITY_SNDCP);
        GPPS_First_Stored_Peer = 0;
      }
      else
      {
        GPPS_First_Stored_Peer = DTI_ENTITY_SNDCP;
      }
      break;
  }
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : PSA_PPPF                |
| STATE   : code                        ROUTINE : PPP_UART_connect_dti_cb |
+-------------------------------------------------------------------------+

  PURPOSE : Callback for DTI connection between PPP Server and UART.

*/
GLOBAL BOOL PPP_UART_connect_dti_cb(UBYTE dti_id, T_DTI_CONN_STATE result_type)
{
  TRACE_FUNCTION("PPP_UART_connect_dti_cb");

  switch( result_type)
  {
    case DTI_CONN_STATE_DISCONNECTING:
      break;
    case DTI_CONN_STATE_DISCONNECTED:
      dti_cntrl_clear_conn_parms(dti_id);
      if (dti_cntrl_is_dti_id_to_reconnect(dti_id))
      {
        T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};
        T_DTI_CNTRL info;

        if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
        {
          TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
          return FALSE;
        }

        if (info.dev_id EQ DTI_ENTITY_UART)
        {
          io_setDCD ((T_ACI_CMD_SRC)info.src_id, IO_DCD_OFF);

          dti_cntrl_clear_dti_id_to_reconnect(dti_id);
          dti_cntrl_est_dpath_indirect ( info.src_id,
                                         entity_list,
                                         1,
                                         SPLIT,
                                         atiUART_dti_cb,
                                         DTI_CPBLTY_CMD,
                                         DTI_CID_NOTPRESENT);
        }
#ifdef FF_PSI
        else if (info.dev_id EQ DTI_ENTITY_PSI)
        {
          io_setDCD ((T_ACI_CMD_SRC)info.src_id, IO_DCD_OFF);

          dti_cntrl_clear_dti_id_to_reconnect(dti_id);
          dti_cntrl_est_dpath_indirect ( info.src_id,
                                         entity_list,
                                         1,
                                         SPLIT,
                                         atiPSI_dti_cb,
                                         DTI_CPBLTY_CMD,
                                         DTI_CID_NOTPRESENT);
        }
#endif /*FF_PSI*/
        else
           return FALSE;
      }
      break;

    case DTI_CONN_STATE_CONNECTING:
      break;
    case DTI_CONN_STATE_CONNECTED:
      break;

    case DTI_CONN_STATE_ERROR:
      /* connection not possible: disconnect PPP */
      dti_cntrl_close_dpath_from_dti_id(dti_id);
      break;
    case DTI_CONN_STATE_UNKNOWN:
    default:
      TRACE_EVENT("PPP_UART_connect_dti_cb call with not awaited value");
      break;
  }
  return TRUE;
}

#if 0
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_PPPF                |
|                                 ROUTINE : psaGPPP_shrPrmDump       |
+-------------------------------------------------------------------+

  PURPOSE : this function dumps the shared parameter to the debug
            output.
*/

GLOBAL void psaGPPP_shrPrmDump ( void )
{
#ifdef TRACING

  char  lnBuf [80];             /* holds buffer for output line */
  char  mccBuf[SIZE_MCC + 1];   /* MCC converted to printable C-string */
  char  mncBuf[SIZE_MNC + 1];   /* MNC converted to printable C-string */
  SHORT chrNr;                  /* holds number of processed chars */
  SHORT cnt;                    /* holds a counter */

 /* --- PLMN list ------------------------------------------------*/
  for( cnt = 0; cnt<MAX_PLMN_ID AND
                mmShrdPrm.PLMNLst[cnt].v_plmn NEQ INVLD_PLMN; cnt++ )
  {
    chrNr  = sprintf( lnBuf, "%*.*s[%2d]", HDR_WDT, HDR_WDT, " PLMN list",cnt );
    utl_BCD2String (mccBuf, mmShrdPrm.PLMNLst[cnt].mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.PLMNLst[cnt].mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s",
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
    TRACE_EVENT( lnBuf );
  }

  /* --- used PLMN ------------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " used PLMN" );
  if( mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN )
  {
    utl_BCD2String (mccBuf, mmShrdPrm.usedPLMN.mcc, SIZE_MCC);
    utl_BCD2String (mncBuf, mmShrdPrm.usedPLMN.mnc, SIZE_MNC);
    chrNr += sprintf( lnBuf+chrNr, "%*s %*s",
                      ITM_WDT/2, ITM_WDT/2, mccBuf, mncBuf);
  }
  else
  {
    chrNr += sprintf( lnBuf+chrNr, "%*s", ITM_WDT, "none" );
  }
  TRACE_EVENT( lnBuf );

  /* --- registration mode ----------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr mode" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.setPrm[0].regMode );
  TRACE_EVENT( lnBuf );

  /* --- registration status --------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "rgstr stat" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.regStat );
  TRACE_EVENT( lnBuf );

  /* --- search result --------------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, " srch rslt" );
  chrNr += sprintf( lnBuf+chrNr, "%*hd", ITM_WDT,
                                         mmShrdPrm.srchRslt );
  TRACE_EVENT( lnBuf );

  /* --- de-registration cause ------------------------------------*/
  chrNr  = sprintf( lnBuf, "%*.*s", HDR_WDT, HDR_WDT, "dereg caus" );
  chrNr += sprintf( lnBuf+chrNr, "%*X", ITM_WDT,
                                        mmShrdPrm.deregCs );
  TRACE_EVENT( lnBuf );

#endif  /* of #ifdef TRACING */
}
#endif  /* #if 0 */

#endif  /* GPRS */
/*==== EOF ========================================================*/


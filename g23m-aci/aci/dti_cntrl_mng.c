/*
+-----------------------------------------------------------------------------
|  Project :  ...
|  Modul   :  dti_cntrl_mng.c
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
|  Purpose :  This modul ...
+-----------------------------------------------------------------------------
*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef DTI

#ifndef DTI_CNTRL_MNG_C
#define DTI_CNTRL_MNG_C
#endif

#ifdef MFW
#define ENTITY_MFW
#else
#ifdef SMI
#define ENTITY_SMI
#else
#define ENTITY_ACI
#endif
#endif

#define ACI_MEMBER

#include "aci_all.h"

#include "dti.h"      /* functionality of the dti library */
#include "aci.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"

#include "aci_cmh.h"
#include "aci_mem.h"
#include "aci_lst.h"

#include "dti_conn_mng.h"
#include "cmh_dti.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#include "psa.h"
#include "psa_l2r.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"

#ifdef FF_FAX
#include "psa_t30.h"
#include "psa_tra.h"
#endif

#else

#include "psa.h"
#include "cmh.h"

#endif /* #ifdef FAX_AND_DATA */

#include "psa_aaa.h"

#if defined (GPRS) AND defined (UART)
#include "gaci.h"
#include "gaci_cmh.h"
#include "pcm.h"
#include "psa_gmm.h"
#include "cmh_gmm.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#include "psa_gppp.h"
#include "cmh_gppp.h"
#ifdef FF_PKTIO
#include "psa_pktio.h"
#endif
#include "psa_upm.h"
#include "psa_snd.h"
#endif /* GPRS */

#if defined (FF_WAP) AND defined (GPRS) || defined (FF_SAT_E)
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#endif

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "psa_ppp_w.h"
#include "wap_aci.h"
#endif /* WAP or FF_PPP */

#ifdef UART
#include "dti_cntrl_mng.h"
#include "psa_uart.h"
#include "ati_src_uart.h"
#endif
#ifdef FF_PSI
#include "psa_psi.h"
#endif
#include "sap_dti.h"

#ifdef SIM_TOOLKIT
#include "psa.h"
#include "psa_sim.h"
#endif

 extern void psaTCPIP_Dti_Req(T_DTI_CONN_LINK_ID dti_id, 
                             UBYTE peer_to_connect_to, UBYTE dti_conn);
/*
 * list for devices maintained by DTI Control Manager
 */
LOCAL T_ACI_LIST *dti_cntrl_list = NULL;
/* static pointer to manage output of dti_cntrl_list  elements */
static const T_ACI_LIST  *dti_cntrl_dev_ptr = NULL;
/*
 * a customer can install up to 3 own entities involved in data transmission
 * every customer entity has its own call back function, noted in this array
 */
LOCAL T_DTI_CNTRL_REGISTERED_EXT_CB reg_ext_cb[DTI_MAX_EXT_CB] =
{
  {DTI_ENTITY_INVALID, NULL},
  {DTI_ENTITY_INVALID, NULL},
  {DTI_ENTITY_INVALID, NULL}
};


/*
 * reuse of the global variable g_cur_cap due to the fact that the
 * function dti_cntrl_est_dpath_indirect() is (always) called with the
 * SPLIT mode, which means that there is the calling of
 * dti_cntrl_entity_disconnected() which causes to reset the current
 * capability and then there is the calling of dti_cntrl_entity_connected()
 * and here we need the capability given in dti_cntrl_est_dpath_indirect()
 */
UBYTE g_cur_cap;

/* bit set for DTI ID which should be reconnected to ACI */
ULONG dti_cntrl_reconnect_to_aci;

/*********************** LOCAL FUNCTIONS *****************************/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_search_dev_id |
+--------------------------------------------------------------------+

  PURPOSE : help function for
            find_element_by_tupel (dti_cntrl_list, dev_id, dev_no, sub_no, dti_cntrl_search_dev_id);
*/
LOCAL BOOL dti_cntrl_search_dev_id (T_DTI_ENTITY_ID dev_id, UBYTE dev_no, UBYTE sub_no, void *elem)
{
  T_DTI_CNTRL *compared = (T_DTI_CNTRL *)elem;

  if (compared NEQ NULL)
    if (compared->dev_id EQ dev_id)
      if (compared->dev_no EQ dev_no)
        if (compared->sub_no EQ sub_no
#ifdef _SIMULATION_

OR 0 EQ sub_no AND 255 EQ compared->sub_no

#endif    
          )
          return (TRUE);
  return (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : DTI_CNTRL             |
| STATE   : code                     ROUTINE : find_element_by_tupel |
+--------------------------------------------------------------------+

  PURPOSE : find element in dti_cntrl_list
*/
typedef BOOL  T_LIST_HELP_FCT (T_DTI_ENTITY_ID criterium1, UBYTE criterium2, UBYTE criterium3, void *elem);

LOCAL void *find_element_by_tupel (T_ACI_LIST         *search_list,
                                   T_DTI_ENTITY_ID     criterium1,
                                   UBYTE               criterium2,
                                   UBYTE               criterium3,
                                   T_LIST_HELP_FCT     test_criterium)
{
  T_ACI_LIST *current = NULL;

  if ((search_list EQ NULL) OR (search_list->msg EQ NULL))
  {
    return (NULL);
  }

  current = search_list;

  do
  {
    if (current->msg NEQ NULL)
    {
      if (test_criterium (criterium1, criterium2, criterium3, current->msg))
      {
        return (current->msg);
      }
      else
      {
        current = current->next;
      }
    }
  }while (current NEQ NULL);

  return (NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_call_reg_fct  |
+--------------------------------------------------------------------+

  PURPOSE : this function is called by dti_cntrl_maintain_entity(),
            which is a call back function used by the DTI Connection
            Manager, in such a case, where a customer installed its
            own entity involved during data transmission
*/
LOCAL BOOL dti_cntrl_call_reg_fct( T_DTI_ENTITY_ID    entity_id,
                                   T_DTI_CONN_LINK_ID link_id,
                                   T_DTI_ENTITY_ID    peer_entity_id,
                                   UBYTE              dti_conn )
{
  UBYTE i = 0;

  TRACE_FUNCTION("dti_cntrl_call_reg_fct()");

  for (i=0; i<DTI_MAX_EXT_CB; ++i)
  {
    if (reg_ext_cb[i].ent_id EQ entity_id)
    {
      if (reg_ext_cb[i].fct)
      {
        return (reg_ext_cb[i].fct(link_id, peer_entity_id, dti_conn));
      }
    }
  }
  return (FALSE);
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                         |
| STATE   : code                   ROUTINE : dti_cntrl_set_redirection         |
+------------------------------------------------------------------------------+

  PURPOSE : sub function of dti_cntrl_set_redirect_from_src and
                            dti_cntrl_set_redirect_from_device
*/
LOCAL BOOL dti_cntrl_set_redirection (T_DTI_CNTRL *dti_cntrl_dev,
                                      T_DTI_CNTRL *indirect_dev,
                                      UBYTE        mode,
                                      UBYTE        capability,
                                      UBYTE        cid)
{
  TRACE_FUNCTION("dti_cntrl_set_redirection()");

  switch (capability)
  {
    case (DTI_CPBLTY_PKT):
    {
      if (mode EQ DTI_MODE_DELETE)
      {
        dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].mode        = DTI_MODE_NIL;
        dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].redirection = NULL;
      }
      else
      {
        dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].mode        = mode;
        dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].redirection = indirect_dev;
      }
      break;
    }
    case (DTI_CPBLTY_SER):
    {
      if (mode EQ DTI_MODE_DELETE)
      {
        dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].mode        = DTI_MODE_NIL;
        dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].redirection = NULL;
      }
      else
      {
        dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].mode        = mode;
        dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].redirection = indirect_dev;
      }
      break;
    }
    default:
    {
      TRACE_EVENT("given capability is not SER or PKT");
      return (FALSE);
    }
  }
  return (TRUE);
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                         |
| STATE   : code                   ROUTINE : dti_cntrl_get_redirection         |
+------------------------------------------------------------------------------+

  PURPOSE : sub function of dti_cntrl_get_first_redirection and
                            dti_cntrl_get_next_redirection
*/
LOCAL BOOL dti_cntrl_get_redirection (T_DTI_CNTRL *dti_cntrl_dev,
                                      T_DTI_CNTRL *indirection,  /* out parameter */
                                      UBYTE        cid,
                                      UBYTE        capability)
{
  UBYTE mode = DTI_MODE_NIL; /* remember temporary */

  TRACE_FUNCTION("dti_cntrl_get_redirection()");

  switch (capability)
  {
    case (DTI_CPBLTY_PKT):
    {
      for (cid += 1; cid<DTI_MAX_REDIRECTIONS; cid++) /* watch the start value */
      {
        if (dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].redirection)
        {
          mode = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].mode;
          /*
           * set the pointer to the redirected device for packet data transmission
           */
          dti_cntrl_dev = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].redirection;
          break; 
        }
      }
      break;
    }
    case (DTI_CPBLTY_SER):
    {
      for (cid += 1; cid<DTI_MAX_REDIRECTIONS; cid++) /* watch the start value */
      {
        if (dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].redirection)
        {
          mode = dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].mode;
          /*
           * set the pointer to the redirected device for serial data transmission
           */
          dti_cntrl_dev = dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].redirection;
          break;
        }
      }
      break;
    }
    default:
    {
      TRACE_EVENT("given capability is not SER or PKT");
      indirection = NULL; /* make it invalid */
      return (FALSE);
    }
  }

  if (cid >= DTI_MAX_REDIRECTIONS)
  {
    indirection = NULL; /* there is no further redirection */
  }
  else
  {
    /*
     * fill the out parameter
     */
    indirection->dev_id        = dti_cntrl_dev->dev_id;
    indirection->dev_no        = dti_cntrl_dev->dev_no;
    indirection->sub_no        = dti_cntrl_dev->sub_no;
    indirection->capability    = dti_cntrl_dev->capability;
    indirection->src_id        = dti_cntrl_dev->src_id;
    indirection->dti_id        = dti_cntrl_dev->dti_id;
    indirection->port_number   = dti_cntrl_dev->port_number;
    indirection->cur_cap       = dti_cntrl_dev->cur_cap;
    indirection->driver_id     = dti_cntrl_dev->driver_id;
    indirection->dio_ctrl_id   = dti_cntrl_dev->dio_ctrl_id;
    /*
     * ! important ! with >>DTI_DEV_I_AM_THE_ONE<< we indicate that the
     * redirect_info is now _actually_ the info about this device
     */
    indirection->redirect_info.info.cid        = cid;
    indirection->redirect_info.info.mode       = mode;
    indirection->redirect_info.info.capability = capability;
    indirection->redirect_info.info.direction  = DTI_DEV_I_AM_THE_ONE;
  }
  return (TRUE);
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                         |
| STATE   : code                   ROUTINE : dti_cntrl_get_info                |
+------------------------------------------------------------------------------+

  PURPOSE : sub function of the three functions
            dti_cntrl_get_info_from_src_id
            dti_cntrl_get_info_from_dti_id
            dti_cntrl_get_info_from_dev_id
*/
LOCAL BOOL dti_cntrl_get_info (T_DTI_CNTRL  *info, /* out parameter */
                               T_DTI_CNTRL  *dti_cntrl_dev)
{
  UBYTE  i  = 0;

  TRACE_FUNCTION("dti_cntrl_get_info()");

  info->dev_id        = dti_cntrl_dev->dev_id;
  info->dev_no        = dti_cntrl_dev->dev_no;
  info->sub_no        = dti_cntrl_dev->sub_no;
  info->capability    = dti_cntrl_dev->capability;
  info->src_id        = dti_cntrl_dev->src_id;
  info->dti_id        = dti_cntrl_dev->dti_id;
  info->port_number   = dti_cntrl_dev->port_number;
  info->cur_cap       = dti_cntrl_dev->cur_cap;
  info->driver_id     = dti_cntrl_dev->driver_id;
  info->dio_ctrl_id   = dti_cntrl_dev->dio_ctrl_id;
  /*
   * init a possible indirection info with default values ( all 0xFF )
   */
  info->redirect_info.info.cid        = DTI_CID_NOTPRESENT;
  info->redirect_info.info.mode       = DTI_MODE_NIL;
  info->redirect_info.info.capability = DTI_CPBLTY_NO;
  info->redirect_info.info.direction  = DTI_DIRECTION_NOTPRESENT;
  /*
   * search, whether there is a redirection, start with packet device
   */
  for (i=0; i<DTI_MAX_REDIRECTIONS; i++)
  {
    if (dti_cntrl_dev->redirect_info.tbl->pkt_redirect[i].redirection)
    {
      break;
    }
  }
  if (i < DTI_MAX_REDIRECTIONS)
  {
    info->redirect_info.info.cid        = i;
    info->redirect_info.info.mode       = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[i].mode;
    info->redirect_info.info.capability = DTI_CPBLTY_PKT;
    info->redirect_info.info.direction  = DTI_DEV_IS_REDIRECTED;
  }
  /*
   * there is no redirection to a packet device, so search for redirection to a serial device
   */
  for (i=0; i<DTI_MAX_REDIRECTIONS; i++)
  {
    if (dti_cntrl_dev->redirect_info.tbl->ser_redirect[i].redirection)
    {
      break;
    }
  }
  if (i < DTI_MAX_REDIRECTIONS)
  {
    info->redirect_info.info.cid        = i;
    info->redirect_info.info.mode       = dti_cntrl_dev->redirect_info.tbl->ser_redirect[i].mode;
    info->redirect_info.info.capability = DTI_CPBLTY_SER;
    info->redirect_info.info.direction  = DTI_DEV_IS_REDIRECTED;
  }
  return (TRUE);
}


/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                         |
| STATE   : code                   ROUTINE : dti_cntrl_maintain_entity_connect |
+------------------------------------------------------------------------------+

  PURPOSE : sub function of dti_cntrl_maintain_entity, called when the DTI
            Connection Manager wants to connect entites for data transmission

*/
LOCAL BOOL dti_cntrl_maintain_entity_connect ( T_DTI_CONN_LINK_ID  link_id,
                                               T_DTI_ENTITY_ID     cur_ent_id,
                                               T_DTI_ENTITY_ID     peer_ent_id,
                                               UBYTE               dti_conn)
{
  TRACE_FUNCTION("dti_cntrl_maintain_entity_connect()");

#ifdef FF_TCP_IP
  /* If the peer entity is AAA, delay the connect request until AAA has
   * responded to the AAA_DTI_IND. ati_src_riv_dti_rsp() calls
   * dti_cntrl_connect_after_aaa_dti_rsp(), which in turn calls this function,
   * on receiving the response. */
  {
    if (peer_ent_id EQ DTI_ENTITY_AAA)
    {
      T_DTI_CNTRL *dti_cntrl_dev; /* Information about DTI link. */
      UBYTE dti_id ;              /* DTI id of this connection. */

      dti_id = EXTRACT_DTI_ID(link_id) ;
      dti_cntrl_dev = (T_DTI_CNTRL *) find_element (dti_cntrl_list, dti_id,
                                                    dti_cntrl_search_dti_id) ;
      if (!dti_cntrl_dev->had_aaa_dti_rsp)
      {
        dti_cntrl_dev->save_link_id = link_id ;
        dti_cntrl_dev->save_cur_ent_id = cur_ent_id ;
        dti_cntrl_dev->save_dti_conn = dti_conn ;
        return TRUE ;
      }
      dti_cntrl_dev->had_aaa_dti_rsp = FALSE;

    }
  }
#endif /* FF_TCP_IP */

  switch ( cur_ent_id )
  {
  case DTI_ENTITY_ACI: /* entity ACI */
    /* handle ACI as independent entity */
    psaACI_Dti_Req(link_id, peer_ent_id, ACI_CONNECT_DTI);
    break;

  case DTI_ENTITY_UART: /* entity UART */
    psaUART_SetDTIReq (link_id, peer_ent_id);
    break;

#ifdef FF_TRACE_OVER_MTST
  case DTI_ENTITY_MTST: /* for traces */
    psaMTST_Switch_Trace (link_id, peer_ent_id, TRUE);
    break;
#endif

#ifdef FAX_AND_DATA
  case DTI_ENTITY_TRA:
    psaTRA_Dti_Req( link_id, TRA_CONNECT_DTI, (UBYTE)peer_ent_id );
    break;

  case DTI_ENTITY_L2R:
    psaL2R_Enable (link_id, (UBYTE)peer_ent_id);
    break;

#ifdef FF_FAX
  case DTI_ENTITY_T30:
    psaT30_Dti_Req( link_id, T30_CONNECT_DTI );
    break;
#endif

#ifdef BT_ADAPTER
  case DTI_ENTITY_BLUETOOTH:
    psaBTI_Dti_Req( link_id, peer_ent_id, BTI_CONNECT_DTI );
    break;
#endif /* BT_ADAPTER */

#endif /* FAX_AND_DATA */

#ifdef GPRS
  case DTI_ENTITY_PPPS:
    psaGPPPS_Dti_Req(link_id, (UBYTE) peer_ent_id); /* rather use dti_id instead of link_id ???? */
    break;
#ifdef FF_PKTIO
  case DTI_ENTITY_PKTIO:
    psaPKT_Dti_Req((ULONG)link_id, (UBYTE)peer_ent_id, PKT_CONNECT_DTI);
    break;
#endif /* FF_PKTIO */
  case DTI_ENTITY_SNDCP:
    psa_sn_dti_req( link_id, peer_ent_id, NAS_CONNECT_DTI );
    break;
#endif /* GPRS */
#ifdef FF_PSI
  case DTI_ENTITY_PSI:
    psaPSI_Dti_Req((ULONG)link_id, (UBYTE)peer_ent_id, PSI_CONNECT_DTI);
    break;
#endif /*FF_PSI*/
  case DTI_ENTITY_NULL:
    dti_cntrl_entity_connected (link_id, DTI_ENTITY_NULL, DTI_OK);
    break;

#if defined (FF_WAP) || defined (FF_PPP) || defined (FF_GPF_TCPIP) || defined (FF_SAT_E) 
  case DTI_ENTITY_PPPC:
    psaPPP_Establish(link_id, (UBYTE) peer_ent_id);
    break;
#endif /* WAP or FF_PPP or SAT E */
#ifdef CO_UDP_IP 
  case DTI_ENTITY_IP:
    psaIPA_Dti_Req(link_id, (UBYTE) peer_ent_id, IPA_CONNECT_DTI);
    break;

  case DTI_ENTITY_UDP:
    psaUDPA_Dti_Req(link_id, (UBYTE) peer_ent_id, UDPA_CONNECT_DTI);
    break;

#ifdef FF_WAP 
  case DTI_ENTITY_WAP:
    psaWAP_Dti_Req(link_id, (UBYTE) peer_ent_id, WAP_CONNECT_DTI);
    break;
#endif /* WAP */

#endif /* WAP || SAT E */

#ifdef FF_GPF_TCPIP
  case DTI_ENTITY_TCPIP:
    psaTCPIP_Dti_Req(link_id, (UBYTE) peer_ent_id, TCPIP_CONNECT_DTI);
    break;
#endif

#ifdef FF_TCP_IP
  case DTI_ENTITY_AAA: /* entity AAA */
    psaAAA_dti_ind (link_id, peer_ent_id);
    break;
#endif

#ifdef FF_SAT_E
    /* SAT class e */
  case DTI_ENTITY_SIM:
    /* init connection SIM--PEER */ 
    /* open DTI connection */
    psaSIM_Dti_Req(link_id);
    break;
#endif

  default:
    /* search for registered connect function */
    return dti_cntrl_call_reg_fct(cur_ent_id, link_id, peer_ent_id, dti_conn);
  }
  return (TRUE);
}

#ifdef FF_TCP_IP
EXTERN void dti_cntrl_connect_after_aaa_dti_rsp(UBYTE dti_id)
{
  /* If the peer entity is AAA, the connect request has been delayed, until
   * AAA has responded to the AAA_DTI_IND. ati_src_riv_dti_rsp() calls this
   * function on receiving this response. */

  T_DTI_CNTRL *dti_cntrl_dev ;  /* Information about DTI link. */

  TRACE_FUNCTION("dti_cntrl_connect_after_aaa_dti_rsp()");
  dti_cntrl_dev = (T_DTI_CNTRL *) find_element (dti_cntrl_list, dti_id,
                                                dti_cntrl_search_dti_id) ;
  dti_cntrl_dev->had_aaa_dti_rsp = TRUE ;
  dti_cntrl_maintain_entity_connect(dti_cntrl_dev->save_link_id,
                                    dti_cntrl_dev->save_cur_ent_id,
                                    DTI_ENTITY_AAA,
                                    dti_cntrl_dev->save_dti_conn) ;
}
#endif /* FF_TCP_IP */

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                            |
| STATE   : code                   ROUTINE : dti_cntrl_maintain_entity_disconnect |
+---------------------------------------------------------------------------------+

  PURPOSE : sub function of dti_cntrl_maintain_entity, called when the DTI
            Connection Manager wants to disconnect entites after data transmission
*/
LOCAL BOOL dti_cntrl_maintain_entity_disconnect ( T_DTI_CONN_LINK_ID  link_id,
                                                  T_DTI_ENTITY_ID     cur_ent_id,
                                                  T_DTI_ENTITY_ID     peer_ent_id,
                                                  UBYTE               dti_conn)
{
  TRACE_FUNCTION("dti_cntrl_maintain_entity_disconnect()");
  switch( cur_ent_id )
  {
  case DTI_ENTITY_ACI:
    psaACI_Dti_Req (link_id, peer_ent_id, ACI_DISCONNECT_DTI);
    break;

  case DTI_ENTITY_UART:
    /* for UART, we have to wait for UART_DTI_IND
     * sent when UART recognizes it's dti connection has been
     * closed down.
     * It would be nice to just send UART_DTI_REQ
     * here in order to make handling of the UART entity
     * more similar to the other entities..
     */
    break;

#ifdef FF_TRACE_OVER_MTST
  case DTI_ENTITY_MTST: /* for traces */
    if (psaMTST_Switch_Trace (link_id, peer_ent_id, FALSE) EQ PEI_ERROR)
    {
      return (FALSE);
    }
    break;
#endif

#ifdef FAX_AND_DATA
  case DTI_ENTITY_TRA:
    psaTRA_Dti_Req (link_id, TRA_DISCONNECT_DTI, (UBYTE)peer_ent_id);
    break;

#ifdef FF_FAX
  case DTI_ENTITY_T30:
    psaT30_Dti_Req (link_id, T30_DISCONNECT_DTI);
    break;
#endif

  case DTI_ENTITY_L2R:
    /* in the case of disconnecting L2R,
    it has to be DEACTIVATED */
    psaL2R_Deactivate();
    break;

#ifdef BT_ADAPTER
  case DTI_ENTITY_BLUETOOTH:
    psaBTI_Dti_Req(link_id, peer_ent_id, BTI_DISCONNECT_DTI);
    break;
#endif /* BT_ADAPTER */

#endif  /* FAX_AND_DATA */

#ifdef GPRS
  case DTI_ENTITY_PPPS: 	
/* Do nothing. Termination of PPP is done by the CMH_SM or CMH_GPPP */
    break;

  case DTI_ENTITY_SNDCP:
    psa_sn_dti_req( link_id, peer_ent_id, NAS_DISCONNECT_DTI );
    break;
#ifdef FF_PKTIO
   case DTI_ENTITY_PKTIO:
     psaPKT_Dti_Req((ULONG)link_id, (UBYTE)peer_ent_id, PKT_DISCONNECT_DTI);
     break;
#endif /* FF_PKTIO */
#endif /* GPRS */

#ifdef FF_PSI
   case DTI_ENTITY_PSI:
     psaPSI_Dti_Req((ULONG)link_id, (UBYTE)peer_ent_id, PSI_DISCONNECT_DTI);
     break;
#endif /*FF_PSI*/

#ifdef CO_UDP_IP
  case DTI_ENTITY_IP:
    psaIPA_Dti_Req(link_id, (UBYTE)peer_ent_id, IPA_DISCONNECT_DTI);
    break;

  case DTI_ENTITY_UDP:
    /* for UDP, we have to wait for UDPA_DTI_IND(DISCONNECT) which is
     * sent when UDP recognizes it's dti connection has been
     * closed down
     */
    break;
#endif /* CO_UDP_IP */

#ifdef FF_WAP
  case DTI_ENTITY_WAP:
    psaWAP_Dti_Req(link_id, (UBYTE) peer_ent_id, WAP_DISCONNECT_DTI);
    break;
#endif /* WAP */

#ifdef FF_GPF_TCPIP
  case DTI_ENTITY_TCPIP:
    /* for TCP/IP, we have to wait for TCPIP_DTI_IND(DISCONNECT) which is
     * sent when TCP/IP recognizes its DTI connection has been closed down */
    psaTCPIP_Dti_Req(link_id, (UBYTE) peer_ent_id, TCPIP_DISCONNECT_DTI);
    break;
#endif /* FF_GPF_TCPIP */

#ifdef FF_TCP_IP
  case DTI_ENTITY_AAA: /* entity AAA */
    psaAAA_disconnect_ind (link_id);
    break;
#endif
#ifdef FF_PPP
  case DTI_ENTITY_PPPC:
    psaPPP_Terminate(UP);
    break;
#endif

#ifdef FF_SAT_E
    /* SAT class e */
  case DTI_ENTITY_SIM:
    psaSIM_Dti_Req(link_id);
    break;
#endif

  default:
    /* search for registered connect function */
    return dti_cntrl_call_reg_fct(cur_ent_id, link_id, peer_ent_id, dti_conn);
  }

  return (TRUE);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                 |
| STATE   : code                   ROUTINE : dti_cntrl_maintain_entity |
+----------------------------------------------------------------------+

  PURPOSE : this function is used by the DTI Connection Manager as a
            call back function for connecting/disconnecting.
            The registeration of dti_cntrl_maintain_entity in the
            DTI Connection Manager takes place with:
              dti_conn_init (dti_cntrl_maintain_entity);
*/
LOCAL BOOL dti_cntrl_maintain_entity ( T_DTI_CONN_LINK_ID  link_id,
                                       T_DTI_ENTITY_ID     cur_ent_id,
                                       T_DTI_ENTITY_ID     peer_ent_id,
                                       UBYTE               dti_conn)
{
  TRACE_FUNCTION("dti_cntrl_maintain_entity()");

  if (dti_conn EQ DTI_CONNECT)
  {
    return (dti_cntrl_maintain_entity_connect (link_id, cur_ent_id, peer_ent_id, dti_conn));
  }
  else /* DTI_DISCONNECT */
  {
    return (dti_cntrl_maintain_entity_disconnect (link_id, cur_ent_id, peer_ent_id, dti_conn));
  }
}
/********************** GLOBAL FUNCTIONS *****************************/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_search_src_id |
+--------------------------------------------------------------------+

  PURPOSE : help function for
            find_element (dti_cntrl_list, dti_id, dti_cntrl_search_src_id);
*/
GLOBAL BOOL dti_cntrl_search_src_id (UBYTE src_id, void *elem)
{
  T_DTI_CNTRL *compared = (T_DTI_CNTRL *)elem;

  if (compared NEQ NULL)
    if (compared->src_id EQ src_id)
      return (TRUE);
  return (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_search_dti_id |
+--------------------------------------------------------------------+

  PURPOSE : help function for
            find_element (dti_cntrl_list, dti_id, dti_cntrl_search_dti_id);
*/
GLOBAL BOOL dti_cntrl_search_dti_id (UBYTE dti_id, void *elem)
{
  T_DTI_CNTRL *compared = (T_DTI_CNTRL *)elem;

  if (compared NEQ NULL)
    if (compared->dti_id EQ dti_id)
      return (TRUE);
  return (FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_init          |
+--------------------------------------------------------------------+

  PURPOSE : create the global defined dti_cntrl_list and
            register dti_cntrl_maintain_entity as call back
*/
GLOBAL void dti_cntrl_init (void)
{
  UBYTE i;

  TRACE_FUNCTION("dti_cntrl_init()");

  for (i=0; i<MAX_DTI_CONN_LINK_IDS; i++)
  {
    dti_aci_data_base[i].link_id  = DTI_LINK_ID_NOTPRESENT;
    dti_aci_data_base[i].dev_no1  = DTI_DEV_NO_NOTPRESENT;
    dti_aci_data_base[i].dev_no2  = DTI_DEV_NO_NOTPRESENT;
    dti_aci_data_base[i].ent_id1  = DTI_ENTITY_INVALID;
    dti_aci_data_base[i].ent_id2  = DTI_ENTITY_INVALID;
    dti_aci_data_base[i].sub_no1  = DTI_SUB_NO_NOTPRESENT;
    dti_aci_data_base[i].sub_no2  = DTI_SUB_NO_NOTPRESENT;
  }

  dti_cntrl_reconnect_to_aci = 0;

  dti_cntrl_list = new_list ();

  dti_conn_init (dti_cntrl_maintain_entity);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_new_dti       |
+--------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL UBYTE  dti_cntrl_new_dti (UBYTE  dti_id)
{

  TRACE_FUNCTION("dti_cntrl_new_dti()");

  /* call DTI connection manager function to get a DTI ID */
  if ((dti_id = dti_conn_new(dti_id)) EQ DTI_DTI_ID_NOTPRESENT)
  {
    TRACE_EVENT("couldn't create new dti_id");
    return (FALSE);
  }

  return (dti_id);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_new_device    |
+--------------------------------------------------------------------+

  PURPOSE : a PSA wants to set up a device.
            in case of ATI_RIV called this function, then port_num
            is meaningful number, else port_num = NO_PORT_NUMBER.
            we keep the port number here to frees ATI-RIV from
            maintaining its own redundant list.
*/
GLOBAL BOOL   dti_cntrl_new_device (UBYTE  src_id,
                                    UBYTE  dev_id,
                                    UBYTE  dev_no,
                                    UBYTE  sub_no,
                                    UBYTE  port_num,
                                    UBYTE  capability,
                                    UBYTE  driver_id,
                                    UBYTE  dio_ctrl_id)
{
  UBYTE         dti_id         = DTI_DTI_ID_NOTPRESENT;
  T_DTI_CNTRL  *dti_cntrl_dev  = NULL;
  USHORT        i              = 0;

  TRACE_FUNCTION("dti_cntrl_new_device()");

  /* call DTI connection manager function to get a DTI ID */
  if ((dti_id = dti_conn_new(DTI_DTI_ID_NOTPRESENT)) EQ DTI_DTI_ID_NOTPRESENT)
  {
    TRACE_EVENT("couldn't create new dti_id");
    return (FALSE);
  }

  ACI_MALLOC (dti_cntrl_dev, sizeof (T_DTI_CNTRL));
  ACI_MALLOC (dti_cntrl_dev->redirect_info.tbl, sizeof (T_DTI_CNTRL_REDIRECT_INTERN));

  dti_cntrl_dev->dev_id      = (T_DTI_ENTITY_ID)dev_id;
  dti_cntrl_dev->dev_no      = dev_no;
  dti_cntrl_dev->sub_no      = sub_no;        /* multiplexed sub channels          */
  dti_cntrl_dev->capability  = capability;    /* what the device is capable to     */
  dti_cntrl_dev->src_id      = src_id;
  dti_cntrl_dev->dti_id      = dti_id;
  dti_cntrl_dev->port_number = port_num;
  dti_cntrl_dev->driver_id   = driver_id;
  dti_cntrl_dev->dio_ctrl_id = dio_ctrl_id;

  #ifdef FF_TCP_IP
  dti_cntrl_dev->had_aaa_dti_rsp = FALSE ;
  dti_cntrl_dev->save_link_id    = 0 ;
  dti_cntrl_dev->save_cur_ent_id = 0 ;
  dti_cntrl_dev->save_dti_conn   = 0 ;
  #endif /* FF_TCP_IP */
  
  if ((src_id NEQ DTI_SRC_ID_NOTPRESENT) AND (src_id <= CMD_SRC_MAX))
  {
    dti_cntrl_dev->cur_cap   = DTI_CPBLTY_CMD; /* has been called by an AT src, so at least CMD capability */
  }
  else
  {
    dti_cntrl_dev->cur_cap   = DTI_CPBLTY_NO;  /* has not been called by an AT src and there is no connection yet */
  }

  for (i=0; i<DTI_MAX_REDIRECTIONS; ++i)
  {
    dti_cntrl_dev->redirect_info.tbl->ser_redirect[i].mode        = DTI_MODE_PERM;
    dti_cntrl_dev->redirect_info.tbl->ser_redirect[i].redirection = NULL;
    dti_cntrl_dev->redirect_info.tbl->pkt_redirect[i].mode        = DTI_MODE_PERM;
    dti_cntrl_dev->redirect_info.tbl->pkt_redirect[i].redirection = NULL;
  }

  return (insert_list (dti_cntrl_list, dti_cntrl_dev));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_new_device    |
+--------------------------------------------------------------------+

  PURPOSE : change the sub channel number (for multiplexer)
*/
GLOBAL void dti_cntrl_change_sub_no (UBYTE  src_id,
                                     UBYTE  sub_no)
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);

  TRACE_FUNCTION("dti_cntrl_change_sub_no()");

  if (dti_cntrl_dev NEQ NULL)
  {
    dti_cntrl_dev->sub_no = sub_no; /* multiplexed sub channels */
  }
  else
  {
    TRACE_EVENT_P1("no registered device found for src_id=%u", src_id);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_est_dpath     |
+--------------------------------------------------------------------+

  PURPOSE : establish a data connection path directly

*/
GLOBAL BOOL dti_cntrl_est_dpath ( UBYTE               dti_id,
                                  T_DTI_ENTITY_ID    *entity_list,
                                  UBYTE               num_entities,
                                  T_DTI_CONN_MODE     mode,         /* split/append */
                                  T_DTI_CONN_CB      *cb)
{
  TRACE_FUNCTION("dti_cntrl_est_dpath()");

  return (dti_conn_est_dpath (dti_id, entity_list, num_entities, mode, cb));
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                    |
| STATE   : code                   ROUTINE : dti_cntrl_est_dpath_indirect |
+-------------------------------------------------------------------------+

  PURPOSE : establish a data connection path, for which a redirection has been set up.
            a redirection relates to a serial or packet device indicated by capability.
            with the cid we find in ser_redirect[cid] respectively pkt_redirect[cid]
            the pointer to the redirected device maintenance in dti_cntrl_list, from
            which we get the dti_id to pass it to the DTI Connection Manager.
*/
GLOBAL BOOL   dti_cntrl_est_dpath_indirect (UBYTE               src_id,
                                            T_DTI_ENTITY_ID    *entity_list,
                                            UBYTE               num_entities,
                                            T_DTI_CONN_MODE     mode,         /* split/append */
                                            T_DTI_CONN_CB      *cb,
                                            UBYTE               capability,
                                            UBYTE               cid)
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);
  T_DTI_CNTRL  *indirect_dev   = NULL; /* the pointer to the redirected device maintenance */
  UBYTE         dti_id         = DTI_DTI_ID_NOTPRESENT;
  UBYTE         mode_once_perm = DTI_MODE_NIL;
  T_DTI_ENTITY_ID  *new_entity_list;
  BOOL ret_value;

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  extern CHAR gob_tst_buf[];
#endif

  TRACE_FUNCTION("dti_cntrl_est_dpath_indirect()");

  if (dti_cntrl_dev EQ NULL)
  {
    TRACE_EVENT_P1("[ERR]Device with src_id=%d was not registered", src_id);
    return FALSE;
  }

  ACI_MALLOC(new_entity_list, sizeof(T_DTI_ENTITY_ID)*(num_entities+1) );
  memcpy(&new_entity_list[1], entity_list, sizeof(T_DTI_ENTITY_ID)*num_entities);

  if (cid EQ DTI_CID_NOTPRESENT)
  {
    cid = 0;
  }

  /*
   * requested capability for data transmission can be SER ,PKT or CMD
   */
  switch (capability)
  {
    case (DTI_CPBLTY_PKT):
    {
      mode_once_perm        = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].mode;
      indirect_dev          = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[cid].redirection;

      /*
       * if there is no redirection for the given cid (so it is a GPRS call)
       * then look in the "redirection for all cids" entry
       */
      if ((indirect_dev EQ NULL) AND (cid NEQ 0))
      {
        mode_once_perm        = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[0].mode;
        indirect_dev          = dti_cntrl_dev->redirect_info.tbl->pkt_redirect[0].redirection;
      }

      if(indirect_dev NEQ NULL)
      {
        indirect_dev->cur_cap = DTI_CPBLTY_PKT;
      }
      break;
    }
    case (DTI_CPBLTY_SER):
    {
      mode_once_perm        = dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].mode;
      indirect_dev          = dti_cntrl_dev->redirect_info.tbl->ser_redirect[cid].redirection;

      /*
       * if there is no redirection for the given cid (so it is a GPRS call)
       * then look in the "redirection for all cids" entry
       */
      if ((indirect_dev EQ NULL) AND (cid NEQ 0))
      {
        mode_once_perm        = dti_cntrl_dev->redirect_info.tbl->ser_redirect[0].mode;
        indirect_dev          = dti_cntrl_dev->redirect_info.tbl->ser_redirect[0].redirection;
#ifdef RMV_15_04_03
        TRACE_EVENT("redirection used");
        TRACE_EVENT_P1("cid: %d", cid);
#endif
      }

      if(indirect_dev NEQ NULL)
      {
      	indirect_dev->cur_cap = DTI_CPBLTY_SER;
      }
      break;
    }
    case (DTI_CPBLTY_CMD):
      dti_cntrl_dev->cur_cap = DTI_CPBLTY_CMD;
      break;
    default:
    {
      return (FALSE);
    }
  }

  if (indirect_dev EQ NULL)
  {
    /* verify that the device is able to work in the requested capability */
    if ((BITFIELD_CHECK (capability, dti_cntrl_dev->capability)) EQ FALSE)
    {
      return (FALSE);
    }

    dti_id = dti_cntrl_dev->dti_id;
    new_entity_list[0] = dti_cntrl_dev->dev_id;
  }
  else
  {
    /* verify that the device is able to work in the requested capability */
    if ((BITFIELD_CHECK (capability, indirect_dev->capability)) EQ FALSE)
    {
      return (FALSE);
    }

    dti_id = indirect_dev->dti_id;
    new_entity_list[0] = indirect_dev->dev_id;
    if (mode_once_perm EQ DTI_MODE_ONCE)
    {
      /* the redirected data transmission connection is not permanent,
       * so remove the pointer to the redirected device maintenance */
      indirect_dev = NULL;
    }
  }

  num_entities++;
  /*
   * remember capability, we need it again in dti_cntrl_entity_connected()
   */
  g_cur_cap = capability;

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  TRACE_EVENT_P1("num_entities: %d", num_entities);
  TRACE_EVENT_P1("g_cur_cap: %d", g_cur_cap);
  TRACE_EVENT_P1("new_entity_list[0]: %d", new_entity_list[0]);
  TRACE_EVENT_P1("new_entity_list[1]: %d", new_entity_list[1]);
  TRACE_EVENT_P1("new_entity_list[2]: %d", new_entity_list[2]);
  TRACE_EVENT_P1("dti_cntrl_dev->dti_id: %d", dti_cntrl_dev->dti_id);
  if (indirect_dev)
  {
    TRACE_EVENT_P1("indirect_dev->dti_id: %d", indirect_dev->dti_id);
  }

  TRACE_EVENT(gob_tst_buf);
#endif

  /* clear reconnect flag for the new connction */
  dti_cntrl_clear_dti_id_to_reconnect(dti_id);

  ret_value = dti_conn_est_dpath (dti_id, new_entity_list, num_entities, mode, cb);

  ACI_MFREE(new_entity_list);
  return ret_value;
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411) MODULE  : DTI_CNTRL                           |
| STATE   : code           ROUTINE : dti_cntrl_is_dti_channel_connected  |
+------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL dti_cntrl_is_dti_channel_connected (T_DTI_ENTITY_ID ent_id, UBYTE dti_id)
{
  TRACE_FUNCTION("dti_cntrl_is_dti_channel_connected()");

  return (dti_conn_is_dti_channel_connected (ent_id, dti_id));
}

/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411) MODULE  : DTI_CNTRL                             |
| STATE   : code           ROUTINE : dti_cntrl_is_dti_channel_disconnected |
+--------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL dti_cntrl_is_dti_channel_disconnected (UBYTE dti_id)
{
  TRACE_FUNCTION("dti_cntrl_is_dti_channel_disconnected()");

  return (dti_conn_is_dti_channel_disconnected (dti_id));
}
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411) MODULE  : DTI_CNTRL                           |
| STATE   : code           ROUTINE : dti_cntrl_close_dpath_from_src_id   |
+------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL dti_cntrl_close_dpath_from_src_id (UBYTE  src_id)
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);

  TRACE_FUNCTION("dti_cntrl_close_dpath_from_src_id()");

  return (dti_conn_close_dpath (dti_cntrl_dev->dti_id));
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411) MODULE  : DTI_CNTRL                           |
| STATE   : code           ROUTINE : dti_cntrl_close_dpath_from_dti_id   |
+------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_close_dpath_from_dti_id (UBYTE  dti_id)
{
  TRACE_FUNCTION("dti_cntrl_close_dpath_from_dti_id()");

  return (dti_conn_close_dpath (dti_id));
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                       |
| STATE   : code                   ROUTINE : dti_cntrl_get_info_from_src_id  |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_get_info_from_src_id (UBYTE         src_id,
                                              T_DTI_CNTRL  *info) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);

  TRACE_FUNCTION("dti_cntrl_get_info_from_src_id()");

  if (dti_cntrl_dev)
  {
    return (dti_cntrl_get_info (info, dti_cntrl_dev));
  }
  return (FALSE);
}
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                       |
| STATE   : code                   ROUTINE : dti_cntrl_get_info_from_dti_id  |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_get_info_from_dti_id (UBYTE         dti_id,
                                              T_DTI_CNTRL  *info) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev = (T_DTI_CNTRL *)find_element (dti_cntrl_list, dti_id, dti_cntrl_search_dti_id);

  TRACE_FUNCTION("dti_cntrl_get_info_from_dti_id()");

  if (dti_cntrl_dev)
  {
    return (dti_cntrl_get_info (info, dti_cntrl_dev));
  }
  return (FALSE);
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                       |
| STATE   : code                   ROUTINE : dti_cntrl_get_info_from_dev_id  |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_get_info_from_dev_id (T_DTI_ENTITY_ID      dev_id,
                                              UBYTE                dev_no,
                                              UBYTE                sub_no,
                                              T_DTI_CNTRL         *info) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev = (T_DTI_CNTRL *)find_element_by_tupel (dti_cntrl_list,
                                                                      dev_id,
                                                                      dev_no,
                                                                      sub_no,
                                                                      dti_cntrl_search_dev_id);
  TRACE_FUNCTION("dti_cntrl_info_from_dev_id()");

  if (dti_cntrl_dev)
  {
    return (dti_cntrl_get_info (info, dti_cntrl_dev));
  }
  return (FALSE);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                        |
| STATE   : code                   ROUTINE : dti_cntrl_set_redirect_from_src  |
+-----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_set_redirect_from_src    (UBYTE               src_id,
                                                  UBYTE               mode,
                                                  T_DTI_ENTITY_ID     dst_dev_id,
                                                  UBYTE               dst_dev_no,
                                                  UBYTE               dst_sub_no,
                                                  UBYTE               capability,
                                                  UBYTE               cid)
{
  T_DTI_CNTRL  *dti_cntrl_dev = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);
  T_DTI_CNTRL  *indirect_dev  = (T_DTI_CNTRL *)find_element_by_tupel (dti_cntrl_list,
                                                                      dst_dev_id,
                                                                      dst_dev_no,
                                                                      dst_sub_no,
                                                                      dti_cntrl_search_dev_id);

  TRACE_FUNCTION("dti_cntrl_set_redirect_from_src()");

  if (dti_cntrl_dev AND indirect_dev)
  {
    return (dti_cntrl_set_redirection (dti_cntrl_dev, indirect_dev, mode, capability, cid));
  }

  if (dti_cntrl_dev EQ 0)
  {
    TRACE_EVENT_P1("dti_cntrl_set_redirect_from_src():[ERR] no list element for src_id=%d", src_id);
  }
  if (indirect_dev EQ 0)
  {
    TRACE_EVENT_P3("dti_cntrl_set_redirect_from_src():[ERR] no list element for dev_id=%d, dev_no=%d, sub_no=%d", dst_dev_id,
                                                                                                                  dst_dev_no,
                                                                                                                  dst_sub_no);
  }
  return (FALSE);
}

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                           |
| STATE   : code                   ROUTINE : dti_cntrl_set_redirect_from_device  |
+--------------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL   dti_cntrl_set_redirect_from_device (UBYTE               mode,
                                                  T_DTI_ENTITY_ID     dst_dev_id,
                                                  UBYTE               dst_dev_no,
                                                  UBYTE               dst_sub_no,
                                                  T_DTI_ENTITY_ID     src_dev_id,
                                                  UBYTE               src_dev_no,
                                                  UBYTE               src_sub_no,
                                                  UBYTE               capability,
                                                  UBYTE               cid)
{
  T_DTI_CNTRL  *source_dev = (T_DTI_CNTRL *)find_element_by_tupel (dti_cntrl_list,
                                                                      src_dev_id,
                                                                      src_dev_no,
                                                                      src_sub_no,
                                                                      dti_cntrl_search_dev_id);
  T_DTI_CNTRL  *destination_dev = (T_DTI_CNTRL *)find_element_by_tupel (dti_cntrl_list,
                                                                        dst_dev_id,
                                                                        dst_dev_no,
                                                                        dst_sub_no,
                                                                        dti_cntrl_search_dev_id);

  TRACE_FUNCTION("dti_cntrl_set_redirect_from_device()");

  if (source_dev AND destination_dev)
  {
    if ((BITFIELD_CHECK (source_dev->capability, DTI_CPBLTY_CMD)) EQ FALSE)
    {
      TRACE_EVENT("source device has no command capability !");
      return (FALSE);
    }

    return (dti_cntrl_set_redirection (source_dev, destination_dev, mode, capability, cid));
  }
  return (FALSE);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)           MODULE  : DTI_CNTRL                      |
| STATE   : code                     ROUTINE : dti_cntrl_get_first_device     |
+-----------------------------------------------------------------------------+

  PURPOSE :   this function is used in conjunction with
                     1:AT%DATA where all devices
                       and their possible redirections for data transmission have to
                       be listed. so start with the first element of dti_cntrl_list
                     2.AT%DINF where all devices have to be listed, start with first element
*/
GLOBAL BOOL   dti_cntrl_get_first_device (T_DTI_CNTRL  *info) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev;

  TRACE_FUNCTION("dti_cntrl_get_first_device()");

  dti_cntrl_dev_ptr = dti_cntrl_list;
  dti_cntrl_dev  = (T_DTI_CNTRL *)(dti_cntrl_dev_ptr->msg);

  if (dti_cntrl_dev)
  {
    return (dti_cntrl_get_info (info, dti_cntrl_dev));
  }
  return (FALSE);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : DTI_CNTRL                   |
| STATE   : code                        ROUTINE : dti_cntrl_get_next_device   |
+-----------------------------------------------------------------------------+

  PURPOSE :   this function is used in conjunction with AT%DINF where all devices have
              to be listed, give next element
*/
GLOBAL BOOL   dti_cntrl_get_next_device (T_DTI_CNTRL  *info) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev;

  TRACE_FUNCTION("dti_cntrl_get_next_device()");

  if(dti_cntrl_dev_ptr->next NEQ NULL)
  {
     dti_cntrl_dev_ptr = 	dti_cntrl_dev_ptr->next;
     dti_cntrl_dev  = (T_DTI_CNTRL *)(dti_cntrl_dev_ptr->msg);

     if (dti_cntrl_dev)
     {
       return (dti_cntrl_get_info (info, dti_cntrl_dev));
     }
  }
  else
  	dti_cntrl_dev_ptr = NULL;
  return (FALSE);
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                        |
| STATE   : code                   ROUTINE : dti_cntrl_get_first_redirection  |
+-----------------------------------------------------------------------------+

  PURPOSE :   to the given src_id and capability (SER/PKT) find the first
              redirection.
*/
GLOBAL BOOL   dti_cntrl_get_first_redirection (UBYTE         src_id,
                                               UBYTE         capability,
                                               T_DTI_CNTRL  *redirection) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);

  TRACE_FUNCTION("dti_cntrl_get_first_redirection()");

  /*
   * for the cid we set here -1, because the sub function will first increase it by one
   * so we access ser_redirect[0] respectively pkt_redirect[0]
   */
  return (dti_cntrl_get_redirection (dti_cntrl_dev, redirection, 0xFF, capability));
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                       |
| STATE   : code                   ROUTINE : dti_cntrl_get_next_redirection  |
+----------------------------------------------------------------------------+

  PURPOSE :   to the given src_id and capability (SER/PKT) find the next
              redirection >>after<< the given cid.
*/
GLOBAL BOOL   dti_cntrl_get_next_redirection (UBYTE         src_id,
                                              UBYTE         cid,
                                              UBYTE         capability,
                                              T_DTI_CNTRL  *redirection) /* out parameter */
{
  T_DTI_CNTRL  *dti_cntrl_dev  = (T_DTI_CNTRL *)find_element (dti_cntrl_list, src_id, dti_cntrl_search_src_id);

  TRACE_FUNCTION("dti_cntrl_get_next_redirection()");

  return (dti_cntrl_get_redirection (dti_cntrl_dev, redirection, cid, capability));
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                    |
| STATE   : code                   ROUTINE : dti_cntrl_entity_connected   |
+-------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void   dti_cntrl_entity_connected (ULONG             link_id,
                                          T_DTI_ENTITY_ID   entity_id,
                                          T_DTI_CONN_RESULT result)
{
  UBYTE         dti_id        = EXTRACT_DTI_ID(link_id);
  T_DTI_CNTRL  *dti_cntrl_dev = (T_DTI_CNTRL *)find_element (dti_cntrl_list, dti_id, dti_cntrl_search_dti_id);

  TRACE_FUNCTION("dti_cntrl_entity_connected()");

  if (dti_cntrl_dev AND (result EQ DTI_ERROR))
  {
    dti_cntrl_dev->cur_cap = DTI_CPBLTY_NO; /* reset current capability */
  }

  /* call DTI Connection Mng function */
  dti_conn_entity_connected (link_id, entity_id, result);

  /* set current capability if DTI channel is connected */
  if (dti_cntrl_dev AND (dti_conn_is_dti_channel_connected(entity_id, dti_id) EQ TRUE))
  {
    dti_cntrl_dev->cur_cap = g_cur_cap;
  }
}
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL                       |
| STATE   : code                   ROUTINE : dti_cntrl_entity_disconnected   |
+----------------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void   dti_cntrl_entity_disconnected (ULONG             link_id,
                                             T_DTI_ENTITY_ID   entity_id)
{
  UBYTE         dti_id        = EXTRACT_DTI_ID(link_id);
  T_DTI_CNTRL  *dti_cntrl_dev = (T_DTI_CNTRL *)find_element (dti_cntrl_list, dti_id, dti_cntrl_search_dti_id);

  TRACE_FUNCTION("dti_cntrl_entity_disconnected()");

  if (dti_cntrl_dev)
  {
    dti_cntrl_dev->cur_cap = DTI_CPBLTY_NO; /* reset current capability */
  }

  /* call DTI Connection Mng function */
  dti_conn_entity_disconnected( link_id, entity_id );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_erase_entry   |
+--------------------------------------------------------------------+

  PURPOSE : This function removes a registered device from
            DTI Cntrl Mng list and removes its DTI ID from the list of
            DTI Conn Mng.

*/
GLOBAL void dti_cntrl_erase_entry (UBYTE dti_id)
{
  T_DTI_CNTRL  *dti_cntrl_dev = NULL;

  TRACE_FUNCTION("dti_cntrl_erase_entry()");

  dti_cntrl_dev = remove_element (dti_cntrl_list, dti_id, dti_cntrl_search_dti_id);

  if (dti_cntrl_dev EQ NULL)
  {
    TRACE_EVENT_P1("Device with DTI ID %d was not registered", dti_id);
  }
  else
  {
    /*
     * redirect_info is a union, where we use the union element tbl,
     * which is a pointer to an allocated structure
     * of type T_DTI_CNTRL_REDIRECT_INTERN
     */
    if (dti_cntrl_dev->redirect_info.tbl NEQ NULL)
    {
      ACI_MFREE (dti_cntrl_dev->redirect_info.tbl);
      dti_cntrl_dev->redirect_info.tbl = NULL;
    }
    ACI_MFREE (dti_cntrl_dev);
  }

  /* call DTI connection manager function */
  dti_conn_erase_entry(dti_id);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : DTI_CNTRL               |
| STATE   : code                   ROUTINE : dti_cntrl_reg_new_fct   |
+--------------------------------------------------------------------+

  PURPOSE : register an call back function, which is used when a
            customer/external has its own entity involved during
            a data transmission.
            The registered function is called by dti_cntrl_maintain_entity()
            only, where dti_cntrl_maintain_entity() itself is a
            call back function used by the DTI Connection Manager
*/
GLOBAL BOOL  dti_cntrl_reg_new_fct  (T_DTI_ENTITY_ID   entity_id,
                                     T_DTI_EXT_CB     *fct)
{
  UBYTE i = 0;

  TRACE_FUNCTION("dti_cntrl_reg_new_fct()");

  if (entity_id <= DTI_ENTITY_MAX)
  {
    TRACE_EVENT("ENTITY ID already in use");
    return (FALSE);
  }

  for (i=0; i<DTI_MAX_EXT_CB; i++)
  {
    if (reg_ext_cb[i].ent_id EQ DTI_ENTITY_INVALID)
    {
      reg_ext_cb[i].ent_id = entity_id;
      reg_ext_cb[i].fct    = fct;
      return (TRUE);
    }
  }
  return (FALSE);
}

/*
 * special temporary stuff
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : DTI_CNTRL                |
| STATE   : code                  ROUTINE : dti_cntrl_get_link_id()  |
+--------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_DTI_CONN_LINK_ID dti_cntrl_get_link_id( T_DTI_ENTITY_ID ent_id,
                                                 UBYTE           dev_no,
                                                 UBYTE           sub_no)
{
  UBYTE i;

  TRACE_FUNCTION("dti_cntrl_get_link_id()");

  for (i=0; i<MAX_DTI_CONN_LINK_IDS; i++)
  {
    if ( ((dti_aci_data_base[i].ent_id1 EQ ent_id)
           AND (dti_aci_data_base[i].dev_no1 EQ dev_no)
           AND (dti_aci_data_base[i].sub_no1 EQ sub_no))
         OR
         ((dti_aci_data_base[i].ent_id2 EQ ent_id)
           AND (dti_aci_data_base[i].dev_no2 EQ dev_no)
           AND (dti_aci_data_base[i].sub_no2 EQ sub_no)) )
      return (dti_aci_data_base[i].link_id);
  }

  TRACE_EVENT("link_id not found");
  return DTI_LINK_ID_NOTPRESENT;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : DTI_CNTRL                |
| STATE   : code                  ROUTINE : dti_cntrl_get_peer_()    |
+--------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL T_DTI_ENTITY_ID dti_cntrl_get_peer( T_DTI_ENTITY_ID ent_id,
                                           UBYTE           dev_no,
                                           UBYTE           sub_no)
{
  UBYTE i;

  TRACE_FUNCTION("dti_cntrl_get_peer()");

  for (i=0; i<MAX_DTI_CONN_LINK_IDS; i++)
  {
    if  ((dti_aci_data_base[i].ent_id1 EQ ent_id)
          AND (dti_aci_data_base[i].dev_no1 EQ dev_no)
          AND (dti_aci_data_base[i].sub_no1 EQ sub_no))
    {
      return dti_aci_data_base[i].ent_id2;
    }
    if  ((dti_aci_data_base[i].ent_id2 EQ ent_id)
          AND (dti_aci_data_base[i].dev_no2 EQ dev_no)
          AND (dti_aci_data_base[i].sub_no2 EQ sub_no))
    {
      return dti_aci_data_base[i].ent_id1;
    }
  }

  TRACE_EVENT("peer entity not found");
  return DTI_ENTITY_INVALID;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : DTI_CNTRL                  |
| STATE   : code                ROUTINE : dti_cntrl_set_conn_parms() |
+--------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL dti_cntrl_set_conn_parms( T_DTI_CONN_LINK_ID link_id,
                                      T_DTI_ENTITY_ID    ent_id,
                                      UBYTE              dev_no,
                                      UBYTE              sub_no )
{
  UBYTE i;

  TRACE_FUNCTION("dti_cntrl_set_conn_parms()");

  for (i=0; i<MAX_DTI_CONN_LINK_IDS; i++)
  {
    if (dti_aci_data_base[i].link_id EQ DTI_LINK_ID_NOTPRESENT)
    {
      dti_aci_data_base[i].link_id  = link_id;

      dti_aci_data_base[i].ent_id1 = ent_id;
      dti_aci_data_base[i].dev_no1 = dev_no;
      dti_aci_data_base[i].sub_no1 = sub_no;

      return TRUE;
    }
    else if (dti_aci_data_base[i].link_id EQ link_id)
    {
      dti_aci_data_base[i].ent_id2 = ent_id;
      dti_aci_data_base[i].dev_no2 = dev_no;
      dti_aci_data_base[i].sub_no2 = sub_no;

      return TRUE;
    }
  }

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : DTI_CNTRL                  |
| STATE   : code                ROUTINE : dti_cntrl_clear_conn_parms |
+--------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void dti_cntrl_clear_conn_parms( UBYTE dti_id )
{
  UBYTE i;

  TRACE_FUNCTION("dti_cntrl_clear_conn_parms()");

  for (i=0; i<MAX_DTI_CONN_LINK_IDS; i++)
  {
    if (EXTRACT_DTI_ID(dti_aci_data_base[i].link_id) EQ dti_id)
    {
      dti_aci_data_base[i].link_id  = DTI_LINK_ID_NOTPRESENT;
      dti_aci_data_base[i].dev_no1  = DTI_DEV_NO_NOTPRESENT;
      dti_aci_data_base[i].dev_no2  = DTI_DEV_NO_NOTPRESENT;
      dti_aci_data_base[i].ent_id1  = DTI_ENTITY_INVALID;
      dti_aci_data_base[i].ent_id2  = DTI_ENTITY_INVALID;
      dti_aci_data_base[i].sub_no1  = DTI_SUB_NO_NOTPRESENT;
      dti_aci_data_base[i].sub_no2  = DTI_SUB_NO_NOTPRESENT;
    }
  }
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)  MODULE  : DTI_CNTRL                       |
| STATE   : code            ROUTINE : dti_cntrl_close_all_connections |
+---------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void dti_cntrl_close_all_connections()
{
  T_DTI_CNTRL  *dti_cntrl_dev;
  USHORT        dti_cntrl_list_num = 0;
  USHORT        i                  = 0;
  USHORT        k                  = 0;

  TRACE_FUNCTION("dti_cntrl_close_all_connections()");

  dti_cntrl_list_num = get_list_count (dti_cntrl_list);
  /*
   * we clear the list from head to tail
   */
  while (i <= dti_cntrl_list_num)
  {
    dti_cntrl_dev = (T_DTI_CNTRL *)remove_first_element (dti_cntrl_list);

    if (dti_cntrl_dev)
    {
      for (k=0; k<DTI_MAX_REDIRECTIONS; ++k)
      {
        dti_cntrl_dev->redirect_info.tbl->ser_redirect[k].redirection = NULL;
        dti_cntrl_dev->redirect_info.tbl->pkt_redirect[k].redirection = NULL;
      }

      ACI_MFREE (dti_cntrl_dev->redirect_info.tbl);
      ACI_MFREE (dti_cntrl_dev);
    }
    ++i;
  }

  dti_conn_close_all_connections();
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)  MODULE  : DTI_CNTRL                        |
| STATE   : code            ROUTINE : dti_cntrl_set_dti_id_to_reconnect|
+----------------------------------------------------------------------+

  PURPOSE : This function sets a flag that means that if this DTI 
            connection is completely disconnected then reconnect the 
            registered device to ACI.
*/
GLOBAL void dti_cntrl_set_dti_id_to_reconnect(UBYTE dti_id)
{
  ULONG tmp_bit_field = 0x01 << (dti_id);

  TRACE_FUNCTION("dti_cntrl_set_dti_id_to_reconnect()");

  BITFIELD_SET(dti_cntrl_reconnect_to_aci, tmp_bit_field);
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)  MODULE  : DTI_CNTRL                          |
| STATE   : code            ROUTINE : dti_cntrl_clear_dti_id_to_reconnect|
+------------------------------------------------------------------------+

  PURPOSE : Clears the 'Reconnect-to-ACI' flag.

*/
GLOBAL void dti_cntrl_clear_dti_id_to_reconnect(UBYTE dti_id)
{
  ULONG tmp_bit_field = 0x01 << (dti_id);

  TRACE_FUNCTION("dti_cntrl_clear_dti_id_to_reconnect()");

  BITFIELD_CLEAR(dti_cntrl_reconnect_to_aci, tmp_bit_field);
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)  MODULE  : DTI_CNTRL                          |
| STATE   : code            ROUTINE : dti_cntrl_is_dti_id_to_reconnect   |
+------------------------------------------------------------------------+

  PURPOSE : Queries the 'Reconnect-to-ACI' flag.

*/
GLOBAL BOOL dti_cntrl_is_dti_id_to_reconnect(UBYTE dti_id)
{
  ULONG tmp_bit_field = 0x01 << (dti_id);

  TRACE_FUNCTION("dti_cntrl_is_dti_id_to_reconnect()");

  if (BITFIELD_CHECK(dti_cntrl_reconnect_to_aci, tmp_bit_field))
  {
    TRACE_EVENT_P1("dti_id=%d must be reconnected", dti_id);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#endif /* DTI */

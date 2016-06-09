/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This file consists of the BAT Extension Mechanism
|
+----------------------------------------------------------------------------- 
*/ 
/*==== INCLUDES ===================================================*/
#include "aci_all.h"
#include "aci_mem.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"

#include "psa_psi.h"
#include "cmh_psi.h"
#include "ati_src_psi.h"
#include "sap_dti.h"
#include "psa.h"
#include "cmh.h"

#include "aci_bat_cmh.h"
#include "aci_bat.h"
#include "aci_bat_err.h"
#include "aci_bat_cb.h"  /* for rBAT_OK */

#include "ati_ext_mech.h"
#include "aci_bat_ext.h"


#include "l2p_types.h"
#include "l2p.h"
#include "bat_ctrl.h"
#include "aci_cmd.h"

#ifdef _SIMULATION_
#include "gdd_aci.h"
#endif

typedef struct
{
  U8 ati_custom_src_id;  /* ATI source, which handles the custom cmd/rsp */
  U8 bat_custom_src_id;  /* BAT source, which is the peer to ATI source  */
} T_aci_bat_ext_globs;


T_aci_bat_ext_globs aci_bat_ext_globs = {0xFF, 0xFF};


/*
 * The very first cutom AT command has no peer. So find it by broadcasting the command
 * over all BAT sources. A BAT source is associated with a DTI connection between PSI
 * and ACI. A DTI connection is associated with a certain BAT Lib instance. A BAT Lib
 * instance can be created by a third party application suite, by Bluetooth, by TCP/IP
 * its bearer manager BEAM and others in future.
 * After sending the very first custom command over BAT via broadcast, we wait for the
 * very first answer. Then we know our peer and will address it directly with all 
 * following custom commands.
 * The following simplified MSC illustrates this.
 * The BAT["XXX"] shall express that the string "XXX" is content of the BAT custom container.
 * In the MSC at sequence 8 we got an answer to the custom command. Here we know our peer.
 * Sequence 11 is neccessary to make the state machine of BAT LIB happy.

+---+               +---+                +---+                 +---+                  +---+    +---+
|PC |               |ATI|                |BAT|                 |BAT|                  |APP|    |APP|
|APP|               |EXT|                |MOD|                 |LIB|                  | 1 | .. | N |
+---+               +---+                +---+                 +---+                  +---+    +---+
  |                   |                    |                     |                      |        |
  |                   |                    |                     |                      |        |
  | "AT^SOMETHING=1"  |aci_bat_send_custom_cmd                   |                      |        |
1 |------------------>| "AT^SOMETHING=1"   |                     |                      |        |
2 |                   |------------------->| BAT["AT^SOMETH...]  |                      |        |
3 |                   |                    |-------------------->| uns_cb("AT^SOMETH...)|        |
4 |                   |                    | with DTI over PSI   |--------------------->|        |
  |                   |                    |                     |                      |        |
  |                   |                    |  .. broadcast ..    |                      |        |
  |                   |                    |                     |                      |        |
  |                   |                    | BAT["AT^SOMETH...]  |                      |        |
5 |                   |                    |-------------------->|      uns_cb("AT^SOMETH...)    |
6 |                   |                    |                     |------------------------------>|
  |                   |                    |                     |                      |        |
  |                   |                    |                     |   bat_send("OK")     |        |
7 |                   |                    |      BAT["OK"]      |<---------------------|        |
8 |                   | rEXT_Response_BAT  |<--------------------|                      |        |
9 |     "OK"          |<-------------------|                     |                      |        |
10|<------------------|                    |       BAT_OK        |                      |        |
11|                   |                    |-------------------->| rsp_cb(BAT_OK)       |        |
12|                   |                    |                     |--------------------->|        |
  |                   |                    |                     |                      |        |
 */
LOCAL T_ACI_BAT_RSLT aci_bat_find_peer (T_BAT_cmd_send *cmd)
{
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;
  T_ACI_DTI_PRC_PSI  *src_infos_psi = NULL;
  int errVal      = -1;
  U8  client_id   = 0xFC; /* custom channel */
  int source, client;
  BOOL custom_cmd_has_been_send;
  BOOL has_it_been_send_at_all = FALSE;

  TRACE_FUNCTION("aci_bat_find_peer()");

  for (source = 0; source < CMD_SRC_MAX; source++)
  {
    custom_cmd_has_been_send = FALSE;
    if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)source) NEQ CMD_MODE_BAT)
    {
      continue; /* it is not a BAT source */
    }

    src_infos_psi = find_element(psi_src_params, source, cmhPSItest_srcId);

    for (client=0; client < src_infos_psi->max_clients; client++)
    {
      if (custom_cmd_has_been_send EQ TRUE)
      {
        break;
      }   
      if (src_infos_psi->bat_client[client].curCmd EQ (T_BAT_ctrl_params)-1)
      {
        l2p_status = L2P_Send(aci_bat_get_l2p_id(source),
                              client_id,
                              cmd->ctrl_params,
                              cmd->params.ptr_custom, 
                              cmd->params.ptr_custom->c_buf + sizeof(U16), /* sizeof(c_buf) */
                              &errVal);
        switch (l2p_status)
        {
          case (L2P_STAT_SUCCESS):
          {
            custom_cmd_has_been_send = TRUE;
            has_it_been_send_at_all  = TRUE;
            #ifndef _SIMULATION_
            /* make DTI happy with its flow control stuff */
            psa_psi_DTI_getdata ((UBYTE)(src_infos_psi->srcId), DTI_ENTITY_PSI);
            #endif
            break;
          }
          default:
          {
            TRACE_EVENT_P1("aci_bat_find_peer(): l2p_status = %d", l2p_status);
            TRACE_EVENT_P1("aci_bat_find_peer(): errVal     = %d", errVal); 
            return (ACI_BAT_FAIL);
          }
        }
        break;
      }
    }
    if (custom_cmd_has_been_send EQ FALSE)
    {
      /* either all BAT channels associated to this source are busy or L2P did peg out */
      TRACE_EVENT_P1("aci_bat_find_peer(): missed to send custom command for BAT source = %d", source);
    }
  } /* end of for (source = 0 ; source < CMD_SRC_MAX; source++) */
  if (has_it_been_send_at_all EQ FALSE)
  {
    TRACE_ERROR("aci_bat_find_peer(): was not able to send custom command");
    return (ACI_BAT_FAIL);
  }
  return (ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)         MODULE  : ACI_BAT                 |
| STATE   : code                   ROUTINE : aci_bat_send_custom_cmd |
+--------------------------------------------------------------------+

  PURPOSE : called by rEXT_Execute(), see ati_ext_mech.c
*/
GLOBAL T_ACI_BAT_RSLT aci_bat_send_custom_cmd (unsigned char src_id, T_BAT_cmd_send *cmd)
{
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  int errVal    = -1;
  U8 client_id  = 0xFC; /* custom channel */

  TRACE_FUNCTION("aci_bat_send_custom_cmd()");

  if (BITFIELD_CHECK(cmd->ctrl_params, BAT_BIT_13)) /* 0x2xxx = custom command  */
  {
    TRACE_EVENT_P1("aci_bat_send_custom_cmd(): custom cmd 0x%04X over custom channel", cmd->ctrl_params);

    if (aci_bat_ext_globs.ati_custom_src_id EQ 0xFF)
    {
      aci_bat_ext_globs.ati_custom_src_id = src_id; /* remember ATI src_id, from which custom cmd has been sent */
    }
    if (aci_bat_ext_globs.ati_custom_src_id NEQ src_id) /* currently allow only one ATI source to send custom cmds */
    {
      TRACE_EVENT_P2("aci_bat_send_custom_cmd(): ERROR ati_custom_src_id %d != src_id %d", 
                      aci_bat_ext_globs.ati_custom_src_id, src_id);
      return (ACI_BAT_FAIL);
    }
    if (aci_bat_ext_globs.bat_custom_src_id EQ 0xFF)
    {
      ret = aci_bat_find_peer(cmd);
      switch (ret)
      {
        case (ACI_BAT_CMPL):
          return (ACI_BAT_EXCT); /* because response comes asynchronously */
        default:
          break;
      }
    }
    else
    {
      l2p_status = L2P_Send(aci_bat_get_l2p_id(aci_bat_ext_globs.bat_custom_src_id),
                            client_id,
                            cmd->ctrl_params,
                            cmd->params.ptr_custom, 
                            cmd->params.ptr_custom->c_buf + sizeof(U16), /* sizeof(c_buf) */
                            &errVal);
      switch (l2p_status)
      {
        case (L2P_STAT_SUCCESS):
        {
          #ifndef _SIMULATION_
          /* make DTI happy with its flow control stuff */
          psa_psi_DTI_getdata ((UBYTE)(aci_bat_ext_globs.bat_custom_src_id), DTI_ENTITY_PSI);
          #endif
          return (ACI_BAT_EXCT); /* because response comes asynchronously */
        }
        default:
        {
          TRACE_EVENT_P1("aci_bat_send_custom_cmd(): l2p_status = %d", l2p_status);
          TRACE_EVENT_P1("aci_bat_send_custom_cmd(): errVal     = %d", errVal); 
          
        }
      }
    }
  }
  else
  {
    TRACE_EVENT_P1("aci_bat_send_custom_cmd(): ERROR 0x%04X is not a valid custom cmd identifier", 
                    cmd->ctrl_params);
  }
  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_rcv_custom   |
+--------------------------------------------------------------------+

  PURPOSE : called by aci_bat_l2p_msg_rxd_cb(), see aci_bat.c
            The dataPtr points to a content the customer is responsible for.
            It can be a ASCII string or any binary data.
            The dataSize information is already checked by BAT Lib that it
            does not exceed BAT_MAX_CUSTOM_CMD_LEN.
*/
GLOBAL T_ACI_BAT_RSLT aci_bat_rcv_custom (unsigned char src_id,
                                          unsigned char client,
                                          unsigned int dataTag, 
                                          void *dataPtr, 
                                          unsigned short dataSize)
{
  T_L2P_STATUS l2p_status = L2P_STAT_UNKNOWN_ERROR;
  int errVal    = -1;
  U8 client_id  = client; /* send back on the same client channel */
  T_ATI_EXT_RETURN ret = ATI_EXT_FAIL;
  T_BAT_cmd_response rsp;
  T_BAT_res_plus_ext_error ext_err;
  T_BAT_no_parameter dummy;
  T_BAT_custom custom;

  TRACE_FUNCTION("aci_bat_rcv_custom()");

  TRACE_EVENT_P3("aci_bat_rcv_custom(): tag = 0x%X, size = %d, data = %s", dataTag, dataSize, dataPtr);

  rsp.ctrl_response = (T_BAT_ctrl_response)dataTag;
  rsp.response.ptr_custom = &custom;
  memcpy((U8 *)&custom, (U8 *)dataPtr, dataSize);
  
  aci_bat_ext_globs.bat_custom_src_id = src_id;

  ret = rEXT_Response_BAT (aci_bat_ext_globs.ati_custom_src_id, &rsp);

  /*
   * this response to BAT Lib is neccessary to satisfy it's internal state machine
   */
  switch (ret)
  {
    case (ATI_EXT_CMPL):
    {      
      rsp.ctrl_response = BAT_RES_AT_OK;
      rsp.response.ptr_at_ok = &dummy;
      dummy.bat_dummy = 0xFF;
      
      l2p_status = L2P_Send(aci_bat_get_l2p_id(aci_bat_ext_globs.bat_custom_src_id),
                      client_id,
                      rsp.ctrl_response,
                      rsp.response.ptr_at_ok, 
                      sizeof(T_BAT_no_parameter),
                      &errVal);
      break;
    }
    default:
    {
      TRACE_EVENT("aci_bat_rcv_custom(): rEXT_Response_BAT FAILED");

      rsp.ctrl_response = BAT_RES_PLUS_EXT_ERROR;
      rsp.response.ptr_plus_ext_error = &ext_err;
      ext_err.err = BAT_EXT_ERROR_EXT_ERR_UNKNOWN; 

      l2p_status = L2P_Send(aci_bat_get_l2p_id(aci_bat_ext_globs.bat_custom_src_id),
                      client_id,
                      rsp.ctrl_response,
                      rsp.response.ptr_plus_ext_error, 
                      sizeof(T_BAT_plus_ext_error_err),
                      &errVal);
      break;
    }
  }

  switch (l2p_status)
  {
    case (L2P_STAT_SUCCESS):
    {
      #ifndef _SIMULATION_
      /* make DTI happy with its flow control stuff */
      psa_psi_DTI_getdata ((UBYTE)(aci_bat_ext_globs.bat_custom_src_id), DTI_ENTITY_PSI);
      #endif
      break;
    }
    default:
    {
      TRACE_EVENT_P1("aci_bat_rcv_custom(): l2p_status = %d", l2p_status);
      TRACE_EVENT_P1("aci_bat_rcv_custom(): errVal     = %d", errVal); 
      return (ACI_BAT_FAIL);
    }
  }
  return (ACI_BAT_CMPL);
}



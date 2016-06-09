/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI
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
|  Purpose :  This Modul holds the main functions
|             for the AT Command Interpreter
+-----------------------------------------------------------------------------
*/

#ifndef ACI_ACI_C
#define ACI_ACI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#ifdef _SIMULATION_
#include "dti.h"
#include "dti_conn_mng.h"

#ifdef UART
#include "psa_uart.h"
#endif

#ifdef FF_PSI
#include "psa_psi.h"
#endif /*FF_PSI*/
#include "ati_src_tst.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */


#include "psa.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_cc.h"
#include "aci_lst.h"
#include "ati_cmd.h"
#include "ati_ext_mech.h"

#ifdef FF_ATI
#include "aci_io.h"
#endif

#ifdef FAX_AND_DATA
#include "psa_ra.h"
#include "cmh_ra.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef BT_ADAPTER
#include "dti.h"

#include "bti.h"
#include "bti_int.h"
#include "bti_aci.h"
#include "ati_src_bt.h"
#endif


#ifdef FF_ATI_BAT
#include "p_bat.h"
#include "aci_bat.h"
#ifdef _SIMULATION_
#include "line_edit.h"
#include "ati_bat.h"
#endif
#endif


/*===== EXPORT =====================================================*/
/*===== PRIVATE ====================================================*/

/*===== VARIABLES ==================================================*/
T_ACI_CMD_MODE _mode=CMD_MODE_NONE;
UBYTE srcId_cb;
GLOBAL BOOL _g_ati_trc_enabled = FALSE;

/* 0 (CMD_SRC_LCL) are reserved */
GLOBAL USHORT used_sources = 1;

/*===== EXTERNALS ==================================================*/
#ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
EXTERN T_ACI_LIST    *ati_src_list;
#endif

/*===== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_cmd_req    |
+--------------------------------------------------------------------+

  PURPOSE : handle ACI command request primitive. The original comment
	    said "It is only a signal and must not be freed by PFREE",
	    but it appears to have been wrong even before we touched
	    this code.

  FreeCalypso change: this primitive (not signal!) was originally used
  for by TI's _SIMULATION_ environment (the code in ati_src_tst.c,
  removed in the TCS3.2 version we got), but we have repurposed it
  for our AT-over-RVTMUX implementation in ati_src_rvt.c.

*/

GLOBAL void aci_aci_cmd_req (T_ACI_CMD_REQ *cmd)
{
  TRACE_FUNCTION("aci_aci_cmd_req()");

#ifdef FF_ATI
  if ( cmd->cmd_len < MAX_TRC_LEN )
  {
    cmd->cmd_seq[cmd->cmd_len] = '\0';
    TRACE_EVENT_P1 ("%s", cmd->cmd_seq);
  }

#if defined _SIMULATION_ AND defined DTI
  if (cmd->cmd_src >= CMD_SRC_EXT AND cmd->cmd_src <= MAX_TST_SRC)
  {
#ifdef FF_ATI_BAT
    U8 src_id = tst_src_id[cmd->cmd_src - 1];
#endif
    ati_src_tst_proc_cmd (cmd);
#ifdef FF_ATI_BAT
    if (aci_cmd_src_mode_get(src_id) EQ CMD_MODE_BAT) /* simulation with BAT library */
    {
      T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, src_id, search_ati_src_id);
      T_ACI_DTI_PRC_PSI *src_infos_psi = find_element (psi_src_params, src_id, cmhPSItest_srcId);
      while (ledit_ctrl(src_params->src_id,LEDIT_CTRL_MORE_CMDS, NULL) EQ LEDIT_CMPL)
      {
        ati_bat_response_simulation(&(src_infos_psi->res)); /* intermediate response */
      }
      if (src_params->cmd_state NEQ CMD_IDLE)
      {
        if (src_infos_psi->res.response AND (src_params->curAtCmd NEQ AT_CMD_CMGS))
        {
          ati_bat_response_simulation(&(src_infos_psi->res)); /* final response */
          Perform_ati_bat_maint(src_id);
        }
        src_infos_psi->bat_client[src_infos_psi->active_client].curCmd = (T_BAT_ctrl_params)-1;
      }
    }
#endif /* FF_ATI_BAT */
  }
  else
  {
    TRACE_EVENT_P1 ("aci_aci_cmd_req(): [WRN] srcId=%d, but must be 0x01 or 0x02", cmd->cmd_src);
  }
#elif CONFIG_AT_RVTMUX

  ati_src_rvt_proc_cmd (cmd);

#endif	/* old _SIMULATION_ vs FreeCalypso AT-over-RVTMUX */
#endif	/* FF_ATI */

  PFREE (cmd);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_cmd_req    |
+--------------------------------------------------------------------+

  PURPOSE : handle ACI command request primitive. It is only a
            signal and must not be freed by PFREE.

*/
#ifdef BT_ADAPTER
GLOBAL void aci_aci_cmd_req_bt (T_ACI_CMD_REQ_BT *cmd)
{
  TRACE_FUNCTION("aci_aci_cmd_req_bt()");

#ifdef FF_ATI
  if ( cmd->cmd_len <= 80 )
  {
    if( !strchr((char *)cmd->cmd_seq,'%'))
      TRACE_EVENT ((char *)cmd->cmd_seq);
  }

  if (cmd->cmd_src EQ CMD_SRC_EXT)
  {
  ati_src_bt_proc_cmd(cmd);
  }

#endif
  PFREE(cmd);
}
#endif /* BT_ADAPTER */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_abort_req  |
+--------------------------------------------------------------------+

  PURPOSE : handle aci data mode abort request

*/

GLOBAL void aci_aci_abort_req (T_ACI_ABORT_REQ *aci_abort_req)
{
  #ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
  EXTERN void aciAbort (UBYTE srcId);
  #endif

  UBYTE srcId;

  TRACE_FUNCTION ("aci_aci_abort_req()");

  srcId = aci_abort_req->cmd_src;

#ifdef DTI
#ifdef _SIMULATION_
  if (srcId >= CMD_SRC_EXT AND srcId <= MAX_TST_SRC)
  {
    ati_src_tst_abort(srcId);
    return;
  }
#endif /* _SIMULATION_ */
#endif /* DTI */

  #ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
  if (aci_abort_req->cause EQ ABT_ABORT_CMD)
    aciAbort (srcId);
  #endif

#ifdef FAX_AND_DATA
  if (aci_abort_req->cause EQ ABT_ABORT_DATA)
    cmhRA_Escape ();
#endif

#ifdef BT_ADAPTER
  btiaci_at_abort_res(srcId);
#endif /* BT_ADAPTER */

  PFREE (aci_abort_req);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_abort_req  |
+--------------------------------------------------------------------+

  PURPOSE : handle aci (data mode) abort request for BT

*/
#ifdef BT_ADAPTER
GLOBAL void aci_aci_abort_req_bt (T_ACI_ABORT_REQ_BT *aci_abort_req)
{
/*  EXTERN void aciAbort (UBYTE srcId);*/

  UBYTE srcId;

  TRACE_FUNCTION ("aci_aci_abort_req_bt()");

  srcId = aci_abort_req->src_id;

  #ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
  if (aci_abort_req->cause EQ ABT_ABORT_CMD)
    aciAbort (srcId);
  #endif

#ifdef FAX_AND_DATA
  if (aci_abort_req->cause EQ ABT_ABORT_DATA)
    cmhRA_Escape ();
#endif

  btiaci_at_abort_res(get_port_by_srcID(srcId));


  PFREE (aci_abort_req);

}
#endif /* BT_ADAPTER */

#ifdef FF_ATI

GLOBAL UBYTE aci_init (T_ATI_SRC_TYPE src_type)
{
  int   i;
  UBYTE tst_src;

  TRACE_FUNCTION ("aci_init ()");

#ifdef _SIMULATION_
  if (src_type EQ ATI_SRC_TYPE_TST)
  {
    for (i = (CMD_SRC_MAX-1); i >= 0; i--)
    {
      tst_src = (0x01 << i) & used_sources;
      if (!tst_src)
        break;
    }
    if (tst_src)
    {
      return (0);
    }

    used_sources |= (0x01 << i);
    return (UBYTE) (i);
  }
#endif

  for (i = 0; i < CMD_SRC_MAX; i++)
  {
    tst_src = (0x01 << i) & used_sources;
    if (!tst_src)
      break;
  }
  if (tst_src)
  {
    TRACE_EVENT_P1("No more sources available ! used_sources: %04X", used_sources);
    return (0);
  }
  used_sources |= (0x01 << i);

  if (src_type EQ ATI_SRC_TYPE_BLUETOOTH)
  {
    used_sources |= (0x01 << (i + 8));
  }

  TRACE_EVENT_P2("New source %d ! used_sources: %04X", i, used_sources);
  return (i);
}

#endif /* FF_ATI */

GLOBAL void aci_finit (UBYTE src_id)
{
  UBYTE tst_src;

  TRACE_FUNCTION ("aci_finit ()");

  tst_src = 0x01 << src_id;
  if (!(used_sources & tst_src))
  {
    TRACE_EVENT ("[ERR] aci_finit(): invalid source");
    return;
  }
  used_sources = used_sources & ~tst_src;

  /* for BT */
  tst_src = 0x01 << (src_id + 8);
  used_sources = used_sources & ~tst_src;
}


#ifdef BT_ADAPTER
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_cmd_res    |
+--------------------------------------------------------------------+

  PURPOSE : handle aci_cmd_res

*/

GLOBAL void aci_aci_cmd_res (T_ACI_CMD_RES *aci_cmd_res)
{
  TRACE_FUNCTION ("aci_aci_cmd_res()");

  /*
  This is currently just ignored !
  Actually ACI_CMD_RES is an acknowledgement for ACI_CMD_IND. It should
  be used to determine if an ACI_CMD_IND may be sent. After sending one
  ACI_CMD_IND should wait for ACI_CMD_RES before another ACI_CMD_IND may
  be sent. This behaviour shall be implemented later.
  */
  PFREE (aci_cmd_res);

}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_cmd_res    |
+--------------------------------------------------------------------+

  PURPOSE : handle aci_cmd_res_bt

*/

GLOBAL void aci_aci_cmd_res_bt (T_ACI_CMD_RES_BT *aci_cmd_res)
{

  ULONG port_nb;
  T_ATI_BT_OUTPUT *cur_cmd;

  TRACE_FUNCTION ("aci_aci_cmd_res()");

  /*
  This is currently just ignored !
  Actually ACI_CMD_RES is an acknowledgement for ACI_CMD_IND. It should
  be used to determine if an ACI_CMD_IND may be sent. After sending one
  ACI_CMD_IND should wait for ACI_CMD_RES before another ACI_CMD_IND may
  be sent. This behaviour shall be implemented later.
  */

  port_nb = get_port_by_srcID(aci_cmd_res->src_id);
  PFREE (aci_cmd_res);

  if (bti_port_table[port_nb].atqueue EQ NULL)
  {
    bti_port_table[port_nb].at_flow_on = TRUE;
    TRACE_EVENT("BTA queue empty");
    return;
  }

  cur_cmd = bti_port_table[port_nb].atqueue;

  if (cur_cmd->output_type & ATI_BT_CONFIRM_COMMAND)
  {
    TRACE_EVENT("BTA confirm in queue");
    bti_port_table[port_nb].atqueue = cur_cmd->next;

    btiaci_at_cmd_res(port_nb, (char *)(cur_cmd->output));
    MFREE(cur_cmd);

    if (bti_port_table[port_nb].atqueue EQ NULL)
    {
      TRACE_EVENT("BTA queue empty");
      bti_port_table[port_nb].at_flow_on = TRUE;
      return;
    }
    else
    {
      cur_cmd = bti_port_table[port_nb].atqueue;
    }
  }

  if ((cur_cmd->output_type & ATI_BT_NORMAL_COMMAND) ||
      (cur_cmd->output_type & ATI_BT_INDICATION_COMMAND))
  {
    TRACE_EVENT("BTA command in queue");
    bti_port_table[port_nb].at_flow_on = FALSE;

    bti_port_table[port_nb].atqueue = cur_cmd->next;

    TRACE_EVENT_P1("sending %s", cur_cmd->output);
    btiaci_at_cmd_req(port_nb, (char *)(cur_cmd->output));
    MFREE(cur_cmd);

#ifdef _SIMULATION_
    cur_cmd = bti_port_table[port_nb].atqueue;
    while (cur_cmd->next NEQ NULL) /* search end of queue */
    {
      TRACE_EVENT_P1("in queue: %s", cur_cmd->output); /* just for debugging */
      cur_cmd = cur_cmd->next;
    }
#endif

  }
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_init_res   |
+--------------------------------------------------------------------+

  PURPOSE : handle aci_init_res

*/

GLOBAL void aci_aci_init_res (T_ACI_INIT_RES *aci_init_res)
{
  TRACE_FUNCTION ("aci_aci_init_res()");

  PFREE (aci_init_res);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_deinit_req |
+--------------------------------------------------------------------+

  PURPOSE : handle aci_deinit_req

*/

GLOBAL void aci_aci_deinit_req (T_ACI_DEINIT_REQ *aci_deinit_req)
{
  TRACE_FUNCTION ("aci_aci_deinit_req()");

  io_closePort(0);

  btiaci_at_deinit_res();

  PFREE (aci_deinit_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_open_port_req|
+--------------------------------------------------------------------+

  PURPOSE : handle aci_open_port_req

*/

GLOBAL void aci_aci_open_port_req (T_ACI_OPEN_PORT_REQ *aci_open_port_req)
{
  ULONG port;
  T_BTI_ACK result;

  TRACE_FUNCTION ("aci_aci_open_port_req()");

  port = aci_open_port_req->port_nb;

  result = io_openPort(port);

  PFREE (aci_open_port_req);

  btiaci_at_open_port_res(port, result);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_close_port_req|
+--------------------------------------------------------------------+

  PURPOSE : handle aci_close_port_req

*/

GLOBAL void aci_aci_close_port_req (T_ACI_CLOSE_PORT_REQ *aci_close_port_req)
{
  ULONG port;

  TRACE_FUNCTION ("aci_aci_close_port_req()");

  port = aci_close_port_req->port_nb;

  io_closePort(port);

  PFREE (aci_close_port_req);

  btiaci_at_close_port_res(port);
}

#endif /* BT_ADAPTER */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_trc_ind    |
+--------------------------------------------------------------------+

  PURPOSE : This function call is only relevant for the acia_adater. The acia_adapter
  sends for every AT-cmd a psignal to aci  (task switch).

*/
#ifdef FF_MMI_RIV

EXTERN void acia_callback_for_ACI (void *AT_command_ptr);

GLOBAL void aci_aci_riv_cmd_req (T_ACI_RIV_CMD_REQ *cmd_ptr)
{
  TRACE_FUNCTION ("aci_aci_riv_cmd_req()");

acia_callback_for_ACI ((void *)cmd_ptr); 
}
#endif /* FF_MMI_RIV */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_trc_ind    |
+--------------------------------------------------------------------+

  PURPOSE : handle ACI trace indication primitive.

*/

GLOBAL void aci_aci_trc_ind (T_ACI_TRC_IND *trc_ind)
{
  T_ATI_SRC_PARAMS *src_params;

  TRACE_FUNCTION("aci_aci_trc_ind()");

  if (!_g_ati_trc_enabled)
  {
    PFREE (trc_ind);
    return;
  }

  trc_ind->trc_buf[MAX_TRC_LEN-1] = '\0';

#ifdef FF_ATI
  if ( trc_ind->trc_len >= MAX_TRC_LEN )
  {
    UBYTE c;
    c = trc_ind->trc_buf[MAX_TRC_LEN-1];
    trc_ind->trc_buf[MAX_TRC_LEN-1] = '\0';
    TRACE_EVENT_P1 ("%s", trc_ind->trc_buf);
    trc_ind->trc_buf[MAX_TRC_LEN-1] = c;
  }
  else
  {
    TRACE_EVENT_P1 ("%s", trc_ind->trc_buf);
  }
#endif  /* FF_ATI */
#ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
  /* check if the src is valid */
  src_params = find_element (ati_src_list, trc_ind->cmd_src, search_ati_src_id);
#ifdef FF_PSI
  if ((src_params EQ NULL) OR ((src_params->src_type NEQ ATI_SRC_TYPE_UART) AND 
                        (src_params->src_type NEQ ATI_SRC_TYPE_PSI)))
#else
  if ((src_params EQ NULL) OR (src_params->src_type NEQ ATI_SRC_TYPE_UART))
#endif /*FF_PSI*/
  {
    src_params = get_next_element (ati_src_list, NULL);/* first element */
    while (src_params NEQ NULL)
    {
#ifdef FF_PSI
      if (src_params->src_type EQ ATI_SRC_TYPE_UART OR src_params->src_type EQ ATI_SRC_TYPE_PSI)
#else
      if (src_params->src_type EQ ATI_SRC_TYPE_UART)
#endif /*FF_PSI*/
      {
        break;
      }
      src_params = get_next_element (ati_src_list, src_params);
    }
  }

  if (src_params NEQ NULL)
  {
    io_sendIndication(src_params->src_id, (CHAR *)trc_ind->trc_buf, ATI_FORCED_OUTPUT);
  }
#endif  /* LOCOSTO_LITE_2_54_COMPILER_ERROR */
  PFREE (trc_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI                |
| STATE   : code                        ROUTINE : aci_aci_ext_ind    |
+--------------------------------------------------------------------+

  PURPOSE : handle ACI extension signal indication primitive.

*/

GLOBAL void aci_aci_ext_ind (T_ACI_EXT_IND *aci_ext_ind)
{
  TRACE_FUNCTION ("aci_aci_ext_ind()");
  #ifndef LOCOSTO_LITE_2_54_COMPILER_ERROR
  rEXT_Signal (aci_ext_ind);
  #endif
  PFREE (aci_ext_ind);
}



#endif /* ACI_ACI_C */

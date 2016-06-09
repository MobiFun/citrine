/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_UARTP
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
|             primitives sent to the protocol stack adapter by the DTI
|             interface.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef UART

#ifndef PSA_UARTS_C
#define PSA_UARTS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci.h"
#include "aci_lst.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "psa_uart.h"
#include "cmh_uart.h"

#include "aci_mem.h"

#include "aci_io.h"

#include "sap_dti.h"


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

LOCAL USHORT get_desc_size (T_desc2 *data)
{
  USHORT size = 0;
  T_desc2 *next_data;

  next_data = data;
  while (next_data NEQ NULL)
  {
    size += next_data->len;
    next_data = (T_desc2*)next_data->next;
  }

  return (size);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psa_dti_data_req             |
+-------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void psaDTI_data_req (
                               T_desc2 *data,
                               T_ACI_DTI_PRC *src_infos,
                               T_DTI_ENTITY_ID peer_id
                               )
{
  USHORT data_len = 0;

  TRACE_FUNCTION("psaDTI_data_req");

  {
    PALLOC_DESC2(dti_data_ind, DTI2_DATA_IND);

    data_len = get_desc_size (data);

    dti_data_ind->desc_list2.list_len  = data_len;
    dti_data_ind->desc_list2.first     = (ULONG)data;

    dti_data_ind->parameters.p_id               = 0; /*dummy_ubyte; */

    if (BITFIELD_CHECK (src_infos->data_cntr, UART_DTI_FLOW_OFF))
      dti_data_ind->parameters.st_lines.st_flow = DTI_FLOW_OFF;
    else
      dti_data_ind->parameters.st_lines.st_flow = DTI_FLOW_ON;

    if (BITFIELD_CHECK (src_infos->data_cntr, UART_DTI_SB_BIT))
    {
      TRACE_EVENT("SB is ON");
      dti_data_ind->parameters.st_lines.st_line_sb = DTI_SB_ON;
    }
    else
    {
      TRACE_EVENT("SB is OFF");
      dti_data_ind->parameters.st_lines.st_line_sb = DTI_SB_OFF;
    }

    dti_data_ind->parameters.st_lines.st_line_sa = DTI_SA_ON;
    dti_data_ind->parameters.st_lines.st_break_len = DTI_BREAK_OFF;

#ifdef DTI
    dti_send_data
    (
      aci_hDTI,
      src_infos->srcId,
      (UBYTE)peer_id,
      ACI_DTI_DN_CHANNEL,
      dti_data_ind
    );
#endif
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psa_dti_getdata_req          |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaDTI_getdata( UBYTE src_id, T_DTI_ENTITY_ID peer_id)
{
  T_ACI_DTI_PRC *src_infos = NULL;

  TRACE_FUNCTION("psaDTI_getdata");

  src_infos = find_element (uart_src_params, src_id, cmhUARTtest_srcId);
  if (src_infos EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaDTI_getdata: link_id=%d not found", src_id) ;
    return ;
  }

  if( src_infos->RecState NEQ RECEIVING)
  {
    src_infos->RecState = READY_REC;
  }

#ifdef DTI
  {
    dti_start
    (
      aci_hDTI,
      src_id,
      (UBYTE)peer_id,
      ACI_DTI_DN_CHANNEL
    );
  }
#endif /* DTI */
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psaUART_DCDreq               |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_DCDreq( UBYTE c_Id, UBYTE line_state )
{
  T_ACI_DTI_PRC *cmd  = NULL;

  TRACE_FUNCTION("psaUART_DCDreq");

  cmd = find_element (uart_src_params, c_Id, cmhUARTtest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaUART_DCDreq: c_Id=%d not found", c_Id) ;
    return ;
  }

  {
    PALLOC( uart_dcd_req, UART_DCD_REQ);

    uart_dcd_req -> device = cmd->device;
    uart_dcd_req -> dlci   = cmd->dlci;
    if( line_state EQ IO_DCD_ON )
    {
      uart_dcd_req -> line_state = UART_LINE_ON;
    }
    else if ( line_state EQ IO_DCD_OFF )
    {
      uart_dcd_req -> line_state = UART_LINE_OFF;
    }

    PSENDX( UART, uart_dcd_req );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psaUART_ESCAPEreq               |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_ESCAPEreq( UBYTE c_Id, UBYTE detection )
{
  T_ACI_DTI_PRC *cmd  = NULL;

  TRACE_FUNCTION("psaUART_ESCAPEreq");

  cmd = find_element (uart_src_params, c_Id, cmhUARTtest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaUART_ESCAPEreq: c_Id=%d not found", c_Id) ;
    return ;
  }

  { 
    PALLOC( uart_escape_req, UART_ESCAPE_REQ);

    uart_escape_req -> device    = cmd->device;
    uart_escape_req -> dlci      = cmd->dlci;
    uart_escape_req -> detection = detection;

    PSENDX( UART, uart_escape_req );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psaUART_RINGreq              |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_RINGreq( UBYTE c_Id, UBYTE line_state )
{
  T_ACI_DTI_PRC *cmd  = NULL;

  TRACE_FUNCTION("psaUART_RINGreq");

  cmd = find_element (uart_src_params, c_Id, cmhUARTtest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaUART_RINGreq: c_Id=%d not found", c_Id) ;
    return ;
  }

  if (BITFIELD_CHECK (cmd->data_cntr, UART_RING_RUNNING))
  {
    TRACE_EVENT_P2 ("[WRN] psaUART_RINGreq(): no CNF for previous REQ (device: %d; dlci=%d)",
                    cmd->device, cmd->dlci);
    return;
  }

  /*
     store that UART_RING_REQ was send
     this is important so that the primitive queue in the UART entity not
     overloaded if no PC is connected
  */
  BITFIELD_SET (cmd->data_cntr, UART_RING_RUNNING);

  {
    PALLOC( uart_ring_req, UART_RING_REQ);

    uart_ring_req -> device = cmd->device;
    uart_ring_req -> dlci   = cmd->dlci;
    if( line_state EQ IO_RING_ON)
    {
      uart_ring_req -> line_state = UART_LINE_ON;
    }
    else if( line_state EQ IO_RING_OFF )
    {
      uart_ring_req -> line_state = UART_LINE_OFF;
    }

    PSENDX( UART, uart_ring_req );
  }
}

#ifdef DTI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_IPA                      |
|                            ROUTINE : psaUART_SetDTIReq            |
+-------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL void psaUART_SetDTIReq( T_DTI_CONN_LINK_ID link_id, T_DTI_ENTITY_ID conn_peer_Id )
{
  T_ACI_DTI_PRC *cmd   = NULL;
  CHAR *p_ent_name;
  T_DTI_CNTRL    info;

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  extern CHAR gob_tst_buf[];
#endif

  PALLOC( uart_dti_req, UART_DTI_REQ);

  TRACE_FUNCTION("psaUART_SetDTIReq");

  if (dti_cntrl_get_info_from_dti_id( EXTRACT_DTI_ID(link_id), &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", EXTRACT_DTI_ID(link_id));
    return;
  }

  cmd = find_element (uart_src_params, info.src_id, cmhUARTtest_srcId);
  if (cmd EQ NULL)
  {
    TRACE_EVENT_P1("[ERR] psaUART_SetDTIReq: srcId=%d not found", info.src_id) ;
    return;
  }

  if (dti_cntrl_set_conn_parms(link_id, DTI_ENTITY_UART, cmd->device, cmd->dlci) EQ FALSE)
  {
    return;
  }

  uart_dti_req -> dti_conn = UART_CONNECT_DTI;
  uart_dti_req -> device = cmd->device;
  uart_dti_req -> dlci = cmd->dlci;
  uart_dti_req -> direction = DTI_CHANNEL_TO_HIGHER_LAYER;
  uart_dti_req -> link_id = link_id;


  switch( conn_peer_Id )
  {
  case( DTI_ENTITY_ACI ):
    p_ent_name = &ACI_NAME[0];
    break;
  case( DTI_ENTITY_L2R ):
    p_ent_name = &L2R_NAME[0];
    break;
  case( DTI_ENTITY_TRA ):
    p_ent_name = &TRA_NAME[0];
    break;
  case( DTI_ENTITY_T30 ):
    p_ent_name = &T30_NAME[0];
    break;
#ifdef FF_TRACE_OVER_MTST
  case( DTI_ENTITY_MTST):
    p_ent_name = &MTST_NAME[0];
    break;
#endif /* FF_TRACE_OVER_MTST */
#ifdef GPRS
  case( DTI_ENTITY_PPPS ):
    p_ent_name = &PPP_NAME[0];
    break;
#endif  /* GPRS */

  default:
    TRACE_EVENT("unknown conn_peer_Id: UART_DTI_REQ not sent");
    PFREE(uart_dti_req);
    return;
  }

#ifdef GPRS
#ifdef _SIMULATION_
  p_ent_name = (CHAR *) 0xfe1234ef;
#endif  /* _SIMULATION_ */
#endif  /* GPRS */

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "usr:");
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "%x,%x,%s ",cmd->device, link_id, p_ent_name);
#endif

  uart_dti_req->entity_name = (ULONG)p_ent_name;
  PSENDX( UART, uart_dti_req );
}
#endif
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : psaUART_StartMux             |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_StartMux( UBYTE device,
                                    UBYTE mode,
                                    UBYTE frame_type,
                                    USHORT N1,
                                    UBYTE T1,
                                    UBYTE N2,
                                    UBYTE T2,
                                    UBYTE T3 )
{
  PALLOC(uart_mux_start_req, UART_MUX_START_REQ);

  uart_mux_start_req -> device = device;

  uart_mux_start_req -> mode = mode;
  uart_mux_start_req -> frame_type = frame_type;

  uart_mux_start_req -> n1 = N1;
  uart_mux_start_req -> t1 = T1;
  uart_mux_start_req -> n2 = N2;
  uart_mux_start_req -> t2 = T2;
  uart_mux_start_req -> t3 = T3;

  PSENDX( UART, uart_mux_start_req );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                      |
|                            ROUTINE : psaUART_MuxRes             |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_MuxRes( UBYTE device,
                                  UBYTE dlci,
                                  USHORT N1 )
{
  PALLOC(uart_mux_dlc_establish_res, UART_MUX_DLC_ESTABLISH_RES);

  uart_mux_dlc_establish_res -> device = device;
  uart_mux_dlc_establish_res -> dlci   = dlci;
  uart_mux_dlc_establish_res -> n1     = N1;

  PSENDX( UART, uart_mux_dlc_establish_res );
}

/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                          |
|                            ROUTINE : psaUART_SetParameters             |
+------------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psaUART_SetParameters( UBYTE device, T_comPar *comPar )
{
  PALLOC(uart_parameters_req, UART_PARAMETERS_REQ);

  uart_parameters_req->device             = device;

  uart_parameters_req->comPar   = *comPar;

  PSENDX( UART, uart_parameters_req );
}

#endif /* UART */

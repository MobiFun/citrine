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

#ifndef PSA_UARTP_C
#define PSA_UARTP_C
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
#include "ati_io.h"
#include "cmh_uart.h"

#include "aci_mem.h"

#include "ati_src_uart.h"

#include "sap_dti.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/

EXTERN void cmhUART_erase_elem_received_cmd (UBYTE srcId);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
CHAR gob_tst_buf[80];
#endif

/********** UART related primitives *************************/

/*
+-------------------------------------------------------------------+       
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : psa_uart_parameters_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : .

*/

GLOBAL void psa_uart_parameters_cnf( T_UART_PARAMETERS_CNF *uart_parameters_cnf )
{
  TRACE_FUNCTION("psa_uart_parameters_cnf");

  cmhUART_ParmsUpdated( uart_parameters_cnf->device );
  
  PFREE(uart_parameters_cnf);
}


/*
+-------------------------------------------------------------------+       
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : psa_uart_parameters_ind      |
+-------------------------------------------------------------------+

  PURPOSE : .

TODO: set uart_instances parameter for aci here..

*/

GLOBAL void psa_uart_parameters_ind( T_UART_PARAMETERS_IND *uart_parameters_ind )
{
  UBYTE i;
  UBYTE srcId;

  TRACE_FUNCTION("psa_uart_parameters_ind");

  /* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  memset (gob_tst_buf, 0, 80);
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "pi:%d ", uart_parameters_ind->uart_instances);
#endif

  /*FF_ATI*/
  for (i=0;i<uart_parameters_ind->uart_instances;i++)
  {
    srcId = uart_new_source (i, UART_DLCI_NOT_MULTIPLEXED);
    cmhUART_startConnection (srcId, DEVICE_TYPE_URT);
  }

  PFREE(uart_parameters_ind);

#if defined (RMV_01_04_03) AND defined (DTI)
  {
    /* this is the same as AT%DATA=2,"UART",1,,"SER","UART",0, which is issued by RIL */
    BOOL rv = dti_cntrl_set_redirect_from_device ((UBYTE)DTI_MODE_PERM,
                                           DTI_ENTITY_UART,
                                           1,
                                           0,
                                           DTI_ENTITY_UART,
                                           0,
                                           0,
                                           DTI_CPBLTY_SER,
                                           0);

    if (rv EQ TRUE)
    {
      TRACE_EVENT("dti_cntrl_set_redirect_from_device returned TRUE");
    }
    else
    {
      TRACE_EVENT("dti_cntrl_set_redirect_from_device returned FALSE");
    }
  }
#endif
}




/*
+-------------------------------------------------------------------+       
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : psa_uart_dti_cnf         |
+-------------------------------------------------------------------+

  PURPOSE : .

*/
GLOBAL void psa_uart_dti_cnf( T_UART_DTI_CNF *uart_dti_cnf )
{
  T_ACI_DTI_PRC  *src_infos = NULL;
  T_DTI_CONN_LINK_ID link_id;

  TRACE_FUNCTION("psa_uart_dti_cnf");

  src_infos = cmhUART_find_dlci (uart_src_params, 
                                 uart_dti_cnf->device, 
                                 uart_dti_cnf->dlci);

  /* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "utc:%d ", uart_dti_cnf->device);
#endif
  
  if (src_infos == NULL)
  {
    TRACE_EVENT ("psa_uart_dti_cnf: src_infos EQ NULL");
    PFREE(uart_dti_cnf);
    return;
  }
#ifdef DTI
  link_id = dti_cntrl_get_link_id( DTI_ENTITY_UART, uart_dti_cnf->device, uart_dti_cnf->dlci );

  if (uart_dti_cnf->dti_conn EQ UART_DISCONNECT_DTI)
  {
    /* tell DTI MNG that connection is closed */
    dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_UART );
  }
  else if (uart_dti_cnf->dti_conn EQ UART_CONNECT_DTI)
  {
    /* tell DTI MNG that connection is completed */
    dti_cntrl_entity_connected( link_id, DTI_ENTITY_UART, DTI_OK );
  }
#endif
  PFREE(uart_dti_cnf);
}
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                       |
|                            ROUTINE : psa_UART_error_IND             |
+---------------------------------------------------------------------+

  PURPOSE : 

*/
EXTERN void uart_erase_source( UBYTE srcId );

GLOBAL void psa_uart_error_ind( T_UART_ERROR_IND *uart_error_ind )
{
  T_ACI_DTI_PRC *cmd;
  T_DTI_CONN_LINK_ID link_id;
  CHAR buf[40];

  TRACE_FUNCTION("psa_uart_error_ind");

  /* for tracing of establishing of CMD channels for dual port version */
#ifdef RMV_15_04_03
  sprintf(gob_tst_buf+strlen(gob_tst_buf), "uei ");
#endif 
  
  cmd = cmhUART_find_dlci( uart_src_params, 
                            uart_error_ind->device,
                            uart_error_ind->dlci);
#ifdef DTI
  switch( uart_error_ind->error )
  {
  case( UART_ERROR_NO_CHANNEL ):
    sprintf(buf, "Channel number %d cannot be created", cmd->srcId);
    TRACE_EVENT(buf);
    link_id = dti_cntrl_get_link_id( DTI_ENTITY_UART, uart_error_ind->device, uart_error_ind->dlci );

    /* tell DTI manager that establishment has failed */
    dti_cntrl_entity_connected( link_id, DTI_ENTITY_UART, DTI_ERROR);
    break;

    case( UART_ERROR_MUX_ESTABLISH_FAIL ):
    case( UART_ERROR_MUX_NO_RESPONSE ):
    default:
      TRACE_EVENT("UART_ERROR_IND received with cause other than UART_ERROR_NO_CHANNEL");
    break;
  }
#endif
  PFREE(uart_error_ind);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  :PSA_UART                        |
|                            ROUTINE : psa_uart_disable_cnf           |
+---------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void psa_uart_disable_cnf(T_UART_DISABLE_CNF *uart_disable_cnf)
{
  TRACE_FUNCTION("psa_uart_disable_cnf");

  PFREE(uart_disable_cnf);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  :PSA_UART                        |
|                            ROUTINE : psa_uart_ring_cnf              |
+---------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void psa_uart_ring_cnf(T_UART_RING_CNF *uart_ring_cnf)
{
  T_ACI_DTI_PRC  *src_infos = NULL;

  TRACE_FUNCTION("psa_uart_ring_cnf");
  
  src_infos = cmhUART_find_dlci (uart_src_params, 
                                 uart_ring_cnf->device, 
                                 uart_ring_cnf->dlci);
    
  if (src_infos EQ NULL)      
  {
    TRACE_EVENT_P2 ("[ERR] psa_uart_ring_cnf: not found: device=%d; dlci=%d",
                    uart_ring_cnf->device, uart_ring_cnf->dlci);
  }
  else
  {
    BITFIELD_CLEAR (src_infos->data_cntr, UART_RING_RUNNING);
  }
  PFREE(uart_ring_cnf);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  :PSA_UART                        |
|                            ROUTINE : psa_uart_dcd_cnf               |
+---------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void psa_uart_dcd_cnf(T_UART_DCD_CNF *uart_dcd_cnf )
{
  TRACE_FUNCTION("psa_uart_dcd_cnf");

  PFREE(uart_dcd_cnf);
}
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                       |
|                            ROUTINE : psa_uart_escape_cnf            |
+---------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL void psa_uart_escape_cnf(T_UART_ESCAPE_CNF *uart_escape_cnf )
{
  TRACE_FUNCTION("psa_uart_escape_cnf");

  PFREE(uart_escape_cnf);
}


/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                       |
|                            ROUTINE : psa_uart_dti_ind               |
+---------------------------------------------------------------------+

  PURPOSE : 

this primitive is sent by UART to indicate that a dti-channel has been
closed (or opened, theoretically) without previous request from ACI.

*/
GLOBAL void psa_uart_dti_ind( T_UART_DTI_IND 
                                                 *uart_dti_ind )
{
  T_DTI_CONN_LINK_ID link_id = DTI_LINK_ID_NOTPRESENT;
  T_ACI_DTI_PRC  *src_infos = NULL;

  TRACE_FUNCTION("psa_uart_dti_ind");

  src_infos = cmhUART_find_dlci (uart_src_params, 
                                 uart_dti_ind->device, 
                                 uart_dti_ind->dlci);
#ifdef DTI    
  if (src_infos EQ NULL)      
  {
    TRACE_EVENT_P2 ("[ERR] psa_uart_dti_ind: not found: device=%d; dlci=%d",
                    uart_dti_ind->device, uart_dti_ind->dlci);
  }
  else
  {
    link_id = dti_cntrl_get_link_id( DTI_ENTITY_UART, uart_dti_ind->device, uart_dti_ind->dlci );

    if (uart_dti_ind->dti_conn EQ UART_DISCONNECT_DTI)
    {
      /* tell DTI MNG that connection is closed */
      dti_cntrl_entity_disconnected( link_id, DTI_ENTITY_UART );

    }    
    else if (uart_dti_ind->dti_conn EQ UART_CONNECT_DTI)
    {
      dti_cntrl_entity_connected( link_id, DTI_ENTITY_UART, DTI_OK );
    }
  }
#endif
  PFREE(uart_dti_ind);
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                       |
|                            ROUTINE : psa_uart_detected_ind          |
+---------------------------------------------------------------------+

  PURPOSE : 

TODO!!! 

*/

GLOBAL void psa_uart_detected_ind(T_UART_DETECTED_IND *uart_detected_ind )
{
  TRACE_FUNCTION("psa_uart_detected_ind()");

  cmhUART_DetectedESC_DTR ( uart_detected_ind->device,
                            uart_detected_ind->dlci,
                            uart_detected_ind->cause );

  PFREE(uart_detected_ind);
}


/********* DTI related prims ****************************/




/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : trace_buffer                 |
+-------------------------------------------------------------------+

  PURPOSE : 

*/
/*
LOCAL void trace_buffer (UBYTE *buffer, int len)
{
  char trcBuf[80];
  char *writeP;
  int i;

  sprintf (trcBuf, "Buffer Len.: %d", len);
  TRACE_EVENT (trcBuf);
  
  writeP = trcBuf;
  for (i = 0; i < len; i++)
  {
    writeP += sprintf (writeP, "%02X ", buffer[i]);
  }
  *writeP = '\0';
  TRACE_EVENT ("===================================================");
  TRACE_EVENT (trcBuf);
  TRACE_EVENT ("===================================================");
}
*/
/************ MUX related primitives ******************************/

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  :PSA_UART                        |
|                            ROUTINE : psa_uart_mux_start_cnf         |
+---------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void psa_uart_mux_start_cnf(T_UART_MUX_START_CNF  *uart_mux_start_cnf)
{
  TRACE_FUNCTION("psa_uart_mux_start_cnf");

  PFREE(uart_mux_start_cnf);
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                                   |
|                            ROUTINE : psa_uart_mux_dlc_establish_ind             |
+---------------------------------------------------------------------------------+

  PURPOSE : New dlci channel created... Device multiplexed.

*/

GLOBAL void psa_uart_mux_dlc_establish_ind
                         ( T_UART_MUX_DLC_ESTABLISH_IND *uart_mux_dlc_establish_ind )
{
  UBYTE srcId;
  T_ACI_DTI_PRC *cmd;
  BOOL Need_New_Source;

  TRACE_FUNCTION("psa_uart_mux_dlc_establish_ind");

  /* send response to UART */
  psaUART_MuxRes(uart_mux_dlc_establish_ind->device,
                 uart_mux_dlc_establish_ind->dlci,
                 uart_mux_dlc_establish_ind->n1);
  
  /* search if it is the first multiplexed channel to be established for this 
           particular device */
  cmd = cmhUART_find_dlci (uart_src_params, 
                           uart_mux_dlc_establish_ind->device,
                           UART_DLCI_NOT_MULTIPLEXED);

  if (cmd EQ NULL)
    /* means device has not been found: may (possibly ??) happen if some errors_ind
    from UART have lead to erasing all previous sources of the device */
    Need_New_Source = TRUE;
  else if (cmd->dlci NEQ UART_DLCI_NOT_MULTIPLEXED)
    /* then a new source is needed
      (device aready has multiplexed channels) */
    Need_New_Source = TRUE;
  else
    Need_New_Source = FALSE;

#ifdef DTI
  /* create new source */
  if( Need_New_Source )  
  {
    srcId = uart_new_source( uart_mux_dlc_establish_ind->device, 
                             uart_mux_dlc_establish_ind->dlci );
  
    cmd = find_element (uart_src_params, srcId, cmhUARTtest_srcId);
    if (cmd EQ NULL)
    {
      TRACE_EVENT_P1("[ERR] psa_uart_mux_dlc_establish_ind: "
                     "srcId=%d not found", srcId) ;
      return ;
    }    
    /* start new uart dti channel */
    cmhUART_startConnection (srcId, DEVICE_TYPE_MUX);
  }
  else /* use previous one */
  { 
    T_DTI_ENTITY_ID entity_list[] = {DTI_ENTITY_ACI};

    /*lint -e613 (Warning --Possible use of null pointer) */
    srcId = cmd->srcId;
    uart_InitCmdStruct( cmd );
    cmd->dlci = uart_mux_dlc_establish_ind->dlci;
    cmd->LineState = LINE_CONNECTING;
    /*lint +e613 (Warning --Possible use of null pointer) */

    cmhUART_ChangeDeviceType  (srcId, DEVICE_TYPE_MUX);

    dti_cntrl_est_dpath_indirect ( srcId,
                                   entity_list,
                                   1,
                                   SPLIT,
                                   atiUART_dti_cb,
                                   DTI_CPBLTY_CMD,
                                   DTI_CID_NOTPRESENT);

    dti_cntrl_change_sub_no(srcId, uart_mux_dlc_establish_ind->dlci);
  }

#endif

  PFREE(uart_mux_dlc_establish_ind);
}


/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                                   |
|                            ROUTINE : psa_uart_mux_dlc_release_ind               |
+---------------------------------------------------------------------------------+

  PURPOSE : 

*/
GLOBAL void psa_uart_mux_dlc_release_ind( T_UART_MUX_DLC_RELEASE_IND *uart_mux_dlc_release_ind )
{
  T_ACI_DTI_PRC *cmd;
  CHAR buf[40];

  TRACE_FUNCTION("psa_uart_mux_dlc_release_ind");

  cmd = cmhUART_find_dlci (uart_src_params, 
                           uart_mux_dlc_release_ind->device,
                           uart_mux_dlc_release_ind->dlci);

  sprintf(buf,"Channel number %d cannot be created",cmd->srcId);
  TRACE_EVENT(buf);

  uart_erase_source (cmd->srcId);
  cmhUART_erase_elem_received_cmd (cmd->srcId);
  PFREE(uart_mux_dlc_release_ind);
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                                   |
|                            ROUTINE : psa_uart_mux_close_ind                     |
+---------------------------------------------------------------------------------+

  PURPOSE : 

*/
GLOBAL void psa_uart_mux_close_ind( T_UART_MUX_CLOSE_IND *uart_mux_close_ind )
{
  T_ACI_DTI_PRC *cmd;
  UBYTE srcId;

  TRACE_FUNCTION("psa_uart_mux_close_ind");

  TRACE_EVENT_P1("MUX closed on device number: %d", uart_mux_close_ind->device);

  do
  {
    cmd = cmhUART_find_dlci (uart_src_params, 
                             uart_mux_close_ind->device,
                             UART_DLCI_NOT_MULTIPLEXED);

    if( cmd EQ NULL )
      break;
    
    TRACE_EVENT_P1("Mux Channel with src id %d was released", cmd->srcId);

    uart_erase_source( cmd->srcId );
    cmhUART_erase_elem_received_cmd( cmd->srcId );

  } while( TRUE );


  
  /* reconnect device to uart in non-MUX mode */
  srcId = uart_new_source( uart_mux_close_ind->device, UART_DLCI_NOT_MULTIPLEXED );
  
  /* start uart dti channel */
  cmhUART_startConnection (srcId, DEVICE_TYPE_URT);

  PFREE(uart_mux_close_ind);
}

#endif /* UART */

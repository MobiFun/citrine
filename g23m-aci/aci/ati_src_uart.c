/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  ATI_SRC_UARTC
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
|  Purpose :  .
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef UART

#ifndef ATI_SRC_UART_C
#define ATI_SRC_UART_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_lst.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#ifdef UART
#include "psa_uart.h"
#include "cmh_uart.h"
#endif

#include "aci_io.h"

#ifdef UART
#include "ati_src_uart.h"
#endif

#include "aci_mem.h"
#include "aci.h"
#include "psa.h"
#include "cmh.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

EXTERN T_ACI_LIST *uart_src_params;
#ifdef _SIMULATION_
EXTERN T_ACI_LIST *ati_src_list;
#endif /* _SIMULATION_ */

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIR                |
|                                 ROUTINE : uart_new_source          |
+-------------------------------------------------------------------+

  PURPOSE : Create a new source for ATI with the corresponding struct.
*/

#ifdef UART
GLOBAL void uart_InitCmdStruct( T_ACI_DTI_PRC *cmd_struct )
{
  cmd_struct->device            = 0;
  cmd_struct->dlci              = NOT_PRESENT_8BIT;
  cmd_struct->RecState          = NOT_INITIALIZED;
  cmd_struct->LineState         = LINE_CONNECTING;
  cmd_struct->run_cmd           = FALSE;
  cmd_struct->first_output      = FALSE;
  cmd_struct->data_cntr         = 0;
  cmd_struct->MuxParms          = NULL;
  cmd_struct->large_type        = FALSE;
  cmd_struct->data_buffer.data_list           = NULL;
  cmd_struct->data_buffer.max_buffer_size     = 0;
  cmd_struct->data_buffer.create_time                     = 0;
  cmd_struct->device_type       = DEVICE_TYPE_UNKNOWN;
}
#endif

GLOBAL UBYTE uart_new_source( UBYTE device, UBYTE dlci )
{
  T_ACI_DTI_PRC *cmd_struct;
  UBYTE         src_id;

  TRACE_FUNCTION("uart_new_source");
 
  src_id = ati_init(ATI_SRC_TYPE_UART, uart_src_result_cb, uart_src_line_state_cb);

  TRACE_EVENT_P1 ("uart_new_source: srcId=%d", src_id);

  ACI_MALLOC (cmd_struct, sizeof(T_ACI_DTI_PRC));

#ifdef UART
  uart_InitCmdStruct( cmd_struct );
#endif

  cmd_struct->srcId = src_id;
  cmd_struct->device = device;
  cmd_struct->dlci = dlci;

  /*
  *   Determine the mode of the command source based on the
  *   source ID.
  */
  if (src_id<CMD_SRC_MAX)
  {
    if (src_id==CMD_SRC_LCL)
    {
      aci_cmd_src_mode_set(src_id,CMD_MODE_ACI);
    }
    else
    {
#ifdef FF_ATI_BAT
      aci_cmd_src_mode_set(src_id,CMD_MODE_BAT);
#else
      aci_cmd_src_mode_set(src_id,CMD_MODE_ATI);
#endif
    }
  }
   
  insert_list (uart_src_params, cmd_struct);
  cmhUART_AddDeviceToComParameter( device );

  return(src_id);
}

#ifdef _SIMULATION_
GLOBAL void uart_new_source_for_aci_src_tst( UBYTE src_id, UBYTE device, UBYTE dlci )
{
  T_ACI_DTI_PRC *cmd_struct;

  TRACE_FUNCTION("uart_new_source_for_aci_src_tst");
 
  ACI_MALLOC (cmd_struct, sizeof(T_ACI_DTI_PRC));

#ifdef UART
  uart_InitCmdStruct( cmd_struct );
#endif

  cmd_struct->srcId     = src_id;
  cmd_struct->device    = device;
  cmd_struct->dlci      = dlci;

  /*
  *   Determine the mode of the command source based on the
  *   source ID.
  */
  if (src_id<CMD_SRC_MAX)
  {
    if (src_id==CMD_SRC_LCL)
    {
      aci_cmd_src_mode_set(src_id,CMD_MODE_ACI);
    }
    else
    {
#ifdef FF_ATI_BAT
      aci_cmd_src_mode_set(src_id,CMD_MODE_BAT);
#else
      aci_cmd_src_mode_set(src_id,CMD_MODE_ATI);
#endif
    }
  }
  
  insert_list (uart_src_params, cmd_struct);
  cmhUART_AddDeviceToComParameter( device );
}
#endif
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIR                |
|                                 ROUTINE : uart_erase_source       |
+-------------------------------------------------------------------+

  PURPOSE : erase an entry element from the uart_src_params list.
*/
#ifdef DTI 
EXTERN void dti_cntrl_erase_entry(UBYTE dti_id); 
#endif

GLOBAL void uart_erase_source( UBYTE srcId )
{
#ifdef DTI 
  T_DTI_CNTRL info;
#endif

  TRACE_FUNCTION("uart_erase_source");
 
  ati_finit (srcId);
#ifdef DTI 
  dti_cntrl_get_info_from_src_id (srcId, &info);

  dti_cntrl_erase_entry( info.dti_id );

  dti_cntrl_clear_conn_parms( info.dti_id );
#endif
  TRACE_EVENT_P1("Source nb %d deleted", srcId);

}
#ifdef DTI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_UART                     |
|                            ROUTINE : atiUART_dti_callback         |
+-------------------------------------------------------------------+

  PURPOSE : callback for dti manager.

*/
GLOBAL BOOL atiUART_dti_cb( UBYTE dti_id, T_DTI_CONN_STATE result_type )
{
#ifdef _SIMULATION_
  T_ATI_SRC_PARAMS *src_params;
#endif /* _SIMULATION_ */

  T_ACI_DTI_PRC *srcInfos = NULL;
  UBYTE saveState;
  BOOL  is_uart_src = TRUE;      /* for simulation */
  T_DTI_CNTRL    info;
  
  TRACE_FUNCTION("atiUART_dti_cb");


  if (dti_cntrl_get_info_from_dti_id( dti_id, &info) EQ FALSE)
  {
    TRACE_EVENT_P1("cannot find info for dti_id=%d", dti_id);
    return FALSE;
  }

  srcInfos = find_element (uart_src_params, info.src_id, cmhUARTtest_srcId);
  if (srcInfos EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] atiUART_dti_cb: srcId=%d not found", info.src_id) ;
    return FALSE ;
  }

  saveState = srcInfos->LineState;      

#ifdef _SIMULATION_
  src_params = find_element (ati_src_list, info.src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT ("[ERR] SIMULATION: atiUART_dti_cb: source ID not found");
    return FALSE;
  }
  if (src_params->src_type EQ ATI_SRC_TYPE_TST)
  {
    is_uart_src = FALSE;
  }
#endif /* _SIMULATION_ */


  switch(result_type)
  {
    /* UART and ACI connected */
    /* UART and ACI connected */
    case(DTI_CONN_STATE_CONNECTED):
#ifdef RMV_01_04_03
#ifdef FF_TWO_UART_PORTS
      if (srcInfos->srcId EQ UART_DATA_CHANNEL)
      {
        srcInfos->LineState = LINE_CONNECTED;
        return (TRUE);
      }
#endif /* FF_MULTI_PORT */
#endif

      /* first set the line state, then care for DCD */
      srcInfos->LineState = LINE_CONNECTED;

      ati_switch_mode(info.src_id, ATI_CMD_MODE);

      if (saveState EQ LINE_CONNECTING)
      {

        if (is_uart_src)
        {
          /* creation of a new connection: set DCD */
          io_setDCD ((T_ACI_CMD_SRC)info.src_id, IO_DCD_OFF);
        }

#ifndef _SIMULATION_
        /* AV2-Plattform: Inform the user about the cause for the last stack crash.
            not (yet) enabled since this could cause problems with other customers.
            setatPercentDAR("", srcInfos->srcId);
        */
#ifdef FF_BAT
        if (aci_cmd_src_mode_get((T_ACI_CMD_SRC)info.src_id) NEQ CMD_MODE_BAT)
#endif
        {
          io_sendMessage (srcInfos->srcId, "AT-Command Interpreter ready", 
                          ATI_NORMAL_OUTPUT);
        }
#endif /* _SIMULATION_ */
      }

      if (is_uart_src)
      {
        /* tells UART that ACI is ready to get some data */
        cmhUART_getdata ((T_ACI_CMD_SRC)srcInfos->srcId);
      }
      break;
    /* UART and data entity temporary disconnected */ 
    case(DTI_CONN_STATE_CONNECTING):
      break;
      
    case(DTI_CONN_STATE_DISCONNECTING):
      srcInfos->LineState = LINE_TMP_DISCONNECTING;
      ati_switch_mode(info.src_id, ATI_DATA_MODE);
      break;
      
    case(DTI_CONN_STATE_DISCONNECTED):
      srcInfos->LineState = LINE_TMP_DISCONNECTED;
#ifdef DTI
      dti_cntrl_clear_conn_parms( dti_id );
#endif /* DTI */
      break;

    case(DTI_CONN_STATE_ERROR):
#ifdef DTI
      dti_cntrl_close_dpath_from_dti_id( dti_id );
#endif /* DTI */
      break;
    default:
      break;
  }

  /* Maybe drop call if DTR is asserted */
  if (uartShrdPrm.dtr_clearcall EQ TRUE)
  {
    T_ACI_RETURN l_ret;

    l_ret = sAT_H((T_ACI_CMD_SRC)info.src_id);
    if (l_ret NEQ AT_EXCT AND l_ret NEQ AT_CMPL)
    {
      uartShrdPrm.dtr_clearcall = FALSE;
      TRACE_ERROR("DTR-Drop sAT_H() error");
    }
  }

  return TRUE;  
}
#endif /* DTI */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DTIR                |
|                                 ROUTINE : uart_src_proc_chars     |
+-------------------------------------------------------------------+

  PURPOSE : Check if characters received are ok, and pack them into the command line.
            return TRUE if ACI allows more data from DTI.
*/

GLOBAL BOOL uart_src_proc_chars ( UBYTE *chars, 
                                  USHORT len, 
                                  T_ACI_DTI_PRC *elem )
{
  if (len >= MAX_CMD_LEN)
  {
    TRACE_ERROR ("[ERR] [ATI]: command line too long");
    return (TRUE);
  }
  if (elem->run_cmd)
  {
    ati_abort (elem->srcId);
  }
  else
  {
    ati_execute (elem->srcId, chars, len);
  }
  return (TRUE);
}

#endif /* UART */

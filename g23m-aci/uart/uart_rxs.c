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
|  Purpose :  This modul is part of the entity UART and implements all 
|             functions to handles the incoming process internal signals as  
|             described in the SDL-documentation (RX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RXS_C
#define UART_RXS_C
#endif /* !UART_RXS_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

#ifndef FF_MULTI_PORT
/*==== INCLUDES =============================================================*/

#ifdef WIN32
#include "nucleus.h"
#endif /* WIN32 */
#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "macdef.h"     /* to get a lot of macros */
#include "custom.h"
#include "gsm.h"        /* to get a lot of macros */
#include "cnf_uart.h"   /* to get cnf-definitions */
#include "mon_uart.h"   /* to get mon-definitions */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#ifdef DTILIB
#include "dti.h"        /* to get dti lib */
#endif /* DTILIB */
#include "pei.h"        /* to get PEI interface */
#ifdef _TARGET_
#include "../../serial/serialswitch.h"
#include "../../serial/traceswitch.h"
#else /* _TARGET_ */
#include "serial_dat.h" /* to get definitions of serial driver */
#endif /* _TARGET_ */
#include "uart.h"       /* to get the global entity definitions */

#include "uart_rxf.h"   /* to get rx functions */

#include "uart_kers.h"   /* to get ker signals */
#include "uart_dtxs.h"   /* to get dtx signals */

#ifdef _SIMULATION_
#include "uart_rxp.h"   /* to get rx_readdata */
#endif /* _SIMULATION_ */
/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_dead_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RX_DEAD_MODE_REQ. If this
|               signal is called the service expectes an disabled UART to work
|               correctly.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_dead_mode_req () 
{
  TRACE_ISIG( "sig_ker_rx_dead_mode_req" );

  uart_data->rx.read_permission = FALSE;
  uart_data->rx.prev_lines      = 0;
  uart_data->rx.dlc_instance    = UART_EMPTY_INSTANCE;
  uart_data->rx.escape          = FALSE;
  uart_data->rx.receive_state   = UART_RX_NOT_RECEIVING;
  uart_data->rx.analyze_state   = UART_RX_ERROR;
  uart_data->rx.fcs             = UART_INITFCS;
  uart_data->rx.address_field   = 0;
  uart_data->rx.stored_len      = 0;

  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_READY:
    case RX_MUX:
      SET_STATE( UART_SERVICE_RX, RX_DEAD );
      break;

    case RX_DEAD:
      break;
    default:
      TRACE_ERROR( "SIG_KER_RX_DEAD_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_dead_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_ready_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RX_READY_MODE_REQ.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_ready_mode_req () 
{
  TRACE_ISIG( "sig_ker_rx_ready_mode_req" );

  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_DEAD:
      SET_STATE( UART_SERVICE_RX, RX_READY );
      uart_data->rx.read_permission = FALSE;
      uart_data->rx.dlc_instance    = UART_EMPTY_INSTANCE;
      uart_data->rx.receive_state   = UART_RX_NOT_RECEIVING;
      uart_data->rx.analyze_state   = UART_RX_ERROR;
      break;

    case RX_MUX:
      SET_STATE( UART_SERVICE_RX, RX_READY );
      if(uart_data->rx.read_permission EQ FALSE)
      {
        uart_data->rx.dlc_instance  = UART_EMPTY_INSTANCE;
        uart_data->rx.analyze_state = UART_RX_ERROR;
      }
      break;

    case RX_READY:
      break;

    default:
      TRACE_ERROR( "SIG_KER_RX_READY_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_ready_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_mux_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RX_MUX_MODE_REQ.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_mux_mode_req () 
{
  TRACE_ISIG( "sig_ker_rx_mux_mode_req" );

  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_DEAD:
      SET_STATE( UART_SERVICE_RX, RX_MUX );
      uart_data->rx.read_permission = FALSE;
      uart_data->rx.dlc_instance    = UART_CONTROL_INSTANCE;
      uart_data->rx.receive_state   = UART_RX_NOT_RECEIVING;
      uart_data->rx.analyze_state   = UART_RX_ERROR;
      break;

    case RX_READY:
      SET_STATE( UART_SERVICE_RX, RX_MUX );
      if(uart_data->rx.read_permission EQ FALSE)
      {
        uart_data->rx.dlc_instance  = UART_CONTROL_INSTANCE;
        uart_data->rx.analyze_state = UART_RX_ERROR;
      }
      break;

    case RX_MUX:
      break;

    default:
      TRACE_ERROR( "SIG_KER_RX_MUX_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_mux_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_rx_ready_to_receive_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_RX_READY_TO_RECEIVE_REQ.
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DTX
|               receive_data - descriptor to write
|               receive_pos  - position to start write
|               receive_size - size of descriptor to write
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_rx_ready_to_receive_req (UBYTE   dlc_instance,
                                             T_desc2* receive_data, 
                                             USHORT  receive_pos,
                                             USHORT  receive_size) 
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_dtx_rx_ready_to_receive_req" );

  dlc = &uart_data->dlc_table[dlc_instance];
  dlc->receive_pos  = receive_pos;
  dlc->receive_size = receive_size;
  dlc->receive_data = receive_data;

  /*
   * start receiving
   */
  if(uart_data->rx.receive_state EQ UART_RX_RECEIVING)
  {
    if(uart_data->rx.read_permission EQ FALSE)
      dlc->receive_process = UART_RX_PROCESS_READY;
  }
  else
  {
    dlc->receive_process = UART_RX_PROCESS_READY;
    uart_data->rx.receive_state = UART_RX_RECEIVING;
    switch( GET_STATE( UART_SERVICE_RX ) )
    {
      case RX_READY:
        uart_data->rx.dlc_instance  = UART_EMPTY_INSTANCE;
        uart_data->rx.analyze_state = UART_RX_ERROR;
        break;

      case RX_MUX:
        if(uart_data->rx.dlc_instance EQ UART_EMPTY_INSTANCE)
        {
          uart_data->rx.dlc_instance  = UART_CONTROL_INSTANCE;
          uart_data->rx.analyze_state = UART_RX_ERROR;
        }
        break;

      default:
        TRACE_ERROR( "SIG_DTX_RX_READY_TO_RECEIVE_REQ unexpected" );
        break;
    }
#ifdef _SIMULATION_
    if(rx_inpavail(uart_data->device) > 0)
#else /* _SIMULATION_ */
    if(UF_InpAvail (uart_data->device) > 0)
#endif /* _SIMULATION_ */
    {
      /*
       * inform channel about reading
       * because previous receive_state was NOT_READING
       * there is only one channel which must be informed
       */
      uart_data->rx.read_permission = TRUE;
      uart_data->dtx                = dlc->dtx;
      sig_rx_dtx_receiving_ind();
    }
    else
      uart_data->rx.read_permission = FALSE;

    if(uart_data EQ (&(uart_data_base[0])))
    {
      TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
      rx_readdata(0);
#else /* _SIMULATION_ */
      UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_0);
#endif /* else _SIMULATION_ */
    }
#ifdef FF_TWO_UART_PORTS
    else if(uart_data EQ (&(uart_data_base[1])))
    {
      TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
      rx_readdata(1);
#else /* _SIMULATION_ */
      UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_1);
#endif /* else _SIMULATION_ */
    }
#endif /* FF_TWO_UART_PORTS */
    else
    {
      TRACE_ERROR("wrong value of uart_data");
    }
  }
} /* sig_dtx_rx_ready_to_receive_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_ready_to_receive_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RX_READY_TO_RECEIVE_REQ.
|
| Parameters  : receive_data - descriptor to write
|               receive_pos  - position to start write
|               receive_size - size of descriptor to write
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_ready_to_receive_req (T_desc2* receive_data, 
                                             USHORT  receive_pos,
                                             USHORT  receive_size) 
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_ker_rx_ready_to_receive_req" );

  dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_MUX:
      /*
       * store data in control instance
       */
      dlc->receive_pos  = receive_pos;
      dlc->receive_size = receive_size;
      dlc->receive_data = receive_data;
      /*
       * start receiving
       */
      if(uart_data->rx.receive_state EQ UART_RX_RECEIVING)
      {
        if(uart_data->rx.read_permission EQ FALSE)
          dlc->receive_process = UART_RX_PROCESS_READY;
      }
      else
      {
        dlc->receive_process = UART_RX_PROCESS_READY;
        /*
         * start receiving
         */
        uart_data->rx.receive_state = UART_RX_RECEIVING;
        if(uart_data->rx.dlc_instance EQ UART_EMPTY_INSTANCE)
        {
          uart_data->rx.dlc_instance  = UART_CONTROL_INSTANCE;
          uart_data->rx.analyze_state = UART_RX_ERROR;
        }
#ifdef _SIMULATION_
        if(rx_inpavail(uart_data->device) > 0)
#else /* _SIMULATION_ */
        if(UF_InpAvail (uart_data->device) > 0)
#endif /* _SIMULATION_ */
        {
          /*
           * inform channel about reading
           */
          uart_data->rx.read_permission = TRUE;
          sig_rx_ker_receiving_ind();
        }
        else
          uart_data->rx.read_permission = FALSE;

        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
          rx_readdata(0);
#else /* _SIMULATION_ */
          UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
          rx_readdata(1);
#else /* _SIMULATION_ */
          UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_1);
#endif /* else _SIMULATION_ */
        }
#endif /* FF_TWO_UART_PORTS */
        else
        {
          TRACE_ERROR("wrong value of uart_data");
        }
      }
      break;

    default:
      TRACE_ERROR( "SIG_KER_RX_READY_TO_RECEIVE_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_ready_to_receive_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_rx_not_ready_to_receive_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal 
|               SIG_DTX_RX_NOT_READY_TO_RECEIVE_REQ.
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DTX
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_rx_not_ready_to_receive_req (UBYTE dlc_instance)
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_dtx_rx_not_ready_to_receive_req" );

  dlc = &uart_data->dlc_table[dlc_instance];
  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_READY:
    case RX_MUX:
      dlc->receive_data    = NULL;
      dlc->receive_process = UART_RX_PROCESS_STOP;
      break;

    default:
      TRACE_ERROR( "SIG_DTX_RX_NOT_READY_TO_RECEIVE_REQ unexpected" );
      break;
  }
} /* sig_dtx_rx_not_ready_to_receive_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_not_ready_to_receive_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal 
|               SIG_KER_RX_NOT_READY_TO_RECEIVE_REQ.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_not_ready_to_receive_req () 
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_ker_rx_not_ready_to_receive_req" );

  dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_MUX:
      dlc->receive_data    = NULL;
      dlc->receive_process = UART_RX_PROCESS_STOP;
      break;

    default:
      TRACE_ERROR( "SIG_KER_RX_NOT_READY_TO_RECEIVE_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_not_ready_to_receive_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rx_restart_read_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RX_RESTART_READ_REQ
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rx_restart_read_req () 
{
  TRACE_ISIG( "sig_ker_rx_restart_read_req" );

  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_READY:
    case RX_MUX:
      if(uart_data->rx.receive_state EQ UART_RX_RECEIVING)
      {
        /*
         * restart readOutFunc
         */
        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
          rx_readdata(0);
#else /* _SIMULATION_ */
          UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_ReadData()");
#ifdef _SIMULATION_
          rx_readdata(1);
#else /* _SIMULATION_ */
          UF_ReadData (uart_data->device, sm_suspend, rx_readOutFunc_1);
#endif /* else _SIMULATION_ */
        }
#endif /* FF_TWO_UART_PORTS */
        else
        {
          TRACE_ERROR("wrong value of uart_data");
        }
      }
      break;

    default:
      TRACE_ERROR( "SIG_KER_RX_RESTART_READ_REQ unexpected" );
      break;
  }
} /* sig_ker_rx_restart_read_req() */
#endif /* !FF_MULTI_PORT */

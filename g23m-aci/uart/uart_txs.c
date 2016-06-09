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
|             described in the SDL-documentation (TX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_TXS_C
#define UART_TXS_C
#endif /* !UART_TXS_C */

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

#include "uart_txf.h"   /* to get tx functions */

#include "uart_kers.h"   /* to get ker signals */
#include "uart_drxs.h"   /* to get drx signals */

#ifdef _SIMULATION_
#include <stdio.h>      /* to get sprintf */
#include "uart_txp.h"   /* to get tx_writedata */
#endif /* _SIMULATION_ */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_dead_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_DEAD_MODE_REQ. If this
|               signal is called the service expectes an disabled UART to work
|               correctly.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_dead_mode_req ()
{
  TRACE_ISIG( "sig_ker_tx_dead_mode_req" );

  uart_data->tx.lines        = 0x80000000; /* invalid */
  uart_data->tx.dlc_instance = UART_EMPTY_INSTANCE;
  uart_data->tx.p_zero       = 0;
  uart_data->tx.send_state   = UART_TX_NOT_SENDING;

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_READY:
    case TX_MUX:
      SET_STATE( UART_SERVICE_TX, TX_DEAD );
      break;

    case TX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_TX_DEAD_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_dead_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_ready_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_READY_MODE_REQ.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_ready_mode_req ()
{
  TRACE_ISIG( "sig_ker_tx_ready_mode_req" );


  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_DEAD:
      SET_STATE( UART_SERVICE_TX, TX_READY );
      uart_data->tx.dlc_instance = UART_EMPTY_INSTANCE;
      uart_data->tx.send_state   = UART_TX_NOT_SENDING;
      break;

    case TX_MUX:
      SET_STATE( UART_SERVICE_TX, TX_READY );
      if(uart_data->tx.send_state EQ UART_TX_NOT_SENDING)
        uart_data->tx.dlc_instance = UART_EMPTY_INSTANCE;
      break;

    case TX_READY:
      break;

    default:
      TRACE_ERROR( "SIG_KER_TX_READY_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_ready_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_mux_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_MUX_MODE_REQ
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_mux_mode_req ()
{
  TRACE_ISIG( "sig_ker_tx_mux_mode_req" );

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_DEAD:
      SET_STATE( UART_SERVICE_TX, TX_MUX );
      uart_data->tx.send_state = UART_TX_NOT_SENDING;
      break;

    case TX_READY:
      SET_STATE( UART_SERVICE_TX, TX_MUX );
      break;

    case TX_MUX:
      break;

    default:
      TRACE_ERROR( "SIG_KER_TX_MUX_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_mux_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_drx_tx_data_available_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DRX_TX_DATA_AVAILABLE_IND
|
| Parameters  : dlc_instance  - dlc instance wich belongs to calling DRX
|               transmit_data - descriptors to transmit
|               transmit_pos  - position to start tranmission in first desc
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_drx_tx_data_available_ind (UBYTE   dlc_instance,
                                           T_desc2* transmit_data,
                                           USHORT  transmit_pos)
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_drx_tx_data_available_ind" );

  dlc = &uart_data->dlc_table[dlc_instance];
  dlc->transmit_data = transmit_data;
  dlc->transmit_pos  = transmit_pos;
  dlc->p_counter     = dlc->priority + uart_data->tx.p_zero;

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_READY:
      if(uart_data->tx.send_state EQ UART_TX_NOT_SENDING)
      {
        uart_data->tx.dlc_instance = UART_EMPTY_INSTANCE;
        /*
         * inform dlc about sending
         */
        if(dlc->transmit_data EQ NULL)
        {
          /*
           * no more data
           */
          break;
        }
        uart_data->tx.send_state = UART_TX_SENDING;
        uart_data->drx           = dlc->drx;
        sig_tx_drx_sending_req();
        /*
         * transmit data
         */
        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(0);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(1);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_1);
#endif /* else _SIMULATION_ */
        }
#endif /* FF_TWO_UART_PORTS */
        else
        {
          TRACE_ERROR("wrong value of uart_data");
        }
      }
      break;

    case TX_MUX:
      if(uart_data->tx.send_state EQ UART_TX_NOT_SENDING)
      {
        /*
         * determine next dlc allow to send
         */
        tx_next_send_allowed();
        /*
         * inform dlc about sending
         */
        if(uart_data->tx.dlc_instance EQ UART_EMPTY_INSTANCE)
        {
          /*
           * queue empty
           */
          break;
        }
        dlc = &uart_data->dlc_table[uart_data->tx.dlc_instance];
        uart_data->tx.send_state = UART_TX_SENDING;
        if(uart_data->tx.dlc_instance EQ UART_CONTROL_INSTANCE)
        {
          /*
           * Control channel
           */
          sig_tx_ker_sending_ind();
        }
        else
        {
          /*
           * Data channel
           */
          uart_data->drx = dlc->drx;
          sig_tx_drx_sending_req();
        }
        /*
         * transmit data
         */
#ifndef _SIMULATION_
        tx_flushUart();
#endif /* !_SIMULATION_ */
        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(0);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(1);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_1);
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
      TRACE_ERROR( "SIG_DRX_TX_DATA_AVAILABLE_IND unexpected" );
      break;
  }
} /* sig_drx_tx_data_available_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_data_available_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_DATA_AVAILABLE_REQ
|
| Parameters  : transmit_data - descriptors to transmit
|               transmit_pos  - position to start tranmission in first desc
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_data_available_req (T_desc2* transmit_data,
                                           USHORT  transmit_pos)
{
  T_DLC*  dlc;

  TRACE_ISIG( "sig_ker_tx_data_available_req" );

  dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  dlc->transmit_data = transmit_data;
  dlc->transmit_pos  = transmit_pos;
  dlc->p_counter     = dlc->priority + uart_data->tx.p_zero;

#ifdef _SIMULATION_
  if(transmit_data->len)
  {
    USHORT  i;
    USHORT  pos;
    char    buf[90];
    /*
     * trace output
     */
    TRACE_EVENT("====== OUT");
    i   = 0;
    pos = 0;
    while(pos < transmit_data->len)
    {
      i+= sprintf(&buf[i], "0x%02x, ", transmit_data->buffer[pos]);
      pos++;
      if(i > 80)
      {
        TRACE_EVENT( buf );
        i = 0;
      }
      else if(pos >= transmit_data->len)
      {
        TRACE_EVENT( buf );
      }
    }
  }
#endif /* _SIMULATION_ */

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_MUX:
      if(uart_data->tx.send_state EQ UART_TX_NOT_SENDING)
      {
        /*
         * determine next dlc allow to send
         */
        tx_next_send_allowed();
        /*
         * inform dlc about sending
         */
        if(uart_data->tx.dlc_instance == UART_EMPTY_INSTANCE)
        {
          /*
           * queue empty
           */
          break;
        }
        uart_data->tx.send_state = UART_TX_SENDING;
        dlc = &uart_data->dlc_table[uart_data->tx.dlc_instance];
        if(uart_data->tx.dlc_instance EQ UART_CONTROL_INSTANCE)
        {
          /*
           * Control channel
           */
          sig_tx_ker_sending_ind();
        }
        else
        {
          /*
           * Data channel
           */
          uart_data->drx = dlc->drx;
          sig_tx_drx_sending_req();
        }
        /*
         * transmit data
         */
#ifndef _SIMULATION_
        tx_flushUart();
#endif /* !_SIMULATION_ */
        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(0);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(1);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_1);
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
      TRACE_ERROR( "SIG_KER_TX_DATA_AVAILABLE_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_data_available_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_drx_tx_data_not_available_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DRX_TX_DATA_NOT_AVAILABLE_IND
|
| Parameters  : dlc_instance  - dlc instance wich belongs to calling DRX
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_drx_tx_data_not_available_ind (UBYTE dlc_instance)
{
  TRACE_ISIG( "sig_drx_tx_data_not_available_ind" );

  uart_data->dlc_table[dlc_instance].transmit_data = NULL;
} /* sig_drx_tx_data_not_available_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_data_not_available_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_DATA_NOT_AVAILABLE_REQ
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_data_not_available_req ()
{
  TRACE_ISIG( "sig_ker_tx_data_not_available_req" );

  uart_data->dlc_table[UART_CONTROL_INSTANCE].transmit_data = NULL;
} /* sig_ker_tx_data_not_available_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_line_states_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_LINE_STATES_REQ
|
| Parameters  : dlc_instance - DLC which contains new line states
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_line_states_req (UBYTE dlc_instance)
{
  T_DLC*  dlc;
  SHORT   ret = 0;        /* Error returned from a function */

  TRACE_ISIG( "sig_ker_tx_line_states_req" );

  /*
   * set DLC
   */
  dlc = &uart_data->dlc_table[dlc_instance];

  /*
   * UART number has to be checked.
   * UART IrDA (UAF_UART_0) can't be used for F&D on Ulysse because hardware
   * flow control is not supported.
   * DCD and DTR are not supported on UART Irda on C & D-Sample.
   */
  if((uart_data->tx.lines != dlc->lines))
  {
    /*
     * set new line states separatly because
     * if one line is not supported by the driver
     * we can still set the other lines
     */
    if(dlc->lines & UART_SA_TX_MASK)
    {
      if(((ret = UF_SetLineState(uart_data->device, (ULONG)(SA_MASK),
                                 (ULONG)(SA_MASK))) != UF_OK) && 
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }
    else
    {
      if(((ret = UF_SetLineState(uart_data->device, 0, 
                                 (ULONG)(SA_MASK))) != UF_OK) && 
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }

    if(dlc->lines & UART_SB_TX_MASK) /* also DCD */
    {
      if(((ret = UF_SetLineState(uart_data->device, 0, 
                                 (ULONG)(SB_MASK))) != UF_OK) &&
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }
    else
    {
      if(((ret = UF_SetLineState(uart_data->device, (ULONG)(SB_MASK),
                                (ULONG)(SB_MASK))) != UF_OK) && 
                                (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }

    if(dlc->lines & UART_X_TX_MASK)
    {
      if(((ret = UF_SetLineState(uart_data->device, (ULONG)(X_MASK),
                                (ULONG)(X_MASK))) != UF_OK) && 
                                (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }
    else
    {
      if(((ret = UF_SetLineState(uart_data->device, 0, 
                                 (ULONG)(X_MASK))) != UF_OK) &&
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }

    if(dlc->lines & UART_RI_MASK)
    {
      if(((ret = UF_SetLineState(uart_data->device, (ULONG)(RI_MASK),
                                 (ULONG)(RI_MASK))) != UF_OK) && 
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }
    else
    {
      if(((ret = UF_SetLineState(uart_data->device, 0, 
                                 (ULONG)(RI_MASK))) != UF_OK) && 
                                 (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
    }

    if(dlc->lines & UART_BRK_TX_MASK)
    {
      /*
       * send break
       */
      if(((ret = UF_SetLineState(uart_data->device,
                                 (ULONG)((1UL<<BRK) |
                                 (((dlc->lines & UART_BRKLEN_TX_MASK)
                                 >> UART_BRKLEN_TX_POS)
                                 << BRKLEN)),
                                 (ULONG)(BRK_MASK  | BRK_LEN_MASK))) != UF_OK)
                                 && (uart_data->device != 0))
      {
        TRACE_ERROR_P2("UF driver: SetLineState failed, [%d], uart_txs.c(%d)", 
                        ret, __LINE__);
      }
      /*
       * break sent, so clear break flag
       */
      dlc->lines&= ~UART_BRK_TX_MASK;
    }
    uart_data->tx.lines = dlc->lines;
  }
} /* sig_ker_tx_line_states_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_flush_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_FLUSH_REQ
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_flush_req ()
{
  TRACE_ISIG( "sig_ker_tx_flush_req" );

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_READY:
    case TX_MUX:
#ifndef _SIMULATION_
      tx_flushUart();
#endif /* !_SIMULATION_ */
      sig_tx_ker_flushed_ind();
      break;

    default:
      TRACE_ERROR( "SIG_KER_TX_FLUSH_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_flush_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_tx_restart_write_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_TX_RESTART_WRITE_REQ
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_tx_restart_write_req ()
{
  TRACE_ISIG( "sig_ker_tx_restart_write_req" );

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_READY:
    case TX_MUX:
      if(uart_data->tx.send_state EQ UART_TX_SENDING)
      {
        /*
         * restart writeInFunc
         */
#ifndef _SIMULATION_
        tx_flushUart();
#endif /* !_SIMULATION_ */
        if(uart_data EQ (&(uart_data_base[0])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(0);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_0);
#endif /* else _SIMULATION_ */
        }
#ifdef FF_TWO_UART_PORTS
        else if(uart_data EQ (&(uart_data_base[1])))
        {
          TRACE_EVENT("UF_WriteData()");
#ifdef _SIMULATION_
          tx_writedata(1);
#else /* _SIMULATION_ */
          UF_WriteData (uart_data->device, sm_suspend, tx_writeInFunc_1);
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
      TRACE_ERROR( "SIG_KER_TX_RESTART_WRITE_REQ unexpected" );
      break;
  }
} /* sig_ker_tx_restart_write_req() */
#endif /* !FF_MULTI_PORT */

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
|             functions to handles the incoming primitives as described in
|             the SDL-documentation (RX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_RXP_C
#define UART_RXP_C
#endif /* !UART_RXP_C */

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
#include "dti.h"        /* to get dti lib */
#include "pei.h"        /* to get PEI interfac */
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
#include <stdio.h>      /* to get sprintf */
#include "uart_rxp.h"   /* to get rx_readdata */
#endif /* _SIMULATION_ */
/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : rx_uart_driver_received_ind
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_DRIVER_RECEIVED_IND
|
| Parameters  : *uart_device - affected device database
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_uart_driver_received_ind ( T_UART_DATA* uart_device )
{
  USHORT    i;
  BOOL      continuous;
  T_DLC*    dlc;
  T_desc2*  temp_desc = NULL;
  ULONG     line_states;

  TRACE_EVENT( "rx_uart_driver_received_ind()" );

  /*
   * set affected instance
   */
  uart_data = uart_device;

  /*
   * inform about new line states
   */
  if(uart_data->rx.lines NEQ uart_data->rx.prev_lines)
  {
    /*
     * convert line states and send it
     */
    line_states = 0;
    if(uart_data->rx.lines & X_MASK)
    {
      line_states|= UART_X_RX_MASK;
      if(!(uart_data->rx.prev_lines & X_MASK))
      {
        TRACE_EVENT("RX Flow Control: stop");
      }
    }
    else if(uart_data->rx.prev_lines & X_MASK)
    {
      TRACE_EVENT("RX Flow Control: start");
    }

    if(uart_data->rx.lines & SA_MASK)
    {
      line_states|= UART_SA_RX_MASK;
      if(!(uart_data->rx.prev_lines & SA_MASK))
      {
        TRACE_EVENT("DTR: drop");
      }
    }
    else if(uart_data->rx.prev_lines & SA_MASK)
    {
      TRACE_EVENT("DTR: on");
    }

    if(uart_data->rx.lines & SB_MASK)
    {
      line_states|= UART_SB_RX_MASK;
      if(!(uart_data->rx.prev_lines & SB_MASK))
      {
        TRACE_EVENT("RTS: off");
      }
    }
    else if(uart_data->rx.prev_lines & SB_MASK)
    {
      TRACE_EVENT("RTS: on");
    }

    if(uart_data->rx.lines & ESC_MASK)
    {
      line_states|= UART_ESC_RX_MASK;
      TRACE_EVENT("Escape Sequence detected");
    }

    if(uart_data->rx.lines & BRK_MASK)
    {
      line_states|= UART_BRK_RX_MASK;
      line_states|= (((uart_data->rx.lines & BRK_LEN_MASK)
                     >> BRKLEN) << UART_BRKLEN_RX_POS);
      TRACE_EVENT("Break detected");
    }
    /*
     * store new line states
     */
    uart_data->rx.lines&= ~(ESC_MASK |
                            BRK_MASK |
                            BRK_LEN_MASK);
    uart_data->rx.prev_lines = uart_data->rx.lines;
    /*
     * inform MMI
     */
    sig_rx_ker_line_states_ind(line_states);
  }

  switch( GET_STATE( UART_SERVICE_RX ) )
  {
    case RX_READY:
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      if(uart_data->rx.dlc_instance NEQ UART_EMPTY_INSTANCE)
      {
        uart_data->rx.dlc_instance  = UART_EMPTY_INSTANCE;
        uart_data->rx.analyze_state = UART_RX_ERROR;
        /*
         * if ISR has read out some data
         * inform all channels about data reception
         */
        if(uart_data->rx.read_permission)
        {
          for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
          {
            switch(uart_data->dlc_table[i].receive_process)
            {
              case UART_RX_PROCESS_READY:
              case UART_RX_PROCESS_COMPLETE:
                /*
                 * inform all channels about data reception
                 */
                uart_data->dlc_table[i].receive_process = UART_RX_PROCESS_STOP;
                temp_desc = uart_data->dlc_table[i].receive_data;
                uart_data->dlc_table[i].receive_data = NULL;
                if(i EQ UART_CONTROL_INSTANCE)
                {
                  /*
                   * Control channel
                   */
                  sig_rx_ker_data_received_ind(
                    temp_desc,
                    uart_data->dlc_table[i].receive_pos);
                }
                else
                {
                  /*
                   * Data channel
                   */
                  uart_data->dtx = uart_data->dlc_table[i].dtx;
                  sig_rx_dtx_data_received_ind(
                    temp_desc,
                    uart_data->dlc_table[i].receive_pos);
                }
                /* fall through */
              case UART_RX_PROCESS_STOP:
                /*
                 * add new channels which want to receive
                 */
                if(uart_data->dlc_table[i].receive_data)
                  uart_data->dlc_table[i].receive_process = UART_RX_PROCESS_READY;
                break;

              default:
                TRACE_EVENT_P2("Unexpected DLC process state: %d | uart_rxp.c(%d)",
                               dlc->receive_process, __LINE__);
                break;
            }
          }
        }
      }
      else
      {
        switch(dlc->receive_process)
        {
          case UART_RX_PROCESS_READY:
          case UART_RX_PROCESS_COMPLETE:
            /*
             * if ISR has read out some data
             * inform channel about data reception
             */
            if(uart_data->rx.read_permission)
            {
              /*
               * inform channel about data reception
               */
              dlc->receive_process = UART_RX_PROCESS_STOP;
              temp_desc            = dlc->receive_data;
              dlc->receive_data    = NULL;
              uart_data->dtx       = dlc->dtx;
              sig_rx_dtx_data_received_ind(temp_desc, dlc->receive_pos);
            }
            /* fall through */
          case UART_RX_PROCESS_STOP:
            /*
             * add new channel which want to receive
             */
            if(dlc->receive_data)
              dlc->receive_process = UART_RX_PROCESS_READY;
            break;

          default:
            TRACE_EVENT_P2("Unexpected DLC process state: %d | uart_rxp.c(%d)",
                                               dlc->receive_process, __LINE__);
            break;
        }
      }
      if(dlc->receive_process EQ UART_RX_PROCESS_STOP)
      {
        uart_data->rx.receive_state = UART_RX_NOT_RECEIVING;
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
      break;

    case RX_MUX:
      if(uart_data->rx.dlc_instance EQ UART_EMPTY_INSTANCE)
      {
        uart_data->rx.dlc_instance  = UART_CONTROL_INSTANCE;
        uart_data->rx.analyze_state = UART_RX_ERROR;
      }
      continuous = FALSE;
      for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        switch(dlc->receive_process)
        {
          case UART_RX_PROCESS_READY:
          case UART_RX_PROCESS_COMPLETE:
            /*
             * if ISR has read out some data
             * inform all channels about data reception
             */
            if(uart_data->rx.read_permission)
            {
              dlc->receive_process = UART_RX_PROCESS_STOP;
              temp_desc            = dlc->receive_data;
              dlc->receive_data    = NULL;
              if(i EQ UART_CONTROL_INSTANCE)
              {
                /*
                 * Control channel
                 */
                sig_rx_ker_data_received_ind(temp_desc, dlc->receive_pos);
              }
              else
              {
                /*
                 * Data channel
                 */
                uart_data->dtx = dlc->dtx;
                sig_rx_dtx_data_received_ind(temp_desc, dlc->receive_pos);
              }
            }
            /* fall through */
          case UART_RX_PROCESS_STOP:
            /*
             * add new channels which want to receive
             */
            if(dlc->receive_data)
              dlc->receive_process = UART_RX_PROCESS_READY;
            break;

          default:
            TRACE_EVENT_P2("Unexpected DLC process state: %d, uart_rxp.c(%d)",
                                              dlc->receive_process, __LINE__);
            break;
}
        if(dlc->receive_process NEQ UART_RX_PROCESS_STOP)
          continuous = TRUE;
      }
      /*
       * check whether there is a channel to receive
       */
      if(continuous NEQ TRUE)
      {
        uart_data->rx.receive_state = UART_RX_NOT_RECEIVING;
        break;
      }
#ifdef _SIMULATION_
      if(rx_inpavail(uart_data->device) > 0)
#else /* _SIMULATION_ */
      if(UF_InpAvail (uart_data->device) > 0)
#endif /* _SIMULATION_ */
      {
        /*
         * inform each channel about reading
         */
        uart_data->rx.read_permission = TRUE;
        for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
        {
          if(uart_data->dlc_table[i].receive_process EQ UART_RX_PROCESS_READY)
          {
            if(i EQ UART_CONTROL_INSTANCE)
            {
              /*
               * Control channel
               */
              sig_rx_ker_receiving_ind();
            }
            else
            {
              /*
               * Data channel
               */
              uart_data->dtx = uart_data->dlc_table[i].dtx;
              sig_rx_dtx_receiving_ind();
            }
          }
        }
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
      break;

    default:
      TRACE_ERROR( "UART_DRIVER_RECEIVED_IND unexpected" );
      break;
  }
} /* rx_uart_driver_received_ind() */



#ifdef _SIMULATION_
/*
+------------------------------------------------------------------------------
| Function    : rx_dti_data_test_ind
+------------------------------------------------------------------------------
| Description : Handles the primitive DTI_DATA_TEST_IND
|
| Parameters  : *dti_data_test_ind - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_dti_data_test_ind ( T_DTI2_DATA_TEST_IND *dti_data_test_ind )
{
  char          buf[100];
  T_reInstMode  reInstall;
  USHORT        size[2];
  USHORT        pos;
  USHORT        i;
  T_UART_DATA*  uart_device;

  TRACE_FUNCTION( "rx_dti_data_test_ind" );

  /*
   * set UART instance
   */
  uart_device = &(uart_data_base[UART_TEST_C_ID_1]);
  /*
   * copy data to simulation buffer
   */
  MFREE_DESC2(uart_device->rx.sim_buffer);
  MALLOC(uart_device->rx.sim_buffer, (USHORT)(sizeof(T_desc2) - 1 +
                                     (dti_data_test_ind->sdu.l_buf >> 3)));
  memcpy(uart_device->rx.sim_buffer->buffer,
         &dti_data_test_ind->sdu.buf[dti_data_test_ind->sdu.o_buf >> 3],
         dti_data_test_ind->sdu.l_buf >> 3);
  uart_device->rx.sim_buffer->len  = dti_data_test_ind->sdu.l_buf >> 3;
  uart_device->rx.sim_buffer->next = (ULONG)NULL;
  uart_device->rx.sim_pos          = 0;
  /*
   * trace output
   */
  sprintf(buf, "UART device %d:", dti_data_test_ind->link_id);
  TRACE_FUNCTION( buf );
  i   = 0;
  pos = uart_device->rx.sim_pos;
  while(pos < uart_device->rx.sim_buffer->len)
  {
    i+= sprintf(&buf[i], "0x%02x, ", uart_device->rx.sim_buffer->buffer[pos]);
    pos++;
    if(i > 80)
    {
      TRACE_FUNCTION( buf );
      i = 0;
    }
    else if(pos >= uart_device->rx.sim_buffer->len)
    {
      TRACE_FUNCTION( buf );
    }
  }
  /*
   * set values for ISR
   */
  uart_device->rx.source[0] = &uart_device->rx.sim_buffer->buffer[
                              uart_device->rx.sim_pos];
  uart_device->rx.source[1] = NULL;
  size[0]                   = uart_device->rx.sim_buffer->len -
                              uart_device->rx.sim_pos;
  size[1]                   = 0;

  /*
   * call actual function
   */
  rx_readOutFunc_0 (FALSE, &reInstall, 1, uart_device->rx.source, size, 0);

  /*
   * store return values
   */
  if(size[0] EQ 0)
  {
    MFREE_DESC2(uart_device->rx.sim_buffer);
    uart_device->rx.sim_buffer = NULL;
  }
  else
    uart_device->rx.sim_pos = uart_device->rx.sim_buffer->len - size[0];

  /*
   * free the primitive
   */
  PFREE(dti_data_test_ind);
} /* rx_dti_data_test_ind() */



/*
+------------------------------------------------------------------------------
| Function    : rx_readdata
+------------------------------------------------------------------------------
| Description : Simulates a UF_ReadData() call.
|
| Parameters  : caller - calling UART instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void rx_readdata (UBYTE caller)
{
  T_reInstMode  reInstall;
  USHORT        size[2];
  T_UART_DATA*  uart_device;

  TRACE_FUNCTION( "rx_readdata" );

  /*
   * set UART instance
   */
  uart_device = &(uart_data_base[caller]);

  if(uart_device->rx.sim_buffer EQ NULL)
  {
    /*
     * send DTI_GETDATA_REQ
     */
    PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
    dti_getdata_req->link_id = LINK_READDATA_PORT_1; /* for usual read_data */
    PSEND (hCommMMI, dti_getdata_req);
  }
  else
  {
    /*
     * set values for ISR
     */
    uart_device->rx.source[0] = NULL;
    uart_device->rx.source[1] = &uart_device->rx.sim_buffer->buffer[
                                 uart_device->rx.sim_pos];
    size[0]                   = 0;
    size[1]                   = uart_device->rx.sim_buffer->len -
                                uart_device->rx.sim_pos;

    /*
     * call actual function
     */
    if(caller EQ 0)
    {
      rx_readOutFunc_0 (FALSE, &reInstall, 2, uart_device->rx.source, size, 0);
    }
#ifdef FF_TWO_UART_PORTS
    else if(caller EQ 1)
    {
      rx_readOutFunc_1 (FALSE, &reInstall, 2, uart_device->rx.source, size, 0);
    }
#endif /* FF_TWO_UART_PORTS */
    else
    {
      TRACE_ERROR("wrong caller value");
    }

    /*
     * store return values
     */
    if(size[1] EQ 0)
    {
      MFREE_DESC2(uart_device->rx.sim_buffer);
      uart_device->rx.sim_buffer = NULL;
    }
    else
      uart_device->rx.sim_pos = uart_device->rx.sim_buffer->len - size[1];
  }
} /* rx_readdata() */



/*
+------------------------------------------------------------------------------
| Function    : rx_inpavail
+------------------------------------------------------------------------------
| Description : Simulates a UF_InpAvail() call.
|
| Parameters  : caller - calling UART instance
|
| Return      : number of octets in Input Queue
|
+------------------------------------------------------------------------------
*/
GLOBAL USHORT rx_inpavail (UBYTE caller)
{
  T_UART_DATA*  uart_device;

  TRACE_FUNCTION( "rx_inpavail" );

  /*
   * set UART instance
   */
  uart_device = &(uart_data_base[caller]);

  if(uart_device->rx.sim_buffer)
    return uart_device->rx.sim_buffer->len - uart_device->rx.sim_pos;
  else
    return 0;
} /* rx_inpavail() */
#endif /* _SIMULATION_ */
#endif /* !FF_MULTI_PORT */

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
|             the SDL-documentation (TX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_TXP_C
#define UART_TXP_C
#endif /* !UART_TXP_C */

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
| Function    : tx_uart_driver_sent_ind
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_DRIVER_SENT_IND
|
| Parameters  : *uart_device - affected device database
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_uart_driver_sent_ind ( T_UART_DATA* uart_device )
{
  T_desc2* temp_desc;
  T_DLC*  dlc;

  TRACE_EVENT( "tx_uart_driver_sent_ind()" );

  /*
   * set affected instance
   */
  uart_data = uart_device;

  switch( GET_STATE( UART_SERVICE_TX ) )
  {
    case TX_READY:
      /*
       * send DATA_SENT signal
       */
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      uart_data->tx.dlc_instance = UART_EMPTY_INSTANCE;
      temp_desc                  = dlc->transmit_data;
      dlc->transmit_data         = NULL;
      uart_data->drx             = dlc->drx;
      sig_tx_drx_data_sent_req(temp_desc, dlc->transmit_pos);
      /*
       * determine whether there is still data to send
       */
      if(dlc->transmit_data EQ NULL)
      {
        /*
         * no more data
         */
        uart_data->tx.send_state = UART_TX_NOT_SENDING;
        break;
      }
      /*
       * inform dlc about sending
       */
      uart_data->drx = dlc->drx;
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
      break;

    case TX_MUX:
      /*
       * send DATA_SENT signal
       */
      if(uart_data->tx.dlc_instance EQ UART_EMPTY_INSTANCE)
      {
        /*
         * use entry 0 for raw data
         */
        dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      }
      else
      {
        dlc = &uart_data->dlc_table[uart_data->tx.dlc_instance];
      }
      temp_desc          = dlc->transmit_data;
      dlc->transmit_data = NULL;
      if(uart_data->tx.dlc_instance EQ UART_CONTROL_INSTANCE)
        sig_tx_ker_data_sent_ind(temp_desc, dlc->transmit_pos);
      else
      {
        uart_data->drx = dlc->drx;
        sig_tx_drx_data_sent_req(temp_desc, dlc->transmit_pos);
      }
      /*
       * determine next dlc allow to send
       */
      tx_next_send_allowed();
      if(uart_data->tx.dlc_instance EQ UART_EMPTY_INSTANCE)
      {
        /*
         * queue empty
         */
        uart_data->tx.send_state = UART_TX_NOT_SENDING;
        break;
      }
      /*
       * inform dlc about sending
       */
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
      break;

    default:
      TRACE_ERROR( "UART_DRIVER_SENT_IND unexpected" );
      break;
  }
} /* tx_uart_driver_sent_ind() */



#ifdef _SIMULATION_
/*
+------------------------------------------------------------------------------
| Function    : tx_dti_ready_ind
+------------------------------------------------------------------------------
| Description : Handles the primitive DTI_READY_IND
|
| Parameters  : *dti_ready_ind - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_dti_ready_ind ( T_DTI2_READY_IND *dti_ready_ind )
{
  TRACE_FUNCTION( "tx_dti_ready_ind" );
  
  /*
   * free the received primitive
   */
  PFREE(dti_ready_ind);
#ifdef DTI2 /* should combine "caller" and "read_data" */
            /* when using both devices in the test document?! */
  tx_uart_driver_sent_ind(&(uart_data_base[UART_TEST_C_ID_1]));
#else  /* DTI2 */
  tx_uart_driver_sent_ind(&(uart_data_base[dti_ready_ind->c_id]));
#endif /* DTI2 */
} /* tx_dti_ready_ind() */



/*
+------------------------------------------------------------------------------
| Function    : tx_writedata
+------------------------------------------------------------------------------
| Description : Simulates UF_WriteData call
|
| Parameters  : caller - calling UART instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void tx_writedata (UBYTE caller)
{
  char          buf[100];
  T_reInstMode  reInstall;
  USHORT        size[2];
  USHORT        pos;
  USHORT        i;
  T_UART_DATA*  uart_device;

  TRACE_FUNCTION( "tx_writedata" );

  /*
   * set UART instance
   */
  uart_device = &(uart_data_base[caller]);

  {
    /*
     * calculation of SDU length: ((((frame-size + 3) * 2) + 2) * 8)
     */
    PALLOC_SDU (dti_data_test_req, DTI2_DATA_TEST_REQ, 1088);
#ifdef DTI2 /* should switch for C_ID_1/2 when using both devices in the test document?! */
    dti_data_test_req->link_id               = LINK_WRITEDATA_PORT_1; /* for write_data call */
    dti_data_test_req->parameters.p_id       = 0;
    dti_data_test_req->parameters.st_lines.st_flow    = 0;
    dti_data_test_req->parameters.st_lines.st_line_sa = 0;
    dti_data_test_req->parameters.st_lines.st_line_sb = 0;
    dti_data_test_req->parameters.st_lines.st_break_len = 0;
    dti_data_test_req->sdu.o_buf             = 0;
#else  /* DTI2 */
    dti_data_test_req->tui        = 3; /* for write_data call */
    dti_data_test_req->c_id       = caller;
    dti_data_test_req->p_id       = 0;
    dti_data_test_req->op_ack     = 0;
    dti_data_test_req->st_flow    = 0;
    dti_data_test_req->st_line_sa = 0;
    dti_data_test_req->st_line_sb = 0;
    dti_data_test_req->st_escape  = 0;
    dti_data_test_req->sdu.o_buf  = 0;
#endif /* DTI2 */

    uart_device->tx.dest[0] = dti_data_test_req->sdu.buf;
    uart_device->tx.dest[1] = NULL;
    size[0]                 = (USHORT)(1088 >> 3);
    size[1]                 = 0;

    /*
     * call actual function
     */
    if(caller EQ 0)
    {
      tx_writeInFunc_0 (FALSE, &reInstall, 1, uart_device->tx.dest, size);
    }
#ifdef FF_TWO_UART_PORTS
    else if(caller EQ 1)
    {
      tx_writeInFunc_1 (FALSE, &reInstall, 1, uart_device->tx.dest, size);
    }
#endif /* FF_TWO_UART_PORTS */
    else
    {
      TRACE_ERROR("wrong caller value");
    }

    /*
     * set length of sdu
     */
    dti_data_test_req->sdu.l_buf  = ((USHORT)(1088 >> 3) - size[0]) << 3;

    /*
     * trace output 
     */
    i   = 0;
    pos = 0;
    while(pos < (dti_data_test_req->sdu.l_buf >> 3))
    {
      i+= sprintf(&buf[i], "0x%02x, ", dti_data_test_req->sdu.buf[pos]);
      pos++;
      if(i > 80)
      {
        TRACE_FUNCTION( buf );
        i = 0;
      }
      else if(pos >= (dti_data_test_req->sdu.l_buf >> 3))
      {
        TRACE_FUNCTION( buf );
      }
    }
    /*
     * send primitive
     */
    PSEND (hCommMMI, dti_data_test_req);
  }
} /* tx_writedata() */
#endif /* _SIMULATION_ */
#endif /* !FF_MULTI_PORT */

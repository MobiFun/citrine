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
|             the SDL-documentation (DTX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DTXP_C
#define UART_DTXP_C
#endif /* !UART_DTXP_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

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
#ifdef FF_MULTI_PORT
#include "gsi.h"        /* to get definitions of serial driver */
#else /* FF_MULTI_PORT */
#ifdef _TARGET_
#include "../../serial/serialswitch.h"
#include "../../serial/traceswitch.h"
#else /* _TARGET_ */
#include "serial_dat.h" /* to get definitions of serial driver */
#endif /* _TARGET_ */
#endif /* FF_MULTI_PORT */
#include "uart.h"       /* to get the global entity definitions */

#include "uart_dtxf.h"  /* to get DTX function definitions */
#include "uart_dtxp.h"  /* to get DTX primitive definitions */
#include "uart_kers.h"  /* to get signal definitions of service KER */
#ifdef FF_MULTI_PORT
#include "uart_prxs.h"  /* to get signal definitions for service TX */
#else /* FF_MULTI_PORT */
#include "uart_rxs.h"   /* to get signal definitions of service RX */
#endif /* FF_MULTI_PORT */

#ifdef _SIMULATION_
#include <stdio.h>      /* to get sprintf */
#endif /* _SIMULATION_ */
#include <string.h>    /* JK, delete warnings: to get memcpy */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


#ifdef DTILIB

/*
+------------------------------------------------------------------------------
| Function    : sig_dti_dtx_tx_buffer_ready_ind
+------------------------------------------------------------------------------
| Description : Handles the DTILIB callback call DTI_REASON_TX_BUFFER_READY
|               
|               This signal means that data may be sent over a dti connection.
|
| Parameters  : -
|
+------------------------------------------------------------------------------
 *
 * when not using DTILIB, the functionality of sig_dti_dtx_tx_buffer_ready_ind()
 * is to be found in dtx_dti_getdata_req(). When debugging, please have a look at
 * both versions!!
 */

GLOBAL void sig_dti_dtx_tx_buffer_ready_ind()
{
  USHORT            old_write_pos;
  T_DATA_FLOW_STATE old_data_flow;
  T_desc2*          temp_desc;

  TRACE_FUNCTION( "sig_dti_dtx_tx_buffer_ready_ind" );

#ifdef FLOW_TRACE
  sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_UP, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_NOT_READY:
      old_data_flow = uart_data->dtx->data_flow;
      old_write_pos = uart_data->dtx->write_pos;
      /*
       * enable data flow if necessary
       */
      if(uart_data->dtx->data_flow NEQ UART_FLOW_ENABLED)
      {
        uart_data->dtx->data_flow = UART_FLOW_ENABLED;
        sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
      }

        if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
        {
          /*
           * if data to send available or
           * line states changed
           */
          if((uart_data->dtx->to_send_data->len) ||
             (uart_data->dtx->lines_changed))
          {
            PALLOC_DESC2 (dti_data_ind, DTI2_DATA_IND);
            SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

            if(uart_data->dtx->to_send_data->len)
            {
              /*
               * mark entity descriptor as invalid, since data will be forwarded
               */
              temp_desc                    = uart_data->dtx->to_send_data;
              uart_data->dtx->to_send_data = NULL;

              dti_data_ind->desc_list2.first    = (ULONG)temp_desc;
              dti_data_ind->desc_list2.list_len = temp_desc->len;

              /*
               * calculate new size multiplier according to fillrate of buffer
               */
              dtx_calculate_size_multiplier (temp_desc, old_data_flow);
              /*
               * allocate a new descriptor with size according to new size_multiplier
               */
              dtx_allocate_resources();
              /*
               * Check for data which has not yet been validated, i.e. because
               * the frame containing the data has not yet been received completely.
               * In this case, the not yet validated data is copied to the newly
               * allocated descriptor.
               */
              if(old_write_pos > temp_desc->len)
              {
                memcpy(uart_data->dtx->to_send_data->buffer,
                       &temp_desc->buffer[temp_desc->len],
                       old_write_pos - temp_desc->len);
                uart_data->dtx->write_pos = old_write_pos - temp_desc->len;
              }
              uart_data->dtx->esd_pos-= temp_desc->len;
            }
            else
            {
              /*
               * just line states has been changed
               */
              dti_data_ind->desc_list2.first    = (ULONG)NULL;
              dti_data_ind->desc_list2.list_len = 0;
            }

            /*
             * set line states and
             * mark line states as unchanged;
             */
#ifdef DTI2
            dti_data_ind->parameters.st_lines.st_flow         = uart_data->dtx->st_flow;
            dti_data_ind->parameters.st_lines.st_line_sa      = uart_data->dtx->st_line_sa;
            dti_data_ind->parameters.st_lines.st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->parameters.st_lines.st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->parameters.st_lines.st_break_len    = uart_data->dtx->st_break_len;
#else  /* DTI2 */
            dti_data_ind->st_flow         = uart_data->dtx->st_flow;
            dti_data_ind->st_line_sa      = uart_data->dtx->st_line_sa;
            dti_data_ind->st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->st_escape       = uart_data->dtx->st_escape;
            dti_data_ind->tui  = uart_data->tui_uart;
#endif /* DTI2 */
            uart_data->dtx->lines_changed = FALSE;
            uart_data->dtx->st_break_len  = DTI_BREAK_OFF;

#ifdef _SIMULATION_
#ifndef DTI2
            dti_data_ind->op_ack = OP_UNACK;
#endif /* !DTI2 */
#else /* _SIMULATION_ */
#ifdef FLOW_TRACE
            sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_UP, FLOW_TRACE_TOP, FALSE);
#endif /* FLOW_TRACE */
#endif /* _SIMULATION_ */
            dti_send_data(
              uart_hDTI,
              uart_data->device,
              UART_DTI_UP_INTERFACE,
              uart_data->dtx->dlc_instance,
              dti_data_ind
              );
          }
          else
          {
            /*
             * no data to send
             */
            SET_STATE( UART_SERVICE_DTX, DTX_READY );
          }
          sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                           uart_data->dtx->to_send_data, 
                                           uart_data->dtx->write_pos, 
                                           uart_data->dtx->cur_desc_size );
        }
        else
        {
          /*
           * DTX service is currently receiving,
           * data will be forwarded to upper layer when 
           * sig_rx_dtx_data_received_ind() is called
           */
          SET_STATE( UART_SERVICE_DTX, DTX_READY );
        }
        break;

    default:
      TRACE_FUNCTION( "DTI_GETDATA_REQ unexpected" );
      break;
  }
} /* sig_dti_dtx_tx_buffer_ready_ind() */


/*
+------------------------------------------------------------------------------
| Function    : sig_dti_dtx_tx_buffer_full_ind
+------------------------------------------------------------------------------
| Description : Handles the DTILIB callback call DTI_REASON_TX_BUFFER_FULL
|               Since no send queue is used, this function does not have
|               any functionality by now
|               
|               This signal means that data may not be sent over a dti connection.
|
| Parameters  : -
|
+------------------------------------------------------------------------------
*/

GLOBAL void sig_dti_dtx_tx_buffer_full_ind()
{
  TRACE_FUNCTION( "sig_dti_dtx_tx_buffer_full_ind" );
} /* sig_dti_dtx_tx_buffer_full_ind() */

#else  /* DTILIB */
/*
+------------------------------------------------------------------------------
| Function    : dtx_dti_getdata_req
+------------------------------------------------------------------------------
| Description : Handles the primitive DTI_GETDATA_REQ
|
| Parameters  : *dti_getdata_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
 *
 * when using DTILIB, the functionality of dtx_dti_getdata_req() is to be found
 * in sig_dti_dtx_tx_buffer_ready_ind(). When debugging, please have a look at
 * both versions!!
 */
GLOBAL void dtx_dti_getdata_req ( T_DTI_GETDATA_REQ *dti_getdata_req )
{
  USHORT            old_write_pos;
  T_DATA_FLOW_STATE old_data_flow;
  T_desc2*          temp_desc;

  TRACE_FUNCTION( "dtx_dti_getdata_req" );

#ifdef FLOW_TRACE
  sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_UP, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */

#ifdef UART_RANGE_CHECK
  if(dti_getdata_req EQ NULL)
  {
    TRACE_EVENT("ERROR: dti_getdata_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)dti_getdata_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: dti_getdata_req=%08x is not allocated",
                    dti_getdata_req);
  }
#endif /* UART_RANGE_CHECK */

  if( pei_select_instances( dti_getdata_req->c_id ) EQ TRUE )
  {
    switch( GET_STATE( UART_SERVICE_DTX ) )
    {
      case DTX_NOT_READY:
        old_data_flow = uart_data->dtx->data_flow;
        old_write_pos = uart_data->dtx->write_pos;
        /*
         * enable data flow if necessary
         */
        if(uart_data->dtx->data_flow NEQ UART_FLOW_ENABLED)
        {
          uart_data->dtx->data_flow = UART_FLOW_ENABLED;
          sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
        }

        if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
        {
          /*
           * if data to send available or
           * line states changed
           */
          if((uart_data->dtx->to_send_data->len) ||
             (uart_data->dtx->lines_changed))
          {
            PALLOC_DESC2 (dti_data_ind, DTI_DATA_IND);
            SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

            if(uart_data->dtx->to_send_data->len)
            {
              /*
               * mark entity descriptor as invalid, since data will be forwarded
               */
              temp_desc                    = uart_data->dtx->to_send_data;
              uart_data->dtx->to_send_data = NULL;

              dti_data_ind->desc_list2.first    = (ULONG)temp_desc;
              dti_data_ind->desc_list2.list_len = temp_desc->len;

              /*
               * calculate new size multiplier according to fillrate of buffer
               */
              dtx_calculate_size_multiplier (temp_desc, old_data_flow);
              /*
               * allocate a new descriptor with size according to new size_multiplier
               */
              dtx_allocate_resources();
              /*
               * Check for data which has not yet been validated, i.e. because
               * the frame containing the data has not yet been received completely.
               * In this case, the not yet validated data is copied to the newly
               * allocated descriptor.
               */
              if(old_write_pos > temp_desc->len)
              {
                memcpy(uart_data->dtx->to_send_data->buffer,
                       &temp_desc->buffer[temp_desc->len],
                       old_write_pos - temp_desc->len);
                uart_data->dtx->write_pos = old_write_pos - temp_desc->len;
              }
              uart_data->dtx->esd_pos-= temp_desc->len;
            }
            else
            {
              /*
               * just line states has been changed
               */
              dti_data_ind->desc_list2.first    = (ULONG)NULL;
              dti_data_ind->desc_list2.list_len = 0;
            }

            /*
             * set line states and
             * mark line states as unchanged;
             */
            dti_data_ind->st_flow         = uart_data->dtx->st_flow;
            dti_data_ind->st_line_sa      = uart_data->dtx->st_line_sa;
            dti_data_ind->st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->st_line_sb      = uart_data->dtx->st_line_sb;
            dti_data_ind->st_escape       = uart_data->dtx->st_escape;
            uart_data->dtx->lines_changed = FALSE;
            uart_data->dtx->st_escape     = DTI_ESC_OFF;

            dti_data_ind->tui  = uart_data->tui_uart;
            dti_data_ind->c_id = dtx_get_channel_id();
#ifdef _SIMULATION_
            dti_data_ind->op_ack = OP_UNACK;

            dtx_psend_dti_data_test_ind(dti_data_ind);
#else /* _SIMULATION_ */
#ifdef FLOW_TRACE
            sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_UP, FLOW_TRACE_TOP, FALSE);
#endif /* FLOW_TRACE */
            PSEND (uart_data->dtx->hComm_DTX_UPLINK, dti_data_ind);
#endif /* _SIMULATION_ */
          }
          else
          {
            /*
             * no data to send
             */
            SET_STATE( UART_SERVICE_DTX, DTX_READY );
          }
          sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                           uart_data->dtx->to_send_data, 
                                           uart_data->dtx->write_pos, 
                                           uart_data->dtx->cur_desc_size );
        }
        else
        {
          /*
           * DTX service is currently receiving,
           * data will be forwarded to upper layer when 
           * sig_rx_dtx_data_received_ind() is called
           */
          SET_STATE( UART_SERVICE_DTX, DTX_READY );
        }
        break;

      default:
        TRACE_FUNCTION( "DTI_GETDATA_REQ unexpected" );
        break;
    }
  }
  PFREE( dti_getdata_req );
} /* dtx_dti_getdata_req() */
#endif /* DTILIB */


#if defined ( _SIMULATION_ )
#ifndef DTILIB
/*
+------------------------------------------------------------------------------
| Function    : dtx_psend_dti_data_test_ind
+------------------------------------------------------------------------------
| Description : Copies the content of the given DTI DATA IND primitive to a new
|               DTI_DATA_TEST_IND primitive, sends the primitive 
|               DTI_DATA_TEST_IND to hCommUPLINK and frees the original 
|               DTI_DATA_IND primitive. 
|               Note: function is only needed in case of simulation target
|
| Parameters  : *dti_data_ind - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_psend_dti_data_test_ind ( T_DTI_DATA_IND *dti_data_ind )
{
  T_desc2* temp_desc;
  USHORT  packet_len;
  USHORT  pos;
  char    buf[100];
  USHORT  i;

  TRACE_FUNCTION( "dtx_psend_dti_data_test_ind" );

  /*
   * create new primitive and copy all parameters into new primitive
   */
  packet_len = dti_data_ind->desc_list2.list_len;
  {
    PALLOC_SDU (dti_data_test_ind, 
                DTI_DATA_TEST_IND, 
                (USHORT)(packet_len << 3));
    dti_data_test_ind->tui        = dti_data_ind->tui;
    dti_data_test_ind->c_id       = dti_data_ind->c_id;
    dti_data_test_ind->p_id       = dti_data_ind->p_id;
    dti_data_test_ind->op_ack     = dti_data_ind->op_ack;
    dti_data_test_ind->st_flow    = dti_data_ind->st_flow;
    dti_data_test_ind->st_line_sa = dti_data_ind->st_line_sa;
    dti_data_test_ind->st_line_sb = dti_data_ind->st_line_sb;
    dti_data_test_ind->st_escape  = dti_data_ind->st_escape;
    /*
     * copy generic data descriptor in sdu
     */
    temp_desc = (T_desc2*)dti_data_ind->desc_list2.first;
    dti_data_test_ind->sdu.o_buf = 0;
    dti_data_test_ind->sdu.l_buf = (packet_len << 3);
    pos = 0;
    while(temp_desc)
    {
      memcpy(&dti_data_test_ind->sdu.buf[pos], 
             temp_desc->buffer, 
             temp_desc->len);
      pos      += temp_desc->len;
      temp_desc = (T_desc2*)temp_desc->next;
    }
    /*
     * trace output 
     */
    i   = 0;
    pos = 0;
    while(pos < packet_len)
    {
      i+= sprintf(&buf[i], "0x%02x, ", dti_data_test_ind->sdu.buf[pos]);
      pos++;
      if(i > 80)
      {
        TRACE_FUNCTION( buf );
        i = 0;
      }
      else if(pos >= packet_len)
      {
        TRACE_FUNCTION( buf );
      }
    }
    /*
     * send primitive
     */
    PSEND (uart_data->dtx->hComm_DTX_UPLINK, dti_data_test_ind);
  }
  /*
   * free the primitive
   */
  PFREE_DESC(dti_data_ind);
} /* dtx_psend_dti_data_test_ind() */
#endif /* !DTILIB */
#endif /* _SIMULATION_ */


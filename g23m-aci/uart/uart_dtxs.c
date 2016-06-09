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
|             described in the SDL-documentation (DTX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_DTXS_C
#define UART_DTXS_C
#endif /* !UART_DTXS_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

/*==== INCLUDES =============================================================*/

#ifdef _SIMULATION_
#include <stdio.h>
#endif
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
#include "uart_kers.h"  /* to get KER signal definitions */
#include "uart_rts.h"   /* to get RT  signal definitions */
#ifdef FF_MULTI_PORT
#include "uart_prxs.h"  /* to get signal definitions for service TX */
#else /* FF_MULTI_PORT */
#include "uart_rxs.h"   /* to get TX signal definitions */
#endif /* FF_MULTI_PORT */
#include <string.h>    /* JK, delete warnings: to get memmove, memcpy */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_ready_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_READY_MODE_REQ
|
| Parameters  : dlc_instance - dlc instance wich belongs to this DTX instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_ready_mode_req (UBYTE dlc_instance)
{
  TRACE_ISIG( "sig_ker_dtx_ready_mode_req" );

  uart_data->dtx->dlc_instance = dlc_instance;

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_DEAD:
      SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

      /*
     * reset line states
     */
    uart_data->dtx->st_flow       = DTI_FLOW_ON;
    uart_data->dtx->st_line_sa    = DTI_SA_ON;
    uart_data->dtx->st_line_sb    = DTI_SB_ON;
    uart_data->dtx->st_break_len  = DTI_BREAK_OFF;
    uart_data->dtx->detect_escape = TRUE;

      uart_data->dtx->data_flow = UART_FLOW_ENABLED;
      if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
      {
        /*
         * reset escape sequence detection
         */
        dtx_set_esd_state( UART_ESD_NULL );
        uart_data->dtx->detect_escape = TRUE;
        if(vsi_t_time (VSI_CALLER &(uart_data->dtx->esd_guard_time))
                                                         NEQ VSI_OK)
        {
          TRACE_ERROR_P1("VSI entity: Can't restart timer, uart_dtxs.c(%d)",
                                                                  __LINE__);
        }
        uart_data->dtx->esd_pos = 0;
        /*
         * start reception
         */
        dtx_allocate_resources();
        sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                         uart_data->dtx->to_send_data,
                                         uart_data->dtx->write_pos,
                                         uart_data->dtx->cur_desc_size );
      }
      break;

    case DTX_READY:
    case DTX_NOT_READY:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DTX_READY_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_dtx_ready_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_dead_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_DEAD_MODE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_dead_mode_req ()
{
  TRACE_ISIG( "sig_ker_dtx_dead_mode_req" );

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_READY:
    case DTX_NOT_READY:
      SET_STATE( UART_SERVICE_DTX, DTX_DEAD );
      /*
       * reset hComm_DTX_UPLINK and size_multiplier
       */
      uart_data->dtx->dti_dtx_state   = DTI_CLOSED;
      uart_data->dtx->size_multiplier = 3;

      if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
      {
        /*
         * free recources and stop receiving
         */
        sig_dtx_rx_not_ready_to_receive_req(uart_data->dtx->dlc_instance);
        dtx_free_resources();
      }
      else
        uart_data->dtx->receiving_state = UART_DTX_INVALID;
      break;

    case DTX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DTX_DEAD_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_dtx_dead_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rx_dtx_receiving_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RX_DTX_RECEIVING_IND
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rx_dtx_receiving_ind ()
{
  TRACE_ISIG( "sig_rx_dtx_receiving_ind" );

  uart_data->dtx->receiving_state = UART_DTX_RECEIVING;

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_READY:
    case DTX_NOT_READY:
      break;

    default:
      TRACE_ERROR( "SIG_RX_DTX_RECEIVING_IND unexpected" );
      break;
  }
} /* sig_ker_dtx_receiving_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rx_dtx_data_received_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RX_DTX_DATA_RECEIVED_IND
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rx_dtx_data_received_ind (T_desc2* received_data,
                                          USHORT  write_pos)
{
  T_DATA_FLOW_STATE old_data_flow;
  T_desc2*          temp_desc;
  USHORT            esd_pos;
  T_TIME            cur_time;

  TRACE_ISIG( "sig_rx_dtx_data_received_ind" );

#ifdef UART_RANGE_CHECK
  if(received_data EQ NULL)
  {
    TRACE_EVENT("ERROR: received_data is NULL");
  }
  else if((*((ULONG*)((UBYTE*)received_data - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: received_data=%08x is not allocated",
                    received_data);
  }
  if(uart_data->dtx->to_send_data NEQ received_data)
  {
    TRACE_EVENT_P2("ERROR: to_send_data=%08x NEQ received_data=%08x",
                    uart_data->dtx->to_send_data,
                    received_data);
  }
  if(uart_data->dtx->to_send_data->len > uart_data->dtx->cur_desc_size)
  {
    TRACE_EVENT_P2("ERROR: to_send_data->len=%d > cur_desc_size=%d",
                    uart_data->dtx->to_send_data->len,
                    uart_data->dtx->cur_desc_size);
  }
  if(write_pos > uart_data->dtx->cur_desc_size)
  {
    TRACE_EVENT_P2("ERROR: write_pos=%d > cur_desc_size=%d",
                    write_pos,
                    uart_data->dtx->cur_desc_size);
  }
  if(uart_data->dtx->to_send_data->len > write_pos)
  {
    TRACE_EVENT_P2("ERROR: to_send_data->len=%d > write_pos=%d",
                    uart_data->dtx->to_send_data->len,
                    write_pos);
  }
  if(uart_data->dtx->esd_pos > write_pos)
  {
    TRACE_EVENT_P2("ERROR: esd_pos=%d > write_pos=%d",
                    uart_data->dtx->esd_pos,
                    write_pos);
  }
  switch(dtx_get_esd_state())
  {
    case UART_ESD_DETECTED:
      if(uart_data->dtx->esd_pos < 3)
      {
        TRACE_EVENT_P1("ERROR: esd_pos=%d < 3 in UART_ESD_DETECTED state",
                        uart_data->dtx->esd_pos);
      }
      break;
    case UART_ESD_CHAR_1:
      if(uart_data->dtx->esd_pos < 1)
      {
        TRACE_EVENT_P1("ERROR: esd_pos=%d < 1 in UART_ESD_CHAR_1 state",
                        uart_data->dtx->esd_pos);
      }
      break;
    case UART_ESD_CHAR_2:
      if(uart_data->dtx->esd_pos < 2)
      {
        TRACE_EVENT_P1("ERROR: esd_pos=%d < 2 in UART_ESD_CHAR_2 state",
                        uart_data->dtx->esd_pos);
      }
      break;
    case UART_ESD_CHAR_3:
      if(uart_data->dtx->esd_pos < 3)
      {
        TRACE_EVENT_P1("ERROR: esd_pos=%d < 3 in UART_ESD_CHAR_3 state",
                        uart_data->dtx->esd_pos);
      }
      break;
  }
#endif /* UART_RANGE_CHECK */

  /*
   * store new write position,
   * current data_flow state and
   * current data descriptor
   */
  uart_data->dtx->write_pos = write_pos;
  old_data_flow             = uart_data->dtx->data_flow;
  temp_desc                 = received_data;

  /*
   * Introduce local variable here in order to prevent
   * the target compiler from doing wrong calculations ...
   */
  esd_pos = uart_data->dtx->esd_pos;

  /*
   * escape sequence detection
   */
  if(uart_data->dtx->receiving_state EQ UART_DTX_RECEIVING &&
     uart_data->dtx->detect_escape EQ TRUE)
  {
    if (vsi_t_time (VSI_CALLER &cur_time) EQ VSI_OK)
    {
      switch(dtx_get_esd_state())
      {
        case UART_ESD_DETECTED:
          /*
           * remove escape characters because escape sequence was detected
           */
          if(uart_data->dtx->write_pos > esd_pos)
          {
            memmove(&temp_desc->buffer[esd_pos - 3],
                    &temp_desc->buffer[esd_pos],
                    uart_data->dtx->write_pos - esd_pos);
          }
          uart_data->dtx->write_pos-= 3;
          esd_pos -= 3;
          uart_data->dtx->esd_pos = esd_pos;
          dtx_set_esd_state( UART_ESD_NULL );
          /* fall through */
        case UART_ESD_NULL:
#ifdef _SIMULATION_
          TRACE_EVENT_P2("uart_data->act_gp: %d, silence: %d",
                          uart_data->act_gp,
                          (cur_time - uart_data->dtx->esd_guard_time));
#endif /* _SIMULATION_ */
          if(esd_pos >= temp_desc->len)
            break;

          if(((cur_time -
               uart_data->dtx->esd_guard_time) < uart_data->act_gp) ||
             (temp_desc->
              buffer[esd_pos] NEQ uart_data->act_ec))
          {
            /*
             * set new reference time and
             * update esd_pos
             */
            esd_pos                        = temp_desc->len;
            uart_data->dtx->esd_pos        = esd_pos;
            uart_data->dtx->esd_guard_time = cur_time;
            break;
          }
          /*
           * first guard period complete and
           * first escape character detected
           */
#ifdef _SIMULATION_
          TRACE_EVENT("+ + + first guard period complete + + +");
#endif /* _SIMULATION_ */
          dtx_set_esd_state( UART_ESD_CHAR_1 );
          esd_pos++;
          uart_data->dtx->esd_pos        = esd_pos;
          uart_data->dtx->esd_guard_time = cur_time;
          sig_dtx_rt_start_tesd_req (uart_data->act_gp);
          /* fall trough */
        case UART_ESD_CHAR_1:
          if(esd_pos >= temp_desc->len)
          {
            /*
             * hide 1 character
             */
            temp_desc->len  = esd_pos - 1;
            temp_desc->size = temp_desc->len;
            break;
          }

          if(temp_desc->
             buffer[esd_pos] NEQ uart_data->act_ec)
          {
            /*
             * second character is not an escape character
             */
            dtx_set_esd_state( UART_ESD_NULL );
            esd_pos                        = temp_desc->len;
            uart_data->dtx->esd_pos        = esd_pos;
            uart_data->dtx->esd_guard_time = cur_time;
            break;
          }
          /*
           * second escape character received
           */
          dtx_set_esd_state( UART_ESD_CHAR_2 );
          esd_pos++;
          uart_data->dtx->esd_pos = esd_pos;
          /* fall trough */
        case UART_ESD_CHAR_2:
          if(esd_pos >= temp_desc->len)
          {
            /*
             * hide 2 characters
             */
            temp_desc->len  = esd_pos - 2;
            temp_desc->size = temp_desc->len;
            break;
          }
          /*
           * set new reference time
           */
          uart_data->dtx->esd_guard_time = cur_time;

          if(temp_desc->
             buffer[esd_pos] NEQ uart_data->act_ec)
          {
            /*
             * third character is not an escape character
             */
            dtx_set_esd_state( UART_ESD_NULL );
            esd_pos                 = temp_desc->len;
            uart_data->dtx->esd_pos = esd_pos;
            break;
          }
          /*
           * third escape character received
           */
          dtx_set_esd_state( UART_ESD_CHAR_3 );
          esd_pos++;
          uart_data->dtx->esd_pos = esd_pos;
          sig_dtx_rt_start_tesd_req (uart_data->act_gp);
          /* fall trough */
        case UART_ESD_CHAR_3:
          if(esd_pos >= temp_desc->len)
          {
            /*
             * hide 3 characters
             */
            temp_desc->len  = esd_pos - 3;
            temp_desc->size = temp_desc->len;
            break;
          }
          /*
           * fourth character received
           */
          dtx_set_esd_state( UART_ESD_NULL );
          esd_pos                        = temp_desc->len;
          uart_data->dtx->esd_pos        = esd_pos;
          uart_data->dtx->esd_guard_time = cur_time;
          break;

        default:
          TRACE_ERROR("wrong esd state");
          break;
      }
    }
    else
    {
      TRACE_ERROR_P1("VSI entity: Can't restart timer, uart_dtxs.c(%d)",
                                                              __LINE__);
    }
  }

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_DEAD:
      dtx_free_resources();
      uart_data->dtx->receiving_state = UART_DTX_NOT_RECEIVING;
      break;

    case DTX_READY:
      /*
       * enable data flow if necessary
       */
      if(old_data_flow NEQ UART_FLOW_ENABLED)
      {
        uart_data->dtx->data_flow = UART_FLOW_ENABLED;
        sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
      }

      if(uart_data->dtx->receiving_state EQ UART_DTX_RECEIVING)
      {
        /*
         * if data to send available or
         * line states changed
         */
        if((temp_desc->len) ||
           (uart_data->dtx->lines_changed))
        {
          PALLOC_DESC2 (dti_data_ind, DTI2_DATA_IND);
          SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

          if(temp_desc->len)
          {
            /*
             * mark entity descriptor as invalid, since data will be forwarded
             */
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
            if(uart_data->dtx->write_pos > temp_desc->len)
            {
              memcpy(uart_data->dtx->to_send_data->buffer,
                     &temp_desc->buffer[temp_desc->len],
                     uart_data->dtx->write_pos - temp_desc->len);
              uart_data->dtx->write_pos-= temp_desc->len;
            }
            esd_pos-= temp_desc->len;
            uart_data->dtx->esd_pos = esd_pos;
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
          dti_data_ind->parameters.st_lines.st_flow      = uart_data->dtx->st_flow;
          dti_data_ind->parameters.st_lines.st_line_sa   = uart_data->dtx->st_line_sa;
          dti_data_ind->parameters.st_lines.st_line_sb   = uart_data->dtx->st_line_sb;
          dti_data_ind->parameters.st_lines.st_break_len = uart_data->dtx->st_break_len;

          uart_data->dtx->lines_changed = FALSE;
          uart_data->dtx->st_break_len  = DTI_BREAK_OFF;

          dti_send_data(
            uart_hDTI,
            uart_data->device,
            UART_DTI_UP_INTERFACE,
            uart_data->dtx->dlc_instance,
            dti_data_ind
            );
        }
      }
      else
      {
        /*
         * invalid data
         * free recources and allocate a new descriptor because
         * size_multiplier may have changed
         */
        dtx_free_resources();
        dtx_allocate_resources();
        /*
         * reset escape sequence detection
         */
        dtx_set_esd_state( UART_ESD_NULL );
        if(vsi_t_time (VSI_CALLER &(uart_data->dtx->esd_guard_time)) NEQ VSI_OK)
        {
          TRACE_ERROR_P1("VSI entity: Can't restart timer, uart_dtxs.c(%d)",
                                                                  __LINE__);
        }

        esd_pos = 0;
        uart_data->dtx->esd_pos = esd_pos;
      }

      /*
       * signal availability to receive to RX service
       */
      uart_data->dtx->receiving_state = UART_DTX_NOT_RECEIVING;
      sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                       uart_data->dtx->to_send_data,
                                       uart_data->dtx->write_pos,
                                       uart_data->dtx->cur_desc_size );
      break;

    case DTX_NOT_READY:
      if(uart_data->dtx->receiving_state EQ UART_DTX_RECEIVING)
      {
        uart_data->dtx->receiving_state = UART_DTX_NOT_RECEIVING;
        if((old_data_flow NEQ UART_FLOW_DISABLED) &&
           ((uart_data->dtx->cur_desc_size - temp_desc->len) <
            ((USHORT)uart_data->n1 << 1)))
        {
          /*
           * the service DTX is receiving but there is not enough space left
           * therefore it is necessary to disable the data flow
           */
          uart_data->dtx->data_flow = UART_FLOW_DISABLED;
          sig_dtx_ker_disable_ind(uart_data->dtx->dlc_instance);
        }
        if((uart_data->dtx->cur_desc_size -
            esd_pos) >= uart_data->n1)
        {
          /*
           * there is still enough space left to continue reception
           */
          sig_dtx_rx_ready_to_receive_req(uart_data->dtx->dlc_instance,
                                          uart_data->dtx->to_send_data,
                                          uart_data->dtx->write_pos,
                                          uart_data->dtx->cur_desc_size);
        }
      }
      else
      {
        /*
         * invalid data
         * free recources and allocate a new descriptor
         * because size_multiplier may have changed
         */
        dtx_free_resources();
        dtx_allocate_resources();
        /*
         * reset escape sequence detection
         */
        dtx_set_esd_state( UART_ESD_NULL );
        if(vsi_t_time (VSI_CALLER &(uart_data->dtx->esd_guard_time)) NEQ VSI_OK)
        {
          TRACE_ERROR_P1("VSI entity: Can't restart timer, uart_dtxs.c(%d)",
                                                                  __LINE__);
        }

        esd_pos = 0;
        uart_data->dtx->esd_pos = esd_pos;
        /*
         * enable flow control if necessary
         */
        if(old_data_flow NEQ UART_FLOW_ENABLED)
        {
          uart_data->dtx->data_flow = UART_FLOW_ENABLED;
          sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
        }
        /*
         * signal availability to receive to RX service
         */
        uart_data->dtx->receiving_state = UART_DTX_NOT_RECEIVING;
        sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                         uart_data->dtx->to_send_data,
                                         uart_data->dtx->write_pos,
                                         uart_data->dtx->cur_desc_size );
      }
      break;

    default:
      TRACE_ERROR( "SIG_RX_DTX_DATA_RECEIVED_IND unexpected" );
      break;

  }
} /* sig_rx_dtx_data_received_ind() */


/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_line_states_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_LINE_STATES_REQ
|               which indicates that one or more line status signals have
|               changed
|
| Parameters  : st_flow      - flow control state (X bit)
|               st_line_sa   - line state SA
|               st_line_sa   - line state SB
|               st_break_len - break length
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_line_states_req(UBYTE st_flow,
                                        UBYTE st_line_sa,
                                        UBYTE st_line_sb,
                                        UBYTE st_break_len)
{
  TRACE_ISIG( "sig_ker_dtx_line_states_req" );

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_READY:
      if((st_flow      NEQ uart_data->dtx->st_flow)    ||
         (st_line_sa   NEQ uart_data->dtx->st_line_sa) ||
         (st_line_sb   NEQ uart_data->dtx->st_line_sb) ||
         (st_break_len NEQ uart_data->dtx->st_break_len))
      {
        /*
         * send line states to DTI peer
         */
        PALLOC_DESC2 (dti_data_ind, DTI2_DATA_IND);
        SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

        /*
         * store new line states
         */
        uart_data->dtx->st_flow       = st_flow;
        uart_data->dtx->st_line_sa    = st_line_sa;
        uart_data->dtx->st_line_sb    = st_line_sb;
        uart_data->dtx->st_break_len  = st_break_len;

        /*
         * just line states has been changed
         */
        dti_data_ind->desc_list2.first    = (ULONG)NULL;
        dti_data_ind->desc_list2.list_len = 0;
        /*
         * set line states and
         * mark line states as unchanged;
         */
        dti_data_ind->parameters.st_lines.st_flow      = uart_data->dtx->st_flow;
        dti_data_ind->parameters.st_lines.st_line_sa   = uart_data->dtx->st_line_sa;
        dti_data_ind->parameters.st_lines.st_line_sb   = uart_data->dtx->st_line_sb;
        dti_data_ind->parameters.st_lines.st_line_sb   = uart_data->dtx->st_line_sb;
        dti_data_ind->parameters.st_lines.st_break_len = uart_data->dtx->st_break_len;

        uart_data->dtx->lines_changed = FALSE;
        uart_data->dtx->st_break_len  = DTI_BREAK_OFF;

        dti_send_data(
          uart_hDTI,
          uart_data->device,
          UART_DTI_UP_INTERFACE,
          uart_data->dtx->dlc_instance,
          dti_data_ind
          );
      }
      break;

    case DTX_NOT_READY:
      if((st_flow      NEQ uart_data->dtx->st_flow)    ||
         (st_line_sa   NEQ uart_data->dtx->st_line_sa) ||
         (st_line_sb   NEQ uart_data->dtx->st_line_sb) ||
         (st_break_len NEQ uart_data->dtx->st_break_len))
      {
        /*
         * If previously break detected keep information in
         * uart_data->dtx->st_break_len
         */
        if(uart_data->dtx->st_break_len EQ DTI_BREAK_OFF)
          uart_data->dtx->st_break_len = st_break_len;
        /*
         * store new line states
         */
        uart_data->dtx->st_flow    = st_flow;
        uart_data->dtx->st_line_sa = st_line_sa;
        uart_data->dtx->st_line_sb = st_line_sb;
        /*
         * mark line states as changed
         */
        uart_data->dtx->lines_changed = TRUE;
      }
      break;

    default:
      TRACE_ERROR( "SIG_KER_DTX_LINE_STATES_REQ unexpected" );
      break;
  }
} /* sig_ker_dtx_line_states_req */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_detect_escape_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_DETECT_ESCAPE_REQ
|               which enables escape sequence detection
|
| Parameters  : detect_escape - TRUE/FALSE
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_detect_escape_req (UBYTE detect_escape)
{
  TRACE_ISIG( "sig_ker_dtx_detect_req" );

  uart_data->dtx->detect_escape = detect_escape;
} /* sig_ker_dtx_detect_req() */


/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_disconnected_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_DISCONNECTED_MODE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_disconnected_mode_req ()
{
  TRACE_ISIG( "sig_ker_dtx_disconnected_mode_req" );

  uart_data->dtx->dti_dtx_state = DTI_CLOSED;

  switch(GET_STATE( UART_SERVICE_DTX) )
  {
    case DTX_READY:
      SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );
      break;
    case DTX_NOT_READY:
      break;
    default:
      TRACE_ERROR( "SIG_KER_DTX_DISCONNECTED_MODE_REQ unexpected" );
      break;
  }

} /* sig_ker_dtx_disconnected_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_set_dtilib_peer_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_SET_DTI_PEER_REQ
|               which is used to inform the service DTX that from now on it
|               needs to communicate with a (new) peer
|
| Parameters  : -
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_set_dtilib_peer_req ()

{
  TRACE_ISIG( "sig_ker_dtx_set_dtilib_peer_req" );

  /*
   * set dtilib parameters
   */
  uart_data->dtx->dti_dtx_state = DTI_IDLE;

  /*
   * reset size_multiplier
   */
  uart_data->dtx->size_multiplier = 3;

  /*
   * switch to new DTX state depending on current state
   */
  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_READY:
      SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );
      /* fall through */
    case DTX_NOT_READY:
      {
        if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
        {
          /*
           * reset received data
           */
          dtx_free_resources();
          dtx_allocate_resources();
          /*
           * reset escape sequence detection
           */
          dtx_set_esd_state( UART_ESD_NULL );
          if(vsi_t_time (VSI_CALLER &(uart_data->dtx->esd_guard_time)) NEQ VSI_OK)
          {
          TRACE_ERROR_P1("VSI entity: Can't restart timer, uart_dtxs.c(%d)",
                                                                  __LINE__);
          }

          uart_data->dtx->esd_pos = 0;
          /*
           * enable flow control if necessary
           */
          if(uart_data->dtx->data_flow NEQ UART_FLOW_ENABLED)
          {
            uart_data->dtx->data_flow = UART_FLOW_ENABLED;
            sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
          }
          /*
           * signal availability to receive to RX service
           */
          sig_dtx_rx_ready_to_receive_req( uart_data->dtx->dlc_instance,
                                           uart_data->dtx->to_send_data,
                                           uart_data->dtx->write_pos,
                                           uart_data->dtx->cur_desc_size );
        }
        else
          uart_data->dtx->receiving_state = UART_DTX_INVALID;
      }
      break;

    case DTX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DTX_SET_DTI_PEER_REQ unexpected" );
      break;
  }
} /* sig_ker_dtx_set_dtilib_peer_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_dtx_timeout_tesd_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DTX_TIMEOUT_TESD_REQ
|               which is used to inform the service DTX that the Escape
|               Sequence Guard Period timer has expired.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_dtx_timeout_tesd_req()
{
  T_DATA_FLOW_STATE old_data_flow;
  T_TIME            cur_time;
  T_TIME            elapsed;
  T_desc2*          temp_desc;

  TRACE_ISIG( "sig_ker_dtx_timeout_tesd_req" );

  switch( GET_STATE( UART_SERVICE_DTX ) )
  {
    case DTX_READY:
      if (vsi_t_time (VSI_CALLER &cur_time) EQ VSI_OK)
      {
        elapsed = cur_time - uart_data->dtx->esd_guard_time;
        switch (dtx_get_esd_state())
        {
          case UART_ESD_DETECTED:
          case UART_ESD_NULL:
            break;

          case UART_ESD_CHAR_1:
          case UART_ESD_CHAR_2:
            if(elapsed < uart_data->act_gp)
            {
              /*
               * escape sequence guard period not complete: start timer
               * with remaining time value
               */
              sig_dtx_rt_start_tesd_req (uart_data->act_gp - elapsed);
            }
            else
            {
              /*
               * Guard Period complete
               * reset detection because detected characters do not belong to
               * an escape sequence
               */
              dtx_set_esd_state( UART_ESD_NULL );

              /*
               * if possible send the escape character
               */
              if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
              {
                /*
                 * Guard Period complete
                 * send one escape character to DTI peer and reset detection
                 */
                PALLOC_DESC2 (dti_data_ind, DTI2_DATA_IND);
                SET_STATE( UART_SERVICE_DTX, DTX_NOT_READY );

                /*
                 * enable data flow if necessary
                 */
                old_data_flow = uart_data->dtx->data_flow;
                if(uart_data->dtx->data_flow NEQ UART_FLOW_ENABLED)
                {
                  uart_data->dtx->data_flow = UART_FLOW_ENABLED;
                  sig_dtx_ker_enable_ind(uart_data->dtx->dlc_instance);
                }
                /*
                 * mark entity descriptor as invalid, since data will be
                 * forwarded
                 */
                temp_desc                    = uart_data->dtx->to_send_data;
                uart_data->dtx->to_send_data = NULL;

                /*
                 * make escape character valid to send and insert values in
                 * primitive
                 */
                temp_desc->len  = uart_data->dtx->esd_pos;
                temp_desc->size = temp_desc->len;

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
                if(uart_data->dtx->write_pos > temp_desc->len)
                {
                  memcpy(uart_data->dtx->to_send_data->buffer,
                         &temp_desc->buffer[temp_desc->len],
                         uart_data->dtx->write_pos - temp_desc->len);
                  uart_data->dtx->write_pos-= temp_desc->len;
                }
                uart_data->dtx->esd_pos = 0;
                sig_dtx_rx_ready_to_receive_req(uart_data->dtx->dlc_instance,
                                                uart_data->dtx->to_send_data,
                                                uart_data->dtx->write_pos,
                                                uart_data->dtx->cur_desc_size);

                /*
                 * set line states
                 */
                dti_data_ind->parameters.st_lines.st_flow      = uart_data->dtx->st_flow;
                dti_data_ind->parameters.st_lines.st_line_sa   = uart_data->dtx->st_line_sa;
                dti_data_ind->parameters.st_lines.st_line_sb   = uart_data->dtx->st_line_sb;
                dti_data_ind->parameters.st_lines.st_break_len = uart_data->dtx->st_break_len;

                uart_data->dtx->lines_changed = FALSE;
                uart_data->dtx->st_break_len  = DTI_BREAK_OFF;

#ifdef _SIMULATION_
                dti_data_ind->parameters.p_id = DTI_PID_UOS;
#endif /* _SIMULATION_ */
                dti_send_data(
                  uart_hDTI,
                  uart_data->device,
                  UART_DTI_UP_INTERFACE,
                  uart_data->dtx->dlc_instance,
                  dti_data_ind
                  );
              }
            }
            break;

          case UART_ESD_CHAR_3:
            if(elapsed < uart_data->act_gp)
            {
              /*
               * escape sequence guard period not complete: start timer
               * with remaining time value
               */
              sig_dtx_rt_start_tesd_req (uart_data->act_gp - elapsed);
            }
            else
            {
              /*
               * Guard Period complete
               * Escape Sequence detected
               */
              /*
               * remove escape characters from data stream
               */
              if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
              {
                if(uart_data->dtx->write_pos > 3)
                {
                  memmove(uart_data->dtx->to_send_data->buffer,
                          &uart_data->dtx->to_send_data->buffer[3],
                          uart_data->dtx->write_pos - 3)
                  ; /*lint !e416 creation of out-of-bounds pointer */
                  uart_data->dtx->write_pos-= 3;
                }
                else
                  uart_data->dtx->write_pos = 0;
                uart_data->dtx->esd_pos = 0;
                sig_dtx_rx_ready_to_receive_req(uart_data->dtx->dlc_instance,
                                                uart_data->dtx->to_send_data,
                                                uart_data->dtx->write_pos,
                                                uart_data->dtx->cur_desc_size);
                /*
                 * Reset the state of the Escape Sequence Detection
                 */
                dtx_set_esd_state( UART_ESD_NULL );
              }
              else
              {
                /*
                 * escape characters are not removeable
                 * so we will do this later
                 */
#ifdef _SIMULATION_
                TRACE_EVENT("ESD: escape characters are not removeable");
#endif /* _SIMULATION_ */
                dtx_set_esd_state( UART_ESD_DETECTED );
              }
#ifdef _SIMULATION_
              TRACE_EVENT_P3("+ + + dlc_instance: %d, silence %d \
                             (from %d) Escape Sequence Detected + + + ",
                             uart_data->dtx->dlc_instance,
                             elapsed,
                             uart_data->dtx->esd_guard_time);
#endif /* _SIMULATION_ */

              /*
               * send detected escape sequence to MMI
               */
              sig_dtx_ker_escape_detected_ind(uart_data->dtx->dlc_instance);
            }
            break;

          default:
            {
              TRACE_ERROR_P1("Error: wrong ESD state, uart_dtxs.c(%d)", __LINE__);
            }
            break;
        }
      }
      else
      {
          TRACE_ERROR_P1("VSI entity: Cannot restart timer, uart_dtxs.c(%d)",
                                                                   __LINE__);
      }
      break;

    case DTX_NOT_READY:
      if (vsi_t_time (VSI_CALLER &cur_time) EQ VSI_OK)
      {
        elapsed = cur_time - uart_data->dtx->esd_guard_time;
        switch (dtx_get_esd_state())
        {
          case UART_ESD_DETECTED:
          case UART_ESD_NULL:
            break;

          case UART_ESD_CHAR_1:
          case UART_ESD_CHAR_2:
            if(elapsed < uart_data->act_gp)
            {
              /*
               * escape sequence guard period not complete: start timer
               * with remaining time value
               */
              sig_dtx_rt_start_tesd_req (uart_data->act_gp - elapsed);
            }
            else
            {
              /*
               * Guard Period complete
               * reset detection because detected characters do not belong to
               * an escape sequence
               */
              dtx_set_esd_state( UART_ESD_NULL );

              /*
               * if possible insert escape characters to usual data stream
               */
              if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
              {
                /*
                 * make escape character valid to send and insert values in
                 * primitive
                 */
                uart_data->dtx->to_send_data->len    = uart_data->dtx->esd_pos;
                uart_data->dtx->to_send_data->size   = uart_data->dtx->esd_pos;
                uart_data->dtx->to_send_data->offset = 0;

                if((uart_data->dtx->cur_desc_size -
                    uart_data->dtx->esd_pos) >= uart_data->n1)
                {
                  /*
                   * there is still enough space left to continue reception
                   */
                  sig_dtx_rx_ready_to_receive_req(
                    uart_data->dtx->dlc_instance,
                    uart_data->dtx->to_send_data,
                    uart_data->dtx->write_pos,
                    uart_data->dtx->cur_desc_size);
                }

              }
            }
            break;

          case UART_ESD_CHAR_3:
            if(elapsed < uart_data->act_gp)
            {
              /*
               * escape sequence guard period not complete: start timer
               * with remaining time value
               */
              sig_dtx_rt_start_tesd_req (uart_data->act_gp - elapsed);
            }
            else
            {
              /*
               * Guard Period complete
               * Escape Sequence detected
               * store the occurence of the Escape Sequence
               */
#ifdef _SIMULATION_
              TRACE_EVENT_P3("+ + + dlc_instance: %d, silence %d \
                             (from %d) Escape Sequence Detected + + + ",
                             uart_data->dtx->dlc_instance,
                             elapsed,
                             uart_data->dtx->esd_guard_time);
#endif /* _SIMULATION_ */
              /*
               * remove escape characters from data stream
               */
              if(uart_data->dtx->receiving_state EQ UART_DTX_NOT_RECEIVING)
              {
                if(uart_data->dtx->write_pos > uart_data->dtx->esd_pos)
                {
                  memmove(
                    &uart_data->dtx->to_send_data->buffer[
                      uart_data->dtx->esd_pos - 3],
                    &uart_data->dtx->to_send_data->buffer[
                      uart_data->dtx->esd_pos],
                    uart_data->dtx->write_pos - uart_data->dtx->esd_pos);
                }
                uart_data->dtx->write_pos-= 3;
                uart_data->dtx->esd_pos  -= 3;
                /*
                 * Reset the state of the Escape Sequence Detection
                 */
                dtx_set_esd_state( UART_ESD_NULL );

                if((uart_data->dtx->cur_desc_size -
                    uart_data->dtx->esd_pos) >= uart_data->n1)
                {
                  /*
                   * there is still enough space left to continue reception
                   */
                  sig_dtx_rx_ready_to_receive_req(
                    uart_data->dtx->dlc_instance,
                    uart_data->dtx->to_send_data,
                    uart_data->dtx->write_pos,
                    uart_data->dtx->cur_desc_size);
                }
              }
              else
              {
                /*
                 * escape characters are not removeable
                 * so we will do this later
                 */
#ifdef _SIMULATION_
                TRACE_EVENT("ESD: escape characters are not removeable");
#endif /* _SIMULATION_ */
                dtx_set_esd_state( UART_ESD_DETECTED );
              }

              /*
               * send detected escape sequence to MMI
               */
              sig_dtx_ker_escape_detected_ind(uart_data->dtx->dlc_instance);
            }
            break;

          default:
            TRACE_ERROR("wrong esd state");
            break;
        }
      }
      break;

    case DTX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DTX_TIMEOUT_TESD_REQ unexpected" );
      break;
  }
} /* sig_ker_dtx_timeout_tesd_req() */

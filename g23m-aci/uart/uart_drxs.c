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
|             described in the SDL-documentation (DRX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXS_C
#define UART_DRXS_C
#endif /* !UART_DRXS_C */

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

#include "uart_drxf.h"  /* to get the DRX function declarations */
#include "uart_kers.h"  /* to get the KER signal declarations */
#ifdef FF_MULTI_PORT
#include "uart_ptxs.h"  /* to get signal definitions for service TX */
#else /* FF_MULTI_PORT */
#include "uart_txs.h"   /* to get signal definitions for service TX */
#endif /* FF_MULTI_PORT */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_ready_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_READY_MODE_REQ
|
| Parameters  : dlc_instance - dlc instance wich belongs to this DRX instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_ready_mode_req (UBYTE dlc_instance)
{
  TRACE_ISIG( "sig_ker_drx_ready_mode_req" );

  uart_data->drx->dlc_instance = dlc_instance;

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_DEAD:
      /*
       * new DLC starts with enabled data flow
       */
      uart_data->drx->data_flow = UART_FLOW_ENABLED;

#ifdef DTILIB
      if((uart_data->drx->dti_drx_state NEQ DTI_CLOSED ) &&
         (uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING))
      {
        /*
         * signal availability to higher layer if currently not sending
         */
        SET_STATE( UART_SERVICE_DRX, DRX_READY );
#ifdef FLOW_TRACE
        sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
        dti_start(
          uart_hDTI, 
          uart_data->device, 
          UART_DTI_UP_INTERFACE, 
          uart_data->drx->dlc_instance);
      }
#else  /* DTILIB */
      if((uart_data->drx->hComm_DRX_UPLINK NEQ VSI_ERROR ) &&
         (uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING))
      {
        /*
         * signal availability to higher layer if currently not sending
         */
        PALLOC (dti_ready_ind, DTI_READY_IND);
        SET_STATE( UART_SERVICE_DRX, DRX_READY );
        dti_ready_ind->tui  = uart_data->tui_uart;
        dti_ready_ind->c_id = drx_get_channel_id();
#ifdef _SIMULATION_
        dti_ready_ind->op_ack = 0;
#endif /* _SIMULATION_ */
#ifdef FLOW_TRACE
        sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
        PSEND (uart_data->drx->hComm_DRX_UPLINK, dti_ready_ind);
      }
#endif /* DTILIB */
      else
      {
        /*
         * no peer yet, just switch to NOT READY and wait
         * for the signal SIG_KER_DRX_SET_DTI_PEER_REQ
         */
#ifdef DTILIB
        if(uart_data->drx->dti_drx_state NEQ DTI_CLOSED )
          dti_stop(
            uart_hDTI, 
            uart_data->device, 
            UART_DTI_UP_INTERFACE, 
            uart_data->drx->dlc_instance
            );
#endif /* DTILIB */
        SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
      }
      break;

    case DRX_READY:
    case DRX_NOT_READY:
    case DRX_FLUSHING:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_READY_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_ready_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_dead_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_DEAD_MODE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_dead_mode_req () 
{ 
  TRACE_ISIG( "sig_ker_drx_dead_mode_req" );
  
  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
      SET_STATE( UART_SERVICE_DRX, DRX_DEAD );
      /*
       * free all resources
       */
#ifdef DTILIB
      uart_data->drx->dti_drx_state = DTI_CLOSED;
#else  /* DTILIB */
      uart_data->drx->hComm_DRX_UPLINK = VSI_ERROR;
#endif /* DTILIB */
      drx_free_resources();
      break;

    case DRX_FLUSHING:
      sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
      /* fall through */
    case DRX_NOT_READY:
      SET_STATE( UART_SERVICE_DRX, DRX_DEAD );

#ifdef DTILIB
      uart_data->drx->dti_drx_state = DTI_CLOSED;
#else  /* DTILIB */
      uart_data->drx->hComm_DRX_UPLINK = VSI_ERROR;
#endif /* DTILIB */
      if(uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING)
      {
        /*
         * signal that there is not any more data available
         */
        sig_drx_tx_data_not_available_ind(uart_data->drx->dlc_instance);
        /*
         * free all resources:
         */
        drx_free_resources();
      }
      else
        uart_data->drx->sending_state = UART_DRX_INVALID;
      break;

    case DRX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_DEAD_MODE_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_dead_mode_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_enable_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_ENABLE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_enable_req () 
{
  TRACE_ISIG( "sig_ker_drx_enable_req" );

  uart_data->drx->data_flow = UART_FLOW_ENABLED;

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
      break;

    case DRX_FLUSHING:
    case DRX_NOT_READY:
      /*
       * if DRX is not already sending and there is some data to be sent out,
       * notify service TX that there is data available
       */
      if(uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING)
      {
        if(uart_data->drx->received_data)
          sig_drx_tx_data_available_ind( uart_data->drx->dlc_instance,
                                         uart_data->drx->received_data, 
                                         uart_data->drx->read_pos);
        else
#ifdef DTILIB
          if( uart_data->drx->dti_drx_state NEQ DTI_CLOSED )
          {
            SET_STATE( UART_SERVICE_DRX, DRX_READY );
#ifdef FLOW_TRACE
            sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
            dti_start(
              uart_hDTI, 
              uart_data->device, 
              UART_DTI_UP_INTERFACE, 
              uart_data->drx->dlc_instance
              );
          }
#else  /* DTILIB */
          if(uart_data->drx->hComm_DRX_UPLINK NEQ VSI_ERROR)
        {
          /*
           * nothing more to send,
           * signal to higher layer that we are able to receive more data
           */
          PALLOC (dti_ready_ind, DTI_READY_IND);
          SET_STATE( UART_SERVICE_DRX, DRX_READY );
          dti_ready_ind->tui  = uart_data->tui_uart;
          dti_ready_ind->c_id = drx_get_channel_id();
#ifdef _SIMULATION_
          dti_ready_ind->op_ack = 0;
#endif /* _SIMULATION_ */
#ifdef FLOW_TRACE
          sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
          PSEND (uart_data->drx->hComm_DRX_UPLINK, dti_ready_ind);
        }
#endif /* DTILIB */
      }
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_ENABLE_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_enable_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_disable_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_DISABLE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_disable_req () 
{
  TRACE_ISIG( "sig_ker_drx_disable_req" );

  uart_data->drx->data_flow = UART_FLOW_DISABLED;

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
      break;

    case DRX_FLUSHING:
    case DRX_NOT_READY:
      /*
       * stop sending if possible
       */
      if(uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING)
        sig_drx_tx_data_not_available_ind(uart_data->drx->dlc_instance);
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_DISABLE_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_disable_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_tx_drx_sending_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_TX_DRX_SENDING_REQ
|
| Parameters  : dummy - description of parameter dummy
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_tx_drx_sending_req () 
{ 
  TRACE_ISIG( "sig_tx_drx_sending_req" );
  
  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_FLUSHING:
    case DRX_NOT_READY:
      uart_data->drx->sending_state = UART_DRX_SENDING;
      break;

    default:
      TRACE_ERROR( "SIG_TX_DRX_SENDING_REQ unexpected" );
      break;
  }
} /* sig_tx_drx_sending_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_tx_drx_data_sent_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_TX_DRX_DATA_SENT_REQ
|
| Parameters  : rest_data - generic data descriptor of "still to send" data
|                     pos - current position in first rest data buffer
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_tx_drx_data_sent_req ( T_desc2 *rest_data, USHORT pos ) 
{
  T_desc2 *next_desc;
  T_desc2 *desc;

  TRACE_ISIG( "sig_tx_drx_data_sent_req" );

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_DEAD:
      uart_data->drx->sending_state = UART_DRX_NOT_SENDING;
      /*
       * Since the service is dead, free all resources and the rest of data
       */
      drx_free_resources();
      break;

    case DRX_NOT_READY:
      if(uart_data->drx->sending_state EQ UART_DRX_INVALID)
      {
        /*
         * because we are in an invalid sending state, free all resources
         */
        drx_free_resources();
      }
      else
      {
        /*
         * free all data descriptors in front of the remaining data
         */
        desc = uart_data->drx->received_data;
        while((desc NEQ rest_data) && (desc NEQ NULL))
        {
          next_desc = (T_desc2 *)desc->next;           
          MFREE (desc);                               
          desc = next_desc;                            
        }
        /*
         * set received_data descriptor to remaining data, remember position
         */
        uart_data->drx->received_data = desc;
        uart_data->drx->read_pos      = pos;
      }

      uart_data->drx->sending_state = UART_DRX_NOT_SENDING;

      if(uart_data->drx->data_flow EQ UART_FLOW_ENABLED)
      {
        if(uart_data->drx->received_data)
        {
          /*
           * data flow is enabled and there is more data to send
           */
          sig_drx_tx_data_available_ind(uart_data->drx->dlc_instance,
                                        uart_data->drx->received_data, 
                                        uart_data->drx->read_pos);
        }
        else 
          /*
           * data flow is enabled but there is nothing more to send,
           * so do positive flow control
           */
#ifdef DTILIB
          if( uart_data->drx->dti_drx_state NEQ DTI_CLOSED )
          {
            SET_STATE( UART_SERVICE_DRX, DRX_READY );
#ifdef FLOW_TRACE
            sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
            dti_start(
              uart_hDTI, 
              uart_data->device, 
              UART_DTI_UP_INTERFACE, 
              uart_data->drx->dlc_instance
              );
          }
#else  /* DTILIB */
          if(uart_data->drx->hComm_DRX_UPLINK NEQ VSI_ERROR)
        {
          PALLOC (dti_ready_ind, DTI_READY_IND);
          SET_STATE( UART_SERVICE_DRX, DRX_READY );
          dti_ready_ind->tui  = uart_data->tui_uart;
          dti_ready_ind->c_id = drx_get_channel_id();
#ifdef _SIMULATION_
          dti_ready_ind->op_ack = 0;
#endif /* _SIMULATION_ */
#ifdef FLOW_TRACE
          sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
          PSEND (uart_data->drx->hComm_DRX_UPLINK, dti_ready_ind);
        }
#endif /* DTILIB */
      }
      break;

    case DRX_FLUSHING:
      /*
       * free all data descriptors in front of the remaining data
       */
      desc = uart_data->drx->received_data;
      while((desc NEQ rest_data) && (desc NEQ NULL))
      {
        next_desc = (T_desc2 *)desc->next;           
        MFREE (desc);                               
        desc = next_desc;                            
      }
      /*
       * set received_data descriptor to remaining data, remember position
       */
      uart_data->drx->received_data = desc;
      uart_data->drx->read_pos      = pos;

      uart_data->drx->sending_state = UART_DRX_NOT_SENDING;

      if(uart_data->drx->received_data)
      {
        if(uart_data->drx->data_flow EQ UART_FLOW_ENABLED)
          /*
           * data flow is enabled and there is more data to send
           */
          sig_drx_tx_data_available_ind(uart_data->drx->dlc_instance,
                                        uart_data->drx->received_data, 
                                        uart_data->drx->read_pos);
      }
      else 
#ifdef DTILIB
          if(( uart_data->drx->dti_drx_state NEQ DTI_CLOSED ) &&
              (uart_data->drx->data_flow EQ UART_FLOW_ENABLED))
          {
            SET_STATE( UART_SERVICE_DRX, DRX_READY );
            sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
#ifdef FLOW_TRACE
            sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
            dti_start(
              uart_hDTI, 
              uart_data->device, 
              UART_DTI_UP_INTERFACE, 
              uart_data->drx->dlc_instance
              );
          }
#else  /* DTILIB */
        if((uart_data->drx->hComm_DRX_UPLINK NEQ VSI_ERROR) &&
              (uart_data->drx->data_flow EQ UART_FLOW_ENABLED))
      {
        /*
         * data flow is enabled but there is nothing more to send,
         * so send flush signal and positive flow control
         */
        PALLOC (dti_ready_ind, DTI_READY_IND);
        SET_STATE( UART_SERVICE_DRX, DRX_READY );
        sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
        dti_ready_ind->tui  = uart_data->tui_uart;
        dti_ready_ind->c_id = drx_get_channel_id();
#ifdef _SIMULATION_
        dti_ready_ind->op_ack = 0;
#endif /* _SIMULATION_ */
#ifdef FLOW_TRACE
        sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
        PSEND (uart_data->drx->hComm_DRX_UPLINK, dti_ready_ind);
      }
#endif /* DTILIB */
      else
      {
        /*
         * data flushed
         */
        SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
#ifdef DTILIB
        dti_stop(
          uart_hDTI, 
          uart_data->device, 
          UART_DTI_UP_INTERFACE, 
          uart_data->drx->dlc_instance
          );
#endif /* DTILIB */
        sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
      }
      break;

    default:
      TRACE_ERROR( "SIG_TX_DRX_DATA_SENT_REQ unexpected" );
      break;
  }
} /* sig_tx_drx_data_sent_req() */



#ifdef DTILIB

/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_disconnected_mode_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_DISCONNECTED_MODE_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_disconnected_mode_req () 
{ 
  TRACE_ISIG( "sig_ker_drx_disconnected_mode_req" );
  
  uart_data->drx->dti_drx_state = DTI_CLOSED;

  switch(GET_STATE( UART_SERVICE_DRX) )
  {
    case DRX_READY:
      SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
      break;
    case DRX_NOT_READY:
      break;
    default:
      TRACE_ERROR( "SIG_KER_DRX_DISCONNECTED_MODE_REQ unexpected" );
      break;
  }

} /* sig_ker_drx_disconnected_mode_req() */
    
  

/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_set_dtilib_peer_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_SET_DTI_PEER_REQ
|               which is used to inform the service DRX that from now on it
|               needs to communicate with a (new) peer
|
| Parameters  : -
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_set_dtilib_peer_req ()
{
  TRACE_ISIG( "sig_ker_drx_set_dtilib_peer_req" );

  uart_data->drx->dti_drx_state = DTI_IDLE;

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
    case DRX_NOT_READY:
      /*
       * signal availability to higher layer if currently not sending
       */
      if((uart_data->drx->received_data EQ NULL)          &&
         (uart_data->drx->data_flow EQ UART_FLOW_ENABLED) &&
         (uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING))
      {
        SET_STATE( UART_SERVICE_DRX, DRX_READY );
#ifdef FLOW_TRACE
        sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
        dti_start(
          uart_hDTI,
          uart_data->device,
          UART_DTI_UP_INTERFACE,
          uart_data->drx->dlc_instance
          );
      }
      else
      {
        SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
        dti_stop(
          uart_hDTI, 
          uart_data->device, 
          UART_DTI_UP_INTERFACE,
          uart_data->drx->dlc_instance
          );
      }
      break;

    case DRX_FLUSHING:
    case DRX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_SET_DTI_PEER_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_set_dtilib_peer_req() */


#else  /* DTILIB */

/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_set_dti_peer_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_SET_DTI_PEER_REQ
|               which is used to inform the service DRX that from now on it
|               needs to communicate with a (new) peer
|
| Parameters  : tui_peer    - transmission unit identifier of peer
|               peer_handle - VSI handle of peer (channel has to be opened)
|               c_id        - channel identifier of peer
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_set_dti_peer_req (USHORT tui_peer, 
                                          T_HANDLE peer_handle, 
                                          UBYTE c_id) 
{
  USHORT i,pos;

  TRACE_ISIG( "sig_ker_drx_set_dti_peer_req" );

  /*
   * set new vsi handle and
   */
  uart_data->drx->hComm_DRX_UPLINK = peer_handle;

  /* 
   * search position in table
   */
  pos = UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS;
  for(i = 0; i < pos; i++)
  {
    if((uart_cid_table[i].drx EQ uart_data->drx) ||
       (uart_cid_table[i].c_id EQ c_id))
    {
      pos = i;
    }
  }

  if(pos < (UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS))
  {
    /*
     * valid position in table, update entry
     */
    uart_cid_table[pos].c_id      = c_id;
    uart_cid_table[pos].drx       = uart_data->drx;
    uart_cid_table[pos].uart_data = uart_data;
  }
  else
  {
    /*
     * create new entry in table
     */
    for(i = 0; i < UART_INSTANCES * UART_MAX_NUMBER_OF_CHANNELS; i++)
    {
      if(uart_cid_table[i].dtx EQ NULL)
      {
        /*
         * free space found, insert data
         */ 
        uart_cid_table[i].c_id      = c_id;
        uart_cid_table[i].drx       = uart_data->drx;
        uart_cid_table[i].uart_data = uart_data;
        break;
      }
    }
  }

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
    case DRX_NOT_READY:
      /*
       * signal availability to higher layer if currently not sending
       */
      if((uart_data->drx->received_data EQ NULL)          &&
         (uart_data->drx->data_flow EQ UART_FLOW_ENABLED) &&
         (uart_data->drx->sending_state EQ UART_DRX_NOT_SENDING))
      {
        PALLOC (dti_ready_ind, DTI_READY_IND);
        SET_STATE( UART_SERVICE_DRX, DRX_READY );
        dti_ready_ind->tui  = uart_data->tui_uart;
        dti_ready_ind->c_id = drx_get_channel_id();
#ifdef _SIMULATION_
        dti_ready_ind->op_ack = 0;
#endif /* _SIMULATION_ */
#ifdef FLOW_TRACE
        sndcp_trace_flow_control(FLOW_TRACE_UART, FLOW_TRACE_DOWN, FLOW_TRACE_TOP, TRUE);
#endif /* FLOW_TRACE */
        PSEND (uart_data->drx->hComm_DRX_UPLINK, dti_ready_ind);
      }
      else
      {
        SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
      }
      break;

    case DRX_FLUSHING:
    case DRX_DEAD:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_SET_DTI_PEER_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_set_dti_peer_req() */

#endif /* DTILIB */


/*
+------------------------------------------------------------------------------
| Function    : sig_ker_drx_flush_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_DRX_FLUSH_REQ which is used
|               to trigger flushing of the service.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_drx_flush_req () 
{
  TRACE_ISIG( "sig_ker_drx_flush_req" );

  switch( GET_STATE( UART_SERVICE_DRX ) )
  {
    case DRX_READY:
      sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
      break;

    case DRX_NOT_READY:
      if((uart_data->drx->received_data) &&
         (uart_data->drx->sending_state NEQ UART_DRX_INVALID))
      {
        SET_STATE( UART_SERVICE_DRX, DRX_FLUSHING );
      }
      else
      {
        sig_any_ker_flushed_ind(uart_data->drx->dlc_instance);
      }
      break;

    case DRX_FLUSHING:
      break;

    default:
      TRACE_ERROR( "SIG_KER_DRX_FLUSH_REQ unexpected" );
      break;
  }
} /* sig_ker_drx_flush_req() */

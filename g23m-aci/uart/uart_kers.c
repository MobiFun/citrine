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
|             described in the SDL-documentation (KER-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_KERS_C
#define UART_KERS_C
#endif /* !UART_KERS_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

/*
 * Turn off spurious LINT warnings
 */
 /*lint -e415 access of out-of-bounds pointer */

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

#ifdef FF_MULTI_PORT
#include "uart_ptxs.h"  /* to get signal definitions for service TX */
#include "uart_prxs.h"  /* to get rx signals */
#else /* FF_MULTI_PORT */
#include "uart_txs.h"   /* to get signal definitions of service TX */
#include "uart_rxs.h"   /* to get signal definitions of service RX */
#endif /* FF_MULTI_PORT */
#include "uart_dtxs.h"  /* to get signal definitions of service DTX */
#include "uart_drxs.h"  /* to get signal definitions of service DRX */
#include "uart_kerf.h"  /* to get function definitions of service KER */
#include "uart_rts.h"   /* to get signal definitions of service RT */

#ifndef _TARGET_
#include <stdio.h>      /* to get sprintf */
#endif /* !_TARGET_ */
#include <string.h>      /* JK, delete warnings: to get memcpy */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_drx_ker_line_states_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DRX_KER_LINE_STATES_IND
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DRX
|               st_flow      - flow control state (X bit)
|               st_line_sa   - line state SA
|               st_line_sa   - line state SB
|               st_break_len - break state
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_drx_ker_line_states_ind(UBYTE  dlc_instance,
                                        UBYTE  st_flow,
                                        UBYTE  st_line_sa,
                                        UBYTE  st_line_sb,
                                        USHORT st_break_len)
{
  T_DLC*  dlc;
  ULONG   old_lines;
  ULONG   new_lines;

  TRACE_ISIG( "sig_drx_ker_line_states_ind" );

  /*
   * set DLC
   */
  dlc = &uart_data->dlc_table[dlc_instance];

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
      /*
       * check for changed line states
       */
      old_lines = dlc->lines & (UART_SA_TX_MASK |
                                UART_SB_TX_MASK |
                                UART_X_TX_MASK  |
                                UART_BRK_TX_MASK);
      new_lines = ((ULONG)st_line_sa << UART_SA_TX_POS) |
                   ((ULONG)st_line_sb << UART_SB_TX_POS) |
                   ((ULONG)st_flow    << UART_X_TX_POS);
      if(st_break_len NEQ DTI_BREAK_OFF)
      {
        TRACE_EVENT("send Break");
        new_lines|= UART_BRK_TX_MASK;
      }

      if(old_lines NEQ new_lines)
      {
        /*
         * trace changes
         */
        if(new_lines & UART_X_TX_MASK)
        {
          if(!(old_lines & UART_X_TX_MASK))
          {
            TRACE_EVENT("TX Flow Control: stop");
          }
        }
        else if(old_lines & UART_X_TX_MASK)
        {
          TRACE_EVENT("TX Flow Control: start");
        }

        if(new_lines & UART_SA_TX_MASK)
        {
          if(!(old_lines & UART_SA_TX_MASK))
          {
            TRACE_EVENT("DSR: off");
          }
        }
        else if(old_lines & UART_SA_TX_MASK)
        {
          TRACE_EVENT("DSR: on");
        }

        if(new_lines & UART_SB_TX_MASK)
        {
          if(!(old_lines & UART_SB_TX_MASK))
          {
            TRACE_EVENT("DCD: off");
          }
        }
        else if(old_lines & UART_SB_TX_MASK)
        {
          TRACE_EVENT("DCD: on");
        }

        /*
         * send new line states
         * but flush UART before
         */
        dlc->lines&= ~(UART_SA_TX_MASK  |
                       UART_SB_TX_MASK  |
                       UART_X_TX_MASK   |
                       UART_BRK_TX_MASK |
                       UART_BRKLEN_TX_MASK);

        if(st_break_len NEQ DTI_BREAK_OFF)
          new_lines|= ((ULONG)st_break_len << UART_BRKLEN_TX_POS);

        dlc->lines|= new_lines;

        dlc->received_prim|= UART_DTI_DATA_REQ_MASK;

        if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
        {
          uart_data->ker.flush_state = UART_KER_TX_FLUSH;
          sig_ker_tx_flush_req();
        }
      }
      break;

    case KER_MUX:
      /*
       * check for changed line states
       */
      old_lines = dlc->lines & (UART_CTS_MASK |
                                UART_DSR_MASK |
                                UART_DCD_MASK |
                                UART_BRK_TX_MASK);
      new_lines = ((ULONG)st_flow << UART_CTS_POS)    |
                  ((ULONG)st_line_sa << UART_DSR_POS) |
                  ((ULONG)st_line_sb << UART_DCD_POS);

      if(st_break_len NEQ DTI_BREAK_OFF)
      {
        TRACE_EVENT_P1("send Break - DLCI=%d", dlc->dlci);
        new_lines|= UART_BRK_TX_MASK;
      }

      if(old_lines NEQ new_lines)
      {
        /*
         * trace changes
         */
        if(new_lines & UART_CTS_MASK)
        {
          if(!(old_lines & UART_CTS_MASK))
          {
            TRACE_EVENT_P1("TX Flow Control: stop - DLCI=%d", dlc->dlci);
          }
        }
        else if(old_lines & UART_CTS_MASK)
        {
          TRACE_EVENT_P1("TX Flow Control: start - DLCI=%d", dlc->dlci);
        }

        if(new_lines & UART_DSR_MASK)
        {
          if(!(old_lines & UART_DSR_MASK))
          {
            TRACE_EVENT_P1("DSR: off - DLCI=%d", dlc->dlci);
          }
        }
        else if(old_lines & UART_DSR_MASK)
        {
          TRACE_EVENT_P1("DSR: on - DLCI=%d", dlc->dlci);
        }

        if(new_lines & UART_DCD_MASK)
        {
          if(!(old_lines & UART_DCD_MASK))
          {
            TRACE_EVENT_P1("DCD: off - DLCI=%d", dlc->dlci);
          }
        }
        else if(old_lines & UART_DCD_MASK)
        {
          TRACE_EVENT_P1("DCD: on - DLCI=%d", dlc->dlci);
        }
        /*
         * build and send MSC command
         */
        dlc->lines&= ~(UART_CTS_MASK    |
                       UART_DSR_MASK    |
                       UART_DCD_MASK    |
                       UART_BRK_TX_MASK |
                       UART_BRKLEN_TX_MASK);

        if(st_break_len NEQ DTI_BREAK_OFF)
          new_lines|= ((ULONG)st_break_len << UART_BRKLEN_TX_POS);

        dlc->lines|= new_lines;
        ker_mux_send_line_states(dlc_instance);
      }
      break;

    default:
      TRACE_ERROR( "SIG_DRX_KER_LINE_STATES_IND unexpected" );
      break;
  }
} /* sig_drx_ker_line_states_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_ker_enable_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_KER_ENABLE_IND
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DTX
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_ker_enable_ind (UBYTE dlc_instance)
{
  TRACE_ISIG( "sig_dtx_ker_enable_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX:
      /*
       * build and send MSC command
       * enable flow control in line states
       */
      uart_data->dlc_table[dlc_instance].lines&= ~UART_FC_TX_MASK;
      /*
       * send new line states
       */
      ker_mux_send_line_states(dlc_instance);
      break;

    case KER_READY:
      break;

    default:
      TRACE_ERROR( "SIG_DTX_KER_ENABLE_IND unexpected" );
      break;
  }
} /* sig_dtx_ker_enable_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_ker_disable_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_KER_DISABLE_IND
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DTX
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_ker_disable_ind (UBYTE dlc_instance)
{
  TRACE_ISIG( "sig_dtx_ker_disable_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX:
      /*
       * build and send MSC command
       * disable flow control in line states
       */
      uart_data->dlc_table[dlc_instance].lines|= UART_FC_TX_MASK;
      /*
       * send new line states
       */
      ker_mux_send_line_states(dlc_instance);
      break;

    case KER_READY:
      break;

    default:
      TRACE_ERROR( "SIG_DTX_KER_DISABLE_IND unexpected" );
      break;
  }
} /* sig_dtx_ker_disable_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_ker_escape_detected_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_KER_ESCAPE_DETECTED_IND
|
| Parameters  : dlc_instance - dlc instance wich belongs to calling DTX
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_ker_escape_detected_ind (U8 dlc_instance)
{
  T_DLC *dlc;

  TRACE_FUNCTION( "sig_dtx_ker_escape_detected_ind" );

  /*
   * set DLC
   */
  dlc = &uart_data->dlc_table[dlc_instance];
  {
    PALLOC (uart_detected_ind, UART_DETECTED_IND);
    uart_detected_ind->device = uart_data->device;
    uart_detected_ind->dlci   = dlc->dlci;
    uart_detected_ind->cause  = UART_DETECT_ESC;
    PSEND (hCommMMI, uart_detected_ind);
  }
} /* sig_dtx_ker_escape_detected_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_any_ker_flushed_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_ANY_KER_FLUSHED_IND
|
| Parameters  : dlc_instance - dlc instance wich belongs to this signal
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_any_ker_flushed_ind (UBYTE dlc_instance)
{
  T_DLC*  dlc;
  UBYTE   i;

  TRACE_ISIG( "sig_any_ker_flushed_ind" );

  /*
   * reset flush indicator
   */
  dlc          = &uart_data->dlc_table[dlc_instance];
  dlc->flushed = TRUE;
  /*
   * if the primitive which has triggered the flush belongs to the whole port
   * we will check each DLC for flush, if it only belongs to this DLC we start
   * TX flushing immediately
   */
  if(uart_data->ker.received_prim)
  {
    /*
     * check each DLC
     */
    for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
    {
      dlc = &uart_data->dlc_table[i];
      if(dlc->flushed NEQ TRUE)
        return;
    }
  }
  /*
   * start TX flushing
   */
  uart_data->ker.flush_state = UART_KER_TX_FLUSH;
  sig_ker_tx_flush_req();
} /* sig_any_ker_flushed_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_tx_ker_flushed_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_TX_KER_FLUSHED_IND
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_tx_ker_flushed_ind ()
{
  T_DLC*  dlc;
  UBYTE   i;
#ifndef  _SIMULATION_
  USHORT  error_code;
#endif
  TRACE_ISIG( "sig_tx_ker_flushed_ind" );

  if(uart_data->ker.flush_state EQ UART_KER_TX_FLUSH)
  {
    /*
     * primitives of the port
     */
    /*
     * UART_PARAMETERS_REQ
     */
    if(uart_data->ker.received_prim & UART_PARAMETERS_REQ_MASK)
    {
      /*
       * disable UART while set new communication parameters
       */
#ifdef _SIMULATION_
      {
        /*
         * send DTI_GETDATA_REQ
         */
        PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
        dti_getdata_req->link_id = LINK_DISABLE_PORT_1; /* for disable */
        PSEND (hCommMMI, dti_getdata_req);
      }
#else /* _SIMULATION_ */
#ifndef FF_MULTI_PORT
      if((error_code = UF_Enable (uart_data->device, FALSE)) NEQ UF_OK)
      {
          TRACE_ERROR_P2("UF Driver: Can't disable UART, [%d], uart_kerp.c(%d)",
                                                           error_code,__LINE__);
      }
#endif /* !FF_MULTI_PORT */
#endif /* _SIMULATION */
      /*
       * set new parameters
       */
      ker_setupUart();
#ifdef _SIMULATION_
      {
        /*
         * send DTI_GETDATA_REQ
         */
        PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
        dti_getdata_req->link_id = LINK_ENABLE_PORT_1; /* for enable */
        PSEND (hCommMMI, dti_getdata_req);
      }
#else /* _SIMULATION_ */
#ifndef FF_MULTI_PORT
      if((error_code = UF_Enable (uart_data->device, TRUE)) NEQ UF_OK)
      {
          TRACE_ERROR_P2("UF Driver: Can't enable UART, [%d], uart_kerp.c(%d)",
                                                          error_code,__LINE__);
      }
#endif /* !FF_MULTI_PORT */
#endif /* _SIMULATION */
      /*
       * restart suspended read and write procedures
       */
      sig_ker_tx_restart_write_req();
      sig_ker_rx_restart_read_req();
      /*
       * send confirm primitive
       */
      {
        PALLOC (uart_parameters_cnf, UART_PARAMETERS_CNF);
        uart_parameters_cnf->device = uart_data->device;
        PSEND (hCommMMI, uart_parameters_cnf);
      }
    }

    /*
     * UART_DISABLE_REQ
     */
    if(uart_data->ker.received_prim & UART_DISABLE_REQ_MASK)
    {
      switch( GET_STATE( UART_SERVICE_KER ) )
      {
        case KER_MUX:
        case KER_MUX_ESTABLISH:
        case KER_MUX_DLC_CLOSING:
        case KER_MUX_CLOSING:
        case KER_MUX_CLOSED:
          /*
           * stop timers and
           * remove all DLC instances
           */
          ker_mux_close_down();
          break;

        case KER_READY:
          /*
           * close dlc channel
           */
          ker_mux_dlc_release(UART_CONTROL_INSTANCE);
          break;

        case KER_DEAD:
          break;

        default:
          TRACE_ERROR( "SIG_TX_KER_FLUSHED_IND unexpected" );
          break;
      }

      SET_STATE( UART_SERVICE_KER, KER_DEAD );

      /*
       * disable UART
       */
#ifdef _SIMULATION_
      {
        /*
         * send DTI_GETDATA_REQ
         */
        PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
        dti_getdata_req->link_id = LINK_DISABLE_PORT_1; /* for disable */
        PSEND (hCommMMI, dti_getdata_req);
      }
#else /* _SIMULATION_ */
#ifndef FF_MULTI_PORT
      if((error_code = UF_Enable (uart_data->device, FALSE)) NEQ UF_OK)
      {
          TRACE_ERROR_P2("UF Driver: Can't disable UART, [%d], uart_kerp.c(%d)",
                                                           error_code,__LINE__);
      }
#endif /* !FF_MULTI_PORT */
#endif /* _SIMULATION */
      sig_ker_rx_dead_mode_req();
      sig_ker_tx_dead_mode_req();
      /*
       * send confirm primitive
       */
      {
        PALLOC (uart_disable_cnf, UART_DISABLE_CNF);
        uart_disable_cnf->device = uart_data->device;
        PSEND (hCommMMI, uart_disable_cnf);
      }
    }

    /*
     * UART_MUX_START_REQ
     */
    if(uart_data->ker.received_prim & UART_MUX_START_REQ_MASK)
    {
      switch( GET_STATE( UART_SERVICE_KER ) )
      {
        case KER_READY:
          /*
           * close dlc channel
           */
          ker_mux_dlc_release(UART_CONTROL_INSTANCE);
          break;

        default:
          TRACE_ERROR( "SIG_TX_KER_FLUSHED_IND unexpected" );
          break;
      }

      SET_STATE( UART_SERVICE_KER, KER_MUX_ESTABLISH );

      /*
       * set RX and TX service in mux mode
       */
      sig_ker_rx_mux_mode_req();
      sig_ker_tx_mux_mode_req();
      /*
       * start reception
       */
      MALLOC(uart_data->ker.rx_data_desc, (USHORT)(sizeof(T_desc2) -
                                           1 +
                                           uart_data->n1 +
                                           2));
      uart_data->ker.rx_data_desc->next = (ULONG)NULL;
      uart_data->ker.rx_data_desc->len  = 0;
      sig_ker_rx_ready_to_receive_req(uart_data->ker.rx_data_desc,
                                      0,
                                      (USHORT)(uart_data->n1 + 2));
      /*
       * start timer
       */
      sig_ker_rt_start_t3_req();

      /*
       * send confirm primitive
       */
      {
        PALLOC (uart_mux_start_cnf, UART_MUX_START_CNF);
        uart_mux_start_cnf->device = uart_data->device;
        PSEND (hCommMMI, uart_mux_start_cnf);
      }
    }

    /*
     * UART_MUX_CLOSE_REQ
     */
    if(uart_data->ker.received_prim & UART_MUX_CLOSE_REQ_MASK)
    {
      switch( GET_STATE( UART_SERVICE_KER ) )
      {
        case KER_MUX_CLOSED:
          if(uart_data->ker.receiving_state NEQ UART_KER_RECEIVING)
          {
            PALLOC(uart_mux_close_ind, UART_MUX_CLOSE_IND);
            SET_STATE( UART_SERVICE_KER, KER_READY );
            /*
             * stop receiving
             */
            sig_ker_rx_not_ready_to_receive_req();
            MFREE_DESC2(uart_data->ker.rx_data_desc);
            uart_data->ker.rx_data_desc = NULL;
            /*
             * set dlc values
             */
            dlc           = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
            dlc->drx      = &uart_data->drx_base[0];
            dlc->dtx      = &uart_data->dtx_base[0];
            dlc->dlci     = 0;
            dlc->priority = 0;
            /*
             * set RX and TX in ready mode
             */
            sig_ker_rx_ready_mode_req();
            sig_ker_tx_ready_mode_req();
            /*
             * set frame size for ready mode
             */
            uart_data->n1 = UART_N1_READY_MODE;
            /*
             * set DRX and DTX in ready mode
             */
            uart_data->dtx = dlc->dtx;
            uart_data->drx = dlc->drx;
            sig_ker_drx_ready_mode_req(UART_CONTROL_INSTANCE);
            sig_ker_dtx_ready_mode_req(UART_CONTROL_INSTANCE);
            /*
             * inform ACI about entering ready mode
             */
            uart_mux_close_ind->device = uart_data->device;
            PSEND(hCommMMI, uart_mux_close_ind);
          }
          break;

        default:
          TRACE_ERROR( "SIG_TX_KER_FLUSHED_IND unexpected" );
          break;
      }
    }

    /*
     * no more UART port related primitives
     * so clear all flags
     */
    uart_data->ker.received_prim = 0;

    /*
     * primitives of the DLC
     */
    for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
    {
      dlc = &uart_data->dlc_table[i];
      if(dlc->received_prim)
      {
        /*
         * UART_RING_REQ
         * UART_DCD_REQ
         * DTI_DATA_REQ (line states)
         */
        if((dlc->received_prim & UART_RING_REQ_MASK)  ||
           (dlc->received_prim & UART_DCD_REQ_MASK)   ||
           (dlc->received_prim & UART_DTI_DATA_REQ_MASK))
        {
          /*
           * send new line states
           */
          switch( GET_STATE( UART_SERVICE_KER ) )
          {
            case KER_READY:
              sig_ker_tx_line_states_req(UART_CONTROL_INSTANCE);
              break;

            case KER_MUX:
              /*
               * send MSC frame to peer
               */
              ker_mux_send_line_states (uart_data->dlc_instance[dlc->dlci]);
              break;

            default:
              if(dlc->received_prim & UART_RING_REQ_MASK)
              {
                TRACE_ERROR( "UART_RING_REQ unexpected" );
              }
              if(dlc->received_prim & UART_DCD_REQ_MASK)
              {
                TRACE_ERROR( "UART_DCD_REQ unexpected" );
              }
              break;
          }
          /*
           * send confirm primitives
           */
          if(dlc->received_prim & UART_RING_REQ_MASK)
          {
            PALLOC (uart_ring_cnf, UART_RING_CNF);
            uart_ring_cnf->device = uart_data->device;
            uart_ring_cnf->dlci   = dlc->dlci;
            PSEND (hCommMMI, uart_ring_cnf);
          }
          if(dlc->received_prim & UART_DCD_REQ_MASK)
          {
            PALLOC (uart_dcd_cnf, UART_DCD_CNF);
            uart_dcd_cnf->device = uart_data->device;
            uart_dcd_cnf->dlci   = dlc->dlci;
            PSEND (hCommMMI, uart_dcd_cnf);
          }
        }
        /*
         * no more DLC related primitives
         * so clear all flags
         */
        dlc->received_prim = 0;
      }
    }
    uart_data->ker.flush_state = UART_KER_NOT_FLUSHING;
  }
} /* sig_tx_ker_flushed_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_tx_ker_sending_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_TX_KER_SENDING_IND
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_tx_ker_sending_ind ()
{
  TRACE_ISIG( "sig_tx_ker_sending_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX_ESTABLISH:
    case KER_MUX:
    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      uart_data->ker.sending_state = UART_KER_SENDING;
      break;

    default:
      TRACE_ERROR( "SIG_TX_KER_SENDING_IND unexpected" );
      break;
  }
} /* sig_tx_ker_sending_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_tx_ker_data_sent_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_TX_KER_DATA_SENT_IND
|
| Parameters  : rest_data - not yet sent data
|               write_pos - position where the not yet sent data starts
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_tx_ker_data_sent_ind (T_desc2* rest_data, USHORT read_pos)
{
  TRACE_ISIG( "sig_tx_ker_data_sent_ind" );

  /*
   * free sent data descriptor
   */
  MFREE_DESC2(uart_data->ker.tx_data_desc);
  uart_data->ker.tx_data_desc  = NULL;
  uart_data->ker.sending_state = UART_KER_NOT_SENDING;
  if(uart_data->ker.tx_data_waiting)
  {
    /*
     * send next descriptor in waiting queue
     */
    uart_data->ker.tx_data_desc    = uart_data->ker.tx_data_waiting;
    uart_data->ker.tx_data_waiting =
      (T_desc2*)uart_data->ker.tx_data_waiting->next;
    /*
     * only one descriptor is sent at a time
     */
    uart_data->ker.tx_data_desc->next = (ULONG)NULL;
    sig_ker_tx_data_available_req( uart_data->ker.tx_data_desc, 0);
  }
  else
  {
    switch( GET_STATE( UART_SERVICE_KER ) )
    {
      case KER_MUX_ESTABLISH:
      case KER_MUX:
      case KER_MUX_DLC_CLOSING:
      case KER_MUX_CLOSING:
        break;

      case KER_MUX_CLOSED:
        /*
         * flush UART before enter ready mode
         */
        uart_data->ker.received_prim|= UART_MUX_CLOSE_REQ_MASK;

        if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
        {
          uart_data->ker.flush_state = UART_KER_TX_FLUSH;
          sig_ker_tx_flush_req();
        }
        break;

      default:
        TRACE_ERROR( "SIG_TX_KER_DATA_SENT_IND unexpected" );
        break;
    }
  }
} /* sig_tx_ker_data_sent_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rx_ker_receiving_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RX_KER_RECEIVING_IND
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rx_ker_receiving_ind ()
{
  TRACE_ISIG( "sig_rx_ker_receiving_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX_ESTABLISH:
    case KER_MUX:
    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      uart_data->ker.receiving_state = UART_KER_RECEIVING;
      break;
    default:
      TRACE_ERROR( "SIG_RX_KER_RECEIVING_IND unexpected" );
      break;
  }
} /* sig_rx_ker_receiving_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rx_ker_data_received_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RX_KER_DATA_RECEIVED_IND
|               which is used to process a received frame.
|
|               Precondition is that the frame has a length >2 without flags.
|
| Parameters  : received_data - received data
|               write_pos     - write position for the next reception
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rx_ker_data_received_ind (T_desc2* received_data,
                                          USHORT write_pos)
{
  UBYTE   i;
  UBYTE   dlci;
  T_DLC*  dlc;
  ULONG   forward;
  BOOL    continuous;

  TRACE_ISIG( "sig_rx_ker_data_received_ind" );

#ifndef _TARGET_
  if(received_data->len)
  {
    USHORT  pos;
    char    buf[90];
    /*
     * trace output
     */
    TRACE_EVENT("======= IN");
    i   = 0;
    pos = 0;
    while(pos < received_data->len)
    {
      i+= sprintf(&buf[i], "0x%02x, ", received_data->buffer[pos]);
      pos++;
      if(i > 80)
      {
        TRACE_EVENT( buf );
        i = 0;
      }
      else if(pos >= received_data->len)
      {
        TRACE_EVENT( buf );
      }
    }
  }
#endif /* !_TARGET_ */

  uart_data->ker.receiving_state = UART_KER_NOT_RECEIVING;

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX_ESTABLISH:
      if(received_data->len EQ 0)
      {
        /*
         * prepare next reception
         */
        sig_ker_rx_ready_to_receive_req(received_data,
                                        write_pos,
                                        (USHORT)(uart_data->n1 + 2));
        break;
      }
      dlci = received_data->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
      if((dlci EQ UART_DLCI_CONTROL) &&
         (received_data->buffer[UART_OFFSET_CONTROL] EQ UART_SABM_FRAME))
      {
        /*
         * analyze SABM frame
         */
        forward = 0;
        ker_receive_sabm_frame(&forward, received_data);
        /*
         * set dlc value
         */
        dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
        if((forward & UART_FORWARD_SABM) &&
           (dlc->connection_state EQ UART_CONNECTION_SABM_RCVD) &&
           (uart_data->dlc_instance[dlci] NEQ UART_CONTROL_INSTANCE))
        {
          SET_STATE( UART_SERVICE_KER, KER_MUX );
          /*
           * stop timer
           */
          sig_ker_rt_stop_t3_req();
          /*
           * setup dlc parameter
           */
          uart_data->dlc_instance[dlci] = UART_CONTROL_INSTANCE;
          dlc->connection_state         = UART_CONNECTION_OPEN;
          /*
           * set priority
           */
          dlc->priority = 0;
          /*
           * send response frame
           */
          if(forward & UART_FORWARD_RESPONSE)
          {
            ker_mux_send_frame(received_data);
            /*
             * prepare next reception
             */
            MALLOC(received_data, (USHORT)(sizeof(T_desc2) - 1
                                                          + uart_data->n1
                                                          + 2));
            received_data->next = (ULONG)NULL;
          }
        }
      }
      /*
       * prepare next reception
       */
      received_data->len = 0;
      uart_data->ker.rx_data_desc = received_data;
      sig_ker_rx_ready_to_receive_req(received_data,
                                      0,
                                      (USHORT)(uart_data->n1 + 2));
      break;

    case KER_MUX:
      if(received_data->len EQ 0)
      {
        /*
         * prepare next reception
         */
        sig_ker_rx_ready_to_receive_req(received_data,
                                        write_pos,
                                        (USHORT)(uart_data->n1 + 2));
        break;
      }
      /*
       * analyze frame and messages
       */
      forward = 0;
      switch(received_data->buffer[UART_OFFSET_CONTROL])
      {
        case UART_SABM_FRAME:
          ker_receive_sabm_frame(&forward, received_data);
          break;

        case UART_UA_FRAME:
          ker_receive_ua_frame(&forward, received_data);
          break;

        case UART_DM_CONTROL_FRAME:
        case UART_DM_DATA_FRAME:
          ker_receive_dm_frame(&forward, received_data);
          break;

        case UART_DISC_FRAME:
          ker_receive_disc_frame(&forward, received_data);
          break;

        case UART_UIH_CONTROL_FRAME:
          ker_receive_uih_control_frame(&forward, received_data);
          break;

        case UART_UIH_DATA_FRAME:
          ker_receive_uih_data_frame(&forward, received_data);
          break;

        default:
        {
          TRACE_EVENT_P2("Warning: Unexpected HDLC value: %d, uart_kers.c(%d)",
                         received_data->buffer[UART_OFFSET_CONTROL], __LINE__);
          break;
        }
      }
      /*
       * remove resonded frames and send next frames in command queue
       */
      for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        switch(dlc->connection_state)
        {
          case UART_CONNECTION_OPEN:
          case UART_CONNECTION_DISC_SENT:
            if((dlc->last_command NEQ NULL) &&
               (dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                UART_UIH_CONTROL_FRAME) &&
               (dlc->last_command->len EQ UART_OFFSET_INFO))
            {
              /*
               * remove responded UIH frame
               */
              MFREE_DESC2(dlc->last_command);
              dlc->last_command = NULL;
              uart_data->ker.nr_t2--;
              if( uart_data->ker.nr_t2 EQ 0 )
                sig_ker_rt_stop_t2_req();
            }
            if((dlc->last_command  EQ NULL) &&
               (dlc->next_command NEQ NULL))
            {
              T_desc2* temp_desc;
              /*
               * transmit next command frame
               */
              dlc->last_command       = dlc->next_command;
              dlc->next_command       = (T_desc2*)dlc->next_command->next;
              dlc->last_command->next = (ULONG)NULL;

              MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 +
                                dlc->last_command->len));
              temp_desc->next = (ULONG)NULL;
              temp_desc->len  = dlc->last_command->len;
              memcpy(temp_desc->buffer,
                     dlc->last_command->buffer,
                     dlc->last_command->len);

              dlc->retransmissions = 0;
              ker_mux_send_frame(temp_desc);

              if(dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                 UART_UIH_CONTROL_FRAME)
              {
                uart_data->ker.nr_t2++;
                sig_ker_rt_start_t2_req();
              }
              else
              {
                uart_data->ker.nr_t1++;
                sig_ker_rt_start_t1_req();
              }
            }
            break;

          case UART_CONNECTION_DEAD:
            break;

          default:
          {
            TRACE_EVENT_P2("Unexpected DLC connection state: %d, uart_kers.c(%d)",
                                                 dlc->connection_state, __LINE__);
            break;
          }
        }
      }
      /*
       * process results of analysis
       */
      if(forward)
      {
        for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
        {
          if(i NEQ UART_CONTROL_INSTANCE)
          {
            /*
             * set dlc values
             */
            dlc  = &uart_data->dlc_table[i];
            dlci = dlc->dlci;
            /*
             * channel to open
             */
            if((forward & UART_FORWARD_SABM) &&
               (dlc->connection_state EQ UART_CONNECTION_SABM_RCVD) &&
               (uart_data->dlc_instance[dlci] NEQ i))
            {
              PALLOC(uart_mux_dlc_establish_ind, UART_MUX_DLC_ESTABLISH_IND);
              /*
               * setup dlc parameter
               */
              uart_data->dlc_instance[dlci] = i;
              dlc->drx = &uart_data->drx_base[i];
              dlc->dtx = &uart_data->dtx_base[i];
              /*
               * set priority
               */
              if(dlci < 8)
                dlc->priority = 7;
              else if(dlci < 16)
                dlc->priority = 15;
              else if(dlci < 24)
                dlc->priority = 23;
              else if(dlci < 32)
                dlc->priority = 31;
              else if(dlci < 40)
                dlc->priority = 39;
              else if(dlci < 48)
                dlc->priority = 47;
              else if(dlci < 56)
                dlc->priority = 55;
              else
                dlc->priority = 61;
              /*
               * store response frame and
               * clear appropriate bit in forward mask
               */
              dlc->next_command = received_data;
              forward          &= ~UART_FORWARD_RESPONSE;
              /*
               * create new reception frame
               */
              MALLOC(received_data, (USHORT)(sizeof(T_desc2) - 1
                                                            + uart_data->n1
                                                            + 2));
              received_data->next = (ULONG)NULL;
              /*
               * inform ACI about new DLC
               */
              uart_mux_dlc_establish_ind->device = uart_data->device;
              uart_mux_dlc_establish_ind->dlci   = dlci;
              uart_mux_dlc_establish_ind->
                                     convergence = UART_MUX_CONVERGENCE_UOS;
              uart_mux_dlc_establish_ind->n1     = uart_data->n1;
              uart_mux_dlc_establish_ind->
                                         service = UART_MUX_SERVICE_AT;
              PSEND (hCommMMI, uart_mux_dlc_establish_ind);
            }
            /*
             * channel to close
             */
            if((forward & UART_FORWARD_DLC_RELEASE) &&
               (dlc->connection_state EQ UART_CONNECTION_DEAD) &&
               (dlci NEQ UART_DLCI_INVALID))
            {
              PALLOC(uart_mux_dlc_release_ind, UART_MUX_DLC_RELEASE_IND);
              /*
               * release channel
               */
              ker_mux_dlc_release(i);
              /*
               * inform ACI about DLC release
               */
              uart_mux_dlc_release_ind->device = uart_data->device;
              uart_mux_dlc_release_ind->dlci   = dlci;
              PSEND(hCommMMI, uart_mux_dlc_release_ind);
            }
            /*
             * set Flow Control ON
             */
            if((forward & UART_FORWARD_FCON) &&
               (dlc->connection_state EQ UART_CONNECTION_OPEN) &&
               (!(dlc->lines & UART_FC_RX_MASK)))
            {
              uart_data->drx = dlc->drx;
              sig_ker_drx_enable_req();
            }
            /*
             * set Flow Control OFF
             */
            if((forward & UART_FORWARD_FCOFF) &&
               (dlc->connection_state EQ UART_CONNECTION_OPEN) &&
               (!(dlc->lines & UART_FC_RX_MASK)))
            {
              uart_data->drx = dlc->drx;
              sig_ker_drx_disable_req();
            }
            /*
             * send status lines
             */
            if((forward & UART_FORWARD_MSC) &&
               (dlc->connection_state EQ UART_CONNECTION_OPEN))
            {
              UBYTE st_flow;
              UBYTE st_line_sa;
              UBYTE st_break_len;
              /*
               * send primitive if DTR drop is detected
               */
              if(dlc->lines & UART_DTR_MASK)
              {
                PALLOC (uart_detected_ind, UART_DETECTED_IND);
                uart_detected_ind->device = uart_data->device;
                uart_detected_ind->dlci   = dlc->dlci;
                uart_detected_ind->cause  = UART_DETECT_DTR;
                PSEND (hCommMMI, uart_detected_ind);
              }
              /*
               * set line states
               */
              if(dlc->lines & UART_RTS_MASK)
                st_flow = DTI_FLOW_OFF;
              else
                st_flow = DTI_FLOW_ON;
              if(dlc->lines & UART_DTR_MASK)
                st_line_sa = DTI_SA_OFF;
              else
                st_line_sa = DTI_SA_ON;
              /*
               * set break
               */
              if(dlc->lines & UART_BRK_RX_MASK)
                st_break_len = (UBYTE)((dlc->lines & UART_BRKLEN_RX_MASK) >>
                                                     UART_BRKLEN_RX_POS);
              else
                st_break_len = DTI_BREAK_OFF;
              /*
               * send flow control signals
               */
              if(uart_data->ker.data_flow_tx EQ UART_FLOW_ENABLED)
              {
                uart_data->drx = dlc->drx;
                if(dlc->lines & UART_FC_RX_MASK)
                  sig_ker_drx_disable_req();
                else
                  sig_ker_drx_enable_req();
              }
              /*
               * send line states
               */
              uart_data->dtx = dlc->dtx;
              sig_ker_dtx_line_states_req(st_flow,
                                          st_line_sa,
                                          DTI_SB_ON,
                                          st_break_len);
            }
          }
        }
        /*
         * send response frame
         */
        if(forward & UART_FORWARD_RESPONSE)
        {
          ker_mux_send_frame(received_data);
          uart_data->ker.rx_data_desc = NULL;
        }
        /*
         * Close-Down multiplexer
         */
        if(forward & UART_FORWARD_CLD)
        {
          SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSED );
          /*
           * stop timers and
           * remove all DLC instances
           */
          ker_mux_close_down();
          /*
           * flush UART before change the state
           */
          if(uart_data->ker.tx_data_desc EQ NULL)
          {
            uart_data->ker.received_prim|= UART_MUX_CLOSE_REQ_MASK;

            if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
            {
              uart_data->ker.flush_state = UART_KER_TX_FLUSH;
              sig_ker_tx_flush_req();
            }
          }
        }
      }
      /*
       * prepare next reception
       */
      if(forward & UART_FORWARD_RESPONSE)
      {
        MALLOC(received_data, (USHORT)(sizeof(T_desc2) - 1
                                                      + uart_data->n1
                                                      + 2));
        received_data->next = (ULONG)NULL;
      }
      received_data->len = 0;
      uart_data->ker.rx_data_desc = received_data;
      sig_ker_rx_ready_to_receive_req(received_data,
                                      0,
                                      (USHORT)(uart_data->n1 + 2));
      break;

    case KER_MUX_DLC_CLOSING:
      if(received_data->len EQ 0)
      {
        /*
         * prepare next reception
         */
        sig_ker_rx_ready_to_receive_req(received_data,
                                        write_pos,
                                        (USHORT)(uart_data->n1 + 2));
        break;
      }
      /*
       * analyze frame and messages
       */
      forward = 0;
      switch(received_data->buffer[UART_OFFSET_CONTROL])
      {
        case UART_SABM_FRAME:
          ker_receive_sabm_frame(&forward, received_data);
          break;

        case UART_UA_FRAME:
          ker_receive_ua_frame(&forward, received_data);
          break;

        case UART_DM_CONTROL_FRAME:
        case UART_DM_DATA_FRAME:
          ker_receive_dm_frame(&forward, received_data);
          break;

        case UART_DISC_FRAME:
          ker_receive_disc_frame(&forward, received_data);
          break;

        case UART_UIH_CONTROL_FRAME:
          ker_receive_uih_control_frame(&forward, received_data);
          break;

        case UART_UIH_DATA_FRAME:
          ker_receive_uih_data_frame(&forward, received_data);
          break;

        default:
        {
          TRACE_EVENT_P2("Warning: Unexpected HDLC value: %d, uart_kers.c(%d)",
                         received_data->buffer[UART_OFFSET_CONTROL], __LINE__);
          break;
        }
      }
      /*
       * remove resonded frames and send next frames in command queue
       */
      for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        switch(dlc->connection_state)
        {
          case UART_CONNECTION_OPEN:
          case UART_CONNECTION_DISC_SENT:
            if((dlc->last_command NEQ NULL) &&
               (dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                UART_UIH_CONTROL_FRAME) &&
               (dlc->last_command->len EQ UART_OFFSET_INFO))
            {
              /*
               * remove responded UIH frame
               */
              MFREE_DESC2(dlc->last_command);
              dlc->last_command = NULL;
              uart_data->ker.nr_t2--;
              if( uart_data->ker.nr_t2 EQ 0 )
                sig_ker_rt_stop_t2_req();
            }
            if((dlc->last_command  EQ NULL) &&
               (dlc->next_command NEQ NULL))
            {
              T_desc2* temp_desc;
              /*
               * transmit next command frame
               */
              dlc->last_command       = dlc->next_command;
              dlc->next_command       = (T_desc2*)dlc->next_command->next;
              dlc->last_command->next = (ULONG)NULL;

              MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 +
                                dlc->last_command->len));
              temp_desc->next = (ULONG)NULL;
              temp_desc->len  = dlc->last_command->len;
              memcpy(temp_desc->buffer,
                     dlc->last_command->buffer,
                     dlc->last_command->len);

              dlc->retransmissions = 0;
              ker_mux_send_frame(temp_desc);

              if(dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                 UART_UIH_CONTROL_FRAME)
              {
                uart_data->ker.nr_t2++;
                sig_ker_rt_start_t2_req();
              }
              else
              {
                uart_data->ker.nr_t1++;
                sig_ker_rt_start_t1_req();
              }
            }
            break;

          case UART_CONNECTION_DEAD:
            break;

          default:
          {
            TRACE_EVENT_P2("Unexpected DLC connection state: %d, uart_kers.c(%d)",
                                                 dlc->connection_state, __LINE__);
            break;
          }
        }
      }
      /*
       * process results of analysis
       */
      if(forward)
      {
        continuous = FALSE;
        for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
        {
          if(i NEQ UART_CONTROL_INSTANCE)
          {
            /*
             * set dlc values
             */
            dlc  = &uart_data->dlc_table[i];
            dlci = dlc->dlci;
            /*
             * channel to open
             */
            if((forward & UART_FORWARD_SABM) &&
               (dlc->connection_state EQ UART_CONNECTION_SABM_RCVD))
            {
              /*
               * reject all attempts to open a channel
               */
              received_data->buffer[UART_OFFSET_CONTROL] =
                UART_DM_CONTROL_FRAME;
              /*
               * release channel
               */
              ker_mux_dlc_release(i);
            }
            /*
             * channel to close
             */
            if((forward & UART_FORWARD_DLC_RELEASE) &&
               (dlc->connection_state EQ UART_CONNECTION_DEAD) &&
               (dlci NEQ UART_DLCI_INVALID))
            {
              /*
               * release channel
               */
              ker_mux_dlc_release(i);
            }
            if(dlc->connection_state NEQ UART_CONNECTION_DEAD)
              continuous = TRUE;
          }
        }
        /*
         * send response frame
         */
        if(forward & UART_FORWARD_RESPONSE)
        {
          ker_mux_send_frame(received_data);
          uart_data->ker.rx_data_desc = NULL;
        }
        /*
         * Close-Down multiplexer
         */
        if(forward & UART_FORWARD_CLD)
        {
          SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSED );
          /*
           * stop timers and
           * remove all DLC instances
           */
          ker_mux_close_down();
          /*
           * flush UART before change the state
           */
          if(uart_data->ker.tx_data_desc EQ NULL)
          {
            uart_data->ker.received_prim|= UART_MUX_CLOSE_REQ_MASK;

            if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
            {
              uart_data->ker.flush_state = UART_KER_TX_FLUSH;
              sig_ker_tx_flush_req();
            }
          }
        }
        /*
         * change state if all channels are closed
         */
        else if(continuous EQ FALSE)
        {
          SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSING );
          /*
           * build and send CLD command frame:
           */
          ker_mux_send_close_down();
          /*
           * start timer
           */
          sig_ker_rt_start_t3_req();
        }
      }
      /*
       * prepare next reception
       */
      if(forward & UART_FORWARD_RESPONSE)
      {
        MALLOC(received_data, (USHORT)(sizeof(T_desc2) - 1
                                                      + uart_data->n1
                                                      + 2));
        received_data->next = (ULONG)NULL;
      }
      received_data->len = 0;
      uart_data->ker.rx_data_desc = received_data;
      sig_ker_rx_ready_to_receive_req(received_data,
                                      0,
                                      (USHORT)(uart_data->n1 + 2));
      break;

    case KER_MUX_CLOSING:
      if(received_data->len EQ 0)
      {
        /*
         * prepare next reception
         */
        sig_ker_rx_ready_to_receive_req(received_data,
                                        write_pos,
                                        (USHORT)(uart_data->n1 + 2));
        break;
      }
      /*
       * analyze frame and messages
       */
      forward = 0;
      switch(received_data->buffer[UART_OFFSET_CONTROL])
      {
        case UART_SABM_FRAME:
          ker_receive_sabm_frame(&forward, received_data);
          break;

        case UART_UA_FRAME:
          ker_receive_ua_frame(&forward, received_data);
          break;

        case UART_DM_CONTROL_FRAME:
        case UART_DM_DATA_FRAME:
          ker_receive_dm_frame(&forward, received_data);
          break;

        case UART_DISC_FRAME:
          ker_receive_disc_frame(&forward, received_data);
          break;

        case UART_UIH_CONTROL_FRAME:
          ker_receive_uih_control_frame(&forward, received_data);
          break;

        case UART_UIH_DATA_FRAME:
          ker_receive_uih_data_frame(&forward, received_data);
          break;

        default:
        {
          TRACE_EVENT_P2("Warning: Unexpected HDLC value: %d, uart_kers.c(%d)",
                         received_data->buffer[UART_OFFSET_CONTROL], __LINE__);
          break;
        }
      }
      /*
       * remove resonded frames and send next frames in command queue
       */
      for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        switch(dlc->connection_state)
        {
          case UART_CONNECTION_OPEN:
          case UART_CONNECTION_DISC_SENT:
            if((dlc->last_command NEQ NULL) &&
               (dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                UART_UIH_CONTROL_FRAME) &&
               (dlc->last_command->len EQ UART_OFFSET_INFO))
            {
              /*
               * remove responded UIH frame
               */
              MFREE_DESC2(dlc->last_command);
              dlc->last_command = NULL;
              uart_data->ker.nr_t2--;
              if( uart_data->ker.nr_t2 EQ 0 )
                sig_ker_rt_stop_t2_req();
            }
            if((dlc->last_command  EQ NULL) &&
               (dlc->next_command NEQ NULL))
            {
              T_desc2* temp_desc;
              /*
               * transmit next command frame
               */
              dlc->last_command       = dlc->next_command;
              dlc->next_command       = (T_desc2*)dlc->next_command->next;
              dlc->last_command->next = (ULONG)NULL;

              MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 +
                                dlc->last_command->len));
              temp_desc->next = (ULONG)NULL;
              temp_desc->len  = dlc->last_command->len;
              memcpy(temp_desc->buffer,
                     dlc->last_command->buffer,
                     dlc->last_command->len);

              dlc->retransmissions = 0;
              ker_mux_send_frame(temp_desc);

              if(dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
                 UART_UIH_CONTROL_FRAME)
              {
                uart_data->ker.nr_t2++;
                sig_ker_rt_start_t2_req();
              }
              else
              {
                uart_data->ker.nr_t1++;
                sig_ker_rt_start_t1_req();
              }
            }
            break;

          case UART_CONNECTION_DEAD:
            break;

          default:
          {
            TRACE_EVENT_P2("Unexpected DLC connection state: %d, uart_kers.c(%d)",
                                                  dlc->connection_state,__LINE__);
            break;
          }
        }
      }
      /*
       * process results of analysis
       */
      if(forward)
      {
        for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
        {
          if(i NEQ UART_CONTROL_INSTANCE)
          {
            /*
             * set dlc values
             */
            dlc  = &uart_data->dlc_table[i];
            dlci = dlc->dlci;
            /*
             * channel to open
             */
            if((forward & UART_FORWARD_SABM) &&
               (dlc->connection_state EQ UART_CONNECTION_SABM_RCVD))
            {
              /*
               * reject all attempts to open a channel
               */
              received_data->buffer[UART_OFFSET_CONTROL] =
                UART_DM_CONTROL_FRAME;
              /*
               * release channel
               */
              ker_mux_dlc_release(i);
            }
            /*
             * channel to close
             */
            if((forward & UART_FORWARD_DLC_RELEASE) &&
               (dlc->connection_state EQ UART_CONNECTION_DEAD) &&
               (dlci NEQ UART_DLCI_INVALID))
            {
              /*
               * release channel
               */
              ker_mux_dlc_release(i);
            }
          }
        }
        /*
         * send response frame
         */
        if(forward & UART_FORWARD_RESPONSE)
        {
          ker_mux_send_frame(received_data);
          uart_data->ker.rx_data_desc = NULL;
        }
        /*
         * Close-Down multiplexer
         */
        if(forward & UART_FORWARD_CLD)
        {
          SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSED );
          /*
           * stop timers and
           * remove all DLC instances
           */
          ker_mux_close_down();
          /*
           * flush UART before change the state
           */
          if(uart_data->ker.tx_data_desc EQ NULL)
          {
            uart_data->ker.received_prim|= UART_MUX_CLOSE_REQ_MASK;

            if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
            {
              uart_data->ker.flush_state = UART_KER_TX_FLUSH;
              sig_ker_tx_flush_req();
            }
          }
        }
      }
      /*
       * prepare next reception
       */
      if(forward & UART_FORWARD_RESPONSE)
      {
        MALLOC(received_data, (USHORT)(sizeof(T_desc2) - 1
                                                      + uart_data->n1
                                                      + 2));
        received_data->next = (ULONG)NULL;
      }
      received_data->len = 0;
      uart_data->ker.rx_data_desc = received_data;
      sig_ker_rx_ready_to_receive_req(received_data,
                                      0,
                                      (USHORT)(uart_data->n1 + 2));
      break;

    case KER_MUX_CLOSED:
      if((uart_data->ker.tx_data_desc EQ NULL) &&
         (uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING))
      {
        PALLOC(uart_mux_close_ind, UART_MUX_CLOSE_IND);
        SET_STATE( UART_SERVICE_KER, KER_READY );
        /*
         * free receiving buffer
         */
        MFREE_DESC2(uart_data->ker.rx_data_desc);
        uart_data->ker.rx_data_desc = NULL;
        /*
         * set dlc values
         */
        dlc            = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
        dlc->drx       = &uart_data->drx_base[0];
        dlc->dtx       = &uart_data->dtx_base[0];
        dlc->dlci      = 0;
        dlc->priority  = 0;
        /*
         * set RX and TX in ready mode
         */
        sig_ker_rx_ready_mode_req();
        sig_ker_tx_ready_mode_req();
        /*
         * set frame size for ready mode
         */
        uart_data->n1 = UART_N1_READY_MODE;
        /*
         * set DRX and DTX in ready mode
         */
        uart_data->dtx = dlc->dtx;
        uart_data->drx = dlc->drx;
        sig_ker_drx_ready_mode_req(UART_CONTROL_INSTANCE);
        sig_ker_dtx_ready_mode_req(UART_CONTROL_INSTANCE);
        /*
         * inform ACI about entering ready mode
         */
        uart_mux_close_ind->device = uart_data->device;
        PSEND(hCommMMI, uart_mux_close_ind);
      }
      else if(received_data->len EQ 0)
      {
        /*
         * prepare next reception
         */
        sig_ker_rx_ready_to_receive_req(received_data,
                                        write_pos,
                                        (USHORT)(uart_data->n1 + 2));
      }
      else
      {
        /*
         * prepare next reception
         */
        received_data->len = 0;
        sig_ker_rx_ready_to_receive_req(received_data,
                                        0,
                                        (USHORT)(uart_data->n1 + 2));
      }
      break;

    default:
      TRACE_ERROR( "SIG_RX_KER_DATA_RECEIVED_IND unexpected" );
      break;
  }
} /* sig_rx_ker_data_received_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rx_ker_line_states_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RX_KER_LINE_STATES_IND
|
| Parameters  : line_states - new line states
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rx_ker_line_states_ind (ULONG line_states)
{
  T_DLC*  dlc;
  UBYTE   st_flow;
  UBYTE   st_line_sa;
  UBYTE   st_line_sb;
  UBYTE   st_break_len;

  TRACE_ISIG( "sig_rx_ker_line_states_ind" );

  /*
   * set DLC
   */
  dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  /*
   * send primitive if escape sequence or DTR drop is detected
   */
  if(line_states & UART_ESC_RX_MASK)
  {
    PALLOC (uart_detected_ind, UART_DETECTED_IND);
    uart_detected_ind->device = uart_data->device;
    uart_detected_ind->dlci   = dlc->dlci;
    uart_detected_ind->cause  = UART_DETECT_ESC;
    PSEND (hCommMMI, uart_detected_ind);
  }
  if((line_states & UART_DTR_MASK) AND
     (!(dlc->lines & UART_DTR_MASK)))
  {
    PALLOC (uart_detected_ind, UART_DETECTED_IND);
    uart_detected_ind->device = uart_data->device;
    uart_detected_ind->dlci   = dlc->dlci;
    uart_detected_ind->cause  = UART_DETECT_DTR;
    PSEND (hCommMMI, uart_detected_ind);
  }

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
      /*
       * set line states
       */
      if(line_states & UART_X_RX_MASK)
        st_flow = DTI_FLOW_OFF;
      else
        st_flow = DTI_FLOW_ON;

      if(line_states & UART_SA_RX_MASK)
        st_line_sa = DTI_SA_OFF;
      else
        st_line_sa = DTI_SA_ON;

      if(line_states & UART_SB_RX_MASK)
        st_line_sb = DTI_SB_OFF;
      else
        st_line_sb = DTI_SB_ON;

      if(line_states & UART_BRK_RX_MASK)
        st_break_len = (UBYTE)((line_states & UART_BRKLEN_RX_MASK)
                               >> UART_BRKLEN_RX_POS);
      else
        st_break_len = DTI_BREAK_OFF;

      uart_data->dtx = dlc->dtx;
      sig_ker_dtx_line_states_req(st_flow,
                                  st_line_sa,
                                  st_line_sb,
                                  st_break_len);
      break;

    case KER_MUX_ESTABLISH:
    case KER_MUX:
    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      break;

    default:
      TRACE_ERROR( "SIG_RX_KER_LINE_STATES_IND unexpected" );
      break;
  }
  /*
   * store new line states
   */
  dlc->lines &= ~(UART_X_RX_MASK   |
                  UART_SA_RX_MASK  |
                  UART_SB_RX_MASK  |
                  UART_ESC_RX_MASK |
                  UART_BRK_RX_MASK |
                  UART_BRKLEN_RX_MASK);
  line_states&= ~(UART_ESC_RX_MASK |
                  UART_BRK_RX_MASK |
                  UART_BRKLEN_RX_MASK);
  dlc->lines |= line_states;
} /* sig_rx_ker_line_states_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rt_ker_timeout_t1_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RT_KER_TIMEOUT_T1_IND
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rt_ker_timeout_t1_ind ()
{
  UBYTE   i;
  T_desc2* temp_desc;
  T_DLC*  dlc;
  UBYTE   j;
  UBYTE   dlci;
  ULONG   forward;
  BOOL    continuous;

  TRACE_ISIG( "sig_rt_ker_timeout_t1_ind" );

  for( i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++ )
  {
    /*
     * set dlc value
     */
    dlc = &uart_data->dlc_table[i];
    switch(dlc->connection_state)
    {
      case UART_CONNECTION_OPEN:
      case UART_CONNECTION_DISC_SENT:
        /*
         * T1 is used for non UIH Control frames
         */
        if((dlc->last_command NEQ NULL) &&
           (dlc->last_command->buffer[UART_OFFSET_CONTROL] NEQ
             UART_UIH_CONTROL_FRAME))
        {
          if(dlc->retransmissions < uart_data->ker.n2)
          {
            /*
             * retransmission of Control frame
             */
            dlc->retransmissions++;
            MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1
              + dlc->last_command->len));

            temp_desc->next = (ULONG)NULL;
            temp_desc->len  = dlc->last_command->len;
            memcpy(temp_desc->buffer,
                   dlc->last_command->buffer,
                   dlc->last_command->len);

            ker_mux_send_frame( temp_desc );
          }
          else
          {
            /*
             * maximum number of retransmissions reached
             */
            switch( GET_STATE( UART_SERVICE_KER ) )
            {
              case KER_MUX:
                /*
                 * act as on reception of a DM frame
                 */
                /*
                 * create DM frame
                 */
                MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 + 2));
                temp_desc->next = (ULONG)NULL;
                temp_desc->len  = 2;
                memcpy(temp_desc->buffer, dlc->last_command->buffer, 2)
                ;/*lint !e419 !e420 apparent data overrun and access beyond array*/
                temp_desc->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
                /*
                 * act as on reception of a DM frame
                 */
                forward = 0;
                ker_receive_dm_frame(&forward, temp_desc);
                /*
                 * process results of analysis
                 */
                if(forward)
                {
                  for(j=0; j <= UART_MAX_NUMBER_OF_CHANNELS; j++)
                  {
                    if(j NEQ UART_CONTROL_INSTANCE)
                    {
                      /*
                       * set dlc values
                       */
                      dlc  = &uart_data->dlc_table[j];
                      dlci = dlc->dlci;
                      /*
                       * channel to close
                       */
                      if((forward & UART_FORWARD_DLC_RELEASE) &&
                         (dlc->connection_state EQ UART_CONNECTION_DEAD) &&
                         (dlci NEQ UART_DLCI_INVALID))
                      {
                        PALLOC(uart_mux_dlc_release_ind,
                               UART_MUX_DLC_RELEASE_IND);
                        /*
                         * release channel
                         */
                        ker_mux_dlc_release(j);
                        /*
                         * inform ACI about DLC release
                         */
                        uart_mux_dlc_release_ind->device = uart_data->device;
                        uart_mux_dlc_release_ind->dlci   = dlci;
                        PSEND(hCommMMI, uart_mux_dlc_release_ind);
                      }
                    }
                  }
                }
                break;

              case KER_MUX_DLC_CLOSING:
                /*
                 * act as on reception of a DM frame
                 */
                /*
                 * create DM frame
                 */
                MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 + 2));
                temp_desc->next = (ULONG)NULL;
                temp_desc->len  = 2;
                memcpy(temp_desc->buffer, dlc->last_command->buffer, 2)
                ;/*lint !e419 !e420 apparent data overrun and access beyond array*/
                temp_desc->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
                /*
                 * act as on reception of a DM frame
                 */
                forward = 0;
                ker_receive_dm_frame(&forward, temp_desc);
                /*
                 * process results of analysis
                 */
                if(forward)
                {
                  continuous = FALSE;
                  for(j=0; j <= UART_MAX_NUMBER_OF_CHANNELS; j++)
                  {
                    if(j NEQ UART_CONTROL_INSTANCE)
                    {
                      /*
                       * set dlc values
                       */
                      dlc  = &uart_data->dlc_table[j];
                      dlci = dlc->dlci;
                      /*
                       * channel to close
                       */
                      if((forward & UART_FORWARD_DLC_RELEASE) &&
                         (dlc->connection_state EQ UART_CONNECTION_DEAD) &&
                         (dlci NEQ UART_DLCI_INVALID))
                      {
                        /*
                         * release channel
                         */
                        ker_mux_dlc_release(j);
                      }
                      if(dlc->connection_state NEQ UART_CONNECTION_DEAD)
                        continuous = TRUE;
                    }
                  }
                  /*
                   * change state if all channels are closed
                   */
                  if(continuous EQ FALSE)
                  {
                    SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSING );
                    /*
                     * build and send CLD command frame:
                     */
                    ker_mux_send_close_down();
                    /*
                     * start timer
                     */
                    sig_ker_rt_start_t3_req();
                  }
                }
                break;

              case KER_MUX_CLOSING:
                /*
                 * do not care about retransmission couter
                 * retransmission stops if T3 expires
                 */
                dlc->retransmissions++;
                MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1
                  + dlc->last_command->len));

                temp_desc->next = (ULONG)NULL;
                temp_desc->len  = dlc->last_command->len;
                memcpy(temp_desc->buffer,
                       dlc->last_command->buffer,
                       dlc->last_command->len);

                ker_mux_send_frame( temp_desc );
                break;

              default:
                TRACE_ERROR( "SIG_RT_KER_TIMEOUT_T1_IND unexpected" );
                break;
            }
          }
        }
        break;

      case UART_CONNECTION_DEAD:
        break;

      default:
      {
        TRACE_EVENT_P2("Unexpected DLC connection state: %d, uart_kers.c(%d)",
                                             dlc->connection_state, __LINE__);
        break;
      }
    }
  }
  if(uart_data->ker.nr_t1)
  {
    /*
     * restart timer t1
     */
    sig_ker_rt_start_t1_req();
  }
} /* sig_rt_ker_timeout_t1_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rt_ker_timeout_t2_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RT_KER_TIMEOUT_T2_IND
|
|               This signal means that no response to a sent command frame
|               was received within the allowed time. It is assumed that
|               the frame has been lost and a retransmission is done if the
|               maximum number of retransmissions is not reached yet.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rt_ker_timeout_t2_ind ()
{
  UBYTE   i;
  T_desc2* temp_desc;
  T_DLC*  dlc;

  TRACE_ISIG( "sig_rt_ker_timeout_t2_ind" );

  for( i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++ )
  {
    /*
     * set dlc value
     */
    dlc = &uart_data->dlc_table[i];
    switch(dlc->connection_state)
    {
      case UART_CONNECTION_OPEN:
      case UART_CONNECTION_DISC_SENT:
        /*
         * T2 is only used for UIH Control frames
         */
        if((dlc->last_command NEQ NULL) &&
           (dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
             UART_UIH_CONTROL_FRAME))
        {
          if(dlc->retransmissions < uart_data->ker.n2)
          {
            /*
             * retransmission of Control frame
             */
            dlc->retransmissions++;
            MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1
              + dlc->last_command->len));

            temp_desc->next = (ULONG)NULL;
            temp_desc->len  = dlc->last_command->len;
            memcpy(temp_desc->buffer,
                   dlc->last_command->buffer,
                   dlc->last_command->len);

            ker_mux_send_frame( temp_desc );
          }
          else
          {
            /*
             * maximum number of retransmissions reached
             */
            switch( GET_STATE( UART_SERVICE_KER ) )
            {
              case KER_MUX:
              case KER_MUX_DLC_CLOSING:
                /*
                 * skip this frame and send next frame
                 */
                /*
                 * remove current UIH frame
                 */
                MFREE_DESC2(dlc->last_command);
                dlc->last_command = NULL;
                uart_data->ker.nr_t2--;
                if(dlc->next_command)
                {
                  /*
                   * transmit next command frame
                   */
                  dlc->last_command       = dlc->next_command;
                  dlc->next_command       = (T_desc2*)dlc->next_command->next;
                  dlc->last_command->next = (ULONG)NULL;

                  MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1 +
                                    dlc->last_command->len));
                  temp_desc->next = (ULONG)NULL;
                  temp_desc->len  = dlc->last_command->len;
                  memcpy(temp_desc->buffer,
                         dlc->last_command->buffer,
                         dlc->last_command->len);

                  dlc->retransmissions = 0;
                  ker_mux_send_frame(temp_desc);
                  uart_data->ker.nr_t2++;
                }
                break;

              case KER_MUX_CLOSING:
                /*
                 * do not care about retransmission couter
                 * retransmission stops if T3 expires
                 */
                dlc->retransmissions++;
                MALLOC(temp_desc, (USHORT)(sizeof( T_desc2 ) - 1
                  + dlc->last_command->len));

                temp_desc->next = (ULONG)NULL;
                temp_desc->len  = dlc->last_command->len;
                memcpy(temp_desc->buffer,
                       dlc->last_command->buffer,
                       dlc->last_command->len);

                ker_mux_send_frame( temp_desc );
                break;

              default:
                TRACE_ERROR( "SIG_RT_KER_TIMEOUT_T2_IND unexpected" );
                break;
            }
          }
        }
        break;

      case UART_CONNECTION_DEAD:
        break;

      default:
      {
        TRACE_EVENT_P2("Unexpected DLC connection state: %d, uart_kers.c(%d)",
                                             dlc->connection_state, __LINE__);
        break;
      }
    }
  }
  if(uart_data->ker.nr_t2)
  {
    /*
     * restart timer t2
     */
    sig_ker_rt_start_t2_req();
  }
} /* sig_rt_ker_timeout_t2_ind() */



/*
+------------------------------------------------------------------------------
| Function    : sig_rt_ker_timeout_t3_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RT_KER_TIMEOUT_T3_IND
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rt_ker_timeout_t3_ind ()
{
  TRACE_ISIG( "sig_rt_ker_timeout_t3_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX_ESTABLISH:
    case KER_MUX_CLOSING:
      SET_STATE( UART_SERVICE_KER, KER_MUX_CLOSED );
      /*
       * stop timers and
       * remove all DLC instances
       */
      ker_mux_close_down();
      if(uart_data->ker.tx_data_desc EQ NULL)
      {
        uart_data->ker.received_prim|= UART_MUX_CLOSE_REQ_MASK;

        if(uart_data->ker.flush_state EQ UART_KER_NOT_FLUSHING)
        {
          uart_data->ker.flush_state = UART_KER_TX_FLUSH;
          sig_ker_tx_flush_req();
        }
      }
      break;

    default:
      TRACE_ERROR( "SIG_RT_KER_TIMEOUT_T3_IND unexpected" );
      break;
  }
} /* sig_rt_ker_timeout_t3_ind() */


/*
+------------------------------------------------------------------------------
| Function    : sig_rt_ker_timeout_tesd_ind
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_RT_KER_TIMEOUT_TESD_IND
|
|               This signal means that the trailing guard period has completed.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_rt_ker_timeout_tesd_ind ()
{
  UBYTE   i;

  TRACE_ISIG( "sig_rt_ker_timeout_tesd_ind" );

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
    case KER_MUX:
      for( i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++ )
      { /*
         * set current dtx
         */
        uart_data->dtx = &uart_data->dtx_base[i];
        sig_ker_dtx_timeout_tesd_req();
      }
      break;

    default:
      TRACE_ERROR( "SIG_RT_KER_TIMEOUT_TESD_IND unexpected" );
      break;
  }
} /* sig_rt_ker_timeout_tesd_ind() */


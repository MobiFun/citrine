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
|             procedures and functions as described in the
|             SDL-documentation (KER-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_KERF_C
#define UART_KERF_C
#endif /* !UART_KERF_C */

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_UART

/*
 * Turn off spurious LINT warnings
 */
 /*lint -e415 access of out-of-bounds pointer */
 /*lint -e416 creation of out-of-bounds pointer */
 /*lint -e661 possible access of out-of-bounds pointer */
 /*lint -e662 possible craetion of out-of-bounds pointer */


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

#include "uart_kerf.h"  /* to get KER function definitions */
#include "uart_drxs.h"  /* to get signal definitions for service DRX */
#include "uart_dtxs.h"  /* to get signal definitions for service DTX */
#ifdef FF_MULTI_PORT
#include "uart_ptxs.h"  /* to get signal definitions for service TX */
#else /* FF_MULTI_PORT */
#include "uart_txs.h"   /* to get signal definitions for service TX */
#endif /* FF_MULTI_PORT */
#include "uart_rts.h"   /* to get signal definitions for service RT */
#include <string.h>    /* JK, delete warnings: to get memmove */
/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : ker_setupUart
+------------------------------------------------------------------------------
| Description : The function ker_setupUart() sets the communication parameters
|               of UART
|
| Parameters  : no parameter
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_setupUart(void)
{
  T_UFRET ret; /* Error code returned from a function */

  TRACE_FUNCTION( "ker_setupUart" );

#ifdef FF_MULTI_PORT
  /*
   * set new XON / XOFF character
   */
  uart_data->xon  = uart_data->ker.act_dcb.XON;
  uart_data->xoff = uart_data->ker.act_dcb.XOFF;
  /*
   * set new parameters
   */
#ifndef _SIMULATION_
  if((ret = GSI_SetConfig(uart_data->device, &uart_data->ker.act_dcb))
                                                           NEQ DRV_OK)
  {
    TRACE_ERROR_P2
      ("GSI driver: Serial devise configuration failed; [%d], uart_kerf.c(%d)",
                                                                ret, __LINE__);
  }
#endif /* !_SIMULATION_ */
#else /* FF_MULTI_PORT */
  /*
   * set new XON / XOFF character
   */
  uart_data->xon  = uart_data->ker.act_xon;
  uart_data->xoff = uart_data->ker.act_xoff;
  /*
   * set new escape sequence parameters
   */
  uart_data->act_ec = uart_data->ker.act_ec;
  uart_data->act_gp = uart_data->ker.act_gp;
  /*
   * set new parameters
   */
  /*
   * set up the escape sequence
   */
  ret = UF_SetEscape (uart_data->device,
                      uart_data->act_ec,
                      uart_data->act_gp);
#ifdef _SIMULATION_
  TRACE_EVENT_P1 ("UF_SetEscape() = %x", (USHORT) ret);
#endif /* _SIMULATION_ */
  while ((ret = UF_SetComPar (uart_data->device,
                              uart_data->ker.act_br,
                              uart_data->ker.act_bpc,
                              uart_data->ker.act_sb,
                              uart_data->ker.act_par)) EQ UF_NOT_READY)
  {
    if(vsi_t_sleep (VSI_CALLER ONE_FRAME) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't suspend thread, uart_kerf.c(%d)",
                                                               __LINE__);
    }
  }

  /*
   * set new flow control
   */
  if (ret EQ UF_OK)
  {
    if((ret = UF_SetFlowCtrl (uart_data->device, uart_data->ker.act_fc_rx,
                              uart_data->xon, uart_data->xoff) NEQ UF_OK)
                              AND (uart_data->device NEQ 0))
    {
      TRACE_ERROR_P2("UF driver: Can't set new flow control, [%d], uart_kerf(%d)",
                                                                   ret, __LINE__);
    }
  }
#endif /* FF_MULTI_PORT */
} /* ker_setupUart() */



/*
+------------------------------------------------------------------------------
| Function    : ker_init
+------------------------------------------------------------------------------
| Description : The function ker_init() initializes the UART
|
| Parameters  : no parameter
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_init ()
{
#ifdef FF_MULTI_PORT
#ifndef _SIMULATION_
  T_DRV_EXPORT* drv_export;
#endif /* !_SIMULATION_ */
#endif /* FF_MULTI_PORT */

  T_UFRET ret;       /* Error code returned from a function */

  TRACE_FUNCTION( "ker_init" );

  /*
   * initialize values
   */
#ifdef FF_MULTI_PORT
  uart_data->ker.act_dcb.Baud          = GSI_BAUD_9600;
  uart_data->ker.act_dcb.DataBits      = GSI_CHAR8;
  uart_data->ker.act_dcb.StopBits      = GSI_STOP1;
  uart_data->ker.act_dcb.Parity        = GSI_PARITYNO;
  uart_data->ker.act_dcb.RxFlowControl = GSI_FLOWHW;
  uart_data->ker.act_dcb.TxFlowControl = GSI_FLOWHW;
  uart_data->ker.act_dcb.RxBufferSize  = GSI_MAX_BUFFER_SIZE;
  uart_data->ker.act_dcb.TxBufferSize  = GSI_MAX_BUFFER_SIZE;
  uart_data->ker.act_dcb.RxThreshold   = 1;
  uart_data->ker.act_dcb.TxThreshold   = 1;
  uart_data->ker.act_dcb.XON           = UART_IO_XON_DEFAULT;
  uart_data->ker.act_dcb.XOFF          = UART_IO_XOFF_DEFAULT;
  uart_data->ker.act_dcb.EscChar       = UART_ESC_CHARACTER_DEFAULT;
  uart_data->ker.act_dcb.GuardPeriod   = UART_GUARD_PERIOD_DEFAULT;
#else /* FF_MULTI_PORT */
  uart_data->ker.act_br    = UF_BAUD_9600; /* 9600 baud */
  uart_data->ker.act_bpc   = bpc_8;        /* 8 bits per character */
  uart_data->ker.act_sb    = sb_1;         /* 1 stop bit */
  uart_data->ker.act_par   = pa_none;      /* no parity no space */
  uart_data->ker.act_fc_rx = fc_rts;       /* Hardware flow control */
  uart_data->ker.act_fc_tx = fc_rts;       /* Hardware flow control */
  uart_data->ker.act_xon   = UART_IO_XON_DEFAULT;  /* XOn character */
  uart_data->ker.act_xoff  = UART_IO_XOFF_DEFAULT; /* XOff character */
  uart_data->ker.act_ec    = UART_IO_ESC_CHAR_DEFAULT; /* escape character */
  uart_data->ker.act_gp    = UART_IO_ESC_GP_DEFAULT;   /* guard period */
#endif /* FF_MULTI_PORT */

  /* bitfield of received UART primitives */
  uart_data->ker.received_prim = 0;
  uart_data->ker.flush_state   = UART_KER_NOT_FLUSHING;

  uart_data->ker.rx_data_desc    = NULL;    /* data received from peer  */
  uart_data->ker.receiving_state = UART_KER_NOT_RECEIVING;

  uart_data->ker.tx_data_desc    = NULL;    /* data to be sent to peer */
  /* data waiting for access to tx_data_desc */
  uart_data->ker.tx_data_waiting = NULL;
  uart_data->ker.sending_state   = UART_KER_NOT_SENDING;
  uart_data->ker.data_flow_tx    = UART_FLOW_ENABLED;
  uart_data->ker.nr_t1           = 0;       /* nr running T1 timers yet */
  uart_data->ker.nr_t2           = 0;       /* nr running T2 timers yet */
  uart_data->ker.n2              = 0;       /* max nr of retransmissions */

#ifdef FF_MULTI_PORT
  /*
   * initialize driver
   */
#ifndef _SIMULATION_
  if((ret=GSI_Init(uart_data->device, uart_data->device,
                          pei_uart_driver_signal, &drv_export)) NEQ DRV_OK)
  {
    TRACE_ERROR_P2("GSI driver: InitSerialDevice failed, [%d], uart_kerf.c(%d)",
                                                                 ret, __LINE__);
  }
#endif /* _SIMULATION_ */
  /*
   * set driver signals
   */
#ifndef _SIMULATION_
  if((ret = GSI_SetSignal(uart_data->device, DRV_SIGTYPE_READ  |
                                             DRV_SIGTYPE_WRITE |
                                             DRV_SIGTYPE_FLUSH) NEQ DRV_OK)
  {
    TRACE_ERROR_P2("GSI entity: SetSignals failed, [%d], uart_kerf.c(%d)",
                                                           ret, __LINE__);
  }
#endif /* _SIMULATION_ */
#else /* FF_MULTI_PORT */
  /*
   * initialize driver
   */
  if((ret = UF_Init (uart_data->device)) NEQ UF_OK)
  {
    TRACE_ERROR_P2("UF driver: InitSerialDevice failed, [%d], uart_kerf.c(%d)",
                                                                ret, __LINE__);
  }
#ifdef _SIMULATION_
  TRACE_EVENT_P1 ("UF_Init() = %x", (USHORT) ret);
#endif /* _SIMULATION_ */

  /*
   * disable UART
   */
  if((ret = UF_Enable (uart_data->device, FALSE)) NEQ UF_OK)
  {
    TRACE_ERROR_P2("UF driver: DisableDriver failed, [%d], uart_kerf.c(%d)",
                                                             ret, __LINE__);
  }
  /*
   * set buffer size
   */
  if((ret = UF_SetBuffer (uart_data->device, UF_MAX_BUFFER_SIZE, 1, 1))
                                                             NEQ UF_OK)
  {
    TRACE_ERROR_P2("UF driver: SetBufferSize failed, [%d], uart_kerf.c(%d)",
                                                             ret, __LINE__);
  }
#ifdef _SIMULATION_
  TRACE_EVENT_P1 ("UF_SetBuffer() = %x", (USHORT) ret);
  TRACE_EVENT_P1 ("Buffer avail = %d",
                  (USHORT) UF_OutpAvail (uart_data->device));
#endif /* _SIMULATION_ */
#endif /* FF_MULTI_PORT */

  /*
   * set communication parameters
   */
  ker_setupUart();

  INIT_STATE( UART_SERVICE_KER , KER_DEAD );

} /* ker_init() */



/*
+------------------------------------------------------------------------------
| Function    : ker_analyze_frame_info_command
+------------------------------------------------------------------------------
| Description : The function ker_analyze_frame_info_command() analyzes the
|               information field of incoming frames.
|               The appropriate internal signals are triggered and a response
|               frame information field is generated.
|
|               Precondition is that the frame check sequence has been
|               verified, the flags have been removed from the frame and
|               message resonses have been removed from the frame.
|
| Parameters  : forward - result of analysis
|               frame   - descriptor which includes frame type
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_analyze_frame_info_command (ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  USHORT  i;
  USHORT  pos;
  USHORT  len;

  TRACE_FUNCTION( "ker_analyze_frame_info_command" );

  pos = UART_OFFSET_INFO;

  /*
   * parse frame info field until last octet is reached
   * (minimal message has 2 bytes: type + value)
   */
  while(pos < frame->len)
  {
    len = 0;
    while(!(frame->buffer[pos + len] & UART_EA))
    {
      len++;
    }
    len+= (frame->buffer[pos + 1] >> UART_MSG_LENGTH_POS) + 2;
    switch(frame->buffer[pos])
    {
      case UART_MSG_TYPE_CLD_C:
        /*
         * Close Down
         */
        *forward|= UART_FORWARD_CLD;
        frame->buffer[pos] = UART_MSG_TYPE_CLD_R;
        break;
      case UART_MSG_TYPE_FCON_C:
        /*
         * Flow Control On
         * inform all DRX instances except Control channel
         */
        uart_data->ker.data_flow_tx = UART_FLOW_ENABLED;
        *forward|= UART_FORWARD_FCON;
        frame->buffer[pos] = UART_MSG_TYPE_FCON_R;
        break;
      case UART_MSG_TYPE_FCOFF_C:
        /*
         * Flow Control Off
         * inform all DRX instances except Control channel
         */
        uart_data->ker.data_flow_tx = UART_FLOW_DISABLED;
        *forward|= UART_FORWARD_FCOFF;
        frame->buffer[pos] = UART_MSG_TYPE_FCOFF_R;
        break;
      case UART_MSG_TYPE_MSC_C:
        /*
         * Modem Status Command
         * can be 2 or 3 octets
         * (depends if break signal is included or not)
         */
        dlci = frame->buffer[pos+2] >> UART_DLCI_POS;

        if((dlci NEQ UART_DLCI_CONTROL) &&
           (uart_data->dlc_instance[dlci] NEQ UART_EMPTY_INSTANCE))
        {
          dlc = &uart_data->dlc_table[uart_data->dlc_instance[dlci]];

          /*
           * set flow control
           */
          if(frame->buffer[pos+3] & UART_MSC_FC_MASK)
            dlc->lines|= UART_FC_RX_MASK;
          else
            dlc->lines&= ~UART_FC_RX_MASK;

          /*
           * set line states
           */
          if(frame->buffer[pos+3] & UART_MSC_RTR_MASK)
            dlc->lines&= ~UART_RTS_MASK;
          else
            dlc->lines|= UART_RTS_MASK;

          if(frame->buffer[pos+3] & UART_MSC_RTC_MASK)
            dlc->lines&= ~UART_DTR_MASK;
          else
            dlc->lines|= UART_DTR_MASK;

          if((len > 4) &&
             (frame->buffer[pos+4] & UART_MSC_BRK_MASK))
          {
            dlc->lines|= UART_BRK_RX_MASK;
            dlc->lines|= ((ULONG)(frame->buffer[pos+4] & UART_MSC_BRKLEN_MASK) >>
                          UART_MSC_BRKLEN_POS) <<
                          UART_BRKLEN_RX_POS;
          }
          *forward|= UART_FORWARD_MSC;
        }
        else
        {
          TRACE_EVENT( "sig_ker_ker_MSC_C: MSC for control channel or \
                                                 not established DLC" );
        };
        frame->buffer[pos] = UART_MSG_TYPE_MSC_R;
        break;
      default:
        TRACE_EVENT( "ker_analyze_frame_info_command: received \
                                     unsupported message type" );
        /*
         * create Non Supported Command response
         */
        i = 0;
        while(!(frame->buffer[pos + i] & UART_EA))
        {
          i++;
        }
        if(frame->len < uart_data->n1)
        {
          /*
           * move commands behind current command
           */
          if(frame->len > (pos + len))
          {
            if(len NEQ (i + 3))
            {
              memmove(&frame->buffer[pos + i + 3],
                      &frame->buffer[pos + len],
                      frame->len - pos - len)
              ;/*lint !e797 Conceivable creation of out-of-bounds pointer*/
              frame->len = frame->len - len + i + 3;
            }
          }
          else
            frame->len = pos + i + 3;
          /*
           * insert Non Supported Command
           */
          len = i + 3;
          /*lint -e669 -e670 (Warning -- data overrun/access beyond array) */
          memmove(&frame->buffer[pos + 2], &frame->buffer[pos], i + 1)
            ;/*lint !e803 !e804 Conceivable data overrun and access beyond array*/
          /*lint +e669 +e670 (Warning -- data overrun/access beyond array) */
          frame->buffer[pos + 1] = (((UBYTE)i + 1) << UART_MSG_LENGTH_POS) |
                                                      UART_EA;
          frame->buffer[pos]     = UART_MSG_TYPE_NSC_R;
        }
        else
        {
          /*
           * remove command
           */
          if(frame->len > (pos + len))
          {
            memmove(&frame->buffer[pos],
                    &frame->buffer[pos + len],
                    frame->len - pos - len);
            frame->len-= len;
          }
          else
            frame->len = pos;
          len = 0;
        }
        break;
    }
    pos+= len;
  }

} /* ker_analyze_frame_info_command() */



/*
+------------------------------------------------------------------------------
| Function    : ker_search_msg_type
+------------------------------------------------------------------------------
| Description : The function ker_search_msg_type() searches for a message type
|               in a frame.
|
| Parameters  : frame - descriptor which includes message type
|               pos   - position to start searching
|               type  - message type to search for
|
| Return      : indicator whether message type was found
|
+------------------------------------------------------------------------------
*/
LOCAL BOOL ker_search_msg_type (T_desc2* frame, USHORT* pos, UBYTE type)
{
  TRACE_FUNCTION( "ker_search_msg_type" );

  while(*pos < frame->len)
  {
    if(frame->buffer[*pos] EQ type)
    {
      return TRUE;
    }
    *pos+= (frame->buffer[*pos + 1] >> UART_MSG_LENGTH_POS) + 2;
  }
  return FALSE;
} /* ker_search_msg_type() */



/*
+------------------------------------------------------------------------------
| Function    : ker_analyze_frame_info_resonse
+------------------------------------------------------------------------------
| Description : The function ker_analyze_frame_info_response() analyzes the
|               information field of incoming frames.
|               The appropriate internal signals are triggered and the
|               responses are removed from the information field.
|
|               Precondition is that the frame check sequence has been verified
|               and that the flags have been removed from the frame.
|
| Parameters  : forward - result of analysis
|               frame   - descriptor which includes frame type
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_analyze_frame_info_response (ULONG* forward, T_desc2* frame)
{
  USHORT  pos;
  USHORT  len;
  T_DLC*  dlc;
  ULONG   forward_value;
  USHORT  search_pos;
  USHORT  search_len;
  UBYTE   search_command;
  BOOL    search_found;
  BOOL    search_whole_msg;
  USHORT  i;

  TRACE_FUNCTION( "ker_analyze_frame_info_response" );

  /*
   * check for correct message structure:
   * - minimal message length == 2 octets
   * - frame must end with the last message
   */
  pos = UART_OFFSET_INFO;
  while(pos < frame->len)
  {
    len = 0;
    if(!(frame->buffer[pos] & UART_EA))
    {
      /*
       * Type field greater than one octet
       */
      do
        len++;
      while(((pos + len) < frame->len) &&
            (!(frame->buffer[pos + len] & UART_EA)));
    }
    if(((pos + len + 1) < frame->len) &&
       (frame->buffer[pos + len + 1] & UART_EA))
    {
      len+= (frame->buffer[pos + 1] >> UART_MSG_LENGTH_POS) + 2;
      if((pos + len) > frame->len)
        /*
         * given length in length field to long
         * remove information field
         */
        frame->len = UART_OFFSET_INFO;
      else
        pos += len;
    }
    else
      /*
       * one octet length field expected, but not found
       * remove information field
       */
      frame->len = UART_OFFSET_INFO;
  }
  /*
   * parse frame info field until last octet is reached
   */
  pos = UART_OFFSET_INFO;
  while(pos < frame->len)
  {
    len = 0;
    while(!(frame->buffer[pos + len] & UART_EA))
    {
      len++;
    }
    len+= (frame->buffer[pos + 1] >> UART_MSG_LENGTH_POS) + 2;
    if(frame->buffer[pos] & UART_CR)
    {
      /*
       * command detected move to next message
       */
      pos+= len;
    }
    else
    {
      /*
       * analyze response message
       */
      switch( frame->buffer[pos] )
      {
        case UART_MSG_TYPE_CLD_R:
          /*
           * Close Down
           */
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          search_command   = UART_MSG_TYPE_CLD_C;
          forward_value    = UART_FORWARD_CLD;
          search_whole_msg = TRUE;
          break;

        case UART_MSG_TYPE_FCON_R:
          /*
           * Flow Control On
           */
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          search_command   = UART_MSG_TYPE_FCON_C;
          forward_value    = 0;
          search_whole_msg = TRUE;
          break;

        case UART_MSG_TYPE_FCOFF_R:
          /*
           * Flow Control Off
           */
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          search_command   = UART_MSG_TYPE_FCOFF_C;
          forward_value    = 0;
          search_whole_msg = TRUE;
          break;

        case UART_MSG_TYPE_MSC_R:
          /*
           * Modem Status Command
           */
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          search_command   = UART_MSG_TYPE_MSC_C;
          forward_value    = 0;
          search_whole_msg = TRUE;
          break;

        case UART_MSG_TYPE_NSC_R:
          /*
           * not supported command,
           */
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          if(len > 2)
            search_command = frame->buffer[pos + 2];
          else
            search_command = 0;
          switch(search_command)
          {
            case UART_MSG_TYPE_CLD_C:
              forward_value = UART_FORWARD_CLD;
              break;
            default:
              forward_value = 0;
              break;
          }
          search_whole_msg = FALSE;
          break;

        default:
          TRACE_ERROR( "Error in ker_analyze_frame_info_response: \
                        Unsupported message type received");
          dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
          search_command   = 0;
          forward_value    = 0;
          search_whole_msg = TRUE;
          break;
      }
      /*
       * search and remove command message
       */
      if(dlc->last_command NEQ NULL)
      {
        search_pos   = UART_OFFSET_INFO;
        search_found = FALSE;
        while((search_found EQ FALSE) &&
              (ker_search_msg_type(dlc->last_command,
                                   &search_pos,
                                   search_command) EQ TRUE))
        {
          search_len   = (dlc->last_command->buffer[search_pos + 1] >>
                          UART_MSG_LENGTH_POS) + 2;
          search_found = TRUE;
          if(search_whole_msg EQ TRUE)
          {
            /*
             * check whole message
             */
            for(i=1; i < search_len; i++)
            {
              if(dlc->last_command->buffer[search_pos + i] NEQ
                             frame->buffer[pos + i])
                search_found = FALSE;
            }
          }
          if(search_found EQ TRUE)
          {
            /*
             * corresponding command message found
             * remove it
             */
            if(dlc->last_command->len > (search_pos + search_len))
            {
              memmove(&dlc->last_command->buffer[search_pos],
                      &dlc->last_command->buffer[search_pos + search_len],
                      dlc->last_command->len - search_pos - search_len);
              dlc->last_command->len-= search_len;
            }
            else
              dlc->last_command->len = search_pos;
            /*
             * set retransmissions to zero and
             * set forward parameter
             */
            dlc->retransmissions = 0;
            *forward            |= forward_value;
          }
          else
          {
            search_pos+= search_len;
          }
        }
      }
      /*
       * remove resonse message
       */
      if(frame->len > (pos + len))
      {
        memmove(&frame->buffer[pos],
                &frame->buffer[pos + len],
                frame->len - pos - len);
        frame->len-= len;
      }
      else
        frame->len = pos;
    }
  }
} /* ker_analyze_frame_info_response() */



/*
+------------------------------------------------------------------------------
| Function    : ker_mux_dlc_release
+------------------------------------------------------------------------------
| Description : This function closes one open multiplexer channel
|
| Parameters  : dlc_instance - instance of dlc to release
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_dlc_release (UBYTE dlc_instance)
{
  T_DLC*  dlc;
  UBYTE   dlci;

  TRACE_FUNCTION( "ker_mux_dlc_release" );

  /*
   * set dlc values
   */
  dlc  = &uart_data->dlc_table[dlc_instance];
  dlci = dlc->dlci;
  /*
   * stop timer if this was the last running,
   * free copy of last command frame
   */
  if(dlc->last_command NEQ NULL)
  {
    if(dlc->last_command->buffer[UART_OFFSET_CONTROL] EQ
        UART_UIH_CONTROL_FRAME)
    {
      uart_data->ker.nr_t2--;
      if( uart_data->ker.nr_t2 EQ 0 )
        sig_ker_rt_stop_t2_req();
    }
    else
    {
      uart_data->ker.nr_t1--;
      if( uart_data->ker.nr_t1 EQ 0 )
        sig_ker_rt_stop_t1_req();
    }
    MFREE_DESC2(dlc->last_command);
    dlc->last_command = NULL;
  }
  /*
   * set connection state
   */
  dlc->connection_state = UART_CONNECTION_DEAD;
  /*
   * remove DLC instance
   */
  if(dlc->next_command NEQ NULL)
  {
    MFREE_DESC2(dlc->next_command);
    dlc->next_command = NULL;
  }
  uart_data->dlc_instance[dlci] = UART_EMPTY_INSTANCE;
  dlc->dlci                     = UART_DLCI_INVALID;
  /*
   * close DTI connection
   */
  uart_data->drx = dlc->drx;
  uart_data->dtx = dlc->dtx;
  sig_ker_drx_dead_mode_req();
  sig_ker_dtx_dead_mode_req();
  if(dlc->dti_state NEQ DTI_CLOSED)
  {
    dti_close(uart_hDTI, uart_data->device,
              UART_DTI_UP_INTERFACE, dlc_instance, FALSE);
    dlc->dti_state = DTI_CLOSED;
  }
} /* ker_mux_dlc_release() */



/*
+------------------------------------------------------------------------------
| Function    : ker_mux_close_down
+------------------------------------------------------------------------------
| Description : This function closes all currently open multiplexer channels.
|
| Parameters  : no parameter
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_close_down ()
{
  UBYTE   i;

  TRACE_FUNCTION( "ker_mux_close_down" );

  /*
   * close all channels
   */
  for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
  {
    /*
     * set dlc values
     */
    if(uart_data->dlc_table[i].dlci NEQ UART_DLCI_INVALID)
      ker_mux_dlc_release(i);
  }
  /*
   * stop timers
   */
  uart_data->ker.nr_t1 = 0;
  sig_ker_rt_stop_t1_req();
  uart_data->ker.nr_t2 = 0;
  sig_ker_rt_stop_t2_req();
  sig_ker_rt_stop_t3_req();

} /* ker_mux_close_down() */

/*
+------------------------------------------------------------------------------
| Function    : ker_mux_send_frame
+------------------------------------------------------------------------------
| Description : This function is used to send out a frame in multiplexer mode.
|               It checks if the KER service is currently sending. If not, the
|               descriptor is put in the output queue for the TX service and
|               the service is notified that data is available. If the KER
|               service is in state sending, the descriptor is put in a second
|               queue for later processing.
|
| Parameters  : frame - descriptor with frame to send
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_send_frame (T_desc2* frame)
{
  T_desc2* desc;

  TRACE_FUNCTION( "ker_mux_send_frame" );

  if(uart_data->ker.tx_data_desc)
  {
    /*
     * currently sending, put frame in waiting queue
     */
    desc = uart_data->ker.tx_data_waiting;
    if(desc)
    {
      while(desc->next NEQ (ULONG)NULL)
        desc = (T_desc2*)desc->next;
      desc->next = (ULONG)frame;
    }
    else
      uart_data->ker.tx_data_waiting = frame;
  }
  else
  {
    /*
     * send frame immediately
     */
    uart_data->ker.tx_data_desc = frame;
    sig_ker_tx_data_available_req(uart_data->ker.tx_data_desc, 0);
  }
} /* ker_mux_send_frame() */



/*
+------------------------------------------------------------------------------
| Function    : ker_mux_send_command_frame
+------------------------------------------------------------------------------
| Description : This function is used to send out a command frame in
|               multiplexer mode.
|               It enables the response timer and saves a copy of the
|               command frame in the DLC's last_command variable so the
|               frame can be retransmitted if the timer expires.
|
| Parameters  : dlc_instance - dlc instance the command frame belongs to
|               frame        - descriptor with command frame to send
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_send_command_frame (UBYTE dlc_instance, T_desc2* frame)
{
  T_DLC*   dlc;
  T_desc2* desc;

  TRACE_FUNCTION( "ker_mux_send_command_frame" );

  dlc = &uart_data->dlc_table[dlc_instance];
  /*
   * copy frame
   */
  if(dlc->last_command NEQ NULL)
  {
    /*
     * currently sending, put command frame in waiting queue
     */
    desc = dlc->next_command;
    if(desc)
    {
      while(desc->next NEQ (ULONG)NULL)
        desc = (T_desc2*)desc->next;
      desc->next = (ULONG)frame;
    }
    else
      dlc->next_command = frame;
  }
  else
  {
    MALLOC(dlc->last_command, (USHORT)(sizeof( T_desc2 ) - 1 +
                               frame->len));

    dlc->last_command->next = (ULONG)NULL;
    dlc->last_command->len  = frame->len;
    memcpy(dlc->last_command->buffer, frame->buffer, frame->len);

    /*
     * set response timer and counter
     */
    dlc->retransmissions = 0;
    if(frame->buffer[UART_OFFSET_CONTROL] EQ UART_UIH_CONTROL_FRAME)
    {
      /*
       * usual UIH Command frame
       * use T2 timer
       */
      uart_data->ker.nr_t2++;
      sig_ker_rt_start_t2_req();
    }
    else
    {
      /*
       * DISC frame
       * use T1 timer
       */
      uart_data->ker.nr_t1++;
      sig_ker_rt_start_t1_req();
    }

    /*
     * use the usual frame send function to transmit the frame
     */
    ker_mux_send_frame( frame );
  }
} /* ker_mux_send_command_frame() */



/*
+------------------------------------------------------------------------------
| Function    : ker_mux_send_line_states
+------------------------------------------------------------------------------
| Description : This function is used to send out a frame in multiplexer mode.
|               It creates an UIH frame with MSC command and includes new line
|               states.
|
| Parameters  : dlc_instance - instance of DLC
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_send_line_states(UBYTE dlc_instance)
{
  T_DLC*   dlc;
  T_desc2* frame;
  USHORT   pos;
  ULONG    line_states;

  TRACE_FUNCTION( "ker_mux_send_line_states" );

  dlc         = &uart_data->dlc_table[dlc_instance];
  line_states = dlc->lines;
  /*
   * allocate memory
   */
  MALLOC(frame, (USHORT)(sizeof( T_desc2 ) - 1 + uart_data->n1 + 2));
  frame->next = (ULONG)NULL;

  /*
   * fill frame
   */
    /*
     * address field
     */
  pos = 0;
  frame->buffer[pos] = (UART_DLCI_CONTROL << UART_DLCI_POS) | UART_EA;
  pos++;
    /*
     * control field
     */
  frame->buffer[pos] = UART_UIH_CONTROL_FRAME;
  pos++;
    /*
     * type field
     */
  frame->buffer[pos] = UART_MSG_TYPE_MSC_C;
  pos++;
    /*
     * length field
     */
  if(line_states & UART_BRK_TX_MASK)
    /*
     * length 3 with break field
     */
    frame->buffer[pos] = (3 << UART_MSG_LENGTH_POS) | UART_EA;
  else
    /*
     * length 2 without break field
     */
    frame->buffer[pos] = (2 << UART_MSG_LENGTH_POS) | UART_EA;
  pos++;
    /*
     * DLCI field
     */
  frame->buffer[pos] = (dlc->dlci << UART_DLCI_POS) | UART_CR | UART_EA;
  pos++;
    /*
     * V.24 signals
     */
  frame->buffer[pos] = UART_EA;
  if(!(line_states & UART_DCD_MASK))
    frame->buffer[pos] |= UART_MSC_DV_MASK;

  if(line_states & UART_RI_MASK)
    frame->buffer[pos] |= UART_MSC_IC_MASK;

  if(!(line_states & UART_CTS_MASK))
    frame->buffer[pos] |= UART_MSC_RTR_MASK;

  if(!(line_states & UART_DSR_MASK))
    frame->buffer[pos] |= UART_MSC_RTC_MASK;

  if(line_states & UART_FC_TX_MASK)
    frame->buffer[pos] |= UART_MSC_FC_MASK;

  pos++;

  /*
   * break signal
   */
  if(line_states & UART_BRK_TX_MASK)
  {
    frame->buffer[pos] = (((UBYTE)((line_states & UART_BRKLEN_TX_MASK) >>
                                                  UART_BRKLEN_TX_POS)) <<
                                                  UART_MSC_BRKLEN_POS) |
                                                  UART_MSC_BRK_MASK |
                                                  UART_EA;
    pos++;
    /*
     * break sent, so clear break flag
     */
    dlc->lines&= ~UART_BRK_TX_MASK;
  }


  /*
   * send frame
   */
  frame->len = pos;
  ker_mux_send_command_frame(UART_CONTROL_INSTANCE, frame);

} /* ker_mux_send_line_states() */



/*
+------------------------------------------------------------------------------
| Function    : ker_mux_send_close_down
+------------------------------------------------------------------------------
| Description : This function is used to send out a frame in multiplexer mode.
|               It creates an UIH frame with CLD command
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_mux_send_close_down()
{
  T_desc2* frame;
  USHORT   pos;

  TRACE_FUNCTION( "ker_mux_send_close_down" );

  /*
   * allocate memory
   */
  MALLOC(frame, (USHORT)(sizeof( T_desc2 ) - 1 + uart_data->n1 + 2));
  frame->next = (ULONG)NULL;

  /*
   * fill frame
   */
    /*
     * address field
     */
  pos = 0;
  frame->buffer[pos] = (UART_DLCI_CONTROL << UART_DLCI_POS) | UART_EA;
  pos++;
    /*
     * control field
     */
  frame->buffer[pos] = UART_UIH_CONTROL_FRAME;
  pos++;
    /*
     * type field
     */
  frame->buffer[pos] = UART_MSG_TYPE_CLD_C;
  pos++;
    /*
     * length field
     */
  frame->buffer[pos] = UART_EA;
  pos++;

  /*
   * send frame
   */
  frame->len = pos;
  frame->size = pos;
  frame->offset = 0;
  ker_mux_send_command_frame(UART_CONTROL_INSTANCE, frame);
} /* ker_mux_send_close_down() */



/*
+------------------------------------------------------------------------------
| Function    : ker_send_disc_frame
+------------------------------------------------------------------------------
| Description : This function is used to send out a frame in multiplexer mode.
|               It creates an DISC frame and sends it.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_send_disc_frame(UBYTE dlci)
{
  T_desc2* frame;
  USHORT   pos;

  TRACE_FUNCTION( "ker_send_disc_frame" );

  /*
   * allocate memory
   */
  MALLOC(frame, (USHORT)(sizeof(T_desc2) - 1 + 2));
  frame->next = (ULONG)NULL;

  /*
   * fill frame
   */
    /*
     * address field
     */
  pos = 0;
  frame->buffer[pos] = (dlci << UART_DLCI_POS) | UART_EA;
  pos++;
    /*
     * control field
     */
  frame->buffer[pos] = UART_DISC_FRAME;
  pos++;

  /*
   * send frame
   */
  frame->len = pos;
  ker_mux_send_command_frame(uart_data->dlc_instance[dlci], frame);

} /* ker_send_disc_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_sabm_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received SABM frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_sabm_frame(ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  UBYTE   dlc_instance;
  UBYTE   i;

  TRACE_FUNCTION( "ker_receive_sabm_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * analyze message responses
   */
  ker_analyze_frame_info_response(forward, frame);
  /*
   * check whether frame for an existing channel
   */
  if(dlc_instance != UART_EMPTY_INSTANCE)
  {
    /*
     * set DLC to this channel
     */
    dlc = &uart_data->dlc_table[dlc_instance];
    switch(dlc->connection_state)
    {
      case UART_CONNECTION_OPEN:
        /*
         * send UA frame
         */
        ker_analyze_frame_info_command(forward, frame);
        frame->buffer[UART_OFFSET_CONTROL] = UART_UA_FRAME;
        *forward                          |= UART_FORWARD_RESPONSE;
        break;

      case UART_CONNECTION_DISC_SENT:
        /*
         * send DM frame
         */
        ker_analyze_frame_info_command(forward, frame);
        frame->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
        *forward                          |= UART_FORWARD_RESPONSE;
        break;

      case UART_CONNECTION_SABM_RCVD:
        break;

      default:
        TRACE_ERROR( "DLC CONNECTION_STATE unexpected" );
        break;
    }
  }
  else
  {
    ker_analyze_frame_info_command(forward, frame);
    if( dlci EQ UART_DLCI_CONTROL )
    {
      /*
       * this is a SABM frame for the control channel,
       * therefore the appropriate instance is UART_CONTROL_INSTANCE
       */
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      if(dlc->dlci EQ UART_DLCI_INVALID)
      {
        i = UART_CONTROL_INSTANCE;
        /*
         * mark DLC as opened
         */
        dlc->connection_state = UART_CONNECTION_SABM_RCVD;
        dlc->dlci             = dlci;

        /*
         * create UA response frame
         */
        frame->buffer[UART_OFFSET_CONTROL] = UART_UA_FRAME;
        *forward                          |= UART_FORWARD_SABM;
      }
      else
      {
        i = UART_MAX_NUMBER_OF_CHANNELS + 1;
      }
    }
    else
    {
      /*
       * if new, check whether there is a free channel left
       */
      for(i=0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++ )
      {
        if(uart_data->dlc_table[i].dlci EQ UART_DLCI_INVALID)
        {
          dlc = &uart_data->dlc_table[i];
          /*
           * mark DLC as opened
           */
          dlc->connection_state = UART_CONNECTION_SABM_RCVD;
          dlc->dlci             = dlci;

          /*
           * create UA response frame
           */
          frame->buffer[UART_OFFSET_CONTROL] = UART_UA_FRAME;
          *forward                          |= UART_FORWARD_SABM;
          break;
        }
      }
    }
    if( i > UART_MAX_NUMBER_OF_CHANNELS )
    {
      /*
       * no free channel found, return DM frame
       */
      frame->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
    }
    *forward|= UART_FORWARD_RESPONSE;
  }
} /* ker_receive_sabm_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_ua_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received UA frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_ua_frame(ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  UBYTE   dlc_instance;

  TRACE_FUNCTION( "ker_receive_ua_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * analyze message responses
   */
  ker_analyze_frame_info_response(forward, frame);
  /*
   * check whether frame for an existing channel
   */
  if(dlc_instance != UART_EMPTY_INSTANCE)
  {
    /*
     * set DLC to this channel
     */
    dlc = &uart_data->dlc_table[dlc_instance];
    switch(dlc->connection_state)
    {
      case UART_CONNECTION_DISC_SENT:
        /*
         * remove DISC frame
         */
        MFREE_DESC2(dlc->last_command);
        dlc->last_command = NULL;
        uart_data->ker.nr_t1--;
        if( uart_data->ker.nr_t1 EQ 0 )
          sig_ker_rt_stop_t1_req();
        /*
         * mark channel as closed
         */
        dlc->connection_state = UART_CONNECTION_DEAD;
        *forward             |= UART_FORWARD_DLC_RELEASE;
        break;

      case UART_CONNECTION_SABM_RCVD:
      case UART_CONNECTION_OPEN:
        break;

      default:
        TRACE_ERROR( "DLC CONNECTION_STATE unexpected" );
        break;
    }
  }
} /* ker_receive_ua_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_dm_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received DM frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_dm_frame(ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  UBYTE   dlc_instance;

  TRACE_FUNCTION( "ker_receive_dm_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * analyze message responses
   */
  ker_analyze_frame_info_response(forward, frame);
  /*
   * check whether frame for an existing channel
   * and not for Control channel
   */
  if((dlc_instance NEQ UART_EMPTY_INSTANCE) &&
     (dlci NEQ UART_DLCI_CONTROL))
  {
    /*
     * set DLC to this channel
     */
    dlc = &uart_data->dlc_table[dlc_instance];
    switch(dlc->connection_state)
    {
      case UART_CONNECTION_DISC_SENT:
        /*
         * remove DISC frame
         */
        MFREE_DESC2(dlc->last_command);
        dlc->last_command = NULL;
        uart_data->ker.nr_t1--;
        if( uart_data->ker.nr_t1 EQ 0 )
          sig_ker_rt_stop_t1_req();
        /* fall through */
      case UART_CONNECTION_OPEN:
        /*
         * mark channel as closed
         */
        dlc->connection_state = UART_CONNECTION_DEAD;
        *forward             |= UART_FORWARD_DLC_RELEASE;
        break;

      case UART_CONNECTION_SABM_RCVD:
        break;

      default:
        TRACE_ERROR( "DLC CONNECTION_STATE unexpected" );
        break;
    }
  }
} /* ker_receive_dm_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_disc_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received DISC frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_disc_frame(ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  UBYTE   dlc_instance;

  TRACE_FUNCTION( "ker_receive_disc_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * analyze messages
   */
  ker_analyze_frame_info_response(forward, frame);
  ker_analyze_frame_info_command(forward, frame);
  /*
   * check whether frame for an existing channel
   */
  if(dlc_instance NEQ UART_EMPTY_INSTANCE)
  {
    /*
     * set DLC to this channel
     */
    dlc = &uart_data->dlc_table[dlc_instance];
    if(dlci EQ UART_DLCI_CONTROL)
    {
      /*
       * send UA frame
       */
      frame->buffer[UART_OFFSET_CONTROL] = UART_UA_FRAME;
      *forward                          |= UART_FORWARD_CLD;
    }
    else
    {
      switch(dlc->connection_state)
      {
        case UART_CONNECTION_DISC_SENT:
          /*
           * remove DISC frame
           */
          MFREE_DESC2(dlc->last_command);
          dlc->last_command = NULL;
          uart_data->ker.nr_t1--;
          if( uart_data->ker.nr_t1 EQ 0 )
            sig_ker_rt_stop_t1_req();
          /* fall through */
        case UART_CONNECTION_SABM_RCVD:
        case UART_CONNECTION_OPEN:
          /*
           * mark channel as closed
           */
          dlc->connection_state = UART_CONNECTION_DEAD;
          /*
           * send UA frame
           */
          frame->buffer[UART_OFFSET_CONTROL] = UART_UA_FRAME;
          *forward                          |= UART_FORWARD_DLC_RELEASE;
          break;

        default:
          TRACE_ERROR( "DLC CONNECTION_STATE unexpected" );
          break;
      }
    }
  }
  else
  {
    /*
     * send DM frame
     */
    frame->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
  }
  *forward|= UART_FORWARD_RESPONSE;
} /* ker_receive_disc_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_uih_control_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received UIH Control frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_uih_control_frame(ULONG* forward, T_desc2* frame)
{
  T_DLC*  dlc;
  UBYTE   dlci;
  UBYTE   dlc_instance;

  TRACE_FUNCTION( "ker_receive_uih_control_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * analyze message responses
   */
  ker_analyze_frame_info_response(forward, frame);
  /*
   * check whether frame for an existing channel
   */
  if(dlc_instance NEQ UART_EMPTY_INSTANCE)
  {
    /*
     * set DLC to this channel
     */
    dlc = &uart_data->dlc_table[dlc_instance];
    /*
     * check whether it is an command frame
     * discard frame if it is a response frame
     */
    if(frame->buffer[UART_OFFSET_ADDRESS] & UART_CR)
    {
      switch(dlc->connection_state)
      {
        case UART_CONNECTION_OPEN:
          /*
           * send UIH response frame
           */
          ker_analyze_frame_info_command(forward, frame);
          *forward|= UART_FORWARD_RESPONSE;
          break;

        case UART_CONNECTION_DISC_SENT:
          /*
           * send DM frame
           */
          ker_analyze_frame_info_command(forward, frame);
          frame->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
          *forward                          |= UART_FORWARD_RESPONSE;
          break;

        case UART_CONNECTION_SABM_RCVD:
          break;

        default:
          TRACE_ERROR( "DLC CONNECTION_STATE unexpected" );
          break;
      }
    }
  }
  else
  {
    /*
     * send DM frame
     */
    ker_analyze_frame_info_command(forward, frame);
    frame->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME;
    *forward                          |= UART_FORWARD_RESPONSE;
  }
} /* ker_receive_uih_control_frame */



/*
+------------------------------------------------------------------------------
| Function    : ker_receive_uih_data_frame
+------------------------------------------------------------------------------
| Description : This function analyzes received UIH Data frames.
|
| Parameters  : forward - result of analysis
|               frame   - frame to analyze
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_receive_uih_data_frame(ULONG* forward, T_desc2* frame)
{
  UBYTE dlci;
  UBYTE dlc_instance;

  TRACE_FUNCTION( "ker_receive_uih_data_frame" );

  dlci         = frame->buffer[UART_OFFSET_ADDRESS] >> UART_DLCI_POS;
  dlc_instance = uart_data->dlc_instance[dlci];

  /*
   * check whether frame for a not existing channel
   * discard packet if it is for an extisting channel
   */
  if(dlc_instance EQ UART_EMPTY_INSTANCE)
  {
    /*
     * send DM frame
     * shorten information field
     */
    frame->buffer[UART_OFFSET_CONTROL] = UART_DM_DATA_FRAME;
    frame->len                         = UART_OFFSET_INFO;
    *forward                          |= UART_FORWARD_RESPONSE;
  }
} /* ker_receive_uih_data_frame */

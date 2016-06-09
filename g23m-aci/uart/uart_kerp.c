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
|             the SDL-documentation (KER-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_KERP_C
#define UART_KERP_C
#endif /* !UART_KERP_C */

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

#include "uart_kerf.h"  /* to get ker functions */
#include "uart_kers.h"  /* to get ker signals */
#include "uart_drxs.h"  /* to get drx signals */
#include "uart_dtxs.h"  /* to get dtx signals */
#include "uart_rts.h"   /* to get  rt signals */
#ifdef FF_MULTI_PORT
#include "uart_prxs.h"  /* to get  rx signals */
#include "uart_ptxs.h"  /* to get  tx signals */
#else /* FF_MULTI_PORT */
#include "uart_rxs.h"   /* to get  rx signals */
#include "uart_txs.h"   /* to get  tx signals */
#endif /* FF_MULTI_PORT */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_parameters_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_PARAMETERS_REQ
|
| Parameters  : *uart_parameters_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_parameters_req (
              T_UART_PARAMETERS_REQ *uart_parameters_req)
{
  UBYTE   i;
  T_DLC*  dlc;

  TRACE_FUNCTION( "ker_uart_parameters_req" );

#ifdef UART_RANGE_CHECK
  if(uart_parameters_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_parameters_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_parameters_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_parameters_req=%08x is not allocated",
                    uart_parameters_req);
  }
  else if(uart_parameters_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_parameters_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_parameters_req->device];

#ifdef FF_MULTI_PORT
  /*
   * set new baud rate
   */
  switch (uart_parameters_req->comPar.speed)
  {
    case UART_IO_SPEED_AUTO:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_AUTO;
      break;
    case UART_IO_SPEED_75:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_75;
      break;
    case UART_IO_SPEED_150:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_150;
      break;
    case UART_IO_SPEED_300:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_300;
      break;
    case UART_IO_SPEED_600:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_600;
      break;
    case UART_IO_SPEED_1200:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_1200;
      break;
    case UART_IO_SPEED_2400:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_2400;
      break;
    case UART_IO_SPEED_4800:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_4800;
      break;
    case UART_IO_SPEED_7200:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_7200;
      break;
    case UART_IO_SPEED_9600:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_9600;
      break;
    case UART_IO_SPEED_14400:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_14400;
      break;
    case UART_IO_SPEED_19200:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_19200;
      break;
    case UART_IO_SPEED_28800:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_28800;
      break;
    case UART_IO_SPEED_33900:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_33900;
      break;
    case UART_IO_SPEED_38400:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_38400;
      break;
    case UART_IO_SPEED_57600:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_57600;
      break;
    case UART_IO_SPEED_115200:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_115200;
      break;
    case UART_IO_SPEED_203125:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_203125;
      break;
    case UART_IO_SPEED_406250:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_406250;
      break;
    case UART_IO_SPEED_812500:
      uart_data->ker.act_dcb.Baud = GSI_BAUD_812500;
      break;
    case UART_IO_SPEED_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: baudrate unexpected" );
      break;
  }

  /*
   * set new bits per character
   */
  switch (uart_parameters_req->comPar.bpc)
  {
    case UART_IO_BPC_7:
      uart_data->ker.act_dcb.DataBits = GSI_CHAR7;
      break;
    case UART_IO_BPC_8:
      uart_data->ker.act_dcb.DataBits = GSI_CHAR8;
      break;
    case UART_IO_BPC_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: bpc unexpected" );
      break;
  }

  /*
   * set new stop bit
   */
  switch (uart_parameters_req->comPar.nsb)
  {
    case UART_IO_SB_1:
      uart_data->ker.act_dcb.StopBits = GSI_STOP1;
      break;
    case UART_IO_SB_2:
      uart_data->ker.act_dcb.StopBits = GSI_STOP2;
      break;
    case UART_IO_SB_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: stop bits unexpected" );
      break;
  }

  /*
   * set new parity
   */
  switch (uart_parameters_req->comPar.parity)
  {
    case UART_IO_PA_NONE:
      uart_data->ker.act_dcb.Parity = GSI_PARITYNO;
      break;
    case UART_IO_PA_EVEN:
      uart_data->ker.act_dcb.Parity = GSI_PARITYEVEN;
      break;
    case UART_IO_PA_ODD:
      uart_data->ker.act_dcb.Parity = GSI_PARITYODD;
      break;
    case UART_IO_PA_SPACE:
      uart_data->ker.act_dcb.Parity = GSI_PARITYSPACE;
      break;
    case UART_IO_PA_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: parity unexpected" );
      break;
  }

  /*
   * set new RX flow control
   */
  switch (uart_parameters_req->comPar.flow_rx)
  {
    case UART_IO_FC_RX_NONE:
      uart_data->ker.act_dcb.RxFlowControl = GSI_FLOWNO;
      break;
    case UART_IO_FC_RX_RTS:
      uart_data->ker.act_dcb.RxFlowControl = GSI_FLOWHW;
      break;
    case UART_IO_FC_RX_XOFF:
      uart_data->ker.act_dcb.RxFlowControl = GSI_FLOWSW;
      break;
    case UART_IO_FC_RX_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: RX flow control unexpected" );
      break;
  }

  /*
   * set new TX flow control
   */
  switch (uart_parameters_req->comPar.flow_tx)
  {
    case UART_IO_FC_TX_NONE:
      uart_data->ker.act_dcb.TxFlowControl = GSI_FLOWNO;
      break;
    case UART_IO_FC_TX_RTS:
      uart_data->ker.act_dcb.TxFlowControl = GSI_FLOWHW;
      break;
    case UART_IO_FC_TX_XOFF:
      uart_data->ker.act_dcb.TxFlowControl = GSI_FLOWSW;
      break;
    case UART_IO_FC_TX_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: TX flow control unexpected" );
      break;
  }

  /*
   * set new XON / XOFF values
   */
  if(uart_parameters_req->comPar.xon_valid EQ UART_IO_XON_VALID)
    uart_data->ker.act_dcb.XON = uart_parameters_req->comPar.xon;
  if(uart_parameters_req->comPar.xoff_valid EQ UART_IO_XOFF_VALID)
    uart_data->ker.act_dcb.XOFF = uart_parameters_req->comPar.xoff;
#else /* FF_MULTI_PORT */
  /*
   * set new baud rate
   */
  switch (uart_parameters_req->comPar.speed)
  {
    case UART_IO_SPEED_AUTO:
      uart_data->ker.act_br = UF_BAUD_AUTO;
      break;
    case UART_IO_SPEED_75:
      uart_data->ker.act_br = UF_BAUD_75;
      break;
    case UART_IO_SPEED_150:
      uart_data->ker.act_br = UF_BAUD_150;
      break;
    case UART_IO_SPEED_300:
      uart_data->ker.act_br = UF_BAUD_300;
      break;
    case UART_IO_SPEED_600:
      uart_data->ker.act_br = UF_BAUD_600;
      break;
    case UART_IO_SPEED_1200:
      uart_data->ker.act_br = UF_BAUD_1200;
      break;
    case UART_IO_SPEED_2400:
      uart_data->ker.act_br = UF_BAUD_2400;
      break;
    case UART_IO_SPEED_4800:
      uart_data->ker.act_br = UF_BAUD_4800;
      break;
    case UART_IO_SPEED_7200:
      uart_data->ker.act_br = UF_BAUD_7200;
      break;
    case UART_IO_SPEED_9600:
      uart_data->ker.act_br = UF_BAUD_9600;
      break;
    case UART_IO_SPEED_14400:
      uart_data->ker.act_br = UF_BAUD_14400;
      break;
    case UART_IO_SPEED_19200:
      uart_data->ker.act_br = UF_BAUD_19200;
      break;
    case UART_IO_SPEED_28800:
      uart_data->ker.act_br = UF_BAUD_28800;
      break;
    case UART_IO_SPEED_33900:
      uart_data->ker.act_br = UF_BAUD_33900;
      break;
    case UART_IO_SPEED_38400:
      uart_data->ker.act_br = UF_BAUD_38400;
      break;
    case UART_IO_SPEED_57600:
      uart_data->ker.act_br = UF_BAUD_57600;
      break;
    case UART_IO_SPEED_115200:
      uart_data->ker.act_br = UF_BAUD_115200;
      break;
    case UART_IO_SPEED_203125:
      uart_data->ker.act_br = UF_BAUD_203125;
      break;
    case UART_IO_SPEED_406250:
      uart_data->ker.act_br = UF_BAUD_406250;
      break;
    case UART_IO_SPEED_812500:
      uart_data->ker.act_br = UF_BAUD_812500;
      break;
    case UART_IO_SPEED_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: baudrate unexpected" );
      break;
  }

  /*
   * set new RX flow control
   */
  switch (uart_parameters_req->comPar.flow_rx)
  {
    case UART_IO_FC_RX_NONE:
      uart_data->ker.act_fc_rx = fc_none;
      break;
    case UART_IO_FC_RX_RTS:
      uart_data->ker.act_fc_rx = fc_rts;
      break;
    case UART_IO_FC_RX_XOFF:
      uart_data->ker.act_fc_rx = fc_xoff;
      break;
    case UART_IO_FC_RX_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: RX flow control unexpected" );
      break;
  }

  /*
   * set new TX flow control
   */
  switch (uart_parameters_req->comPar.flow_tx)
  {
    case UART_IO_FC_TX_NONE:
      uart_data->ker.act_fc_tx = fc_none;
      break;
    case UART_IO_FC_TX_RTS:
      uart_data->ker.act_fc_tx = fc_rts;
      break;
    case UART_IO_FC_TX_XOFF:
      uart_data->ker.act_fc_tx = fc_xoff;
      break;
    case UART_IO_FC_TX_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: TX flow control unexpected" );
      break;
  }

  /*
   * set new bits per character
   */
  switch (uart_parameters_req->comPar.bpc)
  {
    case UART_IO_BPC_7:
      uart_data->ker.act_bpc = bpc_7;
      break;
    case UART_IO_BPC_8:
      uart_data->ker.act_bpc = bpc_8;
      break;
    case UART_IO_BPC_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: bpc unexpected" );
      break;
  }

  /*
   * set new stop bit
   */
  switch (uart_parameters_req->comPar.nsb)
  {
    case UART_IO_SB_1:
      uart_data->ker.act_sb = sb_1;
      break;
    case UART_IO_SB_2:
      uart_data->ker.act_sb = sb_2;
      break;
    case UART_IO_SB_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: stop bits unexpected" );
      break;
  }

  /*
   * set new parity
   */
  switch (uart_parameters_req->comPar.parity)
  {
    case UART_IO_PA_NONE:
      uart_data->ker.act_par = pa_none;
      break;
    case UART_IO_PA_EVEN:
      uart_data->ker.act_par = pa_even;
      break;
    case UART_IO_PA_ODD:
      uart_data->ker.act_par = pa_odd;
      break;
    case UART_IO_PA_SPACE:
      uart_data->ker.act_par = pa_space;
      break;
    case UART_IO_PA_UNDEF: /* unchanged */
      break;
    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ: parity unexpected" );
      break;
  }

  /*
   * set new XON / XOFF values
   */
  if(uart_parameters_req->comPar.xon_valid EQ UART_IO_XON_VALID)
    uart_data->ker.act_xon = uart_parameters_req->comPar.xon;
  if(uart_parameters_req->comPar.xoff_valid EQ UART_IO_XOFF_VALID)
    uart_data->ker.act_xoff = uart_parameters_req->comPar.xoff;
#endif /* FF_MULTI_PORT */

  /*
   * set new escape detection values
   */
  if(uart_parameters_req->comPar.esc_valid EQ UART_IO_ESC_VALID)
  {
#ifdef FF_MULTI_PORT
    uart_data->ker.act_dcb.EscChar     = uart_parameters_req->comPar.esc_char;
    uart_data->ker.act_dcb.GuardPeriod = uart_parameters_req->comPar.esc_gp;
#else /* FF_MULTI_PORT */
    uart_data->ker.act_ec = uart_parameters_req->comPar.esc_char;
    uart_data->ker.act_gp = uart_parameters_req->comPar.esc_gp;
#endif /* FF_MULTI_PORT */
  }

  /*
   * free the received primitive
   */
  PFREE(uart_parameters_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
      /*
       * flush all still to send data
       */
      uart_data->ker.received_prim|= UART_PARAMETERS_REQ_MASK;
      uart_data->ker.flush_state   = UART_KER_DRX_FLUSH;
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      dlc->flushed   = FALSE;
      uart_data->drx = dlc->drx;
      sig_ker_drx_flush_req();
      break;

    case KER_MUX:
    case KER_MUX_ESTABLISH:
    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      /*
       * flush all still to send data
       */
      uart_data->ker.received_prim|= UART_PARAMETERS_REQ_MASK;
      uart_data->ker.flush_state   = UART_KER_DRX_FLUSH;
      /*
       * mark all to flush DLCs
       */
      for(i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        if((i EQ UART_CONTROL_INSTANCE) ||
           (dlc->connection_state EQ UART_CONNECTION_OPEN))
        {
          dlc->flushed = FALSE;
        }
      }
      /*
       * initiate flush in all to flushed DLCs
       */
      for(i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        if(dlc->flushed NEQ TRUE)
        {
          if(i EQ UART_CONTROL_INSTANCE)
          {
            if(uart_data->ker.tx_data_desc EQ NULL)
              sig_any_ker_flushed_ind(UART_CONTROL_INSTANCE);
          }
          else
          {
            uart_data->drx = dlc->drx;
            sig_ker_drx_flush_req();
          }
        }
      }
      break;

    case KER_DEAD:
      /*
       * set new UART parameters
       */
      ker_setupUart();

#ifdef FF_MULTI_PORT
      uart_data->ker.act_ec = uart_data->ker.act_dcb.EscChar;
      uart_data->ker.act_gp = uart_data->ker.act_dcb.GuardPeriod;
#endif /* FF_MULTI_PORT */

      /*
       * send confirm primitive
       */
      {
        PALLOC (uart_parameters_cnf, UART_PARAMETERS_CNF);
        uart_parameters_cnf->device = uart_data->device;
        PSEND (hCommMMI, uart_parameters_cnf);
      }
      break;

    default:
      TRACE_ERROR( "UART_PARAMETERS_REQ unexpected" );
      break;
  }
} /* ker_uart_parameters_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_dti_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_DTI_REQ
|
| Parameters  : *uart_dti_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_dti_req ( T_UART_DTI_REQ *uart_dti_req )
{
  T_DLC* dlc;
  UBYTE  dlc_instance; /* dlc_instance is used as the channel number for dtilib */
#ifdef DTILIB
  U8          isOK;
#else  /* DTILIB */
  T_HANDLE    hCommUPLINK;
#endif /* DTILIB */

  TRACE_FUNCTION( "ker_uart_dti_req" );

#ifdef UART_RANGE_CHECK
  if(uart_dti_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_dti_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_dti_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_dti_req=%08x is not allocated",
                    uart_dti_req);
  }
  else if(uart_dti_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_dti_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_dti_req->device];

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX:
    case KER_MUX_DLC_CLOSING:
      dlc_instance = uart_data->dlc_instance[uart_dti_req->dlci];
      break;

    default:
      dlc_instance = UART_CONTROL_INSTANCE;
      break;
  }

  dlc = &uart_data->dlc_table[dlc_instance];
  /*
   * close communication channel
   * if it is already closed, silently discard primitive
   */
  if(dlc->dti_state NEQ DTI_CLOSED)
  {
    dti_close(
      uart_hDTI,
      uart_data->device,
      UART_DTI_UP_INTERFACE,
      dlc_instance,
      FALSE
      );
    dlc->dti_state = DTI_CLOSED;

    /*
     * no reopen
     */
    if (uart_dti_req->dti_conn EQ UART_DISCONNECT_DTI)
    {
      PALLOC (uart_dti_cnf, UART_DTI_CNF);
      uart_data->drx = dlc->drx;
      uart_data->dtx = dlc->dtx;
      sig_ker_drx_disconnected_mode_req();
      sig_ker_dtx_disconnected_mode_req();

      /*
       * send confirm primitive
       */
      uart_dti_cnf->device = uart_data->device;
      uart_dti_cnf->dlci   = uart_data->dlc_table[dlc_instance].dlci; /* EQ dlci */
      uart_dti_cnf->dti_conn = UART_DISCONNECT_DTI;
      PSEND (hCommMMI, uart_dti_cnf);

      return;
    }
  }

#ifdef DTILIB
  if( GET_STATE( UART_SERVICE_KER ) EQ KER_DEAD )
  {
    /*
     * set dlc values - this has to happen before
     * the call of dti_open, because within that call
     * a callback-function may be called which makes
     * use of them..
     */
    dlc           = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
    dlc->drx      = &uart_data->drx_base[0];
    dlc->dtx      = &uart_data->dtx_base[0];
    dlc->dlci     = 0;
    uart_data->dlc_instance[0] = UART_CONTROL_INSTANCE;
    dlc->priority = 0;
    dlc->lines    = 0;
  }

  /*
   * set DTI connection
   */
  dlc->dti_state    = DTI_SETUP;

  uart_data->drx   = dlc->drx;
  uart_data->dtx   = dlc->dtx;
#endif /* DTILIB */

#ifdef _SIMULATION_
  /*
   * !!! Problem with entity_name which is a pointer in new SAP
   * !!! therefore we set the peer entity simply to "MMI".
   * !!! This should be corrected in order to allow test case simulations
   * !!! with other peer entities.
   */

#ifdef DTILIB
  isOK = dti_open(
    uart_hDTI,
    uart_data->device,
    UART_DTI_UP_INTERFACE,
    dlc_instance,
    UART_UPLINK_QUEUE_SIZE,
    uart_dti_req->direction,
    DTI_QUEUE_UNUSED,
    DTI_VERSION_10,
    "MMI",
    uart_dti_req->link_id
    );
#else  /* DTILIB */
  hCommUPLINK = vsi_c_open (VSI_CALLER "MMI");
#endif /* DTILIB */
#else /* _SIMULATION_ */
  /*
   * open new communication channel
   */
#ifdef DTILIB
  isOK = dti_open(
    uart_hDTI,
    uart_data->device,
    UART_DTI_UP_INTERFACE,
    dlc_instance,
    UART_UPLINK_QUEUE_SIZE,
    uart_dti_req->direction,
    DTI_QUEUE_UNUSED,
    DTI_VERSION_10,
    (U8*)(uart_dti_req->entity_name),
    uart_dti_req->link_id
    );
#else  /* DTILIB */
  hCommUPLINK = vsi_c_open (VSI_CALLER
                            (char *)(uart_dti_req->entity_name));
#endif /* DTILIB */
#endif /* _SIMULATION_ */


#ifdef DTILIB
  if(!isOK)
  /* error?! */
  /*
   * NOTE: internal initialization of the new communication channel
   * is done in sig_dti_ker_connection_opened() when using DTILIB
   *
   * when debugging the below code, please also have a look there!!
   */
#else  /* DTILIB */
  uart_data->tui_uart = uart_dti_req->tui_uart;
  if(hCommUPLINK >= VSI_OK)
  {
    PALLOC (uart_dti_cnf, UART_DTI_CNF);
    /*
     * send confirm primitive
     */
    uart_dti_cnf->device = uart_data->device;
    uart_dti_cnf->dlci   = uart_dti_req->dlci;
    uart_dti_cnf->dti_conn = UART_CONNECT_DTI;
    PSEND (hCommMMI, uart_dti_cnf);
    /*
     * initialize new communication channel
     */
    switch( GET_STATE( UART_SERVICE_KER ) )
    {
      case KER_DEAD:
        SET_STATE( UART_SERVICE_KER, KER_READY );
        /*
         * enable UART
         */
#ifdef _SIMULATION_
        {
          /*
           * send DTI_GETDATA_REQ
           */
          PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
          dti_getdata_req->tui    = 2; /* for enable */
          dti_getdata_req->c_id   = 0;
          dti_getdata_req->op_ack = 0;
          PSEND (hCommMMI, dti_getdata_req);
        }
#else /* _SIMULATION_ */
#ifndef FF_MULTI_PORT
        if((ret=UF_Enable (uart_data->device, TRUE)) NEQ UF_OK)
        {
          TRACE_ERROR_P2("UF Driver: Can't enable UART, [%d], uart_kerp.c(%d)",
                                                                 ret,__LINE__);
        }
#endif /* !FF_MULTI_PORT */
#endif /* _SIMULATION */
        /*
         * set dlc values
         */
        dlc           = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
        dlc->drx      = &uart_data->drx_base[0];
        dlc->dtx      = &uart_data->dtx_base[0];
        dlc->dlci     = 0;
        dlc->priority = 0;
        dlc->lines    = 0;
        /*
         * set RX and TX in ready mode
         */
        sig_ker_rx_ready_mode_req();
        sig_ker_tx_ready_mode_req();
        /*
         * set DTI connection
         */
        dlc->hCommUPLINK = hCommUPLINK;
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;
        sig_ker_drx_ready_mode_req(UART_CONTROL_INSTANCE);
        sig_ker_dtx_ready_mode_req(UART_CONTROL_INSTANCE);
        sig_ker_drx_set_dti_peer_req(uart_dti_req->tui_peer,
                                     hCommUPLINK,
                                     uart_dti_req->c_id);
        sig_ker_dtx_set_dti_peer_req(uart_dti_req->tui_peer,
                                     hCommUPLINK,
                                     uart_dti_req->c_id);

        break;

      case KER_READY:
      case KER_MUX:
        dlc->hCommUPLINK = hCommUPLINK;
        uart_data->drx   = dlc->drx;
        uart_data->dtx   = dlc->dtx;
        sig_ker_drx_set_dti_peer_req(uart_dti_req->tui_peer,
                                     hCommUPLINK,
                                     uart_dti_req->c_id);
        sig_ker_dtx_set_dti_peer_req(uart_dti_req->tui_peer,
                                     hCommUPLINK,
                                     uart_dti_req->c_id);
        break;

      default:
        TRACE_ERROR( "UART_DTI_REQ unexpected" );
        break;
    }
  }
  else /* UPLINK OK   */
#endif /* DTILIB */
  {
    /*
     * send error primitive if communication channel is not opened
     */
    PALLOC (uart_error_ind, UART_ERROR_IND);
    uart_error_ind->device = uart_data->device;
    uart_error_ind->dlci   = uart_dti_req->dlci;
    uart_error_ind->error  = UART_ERROR_NO_CHANNEL;
    PSEND (hCommMMI, uart_error_ind);
  }

  /*
   * free the received primitive
   */
  PFREE(uart_dti_req);
} /* ker_uart_dti_req() */


/*
+------------------------------------------------------------------------------
| Function    : ker_uart_disable_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_DISABLE_REQ
|
| Parameters  : *uart_disable_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_disable_req ( T_UART_DISABLE_REQ *uart_disable_req )
{
  UBYTE   i;
  T_DLC*  dlc;

  TRACE_FUNCTION( "ker_uart_disable_req" );

#ifdef UART_RANGE_CHECK
  if(uart_disable_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_disable_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_disable_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_disable_req=%08x is not allocated",
                    uart_disable_req);
  }
  else if(uart_disable_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_disable_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_disable_req->device];

  /*
   * free the received primitive
   */
  PFREE(uart_disable_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
      /*
       * flush all still to send data
       */
      uart_data->ker.received_prim|= UART_DISABLE_REQ_MASK;
      uart_data->ker.flush_state   = UART_KER_DRX_FLUSH;
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      dlc->flushed   = FALSE;
      uart_data->drx = dlc->drx;
      sig_ker_drx_flush_req();
      break;

    case KER_MUX:
    case KER_MUX_ESTABLISH:
    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      /*
       * flush all still to send data
       */
      uart_data->ker.received_prim|= UART_DISABLE_REQ_MASK;
      uart_data->ker.flush_state   = UART_KER_DRX_FLUSH;
      /*
       * mark all to flush DLCs
       */
      for(i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        if((i EQ UART_CONTROL_INSTANCE) ||
           (dlc->connection_state EQ UART_CONNECTION_OPEN))
        {
          dlc->flushed = FALSE;
        }
      }
      /*
       * initiate flush in all to flushed DLCs
       */
      for(i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        if(dlc->flushed NEQ TRUE)
        {
          if(i EQ UART_CONTROL_INSTANCE)
          {
            if(uart_data->ker.tx_data_desc EQ NULL)
              sig_any_ker_flushed_ind(UART_CONTROL_INSTANCE);
          }
          else
          {
            uart_data->drx = dlc->drx;
            sig_ker_drx_flush_req();
          }
        }
      }
      break;

    case KER_DEAD:
      /*
       * send confirm primitive
       */
      {
        PALLOC (uart_disable_cnf, UART_DISABLE_CNF);
        uart_disable_cnf->device = uart_data->device;
        PSEND (hCommMMI, uart_disable_cnf);
      }
      break;

    default:
      TRACE_ERROR( "UART_DISABLE_REQ unexpected" );
      break;
  }
} /* ker_uart_disable_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_ring_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_RING_REQ
|
| Parameters  : *uart_ring_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_ring_req ( T_UART_RING_REQ *uart_ring_req )
{
  T_DLC*  dlc;

  TRACE_FUNCTION( "ker_uart_ring_req" );

#ifdef UART_RANGE_CHECK
  if(uart_ring_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_ring_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_ring_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_ring_req=%08x is not allocated",
                    uart_ring_req);
  }
  else if(uart_ring_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_ring_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_ring_req->device];

  /*
   * set DLC instance
   */
  if((GET_STATE( UART_SERVICE_KER )) EQ KER_READY)
    dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  else
    dlc = &uart_data->dlc_table[uart_data->
                      dlc_instance[uart_ring_req->dlci]];

  /*
   * store new line states
   */
  if(uart_ring_req->line_state EQ UART_LINE_ON)
  {
    TRACE_EVENT_P1("RING: on - DLCI=%d", uart_ring_req->dlci);
    dlc->lines|= UART_RI_MASK;
  }
  else
  {
    TRACE_EVENT_P1("RING: off - DLCI=%d", uart_ring_req->dlci);
    dlc->lines&= ~(UART_RI_MASK);
  }

  /*
   * free the received primitive
   */
  PFREE(uart_ring_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
    case KER_MUX:
      /*
       * flush all still to send data
       */
      dlc->received_prim        |= UART_RING_REQ_MASK;
      uart_data->ker.flush_state = UART_KER_DRX_FLUSH;
      dlc->flushed   = FALSE;
      uart_data->drx = dlc->drx;
      sig_ker_drx_flush_req();
      break;

    default:
      TRACE_ERROR( "UART_RING_REQ unexpected" );
      break;
  }
} /* ker_uart_ring_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_dcd_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_DCD_REQ
|
| Parameters  : *uart_dcd_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_dcd_req ( T_UART_DCD_REQ *uart_dcd_req )
{
  T_DLC*  dlc;

  TRACE_FUNCTION( "ker_uart_dcd_req" );

#ifdef UART_RANGE_CHECK
  if(uart_dcd_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_dcd_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_dcd_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_dcd_req=%08x is not allocated",
                    uart_dcd_req);
  }
  else if(uart_dcd_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_dcd_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_dcd_req->device];

  /*
   * set DLC instance
   */
  if((GET_STATE( UART_SERVICE_KER )) EQ KER_READY)
    dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  else
    dlc = &uart_data->dlc_table[uart_data->
                      dlc_instance[uart_dcd_req->dlci]];

  /*
   * store new line states
   */
  if(uart_dcd_req->line_state EQ UART_LINE_ON)
  {
    TRACE_EVENT_P1("DCD: on - DLCI=%d", uart_dcd_req->dlci);
    dlc->lines&= ~(UART_DCD_MASK);
  }
  else
  {
    TRACE_EVENT_P1("DCD: off - DLCI=%d", uart_dcd_req->dlci);
    dlc->lines|= UART_DCD_MASK;
  }

  /*
   * free the received primitive
   */
  PFREE(uart_dcd_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
    case KER_MUX:
      /*
       * flush all still to send data
       */
      dlc->received_prim        |= UART_DCD_REQ_MASK;
      uart_data->ker.flush_state = UART_KER_DRX_FLUSH;
      dlc->flushed   = FALSE;
      uart_data->drx = dlc->drx;
      sig_ker_drx_flush_req();
      break;

    default:
      TRACE_ERROR( "UART_DCD_REQ unexpected" );
      break;
  }
} /* ker_uart_dcd_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_escape_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_ESCAPE_REQ
|
| Parameters  : *uart_escape_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_escape_req ( T_UART_ESCAPE_REQ *uart_escape_req )
{

  TRACE_FUNCTION( "ker_uart_escape_req" );

#ifdef UART_RANGE_CHECK
  if(uart_escape_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_escape_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_escape_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_escape_req=%08x is not allocated",
                    uart_escape_req);
  }
  else if(uart_escape_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_escape_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_escape_req->device];

  /*
   * set DLC instance
   */
/* Following part of code is not required so put under comment ,but may be used 
  *  in future
  */  
 /*
  if((GET_STATE( UART_SERVICE_KER )) EQ KER_READY)
    dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
  else
    dlc = &uart_data->dlc_table[uart_data->
                      dlc_instance[uart_escape_req->dlci]];
*/
  /*
   * Send confirmation to ACI
   */
  {
    PALLOC (uart_escape_cnf, UART_ESCAPE_CNF);

    uart_escape_cnf->device = uart_escape_req->device;
    uart_escape_cnf->dlci = uart_escape_req->dlci;

    PSEND (hCommMMI, uart_escape_cnf);
  }

  /*
   * TODO: Set escape on/off parameter in dtx,
   *       Call UF_SetEscape and handle/stop ongoing
   *       escape sequence detection if required
   */

  /*
   * free the received primitive
   */
  PFREE(uart_escape_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
    case KER_MUX:
      break;

    default:
      TRACE_ERROR( "UART_ESCAPE_REQ unexpected" );
      break;
  }
} /* ker_uart_escape_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_start_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_START_REQ
|
| Parameters  : *uart_mux_start_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_start_req ( T_UART_MUX_START_REQ *uart_mux_start_req )
{
  T_DLC*  dlc;
#ifndef  _SIMULATION_
  USHORT  ret;
#endif
  TRACE_FUNCTION( "ker_uart_mux_start_req" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_start_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_start_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_start_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_start_req=%08x is not allocated",
                    uart_mux_start_req);
  }
  else if(uart_mux_start_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_start_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_start_req->device];

  /*
   * set parameters
   */
  uart_data->n1     = uart_mux_start_req->n1;
  uart_data->ker.n2 = uart_mux_start_req->n2;
  sig_ker_rt_parameters_req(uart_mux_start_req->t1,
                            uart_mux_start_req->t2,
                            uart_mux_start_req->t3);

  /*
   * free the received primitive
   */
  PFREE(uart_mux_start_req);

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_READY:
      /*
       * flush all still to send data
       */
      uart_data->ker.received_prim|= UART_MUX_START_REQ_MASK;
      uart_data->ker.flush_state   = UART_KER_DRX_FLUSH;
      dlc = &uart_data->dlc_table[UART_CONTROL_INSTANCE];
      dlc->flushed   = FALSE;
      uart_data->drx = dlc->drx;
      sig_ker_drx_flush_req();
      break;

    case KER_DEAD:
      SET_STATE( UART_SERVICE_KER, KER_MUX_ESTABLISH );
#ifdef _SIMULATION_
      {
        /*
         * send DTI_GETDATA_REQ
         */
        PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
#ifdef DTI2
          dti_getdata_req->link_id = LINK_ENABLE_PORT_1; /* for enable */
#else  /* DTI2 */
          dti_getdata_req->tui    = 2; /* for enable */
          dti_getdata_req->c_id   = 0;
          dti_getdata_req->op_ack = 0;
#endif /* DTI2 */
        PSEND (hCommMMI, dti_getdata_req);
      }
#else /* _SIMULATION_ */
#ifndef FF_MULTI_PORT
      if((ret = UF_Enable (uart_data->device, TRUE)) NEQ UF_OK)
      {
          TRACE_ERROR_P2("UF Driver: Can't enable UART, [%d], uart_kerp.c(%d)",
                                                                 ret,__LINE__);
      }
#endif /* !FF_MULTI_PORT */
#endif /* _SIMULATION */

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
      break;

    default:
      TRACE_ERROR( "UART_MUX_START_REQ unexpected" );
      break;
  }
} /* ker_uart_mux_start_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_dlc_establish_res
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_DLC_ESTABLISH_RES
|
| Parameters  : *uart_mux_dlc_establish_res - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_dlc_establish_res (
              T_UART_MUX_DLC_ESTABLISH_RES *uart_mux_dlc_establish_res )
{
  T_desc2* temp_desc;
  T_DLC*   dlc;

  TRACE_FUNCTION( "uart_mux_dlc_establish_res" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_dlc_establish_res EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_dlc_establish_res is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_dlc_establish_res
                            - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_dlc_establish_res=%08x is not allocated",
                    uart_mux_dlc_establish_res);
  }
  else if(uart_mux_dlc_establish_res->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_dlc_establish_res->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_dlc_establish_res->device];

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX:
      dlc = &uart_data->dlc_table[uart_data->
             dlc_instance[uart_mux_dlc_establish_res->dlci]];
      dlc->connection_state = UART_CONNECTION_OPEN;
      /*
       * send UA response
       */
      temp_desc         = dlc->next_command;
      dlc->next_command = NULL;
      ker_mux_send_frame(temp_desc);
      /*
       * start Data services
       */
      uart_data->drx = dlc->drx;
      uart_data->dtx = dlc->dtx;
      sig_ker_drx_ready_mode_req(uart_data->
        dlc_instance[uart_mux_dlc_establish_res->dlci]);
      sig_ker_dtx_ready_mode_req(uart_data->
        dlc_instance[uart_mux_dlc_establish_res->dlci]);
      break;

    default:
      TRACE_ERROR( "UART_MUX_DLC_ESTABLISH_RES unexpected" );
      break;
  }
  /*
   * free the received primitive
   */
  PFREE(uart_mux_dlc_establish_res);
} /* uart_mux_dlc_establish_res() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_dlc_release_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_DLC_RELEASE_REQ
|
| Parameters  : *ker_uart_mux_dlc_release_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_dlc_release_req (
              T_UART_MUX_DLC_RELEASE_REQ *uart_mux_dlc_release_req )
{
  T_desc2* temp_desc;
  T_DLC*   dlc;
  UBYTE    dlc_instance; /* channel */

  TRACE_FUNCTION( "ker_uart_mux_dlc_release_req" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_dlc_release_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_dlc_release_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_dlc_release_req
                            - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_dlc_release_req=%08x is not allocated",
                    uart_mux_dlc_release_req);
  }
  else if(uart_mux_dlc_release_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_dlc_release_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_dlc_release_req->device];

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX:
      dlc_instance = uart_data->dlc_instance[uart_mux_dlc_release_req->dlci];
      dlc = &uart_data->dlc_table[dlc_instance];
      /*
       * close DTI connection
       */
      uart_data->drx = dlc->drx;
      uart_data->dtx = dlc->dtx;
      sig_ker_drx_dead_mode_req();
      sig_ker_dtx_dead_mode_req();
#ifdef DTILIB
      if(dlc->dti_state NEQ DTI_CLOSED)
      {
      dti_close(
        uart_hDTI,
        uart_data->device,
        UART_DTI_UP_INTERFACE,
        dlc_instance,
        FALSE
        );
        dlc->dti_state = DTI_CLOSED;
      }
#else  /* DTILIB */
      if(dlc->hCommUPLINK NEQ VSI_ERROR)
      {
        vsi_c_close (VSI_CALLER dlc->hCommUPLINK);
        dlc->hCommUPLINK = VSI_ERROR;
      }
#endif /* DTILIB */
      switch(dlc->connection_state)
      {
        case UART_CONNECTION_SABM_RCVD:
          /*
           * negative response for an UART_MUX_DLC_ESTABLISH_IND
           * send DM response
           */
          temp_desc         = dlc->next_command;
          dlc->next_command = NULL;
          temp_desc->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME
                          ;/*lint !e415 access of out-of-bounds pointer*/
          ker_mux_send_frame(temp_desc);
          ker_mux_dlc_release(dlc_instance);
          break;

        case UART_CONNECTION_OPEN:
          dlc->connection_state = UART_CONNECTION_DISC_SENT;
          ker_send_disc_frame(uart_mux_dlc_release_req->dlci);
          break;
        case UART_CONNECTION_DEAD:
          break;

        default:
            TRACE_EVENT_P3("Warning: Unexpected DLC connection state: %d - \
                            %s(%d)", dlc->connection_state, __FILE__, __LINE__);
          break;
      }
      break;

    default:
      TRACE_ERROR( "UART_MUX_DLC_RELEASE_REQ unexpected" );
      break;
  }
  /*
   * free the received primitive
   */
  PFREE(uart_mux_dlc_release_req);
} /* ker_uart_mux_dlc_release_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_sleep_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_SLEEP_REQ
|
| Parameters  : *uart_mux_sleep_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_sleep_req (
              T_UART_MUX_SLEEP_REQ *uart_mux_sleep_req )
{
  TRACE_FUNCTION( "uart_mux_sleep_req" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_sleep_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_sleep_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_sleep_req
                            - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_sleep_req=%08x is not allocated",
                    uart_mux_sleep_req);
  }
  else if(uart_mux_sleep_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_sleep_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_sleep_req->device];

  /*
   * Primitive UART_MUX_SLEEP_REQ is not supported
   */
  TRACE_ERROR( "UART_MUX_SLEEP_REQ unexpected" );

  /*
   * free the received primitive
   */
  PFREE(uart_mux_sleep_req);
} /* ker_uart_mux_sleep_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_wakeup_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_WAKEUP_REQ
|
| Parameters  : *uart_mux_wakeup_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_wakeup_req (
              T_UART_MUX_WAKEUP_REQ *uart_mux_wakeup_req )
{
  TRACE_FUNCTION( "ker_uart_mux_wakeup_req" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_wakeup_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_wakeup_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_wakeup_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_wakeup_req=%08x is not allocated",
                    uart_mux_wakeup_req);
  }
  else if(uart_mux_wakeup_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_wakeup_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_wakeup_req->device];
  /*
   * Primitive UART_MUX_WAKEUP_REQ is not supported
   */
  TRACE_ERROR( "UART_MUX_WAKEUP_REQ unexpected" );
  /*
   * free the received primitive
   */
  PFREE(uart_mux_wakeup_req);
} /* ker_uart_mux_wakeup_req() */



/*
+------------------------------------------------------------------------------
| Function    : ker_uart_mux_close_req
+------------------------------------------------------------------------------
| Description : Handles the primitive UART_MUX_CLOSE_REQ
|
| Parameters  : *uart_mux_close_req - Ptr to primitive payload
|
+------------------------------------------------------------------------------
*/
GLOBAL void ker_uart_mux_close_req (
              T_UART_MUX_CLOSE_REQ *uart_mux_close_req )
{
  UBYTE    i;
  T_DLC*   dlc;
  T_desc2* temp_desc;
  BOOL     continuous;


  TRACE_FUNCTION( "ker_uart_mux_close_req" );

#ifdef UART_RANGE_CHECK
  if(uart_mux_close_req EQ NULL)
  {
    TRACE_EVENT("ERROR: uart_mux_close_req is NULL");
  }
  else if((*((ULONG*)((UBYTE*)uart_mux_close_req - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: uart_mux_close_req=%08x is not allocated",
                    uart_mux_close_req);
  }
  else if(uart_mux_close_req->device >= UART_INSTANCES)
  {
    TRACE_EVENT_P2("ERROR: device=%d is greater than UART_INSTANCES=%d",
                    uart_mux_close_req->device,
                    UART_INSTANCES);
  }
#endif /* UART_RANGE_CHECK */

  /*
   * set UART instance
   */
  uart_data = &uart_data_base[uart_mux_close_req->device];

  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_MUX_ESTABLISH:
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

    case KER_MUX:
      SET_STATE( UART_SERVICE_KER, KER_MUX_DLC_CLOSING );
      /*
       * close all VSI channels and Data services
       */
      continuous = TRUE;
      for(i = 0; i <= UART_MAX_NUMBER_OF_CHANNELS; i++)
      {
        dlc = &uart_data->dlc_table[i];
        /*
         * close all DLC channels except Control channel
         */
        if(i NEQ UART_CONTROL_INSTANCE)
        {
          switch(dlc->connection_state)
          {
            case UART_CONNECTION_SABM_RCVD:
              /*
               * send DM response
               */
              temp_desc         = dlc->next_command;
              dlc->next_command = NULL;
              temp_desc->buffer[UART_OFFSET_CONTROL] = UART_DM_CONTROL_FRAME
                               ;/*lint !e415 access of out-of-bounds pointer*/
              ker_mux_send_frame(temp_desc);
              ker_mux_dlc_release(i);
              break;

            case UART_CONNECTION_OPEN:
              dlc->connection_state = UART_CONNECTION_DISC_SENT;
              ker_send_disc_frame(dlc->dlci);
              continuous = FALSE;
              break;

            case UART_CONNECTION_DISC_SENT:
              continuous = FALSE;
              break;

            case UART_CONNECTION_DEAD:
              break;

            default:
              TRACE_EVENT_P3("Warning: Unexpected DLC connection state: %d - \
                             %s(%d)",dlc->connection_state,__FILE__, __LINE__);
              break;
          }
        }
      }
      if(continuous EQ TRUE)
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
      break;

    case KER_MUX_DLC_CLOSING:
    case KER_MUX_CLOSING:
    case KER_MUX_CLOSED:
      break;

    default:
      TRACE_ERROR( "UART_MUX_CLOSE_REQ unexpected" );
      break;
  }

  /*
   * free the received primitive
   */
  PFREE(uart_mux_close_req);

} /* ker_uart_mux_close_req() */



#ifdef DTILIB
/*
+------------------------------------------------------------------------------
| Function    : sig_dti_ker_connection_opened_ind
+------------------------------------------------------------------------------
| Description : Handles the DTILIB callback call DTI_REASON_CONNECTION_OPENED
|
|               This signal means that a dti connection has been successfully opened.
|
| Parameter   : dlc_instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dti_ker_connection_opened_ind (UBYTE dlc_instance)
{
#ifndef _SIMULATION_
  USHORT ret;
#endif
  PALLOC (uart_dti_cnf, UART_DTI_CNF);

  TRACE_FUNCTION( "sig_dti_ker_connection_opened_ind" );

  /*
   * send confirm primitive
   */
  uart_dti_cnf->device = uart_data->device;
  uart_dti_cnf->dlci   = uart_data->dlc_table[dlc_instance].dlci; /* EQ dlci */
  uart_dti_cnf->dti_conn = UART_CONNECT_DTI;
  PSEND (hCommMMI, uart_dti_cnf);

  /*
   * initialize new communication channel
   */
  switch( GET_STATE( UART_SERVICE_KER ) )
  {
    case KER_DEAD:
      SET_STATE( UART_SERVICE_KER, KER_READY );
      /*
       * enable UART
       */
#ifdef _SIMULATION_
      {
        /*
         * send DTI_GETDATA_REQ
         */
        PALLOC (dti_getdata_req, DTI2_GETDATA_REQ);
#ifdef DTI2
        dti_getdata_req->link_id = LINK_ENABLE_PORT_1; /* for enable */
#else  /* DTI2 */
        dti_getdata_req->tui    = 2; /* for enable */
        dti_getdata_req->c_id   = 0;
        dti_getdata_req->op_ack = 0;
#endif /* DTI2 */
        PSEND (hCommMMI, dti_getdata_req);
      }
#else /* _SIMULATION_ */
      if((ret = UF_Enable (uart_data->device, TRUE)) NEQ UF_OK)
      {
          TRACE_ERROR_P2("UF Driver: Can't enable UART, [%d], uart_kerp.c(%d)",
                                                                 ret,__LINE__);
      }
#endif /* _SIMULATION */

      /*
       * set RX and TX in ready mode
       */
      sig_ker_rx_ready_mode_req();
      sig_ker_tx_ready_mode_req();

      /*
       * set DTI connection
       */
      sig_ker_drx_ready_mode_req(UART_CONTROL_INSTANCE);
      sig_ker_dtx_ready_mode_req(UART_CONTROL_INSTANCE);
      sig_ker_drx_set_dtilib_peer_req();
      sig_ker_dtx_set_dtilib_peer_req();
      break;

    case KER_READY:
    case KER_MUX:
      sig_ker_drx_set_dtilib_peer_req();
      sig_ker_dtx_set_dtilib_peer_req();
      break;

    default:
      TRACE_ERROR( "UART_DTI_REQ unexpected" );
      break;
  }
} /* sig_dti_ker_connection_opened_ind */



/*
+------------------------------------------------------------------------------
| Function    : sig_dti_ker_connection_closed_ind
+------------------------------------------------------------------------------
| Description : Handles the DTILIB callback call DTI_REASON_CONNECTION_CLOSED
|
|               This signal means that a dti connection has been closed by
|               the neighbour entity.
|
| Parameters  : dlc_instance - affected dlc instance
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dti_ker_connection_closed_ind (U8 dlc_instance)
{
  T_DLC *dlc;

  TRACE_FUNCTION( "sig_dti_ker_connection_closed_ind" );

  /*
   * set DLC
   */
  dlc = &uart_data->dlc_table[dlc_instance];
  uart_data->drx = dlc->drx;
  uart_data->dtx = dlc->dtx;
  /*
   * set DTI connection to closed state
   * if it is already closed, do nothing
   */
  if(dlc->dti_state NEQ DTI_CLOSED)
  {
    dlc->dti_state = DTI_CLOSED;
    sig_ker_drx_disconnected_mode_req();
    sig_ker_dtx_disconnected_mode_req();
    /*
     * inform MMI
     */
    {
      PALLOC (uart_dti_ind, UART_DTI_IND);
      uart_dti_ind->device = uart_data->device;
      uart_dti_ind->dlci   = dlc->dlci;
      uart_dti_ind->dti_conn = UART_DISCONNECT_DTI;
      PSEND (hCommMMI, uart_dti_ind);
    }
  }
} /* sig_dti_ker_connection_closed_ind */
#endif /* DTILIB */

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
|             SDL-documentation (DTX-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_DTXF_C
#define UART_DTXF_C
#endif /* !UART_DTXF_C */

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

/*==== CONST ================================================================*/

/*
 * names of escape sequence detection state machine
 */
#ifdef _SIMULATION_
char * uart_esd_state_names[] =
{
  "UART_ESD_NULL",
  "UART_ESD_CHAR_1",
  "UART_ESD_CHAR_2",
  "UART_ESD_CHAR_3",
  "UART_ESD_DETECTED"
};
#endif /* _SIMULATION_ */

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : dtx_get_esd_state
+------------------------------------------------------------------------------
| Description : This function returns the current state of the Escape Sequence
|               Detection.
|
| Parameters  : none
|
| Return      : ESD State
|
+------------------------------------------------------------------------------
*/
GLOBAL T_ESD_STATE dtx_get_esd_state()
{
  TRACE_FUNCTION ("dtx_get_esd_state");

#ifdef _SIMULATION_
  TRACE_EVENT_P1 ("esd_state is [%s]",
                  uart_esd_state_names[uart_data->dtx->esd_state]);
#endif /* _SIMULATION_ */

  return uart_data->dtx->esd_state;
} /* dtx_get_esd_state() */



/*
+------------------------------------------------------------------------------
| Function    : dtx_set_esd_state
+------------------------------------------------------------------------------
| Description : This function sets the state of the Escape Sequence Detection.
|
| Parameters  : new ESD State
|
| Return      : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_set_esd_state( T_ESD_STATE new_esd_state )
{
  TRACE_FUNCTION ("dtx_set_esd_state");

#ifdef _SIMULATION_
  if(uart_data->dtx->esd_state != new_esd_state)
  {
    TRACE_EVENT_P2 ("esd_state [%s] -> [%s]",
                    uart_esd_state_names[uart_data->dtx->esd_state],
                    uart_esd_state_names[new_esd_state]);
    uart_data->dtx->esd_state = new_esd_state;
    uart_data->dtx->esd_state_name = uart_esd_state_names[new_esd_state];
  }
  else
  {
    TRACE_EVENT_P1 ("esd_state remains [%s]",
                    uart_esd_state_names[uart_data->dtx->esd_state]);
  }
#else /* _SIMULATION_ */
  if(uart_data->dtx->esd_state != new_esd_state)
    uart_data->dtx->esd_state = new_esd_state;
#endif /* _SIMULATION_ */
} /* dtx_set_esd_state() */



/*
+------------------------------------------------------------------------------
| Function    : dtx_init
+------------------------------------------------------------------------------
| Description : The function dtx_init() initializes the service DTX
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_init ()
{
  USHORT i;

  TRACE_FUNCTION( "dtx_init" );

  for( i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++ )
  {
    /*
     * select next instance of service DTX
     */
    uart_data->dtx = &( uart_data->dtx_base[i] );

    /*
     * initialize service specific variables
     */
    uart_data->dtx->to_send_data  = NULL;
    uart_data->dtx->cur_desc_size = 0;
    uart_data->dtx->write_pos     = 0;
    uart_data->dtx->size_multiplier   = 3;
    uart_data->dtx->st_flow           = DTI_FLOW_ON;
    uart_data->dtx->st_line_sa        = DTI_SA_ON;
    uart_data->dtx->st_line_sb        = DTI_SB_ON;
    uart_data->dtx->st_break_len      = DTI_BREAK_OFF;
    uart_data->dtx->detect_escape     = TRUE;
    uart_data->dtx->lines_changed     = FALSE;
    uart_data->dtx->receiving_state   = UART_DTX_NOT_RECEIVING;
    uart_data->dtx->data_flow         = UART_FLOW_DISABLED;
    dtx_set_esd_state( UART_ESD_NULL );
    uart_data->dtx->esd_guard_time    = 0;
    uart_data->dtx->esd_pos           = 0;
    uart_data->dtx->dti_dtx_state = DTI_CLOSED;

    INIT_STATE( UART_SERVICE_DTX , DTX_DEAD );
  }
} /* dtx_init() */



/*
+------------------------------------------------------------------------------
| Function    : dtx_free_resources
+------------------------------------------------------------------------------
| Description : The function dtx_free_resources() frees the currently allocated
|               resources of the service DTX.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_free_resources ()
{
  TRACE_FUNCTION( "dtx_free_resources()" );

  /*
   * free all resources
   */
  if(uart_data->dtx->to_send_data)
  {
    MFREE_DESC2( uart_data->dtx->to_send_data );
    uart_data->dtx->to_send_data  = NULL;
  }
  uart_data->dtx->cur_desc_size = 0;
  uart_data->dtx->write_pos     = 0;

} /* dtx_free_resources() */

/*
+------------------------------------------------------------------------------
| Function    : dtx_exit
+------------------------------------------------------------------------------
| Description : The function dtx_exit() frees the currently allocated
|               resources of the service DTX.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_exit ()
{
  ULONG i;

  TRACE_FUNCTION( "dtx_exit()" );

  /*
   * free all resources
   */
  if(uart_data->dtx->to_send_data)
  {
    MFREE_DESC2( uart_data->dtx->to_send_data );
    uart_data->dtx->to_send_data  = NULL;
  }
  uart_data->dtx->cur_desc_size = 0;
  uart_data->dtx->write_pos     = 0;

  if(uart_data->ker.tx_data_waiting)
  {
    MFREE_DESC2(uart_data->ker.tx_data_waiting);
    uart_data->ker.tx_data_waiting = NULL;
  }
  for(i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++)
  {
    if(uart_data->dlc_table[i].transmit_data)
    {
      MFREE_DESC2(uart_data->dlc_table[i].transmit_data);
      uart_data->dlc_table[i].transmit_data = NULL;
    }

    if(uart_data->dlc_table[i].receive_data)
    {
      MFREE_DESC2(uart_data->dlc_table[i].receive_data);
      uart_data->dlc_table[i].receive_data = NULL;
    }
  }

} /* dtx_exit() */


/*
+------------------------------------------------------------------------------
| Function    : dtx_allocate_resources
+------------------------------------------------------------------------------
| Description : The function dtx_allocate_resources() allocates a new set of
|               resources for the service DTX.
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_allocate_resources ()
{
  TRACE_FUNCTION( "dtx_allocate_resources()" );

  /*
   * allocate a new descriptor
   * with a size of: size_multiplier * N1 (frame size)
   */
  uart_data->dtx->cur_desc_size = uart_data->n1 *
                                  uart_data->dtx->size_multiplier;
  MALLOC(uart_data->dtx->to_send_data,
         (USHORT)(sizeof(T_desc2) - 1 + uart_data->dtx->cur_desc_size));
  uart_data->dtx->to_send_data->len  = 0;
  uart_data->dtx->to_send_data->size  = 0;
  uart_data->dtx->to_send_data->offset  = 0;
  uart_data->dtx->to_send_data->next = (ULONG) NULL;
  uart_data->dtx->write_pos          = 0;
} /* dtx_allocate_resources() */



/*
+------------------------------------------------------------------------------
| Function    : dtx_calculate_size_multiplier
+------------------------------------------------------------------------------
| Description : The function dtx_calculate_size_multiplier() calculates the new
|               size_multiplier value.
|
| Parameters  : desc_to_send - data descriptor that should be sent
|               data_flow    - state of data flow befor we want to sent
|
+------------------------------------------------------------------------------
*/
GLOBAL void dtx_calculate_size_multiplier (T_desc2* desc_to_send,
                                           T_DATA_FLOW_STATE data_flow)
{
  TRACE_FUNCTION( "dtx_calculate_size_multiplier()" );

  /*
   * calculate new size multiplier according to fillrate of buffer
   */
  if((uart_data->dtx->size_multiplier < UART_DTX_MAX_SIZE_MULTIPLIER ) &&
     (data_flow EQ UART_FLOW_DISABLED))
  {
    /*
     * buffer is rather full, so increase the multiplier
     */
#ifdef _SIMULATION_
    TRACE_EVENT( " sig_rx_dtx_data_received_ind(): buffer size_multiplier increased " );
#endif /* _SIMULATION_ */
    uart_data->dtx->size_multiplier++;
  }
  else if((uart_data->dtx->size_multiplier > 3) &&
          (data_flow EQ UART_FLOW_ENABLED)  &&
          ((uart_data->dtx->cur_desc_size - desc_to_send->len) >
           ((USHORT)uart_data->n1 << 1)))
  {
    /*
     * buffer is rather empty, so decrease the multiplier
     */
#ifdef _SIMULATION_
    TRACE_EVENT( " sig_rx_dtx_data_received_ind(): buffer size_multiplier decreased " );
#endif /* _SIMULATION_ */
    uart_data->dtx->size_multiplier--;
  }
} /* dtx_calculate_size_multiplier() */

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
|             functions to handle the incoming process internal signals as
|             described in the SDL-documentation (RT-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_RTS_C
#define UART_RTS_C
#endif /* !UART_RTS_C */

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

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_parameters_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_PARAMETERS_REQ
|               This signal sets new start values for the three multiplexer
|               timers:
|               T1 - acknowledgement timer (in units of 10 ms)
|               T2 - response timer for multiplexer control channel
|                    (in units of 10 ms)
|               T3 - wake-up response timer (in seconds)
|
| Parameters  : t1 - new start value of timer T1
|               t2 - new start value of timer T2
|               t3 - new start value of timer T3
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_parameters_req (UBYTE t1, UBYTE t2, UBYTE t3)
{
  TRACE_ISIG( "sig_ker_rt_parameters_req" );

  /*
   * set set new start values of timers
   */
  uart_data->rt.t1 = (T_TIME)t1 * 10;
  uart_data->rt.t2 = (T_TIME)t2 * 10;
  uart_data->rt.t3 = (T_TIME)t3 * 1000;
} /* sig_ker_rt_parameters_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_start_t1_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_START_T1_REQ
|               which is used to (re-)start the timer t1
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_start_t1_req ()
{
  TRACE_ISIG( "sig_ker_rt_start_t1_req" );

  if(TIMER_START(UART_handle, uart_data->timer_t1_index, uart_data->rt.t1 ) NEQ VSI_OK)
  {
    TRACE_ERROR_P1("VSI entity: Can't start timer t1, uart_rts.c(%d)", __LINE__);
  }

  uart_data->rt.state_t1 = UART_RT_STARTED;

} /* sig_ker_rt_start_t1_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_start_t2_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_START_T2_REQ
|               which is used to (re-)start the timer t2
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_start_t2_req ()
{
  TRACE_ISIG( "sig_ker_rt_start_t2_req" );


  if(TIMER_START(UART_handle, uart_data->timer_t2_index, uart_data->rt.t2 ) NEQ VSI_OK)
  {
    TRACE_ERROR_P1("VSI entity: Can't start timer t2, uart_rts.c(%d)", __LINE__);
  }

  uart_data->rt.state_t2 = UART_RT_STARTED;

} /* sig_ker_rt_start_t2_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_start_t3_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_START_T3_REQ
|               which is used to (re-)start the timer t3
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_start_t3_req ()
{
  TRACE_ISIG( "sig_ker_rt_start_t3_req" );


  if(TIMER_START(UART_handle, uart_data->timer_t3_index, uart_data->rt.t3 ) NEQ VSI_OK)
  {
    TRACE_ERROR_P1("VSI entity: Can't start timer t3, uart_rts.c(%d)", __LINE__);
  }

  uart_data->rt.state_t3 = UART_RT_STARTED;

} /* sig_ker_rt_start_t3_req() */


/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_rt_start_tesd_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_RT_START_TESD_REQ
|               which is used to start the timer tesd
|
| Parameters  : tesd_value  - startvalue of TESD
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_rt_start_tesd_req (T_TIME tesd_value)
{
  TRACE_ISIG( "sig_dtx_rt_start_tesd_req" );
  /*
   * store lowest value
   */
  if (tesd_value < uart_data->rt.tesd)
  {
#ifdef _SIMULATION_
    TRACE_EVENT_P1("ESD: New start value uart_data->rt.tesd: %d", tesd_value);
#endif /* _SIMULATION_ */
    uart_data->rt.tesd = tesd_value;
  }
  /*
   * 1. This is the "first" call to sig_dtx_rt_start_tesd_req:
   *    state is UART_RT_STOPPED
   * 2. Called from sig_ker_dtx_timeout_tesd_req:
   *    state is still UART_RT_STARTED (is reset to UART_RT_STOPPED when none
   *    of the DLCs wants to restart TESD)
   */
  if (uart_data->rt.state_tesd EQ UART_RT_STOPPED)
  {
#ifdef _SIMULATION_
    TRACE_EVENT_P1("ESD: Start timer TESD( %d )", uart_data->rt.tesd);
#endif /* _SIMULATION_ */

    if(TIMER_START (UART_handle, uart_data->timer_tesd_index, uart_data->rt.tesd ) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't start timer, uart_rts.c(%d)", __LINE__);
    }
    uart_data->rt.state_tesd = UART_RT_STARTED;
  }
#ifdef _SIMULATION_
  else
  {
    TRACE_EVENT("ESD: Timer TESD will be started later !");
  }
#endif /* _SIMULATION_ */
} /* sig_dtx_rt_start_tesd_req() */


/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_stop_t1_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_STOP_T1_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_stop_t1_req ()
{
  TRACE_ISIG( "sig_ker_rt_stop_t1_req" );

  if( uart_data->rt.state_t1 EQ UART_RT_STARTED )
  {
    if(TIMER_STOP(UART_handle, uart_data->timer_t1_index ) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't stop timer t1, uartrts.c(%d)", __LINE__);
    }
  }

  uart_data->rt.state_t1 = UART_RT_STOPPED;
} /* sig_ker_rt_stop_t1_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_stop_t2_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_STOP_T2_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_stop_t2_req ()
{
  TRACE_ISIG( "sig_ker_rt_stop_t2_req" );

  if( uart_data->rt.state_t2 EQ UART_RT_STARTED )
  {
    if(TIMER_STOP(UART_handle, uart_data->timer_t2_index ) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't stop timer t2, uart_rts.c(%d)",__LINE__);
    }
  }

  uart_data->rt.state_t2 = UART_RT_STOPPED;
} /* sig_ker_rt_stop_t2_req() */



/*
+------------------------------------------------------------------------------
| Function    : sig_ker_rt_stop_t3_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_KER_RT_STOP_T3_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_ker_rt_stop_t3_req ()
{
  TRACE_ISIG( "sig_ker_rt_stop_t3_req" );

  if( uart_data->rt.state_t3 EQ UART_RT_STARTED )
  {
    if(TIMER_STOP(UART_handle, uart_data->timer_t3_index ) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't stop timer t3, uart_rts.c(%d)", __LINE__);
    }
  }

  uart_data->rt.state_t3 = UART_RT_STOPPED;
} /* sig_ker_rt_stop_t3_req() */


/*
+------------------------------------------------------------------------------
| Function    : sig_dtx_rt_stop_tesd_req
+------------------------------------------------------------------------------
| Description : Handles the internal signal SIG_DTX_RT_STOP_TESD_REQ
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dtx_rt_stop_tesd_req ()
{
  TRACE_ISIG( "sig_dtx_rt_stop_tesd_req" );

  if( uart_data->rt.state_tesd EQ UART_RT_STARTED )
  {
    if(TIMER_STOP(UART_handle, uart_data->timer_tesd_index ) NEQ VSI_OK)
    {
      TRACE_ERROR_P1("VSI entity: Can't stop timer, uart_rts.c(%d)", __LINE__);
    }
}

  uart_data->rt.state_tesd = UART_RT_STOPPED;
} /* sig_dtx_rt_stop_tesd_req() */

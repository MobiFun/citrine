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
|             functions to handle the incoming primitives as described in
|             the SDL-documentation (RT-statemachine)
+-----------------------------------------------------------------------------
*/

#ifndef UART_RTP_C
#define UART_RTP_C
#endif /* !UART_RTP_C */

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

#include "uart_kers.h"  /* to get KER signal definitions */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : rt_t1_expired
+------------------------------------------------------------------------------
| Description : Handles the expitration of the acknowledge timer t1.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void rt_t1_expired ()
{
  TRACE_FUNCTION( "rt_t1_expired" );

  switch( uart_data->rt.state_t1 )
  {
    case UART_RT_STARTED:
      uart_data->rt.state_t1 = UART_RT_STOPPED;
      sig_rt_ker_timeout_t1_ind();
      break;
    default:
      TRACE_ERROR( "RT_T1_EXPIRED unexpected" );
      break;
  }
} /* rt_t1_expired() */



/*
+------------------------------------------------------------------------------
| Function    : rt_t2_expired
+------------------------------------------------------------------------------
| Description : Handles the expitration of the response timer t2.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void rt_t2_expired ()
{
  TRACE_FUNCTION( "rt_t2_expired" );

  switch( uart_data->rt.state_t2 )
  {
    case UART_RT_STARTED:
      uart_data->rt.state_t2 = UART_RT_STOPPED;
      sig_rt_ker_timeout_t2_ind();
      break;
    default:
      TRACE_ERROR( "RT_T2_EXPIRED unexpected" );
      break;
  }
} /* rt_t2_expired() */



/*
+------------------------------------------------------------------------------
| Function    : rt_t3_expired
+------------------------------------------------------------------------------
| Description : Handles the expiration of the wake-up timer t3.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void rt_t3_expired ()
{
  TRACE_FUNCTION( "rt_t3_expired" );

  switch( uart_data->rt.state_t3 )
  {
    case UART_RT_STARTED:
      uart_data->rt.state_t3 = UART_RT_STOPPED;
      sig_rt_ker_timeout_t3_ind();
      break;
    default:
      TRACE_ERROR( "RT_T3_EXPIRED unexpected" );
      break;
  }
} /* rt_t3_expired() */


/*
+------------------------------------------------------------------------------
| Function    : rt_tesd_expired
+------------------------------------------------------------------------------
| Description : Handles the expiration of the timer tesd.
|
| Parameters  : no parameters
|
+------------------------------------------------------------------------------
*/
GLOBAL void rt_tesd_expired ()
{
  TRACE_FUNCTION( "rt_tesd_expired" );

  switch( uart_data->rt.state_tesd )
  {
    case UART_RT_STARTED:
/*      uart_data->rt.state_tesd = UART_RT_STOPPED; state_tesd is not reset here
 * Every DLC that needs TESD will call "sig_dtx_rt_start_tesd_req" but only to
 * indicate its TESD-startvalue. The timer is started hereafter with the lowest
 * startvalue.
 */
      uart_data->rt.tesd = (T_TIME)-1;
      sig_rt_ker_timeout_tesd_ind();
      if( uart_data->rt.tesd != ((T_TIME)-1) )
      { /*
         * timer needs to be restarted
         */
#ifdef _SIMULATION_
        char buf[80];
        sprintf(buf, "ESD: Restart timer TESD: %d", uart_data->rt.tesd);
        TRACE_EVENT(buf);
#endif /* _SIMULATION_ */
        if (TIMER_START(UART_handle, uart_data->timer_tesd_index, uart_data->rt.tesd) NEQ VSI_OK)
        {
          TRACE_EVENT_P1("VSI entity: Can't start timer, uart_rtp(%d)",
                                                             __LINE__);
        }
        uart_data->rt.state_tesd = UART_RT_STARTED;
      }
      else
      {
#ifdef _SIMULATION_
        TRACE_EVENT("Nobody restarted the timer");
#endif /* _SIMULATION_ */
        uart_data->rt.state_tesd = UART_RT_STOPPED;
      }
      break;
    default:
      TRACE_ERROR( "RT_TESD_EXPIRED unexpected" );
      break;
  }
} /* rt_tesd_expired() */

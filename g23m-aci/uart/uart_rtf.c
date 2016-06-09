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
|             SDL-documentation (RT-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RTF_C
#define UART_RTF_C
#endif /* !UART_RTF_C */

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

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : rt_init
+------------------------------------------------------------------------------
| Description : The function rt_init() initializes the service RT which is
|               responsible for timer handling. 
|
| Parameters  : none
|
+------------------------------------------------------------------------------
*/
GLOBAL void rt_init ()
{ 
  TRACE_FUNCTION( "rt_init" );

  /*
   * initially all timers are stopped
   */

  uart_data->rt.state_t1 = UART_RT_STOPPED;
  uart_data->rt.state_t2 = UART_RT_STOPPED;
  uart_data->rt.state_t3 = UART_RT_STOPPED;
  uart_data->rt.state_tesd = UART_RT_STOPPED;

  /*
   * set default values of timers
   */
  uart_data->rt.t1 = UART_MUX_T1_DEFAULT;
  uart_data->rt.t2 = UART_MUX_T2_DEFAULT;
  uart_data->rt.t3 = UART_MUX_T3_DEFAULT;
  uart_data->rt.tesd = (T_TIME)-1;

  INIT_STATE( UART_SERVICE_RT , RT_STATE );  
} /* rt_init() */

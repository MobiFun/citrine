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
|             SDL-documentation (DRX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXF_C
#define UART_DRXF_C
#endif /* !UART_DRXF_C */

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

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
     Function    : drx_init
+------------------------------------------------------------------------------
     Description : The function drx_init() initializes the service DRX
    
     Parameters  : none
    
+------------------------------------------------------------------------------
*/
GLOBAL void drx_init ()
{ 
  UBYTE i;

  TRACE_FUNCTION( "drx_init" );

  for( i = 0; i < UART_MAX_NUMBER_OF_CHANNELS; i++ )
  {
    /*
     * select next instance of service DRX
     */
    uart_data->drx = &uart_data->drx_base[i];

    /*
     * initialize service instance specific variables
     */
    uart_data->drx->sending_state    = UART_DRX_NOT_SENDING;
    uart_data->drx->data_flow        = UART_FLOW_ENABLED;

    uart_data->drx->dlc_instance     = i;

    uart_data->drx->received_data    = NULL;
    uart_data->drx->read_pos         = 0;

    uart_data->drx->dti_drx_state = DTI_CLOSED;

    INIT_STATE( UART_SERVICE_DRX , DRX_DEAD );
  }
} /* drx_init() */



/*
+------------------------------------------------------------------------------
     Function    : drx_free_resources
+------------------------------------------------------------------------------
     Description : The function drx_free_resources() frees all resources of the 
                   service DRX
    
     Parameters  : none
    
+------------------------------------------------------------------------------
*/
GLOBAL void drx_free_resources ()
{
  TRACE_FUNCTION( "drx_free_resources" );

  if(uart_data->drx->received_data)
  {
    MFREE_DESC2( uart_data->drx->received_data );
  }
  uart_data->drx->received_data = NULL;
  uart_data->drx->read_pos = 0;
} /* drx_free_resources() */

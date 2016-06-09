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
|             the SDL-documentation (DRX-statemachine)
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXP_C
#define UART_DRXP_C
#endif /* !UART_DRXP_C */

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

#include "uart_kers.h"  /* to get signal definitions for service KER */
#ifdef FF_MULTI_PORT
#include "uart_ptxs.h"  /* to get signal definitions for service TX */
#else /* FF_MULTI_PORT */
#include "uart_txs.h"   /* to get signal definitions for service TX */
#endif /* FF_MULTI_PORT */
#include "uart_drxf.h"  /* to get function definitions for service DRX */

#ifdef _SIMULATION_
#include <stdio.h>      /* to get sprintf */
#endif /* _SIMULATION_ */

/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/



/*
+------------------------------------------------------------------------------
| Function    : sig_dti_drx_data_received_ind
+------------------------------------------------------------------------------
| Description : Handles the DTILIB callback call DTI_REASON_DATA_RECEIVED
|               This signal means that data has been received on a dti
|               connection.
|
| Parameter   : dti_data2_ind
|
+------------------------------------------------------------------------------
*/
GLOBAL void sig_dti_drx_data_received_ind(T_DTI2_DATA_IND *dti_data2_ind)
{
  T_desc2*  temp_desc1;
  T_desc2*  temp_desc2;

  TRACE_FUNCTION( "drx_dti_data_req" );

#ifdef UART_RANGE_CHECK
  if(dti_data2_ind EQ NULL)
  {
    TRACE_EVENT("ERROR: dti_data2_ind is NULL");
  }
  else if((*((ULONG*)((UBYTE*)dti_data2_ind - sizeof(T_PRIM_HEADER) - 8))) NEQ 0)
  {
    TRACE_EVENT_P1("ERROR: dti_data2_ind=%08x is not allocated",
                    dti_data2_ind);
  }
  else
  {
    T_desc2* range_desc;
    USHORT  range_sum;
    if((dti_data2_ind->parameters.st_lines.st_flow NEQ DTI_FLOW_ON) &&
       (dti_data2_ind->parameters.st_lines.st_flow NEQ DTI_FLOW_OFF))
    {
      TRACE_EVENT_P1("ERROR: st_flow=%d is invalid",
                      dti_data2_ind->parameters.st_lines.st_flow);
    }
    if((dti_data2_ind->parameters.st_lines.st_line_sa NEQ DTI_SA_ON) &&
       (dti_data2_ind->parameters.st_lines.st_line_sa NEQ DTI_SA_OFF))
    {
      TRACE_EVENT_P1("ERROR: st_line_sa=%d is invalid",
                      dti_data2_ind->parameters.st_lines.st_line_sa);
    }
    if((dti_data2_ind->parameters.st_lines.st_line_sb NEQ DTI_SB_ON) &&
       (dti_data2_ind->parameters.st_lines.st_line_sb NEQ DTI_SB_OFF))
    {
      TRACE_EVENT_P1("ERROR: st_line_sb=%d is invalid",
                      dti_data2_ind->parameters.st_lines.st_line_sb);
    }
    range_desc = (T_desc2*)(dti_data2_ind->desc_list2.first);
    range_sum  = 0;
    while((range_desc) &&
          ((*((ULONG*)((UBYTE*)range_desc - 8))) EQ 0))
    {
      range_sum += range_desc->len;
      range_desc = (T_desc2*)(range_desc->next);
    }
    if(range_desc)
    {
      TRACE_EVENT_P1("ERROR: data descriptor (%08x) not allocated",
                      range_desc);
    }
    else if(range_sum NEQ dti_data_req->desc_list2.list_len)
    {
      TRACE_EVENT_P2("ERROR: datalength=%d NEQ list_len=%d",
                      range_sum,
                      dti_data2_ind->desc_list2.list_len);
    }
  }
#endif /* UART_RANGE_CHECK */

  switch( GET_STATE( UART_SERVICE_DRX ) )
    {
      case DRX_READY:
        SET_STATE( UART_SERVICE_DRX, DRX_NOT_READY );
        dti_stop(
          uart_hDTI, 
          uart_data->device, 
          UART_DTI_UP_INTERFACE, 
          uart_data->drx->dlc_instance
          );
        /*
         * store data descriptor
         */
        temp_desc1 = (T_desc2*)(dti_data2_ind->desc_list2.first);
        while((temp_desc1) &&
              (temp_desc1->len EQ 0))
        {
          temp_desc2 = (T_desc2*)temp_desc1->next;
          MFREE(temp_desc1);
          temp_desc1 = temp_desc2;
        }
        uart_data->drx->received_data = temp_desc1;
        uart_data->drx->read_pos      = 0;

        /*
         * inform kernel about line states
         */
        sig_drx_ker_line_states_ind(uart_data->drx->dlc_instance,
                                    dti_data2_ind->parameters.st_lines.st_flow,
                                    dti_data2_ind->parameters.st_lines.st_line_sa,
                                    dti_data2_ind->parameters.st_lines.st_line_sb,
                                    dti_data2_ind->parameters.st_lines.st_break_len);
        /*
         * check if service can send data to peer
         */
        if(uart_data->drx->data_flow EQ UART_FLOW_ENABLED)
        {
          /*
           * yes, now if there is data to send, do it
           */
          if(uart_data->drx->received_data)
          {
            sig_drx_tx_data_available_ind( uart_data->drx->dlc_instance,
                                           uart_data->drx->received_data,
                                           uart_data->drx->read_pos );
          }
          else
          {
            /*
             * no data to send, do positive flow control towards upper layer
             */
            SET_STATE( UART_SERVICE_DRX, DRX_READY );
            dti_start(
              uart_hDTI, 
              uart_data->device, 
              UART_DTI_UP_INTERFACE, 
              uart_data->drx->dlc_instance
              );
          }
        }
        break;

      default:
        TRACE_ERROR( "DTI_DATA2_IND unexpected" );
        /*
         * since the descriptor will not be sent, free it
         */
        MFREE_DESC2( dti_data2_ind->desc_list2.first );
        break;
    }

    /*
     * free primitive (freeing of descriptors has been done before if necessary) 
     */
    PFREE( dti_data2_ind );
} /* sig_dti_drx_data_received_ind() */

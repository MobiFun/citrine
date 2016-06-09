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
|  Purpose :  Definitions for uart_kerp.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_KERP_H
#define UART_KERP_H


EXTERN void ker_uart_parameters_req ( 
              T_UART_PARAMETERS_REQ *uart_parameters_req );

EXTERN void ker_uart_dti_req ( T_UART_DTI_REQ *uart_set_dti_req );

EXTERN void ker_uart_disable_req ( T_UART_DISABLE_REQ *uart_disable_req );

EXTERN void ker_uart_ring_req ( T_UART_RING_REQ *uart_ring_req );

EXTERN void ker_uart_dcd_req ( T_UART_DCD_REQ *uart_dcd_req );

EXTERN void ker_uart_escape_req ( T_UART_ESCAPE_REQ *uart_escape_req );

EXTERN void ker_uart_mux_start_req ( T_UART_MUX_START_REQ *uart_mux_start_req );

EXTERN void ker_uart_mux_dlc_establish_res ( 
              T_UART_MUX_DLC_ESTABLISH_RES *uart_mux_dlc_establish_res );

EXTERN void ker_uart_mux_dlc_release_req (
              T_UART_MUX_DLC_RELEASE_REQ *uart_mux_dlc_release_req );

EXTERN void ker_uart_mux_sleep_req (
              T_UART_MUX_SLEEP_REQ *uart_mux_sleep_req );

EXTERN void ker_uart_mux_wakeup_req (
              T_UART_MUX_WAKEUP_REQ *uart_mux_wakeup_req );

EXTERN void ker_uart_mux_close_req (
              T_UART_MUX_CLOSE_REQ *uart_mux_close_req );

#ifdef DTILIB
EXTERN void sig_dti_ker_connection_opened_ind ( UBYTE dlc_instance );
#endif /* DTILIB */

#endif /* !UART_KERP_H */

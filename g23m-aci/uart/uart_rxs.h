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
|  Purpose :  Definitions for uart_rxs.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RXS_H
#define UART_RXS_H


EXTERN void sig_ker_rx_dead_mode_req (void);

EXTERN void sig_ker_rx_ready_mode_req (void);

EXTERN void sig_ker_rx_mux_mode_req (void);

EXTERN void sig_dtx_rx_ready_to_receive_req (UBYTE   dlc_instance,
                                             T_desc2* receive_data, 
                                             USHORT  receive_pos,
                                             USHORT  receive_size);

EXTERN void sig_ker_rx_ready_to_receive_req (T_desc2* receive_data, 
                                             USHORT  receive_pos,
                                             USHORT  receive_size);

EXTERN void sig_dtx_rx_not_ready_to_receive_req (UBYTE dlc_instance);

EXTERN void sig_ker_rx_not_ready_to_receive_req (void);

EXTERN void sig_ker_rx_restart_read_req (void);

#endif /* !UART_RXS_H */


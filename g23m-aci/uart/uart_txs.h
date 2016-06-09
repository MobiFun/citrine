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
|  Purpose :  Definitions for uart_txs.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_TXS_H
#define UART_TXS_H


EXTERN void sig_ker_tx_dead_mode_req (void);

EXTERN void sig_ker_tx_ready_mode_req (void);

EXTERN void sig_ker_tx_mux_mode_req (void);

EXTERN void sig_drx_tx_data_available_ind (UBYTE   dlc_instance,
                                           T_desc2* transmit_data, 
                                           USHORT  transmit_pos);

EXTERN void sig_ker_tx_data_available_req (T_desc2* transmit_data, 
                                           USHORT  transmit_pos);

EXTERN void sig_drx_tx_data_not_available_ind (UBYTE dlc_instance);

EXTERN void sig_ker_tx_data_not_available_req (void);

EXTERN void sig_ker_tx_line_states_req (UBYTE dlc_instance);

EXTERN void sig_ker_tx_flush_req (void);

EXTERN void sig_ker_tx_restart_write_req (void);

#endif /* !UART_TXS_H */


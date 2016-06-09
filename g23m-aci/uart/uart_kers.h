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
|  Purpose :  Definitions for uart_kers.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_KERS_H
#define UART_KERS_H

EXTERN void sig_drx_ker_line_states_ind(UBYTE  dlc_instance,
                                        UBYTE  st_flow,
                                        UBYTE  st_line_sa,
                                        UBYTE  st_line_sb,
                                        USHORT st_break_len);

EXTERN void sig_dtx_ker_enable_ind(UBYTE dlc_instance);

EXTERN void sig_dtx_ker_disable_ind(UBYTE dlc_instance);

EXTERN void sig_dtx_ker_escape_detected_ind(U8 dlc_instance);

EXTERN void sig_any_ker_flushed_ind(UBYTE dlc_instance);

EXTERN void sig_tx_ker_flushed_ind(void);

EXTERN void sig_tx_ker_sending_ind (void);

EXTERN void sig_tx_ker_data_sent_ind (T_desc2* rest_data, USHORT write_pos);

EXTERN void sig_rx_ker_receiving_ind (void);

EXTERN void sig_rx_ker_data_received_ind (T_desc2* received_data, 
                                          USHORT write_pos);

EXTERN void sig_rx_ker_line_states_ind (ULONG line_states);

EXTERN void sig_rt_ker_timeout_t1_ind (void);

EXTERN void sig_rt_ker_timeout_t2_ind (void);

EXTERN void sig_rt_ker_timeout_t3_ind (void);

EXTERN void sig_rt_ker_timeout_tesd_ind (void);

EXTERN void sig_dti_ker_connection_closed_ind (U8 dlc_instance);

#endif /* !UART_KERS_H */

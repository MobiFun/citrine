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
|  Purpose :  Definitions for uart_dtxs.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DTXS_H
#define UART_DTXS_H


EXTERN void sig_ker_dtx_ready_mode_req (UBYTE dlc_instance);

EXTERN void sig_ker_dtx_dead_mode_req (void);

EXTERN void sig_ker_dtx_line_states_req(UBYTE st_flow,
                                        UBYTE st_line_sa,
                                        UBYTE st_line_sb,
                                        UBYTE st_break_len);

EXTERN void sig_rx_dtx_receiving_ind (void);

EXTERN void sig_rx_dtx_data_received_ind (T_desc2* received_data, 
                                          USHORT  write_pos);

EXTERN void sig_ker_dtx_set_dti_peer_req (USHORT tui_peer, 
                                          T_HANDLE peer_handle, 
                                          UBYTE c_id);

#ifdef DTILIB
EXTERN void sig_ker_dtx_set_dtilib_peer_req (void);
#ifdef DTI2
EXTERN void sig_ker_dtx_disconnected_mode_req(void);
EXTERN void sig_ker_dtx_detect_escape_req (UBYTE detect_escape); 
#endif /* DTI2 */
#endif

EXTERN void sig_ker_dtx_timeout_tesd_req (void);


#endif /* !UART_DTXS_H */


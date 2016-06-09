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
|  Purpose :  Definitions for uart_kerf.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_KERF_H
#define UART_KERF_H


EXTERN void ker_setupUart (void);

EXTERN void ker_init (void);

EXTERN void ker_analyze_frame_info_command (ULONG* forward, T_desc2* frame);

EXTERN void ker_analyze_frame_info_response (ULONG* forward, T_desc2* frame);

EXTERN void ker_mux_dlc_release (UBYTE dlc_instance);

EXTERN void ker_mux_close_down (void);

EXTERN void ker_mux_send_frame (T_desc2* frame);

EXTERN void ker_mux_send_command_frame (UBYTE dlc_instance, T_desc2* frame);

EXTERN void ker_mux_send_line_states(UBYTE dlc_instance);

EXTERN void ker_mux_send_close_down(void);

EXTERN void ker_send_disc_frame (UBYTE dlci);

EXTERN void ker_receive_sabm_frame(ULONG* forward, T_desc2* frame);

EXTERN void ker_receive_ua_frame(ULONG* forward, T_desc2* frame);

EXTERN void ker_receive_dm_frame(ULONG* forward, T_desc2* frame);

EXTERN void ker_receive_disc_frame(ULONG* forward, T_desc2* frame);

EXTERN void ker_receive_uih_control_frame(ULONG* forward, T_desc2* frame);

EXTERN void ker_receive_uih_data_frame(ULONG* forward, T_desc2* frame);

#endif /* !UART_KERF_H */


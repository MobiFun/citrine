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
|  Purpose :  Definitions for uart_drxs.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXS_H
#define UART_DRXS_H


EXTERN void sig_ker_drx_ready_mode_req (UBYTE dlc_instance);

EXTERN void sig_ker_drx_dead_mode_req (void);

EXTERN void sig_ker_drx_enable_req (void);

EXTERN void sig_ker_drx_disable_req (void);

EXTERN void sig_tx_drx_sending_req (void);

EXTERN void sig_tx_drx_data_sent_req (T_desc2* rest_data, USHORT pos);

EXTERN void sig_ker_drx_set_dti_peer_req (USHORT tui_peer, 
                                          T_HANDLE peer_handle, 
                                          UBYTE c_id);

#ifdef DTILIB
EXTERN void sig_ker_drx_set_dtilib_peer_req (void);
#ifdef DTI2
EXTERN void sig_ker_drx_disconnected_mode_req(void);
#endif /* DTI2 */
#endif /* DTILIB */

EXTERN void sig_ker_drx_flush_req (void);

#endif /* !UART_DRXS_H */

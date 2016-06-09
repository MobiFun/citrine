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
|  Purpose :  Definitions for uart_rts.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RTS_H
#define UART_RTS_H


EXTERN void sig_ker_rt_parameters_req ( UBYTE t1, UBYTE t2, UBYTE t3);

EXTERN void sig_ker_rt_start_t1_req (void);

EXTERN void sig_ker_rt_start_t2_req (void);

EXTERN void sig_ker_rt_start_t3_req (void);

EXTERN void sig_dtx_rt_start_tesd_req (T_TIME tesd_value);

EXTERN void sig_ker_rt_stop_t1_req (void);

EXTERN void sig_ker_rt_stop_t2_req (void);

EXTERN void sig_ker_rt_stop_t3_req (void);

EXTERN void sig_dtx_rt_stop_tesd_req (void);

#endif /* !UART_RTS_H */

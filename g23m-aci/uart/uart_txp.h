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
|  Purpose :  Definitions for uart_txp.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_TXP_H
#define UART_TXP_H


EXTERN void tx_uart_driver_sent_ind ( T_UART_DATA* uart_device );

#ifdef _SIMULATION_
EXTERN void tx_dti_ready_ind ( T_DTI2_READY_IND *dti_ready_ind );

EXTERN void tx_writedata (UBYTE caller);
#endif /* _SIMULATION_ */

#endif /* !UART_TXP_H */

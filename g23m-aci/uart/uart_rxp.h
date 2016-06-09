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
|  Purpose :  Definitions for uart_rxp.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RXP_H
#define UART_RXP_H


EXTERN void rx_uart_driver_received_ind ( T_UART_DATA* uart_device );

#ifdef _SIMULATION_
EXTERN void rx_dti_data_test_ind ( T_DTI2_DATA_TEST_IND *dti_data_test_ind );

EXTERN void rx_readdata (UBYTE caller);

EXTERN USHORT rx_inpavail (UBYTE caller);
#endif /* _SIMULATION_ */

#endif /* !UART_RXP_H */

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
|  Purpose :  Definitions for uart_rxf.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_RXF_H
#define UART_RXF_H


EXTERN void rx_init (void);

EXTERN void rx_readOutFunc_0 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          nsource,
                              UBYTE         *source[],
                              USHORT        *size,
                              ULONG          state);

#ifdef FF_TWO_UART_PORTS
EXTERN void rx_readOutFunc_1 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          nsource,
                              UBYTE         *source[],
                              USHORT        *size,
                              ULONG          state);
#endif /* FF_TWO_UART_PORTS */
#endif /* !UART_RXF_H */


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
|  Purpose :  Definitions for uart_txf.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_TXF_H
#define UART_TXF_H


EXTERN void tx_init (void);

EXTERN void tx_flushUart (void);

EXTERN void tx_next_send_allowed (void);

EXTERN void tx_writeInFunc_0 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          ndest,
                              UBYTE         *dest[],
                              USHORT        *size);

#ifdef FF_TWO_UART_PORTS
EXTERN void tx_writeInFunc_1 (BOOL           cldFromIrq,
                              T_reInstMode  *reInstall,
                              UBYTE          ndest,
                              UBYTE         *dest[],
                              USHORT        *size);
#endif /* FF_TWO_UART_PORTS */

#endif /* !UART_TXF_H */


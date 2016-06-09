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
|  Purpose :  Definitions for uart_drxf.c 
+----------------------------------------------------------------------------- 
*/ 

#ifndef UART_DRXF_H
#define UART_DRXF_H


EXTERN void drx_free_resources (void);

EXTERN void drx_init (void);

GLOBAL UBYTE drx_get_channel_id(void);

#endif /* !UART_DRXF_H */


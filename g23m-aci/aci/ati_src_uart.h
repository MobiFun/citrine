/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\ati_src_uart.h
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#ifndef ATI_SRC_UART_H
#define ATI_SRC_UART_H

EXTERN BOOL uart_src_proc_chars (UBYTE *chars, 
                                 USHORT len, 
                                 T_ACI_DTI_PRC *prc_cmd);

EXTERN T_ATI_RESULT_CB      uart_src_result_cb;
EXTERN T_ATI_LINE_STATE_CB  uart_src_line_state_cb;

EXTERN USHORT format_output (UBYTE *src, USHORT src_len, UBYTE *dst,
                             T_ATI_OUTPUT_TYPE output_type);

EXTERN T_ACI_LIST *uart_src_params;

EXTERN void ati_switch_mode(UBYTE srcId, T_ATI_IO_MODE mode);

EXTERN void uart_send_buffer_data(UBYTE src_id);

#endif /* ATI_SRC_UART_H */

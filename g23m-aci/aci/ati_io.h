/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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

#ifndef ATI_IO_H
#define ATI_IO_H

EXTERN void  io_sendChar ( CHAR a, UBYTE srcId );

EXTERN UBYTE uart_new_source( UBYTE device, UBYTE dlci );
#ifdef UART
EXTERN void uart_InitCmdStruct( T_ACI_DTI_PRC *cmd_struct );
#endif
#define IS_END_CRLF_OUTPUT(x) (x & ATI_END_CRLF_OUTPUT)
#define IS_BEGIN_CRLF_OUTPUT(x) (x & ATI_BEGIN_CRLF_OUTPUT)

#define IS_ECHO_OUTPUT(x) (x & ATI_ECHO_OUTPUT)
#define IS_NORMAL_OUTPUT(x) (x & ATI_NORMAL_OUTPUT)
#define IS_CONFIRM_OUTPUT(x) (x & ATI_CONFIRM_OUTPUT)
#define IS_INDICATION_OUTPUT(x) (x & ATI_INDICATION_OUTPUT)
#define IS_FORCED_OUTPUT(x) (x & ATI_FORCED_OUTPUT)
#define IS_ERROR_OUTPUT(x) (x & ATI_ERROR_OUTPUT)
#define IS_INFORMATION_TEXT_OUTPUT(x) (x & ATI_INFORMATION_TEXT_OUTPUT)
#define IS_RESULT_CODE_OUTPUT(x) (x & ATI_RESULT_CODE_OUTPUT)
#endif

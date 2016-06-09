/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\ati_src_sat.h
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
#ifdef SIM_TOOLKIT

#ifndef ATI_SRC_SAT_H
#define ATI_SRC_SAT_H

EXTERN BOOL sat_src_proc_chars (UBYTE *chars);

EXTERN T_ATI_RESULT_CB  sat_src_result_cb;
//EXTERN T_ATI_LINE_STATE_CB  sat_src_line_state_cb;

EXTERN void sat_new_source( void );
EXTERN BOOL ati_execute_sat_cmd (T_ATI_SRC_PARAMS *src_params,
                                 UBYTE *chars, USHORT len);

#endif /* ATI_SRC_UART_H */

#endif /* SIM_TOOLKIT */

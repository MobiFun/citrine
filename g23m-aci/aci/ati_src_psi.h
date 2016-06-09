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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#ifdef FF_PSI

#ifndef ATI_SRC_PSI_H
#define ATI_SRC_PSI_H

EXTERN BOOL psi_src_proc_chars (UBYTE *chars, USHORT len, T_ACI_DTI_PRC_PSI *prc_cmd);
EXTERN void psi_erase_source( UBYTE srcId );
EXTERN T_ATI_RESULT_CB      psi_src_result_cb;
EXTERN T_ATI_LINE_STATE_CB  psi_src_line_state_cb;
EXTERN UBYTE psi_new_source( U32 devId, UBYTE dlci , U32 dio_driver_type);/* RM 26-05-04 Nice */

EXTERN USHORT format_output (UBYTE *src, USHORT src_len, UBYTE *dst,
                             T_ATI_OUTPUT_TYPE output_type);

EXTERN T_ACI_LIST *psi_src_params;

EXTERN void ati_switch_mode(UBYTE srcId, T_ATI_IO_MODE mode);

EXTERN void psi_send_buffer_data(UBYTE src_id);

EXTERN void psi_buffer_data (T_ACI_DTI_PRC_PSI *src_infos, T_desc2 *send_data);

#ifdef _SIMULATION_
EXTERN void psi_src_test_sendString (UBYTE *string, USHORT string_len,
                             T_ACI_DTI_PRC_PSI     *src_infos,
                             T_ATI_OUTPUT_TYPE output_type);
#endif /*_SIMULATION_*/
#endif /*ATI_SRC_PSI_H*/
#endif /*FF_PSI*/
/*==== EOF =======================================================*/


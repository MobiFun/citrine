/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This header file consists of the prototype for aci_bat.c
|
+----------------------------------------------------------------------------- 
*/ 
#ifndef ACI_BAT_H
#define ACI_BAT_H

#include "bat_ctrl.h" /* from the bat library to see T_BATC_signal */

#ifndef ACI_BAT_CMH_H
#include "aci_bat_cmh.h"
#endif


#ifdef _SIMULATION_
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __TODO__ __FILE__ "("__STR1__(__LINE__)") : ToDo : "
#else
#define __TODO__
#endif

#define ACI_BAT_MAX_RESPONSES MAX_PB_ENTR /* for CPBR and similiar multi line response commands */

GLOBAL void aci_bat_send_buffer_dti_data (UBYTE src_id);
GLOBAL void aci_bat_check_for_multi_line_response_command(UBYTE src_id);
GLOBAL void aci_bat_rcv      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_DTI2_DATA_IND *dti_data_ind);
GLOBAL void aci_bat_ctrl_rcv (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BATC_signal *signal);
GLOBAL void aci_bat_send     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_response *resp);
GLOBAL T_ACI_BAT_RSLT     aci_bat_wrapper     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_BAT_ctrl_params  aci_bat_cur_cmd_get (T_ACI_DTI_PRC_PSI *src_infos_psi);
GLOBAL T_BAT_plus_cscs_cs aci_bat_cs_get      (T_ACI_DTI_PRC_PSI *src_infos_psi);
GLOBAL void               aci_bat_cs_set      (T_ACI_DTI_PRC_PSI *src_infos_psi,T_BAT_plus_cscs_cs cs);
GLOBAL U8                 aci_bat_get_l2p_id  (U8 src_id);
GLOBAL void               aci_bat_run_cmd     (T_ACI_DTI_PRC_PSI *src_infos_psi);

#endif


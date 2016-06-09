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
|  Purpose :  This header file consists of the prototype for
|
|             aci_bat_bas.c
|             aci_bat_cc.c
|             aci_bat_cphs.c
|             aci_bat_csc.c
|             aci_bat_dti.c
|             aci_bat_em.c
|             aci_bat_fd.c
|             aci_bat_gprs.c
|             aci_bat_lcs.c
|             aci_bat_me.c
|             aci_bat_mm.c
|             aci_bat_phb.c
|             aci_bat_sat.c
|             aci_bat_sim.c
|             aci_bat_sms.c
|             aci_bat_ss.c
|
+----------------------------------------------------------------------------- 
*/ 
#ifndef ACI_BAT_CMH_H
#define ACI_BAT_CMH_H

#include "aci_cmh.h"
#include "psa.h"
#include "cmh.h"      /* extern definition of aciErrDesc */
#include "p_bat.h"
#include "psa_psi.h"  /* is protected with FF_PSI */

typedef enum
{
  ACI_BAT_FAIL = AT_FAIL,  /* the same as in T_ACI_RETURN */
  ACI_BAT_CMPL = AT_CMPL,
  ACI_BAT_EXCT = AT_EXCT,
  ACI_BAT_BUSY = AT_BUSY
} T_ACI_BAT_RSLT;

#define ACI_BAT_GET_ERR_CLASS() ((aciErrDesc & 0xFFFF0000) >> 16)
#define ACI_BAT_GET_ERR_VALUE()  (aciErrDesc & 0x0000FFFF)
#define ACI_BAT_RESET_ERR_DESC() (aciErrDesc = 0xFFFFFFFF)
#define ACI_BAT_VALID_ERR_DESC() (aciErrDesc NEQ 0xFFFFFFFF)

GLOBAL T_ACI_BAT_RSLT BAT_A                (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_O                (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_Z                (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_D                (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_H                (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_ANDC             (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT BAT_ANDF             (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentALS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentALS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentALS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef TI_PS_FF_AT_P_CMD_ATR
GLOBAL T_ACI_BAT_RSLT qBAT_PercentATR      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_P_CMD_ATR */
GLOBAL T_ACI_BAT_RSLT qBAT_PercentBAND     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentBAND     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentBAND     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCCBS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCCBS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGAATT   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGAATT   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGCLASS  (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGCLASS  (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGMM     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGPCO    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGPPP    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGPPP    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGREG    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCHLD     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCHPL     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCNAP     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCOPS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCOPS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCOPS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPALS    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPALS    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPCFU    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPHS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPHS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPINF    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPINF    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPMB     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPMB     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCPMBW    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPMBW    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPNUMS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCPNUMS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPOPN    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPRSM    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPRSM    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPVWI    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCREG     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCSQ      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCTTY     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCTTY     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef TI_PS_FF_AT_P_CMD_CUST
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCUST     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCUST     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_P_CMD_CUST */
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCWUP     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentDATA     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentDATA     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentDINF     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentEFRSLT   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSIMEF   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSIMEF   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentEM       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBO         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCLASS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCR         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFEA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFFC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIE         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFMS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFNS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPW         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFRQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFSA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentPPP      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentPPP      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentPVRF     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentPVRF     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentRDL      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentRDL      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef TI_PS_FF_AT_P_CMD_RDLB
GLOBAL T_ACI_BAT_RSLT sBAT_PercentRDLB     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentRDLB     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATC     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSATC     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATCC    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSATCC    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATE     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATR     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATT     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSNCNT    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSNCNT    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentVTS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCACM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCACM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCAMM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCAMM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCAOC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCBC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef FAX_AND_DATA
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCBST        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCBST        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* FAX_AND_DATA */
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCFC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCLK        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCCLK        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCCUG        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCUG        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCWA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCEER        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCFUN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCFUN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGACT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGACT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGANS       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGATT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGATT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGCLASS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGCLASS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDATA      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGDCONT     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDCONT     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef REL99
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGCMOD      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDSCONT    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGDSCONT    (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQREQ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGEQREQ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQMIN     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGEQMIN     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQNEG     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGTFT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGTFT       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGPADDR     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCGPADDR     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGQMIN      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGQMIN      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGQREQ      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGQREQ      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGREG       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGSMS       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGSMS       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCGSMS       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCIMI        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCIND        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCIND        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLAN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCLAN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLAN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLCC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLCK        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLIP        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCDIP        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLIR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLIR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLVL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLVL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMER        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMER        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGD        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGW        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMOD        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMOD        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMSS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMUT        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMUT        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMUX        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMUX        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNMA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCNMI        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNMI        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNUM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCOLP        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCOPS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCOPS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCOPS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPAS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBF        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBF        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentPBCF     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentPBCF     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPBS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPBS     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBW        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBW        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPIN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPIN        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPMS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPMS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPUC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPUC        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPWD        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRES        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCRES        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRLP        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCRLP        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRSM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSAS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCSAS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCB        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCB        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSIM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSMS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSMS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSNS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSNS        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSTA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSTA        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSVM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSVM        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCTFR        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCTZU        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCTZU        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCUSD        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusDS          (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusDS          (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFAP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFAP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBO         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBO         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFBU         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFBU         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCLASS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCLASS      (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCR         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCR         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFCT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFCT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFFC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFFC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFHS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFHS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIE         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIE         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFIT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFIT         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLO         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLO         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFLP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFLP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFMS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFMS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFNS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFNS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPI         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFPW         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFPW         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFRQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFRQ         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFSA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFSA         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusFSP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusFSP         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusICF         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusICF         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusIFC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusIFC         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusIPR         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PlusIPR         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusVTS         (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#ifdef TI_PS_FF_AT_CMD_WS46
GLOBAL T_ACI_BAT_RSLT qBAT_PlusWS46        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusWS46        (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_CMD_WS46 */
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGR     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGL     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMI       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMM       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMR       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGSN       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCTZV     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCNIV     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCNIV (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);

#ifdef SIM_PERS
GLOBAL T_ACI_BAT_RSLT sBAT_PercentMEPD     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* SIM_PERS */
#ifdef REL99
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGRS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCMGRS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif
#ifdef TI_PS_FF_AT_CMD_P_ECC
GLOBAL T_ACI_BAT_RSLT sBAT_PercentECC     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
#endif /* TI_PS_FF_AT_CMD_P_ECC */
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCSQ     (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCHUP       (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd);
/*
 * multi response line commands have a "re entrant" callback in front of the actual command handler
 */
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBR_CB (T_ACI_DTI_PRC_PSI *src_infos_psi);
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBF_CB (T_ACI_DTI_PRC_PSI *src_infos_psi);

#endif

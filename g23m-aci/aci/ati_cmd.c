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
|  Purpose :  AT Command Interpreter: Processing AT-Commands strings.
+-----------------------------------------------------------------------------
*/
#ifndef ACI_CMD_C
#define ACI_CMD_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "dti.h"      /* functionality of the dti library */
#include "line_edit.h" /* SKA 2002-09-05 */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "psa.h"
#include "cmh.h"

#include "aci_lst.h"
#include "dti_conn_mng.h"
#ifdef UART
#include "psa_uart.h"
#endif
#include "ati_io.h"
#include "aci_mem.h"

#ifdef SIM_TOOLKIT
#include "ati_src_sat.h"
#include "psa_cc.h"
#include "psa_sat.h"
#endif /* SIM_TOOLKIT */

#ifdef GPRS
#include "gaci_cmh.h"
#include "gaci_cmd.h"
#endif /* GPRS */

#include "aci_prs.h"


#ifndef _SIMULATION_
#ifdef UART
#include "cmh_uart.h"
#endif
#endif

#include "psa_sms.h"
#include "aci.h"
#include "ati_ext_mech.h"

#ifdef FF_ATI_BAT
#include "ati_bat.h"
#include "aci_bat.h"
#endif

#define OLD_NON_STANDARDIZED_ATCMD "&ABDEFHILMOPQSTVWXZ"

/*==== EXTERNALS ======================================================*/
EXTERN T_SMS_SHRD_PRM smsShrdPrm;
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;

/*==== LOCALS =========================================================*/
LOCAL  BOOL        aci_SrchFwrdLst ( UBYTE srcId ,CHAR* searchFor );
LOCAL  T_ATI_RSLT  aci_FwrdToEXT   ( T_ATI_SRC_PARAMS *src_params, T_LEDIT_ATCMD *cmd);
LOCAL  BOOL        aci_cmhActive   ( UBYTE srcId     );

LOCAL void send_output_type_signal (T_ATI_SRC_PARAMS *src_params, UBYTE output_type);
LOCAL void check_and_send_output_type_signal (T_ATI_SRC_PARAMS *src_params, CHAR *cmd);

LOCAL  SHORT find_index(CHAR * cl);

/*==== EXPORT ======================================================*/

EXTERN CHAR  *cmdCmsError ( T_ACI_CMS_ERR e );
EXTERN CHAR  *cmdCmeError ( T_ACI_CME_ERR err );
EXTERN CHAR  *cmdAtError (AtErrCode e);
EXTERN void io_sendChar (CHAR  a, UBYTE srcId);

EXTERN T_ATI_RSLT atPercentCPRIM (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCLAC (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusGCI(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusGCI(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCCWE(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCCWE(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCGMI ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT atPlusCGMM ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT atPlusCGMR ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT atPlusCGSN ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT atPlusCSAS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSAS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSAS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLVL ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT tesatPlusCLVL ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPlusCLVL ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentCCBS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCCBS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCMGL(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMGL(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMGL(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT aciPrcsPlusCG(CHAR* cl, UBYTE srcId);
EXTERN T_ATI_RSLT setaciPrcsVolLevel(CHAR* cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesaciPrcsVolLevel(CHAR* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queaciPrcsVolLevel(CHAR* cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCOPS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPOL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCPOL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCPOL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCOPN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCOPN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCFUN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCFUN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPIN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCPIN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPAS (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusCLAE (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLAE (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusCLIP (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCDIP (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLIP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCDIP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLIR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLIR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCOLP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCOLP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCMOD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMOD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSTA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSTA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCHUP (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLCK (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPWD (CHAR *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_SECP
EXTERN T_ATI_RSLT setatPercentSECP  (CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_SECP */
#ifdef TI_PS_FF_AT_P_CMD_SECS
EXTERN T_ATI_RSLT setatPercentSECS (CHAR *cl, UBYTE srcId);
EXTERN  T_ATI_RSLT quetatPercentSECS (CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_SECS */
EXTERN T_ATI_RSLT setatPlusCREG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCREG (CHAR *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPercentCREG (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPercentCREG (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCRES (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCRES (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCRES (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCRSL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCRSL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCRSL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCCUG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCCUG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCTFR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCCFC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCCWA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCCWA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCRC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCR   (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCRC  (CHAR *cl, UBYTE srcId);
#ifdef FAX_AND_DATA
EXTERN T_ATI_RSLT setatPlusCBST (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCBST (CHAR *cl, UBYTE srcId);
#endif /* FAX_AND_DATA */
EXTERN T_ATI_RSLT setatPlus_PercentCHLD(char *cl, UBYTE srcId, T_ACI_CHLD_CMD cmdType);
EXTERN T_ATI_RSLT setatPlusCHLD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCHLD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCEER (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCSSD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSSN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSSN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCSSN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCSSN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCUSD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCUSD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSNS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSNS (CHAR *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_CMD_WS46
EXTERN T_ATI_RSLT setatPlusWS46 (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusWS46 (CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_CMD_WS46 */
EXTERN T_ATI_RSLT setatPlusCIMI(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLCC(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCLCC(char *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_DBGINFO
EXTERN T_ATI_RSLT setatPercentDBGINFO(char *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_DBGINFO */
EXTERN T_ATI_RSLT setatPlusCSQ  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSQ  (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCSQ (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCSQ (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCSQ (CHAR *cl, UBYTE srcId);

#ifdef SIM_PERS
 /*For %MEPD -for querying ME Personalisation Data... Added on 11/03/2005*/
EXTERN T_ATI_RSLT  setatPercentMEPD(CHAR *cl, UBYTE srcId);
#endif

EXTERN T_ATI_RSLT atPlusCBC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCMUT (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMUT (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMUT (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCNUM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCRSM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSIM (CHAR *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_ATR
EXTERN T_ATI_RSLT queatPercentATR ( CHAR *cl, UBYTE srcId );
#endif /* TI_PS_FF_AT_P_CMD_ATR */
EXTERN T_ATI_RSLT atPlusCSMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCPMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCPMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCPMS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCMGF (CHAR *cl, UBYTE srcId);
#if defined FF_MMI_RIV
EXTERN T_ATI_RSLT setatPercentSMBS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSMBS (CHAR *cl, UBYTE srcId);
#endif /* FF_MMI_RIV */
EXTERN T_ATI_RSLT tesatPlusCMGF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMGF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCSCA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSCA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSCA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCSDH (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSDH (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCSMP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCSMP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSMP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSDH (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSDH (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCSCS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSCS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCSCB (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSCB (CHAR *cl, UBYTE srcId);
#ifdef FF_HOMEZONE
EXTERN T_ATI_RSLT setatPercentCBHZ (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCBHZ (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCBHZ (CHAR *cl, UBYTE srcId);
#endif /* FF_HOMEZONE */
EXTERN T_ATI_RSLT atPlusCMGR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMGR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMGR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCMSS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCMGD (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCNMI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCNMI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCNMI (CHAR *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
EXTERN T_ATI_RSLT setatPercentCPRSM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPRSM (CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
EXTERN T_ATI_RSLT setatPlusCMEE (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMEE (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPBS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCPBS (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentPBCF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentPBCF (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT queatPercentCPBS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPBW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCPBW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPBR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCPBR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPBF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCPBF (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusCSVM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCSVM (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusCLAN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLAN (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCLAN (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusVTS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentVTS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentTEST (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentNRG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentNRG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentNRG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atAndF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atAndC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atAndD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atAndK (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusGCAP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusIPR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusIPR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusICF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusICF (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusIFC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusIFC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFLO (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFLO (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusILRR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusILRR (CHAR *cl, UBYTE srcId);
/* AT commands for DTI managment */
#if defined (DTI)
EXTERN T_ATI_RSLT setatPercentDATA ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentDATA ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentDINF ( char *cl, UBYTE srcId );
#endif
/* trace function */
#ifdef FF_TRACE_OVER_MTST
EXTERN T_ATI_RSLT setatPercentMTST (CHAR  *cl, UBYTE srcId );
#endif /* FF_TRACE_OVER_MTST */

EXTERN T_ATI_RSLT setatPlusCMUX (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMUX (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMUX (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCUNS(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCUNS(char *cl, UBYTE srcId);

/*-- Advice of Charge Commands --  */
EXTERN T_ATI_RSLT setatPlusCAOC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCAOC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCACM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCACM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCAMM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCAMM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCPUC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCPUC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCACM (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCAOC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCTV (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCOLR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCPI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCTTY (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCTTY (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atBW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atD (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atEFLMQVX (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atH (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atTP(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atZ (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atAndW (char *cl, UBYTE srcId);

/*
 * AT commands for location service
 */
#if defined FF_EOTD
EXTERN T_ATI_RSLT setatPlusCLSA    (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLSA   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLOM   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLOM  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLPS    (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCLPS   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCLSR    (char *cl, UBYTE srcId);
#endif /* FF_EOTD */

#ifdef FAX_AND_DATA
EXTERN T_ATI_RSLT atO (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusDS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusDS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusDR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusDR (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCRLP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCRLP (CHAR *cl, UBYTE srcId);

/* --- Parameter Commands-------- */

#ifdef FF_FAX

EXTERN T_ATI_RSLT setatPlusFAP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFAP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFBS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFBU  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFBU  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFBO  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFBO  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFCC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFCR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFCT  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCT  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFCLASS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCLASS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFCQ  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFCQ  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFDR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFDT  (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusFEA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFEA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFFC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFFC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFHS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFIT  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFIT  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFIE  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFIE  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFIP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFIS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFIS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFKS  (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT atPlusFMI  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusFMM  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusFMR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFMS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFMS  (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusFLI  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFLI  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFLP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFLP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFND  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFND  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFNR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFNR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFNS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFNS  (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusFPA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFPA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFPI  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFPI  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFPS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFPS  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFPW  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFPW  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFRQ  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFRQ  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFSA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFSA  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusFSP  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusFSP  (CHAR *cl, UBYTE srcId);

#endif /* FF_FAX */

#endif /* FAX_AND_DATA */

#ifdef SIM_TOOLKIT

/* -- Action Commands------------- */

EXTERN T_ATI_RSLT atPercentSATE  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentSATC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentSATC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSATC  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentSATR  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentSATT  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentSIMEF  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSIMEF  (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentEFRSLT  (CHAR *cl, UBYTE srcId);
#endif /* SIM_TOOLKIT */

EXTERN T_ATI_RSLT atPlusCMGS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMGS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMGS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCMGW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMGW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMGW (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCMGC (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPlusCNMA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCNMA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCNMA (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCMGC (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT atPercentVER (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCNAP(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCNAP(char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentALS ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT tesatPercentALS ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentALS ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentBAND( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT tesatPercentBAND( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentBAND( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentCPRI( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentCPRI( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPlusCTZR( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPlusCTZR( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPlusCTZU( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPlusCTZU( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPlusCCLK (char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPlusCCLK (char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentCTZV( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentCTZV( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentCNIV( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentCNIV( char *cl, UBYTE srcId );

EXTERN T_ATI_RSLT setatPercentDAR( char *cl, UBYTE srcId );

/* EXTERN T_ATI_RSLT queatPercentDAR( char *cl, UBYTE srcId ); */
#ifdef TI_PS_FF_AT_P_CMD_CSCN
EXTERN T_ATI_RSLT setatPercentCSCN( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentCSCN( char *cl, UBYTE srcId );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

#ifdef REL99
EXTERN T_ATI_RSLT setatPercentCMGRS ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentCMGRS ( char *cl, UBYTE srcId );
#endif

#if defined(FF_WAP) || defined(FF_PPP) || defined (FF_SAT_E)
EXTERN T_ATI_RSLT atPercentPPP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentPPP (CHAR *cl, UBYTE srcId);
#endif /* defined(WAP) || defined(FF_PPP) || defined (FF_SAT_E)*/

#if defined(FF_WAP) || defined (FF_SAT_E)
EXTERN T_ATI_RSLT atPercentWAP (CHAR *cl, UBYTE srcId);
#endif /* WAP or SAT E */

#ifdef GPRS
/*
 * declaration of functions
 * for GSM 07.07 version 6.4.0 Release 1997
 */
EXTERN T_ATI_RSLT setatPlusCGDCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGQREQ  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGQMIN  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGATT   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGACT   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGDATA  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGPADDR (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGAUTO  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGANS   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGCLASS (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGEREP  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGREG   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGSMS   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCGEREP  (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT queatPlusCGDCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGQREQ  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGQMIN  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGATT   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGACT   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGAUTO  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGCLASS (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGEREP  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGREG   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGSMS   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCGEREP  (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT tesatPlusCGDCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGPADDR (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGSMS   (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGAATT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCGAATT (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentSNCNT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSNCNT (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGMM (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGPCO (char *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGPPP (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCGPPP (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGREG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCGREG (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCGCLASS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCGCLASS (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCREG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCREG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCREG (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCREG (CHAR *cl, UBYTE srcId);
#ifdef REL99
EXTERN T_ATI_RSLT setatPlusCGEQREQ  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGEQMIN  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGEQNEG  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGCMOD   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGEQREQ  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGEQMIN  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGEQNEG  (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGCMOD   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGDSCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGDSCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGDSCONT (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGTFT    (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCGTFT    (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPlusCGTFT    (char *cl, UBYTE srcId);
#endif  /* REL99 */
#endif  /* GPRS */

#ifdef FF_EM_MODE
/*
    AT commands for the engineering mode
*/
EXTERN T_ATI_RSLT setatPercentEM   (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentEMET (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentEMETS(char *cl, UBYTE srcId);
#endif /* FF_EM_MODE */

#ifdef FF_CPHS
EXTERN T_ATI_RSLT setatPercentCPHS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPHS(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPNUMS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCPNUMS(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPALS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPALS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCPALS(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPVWI(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT queatPercentCPOPN(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPINF(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPINF(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPMB(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCPMB(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPMBW(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT tesatPercentCPMBW(CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPercentCPCFU(CHAR *cl, UBYTE srcId);
#endif /* FF_CPHS */

EXTERN T_ATI_RSLT setatPercentCHPL ( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentPVRF( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT queatPercentPVRF( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentCWUP( char *cl, UBYTE srcId );

EXTERN T_ATI_RSLT queatPercentSIMIND( char *cl, UBYTE srcId );
EXTERN T_ATI_RSLT setatPercentSIMIND( char *cl, UBYTE srcId );

LOCAL  T_ATI_RSLT test_gen(CHAR *cl, UBYTE srcId);
#if defined MFW AND defined TI_PS_FF_AT_P_CMD_MMITEST
/* MMI TEST */
EXTERN T_ATI_RSLT setatPercentMMITEST (char *cl, UBYTE srcId);
#endif

/* AT commands for redialling */
EXTERN T_ATI_RSLT setatPercentRDL (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentRDL (char* cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_RDLB
EXTERN T_ATI_RSLT setatPercentRDLB (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentRDLB (char* cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

/* AT command for reporting States*/
EXTERN T_ATI_RSLT setatPercentCSTAT (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCSTAT (char* cl, UBYTE srcId);

EXTERN T_ATI_RSLT setatPlusCIND (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCIND (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCMER (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMER (char *cl, UBYTE srcId);
#ifdef TI_PS_FF_AT_P_CMD_CUST
EXTERN T_ATI_RSLT setatPercentCUST (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentCUST (char* cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_CUST */
EXTERN T_ATI_RSLT setatPercentSATCC (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSATCC (char* cl, UBYTE srcId);

#ifdef FF_DUAL_SIM
EXTERN T_ATI_RSLT setatPercentSIM (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentSIM (char* cl, UBYTE srcId);
#endif /*FF_DUAL_SIM*/

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
EXTERN T_ATI_RSLT setatPercentCUSCFG (char* cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */
#ifdef TI_PS_FF_AT_P_CMD_CUSDR
EXTERN T_ATI_RSLT setatPercentCUSDR  (char* cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_CUSDR */

EXTERN T_ATI_RSLT setatPlusCMMS (char* cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCMMS (char* cl, UBYTE srcId);

#ifdef TI_PS_FF_AT_P_CMD_STDR
EXTERN T_ATI_RSLT setatPercentSTDR ( char *cl, UBYTE srcId );
#endif /* TI_PS_FF_AT_P_CMD_STDR */
EXTERN T_ATI_RSLT setatPercentCMGMDU ( char *cl, UBYTE srcId );

EXTERN T_ATI_RSLT atPercentCMGL (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT atPercentCMGR (CHAR *cl, UBYTE srcId);

EXTERN T_ATI_RSLT tesatPercentCMGL(CHAR *cl, UBYTE srcId);

#ifdef FF_CPHS_REL4
EXTERN T_ATI_RSLT setatPercentCFIS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentMWIS(CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentMWI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPercentMWI (CHAR *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentMBDN (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentMBI (char *cl, UBYTE srcId);
#endif /* FF_CPHS_REL4 */

#ifdef TI_PS_FF_AT_CMD_P_ECC
EXTERN T_ATI_RSLT setatPercentECC (CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_CMD_P_ECC */

#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN T_ATI_RSLT setatPercentCTREG(CHAR *cl, UBYTE srcId);
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

EXTERN T_ATI_RSLT setatPlusCVHU( char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCVHU( char *cl, UBYTE srcId);

LOCAL const ATCommand_bas cmds_bas[] =
{
  {"A", atA,       0,       0     },
  {"B", atBW,      0,       0     },
  { 0,  0,        "&C",     atAndC},
  {"D", atD,      "&D",     atAndD},
  {"E", atEFLMQVX, 0,       0     },
  {"F", atEFLMQVX,"&F",     atAndF},
  { 0,  0,         0,       0     },
  {"H", atH,       0,       0     },
  {"I", atI,       0,       0     },
  { 0,  0,         0,       0     },
  { 0,  0,        "&K",     atAndK},
  {"L", atEFLMQVX, 0,       0     },
  {"M", atEFLMQVX, 0,       0     },
  { 0,  0,         0,       0     },
#ifdef FAX_AND_DATA    /* Fax and Data Module only */
  {"O", atO,       0,       0     },
#else
  { 0,  0,         0,        0     },
#endif
  {"P", atTP,      0,       0     },
  {"Q", atEFLMQVX, 0,       0     },
  { 0,  0,         0,       0     },
  {"S", atS,       0,       0     },
  {"T", atTP,      0,       0     },
  { 0,  0,         0,       0     },
  {"V", atEFLMQVX, 0,       0     },
  {"W", atBW,     "&W",     atAndW},
  {"X", atEFLMQVX, 0,       0     },
  { 0,  0,         0,       0     },
  {"Z", atZ,       0,       0     }
};


#ifdef FAX_AND_DATA
#define FD_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#ifdef V42BIS
#define V42_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define V42_CMD(a,b,c,d,e,f)
#endif /* V42BIS */
#else
#define FD_CMD(a,b,c,d,e,f)
#define V42_CMD(a,b,c,d,e,f)
#endif /* FAX_AND_DATA */

#if defined (SIM_TOOLKIT)
#define SAT_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define SAT_CMD(a,b,c,d,e,f)
#endif /* SIM_TOOLKIT */

#if defined (FF_WAP)
#define WAP_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define WAP_CMD(a,b,c,d,e,f)
#endif  /* WAP */

#if defined FF_EOTD
#define LOC_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define LOC_CMD(a,b,c,d,e,f)
#endif

#ifdef GPRS
  /* n.b. context is packet data equivalent of a cct switched call  */
  /* existing +H,+D,+S,+ABORT and +CFUNC are also affected*/
#define GPRS_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define GPRS_CMD(a,b,c,d,e,f)
#endif /* GPRS */

#ifdef FF_EM_MODE
#define EM_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define EM_CMD(a,b,c,d,e,f)
#endif /* FF_EM_MODE */

#ifdef FF_CPHS
#define CPHS_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define CPHS_CMD(a,b,c,d,e,f)
#endif /* FF_CPHS */

#ifdef FF_TRACE_OVER_MTST
#define MTST_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define MTST_CMD(a,b,c,d,e,f)
#endif /* FF_TRACE_OVER_MTST */

#ifdef FF_HOMEZONE
#define HZ_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define HZ_CMD(a,b,c,d,e,f)
#endif /* FF_HOMEZONE */

#ifdef MFW
#define MFW_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define MFW_CMD(a,b,c,d,e,f)
#endif /* MFW */

#ifdef FF_TTY
#define TTY_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define TTY_CMD(a,b,c,d,e,f)
#endif /* FF_TTY */

#if defined FF_MMI_RIV
#define SMBS_CMD(a,b,c,d,e,f) {a,b,c,d,e,f},
#else
#define SMBS_CMD(a,b,c,d,e,f)
#endif /* FF_MMI_RIV */

#if defined (SMS_PDU_SUPPORT)
#define CMGF_TEST_OUTPUT "%s: (0,1)"
#else
#define CMGF_TEST_OUTPUT "%s: (1)"
#endif /* SMS_PDU_SUPPORT */

LOCAL const ATCommand cmds [] =
{
    {NULL,AT_CMD_NONE,NULL,NULL,NULL,NULL},
    {"+CACM",       AT_CMD_CACM,   setatPlusCACM,    test_gen,       queatPlusCACM,    0},
    {"+CAMM",       AT_CMD_CAMM,   setatPlusCAMM,    test_gen,       queatPlusCAMM,    0},
    {"+CAOC",       AT_CMD_CAOC,   setatPlusCAOC,    test_gen,       queatPlusCAOC,    "%s: (0-2)"},
    {"+CBC",        AT_CMD_CBC,    atPlusCBC,        test_gen,       0,                "%s: (0-3),(0-100)"},
    FD_CMD("+CBST", AT_CMD_CBST,   setatPlusCBST,    test_gen,       queatPlusCBST,    "%s: (0-7,12,14,65,66,68,70,71,75),(0),(0-3)")
    {"+CCFC",       AT_CMD_CCFC,   setatPlusCCFC,    test_gen,       0,                "%s: (0-5)"},
    {"+CCUG",       AT_CMD_CCUG,   setatPlusCCUG,    test_gen,       queatPlusCCUG,    "%s: (0,1),(0-10),(0-3)"},
    {"+CCWA",       AT_CMD_CCWA,   setatPlusCCWA,    test_gen,       queatPlusCCWA,    "%s: (0,1)"},
    {"+CCWE",       AT_CMD_CCWE,   setatPlusCCWE,    test_gen,       queatPlusCCWE,    "%s: (0,1)"},
    {"+CEER",       AT_CMD_CEER,   atPlusCEER,       test_gen,       0,                0},
    {"+CFUN",       AT_CMD_CFUN,   setatPlusCFUN,    test_gen,       queatPlusCFUN,    "%s: (0,1,4),(0)"},
GPRS_CMD("+CGACT",  AT_CMD_CGACT,  setatPlusCGACT,   test_gen,       queatPlusCGACT,   "%s: (0,1)")            /* GPRS: context activation/deactivation */
GPRS_CMD("+CGANS",  AT_CMD_CGANS,  setatPlusCGANS,   test_gen,       0,                "%s: (0,1),(\"PPP\")")  /* GPRS: answer NW context req */
GPRS_CMD("+CGATT",  AT_CMD_CGATT,  setatPlusCGATT,   test_gen,       queatPlusCGATT,   "%s: (0,1)")            /* GPRS: GPRS attach/detach */
GPRS_CMD("+CGAUTO", AT_CMD_CGAUTO, setatPlusCGAUTO,  test_gen,       queatPlusCGAUTO,  "%s: (0-3)")            /* GPRS: auto answer/modem comp control */
GPRS_CMD("+CGCLASS",AT_CMD_CGCLASS,setatPlusCGCLASS, test_gen,       queatPlusCGCLASS, "%s: (\"B\",\"CG\",\"CC\")")  /* GPRS: GPRS class change */
GPRS_CMD("+CGDATA", AT_CMD_CGDATA, setatPlusCGDATA,  test_gen,       0,                "%s: \"PPP\"")          /* GPRS: data connect */
GPRS_CMD("+CGDCONT",AT_CMD_CGDCONT,setatPlusCGDCONT, tesatPlusCGDCONT,queatPlusCGDCONT,     0)               /* GPRS: PDP context definition */
GPRS_CMD("+CGEREP", AT_CMD_CGEREP, setatPlusCGEREP,  test_gen,       queatPlusCGEREP,  "%s: (0-2),(0,1)")      /* GPRS event reporting */
    {"+CGMI",       AT_CMD_CGMI,   atPlusCGMI,       test_gen,       0,                0},
    {"+CGMM",       AT_CMD_CGMM,   atPlusCGMM,       test_gen,       0,                0},
    {"+CGMR",       AT_CMD_CGMR,   atPlusCGMR,       test_gen,       0,                0},
GPRS_CMD("+CGPADDR",AT_CMD_CGPADDR,setatPlusCGPADDR, tesatPlusCGPADDR,       0,     0)                       /* GPRS: show PDP addresses */
GPRS_CMD("+CGQMIN", AT_CMD_CGQMIN, setatPlusCGQMIN,  test_gen,       queatPlusCGQMIN,  "%s: \"IP\",(1-3),(1-4),(1-5),(1-9),(1-18,31)") /* GPRS: context min QOS requirement */
GPRS_CMD("+CGQREQ", AT_CMD_CGQREQ, setatPlusCGQREQ,  test_gen,       queatPlusCGQREQ,  "%s: \"IP\",(1-3),(1-4),(1-5),(1-9),(1-18,31)") /* GPRS: context QOS request */
GPRS_CMD("+CGREG",  AT_CMD_CGREG,  setatPlusCGREG,   test_gen,       queatPlusCGREG,   "%s: (0-2)")            /* GPRS network registration status  */
GPRS_CMD("+CGSMS",  AT_CMD_CGSMS,  setatPlusCGSMS,   tesatPlusCGSMS, queatPlusCGSMS,   0)                    /* GPRS: define MO SMS behaviour */
#ifdef REL99
GPRS_CMD("+CGDSCONT",AT_CMD_CGDSCONT,setatPlusCGDSCONT, tesatPlusCGDSCONT,queatPlusCGDSCONT,0)
GPRS_CMD("+CGEQREQ", AT_CMD_CGEQREQ, setatPlusCGEQREQ,  test_gen,         queatPlusCGEQREQ, "%s: \"cid\",(0-4),(0-2048),(0-2048),(0-2048),(0-2048),(0-2),(0-1500/1502 for PPP),(1E1,1E2,7E3,1E3,1E4,1E5,1E6),(5E2,1E2,5E3,4E3,1E3,1E4,1E5,1E6),(0-3),(0-65535),(0-3)")
GPRS_CMD("+CGEQMIN", AT_CMD_CGEQMIN, setatPlusCGEQMIN,  test_gen,         queatPlusCGEQMIN, "%s: \"cid\",(0-3),(1-2048),(1-2048),(1-2048),(1-2048),(0-1),(0-1500/1502 for PPP),(1E1,1E2,7E3,1E3,1E4,1E5,1E6),(5E2,1E2,5E3,4E3,1E3,1E4,1E5,1E6),(0-2),(0-65535),(0-3)")
GPRS_CMD("+CGEQNEG", AT_CMD_CGEQNEG, setatPlusCGEQNEG,  tesatPlusCGEQNEG, 0,                0)
GPRS_CMD("+CGCMOD",  AT_CMD_CGCMOD,  setatPlusCGCMOD,   tesatPlusCGCMOD,  0,                0)
GPRS_CMD("+CGTFT",   AT_CMD_CGTFT,   setatPlusCGTFT,    tesatPlusCGTFT,   queatPlusCGTFT,   0)
#endif /* REL99 */
    {"+CGSN",       AT_CMD_CGSN,   atPlusCGSN,       test_gen,       0,                0},
    {"+CHLD",       AT_CMD_CHLD,   setatPlusCHLD,    test_gen,       0/*return cl*/,   "%s: (0,1,1x,2,2x,3,4)"},
    {"+CHUP",       AT_CMD_CHUP,   setatPlusCHUP,    test_gen,       0,                0},
    {"+CIMI",       AT_CMD_CIMI,   setatPlusCIMI,    test_gen,       0,                0},
    {"+CLAC",       AT_CMD_CLAC,   atPlusCLAC,       0,              0,                0},
    {"+CLAE",       AT_CMD_CLAE,   setatPlusCLAE,    test_gen,       queatPlusCLAE,    "%s: (0-1)"},
    {"+CLAN",       AT_CMD_CLAN,   setatPlusCLAN,    tesatPlusCLAN,  queatPlusCLAN,    0},
    {"+CLCC",       AT_CMD_CLCC,   setatPlusCLCC,    test_gen,       0,                0},
    {"+CLCK",       AT_CMD_CLCK,   setatPlusCLCK,    test_gen,       0,                "%s: (\"SC\",\"AO\",\"OI\",\"OX\",\"AI\",\"IR\",\"AB\",\"AG\",\"AC\",\"FD\",\"PS\",\"PN\",\"PU\",\"PP\",\"PC\",\"PF\",\"FC\",\"FM\",\"MU\",\"MM\",\"BP\",\"PL\",\"LA\",\"AL\")"},
    {"+CLIP",       AT_CMD_CLIP,   setatPlusCLIP,    test_gen,       queatPlusCLIP,    "%s: (0,1)"},
    {"+CDIP",       AT_CMD_CDIP,   setatPlusCDIP,    test_gen,       queatPlusCDIP,    "%s: (0,1)"},    
    {"+CLIR",       AT_CMD_CLIR,   setatPlusCLIR,    test_gen,       queatPlusCLIR,    "%s: (0,1,2)"},
    {"+CLVL",       AT_CMD_CLVL,   setatPlusCLVL,    tesatPlusCLVL,  queatPlusCLVL,    0},
    {"+CMEE",       AT_CMD_CMEE,   setatPlusCMEE,    test_gen,       queatPlusCMEE,    "%s: (0-2)"},
    {"+CMGC",       AT_CMD_CMGC,   atPlusCMGC,       tesatPlusCMGC,  0,                0},
    {"+CMGD",       AT_CMD_CMGD,   setatPlusCMGD,    test_gen,       0,                "%s: (0-255),(0-4)"},
    {"+CMGF",       AT_CMD_CMGF,   atPlusCMGF,       test_gen,       queatPlusCMGF,    CMGF_TEST_OUTPUT},
#ifdef SIM_PERS
    {"%MEPD",       AT_CMD_MEPD,   setatPercentMEPD,    test_gen,   0,                "%s: (\"MAX\",\"ALE\",\"RFM\",\"RFA\",\"RSM\",\"RSA\",\"TMF\",\"ETF\",\"AIF\")"},
    /*For %MEPD -for querying ME Personalisation Data... Added on 11/03/2005*/
#endif

SMBS_CMD("%SMBS",   AT_CMD_SMBS, setatPercentSMBS, test_gen,       queatPercentSMBS, "%s: (0,1)")
    {"+CMGL",       AT_CMD_CMGL, atPlusCMGL,       tesatPlusCMGL,  queatPlusCMGL,    "%s: \"REC UNREAD\", \"REC READ\", \"STO UNSENT\", \"STO SENT\", \"ALL\""},
    {"+CMGR",       AT_CMD_CMGR, atPlusCMGR,       test_gen,       test_gen,         0},
    {"+CMGS",       AT_CMD_CMGS, atPlusCMGS,       tesatPlusCMGS,  queatPlusCMGS,    0},
    {"+CMGW",       AT_CMD_CMGW, atPlusCMGW,       tesatPlusCMGW,  queatPlusCMGW,    0},
    {"+CMOD",       AT_CMD_CMOD, setatPlusCMOD,    test_gen,       queatPlusCMOD,    "%s: (0-3)"},
    {"+CMSS",       AT_CMD_CMSS, atPlusCMSS,       test_gen,       0,                0},
    {"+CMMS",       AT_CMD_CMMS, setatPlusCMMS,    test_gen,       queatPlusCMMS,    "%s: (0-2)"},
    {"+CMUT",       AT_CMD_CMUT, setatPlusCMUT,    tesatPlusCMUT,  queatPlusCMUT,    0},
    {"+CMUX",       AT_CMD_CMUX, setatPlusCMUX,    tesatPlusCMUX,  queatPlusCMUX,    0},
    {"+CNMA",       AT_CMD_CNMA, atPlusCNMA,       tesatPlusCNMA,  queatPlusCNMA,    0},
    {"+CNMI",       AT_CMD_CNMI, atPlusCNMI,       tesatPlusCNMI,  queatPlusCNMI,    0},
    {"+CNUM",       AT_CMD_CNUM, setatPlusCNUM,    test_gen,       0,                0},
    {"+COLP",       AT_CMD_COLP, setatPlusCOLP,    test_gen,       queatPlusCOLP,    "%s: (0,1)"},
    {"+COPN",       AT_CMD_COPN, setatPlusCOPN,    test_gen,       0,                0},
    {"+COPS",       AT_CMD_COPS, setatPlusCOPS,    tesatPlusCOPS,  queatPlusCOPS,    0},
    {"+CPAS",       AT_CMD_CPAS, setatPlusCPAS,    test_gen,       0,                "%s: (0-5)"},
    {"+CPBF",       AT_CMD_CPBF, setatPlusCPBF,    tesatPlusCPBF,  0,                0},
    {"+CPBR",       AT_CMD_CPBR, setatPlusCPBR,    tesatPlusCPBR,  0,                0},
    {"+CPBS",       AT_CMD_CPBS, setatPlusCPBS,    test_gen,       queatPlusCPBS,    "%s: (\"EN\",\"BD\",\"FD\",\"DC\",\"LD\",\"RC\",\"LR\",\"MT\",\"AD\",\"SM\",\"SD\",\"MC\",\"LM\",\"AF\",\"ON\",\"UD\")"},
    {"+CPBW",       AT_CMD_CPBW, setatPlusCPBW,    tesatPlusCPBW,  test_gen,         0},
    {"+CPIN",       AT_CMD_CPIN, setatPlusCPIN,    test_gen,       queatPlusCPIN,    0},
    {"+CPMS",       AT_CMD_CPMS, atPlusCPMS,       test_gen,       queatPlusCPMS,    "%s: (\"ME\",\"SM\"),(\"ME\",\"SM\"),(\"ME\",\"SM\")"},
    {"+CPOL",       AT_CMD_CPOL, setatPlusCPOL,    tesatPlusCPOL,  queatPlusCPOL,    0},
    {"+CPUC",       AT_CMD_CPUC, setatPlusCPUC,    test_gen,       queatPlusCPUC,    0},
    {"+CPWD",       AT_CMD_CPWD, setatPlusCPWD,    test_gen,       0,                "%s: (\"SC\",8),(\"AO\",4),(\"OI\",4),(\"OX\",4),(\"AI\",4),(\"IR\",4),(\"AB\",4),(\"AG\",4),(\"AC\",4),(\"P2\",8),(\"PS\",16),(\"PF\",16),(\"PN\",16),(\"PU\",16),(\"PP\",16),(\"PC\",16),(\"PL\",4),(\"LA\",4)"},
    {"+CR",         AT_CMD_CR,   setatPlusCR,      test_gen,       queatPlusCR,      "%s: (0,1)"},
    {"+CRC",        AT_CMD_CRC,  setatPlusCRC,     test_gen,       queatPlusCRC,     "%s: (0,1)"},
    {"+CREG",       AT_CMD_CREG, setatPlusCREG,    test_gen,       queatPlusCREG,    "%s: (0-2)"},
    {"+CRES",       AT_CMD_CRES, atPlusCRES,       tesatPlusCRES,  0,                0},
FD_CMD("+CRLP",     AT_CMD_CRLP, setatPlusCRLP,    test_gen,       queatPlusCRLP,    "%s: (0-61),(0-61),(39-255),(1-255)")
    {"+CRSL",       AT_CMD_CRSL, setatPlusCRSL,    tesatPlusCRSL,  queatPlusCRSL,    0},
    {"+CRSM",       AT_CMD_CRSM, setatPlusCRSM,    test_gen,       0,                0},
    {"+CSAS",       AT_CMD_CSAS, atPlusCSAS,       tesatPlusCSAS,  0,                0},
    {"+CSCA",       AT_CMD_CSCA, atPlusCSCA,       test_gen,       queatPlusCSCA,    0},
    {"+CSCB",       AT_CMD_CSCB, atPlusCSCB,       test_gen,       queatPlusCSCB,    "%s: (0,1)"},
    {"+CSCS",       AT_CMD_CSCS, setatPlusCSCS,    test_gen,       queatPlusCSCS,    "%s: \"GSM\",\"IRA\",\"PCCP437\",\"PCDN\",\"8859-1\",\"HEX\",\"UCS2\""},
    {"+CSDH",       AT_CMD_CSDH, atPlusCSDH,       test_gen,       queatPlusCSDH,    "%s: (0,1)"},
    {"+CSIM",       AT_CMD_CSIM, setatPlusCSIM,    test_gen,       0,                0},
    {"+CSMP",       AT_CMD_CSMP, atPlusCSMP,       test_gen,       queatPlusCSMP,    0},
    {"+CSMS",       AT_CMD_CSMS, atPlusCSMS,       test_gen,       queatPlusCSMS,    "%s: (0,1)"},
    {"+CSNS",       AT_CMD_CSNS, setatPlusCSNS,    test_gen,       queatPlusCSNS,    "%s: (0-7)"},
    {"+CSQ",        AT_CMD_CSQ,  setatPlusCSQ,     tesatPlusCSQ,   0,                0},
    {"%CSQ",        AT_CMD_P_CSQ,setatPercentCSQ,  tesatPercentCSQ,queatPercentCSQ,  "%s: (0,1)"},
    {"+CSSN",       AT_CMD_CSSN, setatPlusCSSN,    test_gen,       queatPlusCSSN,    "%s: (0,1),(0,1)"},
    {"+CSTA",       AT_CMD_CSTA, setatPlusCSTA,    test_gen,       queatPlusCSTA,    "%s: (129,145)"},
    {"+CSVM",       AT_CMD_CSVM, setatPlusCSVM,    test_gen,       queatPlusCSVM,    "%s: (0,1),(129,145,161)"},
    {"+CTFR",       AT_CMD_CTFR, setatPlusCTFR,    test_gen,       0,                0},
    {"+CUSD",       AT_CMD_CUSD, setatPlusCUSD,    test_gen,       queatPlusCUSD,    "%s: (0,1,2)"},
FD_CMD("+DR",       AT_CMD_DR,   setatPlusDR,      test_gen,       queatPlusDR,       "%s: (0,1)")
V42_CMD("+DS",      AT_CMD_DS,   setatPlusDS,      test_gen,       queatPlusDS ,      "%s: (0-3),(0,1),(512-65535),(6-250)")

#if defined (FF_FAX) AND defined (DTI)
FD_CMD("+FAP",      AT_CMD_FAP,  setatPlusFAP,     test_gen,       queatPlusFAP,      "%s:(0,1),(0,1),(0,1)")
FD_CMD("+FBO",      AT_CMD_FBO,  setatPlusFBO,     test_gen,       queatPlusFBO,      "%s:(0-3)")
FD_CMD("+FBS",      AT_CMD_FBS,  0,                0,              queatPlusFBS,      0)
FD_CMD("+FBU",      AT_CMD_FBU,  setatPlusFBU,     test_gen,       queatPlusFBU,      "%s:(0-1)")
FD_CMD("+FCC",      AT_CMD_FCC,  setatPlusFCC,     test_gen,       queatPlusFCC,      "%s:(0-64),(0-5),(0-2),(0-2),(0),(0),(0),(0-7),(0)")
FD_CMD("+FCLASS",   AT_CMD_FCLASS,setatPlusFCLASS,test_gen,       queatPlusFCLASS,   "0,2.0,8")
FD_CMD("+FCQ",      AT_CMD_FCQ,  setatPlusFCQ,     test_gen,       queatPlusFCQ,      "%s:(0-2),(0-2)")
FD_CMD("+FCR",      AT_CMD_FCR,  setatPlusFCR,     test_gen,       queatPlusFCR,      "%s:(0,1)")
FD_CMD("+FCS",      AT_CMD_FCS,  0,                0,              queatPlusFCS,      0)
FD_CMD("+FCT",      AT_CMD_FCT,  setatPlusFCT,     test_gen,       queatPlusFCT,      "%s:(1E)")
FD_CMD("+FDR",      AT_CMD_FDR,  setatPlusFDR,     0,              0,                 0)
FD_CMD("+FDT",      AT_CMD_FDT,  setatPlusFDT,     0,              0,                 0)
FD_CMD("+FEA",      AT_CMD_FEA,  setatPlusFEA,     test_gen,       queatPlusFEA,      "%s:(0)")
FD_CMD("+FFC",      AT_CMD_FFC,  setatPlusFFC,     test_gen,       queatPlusFFC,      "%s:(0),(0),(0),(0)")
FD_CMD("+FHS",      AT_CMD_FHS,  0,                0,              queatPlusFHS,      0)
FD_CMD("+FIE",      AT_CMD_FIE,  setatPlusFIE,     test_gen,       queatPlusFIE,      "%s:(0,1)")
FD_CMD("+FIP",      AT_CMD_FIP,  setatPlusFIP,     0,              0,                 0)
FD_CMD("+FIS",      AT_CMD_FIS,  setatPlusFIS,     test_gen,       queatPlusFIS,      "%s:(0-64),(0-5),(0-4),(0-2),(0),(0),(0),(0-7),(0)")
FD_CMD("+FIT",      AT_CMD_FIT,  setatPlusFIT,     test_gen,       queatPlusFIT,      "%s:(0-255),(0,1)")
FD_CMD("+FKS",      AT_CMD_FKS,  setatPlusFKS,     0,              0,                 0)
FD_CMD("+FLI",      AT_CMD_FLI,  setatPlusFLI,     test_gen,       queatPlusFLI,      "%s:\"(20-7E)\"")
FD_CMD("+FLO",      AT_CMD_FLO,  setatPlusFLO,     test_gen,       queatPlusFLO,      "%s: (0-2)")
FD_CMD("+FLP",      AT_CMD_FLP,  setatPlusFLP,     test_gen,       queatPlusFLP,      "%s:(0,1)")
FD_CMD("+FMI",      AT_CMD_FMI,  0,                0,              atPlusFMI,         0)
FD_CMD("+FMM",      AT_CMD_FMM,  0,                0,              atPlusFMM,         0)
FD_CMD("+FMR",      AT_CMD_FMR,  0,                0,              atPlusFMR,         0)
FD_CMD("+FMS",      AT_CMD_FMS,  setatPlusFMS,     test_gen,       queatPlusFMS,      "%s:(0-5)")
FD_CMD("+FND",      AT_CMD_FND,  setatPlusFND,     test_gen,       queatPlusFND,      "%s:(0,1)")
FD_CMD("+FNR",      AT_CMD_FNR,  setatPlusFNR,     test_gen,       queatPlusFNR,      "%s:(0,1),(0,1),(0,1),(0,1)")
FD_CMD("+FNS",      AT_CMD_FNS,  setatPlusFNS,     test_gen,       queatPlusFNS,      "%s:35")
FD_CMD("+FPA",      AT_CMD_FPA,  setatPlusFPA,     test_gen,       queatPlusFPA,      "%s:\"(20-7E)\"")
FD_CMD("+FPI",      AT_CMD_FPI,  setatPlusFPI,     test_gen,       queatPlusFPI,      "%s:\"(20-7E)\"")
FD_CMD("+FPS",      AT_CMD_FPS,  setatPlusFPS,     test_gen,       queatPlusFPS,      "%s:(1-5)")
FD_CMD("+FPW",      AT_CMD_FPW,  setatPlusFPW,     test_gen,       queatPlusFPW,      "%s:\"(20-7E)\"")
FD_CMD("+FRQ",      AT_CMD_FRQ,  setatPlusFRQ,     test_gen,       queatPlusFRQ,      "%s:(0),(0)")
FD_CMD("+FSA",      AT_CMD_FSA,  setatPlusFSA,     test_gen,       queatPlusFSA,      "%s:\"(20-7E)\"")
FD_CMD("+FSP",      AT_CMD_FSP,  setatPlusFSP,     test_gen,       queatPlusFSP,      "%s:(0,1)")
#endif /* FF_FAX */

    {"+GCAP",       AT_CMD_GCAP, atPlusGCAP,       0,              atPlusGCAP,        0},
    {"+GCI",        AT_CMD_GCI,  setatPlusGCI,     0,              queatPlusGCI,      0},
    {"+GMI",        AT_CMD_GMI,  atPlusCGMI,       test_gen,       0,                 0},
    {"+GMM",        AT_CMD_GMM,  atPlusCGMM,       test_gen,       0,                 0},
    {"+GMR",        AT_CMD_GMR,  atPlusCGMR,       test_gen,       0,                 0},
    {"+GSN",        AT_CMD_GSN,  atPlusCGSN,       test_gen,       0,                 0},
    {"+ICF",        AT_CMD_ICF,  setatPlusICF,     test_gen,       queatPlusICF,      "%s: (1-6),(0-3)"},
    {"+IFC",        AT_CMD_IFC,  setatPlusIFC,     test_gen,       queatPlusIFC,      "%s: (0-2),(0-2)"},
    {"+ILRR",       AT_CMD_ILRR, setatPlusILRR,    test_gen,       queatPlusILRR,     "%s: (0,1)"},
    {"+IPR",        AT_CMD_IPR,  setatPlusIPR,     test_gen,       queatPlusIPR,      "%s: (0,1200,2400,4800,9600,14400,19200,28800,38400,57600,115200),(75,150,300,600,1200,2400,4800,7200,9600,14400,19200,28800,33900,38400,57600,115200)"},
    {"+VTS",        AT_CMD_VST,  setatPlusVTS,     test_gen,       0,                 "%s: (0-9,#,*,A-D),(1-255)"},
#ifdef TI_PS_FF_AT_CMD_WS46
    {"+WS46",       AT_CMD_WS46, setatPlusWS46,    test_gen,       queatPlusWS46,     "%s: (12)"},
#endif /* TI_PS_FF_AT_CMD_WS46 */
    {"%ALS",        AT_CMD_ALS,  setatPercentALS,  tesatPercentALS,queatPercentALS,   0},
#ifdef TI_PS_FF_AT_P_CMD_ATR
    {"%ATR",        AT_CMD_ATR,  0,                test_gen,       queatPercentATR,   0},
#endif /* TI_PS_FF_AT_P_CMD_ATR */
LOC_CMD("+CLSA",    AT_CMD_CLSA, setatPlusCLSA,    test_gen,       queatPlusCLSA,     "%s: (0,1)")
LOC_CMD("+CLOM",    AT_CMD_CLOM, setatPlusCLOM,    test_gen,       queatPlusCLOM,     "%s: (0,1)")
LOC_CMD("+CLPS",    AT_CMD_CLPS, setatPlusCLPS,    test_gen,       queatPlusCLPS,     "%s: (0,1),(1-5),(1-120)")
LOC_CMD("+CLSR",    AT_CMD_CLSR, setatPlusCLSR,    test_gen,       0,                 "%s: (0,1),(0,1),(0,1)")
    {"%BAND",       AT_CMD_BAND, setatPercentBAND, tesatPercentBAND,queatPercentBAND, 0},
    {"%CACM",       AT_CMD_P_CACM,atPercentCACM,   test_gen,       0,                 0},
    {"%CAOC",       AT_CMD_P_CAOC,atPercentCAOC,   test_gen,       0,                 0},
    {"%CCBS",       AT_CMD_CCBS, setatPercentCCBS, test_gen,       queatPercentCCBS,  "%s: (0,1),(0-5)"},
#ifdef TI_PS_FF_AT_P_CMD_STDR
    {"%STDR",       AT_CMD_STDR, setatPercentSTDR, test_gen,       0,                 0},  
#endif /* TI_PS_FF_AT_P_CMD_STDR */
GPRS_CMD("%CGAATT", AT_CMD_CGAATT,setatPercentCGAATT,test_gen,     queatPercentCGAATT,"%s: (0,1),(0,1)") /* GPRS: GPRS attach/detach mode */
GPRS_CMD("%CGMM",   AT_CMD_P_CGMM,setatPercentCGMM, test_gen,      0,                 "%s: (0,1),(0,1)")
GPRS_CMD("%CGREG",  AT_CMD_P_CGREG,setatPercentCGREG,test_gen,     queatPercentCGREG, "%s: (0-3)")       /* extended GPRS network registration status  */
    {"%CNAP",       AT_CMD_CNAP, setatPercentCNAP, test_gen,       queatPercentCNAP,  "%s: (0,1)"},
    {"%CPI",        AT_CMD_CPI,  setatPercentCPI,  test_gen,       queatPercentCPI,   "%s: (0-4)"},
TTY_CMD("%CTTY",    AT_CMD_CTTY, setatPercentCTTY, test_gen,       queatPercentCTTY,  "%s: (0,1),(0-3)")
    {"%COLR",       AT_CMD_COLR, atPercentCOLR,    test_gen,       0,                 0},
    {"%CPRIM",      AT_CMD_CPRIM,atPercentCPRIM,   test_gen,       0,                 0},
    {"%CTV",        AT_CMD_CTV,  atPercentCTV,     test_gen,       0,                 0},
    {"%CUNS",       AT_CMD_CUNS, setatPercentCUNS, test_gen,       queatPercentCUNS,  "%s: (0-2)"},
    {"%NRG",        AT_CMD_NRG,  setatPercentNRG,  tesatPercentNRG,queatPercentNRG,   0},
#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_SAT_E)
    {"%PPP",        AT_CMD_PPP,  atPercentPPP,     test_gen,       queatPercentPPP,   "%s: (0-3)"},
#endif /* defined(WAP) || defined(FF_PPP) || defned(FF_SAT_E) */
SAT_CMD("%SATC",    AT_CMD_SATC, setatPercentSATC, tesatPercentSATC,queatPercentSATC, 0)
SAT_CMD("%SATE",    AT_CMD_SATE, atPercentSATE,    test_gen,       0,                 0)
SAT_CMD("%SATR",    AT_CMD_SATR, atPercentSATR,    test_gen,       0,                 0)
SAT_CMD("%SATT",    AT_CMD_SATT, atPercentSATT,    test_gen,       0,                 0)
MTST_CMD("%MTST",   AT_CMD_MTST, setatPercentMTST, test_gen,       0,                 "%s: (0,1)")
GPRS_CMD("%SNCNT",  AT_CMD_SNCNT,setatPercentSNCNT,test_gen,       queatPercentSNCNT, "%s: (0)")
    {"%VER",        AT_CMD_VER,  atPercentVER,     test_gen,       0,                 0},
GPRS_CMD("%CGCLASS",AT_CMD_P_CGCLASS,setatPercentCGCLASS,test_gen, queatPercentCGCLASS, "%s: (\"B\",\"BG\",\"BC\",\"BX\",\"CG\",\"CC\")")
GPRS_CMD("%CGPCO",  AT_CMD_CGPCO,setatPercentCGPCO,test_gen,       0,                 "%s: (0,1),(0,1),(1-2)")
GPRS_CMD("%CGPPP",  AT_CMD_CGPPP,setatPercentCGPPP,test_gen,       queatPercentCGPPP, "%s: (0-3)")
EM_CMD ("%EM",      AT_CMD_EM,   setatPercentEM,   test_gen,       0,                 "%s: (2-3),(1-13)")
EM_CMD ("%EMET",    AT_CMD_EMET, setatPercentEMET, test_gen,       0,                 "%s: (1-8)")
EM_CMD ("%EMETS",   AT_CMD_EMETS,setatPercentEMETS,test_gen,       0,                 "%s: (1-8),(FFFFFFFF),(FFFFFFFF)")
WAP_CMD("%WAP",     AT_CMD_WAP,  atPercentWAP,     test_gen,       0,                 0)
HZ_CMD ("%CBHZ",    AT_CMD_CBHZ, setatPercentCBHZ, tesatPercentCBHZ,queatPercentCBHZ, 0)
CPHS_CMD("%CPHS",   AT_CMD_CPHS, setatPercentCPHS, test_gen,       queatPercentCPHS,  "%s: (0-2)")
CPHS_CMD("%CPNUMS", AT_CMD_CPNUMS,setatPercentCPNUMS,tesatPercentCPNUMS, 0,           0)
CPHS_CMD("%CPALS",  AT_CMD_CPALS,setatPercentCPALS,tesatPercentCPALS,queatPercentCPALS, 0)
CPHS_CMD("%CPVWI",  AT_CMD_CPVWI,setatPercentCPVWI,test_gen,       0,                 "%s: (0-2),(1,256)")
CPHS_CMD("%CPOPN",  AT_CMD_CPOPN,0,                test_gen,       queatPercentCPOPN, 0)
CPHS_CMD("%CPCFU",  AT_CMD_CPCFU,setatPercentCPCFU,test_gen,       0,                 "%s: (0-2),(1,256)")
CPHS_CMD("%CPINF",  AT_CMD_CPINF,setatPercentCPINF,test_gen,       queatPercentCPINF, 0)
CPHS_CMD("%CPMB",   AT_CMD_CPMB, setatPercentCPMB, test_gen,       queatPercentCPMB,  "%s: 4")
    {"%CPRI",       AT_CMD_CPRI, setatPercentCPRI, test_gen,       queatPercentCPRI,  "%s: (0,1)"},
#ifdef DTI
    {"%DATA",       AT_CMD_DATA, setatPercentDATA, test_gen,       queatPercentDATA,  "%s: (0,1,2)"},
    {"%DINF",       AT_CMD_DINF, setatPercentDINF, test_gen,       0,                 "%s: (0,1)"},
#endif
    {"%CLCC",       AT_CMD_P_CLCC,setatPercentCLCC,test_gen,       0,                 0},
#ifdef TI_PS_FF_AT_P_CMD_DBGINFO
    {"%DBGINFO",    AT_CMD_P_DBGINFO,setatPercentDBGINFO,            test_gen,       0, 0},
#endif /* TI_PS_FF_AT_P_CMD_DBGINFO */
    {"%VTS",        AT_CMD_P_VST,  setatPercentVTS,test_gen,       0,                 "%s: (0,1,2,3,4,5,6,7,8,9,A,B,C,D,#,*),(0-2)"},
    {"%CHPL",       AT_CMD_CHPL, setatPercentCHPL, test_gen,       0,                 "%s: (0,1,2)"},
    {"%CREG",       AT_CMD_P_CREG,setatPercentCREG,test_gen,       queatPercentCREG,  "%s: (0-2)"},
    {"+CTZR",       AT_CMD_CTZR, setatPlusCTZR,    test_gen,       queatPlusCTZR,     "%s: (0,1)"},
    {"+CTZU",       AT_CMD_CTZU, setatPlusCTZU,    test_gen,       queatPlusCTZU,     "%s: (0,1)"},
    {"%CTZV",       AT_CMD_P_CTZV, setatPercentCTZV, test_gen,     queatPercentCTZV,  "%s: (0,1)"},
    {"%CNIV",       AT_CMD_P_CNIV, setatPercentCNIV, test_gen,     queatPercentCNIV,  "%s: (0,1)"},
    {"%PVRF",       AT_CMD_PVRF, setatPercentPVRF, test_gen,       queatPercentPVRF,  "%s: (0-2)"},
    {"%CWUP",       AT_CMD_CWUP, setatPercentCWUP, test_gen,       0,                 0},
    {"%DAR",        AT_CMD_DAR,  setatPercentDAR,  test_gen,       0,                 0},
    {"+CIND",       AT_CMD_CIND, setatPlusCIND,    test_gen,       queatPlusCIND,     "%s: (\"signal\", (0-5)), (\"smsfull\", (0-1))"},
    {"+CMER",       AT_CMD_CMER, setatPlusCMER,    test_gen,       queatPlusCMER,     "%s: (0-2), (0), (0), (0-2), (0,1)"},
#ifdef TI_PS_FF_AT_P_CMD_CSCN
    {"%CSCN",       AT_CMD_CSCN, setatPercentCSCN, test_gen,       queatPercentCSCN,  "%s: (0,1),(0,1,2),(0,1),(0,1,2)"},
#endif /* TI_PS_FF_AT_P_CMD_CSCN */
#ifdef TI_PS_FF_AT_P_CMD_MMITEST
MFW_CMD("%MMITEST", AT_CMD_NONE, setatPercentMMITEST, test_gen,    0,                 0)
#endif
    {"%RDL",        AT_CMD_RDL,  setatPercentRDL,  test_gen,       queatPercentRDL,   "%s: (0,1),(0,1)"},
#ifdef TI_PS_FF_AT_P_CMD_RDLB
    {"%RDLB",       AT_CMD_RDLB, setatPercentRDLB, test_gen,       queatPercentRDLB,  "%s: (10),(0,1)"},
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
    {"%CSTAT",      AT_CMD_CSTAT,setatPercentCSTAT,test_gen,       queatPercentCSTAT, "%s: (0,1)"},
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
    {"%CPRSM",      AT_CMD_CPRSM,setatPercentCPRSM,test_gen,       queatPercentCPRSM, "%s: (0,1)"},
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
    {"%CHLD",       AT_CMD_P_CHLD, setatPercentCHLD, test_gen,     0,                 "%s: (0,1,1x,2,2x,3,4,6,6x,7x,h|H,i|I)"},
    {"%SIMIND",     AT_CMD_P_SIMIND, setatPercentSIMIND, test_gen, queatPercentSIMIND,"%s: (0,1)"},
#ifdef TI_PS_FF_AT_P_CMD_SECP
    {"%SECP",        AT_CMD_P_SECP, setatPercentSECP, test_gen, 0,   0},
#endif /* TI_PS_FF_AT_P_CMD_SECP */
#ifdef TI_PS_FF_AT_P_CMD_SECS
    {"%SECS",       AT_CMD_P_SECS, setatPercentSECS,  test_gen,  quetatPercentSECS,   "%s: (0,1)"},
#endif /* TI_PS_FF_AT_P_CMD_SECS */
    {"%CSSN",  AT_CMD_P_CSSN,setatPercentCSSN,test_gen,queatPercentCSSN,"%s: (0,1,2),(0,1)"},
    {"+CCLK",       AT_CMD_CCLK,     setatPlusCCLK,        test_gen,     queatPlusCCLK,       0},
    {"%CSSD",       AT_CMD_CSSD, atPercentCSSD,    test_gen,       0,                 0},
    {"%COPS",       AT_CMD_P_COPS, setatPercentCOPS,    tesatPercentCOPS,  queatPercentCOPS,    0},

CPHS_CMD("%CPMBW",  AT_CMD_CPMBW, setatPercentCPMBW, tesatPercentCPMBW,0,             0)
#ifdef TI_PS_FF_AT_P_CMD_CUST
    {"%CUST",      AT_CMD_CUST, setatPercentCUST, test_gen,       queatPercentCUST,  "%s: (0,1)"},
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    {"%SATCC",    AT_CMD_SATCC,setatPercentSATCC, test_gen,       queatPercentSATCC, "%s: (0,1)"},
    {"%COPN",       AT_CMD_P_COPN, setatPercentCOPN,    test_gen,       0,                0},
GPRS_CMD("%CGEREP", AT_CMD_P_CGEREP, setatPercentCGEREP,  test_gen,       queatPercentCGEREP,  "%s: (0,1)")      /* GPRS event reporting */
#ifdef FF_DUAL_SIM
    {"%SIM",       AT_CMD_SIM,  setatPercentSIM,  test_gen,       queatPercentSIM,   "%s: (0-2)"},
#endif /*FF_DUAL_SIM*/
#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
#ifdef _SIMULATION_
    {"%CUSCFG",    AT_CMD_CUSCFG,   setatPercentCUSCFG,  test_gen,          0,        "%s: (\"CCMOSM\", (0-2)),(\"CCMOC\", (0-2)),(\"CCMOSS\",(0-2)), (\"CCMOUSSD\", (0-2)) ,(\"2BDIAL\",(0-2)), (\"EUSSDR\", (0-2)), (\"ETMOBEONSR\", (0-2)), (\"USSDASMOC\", (0-2))"},
#else
    {"%CUSCFG",    AT_CMD_CUSCFG,   setatPercentCUSCFG,  0,                 0,        0},
#endif
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */
#ifdef TI_PS_FF_AT_P_CMD_CUSDR
    {"%CUSDR",     AT_CMD_CUSDR,   setatPercentCUSDR,    test_gen,          0,         "%s: (0-2)"},
#endif /* TI_PS_FF_AT_P_CMD_CUSDR */
    {"%CPBS",      AT_CMD_P_CPBS, 0 ,test_gen, queatPercentCPBS, 0},
    {"%PBCF",       AT_CMD_P_PBCF, setatPercentPBCF,    test_gen,  queatPercentPBCF, "%s: (0-1), (0-1), (0-1)"},
 SAT_CMD("%SIMEF",      AT_CMD_SIMEF,  setatPercentSIMEF,  test_gen, queatPercentSIMEF,  "%s: (0-1)")
 SAT_CMD("%EFRSLT",     AT_CMD_EFRSLT, setatPercentEFRSLT, test_gen, 0,                  "%s: (0-1)")
    {"%CMGMDU",    AT_CMD_P_CMGMDU,   setatPercentCMGMDU, 0,        0,                0},
    {"%CMGL",          AT_CMD_P_CMGL, atPercentCMGL,      tesatPercentCMGL, NULL,      0},
    {"%CMGR",          AT_CMD_P_CMGR, atPercentCMGR,      NULL,             NULL,      0},
#ifdef FF_CPHS_REL4
    {"%CFIS",    AT_CMD_P_CFIS,   setatPercentCFIS,  test_gen,          0,        "%s:0-2 [,1-4]"},
    {"%MWIS",    AT_CMD_P_MWIS,   setatPercentMWIS,  test_gen,          0,        "%s:0-2"},
    {"%MWI",     AT_CMD_P_MWI,    setatPercentMWI,   test_gen,       queatPercentMWI,   "%s: 0-1"},
    {"%MBI",     AT_CMD_P_MBI,    setatPercentMBI,   test_gen,         NULL,     "%s: (0-4)"},
    {"%MBDN",    AT_CMD_P_MBDN,   setatPercentMBDN,  test_gen,         NULL,     "%s: (0-16)"},
#endif /* FF_CPHS_REL4 */
#ifdef TI_PS_FF_AT_CMD_P_ECC
    {"%ECC",           AT_CMD_P_ECC,  setatPercentECC,    0,                0,         0},
#endif /* TI_PS_FF_AT_CMD_P_ECC */
#ifdef TI_PS_FF_AT_P_CMD_CTREG
    {"%CTREG",     AT_CMD_CTREG, setatPercentCTREG, test_gen, 0,                  "%s: (1-2), (1-2)"},
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
#ifdef REL99
    {"%CMGRS",      AT_CMD_P_CMGRS, setatPercentCMGRS,  test_gen,    queatPercentCMGRS,  "%s: (0-2)"},
#endif /* REL99 */
    {"+CVHU",       AT_CMD_CVHU, setatPlusCVHU, test_gen, queatPlusCVHU, "%s: (0-2)"},
    {NULL,AT_CMD_NONE,NULL,NULL,NULL,NULL}
};

static const char large_output_cmd[5][6] =
{
  {"+COPN"},
  {"+CPBF"},
  {"+CPBR"},
  {"%EM"},
  {""}
};


/*==== TYPES ======================================================*/
GLOBAL CHAR   *cmdErrStr       = NULL;   /* Error Message  */

GLOBAL T_ACI_AT_CMD curAbrtCmd = AT_CMD_NONE;
GLOBAL T_ACI_LIST *ati_src_list = NULL;

/* if a customer runs its own AT cmd by extension mechanism,
 * this variable must be reset to 0xFF when completed or aborted */
GLOBAL UBYTE src_id_ext = 0xFF;

/* global buffer for ioSendxxx */
GLOBAL char g_sa[MAX_CMD_LEN];

LOCAL  CHAR** atCmdFwrdLst  = NULL;/*command list pointer for the old extension mechanism*/
LOCAL  CHAR** atCmdFwrdLst_v2 = NULL;/*command list pointer for the new extension mechanism*/
LOCAL  BOOL ext_v2_flag = FALSE;/*the flag of which extension mechanism to be used*/
#ifdef SIM_TOOLKIT
EXTERN void aci_sat_cmd_init (void);
#endif
EXTERN void cmd_clearCnmiBuf (void);

#ifdef FF_ATI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_UTIL                     |
| STATE   : code             ROUTINE : trace_run_cmd_line           |
+-------------------------------------------------------------------+

  PURPOSE : This function trace the command line
*/

void trace_run_cmd_line (char *prefix, UBYTE src_id, char *cmd_name, char *cmd_params)
{
  char trcBuf[80], nameBuf[40];
  int dst_i;

  /* Truncate cmd_name to 40 chars to avoid buffer overflow */
  strncpy(nameBuf, cmd_name, 39);
  nameBuf[39]= '\0';
  if (nameBuf[36]) /* and add trail "..." if needed */
  {
    nameBuf[38] = nameBuf[37] = nameBuf[36] = '.';
  }

  dst_i = sprintf (trcBuf, "%s(Src %d) cmd:%s param:", (prefix) ? prefix : "", src_id, nameBuf);

  if (dst_i < 79)
  {
    strncpy(&trcBuf[dst_i], (cmd_params) ? cmd_params : "no param!", 79-dst_i);
  }
  trcBuf[79] = '\0';

  if (trcBuf[76])
  {
    trcBuf[76] = trcBuf[77] = trcBuf[78] = '.';  /* add trailing "..." if string is >=76 */
  }

  TRACE_EVENT_P1("%s",trcBuf);
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ATI                |
| STATE   : code                        ROUTINE : map_aci_2_ati_rslt |
+--------------------------------------------------------------------+

  PURPOSE : map the result of ACI to ATI

*/
T_ATI_RSLT map_aci_2_ati_rslt (T_ACI_RETURN rslt)
{
  switch (rslt)
  {
    default:
    case AT_FAIL:
    {
      if (cmdErrStr EQ NULL)
      {
        cmdCmeError(CME_ERR_Unknown);
      }
      return (ATI_FAIL);
    }
    case AT_BUSY:
      return ATI_BUSY;
    case AT_EXCT:
      return (ATI_EXCT);
    case AT_CMPL:
      return (ATI_CMPL);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_LST            |
| STATE   : code                        ROUTINE : cmhUARTtest_srcId  |
+--------------------------------------------------------------------+

  PURPOSE : search function for source Id in the structure T_ATI_SRC_PARAMS.

*/

GLOBAL BOOL search_ati_src_id (UBYTE src_id, void *elem)
{
  T_ATI_SRC_PARAMS *compared = (T_ATI_SRC_PARAMS *)elem;

  if (compared NEQ NULL)
    if (compared->src_id EQ src_id )
      return TRUE;
  return FALSE;
}

GLOBAL void ati_creg_init( UBYTE srcId, T_ACI_CREG_CMD cmd )
{
  T_ATI_REG_MOD_LAC_CID *mod_lac_cid;

  /* set +CREG / %CREG / +CGREG / %CGEREG related parameters */

  switch( cmd )
  {
    case(CREG_CMD):
      ati_user_output_cfg[srcId].creg.last_presented_state = CREG_STAT_NotPresent;
      mod_lac_cid = &ati_user_output_cfg[srcId].creg.mod_lac_cid;
      break;

  case(PercentCREG_CMD):
    ati_user_output_cfg[srcId].percent_creg.last_presented_state = CREG_STAT_NotPresent;
    mod_lac_cid = &ati_user_output_cfg[srcId].percent_creg.mod_lac_cid;
    break;


#ifdef GPRS
    case(PlusCGREG_CMD):
      ati_gprs_user_output_cfg[srcId].plus_cgreg.last_presented_state = CGREG_STAT_NOT_PRESENT;
      mod_lac_cid = &ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid;
      break;

    case(PercentCGREG_CMD):
      ati_gprs_user_output_cfg[srcId].percent_cgreg.last_presented_state = P_CGREG_STAT_NOT_PRESENT;
      mod_lac_cid = &ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid;
      break;
#endif /* GPRS */

    default:
      TRACE_ERROR ("ati_creg_init() called with invalid cmd!");
      return; /* avoid setting uninitialized mod_lac_cid */

  }

  mod_lac_cid->pres_mode            = CREG_MOD_OFF;
  mod_lac_cid->last_presented_lac   = NOT_PRESENT_16BIT;
  mod_lac_cid->last_presented_cid   = NOT_PRESENT_16BIT;
}

/*
+--------------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD                        |
| STATE   : code                        ROUTINE : ati_cmd_init_each_source       |
+--------------------------------------------------------------------------------+

  PURPOSE : initialize ATI parameters for 1 source.

*/

GLOBAL void ati_cmd_init_each_source( UBYTE srcId )
{
  /* Ensure Id set for CME CMS CEER... */
  srcId_cb = srcId ;

  ati_user_output_cfg[srcId].atE=1; /* Command Echo ON */
  ati_user_output_cfg[srcId].atQ=0; /* Result Code Suppression not suppressed */
  ati_user_output_cfg[srcId].atX=0; /* Result Code Selection and Call Progress Monitoring Control:
                                       CONNECT result code is given upon entering online data state.
                                       Dial tone and busy detection are disabled.
                                     */

  ati_user_output_cfg[srcId].CR_stat   = 0;
  ati_user_output_cfg[srcId].CRC_stat  = 0;
  ati_user_output_cfg[srcId].CLIP_stat = 0;
  ati_user_output_cfg[srcId].CDIP_stat = 0;  
  ati_user_output_cfg[srcId].DR_stat   = 0;
  ati_user_output_cfg[srcId].ILRR_stat = 0;
  ati_user_output_cfg[srcId].CSDH_stat = 1;
  ati_user_output_cfg[srcId].CSSI_stat = 0;
  ati_user_output_cfg[srcId].CSSU_stat = 0;
  ati_user_output_cfg[srcId].CUSD_stat = 0;
  ati_user_output_cfg[srcId].CPI_stat  = 0;
  ati_user_output_cfg[srcId].CCWE_stat = 0;
  ati_user_output_cfg[srcId].CAOC_stat = 0;
  ati_user_output_cfg[srcId].CMEE_stat = CMEE_MOD_Disable;
  ati_user_output_cfg[srcId].SIMIND_stat = 0;
  ati_user_output_cfg[srcId].cscsChset = CSCS_CHSET_Ira;

  ati_user_output_cfg[srcId].CCWV_charging = CCWV_CHRG_Termination;
  ati_user_output_cfg[srcId].cnap_mode = CNAP_DISABLED;

  ati_user_output_cfg[srcId].CSTAT_stat = 0;

  ati_user_output_cfg[srcId].CPRI_stat = 0;

#ifdef FF_CPHS_REL4
  ati_user_output_cfg[srcId].MWI_stat  = 0;
#endif

  /* init CREG param */
  ati_creg_init(srcId, CREG_CMD);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : ati_cmd_init       |
+--------------------------------------------------------------------+

  PURPOSE : sets S-Registers to default values ( should be called once at program start)

*/

GLOBAL void ati_cmd_init(void)
{
  UBYTE i;

  TRACE_FUNCTION ("ati_cmd_init()");

  at.S[0]=0;      /* no auto answer when ring*/
  at.S[1]=0;      /* holds the number of ring counts */
  at.S[3]=13;
  at.S[4]=10;
  at.S[5]=8;
  at.S[6]=2;
  at.S[7]=60;
  at.S[8]=2;
  at.S[10]=1;
  at.S30=1;
#ifdef GPRS
  at.S99=1;       /* automatic reject network initiated context prior the n-th ring */
#endif /* GPRS */
  at.s1415.atM=0; /* Monitor Speaker Mode OFF */
  at.s1415.atL=0; /* Monitor Speaker Loudness LOW */
  at.s1415.atV=1; /* DCE Response Format:
                     DCE transmits full headers and trailers
                     and verbose response text.
                   */
  SMS_TEXT_MODE = SMS_TEXT_MODE_NONE;

  at.flags.COLP_stat = 0;
  at.flags.CCWA_stat = 0;
  at.flags.CCBS_stat = 0;
  at.rngPrms.srcID_S0     = NOT_PRESENT_8BIT;
#ifdef GPRS
  at.rngPrms.srcID_CGAUTO = (char)NOT_PRESENT_8BIT;
#endif
  at.rngPrms.rngCnt  = 1;
  at.rngPrms.isRng   = FALSE;
  at.rngPrms.mode    = CRING_MOD_NotPresent;
  at.rngPrms.type1   = CRING_SERV_TYP_NotPresent;
  at.rngPrms.type2   = CRING_SERV_TYP_NotPresent;
  at.CNMI_mode = CNMI_MOD_Buffer;
  at.CNMI_bfr  = CNMI_BFR_Flush;

  at.clipPrms.stat = CLIP_STAT_NotPresent;
  memset( at.clipPrms.number, 0, sizeof(at.clipPrms.number));
  memset(&at.clipPrms.type, 0, sizeof(T_ACI_TOA));
  at.clipPrms.validity = MNCC_PRES_NOT_PRES;
  memset( at.clipPrms.subaddr, 0, sizeof(at.clipPrms.subaddr));
  memset(&at.clipPrms.satype, 0, sizeof(T_ACI_TOS));
#ifdef NO_ASCIIZ
  memset(&at.clipPrms.alpha, 0, sizeof(T_ACI_PB_TEXT));
#else
  memset(at.clipPrms.alpha, 0, sizeof(at.clipPrms.alpha));
#endif

#ifdef SIM_TOOLKIT
  sat_new_source();
  aci_sat_cmd_init();
#endif

#if CONFIG_AT_RVTMUX
  ati_src_rvt_register();
#endif

  cmd_clearCnmiBuf ();

  /* for ATI parameters that are source dependant */
  for(i=0;i<CMD_SRC_MAX;i++)
  {
    ati_cmd_init_each_source( i );
#ifdef GPRS
    gaci_ati_cmd_init_each_source( i );
#endif /* GPRS */
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : ati_cmd_reset      |
+--------------------------------------------------------------------+

  PURPOSE : sets ATI command Parameter to default values

*/

GLOBAL void ati_cmd_reset(UBYTE srcId)
{
  TRACE_FUNCTION ("ati_cmd_reset()");

  at.S[0]=0;      /* no auto answer when ring*/
  at.S[1]=0;      /* holds the number of ring counts */
  at.S[3]=13;
  at.S[4]=10;
  at.S[5]=8;
  at.S[6]=2;
  at.S[7]=60;
  at.S[8]=2;
  at.S[10]=1;
  at.S30=1;
#ifdef GPRS
  at.S99=1;       /* automatic reject network initiated context prior the n-th ring */
#endif /* GPRS */
  at.s1415.atM=0; /* Monitor Speaker Mode OFF */
  at.s1415.atL=0; /* Monitor Speaker Loudness LOW */
  at.s1415.atV=1; /* DCE Response Format:
                     DCE transmits full headers and trailers
                     and verbose response text.
                   */
  at.flags.COLP_stat = 0;
  at.flags.CCWA_stat = 0;
  at.flags.CCBS_stat = 0;

  ati_cmd_init_each_source(srcId);
#ifdef GPRS
  gaci_ati_cmd_init_each_source(srcId);
#endif /* GPRS */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : compact            |
+--------------------------------------------------------------------+

  PURPOSE : eliminates whitspaces in commandline

*/

GLOBAL USHORT compact (char *s, USHORT len)
{
  char   *pWrite   = s;
  char   *pRead    = s;
  BOOL    isString = FALSE;

  while ( pRead - s < len )
  {
    if (*pRead EQ '"')
      isString = !isString;

    if (isString OR *pRead > ' ')
      *pWrite++ = *pRead;

    pRead++;
  }

  *pWrite = '\0';

  return ( pWrite - s );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : rise               |
+--------------------------------------------------------------------+

  PURPOSE : changes commands only to upper

*/
GLOBAL void rise(char *s, USHORT len)
{
  char* begin = s;

  while (s - begin < len)
  {
    if (*s EQ '"')
    {
      do
      {
        s++;
      } while(s - begin < len AND *s !='"');
    }
    else
      *s=toupper(*s);
    s++;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : check_str          |
+--------------------------------------------------------------------+

  PURPOSE : checks given string if characters appear that shouldn't be in the string

*/

GLOBAL BOOL check_str(char * string,char * sample)
{
  for ( ; *string; string++)
  {
    if (!strchr(sample, *string))
      return FALSE;
  }
  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : toa_demerge        |
+--------------------------------------------------------------------+

  PURPOSE : splits TOA octet into TOA structure

*/

GLOBAL T_ACI_TOA toa_demerge (SHORT type)
{
  T_ACI_TOA toa;

  toa.ton = (T_ACI_TOA_TON)((type & 0x70)>>4);
  switch (toa.ton)                   /*valid values 0-4*/
  {
/*  case TON_NotPresent   :  =-1,  */
    case TON_Unknown      : /* = 0,  */
    case TON_International:
    case TON_National     :
    case TON_NetSpecific  :
    case TON_DedAccess    :
/*  case TON_Alphanumeric :
    case TON_Abbreviated  :
    case TON_Extended     :
    ffs 
*/
      break;

    default:
      toa.ton = TON_NotPresent;
      break;
  }

  toa.npi = (T_ACI_TOA_NPI)(type & 0xF);
  switch (toa.npi)                  /*valid values 0,1,3,4,8,9*/
  {
/*  case NPI_NotPresent   :  =-1, */
    case NPI_Unknown      : /* = 0, */
    case NPI_IsdnTelephony: /* = 1, */
    case NPI_Data         : /* = 3, */
    case NPI_Telex        : /* = 4, */
    case NPI_National     : /* = 8, */
    case NPI_Private      : /* = 9, */
/*  case NPI_ERMES        :  = 10,   ffs */
/*  case NPI_CTS          :  = 11    ffs */
      break;

    default:
      toa.npi = NPI_NotPresent;
      break;
  }

  return toa;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : toa_sms_demerge    |
+--------------------------------------------------------------------+

  PURPOSE : splits TOA octet into TOA structure, according to 23.040

*/
GLOBAL T_ACI_TOA toa_sms_demerge (SHORT type)
{
  T_ACI_TOA toa;

  toa = toa_demerge(type);
  switch ((type & 0x70)>>4)
  {
    case TON_Alphanumeric :      /* additionally handle Alphanumeric */
      toa.ton = TON_Alphanumeric;
      break;
    default:
      break;
  }

  return toa;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : tos_merge          |
+--------------------------------------------------------------------+

  PURPOSE : builds type of sub-address octet from TOS structure

*/
GLOBAL SHORT tos_merge (T_ACI_TOS satype)
{
  return   ((satype.tos << 4) & 0x70)
         | ((satype.oe  << 3) & 0x08)
         |                      0x80;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : tos_demerge        |
+--------------------------------------------------------------------+

  PURPOSE : splits TOS structure into TOS structure

*/
GLOBAL T_ACI_TOS tos_demerge (SHORT tsatype)
{
  T_ACI_TOS satype;

  satype.tos = (T_ACI_TOS_TOS)((tsatype & 0x70) >> 4);
  switch(satype.tos)                  /*valid values 0,2*/
  {
/*  case TOS_NotPresent:  =-1, */
    case TOS_Nsap      :/*  = 0, */
    case TOS_User      :/*  = 2 */
      break;

    default:
      satype.tos = TOS_NotPresent;
      break;
  }

  satype.oe  = (T_ACI_TOS_OE)((tsatype & 0x08) >> 3);
  switch(satype.oe)                 /*valid Values 0,1*/
  {
/*  case OE_NotPresent:  =-1, */
    case OE_Even      :/*   = 0, */
    case OE_Odd       :/*   = 1  */
      break;

    default:
      satype.oe = OE_NotPresent;
      break;
  }

  return satype;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : ci_remTrailCom     |
+--------------------------------------------------------------------+

  PURPOSE : remove the trailing commas of an output line.

*/
GLOBAL void ci_remTrailCom (CHAR* outLine, USHORT len)
{
  while (len)
  {
    if (outLine[--len] EQ ',')
      outLine[len] = '\0';
    else
      break;
  }
}


LOCAL void send_output_type_signal (T_ATI_SRC_PARAMS *src_params,
                                    UBYTE output_type)
{
  if (src_params->line_state_cb NEQ NULL)
  {
    src_params->line_state_cb (src_params->src_id,
                               ATI_LINE_STATE_OUTPUT_TYPE,
                               (ULONG)output_type);
  }
  else
  {
    TRACE_EVENT_P1 ("[WRN] send_output_type_signal: no callback for srcId=%d",
                    src_params->src_id);
  }
}


LOCAL void check_and_send_output_type_signal (T_ATI_SRC_PARAMS *src_params, CHAR *cmd)
{
  int i = 0;

  while (*large_output_cmd[i] NEQ '\0')
  {
    if (strcmp (large_output_cmd[i], cmd) EQ 0)
    {
      TRACE_EVENT("Command necessiting large output detected");

      send_output_type_signal (src_params, ATI_OUTPUT_TYPE_LARGE);
      return;
    }

    i++;
  }

  send_output_type_signal (src_params, ATI_OUTPUT_TYPE_NORMAL);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aciAbort           |
+--------------------------------------------------------------------+

  PURPOSE : abort currently executing command

*/

GLOBAL void aciAbort (UBYTE srcId)
{
  T_ACI_RETURN rslt;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION ("aciAbort()");

  if ( src_params->curAtCmd EQ AT_CMD_NONE OR
       src_params->curAtCmd EQ AT_CMD_ABRT    )
  {
    return;
  }

  if ( src_params->curAtCmd EQ AT_CMD_EXT )
  {
    g_sa[0] = ( CHAR ) ACI_EXT_STRG_END;
    if (ext_v2_flag EQ TRUE)
    {
      rslt = rEXT_Abort(srcId);
    }
    else
    {
      rslt = rAT_ACP ( g_sa, MAX_CMD_LEN - 1 );
      aci_sendPString ( srcId, g_sa );
    }
    switch ( rslt )
    {
      case ( AT_EXCT ):
        curAbrtCmd = AT_CMD_ABRT;
        break;

      case ( AT_CMPL ):
        src_params->curAtCmd    = AT_CMD_NONE;
        ledit_ctrl (srcId, LEDIT_CTRL_CMPL, NULL);
        io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
        break;

      default:
        {
          io_sendMessage ( srcId, cmdCmeError ( CME_ERR_FailedToAbort ),
                                 (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_RESULT_CODE_OUTPUT));
        }
        break;
    }
  }
  else
  {
    switch ( sAT_Abort ((T_ACI_CMD_SRC)srcId, src_params->curAtCmd) )
    {
      case ( AT_EXCT ):
        curAbrtCmd = AT_CMD_ABRT;
        break;

      case ( AT_CMPL ):
        src_params->curAtCmd    = AT_CMD_NONE;
        ledit_ctrl (srcId, LEDIT_CTRL_CMPL, NULL);
        io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
        cmdErrStr   = NULL;
        break;

      default:
        switch ( src_params->curAtCmd )
        {
          case ( AT_CMD_CMGC ):
          case ( AT_CMD_CNMI ):
          case ( AT_CMD_CSMS ):
          case ( AT_CMD_CMGD ):
          case ( AT_CMD_CMGR ):
          case ( AT_CMD_CMGW ):
          case ( AT_CMD_CMGS ):
          case ( AT_CMD_CMSS ):
          case ( AT_CMD_CPMS ):
          case ( AT_CMD_CSMP ):
          case ( AT_CMD_CSCA ):
          case ( AT_CMD_CRES ):
          case ( AT_CMD_CSAS ):
          case ( AT_CMD_CMGL ):
            io_sendMessage ( srcId, cmdCmsError ( CMS_ERR_FailedToAbort ),
                        (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_RESULT_CODE_OUTPUT));
            break;

          default:
            io_sendMessage ( srcId, cmdCmeError ( CME_ERR_FailedToAbort ),
                                    (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT | ATI_RESULT_CODE_OUTPUT));
        }
        break;
    }
  }
  cmdErrStr = NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE :ati_execute_sat_cmd |
+--------------------------------------------------------------------+

  PURPOSE :

*/
#ifdef SIM_TOOLKIT
GLOBAL BOOL ati_execute_sat_cmd (T_ATI_SRC_PARAMS *src_params,
                                 UBYTE *chars, USHORT len)
{
  BOOL  end_cmd_found = FALSE;
  UBYTE i;
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION ("ati_execute_sat_cmd ()");

  for (i=0; i < len; i++)
  {
    if( chars[i] EQ '\r' ) /* for SAT hard coded S3=CR */
    {
      end_cmd_found = TRUE;
      break;
    }
  }

  if( !end_cmd_found )
  {
    TRACE_EVENT("ERROR: run at command not terminated by at.S[3]");
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return(FALSE);
  }

  ati_execute (src_params->src_id, chars, len);
  return(TRUE);
}
#endif /* SIM_TOOLKIT */


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_execute_config_cmd  |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL BOOL ati_execute_config_cmd (UBYTE *chars, USHORT len)
{
  BOOL  S3_CR_found = FALSE;
  UBYTE i;
  T_ATI_SRC_PARAMS *src_params = NULL;
  static UBYTE     cnf_src_id = 0;
  T_LEDIT_ATCMD   *cmd = NULL;

  TRACE_FUNCTION ("ati_execute_config_cmd()");

  if (!cnf_src_id)
  {
    cnf_src_id = ati_init (ATI_SRC_TYPE_TST, NULL, NULL);
    if (!cnf_src_id)
    {
      TRACE_EVENT ("[ERR] ati_execute_config_cmd: cannot create source");
      return (FALSE);
    }
    TRACE_EVENT_P1 ("ati_execute_config_cmd: srcId=%d", cnf_src_id);
  }
  src_params = find_element (ati_src_list, cnf_src_id, search_ati_src_id);

  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] ati_execute_config_cmd: source ID=%d not found",
                    cnf_src_id);
    return (FALSE);
  }
  if (src_params->cmd_state EQ CMD_RUNNING)
  {
    if ((ledit_get_current (src_params->src_id, &cmd) EQ LEDIT_CMPL) AND cmd)
    {
      TRACE_EVENT_P1 ("[WRN] ati_execute_config_cmd: command=%s is running", cmd->name);
    }
    else
    {
      TRACE_EVENT ("[ERR] ati_execute_config_cmd: command not available !"); /* then we have a real problem */
    }
    return (FALSE);
  }

  for (i=0; i < len; i++)
  {
    if( chars[i] EQ '\r' ) /*  */
    {
      S3_CR_found = TRUE;
      break;
    }
  }

  if (S3_CR_found EQ FALSE)
  {
    /*
     * tell line edit that the line termination is not default '\r'
     */
    T_LEDIT line;
    line.S3  = 0x00; /* <== NULL terminated cmd line */
    line.S4  = at.S[4];
    line.S5  = at.S[5];
    line.smsEnd = 0x1a;
    line.atE = ati_user_output_cfg[src_params->src_id].atE;
    ledit_set_config (src_params->src_id, line);
    len = len+1;
  }

  ati_execute (src_params->src_id, chars, len);

  /* wait for command to be processed before allowing new command */
  return (FALSE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_abort               |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL BOOL ati_abort (UBYTE src_id)
{
  aciAbort (src_id);

  return (TRUE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_run_cmd_failed      |
+-------------------------------------------------------------------+

  PURPOSE :
*/
/*
static void ati_run_cmd_failed (UBYTE src_id)
{
   T_LEDIT_ERR *err = NULL;
   *
   * get the error reason
   *
  ledit_ctrl (src_id,LEDIT_CTRL_ERROR, &err);
  if (err AND err->msg)
  {
    io_sendMessageEx (src_id, err->msg, ATI_CONFIRM_OUTPUT);
  }
  else
  {
    io_sendMessageEx (src_id, "PANIC: NULL pointer !", ATI_CONFIRM_OUTPUT);
  }
}
*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_run_ext_cmd         |
+-------------------------------------------------------------------+

  PURPOSE : run a single extended AT command got from line edit
*/
static T_ATI_RSLT ati_run_ext_cmd (T_ATI_SRC_PARAMS *src_params, T_LEDIT_ATCMD *cmd)
{
  USHORT idx_fun = 0;
  TRACE_FUNCTION("ati_run_ext_cmd()");
  /*
   * doe we have it in cmds[] table ?
   */
  if ((idx_fun = find_index(cmd->name)) EQ 0)
  {
     if (ext_v2_flag EQ TRUE)
     {
    /*
     * AT cmd not listed in cmds[], return error
     */
       cmdCmeError(CME_ERR_OpNotAllow);
       return (ATI_FAIL);
     }
     else
     {
    /*
     * AT cmd not listed in cmds[], so try it with extension mechanism
     */
       return (aci_FwrdToEXT (src_params, cmd));
     }
  }

  /*
   * remember the binary representation of the cmd
   */
  src_params->curAtCmd = cmds[idx_fun].binKey;

  switch (cmd->type.sType)
  {
    case LEDIT_ATCMD_QUERY:
    {
      if (cmds[idx_fun].qfnc)
      {
        return (cmds[idx_fun].qfnc(cmd->params, src_params->src_id));
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL); /* no function to query this cmd */
      }
    }
    case LEDIT_ATCMD_SET:
    {
      if (cmds[idx_fun].sfnc)
      {
        return (cmds[idx_fun].sfnc(cmd->params, src_params->src_id));
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL); /* no function to set this cmd */
      }
    }
    case LEDIT_ATCMD_TEST:
    {
      if (cmds[idx_fun].tfnc)
      {
        return (cmds[idx_fun].tfnc(cmd->params, src_params->src_id));
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL); /* no function to test this cmd */
      }
    }
    default:
    {
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_run_cmd             |
+-------------------------------------------------------------------+

  PURPOSE : run a single AT command got from line edit
*/
static T_ATI_RSLT ati_run_cmd (T_ATI_SRC_PARAMS *src_params, T_LEDIT_ATCMD *cmd)
{
  U16 idx_fun = 0;

  cmdErrStr = NULL; /* reset error context */

  trace_run_cmd_line ("ati_run_cmd", src_params->src_id, cmd->name, cmd->params);

  check_and_send_output_type_signal (src_params, cmd->name);

  if ((ext_v2_flag EQ TRUE) AND aci_SrchFwrdLst (src_params->src_id, cmd->name))
  {
    return (aci_FwrdToEXT (src_params, cmd));
  }

  switch (cmd->type.mType)
  {
    /*********************/
    case LEDIT_ATCMD_DIAL:
    {
      return (atD (cmd->params, src_params->src_id));
    }
    /*********************/
    case LEDIT_ATCMD_AND:
    {
      if (cmds_bas[*(cmd->name) - 'A'].and_fnc NEQ NULL)
      {
        idx_fun = *(cmd->name) - 'A';
        return (cmds_bas[idx_fun].and_fnc(cmd->params, src_params->src_id));
      }
      else if (ext_v2_flag EQ FALSE)
      {
       /*
        * basic AT& cmd not listed in cmds_bas[], so try it with extension mechanism
        */
       return (aci_FwrdToEXT (src_params, cmd));
      }
      return ATI_FAIL;
    }
    /*********************/
    case LEDIT_ATCMD_BASIC:
    {
      if (cmds_bas[*(cmd->name) - 'A'].fnc NEQ NULL)
      {
        /*
         * this is fix for atEFLMQVX, because there is a track backing to the cmd letter
         * e.g.: ATE0 -> we put E0 in tmp_buf, but pass &tmp_buf[1] !
         */
        CHAR tmp_buf[8];
        tmp_buf[0] = *(cmd->name);
        strcpy(&tmp_buf[1], cmd->params);
        idx_fun = *(cmd->name) - 'A';
        return (cmds_bas[idx_fun].fnc(&tmp_buf[1], src_params->src_id));
      }
      else if (ext_v2_flag EQ FALSE)
      {
        /*
         * basic AT cmd not listed in cmds_bas[], so try it with extension mechanism
         */
        return (aci_FwrdToEXT (src_params, cmd));
      }
      return ATI_FAIL;
    }
    /*********************/
    case LEDIT_ATCMD_S:
    {
      /*
       * unfortunately we have to put the reg number, the cmd type and the params together in one buffer
       * else the ATCommand_bas cmds_bas[] does not work
       * e.g.: tmpBuf: 3=127 --> came from ATS3=127 --> user want to change termination character to 127
       */
      char tmpBuf[20];
      switch (cmd->type.sType)
      {
        case LEDIT_ATCMD_QUERY:
        {
          sprintf(tmpBuf,"%s?", cmd->name+1);
          return (atS (tmpBuf, src_params->src_id));
        }
        case LEDIT_ATCMD_TEST:
        {
          sprintf(tmpBuf,"%s=?", cmd->name+1);
          return (atS (tmpBuf, src_params->src_id));
        }
        case LEDIT_ATCMD_SET:
        {
          sprintf(tmpBuf,"%s=%s", cmd->name+1, cmd->params);
          return (atS (tmpBuf, src_params->src_id));
        }
      }
      break;
    }
    /*********************/
    case LEDIT_ATCMD_EXTENDED:
    {
      /*
       * do we have to push to extension mechanism ?
       */
      if (!aci_cmhActive (src_params->src_id) AND aci_SrchFwrdLst (src_params->src_id, cmd->name))
      {
        if (src_params EQ NULL)
        {
          TRACE_EVENT ("[ERR] ati_execute: source ID not found");
          return (ATI_FAIL);
        }
        return (aci_FwrdToEXT (src_params, cmd));
      }
      return (ati_run_ext_cmd (src_params, cmd));
    }
  } /* end of switch (cmd->type.mType) */

/*  default: */
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_run_cmd_line_next   |
+-------------------------------------------------------------------+

  PURPOSE : - after line edit processed a raw AT command line to
              canonical form, call here the appropriate functions
              for each single AT command
*/
LOCAL T_ATI_RSLT ati_run_cmd_line_next (T_ATI_SRC_PARAMS *src_params)
{
  T_ATI_RSLT rv_ati  = ATI_FAIL;
  T_LEDIT_ATCMD *cmd = NULL;

  if (src_params->text_mode EQ TXT_MODE)
  {
    /*
     * here we are when changed from cmd to text mode
     *
     * we use the current cmd (e.g.: +CMGS) where the parameter was the destination adress
     * but now we put the text as parameter
     * example: AT+CMGS="1234567890";+CPBR=1,100
     *                              |__change to text mode (user interactively edit the text)
     * "Hello this is SMS text<Ctrl-Z>"
     *            +CMGS="Hello this is SMS text<Ctrl-Z>"  <-- call now CMGS a second time, but with text
     */
    if ((ledit_get_current (src_params->src_id, &cmd) EQ LEDIT_CMPL) AND cmd)
    {
      if (ledit_get_text (src_params->src_id, &cmd->params) EQ LEDIT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown); /* hmm, internal failure */
      }

      return (ati_run_cmd (src_params, cmd));  /* process the text */
    }
    else
    {
      cmdCmeError(CME_ERR_Unknown); /* hmm, internal failure */
      return (ATI_FAIL);
    }
  }
  else /* CMD_MODE */
  {
    cmd    = NULL;
    rv_ati = ATI_CMPL; /* when there is no further cmd */
    /*
     * and possibly next commands follows
     * (when the last cmd sent a primitive to another entity,
     *  then ati_run_cmd_line was called by rCI_OK (asynchroniously),
     *  instead of ati_execute (the synchronized case))
     */

    while (ledit_ctrl(src_params->src_id,LEDIT_CTRL_MORE_CMDS, NULL) EQ LEDIT_CMPL)
    {
      if (ledit_get_next (src_params->src_id, &cmd) EQ LEDIT_FAIL)
        return ATI_FAIL;
      if (cmd EQ NULL)
        return ATI_FAIL;
      if ((rv_ati = ati_run_cmd (src_params, cmd)) NEQ ATI_CMPL)
      {
        break;
      }
      cmd = NULL;
    }
    return rv_ati;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_run_cmd_line        |
+-------------------------------------------------------------------+

  PURPOSE : - after line edit processed a raw AT command line to
              canonical form, call here the appropriate functions
              for each single AT command
*/
LOCAL T_ATI_RSLT ati_run_cmd_line (T_ATI_SRC_PARAMS *src_params)
{
  T_ATI_RSLT rv_ati  = ATI_FAIL;

  T_LEDIT_ATCMD *cmd = NULL;
  TRACE_FUNCTION("ati_run_cmd_line()");

  if (src_params->cmd_state EQ CMD_TYPING) /* a new command line */
  {
    trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_RUNNING);
    src_params->cmd_state = CMD_RUNNING;
    /*
     * start with the first command
     */
    if ((ledit_get_first (src_params->src_id, &cmd) EQ LEDIT_CMPL) AND cmd)
    {
      rv_ati = ati_run_cmd (src_params, cmd);
    }
    else
    {
      TRACE_EVENT("failed to run first AT command !");
      cmdCmeError(CME_ERR_Unknown); /* hmm, internal failure */
    }
    if (rv_ati EQ ATI_CMPL)
    {
      rv_ati = ati_run_cmd_line_next (src_params); /* for possible next synchronious cmds */
    }
  }
  else
  {
    rv_ati = ati_run_cmd_line_next (src_params); /* for asynchronious cmds */
  }
  /*
   * evaluate the return value of the AT cmd
   */
  if (rv_ati NEQ ATI_EXCT)
  {
    /*
     * tell line edit that the cmd line is finished
     * and to be able to receive a new one
     */
    ledit_ctrl (src_params->src_id,LEDIT_CTRL_CMPL, NULL);

    trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_IDLE);
    src_params->cmd_state = CMD_IDLE;
    src_params->text_mode = CMD_MODE;
  }

  switch (rv_ati)
  {
    case (ATI_CMPL_NO_OUTPUT):
      TRACE_EVENT("AT command line processed, but look for rCI_OK for success") ;
      break;

    case (ATI_CMPL):
      /*
       * if extension command reset the global src_id_ext
       */
      if (src_id_ext EQ src_params->src_id)
      {
        src_id_ext = 0xFF;
      }

      TRACE_EVENT("AT command line successfully processed") ;
      io_sendConfirm (src_params->src_id, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
      break;

    case (ATI_FAIL):
      /*
       * if extension command reset the global src_id_ext
       */
      if (src_id_ext EQ src_params->src_id)
      {
        src_id_ext = 0xFF;
      }

      if (cmdErrStr EQ NULL)
      {
        /* set cmdErrStr to default error */
        cmdAtError(atError);
      }

      io_sendConfirm (src_params->src_id, cmdErrStr, ATI_ERROR_OUTPUT);
      cmdErrStr = NULL;
      break;

    case (ATI_FAIL_NO_OUTPUT):
      break;

    case (ATI_BUSY):
      TRACE_EVENT("BUSY !") ;
      cmdCmeError(CME_ERR_Unknown);
      io_sendConfirm (src_params->src_id, cmdErrStr, ATI_ERROR_OUTPUT);
      cmdErrStr = NULL;
      break;

    case (ATI_EXCT):
      break;

  } /* end of switch (rv_ati) */

  return (rv_ati);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_execute_cmd_line    |
+-------------------------------------------------------------------+

  PURPOSE : called by ati_execute_eval_rslt when LEDIT_CMPL
            or LEDIT_ESC (but after <ESC> a SMS there are further cmds).
            rCI_OK calls asynchroniously this function, as well
*/
/*
 * first_prompt is used by ati_execute_cmd_line() and ati_execute_eval_rslt() only
 */
static BOOL first_prompt = FALSE;

GLOBAL T_ATI_RSLT ati_execute_cmd_line (T_ATI_SRC_PARAMS *src_params)
{
  /*
   * process the command line
   */
  T_ATI_RSLT rv_ati = ati_run_cmd_line (src_params);

  if ((rv_ati EQ ATI_EXCT) AND (src_params->text_mode EQ TXT_MODE))
  {
    if(first_prompt EQ FALSE)
    {
      /*
       * emit the very first prompt "> " when switched from cmd to text mode
       */
      ledit_ctrl (src_params->src_id,LEDIT_CTRL_PROMPT, NULL);
      io_sendMessageEx (src_params->src_id, g_ledit_echoBuf, ATI_CONFIRM_OUTPUT);
      first_prompt = TRUE;
    }
  }
  else
  {
    first_prompt = FALSE; /* reset in CMD_MODE */
  }
  return (rv_ati);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_execute_eval_rslt   |
+-------------------------------------------------------------------+

  PURPOSE : evaluate the result value of line edit and execute the AT cmd line
*/
LOCAL T_ATI_RSLT ati_execute_eval_rslt (UBYTE src_id, T_ATI_SRC_PARAMS *src_params, T_LEDIT_RSLT  rv_ledit)
{
  TRACE_FUNCTION ("ati_execute_eval_rslt()");

  if ((rv_ledit EQ LEDIT_FAIL)
   OR (rv_ledit EQ LEDIT_OK))   /* for <CR> and [aAtT]<CR> they want to see an OK */
  {
    ledit_ctrl (src_id,LEDIT_CTRL_CMPL, NULL);
    trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_IDLE);
    src_params->cmd_state = CMD_IDLE;
    src_params->text_mode = CMD_MODE;
  }

  switch (rv_ledit)
  {
    case (LEDIT_CMPL):  /* line edit has a valid AT command line or text line */
    {
      char trcBuf[80];
      TRACE_EVENT ("ati_execute_eval_rslt(): rv_ledit = LEDIT_CMPL");
      /*
       * emit a trace of the entire cmd line
       * we have to put the prefix AT here, because ledit
       * keeps a canonical cmd line, where prefix and white spaces are stripped off
       */
      trcBuf[0]='A';
      trcBuf[1]='T';
      if ((ledit_trace_line (src_id, &trcBuf[2])) EQ LEDIT_CMPL)
      {
        trace_cmd_line ("IN:", trcBuf,
                               src_id,
                               (USHORT)strlen(trcBuf));
      }

      return (ati_execute_cmd_line (src_params));
    }
    case (LEDIT_FAIL): /* non valid command line */
    {
      TRACE_EVENT ("ati_execute_eval_rslt(): rv_ledit = LEDIT_FAIL");
      io_sendConfirm (src_params->src_id, cmdAtError(atError), ATI_ERROR_OUTPUT);
      cmdErrStr = NULL;
      return (ATI_FAIL);
    }
    case (LEDIT_ESC):   /* if in text mode <ESC> was sent, then no error message, but an OK */
    {
      TRACE_EVENT("ati_execute_eval_rslt(): Send message command cancelled by user and catched by line edit");
      src_params->text_mode = CMD_MODE;
      first_prompt = FALSE;

      if (ledit_ctrl(src_params->src_id,LEDIT_CTRL_MORE_CMDS, NULL) EQ LEDIT_CMPL)
      {
        /* <Esc> was sent, but there are further cmds */
        return (ati_execute_cmd_line (src_params));
      }
      else
      {
        ledit_ctrl (src_id,LEDIT_CTRL_CMPL, NULL);
        trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_IDLE);
        src_params->cmd_state = CMD_IDLE;
        io_sendConfirm (src_params->src_id, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
        return (ATI_CMPL);
      }
    }
    case (LEDIT_COLLECT):  /* line edit is (still) in COLLECT mode (both cmd/text mode) */
    {
      TRACE_EVENT ("ati_execute_eval_rslt(): rv_ledit = LEDIT_COLLECT");
      return (ATI_EXCT);
    }
    case (LEDIT_OK):  /* line edit recognized a single <CR> or simple "AT<CR>", so emit an OK */
    {
      TRACE_EVENT ("ati_execute_eval_rslt(): rv_ledit = LEDIT_OK");
      io_sendConfirm (src_params->src_id, cmdAtError(atOk), ATI_NORMAL_OUTPUT);
      return (ATI_CMPL);
    }
    case (LEDIT_IGNORE):  /* malberto will be interpreted as ato ;-) */
    {
      TRACE_EVENT ("ati_execute_eval_rslt(): rv_ledit = LEDIT_IGNORE");
      return (ATI_EXCT);
    }
    default:
    {
      TRACE_ERROR("ati_execute(): invalid state");
      return (ATI_FAIL);
    }
  } /* end of switch (rv_ledit) */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_execute             |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ATI_RSLT ati_execute (UBYTE src_id, UBYTE *chars, USHORT len)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, src_id, search_ati_src_id);

  T_LEDIT_RSLT   rv_ledit = LEDIT_FAIL;

  TRACE_FUNCTION ("ati_execute()");

  if (src_params EQ NULL)
  {
    TRACE_EVENT ("[ERR] ati_execute: source ID not found");
    return (ATI_FAIL);
  }

  /* Global variable srcId_cb is set with current source Id */ 
  srcId_cb = src_id;
  
  /*
   * the source sent a char, chunk of chars or a complete AT command line,
   * so ATI is for this source in state CMD_TYPING
   */
  if (src_params->cmd_state EQ CMD_RUNNING)
  {
    ;  /* keep CMD_RUNNING state for the second call of SMS-edit functions CMGW and CMGS in TEXT-Mode */
  }
  else /* trigger starting of new cmdline */
  {
    trace_cmd_state(src_params->src_id, src_params->cmd_state, CMD_TYPING);
    src_params->cmd_state = CMD_TYPING;
  }

  /*
   * call line edit to pre process the command line
   */
  if (src_params->text_mode EQ CMD_MODE)
  {
    rv_ledit = ledit_cmd (src_id, chars, len);
  }
  else
  {
    rv_ledit = ledit_text (src_id, chars, len);
  }
  /*
   * echo the received characters when source has been set to echo,
   * but not for e.g.: the "mAlberTO" phenomenon --> ATO, where "m lber" are ignored
   */
  if ((ati_user_output_cfg[src_params->src_id].atE) AND (rv_ledit NEQ LEDIT_IGNORE))
  {
    /* if only '\b' is printed clean the current character */
    if ((g_ledit_echoBuf[0] EQ at.S[5]) AND (g_ledit_echoBuf[1] EQ '\0'))
    {
      char delseq[4];
      delseq[0]=at.S[5];
      delseq[1]=' ';
      delseq[2]=at.S[5];
      delseq[3]='\0';
      /* This will wipe current character from terminal */
      io_sendMessageEx (src_params->src_id, delseq, ATI_ECHO_OUTPUT);
    }
    else
    {
      io_sendMessageEx (src_params->src_id, g_ledit_echoBuf, ATI_ECHO_OUTPUT);
    }
  }
  /*
   * evaluate the result value of line edit and execute the cmd line
   */
  return (ati_execute_eval_rslt (src_id, src_params, rv_ledit));
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : init_ati                |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void init_ati (void)
{
  ati_src_list = new_list ();
  rEXT_Init();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_init                |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL UBYTE ati_init (T_ATI_SRC_TYPE         src_type,
                       T_ATI_RESULT_CB        *result_cb,
                       T_ATI_LINE_STATE_CB    *line_state_cb)
{
  UBYTE             src_id;
  T_ATI_SRC_PARAMS  *src_params;

  TRACE_FUNCTION ("ati_init ()");

  src_id = aci_init (src_type);
  if (src_id EQ 0)
  {
    return (0);
  }

  aci_cmd_src_mode_set(src_id,CMD_MODE_ATI);

#ifdef FF_ATI_BAT
  /* we use test or uart source as application and user of the BAT library */
  if ((src_type EQ ATI_SRC_TYPE_TST) 
   OR (src_type EQ ATI_SRC_TYPE_UART))
  {
    if (ati_bat_globs.bat_lib_initialized EQ FALSE)
    {
      ati_bat_lib_init(src_id);
    } 
    ati_bat_lib_new(src_id, src_type); /* one instance per ATI source */
    ati_bat_open_client(src_id);       /* but only one client per instance */
  } 
#endif /* FF_ATI_BAT */

  ACI_MALLOC (src_params, sizeof (T_ATI_SRC_PARAMS));

  src_params->src_id            = src_id;
  src_params->src_type          = src_type;
  src_params->result_cb         = result_cb;
  src_params->line_state_cb     = line_state_cb;
  src_params->cmd_state         = CMD_IDLE;
  src_params->buff_uns_mode     = BUFF_RUNNING;
  src_params->indication_buffer = NULL;
  src_params->text_mode         = CMD_MODE;
  src_params->curAtCmd          = AT_CMD_NONE;
  src_params->mode              = ATI_UNKN_MODE;

  insert_list (ati_src_list, src_params);

  return (src_id);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ati_cmd                 |
|                                 ROUTINE : ati_finit               |
+-------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL void ati_finit (UBYTE src_id)
{
  T_ATI_SRC_PARAMS  *src_params;

  TRACE_EVENT_P1("ati_finit(): source: %d", src_id);

  src_params = remove_element (ati_src_list, src_id, search_ati_src_id);
  if (src_params EQ NULL)
  {
    /* often seen with pei_exit() during simulation, because tries to close all sources,
       but there are not so many open */
    TRACE_EVENT_P1 ("ati_finit(): source: %d not found", src_id);
    return;
  }

  ACI_MFREE (src_params);

  aci_finit (src_id);

#ifdef FF_ATI_BAT
  ati_bat_close_client(src_id);
  if (ati_bat_all_clients_closed())
  {
    ati_bat_lib_finit();
  }
#endif 
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: aci_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : A timeout has occured for a timer. The function returns
            TRUE, if it is a timer which is handled by ACI, else FALSE
            is returned to indicate that the timer has not been
            processed.

*/
UBYTE aci_timeout (USHORT index)
{
  TRACE_FUNCTION ("aci_timeout()");

  if (index EQ ACI_TRING)
  {
    /*
     * timeout ring timer
     */
    aciRingTimeout ();
    return TRUE;
  }
  if (index EQ ACI_CNMA_TIMER_HANDLE)
  {
    /*
     * timeout +CNMA waiting timer
     */
    aciCnmaTimeout();
    return TRUE;
  }

  /*
   * the timeout is not for ACI
   */
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: aciRingTimeout      |
+--------------------------------------------------------------------+

  PURPOSE : +CNMA timeout. Relay function for cmd_handleCnmaTimeout();
*/
void aciCnmaTimeout( void )
{
  cmd_handleCnmaTimeout();
}


/*
+--------------------------------------------------------------------+
| PROJECT: GSM-PS (6147)                MODULE:  ACI_CMD             |
| STATE  : code                         ROUTINE: aciRingTimeout      |
+--------------------------------------------------------------------+

  PURPOSE : Ring timeout. Check S0 register for automatic answer mode.
            Indicate ring and accept call if S0 value is reached
*/
void aciRingTimeout ( void )
{
  UBYTE idx;

  TRACE_FUNCTION ("aciRingTimeout()");

  /* indicate ringing */
  at.rngPrms.rngCnt++;

  for (idx = 0; idx < CMD_SRC_MAX; idx++)
  {
    if ((idx NEQ CMD_SRC_LCL) AND IS_SRC_USED (idx))
    {
      srcId_cb = idx;
      if (!at.rngPrms.isRng) /* only handle the subsequent sources if ringing was not stopped */
        break;
      rCI_PlusCRING ( at.rngPrms.mode, at.rngPrms.type1, at.rngPrms.type2 );

#ifdef NO_ASCIIZ
      rCI_PlusCLIP  ( at.clipPrms.stat, strlen(at.clipPrms.number)?(at.clipPrms.number):NULL, &at.clipPrms.type,
                      at.clipPrms.validity, strlen(at.clipPrms.subaddr)?(at.clipPrms.subaddr):NULL,
                      &at.clipPrms.satype, (at.clipPrms.alpha.len)?(&at.clipPrms.alpha):NULL );
#else
      rCI_PlusCLIP  ( at.clipPrms.stat, strlen(at.clipPrms.number)?(at.clipPrms.number):NULL, &at.clipPrms.type,
                      at.clipPrms.validity, strlen(at.clipPrms.subaddr)?(at.clipPrms.subaddr):NULL,
                      &at.clipPrms.satype, strlen(at.clipPrms.alpha)?(at.clipPrms.alpha):NULL );
#endif
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : find_index         |
+--------------------------------------------------------------------+

  PURPOSE : find index_function

*/
static SHORT find_index(CHAR * cl)
{
  int i = 1;

  while (cmds[i].key NEQ NULL)
  {
    if (strcmp (cl, cmds[i].key) EQ 0) /* command found */
      return (i);

    i++;
  }

  return (0);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ATI                          |
| STATE   : code             ROUTINE : ati_get_cmds_key             |
+-------------------------------------------------------------------+

  PURPOSE : This function returns the cmd key "+CMD" from the command id
*/

GLOBAL void ati_get_cmds_key (T_ACI_AT_CMD cmd_id, CHAR **cmd_key, CHAR **testcmd_output)
{
  int i = 1;

  while (cmds[i].key NEQ NULL)
  {
    if (cmds[i].binKey EQ cmd_id) /* command found */
    {
      break;
    }
    i++;
  }

  if (cmds[i].key EQ NULL)
  {
    TRACE_EVENT_P1("Command not found !!! cmd_id: %d", cmd_id);
    return;
  }

  if (cmd_key NEQ NULL)
  {
    *cmd_key = cmds[i].key;
  }

  if (testcmd_output NEQ NULL)
  {
    *testcmd_output = cmds[i].output1;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : resp_disp          |
+--------------------------------------------------------------------+

  PURPOSE : display response
*/

GLOBAL CHAR *resp_disp (UBYTE srcId, CHAR *cl,CHAR *type, ...)
      /*types: "llsle..." b:BYTE l:LONG s:SHORT
                     e:enum or int   x: enum in X  */
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  /*lint -e516 -e10*/
  va_list varpars;          /* varpars not to be initialised as it is not scalar */
  USHORT  i           = 0;
  SHORT   pos,
          buf_length,
          type_length = 0;
  CHAR    *cmd_key = "";

  if (type NEQ NULL)
  {
    type_length = strlen(type);
  }
 
  va_start (varpars, type);

  ati_get_cmds_key (src_params->curAtCmd, &cmd_key, NULL);

  pos = sprintf (g_sa,"%s: ", cmd_key);

  while ((type NEQ NULL) AND (i < type_length))
  {
    switch(*type)
    {
    case('b'):
       pos += sprintf(g_sa+pos,"%d,",*(va_arg (varpars,unsigned char*)));
      break;
    case('e'):
      pos += sprintf(g_sa+pos,"%d,",*(va_arg (varpars,int*)));
      break;
    case('l'):
      pos += sprintf(g_sa+pos,"%d,",*(va_arg (varpars,long*)));
      break;
    case('s'):
      pos += sprintf(g_sa+pos,"%d,",*(va_arg (varpars,short*)));
      break;
    case('x'):
      pos += sprintf(g_sa+pos,"%X,",*(va_arg (varpars,int*)));
    }
    i++;
    type++;
  }
  buf_length = strlen(g_sa);
  g_sa[buf_length - 1] = '\0'; /* to get rid of the last comma */

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  va_end (varpars);        /* Reset variable arguments.      */
  return cl;
/*lint +e516 +e10*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : test_gen           |
+--------------------------------------------------------------------+

  PURPOSE : generic test function
*/

static T_ATI_RSLT test_gen(CHAR * cl, UBYTE srcId)
{
  CHAR *cmd_key = "";
  CHAR *testcmd_output = "";
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("test_gen()");

  ati_get_cmds_key (src_params->curAtCmd, &cmd_key, &testcmd_output);

  if (testcmd_output NEQ NULL)
  {
    sprintf (g_sa, testcmd_output, cmd_key);
    io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLAC         |
+--------------------------------------------------------------------+

  PURPOSE : +CLAC command (List AT commands list)
*/

GLOBAL T_ATI_RSLT atPlusCLAC (char *cl, UBYTE srcId)
{
  int i = 1 /*0*/; /* because of new cmds[] table */
  int j = 0;

  TRACE_FUNCTION("atPlusCLAC()");

  if ( *cl EQ '\0' OR *cl EQ ';' )
  {
    if ( *cl EQ ';' )
      cl++;

      while (cmds[i].key NEQ NULL)
      {
        sprintf(g_sa,"AT%s",cmds[i].key);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        i++;
      }
    for (;j<26;j++)
    {
      if (cmds_bas[j].key)                 /* Output if present */
      {
        sprintf(g_sa,"AT%s",cmds_bas[j].key);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
      if (cmds_bas[j].and_key)             /* Output if present */
      {
        sprintf(g_sa,"AT%s",cmds_bas[j].and_key);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
    }
  }
  else
  {
    cmdCmeError (CME_ERR_OpNotSupp);
    return (ATI_FAIL);
  }

  return (ATI_CMPL);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusGCI          |
+--------------------------------------------------------------------+

  PURPOSE : +GCI command (manufacturer identification)
*/

GLOBAL T_ATI_RSLT setatPlusGCI(char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret;
  UBYTE           country; /* check if long enough */

  TRACE_FUNCTION("setatPlusGCI()");

  switch (*cl)
  {
  case('='):
    {
      cl++;
      cl=parse(cl,"h",&country);
      if(!cl)/*OR country > 0xff) this expression always returns false (LINT)*/
      {
        cmdAtError(atError);
        return (ATI_FAIL);
      }
      ret = sAT_PlusGCI((T_ACI_CMD_SRC)srcId ,country);
      break;
    }
  default:
    {
      cmdAtError(atError);
      return (ATI_FAIL);
    }
  }
  if (ret EQ AT_FAIL)
  {
    cmdAtError(atError);
    return (ATI_FAIL);
  }
  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPlusGCI(char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret;
  UBYTE           country; /* check if long enough */

  TRACE_FUNCTION("atPlusGCI()");

  ret=qAT_PlusGCI((T_ACI_CMD_SRC)srcId,&country);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+GCI:%02X",country);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdAtError(atError);
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCCWE         |
+--------------------------------------------------------------------+

  PURPOSE : +CCWE Call Meter Maximum Event
*/

GLOBAL T_ATI_RSLT setatPlusCCWE(char *cl, UBYTE srcId)
{
  SHORT mode = ACI_NumParmNotPresent;

  TRACE_FUNCTION ( "setatPlusCCWE()" );

  cl = parse ( cl, "d", &mode );
  if ( !cl OR mode > 1 OR mode < 0 )
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return (ATI_FAIL);
  }

  ati_user_output_cfg[srcId].CCWE_stat = ( UBYTE ) mode;
  return (ATI_CMPL);
}


GLOBAL T_ATI_RSLT queatPlusCCWE(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION ( "queatPlusCCWE()" );

  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].CCWE_stat);
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_sendPString    |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to send a PASCAL like string to
            the serial interface.
*/
GLOBAL void aci_sendPString (UBYTE srcId, CHAR* buffer)
{
  UBYTE nextLen;
  UBYTE actLen;

  TRACE_FUNCTION ( "aci_sendPString ()" );

  nextLen = buffer[0];

  while ( nextLen NEQ ACI_EXT_STRG_END)
  {
    actLen             = nextLen;
    nextLen            = buffer[actLen + 1];
    buffer[actLen + 1] = '\0';

    io_sendMessage ( srcId, &buffer[1], ATI_NORMAL_OUTPUT );

    buffer[actLen + 1] = (CHAR)nextLen;

    buffer = &buffer[actLen + 1];
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : sAT_CfgEXT         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to store the list of AT command
            which are forwarded to the AT command extension module
            in case mobile is not registered (neither limited nor
            full service is available).
*/
GLOBAL T_ACI_RETURN sAT_CfgEXT ( CHAR** fwrdLst )
{
  TRACE_FUNCTION ( "sAT_CfgEXT ()" );

  atCmdFwrdLst = fwrdLst;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_SrchFwrdLst    |
+--------------------------------------------------------------------+
  PURPOSE : This function is used to search for a specific string in
            the list of AT commands which will be forwarded if
            necessary.
*/
LOCAL BOOL aci_SrchFwrdLst ( UBYTE srcId, CHAR* searchFor )
{
  CHAR** pTmp;
  int i=0;

  TRACE_FUNCTION ( "aci_SrchFwrdLst ()" );

  if (ext_v2_flag EQ TRUE)
  {
    TRACE_EVENT ("New Extension Mechanism...");
    pTmp = atCmdFwrdLst_v2;
  }
  else
  {
    TRACE_EVENT ("Old Extension Mechanism...");
    pTmp = atCmdFwrdLst;
  }

  if (pTmp)
  {
    while (*(pTmp+i) NEQ NULL)
    {
      if ( strcmp ( *(pTmp+i), searchFor ) EQ 0 )
      {
        TRACE_EVENT_P1 ( "aci_SrchFwrdLst(): %s found", searchFor);
        return TRUE;
      }
      i++;
    }
  }
  else
  {
    TRACE_EVENT( "aci_SrchFwrdLst(): atCmdFwrdLst does not exist");
  }
  return FALSE;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_FwrdToEXT      |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to forward an AT command to the
            extension handler.
*/
LOCAL T_ATI_RSLT aci_FwrdToEXT (T_ATI_SRC_PARAMS *src_params, T_LEDIT_ATCMD *cmd)
{
  USHORT lenExt        = 0;
  char   *extCmd       = NULL;
  T_ACI_RETURN extRslt = AT_FAIL;

  TRACE_FUNCTION ( "aci_FwrdToEXT ()" );

  ACI_MALLOC(extCmd, MAX_CMD_LEN);
  *extCmd = '\0';

  if (src_id_ext NEQ 0xFF)
  {
    TRACE_EVENT("aci_FwrdToEXT(): src_id_ext has not been reset to 0xFF");
  }

  /*
   * only this source can currently run extension commands
   * the reset to 0xFF takes place in ati_run_cmd_line() and rCI_PlusCME() (asynchronious case)
   */
  src_id_ext = src_params->src_id;
  /*
   * we have to build up the extension cmd in the same old manner before using of line edit
   */
  /* All sprintfs below are now replaced by strcats . Bug 18239 */

  if(cmd->type.append_Flag EQ TRUE)
  {
    switch(cmd->type.sType)
    {
      case LEDIT_ATCMD_SET :
      {
        if ((strlen(cmd->params)) NEQ 0)
        {
          strcat(extCmd,cmd->name);
          strcat(extCmd,"=");
          strcat(extCmd,cmd->params);
        }
        else
        {
          strcat(extCmd,cmd->name);
        }
        break;
      }

      case  LEDIT_ATCMD_QUERY :
      {
        if ((strlen(cmd->params)) NEQ 0)
        {
          strcat(extCmd,cmd->name);
          strcat(extCmd,"?");
          strcat(extCmd,cmd->params);
        }
        else
        {
          strcat(extCmd,cmd->name);
          strcat(extCmd,"?");
        }
        break;
      }

      case LEDIT_ATCMD_TEST:
      {
        if ((strlen(cmd->params)) NEQ 0)
        {
          strcat(extCmd,cmd->name);
          strcat(extCmd,"=");
          strcat(extCmd,"?");
          strcat(extCmd,cmd->params);
        }
        else
        {
          strcat(extCmd,cmd->name);
          strcat(extCmd,"=");
          strcat(extCmd,"?");
        }
        break;
      }
    }
  }

  if(cmd->type.append_Flag EQ FALSE)
  {
     switch(cmd->type.sType)
     {
       case LEDIT_ATCMD_SET:
       case LEDIT_ATCMD_QUERY:
       case LEDIT_ATCMD_TEST:
       {
         strcat(extCmd,cmd->name);
         break;
       }
       default:
       {
         strcat(extCmd,cmd->name);
         break;
       }
     }
  }

  lenExt = strlen(extCmd);
  TRACE_EVENT_P1("aci_FwrdToEXT(): Total Command Length %x ",  lenExt);

  if (strchr (OLD_NON_STANDARDIZED_ATCMD, extCmd[0]))
  {
    if (*extCmd EQ '&')
    {
      if (strchr (OLD_NON_STANDARDIZED_ATCMD, extCmd[1]) EQ NULL)
      {
        TRACE_EVENT("[ERR]aci_FwrdToEXT ()");
        cmdAtError (atError);
        ACI_MFREE(extCmd);
        return (ATI_FAIL);
      }
    }
  }

  send_output_type_signal (src_params, ATI_OUTPUT_TYPE_NORMAL);

  g_sa[0] = (CHAR) ACI_EXT_STRG_END;

  TRACE_EVENT_P1("aci_FwrdToEXT(): running cmd %s", extCmd);
  if (ext_v2_flag EQ TRUE)
  {
    extRslt = rEXT_Execute (src_params->src_id, extCmd);
  }
  else
  {
    extRslt = rAT_EXT (extCmd, &lenExt, g_sa, MAX_CMD_LEN - 1);
    aci_sendPString (src_params->src_id, g_sa);
  }

  ACI_MFREE(extCmd);

  switch (extRslt)
  {
    case (AT_CMPL):
    {
      cmdErrStr = NULL;
      TRACE_EVENT("aci_FwrdToEXT(): returns with ATI_CMPL");
      return (ATI_CMPL);
    }
    case (AT_EXCT):
    {
      cmdErrStr   = NULL;
      src_params->curAtCmd = AT_CMD_EXT;
      TRACE_EVENT("aci_FwrdToEXT(): returns with ATI_EXCT");
      return (ATI_EXCT);
    }

    default:
    {
      cmdAtError (atError);
      TRACE_EVENT("aci_FwrdToEXT(): returns with ATI_FAIL");
      return (ATI_FAIL);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_cmhActive      |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to calculate whether the mobile is
            in service.
*/
LOCAL BOOL aci_cmhActive ( UBYTE srcId )
{
  BOOL           rslt     = FALSE;
  T_ACI_NRG_RGMD regMode;
  T_ACI_NRG_SVMD srvMode;
  T_ACI_NRG_FRMT oprFrmt;
  T_ACI_NRG_SVMD srvStat;
  CHAR           oper[MAX_ALPHA_OPER_LEN];

  TRACE_FUNCTION ( "aci_cmhActive ()" );

  qAT_PercentNRG( (T_ACI_CMD_SRC)srcId,
                  &regMode,
                  &srvMode,
                  &oprFrmt,
                  &srvStat,
                  oper);

  if ( srvStat EQ NRG_SVMD_Full    OR
       srvStat EQ NRG_SVMD_Limited    )
  {
    rslt = TRUE;
  }

  return rslt;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : ati_switch_mode    |
+--------------------------------------------------------------------+

  PURPOSE : changes mode from command mode to data mode and vice-versa.

*/

GLOBAL void ati_switch_mode (UBYTE srcId,
                            T_ATI_IO_MODE mode)
{
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendMessage: srcId=%d not found", srcId);
    return;
  }

  src_params->mode = mode;

  if (mode EQ ATI_CMD_MODE)
  {
    TRACE_EVENT("IO mode changed to CMD MODE");
    cmd_flushCievBuf(src_params->src_id);

    /* The CNMI buffer is flushed only if the current source
     * matches with the source interested in SMS indications */ /* Issue 25033 */
    if ( src_params->src_id EQ smsShrdPrm.smsSrcId )
    {
      /* If Service is not GSM phase 2+, flushing is done at once,
         because acknowledge is not necessary */
      if( smsShrdPrm.CSMSservice NEQ CSMS_SERV_GsmPh2Plus )
      {
        cmd_flushCnmiBuf();
      }
      else
      {
        cmd_flushCnmiBufOneByOne();
      }
    }
  }
  else
  {
    TRACE_EVENT("IO mode changed to DATA MODE");
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : ati_get_mode       |
+--------------------------------------------------------------------+

  PURPOSE : returns current IO-mode
*/
GLOBAL T_ATI_IO_MODE ati_get_mode( UBYTE srcId )
{
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
    TRACE_EVENT_P1 ("[ERR] io_sendMessage: srcId=%d not found", srcId);
    return ATI_UNKN_MODE;
  }

  return( src_params->mode );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS                      MODULE  : ATI_IO             |
| STATE   : code                        ROUTINE : format             |
+--------------------------------------------------------------------+

  PURPOSE : format the ouput

*/

GLOBAL USHORT format_output (UBYTE *src, USHORT src_len, UBYTE *dst,
                             T_ATI_OUTPUT_TYPE output_type)
{
  USHORT dst_len = 0;

  if (at.s1415.atV)
  {
    /*
     * verbose mode
     */
    if (IS_BEGIN_CRLF_OUTPUT (output_type))
    {
      dst[0] = at.S[3];
      dst_len++;
      dst[1] = at.S[4];
      dst_len++;
    }

    memcpy (dst + dst_len, src, src_len);
    dst_len += src_len;

    if (IS_END_CRLF_OUTPUT (output_type))
    {
      dst[dst_len] = at.S[3];
      dst_len++;
      dst[dst_len] = at.S[4];
      dst_len++;
    }

    return (dst_len);
  }

  /*
   * non-verbose mode
   */

  memcpy (dst + dst_len, src, src_len);
  dst_len += src_len;

  if (IS_ECHO_OUTPUT (output_type))
  {
    return (dst_len);
  }

  dst[dst_len] = at.S[3];
  dst_len++;

  if (!(IS_CONFIRM_OUTPUT(output_type) OR IS_RESULT_CODE_OUTPUT(output_type)))
  {
    dst[dst_len] = at.S[4];
    dst_len++;
  }

  return (dst_len);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_RET            |
| STATE   : code                        ROUTINE : ati_is_src_type    |
+--------------------------------------------------------------------+

  PURPOSE : this function checks if the given source ID is of given type
  (example: is it a SAT source (for RUN AT COMMAND)
*/

GLOBAL BOOL ati_is_src_type( UBYTE srcId, T_ATI_SRC_TYPE source_type )
{
#ifdef SIM_TOOLKIT
  T_ATI_SRC_PARAMS *src_params;
#endif /* SIM_TOOLKIT */

  if (!IS_SRC_USED(srcId))
  {
    return(FALSE);
  }

#ifdef SIM_TOOLKIT
  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if (src_params EQ NULL)
  {
     return FALSE;
  }

  if( src_params->src_type EQ source_type )
  {
    TRACE_EVENT_P2("source %d is of type %d", srcId, source_type);
    return( TRUE );
  }
  else
#endif /* SIM_TOOLKIT */ /* always returns FALSE if SIM_TOOLKIT isn't defined */
  {
    return( FALSE );
  }
}


/*
  PURPOSE :This function takes in the AT command string as input parameter and
returns the corresponding command ID. This function provides an interface for
get_command_id_in_wn_list () to get the command Id from a local table cmd[ ]
with a command name string as input.
*/

GLOBAL T_ACI_AT_CMD get_command_id (CHAR *command_str)
{
    int i;
    T_ACI_AT_CMD command =  AT_CMD_NONE;

    TRACE_FUNCTION ("get_command_id()");

    for (i = 1; cmds [i].key NEQ NULL AND strcmp (cmds [i].key, command_str); i ++)  { }
    command = cmds [i].binKey;
    TRACE_EVENT_P1 ("command Id is: %d", command);
    return (command);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_Output        |
+--------------------------------------------------------------------+

  PURPOSE : this function provides the interface for the extension mechanism to output
                   messages to the source. If the message is too long the calling function can
                   seperate the message into several parts and choose the right output type.
*/

GLOBAL T_ATI_RSLT sEXT_Output (UBYTE  src_id, T_ATI_EXT_FORMAT output_format,
                                  CHAR  *output)
{
   T_ATI_SRC_PARAMS *src_params;

   TRACE_FUNCTION ("sEXT_Output()");
   src_params = find_element (ati_src_list, src_id, search_ati_src_id);
   if (src_params EQ NULL)
   {
     TRACE_EVENT_P1 ("[ERR] sEXT_Output: srcId=%d not found", src_id);
     return ATI_FAIL;
   }
/*   if (src_id NEQ src_id_ext)
   {
     TRACE_EVENT ("[ERR] sEXT_Output: No extension command running.");
     return ATI_FAIL;
   } */
   switch (output_format)
   {
     case ATI_EXT_PART_BEGIN:
      io_sendMessageEx(src_id, output, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT|ATI_BEGIN_CRLF_OUTPUT ));
      break;
     case ATI_EXT_PART_LAST:
      io_sendMessageEx(src_id, output, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT|ATI_END_CRLF_OUTPUT ));
      break;
     case ATI_EXT_PART_LINE:
      io_sendMessageEx(src_id, output, ATI_NORMAL_OUTPUT);
      break;
     case ATI_EXT_CMPL_LINE:
      io_sendMessageEx(src_id, output, (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT|ATI_END_CRLF_OUTPUT ));
      break;
     default:
      return ATI_FAIL;
   }
   return ATI_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_Indication    |
+--------------------------------------------------------------------+

  PURPOSE : this function provides the interface for the extension mechanism to output
                   indications (unsolistic output) to the source.
*/

GLOBAL T_ATI_RSLT sEXT_Indication (UBYTE src_id, CHAR *indication_string)
{
   TRACE_FUNCTION ("sEXT_Indication()");
   if (indication_string EQ NULL)
   {
     TRACE_EVENT ("[ERR] sEXT_Indication: No indication string.");
     return ATI_FAIL;
   }
   if (src_id NEQ src_id_ext)
   {
     TRACE_EVENT ("[ERR] sEXT_Indication: No extension command is running.");
     return ATI_FAIL;
   }
   io_sendIndication(src_id, indication_string, ATI_FORCED_OUTPUT);
   return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_Confirm       |
+--------------------------------------------------------------------+

  PURPOSE : this function provides the interface for the extension mechanism to output
                   a confirmations to the source. A confirmation should be sent in the cases
                   where an AT command runs for a long time and only ATI_EXT_EXCT has
                   been returned.
*/
GLOBAL T_ATI_RSLT sEXT_Confirm (UBYTE src_id)
{
   T_ATI_SRC_PARAMS *src_params;
   TRACE_FUNCTION ("sEXT_Confirm()");

   src_params = find_element (ati_src_list, src_id, search_ati_src_id);
   if (src_params EQ NULL)
   {
     TRACE_EVENT_P1 ("[ERR] sEXT_Confirm: srcId=%d not found", src_id);
     return ATI_FAIL;
   }
   if (src_id NEQ src_id_ext)
   {
     TRACE_EVENT ("[ERR] sEXT_Confirm: No extension command running.");
     return ATI_FAIL;
   }
   TRACE_EVENT("Extension command signals a successful result.") ;
   srcId_cb = src_id;
   rCI_OK (AT_CMD_EXT);
   return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_Error         |
+--------------------------------------------------------------------+

  PURPOSE : this function provides the interface for the extension mechanism to output
            an error message to the source. An error message should be sent in the
            cases where error happens or when an AT command runs for a long time
            and only ATI_EXT_EXCT has been returned before an error happens. By
            calling this function the AT interpreter will be freed to run a second command.
*/
GLOBAL T_ATI_RSLT sEXT_Error  (UBYTE src_id, T_ACI_CME_ERR err)
{
   TRACE_FUNCTION ("sEXT_Error()");
   if (src_id NEQ src_id_ext)
   {
     TRACE_EVENT ("[ERR] sEXT_Error: No extension command running.");
     return ATI_FAIL;
   }
   srcId_cb = src_id;
   rCI_PlusCME (AT_CMD_EXT, err);
   return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_Init          |
+--------------------------------------------------------------------+

  PURPOSE : this function provides the interface for the extension mechanism
*/
GLOBAL T_ATI_RSLT sEXT_Init (CHAR *cmd_list[])
{
  TRACE_FUNCTION("sEXT_Init()");
  if (*cmd_list EQ NULL)
  {
    TRACE_EVENT ("The old EXT mechanism will be used.");
    ext_v2_flag = FALSE;
  }
  else
  {
    TRACE_EVENT ("The new EXT mechanism will be used.");
    ext_v2_flag = TRUE;
    atCmdFwrdLst_v2 = cmd_list;
  }
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : sEXT_finit         |
+--------------------------------------------------------------------+

  PURPOSE : this function frees the memory of the extension command list

GLOBAL T_ATI_RSLT sEXT_Finit ()
{
  ACI_MFREE(atCmdFwrdLst);
  return ATI_CMPL;
}*/
#endif /* ATI_CMD_C */


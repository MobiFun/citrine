/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SMSF
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
|  Purpose :  This module defines the functions for the protocol
|             stack adapter for the registration part of mobility
|             management. 
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SMSF_C
#define PSA_SMSF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#undef TRACING

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "aci_lst.h"
#include "aci_mem.h"
#include "psa.h"
#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#endif
#include "psa_sms.h"
#include "psa_util.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "psa_sim.h"

#if (defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)
#include "conc_sms.h"
#endif

/*==== CONSTANTS ==================================================*/
#define ITM_WDT         (14)    /* item width in chars */
#define HDR_WDT         (10)    /* header width in chars */

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
GLOBAL T_ACI_LIST *set_prm_list = NULL;
/*==== FUNCTIONS ==================================================*/

#ifdef SIM_TOOLKIT
EXTERN BOOL cmhSMS_FileUpdate (int, T_SIM_FILE_UPDATE_IND *);
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSF                |
|                                 ROUTINE : psaSMS_Init             |
+-------------------------------------------------------------------+

  PURPOSE : initialize the protocol stack adapter for SMS.

*/

/* MACRO: initializer for set parameter */
#define INIT_SET_PARM( dest, def )\
    {smsShrdPrm.pSetPrm[0]->dest = def;}





GLOBAL void psaSMS_InitParams ( void )
{
  INIT_SET_PARM( sca.ton,     SMS_TON_UNKNOWN                     );
  INIT_SET_PARM( sca.npi,     SMS_NPI_UNKNOWN                     );
  INIT_SET_PARM( sca.c_num,   0                                   );
  INIT_SET_PARM( vpRel,       TP_VP_RELATIVE_DEFAULT              );
  INIT_SET_PARM( pid,         SMS_PID_DEFAULT                     );
  INIT_SET_PARM( dcs,         SMS_DCS_GRP_DEF                     );
  INIT_SET_PARM( msgType,     (TP_MTI_SMS_SUBMIT + TP_VPF_RELATIVE));

  smsShrdPrm.cbmPrm.cbmFoundIds          = 0;
  memset(smsShrdPrm.cbmPrm.msgId, DEF_MID_RANGE, sizeof(smsShrdPrm.cbmPrm.msgId));
}
GLOBAL void psaSMS_Init ( void )
{
  int LpCnt;            /* holds loop counter for macro */
  T_SMS_SET_PRM * elem;

/*
 *-------------------------------------------------------------------
 * set default parms
 *-------------------------------------------------------------------
 */
  smsShrdPrm.smsStat      =  SMS_STATE_NOT_AVAILABLE;
  smsShrdPrm.accessEnabled = FALSE;
  smsShrdPrm.aci_sms_parameter.simTotal = 0;
  smsShrdPrm.aci_sms_parameter.simUsed  = 0;
  smsShrdPrm.aci_sms_parameter.meTotal  = 0;
  smsShrdPrm.aci_sms_parameter.meUsed   = 0;
  smsShrdPrm.owner        = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.cbmPrm.cbchOwner    = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.rslt         = 0x200/*CS_OK*/;

  smsShrdPrm.aci_sms_parameter.snd_msg_ref  = 0;

  smsShrdPrm.aci_sms_parameter.smsParamRecLen = 0;
  smsShrdPrm.aci_sms_parameter.smsParamMaxRec = 0;
  smsShrdPrm.mtHndl               = MT0;
  smsShrdPrm.srHndl               = DS0;
  smsShrdPrm.cbmPrm.cbmHndl              = BM0;
  smsShrdPrm.cbmPrm.cbmMode              = CBCH_ACCEPT;
  smsShrdPrm.cbmPrm.cbmFoundIds          = 0;
  smsShrdPrm.cbmPrm.cbmSIMmaxId          = 0;
  smsShrdPrm.cbmPrm.cbmSIMmaxIdRge       = 0;

  smsShrdPrm.tpdu.tp_submit              = NULL;
  smsShrdPrm.tpdu.tp_deliver             = NULL;
  smsShrdPrm.tpdu.tp_command             = NULL;

#ifdef FF_HOMEZONE
  smsShrdPrm.cbmPrm.hzMode    = CBHZ_MOD_NotActive;
  smsShrdPrm.cbmPrm.hzDcs     = CS_GsmDef;
  smsShrdPrm.cbmPrm.hzTimeout = CBHZ_DEF_TIMEOUT;
#endif /* FF_HOMEZONE */

  smsShrdPrm.mem1                        = MEM_SM;
  smsShrdPrm.mem2                        = MEM_SM;
  smsShrdPrm.mem3                        = MEM_SM;
  smsShrdPrm.status                      = CMGD_DEL_INDEX;
  smsShrdPrm.cnma_ack_expected           = FALSE;
#ifdef REL99
  smsShrdPrm.auto_repeat_flag            = CMGRS_MODE_DISABLE_AUTO_RETRANS;
  smsShrdPrm.is_msg_present_for_retrans  = FALSE;
#endif /* REL99 */

  smsShrdPrm.uiInternalSmsStorage        = CMD_SRC_NONE;
  smsShrdPrm.smsSrcId                    = CMD_SRC_NONE;

  smsShrdPrm.CSMSservice                 = CSMS_SERV_GsmPh2;
  smsShrdPrm.CSMSmt                      = CSMS_SUPP_Enable;
  smsShrdPrm.CSMSmo                      = CSMS_SUPP_Enable;
  smsShrdPrm.CSMSbm                      = CSMS_SUPP_Enable;

  smsShrdPrm.CNMImt                      = CNMI_MT_NoSmsDeliverInd;
  smsShrdPrm.CNMIbm                      = CNMI_BM_NoCbmInd;
  smsShrdPrm.CNMIds                      = CNMI_DS_NoSmsStatRpt;


  smsShrdPrm.prmRdSeq                    = SMS_READ_SIM_CMPL;
  smsShrdPrm.pDecMsg                     = NULL;
  
  smsShrdPrm.rplyCB.cmss                 = NULL;
  smsShrdPrm.errorCB                     = NULL;

  smsShrdPrm.CMMSmode                    = CMMS_MODE_DEF;

#ifdef FF_MMI_RIV
  smsShrdPrm.perccmgf_smbs_mode = PERC_SMBS_MOD_DISABLE;
#endif /* #ifdef FF_MMI_RIV */

  if (set_prm_list EQ NULL)
  {
    set_prm_list = new_list();
    ACI_MALLOC(elem, sizeof(T_SMS_SET_PRM));
    memset(elem, 0, sizeof(T_SMS_SET_PRM));
    insert_list(set_prm_list, elem);
    
    /* all set prm pointer should point to this element */
    for( LpCnt = 0; LpCnt < OWN_SRC_MAX; LpCnt++ )
    {
      smsShrdPrm.pSetPrm[LpCnt] = elem;
    }
  }

#ifndef SMS_PDU_SUPPORT
  INIT_SET_PARM( CMGFmode,     CMGF_MOD_Txt                       );
#else
  INIT_SET_PARM( CMGFmode,     CMGF_MOD_Pdu                       );
#endif

  INIT_SET_PARM( prflId,      1                                   );
  INIT_SET_PARM( isCopy,      FALSE                               );
  INIT_SET_PARM( numOfRefs,   OWN_SRC_MAX                             );
  INIT_SET_PARM( sca.ton,     SMS_TON_UNKNOWN                     );
  INIT_SET_PARM( sca.npi,     SMS_NPI_UNKNOWN                     );
  INIT_SET_PARM( sca.c_num,   0                                   );
  INIT_SET_PARM( vpRel,       TP_VP_RELATIVE_DEFAULT              );
  INIT_SET_PARM( pid,         SMS_PID_DEFAULT                     );
  INIT_SET_PARM( dcs,         SMS_DCS_GRP_DEF                     );
  INIT_SET_PARM( msgType,     (TP_MTI_SMS_SUBMIT + TP_VPF_RELATIVE));

  memset(smsShrdPrm.cbmPrm.msgId, DEF_MID_RANGE, sizeof(smsShrdPrm.cbmPrm.msgId));
  memset(smsShrdPrm.cbmPrm.dcsId, DEF_DCS_RANGE, sizeof(smsShrdPrm.cbmPrm.dcsId));

  smsShrdPrm.pSetPrm[0]->vpAbs.year  [0] = 7;
  smsShrdPrm.pSetPrm[0]->vpAbs.year  [1] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.month [0] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.month [1] = 1;
  smsShrdPrm.pSetPrm[0]->vpAbs.day   [0] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.day   [1] = 1;
  smsShrdPrm.pSetPrm[0]->vpAbs.hour  [0] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.hour  [1] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.minute[0] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.minute[1] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.second[0] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.second[1] = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.tz_lsb    = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.tz_sign   = 0;
  smsShrdPrm.pSetPrm[0]->vpAbs.tz_msb    = 0;

  memset(&smsShrdPrm.pSetPrm[0]->vpEnh, 0, sizeof(smsShrdPrm.pSetPrm[0]->vpEnh));
  smsShrdPrm.pSetPrm[0]->vpEnh.tvpf        = SMS_TVPF_RELATIVE;
  smsShrdPrm.pSetPrm[0]->vpEnh.v_tp_vp_rel = 1;
  smsShrdPrm.pSetPrm[0]->vpEnh.tp_vp_rel   = TP_VP_RELATIVE_DEFAULT;

#ifdef SIM_TOOLKIT
  simShrdPrm.fuRef = -1;
  if (!psaSAT_FURegister (cmhSMS_FileUpdate))
  {
    TRACE_EVENT ("FAILED to register the handler cmhSMS_FileUpdate() for FU");
  }
#endif

#if ((defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_)) AND defined TI_PS_FF_CONC_SMS
  concSMS_Init();
#endif
}

/*==== EOF ========================================================*/
 

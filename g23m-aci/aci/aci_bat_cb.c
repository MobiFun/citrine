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
|  Purpose :  This BAT callback  modul is ...
| 
+----------------------------------------------------------------------------- 
*/ 
 
#include "aci_all.h"     /* includes prim.h, which includes p_bat.h */

#define  ACI_BAT_CB_GLOBALS /* define this file as owner of globals in aci_bat_cb.h */
#include "aci_bat_cb.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 

#include "psa_psi.h"
#include "p_bat.h"
#include "aci_bat_err.h"
#include "aci_bat.h"
#include "aci_cmh.h"
#include "psa.h"
#include "cmh.h"
#include "aci_lst.h"
#include "cmh_psi.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_fd.h"
#include "cmh_sms.h"
#include "cphs.h"

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_src_info     |
+--------------------------------------------------------------------+

  PURPOSE : Can be called from rBAT_ functions in order to find the
            command source ID and a pointer to the PSI source
            information.
*/
LOCAL void aci_bat_src_info (T_ACI_CMD_SRC *src_id, T_ACI_DTI_PRC_PSI **psi_src_info)
{
  /*
  *   The source ID is copied from the global variable srcId_cb, 
  *   which is set by the macro R_AT.
  */
  *src_id=(T_ACI_CMD_SRC)srcId_cb;

  /*
  *   Use the function 'find_element()' to search for the PSI
  *   source that has the correct command source ID.
  */
  *psi_src_info=find_element(psi_src_params,srcId_cb,cmhPSItest_srcId);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : aci_bat_lang_str     |
+--------------------------------------------------------------------+

  PURPOSE : Work out the string for the indicated language.
*/
LOCAL CHAR *aci_bat_lang_str (T_ACI_LAN_SUP *CLang)
{
  UBYTE i;

  if (CLang EQ NULL)
    return(NULL);

  /*
  *   If a string is provided, use that.
  */
  if (CLang->str)
    return(CLang->str);

  /*
  *   GSM 07.07 says that "AUTO" should be used for automatic mode.
  */  
  if (CLang->lng EQ CLAN_LNG_AUT)
    return("AUTO");

  /*
  *   Use the lookup table 'lngs' to find the string.
  */
  for (i=0;lngs[i].str;i++)
  {
    if (CLang->lng EQ lngs[i].lng)
      return(lngs[i].str);
  }

  return(NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_OK              |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_OK(
  T_ACI_AT_CMD cmd)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION ("rBAT_OK()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if (cmd EQ AT_CMD_D)
  {
    /*
    *   We are not waiting for a final response to ATD any more.
    */
    src_infos->bat_client[src_infos->active_client].atd_live=FALSE;
  }

  resp.ctrl_response=BAT_RES_AT_OK;

  resp.response.ptr_at_ok = &dummy;
  dummy.bat_dummy = 0xFF;
  
  if (src_infos->bat_client[src_infos->active_client].curCmd EQ (T_BAT_ctrl_params)BATC_ABORT_COMMAND_CNF)/*lint !e650 (Warning - constant out of range for operator) */
  {
    T_BATC_confirm confirm;
    T_BATC_abort_cmd abort_cmd;
    confirm.rsp_params  = BATC_ABORT_COMMAND_CNF;        
    abort_cmd.client_id = src_infos->active_client;  
    confirm.rsp.ptr_bat_abort_command_cnf = &abort_cmd;
    aci_bat_send(src_infos,(T_BAT_cmd_response *)&confirm);
    return;
  }

#ifdef _SIMULATION_
  src_infos->bat_client[src_infos->active_client].curCmd = (T_BAT_ctrl_params)-1;
#endif

  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_CONNECT         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_CONNECT(
  T_ACI_AT_CMD cmdId,
  T_ACI_BS_SPEED speed,
  SHORT cId,
  BOOL flow_cntr)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_res_at_connect connect_data;
  T_BAT_cmd_response resp;

  TRACE_FUNCTION ("rBAT_CONNECT()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
  *   We are not waiting for a final response to ATD any more.
  */
  src_infos->bat_client[src_infos->active_client].atd_live=FALSE;

  resp.ctrl_response=BAT_RES_AT_CONNECT;
  resp.response.ptr_at_connect=&connect_data;

  /*
  *   The ACI and BAT versions of 'speed' are quite a bit different,
  *   so we need this conversion. Note that we will be losing
  *   the "cmdId" and "cId" as these are not in the BAT.
  */
  switch(speed)
  {
    default:
    case BS_SPEED_NotPresent:
    case BS_SPEED_AUTO:
    case BS_SPEED_28800_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_NOT_PRESENT;
      break;

    case BS_SPEED_300_V21:
    case BS_SPEED_300_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_300;
      break;

    case BS_SPEED_1200_V22:
    case BS_SPEED_1200_75_V23:
    case BS_SPEED_1200_V120:
    case BS_SPEED_1200_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_1200;
      break;

    case BS_SPEED_2400_V22bis:
    case BS_SPEED_2400_V26ter:
    case BS_SPEED_2400_V120:
    case BS_SPEED_2400_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_2400;
      break;

    case BS_SPEED_4800_V32:
    case BS_SPEED_4800_V120:
    case BS_SPEED_4800_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_4800;
      break;

    case BS_SPEED_9600_V32:
    case BS_SPEED_9600_V34:
    case BS_SPEED_9600_V120:
    case BS_SPEED_9600_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_9600;
      break;

    case BS_SPEED_14400_V34:
    case BS_SPEED_14400_V120:
    case BS_SPEED_14400_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_14400;
      break;

    case BS_SPEED_19200_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_19200;
      break;

    case BS_SPEED_38400_V110:
      connect_data.data_rate=BAT_AT_CONNECT_RATE_38400;
      break;
  }

  aci_bat_send(src_infos,&resp);
  
  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCME         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCME(
  T_ACI_AT_CMD cmdId,
  T_ACI_CME_ERR err)
{
  T_BAT_cmd_response resp;
  T_BAT_res_plus_cme_error error_data;
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;

  TRACE_FUNCTION ("rBAT_PlusCME()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_PLUS_CME_ERROR;
  resp.response.ptr_plus_cme_error=&error_data;


  /* If the err is NotPresent or Unknown try to get the CME error 
     from aciErrDesc and display to the terminal */
  if ((err EQ CME_ERR_NotPresent) OR (err EQ CME_ERR_Unknown))
  {
    if (ACI_BAT_GET_ERR_CLASS() EQ ACI_ERR_CLASS_Cme)
    {
      err = (T_ACI_CME_ERR) ( ACI_BAT_GET_ERR_VALUE() );
      ACI_BAT_RESET_ERR_DESC();
    }
  }

#ifdef TI_PS_FF_BAT_CPHS_ERR_RES
 /* 
  * Valid CME Error for %CPHS if CPHS not supported by SIM
  * Customer specific. Put under custom flag TI_PS_FF_BAT_CPHS_ERR_RES
  */
  if (cmdId EQ AT_CMD_CPHS AND
      cphs_check_status() EQ CPHS_NOT_INIT)
  {
    err = CME_ERR_OpNotSupp;
  }
#endif /* TI_PS_FF_BAT_CPHS_ERR_RES */

  /*
  *   This relies on T_ACI_CME_ERR being the same as
  *   T_BAT_plus_cme_error_error, which (apart from a couple of
  *   omissions in each enumeration) it is.
  */
  resp.response.ptr_plus_cme_error->error=(T_BAT_plus_cme_error_error)err;

  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos); 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_NO_CARRIER      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_NO_CARRIER(
  T_ACI_AT_CMD cmdId,
  SHORT cId)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_no_parameter dummy;
  
  TRACE_FUNCTION ("rBAT_NO_CARRIER()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
   *   atd_live is TRUE, send final response else unsolicited response.
  */
  if(src_infos->bat_client[src_infos->active_client].atd_live EQ TRUE)
  {
    src_infos->bat_client[src_infos->active_client].atd_live=FALSE;
  resp.ctrl_response = BAT_RES_AT_NO_CARRIER_FINAL;
  resp.response.ptr_at_no_carrier_final = &dummy;
  }
  else
  {
    resp.ctrl_response = BAT_RES_UNS_AT_NO_CARRIER;
    resp.response.ptr_res_uns_at_no_carrier = &dummy;
  }

  dummy.bat_dummy = 0xFF;
  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCRING       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCRING(
  T_ACI_CRING_MOD mode,
  T_ACI_CRING_SERV_TYP type1,
  T_ACI_CRING_SERV_TYP type2)
{
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cring cring_data;
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;

  TRACE_FUNCTION ("rBAT_PlusCRING()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CRING;
  resp.response.ptr_plus_cring=&cring_data;

  /*
  *   This relies on T_ACI_CRING_SERV_TYP being the same as
  *   T_BAT_cring_type/T_BAT_plus_cring_type2.
  */
  cring_data.type1=(T_BAT_cring_type)type1;
  cring_data.type2=(T_BAT_plus_cring_type2)type2;

  /*
  *   This relies on T_ACI_CRING_MOD being the same as
  *   T_BAT_plus_cring_alt.
  */
  cring_data.alt=(T_BAT_plus_cring_alt)mode;

  /*
  *   Mark as not present all the information in the BAT message
  *   that we don't have.
  */
  cring_data.v_pdp_addr=FALSE;
  cring_data.v_l2p=FALSE;
  cring_data.pdp_type=BAT_PDP_TYPE_NOT_PRESENT;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusDR          |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusDR(
  T_ACI_DR_TYP type)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusDR() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusDR() - FAKE");
  rCI_PlusDR(type);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCR          |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCR(
  T_ACI_CRING_SERV_TYP service)
{
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cr cr_data;
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  CHAR *s;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCR()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CR;
  resp.response.ptr_res_plus_cr=&cr_data;

  switch(service)
  {
    case CRING_SERV_TYP_Async:
      s="ASYNC";
      break;

    case CRING_SERV_TYP_Sync:
      s="SYNC";
      break;
      
    case CRING_SERV_TYP_RelAsync:
      s="REL ASYNC";
      break;

    case CRING_SERV_TYP_RelSync:
      s="REL SYNC";
      break;

#ifdef GPRS
    case CRING_SERV_TYP_GPRS:
      s="GPRS";
      break;
#endif

    default:
      /*
      *   Unrecognised value, get out now.
      */
      return;    
  }

  len=strlen(s);

  /*
  *   Check that we have enough room in the BAT structure, if we
  *   haven't we may as well give up at this point.
  */
  if (len>BAT_MAX_CR_SERV_LEN)
    return;

  /*
  *   Copy the data into the BAT structure and set the counter.
  */
  memcpy(cr_data.serv,s,BAT_MAX_CR_SERV_LEN);
  cr_data.c_serv=(U8)len;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCPIN        |
+--------------------------------------------------------------------+

  PURPOSE : Response to qBAT_PlusCPIN(). Also see simactivated_cpinresult().
*/
GLOBAL void rBAT_PlusCPIN(
  T_ACI_CPIN_RSLT code)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cpin cpin_data;
 
  TRACE_FUNCTION ("rBAT_PlusCPIN()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_QUE_PLUS_CPIN;
  resp.response.ptr_que_plus_cpin=&cpin_data;

  /*
  *   Warning - this assumes that T_ACI_CPIN_RSLT and
  *   T_BAT_VAL_plus_cpin_code are eqivalent. They are, except that
  *   T_ACI_CPIN_RSLT has extra values not in the BAT.
  */
  resp.response.ptr_que_plus_cpin->code=(T_BAT_VAL_plus_cpin_code)code; 

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCOPS        |
+--------------------------------------------------------------------+

  PURPOSE : See cmhMM_NetworkLst() for how this is used.
*/
GLOBAL void rBAT_PlusCOPS(
  SHORT lastIdx,
  T_ACI_COPS_OPDESC *operLst)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_cops cops_data;
  USHORT n;

  TRACE_FUNCTION ("rBAT_PlusCOPS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_TST_PLUS_COPS;
  resp.response.ptr_tst_plus_cops=&cops_data;

  /*
  *   This has delivered a list, however the BAT response only
  *   allows for single items. So we must send each one 
  *   individually. The use of null values of longOper and
  *   shortOper as list terminators is as in rAT_PlusCOPS().
  */
  n=0;

  while ((operLst[n].longOper) AND (operLst[n].shortOper))
  {
    /*
    *   This assumes that T_BAT_VAL_plus_cops_status and T_ACI_COPS_STAT
    *   are identical (currently they are).
    */
    cops_data.status=(T_BAT_plus_cops_status)operLst[n].status;

    /*
    *   Need to check this. Are all the operator names necessarily
    *   present?
    */
    strncpy((CHAR *)cops_data.long_oper,operLst[n].longOper,BAT_MAX_COPS_LONG_OPER_LEN);
    cops_data.v_long_oper=TRUE;
    cops_data.c_long_oper=strlen((CHAR*)cops_data.long_oper);

    strncpy((CHAR *)cops_data.short_oper,operLst[n].shortOper,BAT_MAX_COPS_SHORT_OPER_LEN);
    cops_data.v_short_oper=TRUE;
    cops_data.c_short_oper=strlen((CHAR*)cops_data.short_oper);

    strncpy((CHAR *)cops_data.num_oper,operLst[n].numOper,BAT_MAX_COPS_NUM_OPER_LEN);
    cops_data.v_num_oper=TRUE;
    cops_data.c_num_oper=strlen((CHAR*)cops_data.num_oper);

    aci_bat_send(src_infos,&resp);
    
    n++;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCOPS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCOPS(
  SHORT lastIdx,
  T_ACI_COPS_OPDESC *operLst)
{

#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentCOPS() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentCOPS() - FAKE");
  rCI_PercentCOPS(lastIdx,operLst);

#endif

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCREG        |
+--------------------------------------------------------------------+

  PURPOSE : Notification of registration status. There is no +CREG in
            BAT, so we use %CREG instead.
*/
GLOBAL void rBAT_PlusCREG(
  T_ACI_CREG_STAT status,
  USHORT lac,
  USHORT cid)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_creg creg_data;

  TRACE_FUNCTION ("rBAT_PlusCREG()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CREG;
  resp.response.ptr_res_percent_creg=&creg_data;

  /*
  *   This relies on T_ACI_CREG_STAT and T_BAT_percent_creg_stat being
  *   identical.
  */
  creg_data.stat=(T_BAT_percent_creg_stat)status;

  creg_data.lac=(S32)lac;
  creg_data.ci=(S32)cid;
  creg_data.gprs_ind=BAT_P_CREG_GPRS_IND_UNKNOWN;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCCWA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCCWA(
  T_ACI_CLSSTAT *clsStat,
  CHAR *number,
  T_ACI_TOA *type,
  U8 validity,
  T_ACI_CLASS class_type,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha)
#else
  CHAR *alpha)
#endif
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_ccwa ccwa_data;
  T_BAT_res_set_plus_ccwa ccwa_q_data;
  T_ACI_CLASS test_class;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCCWA()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if(clsStat)          /*Callback for sBAT_PlusCCWA guery (mode 2)*/
  {
    resp.ctrl_response = BAT_RES_SET_PLUS_CCWA;
    resp.response.ptr_set_plus_ccwa = &ccwa_q_data;
    
    if (clsStat->status EQ STATUS_Active)
    {
      //src_params->curAtCmd = AT_CMD_NONE;
      test_class = CLASS_Vce;
      while( test_class < 2*CLASS_Fax )
      {
        if( clsStat->class_type & test_class )
        {
          TRACE_EVENT_P1("test_class: %d", test_class);
          ccwa_q_data.status = (T_BAT_plus_ccwa_status)clsStat->status; 
          ccwa_q_data.bearer_class = (T_BAT_plus_ccwa_bearer_class)test_class; 

          aci_bat_send(src_infos,&resp);          
        }
        test_class *= CLASS_Dat; /* test next class */
      }
    }
    else
    {

      ccwa_q_data.status = (T_BAT_plus_ccwa_status)clsStat->status; 
      ccwa_q_data.bearer_class = BAT_CCWA_CLASS_NOT_PRESENT; 

      aci_bat_send(src_infos,&resp);          
    }
    
  }
  else 
  {
  resp.ctrl_response=BAT_RES_UNS_PLUS_CCWA;
  resp.response.ptr_res_plus_ccwa=&ccwa_data;

  if (number EQ NULL)
  {
    ccwa_data.number[0]=0;
    ccwa_data.c_number=0;
  }
  else
  {
    len=strlen(number);

    if (len<=BAT_MAX_CCWA_NUMBER_LEN)
    {
      memcpy(ccwa_data.number,number,len);
      ccwa_data.c_number=(U8)len;
    }
    else
    {
      /*
      *   This is highly unlikely to happen, but if the number is too
      *   big, then put as many digits as will fit in the BAT structure
      *   from the END of the number.
      */
      memcpy(
        ccwa_data.number,
        number+(len-BAT_MAX_CCWA_NUMBER_LEN),
        BAT_MAX_CCWA_NUMBER_LEN);

      ccwa_data.c_number=(U8)BAT_MAX_CCWA_NUMBER_LEN;
    }
  }

  if (type EQ NULL)
  {
    /*
    *   This is mandatory in the BAT so we have to do something here.
    */
    T_ACI_TOA t;

    t.npi=NPI_Unknown;
    t.ton=TON_Unknown;

    ccwa_data.type=(U8)toa_merge(t);
  }
  else
  {
    ccwa_data.type=(U8)toa_merge(*type);
  }

  /*
  *   This relies on T_ACI_CLASS being equivalent to
  *   T_BAT_plus_ccwa_bearer_class.
  */
      ccwa_data.bearer_class=(T_BAT_plus_ccwa_bearer_class)class_type;
  
  /*
  *   Alphanumeric data
  */
  if (alpha NEQ NULL)
  {
    USHORT len=0;

    ccwa_data.v_alpha=TRUE;

#ifdef NO_ASCIIZ

      utl_chsetFromSim(
      alpha->data,
      alpha->len,
      ccwa_data.alpha,
      BAT_MAX_PHB_NUM_LEN,
      &len,
      GSM_ALPHA_Def);

#else

    utl_chsetFromSim(
      alpha,
      strlen(alpha),
      ccwa_data.alpha,
      BAT_MAX_PHB_NUM_LEN,
      &len,
      GSM_ALPHA_Int);

#endif

    ccwa_data.c_alpha=(U8)len;
  }
  else
  {
    ccwa_data.v_alpha=FALSE;
  }

  ccwa_data.cli=(T_BAT_cli)validity;

  aci_bat_send(src_infos,&resp);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCLIP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCLIP(
  T_ACI_CLIP_STAT stat,
  CHAR *number,
  T_ACI_TOA * type,
  U8 validity,
  CHAR *subaddr,
  T_ACI_TOS *satype,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha)
#else
  CHAR *alpha)
#endif /*NO_ASCIIZ*/
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_clip clip_data;
  T_BAT_res_que_plus_clip que_clip_buffer;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCLIP()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if( stat EQ CLIP_STAT_NotPresent )
  {
    resp.response.ptr_res_plus_clip=&clip_data;
    resp.ctrl_response=BAT_RES_UNS_PLUS_CLIP;
    if (number EQ NULL)
    {
      clip_data.number[0]=0;
      clip_data.c_number=0;
    }
    else
    {
      len=strlen(number);

      if (len<=BAT_MAX_CLIP_NUMBER_LEN)
      {
        memcpy(clip_data.number,number,len);
        clip_data.c_number=(U8)len;
      }
      else
      {
        /*
        *   This is highly unlikely to happen, but if the number is too
        *   big, then put as many digits as will fit in the BAT structure
        *   from the END of the number.
        */
        memcpy(
          clip_data.number,
          number+(len-BAT_MAX_CLIP_NUMBER_LEN),
          BAT_MAX_CLIP_NUMBER_LEN);

        clip_data.c_number=(U8)BAT_MAX_CLIP_NUMBER_LEN;
      }
    }

    if (type EQ NULL)
    {
      /*
      *   This is mandatory in the BAT so we have to do something here.
      */
      T_ACI_TOA t;

      t.npi=NPI_Unknown;
      t.ton=TON_Unknown;

      clip_data.type=(U8)toa_merge(t);
    }
    else
    {
      clip_data.type=(U8)toa_merge(*type);
    }

    if (subaddr EQ NULL)
    {
      clip_data.v_subaddr=FALSE;
      clip_data.satype=-1;
    }
    else
    {
      clip_data.v_subaddr=TRUE;

      if (satype EQ NULL)
        clip_data.satype=-1;
      else
        clip_data.satype=(S16)tos_merge(*satype);

      len=strlen(subaddr);

      /*
      *   Easiest thing to do is copy as much subaddress data as the
      *   BAT structure can take. The length field will ensure that
      *   there are no problems, and will result in the subaddress
      *   being truncated if it is too long.
      */
      memcpy(clip_data.subaddr,subaddr,BAT_MAX_SUBADDR_LENGTH);
      
      if (len>BAT_MAX_SUBADDR_LENGTH)
        clip_data.c_subaddr=BAT_MAX_SUBADDR_LENGTH;
      else
        clip_data.c_subaddr=len;
    }

    clip_data.cli=(T_BAT_cli)validity;

    /*
    *   Alphanumeric data
    */
    if (alpha NEQ NULL)
    {
      USHORT len;

      clip_data.v_alpha=TRUE;
      
#ifdef NO_ASCIIZ

      utl_chsetFromSim(
        alpha->data,
        alpha->len,
        (UBYTE *)clip_data.alpha,
        BAT_MAX_PHB_NUM_LEN,
        &len,
        GSM_ALPHA_Def);

#else

      utl_chsetFromSim(
        alpha,
        strlen(alpha),
        (UBYTE *)clip_data.alpha,
        BAT_MAX_PHB_NUM_LEN,
        &len,
        GSM_ALPHA_Def);

#endif /*NO_ASCIIZ*/

      clip_data.c_alpha=(U8)len;
    }
    else
    {
      clip_data.v_alpha=FALSE;
    }
  }
  else
  {
    resp.ctrl_response=BAT_RES_QUE_PLUS_CLIP;
    resp.response.ptr_que_plus_clip=&que_clip_buffer;
    que_clip_buffer.m = (T_BAT_plus_clip_m)stat;
  }
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCDIP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCDIP(CHAR *number,
                          T_ACI_TOA * type,
                          CHAR *subaddr,
                          T_ACI_TOS *satype)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cdip cdip_data;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCDIP()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CDIP;
  resp.response.ptr_res_plus_cdip=&cdip_data;

  if (number EQ NULL)
  {
    cdip_data.number[0]=0;
    cdip_data.c_number=0;
  }
  else
  {
    len=strlen(number);

    if (len<=BAT_MAX_CDIP_NUMBER_LEN)
    {
      memcpy(cdip_data.number,number,len);
      cdip_data.c_number=(U8)len;
    }
    else
    {
      /*
      *   This is highly unlikely to happen, but if the number is too
      *   big, then put as many digits as will fit in the BAT structure
      *   from the END of the number.
      */
      memcpy(
        cdip_data.number,
        number+(len-BAT_MAX_CDIP_NUMBER_LEN),
        BAT_MAX_CDIP_NUMBER_LEN);

      cdip_data.c_number=(U8)BAT_MAX_CDIP_NUMBER_LEN;
    }
  }

  if (type EQ NULL)
  {
    /*
    *   This is mandatory in the BAT so we have to do something here.
    */
    T_ACI_TOA t;

    t.npi=NPI_Unknown;
    t.ton=TON_Unknown;

    cdip_data.type=(U8)toa_merge(t);
  }
  else
  {
    cdip_data.type=(U8)toa_merge(*type);
  }

  if (subaddr EQ NULL)
  {
    cdip_data.v_subaddr=FALSE;
    cdip_data.satype=-1;
  }
  else
  {
    cdip_data.v_subaddr=TRUE;

    if (satype EQ NULL)
      cdip_data.satype=-1;
    else
      cdip_data.satype=(S16)tos_merge(*satype);

    len=strlen(subaddr);

    /*
    *   Easiest thing to do is copy as much subaddress data as the
    *   BAT structure can take. The length field will ensure that
    *   there are no problems, and will result in the subaddress
    *   being truncated if it is too long.
    */
    memcpy(cdip_data.subaddr,subaddr,BAT_MAX_SUBADDR_LENGTH);

    if (len>BAT_MAX_SUBADDR_LENGTH)
      cdip_data.c_subaddr=BAT_MAX_SUBADDR_LENGTH;
    else
      cdip_data.c_subaddr=len;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCOLP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCOLP(
  T_ACI_COLP_STAT stat,
  CHAR *number,
  T_ACI_TOA *type,
  CHAR *subaddr,
  T_ACI_TOS *satype,
#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT *alpha)
#else
  CHAR *alpha)
#endif /*NO_ASCIIZ*/
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_colp colp_data;
  T_BAT_res_que_plus_colp que_colp_buffer;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCOLP()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if (stat EQ COLP_STAT_NotPresent) /* Function call if connection is made*/
  {
    resp.ctrl_response=BAT_RES_UNS_PLUS_COLP;
    resp.response.ptr_res_plus_colp=&colp_data;

    if (number EQ NULL)
    {
      colp_data.v_number=FALSE;
    }
    else
    {
      len=strlen(number);

      colp_data.v_number=TRUE;

      if (len<=BAT_MAX_COLP_NUMBER_LEN)
      {
        memcpy(colp_data.number,number,len);
        colp_data.c_number=(U8)len;
      }
      else
      {
      /*
      *   This is highly unlikely to happen, but if the number is too
      *   big, then put as many digits as will fit in the BAT structure
      *   from the END of the number.
        */
        memcpy(
          colp_data.number,
          number+(len-BAT_MAX_COLP_NUMBER_LEN),
          BAT_MAX_COLP_NUMBER_LEN);

        colp_data.c_number=(U8)BAT_MAX_COLP_NUMBER_LEN;
      }
    }

    if (type EQ NULL)
    {
    /*
    *   This is mandatory in the BAT so we have to do something here.
      */
      T_ACI_TOA t;

      t.npi=NPI_Unknown;
      t.ton=TON_Unknown;

      colp_data.type=(U8)toa_merge(t);
    }
    else
    {
      colp_data.type=(U8)toa_merge(*type);
    }

    if (subaddr EQ NULL)
    {
      colp_data.v_subaddr=FALSE;
      colp_data.satype=-1;
    }
    else
    {
      colp_data.v_subaddr=TRUE;

      if (satype EQ NULL)
        colp_data.satype=-1;
      else
        colp_data.satype=(S16)tos_merge(*satype);

      len=strlen(subaddr);

      /*
      *   Easiest thing to do is copy as much subaddress data as the
      *   BAT structure can take. The length field will ensure that
      *   there are no problems, and will result in the subaddress
      *   being truncated if it is too long.
      */
      memcpy(colp_data.subaddr,subaddr,BAT_MAX_SUBADDR_LENGTH);

      if (len>BAT_MAX_SUBADDR_LENGTH)
        colp_data.c_subaddr=BAT_MAX_SUBADDR_LENGTH;
      else
        colp_data.c_subaddr=len;
    }

    /*
    *   Alphanumeric data
    */
    if (alpha!=NULL)
    {
      USHORT len;

      colp_data.v_alpha=TRUE;
      
#ifdef NO_ASCIIZ

      utl_chsetFromSim(
        alpha->data,
        alpha->len,
        (UBYTE *)colp_data.alpha,
        BAT_MAX_COLP_ALPHA_LEN,
        &len,
        GSM_ALPHA_Def);

#else

      utl_chsetFromSim(
        alpha,
        strlen(alpha),
        (UBYTE *)colp_data.alpha,
        BAT_MAX_COLP_ALPHA_LEN,
        &len,
        GSM_ALPHA_Def);

#endif /*NO_ASCIIZ*/

      colp_data.c_alpha=(U8)len;
    }
    else
    {
      colp_data.v_alpha=FALSE;
    }
  }
  else
  {
    que_colp_buffer.m = (T_BAT_plus_colp_m)stat;
    resp.ctrl_response=BAT_RES_QUE_PLUS_COLP;
    resp.response.ptr_que_plus_colp=&que_colp_buffer;
  }
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCRING_OFF   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCRING_OFF(
    SHORT cId)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCRING_OFF() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCRING_OFF() - FAKE");
  rCI_PlusCRING_OFF(cId);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMS         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMS(
  T_ACI_AT_CMD cmdId,
  T_ACI_CMS_ERR err,
  T_EXT_CMS_ERROR *conc_error)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_plus_cms_error error_data;

  TRACE_FUNCTION ("rBAT_PlusCMS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_PLUS_CMS_ERROR;
  resp.response.ptr_plus_cms_error=&error_data;

  /*
  *   This relies on T_ACI_CMS_ERR being the same as
  *   T_BAT_plus_cms_error_err.
  */
  resp.response.ptr_plus_cms_error->err=(T_BAT_plus_cms_error_err)err;

  /*
  *   BAT does not include cmdId or conc_error, so these are
  *   discarded.
  */

  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos); 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCSMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCSMS(
  T_ACI_CSMS_SERV service,
  T_ACI_CSMS_SUPP mt,
  T_ACI_CSMS_SUPP mo,
  T_ACI_CSMS_SUPP bm)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_csms csms_data;

  TRACE_FUNCTION ("rBAT_PlusCSMS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_QUE_PLUS_CSMS;
  resp.response.ptr_que_plus_csms=&csms_data;

  /*
  *   This relies on T_ACI_CSMS_SERV being equivalent to
  *   T_BAT_plus_csms_service.
  */
  csms_data.service=(T_BAT_plus_csms_service)service;

  /*
  *   This relies on T_ACI_CSMS_SUPP being equivalent to
  *   T_BAT_plus_csms_mt/mo/bm.
  */
  csms_data.mt=(T_BAT_plus_csms_mt)mt;
  csms_data.mo=(T_BAT_plus_csms_mo)mo;
  csms_data.bm=(T_BAT_plus_csms_bm)bm;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGS(
  T_MNSMS_SUBMIT_CNF *mnsms_submit_cnf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmgs cmgs_data;

  TRACE_FUNCTION ("rBAT_PlusCMGS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMGS;
  resp.response.ptr_set_plus_cmgs=&cmgs_data;

  /*
  *   Note that all the rest of the information is lost, as it is
  *   not in the BAT.
  */
  cmgs_data.mr=(U8)mnsms_submit_cnf->tp_mr;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMSS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMSS(
  T_MNSMS_SUBMIT_CNF *mnsms_submit_cnf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmss cmss_data;
  UBYTE len = 0;
  UBYTE sca_len = 0;
  UBYTE ackpdu_len = 0;

  TRACE_FUNCTION ("rBAT_PlusCMSS()");

  if (mnsms_submit_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_submit_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMSS;
  resp.response.ptr_set_plus_cmss=&cmss_data;


  cmss_data.mr=mnsms_submit_cnf->tp_mr;

  cmss_data.c_ackpdu = 0;

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_submit_cnf->sms_sdu.l_buf)
    {
      sca_len = mnsms_submit_cnf->sms_sdu.buf[0] + 1;
      ackpdu_len = len - sca_len;
      /*
      *   Now copy the PDU into the BAT structure without SCA field.
      */
      memcpy(cmss_data.ackpdu,&mnsms_submit_cnf->sms_sdu.buf[sca_len],ackpdu_len);
      cmss_data.c_ackpdu = ackpdu_len;
    }
  }
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGW        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGW(
  UBYTE index,
  UBYTE numSeg)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmgw cmgw_data;

  TRACE_FUNCTION ("rBAT_PlusCMGW()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMGW;
  resp.response.ptr_set_plus_cmgw=&cmgw_data;

  cmgw_data.index=(U8)index;

  /*
  *   numSeg is lost as it is not in the BAT.
  */

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCDS         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCDS(
  T_MNSMS_STATUS_IND *mnsms_status_ind)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cds cds_data;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCDS()");

  if (mnsms_status_ind EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_status_ind->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CDS;
  resp.response.ptr_res_plus_cds=&cds_data;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cds_data.pdu,mnsms_status_ind->sms_sdu.buf,len);
  cds_data.c_pdu=(U8)len;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGC(
  T_MNSMS_COMMAND_CNF *mnsms_command_cnf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmgc cmgc_data;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCMGC()");

  if (mnsms_command_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_command_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMGC;
  resp.response.ptr_set_plus_cmgc=&cmgc_data;

  cmgc_data.mr=mnsms_command_cnf->tp_mr;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmgc_data.ackpdu,mnsms_command_cnf->sms_sdu.buf,len);
  cmgc_data.c_ackpdu=(U8)len;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGD(void)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCMGD() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCMGD() - FAKE");
  rCI_PlusCMGD();

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGR(
  T_MNSMS_READ_CNF *mnsms_read_cnf,
  T_ACI_CMGR_CBM *cbm)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmgr cmgr_data;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT stat;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCMGR()");

  if (mnsms_read_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMGR;
  resp.response.ptr_set_plus_cmgr=&cmgr_data;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmgr_data.pdu,mnsms_read_cnf->sms_sdu.buf,len);
  cmgr_data.c_pdu=len;

  /*
  *   Convert status from PSA type to CMH type.
  */
  cmhSMS_getStatCmh(mnsms_read_cnf->status,&stat);

  /*
  *   This relies on T_ACI_SMS_STAT and T_BAT_sms_stat being equivalent
  *   (apart from the extra 'invalid' value that the ACI has).
  */
  if (stat EQ SMS_STAT_Invalid)
    cmgr_data.sms_stat=BAT_STAT_NOT_PRESENT;
  else
    cmgr_data.sms_stat=(T_BAT_sms_stat)stat;

  /*
  *   Try and get the alphanumeric (phonebook) data.
  */
  cmhSMS_getPhbEntry(mnsms_read_cnf->sms_sdu.buf,&alpha,stat);

  if (alpha.len)
  {
    USHORT len_cvtd_alpha;

    utl_chsetFromGsm(
      (UBYTE*)alpha.data,
      alpha.len,
      (UBYTE*)cmgr_data.alpha,
      BAT_MAX_CMGR_ALPHA,
      &len_cvtd_alpha,
      GSM_ALPHA_Def);

    cmgr_data.c_alpha=(U8)len_cvtd_alpha;
    cmgr_data.v_alpha=TRUE;

    cmgr_data.alpha_cs=(T_BAT_plus_cmgr_alpha_cs)aci_bat_cs_get(src_infos);
  }
  else
  {
    cmgr_data.v_alpha=FALSE;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCMGR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCMGR(
  T_MNSMS_READ_CNF *mnsms_read_cnf,
  T_ACI_CMGR_CBM *cbm)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_percent_cmgr cmgr_data;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT stat;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PercentCMGR()");

  if (mnsms_read_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PERCENT_CMGR;
  resp.response.ptr_set_percent_cmgr=&cmgr_data;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmgr_data.pdu,mnsms_read_cnf->sms_sdu.buf,len);
  cmgr_data.c_pdu=len;

  /*
  *   Convert status from PSA type to CMH type.
  */
  cmhSMS_getStatCmh(mnsms_read_cnf->status,&stat);

  /*
  *   This relies on T_ACI_SMS_STAT and T_BAT_sms_stat being equivalent
  *   (apart from the extra 'invalid' value that the ACI has).
  */
  if (stat EQ SMS_STAT_Invalid)
    cmgr_data.sms_stat=BAT_STAT_NOT_PRESENT;
  else
    cmgr_data.sms_stat=(T_BAT_sms_stat)stat;

  /*
  *   Try and get the alphanumeric (phonebook) data.
  */
  cmhSMS_getPhbEntry(mnsms_read_cnf->sms_sdu.buf,&alpha,stat);

  if (alpha.len)
  {
    USHORT len_cvtd_alpha;

    utl_chsetFromGsm(
      (UBYTE*)alpha.data,
      alpha.len,
      (UBYTE*)cmgr_data.alpha,
      BAT_MAX_CMGR_ALPHA,
      &len_cvtd_alpha,
      GSM_ALPHA_Def);

    cmgr_data.c_alpha=(U8)len_cvtd_alpha;
    cmgr_data.v_alpha=TRUE;

    cmgr_data.alpha_cs=(T_BAT_percent_cmgr_alpha_cs)aci_bat_cs_get(src_infos);
  }
  else
  {
    cmgr_data.v_alpha=FALSE;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMGL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMGL(
  T_MNSMS_READ_CNF *mnsms_read_cnf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cmgl cmgl_data;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT stat;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCMGL()");

  if (mnsms_read_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CMGL;
  resp.response.ptr_set_plus_cmgl=&cmgl_data;

  cmgl_data.sms_index=(U16)mnsms_read_cnf->rec_num;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmgl_data.pdu,mnsms_read_cnf->sms_sdu.buf,len);
  cmgl_data.c_pdu=len;

  /*
  *   Try and get the alphanumeric (phonebook) data.
  */
  cmhSMS_getStatCmh(mnsms_read_cnf->status,&stat);
  cmhSMS_getPhbEntry(mnsms_read_cnf->sms_sdu.buf,&alpha,stat);

  if (alpha.len)
  {
    USHORT len_cvtd_alpha;

    utl_chsetFromGsm(
      (UBYTE*)alpha.data,
      alpha.len,
      (UBYTE*)cmgl_data.alpha,
      BAT_MAX_CMGL_ALPHA,
      &len_cvtd_alpha,
      GSM_ALPHA_Def);

    cmgl_data.c_alpha=(U8)len_cvtd_alpha;
    cmgl_data.v_alpha=TRUE;

    cmgl_data.alpha_cs=(T_BAT_plus_cmgl_alpha_cs)aci_bat_cs_get(src_infos);
  }
  else
  {
    cmgl_data.v_alpha=FALSE;
  }

  /*
  *   This relies on T_ACI_SMS_STAT and T_BAT_plus_cmgl_stat being equivalent
  *   (apart from the extra 'invalid' value that the ACI has).
  */
  if (stat EQ SMS_STAT_Invalid)
    cmgl_data.stat=BAT_CMGL_STAT_NOT_PRESENT;
  else
    cmgl_data.stat=(T_BAT_plus_cmgl_stat)stat;

  aci_bat_send(src_infos,&resp);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCMGL     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCMGL(
  T_MNSMS_READ_CNF *mnsms_read_cnf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_percent_cmgl cmgl_data;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT stat;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PercentCMGL()");

  if (mnsms_read_cnf EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PERCENT_CMGL;
  resp.response.ptr_set_percent_cmgl=&cmgl_data;

  cmgl_data.sms_index=(U16)mnsms_read_cnf->rec_num;

  /*
  *   Now copy the PDU into the BAT structure.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmgl_data.pdu,mnsms_read_cnf->sms_sdu.buf,len);
  cmgl_data.c_pdu=len;

  /*
  *   Try and get the alphanumeric (phonebook) data.
  */
  cmhSMS_getStatCmh(mnsms_read_cnf->status,&stat);
  cmhSMS_getPhbEntry(mnsms_read_cnf->sms_sdu.buf,&alpha,stat);

  if (alpha.len)
  {
    USHORT len_cvtd_alpha;

    utl_chsetFromGsm(
      (UBYTE*)alpha.data,
      alpha.len,
      (UBYTE*)cmgl_data.alpha,
      BAT_MAX_CMGL_ALPHA,
      &len_cvtd_alpha,
      GSM_ALPHA_Def);

    cmgl_data.c_alpha=(U8)len_cvtd_alpha;
    cmgl_data.v_alpha=TRUE;

    cmgl_data.alpha_cs=(T_BAT_percent_cmgl_alpha_cs)aci_bat_cs_get(src_infos);
  }
  else
  {
    cmgl_data.v_alpha=FALSE;
  }

  /*
  *   This relies on T_ACI_SMS_STAT and T_BAT_plus_cmgl_stat being equivalent
  *   (apart from the extra 'invalid' value that the ACI has).
  */
  if (stat EQ SMS_STAT_Invalid)
    cmgl_data.stat=BAT_P_CMGL_STAT_NOT_PRESENT;
  else
    cmgl_data.stat=(T_BAT_percent_cmgl_stat)stat;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMTI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCMTI(
  T_ACI_SMS_STOR mem,
  UBYTE index)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cmti cmti_data;

  TRACE_FUNCTION ("rBAT_PlusCMTI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CMTI;
  resp.response.ptr_res_plus_cmti=&cmti_data;

  /*
  *   This relies on T_ACI_SMS_STOR being equivalent to T_BAT_sms_mem.
  */
  cmti_data.sms_mem=(T_BAT_sms_mem)mem;

  cmti_data.sms_index=(U16)index;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCMT         |
+--------------------------------------------------------------------+

  PURPOSE : Report an incoming mobile terminated message.
*/
GLOBAL void rBAT_PlusCMT(
  T_MNSMS_MESSAGE_IND *msg)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cmt cmt_data;
  UBYTE len;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT stat;
  
  TRACE_FUNCTION ("rBAT_PlusCMT()");

  /*
  *   This shouldn't happen, but check anyway.
  */
  if (msg EQ NULL)
    return;

  /*
  *   Calculate the PDU length in bytes from the number in bits.
  */
  len=((msg->sms_sdu.l_buf+7)/8);

  /*
  *   The PDU data is mandatory, and a truncated version would be
  *   meaningless, so if it won't fit in the BAT message we should
  *   give up. This shouldn't happen anyway.
  */
  if (len>BAT_MAX_SM_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CMT;
  resp.response.ptr_res_plus_cmt=&cmt_data;

  /*
  *   Now copy the PDU data into the BAT structure, secure in the
  *   knowledge that we have enough room due to the check earlier.
  */
  /*lint -e670 (Warning - Possible access beyond array for function) */
  memcpy(cmt_data.pdu,&msg->sms_sdu.buf,len);
  cmt_data.c_pdu=(U8)len;

  /*
  *   Try and get the alphanumeric (phonebook) data.
  */
  cmhSMS_getStatCmh(msg->status,&stat);
  cmhSMS_getPhbEntry(msg->sms_sdu.buf,&alpha,stat);

  if (alpha.len)
  {
    USHORT len_cvtd_alpha;

    /*
    *   This relies on T_ACI_CSCS_CHSET being identical to
    *   T_BAT_plus_cscs_cs.
    */
    utl_chsetFromGsm(
      (UBYTE*)alpha.data,
      alpha.len,
      (UBYTE*)cmt_data.alpha,
      BAT_MAX_CMT_ALPHA,
      &len_cvtd_alpha,
      GSM_ALPHA_Def);

    cmt_data.c_alpha=(U8)len_cvtd_alpha;
    cmt_data.v_alpha=TRUE;

    cmt_data.alpha_cs=(T_BAT_plus_cmt_alpha_cs)aci_bat_cs_get(src_infos);
  }
  else
  {
    cmt_data.v_alpha=FALSE;
  }

  aci_bat_send(src_infos,&resp);
}
   
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCBM         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCBM(
  T_MMI_CBCH_IND *mmi_cbch_ind)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cbm cbm_data;

  TRACE_FUNCTION ("rBAT_PlusCBM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CBM;
  resp.response.ptr_res_plus_cbm=&cbm_data;

  /*
  *   If the data received is too long for the BAT structure it will
  *   effectively be truncated.
  */
  if (mmi_cbch_ind->cbch_len>BAT_MAX_SM_LEN)
    cbm_data.c_pdu=BAT_MAX_SM_LEN;
  else
    cbm_data.c_pdu=(U8)mmi_cbch_ind->cbch_len;

  memcpy(cbm_data.pdu,mmi_cbch_ind->cbch_msg,sizeof(mmi_cbch_ind->cbch_msg));

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCPMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCPMS(
  T_ACI_SMS_STOR_OCC *mem1,
  T_ACI_SMS_STOR_OCC *mem2,
  T_ACI_SMS_STOR_OCC *mem3)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cpms cpms_data;

  TRACE_FUNCTION ("rBAT_PlusCPMS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CPMS;
  resp.response.ptr_set_plus_cpms=&cpms_data;

  if (mem1 EQ NULL)
  {
    /*
    *   If this happens we would be required to populate mandatory
    *   parameters without the information to do so. Give up.
    */
    return;
  }

  cpms_data.mem1=(T_BAT_plus_cpms_mem1)mem1->mem;
  cpms_data.used1=(U8)mem1->used;
  cpms_data.total1=(U8)mem1->total;

  if (mem2)
  {
    cpms_data.mem2=(T_BAT_plus_cpms_mem2)mem2->mem;
    cpms_data.used2=(U8)mem2->used;
    cpms_data.total2=(U8)mem2->total;
  }
  else
  {
    cpms_data.mem2=BAT_CPMS_MEM2_NOT_PRESENT;
    cpms_data.used2=0;
    cpms_data.total2=0;
  }

  if (mem3)
  {
    cpms_data.mem3=(T_BAT_plus_cpms_mem3)mem3->mem;
    cpms_data.used3=(U8)mem3->used;
    cpms_data.total3=(U8)mem3->total;
  }
  else
  {
    cpms_data.mem3=BAT_CPMS_MEM3_NOT_PRESENT;
    cpms_data.used3=0;
    cpms_data.total3=0;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusILRR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusILRR(
  T_ACI_BS_SPEED speed,
  T_ACI_BS_FRM format,
  T_ACI_BS_PAR parity)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_ilrr ilrr_data;

  TRACE_FUNCTION ("rBAT_PlusILRR()");

  resp.ctrl_response=BAT_RES_UNS_PLUS_ILRR;
  resp.response.ptr_res_plus_ilrr=&ilrr_data;

  /*
  *   T_ACI_BS_SPEED is very different from T_BAT_plus_ilrr_rate,
  *   so we need this conversion.
  */
  switch (speed)
  {
    case BS_SPEED_NotPresent:
      /*
      *   BAT does not include a 'not present' value, but I don't
      *   see any better alternative than this.
      */
      ilrr_data.rate=(T_BAT_plus_ilrr_rate) -1;
      break;

    case BS_SPEED_300_V21:
    case BS_SPEED_300_V110:
      ilrr_data.rate=BAT_ILRR_RATE_300;
      break;

    case BS_SPEED_1200_V22:
    case BS_SPEED_1200_75_V23:
    case BS_SPEED_1200_V120:
    case BS_SPEED_1200_V110:
      ilrr_data.rate=BAT_ILRR_RATE_1200;
      break;

    case BS_SPEED_2400_V22bis:
    case BS_SPEED_2400_V26ter:
    case BS_SPEED_2400_V120:
    case BS_SPEED_2400_V110:
      ilrr_data.rate=BAT_ILRR_RATE_2400;
      break;

    case BS_SPEED_4800_V32:
    case BS_SPEED_4800_V120:
    case BS_SPEED_4800_V110:
      ilrr_data.rate=BAT_ILRR_RATE_4800;
      break;

    case BS_SPEED_9600_V32:
    case BS_SPEED_9600_V34:
    case BS_SPEED_9600_V120:
    case BS_SPEED_9600_V110:
      ilrr_data.rate=BAT_ILRR_RATE_9600;
      break;

    case BS_SPEED_14400_V34:
    case BS_SPEED_14400_V120:
    case BS_SPEED_14400_V110:
      ilrr_data.rate=BAT_ILRR_RATE_14400;
      break;

    case BS_SPEED_19200_V110:
      ilrr_data.rate=BAT_ILRR_RATE_19200;
      break;

    case BS_SPEED_38400_V110:
      ilrr_data.rate=BAT_ILRR_RATE_38400;
      break;

    default:
    case BS_SPEED_AUTO:
    case BS_SPEED_28800_V110:
      ilrr_data.rate=BAT_ILRR_RATE_1;
      break;
  }

  /*
  *   This relies on T_ACI_BS_FRM and T_BAT_framing_format being
  *   identical.
  */
  ilrr_data.framing_format=(T_BAT_framing_format)format;

  /*
  *   This relies on T_ACI_BS_PAR and T_BAT_framing_format being
  *   identical.
  */
  ilrr_data.framing_parity=(T_BAT_framing_parity)parity;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_BUSY            |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_BUSY(
  T_ACI_AT_CMD cmdId,
  SHORT cId)
{
  T_BAT_cmd_response resp;
  T_BAT_no_parameter dummy;  
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;

  TRACE_FUNCTION ("rBAT_BUSY()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if (src_infos->bat_client[src_infos->active_client].atd_live EQ TRUE)
  {
    /*
    *   We were still waiting for a final response to ATD - this is it.
    */
    resp.ctrl_response=BAT_RES_AT_BUSY;
    src_infos->bat_client[src_infos->active_client].atd_live=FALSE;
  }
  else
  {
    /*
    *   We've already had the final response to ATD, so this must be
    *   an unsolicited message.
    */
    resp.ctrl_response=BAT_RES_UNS_AT_BUSY;
  }

  resp.response.ptr_at_busy = &dummy;
  dummy.bat_dummy = 0xFF;

  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos); 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_NO_ANSWER       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_NO_ANSWER(
  T_ACI_AT_CMD cmdId,
  SHORT cId)
{
  T_BAT_cmd_response resp;
  T_BAT_no_parameter dummy;
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;

  TRACE_FUNCTION ("rBAT_NO_ANSWER()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if (src_infos->bat_client[src_infos->active_client].atd_live EQ TRUE)
  {
    /*
    *   We were still waiting for a final response to ATD - this is it.
    */
    resp.ctrl_response=BAT_RES_AT_NO_ANSWER;
    src_infos->bat_client[src_infos->active_client].atd_live=FALSE;
  }
  else
  {
    /*
    *   We've already had the final response to ATD, so this must be
    *   an unsolicited message.
    */
    resp.ctrl_response=BAT_RES_UNS_AT_NO_ANSWER;
  }

  resp.response.ptr_at_no_answer = &dummy;
  dummy.bat_dummy = 0xFF;
  
  aci_bat_send(src_infos,&resp);

  /* check for possible next command queued */
  aci_bat_run_cmd (src_infos);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSIMREM   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSIMREM(
  T_ACI_SIMREM_TYPE srType)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_simrem simrem_data;

  TRACE_FUNCTION ("rBAT_PercentSIMREM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_SIMREM;
  resp.response.ptr_res_percent_simrem=&simrem_data;

  /*
  *   This relies on T_ACI_SIMREM_TYPE being identical to
  *   T_BAT_percent_simrem_m.
  */
  simrem_data.m=(T_BAT_percent_simrem_m)srType;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCLIR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCLIR(
  T_ACI_CLIR_MOD mode,
  T_ACI_CLIR_STAT stat)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clir clir_data;

  TRACE_FUNCTION ("rBAT_PlusCLIR()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_QUE_PLUS_CLIR;
  resp.response.ptr_que_plus_clir=&clir_data;

  /*
  *   T_ACI_CLIR_STAT and T_BAT_plus_clir_m are identical except for
  *   the fact that the BAT version does not include a 'not present'
  *   value.
  */
  if (stat EQ CLIR_STAT_NotPresent)
    clir_data.m=BAT_CLIR_M_UNKNOWN;
  else
    clir_data.m=(T_BAT_plus_clir_m)stat;

  /*
  *   This relies on T_ACI_CLIR_MOD and T_BAT_plus_clir_n being
  *   identical.
  */
  clir_data.n=(T_BAT_plus_clir_n)mode;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCOLR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCOLR(
  T_ACI_COLR_STAT stat)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentCOLR() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentCOLR() - FAKE");
  rCI_PercentCOLR(stat);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCSSI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCSSI(
  T_ACI_CSSI_CODE code,
  SHORT index)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cssi cssi_data;

  TRACE_FUNCTION ("rBAT_PlusCSSI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CSSI;
  resp.response.ptr_res_plus_cssi=&cssi_data;

  /*
  *   This relies on T_ACI_CSSI_CODE being identical to
  *   T_BAT_plus_cssi_code1.
  */
  cssi_data.code1=(T_BAT_plus_cssi_code1)code;

  cssi_data.index=(S32)index;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCSSU        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCSSU(
  T_ACI_CSSU_CODE code,
  SHORT index,
  CHAR *number,
  T_ACI_TOA *type,
  CHAR *subaddr,
  T_ACI_TOS *satype)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cssu cssu_data;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCSSU()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CSSU;
  resp.response.ptr_res_plus_cssu=&cssu_data;

  /*
  *   This relies on T_ACI_CSSU_CODE and T_BAT_plus_cssu_code2
  *   being identical.
  */
  cssu_data.code2=(T_BAT_plus_cssu_code2)code;

  cssu_data.index=(S32)index;

  if (number EQ NULL)
  {
    cssu_data.v_number=FALSE;
  }
  else
  {
    len=strlen(number);

    cssu_data.v_number=TRUE;

    if (len<=BAT_MAX_CSSU_NUMBER_LEN)
    {
      memcpy(cssu_data.number,number,len);
      cssu_data.c_number=(U8)len;
    }
    else
    {
      /*
      *   This is highly unlikely to happen, but if the number is too
      *   big, then put as many digits as will fit in the BAT structure
      *   from the END of the number.
      */
      memcpy(
        cssu_data.number,
        number+(len-BAT_MAX_CSSU_NUMBER_LEN),
        BAT_MAX_CSSU_NUMBER_LEN);

      cssu_data.c_number=(U8)BAT_MAX_CSSU_NUMBER_LEN;
    }
  }

  if (type EQ NULL)
  {
    cssu_data.type=-1;
  }
  else
  {
    cssu_data.type=(U16)toa_merge(*type);
  }

  if (subaddr EQ NULL)
  {
    cssu_data.v_subaddr=FALSE;
    cssu_data.satype=-1;
  }
  else
  {
    cssu_data.v_subaddr=TRUE;

    if (satype EQ NULL)
      cssu_data.satype=-1;
    else
      cssu_data.satype=(S16)tos_merge(*satype);

    len=strlen(subaddr);

    /*
    *   Easiest thing to do is copy as much subaddress data as the
    *   BAT structure can take. The length field will ensure that
    *   there are no problems, and will result in the subaddress
    *   being truncated if it is too long.
    */
    memcpy(cssu_data.subaddr,subaddr,BAT_MAX_SUBADDR_LENGTH);

    if (len>BAT_MAX_SUBADDR_LENGTH)
      cssu_data.c_subaddr=BAT_MAX_SUBADDR_LENGTH;
    else
      cssu_data.c_subaddr=len;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCUSD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCUSD(
  T_ACI_CUSD_MOD m,
  T_ACI_USSD_DATA *ussd,
  SHORT dcs)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cusd cusd_data;

  TRACE_FUNCTION ("rBAT_PlusCUSD()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CUSD;
  resp.response.ptr_res_plus_cusd=&cusd_data;

  /*
  *   This assumes that T_ACI_CUSD_MOD and
  *   T_BAT_VAL_plus_cusd_m are equivalent.
  */
  cusd_data.m=(T_BAT_VAL_plus_cusd_m)m;

  if (ussd)
  {
    USHORT len;
	
    /* If USSD string is coming as 7-bit default GSM char from N/w 
    * and ME uses 2 bytes UCS2 character set for display.
    */ 
    if (( cmh_getAlphabetCb ( dcs ) EQ CSCS_ALPHA_7_Bit ) AND 
		(ati_user_output_cfg[src_id].cscsChset EQ CSCS_CHSET_Ucs2))
    {
      utl_ucs2FromGsm_ussd( 
                            ussd->data,
                            ussd->len,
                           (UBYTE *)cusd_data.str,
                            BAT_MAX_USSD_LEN,
                            &len,
                            GSM_ALPHA_Def,
                            CSCS_ALPHA_8_Bit );
    }
    else {
      utl_ussdDtaToTe(
      ussd->data,
      ussd->len,
      (UBYTE *)cusd_data.str,
      BAT_MAX_USSD_LEN,
      &len,
      (UBYTE)dcs);
    }
    cusd_data.c_str=(U16)len;
    cusd_data.v_str=TRUE;
  }
  else
  {
    cusd_data.v_str=FALSE;
  }

  cusd_data.dcs=(S16)dcs;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCCFC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCCFC (T_ACI_CCFC_SET* setting)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_ccfc ccfc_data = {BAT_CCFC_STATUS_NOT_ACTIVE,
  	                                                      BAT_CCFC_CLASS_NOT_PRESENT,
  	                                                      0,0,{0},0,0,0,0,{0},0,
  	                                                      BAT_CCFC_TIME_NOT_PRESENT};

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  TRACE_FUNCTION ("rBAT_PlusCCFC()");

  /* fill the BAT response struct */
  resp.ctrl_response = BAT_RES_SET_PLUS_CCFC;
  resp.response.ptr_set_plus_ccfc = &ccfc_data;

  ccfc_data.status = (T_BAT_plus_ccfc_status)setting->clsstat.status;
  ccfc_data.bearer_class = (T_BAT_plus_ccfc_bearer_class)setting->clsstat.class_type;

  if (ccfc_data.c_number = strlen(setting->number))
  {
    ccfc_data.v_number = TRUE;
    memcpy(ccfc_data.number, setting->number, ccfc_data.c_number);
  }

  /* Convert type to string */
  ccfc_data.type = toa_merge(setting->type);
  
  if(ccfc_data.c_subaddr = strlen(setting->subaddr))
  {
    ccfc_data.v_subaddr = TRUE;
    memcpy(ccfc_data.subaddr, setting->subaddr, ccfc_data.c_subaddr);
  }

  /* Convert sattype to string */
  ccfc_data.satype = tos_merge(setting->satype);

  ccfc_data.time = (T_BAT_plus_ccfc_time)setting->time;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCLCK        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCLCK(
  T_ACI_CLSSTAT *clsStat)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_clck clck_data;

  TRACE_FUNCTION ("rBAT_PlusCLCK()");

  /*
  *   This shouldn't happen, but there's no point carrying on
  *   if it does.
  */
  if (clsStat EQ NULL)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CLCK;
  resp.response.ptr_set_plus_clck=&clck_data;

  /*
  *   This relies on T_ACI_CLASS being identical to
  *   T_BAT_bearer_class.
  */
  clck_data.bearer_class=(T_BAT_bearer_class)clsStat->class_type;

  /*
  *   This relies on T_ACI_STATUS being identical to
  *   T_BAT_plus_clck_status. It is, except that the BAT does not
  *   have a 'not present' value.
  */
  clck_data.status=(T_BAT_plus_clck_status)clsStat->status;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCIMI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCIMI(
  CHAR *imsi)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cimi cimi_data;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCIMI()");

  /*
  *   This shouldn't happen, but there's no point carrying on
  *   if it does.
  */
  if (imsi EQ NULL)
    return;

  len=strlen(imsi);

  /*
  *   There is also no point carrying on if the BAT structure has
  *   insufficient space to store the whole IMSI.
  */
  if (len>BAT_MAX_CIMI_IMSI_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CIMI;
  resp.response.ptr_set_plus_cimi=&cimi_data;

  memcpy(cimi_data.imsi,imsi,BAT_MAX_CIMI_IMSI_LEN);
  cimi_data.c_imsi=(U8)len;

  aci_bat_send(src_infos,&resp);
}

#ifdef SIM_TOOLKIT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSATI     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSATI     ( SHORT len, UBYTE* satCmd )
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_sati sati_data = {0};


  TRACE_FUNCTION ("rBAT_PercentSATI()");
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /* fill the response struct and call aci_bat_send() */
  resp.ctrl_response = BAT_RES_UNS_PERCENT_SATI;
  resp.response.ptr_res_percent_sati = &sati_data;

  /* Limit copy length to the BAT size */
  if(sati_data.c_satcmd = len)
      memcpy(sati_data.satcmd, satCmd,
             (sati_data.c_satcmd <= BAT_MAX_SATI_SAT_CMD_LEN) ?
             sati_data.c_satcmd : BAT_MAX_SATI_SAT_CMD_LEN);
  
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSATE     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSATE (SHORT len, UBYTE* satCmd)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_percent_sate sate_data = {0};

  TRACE_FUNCTION ("rBAT_PercentSATE()");
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /* fill the response struct and call aci_bat_send() */
  resp.ctrl_response = BAT_RES_SET_PERCENT_SATE;
  resp.response.ptr_set_percent_sate = &sate_data;

  /* Limit copy length to the BAT size */
  if(sate_data.c_satrsp = len)
      memcpy(sate_data.satrsp, satCmd,
             (sate_data.c_satrsp <= BAT_MAX_SATE_SAT_RSP_LEN) ?
             sate_data.c_satrsp : BAT_MAX_SATE_SAT_RSP_LEN);
  
  aci_bat_send(src_infos,&resp);
}
#endif /* SIM_TOOLKIT */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentIMEI     |
+--------------------------------------------------------------------+

  PURPOSE : response to atd*#06#  query the IMEI
*/

GLOBAL void rBAT_PercentIMEI(T_ACI_IMEI *imei)
{
  T_ACI_CMD_SRC              src_id;
  T_ACI_DTI_PRC_PSI         *src_infos;
  T_BAT_cmd_response         resp;
  T_BAT_res_que_percent_imei rsp_imei;

  TRACE_FUNCTION ("rBAT_PercentIMEI()");

  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response = BAT_RES_QUE_PERCENT_IMEI;
  resp.response.ptr_que_percent_imei = &rsp_imei;

  memcpy(&rsp_imei, imei, sizeof(T_BAT_res_que_percent_imei));

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentKSIR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentKSIR (T_ACI_KSIR *ksStat)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  
  TRACE_FUNCTION ("rBAT_PercentKSIR()");

  aci_bat_src_info(&src_id,&src_infos);

  utl_cb_percentKSIR ((U8)src_id, ksStat);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPI      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPI(
  SHORT cId,
  T_ACI_CPI_MSG msgType,
  T_ACI_CPI_IBT ibt,
  T_ACI_CPI_TCH tch,
  USHORT cause)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cpi cpi_data;
  T_ACI_CLCC_CALDESC clist[MAX_CALL_NR];
  UBYTE len;
  SHORT i;

  TRACE_FUNCTION ("rBAT_PercentCPI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CPI;
  resp.response.ptr_res_percent_cpi=&cpi_data;

  cpi_data.cid=(U8)cId;

  /*
  *   This relies on T_ACI_CPI_MSG being equivalent to 
  *   T_BAT_percent_cpi_msgtype.
  */
  cpi_data.msgtype=(T_BAT_percent_cpi_msgtype)msgType;

  /*
  *   This relies on T_ACI_CPI_IBT being equivalent to 
  *   T_BAT_percent_cpi_ibt.
  */
  cpi_data.ibt=(T_BAT_percent_cpi_ibt)ibt;

  /*
  *   This relies on T_ACI_CPI_TCH being equivalent to 
  *   T_BAT_percent_cpi_tch.
  */
  cpi_data.tch=(T_BAT_percent_cpi_tch)tch;

  /*
   * Issue : OMAPS00061262, Removed the two TI internal Causes 
   * MNCC_CAUSE_REEST_STARTED & MNCC_CAUSE_REEST_FINISHED from the %CPI
   */
  if( GET_CAUSE_ORIGIN_ENTITY(cause) EQ MNCC_CC_ORIGINATING_ENTITY  AND 
        GET_CAUSE_DEFBY(cause) EQ DEFBY_STD )
  {         
     cpi_data.cause=(S32)GET_CAUSE_VALUE(cause);
  }
  else
  {
    cpi_data.cause=(S32) BAT_PARAMETER_NOT_PRESENT;
  }

  /*
  *   Set all the rest of the information to 'not present' as a
  *   default.
  */
  cpi_data.dir=NOT_PRESENT_8BIT;
  cpi_data.mode=-1;
  cpi_data.type=-1;
  cpi_data.v_number=FALSE;
  cpi_data.v_alpha=FALSE;
  cpi_data.line=-1;
  cpi_data.prog_desc = BAT_P_CPI_PROG_DESC_NOT_PRESENT;

  /*
  *   Now try to obtain the rest of the information.
  */
  if (qAT_PercentCLCC(src_id,clist) EQ AT_CMPL)
  {
    for (i=0;i<MAX_CALL_NR;i++)
    {
      T_ACI_CLCC_CALDESC *call;

      /*
      *   Get a pointer to this particular call for convenience.
      */
      call=&clist[i];

      if (call->idx EQ ACI_NumParmNotPresent)
        break;

      if (call->idx EQ cId)
      {
        cpi_data.dir=(U8)call->dir;
        cpi_data.mode=(S16)call->mode;

        len=strlen(call->number);

        if (len AND (len<=BAT_MAX_CPI_NUMBER_LEN))
        {
          cpi_data.v_number=TRUE;
          memcpy(cpi_data.number,call->number,len);
          cpi_data.c_number=(U8)len;
        }

        if (call->type.ton NEQ TON_NotPresent)
          cpi_data.type=(S16)toa_merge(call->type);
      
        if (call->alpha.len)
        {
          USHORT len_cvtd_alpha;

#ifdef NO_ASCIIZ
          utl_chsetFromGsm(
            call->alpha.data,
            call->alpha.len,
            cpi_data.alpha,
            BAT_MAX_CPI_ALPHA_LEN,
            &len_cvtd_alpha,
            GSM_ALPHA_Def);
#else
          utl_chsetFromGsm(
            call->alpha,
            0,
            cpi_data.alpha,
            BAT_MAX_CPI_ALPHA_LEN,
            &len_cvtd_alpha,
            GSM_ALPHA_Int);
#endif

          cpi_data.c_alpha=(U8)len_cvtd_alpha;
          cpi_data.v_alpha=TRUE;
        }

        cpi_data.line=(S16)call->class_type;
        cpi_data.prog_desc = (T_BAT_percent_cpi_prog_desc)call->prog_desc;

        /*
        *   We have the information, now break out of the for loop.
        */
        break;

      }
    }
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCTYI     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCTYI(
  T_ACI_CTTY_NEG neg,
  T_ACI_CTTY_TRX trx)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_ctyi ctyi_data;

  TRACE_FUNCTION ("rBAT_PercentCTYI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CTYI;
  resp.response.ptr_res_percent_ctyi=&ctyi_data;

  /*
  *   This relies on T_ACI_CTTY_NEG and T_BAT_percent_ctyi_neg
  *   being identical.
  */
  ctyi_data.neg=(T_BAT_percent_ctyi_neg)neg;

  /*
  *   The BAT has a 'not present' value but not an 'unknown' value,
  *   vice versa for the ACI.
  */
  if (trx EQ CTTY_TRX_Unknown)
    ctyi_data.trx=BAT_TRX_NOT_PRESENT;
  else
    ctyi_data.trx=(T_BAT_trx)trx;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCNUM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCNUM(
  T_ACI_CNUM_MSISDN *msisdn,
  UBYTE num)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cnum cnum_data;
  UBYTE i;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PlusCNUM()");

  /*
  *   This shouldn't happen, but there's no point carrying on
  *   if it does.
  */
  if (msisdn EQ NULL)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CNUM;
  resp.response.ptr_set_plus_cnum=&cnum_data;

  /*
  *   This has delivered a list, however the BAT response only
  *   allows for single items. So we must send each one 
  *   individually.
  */
  for (i=0;i<num;i++)
  {
    T_ACI_CNUM_MSISDN *m;

    /*
    *   Get a pointer to this particular item for convenience.
    */
    m=&msisdn[i];

    if (m->vldFlag EQ TRUE)
    {
      len=strlen(m->alpha);

      if (len)
      {
        USHORT cvtd_len=0;

        cnum_data.v_alpha=TRUE;

        utl_chsetFromGsm(
          (UBYTE *)m->alpha,
          (USHORT)len,
          (UBYTE *)cnum_data.alpha,
          BAT_MAX_CNUM_ALPHA_LEN,
          &cvtd_len,
          GSM_ALPHA_Int);

        cnum_data.c_alpha=(U8)cvtd_len;
      }
      else
      {
        cnum_data.v_alpha=FALSE;
      }

      len=strlen(m->number);

      if (len<=BAT_MAX_CNUM_NUMBER_LEN)
      {
        memcpy(cnum_data.number,m->number,len);
        cnum_data.c_number=(U8)len;
      }
      else
      {
        /*
        *   This is highly unlikely to happen, but if the number is too
        *   big, then put as many digits as will fit in the BAT structure
        *   from the END of the number.
        */
        memcpy(
          cnum_data.number,
          m->number+(len-BAT_MAX_CNUM_NUMBER_LEN),
          BAT_MAX_CNUM_NUMBER_LEN);/*lint -e670 (Warning - Possible access beyond array for function)*/

        cnum_data.c_number=(U8)BAT_MAX_CNUM_NUMBER_LEN;
      }

      cnum_data.type=(U8)toa_merge(m->type);

      cnum_data.index=(U8)i;

      /*
      *   Note that we are losing 'speed', 'service' and 'itc'
      *   as they are not in the BAT.
      */
      
      aci_bat_send(src_infos,&resp);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCPOL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCPOL(
  SHORT startIdx,
  SHORT lastIdx,
  T_ACI_CPOL_OPDESC *operLst,
  SHORT usdNtry)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;

  TRACE_FUNCTION ("rBAT_PlusCPOL()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
  *   This could be a response to either a query or a test command.
  */
  if (aci_bat_cur_cmd_get(src_infos) EQ BAT_CMD_TST_PLUS_CPOL)
  {
    T_BAT_res_tst_plus_cpol cpol_data;

    resp.ctrl_response=BAT_RES_TST_PLUS_CPOL;
    resp.response.ptr_tst_plus_cpol=&cpol_data;

    /*
    *   Note that we lose usdNtry, which is not in the BAT (or in
    *   GSM 07.07) but is in the ACI. Whether this will give us
    *   problems is for further study.
    */
    cpol_data.index1=((S16)startIdx EQ ACI_NumParmNotPresent) ? 1 : (S16)startIdx;
    cpol_data.index2=(S16)lastIdx;

    aci_bat_send(src_infos,&resp);
  }
  else if (operLst NEQ NULL)
  {
    T_BAT_res_que_plus_cpol cpol_data;
    UBYTE i;

    resp.ctrl_response=BAT_RES_QUE_PLUS_CPOL;
    resp.response.ptr_que_plus_cpol=&cpol_data;

    /*
    *   This has delivered a list, however the BAT response only
    *   allows for single items. So we must send each one 
    *   individually.
    */
    for (i=0;i<MAX_OPER;i++)    
    {
      T_ACI_CPOL_OPDESC *o;
      UBYTE len;

      /*
      *   Get a pointer to this particular item for convenience.
      */
      o=&operLst[i];

      /*
      *   Stop when we reach the end of the list.
      */
      if (o->index EQ ACI_NumParmNotPresent)
        break;

      cpol_data.index=(U8)o->index;

      /*
      *   This relies on T_ACI_CPOL_FRMT and T_BAT_plus_cpol_format
      *   being identical.
      */
      cpol_data.format=(T_BAT_plus_cpol_format)o->format;

      len=strlen(o->oper);

      /*
      *   We copy as much of the operator name as the BAT structure
      *   can handle. If it is too long it will effectively be
      *   truncated.
      */
      if (len>BAT_MAX_CPOL_OPER_LEN)
        cpol_data.c_oper=BAT_MAX_CPOL_OPER_LEN;
      else
        cpol_data.c_oper=(U8)len;

      memcpy(cpol_data.oper,o->oper,sizeof(o->oper));

      aci_bat_send(src_infos,&resp);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCCCM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCCCM(
  LONG *ccm)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cccm cccm_data;

  TRACE_FUNCTION ("rBAT_PlusCCCM()");

  /*
  *   This shouldn't happen, but there's no point carrying on
  *   if it does.
  */
  if (ccm EQ NULL)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CCCM;
  resp.response.ptr_res_plus_cccm=&cccm_data;

  cccm_data.ccm=(U32)*ccm;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCTV      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCTV(void)
{
#ifndef FF_ATI_BAT
  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentCTV() - IGNORED");
#else
  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentCTV() - FAKE");
  rCI_PercentCTV();
#endif
}

#ifdef SIM_TOOLKIT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSATN     |
+--------------------------------------------------------------------+

  PURPOSE : 
  
  N.B. Not all ACI parameters are supported by BAT
       ACI parameters are SHORT len, UBYTE* satCmd, T_ACI_SATN_CNTRL_TYPE  cntrl_type
*/
GLOBAL void rBAT_PercentSATN (SHORT len, UBYTE* satCmd, T_ACI_SATN_CNTRL_TYPE  cntrl_type)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_satn satn_data = {0};

  TRACE_FUNCTION ("rBAT_PercentSATN()");
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /* fill the response struct and call aci_bat_send() */
  resp.ctrl_response = BAT_RES_UNS_PERCENT_SATN;
  resp.response.ptr_res_percent_satn = &satn_data;

  if(satn_data.c_satcmd = len)
      memcpy(satn_data.satcmd, satCmd,
             (satn_data.c_satcmd <= BAT_MAX_SATN_SAT_CMD_LEN) ?
             satn_data.c_satcmd : BAT_MAX_SATN_SAT_CMD_LEN);
  
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSATA     |
+--------------------------------------------------------------------+

  PURPOSE : 
  
  N.B. Not all ACI parameters are supported by BAT
       ACI parameters are SHORT cId, LONG rdlTimeout_ms, T_ACI_SATA_ADD* addParm
*/
#ifdef FF_SAT_E
GLOBAL void rBAT_PercentSATA (SHORT cId, LONG rdlTimeout_ms, T_ACI_SATA_ADD* addParm)
#else
GLOBAL void rBAT_PercentSATA (SHORT cId, LONG rdlTimeout_ms)
#endif
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_sata sata_data = {0};

  TRACE_FUNCTION ("rBAT_PercentSATA()");

#ifdef FF_SAT_E
  /* Store for use with rCI_PercentSATA - ATI maintenace 
   * Addparm is a global variable ! */
  memcpy(&Addparm, addParm, sizeof(T_ACI_SATA_ADD));
#endif
  
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /* fill the response struct and call aci_bat_send() */
  resp.ctrl_response = BAT_RES_UNS_PERCENT_SATA;
  resp.response.ptr_res_percent_sata = &sata_data;
  sata_data.redial_timeout = (S32)rdlTimeout_ms;
  aci_bat_send(src_infos,&resp);
}
#endif /* SIM_TOOLKIT */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_sms_ready       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_sms_ready(void)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_sms_ready() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_sms_ready() - FAKE");
  rCI_sms_ready();

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_phb_status      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_phb_status(
  T_ACI_PB_STAT status)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_phb_status() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_phb_status() - FAKE");
  rCI_phb_status(status);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSIMINS   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSIMINS(
  T_ACI_CME_ERR err)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_simins simins_data;

  TRACE_FUNCTION ("rBAT_PercentSIMINS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_SIMINS;
  resp.response.ptr_res_percent_simins=&simins_data;

  /*
  *   T_ACI_CME_ERR is different from T_BAT_percent_simins_n, so
  *   we need this conversion.
  */
  switch (err)
  {
    case CME_ERR_NotPresent:
      simins_data.n=BAT_P_SIMINS_N_NO_PIN;
      break;

    case CME_ERR_SimPinReq:
      simins_data.n=BAT_P_SIMINS_N_SIM_PIN;
      break;

    case CME_ERR_SimPukReq:
      simins_data.n=BAT_P_SIMINS_N_SIM_PUK;
      break;

    case CME_ERR_SimFail:
      simins_data.n=BAT_P_SIMINS_N_SIM_FAIL;
      break;

    case CME_ERR_SimBusy:
      simins_data.n=BAT_P_SIMINS_N_SIM_BUSY;
      break;

    case CME_ERR_SimWrong:
      simins_data.n=BAT_P_SIMINS_N_SIM_WRONG;
      break;

    case CME_ERR_SimResetNeeded:
      simins_data.n=BAT_P_SIMINS_N_SIM_RESET_NEEDED;
      break;

    default:
      simins_data.n=BAT_P_SIMINS_N_UNKNOWN;
      break;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCRSM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCRSM(
  SHORT sw1,
  SHORT sw2,
  SHORT rspLen,
  UBYTE *rsp)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_crsm crsm_data;

  TRACE_FUNCTION ("rBAT_PlusCRSM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CRSM;
  resp.response.ptr_set_plus_crsm=&crsm_data;

  crsm_data.sw1=(U8)sw1;
  crsm_data.sw2=(U8)sw2;
  
  if (rsp EQ NULL)
  {
    crsm_data.c_response=0;
  }
  else
  {
    /*
    *   There's nothing meaningful that we can do if there is too
    *   much data to fit in the BAT message, so give up.
    */
    if (rspLen>BAT_MAX_CRSM_RESPONSE_LEN)
      return;

    crsm_data.c_response=(U16)rspLen;
    memcpy(crsm_data.response,rsp,BAT_MAX_CRSM_RESPONSE_LEN);
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCSIM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCSIM(
  SHORT len,
  UBYTE *rsp)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_csim csim_data;

  TRACE_FUNCTION ("rBAT_PlusCSIM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_SET_PLUS_CSIM;
  resp.response.ptr_set_plus_csim=&csim_data;

  if (rsp EQ NULL)
  {
    csim_data.c_response=0;
  }
  else
  {
    /*
    *   There's nothing meaningful that we can do if there is too
    *   much data to fit in the BAT message, so give up.
    */
    if (len>BAT_MAX_CSIM_RESP_LEN)
      return;

    csim_data.c_response=(U16)len;
    memcpy(csim_data.response,rsp,BAT_MAX_CSIM_RESP_LEN);
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCCBS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCCBS(
  T_ACI_CCBS_IND ind,
  T_ACI_CCBS_STAT status,
  T_ACI_CCBS_SET *setting,
  BOOL intermediate_result)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PercentCCBS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  if (intermediate_result EQ FALSE)
  {
    T_BAT_res_uns_percent_ccbs ccbs_data;

    resp.ctrl_response=BAT_RES_UNS_PERCENT_CCBS;
    resp.response.ptr_res_percent_ccbs=&ccbs_data;

    /*
    *   This relies on T_ACI_CCBS_IND being identical to T_BAT_percent_ccbs_ind
    *   and T_ACI_CCBS_STAT being identical to T_BAT_percent_ccbs_stat.
    */
    ccbs_data.ind=(T_BAT_percent_ccbs_ind)ind;
    ccbs_data.stat=(T_BAT_percent_ccbs_stat)status;

    /*
    *   As a default, set all the rest of the parameters as 'not present'.
    */
    ccbs_data.idx=BAT_P_CCBS_INDEX_NOT_PRESENT;
    ccbs_data.v_number=FALSE;
    ccbs_data.type=-1;
    ccbs_data.v_number=FALSE;
    ccbs_data.satype=-1;
    ccbs_data.bearer_class=BAT_CLASS_NOT_PRESENT;
    ccbs_data.ptn=BAT_P_CCBS_PATTERN_NOT_PRESENT;

    if (setting NEQ NULL)
    {
      if (setting->number[0])
      {
        len=strlen(setting->number);

        if (len<=BAT_MAX_CCBS_NUMBER_LEN)
        {
          memcpy(ccbs_data.number,setting->number,len);
          ccbs_data.c_number=(U8)len;
        }
        else
        {
          /*
          *   This is highly unlikely to happen, but if the number is too
          *   big, then put as many digits as will fit in the BAT structure
          *   from the END of the number.
          */
          memcpy(
            ccbs_data.number,
            setting->number+(len-BAT_MAX_CCBS_NUMBER_LEN),
            BAT_MAX_CCBS_NUMBER_LEN);/*lint -e670 (Warning - Possible access beyond array for function)*/

          ccbs_data.c_number=(U8)BAT_MAX_CCBS_NUMBER_LEN;
        }

        ccbs_data.v_number=TRUE;
        ccbs_data.type=(S16)toa_merge(setting->type);
      }

      if (setting->subaddr[0])
      {
        len=strlen(setting->subaddr);

        /*
        *   If the subaddress is too big to fit in the BAT message, 
        *   just leave it out.
        */
        if (len<=BAT_MAX_SUBADDR_LENGTH)
        {
          memcpy(ccbs_data.subaddr,setting->subaddr,len);
          ccbs_data.c_subaddr=(U8)len;
          ccbs_data.v_subaddr=TRUE;
          ccbs_data.satype=(S16)tos_merge(setting->satype);
        }
      }

      /*
      *   This relies on T_ACI_CLASS being identical to T_BAT_bearer_class and
      *   T_ACI_ALRT_PATTERN being identical to T_BAT_percent_ccbs_ptn.
      */
      ccbs_data.bearer_class=(T_BAT_bearer_class)setting->class_type;
      ccbs_data.ptn=(T_BAT_percent_ccbs_ptn)setting->alrtPtn;

      /*
      *   This relies on the value presented being equivalent to
      *   T_BAT_percent_ccbs_idx.
      */
      ccbs_data.idx=(T_BAT_percent_ccbs_idx)setting->idx;
    }
  }
  else
  {
    T_BAT_res_que_percent_ccbs ccbs_data;

    resp.ctrl_response=BAT_RES_QUE_PERCENT_CCBS;
    resp.response.ptr_que_percent_ccbs=&ccbs_data;

    /*
    *   This relies on T_ACI_CCBS_IND being identical to T_BAT_percent_ccbs_ind
    *   and T_ACI_CCBS_STAT being identical to T_BAT_percent_ccbs_stat.
    */
    ccbs_data.ind=(T_BAT_percent_ccbs_ind)ind;
    ccbs_data.stat=(T_BAT_percent_ccbs_stat)status;

    /*
    *   As a default, set all the rest of the parameters as 'not present'.
    */
    ccbs_data.idx=BAT_P_CCBS_INDEX_NOT_PRESENT;
    ccbs_data.v_number=FALSE;
    ccbs_data.type=-1;
    ccbs_data.v_number=FALSE;
    ccbs_data.satype=-1;
    ccbs_data.bearer_class=BAT_CLASS_NOT_PRESENT;
    ccbs_data.ptn=BAT_P_CCBS_PATTERN_NOT_PRESENT;

    if (setting NEQ NULL)
    {
      if (setting->number[0])
      {
        len=strlen(setting->number);

        if (len<=BAT_MAX_CCBS_NUMBER_LEN)
        {
          memcpy(ccbs_data.number,setting->number,len);
          ccbs_data.c_number=(U8)len;
        }
        else
        {
          /*
          *   This is highly unlikely to happen, but if the number is too
          *   big, then put as many digits as will fit in the BAT structure
          *   from the END of the number.
          */
          memcpy(
            ccbs_data.number,
            setting->number+(len-BAT_MAX_CCBS_NUMBER_LEN),
            BAT_MAX_CCBS_NUMBER_LEN);/*lint -e670 (Warning - Possible access beyond array for function)*/

          ccbs_data.c_number=(U8)BAT_MAX_CCBS_NUMBER_LEN;
        }

        ccbs_data.v_number=TRUE;
        ccbs_data.type=(S16)toa_merge(setting->type);
      }

      if (setting->subaddr[0])
      {
        len=strlen(setting->subaddr);

        /*
        *   If the subaddress is too big to fit in the BAT message, 
        *   just leave it out.
        */
        if (len<=BAT_MAX_SUBADDR_LENGTH)
        {
          memcpy(ccbs_data.subaddr,setting->subaddr,len);
          ccbs_data.c_subaddr=(U8)len;
          ccbs_data.v_subaddr=TRUE;
          ccbs_data.satype=(S16)tos_merge(setting->satype);
        }
      }

      /*
      *   This relies on T_ACI_CLASS being identical to T_BAT_bearer_class and
      *   T_ACI_ALRT_PATTERN being identical to T_BAT_percent_ccbs_ptn.
      */
      ccbs_data.bearer_class=(T_BAT_bearer_class)setting->class_type;
      ccbs_data.ptn=(T_BAT_percent_ccbs_ptn)setting->alrtPtn;

      /*
      *   This relies on the value presented being equivalent to
      *   T_BAT_percent_ccbs_idx.
      */
      ccbs_data.idx=(T_BAT_percent_ccbs_idx)setting->idx;
    }
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCCWV        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCCWV(
  T_ACI_CCWV_CHRG charging)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_no_parameter dummy;
  
  TRACE_FUNCTION ("rBAT_PlusCCWV()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response = BAT_RES_UNS_PLUS_CCWV;
  resp.response.ptr_plus_ccwv = &dummy;
  dummy.bat_dummy = 0xFF;

  /*
  *   Note that the 'charging' parameter is lost as it is not in
  *   the BAT.
  */

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCNAP     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCNAP(
  T_callingName *NameId,
  T_ACI_CNAP_STATUS status)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cnap cnap_data;
  T_BAT_res_uns_percent_cnap uns_cnap_data;



  TRACE_FUNCTION ("rBAT_PercentCNAP()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
  *   This could be either an unsolicited event or a response to a 
  *   query. We have no way of knowing which it was, so we assume that
  *   if we have a NameId, it was unsolicited.
  */
  if (NameId EQ NULL)
  {
    resp.ctrl_response=BAT_RES_QUE_PERCENT_CNAP;
    resp.response.ptr_que_percent_cnap = &cnap_data;

    /*
    *   This relies on T_ACI_CNAP_STATUS being identical to
    *   T_BAT_percent_cnap_status.
    */
    cnap_data.status=(T_BAT_percent_cnap_status)status;
  }
  else
  {
    T_namePresentationAllowed *cnap_name_info;

    resp.ctrl_response=BAT_RES_UNS_PERCENT_CNAP;
    resp.response.ptr_res_percent_cnap = &uns_cnap_data;

    cnap_name_info=NULL;

    if (NameId->v_namePresentationAllowed)
    {
      uns_cnap_data.pres_mode=BAT_P_CNAP_PRES_NAME;
      cnap_name_info=&NameId->namePresentationAllowed;
    }
    else if (NameId->v_presentationRestricted)
    {
      uns_cnap_data.pres_mode=BAT_P_CNAP_PRES_RESTRICTED;
    }
    else if (NameId->v_nameUnavailable)
    {
      uns_cnap_data.pres_mode=BAT_P_CNAP_PRES_NO_NAME;
    }
    else if (NameId->v_namePresentationRestricted)
    {
      uns_cnap_data.pres_mode=BAT_P_CNAP_PRES_NAME_RESTRICTED;
      cnap_name_info=&NameId->namePresentationRestricted;
    }

    /*
    *   As a default, set the rest of the parameters as 'not present'.
    */
    uns_cnap_data.dcs=-1;
    uns_cnap_data.v_name=FALSE;

    if (cnap_name_info)
    {
      if (cnap_name_info->v_dataCodingScheme)
        uns_cnap_data.dcs=(S16)cnap_name_info->dataCodingScheme;

      if ((cnap_name_info->v_nameString) AND
        (cnap_name_info->v_lengthInCharacters))
      {
        uns_cnap_data.v_name=TRUE;

        /*
        *   Copy as much of the data into the BAT message as it can
        *   handle. If it is too long it will effectively be truncated.
        */
        if (cnap_name_info->lengthInCharacters>BAT_MAX_CNAP_NAME_LEN)
          uns_cnap_data.c_name=BAT_MAX_CNAP_NAME_LEN;
        else
          uns_cnap_data.c_name=cnap_name_info->lengthInCharacters;

        memcpy(
          uns_cnap_data.name,
          &cnap_name_info->nameString.b_nameString,
          BAT_MAX_CNAP_NAME_LEN);
      }
    }
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_SignalSMS       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_SignalSMS(
  UBYTE state)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_SignalSMS() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_SignalSMS() - FAKE");
  rCI_SignalSMS(state);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCLAN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCLAN(
  T_ACI_LAN_SUP *CLang)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clan clan_data;
  CHAR *str;
 
  TRACE_FUNCTION ("rBAT_PlusCLAN()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_QUE_PLUS_CLAN;
  resp.response.ptr_que_plus_clan=&clan_data;

  str=aci_bat_lang_str(CLang);

  if (str EQ NULL)
    return;

  clan_data.c_code=strlen(str);

  if (clan_data.c_code<=BAT_MAX_CLAN_CODE_LEN)
  {
    memcpy(clan_data.code,str,BAT_MAX_CLAN_CODE_LEN);
    aci_bat_send(src_infos,&resp);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCLAE        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCLAE(
  T_ACI_LAN_SUP *CLang)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_clae clae_data;
  CHAR *str;

  TRACE_FUNCTION ("rBAT_PlusCLAE()");

  /*
  *   This shouldn't happen, but there's no point carrying on
  *   if it does.
  */
  if (CLang EQ NULL)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CLAE;
  resp.response.ptr_res_plus_clae=&clae_data;

  str=aci_bat_lang_str(CLang);

  if (str EQ NULL)
    return;

  clae_data.c_code=strlen(str);

  if (clae_data.c_code<=BAT_MAX_CLAE_CODE_LEN)
  {
    memcpy(clae_data.code,str,BAT_MAX_CLAE_CODE_LEN);
    aci_bat_send(src_infos,&resp);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCSQ      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
#ifdef FF_PS_RSSI
GLOBAL void rBAT_PercentCSQ(
  UBYTE rssi,
  UBYTE ber,
  UBYTE actlevel,
  UBYTE min_access_level)
#else
GLOBAL void rBAT_PercentCSQ(
  UBYTE rssi,
  UBYTE ber,
  UBYTE actlevel)
#endif
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_csq csq_data;

  TRACE_FUNCTION ("rBAT_PercentCSQ()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CSQ;
  resp.response.ptr_res_percent_csq=&csq_data;

  csq_data.rssi=(S16)rssi;
  csq_data.ber=(S16)ber;
  csq_data.actlevel=(S16)actlevel;

#ifdef FF_PS_RSSI
  csq_data.min_access_level = (S16)min_access_level;
#endif
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentALS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentALS(
  T_ACI_ALS_MOD ALSmode)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_percent_als als_data;
 
  TRACE_FUNCTION ("rBAT_PercentALS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_TST_PERCENT_ALS;
  resp.response.ptr_tst_percent_als=&als_data;

  /*
  *   T_ACI_ALS_MOD is not the same as T_BAT_percent_als_mode, so
  *   we need this conversion.
  */
  switch (ALSmode)
  {
    default:
    case ALS_MOD_NOTPRESENT:
      /*
      *   As the only (mandatory) parameter has no value allowed for
      *   in BAT we may as well give up.
      */
      return;
 
    case ALS_MOD_SPEECH:
      als_data.mode=BAT_P_ALS_MOD_SPEECH;
      break;

    case ALS_MOD_AUX_SPEECH:
      als_data.mode=BAT_P_ALS_MOD_AUX_SPEECH;
      break;
  }

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCTZV        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
#ifdef FF_TIMEZONE
GLOBAL void rBAT_PlusCTZV(S32 timezone)
#else
GLOBAL void rBAT_PlusCTZV(UBYTE *timezone)
#endif
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_ctzv ctzv_data;

  TRACE_FUNCTION ("rBAT_PlusCTZV()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CTZV;
  resp.response.ptr_res_plus_ctzv=&ctzv_data;

#ifdef FF_TIMEZONE
  ctzv_data.tz=(U8)timezone;
#else
  if (timezone EQ NULL)
    return;
  else
    ctzv_data.tz=(U8)*timezone;
#endif
  
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCREG     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCREG(
  T_ACI_CREG_STAT status,
  USHORT lac,
  USHORT cid,
  T_ACI_P_CREG_GPRS_IND gprs_ind,
  U8     rt)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_creg creg_data;

  TRACE_FUNCTION ("rBAT_PercentCREG()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CREG;
  resp.response.ptr_res_percent_creg=&creg_data;

  /*
  *   This relies on T_ACI_CREG_STAT and T_BAT_percent_creg_stat being
  *   identical.
  */
  creg_data.stat=(T_BAT_percent_creg_stat)status;

  creg_data.lac=(S32)lac;
  creg_data.ci=(S32)cid;

  /*
  *   This relies on T_ACI_P_CREG_GPRS_IND and T_BAT_percent_creg_gprs_ind
  *   being identical.
  */
  creg_data.gprs_ind=(T_BAT_percent_creg_gprs_ind)gprs_ind;
  creg_data.rt=(T_BAT_percent_creg_rt)rt;

  aci_bat_send(src_infos,&resp);
}

#ifdef GPRS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGACT       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCGACT(
  SHORT link_id)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCGACT() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCGACT() - FAKE");
  rCI_PlusCGACT(link_id);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGDATA      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCGDATA(
  SHORT link_id)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCGDATA() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCGDATA() - FAKE");
  rCI_PlusCGDATA(link_id);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGANS       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCGANS(
  SHORT link_id)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCGANS() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCGANS() - FAKE");
  rCI_PlusCGANS(link_id);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGEREP      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCGEREP(
  T_CGEREP_EVENT event,
  T_CGEREP_EVENT_REP_PARAM *param)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCGEREP() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCGEREP() - FAKE");
  rCI_PlusCGEREP(event,param);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGREG       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCGREG(
  T_CGREG_STAT stat,
  USHORT lac,
  USHORT ci)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_plus_cgreg cgreg_data;

  TRACE_FUNCTION ("rBAT_PlusCGREG()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PLUS_CGREG;
  resp.response.ptr_res_plus_cgreg=&cgreg_data;

  /*
  *   This relies on T_CGREG_STAT and T_BAT_plus_cgreg_stat being
  *   identical. They are, except in that the ACI version has a
  *   'not present' value. We deal with it by using 'unknown'.
  */
  if (stat EQ CGREG_STAT_NOT_PRESENT)
    cgreg_data.stat=BAT_CGREG_STAT_UNKN;
  else
    cgreg_data.stat=(T_BAT_plus_cgreg_stat)stat;

  cgreg_data.lac=(S32)lac;
  cgreg_data.ci=(S32)ci;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_changedQOS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_changedQOS(
  SHORT cid,
  T_PS_qos *qos)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_changedQOS() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_changedQOS() - FAKE");
  rCI_changedQOS((U8)cid,qos);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSNCNT    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSNCNT(
  UBYTE c_id,
  ULONG octets_uplink,
  ULONG octets_downlink,
  ULONG packets_uplink,
  ULONG packets_downlink)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_sncnt sncnt_data;

  TRACE_FUNCTION ("rBAT_PercentSNCNT()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_SNCNT;
  resp.response.ptr_uns_percent_sncnt=&sncnt_data;

  /*
  *   Warning: this could potentially deliver a value outside
  *   the range specified by BAT.
  */
  sncnt_data.cid=(T_BAT_pdp_cid)c_id;

  sncnt_data.oct_up=(U32)octets_uplink;
  sncnt_data.oct_down=(U32)octets_downlink;
  sncnt_data.pkt_up=(U32)packets_uplink;
  sncnt_data.pkt_down=(U32)packets_downlink;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCGREG    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCGREG(
  T_P_CGREG_STAT stat,
  USHORT lac,
  USHORT ci,
  BOOL bActiveContext)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cgreg cgreg_data;

  TRACE_FUNCTION ("rBAT_PercentCGREG()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CGREG;
  resp.response.ptr_res_percent_cgreg=&cgreg_data;

  /*
  *   This relies on T_P_CGREG_STAT and T_BAT_percent_cgreg_stat being
  *   identical. They are, except in that the ACI version has a
  *   'not present' value. We deal with it by using 'unknown'.
  */
  if (stat EQ P_CGREG_STAT_NOT_PRESENT)
    cgreg_data.stat=BAT_P_CGREG_STAT_UNKN;
  else
    cgreg_data.stat=(T_BAT_percent_cgreg_stat)stat;

  cgreg_data.lac=(S32)lac;
  cgreg_data.ci=(S32)ci;

  aci_bat_send(src_infos,&resp);
}

#endif /* GPRS */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentEM       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentEM(
  T_EM_VAL *val_tmp)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentEM() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentEM() - FAKE");
  rCI_PercentEM(val_tmp);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentEMET     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentEMET(
  T_DRV_SIGNAL_EM_EVENT *Signal)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentEMET() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentEMET() - FAKE");
  rCI_PercentEMET(Signal);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentEMETS    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentEMETS(
  UBYTE entity)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PercentEMETS() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PercentEMETS() - FAKE");
  rCI_PercentEMETS(entity);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPNUMS   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPNUMS(
  UBYTE element_index,
  UBYTE index_level,
  CHAR *alpha_tag,
  CHAR *number,
  BOOL premium_flag,
  BOOL network_flag)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  UBYTE len;

  TRACE_FUNCTION ("rBAT_PercentCPNUMS()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
  *   This is either a response to a test or a set command.
  */
  if (aci_bat_cur_cmd_get(src_infos) EQ BAT_CMD_TST_PERCENT_CPNUMS)
  {
    T_BAT_res_tst_percent_cpnums cpnums_data;

    resp.ctrl_response=BAT_RES_TST_PERCENT_CPNUMS;
    resp.response.ptr_tst_percent_cpnums=&cpnums_data;

    cpnums_data.element_id=(U8)element_index;
    cpnums_data.index_level=(U8)index_level;

    cpnums_data.c_alpha_tag=0;

    if (alpha_tag NEQ NULL)
    {
      len=strlen(alpha_tag);

      if (len)
      {
        USHORT len_cvtd_text=0;

        /*
        *   Convert the data read from the SIM into the currently 
        *   selected character set.
        */
        utl_chsetFromSim(
          (UBYTE *)alpha_tag,
          (USHORT)len,
          (UBYTE *)cpnums_data.alpha_tag,
          BAT_MAX_CPNUMS_ALPHA_TAG_LEN,
          &len_cvtd_text,
          GSM_ALPHA_Def);

        cpnums_data.c_alpha_tag=(U8)len_cvtd_text;
      }
    }

    if (number EQ NULL)
    {
      cpnums_data.c_number=0;
      cpnums_data.number[0]=0;
    }
    else
    {
      len=strlen(number);

      if (len<=BAT_MAX_CPNUMS_NUMBER_LEN)
      {
        memcpy(cpnums_data.number,number,len);
        cpnums_data.c_number=(U8)len;
      }
      else
      {
        /*
        *   This is highly unlikely to happen, but if the number is too
        *   big, then put as many digits as will fit in the BAT structure
        *   from the END of the number.
        */
        memcpy(
          cpnums_data.number,
          number+(len-BAT_MAX_CPNUMS_NUMBER_LEN),
          BAT_MAX_CPNUMS_NUMBER_LEN);

        cpnums_data.c_number=(U8)BAT_MAX_CPNUMS_NUMBER_LEN;
      }
    }

    cpnums_data.premium_flag=(premium_flag EQ TRUE) ?
      BAT_P_CPNUMS_PREMIUM_FLAG_SET:BAT_P_CPNUMS_PREMIUM_FLAG_NOT_SET;

    cpnums_data.network_flag=(network_flag EQ TRUE) ?
      BAT_P_CPNUMS_NETWORK_SET:BAT_P_CPNUMS_NETWORK_NOT_SET;

    aci_bat_send(src_infos,&resp);
  }
  else
  {
    T_BAT_res_set_percent_cpnums cpnums_data;

    resp.ctrl_response=BAT_RES_SET_PERCENT_CPNUMS;
    resp.response.ptr_set_percent_cpnums=&cpnums_data;

    cpnums_data.element_id=(U8)element_index;
    cpnums_data.index_level=(U8)index_level;

    cpnums_data.c_alpha_tag=0;

    if (alpha_tag NEQ NULL)
    {
      len=strlen(alpha_tag);

      if (len)
      {
        USHORT len_cvtd_text=0;

        /*
        *   Convert the data read from the SIM into the currently 
        *   selected character set.
        */
        utl_chsetFromSim(
          (UBYTE *)alpha_tag,
          (USHORT)len,
          (UBYTE *)cpnums_data.alpha_tag,
          BAT_MAX_CPNUMS_ALPHA_TAG_LEN,
          &len_cvtd_text,
          GSM_ALPHA_Def);

        cpnums_data.c_alpha_tag=(U8)len_cvtd_text;
      }
    }

    if (number EQ NULL)
    {
      cpnums_data.c_number=0;
      cpnums_data.number[0]=0;
    }
    else
    {
      len=strlen(number);

      if (len<=BAT_MAX_CPNUMS_NUMBER_LEN)
      {
        memcpy(cpnums_data.number,number,len);
        cpnums_data.c_number=(U8)len;
      }
      else
      {
        /*
        *   This is highly unlikely to happen, but if the number is too
        *   big, then put as many digits as will fit in the BAT structure
        *   from the END of the number.
        */
        memcpy(
          cpnums_data.number,
          number+(len-BAT_MAX_CPNUMS_NUMBER_LEN),
          BAT_MAX_CPNUMS_NUMBER_LEN);

        cpnums_data.c_number=(U8)BAT_MAX_CPNUMS_NUMBER_LEN;
      }
    }

    cpnums_data.premium_flag=(premium_flag EQ TRUE) ?
      BAT_P_CPNUMS_PREMIUM_FLAG_SET:BAT_P_CPNUMS_PREMIUM_FLAG_NOT_SET;

    cpnums_data.network_flag=(network_flag EQ TRUE) ?
      BAT_P_CPNUMS_NETWORK_SET:BAT_P_CPNUMS_NETWORK_NOT_SET;

    aci_bat_send(src_infos,&resp);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPVWI    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPVWI(
  UBYTE flag_set, 
  USHORT line)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;

  TRACE_FUNCTION ("rBAT_PercentCPVWI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  /*
  *   This could be either a response to a set command, or an 
  *   unsolicited event.
  */
  if (aci_bat_cur_cmd_get(src_infos) NEQ BAT_CMD_SET_PERCENT_CPVWI)
  {
    T_BAT_res_uns_percent_cpvwi cpvwi_data;

    resp.ctrl_response=BAT_RES_UNS_PERCENT_CPVWI;
    resp.response.ptr_res_percent_cpvwi=&cpvwi_data;

    cpvwi_data.line=(T_BAT_percent_cpvwi_lines)line;

    cpvwi_data.status=(flag_set EQ TRUE) ?
      BAT_P_CPVWI_FLAG_ACTIVATED:BAT_P_CPVWI_FLAG_DEACTIVATED;

    aci_bat_send(src_infos,&resp);
  }
  else
  {
    T_BAT_res_set_percent_cpvwi cpvwi_data;

    resp.ctrl_response=BAT_RES_SET_PERCENT_CPVWI;
    resp.response.ptr_set_percent_cpvwi=&cpvwi_data;

    cpvwi_data.line=(T_BAT_percent_cpvwi_lines)line;

    cpvwi_data.status=(flag_set EQ TRUE) ?
      BAT_P_CPVWI_FLAG_ACTIVATED:BAT_P_CPVWI_FLAG_DEACTIVATED;

    aci_bat_send(src_infos,&resp);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPROAM   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPROAM(
  UBYTE roam_status)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cproam cproam_data;

  TRACE_FUNCTION ("rBAT_PercentCPROAM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CPROAM;
  resp.response.ptr_res_percent_cproam=&cproam_data;

  cproam_data.roam_status=(roam_status) ?
    BAT_P_CPROAM_STATUS_ROAMING:BAT_P_CPROAM_STATUS_NOT_ROAMING;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCIEV        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PlusCIEV(
  T_ACI_MM_CIND_VAL_TYPE sCindValues,
  T_ACI_MM_CMER_VAL_TYPE sCmerSettings)
{
#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_PlusCIEV() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_PlusCIEV() - FAKE");
  rCI_PlusCIEV(sCindValues,sCmerSettings);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentRDL      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentRDL(
  T_ACI_CC_REDIAL_STATE state)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_rdl rdl_data;

  TRACE_FUNCTION ("rBAT_PercentRDL()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_RDL;
  resp.response.ptr_res_percent_rdl=&rdl_data;

  /*
  *   This relies on T_ACI_CC_REDIAL_STATE being identical to
  *   T_BAT_percent_rdl_state.
  */
  rdl_data.state=(T_BAT_percent_rdl_state)state;

  aci_bat_send(src_infos,&resp);
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentRDLB     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentRDLB(
  T_ACI_CC_RDL_BLACKL_STATE state)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_rdlb rdlb_data;

  TRACE_FUNCTION ("rBAT_PercentRDLB()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_RDLB;
  resp.response.ptr_res_percent_rdlb=&rdlb_data;

  /*
  *   This relies on T_ACI_CC_RDL_BLACKL_STATE being identical to
  *   T_BAT_percent_rdlb_state.
  */
  rdlb_data.state=(T_BAT_percent_rdlb_state)state;

  aci_bat_send(src_infos,&resp);
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCCCN     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCCCN(
  T_ACI_FAC_DIR tDirection,
  SHORT cId,
  T_MNCC_fac_inf *fie)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cccn cccn_data;

  TRACE_FUNCTION ("rBAT_PercentCCCN()");

  /*
  *   No point continuing if we don't have this.
  */
  if (fie EQ NULL)
    return;

  /*
  *   There's also no point continuing if the facility information
  *   can't fit into the BAT message.
  */
  if (fie->l_fac>BAT_MAX_CCCN_FACILITY_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CCCN;
  resp.response.ptr_res_percent_cccn=&cccn_data;

  /*
  *   This relies on T_ACI_FAC_DIR and T_BAT_percent_cccn_direction
  *   being identical. They are, except that the ACI has a 'not
  *   present' value.
  */
  cccn_data.direction=(T_BAT_percent_cccn_direction)tDirection;

  cccn_data.cid=(U16)cId;

  /*
  *   Copy the facility data, secure in the knowledge that we have
  *   enough room for it.
  */
  memcpy(cccn_data.facility,fie->fac,BAT_MAX_CCCN_FACILITY_LEN);
  cccn_data.c_facility=(U8)fie->l_fac;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCSSN     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCSSN(
  T_ACI_FAC_DIR tDirection,
  T_ACI_FAC_TRANS_TYPE tType,
  T_MNCC_fac_inf *fie)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cssn cssn_data;

  TRACE_FUNCTION ("rBAT_PercentCSSN()");

  /*
  *   No point continuing if we don't have this.
  */
  if (fie EQ NULL)
    return;

  /*
  *   There's also no point continuing if the facility information
  *   can't fit into the BAT message.
  */
  if (fie->l_fac>BAT_MAX_CSSN_FACILITY_LEN)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CSSN;
  resp.response.ptr_res_percent_cssn=&cssn_data;

  /*
  *   This relies on T_ACI_FAC_DIR being the same as
  *   T_BAT_percent_cssn_direction, and T_ACI_FAC_TRANS_TYPE being the
  *   same as T_BAT_percent_cssn_trans_type. They are, except that
  *   T_ACI_FAC_DIR also has a 'not present' value.
  */
  cssn_data.direction=(T_BAT_percent_cssn_direction)tDirection;
  cssn_data.trans_type=(T_BAT_percent_cssn_trans_type)tType;

  /*
  *   Copy the facility data, secure in the knowledge that we have
  *   enough room for it.
  */
  memcpy(cssn_data.facility,fie->fac,BAT_MAX_CSSN_FACILITY_LEN);
  cssn_data.c_facility=(U8)fie->l_fac;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCSTAT    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCSTAT(
  T_ACI_STATE_MSG msgType)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cstat cstat_data;

  TRACE_FUNCTION ("rBAT_PercentCSTAT()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CSTAT;
  resp.response.ptr_res_percent_cstat=&cstat_data;

  /*
  *   This relies on T_ACI_ENTITY_ID_MSG being the same as
  *   T_BAT_percent_cstat_entity_id and T_ACI_ENTITY_STATE_MSG being
  *   the same as T_BAT_percent_cstat_status.
  */
  cstat_data.entity_id=(T_BAT_percent_cstat_entity_id)msgType.entityId;
  cstat_data.status=(T_BAT_percent_cstat_status)msgType.entityState;
  
  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_Z               |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_Z(void)
{

#ifndef FF_ATI_BAT

  /*
  *   This is not included in the BAT interface, so ignore it.
  */
  TRACE_FUNCTION ("rBAT_Z() - IGNORED");

#else

  /*
  *   But for test purposes, fake a response by calling the equivalent
  *   ATI function directly.
  */
  TRACE_FUNCTION ("rBAT_Z() - FAKE");
  rCI_Z();

#endif

}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPRSM    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPRSM(
  T_ACI_CPRSM_MOD mode)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cprsm cprsm_data;

  TRACE_FUNCTION ("rBAT_PercentCPRSM()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_QUE_PERCENT_CPRSM;
  resp.response.ptr_que_percent_cprsm=&cprsm_data;

  /*
  *   This relies on T_ACI_CPRSM_MOD and T_BAT_percent_cprsm_mode being
  *   identical. They are, except that the ACI version includes a
  *   'not present' value.
  */
  cprsm_data.mode=(T_BAT_percent_cprsm_mode)mode;

  aci_bat_send(src_infos,&resp);
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCTZV     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCTZV(
  T_MMR_INFO_IND *mmr_info_ind,
  S32 timezone)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_ctzv ctzv_data;
 
  TRACE_FUNCTION ("rBAT_PercentCTZV()");

  if (mmr_info_ind EQ NULL)
    return;

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CTZV;
  resp.response.ptr_res_percent_ctzv=&ctzv_data;

  if ((mmr_info_ind->plmn.v_plmn) AND
    ((SIZE_MCC+SIZE_MNC) <= BAT_MAX_CTZV_NUM_OPER_LEN))
  {
    ctzv_data.v_num_oper=TRUE;
    memcpy(ctzv_data.num_oper,mmr_info_ind->plmn.mcc,SIZE_MCC);
    memcpy(ctzv_data.num_oper+SIZE_MCC,mmr_info_ind->plmn.mnc,SIZE_MNC);
    ctzv_data.c_num_oper=(U8)(SIZE_MCC+SIZE_MNC);
  }
  else
  {
    ctzv_data.v_num_oper=FALSE;
  }

  if (mmr_info_ind->short_name.v_name)
  {
    USHORT len;

    utl_chsetFromGsm(
      mmr_info_ind->short_name.text,
      mmr_info_ind->short_name.c_text,
      ctzv_data.short_oper,
      BAT_MAX_CTZV_SHORT_OPER,
      &len,
      GSM_ALPHA_Def);

    ctzv_data.v_short_oper=TRUE;
    ctzv_data.c_short_oper=(U8)len;

    /* Extend BAT to included add_ci parameter */
    ctzv_data.add_ci = mmr_info_ind->short_name.add_ci;
  }
  else
  {
    ctzv_data.v_short_oper=FALSE;
  }

  if (mmr_info_ind->full_name.v_name)
  {
    USHORT len;

    utl_chsetFromGsm(
      mmr_info_ind->full_name.text,
      mmr_info_ind->full_name.c_text,
      ctzv_data.long_oper,
      BAT_MAX_CTZV_LONG_OPER,
      &len,
      GSM_ALPHA_Def);

    ctzv_data.v_long_oper=TRUE;
    ctzv_data.c_long_oper=(U8)len;
     
    /* Extend BAT to included add_ci parameter */
    ctzv_data.add_ci = mmr_info_ind->full_name.add_ci;
  }
  else
  {
    ctzv_data.v_long_oper=FALSE;
  }
  
  ctzv_data.year=(U8)mmr_info_ind->time.year;
  ctzv_data.month=(U8)mmr_info_ind->time.month;
  ctzv_data.day=(U8)mmr_info_ind->time.day;
  ctzv_data.hour=(U8)mmr_info_ind->time.hour;
  ctzv_data.minutes=(U8)mmr_info_ind->time.minute;
  ctzv_data.seconds=(U8)mmr_info_ind->time.second;
  ctzv_data.time_zone=(S8)timezone;

  aci_bat_send(src_infos,&resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCNIV     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCNIV(T_MMR_INFO_IND *mmr_info_ind)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cniv cniv_data;
  
  TRACE_FUNCTION ("rBAT_PercentCNIV()");
  
  if (mmr_info_ind EQ NULL)
  {
    return;
  }
  
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);
  
  resp.ctrl_response=BAT_RES_UNS_PERCENT_CNIV;
  resp.response.ptr_res_percent_cniv=&cniv_data;
  
  if ((mmr_info_ind->plmn.v_plmn) AND
    ((SIZE_MCC+SIZE_MNC) <= BAT_MAX_CNIV_NUM_OPER_LEN))
  {
    cniv_data.v_num_oper=TRUE;
    memcpy(cniv_data.num_oper,mmr_info_ind->plmn.mcc,SIZE_MCC);
    memcpy(cniv_data.num_oper+SIZE_MCC,mmr_info_ind->plmn.mnc,SIZE_MNC);
    cniv_data.c_num_oper=(U8)(SIZE_MCC+SIZE_MNC);
  }
  else
  {
    cniv_data.v_num_oper=FALSE;
  }
  
  if (mmr_info_ind->short_name.v_name)
  {        
    cniv_data.v_short_oper=TRUE;
    cniv_data.c_short_oper=mmr_info_ind->short_name.c_text;
    cniv_data.dcs_short_oper = mmr_info_ind->short_name.dcs;
    memcpy(cniv_data.short_oper,mmr_info_ind->short_name.text,mmr_info_ind->short_name.c_text);
    
    /* Extend BAT to included add_ci parameter */
    cniv_data.add_ci = mmr_info_ind->short_name.add_ci;
  }
  else
  {
    cniv_data.v_short_oper=FALSE;
  }
  
  if (mmr_info_ind->full_name.v_name)
  {    
    cniv_data.v_long_oper=TRUE;
    cniv_data.c_long_oper=mmr_info_ind->full_name.c_text;
    cniv_data.dcs_long_oper = mmr_info_ind->full_name.dcs;
    memcpy(cniv_data.long_oper,mmr_info_ind->full_name.text,mmr_info_ind->full_name.c_text);
    
    /* Extend BAT to included add_ci parameter */
    cniv_data.add_ci = mmr_info_ind->full_name.add_ci;
  }
  else
  {
    cniv_data.v_long_oper=FALSE;
  }
  
  aci_bat_send(src_infos,&resp);
}

#ifdef GPRS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCGEV     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCGEV (T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cgev cgev_data;
  U8 len;

  TRACE_FUNCTION ("rBAT_PercentCGEV()");

  if (event EQ CGEREP_EVENT_INVALID)
  {
    return;
  }
  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

#ifdef _SIMULATION_
  if (ati_user_output_cfg[src_id].Percent_CGEREP_stat EQ 0)
  {
    return; /* don't indicate %CGEV to a source on which AT%CGEREP was not set up */
  }
#endif

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CGEV;
  resp.response.ptr_res_percent_cgev=&cgev_data;

  /*
  *   This relies on T_CGEREP_EVENT being identical to
  *   T_BAT_percent_cgev_evt, which it is apart from the additional
  *   'invalid' value in the ACI which we dealt with earlier.
  */
  cgev_data.evt=(T_BAT_percent_cgev_evt)event;
  cgev_data.pdp_type=BAT_PDP_TYPE_NOT_PRESENT;
  cgev_data.v_pdp_addr=FALSE;
  cgev_data.cid=BAT_PDP_CID_NOT_PRESENT;
  cgev_data.p_mobile_class=BAT_P_MOBILE_CLASS_NOT_PRESENT;

  if (param NEQ NULL)
  {
    switch (event)
    {
      case CGEREP_EVENT_REJECT:
      {
        switch(param->reject.pdp_addr.ctrl_ip_address)
        {
           case NAS_is_ipv4:
             len = sizeof(param->reject.pdp_addr.ip_address.ipv4_addr.a4);
             cgev_data.pdp_type = BAT_PDP_TYPE_IPV4;
             if ((len > 0) AND (len <= BAT_MAX_PDP_ADD))
             {
                cgev_data.v_pdp_addr = TRUE;
                cgev_data.c_pdp_addr = (U8)len;
                memcpy(cgev_data.pdp_addr,param->reject.pdp_addr.ip_address.ipv4_addr.a4,len);
             }
            break;
           case NAS_is_ipv6:
             len = sizeof(param->reject.pdp_addr.ip_address.ipv6_addr.a6);
             cgev_data.pdp_type = BAT_PDP_TYPE_IPV6;
             if ((len > 0) AND (len <= BAT_MAX_PDP_ADD))
             {
                cgev_data.v_pdp_addr = TRUE;
                cgev_data.c_pdp_addr = (U8)len;
                memcpy(cgev_data.pdp_addr,param->reject.pdp_addr.ip_address.ipv6_addr.a6,len);
             }
            break;
           default:
            break;
        }
      }
      break;


      case CGEREP_EVENT_NW_CLASS:
      case CGEREP_EVENT_ME_CLASS:
        /*
        *   This relies on T_PERCENT_CGCLASS being identical to
        *   T_BAT_p_mobile_class.
        */
        cgev_data.p_mobile_class=(T_BAT_p_mobile_class)param->mobile_class;
        break;

      case CGEREP_EVENT_NW_REACT:
      case CGEREP_EVENT_NW_DEACT:
      case CGEREP_EVENT_ME_DEACT:
      case CGEREP_EVENT_NW_ACT:
      case CGEREP_EVENT_ME_ACT:
        cgev_data.cid=(T_BAT_pdp_cid)param->act.cid;
        switch(param->act.pdp_addr.ctrl_ip_address)
        {
           case NAS_is_ipv4:
             len = sizeof(param->act.pdp_addr.ip_address.ipv4_addr.a4);
             cgev_data.pdp_type = BAT_PDP_TYPE_IPV4;
             if ((len > 0) AND (len <= BAT_MAX_PDP_ADD))
             {
                cgev_data.v_pdp_addr = TRUE;
                cgev_data.c_pdp_addr = (U8)len;
                memcpy(cgev_data.pdp_addr,param->act.pdp_addr.ip_address.ipv4_addr.a4,len);
             }
            break;
           case NAS_is_ipv6:
             len = sizeof(param->act.pdp_addr.ip_address.ipv6_addr.a6);
             cgev_data.pdp_type = BAT_PDP_TYPE_IPV6;
             if ((len > 0) AND (len <= BAT_MAX_PDP_ADD))
             {
                cgev_data.v_pdp_addr = TRUE;
                cgev_data.c_pdp_addr = (U8)len;
                memcpy(cgev_data.pdp_addr,param->act.pdp_addr.ip_address.ipv6_addr.a6,len);
             }
            break;
           default:
            break;
         }
        break;

      default:
        break;
    }
  }

  aci_bat_send(src_infos,&resp);
}
#endif /* GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentCPRI     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentCPRI(
  UBYTE gsm_ciph,
  UBYTE gprs_ciph)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cpri cpri_data;
 
  TRACE_FUNCTION ("rBAT_PercentCPRI()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response=BAT_RES_UNS_PERCENT_CPRI;
  resp.response.ptr_res_percent_cpri=&cpri_data;

  cpri_data.gsm_ciph=(T_BAT_percent_cpri_gsm_ciph)gsm_ciph;
  cpri_data.gprs_ciph=(T_BAT_percent_cpri_gprs_ciph)gprs_ciph;

  aci_bat_send(src_infos,&resp);
}



#ifdef FF_FAX
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFIS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFIS( T_ACI_F_VR vr,
                          T_ACI_F_BR br,
                          T_ACI_F_WD wd,
                          T_ACI_F_LN ln,
                          T_ACI_F_DF df,
                          T_ACI_F_EC ec,
                          T_ACI_F_BF bf,
                          T_ACI_F_ST st,
                          T_ACI_F_JP jp) 
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fis fis;

  TRACE_FUNCTION ("rBAT_PlusFIS()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FIS;
  resp.response.ptr_que_plus_fis = &fis;

  fis.vr = (T_BAT_fax_vr)vr;
  fis.br = (T_BAT_fax_br)br;
  fis.wd = (T_BAT_fax_wd)wd;
  fis.ln = (T_BAT_fax_ln)ln;
  fis.df = (T_BAT_fax_df)df;
  fis.ec = (T_BAT_fax_ec)ec;
  fis.bf = (T_BAT_fax_bf)bf;
  fis.st = (T_BAT_fax_st)st;
  fis.jp = (T_BAT_fax_jp)jp;

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFCS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFCS( T_ACI_F_VR vr,
                          T_ACI_F_BR br,
                          T_ACI_F_WD wd,
                          T_ACI_F_LN ln,
                          T_ACI_F_DF df,
                          T_ACI_F_EC ec,
                          T_ACI_F_BF bf,
                          T_ACI_F_ST st,
                          T_ACI_F_JP jp) 
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fcs fcs;

  TRACE_FUNCTION ("rBAT_PlusFCS()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCS;
  resp.response.ptr_que_plus_fcs = &fcs;

  fcs.vr = (T_BAT_fax_vr)vr;
  fcs.br = (T_BAT_fax_br)br;
  fcs.wd = (T_BAT_fax_wd)wd;
  fcs.ln = (T_BAT_fax_ln)ln;
  fcs.df = (T_BAT_fax_df)df;
  fcs.ec = (T_BAT_fax_ec)ec;
  fcs.bf = (T_BAT_fax_bf)bf;
  fcs.st = (T_BAT_fax_st)st;
  fcs.jp = (T_BAT_fax_jp)jp;

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFSA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFSA(U8 c_sub_str, U8 *sub_str)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fsa fsa;

  TRACE_FUNCTION ("rBAT_PlusFSA()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FSA;
  resp.response.ptr_que_plus_fsa = &fsa;
  fsa.c_sub_str = c_sub_str;
  memcpy(fsa.sub_str, sub_str, c_sub_str);

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFPA         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFPA(U8 c_spa_str, U8 *spa_str)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpa fpa;

  TRACE_FUNCTION ("rBAT_PlusFPA()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPA;
  resp.response.ptr_que_plus_fpa = &fpa;
  fpa.c_spa_str = c_spa_str;
  memcpy(fpa.spa_str, spa_str, c_spa_str);

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFPW         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFPW(U8 c_pw_str, U8 *pw_str)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpw fpw;

  TRACE_FUNCTION ("rBAT_PlusFPW()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPW;
  resp.response.ptr_que_plus_fpw = &fpw;
  fpw.c_pw_str = c_pw_str;
  memcpy(fpw.pw_str, pw_str, c_pw_str);

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFPI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFPI(U8 c_id_str, U8 *id_str)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpi fpi;

  TRACE_FUNCTION ("rBAT_PlusFPI()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPI;
  resp.response.ptr_que_plus_fpi = &fpi;
  fpi.c_id_str = c_id_str;
  memcpy(fpi.id_str, id_str, c_id_str);

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFNS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFNS(U8 c_nsf, U8 *nsf)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fns fns;

  TRACE_FUNCTION ("rBAT_PlusFNS()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FNS;
  resp.response.ptr_que_plus_fns = &fns;
  fns.c_nsf = c_nsf;
  memcpy(fns.nsf, nsf, c_nsf);

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFHS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFHS(T_BAT_plus_fhs_status status)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fhs fhs;

  TRACE_FUNCTION ("rBAT_PlusFHS()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FHS;
  resp.response.ptr_que_plus_fhs = &fhs;
  fhs.status = status;

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFPS         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFPS(T_BAT_plus_fps_ppr ppr)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fps fps;

  TRACE_FUNCTION ("rBAT_PlusFPS()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPS;
  resp.response.ptr_que_plus_fps = &fps;
  fps.ppr = ppr;

  aci_bat_send(src_infos, &resp);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFCO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFCO(void)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fco fco;

  TRACE_FUNCTION ("rBAT_PlusFCO()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCO;
  resp.response.ptr_que_plus_fco = &fco;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFTI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFTI(CHAR *tsi)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fti fti;

  TRACE_FUNCTION ("rBAT_PlusFTI()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FTI;
  resp.response.ptr_que_plus_fti = &fti;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFCI         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFCI(CHAR *rmtId)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fci fci;

  TRACE_FUNCTION ("rBAT_PlusFCI()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FCI;
  resp.response.ptr_que_plus_fci = &fci;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFHT         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFHT(U16 len, U8 *hdlc)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fht fht;

  TRACE_FUNCTION ("rBAT_PlusFHT()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FHT;
  resp.response.ptr_que_plus_fht = &fht;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFHR         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFHR(U16 len, U8 *hdlc)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fhr fhr;

  TRACE_FUNCTION ("rBAT_PlusFHR()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FHR;
  resp.response.ptr_que_plus_fhr = &fhr;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFET         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFET(T_ACI_FET_PPM ppm)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fet fet;

  TRACE_FUNCTION ("rBAT_PlusFET()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FET;
  resp.response.ptr_que_plus_fet = &fet;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFVO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFVO(void)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fvo fvo;

  TRACE_FUNCTION ("rBAT_PlusFVO()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FVO;
  resp.response.ptr_que_plus_fvo = &fvo;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFPO         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFPO(void)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fpo fpo;

  TRACE_FUNCTION ("rBAT_PlusFPO()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FPO;
  resp.response.ptr_que_plus_fpo = &fpo;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFNF         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFNF(U16 len, U8 *nsf)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fnf fnf;

  TRACE_FUNCTION ("rBAT_PlusFNF()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FNF;
  resp.response.ptr_que_plus_fnf = &fnf;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFNC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFNC(U16 len, U8 *nsc)
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_fnc fnc;

  TRACE_FUNCTION ("rBAT_PlusFNC()");
 
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_QUE_PLUS_FNC;
  resp.response.ptr_que_plus_fnc = &fnc;

  aci_bat_send(src_infos, &resp);
*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusFTC         |
+--------------------------------------------------------------------+

  PURPOSE : ITU-T.32
*/
GLOBAL void rBAT_PlusFTC( T_ACI_F_VR vr,
                          T_ACI_F_BR br,
                          T_ACI_F_WD wd,
                          T_ACI_F_LN ln,
                          T_ACI_F_DF df,
                          T_ACI_F_EC ec,
                          T_ACI_F_BF bf,
                          T_ACI_F_ST st,
                          T_ACI_F_JP jp) 
{
/*
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ftc ftc;

  TRACE_FUNCTION ("rBAT_PlusFTC()");

  aci_bat_src_info(&src_id, &src_infos);
 
  resp.ctrl_response = BAT_RES_QUE_PLUS_FTC;
  resp.response.ptr_que_plus_ftc = &ftc;

  aci_bat_send(src_infos, &resp);
*/
}

#endif  /* FF_FAX */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PercentSIMEF    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void rBAT_PercentSIMEF(T_SIM_FILE_UPDATE_IND *sim_file_update_ind)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_simef simef_data;
  UBYTE n;
 
  TRACE_FUNCTION ("rBAT_PercentSIMEF()");

  /*
  *   Get the source ID and a pointer to the PSI source information.
  */
  aci_bat_src_info(&src_id, &src_infos);

  resp.ctrl_response = BAT_RES_UNS_PERCENT_SIMEF;
  resp.response.ptr_res_percent_simef = &simef_data;

  for (n=0; ((n<sim_file_update_ind->val_nr) AND (n<BAT_MAX_SIMEF_EF_LEN)); n++)
  {
    memcpy(&(simef_data.ef[n]), &(sim_file_update_ind->file_info[n]), sizeof(T_BAT_file_info));
  }

  simef_data.c_ef = (U8)sim_file_update_ind->val_nr;

  aci_bat_send(src_infos,&resp);
}


#ifdef REL99
GLOBAL void rBAT_PercentCMGRS (UBYTE mode,
                              T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                              T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind)
{
  T_ACI_CMD_SRC src_id;
  T_ACI_DTI_PRC_PSI *src_infos;
  T_BAT_cmd_response resp;
  T_BAT_res_uns_percent_cmgrs cmgrs_data;

  TRACE_FUNCTION ("rBAT_PercentSIMEF()");

  /*
   *   Get the source ID and a pointer to the PSI source information.
   */
  aci_bat_src_info(&src_id,&src_infos);

  resp.ctrl_response = BAT_RES_UNS_PERCENT_CMGRS;
  resp.response.ptr_res_percent_cmgrs = &cmgrs_data;

  if (mode EQ BAT_CMGRS_MODE_MANUAL_RETRANS AND mnsms_retrans_cnf NEQ NULL)
  {
    cmgrs_data.mode  = (T_BAT_percent_cmgrs_mode)mode;
    cmgrs_data.tp_mr = mnsms_retrans_cnf->tp_mr;
  }
  else if (mode EQ BAT_CMGRS_MODE_ENABLE_AUTO_RETRANS AND mnsms_send_prog_ind NEQ NULL)
  {
    cmgrs_data.mode         = (T_BAT_percent_cmgrs_mode)mode;
    cmgrs_data.resend_count = mnsms_send_prog_ind->resend_count;
    cmgrs_data.max_retrans  = mnsms_send_prog_ind->max_retrans;
  }
  else
  {
    TRACE_ERROR("%CMGRS: wrong combination of parameters");
    return;
  }
  aci_bat_send(src_infos,&resp);
}
#endif


#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : rBAT_PlusCGCMOD      |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void rBAT_PlusCGCMOD ( void )
{
  TRACE_FUNCTION("rBAT_PlusCGCMOD()");
  return;
}
#endif /* REL99 */




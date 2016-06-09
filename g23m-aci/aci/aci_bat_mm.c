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
|  Purpose :  This BAT wrapper modul is ...
| 
+----------------------------------------------------------------------------- 
*/ 

#include "aci_all.h"     /* includes prim.h, which includes p_bat.h */ 
#include "aci_cmh.h"     /* prototypes of sAT_,qAT_,tAT_    */ 
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 

#ifdef FF_ATI_BAT
#include "typedefs.h"
#include "gdd.h"
#include "bat.h"
#include "aci_bat.h"
#endif /* FF_ATI_BAT */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCOPS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
#undef  min
#define min(a,b) ((a) < (b) ? (a) : (b)) /* from gpf\inc\vcms */

GLOBAL T_ACI_BAT_RSLT sBAT_PlusCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  U8   c_oper = cmd->params.ptr_set_plus_cops->c_oper;
  CHAR op[MAX_ALPHA_OPER_LEN] = {0};
  
  TRACE_FUNCTION ("sBAT_PlusCOPS()");

  c_oper = min(c_oper, MAX_ALPHA_OPER_LEN);

  if (cmd->params.ptr_set_plus_cops->v_oper EQ TRUE)
    memcpy(op, cmd->params.ptr_set_plus_cops->oper, c_oper);
  
  ret =(T_ACI_BAT_RSLT) sAT_PlusCOPS ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                      (T_ACI_COPS_MOD)cmd->params.ptr_set_plus_cops->mode,
                      (T_ACI_COPS_FRMT)cmd->params.ptr_set_plus_cops->format, op);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCOPS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  CHAR op[MAX_ALPHA_OPER_LEN] = {0};

  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cops cops;

  TRACE_FUNCTION ("qBAT_PlusCOPS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_COPS;
  resp.response.ptr_que_plus_cops = &cops;

  ret =(T_ACI_BAT_RSLT) qAT_PlusCOPS((T_ACI_CMD_SRC)src_infos_psi->srcId, &mode, &format, op);

  cops.mode    = (T_BAT_plus_cops_mode)mode;
  cops.format  = (T_BAT_plus_cops_format)format;
  cops.c_oper  = strlen(op);
  cops.v_oper  = cops.c_oper ? TRUE : FALSE;

  if (cops.v_oper)
  {
    memcpy(cops.oper, op, cops.c_oper);
  }
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCOPS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_COPS_LST operLst;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_cops *pTPC;

  SHORT startIdx = 0, lastIdx = 0;
  BOOL END_OF_LIST = FALSE;

  TRACE_FUNCTION ("tBAT_PlusCOPS()");

  memset(&resp, 0x00, sizeof(T_BAT_cmd_response));
  pTPC = resp.response.ptr_tst_plus_cops;
  resp.ctrl_response = BAT_RES_TST_PERCENT_COPS;
  
  do
  {
    int i;

    startIdx += lastIdx;
    ret =(T_ACI_BAT_RSLT) tAT_PlusCOPS((T_ACI_CMD_SRC)src_infos_psi->srcId, startIdx, &lastIdx, &operLst[0]);
    if (ret NEQ ACI_BAT_CMPL)
      return (ret);
    
    for (i = 0; i < lastIdx; i++)
    {
      if ((operLst[i].status EQ COPS_STAT_NotPresent) AND (*operLst[i].numOper EQ 0))
      {
        END_OF_LIST = TRUE;
        break;
      }
      /* Fill BAT resp structs */
      pTPC->status = (T_BAT_plus_cops_status)operLst[i].status;
      pTPC->c_short_oper = strlen(operLst[i].shortOper);
      pTPC->c_long_oper = strlen(operLst[i].longOper);
      pTPC->c_num_oper = strlen(operLst[i].numOper);

      /* Check if there is long operator name, set valid flag and store it. */
      if(pTPC->c_long_oper)
      {
        pTPC->v_long_oper = TRUE;
        strncpy((CHAR*)pTPC->long_oper, operLst[i].longOper, pTPC->c_long_oper);
      }

      /* Check if there is short operator name, set valid flag and store it. */
      if(pTPC->c_short_oper)
      {
        pTPC->v_short_oper = TRUE;
        strncpy((CHAR*)pTPC->short_oper, operLst[i].shortOper, pTPC->c_short_oper);
      }

      /* Check if there is num operator name, set valid flag and store it. */
      if(pTPC->c_num_oper)
      {
        pTPC->v_num_oper = TRUE;
        strncpy((CHAR*)pTPC->num_oper, operLst[i].numOper, pTPC->c_num_oper);
      }

      aci_bat_send(src_infos_psi, &resp);
    }
    return (ACI_BAT_CMPL);
  } while(!END_OF_LIST);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCOPS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  U8   c_oper = cmd->params.ptr_set_percent_cops->c_oper;
  CHAR op[MAX_ALPHA_OPER_LEN] = {0};

  TRACE_FUNCTION ("sBAT_PercentCOPS()");

  c_oper = min(c_oper, MAX_ALPHA_OPER_LEN);

  if (cmd->params.ptr_set_percent_cops->v_oper EQ TRUE)
    memcpy(op, cmd->params.ptr_set_percent_cops->oper, c_oper);

  ret =(T_ACI_BAT_RSLT) sAT_PercentCOPS ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                         (T_ACI_COPS_MOD)cmd->params.ptr_set_percent_cops->mode,
                         (T_ACI_COPS_FRMT)cmd->params.ptr_set_percent_cops->format, op);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentCOPS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_COPS_LST operLst;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_percent_cops cops;   

  SHORT startIdx = 0, lastIdx = 0;
  BOOL END_OF_LIST = FALSE;

  TRACE_FUNCTION ("tBAT_PercentCOPS()");

  resp.ctrl_response = BAT_RES_TST_PERCENT_COPS;
  resp.response.ptr_tst_percent_cops = &cops;
  
  do
  {
    int i;

    startIdx += lastIdx;
    ret =(T_ACI_BAT_RSLT) tAT_PlusCOPS((T_ACI_CMD_SRC)src_infos_psi->srcId, startIdx, &lastIdx, &operLst[0]);
    if (ret NEQ ACI_BAT_CMPL)
      return (ret);
    
    for (i = 0; i < lastIdx; i++)
    {
      if ((operLst[i].status EQ COPS_STAT_NotPresent) AND (*operLst[i].numOper EQ 0))
      {
        END_OF_LIST = TRUE;
        break;
      }
      /* Fill BAT resp structs */
      cops.status = (T_BAT_percent_cops_status)operLst[i].status;
      cops.c_short_oper = strlen(operLst[i].shortOper);
      cops.c_long_oper = strlen(operLst[i].longOper);
      cops.c_num_oper = strlen(operLst[i].numOper);

      /* Check if there is long operator name, set valid flag and store it. */
      if(cops.c_long_oper)
      {
        cops.v_long_oper = TRUE;
        strncpy((CHAR*)cops.long_oper, operLst[i].longOper, cops.c_long_oper);
      }

      /* Check if there is short operator name, set valid flag and store it. */
      if(cops.c_short_oper)
      {
        cops.v_short_oper = TRUE;
        strncpy((CHAR*)cops.short_oper, operLst[i].shortOper, cops.c_short_oper);
      }

      /* Check if there is num operator name, set valid flag and store it. */
      if(cops.c_num_oper)
      {
        cops.v_num_oper = TRUE;
        strncpy((CHAR*)cops.num_oper, operLst[i].numOper, cops.c_num_oper);
      }

      aci_bat_send(src_infos_psi, &resp);
    }
    return (ACI_BAT_CMPL);
  } while(!END_OF_LIST);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCOPS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCOPS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  CHAR op[MAX_ALPHA_OPER_LEN] = {0};

  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  T_ACI_COPS_SVST svrStatus = COPS_SVST_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cops cops;

  TRACE_FUNCTION ("qBAT_PercentCOPS()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_COPS;
  resp.response.ptr_que_percent_cops = &cops;

  ret =(T_ACI_BAT_RSLT) qAT_PercentCOPS((T_ACI_CMD_SRC)src_infos_psi->srcId, &mode, &format, &svrStatus, op);

  cops.mode    = (T_BAT_percent_cops_mode)mode;
  cops.format  = (T_BAT_percent_cops_format)format;
  cops.service = (T_BAT_percent_cops_service)svrStatus;
  cops.c_oper  = strlen(op);
  cops.v_oper  = cops.c_oper ? TRUE : FALSE;
  
  if (cops.v_oper)
  {
    memcpy(cops.oper, op, cops.c_oper);
  }
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCREG     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCREG(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_creg creg;
 
  TRACE_FUNCTION ("qBAT_PercentCREG()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_CREG;
  resp.response.ptr_que_percent_creg = &creg;

  ret =(T_ACI_BAT_RSLT) qAT_PercentCREG((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (T_ACI_CREG_STAT*)&creg.stat,
                        (USHORT*)&creg.lac,
                        (USHORT*)&creg.ci,
                        (T_ACI_P_CREG_GPRS_IND*)&creg.gprs_ind,
                         (U8*)&creg.rt);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

#ifdef TI_PS_FF_AT_CMD_WS46
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusWS46        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusWS46(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_plus_ws46_n mode = cmd->params.ptr_set_plus_ws46->n;

  TRACE_FUNCTION ("sBAT_PlusWS46()");

  ret =(T_ACI_BAT_RSLT) sAT_PlusWS46((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_WS46_MOD)mode);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusWS46        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusWS46(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ws46 ws46;

  TRACE_FUNCTION ("qBAT_PlusWS46()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_WS46;
  resp.response.ptr_que_plus_ws46 = &ws46;

  ret = (T_ACI_BAT_RSLT) qAT_PlusWS46((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_WS46_MOD *)&ws46.n);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}
#endif /* TI_PS_FF_AT_CMD_WS46 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCHPL     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCHPL(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_OPER_NTRY oper;
  UBYTE len;
    
  TRACE_FUNCTION ("sBAT_PercentCHPL()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PercentCHPL((T_ACI_CMD_SRC) src_infos_psi->srcId,&oper);

  /*
  *   If the command completes, we must send a response.
  */
  if (ret==ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_percent_chpl chpl_data;

    resp.ctrl_response=BAT_RES_SET_PERCENT_CHPL;
    resp.response.ptr_set_percent_chpl=&chpl_data;

    chpl_data.type=BAT_P_CHPL_TYPE_FROM_IMS;

    switch (cmd->params.ptr_set_percent_chpl->oprfrmt)
    {
      case BAT_P_CHPL_OPRFRMT_LONG:
        len=strlen(oper.longName);

        if (len<=BAT_MAX_CHPL_OPER_LEN)
        {
          memcpy(chpl_data.oper,oper.longName,len);
          chpl_data.c_oper=(U8)len;
        }
        else
        {
          return(ACI_BAT_FAIL);
        }
        break;

      case BAT_P_CHPL_OPRFRMT_SHORT:
        len=strlen(oper.shrtName);

        if (len<=BAT_MAX_CHPL_OPER_LEN)
        {
          memcpy(chpl_data.oper,oper.shrtName,len);
          chpl_data.c_oper=(U8)len;
        }
        else
        {
          return(ACI_BAT_FAIL);
        }
        break;
 
      case BAT_P_CHPL_OPRFRMT_NUMERIC:
        /*
        *   Representation of the numeric format as in the ATI.
        */
        if ((oper.mnc & 0x00F)==0xF)
          sprintf((char *)chpl_data.oper, "%03x,%02x",oper.mcc, oper.mnc >> 4);
        else
          sprintf((char *)chpl_data.oper, "%03x,%03x",oper.mcc, oper.mnc);

        chpl_data.c_oper=(U8)strlen((char *)chpl_data.oper);
        break;

      default:
        return(ACI_BAT_FAIL);
    }

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCTZU        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCTZU(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusCTZU()");

  ret =(T_ACI_BAT_RSLT) sAT_PlusCTZU ((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CTZU_MODE)cmd->params.ptr_set_plus_ctzu->onoff);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCTZU        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCTZU(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ctzu ctzu;
  
  TRACE_FUNCTION ("qBAT_PlusCTZU()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CTZU;
  resp.response.ptr_que_plus_ctzu = &ctzu;
  ret =(T_ACI_BAT_RSLT) qAT_PlusCTZU((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CTZU_MODE*)&ctzu.onoff);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCWUP     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCWUP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PercentCWUP()");

  ret =(T_ACI_BAT_RSLT) sAT_PercentCWUP ((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CWUP_TYPE)cmd->params.ptr_set_percent_cwup->type);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCTZV     |
+--------------------------------------------------------------------+

  PURPOSE : %CTZV command mode (time and date report)
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCTZV (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentCTZV()");

  ret =(T_ACI_BAT_RSLT) sAT_PercentCTZV ((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                         (T_ACI_PCTZV_MODE)cmd->params.ptr_set_percent_ctzv->mode);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCNIV     |
+--------------------------------------------------------------------+

  PURPOSE : %CNIV set command mode (Network name report)
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCNIV (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentCNIV()");

  ret =(T_ACI_BAT_RSLT) sAT_PercentCNIV ((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                         (T_ACI_CNIV_MODE)cmd->params.ptr_set_percent_cniv->mode);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCNIV     |
+--------------------------------------------------------------------+

  PURPOSE : %CNIV query command mode
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCNIV (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cniv cniv;

  TRACE_FUNCTION ("qBAT_PercentCNIV()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CNIV;
  resp.response.ptr_que_percent_cniv = &cniv;

  ret =(T_ACI_BAT_RSLT) qAT_PercentCNIV ((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                         (T_ACI_CNIV_MODE*)&cniv.mode);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCIND        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCIND(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PlusCIND()");
  
  ret =(T_ACI_BAT_RSLT) sAT_PlusCIND((T_ACI_CMD_SRC)src_infos_psi->srcId,
                      (T_ACI_CIND_SIGNAL_TYPE)cmd->params.ptr_set_plus_cind->signal_ctrl,
                      (T_ACI_CIND_SMSFULL_TYPE)cmd->params.ptr_set_plus_cind->smsfull_ctrl);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCIND        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCIND(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cind cind;

  TRACE_FUNCTION ("qBAT_PlusCIND()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CIND;
  resp.response.ptr_que_plus_cind = &cind;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCIND((T_ACI_CMD_SRC)src_infos_psi->srcId,
               (T_ACI_CIND_SIGNAL_TYPE*)&cind.signal, 
               (T_ACI_CIND_SMSFULL_TYPE*)&cind.smsfull); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMER        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMER(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  T_ACI_CMER_MODE_TYPE sCmerModeSettings = CMER_MODE_INVALID;
  T_ACI_CMER_IND_TYPE  sCmerIndicationSettings = CMER_INDICATOR_INVALID;
  T_ACI_CMER_BFR_TYPE  sCmerBfrSettings = CMER_BFR_INVALID;

  TRACE_FUNCTION ("sBAT_PlusCMER()");
  
  switch (cmd->params.ptr_set_plus_cmer->ind_option)
  {
  case BAT_CMER_IND_OPTION_LOWER:
    ret =(T_ACI_BAT_RSLT) sAT_PlusCMER((T_ACI_CMD_SRC)src_infos_psi->srcId,
                       sCmerModeSettings,
                       sCmerIndicationSettings,
                       sCmerBfrSettings);
    break;

  case BAT_CMER_IND_OPTION_GREATER_EQUAL:
    ret = (T_ACI_BAT_RSLT)sAT_PlusCMER((T_ACI_CMD_SRC)src_infos_psi->srcId,
                       sCmerModeSettings,
                       sCmerIndicationSettings,
                       sCmerBfrSettings);
    break;

  case BAT_CMER_IND_OPTION_NOT_PRESENT:
  default:
    return ACI_BAT_FAIL;
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCMER        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMER(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cmer cmer;

  T_ACI_CMER_MODE_TYPE sCmerModeSettings;
  T_ACI_CMER_IND_TYPE  sCmerIndicationSettings;
  T_ACI_CMER_BFR_TYPE  sCmerBfrSettings;

  TRACE_FUNCTION ("qBAT_PlusCMER()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CMER;
  resp.response.ptr_que_plus_cmer = &cmer;

  ret =(T_ACI_BAT_RSLT) qAT_PlusCMER((T_ACI_CMD_SRC)src_infos_psi->srcId,
                     &sCmerModeSettings,
                     &sCmerIndicationSettings,
                     &sCmerBfrSettings);

  switch (cmd->params.ptr_que_plus_cmer->bat_dummy)
  {
  default:
    break;
  }
  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}


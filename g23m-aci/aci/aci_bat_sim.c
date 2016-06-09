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
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_bat.h"
#include "psa.h"
#include "cmh.h"
#include "audio.h"
#include "aci_ext_pers.h"
#include "aci_slock.h"



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPIN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPIN        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char pinOld [MAX_PWD_LENGTH]={0};  
  char pinNew [MAX_PWD_LENGTH]={0};
  char* p_pinNew;
  UBYTE slockBlocked = 0;
  
  TRACE_FUNCTION ("sBAT_PlusCPIN()");

  p_pinNew = pinNew;
  memset(pinOld,0,MAX_PWD_LENGTH);
   
  memcpy(pinOld,cmd->params.ptr_set_plus_cpin->pin,cmd->params.ptr_set_plus_cpin->c_pin);
  if(cmd->params.ptr_set_plus_cpin->v_newpin)
  {
    memcpy(pinNew,cmd->params.ptr_set_plus_cpin->newpin,cmd->params.ptr_set_plus_cpin->c_newpin);
  }
  else
  {
    p_pinNew = NULL;
  }
  #ifdef SIM_PERS
  slockBlocked = AciSLockShrd.blocked;
  #endif
  ret = (T_ACI_BAT_RSLT)sAT_PlusCPIN ((T_ACI_CMD_SRC)src_infos_psi->srcId,pinOld,p_pinNew);

  switch((T_ACI_RETURN)ret)
  {
    case(AT_EXCT):
    case (AT_FAIL):
    {
      break;
    }
    case(AT_BUSY):
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_SimBusy);
      break;
    }
    case(AT_CMPL):
    {
      if (!slockBlocked)
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
      }
      break;
    }
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPIN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPIN        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CPIN_RSLT code;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cpin que_cpin_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCPIN()");
  
  ret=(T_ACI_BAT_RSLT) qAT_PlusCPIN ((T_ACI_CMD_SRC)src_infos_psi->srcId,&code);  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CPIN; /*PIN status is requested*/ 
  resp.response.ptr_que_plus_cpin = &que_cpin_buffer;
  
  switch ((T_ACI_RETURN)ret)
  {
    case(AT_CMPL):
    {
      resp.response.ptr_que_plus_cpin->code = (T_BAT_plus_cpin_code)code;
      aci_bat_send(src_infos_psi,&resp);
      break;
    }
    case(AT_EXCT):
    default :
    {
      break;
    }
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPOL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cpol_format format = BAT_CPOL_FORMAT_NOT_PRESENT ;
  SHORT           index = (SHORT)BAT_PARAMETER_NOT_PRESENT;
  char            op[20] = {0};
  char* p_op;
  
  TRACE_FUNCTION ("sBAT_PlusCPOL()");
  
  p_op = op;
  
  index = cmd->params.ptr_set_plus_cpol->index;
  format = cmd->params.ptr_set_plus_cpol->format;
  if(cmd->params.ptr_set_plus_cpol->v_oper)
  {
    memcpy(op,cmd->params.ptr_set_plus_cpol->oper,cmd->params.ptr_set_plus_cpol->c_oper);
  }
  else
  {
    p_op = NULL;
  }  
 
  ret=(T_ACI_BAT_RSLT)sAT_PlusCPOL((T_ACI_CMD_SRC)src_infos_psi->srcId,index,(T_ACI_CPOL_FRMT)format,p_op,
                   (SHORT)BAT_PARAMETER_NOT_PRESENT,CPOL_MOD_NotPresent);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCPOL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  SHORT           lastIdx=0,usdNtry=0;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_cpol tst_cpol_buffer;
  
  TRACE_FUNCTION ("tBAT_PlusCPOL()");
  resp.ctrl_response = BAT_RES_TST_PLUS_CPOL;
  resp.response.ptr_tst_plus_cpol = &tst_cpol_buffer;  

  ret =(T_ACI_BAT_RSLT) tAT_PlusCPOL((T_ACI_CMD_SRC)src_infos_psi->srcId,&lastIdx,&usdNtry);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_tst_plus_cpol->index1 = 1;
    resp.response.ptr_tst_plus_cpol->index2 = lastIdx;
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPOL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPOL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  //T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CPOL_LST  plmnSelLst;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cpol que_cpol_buffer;  
  SHORT           startIdx=0,lastIdx=0;
  UBYTE           idx;
  BOOL            loop;
  
  TRACE_FUNCTION ("qBAT_PlusCPOL()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CPOL;
  resp.response.ptr_que_plus_cpol = &que_cpol_buffer;

  switch (qAT_PlusCPOL ((T_ACI_CMD_SRC) src_infos_psi->srcId,1, &lastIdx,&plmnSelLst[0],CPOL_MOD_NotPresent))
  {
    case AT_CMPL:
    {
      loop = TRUE;
      do
      {
        if( lastIdx EQ (SHORT)BAT_PARAMETER_NOT_PRESENT )
        {
          break;
        }
        startIdx = lastIdx+1;

        for( idx=0; idx < MAX_OPER; idx++ )
        {
          if( plmnSelLst[idx].index EQ (SHORT)BAT_PARAMETER_NOT_PRESENT )
          {
            loop = FALSE;
            break;
          }
          resp.response.ptr_que_plus_cpol->format = (T_BAT_plus_cpol_format)plmnSelLst[idx].format;
          resp.response.ptr_que_plus_cpol->index = (U8)plmnSelLst[idx].index;
          resp.response.ptr_que_plus_cpol->c_oper = strlen(plmnSelLst[idx].oper);
          memcpy(resp.response.ptr_que_plus_cpol->oper,plmnSelLst[idx].oper,strlen(plmnSelLst[idx].oper));
          aci_bat_send(src_infos_psi,&resp);
        }

        if( qAT_PlusCPOL((T_ACI_CMD_SRC)src_infos_psi->srcId, startIdx, &lastIdx, &plmnSelLst[0],
                         CPOL_MOD_NotPresent) EQ AT_FAIL OR !loop)
        {
          break;
        }
      }
      while( loop );
      return ACI_BAT_CMPL;
    }

    case AT_EXCT:
    {
      return ACI_BAT_EXCT;
    }  

    default:
    {
      return ACI_BAT_FAIL;
    }
  }  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCNUM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNUM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PlusCNUM()");

  if(qAT_PlusCNUM ((T_ACI_CMD_SRC) src_infos_psi->srcId, CNUM_MOD_NewRead) EQ AT_EXCT)
  {
    return ACI_BAT_EXCT;
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCFUN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCFUN        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cfun_fun  fun = BAT_CFUN_FUN_NOT_PRESENT;
  T_BAT_plus_cfun_rst rst = BAT_CFUN_RST_NOT_PRESENT;
    
  TRACE_FUNCTION ("sBAT_PlusCFUN()");

  /*
  *   This relies on T_BAT_VAL_plus_cfun_fun being identical to 
  *   T_ACI_CFUN_FUN and T_BAT_VAL_plus_cfun_rst being identical
  *   to T_ACI_CFUN_RST. At the time of writing, they are.
  */
  fun=cmd->params.ptr_set_plus_cfun->fun;
  rst=cmd->params.ptr_set_plus_cfun->rst;

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCFUN((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CFUN_FUN)fun,(T_ACI_CFUN_RST)rst);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCFUN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCFUN (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CFUN_FUN fun;

  TRACE_FUNCTION ("qBAT_PlusCFUN()");

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCFUN((T_ACI_CMD_SRC)src_infos_psi->srcId,&fun);

  /*
  *   If the query completes, we must send a response.
  */
  if (ret==ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_cfun cfun_data;

    resp.ctrl_response=BAT_RES_QUE_PLUS_CFUN;
    resp.response.ptr_que_plus_cfun=&cfun_data;

    /*
    *   This relies on T_BAT_plus_cfun_fun being identical to 
    *   T_ACI_CFUN_FUN.
    */
    cfun_data.fun=(T_BAT_plus_cfun_fun)fun;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCIMI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCIMI        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR imsi[MAX_IMSI_LEN+1];
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cimi set_cimi_buffer;
  
  TRACE_FUNCTION ("sBAT_PlusCIMI()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CIMI ;
  resp.response.ptr_set_plus_cimi = &set_cimi_buffer;
    
  ret =(T_ACI_BAT_RSLT) qAT_PlusCIMI((T_ACI_CMD_SRC)src_infos_psi->srcId, imsi);
  if (ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_set_plus_cimi->c_imsi = strlen(imsi);
    memcpy(resp.response.ptr_set_plus_cimi->imsi,imsi,resp.response.ptr_set_plus_cimi->c_imsi);
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCRSM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRSM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_crsm_command command;
  UBYTE           *data;
  SHORT            fileId     = (SHORT)BAT_PARAMETER_NOT_PRESENT,
                   p1         = (SHORT)BAT_PARAMETER_NOT_PRESENT,
                   p2         = (SHORT)BAT_PARAMETER_NOT_PRESENT,
                   p3         = (SHORT)BAT_PARAMETER_NOT_PRESENT;
    
  TRACE_FUNCTION ("sBAT_PlusCRSM()");
  
  command = cmd->params.ptr_set_plus_crsm->command;
  if(cmd->params.ptr_set_plus_crsm->fileid NEQ (S32)BAT_PARAMETER_NOT_PRESENT)
  {
    fileId = (SHORT)cmd->params.ptr_set_plus_crsm->fileid;
  }
  p1 = cmd->params.ptr_set_plus_crsm->p1;
  p2 = cmd->params.ptr_set_plus_crsm->p2;
  p3 = cmd->params.ptr_set_plus_crsm->p3;
  data = cmd->params.ptr_set_plus_crsm->data;

  ret=(T_ACI_BAT_RSLT)sAT_PlusCRSM(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    (T_ACI_CRSM_CMD)command,
    fileId,
    p1,p2,p3,
    (SHORT)cmd->params.ptr_set_plus_crsm->c_data,
    data);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSIM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSIM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE           *data;
  USHORT           lenData    = 0;
    
  TRACE_FUNCTION ("sBAT_PlusCSIM()");
  
  data = cmd->params.ptr_set_plus_csim->command;
  lenData=(USHORT)cmd->params.ptr_set_plus_csim->c_command;
  
  if (lenData EQ 0)
    return ACI_BAT_FAIL;

  ret =(T_ACI_BAT_RSLT) sAT_PlusCSIM((T_ACI_CMD_SRC) src_infos_psi->srcId, lenData, data );

  return ret;
}

#ifdef TI_PS_FF_AT_P_CMD_ATR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentATR      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentATR      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE phase;
  UBYTE atr_len;
  UBYTE atr_info[MAX_SIM_ATR];
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_atr que_atr_buffer;
  
  TRACE_FUNCTION ("qBAT_PercentATR()");
  resp.ctrl_response = BAT_RES_QUE_PERCENT_ATR ;
  resp.response.ptr_que_percent_atr = &que_atr_buffer;

  ret =(T_ACI_BAT_RSLT) qAT_PercentATR((T_ACI_CMD_SRC) src_infos_psi->srcId, &phase, &atr_len, atr_info );
  if(ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_que_percent_atr->phase = (T_BAT_percent_atr_phase)phase ;
    resp.response.ptr_que_percent_atr->c_atr = atr_len;
    memcpy(resp.response.ptr_que_percent_atr->atr,atr_info,resp.response.ptr_que_percent_atr->c_atr);
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}
#endif /* TI_PS_FF_AT_P_CMD_ATR */  

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentPVRF     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentPVRF     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR            pin[10];
  CHAR            newpin[10];
  CHAR*           p_newpin;
  
  TRACE_FUNCTION ("sBAT_PercentPVRF()");

  p_newpin = newpin;
  memset(pin,0,sizeof(pin));
  
  if (cmd->params.ptr_set_percent_pvrf->type <= BAT_P_PVRF_TYPE_PUK2)/*lint !e685 (Warning : always evaluates to false)*/
  {
    
    memcpy(pin,cmd->params.ptr_set_percent_pvrf->pin,cmd->params.ptr_set_percent_pvrf->c_pin);
    if(cmd->params.ptr_set_percent_pvrf->v_newpin)
    {
      memcpy(newpin,cmd->params.ptr_set_percent_pvrf->newpin,cmd->params.ptr_set_percent_pvrf->c_newpin);
    }
    else
    {
      p_newpin = NULL;
    }
    ret =(T_ACI_BAT_RSLT) sAT_PercentPVRF ((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_PVRF_TYPE)cmd->params.ptr_set_percent_pvrf->type, pin, p_newpin);
    return(ret);
  }
  else
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentPVRF     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentPVRF     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_pvrf que_pvrf_buffer;
  SHORT           pn2cnt = 0;
  SHORT           pk1cnt = 0;
  SHORT           pk2cnt = 0;
  T_BAT_percent_pvrf_ps1 ps1 = BAT_P_PVRF_PS1_NOT_PRESENT ;
  T_BAT_percent_pvrf_ps2 ps2 = BAT_P_PVRF_PS2_NOT_PRESENT ;
  
  TRACE_FUNCTION ("qBAT_PercentPVRF()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_PVRF ; 
  resp.response.ptr_que_percent_pvrf = &que_pvrf_buffer;
  
  resp.response.ptr_que_percent_pvrf->pn2cnt = pn2cnt;
  resp.response.ptr_que_percent_pvrf->pk1cnt = pk1cnt;
  resp.response.ptr_que_percent_pvrf->pk2cnt = pk2cnt;
  resp.response.ptr_que_percent_pvrf->ps1 = ps1;
  resp.response.ptr_que_percent_pvrf->ps2 = ps2;

  aci_bat_send(src_infos_psi,&resp);
  return (ACI_BAT_CMPL);
 }

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCUST     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCUST     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{    
  TRACE_FUNCTION ("sBAT_PercentCUST()");
  
  switch(cmd->params.ptr_set_percent_cust->mode)
  {
    case(BAT_P_CUST_MODE_NORMAL):
    case(BAT_P_CUST_MODE_BEHAVIOUR_1):
    {
      if (sAT_PercentCUST((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_CUST_MOD)(cmd->params.ptr_set_percent_cust->mode)) EQ AT_FAIL)
      {
        return (ACI_BAT_FAIL);
      }
      break;
    }
    default:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
      return (ACI_BAT_FAIL);
    }  
  }
  return ACI_BAT_CMPL;
  }

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCUST     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCUST     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_cust_mode   mode;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cust que_cust_buffer;
    
  TRACE_FUNCTION ("qBAT_PercentCUST()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_CUST; 
  resp.response.ptr_que_percent_cust = &que_cust_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentCUST((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_CUST_MOD *)&mode);
  if (ret NEQ ACI_BAT_CMPL)
  {
    return (ACI_BAT_FAIL);
  }
  resp.response.ptr_que_percent_cust->mode = mode;
  aci_bat_send(src_infos_psi,&resp);
  return(ret);
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSATCC    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATCC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                         T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
    
  TRACE_FUNCTION ("sBAT_PercentSATCC()");

  ret = (T_ACI_BAT_RSLT)sAT_PercentSATCC((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_SAT_CC_MOD)cmd->params.ptr_set_percent_satcc->mode);

  return ret;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentSATCC    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSATCC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                         T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT              ret = ACI_BAT_FAIL;
  T_BAT_percent_satcc_mode    mode;
  T_BAT_cmd_response          resp;
  T_BAT_res_que_percent_satcc que_satcc_buffer;

  TRACE_FUNCTION ("qBAT_PercentSATCC()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_SATCC;

  resp.response.ptr_que_percent_satcc = &que_satcc_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentSATCC((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_SAT_CC_MOD *)&mode);

  if(ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_que_percent_satcc->mode = mode;
    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusEFRSLT      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentEFRSLT   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_EFRSLT_RES result;
  T_ACI_BAT_RSLT ret;
    
  TRACE_FUNCTION ("sBAT_PercentEFRSLT()");

  /*
  *   This relies on T_BAT_percent_efrslt_result being identical to 
  *   T_ACI_EFRSLT_RES.
  */
  result=(T_ACI_EFRSLT_RES)cmd->params.ptr_set_percent_efrslt->result;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PercentEFRSLT((T_ACI_CMD_SRC)src_infos_psi->srcId,result);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSIMEF      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSIMEF   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_SIMEF_MODE mode;
  T_ACI_BAT_RSLT ret;
    
  TRACE_FUNCTION ("sBAT_PercentSIMEF()");

  /*
  *   This relies on T_BAT_percent_simef_mode being identical to 
  *   T_ACI_SIMEF_MODE.
  */
  mode=(T_ACI_SIMEF_MODE)cmd->params.ptr_set_percent_simef->mode;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PercentSIMEF((T_ACI_CMD_SRC)src_infos_psi->srcId,mode);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentSIMEF    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSIMEF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                         T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT              ret = ACI_BAT_FAIL;
  T_BAT_percent_simef_mode    mode;
  T_BAT_cmd_response          resp;
  T_BAT_res_que_percent_simef que_simef_buffer;

  TRACE_FUNCTION ("qBAT_PercentSIMEF()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_SIMEF;

  resp.response.ptr_res_que_percent_simef = &que_simef_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentSIMEF((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_SIMEF_MODE *)&mode);

  if(ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_res_que_percent_simef->mode = mode;
    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

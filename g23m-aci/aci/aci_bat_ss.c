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
#include "aci_cmh.h" 
#include "ati_cmd.h" 
#include "aci_cmd.h" /* prototypes of helper functions */ 
#include "psa.h"
#include "cmh.h"
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "aci_bat.h"
#include "aci_bat_err.h" /*prototypes of err functions for BAT*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCTFR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCTFR        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     type;
  T_ACI_TOA    *p_type;
  T_ACI_TOS     satype;
  T_ACI_TOS    *p_satype;
  //CHAR         *subadr = subBuf;
  char          subBuf[MAX_SUBADDR_LEN]={0};
  char*         p_subBuf;
  SHORT         toa_oct = 0;
  SHORT         tos_oct = 0;
  CHAR          numBuf[MAX_B_SUBSCR_NUM_LEN];
  
  TRACE_FUNCTION ("sBAT_PlusCTFR()");

  p_type   = &type;
  p_satype = &satype;  
  p_subBuf = subBuf;
  memset( numBuf, 0, sizeof(numBuf));
  
  memcpy(numBuf,cmd->params.ptr_set_plus_ctfr->number,cmd->params.ptr_set_plus_ctfr->c_number);

  if ( cmd->params.ptr_set_plus_ctfr->type EQ (S16)BAT_PARAMETER_NOT_PRESENT )
  {
    p_type = NULL;
  }
  else
  {
    toa_oct = cmd->params.ptr_set_plus_ctfr->type;
  }
  if ( cmd->params.ptr_set_plus_ctfr->satype EQ (S16)BAT_PARAMETER_NOT_PRESENT )
  {
    p_satype = NULL;
  }
  else
  {
    tos_oct = cmd->params.ptr_set_plus_ctfr->satype;
  }
  /* Type Of Address present */
  type = toa_demerge (toa_oct);
  if (type.ton < 0 OR type.npi < 0)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  satype = tos_demerge (tos_oct);
  if (satype.tos < 0 OR satype.oe < 0)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
     
  if(cmd->params.ptr_set_plus_ctfr->v_subaddr)
  {
    memcpy(subBuf,cmd->params.ptr_set_plus_ctfr->subaddr,cmd->params.ptr_set_plus_ctfr->c_subaddr);
  }
  else
  {
   p_subBuf = NULL;
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCTFR ((T_ACI_CMD_SRC)src_infos_psi->srcId, numBuf, p_type, p_subBuf, p_satype); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCCFC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCFC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_ccfc_reason       reason;
  T_BAT_plus_ccfc_mode       mode;
  T_ACI_TOA           type;
  T_ACI_TOA          *p_type;
  T_ACI_TOS           satype;
  T_ACI_TOS          *p_satype;
  T_BAT_plus_ccfc_bearer_class         class_type=BAT_CCFC_CLASS_NOT_PRESENT;
  //CHAR               *subadr = subBuf;
  char           subBuf[MAX_SUBADDR_LEN]={0};
  char*          p_subBuf; 
  T_BAT_plus_ccfc_time             time=BAT_CCFC_TIME_NOT_PRESENT;
  SHORT               toa_oct=0;
  SHORT               tos_oct=0;
  CHAR                numBuf[MAX_B_SUBSCR_NUM_LEN];
  CHAR*               p_numBuf;
  
  TRACE_FUNCTION ("sBAT_PlusCCFC()");

  p_type=&type;
  p_satype=&satype;  
  p_subBuf = subBuf;
  p_numBuf = numBuf;
  memset( numBuf, 0, sizeof(numBuf));
  
  reason = cmd->params.ptr_set_plus_ccfc->reason;
  mode = cmd->params.ptr_set_plus_ccfc->mode;
  toa_oct = cmd->params.ptr_set_plus_ccfc->type;
  tos_oct = cmd->params.ptr_set_plus_ccfc->satype;
  class_type = cmd->params.ptr_set_plus_ccfc->bearer_class;
  time = cmd->params.ptr_set_plus_ccfc->time;
    
  if (mode EQ BAT_CCFC_MODE_QUERY)                     /*query mode*/
  {
    ret = (T_ACI_BAT_RSLT)qAT_PlusCCFC((T_ACI_CMD_SRC)src_infos_psi->srcId,
		      (T_ACI_CCFC_RSN)reason,(T_ACI_CLASS)class_type);
    switch ((T_ACI_RETURN)ret)
    {
      case AT_EXCT:
      case AT_BUSY:
      case AT_FAIL:
      {
        return ret;
      }
      default:
      {
        return (ACI_BAT_FAIL);
      }
    }
  }
  if(cmd->params.ptr_set_plus_ccfc->v_subaddr)
  {
    memcpy(subBuf,cmd->params.ptr_set_plus_ccfc->subaddr,cmd->params.ptr_set_plus_ccfc->c_subaddr);
  }
  else
  {
    p_subBuf = NULL;
  }

  if ( cmd->params.ptr_set_plus_ccfc->type EQ (S16)BAT_PARAMETER_NOT_PRESENT )
  {
    p_type = NULL;
  }  
  else
  {
    toa_oct = cmd->params.ptr_set_plus_ccfc->type;
  }  
    
  if ( cmd->params.ptr_set_plus_ccfc->satype EQ (S16)BAT_PARAMETER_NOT_PRESENT )
  {
    p_satype = NULL;
  }
  else
  {
    tos_oct = cmd->params.ptr_set_plus_ccfc->satype;
  }
    /* Type Of Address present */
  type = toa_demerge (toa_oct);
  if (type.ton < 0 OR type.npi < 0)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  
  satype = tos_demerge (tos_oct);
  if (satype.tos < 0 OR satype.oe < 0)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }

  if(cmd->params.ptr_set_plus_ccfc->v_number)
  {
    memcpy(numBuf,cmd->params.ptr_set_plus_ccfc->number,cmd->params.ptr_set_plus_ccfc->c_number);
  }
  else
  {
    p_numBuf = NULL; 	 
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCCFC((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CCFC_RSN)reason,
  	     (T_ACI_CCFC_MOD)mode,p_numBuf,p_type,(T_ACI_CLASS)class_type,p_subBuf,p_satype,time); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCLCK        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLCK        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_clck_fac       fac_num;
  T_BAT_plus_clck_mode       mod;
  T_BAT_bearer_class      class_type = BAT_CLASS_NOT_PRESENT;
  T_BAT_cmd_response resp;
  T_ACI_CLSSTAT       clsStat;
  char                passwd[MAX_PWD_LENGTH]={0};
    
  TRACE_FUNCTION ("sBAT_PlusCLCK()");
  
  fac_num = cmd->params.ptr_set_plus_clck->fac;
  mod = cmd->params.ptr_set_plus_clck->mode;
  class_type = cmd->params.ptr_set_plus_clck->bearer_class;

    
  if (mod EQ BAT_CLCK_MODE_QUERY)                     /*query mode*/
  {
    T_BAT_res_set_plus_clck clck;
    ret=(T_ACI_BAT_RSLT)qAT_PlusCLCK((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_FAC)fac_num,
		 (T_ACI_CLASS)class_type,&clsStat);
    resp.ctrl_response = BAT_RES_SET_PLUS_CLCK;
    resp.response.ptr_set_plus_clck = &clck;
    resp.response.ptr_set_plus_clck->status = BAT_CLCK_STATUS_NOT_ACTIVE ; 
    resp.response.ptr_set_plus_clck->bearer_class = BAT_CLASS_NOT_PRESENT;
     
    if(ret EQ ACI_BAT_CMPL)
    {
      switch(clsStat.status)
      {
        case STATUS_Active:
          resp.response.ptr_set_plus_clck->status = BAT_CLCK_STATUS_ACTIVE;
          break;
        case STATUS_NotActive:
          resp.response.ptr_set_plus_clck->status = BAT_CLCK_STATUS_NOT_ACTIVE;
          break;        
      }

      if(clsStat.class_type NEQ CLASS_NotPresent)
      {
        resp.response.ptr_set_plus_clck->bearer_class = (T_BAT_bearer_class)clsStat.class_type;        
      }
      aci_bat_send(src_infos_psi , &resp);
      return (ACI_BAT_CMPL);
    }
  }
  else                              /*set mode*/
  {
    if(cmd->params.ptr_set_plus_clck->v_passwd)
    {
      memcpy(passwd,cmd->params.ptr_set_plus_clck->passwd,cmd->params.ptr_set_plus_clck->c_passwd);
    }
    ret = (T_ACI_BAT_RSLT)sAT_PlusCLCK((T_ACI_CMD_SRC)src_infos_psi->srcId,
		(T_ACI_FAC)fac_num,(T_ACI_CLCK_MOD)mod,passwd,(T_ACI_CLASS)class_type);
  }

  switch((T_ACI_RETURN)ret)
  {
    case AT_EXCT:
    case AT_FAIL:
    {
      break;
    }  
    case(AT_BUSY):
    {
      TRACE_EVENT("sBAT_PlusCLCK(): ME is busy");
      break;
    }  
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPWD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPWD        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cpwd_fac      fac_num;
  CHAR                oldpwd[MAX_PWD_LENGTH]={0};
  CHAR                newpwd[MAX_PWD_LENGTH]={0};
      
  TRACE_FUNCTION ("sBAT_PlusCPWD()");
 
  fac_num = cmd->params.ptr_set_plus_cpwd->fac;
  memcpy(oldpwd, cmd->params.ptr_set_plus_cpwd->oldpwd,cmd->params.ptr_set_plus_cpwd->c_oldpwd);
  memcpy(newpwd, cmd->params.ptr_set_plus_cpwd->newpwd,cmd->params.ptr_set_plus_cpwd->c_newpwd);
  ret = (T_ACI_BAT_RSLT)sAT_PlusCPWD((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_FAC)fac_num,oldpwd,newpwd); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCUSD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCUSD        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_USSD_DATA ussd;
  T_ACI_USSD_DATA *ussd_ptr;
  T_BAT_cmd_set_plus_cusd *cusd;

  TRACE_FUNCTION ("sBAT_PlusCUSD()");

  /*
  *   Get a pointer to the BAT structure for convenience.
  */
  cusd=cmd->params.ptr_set_plus_cusd;

  ussd_ptr=NULL;

  if (cusd->v_str)
  {
    USHORT len;

    utl_ussdDtaFromTe (
      (UBYTE*)cusd->str,
      cusd->c_str,
      (UBYTE*)ussd.data,
      &len,
#ifdef REL99
      sizeof(ussd.data),
#endif /* REL99 */
      (UBYTE)cusd->dcs);
  
    if (len>MAX_USSD_LEN)
    {
      /*
      *   The USSD string is too big for the ACI structure. This should
      *   not happen, and if it does means that we have overwritten
      *   memory.
      */
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_TxtToLong);
      return(ACI_BAT_FAIL);
    }

    ussd.len=(UBYTE)len;
    ussd_ptr=&ussd;
  }

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCUSD(
        (T_ACI_CMD_SRC)src_infos_psi->srcId,
      ussd_ptr,
      (SHORT)cusd->dcs);

  return(ret);
}

#ifdef SIM_PERS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentMEPD     |
+--------------------------------------------------------------------+

  PURPOSE : Mobile Equipment Personalization Data (SIM LOCK)
            The set command has query semantic
*/

GLOBAL T_ACI_BAT_RSLT sBAT_PercentMEPD (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_RETURN ret;
  T_SUP_INFO   sup_info;

  TRACE_FUNCTION("sBAT_PercentMEPD()");

  memset(&sup_info, 0, sizeof(T_SUP_INFO));
  sup_info.infoType =(T_SUP_INFO_TYPE)cmd->params.ptr_set_percent_mepd->sup_info_type;
  
  ret = qAT_PercentMEPD((T_ACI_CMD_SRC)src_infos_psi->srcId, &sup_info);

  if (ret EQ AT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_percent_mepd mepd;

    resp.ctrl_response = BAT_RES_SET_PERCENT_MEPD;
    resp.response.ptr_set_percent_mepd = &mepd;
    mepd.datavalue = sup_info.datavalue;
    
    aci_bat_send(src_infos_psi, &resp);
    return(ACI_BAT_CMPL);
  }
  
  return((T_ACI_BAT_RSLT)ret);
}
#endif /* SIM_PERS */

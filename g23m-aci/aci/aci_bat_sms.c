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
#include "ati_cmd.h" 
#include "aci_cmd.h" 
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "aci_bat.h"
#include "psa.h"
#include "psa_sms.h"
#include "cmh.h"

EXTERN T_SMS_SHRD_PRM smsShrdPrm;

EXTERN SHORT  cmhSMS_getPrfRge ( void );

EXTERN T_ATI_RSLT txt_rdmode_to_smsrdmode(char *txt_rdmode);



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSMS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PlusCSMS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCSMS((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CSMS_SERV)cmd->params.ptr_set_plus_csms->service);
  
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSMS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CSMS_SERV service;
  T_ACI_CSMS_SUPP mt;
  T_ACI_CSMS_SUPP mo;
  T_ACI_CSMS_SUPP bm;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_csms que_csms_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCSMS()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CSMS; 
  resp.response.ptr_que_plus_csms = &que_csms_buffer;
  ret = (T_ACI_BAT_RSLT)qAT_PlusCSMS((T_ACI_CMD_SRC)src_infos_psi->srcId,&service,&mt,&mo,&bm);
  switch(ret)
  {
  case ACI_BAT_CMPL :
    resp.response.ptr_que_plus_csms->service = (T_BAT_plus_csms_service)service;
    resp.response.ptr_que_plus_csms->mt = (T_BAT_plus_csms_mt)mt;
    resp.response.ptr_que_plus_csms->mo = (T_BAT_plus_csms_mo)mo;
    resp.response.ptr_que_plus_csms->bm = (T_BAT_plus_csms_bm)bm;
    aci_bat_send(src_infos_psi,&resp);
   /*lint -fallthrough*/
  default :
    return (ret);
  }  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPMS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cpms_mem1      mem1;    
  T_BAT_plus_cpms_mem2      mem2 = BAT_CPMS_MEM2_NOT_PRESENT;
  T_BAT_plus_cpms_mem3      mem3 = BAT_CPMS_MEM3_NOT_PRESENT;  
  
  TRACE_FUNCTION ("sBAT_PlusCPMS()");

  mem1 = cmd->params.ptr_set_plus_cpms->mem1;  
  mem2 = cmd->params.ptr_set_plus_cpms->mem2;
  mem3 = cmd->params.ptr_set_plus_cpms->mem3;  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCPMS ( (T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_SMS_STOR)mem1, (T_ACI_SMS_STOR)mem2, (T_ACI_SMS_STOR)mem3);
  
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPMS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPMS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("qBAT_PlusCPMS()");
  ret = (T_ACI_BAT_RSLT)qAT_PlusCPMS ( (T_ACI_CMD_SRC)src_infos_psi->srcId );
  /* ..............RECEHECK............. */
  /* Needs to be looked into. The cmd params are not filled thru this function call
  see implementation of qAT_plusCPMS, Needs to be rewritten */

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSCA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     tosca;
  T_ACI_TOA    *p_tosca;
  CHAR          sca[MAX_SMS_NUM_LEN];
  
  TRACE_FUNCTION ("sBAT_PlusCSCA()");

  memset (sca, 0, sizeof(sca));
  p_tosca=&tosca;

  if(cmd->params.ptr_set_plus_csca->tosca EQ (S16)BAT_PARAMETER_NOT_PRESENT)
  {
    p_tosca = NULL;
  }
  else
  {
    tosca=toa_demerge(cmd->params.ptr_set_plus_csca->tosca);
    if (tosca.ton < 0 OR tosca.npi < 0)
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
      return (ACI_BAT_FAIL);
    }
  }
  memcpy(sca,cmd->params.ptr_set_plus_csca->sca,cmd->params.ptr_set_plus_csca->c_sca);  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCSCA((T_ACI_CMD_SRC)src_infos_psi->srcId,sca,p_tosca);

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSCA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     tosca;
  SHORT         octet=0;
  CHAR          sca[MAX_SMS_NUM_LEN];
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_csca que_csca_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCSCA()");
  ret = (T_ACI_BAT_RSLT)qAT_PlusCSCA ((T_ACI_CMD_SRC)src_infos_psi->srcId,sca,&tosca);
  resp.ctrl_response = BAT_RES_QUE_PLUS_CSCA; 
  resp.response.ptr_que_plus_csca = &que_csca_buffer;
  switch(ret)
  {
  case ACI_BAT_CMPL :
    octet=toa_merge(tosca);
    resp.response.ptr_que_plus_csca->tosca = (U8)octet;
    resp.response.ptr_que_plus_csca->c_sca = strlen(sca);
    /* Are we sure the length is strlen(sca)? */
    memcpy(resp.response.ptr_que_plus_csca->sca,sca,strlen(sca));
    aci_bat_send(src_infos_psi,&resp);
  /*lint -fallthrough*/
  default :
    return (ret);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSCB        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCB        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  USHORT *p_mids;
  UBYTE  *p_dcss;
  
  TRACE_FUNCTION ("sBAT_PlusCSCB()");

  if(cmd->params.ptr_set_plus_cscb->v_mids)
  {
    p_mids = cmd->params.ptr_set_plus_cscb->mids;
  }
  else
  {
    p_mids = NULL;
  }
  if(cmd->params.ptr_set_plus_cscb->v_dcss)
  {
    p_dcss = cmd->params.ptr_set_plus_cscb->dcss;
  }
  else
  {
    p_dcss = NULL;
  }
  ret=(T_ACI_BAT_RSLT)sAT_PlusCSCB((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CSCB_MOD)cmd->params.ptr_set_plus_cscb->mode, p_mids, p_dcss);

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSCB        |
+--------------------------------------------------------------------+

  PURPOSE : to query the cell broadcast channels, we pass to qAT_PlusCSCB()
            two arrays (mids and dcss), which are filled in the following way:
            - two consecutive array entries form one logical information
            - when both entries have the same value, then it is a single channel
            - when both entries have different values, then it is a range of channels.
            mids is more or less a copy of EF CBMIR of the SIM. refer to 11.11
            A string based response +CSCB: 0,"2,3,4-6,7,922-1003","0-3,5"
            is on binary level as follow:
            mode = BAT_CSCB_MODE_ACCEPT; <=== 0 = accepted
            mids[0] = 0x0002;
            mids[1] = 0x0002;
            mids[2] = 0x0003;
            mids[3] = 0x0003;
            mids[4] = 0x0004; <=== range lower bound
            mids[5] = 0x0006; <=== range upper bound
            mids[6] = 0x0007;
            mids[7] = 0x0007;
            mids[8] = 0x039A; <=== range lower bound  
            mids[9] = 0x03EB; <=== range upper bound
            dcss[0] = 0x00;
            dcss[1] = 0x03;
            dcss[2] = 0x05;
            dcss[3] = 0x05;
            
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCB        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT       ret = ACI_BAT_FAIL;
  T_BAT_cmd_response   resp;
  T_BAT_res_que_plus_cscb cscb;
  int i;
  
  TRACE_FUNCTION ("qBAT_PlusCSCB()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CSCB; 
  resp.response.ptr_que_plus_cscb = &cscb;

  cscb.c_mids = 0;
  cscb.v_mids = FALSE;
  cscb.c_dcss = 0;
  cscb.v_dcss = FALSE;

  memset(cscb.mids, 0xFF, BAT_MAX_IDENTS);
  memset(cscb.dcss, 0xFF, BAT_MAX_IDENTS);
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCSCB((T_ACI_CMD_SRC)src_infos_psi->srcId,
                    (T_ACI_CSCB_MOD*)&cscb.mode,
                     cscb.mids,
                     cscb.dcss);

  switch(ret)
  {
  case ACI_BAT_CMPL :
    i = 0;
    
    if  ((cscb.mids[0] NEQ 0xFFFF) 
     AND (cscb.mids[1] NEQ 0xFFFF))
    {
      cscb.v_mids = TRUE;
    }
    
    while ((i < BAT_MAX_IDENTS-1)
      AND  (cscb.mids[i]   NEQ 0xFFFF) 
      AND  (cscb.mids[i+1] NEQ 0xFFFF))
    {
      i += 2;
      cscb.c_mids += 2;
    }
    
    i = 0;
    
    if  ((cscb.dcss[0]   NEQ 0xFF)    
     AND (cscb.dcss[1] NEQ 0xFF))
    {
      cscb.v_dcss = TRUE;
    }
    
    while ((i < BAT_MAX_IDENTS-1)
      AND  (cscb.dcss[i]   NEQ 0xFF)
      AND  (cscb.dcss[i+1] NEQ 0xFF))
    {
      i += 2;
      cscb.c_dcss += 2;
    }
    
    aci_bat_send(src_infos_psi,&resp);
  /*lint -fallthrough*/
  default :
    return (ret);
  }
}

#ifdef _SIMULATION_
#pragma message( __TODO__"FF_HOMEZONE is currently a problem for linking" )
#endif
/* #ifdef FF_HOMEZONE */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCBHZ     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
 
  TRACE_FUNCTION ("sBAT_PercentCBHZ()");
  
/* CBHZ_xxx depends on FF_HOMEZONE
  ret = sAT_PercentCBHZ(src_infos_psi->srcId,cmd->params.ptr_set_percent_cbhz->mode
                          ,cmd->params.ptr_set_percent_cbhz->dcs,
                          cmd->params.ptr_set_percent_cbhz->timeout);
  if (ret EQ AT_FAIL)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return (ACI_BAT_FAIL);
  }
  return (ACI_BAT_CMPL); 
*/
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentCBHZ     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
/* CBHZ_xxx depends on FF_HOMEZONE  
  T_BAT_cmd_response resp;
  T_BAT_res_tst_percent_cbhz tst_cbhz_buffer;
  T_BAT_percent_cbhz_mode   mode;
  T_ACI_CS        dcs;
  T_BAT_percent_cbhz_timeout timeout = BAT_P_CBHZ_TIMOUT_NOT_PRESENT;
  
  TRACE_FUNCTION ("tBAT_PercentCBHZ()");
  resp.ctrl_response = BAT_RES_QUE_PERCENT_CBHZ; 
  resp.response.ptr_tst_percent_cbhz = &tst_cbhz_buffer;


  resp.response.ptr_tst_percent_cbhz->mode =  CBHZ_MOD_Active;
  resp.response.ptr_tst_percent_cbhz->dcs =  CS_Ascii; 
  resp.response.ptr_tst_percent_cbhz->timeout = CBHZ_MAX_TIMEOUT;

  dont know how to fill the lower values ??? 
  CBHZ_xxx depends on FF_HOMEZONE
  aci_bat_send(src_infos_psi,&resp);
  return(ACI_BAT_CMPL);
*/
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCBHZ     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCBHZ     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
/* qAT_PercentCBHZ depends on FF_HOMEZONE
  T_BAT_percent_cbhz_mode   mode;
  T_ACI_CS        dcs;
  T_BAT_percent_cbhz_timeout timeout = BAT_P_CBHZ_TIMOUT_NOT_PRESENT;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cbhz que_cbhz_buffer;
  
  TRACE_FUNCTION ("qBAT_PercentCBHZ()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CBHZ; 
  resp.response.ptr_que_percent_cbhz = &que_cbhz_buffer;
  ret = qAT_PercentCBHZ(src_infos_psi->srcId, &mode, &dcs, &timeout);
  if (ret EQ AT_FAIL)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_NotPresent);
    return (ATI_FAIL);
  }
  resp.response.ptr_que_percent_cbhz->mode = mode;
  resp.response.ptr_que_percent_cbhz->timeout = timeout;
  resp.response.ptr_que_percent_cbhz->dcs = dcs;
*/
  return(ret);
}

/* #endif *//* FF_HOMEZONE */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSAS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSAS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
      
  TRACE_FUNCTION ("sBAT_PlusCSAS()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCSAS ((T_ACI_CMD_SRC)src_infos_psi->srcId,cmd->params.ptr_set_plus_csas->profile);

  return ret;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCSAS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCSAS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  SHORT        profile  = -1;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_csas tst_csas_buffer;

      
  TRACE_FUNCTION ("tBAT_PlusCSAS()");

  if ((profile = cmhSMS_getPrfRge ()) < 1)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_MemFail );
    return (ACI_BAT_FAIL);
  }
  else
  {
    resp.ctrl_response = BAT_RES_TST_PLUS_CSAS; 
    resp.response.ptr_tst_plus_csas = &tst_csas_buffer;
    resp.response.ptr_tst_plus_csas->max = (profile - 1) ;
    aci_bat_send(src_infos_psi,&resp);
    return (ACI_BAT_CMPL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCRES        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCRES        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PlusCRES()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCRES ((T_ACI_CMD_SRC)src_infos_psi->srcId,cmd->params.ptr_set_plus_cres->profile);

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCRES        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCRES        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_CRES       profile ;
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_cres tst_cres_buffer;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
      
  TRACE_FUNCTION ("tBAT_PlusCRES()");

  ret = (T_ACI_BAT_RSLT)tAT_PlusCRES ((T_ACI_CMD_SRC)src_infos_psi->srcId, &profile);
  switch (ret)
  {
    case ACI_BAT_CMPL:
    {
      break;
    }
    case ACI_BAT_FAIL:
    default:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_MemFail );
      return (ACI_BAT_FAIL);
    }
  }
  resp.ctrl_response = BAT_RES_TST_PLUS_CRES; 
  resp.response.ptr_tst_plus_cres = &tst_cres_buffer;
  resp.response.ptr_tst_plus_cres->max = profile.max ;
  /* there is no min field to fill in */
  //resp.response.ptr_tst_plus_cres->min = profile.min ;
  aci_bat_send(src_infos_psi,&resp);
  return (ACI_BAT_CMPL);
}


  /*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGR        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  SHORT idx=-1;
  
  TRACE_FUNCTION ("sBAT_PlusCMGR()");

  idx = cmd->params.ptr_set_plus_cmgr->sms_index;
  if ( idx > 255 OR idx < 0)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    return (ACI_BAT_FAIL);
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMGR_Gl((T_ACI_CMD_SRC)src_infos_psi->srcId, (UBYTE)idx, SMS_READ_Normal, NULL);
 /* 
  TO DO 
  ret = sAT_PlusCMGR_Gl(src_infos_psi->srcId, (UBYTE)idx, cmglRead, NULL);
 
*/
  return (ret);
}

  /*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCMGR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGR    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  SHORT idx=-1;
#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
#endif
  CHAR           *txtRdMode = NULL;

  
  TRACE_FUNCTION ("sBAT_PercentCMGR()");

  smsReadMode  = SMS_READ_Normal;

#ifdef SMS_PDU_SUPPORT
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)src_infos_psi->srcId, &mode);
    if (mode EQ 0)
    {
      /*
       * %CMGR=[index],[preview]
       */
      idx = cmd->params.ptr_set_percent_cmgr->sms_index;
      smsReadMode  = (T_ACI_SMS_READ)cmd->params.ptr_set_percent_cmgr->rdmode;
      if ( idx > 255 OR idx < 0 OR smsReadMode  > SMS_READ_StatusChange)
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
        return (ACI_BAT_FAIL);
      }
    }
    else
#endif
    {
      idx = cmd->params.ptr_set_percent_cmgr->sms_index;
      if ( idx > 255 OR idx < 0)
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
        return (ACI_BAT_FAIL);
      }
      txtRdMode = (CHAR *)&cmd->params.ptr_set_percent_cmgr->rdmode;
      if (txt_rdmode_to_smsrdmode(txtRdMode) EQ ATI_FAIL)
      {
        return (ACI_BAT_FAIL);
      }
    }
    ret = (T_ACI_BAT_RSLT)sAT_PercentCMGR_Gl((T_ACI_CMD_SRC)src_infos_psi->srcId, (UBYTE)idx, smsReadMode , NULL);

  return (ret);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_SM_DATA pdu;

  TRACE_FUNCTION ("sBAT_PlusCMGS()");

  /*
  *   Check that 'data' in the T_ACI_SM_DATA structure is large enough
  *   to store the data in the BAT command. If it isn't then we may
  *   as well give up.
  */
  if (cmd->params.ptr_set_plus_cmgs->c_pdu>MAX_SM_LEN)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Ext,EXT_ERR_Parameter);
    return(ACI_BAT_FAIL);
  }
  
  /*
  *   Copy the data, secure in the knowledge that we have enough room.
  */
  memcpy(
    pdu.data,
    cmd->params.ptr_set_plus_cmgs->pdu,
    MAX_SM_LEN);

  pdu.len=cmd->params.ptr_set_plus_cmgs->c_pdu;

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCMGSPdu((T_ACI_CMD_SRC)src_infos_psi->srcId,&pdu);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMSS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMSS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     toda;
  T_ACI_TOA    *p_toda;
  SHORT         index=-1,
                toda_val=0;
  CHAR          da[MAX_SMS_NUM_LEN] , *p_da;
  
  TRACE_FUNCTION ("sBAT_PlusCMSS()");

  p_toda=&toda;
  p_da=da;

  index = cmd->params.ptr_set_plus_cmss->index;
  if(cmd->params.ptr_set_plus_cmss->v_da)
  {
    memcpy(da,cmd->params.ptr_set_plus_cmss->da,cmd->params.ptr_set_plus_cmss->c_da);
  }
  else
  {
    p_da = NULL;
  }
  toda_val=cmd->params.ptr_set_plus_cmss->toda;
  if(toda_val EQ (SHORT)BAT_PARAMETER_NOT_PRESENT)
  {
    p_toda=NULL;
  }
  else
  {
    toda=toa_demerge(toda_val);
    if (toda.ton < 0 OR toda.npi < 0)
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_NotPresent); 
      return (ACI_BAT_FAIL);
    }
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMSS_Gl((T_ACI_CMD_SRC)src_infos_psi->srcId, (UBYTE)index, p_da, p_toda, NULL, NULL);

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGW        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGW        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_SMS_STAT    stat;
  T_ACI_SM_DATA     pdu;
  UBYTE sca_len;
  UBYTE pdu_message_octets_length = 0;
  
  TRACE_FUNCTION ("sBAT_PlusCMGW()");
  
  stat = (T_ACI_SMS_STAT)cmd->params.ptr_set_plus_cmgw->sms_stat;
  sca_len = cmd->params.ptr_set_plus_cmgw->pdu[0];
  /* The first octet in pdu is length of sca */  
  
  if (sca_len > ((MAX_SMS_ADDR_DIG+1)/2) + 1)
  {
    TRACE_ERROR("SCA too long !!!");
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    return (ACI_BAT_FAIL);
  }

  pdu_message_octets_length = (cmd->params.ptr_set_plus_cmgw->length-2-sca_len*2)/2;
  if(pdu_message_octets_length NEQ cmd->params.ptr_set_plus_cmgw->length)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    TRACE_EVENT("ERROR: input pdu message length do not match the real length!");
    return (ACI_BAT_FAIL);
  }
  memcpy(pdu.data,cmd->params.ptr_set_plus_cmgw->pdu,cmd->params.ptr_set_plus_cmgw->c_pdu);
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMGWPdu ((T_ACI_CMD_SRC)src_infos_psi->srcId, (UBYTE)stat, &pdu);

  return (ret);
}

/*
|+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT               |
| STATE   : code                       ROUTINE : sBAT_PlusCNMA        |
+---------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNMA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{  
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_SM_DATA  pdu = {0}, *p_pdu;
  
  TRACE_FUNCTION ("sBAT_PlusCNMA()");
  p_pdu = &pdu;
  if(cmd->params.ptr_set_plus_cnma->v_pdu)
  {
    memcpy(pdu.data,cmd->params.ptr_set_plus_cnma->pdu,cmd->params.ptr_set_plus_cnma->c_pdu);
    pdu.len = cmd->params.ptr_set_plus_cnma->c_pdu ; 
  }
  else
  {
    p_pdu = NULL;
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCNMAPdu ((T_ACI_CMD_SRC)src_infos_psi->srcId, (SHORT)cmd->params.ptr_set_plus_cnma->n, p_pdu);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGD        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  SHORT index=-1;
  SHORT status = BAT_CMGD_DEL_INDEX;

  
  TRACE_FUNCTION ("sBAT_PlusCMGD()");
 
  index = cmd->params.ptr_set_plus_cmgd->index;
  status = cmd->params.ptr_set_plus_cmgd->status;

  if((index > 255 OR index < 0) OR ((status < BAT_CMGD_DEL_INDEX) OR (status > BAT_CMGD_DEL_ALL)))
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    return (ACI_BAT_CMPL);
  }

  if (status > BAT_CMGD_DEL_INDEX)
  {
    index = 0;
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMGD_Gl((T_ACI_CMD_SRC)src_infos_psi->srcId,(UBYTE)index,status,NULL,NULL);

  return (ret);
}
  
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_SM_DATA     pdu;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE sca_len;

  TRACE_FUNCTION ("sBAT_PlusCMGC()");

  sca_len = cmd->params.ptr_set_plus_cmgc->pdu[0];
  /* The first octet in pdu is length of sca */  

  if (sca_len > ((MAX_SMS_ADDR_DIG+1)/2) + 1)
  {
    TRACE_ERROR("SCA too long !!!");
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    return (ACI_BAT_FAIL);
  }
   /*
  pdu_message_octets_length = (cmd->params.ptr_set_plus_cmgc->length-2-sca_len*2)/2;
 
  if(pdu_message_octets_length NEQ cmd->params.ptr_set_plus_cmgc->length)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
    TRACE_EVENT("ERROR: input pdu message length in cmgc do not match the real length!");
    return (ACI_BAT_FAIL);
  }
  length paramter is not present in cmgc struct ??? its there in old ati code
  */
  memcpy(pdu.data,cmd->params.ptr_set_plus_cmgc->pdu,cmd->params.ptr_set_plus_cmgc->c_pdu);
  pdu.len = cmd->params.ptr_set_plus_cmgc->c_pdu ;
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMGCPdu ( (T_ACI_CMD_SRC)src_infos_psi->srcId, &pdu );

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMGL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMGL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PlusCMGL()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCMGL ( (T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_SMS_STAT)cmd->params.ptr_set_plus_cmgl->stat, 0, SMS_READ_Normal );

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCMGL     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGL   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
#endif
  CHAR           *txtRdMode = NULL;
  CHAR           *txtStat = NULL;
  USHORT i;

  TRACE_FUNCTION ("sBAT_PercentCMGL()");

#ifdef SMS_PDU_SUPPORT
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)src_infos_psi->srcId, &mode);
    if (mode EQ 0)
    {
      /*
       * %CMGL=[stat],[preview]
       */
      smsReadMode  = (T_ACI_SMS_READ)cmd->params.ptr_set_percent_cmgl->rdmode;
      cmglStat = (T_ACI_SMS_STAT)cmd->params.ptr_set_percent_cmgl->stat;
      if ( ((cmglStat < SMS_STAT_RecUnread OR cmglStat > SMS_STAT_All)
                  AND smsReadMode EQ SMS_READ_NotPresent) 
            OR (smsReadMode  < (T_ACI_SMS_READ)SMS_STAT_RecUnread AND smsReadMode  > SMS_READ_NotPresent) 
            OR (smsReadMode  > SMS_READ_StatusChange))
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
        return (ACI_BAT_FAIL);
      }
    }
    else
#endif
    {
      txtStat = (char*)&cmd->params.ptr_set_percent_cmgl->stat;
      txtRdMode = (char*)&cmd->params.ptr_set_percent_cmgl->rdmode;
      if (txtStat[0] EQ '\0')
      {
        strcpy (txtStat, sms_stat[4].name); /* ALL as default */
      }
      i = 0;
      while ( sms_stat[i].name NEQ NULL            AND
              strcmp ( sms_stat[i].name, txtStat ) NEQ 0 )
        i++;

      if ( sms_stat[i].name EQ NULL )
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_OpNotAllowed);
        return (ACI_BAT_FAIL);
      }
      cmglStat = sms_stat[i].stat;

      if (txt_rdmode_to_smsrdmode(txtRdMode) EQ ATI_FAIL)
      {
        return (ACI_BAT_FAIL);
      }
    }

  ret = (T_ACI_BAT_RSLT)sAT_PercentCMGL ( (T_ACI_CMD_SRC)src_infos_psi->srcId, cmglStat, smsReadMode  );

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCNMI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/


GLOBAL T_ACI_BAT_RSLT sBAT_PlusCNMI        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PlusCNMI()");

  ret = (T_ACI_BAT_RSLT)sAT_PlusCNMI ( (T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CNMI_MT)cmd->params.ptr_set_plus_cnmi->mt, (T_ACI_CNMI_BM)cmd->params.ptr_set_plus_cnmi->bm, (T_ACI_CNMI_DS)cmd->params.ptr_set_plus_cnmi->ds );
  switch(ret)
  {
  case ACI_BAT_CMPL :
    if(src_infos_psi->srcId NEQ CMD_SRC_LCL)
    {
      smsShrdPrm.smsSrcId = (T_ACI_CMD_SRC)src_infos_psi->srcId;
    }
  /*lint -fallthrough*/
  default :
    return(ret);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCNMI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCNMI        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret           = ACI_BAT_FAIL;
  T_BAT_plus_cnmi_mt        mt = BAT_CNMI_MT_NOT_PRESENT;                       /*<  0:  4> (enum=32bit)<->T_BAT_plus_cnmi_mt Indication routing for SMS-DELIVERs */
  T_BAT_plus_cnmi_bm        bm = BAT_CNMI_BM_NOT_PRESENT;                       /*<  4:  4> (enum=32bit)<->T_BAT_plus_cnmi_bm Indication routing for CBMs */
  T_BAT_plus_cnmi_ds        ds = BAT_CNMI_DS_NOT_PRESENT;        
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cnmi que_cnmi_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCNMI()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CNMI; 
  resp.response.ptr_que_plus_cnmi = &que_cnmi_buffer;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCNMI ( (T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CNMI_MT*)&mt,(T_ACI_CNMI_BM*)&bm,(T_ACI_CNMI_DS*)&ds );
  switch(ret)
  {
  case ACI_BAT_CMPL :
    resp.response.ptr_que_plus_cnmi->mt = mt;
    resp.response.ptr_que_plus_cnmi->bm = bm;
    resp.response.ptr_que_plus_cnmi->ds = ds;
    aci_bat_send(src_infos_psi,&resp);
  /*lint -fallthrough*/
  default :
    return (ret);
  }
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPRSM    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPRSM    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  //T_ACI_CPRSM_MOD mode = CPRSM_MOD_NotPresent;
  
  TRACE_FUNCTION ("sBAT_PercentCPRSM()");
  
  ret = (T_ACI_BAT_RSLT)sAT_PercentCPRSM((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CPRSM_MOD)cmd->params.ptr_set_percent_cprsm->mode);
  switch (ret)
  {
    case (ACI_BAT_CMPL):
    case (ACI_BAT_EXCT):
    {
      break;
    }
    default:
    {      
      TRACE_EVENT_P1("+CPRSM ERROR: undefined result: %d", ret);
      ACI_ERR_DESC(ACI_ERR_CLASS_Cms,CMS_ERR_UnknownErr);
      return ACI_BAT_FAIL;
    }
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPRSM    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPRSM    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
    
  TRACE_FUNCTION ("qBAT_PercentCPRSM()");
  ret = (T_ACI_BAT_RSLT)qAT_PercentCPRSM( (T_ACI_CMD_SRC)src_infos_psi->srcId );
  /* Does not fill in the response structure? */
  /* ACI does not fill in response mode in qAT_PercentCPRSM?*/
  
  return (ret);
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

#ifdef REL99
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCMGRS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION("sBAT_PercentCMGRS()");

  if(cmd->params.ptr_set_percent_cmgrs->mode > BAT_CMGRS_MODE_MANUAL_RETRANS)
  {
    return (ACI_BAT_FAIL);
  }

  ret = (T_ACI_BAT_RSLT)sAT_PercentCMGRS((T_ACI_CMD_SRC)src_infos_psi->srcId,
                         (T_ACI_CMGRS_MODE)cmd->params.ptr_set_percent_cmgrs->mode);
  return (ret);
}

GLOBAL T_ACI_BAT_RSLT qBAT_PercentCMGRS   (T_ACI_DTI_PRC_PSI *src_infos_psi, T_BAT_cmd_send *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  UBYTE        auto_rep_flag;
  T_BAT_res_que_percent_cmgrs que_cmgrs;

  TRACE_FUNCTION ("qBAT_PercentCMGRS()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CMGRS;
  resp.response.ptr_que_percent_cmgrs = &que_cmgrs;

  ret = (T_ACI_BAT_RSLT)qAT_PercentCMGRS((T_ACI_CMD_SRC)src_infos_psi->srcId,&auto_rep_flag);

  switch(ret)
  {
  case ACI_BAT_CMPL :
    resp.response.ptr_que_percent_cmgrs->mode = (T_BAT_percent_cmgrs_mode)auto_rep_flag;
    aci_bat_send(src_infos_psi,&resp);
  /*lint -fallthrough*/
  default :
    return (ret);
  }
} 
#endif /* REL99 */


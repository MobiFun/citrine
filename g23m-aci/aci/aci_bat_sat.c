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
#include "aci_bat.h"

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSATC     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentSATC()");

  ret = (T_ACI_BAT_RSLT)sAT_PercentSATC((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (SHORT)cmd->params.ptr_set_percent_satc->c_satprfl,
                        (UBYTE *)cmd->params.ptr_set_percent_satc->satprfl);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentSATC     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSATC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_satc satc;

  TRACE_FUNCTION ("qBAT_PercentSATC()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_SATC;
  resp.response.ptr_que_percent_satc = &satc;

  ret =(T_ACI_BAT_RSLT) qAT_PercentSATC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                        (S16*)&satc.c_satprfl, satc.satprfl);

  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSATE     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATE(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentSATE()");
  
  ret = (T_ACI_BAT_RSLT)sAT_PercentSATE((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (SHORT)cmd->params.ptr_set_percent_sate->c_satcmd,
                        (UBYTE *)cmd->params.ptr_set_percent_sate->satcmd);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSATR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATR(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentSATR()");
  
  ret =(T_ACI_BAT_RSLT) sAT_PercentSATR((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (SHORT)cmd->params.ptr_set_percent_satr->c_satrsp,
                        (UBYTE *)cmd->params.ptr_set_percent_satr->satrsp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSATT     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSATT(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentSATT()");
  
  ret =(T_ACI_BAT_RSLT) sAT_PercentSATT((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (T_ACI_SATT_CS)cmd->params.ptr_set_percent_satt->cs);
  return (ret);
}



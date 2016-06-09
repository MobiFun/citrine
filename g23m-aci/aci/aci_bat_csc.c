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
#include "aci_cmd.h"
#include "aci_lst.h" 
#include "ati_int.h"
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "aci_bat.h"


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentBAND     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentBAND(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_set_percent_band *band = cmd->params.ptr_set_percent_band;

  TRACE_FUNCTION ("sBAT_PercentBAND()");
  
  /* Pass the value directly as given by the BAT application */
  if (band->band NEQ BAT_P_BAND_BAND_NOT_PRESENT)
  {
    ret = (T_ACI_BAT_RSLT)sAT_PercentBAND((T_ACI_CMD_SRC)src_infos_psi->srcId, 
		                                                          (T_ACI_BAND_MODE)band->mode, (U8)band->band);
  }

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentBAND     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentBAND(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  TRACE_FUNCTION ("tBAT_PercentBAND()");
  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentBAND     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentBAND(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_band band;

  TRACE_FUNCTION ("qBAT_PercentBAND()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_BAND;
  resp.response.ptr_que_percent_band = &band;
  /* Band initial value should be assigned to 0 */
  resp.response.ptr_que_percent_band->band = BAT_P_BAND_BAND_NOT_PRESENT; 

  ret = (T_ACI_BAT_RSLT)qAT_PercentBAND((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	                       (T_ACI_BAND_MODE*)&band.mode, (U8*)&band.band);
  
  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPI      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  /* T_BAT_res_que_percent_cpi cpi; */

  TRACE_FUNCTION ("qBAT_PercentCPI()");
  
  /* resp.ctrl_response = BAT_RES_QUE_PERCENT_CPI; */
  /* hmm, do not know which ACI function to call ! */ 
  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCNAP     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCNAP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cnap cnap;

  TRACE_FUNCTION ("qBAT_PercentCNAP()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CNAP;
  resp.response.ptr_que_percent_cnap = &cnap;

  ret = (T_ACI_BAT_RSLT)qAT_PercentCNAP((T_ACI_CMD_SRC)src_infos_psi->srcId);
  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentALS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentALS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_ALS_MOD ALSmode;

  TRACE_FUNCTION ("sBAT_PercentALS()");

#ifndef _SIMULATION_
/* mapping for SAP BAT %ALS values according to 8415.052.doc */
  switch( cmd->params.ptr_set_percent_als->mode )
  {
      case BAT_P_ALS_MOD_SPEECH:
        ALSmode = ALS_MOD_SPEECH;
        break;
  
      case BAT_P_ALS_MOD_AUX_SPEECH:
        ALSmode = ALS_MOD_AUX_SPEECH;
        break;
  
      default:
        cmdCmeError(CME_ERR_OpNotSupp);
        return (ACI_BAT_FAIL);
  }
#else /* if SIMULATION */
  ALSmode = cmd->params.ptr_set_percent_als->mode;
#endif /* SIMULATION */ 
  ret = (T_ACI_BAT_RSLT)sAT_PercentALS((T_ACI_CMD_SRC)src_infos_psi->srcId, ALSmode);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentALS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentALS      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  TRACE_FUNCTION ("tBAT_PercentALS()");
  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentALS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentALS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_als als;
  T_ACI_ALS_MOD als_aci_mode;

  TRACE_FUNCTION ("qBAT_PercentALS()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_ALS;
  resp.response.ptr_que_percent_als = &als;

  ret = (T_ACI_BAT_RSLT)qAT_PercentALS((T_ACI_CMD_SRC)src_infos_psi->srcId,&als_aci_mode);
  switch(als_aci_mode)
  {
      case ALS_MOD_SPEECH:
        als.mode = BAT_P_ALS_MOD_SPEECH;
        break;
  
      case ALS_MOD_AUX_SPEECH:
        als.mode = BAT_P_ALS_MOD_AUX_SPEECH;
        break;
  
      default:
        cmdCmeError(CME_ERR_OpNotSupp);
        return (ACI_BAT_FAIL);
  }
  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentPPP      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_SAT_E)

GLOBAL T_ACI_BAT_RSLT sBAT_PercentPPP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentPPP()");

  ret =   (T_ACI_BAT_RSLT)sAT_PercentPPP((T_ACI_CMD_SRC)src_infos_psi->srcId,
            (T_ACI_PPP_PROT)cmd->params.ptr_set_percent_ppp->authprot,
            (CHAR *)cmd->params.ptr_set_percent_ppp->user,
            (CHAR *)cmd->params.ptr_set_percent_ppp->password,
            (T_ACI_PPP_CON)cmd->params.ptr_set_percent_ppp->conn_type);

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentPPP      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/

GLOBAL T_ACI_BAT_RSLT qBAT_PercentPPP(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_ppp ppp;

  TRACE_FUNCTION ("qBAT_PercentPPP()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_PPP;
  resp.response.ptr_que_percent_ppp = &ppp;

  ret = (T_ACI_BAT_RSLT) qAT_PercentPPP(src_infos_psi->srcId, (ULONG*)&ppp.ipaddr, (ULONG*)&ppp.dns1,(ULONG*)&ppp.dns2);
  aci_bat_send(src_infos_psi, &resp);
  return (ret);
}
#endif /* defined(FF_WAP) || defined(FF_PPP) || defined(FF_SAT_E) */


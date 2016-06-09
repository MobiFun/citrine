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
#include "pwr.h"         /* see sBAT_PlusCBC() */
#include "audio.h"       /* see sBAT_PlusCLVL(), qBAT_PlusCLVL() */
#include "pcm.h"        /* see aciBatPrcsPlusCG() */

LOCAL T_ACI_BAT_RSLT aciBatPrcsPlusCG (UBYTE *rc, USHORT *le, size_t ml, CHAR *ef);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_Z                |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT BAT_Z (T_ACI_DTI_PRC_PSI *src_infos_psi,
                             T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("BAT_Z()");

  ret=(T_ACI_BAT_RSLT)sAT_Z((T_ACI_CMD_SRC)src_infos_psi->srcId,0);

  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPAS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPAS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_CPAS_PAS pas;

  TRACE_FUNCTION ("sBAT_PlusCPAS()");

  /*
  *   The ACI has implemented the 'set' operation of +CPAS as a qAT function.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCPAS((T_ACI_CMD_SRC)src_infos_psi->srcId,&pas);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_plus_cpas cpas_data;

    resp.ctrl_response=BAT_RES_SET_PLUS_CPAS;
    resp.response.ptr_set_plus_cpas=&cpas_data;

    /*
    *   This relies on T_BAT_plus_cpas_pas being identical to 
    *   T_ACI_CPAS_PAS. They aren't quite the same, the BAT version
    *   doesn't have a 'not present' value, but as the parameter in
    *   question is mandatory this shouldn't be a problem.
    */
    cpas_data.pas=(T_BAT_plus_cpas_pas)pas;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_ANDC             |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT BAT_ANDC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_DCD_MOD dcd;

  TRACE_FUNCTION ("BAT_ANDC()");

  /*
  *   This relies on T_BAT_andc_value being identical to 
  *   T_ACI_DCD_MOD.
  */
  dcd=(T_ACI_DCD_MOD)cmd->params.ptr_andc->value;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_AndC((T_ACI_CMD_SRC)src_infos_psi->srcId,dcd);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_ANDF             |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT BAT_ANDF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("BAT_ANDF()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_AndF((T_ACI_CMD_SRC)src_infos_psi->srcId,0);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSQ         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSQ (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  UBYTE rssi;
  UBYTE ber;
  UBYTE actlevel;
#ifdef FF_PS_RSSI
  UBYTE min_access_level;
#endif

  TRACE_FUNCTION ("sBAT_PlusCSQ()");

  /*
  *   The ACI has implemented the 'set' operation of +CSQ as a qAT function.
  *   There isn't a separate qAT_PlusCSQ() function, so we must use the
  *   'percent' version and ignore the extra 'actlevel' and min_access_level
  *   value returned.
  */
#ifdef FF_PS_RSSI
  ret=(T_ACI_BAT_RSLT)qAT_PercentCSQ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                                     &rssi,
                                     &ber,
                                     &actlevel,
                                     &min_access_level);
#else
  ret=(T_ACI_BAT_RSLT)qAT_PercentCSQ(src_infos_psi->srcId,&rssi,&ber,&actlevel);
#endif

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_plus_csq csq_data;

    resp.ctrl_response=BAT_RES_SET_PLUS_CSQ;
    resp.response.ptr_set_plus_csq=&csq_data;

    csq_data.rssi=(U8)rssi;
    csq_data.ber=(U8)ber;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCSQ      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCSQ (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PercentCSQ()");

  ret = (T_ACI_BAT_RSLT)sAT_PercentCSQ ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                                   (T_ACI_CSQ_MODE)cmd->params.ptr_set_percent_csq->csq_mode);
  
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_percent_csq csq_data;

    resp.ctrl_response = BAT_RES_SET_PERCENT_CSQ;
    resp.response.ptr_set_percent_csq = &csq_data;

    csq_data.csq_mode = cmd->params.ptr_set_percent_csq->csq_mode;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCSQ      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCSQ (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  UBYTE rssi;
  UBYTE ber;
  UBYTE actlevel;
#ifdef FF_PS_RSSI
  UBYTE min_access_level;
#endif

  TRACE_FUNCTION ("qBAT_PercentCSQ()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
#ifdef FF_PS_RSSI
  ret=(T_ACI_BAT_RSLT)qAT_PercentCSQ((T_ACI_CMD_SRC)src_infos_psi->srcId,
                                     &rssi,
                                     &ber,
                                     &actlevel,
                                     &min_access_level);
#else
  ret=(T_ACI_BAT_RSLT)qAT_PercentCSQ(src_infos_psi->srcId,&rssi,&ber,&actlevel);
#endif

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_csq csq_data;

    resp.ctrl_response=BAT_RES_QUE_PERCENT_CSQ;
    resp.response.ptr_que_percent_csq=&csq_data;

    csq_data.rssi=(U8)rssi;
    csq_data.ber=(U8)ber;
    csq_data.actlevel=(U8)actlevel;

#ifdef FF_PS_RSSI
    csq_data.min_access_level = (U8)min_access_level;
#endif

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCBC         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCBC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  pwr_Status_Type powerStat;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cbc cbc_data;

  TRACE_FUNCTION("sBAT_PlusCBC()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT. This code is adapted from the ATI function
  *   atPlusCBC().
  */

  if (pwr_GetStatus(&powerStat) NEQ DRV_OK)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  resp.ctrl_response=BAT_RES_SET_PLUS_CBC;
  resp.response.ptr_set_plus_cbc=&cbc_data;

  switch(powerStat.Status)
  {
    case PWR_EXTPOWER_ON:
      cbc_data.bsc=BAT_CBC_BSC_NOBATTERY;
      break;

    case PWR_CHARGER_ON:
      cbc_data.bsc=BAT_CBC_BSC_BATTERY;
      break;

    default:
      cbc_data.bsc=BAT_CBC_BSC_BATTERYPOWERED;
      break;
  }

  cbc_data.bcl=(T_BAT_plus_cbc_bcl)powerStat.ChargeLevel;

  aci_bat_send(src_infos_psi,&resp);

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCLVL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLVL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  TRACE_FUNCTION ("sBAT_PlusCLVL()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */

  if (audio_SetAmplf(AUDIO_SPEAKER,(UBYTE)cmd->params.ptr_set_plus_clvl->level) NEQ DRV_OK)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCLVL        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLVL        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  UBYTE vol;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clvl clvl_data;

  TRACE_FUNCTION ("qBAT_PlusCLVL()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */

  if (audio_GetAmplf(AUDIO_MICROPHONE,&vol) NEQ DRV_OK)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  resp.ctrl_response=BAT_RES_QUE_PLUS_CLVL;
  resp.response.ptr_que_plus_clvl=&clvl_data;

  clvl_data.level=(U8)vol;

  aci_bat_send(src_infos_psi,&resp);

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMUT        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMUT (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  UBYTE mute;

  TRACE_FUNCTION ("sBAT_PlusCMUT()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */
  mute=(cmd->params.ptr_set_plus_cmut->n EQ BAT_CMUT_N_ON) ? AUDIO_MUTING_ON:AUDIO_MUTING_OFF;

  if(audio_SetMute(AUDIO_MICROPHONE,mute) NEQ DRV_OK)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCMUT        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMUT (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  UBYTE mute;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cmut cmut_data;

  TRACE_FUNCTION ("qBAT_PlusCMUT()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */

  if (audio_GetMute(AUDIO_MICROPHONE,&mute) NEQ DRV_OK)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  resp.ctrl_response=BAT_RES_QUE_PLUS_CMUT;
  resp.response.ptr_que_plus_cmut=&cmut_data;

  cmut_data.n=(U8)(mute EQ AUDIO_MUTING_ON) ? BAT_CMUT_N_ON:BAT_CMUT_N_OFF;

  aci_bat_send(src_infos_psi,&resp);

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSCS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSCS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_BAT_plus_cscs_cs cs;

  TRACE_FUNCTION ("sBAT_PlusCSCS()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */

  cs=cmd->params.ptr_set_plus_cscs->cs;

  /*
  *   As we are handling this within BAT and not forwarding it to
  *   the ACI we should check the input parameter.
  */
  if ((cs<BAT_CSCS_CS_IRA) || (cs>BAT_CSCS_CS_UCS2))
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  aci_bat_cs_set(src_infos_psi,cs);

  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSCS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSCS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cscs cscs_data;

  TRACE_FUNCTION ("qBAT_PlusCSCS()");

  /*
  *   There is no equivalent ACI function, so we must handle this
  *   within BAT.
  */

  resp.ctrl_response=BAT_RES_QUE_PLUS_CSCS;
  resp.response.ptr_que_plus_cscs=&cscs_data;

  cscs_data.cs=aci_bat_cs_get(src_infos_psi);

  aci_bat_send(src_infos_psi,&resp);
  
  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCCBS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCCBS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  SHORT idx;

  TRACE_FUNCTION ("sBAT_PercentCCBS()");

  idx=(SHORT)cmd->params.ptr_set_percent_ccbs->idx;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PercentCCBS((T_ACI_CMD_SRC)src_infos_psi->srcId,idx);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCCBS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCCBS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("qBAT_PercentCCBS()");
  
  ret=(T_ACI_BAT_RSLT)qAT_PercentCCBS((T_ACI_CMD_SRC)src_infos_psi->srcId);

  /*
  *   qAT_PercentCCBS() never returns AT_CMPL, so we do not send
  *   the response here.
  */

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMUX        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMUX (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_set_plus_cmux *cmux;
  
  TRACE_FUNCTION ("sBAT_PlusCMUX()");

  /*
  *   Get a pointer to the CMUX data for convenience.
  */
  cmux=cmd->params.ptr_set_plus_cmux;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT. Note that we lose
  *   'k' as it is not in the ACI.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCMUX(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    (UBYTE)cmux->mode,
    (UBYTE)cmux->subset,
    (UBYTE)cmux->port_speed,
    (USHORT)cmux->n1,
    (UBYTE)cmux->t1,
    (UBYTE)cmux->n2,
    (UBYTE)cmux->t2,
    (UBYTE)cmux->t3);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCMUX        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMUX (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  TRACE_FUNCTION ("qBAT_PlusCMUX()");

  /*
  *   There is no ACI equivalent.
  */
  return(ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusIPR         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusIPR (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_BD_RATE rate;

  TRACE_FUNCTION ("sBAT_PlusIPR()");

  /*
  *   This relies on T_ACI_BD_RATE and T_BAT_plus_ipr_rate being
  *   identical. In fact the BAT has an extra value, but this should
  *   not cause problems as there is a parameter check in
  *   sAT_PlusIPR().
  */
  rate=(T_ACI_BD_RATE)cmd->params.ptr_set_plus_ipr->rate;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusIPR((T_ACI_CMD_SRC)src_infos_psi->srcId,rate);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusIPR         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusIPR (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_BD_RATE rate;

  TRACE_FUNCTION ("qBAT_PlusIPR()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusIPR((T_ACI_CMD_SRC)src_infos_psi->srcId,&rate);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_ipr ipr_data;

    resp.ctrl_response=BAT_RES_QUE_PLUS_IPR;
    resp.response.ptr_que_plus_ipr=&ipr_data;

    /*
    *   This relies on T_ACI_BD_RATE and T_BAT_plus_ipr_rate being
    *   identical. In fact the BAT has an extra value, but that is
    *   irrelevant in this direction.
    */
    ipr_data.rate=(T_BAT_plus_ipr_rate)rate;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusICF         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusICF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_BS_FRM format;
  T_ACI_BS_PAR parity;

  TRACE_FUNCTION ("sBAT_PlusICF()");

  /*
  *   This relies on T_ACI_BS_FRM being identical to T_BAT_framing_format
  *   and T_ACI_BS_PAR being identical to T_BAT_framing_parity.
  */
  format=(T_ACI_BS_FRM)cmd->params.ptr_set_plus_icf->framing_format;
  parity=(T_ACI_BS_PAR)cmd->params.ptr_set_plus_icf->framing_parity;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusICF((T_ACI_CMD_SRC)src_infos_psi->srcId,format,parity);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusICF         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusICF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_BS_FRM format;
  T_ACI_BS_PAR parity;

  TRACE_FUNCTION ("qBAT_PlusICF()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusICF((T_ACI_CMD_SRC)src_infos_psi->srcId,&format,&parity);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_icf icf_data;

    resp.ctrl_response=BAT_RES_QUE_PLUS_ICF;
    resp.response.ptr_que_plus_icf=&icf_data;

    /*
    *   This relies on T_ACI_BS_FRM being identical to T_BAT_framing_format
    *   and T_ACI_BS_PAR being identical to T_BAT_framing_parity.
    */
    icf_data.framing_format=(T_BAT_framing_format)format;
    icf_data.framing_parity=(T_BAT_framing_parity)parity;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusIFC         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusIFC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_RX_FLOW_CTRL DCE_by_DTE;
  T_ACI_RX_FLOW_CTRL DTE_by_DCE;

  TRACE_FUNCTION ("sBAT_PlusIFC()");

  /*
  *   This relies on T_ACI_RX_FLOW_CONTROL being identical to both
  *   T_BAT_plus_ifc_by_te and T_BAT_plus_ifc_by_ta. Note that
  *   sAT_PlusIFC() really does use T_ACI_RX_FLOW_CTRL for both
  *   parameters even though a T_ACI_TX_FLOW_CTRL exists (they are
  *   equivalent anyway).
  */
  DCE_by_DTE=(T_ACI_RX_FLOW_CTRL)cmd->params.ptr_set_plus_ifc->by_te;
  DTE_by_DCE=(T_ACI_RX_FLOW_CTRL)cmd->params.ptr_set_plus_ifc->by_ta;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusIFC((T_ACI_CMD_SRC)src_infos_psi->srcId,DCE_by_DTE,DTE_by_DCE);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusIFC         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusIFC (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                    T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_RX_FLOW_CTRL DCE_by_DTE;
  T_ACI_RX_FLOW_CTRL DTE_by_DCE;

  TRACE_FUNCTION ("qBAT_PlusIFC()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusIFC((T_ACI_CMD_SRC)src_infos_psi->srcId,&DCE_by_DTE,&DTE_by_DCE);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_ifc ifc_data;

    resp.ctrl_response=BAT_RES_QUE_PLUS_IFC;
    resp.response.ptr_que_plus_ifc=&ifc_data;

    /*
    *   This relies on T_ACI_RX_FLOW_CONTROL being identical to both
    *   T_BAT_plus_ifc_by_te and T_BAT_plus_ifc_by_ta. Note that
    *   qAT_PlusIFC() really does use T_ACI_RX_FLOW_CTRL for both
    *   parameters even though a T_ACI_TX_FLOW_CTRL exists (they are
    *   equivalent anyway).
    */
    ifc_data.by_te=(T_BAT_plus_ifc_by_te)DCE_by_DTE;
    ifc_data.by_ta=(T_BAT_plus_ifc_by_ta)DTE_by_DCE;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCCLK        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCLK (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_RTC_DATE date;
  T_ACI_RTC_TIME time;
  int time_zone;

  TRACE_FUNCTION ("sBAT_PlusCCLK()");

  date.year=(USHORT)(cmd->params.ptr_set_plus_cclk->year+2000);
  date.month=(UBYTE)cmd->params.ptr_set_plus_cclk->month;
  date.day=(UBYTE)cmd->params.ptr_set_plus_cclk->day;
  time.hour=(UBYTE)cmd->params.ptr_set_plus_cclk->hour;
  time.minute=(UBYTE)cmd->params.ptr_set_plus_cclk->minutes;
  time.second=(UBYTE)cmd->params.ptr_set_plus_cclk->seconds;

  /*
  *   Time format and PM flag are not in the BAT message. Use the same
  *   values that the ATI does in this situation.
  */
  time.format=TIME_FORMAT_24HOUR;
  time.PM_flag=0;

  time_zone=(int)cmd->params.ptr_set_plus_cclk->time_zone;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCCLK((T_ACI_CMD_SRC)src_infos_psi->srcId,&date,&time,time_zone);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCCLK        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCCLK (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_RTC_DATE date;
  T_ACI_RTC_TIME time;
  int time_zone;

  TRACE_FUNCTION ("qBAT_PlusCCLK()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCCLK((T_ACI_CMD_SRC)src_infos_psi->srcId,&date,&time,&time_zone);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_cclk cclk_data;

    resp.ctrl_response=BAT_RES_QUE_PLUS_CCLK;
    resp.response.ptr_que_plus_cclk=&cclk_data;

    cclk_data.year=(U8)(date.year-2000);
    cclk_data.month=(U8)date.month;
    cclk_data.day=(U8)date.day;

    cclk_data.hour=(U8)time.hour;

    /*
    *   BAT only handles a 24 hour clock. So if the information arrives
    *   in 12 hour format we must convert it.
    */
    if (time.format EQ TIME_FORMAT_12HOUR)
    {
      if ((time.PM_flag) AND (time.hour<12))
        cclk_data.hour+=12;
      else if ((time.PM_flag EQ 0) AND (time.hour EQ 12))
        cclk_data.hour=0;
    }

    cclk_data.minutes=(U8)time.minute;
    cclk_data.seconds=(U8)time.second;

    cclk_data.time_zone=(S8)time_zone;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGMI        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cgmi set_cgmi_buffer;
  
  TRACE_FUNCTION ("sBAT_PlusCGMI()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CGMI ;
  resp.response.ptr_set_plus_cgmi = &set_cgmi_buffer;
    
  ret = aciBatPrcsPlusCG(resp.response.ptr_set_plus_cgmi->manufacturer, &resp.response.ptr_set_plus_cgmi->c_manufacturer, 
                                        sizeof(resp.response.ptr_set_plus_cgmi->manufacturer), EF_CGMI_ID);
  if (ret EQ ACI_BAT_CMPL)
  {
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGMM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMM(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cgmm set_cgmm_buffer;
  
  TRACE_FUNCTION ("sBAT_PlusCGMM()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CGMM ;
  resp.response.ptr_set_plus_cgmm = &set_cgmm_buffer;
    
  ret = aciBatPrcsPlusCG(resp.response.ptr_set_plus_cgmm->model, &resp.response.ptr_set_plus_cgmm->c_model, 
                                        sizeof(resp.response.ptr_set_plus_cgmm->model), EF_CGMM_ID);
  if (ret EQ ACI_BAT_CMPL)
  {
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGMR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGMR(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cgmr set_cgmr_buffer;
  
  TRACE_FUNCTION ("sBAT_PlusCGMR()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CGMR ;
  resp.response.ptr_set_plus_cgmr = &set_cgmr_buffer;
    
  ret = aciBatPrcsPlusCG(resp.response.ptr_set_plus_cgmr->revision, &resp.response.ptr_set_plus_cgmr->c_revision, 
                                        sizeof(resp.response.ptr_set_plus_cgmr->revision), EF_CGMR_ID);
  if (ret EQ ACI_BAT_CMPL)
  {
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGSN(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cgsn set_cgsn_buffer;
  
  TRACE_FUNCTION ("sBAT_PlusCGSN()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CGSN ;
  resp.response.ptr_set_plus_cgsn = &set_cgsn_buffer;
    
  ret = aciBatPrcsPlusCG(resp.response.ptr_set_plus_cgsn->sn, &resp.response.ptr_set_plus_cgsn->c_sn, 
                                        sizeof(resp.response.ptr_set_plus_cgsn->sn), EF_CGSN_ID);
  if (ret EQ ACI_BAT_CMPL)
  {
    aci_bat_send(src_infos_psi,&resp);
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_BAT            |
| STATE   : code                        ROUTINE : aciBatPrcsPlusCG      |
+--------------------------------------------------------------------+

  PURPOSE :
*/
LOCAL T_ACI_BAT_RSLT aciBatPrcsPlusCG (UBYTE *rc, USHORT *le, size_t ml, CHAR *ef)
{
  pcm_FileInfo_Type fileInfo;
  USHORT i;

  TRACE_FUNCTION("aciBatPrcsPlusCG()");

  if (pcm_GetFileInfo ((UBYTE* )ef, &fileInfo) NEQ DRV_OK)
  {
    cmdCmeError (CME_ERR_MemFail);
    return ACI_BAT_FAIL;
  }

  if (fileInfo.FileSize <= ml)
  {
    if (pcm_ReadFile ((UBYTE*)ef, fileInfo.FileSize,
                      (UBYTE*)rc, &fileInfo.Version) NEQ DRV_OK)
    {
      cmdCmeError (CME_ERR_MemFail);
      return ACI_BAT_FAIL;
    }

    i = 0;
    while (rc[i] NEQ 0xFF)
    {
      i++;
    }
    *le = i;
     
    return ACI_BAT_CMPL;
  }
  else 
  {
    TRACE_FUNCTION("aciBatPrcsPlusCG(): file size is larger than the defined response length!");
    cmdCmeError (CME_ERR_MemFail);
    return ACI_BAT_FAIL;
  }
}



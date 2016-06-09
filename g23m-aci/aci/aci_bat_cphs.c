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
#include "cphs.h"
#include "aci_cphs.h"
#include "aci_cmd.h"

/*
 * commonly used query for %CPCFU and %CPVWI
 * %CPCFU: <status>[,<line>]
 * %CPVWI: <status>[,<line>]
 *
 * status acts as a global status information for all lines.
 * if any line is set then status is set/activated
 * and in the line bit field you can check which one.
 * the strange thing is that the numerical value for status
 * is exactly complementary between %CPCFU and %CPVWI
 * see also 8415_052.doc
 */
LOCAL T_ACI_BAT_RSLT query_cphs_lines (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd,
                                       T_CPHS_LINES       lines)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_RETURN (*fptr)(T_ACI_CMD_SRC srcId, U8 *cfu_set, T_CPHS_LINES line);
  T_CPHS_LINES line         = CPHS_LINE_NULL;
  T_CPHS_LINES line_bitmask = CPHS_LINE_NULL;
  U16          i, end;
  U8           flag_set;

  T_BAT_res_set_percent_cpvwi cpvwi;
  T_BAT_res_set_percent_cpcfu cpcfu;
  T_BAT_cmd_response          resp; 
 
  switch (cmd->ctrl_params)
  {
    case (BAT_CMD_SET_PERCENT_CPCFU):
    {
      fptr = qAT_PercentCPCFU;
      break;
    }
    case (BAT_CMD_SET_PERCENT_CPVWI):
    {
      fptr = qAT_PercentCPVWI;
      break;
    }
    default:
    {
      return (ACI_BAT_FAIL);
    }
  }

  end = 8 * sizeof(T_CPHS_LINES);
  /* rotate over all bits */
  for (i=0; i<end; i++)
  {
    line = 1 << i;
    if (lines & line)
    {
      ret = (T_ACI_BAT_RSLT)fptr((T_ACI_CMD_SRC)src_infos_psi->srcId, &flag_set, line);

      if(ret NEQ (T_ACI_BAT_RSLT)AT_CMPL)
      {
        return(ACI_BAT_FAIL);
      }

      if(flag_set EQ CPHS_FLAG_ACTIVATED)
      {
        line_bitmask |= line;
      }
    }
    else
    {
      continue; /* bit not set --> do not check this line */
    } 
  }

  switch (cmd->ctrl_params)
  {
    case (BAT_CMD_SET_PERCENT_CPCFU):
    {
      cpcfu.line = (T_BAT_percent_cpcfu_line)line_bitmask;
      cpcfu.status = (line_bitmask EQ CPHS_LINE_NULL) 
                     ? BAT_P_CPCFU_STATUS_DEACTIVATE
                     : BAT_P_CPCFU_STATUS_ACTIVATED;    
      resp.ctrl_response = BAT_RES_SET_PERCENT_CPCFU;
      resp.response.ptr_set_percent_cpcfu = &cpcfu;
      /* 
       * according to 8415_052.doc CPCFU status: 0 Activated, 1 Deactivated,
       * which must be an error, because on ATI the status is given as 1, when any line has been set.
       * But the nice thing with BAT is the using of symbolic constants, where the actual numeric
       * value for BAT_P_CPCFU_STATUS_ACTIVATED is unimportant as long as the client code uses it.
       * If they use numerical value instead of symbolic constant, then ... we cannot help ...
       */
      TRACE_EVENT_P2("query_cphs_lines(): status = %i, line = 0x%02X",cpcfu.status, cpcfu.line);
      break;
    }
    case (BAT_CMD_SET_PERCENT_CPVWI):
    {
      cpvwi.line = (T_BAT_percent_cpvwi_lines)line_bitmask;
      cpvwi.status = (line_bitmask EQ CPHS_LINE_NULL) 
                     ? BAT_P_CPVWI_FLAG_DEACTIVATED
                     : BAT_P_CPVWI_FLAG_ACTIVATED;
      resp.ctrl_response = BAT_RES_SET_PERCENT_CPVWI;
      resp.response.ptr_set_percent_cpvwi = &cpvwi;
      TRACE_EVENT_P2("query_cphs_lines(): status = %i, line = 0x%02X",cpvwi.status, cpvwi.line);
      break;
    }
    default:
    {
      return (ACI_BAT_FAIL);
    }
  }

  aci_bat_send(src_infos_psi, &resp);
  return (ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPHS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPHS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentCPHS()");
  ret = (T_ACI_BAT_RSLT)sAT_PercentCPHS((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                        (T_ACI_CPHS_INIT)cmd->params.ptr_set_percent_cphs->init_mode); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPHS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPHS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cphs cphs;

  TRACE_FUNCTION ("qBAT_PercentCPHS()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CPHS; 
  resp.response.ptr_que_percent_cphs = &cphs;

  ret = (T_ACI_BAT_RSLT)qAT_PercentCPHS((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_CPHS_INIT*)&cphs.init_mode); 

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPNUMS   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPNUMS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                         T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;

  TRACE_FUNCTION ("sBAT_PercentCPNUMS()");

  ret = (T_ACI_BAT_RSLT)sAT_PercentCPNUMS((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                          cmd->params.ptr_set_percent_cpnums->element_id, 
                          (U8)cmd->params.ptr_set_percent_cpnums->mode);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentCPNUMS   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCPNUMS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                         T_BAT_cmd_send    *cmd)
{
  return(ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPALS    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPALS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PercentCPALS()");
  /* adjust the left hand side
    xxx = cmd->params.ptr_set_percent_cpals->call_id;
  */
  /* hmm, do not know which ACI function to call ! */ 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPALS    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPALS(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cpals cpals;

  T_CPHS_LINES  active_line; 
  CHAR          *line_desc; 
  U8            max_line_desc = 30; /* should be enough ! */ 

  TRACE_FUNCTION ("qBAT_PercentCPALS()");

  MALLOC(line_desc, max_line_desc);

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CPALS; 
  resp.response.ptr_que_percent_cpals = &cpals;

  ret = (T_ACI_BAT_RSLT)qAT_PercentCPALS((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                          (U8)cpals.lines,
                          &active_line,
                          line_desc, 
                          &max_line_desc);
  MFREE(line_desc);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPVWI    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPVWI(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CPHS_LINES lines = CPHS_LINE_NULL;

  TRACE_FUNCTION ("sBAT_PercentCPVWI()");

  /* According to 8415_52.doc the parameter line is mandatory,
   * but implementation in ATI/ACI allows this one to be omittable.
   * So, BAT goes with them. In case of line is omitted, 
   * set it to all possible. Else leave the bitfield as is (BAT == ACI).
   */
  if (cmd->params.ptr_set_percent_cpvwi->lines EQ BAT_P_CPVWI_LINES_NOT_PRESENT)
  {
    lines = CPHS_LINE1 + CPHS_LINE_DATA + CPHS_LINE_FAX + CPHS_LINE2;   
  }
  else
  {
    lines = cmd->params.ptr_set_percent_cpvwi->lines;
  }

  if (cmd->params.ptr_set_percent_cpvwi->mode EQ BAT_P_CPVWI_MODE_QUERY)
  {
    ret = query_cphs_lines(src_infos_psi, cmd, lines);
  }
  else
  {  
    ret = (T_ACI_BAT_RSLT)sAT_PercentCPVWI((T_ACI_CMD_SRC)src_infos_psi->srcId, 
                           (U8)cmd->params.ptr_set_percent_cpvwi->mode,
                           lines);
  }

  return(ret);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPOPN    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPOPN(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cpopn cpopn;

  TRACE_FUNCTION ("qBAT_PercentCPOPN()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CPOPN; 
  resp.response.ptr_que_percent_cpopn = &cpopn;

  ret = (T_ACI_BAT_RSLT)qAT_PercentCPOPN((T_ACI_CMD_SRC)src_infos_psi->srcId,
                          (CHAR*)cpopn.long_name, 
                          &cpopn.c_long_name,
                          (CHAR*)cpopn.short_name, 
                          &cpopn.c_short_name);

  aci_bat_send(src_infos_psi, &resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPINF    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPINF(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_set_percent_cpinf *cpinf = cmd->params.ptr_set_percent_cpinf;
  UBYTE  serv_group[CPHS_MAX_CSP];
  UBYTE  serv_len = 0;

  TRACE_FUNCTION ("sBAT_PercentCPINF()");

  memset(serv_group,0,sizeof(serv_group));

  /* CSP exceeding maximum value display text too long */
  if ( cpinf->c_csp > BAT_MAX_CPINF_CSP_LEN )
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_TxtToLong);
    return(ACI_BAT_FAIL);
  }

  serv_len = utl_HexStrToBin((UBYTE*)cpinf->csp, cpinf->c_csp, serv_group, CPHS_MAX_CSP);
  
  /* Odd length CSP or empty CSP display operation not allowed */
  if(((serv_len % 2) NEQ 0) OR (serv_len EQ 0))
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return(ACI_BAT_FAIL);
  }

  ret = (T_ACI_BAT_RSLT)sAT_PercentCPINF( (T_ACI_CMD_SRC)src_infos_psi->srcId,
                          serv_group,
                          serv_len);

  return(ret);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPINF    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPINF(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cpinf cpinf;

  TRACE_FUNCTION ("qBAT_PercentCPINF()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CPINF; 
  resp.response.ptr_que_percent_cpinf = &cpinf;

  cpinf.phase = BAT_P_CPINF_PHASE_1;
  cpinf.sst   = 0;
  cpinf.c_csp = BAT_MAX_CPINF_CSP_LEN;/* Since the BAT_MAX_CPINF_CSP_LEN length has been updated to 22,then there is no need of BAT_MAX_CPINF_CSP2_LEN */  
  cpinf.c_csp2 = BAT_MAX_CPINF_CSP_LEN;
  memset(cpinf.csp, 0x00, BAT_MAX_CPINF_CSP_LEN);
  memset(cpinf.csp2, 0x00, BAT_MAX_CPINF_CSP_LEN);

  ret = (T_ACI_RETURN)qAT_PercentCPINF((T_ACI_CMD_SRC)src_infos_psi->srcId,
                          (U8*)&cpinf.phase,
                          &cpinf.sst, 
                          (CHAR*)cpinf.csp,
                          (CHAR*)cpinf.csp2,
                          &cpinf.c_csp,
                          &cpinf.c_csp2);

  if (ret EQ AT_CMPL)
  {
    aci_bat_send(src_infos_psi, &resp);
    return (ACI_BAT_CMPL);
  }

  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPMB     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPMB(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_CPHS_LINES line;
  CHAR number[CPHS_MAX_MB_NUMBER_LEN];
  CHAR alpha_id[CPHS_MAX_MB_ALPHA_LEN];
  T_ACI_TOA_TON ton;
  T_ACI_TOA_NPI npi;

  TRACE_FUNCTION ("sBAT_PercentCPMB()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PercentCPMB(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    (UBYTE)cmd->params.ptr_set_percent_cpmb->record_id,
    &line,number,&ton,&npi,alpha_id,NULL);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_percent_cpmb cpmb_data;
    UBYTE len;
    T_ACI_TOA toa;
 
    resp.ctrl_response=BAT_RES_SET_PERCENT_CPMB;
    resp.response.ptr_set_percent_cpmb=&cpmb_data;

    cpmb_data.record_id=(T_BAT_record_id)cmd->params.ptr_set_percent_cpmb->record_id;
    cpmb_data.line=(T_BAT_percent_cpmb_line)line;

    len=strlen(number);

    /*
    *   This is in line with the existing ATI implementation (see
    *   setatPercentCPMB()) and is necessary to pass tests such as
    *   ACI583.
    */ 
    if (len EQ 0)
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_NotFound);
      return(ACI_BAT_FAIL);
    }

    /*
    *   Check that the number isn't too long for the BAT message, 
    *   if it is it will effectively be truncated.
    */
    if (len>BAT_MAX_CPMB_NUMBER_LEN)
      len=BAT_MAX_CPMB_NUMBER_LEN;

    if (len)
    {
      memcpy(cpmb_data.number,number,len);
      cpmb_data.v_number=TRUE;
      cpmb_data.c_number=(U8)len;
    }
    else
    {
      cpmb_data.v_number=FALSE;
    }

    len=strlen(alpha_id);

    if (len)
    {
      USHORT len_cvtd_text=0;

      /*
      *   Convert the data read from the SIM into the currently 
      *   selected character set.
      */
      utl_chsetFromSim(
        (UBYTE *)alpha_id,
        (USHORT)len,
        (UBYTE *)cpmb_data.alpha_id,
        BAT_MAX_CPMB_ALPHA_ID_LEN,
        &len_cvtd_text,
        GSM_ALPHA_Def);

      cpmb_data.v_alpha_id=TRUE;
      cpmb_data.c_alpha_id=(U8)len_cvtd_text;
    }
    else
    {
      cpmb_data.v_alpha_id=FALSE;
    }

    /*
    *   Get the type of number and numbering plan into a T_ACI_TOA structure
    *   so that we can use toa_merge().
    */
    toa.ton=ton;
    toa.npi=npi;

    /*
    *   BAT stores the type of address in a single 8-bit value (it is S16 in
    *   the message only to allow for a 'not present' value of -1).
    */
    cpmb_data.type=(S16)toa_merge(toa);

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPMB     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPMB(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                       T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  UBYTE first;

  TRACE_FUNCTION ("qBAT_PercentCPMB()");

  /*
  *   Call the corresponding qAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PercentCPMB(
         (T_ACI_CMD_SRC)src_infos_psi->srcId,
    (UBYTE)ACI_NumParmNotPresent,NULL,NULL,NULL,NULL,NULL,&first);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_cpmb cpmb_data;
 
    resp.ctrl_response=BAT_RES_QUE_PERCENT_CPMB;
    resp.response.ptr_que_percent_cpmb=&cpmb_data;

    cpmb_data.first=(U8)first;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPMBW    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPMBW(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_set_percent_cpmbw *cpmbw = cmd->params.ptr_set_percent_cpmbw;
  T_ACI_TOA type;
  T_ACI_TOA *p_type;
  CHAR number[BAT_MAX_CPMBW_NUMBER_LEN+1];
  CHAR *p_number;
  T_CPHS_PB_TEXT text;
  T_CPHS_PB_TEXT *p_text;

  TRACE_FUNCTION ("sBAT_PercentCPMBW()");

  if (cpmbw->type EQ -1)
  {
    p_type=NULL;
  }
  else
  {
    p_type=&type;
    type=toa_demerge(cpmbw->type);
  }

  if (cpmbw->v_number EQ FALSE)
  {
    p_number=NULL;
  }
  else
  {
    /*
    *   Mailbox number is present, so make a zero-terminated version
    *   and set a pointer to it.
    */
    p_number=number;
    memset(number,0,sizeof(number));
    memcpy(number,cpmbw->number,cpmbw->c_number);
  }

  if (cpmbw->v_text EQ FALSE)
  {
    p_text=NULL;
  }
  else
  {
    USHORT len;

    p_text=&text;

    /*
    *   Convert from the currently selected character set into the
    *   SIM format.
    */
    utl_chsetToSim(
      (UBYTE *)cpmbw->text,
      (USHORT)cpmbw->c_text,
      (UBYTE *)text.data,
      &len,
      GSM_ALPHA_Def);

    if (len>CPHS_MAX_MB_ALPHA_LEN)
    {
      /*
      *   Shouldn't get here as that would mean that memory has been
      *   overwritten. Currently utl_chsetToSim() has no specific mechanism
      *   to prevent this.
      */
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_TxtToLong);
      return(ACI_BAT_FAIL);
    }
          
    /*
    *   Set the length. Note that we cannot do this by passing a
    *   reference to text.len into utl_chsetToSim() because it is
    *   the wrong type.
    */
    text.len=(UBYTE)len;
    
    p_text=&text;
  }

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PercentCPMBW(
    (T_ACI_CMD_SRC)src_infos_psi->srcId, 
    (SHORT)cpmbw->record_id,
    p_number,
    p_type,
    p_text);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PercentCPMBW    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PercentCPMBW(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  SHORT          first_idx = 0;
  SHORT          last_idx  = 0;
  UBYTE          tlength   = 0;
  UBYTE          nlength   = 0;

  TRACE_FUNCTION ("tBAT_PercentCPMBW()");

  ret=(T_ACI_BAT_RSLT)tAT_PercentCPMBW((T_ACI_CMD_SRC)src_infos_psi->srcId,
                                       &first_idx,
                                       &last_idx,
                                       &nlength,
                                       &tlength);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_tst_percent_cpmbw cpmbw_data;
 
    resp.ctrl_response = BAT_RES_TST_PERCENT_CPMBW;
    resp.response.ptr_tst_percent_cpmbw = &cpmbw_data;

    cpmbw_data.min_rec_id = (U8)first_idx;
    cpmbw_data.max_rec_id = (U8)last_idx;
    cpmbw_data.nlength    = nlength;
    cpmbw_data.tlength    = tlength;

    /* 
     * currently c_type_range and type_range are initialized to zero
     * TODO : %CPMBW types
     */
    cpmbw_data.c_type_range = 0;
    cpmbw_data.type_range[0] = 0;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCPCFU    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCPCFU(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CPHS_LINES lines = CPHS_LINE_NULL;

  TRACE_FUNCTION ("sBAT_PercentCPCFU()");

  /* According to 8415_52.doc the parameter line is mandatory,
   * but implementation in ATI/ACI allows this one to be omittable.
   * So, BAT goes with them. In case of line is omitted, 
   * set it to all possible. Else leave the bitfield as is (BAT == ACI).
   */
  if (cmd->params.ptr_set_percent_cpcfu->line EQ BAT_P_CPCFU_LINE_NOT_PRESENT)
  {
    lines = CPHS_LINE1 + CPHS_LINE_DATA + CPHS_LINE_FAX + CPHS_LINE2;   
  }
  else
  {
    lines = cmd->params.ptr_set_percent_cpcfu->line;
  }

  if (cmd->params.ptr_set_percent_cpcfu->mode EQ BAT_P_CPCFU_MODE_QUERY)
  {
    ret = query_cphs_lines(src_infos_psi, cmd, lines);
  }
  else
  {  
    ret = (T_ACI_BAT_RSLT)sAT_PercentCPCFU((T_ACI_CMD_SRC)src_infos_psi->srcId,
                           (U8)cmd->params.ptr_set_percent_cpcfu->mode,
                            lines);
  }
  return(ret);
}



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
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"     /* MAKROS ACI_MALLOC, ACI_MFREE*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentPBCF     |
+--------------------------------------------------------------------+

  PURPOSE : Select Phonebook Configuration.
*/

GLOBAL T_ACI_BAT_RSLT sBAT_PercentPBCF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_PBCF_LDN ldn = PBCF_LDN_NotPresent;
  T_ACI_PBCF_LRN lrn = PBCF_LRN_NotPresent;
  T_ACI_PBCF_LMN lmn = PBCF_LMN_NotPresent;

  TRACE_FUNCTION ("sBAT_PercentPBCF()");

  ldn = (T_ACI_PBCF_LDN)cmd->params.ptr_set_percent_pbcf->ldn;
  lrn = (T_ACI_PBCF_LRN)cmd->params.ptr_set_percent_pbcf->lrn;
  lmn = (T_ACI_PBCF_LMN)cmd->params.ptr_set_percent_pbcf->lmn;

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  /* for BAT we use the old spec, which means the third param is not supported */
  ret = (T_ACI_BAT_RSLT)sAT_PercentPBCF((T_ACI_CMD_SRC)src_infos_psi->srcId, ldn, lrn, lmn);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPBS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentPBCF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_PBCF_LDN ldn = PBCF_LDN_NotPresent;
  T_ACI_PBCF_LRN lrn = PBCF_LRN_NotPresent;
  T_ACI_PBCF_LMN lmn = PBCF_LMN_NotPresent;

  TRACE_FUNCTION ("qBAT_PercentPBCF()");

  /*
  *   Call the corresponding qAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PercentPBCF((T_ACI_CMD_SRC)src_infos_psi->srcId, &ldn, &lrn, &lmn);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_pbcf pbcf_data;
 
    resp.ctrl_response=BAT_RES_QUE_PERCENT_PBCF;
    resp.response.ptr_que_percent_pbcf = &pbcf_data;

    /*
    *   This relies on T_BAT_percent_pbcf_ldn being identical to 
    *   T_ACI_PBCF_LDN and so forth for LRN and LMN.
    */
    pbcf_data.ldn=(T_BAT_percent_pbcf_ldn)ldn; 
    pbcf_data.lrn=(T_BAT_percent_pbcf_lrn)lrn; 
    pbcf_data.lmn=(T_BAT_percent_pbcf_lmn)lmn; 

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPBS        |
+--------------------------------------------------------------------+

  PURPOSE : Select phonebook memory storage.
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_PB_STOR stor;

  TRACE_FUNCTION ("sBAT_PlusCPBS()");

  /*
  *   This relies on T_BAT_storage being identical to 
  *   T_ACI_PB_STOR.
  */
  stor=(T_ACI_PB_STOR)cmd->params.ptr_set_plus_cpbs->storage;

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  /* for BAT we use the old spec, which means the third param is not supported */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCPBS((T_ACI_CMD_SRC)src_infos_psi->srcId,stor, NULL);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPBS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPBS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_PB_STOR stor;
  SHORT used;
  SHORT total;

  TRACE_FUNCTION ("qBAT_PlusCPBS()");

  /*
  *   Call the corresponding qAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCPBS((T_ACI_CMD_SRC)src_infos_psi->srcId,&stor,&used,&total);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_cpbs cpbs_data;
 
    resp.ctrl_response=BAT_RES_QUE_PLUS_CPBS;
    resp.response.ptr_que_plus_cpbs=&cpbs_data;

    /*
    *   This relies on T_BAT_storage being identical to 
    *   T_ACI_PB_STOR.
    */
    cpbs_data.storage=(T_BAT_storage)stor; 

    cpbs_data.total=(S16)total;
    cpbs_data.used=(S16)used;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCPBS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCPBS (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                        T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_PB_STOR stor;
  SHORT used;
  SHORT total;
  SHORT first;
  SHORT used_ext;
  SHORT total_ext;

  TRACE_FUNCTION ("qBAT_PercentCPBS()");

  /*
  *   Call the corresponding qAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PercentCPBS((T_ACI_CMD_SRC)src_infos_psi->srcId,&stor,
                                      &used,&total,
                                      &first,
                                      &used_ext, &total_ext);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_cpbs cpbs_data;
 
    resp.ctrl_response=BAT_RES_QUE_PERCENT_CPBS;
    resp.response.ptr_que_percent_cpbs=&cpbs_data;

    /*
    *   This relies on T_BAT_storage being identical to 
    *   T_ACI_PB_STOR.
    */
    cpbs_data.storage=(T_BAT_storage)stor; 

    cpbs_data.total=(S16)total;
    cpbs_data.used=(S16)used;
    cpbs_data.first=(S16)first;
    cpbs_data.used_ext=(S16)used_ext;
    cpbs_data.total_ext=(S16)total_ext;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPBW        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBW (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  SHORT index;
  CHAR number[BAT_MAX_CPBW_NUMBER_LEN+1];
  CHAR *p_number;
  T_ACI_TOA toa;
  T_ACI_TOA *p_toa;
  T_ACI_PB_TEXT text;
  T_ACI_PB_TEXT *p_text;
  T_ACI_VP_ABS dmy_date_time;
  T_BAT_cmd_set_plus_cpbw *cpbw;

  TRACE_FUNCTION ("sBAT_PlusCPBW()");

  /*
  *   Get a pointer to the CPBW structure for convenience.
  */
  cpbw=cmd->params.ptr_set_plus_cpbw;

  index=(SHORT)cpbw->index;

  if (cpbw->v_number EQ TRUE)
  {
    /*
    *   The idea here is to get the 'number' parameter into a
    *   null terminated string, which is what sAT_PlusCPBW() expects, but
    *   is not guaranteed by BAT.
    */
    memset(number,0,sizeof(number));
    memcpy(number,cpbw->number,cpbw->c_number);
    p_number=number;

    /*
    *   BAT stores the type of address in a single value.
    */
    if (cpbw->type NEQ -1)
    {
      toa=toa_demerge(cpbw->type);
      p_toa=&toa;
    }
    else
    {
      p_toa=NULL;
    }
  }
  else
  {
    p_number=NULL;
    p_toa=NULL;
  } 

  /*
  *   Associated text
  */
  if (cpbw->v_text EQ TRUE)
  {
    USHORT len;

    /*
    *   Convert from the currently selected character set into the
    *   SIM format.
    */
    utl_chsetToSim(
      (UBYTE *)cpbw->text,
      (USHORT)cpbw->c_text,
      (UBYTE *)text.data,
      &len,
      GSM_ALPHA_Def);
          
    /*
    *   Set the length. Note that we cannot do this by passing a
    *   reference to text.len into utl_chsetToSim() because it is
    *   the wrong type.
    */
    text.len=(UBYTE)len;      
    
    text.cs=CS_Sim;

    p_text=&text;
  }
  else
  {
    p_text=NULL;
  }

  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCPBW(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    index,
    p_number,
    p_toa,
    p_text,
    &dmy_date_time);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCPBW        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBW        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  SHORT first;
  SHORT last;
  UBYTE nlength;
  UBYTE tlength;

  TRACE_FUNCTION ("tBAT_PlusCPBW()");

  /*
  *   Call the corresponding tAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)tAT_PlusCPBW((T_ACI_CMD_SRC)src_infos_psi->srcId,&first,&last,&nlength,&tlength);

  /*
  *   If the command completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_tst_plus_cpbw cpbw_data;
 
    resp.ctrl_response=BAT_RES_TST_PLUS_CPBW;
    resp.response.ptr_tst_plus_cpbw=&cpbw_data;

    cpbw_data.index1=(U8)first;
    cpbw_data.index2=(U8)last;
    cpbw_data.nlength=(S16)nlength;
    cpbw_data.tlength=(S16)tlength;
#ifdef _SIMULATION_
#pragma message(__TODO__"+CPBW types")  
#endif
    cpbw_data.c_types=0;
    cpbw_data.types[0]=0;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPBR        |
+--------------------------------------------------------------------+

  PURPOSE : Read phonebook entries. If the application wants to read
            more than ACI_BAT_MAX_RESPONSES, then the responses are
            sent block by block (one block = ACI_BAT_MAX_RESPONSES).
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBR_CB (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cpbr cpbr;

  cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBR;
  cmd.params.ptr_set_plus_cpbr = &cpbr;

  cpbr.index1 = (U8)src_infos_psi->index1;
  cpbr.index2 = (S16)src_infos_psi->index2;

  return (sBAT_PlusCPBR(src_infos_psi, &cmd));
}


GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBR (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  SHORT          start_idx;
  SHORT          stop_idx;
  SHORT          last_idx = ACI_NumParmNotPresent;
  T_ACI_PB_ENTR  pb_lst[MAX_PB_ENTR];

  TRACE_FUNCTION ("sBAT_PlusCPBR()");

  /* define here the variables, which are passed as parameters to sAT_PlusCPBR */
  start_idx = (SHORT)cmd->params.ptr_set_plus_cpbr->index1;
  stop_idx  = (SHORT)cmd->params.ptr_set_plus_cpbr->index2;
  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCPBR((T_ACI_CMD_SRC)src_infos_psi->srcId, start_idx, stop_idx, &last_idx, &pb_lst[0]);
  if (ret EQ ACI_BAT_CMPL)
  {
    SHORT i;
    T_BAT_cmd_response resp;
    T_BAT_res_set_plus_cpbr cpbr_data;
  
    resp.ctrl_response = BAT_RES_SET_PLUS_CPBR;
    resp.response.ptr_set_plus_cpbr = &cpbr_data;
  
    /*
     * sAT_PlusCPBR() provides in pb_lst[] up to five entries (MAX_PB_ENTR)
     */
    for (i = 0; ((i < MAX_PB_ENTR) AND (pb_lst[i].index NEQ ACI_NumParmNotPresent)); i++)
    {
      USHORT len_cvtd_text=0;
 
      cpbr_data.index = (U8)pb_lst[i].index;
  
      cpbr_data.c_number = strlen((CHAR*)pb_lst[i].number);
      
      cpbr_data.c_number = (cpbr_data.c_number > BAT_MAX_CPBR_NUMBER_LEN) 
                            ? BAT_MAX_CPBR_NUMBER_LEN 
                            : cpbr_data.c_number;

      strncpy((CHAR*)cpbr_data.number, pb_lst[i].number, cpbr_data.c_number+1);  /* Include null terminator for Resp_.. () */
  
      /* BAT stores the type of address in a single 8-bit value. */
      cpbr_data.type = toa_merge(pb_lst[i].type);
     
      /*
      *   Convert the data read from the SIM into the currently 
      *   selected character set.
      */
      utl_chsetFromSim(
        (UBYTE *)pb_lst[i].text.data,
        pb_lst[i].text.len,
        (UBYTE *)cpbr_data.text,
        BAT_MAX_CPBR_TEXT_LEN,
        &len_cvtd_text,
        GSM_ALPHA_Def);

      cpbr_data.c_text=(U8)len_cvtd_text;

      TRACE_EVENT_P2("sBAT_PlusCPBR(): index = %i c_text = %d", cpbr_data.index, cpbr_data.c_text);  
      
      /* vsi_t_sleep (aci_handle, 500); */
      aci_bat_send(src_infos_psi,&resp);
    }
    /* remember updated start index and initial stop index for possible next block of entries to send to APP */
    src_infos_psi->index1 = last_idx + 1;
    src_infos_psi->index2 = stop_idx;
  }
  else
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);      
  }

  if (last_idx < stop_idx)
  {
    src_infos_psi->large_type = TRUE;
    return (ACI_BAT_EXCT);
  }

  src_infos_psi->large_type = FALSE;
  return (ACI_BAT_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCPBR        |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBR (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret=ACI_BAT_FAIL;
  SHORT first;
  SHORT last;
  UBYTE nlength;
  UBYTE tlength;

  TRACE_FUNCTION ("tBAT_PlusCPBR()");

  /*
  *   Call the corresponding tAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)tAT_PlusCPBR((T_ACI_CMD_SRC)src_infos_psi->srcId,&first,&last,&nlength,&tlength);

  /*
  *   If the command completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_tst_plus_cpbr cpbr_data;
 
    resp.ctrl_response=BAT_RES_TST_PLUS_CPBR;
    resp.response.ptr_tst_plus_cpbr=&cpbr_data;

    cpbr_data.index_f=(U8)first;
    cpbr_data.index_l=(U8)last;
    cpbr_data.nlength=(S16)nlength;
    cpbr_data.tlength=(S16)tlength;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPBF        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBF_CB (T_ACI_DTI_PRC_PSI *src_infos_psi)
{
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cpbf cpbf;

  cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBF;
  cmd.params.ptr_set_plus_cpbf = &cpbf;

  TRACE_FUNCTION ("sBAT_PlusCPBF_CB()");

  /* restore search string if ongoing command */
  if ( src_infos_psi->search_str_ptr NEQ NULL )
  {
    TRACE_EVENT("Restore search string"); 
    /* index 1 stores the length of the string in the case of cpbf */
    cpbf.c_findtext = (U8)src_infos_psi->index1;
    memcpy(cpbf.findtext, src_infos_psi->search_str_ptr, BAT_MAX_CPBF_FIND_TEXT_LEN);
  }
  return (sBAT_PlusCPBF(src_infos_psi, &cmd));
}
  
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPBF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_PB_TEXT pbtext;
  USHORT len;
  T_ACI_BAT_RSLT ret;
  T_BAT_cmd_set_plus_cpbf *cpbf_cmd;
  SHORT found = -1;
  T_ACI_PB_ENTR pb[MAX_PB_ENTR];
  SHORT i;
  T_ACI_CPBF_MOD search_mode; // replaced by global var
  T_BAT_cmd_response resp;
  T_BAT_res_set_plus_cpbf cpbf_rsp;

  TRACE_FUNCTION ("sBAT_PlusCPBF()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CPBF;
  resp.response.ptr_set_plus_cpbf = &cpbf_rsp;

  /*
  *   Get a pointer to the CPBF data for convenience.
  */
  cpbf_cmd = cmd->params.ptr_set_plus_cpbf;

  /*
  *   We may have to call sAT_PlusCPBF() more than once, if there are
  *   more than MAX_PB_ENTR entries found. The search mode is used
  *   to tell the ACI whether this is a new search or a continuation of
  *   the old one.
  *   If there has been a valid string pointer saved to the search_str_ptr
  *   the seek has to be continued. 
  */

  if  ((src_infos_psi->search_str_ptr NEQ NULL)
   AND (strncmp((char *)src_infos_psi->search_str_ptr, (char *)cpbf_cmd->findtext, cpbf_cmd->c_findtext) EQ 0))
  {
    found = src_infos_psi->index2; /* recover number found*/
    search_mode = CPBF_MOD_NextSearch;
    TRACE_EVENT_P1("Stored Search string: %s", src_infos_psi->search_str_ptr);
    TRACE_EVENT_P1("Stored String length: %d", src_infos_psi->index1);
  }
  else
  {
    search_mode = CPBF_MOD_NewSearch;
  }
  
  /*
  *   If the search string has zero length or is larger than the ACI function
  *   can handle then there's no point continuing.
  *   In fact there should be a differentiation between UCS2 and 8bit based coding schemes. 
  *   Additionally it has to be considered that the string given within the BAT command is ACSII encoded. 
  */
 if (((ati_user_output_cfg[src_infos_psi->srcId].cscsChset EQ CSCS_CHSET_Ucs2)
        AND ((cpbf_cmd->c_findtext>BAT_MAX_CPBF_FIND_TEXT_LEN) OR (cpbf_cmd->c_findtext<1)) )
      OR
      ((ati_user_output_cfg[src_infos_psi->srcId].cscsChset NEQ CSCS_CHSET_Ucs2)
        AND ((cpbf_cmd->c_findtext>MAX_PHB_NUM_LEN) OR (cpbf_cmd->c_findtext<1)) ) )
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(ACI_BAT_FAIL);
  }

  TRACE_EVENT_P1("Search string: %s", cpbf_cmd->findtext);
  TRACE_EVENT_P1("String length: %d", cpbf_cmd->c_findtext);

  utl_chsetToSim(
    (UBYTE *)cpbf_cmd->findtext,
    (USHORT)cpbf_cmd->c_findtext,
    (UBYTE *)pbtext.data,
    &len,
    GSM_ALPHA_Def);

  if (len > MAX_PHB_NUM_LEN)
  {
    /*
    *   Shouldn't get here as that would mean that memory has been
    *   overwritten. Currently utl_chsetToSim() has no specific mechanism
    *   to prevent this.
    */
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_TxtToLong);
    return(ACI_BAT_FAIL);
  }

  pbtext.len = (UBYTE)len;
  pbtext.cs  = (T_ACI_CS)CS_Sim;

  
  /*
  *   Call the corresponding sAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */

  TRACE_EVENT_P1("search_mode: %d", search_mode);  
  TRACE_EVENT_P1("pbtext.data: %s", pbtext.data);
  TRACE_EVENT_P1("pbtext.len:  %d", pbtext.len);


  /* found is not overwritten in the case of NEXT_SEARCH */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCPBF(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    &pbtext,
    search_mode,
    &found,
    pb);

  TRACE_EVENT_P1("sAT_PlusCPBF returns: %d", ret);
  TRACE_EVENT_P1("-- found entries: %d", found);

  /*
  *   If the command fails, get out now.
  */
  if (ret NEQ ACI_BAT_CMPL)
    return(ret);

  /*
  *   We have received a list, however the BAT response only
  *   allows for single items. So we must send each one 
  *   individually.
  */
  for (i=0;((i<found) AND (i<MAX_PB_ENTR));i++)
  {
    USHORT len_cvtd_text=0;

    cpbf_rsp.index=(U8)pb[i].index;

    TRACE_EVENT("send the found entry"); 
    strncpy((CHAR*)cpbf_rsp.number,pb[i].number,BAT_MAX_CPBF_NUMBER_LEN);
    cpbf_rsp.c_number=strlen((CHAR*)cpbf_rsp.number);

    /*
    *   BAT stores the type of address in a single 8-bit value.
    */
    cpbf_rsp.type=toa_merge(pb[i].type);

    /*
    *   Convert the data read from the SIM into the currently 
    *   selected character set.
    */
    utl_chsetFromSim(
      (UBYTE *)pb[i].text.data,
      pb[i].text.len,
      (UBYTE *)cpbf_rsp.text,
      BAT_MAX_CPBF_TEXT_LEN,
      &len_cvtd_text,
      GSM_ALPHA_Def);

    cpbf_rsp.c_text=(U8)len_cvtd_text;

    /* vsi_t_sleep (aci_handle, 500); */

    aci_bat_send(src_infos_psi,&resp);
  }

  /*
  *   Set the search mode for the next iteration, should there be
  *   one.
  */
   
  if (found>MAX_PB_ENTR)
  {
    TRACE_EVENT("More entries found"); 
    /* remember the string to search for (only necessary for first iteration)*/
    if (search_mode EQ CPBF_MOD_NewSearch)
    {
      TRACE_EVENT("Store string"); 
      src_infos_psi->index1 = (U16)cpbf_cmd->c_findtext; /* str length */
      ACI_MALLOC(src_infos_psi->search_str_ptr, BAT_MAX_CPBF_FIND_TEXT_LEN); 
      memcpy(src_infos_psi->search_str_ptr, cpbf_cmd->findtext, BAT_MAX_CPBF_FIND_TEXT_LEN);
    }
    /* This is a multi-line response */
    src_infos_psi->large_type = TRUE;
    /* Save the number of found items - MAX_PB_ENTR send to user */
    src_infos_psi->index2 = found - MAX_PB_ENTR; 
    /* Still executing */
    ret = ACI_BAT_EXCT;
  }
  else /* no further entries to find */
  {
    TRACE_EVENT("reset search string"); 
    /* reset search string info for iterations */
    src_infos_psi->index1 = 0; /* str length reset */
    src_infos_psi->index2 = 0; /* number found reset */
    if (src_infos_psi->search_str_ptr NEQ NULL) /* str mem reset, if necessary */
    {
      ACI_MFREE(src_infos_psi->search_str_ptr);
      src_infos_psi->search_str_ptr = NULL;
    }
    /* This is NO multiline response */
    src_infos_psi->large_type = FALSE;
    /* completed */
    ret = ACI_BAT_CMPL;
  }
  
  return (ret);  
  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCPBF        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCPBF (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  UBYTE nlength;
  UBYTE tlength;

  TRACE_FUNCTION ("tBAT_PlusCPBF()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)tAT_PlusCPBF((T_ACI_CMD_SRC)src_infos_psi->srcId,&nlength,&tlength);

  /*
  *   If the command completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_tst_plus_cpbf cpbf_data;
 
    resp.ctrl_response=BAT_RES_TST_PLUS_CPBF;
    resp.response.ptr_tst_plus_cpbf=&cpbf_data;

    cpbf_data.nlength=(S16)nlength;
    cpbf_data.tlength=(S16)tlength;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSVM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSVM (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_TOA toa;
  CHAR number[BAT_MAX_CSVM_NUMBER_LEN+1];
  CHAR *pnumber;
  T_ACI_CSVM_MOD mode;
  T_BAT_cmd_set_plus_csvm *csvm;
  UBYTE num_len;

  TRACE_FUNCTION ("sBAT_PlusCSVM()");

  /*
  *   Get a pointer to the BAT structure for convenience.
  */
  csvm=cmd->params.ptr_set_plus_csvm;

  /*
  *   This assumes that T_ACI_CSVM_MOD and T_BAT_plus_csvm_mode are
  *   equivalent. They are, except that the ACI version has a
  *   NotPresent value - this is not a problem here as we are converting
  *   from BAT format to ACI format.
  */
  mode=(T_ACI_CSVM_MOD)csvm->mode;

  if (csvm->v_number EQ TRUE)
  {
    /*
    *   I don't think that CSVM requires a null-terminated string, but
    *   it's best to err on the side of caution.
    */
    pnumber=number;
    memset(number,0,sizeof(number));
    memcpy(number,csvm->number,BAT_MAX_CSVM_NUMBER_LEN);

    if (csvm->c_number>BAT_MAX_CSVM_NUMBER_LEN)
      num_len=(UBYTE)BAT_MAX_CSVM_NUMBER_LEN;
    else
      num_len=(UBYTE)csvm->c_number;
  }
  else
  {
    pnumber=NULL;
    num_len=0;
  }

  /*
  *   BAT stores the type of address in a single 8-bit value. We need to
  *   convert this to the ACI format.
  */
  toa=toa_demerge(csvm->type);

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCSVM((T_ACI_CMD_SRC)src_infos_psi->srcId,mode,pnumber,num_len,&toa);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSVM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSVM (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CSVM_MOD mode;
  CHAR number[BAT_MAX_CSVM_NUMBER_LEN+1];
  SHORT toa_val;

  TRACE_FUNCTION ("qBAT_PlusCSVM()");

  /*
  *   Call the corresponding qAT function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCSVM(
    (T_ACI_CMD_SRC)src_infos_psi->srcId,
    &mode,
    number,
    BAT_MAX_CSVM_NUMBER_LEN,
    &toa_val);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_csvm csvm_data;
 
    resp.ctrl_response=BAT_RES_QUE_PLUS_CSVM;
    resp.response.ptr_que_plus_csvm=&csvm_data;

    /*
    *   We can't just cast the ACI value to the BAT value as it
    *   might be CSVM_MOD_NotPresent, for which the BAT has no
    *   equivalent.
    */
    csvm_data.mode=(mode EQ CSVM_MOD_Enable) ? BAT_CSVM_MODE_ENABLE:BAT_CSVM_MODE_DISABLE;

    /*
    *   We trust the ACI to give us a null-terminated string.
    */
    strcpy((CHAR*)csvm_data.number,number);
    csvm_data.c_number=strlen((CHAR*)csvm_data.number);

    /*
    *   Note that in this instance the ACI is giving us the TOA as
    *   a single number, rather than in its usual T_ACI_TOA format.
    */
    csvm_data.type=(U8)toa_val;

    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCLAN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLAN (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_LAN_SUP lng;
  CHAR str[BAT_MAX_CLAN_CODE_LEN+1];
  USHORT i;

  TRACE_FUNCTION ("sBAT_PlusCLAN()");

  /*
  *   Ensure that we have a null-terminated string.
  */
  memset(str,0,sizeof(str));
  memcpy(str,cmd->params.ptr_set_plus_clan->code,BAT_MAX_CLAN_CODE_LEN);

  lng.str=NULL;

  /*
  *   The BAT provides the language in string form only, we must also
  *   calculate the corresponding value. To do this we use the lookup
  *   table "lngs".
  *
  *   GSM 07.07 says that "AUTO" should be used for automatic mode, but
  *   the (ATI) code uses "au". Here we want to accept both.
  */  
  if (!strcmp(str,"AUTO"))
  {
    lng.lng=CLAN_LNG_AUT;
    lng.str="AUTO";
  }
  else
  {
    for (i=0;lngs[i].str;i++)
    {
      if (!strcmp(lngs[i].str,str))
      {
        lng.str=lngs[i].str;
        lng.lng=lngs[i].lng;
        break;
      }
    }
  }

  if (lng.str==NULL)
  {
    /*
    *   Can't work out the language so give up.
    */
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotSupp);
    return(ACI_BAT_FAIL);
  }

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)sAT_PlusCLAN((T_ACI_CMD_SRC)src_infos_psi->srcId,&lng);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCLAN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLAN (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_LAN_SUP lng;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clan clan_data;

  TRACE_FUNCTION ("qBAT_PlusCLAN()");

  resp.ctrl_response=BAT_RES_QUE_PLUS_CLAN;
  resp.response.ptr_que_plus_clan=&clan_data;

  memset(&clan_data,0,sizeof(clan_data));
  lng.str=(CHAR *)clan_data.code;

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  *   We are trusting this function not to deliver too long
  *   a string. At the time of writing, it can only give us two
  *   characters, which we easily have room for in the BAT
  *   message.
  */
  ret=(T_ACI_BAT_RSLT)qAT_PlusCLAN((T_ACI_CMD_SRC)src_infos_psi->srcId,&lng);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    clan_data.c_code=strlen(lng.str);
    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCLAN        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCLAN (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                     T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_ACI_LAN_SUP lng[MAX_LAN];
  SHORT last_idx;

  TRACE_FUNCTION ("tBAT_PlusCLAN()");

  /*
  *   Call the corresponding ACI function. T_ACI_BAT_RSLT is
  *   assumed to be equivalent to T_ACI_RESULT.
  */
  ret=(T_ACI_BAT_RSLT)tAT_PlusCLAN((T_ACI_CMD_SRC)src_infos_psi->srcId,&last_idx,lng);

  /*
  *   If the command completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    USHORT i;
    T_BAT_cmd_response resp;
    T_BAT_res_tst_plus_clan clan_data;
 
    resp.ctrl_response=BAT_RES_TST_PLUS_CLAN;
    resp.response.ptr_tst_plus_clan=&clan_data;

    /*
    *   We have received a list, however the BAT response only
    *   allows for single items. So we must send each one 
    *   individually.
    */
    for (i=0;i<last_idx;i++)
    {
      clan_data.c_code=strlen(lng[i].str);

      /*
      *   Discard any languages for which the string is too long for
      *   BAT (this should never happen).
      */
      if (clan_data.c_code<=BAT_MAX_CLAN_CODE_LEN)
      {
        memcpy(clan_data.code,lng[i].str,BAT_MAX_CLAN_CODE_LEN);
        aci_bat_send(src_infos_psi,&resp);
      }
    }
  }

  return(ret);
}


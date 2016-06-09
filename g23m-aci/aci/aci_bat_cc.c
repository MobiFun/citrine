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
 
#include "aci_all.h"
#undef FF_ATI
#include "aci_cmh.h"
#include "aci_cmd.h"
#include "aci.h"
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */
#include "aci_bat.h"  
#ifdef _SIMULATION_
#pragma message(__TODO__"aci_prs.h: EXTERN char *parse(char *b,char *f, ...); of ati_prs.c !")    
#endif 
#include "aci_prs.h" 

/* implemented in aci_util.c, but prototype in ati_int.h ! Header file cleanup needed !  */
EXTERN CHAR *strupper (CHAR *s);

/*================== local functions prototypes =============================*/

LOCAL T_ACI_BAT_RSLT aci_bat_call_from_phonebook(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                                 T_BAT_cmd_send    *cmd);

LOCAL T_ACI_BAT_RSLT aci_bat_call_with_number   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                                 T_BAT_cmd_send    *cmd);

LOCAL  void aci_bat_chld_mode_mapping (T_BAT_cmd_set_percent_chld* bat_chld_param,
                                       T_ACI_CHLD_MOD    *mode);
/*================== local functions ========================================*/

/*
 * special hint:
 * There is hard coded the definition of NO_ASCIIZ (see phb.h, aci.h and aci_cmh.h), 
 * which means it is always defined regardless what busybe configures.
 * So, we never work with NULL terminated strings, 
 * but with character arrays, which have a length information in addition
 * to know how many characters to process.
 * In this code there is several times this #ifdef  NO_ASCIIZ
 * just to go with the current implementation of ACI.
 * But NULL terminated strings are not covered !
 */

LOCAL void aci_bat_clean_dial_str (char *dial_str, int dial_len)
{
#define VALID_CHARS "*#+-PpWw"
  int   i,j,k = 0;
  int   charLen  = sizeof(VALID_CHARS);
  char  *valChrs = VALID_CHARS;
  int   match    = FALSE;

  for (i=0; i<dial_len; i++)
  {
    match = FALSE;
    if ((dial_str[i] < '0') || (dial_str[i] > 'D'))
    {    
      for (j=0; j<charLen; j++) 
      {
        if (dial_str[i] == valChrs[j]) /* "*#+-PpWw" */
        {
          match = TRUE;
          break;
        }
      }
    }
    else  /* "0123456789ABCD" */
    {
      match = TRUE;
    }
    if (match)
    {
      dial_str[k] = dial_str[i];
      k++;
    }
  }
  if (k < dial_len)
  {
    dial_str[k] = 0x0; /* is there always 1 byte space for the '\0' ? */
  }
}



LOCAL void aci_bat_check_clir_cug_type (U8                 *dial_string, 
                                        T_ACI_D_CLIR_OVRD  *clir_ovrd, 
                                        T_ACI_D_CUG_CTRL   *cug_ctrl, 
                                        T_ACI_D_TOC        *call_type)
{
  if (*dial_string EQ 'i')
  {
    *clir_ovrd = D_CLIR_OVRD_Supp;
    dial_string++;
  }
  if (*dial_string EQ 'I')
  {
    *clir_ovrd = D_CLIR_OVRD_Invoc;
    dial_string++;
  }
  if (*dial_string EQ 'g'OR *dial_string EQ 'G')
  {
    *cug_ctrl = D_CUG_CTRL_Present;
    dial_string++;
  }
  if (*dial_string EQ ';')
  {
    *call_type = D_TOC_Voice;
  }
}

#define NAME_BUF_LEN 32

LOCAL T_ACI_BAT_RSLT aci_bat_call_from_phonebook(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                                 T_BAT_cmd_send    *cmd)
{
  T_bat_aci_client_maintain *bat_client;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR  *dial_string   = NULL;
  int  i;
  S16  index         = -1;
  S16  pos;
  char alpha[NAME_BUF_LEN] = {'\0'};
  U16  lenAlpha            = 0;
  char cvtdAlpha[NAME_BUF_LEN] = {'\0'};
  U16  lenCvtdAlpha            = 0;
  char t[MAX_DIAL_LEN-1] = {'\0'};
  char *t_p              = NULL;

  T_ACI_PB_STOR mem_stor      = PB_STOR_NotPresent;
  T_ACI_D_CLIR_OVRD clir_ovrd = D_CLIR_OVRD_Default;
  T_ACI_D_CUG_CTRL  cug_ctrl  = D_CUG_CTRL_NotPresent;
  T_ACI_D_TOC       call_type = D_TOC_Data;

#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT     pbText;
#endif

  TRACE_FUNCTION ("aci_bat_call_from_phonebook()");

  dial_string = (CHAR*)cmd->params.ptr_at_d->dial_string;
  dial_string++; /* skip the already parsed '>'; */

  if (*dial_string EQ '"')
  {
    /* call with a given name of phonebook e.g.: ATD>"Doe Joe" */
    dial_string = parse(dial_string,"z",
                        (LONG)NAME_BUF_LEN,
                        strlen(dial_string),
                        dial_string,
                        &lenAlpha,
                        alpha);
    if (dial_string EQ NULL)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
    else if (strcspn(dial_string,"iIgG;") NEQ 0)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
#ifdef NO_ASCIIZ
    utl_chsetToSim ((UBYTE*)alpha, lenAlpha, (UBYTE*)cvtdAlpha, &lenCvtdAlpha, GSM_ALPHA_Def);
#else
  TRACE_ERROR("NO_ASCIIZ is defined in aci.h, phb.c and aci_cmh.h, so NO support for NULL terminated strings");
#endif
    t_p = cvtdAlpha;
  }
  else
  {
    /* call from a specific phonebook entry e.g.: ATD>MT7 */
    if ((toupper(*dial_string)) >= 'A' AND (toupper(*dial_string)) <= 'Z')
    {
      for (i=0;phb_mem_names[i].name NEQ 0;i++)
      {
        strupper(dial_string);
        if (strncmp(dial_string,phb_mem_names[i].name,2) EQ 0)
        {
          mem_stor=phb_mem_names[i].stor;
          break;
        }
        else 
        {
          mem_stor=PB_STOR_NotPresent;
        }
      }
      if (mem_stor < 0)
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
        return (ACI_BAT_FAIL);
      }
      dial_string=dial_string+2;
    }
    pos=strcspn(dial_string,"iIgG;");            /* determine index */
    if (pos EQ 0 OR pos >= (MAX_DIAL_LEN-1))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
    strncpy(t,dial_string,pos);
    dial_string=dial_string+pos;
    index =(SHORT)atoi(t);
   
    if(!index OR index > 300)
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
    t_p = NULL;
  }

  aci_bat_check_clir_cug_type ((U8*)dial_string, &clir_ovrd, &cug_ctrl, &call_type);
  
#ifdef NO_ASCIIZ
  if ( t_p NEQ NULL )
  {
    pbText.cs = CS_Sim;
    pbText.len = (UBYTE)lenCvtdAlpha;
    memcpy(pbText.data, t_p, pbText.len);
  }
  else
  {
    pbText.cs = CS_NotPresent;
    pbText.len = 0;
  }

  /*
   *   Get a pointer to the client information for convenience.
   */
  bat_client=&src_infos_psi->bat_client[src_infos_psi->active_client];

  /*
   *   Set the flag that indicates that we are waiting for a final
   *   response to the ATD.
   */
  bat_client->atd_live=TRUE;

  ret = (T_ACI_BAT_RSLT)sAT_Dm((T_ACI_CMD_SRC)src_infos_psi->srcId, &pbText, mem_stor, index, clir_ovrd, cug_ctrl, call_type);

  /*
   *   If the command isn't executing, we can't still be waiting for a
   *   final response, so clear the flag.
   */
  if (ret NEQ ACI_BAT_EXCT)
  {
    bat_client->atd_live=FALSE;
  }

#else
  TRACE_ERROR("NO_ASCIIZ is defined in aci.h, phb.c and aci_cmh.h, so NO support for NULL terminated strings");
#endif
  return (ret);
}

LOCAL T_ACI_BAT_RSLT aci_bat_call_with_number   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                                 T_BAT_cmd_send    *cmd)
{
  T_bat_aci_client_maintain *bat_client;
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR  *dial_string   = NULL;
  S16  pos;
  char t[MAX_DIAL_LEN-1] = {'\0'};

  T_ACI_D_CLIR_OVRD clir_ovrd = D_CLIR_OVRD_Default;
  T_ACI_D_CUG_CTRL  cug_ctrl  = D_CUG_CTRL_NotPresent;
  T_ACI_D_TOC       call_type = D_TOC_Data;
  
  TRACE_FUNCTION ("aci_bat_call_with_number()");

  dial_string = (CHAR*)cmd->params.ptr_at_d->dial_string;
  
  pos = strcspn(dial_string,"iIgG;"); /* CLIR, CUG and call type are passed separately to sAT_Dn */
  if (pos >= MAX_DIAL_LEN-1)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  if (pos)
  {
    strncpy(t, dial_string, pos);
    dial_string = dial_string + pos;
    t[pos]='\0';
  }
  else
  {
   strcpy(t,dial_string);
  }

  aci_bat_clean_dial_str (t, sizeof(t));
  
  aci_bat_check_clir_cug_type ((U8*)dial_string, &clir_ovrd, &cug_ctrl, &call_type);

   /*
    *   Get a pointer to the client information for convenience.
    */
  bat_client=&src_infos_psi->bat_client[src_infos_psi->active_client];

  /*
   *   Set the flag that indicates that we are waiting for a final
   *   response to the ATD.
   */
  bat_client->atd_live=TRUE;

  ret = (T_ACI_BAT_RSLT)sAT_Dn((T_ACI_CMD_SRC)src_infos_psi->srcId, t, clir_ovrd, cug_ctrl, call_type);

  /*
   *   If the command isn't executing, we can't still be waiting for a
   *   final response, so clear the flag.
   */
  if (ret NEQ ACI_BAT_EXCT)
  {
    bat_client->atd_live=FALSE;
  }

  return (ret);
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT                   |
| STATE   : code                      ROUTINE : aci_bat_chld_mode_mapping |
+-------------------------------------------------------------------------+

  PURPOSE : Maps the CHLD mode passed by BAT application to 
            appropriate ENUM value
*/

LOCAL void aci_bat_chld_mode_mapping (T_BAT_cmd_set_percent_chld* bat_chld_param,
                                      T_ACI_CHLD_MOD    *mode)
{
  TRACE_FUNCTION("aci_bat_chld_mode_mapping()");

  switch(bat_chld_param->n)
  {
    case (BAT_P_CHLD_N_RELHLDORUDUB):
      *mode = CHLD_MOD_RelHldOrUdub;
      break;

    case (BAT_P_CHLD_N_RELACTANDACPT):
      if ( bat_chld_param->x EQ ((S16)BAT_PARAMETER_NOT_PRESENT) )
      {
        *mode = CHLD_MOD_RelActAndAcpt;
      }
      else
      {
        *mode = CHLD_MOD_RelActSpec;
      }
      break;

    case(BAT_P_CHLD_N_HLDACTANDACPT):
      if ( bat_chld_param->x EQ ((S16)BAT_PARAMETER_NOT_PRESENT) )
      {
        *mode = CHLD_MOD_HldActAndAcpt;
      }
      else
      {
        *mode = CHLD_MOD_HldActExc;
      }
      break;

    case(BAT_P_CHLD_N_ADDHLD):
      *mode = CHLD_MOD_AddHld;
      break;

    case(BAT_P_CHLD_N_ECT):
      *mode = CHLD_MOD_Ect;
      break;

    case(BAT_P_CHLD_N_CCBS):
      *mode = CHLD_MOD_Ccbs;
      break;

    case(BAT_P_CHLD_N_SWAP):
      if ( bat_chld_param->x EQ ((S16)BAT_PARAMETER_NOT_PRESENT) )
      {
        *mode = CHLD_MOD_RetrieveHoldCall;
      }
      else
      {
        *mode = CHLD_MOD_RetrieveHoldCallSpec;
      }
      break;

    case(BAT_P_CHLD_N_RELANYSPEC):
        *mode = CHLD_MOD_RelAnySpec;
      break;

    case(BAT_P_CHLD_N_ONLYHOLD):
      /* FTA requires an extra mode: Put on hold (without
       * accepting automatically waiting or held calls)
       */
      *mode = CHLD_MOD_OnlyHold;
      break;
    
    case(BAT_P_CHLD_N_RELDIALCALL):
      /* Release dialing call, without dropping current call. */
      *mode = CHLD_MOD_RelDialCall;
      break;
    
    default:
      break;
  }
}

/*================== global functions ========================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_D                |
+--------------------------------------------------------------------+

  PURPOSE : dial command for voice, data or USSD

  1.   call from phone book ?          --> ATD>"Doe Joe"
  1.a  with CUG ON/OFF and/or CLIP ?   --> ATD>"Doe Joe"G
  1.b  what character set ?
  2    call from specific phone book ? --> ATD>MT7
  2.a  with CUG ON/OFF and/or CLIP ?   --> ATD>MT7I
  3.   call with number ?              --> ATD1234567 
  3.a  with CUG ON/OFF and/or CLIP ?   --> ATD1234567g
  4.   is the call a voice call ?      --> ATDxxxxxxxx; <-- semicolon ?

  up to here in general voice or data call is distinguished.
  whether the data call is CSD, GPRS or USSD (checked by ksd_xxx() within cmhCC_Dial())
  is handled on CMH level. see cmhCC_Dial()

*/
GLOBAL T_ACI_BAT_RSLT BAT_D               (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                           T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  U8 *dial_string = NULL;
  TRACE_FUNCTION ("BAT_D()");

  if (cmd->params.ptr_at_d->c_dial_string) 
  {
    if (cmd->params.ptr_at_d->c_dial_string > MAX_DIAL_LEN-1)
    {
       ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
       return (ACI_BAT_FAIL);
    }
    dial_string = cmd->params.ptr_at_d->dial_string;
    if (*dial_string EQ '>')
    {
      ret = aci_bat_call_from_phonebook(src_infos_psi, cmd);
    }
    else
    {
      ret = aci_bat_call_with_number(src_infos_psi, cmd);
    }
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown); /* No dial string, so unknown error */
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : BAT_A                |
+--------------------------------------------------------------------+

  PURPOSE : ATA on binary level there is just a cmd.ctrl_params = BAT_CMD_AT_A
            no additional parameter needed.
*/
GLOBAL T_ACI_BAT_RSLT BAT_A               (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                           T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  TRACE_FUNCTION ("BAT_A()");
  ret = (T_ACI_BAT_RSLT)sAT_A((T_ACI_CMD_SRC)src_infos_psi->srcId);
  if (ret EQ ACI_BAT_FAIL)
  {
    if (!(ACI_BAT_VALID_ERR_DESC()))    /* has sAT_A already set an error value ? */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown); /* No, so unknown error */
    }
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_H               |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT BAT_H              (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                           T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  TRACE_FUNCTION ("BAT_H()");
  ret = (T_ACI_BAT_RSLT)sAT_H((T_ACI_CMD_SRC)src_infos_psi->srcId);
  if (ret EQ ACI_BAT_FAIL)
  {
    if (!(ACI_BAT_VALID_ERR_DESC()))    /* has sAT_H already set an error value ? */
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown); /* No, so unknown error */
    }
  }
  return(ret);
}

#ifdef FAX_AND_DATA
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCBST        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCBST        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
   
  TRACE_FUNCTION ("sBAT_PlusCBST()");
  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCBST((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_ACI_BS_SPEED)cmd->params.ptr_set_plus_cbst->speed,
                               (T_ACI_CBST_NAM)cmd->params.ptr_set_plus_cbst->name,
                               (T_ACI_CBST_CE)cmd->params.ptr_set_plus_cbst->ce);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCBST        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCBST        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cbst_speed  speed = BAT_CBST_SPD_NOT_PRESENT;
  T_BAT_plus_cbst_name  name  = BAT_CBST_NAM_NOT_PRESENT ;
  T_BAT_plus_cbst_ce   ce    = BAT_CBST_CE_NOT_PRESENT;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cbst que_cbst_buffer;  
  
  TRACE_FUNCTION ("qBAT_PlusCBST()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CBST;
  resp.response.ptr_que_plus_cbst = &que_cbst_buffer;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCBST((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	   (T_ACI_BS_SPEED*)&speed,(T_ACI_CBST_NAM*)&name,(T_ACI_CBST_CE*)&ce);
  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_cbst->ce = ce;
    resp.response.ptr_que_plus_cbst->speed = speed;
    resp.response.ptr_que_plus_cbst->name = name;
    aci_bat_send(src_infos_psi,&resp);
    return ACI_BAT_CMPL;
  }
  else
  {
    return ACI_BAT_FAIL;
  }
}
#endif /* FAX_AND_DATA */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCTTY     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCTTY     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
    
  TRACE_FUNCTION ("sBAT_PercentCTTY()");
  
  ret = (T_ACI_BAT_RSLT)sAT_PercentCTTY ((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	                    CTTY_MOD_Enable, (T_ACI_CTTY_REQ)cmd->params.ptr_set_percent_ctty->req);
  /*sending hardcoded enable mode for sAT_XXXX */
  
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCTTY     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCTTY     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_ctty_req  req ;
  UBYTE mode;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_ctty que_tty_buffer;
  T_BAT_trx trx;
  T_BAT_percent_ctty_m  state;
  
  TRACE_FUNCTION ("qBAT_PercentCTTY()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_CTTY ;
  resp.response.ptr_que_percent_ctty = &que_tty_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentCTTY ((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	(T_ACI_CTTY_MOD *)&mode,(T_ACI_CTTY_REQ *)&req,(T_ACI_CTTY_STAT *)&state,(T_ACI_CTTY_TRX *)&trx);
  resp.response.ptr_que_percent_ctty->m= state ;
  resp.response.ptr_que_percent_ctty->req = req;
  resp.response.ptr_que_percent_ctty->trx = trx;
  
  aci_bat_send(src_infos_psi,&resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSTA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSTA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     type,
                *p_type;
  
  TRACE_FUNCTION ("sBAT_PlusCSTA()");
  
  p_type=&type;
  if (cmd->params.ptr_set_plus_csta->type EQ ((S16)BAT_PARAMETER_NOT_PRESENT))
  {
    p_type = NULL;
  }
  else
  {
    type = toa_demerge(cmd->params.ptr_set_plus_csta->type);
    if (type.ton < 0 OR type.npi < 0)
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusCSTA((T_ACI_CMD_SRC)src_infos_psi->srcId,p_type); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSTA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSTA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_TOA     type;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_csta que_csta_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCSTA()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CSTA;
  resp.response.ptr_que_plus_csta = &que_csta_buffer;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCSTA((T_ACI_CMD_SRC)src_infos_psi->srcId,&type);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  { 
    resp.response.ptr_que_plus_csta->type = toa_merge (type);
    aci_bat_send(src_infos_psi, &resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCHLD     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCHLD     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CHLD_MOD    mode       = CHLD_MOD_NotPresent;
  CHAR              X_prm[2];
  CHAR*             p_X_prm;
  
  TRACE_FUNCTION ("sBAT_PercentCHLD()");

  X_prm[0] = (CHAR)NOT_PRESENT_8BIT;
  p_X_prm = X_prm;
  
  /* 
   * Map the mode from BAT application to appropriate ENUM values 
   * present in ACI
   */
  aci_bat_chld_mode_mapping (cmd->params.ptr_set_percent_chld, &mode);

  if(cmd->params.ptr_set_percent_chld->x NEQ ((S16)BAT_PARAMETER_NOT_PRESENT))
  {
    X_prm[0] = (char)(cmd->params.ptr_set_percent_chld->x + 0x30);  /* 0x30 = '0' */
    X_prm[1] = '\0';
  }
  else
  {
    p_X_prm = NULL;
  }
  ret = (T_ACI_BAT_RSLT)sAT_PercentCHLD((T_ACI_CMD_SRC)src_infos_psi->srcId, mode, p_X_prm );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCLIP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLIP        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CLIP_STAT   stat       = CLIP_STAT_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clip que_clip_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCLIP()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CLIP;
  resp.response.ptr_que_plus_clip = &que_clip_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCLIP((T_ACI_CMD_SRC)src_infos_psi->srcId,&stat);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_clip->m = (T_BAT_plus_clip_m)stat ;
    aci_bat_send(src_infos_psi, &resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCDIP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCDIP        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_CMPL;
  T_ACI_CDIP_STAT   stat       = CDIP_STAT_Unknown;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cdip que_cdip_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCDIP()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CDIP;
  resp.response.ptr_que_plus_cdip = &que_cdip_buffer;
  
  resp.response.ptr_que_plus_cdip->m = (T_BAT_plus_cdip_m)stat ;
  aci_bat_send(src_infos_psi, &resp);

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCOLP        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCOLP        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_COLP_STAT   stat       = COLP_STAT_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_colp que_colp_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCOLP()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_COLP;
  resp.response.ptr_que_plus_colp = &que_colp_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCOLP((T_ACI_CMD_SRC)src_infos_psi->srcId,&stat);
  if (ret EQ (T_ACI_BAT_RSLT)AT_CMPL)
  {
    resp.response.ptr_que_plus_colp->m = (T_BAT_plus_colp_m)stat ;
    aci_bat_send(src_infos_psi, &resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCLIR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCLIR        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
    
  TRACE_FUNCTION ("sBAT_PlusCLIR()");
  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCLIR((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	                  (T_ACI_CLIR_MOD)cmd->params.ptr_set_plus_clir->n);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCLIR        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLIR        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT  ret  = ACI_BAT_FAIL;
  T_ACI_CLIR_STAT stat = CLIR_STAT_NotPresent;
  T_ACI_CLIR_MOD  mode = CLIR_MOD_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clir clir;
  
  TRACE_FUNCTION ("qBAT_PlusCLIR()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CLIR;
  resp.response.ptr_que_plus_clir = &clir;

  ret = (T_ACI_BAT_RSLT)qAT_PlusCLIR ((T_ACI_CMD_SRC)src_infos_psi->srcId,&mode,&stat);

  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_clir->m = (T_BAT_plus_clir_m)stat;
    resp.response.ptr_que_plus_clir->n = (T_BAT_plus_clir_n)mode;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCCUG        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCUG        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_ccug_n    n = BAT_CCUG_N_NOT_PRESENT ;
  T_BAT_plus_ccug_index  index = BAT_CCUG_INDEX_NOT_PRESENT ; 
  T_BAT_plus_ccug_info  info = BAT_CCUG_INFO_NOT_PRESENT ;
  
  TRACE_FUNCTION ("sBAT_PlusCCUG()");
  
  n = cmd->params.ptr_set_plus_ccug->n;
  index = cmd->params.ptr_set_plus_ccug->index;
  info = cmd->params.ptr_set_plus_ccug->info;
  
  ret = (T_ACI_BAT_RSLT)sAT_PlusCCUG((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	     (T_ACI_CCUG_MOD)n,(T_ACI_CCUG_IDX)index,(T_ACI_CCUG_INFO)info);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCCUG        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCCUG        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CCUG_MOD  mode = CCUG_MOD_NotPresent;
  T_ACI_CCUG_IDX  idx  = CCUG_IDX_NotPresent;
  T_ACI_CCUG_INFO info = CCUG_INFO_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_ccug que_ccug_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCCUG()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CCUG;
  resp.response.ptr_que_plus_ccug = &que_ccug_buffer;
    
  ret = (T_ACI_BAT_RSLT)qAT_PlusCCUG ((T_ACI_CMD_SRC)src_infos_psi->srcId,&mode,&idx,&info);
  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_ccug->n = (T_BAT_plus_ccug_n)mode;
    resp.response.ptr_que_plus_ccug->index = (T_BAT_plus_ccug_index)idx;
    resp.response.ptr_que_plus_ccug->info = (T_BAT_plus_ccug_info)info;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCMOD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCMOD        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_cmod_mode mode = BAT_CMOD_MODE_NOT_PRESENT;
  
  TRACE_FUNCTION ("sBAT_PlusCMOD()");
  
  mode = cmd->params.ptr_set_plus_cmod->mode;
  ret = (T_ACI_BAT_RSLT)sAT_PlusCMOD ((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	                (T_ACI_CMOD_MOD)mode);
  
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCMOD        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCMOD        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CMOD_MOD mode = CMOD_MOD_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cmod que_cmod_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCMOD()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CMOD;
  resp.response.ptr_que_plus_cmod = &que_cmod_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCMOD ((T_ACI_CMD_SRC)src_infos_psi->srcId,&mode);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_cmod->mode = (T_BAT_plus_cmod_mode)mode;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCR          |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCR          (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PlusCR()");
  
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCR          |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCR          (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_res_uns_plus_cr cr;

  TRACE_FUNCTION ("qBAT_PlusCR()");

  memset(&cr,0x00,sizeof(T_BAT_res_uns_plus_cr));
    
  /* Check This Out */
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCAOC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCAOC        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  LONG          ccm  = 0L;
  SHORT         mode = 0;
  T_BAT_cmd_response      resp;
  T_BAT_res_set_plus_caoc caoc;
  
  TRACE_FUNCTION ("sBAT_PlusCAOC()");

  resp.ctrl_response = BAT_RES_SET_PLUS_CAOC;
  resp.response.ptr_set_plus_caoc = &caoc;
 
  mode = cmd->params.ptr_set_plus_caoc->mode;
  if(mode EQ BAT_CAOC_MODE_QUERY)
  {
    ret = (T_ACI_BAT_RSLT)qAT_PlusCAOC((T_ACI_CMD_SRC)src_infos_psi->srcId,&ccm);
  }
  else
  {
    return(ACI_BAT_FAIL);
  }
  
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_set_plus_caoc->ccm = ccm;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
 }

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCACM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCACM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char pin2 [MAX_PWD_LENGTH]   = {0} ;
  
  TRACE_FUNCTION ("sBAT_PlusCACM()");
  
  memcpy(pin2,cmd->params.ptr_set_plus_cacm->passwd,cmd->params.ptr_set_plus_cacm->c_passwd);
  ret = (T_ACI_BAT_RSLT)sAT_PlusCACM((T_ACI_CMD_SRC)src_infos_psi->srcId,pin2);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCACM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCACM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  LONG         acm = 0L;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cacm que_cacm_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCACM()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CACM;
  resp.response.ptr_que_plus_cacm = &que_cacm_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCACM((T_ACI_CMD_SRC)src_infos_psi->srcId,&acm);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_cacm->acm = acm;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCAMM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCAMM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char pin2       [MAX_PWD_LENGTH] = {0};
  char* p_pin2;
  
  TRACE_FUNCTION ("sBAT_PlusCAMM()");
  
  p_pin2 = pin2;  
  if(cmd->params.ptr_set_plus_camm->v_passwd)
  {
    memcpy(pin2,cmd->params.ptr_set_plus_camm->passwd,cmd->params.ptr_set_plus_camm->c_passwd);
  }
/*  else
  {
    p_pin2 = NULL;
  } */
  ret = (T_ACI_BAT_RSLT)sAT_PlusCAMM((T_ACI_CMD_SRC)src_infos_psi->srcId,
                cmd->params.ptr_set_plus_camm->acmmax,p_pin2);
  switch ((T_ACI_RETURN)ret)
  {
    case AT_BUSY:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_SimBusy);
      break;
    } 
    default:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
      break;
    } 
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCAMM        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCAMM        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  LONG acmmax      = 0L;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_camm que_camm_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCAMM()");
  
  resp.ctrl_response = BAT_RES_QUE_PLUS_CAMM;
  resp.response.ptr_que_plus_camm = &que_camm_buffer;
  ret = (T_ACI_BAT_RSLT)qAT_PlusCAMM((T_ACI_CMD_SRC)src_infos_psi->srcId,&acmmax);
  if (ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_camm->acmmax = acmmax;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCPUC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCPUC        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char         currency     [MAX_CUR_LEN]       = {0x00};
  char         cvtdCurrency [2*MAX_CUR_LEN]     = {0x00};
  USHORT       lenCvtdCurrency                  = 0;
  char         ppu          [MAX_PPU_LENGTH]    = {0x00};
  char         pin2         [MAX_PWD_LENGTH]    = {0x00};
  char*        p_pin2;
  
  TRACE_FUNCTION ("sBAT_PlusCPUC()");

  p_pin2 = pin2;
  
  memcpy(currency,cmd->params.ptr_set_plus_cpuc->currency,cmd->params.ptr_set_plus_cpuc->c_currency);
  memcpy(ppu,cmd->params.ptr_set_plus_cpuc->ppu,cmd->params.ptr_set_plus_cpuc->c_ppu);
  if(cmd->params.ptr_set_plus_cpuc->v_passwd)
  {
    memcpy(pin2,cmd->params.ptr_set_plus_cpuc->passwd,cmd->params.ptr_set_plus_cpuc->c_passwd);
  }
  else
  {
    p_pin2 = NULL;
  }
  utl_chsetToGsm ((UBYTE*)currency,
                   cmd->params.ptr_set_plus_cpuc->c_currency,
                   (UBYTE*)cvtdCurrency,
                   &lenCvtdCurrency,
#ifdef REL99
                   sizeof(cvtdCurrency),
#endif /* REL99 */
                   GSM_ALPHA_Int);
  ret = (T_ACI_BAT_RSLT)sAT_PlusCPUC((T_ACI_CMD_SRC)src_infos_psi->srcId,cvtdCurrency,ppu,p_pin2);
  switch ((T_ACI_RETURN)ret)
  {
    case AT_BUSY:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_SimBusy);
      break;
    } 
    default:
    {
      ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
      break;
    } 
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCPUC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCPUC        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char         currency     [MAX_CUR_LEN]       = {0x00};
  char         cvtdCurrency [2*MAX_CUR_LEN]     = {0x00};
  USHORT       lenCvtdCurrency                  = 0;
  char         ppu          [MAX_PPU_LENGTH]    = {0x00};
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cpuc que_cpuc_buffer;
  
 
  TRACE_FUNCTION ("qBAT_PlusCPUC()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CPUC;
  resp.response.ptr_que_plus_cpuc = &que_cpuc_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCPUC((T_ACI_CMD_SRC)src_infos_psi->srcId,currency, ppu);
  if( ret NEQ ((T_ACI_BAT_RSLT)AT_CMPL) )
  {
    return (ACI_BAT_FAIL);
  }
  utl_chsetFromGsm((UBYTE*)currency,
                   (USHORT)strlen(currency),
                   (UBYTE*)cvtdCurrency,
                   sizeof(cvtdCurrency),
                   &lenCvtdCurrency,
                   GSM_ALPHA_Def);
  
  resp.response.ptr_que_plus_cpuc->c_currency = (U8)lenCvtdCurrency;
  resp.response.ptr_que_plus_cpuc->c_ppu = strlen(ppu);
  memcpy(resp.response.ptr_que_plus_cpuc->currency,cvtdCurrency,lenCvtdCurrency);
  memcpy(resp.response.ptr_que_plus_cpuc->ppu,ppu,strlen(ppu));
  aci_bat_send(src_infos_psi,&resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCLCC        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCLCC        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CLCC_CALDESC *calLst       = NULL;
  UBYTE               lstIdx       = 0;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_clcc que_clcc_buffer;
  
 
  TRACE_FUNCTION ("qBAT_PlusCLCC()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CLCC;
  resp.response.ptr_que_plus_clcc = &que_clcc_buffer;
  
  MALLOC (calLst, MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC));
  ret = (T_ACI_BAT_RSLT)qAT_PlusCLCC((T_ACI_CMD_SRC)src_infos_psi->srcId, calLst);
                     
  if( ret NEQ ((T_ACI_BAT_RSLT)AT_CMPL) )
  {
    return (ACI_BAT_FAIL);
  }

  /* BAT has only one instance of this, so send one call info at a time */
  for( lstIdx = 0; lstIdx < MAX_CALL_NR; lstIdx++ )
  {
    if( calLst[lstIdx].idx EQ ACI_NumParmNotPresent )
      break;
  
    /* Reset before sending data for each call id */
    memset(resp.response.ptr_que_plus_clcc,0x00,sizeof(T_BAT_res_que_plus_clcc));
  
    resp.response.ptr_que_plus_clcc->idx = (S16)calLst[lstIdx].idx;
    resp.response.ptr_que_plus_clcc->dir = (T_BAT_plus_clcc_dir)calLst[lstIdx].dir;
    resp.response.ptr_que_plus_clcc->stat = (T_BAT_plus_clcc_stat)calLst[lstIdx].stat;
    resp.response.ptr_que_plus_clcc->mode = (T_BAT_plus_clcc_mode)calLst[lstIdx].mode;
    resp.response.ptr_que_plus_clcc->mpty = (T_BAT_plus_clcc_mpty)calLst[lstIdx].mpty;

    if (calLst[lstIdx].number[0] NEQ 0x0)
    {
      resp.response.ptr_que_plus_clcc->v_number = TRUE;
      resp.response.ptr_que_plus_clcc->c_number = strlen(calLst[lstIdx].number);

      if (resp.response.ptr_que_plus_clcc->c_number > BAT_MAX_CLCC_NUMBER_LEN)
          resp.response.ptr_que_plus_clcc->c_number = (BAT_MAX_CLCC_NUMBER_LEN - 1);
      
      memcpy(resp.response.ptr_que_plus_clcc->number,
             calLst[lstIdx].number,
             resp.response.ptr_que_plus_clcc->c_number + 1);   /* Include Null terminator */
      
      if (calLst[lstIdx].type.ton NEQ TON_NotPresent)
      {
        S16 type = toa_merge(calLst[lstIdx].type);
        resp.response.ptr_que_plus_clcc->type = type;
      }
#ifdef NO_ASCIIZ
      if (calLst[lstIdx].alpha.len NEQ 0x0)
      {
        resp.response.ptr_que_plus_clcc->v_alpha = TRUE;
        resp.response.ptr_que_plus_clcc->c_alpha = (U8)calLst[lstIdx].alpha.len;

        /* Limit alpha length to BAT size */
        if (resp.response.ptr_que_plus_clcc->c_alpha > BAT_MAX_PHB_NUM_LEN)
            resp.response.ptr_que_plus_clcc->c_alpha = BAT_MAX_PHB_NUM_LEN;
        
        memcpy(resp.response.ptr_que_plus_clcc->alpha,
               calLst[lstIdx].alpha.data,
               resp.response.ptr_que_plus_clcc->c_alpha);
      }
#else  /* #ifdef NO_ASCIIZ */
      if (calLst[lstIdx].alpha[0] NEQ 0x0)
      {
        resp.response.ptr_que_plus_clcc->v_alpha = TRUE;

        /* Limit alpha length to BAT size */
        resp.response.ptr_que_plus_clcc->c_alpha = strlen(calLst[lstIdx].alpha);
        if (resp.response.ptr_que_plus_clcc->c_alpha > MAX_ALPHA_LEN)   /* Has to be BAT_MAX..*/
            resp.response.ptr_que_plus_clcc->c_alpha = (MAX_ALPHA_LEN - 1);
        
        memcpy(resp.response.ptr_que_plus_clcc->alpha,
               calLst[lstIdx].alpha,
               (resp.response.ptr_que_plus_clcc->c_alpha)+1);  /* Include the NULL terminator */
      }
#endif /* #ifdef NO_ASCIIZ */
    }

    aci_bat_send(src_infos_psi,&resp);
  }
  
  MFREE (calLst);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCCWA        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCCWA        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_ccwa_mode mode       = BAT_CCWA_MODE_NOT_PRESENT ;
  T_ACI_CLASS    class_type = CLASS_NotPresent;
    
  TRACE_FUNCTION ("sBAT_PlusCCWA()");
  
  mode = cmd->params.ptr_set_plus_ccwa->mode;
  class_type = (T_ACI_CLASS)cmd->params.ptr_set_plus_ccwa->bearer_class;
  if (mode EQ BAT_CCWA_MODE_QUERY)
  {
    ret = (T_ACI_BAT_RSLT)qAT_PlusCCWA ((T_ACI_CMD_SRC)src_infos_psi->srcId,class_type);
  }
  else
  {
    ret = (T_ACI_BAT_RSLT)sAT_PlusCCWA ((T_ACI_CMD_SRC)src_infos_psi->srcId,
		(T_ACI_CCWA_MOD)mode,class_type);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusVTS         |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusVTS         (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  CHAR              dtmf       = '\0';
  
  TRACE_FUNCTION ("sBAT_PlusVTS()");
  
  dtmf = cmd->params.ptr_set_plus_vts->dtmf;
  ret = (T_ACI_BAT_RSLT)sAT_PlusVTS((T_ACI_CMD_SRC)src_infos_psi->srcId,dtmf,VTS_MOD_Auto);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentRDL      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentRDL      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_rdl_mode rdlmode = BAT_P_RDL_MODE_NOT_PRESENT ;
  
  TRACE_FUNCTION ("sBAT_PercentRDL()");
  
  rdlmode = cmd->params.ptr_set_percent_rdl->mode;
  ret = (T_ACI_BAT_RSLT)sAT_PercentRDL ((T_ACI_CMD_SRC)src_infos_psi->srcId, 
  	                       (T_ACI_CC_REDIAL_MODE)rdlmode, NOTIF_USER);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentRDL      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentRDL      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CC_REDIAL_MODE rdlmode = AUTOM_REP_NOT_PRESENT;
  T_ACI_CC_REDIAL_NOTIF usr_notif = NOTIF_NO_PRESENT;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_rdl que_rdl_buffer;
  
  TRACE_FUNCTION ("qBAT_PercentRDL()");
  
  resp.ctrl_response = BAT_RES_QUE_PERCENT_RDL;
  resp.response.ptr_que_percent_rdl = &que_rdl_buffer;
  ret = (T_ACI_BAT_RSLT)qAT_PercentRDL((T_ACI_CMD_SRC)src_infos_psi->srcId,&rdlmode, &usr_notif);
  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_percent_rdl->mode = (T_BAT_percent_rdl_mode)rdlmode;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentRDLB     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentRDLB     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_rdlb_mode rdlmode = BAT_P_RDLB_MODE_NOT_PRESENT ;
  
  TRACE_FUNCTION ("sBAT_PercentRDLB()");
  
  rdlmode = cmd->params.ptr_set_percent_rdlb->mode;
  ret = (T_ACI_BAT_RSLT)sAT_PercentRDLB((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	         (T_ACI_CC_REDIAL_BLMODE)rdlmode,NOTIF_USER);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentRDLB     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentRDLB     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CC_REDIAL_BLACKL blackl;
  T_ACI_CC_REDIAL_NOTIF usr_notif= NOTIF_NO_PRESENT;
  UBYTE         i,j;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_rdlb que_rdlb_buffer;
  
  TRACE_FUNCTION ("qBAT_PercentRDLB()");
  
  memset(&blackl, 0, sizeof(T_ACI_CC_REDIAL_BLACKL));
  resp.ctrl_response = BAT_RES_QUE_PERCENT_RDLB;
  resp.response.ptr_que_percent_rdlb = &que_rdlb_buffer; 
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentRDLB ((T_ACI_CMD_SRC)src_infos_psi->srcId,&blackl,&usr_notif);
  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    for(i=0; i<blackl.blCount; i++) /* output black list */
    {
      memset(resp.response.ptr_que_percent_rdlb->number,0,BAT_MAX_RDLB_NUMBER_LEN);
      resp.response.ptr_que_percent_rdlb->type = (UBYTE)toa_merge(blackl.blNum[i].type);
      resp.response.ptr_que_percent_rdlb->c_number = blackl.blNum[i].numb_len;
      for(j=0; j<blackl.blNum[i].numb_len; j++)
      {
        blackl.blNum[i].number[j] = (blackl.blNum[i].number[j] | 0x30);
        /* dont know why we do this ! */
      }
      memcpy(resp.response.ptr_que_percent_rdlb->number,blackl.blNum[i].number,blackl.blNum[i].numb_len);
      aci_bat_send(src_infos_psi,&resp);
    }
  }
  return(ret);
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentVTS      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentVTS      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_dtmf  dtmf;
  T_BAT_percent_vts_mode  mode;
  
  TRACE_FUNCTION ("sBAT_PercentVTS()");
  
  dtmf = cmd->params.ptr_set_percent_vts->dtmf;
  mode = cmd->params.ptr_set_percent_vts->mode;
  if (mode EQ ((T_BAT_percent_vts_mode)VTS_MOD_NotPresent))
  {
    mode = (T_BAT_percent_vts_mode)VTS_MOD_Auto;
  }
  ret = (T_ACI_BAT_RSLT)sAT_PlusVTS((T_ACI_CMD_SRC)src_infos_psi->srcId,
  	               (char)dtmf,(T_ACI_VTS_MOD)mode); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCSNS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCSNS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_plus_csns_mode      mode ;
 
  TRACE_FUNCTION ("sBAT_PlusCSNS()");
  
  mode = cmd->params.ptr_set_plus_csns->mode;
  ret = (T_ACI_BAT_RSLT)sAT_PlusCSNS ((T_ACI_CMD_SRC)src_infos_psi->srcId, (T_ACI_CSNS_MOD)mode);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCSNS        |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCSNS        (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_CSNS_MOD mode = CSNS_MOD_NotPresent;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_csns que_csns_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCSNS()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CSNS;
  resp.response.ptr_que_plus_csns = &que_csns_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCSNS ((T_ACI_CMD_SRC)src_infos_psi->srcId,&mode);
  if(ret EQ ((T_ACI_BAT_RSLT)AT_CMPL))
  {
    resp.response.ptr_que_plus_csns->mode = (T_BAT_plus_csns_mode)mode;
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

#ifdef TI_PS_FF_AT_CMD_P_ECC
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentECC      |
+--------------------------------------------------------------------+

  PURPOSE : setting of additional ECC numbers by external applications
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentECC(T_ACI_DTI_PRC_PSI *src_infos_psi,
                                      T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;

  TRACE_FUNCTION ("sBAT_PercentECC()");

  ret = (T_ACI_BAT_RSLT)sAT_PercentECC((T_ACI_CMD_SRC)src_infos_psi->srcId,
                       cmd->params.ptr_set_percent_ecc->index,
                       (char *)cmd->params.ptr_set_percent_ecc->number);
  return (ret);
}
#endif /* TI_PS_FF_AT_CMD_P_ECC */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCHUP         |
+--------------------------------------------------------------------+

  PURPOSE : +CHUP command (Hangs up call)
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCHUP       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                           T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  TRACE_FUNCTION ("sBAT_PlusCHUP()");
  ret = (T_ACI_BAT_RSLT)sAT_PlusCHUP((T_ACI_CMD_SRC)src_infos_psi->srcId);
  return(ret);
}




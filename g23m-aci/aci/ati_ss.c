/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command Interpreter: Supplementary Service related commands.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_SS_C
#define ATI_SS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "aci_lst.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include "aci_mem.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

typedef struct
{
  char *name;
  T_ACI_FAC fac;
} net_fac;

const net_fac fac[] =
{
  {"SC",  FAC_Sc},
  {"AO",  FAC_Ao},
  {"OI",  FAC_Oi},
  {"OX", FAC_Ox},
  {"AI",  FAC_Ai},
  {"IR",  FAC_Ir},
  {"AB", FAC_Ab},
  {"AG",  FAC_Ag},
  {"AC",  FAC_Ac},
  {"FD",  FAC_Fd},
  {"PN",  FAC_Pn},  /* Network personalisation of the ME */
  {"PU",  FAC_Pu},  /* Network subset personalisation of the ME */
  {"PP",  FAC_Pp},  /* Service provider personalisation of the ME */
  {"PC",  FAC_Pc},  /* Corporate personalisation of the ME */
  {"PS",  FAC_Ps},     /* SIM personalisation */
  {"PF",  FAC_Pf},      /* Personalisation on first inserted SIM */
  {"AL",  FAC_Al}, /* ALS settings locked by CHV2 */
  {"P2",  FAC_P2},
  #ifdef SIM_PERS
  {"BP",  FAC_Bl}, /* Blocked Network personalisation of the ME */
  {"FC",  FAC_Fc},
  {"FM",  FAC_Fcm},
  {"MU",  FAC_Mu},
  {"MM",  FAC_Mum},
  #endif
  #ifdef FF_PHONE_LOCK
  {"PL",  FAC_Pl},
  {"LA",  FAC_Apl},
  #endif
  {0,     FAC_NotPresent}

};

#ifdef SIM_PERS

typedef struct
{
  char *name;
  T_SUP_INFO_TYPE sup_info;
} net_mepd_sup_info;

const net_mepd_sup_info mepd_sup_info[] =
{
  {"MAX",  FCMAX},
  {"ALE",   FCATTEMPTSLEFT},
  {"RFM",   FCRESETFAILMAX},
  {"RFA",   FCRESETFAILATTEMPTSLEFT},
  {"RSM",  FCRESETSUCCESSMAX},
  {"RSA",  FCRESETSUCCESSATTEMPTSLEFT},
  {"TMF",  TIMERFLAG},
  {"ETF",   ETSIFLAG},
  {"AIF",   AIRTELINDFLAG},
  {0,         CMEPD_SUP_INFO_NotPresent}
};

#endif
GLOBAL char           dialBuf[MAX_DIAL_LEN];
GLOBAL char           subBuf[MAX_SUBADDR_LEN];

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCTFR      |
+--------------------------------------------------------------------+

  PURPOSE : +CTFR command (Call TRansfer, Call Deflection)
*/
GLOBAL T_ATI_RSLT setatPlusCTFR (char * cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_TOA     type;
  T_ACI_TOA    *p_type;
  T_ACI_TOS     satype;
  T_ACI_TOS    *p_satype;
  CHAR         *subadr = subBuf;
  SHORT         toa_oct = 0;
  SHORT         tos_oct = 0;
  CHAR          numBuf[MAX_B_SUBSCR_NUM_LEN];

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CTFR;

  /* init */
  p_type   = &type;
  p_satype = &satype;
  memset( numBuf, 0, sizeof(numBuf));
  memset( subadr, 0, MAX_SUBADDR_LEN);

  cl = parse(cl,"srsr",(LONG)MAX_B_SUBSCR_NUM_LEN, numBuf, &toa_oct,
                     (LONG)MAX_SUBADDR_LEN, subadr, &tos_oct);

  /* Process number parameter (mandatory) */
  if(!cl OR numBuf[0] EQ '\0')
  {
    /* Number is mandatory. No number parameter => error */
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  /* Process Type Of Address (optional) */
  if(toa_oct EQ 0)
  {
    /* Type Of Address not present */
    p_type = NULL;
  }
  else
  {
    /* Type Of Address present */
    type = toa_demerge (toa_oct);
    if (type.ton < 0 OR type.npi < 0)
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  }

  /* Process subadr (optional) */
  if( subadr[0] EQ 0 )
  {
    /* subadr not present */
    subadr = NULL;
  }

  /* Process Type Of Subaddress (optional) */
  if(tos_oct EQ 0)
  {
    /* Type Of Subaddress not present */
    p_satype = NULL;
  }
  else
  {
    satype = tos_demerge (tos_oct);
    if (satype.tos < 0 OR satype.oe < 0)
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  }    

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ctfr my_bat_set_plus_ctfr;

  TRACE_FUNCTION("setatPlusCTFR() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ctfr, 0, sizeof(my_bat_set_plus_ctfr));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CTFR;
  cmd.params.ptr_set_plus_ctfr = &my_bat_set_plus_ctfr;

  my_bat_set_plus_ctfr.c_number = strlen(numBuf);
  memcpy(my_bat_set_plus_ctfr.number, numBuf, my_bat_set_plus_ctfr.c_number);
  my_bat_set_plus_ctfr.type = (S16)p_type;
  my_bat_set_plus_ctfr.v_subaddr = 0; /*??? */
  my_bat_set_plus_ctfr.c_subaddr = strlen(subadr);
  memcpy(my_bat_set_plus_ctfr.subaddr, subadr, my_bat_set_plus_ctfr.c_subaddr);
  my_bat_set_plus_ctfr.satype = (S16)p_satype;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusCTFR()");

  ret = sAT_PlusCTFR ((T_ACI_CMD_SRC)srcId, numBuf, p_type, subadr, p_satype);

  if (ret NEQ AT_EXCT)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atiSShandleCSSN         |
+--------------------------------------------------------------------+

  PURPOSE : Handles the +CSSN and %CSSN set commands.*/

GLOBAL T_ATI_RSLT atiSShandleCSSN (char* cl, UBYTE srcId)
{
int cssiMode = ati_user_output_cfg[srcId].CSSI_stat, cssuMode = ati_user_output_cfg[srcId].CSSU_stat;

  TRACE_FUNCTION("atiSShandleCSSN()");

  if (*cl EQ 0 OR *cl EQ ';')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  cl = parse(cl,"dd",&cssiMode,&cssuMode);
  /* A value of 2 for cssiMode is now accepted. This value will only be set by %CSSN */
  if(!cl OR cssiMode > 2 OR cssiMode < 0 OR 
            cssuMode > 1 OR cssuMode < 0    )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  ati_user_output_cfg[srcId].CSSI_stat=(UBYTE)cssiMode;
  ati_user_output_cfg[srcId].CSSU_stat=(UBYTE)cssuMode;

  if (*cl NEQ '\0' AND *cl NEQ ';')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  return (ATI_CMPL);

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSSN         |
+--------------------------------------------------------------------+

  PURPOSE : +CSSN command (supplementary service notifications)
*/
GLOBAL T_ATI_RSLT setatPlusCSSN (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPLusCSSN()");

  /*Check that the range of the first parameter "CSSI mode" is valid (0 or 1).Further parameters are
  checked in the function atiSShandleCSSN()*/
  if ((*cl EQ '0') OR (*cl EQ '1') OR (*cl EQ ','))
      return atiSShandleCSSN(cl,srcId);
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCSSN         |
+--------------------------------------------------------------------+

  PURPOSE : %CSSN command (supplementary service notifications extension)
*/
GLOBAL T_ATI_RSLT setatPercentCSSN (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPercentCSSN()");

  return atiSShandleCSSN(cl,srcId);
}



GLOBAL T_ATI_RSLT queatPlusCSSN (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPLusCSSN()");

  resp_disp(srcId, cl,"bb",&ati_user_output_cfg[srcId].CSSI_stat,&ati_user_output_cfg[srcId].CSSU_stat);
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCSSN         |
+--------------------------------------------------------------------+

  PURPOSE : Handles the %CSSN query command.*/


GLOBAL T_ATI_RSLT queatPercentCSSN (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPercentCSSN()");

  resp_disp(srcId, cl,"bb",&ati_user_output_cfg[srcId].CSSI_stat,&ati_user_output_cfg[srcId].CSSU_stat);
  return (ATI_CMPL);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCCFC         |
+--------------------------------------------------------------------+

  PURPOSE : +CCFC call forwarding
*/

GLOBAL T_ATI_RSLT setatPlusCCFC(char *cl, UBYTE srcId)
{
  T_ACI_RETURN        ret = AT_FAIL;
  T_ACI_CCFC_RSN      reason=CCFC_RSN_NotPresent;
  T_ACI_CCFC_MOD      mode=CCFC_MOD_NotPresent;
  T_ACI_TOA           type;
  T_ACI_TOA          *p_type;
  T_ACI_TOS           satype;
  T_ACI_TOS          *p_satype;
  T_ACI_CLASS         class_type=CLASS_NotPresent;
  CHAR               *subadr = subBuf;
  SHORT               time=-1;
  SHORT               toa_oct=0;
  SHORT               tos_oct=0;
  CHAR                numBuf[MAX_B_SUBSCR_NUM_LEN];
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  
  /* init */
  memset( numBuf, 0, sizeof(numBuf));
  memset( subadr, 0, MAX_SUBADDR_LEN);

  TRACE_FUNCTION("setatPLusCCFC()");

  p_type=&type;
  p_satype=&satype;

  cl = parse(cl,"ddsrdsrr",&reason,&mode,
           (LONG)MAX_B_SUBSCR_NUM_LEN,numBuf,&toa_oct,&class_type,
           (LONG)MAX_SUBADDR_LEN,subadr,&tos_oct,&time);

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  if (mode EQ 2)                                                             /*query mode*/
  {
#ifdef  FF_ATI_BAT
    {
      T_BAT_cmd_send cmd;
      T_BAT_cmd_set_plus_ccfc my_bat_set_plus_ccfc = {0};

      TRACE_FUNCTION("setatPlusCCFC() {Query mode=2} calls bat_send() <=== as APPLICATION");

      cmd.ctrl_params = BAT_CMD_SET_PLUS_CCFC;
      cmd.params.ptr_set_plus_ccfc = &my_bat_set_plus_ccfc;

      my_bat_set_plus_ccfc.reason = (T_BAT_plus_ccfc_reason)reason;
      my_bat_set_plus_ccfc.mode = (T_BAT_plus_ccfc_mode)mode;
      my_bat_set_plus_ccfc.bearer_class = (T_BAT_plus_ccfc_bearer_class)class_type;
      src_params->curAtCmd = AT_CMD_CCFC;

      bat_send(ati_bat_get_client(srcId), &cmd);
      return ATI_EXCT; /* executing, because response is passed by callback function */
    }
#else /* no FF_ATI_BAT */
    ret = qAT_PlusCCFC((T_ACI_CMD_SRC)srcId,reason,class_type);
    switch (ret)
    {
      case AT_EXCT:
      {
        src_params->curAtCmd    = AT_CMD_CCFC;
        return (ATI_EXCT);
      }
      case AT_BUSY:
      {
        TRACE_EVENT("setatPLusCCFC(): qAT_PlusCCFC returns BUSY");
        return (ATI_BUSY);
      }
      case AT_FAIL:
      {
        TRACE_EVENT("setatPLusCCFC(): qAT_PlusCCFC returns FAIL");
        cmdCmeError(CME_ERR_Unknown); /* Extended error returned by qAT_PlusCCFC */
        return (ATI_FAIL);
      }
      default:
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
    }
#endif /* no FF_ATI_BAT */
  }
  else                                                                    /*set mode*/
  {
    if( subadr[0] EQ 0 )
    {
      /* subadr has been omitted in command */
      subadr = NULL;
    }

    if(toa_oct EQ 0)
      p_type=NULL;
    else
    {
      type=toa_demerge(toa_oct);
      if (type.ton < 0 OR type.npi < 0)
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
    if(tos_oct EQ 0)
      p_satype=NULL;
    else
    {
      satype=tos_demerge(tos_oct);
      if(satype.tos < 0 OR satype.oe < 0)
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
#ifdef  FF_ATI_BAT
    {
       T_BAT_cmd_send cmd;
       T_BAT_cmd_set_plus_ccfc my_bat_set_plus_ccfc = {0};

      TRACE_FUNCTION("setatPlusCCFC() calls bat_send() <=== as APPLICATION");

      cmd.ctrl_params = BAT_CMD_SET_PLUS_CCFC;
      cmd.params.ptr_set_plus_ccfc = &my_bat_set_plus_ccfc;

      my_bat_set_plus_ccfc.reason = (T_BAT_plus_ccfc_reason)reason;
      my_bat_set_plus_ccfc.mode = (T_BAT_plus_ccfc_mode)mode;

      /* check for number string and store for BAT send */
      if (my_bat_set_plus_ccfc.c_number = strlen(numBuf))
      {
        my_bat_set_plus_ccfc.v_number = TRUE;
        memcpy(my_bat_set_plus_ccfc.number, numBuf, my_bat_set_plus_ccfc.c_number);
      }

      my_bat_set_plus_ccfc.type = (S16)toa_oct;

      /* check for subaddr string and store for BAT send */
      if (subadr NEQ NULL AND
          (my_bat_set_plus_ccfc.c_subaddr = strlen(subadr)))
      {
        my_bat_set_plus_ccfc.v_subaddr = TRUE;
        memcpy(my_bat_set_plus_ccfc.subaddr, subadr, my_bat_set_plus_ccfc.c_subaddr);
      }

      my_bat_set_plus_ccfc.satype = (S16)tos_oct;
      my_bat_set_plus_ccfc.bearer_class = (T_BAT_plus_ccfc_bearer_class)class_type;
      my_bat_set_plus_ccfc.time = (T_BAT_plus_ccfc_time)time;

      src_params->curAtCmd = AT_CMD_CCFC;

      bat_send(ati_bat_get_client(srcId), &cmd);
      return ATI_EXCT; /* executing, because response is passed by callback function */
    }
#else /* no FF_ATI_BAT */
    if( numBuf[0] EQ '\0' )
    {
      /* number has been omitted in command */
      ret = sAT_PlusCCFC((T_ACI_CMD_SRC)srcId,reason,mode,NULL,p_type,class_type,subadr,p_satype,time);
    }
    else
    {
      ret = sAT_PlusCCFC((T_ACI_CMD_SRC)srcId,reason,mode,numBuf,p_type,class_type,subadr,p_satype,time);
    }
    if (ret EQ AT_EXCT)
    {
      src_params->curAtCmd    = AT_CMD_CCFC;
    }
    else
    {
      cmdCmeError(CME_ERR_Unknown);
    }
#endif /* no FF_ATI_BAT */
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLCK         |
+--------------------------------------------------------------------+

  PURPOSE : +CLCK command (Select facility lock settings)
*/

GLOBAL T_ATI_RSLT setatPlusCLCK(char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret = AT_FAIL;
  T_ACI_FAC fac_num = FAC_NotPresent;
  T_ACI_CLCK_MOD mod = CLCK_MOD_NotPresent;
  T_ACI_CLASS    class_type = CLASS_NotPresent;
  char           passwd[MAX_PWD_LENGTH] = {0};
  char           fac_str[3] = {0,0,0};
  USHORT         i;
  
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CLCK;
  TRACE_FUNCTION("setatPLusCLCK() ");

  cl = parse(cl, "sdnd", (LONG)3, fac_str, &mod, (LONG)MAX_PWD_LENGTH, passwd, &class_type);
  
  if (!cl OR *fac_str EQ 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  strcpy(fac_str, strupper(fac_str));
  for (i=0;fac[i].name NEQ 0; i++)
  {
    if (strcmp(fac[i].name, fac_str) EQ 0 )
    {
      fac_num = fac[i].fac;           /*translates facility string in facility number */
      break;
    }
  }
  if (fac_num EQ FAC_NotPresent) /* facility non existent*/
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_clck my_bat_set_plus_clck;

    TRACE_FUNCTION("setatPLusCLCK() calls bat_send() <=== as APPLICATION");

    memset(&my_bat_set_plus_clck, 0, sizeof(my_bat_set_plus_clck));
    cmd.ctrl_params = BAT_CMD_SET_PLUS_CLCK;
    cmd.params.ptr_set_plus_clck = &my_bat_set_plus_clck;

    my_bat_set_plus_clck.fac = fac_num;                     
    my_bat_set_plus_clck.mode = mod;
    if(mod NEQ CLCK_MODE_QUERY) /*query mode*/
    {
      my_bat_set_plus_clck.v_passwd = 1;                
      my_bat_set_plus_clck.c_passwd = strlen(passwd);                
      memcpy(my_bat_set_plus_clck.passwd, passwd, my_bat_set_plus_clck.c_passwd);
    }

    my_bat_set_plus_clck.bearer_class = class_type;             

    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  {
    T_ACI_CLSSTAT  clsStat;
    SHORT          pos;

    TRACE_FUNCTION("setatPLusCLCK()");
  
    if (mod EQ CLCK_MOD_NotPresent) /* mode non existent*/
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return(ATI_FAIL);
    }
    if (mod EQ CLCK_MODE_QUERY) /*query mode*/
    {
      ret = qAT_PlusCLCK((T_ACI_CMD_SRC)srcId, fac_num, class_type, &clsStat);

      if(ret EQ AT_CMPL)
      {
        pos = sprintf(g_sa,"+CLCK: ");

        if (clsStat.status NEQ STATUS_NotPresent) 
        {
          pos += sprintf(g_sa+pos,"%d,",clsStat.status);
        }
        else
        {
          pos += sprintf(g_sa+pos,",");
        }
        if (clsStat.class_type NEQ CLASS_NotPresent)
        {
          pos += sprintf(g_sa+pos,"%d,",clsStat.class_type);
        }
        ci_remTrailCom(g_sa, pos);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        return (ATI_CMPL);
      }
    }
    else /*set mode*/
    {
      ret = sAT_PlusCLCK((T_ACI_CMD_SRC)srcId, fac_num, mod, passwd, class_type);
    }

    switch(ret)
    {
      case(AT_EXCT):
        break;

      case(AT_BUSY):
        cmdCmeError(CME_ERR_Unknown);
        TRACE_EVENT("ME is busy");
        break;

      case(AT_FAIL):
        cmdCmeError(CME_ERR_Unknown);
        break;
    }
    return (map_aci_2_ati_rslt(ret));
  }
#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPWD         |
+--------------------------------------------------------------------+

  PURPOSE : +CPWD command (changes password)
*/

GLOBAL T_ATI_RSLT setatPlusCPWD(char *cl, UBYTE srcId)
{
  T_ACI_RETURN        ret = AT_FAIL;
  T_ACI_FAC      fac_num = FAC_NotPresent;
  CHAR                oldpwd[MAX_PWD_LENGTH]={0};
  CHAR                newpwd[MAX_PWD_LENGTH]={0};
  CHAR                fac_str[3]={0};
  USHORT              i;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CPWD;

  cl = parse(cl, "snn", (LONG)3, fac_str, (LONG)MAX_PWD_LENGTH, oldpwd, (LONG)MAX_PWD_LENGTH, newpwd);
  strcpy(fac_str, (char *)strupper(fac_str));
  if(!cl OR *fac_str EQ 0 OR *oldpwd EQ 0 OR *newpwd EQ 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  for (i = 0; fac[i].name NEQ 0; i++)
  {
    if (strcmp(fac[i].name,fac_str) EQ 0 )
    {
      fac_num=fac[i].fac;           /*translates facility string in facility number */
      break;
    }
  }
  if (fac[i].name EQ 0)                   /* facility non existent*/
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cpwd my_bat_set_plus_cpwd;

  TRACE_FUNCTION("setatPLusCPWD() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cpwd, 0, sizeof(my_bat_set_plus_cpwd));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CPWD;
  cmd.params.ptr_set_plus_cpwd = &my_bat_set_plus_cpwd;

  my_bat_set_plus_cpwd.fac = fac_num;       
  my_bat_set_plus_cpwd.c_oldpwd = strlen(oldpwd);  
  memcpy(my_bat_set_plus_cpwd.oldpwd, oldpwd, my_bat_set_plus_cpwd.c_oldpwd); 
  my_bat_set_plus_cpwd.c_newpwd = strlen(newpwd); 
  memcpy(my_bat_set_plus_cpwd.newpwd, newpwd, my_bat_set_plus_cpwd.c_newpwd); 

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCPWD()");

  ret = sAT_PlusCPWD((T_ACI_CMD_SRC)srcId, fac_num, oldpwd, newpwd);
  if (ret EQ AT_CMPL)
  {
    return (ATI_CMPL);
  }
  else if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd = AT_CMD_CPWD;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCUSD         |
+--------------------------------------------------------------------+

  PURPOSE : +CUSD Unstructured supplementary data
*/
GLOBAL T_ATI_RSLT setatPlusCUSD(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret                     = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  CHAR             ussd    [MAX_USSD_LEN]  = {0x00};
  USHORT           lenUssd                 = 0;

#ifndef FF_ATI_BAT
  T_ACI_USSD_DATA  cvtdUssd;
  T_ACI_USSD_DATA* p_cvtdUssd              = NULL;
  USHORT           lenCvtdUssd             = 0;
#endif

  SHORT            dcs                     = ACI_NumParmNotPresent,
                   stat                    = ACI_NumParmNotPresent;
  BOOL             status_changed          = FALSE;

  TRACE_FUNCTION("setatPlusCUSD()");

  cl = parse(cl,"rzr",
           &stat,
           (LONG)MAX_USSD_LEN,
           strlen(cl),
           cl,
           &lenUssd,
           ussd,
           &dcs);

  if(cl EQ NULL OR 
     stat > 2)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if(stat EQ 2)
  {
    /* CANCEL ONGOING USSD OPERATION */
    ret = sAT_end_ussd((T_ACI_CMD_SRC)srcId);
  }
  else
  {
    if (dcs EQ 17 AND ati_user_output_cfg[srcId].cscsChset NEQ CSCS_CHSET_Ucs2)
    {
      cmdCmeError(CME_ERR_OpNotAllow);    /* it makes no sense if TE<->MS is GSM7 but we talk to the Network
                                             in UCS2 since afterwards we need to convert it back to GSM7 and
                                             we will loose some characters */
      return (ATI_FAIL);
    }

    if (stat NEQ ACI_NumParmNotPresent )
    {
      /* SET CUSD Presentation Flag only for values stat 0 and 1 */
      ati_user_output_cfg[srcId].CUSD_stat = (UBYTE)stat;
      status_changed = TRUE;
    }

    if (dcs EQ ACI_NumParmNotPresent )
    {
      /* GSM 03.38 [25] Cell Broadcast Data Coding Scheme in integer format (default 0) */
      dcs = 0;
    }

    srcId_cb = srcId;

#ifdef FF_ATI_BAT

    {
      T_BAT_cmd_send cmd;
      T_BAT_cmd_set_plus_cusd cusd;

      cmd.ctrl_params=BAT_CMD_SET_PLUS_CUSD;
      cmd.params.ptr_set_plus_cusd=&cusd;

      /*
      *   This is the only possible value for 'n', as the other is
      *   dealt with further up this function.
      */
      cusd.n=BAT_CUSD_N_NOT_PRESENT;

      /*
      *   Simplest thing to do is copy in as much data as the BAT
      *   structure can handle. If there is too much we won't
      *   overwrite memory.
      */
      memcpy(cusd.str,ussd,BAT_MAX_USSD_LEN);

      /*
      *   Set the length. If there is too much data it will
      *   effectively be truncated.
      */
      cusd.c_str=(U8)((lenUssd>BAT_MAX_USSD_LEN) ? BAT_MAX_USSD_LEN:lenUssd);
      cusd.v_str=TRUE;

      cusd.dcs=(S16)dcs;

      bat_send(ati_bat_get_client(srcId), &cmd);

      src_params->curAtCmd=AT_CMD_CUSD;

      return(ATI_EXCT);
    }

#else /* no FF_ATI_BAT */

    utl_ussdDtaFromTe ((UBYTE*)ussd,
                       lenUssd,
                       (UBYTE*)cvtdUssd.data,
                       &lenCvtdUssd, 
#ifdef REL99
                       sizeof(cvtdUssd.data),
#endif /* REL99 */
                       (UBYTE)dcs);
  
    cvtdUssd.len = (UBYTE)lenCvtdUssd;
  
    if (cvtdUssd.len NEQ 0)
    {
      p_cvtdUssd=&cvtdUssd;
    }

    if( p_cvtdUssd NEQ NULL ) 
    {
      ret = sAT_PlusCUSD((T_ACI_CMD_SRC)srcId, p_cvtdUssd, dcs);
    }
    else
    {
      if (lenUssd NEQ 0)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
    }
#endif /* no FF_ATI_BAT */
    
  }

  if (ret EQ AT_CMPL)
  {
    return (ATI_CMPL);
  }
  else if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_CUSD;
    return (ATI_EXCT);
  }
  else if(status_changed)
  {
    return (ATI_CMPL);
  }

  cmdCmeError(CME_ERR_Unknown);
  return (ATI_FAIL);
}

GLOBAL T_ATI_RSLT queatPlusCUSD(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPlusCUSD()");

  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].CUSD_stat);
  return (ATI_CMPL);
}

#ifdef TI_PS_FF_AT_P_CMD_CSCN
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCSCN      |
+--------------------------------------------------------------------+

  PURPOSE : %CSCN - network service change notifications
*/
GLOBAL T_ATI_RSLT setatPercentCSCN (char *cl, UBYTE srcId)
{
  T_ACI_SS_CSCN_MOD_STATE     ss_switch    = SS_CSCN_MOD_STATE_INVALID;
  T_ACI_SS_CSCN_MOD_DIRECTION ss_direction = SS_CSCN_MOD_DIR_INVALID;
  T_ACI_CC_CSCN_MOD_STATE     cc_switch    = CC_CSCN_MOD_STATE_INVALID;
  T_ACI_CC_CSCN_MOD_DIRECTION cc_direction = CC_CSCN_MOD_DIR_INVALID;

  TRACE_FUNCTION("setatPercentCSCN()");

  if((cl EQ NULL) OR (*cl EQ '\0'))  /* No parameter passed ?? */
  {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }

  cl = parse(cl,"dddd", &ss_switch, &ss_direction, &cc_switch, &cc_direction);

   /* valid parameter ?? */
  if( (ss_switch    >= SS_CSCN_MOD_STATE_MAX)       OR
      (ss_direction >= SS_CSCN_MOD_DIR_MAX) OR
      (cc_switch    >= CC_CSCN_MOD_STATE_MAX)       OR
      (cc_direction >= CC_CSCN_MOD_DIR_MAX)    )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if (sAT_PercentCSCN( (T_ACI_CMD_SRC)srcId, ss_switch, ss_direction, cc_switch, cc_direction ) NEQ AT_CMPL)
    return (ATI_FAIL);

  return (ATI_CMPL);
}

/* ------------------------------------------------------------------------- */
GLOBAL T_ATI_RSLT queatPercentCSCN (char* cl, UBYTE srcId)
/* ------------------------------------------------------------------------- */
{
  T_ACI_SS_CSCN_MOD_STATE     ss_switch;
  T_ACI_SS_CSCN_MOD_DIRECTION ss_direction;
  T_ACI_CC_CSCN_MOD_STATE     cc_switch;
  T_ACI_CC_CSCN_MOD_DIRECTION cc_direction;

  TRACE_FUNCTION("queatPercentCSCN()");

  if (qAT_PercentCSCN( (T_ACI_CMD_SRC)srcId, &ss_switch, &ss_direction, &cc_switch, &cc_direction ) NEQ AT_CMPL)
    return (ATI_FAIL);

  if( ss_switch    EQ SS_CSCN_MOD_STATE_INVALID )  ss_switch    = SS_CSCN_MOD_STATE_OFF;
  if( ss_direction EQ SS_CSCN_MOD_DIR_INVALID )    ss_direction = SS_CSCN_MOD_DIR_IN;
  if( cc_switch    EQ CC_CSCN_MOD_STATE_INVALID )  cc_switch    = CC_CSCN_MOD_STATE_OFF;
  if( cc_direction EQ CC_CSCN_MOD_DIR_INVALID )    cc_direction = CC_CSCN_MOD_DIR_IN;

  resp_disp(srcId, cl, "ssss", &ss_switch, &ss_direction, &cc_switch, &cc_direction);
  return (ATI_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

#ifdef TI_PS_FF_AT_P_CMD_CUSDR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCUSDR  |
+--------------------------------------------------------------------+

  PURPOSE : %CUSDR - Extended response for n/w initiated USSD.
*/
GLOBAL T_ATI_RSLT setatPercentCUSDR(char* cl, UBYTE srcId)
{
  T_ACI_CUSDR_RES response;
  T_ACI_RETURN ret   = AT_FAIL;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentCUSDR()");

  cl = parse(cl,"d",&response);
  
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PercentCUSDR((T_ACI_CMD_SRC)srcId, response);

  return (map_aci_2_ati_rslt(ret));

}
#endif /* TI_PS_FF_AT_P_CMD_CUSDR */


#ifdef SIM_PERS

/* ----------Added on 11/03/2005--------------------*/
/*-------------For %MEPD AT comand----------------- */
/* ------------------------------------------------------------------------- */
/*
+------------------------------------------------------------------------------
|  Function    : setatPercentMEPD
+------------------------------------------------------------------------------
|  Description : For %MEPD= AT comand
|
|  Parameters  :char* cl        
|         UBYTE srcId   
|
|  Return      :     ATI_FAIL            -  at command failed
|                    ATI_FAIL_NO_OUTPUT  -  at command failed with no output
|                    ATI_BUSY            -  ATI busy
|                    ATI_EXCT            -  ATI is executing the command
|                    ATI_CMPL            -  at command successfully executed
|                    ATI_CMPL_NO_OUTPUT  -  at command successfully executed with no output
|
+------------------------------------------------------------------------------
*/
GLOBAL T_ATI_RSLT setatPercentMEPD (char* cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL; 
  char           sup_info_str[4] = {0,0,0,0};
  T_SUP_INFO_TYPE sup_info_num = CMEPD_SUP_INFO_NotPresent;
  T_SUP_INFO sup_info;  /*return Value of MEPD CFG Data */
  USHORT         i;
 T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_MEPD;
  TRACE_FUNCTION("setatPercentMEPD()");
    
  /*input functionality*/
  cl = parse (cl,"s",(LONG)4, sup_info_str);
  
  if ( !cl OR (sup_info_str[0] EQ 0))
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return  (ATI_FAIL);
  }
  
  strcpy(sup_info_str, strupper(sup_info_str));
  for (i=0;mepd_sup_info[i].name NEQ 0; i++)
  {
    if (strcmp(mepd_sup_info[i].name, sup_info_str) EQ 0 )
    {
      sup_info_num = mepd_sup_info[i].sup_info;           /*translates facility string in facility number */
      break;
    }
  }
  if (sup_info_num EQ CMEPD_SUP_INFO_NotPresent) /* facility non existent*/
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  sup_info.infoType=sup_info_num;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_mepd mepd;

    memset(&mepd, 0, sizeof(mepd));
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_MEPD;
    cmd.params.ptr_set_percent_mepd = &mepd;
    mepd.sup_info_type = (T_BAT_percent_mepd_sup_info)sup_info_num;

    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else
  ret = qAT_PercentMEPD((T_ACI_CMD_SRC)srcId, &sup_info);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId,cl,"b", &sup_info.datavalue);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return  (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */
}

#endif /* SIM_PERS */
#endif /* ATI_SS_C */

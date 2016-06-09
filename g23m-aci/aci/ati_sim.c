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
|  Purpose :  AT Command Interpreter: SIM related functions.
+-----------------------------------------------------------------------------
*/

#ifndef ATI_SIM_C
#define ATI_SIM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"

#include "gdi.h"
#include "audio.h"

#include "aci_mem.h"
#include "aci_ext_pers.h"
#include "aci_slock.h"
#include "aci_prs.h"
#include "aci_lst.h"
#include "ati_int.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

/*==== EXPORT ==================================================*/

EXTERN CHAR *cmdExtError (T_ACI_EXT_ERR e);

#ifdef TI_PS_FF_AT_P_CMD_SECP
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentSECP   |
+--------------------------------------------------------------------+

  PURPOSE : %SECP command (PIN functionality for MMI Security)
*/

GLOBAL T_ATI_RSLT setatPercentSECP(CHAR * cl, UBYTE srcId)
{
  char pinOld [MAX_PWD_LENGTH]={0};                 
  char pinNew [MAX_PWD_LENGTH]={0};
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION ("setatPercentSECP()");

  cl = parse(cl, "nn", (LONG)MAX_PWD_LENGTH, pinOld,(LONG)MAX_PWD_LENGTH, pinNew);
  if ( !cl OR (*pinOld EQ '\0' AND *pinNew EQ '\0') )  
  {   
    cmdCmeError(CME_ERR_OpNotAllow);    
    return ATI_FAIL;  
  }
  
  ret = sAT_PercentSECP((T_ACI_CMD_SRC)srcId, pinOld, pinNew);

  return (map_aci_2_ati_rslt(ret));
}
#endif /* TI_PS_FF_AT_P_CMD_SECP */

#ifdef TI_PS_FF_AT_P_CMD_SECS
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentSECS         |
+--------------------------------------------------------------------+

  PURPOSE : %SECS command (This function changes the state of the MMI Security code
                  It requires the existing security code as one of the parameters)
*/


GLOBAL  T_ATI_RSLT setatPercentSECS (CHAR *cl, UBYTE srcId)
{
  char code [MAX_PWD_LENGTH]={0};
  T_ACI_SECS_STA securityState = SECS_STA_NotPresent;
  T_ACI_RETURN ret = AT_FAIL;
  

  TRACE_FUNCTION ("setatPercentSECS()");

  /* Get the required state and the Security code from the command parameters*/
  cl = parse(cl, "dn", &securityState, (LONG)MAX_PWD_LENGTH, code);

  if ( !cl OR *code EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  ret = sAT_PercentSECS((T_ACI_CMD_SRC)srcId,securityState,code);
  return (map_aci_2_ati_rslt(ret));


}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD                                 |
| STATE   : code                        ROUTINE : qetatPercentSECS                               |
+--------------------------------------------------------------------+

  PURPOSE : %SECS? command (It queries the state of the MMI Security code)
*/


GLOBAL  T_ATI_RSLT quetatPercentSECS (CHAR *cl, UBYTE srcId)
{
  T_ACI_SECS_STA securityState = SECS_STA_NotPresent;
  T_ACI_RETURN ret = AT_FAIL;
  

  TRACE_FUNCTION ("qetatPercentSECS()");

  /* Get the State value from the Command Handler */
  ret = qAT_PercentSECS((T_ACI_CMD_SRC)srcId, &securityState);

    if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"e",&securityState);
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }


}
#endif /* TI_PS_FF_AT_P_CMD_SECS */


GLOBAL CHAR *CPIN_RESULT(T_ACI_CPIN_RSLT code)
{
  switch (code)
  {
    default:
    case(CPIN_RSLT_NotPresent): return"not present";
    case(CPIN_RSLT_SimReady):   return"READY";
    case(CPIN_RSLT_SimPinReq):  return"SIM PIN";
    case(CPIN_RSLT_SimPukReq):  return"SIM PUK";
    case(CPIN_RSLT_PhSimPinReq):return"PH-SIM PIN";
    case(CPIN_RSLT_SimPin2Req): return"SIM PIN2";
    case(CPIN_RSLT_SimPuk2Req): return"SIM PUK2";
        /* OVK: Extended list of all possible result according to 07.07 */
    case(CPIN_RSLT_PhFSimPinReq): return"PH-FSIM PIN";
    case(CPIN_RSLT_PhFSimPukReq): return"PH-FSIM PUK";
    case(CPIN_RSLT_PhNetPinReq): return"PH-NET PIN";
    case(CPIN_RSLT_PhNetPukReq): return"PH_NET PUK";
    case(CPIN_RSLT_PhNetSubPinReq): return"PH-NETSUB PIN";
    case(CPIN_RSLT_PhNetSubPukReq): return"PH-NETSUB PUK";
    case(CPIN_RSLT_PhSPPinReq): return"PH-SP PIN";
    case(CPIN_RSLT_PhSPPukReq): return"PH-SP PUK";
    case(CPIN_RSLT_PhCorpPinReq): return"PH-CORP PIN";
    case(CPIN_RSLT_PhCorpPukReq): return"PH-CORP PUK";


  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPIN         |
+--------------------------------------------------------------------+

  PURPOSE : +CPIN command (PIN functionality of ME)
*/

GLOBAL T_ATI_RSLT setatPlusCPIN (char *cl, UBYTE srcId)
{
  char pinOld [MAX_PWD_LENGTH]={0};                 /* ES!! enough for bad guys?*/
  char pinNew [MAX_PWD_LENGTH]={0};
  T_ACI_RETURN ret = AT_FAIL;
  UBYTE slockBlocked = 0;              /* @GBR: AT_CMPL is interpreted as error per default. This is wrong, if the ME is SIMLOCKed */

  TRACE_FUNCTION ("setatPlusCPIN()");

  cl = parse(cl,"nn",(LONG)MAX_PWD_LENGTH,&pinOld,(LONG)MAX_PWD_LENGTH,&pinNew);
  if ( !cl OR *pinOld EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  #ifdef SIM_PERS
  slockBlocked = AciSLockShrd.blocked;
  #endif
#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpin my_bat_set_plus_cpin = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
    
    TRACE_FUNCTION("setatPlusCPIN() calls bat_send() <=== as APPLICATION");

    /* memset(&my_bat_set_plus_cpin,  FALSE, sizeof(my_bat_set_plus_cpin)); Already initilaised */
    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPIN;
    cmd.params.ptr_set_plus_cpin = &my_bat_set_plus_cpin;

    /* Set old pin BAT attribs */
    my_bat_set_plus_cpin.c_pin = strlen(pinOld);
    memcpy(my_bat_set_plus_cpin.pin, pinOld, BAT_MAX_CPIN_PIN_LEN);

    /* Set new pin BAT attribs */
    if(my_bat_set_plus_cpin.c_newpin = strlen(pinNew))
    {
      my_bat_set_plus_cpin.v_newpin = TRUE;
      memcpy(my_bat_set_plus_cpin.pin, pinNew, BAT_MAX_CPIN_PIN_LEN);
    }

    src_params->curAtCmd = AT_CMD_CPIN;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
  
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("setatPlusCPIN()");
  ret = sAT_PlusCPIN ((T_ACI_CMD_SRC)srcId,pinOld,pinNew);

  switch(ret)
  {
    case(AT_EXCT):
    {
      break;
    }
    case(AT_BUSY):
    {
      cmdCmeError(CME_ERR_SimBusy);
      break;
    }
    case(AT_FAIL):
    {
      cmdCmeError(CME_ERR_Unknown);
      break;
    }
    case(AT_CMPL):
    {
      if (!slockBlocked)
      {
        cmdCmeError(CME_ERR_Unknown);
      }
      break;
    }
  }
  return (map_aci_2_ati_rslt(ret));
#endif  /* FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCPIN (char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  src_params->curAtCmd = AT_CMD_CPIN;

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPlusCPIN() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CPIN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cpin = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  {
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CPIN_RSLT code;

  TRACE_FUNCTION ("queatPlusCPIN()");

  ret= qAT_PlusCPIN ((T_ACI_CMD_SRC)srcId,&code);    /*PIN status is requested*/
  switch (ret)
  {
    case(AT_CMPL):
    {
      sprintf (g_sa,"+CPIN: %s",CPIN_RESULT(code));
      io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
      break;
    }
    case(AT_EXCT):
    {
      src_params->curAtCmd    = AT_CMD_CPIN;
      break;
    }
    default:
    {
      cmdCmeError (CME_ERR_Unknown);
      break;
    }
  }
  return (map_aci_2_ati_rslt(ret));
  }
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPOL         |
+--------------------------------------------------------------------+

  PURPOSE : +CPOL command (preferred operator list)
*/

GLOBAL T_ATI_RSLT setatPlusCPOL (char *cl, UBYTE srcId)
{
  T_ACI_CPOL_FRMT format = CPOL_FRMT_NotPresent;
  SHORT           index = ACI_NumParmNotPresent;
  char            op[20] = {0};
  T_ACI_RETURN    ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCPOL()");

  cl = parse(cl,"dds",&index,&format,(LONG)20,op);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpol cpol;
    UBYTE len;

    len=strlen(op);

    /*
    *   If the operator name is too big for the BAT structure, there's
    *   nothing we can do. A truncated version would be meaningless, and
    *   to omit it would result in an operator being deleted from the
    *   list.
    */
    if (len>BAT_MAX_CPOL_OPER_LEN)
      return(ATI_FAIL);

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPOL;
    cmd.params.ptr_set_plus_cpol=&cpol;

    cpol.index=(S16)index;

    /*
    *   This relies on T_ACI_CPOL_FRMT being identical to
    *   T_BAT_plus_cpol_format.
    */
    cpol.format=(T_BAT_plus_cpol_format)format;

    if (len>0)
    {
      memcpy(cpol.oper,op,len);
      cpol.v_oper=TRUE;
      cpol.c_oper=(U8)len;
    }
    else
    {
      cpol.v_oper=FALSE;
    }

    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CPOL;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

#ifdef WIN32
  ret=sAT_PlusCPOL(srcId,index,format,
                   ((op[0] NEQ 0x0)?op:NULL),
                   cpolIdx2,
                   cpolMode);
#else
  ret=sAT_PlusCPOL((T_ACI_CMD_SRC)srcId,index,format,
                   ((op[0] NEQ 0x0)?op:NULL),
                   ACI_NumParmNotPresent,
                   CPOL_MOD_NotPresent);
#endif


  switch (ret)
  {
  case (AT_CMPL):
    return ATI_CMPL;
  case (AT_EXCT):
    src_params->curAtCmd    = AT_CMD_CPOL;
    return ATI_EXCT;
  default:
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT tesatPlusCPOL (char *cl, UBYTE srcId)
{
  SHORT        lastIdx=0,usdNtry=0;
  T_ACI_RETURN    ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("tesatPlusCPOL()");

  src_params->curAtCmd=AT_CMD_CPOL;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_TST_PLUS_CPOL;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_tst_plus_cpol = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = tAT_PlusCPOL((T_ACI_CMD_SRC)srcId,&lastIdx,&usdNtry);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+CPOL: (1-%d),(0-2)", lastIdx);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd  = AT_CMD_CPOL;
  }
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCPOL (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_CPOL_LST  plmnSelLst;
  UBYTE           idx;
  BOOL            loop;
#endif

  SHORT        startIdx=0,lastIdx=0;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd=AT_CMD_CPOL;

  TRACE_FUNCTION("queatPlusCPOL()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CPOL;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cpol = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

#ifdef WIN32
  switch (qAT_PlusCPOL ( srcId,
                         1, &lastIdx,
                         &plmnSelLst[0],
                         cpolMode))
#else
  switch (qAT_PlusCPOL ( (T_ACI_CMD_SRC)srcId,
                         1, &lastIdx,
                         &plmnSelLst[0],
                         CPOL_MOD_NotPresent))
#endif
  {
    case AT_CMPL:
      loop = TRUE;
      do
      {
        if( lastIdx EQ ACI_NumParmNotPresent )
          break;

        startIdx = lastIdx+1;

        for( idx=0; idx < MAX_OPER; idx++ )
        {
          if( plmnSelLst[idx].index EQ ACI_NumParmNotPresent )
          {
            loop = FALSE;
            break;
          }
          sprintf(g_sa,"+CPOL: %d,%d,\"%s\"", plmnSelLst[idx].index,
                                              plmnSelLst[idx].format,
                                              plmnSelLst[idx].oper );
          io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        }

        if( qAT_PlusCPOL((T_ACI_CMD_SRC)srcId, startIdx, &lastIdx, &plmnSelLst[0],
#ifdef WIN32
            cpolMode
#else
            CPOL_MOD_NotPresent
#endif
           ) EQ AT_FAIL OR !loop)
        {
          break;
        }
      }
      while( loop );
      return ATI_CMPL;

    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_CPOL;
      return ATI_EXCT;

    default:
      cmdCmeError(CME_ERR_Unknown);
      return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCNUM         |
+--------------------------------------------------------------------+

  PURPOSE : +CNUM command (subscriber number)
*/

GLOBAL T_ATI_RSLT setatPlusCNUM (char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("setatPlusCNUM()");

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("setatPlusCNUM() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CNUM;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cnum = &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);
 
    src_params->curAtCmd=AT_CMD_CNUM;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  if ( qAT_PlusCNUM ( (T_ACI_CMD_SRC)srcId, CNUM_MOD_NewRead ) EQ AT_EXCT )
  {
    src_params->curAtCmd    = AT_CMD_CNUM;
    return ATI_EXCT;
  }
  else
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCFUN         |
+--------------------------------------------------------------------+

  PURPOSE : +CFUN command (Phone functionality)
*/

GLOBAL T_ATI_RSLT setatPlusCFUN (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret   = AT_FAIL;
  T_ACI_CFUN_FUN fun = CFUN_FUN_NotPresent;
  T_ACI_CFUN_RST rst = CFUN_RST_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CFUN;
  
  cl = parse (cl, "dd", &fun, &rst);
  if ( !cl )
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cfun my_bat_set_plus_cfun;
    
    TRACE_FUNCTION("setatPlusCFUN() calls bat_send() <=== as APPLICATION");

    memset(&my_bat_set_plus_cfun,  FALSE, sizeof(my_bat_set_plus_cfun));
    cmd.ctrl_params = BAT_CMD_SET_PLUS_CFUN;
    cmd.params.ptr_set_plus_cfun = &my_bat_set_plus_cfun;

    my_bat_set_plus_cfun.fun = (T_BAT_VAL_plus_cfun_fun)fun;
    my_bat_set_plus_cfun.rst = (T_BAT_VAL_plus_cfun_rst)rst;


    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  {
    TRACE_FUNCTION("setatPlusCFUN()");
    ret = sAT_PlusCFUN ((T_ACI_CMD_SRC)srcId,fun,rst);
    switch (ret)
    {
      case AT_EXCT:               /*Command is executing*/
        audio_SetAmplf (AUDIO_MICROPHONE, (UBYTE)75);
        audio_SetAmplf (AUDIO_SPEAKER,    (UBYTE)175);
        src_params->curAtCmd    = AT_CMD_CFUN;
        break;

      case AT_CMPL:
        src_params->curAtCmd    = AT_CMD_NONE;
        break;

      default: /* AT_FAIL or nothing was returned */
        cmdExtError (EXT_ERR_NotPresent);
        break;
    }
    return (map_aci_2_ati_rslt (ret));
  }
#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCFUN (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CFUN_FUN fun = CFUN_FUN_NotPresent;

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPlusCFUN() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CFUN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cfun = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  TRACE_FUNCTION("queatPlusCFUN()");

  ret = qAT_PlusCFUN((T_ACI_CMD_SRC)srcId, &fun);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"e",&fun);
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCIMI         |
+--------------------------------------------------------------------+

  PURPOSE : +CIMI command (request international mobile subscriber
                           identity)
*/
GLOBAL T_ATI_RSLT setatPlusCIMI(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  CHAR imsi[MAX_IMSI_LEN+1];        /* +1 for '\0' */
#endif

  T_ACI_RETURN   ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCIMI()");

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("setatPlusCIMI() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CIMI;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cimi = &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);
 
    src_params->curAtCmd=AT_CMD_CIMI;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = qAT_PlusCIMI((T_ACI_CMD_SRC)srcId, imsi);
  switch (ret)
  {
    case( AT_CMPL ):

      sprintf(g_sa,"%s",imsi);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      break;

    case( AT_EXCT ):
      src_params->curAtCmd    = AT_CMD_CIMI;
      break;

    default:

      break;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCRSM         |
+--------------------------------------------------------------------+

  PURPOSE : +CRSM Restricted SIM access
*/
GLOBAL T_ATI_RSLT setatPlusCRSM(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret        = AT_FAIL;
  UBYTE           *data;
  USHORT           lenData    = 0;
  T_ACI_CRSM_CMD   cmd        = CRSM_CMD_NotPresent;
  SHORT            fileId     = ACI_NumParmNotPresent,
                   p1         = ACI_NumParmNotPresent,
                   p2         = ACI_NumParmNotPresent,
                   p3         = ACI_NumParmNotPresent;
  USHORT           lenDataStr = 0;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("setatPlusCRSM");

  cl = parse(cl,"drrrrl", &cmd,&fileId, &p1, &p2, &p3,&lenDataStr,&data );
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  utl_hexToGsm( data, lenDataStr, data, &lenData, GSM_ALPHA_Def,
                CSCS_ALPHA_Reserved );

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send bat_cmd;
    T_BAT_cmd_set_plus_crsm crsm;

    /*
    *   If the data is too big for the BAT structure, there's
    *   nothing we can do. A truncated version would be meaningless.
    */
    if (lenData>BAT_MAX_CRSM_DATA_LEN)
      return(ATI_FAIL);

    bat_cmd.ctrl_params = BAT_CMD_SET_PLUS_CRSM;
    bat_cmd.params.ptr_set_plus_crsm=&crsm;

    /*
    *   This relies on T_ACI_CRSM_COMMAND and T_BAT_plus_crsm_command
    *   being identical. They are, except in that the ACI version has
    *   a 'not present' value. This is not an issue in this direction.
    */
    crsm.command=(T_BAT_plus_crsm_command)cmd;

    crsm.fileid=(S32)fileId;
    crsm.p1=(S16)p1;
    crsm.p2=(S16)p2;
    crsm.p3=(S16)p3;
    crsm.c_data=(U8)lenData;
    memcpy(crsm.data,data,lenData);

    bat_send(ati_bat_get_client(srcId),&bat_cmd);

    src_params->curAtCmd=AT_CMD_CRSM;
    return(ATI_EXCT);
  }
#else /* no FF_ATI_BAT */

  ret = sAT_PlusCRSM( (T_ACI_CMD_SRC)srcId, cmd, fileId, p1, p2, p3,
                      lenData, data );
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd = AT_CMD_CRSM;
  }
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCSIM      |
+--------------------------------------------------------------------+

  PURPOSE : +CSIM Generic SIM access
*/
GLOBAL T_ATI_RSLT setatPlusCSIM(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret        = AT_FAIL;
  USHORT           given_length;
  UBYTE           *data;
  USHORT           lenData    = 0;
  USHORT           lenDataStr = 0;


  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("setatPlusCSIM");

  cl = parse(cl,"rl", &given_length, &lenDataStr, &data );
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  if ((given_length > MAX_SIM_TRANSP*2)
       OR (given_length NEQ lenDataStr)
       OR (lenDataStr EQ 0))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  lenData = utl_HexStrToBin(data, lenDataStr, data, MAX_SIM_TRANSP);

  if (lenData EQ 0)
  {
    TRACE_EVENT("invalid character in <command>");
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send bat_cmd;
    T_BAT_cmd_set_plus_csim csim;

    /*
    *   If the data is too big for the BAT structure, there's
    *   nothing we can do. A truncated version would be meaningless.
    */
    if (lenData>BAT_MAX_CSIM_CMD_LEN)
      return(ATI_FAIL);

    bat_cmd.ctrl_params = BAT_CMD_SET_PLUS_CSIM;
    bat_cmd.params.ptr_set_plus_csim=&csim;

    csim.c_command=(U8)lenData;
    memcpy(csim.command,data,lenData);

    bat_send(ati_bat_get_client(srcId),&bat_cmd);

    src_params->curAtCmd=AT_CMD_CRSM;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = sAT_PlusCSIM( (T_ACI_CMD_SRC)srcId, lenData, data );
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd = AT_CMD_CSIM;
  }
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

#ifdef TI_PS_FF_AT_P_CMD_ATR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentATR    |
+--------------------------------------------------------------------+

  PURPOSE : %ATR answer to reset (query)
*/
GLOBAL T_ATI_RSLT queatPercentATR ( char *cl, UBYTE srcId )
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret;
  UBYTE i;
  UBYTE phase;
  UBYTE atr_len = 0;
  UBYTE atr_info[MAX_SIM_ATR];
#endif

  TRACE_FUNCTION( "queatPercentATR()" );

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_ATR;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_atr = &dummy;

    /*
    *   Without this we will lose track of the source ID by the time the
    *   response arrives.
    */
    srcId_cb=srcId;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PercentATR( (T_ACI_CMD_SRC)srcId, &phase, &atr_len, atr_info );

  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  else
  {
    if(phase NEQ NOT_PRESENT_8BIT)
    {
      i=sprintf(g_sa,"%s%d,", "%ATR: ", phase);/* if phase available.. return phase*/
    }
    else
    {
      i=sprintf(g_sa,"%s", "%ATR: FFFF");/* else return FFFF*/
    }

    if(atr_len)                    /* if ATR data available... return ATR data*/
    {
      sprintf(g_sa+i,","); 
      utl_binToHex( atr_info, atr_len, g_sa+i );
    }
        
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}
#endif /* TI_PS_FF_AT_P_CMD_ATR */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCPRI      |
+--------------------------------------------------------------------+

  PURPOSE : %CPRI command (select cipher indication mode)
*/

GLOBAL T_ATI_RSLT setatPercentCPRI ( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ACI_return = AT_FAIL;
  UBYTE CPRImode;

  TRACE_FUNCTION( "setatPercentCPRI()" );

  switch( *cl )
  {
  case '0':
  case '1':
    CPRImode = *cl - 0x30; /* make binary value */
    cl++;
    ACI_return = sAT_PercentCPRI( (T_ACI_CMD_SRC)srcId, CPRImode );
    if( ACI_return NEQ AT_CMPL )
    {
      cmdCmeError(CME_ERR_Unknown);
    }
    else                                         /* If ACI_return is AT_CMPL,updates the CPRI_stat with CPRImode */
    {
      ati_user_output_cfg[srcId].CPRI_stat = CPRImode;
    }
    break;
  default:
    cmdCmeError(CME_ERR_OpNotAllow);
    break;
  }
  return (map_aci_2_ati_rslt(ACI_return));
}


GLOBAL T_ATI_RSLT queatPercentCPRI ( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ACI_return = AT_FAIL;
  UBYTE CPRIenabled;

  TRACE_FUNCTION( "queatPercentCPRI()" );

  ACI_return = qAT_PercentCPRI( (T_ACI_CMD_SRC)srcId, &CPRIenabled );
  
  if( ACI_return NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  else
  {
    if (CPRIenabled)
    {
      CPRIenabled = ati_user_output_cfg[srcId].CPRI_stat;
    }
    else
    {
      CPRIenabled = CI_DISABLED;
    }
    resp_disp(srcId, cl,"b",&CPRIenabled);
  }
  return (map_aci_2_ati_rslt(ACI_return));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  :                     |
| STATE   : code                       ROUTINE :  setatPercentPVRF   |
+--------------------------------------------------------------------+

  PURPOSE : return information relating to status of PIN.

*/

GLOBAL T_ATI_RSLT setatPercentPVRF( char *cl, UBYTE srcId )
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_ACI_PVRF_TYPE type;
  CHAR            pin[10];
  CHAR            newpin[10];
  T_ACI_RETURN    ret          = AT_FAIL;

  TRACE_FUNCTION( "setatPercentPVRF()" );

  /* parse mode */
  cl = parse (cl, "dss", &type, (LONG) sizeof (pin),pin, (LONG) sizeof (newpin), newpin);
  if (!cl OR (type > PVRF_TYPE_Puk2) OR  (type <= PVRF_TYPE_NotPresent))/*lint !e685 (Warning -- :685 Relational operator '>' always evaluates to 'false ) */
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_pvrf pvrf;
    UBYTE len;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_PVRF;
    cmd.params.ptr_set_percent_pvrf=&pvrf;

    /*
    *   This relies on T_ACI_PVRF_TYPE and T_BAT_percent_pvrf_type
    *   being identical. They are, except in that the ACI version has
    *   a 'not present' value. In this direction, that doesn't matter.
    */
    pvrf.type=(T_BAT_percent_pvrf_type)type;

    len=strlen(pin);
    if (len>BAT_MAX_PVRF_PIN_LEN)
      return(ATI_FAIL);

    pvrf.c_pin=(U8)len;
    memcpy(pvrf.pin,pin,len);

    len=strlen(newpin);
    if (len>BAT_MAX_PVRF_NEW_PIN_LEN)
      return(ATI_FAIL);

    if (len)
    {
      pvrf.v_newpin=TRUE;
      pvrf.c_newpin=(U8)len;
      memcpy(pvrf.newpin,newpin,len);
    }
    else
    {
      pvrf.v_newpin=FALSE;
    }
    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_PVRF;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = sAT_PercentPVRF ((T_ACI_CMD_SRC)srcId, type, pin, newpin);

  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      break;
    case (AT_EXCT):
      src_params->curAtCmd = AT_CMD_PVRF;
      break;
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      break;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : queatPercentPVRF   |
+--------------------------------------------------------------------+

  PURPOSE : %PVRF command
*/
GLOBAL T_ATI_RSLT queatPercentPVRF (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPercentPVRF()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_PVRF;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_pvrf = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */
  {
  T_ACI_PVRF_STAT ps1;
  T_ACI_PVRF_STAT ps2;
  CHAR  *me = "%PVRF: ";
  SHORT  pn1cnt = 0;
  SHORT  pn2cnt = 0;
  SHORT  pk1cnt = 0;
  SHORT  pk2cnt = 0;

  T_ACI_RETURN ret = qAT_PercentPVRF((T_ACI_CMD_SRC)srcId, &pn1cnt, &pn2cnt, &pk1cnt, &pk2cnt, &ps1, &ps2);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf(g_sa,"%s%d, %d, %d, %d, %d, %d", me, pn1cnt, pn2cnt, pk1cnt, pk2cnt, ps1, ps2);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
  }
#endif /* no FF_ATI_BAT */
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE : setatPercentCUST                   |
+--------------------------------------------------------------------+

  PURPOSE : %CUST command ( Set Customisation Mode Function )
*/

GLOBAL T_ATI_RSLT setatPercentCUST ( char *cl, UBYTE srcId ) /* Function protype is T_ATI_RSLT not T_ACI_RETURN so it is changed */
{
  TRACE_FUNCTION( "setatPercentCUST()" );

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cust cust;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CUST;
    cmd.params.ptr_set_percent_cust=&cust;

    switch (*cl)
    {
      case '0':
        cust.mode=BAT_P_CUST_MODE_NORMAL;
        break;

      case '1':
        cust.mode=BAT_P_CUST_MODE_BEHAVIOUR_1;
        break;

      default:
        cmdCmeError(CME_ERR_OpNotAllow);
        return(ATI_FAIL);
    }
    bat_send(ati_bat_get_client(srcId), &cmd);

    return(ATI_EXCT);
  }
#else /* no FF_ATI_BAT */

  switch( *cl )
  {
    case '0':
      if (sAT_PercentCUST((T_ACI_CMD_SRC)srcId, CUST_NORMAL_BEHAVIOUR) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    case '1':
      if (sAT_PercentCUST ((T_ACI_CMD_SRC)srcId, CUST_MODE_BEHAVIOUR_1) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
   }

  return (ATI_CMPL);

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCUST      |
+--------------------------------------------------------------------+

  PURPOSE : %CUST command (Customisaton Mode)
*/
GLOBAL T_ATI_RSLT queatPercentCUST (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPercentCUST()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CUST;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_cust = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */
  {
    T_CUST_MOD mode;
    T_ACI_RETURN ret = qAT_PercentCUST((T_ACI_CMD_SRC)srcId, &mode);

    if (ret NEQ AT_CMPL)
    {
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
    }

    sprintf(g_sa,"%%CUST: %d", mode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

    return ATI_CMPL;
  }
#endif /* no FF_ATI_BAT */
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE : setatPercentSATCC                   |
+--------------------------------------------------------------------+

  PURPOSE : %SATCC command ( Enable/Disable the Call/Short Message Control By SIM )
*/

GLOBAL T_ATI_RSLT setatPercentSATCC ( char *cl, UBYTE srcId ) /* Function protype is T_ATI_RSLT not T_ACI_RETURN so it is changed */

{
#ifdef FF_ATI_BAT
  T_BAT_cmd_send              bat_cmd;
  T_BAT_cmd_set_percent_satcc satcc;
#endif
  TRACE_FUNCTION( "setatPercentSATCC()" );

#ifdef FF_ATI_BAT
  switch( *cl )
  {
    case '0':
      satcc.mode = BAT_P_SATCC_MODE_INACTIVE;
      break;
    case '1':
      satcc.mode = BAT_P_SATCC_MODE_ACTIVE;
      break;
    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }

  bat_cmd.ctrl_params                  = BAT_CMD_SET_PERCENT_SATCC;
  bat_cmd.params.ptr_set_percent_satcc = &satcc;

  bat_send(ati_bat_get_client(srcId),&bat_cmd);

  return(ATI_EXCT);

#else
  switch( *cl )
  {
    case '0':
      if (sAT_PercentSATCC((T_ACI_CMD_SRC)srcId, SATCC_CONTROL_BY_SIM_INACTIVE) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    case '1':
      if (sAT_PercentSATCC ((T_ACI_CMD_SRC)srcId, SATCC_CONTROL_BY_SIM_ACTIVE) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
   }

  return (ATI_CMPL);

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentSATCC      |
+--------------------------------------------------------------------+

  PURPOSE : %SATCC command (Query Call/Short Message control By SIM)
*/
GLOBAL T_ATI_RSLT queatPercentSATCC (char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  T_BAT_cmd_send     bat_cmd;
  T_BAT_no_parameter satcc;
#else
  T_SAT_CC_MOD    mode;
  T_ACI_RETURN      ret = AT_FAIL;
#endif

  TRACE_FUNCTION("queatPercentSATCC()");

#ifdef FF_ATI_BAT
  bat_cmd.ctrl_params                  = BAT_CMD_QUE_PERCENT_SATCC;
  bat_cmd.params.ptr_que_percent_satcc = &satcc;

  bat_send(ati_bat_get_client(srcId),&bat_cmd);

  return(ATI_EXCT);

#else

  ret = qAT_PercentSATCC((T_ACI_CMD_SRC)srcId, &mode);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf(g_sa,"%%SATCC: %d", mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
	
#endif /*FF_ATI_BAT*/ 

  return ATI_CMPL;
} 

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  :  ACI_CMD            |
| STATE   : code                       ROUTINE :  setatPercentSIMIND |
+--------------------------------------------------------------------+

  PURPOSE : enable %SIMINS / %SIMREM indications
*/

GLOBAL T_ATI_RSLT setatPercentSIMIND(char *cl, UBYTE srcId)
{
  int mode = 1;

  TRACE_FUNCTION("setatPercentSIMIND()");

  /* parse mode */
  cl = parse (cl, "d", &mode);

  if (mode EQ 0 OR mode EQ 1)
  {
    ati_user_output_cfg[srcId].SIMIND_stat = mode;
    return (ATI_CMPL);
  }
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentSIMIND |
+--------------------------------------------------------------------+

  PURPOSE : query status of %SIMINS / %SIMREM indications
*/
GLOBAL T_ATI_RSLT queatPercentSIMIND(char *cl, UBYTE srcId)
{
  CHAR           *me  =  "%SIMIND";

  TRACE_FUNCTION("queatPercentSIMIND()");

  sprintf(g_sa,"%s: %d", me, ati_user_output_cfg[srcId].SIMIND_stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

#ifdef FF_DUAL_SIM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCFUN         |
+--------------------------------------------------------------------+

  PURPOSE : %SIM command (SIM Selection)
*/

GLOBAL T_ATI_RSLT setatPercentSIM (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret   = AT_FAIL;
  UBYTE sim_num;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentSIM()");

  /*input functionality*/
  cl = parse (cl,"d",&sim_num);
  if ( !cl )
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return(ATI_FAIL);
  }
  
  ret = sAT_PercentSIM (srcId,sim_num);
  switch (ret)
  {
    case AT_EXCT:               /*Command is executing*/
      src_params->curAtCmd    = AT_CMD_SIM;
      break;

    case AT_FAIL:
      src_params->curAtCmd    = AT_CMD_NONE;
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));
}

GLOBAL T_ATI_RSLT queatPercentSIM (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  UBYTE sim_num;

  TRACE_FUNCTION("queatPercentSIM()");

  ret = qAT_PercentSIM(srcId, &sim_num);

  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"b",&sim_num);
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
}
#endif /*FF_DUAL_SIM*/

#ifdef FF_CPHS_REL4
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCFIS   |
+--------------------------------------------------------------------+

  PURPOSE : %CFIS command (Call Forwarding Unconditional)
*/

GLOBAL T_ATI_RSLT setatPercentCFIS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  T_ACI_CFIS_MOD   cfisMode  = CFIS_MOD_NotPresent;
  UBYTE            index     = NOT_PRESENT_8BIT;
  UBYTE            mspId;
  UBYTE            cfuStat;
  CHAR             number[MAX_PHB_NUM_LEN];
  T_ACI_TOA        type;
  T_ACI_TOA        *p_type;
  SHORT            toa_oct   = 0;
  UBYTE            cc2_Id    = NOT_PRESENT_8BIT;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION( "setatPercentCFIS()" );

  /* init */
  memset( number, 0, sizeof(number));

  cl = parse (cl, "ddddsdd", &cfisMode, &index, &mspId, &cfuStat,
             (LONG)MAX_PHB_NUM_LEN, number, &toa_oct , &cc2_Id);

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if (toa_oct EQ 0)
  {
    p_type = NULL;
  }
  else
  {
    type=toa_demerge(toa_oct);
    p_type = &type;
  }

  if( cfisMode EQ CFIS_MOD_Read)
  {
    ret=qAT_PercentCFIS(srcId, index );
  }
  else
  {
    ret = sAT_PercentCFIS(srcId, cfisMode,index,mspId, cfuStat,
                          number, p_type, cc2_Id);
  }

  switch (ret)
  {
    case AT_EXCT:               /*Command is executing*/
      src_params->curAtCmd = AT_CMD_P_CFIS;
      break;

    default:
      src_params->curAtCmd = AT_CMD_NONE;
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentMWIS   |
+--------------------------------------------------------------------+

  PURPOSE : %MWIS command (Message Waiting Indication Status)
*/

GLOBAL T_ATI_RSLT setatPercentMWIS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  T_ACI_MWIS_MOD   mwisMode  = MWIS_MOD_Invalid;
  UBYTE            mspId     = NOT_PRESENT_8BIT;
  T_ACI_MWIS_MWI   mwis;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION( "setatPercentMWIS()" );

  memset(&mwis, 0, sizeof(T_ACI_MWIS_MWI));
  cl = parse (cl, "ddddddd", &mwisMode, &mspId, &mwis.mwiStat,
              &mwis.mwis_count_voice, &mwis.mwis_count_fax,
              &mwis.mwis_count_email,&mwis.mwis_count_other);

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if( mwisMode EQ MWIS_MOD_Read)
  {
    ret = qAT_PercentMWIS(srcId, mspId );
  }
  else
  {
    ret = sAT_PercentMWIS(srcId, mwisMode,mspId, &mwis);
  }

  switch (ret)
  {
    case AT_EXCT:               /*Command is executing*/
      src_params->curAtCmd = AT_CMD_P_MWIS;
      break;

    default:
      src_params->curAtCmd = AT_CMD_NONE;
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentMWI    |
+--------------------------------------------------------------------+

  PURPOSE : %MWI command (Message Waiting Indication)
*/

GLOBAL T_ATI_RSLT setatPercentMWI(char *cl, UBYTE srcId)
{
  SHORT mode = ACI_NumParmNotPresent;

  TRACE_FUNCTION("setatPercentMWI()");

  cl = parse(cl,"r",&mode);
  if(!cl OR mode > 1 OR mode < 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ati_user_output_cfg[srcId].MWI_stat=(UBYTE)mode;
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentMWI    |
+--------------------------------------------------------------------+

  PURPOSE : %MWI command (Message Waiting Indication)
*/

GLOBAL T_ATI_RSLT queatPercentMWI(char *cl, UBYTE srcId)
{
  UBYTE   mode;

  TRACE_FUNCTION("queatPercentMWI()");

  mode = ati_user_output_cfg[srcId].MWI_stat;
  resp_disp(srcId, cl,"b",&mode);
  return (ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentMBI    |
+--------------------------------------------------------------------+

  PURPOSE : %MBI command (Mailbox Identifier)
*/

GLOBAL T_ATI_RSLT setatPercentMBI (char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  T_ACI_MBN_MODE   mbiMode   = MBN_Mode_Invalid;
  UBYTE            index     = NOT_PRESENT_8BIT;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION( "setatPercentMBI()" );

  cl = parse (cl, "dd", &mbiMode, &index);

  if(!cl OR (mbiMode NEQ MBN_Mode_Read))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = qAT_PercentMBI(srcId, index );

  switch (ret)
  {
    case AT_EXCT:               /*Command is executing*/
      src_params->curAtCmd = AT_CMD_P_MBI;
      break;

    default:
      src_params->curAtCmd = AT_CMD_NONE;
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentMBDN   |
+--------------------------------------------------------------------+

  PURPOSE : %MBDN command (Mailbox Dialling Number)
*/

GLOBAL T_ATI_RSLT setatPercentMBDN (char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  T_ACI_MBN_MODE   mbdnMode  = MBN_Mode_Invalid;
  UBYTE            index     = 0;
  CHAR             number[MAX_MB_NUM_LEN];
  CHAR    alpha_id[MAX_ALPHA_LEN] = {0x00};
  USHORT  len_alpha_id = 0;   
  USHORT  lenCvtd      = 0;         /* holds length of converted text */
  T_ACI_PB_TEXT *p_text = NULL;     /* holds pointer to converted text */
  T_ACI_PB_TEXT  cvtdText;          /* holds text converted to SIM alpha */
  T_ACI_TOA        type;
  T_ACI_TOA        *p_type;
  CHAR  *p_number;
  SHORT            toa_oct   = 0;
  UBYTE            cc2_Id    = NOT_PRESENT_8BIT;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION( "setatPercentMBDN()" );

  /* init */
  memset( number, 0, sizeof(number));

  cl = parse (cl, "ddsddz", &mbdnMode, &index,(LONG)MAX_MB_NUM_LEN, number,
              &toa_oct , &cc2_Id, (LONG)MAX_ALPHA_LEN,
              strlen(cl), cl, &len_alpha_id, alpha_id);

  if(!cl OR (mbdnMode < 0 OR mbdnMode > 2) OR (!index))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  /* translate alpha text to SIM */  
  if ( len_alpha_id NEQ 0 )
  {
    utl_chsetToSim ((UBYTE*)alpha_id, len_alpha_id, (UBYTE*)cvtdText.data,
                     &lenCvtd, GSM_ALPHA_Def);
  }

  if (toa_oct EQ 0)
  {
    p_type = NULL;
  }
  else
  {
    type=toa_demerge(toa_oct);
    p_type = &type;
  }

  /* when the text is empty p_text to NULL */
  if (lenCvtd EQ 0 )
  {
    p_text = NULL;
  }
  else
  {
    p_text = &cvtdText;
    cvtdText.len = (UBYTE)lenCvtd;
  }

  /* set pointer for number */
  if (number[0] EQ '\0') /* The number given is empty */
  {
    p_number = NULL;
  }
  else
  {
    p_number = number;
  }

  if( mbdnMode EQ MBN_Mode_Read)
  {
    ret = qAT_PercentMBDN(srcId, index );
  }
  else
  {
    ret = sAT_PercentMBDN(srcId, mbdnMode,index,
                          p_number, p_type, cc2_Id, p_text);
  }

  switch (ret)
  {
    case AT_EXCT:               /*Command is executing*/
      src_params->curAtCmd = AT_CMD_P_MBDN;
      break;

    default:
      src_params->curAtCmd = AT_CMD_NONE;
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));
}
#endif /* FF_CPHS_REL4 */

#endif /* ATI_SIM_C */

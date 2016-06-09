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
|  Purpose :  AT Command Interpreter: Mobility Management related functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_MM_C
#define ATI_MM_C

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

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "psa.h"
#include "cmh.h"
#include "psa_sim.h"
#include "cmh_sim.h"
#include "psa_mm.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

EXTERN T_ACI_CFUN_FUN CFUNfun;
EXTERN T_CIEV_SIGNAL_BUFFER   asCievSignalBuf;
EXTERN T_CIEV_SMSFULL_BUFFER  asCievSmsFullBuf;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCops         |
+--------------------------------------------------------------------+

  PURPOSE : +COPS command (operator selection)
*/

GLOBAL T_ATI_RSLT setatPlusCOPS (char *cl, UBYTE srcId)
{
  CHAR            op[MAX_ALPHA_OPER_LEN] = {0};
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCops()");

  if (CFUNfun NEQ CFUN_FUN_Full)
  {
    TRACE_EVENT_P1("+COPS error: cannot proceed Mobile has no Full CFUN: %d", CFUNfun);
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  cl = parse (cl,"dds",&mode,&format,(LONG)(MAX_ALPHA_OPER_LEN-1),op);     /*split parameter string in int,int,str*/
  if (!cl OR ((mode EQ COPS_MOD_Man OR mode EQ COPS_MOD_Both)
                   AND op[0] EQ 0))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cops cops;
    UBYTE len;

    len=strlen(op);

    /*
    *   If the operator name is too long we can't really do anything
    *   meaningful, so generate an error and give up.
    */
    if (len>BAT_MAX_COPS_OPER_LEN)
    {
      cmdCmeError(CME_ERR_Unknown);
      return(ATI_FAIL);
    }

    cmd.ctrl_params = BAT_CMD_SET_PLUS_COPS;
    cmd.params.ptr_set_plus_cops=&cops;

    /*
    *   This relies on T_ACI_COPS_MOD and T_BAT_plus_cops_mode being
    *   equivalent.
    */
    cops.mode=(T_BAT_plus_cops_mode)mode;

    /*
    *   This relies on T_ACI_COPS_FRMT and T_BAT_plus_cops_format being
    *   equivalent.
    */
    cops.format=(T_BAT_plus_cops_format)format;
    
    cops.c_oper=(U8)len;

    if (len)
    {
      memcpy(cops.oper,op,BAT_MAX_COPS_OPER_LEN);
      cops.v_oper=TRUE;
    }
    else
    {
      cops.v_oper=FALSE;
    }
    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd = AT_CMD_COPS;
    return ATI_EXCT;
  }
#else

  ret = sAT_PlusCOPS ((T_ACI_CMD_SRC)srcId,mode,format,op);
  switch (ret)
  {
  case (AT_CMPL):                         /*operation completed*/
    break;
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_COPS;
    break;
  default:
    cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
    break;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /*FF_ATI_BAT*/
}

#ifdef FF_ATI_BAT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)      MODULE  : ACI_BAT                    |
| STATE   : code                ROUTINE : aci_bat_HandleTestCOPS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
T_ATI_RSLT atiHandleTestCOPS(UBYTE srcId)
{
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  cmd.ctrl_params = BAT_CMD_TST_PERCENT_COPS;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_plus_cops = &dummy;
  
  TRACE_FUNCTION("aci_bat_HandleTestCOPS() calls bat_send() <=== as APPLICATION");

#ifndef FF_DUAL_SIM
  if (CFUNfun NEQ CFUN_FUN_Full)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
#endif /*FF_DUAL_SIM*/

  bat_send(ati_bat_get_client(srcId), &cmd);
  return ATI_EXCT; /* executing, because response is passed by callback function */
}

#else /* no FF_ATI_BAT */

GLOBAL T_ATI_RSLT atiHandleTestCOPS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret      = AT_FAIL;
  SHORT           startIdx = 0,
                  lastIdx  = 0,
                  i;
  T_ACI_COPS_LST  operLst;
  BOOL            END_OF_LIST = FALSE;


  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  char* outputCommand;
  

  TRACE_FUNCTION("atiHandleTestCOPS()");

#ifndef FF_DUAL_SIM
  if (CFUNfun NEQ CFUN_FUN_Full)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
#endif /*FF_DUAL_SIM*/
  do
  {
    startIdx += lastIdx;
    switch (src_params->curAtCmd)
    {
        case AT_CMD_COPS:
           ret = tAT_PlusCOPS((T_ACI_CMD_SRC)srcId,startIdx,&lastIdx,&operLst[0]);
        break;
        case AT_CMD_P_COPS:
           ret = tAT_PercentCOPS((T_ACI_CMD_SRC)srcId,startIdx,&lastIdx,&operLst[0]);
        break;
        default:
           cmdCmeError(CME_ERR_Unknown);
        return ATI_FAIL;
    }
    if (ret EQ AT_CMPL)
    {
      for(i=0;i<lastIdx;i++)
      {
        if ((operLst[i].status < 0) AND (*operLst[i].numOper EQ 0))
        {
          END_OF_LIST = TRUE;
          break;
        }

        /* What COPS command is active? */
        switch (src_params->curAtCmd)
        {
        case AT_CMD_COPS:
           outputCommand = "+COPS";
        break;
        case AT_CMD_P_COPS:
           outputCommand ="%COPS";
        break;
        default:
           cmdCmeError(CME_ERR_Unknown);
        return ATI_FAIL;
        }
             
        sprintf(g_sa,"%s: %d,\"%s\",\"%s\",%s",outputCommand,
                operLst[i].status,operLst[i].longOper,
                operLst[i].shortOper,operLst[i].numOper);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

      }
    }

    switch(ret)
    {
      case AT_EXCT:
        return ATI_EXCT;
        
      case AT_FAIL:
        cmdCmeError(CME_ERR_Unknown);
        return ATI_FAIL;
        
      case AT_BUSY:
        return ATI_BUSY;

      default:
        break;/* Does nothing in this case */
    }

  }
  while(!END_OF_LIST);

  return ATI_CMPL;
}

#endif /* no FF_ATI_BAT */


GLOBAL T_ATI_RSLT tesatPlusCOPS (char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  TRACE_FUNCTION("tesatPlusCOPS() calls bat_send() <=== as APPLICATION");
  return atiHandleTestCOPS(srcId);

#else /* FF_ATI_BAT */
  TRACE_FUNCTION("tesatPlusCOPS()");
  return atiHandleTestCOPS(cl,srcId);
#endif /* FF_ATI_BAT */

}

GLOBAL T_ATI_RSLT queatPlusCOPS (char *cl, UBYTE srcId)
{
  CHAR            op[MAX_ALPHA_OPER_LEN] = {0};
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;

  TRACE_FUNCTION("queatPlusCops()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_COPS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cops = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PlusCOPS((T_ACI_CMD_SRC)srcId,&mode,&format,op);
  if (ret EQ AT_CMPL)
  {
    if (*op)
      sprintf(g_sa,"+COPS: %d,%d,\"%s\"",mode,format,op);
    else
      sprintf(g_sa,"+COPS: %d",mode);
    
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCOPS         |
+--------------------------------------------------------------------+

  PURPOSE : %COPS command (operator selection)
*/

GLOBAL T_ATI_RSLT setatPercentCOPS (char *cl, UBYTE srcId)
{
  CHAR            op[MAX_ALPHA_OPER_LEN] = {0};
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  if (CFUNfun NEQ CFUN_FUN_Full)
  {
    TRACE_EVENT_P1("%COPS error: cannot proceed Mobile has no Full CFUN: %d", CFUNfun);
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  cl =  parse (cl,"dds",&mode,&format,(LONG)(MAX_ALPHA_OPER_LEN-1),op);    /*split parameter string in int,int,str*/
  if (!cl) /*For %COPS, Manual selection without Operator parameter is allowed */
  {    
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cops my_bat_set_percent_cops;
    UBYTE OperLength = strlen(op);
    
    TRACE_FUNCTION("setatPercentCOPS() calls bat_send() <=== as APPLICATION");

    memset(&my_bat_set_percent_cops, sizeof(my_bat_set_percent_cops), FALSE);
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_COPS;
    cmd.params.ptr_set_percent_cops = &my_bat_set_percent_cops;

      my_bat_set_percent_cops.mode = (T_BAT_VAL_percent_cops_mode)mode;
      my_bat_set_percent_cops.format = (T_BAT_VAL_percent_cops_format)format;
    
    if (OperLength NEQ 0 AND
        OperLength <= BAT_MAX_COPS_LONG_OPER_LEN)
    {
      memcpy(&my_bat_set_percent_cops.oper, op, BAT_MAX_COPS_LONG_OPER_LEN);
    }

     src_params->curAtCmd = AT_CMD_P_COPS;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
  
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("setatPercentCOPS()");
  ret = sAT_PercentCOPS ((T_ACI_CMD_SRC)srcId,mode,format,op);
  switch (ret)
  {
  case (AT_CMPL):                         /*operation completed*/
    break;
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_P_COPS;
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
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : tesatPercentCOPS         |
+--------------------------------------------------------------------+

  PURPOSE : %COPS command (operator test)
*/


GLOBAL T_ATI_RSLT tesatPercentCOPS (char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  TRACE_FUNCTION("tesatPercentCOPS() calls bat_send() <=== as APPLICATION");
  return atiHandleTestCOPS(srcId);

#else /* FF_ATI_BAT */
  TRACE_FUNCTION("tesatPercentCOPS()");
  return atiHandleTestCOPS(cl,srcId);
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCOPS         |
+--------------------------------------------------------------------+

  PURPOSE : %COPS command (operator query)
*/


GLOBAL T_ATI_RSLT queatPercentCOPS (char *cl, UBYTE srcId)
{
  CHAR            op[MAX_ALPHA_OPER_LEN] = {0};
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_COPS_MOD  mode = COPS_MOD_NotPresent;
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  T_ACI_COPS_SVST svrStatus = COPS_SVST_NotPresent;

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPercentCOPS() calls bat_send() <=== as APPLICATION");
    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_COPS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_cops = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("queatPercentCOPS()");

  ret = qAT_PercentCOPS((T_ACI_CMD_SRC)srcId,&mode,&format,&svrStatus,op);
  if (ret EQ AT_CMPL)
  {
    /* Added an empty comma and the service Status parameter */
    sprintf(g_sa,"%s: %d,%d,\"%s\",,%d","%COPS",mode,format,op,svrStatus);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);         /*command failed*/
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCREG   |
+--------------------------------------------------------------------+

  PURPOSE : %CREG command (Network registration)
*/

GLOBAL T_ATI_RSLT setatPercentCREG (char *cl, UBYTE srcId)
{
  T_ATI_CREG_MOD mode=CREG_MOD_OFF;

  TRACE_FUNCTION("setatPercentCREG()");

  cl=parse(cl,"d",&mode);

  switch(mode)
  {
    case CREG_MOD_OFF:
    case CREG_MOD_ON:
    case CREG_MOD_LOC_INF_ON:
      /* user wants to set mode: reset CREG related ATI parameters */
      ati_creg_init(srcId, PercentCREG_CMD);
      ati_user_output_cfg[srcId].percent_creg.mod_lac_cid.pres_mode = mode;
      return ATI_CMPL;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : queatPercentCREG   |
+--------------------------------------------------------------------+

  PURPOSE : %CREG command
*/
GLOBAL T_ATI_RSLT queatPercentCREG (char *cl, UBYTE srcId)
{

#ifndef FF_ATI_BAT
  T_ACI_CREG_STAT       stat;
  USHORT                lac, cid;
  T_ATI_CREG_MOD        mode;
  T_ACI_P_CREG_GPRS_IND gprs_ind;
  short                 pos;
  U8                    rt; 
#endif

  TRACE_FUNCTION("queatPercentCREG()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CREG;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_creg = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */
  
  mode = ati_user_output_cfg[srcId].percent_creg.mod_lac_cid.pres_mode;
  
  if (qAT_PercentCREG((T_ACI_CMD_SRC)srcId, &stat, &lac, &cid, &gprs_ind, &rt) NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }

  pos = sprintf(g_sa, "%s: %d,%d", "%CREG", mode, stat);
  
  if ( (mode EQ CREG_MOD_LOC_INF_ON)  AND
      (stat EQ CREG_STAT_Reg  OR  stat EQ CREG_STAT_Roam) )
    pos += sprintf(g_sa+pos, ",\"%04X\",\"%04X\"", lac, cid);
  else
    pos += sprintf(g_sa+pos, ",,");
  
  sprintf(g_sa+pos,",%d", gprs_ind);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCREG      |
+--------------------------------------------------------------------+

  PURPOSE : +CREG command (Network registration)
*/

GLOBAL T_ATI_RSLT setatPlusCREG (char *cl, UBYTE srcId)
{
  T_ATI_CREG_MOD mode=CREG_MOD_OFF;

  TRACE_FUNCTION("setatPlusCREG()");

  cl=parse(cl,"d",&mode);
  
  switch(mode)
  {
    case CREG_MOD_OFF:
    case CREG_MOD_ON:
    case CREG_MOD_LOC_INF_ON:
      /* user wants to set mode: reset CREG related ATI parameters */
      ati_creg_init(srcId, CREG_CMD);
      ati_user_output_cfg[srcId].creg.mod_lac_cid.pres_mode = mode;
      return ATI_CMPL;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
  }
}

GLOBAL T_ATI_RSLT queatPlusCREG (char *cl, UBYTE srcId)
{
  T_ACI_CREG_STAT stat;
  USHORT          lac, cid;
  T_ATI_CREG_MOD  mode;
  short           pos;

  TRACE_FUNCTION("queatPlusCREG()");

  mode = ati_user_output_cfg[srcId].creg.mod_lac_cid.pres_mode;

  if (qAT_PlusCREG((T_ACI_CMD_SRC)srcId, &stat, &lac, &cid) NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }

  pos = sprintf(g_sa, "+CREG: %d,%d", mode, stat);

  if (mode EQ CREG_MOD_LOC_INF_ON)
  {
    if (stat EQ CREG_STAT_Reg  OR  stat EQ CREG_STAT_Roam)
      sprintf(g_sa+pos, ",\"%04X\",\"%04X\"", lac, cid);
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCopn         |
+--------------------------------------------------------------------+

  PURPOSE : +COPN command (read operator names)
*/

GLOBAL T_ATI_RSLT setatPlusCOPN (char *cl, UBYTE srcId)
{
  SHORT startIdx, lastIdx;
  T_ACI_COPN_LST operLst;
  UBYTE idx;
  BOOL  loop;
  BOOL ret;

  TRACE_FUNCTION("setatPlusCOPN()");

  /* read out the PCM list */
  startIdx = 0;
  loop     = TRUE;

  do
  {
    if ( qAT_PlusCOPN((T_ACI_CMD_SRC)srcId, COPN_LID_Pcm, startIdx,
                     &lastIdx, operLst) EQ AT_FAIL )
      break;

    for( idx=0; idx < MAX_OPER; idx++ )
    {
      if ( operLst[idx].numOper[0] EQ 0x0 )
      {
        loop = FALSE;
        break;
      }

      sprintf(g_sa,"+COPN: \"%s\",\"%s\"",operLst[idx].numOper,
                                        operLst[idx].alphaOper );
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
    startIdx = lastIdx+1;
  }
  while( loop );

  /* read out the constant list */
  startIdx = 0;

  ret = cmd_readCOPNBlock(startIdx,srcId);
 
  if (ret EQ TRUE)
   return ATI_CMPL;
  else 
   return ATI_EXCT;
 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCOPN         |
+--------------------------------------------------------------------+

  PURPOSE : %COPN command (get  MCC and MNC, long name or short name given one of the them)
*/

GLOBAL T_ATI_RSLT setatPercentCOPN (char *cl, UBYTE srcId)
{
  T_ACI_COPS_FRMT format = COPS_FRMT_NotPresent;
  CHAR            op[MAX_ALPHA_OPER_LEN] = {0};
  T_ACI_OPER_NTRY * oper_ntry;
  USHORT i;

  TRACE_FUNCTION ("setatPercentCOPN()");

  cl = parse (cl,"ds", &format, (LONG)MAX_ALPHA_OPER_LEN, op); 

  if ((!cl) OR (format < COPS_FRMT_Long) OR (format > COPS_FRMT_Numeric) OR *op EQ '\0')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  if (format EQ COPS_FRMT_Numeric)
  {
    if ((strlen(op) NEQ (SIZE_MNC + SIZE_MCC -1)) AND 
        (strlen(op) NEQ (SIZE_MNC + SIZE_MCC)))
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
    }

    for (i = 0; *(op+i) NEQ '\0'; i ++)
    {
      if (!((*(op+i) >= '0')AND(*(op+i) <= '9')))
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return ATI_FAIL;
      }
    }
  }

  ACI_MALLOC (oper_ntry, sizeof (T_ACI_OPER_NTRY));

  if (qAT_PercentCOPN( (T_ACI_CMD_SRC)srcId, format, op, oper_ntry) EQ AT_FAIL)
  {
    sprintf (g_sa, "%s", "%COPN: ");
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE (oper_ntry);
    return ATI_CMPL; 
  } 

  if ((oper_ntry->mnc & 0x00F) EQ 0xF)
  {
    sprintf (g_sa, "%s\"%s\",\"%s\",\"%03X%02X\",%d", "%COPN: ", oper_ntry->longName, oper_ntry->shrtName,
             oper_ntry->mcc, oper_ntry->mnc >> 4, oper_ntry->source);
  }
  else
  {
    sprintf (g_sa, "%s\"%s\",\"%s\",\"%03X%03X\",%d", "%COPN: ", oper_ntry->longName, oper_ntry->shrtName,
             oper_ntry->mcc, oper_ntry->mnc, oper_ntry->source);
  }
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  ACI_MFREE (oper_ntry);
  return ATI_CMPL;

 
}

#ifdef TI_PS_FF_AT_CMD_WS46
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusWS46         |
+--------------------------------------------------------------------+

  PURPOSE : +WS46 command (select wireless network)
*/

GLOBAL T_ATI_RSLT setatPlusWS46 (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_WS46_MOD mode=WS46_MOD_NotPresent;

  TRACE_FUNCTION("setatPLusWS46()");

  cl = parse(cl,"d",&mode);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_ws46 ws46;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_WS46;
    cmd.params.ptr_set_plus_ws46=&ws46;

    ws46.n=(T_BAT_plus_ws46_n)mode;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 

#else /* no FF_ATI_BAT */
 
  ret=sAT_PlusWS46((T_ACI_CMD_SRC)srcId,mode);

  if (ret EQ AT_CMPL)
  {
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusWS46 (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_WS46_MOD mode=WS46_MOD_NotPresent;

  TRACE_FUNCTION("queatPLusWS46()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_WS46;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_ws46 = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */
 
  ret = qAT_PlusWS46((T_ACI_CMD_SRC)srcId,&mode);

  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"e",&mode);
    return ATI_CMPL;
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}
#endif /* TI_PS_FF_WS46 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)             MODULE  :                     |
| STATE   : code                       ROUTINE :  setatPercentCHPL   |
+--------------------------------------------------------------------+

  PURPOSE : show list of all available entries in the SIM field EF_HPLMNwAcT.
            
            Command            Possible Response
            %CHPL=oprFrmt      %CHPL: <opr>
            %CHPL=?            %CHPL: (list of supported <oprFrmt>s)

            <oprFrmt>:
              0    long format alphanumeric <opr>
              1    short format alphanumeric <opr>
              2    numeric <opr>

            <opr>: string type; <oprFrmt> indicates if the format is
              alphanumeric or numeric.
              Long alphanumeric can be up to 16 characters.
              Short alphanumeric can be up to 8 characters.

            Numeric format is the GSM LAI number, which consists of:
              3 BCD country code.
              2 BCD network code.

	The returned <opr> string shall not be in BCD format, but
	in IRA characters converted from BCD. Hence the number
	will have the structure (CC digit 3)(CC digit 2)(CC digit 1)
	(NW digit 2)(NW digit 1).

*/

GLOBAL T_ATI_RSLT setatPercentCHPL( char *cl, UBYTE srcId )
{
  T_ACI_CHPL_FRMT   format          = CHPL_FRMT_NotPresent;

#ifndef FF_ATI_BAT
  T_ACI_OPER_NTRY   oper;
#endif

  T_ACI_RETURN      ret             = AT_FAIL;
  
  TRACE_FUNCTION( "setatPercentCHPL()" );

  if (CFUNfun NEQ CFUN_FUN_Full)
  {
    TRACE_EVENT_P1("+COPS error: cannot proceed Mobile has no Full CFUN: %d", CFUNfun);
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  /* parse mode */
  cl = parse (cl, "d", &format);
  if ((!cl) OR (format > CHPL_FRMT_Numeric) OR (format < CHPL_FRMT_NotPresent))
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_chpl chpl;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CHPL;
    cmd.params.ptr_set_percent_chpl=&chpl;

    /*
    *   This relies on T_ACI_CHPL_FRMT and T_BAT_percent_chpl_oprfrmt being
    *   identical.
    */
    chpl.oprfrmt=(T_BAT_percent_chpl_oprfrmt)format;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 

#else /* no FF_ATI_BAT */

  ret = sAT_PercentCHPL ((T_ACI_CMD_SRC)srcId, &oper);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  switch (format)
  {
    case CHPL_FRMT_Long :
      sprintf(g_sa, "%s: %s", "%CHPL", oper.longName);
      break;
  
    case CHPL_FRMT_Short :
      sprintf(g_sa, "%s: %s", "%CHPL", oper.shrtName);
      break;
      
    case CHPL_FRMT_Numeric :
      if ((oper.mnc & 0x00F) EQ 0xF)
        /*lint -e{702} */
        sprintf(g_sa, "%s: %03x,%02x", "%CHPL", oper.mcc, oper.mnc >> 4);
      else
        sprintf(g_sa, "%s: %03x,%03x", "%CHPL", oper.mcc, oper.mnc);
      break;
    
    default :
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);

  return(ATI_CMPL);

#endif /* no FF_ATI_BAT */
}
  
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCTZR      |
+--------------------------------------------------------------------+

  PURPOSE : %CTZR command (Timezone Mode)
*/
GLOBAL T_ATI_RSLT setatPlusCTZR (char *cl, UBYTE srcId)
{
  T_ACI_CTZR_MODE mode         = CTZR_MODE_OFF;    /* CTZR  */
  T_ACI_RETURN    ret          = AT_FAIL;
  
  TRACE_FUNCTION( "setatPlusCTZR()" );

  /* parse mode */
  cl = parse (cl, "d", &mode);
  if ((!cl) OR (mode > CTZR_MODE_ON) OR (mode < CTZR_MODE_OFF))/*lint !e685 (Warning -- 685: Relational operator '>' always evaluates to 'false ) */
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PlusCTZR ((T_ACI_CMD_SRC)srcId, mode);

  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      break;
  }
  return (map_aci_2_ati_rslt(ret));

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCTZR      |
+--------------------------------------------------------------------+

  PURPOSE : %CTZR command (Timezone Mode)
*/
GLOBAL T_ATI_RSLT queatPlusCTZR (char *cl, UBYTE srcId)
{
  T_ACI_CTZR_MODE mode;
  T_ACI_RETURN      ret = AT_FAIL;

  TRACE_FUNCTION("queatPlusCTZR()");

  ret = qAT_PlusCTZR((T_ACI_CMD_SRC)srcId, &mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf(g_sa,"+CTZR: %d", mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCTZU      |
+--------------------------------------------------------------------+

  PURPOSE : +CTZU command (Timezone Update)
*/
GLOBAL T_ATI_RSLT setatPlusCTZU (char *cl, UBYTE srcId)
{
  T_ACI_CTZU_MODE mode         = CTZU_MODE_OFF;    /* CTZU  */
  T_ACI_RETURN    ret          = AT_FAIL;
  
  TRACE_FUNCTION( "setatPlusCTZU()" );

  /* parse mode */
  cl = parse (cl, "d", &mode);
  if ((!cl) OR  (mode > CTZU_MODE_ON) OR (mode < CTZU_MODE_OFF))/*lint !e685 (Warning -- 685: Relational operator '>' always evaluates to 'false ) */
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_ctzu ctzu;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CTZU;
    cmd.params.ptr_set_plus_ctzu=&ctzu;

    /*
    *   This relies on T_ACI_CTZU_MODE and T_BAT_plus_ctzu_onoff being
    *   identical.
    */
    ctzu.onoff=(T_BAT_plus_ctzu_onoff)mode;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 

#else /* no FF_ATI_BAT */

  ret = sAT_PlusCTZU ((T_ACI_CMD_SRC)srcId, mode);

  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
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
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCTZV   |
+--------------------------------------------------------------------+

  PURPOSE : %CTZV command (time and date report)
*/
GLOBAL T_ATI_RSLT setatPercentCTZV (char *cl, UBYTE srcId)
{
  T_ACI_PCTZV_MODE mode         = PCTZV_MODE_OFF;    /* %CTZV  */
  T_ACI_RETURN    ret           = AT_FAIL;
  
  TRACE_FUNCTION( "setatPercentCTZV()" );

  /* parse mode */
  cl = parse (cl, "d", &mode);
  if ((!cl) OR (mode > PCTZV_MODE_ON) OR  (mode < PCTZV_MODE_OFF))/*lint !e685 (Warning -- 685: Relational operator '>' always evaluates to 'false ) */
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PercentCTZV ((T_ACI_CMD_SRC)srcId, mode);

  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      break;
  }
  return (map_aci_2_ati_rslt(ret));

}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCNIV   |
+--------------------------------------------------------------------+

  PURPOSE : %CNIV command (network identity report)
*/
GLOBAL T_ATI_RSLT setatPercentCNIV (char *cl, UBYTE srcId)
{
  T_ACI_CNIV_MODE mode         = CNIV_MODE_OFF;    /* CNIV  */
  T_ACI_RETURN    ret          = AT_FAIL;

  TRACE_FUNCTION( "setatPercentCNIV()" );

  /* parse mode */
  cl = parse (cl, "d", &mode);
  if ((!cl) OR (mode > CNIV_MODE_ON) OR  (mode < CNIV_MODE_OFF))/*lint !e685 (Warning -- 685: Relational operator '>' always evaluates to 'false ) */
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  
#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cniv my_bat_set_percent_cniv;    
        
    memset(&my_bat_set_percent_cniv,  0 , sizeof(my_bat_set_percent_cniv));
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CNIV;
    cmd.params.ptr_set_percent_cniv = &my_bat_set_percent_cniv;
    
    my_bat_set_percent_cniv.mode= (T_BAT_VAL_percent_cniv_mode)mode;
    
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else
  {
    ret = sAT_PercentCNIV ((T_ACI_CMD_SRC)srcId, mode);
    
    switch (ret)
    {
    case (AT_CMPL):                         /*operation completed*/
      break;
      
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      break;
    }
    return (map_aci_2_ati_rslt(ret));
  }
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCTZU      |
+--------------------------------------------------------------------+

  PURPOSE : +CTZU query command (Timezone update)
*/
GLOBAL T_ATI_RSLT queatPlusCTZU (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_CTZU_MODE mode;
  T_ACI_RETURN      ret = AT_FAIL;
#endif

  TRACE_FUNCTION("queatPlusCTZU()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CTZU;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_ctzu = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PlusCTZU((T_ACI_CMD_SRC)srcId, &mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf(g_sa,"+CTZU: %d", mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCTZV   |
+--------------------------------------------------------------------+

  PURPOSE : %CTZV command (time and date report)
*/
GLOBAL T_ATI_RSLT queatPercentCTZV (char *cl, UBYTE srcId)
{
  T_ACI_PCTZV_MODE mode;
  T_ACI_RETURN      ret = AT_FAIL;

  TRACE_FUNCTION("queatPercentCTZV()");

  ret = qAT_PercentCTZV((T_ACI_CMD_SRC)srcId, &mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf(g_sa,"%s: %d","%CTZV", mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return ATI_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCNIV   |
+--------------------------------------------------------------------+

  PURPOSE : %CNIV command (time and date report)
*/
GLOBAL T_ATI_RSLT queatPercentCNIV (char *cl, UBYTE srcId)
{
  
  TRACE_FUNCTION("queatPercentCNIV()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;
    
    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CNIV;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_cniv= &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);

    return(ATI_EXCT);
  } 
#else
{
  T_ACI_CNIV_MODE mode;
  T_ACI_RETURN      ret = AT_FAIL;

  ret = qAT_PercentCNIV((T_ACI_CMD_SRC)srcId, &mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  sprintf(g_sa,"%s: %d","%CNIV", mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return ATI_CMPL;
}
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentTREG   |
+--------------------------------------------------------------------+

  PURPOSE : %CWUP command (force power campaign)
*/

GLOBAL T_ATI_RSLT setatPercentCWUP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret          = AT_FAIL;
  T_ACI_CWUP_TYPE type;
  
  TRACE_FUNCTION( "setatPercentCWUP()" );

  /* parse mode */
  cl = parse (cl, "d", &type);
  if ((!cl) OR (type NEQ CWUP_TYPE_RR))
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cwup cwup;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CWUP;
    cmd.params.ptr_set_percent_cwup=&cwup;

    /*
    *   This relies on T_ACI_CWUP_TYPE and T_BAT_percent_cwup_type being
    *   identical.
    */
    cwup.type=(T_BAT_percent_cwup_type)type;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 

#else /* no FF_ATI_BAT */

  ret = sAT_PercentCWUP ((T_ACI_CMD_SRC)srcId, type);

  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
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
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCIND         |
+--------------------------------------------------------------------+

  PURPOSE : +CIND command
            (configures the signal strength and SMS full indication)
*/
GLOBAL T_ATI_RSLT setatPlusCIND (char *cl, UBYTE srcId)
{
  T_ACI_RETURN tRet = AT_FAIL;
  T_ACI_MM_CIND_VAL_TYPE sCindSettings;

  TRACE_FUNCTION("setatPLusCIND()");

  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  sCindSettings.sCindSignalParam   =  CIND_SIGNAL_INDICATOR_LVL0;
  sCindSettings.sCindSmsFullParam =  CIND_SMSFULL_INDICATOR_MEMAVAIL;

  cl = parse(cl,"dd", &sCindSettings.sCindSignalParam,
                      &sCindSettings.sCindSmsFullParam);

  if ((sCindSettings.sCindSignalParam < CIND_SIGNAL_INDICATOR_LVL0 OR
      sCindSettings.sCindSignalParam > CIND_SIGNAL_INDICATOR_LVL5) OR
     (sCindSettings.sCindSmsFullParam < CIND_SMSFULL_INDICATOR_MEMAVAIL OR
      sCindSettings.sCindSmsFullParam > CIND_SMSFULL_INDICATOR_MEMFULL))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cind cind;

    /*
    *   Without this we will lose track of the source ID by the time the
    *   response arrives.
    */
    srcId_cb=srcId;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CIND;
    cmd.params.ptr_set_plus_cind=&cind;

    /*
    *   This relies on T_ACI_CIND_SIGNAL_TYPE being identical to
    *   T_BAT_plus_cind_signal_ctrl and T_ACI_CIND_SMSFULL_TYPE being
    *   identical to T_BAT_plus_cind_smsfull_ctrl.
    */
    cind.signal_ctrl=(T_BAT_plus_cind_signal_ctrl)sCindSettings.sCindSignalParam;
    cind.smsfull_ctrl=(T_BAT_plus_cind_smsfull_ctrl)sCindSettings.sCindSmsFullParam;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  tRet = sAT_PlusCIND( (T_ACI_CMD_SRC)srcId, sCindSettings.sCindSignalParam,
                              sCindSettings.sCindSmsFullParam );

  if (tRet EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCIND      |
+--------------------------------------------------------------------+

  PURPOSE : +CIND query command
            (query the configured unsolicited signal strength 
             and SMS full indication settings)
*/
GLOBAL T_ATI_RSLT queatPlusCIND ( char *cl, UBYTE srcId )
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN           sRet = AT_FAIL;
  T_ACI_MM_CIND_VAL_TYPE sCindSettings;
#endif

  TRACE_FUNCTION("queatPlusCIND()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CIND;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cind = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  sRet = qAT_PlusCIND( (T_ACI_CMD_SRC)srcId, &sCindSettings.sCindSignalParam,
                              &sCindSettings.sCindSmsFullParam );

  if ( sCindSettings.sCindSignalParam  EQ CIND_SIGNAL_INDICATOR_INVALID )  { sCindSettings.sCindSignalParam  = CIND_SIGNAL_INDICATOR_LVL0; }
  if ( sCindSettings.sCindSmsFullParam EQ CIND_SMSFULL_INDICATOR_INVALID ) { sCindSettings.sCindSmsFullParam = CIND_SMSFULL_INDICATOR_MEMAVAIL; }

  if (sRet EQ AT_CMPL)
  {
    resp_disp( srcId, cl,"ee", &sCindSettings.sCindSignalParam,
                               &sCindSettings.sCindSmsFullParam );
    return ATI_CMPL;
  }
  else
  { /* error case */
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMER         |
+--------------------------------------------------------------------+

  PURPOSE : +CMER command
            (configures the kind of indication handling toward terminal)
*/
GLOBAL T_ATI_RSLT setatPlusCMER ( char *cl, UBYTE srcId )
{
/* UINT                   uiBufferCounter; */
  T_ACI_RETURN           tRet = AT_FAIL;
  T_ACI_MM_CMER_VAL_TYPE sCmerSettings;
  T_ACI_MM_CMER_VAL_TYPE sOldCmerSettings;
  T_ACI_MM_CMER_VAL_TYPE sEnteredCmerSettings;
  UINT                   uiDummy1 = 0u;
  UINT                   uiDummy2 = 0u;

  TRACE_FUNCTION("setatPLusCMER()");

  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  sCmerSettings.sCmerModeParam = CMER_MODE_INVALID;
  sCmerSettings.sCmerIndParam  = CMER_INDICATOR_INVALID;
  sCmerSettings.sCmerBfrParam  = CMER_BFR_INVALID;

  cl = parse(cl,"ddddd", &sCmerSettings.sCmerModeParam,
                       &uiDummy1,
                       &uiDummy2,
                       &sCmerSettings.sCmerIndParam,
                       &sCmerSettings.sCmerBfrParam );

  if ((sCmerSettings.sCmerModeParam < CMER_MODE_INVALID OR
      sCmerSettings.sCmerModeParam > CMER_MODE_2) OR
     (sCmerSettings.sCmerIndParam < CMER_INDICATOR_INVALID OR
      sCmerSettings.sCmerIndParam > CMER_INDICATOR_2) OR
     (sCmerSettings.sCmerBfrParam < CMER_BFR_INVALID OR
      sCmerSettings.sCmerBfrParam > CMER_BFR_1) OR
     (uiDummy1 NEQ 0u) OR ((uiDummy2 NEQ 0u)))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  cmhMM_GetCmerSettings( (T_ACI_CMD_SRC)srcId, &sOldCmerSettings );

  sEnteredCmerSettings.sCmerModeParam = sCmerSettings.sCmerModeParam;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cmer cmer;

    /*
    *   Without this we will lose track of the source ID by the time the
    *   response arrives.
    */
    srcId_cb=srcId;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CMER;
    cmd.params.ptr_set_plus_cmer=&cmer;

    cmer.ind_option=BAT_CMER_IND_OPTION_NOT_PRESENT;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  tRet = sAT_PlusCMER((T_ACI_CMD_SRC)srcId, sCmerSettings.sCmerModeParam,
                              sCmerSettings.sCmerIndParam,
                              sCmerSettings.sCmerBfrParam );
  if (tRet EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  if ( sCmerSettings.sCmerModeParam EQ CMER_MODE_INVALID )      { sCmerSettings.sCmerModeParam = sOldCmerSettings.sCmerModeParam; }
  if ( sCmerSettings.sCmerIndParam  EQ CMER_INDICATOR_INVALID ) { sCmerSettings.sCmerIndParam  = sOldCmerSettings.sCmerIndParam; }
  if ( sCmerSettings.sCmerBfrParam  EQ CMER_BFR_INVALID )       { sCmerSettings.sCmerBfrParam  = sOldCmerSettings.sCmerBfrParam; }
  if ( sCmerSettings.sCmerIndParam  EQ CMER_INDICATOR_INVALID ) { sCmerSettings.sCmerIndParam  = CMER_INDICATOR_0; }
  if ( sCmerSettings.sCmerBfrParam  EQ CMER_BFR_INVALID )       { sCmerSettings.sCmerBfrParam  = CMER_BFR_0; }

  switch( sCmerSettings.sCmerModeParam )
  {
    case CMER_MODE_INVALID :
    case CMER_MODE_0 :
    { /* always buffer --> do nothing */
      break;
    }
    case CMER_MODE_1 :
    case CMER_MODE_2 :
    { /* flush and/or clear buffer */
      if ( (sEnteredCmerSettings.sCmerModeParam EQ CMER_MODE_1) OR
          (sEnteredCmerSettings.sCmerModeParam EQ CMER_MODE_2) )
      {
        if ( sCmerSettings.sCmerBfrParam EQ CMER_BFR_0 )
        {
          cmd_clearCievBuf( srcId );
        }
        else
        { /* BFR == '2' */
          cmd_flushCievBuf( srcId );
        }
        
      }
      break;
    }
    default :
    { /* error case */
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
    }
  }
  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCMER      |
+--------------------------------------------------------------------+

  PURPOSE : +CMER query command
            (query the configured indication handling settings 
             toward terminal )
*/
GLOBAL T_ATI_RSLT queatPlusCMER ( char *cl, UBYTE srcId )
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN           sRet = AT_FAIL;
  T_ACI_MM_CMER_VAL_TYPE sCmerSettings;
  UINT                   uiDummy = 0u;
#endif

  TRACE_FUNCTION("queatPLusCMER()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CMER;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cmer = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  sRet = qAT_PlusCMER( (T_ACI_CMD_SRC)srcId, &sCmerSettings.sCmerModeParam,
                              &sCmerSettings.sCmerIndParam,
                              &sCmerSettings.sCmerBfrParam );

  if (sRet NEQ AT_CMPL)
  { 
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  if ( sCmerSettings.sCmerModeParam EQ CMER_MODE_INVALID ) { sCmerSettings.sCmerModeParam = CMER_MODE_0; };
  if ( sCmerSettings.sCmerIndParam  EQ CMER_INDICATOR_INVALID ) { sCmerSettings.sCmerIndParam  = CMER_INDICATOR_0; };
  if ( sCmerSettings.sCmerBfrParam  EQ CMER_BFR_INVALID ) { sCmerSettings.sCmerBfrParam  = CMER_BFR_0; };

  resp_disp( srcId, cl,"eeeee",&sCmerSettings.sCmerModeParam,
                             &uiDummy,
                             &uiDummy,
                             &sCmerSettings.sCmerIndParam,
                             &sCmerSettings.sCmerBfrParam );
  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_flushCievBuf   |
+--------------------------------------------------------------------+

  PURPOSE : This function flushes the buffer for storing new signal-
            and/or SMS-full indications while the serial interface is
            released from data transmission.
*/
GLOBAL void cmd_flushCievBuf ( UBYTE srcId )
{
  UINT                   uiBufferCounter;
  UINT                   uiTempLastIndex;
  T_ACI_MM_CIND_VAL_TYPE sCindValues;
  T_ACI_MM_CMER_VAL_TYPE sCmerSettings;

  TRACE_FUNCTION("cmd_flushCievBuf");

  /* Signal buffer */
  uiTempLastIndex = asCievSignalBuf.uiLastIndex;
  cmhMM_GetCmerSettings((T_ACI_CMD_SRC)srcId, &sCmerSettings);
  for( uiBufferCounter=0; uiBufferCounter < uiTempLastIndex; uiBufferCounter++ )
  {
    sCindValues.sCindSignalParam = asCievSignalBuf.asBufferValues[uiBufferCounter];
    sCindValues.sCindSmsFullParam = CIND_SMSFULL_INDICATOR_INVALID;
    rCI_PlusCIEV( sCindValues, sCmerSettings );
    asCievSignalBuf.asBufferValues[uiBufferCounter] = CIND_SIGNAL_INDICATOR_INVALID;
    asCievSignalBuf.uiLastIndex--;
  }

  /* SMS Full buffer */
  uiTempLastIndex = asCievSmsFullBuf.uiLastIndex;
  for( uiBufferCounter=0; uiBufferCounter < uiTempLastIndex; uiBufferCounter++ )
  {
    sCindValues.sCindSmsFullParam = asCievSmsFullBuf.asBufferValues[uiBufferCounter];
    sCindValues.sCindSignalParam  = CIND_SIGNAL_INDICATOR_INVALID;
    rCI_PlusCIEV( sCindValues, sCmerSettings );
    asCievSmsFullBuf.asBufferValues[uiBufferCounter] = CIND_SMSFULL_INDICATOR_INVALID;
    asCievSmsFullBuf.uiLastIndex--;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_clearCievBuf   |
+--------------------------------------------------------------------+

  PURPOSE : This function clear the buffer for storing new signal-
            and/or SMS-full indications
*/
GLOBAL void cmd_clearCievBuf ( UBYTE srcId )
{
    UBYTE tSrc;
 
    for( tSrc=0; tSrc < CIEV_BUF_SIZE; tSrc++ )
    {
      asCievSignalBuf.asBufferValues[tSrc]  = CIND_SIGNAL_INDICATOR_INVALID;
      asCievSmsFullBuf.asBufferValues[tSrc] = CIND_SMSFULL_INDICATOR_INVALID;
    }
    asCievSignalBuf.uiLastIndex  = 0u;
    asCievSmsFullBuf.uiLastIndex = 0u;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_PlusCOPN_CB   |
+--------------------------------------------------------------------+

  PURPOSE : Call back function for cmd_PlusCOPN_CB
*/

GLOBAL void cmd_PlusCOPN_CB ( UBYTE src_id )
{
  SHORT startIdx;
  UBYTE curCmd;
  T_ATI_SRC_PARAMS *src_params;
  T_COPN_VAR *COPN_Var;
  BOOL ret;


  COPN_Var = &mmShrdPrm.COPN_VAR[src_id];


  startIdx = COPN_Var->Start_Count;

   
  TRACE_FUNCTION("cmd_PlusCOPN_CB()");

  ret = cmd_readCOPNBlock(startIdx,src_id);

  if (ret EQ TRUE)
  {
    src_params = find_element (ati_src_list, src_id, search_ati_src_id);
    curCmd = src_params->curAtCmd;
    R_AT( RAT_OK, (T_ACI_CMD_SRC)src_id )
     ( curCmd );
  }
}

GLOBAL BOOL cmd_readCOPNBlock( SHORT startIdx, UBYTE src_id )
{
  SHORT lastIdx;
  UBYTE idx;
  T_ACI_COPN_LST operLst;
  T_COPN_VAR *COPN_Var;
  
  COPN_Var = &mmShrdPrm.COPN_VAR[src_id];
  
  TRACE_FUNCTION("cmd_readCOPNBlock()");

  if( qAT_PlusCOPN((T_ACI_CMD_SRC)src_id, COPN_LID_Cnst, startIdx,
                     &lastIdx, operLst) EQ AT_FAIL )
     return TRUE;
  
  for( idx=0; idx < MAX_OPER; idx++ )
  {
   if( operLst[idx].numOper[0] EQ 0x0 )
     return TRUE;
   
  sprintf(g_sa,"+COPN: \"%s\",\"%s\"",operLst[idx].numOper,
                                        operLst[idx].alphaOper );
  io_sendMessage(src_id, g_sa, ATI_NORMAL_OUTPUT);
  }
  startIdx = lastIdx+1;
  COPN_Var->Start_Count = startIdx;
  return FALSE;
}


#endif /* ATI_MM_C */

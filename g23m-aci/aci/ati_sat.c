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
|  Purpose :  AT Command Interpreter for SAT related commands
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef ATI_SAT_C
#define ATI_SAT_C

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_lst.h"

#include "aci_io.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"
#endif /* FF_ATI_BAT */

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
GLOBAL UBYTE sat[CMD_SRC_MAX];

EXTERN UBYTE run_at_id; 

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_SAT_CMD        |
| STATE   : code                        ROUTINE : atPercentSATC      |
+--------------------------------------------------------------------+

  PURPOSE : %SATC Configuration for SIM application toolkit.
*/

GLOBAL void aci_sat_cmd_init (void)
{
  T_ACI_CMD_SRC i;
  for (i=CMD_SRC_LCL; i<CMD_SRC_MAX; i++)
    sat[i] = 0; /* no SIM Toolkit indications */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_SAT_CMD        |
| STATE   : code                        ROUTINE : atPercentSATC      |
+--------------------------------------------------------------------+

  PURPOSE : %SATC Configuration for SIM application toolkit.
*/

GLOBAL T_ATI_RSLT setatPercentSATC(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret        = AT_FAIL;
  USHORT           lenPrfl    = 0;
  USHORT           lenPrflStr = 0;
  SHORT            stat       = -1;
  UBYTE           *satPrflStr = NULL;

  cl=parse(cl,"rl", &stat,&lenPrflStr,&satPrflStr );
#ifdef FF_SAT_E
  if(!cl OR (stat NEQ -1 AND stat > SATC_ENA_CL_E))
#else
  if(!cl OR (stat NEQ -1 AND stat > 1))
#endif 
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return(ATI_FAIL);
  }

  if( stat NEQ -1 )
  {
    sat[srcId] = (UBYTE)stat;
  }
  
  if( lenPrflStr )
  {
    utl_hexToGsm( satPrflStr, lenPrflStr, satPrflStr, &lenPrfl, GSM_ALPHA_Def,
                  CSCS_ALPHA_Reserved );

#ifdef  FF_ATI_BAT
    {
      T_BAT_cmd_send cmd;
      T_BAT_cmd_set_percent_satc my_bat_set_percent_satc = {0};
      T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

      TRACE_FUNCTION("setatPercentSATC() calls bat_send() <=== as APPLICATION");

      cmd.ctrl_params = BAT_CMD_SET_PERCENT_SATC;
      cmd.params.ptr_set_percent_satc = &my_bat_set_percent_satc;

      /* Copy and trucate if necessary the satcmd string */
      if(my_bat_set_percent_satc.c_satprfl = (U8)lenPrfl)
      {
        memcpy(my_bat_set_percent_satc.satprfl,
               satPrflStr,
               my_bat_set_percent_satc.c_satprfl <= BAT_MAX_CPIN_PIN_LEN ?
               my_bat_set_percent_satc.c_satprfl : BAT_MAX_CPIN_PIN_LEN);
      }

      src_params->curAtCmd = AT_CMD_SATC;

      bat_send(ati_bat_get_client(srcId), &cmd);
      return ATI_EXCT; /* executing, because response is passed by callback function */
    }
    
#else /* no FF_ATI_BAT */

    TRACE_FUNCTION("setatPercentSATC");

    ret = sAT_PercentSATC( (T_ACI_CMD_SRC)srcId, lenPrfl, satPrflStr );
    if (ret EQ AT_FAIL)
    {
      cmdCmeError(CME_ERR_Unknown);
    }
    return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */  
  }
  return(ATI_CMPL);
}

GLOBAL T_ATI_RSLT tesatPercentSATC(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPercentSATC");

  sprintf(g_sa,"%s: (0,1),(%d)", "%SATC", MAX_STK_PRF*2);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

GLOBAL T_ATI_RSLT queatPercentSATC(char *cl, UBYTE srcId)
{
#ifdef  FF_ATI_BAT

  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPercentSATC() calls bat_send() <=== as APPLICATION");
  
  cmd.ctrl_params = BAT_CMD_QUE_PERCENT_SATC;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_percent_satc = &dummy;
  bat_send(ati_bat_get_client(srcId), &cmd);
  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */

  T_ACI_RETURN     ret        = AT_FAIL;
  USHORT           lenPrfl    = 0;
  UBYTE            satPrfl[MAX_STK_PRF];
  UBYTE            i;

  TRACE_FUNCTION("queatPercentSATC");

  ret = qAT_PercentSATC((T_ACI_CMD_SRC)srcId,(SHORT*)&lenPrfl, satPrfl);
  if (ret EQ AT_CMPL)
  {
    i=sprintf(g_sa,"%s: %d,\"", "%SATC", sat[srcId]);

    if (lenPrfl)
    {
      utl_binToHex( satPrfl, lenPrfl, g_sa +i );
    }

    i+=lenPrfl*2;
    g_sa[i++]='"';
    g_sa[i]='\0';
    
    ci_remTrailCom(g_sa,(USHORT)strlen(g_sa));
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
| STATE   : code                        ROUTINE : atPercentSATE      |
+--------------------------------------------------------------------+

  PURPOSE : %SATE Send SAT envelope command
*/
GLOBAL T_ATI_RSLT atPercentSATE(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  UBYTE           *satCmd    = NULL ;
  USHORT           lenCmd    = 0;
  USHORT           lenCmdStr = 0;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl,"l", &lenCmdStr,&satCmd );

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  utl_hexToGsm( satCmd, lenCmdStr, satCmd, &lenCmd, GSM_ALPHA_Def,
                CSCS_ALPHA_Reserved );

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_sate my_bat_set_percent_sate = {0};

    TRACE_FUNCTION("setatPercentSATE() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_SATE;
    cmd.params.ptr_set_percent_sate = &my_bat_set_percent_sate;

    /* Copy and trucate if necessary the satemd string */
    if(my_bat_set_percent_sate.c_satcmd = (U8)lenCmd)
    {
      memcpy(my_bat_set_percent_sate.satcmd,
             satCmd,
             my_bat_set_percent_sate.c_satcmd <= BAT_MAX_STK_CMD ?
             my_bat_set_percent_sate.c_satcmd : BAT_MAX_STK_CMD);
    }

    src_params->curAtCmd = AT_CMD_SATE;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
    
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("atPercentSATE");

  ret = sAT_PercentSATE( (T_ACI_CMD_SRC)srcId, lenCmd, satCmd );
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_SATE;
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
| STATE   : code                        ROUTINE : atPercentSATR      |
+--------------------------------------------------------------------+

  PURPOSE : %SATE Send SAT command response
*/
GLOBAL T_ATI_RSLT atPercentSATR(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  UBYTE           *satRsp    = NULL ;
  USHORT           lenRsp    = 0;
  USHORT           lenRspStr = 0;

  cl = parse(cl,"l", &lenRspStr,&satRsp );

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  utl_hexToGsm( satRsp, lenRspStr, satRsp, &lenRsp, GSM_ALPHA_Def,
                CSCS_ALPHA_Reserved );

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_satr my_bat_set_percent_satr = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

    TRACE_FUNCTION("setatPercentSATR() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_SATR;
    cmd.params.ptr_set_percent_satr = &my_bat_set_percent_satr;

    /* Copy and trucate if necessary the satrmd string */
    if(my_bat_set_percent_satr.c_satrsp = (U8)lenRsp)
    {
      memcpy(my_bat_set_percent_satr.satrsp,
             satRsp,
             my_bat_set_percent_satr.c_satrsp <= BAT_MAX_STK_CMD ?
             my_bat_set_percent_satr.c_satrsp : BAT_MAX_STK_CMD);
    }

    src_params->curAtCmd = AT_CMD_SATR;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
    
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("atPercentSATR");

  ret = sAT_PercentSATR((T_ACI_CMD_SRC)srcId, lenRsp, satRsp );

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentSATT      |
+--------------------------------------------------------------------+

  PURPOSE : %SATT Terminate SAT command or session
*/
GLOBAL T_ATI_RSLT atPercentSATT(char *cl, UBYTE srcId)
{
  T_ACI_RETURN     ret       = AT_FAIL;
  T_ACI_SATT_CS    cs        = SATT_CS_NotPresent;


  cl = parse(cl,"d", &cs );
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_satt my_bat_set_percent_satt = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

    TRACE_FUNCTION("setatPercentSATT() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_SATT;
    cmd.params.ptr_set_percent_satt = &my_bat_set_percent_satt;

    my_bat_set_percent_satt.cs = (T_BAT_percent_satt_cs)cs;

    src_params->curAtCmd = AT_CMD_SATT;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
    
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("atPercentSATT");

  ret = sAT_PercentSATT( (T_ACI_CMD_SRC)srcId, cs );

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentEFRSLT |
+--------------------------------------------------------------------+

  PURPOSE : %EFRSLT  Response to EF update indication
*/
GLOBAL T_ATI_RSLT setatPercentEFRSLT(char *cl,UBYTE srcId)
{
  T_ACI_EFRSLT_RES res;
  T_ACI_RETURN ret;

  TRACE_FUNCTION("setatPercentEFRSLT()");

  switch(*cl)
  {
    case '0':
      res=EFRSLT_RES_FAIL;
      break;

    case '1':
      res=EFRSLT_RES_OK;
      break;

    default:
      cmdCmeError(CME_ERR_OpNotSupp);
      return(ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_efrslt efrslt;

    cmd.ctrl_params=BAT_CMD_SET_PERCENT_EFRSLT;
    cmd.params.ptr_set_percent_efrslt=&efrslt;

    /*
    *   This relies on T_BAT_percent_efrslt_result and T_ACI_EFRSLT_RES
    *   being identical.
    */
    efrslt.result=(T_BAT_percent_efrslt_result)res;

    bat_send(ati_bat_get_client(srcId),&cmd);
    return(ATI_EXCT);
  }

#else

  ret=sAT_PercentEFRSLT((T_ACI_CMD_SRC)srcId,res);
  return(map_aci_2_ati_rslt(ret));

#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentSIMEF  |
+--------------------------------------------------------------------+

  PURPOSE : %SIMEF  Receive notification of EF updates
*/
GLOBAL T_ATI_RSLT setatPercentSIMEF(char *cl,UBYTE srcId)
{
  T_ACI_SIMEF_MODE mode;
  T_ACI_RETURN ret;

  TRACE_FUNCTION("setatPercentSIMEF()");

  switch(*cl)
  {
    case '0':
      mode=SIMEF_MODE_OFF;
      break;

    case '1':
      mode=SIMEF_MODE_ON;
      break;

    default:
      cmdCmeError(CME_ERR_OpNotSupp);
      return(ATI_FAIL);
  }
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_simef simef;

    cmd.ctrl_params=BAT_CMD_SET_PERCENT_SIMEF;
    cmd.params.ptr_set_percent_simef=&simef;

    /*
    *   This relies on T_BAT_percent_simef_mode and T_ACI_SIMEF_MODE
    *   being identical.
    */
    simef.mode = (T_BAT_percent_simef_mode)mode;

    bat_send(ati_bat_get_client(srcId),&cmd);
    return(ATI_EXCT);
  }

#else
  ret=sAT_PercentSIMEF((T_ACI_CMD_SRC)srcId,mode);

  return(map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentSIMEF  |
+--------------------------------------------------------------------+

  PURPOSE : %SIMEF  Receive notification of EF updates
*/
GLOBAL T_ATI_RSLT queatPercentSIMEF(char *cl, UBYTE srcId)
{
#ifdef  FF_ATI_BAT

  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPercentSIMEF() calls bat_send() <=== as APPLICATION");
  
  cmd.ctrl_params = BAT_CMD_QUE_PERCENT_SIMEF;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_percent_simef = &dummy;
  bat_send(ati_bat_get_client(srcId), &cmd);
  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */
  T_ACI_RETURN ret;
  T_ACI_SIMEF_MODE mode;

  TRACE_FUNCTION("queatPercentSIMEF");

  ret=qAT_PercentSIMEF((T_ACI_CMD_SRC)srcId,&mode);

  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s: %d","%SIMEF",mode);
    io_sendMessage(srcId,g_sa,ATI_NORMAL_OUTPUT);
  }

  return(map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : handle_rCI_PercentSATx |
+--------------------------------------------------------------------+

  PURPOSE : handles chunked output for several rCI_PercentSAT functions
*/

LOCAL void handle_rCI_PercentSATx ( CHAR           *prefix,
                                    SHORT           len,
                                    UBYTE          *satCmd,
                               T_ACI_SATN_CNTRL_TYPE  cntrl_type)
{
  SHORT i;
  UBYTE srcId = srcId_cb;
  SHORT chunksize;
  T_ATI_OUTPUT_TYPE output_type =(T_ATI_OUTPUT_TYPE) (ATI_INDICATION_OUTPUT |
                                  ATI_BEGIN_CRLF_OUTPUT);

  if( sat[srcId] )
  {
    i=sprintf(g_sa, "%s: \"", prefix);
    do
    {
      chunksize = (sizeof(g_sa)-i-2)/2; /* -2 for '"' and '\0' at the end, /2 since we occupy two bytes after binToHex */

      if (cntrl_type NEQ SATN_CNTRL_BY_SIM_Not_Present)
      {
        chunksize -= 2; /* Take another 2 characters off for the ,<cntrl_type> */
      }
      chunksize = MINIMUM(len, chunksize);

      utl_binToHex( satCmd, chunksize, g_sa+i );

      if ((len -= chunksize) EQ 0)
      {
        /* end reached? then append CRLF to the last output */
        output_type |= ATI_END_CRLF_OUTPUT;

        i+=2*chunksize;

        g_sa[i++]='"';        /* closing quotation marks */

        /*
        ** Add the control type onto the end, if given
        */
        if (cntrl_type NEQ SATN_CNTRL_BY_SIM_Not_Present)
        {
            g_sa[i++] = ',';
            switch(cntrl_type)
            {
                case SATN_CNTRL_BY_SIM_CALL:
                    g_sa[i++] = '0';
                    break;

                case SATN_CNTRL_BY_SIM_SS:
                    g_sa[i++] = '1';
                    break;

                case SATN_CNTRL_BY_SIM_USSD:
                    g_sa[i++] = '2';
                    break;

                case SATN_CNTRL_BY_SIM_SMS:
                    g_sa[i++] = '3';
                    break;

                default:
                    i--;
                    break;
            }

        }

        g_sa[i]='\0';
      }

      io_sendMessageEx(srcId, g_sa, output_type);

      /* remove the BEGIN_CRLF for the next possible chunk */
      output_type &= (T_ATI_OUTPUT_TYPE)~ATI_BEGIN_CRLF_OUTPUT;

      g_sa[0] = '\0';
      i=0;
      satCmd += chunksize;
    } while (len);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSATI    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSATI call back

*/

GLOBAL void rCI_PercentSATI  ( /*UBYTE srcId,*/
                               SHORT           len,
                               UBYTE          *satCmd)
{
  TRACE_FUNCTION("rCI_PercentSATI()");
  handle_rCI_PercentSATx ("%SATI", len, satCmd, SATN_CNTRL_BY_SIM_Not_Present);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSATE    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSATE call back

*/

GLOBAL void rCI_PercentSATE  ( /*UBYTE srcId,*/
                               SHORT           len,
                               UBYTE          *satCmd)
{
  TRACE_FUNCTION("rCI_PercentSATE()");
  handle_rCI_PercentSATx ("%SATE", len, satCmd, SATN_CNTRL_BY_SIM_Not_Present);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSATN    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSATN call back

*/

GLOBAL void rCI_PercentSATN  ( /*UBYTE srcId,*/
                               SHORT           len,
                               UBYTE          *satCmd,
                               T_ACI_SATN_CNTRL_TYPE  cntrl_type)
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentSATN()");

  if(srcId NEQ run_at_id)
  {
    handle_rCI_PercentSATx ("%SATN", len, satCmd, cntrl_type);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentSATA    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentSATA call back

*/

#ifdef FF_SAT_E
GLOBAL void rCI_PercentSATA  ( /*UBYTE srcId,*/
                               SHORT cId,
                               LONG  rdlTimeout_ms,
                               T_ACI_SATA_ADD *addParm )
#else
GLOBAL void rCI_PercentSATA  ( /*UBYTE srcId,*/
                               SHORT cId,
                               LONG  rdlTimeout_ms)
#endif /* FF_SAT_E */
{
  UBYTE i;
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentSATA()");

  if( sat[srcId] )
  {
    i=sprintf(g_sa,"%s: ","%SATA");

    if(rdlTimeout_ms NEQ ACI_NumParmNotPresent)
    {
      i+=sprintf( g_sa+i, "%d", rdlTimeout_ms );
    }
    
#ifdef FF_SAT_E  
    if( sat[srcId] EQ SATC_ENA_CL_E )
    {
      sprintf( g_sa+i, ",%d,%d", addParm->chnType, addParm->chnEst );
    }
#endif /* FF_SAT_E */

    io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
}

#endif /* ATI_SAT_C */

#endif /* #ifdef SIM_TOOLKIT */


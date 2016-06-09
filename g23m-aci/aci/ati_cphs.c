/*
+--------------------------------------------------------------------+
| PROJECT:                              $Workfile:: ati_cphs.c      $|
| $Author::                             $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : ATI

   PURPOSE : CPHS related proprietery AT commands.
*/


#ifndef ATI_CPHS_C
#define ATI_CPHS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_lst.h"
#include "ati_int.h"
#include "aci_prs.h"
#include "aci_mem.h"
#include "aci_io.h"

#include "cphs.h"
#include "aci_cphs.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

LOCAL T_ATI_RSLT query_indicator_flags(UBYTE srcId, T_CPHS_LINES queried_lines, T_ACI_AT_CMD indicator_type);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPHS      |
+--------------------------------------------------------------------+

  PURPOSE : %CPHS: CPHS module initialing/closing/refreshing
*/

GLOBAL T_ATI_RSLT setatPercentCPHS (CHAR *cl, UBYTE srcId)
{
  T_ACI_CPHS_INIT  initMode;
  T_ACI_RETURN     ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl, "d", &initMode);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cphs my_bat_set_percent_cphs;

  TRACE_FUNCTION("setatPercentCPHS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cphs, 0, sizeof(my_bat_set_percent_cphs));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPHS;
  cmd.params.ptr_set_percent_cphs = &my_bat_set_percent_cphs;

  my_bat_set_percent_cphs.init_mode = initMode;
  src_params->curAtCmd = AT_CMD_CPHS;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  TRACE_FUNCTION("setatPercentCPHS()");

  ret=sAT_PercentCPHS((T_ACI_CMD_SRC)srcId, initMode);

  switch (ret)
  {
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_CPHS;
    break;

  case (AT_BUSY):
    cmdCmeError(CME_ERR_SimBusy);
    return(ATI_FAIL);

  case (AT_FAIL):
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  return(map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPercentCPHS (CHAR *cl, UBYTE srcId)
{
  T_ACI_CPHS_INIT initMode;
  T_ACI_RETURN    ret;

  TRACE_FUNCTION("queatPercentCPHS()");

  ret = qAT_PercentCPHS((T_ACI_CMD_SRC)srcId, &initMode);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  sprintf(g_sa, "%s%d", "%CPHS: ", initMode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return(ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPNUMS    |
+--------------------------------------------------------------------+

  PURPOSE : %CPNUMS: CPHS information numbers management.
*/

GLOBAL T_ATI_RSLT setatPercentCPNUMS (CHAR *cl, UBYTE srcId)
{
  SHORT              element_id;
  T_CPHS_CPNUMS_MODE mode;
  T_ACI_RETURN       ret = AT_FAIL;

  cl = parse(cl, "rd", &element_id, &mode);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cpnums my_bat_set_percent_cpnums;

  TRACE_FUNCTION("setatPercentYYYY() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cpnums, 0, sizeof(my_bat_set_percent_cpnums));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPNUMS;
  cmd.params.ptr_set_percent_cpnums = &my_bat_set_percent_cpnums;

  my_bat_set_percent_cpnums.element_id = (U8)element_id;
  my_bat_set_percent_cpnums.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentCPNUMS()");

  ret=sAT_PercentCPNUMS((T_ACI_CMD_SRC)srcId, (UBYTE)element_id, (UBYTE)mode);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  return(ATI_CMPL);  

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT tesatPercentCPNUMS (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret;

  TRACE_FUNCTION("tesatPercentCPNUMS()");

  ret = tAT_PercentCPNUMS((T_ACI_CMD_SRC)srcId);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }
  return(ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCPNUMS  |
+--------------------------------------------------------------------+

  PURPOSE : handles AT%CPNUMS call back
*/
GLOBAL void rCI_PercentCPNUMS(UBYTE element_index,
                              UBYTE index_level,
                              CHAR  *alpha_tag,
                              CHAR  *number,
                              BOOL  premium_flag,
                              BOOL  network_flag,
                              UBYTE type_of_address)
{
  UBYTE srcId = srcId_cb;
  CHAR cvtd_text[4*CPHS_MAX_INF_ALPHA_TAG];
  USHORT len_cvtd=0;
  USHORT pos=0;

  TRACE_FUNCTION("rCI_PercentCPNUMS()");

  utl_chsetFromSim(
    (UBYTE *)alpha_tag,
    (USHORT)strlen(alpha_tag),
    (UBYTE *)cvtd_text,
    sizeof(cvtd_text),
    &len_cvtd,
    GSM_ALPHA_Def);

  pos=sprintf(g_sa,"%s%d,","%CPNUMS: ",element_index);
  pos+=sprints(g_sa+pos,cvtd_text,len_cvtd);
  pos+=sprintf(g_sa+pos,",\"%s\",%d,%d,%d,%d",number,index_level,premium_flag,network_flag,type_of_address);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

#ifndef MFW
#ifndef FF_MMI_RIV
/* dummy in case of SMI */
GLOBAL void rAT_PercentCPNUMS(UBYTE element_index,
                              UBYTE index_level,
                              CHAR  *alpha_tag,
                              CHAR  *number,
                              BOOL  premium_flag,
                              BOOL  network_flag,
                              UBYTE type_of_address)
{
}
#endif /*ndef FF_MMI_RIV */
#endif /* ndef MFW */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_RET            |
| STATE   : code                        ROUTINE : rCI_PercentCPROAM  |
+--------------------------------------------------------------------+

  PURPOSE : handles %CPROAM indication
            roam_status = 1 mobile entering roaming service // = 0 going back to home netwrok
*/
GLOBAL void rCI_PercentCPROAM(UBYTE roam_status)
{
  UBYTE srcId = srcId_cb;

  TRACE_FUNCTION("rCI_PercentCPROAM()");

  sprintf(g_sa, "%s: %d", "%CPROAM", roam_status);

  io_sendIndication(srcId, g_sa, ATI_NORMAL_OUTPUT);
}

#ifndef MFW
#ifndef FF_MMI_RIV
/* dummy in case of SMI */
GLOBAL void rAT_PercentCPROAM(UBYTE roam_status)
{}
#endif /* ndef FF_MMI_RIV */
#endif /* ndef MFW */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPALS     |
+--------------------------------------------------------------------+

  PURPOSE : %CPALS: CPHS Alternate Line Service.
*/

LOCAL T_ATI_RSLT proceed_CPALS(UBYTE srcId, UBYTE call_id)
{
  T_ACI_RETURN ret;
  T_CPHS_LINES active_line;
  CHAR         *line_desc;
  UBYTE        max_line_desc;

  max_line_desc = 30; /* should be enough ! */
  MALLOC(line_desc, max_line_desc);

  ret = qAT_PercentCPALS((T_ACI_CMD_SRC)srcId, call_id, &active_line, line_desc, &max_line_desc);

  if (ret EQ AT_EXCT)
  {
    TRACE_EVENT_P1("Quite a line_desc !: %d", max_line_desc);
    MFREE(line_desc);
    MALLOC(line_desc, max_line_desc);

    /* retry */
    ret = qAT_PercentCPALS((T_ACI_CMD_SRC)srcId, call_id, &active_line, line_desc, &max_line_desc);
  }

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    MFREE(line_desc);
    return(ATI_FAIL);
  }

  sprintf(g_sa, "%s: %d,\"%s\"", "%CPALS", active_line, line_desc);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  MFREE(line_desc);

  return(ATI_CMPL);
}

GLOBAL T_ATI_RSLT setatPercentCPALS (CHAR *cl, UBYTE srcId)
{
  SHORT call_id;

  cl = parse(cl, "r", &call_id);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cpals my_bat_set_percent_cpals;

  TRACE_FUNCTION("setatPercentCPALS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cpals, 0, sizeof(my_bat_set_percent_cpals));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPALS;
  cmd.params.ptr_set_percent_cpals = &my_bat_set_percent_cpals;

  my_bat_set_percent_cpals.call_id = (U8)call_id;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentCPALS()");
  return proceed_CPALS(srcId, (UBYTE)call_id);

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPercentCPALS (CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPercentCPALS()");

  return(proceed_CPALS(srcId, NOT_PRESENT_8BIT));
}

GLOBAL T_ATI_RSLT tesatPercentCPALS (CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPercentCPALS()");

  return(ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPVWI     |
+--------------------------------------------------------------------+

  PURPOSE : %CPVWI: set/clear/query voice message waiting flags
*/

GLOBAL T_ATI_RSLT setatPercentCPVWI(CHAR *cl, UBYTE srcId)
{
  T_CPHS_LINES     i = 0, 
                   typed_lines  = NOT_PRESENT_16BIT,
                   line = 0;
  SHORT            action = 0;
  T_ACI_RETURN     ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element( ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl,"rr", &action, &typed_lines);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return(ATI_FAIL);
  }

  if (typed_lines EQ NOT_PRESENT_16BIT)
  {
    TRACE_EVENT("typed_lines omitted");

    typed_lines = 0;

    /* lines parameter omitted: Provides VWI flags for all lines */
    for(i=0;i<8*sizeof(T_CPHS_LINES);i++)
    {
      line = CPHS_LINE1 << i;

      if (cphs_line_makes_sense(line))
      {
        typed_lines |= line;
      } 
    }
  }

  TRACE_EVENT_P2("setatPercentCPVWI action: %d, lines: %d", action, typed_lines);

  
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cpvwi my_bat_set_percent_cpvwi;

  TRACE_FUNCTION("setatPercentCPVWI() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cpvwi, 0, sizeof(my_bat_set_percent_cpvwi));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPVWI;
  cmd.params.ptr_set_percent_cpvwi = &my_bat_set_percent_cpvwi;

  my_bat_set_percent_cpvwi.mode = action;
  my_bat_set_percent_cpvwi.lines = typed_lines;
  src_params->curAtCmd = AT_CMD_CPVWI;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  /******* QUERY ********/
  if (action EQ CPHS_QUERY_WAITING_FLAG)
  {
    return query_indicator_flags(srcId, typed_lines, AT_CMD_CPVWI);
  }
  
  TRACE_FUNCTION("setatPercentCPVWI()");

  /******* SETTING ********/
  ret = sAT_PercentCPVWI((T_ACI_CMD_SRC)srcId, (UBYTE)action, typed_lines);

  switch (ret)
  {
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_CPVWI;
    break;
  
  case (AT_BUSY):
    cmdCmeError(CME_ERR_SimBusy);
    return(ATI_FAIL);

  case (AT_FAIL):
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  return(map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ATI_CPHS           |
| STATE   : code                        ROUTINE : rCI_PercentCPVWI   |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PercentCPVWI call back

*/

GLOBAL void rCI_PercentCPVWI  (UBYTE  flag_set, 
                               USHORT line)
{
  UBYTE srcId = srcId_cb;
  
  TRACE_FUNCTION("rCI_PercentCPVWI()");

  sprintf(g_sa, "%s: %d,%d", "%CPVWI", flag_set, line);
  io_sendIndication((UBYTE)srcId, g_sa, ATI_NORMAL_OUTPUT);
}

#ifndef MFW
#ifndef FF_MMI_RIV
/* dummy in case of SMI */
GLOBAL void rAT_PercentCPVWI ( UBYTE         flag_set, 
                               USHORT        line)
{ }
#endif /* FF_MMI_RIV */
#endif /* MFW */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPOPN     |
+--------------------------------------------------------------------+

  PURPOSE : %CPOPN: query operator name string
*/

GLOBAL T_ATI_RSLT queatPercentCPOPN(CHAR *cl, UBYTE srcId)
{
  CHAR long_name[CPHS_MAX_OPER_LONG];
  CHAR short_name[CPHS_MAX_OPER_SHORT];
  UBYTE long_len;
  UBYTE short_len;
  T_ACI_RETURN ret;

  TRACE_FUNCTION("queatPercentCPOPN()");

  long_len  = CPHS_MAX_OPER_LONG;
  short_len = CPHS_MAX_OPER_SHORT;
  ret = qAT_PercentCPOPN((T_ACI_CMD_SRC)srcId, long_name, &long_len, short_name, &short_len);

  if (ret NEQ AT_CMPL)
  {
    if (short_len > 0 OR
       long_len > 0)
    {
      TRACE_EVENT_P2("Buffer for Long Name or Short Name too small needed: %d, %d", long_len, short_len);
    }

    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  if ( short_len > 0 )
  {
    sprintf(g_sa, "%s: \"%s\",\"%s\"", "%CPOPN", long_name, short_name);
  }
  else
  {
    sprintf(g_sa, "%s: \"%s\"", "%CPOPN", long_name);
  }
  io_sendMessage((UBYTE)srcId, g_sa, ATI_NORMAL_OUTPUT);
    
  return(ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCPINF  |
+--------------------------------------------------------------------+

  PURPOSE : %CPINF: set customer service profile
*/

GLOBAL T_ATI_RSLT setatPercentCPINF(CHAR *cl, UBYTE srcId)
{
  CHAR   csp[CPHS_MAX_CSP*2 + 1];
  USHORT csp_size = 0;
  UBYTE index;                     /* for parsing failure analysis */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  
  /* initialize the local varibles */
  memset(csp,0,sizeof(csp));

  cl = parse(cl, "z",
             (LONG)sizeof (csp),
             strlen(cl),
             cl,
             &csp_size,
             csp);
 
  if (cl EQ NULL)
  {
    index = get_parse_index();         /* get the problematic component */
    TRACE_EVENT_P1("index %d",index);
    switch (index)
    {
      case 1:
        cmdCmeError(CME_ERR_TxtToLong);
        break;
      default:
        cmdCmeError(CME_ERR_OpNotAllow);
        break;
    }
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cpinf cpinf;
    TRACE_FUNCTION("setatPercentCPINF() calls bat_send() <=== as APPLICATION");
    
    memset(&cpinf, 0, sizeof(cpinf));
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPINF;
    cmd.params.ptr_set_percent_cpinf = &cpinf;
    cpinf.c_csp = (U8)csp_size;
    memcpy(cpinf.csp, csp, csp_size);
    src_params->curAtCmd = AT_CMD_CPINF;
    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT;
  }
#else
  {
    T_ACI_RETURN ret = AT_FAIL;
    UBYTE  serv_group[CPHS_MAX_CSP];
    UBYTE  serv_len = 0;

    TRACE_FUNCTION("setatPercentCPINF()");

    memset(serv_group,0,sizeof(serv_group));

    serv_len = utl_HexStrToBin((UBYTE*)csp, csp_size, serv_group, CPHS_MAX_CSP);
    
    /* Odd length CSP or empty CSP display operation not allowed */
    if(((serv_len % 2) NEQ 0) OR (serv_len EQ 0))
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
    }
    /* write customer service profile (csp) */
    ret = sAT_PercentCPINF( (T_ACI_CMD_SRC)srcId,
                            serv_group,
                            serv_len);

    switch (ret)
    {
      case (AT_EXCT):
        src_params->curAtCmd = AT_CMD_CPINF;
        break;
      case (AT_CMPL):
        break;
      default:
      break;
    }
  return(map_aci_2_ati_rslt(ret));
  }
#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPINF     |
+--------------------------------------------------------------------+

  PURPOSE : %CPINF: query cphs information and customer service profile
*/

GLOBAL T_ATI_RSLT queatPercentCPINF(CHAR *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPercentCPINF() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CPINF;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_percent_cpinf = &dummy;

  bat_send(ati_bat_get_client(srcId),&cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* FF_ATI_BAT */

  UBYTE  phase;
  USHORT sst;
  CHAR   csp[CPHS_MAX_CSP*2 + 1];
  CHAR   csp2[CPHS_MAX_CSP*2 + 1]; /* Since the CPHS_MAX_CSP length has been updated to 22,then there is no need of CPHS_MAX_CSP2 */  
  UBYTE  max_csp_size = CPHS_MAX_CSP*2 + 1;
  UBYTE  max_csp2_size = CPHS_MAX_CSP*2 + 1;
  T_ACI_RETURN ret;

  TRACE_FUNCTION("queatPercentCPINF()");

  ret = qAT_PercentCPINF ((T_ACI_CMD_SRC)srcId, 
                          &phase, 
                          &sst, 
                          csp, 
                          csp2, 
                          &max_csp_size, 
                          &max_csp2_size);

  if (ret NEQ AT_CMPL)
  {
    if (max_csp_size > 0)
    {
      TRACE_EVENT_P1("Buffer for CSP is too small ! needed: %d", max_csp_size);
    }

    if (max_csp2_size > 0)
    {
      TRACE_EVENT_P1("Buffer for CSP2 is too small ! needed: %d", max_csp2_size);
    }
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  if ( max_csp_size > 0 )
  {
    if ( max_csp2_size > 0 )
    {
      sprintf(g_sa, "%s: %d,\"%04X\",\"%s\",\"%s\"", "%CPINF",phase, sst, csp, csp2);
    }
    else
    {
      sprintf(g_sa, "%s: %d,\"%04X\",\"%s\"", "%CPINF",phase, sst, csp);
    }
  }
  else
  {
    sprintf(g_sa, "%s: %d,\"%04X\"", "%CPINF", phase, sst);
  }
  io_sendMessage((UBYTE)srcId, g_sa, ATI_NORMAL_OUTPUT);
  
  return(ATI_CMPL);
  
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPMB      |
+--------------------------------------------------------------------+

  PURPOSE : %CPMB: query mailbox numbers
*/

GLOBAL T_ATI_RSLT setatPercentCPMB(CHAR *cl, UBYTE srcId)
{
  SHORT rec_id;
  T_ACI_RETURN ret = AT_FAIL;

  cl = parse(cl, "r", &rec_id);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cpmb my_bat_set_percent_cpmb;

  TRACE_FUNCTION("setatPercentCPMB() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cpmb, 0, sizeof(my_bat_set_percent_cpmb));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPMB;
  cmd.params.ptr_set_percent_cpmb = &my_bat_set_percent_cpmb;

  my_bat_set_percent_cpmb.record_id = rec_id;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  {
  T_CPHS_LINES line;
  CHAR number[CPHS_MAX_MB_NUMBER_LEN];
  CHAR alpha_id[CPHS_MAX_MB_ALPHA_LEN];
  T_ACI_TOA_TON ton;
  T_ACI_TOA_NPI npi;
  UBYTE         toa;
  CHAR cvtd_text[4*CPHS_MAX_MB_ALPHA_LEN];
  USHORT len_cvtd=0;
  USHORT pos=0;

  
  TRACE_FUNCTION("setatPercentCPMB()");

  ret = qAT_PercentCPMB((T_ACI_CMD_SRC)srcId, (UBYTE)rec_id, &line, number, &ton, &npi, alpha_id, NULL);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  /* check if empty mb entry */
  if (number[0] EQ '\0')
  { /* do not send to ATI */
    cmdCmeError(CME_ERR_NotFound);
    return(ATI_FAIL);
  }

  toa = ( ( ( ton << 4 ) & 0xF0 ) + ( npi & 0x0F ) ) | 0x80;

  utl_chsetFromSim(
    (UBYTE *)alpha_id,
    (USHORT)strlen(alpha_id),
    (UBYTE *)cvtd_text,
    sizeof(cvtd_text),
    &len_cvtd,
    GSM_ALPHA_Def);

  pos=sprintf(g_sa,"%s: %d,%d,\"%s\",%d,", "%CPMB", rec_id, line, number, toa);
  pos+=sprints(g_sa+pos,cvtd_text,len_cvtd);

  io_sendMessage((UBYTE)srcId, g_sa, ATI_NORMAL_OUTPUT);

  return(ATI_CMPL);
  }
#endif /* no FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCPMB   |
+--------------------------------------------------------------------+

  PURPOSE : %CPMB?: query mailbox numbers
*/
GLOBAL T_ATI_RSLT queatPercentCPMB(char *cl, UBYTE srcId)
{
#ifdef  FF_ATI_BAT
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPercentCPMB() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params=BAT_CMD_QUE_PERCENT_CPMB;
  dummy.bat_dummy=0xFF;
  cmd.params.ptr_que_percent_cpmb=&dummy;

  bat_send(ati_bat_get_client(srcId),&cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* FF_ATI_BAT */
  char *me="%CPMB: ";
  T_ACI_RETURN    ret;
  UBYTE           first;

  TRACE_FUNCTION("queatPercentCPMB()");

  /*
  *   Call the corresponding ACI function.
  */
  ret=qAT_PercentCPMB((T_ACI_CMD_SRC)srcId,(UBYTE)ACI_NumParmNotPresent,NULL,NULL,NULL,NULL,NULL,&first);

  /*
  *   If the query completes successfully, build and send the
  *   answer.
  */
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d",me,first);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }

  return (map_aci_2_ati_rslt(ret));
#endif /* FF_ATI_BAT */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPMBW     |
+--------------------------------------------------------------------+

  PURPOSE : %CPMB: write mailbox numbers
*/

GLOBAL T_ATI_RSLT setatPercentCPMBW(CHAR *cl, UBYTE srcId)
{
  CHAR    number[CPHS_MAX_MB_NUMBER_LEN];

#ifndef FF_ATI_BAT
  CHAR   *p_number;
  
  T_CPHS_PB_TEXT  cvtdText;     /* holds text converted to SIM alpha */
  T_ACI_TOA  toa;               /* holds the type of record */
  T_ACI_RETURN ret;
#endif

  UBYTE index; /* for parsing failure analysis */
  
  CHAR    alpha_id[CPHS_MAX_MB_ALPHA_LEN] = {0x00};
  USHORT  len_alpha_id = 0;   
  USHORT  lenCvtd      = 0;         /* holds length of converted text */
  T_CPHS_PB_TEXT *p_text = NULL;    /* holds pointer to converted text */
 
  SHORT      toa_val =  0;  /* holds type of record value */
  SHORT      rec_id  = -1;  /* holds index of record */

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentCPMBW()");

  /* initialize */
  memset(number,0,sizeof(number));

  /***********************************************/
  /*** prepare parameters and test their range ***/
  /***********************************************/

  /* parse command string */  
  cl = parse(cl,"rsrz",
              &rec_id,
              (LONG)CPHS_MAX_MB_NUMBER_LEN,
              number,
              &toa_val,
              (LONG)CPHS_MAX_MB_ALPHA_LEN,
              strlen(cl),
              cl,
              &len_alpha_id,
              alpha_id);

  /* Throw error if parsing has been failed */
  if (!cl)
  {
    TRACE_ERROR("ERROR: parsing failed !");
    /* analyse error component of input */
    index = get_parse_index(); /* get the problematic component */
    switch (index)
    {
      case 3:   /* component 2 is the number */
        cmdCmeError(CME_ERR_DialToLong);
        break;
      case 5:   /* component 4 is the alpha tag */
        cmdCmeError(CME_ERR_TxtToLong);
        break;
      default:
        cmdCmeError(CME_ERR_OpNotAllow);
        break;
    }
    return ATI_FAIL;
  }

  /* test if values are in correct ranges */
  if ( rec_id >= CPHS_MAX_MB_ENTRIES) /* phonebook index is greater than max. */
  {
    TRACE_ERROR("ERROR: phonebook index is greater than max possible index of SIM");
    cmdCmeError(CME_ERR_InvIdx);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cpmbw cpmbw;
    int len;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPMBW;
    cmd.params.ptr_set_percent_cpmbw=&cpmbw;

    cpmbw.record_id=(T_BAT_record_id)rec_id;

    len=strlen(number);

    if ((len<0) OR (len>BAT_MAX_CPMBW_NUMBER_LEN))
    {
      /*
      *   Either the number wasn't present, or it was too big to
      *   fit in the BAT message.
      */
      cpmbw.v_number=FALSE;
      cpmbw.type=-1;
    }
    else
    {
      cpmbw.v_number=TRUE;
      cpmbw.c_number=(U8)len;
      memcpy(cpmbw.number,number,len);
      cpmbw.type=(toa_val>0) ? toa_val:-1;
    }

    if ((len_alpha_id>0) AND (len_alpha_id<=BAT_MAX_CPMBW_TEXT_LEN))
    {
      cpmbw.v_text=TRUE;
      cpmbw.c_text=(U8)len_alpha_id;
      memcpy(cpmbw.text,alpha_id,len_alpha_id);
    }
    else
    {
      /*
      *   Either the alphanumeric data wasn't present, or it was
      *   too big to fit in the BAT message.
      */
      cpmbw.v_text=FALSE;
    }

    bat_send(ati_bat_get_client(srcId),&cmd);
    return(ATI_EXCT);
  }

#else /* OLD FUNCTION BODY */

  /*
  *   Initialise 'converted text'.
  */
  cvtdText.len = 0;
  memset(cvtdText.data,0,sizeof(cvtdText.data));

  /* extract type structure from type value */
  if (toa_val > 0)
  {
    toa   = toa_demerge(toa_val);
  }
  /* translate alpha text to SIM */  
  if ( len_alpha_id NEQ 0 )
  {
    srcId_cb = srcId;
    utl_chsetToSim ((UBYTE*)alpha_id, len_alpha_id, (UBYTE*)cvtdText.data,
                     &lenCvtd, GSM_ALPHA_Def);
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

  /* when the text and the number are both empty, set p_text to NULL */
  if (lenCvtd EQ 0 AND p_number EQ NULL)
  {
    p_text = NULL;
  }
  else
  {
    p_text = &cvtdText;
    cvtdText.len = (UBYTE)lenCvtd;
  }

  /* write number into cphs mailbox number phonebook */
  ret = sAT_PercentCPMBW((T_ACI_CMD_SRC)srcId, rec_id, number, &toa, p_text);
  
  switch (ret)
  {
  case (AT_CMPL):                         /*operation completed*/
    break;
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_CPMBW;
    break;
  default:
    cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
    break;
  }
  
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPMBW     |
+--------------------------------------------------------------------+

  PURPOSE : %CPMB: test command 
*/

GLOBAL T_ATI_RSLT tesatPercentCPMBW(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char           *me = "%CPMBW: ";
  SHORT           first_idx = 0;
  SHORT           last_idx  = 0;  
  UBYTE           tlength   = 0;
  UBYTE           nlength   = 0;
  T_ACI_RETURN    ret = AT_FAIL;
#endif

  TRACE_FUNCTION("tesatPercentCPMBW");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_TST_PERCENT_CPMBW;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_tst_percent_cpmbw = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else
  ret = tAT_PercentCPMBW((T_ACI_CMD_SRC)srcId,&first_idx,&last_idx,&nlength,&tlength);

  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s(%d-%d),%d,%s,%d",me,first_idx,last_idx,nlength,"(128-201)",tlength);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPCFU     |
+--------------------------------------------------------------------+

  PURPOSE : %CPCFU: call diverted flag status
*/
typedef T_ACI_RETURN T_QUERY_INDICATOR_CMH_CPHS_FNC (T_ACI_CMD_SRC srcId, UBYTE *cfu_set, T_CPHS_LINES line);

LOCAL T_ATI_RSLT query_indicator_flags(UBYTE srcId, T_CPHS_LINES queried_lines, T_ACI_AT_CMD indicator_type)
{
  T_ACI_RETURN ret;
  UBYTE        flag_set;
  T_CPHS_LINES line             = CPHS_LINE_NULL,
               set_line_bitmask = CPHS_LINE_NULL;
  USHORT       i;
  T_QUERY_INDICATOR_CMH_CPHS_FNC *cmh_query_ind_fnc = NULL;
  CHAR         *cmd_name = NULL;

  switch(indicator_type)
  {
  case(AT_CMD_CPCFU):
    cmh_query_ind_fnc = qAT_PercentCPCFU;
    cmd_name = "%CPCFU";
    break;

  case(AT_CMD_CPVWI):
    cmh_query_ind_fnc = qAT_PercentCPVWI;
    cmd_name = "%CPVWI";
    break;
  }

  /* Provides flags for queried lines */
  for(i=0; i<8*sizeof(T_CPHS_LINES); i++)
  {
    line = CPHS_LINE1 << i;

    if ((line & queried_lines) EQ 0x00)
    {
      /* line has not been queried */
      continue;
    }

    ret = cmh_query_ind_fnc((T_ACI_CMD_SRC)srcId, &flag_set, line)/*lint -e613*/;

    if (ret NEQ AT_CMPL)
    {
      TRACE_EVENT_P1("query_indicator_flags error: %d", ret);
      return(map_aci_2_ati_rslt(ret));
    }

    if (flag_set EQ CPHS_FLAG_ACTIVATED)
    {
      set_line_bitmask |= line;
    }
  }

  if (set_line_bitmask NEQ CPHS_LINE_NULL)
  {
    sprintf(g_sa, "%s: %d,%d", cmd_name, CPHS_FLAG_ACTIVATED, set_line_bitmask);
  }
  else
  {
    sprintf(g_sa, "%s: %d",cmd_name, CPHS_FLAG_DEACTIVATED);
  }

  io_sendMessage((UBYTE)srcId, g_sa, ATI_NORMAL_OUTPUT);

  return(ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPCFU     |
+--------------------------------------------------------------------+

  PURPOSE : %CPVWI: set/clear/query CFU flags
*/

GLOBAL T_ATI_RSLT setatPercentCPCFU(CHAR *cl, UBYTE srcId)
{
  T_CPHS_LINES     i, 
                   typed_lines  = NOT_PRESENT_16BIT,
                   line;
  SHORT            action;
  T_ACI_RETURN     ret;
  T_ATI_SRC_PARAMS *src_params = find_element( ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl,"rr", &action, &typed_lines);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return(ATI_FAIL);
  }

  if (typed_lines EQ NOT_PRESENT_16BIT)
  {
    TRACE_EVENT("typed_lines omitted");

    typed_lines = 0;

    /* lines parameter omitted: Provides CFU flags for all lines */
    for(i=0;i<8*sizeof(T_CPHS_LINES);i++)
    {
      line = CPHS_LINE1 << i;

      if (cphs_line_makes_sense(line))
      {
        typed_lines |= line;
      } 
    }
  }

  TRACE_EVENT_P2("setatPercentCPCFU action: %d, lines: %d", action, typed_lines);

  #ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cpcfu my_bat_set_percent_cpcfu;

  TRACE_FUNCTION("setatPercentCPCFU() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cpcfu, 0, sizeof(my_bat_set_percent_cpcfu));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPCFU;
  cmd.params.ptr_set_percent_cpcfu = &my_bat_set_percent_cpcfu;

  my_bat_set_percent_cpcfu.mode = action;
  my_bat_set_percent_cpcfu.line = typed_lines;
  src_params->curAtCmd = AT_CMD_CPCFU;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  /******* QUERY ********/
  if (action EQ CPHS_QUERY_CFU_FLAG)
  {
    return query_indicator_flags(srcId, typed_lines, AT_CMD_CPCFU);    
  }
  
  TRACE_FUNCTION("setatPercentCPCFU()");

  /******* SETTING ********/
  ret = sAT_PercentCPCFU((T_ACI_CMD_SRC)srcId, (UBYTE)action, typed_lines);

  switch (ret)
  {
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_CPCFU;
    break;
  
  case (AT_BUSY):
    cmdCmeError(CME_ERR_SimBusy);
    return(ATI_FAIL);

  case (AT_FAIL):
    cmdCmeError(CME_ERR_Unknown);
    return(ATI_FAIL);
  }

  return(map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}


#endif /* CPHS_C */




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
|  Purpose :  AT Command: CONDAT specific commands.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_CSC_C
#define ATI_CSC_C

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

#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_GPF_TCPIP) || defined (FF_SAT_E)
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
#include "wap_aci.h"
#include "psa_ppp_w.h"
#endif   /* FF_WAP || FF_PPP || FF_GPF_TCPIP || FF_SAT_E */

#include "aci_mem.h"
#include "ati_cmd.h"
#include "aci_lst.h"
#include "aci_prs.h"

#include "psa_mmi.h"

#include "ati_int.h"

/* only needed for testing AT%DAR=5 */
/*
#include "aci.h"
*/

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */
EXTERN UBYTE std;

typedef struct
{
  char *name;
  T_ACI_CUSCFG_FAC fac;
} cuscfg_fac;

const cuscfg_fac cus_fac[] = 
{
  {"CCMOSM", CUSCFG_FAC_MO_SM_Control},
  {"CCMOC", CUSCFG_FAC_MO_Call_Control},
  {"CCMOSS", CUSCFG_FAC_MO_SS_Control},
  {"CCMOUSSD", CUSCFG_FAC_MO_USSD_Control},
  {"2BDIAL", CUSCFG_FAC_2_Digit_Call},
  {"EUSSDR", CUSCFG_FAC_Ext_USSD_Res},
  {"ETMOBEONSR", CUSCFG_FAC_T_MOBILE_Eons},
  {"USSDASMOC", CUSCFG_FAC_USSD_As_MO_Call},
  {0, CUSCFG_FAC_Not_Present}
};

#define MAX_FAC_STR_LENGTH 20
#define MAX_VALUE_LENGTH   MAX_FAC_STR_LENGTH

 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS                MODULE  : CMH_MMIS                 |
| STATE   : code                  ROUTINE : atPercentCPRIM           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CPRIM AT command
            which is used to send a system primitive (CONFIG PRIMITIVE)
            to a entity or frame.
*/

GLOBAL T_ATI_RSLT atPercentCPRIM (char *cl, UBYTE srcId)
{
  char receiver_name[10];
  char config_str[80];
  T_HANDLE receiver_handle;

  TRACE_FUNCTION ("atPercentCPRIM()");

  *receiver_name  = '\0';
  *config_str     = '\0';
  
  cl = parse(cl, "ss", (LONG)sizeof (receiver_name), receiver_name, 
                       (LONG)sizeof (config_str), config_str);

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    TRACE_EVENT ("atPercentCPRIM(): parse error");
    return (ATI_FAIL);
  }

  receiver_handle = vsi_c_open (VSI_CALLER receiver_name);
  if (receiver_handle < VSI_OK)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    TRACE_EVENT_P1 ("atPercentCPRIM(): wrong entity [%s]", receiver_name);
    return (ATI_FAIL);
  }

  psaMMI_ConfigPrim (receiver_handle, config_str);

  vsi_c_close (VSI_CALLER receiver_handle);

  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentBAND      |
+--------------------------------------------------------------------+

  PURPOSE : %BAND multiband configuration command
*/

GLOBAL T_ATI_RSLT setatPercentBAND (CHAR *cl, UBYTE srcId)
{
  T_ACI_BAND_MODE   bandMode;
  USHORT            ushort_bandType = 0xFFFF; /* ushort because of parser **##!!## */
  T_ACI_RETURN      ret             = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params      = find_element (ati_src_list, srcId, search_ati_src_id);
  
  src_params->curAtCmd = AT_CMD_BAND;
  
  cl = parse(cl,"dr",&bandMode,&ushort_bandType);
  if (cl EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  /* prechecking: only place where consistence can be checked (whether
     bandType parameter is present or not) */
  if( bandMode        EQ BAND_MODE_Manual AND
      ( (ushort_bandType & 0xFF00) NEQ 0x0000) )
  {
    TRACE_EVENT("Error: AT%%BAND manual mode but no bands chosen");
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  else if( bandMode        EQ  BAND_MODE_Auto AND
           ushort_bandType NEQ 0xFFFF )
  {
    TRACE_EVENT("Error: AT%%BAND auto mode but band type has been given: inconsistent");
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  TRACE_EVENT_P1("ushort_bandType: %04X", ushort_bandType);
  TRACE_EVENT_P1("bandMode: %d", bandMode);
  
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_band my_bat_set_percent_band;

  TRACE_FUNCTION("setatPercentBAND() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_band, 0, sizeof(my_bat_set_percent_band));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_BAND;
  cmd.params.ptr_set_percent_band = &my_bat_set_percent_band;

  my_bat_set_percent_band.mode = bandMode;
  my_bat_set_percent_band.band = ushort_bandType;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentBAND()");

  ret = sAT_PercentBAND((T_ACI_CMD_SRC)srcId, bandMode, (UBYTE)ushort_bandType);

  switch (ret)
  {
  case (AT_CMPL):
    break;

  case (AT_EXCT):               
    break;
  
  default:
    cmdCmeError(CME_ERR_Unknown);         
    break;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT*/
}

/* at the moment 5 levels of bands are supported */
/*     BITS:  |    5   |   4   |    3    |    2    |    1   |      
              | GSM850 | E_GSM | GSM1900 | GSM1800 | GSM900 |  */
#define HIGHEST_BAND_SUPPORTED (0x1F)

GLOBAL T_ATI_RSLT tesatPercentBAND (CHAR *cl, UBYTE srcId)
{
  T_ACI_BAND_MODE MaxBandMode;
  UBYTE           bands_bitfield;
  T_ACI_RETURN    ret = AT_FAIL;
  UBYTE           c_set_bit = 0;
  USHORT          pos       = 0;
  UBYTE           i, tested_filter_byte, highest_set_bit_only=0;

  TRACE_FUNCTION("tesatPercentBAND()");

  
  ret = tAT_PercentBAND( (T_ACI_CMD_SRC)srcId, 
                         &MaxBandMode,
                         &bands_bitfield);

  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_CMPL);
  }  

  /* initialization of the string */
  pos += sprintf(g_sa, "%s: (0-%d)", "%BAND", MaxBandMode);
  pos += sprintf( g_sa+pos, ",(" );

  if(bands_bitfield EQ 0x00)
  {
    /* no particular settings from manufacturer */
    pos += sprintf( g_sa+pos, "1-%d)", HIGHEST_BAND_SUPPORTED );
  }
  else
  {
    /* search holes between highest bit and lowest bit */
    for( i=0; i<8 ; i++ )
    {
      tested_filter_byte = 0x01 << i;
    
      if( (bands_bitfield & tested_filter_byte) )
      {
        highest_set_bit_only = tested_filter_byte;
        c_set_bit++;
      }
    }
    
    if(c_set_bit) /* alignment at the end */
    {
      c_set_bit--;
    }

    if( highest_set_bit_only EQ (0x01 << c_set_bit) )
    {
      /* there is no hole in the supported values, form 1 to max_value */
      pos += sprintf( g_sa+pos, "1-%d)", bands_bitfield );
    }
    else
    {
      for(i=1 ; i<=bands_bitfield; i++)  /* bands_bitfield is obviously the highest possible value */
      {
        if( (i & bands_bitfield) EQ i )
        { 
          pos += sprintf( g_sa+pos, "%d,", i);
        }
        if(pos>MAX_CMD_LEN-5) /* -5 to have some buffer */
        {
          TRACE_EVENT("output buffer is too short !!!");
          break;
        }
      }
      pos--; /* erase last comma */
      sprintf( g_sa+pos, ")");
    }  
  }       

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPercentBAND (CHAR *cl, UBYTE srcId)
{
  T_ACI_BAND_MODE bandMode;
  UBYTE           bandType;
  T_ACI_RETURN    ret = AT_FAIL;
  
  TRACE_FUNCTION("queatPercentBAND()");

  ret = qAT_PercentBAND((T_ACI_CMD_SRC)srcId, &bandMode, &bandType);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  if( bandType EQ 0x00
    OR bandMode EQ BAND_MODE_Auto )
  {
    sprintf(g_sa, "%s: %d", "%BAND", bandMode);
  }
  else
  {
    sprintf(g_sa, "%s: %d,%d", "%BAND", bandMode, bandType);
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentNRG       |
+--------------------------------------------------------------------+

  PURPOSE : %NRG extendet registration command
*/

GLOBAL T_ATI_RSLT setatPercentNRG (char *cl, UBYTE srcId)
{
  char op[MAX_ALPHA_OPER_LEN]={""};
  T_ACI_RETURN      ret        = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_ACI_NRG_RGMD    regMode    = NRG_RGMD_NotPresent;
  T_ACI_NRG_SVMD    srvMode    = NRG_SVMD_NotPresent;
  T_ACI_NRG_FRMT    oprFrmt    = NRG_FRMT_NotPresent;

  TRACE_FUNCTION("setatPercentNRG()");

  /* split parameter string in int,int,int,str */
  cl = parse(cl,"ddds",&regMode,&srvMode,&oprFrmt,(LONG)20,op);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PercentNRG((T_ACI_CMD_SRC)srcId,regMode,srvMode,oprFrmt,op);

  switch (ret)
  {
  case (AT_CMPL):                         /*operation completed*/
    break;
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_NRG;
    break;
  default:
    cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
    break;
  }
  return (map_aci_2_ati_rslt(ret));
}


GLOBAL T_ATI_RSLT queatPercentNRG (char *cl, UBYTE srcId)
{
  char op[MAX_ALPHA_OPER_LEN];
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_NRG_RGMD regMode= NRG_RGMD_NotPresent;
  T_ACI_NRG_SVMD srvMode= NRG_SVMD_NotPresent;
  T_ACI_NRG_SVMD srvStat= NRG_SVMD_NotPresent;
  T_ACI_NRG_FRMT oprFrmt= NRG_FRMT_NotPresent;

  TRACE_FUNCTION("queatPercentNRG()");

  ret = qAT_PercentNRG((T_ACI_CMD_SRC)srcId,&regMode,&srvMode,
                           &oprFrmt,&srvStat,op);
  if (ret EQ AT_CMPL)
  {
    if( *op )
      sprintf(g_sa,"%s: %d,%d,%d,%d,\"%s\"", "%NRG", regMode,srvMode,oprFrmt,srvStat,op);
    else
      sprintf(g_sa,"%s: %d,%d,,%d", "%NRG", regMode,srvMode,srvStat);

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);         /*command failed*/
  }
  return (map_aci_2_ati_rslt(ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCAOC      |
+--------------------------------------------------------------------+

  PURPOSE : %CAOC / %CACM command (Advice of Charge)
*/
LOCAL T_ATI_RSLT QueryCCM_ACM(CHAR *cl,T_ACI_AT_CMD cmd, UBYTE srcId)
{
  char         currency     [MAX_CUR_LEN]       = {0x00};
  char         cvtdCurrency [2*MAX_CUR_LEN]     = {0x00};
  USHORT       lenCvtdCurrency                  = 0;
  USHORT       pos                              = 0;
  char         ppu          [MAX_PPU_LENGTH]    = {0x00};
  T_ACI_RETURN ret = AT_FAIL;

  switch(cmd)
  {
  case(AT_CMD_CAOC ):
    /* 
     * AT%CAOC queries the actual ccm value using PUCT
     */
    ret = qAT_PercentCAOC((T_ACI_CMD_SRC)srcId, currency, ppu);
    pos = sprintf(g_sa,"%s: ","%CAOC");
    break;
  case(AT_CMD_CACM):
    /* 
     * AT%CACM requests the ACM using PUCT.
     */
    ret = qAT_PercentCACM((T_ACI_CMD_SRC)srcId,currency,ppu);
    pos = sprintf(g_sa,"%s: ","%CACM");
    break;
  }
  
  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  
  srcId_cb = srcId;
  utl_chsetFromGsm((UBYTE*)currency,
                   (USHORT)strlen(currency),
                   (UBYTE*)cvtdCurrency,
                   sizeof(cvtdCurrency),
                   &lenCvtdCurrency,
                   GSM_ALPHA_Def);
  pos+=sprints(g_sa+pos,cvtdCurrency,lenCvtdCurrency);
  pos+=sprintf(g_sa+pos,",\"%s\"",ppu);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (map_aci_2_ati_rslt(ret));
}


GLOBAL T_ATI_RSLT atPercentCAOC (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("atPercentCAOC()");

  return(QueryCCM_ACM(cl,AT_CMD_CAOC,srcId));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCACM      |
+--------------------------------------------------------------------+

  PURPOSE : %CACM command (Advice of Charge, Accumulated Call Meter)
*/
GLOBAL T_ATI_RSLT atPercentCACM (char* cl, UBYTE srcId)
{
  TRACE_FUNCTION("atPercentCACM()");

  return(QueryCCM_ACM(cl,AT_CMD_CACM,srcId));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCTV       |
+--------------------------------------------------------------------+

  PURPOSE : %CTV command (Current Call Timer Values)
*/
GLOBAL T_ATI_RSLT atPercentCTV (char* cl, UBYTE srcId)
{
  LONG  ctv;
  T_ACI_RETURN ret = AT_FAIL;
  
  TRACE_FUNCTION("atPercentCTV()");

  switch (*cl)
  {
    case('\0'):
      /* 
       * AT%CTV queries the call timer value
       */
      ret = qAT_PercentCTV((T_ACI_CMD_SRC)srcId, &ctv);
      if( ret NEQ AT_CMPL )
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      resp_disp(srcId, cl,"l",&ctv);
      break;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCPI       |
+--------------------------------------------------------------------+

  PURPOSE : %CPI command (Call proceeding indication)
*/

GLOBAL T_ATI_RSLT setatPercentCPI(char *cl, UBYTE srcId)
{
  SHORT mode = ACI_NumParmNotPresent;

  TRACE_FUNCTION("setatPercentCPI()");

  cl = parse(cl,"r",&mode);
  if(!cl OR mode > 4 OR mode < 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ati_user_output_cfg[srcId].CPI_stat=(UBYTE)mode;
  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPercentCPI(char *cl, UBYTE srcId)
{
  int   value;

  TRACE_FUNCTION("queatPercentCPI()");

  value = (int)ati_user_output_cfg[srcId].CPI_stat;
  resp_disp(srcId, cl,"e",&value);
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentVER       |
+--------------------------------------------------------------------+

  PURPOSE : %VER displays version of entities
*/

#if !defined (WIN32) AND !defined(_lint)
EXTERN CHAR* aci_version(void);
EXTERN CHAR* cc_version(void);
//EXTERN CHAR* ccd_version(void);
EXTERN CHAR* dl_version(void);
EXTERN CHAR* mm_version(void);
EXTERN CHAR* rr_version(void);
EXTERN CHAR* sim_version(void);
EXTERN CHAR* sms_version(void);
EXTERN CHAR* ss_version(void);
EXTERN CHAR* l1_version(void);
#ifdef SMI
EXTERN CHAR* smi_version(void);
#endif
#ifdef MFW
EXTERN CHAR* bmi_version(void);
EXTERN CHAR* mfw_version(void);
#endif
#ifdef FAX_AND_DATA
EXTERN CHAR* l2r_version(void);
EXTERN CHAR* ra_version(void);
EXTERN CHAR* rlp_version(void);
#ifdef FF_FAX
EXTERN CHAR* fad_version(void);
EXTERN CHAR* t30_version(void);
#endif /* FF_FAX */
#endif /* FAX_AND_DATA */
#endif

GLOBAL T_ATI_RSLT atPercentVER (char* cl, UBYTE srcId)
{
  char *me="%VER: ";
  
  TRACE_FUNCTION("atPercentVER()");
#if !defined (WIN32) AND !defined(_lint)
  sprintf(g_sa,"%s%s",me,aci_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,cc_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  //sprintf(g_sa,"%s%s",me,ccd_version());
  //io_sendMessage(g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,dl_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,mm_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,rr_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,sim_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,sms_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,ss_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,l1_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#ifndef NEW_FRAME
  sprintf(g_sa,"%s%s",me,comframe_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif
#ifdef SMI
  sprintf(g_sa,"%s%s",me,smi_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif
#ifdef MFW
  sprintf(g_sa,"%s%s",me,bmi_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,mfw_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif

#ifdef FAX_AND_DATA
  sprintf(g_sa,"%s%s",me,l2r_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,ra_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,rlp_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

#ifdef FF_FAX
  sprintf(g_sa,"%s%s",me,fad_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  sprintf(g_sa,"%s%s",me,t30_version());
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
#endif /* FF_FAX */

#endif /* FAX_AND_DATA */

#endif

  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCNAP      |
+--------------------------------------------------------------------+

  PURPOSE : %CNAP command (Calling NAme Presentation)
*/

GLOBAL T_ATI_RSLT setatPercentCNAP(char *cl, UBYTE srcId)
{
  T_ATI_CNAP_MODE cnap_mode;

  TRACE_FUNCTION("setatPercentCNAP()");

  cnap_mode =(T_ATI_CNAP_MODE)(*cl - '0');

  switch(cnap_mode)
  {
    case(CNAP_DISABLED):
    case(CNAP_ENABLED):
      ati_user_output_cfg[srcId].cnap_mode = cnap_mode; 
      cl++;
      return (ATI_CMPL);

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }
}

GLOBAL T_ATI_RSLT queatPercentCNAP(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("queatPercentCNAP()");

  ret = qAT_PercentCNAP((T_ACI_CMD_SRC)srcId);
  if( ret NEQ AT_EXCT )
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentALS       |
+--------------------------------------------------------------------+

  PURPOSE : %ALS command (select the aternate line)
*/


GLOBAL T_ATI_RSLT setatPercentALS ( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ACI_return = AT_FAIL;
  T_ACI_ALS_MOD ALSmode;

  TRACE_FUNCTION( "setatPercentALS()" );

  switch( *cl )
  {
    case '0':
      ALSmode = ALS_MOD_SPEECH;
      break;

    case '1':
      ALSmode = ALS_MOD_AUX_SPEECH;
      break;

    default:
      cmdCmeError(CME_ERR_OpNotSupp);
      return (map_aci_2_ati_rslt(ACI_return));
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_als my_bat_set_percent_als;

  TRACE_FUNCTION("setatPercentALS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_als, 0, sizeof(my_bat_set_percent_als));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_ALS;
  cmd.params.ptr_set_percent_als = &my_bat_set_percent_als;

  my_bat_set_percent_als.mode = ALSmode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  
  ACI_return = sAT_PercentALS( (T_ACI_CMD_SRC)srcId, ALSmode );
  
  if( ACI_return EQ AT_FAIL )
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ACI_return));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT tesatPercentALS ( char *cl, UBYTE srcId )
{
  char *me = "%ALS";
  T_ACI_RETURN ACI_return = AT_FAIL ;
  T_ACI_ALS_MOD ALSmode;

  TRACE_FUNCTION( "tesatPercentALS()" );

  ACI_return = tAT_PercentALS((T_ACI_CMD_SRC)srcId, &ALSmode );

  if ( ACI_return EQ AT_FAIL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  else if ( ACI_return EQ AT_CMPL )
  {
    if (ALSmode EQ ALS_MOD_NOTPRESENT)
    {
      cmdCmeError(CME_ERR_OpNotSupp);
      return (ATI_FAIL);
    }
    if (ALSmode EQ ALS_MOD_SPEECH)
      sprintf(g_sa,"%s: (0)", me);
    if (ALSmode EQ ALS_MOD_AUX_SPEECH)
      sprintf(g_sa,"%s: (1)", me);
    if (ALSmode EQ (ALS_MOD_SPEECH | ALS_MOD_AUX_SPEECH))
      sprintf(g_sa,"%s: (0,1)", me);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return (map_aci_2_ati_rslt(ACI_return));
}

  
GLOBAL T_ATI_RSLT queatPercentALS ( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ACI_return = AT_FAIL;
  T_ACI_ALS_MOD    ALSmode;
  UBYTE            ati_ALSmode;

  TRACE_FUNCTION( "queatPercentALS()" );
  
  ACI_return = qAT_PercentALS( (T_ACI_CMD_SRC)srcId, &ALSmode );
  if( ACI_return NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  switch( ALSmode )
  {
  case(ALS_MOD_SPEECH):
    ati_ALSmode = 0;
    break;

  case(ALS_MOD_AUX_SPEECH):
    ati_ALSmode = 1;
    break;

  case(ALS_MOD_NOTPRESENT):
  default:
    cmdCmeError(CME_ERR_OpNotSupp);
    return (ATI_FAIL);
  }

  resp_disp(srcId, cl,"b",&ati_ALSmode);
  return (map_aci_2_ati_rslt(ACI_return));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCOLR      |
+--------------------------------------------------------------------+

  PURPOSE : %COLR command 
            no parameter. 
*/

GLOBAL T_ATI_RSLT atPercentCOLR(CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("atPercentCOLR()");

  if( *cl NEQ '\0'
      AND *cl NEQ ';')
  {
    cmdCmeError( CME_ERR_OpNotAllow );
    return (ATI_FAIL);
  }
  
  ret = qAT_PercentCOLR( (T_ACI_CMD_SRC)srcId );

  if( ret NEQ AT_EXCT )
  {
    cmdCmeError( CME_ERR_Unknown );
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM                         MODULE  : ATI_CMD            |
| STATE   : code                        ROUTINE : atPercentCUNS      |
+--------------------------------------------------------------------+

  PURPOSE : %CUNS command 
            parameters: buffering_mode.
            0: unsolicited messages may occur any time.
            1: unsolicited messages can't happen while typing a command.
            2: unsolicited messages can't happen between starting typing a command
                               and its final result. 
*/

GLOBAL T_ATI_RSLT setatPercentCUNS(char *cl, UBYTE srcId)
{
  SHORT buffering_mode = -1;
  T_ATI_SRC_PARAMS *src_params;

  cl = parse(cl,"d",&buffering_mode);

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  if( cl EQ NULL
   OR src_params EQ NULL
   OR buffering_mode < 0
   OR buffering_mode > 2)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  src_params->buff_uns_mode = (T_ATI_BUFF_UNS_MODE)buffering_mode;
  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPercentCUNS(char *cl, UBYTE srcId)
{
  UBYTE buffering_mode = ((UBYTE)-1);  /* adjust is UBYTE type so typecasting -1 with UBYTE  */
  T_ATI_SRC_PARAMS *src_params;

  src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  if(src_params EQ NULL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  buffering_mode = (UBYTE)src_params->buff_uns_mode;
  resp_disp(srcId, cl, "b", &buffering_mode);
  return (ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentPPP       |
+--------------------------------------------------------------------+

  PURPOSE : %PPP command 
            parameters: protocol, login name, password. 
*/
#ifdef DTI
#if defined(FF_WAP) || defined(FF_PPP) || defined(FF_GPF_TCPIP) || defined(FF_SAT_E)
GLOBAL T_ATI_RSLT atPercentPPP (CHAR *cl, UBYTE srcId)
{
  T_ACI_PPP_PROT  protocol;
  T_ACI_RETURN    ret = AT_FAIL;
  CHAR            buffer_login[sizeof(pppShrdPrm.ppp_login)];
  CHAR            buffer_pwd[sizeof(pppShrdPrm.ppp_password)];
  T_ACI_PPP_CON   con_type = USE_NO_PPP_FOR_AAA;

  cl=parse(cl,"dssd",&protocol, sizeof(buffer_login),
                               &buffer_login,
                               sizeof(buffer_pwd),
                               &buffer_pwd,
                               &con_type);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_ppp my_bat_set_percent_ppp;
        
    TRACE_FUNCTION("atPercentPPP() calls bat_send() <=== as APPLICATION");

    memset(&my_bat_set_percent_ppp,0,sizeof(T_BAT_cmd_set_percent_ppp));
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_PPP;
    cmd.params.ptr_set_percent_ppp = &my_bat_set_percent_ppp;
    
    my_bat_set_percent_ppp.authprot = (T_BAT_percent_ppp_authprot)protocol;
    if(my_bat_set_percent_ppp.c_user = strlen(buffer_login))
    {
      my_bat_set_percent_ppp.v_user = TRUE;
      memcpy(my_bat_set_percent_ppp.user,buffer_login,
             my_bat_set_percent_ppp.c_user<=BAT_MAX_PPP_USER_LEN ?
             my_bat_set_percent_ppp.c_user : BAT_MAX_PPP_USER_LEN);
    }
    if(my_bat_set_percent_ppp.c_password = strlen(buffer_pwd))
    {
      my_bat_set_percent_ppp.v_password = TRUE;
      memcpy(my_bat_set_percent_ppp.password,buffer_pwd,
             my_bat_set_percent_ppp.c_password<=BAT_MAX_PPP_PASSW_LEN ?
             my_bat_set_percent_ppp.c_password : BAT_MAX_PPP_PASSW_LEN);
    }
    my_bat_set_percent_ppp.conn_type = (T_BAT_percent_ppp_conn_type)con_type;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }

#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("atPercentPPP()");

  ret = sAT_PercentPPP((T_ACI_CMD_SRC)srcId,protocol,
                                   buffer_login,
                                   buffer_pwd,
                                   con_type);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* FF_ATI_BAT */  
}


/** Query PPP-negotiated parameters after connection setup.
 *
 */
GLOBAL T_ATI_RSLT queatPercentPPP (CHAR *cl, UBYTE srcId)
{
  ULONG ipaddr = 0 ;            /* Own IP address. */
  ULONG dns1 = 0, dns2 = 0 ;    /* Domain Name Server addresses. */

  qAT_PercentPPP(srcId, &ipaddr, &dns1, &dns2) ;
  
  /* WARNING: this is for little endian only! */
#define IPADDR_BYTE(b, a) (((a) >> ((b) * 8)) & 0xff)

  sprintf(g_sa, "%%PPP: %u.%u.%u.%u,%u.%u.%u.%u,%u.%u.%u.%u",
          IPADDR_BYTE(3, ipaddr),
          IPADDR_BYTE(2, ipaddr),
          IPADDR_BYTE(1, ipaddr),
          IPADDR_BYTE(0, ipaddr),
          IPADDR_BYTE(3, dns1),
          IPADDR_BYTE(2, dns1),
          IPADDR_BYTE(1, dns1),
          IPADDR_BYTE(0, dns1),
          IPADDR_BYTE(3, dns2),
          IPADDR_BYTE(2, dns2),
          IPADDR_BYTE(1, dns2),
          IPADDR_BYTE(0, dns2)) ;
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL ;
}

#endif /* (FF_WAP) || (FF_PPP) || (FF_GPF_TCPIP) || (FF_SAT_E) */
#endif /* DTI */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentWAP       |
+--------------------------------------------------------------------+

  PURPOSE : %WAP command 
            no parameters. 
*/

#ifdef FF_WAP
GLOBAL T_ATI_RSLT atPercentWAP (CHAR *cl, UBYTE srcId)
{
  SHORT setwap_falg;

  TRACE_FUNCTION("atPercentWAP()");

  switch(*cl)
  {
  case('0'):
  case('1'):
    setwap_falg = ( (*cl - '0')? 1 : 0 );
    cl++;
    sAT_PercentWAP(srcId,setwap_falg);
    break;

  default:
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  return (ATI_CMPL);
}
#endif /* FF_WAP */


/*
+------------------------------------------------------------------------------
|  Function     :  SH - setatPercentMMITEST
+------------------------------------------------------------------------------
|  Description  :  Test AT cmd for MMI
| This command has been introduced in order to use the AT command interface for some MMI
| specific testing. It shoudnt be compiled without MMI.
|                   
|
|  Parameters   :  char  *cl
|                  UBYTE srcId
|
|  Return       : ATI_CMPL  
+------------------------------------------------------------------------------
*/


#if defined MFW AND defined TI_PS_FF_AT_P_CMD_MMITEST


GLOBAL T_ATI_RSLT setatPercentMMITEST (char *cl, UBYTE srcId)
{
  T_ACI_RETURN           ret;

  TRACE_FUNCTION("setatPercentMMITEST()");

  if (*cl NEQ '=') 
  {
   cmdCmeError (CME_ERR_OpNotAllow);
   return (ATI_FAIL);
  }

  cl++;
  
  ret = sAT_PercentMMITEST((T_ACI_CMD_SRC)srcId, cl);
  
  switch(ret)
  {
    case AT_CMPL:
      return (ATI_CMPL);

    case AT_FAIL:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);   

    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  } /* switch */
} /* setatPercentEM */
#endif /* MFW */

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)                     MODULE  : ACI_CMD          |
| STATE   : code                               ROUTINE : setatPercentSMBS |
+-------------------------------------------------------------------------+

  PURPOSE : %SMBS command (Enable presenting of both Text and PDU formats to SMBS)

  !!!!!!!!!! ONLY FOR TEST PURPOSES !!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#if defined FF_MMI_RIV
GLOBAL T_ATI_RSLT setatPercentSMBS (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN        ret  = AT_FAIL;
  T_ACI_PERC_SMBS_MOD mode = PERC_SMBS_MOD_NotPresent;

  TRACE_FUNCTION("setatPercentSMBS()");

  cl=parse(cl, "d", &mode);
  if (cl EQ NULL)
  {
    /* parse problems */
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
  }

  switch (mode)
  {
    case(PERC_SMBS_MOD_DISABLE):
    case(PERC_SMBS_MOD_ENABLE):
      break;

    default:
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
   }

   /* Parameter is allowed then set */
   ret = sAT_PercentSMBS (srcId, mode);
   if (ret EQ AT_FAIL)
   {
     cmdCmsError(CMS_ERR_UnSpecErr);
     return (ATI_FAIL);
   }
   return (map_aci_2_ati_rslt(ret));
}

/*  !!!!!!!!!! ONLY FOR TEST PURPOSES !!!!!!!!!!!!!!!!!!!!!!!!!! */
GLOBAL T_ATI_RSLT queatPercentSMBS (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_PERC_SMBS_MOD mode = PERC_SMBS_MOD_NotPresent;

  TRACE_FUNCTION("queatPercentSMBS()");

  ret = qAT_PercentSMBS(srcId, &mode);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d", "%SMBS: ", mode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  cmdCmsError(CMS_ERR_NotPresent);  // use aciErrDesc
  return (ATI_FAIL);
}
#endif /* FF_MMI_RIV */




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentDAR       |
+--------------------------------------------------------------------+

  PURPOSE : %DAR command 
*/


/* the following is from os.h derived for the AT%DAR command */
#define FIRST_ENTRY   0xff
#define NEXT_ENTRY    0xfe
#define OS_OK         0

GLOBAL T_ATI_RSLT setatPercentDAR (CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("atPercentDAR()");

  switch(*cl)
  {
    case('\0'):
      break;
    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);

/* this is only for internal testing of the %DAR command to force a crash of the stack */
/*
    case('1'):
      // do an endless loop within ACI
      while (TRUE)
        ;
      break;    // never reached

    case('2'):
      // consume all memory within ACI
      while (TRUE)
      {
        char *a;
        ACI_MALLOC(a, 10);
      }
      break;    // never reached

    case('3'):
      // jump to 0x00000000;
      {
        void (*a)(void)=0x00000000;
        (*a)();
      }
      break;    // never reached

    case('4'):
      // overwrite all memory
      memset(0x00000000, 0, 0xffffffff);
      break;    // never reached

    case('5'):
      // send many messages to ACI
      while (TRUE)
      {
        PALLOC(mncc_alert_ind, MNCC_ALERT_IND);
        mncc_alert_ind->ti = 0;
        PSENDX(ACI, mncc_alert_ind);
      }
      break;    // never reached

    case('6'):
      // generate many traces
      {
        int i;
        char buf[]="54 68 65 20 71 75 69 63-6B 20 62 72 6F 77 6E 20   The quick brown";
        for (i=0; TRUE; i++)
        {
          TRACE_EVENT_P2 ("%08X  %s", i, buf)
        }
      }
      break;    // never reached
*/
  
  }
  return (ATI_CMPL);
}



/*
GLOBAL T_ATI_RSLT queatPercentDAR (CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPercentDAR()");
  sprintf(g_sa, "Dummy");
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (ATI_CMPL);
}
*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : setatPercentCSTAT    | 
+--------------------------------------------------------------------+

  PURPOSE : %CSTAT command (Enable/Disable unsolicited status reports 
  from SIM processes)
*/

GLOBAL T_ATI_RSLT setatPercentCSTAT (char* cl, UBYTE srcId)
{
  SHORT mode = ACI_NumParmNotPresent;

  TRACE_FUNCTION("setatPercentCSTAT()");

  cl = parse(cl,"r",&mode);
  if(!cl OR mode > 1 OR mode < 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ati_user_output_cfg[srcId].CSTAT_stat=(UBYTE)mode;
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : queatPercentCSTAT    | 
+--------------------------------------------------------------------+

  PURPOSE : %CSTAT command (query unsolicited status reports 
  from SIM processes)
*/

GLOBAL T_ATI_RSLT queatPercentCSTAT (char* cl, UBYTE srcId)
{

  int   value;

  TRACE_FUNCTION("queatPercentCSTAT()");

  value = (int)ati_user_output_cfg[srcId].CSTAT_stat;
  resp_disp(srcId, cl,"e",&value);
  return (ATI_CMPL);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : tesatPercentNRG      | 
+--------------------------------------------------------------------+

  PURPOSE :  AT%NRG=? command 
*/
GLOBAL T_ATI_RSLT tesatPercentNRG (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  CHAR *cmd_key = "%NRG:";
  CHAR testcmd_output[25];
  T_ACI_NRG NRG_options;

  TRACE_FUNCTION("tesatPercentNRG()");

  ret = tAT_PercentNRG((T_ACI_CMD_SRC)srcId, &NRG_options);

  if(ret EQ AT_CMPL)
  {
    if(NRG_options.reg_mode)
    {
      strcpy(testcmd_output, "(");
      if(NRG_options.reg_mode & NRG_REG_Auto)
      {
        strcat(testcmd_output, "0");
      }
      if(NRG_options.reg_mode & NRG_REG_Manual)
      {
        strcat(testcmd_output, ",1");
      }
      if(NRG_options.reg_mode & NRG_REG_Both)
      {
        strcat(testcmd_output, ",4");
      }
      strcat(testcmd_output, ")");
    }
    
    if(NRG_options.srv_mode)
    {
      if(NRG_options.srv_mode EQ (NRG_SRV_Full | NRG_SRV_Limited | NRG_SRV_NoSrv | NRG_SRV_SetRegModeOnly))
      {
        strcat(testcmd_output, ",(0-3)");
      }
      else
      {
        strcat(testcmd_output, ",(");
        if(NRG_options.srv_mode & NRG_SRV_Full)
        {
          strcat(testcmd_output, "0");
        }
        if(NRG_options.srv_mode & NRG_SRV_Limited)
        {
          strcat(testcmd_output, ",1");
        }
        if(NRG_options.srv_mode & NRG_SRV_NoSrv)
        {
          strcat(testcmd_output, ",2");
        }
        if(NRG_options.srv_mode & NRG_SRV_SetRegModeOnly)
        {
          strcat(testcmd_output, ",3");
        }
        strcat(testcmd_output, ")");
      }

    }

    if(NRG_options.opr_frmt)
    {
      if(NRG_options.opr_frmt EQ (NRG_OPR_Long | NRG_OPR_Short | NRG_OPR_Numeric))
      {
        strcat(testcmd_output, ",(0-2)");
      }
      else
      {
        strcat(testcmd_output, ",(");
        if(NRG_options.opr_frmt & NRG_OPR_Long)
        {
          strcat(testcmd_output, "0");
        }
        if(NRG_options.opr_frmt & NRG_OPR_Short)
        {
          strcat(testcmd_output, ",1");
        }
        if(NRG_options.opr_frmt & NRG_OPR_Numeric)
        {
          strcat(testcmd_output, ",2");
        }
        strcat(testcmd_output, ")");
      }

    }
    sprintf(g_sa,"%s %s",cmd_key, testcmd_output);
    io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
    return(ATI_CMPL);
  }
  return(map_aci_2_ati_rslt(ret));
}

#ifdef TI_PS_FF_AT_P_CMD_CUSCFG
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : setatPercentCSTAT    | 
+--------------------------------------------------------------------+

  PURPOSE : %CUSCFG command (Enable/Disable customized handling of facilities specified)
*/

GLOBAL T_ATI_RSLT setatPercentCUSCFG (char* cl, UBYTE srcId)
{
#ifdef _SIMULATION_
  T_ACI_RETURN ret   = AT_FAIL;
  T_ACI_CUSCFG_FAC fac_num = CUSCFG_FAC_Not_Present;
  T_ACI_CUSCFG_MOD mode;
  T_ACI_CUSCFG_STAT status;
  char fac_str[MAX_FAC_STR_LENGTH] = {0};
  char value[MAX_VALUE_LENGTH] = {0};
  USHORT              i;
  SHORT               pos;
#endif /* _SIMULATION_ */

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentCUSCFG()");

  /*
   * The command being added for a customer should be blocked from other users
   * shall be used only for Windows simulation
   */
#ifdef _SIMULATION_

  cl = parse(cl,"sds",(LONG)MAX_FAC_STR_LENGTH,fac_str, &mode, (LONG)MAX_VALUE_LENGTH, value);
  
  if(!cl OR *fac_str EQ 0 OR mode < 0 OR mode >2)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  strcpy(fac_str, strupper(fac_str));
  for (i=0;cus_fac[i].name NEQ NULL; i++)
  {
    if (strcmp(cus_fac[i].name,fac_str) EQ 0 )
    {
      fac_num=cus_fac[i].fac;           /*translates facility string in facility number */
      break;
    }
  }

  if(fac_num EQ CUSCFG_FAC_Not_Present)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if(mode EQ CUSCFG_MOD_Query)
  {
    ret = qAT_PercentCUSCFG(srcId, fac_num, &status);
    if(ret EQ AT_CMPL)
    {
      pos = sprintf(g_sa,"%s: %d", "%CUSCFG", status);

      ci_remTrailCom(g_sa, pos);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return (ATI_CMPL);
    }
      
  }
  else
  {
     ret = sAT_PercentCUSCFG(srcId, fac_num, mode, value);
  }

  if(ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  
  return (map_aci_2_ati_rslt(ret));

#else
  cmdCmeError(CME_ERR_OpNotAllow);
  return (ATI_FAIL);
#endif

}
#endif /* TI_PS_FF_AT_P_CMD_CUSCFG */

#ifdef TI_PS_FF_AT_P_CMD_STDR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentSTDR   |
+--------------------------------------------------------------------+

  PURPOSE : %STDR  Read variable <std>
*/

GLOBAL T_ATI_RSLT setatPercentSTDR (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  UBYTE         rvstd;

  TRACE_FUNCTION("setatPercentSTDR()");

  switch (*cl)
  {
    case('\0'):
      /* 
       * AT%STDR queries the value of <std> variable
       */
      ret = qAT_PercentSTDR ((T_ACI_CMD_SRC)srcId, &rvstd);
      if( ret NEQ AT_CMPL )
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      resp_disp(srcId, cl,"b",&rvstd);
      break;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
}
#endif /* TI_PS_FF_AT_P_CMD_STDR */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCMGMDU |
+--------------------------------------------------------------------+

  PURPOSE : %CMGMDU  Reset the TP-Reply path
*/

GLOBAL T_ATI_RSLT setatPercentCMGMDU (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  SHORT idx=-1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentCMGMDU()");

  cl = parse(cl,"r",&idx);
  if ( !cl OR idx > 255 OR idx < 0)
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }
#ifdef _CONC_TESTING_
  ret = sAT_PercentCMGMDU (srcId, (UBYTE)idx);
#else
  ret = sAT_PercentCMGMDU_Gl((T_ACI_CMD_SRC)srcId, (UBYTE)idx, NULL);
#endif

  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_P_CMGMDU;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);
    return (ATI_FAIL);
  }
}

#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCTREG  |
+--------------------------------------------------------------------+

  PURPOSE : %CTREG  Sets or Reads the any one of the two tables
             present in the Common shared location.
            present in the RR enity. Usage of %CTREG command is limited
            and cannot be used during loc update. The best time to update
            is before +CFUN=1.
*/

GLOBAL T_ATI_RSLT setatPercentCTREG (CHAR *cl, UBYTE srcId)
{
  UBYTE table[MAX_CTREG_TAB_LEN*2 + 1];
  USHORT tab_len  = 0;
  UBYTE i = 0;
  T_ACI_RETURN  ret = AT_FAIL;
  T_TREG treg;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  /* initialize the local varibles */
  memset(table,NOT_PRESENT_8BIT,sizeof(table));

  TRACE_FUNCTION("setatPercentCTREG()");

  cl = parse(cl,"dds",&treg.mode,&treg.tab_id,sizeof(table),table);

  /*
   * no mode & tab_id values are allowed other than 1-2
   */
  if( !cl OR (treg.mode < TREG_READ_MODE OR treg.mode > TREG_WRITE_MODE)
          OR (treg.tab_id < NOSERVICE_MODE_TIME OR treg.tab_id > LIMSERVICE_MODE_TIME))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if(treg.mode EQ TREG_READ_MODE)
  {
    /*
     * If User gives Table values in read mode then Error is send to the terminal.
     */
    if(*table NEQ NOT_PRESENT_8BIT)
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      return ATI_FAIL;
    }

    ret = qAT_PercentCTREG ((T_ACI_CMD_SRC)srcId,&treg);

    i=sprintf(g_sa,"%s: %d, %d, \"","%CTREG",treg.mode, treg.tab_id);
    utl_binToHex(treg.tab_val,MAX_CTREG_TAB_LEN,g_sa+i);
    sprintf(g_sa+strlen(g_sa),"\"");

    if(ret EQ AT_CMPL)
    {
      io_sendMessageEx(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
    else
    {
      cmdCmeError (CME_ERR_Unknown);
    }
  }
  else
  {
    tab_len = utl_HexStrToBin((UBYTE*)table, (MAX_CTREG_TAB_LEN*2),
                                treg.tab_val, MAX_CTREG_TAB_LEN);
  
    if(tab_len NEQ MAX_CTREG_TAB_LEN)
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
    }

    ret = sAT_PercentCTREG((T_ACI_CMD_SRC)srcId,&treg);

    if (ret NEQ AT_CMPL)
    {
      cmdCmeError (CME_ERR_Unknown);      
    }
  }
  return (map_aci_2_ati_rslt(ret));
}
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#endif /* ATI_CSC_C */

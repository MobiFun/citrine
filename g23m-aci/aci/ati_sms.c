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
|  Purpose :  AT Command Interpreter: SMS related functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_SMS_C
#define ATI_SMS_C

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

#include "aci_mem.h"

#include "aci_lst.h"
#include "conc_sms.h"
#include "aci_prs.h"

#include "ati_int.h"

#include "cmh_sms.h"
#include "psa.h"
#include "psa_sms.h"
#include "aci.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

//TISH modified for MSIM
#undef WIN32

EXTERN T_SMS_SHRD_PRM smsShrdPrm;

const SMS_Memory sms_mem [] =
{
  {"ME",  SMS_STOR_Me},
  {"SM",  SMS_STOR_Sm},
  {0,SMS_STOR_Me}
};

#if defined (SMS_PDU_SUPPORT)
LOCAL T_ATI_RSLT atPlusCMGSText (char *cl, UBYTE srcId);
LOCAL T_ATI_RSLT atPlusCMGWText (char *cl, UBYTE srcId);
LOCAL T_ATI_RSLT atPlusCMGCText (char *cl, UBYTE srcId);
LOCAL T_ATI_RSLT atPlusCNMAText (char *cl, UBYTE srcId);
#endif

/* move the define into aci_cmd.h */

GLOBAL void         cmd_clearCnmiBuf    (void);

LOCAL  T_CNMI_BUFFER    cnmiBuf;
GLOBAL BOOL             cnmiFlushInProgress   = FALSE;
GLOBAL S16              waitForCnmaFromBuffer_SrcId = CMD_SRC_NONE;
LOCAL  CHAR daBuf[MAX_SMS_NUM_LEN*4];  /* multyply by 4 is required in case of UCS2 charecters */

EXTERN SHORT  cmhSMS_getPrfRge ( void );
EXTERN BOOL check_str(char * string,char * sample);

#ifdef _SIMULATION_
GLOBAL char subBuf[MAX_SUBADDR_LEN*4];
#endif /* _SIMULATION_ */

GLOBAL SHORT  toda_val;
GLOBAL SHORT  fo, ct, pid, mn;

GLOBAL UBYTE  cpmsCallType = NONE_CALL;

/*
 *
 ----------------------------SMS Commands-----------------------------------
 *
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSMS         |
+--------------------------------------------------------------------+

  PURPOSE : +CSMS command (Select Message Service)
*/

GLOBAL T_ATI_RSLT atPlusCSMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CSMS_SERV service;

  TRACE_FUNCTION("atPlusCSMS()");

  /*input functionality*/
  cl = parse (cl,"d",&service);
  if ( !cl )
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }
  ret = sAT_PlusCSMS((T_ACI_CMD_SRC)srcId,service);
  if (ret EQ AT_FAIL)
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPlusCSMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CSMS_SERV service;
  T_ACI_CSMS_SUPP mt;
  T_ACI_CSMS_SUPP mo;
  T_ACI_CSMS_SUPP bm;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  ret = qAT_PlusCSMS((T_ACI_CMD_SRC)srcId,&service,&mt,&mo,&bm);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+CSMS: %d,%d,%d,%d",service,mt,mo,bm);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_CSMS;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPMS         |
+--------------------------------------------------------------------+

  PURPOSE : +CPMS command (Select Preferred Message Storage)
*/

GLOBAL T_ATI_RSLT atPlusCPMS (char *cl, UBYTE srcId)
{
  CHAR               memstr1[3] = {0};
  CHAR               memstr2[3] = {0};
  CHAR               memstr3[3] = {0};
  T_ACI_RETURN       ret = AT_FAIL;
  T_ACI_SMS_STOR_OCC mem1;
  T_ACI_SMS_STOR_OCC mem2;
  T_ACI_SMS_STOR_OCC mem3;
  BOOL               f1         = TRUE;
  BOOL               f2         = TRUE;
  BOOL               f3         = TRUE;
  SHORT              i;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCPMS()");

  mem1.mem = SMS_STOR_NotPresent;
  mem2.mem = SMS_STOR_NotPresent;
  mem3.mem = SMS_STOR_NotPresent;

  /*input functionality*/
  cl = parse ( cl, "sss", (LONG)3, memstr1,
                          (LONG)3, memstr2,
                          (LONG)3, memstr3 );

  strupper ( memstr1 );
  strupper ( memstr2 );
  strupper ( memstr3 );

  for ( i=0; sms_mem[i].name NEQ NULL; i++ )
  {
    if ( ( strcmp ( sms_mem[i].name, memstr1 ) EQ 0 ) AND f1 )
    {
      mem1.mem = sms_mem[i].stor;
      f1       = FALSE;
    }

    if ( ( strcmp ( sms_mem[i].name, memstr2 ) EQ 0 ) AND f2 )
    {
      mem2.mem = sms_mem[i].stor;
      f2       = FALSE;
    }

    if ( ( strcmp ( sms_mem[i].name, memstr3 ) EQ 0 ) AND f3 )
    {
      mem3.mem = sms_mem[i].stor;
      f3       = FALSE;
    }
  }

  if (!cl OR f1 /*OR f2 OR f3*/ )
  {
    cmdCmsError ( CMS_ERR_OpNotAllowed );
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
{
  T_BAT_cmd_send     cmd;
  T_BAT_cmd_set_plus_cpms cpms;
  cmd.ctrl_params              = BAT_CMD_SET_PLUS_CPMS;
  cmd.params.ptr_set_plus_cpms = &cpms;

  cpms.mem1 = (T_BAT_plus_cpms_mem1)mem1.mem;
  cpms.mem2 = (T_BAT_plus_cpms_mem2)mem2.mem;
  cpms.mem3 = (T_BAT_plus_cpms_mem3)mem3.mem;
  
  TRACE_FUNCTION("atPlusCPMS() calls bat_send() <=== as APPLICATION");
  bat_send(ati_bat_get_client(srcId), &cmd);

  return (AT_EXCT);
}
#else /* FF_ATI_BAT */

  ret = sAT_PlusCPMS ( (T_ACI_CMD_SRC)srcId, mem1.mem, mem2.mem, mem3.mem );

  switch(ret)
  {
    case(AT_CMPL):
    {
      src_params->curAtCmd     = AT_CMD_NONE;
      return (ATI_CMPL);
    }
    default:
    {
      cmdCmsError (CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
    }
  }
  #endif /* FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCPMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN       ret;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  /*returns current Memory setting*/
  ret = qAT_PlusCPMS ((T_ACI_CMD_SRC)srcId );

  if ( ret EQ AT_CMPL )
  {
    src_params->curAtCmd     = AT_CMD_NONE;
    return (ATI_CMPL);
  }
  else if ( ret EQ AT_EXCT )
  {
    src_params->curAtCmd     = AT_CMD_CPMS;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError ( CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGF         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGF command (Select Message Format)
*/

GLOBAL T_ATI_RSLT atPlusCMGF (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CMGF_MOD mode=CMGF_MOD_NotPresent;

  TRACE_FUNCTION("atPlusCMGF()");

  /*
   * input functionality
   */
  cl = parse(cl,"d",&mode);
  if ( cl)
  {
    switch (mode)
    {
      case CMGF_MOD_Pdu:  /* PDU mode */
#if !defined (SMS_PDU_SUPPORT)
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
#else
         break;
#endif
       case CMGF_MOD_Txt:  /* Text Mode */
       default:
         break;
     }
   }
   else
   {
     /*
      * parse problems
      */
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }
   /*
    * Parameter is allowed then set
    */
   ret = sAT_PlusCMGF ((T_ACI_CMD_SRC)srcId,mode);
   if (ret EQ AT_FAIL)
   {
     cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
     return (ATI_FAIL);
   }
   return (map_aci_2_ati_rslt(ret));
}

GLOBAL T_ATI_RSLT queatPlusCMGF (char *cl, UBYTE srcId)
{
  char *me="+CMGF: ";
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CMGF_MOD mode=CMGF_MOD_NotPresent;

  TRACE_FUNCTION("queatPlusCMGF()");

  /*
   * returns current message format PDU/TEXT
   */
  ret = qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d",me,mode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSCA         |
+--------------------------------------------------------------------+

  PURPOSE : +CSCA command (Select Message Service Centre)
*/

GLOBAL T_ATI_RSLT atPlusCSCA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_TOA     tosca;
  T_ACI_TOA    *p_tosca;
  SHORT         octet=0;
  CHAR          sca[MAX_SMS_NUM_LEN];

  memset (sca, 0, sizeof(sca));
  p_tosca=&tosca;

  TRACE_FUNCTION("atPlusCSCA()");

  /*input functionality*/
  cl = parse (cl,"sr",(LONG)MAX_SMS_NUM_LEN,sca,&octet);
  if ( !cl OR octet > 0xC9 OR sca[0] EQ '\0')
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
  if(octet EQ 0)
    p_tosca=NULL;
  else
  {
    tosca=toa_demerge(octet);
    if (tosca.ton < 0 OR tosca.npi < 0)
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
  }
#ifdef FF_ATI_BAT
{
  T_BAT_cmd_send     cmd;
  T_BAT_cmd_set_plus_csca csca;
  
  cmd.ctrl_params              = BAT_CMD_SET_PLUS_CSCA;
  cmd.params.ptr_set_plus_csca = &csca;

  csca.c_sca = strlen(sca);  
  memcpy(csca.sca, sca, csca.c_sca);
  csca.tosca = octet;  

  TRACE_FUNCTION("atPlusCSCA() calls bat_send() <=== as APPLICATION");
  bat_send(ati_bat_get_client(srcId), &cmd);

  return (AT_EXCT);
}
#else /* FF_ATI_BAT */
 
  ret = sAT_PlusCSCA((T_ACI_CMD_SRC)srcId,sca,p_tosca);
  if (ret EQ AT_FAIL)
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCSCA (char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  T_BAT_cmd_send     cmd;
  T_BAT_no_parameter dummy;
  
  cmd.ctrl_params              = BAT_CMD_QUE_PLUS_CSCA;
  dummy.bat_dummy              = 0xFF;
  cmd.params.ptr_que_plus_csca = &dummy;

  TRACE_FUNCTION("queatPlusCSCA() calls bat_send() <=== as APPLICATION");
  bat_send(ati_bat_get_client(srcId), &cmd);

  return (AT_EXCT);

#else /* FF_ATI_BAT */

  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_TOA     tosca;
  SHORT         octet=0;
  CHAR          sca[MAX_SMS_NUM_LEN];

  ret = qAT_PlusCSCA ((T_ACI_CMD_SRC)srcId,sca,&tosca);
  if (ret EQ AT_CMPL)
  {
    octet=toa_merge(tosca);
    sprintf(g_sa, "+CSCA: \"%s%s\",%d", (tosca.ton EQ TON_International)?"+":"", sca, octet);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_decodeVpabs    |
+--------------------------------------------------------------------+

  PURPOSE : This function decodes a given string to the T_ACI_VP_ABS
            enumeration type.
*/
LOCAL BOOL aci_decodeVpabs ( CHAR*         vpabs_str,
                             T_ACI_VP_ABS* vpabs_enum )
{
  if ( strlen( vpabs_str ) NEQ 20 OR

       vpabs_str[2]  NEQ '/' OR vpabs_str[5]  NEQ '/' OR
       vpabs_str[8]  NEQ ',' OR vpabs_str[11] NEQ ':' OR
       vpabs_str[14] NEQ ':' OR

       ( vpabs_str[17] NEQ '+' AND vpabs_str[17] NEQ '-' ) OR

       vpabs_str[18] < '0' OR vpabs_str[18] > '9' OR
       vpabs_str[19] < '0' OR vpabs_str[19] > '9'             )

    return FALSE;

  vpabs_enum->year  [0] = vpabs_str[0]  - 0x30;
  vpabs_enum->year  [1] = vpabs_str[1]  - 0x30;

  vpabs_enum->month [0] = vpabs_str[3]  - 0x30;
  vpabs_enum->month [1] = vpabs_str[4]  - 0x30;

  vpabs_enum->day   [0] = vpabs_str[6]  - 0x30;
  vpabs_enum->day   [1] = vpabs_str[7]  - 0x30;

  vpabs_enum->hour  [0] = vpabs_str[9]  - 0x30;
  vpabs_enum->hour  [1] = vpabs_str[10] - 0x30;

  vpabs_enum->minute[0] = vpabs_str[12] - 0x30;
  vpabs_enum->minute[1] = vpabs_str[13] - 0x30;

  vpabs_enum->second[0] = vpabs_str[15] - 0x30;
  vpabs_enum->second[1] = vpabs_str[16] - 0x30;

  vpabs_enum->timezone  = (SHORT) atoi( &vpabs_str[17] );

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aci_decodeVpenh    |
+--------------------------------------------------------------------+

  PURPOSE : This function decodes a given string to the T_ACI_VP_ENH
            type.
*/
LOCAL BOOL aci_decodeVpenh ( CHAR*         vpenh_str,
                             T_ACI_VP_ENH* vpenh )
{
  UBYTE shift_byte = 0;

  if (vpenh_str[0] EQ '\0')
  {
    return FALSE;
  }

  /* functionality indicator */
  vpenh->func_ind = (vpenh_str[0]  - 0x30) << 4;
  vpenh->func_ind += (vpenh_str[1]  - 0x30);

  /* extension octet */
  if (vpenh->func_ind & TP_VPF_ENH_EXT_BIT_MASK)
  {
    shift_byte = 2;
    vpenh->ext_oct = (vpenh_str[2]  - 0x30) << 4;
    vpenh->ext_oct += (vpenh_str[3]  - 0x30);
  }

  if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) > TP_VPF_ENH_HRS)
  {
    return FALSE;
  }

  /* process validity period values */
  if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_REL)
  {
    utl_HexStrToBin ((UBYTE*)&vpenh_str[2+shift_byte], 2, &vpenh->val.vpenh_relative, 1);
  }
  else if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_SEC)
  {
    utl_HexStrToBin ((UBYTE*)&vpenh_str[2+shift_byte], 2, &vpenh->val.vpenh_seconds, 1);
  }
  else if ((vpenh->func_ind & TP_VPF_ENH_FORMAT_MASK) EQ TP_VPF_ENH_HRS)
  {
    vpenh->val.vpenh_hours.hour  [0] = vpenh_str[3+shift_byte] - 0x30;
    vpenh->val.vpenh_hours.hour  [1] = vpenh_str[2+shift_byte] - 0x30;

    vpenh->val.vpenh_hours.minute[0] = vpenh_str[5+shift_byte] - 0x30;
    vpenh->val.vpenh_hours.minute[1] = vpenh_str[4+shift_byte] - 0x30;

    vpenh->val.vpenh_hours.second[0] = vpenh_str[7+shift_byte] - 0x30;
    vpenh->val.vpenh_hours.second[1] = vpenh_str[6+shift_byte] - 0x30;
  }

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSMP         |
+--------------------------------------------------------------------+

  PURPOSE : +CSMP command (Select Text Mode Parameters)
*/

GLOBAL T_ATI_RSLT atPlusCSMP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret           = AT_FAIL;
  SHORT         fo            = ACI_NumParmNotPresent,
                vprel         = ACI_NumParmNotPresent,
                pid           = ACI_NumParmNotPresent,
                dcs           = ACI_NumParmNotPresent;
  USHORT        i             = 0;
  char          vpabs_str[25] = {'\0'},
                vpenh_str[15] = {'\0'},
                fo_str[4];

  T_ACI_VP_ABS  vpabs;
  T_ACI_VP_ABS* pVpabs        = NULL;
  T_ACI_VP_ENH  vpenh;

  TRACE_FUNCTION("atPlusCSMP()");

   /*input functionality*/
   while (*cl NEQ ',' AND *cl NEQ '\0' AND i < sizeof(fo_str) - 1)
   {
     fo_str[i]=*cl;
     cl++;
     i++;
   }
   if (*cl)
   {
     cl++;
   }
   fo_str[i]='\0';
   if (strlen (fo_str) > 0)
   {
     fo=atoi(fo_str);
   }
   if (fo > 255)
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return ATI_FAIL;
   }
   if (fo NEQ ACI_NumParmNotPresent AND
       (fo & TP_VPF_MASK) EQ TP_VPF_ABSOLUTE)
   {
     pVpabs = &vpabs;

     cl = parse(cl,"srr",(LONG)25,vpabs_str,&pid,&dcs);

     if (!cl OR strlen(vpabs_str) > 20 OR pid > 255 OR
         dcs > 255 OR !aci_decodeVpabs (vpabs_str, &vpabs))
     {
       cmdCmsError(CMS_ERR_OpNotAllowed);
       return (ATI_FAIL);
     }
   }
   else if (fo NEQ ACI_NumParmNotPresent AND
       (fo & TP_VPF_MASK) EQ TP_VPF_ENHANCED)
   {
     cl = parse(cl,"srr",(LONG)19,vpenh_str,&pid,&dcs);
     if (!cl OR strlen(vpenh_str) > 14 OR pid > 255 OR
         dcs > 255 OR !aci_decodeVpenh (vpenh_str, &vpenh))
     {
       cmdCmsError(CMS_ERR_OpNotAllowed);
       return (ATI_FAIL);
     }
   }
   else if (fo NEQ ACI_NumParmNotPresent AND
       (fo & TP_VPF_MASK) EQ TP_VPF_RELATIVE)
   {
     cl = parse(cl,"rrr",&vprel,&pid,&dcs);
   }
   else
   {
     if (*cl EQ '\"')
     {
       pVpabs = &vpabs;

       cl = parse(cl,"srr",(LONG)25,vpabs_str,&pid,&dcs);

       if (!cl OR strlen(vpabs_str) > 20 OR pid > 255 OR
           dcs > 255 OR !aci_decodeVpabs (vpabs_str, &vpabs))
       {
         cmdCmsError(CMS_ERR_OpNotAllowed);
         return (ATI_FAIL);
       }
     }
     else
     {
       cl = parse(cl,"rrr",&vprel,&pid,&dcs);

       if ( !cl OR vprel > 255 OR pid > 255 OR dcs > 255)
       {
         cmdCmsError(CMS_ERR_OpNotAllowed);
         return (ATI_FAIL);
       }
     }
   }
   ret = sAT_PlusCSMP((T_ACI_CMD_SRC)srcId,fo,vprel,pVpabs,&vpenh,pid,dcs);
   if (ret EQ AT_FAIL)
   {
     cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
     return (ATI_FAIL);
   }
   return (map_aci_2_ati_rslt(ret));
}

GLOBAL T_ATI_RSLT queatPlusCSMP (char *cl, UBYTE srcId)
{
  char*         me            = "+CSMP: ";
  T_ACI_RETURN  ret           = AT_FAIL;
  SHORT         fo            = ACI_NumParmNotPresent,
                vprel         = ACI_NumParmNotPresent,
                pid           = ACI_NumParmNotPresent,
                dcs           = ACI_NumParmNotPresent;
  char          vpabs_str[25] = {'\0'};
  char          vpenh_str[15] = {'\0'};
  T_ACI_VP_ABS  vpabs;
  T_ACI_VP_ENH  vpenh;

  TRACE_FUNCTION("queatPlusCSMP()");

  ret = qAT_PlusCSMP((T_ACI_CMD_SRC)srcId,&fo,&vprel,&vpabs,&vpenh,&pid,&dcs);
  if (ret EQ AT_CMPL)
  {
    if ((fo & TP_VPF_MASK) EQ TP_VPF_ABSOLUTE)   /*Bits 4,3 -> 11*/
    {
      sprintf(vpabs_str,"\"%d%d/%d%d/%d%d,%d%d:%d%d:%d%d%+03d\"",
              vpabs.year  [0], vpabs.year  [1],
              vpabs.month [0], vpabs.month [1],
              vpabs.day   [0], vpabs.day   [1],
              vpabs.hour  [0], vpabs.hour  [1],
              vpabs.minute[0], vpabs.minute[1],
              vpabs.second[0], vpabs.second[1],
              vpabs.timezone);
      sprintf(g_sa,"%s%d,%s,%d,%d",me,fo,vpabs_str,pid,dcs);
    }
    else if ((fo & TP_VPF_MASK) EQ TP_VPF_ENHANCED)
    {
      aci_encodeVpenh ( vpenh_str, &vpenh );
      sprintf(g_sa,"%s%d,\"%s\",%d,%d",me,fo,vpenh_str,pid,dcs);
    }
    else if ((fo & TP_VPF_MASK) EQ TP_VPF_RELATIVE) /*Bits 4,3 -> 10*/
    {
      sprintf(g_sa,"%s%d,%d,%d,%d",me,fo,vprel,pid,dcs);
    }
    else
    {
      sprintf(g_sa,"%s%d,,%d,%d",me,fo,pid,dcs);
    }
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSDH         |
+--------------------------------------------------------------------+

  PURPOSE : +CSDH command (Show Text Mode Parameters)
*/

GLOBAL T_ATI_RSLT atPlusCSDH (char *cl, UBYTE srcId)
{

  T_ACI_CSDH_SHOW ena = CSDH_SHOW_Disable;

  TRACE_FUNCTION("atPlusCSDH()");

   /*input functionality*/
   cl = parse(cl,"d",&ena);
   if ( !cl )
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }
   ati_user_output_cfg[srcId].CSDH_stat=ena;
   return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT tesatPlusCSDH (char *cl, UBYTE srcId)
{
  return(atPlusCSDH (cl, srcId));
}

GLOBAL T_ATI_RSLT queatPlusCSDH (char *cl, UBYTE srcId)
{
  sprintf(g_sa,"+CSDH: %d",ati_user_output_cfg[srcId].CSDH_stat);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSCB         |
+--------------------------------------------------------------------+

  PURPOSE : +CSCB command (select cell broadcast message type)
*/

GLOBAL T_ATI_RSLT atPlusCSCB (char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_CSCB_MOD  mode;
  char            mids_str[6*MAX_CBM_TYPES]={0};    /* CHECK IF LONG ENOUGH !!*/
  char            dcss_str[4*MAX_CBM_TYPES]={0};
  CHAR            buf[6]={0};
  USHORT          mids[MAX_CBM_TYPES];
  UBYTE           dcss[MAX_CBM_TYPES];
  SHORT           pos=0,i=0,j=0;

#ifdef FF_ATI_BAT
  T_BAT_cmd_set_plus_cscb cscb;
#endif


  TRACE_FUNCTION("atPlusCSCB()");

   /*input functionality*/
   cl = parse(cl,"dss",&mode,(LONG)sizeof(mids_str),mids_str,(LONG)sizeof(dcss_str),dcss_str);
   if ( !cl )
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }
   i = j = 0;
   memset (mids, NOT_PRESENT_8BIT, sizeof (mids));
   while (mids_str[i] NEQ '\0' AND j < MAX_CBM_TYPES - 1)
   {
     pos=0;
     while (mids_str[i] >= '0'  AND  mids_str[i] <= '9')
     {
       if (pos >= 5)
       {
         cmdCmsError(CMS_ERR_OpNotAllowed);
         return (ATI_FAIL);
       }
       buf[pos++]=mids_str[i];
       i++;
     }
     buf[pos]='\0';
     if (mids_str[i] NEQ ','  AND  mids_str[i] NEQ '-'  AND  mids_str[i] NEQ '\0')
     {
       cmdCmsError(CMS_ERR_OpNotAllowed);
       return (ATI_FAIL);
     }
     else
     {
       if (mids_str[i] EQ ',' OR mids_str[i] EQ '\0')
       {
         i++;
         mids[j++]=atoi(buf);
         mids[j++]=atoi(buf);
       }
       else
       {
         i++;
         mids[j++]=atoi(buf);
         pos=0;
         while (mids_str[i] NEQ ','  AND  mids_str[i] NEQ '\0')
         {
           if (pos >= 5)
           {
             cmdCmsError(CMS_ERR_OpNotAllowed);
             return (ATI_FAIL);
           }
           buf[pos++]=mids_str[i];
           i++;
         }
         i++;
         mids[j++]=atoi(buf);
       }
     }
   }
 
   if (compact(&mids_str[i],(USHORT)strlen(&mids_str[i])) NEQ 0)
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }

#ifdef FF_ATI_BAT
   memset(cscb.mids, NOT_PRESENT_8BIT, sizeof(cscb.mids));
   cscb.v_mids = j ? TRUE : FALSE;
   cscb.c_mids = (U8)j;
   memcpy(cscb.mids, mids, j*2);
#endif

   i = j = 0;
   memset (dcss, NOT_PRESENT_8BIT, sizeof (dcss));
   while (dcss_str[i] NEQ '\0' AND j < MAX_CBM_TYPES - 1)
   {
     pos=0;
     while (dcss_str[i] >= '0'  AND  dcss_str[i] <= '9')
     {
       if (pos >= 3)
       {
         cmdCmsError(CMS_ERR_OpNotAllowed);
         return (ATI_FAIL);
       }
       buf[pos++]=dcss_str[i];
       i++;
     }
     buf[pos]='\0';
     if (dcss_str[i] NEQ ','  AND  dcss_str[i] NEQ '-'  AND  dcss_str[i] NEQ '\0')
     {
       cmdCmsError(CMS_ERR_OpNotAllowed);
       return (ATI_FAIL);
     }
     else
     {
       if (dcss_str[i] EQ ',' OR dcss_str[i] EQ '\0')
       {
         i++;
         dcss[j++]=atoi(buf);
         dcss[j++]=atoi(buf);
       }
       else
       {
         i++;
         dcss[j++]=atoi(buf);
         pos=0;
         while (dcss_str[i] NEQ ','  AND  dcss_str[i] NEQ '\0')
         {
           if (pos >= 3)
           {
             cmdCmsError(CMS_ERR_OpNotAllowed);
             return (ATI_FAIL);
           }
           buf[pos++]=dcss_str[i];
           i++;
         }
         i++;
         dcss[j++]=atoi(buf);
       }
     }
   }
   if (compact(&dcss_str[i],(USHORT)strlen(&dcss_str[i])) NEQ 0)
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }
   
#ifdef FF_ATI_BAT
   {
      T_BAT_cmd_send cmd;

      cmd.ctrl_params = BAT_CMD_SET_PLUS_CSCB;
      cmd.params.ptr_set_plus_cscb = &cscb;

      cscb.mode = (T_BAT_VAL_plus_cscb_mode)mode;
            
      memset(cscb.dcss, NOT_PRESENT_8BIT, sizeof(cscb.dcss));
      cscb.v_dcss = j ? TRUE : FALSE;;
      cscb.c_dcss = (U8)j;
      memcpy(cscb.dcss, dcss, cscb.c_dcss);
      
      
      TRACE_FUNCTION("atPlusCSCB() calls bat_send() <=== as APPLICATION");
      bat_send(ati_bat_get_client(srcId), &cmd);
      return (AT_EXCT);
   }
#else /* FF_ATI_BAT */
   ret=sAT_PlusCSCB((T_ACI_CMD_SRC)srcId,mode,mids,dcss);
   if (ret EQ AT_FAIL)
   {
     cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
     return (ATI_FAIL);
   }
   return (map_aci_2_ati_rslt(ret));
#endif /* FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCSCB (char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  T_BAT_cmd_send     cmd;
  T_BAT_no_parameter dummy;
  
  cmd.ctrl_params              = BAT_CMD_QUE_PLUS_CSCB;
  dummy.bat_dummy              = 0xFF;
  cmd.params.ptr_que_plus_cscb = &dummy;

  TRACE_FUNCTION("queatPlusCSCB() calls bat_send() <=== as APPLICATION");
  bat_send(ati_bat_get_client(srcId), &cmd);

  return (AT_EXCT);

#else /* FF_ATI_BAT */
  T_ACI_CSCB_MOD  mode;
  char            mids_str[6*MAX_CBM_TYPES+1]={'\0'}; /* +1 for '\0' */  /* CHECK IF LONG ENOUGH !!*/
  char            dcss_str[4*MAX_CBM_TYPES+1]={'\0'};
  USHORT          mids[MAX_CBM_TYPES];
  UBYTE           dcss[MAX_CBM_TYPES];
  SHORT           pos=0,i=0;
  T_ACI_RETURN    ret = AT_FAIL;

  TRACE_FUNCTION("queatPlusCSCB()");

  ret = qAT_PlusCSCB((T_ACI_CMD_SRC)srcId, &mode,mids,dcss);
  if (ret EQ AT_CMPL)
  {
    /*
     *-----------------------------------------------------------
     * assemble the string for message identifiers
     *-----------------------------------------------------------
     */
    pos = i = 0;
    while( mids[i]   NEQ 0xFFFF   AND
           mids[i+1] NEQ 0xFFFF   AND
           i < MAX_CBM_TYPES - 1    )
    {
      if ( mids[i] EQ mids[i+1] )
        pos += sprintf ( mids_str + pos, "%d,", mids[i] );
      else
        pos += sprintf ( mids_str + pos, "%d-%d,", mids[i], mids[i+1] );  /* "65534-65535," max 12 chars */

      i += 2;
    }

    /*lint -e(661) -e(662) */   /* lint 7.5 has here some problems ... */
    if ( pos > 0 )              /* remove the last ',' */
    {
        mids_str[pos-1] = '\0';
    }

    /*
     *-----------------------------------------------------------
     * assemble the string for data coding schemes
     *-----------------------------------------------------------
     */
    pos = i = 0;
    while( dcss[i]   NEQ 0xFF    AND
           dcss[i+1] NEQ 0xFF    AND
           i < MAX_CBM_TYPES - 1     )
    {
      if ( dcss[i] EQ dcss[i+1] )
        pos += sprintf ( dcss_str + pos, "%d,", dcss[i] );
      else
        pos += sprintf ( dcss_str + pos, "%d-%d,", dcss[i], dcss[i+1] );

      i += 2;
    }

    /*lint -e(661) -e(662) */
    if ( pos > 0 )    /* remove the last ',' */
    {
      dcss_str[pos-1] = '\0';
    }

    sprintf(g_sa,"+CSCB: %d,\"%s\",\"%s\"",mode,mids_str,dcss_str);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */
}

#ifdef FF_HOMEZONE
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCBHZ      |
+--------------------------------------------------------------------+

  PURPOSE : %CBHZ command (set homezone activity)
*/

GLOBAL T_ATI_RSLT setatPercentCBHZ (char *cl, UBYTE srcId)
{
  T_ACI_CBHZ_MOD  mode    = CBHZ_MOD_NotPresent;
  T_ACI_CS        dcs     = CS_NotPresent;
  UBYTE           timeout = NOT_PRESENT_8BIT;
  T_ACI_RETURN    ret     = AT_FAIL;

  TRACE_FUNCTION("setatPercentCBHZ()");


  /*input functionality*/

  /*check first parameter */
  switch (*cl)
  {
    case '0':
    case '1':
      mode = (T_ACI_CBHZ_MOD)(*cl - 0x30);
      cl++;
      break;

    default:
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
  }

  /* check second parameter */
  if  ( *cl NEQ '\0' )
  {
    if ( *cl NEQ ',')
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    cl++;

    if ( (*cl >= '0') AND (*cl <= '9') )
    {
      dcs = (T_ACI_CS)(*cl -0x30);
      cl++;
    }

    /* check third parameter */
    if  ( *cl NEQ '\0' )
    {
      if ( *cl NEQ ',')
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
      cl++;

      cl = parse(cl,"d",&timeout);
      if  ( *cl NEQ '\0' )
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    }
  }

  /* parameters ok... process homezone request */
  ret = sAT_PercentCBHZ((T_ACI_CMD_SRC)srcId,mode,dcs,timeout);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (ATI_CMPL);
}


GLOBAL T_ATI_RSLT tesatPercentCBHZ (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPercentCBHZ()");

  sprintf( g_sa, "%s: (%d,%d),(%d-%d),(%d-%d)",
             "%CBHZ", CBHZ_MOD_NotActive, CBHZ_MOD_Active,
             CS_GsmDef, CS_Ascii, CBHZ_MIN_TIMEOUT, CBHZ_MAX_TIMEOUT);


  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT queatPercentCBHZ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret;
  T_ACI_CBHZ_MOD  mode;
  T_ACI_CS        dcs;
  UBYTE           timeout;

  TRACE_FUNCTION("queatPercentCBSHZ()");

  ret = qAT_PercentCBHZ((T_ACI_CMD_SRC)srcId, &mode, &dcs, &timeout);

  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  /*
   *-----------------------------------------------------------
   * assemble the string for output
   *-----------------------------------------------------------
   */

  sprintf( g_sa, "%s: %d,%d,%d", "%CBHZ", mode, dcs, timeout);

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}
#endif /* FF_HOMEZONE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSAS         |
+--------------------------------------------------------------------+

  PURPOSE : +CSAS command (Save SMS Profile)
*/

GLOBAL T_ATI_RSLT atPlusCSAS (char *cl, UBYTE srcId)
{
  SHORT        profile  = -1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCSAS()");

  if ( *cl EQ '\0' )
  {
    profile = 1;
  }
  else
  {
    cl = parse ( cl, "r", &profile );
    if ( !cl )
    {
      cmdCmsError ( CMS_ERR_MemFail );
      return (ATI_FAIL);
    }
    profile++;
  }
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_csas csas;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CSAS;
    cmd.params.ptr_set_plus_csas = &csas;

    csas.profile = profile;

    TRACE_FUNCTION("atPlusCSAS() calls bat_send() <=== as APPLICATION");
    bat_send(ati_bat_get_client(srcId), &cmd);
    return (AT_EXCT);
  }
#else /* FF_ATI_BAT */
  switch (sAT_PlusCSAS ((T_ACI_CMD_SRC)srcId, profile))
  {
    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_CSAS;
      return (ATI_EXCT);

    case AT_BUSY:
      return (ATI_BUSY);

    case AT_CMPL:
      return (ATI_CMPL_NO_OUTPUT);

    case AT_FAIL:           /* execution of command failed */
    default:
      cmdCmsError ( CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT tesatPlusCSAS (char *cl, UBYTE srcId)
{
  SHORT        profile  = -1;

  if ((profile = cmhSMS_getPrfRge ()) < 1)
  {
    cmdCmsError ( CMS_ERR_MemFail );
    return (ATI_FAIL);
  }
  else
  {
    sprintf ( g_sa, "+CSAS: (0-%d)", profile - 1 );
    io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    return (ATI_CMPL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCRES         |
+--------------------------------------------------------------------+

  PURPOSE : +CRES command (Load SMS Profile)
*/

GLOBAL T_ATI_RSLT atPlusCRES (char *cl, UBYTE srcId)
{
  SHORT        profile  = -1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCRES()");

  if ( *cl EQ '\0' )
  {
    profile = 1;
  }
  else
  {
    cl = parse ( cl, "r", &profile );

    if ( !cl )
    {
      cmdCmsError ( CMS_ERR_MemFail );
      return (ATI_FAIL);
    }
    profile++;
  }

  switch (sAT_PlusCRES ((T_ACI_CMD_SRC)srcId, profile))
  {
    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_CRES;
      return (ATI_EXCT);

    case AT_BUSY:
      return (ATI_BUSY);

    case AT_CMPL:
      io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
      return (ATI_CMPL_NO_OUTPUT);

    case AT_FAIL:
    default:
      cmdCmsError ( CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
  }
}

GLOBAL T_ATI_RSLT tesatPlusCRES (char *cl, UBYTE srcId)
{
  T_ACI_CRES    profile;
  T_ACI_RETURN  result;

  result = tAT_PlusCRES ((T_ACI_CMD_SRC)srcId, &profile);
  switch (result)
  {
    case AT_FAIL:
      cmdCmsError ( CMS_ERR_MemFail );
      return (ATI_FAIL);

    case AT_CMPL:
      break;

    default:
      cmdCmsError ( CMS_ERR_MemFail );
      return (ATI_FAIL); 
  }

  sprintf ( g_sa, "+CRES: (%d-%d)", profile.min, profile.max);
  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGR         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGR command (Read Message)
*/

GLOBAL T_ATI_RSLT atPlusCMGR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT idx=-1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCMGR()");

#ifndef WIN32
   cl = parse(cl,"r",&idx);
#else
   cl = parse(cl,"rd",&idx, &smsReadMode );
#endif
  if ( !cl OR idx > 255 OR idx < 0)
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cmgr cmgr;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CMGR;
    cmd.params.ptr_set_plus_cmgr=&cmgr;

    cmgr.sms_index=(U16)idx;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CMGR;
    return(ATI_EXCT);
  }

#else

#ifndef _CONC_TESTING_
#ifndef _SIMULATION_
  ret = sAT_PlusCMGR_Gl((T_ACI_CMD_SRC)srcId, (UBYTE)idx, SMS_READ_Normal, NULL);
#else
  ret = sAT_PlusCMGR_Gl(srcId, (UBYTE)idx, smsReadMode , NULL);
#endif
#else
#ifndef _SIMULATION_
  ret = sAT_PlusCMGR (srcId, (UBYTE)idx, SMS_READ_Normal);
#else
  ret = sAT_PlusCMGR (srcId, (UBYTE)idx, smsReadMode );
#endif
#endif
 if ( ret EQ AT_CMPL )
 {
   return (ATI_CMPL);
 }
 else if (ret EQ AT_EXCT)
 {
   src_params->curAtCmd    = AT_CMD_CMGR;
   return (ATI_EXCT);
 }
 else
 {
   cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
   return (ATI_FAIL);
 }

#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGS         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGS command (SEND Message)
*/

GLOBAL T_ATI_RSLT tesatPlusCMGS (char *cl, UBYTE srcId)
{
  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT queatPlusCMGS (char *cl, UBYTE srcId)
{
  cmdCmsError(CMS_ERR_OpNotAllowed); /* querying CMGS not specified in 07.05 */
  return (ATI_FAIL);
}


GLOBAL T_ATI_RSLT atPlusCMGS (char *cl, UBYTE srcId)
#if defined (SMS_PDU_SUPPORT)
{
  T_ACI_CMGF_MOD mode;
  
  TRACE_FUNCTION("atPlusCMGS()");
  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
  {
    /*
     * handle PDU mode
     */
    return (atPlusCMGSPdu (cl, srcId));
  }
  else
  {
    /*
     * handle Text mode
     */
    return (atPlusCMGSText (cl, srcId));
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : str_to_bcd           |
+--------------------------------------------------------------------+

  PURPOSE : A very specialised routine for converting a 2-byte string
            into one byte of BCD, intended to be used by atPlusCMGSText()
            only.
*/
LOCAL T_ATI_RSLT atPlusCMGSText(char *cl, UBYTE srcId)
#endif /* (SMS_PDU_SUPPORT) */
{
   T_ACI_VP_ABS  vpabs;
   SHORT           fo,
                       vprel,
                       pid,
                       dcs;
   T_ACI_RETURN   ret       = AT_FAIL;
   T_ACI_TOA        toda;
   T_ACI_TOA*      p_toda;
   T_ACI_SM_DATA msg;
   USHORT        lenMsg;
   CHAR*         da = daBuf;
  USHORT        dalen = 0;
  UBYTE         dest_addr[MAX_SMS_NUM_LEN];

   size_t           strg_len;
#ifdef WIN32
  static T_ACI_TOA     tosca;
  SHORT         octet=0;
#endif /* WIN32 */
#if defined WIN32 OR defined _SIMULATION_
  static T_ACI_TOA    *p_tosca;
  static SHORT        isReply;
#endif
#ifdef _SIMULATION_
  CHAR         *sca =subBuf;
#endif /* _SIMULATION_ */

#if defined _CONC_TESTING_ AND defined TI_PS_FF_CONC_SMS
  T_SM_DATA_EXT msg_conc;
#endif

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  if (src_params->text_mode EQ CMD_MODE)
  {
    TRACE_FUNCTION("atPlusCMGSText() CMD_MODE");
    memset (da, 0, MAX_SMS_NUM_LEN);
    toda_val = 0;
    /* input functionality */
    da[0]='\0';
#ifndef WIN32
    switch(ati_user_output_cfg[srcId].cscsChset)
     {
       case CSCS_CHSET_Hex:
         cl = parse (cl,"sr",(LONG)MAX_SMS_NUM_LEN*2,da,&toda_val);
         break;
       case CSCS_CHSET_Ucs2:
         cl = parse (cl,"sr",(LONG)MAX_SMS_NUM_LEN*4,da,&toda_val);
         break;
       default:
         cl = parse (cl,"sr",(LONG)MAX_SMS_NUM_LEN,da,&toda_val);
         break;
     }

    if ( !cl OR da[0] EQ '\0')
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
#else
    *sca    ='\0';
    isReply = -1;
    switch(ati_user_output_cfg[srcId].cscsChset)
    {
      case CSCS_CHSET_Hex:
        cl = parse (cl,"srsrb",(LONG)MAX_SMS_NUM_LEN*2,da,&toda_val,
                           (LONG)MAX_SMS_NUM_LEN*2,sca,&octet,&isReply);
        break;
      case CSCS_CHSET_Ucs2:
        cl = parse (cl,"srsrb",(LONG)MAX_SMS_NUM_LEN*4,da,&toda_val,
                           (LONG)MAX_SMS_NUM_LEN*4,sca,&octet,&isReply);
        break;
      default:
        cl = parse (cl,"srsrb",(LONG)MAX_SMS_NUM_LEN,da,&toda_val,
                           (LONG)MAX_SMS_NUM_LEN,sca,&octet,&isReply);
        break;
    }

    if ( !cl OR da[0] EQ '\0')
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    if (octet EQ 0)
    {
      p_tosca=NULL;
    }
    else
    {
      tosca=toa_demerge(octet);
      if (tosca.ton < 0 OR tosca.npi < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    }
#endif
    if (toda_val)
    {
      toda=toa_sms_demerge(toda_val);
      if (toda.ton < 0 OR toda.npi < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    }

    src_params->text_mode = TXT_MODE;
    return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    TRACE_FUNCTION("atPlusCMGSText() TXT_MODE");

    src_params->text_mode = CMD_MODE;
    /* if the character sent is ESC, then abort command CLB 16.11.00 */
    if (*cl EQ 0x1B)
    {
      TRACE_EVENT("Send message command cancelled by user");

      return ATI_CMPL_NO_OUTPUT;
    }

    if ( toda_val EQ 0 )
    {
#ifndef _SIMULATION_
#endif /* ndef _SIMULATION_ */
      p_toda = NULL;
    }
    else
    {
      toda   = toa_sms_demerge ( toda_val );
      p_toda = &toda;
     if( toda.ton EQ TON_Alphanumeric)
      {
        dalen = strlen(da);

        switch (ati_user_output_cfg[srcId].cscsChset)
        {
        case CSCS_CHSET_Ucs2:
          utl_chsetToGsm ( (UBYTE*)da,
            dalen,
            dest_addr,
            &dalen,
#ifdef REL99
            sizeof(dest_addr),
#endif
            GSM_ALPHA_Def );
          break;
          
        case CSCS_CHSET_Hex:
          utl_chsetToGsm ( (UBYTE*)da,
            dalen,
            dest_addr,
            &dalen,
#ifdef REL99
            sizeof(dest_addr),
#endif
            GSM_ALPHA_Def );
          break;
          
        default:
          utl_chsetToGsm ( (UBYTE*)da,
            dalen,
            dest_addr,
            &dalen,
#ifdef REL99
            sizeof(dest_addr),
#endif
            GSM_ALPHA_Def );
          break;
        }
        if( dalen > (((MAX_SMS_ADDR_DIG/2) * 8) / 7) )
        {
          TRACE_EVENT("DA length is greater then the supported length");
          return(ATI_FAIL);
        }
        da = (CHAR*)dest_addr;
        da[dalen] = '\0'; /* dest_addr is a non NULL terminated string, so it should be terminated by 0 for internal processing */
      }
    }
    
    qAT_PlusCSMP  ( (T_ACI_CMD_SRC)srcId, &fo, &vprel, &vpabs, NULL, &pid, &dcs );

    srcId_cb = srcId;
    strg_len=strlen(cl);

    switch (ati_user_output_cfg[srcId].cscsChset)
    {
      case CSCS_CHSET_Ucs2:
        utl_smDtaFromTe ((UBYTE*)cl,
                         (USHORT)MINIMUM(strg_len,sizeof(msg.data)*4),
                         msg.data,
                         &lenMsg,
#ifdef REL99
                         sizeof(msg.data),
#endif
                         (UBYTE)fo,
                         (UBYTE)dcs );
        break;
      case CSCS_CHSET_Hex:
        utl_smDtaFromTe ((UBYTE*)cl,
                         (USHORT)MINIMUM(strg_len,sizeof(msg.data)*2),
                         msg.data,
                         &lenMsg,
#ifdef REL99
                         sizeof(msg.data),
#endif
                         (UBYTE)fo,
                         (UBYTE)dcs );
        break;
      default:
        utl_smDtaFromTe ((UBYTE*)cl,
                         (USHORT)MINIMUM(strg_len,sizeof(msg.data)),
                         msg.data,
                         &lenMsg,
#ifdef REL99
                         sizeof(msg.data),
#endif
                         (UBYTE)fo,
                         (UBYTE)dcs );
        break;
    }

    msg.len = (UBYTE)lenMsg;
#if !defined _CONC_TESTING_ OR !defined TI_PS_FF_CONC_SMS
#ifndef _SIMULATION_
    ret = sAT_PlusCMGS_Gl((T_ACI_CMD_SRC)srcId, da, p_toda, &msg, NULL, NULL, NULL, -1, NULL, NULL);
#else
    ret = sAT_PlusCMGS_Gl(srcId, da, p_toda, &msg, NULL, sca, p_tosca, isReply, NULL, NULL);
#endif
#else
    ACI_MALLOC(msg_conc.data, msg.len);
    memcpy ( msg_conc.data, msg.data, msg.len);
    msg_conc.len = msg.len;
#ifndef _SIMULATION_
    ret = sAT_PlusCMGS (srcId, da, p_toda, &msg_conc, NULL, NULL, -1);
#else
    ret = sAT_PlusCMGS (srcId, da, p_toda, &msg_conc, sca, p_tosca, isReply);
#endif
#endif

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
    }
    /*
     *  rCI_OK will emitting +CMGS: <mr>[,<scts>]
     */

    return (ATI_EXCT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMSS         |
+--------------------------------------------------------------------+

  PURPOSE : +CMSS command (send message from storage)
*/

GLOBAL T_ATI_RSLT atPlusCMSS (char *cl, UBYTE srcId)
{
  CHAR         *p_da;
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_TOA     toda;
  T_ACI_TOA    *p_toda;
  SHORT         index=-1,
                toda_val=0;
  CHAR          da[MAX_SMS_NUM_LEN*4]; /* 4 is for UCS2 chars */
  USHORT        dalen = 0;
  UBYTE         dest_addr[MAX_SMS_NUM_LEN];

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCMSS()");

  p_toda=&toda;
  p_da=da;
  /*input functionality*/
  da[0]='\0';
   switch(ati_user_output_cfg[srcId].cscsChset)
  {
    case CSCS_CHSET_Hex:
      cl = parse(cl,"rsr",&index,(LONG)MAX_SMS_NUM_LEN*2,da,&toda_val);
      break;
    case CSCS_CHSET_Ucs2:
      cl = parse(cl,"rsr",&index,(LONG)MAX_SMS_NUM_LEN*4,da,&toda_val);
      break;
    default: 
      cl = parse(cl,"rsr",&index,(LONG)MAX_SMS_NUM_LEN,da,&toda_val);
      break;
  }
  if ( !cl OR toda_val > 0xD9 OR index > 255 OR index < 0)
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }
  p_toda=&toda;
  if(toda_val EQ 0)
  {
    p_toda=NULL;
  }
  else
  {
    toda=toa_sms_demerge(toda_val);
    if (toda.ton < 0 OR toda.npi < 0)
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
  }
  if(!da[0])
    p_da=NULL;
  else
  {
    if( toda.ton EQ TON_Alphanumeric)
    {
      dalen = strlen(da);

      switch (ati_user_output_cfg[srcId].cscsChset)
      {
      case CSCS_CHSET_Ucs2:
        utl_chsetToGsm ( (UBYTE*)da, 
          dalen,
          dest_addr, 
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif                            
          GSM_ALPHA_Def );
        break;
        
      case CSCS_CHSET_Hex:
        utl_chsetToGsm ( (UBYTE*)da, 
          dalen,
          dest_addr, 
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif
          GSM_ALPHA_Def );
        break;
        
      default:
        utl_chsetToGsm ( (UBYTE*)da, 
          dalen,
          dest_addr, 
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif
          GSM_ALPHA_Def );
        break;
      }
      if( dalen > (((MAX_SMS_ADDR_DIG/2) * 8) / 7) )
      {
        TRACE_EVENT("DA length is greater then the supported length");
        return(ATI_FAIL);
      }
      p_da = (CHAR*)dest_addr;
      p_da[dalen] = '\0'; /* dest_addr is a non NULL terminated string, so it should be terminated by 0 for internal processing */
    }
  }

#ifdef _CONC_TESTING_
  ret = sAT_PlusCMSS(srcId,(UBYTE)index,p_da,p_toda);
#else
  ret = sAT_PlusCMSS_Gl((T_ACI_CMD_SRC)srcId, (UBYTE)index, p_da, p_toda, NULL, NULL);
#endif
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_CMSS;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCNMA         |
+--------------------------------------------------------------------+

  PURPOSE : +CNMA command (new message acknowledgement)
*/
GLOBAL T_ATI_RSLT queatPlusCNMA (char *cl, UBYTE srcId)
{
  cmdCmsError(CMS_ERR_OpNotAllowed); /* querying CNMA not specified in 07.05 */
  return (ATI_FAIL);
}

GLOBAL T_ATI_RSLT tesatPlusCNMA (char *cl, UBYTE srcId)
{
  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
  {
    /*
     * handle PDU mode: response is +CNMA: (list of supported <n>s)
     */
    io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
    return ATI_CMPL_NO_OUTPUT;
  }
  else
  {
    /*
     * handle Text mode: there is no response
     */
    io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
    return ATI_CMPL_NO_OUTPUT;
  }
}


GLOBAL T_ATI_RSLT atPlusCNMA (char *cl, UBYTE srcId)
#if defined (SMS_PDU_SUPPORT)
{
  T_ACI_CMGF_MOD         mode;

  if(srcId NEQ smsShrdPrm.smsSrcId)
  {
    cmdCmsError(CMS_ERR_NoCnmaAckExpect);
    return ( ATI_FAIL );
  }
  
  if( waitForCnmaFromBuffer_SrcId NEQ CMD_SRC_NONE )
  { /* incomming +CNMA acknowledges an +CMT from CNMI buffer 
       -> acknowledge has been sent to SMS entity */
    TIMERSTOP( ACI_CNMA_TIMER_HANDLE );
    cmd_clearFirstCnmiMessage(); /* The first message in CNMI buffer is cleared */
    waitForCnmaFromBuffer_SrcId = CMD_SRC_NONE;
    /* look for more messages in CNMI buffer for this srcId */
    cmd_flushCnmiBufOneByOne();
    return(ATI_CMPL);
  }
  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    return atPlusCNMAPdu (cl,srcId);
  else
    /*
     * handle Text mode
     */
    return atPlusCNMAText (cl,srcId);
}

LOCAL T_ATI_RSLT atPlusCNMAText (char *cl, UBYTE srcId)
#endif /* (SMS_PDU_SUPPORT) */
{
  TRACE_FUNCTION("atPlusCNMAText()");

  switch (*cl)
  {
    case(0x0):
      switch (sAT_PlusCNMA((T_ACI_CMD_SRC)srcId))
      {
        case( AT_CMPL ):
          cmdAtError ( atOk );
          return (ATI_CMPL);

        default:
          cmdCmsError (CMS_ERR_NotPresent);  /* use aciErrDesc */
          return (ATI_FAIL);
      }
    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGW         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGW command (Write message to memory)
*/

GLOBAL T_ATI_RSLT tesatPlusCMGW (char *cl, UBYTE srcId)
{
  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT queatPlusCMGW (char *cl, UBYTE srcId)
{
  cmdCmsError(CMS_ERR_OpNotAllowed); /* querying CMGW not specified in 07.05 */
  return (ATI_FAIL);
}


GLOBAL T_ATI_RSLT atPlusCMGW (char *cl, UBYTE srcId)
#if defined (SMS_PDU_SUPPORT)
{
  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    return atPlusCMGWPdu (cl, srcId);
  else
    /*
     * handle Text mode
     */
    return atPlusCMGWText (cl, srcId);
}
LOCAL T_ATI_RSLT atPlusCMGWText(char *cl, UBYTE srcId)
#endif /* (SMS_PDU_SUPPORT) */
{
  T_ACI_VP_ABS  vpabs;
  SHORT         fo,
                vprel,
                pid,
                dcs;
  T_ACI_RETURN  ret;
  T_ACI_TOA     toa;
  T_ACI_TOA*    p_toa;
  CHAR          txtStat[15] = {0x00};
  USHORT        i;
  T_ACI_SM_DATA msg;
  USHORT        lenMsg;
  static T_ACI_SMS_STAT stat;
#if defined _SIMULATION_ OR defined WIN32
  static T_ACI_TOA    *p_tosca;
  static SHORT        isReply;
#endif
#ifdef _SIMULATION_
  CHAR         *sca =subBuf;
#endif /* ndef _SIMULATION_ */
#ifdef WIN32
  SHORT         octet=0;
  static T_ACI_TOA     tosca;
#endif /* WIN32 */

#if defined _CONC_TESTING_ AND defined TI_PS_FF_CONC_SMS
  T_SM_DATA_EXT msg_conc;
#endif
  CHAR         *da = daBuf;
  USHORT       dalen = 0;
  UBYTE        dest_addr[MAX_SMS_NUM_LEN];

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCMGWText()");

  p_toa=&toa;

  if (src_params->text_mode EQ CMD_MODE)
  {
    toda_val=0;
    memset(da, 0, MAX_SMS_NUM_LEN);
    stat = SMS_STAT_NotPresent;
    /*input functionality*/
    *da='\0';                      /*sets global destination address to empty string*/
#ifndef WIN32
    switch(ati_user_output_cfg[srcId].cscsChset)
    {
      case CSCS_CHSET_Hex:
        cl = parse(cl,"srs",(LONG)MAX_SMS_NUM_LEN*2,
                        da,
                        &toda_val,
                        (LONG)sizeof(txtStat),
                        txtStat);
        break;
      case CSCS_CHSET_Ucs2:
        cl = parse(cl,"srs",(LONG)MAX_SMS_NUM_LEN*4,
                        da,
                        &toda_val,
                        (LONG)sizeof(txtStat),
                        txtStat);
        break;
      default:
        cl = parse(cl,"srs",(LONG)MAX_SMS_NUM_LEN,
                        da,
                        &toda_val,
                        (LONG)sizeof(txtStat),
                        txtStat);
        break;
    }

    if ( !cl )
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
#else
    *sca='\0';
    isReply = -1;
     switch(ati_user_output_cfg[srcId].cscsChset)
    {
      case CSCS_CHSET_Hex:
        cl = parse(cl,"srssrb",(LONG)MAX_SMS_NUM_LEN*2,
                       da,
                       &toda_val,
                       (LONG)sizeof(txtStat),
                       txtStat,
                       (LONG)MAX_SMS_NUM_LEN*2,sca,&octet,&isReply);
        break;
      case CSCS_CHSET_Ucs2:
        cl = parse(cl,"srssrb",(LONG)MAX_SMS_NUM_LEN*4,
                       da,
                       &toda_val,
                       (LONG)sizeof(txtStat),
                       txtStat,
                       (LONG)MAX_SMS_NUM_LEN*4,sca,&octet,&isReply);
        break;
      default:
        cl = parse(cl,"srssrb",(LONG)MAX_SMS_NUM_LEN,
                       da,
                       &toda_val,
                       (LONG)sizeof(txtStat),
                       txtStat,
                       (LONG)MAX_SMS_NUM_LEN,sca,&octet,&isReply);
        break;
    }

    if ( !cl )
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    if(octet EQ 0)
      p_tosca = NULL;
    else
    {
      tosca=toa_demerge(octet);
      if (tosca.ton < 0 OR tosca.npi < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return 0;
      }
      p_tosca = &tosca;
    }
#endif
    if (toda_val)
    {
      toa = toa_sms_demerge(toda_val);
      if (toa.ton < 0 OR toa.npi < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    }
    i = 0;
    while ( sms_stat[i].name                     NEQ NULL AND
            strcmp ( sms_stat[i].name, txtStat ) NEQ 0        )
    {
      i++;
    }
    stat = sms_stat[i].stat;

    src_params->text_mode = TXT_MODE;
    return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    src_params->text_mode = CMD_MODE;
    /* if the character sent is ESC, then abort command CLB 16.11.00 */
    if (*cl EQ 0x1B)
    {
      TRACE_EVENT("Send message command cancelled by user");

      return ATI_CMPL_NO_OUTPUT;
    }

    if ( toda_val EQ 0 )
      p_toa = NULL;
    else
    {
      toa   = toa_sms_demerge ( toda_val );
      p_toa = &toa;
    }

    qAT_PlusCSMP  ( (T_ACI_CMD_SRC)srcId, &fo, &vprel, &vpabs, NULL, &pid, &dcs );

    srcId_cb = srcId;
    if( toa.ton EQ TON_Alphanumeric)
    {
      dalen = strlen(da);

      switch (ati_user_output_cfg[srcId].cscsChset)
      {
      case CSCS_CHSET_Ucs2:
        utl_chsetToGsm ( (UBYTE*)da,
          dalen,
          dest_addr,
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif
          GSM_ALPHA_Def );
        break;
        
      case CSCS_CHSET_Hex:
        utl_chsetToGsm ( (UBYTE*)da,
          dalen,
          dest_addr,
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif
          GSM_ALPHA_Def );
        break;
      default:
        utl_chsetToGsm ( (UBYTE*)da,
          dalen,
          dest_addr,
          &dalen,
#ifdef REL99
          sizeof(dest_addr),
#endif
          GSM_ALPHA_Def );
        break;
      }
      if( dalen > (((MAX_SMS_ADDR_DIG/2) * 8) / 7) )
      {
        TRACE_EVENT("DA length is greater then the supported length");
        return(ATI_FAIL);
      }
      da = (CHAR*)dest_addr;
      da[dalen] = '\0'; /* dest_addr is a non NULL terminated string, so it should be terminated by 0 for internal processing */
    }

    utl_smDtaFromTe ( (UBYTE*)cl,
                      (USHORT)strlen(cl),
                      (UBYTE*)msg.data,
                      &lenMsg,
#ifdef REL99
                       sizeof(msg.data),
#endif
                      (UBYTE)fo,
                      (UBYTE)dcs );

    msg.len = (UBYTE)lenMsg;

#if !defined _CONC_TESTING_ OR !defined TI_PS_FF_CONC_SMS
#ifndef _SIMULATION_
    ret = sAT_PlusCMGW_Gl((T_ACI_CMD_SRC)srcId,
                           CMGW_IDX_FREE_ENTRY,
                           da, p_toa, stat, NOT_PRESENT_8BIT,
                           &msg, NULL, NULL, NULL, -1,
                           NULL, NULL);
#else
    ret = sAT_PlusCMGW_Gl( srcId,
                           CMGW_IDX_FREE_ENTRY,
                           da, p_toa, stat, NOT_PRESENT_8BIT,
                           &msg, NULL, sca, p_tosca, isReply,
                           NULL, NULL);
#endif
#else
    ACI_MALLOC(msg_conc.data, msg.len);
    memcpy ( msg_conc.data, msg.data, msg.len);
    msg_conc.len = msg.len;
#ifndef _SIMULATION_
    ret = sAT_PlusCMGW ( srcId,
                         CMGW_IDX_FREE_ENTRY,
                         da, p_toa, stat, NOT_PRESENT_8BIT,
                         &msg_conc, NULL, NULL, -1 );
#else
    ret = sAT_PlusCMGW ( srcId,
                         CMGW_IDX_FREE_ENTRY,
                         da, p_toa, stat, NOT_PRESENT_8BIT,
                         &msg_conc, sca, p_tosca, isReply );
#endif
#endif

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
    }
    /*
     *  rCI_OK will emitting +CMGW: <index>
     */
    return (ATI_EXCT);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGD         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGD command (delete message from storage)
*/

GLOBAL T_ATI_RSLT setatPlusCMGD (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT index=-1;
  SHORT status = CMGD_DEL_INDEX;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CMGD;

  /*input functionality*/
  if(!cl OR !*cl)
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return ATI_FAIL;
  }

  cl=parse(cl,"rr",&index,&status);
  if ( !cl OR (index > 255) OR (index < 0) OR
    ((status < CMGD_DEL_INDEX) OR (status > CMGD_DEL_ALL)))
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cmgd my_bat_set_plus_cmgd;

  TRACE_FUNCTION("setatPlusCMGD() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cmgd, 0, sizeof(my_bat_set_plus_cmgd));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CMGD;
  cmd.params.ptr_set_plus_cmgd = &my_bat_set_plus_cmgd;

  my_bat_set_plus_cmgd.index = (U8)index;
  my_bat_set_plus_cmgd.status = status;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusCMGD()");
  /*-----------------------------------------------------------------------
   *  Del flag is greater than zero, set index as Zero so that SMS module 
   *  starts searching for records satisfying the status from first record
   *-----------------------------------------------------------------------
   */
  if ( status > CMGD_DEL_INDEX )
  {
    index = 0;
  }
#ifdef _CONC_TESTING_
  ret = sAT_PlusCMGD(srcId,(UBYTE)index, status);
#else
  ret = sAT_PlusCMGD_Gl((T_ACI_CMD_SRC)srcId, (UBYTE)index, status, NULL, NULL);
#endif

  if (ret EQ AT_EXCT)
  {
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGC         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGC command (Send SMS command)
*/

GLOBAL T_ATI_RSLT tesatPlusCMGC (char *cl, UBYTE srcId)
{
  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT queatPlusCMGC (char *cl, UBYTE srcId)
{
  cmdCmsError(CMS_ERR_OpNotAllowed); /* querying CMGC not specified in 07.05 */
  return (ATI_FAIL);
}

GLOBAL T_ATI_RSLT atPlusCMGC (char *cl, UBYTE srcId)
#if defined (SMS_PDU_SUPPORT)
{
  T_ACI_CMGF_MOD mode;

  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
    /*
     * handle PDU mode
     */
    return atPlusCMGCPdu (cl, srcId);
  else
    /*
     * handle Text mode
     */
    return atPlusCMGCText (cl, srcId);
}

LOCAL T_ATI_RSLT atPlusCMGCText(char *cl, UBYTE srcId)
#endif /* (SMS_PDU_SUPPORT) */
{
/*  char           buf[80]; */
  T_ACI_RETURN   ret = AT_FAIL;
  T_ACI_TOA      toa;
  T_ACI_TOA*     p_toa;
  T_ACI_CMD_DATA cmd;
  USHORT         lenCmd;
  CHAR*          da = daBuf;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCMGCText()");
  p_toa=&toa;
  if (src_params->text_mode EQ CMD_MODE)
  {
    memset(da, 0, MAX_SMS_NUM_LEN);
    fo=-1;
    ct=-1;
    pid=-1;
    mn=-1;
    toda_val=0;
    /*input functionality*/
    da[0]='\0';
    cl = parse(cl,"rrrrsr",&fo,&ct,&pid,&mn,(LONG)MAX_SMS_NUM_LEN,da,&toda_val);

    if ( !cl OR  fo > 255 OR ct > 255 OR
          pid > 255 OR mn > 255 OR toda_val > 0xC9 OR da[0] EQ '\0')
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    if(toda_val)
    {
      toa = toa_demerge(toda_val);
      if (toa.ton < 0 OR toa.npi < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
    }

    src_params->text_mode = TXT_MODE;
    return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    src_params->text_mode = CMD_MODE;
    p_toa=&toa;
    if(toda_val EQ 0)
    {
      p_toa = NULL;
    }
    else
    {
      toa = toa_demerge(toda_val);
    }
    if (!(check_str(cl,"0123456789ABCDEF")))
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    utl_hexToGsm ( (UBYTE*)cl,
                   (USHORT)strlen(cl),
                   (UBYTE*)cmd.data,
                   &lenCmd,
                   GSM_ALPHA_Def,
                   CSCS_ALPHA_8_Bit);
    cmd.len = (UBYTE)lenCmd;

#ifndef _CONC_TESTING_
    ret = sAT_PlusCMGC_Gl ((T_ACI_CMD_SRC)srcId,fo,ct,pid,mn,da,p_toa,&cmd,NULL);
#else
    ret= sAT_PlusCMGC(srcId,fo,ct,pid,mn,da,p_toa,&cmd);
#endif

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  /* use aciErrDesc */
      return (ATI_FAIL);
    }
    /*
     *  rCI_OK will emitting +CMGC: <mr>[,<scts>]
     */
    return (ATI_EXCT);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMGL         |
+--------------------------------------------------------------------+

  PURPOSE : +CMGL command (List Messages)
*/

GLOBAL T_ATI_RSLT queatPlusCMGL (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPlusCMGL()");
    cmdCmsError ( CMS_ERR_OpNotAllowed );
    return (ATI_FAIL);
}

GLOBAL T_ATI_RSLT tesatPlusCMGL (char *cl, UBYTE srcId)
{
#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("tesatPlusCMGL()");
  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
  /*
   * handle PDU mode
   */
    io_sendMessage(srcId, "+CMGL: (0,1,2,3,4)", ATI_NORMAL_OUTPUT);
  else
  /*
   * handle Text mode
   */
#endif
   io_sendMessage(srcId, "+CMGL: (\"REC UNREAD\",\"REC READ\",\"STO UNSENT\",\"STO SENT\",\"ALL\")", ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

GLOBAL T_ATI_RSLT atPlusCMGL (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN   ret;
#endif

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
#endif
  CHAR           txtStat[15];
  USHORT         i;

  TRACE_FUNCTION("atPlusCMGL()");


  if (*cl EQ '\0')
  {
    cmglStat = SMS_STAT_NotPresent;
#ifdef WIN32
    smsReadMode  = SMS_READ_Normal;
#endif
  }
  else
  {
    /*
     * Request of a list of stored SMS messages
     * Parameter stat is optional
     */
#ifdef SMS_PDU_SUPPORT
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
    if (mode EQ 0)
    {
#ifndef WIN32
      /*
       * On the target is the definition according GSM 7.05 CMGL=[stat]
       */
      cmglStat = SMS_STAT_All;
      cl = parse (cl, "d", &cmglStat);
      if ( !cl OR cmglStat < SMS_STAT_RecUnread OR cmglStat > SMS_STAT_All)
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
#else
      /*
       * Under Windows the definition is CMGL=[stat],[preview]
       * to test the capabilities of the functional interface
       */
      cl = parse (cl, "dd", &cmglStat, &smsReadMode );
      if ( !cl OR ((cmglStat < SMS_STAT_RecUnread OR cmglStat > SMS_STAT_All)
           AND smsReadMode  EQ SMS_READ_NotPresent))
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
#endif
    }
    else
#endif
    {
#ifndef WIN32
      /*
       * On the target is the definition according GSM 7.05 CMGL=[stat]
       */
      cl = parse (cl, "s", (LONG)sizeof(txtStat), txtStat);
      if ( !cl OR txtStat[0] EQ '\0')
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
#else
      /*
       * Under Windows the definition is CMGL=[stat],[preview]
       * to test the capabilities of the functional interface
       */
      cl = parse (cl, "sd", (LONG)sizeof(txtStat), txtStat, &smsReadMode );
      if ( !cl OR (txtStat[0] EQ '\0' AND smsReadMode  EQ SMS_READ_NotPresent))
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
      if (txtStat[0] EQ '\0')
        strcpy (txtStat, sms_stat[4].name); /* ALL as default */
#endif
      i = 0;
      while ( sms_stat[i].name NEQ NULL            AND
              strcmp ( sms_stat[i].name, txtStat ) NEQ 0 )
        i++;

      if ( sms_stat[i].name EQ NULL )
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }

      cmglStat = sms_stat[i].stat;
    }
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cmgl cmgl;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CMGL;
    cmd.params.ptr_set_plus_cmgl=&cmgl;

    /*
    *   T_ACI_SMS_STAT and T_BAT_plus_cmgl_stat are not quite
    *   identical - the ACI version has the extra value
    *   SMS_STAT_Invalid.
    */
    if (cmglStat EQ SMS_STAT_Invalid)
      cmgl.stat=BAT_CMGL_STAT_NOT_PRESENT;
    else
      cmgl.stat=(T_BAT_plus_cmgl_stat)cmglStat;

    bat_send(ati_bat_get_client(srcId),&cmd);

    src_params->curAtCmd=AT_CMD_CMGL;
    return(ATI_EXCT);
  }

#else

  /*
   * Request the first five SMS messages
   */
#ifndef WIN32
  ret = sAT_PlusCMGL ( (T_ACI_CMD_SRC)srcId, cmglStat, 0, SMS_READ_Normal );
#else
  ret = sAT_PlusCMGL ( srcId, cmglStat, 0, smsReadMode  );
#endif

  if ( ret EQ AT_EXCT )
  {
    src_params->curAtCmd = AT_CMD_CMGL;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError ( CMS_ERR_NotPresent );  /* use aciErrDesc */
    return (ATI_FAIL);
  }

#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCNMI         |
+--------------------------------------------------------------------+

  PURPOSE : +CNMI command (new messgae indication settings )
*/

GLOBAL T_ATI_RSLT  atPlusCNMI (char *cl, UBYTE srcId)
{
  T_ACI_CNMI_MT  mt   = CNMI_MT_NotPresent;
  T_ACI_CNMI_BM  bm   = CNMI_BM_NotPresent;
  T_ACI_CNMI_DS  ds   = CNMI_DS_NotPresent;
  T_ACI_CNMI_MOD mode = CNMI_MOD_NotPresent;
  T_ACI_CNMI_BFR bfr  = CNMI_BFR_NotPresent;
  T_ACI_RETURN   ret  = AT_FAIL;

  TRACE_FUNCTION("atPlusCNMI()");

  /* input functionality */
  cl = parse ( cl, "ddddd", &mode, &mt, &bm, &ds, &bfr );

  if ( !cl                              OR
        mode <  CNMI_MOD_NotPresent     OR
        mode >  CNMI_MOD_BufferAndFlush OR
        bfr  <  CNMI_BFR_NotPresent     OR
        bfr  >  CNMI_BFR_Clear          OR /*lint !e685 (Warning: Relational operator '>' always evaluates to 'false')*/
      ( bfr  EQ CNMI_BFR_Flush          AND
        strlen ( cl ) > 0                   ) )
  {
      io_sendConfirm ( srcId, cmdCmsError ( CMS_ERR_OpNotAllowed ), ATI_ERROR_OUTPUT );
      cmdErrStr = NULL;
      return ATI_FAIL_NO_OUTPUT;
  }

  ret = sAT_PlusCNMI ((T_ACI_CMD_SRC) srcId, mt, bm, ds );
  if ( (ret EQ AT_FAIL) OR (ret EQ AT_BUSY) )
  {
    io_sendConfirm ( srcId, cmdCmsError ( CMS_ERR_NotPresent ), ATI_ERROR_OUTPUT );
    cmdErrStr = NULL;
    return ATI_FAIL_NO_OUTPUT;
  }

  /* 
   *----------------------Issue 25033--------------------------------
   * The source ID issuing the CNMI command is stored and henceforth 
   * all the unsolicited SMS indications are sent to this source 
   *-----------------------------------------------------------------
   */
  if(srcId NEQ CMD_SRC_LCL)
    smsShrdPrm.smsSrcId = (T_ACI_CMD_SRC)srcId;

  if ( bfr  NEQ CNMI_BFR_NotPresent )
    at.CNMI_bfr  = ( UBYTE ) bfr;

  if ( mode NEQ CNMI_MOD_NotPresent )
    at.CNMI_mode = ( UBYTE ) mode;

  if (mode EQ CNMI_MOD_DiscardOrForward OR  /* 07.05:3.4.1: check for <mode> 1...3  */
      mode EQ CNMI_MOD_BufferAndFlush)
  {
    if ( at.CNMI_bfr EQ CNMI_BFR_Flush )
    {
      io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
      cmdErrStr = NULL;

      if( smsShrdPrm.CSMSservice NEQ CSMS_SERV_GsmPh2Plus )
      {
        cmd_flushCnmiBuf();
      }
      else
      {
        cmd_flushCnmiBufOneByOne();
      }
      return ATI_CMPL_NO_OUTPUT;
    }
    else
    {
      cmd_clearCnmiBuf ();
    }
  }

  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  cmdErrStr = NULL;

  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT  tesatPlusCNMI (char *cl, UBYTE srcId)
{
  /* query parameter ranges */
  io_sendMessage ( srcId, "+CNMI: (0-2),(0-3),(0,2),(0,1),(0,1)", ATI_NORMAL_OUTPUT );
  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  cmdErrStr = NULL;

  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT  queatPlusCNMI (char *cl, UBYTE srcId)
{
  char*          me   = "+CNMI: ";
  T_ACI_CNMI_MT  mt   = CNMI_MT_NotPresent;
  T_ACI_CNMI_BM  bm   = CNMI_BM_NotPresent;
  T_ACI_CNMI_DS  ds   = CNMI_DS_NotPresent;

  if ( qAT_PlusCNMI ( (T_ACI_CMD_SRC)srcId, &mt, &bm, &ds ) EQ AT_CMPL )
  {
    sprintf ( g_sa,"%s%d,%d,%d,%d,%d", me, at.CNMI_mode,
                                     mt, bm, ds, at.CNMI_bfr );
    io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
  }
  else
  {
    io_sendConfirm ( srcId, cmdCmsError ( CMS_ERR_NotPresent ), ATI_ERROR_OUTPUT );
    cmdErrStr = NULL;
    return ATI_FAIL_NO_OUTPUT;
  }

  io_sendConfirm ( srcId, cmdAtError ( atOk ), ATI_NORMAL_OUTPUT );
  cmdErrStr = NULL;
  return ATI_CMPL_NO_OUTPUT;
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCPRSM  |
+--------------------------------------------------------------------+

  PURPOSE : %CPRSM set command 
            set the receiving of SMS to 
            1 -- Pause    or  
            2 -- Resume
           
*/

GLOBAL T_ATI_RSLT setatPercentCPRSM (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret     = AT_FAIL;
  T_ACI_CPRSM_MOD mode = CPRSM_MOD_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_CPRSM;
  
  /* parse command */
  cl = parse (cl, "d", &mode);

  /* check whether parsing has been successful */ 
  if (!cl)
  {
    /* undefined value for mode */
    TRACE_EVENT("+CPRSM ERROR: parse cmd failed!");
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return ATI_FAIL;    
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cprsm my_bat_set_percent_cprsm;

  TRACE_FUNCTION("setatPercentCPRSM() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cprsm, 0, sizeof(my_bat_set_percent_cprsm));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CPRSM;
  cmd.params.ptr_set_percent_cprsm = &my_bat_set_percent_cprsm;

  my_bat_set_percent_cprsm.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentCPRSM()");

  switch (mode) /* check which mode has to be set and set it */ 
  {
    case(CPRSM_MOD_Resume):
    case(CPRSM_MOD_Pause):
    {     
      /* AT interface function called */
      ret = sAT_PercentCPRSM((T_ACI_CMD_SRC)srcId, mode);
      break;
    }
    default:
      /* undefined value for mode */
      TRACE_EVENT_P1("+CPRSM ERROR: undefined value for mode: %d", mode);
      cmdCmsError(CMS_ERR_OpNotSup);
      return ATI_FAIL;
  }

  /* check result */
  switch (ret)
  {
    case (AT_CMPL):
      /* operation successfully completed */
      break;
    case (AT_EXCT):
      /* operation still executing */
      src_params->curAtCmd = AT_CMD_COPS;
      break;
    default:
      /* unknown result type */
      TRACE_EVENT_P1("+CPRSM ERROR: undefined result: %d", ret);
      cmdCmsError(CMS_ERR_UnknownErr);  /*Command failed*/
      return ATI_FAIL;
  }

  /* map error to ATI type and return */ 
  return (map_aci_2_ati_rslt(ret)); 

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCPRSM  |
+--------------------------------------------------------------------+

  PURPOSE : %CPRSM query command 
            request the current state of the pause flag for receiving SMS
*/

GLOBAL T_ATI_RSLT queatPercentCPRSM (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPercentCPRSM()");

  ret = qAT_PercentCPRSM( (T_ACI_CMD_SRC)srcId );


  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd = AT_CMD_CPRSM;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
    return (ATI_FAIL);
  }
}
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_clearCnmiBuf   |
+--------------------------------------------------------------------+

  PURPOSE : This function clears the buffer for storing new message
            indications while the serial interface is reserved for
            data transmission.
*/
GLOBAL void cmd_clearCnmiBuf (void)
{
  UBYTE i;

  for ( i = 0; i < CNMI_BUF_SIZE; i++ )
    cnmiBuf.sCnmiElement[i].type = CNMI_NONE;

  cnmiBuf.next = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_flushCnmiBuf   |
+--------------------------------------------------------------------+

  PURPOSE : This function flushes the buffer for storing new message
            indications while the serial interface is released from
            data transmission.
*/
GLOBAL void cmd_flushCnmiBuf (void)
{
  UBYTE i            = cnmiBuf.next;
  UBYTE entries_read = 0;

  TRACE_FUNCTION("cmd_flushCnmiBuf");

  if ( cnmiBuf.sCnmiElement[0].type EQ CNMI_NONE )
  {
    entries_read = CNMI_BUF_SIZE; /* no need to read anything at all */
    /*moreEntries = FALSE; */
  }
  else if ( cnmiBuf.next                            EQ CNMI_BUF_SIZE OR
            cnmiBuf.sCnmiElement[cnmiBuf.next].type EQ CNMI_NONE )
  {
    i = 0;
  }

  cnmiFlushInProgress = TRUE;

  while ( entries_read < CNMI_BUF_SIZE /*moreEntries*/ )
  {
    if (smsShrdPrm.pDecMsg)
    { /* clear p_sm buffer */
      ACI_MFREE(smsShrdPrm.pDecMsg);
      smsShrdPrm.pDecMsg = NULL;
    }

    switch ( cnmiBuf.sCnmiElement[i].type )
    {
      
      case ( CNMI_CMT ):
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMT ( &cnmiBuf.sCnmiElement[i].indct.cmt );
        break;

      case ( CNMI_CMTI ):
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMTI ( cnmiBuf.sCnmiElement[i].indct.cmti.mem,
                       cnmiBuf.sCnmiElement[i].indct.cmti.index );
        break;

      case ( CNMI_CBM ):
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCBM (  &cnmiBuf.sCnmiElement[i].indct.cbm );
        break;

      case ( CNMI_CDS ):
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCDS (  &cnmiBuf.sCnmiElement[i].indct.cds );
        break;

      default:
/*      moreEntries = FALSE; */
        break;
    }

    cnmiBuf.sCnmiElement[i].type = CNMI_NONE;
    i++;
    entries_read++;

    if ( i EQ CNMI_BUF_SIZE )
      i = 0;
  }

  cnmiFlushInProgress = FALSE;
  cnmiBuf.next        = 0;
}

/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD                  |
| STATE   : code                        ROUTINE : cmd_flushCnmiBufOneByOne |
+--------------------------------------------------------------------------+

  PURPOSE : This function flushes the CNMI buffer in case of Phase2+ mode,
            while the serial interface is released from
            data transmission or an +CNMI <bfr>==Flush is requested.
*/

GLOBAL void cmd_flushCnmiBufOneByOne(void)
{ /* flushing +CMT one by one, because acknowledge is necessary */
  T_CNMI_BUFFER_ELEMENT* psMsgInCnmiBuffer = NULL;
  UINT16                 uiCnmiMsgCounter  = 0;

  for( uiCnmiMsgCounter=0;
       (uiCnmiMsgCounter<cmd_getNumberOfCnmiEntrys()) AND 
       (waitForCnmaFromBuffer_SrcId EQ CMD_SRC_NONE); 
       uiCnmiMsgCounter++ )
  { /* there are entrys in the CNMI buffer --> read it */
    psMsgInCnmiBuffer = cmd_getCnmiMessage(uiCnmiMsgCounter); /* FIFO */

    if( psMsgInCnmiBuffer EQ NULL )
    {
      TRACE_EVENT("ati_switch_mode() : Error at CNMI-buffer handling: NULL-pointer access!!");
      return;
    }
    
    cnmiFlushInProgress = TRUE; /* needed for the rCI_...commands */

    if (smsShrdPrm.pDecMsg)
    { /* clear p_sm buffer */
      ACI_MFREE(smsShrdPrm.pDecMsg);
      smsShrdPrm.pDecMsg = NULL;
    }

    switch ( psMsgInCnmiBuffer->type )
    {
      case ( CNMI_CMTI ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMTI ( psMsgInCnmiBuffer->indct.cmti.mem, psMsgInCnmiBuffer->indct.cmti.index );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CBM ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCBM (  &psMsgInCnmiBuffer->indct.cbm );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CDS ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCDS (  &psMsgInCnmiBuffer->indct.cds );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      /* special handling for +CMT --> acknowledge needed */
      case ( CNMI_CMT ) :
      {
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMT ( &psMsgInCnmiBuffer->indct.cmt );
        TIMERSTART(ACI_CNMA_TIMER_VALUE, ACI_CNMA_TIMER_HANDLE);
        waitForCnmaFromBuffer_SrcId = smsShrdPrm.smsSrcId;
        break;
      }
      default:
        TRACE_EVENT("ati_switch_mode() : wrong CNMI-buffer handling: wrong Msg. type");
    }
  }
  cnmiFlushInProgress = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cmd_addCnmiNtry    |
+--------------------------------------------------------------------+

  PURPOSE : This function add a new entry to the buffer for storing
            new message indications while the serial interface is
            reserved for data transmission.
*/
GLOBAL void cmd_addCnmiNtry ( UBYTE type, T_CNMI_IND* newInd )
{
  TRACE_FUNCTION("cmd_addCnmiNtry");

  if ( type > CNMI_CDS OR type < CNMI_CMT )
    return;

  if ( cnmiBuf.next EQ CNMI_BUF_SIZE )
  {
    cnmiBuf.next = 0;
  }

  switch ( type )
  {
    case ( CNMI_CMT ):
      cnmiBuf.sCnmiElement[cnmiBuf.next].indct.cmt  = newInd -> cmt;
      break;

    case ( CNMI_CMTI ):
      cnmiBuf.sCnmiElement[cnmiBuf.next].indct.cmti = newInd -> cmti;
      break;


    case ( CNMI_CBM ):
      cnmiBuf.sCnmiElement[cnmiBuf.next].indct.cbm  = newInd -> cbm;
      break;

    case ( CNMI_CDS ):
      cnmiBuf.sCnmiElement[cnmiBuf.next].indct.cds  = newInd -> cds;
      break;
  }

  cnmiBuf.sCnmiElement[cnmiBuf.next].type  = type;
  cnmiBuf.next++;
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : getNumberOfCnmiEntrys |
+-----------------------------------------------------------------------+

  PURPOSE : This function is the responsible to get the count 
            of CNMI entrys in the CNMI buffer.
*/
GLOBAL UINT16 cmd_getNumberOfCnmiEntrys(void)
{
  return cnmiBuf.next;
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : getCnmiMessage        |
+-----------------------------------------------------------------------+

  PURPOSE : This function is the responsible to get an entire Msg.
            from the CNMI buffer. That means, this function only returns
            the pointer to the beginning of the CNMI buffer element.
            If the buffer is empty or if the uiIndex parameter is bigger
            than allowed, the return pointer will be NULL.
*/
GLOBAL T_CNMI_BUFFER_ELEMENT* cmd_getCnmiMessage(UINT16 uiIndex)
{
  T_CNMI_BUFFER_ELEMENT* psCnmiBufferElement = NULL;

  if( (cnmiBuf.next EQ 0) OR
      (cnmiBuf.sCnmiElement[0].type EQ CNMI_NONE) OR
      (uiIndex >= CNMI_BUF_SIZE ) )
  {
    return NULL;
  }

  psCnmiBufferElement = &cnmiBuf.sCnmiElement[uiIndex];
  return psCnmiBufferElement; /* returns the pointer to the element */
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : clearFirstCnmiMessage |
+-----------------------------------------------------------------------+

  PURPOSE : This function is the responsible to clear an entire Msg.
            specified by uiIndex from the CNMI buffer.
            After succesful return, the message was delete from the CNMI
            buffer and the buffer has been resorted.
*/
GLOBAL BOOL cmd_clearCnmiMessage(UINT16 uiIndex)
{
  UINT16 uiBufCount;

  if( (cnmiBuf.next EQ 0) OR
      (cnmiBuf.sCnmiElement[0].type EQ CNMI_NONE))
  {
    return FALSE; /* CNMI buffer is empty or flushing is in progress */
  }

  /* delete first entry while resorting the buffer */
  for( uiBufCount = uiIndex; uiBufCount<(CNMI_BUF_SIZE-1); uiBufCount++ )
  {
    memcpy( &cnmiBuf.sCnmiElement[uiBufCount], &cnmiBuf.sCnmiElement[uiBufCount+1], sizeof(T_CNMI_BUFFER_ELEMENT) );
  }

  cnmiBuf.next--;

  return TRUE;
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : getCnmiMessage        |
+-----------------------------------------------------------------------+

  PURPOSE : This function is the responsible to get the first entire Msg.
            from the CNMI buffer. That means, this function only returns
            the pointer to the beginning of the CNMI buffer.
            If the buffer is empty, the return pointer will be NULL.
*/
GLOBAL T_CNMI_BUFFER_ELEMENT* cmd_getFirstCnmiMessage(void)
{
  return cmd_getCnmiMessage(0);
}


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : clearFirstCnmiMessage |
+-----------------------------------------------------------------------+

  PURPOSE : This function is the responsible to clear the first entire Msg.
            from the CNMI buffer. After succesful return, the 
            message was delete from the CNMI buffer.
*/
GLOBAL BOOL cmd_clearFirstCnmiMessage(void)
{
  return cmd_clearCnmiMessage(0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : rCI_IoMode         |
+--------------------------------------------------------------------+

  PURPOSE : This function will be called when IO mode of serial
            interface has changed.
*/
GLOBAL void rCI_IoMode ( void )
{
  TRACE_FUNCTION ( "rCI_IoMode (): actually not awaited !!" );

/*  if ( io_getIoMode () EQ IO_MODE_CMD AND
       at.CNMI_mode    EQ CNMI_MOD_Buffer )
    cmd_flushCnmiBuf (CMD_SRC_ATI);*/
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentCMGRS  |
+--------------------------------------------------------------------+

  PURPOSE : %CMGRS command (Select auto retransmission mode/
            request manual retransmission of last failed SMS)
*/
#ifdef REL99

GLOBAL T_ATI_RSLT setatPercentCMGRS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CMGRS_MODE mode;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentCMGRS()");

  cl = parse (cl,"d",&mode);
  if ( !cl OR mode > CMGRS_MODE_MANUAL_RETRANS)
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cmgrs cmgrs;
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CMGRS;
    cmd.params.ptr_set_percent_cmgrs = &cmgrs;

    cmgrs.mode = (U8)mode;
    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd = AT_CMD_P_CMGRS;
    return(ATI_EXCT);
  }
#else

  ret = sAT_PercentCMGRS((T_ACI_CMD_SRC)srcId,mode);

  if(ret EQ AT_EXCT)
  {
    src_params->curAtCmd = AT_CMD_P_CMGRS;
    return ATI_EXCT;
  }
  else if(ret EQ AT_CMPL)
  {
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError( CME_ERR_Unknown ); // use aciErrDesc
    return (ATI_FAIL);
  }
#endif
}

GLOBAL T_ATI_RSLT queatPercentCMGRS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  cmd.ctrl_params = BAT_RES_QUE_PERCENT_CMGRS;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_percent_cmgrs = &dummy;

  bat_send(ati_bat_get_client(srcId),&cmd);

  return ATI_EXCT;
#else
  UBYTE        auto_rep_flag;

  ret = qAT_PercentCMGRS((T_ACI_CMD_SRC)srcId,&auto_rep_flag);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s: %d","%CMGRS",auto_rep_flag);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  // use aciErrDesc
    return (ATI_FAIL);
  }
#endif
}

#endif /* REL99 */


/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : cmd_handleCnmaTimeout |
+-----------------------------------------------------------------------+

  PURPOSE : This function will be called when the +CNMA Timer has expired.
            Now all to be acknowledged +CMT messages whithin the CNMI
            buffer must be stored in the non-volatile buffer e.g. SIM...
            Note: all other Msgs. different from +CMT will be deliver
            to the source and remove from buffer. */

GLOBAL void cmd_handleCnmaTimeout( void )
{
  UINT16                 uiCnmiMsgCounter  = 0;
  T_CNMI_BUFFER_ELEMENT *psMsgInCnmiBuffer = NULL;

  TRACE_FUNCTION ( "cmd_handleCnmaTimeout()" );

  cmhSMS_resetMtDsCnmiParam();

  if( waitForCnmaFromBuffer_SrcId EQ CMD_SRC_NONE OR 
      waitForCnmaFromBuffer_SrcId NEQ smsShrdPrm.smsSrcId)
  {
    TRACE_EVENT("cmd_handleCnmaTimeout() : Error at CNMI-buffer handling: No +CNMA acknowledge expected!!");
    waitForCnmaFromBuffer_SrcId = CMD_SRC_NONE;
    return;
  }

  for( uiCnmiMsgCounter=0; uiCnmiMsgCounter<cmd_getNumberOfCnmiEntrys(); uiCnmiMsgCounter++ )
  { /* there are entrys in the CNMI buffer --> read it */
    psMsgInCnmiBuffer = cmd_getCnmiMessage(uiCnmiMsgCounter); /* FIFO */

    if( psMsgInCnmiBuffer EQ NULL )
    {
      TRACE_EVENT("cmd_handleCnmaTimeout() : Error at CNMI-buffer handling: NULL-pointer access!!");
      waitForCnmaFromBuffer_SrcId = CMD_SRC_NONE;
      return;
    }
    
    cnmiFlushInProgress = TRUE;

    switch ( psMsgInCnmiBuffer->type )
    {
      case ( CNMI_CMTI ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMTI ( psMsgInCnmiBuffer->indct.cmti.mem, psMsgInCnmiBuffer->indct.cmti.index );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CBM ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCBM (  &psMsgInCnmiBuffer->indct.cbm );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CDS ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCDS (  &psMsgInCnmiBuffer->indct.cds );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      /* special handling for +CMT --> acknowledge needed */
      case ( CNMI_CMT ) :
      {
        T_ACI_SM_DATA sPduData;

        waitForCnmaFromBuffer_SrcId = CMD_SRC_NONE; /* clear CNMA waiting flag */
        srcId_cb = smsShrdPrm.smsSrcId;
        sPduData.len = psMsgInCnmiBuffer->indct.cmt.sms_sdu.l_buf / 8; /* l_buf is in BIT !! */
        memcpy( &sPduData.data, &psMsgInCnmiBuffer->indct.cmt.sms_sdu.buf, SIM_PDU_LEN );
        cmhSMS_storePduToSim( smsShrdPrm.smsSrcId, SMS_RECORD_REC_UNREAD, &sPduData );
        cnmiFlushInProgress = FALSE;
        return; /* only one SMS can be stored at the SIM here */
      }

      default:
        TRACE_EVENT("cmd_handleCnmaTimeout() : wrong CNMI-buffer handling: wrong Msg. type");
    }
    cnmiFlushInProgress = FALSE;
  }
}

/*
+-----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD               |
| STATE   : code                        ROUTINE : cmd_handleCnmaTimeout |
+-----------------------------------------------------------------------+

  PURPOSE : This function stores the next message - correxponding
            to a specified source - into the non volatile memory.
            Note: all other Msgs. different from +CMT will be deliver
            to the source and remove from buffer.
*/
GLOBAL BOOL cmd_storeNextCnmiBufMsgToSim( void )
{
  UINT16                 uiCnmiMsgCounter  = 0;
  T_CNMI_BUFFER_ELEMENT *psMsgInCnmiBuffer = NULL;

  TRACE_FUNCTION ( "cmd_storeNextCnmiBufMsgToSim()" );

  if( smsShrdPrm.uiInternalSmsStorage EQ CMD_SRC_NONE OR
      smsShrdPrm.uiInternalSmsStorage NEQ smsShrdPrm.smsSrcId)
  {
    TRACE_EVENT("cmd_storeNextCnmiBufMsgToSim() : no valid source -> abort !!");
    smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
    return(FALSE);
  }

  for( uiCnmiMsgCounter=0; uiCnmiMsgCounter<cmd_getNumberOfCnmiEntrys(); uiCnmiMsgCounter++ )
  { /* there are entrys in the CNMI buffer --> read it */
    psMsgInCnmiBuffer = cmd_getCnmiMessage(uiCnmiMsgCounter); /* FIFO */

    if( psMsgInCnmiBuffer EQ NULL )
    {
      TRACE_EVENT("cmd_storeNextCnmiBufMsgToSim() : Error at CNMI-buffer handling: NULL-pointer access!!");
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
      return(FALSE);
    }
    
    cnmiFlushInProgress = TRUE;

    switch ( psMsgInCnmiBuffer->type )
    {
      case ( CNMI_CMTI ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCMTI ( psMsgInCnmiBuffer->indct.cmti.mem, psMsgInCnmiBuffer->indct.cmti.index );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CBM ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCBM (  &psMsgInCnmiBuffer->indct.cbm );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      case ( CNMI_CDS ) :
        srcId_cb = smsShrdPrm.smsSrcId;
        rCI_PlusCDS (  &psMsgInCnmiBuffer->indct.cds );
        cmd_clearCnmiMessage(uiCnmiMsgCounter);
        uiCnmiMsgCounter--;
        break;

      /* special handling for +CMT --> acknowledge needed */
      case ( CNMI_CMT ) :
      {
        T_ACI_SM_DATA sPduData;

        srcId_cb = smsShrdPrm.smsSrcId;
        sPduData.len = psMsgInCnmiBuffer->indct.cmt.sms_sdu.l_buf / 8; /* l_buf is in BIT !! */
        memcpy( &sPduData.data, &psMsgInCnmiBuffer->indct.cmt.sms_sdu.buf, SIM_PDU_LEN );
        cmhSMS_storePduToSim( smsShrdPrm.smsSrcId, SMS_RECORD_REC_UNREAD, &sPduData );
        cnmiFlushInProgress = FALSE;
        return(TRUE); /* only one SMS can be stored at the SIM here */
      }

      default:
        TRACE_EVENT("cmd_handleCnmaTimeout() : wrong CNMI-buffer handling: wrong Msg. type");
    }
    cnmiFlushInProgress = FALSE;
  }
  smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE; /* no more +CMT has to be store */
  return(FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCMMS      |
+--------------------------------------------------------------------+

  PURPOSE : +CMMS command (More Messages To Send)
*/

GLOBAL T_ATI_RSLT setatPlusCMMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  S8  mode = 0;

  TRACE_FUNCTION("setatPlusCMMS()");

  cl = parse (cl,"r",&mode);

  /*
   * no mode values allowed other than 0-2
   */
  if ( !cl OR (mode < CMMS_MODE_DEF OR mode > CMMS_MODE_TWO))
  {
    cmdCmsError(CMS_ERR_OpNotAllowed);
    return (ATI_FAIL);
  }
  ret = sAT_PlusCMMS((T_ACI_CMD_SRC)srcId,mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmsError(CMS_ERR_UnknownErr);  
    return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCMMS      |
+--------------------------------------------------------------------+

  PURPOSE : +CMMS query command 
*/
GLOBAL T_ATI_RSLT queatPlusCMMS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  UBYTE         mode = CMMS_MODE_DEF;
  
  TRACE_FUNCTION("queatPlusCMMS()");

  ret = qAT_PlusCMMS ((T_ACI_CMD_SRC)srcId,&mode);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa, "+CMMS: %d",  mode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmsError(CMS_ERR_NotPresent);  
    return (ATI_FAIL);
  }
}

/*
+-------------------------------------------------------------------------+
| PROJECT : GSM-F&D                      MODULE  : ATI_SMS                |
| STATE   : finished                     ROUTINE : txt_rdmode_to_smsrdmode|
+-------------------------------------------------------------------------+

  PURPOSE : converts the text read mode to smsReadMode.

*/
GLOBAL T_ATI_RSLT txt_rdmode_to_smsrdmode(char *txt_rdmode)
{
  UBYTE i = 0;

   if (txt_rdmode[0] EQ '\0')
   {
     strcpy (txt_rdmode, sms_rdmode[0].name); /* NORMAL as default */
   }
   while ( sms_rdmode[i].name NEQ NULL      AND
              strcmp ( sms_rdmode[i].name, txt_rdmode ) NEQ 0 )
   {
      i++;
   }

   if ( sms_rdmode[i].name EQ NULL )
   {
      cmdCmsError ( CMS_ERR_OpNotAllowed );
      return (ATI_FAIL);
   }
   smsReadMode  = sms_rdmode[i].rdmode;
   return ATI_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCMGL      |
+--------------------------------------------------------------------+

  PURPOSE : %CMGL command (List Message)
*/

GLOBAL T_ATI_RSLT atPercentCMGL (char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
#endif
  CHAR           txtStat[15]= {0};
  CHAR           txtRdMode[20] = {0};
  USHORT         i;

  TRACE_FUNCTION("atPercentCMGL()");

  cmglStat = SMS_STAT_NotPresent;
  smsReadMode  = SMS_READ_NotPresent;

  if (*cl EQ '\0')
  {
    smsReadMode  = SMS_READ_Normal;
  }
  else
  {
    /*
     * Request of a list of stored SMS messages
     * Parameter stat is optional
     */
#ifdef SMS_PDU_SUPPORT
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
    if (mode EQ 0)
    {

      /*
       * %CMGL=[stat],[preview]
       */
      cl = parse (cl, "dd", &cmglStat, &smsReadMode );
      if ( !cl 
            OR ((cmglStat < SMS_STAT_RecUnread OR cmglStat > SMS_STAT_All)
                  AND smsReadMode  EQ SMS_READ_NotPresent) 
            OR (cmglStat < SMS_STAT_RecUnread AND smsReadMode > SMS_READ_NotPresent) 
            OR (smsReadMode  > SMS_READ_StatusChange))
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
    }
    else
#endif
    {

      /*
       * %CMGL=[stat],[preview]
       */
      cl = parse (cl, "ss", (LONG)sizeof(txtStat), txtStat, 
                  (LONG)sizeof(txtRdMode), txtRdMode);
      if ( !cl OR (txtStat[0] EQ '\0' AND txtRdMode[0] NEQ '\0'))
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
      if (txtStat[0] EQ '\0')
        strcpy (txtStat, sms_stat[4].name); /* ALL as default */

      i = 0;
      while ( sms_stat[i].name NEQ NULL            AND
              strcmp ( sms_stat[i].name, txtStat ) NEQ 0 )
        i++;

      if ( sms_stat[i].name EQ NULL )
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
      cmglStat = sms_stat[i].stat;
      
      if (txt_rdmode_to_smsrdmode(txtRdMode) EQ ATI_FAIL)
      {
        return (ATI_FAIL);
      }
    }
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cmgl cmgl;

    cmd.ctrl_params=BAT_CMD_SET_PERCENT_CMGL;
    cmd.params.ptr_set_percent_cmgl=&cmgl;

    /*
    *   T_ACI_SMS_STAT and T_BAT_percent_cmgl_stat are not quite
    *   identical - the ACI version has the extra value
    *   SMS_STAT_Invalid.
    */
    if (cmglStat EQ SMS_STAT_Invalid)
      cmgl.stat=BAT_CMGL_STAT_NOT_PRESENT;
    else
      cmgl.stat=(T_BAT_percent_cmgl_stat)cmglStat;

    cmgl.rdmode = smsReadMode ;

    bat_send(ati_bat_get_client(srcId),&cmd);

    src_params->curAtCmd=AT_CMD_P_CMGL;
    return(ATI_EXCT);
  }

#else

  ret = sAT_PercentCMGL ( (T_ACI_CMD_SRC)srcId, cmglStat, smsReadMode  );

  if ( ret EQ AT_EXCT )
  {
    src_params->curAtCmd = AT_CMD_P_CMGL;
    return (ATI_EXCT);
  }
  else
  {
    cmdCmsError ( CMS_ERR_NotPresent );  
    return (ATI_FAIL);
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCMGR         |
+--------------------------------------------------------------------+

  PURPOSE : %CMGR command (Read Message)
*/

GLOBAL T_ATI_RSLT atPercentCMGR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  SHORT idx=-1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
#endif
  CHAR           txtRdMode[20]={0};


  TRACE_FUNCTION("atPercentCMGR()");

  smsReadMode  = SMS_READ_Normal;

#ifdef SMS_PDU_SUPPORT
    /*
     * request current mode
     */
    qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
    if (mode EQ 0)
    {
      /*
       * %CMGR=[index],[preview]
       */
      cl = parse (cl, "rd", &idx, &smsReadMode );
      if ( !cl OR idx > 255 OR idx < 0 OR smsReadMode  > SMS_READ_StatusChange)
      {
        cmdCmsError ( CMS_ERR_OpNotAllowed );
        return (ATI_FAIL);
      }
    }
    else
#endif
    {
      /*
       * %CMGR=[index],[preview]
       */
      cl = parse (cl, "rs", &idx, 
                  (LONG)sizeof(txtRdMode), txtRdMode);
  
      if ( !cl OR idx > 255 OR idx < 0)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }
      if (txt_rdmode_to_smsrdmode(txtRdMode) EQ ATI_FAIL)
      {
        return (ATI_FAIL);
      }
    }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cmgr cmgr;

    cmd.ctrl_params=BAT_CMD_SET_PERCENT_CMGR;
    cmd.params.ptr_set_percent_cmgr=&cmgr;

    cmgr.sms_index=(U16)idx;
    
    cmgr.rdmode = smsReadMode ;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_P_CMGR;
    return(ATI_EXCT);
  }

#else

#ifndef _CONC_TESTING_
  ret = sAT_PercentCMGR_Gl((T_ACI_CMD_SRC)srcId, (UBYTE)idx, smsReadMode , NULL);
#else
  ret = sAT_PercentCMGR (srcId, (UBYTE)idx, smsReadMode );
#endif
 if ( ret EQ AT_CMPL )
 {
   return (ATI_CMPL);
 }
 else if (ret EQ AT_EXCT)
 {
   src_params->curAtCmd    = AT_CMD_P_CMGR;
   return (ATI_EXCT);
 }
 else
 {
   cmdCmsError(CMS_ERR_NotPresent);  /* use aciErrDesc */
   return (ATI_FAIL);
 }

#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT tesatPercentCMGL (char *cl, UBYTE srcId)
{
#ifdef SMS_PDU_SUPPORT
  T_ACI_CMGF_MOD mode;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("tesatPercentCMGL()");
  /*
   * request current mode
   */
  qAT_PlusCMGF((T_ACI_CMD_SRC)srcId, &mode);
  if (mode EQ 0)
  /*
   * handle PDU mode
   */
    io_sendMessage(srcId, "+CMGL: (0,1,2,3,4), (0,1,2)", ATI_NORMAL_OUTPUT);
  else
  /*
   * handle Text mode
   */
#endif
   io_sendMessage(srcId, "+CMGL: (\"REC UNREAD\",\"REC READ\",\"STO UNSENT\",\"STO SENT\",\"ALL\"), \
                                 (\"READ NORMAL\",\"READ PREVIEW\",\"STATUS CHANGE\")", ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}

#endif /* ATI_SMS_C */

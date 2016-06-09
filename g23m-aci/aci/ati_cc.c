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
|  Purpose :  AT Command Interpreter: Call Control related commands.
+----------------------------------------------------------------------------- 
*/ 
#ifndef ATI_CC_C
#define ATI_CC_C

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
#include "aci_lst.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"

#include "aci.h"
#include "l4_tim.h"
#include "aci_mem.h"
#include "aci_prs.h"

#include "aci_lst.h"
#include "ati_int.h"

#ifdef  FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : cleanDialStr       |
+--------------------------------------------------------------------+

  PURPOSE : This function removes all invalid or unsupported characters
            from the null terminated dial string.

*/

LOCAL void cleanDialStr( CHAR * dialStr )
{
  USHORT  i,j,k = 0;
  USHORT  dialLen, charLen;
  CHAR   *valChrs = "0123456789*#+-ABCDPpWw";
  UBYTE   match = FALSE;

  dialLen = strlen(dialStr );
  charLen = strlen(valChrs );

  for (i=0;i<dialLen;i++)
  {
    match=FALSE;

    for (j=0;j<charLen;j++)
    {
      if (dialStr[i] EQ valChrs[j])
      {
        match = TRUE;
        break;
      }
    }

    if (match)
    {
      dialStr[k] = dialStr[i];
      k++;
    }
  }

  dialStr[k] = 0x0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE :                    |
+--------------------------------------------------------------------+

  PURPOSE : D (dial command)
*/

#define NAME_BUF_LEN 32
#define DIAL_BUF_LEN (MAX_DIAL_LEN-1)

GLOBAL T_ATI_RSLT atD (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret = AT_FAIL;
  char              alpha    [NAME_BUF_LEN]   = {'\0'};
  USHORT            lenAlpha                  = 0;
  char              cvtdAlpha[NAME_BUF_LEN]   = {'\0'};
  USHORT            lenCvtdAlpha              = 0;
  char              t        [DIAL_BUF_LEN]   = {'\0'};
  char*             t_p                       = NULL;
  USHORT            pos                       = 0;
  SHORT             index                     = -1;

  T_ACI_PB_STOR     mem_stor                  = PB_STOR_NotPresent;
  T_ACI_D_CLIR_OVRD clir_ovrd                 = D_CLIR_OVRD_Default;
  T_ACI_D_CUG_CTRL  cug_ctrl                  = D_CUG_CTRL_NotPresent;
  T_ACI_D_TOC       call_type                 = D_TOC_Data;
  T_ATI_SRC_PARAMS *src_params                = find_element (ati_src_list, srcId, search_ati_src_id);

  src_params->curAtCmd = AT_CMD_D;

#ifdef FF_ATI_BAT    
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_at_d bat_dial;

    cmd.ctrl_params     = BAT_CMD_AT_D;
    cmd.params.ptr_at_d = &bat_dial;
    
#warning "atD(): check for UCS2 dial string before bat_send()"

    bat_dial.c_dial_string = strlen(cl);
    if (bat_dial.c_dial_string)
    {
      memcpy (bat_dial.dial_string, (U8 *)cl, bat_dial.c_dial_string);

      TRACE_FUNCTION("atD() calls bat_send() <=== as APPLICATION");

      bat_send(ati_bat_get_client(srcId), &cmd);
      cmdErrStr = NULL;
      srcId_cb = srcId;
      rCI_PlusCCWV (CCWV_CHRG_NotPresent);    
      return (ATI_EXCT); /* executing, because response is passed by callback function */
    }
    else
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  }
#else /* no FF_ATI_BAT */
  {
    BOOL Book_Dial;
    U16  i;
#ifdef NO_ASCIIZ
    T_ACI_PB_TEXT     pbText;
#endif 
    
    TRACE_FUNCTION("atD()");

    t_p=t;
    if (*cl EQ '>')             /*call from Phonebook*/
    {
      Book_Dial=TRUE;
      cl++;
      if(*cl EQ '"')    /*origin call to name given by string */
      {
        cl=parse(cl,"z",
                 (LONG)NAME_BUF_LEN,
                 strlen(cl),
                 cl,
                 &lenAlpha,
                 alpha);
        if (cl EQ NULL)
        {
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }
        else if (strcspn(cl,"iIgG;") NEQ 0)
        {
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }
        srcId_cb = srcId;
  #ifdef NO_ASCIIZ
        utl_chsetToSim ( (UBYTE*)alpha,
                         lenAlpha,
                         (UBYTE*)cvtdAlpha,
                         &lenCvtdAlpha,
                         GSM_ALPHA_Def );
  #else  /* #ifdef NO_ASCIIZ */
        utl_chsetToGsm ( (UBYTE*)alpha,
                         lenAlpha,
                         (UBYTE*)cvtdAlpha,
                         &lenCvtdAlpha,
    #ifdef REL99
                         sizeof(cvtdAlpha),
    #endif
                         GSM_ALPHA_Int);
  #endif /* #ifdef NO_ASCIIZ */
        t_p = cvtdAlpha;
      }
      else
      {
        /* call using phonebook with pb specified*/
        if ((toupper(*cl)) >= 65 AND (toupper(*cl)) <= 90)
        {
          for (i=0;phb_mem_names[i].name != 0;i++)
          {
            strupper(cl);
            if (strncmp(cl,phb_mem_names[i].name,2) EQ 0)
            {
              mem_stor=phb_mem_names[i].stor;
              break;
            }
            else mem_stor=PB_STOR_NotPresent;
          }
          if (mem_stor < 0)
          {
            cmdCmeError(CME_ERR_OpNotAllow);
            return (ATI_FAIL);
          }
          cl=cl+2;
        }
        pos=strcspn(cl,"iIgG;");            /* determine index */
        if (pos EQ 0 OR pos >= DIAL_BUF_LEN)
        {
          TRACE_EVENT("incorrect strspn");
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }
        strncpy(t,cl,pos);
        cl=cl+pos;
        index =(SHORT)atoi(t);
   
        if(!index OR index > 300)
        {
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }
        t_p = NULL;
      }
    }
    else
    {
      Book_Dial=FALSE;
      pos=strcspn(cl,"iIgG;");
      if(pos >= DIAL_BUF_LEN)
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
      if (pos)
      {
        strncpy(t,cl,pos);
        cl=cl+pos;
        t[pos]='\0';
      }
      else
      {
       if(strlen(cl) >= DIAL_BUF_LEN)
       {
         cmdCmeError(CME_ERR_OpNotAllow);
         return (ATI_FAIL);
       }
       strcpy(t,cl);
      }

      cleanDialStr(t);
      if (t[0] EQ '\0')
        t_p=NULL;
    }

    if (*cl EQ 'i')
    {
      clir_ovrd=D_CLIR_OVRD_Supp;
      cl++;
    }
    if (*cl EQ 'I')
    {
      clir_ovrd=D_CLIR_OVRD_Invoc;
      cl++;
    }
    if (*cl EQ 'g'OR *cl EQ 'G')
    {
      cug_ctrl=D_CUG_CTRL_Present;
      cl++;
    }
    if (*cl EQ ';')
    {
      call_type=D_TOC_Voice;
      cl++;
    }
    if (*cl NEQ '\0')
    {
      cmdCmeError(CME_ERR_InvDialChar);
      return (ATI_FAIL);
    }

    if(Book_Dial)
    {
  #ifdef NO_ASCIIZ
      if ( t_p NEQ NULL )
      {
        pbText.cs = CS_Sim;
        pbText.len = (UBYTE)lenCvtdAlpha;
        memcpy( pbText.data, t_p, pbText.len );
      }
      else
      {
        pbText.cs = CS_NotPresent;
        pbText.len = 0;
      }
      ret = sAT_Dm( (T_ACI_CMD_SRC)srcId, &pbText, mem_stor, index, clir_ovrd,
                    cug_ctrl, call_type );
  #else  /* #ifdef NO_ASCIIZ */
      ret = sAT_Dm( srcId, t_p, mem_stor, index, clir_ovrd,
                    cug_ctrl, call_type );
  #endif /* #ifdef NO_ASCIIZ */
    }
    else
    {
      ret=sAT_Dn((T_ACI_CMD_SRC)srcId,t_p,clir_ovrd,cug_ctrl,call_type);
    }

    if (ret EQ AT_EXCT)
    {
      if (call_type EQ D_TOC_Voice AND
          at.flags.COLP_stat NEQ 1   )
      {
        cmdErrStr = NULL;
      }
      else
      {
        src_params->curAtCmd    = AT_CMD_D;
      }
      srcId_cb = srcId;
      rCI_PlusCCWV ( CCWV_CHRG_NotPresent );

      return (ATI_EXCT);
    }
    else if( ret EQ AT_CMPL )
    {
      srcId_cb = srcId;
      rCI_OK (AT_CMD_D);
      return (ATI_CMPL_NO_OUTPUT);
    }
    else
    {
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
    }
  }
#endif /* no FF_ATI_BAT */ 
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atA                |
+--------------------------------------------------------------------+

  PURPOSE : A command (Answer a call)
*/

GLOBAL T_ATI_RSLT atA(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret             = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  T_BAT_cmd_send     cmd;
  T_BAT_no_parameter dummy;
    
  TRACE_FUNCTION("atA() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params     = BAT_CMD_AT_A;
  dummy.bat_dummy     = 0xFF;
  cmd.params.ptr_at_a = &dummy;

  if( at.rngPrms.isRng EQ TRUE )
  {
    ati_stop_ring();
  }
  src_params->curAtCmd = AT_CMD_A;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return (ATI_EXCT); /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */
  
  TRACE_FUNCTION("atA()");

  if(*cl NEQ '\0')
  {
    cmdAtError(atError);
  }
  else
  {
    ret=sAT_A((T_ACI_CMD_SRC)srcId);
    if (ret EQ AT_EXCT)
    {
      if( at.rngPrms.isRng EQ TRUE )
      {
        ati_stop_ring();
      }
      src_params->curAtCmd = AT_CMD_A;
    }
    else
    {
      cmdAtError(atError);
    }
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atH                |
+--------------------------------------------------------------------+

  PURPOSE : H command (Hangs up single mode call)
*/
GLOBAL T_ATI_RSLT atH(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret             = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  T_BAT_cmd_send     cmd;
  T_BAT_no_parameter dummy;
  cmd.ctrl_params     = BAT_CMD_AT_H;
  dummy.bat_dummy     = 0xFF;
  cmd.params.ptr_at_h = &dummy;
    
  TRACE_FUNCTION("atH() calls bat_send() <=== as APPLICATION");

  if( at.rngPrms.isRng EQ TRUE )
  {
    ati_stop_ring();
  }
  src_params->curAtCmd = AT_CMD_H;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return (ATI_EXCT); /* executing, because response is passed by callback function */
#else /* no FF_ATI_BAT */

  TRACE_FUNCTION("atH()");

  if (*cl EQ '0' OR *cl EQ '\0')
  {
    if (*cl EQ  '0')
    {
      cl++;
    }

    ret=sAT_H((T_ACI_CMD_SRC)srcId);
    if (ret EQ AT_EXCT)
    {
      if( at.rngPrms.isRng EQ TRUE )
      {
        ati_stop_ring();
      }
      src_params->curAtCmd    = AT_CMD_H;
    }
    else
    {
      cmdAtError(atError);
    }
  }
  else
  {
    cmdAtError(atError);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

#ifdef FAX_AND_DATA
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCBST         |
+--------------------------------------------------------------------+

  PURPOSE : +CBST command (Select Bearer Service Type)
*/

GLOBAL T_ATI_RSLT setatPlusCBST(char *cl, UBYTE srcId)
{
  T_ACI_BS_SPEED  speed = BS_SPEED_NotPresent;
  T_ACI_CBST_NAM  name  = CBST_NAM_NotPresent;
  T_ACI_CBST_CE   ce    = CBST_CE_NotPresent;
  T_ACI_RETURN    ret   = AT_FAIL;

  cl = parse(cl, "ddd", &speed, &name, &ce);
  if(!cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cbst my_bat_set_plus_cbst;

  TRACE_FUNCTION("setatPlusCBST() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cbst, 0, sizeof(my_bat_set_plus_cbst));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CBST;
  cmd.params.ptr_set_plus_cbst = &my_bat_set_plus_cbst;

  my_bat_set_plus_cbst.speed = speed;
  my_bat_set_plus_cbst.name = name;
  my_bat_set_plus_cbst.ce = ce;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("atPLusCBST()");
  {
    ret = sAT_PlusCBST((T_ACI_CMD_SRC)srcId,speed,name,ce);
  }
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  return (ATI_CMPL);

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCBST(char *cl, UBYTE srcId)
{
  T_ACI_BS_SPEED  speed = BS_SPEED_NotPresent;
  T_ACI_CBST_NAM  name  = CBST_NAM_NotPresent;
  T_ACI_CBST_CE   ce    = CBST_CE_NotPresent;
  T_ACI_RETURN    ret   = AT_FAIL;

  TRACE_FUNCTION("atPLusCBST()");

  ret = qAT_PlusCBST((T_ACI_CMD_SRC)srcId,&speed,&name,&ce);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"eee",&speed,&name,&ce);
  }
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);  /* AT_FAIL, AT_EXCT, AT_BUSY ??? */
    return (ATI_FAIL);
  }
  return (ATI_CMPL);

}
#endif /* FAX_AND_DATA */

/*
PURPOSE : Change some AT interface output configuration
            (+CRC +CR +CLIP +COLP)
*/
GLOBAL T_ATI_RSLT GenAtCR_C( CHAR *cl, UBYTE srcId, T_ACI_AT_CMD cmd )
{
  UBYTE x = 0;

  switch(*cl)
  {
    case('0'):
    case('1'):
      /* value ok */
      break;
    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }

  x = *cl - '0';
  cl++;

  switch(cmd)
  {
    case( AT_CMD_CRC ):
      ati_user_output_cfg[srcId].CRC_stat = x;
      return (ATI_CMPL);
    case( AT_CMD_CR ):
      ati_user_output_cfg[srcId].CR_stat = x;
      return (ATI_CMPL);
    case( AT_CMD_CLIP ):
      ati_user_output_cfg[srcId].CLIP_stat = x;
      return (ATI_CMPL);
    case( AT_CMD_CDIP ):
      ati_user_output_cfg[srcId].CDIP_stat = x;
      return (ATI_CMPL);
    case( AT_CMD_COLP ):
      at.flags.COLP_stat = x;
      return (ATI_CMPL);
  }

  cmdCmeError(CME_ERR_OpNotAllow);
  return (ATI_FAIL);
}

#ifdef FF_TTY
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCTTY      |
+--------------------------------------------------------------------+

  PURPOSE : %CTTY command (handle CTM/TTY service)
*/

GLOBAL T_ATI_RSLT setatPercentCTTY (char *cl, UBYTE srcId)
{
  T_ACI_CTTY_MOD mode = CTTY_MOD_NotPresent;
  T_ACI_CTTY_REQ req  = CTTY_REQ_NotPresent;

  cl = parse (cl, "dd", &mode, &req);

  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_ctty my_bat_set_percent_ctty;

  TRACE_FUNCTION("setatPercentCTTY() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_ctty, 0, sizeof(my_bat_set_percent_ctty));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CTTY;
  cmd.params.ptr_set_percent_ctty = &my_bat_set_percent_ctty;

  my_bat_set_percent_ctty.req = req;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */
  {
    T_ACI_RETURN   ret;

    TRACE_FUNCTION("setatPercentCTTY()");

    ret = sAT_PercentCTTY ((T_ACI_CMD_SRC)srcId, mode, req);
    switch (ret)
    {
    case (AT_CMPL):                         /* operation completed */
      break;
    case (AT_EXCT):                         /* not valid */
      ret = AT_CMPL;
      break;
    default:
      cmdCmeError(CME_ERR_NotPresent);      /* Command failed */
      break;
    }
    return (map_aci_2_ati_rslt(ret));
  }
#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPercentCTTY (char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret;
  T_ACI_CTTY_MOD  mode;
  T_ACI_CTTY_REQ  req;
  T_ACI_CTTY_STAT state;
  T_ACI_CTTY_TRX  trx;

  TRACE_FUNCTION("queatPercentCTTY()");

  ret = qAT_PercentCTTY ((T_ACI_CMD_SRC)srcId, &mode, &req, &state, &trx);

  if (ret EQ AT_CMPL)
  {
    if (trx EQ CTTY_TRX_Unknown)
      resp_disp (srcId, cl,"eee",&mode, &req, &state);
    else
      resp_disp (srcId, cl,"eeee",&mode, &req, &state, &trx);
    return ATI_CMPL;
  }
  else
    cmdCmeError (CME_ERR_Unknown);

  return ATI_FAIL;
}
#endif  /* FF_TTY */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCRC          |
+--------------------------------------------------------------------+

  PURPOSE : +CRC command (Select extended incoming call report)
*/

GLOBAL T_ATI_RSLT setatPlusCRC (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCRC()");

  return(GenAtCR_C( cl, srcId, AT_CMD_CRC ));
}

GLOBAL T_ATI_RSLT queatPlusCRC (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPlusCRC()");

  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].CRC_stat);
  return (ATI_CMPL);
}

/*>
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSTA         |
+--------------------------------------------------------------------+

  PURPOSE : +CSTA command (Select type of address)
*/

GLOBAL T_ATI_RSLT setatPlusCSTA(char *cl, UBYTE srcId)
{
  SHORT         toa_val=0;
  T_ACI_TOA     type;
#ifdef FF_ATI_BAT
  T_ACI_TOA *p_type=NULL;
#endif /* FF_ATI_BAT */
  T_ACI_RETURN   ret = AT_FAIL;

  cl = parse(cl, "r", &toa_val);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT
  p_type = &type;
  if (toa_val EQ 0)
    p_type = NULL;
  else
#endif /* FF_ATI_BAT */ 
  {
    type=toa_demerge(toa_val);
    if (type.ton < 0 OR type.npi < 0)
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_csta my_bat_set_plus_csta;

  TRACE_FUNCTION("setatPlusCSTA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_csta, 0, sizeof(my_bat_set_plus_csta));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CSTA;
  cmd.params.ptr_set_plus_csta = &my_bat_set_plus_csta;

  my_bat_set_plus_csta.type = (S16)p_type;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCSTA()");

  ret = sAT_PlusCSTA((T_ACI_CMD_SRC)srcId, &type);

  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCSTA(char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_TOA     type;
  SHORT         toa_val = 0;

  TRACE_FUNCTION("queatPLusCSTA()");

  ret     = qAT_PlusCSTA((T_ACI_CMD_SRC)srcId,&type);
  toa_val = toa_merge (type);
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"s",&toa_val);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCHLD         |
+--------------------------------------------------------------------+

  PURPOSE : common function for +/%CHLD command
*/
LOCAL T_ATI_RSLT setatPlus_PercentCHLD(char *cl, UBYTE srcId, T_ACI_CHLD_CMD cmdType)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  T_ACI_CHLD_MOD    mode       = CHLD_MOD_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  CHAR              CHLD_type  = '\0';
  CHAR              X_prm[2];

  X_prm[0] = (CHAR)NOT_PRESENT_8BIT;
  
  /* check first parameter of CHLD */
  CHLD_type = *cl;
  ++cl;

  /* check for special PERCENT command */
   if((CHLD_type EQ '6') AND (cmdType NEQ CHLD_PercentCmd))
   {
     cmdCmeError(CME_ERR_OpNotAllow);
     return (ATI_FAIL);
   }

  /* check for rest of parameter if necessary (only for AT+CHLD=1 / 2 / or 4) */
  if(  CHLD_type EQ '1'
    OR CHLD_type EQ '2'
    OR CHLD_type EQ '4'
    OR CHLD_type EQ '6'
    OR CHLD_type EQ '7' )
  {
    /* possibility of an X or directory parameter */
    if (*cl EQ 0 OR *cl EQ ';')  /* end of command */
    {
      X_prm[0] = (CHAR)NOT_PRESENT_8BIT;
    }
    else
    {
      if( *cl < '0' OR *cl > '9')
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
      else
      {
        X_prm[0] = *cl;
        X_prm[1] = '\0';
        cl++;
      }
    }
  }

  switch(CHLD_type)
  {
    case ('0'):
      mode = CHLD_MOD_RelHldOrUdub;
      break;

    case ('1'):
      if ( X_prm[0] EQ (CHAR)NOT_PRESENT_8BIT )
      {
        mode = CHLD_MOD_RelActAndAcpt;
      }
      else
      {
        mode = CHLD_MOD_RelActSpec;
      }
      break;

    case('2'):
      if ( X_prm[0] EQ (CHAR)NOT_PRESENT_8BIT )
      {
        mode = CHLD_MOD_HldActAndAcpt;
      }
      else
      {
        mode = CHLD_MOD_HldActExc;
      }
      break;

    case('3'):
      mode = CHLD_MOD_AddHld;
      break;

    case('4'):
      mode = CHLD_MOD_Ect;
      break;

    case('5'):
      mode = CHLD_MOD_Ccbs;
      break;

    case('6'):
      /* S60/Symbian requires an extra mode in %CHLD only: 
         retrieving of an [specific] hold call
         without affecting of an waiting call */
      if ( X_prm[0] EQ (CHAR)NOT_PRESENT_8BIT )
      {
        mode = CHLD_MOD_RetrieveHoldCall;
      }
      else
      {
        mode = CHLD_MOD_RetrieveHoldCallSpec;
      }
      break;

    case('7'):
      /* BMI requires an extra mode in %CHLD only: 
         releasing of an [specific] hold call of any type */
      if ( X_prm[0] EQ (CHAR)NOT_PRESENT_8BIT )
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
      else
      {
        mode = CHLD_MOD_RelAnySpec;
      }
      break;

    case('h'):
    case('H'):
      /* FTA requires an extra mode: Put on hold (without
         accepting automatically waiting or held calls) */
      mode = CHLD_MOD_OnlyHold;
      break;
    
    case('i'):
    case('I'):
      /* Release dialing call, without dropping current call. */
      mode = CHLD_MOD_RelDialCall;
      break;
    
    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_chld my_bat_set_percent_chld;

  TRACE_FUNCTION("setatPlus_PercentCHLD() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_chld, 0, sizeof(my_bat_set_percent_chld));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CHLD;
  cmd.params.ptr_set_percent_chld = &my_bat_set_percent_chld;

  my_bat_set_percent_chld.n = mode;
  my_bat_set_percent_chld.x = (S16)&X_prm;
  src_params->curAtCmd = AT_CMD_CHLD;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlus_PercentCHLD()");

  if( X_prm[0] NEQ (CHAR)NOT_PRESENT_8BIT )
  {
    if(cmdType EQ CHLD_PercentCmd)
    {
      ret = sAT_PercentCHLD( (T_ACI_CMD_SRC)srcId, mode, X_prm );
    }
    else
    {
      ret = sAT_PlusCHLD( (T_ACI_CMD_SRC)srcId, mode, X_prm );
    }
  }
  else
  {
    if(cmdType EQ CHLD_PercentCmd)
    {
      ret = sAT_PercentCHLD( (T_ACI_CMD_SRC)srcId, mode, NULL );
    }
    else
    {
      ret = sAT_PlusCHLD( (T_ACI_CMD_SRC)srcId, mode, NULL );
    }
  }

  switch( ret )
  {
    case AT_CMPL:
      break;

    case AT_EXCT:
      src_params->curAtCmd    = AT_CMD_CHLD;
      break;

    case AT_BUSY:
      cmdCmeError(CME_ERR_OpNotAllow);
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCHLD         |
+--------------------------------------------------------------------+

  PURPOSE : +CHLD command
*/

GLOBAL T_ATI_RSLT setatPlusCHLD(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCHLD()");

  return( setatPlus_PercentCHLD(cl, srcId, CHLD_PlusCmd) );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCHLD         |
+--------------------------------------------------------------------+

  PURPOSE : %CHLD command (same as +CHLD but custom specific)
*/

GLOBAL T_ATI_RSLT setatPercentCHLD(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPercentCHLD()");

  return( setatPlus_PercentCHLD(cl, srcId, CHLD_PercentCmd) );
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCHUP         |
+--------------------------------------------------------------------+

  PURPOSE : +CHUP command (Hangs up call)
*/
GLOBAL T_ATI_RSLT setatPlusCHUP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCHUP()");

  if (*cl EQ ';' OR *cl EQ 0)
  {
#ifdef FF_ATI_BAT
   {
     T_BAT_cmd_send     cmd;
     T_BAT_no_parameter dummy;
     cmd.ctrl_params     = BAT_CMD_SET_PLUS_CHUP;
     dummy.bat_dummy     = 0xFF;
     cmd.params.ptr_set_plus_chup = &dummy;
    
     TRACE_FUNCTION("setatPlusCHUP() calls bat_send() <=== as APPLICATION");

     if( at.rngPrms.isRng EQ TRUE )
     {
       ati_stop_ring();
     }
     src_params->curAtCmd = AT_CMD_CHUP;

     bat_send(ati_bat_get_client(srcId), &cmd);

     return (ATI_EXCT); /* executing, because response is passed by callback function */
   }
#else /* OLD FUNCTION BODY */
    ret=sAT_PlusCHUP((T_ACI_CMD_SRC)srcId);
    if (ret EQ AT_CMPL)
    {
      return (ATI_CMPL);
    }

    else if (ret EQ AT_EXCT)
    {
      if( at.rngPrms.isRng EQ TRUE )
      {
        ati_stop_ring();
      }
      src_params->curAtCmd    = AT_CMD_CHUP;
    }
    else
    {
      cmdCmeError(CME_ERR_Unknown);
    }
#endif
  }
  else
  {
    cmdCmeError(CME_ERR_OpNotAllow);
  }
  return (map_aci_2_ati_rslt (ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLIP         |
+--------------------------------------------------------------------+

  PURPOSE : +CLIP command (calling line identification presentation)
*/
GLOBAL T_ATI_RSLT setatPlusCLIP (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCLIP()");

  return(GenAtCR_C( cl, srcId, AT_CMD_CLIP ));
}

GLOBAL T_ATI_RSLT queatPlusCLIP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  T_ACI_CLIP_STAT   stat       = CLIP_STAT_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPlusCLIP()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_QUE_PLUS_CLIP;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_clip = &dummy;
    src_params->curAtCmd = AT_CMD_CLIP;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PlusCLIP((T_ACI_CMD_SRC)srcId,&stat);

  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"be",&ati_user_output_cfg[srcId].CLIP_stat,&stat);
  }
  else if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_CLIP;
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));

#endif  /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCDIP         |
+--------------------------------------------------------------------+

  PURPOSE : +CDIP command (calling line identification presentation)
*/
GLOBAL T_ATI_RSLT setatPlusCDIP (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCDIP()");

  return(GenAtCR_C( cl, srcId, AT_CMD_CDIP ));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCDIP      |
+--------------------------------------------------------------------+

  PURPOSE : +CDIP command (calling line identification presentation)
*/
GLOBAL T_ATI_RSLT queatPlusCDIP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_CMPL;
  T_ACI_CDIP_STAT   stat       = CDIP_STAT_Unknown;  /* Return this for XT6 - No Network Interrogation */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_QUE_PLUS_CDIP;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cdip = &dummy;
    src_params->curAtCmd = AT_CMD_CDIP;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */
  TRACE_FUNCTION("queatPlusCDIP()");

  resp_disp(srcId, cl,"be",&ati_user_output_cfg[srcId].CDIP_stat,&stat);

  return (map_aci_2_ati_rslt (ret));
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCOLP         |
+--------------------------------------------------------------------+

  PURPOSE : +COLP command (connected line identification presentation)
*/

GLOBAL T_ATI_RSLT setatPlusCOLP (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCOLP()");

  return(GenAtCR_C( cl, srcId, AT_CMD_COLP ));
}

GLOBAL T_ATI_RSLT queatPlusCOLP (char *cl, UBYTE srcId)
{
  T_ACI_COLP_STAT   stat       = COLP_STAT_NotPresent;
  T_ACI_RETURN      ret        = AT_FAIL;
  UBYTE             x          = 0;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPlusCOLP()");

  ret = qAT_PlusCOLP((T_ACI_CMD_SRC)srcId,&stat);
  if (ret EQ AT_CMPL)
  {
    x = (at.flags.COLP_stat?1:0);
    resp_disp(srcId, cl,"be",&x,&stat);
  }
  else if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_COLP;
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLIR         |
+--------------------------------------------------------------------+

  PURPOSE : +CLIR command (connected line identification restriction)
*/

GLOBAL T_ATI_RSLT setatPlusCLIR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret  = AT_FAIL;
  T_ACI_CLIR_MOD  mode = CLIR_MOD_NotPresent;


  switch(*cl)
  {
    case('0'):
    {
      mode=CLIR_MOD_Subscript;
      cl++;
      break;
    }
    case('1'):
    {
      mode=CLIR_MOD_Invoc;
      cl++;
      break;
    }
    case('2'):
    {
      mode=CLIR_MOD_Supp;
      cl++;
      break;
    }
    case('\0'):
    {
      break;
    }
    default:
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  }

  if (!(*cl EQ 0 OR *cl EQ ';'))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_clir my_bat_set_plus_clir;

  TRACE_FUNCTION("setatPLusCLIR() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_clir, 0, sizeof(my_bat_set_plus_clir));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CLIR;
  cmd.params.ptr_set_plus_clir = &my_bat_set_plus_clir;

  my_bat_set_plus_clir.n = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCLIR()");

  ret = sAT_PlusCLIR((T_ACI_CMD_SRC)srcId,mode);
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCLIR (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  T_ACI_CLIR_STAT   stat       = CLIR_STAT_NotPresent;
  T_ACI_CLIR_MOD    mode       = CLIR_MOD_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPLusCLIR()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_QUE_PLUS_CLIR;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_clir = &dummy;
    src_params->curAtCmd = AT_CMD_CLIR;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else /* OLD FUNCTION BODY */

  ret = qAT_PlusCLIR ((T_ACI_CMD_SRC)srcId,&mode,&stat);
  switch (ret)
  {
/*  case AT_CMPL:
    {
      sprintf(sa,"%s%d,%d",me,mode,stat);
      io_sendMessage(sa, ATI_NORMAL_OUTPUT);
      break;
    } */          /* never happens as far as I know */
    case AT_EXCT:
    {
      src_params->curAtCmd    = AT_CMD_CLIR;
      break;
    }
    case AT_FAIL:
    {
      cmdCmeError (CME_ERR_Unknown);
      break;
    }
  }
  return (map_aci_2_ati_rslt (ret));
#endif /* FF_ATI_BAT */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCCUG         |
+--------------------------------------------------------------------+

  PURPOSE : +CCUG command (closed useg group settings)
*/
GLOBAL T_ATI_RSLT setatPlusCCUG (char * cl, UBYTE srcId)
{
  T_ACI_RETURN    ret  = AT_FAIL;
  T_ACI_CCUG_MOD  mode = CCUG_MOD_NotPresent;
  T_ACI_CCUG_IDX  idx  = CCUG_IDX_NotPresent;
  T_ACI_CCUG_INFO info = CCUG_INFO_NotPresent;

  cl = parse(cl, "ddd", &mode, &idx, &info);
  if (!cl)
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ccug my_bat_set_plus_ccug;

  TRACE_FUNCTION("setatPlusCCUG() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ccug, 0, sizeof(my_bat_set_plus_ccug));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CCUG;
  cmd.params.ptr_set_plus_ccug = &my_bat_set_plus_ccug;

  my_bat_set_plus_ccug.n = mode;
  my_bat_set_plus_ccug.index = idx;
  my_bat_set_plus_ccug.info = info;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusCCUG()");

  ret = sAT_PlusCCUG((T_ACI_CMD_SRC)srcId,mode,idx,info);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError (CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCCUG (char * cl, UBYTE srcId)
{
  T_ACI_RETURN    ret  = AT_FAIL;
  T_ACI_CCUG_MOD  mode = CCUG_MOD_NotPresent;
  T_ACI_CCUG_IDX  idx  = CCUG_IDX_NotPresent;
  T_ACI_CCUG_INFO info = CCUG_INFO_NotPresent;

  TRACE_FUNCTION("queatPlusCCUG()");

  ret = qAT_PlusCCUG ((T_ACI_CMD_SRC)srcId,&mode,&idx,&info);
  switch (ret)
  {
    case AT_CMPL:
    {
      resp_disp (srcId, cl,"eee",&mode,&idx,&info);
      break;
    }
    case AT_FAIL:
    {
      cmdCmeError (CME_ERR_Unknown);
      break;
     }
  }
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMOD         |
+--------------------------------------------------------------------+

  PURPOSE : +CMOD command (call mode)
*/

GLOBAL T_ATI_RSLT setatPlusCMOD (char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret  = AT_FAIL;
  T_ACI_CMOD_MOD mode = CMOD_MOD_NotPresent;

  cl = parse(cl, "d", &mode);
  if (!cl)
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cmod my_bat_set_plus_cmod;

  TRACE_FUNCTION("setatPLusCMOD() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cmod, 0, sizeof(my_bat_set_plus_cmod));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CMOD;
  cmd.params.ptr_set_plus_cmod = &my_bat_set_plus_cmod;

  my_bat_set_plus_cmod.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCMOD()");

  ret = sAT_PlusCMOD ((T_ACI_CMD_SRC)srcId,mode);
  switch (ret)
  {
  case AT_FAIL:
    cmdCmeError (CME_ERR_Unknown);
    break;

  default:
    break;
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCMOD (char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret  = AT_FAIL;
  T_ACI_CMOD_MOD mode = CMOD_MOD_NotPresent;

  TRACE_FUNCTION("queatPLusCMOD()");

  ret = qAT_PlusCMOD ((T_ACI_CMD_SRC)srcId,&mode);

  switch (ret)
  {
    case AT_CMPL:
    {
      resp_disp (srcId, cl, "e",&mode);
      break;
    } 
    case AT_FAIL:
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      break;
    } 
  }
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCR           |
+--------------------------------------------------------------------+

  PURPOSE : +CR command (Service reporting control)
*/

GLOBAL T_ATI_RSLT setatPlusCR (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPlusCR()");

  return (GenAtCR_C( cl, srcId, AT_CMD_CR ));
}

GLOBAL T_ATI_RSLT queatPlusCR (char *cl, UBYTE srcId)
{
  UBYTE x = 0;

  TRACE_FUNCTION("queatPlusCR()");

  x = (ati_user_output_cfg[srcId].CR_stat?1:0);

  resp_disp (srcId, cl, "b", &x);

  return (ATI_CMPL); 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCAOC         |
+--------------------------------------------------------------------+

  PURPOSE : +CAOC command (Advice of Charge)
*/
GLOBAL T_ATI_RSLT setatPlusCAOC (char* cl, UBYTE srcId)
{
  LONG          ccm  = 0L;
  SHORT         mode = 0;
  T_ACI_RETURN  ret  = AT_FAIL;

  switch ( *cl )
  {
    case('\0'):
      break;

    default:
    {
      cl = parse(cl, "d", &mode);
      if (!cl)
      {
        cmdCmeError (CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }

      switch(mode)
      {
        case (0):
          /*
           * AT+CAOC=0 queries the actual ccm value
           */
         cl++;
         break;

       case (1):
          /*
           * AT+CAOC=1 deactivates the unsolicited report mode
           */
          ati_user_output_cfg[srcId].CAOC_stat=0;
          return (ATI_CMPL);

       case (2):
          /*
           * AT+CAOC=2 activates the unsolicited report mode
           */
          ati_user_output_cfg[srcId].CAOC_stat=1;
          return (ATI_CMPL);

       default:
          cmdCmeError (CME_ERR_OpNotAllow);
          return (ATI_FAIL);
      }
      break;
    }
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_caoc my_bat_set_plus_caoc;

  TRACE_FUNCTION("setatPLusCAOC() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_caoc, 0, sizeof(my_bat_set_plus_caoc));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CAOC;
  cmd.params.ptr_set_plus_caoc = &my_bat_set_plus_caoc;

  my_bat_set_plus_caoc.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCAOC()");

  ret = qAT_PlusCAOC((T_ACI_CMD_SRC)srcId,&ccm);
  sprintf (g_sa,"+CAOC: \"%06X\"", ccm);
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCAOC (char* cl, UBYTE srcId)
{
  UBYTE x = 0;

  TRACE_FUNCTION("queatPLusCAOC()");

  /*
   * AT+CAOC? requests the actual mode
   */
  x = ati_user_output_cfg[srcId].CAOC_stat + 1;

  resp_disp(srcId, cl,"b",&x);

  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCACM      |
+--------------------------------------------------------------------+

  PURPOSE : +CACM command (Advice of Charge, Accumulated Call Meter)
*/
GLOBAL T_ATI_RSLT setatPlusCACM (char* cl, UBYTE srcId)
{
  char pin2 [MAX_PWD_LENGTH]   = {0} ;
  T_ACI_RETURN           ret   = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl, "n", (LONG)MAX_PWD_LENGTH, &pin2);
  if ( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cacm my_bat_set_plus_cacm;

  TRACE_FUNCTION("setatPLusCACM() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cacm, 0, sizeof(my_bat_set_plus_cacm));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CACM;
  cmd.params.ptr_set_plus_cacm = &my_bat_set_plus_cacm;

  my_bat_set_plus_cacm.c_passwd = strlen(pin2);
  memcpy(my_bat_set_plus_cacm.passwd, pin2, my_bat_set_plus_cacm.c_passwd);
  src_params->curAtCmd = AT_CMD_CACM;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCACM()");

  ret = sAT_PlusCACM((T_ACI_CMD_SRC)srcId,pin2);

  switch (ret)
  {
    case AT_EXCT:
    {
      src_params->curAtCmd    = AT_CMD_CACM;
      break;
    } 
    case AT_BUSY:
    {
      cmdCmeError (CME_ERR_SimBusy);
      break;
    } 
    case AT_FAIL:
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      break;
    } 
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCACM (char* cl, UBYTE srcId)
{
  LONG         acm = 0L;
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("queatPLusCACM()");

  /*
   * +AT+CACM? requests the actual value
   */
  ret = qAT_PlusCACM((T_ACI_CMD_SRC)srcId,&acm);
  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  else
  {
    sprintf(g_sa,"+CACM: \"%06X\"", acm);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return (map_aci_2_ati_rslt (ret));
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCAMM         |
+--------------------------------------------------------------------+

  PURPOSE : +CAMM command (Advice of Charge,
            Accumulated Call Meter Maximum)
*/
GLOBAL T_ATI_RSLT setatPlusCAMM (char* cl, UBYTE srcId)
{
  char pin2       [MAX_PWD_LENGTH] = {0};
  char new_acmmax [MAX_CM_LENGTH]  = {0};
  char *pNewAcmMax = new_acmmax;
  LONG acmmax      = 0L;
  T_ACI_RETURN ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  cl = parse(cl,"sn",(LONG)MAX_CM_LENGTH,  &new_acmmax,
                     (LONG)MAX_PWD_LENGTH, &pin2);

  if ( !cl OR *new_acmmax EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  pNewAcmMax = parse(pNewAcmMax, "y", &acmmax);
  if ( !pNewAcmMax )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_camm my_bat_set_plus_camm;

  TRACE_FUNCTION("setatPLusCAMM() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_camm, 0, sizeof(my_bat_set_plus_camm));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CAMM;
  cmd.params.ptr_set_plus_camm = &my_bat_set_plus_camm;

  my_bat_set_plus_camm.acmmax = (S32)acmmax;
  my_bat_set_plus_camm.c_passwd = strlen(pin2);
  memcpy(my_bat_set_plus_camm.passwd, pin2, my_bat_set_plus_camm.c_passwd);
  src_params->curAtCmd = AT_CMD_CAMM;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCAMM()");

  ret = sAT_PlusCAMM((T_ACI_CMD_SRC)srcId, acmmax, pin2);

  switch (ret)
  {
    case AT_EXCT:
    {
      src_params->curAtCmd = AT_CMD_CAMM;
      break;
    } 
    case AT_BUSY:
    {
      cmdCmeError (CME_ERR_SimBusy);
      break;
    } 
    default:
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      break;
    } 
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCAMM (char* cl, UBYTE srcId)
{
  LONG acmmax      = 0L;
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("queatPLusCAMM()");

  /*
   * +AT+CAMM? requests the actual value
   */
  ret = qAT_PlusCAMM((T_ACI_CMD_SRC)srcId,&acmmax);
  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf (g_sa,"+CAMM: \"%06X\"", acmmax);
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPUC         |
+--------------------------------------------------------------------+

  PURPOSE : +CPUC command (Advice of Charge,
                           Price per unit and currency)
*/
GLOBAL T_ATI_RSLT setatPlusCPUC (char* cl, UBYTE srcId)
{
  char         currency     [MAX_CUR_LEN]       = {0x00};
  char         cvtdCurrency [2*MAX_CUR_LEN]     = {0x00};
  USHORT       lenCurrency                      = 0;
  USHORT       lenCvtdCurrency                  = 0;
  char         ppu          [MAX_PPU_LENGTH]    = {0x00};
  char         pin2         [MAX_PWD_LENGTH]    = {0x00};
  T_ACI_RETURN ret                              = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params                  = find_element (ati_src_list, srcId, search_ati_src_id);

  /*
   * AT+CPUC=currency, ppu, pw sets the PUCT with or without password.
   */
  cl = parse(cl,"zsn",
               (LONG)MAX_CUR_LEN,
                strlen(cl),
                cl,
                &lenCurrency,
                currency,
                (LONG)MAX_PPU_LENGTH,
                ppu,
                (LONG)MAX_PWD_LENGTH,
                pin2);

  if ( !cl OR *ppu EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  srcId_cb = srcId;
  utl_chsetToGsm ((UBYTE*)currency,
                   lenCurrency,
                   (UBYTE*)cvtdCurrency,
                   &lenCvtdCurrency,
#ifdef REL99
                   sizeof(cvtdCurrency),
#endif
                   GSM_ALPHA_Int);

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cpuc my_bat_set_plus_cpuc;

  TRACE_FUNCTION("setatPLusCPUC() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cpuc, 0, sizeof(my_bat_set_plus_cpuc));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CPUC;
  cmd.params.ptr_set_plus_cpuc = &my_bat_set_plus_cpuc;

  my_bat_set_plus_cpuc.c_currency = (U8)lenCurrency;
  memcpy(my_bat_set_plus_cpuc.currency, currency, lenCurrency);
  my_bat_set_plus_cpuc.c_ppu = strlen(ppu);
  memcpy(my_bat_set_plus_cpuc.ppu, ppu, my_bat_set_plus_cpuc.c_ppu);
  my_bat_set_plus_cpuc.c_passwd = strlen(pin2);
  my_bat_set_plus_cpuc.v_passwd = my_bat_set_plus_cpuc.c_passwd ? TRUE : FALSE;
  memcpy(my_bat_set_plus_cpuc.passwd, pin2, my_bat_set_plus_cpuc.c_passwd);
  src_params->curAtCmd = AT_CMD_CPUC;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCPUC()");
  
  ret = sAT_PlusCPUC((T_ACI_CMD_SRC)srcId,cvtdCurrency,ppu,pin2);

  switch (ret)
  {
    case AT_EXCT:
    {
      src_params->curAtCmd    = AT_CMD_CPUC;
      break;
    } 
    case AT_BUSY:
    {
      cmdCmeError (CME_ERR_SimBusy);
      break;
    } 
    default:
    {
      cmdCmeError (CME_ERR_OpNotAllow);
      break;
    } 
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCPUC (char* cl, UBYTE srcId)
{
  char         currency     [MAX_CUR_LEN]       = {0x00};
  char         cvtdCurrency [2*MAX_CUR_LEN]     = {0x00};
  USHORT       lenCvtdCurrency                  = 0;
  USHORT       pos                              = 0;
  char         ppu          [MAX_PPU_LENGTH]    = {0x00};
  T_ACI_RETURN ret                              = AT_FAIL;
  	          
               
  TRACE_FUNCTION("queatPLusCPUC()");

  /*
   * +AT+CPUC? requests the actual value
   */
  ret = qAT_PlusCPUC((T_ACI_CMD_SRC)srcId,currency, ppu);

  if( ret NEQ AT_CMPL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }


  pos = sprintf(g_sa,"+CPUC: ");

  srcId_cb = srcId;
  utl_chsetFromGsm((UBYTE*)currency,
                   (USHORT)strlen(currency),
                   (UBYTE*)cvtdCurrency,
                   sizeof(cvtdCurrency),
                   &lenCvtdCurrency,
                   GSM_ALPHA_Def);
  pos += sprints(g_sa+pos,cvtdCurrency,lenCvtdCurrency);
  pos += sprintf(g_sa+pos,",\"%s\"",ppu);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (map_aci_2_ati_rslt (ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLCC         |
+--------------------------------------------------------------------+

  PURPOSE : +CLCCcommand (list current calls)
*/
GLOBAL T_ATI_RSLT setatPlusCLCC(char *cl, UBYTE srcId)
{
#ifdef  FF_ATI_BAT

  T_BAT_cmd_send     cmd;
  T_BAT_no_parameter dummy;
  cmd.ctrl_params     = BAT_CMD_QUE_PLUS_CLCC;
  dummy.bat_dummy     = 0xFF;
  cmd.params.ptr_que_plus_clcc = &dummy;

  TRACE_FUNCTION("setatPlusCLCC() calls bat_send() <=== as APPLICATION");

  bat_send(ati_bat_get_client(srcId), &cmd);

  return (ATI_EXCT); /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */

  char               *me           = "+CLCC: ";
  T_ACI_CLCC_CALDESC *calLst       = NULL;
  UBYTE               lstIdx       = 0;
  SHORT               pos          = 0;
  USHORT              lenCvtdAlpha = 0;
  UBYTE               type         = 0;

  CHAR                cvtdAlpha[2*MAX_ALPHA_LEN];

  TRACE_FUNCTION("setatPlusCLCC()");

  srcId_cb = srcId; /* For utl_chsetFromGsm() */
  switch (*cl)
  {
    case(';'):
    case(0x0):
      {
        /* Check if we have enough RAM for the following ACI_MALLOC */
        USHORT free, alloc;
        int ret;
        ret = vsi_m_status ( hCommACI,
                             MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC),
                             PRIM_POOL_PARTITION,
                             &free,
                             &alloc );
        if (ret EQ VSI_ERROR || free EQ 0)
        {
          cmdCmeError(CME_ERR_MemFull);
          return (ATI_FAIL);
        }
      }
      ACI_MALLOC (calLst, MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC));
      (void)qAT_PlusCLCC((T_ACI_CMD_SRC)srcId, calLst); /* Always returns AT_CMPL */

      /* 20 is approximatly the amount of extra characters (like ",") */

      for( lstIdx = 0; lstIdx < MAX_CALL_NR; lstIdx++ )
      {
        if( calLst[lstIdx].idx EQ ACI_NumParmNotPresent )
          break;

        pos = sprintf(g_sa,"%s%d",me,calLst[lstIdx].idx);

        if (calLst[lstIdx].dir NEQ CLCC_DIR_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].dir);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].stat NEQ CLCC_STAT_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].stat);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].mode NEQ CLCC_MODE_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].mode);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].mpty NEQ CLCC_MPTY_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].mpty);
        else
          pos += sprintf(g_sa+pos,",");

        if (calLst[lstIdx].number[0] NEQ 0x0)
        {
          pos += sprintf(g_sa+pos,",\"%s\"",calLst[lstIdx].number);
          if (calLst[lstIdx].type.ton NEQ TON_NotPresent)
          {
            type = toa_merge(calLst[lstIdx].type);
            pos += sprintf(g_sa+pos,",%d",(int)type);
          }
#ifdef NO_ASCIIZ
          if (calLst[lstIdx].alpha.len NEQ 0x0)
          {
            pos += sprintf(g_sa+pos,",");
            utl_chsetFromGsm ( calLst[lstIdx].alpha.data,
                               calLst[lstIdx].alpha.len,
                               (UBYTE*)cvtdAlpha,
                               sizeof(cvtdAlpha),
                               &lenCvtdAlpha,
                               GSM_ALPHA_Def );
            pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
          }
#else  /* #ifdef NO_ASCIIZ */
          if (calLst[lstIdx].alpha[0] NEQ 0x0)
          {
            pos += sprintf(g_sa+pos,",");
            utl_chsetFromGsm ( (UBYTE*)calLst[lstIdx].alpha,
                               0,
                               (UBYTE*)cvtdAlpha,
                               sizeof(cvtdAlpha),
                               &lenCvtdAlpha,
                               GSM_ALPHA_Int );
            pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
          }
#endif /* #ifdef NO_ASCIIZ */
        }

        ci_remTrailCom(g_sa, pos);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
      ACI_MFREE (calLst);
      break;
    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }
  return (ATI_CMPL);

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD                                  |
| STATE   : code                        ROUTINE : atPercentCLCC                                    |
+--------------------------------------------------------------------+

  PURPOSE : %CLCCcommand (list current calls)
*/
GLOBAL T_ATI_RSLT setatPercentCLCC(char *cl, UBYTE srcId)
{
  char               *me           = "%CLCC: ";
  T_ACI_CLCC_CALDESC *calLst       = NULL;
  UBYTE               lstIdx       = 0;
  SHORT               pos          = 0;
  USHORT              lenCvtdAlpha = 0;
  UBYTE               type         = 0;
  CHAR                cvtdAlpha[2*MAX_ALPHA_LEN];

  TRACE_FUNCTION("setatPercentCLCC()");

  srcId_cb = srcId;  /* For utl_chsetFromGsm() */

  switch (*cl)
  {
    case(';'):
    case(0x0):
      ACI_MALLOC (calLst, MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC));
      memset(calLst, 0 , MAX_CALL_NR * sizeof (T_ACI_CLCC_CALDESC));
      (void)qAT_PlusCLCC((T_ACI_CMD_SRC)srcId, calLst); /* Always returns AT_CMPL */
     
      /* 20 is approximatly the amount of extra characters (like ",") */

      for( lstIdx = 0; lstIdx < MAX_CALL_NR; lstIdx++ )
      {
        if( calLst[lstIdx].idx EQ ACI_NumParmNotPresent )
          break;

        pos = sprintf(g_sa,"%s%d",me,calLst[lstIdx].idx);

        if (calLst[lstIdx].dir NEQ CLCC_DIR_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].dir);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].stat NEQ CLCC_STAT_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].stat);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].mode NEQ CLCC_MODE_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].mode);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].mpty NEQ CLCC_MPTY_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].mpty);
        else
          pos += sprintf(g_sa+pos,",");
        if (calLst[lstIdx].class_type NEQ CLCC_CLASS_NotPresent)
          pos += sprintf(g_sa+pos,",%d",calLst[lstIdx].class_type);
        else
          pos += sprintf(g_sa+pos,",");

        if (calLst[lstIdx].number[0] NEQ 0x0)
        {
          pos += sprintf(g_sa+pos,",\"%s\"",calLst[lstIdx].number);
          if (calLst[lstIdx].type.ton NEQ TON_NotPresent)
          {
            type = toa_merge(calLst[lstIdx].type);
            pos += sprintf(g_sa+pos,",%d",(int)type);
          }
#ifdef NO_ASCIIZ
          if (calLst[lstIdx].alpha.len NEQ 0x0)
          {
            pos += sprintf(g_sa+pos,",");
            utl_chsetFromGsm ( calLst[lstIdx].alpha.data,
                               calLst[lstIdx].alpha.len,
                               (UBYTE*)cvtdAlpha,
                               sizeof(cvtdAlpha),
                               &lenCvtdAlpha,
                               GSM_ALPHA_Def );
            pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
          }
#else  /* #ifdef NO_ASCIIZ */
          if (calLst[lstIdx].alpha[0] NEQ 0x0)
          {
            pos += sprintf(g_sa+pos,",");
            utl_chsetFromGsm ( (UBYTE*)calLst[lstIdx].alpha,
                               0,
                               (UBYTE*)cvtdAlpha,
                               sizeof(cvtdAlpha),
                               &lenCvtdAlpha,
                               GSM_ALPHA_Int );
            pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
          }
#endif /* #ifdef NO_ASCIIZ */
        }

        ci_remTrailCom(g_sa, pos);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
      ACI_MFREE (calLst);
      break;
    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCCWA         |
+--------------------------------------------------------------------+

  PURPOSE : +CCWA command (Call Waiting Managment)
*/
GLOBAL T_ATI_RSLT setatPlusCCWA (char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret        = AT_FAIL;
  T_ACI_CCWA_MOD mode       = CCWA_MOD_NotInterrogate;
  T_ACI_CLASS    class_type = CLASS_NotPresent;
  SHORT          val        = -1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  
  cl = parse(cl, "rdd", &val, &mode, &class_type);
  if (!cl OR val > 1 )
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if (val >= 0)
  {
    at.flags.CCWA_stat=(UBYTE) val;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ccwa my_bat_set_plus_ccwa;

  TRACE_FUNCTION("setatPLusCCWA() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ccwa, 0, sizeof(my_bat_set_plus_ccwa));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CCWA;
  cmd.params.ptr_set_plus_ccwa = &my_bat_set_plus_ccwa;

  my_bat_set_plus_ccwa.mode = mode;
  my_bat_set_plus_ccwa.bearer_class = class_type;
  src_params->curAtCmd = AT_CMD_CCWA;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCCWA()");

  if (mode EQ CCWA_MOD_Query)  /*query mode*/
  {
    ret = qAT_PlusCCWA ((T_ACI_CMD_SRC)srcId,class_type);
    switch (ret)
    {
    case AT_EXCT:
      src_params->curAtCmd = AT_CMD_CCWA;
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);
      break;
    }
  }
  else  /*set mode*/
  {
    ret = sAT_PlusCCWA ((T_ACI_CMD_SRC)srcId,mode,class_type);
    switch (ret)
    {
    case AT_EXCT:
      break;

    case AT_FAIL:
    case AT_BUSY:
      cmdCmeError (CME_ERR_Unknown);
      break;
    }
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


GLOBAL T_ATI_RSLT queatPlusCCWA (CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPLusCCWA()");

  resp_disp (srcId, cl,"b",&at.flags.CCWA_stat);
  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusVTS          |
+--------------------------------------------------------------------+

  PURPOSE : +VTS command (send DTMF)
*/
GLOBAL T_ATI_RSLT setatPlusVTS (char * cl, UBYTE srcId)
{
  T_ACI_RETURN      ret        = AT_FAIL;
  SHORT             duration   = ACI_NumParmNotPresent;
  CHAR              dtmf       = '\0';
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  if( *cl EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  cl = parse (cl, "ar", (ULONG)1, &dtmf, &duration);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_vts my_bat_set_plus_vts;

  TRACE_FUNCTION("setatPlusVTS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_vts, 0, sizeof(my_bat_set_plus_vts));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_VTS;
  cmd.params.ptr_set_plus_vts = &my_bat_set_plus_vts;

  my_bat_set_plus_vts.dtmf = dtmf;
  src_params->curAtCmd = AT_CMD_VTS;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusVTS()");

  ret = sAT_PlusVTS((T_ACI_CMD_SRC)srcId,dtmf,VTS_MOD_Auto);

  switch (ret)
  {
  case AT_EXCT:
    src_params->curAtCmd = AT_CMD_VTS;
    break;

  case AT_FAIL:
  case AT_BUSY:
    cmdCmeError (CME_ERR_Unknown);
    break;
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentRDL    | 
+--------------------------------------------------------------------+

  PURPOSE : %RDL command (set redial mode)
*/

GLOBAL T_ATI_RSLT setatPercentRDL (char* cl, UBYTE srcId)
{
  T_ACI_CC_REDIAL_MODE rdlmode = AUTOM_REP_NOT_PRESENT;
  T_ACI_CC_REDIAL_NOTIF usr_notif = NOTIF_NO_PRESENT;
  T_ACI_RETURN   ret  = AT_FAIL;
  
  cl = parse(cl, "dd", &rdlmode, &usr_notif);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_rdl my_bat_set_percent_rdl;

  TRACE_FUNCTION("setatPercentRDL() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_rdl, 0, sizeof(my_bat_set_percent_rdl));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_RDL;
  cmd.params.ptr_set_percent_rdl = &my_bat_set_percent_rdl;

  my_bat_set_percent_rdl.mode = (T_BAT_percent_rdl_mode)&rdlmode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentRDL()");

  ret = sAT_PercentRDL ((T_ACI_CMD_SRC)srcId, rdlmode, usr_notif);

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentRDL    | 
+--------------------------------------------------------------------+

  PURPOSE : %RDL command (query redial mode)
*/

GLOBAL T_ATI_RSLT queatPercentRDL (char* cl, UBYTE srcId)
{
  T_ACI_CC_REDIAL_MODE rdlmode = AUTOM_REP_NOT_PRESENT;
  T_ACI_CC_REDIAL_NOTIF usr_notif = NOTIF_NO_PRESENT;
  T_ACI_RETURN   ret  = AT_FAIL;

  TRACE_FUNCTION("queatPercentRDL()");

  ret = qAT_PercentRDL ((T_ACI_CMD_SRC)srcId,&rdlmode, &usr_notif);

  if(ret EQ AT_CMPL)
  {
    resp_disp (srcId, cl,"bb",&rdlmode, &usr_notif);
  }
  else
  {
    cmdCmeError (CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
}

#ifdef TI_PS_FF_AT_P_CMD_RDLB
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : setatPercentRDLB     | 
+--------------------------------------------------------------------+

  PURPOSE : %RDLB command (delete black list of redialling phone
            numbers)
*/

GLOBAL T_ATI_RSLT setatPercentRDLB (char* cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CC_REDIAL_NOTIF usr_notif = NOTIF_NO_PRESENT;
  T_ACI_CC_REDIAL_BLMODE blmode = BLMODE_NO_PRESENT;

  cl = parse(cl, "dd", &blmode, &usr_notif);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_rdlb my_bat_set_percent_rdlb;

  TRACE_FUNCTION("setatPercentRDLB() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_rdlb, 0, sizeof(my_bat_set_percent_rdlb));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_RDLB;
  cmd.params.ptr_set_percent_rdlb = &my_bat_set_percent_rdlb;

  my_bat_set_percent_rdlb.mode = (T_BAT_percent_rdlb_mode)&blmode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentRDLB()");

  ret = sAT_PercentRDLB((T_ACI_CMD_SRC)srcId, blmode, usr_notif);
 
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_CMD              |
| STATE   : code                      ROUTINE : queatPercentRDLB     | 
+--------------------------------------------------------------------+

  PURPOSE : %RDLB command (query black list of redialling phone
            numbers)
*/

GLOBAL T_ATI_RSLT queatPercentRDLB (char* cl, UBYTE srcId)
{
  T_ACI_RETURN   ret  = AT_FAIL;
  T_ACI_CC_REDIAL_BLACKL blackl;
  T_ACI_CC_REDIAL_NOTIF usr_notif= NOTIF_NO_PRESENT;
  UBYTE         i,j;
  UBYTE         type;

  TRACE_FUNCTION("queatPercentRDLB()");

  memset(&blackl, 0, sizeof(T_ACI_CC_REDIAL_BLACKL));

  ret = qAT_PercentRDLB ((T_ACI_CMD_SRC)srcId,&blackl,&usr_notif);

  if(ret EQ AT_CMPL)
  {
    resp_disp (srcId, cl,"b", &usr_notif); /* output user notification state */
    for(i=0; i<blackl.blCount; i++) /* output black list */
    {
        type = (UBYTE)toa_merge(blackl.blNum[i].type);
	for(j=0; j<blackl.blNum[i].numb_len; j++)
        {
          blackl.blNum[i].number[j] = (blackl.blNum[i].number[j] | 0x30);
        }
        sprintf(g_sa,"%s:\"%s\",%d","%RDLB",blackl.blNum[i].number,
                                         (int)type);
	io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
  }
  else
  {
    cmdCmeError (CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentVTS       |
+--------------------------------------------------------------------+

  PURPOSE : %VTS command (send DTMF)
*/
GLOBAL T_ATI_RSLT setatPercentVTS (char * cl, UBYTE srcId)
{
  T_ACI_RETURN      ret         = AT_FAIL;
  CHAR              dtmf        = 0x0;
  T_ATI_SRC_PARAMS  *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_ACI_VTS_MOD     mode        = VTS_MOD_Auto;

  if( *cl EQ '\0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  cl = parse (cl, "ar", (ULONG)1, &dtmf, &mode);

  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if (mode EQ VTS_MOD_NotPresent)
  {
    mode = VTS_MOD_Auto;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_vts my_bat_set_percent_vts;

  TRACE_FUNCTION("setatPercentVTS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_vts, 0, sizeof(my_bat_set_percent_vts));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_VTS;
  cmd.params.ptr_set_percent_vts = &my_bat_set_percent_vts;

  my_bat_set_percent_vts.dtmf = dtmf;
  my_bat_set_percent_vts.mode = mode;
  src_params->curAtCmd = AT_CMD_VTS;
  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentVTS()");

  ret = sAT_PlusVTS((T_ACI_CMD_SRC)srcId,dtmf,mode);

  switch (ret)
  {
    case AT_EXCT:
    {
      src_params->curAtCmd    = AT_CMD_VTS;
      break;
    }
    case AT_FAIL:
    case AT_BUSY:
    {
      cmdCmeError (CME_ERR_Unknown);
      break;
    }
  }

  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSNS         |
+--------------------------------------------------------------------+

  PURPOSE : +CSNS command (single numbering scheme)
*/
GLOBAL T_ATI_RSLT setatPlusCSNS (char* cl, UBYTE srcId)
{
  T_ACI_CSNS_MOD mode = CSNS_MOD_NotPresent;
  T_ACI_RETURN   ret  = AT_FAIL;

  cl= parse (cl, "d", &mode);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_csns my_bat_set_plus_csns;

  TRACE_FUNCTION("setatPlusCSNS() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_csns, 0, sizeof(my_bat_set_plus_csns));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CSNS;
  cmd.params.ptr_set_plus_csns = &my_bat_set_plus_csns;

  my_bat_set_plus_csns.mode = mode;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusCSNS()");

  ret = sAT_PlusCSNS ((T_ACI_CMD_SRC)srcId, mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
  }
  return (map_aci_2_ati_rslt (ret));

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCSNS (char* cl, UBYTE srcId)
{
  T_ACI_CSNS_MOD mode = CSNS_MOD_NotPresent;
  T_ACI_RETURN   ret  = AT_FAIL;

  TRACE_FUNCTION("queatPlusCSNS()");

  ret = qAT_PlusCSNS ((T_ACI_CMD_SRC)srcId,&mode);

  if(ret EQ AT_CMPL)
  {
    resp_disp (srcId, cl,"e",&mode);
  }
  else
  {
    cmdCmeError (CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
}
#ifdef TI_PS_FF_AT_CMD_P_ECC
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentECC    |
+--------------------------------------------------------------------+

  PURPOSE : %ECC command (set additional ECC numbers)
*/

GLOBAL T_ATI_RSLT setatPercentECC (char* cl, UBYTE srcId)
{
  T_ACI_RETURN            ret = AT_FAIL;
  U8                      index;
  char                    ecc_number[ADDITIONAL_ECC_NUMBER_LENGTH+1]; /* +1 for '\0' */
  UBYTE                   idx;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPercentECC()");

  memset(ecc_number,0,sizeof(ecc_number));

  cl = parse(cl,"ds",
          &index,
          (LONG)sizeof(ecc_number),
          ecc_number);

  if (cl EQ NULL)
  {
    idx = get_parse_index();         /* get the problematic component */
    TRACE_EVENT_P1("index %d",idx);
    switch (idx)
    {
      case 1:
        cmdCmeError(CME_ERR_InvIdx);
        break;
      case 2:
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
    T_BAT_cmd_set_percent_ecc ecc;
    
    memset(&ecc, 0, sizeof(ecc));
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_ECC;
    cmd.params.ptr_set_percent_ecc = &ecc;
    ecc.index = index;
    ecc.c_number = (U8)strlen(ecc_number);
    memcpy(ecc.number, ecc_number, ecc.c_number);
    src_params->curAtCmd = AT_CMD_P_ECC;
    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT;
  }
#else
  ret = sAT_PercentECC((T_ACI_CMD_SRC)srcId, index, ecc_number);

  return (map_aci_2_ati_rslt(ret));
#endif
}
#endif /* TI_PS_FF_AT_CMD_P_ECC */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCVHU      | 
+--------------------------------------------------------------------+

  PURPOSE : +CVHU command (set voice hangup control)
*/
GLOBAL T_ATI_RSLT setatPlusCVHU (char* cl, UBYTE srcId)
{
  T_ACI_CVHU_MODE mode = CVHU_DropDTR_IGNORED;
  T_ACI_RETURN ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCVHU()");

  cl= parse (cl,"d",&mode);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cvhu cvhu;
    
    memset(&cvhu, 0, sizeof(cvhu));
    cmd.ctrl_params = BAT_CMD_SET_PLUS_CVHU;
    cmd.params.ptr_set_plus_cvhu = &cvhu;
    cvhu.cvhu_mode = mode;
    src_params->curAtCmd = AT_CMD_CVHU;
    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT;
  }
#else
  ret= sAT_PlusCVHU ((T_ACI_CMD_SRC)srcId, mode);
  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
  }
  return (map_aci_2_ati_rslt (ret));
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCVHU      | 
+--------------------------------------------------------------------+

  PURPOSE : +CVHU command (query voice hangup control)
*/
GLOBAL T_ATI_RSLT queatPlusCVHU (char* cl, UBYTE srcId)
{
  T_ACI_CVHU_MODE mode = CVHU_DropDTR_IGNORED;
  T_ACI_RETURN    ret = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPlusCVHU()");

#ifdef  FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPlusCVHU() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CVHU;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cvhu = &dummy;
    src_params->curAtCmd = AT_CMD_CVHU;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  ret = qAT_PlusCVHU ((T_ACI_CMD_SRC)srcId, &mode);

  if(ret EQ AT_CMPL)
  {
    resp_disp (srcId, cl, "e", &mode);
  }
  else
  {
    cmdCmeError (CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt (ret));
#endif  /* FF_ATI_BAT */
}
#endif /* ATI_CC_C */


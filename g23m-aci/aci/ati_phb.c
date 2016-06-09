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
|  Purpose :  AT commands related to phonebook.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_PHB_C
#define ATI_PHB_C

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
#include "phb.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci_mem.h"
#include "psa.h"
#include "cmh.h"
#include "pcm.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /* FF_ATI_BAT */

/* Global variables for reading PHB entries */
LOCAL SHORT  phb_start_idx, phb_stop_idx;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPBS         |
+--------------------------------------------------------------------+

  PURPOSE : +CPBS (Select Phonebook memory storage)
*/

GLOBAL T_ATI_RSLT setatPlusCPBS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_PB_STOR   stor = PB_STOR_NotPresent; 
  CHAR            pin2[MAX_PWD_LENGTH] = {0};
  CHAR            sto_str[3]={0};
  SHORT           i;

  TRACE_FUNCTION("setatPlusCPBS()");

  cl = parse(cl,"sn",(LONG)3,sto_str,(LONG)MAX_PWD_LENGTH,pin2);
  if(!cl OR *sto_str EQ '\0')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  strupper(sto_str);
  for(i=0;phb_mem_names[i].name NEQ 0;i++)
  {
    if (!strcmp(phb_mem_names[i].name,sto_str) )
    {
      stor=phb_mem_names[i].stor;
      break;
    }
  }
  if(phb_mem_names[i].name EQ 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpbs my_bat_set_plus_cpbs = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
    
    TRACE_FUNCTION("setatPlusCPBS() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBS;
    cmd.params.ptr_set_plus_cpbs = &my_bat_set_plus_cpbs;

    my_bat_set_plus_cpbs.storage = (T_BAT_storage)stor;

    src_params->curAtCmd = AT_CMD_CPBS;
    srcId_cb = srcId;
    bat_send(ati_bat_get_client(srcId), &cmd);
    srcId_cb = srcId;
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  ret = sAT_PlusCPBS((T_ACI_CMD_SRC)srcId,stor,pin2);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }

  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}


GLOBAL T_ATI_RSLT queatPlusCPBS(char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPlusCPBS() calls bat_send() <=== as APPLICATION");
    
    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CPBS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cpbs = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */

  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_PB_STOR   stor;
  SHORT           used;
  SHORT           total;
  SHORT           i;

  TRACE_FUNCTION("queatPlusCPBS()");

  ret = qAT_PlusCPBS((T_ACI_CMD_SRC)srcId,&stor,&used,&total);
  if (ret EQ AT_CMPL)
  {
    for(i=0;phb_mem_names[i].name NEQ NULL;i++)
    {
      if (phb_mem_names[i].stor EQ stor)
      {
        sprintf(g_sa,"+CPBS: \"%s\",%d,%d",phb_mem_names[i].name,used,total);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        break;
      }
    }
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPercentCPBS   |
+--------------------------------------------------------------------+

  PURPOSE : %CPBS (Phonebook memory storage)
*/
GLOBAL T_ATI_RSLT queatPercentCPBS(char *cl, UBYTE srcId)
{
#ifdef  FF_ATI_BAT

  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPercentCPBS() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params=BAT_CMD_QUE_PERCENT_CPBS;
  dummy.bat_dummy=0xFF;
  cmd.params.ptr_que_percent_cpbs=&dummy;
 
  bat_send(ati_bat_get_client(srcId),&cmd);

  /*
  *   Return value indicates executing, the response will come via
  *   the callback function.
  */
  return(ATI_EXCT);

#else /* FF_ATI_BAT */

  char *me="%CPBS: ";
  T_ACI_RETURN    ret;
  T_ACI_PB_STOR   stor;
  SHORT used,total,first,i;
  SHORT used_ext, total_ext;

  TRACE_FUNCTION("queatPercentCPBS()");

  /*
   *   Call the corresponding ACI function.
   */
  ret=qAT_PercentCPBS ((T_ACI_CMD_SRC)srcId, 
                       &stor,
                       &used, &total,
                       &first,
                       &used_ext, &total_ext);

  /*
  *   If the query completes successfully, build and send the
  *   answer.
  */
  if (ret EQ AT_CMPL)
  {
    /*
    *   Run through the list of names to get the name of the
    *   currently selected phonebook.
    */
    for(i=0;phb_mem_names[i].name NEQ NULL;i++)
    {
      if (phb_mem_names[i].stor EQ stor)
      {
        sprintf (g_sa,"%s\"%s\",%d,%d,%d,%d,%d", me, phb_mem_names[i].name,
                 used, total,
                 first,
                 used_ext, total_ext);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        break;
      }
    }
  }

  return (map_aci_2_ati_rslt(ret));

#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPBW         |
+--------------------------------------------------------------------+

  PURPOSE : +CPBW (Write Phonebook entry)
*/
GLOBAL T_ATI_RSLT setatPlusCPBW(char *cl, UBYTE srcId)
{

  UBYTE          pb_status;
  T_ACI_RETURN    ret = AT_FAIL;
  CHAR           *p_number                = NULL;
  CHAR            text    [MAX_ALPHA_LEN * 4] = {0x00};
  USHORT          lenText                 = 0;
  T_ACI_TOA      *p_toa                   = NULL,
                  type;

  SHORT           toa_val                 = 0,
                  index                   = -1;
#ifndef  FF_ATI_BAT
  T_ACI_PB_TEXT  *p_text                  = NULL;
  T_ACI_PB_TEXT   cvtdText;
  USHORT          lenCvtd                 = 0;
  T_ACI_VP_ABS    dmyDateTime;
#endif /* !FF_ATI_BAT */

  T_ACI_PB_STOR   stor;
  SHORT           used;
  SHORT           total;
  CHAR            numBuf[MAX_PHB_NUM_LEN];
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  memset( numBuf, 0, sizeof(numBuf));

  TRACE_FUNCTION("setatPlusCPBW()");


  ret = qAT_PlusCPBS((T_ACI_CMD_SRC)srcId,&stor,&used,&total);
  TRACE_EVENT_P1("Current phonebook storage: %d", stor);

#ifdef TI_PS_FFS_PHB
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
#endif
  
  if (ret EQ AT_CMPL    AND 
      stor EQ PB_STOR_Ed)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

  if ( ati_user_output_cfg[srcId].cscsChset EQ CSCS_CHSET_Gsm)
  {
    cl=parse(cl,"rsrz",
              &index,
              (LONG)MAX_PHB_NUM_LEN,
              numBuf,
              &toa_val,
              (LONG)MAX_ALPHA_LEN,
              strlen(cl),
              cl,
              &lenText,
              text);
  }
  else
  {
    cl=parse(cl,"rsrz",
              &index,
              (LONG)MAX_PHB_NUM_LEN,
              numBuf,
              &toa_val,
              (LONG)MAX_ALPHA_LEN * 4,
              strlen(cl),
              cl,
              &lenText,
              text);
  }

  if(!cl)
  {
    TRACE_ERROR("ERROR: parsing failed !!!");
    index = get_parse_index();       /* get the problematic component */
    switch (index)
    {
      case 2:   /* component 2 is the number */
        cmdCmeError(CME_ERR_DialToLong);
        break;
      case 4:   /* component 4 is the alpha tag */
        cmdCmeError(CME_ERR_TxtToLong);
        break;
      default:
        cmdCmeError(CME_ERR_OpNotAllow);
        break;
    }
    return ATI_FAIL;
  }
  
   /* If the phonebook status is not PHB_READY then SIM BUSY error is indicated to the user.*/
  pb_status_req(&pb_status); /* get phone book status */

  TRACE_EVENT_P1("Current phonebook status: %d", pb_status);
  if (pb_status NEQ PHB_READY)
  {
    if(pb_status EQ PHB_BUSY)
    {
      TRACE_EVENT("Error: Phonebook is busy accessing the SIM");
      cmdCmeError(CME_ERR_SimBusy);
    }
    else if(pb_status EQ PHB_UNKNOWN)
    {
      TRACE_EVENT("Error: Phonebook status unknown");
      cmdCmeError(CME_ERR_Unknown);
    }
    return ATI_FAIL;
  }

  if( index > total) /* phonebook index is greater than max. */
  {                  /* possible index of SIM                */
    TRACE_EVENT("Error: phonebook index is greater than max possible index of SIM");
    cmdCmeError(CME_ERR_InvIdx);
    return ATI_FAIL;
  }

  if (toa_val > 0)
  {
    type=toa_demerge(toa_val);
    p_toa=&type;
  }
  else
    p_toa=NULL;

  if (numBuf[0] EQ '\0')
    p_number=NULL;
  else
    p_number=numBuf;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpbw my_bat_set_plus_cpbw = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

    TRACE_FUNCTION("setatPlusCPBW() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBW;
    cmd.params.ptr_set_plus_cpbw = &my_bat_set_plus_cpbw;

    my_bat_set_plus_cpbw.index = (S16)index;

    /* Load the phone number in BAT struct - if there is one */
    if (p_number)
    {
      if (strlen(p_number)>BAT_MAX_CPBW_NUMBER_LEN)
      {
        /*
        *   If the number is too big for the BAT structure we can't do
        *   anything meaningful, so get out now.
        */
        cmdCmeError(CME_ERR_Unknown);
        return ATI_FAIL;
      }

      my_bat_set_plus_cpbw.c_number = strlen(p_number);
      my_bat_set_plus_cpbw.v_number = TRUE;
      memcpy(my_bat_set_plus_cpbw.number, p_number,BAT_MAX_CPBW_NUMBER_LEN);
    }
    else
    {
      my_bat_set_plus_cpbw.v_number = FALSE;
    }
    
    /*
    *   Load the associated text in BAT struct - if there is any
    */
    if ((lenText EQ 0) AND (p_number EQ NULL))
    {
      my_bat_set_plus_cpbw.v_text = FALSE;
    }
    else
    {
      /*
      *   If the associated text is too big for the BAT structure it
      *   will be truncated.
      */
      if (lenText>BAT_MAX_CPBW_TEXT_LEN)
        my_bat_set_plus_cpbw.c_text = BAT_MAX_CPBW_TEXT_LEN;
      else
        my_bat_set_plus_cpbw.c_text = (U8)lenText;

      my_bat_set_plus_cpbw.v_text = TRUE;
      memcpy(my_bat_set_plus_cpbw.text, text, BAT_MAX_CPBW_TEXT_LEN);
    }

    /* Load type as string input */
    if (p_toa)
      my_bat_set_plus_cpbw.type = (S16)toa_val;
    else
      my_bat_set_plus_cpbw.type = (S16)-1;

    src_params->curAtCmd = AT_CMD_CPBW;
    srcId_cb = srcId;
    bat_send(ati_bat_get_client(srcId), &cmd);
 
    return ATI_EXCT; /* executing, because response is passed by callback function */
  }

#else /* no FF_ATI_BAT */

  if( lenText NEQ 0 )
  {
    srcId_cb = srcId;
    utl_chsetToSim ((UBYTE*)text, lenText, (UBYTE*)cvtdText.data,
                    &lenCvtd, GSM_ALPHA_Def);
  }
  cvtdText.cs = CS_Sim;

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

  ret = sAT_PlusCPBW((T_ACI_CMD_SRC)srcId, index, p_number, p_toa, p_text, &dmyDateTime);
  switch (ret)
  {
    case AT_FAIL:
      switch (ACI_ERR_DESC_CLASS( aciErrDesc ))
      {
        case ACI_ERR_CLASS_Cme:
          cmdCmeError( (T_ACI_CME_ERR)ACI_ERR_DESC_NR( aciErrDesc ) );
          break;
        case ACI_ERR_CLASS_Cms:
        case ACI_ERR_CLASS_Ceer:
        case ACI_ERR_CLASS_Ext:
        default:
          cmdCmeError(CME_ERR_Unknown);
          break;
      }
      break;
    case AT_EXCT:
      src_params->curAtCmd = AT_CMD_CPBW;
      break;

    case AT_BUSY:
      cmdCmeError(CME_ERR_SimBusy);
      break;
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT tesatPlusCPBW(char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT

  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("queatPlusCPBW() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params = BAT_CMD_TST_PLUS_CPBW;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_tst_plus_cpbw = &dummy;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */

  char           *me                      = "+CPBW: ";
  T_ACI_RETURN    ret = AT_FAIL;

  SHORT      first_idx,
                  last_idx;
  UBYTE           nlength,
                  tlength;
  CHAR            numBuf[MAX_PHB_NUM_LEN];

  memset( numBuf, 0, sizeof(numBuf));

  TRACE_FUNCTION("tesatPlusCPBW()");

  ret = tAT_PlusCPBW((T_ACI_CMD_SRC)srcId,&first_idx,&last_idx,&nlength,&tlength);
  if(ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s(%d-%d),%d,%s,%d",me,first_idx,last_idx,nlength,"(128-201)",tlength);
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
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_CMD                      |
| STATE   : code             ROUTINE : cmd_readCPBRBlock            |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBR
            AT command which is responsible for reading phonebook
            entries from memory, 5 at a time.

  */
GLOBAL T_ACI_RETURN cmd_readCPBRBlock ( UBYTE  srcId)
{
  CHAR         cvtdText[4*MAX_ALPHA_LEN]  = {0x00};  /* need enough space for UCS2 strings */
  USHORT       lenCvtdText                = 0;
  USHORT       pos                        = 0;
  SHORT        last_idx                   = ACI_NumParmNotPresent;
  SHORT        i;
  T_ACI_PB_LST pblst;
  UBYTE        type;
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION ("cmd_readCPBRBlock ()");

  ret = sAT_PlusCPBR((T_ACI_CMD_SRC) srcId,phb_start_idx,phb_stop_idx,&last_idx,pblst);
  
    if (ret EQ AT_CMPL)
    {
      for ( i = 0;
            i < MAX_PB_ENTR;
            i++ )
      {
      if(pblst[i].index EQ ACI_NumParmNotPresent)
      {
        break; /* All entries are read */
      }

        type = toa_merge(pblst[i].type);

        pos=sprintf(g_sa,"+CPBR: %d,\"%s\",%d,",
                                         pblst[i].index,
                                         pblst[i].number,
                                         type);
         
        srcId_cb = srcId;
        utl_chsetFromSim((UBYTE*)pblst[i].text.data,
                         pblst[i].text.len,
                         (UBYTE*)cvtdText,
                         sizeof(cvtdText),
                         &lenCvtdText,
                         GSM_ALPHA_Def);

        pos+=sprints(g_sa+pos,cvtdText,lenCvtdText);
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }

    phb_start_idx = last_idx;
    
    /* To read next entry */
    phb_start_idx++;

    if(phb_start_idx EQ (phb_stop_idx + 1))
    {
      return AT_CMPL;
    }
    else
    {
      /* Still entries to read */
      return AT_EXCT;
    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return AT_FAIL;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : ACI_CMD                      |
| STATE   : code             ROUTINE : cmd_PlusCPBR_CB              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBR
            AT command which is responsible for reading phonebook
            entries from memory.

            This is a callback function for reading remaining 
            phonebook entries called from sap_dti.c.
            This function reads 5 entries from where the last 
            reading had stopped.
*/
GLOBAL void cmd_PlusCPBR_CB ( UBYTE  srcId)
{
  T_ACI_RETURN ret;

  TRACE_FUNCTION ("cmd_PlusCPBR_CB()");

  /* Call cmd_readCPBRBlock() 
     when still entries are to be read */
  if(phb_stop_idx >= phb_start_idx)
  {
    ret = cmd_readCPBRBlock(srcId);

    switch(ret)
    {
      case AT_EXCT:
        /* Still entries to read */
        return;
      
      case AT_CMPL:
        /* All the entries are read */
        R_AT( RAT_OK, (T_ACI_CMD_SRC) srcId )
          (AT_CMD_CPBR);
        break;

      default:
        /* Failed to read entries */
        R_AT( RAT_CME, (T_ACI_CMD_SRC) srcId )
          ( AT_CMD_CPBR, CME_ERR_Unknown);
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPBR         |
+--------------------------------------------------------------------+

  PURPOSE : +CPBR (Read Phonebook Entry)
*/

GLOBAL T_ATI_RSLT setatPlusCPBR(char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret;

  TRACE_FUNCTION("setatPlusCPBR()");

  phb_start_idx = ACI_NumParmNotPresent;
  phb_stop_idx  = ACI_NumParmNotPresent;

  cl = parse(cl,"rr",&phb_start_idx,&phb_stop_idx);
  if(!cl OR phb_start_idx > MAX_PB_INDEX OR phb_start_idx < 0 OR phb_stop_idx > MAX_PB_INDEX)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  if(phb_stop_idx EQ ACI_NumParmNotPresent)
  {
    phb_stop_idx=phb_start_idx;
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpbr my_bat_set_plus_cpbr = {0};
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
    
    TRACE_FUNCTION("setatPlusCPBR() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBR;
    cmd.params.ptr_set_plus_cpbr = &my_bat_set_plus_cpbr;

    my_bat_set_plus_cpbr.index1 = (U8) phb_start_idx;
    my_bat_set_plus_cpbr.index2 = (S16) phb_stop_idx;

    src_params->curAtCmd = AT_CMD_CPBR;
    srcId_cb = srcId;
    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* no FF_ATI_BAT */
  
  /* Read 5 entries at a time */
  ret = cmd_readCPBRBlock(srcId);
  
#endif /* no FF_ATI_BAT */

  return (map_aci_2_ati_rslt(ret));
}

GLOBAL T_ATI_RSLT tesatPlusCPBR(char *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT

  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  TRACE_FUNCTION("tesatPlusCPBR() calls bat_send() <=== as APPLICATION");

  cmd.ctrl_params = BAT_CMD_TST_PLUS_CPBR;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_tst_plus_cpbr = &dummy;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */

#else /* no FF_ATI_BAT */

  T_ACI_RETURN ret = AT_FAIL;
  SHORT        first_idx,
               last_idx                   = ACI_NumParmNotPresent;
  UBYTE        nlength,
               tlength;

  TRACE_FUNCTION("tesatPlusCPBR()");

  ret = tAT_PlusCPBR((T_ACI_CMD_SRC)srcId,&first_idx,&last_idx,&nlength,&tlength);
  if(ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+CPBR: (%d-%d),%d,%d",first_idx,last_idx,nlength,tlength);
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
| STATE   : code                        ROUTINE : atPlusCPBF         |
+--------------------------------------------------------------------+

  PURPOSE : +CPBF (Find Text in Phonebook)
*/

GLOBAL T_ATI_RSLT setatPlusCPBF(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret = AT_FAIL;
#ifndef NO_ASCIIZ
  CHAR         cvtdFindstr[MAX_ALPHA_LEN]    = {0x00};
#endif
  USHORT       lenCvtdFindstr                = 0;
  CHAR         cvtdText   [2*MAX_ALPHA_LEN]  = {0x00};
  USHORT       lenCvtdText                   = 0;
  USHORT       pos                           = 0;

  SHORT   i,
          j;
  SHORT        found;
  T_ACI_PB_LST pblst;
  UBYTE        type;

#ifdef NO_ASCIIZ
  T_ACI_PB_TEXT pbText;
#endif /* #ifdef NO_ASCIIZ */
 
#endif /* FF_ATI_BAT */

  CHAR         findstr    [MAX_ALPHA_LEN]    = {0x00};
  USHORT       lenFindstr                    = 0;

  TRACE_FUNCTION("setatPlusCPBF()");

  cl=parse(cl,"z",
              (LONG)MAX_ALPHA_LEN,
              strlen(cl),
              cl,
              &lenFindstr,
              findstr);
  if(!cl OR lenFindstr EQ 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  srcId_cb = srcId;
  
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cpbf cpbf;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CPBF;
    cmd.params.ptr_set_plus_cpbf=&cpbf;

    /*
    *   Check that the string isn't too long for the BAT message.
    */
    if (lenFindstr>BAT_MAX_CPBF_FIND_TEXT_LEN)
    {
      cmdCmeError(CME_ERR_Unknown);
      return ATI_FAIL;
    }

    cpbf.c_findtext=(U8)lenFindstr;
    memcpy(cpbf.findtext,findstr,lenFindstr);

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else /* no FF_ATI_BAT */

#ifdef NO_ASCIIZ
  utl_chsetToSim ( (UBYTE*)findstr,
                   lenFindstr,
                   pbText.data,
                   &lenCvtdFindstr,
                   GSM_ALPHA_Def );

  pbText.cs = CS_Sim;
  pbText.len = (UBYTE)lenCvtdFindstr;

  ret = sAT_PlusCPBF( (T_ACI_CMD_SRC)srcId, &pbText, CPBF_MOD_NewSearch,
                      &found, &pblst[0] );
#else  /* #ifdef NO_ASCIIZ */
  utl_chsetToGsm ( (UBYTE*)findstr,
                   lenFindstr,
                   (UBYTE*)cvtdFindstr,
                   &lenCvtdFindstr,
#ifdef REL99
                   sizeof(cvtdFindstr),
#endif  /* REL99 */
                   GSM_ALPHA_Int );

  ret = sAT_PlusCPBF( srcId, cvtdFindstr, CPBF_MOD_NewSearch,
                      &found, &pblst[0] );
#endif /* #ifdef NO_ASCIIZ */
  if (ret EQ AT_CMPL)
  {
    i=0;
    while(i<MAX_PB_ENTR AND i < found)
    {
      type = toa_merge(pblst[i].type);

      pos=sprintf(g_sa,"+CPBF: %d,\"%s\",%d,",
                                         pblst[i].index,
                                         pblst[i].number,
                                         type);
      utl_chsetFromSim((UBYTE*)pblst[i].text.data,
                       pblst[i].text.len,
                       (UBYTE*)cvtdText,
                       sizeof(cvtdText),
                       &lenCvtdText,
                       GSM_ALPHA_Def);
      pos+=sprints(g_sa+pos,cvtdText,lenCvtdText);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      i++;
    }
    if (found > MAX_PB_ENTR)
    {
      while(i < found)
      {
#ifdef NO_ASCIIZ
        ret = sAT_PlusCPBF( (T_ACI_CMD_SRC)srcId, &pbText,
                            CPBF_MOD_NextSearch, &found, &pblst[0] );
#else  /* #ifdef NO_ASCIIZ */
        ret = sAT_PlusCPBF( srcId, cvtdFindstr,
                            CPBF_MOD_NextSearch, &found, &pblst[0] );
#endif /* #ifdef NO_ASCIIZ */
        if (ret EQ AT_CMPL)
        {
          for(j=0;j < MAX_PB_ENTR AND i < found;j++)
          {
            type = toa_merge(pblst[j].type);

            pos=sprintf(g_sa,"+CPBF: %d,\"%s\",%d,",
                                             pblst[j].index,
                                             pblst[j].number,
                                             type);
            utl_chsetFromGsm((UBYTE*)pblst[j].text.data,
                             pblst[j].text.len,
                             (UBYTE*)cvtdText,
                             sizeof(cvtdText),
                             &lenCvtdText,
                             GSM_ALPHA_Def);
            pos+=sprints(g_sa+pos,cvtdText,lenCvtdText);
            io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
            i++;
          }
        }
        else
        {
          i = found; /* Ensure at least termination of loop */
        }
      }
    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));
  
#endif /* no FF_ATI_BAT */  
}

GLOBAL T_ATI_RSLT tesatPlusCPBF(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret = AT_FAIL;
  UBYTE        nlength,
               tlength;
#endif

  TRACE_FUNCTION("tesatPlusCPBF()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_TST_PLUS_CPBF;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_tst_plus_cpbf = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = tAT_PlusCPBF((T_ACI_CMD_SRC)srcId,&nlength,&tlength);
  if(ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl,"bb",&nlength,&tlength);
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
| STATE   : code                        ROUTINE : atPlusCSVM         |
+--------------------------------------------------------------------+

  PURPOSE : +CSVM command (Set Voice Mail Number )
*/

GLOBAL T_ATI_RSLT setatPlusCSVM (char *cl, UBYTE srcId)
{

  T_ACI_CSVM_MOD mode = CSVM_MOD_NotPresent;
  T_ACI_RETURN   ret = AT_FAIL;
  T_ACI_TOA      type;
  SHORT          toa_val=0;
  CHAR           numBuf[MAX_CC_ORIG_NUM_LEN];

  TRACE_FUNCTION("setatPlusCSVM()");

  memset( numBuf, 0, sizeof(numBuf));

  cl=parse(cl,"dsr",&mode,(LONG)MAX_CC_ORIG_NUM_LEN,
           numBuf,&toa_val);

  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  if ( mode EQ CSVM_MOD_Enable AND
       numBuf[0] EQ '\0' OR
       toa_val < 0)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  if(toa_val NEQ 0)
  {
    type=toa_demerge(toa_val);
  }
  else
  {
    cmh_setToaDef(numBuf,&type);
  }
  

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_csvm csvm;
    UBYTE len;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CSVM;
    cmd.params.ptr_set_plus_csvm=&csvm;

    len=strlen(numBuf);

    csvm.mode=(T_BAT_plus_csvm_mode)mode;
    
    if ((len) AND (len<=BAT_MAX_CSVM_NUMBER_LEN))
    {
      csvm.v_number=TRUE;      
      csvm.c_number=(U8)len;
      memcpy(csvm.number,numBuf,len);
    }
    else
    {
      csvm.v_number=FALSE;      
    }
    csvm.type=(S16)toa_val;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = sAT_PlusCSVM((T_ACI_CMD_SRC)srcId, mode, numBuf, (UBYTE)strlen(numBuf), &type);

  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCSVM (char *cl, UBYTE srcId)
{
  T_ACI_CSVM_MOD mode=CSVM_MOD_NotPresent;
  T_ACI_RETURN   ret = AT_FAIL;
  SHORT          toa_val=0;
  CHAR           numBuf[MAX_CC_ORIG_NUM_LEN];

  TRACE_FUNCTION("queatPlusCSVM()");

  memset( numBuf, 0, sizeof(numBuf));

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CSVM;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_csvm = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PlusCSVM((T_ACI_CMD_SRC)srcId, &mode, numBuf, sizeof(numBuf), &toa_val);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  else
  {
    sprintf(g_sa,"+CSVM: %d,\"%s\",%d",mode,numBuf,toa_val);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }

  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLAN         |
+--------------------------------------------------------------------+

  PURPOSE : +CLAN command (Set language)
*/

GLOBAL T_ATI_RSLT setatPlusCLAN (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN    ret = AT_FAIL;
  CHAR            lng_str[CLAN_CODE_LEN+1]={0};
  SHORT           i;
  T_ACI_LAN_SUP   lngCde;
  CHAR            *auptr="au";
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCLAN()");

  cl = parse(cl,"s",(LONG)3,lng_str);
  if(!cl OR *lng_str EQ '\0')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  if (strcmp(lng_str, auptr))
  {
    for(i=0;lngs[i].str NEQ 0;i++)
    {
      if (!strcmp(lngs[i].str,lng_str))
      {
        lngCde.str=lngs[i].str;
        lngCde.lng=lngs[i].lng;
       break;
      }
    }
    if(lngs[i].str EQ 0)
    {
      cmdCmeError(CME_ERR_OpNotSupp);
      return ATI_FAIL;
    }
  }
  else
  {
    lngCde.str = lng_str;
    lngCde.lng = CLAN_LNG_AUT;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_clan clan;
    UBYTE len;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CLAN;
    cmd.params.ptr_set_plus_clan=&clan;

    len=strlen(lng_str);

    if (len>BAT_MAX_CLAN_CODE_LEN)
    {
      cmdCmeError(CME_ERR_Unknown);
      return ATI_FAIL;
    }

    memcpy(clan.code,lng_str,len);
    clan.c_code=(U8)len;
    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CLAN;
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = sAT_PlusCLAN((T_ACI_CMD_SRC)srcId,&lngCde);
  if (ret EQ AT_EXCT)
  {
    src_params->curAtCmd    = AT_CMD_CLAN;
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPlusCLAN(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  CHAR            *me="+CLAN: ";
  T_ACI_RETURN    ret = AT_FAIL;
  T_ACI_LAN_SUP   lngCde;
  CHAR            lang_buffer[3]; /* 2 chars for language + 0 terminated string */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif

  TRACE_FUNCTION("queatPlusCLAN()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CLAN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_clan = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  /* to be sure that last cipher is 0 (string)*/
  memset(lang_buffer, 0, sizeof(lang_buffer));
  lngCde.str = lang_buffer;

  ret = qAT_PlusCLAN((T_ACI_CMD_SRC)srcId, &lngCde);
  switch(ret)
  {
  case(AT_CMPL):
    sprintf(g_sa,"%s%s", me, lngCde.str);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    break;

  case(AT_EXCT):
    src_params->curAtCmd    = AT_CMD_CLAN;
    break;

  case(AT_BUSY):
    cmdCmeError(CME_ERR_SimBusy);
    break;

  default:
    cmdCmeError(CME_ERR_Unknown);
    break;
  }
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT tesatPlusCLAN (CHAR *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char          *me="+CLAN: ";
  T_ACI_RETURN  ret = AT_FAIL;
  T_ACI_LAN_SUP lnglst[MAX_LAN];
  SHORT         lastIdx;
  UBYTE         i;
  SHORT         pos;
#endif

  TRACE_FUNCTION("tesatPlusCLAN()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_TST_PLUS_CLAN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_tst_plus_clan = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = tAT_PlusCLAN((T_ACI_CMD_SRC)srcId, &lastIdx, &lnglst[0] );
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }

  pos=sprintf(g_sa,"%s",me);

  for(i=0;i<lastIdx;i++)
  {
    pos += sprintf(g_sa+pos,"%s,",lnglst[i].str);
  }

  g_sa[pos-1] = '\0';
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPercentPBCF   |
+--------------------------------------------------------------------+

  PURPOSE : %PBCF command (Set phonebook configuration)
*/

GLOBAL T_ATI_RSLT setatPercentPBCF (CHAR *cl, UBYTE srcId)
{
  T_ACI_PBCF_LDN ldn = PBCF_LDN_NotPresent;
  T_ACI_PBCF_LRN lrn = PBCF_LRN_NotPresent;
  T_ACI_PBCF_LMN lmn = PBCF_LMN_NotPresent;
  T_ACI_RETURN ret = AT_FAIL;

  cl = parse (cl, "ddd", &ldn, &lrn, &lmn);

  /*lint -e685 always evaluates to false*/
  if (!cl OR
      ldn < PBCF_LDN_NotPresent OR ldn > PBCF_LDN_Disable OR
      lrn < PBCF_LRN_NotPresent OR lrn > PBCF_LRN_Disable OR
      lmn < PBCF_LMN_NotPresent OR lmn > PBCF_LMN_Disable)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }
  
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_pbcf pbcf;
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

    TRACE_FUNCTION("setatPercentPBCF() calls bat_send() <=== as APPLICATION");
    
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_PBCF;
    cmd.params.ptr_set_percent_pbcf=&pbcf;

    pbcf.ldn = (T_BAT_percent_pbcf_ldn)ldn;
    pbcf.lrn = (T_BAT_percent_pbcf_lrn)lrn;
    pbcf.lmn = (T_BAT_percent_pbcf_lmn)lmn;
    
    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_P_PBCF;
    return(ATI_EXCT);
  }

#else /* FF_ATI_BAT */
  TRACE_FUNCTION("setatPercentPBCF()");

  ret = sAT_PercentPBCF((T_ACI_CMD_SRC)srcId, ldn, lrn, lmn);
  
  if(ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
  
  return (map_aci_2_ati_rslt(ret));

#endif /* FF_ATI_BAT */
}


GLOBAL T_ATI_RSLT queatPercentPBCF (CHAR *cl, UBYTE srcId)
{
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    TRACE_FUNCTION("queatPercentPBCF() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_PBCF;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_pbcf = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
  
#else /* FF_ATI_BAT */
  CHAR *me = "%PBCF: ";
  T_ACI_PBCF_LDN ldn = PBCF_LDN_NotPresent;
  T_ACI_PBCF_LRN lrn = PBCF_LRN_NotPresent;
  T_ACI_PBCF_LMN lmn = PBCF_LMN_NotPresent;

  TRACE_FUNCTION("queatPercentPBCF()");

  qAT_PercentPBCF ((T_ACI_CMD_SRC)srcId, &ldn, &lrn, &lmn);

  sprintf (g_sa, "%s%d, %d, %d", me, ldn, lrn, lmn);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
#endif /* FF_ATI_BAT */
}

#endif /* ATI_PHB_C */

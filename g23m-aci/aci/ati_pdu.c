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
|  Purpose :  AT Command Interpreter Extension for SMS PDU mode
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_PDU_C
#define ATI_PDU_C

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
#include "aci.h"
#include "psa.h"
#include "psa_sms.h"
#include "aci_lst.h"

#include "cmh.h"
#include "cmh_sms.h"
#include "gdi.h"
#include "audio.h"
#include "p_sim.h"
#include "p_aci.h"
#include "aoc.h"
#include "aci.h"
#include "pcm.h"
#include "rx.h"
#include "pwr.h"

#include "aci_cmd.h"
#include "aci_mem.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

GLOBAL USHORT GsmToHex     (UBYTE in, char * buffer, USHORT pos);
// GLOBAL USHORT HexAdress    (CHAR * adress, UBYTE ton, UBYTE npi, char * buffer, USHORT pos);
// GLOBAL USHORT HexAdressTP  (CHAR * adress, UBYTE ton, UBYTE npi, char * buffer, USHORT pos);
// GLOBAL USHORT HexScts      (T_ACI_VP_ABS * scts, char * buffer, USHORT pos);
// GLOBAL USHORT HexMessage   (T_ACI_SM_DATA * data, UBYTE length, char * buffer, USHORT pos);
// GLOBAL char * GSMAddress   (char * cl, char * address, T_ACI_TOA *tosca);
GLOBAL char * HexToGsm     (char * cl, UBYTE * value);

#ifdef WIN32
EXTERN T_ACI_SMS_READ   smsReadMode;
#endif

LOCAL SHORT  length_of_pdu_message = 0;
LOCAL UBYTE  pdu_error_detected = FALSE;


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : atPlusCMGSPdu      |
+--------------------------------------------------------------------+

  PURPOSE : +CMGS command (SEND Message) in PDU mode.
*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL T_ATI_RSLT atPlusCMGSPdu (char *cl, UBYTE srcId)
{
  T_ACI_RETURN   ret = AT_FAIL;
  T_ACI_SM_DATA  pdu;
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  pdu.len = 0;

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  TRACE_FUNCTION("atPlusCMGSPdu()");

  if (src_params->text_mode EQ CMD_MODE)
  {
    /*
     * Extract the length (excluding SCA octets) outto be parsed from
     * the input pointer cl
     */
    cl = parse (cl,"r",&length_of_pdu_message);
    if ( !cl OR length_of_pdu_message EQ 0)
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    src_params->text_mode = TXT_MODE;
    return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    UBYTE i = 0;
    UBYTE sca_len = 0;
    UBYTE offset  = 0;
    UBYTE pdu_message_octets_lenth = 0;

    src_params->text_mode = CMD_MODE;
    /*
     * Input line with pdu is given
     */
    pdu_error_detected = FALSE;

    HexToGsm (cl, &sca_len);
    
    TRACE_EVENT_P1("%d",sca_len);

    if (sca_len > ((MAX_SMS_ADDR_DIG+1)/2) + 1)
    {
      TRACE_ERROR("SCA too long !!!");
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }

    /*
    calculation of the real length of PDU string

    The first octet in the input is an indicator of the length of the SMSC information supplied.

    And this Octet and the SMSC content do not count in the real length of PDU string.

    so here, the length of the input needs to minus 2 + sca_len*2 and then divide 2.
    */
    pdu_message_octets_lenth = (strlen(cl)-2-sca_len*2)/2;

    TRACE_EVENT_P1("pdu_message_octets_lenth == %d",pdu_message_octets_lenth);

    if(pdu_message_octets_lenth NEQ length_of_pdu_message)
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      TRACE_EVENT("ERROR: input pdu message length do not match the real length!");
      return (ATI_FAIL);
    }

    if (sca_len EQ 0)  /* no SCA given */
    {
      offset = CodeRPAddress(pdu.data,
                                pSMSSetPrm -> sca.c_num,
                                pSMSSetPrm -> sca.ton,
                                pSMSSetPrm -> sca.npi,
                                pSMSSetPrm -> sca.num);

      pdu.len = length_of_pdu_message + offset;
      cl += 2;
    }
    else
    {
      pdu.len = length_of_pdu_message + sca_len + 1;
    }

    for (i=offset; i<pdu.len AND *cl NEQ '\0' AND i<MAX_SM_LEN; i++)
    {
      cl = HexToGsm (cl, &pdu.data[i]);
    }

#ifdef FF_ATI_BAT
    {
      T_BAT_cmd_send cmd;
      T_BAT_cmd_set_plus_cmgs cmgs;

      /*
      *   Check that BAT can handle this size of PDU.
      */
      if (pdu.len>BAT_MAX_SM_LEN)
      {
        TRACE_EVENT("ERROR: PDU too big for BAT");
        cmdCmsError(CMS_ERR_UnknownErr);
        return (ATI_FAIL);
      }

      cmd.ctrl_params=BAT_CMD_SET_PLUS_CMGS;
      cmd.params.ptr_set_plus_cmgs=&cmgs;

      /*
      *   Copy in the PDU data, secure in the knowledge that we have
      *   enough room for it.
      */
      memcpy(&cmgs.pdu,pdu.data,BAT_MAX_SM_LEN);

      cmgs.length=(U8)pdu.len;
      cmgs.c_pdu=(U8)pdu.len;

      bat_send(ati_bat_get_client(srcId), &cmd);
    }

#else /* no FF_ATI_BAT */

    /*
     * send the SMS message using extracted values
     */
    ret = sAT_PlusCMGSPdu ( (T_ACI_CMD_SRC)srcId, &pdu );

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  // use aciErrDesc
      return (ATI_FAIL);
    }

#endif /* no FF_ATI_BAT */

    /*
     *  rCI_OK will emitting +CMGS: <mr>[,<ackpdu>]
     */
    src_params->curAtCmd = AT_CMD_CMGS;
    return ATI_EXCT;
  }
}


#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCNMAPdu      |
+--------------------------------------------------------------------+

  PURPOSE : +CNMA command (new message acknowledgement) in PDU mode
*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL T_ATI_RSLT atPlusCNMAPdu (char *cl, UBYTE srcId)
{
  UBYTE          msg_type;
  static SHORT   n = -1;
  T_ACI_SM_DATA  pdu = {0};
  T_ACI_RETURN   ret = AT_FAIL;
  UBYTE pdu_message_octets_lenth = 0;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);




  TRACE_FUNCTION("atPlusCNMAPdu()");



  if (src_params->text_mode EQ CMD_MODE)
  {
    /*
     * normal command line
     */
    if (*cl EQ '\0')
    {
      /*
       * no parameters, like text mode
       */
      n = 1;
    }
    else
    {
      /*
       * Extract the acknowledge parameter n
       */
      length_of_pdu_message = 0;
      cl = parse(cl,"rr",&n, &length_of_pdu_message);
      if ( !cl)
      {
        cmdCmsError(CMS_ERR_OpNotAllowed);
        return (ATI_FAIL);
      }

   
    
      switch (n)
      {
        case -1:
        case 0:
        case 1:
          n = 1;
          if (length_of_pdu_message)
          {
            src_params->text_mode = TXT_MODE;
            return (ATI_EXCT);
          }
          break;
        case 2:
          if (length_of_pdu_message)
          {
           src_params->text_mode = TXT_MODE;
            return (ATI_EXCT);
          }
         break;
        default:
          cmdCmsError(CMS_ERR_OpNotAllowed);
          return (ATI_FAIL);
      }
    }
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    src_params->text_mode = CMD_MODE;

/*
    calculation of the real length of PDU string

    The entering of PDU is done similarly as specified in command Send Message +CMGS, 
    except that the format of <ackpdu> is used instead of <pdu> 
    (i.e. SMSC address field is not present).

    so here, the length of the input needs to divide 2.
    */


    pdu_message_octets_lenth = strlen(cl)/2;

    TRACE_EVENT_P1("pdu_message_octets_lenth == %d",pdu_message_octets_lenth);

    if(pdu_message_octets_lenth NEQ length_of_pdu_message)
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      TRACE_EVENT("ERROR: input pdu message length do not match the real length!");
      return (ATI_FAIL);
    }

    /*
     * Input line with pdu is given
     */
    pdu_error_detected = FALSE;
    /*
     * get the message type
     */
    HexToGsm (cl, &msg_type);

    if ((msg_type & 3) EQ 0) // SMS_DELIVER_REPORT
    {
      int i;
      pdu.len = (UBYTE)length_of_pdu_message;
      for (i=0; i<length_of_pdu_message; i++)
      {
        cl = HexToGsm (cl, &pdu.data[i]);
      }
    }
    else
      pdu_error_detected = TRUE;

    if (pdu_error_detected)
    {
      cmdCmsError ( CMS_ERR_InValPduMod );
      return (ATI_FAIL);
    }
  }

 

  /*
   * send the SMS command using extracted values
   */
  ret = sAT_PlusCNMAPdu ((T_ACI_CMD_SRC)srcId, n, &pdu);
  switch(ret)
  {
    case AT_FAIL:
      cmdCmsError (CMS_ERR_NotPresent);  // use aciErrDesc
      /*
       * no break
       */
    /*lint -fallthrough */
     case AT_CMPL:
       break;
     default:
       TRACE_EVENT("atPlusCNMAPdu() : Error !! Not AT_FAIL or AT_COMPL");
       break;
  }
  return (map_aci_2_ati_rslt(ret));
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : atPlusCMGWPdu      |
+--------------------------------------------------------------------+

  PURPOSE : +CMGW command (Write message to memory) in PDU mode.
*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL T_ATI_RSLT atPlusCMGWPdu (char *cl, UBYTE srcId)
{
  T_ACI_SMS_STAT    stat;
  T_ACI_RETURN      ret = AT_FAIL;
  static UBYTE      message_status = NOT_PRESENT_8BIT;
  T_SMS_SET_PRM    *pSMSSetPrm; /* points to SMS parameter set */
  T_ACI_SM_DATA     pdu;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  TRACE_FUNCTION("atPlusCMGWPdu()");

  if (src_params->text_mode EQ CMD_MODE)
  {
    /*
     * normal command line
     */
    stat = SMS_STAT_StoUnsent;  /* STO UNSENT as default */
    /*
     * Extract the length and status to be parsed from the input pointer cl
     */
    cl = parse(cl,"rd",&length_of_pdu_message,&stat);
    if ( !cl OR length_of_pdu_message EQ 0)
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }

    if (stat < SMS_STAT_RecUnread OR stat > SMS_STAT_All)
    {
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }
    else
    {
      message_status = (UBYTE)stat;
    }
    /*
     * wait for next input containing the PDU_is_given
     */
    src_params->text_mode = TXT_MODE;
    return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    UBYTE offset = 0;
    UBYTE sca_len;
    UBYTE i;
    UBYTE pdu_message_octets_lenth = 0;

    src_params->text_mode = CMD_MODE;
    /*
     * Input line with pdu is given
     */

    /* if the character sent is ESC, then abort command CLB 16.11.00 */
    if (*cl EQ 0x1B)
    {
      TRACE_EVENT("Send message command cancelled by user");

      return ATI_CMPL_NO_OUTPUT;
    }

    pdu_error_detected = FALSE;

    HexToGsm (cl, &sca_len);

    if (sca_len > ((MAX_SMS_ADDR_DIG+1)/2) + 1)
    {
      TRACE_ERROR("SCA too long !!!");
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }

    /*
    calculation of the real length of PDU string

    The first octet in the input is an indicator of the length of the SMSC information supplied.

    And this Octet and the SMSC content do not count in the real length of PDU string.

    so here, the length of the input needs to minus 2 + sca_len*2 and then divide 2.
    */
    pdu_message_octets_lenth = (strlen(cl)-2-sca_len*2)/2;

    TRACE_EVENT_P1("pdu_message_octets_lenth == %d",pdu_message_octets_lenth);

    if(pdu_message_octets_lenth NEQ length_of_pdu_message)
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      TRACE_EVENT("ERROR: input pdu message length do not match the real length!");
      return (ATI_FAIL);
    }


    if (sca_len EQ 0)  /* no SCA given */
    {
      offset = CodeRPAddress(pdu.data,
                                pSMSSetPrm -> sca.c_num,
                                pSMSSetPrm -> sca.ton,
                                pSMSSetPrm -> sca.npi,
                                pSMSSetPrm -> sca.num);
      cl += 2;
      pdu.len = length_of_pdu_message + offset;
    }
    else
    {
      pdu.len = length_of_pdu_message + sca_len + 1;
    }

    for (i=offset; i<pdu.len AND *cl NEQ '\0' AND i<MAX_SM_LEN; i++)
    {
      cl = HexToGsm (cl, &pdu.data[i]);
    }

    ret = sAT_PlusCMGWPdu ( (T_ACI_CMD_SRC)srcId, message_status, &pdu);

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  // use aciErrDesc
      return (ATI_FAIL);
    }
    /*
     *  rCI_OK will emitting +CMGW: <index>
     */
    src_params->curAtCmd = AT_CMD_CMGW;
    return ATI_EXCT;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : atPlusCMGCPdu      |
+--------------------------------------------------------------------+

  PURPOSE : +CMGC command (Send SMS command) in PDU mode
*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL T_ATI_RSLT atPlusCMGCPdu (char *cl, UBYTE srcId)
{
  T_ACI_RETURN      ret = AT_FAIL;
  T_ACI_SM_DATA     pdu;
  T_SMS_SET_PRM    *pSMSSetPrm; /* points to SMS parameter set */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  pdu.len = 0;

  TRACE_FUNCTION("atPlusCMGCPdu()");

  if (src_params->text_mode EQ CMD_MODE)
  {
    /*
     * normal command line
     */
   /*
    * Extract the length to be parsed from the input pointer cl
    */
   cl = parse(cl,"r",&length_of_pdu_message);
   if ( !cl OR length_of_pdu_message EQ 0)
   {
     cmdCmsError(CMS_ERR_OpNotAllowed);
     return (ATI_FAIL);
   }
   /*
    * wait for next input containing the PDU_is_given
    */
   src_params->text_mode = TXT_MODE;
   return (ATI_EXCT);
  } /* end of if (src_params->text_mode EQ CMD_MODE) */
  else
  {
    UBYTE offset = 0;
    UBYTE sca_len;
    UBYTE i;
    UBYTE pdu_message_octets_lenth = 0;

    src_params->text_mode = CMD_MODE;
    /*
     * Input line with pdu is given
     */

    pdu_error_detected = FALSE;

    HexToGsm (cl, &sca_len);

    if (sca_len > ((MAX_SMS_ADDR_DIG+1)/2) + 1)
    {
      TRACE_ERROR("SCA too long !!!");
      cmdCmsError(CMS_ERR_OpNotAllowed);
      return (ATI_FAIL);
    }

    /*
    calculation of the real length of PDU string

    The first octet in the input is an indicator of the length of the SMSC information supplied.

    And this Octet and the SMSC content do not count in the real length of PDU string.

    so here, the length of the input needs to minus 2 + sca_len*2 and then divide 2.
    */
    pdu_message_octets_lenth = (strlen(cl)-2-sca_len*2)/2;

    TRACE_EVENT_P1("pdu_message_octets_lenth == %d",pdu_message_octets_lenth);

    if(pdu_message_octets_lenth NEQ length_of_pdu_message)
    {
      cmdCmsError (CMS_ERR_OpNotAllowed);
      TRACE_EVENT("ERROR: input pdu message length do not match the real length!");
      return (ATI_FAIL);
    }    

    if (sca_len EQ 0)  /* no SCA given */
    {
      offset = CodeRPAddress(pdu.data,
                                pSMSSetPrm -> sca.c_num,
                                pSMSSetPrm -> sca.ton,
                                pSMSSetPrm -> sca.npi,
                                pSMSSetPrm -> sca.num);
      cl += 2;
      pdu.len = length_of_pdu_message + offset;
    }
    else
    {
      pdu.len = length_of_pdu_message + sca_len + 1;
    }

    for (i=offset; i<pdu.len AND *cl NEQ '\0' AND i<MAX_SM_LEN; i++)
    {
      cl = HexToGsm (cl, &pdu.data[i]);
    }

    ret = sAT_PlusCMGCPdu ((T_ACI_CMD_SRC)srcId, &pdu);

    if ( ret NEQ AT_EXCT )
    {
      cmdCmsError (CMS_ERR_NotPresent);  // use aciErrDesc
      return (ATI_FAIL);
    }
    /*
     *  rCI_OK will emitting +CMGC: <mr>[,<ackpdu>]
     */
    src_params->curAtCmd = AT_CMD_CMGC;
    return ATI_EXCT;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMGLPdu    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGL call back in PDU mode

*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL void rCI_Plus_Percent_CMGLPdu  ( T_MNSMS_READ_CNF *mnsms_read_cnf, 
                                        T_ACI_AT_CMD cmd )
{
  T_ACI_SMS_STAT   stat;

  USHORT  i         = 0;
  USHORT  pos       = 0;
  UBYTE   sca_len;
  UBYTE  length_sms, length=0;
  UBYTE  fo, addr_len, dcs, vp_format=TP_VPF_ABSOLUTE;
  UBYTE  *data_ptr;

  UBYTE srcId = srcId_cb;

#ifndef FF_ATI_BAT
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  T_ACI_PB_TEXT alpha;
#endif

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_Plus_Percent_CMGLPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  /* convert status from PSA type to CMH type */
  cmhSMS_getStatCmh ( mnsms_read_cnf->status, &stat );

  /*
   * print response and index, status of SMS message
   */
  if (cmd EQ AT_CMD_CMGL )
  {
    pos = sprintf ( g_sa, "+CMGL: ");
  }
  else
  {
    pos=sprintf (g_sa, " %s: ", "%CMGL");
  }
  pos += sprintf ( g_sa + pos, "%d,%d,", mnsms_read_cnf->rec_num, stat);

#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents an easier solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }

#else

  cmhSMS_getPhbEntry( mnsms_read_cnf->sms_sdu.buf, &alpha, stat);

  /*
   * print alpha if available
   */
  if ( alpha.len NEQ 0 )
  {
    utl_chsetFromGsm ( (UBYTE*)alpha.data,
                       alpha.len,
                       (UBYTE*)cvtdAlpha,
                       sizeof(cvtdAlpha),
                       &lenCvtdAlpha,
                       GSM_ALPHA_Def );
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
  }

#endif /*FF_ATI_BAT*/

  /* length of SCA including length byte and TOA byte */
  sca_len = mnsms_read_cnf->sms_sdu.buf[0] + 1;

  data_ptr = &mnsms_read_cnf->sms_sdu.buf[sca_len];
  fo  = *data_ptr++;
  length++;

  if ((fo & TP_MTI_MASK) NEQ TP_MTI_SMS_STATUS_REP)
  {
    if ((fo & TP_MTI_MASK) EQ TP_MTI_SMS_SUBMIT)
    {
      /* SMS-SUBMIT */
      data_ptr++;
      length++;
      vp_format = fo & TP_VPF_MASK;
    }
    
    addr_len = *data_ptr;
    length += 4 + (addr_len+1)/2;
    
    /* data_ptr points to TP-DCS now */
    data_ptr += 3 + (addr_len+1)/2;
    
    dcs = *data_ptr++;
    
    switch (vp_format)
    {
    /*
    * Check validity period format bit 4 and 3
      */
    case TP_VPF_NOT_PRESENT:  // TP-VP not present
      break;
    case TP_VPF_RELATIVE:     // TP-VP relative format
      length++;
      data_ptr++;
      break;
    case TP_VPF_ENHANCED:     // TP-VP enhanced format
    case TP_VPF_ABSOLUTE:     // TP-VP absolute format
      length += 7;
      data_ptr += 7;
      break;
    }
    
    /* data_ptr points to TP-UDL now,
    * calculate the length excluding SCA octets
    */
    if (cmhSMS_getAlphabetPp (dcs) EQ 0)
      length_sms = length + (*data_ptr * 7 + 7)/8 + 1; // 7 bit alphabet
    else
      length_sms = length + *data_ptr + 1;
    
    pos += sprintf ( g_sa + pos, ",%d", length_sms );
    io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    pos = 0;
  }
  else
  {
    length_sms=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);
    pos += sprintf ( g_sa + pos, ",%d,", length_sms-sca_len );
    sca_len = 0;
  }

  /*
   * Print PDU + service centre address
   */
  for (i=0;i<(USHORT)(sca_len+length_sms);i++)
    pos = GsmToHex (mnsms_read_cnf->sms_sdu.buf[i], g_sa, pos);

  /*
   * Send response
   */
  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMGRPdu    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGR call back in PDU mode

*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL void rCI_Plus_Percent_CMGRPdu (T_MNSMS_READ_CNF * mnsms_read_cnf,
                                      T_ACI_AT_CMD cmd)
{
  USHORT  pos       = 0;
  T_ACI_SMS_STAT   stat;
  USHORT  i;

  UBYTE  sca_len;
  UBYTE  length_sms, length=0;
  UBYTE  fo, addr_len, dcs, vp_format=TP_VPF_ABSOLUTE;
  UBYTE  *data_ptr;

  UBYTE srcId = srcId_cb;

#ifndef FF_ATI_BAT
  CHAR   cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT lenCvtdAlpha;
  T_ACI_PB_TEXT alpha;
#endif

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_Plus_Percent_CMGRPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  /*
   * print response for SMS message
   */
  if (cmd EQ AT_CMD_CMGR)
  {
    pos = sprintf ( g_sa, "+CMGR: ");
  }
  else
  {
    pos=sprintf (g_sa, " %s: ", "%CMGR");
  }

  /* convert status from PSA type to CMH type */
  cmhSMS_getStatCmh ( mnsms_read_cnf->status, &stat );

  /*
   * print status of the message
   */
  pos += sprintf ( g_sa + pos, "%d,", stat );

#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents a quick and dirty solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }

#else

  cmhSMS_getPhbEntry( mnsms_read_cnf->sms_sdu.buf, &alpha, stat);

  /*
   * print alpha if available
   */
  if ( alpha.len NEQ 0 )
  {
    utl_chsetFromGsm ( (UBYTE*)alpha.data,
                       alpha.len,
                       (UBYTE*)cvtdAlpha,
                       sizeof(cvtdAlpha),
                       &lenCvtdAlpha,
                       GSM_ALPHA_Def );
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
  }

#endif

  /* length of SCA including length byte and TOA byte */
  sca_len = mnsms_read_cnf->sms_sdu.buf[0] + 1;

  data_ptr = &mnsms_read_cnf->sms_sdu.buf[sca_len];
  fo  = *data_ptr++;
  length++;

  if ((fo & TP_MTI_MASK) NEQ TP_MTI_SMS_STATUS_REP)
  {
    if ((fo & TP_MTI_MASK) EQ TP_MTI_SMS_SUBMIT)
    {
      /* SMS-SUBMIT */
      data_ptr++;
      length++;
      vp_format = fo & TP_VPF_MASK;
    }
    
    addr_len = *data_ptr;
    length += 4 + (addr_len+1)/2;
    
    /* data_ptr points to TP-DCS now */
    data_ptr += 3 + (addr_len+1)/2;
    
    dcs = *data_ptr++;
    
    switch (vp_format)
    {
    /*
    * Check validity period format bit 4 and 3
      */
    case TP_VPF_NOT_PRESENT:  // TP-VP not present
      break;
    case TP_VPF_RELATIVE:     // TP-VP relative format
      length++;
      data_ptr++;
      break;
    case TP_VPF_ENHANCED:     // TP-VP enhanced format
    case TP_VPF_ABSOLUTE:     // TP-VP absolute format
      length += 7;
      data_ptr += 7;
      break;
    }
    
    /* data_ptr points to TP-UDL now,
    * calculate the length excluding SCA octets
    */
    if (cmhSMS_getAlphabetPp (dcs) EQ 0)
      length_sms = length + (*data_ptr * 7 + 7)/8 + 1; // 7 bit alphabet
    else
      length_sms = length + *data_ptr + 1;
    
    pos += sprintf ( g_sa + pos, ",%d", length_sms );
    io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
    pos = 0;
  }
  else
  {
    length_sms=((mnsms_read_cnf->sms_sdu.l_buf+7)/8);
    pos += sprintf ( g_sa + pos, ",%d,", length_sms-sca_len );
    sca_len = 0;
  }

  /*
   * Print PDU + service centre address
   */
  /*If the SMS length recevied incorrectly (length_sms > SIM_PDU_LEN)
    then there is a chance of reading the buf[] beyond SIM_PDU_LEN. 
    By using the MINIMUM macro the max length limit
    to SIM_PDU_LEN if the length of SMS is greater than SIM_PDU_LEN 
   */
  length_sms = MINIMUM (sca_len+length_sms, SIM_PDU_LEN);
  for (i=0;i<(USHORT)length_sms;i++)
    pos = GsmToHex (mnsms_read_cnf->sms_sdu.buf[i], g_sa, pos);

  /*
   * Send response
   */
  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMSSPdu    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMSS call back in PDU mode

*/

GLOBAL void rCI_PlusCMSSPdu (T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf)
{
  USHORT pos       = 0;
  UBYTE  length;
  USHORT i;
  UBYTE  srcId = srcId_cb;
  UBYTE sca_len;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMSSPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMSS: %d", mnsms_submit_cnf->tp_mr);


  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_submit_cnf->sms_sdu.l_buf)
    {
      pos+=sprintf(g_sa+pos, ",\"");  /* parameter shall be bounded by double quote characters, see 07.07 <ackpdu> */
      sca_len = mnsms_submit_cnf->sms_sdu.buf[0] + 1;
      length = ((mnsms_submit_cnf->sms_sdu.l_buf+7)/8);

      /*
       * Print ACK PDU (without SCA field)
       */
      for (i=sca_len; i<length; i++)
        pos = GsmToHex (mnsms_submit_cnf->sms_sdu.buf[i], g_sa, pos);

      pos+=sprintf(g_sa+pos, "\"");
    }
  }

  /*
   * Send response
   */
  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMGSPdu    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGS call back in PDU mode

*/

GLOBAL void rCI_PlusCMGSPdu (T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf)
{
  USHORT pos       = 0;
  UBYTE  length;
  UBYTE  srcId = srcId_cb;
  USHORT i;
  UBYTE sca_len;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMGSPdu()");


  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMGS: %d", mnsms_submit_cnf->tp_mr);


  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_submit_cnf->sms_sdu.l_buf)
    {
      pos+=sprintf(g_sa+pos, ",\"");  /* parameter shall be bounded by double quote characters, see 07.07 <ackpdu> */
      sca_len = mnsms_submit_cnf->sms_sdu.buf[0] + 1;
      length = ((mnsms_submit_cnf->sms_sdu.l_buf+7)/8);

      /*
       * Print ACK PDU (without SCA field)
       */
      for (i=sca_len; i<length; i++)
        pos = GsmToHex (mnsms_submit_cnf->sms_sdu.buf[i], g_sa, pos);

      pos+=sprintf(g_sa+pos, "\"");
    }
  }

  /*
   * Send response
   */
  io_sendMessageEx( srcId, g_sa,
                   (T_ATI_OUTPUT_TYPE) (ATI_NORMAL_OUTPUT |
                    ATI_BEGIN_CRLF_OUTPUT |
                    ATI_END_CRLF_OUTPUT));

}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PercentCMGRSPdu|
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGRS call back in PDU mode

*/

GLOBAL void rCI_PercentCMGRSPdu ( UBYTE mode,
                                  T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf,
                                  T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind )
{
  USHORT pos       = 0;
  UBYTE  length;
  UBYTE  srcId = srcId_cb;
  USHORT i;
  UBYTE sca_len = mnsms_retrans_cnf->sms_sdu.buf[0] + 1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PercentCMGRSPdu()");


  src_params->curAtCmd = AT_CMD_NONE;

  if (mode EQ CMGRS_MODE_MANUAL_RETRANS AND mnsms_retrans_cnf NEQ NULL)
  {
    pos=sprintf(g_sa,"%s: %d", "%CMGRS", mnsms_retrans_cnf->tp_mr);


    if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
    {
      pos+=sprintf(g_sa+pos, ",");
      length = ((mnsms_retrans_cnf->sms_sdu.l_buf+7)/8) - sca_len;

      /*
       * Print ACK PDU (without SCA field)
       */
      for (i=sca_len;i<length;i++)
        pos = GsmToHex (mnsms_retrans_cnf->sms_sdu.buf[i], g_sa, pos);
    }
  }
  else if (mode EQ CMGRS_MODE_ENABLE_AUTO_RETRANS AND mnsms_send_prog_ind NEQ NULL)
  {
    pos=sprintf(g_sa,"%s: 1,%d,%d", "%CMGRS", 
                                    mnsms_send_prog_ind->resend_count,
                                    mnsms_send_prog_ind->max_retrans);
  }
  else
  {
    TRACE_ERROR("%CMGRS: wrong combination of parameters");
    return;
  }

  /*
   * Send response
   */
  io_sendMessageEx( srcId, g_sa,
                    (T_ATI_OUTPUT_TYPE)(ATI_NORMAL_OUTPUT |
                    ATI_BEGIN_CRLF_OUTPUT |
                    ATI_END_CRLF_OUTPUT));

}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMGCPdu    |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMGC call back in PDU mode

*/

GLOBAL void rCI_PlusCMGCPdu (T_MNSMS_COMMAND_CNF * mnsms_command_cnf)
{
  USHORT pos       = 0;
  UBYTE  length;
  UBYTE  srcId = srcId_cb;
  USHORT i;
  UBYTE sca_len;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCMGCPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  pos=sprintf(g_sa,"+CMGC: %d", mnsms_command_cnf->tp_mr);

  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
  {
    if (mnsms_command_cnf->sms_sdu.l_buf)
    {
      pos+=sprintf(g_sa+pos, ",\"");  /* parameter shall be bounded by double quote characters, see 07.07 <ackpdu> */
      sca_len = mnsms_command_cnf->sms_sdu.buf[0] + 1;
      length = ((mnsms_command_cnf->sms_sdu.l_buf+7)/8);

      /*
       * Print ACK PDU (without SCA field)
       */
      for (i=sca_len; i<length; i++)
        pos = GsmToHex (mnsms_command_cnf->sms_sdu.buf[i], g_sa, pos);

      pos+=sprintf(g_sa+pos, "\"");
    }
  }

  /*
   * Send response
   */
  io_sendMessageEx( srcId, g_sa,
                   (T_ATI_OUTPUT_TYPE) (ATI_NORMAL_OUTPUT |
                    ATI_BEGIN_CRLF_OUTPUT |
                    ATI_END_CRLF_OUTPUT));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCMTPdu     |
+--------------------------------------------------------------------+

  PURPOSE : handles AT_PlusCMT unsolicited message in PDU mode

*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL void rCI_PlusCMTPdu (T_MNSMS_MESSAGE_IND * mnsms_message_ind)
{
  USHORT  pos       = 0;
  UBYTE   l_sms_with_sca;
  UBYTE   l_sms_without_sca;
  UBYTE   srcId = srcId_cb;

#ifndef FF_ATI_BAT
  CHAR    cvtdAlpha[2*MAX_ALPHA_LEN];
  USHORT  lenCvtdAlpha;
  T_ACI_PB_TEXT alpha;
  T_ACI_SMS_STAT   stat;
#endif

  USHORT i;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  UBYTE sca_len = mnsms_message_ind->sms_sdu.buf[0] + 1;

  TRACE_FUNCTION("rCI_PlusCMTPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  /*
   * print response for SMS message
   */
  pos = sprintf ( g_sa, "+CMT: ");

  /*
   * print alpha if available
   */
#ifdef FF_ATI_BAT

  /*
  *   Extracting the alphanumeric data from the phonebook at this point
  *   would defeat the object (testing the BAT message). Having this
  *   data stored globally represents a quick and dirty solution to the
  *   problem of conveying it here.
  */
  if (smsShrdPrm.alpha_len)
  {
    pos+=sprints(g_sa+pos,smsShrdPrm.alpha,smsShrdPrm.alpha_len);
  }

#else

  /* convert status from PSA type to CMH type */
  cmhSMS_getStatCmh ( mnsms_message_ind->status, &stat );

  cmhSMS_getPhbEntry( mnsms_message_ind->sms_sdu.buf, &alpha, stat);


  if ( alpha.len NEQ 0 )
  {
    utl_chsetFromGsm ( (UBYTE*)alpha.data,
                       alpha.len,
                       (UBYTE*)cvtdAlpha,
                       sizeof(cvtdAlpha),
                       &lenCvtdAlpha,
                       GSM_ALPHA_Def );
    pos += sprints ( g_sa + pos, cvtdAlpha, lenCvtdAlpha );
  }

#endif /*FF_ATI_BAT*/

  l_sms_with_sca = ((mnsms_message_ind->sms_sdu.l_buf+7)/8);
  l_sms_without_sca = l_sms_with_sca - sca_len;

  pos += sprintf ( g_sa + pos, ",%d", l_sms_without_sca );
  io_sendIndication ( srcId, g_sa, ATI_FORCED_OUTPUT );

  pos = 0;

  /*
   * Print PDU + service centre address field
   */
  for (i=0;i<(USHORT)l_sms_with_sca;i++)
    pos = GsmToHex (mnsms_message_ind->sms_sdu.buf[i], g_sa, pos);

  /*
   * Send response
   */
  io_sendIndication ( srcId, g_sa, ATI_FORCED_OUTPUT );

}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCBMPdu     |
+--------------------------------------------------------------------+

  PURPOSE : handles Cell Broadcast unsolicited message in PDU mode

*/

#if defined (SMS_PDU_SUPPORT)

GLOBAL void rCI_PlusCBMPdu (T_MMI_CBCH_IND * mmi_cbch_ind)
{
  int i;
  USHORT pos = 0;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCBMPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  /*
   * print response for CBM message
   */
  pos = sprintf ( g_sa, "+CBM: ");
  pos += sprintf ( g_sa + pos, "%d", (int)mmi_cbch_ind->cbch_len );

  io_sendIndication ( srcId, g_sa, ATI_FORCED_OUTPUT );

  pos = 0;

  for (i = 0; i < (int)mmi_cbch_ind->cbch_len; i++)
    pos = GsmToHex (mmi_cbch_ind->cbch_msg[i], g_sa, pos);

  io_sendMessageEx ( srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_INDICATION_OUTPUT +
                                ATI_FORCED_OUTPUT +
                                ATI_END_CRLF_OUTPUT ));

}
#endif


#if defined (SMS_PDU_SUPPORT)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : rCI_PlusCDSPdu     |
+--------------------------------------------------------------------+

  PURPOSE : handles Status Indication unsolicited message in PDU mode

*/
GLOBAL void rCI_PlusCDSPdu (T_MNSMS_STATUS_IND * mnsms_status_ind)
{
  USHORT  pos       = 0;
  UBYTE   l_sms_with_sca;
  UBYTE   l_sms_without_sca;
  UBYTE sca_len = mnsms_status_ind->sms_sdu.buf[0] + 1;
  USHORT i;
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);


  TRACE_FUNCTION("rCI_PlusCDSPdu()");

  src_params->curAtCmd = AT_CMD_NONE;

  /*
   * print response for SMS message
   */
  pos = sprintf ( g_sa, "+CDS: ");

  l_sms_with_sca = ((mnsms_status_ind->sms_sdu.l_buf+7)/8);
  l_sms_without_sca = l_sms_with_sca - sca_len;

  pos += sprintf ( g_sa + pos, "%d", l_sms_without_sca );

  io_sendIndication ( srcId, g_sa, ATI_FORCED_OUTPUT );

  pos = 0;

  /*
   * Print PDU + service centre address field
   */
  for (i=0;i<(USHORT)l_sms_with_sca;i++)
    pos = GsmToHex (mnsms_status_ind->sms_sdu.buf[i], g_sa, pos);

  /*
   * Send response
   */
  io_sendMessageEx ( srcId, g_sa, (T_ATI_OUTPUT_TYPE)(ATI_INDICATION_OUTPUT +
                                ATI_FORCED_OUTPUT +
                                ATI_END_CRLF_OUTPUT) );

}
#endif /* SMS_PDU_SUPPORT */



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : HexMessage         |
+--------------------------------------------------------------------+

  PURPOSE : converts a sms message to HEX and fill it into a buffer.

*/

/* does not seem to be used */
/*GLOBAL USHORT HexMessage (T_ACI_SM_DATA * data, UBYTE length, char * buffer, USHORT pos)
{
  int i;

  pos = GsmToHex (data->len, buffer, pos);

  for (i=0;i<(int)length;i++)
    pos = GsmToHex (data->data[i], buffer, pos);

  return pos;
} */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : HexScts            |
+--------------------------------------------------------------------+

  PURPOSE : converts a service centre timestamp to HEX and fill it into a buffer.

*/

/* doesn't seem to be used... */
/*GLOBAL USHORT HexScts (T_ACI_VP_ABS * scts, char * buffer, USHORT pos)
{
  UBYTE tz;
  
  TRACE_EVENT_P1("TZ =%x", scts->timezone);
  
  pos = GsmToHex ((UBYTE)((scts->year[1]<<4)+scts->year[0]),
                  buffer, pos);
  pos = GsmToHex ((UBYTE)((scts->month[1]<<4)+scts->month[0]),
                  buffer, pos);
  pos = GsmToHex ((UBYTE)((scts->day[1]<<4)+scts->day[0]),
                  buffer, pos);
  pos = GsmToHex ((UBYTE)((scts->hour[1]<<4)+scts->hour[0]),
                  buffer, pos);
  pos = GsmToHex ((UBYTE)((scts->minute[1]<<4)+scts->minute[0]),
                  buffer, pos);
  pos = GsmToHex ((UBYTE)((scts->second[1]<<4)+scts->second[0]),
                  buffer, pos);

  tz = cmhSMS_setTimezone(scts->timezone);
  pos = GsmToHex (tz, buffer, pos);
  
  return pos;
}*/


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : GsmToHex           |
+--------------------------------------------------------------------+

  PURPOSE : converts a GSM character to HEX and fill it into a buffer.

*/

GLOBAL USHORT GsmToHex (UBYTE in, char * buffer, USHORT pos)
{
  const UBYTE  hexVal[17] = {"0123456789ABCDEF"};

  buffer[pos]   = hexVal[ in >> 4    ];
  buffer[pos+1] = hexVal[ in &  0x0F ];
  buffer[pos+2] = '\0';
  return pos+2;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : HexAdress          |
+--------------------------------------------------------------------+

  PURPOSE : converts a adress to HEX and fill it into a buffer.

*/

#if 0
GLOBAL USHORT HexAdress (CHAR * adress, UBYTE ton, UBYTE npi, char * buffer, USHORT pos)
{
  int i;
  int len;

  len = (strlen (adress)+1)/2;

  pos = GsmToHex  ((UBYTE)(len+1), buffer, pos);

  pos = GsmToHex  ((UBYTE)(0x80 + (ton<<4) + npi), buffer, pos);

  for (i=0;i<len;i++)
  {
    if (i EQ len-1)
    {
      /*
       * last two digits
       */
      if (strlen(adress) & 1)
      {
        pos = GsmToHex ((UBYTE)(0xF0 + (adress[2*i] - 0x30)), buffer, pos);
        continue;
      }
    }
    pos = GsmToHex ((UBYTE)(((adress [2*i+1]-0x30) << 4) +
                     (adress [2*i] - 0x30)), buffer, pos);
  }
  return pos;
}
#endif /* 0 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : HexAdressTP        |
+--------------------------------------------------------------------+

  PURPOSE : converts a adress to HEX and fill it into a buffer for TP-adresses.

*/

#if 0
GLOBAL USHORT HexAdressTP (CHAR * adress, UBYTE ton, UBYTE npi, char * buffer, USHORT pos)
{
  int i;
  int len;

  len = (strlen (adress)+1)/2;

  if ( ton EQ 0x05 AND strlen (adress) NEQ 4 ) /* PATCH VO - if (patch) else ... */
    pos = GsmToHex  ((UBYTE)(strlen (adress) * 2), g_sa, pos);
  else
    pos = GsmToHex  ((UBYTE)strlen(adress), g_sa, pos);

  pos = GsmToHex  ((UBYTE)(0x80 + (ton<<4) + npi), g_sa, pos);

  if ( ton EQ 0x05 AND strlen (adress) NEQ 4 ) /* PATCH VO - if (patch) else ... */
  {
    for (i=0;i<(int)strlen (adress);i++)
      pos = GsmToHex ((UBYTE)adress[i], g_sa, pos);
  }
  else
  {
    for (i=0;i<len;i++)
    {
      if (i EQ len-1)
      {
        /*
         * last two digits
         */
        if (strlen(adress) & 1)
        {
          pos = GsmToHex ((UBYTE)(0xF0 + (adress[2*i] - 0x30)), g_sa, pos);
          continue;
        }
      }
      pos = GsmToHex ((UBYTE)(((adress [2*i+1]-0x30) << 4) + (adress [2*i] - 0x30)),
                      g_sa, pos);
    }
  }
  return pos;
}
#endif /* 0 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : HexToGsm           |
+--------------------------------------------------------------------+

  PURPOSE : converts a HEX value to the GSM character.

*/

GLOBAL char * HexToGsm (char * cl, UBYTE * value)
{
  int i;
  UBYTE character;

  /*
   * initialise the output value
   */
  *value = 0;

  /*
   * for both nibbles
   */
  for (i=0;i<2;i++)
  {
    /*
     * shift the result of the last loop
     */
    *value = (*value) << 4;

    /*
     * Isolate next nibble in ASCII
     */
    character = toupper(*cl++);

    /*
     * convert Nibble character to value
     */
    switch (character)
    {
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
        *value = *value + (character - 'A' + 10);
        break;
      default:
        /*
         * 0-9
         */
        *value = *value + (character - '0');
        break;
    }
  }

  /*
   * return pointer to the rest of PDU
   */
  return cl;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : GSMAddress         |
+--------------------------------------------------------------------+

  PURPOSE : converts a adress coded with HEX values to the internal
            data structures.

*/

/* does not seem to be  used */
#if 0
GLOBAL char * GSMAddress (char * cl, char * address, T_ACI_TOA *tosca)
{
  int i, j;
  UBYTE value;
  UBYTE len;
  UBYTE digit;

  /*
   * calculate the number of octets which are used for the address
   */

  cl = HexToGsm (cl, &value);
  if (value EQ 0)   /* SCA not given */
  {
    address[0] = 0;
    return cl;
  }
  len = value - 1;

  /*
   * check against maximum value
   */
  if (len > MAX_SMS_ADDR_DIG/2)
  {
    pdu_error_detected = TRUE;
    return cl;
  }

  /*
   * calculate the type of number and numbering plan identification
   */
  cl = HexToGsm (cl, &value);

  tosca->ton = (value >> 4) & 7;
  tosca->npi = value & 0x0F;


  /*
   * convert the digits
   */
  j = 0;
  for (i = 0; i < (int)len; i++)
  {
    cl = HexToGsm (cl, &value);

    /*
     * check digit n
     */
    address[j++] = (value & 0x0F) + 0x30;
    /*
     * check digit n+1
     */
    digit = (value >> 4) & 0x0F;
    if (digit < 10)
      address[j++] = digit + 0x30;
  }
  /*
   * string termination
   */
  address[j] = 0;

  /*
   * return pointer to the rest of the PDU
   */
  return cl;
}
#endif /* 0 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : GSMAddressTP       |
+--------------------------------------------------------------------+

  PURPOSE : converts a TP-adress coded with HEX values to the internal
            data structures.

*/

/* does not seem to be used */
#if 0
GLOBAL char * GSMAddressTP (char * cl, char * address, T_ACI_TOA *toda)
{
  int i, j;
  UBYTE value;
  UBYTE len;
  UBYTE digit;

  /*
   * calculate the number of octets which are used for the address
   */
  cl = HexToGsm (cl, &value);
  len = (value + 1)/2;

  /*
   * check against maximum value
   */
  if (len > MAX_SMS_ADDR_DIG/2)
  {
    pdu_error_detected = TRUE;
    return cl;
  }
  /*
   * calculate the type of number and numbering plan identification
   */
  cl = HexToGsm (cl, &value);

  toda->ton = (value >> 4) & 7;
  toda->npi = value & 0x0F;

  /*
   * convert the digits
   */
  j = 0;
  for (i=0;i<(int)len;i++)
  {
    cl = HexToGsm (cl, &value);

    /*
     * check digit n
     */
    address[j++] = (value & 0x0F) + 0x30;
    /*
     * check digit n+1
     */
    digit = (value >> 4) & 0x0F;
    if (digit < 10)
      address[j++] = digit + 0x30;
  }
  /*
   * string termination
   */
  address[j] = 0;

  /*
   * return pointer to the rest of the PDU
   */
  return cl;
}
#endif /* 0 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_PDU            |
| STATE   : code                        ROUTINE : GSMMessage         |
+--------------------------------------------------------------------+

  PURPOSE : converts a sms message from HEX and fill it into a buffer.

*/

#if 0
GLOBAL char *GSMMessage (char *cl, UBYTE *data, SHORT oct_length, SHORT max_length)
{
  SHORT i;

  /*
   * check against maximum value
   */
  if (oct_length > max_length)
  {
    pdu_error_detected = TRUE;
    return cl;
  }
  memset (data, 0, max_length);

  /*
   * copy message
   */
  for (i = 0; i < oct_length; i++)
    cl = HexToGsm (cl, &data[i]);

  return cl;
}
#endif /* 0 */

#endif

#endif /* ATI_PDU_C */

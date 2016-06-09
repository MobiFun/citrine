/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SMSR
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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for
|             the short message service.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_SMSR_C
#define CMH_SMSR_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifdef UART
#include "dti.h"
#include "dti_conn_mng.h"
#endif

#include "aci.h"
#include "aci_lst.h"
#include "aci_mem.h"
#include "psa.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "psa_cc.h"

#include "cmh.h"
#include "cmh_sms.h"
#include "psa_sim.h"
#include "cmh_sim.h"
#include "psa_util.h"
#include "phb.h"

#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#include "cmh_sat.h"
#endif /* of SIM_TOOLKIT */

#if (defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_) OR defined (SMI)
#include "conc_sms.h"
#endif /* ##if (defined (MFW) AND !defined (FF_MMI_RIV)) OR defined (_CONC_TESTING_) OR defined (SMI)*/

#ifdef _CONC_TESTING_
#include "aci_mfw.h"
#endif

/*==== CONSTANTS ==================================================*/

/* TP-User-Data-Header-Indicator (TP-UDHI) */
#define UDHI_MASK      0x40
#define UDHI_PRESENT   0x40
#define UDHI_ABSENT    0x00

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

EXTERN T_ACI_LIST *set_prm_list;

/*==== FUNCTIONS ==================================================*/

/* Implements Measure # 59 */
LOCAL void cmhSMS_SendCbDwnlSIM(UBYTE *data, int data_len);
/* Implements Measure # 104 */
LOCAL void cmhSMS_ResCSAS(T_ACI_CMD_SRC ownBuf, T_ACI_AT_CMD  cmdBuf, USHORT errCode);
/* Implements Measure 114 */
LOCAL void cmhSMS_ErrRes(T_ACI_CMD_SRC ownBuf, T_ACI_AT_CMD  cmdBuf, USHORT cause);

LOCAL BOOL cmhSMS_cpySTtoSM(T_ACI_CMGL_SM *p_sm,T_MNSMS_READ_CNF * mnsms_read_cnf);

/* help function for cmhSMS_SMRead to inform user in case of
 * CCD decoding error with SMS
 * for this case the T_ACI_SMS_STAT has been extended with a value
 * of SMS_STAT_Invalid = -2 (e.g to tell BMI to emit "INVALID MESSAGE") */
LOCAL void cmhSMS_invalidSMS_notification_to_user(T_MNSMS_READ_CNF * mnsms_read_cnf)
{
  T_ACI_AT_CMD  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  T_ACI_CMD_SRC ownBuf = smsShrdPrm.smsEntStat.entOwn;

  TRACE_FUNCTION ("cmhSMS_SMRead_invalidSMS_notification_to_user()");
  if (smsShrdPrm.pDecMsg->stat EQ SMS_STAT_Invalid)
  {
    switch( smsShrdPrm.smsEntStat.curCmd )
    {
      case ( AT_CMD_CMGR ):
      {
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
        smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
        smsShrdPrm.smsEntStat.entOwn =CMD_SRC_NONE; 
        if (smsShrdPrm.rplyCB.cmgr NEQ NULL)
        {
          smsShrdPrm.rplyCB.cmgr( smsShrdPrm.pDecMsg, NULL );
        }
        R_AT ( RAT_OK, ownBuf) ( cmdBuf );
        break;
      }
      case ( AT_CMD_CMGL ):
      {
        R_AT ( RAT_CMGL, smsShrdPrm.smsEntStat.entOwn ) ( smsShrdPrm.pDecMsg ); /* intermediate result */
        if (mnsms_read_cnf->rec_next NEQ SMS_RECORD_NOT_EXIST)
        {
          psaSMS_ReadReq ( smsShrdPrm.mem1, mnsms_read_cnf->rec_next,
                           smsShrdPrm.rdMode, cmglStat );
        }
        break;
      }
    }  
  }
  else
  {
    /*
     * SMS decoding failed, but the status has not been set to SMS_STAT_Invalid
     */
    TRACE_FUNCTION ("cmhSMS_SMRead_invalidSMS_notification_to_MFW(): [ERR] status not SMS_STAT_Invalid !");
  }
}


#ifdef SIM_TOOLKIT
LOCAL void cmhSMS_SATResult (T_ACI_SAT_TERM_RESP *p_resp_data,
                             USHORT              cause)
{
  if (GET_CAUSE_ORIGIN_ENTITY(cause) EQ SMSCP_ORIGINATING_ENTITY)
  { /* CP-CAUSE */
    p_resp_data->add_content = GET_CAUSE_VALUE(cause) | 0x80;
    psaSAT_SendTrmResp( RSLT_NTW_UNAB_PROC, p_resp_data );
  }
  else
  { /* RP-CAUSE or other */
    if (GET_CAUSE_ORIGIN_ENTITY(cause) EQ SMSRP_ORIGINATING_ENTITY)
      p_resp_data->add_content = GET_CAUSE_VALUE(cause);
    else
      p_resp_data->add_content = ADD_NO_CAUSE;
    psaSAT_SendTrmResp( RSLT_SMS_ERR, p_resp_data);
  }
}
#endif /* #ifdef SIM_TOOLKIT */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_CBMIndication         |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            the receiving of a cell broadcast message.
*/
GLOBAL SHORT cmhSMS_CBMIndication ( T_MMI_CBCH_IND * mmi_cbch_ind )
{
  USHORT         sn;       /* serial number               */
  USHORT         mid;      /* message identifier          */
  UBYTE          dcs;      /* data coding scheme          */
  UBYTE          page;     /* actual page number          */
  UBYTE          pages;    /* total number of pages       */
  T_ACI_CBM_DATA msg;      /* cell broadcast message data */
  UBYTE          idx;

  TRACE_FUNCTION ("cmhSMS_CBMIndication ()");

  /*
   *-----------------------------------------------------------------
   * process parameters for new message indication
   *-----------------------------------------------------------------
   */
  sn    = ( ( SHORT )mmi_cbch_ind->cbch_msg[0] << 8 ) +
                     mmi_cbch_ind->cbch_msg[1];
  mid   = ( ( SHORT )mmi_cbch_ind->cbch_msg[2] << 8 ) +
                     mmi_cbch_ind->cbch_msg[3];
  dcs   = mmi_cbch_ind->cbch_msg[4];
  page  = ( mmi_cbch_ind->cbch_msg[5] & 0xF0 ) >> 4;
  pages = ( mmi_cbch_ind->cbch_msg[5] & 0x0F );

  /*
   *-----------------------------------------------------------------
   * process message data, expanding from 7 to 8 bit
   *-----------------------------------------------------------------
   */
  cmhSMS_expdSmsCb ( dcs,
                     &mmi_cbch_ind->cbch_msg[CBCH_HEAD_LEN],
                     (UBYTE)(mmi_cbch_ind->cbch_len - CBCH_HEAD_LEN),
                     msg.data, &msg.len );

  /*
   *-----------------------------------------------------------------
   * new message indication
   *-----------------------------------------------------------------
   */
#ifdef FF_MMI_RIV
  {
    T_ACI_CMGF_MOD sms_input_mode = CMGF_MOD_NotPresent;

    qAT_PlusCMGF (CMD_SRC_LCL, &sms_input_mode);
    if (sms_input_mode EQ CMGF_MOD_Pdu)
    {
      rAT_PlusCBMPdu (mmi_cbch_ind);
    }
    else
    {
      R_AT( RAT_CBM, CMD_SRC_LCL )
          ( sn, mid, dcs, page, pages, &msg );
    }
  }
#else 
  R_AT( RAT_CBM, CMD_SRC_LCL )
    ( sn, mid, dcs, page, pages, &msg );
#endif

  /* If the SMS shared params indicate no valid source interested in 
   * the SMS indications, the indication is buffered */ /* Issue 25033 */
  if( smsShrdPrm.smsSrcId EQ CMD_SRC_NONE )
  {
#ifdef DTI
    T_CNMI_IND ind;

    memcpy (&ind.cbm, mmi_cbch_ind, sizeof(T_MMI_CBCH_IND));
    cmd_addCnmiNtry ( CNMI_CBM, &ind );
#endif
  }
  else
  {
#ifdef FF_ATI
    /* tell any remote source */
    for( idx = CMD_SRC_LCL+1; idx < CMD_SRC_MAX; idx++ )
    {
      if (IS_SRC_USED (idx))
      {
        R_AT( RAT_CBM, (T_ACI_CMD_SRC)idx ) ( mmi_cbch_ind );
      }
    }
#endif
  }

  return 0;
}

/* Implements Measure#32: Row 1105 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_sdu_buf_print         |
+-------------------------------------------------------------------+

  PURPOSE : This function will print the sdu_bug contents
            
*/
GLOBAL void cmhSMS_sdu_buf_print ( U8     *buf,
                                   USHORT offset )
{
  char trcBuf[80];
  char *writeP;
  int count,i,j;

  TRACE_EVENT("buf: ");
  
  count = offset;
  for(i=0;i<2;i++)
  {
    writeP = trcBuf;
    for (j=0; j<20; j++, count++)
    {
      writeP += sprintf (writeP, " %02X", buf[count]);
    }
    *writeP = '\0';
    TRACE_EVENT(trcBuf);
  }        
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSInitState          |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle the
            SMS_STATE_INITIALISING state.
*/
GLOBAL SHORT cmhSMS_SMSInitState ( T_MNSMS_MESSAGE_IND * mnsms_message_ind )
{
#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  T_ACI_CMGL_SM  sm;
  T_ACI_SMS_STOR mem; /* holds message storage   */
  UBYTE retCode = TRUE;
  UBYTE msg_type;
#endif /* SMI OR defined MFW */

#ifdef _CONC_TESTING_
#ifndef NTRACE
/* Implements Measure#32: Row 1105 */
  USHORT offset;

  TRACE_EVENT_P1("initstate:rec_num: %d", mnsms_message_ind->rec_num);
  TRACE_EVENT_P1("initstate:status:  %d", mnsms_message_ind->status);
  offset = mnsms_message_ind->sms_sdu.o_buf>>3;
/* Implements Measure#32: Row 1105 */
  cmhSMS_sdu_buf_print(&(mnsms_message_ind->sms_sdu.buf[0]), offset);
#endif
#endif


  TRACE_FUNCTION ("cmhSMS_SMSInitState()");


  switch (mnsms_message_ind->mem_type)
  {
    case MEM_ME:
      if (mnsms_message_ind->rec_num NEQ SMS_RECORD_NOT_EXIST)
      {
        smsShrdPrm.aci_sms_parameter.meUsed++;
      }
      if (smsShrdPrm.aci_sms_parameter.meTotal EQ 0)
      {
        smsShrdPrm.aci_sms_parameter.meTotal = mnsms_message_ind->rec_max;
      }
      break;
    case MEM_SM:
      if (mnsms_message_ind->rec_num NEQ SMS_RECORD_NOT_EXIST)
      {
        smsShrdPrm.aci_sms_parameter.simUsed++;
        TRACE_EVENT_P1("simUsed: %d", smsShrdPrm.aci_sms_parameter.simUsed);
      }
      if (smsShrdPrm.aci_sms_parameter.simTotal EQ 0)
      {
        smsShrdPrm.aci_sms_parameter.simTotal = mnsms_message_ind->rec_max;
        TRACE_EVENT_P1("simTotal: %d", smsShrdPrm.aci_sms_parameter.simTotal);
      }
      break;
    default:
      TRACE_EVENT("wrong memtype");
  }

#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  cmhSMS_getMemCmh ( mnsms_message_ind->mem_type, &mem );
  cmhSMS_SMSQueryType (&mnsms_message_ind->sms_sdu , &msg_type);
  memset(&sm,0,sizeof(T_ACI_CMGL_SM) );
  if(msg_type NEQ TP_MTI_SMS_STATUS_REP)
  {
    /* Implements Measure # 110 */
    retCode = cmhSMS_cpyMsgIndReadCnf (&sm, 
                                       &mnsms_message_ind->status, 
                                       &mnsms_message_ind->sms_sdu,
                                       mnsms_message_ind->rec_num);
  }
  if (retCode EQ FALSE)
  {
    TRACE_EVENT("cmhSMS_SMSInitState():[ERR] decoding of SMS");
    if (sm.stat NEQ SMS_STAT_Invalid)
    {
       TRACE_EVENT("cmhSMS_SMSInitState():[ERR] SMS decoding failed, but status not SMS_STAT_Invalid");
    }
  }

#if defined FF_MMI_RIV
  /* in case of RIV MMI: check if SMBS is enabled for sending also PDU data */
  if(smsShrdPrm.perccmgf_smbs_mode EQ PERC_SMBS_MOD_ENABLE)
  {
    rAT_PlusCMTPdu (mnsms_message_ind);
  }
#endif /* FF_MMI_RIV */
//TISH, patch for class 0 SMS when initializing. 
//start
  if (mnsms_message_ind->rec_num EQ SMS_RECORD_NOT_EXIST)
  {

         R_AT( RAT_CMT, CMD_SRC_LCL ) (&sm);

  }
  else
//end
  R_AT( RAT_CMTI, CMD_SRC_LCL )
         (mem, sm.msg_ref, &sm);
#endif /* defined SMI OR defined MFW OR defined FF_MMI_RIV */

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSDelCnf             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_DELETE_CNF primitive.
*/
GLOBAL SHORT cmhSMS_SMSDelCnf ( T_MNSMS_DELETE_CNF * mnsms_delete_cnf )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */

#ifdef _CONC_TESTING_
  T_ACI_CMD_SRC srcId;  /* Hold the request Source ID */
  T_CONC_INIT_RETURN ret;
#endif

  TRACE_FUNCTION ("cmhSMS_SMSDelCnf()");

  /*-------------------------------------------------------------------------*
   * In R99 one more flag is added in CMGD command which is used to delete   *
   * multiple messages of the same status ( READ, UNSENT, SENT etc ).        * 
   * For this purpose the setting of global variables are done, once         *
   * the it is confirmed that all the messages of the same status are        * 
   * deleted.                                                                *
   *-------------------------------------------------------------------------*/

   cmdBuf = smsShrdPrm.smsEntStat.curCmd;
   ownBuf = smsShrdPrm.smsEntStat.entOwn;
#ifdef _CONC_TESTING_
   srcId  = smsShrdPrm.smsEntStat.entOwn;
#endif
#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  SET_OWNBUF_CONC;
#endif /*#if defined SMI OR defined MFW*/

    
  if(smsShrdPrm.status > CMGD_DEL_INDEX)
  {
    ownBuf = smsShrdPrm.smsEntStat.entOwn;
  }

  if (IS_CAUSE_INVALID(mnsms_delete_cnf->cause))  /* no error */
  {
    /* Check if the mnsms_delete_req for deleting messages of particular 
       status returned indicating no messages of requested status. Note 
       that in this case also we return OK to the user */ 
    if(mnsms_delete_cnf->rec_num NEQ 0)
    {
      switch (mnsms_delete_cnf->mem_type)
      {
        case MEM_ME:
          smsShrdPrm.aci_sms_parameter.meUsed--;
          break;
        case MEM_SM:
          smsShrdPrm.aci_sms_parameter.simUsed--;
          break;
        default:
          TRACE_EVENT("wrong memtype");
          return AT_FAIL;
      }
    }
  /*-----------------------------------------------------------------------*
   * The below code is the new code added on 07/08/2003 due to the new     *
   * flag for CMGD command as mentioned in R99. This flag handles the      *
   * deletion of multiple messages. The deletion of concatination is       * 
   * handled with different way other than handling normal indexed         *
   * messages. The deletion of normal concatination messages is done as    *
   * in the exiting code. But deletion of concatination message is handled *
   * in the different way when delete flag is greater than ZERO. If the    *
   * delete flag in CMGD command is greater than ZERO, then there is no    * 
   * concatination check is done. After receiving the response from SMS    *
   * with the deleted messages number, then we will do the concatination   *
   * check. If CONC_NEEDED flag is set, then we will clear the             *
   * concatinated list from the ACI. If CONC_NOT_NEEDED is set then check  *
   * for the next record values, if it is greater than ZERO then send one  *
   * more request for SMS to delete that message and repeat the above      *
   * process untill ACI recive's the next record values as ZERO.           *
   *-----------------------------------------------------------------------*/

     if( smsShrdPrm.status > CMGD_DEL_INDEX AND 
         mnsms_delete_cnf->rec_num NEQ 0)
     {
#if defined _CONC_TESTING_ 
       /*-----------------------------------------------------------------*
        *                  Check for the concatination                    *
        *-----------------------------------------------------------------*/
       
       ret= concSMS_initDeleteFromMem (srcId, mnsms_delete_cnf->rec_num);
               
       if( ret EQ CONC_NEEDED )
       {
         SET_CONC;   
         concSMS_DeleteConcList();
       }
#endif
       if ( mnsms_delete_cnf->delete_rec_next NEQ 0 )
       {
         cmhSMS_SendDelete_Req (mnsms_delete_cnf->delete_rec_next,
                                smsShrdPrm.status);
         return 0;
       }
     }
    /*-----------------------------------------------------------------------*
     *                    Reset the command context                          * 
     *-----------------------------------------------------------------------*/

    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
    smsShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;

    /*-----------------------------------------------------------------------*
     * The below code is used to handle the existing way where there is no   *
     * delete falg is set ( i.e. delete flag == 0 )                          *
     *-----------------------------------------------------------------------*/

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
    if (ownBuf EQ CMD_SRC_LCL)
    {
      if( smsShrdPrm.status EQ CMGD_DEL_INDEX )
      {
#ifdef _CONC_TESTING_
        if (!ISSET_CONC) 
        {
#endif /* _CONC_TESTING_ */
          TRACE_EVENT("you just erased a normal SMS !");
          if(smsShrdPrm.rplyCB.cmgd NEQ NULL)
          {
            smsShrdPrm.rplyCB.cmgd( );
          }
          R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#ifdef _CONC_TESTING_
        }
        else
        {
          /*-----------------------------------------------------------------*
           *   Looks just about the same BUT: ISSET_CONC could be unsent     * 
           *   in RAT_CMGD, so let this after checking ISSET_CONC            *
           *-----------------------------------------------------------------*/

          if(smsShrdPrm.rplyCB.cmgd NEQ NULL)
          {
            smsShrdPrm.rplyCB.cmgd( );
          }
        }
#endif /* _CONC_TESTING_ */
      }
      else
      {
        R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
      if(smsShrdPrm.rplyCB.cmgd NEQ NULL)
      {
        smsShrdPrm.rplyCB.cmgd( );
      }

#if defined FF_MMI_RIV
      R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else 
      /* GPF-MMI */
#ifdef _CONC_TESTING_
      if (!ISSET_CONC)
#endif /* _CONC_TESTING_ */
        R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
      }
#endif
    }
    else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*defined MFW OR defined FF_MMI_RIV*/
    {
      R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
    }
  }
  else
  {
    /*-----------------------------------------------------------------------*
     *                    Reset the command context                          * 
     *-----------------------------------------------------------------------*/

    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
    smsShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;

    if (smsShrdPrm.errorCB NEQ NULL)
    {
      smsShrdPrm.errorCB
      (
        cmdBuf,
        cmhSMS_GetCmsFromSms(mnsms_delete_cnf->cause),
        NULL
      );
    }
    else
    {
      R_AT ( RAT_CMS, ownBuf ) ( cmdBuf, cmhSMS_GetCmsFromSms(mnsms_delete_cnf->cause), NULL );
    }
  }
  /* Reset status flag to CMGD_DEL_INDEX = 0 */
  smsShrdPrm.status = CMGD_DEL_INDEX;
  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSStoCnf             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_STORE_CNF primitive.
*/
GLOBAL SHORT cmhSMS_SMSStoCnf ( T_MNSMS_STORE_CNF * mnsms_store_cnf )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */

  TRACE_FUNCTION ("cmhSMS_SMSStoCnf()");

  /*
   *-----------------------------------------------------------------
   * reset the command context
   *-----------------------------------------------------------------
   */
  cmdBuf                       = smsShrdPrm.smsEntStat.curCmd;
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
  ownBuf                       = smsShrdPrm.smsEntStat.entOwn;

#ifdef _CONC_TESTING_
  /* this is only for testing */
  SET_OWNBUF_CONC;
#endif

  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
  smsShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;

  /* Check for current cmd is CMGMDU */
  if ( cmdBuf EQ AT_CMD_P_CMGMDU )
  {
    if (IS_CAUSE_INVALID(mnsms_store_cnf->cause))
    {
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
      if (ownBuf EQ CMD_SRC_LCL)
      {
        if(smsShrdPrm.rplyCB.cmgmdu NEQ NULL)
        {
          smsShrdPrm.rplyCB.cmgmdu();
        }
      }
      else
#endif /*#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_ */
      {
         R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
      }
    }
    else
    {
      if ( ownBuf NEQ CMD_SRC_LCL )
        R_AT ( RAT_CMS, ownBuf )
        (
          cmdBuf,
          cmhSMS_GetCmsFromSms(mnsms_store_cnf->cause),
          NULL
        );
      return -1;
    }
  }
  else if (IS_CAUSE_INVALID(mnsms_store_cnf->cause))
  {
    switch (mnsms_store_cnf->mem_type)
    {
      case MEM_ME:
        smsShrdPrm.aci_sms_parameter.meUsed++;
        break;
      case MEM_SM:
        smsShrdPrm.aci_sms_parameter.simUsed++;
        break;
      default:
        TRACE_EVENT("incorrect memtype");
        return AT_FAIL;
    }
#ifdef FF_ATI
  /* "internal storage" handling -> storing from CNMI-buf to SIM/MS */
  if( smsShrdPrm.uiInternalSmsStorage NEQ CMD_SRC_NONE )
  {
    if(mnsms_store_cnf->cause NEQ SMS_NO_ERROR)
    {
      TRACE_EVENT("cmhSMS_SMSStoCnf() : error at internal SMS storaging");
      cmd_clearCnmiBuf(); /* clear CNMI buffer */
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
      return -1;
    }

    /* storing was succesful -> clear the stored msg. in CNMI buf */
    cmd_clearFirstCnmiMessage();

    /* try with the next msg. in CNMI-buf */
    if( cmd_storeNextCnmiBufMsgToSim() NEQ TRUE ) /* do nothing if no more Msg's in CNMI buffer */
    { /* no more messagees could be sent */
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
    }
    return 0; /* returns immediately if internal storage is running */
  }
#endif /* FF_ATI */
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
    if (ownBuf EQ CMD_SRC_LCL)
    {
      if(smsShrdPrm.rplyCB.cmgw NEQ NULL)
      {
        smsShrdPrm.rplyCB.cmgw( mnsms_store_cnf->rec_num, 1, mnsms_store_cnf->mem_type);
      }
#if defined FF_MMI_RIV
      R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else
      /* GPF-MMI */
      if (!ISSET_CONC) 
      {
        TRACE_EVENT("you just wrote a normal SMS !");
        R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
      }
#endif /*#if !defined FF_MMI_RIV*/
    }
    else
#endif /*#if defined MFW OR defined FF_MMI_RIV*/
    {
      R_AT ( RAT_CMGW, ownBuf ) ( mnsms_store_cnf->rec_num );
      R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
      /* inform BMI */
      cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_CMGW, mnsms_store_cnf->rec_num, 
                             BS_SPEED_NotPresent,CME_ERR_NotPresent);
    }
  }
  else
  {
    if( smsShrdPrm.uiInternalSmsStorage NEQ CMD_SRC_NONE )
    {
      smsShrdPrm.uiInternalSmsStorage = CMD_SRC_NONE;
      return 0;
    }

    /* Implements Measure 114 */
    cmhSMS_ErrRes(ownBuf, cmdBuf, mnsms_store_cnf->cause);
  }
  return 0;
}



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSSbmCnf             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_SUBMIT_CNF primitive.
*/
GLOBAL SHORT cmhSMS_SMSSbmCnf ( T_MNSMS_SUBMIT_CNF * mnsms_submit_cnf )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */
#ifdef SIM_TOOLKIT
  T_ACI_SAT_TERM_RESP resp_data;
#endif


#ifdef _CONC_TESTING_
#ifndef NTRACE
/* Implements Measure#32: Row 1105 */
  USHORT offset;

  TRACE_EVENT_P1("cause: %d", mnsms_submit_cnf->cause);
  offset = mnsms_submit_cnf->sms_sdu.o_buf>>3;
/* Implements Measure#32: Row 1105 */
  cmhSMS_sdu_buf_print(&(mnsms_submit_cnf->sms_sdu.buf[0]), offset);
#endif
#endif

  TRACE_FUNCTION ("cmhSMS_SMSSbmCnf()");

  /*
   *-----------------------------------------------------------------
   * reset the command context
   *-----------------------------------------------------------------
   */
  cmdBuf                       = smsShrdPrm.smsEntStat.curCmd;
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;

  ownBuf                       = smsShrdPrm.smsEntStat.entOwn;

#ifdef _CONC_TESTING_
  /* this is only for testing */
  SET_OWNBUF_CONC;
#endif
  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
  smsShrdPrm.owner             = (T_OWN)CMD_SRC_NONE;

#ifdef SIM_TOOLKIT
  psaSAT_InitTrmResp( &resp_data );
#endif

  if (IS_CAUSE_INVALID(mnsms_submit_cnf->cause))
  {
#ifdef REL99
    /* Since message transmission is successful, set the manual 
       retransmission flag as FALSE */
    smsShrdPrm.is_msg_present_for_retrans = FALSE;
#endif
    smsShrdPrm.aci_sms_parameter.snd_msg_ref = mnsms_submit_cnf->tp_mr;
    switch (cmdBuf)
    {
      case( AT_CMD_CMSS ):
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
        if (ownBuf EQ CMD_SRC_LCL)
        {
          if(smsShrdPrm.rplyCB.cmss NEQ NULL)
          {
            smsShrdPrm.rplyCB.cmss( mnsms_submit_cnf->tp_mr, 1 );
          }
#if defined FF_MMI_RIV
          R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else
          if (!ISSET_CONC) 
          {
            R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
          }
#endif /* FF_MMI_RIV */
        }
        else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*#if defined MFW OR defined FF_MMI_RIV*/
        {
          R_AT ( RAT_CMSS, ownBuf ) ( mnsms_submit_cnf );
          R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
          /* inform BMI */
          cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_CMSS, mnsms_submit_cnf->rec_num,
                                 BS_SPEED_NotPresent,CME_ERR_NotPresent );
        }
        break;
      case( AT_CMD_CMGS ):
#if defined (SIM_TOOLKIT)
        if (ownBuf EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT))
        {
          psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
          break;
        }
#endif
        if (mnsms_submit_cnf->rec_num NEQ SMS_RECORD_NOT_EXIST)
        {
          switch (mnsms_submit_cnf->mem_type)
          {
            case MEM_ME:
              smsShrdPrm.aci_sms_parameter.meUsed++;
              break;
            case MEM_SM:
              smsShrdPrm.aci_sms_parameter.simUsed++;
              break;
            default:
              TRACE_EVENT("incorrect memtype");
              return AT_FAIL;
          }
        }
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
        if (ownBuf EQ CMD_SRC_LCL)
        {
          if(smsShrdPrm.rplyCB.cmgs NEQ NULL)
          {
            smsShrdPrm.rplyCB.cmgs( mnsms_submit_cnf->tp_mr, 1 );
          }
#if defined FF_MMI_RIV
          R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else
          if (!ISSET_CONC)
          {
            R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
          }
#endif /* FF_MMI_RIV */
        }
        else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*#if defined MFW OR defined FF_MMI_RIV*/
        {
          R_AT ( RAT_CMGS, ownBuf ) ( mnsms_submit_cnf );
          R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
          /* inform BMI */
          cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_CMGS, mnsms_submit_cnf->rec_num, 
                                 BS_SPEED_NotPresent,CME_ERR_NotPresent );
        }
    } /* switch */
  }
  else /* if (mnsms_submit_cnf->rslt_type EQ SMS_RT_NON) */
  {
#if defined (SIM_TOOLKIT)
    if (ownBuf EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT))
    {
#ifdef REL99
      /* Manual retransmission is not allowed when last failed message 
         was sent from SAT */
      smsShrdPrm.is_msg_present_for_retrans = FALSE;
#endif /* REL99 */
      cmhSMS_SATResult (&resp_data, mnsms_submit_cnf->cause);
    }
#endif

#ifdef REL99
#if defined _CONC_TESTING_
    if (ISSET_CONC)
    {
      /* Manual retransmission is not allowed when last failed message 
         is part of a long message */
      smsShrdPrm.is_msg_present_for_retrans = FALSE;
    }
    else
#endif
    {
      /* Last failed message is available for retransmission */
      smsShrdPrm.is_msg_present_for_retrans = TRUE;
    }
#endif /* REL99 */

    /* Implements Measure 114 */
    cmhSMS_ErrRes (ownBuf, cmdBuf, mnsms_submit_cnf->cause);
  }
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSCmdCnf             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_COMMAND_CNF primitive.
*/
GLOBAL SHORT cmhSMS_SMSCmdCnf ( T_MNSMS_COMMAND_CNF * mnsms_command_cnf )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */
#ifdef SIM_TOOLKIT
  T_ACI_SAT_TERM_RESP resp_data;
  psaSAT_InitTrmResp( &resp_data );
#endif

  TRACE_FUNCTION ("cmhSMS_SMSCmdCnf()");


  /*
   *-----------------------------------------------------------------
   * reset the command context
   *-----------------------------------------------------------------
   */
  cmdBuf                       = smsShrdPrm.smsEntStat.curCmd;
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;

  ownBuf                       = smsShrdPrm.smsEntStat.entOwn;

  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
  smsShrdPrm.owner  = (T_OWN)CMD_SRC_NONE;

  if (IS_CAUSE_INVALID(mnsms_command_cnf->cause))
  {
#ifdef REL99
    /* Since message transmission is successful, set the manual 
       retransmission flag as FALSE */
    smsShrdPrm.is_msg_present_for_retrans = FALSE;
#endif /* REL99 */
#ifdef _CONC_TESTING_
    /* this is only for testing */
    SET_OWNBUF_CONC;
#endif

#if defined (SIM_TOOLKIT)
    if (ownBuf EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT))
    {
      psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
    }
#endif
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
    if (ownBuf EQ CMD_SRC_LCL)
    {
      if(smsShrdPrm.rplyCB.cmgc NEQ NULL)
      {
        smsShrdPrm.rplyCB.cmgc( mnsms_command_cnf->tp_mr );
      }
#if defined FF_MMI_RIV
      R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else
      if (!ISSET_CONC) 
      {
        R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
      }
#endif /*!defined FF_MMI_RIV*/
    }
    else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*defined MFW OR defined FF_MMI_RIV*/
    {
      R_AT ( RAT_CMGC, ownBuf ) ( mnsms_command_cnf );
      R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
      /* inform BMI */
      /* This does not make much sense, but nice to have it for future requests */
/*    cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_CMGC, -1, -1, -1 ); */
    }

  }
  else
  {
#if defined (SIM_TOOLKIT)
    if (ownBuf EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT))
    {
#ifdef REL99
      /* Manual retransmission is not allowed when last failed message 
         was sent from SAT */
      smsShrdPrm.is_msg_present_for_retrans = FALSE;
#endif /* REL99 */
      cmhSMS_SATResult (&resp_data, mnsms_command_cnf->cause);
    }
#endif

#ifdef REL99
#if defined _CONC_TESTING_
    if (ISSET_CONC) 
    {
      /* Manual retransmission is not allowed when last failed message 
         is part of a long message */
      smsShrdPrm.is_msg_present_for_retrans = FALSE;
    }
    else
#endif
    {
      /* Last failed message is available for retransmission */
      smsShrdPrm.is_msg_present_for_retrans = TRUE;
    }
#endif /* REL99 */

#ifdef REL99
    if (ownBuf NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
#endif


    R_AT ( RAT_CMS, ownBuf )
    (
      cmdBuf,
      cmhSMS_GetCmsFromSms(mnsms_command_cnf->cause),
      NULL
    );
  }
  return 0;
}

#ifdef REL99
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSRetransCnf         |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_RETRANS_CNF primitive.
*/
GLOBAL SHORT cmhSMS_SMSRetransCnf ( T_MNSMS_RETRANS_CNF * mnsms_retrans_cnf )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */

  TRACE_FUNCTION ("cmhSMS_SMSRetransCnf()");

  /*
   *-----------------------------------------------------------------
   * reset the command context
   *-----------------------------------------------------------------
   */
  cmdBuf                       = smsShrdPrm.smsEntStat.curCmd;
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;

  ownBuf                       = smsShrdPrm.smsEntStat.entOwn;

  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
  smsShrdPrm.owner             = OWN_SRC_INV;

  if (IS_CAUSE_INVALID(mnsms_retrans_cnf->cause))
  {
    /* Since message transmission is successful, set the manual 
       retransmission flag as FALSE */
    smsShrdPrm.is_msg_present_for_retrans = FALSE;

    smsShrdPrm.aci_sms_parameter.snd_msg_ref = mnsms_retrans_cnf->tp_mr;   

    if (mnsms_retrans_cnf->rec_num NEQ SMS_RECORD_NOT_EXIST)
    {
      switch (mnsms_retrans_cnf->mem_type)
      {
        case MEM_ME:
          smsShrdPrm.aci_sms_parameter.meUsed++;
          break;
        case MEM_SM:
          smsShrdPrm.aci_sms_parameter.simUsed++;
          break;
        default:
          TRACE_EVENT("incorrect memtype");
          return AT_FAIL;
      }
    }
#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
    if (ownBuf EQ CMD_SRC_LCL)
    { 
      if (smsShrdPrm.rplyCB.cmgrs NEQ NULL)
      {
        smsShrdPrm.rplyCB.cmgrs( CMGRS_MODE_MANUAL_RETRANS, 
                          mnsms_retrans_cnf->tp_mr, 
                          NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
      }
      R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
    }
    else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_*/
    {
      R_AT ( RAT_P_CMGRS, ownBuf ) ( CMGRS_MODE_MANUAL_RETRANS, 
                                     mnsms_retrans_cnf, NULL );
      R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
    }
  }
  else /* if (IS_CAUSE_INVALID(mnsms_retrans_cnf->cause)) */
  {
    if (ownBuf EQ CMD_SRC_LCL)
    {
      if (smsShrdPrm.errorCB NEQ NULL)
      {
        smsShrdPrm.errorCB
        (
          cmdBuf,
          cmhSMS_GetCmsFromSms(mnsms_retrans_cnf->cause),
          NULL
        );
      }
    }
    else if (ownBuf NEQ (T_ACI_CMD_SRC)OWN_SRC_SAT)
    {
      R_AT ( RAT_CMS, ownBuf )
      (
        cmdBuf,
        cmhSMS_GetCmsFromSms(mnsms_retrans_cnf->cause),
        NULL
      );
    }
  }
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSSendProgInd        |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_SEND_PROG_IND primitive.
*/

GLOBAL SHORT cmhSMS_SMSSendProgInd (T_MNSMS_SEND_PROG_IND * mnsms_send_prog_ind)
{
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */

  TRACE_FUNCTION ("cmhSMS_SMSSendProgInd()");

  ownBuf = smsShrdPrm.smsEntStat.entOwn;
       
#if defined (MFW) OR defined (FF_MMI_RIV) OR defined (_CONC_TESTING_)
    if (ownBuf EQ CMD_SRC_LCL)
    {          
      rAT_PercentCMGRS( CMGRS_MODE_ENABLE_AUTO_RETRANS, NOT_PRESENT_8BIT, 
                        mnsms_send_prog_ind->resend_count,
                        mnsms_send_prog_ind->max_retrans);
    }
    else
#endif /*#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_*/
    {
      R_AT ( RAT_P_CMGRS, ownBuf ) ( CMGRS_MODE_ENABLE_AUTO_RETRANS, 
                                     NULL, 
                                     mnsms_send_prog_ind );
    }
  return 0;
}
#endif /* REl99 */


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSErrorInd           |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to handle MNSMS_ERROR_IND primitive.
*/
GLOBAL SHORT cmhSMS_SMSErrorInd ( T_MNSMS_ERROR_IND * mnsms_error_ind )
{
  T_ACI_MM_CIND_VAL_TYPE sIndValues;
  int i;

  TRACE_FUNCTION ("cmhSMS_SMSErrorInd()");

  TRACE_EVENT_P1("MNSMS_ERROR_IND: 0x%4.4X", (int)mnsms_error_ind->cause);

  if( (mnsms_error_ind->cause NEQ SMS_CAUSE_MEM_FULL) AND 
      (mnsms_error_ind->cause NEQ SMS_CAUSE_MEM_AVAIL) )
  {
    smsShrdPrm.cnma_ack_expected = FALSE;
    cmhSMS_resetMtDsCnmiParam();
  }
  else
  { /* process the SMS memory full/avail indication */
    sIndValues.sCindSmsFullParam = (mnsms_error_ind->cause EQ SMS_CAUSE_MEM_AVAIL)
                                   ? CIND_SMSFULL_INDICATOR_MEMAVAIL
                                   : CIND_SMSFULL_INDICATOR_MEMFULL;
    sIndValues.sCindSignalParam  = CIND_SIGNAL_INDICATOR_INVALID;

    for( i = CMD_SRC_LCL; i < CMD_SRC_MAX; i++)
    {
      if( ((cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCINDSettings.sCindSmsFullParam EQ CIND_SMSFULL_INDICATOR_MEMFULL) AND
           (cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerIndParam EQ CMER_INDICATOR_2)) OR
          ((cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCINDSettings.sCindSmsFullParam <= CIND_SMSFULL_INDICATOR_MEMAVAIL) AND
           (cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerIndParam EQ CMER_INDICATOR_1)) )
      { /* process the service of +CIEV */
        TRACE_EVENT("send +CIEV (SMS-full/avail)");
        R_AT (RAT_CIEV, (T_ACI_CMD_SRC)i) ( sIndValues, cmhPrm[i].mmCmdPrm.sIndicationParam.sMmCMERSettings );
      }
    }
  }
  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSDeliver            |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            the receiving of a SMS-DELIVER.
*/
#ifndef NTRACE
LOCAL void trace_SMSDeliver ( T_MNSMS_MESSAGE_IND * mnsms_message_ind )
{
#ifdef _CONC_TESTING_

/* Implements Measure#32: Row 1105 */
  USHORT offset;

  TRACE_FUNCTION ("trace_SMSDeliver");

  TRACE_EVENT_P1("cmt:bitoffset:%d", mnsms_message_ind->sms_sdu.o_buf);
  TRACE_EVENT_P1("cmt:lenght:%d", mnsms_message_ind->sms_sdu.l_buf);
  offset = mnsms_message_ind->sms_sdu.o_buf>>3;
/* Implements Measure#32: Row 1105 */
  cmhSMS_sdu_buf_print(&(mnsms_message_ind->sms_sdu.buf[0]), offset);
#endif
}
#endif

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
LOCAL void rAT_SMS_Deliver(T_MNSMS_MESSAGE_IND * mnsms_message_ind, T_ACI_CMGL_SM *p_sm)
{
#ifdef FF_MMI_RIV
  T_ACI_CMGF_MOD sms_input_mode = CMGF_MOD_NotPresent;
#endif  /* FF_MMI_RIV */ 

  TRACE_FUNCTION ("rAT_SMS_Deliver");

#ifdef FF_MMI_RIV
  if(smsShrdPrm.perccmgf_smbs_mode EQ PERC_SMBS_MOD_ENABLE)
  {
    /*send both formats*/
    rAT_PlusCMTPdu (mnsms_message_ind);    
    R_AT( RAT_CMT, CMD_SRC_LCL ) (p_sm);
    return;  
  }

  /* else: no SMBS enabled, CMGF mode decides */
  qAT_PlusCMGF (CMD_SRC_LCL, &sms_input_mode);
  if (sms_input_mode EQ CMGF_MOD_Pdu)
  {
    rAT_PlusCMTPdu (mnsms_message_ind);
  }
  else
  {
    R_AT( RAT_CMT, CMD_SRC_LCL ) (p_sm);
  }
  return;
#endif   /* FF_MMI_RIV */ 

  /* if GPF MMI call usual callback */
  R_AT( RAT_CMT, CMD_SRC_LCL ) (p_sm);
}
#endif /* SMI OR defined MFW OR defined FF_MMI_RIV */

GLOBAL SHORT cmhSMS_SMSDeliver ( T_MNSMS_MESSAGE_IND * mnsms_message_ind )
{
  UBYTE         idx;
#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  T_ACI_CMGL_SM *p_sm;
#endif /* SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_*/

  TRACE_FUNCTION ("cmhSMS_SMSDeliver");

#ifndef NTRACE
  trace_SMSDeliver ( mnsms_message_ind );
#endif

  if( smsShrdPrm.CNMImt EQ 0 )
  {
    TRACE_EVENT("cmhSMS_SMSDeliver():[ERR] <mt>==0 -> SMS will not be routed ");
    return 0;
  }

#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
  p_sm = smsShrdPrm.pDecMsg;
  /* Implements Measure # 110 */
  if (cmhSMS_cpyMsgIndReadCnf (p_sm, &mnsms_message_ind->status, 
            &mnsms_message_ind->sms_sdu, mnsms_message_ind->rec_num) EQ FALSE )
  {
    TRACE_EVENT("cmhSMS_SMSDeliver():[ERR] decoding of SMS");
    if (p_sm->stat NEQ SMS_STAT_Invalid)
    {
       TRACE_EVENT("cmhSMS_SMSDeliver():[ERR] SMS decoding failed, but status not SMS_STAT_Invalid");
    }
  }
#endif /* SMI OR defined MFW OR defined FF_MMI_RIV */


  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
    smsShrdPrm.cnma_ack_expected = TRUE;

  /*
   *-----------------------------------------------------------------
   * new message indication
   *-----------------------------------------------------------------
   */

  /* inform LOCAL MMI of new message */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  rAT_SMS_Deliver(mnsms_message_ind, p_sm);
#endif /* SMI OR defined MFW OR defined FF_MMI_RIV */

  /* If the SMS shared params indicate no valid source interested in 
   * the SMS indications, the indication is buffered */ /* Issue 25033 */
  if( smsShrdPrm.smsSrcId EQ CMD_SRC_NONE )
  {
    T_CNMI_IND ind;

    TRACE_EVENT("cmhSMS_SMSDeliver(): No registered source for ");
    memcpy ( &ind.cmt, mnsms_message_ind, sizeof ( T_MNSMS_MESSAGE_IND) );
#ifdef DTI
    cmd_addCnmiNtry ( CNMI_CMT, &ind );
#endif
    if ( smsShrdPrm.cnma_ack_expected EQ TRUE )
    { /* the acknowledge must be send automatically, because terminal is busy */
      PALLOC (mnsms_ack_res, MNSMS_ACK_RES);

      TRACE_EVENT("rCI_PlusCMT(): send 'automatic' acknowledge (Phase2+ mode and buffer)");

      mnsms_ack_res->resp          = SMS_RP_ACK;
      mnsms_ack_res->sms_sdu.o_buf = 0;
      mnsms_ack_res->sms_sdu.l_buf = 0;

      PSENDX (SMS, mnsms_ack_res);

      smsShrdPrm.cnma_ack_expected = FALSE;
    }
  }
  else
  {
    /* tell any remote source */
    for( idx = CMD_SRC_LCL+1; idx < CMD_SRC_MAX; idx++ )
    {
#if defined _SIMULATION_ AND defined _CONC_TESTING_
      /* don't display "+CMT: ..." in simulation if conc testing */
      if (!concShrdPrm.concTesting)
#endif
      {
        R_AT( RAT_CMT, (T_ACI_CMD_SRC)idx ) ( mnsms_message_ind );
      }
    }
  }

  if (smsShrdPrm.pDecMsg)
  {
    ACI_MFREE(smsShrdPrm.pDecMsg);
    smsShrdPrm.pDecMsg = NULL;
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSMemory             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            an incoming SMS written to preferred message storage.
*/
GLOBAL SHORT cmhSMS_SMSMemory ( T_MNSMS_MESSAGE_IND * mnsms_message_ind)
{
  USHORT          idx; /* holds command source id */
  T_ACI_SMS_STOR  mem; /* holds message storage   */
  T_ACI_CMGL_SM  sm;

#ifdef _CONC_TESTING_
#ifndef NTRACE
/* Implements Measure#32: Row 1105 */
  USHORT offset;

  TRACE_EVENT_P1("cmti:rec_num: %d", mnsms_message_ind->rec_num);
  TRACE_EVENT_P1("cmti:status:  %d", mnsms_message_ind->status);
  offset = mnsms_message_ind->sms_sdu.o_buf>>3;
/* Implements Measure#32: Row 1105 */
  cmhSMS_sdu_buf_print(&(mnsms_message_ind->sms_sdu.buf[0]), offset);
#endif
#endif

  TRACE_FUNCTION ("cmhSMS_SMSMemory ()");

  cmhSMS_getMemCmh ( mnsms_message_ind->mem_type, &mem );

  switch (mnsms_message_ind->mem_type)
  {
    case MEM_ME:
      smsShrdPrm.aci_sms_parameter.meUsed++;
      break;
    case MEM_SM:
      smsShrdPrm.aci_sms_parameter.simUsed++;
      break;
    default:
      TRACE_EVENT("wrong memtype");
      return AT_FAIL;
  }

#if defined SMI OR defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
  /* Implements Measure # 110 */
  if (cmhSMS_cpyMsgIndReadCnf (&sm, &mnsms_message_ind->status, 
             &mnsms_message_ind->sms_sdu, mnsms_message_ind->rec_num) EQ FALSE )
  {
    TRACE_EVENT("cmhSMS_SMSMemory ():[ERR] decoding of SMS");
    if (sm.stat NEQ SMS_STAT_Invalid)
    {
       TRACE_EVENT("cmhSMS_SMSMemory():[ERR] SMS decoding failed, but status not SMS_STAT_Invalid");
    }
  }
#endif


  /*
   *-----------------------------------------------------------------
   * new message indication
   *-----------------------------------------------------------------
   */


#if defined FF_MMI_RIV
  /* in case of RIV MMI: check if SMBS is enabled for sending also PDU data */
  if(smsShrdPrm.perccmgf_smbs_mode EQ PERC_SMBS_MOD_ENABLE)
  {
    rAT_PlusCMTPdu (mnsms_message_ind);
  }
#endif /* FF_MMI_RIV */

  R_AT( RAT_CMTI, CMD_SRC_LCL ) (mem, sm.msg_ref, &sm);

  /* If the SMS shared params indicate no valid source interested in 
   * the SMS indications, the indication is buffered */ /* Issue 25033 */
  if( smsShrdPrm.smsSrcId EQ CMD_SRC_NONE )
  {
    T_CNMI_IND ind;
    ind.cmti.mem   = mem;
    ind.cmti.index = mnsms_message_ind->rec_num;
#ifdef DTI
    cmd_addCnmiNtry ( CNMI_CMTI, &ind );
#endif
  }
  else
  {
    for( idx = CMD_SRC_LCL+1; idx < CMD_SRC_MAX; idx++ )
    {
#if defined _SIMULATION_ AND defined _CONC_TESTING_
      /* don't display "+CMT: ..." in simulation if conc testing */
      if (!concShrdPrm.concTesting)
#endif
      {
        /* don't need the decoded message for Command Interpreter */
        R_AT( RAT_CMTI, (T_ACI_CMD_SRC)idx ) ( mem, mnsms_message_ind->rec_num );
      }
    }
  }

  return AT_CMPL;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSStatRpt            |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            the receiving of a SMS-STATUS-REPORT.
*/
GLOBAL SHORT cmhSMS_SMSStatRpt ( T_MNSMS_STATUS_IND * mnsms_status_ind )
{
  USHORT idx;                   /* holds command source id   */
  T_ACI_CDS_SM* p_st;

  TRACE_FUNCTION ("cmhSMS_SMSStatRpt ()");


#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CDS_SM) );
  p_st = (T_ACI_CDS_SM*)smsShrdPrm.pDecMsg;
  cmhSMS_cpyStatInd ( p_st, mnsms_status_ind);
#endif


  if (smsShrdPrm.CSMSservice EQ CSMS_SERV_GsmPh2Plus)
    smsShrdPrm.cnma_ack_expected = TRUE;

  /*
   *-----------------------------------------------------------------
   * new message indication
   *-----------------------------------------------------------------
   */
 {
   #ifdef FF_MMI_RIV
     T_ACI_CMGF_MOD sms_input_mode = CMGF_MOD_NotPresent;
   #endif /* FF_MMI_RIV */

   /* If the SMS shared params indicate no valid source interested in 
    * the SMS indications, the indication is buffered */ /* Issue 25033 */
   if( smsShrdPrm.smsSrcId EQ CMD_SRC_NONE )
   {
     T_CNMI_IND ind;

     memcpy ( &ind.cds, mnsms_status_ind, sizeof ( T_MNSMS_STATUS_IND) );
#ifdef DTI
     cmd_addCnmiNtry ( CNMI_CDS, &ind );
#endif
   }

   for( idx = 0; idx < CMD_SRC_MAX; idx++ )
   {
     if (idx EQ CMD_SRC_LCL)
     {
   #ifdef FF_MMI_RIV
       qAT_PlusCMGF (CMD_SRC_LCL, &sms_input_mode);
       if (sms_input_mode EQ CMGF_MOD_Pdu)
       {
         rAT_PlusCDSPdu (mnsms_status_ind);
       }
       else
       {
         R_AT( RAT_CDS, idx ) ( p_st );
       }
   #else
       R_AT( RAT_CDS,(T_ACI_CMD_SRC)idx ) ( p_st );
   #endif 
     }
     else
     {
       R_AT( RAT_CDS, (T_ACI_CMD_SRC)idx ) ( mnsms_status_ind );
     }
            
    }
  }

  if (smsShrdPrm.pDecMsg)
  {
    ACI_MFREE(smsShrdPrm.pDecMsg);
    smsShrdPrm.pDecMsg = NULL;
  }

  return 0;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_cpyLstData            |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to copy the data read from memory.
            It sends a notification using the callback mechanism of
            ACI if necessary.
*/
GLOBAL void cmhSMS_cpyLstData  ( T_ACI_AT_CMD     curCmd,
                                 T_ACI_CMD_SRC    entOwn,
                                 T_MNSMS_READ_CNF * mnsms_read_cnf )
{
  SHORT                 nextIdx;     /* index of next short message */

  TRACE_FUNCTION ("cmhSMS_cpyLstData ()");

  nextIdx = mnsms_read_cnf->rec_next;

  if ( entOwn EQ CMD_SRC_LCL)
  {
#ifdef FF_MMI_RIV    
    {
      T_ACI_CMGF_MOD sms_input_mode = CMGF_MOD_NotPresent;

      qAT_PlusCMGF (CMD_SRC_LCL, &sms_input_mode);
      if (sms_input_mode EQ CMGF_MOD_Pdu )
      {
        if (curCmd EQ AT_CMD_CMGL)
        {
          rAT_PlusCMGLPdu (mnsms_read_cnf);
        }
        else if (curCmd EQ AT_CMD_P_CMGL)
        {
          rAT_PercentCMGLPdu (mnsms_read_cnf);
        }
      }
      else if (curCmd EQ AT_CMD_CMGL)
      {
        R_AT ( RAT_CMGL, entOwn ) ( smsShrdPrm.pDecMsg );
      }
      else if (curCmd EQ AT_CMD_P_CMGL)
      {
        R_AT ( RAT_P_CMGL, entOwn ) ( smsShrdPrm.pDecMsg );
      }
    }
#else
    if (curCmd EQ AT_CMD_CMGL)
    {
      R_AT ( RAT_CMGL, entOwn ) ( smsShrdPrm.pDecMsg );
    }
    else if (curCmd EQ AT_CMD_P_CMGL)
    {
      R_AT ( RAT_P_CMGL, entOwn ) ( smsShrdPrm.pDecMsg );
    }
    
#endif
  }
  else if (curCmd EQ AT_CMD_CMGL)
  {
    R_AT ( RAT_CMGL, entOwn ) ( mnsms_read_cnf );
    /* inform BMI */
    cmh_logRslt ( entOwn, RAT_OK, AT_CMD_CMGL, mnsms_read_cnf->rec_num, 
                           BS_SPEED_NotPresent,CME_ERR_NotPresent);
  }
  else if (curCmd EQ AT_CMD_P_CMGL)
  {
    /* the message shall not be displayed for ReadMode as Status Change */
    if (smsShrdPrm.rdMode NEQ SMS_READ_StatusChange)
    {
      R_AT ( RAT_P_CMGL, entOwn ) ( mnsms_read_cnf );
      /* inform BMI */  
      cmh_logRslt ( entOwn, RAT_OK, AT_CMD_P_CMGL, mnsms_read_cnf->rec_num, 
                             BS_SPEED_NotPresent,CME_ERR_NotPresent );
    }
  }

  if (nextIdx NEQ SMS_RECORD_NOT_EXIST)
  {
    /*
     *-----------------------------------------------------------
     * request the next list elements of given cmglStat state
     *-----------------------------------------------------------
     */
     psaSMS_ReadReq ( smsShrdPrm.mem1, nextIdx,
                               smsShrdPrm.rdMode, cmglStat );
  }
  else
  {
    /*
     *-----------------------------------------------------------
     * no more messages are available
     *-----------------------------------------------------------
     */

    /* reset the command context */
    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
    smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

    R_AT ( RAT_OK,   entOwn ) ( curCmd );
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMReadCMSS            |
+-------------------------------------------------------------------+

  PURPOSE : This function processes the mnsms_read_cnf which was
            triggered by +CMSS.
*/
GLOBAL void cmhSMS_SMReadCMSS ( T_MNSMS_READ_CNF * mnsms_read_cnf)
{
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set       */
  UBYTE srcId;
  UBYTE fo;
  T_ACI_CMGL_SM *p_sm;
  T_tp_da da_addr;
  T_ACI_RETURN ret=AT_CMPL;
  UBYTE modify=SMS_MODIFY_NON;

#ifdef _CONC_TESTING_
/* Implements Measure#32: Row 1105 */
#endif

  TRACE_FUNCTION ("cmhSMS_SMReadCMSS ()");


  srcId = smsShrdPrm.smsEntStat.entOwn;
  pSMSSetPrm = smsShrdPrm.pSetPrm[srcId];

  if (smsShrdPrm.pDecMsg EQ NULL)
  {
    ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
    p_sm = smsShrdPrm.pDecMsg;
    /* decode message */
    /* Implements Measure # 110 */
    cmhSMS_cpyMsgIndReadCnf (p_sm, 
                             &mnsms_read_cnf->status, 
                             &mnsms_read_cnf->sms_sdu,
                             mnsms_read_cnf->rec_num);
  }
  else
    p_sm = smsShrdPrm.pDecMsg;

  memset (&da_addr, 0, sizeof(T_tp_da));

  /* start: check FDN if enabled */
   if (pb_get_fdn_mode () EQ FDN_ENABLE AND
       pb_get_fdn_classtype() EQ CLASS_VceDatFaxSms  )
  {

    TRACE_EVENT_P1("cmhSMS_SMReadCMSS: FDN check: %s", p_sm->adress );

    if (pb_check_fdn (0, (const UBYTE*) p_sm->adress) EQ PHB_OK)
    {
      TRACE_EVENT("cmhSMS_SMReadCMSS: Found match in FDN!");
    }
    else 
    {
      TRACE_EVENT("cmhSMS_SMReadCMSS: No match in FDN found, SMS rejected!");
      /* error notification */
      R_AT( RAT_CMS, smsShrdPrm.smsEntStat.entOwn )
        ( smsShrdPrm.smsEntStat.curCmd, CMS_ERR_OpNotAllowed, NULL );
      /* release context */
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE; 
      smsShrdPrm.smsEntStat.entOwn =CMD_SRC_NONE; 
      return;
    }
  } 
    /* end: check FDN if enabled */

  /* 
   * ACI-SPR-17004: set RFU bits of status to 0 if present since these 
   * should be ignored in a GSM session (GSM 11.11 section 9.3)
   */
  cmhSMS_removeStatusRFUBits( &mnsms_read_cnf->status );

  ACI_MALLOC(smsShrdPrm.tpdu.tp_submit, sizeof(T_TP_SUBMIT));

  /* SMS-SUBMIT received */
  if ((mnsms_read_cnf->status EQ SMS_RECORD_STO_UNSENT)  OR
      (mnsms_read_cnf->status EQ SMS_RECORD_STO_SENT)    OR
      (mnsms_read_cnf->status EQ SMS_RECORD_STAT_UNRCVD) OR
      (mnsms_read_cnf->status EQ SMS_RECORD_STAT_UNSTRD) OR
      (mnsms_read_cnf->status EQ SMS_RECORD_STAT_STRD)      )
  {

    /* destination address from stored message */
    if (p_sm->adress[0] NEQ '\0')
    {
      da_addr.ton = p_sm->toa.ton;
      da_addr.npi = p_sm->toa.npi;
      
      if ( da_addr.ton EQ TON_Alphanumeric )
      {
        cmhSMS_packAlphaNumAddr(p_sm->adress, &da_addr);
      }
      else
      {
        cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num, MAX_SMS_ADDR_DIG, p_sm->adress);
        da_addr.digits = da_addr.c_num;
      }
      
    }

    /* service center address from shared parameter */
    if (pSMSSetPrm->sca.c_num NEQ 0)
    {
      memcpy (&smsShrdPrm.tpdu.sc_addr, &pSMSSetPrm->sca, sizeof(T_rp_addr));
      modify |= SMS_MODIFY_SCA;

    }

    /* ACI-SPR-22528.The memory is allocated for smsShrdPrm.tpdu.tp_submit 
     only when it is necessary*/

    
    cmhSMS_fillTpSubmit (smsShrdPrm.tpdu.tp_submit,
                         (T_ACI_CMD_SRC)srcId,
                         0, /* fo */
                         0,
                         &da_addr,
                         NULL,
                         0,
                         NULL );
  }
  else
  {
    /* SMS-DELIVER received */

    T_ACI_SM_DATA packedData;
    UBYTE byte_offset=0;

    /* service center address from shared parameter */
    if (pSMSSetPrm->sca.c_num NEQ 0)
    {
      memcpy (&smsShrdPrm.tpdu.sc_addr, &pSMSSetPrm->sca, sizeof(T_rp_addr));
    }
    else
    {
      if (p_sm->sca[0] NEQ '\0')
      {
        cmhSMS_getAdrBcd( smsShrdPrm.tpdu.sc_addr.num,
                          &smsShrdPrm.tpdu.sc_addr.c_num,
                          MAX_SMS_ADDR_DIG, p_sm->sca);
        smsShrdPrm.tpdu.sc_addr.ton = p_sm->tosca.ton;
        smsShrdPrm.tpdu.sc_addr.npi = p_sm->tosca.npi;
        smsShrdPrm.tpdu.sc_addr.v_ton = TRUE;
        smsShrdPrm.tpdu.sc_addr.v_npi = TRUE;
      }
    }

    fo = (pSMSSetPrm->msgType & (~TP_MTI_MASK)) | TP_MTI_SMS_SUBMIT;

    if (p_sm->udh.len)
      byte_offset = p_sm->udh.len+1;

    /* ACI-SPR-22528.The memory is allocated for smsShrdPrm.tpdu.tp_submit 
     only when it is necessary*/

    cmhSMS_rdcSmsPp ( byte_offset,
                      p_sm -> dcs,
                      ( UBYTE * ) p_sm->data.data, ( UBYTE ) p_sm->data.len,
                      packedData.data, &packedData.len);

    if (p_sm->adress[0] NEQ '\0')
    {
      cmhSMS_getAdrBcd ( da_addr.num, &da_addr.c_num, MAX_SMS_ADDR_DIG, p_sm->adress);
      da_addr.ton = p_sm->toa.ton;
      da_addr.npi = p_sm->toa.npi;
      da_addr.digits = da_addr.c_num;
    }
    
    cmhSMS_fillTpSubmit (smsShrdPrm.tpdu.tp_submit,
                         (T_ACI_CMD_SRC)srcId,
                         fo,
                         0,
                         &da_addr,
                         &packedData,
                         p_sm->data.len,
                         &p_sm->udh );
   
    modify = SMS_MODIFY_ALL;
  }

  {
    PALLOC (mnsms_submit_req, MNSMS_SUBMIT_REQ);
    mnsms_submit_req->rec_num  = mnsms_read_cnf->rec_num;
    mnsms_submit_req->mem_type = smsShrdPrm.mem2;
    mnsms_submit_req->condx    = SMS_CONDX_OVR_ANY;
    mnsms_submit_req->modify   = modify;

#ifdef REL99
    if(srcId NEQ OWN_SRC_SAT)
    {
      mnsms_submit_req -> auto_rep_flag = smsShrdPrm.auto_repeat_flag;
    }
    else
    {
      mnsms_submit_req -> auto_rep_flag = FALSE;
    }
#endif /* REL99 */


#ifdef SIM_TOOLKIT
    if (psaSIM_ChkSIMSrvSup( SRV_MOSMCtrlSIM ))
    {
        if (simShrdPrm.setPrm[srcId].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_ACTIVE)
        {
            ret = cmhSAT_MoSmCntr( smsShrdPrm.tpdu.sc_addr,
                                 da_addr,
                                 srcId);
        }
        else
        {
            simShrdPrm.setPrm[srcId].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
        }

    }

    if (ret NEQ AT_CMPL)
    {
      /* save primitive address for SIM_TOOLKIT response */
      sat_mnsms_submit_req = mnsms_submit_req;
      return;
    }
#endif
    /* code SMS-SUBMIT here */
    cmhSMS_codeMsg (&mnsms_submit_req->sms_sdu, SMS_VT_SUBMIT,
                    &smsShrdPrm.tpdu.sc_addr, SMS_SUBMIT,
                    (UBYTE*)smsShrdPrm.tpdu.tp_submit);

    PSENDX (SMS, mnsms_submit_req);
#ifdef _CONC_TESTING_
#ifndef NTRACE
/* Implements Measure#32: Row 1105 */
    cmhSMS_sdu_buf_print(&(mnsms_submit_req->sms_sdu.buf[0]), 0);
#endif
#endif
    if (smsShrdPrm.tpdu.tp_submit NEQ NULL)
    {
      ACI_MFREE(smsShrdPrm.tpdu.tp_submit);
      smsShrdPrm.tpdu.tp_submit = NULL;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMRead                |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            a successfull reading of a short message from preferred
            message storage.
*/
#ifndef NTRACE
LOCAL void trace_read_sms( T_MNSMS_READ_CNF * mnsms_read_cnf)
{
/* Implements Measure#32: Row 1105 */

  TRACE_FUNCTION ("trace_read_sms ()");

  TRACE_EVENT_P1(" rec_num:  %u", mnsms_read_cnf->rec_num);
  TRACE_EVENT_P1(" rec_next: %u", mnsms_read_cnf->rec_next);
  TRACE_EVENT_P1(" rec_max:  %u", mnsms_read_cnf->rec_max);
  TRACE_EVENT_P1(" status    %u", mnsms_read_cnf->status);
  TRACE_EVENT_P1(" cause     0x%04x", mnsms_read_cnf->cause);
/* Implements Measure#32: Row 1105 */
  cmhSMS_sdu_buf_print(&(mnsms_read_cnf->sms_sdu.buf[0]), 0);
}
#endif



GLOBAL SHORT cmhSMS_SMRead ( T_MNSMS_READ_CNF * mnsms_read_cnf)
{
  T_ACI_AT_CMD  cmdBuf;       /* buffers current command          */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner            */
  T_ACI_CMGL_SM *p_sm=NULL;
  T_rp_addr rp_addr;
  UBYTE* message;
  T_TP_DELIVER *sms_deliver, decoded_mesg;
  BOOL  retCode = FALSE;
  UBYTE  msg_type;

  TRACE_FUNCTION ("cmhSMS_SMRead ()");

#ifndef NTRACE
  trace_read_sms(mnsms_read_cnf);
#endif

  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  ownBuf = smsShrdPrm.smsEntStat.entOwn;

  /* (SIM) error occurred */
  if (!IS_CAUSE_INVALID(mnsms_read_cnf->cause))
  {
    if (mnsms_read_cnf->cause NEQ SMS_NO_ERROR)
    {
      /* reset the command context */
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
      
      if( ownBuf NEQ CMD_SRC_LCL OR cmdBuf NEQ AT_CMD_P_CMGMDU )
      {
        R_AT ( RAT_CMS, ownBuf )
          (
            cmdBuf,
            cmhSMS_GetCmsFromSms(mnsms_read_cnf->cause),
            NULL
          );
      }
      return 1;
    }
  }

#ifdef _CONC_TESTING_
  /* this is only for testing */
  SET_OWNBUF_CONC;
#endif

  /*
   *-----------------------------------------------------------------
   * ignore primitives with incorrect message status
   *-----------------------------------------------------------------
   */
  TRACE_EVENT_P1("mnsms_read_cnf->status:  %d", mnsms_read_cnf->status);

  if ( (mnsms_read_cnf->status EQ SMS_RECORD_FREE) OR
       (mnsms_read_cnf->status EQ SMS_RECORD_INVALID) )
  {
    /* reset the command context */
    smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
    smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
    smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

    TRACE_EVENT("incorrect message status");
    return 1;
  }

  /* always decode for local source */
  if (ownBuf EQ  CMD_SRC_LCL)                                    /* ACI-SPR-9528 */
  {                                                              /* pass dummy p_sm to CB */
    if (((cmdBuf EQ AT_CMD_CMGL OR cmdBuf EQ AT_CMD_CMGR) OR 
          (cmdBuf EQ AT_CMD_P_CMGR OR cmdBuf EQ AT_CMD_P_CMGL))
          AND smsShrdPrm.rdMode NEQ READ_STATUS_CHANGE)                /* ACI-FIX-9510 */
    {
      retCode = cmhSMS_SMSQueryType (&mnsms_read_cnf->sms_sdu, &msg_type);
      if (retCode EQ FALSE)
      {
        /* reset the command context */
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
        smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
        smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

        TRACE_EVENT("Error decoding message type");
        return 1;
      }
      ACI_MALLOC(smsShrdPrm.pDecMsg, sizeof(T_ACI_CMGL_SM) );
      memset(smsShrdPrm.pDecMsg,0,sizeof(T_ACI_CMGL_SM) );
      p_sm = smsShrdPrm.pDecMsg;
      if (msg_type EQ TP_MTI_SMS_STATUS_REP)
      {
        retCode=cmhSMS_cpySTtoSM(p_sm ,mnsms_read_cnf);
      }
      else
      {
        /* Implements Measure # 110 */
        retCode = cmhSMS_cpyMsgIndReadCnf (p_sm, 
                                           &mnsms_read_cnf->status, 
                                           &mnsms_read_cnf->sms_sdu,
                                           mnsms_read_cnf->rec_num);
      }
      if (retCode EQ FALSE)
      {
        TRACE_EVENT("cmhSMS_SMRead ():[ERR] decoding of SMS");
        cmhSMS_invalidSMS_notification_to_user (mnsms_read_cnf);
      
        ACI_MFREE(smsShrdPrm.pDecMsg);

        /* reset the command context */
//Added by Jinshu Wang, 23/05/2005 OMAPS00108199
//start
	if (smsShrdPrm.smsEntStat.curCmd!=AT_CMD_CMGL)
	{
          smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
          smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
          smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

          smsShrdPrm.pDecMsg = NULL;
	}
//end	
        if (mnsms_read_cnf->rec_next EQ SMS_RECORD_NOT_EXIST)
        {
          smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
          smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
          smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
          if( cmdBuf NEQ AT_CMD_P_CMGMDU )
          {
            R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
          }
        }    
        return 1;
      }
    }
  }
  else
    p_sm = smsShrdPrm.pDecMsg;

  /*
   *-----------------------------------------------------------------
   * check for command context
   *-----------------------------------------------------------------
   */
  switch( smsShrdPrm.smsEntStat.curCmd )
  {
    case ( AT_CMD_CMGR ):
    case ( AT_CMD_P_CMGR ):
    {
      /*
       *-------------------------------------------------------------
       * process event for +CGMR
       *-------------------------------------------------------------
       */

      /* reset the command context */
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

      /* The message need not be displayed for Status Change */
      if (cmdBuf EQ AT_CMD_P_CMGR 
            AND smsShrdPrm.rdMode EQ READ_STATUS_CHANGE)
      {
        R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
        /* inform BMI */
        cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_P_CMGR, mnsms_read_cnf->rec_num, 
                               BS_SPEED_NotPresent,CME_ERR_NotPresent );
        break;
      }


#if defined MFW OR defined FF_MMI_RIV OR defined _CONC_TESTING_
      if (ownBuf            EQ CMD_SRC_LCL) /* AND        ACI-FIX-9510 */
       /* smsShrdPrm.rdMode NEQ READ_STATUS_CHANGE) */   /* call rplyCB (with invalid p_sm) anyway
                                                            to touch all segments while in
                                                            READ_STATUS_CHANGE */
      {
#if defined FF_MMI_RIV
        T_ACI_CMGF_MOD sms_input_mode = CMGF_MOD_NotPresent;
        qAT_PlusCMGF (CMD_SRC_LCL, &sms_input_mode);
        if (sms_input_mode EQ CMGF_MOD_Pdu )
        {
          if (cmdBuf EQ AT_CMD_CMGR )
          {
            rAT_PlusCMGRPdu (mnsms_read_cnf);
          }
          else if (cmdBuf EQ AT_CMD_P_CMGR )
          {
            rAT_PercentCMGRPdu (mnsms_read_cnf);
          }
        }
        else
        {
          if (smsShrdPrm.rplyCB.cmgr NEQ NULL)
          {
            smsShrdPrm.rplyCB.cmgr( p_sm, NULL );
          }
        }
        R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
#else
        /* GPF-MMI */
        if (smsShrdPrm.rplyCB.cmgr NEQ NULL)
        {
          smsShrdPrm.rplyCB.cmgr( p_sm, NULL );
        }
        if (!ISSET_CONC)
        {
          R_AT ( RAT_OK, ownBuf ) ( cmdBuf );
        }
#endif
      }
      else /* if (ownBuf EQ CMD_SRC_LCL) */
#endif /*#if defined MFW OR defined FF_MMI_RIV*/
      {
        if (cmdBuf EQ AT_CMD_CMGR )
        {
          R_AT ( RAT_CMGR, ownBuf ) ( mnsms_read_cnf, NULL );
        }
        else if (cmdBuf EQ AT_CMD_P_CMGR )
        {
          R_AT ( RAT_P_CMGR, ownBuf ) ( mnsms_read_cnf, NULL );
        }

        R_AT ( RAT_OK,   ownBuf ) ( cmdBuf );
        /* inform BMI */
        cmh_logRslt ( ownBuf, RAT_OK, AT_CMD_CMGR, mnsms_read_cnf->rec_num, 
                               BS_SPEED_NotPresent,CME_ERR_NotPresent  );
      }

      break;
    }

    case ( AT_CMD_P_CMGL ):
    case ( AT_CMD_CMGL ):
    {
      /*
       *-------------------------------------------------------------
       * process event for +CMGL
       *-------------------------------------------------------------
       */
      cmhSMS_cpyLstData ( cmdBuf, ownBuf, mnsms_read_cnf );

      break;
    }

    case ( AT_CMD_P_CMGMDU ):
    {
      /*
       *-------------------------------------------------------------
       * process event for %CMGMDU
       *-------------------------------------------------------------
       */
      message = cmhSMS_decodeMsg(&mnsms_read_cnf->sms_sdu, &rp_addr, SMS_VT_DELIVER);
      sms_deliver = (T_TP_DELIVER*)message;
      sms_deliver->tp_rp = 0;  
      memcpy(&decoded_mesg, sms_deliver, sizeof(T_TP_DELIVER));
      
      {
        PALLOC (mnsms_store_req, MNSMS_STORE_REQ);
      
        mnsms_store_req -> mem_type  = smsShrdPrm.mem1;
        mnsms_store_req -> condx     = SMS_CONDX_OVR_ANY;
        mnsms_store_req -> rec_num   = mnsms_read_cnf->rec_num;
        mnsms_store_req -> status    = mnsms_read_cnf->status;

        cmhSMS_codeMsg ( &mnsms_store_req->sms_sdu,
                         SMS_VT_DELIVER,
                         &rp_addr,
                         SMS_DELIVER,
                         (UBYTE*) &decoded_mesg);

        PSENDX (SMS, mnsms_store_req);
      }
      break;
    }
#if defined (SIM_TOOLKIT)
    case ( AT_CMD_CMSS ):
    {
      /*
       *-------------------------------------------------------------
       * process event for +CMSS (only for SAT)
       *-------------------------------------------------------------
       */
      cmhSMS_SMReadCMSS (mnsms_read_cnf);

      break;
    }
#endif

    default:
    {
      TRACE_EVENT("error in command context");
    }
  }

  if (smsShrdPrm.pDecMsg)
  {
    ACI_MFREE(smsShrdPrm.pDecMsg);
    smsShrdPrm.pDecMsg = NULL;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_Result                |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            a successfull transaction concerning the short message
            service.
*/
GLOBAL SHORT cmhSMS_Result ( T_MNSMS_REPORT_IND * mnsms_report_ind )
{
  T_ACI_AT_CMD  cmdBuf; /* buffers current command */
  T_ACI_CMD_SRC ownBuf; /* buffers current owner   */


  TRACE_FUNCTION ("cmhSMS_Result ()");

  if (mnsms_report_ind->v_cmms_mode)
  {
    smsShrdPrm.CMMSmode = mnsms_report_ind->cmms_mode;
    return 0;
  }
  
  cmdBuf            = AT_CMD_NONE;
  ownBuf            = CMD_SRC_LCL;

  smsShrdPrm.smsStat = mnsms_report_ind->state;


  if (mnsms_report_ind->state EQ SMS_STATE_READY)
  {
    /* reading of SMs from SIM is completed,
     * read SMSP, CBMIR, CMBI fields now
     */
    if (cmhSMS_ReadParams ((T_ACI_CMD_SRC)OWN_SRC_INV, AT_CMD_CFUN, 1) NEQ AT_EXCT)
    {
      cmhSMS_ready ();
    }
    R_AT ( RAT_SIG_SMS, ownBuf ) ( SMS_STATE_READY );
  }
  else if (mnsms_report_ind->state EQ SMS_STATE_INITIALISING)
  {
    {     /*ACI-FIX-18055 resetting total and used storage counters of SIM and ME storages which will be updated by cmhSMS_SMSInitState */
      smsShrdPrm.aci_sms_parameter.simTotal = 0;
      smsShrdPrm.aci_sms_parameter.simUsed  = 0;
      smsShrdPrm.aci_sms_parameter.meTotal  = 0;
      smsShrdPrm.aci_sms_parameter.meUsed   = 0;
    } /*ACI-FIX-18055 Changes end here */
    R_AT ( RAT_SIG_SMS, ownBuf ) ( SMS_STATE_INITIALISING );
  }
  else if (mnsms_report_ind->state > SMS_STATE_INITIALISING)
  {
    R_AT ( RAT_CMS, ownBuf ) ( cmdBuf, CMS_ERR_UnknownErr, NULL );
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_WrCnfCSAS             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            the usage of SIM and ME memory.
*/
GLOBAL void cmhSMS_WrCnfCSAS (SHORT aId)
{
  T_ACI_AT_CMD  cmdBuf;       /* buffers current command     */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner       */
  USHORT errCode;
  USHORT dataField = 0xFF;
  UBYTE  dataLen = 0;
  UBYTE  data[MAX_SIM_CMD];

  TRACE_FUNCTION ("cmhSMS_WrCnfCSAS ()");

  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  ownBuf = smsShrdPrm.smsEntStat.entOwn;

  /*
   *-----------------------------------------------------------------
   * check for next SIM access
   *-----------------------------------------------------------------
   */
  switch (smsShrdPrm.prmRdSeq)
  {
  case SMS_READ_SIM_SMSP:       /* EF(SMSP) written */
    errCode = simShrdPrm.atb[aId].errCode;

    if (smsShrdPrm.cbmPrm.cbmSIMmaxIdRge > 0 AND
        cmhSMS_PutCbmirSIM (ownBuf, data,
                            (int)smsShrdPrm.cbmPrm.cbmSIMmaxIdRge * 4)) /* write CBMIR */
    {
      dataField = SIM_CBMIR;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxIdRge * 4;
      break;
    }
    else
      smsShrdPrm.prmRdSeq--;

    if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0 AND
        cmhSMS_PutCbmiSIM (ownBuf, data,
                           (int)smsShrdPrm.cbmPrm.cbmSIMmaxId * 2))   /* write CBMI */
    {
      dataField = SIM_CBMI;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxId * 2;
    }
    else     /* nothing else to write */
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;
    }
    break;

  case SMS_READ_SIM_CBMIR:           /* CBMIR written */
    errCode = simShrdPrm.atb[aId].errCode;

    if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0 AND    /* write CBMI */
        cmhSMS_PutCbmiSIM (ownBuf, data,
                           (int)smsShrdPrm.cbmPrm.cbmSIMmaxId * 2))
    {
      dataField = SIM_CBMI;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxId * 2;
    }
    else           /* nothing else to write */
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;
    break;

  case SMS_READ_SIM_CBMI:  /* CBMI written */
    errCode = simShrdPrm.atb[aId].errCode;

    smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL; /* nothing else to write */
    break;

  default:      /* nothing else to write */
    smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;
    errCode = SIM_CAUSE_OTHER_ERROR;
    break;
  }
  if ( smsShrdPrm.prmRdSeq EQ SMS_READ_SIM_CMPL)
  {
    /* Implements Measure # 104 */
    cmhSMS_ResCSAS(ownBuf, cmdBuf, errCode);
  }
  else
  {
    if (cmhSIM_WriteTranspEF (CMD_SRC_NONE, cmdBuf, FALSE, NULL,
                              dataField, 0, dataLen,
                              data, cmhSMS_WrCnfCSAS) EQ AT_FAIL)
    {
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;

      R_AT( RAT_CMS, ownBuf ) ( cmdBuf, CMS_ERR_UnknownErr, NULL );
    }
    else
      smsShrdPrm.prmRdSeq--;
  }

  simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_RdCnfCRES             |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to notify the command handler of
            the usage of SIM and ME memory.
*/
GLOBAL void cmhSMS_RdCnfCRES (SHORT aId)
{
  T_ACI_AT_CMD  cmdBuf;       /* buffers current command     */
  T_ACI_CMD_SRC ownBuf;       /* buffers current owner       */

  USHORT errCode;
  USHORT dataField = 0xFF;
  UBYTE  dataLen = 0;
  TRACE_FUNCTION ("cmhSMS_RdCnfCRES ()");

  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  ownBuf = smsShrdPrm.smsEntStat.entOwn;

  /*
   *-----------------------------------------------------------------
   * check for next SIM access
   *-----------------------------------------------------------------
   */
  switch (smsShrdPrm.prmRdSeq)
  {
  case SMS_READ_SIM_SMSP:
    if ((errCode = simShrdPrm.atb[aId].errCode) EQ SIM_NO_ERROR)
    {
      /* successful EF(SMSP) read */

      
      if (!cmhSMS_GetPrmSIM (ownBuf,
                             simShrdPrm.atb[aId].exchData,
                             simShrdPrm.atb[aId].dataLen))
      {
        errCode = SIM_CAUSE_OTHER_ERROR;
      }
    }
    if (smsShrdPrm.cbmPrm.cbmSIMmaxIdRge > 0)
    {
      dataField = SIM_CBMIR;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxIdRge * 4;
      break;
    }
    else
      smsShrdPrm.prmRdSeq--;

    if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0)
    {
      dataField = SIM_CBMI;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxId * 2;
      break;
    }
    else
      smsShrdPrm.prmRdSeq--;

    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)ownBuf;
    psaMMI_Cbch();
    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

#ifdef SIM_TOOLKIT
    smsShrdPrm.prmRdSeq--; /* when SAT is on, then SMS_READ_CBMID is defined
                            which is irrelevant for +CRES */
#endif /* of SIM_TOOLKIT */
    
      break;

  case SMS_READ_SIM_CBMIR:
    if ((errCode = simShrdPrm.atb[aId].errCode) EQ SIM_NO_ERROR)
    {                          /* successful EF(CBMIR) read */
      if (!cmhSMS_GetCbmirSIM (ownBuf,
                               simShrdPrm.atb[aId].exchData,
                               simShrdPrm.atb[aId].dataLen))
      {
        errCode = SIM_CAUSE_OTHER_ERROR;
      }
    }
    if (smsShrdPrm.cbmPrm.cbmSIMmaxId > 0)
    {
      dataField = SIM_CBMI;
      dataLen = smsShrdPrm.cbmPrm.cbmSIMmaxId * 2;
    }
    else
    {
      smsShrdPrm.prmRdSeq--;

      smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)ownBuf;
      psaMMI_Cbch();
      smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

#ifdef SIM_TOOLKIT
      smsShrdPrm.prmRdSeq--; /* when SAT is on, then SMS_READ_CBMID is defined
                            which is irrelevant for +CRES */
#endif /* of SIM_TOOLKIT */

    }
    break;

  case SMS_READ_SIM_CBMI:
    if ((errCode = simShrdPrm.atb[aId].errCode) EQ SIM_NO_ERROR)
    {                          /* successful EF(CBMI) read */
      if (!cmhSMS_GetCbmiSIM (ownBuf,
                              simShrdPrm.atb[aId].exchData,
                              simShrdPrm.atb[aId].dataLen))
      {
        errCode = SIM_CAUSE_OTHER_ERROR;
      }
    }
    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)ownBuf;
    psaMMI_Cbch();
    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;

#ifdef SIM_TOOLKIT
    smsShrdPrm.prmRdSeq--; /* when SAT is on, then SMS_READ_CBMID is defined
                            which is irrelevant for +CRES */
#endif /* of SIM_TOOLKIT */

    break;

  default:
    smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;
    errCode = SIM_CAUSE_OTHER_ERROR;
    break;
  }
  if (--smsShrdPrm.prmRdSeq <= SMS_READ_SIM_CMPL)
  {

    /* Implements Measure # 104 */
    cmhSMS_ResCSAS(ownBuf, cmdBuf, errCode);
  }
  else
 if (cmhSIM_ReadTranspEF (CMD_SRC_NONE, cmdBuf, FALSE, NULL,
                             dataField, 0, dataLen,
                             NULL, cmhSMS_RdCnfCRES) EQ AT_FAIL)

    {
      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;

#ifdef SIM_TOOLKIT
      if (ownBuf EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT )/*smsShrdPrm.fuRef >= 0*/)
      {
        psaSAT_FUConfirm ((int)simShrdPrm.fuRef, SIM_FU_ERROR);
      }
      else
      {
#endif
        R_AT( RAT_CMS, ownBuf ) ( cmdBuf, CMS_ERR_UnknownErr, NULL );
#ifdef SIM_TOOLKIT
      }
#endif
    }

  simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_InitSMSP              |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to store the parameters of EF(SMSP).
*/
GLOBAL void cmhSMS_InitSMSP (SHORT aId)
{
  T_ACI_CMD_SRC srcId;
  T_SMS_SET_PRM * elem      ; /* points to SMS parameter set    */
  UBYTE          idx;
  UBYTE          profile;

  TRACE_FUNCTION ("cmhSMS_InitSMSP ()");

  /*
   *-----------------------------------------------------------------
   * process SIM result
   *-----------------------------------------------------------------
   */
  srcId = (T_ACI_CMD_SRC)smsShrdPrm.owner;

  switch (smsShrdPrm.prmRdSeq)
  {
  case SMS_READ_SIM_SMSP:
    if (simShrdPrm.atb[aId].errCode EQ SIM_NO_ERROR)
    {                            /* successful EF(SMSP) read */
      smsShrdPrm.aci_sms_parameter.smsParamMaxRec = simShrdPrm.atb[aId].recMax;
      smsShrdPrm.aci_sms_parameter.smsParamRecLen = simShrdPrm.atb[aId].dataLen;

      if (set_prm_list EQ NULL)
      {
        set_prm_list = new_list();
      }
      profile = simShrdPrm.atb[aId].recNr;
      elem = find_element(set_prm_list, profile, cmhSMS_findPrflId);

      if (elem EQ NULL)
      {
        ACI_MALLOC(elem, sizeof(T_SMS_SET_PRM));
        memset(elem, 0, sizeof(T_SMS_SET_PRM));
        insert_list(set_prm_list, elem);
        elem->prflId = profile;
        elem->numOfRefs = OWN_SRC_MAX;
        for( idx = 0; idx < OWN_SRC_MAX; idx++ )
        {
          smsShrdPrm.pSetPrm[idx] = (T_SMS_SET_PRM*) elem;
        }

      }
      else
      {
        smsShrdPrm.pSetPrm[0] = (T_SMS_SET_PRM*) elem;
      }

      cmhSMS_GetPrmSIM (CMD_SRC_LCL,
                        simShrdPrm.atb[aId].exchData,
                        simShrdPrm.atb[aId].dataLen);
    }
    else
    {
      smsShrdPrm.aci_sms_parameter.smsParamMaxRec = 0;
      smsShrdPrm.aci_sms_parameter.smsParamRecLen = 0;
    }
    if( !psaSIM_ChkSIMSrvSup( SRV_CBMIdRnge ))
    {
      smsShrdPrm.prmRdSeq--;
      goto CBMIR;
    }
    if (cmhSIM_ReadTranspEF (CMD_SRC_NONE, smsShrdPrm.smsEntStat.curCmd,
                             FALSE, NULL, SIM_CBMIR, 0, 255,
                             NULL, cmhSMS_InitSMSP) NEQ AT_FAIL)
      smsShrdPrm.prmRdSeq--;
    else
    {
      goto CBMIR;
    }
    break;

  case SMS_READ_SIM_CBMIR:
    if (simShrdPrm.atb[aId].errCode EQ SIM_NO_ERROR)
    {
      smsShrdPrm.cbmPrm.cbmSIMmaxIdRge = simShrdPrm.atb[aId].dataLen / 4;
      cmhSMS_GetCbmirSIM (CMD_SRC_NONE,
                          simShrdPrm.atb[aId].exchData,
                          simShrdPrm.atb[aId].dataLen);
    }
    else
      smsShrdPrm.cbmPrm.cbmSIMmaxIdRge = 0;

  CBMIR:
    if( !psaSIM_ChkSIMSrvSup( SRV_CBM_Ident ))
    {
      smsShrdPrm.prmRdSeq--;
      goto CBMI;
    }
    if (cmhSIM_ReadTranspEF (CMD_SRC_NONE, smsShrdPrm.smsEntStat.curCmd,
                             FALSE, NULL, SIM_CBMI, 0, 255,
                             NULL, cmhSMS_InitSMSP) NEQ AT_FAIL)
      smsShrdPrm.prmRdSeq--;
    else
    {
#ifdef SIM_TOOLKIT
      goto CBMI;
#else
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;

      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      smsShrdPrm.smsEntStat.entOwn = simShrdPrm.owner = CMD_SRC_NONE;
#endif /* of SIM_TOOLKIT */
    }
    break;

  case SMS_READ_SIM_CBMI:
    if (simShrdPrm.atb[aId].errCode EQ SIM_NO_ERROR)
    {
      smsShrdPrm.cbmPrm.cbmSIMmaxId = simShrdPrm.atb[aId].dataLen / 2;
      cmhSMS_GetCbmiSIM (CMD_SRC_NONE,
                         simShrdPrm.atb[aId].exchData,
                         simShrdPrm.atb[aId].dataLen);
    }
    else
      smsShrdPrm.cbmPrm.cbmSIMmaxId = 0;

#ifdef SIM_TOOLKIT
  CBMI:
    if( (!psaSIM_ChkSIMSrvSup( SRV_DtaDownlCB )) OR 
            smsShrdPrm.owner NEQ OWN_SRC_SAT)
    {
      smsShrdPrm.prmRdSeq--;
      goto CBMID;
    }
    if (cmhSIM_ReadTranspEF (CMD_SRC_NONE, smsShrdPrm.smsEntStat.curCmd,
                             FALSE, NULL, SIM_CBMID, 0, 255,
                             NULL, cmhSMS_InitSMSP) NEQ AT_FAIL)
      smsShrdPrm.prmRdSeq--;
    else
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;

      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
      simShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
    }
    break;
    
  case SMS_READ_SIM_CBMID:
    if (simShrdPrm.atb[aId].errCode EQ SIM_NO_ERROR)
    {
    /* Implements Measure # 59 */
    cmhSMS_SendCbDwnlSIM (simShrdPrm.atb[aId].exchData, 
                          simShrdPrm.atb[aId].dataLen);
    }
    else
      smsShrdPrm.cbmPrm.cbmSIMmaxSATId = 0;


       /*lint -fallthrough */
  default:
  CBMID:

#else
       /*lint -fallthrough*/
  default:
  CBMI:
#endif /*SIM_TOOLKIT */

    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)srcId;
    psaMMI_Cbch();
    smsShrdPrm.cbmPrm.cbchOwner = (T_OWN)CMD_SRC_NONE;
    {
      smsShrdPrm.prmRdSeq = SMS_READ_SIM_CMPL;

      smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
       simShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
      smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
    }
#ifdef SIM_TOOLKIT
    if (srcId EQ ((T_ACI_CMD_SRC)OWN_SRC_SAT) /*smsShrdPrm.fuRef >= 0*/)
    {
      smsShrdPrm.accessEnabled = TRUE;
      psaSAT_FUConfirm (simShrdPrm.fuRef,(USHORT)
            ((simShrdPrm.atb[aId].errCode EQ SIM_NO_ERROR)?
            SIM_FU_SUCC_ADD: SIM_FU_ERROR));
    }
    else
#endif
      cmhSMS_ready ();
    break;
  }

  simShrdPrm.atb[aId].ntryUsdFlg = FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSResumeCnf          |
+-------------------------------------------------------------------+

  PURPOSE : This function processes the MNSMS_RESUME_CNF primitive. 
  
*/
GLOBAL void cmhSMS_SMSResumeCnf (T_MNSMS_RESUME_CNF * mnsms_resume_cnf)
{
  UBYTE cmdBuf;
  UBYTE ownBuf;

  TRACE_FUNCTION("cmhSMS_ResumeCnf()");
  
  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  ownBuf = smsShrdPrm.smsEntStat.entOwn;

  /* check whether resume has been successfully processed */ 
  if (mnsms_resume_cnf->cause EQ SMS_NO_ERROR) 
  {
    R_AT( RAT_OK, (T_ACI_CMD_SRC)ownBuf ) ( cmdBuf );
  }
  else 
  {
    /* command unsuccessful */
    R_AT ( RAT_CMS, (T_ACI_CMD_SRC)ownBuf )
    (
      cmdBuf,
      cmhSMS_GetCmsFromSms(mnsms_resume_cnf->cause),
      NULL
    );
  }
  
  /* reset command */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
  smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSQueryCnf           |
+-------------------------------------------------------------------+

  PURPOSE : This function processes the MNSMS_QUERY_CNF primitive. 
  
*/
GLOBAL void cmhSMS_SMSQueryCnf (T_MNSMS_QUERY_CNF * mnsms_query_cnf)
{
  UBYTE cmdBuf;
  UBYTE ownBuf;

  TRACE_FUNCTION("cmhSMS_SMSQueryCnf()");
  
  cmdBuf = smsShrdPrm.smsEntStat.curCmd;
  ownBuf = smsShrdPrm.smsEntStat.entOwn;

  /* check type of query request and process it */
  switch (mnsms_query_cnf->query_type)
  {
    case SMS_QUERY_DELIVER_STATUS:
    {
      if ( mnsms_query_cnf->v_deliver_status )
      {
#ifdef TI_PS_FF_AT_P_CMD_CPRSM
        R_AT ( RAT_CPRSM, (T_ACI_CMD_SRC)ownBuf )
        ( cmhSMS_convertDeliverStatusToACI(mnsms_query_cnf->deliver_status ) );
#endif /* TI_PS_FF_AT_P_CMD_CPRSM */
        R_AT( RAT_OK, (T_ACI_CMD_SRC)ownBuf ) ( cmdBuf );
      }
      else /* paramter inalid - unspecified protocol error */ 
      {
        TRACE_EVENT("cmhSMS_QueryCnf: ERROR: paramter invalid");
        R_AT ( RAT_CMS, (T_ACI_CMD_SRC)ownBuf )( cmdBuf, CMS_ERR_InValManInfo, NULL );    
      }
      break;
    }
    default: /* unsupported query type - ME failure */
    {
      TRACE_EVENT("cmhSMS_QueryCnf: ERROR: unsupported query type");
      R_AT ( RAT_CMS, (T_ACI_CMD_SRC)ownBuf )( cmdBuf, CMS_ERR_MeFail, NULL );
    }
  } /* end switch */

  /* reset command */
  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
  smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_ReadCbDtaDwnl         |
+-------------------------------------------------------------------+

  PURPOSE : This function processes the T_SIM_MMI_INSERT_IND primitive. 
  
*/
GLOBAL void cmhSMS_ReadCbDtaDwnl (T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind)
{

  TRACE_FUNCTION("cmhSMS_ReadCbDtaDwnl()");

  if (sim_mmi_insert_ind->cbmid_rec.c_rec NEQ 0)
  {
    /* Implements Measure # 59 */
    cmhSMS_SendCbDwnlSIM (sim_mmi_insert_ind->cbmid_rec.rec, 
                          sim_mmi_insert_ind->cbmid_rec.c_rec);
  }
  else
  {
    TRACE_EVENT(" CBMID records zero"); 
    smsShrdPrm.cbmPrm.cbmSIMmaxSATId = 0;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SMSQueryType          |
+-------------------------------------------------------------------+

  PURPOSE : This function queries the message type of the T_MNSMS_MESSAGE_IND. 
  
*/
GLOBAL BOOL cmhSMS_SMSQueryType (T_sms_sdu *sms_sdu,
                                UBYTE *msg_type)
{
  UBYTE ccdRet;

  TRACE_FUNCTION("cmhSMS_SMSQueryType()");

   if (sms_sdu->l_buf EQ 0)
   {
     TRACE_EVENT("empty SDU: no decoding");
     return FALSE;
   }

  CCD_START;
  {

    MCAST( sim_pdu, SIM_PDU ); /* sim_pdu points to _decodedMsg */

    /* decoding outer layer */
    ccdRet = ccd_decodeMsg ( CCDENT_SMS,
                             BOTH /* doesn't work with DOWNLINK!!! */,
                             (T_MSGBUF *) sms_sdu,
                             (UBYTE    *) _decodedMsg, /* target */
                             SMS_VT_SIM_PDU);

    if ((ccdRet NEQ ccdOK) OR (!sim_pdu->v_tpdu)
         OR (_decodedMsg[0] NEQ SMS_VT_SIM_PDU))
    {
      TRACE_EVENT_P1("CCD Decoding Error: %d", ccdRet);
      CCD_END;
      return FALSE;
    }
    *msg_type = sim_pdu->tp_mti;
  }
  CCD_END;

  return TRUE;
  
}

LOCAL BOOL cmhSMS_cpySTtoSM(T_ACI_CMGL_SM *p_sm,T_MNSMS_READ_CNF * mnsms_read_cnf)
{
  T_ACI_CDS_SM   p_st;
  T_MNSMS_STATUS_IND mnsms_status_ind;
  BOOL          retCode = FALSE;

  memcpy (&mnsms_status_ind.sms_sdu, &mnsms_read_cnf->sms_sdu, 
   sizeof (T_sms_sdu));
  retCode = cmhSMS_cpyStatInd ( &p_st, &mnsms_status_ind);
  memcpy(p_sm->adress, p_st.addr, MAX_SMS_ADDR_DIG - 1 );
  cmhSMS_getStatCmh ( mnsms_read_cnf->status, &p_sm->stat );
  p_sm->msg_ref = mnsms_read_cnf->rec_num ;
  p_sm->tp_status=p_st.tp_status ;
  p_sm->toa .ton = p_st.toa .ton ;
  p_sm->toa.npi = p_st.toa.npi;
  p_sm->fo = p_st.fo;
  p_sm->scts =p_st.vpabs_scts ; 

  return (retCode);
}
/* Implements Measure # 59 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_SendCbDwnlSIM          |
+-------------------------------------------------------------------+
  PARAMETERS  : data - CBMID information
                data_len - Number of bytes in 'data' parameter 
  RETURN      : None

  PURPOSE : This function gets the CBMID information and sends to SIM
  
*/

LOCAL void cmhSMS_SendCbDwnlSIM(UBYTE *data, int data_len)
{
  TRACE_FUNCTION("cmhSMS_SendCbDwnlSIM()");
  smsShrdPrm.cbmPrm.cbmSIMmaxSATId = data_len / 2;
  cmhSMS_GetCbDtaDwnlSIM (CMD_SRC_NONE,
                          data,
                          data_len);
  {
    PALLOC (sat_cbch_req, MMI_SAT_CBCH_DWNLD_REQ);

    sat_cbch_req->count = (USHORT)smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds;
    memset (sat_cbch_req->msg_id, NOT_PRESENT_16BIT,
            sizeof (sat_cbch_req->msg_id));
    memcpy (sat_cbch_req->msg_id, smsShrdPrm.cbmPrm.CBDtaDwnlIdent,
            (size_t)smsShrdPrm.cbmPrm.CBDtaDwnlFoundIds * 2);

    PSENDX (PL, sat_cbch_req);
  }
}

/* Implements Measure # 104 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_ResCSAS               |
+-------------------------------------------------------------------+
  PARAMETERS  : ownBuf - ACI command source
                cmdBuf - ACI Command type
                errCode - Error Code from SIM  
  RETURN      : None

  PURPOSE : This function sends response to CSAS
  
*/
LOCAL void cmhSMS_ResCSAS(T_ACI_CMD_SRC ownBuf, T_ACI_AT_CMD  cmdBuf, USHORT errCode)
{
  TRACE_FUNCTION("cmhSMS_ResCSAS()");

  smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
  smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
  smsShrdPrm.smsEntStat.entOwn = CMD_SRC_NONE;

  if (errCode EQ SIM_NO_ERROR)
  {
    R_AT( RAT_OK, ownBuf ) ( cmdBuf );
  }
  else
  {
    R_AT( RAT_CMS, ownBuf )  /* unsuccessful SIM write */
      (
        cmdBuf,
        cmhSMS_GetCmsFromSim ( errCode ),
        NULL
      );
  }
}

/* Implements Measure # 114 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSR                     |
| STATE   : code             ROUTINE : cmhSMS_ErrRes                |
+-------------------------------------------------------------------+
  PARAMETERS  : ownBuf - ACI command source
                cmdBuf - ACI Command type
                cause - Error cause that has come from SMS entity
  RETURN      : None

  PURPOSE : This function sends error response.
  
*/
LOCAL void cmhSMS_ErrRes(T_ACI_CMD_SRC ownBuf, T_ACI_AT_CMD  cmdBuf, USHORT cause)
{
  TRACE_FUNCTION("cmhSMS_ErrRes()");

  if (ownBuf EQ CMD_SRC_LCL)
  {
    if (smsShrdPrm.errorCB NEQ NULL)
    {
      smsShrdPrm.errorCB
      ( 
        cmdBuf,
        cmhSMS_GetCmsFromSms(cause),
        NULL
      );
    }
  }
  else
  {
    R_AT ( RAT_CMS, ownBuf )
    (
      cmdBuf,
      cmhSMS_GetCmsFromSms(cause),
      NULL
    );
  }
}
/*==== EOF ========================================================*/

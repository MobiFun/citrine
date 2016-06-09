/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SMSP
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
|  Purpose :  This module defines the processing functions for the
|             primitives send to the protocol stack adapter by 
|             short message service. 
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SMSP_C
#define PSA_SMSP_C
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

#include "aci.h"
#include "psa.h"
#include "psa_sms.h"
#include "cmh.h"
#include "cmh_sms.h"
#include "aoc.h"
#ifdef SIM_PERS
#include "general.h"  // inluded for UINT8 compilation error in sec_drv.h
#include "sec_drv.h"
#include "aci_ext_pers.h"
#include "aci_slock.h"
#endif

#ifdef GPRS
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa_sm.h"

#include "cmh_sm.h"
#endif  /* GPRS */
/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/
#ifndef PIN_LEN
#define PIN_LEN (8)
#endif
#ifdef SIM_PERS_OTA
#define CTRL_KEY_LEN (PIN_LEN+1)
#define SMS_DATA_IDX 41
#define IMEI_DATA_IDX 73

EXTERN T_SEC_DRV_CONFIGURATION *cfg_data ;
#endif

/*==== FUNCTIONS ==================================================*/
#ifdef SIM_PERS_OTA
void psa_mnsms_OTA_decode_ind ( T_MNSMS_OTA_MESSAGE_IND * ); 
#endif
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_message_ind   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_MESSAGE_IND primitive send by SMS.
            this indicates an incoming SMS.

*/

GLOBAL void psa_mnsms_message_ind 
                               ( T_MNSMS_MESSAGE_IND *mnsms_message_ind )
{
  /* (was psa_mnsms_alert_ind) */
  TRACE_FUNCTION ("psa_mnsms_message_ind()");


  if (smsShrdPrm.smsStat EQ SMS_STATE_INITIALISING)
  {
    TRACE_FUNCTION ("SMS_STATE_INITIALISING");
    cmhSMS_SMSInitState(mnsms_message_ind);
  }
  else
  {
#ifdef FF_CPHS_REL4
    if(!cmhSMS_chk_SpclMsg( mnsms_message_ind ))
    {
#endif
      if (mnsms_message_ind->rec_num EQ SMS_RECORD_NOT_EXIST)
        {
#ifdef FF_CPHS      
        if ( !cmhSMS_voice_mail_ind( &(mnsms_message_ind->sms_sdu)) )
#endif /* FF_CPHS */      
          cmhSMS_SMSDeliver (mnsms_message_ind);
      }
      else
      {
      /*
       *  Incoming SMS may modify the Advice of Charge Parameters
       *  
       *  Read them again
       */
        aoc_sms ();

#ifdef FF_CPHS      
        cmhSMS_voice_mail_ind( &(mnsms_message_ind->sms_sdu));
#endif /* FF_CPHS */    
        cmhSMS_SMSMemory (mnsms_message_ind);
      }
#ifdef FF_CPHS_REL4
    }
#endif
  }
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_message_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_report_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_REPORT_IND primitive send by SMS.
            this indicates the state of the SMS Entity.

*/

GLOBAL void psa_mnsms_report_ind 
                             ( T_MNSMS_REPORT_IND *mnsms_report_ind )
{

  TRACE_FUNCTION ("psa_mnsms_report_ind()");

  cmhSMS_Result(mnsms_report_ind);

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_report_ind);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_status_ind    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_STATUS_IND primitive send by SMS.
            this indicates an incoming SMS status message.

*/

GLOBAL void psa_mnsms_status_ind ( T_MNSMS_STATUS_IND *mnsms_status_ind )
{
  TRACE_FUNCTION ("psa_mnsms_status_ind()");

  cmhSMS_SMSStatRpt (mnsms_status_ind);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_status_ind);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_delete_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_DELETE_CNF primitive send by SMS.

*/

GLOBAL void psa_mnsms_delete_cnf ( T_MNSMS_DELETE_CNF *mnsms_delete_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_delete_cnf()");

  cmhSMS_SMSDelCnf (mnsms_delete_cnf);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_delete_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_read_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_READ_CNF primitive send by SMS.

*/

GLOBAL void psa_mnsms_read_cnf ( T_MNSMS_READ_CNF *mnsms_read_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_read_cnf()");

  cmhSMS_SMRead (mnsms_read_cnf);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_read_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_store_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_STORE_CNF primitive send by SMS.

*/

GLOBAL void psa_mnsms_store_cnf ( T_MNSMS_STORE_CNF *mnsms_store_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_store_cnf()");

  cmhSMS_SMSStoCnf(mnsms_store_cnf);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_store_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_submit_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_SUBMIT_CNF primitive send by SMS.

*/

GLOBAL void psa_mnsms_submit_cnf ( T_MNSMS_SUBMIT_CNF *mnsms_submit_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_submit_cnf()");

  cmhSMS_SMSSbmCnf(mnsms_submit_cnf);

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_submit_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_command_cnf   |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_COMMAND_CNF primitive send by SMS.

*/

GLOBAL void psa_mnsms_command_cnf ( T_MNSMS_COMMAND_CNF *mnsms_command_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_command_cnf()");

  cmhSMS_SMSCmdCnf(mnsms_command_cnf);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_command_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_error_ind     |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_ERROR_IND primitive send by SMS.

*/

GLOBAL void psa_mnsms_error_ind ( T_MNSMS_ERROR_IND *mnsms_error_ind )
{
  TRACE_FUNCTION ("psa_mnsms_error_ind()");

  cmhSMS_SMSErrorInd(mnsms_error_ind);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_error_ind);
}

#ifdef TI_PS_FF_AT_P_CMD_CPRSM
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_resume_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_RESUME_CNF primitive send by SMS.
            This confirms that the receiving of has been SMS is 
            activated.

*/

GLOBAL void psa_mnsms_resume_cnf ( T_MNSMS_RESUME_CNF *mnsms_resume_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_resume_cnf()");

  /* inform command handler */ 
  cmhSMS_SMSResumeCnf(mnsms_resume_cnf);
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE(mnsms_resume_cnf);  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_query_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_TEST_CNF primitive send by SMS.
            This confirms that the receiving of has been SMS is 
            activated.

*/

GLOBAL void psa_mnsms_query_cnf ( T_MNSMS_QUERY_CNF *mnsms_query_cnf)
{
  TRACE_FUNCTION ("psa_mnsms_query_cnf()");

  /* inform command handler */ 
  cmhSMS_SMSQueryCnf( mnsms_query_cnf );
  
  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE(mnsms_query_cnf);  
}
#endif /* TI_PS_FF_AT_P_CMD_RDLB */

#if defined (GPRS) AND defined (DTI)
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_info_cnf      |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_MO_SERV_CNF primitive send by SMS.
            this confirms the requested SMS message service.

*/

GLOBAL void psa_mnsms_mo_serv_cnf 
                            ( T_MNSMS_MO_SERV_CNF *mnsms_mo_serv_cnf )
{

  TRACE_FUNCTION ("psa_mnsms_mo_serv_cnf()");

  /*
   *-------------------------------------------------------------------
   * update sms parameters and notify ACI
   *-------------------------------------------------------------------
   */ 

  cmhSM_sms_service_changed( mnsms_mo_serv_cnf->mo_sms_serv );

/*
 *-------------------------------------------------------------------
 * free the primitive buffer
 *-------------------------------------------------------------------
 */  
  PFREE (mnsms_mo_serv_cnf);

}
#endif  /* GPRS */

GLOBAL void psa_mnsms_OTA_message_ind ( T_MNSMS_OTA_MESSAGE_IND *mnsms_ota_message_ind )
{
  switch (mnsms_ota_message_ind->tp_pid)
  {
   #ifdef SIM_PERS_OTA
       case ACI_PID_ME_DEPERSON :
               psa_mnsms_OTA_decode_ind(mnsms_ota_message_ind); 
       break; 
   #endif
    default: TRACE_ERROR ("INVALID PID");
        break ; 
  }
}

#ifdef SIM_PERS_OTA

/*
+------------------------------------------------------------------------------
|  Function    : psa_mnsms_OTA_decode_ind
+------------------------------------------------------------------------------
|  Description : processes the MNSMS_OTA_DECODE_IND primitive
|
|  Parameters  : mnsms_ota_decode_ind            - primitive
|                
|  Return      : void
|
+------------------------------------------------------------------------------
*/

 void psa_mnsms_OTA_decode_ind ( T_MNSMS_OTA_MESSAGE_IND *mnsms_ota_message_ind )
{
  T_SIMLOCK_TYPE lcktype;
  UBYTE ctrl_key[CTRL_KEY_LEN];
  UBYTE mismatch_key[CTRL_KEY_LEN] = {0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0}; 
  UBYTE imei[16];
  UBYTE imeiBufPtrd[CL_IMEI_SIZE], imeiBufPtrc[16];
  UBYTE data[20];
  T_ACI_SM_DATA sm_data; 
  UBYTE ret_status = 0;
  T_SIMLOCK_STATUS status; 
  UINT i = 0,i2=0, ctr;

  TRACE_FUNCTION ("psa_mnsms_OTA_decode_ind()"); 

   if((mnsms_ota_message_ind->tp_ud.data NEQ NULL) AND (mnsms_ota_message_ind->tp_ud.length NEQ 0))
   {
        cmhSMS_expdSmsPp ( 0,
                         mnsms_ota_message_ind->tp_dcs,
                         mnsms_ota_message_ind->tp_ud.data,
                         mnsms_ota_message_ind->tp_ud.length,
                         sm_data .data,
                         &sm_data.len );
    
    memcpy(imei , (sm_data.data+ IMEI_DATA_IDX-1),(CL_IMEI_SIZE*2));
  
    cl_get_imeisv(CL_IMEI_SIZE, imeiBufPtrd, CL_IMEI_GET_STORED_IMEI);
  
     for (i = 0, i2 = 0; i < CL_IMEI_SIZE; i++, i2+=2)
     {
         imeiBufPtrc[i2] = ((imeiBufPtrd[i] & 0xf0) >> 4) + '0' ; 
        imeiBufPtrc[i2+1] = (imeiBufPtrd[i] & 0x0f)  + '0' ;
     }
  /* Compare 14 digits since 15th and 16th digits are SVN which is not known to NW  (see TS 23.003 [3]) */
   if(memcmp(imei,imeiBufPtrc,(CL_IMEI_SIZE*2)-2 ) EQ 0)
    {
      /*Extract the values from the SMS user data*/
    
      memset(ctrl_key,0,CTRL_KEY_LEN);
      
        aci_ext_personalisation_init();
        
        for(lcktype=SIMLOCK_NETWORK,ctr=SMS_DATA_IDX-1; lcktype<=SIMLOCK_CORPORATE;lcktype++,ctr+=PIN_LEN)
        {
          strncpy ( (char *) ctrl_key, (char*)(sm_data.data +ctr), PIN_LEN);
          ctrl_key[PIN_LEN] = '\0';
          if (memcmp ((char *)ctrl_key,mismatch_key,PIN_LEN) EQ 0)
            ret_status |= aci_ext_personalisation_get_status(lcktype);
          else    
          {
            status =   aci_slock_unlock(lcktype, (char *) ctrl_key); 
            if( status EQ SIMLOCK_FAIL)
              ret_status |= aci_ext_personalisation_get_status(lcktype);
            else 
              ret_status |= status; 
          }
          if(lcktype NEQ SIMLOCK_CORPORATE)
            ret_status  <<= 2;
        }
       aci_ext_personalisation_free();
         
     }
   else 
       ret_status = 0xff; 
    
  
    for (i=0;i<(CL_IMEI_SIZE*2);i++)
    {
        data[i] = imei[i];      
     }
    data[15] = 0;  /*As the IMEI is 15 digits long (see TS 23.003 [3]), 
                    the sixteenth digit present here is ignored by the network during the 
                    de-personalization procedure and should be set to zero.*/
    data[16] = ((ret_status & 0xC0)>>6);
    data[17]= ((ret_status & 0x30) >>4);
    data[18]= ((ret_status & 0x0C) >> 2);
    data[19]= ret_status & 0x03;
     
    {
      PALLOC (mnsms_OTA_message_res, MNSMS_OTA_MESSAGE_RES);
      cmhSMS_rdcSmsPp ( 0,
                      mnsms_ota_message_ind->tp_dcs,
                      ( UBYTE * ) data,  20,
                      mnsms_OTA_message_res->data, &mnsms_OTA_message_res->packedlen);   //Convert the Octets to septets
      mnsms_OTA_message_res->tp_dcs = mnsms_ota_message_ind->tp_dcs ;
      mnsms_OTA_message_res->len = 20; 
#ifdef TI_PS_HCOMM_CHANGE
      PSEND (_hCommSMS, mnsms_OTA_message_res);
#else
      PSEND (hCommSMS, mnsms_OTA_message_res);
#endif
    }
  }
    PFREE (mnsms_ota_message_ind);
}
#endif //SIM_PERS_OTA


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_retrans_cnf    |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_RETRANS_CNF primitive send by SMS.

*/
#ifdef REL99
GLOBAL void psa_mnsms_retrans_cnf ( T_MNSMS_RETRANS_CNF *mnsms_retrans_cnf )
{
  TRACE_FUNCTION ("psa_mnsms_retrans_cnf()");

  cmhSMS_SMSRetransCnf(mnsms_retrans_cnf);

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_retrans_cnf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SMSP                |
|                                 ROUTINE : psa_mnsms_send_prog_ind |
+-------------------------------------------------------------------+

  PURPOSE : processes the MNSMS_SEND_PROG_IND primitive send by SMS.

*/

GLOBAL void psa_mnsms_send_prog_ind ( T_MNSMS_SEND_PROG_IND *mnsms_send_prog_ind )
{
  TRACE_FUNCTION ("psa_mnsms_send_prog_ind()");

  cmhSMS_SMSSendProgInd(mnsms_send_prog_ind);

  /*
   *-------------------------------------------------------------------
   * free the primitive buffer
   *-------------------------------------------------------------------
   */  
  PFREE (mnsms_send_prog_ind);
}
#endif /* REL99 */

/*==== EOF =========================================================*/

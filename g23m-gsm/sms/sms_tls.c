/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_TLP
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
|  Purpose :  This Modul defines the functions for the transfer layer
|             capability of the module Short Message Service.
+----------------------------------------------------------------------------- 
*/ 
 
#ifndef SMS_TLS_C
#define SMS_TLS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cus_sms.h"
#include "cnf_sms.h"
#include "mon_sms.h"
#include "pei.h"
#include "tok.h"
#include "sms.h"
#include "gdi.h"
#include "sms_em.h"

/*==== EXPORT ======================================================*/

/*==== PRIVAT ======================================================*/

/*==== VARIABLES ===================================================*/

/*==== FUNCTIONS ===================================================*/

LOCAL BOOL tl_modify_is_all (UBYTE modify)
{
  switch (modify)
  {
  case SMS_MODIFY_NON:
  case SMS_MODIFY_SCA:
  case SMS_MODIFY_TPOA:
  case SMS_MODIFY_TPOA_SCA:
    return FALSE;
  default:
    break;
  }
  return TRUE;  /* MODIFY_ALL: complete message is given */
}

LOCAL void tl_send_failure(USHORT cause)
{
  GET_INSTANCE_DATA;
           
  /*
   * release connection
   */
   rl_release_req(SMS_INST.ti);

  /* 
   * Send MNSMS_REPORT_IND indicating the cmms_mode value to ACI
   * if CMMS_MODE is equal to 1or 2.
   */
   if(CMMS_ACTIVE)
   {
      tl_cmms_end();
   }
  /*
   * SUBMIT_CNF
   */
   tl_mnsms_submit_cnf (SMS_SEL_MEM(sms_data),
                        SMS_SEL_REC(sms_data),
                        NOT_PRESENT_8BIT,
                        cause,
                        NULL);
}

/*---- SIGNALS -----------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_establish_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_ESTABLISH_CNF.

*/

GLOBAL void tl_establish_cnf (
                              BOOL          success)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_establish_cnf()");

#ifdef GPRS
  if (success EQ FALSE) 
  {
     /*
      * Establishment Failed.
      */ 
     if (SMS_INST.downlink EQ SMS_DOWNLINK_LL_CHECK)
     {
        if (sms_data->mo_dst_pref EQ GPRS_SMS_GPRS_PREF)
        {
           /*
             * using the preferred downlink LL failed, use CSD
             */
           TRACE_EVENT("Establishment failed on LL link, try gsm path instead");
           /* 
             * set downlink type MMSMS
             */
           TRACE_EVENT("downlink = SMS_DOWNLINK_MMSMS");
           SMS_INST.downlink = SMS_DOWNLINK_MMSMS;
           /*
             * try again
             */
           rl_establish_req(SMS_INST.ti);
           return;
        }
        else
        {
           /*
             * unsing the preferred downlink LL failed
             */
           TRACE_EVENT("Establishment failed on LL link, stop working");
           /*
             * handle event in report ind
             */
           tl_report_ind(NULL,SMS_CAUSE_NO_SERVICE);
           return;
        }
     }
     
     else if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
     {
       if(sms_data->mo_dst_pref EQ GPRS_SMS_CCT_PREF)
       {
          /*
            * using the preferred downlink MM failed, use PS
            */
          TRACE_EVENT("Establishment failed on MM link, try gprs path instead");

          /* 
            * set downlink type LL
            */
          TRACE_EVENT("downlink = SMS_DOWNLINK_LL");
          SMS_INST.downlink = SMS_DOWNLINK_LL_CHECK;
          /* 
           * If CMMS_ACTIVE then send the MNSMS_REPORT_IND to ACI informing about
           * the current cmms_mode value as GSM connection is failed.
           */
          if(CMMS_ACTIVE)
          {
            tl_cmms_end();
          }
        
          /*
            * try again
            */
          rl_establish_req(SMS_INST.ti);
          return;
       }
       /*
         * else
         *    <<PS PREF failed CS fallback also failed>> case covered in cp layer.
         */
     }
}
#endif /* #ifdef GPRS */

  switch (SMS_INST_GET_STATE (STATE_TL))
  {
  case TL_ESTABLISH:
     switch (GET_STATE (STATE_EST))
     {
     case EST_SEND:

        tl_establish_cnf_send();
        break;

     case EST_RTX:

        tl_establish_cnf_rtx();
        break;

     case EST_CMD:

        tl_establish_cnf_cmd();
        break;

     case EST_SMMA:

       /*
        * TL state transition: TL_OTHER
        */
        SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
        if (!rl_mem_avail_req ())
        {
          TRACE_EVENT("could not send SMMA");
         /*
          * TL  state transition TL_OTHER
          */
          SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
         /*
          * handle event in report ind
          */
          tl_report_ind(NULL,SMS_CAUSE_ENTITY_BUSY);
        }

        break;
     }
     break;

  default:
    TRACE_ERROR("TL_ESTABLISH_CNF in wrong state received");
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_establish_cnf_send      |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_ESTABLISH_CNF 
                     in state  TL_SEND_ESTABLISH.

*/

GLOBAL void tl_establish_cnf_send (void)
{
  GET_INSTANCE_DATA;
  drv_Return_Type pcm_result;
  T_SIM_PDU      *sim_pdu;

  TRACE_FUNCTION ("tl_establish_cnf_send()");

    if ( (SMS_SEL_REC(sms_data) NEQ SMS_RECORD_NOT_EXIST) AND
         (!tl_modify_is_all (SMS_MODIFY(sms_data))) )
    {
      /*
       * SEND - MODIFY
       */
       switch (SMS_SEL_MEM(sms_data))
       {
       case MEM_SM:

         if (SMS_SEL_REC(sms_data) <= SMS_SIM_PROP(sms_data).max_record)
         {
            tl_set_access_fifo (ACCESS_BY_MMI);
           /*
            * TL state transition: TL_SEND
            */
            SET_STATE (STATE_MMI, MMI_READ);
            SMS_INST_SET_STATE (STATE_TL, TL_SEND);
            tl_sim_read_record_req (SMS_SEL_REC(sms_data));
         }
         else
         {
            tl_send_failure(SMS_CAUSE_INV_INDEX);
           /*
            * TL state transition: TL_IDLE
            */
            SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         }
         break;

       case MEM_ME:

         if (SMS_SEL_REC(sms_data) <= SMS_ME_PROP(sms_data).max_record)
         {
            USHORT max_record;
            UBYTE  misc;
            UBYTE  sim_msg[SIZE_EF_SMS];
            MCAST (cp_data, U_CP_DATA);

/* Implements Measure#32: Row 109 */
            pcm_result = pcm_ReadRecord ((UBYTE *)ef_sms_id,
                                         (USHORT)SMS_SEL_REC(sms_data),
                                         SIZE_EF_SMS,
                                         sim_msg, &misc, &max_record);
            if (pcm_result NEQ PCM_OK)
            {
                SMS_ME_PROP(sms_data).max_record = 0;
               /*
                * stop sending
                */
                tl_send_failure(SMS_CAUSE_MEM_FAIL);
               /*
                * TL state transition: TL_IDLE
                */
                SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
                break;
            }
            MALLOC (sim_pdu, sizeof(T_SIM_PDU));
            CCD_START;
           /*
            * modify, prepare submit
            */
            if (tl_modify_submit (sim_pdu, SMS_MODIFY(sms_data),
                                  SMS_SDU(sms_data), sim_msg) AND
                tl_prepare_submit (sim_pdu, cp_data))
            {
              /*  
               *   RL_DATA_REQ
               */
               if (!rl_data_req (SMS_INST.tp_mr, cp_data))
               {
                 /*
                  * stop sending
                  */
                  tl_send_failure(SMS_CAUSE_ENTITY_BUSY);
                 /*
                  * TL state transition: TL_IDLE
                  */
                  SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
               }
               else
               {
                  SMS_EM_MO_SHORT_MESSAGE;
                 /*
                  * TL state transition: TL_SEND
                  */
                  SMS_INST_SET_STATE (STATE_TL, TL_SEND);
               }
            }
            else
            {
               /*
                * stop sending
                */
                tl_send_failure(SMS_CAUSE_PARAM_WRONG);
               /*
                * TL state transition: TL_IDLE
                */
                SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
            }
            CCD_END;
            MFREE (sim_pdu);
         }
         else
         {
           /*
            * stop sending
            */
            tl_send_failure(SMS_CAUSE_INV_INDEX);
           /*
            * TL state transition: TL_IDLE
            */
            SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         }
         break;

       default:
         /*
          * stop sending
          */
          tl_send_failure(SMS_CAUSE_PARAM_WRONG);
         /*
          * TL state transition: TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
       }
    }
    else  /* no record given, or no previous mem read */
    {
      /*
       * SEND - STRAIGHT
       */
       USHORT cause = SMS_NO_ERROR;

       switch (SMS_SEL_MEM(sms_data))
       {
       case MEM_SM:
         if (SMS_SEL_REC(sms_data) > SMS_SIM_PROP(sms_data).max_record)
         {
            cause = SMS_CAUSE_INV_INDEX;
         }
         break;

       case MEM_ME:
         if (SMS_SEL_REC(sms_data) > SMS_ME_PROP(sms_data).max_record)
         {
            cause = SMS_CAUSE_INV_INDEX;
         }
         break;

       default:
         if (SMS_SEL_REC(sms_data) NEQ SMS_RECORD_NOT_EXIST)
         {
            cause = SMS_CAUSE_PARAM_WRONG;
         }
         break;
       }

       if (cause EQ SMS_NO_ERROR)
       {
          /* Do some additional tests on the SMS SDU */
          if (SMS_SDU(sms_data)->l_buf EQ 0)
          {
             TRACE_ERROR ("MNSMS_SUBMIT_REQ: sms_sdu empty");
             cause = SMS_CAUSE_PARAM_WRONG;
          }
          if ((SMS_SDU(sms_data)->l_buf >> 3) + 
              (SMS_SDU(sms_data)->o_buf >> 3) > SIM_PDU_LEN)
          {
             TRACE_ERROR ("MNSMS_SUBMIT_REQ: sms_sdu out of range");
             cause = SMS_CAUSE_PARAM_WRONG;
          }
       }

       if (cause EQ SMS_NO_ERROR)  /* error on previous parameter check? */
       {
          MCAST (cp_data, U_CP_DATA);
          MALLOC (sim_pdu, sizeof(T_SIM_PDU));

          CCD_START;
          ccd_decodeMsg (CCDENT_SMS, BOTH,
                         (T_MSGBUF *)SMS_SDU(sms_data),
                         (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);
         /*
          * prepare submit
          */
          if (tl_prepare_submit (sim_pdu, cp_data))
          {
            /*  
             *   RL_DATA_REQ
             */
             if (!rl_data_req (SMS_INST.tp_mr, cp_data))
             {
               cause = SMS_CAUSE_ENTITY_BUSY;
             }
             else
             {
               SMS_EM_MO_SHORT_MESSAGE;
              /*
               * TL state transition
               */
               SMS_INST_SET_STATE (STATE_TL, TL_SEND);
             }
          }
          else
          {
             cause = SMS_CAUSE_PARAM_WRONG;
          }

          MFREE (sim_pdu);
          CCD_END;
       }
       if (cause NEQ SMS_NO_ERROR)
       {
         /*
          * stop sending
          */
          tl_send_failure(cause);
         /*
          * TL state transition: TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
       }
    }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_establish_cnf_rtx       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_ESTABLISH_CNF 
                     in state  TL_RTX_ESTABLISH.

*/
GLOBAL void tl_establish_cnf_rtx (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_establish_cnf_rtx()");

  {
      MCAST (cp_data, U_CP_DATA); /* T_U_CP_DATA */
      BUF_tpdu *tpdu = &cp_data->cp_user_data_ul.rp_data_ul.rp_user_data.tpdu;
     /*
      * check whether data req still exists
      */
      if (SMS_DATA_REQ(sms_data) EQ NULL)
      {
        TRACE_ERROR("Horror: cannot retransmit DATA REQ: buffer gone");
        return;
      }
     /*
      * Inform ACI that MO transmission attempt is still going on
      */
     /* tl_mnsms_send_prog_ind(sms_data); */ /* TCS 4.x and above */

      CCD_START;

     /*
      * Revert addition of PD and TI
      */
      SMS_DATA_REQ(sms_data)->sdu.o_buf += BSIZE_TI_PD;
      SMS_DATA_REQ(sms_data)->sdu.l_buf -= BSIZE_TI_PD;

      ccd_decodeMsg (CCDENT_SMS,
                     UPLINK,
                     (T_MSGBUF *)&SMS_DATA_REQ(sms_data)->sdu,
                     (UBYTE *)cp_data,
                     NOT_PRESENT_8BIT);

     /* 
      * Set the TP-RD bit in the SMS-SUBMIT. 
      * No usage of CCD here, this would be inefficient.
      */
      tpdu->b_tpdu[tpdu->o_tpdu >> 3] |= 0x04;
     /*
      * TL state transition: TL_SEND
      */
      SMS_INST_SET_STATE (STATE_TL, TL_SEND);
     /*
      * RL_DATA_REQ
      */
      rl_data_req (SMS_INST.tp_mr, cp_data);

      CCD_END;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_establish_cnf_cmd       |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_ESTABLISH_CNF 
                     in state  TL_CMD_ESTABLISH.

*/

GLOBAL void tl_establish_cnf_cmd(void)
{
  GET_INSTANCE_DATA;
  T_SIM_PDU      *sim_pdu;

  TRACE_FUNCTION ("tl_establish_cnf_cmd()");

    CCD_START;

    MALLOC (sim_pdu, sizeof(T_SIM_PDU));
    memset (sim_pdu, 0, sizeof(T_SIM_PDU));

    ccd_decodeMsg (CCDENT_SMS, BOTH,
                   (T_MSGBUF *)SMS_SDU(sms_data),
                   (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);

    if (sim_pdu->tp_mti EQ SMS_COMMAND AND sim_pdu->v_tpdu)
    {
        MCAST (cp_data, U_CP_DATA);

        /* ++SMS_INST.tp_mr; */
        sim_pdu->tpdu.b_tpdu[1]  = SMS_INST.tp_mr;
        SMS_TP_REF_RET(sms_data) = SMS_INST.tp_mr;

        SMS_INST_SET_STATE (STATE_TL, TL_COMMAND);

        SMS_EM_MO_SHORT_MESSAGE_COMMAND;

        cp_data->cp_user_data_ul.v_rp_error = FALSE;
        cp_data->cp_user_data_ul.v_rp_ack = FALSE;
        memset (&cp_data->cp_user_data_ul.rp_data_ul, 0,
                 sizeof (T_rp_data_ul));

        memcpy (&cp_data->cp_user_data_ul.rp_data_ul.rp_addr,
                &sim_pdu->rp_addr, sizeof (T_rp_addr));
        cp_data->cp_user_data_ul.rp_data_ul.rp_user_data.tp_mti
           = sim_pdu->tp_mti;
        memcpy (&cp_data->cp_user_data_ul.rp_data_ul.rp_user_data.tpdu,
                &sim_pdu->tpdu, sizeof (BUF_tpdu));
        cp_data->cp_user_data_ul.rp_data_ul.rp_user_data.v_tpdu
           = TRUE;
        cp_data->cp_user_data_ul.v_rp_data_ul = TRUE;


       /*
        * RL_DATA_REQ
        */
        if (!rl_data_req (SMS_INST.tp_mr, cp_data))
        {
          tl_mnsms_command_cnf (NOT_PRESENT_8BIT, SMS_CAUSE_ENTITY_BUSY, NULL);
        }
    }
    else
    {
       tl_mnsms_command_cnf (NOT_PRESENT_8BIT, SMS_CAUSE_PARAM_WRONG, NULL);
    }

    MFREE (sim_pdu);
    CCD_END;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_data_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal DATA_IND.

*/

GLOBAL void tl_data_ind (
                         T_rp_data_dl *rp_data_dl)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_data_ind()");

  /* Valid tp-user data will not be set to TRUE unless futher decoding is done
  * so removed check rp_data_dl->rp_addr.v_tpdu 
  */
  if (rp_data_dl EQ NULL OR !rp_data_dl->rp_addr.v_ton)
  { 
   /*
    * RP_ERROR =>
    */
    rl_report_req_error (SMS_RP_CS_INV_MAND_INFO, NULL);
   /*
    * RL_RELEASE_REQ ==>
    */
    rl_release_req(SMS_INST.ti);
    return;
  }
  switch (rp_data_dl->rp_user_data.tp_mti)
  {
    case SMS_DELIVER:
    {
      MCAST (sms_deliver, TP_DELIVER);

      TRACE_EVENT ("SMS DELIVER");

      if (SMS_RP_RCVD(sms_data) EQ NULL)
      {
        MALLOC (SMS_RP_RCVD(sms_data), sizeof(T_rp_data_dl));
      }
      memcpy (SMS_RP_RCVD(sms_data), rp_data_dl,
              sizeof(T_rp_data_dl));
      
      /*
      *If Valid TP user Data is missing in RP Message, send RP ERROR
      * This is done by checking v_tp_ud field
      */
      if ( ccd_decodeMsg (CCDENT_SMS,
                          DOWNLINK,
                          (T_MSGBUF *)&SMS_RP_RCVD(sms_data)->rp_user_data.tpdu,
                          (UBYTE *)sms_deliver,
                          SMS_VT_DELIVER) EQ ccdError )
      {
       /*
        * RP_ERROR =>
        */
        rl_report_req_error (SMS_RP_CS_INV_MAND_INFO, NULL);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);
        break;
      }

      SMS_INST_SET_STATE (STATE_TL, TL_RECEIVE);

      SMS_DCS(sms_data) = sms_deliver->tp_dcs;
      SMS_PID(sms_data) = sms_deliver->tp_pid;
#ifdef FF_SMS_23430
      if (tl_handle_23430 (sms_deliver))
        return;
#endif
      switch (sms_deliver->tp_pid)
      {
        case SMS_PID_SM_TYPE_0:
          /*
           * "A short message type 0 indicates that the ME must acknowledge
           * receipt of the short message but shall discard its contents."
           * [3GPP 23.040 subclause 9.2.3.9]
           */
          rl_report_req_ack (NULL);
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);

          MFREE (SMS_RP_RCVD(sms_data));
          SMS_RP_RCVD(sms_data) = NULL;
          break;

        case SMS_PID_REP_SM_TYPE_1:
        case SMS_PID_REP_SM_TYPE_2:
        case SMS_PID_REP_SM_TYPE_3:
        case SMS_PID_REP_SM_TYPE_4:
        case SMS_PID_REP_SM_TYPE_5:
        case SMS_PID_REP_SM_TYPE_6:
        case SMS_PID_REP_SM_TYPE_7:
        case SMS_PID_RET_CALL_MSG:
          /*
           * replace short messages
           */
          tl_replace_message (sms_deliver);
          break;
        case SMS_PID_SIM_DOWNLOAD:
          /*
           * SIM data download
           */
#ifdef SIM_TOOLKIT
          if (sms_data->download_sms AND
              tl_check_class_2 (sms_deliver->tp_dcs))
          {
            tl_build_envelope_sms_download (SMS_RP_RCVD(sms_data));
          }
          else
#endif
            tl_handle_message (sms_deliver);
          break;

#ifdef REL99
        /* Currently EGPRS over TIA/EIA-136 is not supported and hence it 
         * is treated as below. If TIA/EIA-136 is supported, the message 
         * should be passed to ME for routing to the application handling 
         * ANSI-136 data
         */
        case SMS_PID_ANSI_136_R_DATA:  
          if( tl_check_class_2 (sms_deliver->tp_dcs) )
          {
#ifdef SIM_TOOLKIT
            /* Download it to SIM only if both SAT is supported and data download 
             * service is enabled 
             */
            if (sms_data->download_sms)
            {
              tl_build_envelope_sms_download (SMS_RP_RCVD(sms_data));
            }
            else
            {
              /* 11.14 8.d.0 Sec 7.1.1 mentions about storing the message when 
               * If the service "data download via SMS-PP" is not allocated 
               * and activated only for protocol identifier = SIM data download. 
               * Hence discard the message and send NACK indicating inability 
               * to handle the message*/
              T_rp_user_data *rp_ud;
              MALLOC (rp_ud, sizeof(T_rp_user_data));
              rp_ud->tpdu.o_tpdu = 0;
              rp_ud->tp_mti = rp_ud->tpdu.b_tpdu[0] = SMS_DELIVER_REPORT;
              rp_ud->tpdu.b_tpdu[1] = SMS_FCS_TP_PID_UNSPEC;  /* TP-FCS  */
              rp_ud->tpdu.b_tpdu[2] = 0;                      /* TP-PI   */
              rp_ud->tpdu.l_tpdu    = 24;                     /* 3 bytes */
              rp_ud->v_tpdu = TRUE;
              rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, rp_ud);
              /*
               * RL_RELEASE_REQ ==>
               */
              rl_release_req( SMS_INST.ti);
              MFREE (rp_ud);
              
              SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
              MFREE (SMS_RP_RCVD(sms_data));
              SMS_RP_RCVD(sms_data) = NULL;
            }
#else
            /*
             * Discard the message and send NACK
             */
            T_rp_user_data *rp_ud;
            MALLOC (rp_ud, sizeof(T_rp_user_data));
            rp_ud->tpdu.o_tpdu = 0;
            rp_ud->tp_mti = rp_ud->tpdu.b_tpdu[0] = SMS_DELIVER_REPORT;
            rp_ud->tpdu.b_tpdu[1] = SMS_FCS_TP_PID_UNSPEC;  /* TP-FCS */
            rp_ud->tpdu.b_tpdu[2] = 0;                    /* TP-PI */
            rp_ud->tpdu.l_tpdu = 24;                      /* 3 bytes */
            rp_ud->v_tpdu = TRUE;
            rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, rp_ud);
            /*
             * RL_RELEASE_REQ ==>
             */
            rl_release_req(SMS_INST.ti);
            MFREE (rp_ud);
           
            SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
            MFREE (SMS_RP_RCVD(sms_data));
            SMS_RP_RCVD(sms_data) = NULL;
#endif
          }
          else
          {
            tl_handle_message (sms_deliver);
          }
          break;
#endif /* REL99 */

        case SMS_PID_ME_DEPERSON:
          /*
           * ME de-personalization short message
           */
#ifdef SIM_PERS_OTA
             {
               PALLOC (mnsms_OTA_message_ind, MNSMS_OTA_MESSAGE_IND);
               memcpy (&mnsms_OTA_message_ind->tp_ud, &sms_deliver->tp_ud, sizeof(T_tp_ud));
               mnsms_OTA_message_ind->tp_dcs = sms_deliver->tp_dcs ; 
               mnsms_OTA_message_ind->tp_pid = sms_deliver->tp_pid; 
  #ifdef TI_PS_HCOMM_CHANGE
               PSEND (_hCommMMI, mnsms_OTA_message_ind);
  #else
               PSEND (hCommMMI, mnsms_OTA_message_ind);
  #endif
               break;
             }
#else 
      break;
#endif
        case SMS_PID_ME_DOWNLOAD:
          /*
           * ME download short message
           */
        default:
          tl_handle_message (sms_deliver);
          break;
      }
      break;
    }

    case SMS_STATUS_REPORT:
      TRACE_EVENT ("SMS STATUS REPORT");

      if (sms_data->ds EQ DS1)
      {
        /*
         * only if requested by MMI
         */
        PALLOC (status_ind, MNSMS_STATUS_IND);

        SMS_EM_SMS_STATUS_MESSAGE;

        tl_build_status_rep (rp_data_dl, status_ind);
        PSENDX (MMI, status_ind);

        if (SMS_MT_ACK_MODE(sms_data) EQ SMS_MHC_PH2PLUS)
        {
          SMS_INST_SET_STATE (STATE_TL, TL_RECEIVE);
          break;
        }
      }
      rl_report_req_ack (NULL);
      break;

    default:
     /*
      * RP_ERROR =>
      */
      rl_report_req_error (SMS_RP_CS_SEM_INC_MSG, NULL);
     /*
      * RL_RELEASE_REQ ==>
      */
      rl_release_req(SMS_INST.ti);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_timeout_ind             |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_TIMEOUT_IND.

*/
GLOBAL void tl_timeout_ind ( USHORT         timer_id)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_timeout_ind()");

  switch (SMS_INST_GET_STATE (STATE_TL))
  {
 /* -------------------------------------- */
    case TL_ESTABLISH:
 /* -------------------------------------- */

    switch (GET_STATE(STATE_EST))
    {
    case EST_SEND:
      /*
       * Connection establishment for the SUBMIT has failed
       */
       TRACE_EVENT("Connection establishment for SUBMIT request failed");
        
       TRACE_EVENT_P1 ("TL retrans #%d",  SMS_INST.tl_retx);
        
       if (SMS_INST.tl_retx < TL_MAX_RETANS)
       {
         /*
          * Maximum number of TL retransmissions not exceeded.
          * Retry establishment.
          */
          SMS_INST.tl_retx++;
         /*
          * establish connection
          */
#ifdef REL99
          tl_mnsms_send_prog_ind(); 
#endif /* REL99 */
          tl_establish_connection(FALSE);
       }
       else
       {
         /*
          * clean up instance
          */
          if (SMS_SDU(sms_data) NEQ NULL)
          {
             MFREE (SMS_SDU(sms_data));
             SMS_SDU(sms_data) = NULL;
          }
          SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
         /*
          * TL state transition TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         /*
          * SUBMIT_CNF
          */
          tl_mnsms_submit_cnf (SMS_SEL_MEM(sms_data), 
                               SMS_RECORD_NOT_EXIST,
                               SMS_TP_REF_RET(sms_data),
                               SMS_CAUSE_NET_TIMEOUT,
                               SMS_SDU(sms_data));
         /*
          * End the cmms session
          */
          if(CMMS_ACTIVE)
          {
             tl_cmms_end();
          }

       }
       break;

    case EST_CMD:
      /*
       * Connection establishment for the COMMAND has failed
       */
       TRACE_EVENT("Connection establishment for COMMAND request failed");
        
       TRACE_EVENT_P1 ("TL retrans #%d",  SMS_INST.tl_retx);
        
       if (SMS_INST.tl_retx < TL_MAX_RETANS)
       {
         /*
          * Maximum number of TL retransmissions not exceeded.
          * Retry establishment.
          */
          SMS_INST.tl_retx++;
         /*
          * establish connection
          */
#ifdef REL99
          tl_mnsms_send_prog_ind(); 
#endif
          tl_establish_connection(FALSE);
       }
       else
       {
         /*
          * clean up instance
          */
          if (SMS_SDU(sms_data) NEQ NULL)
          {
             MFREE (SMS_SDU(sms_data));
             SMS_SDU(sms_data) = NULL;
          }
          SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
         /*
          * TL state transition
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         /*
          * COMMAND_CNF
          */
          tl_mnsms_command_cnf (SMS_TP_REF_RET(sms_data),
                                SMS_CAUSE_NET_TIMEOUT,
                                SMS_SDU(sms_data));
       }
       break;

    case EST_RTX:
       TRACE_EVENT("timeout establishment for retransmission ignored");
       break;

    case EST_SMMA:
      /*
       * Connection establishment for the SMMA has failed
       */
       TRACE_EVENT("Connection establishment for SMMA request failed");
        
       TRACE_EVENT_P1 ("TL retrans #%d",  SMS_INST.tl_retx);
        
       if (SMS_INST.tl_retx < TL_MAX_RETANS)
       {
         /*
          * Maximum number of TL retransmissions not exceeded.
          * Retry establishment.
          */
          SMS_INST.tl_retx++;
         /*
          * try again
          */
          SMS_INST.retrans  = TRUE;
         /*
          * establish connection
          */

          tl_establish_connection(FALSE);

       }
       else
       {
         /*
          * TL  state transition TL_OTHER
          */
          SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
         /*
          * handle event in report ind
          */
          tl_report_ind(NULL,SMS_CAUSE_NET_TIMEOUT);
       }
       break;
    }
    break;

 /* -------------------------------------- */
    case TL_SEND:
 /* -------------------------------------- */
      /*
       * sending of the SUBMIT has failed
       */
       TRACE_EVENT("Transmission for SUBMIT request failed");
        
       TRACE_EVENT_P1 ("TL retrans #%d",  SMS_INST.tl_retx);
        
       if (SMS_INST.tl_retx < TL_MAX_RETANS)
       {
         /*
          * Maximum number of TL retransmissions not exceeded.
          * Retry establishment.
          */
          SMS_INST.tl_retx++;
#ifdef REL99
          tl_mnsms_send_prog_ind(); 
#endif /* REl99 */

         /*
          * TL  state transition TL_ESTABLISH
          * EST state transition EST_RTX
          */
          SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
          SET_STATE(STATE_EST,EST_RTX);
         /*
          * establish connection
          */
          tl_establish_connection(FALSE);
       }
       else
       {
         /*
          * clean up instance
          */
          if (SMS_SDU(sms_data) NEQ NULL)
          {
             MFREE (SMS_SDU(sms_data));
             SMS_SDU(sms_data) = NULL;
          }
          SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
         /*
          * TL state transition TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         /*
          * SUBMIT_CNF
          */
          tl_mnsms_submit_cnf (SMS_SEL_MEM(sms_data), 
                               SMS_RECORD_NOT_EXIST,
                               SMS_TP_REF_RET(sms_data),
                               SMS_CAUSE_NET_TIMEOUT,
                               SMS_SDU(sms_data));
          /*
          * End the cmms session
          */
          if(CMMS_ACTIVE)
          {
             tl_cmms_end();
          }
       }
       break;

 /* -------------------------------------- */
    case TL_COMMAND:
 /* -------------------------------------- */
      /*
       * sending of the COMMAND has failed
       */
       TRACE_EVENT("Transmission for COMMAND request failed");
        
       TRACE_EVENT_P1 ("TL retrans #%d",  SMS_INST.tl_retx);
        
       if (SMS_INST.tl_retx < TL_MAX_RETANS)
       {
         /*
          * Maximum number of TL retransmissions not exceeded.
          * Retry establishment.
          */
          SMS_INST.tl_retx++;
#ifdef REL99
          tl_mnsms_send_prog_ind(); 
#endif /* REl99 */
         /*
          * TL  state transition TL_ESTABLISH
          * EST state transition EST_CMD
          */
          SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
          SET_STATE(STATE_EST,EST_CMD);
         /*
          * establish connection
          */
          tl_establish_connection(FALSE);
       }
       else
       {
         /*
          * clean up instance
          */
          if (SMS_SDU(sms_data) NEQ NULL)
          {
             MFREE (SMS_SDU(sms_data));
             SMS_SDU(sms_data) = NULL;
          }
          SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
         /*
          * TL state transition TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
         /*
          * COMMAND_CNF
          */
          tl_mnsms_command_cnf (SMS_TP_REF_RET(sms_data),
                                SMS_CAUSE_NET_TIMEOUT,
                                SMS_SDU(sms_data));
       }
       break;

 /* -------------------------------------- */
    default:
 /* -------------------------------------- */

       // TRACE_EVENT("TL_TIMEOUT_IND ignored");
       if (timer_id EQ TRAM)
       {
         /*
          * sending of the SMMA has failed
          */
          TRACE_EVENT("Transmission for SMMA failed");
         /*
          *  ??? !!! ???
          */
          --SMS_RP_REF(sms_data);
         /*
          * TL  state transition TL_ESTABLISH
          * EST state transition EST_SMMA
          */
          SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
          SET_STATE(STATE_EST,EST_SMMA);
         /*
          * try again
          */
          SMS_INST.retrans  = TRUE;
         /*
          * establish connection
          */
          tl_establish_connection(FALSE);
       }
       else
       {
          tl_report_ind(NULL,SMS_CAUSE_NET_TIMEOUT);
       }
       break;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLS                    |
| STATE   : code                ROUTINE : tl_report_ind              |
+--------------------------------------------------------------------+

  PURPOSE : Process the signal TL_REPORT_IND.

*/

LOCAL UBYTE tl_check_condx_send (T_BACKUP *prop,
                                 UBYTE    mem_type,
                                 UBYTE    condx,
                                 UBYTE    rec_num)
{
  if (condx EQ SMS_CONDX_OVR_NON)
  {
    if ((rec_num = tl_get_free_space (mem_type)) NEQ 0)
      return rec_num;
    else
      return SMS_RECORD_NOT_EXIST;
  }
  else if (condx EQ SMS_CONDX_OVR_MO)
  {
    switch (tl_get_status (prop, rec_num - 1))
    {
    case SMS_RECORD_REC_UNREAD:
    case SMS_RECORD_REC_READ:
      if ((rec_num = tl_get_free_space (mem_type)) NEQ 0)
        return rec_num;
      return SMS_RECORD_NOT_EXIST;

    default:
      break;
    }
  }
  return rec_num;
}

GLOBAL void tl_report_ind (T_rp_user_data *rp_user_data,
                           USHORT         cause)
{
  GET_INSTANCE_DATA;
  T_SIM_PDU *sim_pdu;
  UBYTE rec_num, next_rec_num = 0;

  TRACE_FUNCTION ("tl_report_ind()");

#ifdef REL99
  if (cause NEQ SMS_NO_ERROR)
  {
    SMS_INST.failed_msg_rec_num = SMS_SEL_REC(sms_data);
    SMS_INST.failed_msg_mem = SMS_SEL_MEM(sms_data);
  }
#endif

  if (sms_data)
  {
    if (SMS_SDU(sms_data) NEQ NULL)
    {
      MFREE (SMS_SDU(sms_data));
      SMS_SDU(sms_data) = NULL;
    }

    /* 
     * An SMS DELIVER REPORT TPDU is carried as a RP User Data element within
     * an RP ERROR PDU and is part of the negative acknowledgement to an SMS 
     * DELIVER or SMS STATUS REPORT.
     *
     * An SMS DELIVER REPORT TPDU is also carried as a RP User Data element 
     * within an RP ACK PDU and is part of a positive acknowledgement to a 
     * SMS DELIVER or SMS STATUS REPORT.
     * [3gpp 23.040 clause 9.2.2.1a]
     * 
     * The cause value (invalid / non-invalid) is used to distinguish whether
     * the RP user data (SMS DELIVER REPORT) came with RP-ACK / RP-ERROR.
     */

    if (IS_CAUSE_INVALID(cause) AND
        (rp_user_data NEQ NULL AND rp_user_data->v_tpdu))
    {
      MALLOC (sim_pdu, sizeof(T_SIM_PDU));
      MALLOC (SMS_SDU(sms_data), sizeof(T_sms_sdu));

      memset (&sim_pdu->rp_addr, 0, sizeof(T_rp_addr));
      sim_pdu->tp_mti = rp_user_data->tp_mti;
      sim_pdu->tpdu = rp_user_data->tpdu;
      sim_pdu->v_tpdu = TRUE;
      sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

      SMS_SDU(sms_data)->o_buf = 0;
      SMS_SDU(sms_data)->l_buf = SIM_PDU_LEN<<3;
      if (ccd_codeMsg (CCDENT_SMS, BOTH,
                       (T_MSGBUF *)SMS_SDU(sms_data),
                       (UBYTE *)sim_pdu, SMS_VT_SIM_PDU) NEQ ccdOK)
      {
        MFREE (SMS_SDU(sms_data));
        SMS_SDU(sms_data) = NULL;
      }
      MFREE (sim_pdu);
    }
    switch (SMS_INST_GET_STATE (STATE_TL))
    {
 /* -------------------------------------- */
    case TL_ESTABLISH:
 /* -------------------------------------- */

       switch (GET_STATE(STATE_EST))
       {
       case EST_SEND:
       case EST_RTX:
         /*
          * TL state transition TL_SEND
          */
          SMS_INST_SET_STATE (STATE_TL, TL_SEND);
          break;
       case EST_CMD:
         /*
          * TL state transition TL_COMMAND
          */
          SMS_INST_SET_STATE (STATE_TL, TL_COMMAND);
          break;
       case EST_SMMA:
          if (SMS_INST.retrans)
          {
             SMS_INST.retrans = FALSE;
            /*
             * TL state transition TL_OTHER
             */
             SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
          }
          else
          {
            /*
             * RL_START_TRAM_REQ
             */
             rl_start_tram_req();
             return;
          }
          break;
       default:
          TRACE_ERROR("Invalid EST state!");
         /*
          * TL state transition TL_IDLE
          */
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
          break;
       }
      /*
       * TL_REPORT_IND recursive
       */
       tl_report_ind(rp_user_data,cause);
       return;

 /* -------------------------------------- */
    case TL_SEND:
 /* -------------------------------------- */

      if (!IS_CAUSE_INVALID(cause) AND
          (rp_user_data NEQ NULL) AND (SMS_INST.tl_retx > 0))
      {
        /* 
         * Retransmission case as of 23.040 subclause 9.2.3.6
         * and RP ERROR with RP USER DATA received. Check whether 
         * the RP USER DATA contains a SMS SUBMIT REPORT with TP-FCS set to
         * "SM Rejected - Duplicate SM".
         * If so the transmission has been a success and not a failure.
         */
        MCAST (tp_sbmt_rep_err, TP_SBMT_REP_ERR); /* T_TP_SBMT_REP_ERR */
        BUF_tpdu *tpdu = &rp_user_data->tpdu;
        BYTE ccd_result;

        switch (tpdu->b_tpdu[tpdu->o_tpdu >> 3] & 0x3)
        {
          case SMS_SUBMIT_REPORT:
            /* Copy tpdu from rp_user_data, it's part of decoded msg buffer */
            MALLOC (tpdu, sizeof(BUF_tpdu));
            memcpy (tpdu, &rp_user_data->tpdu, sizeof (BUF_tpdu));
            
            CCD_START;
            ccd_result = ccd_decodeMsg (CCDENT_SMS, DOWNLINK, 
                                        (T_MSGBUF *)tpdu,
                                        (UBYTE *)tp_sbmt_rep_err,
                                        SMS_VT_SBMT_REP_ERR);
            if (ccd_result NEQ ccdOK)
            {
              /* Could error handling be elaborated here? */
              TRACE_ERROR ("ccd decoding problem");
            }

            if (tp_sbmt_rep_err->tp_fcs EQ SMS_FCS_REJ_DUPL_SM)
            {
              TRACE_EVENT ("SC had SMS");
              cause = SMS_NO_ERROR; /* Enter "no error" handling below */
              /* Since the transmission was successful, the data stored for 
                 retransmission can be freed */
              if (SMS_DATA_REQ(sms_data) NEQ NULL)
              {
                PFREE (SMS_DATA_REQ(sms_data));
                SMS_DATA_REQ(sms_data) = NULL;
              }
            }
            CCD_END;

            MFREE (tpdu);
            break;

          default:
            break; /* Do nothing, it is an error */
        }
      }

      if (IS_CAUSE_INVALID(cause) AND SMS_SEL_REC(sms_data) > 0)
      {
        if (SMS_SEL_MEM(sms_data) EQ MEM_SM AND
            SMS_SEL_REC(sms_data) <= SMS_SIM_PROP(sms_data).max_record)
        {
          if ((rec_num = tl_check_condx_send (&SMS_SIM_PROP(sms_data),
                                              MEM_SM,
                                              SMS_CONDX(sms_data),
                                              SMS_SEL_REC(sms_data)))
               NEQ SMS_RECORD_NOT_EXIST)
            SMS_SEL_REC(sms_data) = rec_num;
          else
          {
#ifdef REL99
            if(SMS_INST.failed_msg_retx EQ FALSE)
            {
#endif
              tl_mnsms_submit_cnf (MEM_SM, SMS_RECORD_NOT_EXIST,
                                   SMS_TP_REF_RET(sms_data),
                                   SMS_CAUSE_MEM_FULL, SMS_SDU(sms_data));
#ifdef REL99
            }
            else
            {
              tl_mnsms_retrans_cnf (MEM_SM, SMS_RECORD_NOT_EXIST,
                                   SMS_TP_REF_RET(sms_data),
                                   SMS_CAUSE_MEM_FULL, SMS_SDU(sms_data));              
            }    
#endif
            
            break;
          }
          {
              UBYTE   sim_acc_ix;
              T_SMS_SIM_ACCESS_INFO   *sms_sim_access_info_ptr;
              if (tl_sms_reserve_req_id(&sim_acc_ix))
              {
                PALLOC (update_req, SIM_UPDATE_RECORD_REQ);
                sms_sim_access_info_ptr = &sms_data->sms_sim_access_info[sim_acc_ix];
                update_req->source    = SRC_SMS;
                update_req->req_id     = sim_acc_ix;
                update_req->v_path_info = FALSE;
                update_req->datafield = sms_sim_access_info_ptr->datafield   
                                      = SIM_SMS;
                update_req->record    = sms_sim_access_info_ptr->rec_num     
                                      = SMS_SEL_REC(sms_data);
                update_req->length    = SIM_LENGTH_SMS_RECORD;

                tl_convert_mo_to_mem (SMS_DATA_REQ(sms_data),
                                      update_req->linear_data);

                PSENDX (SIM, update_req);
              }
          }
          tl_store_status (&SMS_SIM_PROP(sms_data), rec_num-1, SIM_SMS_PENDING);
          tl_set_access_fifo (ACCESS_BY_MMI);
          SET_STATE (STATE_MMI, MMI_REPLACE);
         /*
          * concat control
          */
          if (sms_data->concat_cntrl.concatenation NEQ TRUE)
          {
            /*
             * RL_RELEASE_REQ ==>
             */
             rl_release_req(SMS_INST.ti);
          }
          return;
        }
        if (SMS_SEL_MEM(sms_data) EQ MEM_ME AND
            SMS_SEL_REC(sms_data) <= SMS_ME_PROP(sms_data).max_record)
        {
          UBYTE data[SIZE_EF_SMS];

          if ((rec_num = tl_check_condx_send (&SMS_ME_PROP(sms_data),
                                              MEM_ME,
                                              SMS_CONDX(sms_data),
                                              SMS_SEL_REC(sms_data)))
               NEQ SMS_RECORD_NOT_EXIST)
            SMS_SEL_REC(sms_data) = rec_num;
          else
          {
#ifdef REL99
            if(SMS_INST.failed_msg_retx EQ FALSE)
            {
#endif
              tl_mnsms_submit_cnf (MEM_ME, SMS_RECORD_NOT_EXIST,
                                 SMS_TP_REF_RET(sms_data),
                                 SMS_CAUSE_MEM_FULL, SMS_SDU(sms_data));
#ifdef REL99
            }
            else
            {
              tl_mnsms_retrans_cnf (MEM_ME, SMS_RECORD_NOT_EXIST,
                                    SMS_TP_REF_RET(sms_data),
                                    SMS_CAUSE_MEM_FULL, SMS_SDU(sms_data));              
            }
#endif
            break;
          }
          tl_convert_mo_to_mem (SMS_DATA_REQ(sms_data), data);
/* Implements Measure#32: Row 113 */
          if (pcm_WriteRecord ((UBYTE *)ef_sms_id, SMS_SEL_REC(sms_data),
                               SIZE_EF_SMS, data) NEQ DRV_OK)
          {
            cause = SMS_CAUSE_MEM_FAIL;
          }
        }
      }
     /*
      * MNSMS_SUBMIT_CNF =>
      */
#ifdef REL99
      if(SMS_INST.failed_msg_retx EQ FALSE)
      {
#endif
        tl_mnsms_submit_cnf (SMS_SEL_MEM(sms_data), SMS_RECORD_NOT_EXIST,
                     SMS_TP_REF_RET(sms_data), cause, SMS_SDU(sms_data));
#ifdef REL99
      }
      else
      {
        tl_mnsms_retrans_cnf (SMS_SEL_MEM(sms_data), SMS_RECORD_NOT_EXIST,
                     SMS_TP_REF_RET(sms_data), cause, SMS_SDU(sms_data));        
      }    
#endif
     /*
      * concat control
      */
      if ((sms_data->concat_cntrl.concatenation EQ TRUE) AND !CMMS_ACTIVE)
      {
         if (cause EQ SMS_NO_ERROR)
         {
           /*
            * start timer TLCT for next submit req supervision
            */
            sms_timer_start(TLCT);
           /*
            * TL State Transition TL_SEND_CONTD
            */
            SMS_INST_SET_STATE (STATE_TL, TL_SEND_CONTD);
           /*
            * do not release connection now
            */
            sms_data->concat_cntrl.release_pending = TRUE;
            return;
         }
         else
         {
           /*
            * error case: end concatenation series
            */
            sms_data->concat_cntrl.concatenation = FALSE;
         }
      }
      else
      {
      /*
      * cmms control
      */
      if(CMMS_ACTIVE AND (sms_data->cmms_release_pending EQ TRUE))
      {
         tl_cmms_start();
         SMS_INST_SET_STATE (STATE_TL, TL_SEND_CONTD);

         if (sms_data->concat_cntrl.concatenation EQ TRUE)
         {
            if (cause EQ SMS_NO_ERROR)
            {
               sms_data->concat_cntrl.release_pending = TRUE;
            }
            else
            {
           /*
            * error case: end concatenation series
            */
               sms_data->concat_cntrl.concatenation = FALSE;
            }
         }
         return;
      }
      else
      if(CMMS_ACTIVE)
      {
         tl_cmms_end();
      }
      }
      break;

 /* -------------------------------------- */
    case TL_COMMAND:
 /* -------------------------------------- */

#ifdef REL99
      if(SMS_INST.failed_msg_retx EQ FALSE)
      {
#endif

        tl_mnsms_command_cnf (SMS_TP_REF_RET(sms_data), cause, SMS_SDU(sms_data));
#ifdef REL99
      }
      else
      {
        tl_mnsms_retrans_cnf (NOT_PRESENT_8BIT, SMS_RECORD_NOT_EXIST,
                     SMS_TP_REF_RET(sms_data), cause, SMS_SDU(sms_data));        
      }
#endif

      break;

 /* -------------------------------------- */
    case TL_RECEIVE:
 /* -------------------------------------- */

      if (!IS_CAUSE_INVALID(cause))
      {
        switch (GET_STATE (STATE_NET))
        {
        case NET_WRITE:
        case NET_READ:
          break;
        default:
          if (SMS_MT_ACK_MODE(sms_data) EQ SMS_MHC_PH2PLUS)
            tl_mnsms_error_ind ((USHORT)((GET_CAUSE_DEFBY(cause) EQ DEFBY_CONDAT)?
                                cause: SMS_CAUSE_NET_TIMEOUT));
          break;
        }
        SET_STATE (STATE_NET, NET_IDLE);
      }

 /* -------------------------------------- */
    case TL_OTHER:
 /* -------------------------------------- */

      if (GET_STATE (STATE_MMI) EQ MMI_DELETE)
      {
        if (!tl_sms_memo_exceeded (TRUE))
        {        
          if(SMS_REC_STATUS(sms_data) NEQ CMGD_DEL_INDEX)
          {
            if(SMS_SEL_MEM(sms_data) EQ MEM_SM)
            {
              /* Status value is given. Find the next record satisfying this status value */
              next_rec_num = tl_search_record_for_delete (&SMS_SIM_PROP(sms_data), 
                                                          SMS_SEL_REC(sms_data), 
                                                          SMS_REC_STATUS(sms_data));
            }
            else if(SMS_SEL_MEM(sms_data) EQ MEM_ME)
            {
              /* Status value is given. Find the next record satisfying this status value */
              next_rec_num = tl_search_record_for_delete (&SMS_ME_PROP(sms_data), 
                                                          SMS_SEL_REC(sms_data), 
                                                          SMS_REC_STATUS(sms_data));
            }
          }
          tl_mnsms_delete_cnf (SMS_SEL_MEM(sms_data), SMS_SEL_REC(sms_data), next_rec_num,
                                                                            SIM_NO_ERROR);
          SET_STATE (STATE_MMI, MMI_IDLE);
          #ifdef SIM_TOOLKIT
          if (sms_data->file_update_ind NEQ NULL)
          {
            T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
            sms_data->file_update_ind = NULL;
            tl_sim_file_update_ind (file_update_ind);
          }
          #endif
          break;
        }
        else
        {
         /*
          * RL_RELEASE_REQ ==>
          */
          rl_release_req(SMS_INST.ti);
          return;
        }
      }
      else if (GET_STATE (STATE_MMI) EQ MMI_RESUME)
      {
        /*
         *  <<acknowledgment>> of SMMA message for
         *      user initiated RESUME
         *  we expect here that the given tl_report_ind signal
         *  is generated in the following cases:
         *  - success (positive ack for smma)
         *  - error   (negative ack for smma)
         *  - repeated timeout (waiting for ack exceeded limit)
         */
        /*
         * Unset memory full condition in any case
         */
         sms_data->pr_cntrl.delivery_state = SMS_DELIVER_STATUS_RESUME;
        /*
         * save cause
         */
         if (IS_CAUSE_INVALID(cause))
         {
            sms_data->pr_cntrl.save_cause = SMS_NO_ERROR;
         }
         else
         {
            sms_data->pr_cntrl.save_cause = cause;
         }
        /*
         * Unset notification flag on sim
         */
         tl_sms_memo_resume();
         break;
      }
      else if (GET_STATE (STATE_MMI) NEQ MMI_IDLE)
      {
        SET_STATE (STATE_MMI, MMI_IDLE);
        #ifdef SIM_TOOLKIT
        if (sms_data->file_update_ind NEQ NULL)
        {
          T_SIM_FILE_UPDATE_IND *file_update_ind = sms_data->file_update_ind;
          sms_data->file_update_ind = NULL;
          tl_sim_file_update_ind (file_update_ind);
        }
        #endif
        break;
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/

 /* -------------------------------------- */
    case TL_IDLE:
 /* -------------------------------------- */

      if (SMS_ENT_STATE(sms_data) EQ SMS_STATE_INITIALISING)
      {
        if (!tl_sms_memo_exceeded (TRUE))
        {
          tl_mnsms_report_ind (SMS_ENT_STATE(sms_data) = SMS_STATE_READY);
          break;
        }
        else
        {
         /*
          * RL_RELEASE_REQ ==>
          */
          rl_release_req(SMS_INST.ti);
          return;
        }
      }
      break;

 /* -------------------------------------- */
    default:
 /* -------------------------------------- */

      TRACE_EVENT("TL_REPORT_IND ignored");
      break;
    }

    if (SMS_SDU(sms_data) NEQ NULL)
    {
      MFREE (SMS_SDU(sms_data));
      SMS_SDU(sms_data) = NULL;
    }
    SMS_SEL_REC(sms_data) = SMS_RECORD_NOT_EXIST;
   /*
    * TL State Transition TL_IDLE
    */
    SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
   /*
    * RL_RELEASE_REQ ==>
    */
    rl_release_req(SMS_INST.ti);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_cmms_end                |
+--------------------------------------------------------------------+

  PURPOSE : Function used for ending the CMMS Session

*/

GLOBAL void tl_cmms_end(void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_cmms_end()");

  sms_data->cmms_release_pending = FALSE;
  sms_timer_stop(TMMS);
  tl_mnsms_cmms_end_ind();
}

#endif /* #ifndef SMS_TLS_C */

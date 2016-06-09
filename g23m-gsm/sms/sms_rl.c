/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_RL
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
|  Purpose :  This Modul defines the functions for the relay layer
|             of the component SMS.
+----------------------------------------------------------------------------- 
*/   

#ifndef SMS_RL_C
#define SMS_RL_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SMS

/*==== INCLUDES ===================================================*/

#define SAP_RR    /* get RR causes until all causes are in one document */
#define SAP_MMCM  /* same reason */

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
#include "sms_em.h"

/*==== EXPORT ======================================================*/

/*==== PRIVAT ======================================================*/

/*==== VARIABLES ===================================================*/

/*==== FUNCTIONS ===================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the relay layer.

*/

GLOBAL void rl_init (void)
{
  GET_INSTANCE_DATA;    

  TRACE_FUNCTION ("rl_init()");

  sms_data->data[0].state[STATE_RL] = RL_IDLE;
  sms_data->data[1].state[STATE_RL] = RL_IDLE;
}

/*---- SIGNALS -----------------------------------------------------*/

#ifdef GPRS
GLOBAL void rl_proceed (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_proceed()");

  if (SMS_INST_GET_STATE (STATE_RL) EQ RL_WAIT_FOR_SEND_ERROR)
  {
    SMS_INST_SET_STATE (STATE_RL, RL_IDLE);

    tl_report_ind (NULL, CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                   SMSRP_ORIGINATING_ENTITY, SMS_RP_CS_PROTOCOL_ERROR));
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_data_ind                |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_DATA_IND.

*/

GLOBAL void rl_data_ind (T_cp_user_data_dl *cp_user_data_dl)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_data_ind()");

  if (sms_data)
  {
    if (cp_user_data_dl->rp_mti EQ RP_ERROR_DL)
      TRACE_EVENT_P1 ("RP_ERROR.CAUSE rcvd: 0x%2.2X",
                      (int)cp_user_data_dl->rp_error.rp_cause.rp_cause_value);

    switch (SMS_INST_GET_STATE (STATE_RL))
    {
   /* ------------------------------------- */
      case RL_ESTABLISHED:
   /* ------------------------------------- */
      case RL_IDLE:
   /* ------------------------------------- */
      {
        SMS_RP_REF(sms_data) = cp_user_data_dl->reference;
        if (cp_user_data_dl->rp_mti EQ RP_DATA_DL)
        {
         /*
          * RL state transition RL_WAIT_FOR_SEND_ACK
          */
          SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_SEND_ACK);
         /*
          * start timer TR2M
          */
          sms_timer_start(TR2M);

          SMS_EM_RECEIVE_RP_DATA;
         /*
          * TL_DATA_IND =>
          */
          tl_data_ind (&cp_user_data_dl->rp_data_dl);
        }
        else
        {
          MCAST (cp_data, U_CP_DATA);
          /*
           * If Message Id is unknwon then send RP-ERROR with cause #97
           */
          if(cp_user_data_dl->rp_mti > 0x06)
          {
            rl_build_rp_error (cp_user_data_dl->reference,
                               SMS_RP_CS_MSG_NON_EXIST,
                               cp_data, NULL);
          }
          else
          {
            rl_build_rp_error (cp_user_data_dl->reference,
                               SMS_RP_CS_MSG_NOT_COMP,
                               cp_data, NULL);
          }

          SMS_EM_SEND_RP_ERROR;

          cp_data_req (cp_data);
         /*
          * CP_RELEASE_REQ =>
          */
          TRACE_EVENT("CP_RELEASE_REQ_1");
      
          cp_release_req (SMS_INST.ti);
         /*
          * free instance
          */
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }
        break;
      }

   /* ------------------------------------- */
      case RL_WAIT_FOR_ACK:
   /* ------------------------------------- */

        TRACE_FUNCTION ("rl_data_ind(): RP_WAIT_FOR_ACK");

        switch (cp_user_data_dl->rp_mti)
        {
          case RP_ACK_DL:
            TRACE_FUNCTION ("rl_data_ind(): RP_ACK_DL");
           /*
            * stop timer TR1M
            */
            sms_timer_stop(TR1M);
           /*
            * RL state transition RL_IDLE
            */
            SMS_INST_SET_STATE (STATE_RL, RL_IDLE);

            /* 
             * if CMMS mode is 1 or 2 then only make cmm_release_pending TRUE
             */
            if(CMMS_ACTIVE)
            {
               sms_data->cmms_release_pending = TRUE;
            }

           /*
            * TL_REPORT_IND =>
            */
            tl_report_ind (((cp_user_data_dl->rp_ack.v_rp_user_data)?
                            &cp_user_data_dl->rp_ack.rp_user_data:
                            NULL), SMS_NO_ERROR);           

            /* Since the transmission is successful, the data stored for 
               retransmission can be freed */
            if (SMS_DATA_REQ(sms_data) NEQ NULL)
            {
              PFREE (SMS_DATA_REQ(sms_data));
              SMS_DATA_REQ(sms_data) = NULL;
            }

            SMS_EM_RECEIVE_RP_AKNOWLEDGE;

            break;

          case RP_ERROR_DL:
            /* When an RP-error with appropriate case value is 
            * is received, Retransmission of SMS is started
            */
          {
#ifdef REL99
            BOOL retrans_flag = FALSE;
#endif
            TRACE_FUNCTION ("rl_data_ind(): RP_ERROR_DL");
           /*
            * stop timer TR1M
            */
            sms_timer_stop(TR1M);
           /* 
            * if CMMS mode is 1 or 2 then only make cmm_release_pending TRUE
            */
            if(CMMS_ACTIVE)
            {
               sms_data->cmms_release_pending = TRUE;
            }
#ifdef REL99
            /*
             * This is done for Retransmission for the causes
             * listed in the table 8.4/3gPP TS 24.011(Part 1)
             */
            if (SMS_INST.tl_retx < TL_MAX_RETANS)
            {             
              /* Check whether the cause value satisfies the one for retransmission 
               *  listed in the table 8.4/3gPP TS 24.011(Part 1)
               */  
              switch(cp_user_data_dl->rp_error.rp_cause.rp_cause_value)
              {
                case SMS_RP_CS_UNASSIGNED_NUMBER:         
                case SMS_RP_CS_OPERATOR_DET_BARRED:       
                case SMS_RP_CS_CALL_BARRED:      
                case SMS_RP_CS_SM_TRANSFER_REJECTED:
                case SMS_RP_CS_MEM_CAP_EXCEEDED:
                case SMS_RP_CS_UNIDENT_SUBSCRIBER:
                case SMS_RP_CS_FACILITY_REJECTED:
                case SMS_RP_CS_UNKNOWN_SUBSCRIBER:
                case SMS_RP_CS_NET_OUT_OF_ORDER:
                case SMS_RP_CS_NO_RESOURCES:
                case SMS_RP_CS_FAC_NOT_SUBSCRIBED:
                case SMS_RP_CS_FAC_NOT_IMPL:
                case SMS_RP_CS_INV_SM_TR_REF_VAL:
                case SMS_RP_CS_SEM_INC_MSG:
                case SMS_RP_CS_INV_MAND_INFO:
                case SMS_RP_CS_MSG_NON_EXIST:
                case SMS_RP_CS_INFO_NON_EXIST:
                case SMS_RP_CS_PROTOCOL_ERROR:
                case SMS_RP_CS_INTERWORKING:
                  retrans_flag = FALSE;
                  break;
                /* All values other than the ones above are treated as temporary
                  failures. This includes SMS_RP_CS_DEST_OUT_OF_ORDER, 
                  SMS_RP_CS_TEMP_FAILURE, SMS_RP_CS_CONGESTION, 
                  SMS_RP_CS_MSG_NOT_COMP and others non-standard values
                  are taken as temporary error */
                default:
                  retrans_flag = TRUE;
                  break;
              }
            }
            if(retrans_flag EQ TRUE)
            {
              TRACE_EVENT ("Retransmission cause received");
              cp_release_req(SMS_INST.ti);
              TIMERSTART (SMS_INST_TR1M, 20000);
              TRACE_EVENT ("Delay expiry TR1M, retransmit");
            }
            else
#endif
            {
              /*
               * RL state transition RL_IDLE
               */
              SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
              tl_report_ind (((cp_user_data_dl->rp_error.v_rp_user_data)?
                            &cp_user_data_dl->rp_error.rp_user_data: NULL),
                           CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET, SMSRP_ORIGINATING_ENTITY,
                           cp_user_data_dl->rp_error.rp_cause.rp_cause_value));
               SMS_EM_RECEIVE_RP_ERROR;
             }
          }
          break;

          default:
            TRACE_FUNCTION ("rl_data_ind(): default");

           /*
            * stop timer TR1M
            */
            sms_timer_stop(TR1M);

            {
              MCAST (cp_data_ul, U_CP_DATA);
              /*
               * If Message Id is unknwon then send RP-ERROR with cause #97
               */
              if(cp_user_data_dl->rp_mti > 0x06)
              {
                rl_build_rp_error (cp_user_data_dl->reference,
                                   SMS_RP_CS_MSG_NON_EXIST,
                                   cp_data_ul, NULL);
              }
              else
              {
                rl_build_rp_error (cp_user_data_dl->reference,
                                   SMS_RP_CS_PROTOCOL_ERROR,
                                   cp_data_ul, NULL);
              }

              cp_data_req (cp_data_ul);
            }
#ifdef GPRS   /* interworking from flow control */
            if (SMS_LLC_FLOW(sms_data) NEQ SMS_LLC_BUSY_WAITING)
#endif
            {
              SMS_INST_SET_STATE (STATE_RL, RL_IDLE);

              /* 
               * if CMMS mode is 1 or 2 and current link used is GSM then only
               * make cmm_release_pending TRUE
               */
#ifdef GPRS
              if(CMMS_ACTIVE AND (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS))
              {
                 sms_data->cmms_release_pending = TRUE;
              }
#endif
              /*
              * TL_REPORT_IND =>
              */
             /*
              * If Message Id is unknwon then send TL REPORT IND with cause #97
              */
              if(cp_user_data_dl->rp_mti > 0x06)
              {
                tl_report_ind (NULL, CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                               SMSRP_ORIGINATING_ENTITY, SMS_RP_CS_MSG_NON_EXIST));
              }
              else
              {
                tl_report_ind (NULL, CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                               SMSRP_ORIGINATING_ENTITY, SMS_RP_CS_PROTOCOL_ERROR));
              }
             
            }
#ifdef GPRS
            else
            {
              SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_SEND_ERROR);
            }
#endif

            SMS_EM_RECEIVE_UNKNOWN_2;

            break;
        }

        break;

      case RL_WAIT_FOR_SEND_ACK:
        TRACE_FUNCTION ("rl_data_ind(): RP_WAIT_FOR_SEND_ACK");
        {
          MCAST (cp_data_ul, U_CP_DATA);
         /*
          * If Message Id is unknwon then send RP-ERROR with cause #97
          */
          if(cp_user_data_dl->rp_mti > 0x06)
          {
            rl_build_rp_error (cp_user_data_dl->reference,
                               SMS_RP_CS_MSG_NON_EXIST,
                               cp_data_ul, NULL);
          }
          else
          {
            rl_build_rp_error (cp_user_data_dl->reference,
                               SMS_RP_CS_PROTOCOL_ERROR,
                               cp_data_ul, NULL);
          }
          cp_data_req (cp_data_ul);

        }
        break;

   /* ------------------------------------- */
      case RL_WAIT_FOR_SMMA_ACK:
   /* ------------------------------------- */

        TRACE_FUNCTION ("rl_data_ind(): RP_WAIT_FOR_SMMA_ACK");

        switch (cp_user_data_dl->rp_mti)
        {
          case RP_ACK_DL:
            TRACE_FUNCTION ("rl_data_ind(): RP_ACK_DL");
           /*
            * stop timer TR1M
            */
            sms_timer_stop(TR1M);

            SMS_INST.retrans  = FALSE;
           /*
            * RL state transition RL_IDLE
            */
            SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
           /*
            * TL_REPORT_IND =>
            */
            tl_report_ind (NULL, SMS_NO_ERROR);
            break;

          case RP_ERROR_DL:
            TRACE_FUNCTION ("rl_data_ind(): RP_ERROR_DL");

            if (rl_temp_failure (cp_user_data_dl->rp_error.
                                 rp_cause.rp_cause_value))
            {
              if (SMS_INST.retrans)
              {
               /*
                * RL state transition RL_IDLE
                */
                SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
                SMS_INST.retrans = FALSE;

               /*
                * TL_REPORT_IND =>
                */
                tl_report_ind (NULL,
                               CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET, SMSRP_ORIGINATING_ENTITY,
                               cp_user_data_dl->rp_error.rp_cause.rp_cause_value));
                sms_timer_stop(TR1M);

                tl_sms_memo_exceeded (TRUE);
               /*
                * stop timer TR1M
                */
              }
              else
              {
                SMS_INST.retrans  = TRUE;
               /*
                * RL state transition RL_WAIT_FOR_RETRANS_TIMER
                */
                SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_RETRANS_TIMER);
               /*
                * stop timer TR1M
                */
                sms_timer_stop(TR1M);
               /*
                * start timer TRAM
                */
                sms_timer_start(TRAM);
               /*
                * CP_RELEASE_REQ =>
                */
                TRACE_EVENT("CP_RELEASE_REQ_11");
                cp_release_req (SMS_INST.ti);
        
              }
            }
            else
            {
             /*
              * stop timer TR1M
              */

              sms_timer_stop(TR1M);
              SMS_INST.retrans  = FALSE;
             /*
              * RL state transition RL_IDLE
              */
              SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
             /*
              * TL_REPORT_IND =>
              */
          
              tl_report_ind (NULL,
                             CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET, SMSRP_ORIGINATING_ENTITY,
                             cp_user_data_dl->rp_error.rp_cause.rp_cause_value));

              tl_sms_memo_exceeded (TRUE);
                  
            }
            break;

          default:
            TRACE_FUNCTION ("rl_data_ind(): default");
           /*
            * stop timer TR1M
            */
            sms_timer_stop(TR1M);

            SMS_INST.retrans  = FALSE;
           /*
            * RL state transition RL_IDLE
            */
            SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
           /*
            * If Message Id is unknwon then send RP-ERROR with cause #97
            */
            if(cp_user_data_dl->rp_mti > 0x06)
            {
              MCAST (cp_data, U_CP_DATA);
              rl_build_rp_error (cp_user_data_dl->reference,
                                 SMS_RP_CS_MSG_NON_EXIST,
                                 cp_data, NULL);
              SMS_EM_SEND_RP_ERROR;
              cp_data_req (cp_data);
              tl_report_ind (NULL, CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                             SMSRP_ORIGINATING_ENTITY, SMS_RP_CS_MSG_NON_EXIST));
            }
            break;
        }
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_establish_req           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_ESTABLISH_REQ.

*/

GLOBAL void rl_establish_req (UBYTE ti)
{
  GET_INSTANCE_DATA;
    TRACE_FUNCTION_P1 ("rl_establish_req(TI=%u)", ti);
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_IDLE:
       /*
        * RL state transition RL_ESTABLISH
        */
        SMS_INST_SET_STATE (STATE_RL, RL_ESTABLISH);
       /*
        * CP_ESTABLISH_REQ =>
        */
        cp_establish_req(ti);
       /*
        * start timer TR1M
        */
        sms_data->timer_ti = ti;
        sms_timer_start(TR1M);
        break;
      default:
        TRACE_ERROR("RL_ESTABLISH_REQ in wrong state received!");
   }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_establish_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_ESTABLISH_CNF.

*/

GLOBAL void rl_establish_cnf (BOOL         success)
{
   GET_INSTANCE_DATA;
    TRACE_FUNCTION ("rl_establish_cnf()");
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_ESTABLISH:
       /*
        * stop timer TR1M
        */
        sms_timer_stop(TR1M);
        if (success EQ TRUE)
        {
          /*
           * RL state transition RL_ESTABLISHED
           */
           SMS_INST_SET_STATE (STATE_RL, RL_ESTABLISHED);
        }
        else
        {
          /*
           * RL state transition RL_IDLE
           */
           SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
        }
       /*
        * TL_ESTABLISH_CNF =>
        */
        tl_establish_cnf(success);
        break;
      default:
        TRACE_ERROR("RL_ESTABLISH_CNF in wrong state received!");
   }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_release_req             |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_RELEASE_REQ.

*/

GLOBAL void rl_release_req ( UBYTE  ti)
{
    TRACE_FUNCTION_P1 ("rl_release_req(TI=%u)", ti);
   /*
    * CP_RELEASE_REQ =>
    */
    TRACE_EVENT("CP_RELEASE_REQ_13");
    cp_release_req(ti);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_data_req                |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_DATA_REQ.

*/
GLOBAL BOOL rl_data_req (UBYTE       msg_ref,
                         T_U_CP_DATA *cp_data)
{
  GET_INSTANCE_DATA;    
  TRACE_FUNCTION ("rl_data_req()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_ESTABLISHED:
      {
        SMS_INST.msg_ref  = msg_ref;
       /*
        * RL state transtion RL_WAIT_FOR_ACK
        */
        SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_ACK);

        SMS_EM_SEND_RP_DATA;

        cp_data->cp_user_data_ul.rp_mti = RP_DATA_UL;
        cp_data->cp_user_data_ul.reference = msg_ref;
       /*
        * CP_DATA_REQ =>
        */
        cp_data_req (cp_data);
       /*
        * start timer TR1M
        */
        sms_timer_start(TR1M);

        return TRUE;
      }
      default:
        TRACE_ERROR("RL_DATA_REQ in wrong state received!");
        break;
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_error_ind               |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_ERROR_IND.

*/

GLOBAL void rl_error_ind (USHORT     cause)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_error_ind()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
   /* --------------------------------- */
      case RL_ESTABLISH:
   /* --------------------------------- */

        if ( SMS_INST.tl_retx < TL_MAX_RETANS )
        {
          switch ( cause )
          {
            case MMCM_MMCS_MESSAGE_INCOMPAT:
            case RRCS_NORM:
              /* 
               * start timer TR1M with value
               */
              TIMERSTART ( SMS_INST_TR1M, 20000 );
              TRACE_EVENT ( "Delay expiry TR1M, retransmit" );
              return;

            default: /* No retransmission */
              break;
          }
        }

       /*
        * stop timer TR1M
        */
        sms_timer_stop(TR1M);
       /*FALLTHROUGH*/ /*lint -fallthrough*/
   /* --------------------------------- */
      case RL_ESTABLISHED:
   /* --------------------------------- */
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /*
        * free instance
        */
        FREE_SMS_INSTANCE (SMS_INST.ti);
       /*
        * TL_REPORT_IND
        */
        tl_report_ind (NULL, cause);
        break;

   /* --------------------------------- */
      case RL_WAIT_FOR_ACK:
   /* --------------------------------- */
        TRACE_FUNCTION ("rl_error_ind(): RL_WAIT_FOR_ACK");
        /* 
         * The following code is to implement a subset of 
         * 3GPP 23.040 Release 1999 subclause 9.2.3.6.
         * For certain errors the SMS SUBMIT / SMS COMMAND is repeated
         * after timeout of TR1M.
         */
        TRACE_EVENT_P2 ("TL retrans #%d, error_cs %04x",
                        SMS_INST.tl_retx,
                        cause);

        if (SMS_INST.tl_retx < TL_MAX_RETANS)
        {
          switch (cause)
          {
            case SMS_CAUSE_NET_TIMEOUT:
             /* 11.10 test case 34.2.2 step 68 */
             /*
              * start timer TR1M with value
              */
              TIMERSTART (SMS_INST_TR1M, 20000);
              TRACE_EVENT ("Delay expiry TR1M, retransmit");
              return;

            case RRCS_NORM:
            case MMCM_MMCS_MESSAGE_INCOMPAT: /* Make cingular happy, #19189 */
              TRACE_EVENT ("Retransmission after TR1M");
              return;

            default: /* No retransmission */
              break;
          }
        }
       /*
        * stop timer TR1M
        */
        sms_timer_stop(TR1M);
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /*
        * free instance
        */
        FREE_SMS_INSTANCE (SMS_INST.ti);
       /*
        * TL_REPORT_IND =>
        */
        tl_report_ind (NULL, cause);
        break;

   /* --------------------------------- */
      case RL_WAIT_FOR_SEND_ACK:
   /* --------------------------------- */

        TRACE_FUNCTION ("rl_error_ind(): RL_WAIT_FOR_SEND_ACK");
       /*
        * free instance
        */
        FREE_SMS_INSTANCE (SMS_INST.ti);
       /*
        * stop timer TR2M
        */
        sms_timer_stop(TR2M);
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /*
        * TL_REPORT_IND =>
        */
        tl_report_ind (NULL, cause);
        break;

   /* --------------------------------- */
        case RL_WAIT_FOR_SMMA_ACK:
   /* --------------------------------- */

        TRACE_FUNCTION ("rl_error_ind(): RL_WAIT_FOR_SMMA_ACK");
        
        // FREE_SMS_INSTANCE (SMS_INST.ti);

        if (SMS_INST.retrans)
        {
         /*
          * RL state transition RL_IDLE
          */
          SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
          SMS_INST.retrans = FALSE;
         /*
          * TL_REPORT_IND =>
          */
          tl_report_ind (NULL, SMS_CAUSE_NET_TIMEOUT);
         /*
          * stop timer TR1M
          */
          sms_timer_stop(TR1M);
          tl_sms_memo_exceeded (TRUE);

        }
        else
        {
          SMS_INST.retrans  = TRUE;
         /*
          * RL state transition RL_WAIT_FOR_RETRANS_TIMER
          */
          SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_RETRANS_TIMER);
         /*
          * stop timer TR1M
          */
          sms_timer_stop(TR1M);
         /*
          * start timer TRAM
          */
          sms_timer_start(TRAM);
         /*
          * CP_RELEASE_REQ =>
          */
          TRACE_EVENT("CP_RELEASE_REQ_15");
          cp_release_req (SMS_INST.ti);

         /*
          * free instance
          */
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }

        break;
    

      default:
        
        if(CMMS_ACTIVE)
        {
           tl_cmms_end();
        }
       /*
        * free instance
        */
        FREE_SMS_INSTANCE (SMS_INST.ti);
       /*
        * RL state transition RL_IDLE
        */
        
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_mem_avail_req           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_MEM_AVAIL_REQ.

*/

GLOBAL BOOL rl_mem_avail_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_mem_avail_req()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_ESTABLISHED:
      {
        CCD_START;
        {
          MCAST (cp_data, U_CP_DATA);
         /*
          * RL state transition RL_WAIT_FOR_SMMA_ACK
          */
          SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_SMMA_ACK);
          SMS_RP_REF(sms_data) = SMS_INST.tp_mr;

          rl_build_rp_smma (SMS_RP_REF(sms_data), cp_data);

          CCD_END;
         /*
          * CP_DATA_REQ =>
          */
          cp_data_req (cp_data);
        }
       /*
        * start timer TR1M
        */
        sms_timer_start(TR1M);    
        return TRUE;

      default:
        TRACE_ERROR("RL_MEM_AVAIL_REQ in wrong state received!");
        break;
      }
    }
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_start_tram_req          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_START_TRAM_REQ.

*/

GLOBAL void rl_start_tram_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_start_tram_req()");
          
  SMS_INST.retrans  = TRUE;
 /*
  * RL state transition RL_WAIT_FOR_RETRANS_TIMER
  */
  SMS_INST_SET_STATE (STATE_RL, RL_WAIT_FOR_RETRANS_TIMER);
 /*
  * start timer TRAM
  */
  sms_timer_start(TRAM);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_build_rp_ack            |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function RL_BUILD_RP_ACK.

*/

LOCAL void rl_build_rp_ack (UBYTE          msg_ref,
                            T_U_CP_DATA    *cp_data,
                            T_rp_user_data *rp_user_data)
{
  TRACE_FUNCTION ("rl_build_rp_ack()");

  memset (&cp_data->cp_user_data_ul.rp_ack, 0, sizeof (T_rp_ack));

  cp_data->msg_type                  = U_CP_DATA;
  cp_data->cp_user_data_ul.rp_mti    = RP_ACK_UL;
  cp_data->cp_user_data_ul.reference = msg_ref;

  cp_data->cp_user_data_ul.v_rp_data_ul  = FALSE;
  cp_data->cp_user_data_ul.v_rp_error    = FALSE;
  cp_data->cp_user_data_ul.v_rp_ack      = TRUE;

  if (rp_user_data NEQ NULL AND rp_user_data->v_tpdu)
  {
    memcpy (&cp_data->cp_user_data_ul.rp_ack.rp_user_data,
            rp_user_data, sizeof (T_rp_user_data));
    cp_data->cp_user_data_ul.rp_ack.v_rp_user_data = TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_report_req_ack          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_REPORT_REQ_ACK.

*/

GLOBAL void rl_report_req_ack (T_rp_user_data *rp_user_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_report_req_ack()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_WAIT_FOR_SEND_ACK:
      {
        MCAST (cp_data, U_CP_DATA);
       /*
        * stop timer TR2M
        */
        sms_timer_stop(TR2M);

        rl_build_rp_ack (SMS_RP_REF(sms_data), cp_data, rp_user_data);
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /*
        * CP_DATA / RP_ACK ==>
        */
        cp_data_req (cp_data);
       /*
        * CP_RELEASE_REQ =>
        */
        TRACE_EVENT("CP_RELEASE_REQ_16");
        cp_release_req (SMS_INST.ti);
        break;
      }
      default:
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_build_rp_error          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function RL_BUILD_RP_ERROR.

*/

GLOBAL void rl_build_rp_error (UBYTE          msg_ref,
                               USHORT         error,
                               T_U_CP_DATA    *cp_data,
                               T_rp_user_data *rp_user_data)
{
  TRACE_FUNCTION ("rl_build_rp_error()");
  TRACE_EVENT_P1 ("RP_ERROR.CAUSE sent: 0x%2.2X", (int)error);

  memset (&cp_data->cp_user_data_ul.rp_error, 0, sizeof (T_rp_error));

  cp_data->msg_type                  = U_CP_DATA;
  cp_data->cp_user_data_ul.rp_mti    = RP_ERROR_UL;
  cp_data->cp_user_data_ul.reference = msg_ref;

  cp_data->cp_user_data_ul.v_rp_data_ul  = FALSE;
  cp_data->cp_user_data_ul.v_rp_ack      = FALSE;
  cp_data->cp_user_data_ul.v_rp_error    = TRUE;

  cp_data->cp_user_data_ul.rp_error.
           rp_cause.v_rp_cause_value = TRUE;
  if (error < 128)              /* GSM 04.11 cause */
  {
    cp_data->cp_user_data_ul.rp_error.
             rp_cause.rp_cause_value = (UBYTE)error;
  }
  else
  {
    cp_data->cp_user_data_ul.rp_error.
             rp_cause.rp_cause_value = SMS_RP_CS_PROTOCOL_ERROR;
    cp_data->cp_user_data_ul.rp_error.
             v_rp_user_data = TRUE;
  }
  if (rp_user_data NEQ NULL AND rp_user_data->v_tpdu)
  {
    memcpy (&cp_data->cp_user_data_ul.rp_error.rp_user_data,
            rp_user_data, sizeof (T_rp_user_data));
    cp_data->cp_user_data_ul.rp_error.v_rp_user_data = TRUE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_report_req_error        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal RL_REPORT_REQ_ERROR.

*/

GLOBAL void rl_report_req_error (USHORT         cause,
                                 T_rp_user_data *rp_user_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("rl_report_req_error()");

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_WAIT_FOR_SEND_ACK:
      {
        MCAST (cp_data, U_CP_DATA);
       /*
        * stop timer TR2M
        */
        sms_timer_stop(TR2M);

        rl_build_rp_error (SMS_RP_REF(sms_data), cause,
                           cp_data, rp_user_data);
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /*
        * CP_DATA / RP_ERROR ==>
        */
        cp_data_req (cp_data);
        break;
      }
      default:
        break;
    }
  }
}

#if defined (GPRS)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_build_rp_error_gprs     |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function RL_BUILD_RP_ERROR for GSMS.

*/

GLOBAL void rl_build_rp_error_gprs (UBYTE              ti,
                                    T_LL_UNITDATA_REQ  *data_req,
                                    USHORT             error,
                                    UBYTE              msg_ref,
                                    T_U_CP_DATA        *cp_data,
                                    T_stk_cmd          *stk_cmd)
{
  TRACE_FUNCTION ("rl_build_rp_error_gprs()");
  TRACE_EVENT_P1 ("RP_ERROR.CAUSE sent: 0x%4.2X", (int)error);

  data_req->sdu.o_buf = ENCODE_OFFSET;

  cp_data->msg_type                  = U_CP_DATA;
  cp_data->cp_user_data_ul.rp_mti    = RP_ERROR_UL;
  cp_data->cp_user_data_ul.reference = msg_ref;

  cp_data->cp_user_data_ul.v_rp_data_ul  = FALSE;
  cp_data->cp_user_data_ul.v_rp_error    = TRUE;

  memset (&cp_data->cp_user_data_ul.rp_error.rp_cause, 0,
          sizeof (T_rp_cause));

  cp_data->cp_user_data_ul.rp_error.
           rp_cause.v_rp_cause_value = TRUE;
  if (error < 128)              /* GSM 04.11 cause */
  {
    cp_data->cp_user_data_ul.rp_error.
             rp_cause.rp_cause_value = (UBYTE)error;
    cp_data->cp_user_data_ul.rp_error.
             v_rp_user_data = FALSE;
  }
  else
  {
    cp_data->cp_user_data_ul.rp_error.
             rp_cause.rp_cause_value = SMS_RP_CS_PROTOCOL_ERROR;
    cp_data->cp_user_data_ul.rp_error.
             v_rp_user_data = TRUE;
    memset (&cp_data->cp_user_data_ul.rp_error.rp_user_data,
            0, sizeof (T_rp_user_data));
    if (error >= 256)           /* no GSM 03.40 cause */
      error = SMS_FCS_UNSPECIFIED;
    cp_data->cp_user_data_ul.rp_error.rp_user_data.
             v_tpdu = TRUE;
  /*
    cp_data->cp_user_data_ul.rp_error.rp_user_data.
             sms_deliver_rep_err.tp_fcs = (UBYTE)error;
   */
  }

  ccd_codeMsg (CCDENT_SMS,
               UPLINK,
               (T_MSGBUF *) &data_req->sdu,
               (UBYTE *) cp_data,
               NOT_PRESENT_8BIT);
  /*lint -e415 -e416 Likely creation/access of out-of-bounds pointer */
  data_req->sdu.buf[3] = (ti << 4) + PD_SMS;
  data_req->sdu.o_buf  = ENCODE_OFFSET - BSIZE_TI_PD;
  data_req->sdu.l_buf += BSIZE_TI_PD;

  if (stk_cmd NEQ NULL)
  {
    /*
     * response from a SIM Toolkit command
     */
    if (stk_cmd->l_cmd NEQ 0)
    {
      /*
       * response TPDU available
       */
      /* change length of rp-error */
      data_req->sdu.buf[5] += ((stk_cmd->l_cmd >> 3) + 2);
      /* add IEI for rp user data */
      data_req->sdu.buf[10] = 0x41;
      /* add length for rp user data */
      data_req->sdu.buf [11] = stk_cmd->l_cmd >> 3;
      /* add TPDU */
      memcpy (&data_req->sdu.buf [12],
              &stk_cmd->cmd[stk_cmd->o_cmd>>3],
              stk_cmd->l_cmd>>3);
      /* modify length of CP Data */
      data_req->sdu.l_buf += (stk_cmd->l_cmd + 16);
    }
  }
  /*lint +e415 +e416 Likely creation/access of out-of-bounds pointer */
}
#endif /* GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_build_rp_smma           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function RL_BUILD_RP_SMMA.

*/

GLOBAL void rl_build_rp_smma (UBYTE       msg_ref,
                              T_U_CP_DATA *cp_data)
{
  TRACE_FUNCTION ("rl_build_rp_smma()");

  cp_data->cp_user_data_ul.rp_mti    = RP_SMMA_UL;
  cp_data->cp_user_data_ul.reference = msg_ref;

  cp_data->cp_user_data_ul.v_rp_data_ul = FALSE;
  cp_data->cp_user_data_ul.v_rp_error = FALSE;
  cp_data->cp_user_data_ul.v_rp_ack = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_RL                     |
| STATE   : code                ROUTINE : rl_temp_failure            |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function RL_TEMP_FAILURE.
            This is valid for memory available notification attempt.
            [04.11 Table 8.4 part 3]
*/

GLOBAL UBYTE rl_temp_failure (UBYTE cause)
{
  TRACE_FUNCTION ("rl_temp_failure()");

  switch (cause)
  {
    case SMS_RP_CS_UNKNOWN_SUBSCRIBER:
    case SMS_RP_CS_FAC_NOT_IMPL:
    case SMS_RP_CS_SEM_INC_MSG:
    case SMS_RP_CS_INV_MAND_INFO:
    case SMS_RP_CS_MSG_NON_EXIST:
    case SMS_RP_CS_MSG_NOT_COMP:
    case SMS_RP_CS_INFO_NON_EXIST:
    case SMS_RP_CS_PROTOCOL_ERROR:
    case SMS_RP_CS_INTERWORKING:
      return FALSE; /* Non-temporary */

    default:
      return TRUE;  /* Temporary */
  }
}

#endif /* #ifndef SMS_RL_C */

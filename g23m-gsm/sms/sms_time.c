/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_TIME
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
|  Purpose :  This Modul defines the timer handling functions
|             for the component SMS of the mobile station
+----------------------------------------------------------------------------- 
*/ 

#ifndef SMS_TIM_C
#define SMS_TIM_C

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
#include "sms_em.h"

/*==== EXPORT ======================================================*/

/*==== PRIVAT ======================================================*/

/*==== VARIABLES ===================================================*/

/*==== FUNCTIONS ===================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SMS_TIM                  |
| STATE   : code                  ROUTINE : tim_exec_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : execute timeout

*/

GLOBAL void tim_exec_timeout (USHORT index)
{
  GET_INSTANCE_DATA;
  USHORT        timer;
  int           instance;

  /* TRACE_FUNCTION ("tim_exec_timeout()"); */

  timer    = index / MAX_SMS_CALLS;
  instance = index % MAX_SMS_CALLS;
  sms_data->inst = (UBYTE)instance;

  if (timer < MAX_SMS_TIMER)
  {

/* Implements Measure#36 */
#ifdef NCONFIG
    /* partab is not defined when NCONFIG is defined */
    TRACE_EVENT_P1 ("Expired TIMER: timer %d", timer);
#else /* not NCONFIG */
#ifdef OPTION_TIMER
    TRACE_EVENT_P1 ("Expired TIMER %s", partab[timer].keyword);
#endif
#endif /* NCONFIG */

    sms_data->data[instance].t_running[timer] = FALSE;

    switch (timer)
    {
      case TC1M:
        tim_tc1m ();
        break;
      case TR1M:
        tim_tr1m ();
        break;
      case TR2M:
        tim_tr2m ();
        break;
      case TRAM:
        tim_tram ();
        break;
      case TLCT:
        tim_tlct ();
        break;
      case TMMS:
        tim_tmms ();
        break;
      default: /* Not reached */
        break;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SMS_TIM                  |
| STATE   : code                  ROUTINE : tim_stop_timer           |
+--------------------------------------------------------------------+

  PURPOSE : stop timer

*/

GLOBAL void tim_stop_timer (USHORT index)
{
  GET_INSTANCE_DATA;
  USHORT        timer;
  USHORT        instance;

  /* TRACE_FUNCTION ("tim_stop_timer()"); */

  timer    = index / MAX_SMS_CALLS;
  instance = index % MAX_SMS_CALLS;

/* Implements Measure#36 */
#ifdef NCONFIG
    /* partab is not defined when NCONFIG is defined */
  TRACE_EVENT_P1 ("Stop TIMER: timer %d", timer);
#else /* not NCONFIG */
#ifdef OPTION_TIMER
  TRACE_EVENT_P1 ("Stop TIMER %s", partab[timer].keyword);
#endif
#endif /* NCONFIG */

  sms_data->data[instance].t_running[timer] = FALSE;
  TIMER_STOP (sms_handle, index);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : RR_TIM                   |
| STATE   : code                  ROUTINE : tim_start_timer          |
+--------------------------------------------------------------------+

  PURPOSE : start timer

*/

GLOBAL void tim_start_timer (USHORT index, T_TIME value)
{
  GET_INSTANCE_DATA;
  USHORT        timer;
  USHORT        instance;

  /* TRACE_FUNCTION ("tim_start_timer()"); */

  timer    = index / MAX_SMS_CALLS;
  instance = index % MAX_SMS_CALLS;

/* Implements Measure#36 */
#ifdef NCONFIG
    /* partab is not defined when NCONFIG is defined */
  TRACE_EVENT_P1 ("Start TIMER: timer %d", timer);
#else /* not NCONFIG */
#ifdef OPTION_TIMER
  TRACE_EVENT_P1 ("Start TIMER %s", partab[timer].keyword);
#endif
#endif /* NCONFIG */

  sms_data->data[instance].t_running[timer] = TRUE;
  TIMER_START (sms_handle, index, value);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tc1m                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TC1M

*/

GLOBAL void tim_tc1m (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tc1m()");

  SMS_EM_TIMEOUT_TC1M;

  if (sms_data) /*lint !e774 always True*/
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_WAIT_FOR_ACK:
#if defined (GPRS)
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
#endif
      {
        if (SMS_RETX(sms_data) >= MAX_RETRANS)
        {
          cp_send_release_req (SMS_INST.ti);

          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);

          SMS_INST.r_flag = FALSE;

          rl_error_ind (SMS_CAUSE_NET_TIMEOUT);

          /*FREE_SMS_INSTANCE (SMS_INST.ti);*/
          break;
        }
        else
        {
#if defined (GPRS)
          if (SMS_INST.downlink NEQ SMS_DOWNLINK_LL)
          {
#endif
            USHORT bsize_message = D_SDU_OFF (SMS_DATA_REQ(sms_data))+
                                   D_SDU_LEN (SMS_DATA_REQ(sms_data));
            PALLOC_SDU (data_req, MMSMS_DATA_REQ, bsize_message );

            SMS_SDU_COPY (data_req, SMS_DATA_REQ(sms_data), MMSMS_DATA_REQ);
            TRACE_BINDUMP(sms_handle,
                          TC_USER4,
                          "U_CP_DATA",
                          (&(data_req->sdu.buf[0]) + ((data_req->sdu.o_buf >> 3) -1)),
                          ((data_req->sdu.l_buf >> 3) + 1));
            PSENDX (MM, data_req);

#if defined (GPRS)
          }
          else
          {
            if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
            {
              /*
               * retransmit on downlink LL
               */
              cp_send_data_gsms ();

              SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
            }
            else
            {
              SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
            }
          }
#endif /* GPRS */
          SMS_RETX(sms_data)++;
         /*
          * start timer TC1M
          */
          sms_timer_start(TC1M);
          break;
        }
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tr1m                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TR1M

*/

GLOBAL void tim_tr1m (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tr1m()");
        
  SMS_EM_TIMEOUT_TR1M;

  if (sms_data) /*lint !e774 always True*/
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_ESTABLISH:

        TRACE_FUNCTION ("tim_tr1m: RL_ESTABLISH");
        sms_data->data[INST_MO].ti = sms_data->timer_ti;
       /*
        * RL state transition
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /* 
        * CP_ABORT_REQ
        */
        cp_abort_req();
       /*
        * TL_TIMEOUT_IND
        */
        tl_timeout_ind (TR1M);
        break;

      case RL_WAIT_FOR_ACK:

        TRACE_FUNCTION ("tim_tr1m: RL_WAIT_FOR_ACK");
       /*
        * RL state transition
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
       /* 
        * CP_ABORT_REQ
        */
        cp_abort_req ();
       /*
        * TL_TIMEOUT_IND
        */
        tl_timeout_ind (TR1M);

        break;

      case RL_WAIT_FOR_SMMA_ACK:
        TRACE_FUNCTION ("tim_tr1m: RL_WAIT_FOR_SMMA_ACK");

        if (SMS_INST.retrans)
        {
          SMS_INST.retrans = FALSE;
         /*
          * RL state transition RL_IDLE
          */
          SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
         /*
          * TL_REPORT_IND =>
          */	  
          tl_report_ind (NULL, SMS_CAUSE_NET_TIMEOUT);
         /*
          * SIM_UPDATE_REQ =>
          */
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
          * start timer TRAM
          */
          sms_timer_start(TRAM);
         /*
          * CP_RELEASE_REQ =>
          */
          TRACE_EVENT("CP_RELEASE_REQ_19");
          cp_release_req (SMS_INST.ti);
        }

        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tr2m                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TR2M

*/

GLOBAL void tim_tr2m (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tr2m");

  SMS_EM_TIMEOUT_TR2M;

  if (sms_data) /*lint !e774 always True*/
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
      case RL_WAIT_FOR_SEND_ACK:
       /*
        * RP_ERROR =>
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, NULL);
       /*
        * TL_REPORT_IND =>
        */
        tl_report_ind (NULL, SMS_CAUSE_NET_TIMEOUT);
       /*
        * RL state transition RL_IDLE
        */
        SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
        break;

      default:
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tram                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TRAM

*/

GLOBAL void tim_tram (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tram");

  if (sms_data) /*lint !e774 always True*/
  {
    switch (SMS_INST_GET_STATE (STATE_RL))
    {
    case RL_WAIT_FOR_RETRANS_TIMER:
      /*
       * RL state transition RL_IDLE
       */
       SMS_INST_SET_STATE (STATE_RL, RL_IDLE);
      /*
       * TL_TIMEOUT_IND
       */
       tl_timeout_ind (TRAM);
       break;
    default:
       TRACE_ERROR("TRAM expired in wrong state!");
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tlct                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TLCT - concatenation supervision timer

*/

GLOBAL void tim_tlct (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tlct()");

  switch (SMS_INST_GET_STATE (STATE_TL))
  {
  default:

     TRACE_ERROR("Timer TLCT in wrong state");

  case TL_SEND_CONTD:
    /*
     * reset concatenation
     */
     sms_data->concat_cntrl.concatenation   = FALSE;
     sms_data->concat_cntrl.release_pending = FALSE;
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
     * TL State Transition TL_IDLE
     */
     SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
    /*
     * RL_RELEASE_REQ ==>
     */
     rl_release_req(SMS_INST.ti);
     break;

  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : tim_tmms                   |
+--------------------------------------------------------------------+

  PURPOSE : Timeout of timer TMMS - More Message Send timer

*/
GLOBAL void tim_tmms(void)
{

  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tim_tmms()");

  switch (SMS_INST_GET_STATE (STATE_TL))
  {
     default:

     TRACE_ERROR("Timer TMMS in wrong state");

     case TL_SEND_CONTD:
        
        if(CMMS_ACTIVE)
        {
           sms_data->cmms_release_pending = FALSE;
        }
        /*
         * Send the Primitive MNSMS_REPORT_IND to ACI
         */
        tl_mnsms_cmms_end_ind();
        /*
         * TL State Transition TL_IDLE
         */
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        /*
         * RL_RELEASE_REQ ==>
         */
        rl_release_req(SMS_INST.ti);
        break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : sms_timer_start            |
+--------------------------------------------------------------------+

  PURPOSE : start sms timer

*/
GLOBAL void sms_timer_start (USHORT id)
{
  GET_INSTANCE_DATA;
  T_TIME value;

  value = sms_data->timer_values[id];

  tim_start_timer (((USHORT)(id*MAX_SMS_CALLS+sms_data->inst)), value);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : sms_timer_stop             |
+--------------------------------------------------------------------+

  PURPOSE : stop sms timer

*/
GLOBAL void sms_timer_stop (USHORT id)
{
  GET_INSTANCE_DATA;
  tim_stop_timer (((USHORT)(id*MAX_SMS_CALLS+sms_data->inst)));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TIM                    |
| STATE   : code                ROUTINE : sms_timer_check            |
+--------------------------------------------------------------------+

  PURPOSE : check sms timer whether currently running
            returns TRUE if given timer running

*/
GLOBAL BOOL sms_timer_check (USHORT id)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("sms_timer_check()");

  return (SMS_INST.t_running[id]);
}

#endif /* #ifndef SMS_TIM_C */

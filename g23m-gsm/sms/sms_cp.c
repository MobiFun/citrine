/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_CP
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
|  Purpose :  This Modul defines the functions for the control protocol
|             of the component SMS.
+-----------------------------------------------------------------------------
*/

#ifndef SMS_CP_C
#define SMS_CP_C

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
  
/* Implements Measure# 4 */  
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                         |
| STATE   : code                ROUTINE : cp_send_err_ind_errcs          |
+------------------------------------------------------------------------+  

  PURPOSE : This routine process the signal CP_DATA_IND_CP_ERROR for the 
            cases CP_MM_CONNECTION_ESTABLISHED and  CP_WAIT_FOR_ACK    
            
*/

LOCAL void cp_send_err_ind_errcs(UBYTE errcs)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("cp_send_err_ind_errcs()");
  /*
   * Check if the cmms_mode is enabled or not
   * If enabled dont release the MM connection
   */
  if(!CMMS_ACTIVE)
  {
    cp_send_release_req (SMS_INST.ti); 
    SMS_INST_SET_STATE (STATE_CP, CP_IDLE); 
    SMS_INST.r_flag = FALSE; 
  }
  else
  {
    sms_data->cmms_release_pending = TRUE;
  }
 
  rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET,
                SMSCP_ORIGINATING_ENTITY, errcs));  
  
}

/* Implements Measure# 3 */
/*
+------------------------------------------------------------------------- -+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                            |
| STATE   : code                ROUTINE : cp_send_err_ind_msg_type_unknown  |
+---------------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_IND_CP_UNKNOWN for the 
            cases CP_MM_CONNECTION_ESTABLISHED and CP_WAIT_FOR_ACK
            
*/

LOCAL void cp_send_err_ind_msg_type_unknown(void)
{
  GET_INSTANCE_DATA; 
  TRACE_FUNCTION("cp_send_err_ind_msg_type_unknown()");
  
  cp_build_cp_error (SMS_CP_CS_MSG_TYPE_NON_EXIST);
  
  /*
   * Check if the cmms_mode is enabled or not
   * If enabled dont release the MM connection
   */
  if(!CMMS_ACTIVE)
  {
    cp_send_release_req (SMS_INST.ti);
    SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
    SMS_INST.r_flag = FALSE;
  }
  else
  {
    sms_data->cmms_release_pending = TRUE;
  }

  rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                SMSCP_ORIGINATING_ENTITY, SMS_CP_CS_MSG_TYPE_NON_EXIST));  
  
}

/* Implements Measure# 8 */
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                         |
| STATE   : code                ROUTINE : cp_send_err_ind_msg_not_comp   |
+------------------------------------------------------------------------+  

  PURPOSE : This routine process the signals CP_DATA_IND_CP_ACK and CP_DATA_IND_CP_DATA
            for the cases CP_MM_CONNECTION_ESTABLISHED and CP_WAIT_FOR_ACK
            
*/

LOCAL void cp_send_err_ind_msg_not_comp (void)
{
  GET_INSTANCE_DATA; 
  TRACE_FUNCTION("cp_send_err_ind_msg_not_comp()");
  
  cp_build_cp_error (SMS_CP_CS_MSG_NOT_COMP);
  
  cp_send_release_req (SMS_INST.ti);
  SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
  SMS_INST.r_flag = FALSE;
  
  rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                SMSCP_ORIGINATING_ENTITY, SMS_CP_CS_MSG_NOT_COMP));  
  
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the control protocol.

*/

GLOBAL void cp_init (void)
{
  GET_INSTANCE_DATA;	

  TRACE_FUNCTION ("cp_init()");

  memset (sms_data, 0, sizeof (T_SMS_DATA));

  sms_data->data[0].state[STATE_CP] = CP_IDLE;
  sms_data->data[1].state[STATE_CP] = CP_IDLE;

  /*
   * Initialize the timer_values with default values
   */
  sms_data->timer_values[TC1M] = TC1M_VALUE;
  sms_data->timer_values[TR1M] = TR1M_VALUE;
  sms_data->timer_values[TR2M] = TR2M_VALUE;
  sms_data->timer_values[TRAM] = TRAM_VALUE;
  sms_data->timer_values[TLCT] = TLCT_VALUE;
  sms_data->timer_values[TMMS] = TMMS_VALUE;

#if defined (GPRS)
  {
    int i;
    /*
     * default for MO SM: CCT preferred,
     * no LLC flow information known yet
     */
    SMS_ROUTE_PREF(sms_data) = GPRS_SMS_CCT_PREF;
    SMS_LLC_FLOW(sms_data)   = SMS_LLC_UNKNOWN;
    SMS_SMS_FLOW(sms_data)   = SMS_FLOW_UNKNOWN;

    /*
     * initialize downlink info of each instance
     */
    for (i = 0; i < MAX_SMS_CALLS; i++)
    {
      sms_data->data[i].downlink = SMS_DOWNLINK_NONE;
      sms_data->data[i].ack_type = SMS_CP_NONE;
      sms_data->data[i].cp_cause = NOT_PRESENT_8BIT;
      sms_data->data[i].cp_user_data_dl = NULL;
    }
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_add_pd_ti               |
+--------------------------------------------------------------------+

  PURPOSE : Add Protocol Discriminator and Transaction Identifier to
            the SDU and modify its parameters accordingly.
*/
void cp_add_pd_ti (UBYTE ti,
                   T_sdu *sdu)
{
  if (sdu NEQ NULL)
  {
    sdu->o_buf -= BSIZE_TI_PD;
    sdu->l_buf += BSIZE_TI_PD;
    sdu->buf[BYTELEN_POS(sdu->o_buf)] = PD_SMS | (ti << 4);

    memset (sdu->buf, 0, BYTELEN(sdu->o_buf));
  }
}

#if defined (GPRS)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_init_ll_unitdata_req    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize a ll_unitdata_req.

*/
void cp_init_ll_unitdata_req (T_LL_UNITDATA_REQ * unitdata_req)
{
  unitdata_req->sapi              = LL_SAPI_7;
  unitdata_req->tlli              = LL_TLLI_INVALID;
  unitdata_req->ll_qos.delay      = LL_DELAY_SUB;
  unitdata_req->ll_qos.relclass   = LL_RLC_PROT;
  unitdata_req->ll_qos.peak       = LL_PEAK_SUB;
  unitdata_req->ll_qos.preced     = LL_PRECED_SUB;
  unitdata_req->ll_qos.mean       = LL_MEAN_SUB;
  unitdata_req->radio_prio        = LL_RADIO_PRIO_1;
  unitdata_req->cipher            = LL_CIPHER_OFF;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_send_getunitdata_req    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize ready to receive protocol.

*/

GLOBAL void cp_send_getunitdata_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_send_getunitdata_req()");

  if (SMS_SMS_FLOW(sms_data) NEQ SMS_FLOW_AVAILABLE)
  {
    PALLOC (get_unitdata_req, LL_GETUNITDATA_REQ);

    memset (get_unitdata_req, 0, sizeof (T_LL_GETUNITDATA_REQ));
    get_unitdata_req->sapi = LL_SAPI_7;
    get_unitdata_req->tlli = LL_TLLI_INVALID;
    SMS_SMS_FLOW(sms_data) = SMS_FLOW_AVAILABLE;

    PSENDX (LLC, get_unitdata_req);
  }
}
#endif /* GPRS */

/*---- PRIMITIVES --------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_mmsms_error_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMSMS_ERROR_IND.

*/

GLOBAL void cp_mmsms_error_ind (T_MMSMS_ERROR_IND *error_ind)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (error_ind->ti);

  TRACE_FUNCTION_P1 ("cp_mmsms_error_ind(TI=%u)", error_ind->ti);

  if (sms_data)
  {
    TRACE_EVENT_P1 ("MMSMS_ERROR_IND.CAUSE: 0x%4.4hX", error_ind->cause);

    SMS_EM_LOSS_OF_MM_CONNECTION;

    cp_send_release_req (error_ind->ti);

    switch (SMS_INST_GET_STATE (STATE_CP))
    {
    case CP_MM_CONNECTION_PENDING:
      {
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.ti = error_ind->ti;

        /* This is meant for SMS over GSM as preference and GPRS as fallback.
         */

#ifdef GPRS
        if(sms_data->mo_dst_pref EQ GPRS_SMS_CCT_PREF)
        {
          TRACE_FUNCTION ("GSM Failed trying with GPRS ");
          rl_establish_cnf(FALSE);
        }
        else
#endif
        {
          SMS_INST.r_flag = FALSE;
          /*
           * RL_ERROR_IND =>
           */
          rl_error_ind (error_ind->cause);
        }
        break;
      }

    case CP_WAIT_FOR_ACK:
     /*
      * stop timer TC1M
      */
      sms_timer_stop(TC1M);

      SMS_RETX(sms_data)   = 0;
      SMS_INST.r_flag = FALSE;
      SMS_INST.ti = error_ind->ti;
    
      /*FALLTHROUGH*/ /*lint -fallthrough*/

    case CP_MM_CONNECTION_ESTABLISHED:

      SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
      rl_error_ind (error_ind->cause);
      break;

    default:
      SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
      FREE_SMS_INSTANCE (error_ind->ti);
      break;
    }
  }
  PFREE (error_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_mmsms_establish_cnf     |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMSMS_ESTABLISH_CNF.

*/

GLOBAL void cp_mmsms_establish_cnf (T_MMSMS_ESTABLISH_CNF *establish_cnf)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (establish_cnf->ti);

  TRACE_FUNCTION_P1 ("cp_mmsms_establish_cnf(TI=%u)", establish_cnf->ti);

  SMS_EM_MM_CONNECTION_ESTABLISHED;

  if (sms_data)
  {
   /*
    * set TI
    */
    sms_data->data[INST_MO].ti = establish_cnf->ti;
   /*
    * reset ack pending
    */
    SMS_INST.cp_ack_pending = FALSE;
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_MM_CONNECTION_PENDING:
      {
        /*
         * CP state transition
         */
         SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_ESTABLISHED);
        /*
         * RL_ESTABLISH_CNF =>
         */
         rl_establish_cnf(TRUE);
         break;
      }
      default:
        TRACE_ERROR("MMSMS_ESTABLISH_CNF in wrong state received!");
        break;
    }
  }
  PFREE (establish_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_mmsms_release_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMSMS_RELEASE_IND.

*/

GLOBAL void cp_mmsms_release_ind (T_MMSMS_RELEASE_IND *release_ind)
{
  register T_SMS_DATA *sms_data = GET_SMS_INSTANCE (release_ind->ti);

  TRACE_FUNCTION_P1 ("cp_mmsms_release_ind(TI=%u)", release_ind->ti);

  if (sms_data)
  {
    TRACE_EVENT_P1 ("MMSMS_RELEASE_IND: cause=0x%4.4hX", 
                    release_ind->cause);

    SMS_EM_MM_CONNECTION_FAILED;

    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_MM_CONNECTION_PENDING:
   /* ------------------------------------ */
      {
        /*
         * safe TI
         */
        SMS_INST.ti = release_ind->ti;
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);

        /* This is meant for SMS over GSM as preference and GPRS as fallback.
         */

#ifdef GPRS
        if(sms_data->mo_dst_pref EQ GPRS_SMS_CCT_PREF)
        {
           TRACE_FUNCTION ("GSM Failed trying with GPRS ");
           rl_establish_cnf(FALSE);
        }
        else
#endif
        {
           SMS_INST.r_flag = FALSE;
           /* RL_ERROR_IND */
           rl_error_ind (release_ind->cause);
         }
         break;
   	}

   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
       /*
        * CP state transition CP_IDLE
        */
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;
       /*
        * RL_ERROR_IND =>
        */
        rl_error_ind (release_ind->cause);
        break;
      }
   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
       /*
        * CP state transition CP_IDLE
        */
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_RETX(sms_data)   = 0;
        SMS_INST.r_flag = FALSE;
       /*
        * RL_ERROR_IND =>
        */
        rl_error_ind (release_ind->cause);
        break;
      }
   /* ------------------------------------ */
      default:
   /* ------------------------------------ */
       /*
        * CP state transition CP_IDLE
        */
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        FREE_SMS_INSTANCE (release_ind->ti);
        break;
    }    
  }
  PFREE (release_ind);
}

/*---- SIGNALS -----------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_abort_req               |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_ABORT_REQ.

*/

GLOBAL void cp_abort_req (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_abort_req(TI=%u)",SMS_INST.ti);


  SMS_EM_ABORT_OF_MM_CONNECTION;

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
        CCD_START;
       /*
        * check whether CP ACK needs to be send
        */
        if (SMS_INST.cp_ack_pending EQ TRUE)
        {
          cp_build_cp_ack ();
        }
       /*
        * generate CP_ERROR
        */
        cp_build_cp_error (SMS_CP_CS_PROTOCOL_ERROR);
       /*
        * release connection
        */
        SMS_INST.r_flag = FALSE;
        cp_send_release_req (SMS_INST.ti);
        CCD_END;
        break;
      }

   /* ------------------------------------ */
      case CP_MM_CONNECTION_PENDING:
   /* ------------------------------------ */
      {
       /*
        * release connection
        */
        SMS_INST.r_flag = FALSE;
        cp_send_release_req (SMS_INST.ti);
        break;
      }

   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        CCD_START;
       /*
        * check whether CP ACK needs to be send
        */
        if (SMS_INST.cp_ack_pending EQ TRUE)
        {
          cp_build_cp_ack ();
        }
       /*
        * generate CP_ERROR
        */
        cp_build_cp_error (SMS_CP_CS_PROTOCOL_ERROR);
       /*
        * release connection
        */
        SMS_INST.r_flag = FALSE;
        cp_send_release_req (SMS_INST.ti);

        CCD_END;
        break;
      }

#if defined (GPRS)
   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        cp_error_req_gsms (SMS_CP_CS_PROTOCOL_ERROR, FALSE);
        break;
      }
#endif /* GPRS */
    }
#if defined (GPRS)
    if (SMS_CP_ACK_TYPE(sms_data) EQ SMS_CP_NONE)
#endif /* GPRS */
    {
      SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_ind_cp_ack         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_IND_CP_ACK.

*/

GLOBAL void cp_data_ind_cp_ack (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_data_ind_cp_ack(TI=%u)", SMS_INST.ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
        /* Implements Measure# 8 */
        cp_send_err_ind_msg_not_comp ();  
        break;
      }

   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
          sms_timer_stop(TC1M);


        if (SMS_INST.r_flag)
        {
          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
          SMS_INST.r_flag   = FALSE;

          cp_send_release_req (SMS_INST.ti);
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }
        else
        {
          SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_ESTABLISHED);
        }
        break;
      }

#if defined (GPRS)
    /* ------------------------------------ */
     case CP_GSMS_IDLE:
   /* ------------------------------------ */
      {
        if (SMS_INST.r_flag)
        {
          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
          SMS_INST.r_flag   = FALSE;
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }
        else
        {
          cp_error_req_gsms (SMS_CP_CS_MSG_NOT_COMP, TRUE);
        }
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        if (SMS_INST.r_flag)
        {
          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
          SMS_INST.r_flag   = FALSE;
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }
        else
        {
          SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MO_WAIT_FOR_CP_DATA);
        }
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        if (SMS_INST.r_flag)
        {
          SMS_INST.r_flag   = FALSE;
        }
        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
   /* ------------------------------------ */
      {
        cp_error_req_gsms (SMS_CP_CS_MSG_NOT_COMP, FALSE);
        rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                                SMSCP_ORIGINATING_ENTITY, SMS_CP_CS_MSG_NOT_COMP));

        break;
      }
#endif /* GPRS */
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_ind_cp_data        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_IND_CP_DATA.

*/
GLOBAL void cp_data_ind_cp_data (T_D_CP_DATA *cp_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_data_ind_cp_data(TI=%u)", SMS_INST.ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
        if (SMS_INST.r_flag)
        {
          /* Nothing expected anymore */

         /*
          * stop timer TC1M
          */
          sms_timer_stop(TC1M);
          /* Implements Measure# 8 */
          cp_send_err_ind_msg_not_comp ();  
          break;
        }
        else
        {
          TRACE_EVENT ("CP ACK lost");
          cp_data_ind_cp_ack ();
        }
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/

   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
       /*
        * remember to cp ack
        */
        SMS_INST.cp_ack_pending = TRUE;
       /*
        * RL_DATA_IND =>
        */
        rl_data_ind (&cp_data->cp_user_data_dl);
        break;
      }

#if defined (GPRS)
   /* ------------------------------------ */
      case CP_GSMS_IDLE:
   /* ------------------------------------ */
      {
        if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
        {
          cp_build_cp_ack ();
        }
        else
        {
          SMS_CP_ACK_TYPE(sms_data) = SMS_CP_ACK;
          SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
        }
        SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MT_WAIT_FOR_RP_ACK);
        rl_data_ind (&cp_data->cp_user_data_dl);
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
   /* ------------------------------------ */
      {
        if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
        {
         /* 
          * generate CP_ACK
          */
          cp_build_cp_ack ();
         /*
          * RL_DATA_IND =>
          */
          rl_data_ind (&cp_data->cp_user_data_dl);
        }
        else
        {
          if (SMS_CP_UDL(sms_data) EQ NULL)
          {
            MALLOC(SMS_CP_UDL(sms_data), sizeof(T_cp_user_data_dl));
          }
          memcpy (SMS_CP_UDL(sms_data), &cp_data->cp_user_data_dl,
                  sizeof(T_cp_user_data_dl));
          SMS_CP_ACK_TYPE(sms_data) = SMS_CP_ACK;
          SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
          SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
          SMS_INST.r_flag = TRUE;
        }
        break;
      }
   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        cp_error_req_gsms (SMS_CP_CS_MSG_NOT_COMP, FALSE);

        rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                                SMSCP_ORIGINATING_ENTITY, SMS_CP_CS_MSG_NOT_COMP));
        break;
      }
#endif /* GPRS */
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_ind_cp_error       |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_IND_CP_ERROR.
            The cause value is marked as CP-ERROR
*/
GLOBAL void cp_data_ind_cp_error (UBYTE      errcs)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_data_ind_cp_error(TI=%u)", SMS_INST.ti);
  TRACE_EVENT_P1 ("CP Error Cause = %d", (int)errcs);

  if (sms_data)
  {
    TRACE_EVENT_P1 ("CP_ERROR.CAUSE rcvd: 0x%2.2X", (int)errcs);

    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
        /* Implements Measure# 4 */      
        cp_send_err_ind_errcs(errcs);
        break;
      }

   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        /* Implements Measure# 4 */
        cp_send_err_ind_errcs(errcs);
        break;
      }

#if defined (GPRS)
   /* ------------------------------------ */
      case CP_GSMS_IDLE:
   /* ------------------------------------ */
      {
        SMS_INST.r_flag = FALSE;
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);

        /* FREE_SMS_INSTANCE (SMS_INST.ti); */
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
   /* ------------------------------------ */
      {
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;

        rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET,
                                SMSCP_ORIGINATING_ENTITY, errcs));
        /* FREE_SMS_INSTANCE (SMS_INST.ti); */
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);

        SMS_INST.r_flag = FALSE;
        rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_NET,
                      SMSCP_ORIGINATING_ENTITY, errcs));

        /*FREE_SMS_INSTANCE (SMS_INST.ti);*/
        break;
      }
#endif /* GPRS */

    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_ind_cp_unknown     |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_IND_CP_UNKNOWN.

*/
GLOBAL void cp_data_ind_cp_unknown (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_data_ind_cp_unknown(TI=%u)", SMS_INST.ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
   /* ------------------------------------ */
      case CP_MM_CONNECTION_ESTABLISHED:
   /* ------------------------------------ */
      {
        /* Implements Measure# 3 */
        cp_send_err_ind_msg_type_unknown();  
        break;
      }

   /* ------------------------------------ */
      case CP_WAIT_FOR_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
        /* Implements Measure# 3 */  
        cp_send_err_ind_msg_type_unknown();  
        break;
      }

#if defined (GPRS)
   /* ------------------------------------ */
      case CP_GSMS_IDLE:
   /* ------------------------------------ */
      {
        cp_error_req_gsms (SMS_CP_CS_MSG_TYPE_NON_EXIST, TRUE);
        break;
      }

   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
   /* ------------------------------------ */
      {
       /*
        * stop timer TC1M
        */
        sms_timer_stop(TC1M);
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/
   /* ------------------------------------ */
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
   /* ------------------------------------ */
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
   /* ------------------------------------ */
      {
        cp_error_req_gsms (SMS_CP_CS_MSG_TYPE_NON_EXIST, FALSE);
        rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                                SMSCP_ORIGINATING_ENTITY, SMS_CP_CS_MSG_TYPE_NON_EXIST));

        break;
      }
#endif /* GPRS */
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_establish_req           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_ESTABLISH_REQ.

*/

GLOBAL void cp_establish_req (UBYTE ti)
{
  GET_INSTANCE_DATA;
    TRACE_FUNCTION_P1 ("cp_establish_req(TI=%u)",ti);

#if defined (GPRS)
    /*
     * send data on selected downlink
     */
    if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
    {
#endif /* GPRS */

     /*
      * GSM - establish connection
      */
      switch (SMS_INST_GET_STATE (STATE_CP))
      {
     /* ------------------------------------ */
        case CP_IDLE:
     /* ------------------------------------ */
        default:
     /* ------------------------------------ */
        {
          PALLOC (establish_req, MMSMS_ESTABLISH_REQ);

          SMS_EM_MM_CONNECTION_ESTABLISHMENT;

          establish_req->ti = ti;

          SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_PENDING);

          PSENDX (MM, establish_req);
          break;
        }
     /* ------------------------------------ */
        //default:
     /* ------------------------------------ */
        //   TRACE_ERROR("CP_ESTABLISH_REQ in wrong state received!");
      }
#if defined (GPRS)
    }
    else if (SMS_INST.downlink EQ SMS_DOWNLINK_LL_CHECK)
    {
     /*
      * GMMSMS_REG_STATE_REQ ==>
      */
      PALLOC (reg_state_req, GMMSMS_REG_STATE_REQ);
      PSENDX (GMM, reg_state_req);
     /*
      * CP state transition
      */
      SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_PENDING);
     /*
      * set TI
      */
      sms_data->data[INST_MO].ti = ti;
    }
    else
    {
     /*
      * set TI
      */
      sms_data->data[INST_MO].ti = ti;
     /*
      * GPRS - do nothing
      */
      rl_establish_cnf(TRUE); /* synchronous call back */
    }
#endif /* GPRS */

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_req                |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_REQ.

*/

GLOBAL void cp_data_req (T_U_CP_DATA *cp_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_data_req(TI=%u)",SMS_INST.ti);

  if (sms_data)
  {
    /*
     * U_CP_DATA contains a maximum of 252 Bytes
     */
    PALLOC_SDU (data_req, MMSMS_DATA_REQ, LEN_U_CP_DATA);

    data_req->sdu.o_buf = ENCODE_OFFSET;

    cp_data->msg_type = U_CP_DATA;

    ccd_codeMsg (CCDENT_SMS,
                 UPLINK,
                 (T_MSGBUF *)&data_req->sdu,
                 (UBYTE *)cp_data,
                 NOT_PRESENT_8BIT);

    cp_add_pd_ti (SMS_CP_REF(sms_data), &data_req->sdu);
#if defined (GPRS)
    /*
     * send data on selected downlink
     */
    if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
    {
#endif /* GPRS */

      switch (SMS_INST_GET_STATE (STATE_CP))
      {
     /* ------------------------------------ */
        case CP_MM_CONNECTION_ESTABLISHED:
     /* ------------------------------------ */
        {
          USHORT bsize_message = D_SDU_OFF (data_req)+
                                 D_SDU_LEN (data_req);
          PALLOC_SDU (tx_data_req, MMSMS_DATA_REQ, bsize_message);
         /*
          * check whether CP ACK needs to be send
          */
          if (SMS_INST.cp_ack_pending EQ TRUE)
          {
            cp_build_cp_ack ();
          }
         /*
          * reset retry counter
          */
          SMS_RETX(sms_data) = 0;
         /*
          * save data req
          */
          if (SMS_DATA_REQ(sms_data) NEQ NULL)
            PFREE (SMS_DATA_REQ(sms_data));
          SMS_DATA_REQ(sms_data) = data_req;
         /*
          * CP state transition CP_WAIT_FOR_ACK
          */
          SMS_INST_SET_STATE (STATE_CP, CP_WAIT_FOR_ACK);

          SMS_EM_SEND_CP_DATA;

#if FALSE
          PCOPY (tx_data_req, data_req);
#else
          SMS_SDU_COPY (tx_data_req, data_req, MMSMS_DATA_REQ);
#endif
          TRACE_BINDUMP(sms_handle,
                        TC_USER4,
                        "U_CP_DATA",
                        (&(tx_data_req->sdu.buf[0]) + ((tx_data_req->sdu.o_buf >> 3) -1)),
                        ((tx_data_req->sdu.l_buf >> 3) + 1));
          PSENDX (MM, tx_data_req);
         /*
          * start timer TC1M
          */
          sms_timer_start(TC1M);
          break;
        }

     /* ------------------------------------ */
        default:
     /* ------------------------------------ */
        {
          TRACE_ERROR("CP_DATA_REQ in wrong state received!");
          PFREE (data_req);
          break;
        }
      }

#if defined (GPRS)
    }
    else
      cp_data_req_gsms (data_req);
#endif /* GPRS */

  }
}

#if defined (GPRS)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_send_data_gsms          |
+--------------------------------------------------------------------+

  PURPOSE : Send CP_DATA_REQ on LL downlink. LLC availability to be
            checked by the caller
*/

GLOBAL void cp_send_data_gsms (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_send_data_gsms()");

  if (sms_data)
  {
    USHORT bsize_message = ENCODE_OFFSET +
                           D_SDU_LEN (SMS_DATA_REQ(sms_data));
    PALLOC_SDU (unitdata_req, LL_UNITDATA_REQ, bsize_message);

    /*
     * copy stored mo message sdu to unitdata sdu buffer
     */
    SMS_SDU_COPY (unitdata_req, SMS_DATA_REQ(sms_data), LL_UNITDATA_REQ);

    cp_init_ll_unitdata_req (unitdata_req);
    TRACE_BINDUMP(sms_handle,
                  TC_USER4,
                  "U_CP_DATA",
                  (&(unitdata_req->sdu.buf[0]) + ((unitdata_req->sdu.o_buf >> 3) -1)),
                  ((unitdata_req->sdu.l_buf >> 3) + 1));
    PSENDX (LLC, unitdata_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_data_req_gsms           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_DATA_REQ on LL downlink.

*/

GLOBAL void cp_data_req_gsms (T_MMSMS_DATA_REQ *data_req)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_data_req_gsms()");

  if (sms_data)
  {
    /*
     * check if LL is still registered
     */
    if (SMS_INST.downlink EQ SMS_DOWNLINK_LL_CHECK)
    {
      PALLOC (reg_state_req, GMMSMS_REG_STATE_REQ);
      memset (reg_state_req, 0, sizeof (T_GMMSMS_REG_STATE_REQ));
      PSENDX (GMM, reg_state_req);

      SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
    }

    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_IDLE:
      case CP_GSMS_IDLE:
      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
      {
        /*
         * save message for later usage
         */
        if (SMS_DATA_REQ(sms_data) NEQ NULL)
        {
          PFREE (SMS_DATA_REQ(sms_data));
        }
        SMS_DATA_REQ(sms_data) = data_req;

        /*
         * send message if flow control allows
         */
        if (SMS_INST.downlink EQ SMS_DOWNLINK_LL)
        {
          if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
          {
            cp_send_data_gsms ();
            SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
          }
          else
          {
            SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
          }
          if ((SMS_INST_GET_STATE (STATE_CP)) EQ CP_GSMS_MT_WAIT_FOR_RP_ACK)
          {
            SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MT_WAIT_FOR_CP_ACK);
          }
          else if ((SMS_INST_GET_STATE (STATE_CP)) EQ CP_GSMS_MO_WAIT_FOR_CP_DATA)
          {
            SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
          }
          else
          {
            SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MO_WAIT_FOR_CP_ACK);
          }
          SMS_RETX(sms_data) = 0;
         /*
          * start timer TC1M
          */
          sms_timer_start(TC1M);
        }
        break;
      }

      default:
      {
        PFREE (data_req);
        break;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_error_req_gsms          |
+--------------------------------------------------------------------+

  PURPOSE : Sending CP-ERROR over GPRS with consideration of LLC
            flow control.
*/

GLOBAL void cp_error_req_gsms (UBYTE      cause,
                               BOOL       free_ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_error_req_gsms()");

  if (sms_data)
  {
    if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
    {
      CCD_START;
      cp_build_cp_error (cause);
      CCD_END;

      SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
      SMS_INST.r_flag = FALSE;
      if(free_ti)
      {
        FREE_SMS_INSTANCE (SMS_INST.ti);
      }
      sms_data->llc_flow = SMS_LLC_BUSY;
    }
    else
    {
      SMS_CP_ACK_TYPE(sms_data) = SMS_CP_ERROR;
      SMS_CP_CAUSE(sms_data) = cause;

      SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
      SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
    }
  }
}
#endif /* GPRS */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_est_ind_cp_ack          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_EST_IND_CP_ACK.

*/

GLOBAL void cp_est_ind_cp_ack (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_est_ind_cp_ack(TI=%u)",SMS_INST.ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_IDLE:
      {
        cp_build_cp_error (SMS_CP_CS_MSG_NOT_COMP);

        SMS_INST.r_flag = FALSE;
        /*FALLTHROUGH*/ /*lint -fallthrough*/
      default:
        cp_send_release_req (SMS_INST.ti);

        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_est_ind_cp_data         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_EST_IND_CP_DATA.

*/

GLOBAL void cp_est_ind_cp_data (T_D_CP_DATA *cp_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_est_ind_cp_data(TI=%u)",SMS_INST.ti);

  switch (SMS_INST_GET_STATE (STATE_CP))
  {
    case CP_IDLE:
    {
     /*
      * generate CP_ACK
      */
      cp_build_cp_ack ();
     /*
      * CP state transition CP_MM_CONNECTION_ESTABLISHED
      */
      SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_ESTABLISHED);
     /*
      * r flag
      */
      SMS_INST.r_flag         = FALSE;
     /*
      * RL_DATA_IND =>
      */
      rl_data_ind (&cp_data->cp_user_data_dl);
      break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_est_ind_cp_error        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_EST_IND_CP_ERROR.

*/

GLOBAL void cp_est_ind_cp_error (UBYTE      cp_error)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_est_ind_cp_error(TI=%u)", SMS_INST.ti);
  TRACE_EVENT_P1 ("CP Error Cause = %d", (int)cp_error);

  if (sms_data)
  {
    TRACE_EVENT_P1 ("CP_ERROR.CAUSE rcvd: 0x%2.2X", (int)cp_error);

    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_IDLE:
      {
        SMS_INST.r_flag = FALSE;

        cp_send_release_req (SMS_INST.ti);
        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_est_ind_cp_unknown      |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_EST_IND_CP_UNKNOWN.

*/

GLOBAL void cp_est_ind_cp_unknown (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_est_ind_cp_unknown(TI=%u)", SMS_INST.ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_IDLE:
      {
        cp_build_cp_error (SMS_CP_CS_INFO_NON_EXIST);

        SMS_INST.r_flag = FALSE;

        cp_send_release_req (SMS_INST.ti);

        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_release_req             |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal CP_RELEASE_REQ.

*/

GLOBAL void cp_release_req (UBYTE ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION_P1 ("cp_release_req(TI=%u)", ti);

  if (sms_data)
  {
    switch (SMS_INST_GET_STATE (STATE_CP))
    {
      case CP_IDLE:
        SMS_INST.r_flag = FALSE;
        FREE_SMS_INSTANCE (ti);
        break;

      case CP_WAIT_FOR_ACK:
        SMS_INST.r_flag = TRUE;
        break;

      case CP_MM_CONNECTION_PENDING:
       /*
        * new connection establishment pending 
        * while releasing old connection
        * (-> concat sms)
        */
       /*
        * check whether CP ACK needs to be send
        */
        if (SMS_INST.cp_ack_pending EQ TRUE)
        {
          cp_build_cp_ack ();
        }
       /*
        * release connection
        */
        SMS_INST.r_flag = FALSE;
        cp_send_release_req (ti);
        break;

      case CP_MM_CONNECTION_ESTABLISHED:
       /*
        * check whether CP ACK needs to be send
        */
        if (SMS_INST.cp_ack_pending EQ TRUE)
        {
          cp_build_cp_ack ();
        }
       /*
        * CP state transition CP_IDLE
        */
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
       /*
        * release connection
        */
        SMS_INST.r_flag = FALSE;
        cp_send_release_req (ti);
        break;

#ifdef GPRS
      case CP_GSMS_IDLE:
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;
        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;

      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
        SMS_INST.r_flag = TRUE;
        break;

      case CP_GSMS_MO_WAIT_FOR_CP_DATA:
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;
        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;
#endif /* #ifdef GPRS */

      default:
        break;
    }
  }
}

#if defined (GPRS)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_gmmsms_reg_state_cnf    |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal GMMSMS_REG_STATE_CNF (GSMS only).

*/
GLOBAL void cp_gmmsms_reg_state_cnf (T_GMMSMS_REG_STATE_CNF *reg_state_cnf)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("cp_gmmsms_reg_state_cnf()");

  if (sms_data EQ NULL)
  {
     TRACE_ERROR("Horror: sms_data=NULL");
     return;
  }

  if (SMS_INST_GET_STATE (STATE_CP) EQ CP_MM_CONNECTION_PENDING)
  {
     /*
      * CP state transition
      */
      SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
      if (reg_state_cnf->reg_state EQ SMS_RS_REGISTERED)
      {
         TRACE_EVENT("downlink = SMS_DOWNLINK_LL");
         SMS_INST.downlink = SMS_DOWNLINK_LL;
         cp_send_getunitdata_req ();
        /*
         * RL_ESTABLISH_CNF =>
         */
         rl_establish_cnf(TRUE);
      }
      else
      {
        /*
         * RL_ESTABLISH_CNF =>
         */
         rl_establish_cnf(FALSE);
      }
  }
  else if (SMS_INST_GET_STATE (STATE_CP) EQ CP_GSMS_IDLE)
  {
    if (reg_state_cnf->reg_state EQ SMS_RS_REGISTERED)
    {
      SMS_INST.downlink = SMS_DOWNLINK_LL;
      cp_send_getunitdata_req ();

      if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
      {
        /*
         * using LL is possible, send message
         */
        cp_send_data_gsms ();
        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
      }
      else
      {
        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
      }
      SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MO_WAIT_FOR_CP_ACK);
      SMS_RETX(sms_data) = 0;
     /*
      * start timer TC1M
      */
      sms_timer_start(TC1M);
    }
    else
    {
      SMS_LLC_FLOW(sms_data) = SMS_LLC_UNKNOWN;

      if (sms_data->mo_dst_pref EQ GPRS_SMS_GPRS_PREF)
      {
        /*
         * unsing the preferred downlink LL failed, use CSD
         */

        PALLOC (establish_req, MMSMS_ESTABLISH_REQ);
        TRACE_EVENT("GPRS not registered, fallback to CSD");

        establish_req->ti = SMS_INST.ti;

        SMS_INST.downlink = SMS_DOWNLINK_MMSMS;
        SMS_INST_SET_STATE (STATE_CP, CP_MM_CONNECTION_PENDING);

        PSENDX (MM, establish_req);
      }
      else
      {
        /*
         * only GPRS is configured to be used, abort
         */
        cp_send_release_req (SMS_INST.ti);

        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;
        
        SMS_INST.downlink = SMS_DOWNLINK_NONE;

        rl_error_ind (SMS_CAUSE_NO_SERVICE);
        
        /* FREE_SMS_INSTANCE (SMS_INST.ti); */
      }
    }
  }
  PFREE (reg_state_cnf);
}
#endif /* GPRS */

/*---- FUNCTIONS ---------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_build_cp_ack            |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function CP_BUILD_CP_ACK.
            (GPRS: the appropriate downlink is determined first)
*/
GLOBAL void cp_build_cp_ack (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_build_cp_ack()");

 /*
  * reset cp ack pending
  */
  SMS_INST.cp_ack_pending = FALSE;

#if defined (GPRS)
  if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
#endif /* GPRS */
  {
    PALLOC_SDU (data_req, MMSMS_DATA_REQ, ENCODE_OFFSET+BSIZE_B_CP_ACK);

    data_req->sdu.o_buf = ENCODE_OFFSET;
    data_req->sdu.l_buf = BSIZE_B_CP_ACK;

    data_req->sdu.buf[BYTELEN_POS(data_req->sdu.o_buf)] = B_CP_ACK;
    cp_add_pd_ti (SMS_CP_REF(sms_data), &data_req->sdu);

    SMS_EM_SEND_CP_ACKNOWLEDGE;
    TRACE_BINDUMP(sms_handle,
                  TC_USER4,
                  "B_CP_ACK",
                  (&(data_req->sdu.buf[0]) + ((data_req->sdu.o_buf >> 3) -1)),
                  ((data_req->sdu.l_buf >> 3) + 1));
    PSENDX (MM, data_req);
    return;
  }

#if defined (GPRS)
  if ((SMS_INST.downlink EQ SMS_DOWNLINK_LL) /*AND
      (sms_data->llc_flow EQ SMS_LLC_AVAILABLE)*/)
  {
    /*
     * build cp_ack for LL downlink
     */
    PALLOC_SDU (unitdata_req, LL_UNITDATA_REQ, ENCODE_OFFSET+BSIZE_B_CP_ACK);

    unitdata_req->sdu.o_buf = ENCODE_OFFSET;
    unitdata_req->sdu.l_buf = BSIZE_B_CP_ACK;

    unitdata_req->sdu.buf[BYTELEN_POS(unitdata_req->sdu.o_buf)] = B_CP_ACK;
    cp_add_pd_ti (SMS_CP_REF(sms_data), &unitdata_req->sdu);

    cp_init_ll_unitdata_req (unitdata_req);

    SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
    TRACE_BINDUMP(sms_handle,
                  TC_USER4,
                  "B_CP_ACK",
                  (&(unitdata_req->sdu.buf[0]) + ((unitdata_req->sdu.o_buf >> 3) -1)),
                  ((unitdata_req->sdu.l_buf >> 3) + 1));
    PSENDX (LLC, unitdata_req);
  }
#endif /* GPRS */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_build_cp_error          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function CP_BUILD_CP_ERROR.

*/
GLOBAL void cp_build_cp_error (UBYTE      error)
{
  GET_INSTANCE_DATA;
  if (sms_data)
  {
    TRACE_EVENT_P1 ("CP_ERROR.CAUSE sent: 0x%2.2X", (int)error);

#if defined (GPRS)
    if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
#endif /* GPRS */

    {
      MCAST (cp_error, B_CP_ERROR);
      PALLOC_MSG (data_req, MMSMS_DATA_REQ, B_CP_ERROR);

      TRACE_FUNCTION ("cp_build_cp_error()");

      SMS_EM_SEND_CP_ERROR;

     /*
      * check whether CP ACK needs to be send
      */
      if (SMS_INST.cp_ack_pending EQ TRUE)
      {
        cp_build_cp_ack ();
      }

      data_req->sdu.o_buf = ENCODE_OFFSET;

      cp_error->msg_type = B_CP_ERROR;
      cp_error->cp_cause = error;

      ccd_codeMsg (CCDENT_SMS,
                   UPLINK,
                   (T_MSGBUF *) &data_req->sdu,
                   (UBYTE *) _decodedMsg,
                   NOT_PRESENT_8BIT);

      cp_add_pd_ti (SMS_CP_REF(sms_data), &data_req->sdu);
      TRACE_BINDUMP(sms_handle,
                    TC_USER4,
                    "B_CP_ERROR",
                    (&(data_req->sdu.buf[0]) + ((data_req->sdu.o_buf >> 3) -1)),
                    ((data_req->sdu.l_buf >> 3) + 1));
      PSENDX (MM, data_req);
      return;
    }

#if defined (GPRS)
    if ((SMS_INST.downlink EQ SMS_DOWNLINK_LL) /*&&
        (sms_data->llc_flow EQ SMS_LLC_AVAILABLE)*/)
    {
      /*
       * build cp_error for LL downlink
       */
      MCAST (cp_error, B_CP_ERROR);

      PALLOC_MSG (unitdata_req, LL_UNITDATA_REQ, B_CP_ERROR);

      TRACE_FUNCTION ("cp_build_cp_error() - LL downlink");

      unitdata_req->sdu.o_buf = ENCODE_OFFSET;

      cp_error->msg_type = B_CP_ERROR;
      cp_error->cp_cause = error;

      ccd_codeMsg (CCDENT_SMS,
                   UPLINK,
                   (T_MSGBUF *) &unitdata_req->sdu,
                   (UBYTE *) _decodedMsg,
                   NOT_PRESENT_8BIT);

      cp_add_pd_ti (SMS_CP_REF(sms_data), &unitdata_req->sdu);
      cp_init_ll_unitdata_req (unitdata_req);
      TRACE_BINDUMP(sms_handle,
                    TC_USER4,
                    "B_CP_ERROR",
                    (&(unitdata_req->sdu.buf[0]) + ((unitdata_req->sdu.o_buf >> 3) -1)),
                    ((unitdata_req->sdu.l_buf >> 3) + 1));
      PSENDX (LLC, unitdata_req);
    }
#endif /* GPRS */
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_send_release_req        |
+--------------------------------------------------------------------+

  PURPOSE : Send a MMSMS_RELEASE_REQ.
            (GPRS: only if this is not an LL downlink)

*/
GLOBAL void cp_send_release_req ( UBYTE ti)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("cp_send_release_req()");

  if (sms_data)
  {
#if defined (GPRS)
    if (SMS_INST.downlink EQ SMS_DOWNLINK_MMSMS)
    {
#endif
      PALLOC (release_req, MMSMS_RELEASE_REQ);

      release_req->ti = ti;

      PSENDX (MM, release_req);

#if defined (GPRS)
    }
#endif
  }
}

#endif

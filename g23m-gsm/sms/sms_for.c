/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_FOR
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
|  Purpose :  This Modul defines the functions for the formatter
|             of the component SMS.
+-----------------------------------------------------------------------------
*/

#ifndef SMS_FOR_C
#define SMS_FOR_C

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

#if !defined (SHARED_CCD_BUF)
GLOBAL UBYTE               _decodedMsg [MAX_MSTRUCT_LEN_SMS];
#else
GLOBAL UBYTE *             _decodedMsg;
GLOBAL UBYTE               _CCDbuf = FALSE;
#endif

/*==== FUNCTIONS ===================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_init_sms               |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the formatter (void)
*/

GLOBAL void for_init_sms (void)
{
//  TRACE_FUNCTION ("for_init_sms()");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_get_pd_ti              |
+--------------------------------------------------------------------+

  PURPOSE : Get protocol discriminator and transaction identifier if
            the incoming message is of sufficient length to contain
            anythiny useful, this means at least PD, TI and message type.
*/

LOCAL BOOL for_get_pd_ti (T_sdu *p_sdu, UBYTE *p_pd, UBYTE *p_ti)
{
  if (p_sdu->l_buf >= 16)
  {
    *p_pd = p_sdu->buf[p_sdu->o_buf >> 3] & 0x0F;
    *p_ti = p_sdu->buf[p_sdu->o_buf >> 3] >> 4;
    *p_ti ^= 0x8;

    p_sdu->o_buf += BSIZE_TI_PD;
    p_sdu->l_buf -= BSIZE_TI_PD;

    return TRUE;
  }
  TRACE_EVENT ("message too short");
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_get_new_mt_instance    |
+--------------------------------------------------------------------+

  PURPOSE : This function gets a new MT instance. Basically the same 
            is done here as in GET_NEW_SMS_INSTANCE(), but additionally 
            the special case that the CP-Layer of an old MT transaction
            waits for the last CP-ACK before the MMSMS_RELEASE_REQ is
            sent is handled.

*/

LOCAL T_SMS_DATA * for_get_new_mt_instance (UBYTE ti)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_get_new_mt_instance()");

  /*
   * Check here whether we are awaiting the last CP-ACK for a MT transaction.
   * If so, take the new MT message as implicit CP-ACK.
   */
  if (sms_data NEQ NULL) /*lint !e774 (sms_data NEQ NULL) always */
  {
    if (sms_data->data[INST_MT].ti NEQ 0)
    {
      /* Old MT instance still exists */
      sms_data->inst = INST_MT;
      switch (SMS_INST_GET_STATE (STATE_CP))
      {
        case CP_WAIT_FOR_ACK:
  #ifdef GPRS
        case CP_GSMS_MT_WAIT_FOR_CP_ACK:
  #endif
          if (SMS_INST.r_flag)
          {
            /*
             * No further message for the old MT instance expected. Take the
             * MMSMS_ESTABLISH_IND as implicit CP-ACK for the old instance.
             */
            TRACE_EVENT ("Implicit CP-ACK");
            cp_data_ind_cp_ack ();
          }
          break;

        default:
          break; /* Do nothing */
      }
    }
  }

  sms_data = GET_NEW_SMS_INSTANCE(ti);

  return sms_data;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_ccd_rp_error_send      |
+--------------------------------------------------------------------+

  PURPOSE : Perform CCD Error check and send rp errors

*/

LOCAL BOOL for_ccd_rp_error_send (T_D_CP_DATA *cp_data,
                                  T_SMS_DATA *sms_data, UBYTE rp_error)
{
  BOOL rp_error_send = FALSE;

  MCAST (cp_data1, U_CP_DATA);

  if((SMS_INST_GET_STATE (STATE_CP)) EQ CP_MM_CONNECTION_ESTABLISHED)
  {
    rp_error_send = TRUE;
    rl_build_rp_error (cp_data->cp_user_data_dl.reference,
                       rp_error,cp_data1, NULL);
    SMS_EM_SEND_RP_ERROR;
    cp_data_req (cp_data1);
    cp_send_release_req (SMS_INST.ti);

    FREE_SMS_INSTANCE (SMS_INST.ti);
  }
  return rp_error_send;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_ccd_error_check            |
+--------------------------------------------------------------------+

  PURPOSE : Perform CCD Error check and returns errors

*/

LOCAL BOOL for_ccd_error_check (UBYTE *_decodedMsg,T_sdu *buf_sdu,
                                T_SMS_DATA *sms_data,UBYTE ti)
{
  ULONG  ccd_err = ERR_NO_MORE_ERROR; 
  T_CCD_ERR_ENTRY *ccd_err_entry = 0;
  UBYTE p_sdu;
  UBYTE cp_error_cause = 0;
  UBYTE length = 0;
  BOOL cp_error_send = FALSE;
  BOOL rp_error_send = FALSE;


  /* Message offset value */
  p_sdu = (buf_sdu->o_buf/8) - 1;

  ccd_err = ccd_getFirstFault(&ccd_err_entry);
  do
  {
    /*
    * Need to set these values everytime we go through fault list of ccd
    */
    cp_error_send = FALSE;
    rp_error_send = FALSE;
    /*lint -e415 Likely access of out-of-bounds pointer*/
    switch(ccd_err)
    {
      case ERR_ELEM_LEN:          /* ignore message & send STATUS #96 */
        /*
        * This ccd error will come when there is any error in IE length.
        * If cp user data length is greater than 249 Octets then this error
        * is because CP Message IE, so send CP-ERROR
        * if cp user data length is less than 249, then this ccd error is 
        * because of rp IE length, so RP-ERROR
        */
        switch(_decodedMsg[0])
        {          
          case D_CP_DATA:
            length = buf_sdu->buf[p_sdu+2];

            /* 
            * If CP User data length is more than 249 then CCD error is because
            * of CP Message IE else it will be becasue of RP message IE 
            * Send error accordingly
            */
            if(length < MAX_LEN_CP_USER_DATA)
            {
              rp_error_send = for_ccd_rp_error_send((T_D_CP_DATA *)_decodedMsg,
                                                     sms_data,
                                                     SMS_RP_CS_INV_MAND_INFO);
            }
            if(rp_error_send EQ FALSE)
            {
              cp_error_send = TRUE;
              cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            }
            break;
          /* 
          * If Message is CP-ACK or CP-ERROR then this ccd error is due to CP
          * no need to differentiate between RP and CP, These two messages are
          * intended for CP Layer, So send CP-ERROR
          */
          case B_CP_ACK:
          case B_CP_ERROR:
            /* This flag will become FALSE, in case there is CCD warning, so make it 
            *  TRUE till we get CCD Error, especially this will happen in default case
            */
            cp_error_send = TRUE;
            cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            break;
        }
        break;

      case ERR_COMPREH_REQUIRED:  /* ignore message & send STATUS #96 */        
        /* 
        * This error will come when Comprehension bit required set is having error in 
        * IE.Comprehension bits can be set inside only CP-DATA message
        * for RP-ERROR or RP-ACK
        */
        if(_decodedMsg[0] EQ D_CP_DATA)
        {
          /*
          * Comprehension bit error is in RP_ERROR or RP_ACK after RP Cause element
          */
          rp_error_send = for_ccd_rp_error_send((T_D_CP_DATA *)_decodedMsg,
                                                 sms_data,
                                                 SMS_RP_CS_INV_MAND_INFO);
          if(rp_error_send EQ FALSE) 
          {
            cp_error_send = TRUE;
            cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
          }
        }
        break;

      case ERR_MAND_ELEM_MISS:          /* ignore message & send STATUS #96 */
        /* See  bit postions to check whether it is CP or RP Error */
        /*
        * Checking whether this error is in CP message element or RP 
        * message element. This is required to check because both CP and RP
        * Layer decoding is done inside same CCD function
        */
        /* This is for CP Message decoding error - in Length*/
        length = (buf_sdu->l_buf/8);
        switch(buf_sdu->buf[1])
        {
          case D_CP_DATA:
            /* Evaluate length of Mandatory CP-DATA/CP-ERROR message */
            if(length < MIN_CP_LEN)
            {
              cp_error_send = TRUE;
              cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            }
            else
            {
              cp_error_send = FALSE;
            }
            break;

          case B_CP_ERROR:/* This will not have RP DATA, so no need to check for RP */
            /* Evaluate length of Mandatory CP-DATA/CP-ERROR message */
            if(length < MIN_CP_LEN)
            {
              cp_error_send = TRUE;
              cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            }
            break;

          case B_CP_ACK:/* This will not have RP DATA, so no need to check for RP */
            if(length < MIN_CP_ACK_LEN)
            {
              cp_error_send = TRUE;
              cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            }
            break;

          default:
            cp_error_send = TRUE;
            cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
            break;
        }
        break;
              
      case ERR_INVALID_MID: /* ignore message & send STATUS #97 */
        /*
        * Checking whether this error is in CP message element or RP 
        * message element. This is required to check because both CP and RP
        * Layer decoding is done inside same CCD function
        */
       /* See  bit postions to check whether it is CP or RP Error */
       /* This is for CP Message decoding error - in Length*/
        if ((buf_sdu->buf[1] NEQ D_CP_DATA)||(buf_sdu->buf[1] NEQ B_CP_ACK)
             ||(buf_sdu->buf[1] NEQ B_CP_ERROR))
        {
          cp_error_send = TRUE;
          cp_error_cause = SMS_CP_CS_MSG_TYPE_NON_EXIST;
        }
        else
          /* Let RP Layer Handle This ccd error, eventually RP Layer will
          * send RP-ERROR
          */
          /* This is for RP Message decoding error - in Length*/
          cp_error_send = FALSE;
        break;

      case ERR_MSG_LEN:  /* ignore message & send STATUS #96 */
        /* Send CP-ERROR, some problem with Message Length, whole message is garbled */
        cp_error_send = TRUE;
        cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
        break;
      
      case ERR_LEN_MISMATCH:  /* ignore message & send STATUS #96 */
        /* Send CP-ERROR, some problem with Message Length, whole message is garbled */
        cp_error_send = TRUE;
        cp_error_cause = SMS_CP_CS_INV_MAND_INFO;
        break;
      default:
        TRACE_EVENT_P1 ("Unexpected warnings/errors = %u", ccd_err);
        /*
        *Default warnings pass it to RP Layer, decoding at TP will take care of it 
        */
        cp_error_send = FALSE;
        break;
    }
     /*lint +e415 Likely access of out-of-bounds pointer*/
    ccd_err = ccd_getNextFault(&ccd_err_entry);
  }while(ccd_err != ERR_NO_MORE_ERROR);

  if(cp_error_send)
  {
    cp_build_cp_error (cp_error_cause);
    /*
     * Check if the cmms_mode is enabled or not
     * If enabled dont release the MM connection
     */

    if(!CMMS_ACTIVE)
    {
       cp_send_release_req (ti);
       SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
       SMS_INST.r_flag = FALSE;
    }
    else
    {
       sms_data->cmms_release_pending = TRUE;
    }

    rl_error_ind (CAUSE_MAKE(DEFBY_STD, ORIGSIDE_MS,
                             SMSCP_ORIGINATING_ENTITY, cp_error_cause));	


    FREE_SMS_INSTANCE (ti);
  }
  /* These flags return value will decide whether to pass to rp layer further */
  return (!rp_error_send && !cp_error_send);
}


/*---- PRIMITIVES --------------------------------------------------*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_mmsms_establish_ind    |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMSMS_ESTABLISH_IND

*/

GLOBAL void for_mmsms_establish_ind (T_MMSMS_ESTABLISH_IND *mmsms_establish_ind)
{
  UBYTE     ti;
  UBYTE     pd;
  CHAR      *msg_type;
  BOOL      pass_to_rp = TRUE;


  /* Enable the PCO to correctly decode the message */
  PPASS (mmsms_establish_ind, establish_ind, MMSMS_ESTABLISH_IND);

  TRACE_FUNCTION ("for_mmsms_establish_ind()");

  if (for_get_pd_ti (&establish_ind->sdu, &pd, &ti))
  {
    if ((ti & 0x8) AND (pd EQ PD_SMS))
    {
      register T_SMS_DATA *sms_data = for_get_new_mt_instance (ti);

      TRACE_EVENT_P1 ("TI %u", ti);

      if (sms_data)
      {
#if defined (GPRS)
        SMS_INST.downlink = SMS_DOWNLINK_MMSMS;
#endif
    
        SMS_INST.cp_ack_pending = FALSE;

        CCD_START;

        if(ccd_decodeMsg (CCDENT_SMS,
                          DOWNLINK,
                          (T_MSGBUF *) &establish_ind->sdu,
                          (UBYTE *)_decodedMsg,
                          NOT_PRESENT_8BIT) EQ ccdError)
        {
          pass_to_rp = for_ccd_error_check(_decodedMsg,&establish_ind->sdu,sms_data,ti);
        }

        if(pass_to_rp)
        {
          switch (_decodedMsg[0])
          {
            case D_CP_DATA:
              cp_est_ind_cp_data ((T_D_CP_DATA *)_decodedMsg);
              msg_type = "D_CP_DATA";

              SMS_EM_RECEIVE_CP_DATA;

              break;

            case B_CP_ERROR:
            {
              MCAST (error, B_CP_ERROR);

              cp_est_ind_cp_error (error->cp_cause);
              msg_type = "B_CP_ERROR";

              SMS_EM_RECEIVE_CP_ERROR;

              break;
            }

            case B_CP_ACK:
              cp_est_ind_cp_ack ();
              msg_type = "B_CP_ACK";
              SMS_EM_RECEIVE_CP_ACKNOWLEDGE;

              break;

            default:
              cp_est_ind_cp_unknown ();
              msg_type = "UNKNOWN_MESSAGE";

              SMS_EM_RECEIVE_UNKNOWN;

              break;
          }
          TRACE_BINDUMP(sms_handle,
                        TC_USER4,
                        msg_type,
                        (&(establish_ind->sdu.buf[0]) + ((establish_ind->sdu.o_buf >> 3) -1)),
                        ((establish_ind->sdu.l_buf >> 3) + 1));
        }
        CCD_END;
      }
      else
      {
        /*
         * no further instance available
         */
        CCD_START;

        ccd_decodeMsg (CCDENT_SMS,
                       DOWNLINK,
                       (T_MSGBUF *) &establish_ind->sdu,
                       (UBYTE *) _decodedMsg,
                       NOT_PRESENT_8BIT);

        switch (_decodedMsg[0])
        {
          case D_CP_DATA:
          {
            MCAST (d_cp_data, D_CP_DATA);
            MCAST (cp_data, U_CP_DATA);
            /*
             * U_CP_DATA contains a maximum of 252 Bytes
             */
            PALLOC_SDU (data_req, MMSMS_DATA_REQ, LEN_U_CP_DATA);

            data_req->sdu.o_buf = ENCODE_OFFSET;

            rl_build_rp_error (d_cp_data->cp_user_data_dl.reference,
                               SMS_RP_CS_PROTOCOL_ERROR,
                               cp_data, NULL);

            cp_data->msg_type = U_CP_DATA;

            ccd_codeMsg (CCDENT_SMS,
                         UPLINK,
                         (T_MSGBUF *)&data_req->sdu,
                         (UBYTE *)cp_data,
                         NOT_PRESENT_8BIT);

            cp_add_pd_ti (ti, &data_req->sdu);

            TRACE_BINDUMP(sms_handle,
                          TC_USER4,
                          "U_CP_DATA",
                          (&(data_req->sdu.buf[0]) + ((data_req->sdu.o_buf >> 3) -1)),
                          ((data_req->sdu.l_buf >> 3) + 1));
            PSENDX (MM, data_req);
            break;
          }
          default:
            TRACE_EVENT_P1 ("Unexpected Message = %u", _decodedMsg[0]);
            break;
        }
        CCD_END;
        {
          PALLOC (release_req, MMSMS_RELEASE_REQ);

          release_req->ti = ti;
          PSENDX (MM, release_req);
        }
        SMS_EM_UNKNOWN_TRANSACTION;
      }
    }
    else
    {
      TRACE_EVENT_P2 ("TI or PD wrong, PD=%d, TI=%d", pd, ti);
    }
  }
  PFREE (establish_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_FOR                    |
| STATE   : code                ROUTINE : for_mmsms_data_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMSMS_DATA_IND

*/

GLOBAL void for_mmsms_data_ind (T_MMSMS_DATA_IND *mmsms_data_ind)
{
  UBYTE     ti;
  UBYTE     pd;
  CHAR      *msg_type;
  BOOL      pass_to_rp = TRUE;


  /* Enable the PCO to correctly decode the message */
  PPASS (mmsms_data_ind, data_ind, MMSMS_DATA_IND);

  if (for_get_pd_ti (&data_ind->sdu, &pd, &ti))
  {
    TRACE_FUNCTION_P1 ("for_mmsms_data_ind(TI=%u)", ti);

    if (/*((! ti) OR (ti >= 8)) AND*/ (pd EQ PD_SMS))
    {
      register T_SMS_DATA *sms_data = GET_SMS_INSTANCE(ti);

      if (sms_data)
      {
        CCD_START;

        if(ccd_decodeMsg (CCDENT_SMS,
                                DOWNLINK,
                                (T_MSGBUF *) &data_ind->sdu,
                                (UBYTE *)_decodedMsg,
                                NOT_PRESENT_8BIT) EQ ccdError)
        {
          pass_to_rp = for_ccd_error_check(_decodedMsg,&data_ind->sdu,sms_data,ti);
        }
        
        if(pass_to_rp)
        {
          switch (_decodedMsg[0])
          {
            case D_CP_DATA:
              cp_data_ind_cp_data ((T_D_CP_DATA *)_decodedMsg);
              msg_type = "D_CP_DATA";
              break ;

            case B_CP_ERROR:
            {
              MCAST (error, B_CP_ERROR);

              cp_data_ind_cp_error (error->cp_cause);
              msg_type = "B_CP_ERROR";
              break ;
            }

            case B_CP_ACK:
              cp_data_ind_cp_ack ();  
              msg_type = "B_CP_ACK";
              break ;
            default:
              cp_data_ind_cp_unknown ();  
              msg_type = "UNKNOWN MESSAGE";
              break ;
          }
          TRACE_BINDUMP(sms_handle,
                        TC_USER4,
                        msg_type,
                        (&(data_ind->sdu.buf[0]) + ((data_ind->sdu.o_buf >> 3) -1)),
                        ((data_ind->sdu.l_buf >> 3) + 1));
        }
        CCD_END;
      }
    }
    else
    {
      TRACE_EVENT_P1 ("PD wrong, PD=%d", pd);
    }
  }
  else
  {
     TRACE_ERROR("for_mmsms_data_ind() error evaluating PD TI");
  }
  PFREE (data_ind) ;
}

#if defined (GPRS)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : cp_ll_unitdata_ind         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal LL_UNITDATA_IND (GSMS only).

*/
GLOBAL void for_ll_unitdata_ind (T_LL_UNITDATA_IND *ll_unitdata_ind)
{

  UBYTE     ti;
  UBYTE     pd;
  CHAR      *msg_type;

  T_SMS_DATA *sms_global = GET_INSTANCE(0);

  /* Enable the PCO to correctly decode the message */
  PPASS (ll_unitdata_ind, unitdata_ind, LL_UNITDATA_IND);

  TRACE_FUNCTION ("for_ll_unitdata_ind()");

  if (!(unitdata_ind->sapi EQ LL_SAPI_7))
  {
    TRACE_EVENT ("SAPI not LL_SAPI_7");
    PFREE (unitdata_ind);
    return;
  }

  if (sms_global) /*lint !e774 (sms_global) always */
  {
    if (SMS_SMS_FLOW(sms_global) NEQ SMS_FLOW_AVAILABLE)
    {
      TRACE_ERROR ("unexpected primitive");
    }

    /*
     * data has been received, mark flow status as busy
     */
    SMS_SMS_FLOW(sms_global) = SMS_FLOW_BUSY;
  }

  if (for_get_pd_ti (&unitdata_ind->sdu, &pd, &ti))
  {
    if (/*((! ti) OR (ti >= 8)) AND*/ (pd EQ PD_SMS))
    {
      register T_SMS_DATA *sms_data = GET_SMS_INSTANCE(ti);

      if (sms_data)
      {
        CCD_START;

        ccd_decodeMsg (CCDENT_SMS,
                       DOWNLINK,
                       (T_MSGBUF *) &unitdata_ind->sdu,
                       (UBYTE *) _decodedMsg,
                       NOT_PRESENT_8BIT);

        switch (_decodedMsg[0])
        {
          case D_CP_DATA:
            cp_data_ind_cp_data ((T_D_CP_DATA *)_decodedMsg);
            msg_type = "D_CP_DATA";
            break ;

          case B_CP_ERROR:
          {
            MCAST (error, B_CP_ERROR);

            cp_data_ind_cp_error (error->cp_cause);
            msg_type = "B_CP_ERROR";
            break ;
          }

          case B_CP_ACK:
            cp_data_ind_cp_ack ();
            msg_type = "B_CP_ACK";
            break ;
          default:
            cp_data_ind_cp_unknown ();
            msg_type = "UNKNOWN MESSAGE";
            break ;
        }
        CCD_END;
        TRACE_BINDUMP(sms_handle,
                      TC_USER4,
                      msg_type,
                      (&(unitdata_ind->sdu.buf[0]) + ((unitdata_ind->sdu.o_buf >> 3) -1)),
                      ((unitdata_ind->sdu.l_buf >> 3) + 1));
      }
      else
      {
        /*
         * no SMS instance associated with pid, try to create new one
         */
        if (ti & 0x8)
        {
          register T_SMS_DATA *sms_data = for_get_new_mt_instance (ti);

          if (sms_data)
          {
            CCD_START;

            ccd_decodeMsg (CCDENT_SMS,
                           DOWNLINK,
                           (T_MSGBUF *) &unitdata_ind->sdu,
                           (UBYTE *) _decodedMsg,
                           NOT_PRESENT_8BIT);

            SMS_INST_SET_STATE (STATE_CP, CP_GSMS_IDLE);
            SMS_INST.downlink = SMS_DOWNLINK_LL;

            switch (_decodedMsg[0])
            {
            case D_CP_DATA:
              cp_data_ind_cp_data ((T_D_CP_DATA*) _decodedMsg);
              msg_type = "D_CP_DATA";
              break ;

            case B_CP_ERROR:
              {
                MCAST (error, B_CP_ERROR);

                cp_data_ind_cp_error (error->cp_cause);
                msg_type = "B_CP_ERROR";
              }
              break ;

            case B_CP_ACK:
              cp_data_ind_cp_ack ();
              msg_type = "B_CP_ACK";
              break ;

            default:
              cp_data_ind_cp_unknown ();
              msg_type = "UNKNOWN MESSAGE";
              break ;
            }
            CCD_END;
            TRACE_BINDUMP(sms_handle,
                          TC_USER4,
                          msg_type,
                          (&(unitdata_ind->sdu.buf[0]) + ((unitdata_ind->sdu.o_buf >> 3) -1)),
                          ((unitdata_ind->sdu.l_buf >> 3) + 1));
          }
          else
          {
           /*
            * no further instance available, send rp error message
            */
            register T_SMS_DATA* sms_data_main = GET_INSTANCE(0);
            CCD_START;

            ccd_decodeMsg (CCDENT_SMS,
                           DOWNLINK,
                           (T_MSGBUF *) &unitdata_ind->sdu,
                           (UBYTE *) _decodedMsg,
                           NOT_PRESENT_8BIT);
            if (sms_data_main) /*lint !e774 (sms_data_main) always */
            {
              if ((_decodedMsg[0] EQ D_CP_DATA) &&
                  (sms_data_main->llc_flow EQ SMS_LLC_AVAILABLE))
              {
                UBYTE msg_ref;

                MCAST (d_cp_data, D_CP_DATA);
                MCAST (cp_data, U_CP_DATA);
                /*
                 * U_CP_DATA contains a maximum of 252 Bytes
                 */
                PALLOC_SDU (unitdata_req, LL_UNITDATA_REQ, LEN_U_CP_DATA);

                cp_init_ll_unitdata_req (unitdata_req);
                msg_ref = d_cp_data->cp_user_data_dl.reference;

                rl_build_rp_error_gprs (ti, unitdata_req, SMS_RP_CS_PROTOCOL_ERROR,
                                        msg_ref, cp_data, NULL);
                sms_data_main->llc_flow = SMS_LLC_BUSY;
                TRACE_BINDUMP(sms_handle,
                              TC_USER4,
                              "U_CP_DATA",
                              (&(unitdata_req->sdu.buf[0]) + ((unitdata_req->sdu.o_buf >> 3) -1)),
                              ((unitdata_req->sdu.l_buf >> 3) + 1));
                PSENDX (LLC, unitdata_req);
              }
            }
            CCD_END;
          }
        }
        else
        {
          TRACE_EVENT_P1 ("Unknown TI with response flag set, TI=%d", ti);
        }
      }
    }
    else
    {
      TRACE_EVENT_P1 ("PD wrong, PD=%d", pd);
    }
  }

  PFREE (unitdata_ind);

  /*
   * Receive ready, if sending is not blocked
   */
//  if (SMS_LLC_FLOW(sms_global) NEQ SMS_LLC_BUSY_WAITING)
  {
    cp_send_getunitdata_req ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_CP                     |
| STATE   : code                ROUTINE : for_ll_unitready_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Processing the signal LL_UNITREADY_IND (GSMS only).
*/

GLOBAL void for_ll_unitready_ind (T_LL_UNITREADY_IND *unitready_ind)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("ll_unitready_ind()");

  if ((unitready_ind->sapi NEQ LL_SAPI_7) || !sms_data) /*lint !e774 (!sms_data) never */
  {
    TRACE_EVENT ("ll_unitready_ind(): SAPI not LL_SAPI_7 or no SMS instance");
    PFREE (unitready_ind);
    return;
  }

  if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_BUSY_WAITING)
  {
    /*
     * find the message waiting for LLC
     */
    GET_MO_INSTANCE(sms_data);

    if (SMS_INST_GET_STATE (STATE_CP) EQ CP_GSMS_IDLE)
    {
      switch (SMS_CP_ACK_TYPE(sms_data))
      {
      case SMS_CP_ACK:
        cp_build_cp_ack ();
        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;

        if (SMS_CP_UDL(sms_data) NEQ NULL)
        {
          rl_data_ind (SMS_CP_UDL(sms_data));
          MFREE(SMS_CP_UDL(sms_data));
          SMS_CP_UDL(sms_data) = NULL;
        }
        if (SMS_INST.r_flag)
        {
          /*
           * terminate SMS instance
           */
          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
          SMS_INST.r_flag = FALSE;
          FREE_SMS_INSTANCE (SMS_INST.ti);
        }
        break;

      case SMS_CP_ERROR:
        cp_build_cp_error (SMS_CP_CAUSE(sms_data));
        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
        /*
         * terminate SMS instance
         */
        SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
        SMS_INST.r_flag = FALSE;
        FREE_SMS_INSTANCE (SMS_INST.ti);
        break;

      default:
        SMS_LLC_FLOW(sms_data) = SMS_LLC_AVAILABLE;
        break;
      }
      SMS_CP_ACK_TYPE(sms_data) = SMS_CP_NONE;
    }
    else
    {
      SMS_LLC_FLOW(sms_data) = SMS_LLC_AVAILABLE;
    }
    if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
    {
      GET_MT_INSTANCE(sms_data);

      switch (SMS_INST_GET_STATE (STATE_CP))
      {
      case CP_GSMS_IDLE:
      case CP_GSMS_MT_WAIT_FOR_RP_ACK:
      case CP_GSMS_MT_WAIT_FOR_CP_ACK:
        switch (SMS_CP_ACK_TYPE(sms_data))
        {
        case SMS_CP_ACK:
          cp_build_cp_ack ();
          if (SMS_INST_GET_STATE (STATE_CP) EQ CP_GSMS_MT_WAIT_FOR_CP_ACK)
          { /* stored CP-DATA will follow */
            SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY_WAITING;
          }
          else
          {
            SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
          }
          break;

        case SMS_CP_ERROR:
          cp_build_cp_error (SMS_CP_CAUSE(sms_data));
          SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
          /*
           * terminate SMS instance
           */
          SMS_INST_SET_STATE (STATE_CP, CP_IDLE);
          SMS_INST.r_flag = FALSE;
          FREE_SMS_INSTANCE (SMS_INST.ti);
          break;

        default:
          if (SMS_INST_GET_STATE (STATE_CP) EQ CP_GSMS_MT_WAIT_FOR_CP_ACK
               AND SMS_DATA_REQ(sms_data) NEQ 0)
          {
            cp_send_data_gsms ();
            SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
          }
          break;

        }
        SMS_CP_ACK_TYPE(sms_data) = SMS_CP_NONE;
        break;

      default:
        break;
      }
    }
    if (SMS_LLC_FLOW(sms_data) EQ SMS_LLC_AVAILABLE)
    {
      GET_MO_INSTANCE(sms_data);

      switch (SMS_INST_GET_STATE (STATE_CP))
      {
      case CP_GSMS_MO_WAIT_FOR_CP_ACK:
        cp_send_data_gsms ();

        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
        break;

      case CP_GSMS_IDLE:
        cp_send_data_gsms ();
        rl_proceed ();

        SMS_INST.r_flag = TRUE;
        SMS_INST_SET_STATE (STATE_CP, CP_GSMS_MO_WAIT_FOR_CP_ACK);
        SMS_LLC_FLOW(sms_data) = SMS_LLC_BUSY;
        break;

      default:
        break;
      }
    }
  }
  else
  {
    SMS_LLC_FLOW(sms_data) = SMS_LLC_AVAILABLE;
  }
  PFREE (unitready_ind);

//  cp_send_getunitdata_req (sms_data);
}

#endif /* GPRS */

#endif /* #ifndef SMS_FOR_C */

/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CC_FOR
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
|             of the component CC.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_FOR_C
#define CC_FOR_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CC
/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "message.h"
#include "ccdapi.h"
#include "prim.h"
#include "cnf_cc.h"
#include "mon_cc.h"
#include "pei.h"
#include "tok.h"
#include "cc.h"
#include "cc_em.h"

/*==== EXPORT =====================================================*/

/*==== PROTOTYPES =================================================*/
/* Implements Measure#  44 */
LOCAL void for_send_u_release_comp (USHORT cause);
/* Implements Measure#  42 */
LOCAL void for_fill_sdu_and_send (UBYTE msg,
                                  USHORT len);

/* Implements Measure#  11 */
LOCAL void code_msg (UBYTE * msg,
                     USHORT len);



/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
#if !defined (SHARED_CCD_BUF)
GLOBAL UBYTE               _decodedMsg [MAX_MSTRUCT_LEN_CC];
#else
GLOBAL UBYTE *             _decodedMsg;
#endif

/*==== FUNCTIONS ==================================================*/

#if 0
/* xhegadeg 24th May 2005 
   The part of the code between #if 0 to #endif is never executed.*/ 

/*
 Reason For Commenting :
 Issue CC-ENH-30714---->
   All the Layer3 CC messages are now traced via TRACE_BINDUMP.
   So no need of the Function cc_trace_bcap() which was earlier
   used for taking Setup ( Both Uplink & Downlink),Call Confirm,
   Call Proceeding Traces using TRACE_BCAP.
*/
#if defined TRACE_BCAP

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : cc_trace_bcap              |
+--------------------------------------------------------------------+

  PURPOSE : Trace messages containing bearer caps. This has been shown
            to be useful in the past.

*/

LOCAL void cc_trace_bcap (T_CC_DATA *cc_data, 
                    const T_sdu *sdu,
                          UBYTE direction)
{
  const UBYTE *msg_ptr;  /* Pointer to non-decoded message */
  unsigned     msg_len;  /* Length of non-decoded message */
  unsigned     str_len;  /* Length of trace string */

  /* TRACE_FUNCTION ("cc_trace_bcap()"); */

  trace_buf[0] = '\0';
  if (direction EQ UPLINK)
  {
    switch (_decodedMsg[0])
    {
      case U_SETUP:
        strcpy (trace_buf, "U_SETUP:");
        break;
      case U_CALL_CONF:
        strcpy (trace_buf, "U_CCONF:");
        break;
      default:
        break;
    }
  }
  else
  {
    switch (_decodedMsg[0])
    {
      case D_SETUP:
        strcpy (trace_buf, "D_SETUP:");
        break;
      case D_CALL_PROCEED:
        strcpy (trace_buf, "D_CPROC:");
        break;
      default:
        break;
    }
  }

  str_len = strlen (trace_buf);
  msg_len = sdu->l_buf >> 3;
  msg_ptr = &sdu->buf[sdu->o_buf >> 3];

  if (str_len > 0)
  {
    /* Known message containing bearer caps, trace it out */
    unsigned count; /* Number of dumped hex bytes in the trace line */

    count = 0;

    if (msg_len > 40)
      msg_len = 40; /* Limit trace load */

    while (msg_len > 0)
    {
      sprintf (trace_buf + str_len," %02x", *msg_ptr);
      msg_ptr++;
      msg_len--;
      count++;
      str_len += 3;
      if ((msg_len > 0) AND (count >= 20))
      {
        /* Start new trace line */
        TRACE_EVENT (trace_buf);
        strcpy (trace_buf, "        ");
        str_len = strlen (trace_buf);
        count = 0;
      }
    }
    TRACE_EVENT (trace_buf);
  }
}
#endif /* #if defined TRACE_BCAP */
#endif /* #if 0 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_init                   |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the formatter

*/

GLOBAL void for_init (void)
{
  TRACE_FUNCTION ("for_init()");
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_est_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_EST_IND from mobility
            management.

*/

GLOBAL void for_mmcm_est_ind (T_MMCM_ESTABLISH_IND * mmcm_establish_ind)
{
  UBYTE     pd;
  BOOL      rel_flag = FALSE;
  GET_INSTANCE_DATA;

  /* Enable the PCO to correctly decode the message */
  PPASS (mmcm_establish_ind, data, MMCM_ESTABLISH_IND);

  TRACE_FUNCTION ("for_mmcm_est_ind()");

  cc_data->ti_ext = 0;
  cc_data->ti_ext_pres = FALSE;

  GET_PD (data->sdu, pd);
  GET_TI (data->sdu, cc_data->ti);
  cc_data->ti_rec = cc_data->ti;

  if (cc_data->ti >= 8)
    cc_data->ti -= 8;
  else
    cc_data->ti += 8;

  if ((pd NEQ M_MM_PD_CC)
      OR
      (data->sdu.l_buf < 16))
      /*
       * wrong protocol discriminator
       * or message too short
       */
  {
      rel_flag = TRUE;
  }
  else if ((cc_data->ti & 7) EQ 7)
  {
      /* TI value indicates extension octet, not allowed currently */
      /* retrieve the second octet for TI and store it for subsequent handling in or_decode */
      GET_TI_EXT(data->sdu, cc_data->ti_ext);
      if ((cc_data->ti_ext & EXTENDED_TI_VALID) EQ EXTENDED_TI_VALID)
      {
          cc_data->ti_ext_pres = TRUE;
      }
      else
      { /* no extended TI */
          rel_flag = TRUE;
      }
  }
  if (rel_flag==TRUE)
  {
      PPASS (data, rel, MMCM_RELEASE_REQ);
      rel->org_entity = NAS_ORG_ENTITY_CC;
      rel->ti = cc_data->ti;
      PSENDX (MM, rel);
      return;

  }

    cc_data->error       = 0;
    cc_data->error_count = 0;
    for_decode ((T_PRIM*)(D2P(data)), TRUE);

} /* for_mmcm_est_ind() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_data_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCM_DATA_IND from mobility
            management.

*/

GLOBAL void for_mmcm_data_ind (T_MMCM_DATA_IND * mmcm_data_ind)
{
  UBYTE     pd;
  GET_INSTANCE_DATA;

  /* Enable the PCO to correctly decode the message */
  PPASS (mmcm_data_ind, data, MMCM_DATA_IND);

  TRACE_FUNCTION ("for_mmcm_data_ind()");

  cc_data->ti_ext = 0;
  cc_data->ti_ext_pres = FALSE;
  
  GET_PD (data->sdu, pd);
  GET_TI (data->sdu, cc_data->ti);
  cc_data->ti_rec = cc_data->ti;

  if (cc_data->ti >= 8)
    cc_data->ti -= 8;
  else
    cc_data->ti += 8;

  if (((cc_data->ti & 7) EQ 7)
       OR
      (pd NEQ M_MM_PD_CC)
       OR
      (data->sdu.l_buf < 16))
  /*
   * TI value is reserved, not allowed
   * or wrong protocol discriminator
   * or message too short
   */
  {
    PFREE (data);
  }
  else
  {
    cc_data->error = 0;
    for_decode ((T_PRIM*)(D2P(data)), FALSE);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_decode                 |
+--------------------------------------------------------------------+

  PURPOSE : Decoding and distributing of incoming messages.

*/
typedef void (*T_FUNC_PTR) (void);

LOCAL const T_FUNC_PTR mess_table[] = {
  cc_unknown_message,             /*  0: not used                  */
  for_d_alert,                    /*  1: D_ALERT                   */
  for_d_call_proceed,             /*  2: D_CALL_PROCEED            */
  for_d_progress,                 /*  3: D_PROGRESS                */
  for_d_cc_establishment,         /*  4: D_CC_ESTABLISHMENT        */
  for_d_setup,                    /*  5: D_SETUP                   */
  cc_unknown_message,             /*  6: not used                  */
  for_d_connect,                  /*  7: D_CONNECT                 */
  cc_unknown_message,             /*  8: not used                  */
  cc_unknown_message,             /*  9: not used                  */
  cc_unknown_message,             /* 10: not used                  */
  for_d_recall,                   /* 11: D_RECALL                  */
  cc_unknown_message,             /* 12: not used                  */
  cc_unknown_message,             /* 13: not used                  */
  cc_unknown_message,             /* 14: not used                  */
  cc_connect_ack,                 /* 15: B_CONNECT_ACK             */
  for_b_user_information,         /* 16: B_USER_INFO               */
  cc_unknown_message,             /* 17: not used                  */
  cc_unknown_message,             /* 18: not used                  */
  for_b_modify_rej,               /* 19: B_MODIFY_REJ              */
  cc_unknown_message,             /* 20: not used                  */
  cc_unknown_message,             /* 21: not used                  */
  cc_unknown_message,             /* 22: not used                  */
  for_b_modify,                   /* 23: B_MODIFY                  */
  cc_unknown_message,             /* 24: not used                  */
  cc_hold_ack,                    /* 25: D_HOLD_ACK                */
  for_d_hold_rej,                 /* 26: D_HOLD_REJ                */
  cc_unknown_message,             /* 27: not used                  */
  cc_unknown_message,             /* 28: not used                  */
  cc_retrieve_ack,                /* 29: D_RETRIEVE_ACK            */
  for_d_retrieve_rej,             /* 30: D_RETRIEVE_REJ            */
  for_b_modify_comp,              /* 31: B_MODIFY_COMP             */
  cc_unknown_message,             /* 32: not used                  */
  cc_unknown_message,             /* 33: not used                  */
  cc_unknown_message,             /* 34: not used                  */
  cc_unknown_message,             /* 35: not used                  */
  cc_unknown_message,             /* 36: not used                  */
  for_d_disconnect,               /* 37: D_DISCONNECT              */
  cc_unknown_message,             /* 38: not used                  */
  cc_unknown_message,             /* 39: not used                  */
  cc_unknown_message,             /* 40: not used                  */
  cc_unknown_message,             /* 41: not used                  */
  for_d_release_comp,             /* 42: D_RELEASE_COMP            */
  cc_unknown_message,             /* 43: not used                  */
  cc_unknown_message,             /* 44: not used                  */
  for_d_release,                  /* 45: D_RELEASE                 */
  cc_unknown_message,             /* 46: not used                  */
  cc_unknown_message,             /* 47: not used                  */
  cc_unknown_message,             /* 48: not used                  */
  cc_unknown_message,             /* 49: not used                  */
  cc_stop_dtmf_ack,               /* 50: D_STOP_DTMF_ACK           */
  cc_unknown_message,             /* 51: not_used                  */
  cc_status_enquiry,              /* 52: B_STATUS_ENQ              */
  cc_unknown_message,             /* 53: not used                  */
  for_d_start_dtmf_ack,           /* 54: D_START_DTMF_ACK          */
  for_d_start_dtmf_rej,           /* 55: D_START_DTMF_REJ          */
  cc_unknown_message,             /* 56: not used                  */
  for_b_congest_ctrl,             /* 57: B_CONGEST_CTRL            */
  for_d_facility,                 /* 58: D_FACILITY                */
  cc_unknown_message,             /* 59: not used                  */
  cc_unknown_message,             /* 60: not used                  */
  for_b_status,                   /* 61: B_STATUS                  */
  for_b_notify,                   /* 62: B_NOTIFY                  */
  cc_unknown_message              /* 63: not used                  */
};

GLOBAL void for_decode (T_PRIM * prim, BOOL est_flag)
{
  GET_INSTANCE_DATA;
  U8 *payload;
  U16 length;
/* Implements Measure#32: Row 43,...,70 */

  PCAST (prim, data, MMCM_DATA_IND);

  TRACE_FUNCTION ("for_decode()");

  if (cc_data->ti_ext_pres EQ TRUE)
  {
    data->sdu.l_buf -= 16;
    data->sdu.o_buf += 16;
  }
  else
  {
    data->sdu.l_buf -= 8;
    data->sdu.o_buf += 8;
  }


  cc_data->mt = data->sdu.buf[data->sdu.o_buf >> 3];

  /* Obtain the internal shared CCD output buffer */
  CCD_START;

  if (ccd_decodeMsg (CCDENT_CC,
                     DOWNLINK,
                     (T_MSGBUF *) &data->sdu,
                     (UBYTE    *) _decodedMsg,
                     NOT_PRESENT_8BIT) NEQ ccdOK)
  {
    USHORT parlist[6];
    UBYTE  prev_err;
    UBYTE  ccd_err;

    memset (parlist,0, sizeof (parlist));
    ccd_err = ccd_getFirstError (CCDENT_CC, parlist);    
    /*
     * Error Handling
     */
    do      
        {
      TRACE_EVENT_P1 ("cuurent_error = %d", ccd_err);
          switch (ccd_err)
          {
        case ERR_MSG_LEN:                 /* ignore message & send STATUS #96 */
            case ERR_MAND_ELEM_MISS:          /* ignore message & send STATUS #96 */
            case ERR_COMPREH_REQUIRED:        /* ignore message & send STATUS #96 */

              for_set_mandatory_error ((UBYTE)parlist[0]);
              break;

            case ERR_INVALID_MID:             /* ignore message & send STATUS #97 */
              cc_data->error = M_CC_CAUSE_MESSAGE_TYPE_NOT_IMPLEM;
              break;

            case ERR_INTERNAL_ERROR:
#if defined (WIN32)              
              TRACE_EVENT_P1("An internal CCD error occured,CCD error code = %u", ccd_err);
#endif /* #if defined (WIN32) */
              cc_data->error = M_CC_CAUSE_MESSAGE_INCOMPAT;
              break;
            
            case ERR_IE_SEQUENCE:
#if defined (WIN32) 
              TRACE_EVENT_P2("sequence error,  CCD error = %u, Parameter = %d", ccd_err, parlist[0]);
#endif /* #if defined (WIN32) */
               switch (cc_data->state[srv_convert_ti (cc_data->ti)])
               {
                 case M_CC_CS_11:
                 case M_CC_CS_12:
                 case M_CC_CS_19:
                   break;
                 default: /* 24.008: 9.3.18.1.1	Cause ; This information element shall 
                            * be included if this message is used to initiate call clearing.
                            */

                   if (parlist [0] EQ CAUSE_IEI)
                   {
                     for_set_mandatory_error ((UBYTE)parlist[0]);
                   }
                   break;
               }
               break;
       
#if defined (WIN32) /* This part is interesting only in implementation 
                       and testing; may be included if specific handling
                       for specific errors is required in future releases */
            case ERR_IE_NOT_EXPECTED:
              TRACE_EVENT_P1("CCD ERROR: IE not expected. probably an R99 optional elemet like cell notification, eplmn, or t3302, CCD error code = %u", ccd_err);
              break;
            case ERR_MAX_IE_EXCEED:
              TRACE_EVENT_P1("Maximum amount of repeatable information elements has exceeded,  CCD error code = %u", ccd_err);
              break;
            case ERR_MAX_REPEAT:
              TRACE_EVENT_P1("A repeatable element occurs too often in the message,  CCD error code = %u", ccd_err);
              break;
            case ERR_PATTERN_MISMATCH:
            case ERR_INVALID_TYPE:
              TRACE_EVENT_P1 ("MNC decoding erroneous = %u", ccd_err);
              break;
#endif /* #if defined (WIN32) */
            default:
#if defined (WIN32)
  TRACE_EVENT_P1 ("Unexpected warnings/errors = %u", ccd_err);
#endif /* #if defined (WIN32) */
              break; /* No or non-interesting error */
          }           
          
              prev_err = ccd_err;
              ccd_err = ccd_getNextError (CCDENT_CC, parlist);          
        } while ( prev_err NEQ ERR_NO_MORE_ERROR                       
                  AND !cc_check_critical_error(cc_data->error)  );                  
  }
 /*lint -e416 (creation of out-of-bounds pointer)*/
  EM_CC_DOWNLINK_SETUP_RECEIVED;

  /* adding TI and PD header to data */
  data->sdu.l_buf += 8;
  data->sdu.o_buf -= 8;

  payload  = &(data->sdu.buf[0]);          /* beginning of buffer      */
  payload += ((data->sdu.o_buf) >> 3);     /*  plus offset (bytes)     */
  length   = ((data->sdu.l_buf) >> 3);     /* length (bytes, bits / 8) */

/* Implements Measure#32: Row 43,...,70 */
  switch (_decodedMsg[0])
  {
    case D_ALERT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Alert DL ",
                    payload,
                    length);
      break;
    case D_CALL_PROCEED:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Call Proceed DL ",
                    payload,
                    length);
      break;
    case D_PROGRESS:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Progress DL ",
                    payload,
                    length);
      break;
    case D_CC_ESTABLISHMENT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " CC Establishment DL ",
                    payload,
                    length);
      break;
    case D_SETUP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Setup DL ",
                    payload,
                    length);
      break;
    case D_CONNECT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Connect DL ",
                    payload,
                    length);
      break;
    case D_RECALL:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Recall DL ",
                    payload,
                    length);
      break;
    case B_CONNECT_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Connect Acknowledge DL ",
                    payload,
                    length);
      break;
    case B_USER_INFO:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " User Information DL ",
                    payload,
                    length);
      break;
    case B_MODIFY_REJ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Modification Reject DL ",
                    payload,
                    length);
      break;
    case B_MODIFY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Call Modify DL ",
                    payload,
                    length);
      break;
    case D_HOLD_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Hold Acknowledge DL ",
                    payload,
                    length);
      break;
    case D_HOLD_REJ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Hold Reject DL ",
                    payload,
                    length);
      break;
    case D_RETRIEVE_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Retrieve Acknowledge DL ",
                    payload,
                    length);
      break;
    case D_RETRIEVE_REJ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Retrieve Reject DL ",
                    payload,
                    length);
      break;
    case B_MODIFY_COMP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Modify Complete DL ",
                    payload,
                    length);
      break;
    case D_DISCONNECT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Disconnect DL ",
                    payload,
                    length);
      break;
    case D_RELEASE_COMP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Release Complete DL ",
                    payload,
                    length);
      break;
    case D_RELEASE:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Release DL ",
                    payload,
                    length);
      break;
    case D_STOP_DTMF_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Stop DTMF Acknowledge DL ",
                    payload,
                    length);
      break;
    case B_STATUS_ENQ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Status Enquiry DL ",
                    payload,
                    length);
      break;
    case D_START_DTMF_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Start DTMF Acknowledge DL ",
                    payload,
                    length);
      break;
    case D_START_DTMF_REJ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Start DTMF Reject DL ",
                    payload,
                    length);
      break;
    case B_CONGEST_CTRL:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Congestion Control DL ",
                    payload,
                    length);
      break;
    case D_FACILITY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Facility DL ",
                    payload,
                    length);
      break;
    case B_STATUS:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Status DL ",
                    payload,
                    length);
      break;
    case B_NOTIFY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Notify DL ",
                    payload,
                    length);
      break;
    default:
      TRACE_EVENT("Unknown Message");
      break;
  } /*switch(_decodedMsg[0]) */

/* removing TI and PD header from data */
  data->sdu.l_buf -= 8;
  data->sdu.o_buf += 8;

#if 0 
#if defined TRACE_BCAP
  cc_trace_bcap (cc_data, &data->sdu, DOWNLINK);
#endif /* #if defined TRACE_BCAP */
#endif /* #if 0 */

  PFREE (data);

  /*
   * Only the range 0 to 63 is relevant for the cc message type.
   */
  if (est_flag)
  {
    /* 
     * The message was received by MMCC_ESTABLISH_IND.
     */
    UBYTE result;

    switch (_decodedMsg[0])
    {
      case D_SETUP:
        if ((result = srv_define_ti ()) EQ NOT_PRESENT_8BIT)
        {
          /* 
           * Internal problem: CC has run out of call table entries. 
           * Release the call.
           */
          if ( cc_data->ti_rec < 8)   /* 24.008; ch. 8.3.1 c) */
          {
            /* 
             * for a reasoning why CAUSE_USER_BUSY was chosen see the 
             * definition of UDUB in GSM 02.01!
             */
            for_send_u_release_comp (M_CC_CAUSE_USER_BUSY);
            return;
          }
	}

        if (cc_data->ti_ext_pres EQ TRUE)
        {
          /* [ 24.008; ch. 8.3.1]
           * The mobile station and network shall reject a SETUP, EMERGENCY SETUP or START CC
           * message received with octet 1 part of the TI value coded as "111" by sending 
           * RELEASE COMPLETE with cause #81 "Invalid transaction identifier value" The TI value
           * in RELEASE COMPLETE shall be the complete TI value including the extension octet
           * from the message that caused the rejection.
           */

          for_send_u_release_comp (M_CC_CAUSE_INVALID_TI);
          return;            
        }

        cc_data->index_ti = result;
        break;

      case B_STATUS:
        break;

      case D_RELEASE_COMP:
        CCD_END;
        for_rel_req ();
        return;

      default:
        for_send_u_release_comp (M_CC_CAUSE_INVALID_TI);
        return;
    } /* switch (_decodedMsg[0]) */
  }
  else
  {
    /* 
     * The message was received by MMCC_DATA_IND.
     */
    if ((cc_data->index_ti = srv_convert_ti (cc_data->ti)) 
        EQ NOT_PRESENT_8BIT)
    {
      for_send_u_release_comp (M_CC_CAUSE_INVALID_TI);
      return;
    }
  }
  if (_decodedMsg[0] < 64)
  {
    JUMP (*mess_table [_decodedMsg[0] & 63]) ();
  }
  else
  {
    cc_unknown_message ();
  }
} /* for_decode() */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_alert                |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_ALERT message.

*/
GLOBAL void for_d_alert (void)
{
  MCAST (alert, D_ALERT);

  TRACE_FUNCTION ("for_d_alert()");

  if (alert->v_progress)
  {
    if (for_check_progress_indicator (&alert->progress) EQ FALSE)
    {
      for_set_optional_error (PROGRESS_IEI);
    }
  }

  cc_alert (alert);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_call_proceed         |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_CALL_PROCEED message.

*/

GLOBAL void for_d_call_proceed (void)
{
  MCAST (proceed, D_CALL_PROCEED);

  TRACE_FUNCTION ("for_d_call_proceed()");

  if (proceed->v_repeat)
  {
    /* Repeat indicator present */
    if (for_check_repeat_indicator (proceed->repeat) EQ FALSE)
    {
      /* Repeat indicator contains garbage */
      for_set_optional_error (REPEAT_IEI);
    }
    if (!(proceed->v_bearer_cap AND proceed->v_bearer_cap_2))
    {
      /* Repeat indicator, but not both bearer caps present */
      for_set_conditional_error (REPEAT_IEI);
    }
  }
  else
  {
    /* No repeat indicator present */
    if (proceed->v_bearer_cap AND proceed->v_bearer_cap_2)
    {
      /* No repeat indicator, but both bearer caps present */
      for_set_conditional_error (REPEAT_IEI);
    }
  }

  cc_call_proceeding (proceed);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_connect              |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_CONNECT message.

*/

GLOBAL void for_d_connect (void)
{
  MCAST (connect, D_CONNECT);

  TRACE_FUNCTION ("for_d_connect()");

  if (connect->v_progress)
  {
    if (for_check_progress_indicator (&connect->progress) EQ FALSE)
    {
      for_set_optional_error (PROGRESS_IEI);
    }
  }

  if (connect->v_connect_num)
  {
    if (for_check_calling_party_bcd
        ((T_M_CC_calling_num *)&connect->connect_num) EQ FALSE)
    {
      for_set_optional_error (CONNECTED_NUMBER_IEI);
    }
  }

  if (connect->v_connect_subaddr)
  {
    if (for_check_calling_party_sub
        ((T_M_CC_calling_subaddr *)&connect->connect_subaddr) EQ FALSE)
    {
      for_set_optional_error (CONNECTED_SUBADDR_IEI);
    }
  }

  cc_connect (connect);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_facility             |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_FACILITY message.

*/

GLOBAL void for_d_facility (void)
{
  MCAST (facility, D_FACILITY);

  TRACE_FUNCTION ("for_d_facility()");

  cc_facility (facility);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_disconnect           |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_DISCONNECT message.

*/

GLOBAL void for_d_disconnect (void)
{
  MCAST (disconnect, D_DISCONNECT);

  TRACE_FUNCTION ("for_d_disconnect()");

  if (disconnect->v_progress)
  {
    if (for_check_progress_indicator (&disconnect->progress) EQ FALSE)
    {
      for_set_optional_error (PROGRESS_IEI);
    }
  }

  if (for_check_cc_cause (&disconnect->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  cc_disconnect (disconnect);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_modify               |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_MODIFY message.

*/

GLOBAL void for_b_modify (void)
{
  MCAST (modify, B_MODIFY);

  TRACE_FUNCTION ("for_b_modify()");

  cc_modify (modify);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_modify_comp          |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_MODIFY_COMP message.

*/

GLOBAL void for_b_modify_comp (void)
{
  MCAST (modify_com, B_MODIFY_COMP);

  TRACE_FUNCTION ("for_b_modify_comp()");

  cc_modify_complete (modify_com);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_modify_rej           |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_MODIFY_REJ message.

*/

GLOBAL void for_b_modify_rej (void)
{
  MCAST (modify_rej, B_MODIFY_REJ);

  TRACE_FUNCTION ("for_b_modify_rej()");

  if (for_check_cc_cause (&modify_rej->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  cc_modify_reject (modify_rej);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_notify               |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_NOTIFY message.

Issue No 7674 :
Accept the message as according to Anite test case ETSI 11.10 clause 26.8.1.4.2 
and do nothing,no need to send any Notification Indication to ACI.Also check the
State of the CC, if we get a Notify in CC State other than U10 and U26 then Send
STATUS message with cause "Message Type Incompatible" to the Network.
*/

GLOBAL void for_b_notify (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_b_notify()");

  /* Processing an incoming notify message.*/
  switch (cc_data->state[cc_data->index_ti])
  {
      /*
       * CS_0 is handled by the formatter
       */
    case M_CC_CS_10:
    case M_CC_CS_26:
     
      CCD_END;
     
      break;

    default:
      CCD_END;
      /* Implements Measure#  3 and streamline encoding */
      cc_send_status (M_CC_CAUSE_MESSAGE_TYPE_INCOMPAT);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_progress             |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_PROGRESS message.

*/

GLOBAL void for_d_progress (void)
{
  MCAST (progress, D_PROGRESS);

  TRACE_FUNCTION ("for_d_progress()");

  if (for_check_progress_indicator (&progress->progress) EQ FALSE)
  {
    for_set_mandatory_error (PROGRESS_IEI);
  }

  cc_progress (progress);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_release              |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_RELEASE message.

*/

GLOBAL void for_d_release (void)
{
  MCAST (release, D_RELEASE);

  TRACE_FUNCTION ("for_d_release()");

  if (release->v_cc_cause)
  {
    if (for_check_cc_cause (&release->cc_cause) EQ FALSE)
    {
      for_set_optional_error (CAUSE_IEI);
    }
  }

  if (release->v_cc_cause_2)
  {
    if (for_check_cc_cause ((T_M_CC_cc_cause*)&release->cc_cause_2) EQ FALSE)
    {
      for_set_optional_error (CAUSE_IEI);
    }
  }

  cc_release (release);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_release_complete     |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_RELEASE_COMPLETE message.

*/

GLOBAL void for_d_release_comp (void)
{
  MCAST (rel_comp, D_RELEASE_COMP);

  TRACE_FUNCTION ("for_d_release_comp()");

  if (rel_comp->v_cc_cause)
  {
    if (for_check_cc_cause (&rel_comp->cc_cause) EQ FALSE)
    {
      for_set_optional_error (CAUSE_IEI);
    }
  }

  cc_release_complete (rel_comp);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_cc_establishment     |
+--------------------------------------------------------------------+

  PURPOSE : Processing of a D_CC_ESTABLISHMENT message.

*/

GLOBAL void for_d_cc_establishment (void)
{
  MCAST(cc_est, D_CC_ESTABLISHMENT);  
  T_U_SETUP * setup = NULL;
  USHORT len_in_bits = (cc_est->setup_cont.c_setup_msg + 1) << 3;
  BYTE        result;

  TRACE_FUNCTION ("for_d_cc_establishment()");
  
  /* 
   * Decode uplink setup message delivered in setup container.
   */
  {
    PALLOC_SDU (data, MMCM_DATA_REQ, len_in_bits);
    memcpy(&data->sdu.buf[1], 
           cc_est->setup_cont.setup_msg, 
           cc_est->setup_cont.c_setup_msg);
    data->sdu.buf[0] = U_SETUP;
    data->sdu.l_buf = len_in_bits;
    data->sdu.o_buf = 0;
    result = ccd_decodeMsg (CCDENT_CC,  
                            UPLINK,
                            (T_MSGBUF *) &data->sdu,
                            (UBYTE    *) _decodedMsg,
                            NOT_PRESENT_8BIT);

    setup = (T_U_SETUP *)_decodedMsg;
    PFREE (data);
  }

  if (result NEQ ccdOK)
  {
    USHORT parlist[6];
    UBYTE  first_err;

    memset (parlist,0, sizeof (parlist));
    first_err = ccd_getFirstError (CCDENT_CC, parlist);
    /*
     * Error Handling
     */
    switch (first_err)
    {
      case ERR_COMPREH_REQUIRED:    /* comprehension required     */
        for_set_mandatory_error ((UBYTE)parlist[0]);
        break;
      case ERR_MAND_ELEM_MISS:      /* Mandatory elements missing */
        /*
         * Error handling is carried out by checking the content.
         */
        break;
    }
  }

  /* Check repeat indicator bearer capabilities */
  if (setup->v_repeat )
  {
    if (for_check_repeat_indicator (setup->repeat) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }

  /* Check presence of bearer capability I */
  if (setup->v_bearer_cap) 
  {  
    /* Compability check not done here */
  }
  else
    for_set_mandatory_error (BEARER_CAP_IEI);
  
  /* Check calling party subaddress */
  if (setup->v_calling_subaddr) 
  {
    if (for_check_calling_party_sub (&setup->calling_subaddr) EQ FALSE)
    {
      for_set_optional_error (CALLING_PARTY_SUB_IEI);
    }
  }

  /* Check called party BCD number */
  if (setup->v_ul_called_num) 
  {
    if (for_check_called_party_bcd (setup->ul_called_num.ton, 
                                    setup->ul_called_num.npi) EQ FALSE)
    {
      for_set_mandatory_error (CALLED_PARTY_BCD_IEI);
    }
  } 
  else
  {
    for_set_mandatory_error (CALLED_PARTY_BCD_IEI);
  }

  /* Check called party subaddress */
  if (setup->v_called_subaddr)
  {
    if (for_check_called_party_sub (&setup->called_subaddr) EQ FALSE)
    {
      for_set_optional_error (CALLED_PARTY_SUB_IEI);
    }
  }

  /* Check LLC repeat indicator */
  if (setup->v_repeat_2) 
  {
    if (for_check_repeat_indicator (setup->repeat_2) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }

  /* Check HLC repeat indicator */
  if (setup->v_repeat_3)
  {
    if (for_check_repeat_indicator (setup->repeat_3) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }
  
  /* Check BC repeat indicator conditions */
  if (setup->v_repeat) 
  { 
    if (!(setup->v_bearer_cap AND setup->v_bearer_cap_2))
    {
      cc_for_set_conditional_error (REPEAT_IEI);
    }
  }

  /* Check LLC repeat indicator conditions */
  if (setup->v_repeat_2) 
  {
    if (!(setup->v_bearer_cap 
          AND setup->v_low_layer_comp
          AND (setup->v_repeat EQ setup->v_repeat_2)))
    {
      cc_for_set_conditional_error (REPEAT_IEI);
    }
  }

  /* Check second LLC IE condition */
  if (setup->v_low_layer_comp_2)
  {
    if (!setup->v_repeat_2)
    {
      cc_for_set_conditional_error (REPEAT_IEI);
    }
  }

  /* Check HLC repeat indicator conditions */
  if (setup->v_high_layer_comp_2) 
  {
    if (!(setup->v_repeat AND setup->v_high_layer_comp))
    {
      cc_for_set_conditional_error (REPEAT_IEI);
    }
  }

  /* Check second HLC IE condition */
  if (setup->v_high_layer_comp)
  {
    if (!setup->v_repeat_3)
    {
      cc_for_set_conditional_error (REPEAT_IEI);
    }
  }

  /* 
   * The only interesting part of the CC ESTABLISHMENT message 
   * is the setup container, so only this is delivered
   */
  cc_cc_establishment (setup);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_cc_establishment     |
+--------------------------------------------------------------------+

  PURPOSE : Processing of a D_RECALL message.

*/

GLOBAL void for_d_recall (void)
{
  MCAST (recall, D_RECALL);     

  TRACE_FUNCTION ("for_d_recall()");  

  cc_recall(recall);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_setup                |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_SETUP message.

*/

GLOBAL void for_d_setup (void)
{
  MCAST (setup, D_SETUP);
  UBYTE    rep_cnt = 0;
  UBYTE    bc_cnt  = 0;
  UBYTE    lc_cnt  = 0;
  UBYTE    hc_cnt  = 0;

  TRACE_FUNCTION ("for_d_setup()");

  if (setup->v_repeat)
  {
    if (for_check_repeat_indicator (setup->repeat) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }

  if (setup->v_progress)
  {
    if (for_check_progress_indicator (&setup->progress) EQ FALSE)
    {
      for_set_optional_error (PROGRESS_IEI);
    }
  }

  if (setup->v_signal)
  {
    if (for_check_signal (setup->signal) EQ FALSE)
    {
      for_set_optional_error (SIGNAL_IEI);
    }
  }

  if (setup->v_calling_num)
  {
    if (for_check_calling_party_bcd (&setup->calling_num) EQ FALSE)
    {
      for_set_optional_error (CALLING_PARTY_BCD_IEI);
    }
  }

  if (setup->v_calling_subaddr)
  {
    if (for_check_calling_party_sub (&setup->calling_subaddr) EQ FALSE)
    {
      for_set_optional_error (CALLING_PARTY_SUB_IEI);
    }
  }

  if (setup->v_dl_called_num)
  {
    if (for_check_called_party_bcd (setup->dl_called_num.ton, 
                                    setup->dl_called_num.npi) EQ FALSE)
    {
      for_set_optional_error (CALLED_PARTY_BCD_IEI);
    }
  }

  if (setup->v_called_subaddr)
  {
    if (for_check_called_party_sub (&setup->called_subaddr) EQ FALSE)
    {
      for_set_optional_error (CALLED_PARTY_SUB_IEI);
    }
  }

  if (setup->v_repeat_2)
  {
    if (for_check_repeat_indicator (setup->repeat_2) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }

  if (setup->v_repeat_3)
  {
    if (for_check_repeat_indicator (setup->repeat_3) EQ FALSE)
    {
      for_set_optional_error (REPEAT_IEI);
    }
  }

  rep_cnt = setup->v_repeat +
            setup->v_repeat_2 +
            setup->v_repeat_3;

  bc_cnt  = setup->v_bearer_cap +
            setup->v_bearer_cap_2;

  lc_cnt  = setup->v_low_layer_comp +
            setup->v_low_layer_comp_2;

  hc_cnt  = setup->v_high_layer_comp +
            setup->v_high_layer_comp_2;

  switch (rep_cnt)
  {
    case 0:
      if ((bc_cnt EQ 2) OR
          (lc_cnt EQ 2) OR
          (hc_cnt EQ 2))
      {
        for_set_conditional_error (REPEAT_IEI);
      }
      break;

    case 1:
      if ((bc_cnt NEQ 2) OR
          (lc_cnt EQ 2) OR
          (hc_cnt EQ 2))
      {
        for_set_conditional_error (REPEAT_IEI);
      }
      break;

    case 2:
      if ((bc_cnt NEQ 2) OR
          (lc_cnt NEQ 2) OR
          (hc_cnt EQ 2))
      {
        for_set_conditional_error (REPEAT_IEI);
      }
      break;

    case 3:
      if ((bc_cnt NEQ 2) OR
          (lc_cnt NEQ 2) OR
          (hc_cnt NEQ 2))
      {
        for_set_conditional_error (REPEAT_IEI);
      }
      break;
  }

  cc_setup (setup);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_start_dtmf_ack       |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_START_DTMF_ACK message.

*/

GLOBAL void for_d_start_dtmf_ack (void)
{
  MCAST (start_ack, D_START_DTMF_ACK);

  TRACE_FUNCTION ("for_d_start_dtmf_ack()");

  cc_start_dtmf_ack (start_ack);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_start_dtmf_rej       |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_START_DTMF_REJ message.

*/

GLOBAL void for_d_start_dtmf_rej (void)
{
  MCAST (start_rej, D_START_DTMF_REJ);

  TRACE_FUNCTION ("for_d_start_dtmf_rej()");

  if (for_check_cc_cause (&start_rej->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  cc_start_dtmf_rej (start_rej);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_status               |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_STATUS message.

*/

GLOBAL void for_b_status (void)
{
  MCAST (status, B_STATUS); /* T_B_STATUS */

  TRACE_FUNCTION ("for_b_status()");
  if (for_check_cc_cause (&status->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  for_check_call_state (&status->call_state);

  cc_status (status);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_hold_rej             |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_HOLD_REJ message.

*/

GLOBAL void for_d_hold_rej (void)
{
  MCAST (hold_rej, D_HOLD_REJ);

  TRACE_FUNCTION ("for_d_hold_rej()");

  if (for_check_cc_cause (&hold_rej->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  cc_hold_rej (hold_rej);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_d_retrieve_rej         |
+--------------------------------------------------------------------+

  PURPOSE : Processing a D_RETRIEVE_REJ message.

*/

GLOBAL void for_d_retrieve_rej (void)
{
  MCAST (retrieve_rej, D_RETRIEVE_REJ);

  TRACE_FUNCTION ("for_d_retrieve_rej()");

  if (for_check_cc_cause (&retrieve_rej->cc_cause) EQ FALSE)
  {
    for_set_mandatory_error (CAUSE_IEI);
  }

  cc_retrieve_rej (retrieve_rej);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_congest_ctrl         |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_CONGEST_CTRL message.

*/

GLOBAL void for_b_congest_ctrl (void)
{
  MCAST (cong, B_CONGEST_CTRL);

  TRACE_FUNCTION ("for_b_congest_ctrl()");

  if (cong->v_cc_cause)
  {
    if (for_check_cc_cause (&cong->cc_cause) EQ FALSE)
    {
      for_set_optional_error (CAUSE_IEI);
    }
  }

  cc_congestion_control (cong);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_b_user_information     |
+--------------------------------------------------------------------+

  PURPOSE : Processing a B_USER_INFORMATION message.

*/

GLOBAL void for_b_user_information (void)
{
  
  MCAST (user, B_USER_INFO);

  TRACE_FUNCTION ("for_b_user_information()");

  cc_user_information (user);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_sync_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_SYNC_IND from mobility
            management.

*/

GLOBAL void for_mmcm_sync_ind (T_MMCM_SYNC_IND * sync)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_mmcm_sync_ind()");

  cc_data->ti = sync->ti;

  EM_CC_CHANNEL_MODE_CHANGE;     
  
  cc_sync_ind (sync);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_est_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_EST_CNF from mobility
            management.

*/

GLOBAL void for_mmcm_est_cnf (T_MMCM_ESTABLISH_CNF * est_cnf)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_mmcm_est_cnf()");

  cc_data->ti = est_cnf->ti;
  if ((cc_data->index_ti = srv_convert_ti (cc_data->ti))
      NEQ NOT_PRESENT_8BIT)
  {
    cc_est_cnf ();    
  }

  EM_CC_MM_CONNECTION_ESTABLISHED_MO;  
  
  PFREE (est_cnf);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_rel_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_REL_IND from mobility
            management.

*/
GLOBAL void for_mmcm_rel_ind (T_MMCM_RELEASE_IND * rel_ind)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_mmcm_rel_ind()");

  cc_data->ti = rel_ind->ti;
  if ((cc_data->index_ti = srv_convert_ti (cc_data->ti))
      NEQ NOT_PRESENT_8BIT)
  {
    cc_rel_ind (rel_ind->cause);
  }

  EM_CC_LOSS_OF_CONNECTION;
  
  PFREE (rel_ind);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_prompt_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_PROMPT_IND from mobility
            management. In the CC entity nothing is done with the
            primitve, a transaction identifier still is not assigned.
            The reception of the primitive only causes the emission 
            of MNCC_PROMPT_IND to upper layers.

*/

GLOBAL void for_mmcm_prompt_ind (T_MMCM_PROMPT_IND * mmcm_prompt_ind)
{
  TRACE_FUNCTION ("for_mmcm_prompt_ind()");
  
  PFREE (mmcm_prompt_ind);

  {
    PALLOC (mncc_prompt_ind, MNCC_PROMPT_IND);
    PSENDX (MMI, mncc_prompt_ind);

//    EM2
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_err_ind           |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_ERR_IND from mobility
            management.

*/

GLOBAL void for_mmcm_err_ind (T_MMCM_ERROR_IND * err_ind)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_mmcm_err_ind()");

  EM_CC_MM_CONNECTION_FAILED;

  cc_data->ti = err_ind->ti;
  if ((cc_data->index_ti = srv_convert_ti (cc_data->ti))
      NEQ NOT_PRESENT_8BIT)
  {
    cc_err_ind (err_ind);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_mmcm_reest_cnf         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the primitive MMCC_REEST_CNF from mobility
            management.

*/

GLOBAL void for_mmcm_reest_cnf (T_MMCM_REESTABLISH_CNF * reest_cnf)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_mmcm_reest_cnf()");

  cc_data->ti = reest_cnf->ti;
  if ((cc_data->index_ti = srv_convert_ti (cc_data->ti))
      NEQ NOT_PRESENT_8BIT)
  {
    cc_reest_cnf ();
  }

  EM_CC_REESTABLISHED_SUCCESSFUL;

  PFREE (reest_cnf);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_est_req                |
+--------------------------------------------------------------------+

  PURPOSE : Build the primitive MMCC_EST_REQ and send them to
            mobility management.

*/

GLOBAL void for_est_req (T_MMCM_ESTABLISH_REQ * mmcm_establish_req)
{
  TRACE_FUNCTION ("for_est_req()");

  EM_CC_START_MO_CALL;  /*Needs perhaps a review */    

  PSENDX (MM, mmcm_establish_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_rel_req                |
+--------------------------------------------------------------------+

  PURPOSE : Build the primitive MMCC_RELEASE_REQ and send them to
            mobility management.

*/

GLOBAL void for_rel_req (void)
{
  GET_INSTANCE_DATA;

  PALLOC (rel, MMCM_RELEASE_REQ);

  TRACE_FUNCTION ("for_rel_req()");
  rel->org_entity = NAS_ORG_ENTITY_CC;
  rel->ti = cc_data->ti;

  PSENDX (MM, rel);

  EM_CC_RELEASE_SENT;     

  if (cc_data->ti NEQ cc_data->setup_reattempt_ti)
  {
    srv_free_ti ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_reest_req              |
+--------------------------------------------------------------------+

  PURPOSE : Build the primitive MMCC_REEST_REQ and send them to
            mobility management.

*/

GLOBAL void for_reest_req (void)
{
  GET_INSTANCE_DATA;

  PALLOC (reest, MMCM_REESTABLISH_REQ);

  TRACE_FUNCTION ("for_reest_req()");

  reest->ti = cc_data->ti;

  PSENDX (MM, reest);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_start_cc               |
+--------------------------------------------------------------------+

  PURPOSE : Code the START CC message and send it to MM.

*/

GLOBAL void for_start_cc (T_U_START_CC * start_cc)
{
  TRACE_FUNCTION ("for_start_cc()");
  
/* Implements Measure#  38 */
  code_msg ((UBYTE *)start_cc, BSIZE_U_START_CC);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_cc_est_confirm         |
+--------------------------------------------------------------------+

  PURPOSE : Code the CC EST. CONFIRMED message and send it to MM.

*/

GLOBAL void for_cc_est_confirm (T_U_CC_EST_CONF * est_cnf) 
{
  TRACE_FUNCTION ("for_cc_est_confirm()");

/* Implements Measure#  38 */
  code_msg ((UBYTE *)est_cnf, BSIZE_U_CC_EST_CONF);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_setup                  |
+--------------------------------------------------------------------+

  PURPOSE : Code the message setup. The setup
            message is here not sent to MM, but stored.

*/

GLOBAL void for_setup (T_U_SETUP * setup)
{
  GET_INSTANCE_DATA;
  /*
   * the maximum length of the outgoing setup message is 128 Bytes plus
   * the length of the outgoing facility element
   */

  USHORT len_in_bits = LEN_U_SETUP;

  if (setup->v_facility)
    len_in_bits += setup->facility.c_fac * 8;
  {
    PALLOC_SDU (data, MMCM_DATA_REQ, len_in_bits);

    TRACE_FUNCTION ("for_setup()");

    data->sdu.o_buf = CC_ENCODE_OFFSET;

    ccd_codeMsg (CCDENT_CC,
                 UPLINK,
                 (T_MSGBUF *) &data->sdu,
                 (UBYTE    *) setup,
                 NOT_PRESENT_8BIT);

    EM_CC_SENT_SETUP_MESSAGE;/*lint !e416 (creation of out-of-bounds pointer)*/
#if 0
#if defined TRACE_BCAP
    cc_trace_bcap (cc_data, &data->sdu, UPLINK);
#endif
#endif /* #if 0 */

    srv_free_stored_setup (); /* Security */
    cc_data->stored_setup = data;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_emergency_setup        |
+--------------------------------------------------------------------+

  PURPOSE : Code the message emergency setup. The emergency setup
            message is here not sent to MM, but stored.

*/

GLOBAL void for_emergency_setup (T_U_EMERGE_SETUP * emerg_setup)
{
  GET_INSTANCE_DATA;

  PALLOC_MSG (data, MMCM_DATA_REQ, U_EMERGE_SETUP);

    TRACE_FUNCTION ("for_emergency_setup()");

    data->sdu.o_buf = CC_ENCODE_OFFSET;
    ccd_codeMsg (CCDENT_CC,
                 UPLINK,
                 (T_MSGBUF *) &data->sdu,
                 (UBYTE    *) emerg_setup,
                 NOT_PRESENT_8BIT);

    EM_CC_SENT_EMERGENCY_SETUP_MESSAGE;

    srv_free_stored_setup (); /* Security */
    cc_data->stored_setup = data;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_status                 |
+--------------------------------------------------------------------+

  PURPOSE : Code the message status and send them to
            mobility management.

*/

GLOBAL void for_status (T_B_STATUS * status)
{
    TRACE_FUNCTION ("for_status()");
/* Implements Measure#  22 */
  code_msg ((UBYTE *)status, BSIZE_B_STATUS);
    EM_CC_STATUS_SENT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_release_complete       |
+--------------------------------------------------------------------+

  PURPOSE : Code the message release_complete and send them to
            mobility management.

*/

GLOBAL void for_release_complete (T_U_RELEASE_COMP * rel_com)
{
  GET_INSTANCE_DATA;
  /*
   * the sdu has a maximum length of 34 bytes.
   * User-User, facility and ss version are never used
   */
  PALLOC_SDU (data, MMCM_DATA_REQ, LEN_U_RELEASE_COMPLETE);

  TRACE_FUNCTION ("for_release_complete()");

  if (cc_data->ti_ext_pres EQ TRUE)
  {
    data->sdu.o_buf = (USHORT)(CC_ENCODE_OFFSET+8);  /* additional byte for extended TI */
  }
  else
  {
    data->sdu.o_buf = CC_ENCODE_OFFSET;
  }

  ccd_codeMsg (CCDENT_CC,
               UPLINK,
               (T_MSGBUF *) &data->sdu,
               (UBYTE    *) rel_com,
               NOT_PRESENT_8BIT);

  EM_CC_RELEASE_COMPLETE_SENT;
  
  for_pd (data);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_disconnect             |
+--------------------------------------------------------------------+

  PURPOSE : Code the message disconnect and send them to
            mobility management.

*/

GLOBAL void for_disconnect (T_U_DISCONNECT * disconnect)
{
  /*
   * The mandatory part has a length of 33 bytes.
   * User-User, facility and ss version are never used.
   */
  TRACE_FUNCTION ("for_disconnect()");
/* Implements Measure#  11 */
  code_msg ((UBYTE *)disconnect, LEN_U_DISCONNECT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_modify_reject          |
+--------------------------------------------------------------------+

  PURPOSE : Code the message modify reject and send them to
            mobility management.

*/

GLOBAL void for_modify_reject (T_B_MODIFY_REJ * modify_rej)
{
  /*
   * The message has a maximum length of 47 Bytes.
   * HLC and LLC are never used.
   */
  TRACE_FUNCTION ("for_modify_reject()");
/* Implements Measure#  11 */
  code_msg ((UBYTE *)modify_rej, LEN_U_MODIFY_REJ);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_modify_complete        |
+--------------------------------------------------------------------+

  PURPOSE : Code the message modify complete and send them to
            mobility management.

*/
GLOBAL void for_modify_complete (T_B_MODIFY_COMP * mod_com)
{

    TRACE_FUNCTION ("for_modify_complete()");
/* Implements Measure#  27 */
  code_msg ((UBYTE *)mod_com, BSIZE_B_MODIFY_COMP);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_start_dtmf             |
+--------------------------------------------------------------------+

  PURPOSE : Code the message start dtmf and send them to
            mobility management.

*/

GLOBAL void for_start_dtmf ( T_U_START_DTMF * start_dtmf)
{
  TRACE_FUNCTION ("for_start_dtmf()");
/* Implements Measure#  22 */
  code_msg ((UBYTE *)start_dtmf, BSIZE_U_START_DTMF);
  EM_CC_START_DTMF; 
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_stop_dtmf              |
+--------------------------------------------------------------------+

  PURPOSE : Code the message stop dtmf and send them to
            mobility management.

*/

GLOBAL void for_stop_dtmf (void)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_stop_dtmf()");
/* Implements Measure 42  */
  for_fill_sdu_and_send (U_STOP_DTMF, BSIZE_U_STOP_DTMF);
  EM_CC_STOP_DTMF;      
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_call_confirm           |
+--------------------------------------------------------------------+

  PURPOSE : Code the message call confirm and send them to
            mobility management.

*/

/* Here measure Measure#  23 is removed since we are accessing data ptr in 
EM_CC_CALL_CONFIRM_SENT */
GLOBAL void for_call_confirm (T_U_CALL_CONF * call_confirm)
{
  GET_INSTANCE_DATA;
  PALLOC_MSG (data, MMCM_DATA_REQ, U_CALL_CONF);

  TRACE_FUNCTION ("for_call_confirm()");

  data->sdu.o_buf = CC_ENCODE_OFFSET;
  ccd_codeMsg (CCDENT_CC,
               UPLINK,
               (T_MSGBUF *) &data->sdu,
               (UBYTE    *) call_confirm,
               NOT_PRESENT_8BIT);

  EM_CC_CALL_CONFIRM_SENT;/*lint !e416 (creation of out-of-bounds pointer)*/
#if 0
#if defined TRACE_BCAP
  cc_trace_bcap (cc_data, &data->sdu, UPLINK);
#endif /* #if defined TRACE_BCAP */
#endif /* #if 0 */

  for_pd (data);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_release                |
+--------------------------------------------------------------------+

  PURPOSE : Code the message release and send them to
            mobility management.

*/

GLOBAL void for_release (T_U_RELEASE * release)
{
  /*
   * the message has a maximum length of 34 bytes.
   * It does not include cc_cause2, facility,
   * user-user or ss version.
   */
  TRACE_FUNCTION ("for_release()");

/* Implements Measure#  12 */
  code_msg ((UBYTE *)release, LEN_U_RELEASE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_connect_ack            |
+--------------------------------------------------------------------+

  PURPOSE : Code the message connect ack and send them to
            mobility management.

*/

GLOBAL void for_connect_ack (T_B_CONNECT_ACK *b_connect_ack )
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("for_connect_ack()");
/* Implements Measure 34  */
  for_fill_sdu_and_send (B_CONNECT_ACK, BSIZE_B_CONNECT_ACK);
  EM_CC_CONNECT_ACKNOWLEDGE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_retrieve               |
+--------------------------------------------------------------------+

  PURPOSE : Code the message retrieve and send them to
            mobility management.

*/

GLOBAL void for_retrieve (void)
{
  TRACE_FUNCTION ("for_retrieve()");

/* Implements Measure 42  */
  for_fill_sdu_and_send (U_RETRIEVE, BSIZE_U_RETRIEVE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_hold                   |
+--------------------------------------------------------------------+

  PURPOSE : Code the message hold and send them to
            mobility management.

*/

GLOBAL void for_hold (void)
{
  TRACE_FUNCTION ("for_hold()");

/* Implements Measure 42  */
  for_fill_sdu_and_send (U_HOLD, BSIZE_U_HOLD);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_user_information       |
+--------------------------------------------------------------------+

  PURPOSE : Code the message user information and send them to
            mobility management.

*/

GLOBAL void for_user_information (T_B_USER_INFO * user_info)
{
  GET_INSTANCE_DATA;

  /*
   * The length of the outgoing message depends on
   * the length of the user information plus message
   * type, transaction identifier and the more data flag.
   */
  PALLOC_SDU (data, MMCM_DATA_REQ, 
              (USHORT)(LEN_U_USER_USER + user_info->user_user.c_info * 8));

  TRACE_FUNCTION ("for_user_information()");

  EM_CC_USUER_TO_USER_DATA_SENT;

  data->sdu.o_buf = CC_ENCODE_OFFSET;
  ccd_codeMsg (CCDENT_CC,
               UPLINK,
               (T_MSGBUF *) &data->sdu,
               (UBYTE    *) user_info,
               NOT_PRESENT_8BIT);
  for_pd (data);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_congestion_control     |
+--------------------------------------------------------------------+

  PURPOSE : Code the message congestion control and send them to
            mobility management.

*/

GLOBAL void for_congestion_control (T_B_CONGEST_CTRL * cong)
{
  TRACE_FUNCTION ("for_congestion_control()");
/* Implements Measure#  27 */
  code_msg ((UBYTE *)cong, BSIZE_B_CONGEST_CTRL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_modify                 |
+--------------------------------------------------------------------+

  PURPOSE : Code the message modify and send them to
            mobility management.

*/

GLOBAL void for_modify (T_B_MODIFY * modify)
{
  TRACE_FUNCTION ("for_modify()");

/* Implements Measure#  28 */
  code_msg ((UBYTE *)modify, BSIZE_B_MODIFY);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_connect                |
+--------------------------------------------------------------------+

  PURPOSE : Code the message connect and send them to
            mobility management.

*/

GLOBAL void for_connect (T_U_CONNECT * connect)
{
  /*
   * the message contains only message type
   * and transaction identifier and no
   * optional elements
   */

  TRACE_FUNCTION ("for_connect()");

/* Implements Measure#  13 */
  code_msg ((UBYTE *)connect, LEN_U_CONNECT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_alert                  |
+--------------------------------------------------------------------+

  PURPOSE : Code the message alert and send them to
            mobility management.

*/

GLOBAL void for_alert (T_U_ALERT * alert)
{
  /*
   * the message contains only message type and
   * transaction identifier and no optional
   * elements
   */

  TRACE_FUNCTION ("for_alert()");

/* Implements Measure#  14 */
  code_msg ((UBYTE *)alert, LEN_U_ALERT);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_facility               |
+--------------------------------------------------------------------+

  PURPOSE : Code the message facility and send them to
            mobility management.

*/

GLOBAL void for_facility (T_U_FACILITY * facility)
{
  /*
   * The length of the outgoing message depends
   * on the length of the facility information
   * element plus message type, transaction identifier
   * and ss version.
   */
  PALLOC_SDU (data, MMCM_DATA_REQ, 
              (USHORT)(LEN_U_FACILITY + facility->facility.c_fac * 8));

  TRACE_FUNCTION ("for_facility()");

  data->sdu.o_buf = CC_ENCODE_OFFSET;
  ccd_codeMsg (CCDENT_CC,
               UPLINK,
               (T_MSGBUF *) &data->sdu,
               (UBYTE    *) facility,
               NOT_PRESENT_8BIT);
  for_pd (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_pd                     |
+--------------------------------------------------------------------+

  PURPOSE : Add protocol discriminator and transaction identifier.

*/

GLOBAL void for_pd (T_MMCM_DATA_REQ * data)
{
  GET_INSTANCE_DATA;

  U8 *payload;
  U16 length;
  UBYTE *msg_type;
/* Implements Measure#32: Row 71,...,95 */
  

  TRACE_FUNCTION ("for_pd()");

  data->org_entity = NAS_ORG_ENTITY_CC;

  TRACE_EVENT_P1("extended TI %spresent", (cc_data->ti_ext_pres EQ TRUE)? "":"not ");
  if (cc_data->ti_ext_pres EQ TRUE)
  {
    data->sdu.o_buf -= 8;
    data->sdu.l_buf += 8;
    data->sdu.buf[data->sdu.o_buf >> 3] = cc_data->ti_ext;
  }

  SET_PD (data->sdu, M_MM_PD_CC);
  SET_TI (data->sdu, cc_data->ti);
  data->sdu.l_buf += 8;
  data->sdu.o_buf -= 8;

  msg_type = &(data->sdu.buf[CC_ENCODE_OFFSET >> 3]);/*lint !e415 !e416 Likely access or creation of out-of-bounds pointer*/
  payload  = &(data->sdu.buf[0]);           /* beginning of buffer      */
  payload += ((data->sdu.o_buf) >> 3);      /*  plus offset (bytes)     */
  length   = ((data->sdu.l_buf) >> 3);      /* length (bytes, bits / 8) */

  /*lint -e{415}  Likely access or creation of out-of-bounds pointer*/
  switch (*msg_type)
  {
    case U_SETUP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Setup UL ",
                    payload,
                    length);
      break;
    case U_EMERGE_SETUP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Emergency Setup UL ",
                    payload,
                    length);
      break;
    case U_START_CC:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Call Control Capability UL ",
                    payload,
                    length);
      break;
    case U_CC_EST_CONF:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Establishment Confirm UL ",
                    payload,
                    length);
      break;
    case B_STATUS:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Status UL ",
                    payload,
                    length);
      break;
    case U_DISCONNECT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Disconnect UL ",
                    payload,
                    length);
      break;
    case B_MODIFY_REJ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Modify Reject UL ",
                    payload,
                    length);
      break;
    case B_MODIFY_COMP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Modify Complete UL ",
                    payload,
                    length);
      break;
    case U_START_DTMF:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Start DTMF UL ",
                    payload,
                    length);
      break;
    case U_STOP_DTMF:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Stop DTMF UL ",
                    payload,
                    length);
      break;
    case U_CALL_CONF:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Call Confirm UL ",
                    payload,
                    length);
      break;
    case U_RELEASE :
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Release  UL ",
                    payload,
                    length);
      break;
    case B_CONNECT_ACK:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Connect Acknowledge UL ",
                    payload,
                    length);
      break;
    case U_RETRIEVE:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Retrieve UL ",
                    payload,
                    length);
      break;
    case U_HOLD:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Hold UL ",
                    payload,
                    length);
      break;
    case B_USER_INFO:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " User Info UL ",
                    payload,
                    length);
      break;
    case B_CONGEST_CTRL:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Congestion Control UL ",
                    payload,
                    length);
      break;
    case B_MODIFY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Modify UL ",
                    payload,
                    length);
      break;
    case U_CONNECT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Connect UL ",
                    payload,
                    length);
      break;
    case U_ALERT:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Alert UL ",
                    payload,
                    length);
      break;
    case U_FACILITY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    " Facility UL ",
                    payload,
                    length);
      break;
    case U_RELEASE_COMP:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    "Release Complete UL ",
                    payload,
                    length);
      break;
    case B_NOTIFY:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    "Notify UL ",
                    payload,
                    length);
      break;
    case B_STATUS_ENQ:
      TRACE_BINDUMP (cc_handle,
                    TC_USER4,
                    "Status Enquiry UL ",
                    payload,
                    length);
      break;
    default:
      TRACE_EVENT("Unknown Message");
      break;
  } /* switch (*msg_type) */
  PSENDX (MM, data);
}
/* Implements Measure#  11, 12, 13, 14, 22, 27, 28 and 38 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : code_msg                   |
+--------------------------------------------------------------------+

  PURPOSE : Code the message and add protocol discriminator and 
  transaction identifier.

*/

LOCAL void code_msg (UBYTE * mStruct,
                     USHORT len)
{
  TRACE_FUNCTION ("code_msg()");
  {
    /* Here we are using PALLOC_SDU for PALLOC_MSG also since they are same */
    PALLOC_SDU (data, MMCM_DATA_REQ, len);
  
    data->sdu.o_buf = CC_ENCODE_OFFSET;
    ccd_codeMsg (CCDENT_CC,
                 UPLINK,
                 (T_MSGBUF *) &data->sdu,
                 mStruct,
                 NOT_PRESENT_8BIT);
    for_pd (data);
  }
}


/* Implements Measure# 44 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_send_u_release_comp    |
+--------------------------------------------------------------------+

  PURPOSE : Send release complete.

*/
LOCAL void for_send_u_release_comp (USHORT cause)
{
  TRACE_FUNCTION ("for_send_u_release_comp()");

  CCD_END;
  CCD_START;
  /*
   * For MM the connection is still valid
   * CC has already released the connection.
   */
  {
    MCAST (rel_com, U_RELEASE_COMP);

    cc_build_release_complete (rel_com, 
                               CAUSE_MAKE(DEFBY_STD, 
                                          ORIGSIDE_MS, 
                                          MNCC_CC_ORIGINATING_ENTITY,
                                          cause));
    for_release_complete (rel_com);
  }
  CCD_END;
  for_rel_req ();
}

/* Implements Measure# 42 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : CC_FOR                     |
| STATE   : code                ROUTINE : for_retrieve_hold          |
+--------------------------------------------------------------------+

  PURPOSE : Send release complete.

*/

LOCAL void for_fill_sdu_and_send (UBYTE msg,
                                  USHORT len)
{  
  PALLOC_SDU (data, MMCM_DATA_REQ, len);
  data->sdu.o_buf = CC_ENCODE_OFFSET;
  data->sdu.l_buf = 8;
  /*lint -e{415} (warning-- Likely access or creation of out-of-bounds pointer)*/
  data->sdu.buf [CC_ENCODE_OFFSET >> 3] = msg;
  for_pd (data);
}
#endif

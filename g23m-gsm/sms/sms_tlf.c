/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SMS_TLF
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

#ifndef SMS_TLF_C
#define SMS_TLF_C

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

static const  UBYTE ref_nr_23430[] = {
  0xE, 0x3, 0x8, 0x7, 0x9, 0x5, 0xE, 0x3, 0x7};

/*==== VARIABLES ===================================================*/

/* Implements Measure#32: Row 84, 96, 87, 89, 95, 97, 104, 109 & 113 */
const char * const ef_sms_id = EF_SMS_ID;

/*==== FUNCTIONS ===================================================*/

/* Implements Measure# 14 */
/*
+---------------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                           |
| STATE   : code                ROUTINE : tl_send_sim_update_req            |
+---------------------------------------------------------------------------+

  PURPOSE : This routine process SIM update request and SMS memo resume request

*/


LOCAL void tl_send_sim_update_req(UBYTE trans_data, USHORT offset)
{
  GET_INSTANCE_DATA;
  UBYTE sim_acc_ix;
  TRACE_FUNCTION("tl_send_sim_update_req()");  
  /*
   * update sim notification flag
   */
  if (sms_data->sim_phase >= PHASE_2_SIM)
  {
    if (tl_sms_reserve_req_id(&sim_acc_ix))
    {
      PALLOC (update_req, SIM_UPDATE_REQ);

      update_req->source         = SRC_SMS;
      update_req->req_id         = sim_acc_ix;
      update_req->v_path_info    = FALSE;
      update_req->datafield      = 
        sms_data->sms_sim_access_info[sim_acc_ix].datafield   = SIM_SMSS;
      update_req->length         = 1;
      update_req->trans_data[0]  = trans_data;
      update_req->offset         = offset;

      PSENDX (SIM, update_req);

      if(sms_data->inst == INST_MO)
        tl_set_access_fifo (ACCESS_BY_MMI);
      else
        tl_set_access_fifo (ACCESS_BY_NET);
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_get_octet_len           |
|                               ROUTINE : tl_adjust_message_len      |
+--------------------------------------------------------------------+

  PURPOSE : Derive the number of octets of TP-User-Data and the whole
            message regarding the Data Coding Scheme.
*/

LOCAL BOOL tl_udl_count_septet (UBYTE dcs)
{
  BOOL isSeptet = TRUE;

  switch (dcs & 0xF0)
  {
  case 0x00:
  case 0x10:
    if ((dcs & 0xC) EQ 0x4 OR       /* 8-bit data */
        (dcs & 0xC) EQ 0x8)         /* UCS2 data */
      isSeptet = FALSE;
    break;
  case 0x20:                        /* compressed data */
  case 0x30:                        /* compressed data */
  case 0xE0:                        /* UCS2 data */
    isSeptet = FALSE;
    break;
  case 0xF0:
    if ((dcs & 0x4) NEQ 0)          /* 8-bit data */
      isSeptet = FALSE;
    break;
  }
  return isSeptet;
}

LOCAL USHORT tl_get_octet_len (USHORT tp_udl, UBYTE dcs)
{
  if (tl_udl_count_septet (dcs))    /* convert number of septets */
    return (USHORT)((tp_udl + 1) * 7 / 8);

  return tp_udl;
}

GLOBAL void tl_adjust_message_len (UBYTE tp_vt_mti, BUF_tpdu *tpdu)
{
  USHORT oct_len, pre_len;
  UBYTE dcs;
  UBYTE *tp_data, *tp_msg;

  if (tp_vt_mti EQ SMS_VT_SIM_PDU)
  {
    pre_len = tpdu->b_tpdu[0] + 1;
    tp_msg = &tpdu->b_tpdu[pre_len];
    switch (*tp_msg & 0x3)
    {
    case SMS_SUBMIT:
      tp_vt_mti = SMS_VT_SUBMIT;
      break;
    case SMS_DELIVER:
      tp_vt_mti = SMS_VT_DELIVER;
      break;
    case SMS_STATUS_REPORT:
      tp_vt_mti = SMS_VT_STATUS;
      break;
    default:
      return;
    }
  }
  else
  {
    pre_len = 0;
    tp_msg = &tpdu->b_tpdu[0];
  }
  switch (tp_vt_mti)
  {
  case SMS_VT_SUBMIT:
    oct_len = (tp_msg[2] + 1)/2;
    tp_data = &tp_msg[5 + oct_len];
    dcs = *tp_data;

    switch ((tp_msg[0] >> 3) & 0x3)  /* TP-VPF */
    {       /* point to TP-UDL */
    case SMS_VPF_RELATIVE:
      tp_data += 2;
      oct_len += 8;
      break;
    case SMS_VPF_ENHANCED:
    case SMS_VPF_ABSOLUTE:
      tp_data += 8;
      oct_len += 14;
      break;
    default:
      tp_data++;
      oct_len += 7;
      break;
    }
    oct_len += tl_get_octet_len (*tp_data, dcs);
    tpdu->l_tpdu = (oct_len + pre_len) << 3;
    break;
  case SMS_VT_DELIVER:
    oct_len = (tp_msg[1] + 1)/2;
    tp_data = &tp_msg[4 + oct_len];
    dcs = *tp_data;
    tp_data += 8;   /* point to TP-UDL */
    oct_len += (13 + tl_get_octet_len (*tp_data, dcs));
    tpdu->l_tpdu = (oct_len + pre_len) << 3;
    break;
    case SMS_VT_STATUS:
    oct_len = (tp_msg[2] + 1)/2 + 19;
    tp_data = &tp_msg[oct_len]; /* tp_data points to PI */
    if(*tp_data NEQ NOT_PRESENT_8BIT)
    {
      dcs = *(tp_data+2);
      tp_data += 3; /* point to TP-UDL */
      oct_len += (3 + tl_get_octet_len (*tp_data, dcs));
      tpdu->l_tpdu = (oct_len + pre_len) << 3;
    }
    else
    {
      tpdu->l_tpdu = (oct_len + pre_len) << 3;
    }
    break;
  default:
    break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_establish_connection    |
+--------------------------------------------------------------------+

  PURPOSE : Initiate the Establishment of a new connection

*/
GLOBAL void tl_establish_connection ( 
  BOOL            incr    /* true if reference nr increment needed */)
{
  GET_INSTANCE_DATA;
    UBYTE ti;
    TRACE_FUNCTION ("tl_establish_connection()");

    /*
     * new instance
     */
     GET_NEW_SMS_INSTANCE(0); 
    /*
     * increment TI
     */
     ti = csf_get_new_mo_ti();
#if defined (GPRS)
    /*
     * Set downlink according to prefs,
     * in case of LL it has to be checked first if up
     */
     if ( ( (sms_data->mo_dst_pref EQ GPRS_SMS_GPRS_PREF) ||
            (sms_data->mo_dst_pref EQ GPRS_SMS_GPRS_ONLY) ) /*&&
          (  sms_data->llc_flow   NEQ SMS_LLC_UNKNOWN     )*/ )
     {
       SMS_INST.downlink = SMS_DOWNLINK_LL_CHECK;
       TRACE_EVENT("downlink = SMS_DOWNLINK_LL_CHECK");
     }
     else
     {
       SMS_INST.downlink = SMS_DOWNLINK_MMSMS;
       TRACE_EVENT("downlink = SMS_DOWNLINK_MMSMS");
     }
#endif /* GPRS */
     if (incr)
     {
       /*
        * increment reference
        */
        ++SMS_INST.tp_mr;
       /*
        * SIM_UPDATE_REQ
        */
        tl_build_sim_update_req();
     }
     TRACE_EVENT_P2("TI=%u TP_MR=%u", ti, SMS_INST.tp_mr);
    /*
     * RL_ESTABLISH_REQ
     */
     rl_establish_req(ti);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_build_status_rep        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_BUILD_STATUS_REP.

*/

GLOBAL void tl_build_status_rep (T_rp_data_dl       *rp_data_dl,
                                 T_MNSMS_STATUS_IND *status_ind)
{
  T_SIM_PDU *sim_pdu;

  TRACE_FUNCTION ("tl_build_status_rep()");

  memset (&status_ind->sms_sdu, 0, sizeof(T_sms_sdu));

  if (rp_data_dl NEQ NULL)
  {
    MALLOC (sim_pdu, sizeof(T_SIM_PDU));

    sim_pdu->rp_addr = rp_data_dl->rp_addr;
    sim_pdu->tpdu = rp_data_dl->rp_user_data.tpdu;
    sim_pdu->tp_mti = SMS_STATUS_REPORT;
    sim_pdu->v_tpdu = TRUE;
    sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

    ccd_codeMsg (CCDENT_SMS, DOWNLINK,
                 (T_MSGBUF *)&status_ind->sms_sdu,
                 (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);

    MFREE (sim_pdu);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_modify_submit           |
+--------------------------------------------------------------------+

  PURPOSE : The SM taken from storage (rec_data) is modified with
            certain values from parameter 'sms_sdu' according to
            parameter 'modify'. The resulting message is returned in
            parameter 'sim_pdu'.
*/

LOCAL void tl_translate_mt_to_mo (T_TP_SUBMIT  *submit,
                                  T_TP_DELIVER *deliver)
{
  submit->tp_vt_mti = SMS_VT_SUBMIT;
  submit->tp_udhi = deliver->tp_udhi;
  submit->tp_mti = SMS_SUBMIT;
  submit->tp_mr = NOT_PRESENT_8BIT;
  submit->tp_pid = deliver->tp_pid;
  submit->tp_dcs = deliver->tp_dcs;
  if ((submit->v_tp_ud = deliver->v_tp_ud) NEQ 0)
  {
    submit->tp_ud = deliver->tp_ud;
    submit->v_tp_udh_inc = FALSE;
  }
  else if (submit->v_tp_udh_inc= deliver->v_tp_udh_inc)
    submit->tp_udh_inc= deliver->tp_udh_inc;
}

GLOBAL BOOL tl_modify_submit (T_SIM_PDU *sim_pdu,
                              UBYTE     modify,
                              T_sms_sdu *sms_sdu,
                              UBYTE     *rec_data)
{
  T_TP_SUBMIT *submit;
  T_sms_sdu *rec_sdu;
  union {
    T_TP_SUBMIT *submit;
    T_TP_DELIVER *deliver;
  } rec_msg;
  T_rp_addr rp_addr;

  TRACE_FUNCTION ("tl_modify_submit()");

  if (modify >= SMS_MODIFY_ALL)
    return TRUE;    /* ignore message from storage */

  if ((rec_data[0] & 1) EQ SMS_RECORD_FREE)
    return FALSE;   /* empty or invalid record */

  if (ccd_decodeMsg (CCDENT_SMS, BOTH, (T_MSGBUF *)sms_sdu,
                     (UBYTE *)sim_pdu, SMS_VT_SIM_PDU) EQ ccdError)
    return FALSE;

  rp_addr = sim_pdu->rp_addr;   /* keep SCA */

  MALLOC (submit, sizeof(T_TP_SUBMIT));
  memset (submit, 0, sizeof(T_TP_SUBMIT));

  if (ccd_decodeMsg (CCDENT_SMS, UPLINK, (T_MSGBUF *)&sim_pdu->tpdu,
                     (UBYTE *)submit, SMS_VT_SUBMIT) EQ ccdError)
  {
    MFREE (submit);
    return FALSE;
  }
  MALLOC (rec_sdu, sizeof(T_sms_sdu));

  rec_sdu->o_buf = 0;
  rec_sdu->l_buf = SIM_PDU_LEN<<3;
  memcpy (rec_sdu->buf, &rec_data[1], SIM_PDU_LEN);
  tl_adjust_message_len (SMS_VT_SIM_PDU, (BUF_tpdu *)rec_sdu);

  if (ccd_decodeMsg (CCDENT_SMS, BOTH, (T_MSGBUF *)rec_sdu,
                     (UBYTE *)sim_pdu, SMS_VT_SIM_PDU) EQ ccdError)
  {
    MFREE (rec_sdu);
    return FALSE;
  }
  MFREE (rec_sdu);

  switch (modify)
  {
  case SMS_MODIFY_SCA:
    sim_pdu->rp_addr = rp_addr; /* SCA from SUBMIT_REQ */
    /*FALLTHROUGH*/ /*lint -fallthrough*/
  case SMS_MODIFY_NON:
    switch (rec_data[0] & 7)
    {
    case SMS_RECORD_REC_UNREAD:
    case SMS_RECORD_REC_READ:
      if (sim_pdu->tp_mti NEQ SMS_DELIVER OR !sim_pdu->v_tpdu)
      {
        MFREE (submit);
        return FALSE;
      }
      MALLOC (rec_msg.deliver, sizeof(T_TP_DELIVER));
      memset (rec_msg.deliver, 0, sizeof(T_TP_DELIVER));

      if (ccd_decodeMsg (CCDENT_SMS, DOWNLINK,
                         (T_MSGBUF *)&sim_pdu->tpdu,
                         (UBYTE *)rec_msg.deliver,
                         SMS_VT_DELIVER) EQ ccdError)
      {
        MFREE (rec_msg.deliver);
        return FALSE;
      }
      tl_translate_mt_to_mo (submit, rec_msg.deliver);
      memcpy (&submit->tp_da, &rec_msg.deliver->tp_oa,
              sizeof(T_tp_da));
      MFREE (rec_msg.deliver);

      sim_pdu->tpdu.o_tpdu = 0;
      sim_pdu->tpdu.l_tpdu = TPDU_BIT_LEN;
      if (ccd_codeMsg (CCDENT_SMS, UPLINK,
                       (T_MSGBUF *)&sim_pdu->tpdu,
                       (UBYTE *)submit, SMS_VT_SUBMIT) NEQ ccdOK)
      {
        MFREE (submit);
        return FALSE;
      }
      MFREE (submit);
      break;
      /* sim_pdu shall contain a SMS_SUBMIT message */
    default:
      MFREE (submit);
      if (sim_pdu->tp_mti NEQ SMS_SUBMIT OR !sim_pdu->v_tpdu)
      {
        return FALSE;
      }
      break;
    }
    break;

  case SMS_MODIFY_TPOA_SCA:
    sim_pdu->rp_addr = rp_addr; /* SCA from SUBMIT_REQ */
    /* no break */
  case SMS_MODIFY_TPOA:
    switch (rec_data[0] & 7)
    {
    case SMS_RECORD_REC_UNREAD:
    case SMS_RECORD_REC_READ:
      if (sim_pdu->tp_mti NEQ SMS_DELIVER OR !sim_pdu->v_tpdu)
      {
        MFREE (submit);
        return FALSE;
      }
      MALLOC (rec_msg.deliver, sizeof(T_TP_DELIVER));
      memset (rec_msg.deliver, 0, sizeof(T_TP_DELIVER));

      if (ccd_decodeMsg (CCDENT_SMS, DOWNLINK,
                         (T_MSGBUF *)&sim_pdu->tpdu,
                         (UBYTE *)rec_msg.deliver,
                         SMS_VT_DELIVER) EQ ccdError)
      {
        MFREE (rec_msg.deliver);
        return FALSE;
      }
      tl_translate_mt_to_mo (submit, rec_msg.deliver);
      MFREE (rec_msg.deliver);

      sim_pdu->tpdu.o_tpdu = 0;
      sim_pdu->tpdu.l_tpdu = TPDU_BIT_LEN;
      if (ccd_codeMsg (CCDENT_SMS, UPLINK,
                       (T_MSGBUF *)&sim_pdu->tpdu,
                       (UBYTE *)submit, SMS_VT_SUBMIT) NEQ ccdOK)
      {
        MFREE (submit);
        return FALSE;
      }
      MFREE (submit);
      break;
      /* sim_pdu shall contain a SMS_SUBMIT message */
    default:
      if (sim_pdu->tp_mti NEQ SMS_SUBMIT OR !sim_pdu->v_tpdu)
      {
        MFREE (submit);
        return FALSE;
      }
      MALLOC (rec_msg.submit, sizeof(T_TP_SUBMIT));
      memset (rec_msg.submit, 0, sizeof(T_TP_SUBMIT));

      if (ccd_decodeMsg (CCDENT_SMS, UPLINK,
                         (T_MSGBUF *)&sim_pdu->tpdu,
                         (UBYTE *)rec_msg.submit,
                         SMS_VT_SUBMIT) EQ ccdError)
      {
        MFREE (rec_msg.submit);
        MFREE (submit);
        return FALSE;
      }
      rec_msg.submit->tp_da = submit->tp_da;
      MFREE (submit);

      sim_pdu->tpdu.o_tpdu = 0;
      sim_pdu->tpdu.l_tpdu = TPDU_BIT_LEN;
      if (ccd_codeMsg (CCDENT_SMS, UPLINK,
                       (T_MSGBUF *)&sim_pdu->tpdu,
                       (UBYTE *)rec_msg.submit, SMS_VT_SUBMIT) NEQ ccdOK)
      {
        MFREE (rec_msg.submit);
        return FALSE;
      }
      MFREE (rec_msg.submit);
      break;
    }
    break;

  default:
    MFREE (submit);
    break;
  }
  sim_pdu->tp_mti = SMS_SUBMIT;
  tl_adjust_message_len (SMS_VT_SUBMIT, &sim_pdu->tpdu);
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_prepare_submit          |
+--------------------------------------------------------------------+

  PURPOSE : Fills 'cp_data' with required parameters and returns TRUE.
            If the parameter check fails nothing is changed and
            FALSE is returned.

*/

GLOBAL BOOL tl_prepare_submit (
                               T_SIM_PDU   *sim_pdu,
			       T_U_CP_DATA *cp_data)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_prepare_submit()");

  if (sim_pdu->tp_mti EQ SMS_SUBMIT AND sim_pdu->v_tpdu)
  {
    // ++SMS_INST.tp_mr;
    sim_pdu->tpdu.b_tpdu[1] = SMS_INST.tp_mr;

    SMS_TP_REF_RET(sms_data) = SMS_INST.tp_mr;

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

    return TRUE;
  }

  TRACE_EVENT ("tl_prepare_submit() failed");

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_init_complete           |
+--------------------------------------------------------------------+

  PURPOSE :    tasks at initialisation (sim read record loop) completion

  DESCRIPTION: - set init_done flag
               - send smma message if necessary
               - send report indication to user if applicable

*/
GLOBAL void tl_init_complete (void)
{
  GET_INSTANCE_DATA;
  BOOL       notification_flag;
  BOOL       memory_available;
  BOOL       mem_avail_msg;

  TRACE_FUNCTION ("tl_init_complete()");
            
  /* 
   * check whether memory available message must be sent 
   */
   mem_avail_msg = FALSE;
   notification_flag = !sms_data->mem_cap_avail;

   if ( (notification_flag == TRUE) AND
        (sms_data->pr_cntrl.delivery_state == SMS_DELIVER_STATUS_RESUME) )
   {
     memory_available = (tl_get_free_space (MEM_SM) OR
                         tl_get_free_space (MEM_ME)); 
     if (memory_available == TRUE)
     {
      /*
       * initiate the sending of memory available message
       */
       mem_avail_msg = TRUE;
       GET_MO_INSTANCE(sms_data);
      /*
       * TL  state transition TL_ESTABLISH
       * EST state transition EST_SMMA
       */
       SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
       SET_STATE (STATE_EST, EST_SMMA);
      /*
       * 1st shot
       */
       SMS_INST.retrans  = FALSE;
      /*
       * establish connection
       */
       tl_establish_connection(FALSE);
     }
   }
   if (mem_avail_msg == FALSE)
   {
#ifdef GPRS
     cp_send_getunitdata_req ();
#endif
    /*
     * set state: READY
     */
     sms_data->ent_state = SMS_STATE_READY;
    /*
     * report ind to user
     */
     tl_mnsms_report_ind (SMS_STATE_READY);
   }
  /*
   * init done
   */
   sms_data->init_done = TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_convert_sim_error       |
+--------------------------------------------------------------------+

  PURPOSE : Converts a SIM error to the appropriate SMS failure code.

*/

GLOBAL USHORT tl_convert_sim_error (USHORT sim_error)
{
#if 0
  switch (sim_error)
  {
  case SIM_INVALID_PIN_1:
    return SMS_ERR_SIM_PIN1_REQ;
  case SIM_INVALID_PUK_1:
    return SMS_ERR_SIM_PUK1_REQ;
  case SIM_INVALID_PIN_2:
    return SMS_ERR_SIM_PIN2_REQ;
  case SIM_INVALID_PUK_2:
    return SMS_ERR_SIM_PUK2_REQ;
  case SIM_INVALID_OFFSET:
    return SMS_ERR_INV_INDEX;
  case SIM_FATAL_ERROR:
    return SMS_ERR_SIM_MISSING;
  default:
    return SMS_ERR_SIM_FAIL;
  }
#else
  return sim_error; // prelimary
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_convert_mo_to_mem       |
+--------------------------------------------------------------------+

  PURPOSE : Prepare a MO-SM to be stored.

*/

GLOBAL BOOL tl_convert_mo_to_mem (T_MMSMS_DATA_REQ *data_req,
                                  UBYTE            *data)
{
  T_SIM_PDU *sim_pdu;
  T_sms_sdu *sms_sdu;
  MCAST (cp_data, U_CP_DATA);

  TRACE_FUNCTION ("tl_convert_mo_to_mem()");

  data_req->sdu.l_buf -= 8;   // point to CP message type!
  data_req->sdu.o_buf += 8;
  if (ccd_decodeMsg (CCDENT_SMS, UPLINK,
                     (T_MSGBUF *)&data_req->sdu,
                     (UBYTE *)cp_data, NOT_PRESENT_8BIT) EQ ccdError)
  {
    return FALSE;
  }
  if (cp_data->msg_type NEQ U_CP_DATA OR
      cp_data->cp_user_data_ul.rp_mti NEQ RP_DATA_UL)
  {
    return FALSE;
  }
  MALLOC (sim_pdu, sizeof(T_SIM_PDU));
  MALLOC (sms_sdu, sizeof(T_sms_sdu));
  sms_sdu->o_buf = 0;
  sms_sdu->l_buf = SIM_PDU_LEN<<3;

  sim_pdu->rp_addr = cp_data->cp_user_data_ul.rp_data_ul.rp_addr;
  sim_pdu->tpdu = cp_data->cp_user_data_ul.rp_data_ul.rp_user_data.tpdu;
  sim_pdu->tp_mti = SMS_SUBMIT;
  sim_pdu->v_tpdu = TRUE;
  sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

  ccd_codeMsg (CCDENT_SMS, DOWNLINK,
               (T_MSGBUF *)sms_sdu,
               (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);

  /* set status byte */
  data[0] = SMS_RECORD_STO_SENT;
  memcpy (&data[1], sms_sdu->buf, sms_sdu->l_buf >> 3);
  memset (&data[(sms_sdu->l_buf >> 3) + 1], NOT_PRESENT_8BIT,
          (SIZE_EF_SMS-1) - (sms_sdu->l_buf >> 3));

  MFREE (sim_pdu);
  MFREE (sms_sdu);
//  CCD_END;
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_convert_mt_to_mem       |
+--------------------------------------------------------------------+

  PURPOSE : Prepare a MT-SM to be stored.

*/

GLOBAL void tl_convert_mt_to_mem (T_rp_data_dl *rp_data_dl,
                                  UBYTE        *data)
{
  T_SIM_PDU *sim_pdu;
  T_sms_sdu *sms_sdu;

  TRACE_FUNCTION ("tl_convert_mt_to_mem()");

  MALLOC (sim_pdu, sizeof(T_SIM_PDU));
  MALLOC (sms_sdu, sizeof(T_sms_sdu));
  sms_sdu->o_buf = 0;
  sms_sdu->l_buf = SIM_PDU_LEN<<3;

  sim_pdu->rp_addr = rp_data_dl->rp_addr;
  sim_pdu->tpdu = rp_data_dl->rp_user_data.tpdu;
  sim_pdu->tp_mti = SMS_DELIVER;
  sim_pdu->v_tpdu = TRUE;
  sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

  ccd_codeMsg (CCDENT_SMS, DOWNLINK,
               (T_MSGBUF *)sms_sdu,
               (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);

  data[0] = SMS_RECORD_REC_UNREAD;
  memcpy (&data[1], sms_sdu->buf, sms_sdu->l_buf >> 3);
  memset (&data[(sms_sdu->l_buf >> 3) + 1], NOT_PRESENT_8BIT,
          (SIZE_EF_SMS-1) - (sms_sdu->l_buf >> 3));

  MFREE (sim_pdu);
  MFREE (sms_sdu);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_get_free_space          |
+--------------------------------------------------------------------+

  PURPOSE : Checks the first not used record for SMS messages.
            Returns the index of the first free record in the 
            range 1..max_record, if no free record exists 0 is returned.

*/

GLOBAL UBYTE tl_get_free_space (UBYTE mem_type)
{
  UBYTE i;
  GET_INSTANCE_DATA;

  T_BACKUP   * backup;

  TRACE_FUNCTION ("tl_get_free_space()");
  
  if (mem_type EQ MEM_ME)
    backup = &SMS_ME_PROP(sms_data);
  else if (mem_type EQ MEM_SM)
    backup = &SMS_SIM_PROP(sms_data);
  else 
    return 0;

  for (i = 1; i <= backup->max_record; i++)
  {
    if ((tl_get_status (backup, i-1) & 1) EQ 0)
      return i;
  }
  return 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_find_first              |
+--------------------------------------------------------------------+

  PURPOSE : Checks availability of SMS memory on SIM or ME memory.

*/

GLOBAL void tl_find_first (UBYTE mem_type)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_find_first()");

  if (mem_type EQ MEM_SM)
  {
    /*
     * Start with index 1 on SIM card
     */
    tl_set_access_fifo (ACCESS_BY_MMI);
    SET_STATE (STATE_MMI, MMI_FIND_FIRST);
    tl_sim_read_record_req (1);
    sms_data->sim_backup.any_valid = FALSE;
  }
  else
  {
    USHORT max_record;
    UBYTE  version;
    UBYTE  *sim_msg;
    MALLOC (sim_msg, SIZE_EF_SMS);

    sms_data->me_backup.any_valid = FALSE;
    /*
     * look at Mobile Memory
     */
/* Implements Measure#32: Row 84 */
    if (pcm_ReadRecord ((UBYTE *)ef_sms_id, 1, SIZE_EF_SMS,
                        sim_msg, &version, &max_record) EQ PCM_OK)
    {
      if (max_record > MAX_RECORD_ME)
        max_record = MAX_RECORD_ME;
      else if (max_record EQ 0)
        max_record = 1;

      sms_data->me_backup.max_record = (UBYTE)max_record;
      if (tl_find_status_pid (MEM_ME, 1, sim_msg,
                              &sms_data->me_backup))
      {
        tl_message_ind_from_sim (MEM_ME, 1, (UBYTE)max_record, sim_msg);
      }
    }
    else
    {
      /*
       * No SMS in mobile memory
       */
      sms_data->me_backup.max_record = 0;
    }
    MFREE (sim_msg);
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_find_next               |
+--------------------------------------------------------------------+

  PURPOSE : Creates a copy of the status and protocol identifier
            for the next entry.

*/

GLOBAL void tl_find_next (UBYTE mem_type, UBYTE index)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_find_next()");

  if (mem_type EQ MEM_SM)
  {
    /*
     * Start with index on SIM card
     */
    tl_set_access_fifo (ACCESS_BY_MMI);
    SET_STATE (STATE_MMI, MMI_FIND_NEXT);
    tl_sim_read_record_req (index);
  }
  else
  {
    USHORT max_record;
    UBYTE  version;
    UBYTE  *sim_msg;
    MALLOC (sim_msg, SIZE_EF_SMS);

    /*
     * look at Mobile Memory
     */
/* Implements Measure#32: Row 86 */
    if (pcm_ReadRecord ((UBYTE *)ef_sms_id, index, SIZE_EF_SMS,
                        sim_msg, &version, &max_record) EQ PCM_OK)
    {
      if (tl_find_status_pid (MEM_ME, index, sim_msg,
                              &sms_data->me_backup))
        tl_message_ind_from_sim (MEM_ME, index, (UBYTE)max_record, sim_msg);
    }
    else
    {
      /*
       * declare only the successfull records
       * from previous attempts as available
       */
      sms_data->me_backup.max_record = index - 1;
    }
    MFREE (sim_msg);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_get_pid_dcs             |
+--------------------------------------------------------------------+

  PURPOSE : Gets TP-PID and/or TP-DCS from any message type.
            If something is not available, nothing is written to
            *pid and *dcs.
*/

GLOBAL void tl_get_pid_dcs (UBYTE     status,
                            T_sms_sdu *sms_sdu,
                            UBYTE     *pid,
                            UBYTE     *dcs)
{
  UBYTE *ptr;
  int step, rpl;

  TRACE_FUNCTION ("tl_get_pid_dcs()");

  ptr = &sms_sdu->buf[rpl = sms_sdu->buf[0] + 1]; /* points to TP-MTI */
  switch (status)
  {
  case SMS_RECORD_STO_UNSENT:
  case SMS_RECORD_STO_SENT:     /* MO-SM */
    switch (*ptr & 3)
    {
    case SMS_SUBMIT:
      ptr += (*(ptr + 2) + 1) / 2 + 4;
      if (pid NEQ NULL)
        *pid = *ptr;
      if (dcs NEQ NULL)
        *dcs = *(ptr + 2);
      break;

    case SMS_COMMAND:
      if (pid NEQ NULL)
        *pid = *(ptr + 2);
      break;

    default:
      break;
    }
    break;

  case SMS_RECORD_REC_UNREAD:
  case SMS_RECORD_REC_READ:     /* MT-SM */
    switch (*ptr & 3)
    {
    case SMS_DELIVER:
      ptr += (*(ptr + 1) + 1) / 2 + 3;
      if (pid NEQ NULL)
        *pid = *ptr;
      if (dcs NEQ NULL)
        *dcs = *(ptr + 2);
      break;

    case SMS_STATUS_REPORT:
      if ((step = (*(ptr + 2) + 1) / 2 + 19) + rpl < (sms_sdu->l_buf >> 3))
      {
        ptr += step;  /* TP-PI */
        if (pid NEQ NULL AND *ptr & 1)
          *pid = *(ptr + 1);
        if (dcs NEQ NULL AND *ptr & 2)
          *dcs = *(ptr + 2);
      }
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_find_status_pid         |
+--------------------------------------------------------------------+

  PURPOSE : Calculates status and protocol identifier of a SMS
            record and stores the values. Returns TRUE, when a
            valid message is recognized.
*/

GLOBAL BOOL tl_find_status_pid (UBYTE      mem_type,
                                UBYTE      index,
                                UBYTE    * data,
                                T_BACKUP * backup)
{
  UBYTE *ptr;
  BOOL is_valid = FALSE;

  TRACE_FUNCTION ("tl_find_status_pid()");

  /*
   * Store status and clear pid
   */
  tl_store_status (backup, index-1, data[0]);
  tl_store_pid (backup, index-1, 0);

  /*
   * mobile terminated message (bit 3 = 0) and used (bit 1 = 1)
   * check first byte with mask 00000101 = bit 3+1 = 5 must
   * be 1, that means bit 3 = 0 and bit 1 = 1.
   * skip service centre address and set to message type
   */
  ptr = &data[1] + data[1] + 1;

  switch (data[0] & 5)
  {
  case 1:   // MT-SM
    /*
     * check message type to be a SMS-DELIVER
     */
    if ((*ptr & 3) EQ SMS_DELIVER)
    {
      /*
       * skip message type and originator address,
       * *(ptr+1) contains the number of digits of the
       * originator address. The term (*p+1)/2 indicates
       * the number of bytes used for the digits. Plus 3 for
       * message type, the length field and TON/NPI field.
       */
      ptr += (*(ptr + 1) + 1) / 2 + 3;

      /*
       * store protocol identifier
       */
      tl_store_pid (backup, index-1, *ptr);

      /*
       * valid message found
       */
      backup->any_valid = TRUE;
      is_valid = TRUE;
    }
    else if((*ptr & 3) EQ SMS_STATUS_REPORT)
    { 
      /*
       * Indicate that valid Status Report message is found in ME or SIM.
       * No need to store status & pid as only reading and delelting of 
       * Status Report Message is allowed and not replacing of the message.
       *
       */
      backup->any_valid = TRUE;
      is_valid = TRUE;
    }
    else
    {
      /*
       * consider the record as free
       */
      tl_store_status (backup, index-1, 0);
    }
    break;

  case 5:   // MO-SM
    /*
     * mobile originated messages are not to be replaced,
     * but it is checked whether there is really a SMS-SUBMIT
     * stored on the SIM
     */
    if ((*ptr & 3) NEQ SMS_SUBMIT)
    {
      /*
       * consider the record as free
       */
      tl_store_status (backup, index-1, 0);
    }
    else
    {
      /*
       * valid message found
       */
      backup->any_valid = TRUE;
      is_valid = TRUE;
    }
    break;

  default:
    break;
  }
  return is_valid;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_cphs_voice_mail         |
+--------------------------------------------------------------------+

  PURPOSE : Checking whether a CPHS Voice Mail Indication is
            given, which shall not be stored.
*/
#ifdef FF_CPHS
LOCAL BOOL tl_cphs_voice_mail (T_TP_DELIVER *sms_deliver)
{
  if (sms_deliver->tp_oa.digits NEQ 4 OR
      sms_deliver->tp_oa.ton NEQ SMS_TON_ALPHANUMERIC)
    return FALSE;

  switch (sms_deliver->tp_dcs & 0xF0)
  {
  case SMS_DCS_GRP_DEF:
  case SMS_DCS_GRP_CLASS:
    if ((sms_deliver->tp_dcs & 0xC) EQ 0 OR
        (sms_deliver->tp_dcs & 0xC) EQ 0xC)
      break;            /* GSM Default Alphabet */
    /*FALLTHROUGH*/ /*lint -fallthrough*/
  case SMS_DCS_GRP_COMPR:
  case SMS_DCS_GRP_CLASS_COMPR:
  case SMS_DCS_GRP_MW_STORE_UCS2:
    return FALSE;       /* no GSM Default Alphabet */
  case SMS_DCS_DATA_CLASS:
    if (sms_deliver->tp_dcs & 0x4)
      return FALSE;     /* no GSM Default Alphabet */
  }
  if (!sms_deliver->v_tp_ud)
    return FALSE;       /* not only text present */

  if (sms_deliver->tp_ud.length NEQ 1 OR
      (sms_deliver->tp_ud.data[0] & 0x7F) NEQ ' ')
    return FALSE;       /* no single space */

  TRACE_FUNCTION ("CPHS VMS: do not store");

  return TRUE;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_network_on_sim    |
+--------------------------------------------------------------------+

  PURPOSE : Checks the PLMN code as part of the IMSI and notices
            whether a certain network operator is present.
*/
#ifdef FF_SMS_NW_RCG_SIM
GLOBAL void tl_check_network_on_sim  (UBYTE        *data)
{
  GET_INSTANCE_DATA;
  if (data NEQ NULL AND data[0] >= 3 AND
      (data[1] & 0xF7) EQ 0x21  /* ignore parity bit */
      AND data[2] EQ 0x43 AND data[3] EQ 0x03)
    SMS_NETWORK(sms_data) = NW_SIM_23430;
  else
    SMS_NETWORK(sms_data) = NW_SIM_NONE;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_handle_23430            |
+--------------------------------------------------------------------+

  PURPOSE : Special handling of an income Short Message according to
            specifications of operator 23430
*/
#ifdef FF_SMS_23430
GLOBAL BOOL tl_handle_23430 (T_TP_DELIVER *sms_deliver)
{
  GET_INSTANCE_DATA;
  UBYTE record;

  if (SMS_NETWORK(sms_data) NEQ NW_SIM_23430)
    return FALSE;

  if ((sms_deliver->tp_dcs EQ 0 OR sms_deliver->tp_dcs EQ 0xF2)
      AND (int)sms_deliver->tp_oa.digits EQ sizeof(ref_nr_23430)
      AND sms_deliver->tp_oa.ton EQ SMS_TON_ALPHANUMERIC
      AND memcmp (sms_deliver->tp_oa.num, ref_nr_23430,
                  sizeof(ref_nr_23430)) EQ 0)
  {
    record = tl_get_free_space (MEM_SM);
    if (record)
    {
      tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_PENDING);
      tl_sim_conv_update_req (record, SMS_RP_RCVD(sms_data));

      tl_set_access_fifo (ACCESS_BY_NET);
      SET_STATE (STATE_NET, NET_23430_WRITE);

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
    }
    else
    {
      tl_sms_memo_exceeded (FALSE);
    }
    return TRUE;    /* the message has been handled */
  }
  switch (sms_deliver->tp_dcs & 0xF0)
  {
    case SMS_DCS_GRP_CLASS:
    case SMS_DCS_GRP_CLASS_COMPR:
    case SMS_DCS_DATA_CLASS:
      if ((sms_deliver->tp_dcs & 3) EQ 2 AND    /* Class 2 */
          ((sms_deliver->v_tp_ud AND sms_deliver->tp_ud.length EQ 0) OR
           (sms_deliver->v_tp_udh_inc AND sms_deliver->tp_udh_inc.length EQ 0)))
      {
        if (tl_get_free_space (MEM_SM) NEQ 0)
          rl_report_req_ack (NULL);
        else
          tl_sms_memo_exceeded (FALSE);
        return TRUE;    /* the message has been handled */
      }
      break;
    default:
      break;
  }
  return FALSE;     /* no SIM specific message */
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_store_special_sms       |
+--------------------------------------------------------------------+

  PURPOSE : Checking the presence of a special message indication
            requiring the message to be stored.
*/

LOCAL BOOL tl_store_special_sms (T_TP_DELIVER *sms_deliver)
{
  UBYTE *udh;
  int udh_len;

  if (!sms_deliver->tp_udhi OR
      !sms_deliver->v_tp_udh_inc)
    return FALSE;

  udh = &sms_deliver->tp_udh_inc.tp_udh.data[0];
  udh_len = (int)sms_deliver->tp_udh_inc.tp_udh.c_data;

  while (udh_len > 0)
  {
    if (udh[0] EQ 0x01)    /* tag Special Message Indication? */
    {
      if (udh[2] & 0x80)   /* storing required? */
      {
        TRACE_FUNCTION ("tl_store_special_sms: found");
        return TRUE;
      }
    }
    udh_len -= (int)udh[1] + 2;
    udh += udh[1] + 2;     /* next tag */
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_handle_message          |
+--------------------------------------------------------------------+

  PURPOSE : Standard handling of an income Short Message according to
            GSM 03.40 and 03.38
*/

/*
 * DISPLAY   display
 * MEM_ME    store in ME
 * MEM_SM    store in SIM
 * NOTHING   do nothing
 */

static const UBYTE me_table_A [5][7] =
{   /* mt     no class class 0  class 1  class 2  class 3  discard  store */
    /* 0 */ { MEM_ME,  MEM_ME,  MEM_ME,  MEM_SM,  MEM_ME,  MEM_ME,  MEM_ME  },
    /* 1 */ { MEM_ME,  DISPLAY, MEM_ME,  MEM_SM,  MEM_ME,  MEM_ME,  MEM_ME  },
    /* 2 */ { DISPLAY, DISPLAY, DISPLAY, MEM_SM,  DISPLAY, DISPLAY, MEM_ME  },
    /* 3 */ { MEM_ME,  DISPLAY, MEM_ME,  MEM_SM,  DISPLAY, MEM_ME,  MEM_ME  },
    /* 4 */ { MEM_ME,  DISPLAY, MEM_ME,  MEM_SM,  MEM_SM,  DISPLAY, MEM_ME  }
};

static const UBYTE me_table_B [5][7] =
{   /* mt     no class class 0  class 1  class 2  class 3  discard  store */
    /* 0 */ { MEM_SM, DISPLAY, MEM_SM,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 1 */ { MEM_SM, IGNORE,  MEM_SM,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 2 */ { NOTHING, IGNORE,  NOTHING, NOTHING, NOTHING, NOTHING, NOTHING },
    /* 3 */ { MEM_SM, IGNORE,  MEM_SM,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 4 */ { MEM_SM,  IGNORE,  MEM_SM,  NOTHING, MEM_ME,  NOTHING, MEM_SM  }
};

static const UBYTE sm_table_A [5][7] =
{   /* mt     no class class 0  class 1  class 2  class 3  discard  store */
    /* 0 */ { MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM  },
    /* 1 */ { MEM_SM,  DISPLAY, MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM,  MEM_SM  },
    /* 2 */ { DISPLAY, DISPLAY, DISPLAY, MEM_SM,  DISPLAY, DISPLAY, MEM_SM  },
    /* 3 */ { MEM_SM,  DISPLAY, MEM_SM,  MEM_SM,  DISPLAY, MEM_SM,  MEM_SM  },
    /* 4 */ { MEM_SM,  DISPLAY, MEM_ME,  MEM_SM,  MEM_SM,  DISPLAY, MEM_SM  }
};

static const UBYTE sm_table_B [5][7] =
{   /* mt     no class class 0  class 1  class 2  class 3  discard  store */
    /* 0 */ { MEM_ME, DISPLAY, MEM_ME,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 1 */ { MEM_ME, IGNORE,  MEM_ME,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 2 */ { NOTHING, IGNORE,  NOTHING, NOTHING, NOTHING, NOTHING, NOTHING },
    /* 3 */ { MEM_ME, IGNORE,  MEM_ME,  NOTHING, NOTHING, DISPLAY, NOTHING },
    /* 4 */ { MEM_ME,  IGNORE,  MEM_SM,  NOTHING, MEM_ME,  NOTHING, MEM_ME  }
};

GLOBAL void tl_handle_message (T_TP_DELIVER *sms_deliver)
{
  GET_INSTANCE_DATA;
  UBYTE dcs_class;
  UBYTE record;
  UBYTE data[SIZE_EF_SMS];

  TRACE_FUNCTION ("tl_handle_message()");

  if (sms_data->pr_cntrl.delivery_state == SMS_DELIVER_STATUS_PAUSE)
  {
     /* user has blocked message receiption */
     TRACE_EVENT("User has paused MT message delivery");

     tl_sms_memo_pause();

     SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
     return;
  }

#ifdef FF_CPHS
  if (SMS_CPHS(sms_data) AND tl_cphs_voice_mail (sms_deliver))
    dcs_class = 5;              /* discard message */
  else
#endif
  if (tl_store_special_sms (sms_deliver))
    dcs_class = 6;              /* store message */
  else if (sms_deliver->tp_pid EQ SMS_PID_ME_DOWNLOAD)
    dcs_class = 1 + 1;          /* class 1 */
  else if (sms_deliver->tp_pid EQ SMS_PID_SIM_DOWNLOAD)
    dcs_class = 2 + 1;          /* class 2 */
  else switch (sms_deliver->tp_dcs & 0xF0)
  {
    case SMS_DCS_GRP_CLASS:
    case SMS_DCS_GRP_CLASS_COMPR:
    case SMS_DCS_DATA_CLASS:
      dcs_class = (sms_deliver->tp_dcs & 3) + 1;
      break;
    case SMS_DCS_GRP_MW_DISCD:
      dcs_class = 5;            /* discard message */
      break;
    case SMS_DCS_GRP_MW_STORE:
    case SMS_DCS_GRP_MW_STORE_UCS2:
      dcs_class = 6;            /* store message */
      break;
    default:
      dcs_class = 0;            // no class
      break;
  }

  if (sms_data->mem3 EQ MEM_ME)
  {
    sms_data->use_mem_a = me_table_A [sms_data->mt][dcs_class];
    sms_data->use_mem_b = me_table_B [sms_data->mt][dcs_class];
  }
  else
  {
    sms_data->use_mem_a = sm_table_A [sms_data->mt][dcs_class];
    sms_data->use_mem_b = sm_table_B [sms_data->mt][dcs_class];
  }

  /*
   * try alternative A
   */
  switch (sms_data->use_mem_a)
  {
    case DISPLAY:
      /*
       * Only display of the message
       */
      
      TRACE_EVENT("TABLE A: DISPLAY");

      SMS_EM_DISPLAY_MT_SHORT_MESSAGE;

      tl_message_ind_from_net (NOT_PRESENT_8BIT, 0, 0,
                               SMS_RP_RCVD(sms_data));
      {
        if (SMS_MT_ACK_MODE(sms_data) NEQ SMS_MHC_PH2PLUS)
        {
          rl_report_req_ack (NULL);
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        }
      }
      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      return;

    case MEM_ME:
      /*
       * memory type is mobile memory
       */

      TRACE_EVENT("TABLE A: MEM_ME");

      record = tl_get_free_space (MEM_ME);
      if (record NEQ 0)
      {
        tl_convert_mt_to_mem (SMS_RP_RCVD(sms_data), data);
        /*
         * store in mobile memory
         */
/* Implements Measure#32: Row 87 */
        if (pcm_WriteRecord ((UBYTE *)ef_sms_id, record,
                             SIZE_EF_SMS, data) NEQ PCM_OK)
          break;
        /*
         * update status byte and protocol identifier
         */
        tl_store_status (&SMS_ME_PROP(sms_data), record-1, SMS_RECORD_REC_UNREAD);
        tl_store_pid (&SMS_ME_PROP(sms_data), record-1, sms_deliver->tp_pid);
        /*
         * send indication to MMI
         */
        tl_message_ind_from_net (MEM_ME, record,
                                 sms_data->me_backup.max_record,
                                 SMS_RP_RCVD(sms_data));
        /*
         * acknowledge to the infrastructure
         */
        rl_report_req_ack (NULL);
        SMS_INST_SET_STATE (STATE_TL, TL_IDLE);

        MFREE (SMS_RP_RCVD(sms_data));
        SMS_RP_RCVD(sms_data) = NULL;
        return;
      }
      break;

    case MEM_SM:
      /*
       * memory type is SIM card
       */

      TRACE_EVENT("TABLE A: MEM_SM");

      record = tl_get_free_space (MEM_SM);
      if (record)
      {
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_PENDING);
        tl_sim_conv_update_req (record, SMS_RP_RCVD(sms_data));
        
        tl_set_access_fifo (ACCESS_BY_NET);
        SET_STATE (STATE_NET, NET_WRITE);
        return;
      }
      break;
  }
  /*
   * try alternative B
   */
  switch (sms_data->use_mem_b)
  {
    case DISPLAY:
      /*
       * Only display of the message
       */

      TRACE_EVENT("TABLE B: DISPLAY");

      tl_message_ind_from_net (NOT_PRESENT_8BIT, 0, 0,
                               sms_data->rp_data_dl);
      {
        if (SMS_MT_ACK_MODE(sms_data) NEQ SMS_MHC_PH2PLUS)
        {
          rl_report_req_ack (NULL);
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
        }
      }
      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;

    case MEM_ME:
      /*
       * memory type is mobile memory
       */

      TRACE_EVENT("TABLE B: MEM_ME");

      record = tl_get_free_space (MEM_ME);
      if (record)
      {
        tl_convert_mt_to_mem (SMS_RP_RCVD(sms_data), data);
        /*
         * store in mobile memory
         */
/* Implements Measure#32: Row 89 */
        if (pcm_WriteRecord ((UBYTE *)ef_sms_id, record,
                             SIZE_EF_SMS, data) EQ PCM_OK)
        {
          /*
           * update status byte
           */
          tl_store_status (&SMS_ME_PROP(sms_data), record-1, SMS_RECORD_REC_UNREAD);
          tl_store_pid (&SMS_ME_PROP(sms_data), record-1, sms_deliver->tp_pid);
          /*
           * send indication to MMI
           */
          tl_message_ind_from_net (MEM_ME, record,
                                   sms_data->me_backup.max_record,
                                   SMS_RP_RCVD(sms_data));
          /*
           * acknowledge to the infrastructure
           */
          rl_report_req_ack (NULL);
          SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
          SET_STATE (STATE_NET, NET_IDLE);

          MFREE (SMS_RP_RCVD(sms_data));
          SMS_RP_RCVD(sms_data) = NULL;
          break;
        }
      }
      if (tl_get_free_space (MEM_SM))   /* SIM memory available? */
      {
       /*
        * RP_ERROR => 
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, NULL);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);

       /*
        * free instance
        */
        // FREE_SMS_INSTANCE (SMS_INST.ti);
      }
      else
      {
        /* change mem_cap_avail flag on SIM and return error */
        tl_sms_memo_exceeded (FALSE);
      }

      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      SET_STATE (STATE_NET, NET_IDLE);

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;

    case MEM_SM:
      /*
       * memory type is SIM card
       */

      TRACE_EVENT("TABLE B: MEM_SM");

      record = tl_get_free_space (MEM_SM);
      if (record)
      {
        tl_store_status (&SMS_SIM_PROP(sms_data), record-1, SIM_SMS_PENDING);
        tl_sim_conv_update_req (record, SMS_RP_RCVD(sms_data));
        
        tl_set_access_fifo (ACCESS_BY_NET);
        SET_STATE (STATE_NET, NET_WRITE);
        break;
      }
      if (tl_get_free_space (MEM_ME))   /* other memory available? */
      {
       /*
        * RP_ERROR =>
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, NULL);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);

       /*
        * free instance
        */
        // FREE_SMS_INSTANCE (SMS_INST.ti);
      }
      else
      {
        /* change mem_cap_avail flag on SIM and return error */
        tl_sms_memo_exceeded (FALSE);
      }

      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);
      SET_STATE (STATE_NET, NET_IDLE);

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;

    case IGNORE:

      TRACE_EVENT("TABLE B: IGNORE");

      rl_report_req_ack (NULL);
      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;

    default:        /* other memory available? */

      TRACE_EVENT("TABLE B: OTHER?");

      if (tl_get_free_space (MEM_ME) OR tl_get_free_space (MEM_SM))
      {
       /*
        * RP_ERROR =>
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, NULL);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);
       /*
        * free instance
        */
        // FREE_SMS_INSTANCE (SMS_INST.ti);
      }
      else
      {
        /* change mem_cap_avail flag on SIM and return error */
        tl_sms_memo_exceeded (FALSE);
      }

      SMS_INST_SET_STATE (STATE_TL, TL_IDLE);

      MFREE (SMS_RP_RCVD(sms_data));
      SMS_RP_RCVD(sms_data) = NULL;
      break;
  }
  sms_data->use_mem_b = NOTHING;  // not reached if alternative B in progress
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sms_reserve_req_id      |
+--------------------------------------------------------------------+

  PURPOSE : Finds a free entry in for SIM accesses and reserves it for use.
*/

GLOBAL BOOL tl_sms_reserve_req_id (UBYTE *acc_ix_ptr)
{
  GET_INSTANCE_DATA;
  UBYTE       sim_acc_ix;
  

  for (sim_acc_ix = 0; 
       sim_acc_ix < SMS_MAX_SIM_ACCESS AND sms_data->sms_sim_access_info[sim_acc_ix].entry_used; 
       ++sim_acc_ix);

  if (sim_acc_ix < SMS_MAX_SIM_ACCESS) 
  {
    *acc_ix_ptr = sim_acc_ix;
    sms_data->sms_sim_access_info[sim_acc_ix].entry_used = TRUE;
    return TRUE;
  }
  else
  {
    TRACE_ERROR("SMS SIM Access entries used up");    
    return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_build_sim_update_req    |
+--------------------------------------------------------------------+

  PURPOSE : Build and send the SIM_UPDATE_REQ.

*/

GLOBAL void tl_build_sim_update_req  (void)
{
  GET_INSTANCE_DATA;
 
  TRACE_FUNCTION ("tl_build_sim_update_req()");

  /* Implements Measure# 14 */
  tl_send_sim_update_req (SMS_INST.tp_mr, 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TL                     |
| STATE   : code                ROUTINE : tl_message_ind_from_net    |
+--------------------------------------------------------------------+

  PURPOSE : Builds from SIM data and sends the primitive
            MNSMS_MESSAGE_IND to ACI. A NULL pointer for parameter
            'data' generates an empty indication, which shall
            interpreted as existent but empty memory.
*/

GLOBAL void tl_message_ind_from_net (UBYTE        mem_type,
                                     UBYTE        record,
                                     UBYTE        max_record,
                                     T_rp_data_dl *rp_data_dl)
{
  T_SIM_PDU *sim_pdu;

  TRACE_FUNCTION ("tl_message_ind_from_net");

  if (rp_data_dl NEQ NULL)
  {
    PALLOC (message_ind, MNSMS_MESSAGE_IND);
    MALLOC (sim_pdu, sizeof(T_SIM_PDU));
    message_ind->sms_sdu.o_buf = 0;
    message_ind->sms_sdu.l_buf = SIM_PDU_LEN<<3;

    message_ind->status = SMS_RECORD_REC_UNREAD;
    message_ind->mem_type = mem_type;
    message_ind->rec_num = record;
    message_ind->rec_max = max_record;

    SMS_EM_STORE_MT_MESSAGE;

    sim_pdu->rp_addr = rp_data_dl->rp_addr;
    sim_pdu->tpdu = rp_data_dl->rp_user_data.tpdu;
    sim_pdu->tp_mti = SMS_DELIVER;
    sim_pdu->v_tpdu = TRUE;
    sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

    ccd_codeMsg (CCDENT_SMS, DOWNLINK,
                 (T_MSGBUF *)&message_ind->sms_sdu,
                 (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);

    PSENDX (MMI, message_ind);
    MFREE (sim_pdu);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TL                     |
| STATE   : code                ROUTINE : tl_message_ind_from_sim    |
+--------------------------------------------------------------------+

  PURPOSE : Builds from SIM data and sends the primitive
            MNSMS_MESSAGE_IND to ACI. A NULL pointer for parameter
            'data' generates an empty indication, which shall
            interpreted as existent but empty memory.
*/

GLOBAL void tl_message_ind_from_sim (UBYTE        mem_type,
                                     UBYTE        record,
                                     UBYTE        max_record,
                                     UBYTE        *data)
{
  PALLOC (message_ind, MNSMS_MESSAGE_IND);

  TRACE_FUNCTION ("tl_message_ind_from_sim");

  memset (message_ind, 0, sizeof (T_MNSMS_MESSAGE_IND));
  message_ind->mem_type = mem_type;
  message_ind->rec_max = max_record;

  if (data NEQ NULL)
  {
    message_ind->rec_num = record;
    message_ind->status = data[0];
    memcpy (message_ind->sms_sdu.buf, &data[1], SIZE_EF_SMS-1);
    message_ind->sms_sdu.l_buf = (SIZE_EF_SMS-1) << 3;
    tl_adjust_message_len (SMS_VT_SIM_PDU, (BUF_tpdu *)&message_ind->sms_sdu);
  }
  PSENDX (MMI, message_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_read_access_fifo        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_READ_ACCESS_FIFO.

*/

GLOBAL UBYTE tl_read_access_fifo (void)
{
  GET_INSTANCE_DATA;
  UBYTE id;

  TRACE_FUNCTION ("tl_read_access_fifo()");

  if (sms_data->access_fifo [1] NEQ ACCESS_EMPTY)
  {
    id = sms_data->access_fifo [1];
    sms_data->access_fifo [1] = ACCESS_EMPTY;
  }
  else
  {
    id = sms_data->access_fifo [0];
    sms_data->access_fifo [0] = ACCESS_EMPTY;
  }

  return id;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_read_me_memory          |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_READ_ME_MEMORY.

*/

GLOBAL void tl_read_me_memory (USHORT     rec_num,
                               UBYTE      status)
{
  GET_INSTANCE_DATA;
  int index;

  TRACE_FUNCTION ("tl_read_me_memory()");

  if (rec_num <= SMS_ME_PROP(sms_data).max_record)
  {
    SMS_REC_STATUS(sms_data) = tl_check_status_value (status);
    if (rec_num EQ SMS_RECORD_NOT_EXIST)
    {
      /*
       * find first record of given status
       */
      index = tl_search_record (&SMS_ME_PROP(sms_data), 1,
                                SMS_REC_STATUS(sms_data));
    }
    else
      index = (int)rec_num;

    if ((index NEQ SMS_RECORD_NOT_EXIST) AND
        (tl_get_status (&SMS_ME_PROP(sms_data), index-1) & 1))
    {
      USHORT max_record;
      UBYTE misc;
      UBYTE sim_msg[SIZE_EF_SMS];
      /*
       * valid index
       */
/* Implements Measure#32: Row 84 */
      if (pcm_ReadRecord ((UBYTE *)ef_sms_id, (USHORT)index, SIZE_EF_SMS,
                          sim_msg, &misc, &max_record) NEQ PCM_OK)
      {
        tl_mnsms_read_cnf (MEM_ME, (UBYTE)rec_num,
                           NULL, SMS_CAUSE_MEM_FAIL);
        SMS_ME_PROP(sms_data).max_record = 0;
        return;
      }
      tl_store_status (&SMS_ME_PROP(sms_data), index-1, sim_msg[0]);
      if (sim_msg[0] & 1)
      {

        SMS_EM_READ_SHORT_MESSAGE;

        switch (sim_msg[0] & 7)
        {
        case SMS_RECORD_REC_UNREAD:
          /*
           * entry is changeable
           */
          switch (SMS_READ_MODE(sms_data))
          {
          case READ_STATUS_CHANGE:
            sim_msg[0] = SMS_RECORD_REC_READ;
/* Implements Measure#32: Row 84 */
            pcm_WriteRecord ((UBYTE *)ef_sms_id, (USHORT)index,
                             SIZE_EF_SMS, sim_msg);
            tl_store_status (&SMS_ME_PROP(sms_data), index-1, sim_msg[0]);
            tl_mnsms_read_cnf (MEM_ME, (UBYTE)index,
                               NULL, SIM_NO_ERROR);
            break;

          default:
            misc = sim_msg[0];
            sim_msg[0] = SMS_RECORD_REC_READ;
/* Implements Measure#32: Row 84 */
            pcm_WriteRecord ((UBYTE *)ef_sms_id, (USHORT)index,
                             SIZE_EF_SMS, sim_msg);
            tl_store_status (&SMS_ME_PROP(sms_data), index-1, sim_msg[0]);
            sim_msg[0] = misc;
            /*FALLTHROUGH*/ /*lint -fallthrough*/
          case READ_PREVIEW:
            tl_mnsms_read_cnf (MEM_ME, (UBYTE)index,
                               sim_msg, SIM_NO_ERROR);
            break;
          }
          break;

        case SMS_RECORD_STO_SENT:
        case SMS_RECORD_STO_UNSENT:
          /*
           * no status change for STORE_UNSENT!
           */
          if (SMS_READ_MODE(sms_data) EQ READ_STATUS_CHANGE)
          {
            tl_mnsms_read_cnf (MEM_ME, (UBYTE)rec_num,
                               NULL, SMS_CAUSE_OPER_NOT_ALLW);
            return;
          }
          /*FALLTHROUGH*/ /*lint -fallthrough*/
        default:
          /*
           * Status Change is obsolete
           */
          tl_mnsms_read_cnf (MEM_ME, (UBYTE)index,
                             sim_msg, SIM_NO_ERROR);
          break;
        }
        return;
      }
    }
  }
  else
    index = (int)rec_num;
  /*
   * index is wrong or other error
   */
  tl_mnsms_read_cnf (MEM_ME, (UBYTE)index, NULL,
                     (USHORT)((SMS_ME_PROP(sms_data).max_record EQ 0)?
                      SMS_CAUSE_MEM_FAIL: SMS_CAUSE_INV_INDEX));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_read_sim_memory         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_READ_SIM_MEMORY.

*/

GLOBAL void tl_read_sim_memory (
                                USHORT     rec_num,
                                UBYTE      status)
{
  GET_INSTANCE_DATA;
  int index;

  TRACE_FUNCTION ("tl_read_sim_memory()");

  if (rec_num <= (USHORT)SMS_SIM_PROP(sms_data).max_record)
  {
    SMS_REC_STATUS(sms_data) = tl_check_status_value (status);
    if (rec_num EQ SMS_RECORD_NOT_EXIST)
    {
      /*
       * find first record of given status
       */
      index = tl_search_record (&SMS_SIM_PROP(sms_data), 1,
                                SMS_REC_STATUS(sms_data));
    }
    else
      index = (int)rec_num;

    if ((index NEQ SMS_RECORD_NOT_EXIST) AND
        (tl_get_status (&SMS_SIM_PROP(sms_data), index-1) & 1))
    {
      /*
       * valid index
       */
      if (SMS_READ_MODE(sms_data) EQ READ_STATUS_CHANGE)
      {
        switch (tl_get_status (&SMS_SIM_PROP(sms_data), index-1) & 7)
        {
        case SMS_RECORD_REC_UNREAD:
          /*
           * entry is changeable
           */
          break;

        case SMS_RECORD_STO_SENT:
        case SMS_RECORD_STO_UNSENT:
          /*
           * no status change for STORE_UNSENT!
           */
          tl_mnsms_read_cnf (MEM_SM, (UBYTE)rec_num,
                             NULL, SMS_CAUSE_OPER_NOT_ALLW);
          return;

        default:
          /*
           * Status Change is obsolete
           */
          tl_mnsms_read_cnf (MEM_SM, (UBYTE)rec_num,
                             NULL, SIM_NO_ERROR);
          return;
        }
      }
      SMS_SEL_REC(sms_data) = (UBYTE)index;
      tl_set_access_fifo (ACCESS_BY_MMI);
      SET_STATE (STATE_MMI, MMI_READ);
      SMS_INST_SET_STATE (STATE_TL, TL_OTHER);
      tl_sim_read_record_req ((UBYTE)index);
      return;
    }
  }
  /*
   * index is wrong or other error
   */
  tl_mnsms_read_cnf (MEM_SM, (UBYTE)rec_num, NULL,
                     (USHORT)((SMS_SIM_PROP(sms_data).max_record EQ 0)?
                      SMS_CAUSE_MEM_FAIL: SMS_CAUSE_INV_INDEX));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_read_req            |
+--------------------------------------------------------------------+

  PURPOSE : Prepares and sends the primitive SIM_READ_REQ to read a
            binary file from SIM.
*/

GLOBAL void tl_sim_read_req (USHORT datafield,
                             UBYTE  length)
{
  GET_INSTANCE_DATA;

  UBYTE                   sim_acc_ix;

  if (tl_sms_reserve_req_id(&sim_acc_ix))
  {

    PALLOC (sim_read_req, SIM_READ_REQ);
    sim_read_req->source        = SRC_SMS;
    sim_read_req->req_id        = sim_acc_ix;
    sim_read_req->datafield     = 
    sms_data->sms_sim_access_info[sim_acc_ix].datafield   = datafield;
    sim_read_req->v_path_info   = FALSE;    
    sim_read_req->offset        = 0;
    sim_read_req->length        = length;
    sim_read_req->max_length    = 0;
    PSENDX (SIM, sim_read_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_read_record_req     |
+--------------------------------------------------------------------+

  PURPOSE : Prepares and sends the primitive SIM_READ_RECORD_REQ to
            read the given record from EF(SMS).
*/

GLOBAL void tl_sim_read_record_req (UBYTE record)
{
  GET_INSTANCE_DATA;

  UBYTE                   sim_acc_ix;

  if (tl_sms_reserve_req_id(&sim_acc_ix))  
  {

    PALLOC (sim_read_record_req, SIM_READ_RECORD_REQ);

    sim_read_record_req->source      = SRC_SMS;
    sim_read_record_req->req_id      = sim_acc_ix;
    sim_read_record_req->v_path_info = FALSE;
    sim_read_record_req->datafield   = 
    sms_data->sms_sim_access_info[sim_acc_ix].datafield  = SIM_SMS;    
    sim_read_record_req->record      = 
    sms_data->sms_sim_access_info[sim_acc_ix].rec_num    = record;
    sim_read_record_req->length      = SIM_LENGTH_SMS_RECORD;

    PSENDX (SIM, sim_read_record_req);
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_update_req          |
+--------------------------------------------------------------------+

  PURPOSE : Prepares and sends the primitive SIM_READ_UPDATE_REQ to
            update the given record of EF(SMS). Parameter 'status' is
            written to the first byte of the record, the remaining
            bytes are filled with the parameter '*data'. When parameter
            'data' is NULL, then the remaining bytes are filled with
            pattern NOT_PRESENT_8BIT (0xFF).
*/

GLOBAL void tl_sim_update_req (UBYTE record,
                               UBYTE status,
                               UBYTE *data)
{
  GET_INSTANCE_DATA;

  UBYTE                   sim_acc_ix;
    
  if (tl_sms_reserve_req_id(&sim_acc_ix))
  {
    PALLOC (update_record_req, SIM_UPDATE_RECORD_REQ);
    update_record_req->source       = SRC_SMS;
    update_record_req->req_id       = sim_acc_ix;
    update_record_req->v_path_info  = FALSE;
    update_record_req->datafield    = 
    sms_data->sms_sim_access_info[sim_acc_ix].datafield   = SIM_SMS;
    update_record_req->record       = 
    sms_data->sms_sim_access_info[sim_acc_ix].rec_num     = record;

    update_record_req->length       = SIM_LENGTH_SMS_RECORD;
    if (data NEQ NULL)
    {
      update_record_req->linear_data[0] = status;
      memcpy (&update_record_req->linear_data[1],
              data, SIM_LENGTH_SMS_RECORD-1);
    }
    else
    {
      update_record_req->linear_data[0] = SMS_RECORD_FREE;
      memset (&update_record_req->linear_data[1],
              NOT_PRESENT_8BIT, SIM_LENGTH_SMS_RECORD-1);
    }
    PSENDX (SIM, update_record_req);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_conv_update_req     |
+--------------------------------------------------------------------+

  PURPOSE : Prepares and sends the primitive SIM_READ_UPDATE_REQ to
            update the given record of EF(SMS). The data is extracted
            from the RP-DATA IE.
*/

GLOBAL void tl_sim_conv_update_req (UBYTE        record,
                                    T_rp_data_dl *rp_data_dl)
{
  GET_INSTANCE_DATA;

  UBYTE                   sim_acc_ix;

  if (tl_sms_reserve_req_id(&sim_acc_ix))
  {
    PALLOC (update_record_req, SIM_UPDATE_RECORD_REQ);
    update_record_req->source       = SRC_SMS;
    update_record_req->req_id       = sim_acc_ix;
    update_record_req->v_path_info  = FALSE;
    update_record_req->datafield    = 
    sms_data->sms_sim_access_info[sim_acc_ix].datafield = SIM_SMS;
    update_record_req->record       = 
    sms_data->sms_sim_access_info[sim_acc_ix].rec_num = record;
    update_record_req->length       = SIM_LENGTH_SMS_RECORD;
    tl_convert_mt_to_mem (rp_data_dl,
                          update_record_req->linear_data);

    PSENDX (SIM, update_record_req);
  }

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_replace_entry     |
+--------------------------------------------------------------------+

  PURPOSE : Compares the service centre address and the originator
            address for two sms messages during replace procedure.

*/

GLOBAL BOOL tl_check_replace_entry (UBYTE /*T_TP_DELIVER*/ *sms_addr,
                                    UBYTE /*T_TP_DELIVER*/ *sms_storage)
{
  TRACE_FUNCTION ("tl_check_replace_entry()");

  /*
   * skip service centre address
   */
  sms_storage += *sms_storage + 1;
  /*
   * check TP-MTI
   */
  if ((sms_storage[0] & 0x3) NEQ SMS_DELIVER)
  {
    return FALSE;
  }
  /*
   * compare originator address and protocol identifier
   */
  if (memcmp (sms_addr, &sms_storage[1], (*sms_addr + 1) / 2 + 3) EQ 0)
  {
    return TRUE;
  }
  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_replace_message         |
+--------------------------------------------------------------------+

  PURPOSE : Checks the ME and SIM memory for replacing a short message.
            If protocol identifier, service centre address and
            originator address are equal the message will be replaced.
            Else normal handling is processed.

*/

GLOBAL void tl_replace_message (T_TP_DELIVER *sms_deliver)
{
  GET_INSTANCE_DATA;
  int i;

  TRACE_FUNCTION ("tl_replace_message()");

  /*
   * first check mobile memory
   */
  i = tl_check_mt_pid (&sms_data->me_backup, 1, sms_deliver->tp_pid);
  {
    if (i > 0)  /* possible record found */
    {
      USHORT max_record;
      UBYTE version;
      UBYTE data[SIZE_EF_SMS];
      /*
       * look at Mobile Memory
       */
/* Implements Measure#32: Row 84 */
      if (pcm_ReadRecord ((UBYTE *)ef_sms_id, (USHORT)i, SIZE_EF_SMS,
                          data, &version, &max_record) EQ PCM_OK)
      {
        if (tl_check_replace_entry (&SMS_RP_RCVD(sms_data)->rp_user_data.tpdu.b_tpdu[1],
                                    &data[1]))
        {
          /*
           *record shall be replaced, store new one
           */
          tl_convert_mt_to_mem (sms_data->rp_data_dl, data);

/* Implements Measure#32: Row 84 */
          pcm_WriteRecord ((UBYTE *)ef_sms_id, (USHORT)i,
                           SIZE_EF_SMS, data);
          /*
           * send indication to MMI
           */
          tl_message_ind_from_net (MEM_ME, (UBYTE)i,
                                   SMS_ME_PROP(sms_data).max_record,
                                   SMS_RP_RCVD(sms_data));
          /*
           * acknowledge to the infrastructure
           */
          rl_report_req_ack (NULL);

          SMS_EM_REPLACE_SMS_IN_ME;

          MFREE (SMS_RP_RCVD(sms_data));
          SMS_RP_RCVD(sms_data) = NULL;
          return;
        }
      }
    }
  }

  /*
   * now look at the SIM card
   */

  i = tl_check_mt_pid (&sms_data->sim_backup, 1, sms_deliver->tp_pid);
  if (i > 0)
  {
    tl_set_access_fifo (ACCESS_BY_NET);
    SET_STATE (STATE_NET, NET_READ);
    tl_sim_read_record_req ((UBYTE)i);
  }
  else
  {
    tl_handle_message (sms_deliver);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_replace_pid       |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether a given PID for a replacement
            message could match the entry described by backup and index.
*/

LOCAL BOOL tl_check_replace_pid (T_BACKUP     *backup,
                                 unsigned     index,
                                 UBYTE        pid)

{
  if (backup->pid_field EQ NULL)
    return TRUE; /* Maybe of replacement type, further checks needed */
  return ((backup->pid_field[index >> 3] & (1 << (index & 0x7))) NEQ 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_store_pid               |
+--------------------------------------------------------------------+

  PURPOSE : This function stores the information wheter the given
            MT PID is of the replacement type in compressed form.

*/

GLOBAL void tl_store_pid (T_BACKUP *backup,
                           unsigned index,
                           UBYTE    pid)
{
  UBYTE mask;

#ifdef WIN32
  if (backup->mem_type EQ MEM_ME)
    assert (index < MAX_RECORD_ME);
  else if (backup->mem_type EQ MEM_SM)
    assert (index < MAX_RECORD_SIM);
#endif /* #ifdef WIN32 */

  if (backup->pid_field EQ NULL)
    return; /* No caching here */

  mask = 1 << (index & 0x7);

  switch (pid)
  {
    case SMS_PID_REP_SM_TYPE_1:
    case SMS_PID_REP_SM_TYPE_2:
    case SMS_PID_REP_SM_TYPE_3:
    case SMS_PID_REP_SM_TYPE_4:
    case SMS_PID_REP_SM_TYPE_5:
    case SMS_PID_REP_SM_TYPE_6:
    case SMS_PID_REP_SM_TYPE_7:
    case SMS_PID_RET_CALL_MSG:
      backup->pid_field[index >> 3] |= mask;
      break;
    default:
      backup->pid_field[index >> 3] &= ~mask;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_store_status            |
+--------------------------------------------------------------------+

  PURPOSE : This function stores the status into the backup data
            in compressed form.

*/

GLOBAL void tl_store_status (T_BACKUP *backup,
                             unsigned index,
                             UBYTE    status)
{
#ifdef WIN32
  if (backup->mem_type EQ MEM_ME)
    assert (index < MAX_RECORD_ME);
  else if (backup->mem_type EQ MEM_SM)
    assert (index < MAX_RECORD_SIM);
#endif /* #ifdef WIN32 */

  if (backup->status_field EQ NULL)
    return; /* No caching here */

  backup->status_field[index] = status;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_get_status              |
+--------------------------------------------------------------------+

  PURPOSE : This function gets the status. For the asynchronous 
            interface to the SIM the presence of backup data is 
            mandatory, for the synchronous (and fast) interface
            to the FFS the presence of backup data is optional.

*/

GLOBAL UBYTE tl_get_status (T_BACKUP *backup,
                            unsigned index)
{
  USHORT max_record;
  UBYTE  version;

#ifdef WIN32
  if (backup->mem_type EQ MEM_ME)
    assert (index < MAX_RECORD_ME);
  else if (backup->mem_type EQ MEM_SM)
    assert (index < MAX_RECORD_SIM);
#endif /* #ifdef WIN32 */

  switch (backup->mem_type)
  {
    case MEM_ME:
      if (backup->status_field EQ NULL)
      {
        UBYTE status = SMS_RECORD_FREE;
        UBYTE  *sim_msg;
        MALLOC (sim_msg, SIZE_EF_SMS);
/* Implements Measure#32: Row 84 */
        if (pcm_ReadRecord ((UBYTE *)ef_sms_id, (USHORT)(index + 1), SIZE_EF_SMS,
                            sim_msg, &version, &max_record) EQ PCM_OK)
          status = sim_msg[0];
        MFREE (sim_msg);
        return status;
      }
      /*FALLTHROUGH*/ /*lint -fallthrough*/
    case MEM_SM:
      return backup->status_field[index];

    default:
      return SMS_RECORD_FREE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_mt_pid            |
+--------------------------------------------------------------------+

  PURPOSE : Checks the memory for replacing candidates.
            If protocol identifier and status (mt message) are okay
            the index of the record is returned.

*/

GLOBAL int tl_check_mt_pid (T_BACKUP *backup,
                            unsigned index,
                            UBYTE    pid)

{
  int i;

  for (i = (int)index; i <= (int)backup->max_record; i++)
  {
    if (((tl_get_status (backup, i - 1) & 5) EQ 1) AND  /* mt message */
          tl_check_replace_pid (backup, i - 1, pid))    /* replacement type pid */
      return i;
  }
  return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_status_value      |
+--------------------------------------------------------------------+

  PURPOSE : Checks and maps the parameter <status> according to the
            following rule:
            - SMS_RECORD_REC_UNREAD     as is
            - SMS_RECORD_REC_READ       as is
            - SMS_RECORD_STO_UNSENT     as is
            - SMS_RECORD_STO_SENT       as is
            - SMS_RECORD_STAT_UNRCVD    to SMS_RECORD_STO_SENT
            - SMS_RECORD_STAT_UNSTRD    to SMS_RECORD_STO_SENT
            - SMS_RECORD_STAT_STRD      to SMS_RECORD_STO_SENT
            - any other value           to NOT_PRESENT_8BIT
            In conjunction with 'tl_search_record' a memory location
            with comparable properties can be found
*/

GLOBAL UBYTE tl_check_status_value (UBYTE status)
{
  switch (status)
  {
  case SMS_RECORD_REC_UNREAD:
  case SMS_RECORD_REC_READ:
  case SMS_RECORD_STO_UNSENT:
  case SMS_RECORD_STO_SENT:
    return status;
  case SMS_RECORD_STAT_UNRCVD:
  case SMS_RECORD_STAT_UNSTRD:
  case SMS_RECORD_STAT_STRD:
    return SMS_RECORD_STO_SENT;
  default:
    break;
  }
  return NOT_PRESENT_8BIT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_search_record           |
+--------------------------------------------------------------------+

  PURPOSE : Searches the memory for the next message with the given
            status.
            The following codes for parameter <status> are supported:
            - SMS_RECORD_FREE: next free record
            - SMS_RECORD_REC_UNREAD
            - SMS_RECORD_REC_READ
            - SMS_RECORD_STO_UNSENT
            - SMS_RECORD_STO_SENT: next record with any sent message
            - NOT_PRESENT_8BIT: any occupied record
            The index of the record is returned.
*/

GLOBAL int tl_search_record (T_BACKUP *backup,
                             USHORT   index,
                             UBYTE    status)
{
  int i;

  TRACE_FUNCTION ("tl_search_record()"); 

  for (i = (int)index; i <= (int)backup->max_record; i++)
  {
    if (status EQ NOT_PRESENT_8BIT)
    {                       /* next occupied record */
      if ((tl_get_status (backup, i - 1) & 1) NEQ SMS_RECORD_FREE)
        return i;
    }
    else if ((status & 1) EQ SMS_RECORD_FREE)
    {                       /* next free record */
      if ((tl_get_status (backup, i - 1) & 1) EQ SMS_RECORD_FREE)
        return i;
    }
    else if ((status & 7) EQ (tl_get_status (backup, i - 1) & 7))
      return i;
  }
  return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_search_record_for_delete|
+--------------------------------------------------------------------+

  PURPOSE : Searches the SMS DataBase for the next message to be deleted with the given
            status.
            The following codes for parameter <status> are supported:
            - CMGD_DEL_INDEX           delete message only for the index given
            - CMGD_DEL_READ            
            - CMGD_DEL_READ_SENT       
            - CMGD_DEL_READ_SENT_UNSENT
            - CMGD_DEL_ALL             next occupied record 
            The index of the record is returned.
*/

GLOBAL UBYTE tl_search_record_for_delete (T_BACKUP *backup,
                                          UBYTE   index,
                                          UBYTE   status)
{
  UBYTE i;
  UBYTE  file_status;

  TRACE_FUNCTION ("tl_search_record_for_delete()");

  for (i = index; i <= backup->max_record; i++)
  {
    file_status = backup->status_field[i - 1] & 7;
    switch(status)
    {
      case CMGD_DEL_ALL:
      {
        if ((file_status & 1) NEQ SMS_RECORD_FREE)
        {
         return i;
        }
        break;
      }
      case CMGD_DEL_READ:
      {
        if (file_status EQ SMS_RECORD_REC_READ)
        {
          return i;
        }
        break;
      }
      case CMGD_DEL_READ_SENT:
      {
        if (file_status EQ SMS_RECORD_REC_READ || 
            file_status EQ SMS_RECORD_STO_SENT)
        {
          return i;
        }
        break;
      }
      case CMGD_DEL_READ_SENT_UNSENT:
      {
        if (file_status EQ SMS_RECORD_REC_READ || 
            file_status EQ SMS_RECORD_STO_SENT || 
            file_status EQ SMS_RECORD_STO_UNSENT)
        {
          return i;
        }
        break;
      }
    }    
  }
  return SMS_RECORD_NOT_EXIST;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_set_access_fifo         |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_SET_ACCESS_FIFO.

*/

GLOBAL void tl_set_access_fifo (UBYTE access)
{
  GET_INSTANCE_DATA;

  TRACE_FUNCTION ("tl_set_access_fifo()");

  sms_data->access_fifo[1] = sms_data->access_fifo[0];
  sms_data->access_fifo[0] = access;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sms_memo_exceeded       |
+--------------------------------------------------------------------+

  PURPOSE : Checks the SIM memory availability. If an update of
            EF(SMSS) is required, then TRUE is returned. If the memory
            has become full, then the appropriate RP-ERROR is generated.
*/

GLOBAL BOOL tl_sms_memo_exceeded (BOOL       avail)
{
  GET_INSTANCE_DATA;
  BOOL   sim_update = FALSE;
  UBYTE  sim_acc_ix;

  TRACE_FUNCTION ("tl_sms_memo_exceeded()");

  if ((sms_data->sim_phase >= 2) AND (sms_data->mem_cap_avail NEQ avail))
  {

    if (tl_sms_reserve_req_id(&sim_acc_ix))
    {
      PALLOC (update_req, SIM_UPDATE_REQ);
      update_req->source        = SRC_SMS;
      update_req->req_id        = sim_acc_ix;
      update_req->v_path_info   = FALSE;
      update_req->datafield     = sms_data->sms_sim_access_info[sim_acc_ix].datafield = SIM_SMSS;
      update_req->length        = 1;
      update_req->trans_data[0] = (avail)? 0xFF: 0xFE;
      update_req->offset        = 1;

      PSENDX (SIM, update_req);
    
      if(sms_data->inst == INST_MO)
        tl_set_access_fifo (ACCESS_BY_MMI);
      else
        tl_set_access_fifo (ACCESS_BY_NET);

      sim_update = TRUE;
    }
  }
  if (!(sms_data->mem_cap_avail = avail))
  {
   /*
    * RP_ERROR =>
    */
    rl_report_req_error (SMS_RP_CS_MEM_CAP_EXCEEDED, NULL);
   /*
    * RL_RELEASE_REQ ==>
    */
    rl_release_req(SMS_INST.ti);
   /*
    * no memory
    */
  }

  /* inform the ACI about SIM-full/avail via SMS error indication */
  tl_mnsms_error_ind(avail ? SMS_CAUSE_MEM_AVAIL : SMS_CAUSE_MEM_FULL);

  return sim_update;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sms_memo_pause          |
+--------------------------------------------------------------------+

  PURPOSE : used for <<user intiated memory full condition>> (by the
            MNSMS_PAUSE_REQ).
            - update sim notification flag (if not done already)
            - send RP-ERROR (MEM_CAPAPCITY_EXCEEDED) to network
            - release connection
*/

GLOBAL void tl_sms_memo_pause (void)
{
  GET_INSTANCE_DATA;
  UBYTE  sim_acc_ix;
  TRACE_FUNCTION ("tl_sms_memo_pause()");

 /* 
  * generate RP_ERROR(MEM_CAP_EXCEEDED) 
  */
  rl_report_req_error (SMS_RP_CS_MEM_CAP_EXCEEDED,
                       NULL);
 /*
  * RL_RELEASE_REQ ==>
  */
  rl_release_req(SMS_INST.ti);
 /*
  * update sim notification flag if necessary
  */
  if(sms_data->pr_cntrl.mem_full_sent EQ FALSE)
  {
    sms_data->pr_cntrl.mem_full_sent = TRUE;

    if (sms_data->sim_phase >= 2)
    {
      if (tl_sms_reserve_req_id(&sim_acc_ix))
      {
        PALLOC (update_req, SIM_UPDATE_REQ);        
        update_req->req_id        = sim_acc_ix;
        update_req->v_path_info   = FALSE;
        update_req->source        = SRC_SMS;
        update_req->datafield     = 
          sms_data->sms_sim_access_info[sim_acc_ix].datafield = SIM_SMSS;
        update_req->length        = 1;
        update_req->trans_data[0] = 0xFE; /* Notification Flag = TRUE */
        update_req->offset        = 1;
        PSENDX (SIM, update_req);
        if(sms_data->inst == INST_MO)
          tl_set_access_fifo (ACCESS_BY_MMI);
        else
          tl_set_access_fifo (ACCESS_BY_NET);
      }

    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sms_memo_resume         |
+--------------------------------------------------------------------+

  PURPOSE : used for release of <<user intiated memory full condition>>
            (by the MNSMS_RESUME_REQ).
            - update sim notification flag (set to false)
*/

GLOBAL void tl_sms_memo_resume (void)
{
  TRACE_FUNCTION ("tl_sms_memo_resume()");

  /* Implements Measure# 14 */
  tl_send_sim_update_req(0xFF, 1);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_mnsms_report_ind        |
+--------------------------------------------------------------------+

  PURPOSE : Processing the function TL_MNSMS_REPORT_IND.

*/

GLOBAL void tl_mnsms_report_ind (UBYTE ent_state)
{
  PALLOC (report_ind, MNSMS_REPORT_IND);

  report_ind->state = ent_state;

  /*
   * Disable the v_cmms_mode flag as we are not sending current value of the 
   * mode value for CMMS operation.
   */
  
  report_ind->v_cmms_mode = FALSE;

  PSENDX (MMI, report_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_pause                   |
+--------------------------------------------------------------------+

  PURPOSE :    execute sms delivery pause

  DESCRIPTION: set marker for pause so that the next incoming sms
               will be responded with RP_ERROR (Cause: Memory
               Capacity Exceeded).
*/
GLOBAL void tl_pause (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_pause()");

  if (sms_data->pr_cntrl.delivery_state NEQ SMS_DELIVER_STATUS_PAUSE)
  {
    sms_data->pr_cntrl.delivery_state = SMS_DELIVER_STATUS_PAUSE;
    sms_data->pr_cntrl.mem_full_sent  = FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_resume                  |
+--------------------------------------------------------------------+

  PURPOSE :    execute sms delivery resumption

  DESCRIPTION: reset marker for pause and initiate the sending of
               memory available message (SMMA) to network
               to indicate that we can receive sms again.
*/
GLOBAL void tl_resume (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_resume()");

  if ( sms_data->pr_cntrl.delivery_state != SMS_DELIVER_STATUS_PAUSE )
  {
     tl_mnsms_resume_cnf(SMS_NO_ERROR);
     return;
  }

  if ( sms_data->pr_cntrl.mem_full_sent == FALSE )
  {
     sms_data->pr_cntrl.delivery_state = SMS_DELIVER_STATUS_RESUME;
     tl_mnsms_resume_cnf(SMS_NO_ERROR);
     return;
  }

  if (GET_STATE (STATE_MMI) NEQ MMI_IDLE)
  {
     TRACE_ERROR("Unable to process resume request");
     tl_mnsms_resume_cnf(SMS_CAUSE_ENTITY_BUSY);
     return;
  }

  GET_MO_INSTANCE(sms_data);
 /*
  * TL  state transition TL_ESTABLISH
  * EST state transition EST_SMMA
  * MMI state transition MMI_RESUME
  * 
  */
  SET_STATE (STATE_MMI, MMI_RESUME);
  SET_STATE (STATE_EST, EST_SMMA);
  SMS_INST_SET_STATE (STATE_TL, TL_ESTABLISH);
 /*
  * 1st shot
  */
  SMS_INST.retrans  = FALSE;
 /*
  * establish connection
  */
  tl_establish_connection(FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_query_deliver_status    |
+--------------------------------------------------------------------+

  PURPOSE :    handle sms status query for deliver state  

*/
GLOBAL void tl_query_deliver_status (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("tl_query_deliver_status()");

  tl_mnsms_query_cnf(SMS_QUERY_DELIVER_STATUS,sms_data->pr_cntrl.delivery_state);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_concat_check            |
+--------------------------------------------------------------------+

  PURPOSE :    check for concatenated sms  

*/
GLOBAL BOOL  tl_concat_check  ( T_sms_sdu    *sms_sdu)
{
   BYTE                       ccd_result;
   T_SIM_PDU                  *sim_pdu;
   T_TP_SUBMIT                *submit;
   T_SMS_PDU_IE_HDR           *ie_hdr;
   T_SMS_PDU_CONCAT_8BIT_HDR  *concat_8_hdr;
   /* T_SMS_PDU_CONCAT_16BIT_HDR *concat_16_hdr; */
   T_SMS_CONCAT_HDR           *concat_hdr = NULL;

   MALLOC (sim_pdu, sizeof(*sim_pdu));
   MALLOC (submit,  sizeof(*submit));

   //CCD_START;
  /*
   * CCD: SMS SDU -> SIM PDU
   */
   ccd_result = ccd_decodeMsg (CCDENT_SMS,
                               BOTH,
                              (T_MSGBUF *)sms_sdu,
                              (UBYTE *)sim_pdu,
                               SMS_VT_SIM_PDU);
   if (ccd_result NEQ ccdOK)
   {
     /*
      * error understanding the sdu
      */
      TRACE_ERROR ("ccd error decoding sim pdu");
     /*
      * give up
      */
      // CCD_END;
      MFREE (sim_pdu);
      MFREE (submit);
      return FALSE;
   }
   if (sim_pdu->tp_mti NEQ SMS_SUBMIT)
   {
     /*
      * error understanding the sdu
      */
      TRACE_ERROR ("error not a <submit> in sim pdu");
     /*
      * give up
      */
      // CCD_END;
      MFREE (sim_pdu);
      MFREE (submit);
      return FALSE;
   }
  /*
   * CCD: SIM PDU -> SUBMIT
   */
   ccd_result = ccd_decodeMsg (CCDENT_SMS,
                               UPLINK,
                              (T_MSGBUF *)&sim_pdu->tpdu,
                              (UBYTE *)submit,
                               SMS_VT_SUBMIT);
   if (ccd_result NEQ ccdOK)
   {
     /*
      * error understanding the sdu
      */
      TRACE_ERROR ("ccd error decoding submit");
//TISH, patch for ASTec32699,OMAPS00135322
//start
      if (ccd_result NEQ ccdWarning)
      {
        /*
         * give up
         */
        // CCD_END;
        MFREE (sim_pdu);
        MFREE (submit);
        return FALSE;
      }
//end
   }
   if (submit->tp_udhi EQ SMS_UDHI_INCLUDED)
   {
      TRACE_EVENT("UDH INCLUDED");
           
      ie_hdr = (T_SMS_PDU_IE_HDR *)((UBYTE *)&submit->tp_udh_inc.tp_udh.data[0]);

      if (ie_hdr->iei EQ 0x00 ) /* SMS_IEI_CONC_8BIT */
      {
        /* 
         * Concatenation header for 8 bit ref nr
         */
         concat_8_hdr = (T_SMS_PDU_CONCAT_8BIT_HDR *)
            ((UBYTE *)&submit->tp_udh_inc.tp_udh.data[2]);
         TRACE_EVENT_P3("Concatenated: Ref %#x Seq %u Of %u",
             concat_8_hdr->ref_nr,
             concat_8_hdr->seq_nr,
             concat_8_hdr->max_nr);
        /*
         * safe contents of header in common header structure
         */
         MALLOC (concat_hdr, sizeof(*concat_hdr));
         concat_hdr->ref_nr = concat_8_hdr->ref_nr;
         concat_hdr->seq_nr = concat_8_hdr->seq_nr;
         concat_hdr->max_nr = concat_8_hdr->max_nr;
      }
      else
      {
         TRACE_ERROR ("could not find concat header");
      }
   }
   // CCD_END;
  /*
   * concat control
   */
   tl_concat_cntrl(concat_hdr);
  /*
   * clean up
   */
   if (concat_hdr)
      MFREE(concat_hdr);
   MFREE (sim_pdu);
   MFREE (submit);
   return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_concat_cntrl            |
+--------------------------------------------------------------------+

  PURPOSE :    control of concatenated sms  

*/
GLOBAL void tl_concat_cntrl  (
  T_SMS_CONCAT_HDR *concat_hdr /* may be NULL */)
{
 GET_INSTANCE_DATA;
   if (sms_data->concat_cntrl.concatenation EQ FALSE)
   {
     /*
      * Concatenation currently not active
      */
      if (concat_hdr EQ NULL)
      {
        /*
         * nothing to be done
         */
         return;
      }
      else
      {
         if ((concat_hdr->seq_nr EQ 1) AND
             (concat_hdr->seq_nr < concat_hdr->max_nr))
         {
           /*
            * begin of concatenation series
            */
            TRACE_EVENT("concatenation series begin");
            sms_data->concat_cntrl.concatenation   = TRUE;
            sms_data->concat_cntrl.release_pending = FALSE;
            sms_data->concat_cntrl.nr = *concat_hdr;
         }
         else
         {
            TRACE_ERROR("concatenation series implausible nr");
         }
      }
   }
   else
   {
     /*
      * Concatenation currently active
      */
      if (concat_hdr EQ NULL)
      {
          TRACE_ERROR("concatenation series ends unexpected");
          sms_data->concat_cntrl.release_pending = TRUE;
          sms_data->concat_cntrl.end             = TRUE;
      }
      else if ((concat_hdr->seq_nr == concat_hdr->max_nr) AND
          (concat_hdr->seq_nr EQ sms_data->concat_cntrl.nr.seq_nr+1))
      {
          TRACE_EVENT("concatenation series end");
          sms_data->concat_cntrl.release_pending = TRUE;
          sms_data->concat_cntrl.end             = TRUE;
          sms_data->concat_cntrl.nr              = *concat_hdr;
      }
      else if (concat_hdr->seq_nr EQ sms_data->concat_cntrl.nr.seq_nr+1)
      {
         /*
          * plausible
          */
          TRACE_EVENT("concatenation series continued");
          sms_data->concat_cntrl.release_pending = TRUE;
          sms_data->concat_cntrl.end             = FALSE;
          sms_data->concat_cntrl.nr              = *concat_hdr;
      }
      else
      {
          TRACE_ERROR("concatenation series implausible nr");
          sms_data->concat_cntrl.release_pending = TRUE;
          sms_data->concat_cntrl.end             = TRUE;
          sms_data->concat_cntrl.nr              = *concat_hdr;
      }
   }  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_check_class_2           |
+--------------------------------------------------------------------+

  PURPOSE : Checks whether the data coding scheme is class 2.

*/

#ifdef SIM_TOOLKIT
GLOBAL UBYTE tl_check_class_2 (UBYTE dcs)
{
  TRACE_FUNCTION ("tl_class_check()");

  /*
   * check 00x1xx10
   */
  if ((dcs & 0xD3) EQ 0x12)
    return TRUE;

  /*
   * check 1111xx10
   */
  if ((dcs & 0xF3) EQ 0xF2)
    return TRUE;

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)   MODULE  : SMS_TLF                        |
| STATE   : code            ROUTINE : tl_build_envelope_sms_download |
+--------------------------------------------------------------------+

  PURPOSE : Builds an envelope command to the SIM card for SMS
            download.

*/

GLOBAL BOOL tl_build_envelope_sms_download (T_rp_data_dl *rp_data_dl)
{
  GET_INSTANCE_DATA;
  int ber_len, tpdu_len;
  int start_addr, start_tpdu;
  T_stk_cmd *stk_cmd;
  UBYTE *env;

  MCAST (sim_pdu, SIM_PDU);
  PALLOC (sim_toolkit_req, SIM_TOOLKIT_REQ);

  TRACE_FUNCTION ("tl_build_envelope_sms_download()");

  if (rp_data_dl EQ NULL)
  {
    PFREE (sim_toolkit_req);
    return FALSE;
  }
  /*
   * make SIM PDU
   */
  sim_pdu->rp_addr = rp_data_dl->rp_addr;
  sim_pdu->tpdu = rp_data_dl->rp_user_data.tpdu;
  sim_pdu->tp_mti = SMS_DELIVER;
  sim_pdu->v_tpdu = TRUE;
  sim_pdu->tp_vt_mti = SMS_VT_SIM_PDU;

  MALLOC (stk_cmd, sizeof(T_stk_cmd));
  stk_cmd->o_cmd = 0;
  stk_cmd->l_cmd = MAX_STK_CMD<<3;

  ccd_codeMsg (CCDENT_SMS, DOWNLINK,
               (T_MSGBUF *)stk_cmd,
               (UBYTE *)sim_pdu, SMS_VT_SIM_PDU);
  /*
   * calculate variable length items of ENVELOPE
   */
  ber_len = 4;                          /* length of TLV device id */
  /*
   * find RP-ADDR (Service Centre) in SIM_PDU
   */
  start_addr = (int)stk_cmd->o_cmd >> 3;
                                        /* position of RP-ADDR length */
  ber_len += (int)stk_cmd->cmd[start_addr] + 2;
                                        /* add length of TLV address */
  /*
   * find TPDU in SIM PDU
   */
  start_tpdu = start_addr + (int)stk_cmd->cmd[start_addr] + 1;
                                        /* skip RP-Orig Address */
  tpdu_len = ((int)stk_cmd->l_cmd >> 3) - start_tpdu;
                                        /* length of RP-User-Data */

  ber_len += tpdu_len + 2;              /* add length of TLV SMS-TPDU */
  if (tpdu_len >= 128)                  /* 2 byte length coding required? */
    ber_len++;                          /* adjust */
  /*
   * prepare the primitive
   */
  sim_toolkit_req -> source = SRC_SMS;
  sim_toolkit_req -> req_id = 0;

  sim_toolkit_req -> stk_cmd.o_cmd = 0;
  /*
   * BER-TLV Envelope SMS-PP Download
   */
  env = sim_toolkit_req -> stk_cmd.cmd;
  *env++ = 0xD1;
  if (ber_len >= 128)                   /* 2 byte length coding required */
  {
    *env++ = 0x81;
    sim_toolkit_req -> stk_cmd.l_cmd = (USHORT)((ber_len + 3) << 3);
  }
  else
    sim_toolkit_req -> stk_cmd.l_cmd = (USHORT)((ber_len + 2) << 3);
  *env++ = (UBYTE)ber_len;
  /*
   * TLV Device Identities
   */
  *env++ = 0x82;                        /* Tag */
  *env++ = 2;                           /* Length */
  *env++ = 0x83;                        /* Source Network */
  *env++ = 0x81;                        /* Destination SIM */
  /*
   * TLV Address
   */
  *env++ = 0x06;                        /* address tag, optional */
  memcpy (env, &stk_cmd->cmd[start_addr], /* copy length and content of RP-Orig Address */
          stk_cmd->cmd[start_addr] + 1);
  env += stk_cmd->cmd[start_addr] + 1;  /* tag plus content */
  /*
   * env now points to the tag of the TLV SMS-TPDU
   */
  *env++ = 0x8B;                        /* SMS TPDU tag, mandatory */
  if (tpdu_len >= 128)
    *env++ = 0x81;
  *env++ = (UBYTE)tpdu_len;
  memcpy (env,  &stk_cmd->cmd[start_tpdu],  /* copy content of TPDU */
          tpdu_len);
  /*
   * Send to SIM card
   */
  PSENDX (SIM, sim_toolkit_req);
  MFREE (stk_cmd);
  /*
   * Set instance to assign answer from SIM card.
   */
  tl_set_access_fifo (ACCESS_BY_NET);
  SET_STATE (STATE_NET, NET_WRITE);

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_sim_toolkit_confirm     |
+--------------------------------------------------------------------+

  PURPOSE : Getting the terminal response to a SMS - Download.

*/

GLOBAL void tl_sim_toolkit_confirm (T_SIM_TOOLKIT_CNF *toolkit_cnf)
{
  GET_INSTANCE_DATA;
  T_rp_user_data *rp_ud;
  union {
    T_TP_DLVR_REP_ACK *ack;
    T_TP_DLVR_REP_ERR *err;
  } dlvr_rep;

  TRACE_FUNCTION ("tl_sim_toolkit_confirm()");

  CCD_START;

  switch (toolkit_cnf->cause)
  {
    case SIM_NO_ERROR:
      /*
       * The SIM responds with "90 00", the ME shall acknowledge the receipt
       * of the short message to the network using an RP-ACK message.
       *
       * or
       *
       * The SIM responds with "9F XX", the ME shall use the GET RESPONSE
       * command to the response data. The response data will be supplied by
       * the ME in the TP-User-Data Element of the RP-ACK message. It will be
       * send back to the network. The values of protocol identifier and data
       * coding scheme in RP-ACK shall be as in the original message.
       */
      {
        if (toolkit_cnf->stk_cmd.l_cmd > 0)
        {
          MALLOC (rp_ud, sizeof(T_rp_user_data));
          MALLOC (dlvr_rep.ack, sizeof(T_TP_DLVR_REP_ACK));

          dlvr_rep.ack->tp_vt_mti = SMS_VT_DLVR_REP_ACK;
          dlvr_rep.ack->tp_udhi = SMS_UDHI_NOT_INCLUDED;
          dlvr_rep.ack->tp_mti = SMS_DELIVER_REPORT;
          dlvr_rep.ack->v_tp_udh_inc = FALSE;
          memcpy (dlvr_rep.ack->tp_ud.data,
                  &toolkit_cnf->stk_cmd.cmd[toolkit_cnf->stk_cmd.o_cmd >> 3],
                  (size_t)(toolkit_cnf->stk_cmd.l_cmd >> 3));
          dlvr_rep.ack->tp_ud.c_data = (UBYTE)(toolkit_cnf->stk_cmd.l_cmd >> 3);
          dlvr_rep.ack->tp_ud.length = (tl_udl_count_septet (SMS_DCS(sms_data)))?
                                       (UBYTE)(dlvr_rep.ack->tp_ud.c_data * 8 / 7):
                                       dlvr_rep.ack->tp_ud.c_data;
          dlvr_rep.ack->tp_ext = SMS_EXT_NOT_INCLUDED;
          dlvr_rep.ack->v_tp_ud = TRUE;
          dlvr_rep.ack->tp_udl_p = SMS_UD_INCLUDED;
          dlvr_rep.ack->tp_dcs = SMS_DCS(sms_data);
          dlvr_rep.ack->v_tp_dcs = TRUE;
          dlvr_rep.ack->tp_dcs_p = SMS_DCS_INCLUDED;
          dlvr_rep.ack->tp_pid = SMS_PID(sms_data);
          dlvr_rep.ack->v_tp_pid = TRUE;
          dlvr_rep.ack->tp_pid_p = SMS_PID_INCLUDED;

          rp_ud->tpdu.o_tpdu = 0;
          rp_ud->tpdu.l_tpdu = TPDU_BIT_LEN;
          rp_ud->v_tpdu = (ccd_codeMsg (CCDENT_SMS, UPLINK,
                                        (T_MSGBUF *)&rp_ud->tpdu,
                                        (UBYTE *)dlvr_rep.ack,
                                        SMS_VT_DLVR_REP_ACK) EQ ccdOK);
          rp_ud->tp_mti = SMS_DELIVER_REPORT;
          MFREE (dlvr_rep.ack);
        }
        else
          rp_ud = NULL;

        rl_report_req_ack (rp_ud);
        SET_STATE (STATE_NET, NET_IDLE);

        SMS_EM_RECEIVE_SIM_TOOLKIT_DATA_DOWNLOAD;

        if (rp_ud NEQ NULL)
        {
          MFREE (rp_ud);
        }
      }
      break;

    case SIM_CAUSE_SAT_BUSY:
      /*
       * The SIM responds with "93 00", the ME shall either retry the command or
       * send back an RP-ERROR message to the network with the TP-FCS value
       * indicating "SIM Application Toolkit Busy).
       */
      {
        MALLOC (rp_ud, sizeof(T_rp_user_data));

        rp_ud->tpdu.l_tpdu = 40;                    /* 5 bytes */
        rp_ud->tpdu.o_tpdu = 0;

        rp_ud->tpdu.b_tpdu[0] = SMS_DELIVER_REPORT; /* MTI, no UDHI */
        rp_ud->tpdu.b_tpdu[1] = SMS_FCS_SAT_BUSY;   /* SAT busy */
        rp_ud->tpdu.b_tpdu[2] = 0x03;               /* PI: DCS and PID will follow, no UD */
        rp_ud->tpdu.b_tpdu[3] = SMS_PID(sms_data);  /* set PID */
        rp_ud->tpdu.b_tpdu[4] = SMS_DCS(sms_data);  /* set DCS */

        rp_ud->tp_mti = SMS_DELIVER_REPORT;
        rp_ud->v_tpdu = TRUE;

       /*
        * RP_ERROR
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, rp_ud);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);
        SET_STATE (STATE_NET, NET_IDLE);

        MFREE (rp_ud);
      }
      break;

    case SIM_CAUSE_DNL_ERROR:
      /*
       * The ME has indicated in TERMINAL PROFILE that it supports the status
       * word "9E XX" and if the SIM responds with "9E XX", the ME shall use
       * the GET RESPONSE command to get the response data. The response data
       * from the SIM will be supplied by the ME in the TP-User data element
       * of the RP-ERROR message. It will be sent back to the network. The
       * values of protocol identifier and data coding scheme in RP-ERROR
       * shall be as in the original message. The value of TP-FCS element
       * of the RP-ERROR shall be "SIM data download error".
       */
      {
        MALLOC (rp_ud, sizeof(T_rp_user_data));
        MALLOC (dlvr_rep.err, sizeof(T_TP_DLVR_REP_ERR));

        dlvr_rep.err->tp_vt_mti = SMS_VT_DLVR_REP_ERR;
        dlvr_rep.err->tp_udhi = SMS_UDHI_NOT_INCLUDED;
        dlvr_rep.err->tp_mti = SMS_DELIVER_REPORT;
        dlvr_rep.err->tp_fcs = SMS_FCS_SAT_DNL_ERROR;
        dlvr_rep.err->v_tp_udh_inc = FALSE;
        memcpy (dlvr_rep.err->tp_ud.data,
                &toolkit_cnf->stk_cmd.cmd[toolkit_cnf->stk_cmd.o_cmd >> 3],
                (size_t)(toolkit_cnf->stk_cmd.l_cmd >> 3));
        dlvr_rep.err->tp_ud.c_data = (UBYTE)(toolkit_cnf->stk_cmd.l_cmd >> 3);
        dlvr_rep.err->tp_ud.length = (tl_udl_count_septet (SMS_DCS(sms_data)))?
                                     (UBYTE)(dlvr_rep.err->tp_ud.c_data * 8 / 7):
                                     dlvr_rep.err->tp_ud.c_data;
        dlvr_rep.err->tp_ext = SMS_EXT_NOT_INCLUDED;
        dlvr_rep.err->v_tp_ud = TRUE;
        dlvr_rep.err->tp_udl_p = SMS_UD_INCLUDED;
        dlvr_rep.err->tp_dcs = SMS_DCS(sms_data);
        dlvr_rep.err->v_tp_dcs = TRUE;
        dlvr_rep.err->tp_dcs_p = SMS_DCS_INCLUDED;
        dlvr_rep.err->tp_pid = SMS_PID(sms_data);
        dlvr_rep.err->v_tp_pid = TRUE;
        dlvr_rep.err->tp_pid_p = SMS_PID_INCLUDED;

        rp_ud->tpdu.o_tpdu = 0;
        rp_ud->tpdu.l_tpdu = TPDU_BIT_LEN;
        rp_ud->v_tpdu = (ccd_codeMsg (CCDENT_SMS, UPLINK,
                                      (T_MSGBUF *)&rp_ud->tpdu,
                                      (UBYTE *)dlvr_rep.err,
                                      SMS_VT_DLVR_REP_ERR) EQ ccdOK);
        rp_ud->tp_mti = SMS_DELIVER_REPORT;
        MFREE (dlvr_rep.err);

       /*
        * RP_ERROR
        */
        rl_report_req_error (SMS_RP_CS_PROTOCOL_ERROR, rp_ud);
       /*
        * RL_RELEASE_REQ ==>
        */
        rl_release_req(SMS_INST.ti);
        SET_STATE (STATE_NET, NET_IDLE);
        MFREE (rp_ud);
      }
      break;

    default:
      break;
  }
  CCD_END;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8410)       MODULE  : SMS_TLF                    |
| STATE   : code                ROUTINE : tl_cmms_start              |
+--------------------------------------------------------------------+

  PURPOSE : Function used for starting the Timer TMMS and continuing 
            the CMMS mode.

*/

GLOBAL void tl_cmms_start(void)
{
  TRACE_FUNCTION ("tl_cmms_start()");

  if(sms_timer_check(TMMS))
  {
     sms_timer_stop(TMMS);
  }
  sms_timer_start(TMMS);
}
  
#endif

/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This Modul defines the common functions
|             for the component DL of the mobile station.
+-----------------------------------------------------------------------------
*/

#ifndef DL_COM_C
#define DL_COM_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/
#include "typedefs.h"
#include <string.h>
#include "vsi.h"
#if !defined(DL_2TO1)
//#include "p_8010_147_l1_include.h"
#endif  /* DL_2TO1 */
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "mon_dl.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dl.h"
#include "dl_trc.h"
#include "dl_em.h"

/*==== TEST TRACE ===================================================*/
#define TEST_ENTITY_DL

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
LOCAL void com_build_frame            ( 
                                        UBYTE                    ch_type,
                                        UBYTE                    type,
                                        UBYTE                    sapi,
                                        UBYTE                    cr,
                                        UBYTE                    ns,
                                        UBYTE                    nr,
                                        UBYTE                    p_bit,
                                        UBYTE                    length,
                                        UBYTE                    m_bit,
                                        UBYTE                  * pInfoBuffer,
                                        UBYTE                    InfoOffset);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+------------------------------------------------------------------------------
| Function    : com_free_pointer
+------------------------------------------------------------------------------
| Description : frees the pointer given by parameter
+------------------------------------------------------------------------------
*/
GLOBAL void com_free_pointer (void * pointer)
{
  GET_INSTANCE_DATA;
  TRACE_EVENT_WIN_P1 ("com_free_pointer(,p=%08x)", pointer);

#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_com_free_pointer (pointer);
    return;
  }
#endif  /* INVOKE_SIGNAL */

  TRACE_ASSERT(pointer);
  MY_PFREE (pointer);
}

GLOBAL void com_free_queue_buffer (T_QUEUE * queue, USHORT index)
{
  T_DL_DATA_REQ **pp;

  switch (index)
  {
    default:
      if (index <= INDEX_MAX_STORE_BUFFER)
        pp = &queue->store_buffer[index];
      else
        pp = NULL;
      break;
    case INDEX_SENDING_BUFFER:
      pp = &queue->sending_buffer;
      break;
    case INDEX_SWITCH_BUFFER:
      pp = &queue->switch_buffer;
      break;
  }
  if (pp AND *pp)
  {
    COM_FREE_POINTER (*pp);
    *pp = NULL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_clear_queue     |
+--------------------------------------------------------------------+

  PURPOSE : Clearing a DL queue.

*/

GLOBAL void com_clear_queue (UBYTE sapi)
{
  GET_INSTANCE_DATA;
  USHORT i;
  T_QUEUE *queue;
  TRACE_FUNCTION ("com_clear_queue()");

  if (sapi EQ PS_SAPI_0)
  {
    queue = &dl_data->dcch0_queue;
  }
  else
  {
    queue = &dl_data->dcch3_queue;
  }

  for (i=0;i<MAX_QUEUED_MESSAGES;i++)
    if (queue->store_buffer [i] NEQ NULL)
    {
      COM_FREE_QUEUE_BUFFER (queue, i);
    }

  if (queue->sending_buffer NEQ NULL)
  {
    COM_FREE_QUEUE_BUFFER (queue, INDEX_SENDING_BUFFER);
  }

  if (queue->switch_buffer NEQ NULL)
  {
    COM_FREE_QUEUE_BUFFER (queue, INDEX_SWITCH_BUFFER);
  }
  queue->act_length = queue->act_offset = 0;
  queue->no_of_stored_messages = 0;
  memset (&queue->transmit_buffer, 0, sizeof (T_FRAME));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_restore_queue   |
+--------------------------------------------------------------------+

  PURPOSE : Restoring a DL queue. If a new connection shall be
            established, the message (ASSIGNMENT or HANDOVER COMPLETE)
            is transmitted first. So the message is stored in the
            switch buffer. A previously not complete send message will
            be put back into the storing buffer.

*/

GLOBAL void com_restore_queue (UBYTE sapi, T_DL_DATA_REQ* est_req)
{
  GET_INSTANCE_DATA;
  T_QUEUE *queue = sapi EQ PS_SAPI_0 ? &dl_data->dcch0_queue:&dl_data->dcch3_queue;

  TRACE_FUNCTION ("com_restore_queue()");

  if (queue->switch_buffer NEQ NULL)
  {
    TRACE_EVENT_WIN ("free old switch_buffer");
    COM_FREE_QUEUE_BUFFER (queue, INDEX_SWITCH_BUFFER);
  }

  if (est_req AND est_req->sdu.l_buf)
  {
    PPASS (est_req, data_req, DL_DATA_REQ);
    queue->switch_buffer = data_req; /* only valid sdu */

    TRACE_EVENT_WIN_P2 ("new fill of switch_buffer:%p l=%u",
      data_req, est_req->sdu.l_buf>>3);
  }

  if (queue->sending_buffer NEQ NULL)
  {
    TRACE_EVENT_WIN ("restore sending_buffer");
    queue->act_length     = queue->sending_buffer->sdu.l_buf;
    queue->act_offset     = queue->sending_buffer->sdu.o_buf;
  }
  TRACE_EVENT_WIN ("delete transmit_buffer");
  memset (&queue->transmit_buffer, 0, sizeof (T_FRAME));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_recover_queue   |
+--------------------------------------------------------------------+

  PURPOSE : Recover a DL queue after a Reject condition.

*/

GLOBAL void com_recover_queue (UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_QUEUE *queue = sapi EQ PS_SAPI_0 ? &dl_data->dcch0_queue:&dl_data->dcch3_queue;

  TRACE_FUNCTION ("com_recover_queue()");

  if (queue->sending_buffer NEQ NULL)
  {
    if(queue->act_length NEQ 0) /* For last buf, act_length is set to 0 */
    {                           /* and the act_offset is not modified */
      queue->act_offset    -= queue->transmit_buffer.l_buf;
    }
    queue->act_length    += queue->transmit_buffer.l_buf;
  }
  memset (&queue->transmit_buffer, 0, sizeof (T_FRAME));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_read_queue      |
+--------------------------------------------------------------------+

  PURPOSE : Reading the next segment of a message from a DL queue.
            If a message in the switch buffer is stored, this message
            is used. If a message is stored in the sending buffer which
            is not send completely the next segment is copied to the
            transmit buffer. Else the next sending buffer is copied from
            the store buffer of the queue. The first segment of this
            message is copied to the transfer buffer.

*/

GLOBAL void com_read_queue (UBYTE ch_type, UBYTE sapi,
                            UBYTE * m_bit)
{
  GET_INSTANCE_DATA;
  USHORT   i;
  USHORT   length, bit_length;
  T_DL_DATA_REQ * dl_data_req;
  T_QUEUE  *queue = sapi EQ PS_SAPI_0 ? &dl_data->dcch0_queue : &dl_data->dcch3_queue;

  TRACE_FUNCTION ("com_read_queue()");
  TRACE_EVENT_WIN_P1 ("read sapi_%u_queue", sapi);

  switch (ch_type)
  {
    case L2_CHANNEL_SDCCH:
      length = N201_SDCCH;
      break;

    case L2_CHANNEL_SACCH:
      length = N201_SACCH;
      break;

    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
      length = N201_FACCH;
      break;

    default:
      length = 0;
      bit_length = 0;
      break;
  }
  bit_length = length << 3;

  if (queue->switch_buffer NEQ NULL)
  {
    T_DL_DATA_REQ *switch_buffer = queue->switch_buffer;

    TRACE_EVENT_WIN_P2 ("fill transmit_buffer with bytes %u-%u of switch_buffer, no bytes left",
      (switch_buffer->sdu.o_buf>>3),
      ((switch_buffer->sdu.o_buf+switch_buffer->sdu.l_buf)>>3)-1);

    queue->m_bit = * m_bit = 0;
    queue->transmit_buffer.o_buf = 24;
    queue->transmit_buffer.l_buf = switch_buffer->sdu.l_buf;
    for (i=0;i<(switch_buffer->sdu.l_buf>>3);i++)
      queue->transmit_buffer.buf[i+3] =
        switch_buffer->sdu.buf[i+(switch_buffer->sdu.o_buf>>3)];

    /*
    should be freed first after acknowledgement!
    COM_FREE_QUEUE_BUFFER (dl_data, queue, INDEX_SWITCH_BUFFER);
    */
    return;
  }
  else
  {
    if (queue->act_length EQ 0)
    {
      if (queue->sending_buffer NEQ NULL)
      {
        COM_FREE_QUEUE_BUFFER (queue, INDEX_SENDING_BUFFER);
      }
      queue->sending_buffer = queue->store_buffer[0];
      queue->act_offset     = queue->sending_buffer->sdu.o_buf;
      queue->act_length     = queue->sending_buffer->sdu.l_buf;
      TRACE_EVENT_WIN_P2 ("fill sending_buffer with bytes %u-%u of store_buffer[0]",
        (queue->sending_buffer->sdu.o_buf>>3),
        ((queue->sending_buffer->sdu.o_buf+queue->sending_buffer->sdu.l_buf)>>3)-1);

      for (i=0;i<MAX_QUEUED_MESSAGES-1;i++)
        queue->store_buffer[i] = queue->store_buffer[i+1];
      queue->no_of_stored_messages--;

      TRACE_EVENT_WIN_P1 ("left no_of_stored_messages=%u", queue->no_of_stored_messages);
    }
    dl_data_req = queue->sending_buffer;
  }


  if (queue->act_length > bit_length)
  { /*
     * lint Info 702: Shift right of signed quantity:
     * not possible because of the compare one line before
     */
    TRACE_EVENT_WIN_P3 ("fill transmit_buffer with bytes %u-%u of sending_buffer, %u bytes left",
      queue->act_offset>>3, (queue->act_offset>>3)+length-1,
      (queue->act_length-bit_length)>>3);

    queue->m_bit = * m_bit = 1;
    queue->transmit_buffer.o_buf = 24;
    queue->transmit_buffer.l_buf = bit_length;
    for (i=0;i<length;i++)
      queue->transmit_buffer.buf[i+3] =
        dl_data_req->sdu.buf[i+(queue->act_offset>>3)];
    queue->act_offset += bit_length;
    queue->act_length -= bit_length;
  }
  else
  {
    TRACE_EVENT_WIN_P2 ("fill transmit_buffer with bytes %u-%u of sending_buffer, no bytes left",
      queue->act_offset>>3, ((queue->act_offset+queue->act_length)>>3)-1);

    queue->m_bit = * m_bit = 0;
    queue->transmit_buffer.o_buf = 24;
    queue->transmit_buffer.l_buf = queue->act_length;
    for (i=0;i<(queue->act_length>>3);i++)
      queue->transmit_buffer.buf[i+3] =
        dl_data_req->sdu.buf[i+(queue->act_offset>>3)];
    queue->act_length = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_store_queue     |
+--------------------------------------------------------------------+

  PURPOSE : Storing a message into the queue.

*/

GLOBAL void com_store_queue (UBYTE sapi, T_DL_DATA_REQ * data_req)
{
  GET_INSTANCE_DATA;
  T_QUEUE * queue;

  TRACE_FUNCTION ("com_store_queue()");

  if (sapi EQ PS_SAPI_0)
  {
    queue = &dl_data->dcch0_queue;
  }
  else
  {
    queue = &dl_data->dcch3_queue;
  }

  if (queue->no_of_stored_messages < MAX_QUEUED_MESSAGES)
  {
    queue->store_buffer[queue->no_of_stored_messages++] = data_req;
    TRACE_EVENT_WIN_P3 ("sapi_%u_queue: add entry with %u bytes, no_of_stored_messages=%u",
      sapi, data_req->sdu.l_buf>>3, queue->no_of_stored_messages);
  }
  else
  {
    COM_FREE_POINTER (data_req);
    TRACE_EVENT_WIN_P1 ("sapi_%u_queue overflowed", sapi);
  }
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : DL_COM                              |
| STATE   : code                 ROUTINE : com_queue_awaiting_transmission     |
+------------------------------------------------------------------------------+

  PURPOSE : The function checks whether any segment has to sended.
            Function returns TRUE if a frame/segment is awaiting transmission.
            Function returns FALSE if not.

*/

GLOBAL BOOL com_queue_awaiting_transmission (UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_QUEUE * queue;
  BOOL      ret;

  TRACE_EVENT_WIN_P1 ("com_queue_awaiting_transmission(SAPI=%u)", sapi);

  queue = (sapi EQ PS_SAPI_0) ? &dl_data->dcch0_queue : &dl_data->dcch3_queue;
  if (sapi EQ PS_SAPI_0)
  {
    queue = &dl_data->dcch0_queue;
  }
  else
  {
    queue = &dl_data->dcch3_queue;
  }
  if (queue->switch_buffer NEQ NULL)
  {
    TRACE_EVENT_WIN_P1 ("sapi_%u_queue: switch_buffer is awaiting", sapi);
    ret = TRUE;
  }
  else
  {
    if (queue->act_length EQ 0)
    {
      ret = queue->no_of_stored_messages NEQ 0;
      if (ret)
      {
        TRACE_EVENT_WIN_P2 ("sapi_%u_queue: store_buffer is awaiting (no_of_stored_messages=%u)",
          sapi, queue->no_of_stored_messages);
      }
    }
    else
    {
      TRACE_EVENT_WIN_P2 ("sapi_%u_queue: transmit_buffer is awaiting (%u bytes)",
        sapi, queue->act_length);
      ret = TRUE;
    }
  }

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_leave_dedicated |
+--------------------------------------------------------------------+

  PURPOSE : Leave dedicated mode.

*/

GLOBAL void com_leave_dedicated (UBYTE ch_type)
{
  GET_INSTANCE_DATA;
  dl_data->RR_dedicated = FALSE; /* RR is leaving the dedicated mode */
  DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, ch_type, "RR_dedicated:=FALSE");
}

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM                       |
| STATE   : code                       ROUTINE : possible_reset_dcch0_ch_type |
+-----------------------------------------------------------------------------+

  PURPOSE : Reset dedicated channel.

*/
GLOBAL void possible_reset_dcch0_ch_type (void)
{
  GET_INSTANCE_DATA;
  if (
      #if defined(DELAYED_SABM)
        (dl_data->dcch0_sabm_flag NEQ NOT_PRESENT_8BIT) AND
      #endif /* DELAYED_SABM */
      #if defined(DELAYED_RELEASE_IND)
        (dl_data->release_ind_ch_type NEQ NOT_PRESENT_8BIT) AND
      #endif /* DELAYED_RELEASE_IND */
      (dl_data->state[C_DCCH0] <= STATE_IDLE_DL) AND
      (dl_data->cch[C_DCCH0].vtx EQ EMPTY_CMD))
  {
    TRACE_EVENT_WIN_P1 ("reset dcch0_ch_type=%s ->0", CH_TYPE_NAME[dl_data->dcch0_ch_type]);
    dl_data->dcch0_ch_type = 0;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_compare_L3_msg  |
+--------------------------------------------------------------------+

  PURPOSE : The function compares two layer 3 messages.


*/

GLOBAL UBYTE com_compare_L3_msg (T_DL_DATA_REQ * data_ind1, UBYTE * data_ind2)
{
  USHORT length1;
  USHORT length2;
  USHORT pos1;
  USHORT pos2;
  USHORT i;

  TRACE_FUNCTION ("com_compare_L3_msg()");

  /*
   * Calculates Length of SABM and UA layer 3 message
   */
  length1 = data_ind1->sdu.l_buf>>3;    /* length of l3 msg inside SABM frame */
  length2 = ((data_ind2[2] & 0xFC)>>2); /* length of l3 msg inside UA frame */

  if (length1 NEQ length2)
    return FALSE;

  pos1   = data_ind1->sdu.o_buf >> 3;
  pos2   = 3;


  for (i=0; i<length1; i++)
  {
    if (data_ind1->sdu.buf[i+pos1] NEQ data_ind2[i+pos2])
    {
#if defined(DL_TRACE_ENABLED)
      UBYTE sapi = data_ind1->sapi;
      UBYTE trace_channel = TRACE_CH_UNKNOWN;
      switch (data_ind1->ch_type)
      {
      case L2_CHANNEL_SDCCH:
        if (sapi EQ PS_SAPI_0)
          trace_channel = C_DCCH0;
        else if (sapi EQ PS_SAPI_3)
          trace_channel = C_DCCH3;
        break;
      case L2_CHANNEL_FACCH_F:
      case L2_CHANNEL_FACCH_H:
          trace_channel = C_DCCH0;
        break;
      default:
        break;
      }/* endswitch chan */
      DL_OFFLINE_TRACE(TRACE_DL_EVENT, trace_channel, data_ind1->ch_type, "UA doesn´t match");
#endif  /* DL_TRACE_ENABLED */
      return FALSE;
    }
  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_concatenate     |
+--------------------------------------------------------------------+

  PURPOSE : Concenate an imcoming segment.

*/

GLOBAL void com_concatenate (T_DL_DATA_IND ** in_msg,
                             UBYTE * new_data_in)
{
  GET_INSTANCE_DATA;
  USHORT   end_pos;
  USHORT   length;
  USHORT   start_pos;
  #define  L2_HEADER_BYTESIZE  3

  TRACE_FUNCTION ("com_concatenate()");

#if defined(INVOKE_SIGNAL)
    if (dl_data->interrupt_context)
    {
      sig_invoke_com_concatenate (in_msg, new_data_in);
      return;
    }
#endif  /* INVOKE_SIGNAL */

  length    = new_data_in[2] >> 2;

  if (*in_msg EQ NULL)
  {
    /*
     * Nothing stored yet
     */
    USHORT len_in_bits = (length + L2_HEADER_BYTESIZE) << 3;
    PALLOC_SDU (first_data, DL_DATA_IND, len_in_bits );

    first_data->sdu.l_buf  = length << 3;         /* = length * BITS_PER_BYTE */
    first_data->sdu.o_buf  = L2_HEADER_BYTESIZE << 3;/* = L2_HEADER_BYTESIZE * BITS_PER_BYTE */
    /*lint -e419 (Warning -- Apparent data overrun) */
    memset (&first_data->sdu.buf[0], 0, L2_HEADER_BYTESIZE);
    /*lint +e419 (Warning -- Apparent data overrun) */
    /*lint -e416 (Warning -- creation of out-of-bounds pointer) */
    memcpy (&first_data->sdu.buf[L2_HEADER_BYTESIZE],
            &new_data_in[L2_HEADER_BYTESIZE],
            length);
    /*lint +e416 (Warning -- creation of out-of-bounds pointer) */
    *in_msg = first_data;
  }
  else
  {
    T_DL_DATA_IND *previous_data = *in_msg;

    start_pos = previous_data->sdu.l_buf + previous_data->sdu.o_buf;
    end_pos   = (length << 3) + start_pos;
    {
      PALLOC_SDU (entire_data, DL_DATA_IND, end_pos );

      /*lint -e415 (Warning -- access of out-of-bounds pointer) */
      memcpy (entire_data->sdu.buf, previous_data->sdu.buf, start_pos >> 3);
      memcpy (&entire_data->sdu.buf[start_pos >> 3],
              &new_data_in[L2_HEADER_BYTESIZE],
              length);
      /*lint +e415 (Warning -- access of out-of-bounds pointer) */

      entire_data->sdu.l_buf   = previous_data->sdu.l_buf + (length << 3);
      entire_data->sdu.o_buf   = previous_data->sdu.o_buf;

      COM_FREE_POINTER (*in_msg);
      *in_msg = entire_data;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : DL_COM              |
| STATE   : code                       ROUTINE : com_check_nr        |
+--------------------------------------------------------------------+

  PURPOSE : Check the receive number.

*/

GLOBAL UBYTE com_check_nr (UBYTE va, UBYTE vs, UBYTE nr)
{
  BYTE a,b;

  TRACE_FUNCTION ("com_check_nr()");
  /*
   * under GSM 4.06 subclause 3.5.2.3:
   * nr is valid, if and only if ((nr-va) mod 8) <= ((vs-va) mod 8)
   */
  a = (nr+8-va) & 7;
  b = (vs+8-va) & 7;

  return (a <= b);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_prepare_DISC      |
+--------------------------------------------------------------------+

  PURPOSE : Prepares a DISC command.

*/

GLOBAL void com_prepare_DISC (UBYTE channel, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_CCH *pcch = &dl_data->cch[channel];

  pcch->vtx           = DISC_CMD;
  pcch->time_flag     = TRUE;
  pcch->T200_counter  = 0;
  pcch->rc            = 0;
  set_channel_state (channel, STATE_AWAITING_RELEASE);
  TRACE_EVENT_WIN_P3 ("RELEASE_REQ: %s SAPI=%u vtx=%s", CH_TYPE_NAME[pcch->ch_type],
                                      sapi, VTX_NAME[pcch->vtx]);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_UA_response |
+--------------------------------------------------------------------+

  PURPOSE : Build an UA response.

*/

GLOBAL void com_build_UA_response (UBYTE ch_type, UBYTE sapi, UBYTE f_bit)
{
  TRACE_FUNCTION ("com_build_UA_response()");
  com_build_frame (ch_type, UA_FRAME, sapi, MS2BS_RSP, 0, 0, f_bit, 0, 0, NULL, 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_RR_response |
+--------------------------------------------------------------------+

  PURPOSE : Build an RR response.

*/

GLOBAL void com_build_RR_response (UBYTE ch_type,
                                   UBYTE sapi, UBYTE nr, UBYTE f_bit)
{
  TRACE_FUNCTION ("com_build_RR_response()");
  com_build_frame (ch_type, RR_FRAME, sapi, MS2BS_RSP, 0, nr, f_bit, 0, 0, NULL, 0);
}

#if 0
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_RR_command  |
+--------------------------------------------------------------------+

  PURPOSE : Build an RR command.

*/

GLOBAL void com_build_RR_command (T_DL_DATA_STORE * dl_data, UBYTE ch_type,
                                  UBYTE sapi, UBYTE nr, UBYTE p_bit)
{
  TRACE_FUNCTION ("com_build_RR_command()");
  com_build_frame (dl_data, ch_type, RR_FRAME, sapi, MS2BS_CMD, 0, nr, p_bit, 0, 0, NULL, 0);
}
#endif  /* 0 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_REJ_response|
+--------------------------------------------------------------------+

  PURPOSE : Build an REJ response.

*/

GLOBAL void com_build_REJ_response (UBYTE ch_type,
                                    UBYTE sapi, UBYTE nr, UBYTE f_bit)
{
  TRACE_FUNCTION ("com_build_REJ_response()");
  com_build_frame (ch_type, REJ_FRAME, sapi, MS2BS_RSP, 0, nr, f_bit, 0, 0, NULL, 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : DL_COM                 |
| STATE   : code                    ROUTINE : com_build_DISC_command |
+--------------------------------------------------------------------+

  PURPOSE : Build a DISC command.

*/

GLOBAL void com_build_DISC_command (UBYTE ch_type,
                                    UBYTE sapi, UBYTE p_bit)
{
  TRACE_FUNCTION ("com_build_DISC_command()");
  com_build_frame (ch_type, DISC_FRAME, sapi, MS2BS_CMD, 0, 0, p_bit, 0, 0, NULL, 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)        MODULE  : DL_COM                    |
| STATE   : code                 ROUTINE : com_build_SABM            |
+--------------------------------------------------------------------+

  PURPOSE : Build a SABM command with or without Layer 3 message.

*/

GLOBAL void com_build_SABM (UBYTE ch_type,
                            UBYTE sapi, BOOL contention_resultion)
{
  GET_INSTANCE_DATA;
  T_QUEUE *queue = &dl_data->dcch0_queue;
  TRACE_FUNCTION ("com_build_SABM()");
  TRACE_EVENT_WIN_P3 ("com_build_SABM %s SAPI=%u %s", CH_TYPE_NAME[ch_type], sapi,
    (contention_resultion AND queue->switch_buffer) ? "+L3" : "");

  if (contention_resultion AND queue->switch_buffer)
  {
    com_build_frame (ch_type, SABM_FRAME, sapi, MS2BS_CMD, 0, 0, 1,
      (UBYTE)(queue->switch_buffer->sdu.l_buf >> 3), 0,
      queue->switch_buffer->sdu.buf, (UBYTE)(queue->switch_buffer->sdu.o_buf >> 3));
  }
  else
  {
    com_build_frame (ch_type, SABM_FRAME, sapi, MS2BS_CMD, 0, 0, 1, 0, 0, NULL, 0);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_DM_response |
+--------------------------------------------------------------------+

  PURPOSE : Build an DM response.

*/

GLOBAL void com_build_DM_response (UBYTE ch_type,
                                   UBYTE sapi, UBYTE f_bit)
{
  TRACE_FUNCTION ("com_build_DM_response()");
  com_build_frame (ch_type, DM_FRAME, sapi, MS2BS_RSP, 0, 0, f_bit, 0, 0, NULL, 0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_I_command   |
+--------------------------------------------------------------------+

  PURPOSE : Build an I command.

*/

GLOBAL void com_build_I_command (UBYTE ch_type,
                                 UBYTE sapi, UBYTE ns, UBYTE nr, UBYTE p_bit,UBYTE m_bit, T_QUEUE * queue)
{
  TRACE_FUNCTION ("com_build_I_command()");
  com_build_frame (ch_type, I_FRAME, sapi, MS2BS_CMD, ns, nr, p_bit,
    (UBYTE)(queue->transmit_buffer.l_buf >> 3), m_bit,
    queue->transmit_buffer.buf, 0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : DL_COM                 |
| STATE   : code                    ROUTINE : com_build_UI_command   |
+--------------------------------------------------------------------+

  PURPOSE : Build an UI command.

*/

GLOBAL void com_build_UI_command (UBYTE ch_type,
                                  UBYTE sapi, const T_FRAME * buffer)
{
  TRACE_FUNCTION ("com_build_UI_command()");
  com_build_frame (ch_type, UI_FRAME, sapi, MS2BS_CMD, 0, 0, 0,
    (UBYTE)(buffer->l_buf >> 3), 0, (UBYTE*)buffer->buf, (UBYTE)(buffer->o_buf >> 3));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : DL_COM                 |
| STATE   : code                    ROUTINE : com_build_UI_Bter      |
+--------------------------------------------------------------------+

  PURPOSE : Build an UI frame in Bter format.

*/

GLOBAL void com_build_UI_Bter (UBYTE ch_type)
{
  GET_INSTANCE_DATA;
  unsigned off = dl_data->rr_short_pd_buffer.o_buf>>3;
  unsigned len = dl_data->rr_short_pd_buffer.l_buf>>3;
  unsigned foff, maxlen;

  if (ch_type EQ L2_CHANNEL_SACCH)
  {
    maxlen = DL_N201_SACCH_Bter;
    foff = 2;
#ifndef  DL_2TO1
    dl_data->l2_frame.A[0] = dl_data->l2_frame.A[1] = 0; /* place holder for L1 header */
#else
    dl_data->l2_frame.frame_array[0] = dl_data->l2_frame.frame_array[1] = 0; /* place holder for L1 header */
#endif

  }
  else
  {
    foff = 0;
    maxlen = DL_N201_DCCH_Bter;
  }

  if (len > maxlen)
    len = maxlen;
  /*
   * LINT Warning 662: Possible creation of out-of-bounds pointer ...  and
   * LINT Warning 669: Possible data overrun for function 'memset ...
   * can be ignored, because of the right alignment between foff and maxlen.
   */
   /*lint -e662 -e669  Possible creation of out-of-bounds pointer or Possible data overrun*/
#ifndef DL_2TO1
  memcpy (&dl_data->l2_frame.A[foff], dl_data->rr_short_pd_buffer.buf+off, len);
  memset (&dl_data->l2_frame.A[foff+len], 0x2b, maxlen-len);
#else
  memcpy (&dl_data->l2_frame.frame_array[foff], dl_data->rr_short_pd_buffer.buf+off, len);
  memset (&dl_data->l2_frame.frame_array[foff+len], 0x2b, maxlen-len);
#endif
/* lint +e662 +e669 Possible creation of out-of-bounds pointer or Possible data overrun */
/* mark message as handled */
  dl_data->rr_short_pd_buffer.l_buf = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_build_frame       |
+--------------------------------------------------------------------+

  PURPOSE : Builds any frame.

*/
LOCAL void com_build_frame (UBYTE ch_type,
                            UBYTE type, UBYTE sapi, UBYTE cr, UBYTE ns,
                            UBYTE nr, UBYTE p_bit, UBYTE length, UBYTE m_bit,
                            UBYTE* pInfoBuffer, UBYTE InfoOffset)
{
  GET_INSTANCE_DATA;
  UBYTE offset;
  UBYTE maxlength;
#ifndef DL_2TO1
  UBYTE *pOutFrame = dl_data->l2_frame.A;
#else
  UBYTE *pOutFrame = dl_data->l2_frame.frame_array;
#endif /* DL_2TO1*/
#define SPARE 0
#define LPD   0
#define EA    1
#define EL    1

  if (ch_type EQ L2_CHANNEL_SACCH)
  {
    offset = 2;
    maxlength = DL_N201_SACCH_A_B;
    pOutFrame[0] = pOutFrame[1] = 0; /* reset layer 1 header */
  }
  else
  {
    offset = 0;
    maxlength = DL_N201_DCCH_A_B;
  }

  if (length > maxlength)
  {
    TRACE_EVENT_P1 ("Error: framelength to big %d", length);
    length = maxlength; /* for safety's sake */
  }

  /*
   * set header bytes (bit  8    7    6    5    4    3    2    1)
   * address field          |    |    |    |         |    |    |
   *                      Spare  --LPD-    ----SAPI---   C/R  EA
   */
  pOutFrame[offset] = (SPARE << 7) | (LPD << 5) | (sapi << 2) | (cr << 1) | EA;

  /*
   * control field          8    7    6    5    4    3    2    1
   *  I format              ----N(R)---    P    ----N(S)---    0
   *  S format              ----N(R)---   P/F   S    S    0    1
   *  U format              U    U    U   P/F   U    U    1    1
   */
  if ((type & 0x01) EQ 0)
    pOutFrame[offset+1] = (nr << 5) | (p_bit << 4) | (ns << 1);   /* I format */
  else if ((type & 0x02) EQ 0)
    pOutFrame[offset+1] = (nr << 5) | (p_bit << 4) | type;        /* S format */
  else
    pOutFrame[offset+1] = (p_bit << 4) | type;                    /* U format */

  /*
   * length field           8                        3    2    1
   *                        -----length indicator-----    M   EL
   */
  pOutFrame[offset+2] = (length << 2) | (m_bit << 1) | EL;

  /*
   * LINT Warning 662: Possible creation of out-of-bounds pointer ... and
   * LINT Warning 669: Possible data overrun for function 'memcpy ... and
   * LINT Warning 671: Possibly passing to function 'memset ...
   * can be ignored because the sum of offset, 3 plus maxlength (length) does
   * never exceed the maximal size of pOutFrame[].
   */
  if (length && pInfoBuffer)
  {/* copy info bits */
    memcpy (&pOutFrame[offset+3], pInfoBuffer + (InfoOffset ? InfoOffset : 3), length);
    offset += length;
    maxlength -= length;
  }

  /* fill remain of the frame with 0x2b */
  memset (&pOutFrame[offset+3], 0x2b, maxlength);

#undef SPARE
#undef LPD
#undef EA
#undef EL
}/* endfunc com_build_frame */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_data_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Indicates a downlinked message to RR. The queue which is
            taken depend on the sapi value. In case of sapi=0 the function
            checks the layer 3 message for a channel release additionally.
            This is done to speed up the release procedure.

*/
GLOBAL void com_data_ind (UBYTE ch_type, UBYTE sapi, ULONG fn)
{
  GET_INSTANCE_DATA;
  T_DL_DATA_IND *in_msg;

#if defined(INVOKE_SIGNAL)
    if (dl_data->interrupt_context)
    {
      sig_invoke_com_data_ind (ch_type, sapi, fn);
      return;
    }
#endif  /* INVOKE_SIGNAL */

  if (sapi EQ PS_SAPI_0)
  {
    in_msg = dl_data->dcch0_in_msg;

    if (in_msg)
    { /*
       * Purpose: Check if message is channel release, then start release
       * of connection. This will be done to speed up the DISC process.
       */
      /*lint -e415 (Warning -- access of out-of-bounds pointer) */
      /*lint -e416 (Warning -- creation of out-of-bounds pointer) */
      if ((in_msg->sdu.buf[3] EQ 0x06) AND   /* PD RR */
        (in_msg->sdu.buf[4] EQ 0x0D))      /* MT channel release */
      {
      /*lint +e416 (Warning -- creation of out-of-bounds pointer) */
      /*lint +e415 (Warning -- access of out-of-bounds pointer) */
        /*
         * Send immediately (on the next uplink opportunity) a DISC frame
         * on DCCH.
         * Disable dedicated mode -> restrain measurement reports
         */
        dl_data->dcch0_disc_request = TRUE;
        DL_OFFLINE_TRACE(TRACE_DL_EVENT, C_DCCH0, ch_type, "CHANNEL REL received");
#if !defined(LATE_LEAVING_DEDICATED)
        com_leave_dedicated (ch_type);
#endif  /* LATE_LEAVING_DEDICATED */
      }
    }
  }
  else
  {
    in_msg = dl_data->dcch3_in_msg;
  }

  if (in_msg)
  {
    in_msg->ch_type = ch_type;
    in_msg->sapi    = sapi;
    drr_dl_data_ind (sapi, fn);
  }
  else
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT,
      (sapi EQ PS_SAPI_0) ? C_DCCH0 : C_DCCH3, ch_type, "dcchx_in_msg=NULL");
  }
}

#if !defined(NTRACE)
#if defined(DL_TRACE_ENABLED) && defined(DL_IMMEDIATE_TRACE)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_l2trace           |
+--------------------------------------------------------------------+

  PURPOSE : Send L2 trace.

*/

GLOBAL void com_l2trace (UBYTE trace_type,
                         UBYTE channel, UBYTE ch_type, T_TIME trace_time, UBYTE* data)
{
  GET_INSTANCE_DATA;
  ULONG   trace_mask;

#ifdef TI_PS_HCOMM_CHANGE
  vsi_gettracemask(_hCommDL, _hCommDL, &trace_mask);
#else
  vsi_gettracemask(hCommDL, hCommDL, &trace_mask);
#endif
  if ((trace_mask & TC_USER1) EQ 0)
    return;

  if (trace_time EQ 0)
    vsi_t_time (VSI_CALLER &trace_time);

#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_com_l2trace (trace_type, channel, ch_type, trace_time, data);
    return;
  }
#endif  /* INVOKE_SIGNAL */
  dl_fast_trace(trace_type, channel, ch_type, trace_time, trace_mask, data);
}
#endif  /* DL_TRACE_ENABLED && DL_IMMEDIATE_TRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_l3trace           |
+--------------------------------------------------------------------+

  PURPOSE : Send L3 trace.

*/

GLOBAL void com_l3trace (UBYTE type, UBYTE ch_type, UBYTE *frame)
{
  GET_INSTANCE_DATA;
  ULONG trace_mask;

#ifdef TI_PS_HCOMM_CHANGE
  vsi_gettracemask(_hCommDL, _hCommDL, &trace_mask);
#else  
  vsi_gettracemask(hCommDL, hCommDL, &trace_mask);
#endif
  if ((trace_mask & TC_USER4) EQ 0)
    return;

#if defined(INVOKE_SIGNAL)
  if (dl_data->interrupt_context)
  {
    sig_invoke_com_l3trace (type, ch_type, frame);
    return;
  }
#endif  /* INVOKE_SIGNAL */

  if (type EQ TRACE_UPLINK)
  { /* uplink acknowledged */
    T_DL_DATA_REQ *d = (T_DL_DATA_REQ *)frame;
    com_print_l3trace (TRACE_UPLINK, ch_type, d->sapi, (UBYTE *)&d->sdu);
  }
  else if (type EQ TRACE_DOWNLINK)
  { /* downlink */
    T_DL_DATA_IND *d = (T_DL_DATA_IND *)frame;
    com_print_l3trace (TRACE_DOWNLINK, ch_type, d->sapi, (UBYTE *)&d->sdu);
  }
  else if ((type EQ TRACE_UACK_UP) OR (type EQ TRACE_UACK_DN))
  {
    com_print_l3trace (type, ch_type, PS_SAPI_0, frame);
  }
}

GLOBAL void com_print_l3trace (UBYTE type, UBYTE ch_type, UBYTE sapi, UBYTE *l3msg)
{
  char  description[40];

  switch (type)
  {
    case TRACE_UPLINK:
    case TRACE_DOWNLINK:
      {
        T_sdu *sdu = (T_sdu *)l3msg;
        sprintf (description, "L3 %s ch=%u SAPI%u",
          type EQ TRACE_UPLINK ? "UP" : "DN", ch_type, sapi);
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommDL, TC_USER4, description,
                      sdu->buf+(sdu->o_buf>>3), (sdu->l_buf>>3));
#else
        TRACE_BINDUMP(hCommDL, TC_USER4, description,
                      sdu->buf+(sdu->o_buf>>3), (sdu->l_buf>>3));
#endif
      }
      break;
    case TRACE_UACK_UP:
    case TRACE_UACK_DN:
      if (GET_BTER_FORMAT (&l3msg[0]) EQ SHORT_L2_HEADER_TYPE_1)
      {
        sprintf (description, "L3 %s ch=%u SAPI%u RR Short PD header",
          type EQ TRACE_UACK_UP ? "UP" : "DN", ch_type, sapi);
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommDL, TC_USER4, description, l3msg, DL_N201_SACCH_Bter);
#else
        TRACE_BINDUMP(hCommDL, TC_USER4, description, l3msg, DL_N201_SACCH_Bter);
#endif
      }
      else
      {
        sprintf (description, "L3 %s ch=%u SAPI%u",
          type EQ TRACE_UACK_UP ? "UP" : "DN", ch_type, sapi);
#ifdef TI_PS_HCOMM_CHANGE
        TRACE_BINDUMP(_hCommDL, TC_USER4, description, l3msg, DL_N201_SACCH_A_B);
#else
        TRACE_BINDUMP(hCommDL, TC_USER4, description, l3msg, DL_N201_SACCH_A_B);
#endif
      }
      break;
  }
}
#endif  /* !NTRACE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)            MODULE  : DL_COM                |
| STATE   : code                     ROUTINE : com_init_data         |
+--------------------------------------------------------------------+

  PURPOSE : Initializes the data for one instance.

*/

GLOBAL void com_init_data (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("com_init_data()");

  memset (dl_data, 0, sizeof (T_DL_DATA_STORE));

  dl_data->cch[C_DCCH0].T200_counter = 0;
  dl_data->cch[C_DCCH3].T200_counter = 0;
  dl_data->cch[C_SACCH0].T200_counter = 0;
  dl_data->dcch0_in_msg = NULL;
  dl_data->dcch3_in_msg = NULL;

  dl_data->dcch0_disc_request = FALSE;
  dl_data->dcch3_disc_request = FALSE;
  dl_data->RR_dedicated = FALSE;
  dl_data->fn = NOT_PRESENT_32BIT;

#if defined(INVOKE_SIGNAL)
  sig_init_signal_data ();
#endif /* INVOKE_SIGNAL */
}


#if defined (DL_TRACE_ENABLED) || defined (FF_EM_MODE)
/*
*   Some of the functions originally designed for DL tracing are used as well for
*   the engineering mode and therefore defined here.
*/

GLOBAL void com_semaphore_err (void)
{
#if defined(_SIMULATION_)
  SYST_TRACE ("DL:error:semaphore");
  TRACE_ASSERT (1);
#else  /* _SIMULATION_ */
static UCHAR out = 0;
  if (!out)
  {
    out = 1;
    SYST_TRACE ("DL:error:semaphore");
    vsi_t_sleep(VSI_CALLER 10000);
    TRACE_ASSERT (1);
  }
#endif  /* _SIMULATION_ */
}

GLOBAL int com_enter_critical_section (T_HANDLE sem)
{
  if (vsi_s_get (VSI_CALLER sem) NEQ VSI_OK)
  {
    com_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}/*endfunc com_enter_critical_section*/

GLOBAL int com_leave_critical_section (T_HANDLE sem)
{
  if (vsi_s_release (VSI_CALLER sem) NEQ VSI_OK)
  {
    com_semaphore_err();
    return -1;
  }
  else
  {
    return 0;
  }
}/* endfunc com_leave_critical_section */

#if !defined(DL_IMMEDIATE_TRACE)
GLOBAL int com_semaphore_state (T_HANDLE sem)
{
  USHORT  semCount;

  if (vsi_s_status (VSI_CALLER sem, &semCount) NEQ VSI_OK)
  {
    com_semaphore_err ();
    return -1;
  }
  if (semCount EQ 0)
  {
    SYST_TRACE ("DL:semCount == 0");
    return 1;
  }
  else
    return 0;
}
#endif /* !DL_IMMEDIATE_TRACE */
#endif /* defined (DL_TRACE_ENABLED) || defined (FF_EM_MODE) */

#endif  /* DL_COM_C */

/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_DPH
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
|  Purpose :  This Modul defines the functions for distributing
|             all primitives of the lower layers.
+-----------------------------------------------------------------------------
*/

#ifndef DL_DPH_C
#define DL_DPH_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL

/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include <string.h>
#include "pconst.cdg"
#include "vsi.h"
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

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/
#if defined(_SIMULATION_) && !defined(DL_2TO1)
LOCAL void dph_ph_data_req (T_PH_DATA_REQ * data_req);
#endif /* _SIMULATION_ && !DL_2TO1 */

#if defined(DL_2TO1) || defined(USE_L1M_GS001_1)
LOCAL U8 convert_dcch_ch_type (U8 channel_type);
#endif  /* DL_2TO1 || USE_L1M_GS001_1 */

LOCAL void dl_process_downlink (UBYTE error_flag, UBYTE channel_type,
                                UBYTE * frame, ULONG fn);

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
#if defined(DL_2TO1)
#if defined(_SIMULATION_)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_ready_to_send       |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a PH_READY_TO_SEND primitive.
            Uplink opportunity for simulation.

*/

GLOBAL void l1test_call_mphc_read_dcch (T_L1TEST_CALL_MPHC_READ_DCCH * ready)
{
  UBYTE  no_signalling_flag = NO_SIGNALLING;
  T_RADIO_FRAME *frame = NULL;

  GET_INSTANCE_DATA;  
  dl_data->dl_active = FALSE;

  switch (ready->chn_mode)
  {
    case CM_SIGNALLING_ONLY:
      /*
      * Take over the behaviour of dll_read_dcch():
      * No ch_type is given by the layer 1 for SDCCH and FACCH,
      * the first parameter ch_type of the fuction dl1_uplink_ind() get a
      * value set to zero.
      */
      no_signalling_flag = SIG_ONLY;
      break;
    default:
      break;
  }

  frame = dl1_uplink_ind (0, no_signalling_flag);

  if (frame)
  {
    l1test_return_mphc_read_dcch (frame);
  }

  MY_PFREE (ready);
}

GLOBAL void l1test_call_mphc_read_sacch (T_L1TEST_CALL_MPHC_READ_SACCH * ready)
{
  T_RADIO_FRAME *frame = NULL;

  GET_INSTANCE_DATA;
  dl_data->dl_active = FALSE;

  frame = dl1_uplink_ind (L2_CHANNEL_SACCH, SIG_ONLY);
  if (frame)
  {
    l1test_return_mphc_read_sacch (frame);
  }

  MY_PFREE (ready);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : Sending of a PH_DATA_REQ primitive.
            Uplink for simulation.

*/

GLOBAL void l1test_return_mphc_read_dcch (T_RADIO_FRAME * frame)
{
  PALLOC(data_req, L1TEST_RETURN_MPHC_READ_DCCH);

  TRACE_FUNCTION ("l1test_return_mphc_read_dcch()");

  memcpy(data_req->l2_frame.frame_array, frame->frame_array, sizeof(data_req->l2_frame));
  PSEND (hCommPL, data_req);
}

GLOBAL void l1test_return_mphc_read_sacch (T_RADIO_FRAME * frame)
{
  PALLOC(data_req, L1TEST_RETURN_MPHC_READ_SACCH);

  TRACE_FUNCTION ("l1test_return_mphc_read_sacch()");

  memcpy(data_req->l2_frame.frame_array, frame->frame_array, sizeof(data_req->l2_frame));
  PSEND (hCommPL, data_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : l1test_call_mphc_dcch_downlink            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a L1TEST_CALL_MPHC_DCCH_DOWNLINK primitive.

*/

GLOBAL void l1test_call_mphc_dcch_downlink (T_L1TEST_CALL_MPHC_DCCH_DOWNLINK * data_ind)
{
  /*
  * Take over the behaviour of dll_dcch_downlink():
  * No ch_type is given by the layer 1 for SDCCH and FACCH,
  * the second parameter ch_type of the function dl_process_downlink() get a
  * value set to zero.
  */
  dl_data->dl_active = FALSE;

  dl_process_downlink (data_ind->valid_flag, 0, data_ind->l2_frame.frame_array, NOT_PRESENT_32BIT);

  MY_PFREE (data_ind);
}
#endif  /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a PH_DATA_IND primitive.
            This function is only available in stacks >= TCS5 (DL_2TO1).
            A function with the same name exist for the old GSM/GPRS stack.

            It is used for SACCH downlink on target as well as on simulation.
            In opposite to the old GSM/GPRS stack, a functional interface
            (l1test_call_mphc_dcch_downlink) is used by the dual mode stack
            implementation for SDCCH and FACCH during simulation. Therefore
            this function is only used for SACCH.

*/

GLOBAL void dph_ph_data_ind (T_MPHC_PH_DATA_IND * data_ind)
{
  /*
  * Take over the behaviour of dll_dcch_downlink():
  * No ch_type is given by the layer 1 for SDCCH and FACCH,
  * the second parameter ch_type of the function dl_process_downlink() get a
  * value set to zero.
  */

  TRACE_FUNCTION ("dph_ph_data_ind() 2TO1");

  if (data_ind->l2_channel_type EQ L2_CHANNEL_SACCH)
    dl_process_downlink (data_ind->error_cause,
                         L2_CHANNEL_SACCH,
                         data_ind->l2_frame.frame_array,
                         NOT_PRESENT_32BIT);

  MY_PFREE (data_ind);
}

#else /* DL_2TO1 */

#if defined(_SIMULATION_)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_ready_to_send       |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a PH_READY_TO_SEND primitive.
            Uplink opportunity for simulation.

*/

GLOBAL void dph_ph_ready_to_send (T_PH_READY_TO_SEND * ready)
{
  UBYTE   no_signalling_flag = SIG_ONLY;
  T_RADIO_FRAME *frame;
  UBYTE   ch_type;
  switch (ready->ch_type)
  {
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
      no_signalling_flag = NO_SIGNALLING;
      /*lint -fallthrough */
    case L2_CHANNEL_SDCCH:
      /*
      * Take over the behaviour of dll_read_dcch():
      * No ch_type is given by the layer 1 for SDCCH and FACCH,
      * the first parameter ch_type of the function dl1_uplink_ind() get a
      * value set to zero.
      */
      ch_type = 0;
      break;
    default:
      ch_type = ready->ch_type;
      break;
  }

  frame = dl1_uplink_ind (ch_type, no_signalling_flag);
  if (frame)
  {
    PALLOC_SDU (data, PH_DATA_REQ, 23*BITS_PER_BYTE);/* T_PH_DATA_REQ */

    /*
     * In case the value of ready->ch_type was set to zero above it has to set
     * to the value of dcch0_ch_type corresponding the behaviour of the
     * function dl1_uplink_ind().
    if (ready->ch_type EQ 0)
    {
      T_DL_DATA *dl_data = dl_get_data ();
      ready->ch_type = dl_data->dcch0_ch_type ? dl_data->dcch0_ch_type : L2_CHANNEL_SDCCH;
    }
    */

    memcpy (data->sdu.buf, frame, 23);/*lint !e419 (Warning -- Apparent data overrun) */
    if (ready->ch_type EQ L2_CHANNEL_SACCH)
    {
      data->sdu.buf[0] = 0;/* layer 1 header occupies 2 bytes */
      data->sdu.buf[1] = 0;/*lint !e415 (Warning -- access of out-of-bounds pointer) */
      data->sdu.o_buf = 2 * BITS_PER_BYTE;
      data->sdu.l_buf = 21 * BITS_PER_BYTE;
    }
    else
    {
      data->sdu.o_buf = 0;
      data->sdu.l_buf = 23 * BITS_PER_BYTE;
    }
    data->ch_type = ready->ch_type;
    dph_ph_data_req (data);
  }

  MY_PFREE (ready);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_data_req            |
+--------------------------------------------------------------------+

  PURPOSE : Sending of a PH_DATA_REQ primitive.
            Uplink for simulation.

*/

LOCAL void dph_ph_data_req (T_PH_DATA_REQ * data_req)
{
  TRACE_FUNCTION ("dph_ph_data_req()");

#if defined(DL_TRACE_WIN32)
  if (data_req->ch_type EQ L2_CHANNEL_SACCH)
  {
    /*lint -e416 (Warning -- creation of out-of-bounds pointer) */
    FTRC (data_req->ch_type, data_req->sdu.buf+2, 1); /* uplink SACCH */
    /*lint +e416 (Warning -- creation of out-of-bounds pointer) */
  }
  else
  {
    FTRC (data_req->ch_type, data_req->sdu.buf, 1); /* uplink other */
  }
#endif  /* DL_TRACE_WIN32 */

  PSENDX (PL, data_req);
}
#endif  /* _SIMULATION */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dph_ph_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a PH_DATA_IND primitive.
            This function is only available in the old GSM/GPRS stack (<= TCS4).
            A function with the same name exist for the dual mode stack.

            It can be used for SACCH downlink on target as well as on simulation.
            During simulation the old GSM/GPRS stack uses this primitive
            function for SDCCH and FACCH also. In this case it takes over the
            functional interface dll_dcch_downlink().
            In the current implementation of the old GSM/GPRS stack the ALR
            queue is exclusively used for incoming PH_DATA_IND primitives from
            L1 for target builds. Only Acknowledged frames (SAPI=3) and frames
            with Bter format (short PD header) are forwarded to DL and handled
            by this function.

*/

GLOBAL void dph_ph_data_ind (T_PH_DATA_IND * ph_data_ind)
{
  if (ph_data_ind)
  {
    #if defined(_SIMULATION_)
      /*
      * Take over the behaviour of dll_dcch_downlink():
      * No ch_type is given by the layer 1 for SDCCH and FACCH,
      * the first parameter ch_type of the function dl_process_downlink() get a
      * value set to zero.
      */
      switch (ph_data_ind->l2_channel_type)
      {
        case L2_CHANNEL_SDCCH:
        case L2_CHANNEL_FACCH_F:
        case L2_CHANNEL_FACCH_H:
          ph_data_ind->l2_channel_type = 0;
          break;
        default:
          break;
      }
    #else  /* _SIMULATION */
      if (ph_data_ind->l2_channel_type EQ L2_CHANNEL_SACCH)
    #endif  /* _SIMULATION */
        dl_process_downlink (ph_data_ind->error_cause,
                             ph_data_ind->l2_channel_type,
                             ph_data_ind->l2_frame.A,
                             NOT_PRESENT_32BIT);

    MY_PFREE (ph_data_ind);
  }
}
#endif /* DL_2TO1 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dl_process_downlink        |
+--------------------------------------------------------------------+

  PURPOSE : really processing of layer 2 frame downlink.

*/
LOCAL void dl_process_downlink (UBYTE error_flag, UBYTE channel_type, UBYTE * frame, ULONG fn)
{
  GET_INSTANCE_DATA; 

  TRACE_FUNCTION ("dll_process_downlink()");

  /* set the active dedicated channel type if necessary */
  if (channel_type EQ 0)
  {
    TRACE_EVENT_WIN_P3 ("dl_process_downlink(%u,) -> ch=%u FN=%d", channel_type, dl_data->dcch0_ch_type, ((int)fn));
    channel_type = dl_data->dcch0_ch_type  ? dl_data->dcch0_ch_type : L2_CHANNEL_SDCCH;
  }
  else
  {
    TRACE_EVENT_WIN_P1 ("dl_process_downlink(%u,)", channel_type);
  }

  if (channel_type NEQ L2_CHANNEL_SACCH)
    dl_data->interrupt_context = TRUE;

  if (fn NEQ NOT_PRESENT_32BIT)
    dl_data->fn = fn;

#if defined(DL_TRACE_ENABLED)
  {
    UCHAR trace_channel = TRACE_CH_UNKNOWN;
    UCHAR frame_sapi;

    switch (channel_type)
    {
    case L2_CHANNEL_SACCH:
      frame_sapi = ((*(frame + 2)) & 0x1c) >> 2;
      if (frame_sapi EQ PS_SAPI_0)
        trace_channel = C_DCCH0;
      else if (frame_sapi EQ PS_SAPI_3)
        trace_channel = C_DCCH3;
      break;
    case L2_CHANNEL_SDCCH:
      frame_sapi = ((*frame) & 0x1c) >> 2;
      if (frame_sapi EQ PS_SAPI_0)
        trace_channel = C_DCCH0;
      else if (frame_sapi EQ PS_SAPI_3)
        trace_channel = C_DCCH3;
      break;
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
      frame_sapi = ((*frame) & 0x1c) >> 2;
      if (frame_sapi EQ PS_SAPI_0)
        trace_channel = C_DCCH0;
      break;
    default:
      break;
    }/* endswitch chan */
    if (error_flag EQ VALID_BLOCK)
    {
      DL_OFFLINE_TRACE (TRACE_DOWNLINK, trace_channel, channel_type, &frame[0]);
    }
  }
#endif /* DL_TRACE_ENABLED */

#if defined(DL_TRACE_WIN32)
  if (error_flag EQ VALID_BLOCK)
  {
    if (channel_type EQ L2_CHANNEL_SACCH)
      FTRC (channel_type, frame+2, 0); /* downlink SACCH */
    else
      FTRC (channel_type, frame, 0); /* downlink other */
  }
#endif /* DL_TRACE_WIN32 */

  if (channel_type EQ 0)
  {
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, channel_type, "DL:no valid channel type");
  }
  else
  {
    dl_downlink (error_flag, channel_type, &frame[0], fn);
  }

  dl_data->interrupt_context = FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dl1_uplink_ind             |
+--------------------------------------------------------------------+

  PURPOSE : Functional Interface for the uplink direction of the
            logical channels SACCH/SDCCH/FACCH.
            (only used by the target)

*/
GLOBAL T_RADIO_FRAME * dl1_uplink_ind (UBYTE channel_type, UBYTE no_signalling_flag)
{
  T_RADIO_FRAME * pRadioFrame;
  UCHAR   channel;
  UCHAR   sapi;
  UCHAR   signalling = SIG_ONLY;

  GET_INSTANCE_DATA; 

  dl_data->interrupt_context = TRUE;

  /* set the active dedicated channel type if necessary */
  if (channel_type EQ 0)
    channel_type = dl_data->dcch0_ch_type;

  TRACE_EVENT_WIN_P1 ("dl1_uplink_ind(ch_type=%u,)", channel_type);

  switch (channel_type)
  {
    case L2_CHANNEL_SACCH:
      /*
       * The priority arrangement on the SACCH must ensure that if a SAPI = 3
       * frame is awaiting transmission, two SAPI = 0 frames are not sent in
       * consecutive SACCH frames. In addition, for the mobile to network
       * direction it must also be ensured that any SAPI = 3 frame is followed
       * by at least one SAPI = 0 frame.
       *
       * SAPI = 3 is set as default value. The function dl_uplink() together with
       * the variable 'sacch_last_uplink_sapi' determind the actual value.
       * e.g. if last uplinked SACCH frame was one with SAPI=3
       *       or
       *      if no SACCH SAPI=3 frame is awaiting transmission
       *      then a SACCH SAPI=0 frame should be uplinked
       */
      channel = C_SACCH0;
      sapi = PS_SAPI_3;
      TRACE_EVENT_WIN_P2 ("dl1_uplink_ind(%s, %s)",
        CH_TYPE_NAME[channel_type], signalling EQ SIG_ONLY?"SIG_ONLY":"NO_SIGNALLING");
      break;

    default:
      TRACE_ERROR ("dl1_uplink_ind():no valid channel type, use SDCCH instead");
      channel_type = L2_CHANNEL_SDCCH;
      /*lint -fallthrough*/
    case L2_CHANNEL_FACCH_F:
    case L2_CHANNEL_FACCH_H:
    case L2_CHANNEL_SDCCH:
      /*
       * build frame for SAPI = 3 will be called (with lower priority)
       * if no data is waiting for SAPI = 0.
       */
      channel = C_DCCH0;
      sapi = PS_SAPI_0;
      dl_data->cch[channel].ch_type = channel_type;
      signalling = no_signalling_flag;
      TRACE_EVENT_WIN_P3 ("dl1_uplink_ind(%s, %s) SAPI=%u",
        CH_TYPE_NAME[channel_type], signalling EQ SIG_ONLY?"SIG_ONLY":"NO_SIGNALLING",
        sapi);
      break;
  }

  pRadioFrame= dl_uplink (channel, sapi, signalling, FALSE);
  if (pRadioFrame)
  {
#ifndef DL_2TO1
    DL_OFFLINE_TRACE (TRACE_UPLINK, channel, channel_type, &pRadioFrame->A[0]);
#else
    DL_OFFLINE_TRACE (TRACE_UPLINK, channel, channel_type, &pRadioFrame->frame_array[0]);
#endif
  }

  dl_data->interrupt_context = FALSE;
  return pRadioFrame;  /* return pointer to frame to layer 1 */
}

#if defined(DL_2TO1) || defined(USE_L1M_GS001_1)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : convert_dcch_ch_type       |
+--------------------------------------------------------------------+

  PURPOSE : Converts dedicated channel types to layer 2 channel types
            according to L1M_GS001_1 and L1M_GS001_3.

*/
LOCAL U8 convert_dcch_ch_type (U8 channel_type)
{
  UBYTE ch_type;
  switch (channel_type)
  {
    case MPHC_CH_TCH_F:  ch_type = L2_CHANNEL_FACCH_F;break;
    case MPHC_CH_TCH_H:  ch_type = L2_CHANNEL_FACCH_H;break;
    case MPHC_CH_SDCCH_4:
    case MPHC_CH_SDCCH_8:ch_type = L2_CHANNEL_SDCCH;break;
  }
  return ch_type;
}
#endif  /* DL_2TO1 || USE_L1M_GS001_1 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dll_read_dcch              |
+--------------------------------------------------------------------+

  PURPOSE : Functional Interface for the uplink direction of
            SDCCH/FACCH. Only used by the TI target, look into
            TI interface S922 or L1M_GS001_1.

*/
#if defined(DL_2TO1) || defined(USE_L1M_GS001_1)
GLOBAL T_RADIO_FRAME * dll_read_dcch (U8 chn_mode, U8 channel_type)
{
  return dl1_uplink_ind(convert_dcch_ch_type(channel_type), chn_mode);
}
#else  /* DL_2TO1 || USE_L1M_GS001_1 */
GLOBAL T_RADIO_FRAME * dll_read_dcch (U8 chn_mode)
{
  /* 0 stands for the currently active SDCCH and FACCH channel type */
  return dl1_uplink_ind(0, chn_mode);
}
#endif  /* DL_2TO1 || USE_L1M_GS001_1 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dll_read_sacch             |
+--------------------------------------------------------------------+

  PURPOSE : Functional Interface for the uplink direction of SACCH.
            Only used by the TI target, look into
            TI interface S922 or L1M_GS001_1.

*/
GLOBAL T_RADIO_FRAME * dll_read_sacch (UBYTE chn_mode)
{
  return dl1_uplink_ind (L2_CHANNEL_SACCH, SIG_ONLY);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : DL_DPH                     |
| STATE   : code                ROUTINE : dll_dcch_downlink          |
+--------------------------------------------------------------------+

  PURPOSE : Functional Interface for the downlink direction of
            FACCH/DCCH. Only used by the TI target, look into
            TI interface S922 or L1M_GS001_1.

*/
#if defined(DL_2TO1) || defined(USE_L1M_GS001_1)
GLOBAL void dll_dcch_downlink(U32 * data_ptr, U8 valid_flag, U8 channel_type, U32 fn)
{
  if ((data_ptr NEQ NULL) AND (valid_flag EQ DATA_VALID))
  {
    dl_process_downlink (VALID_BLOCK, convert_dcch_ch_type(channel_type),
                         (UBYTE *)data_ptr, fn);
  }
}
#else  /* DL_2TO1 || USE_L1M_GS001_1 */
#if defined(SEND_FN_TO_L2_IN_DCCH) && (SEND_FN_TO_L2_IN_DCCH == 1)
  GLOBAL void dll_dcch_downlink(U32 * data_ptr, U8 valid_flag, U32 fn)
  {
    if ((data_ptr NEQ NULL) AND valid_flag)
    {
      /*
       * channel type 0 stands for currently active dedicated
       * SDCCH or FACCH channel type
       */
      dl_process_downlink (VALID_BLOCK, 0, (UBYTE *)data_ptr, fn);
    }
  }
#else /* SEND_FN_TO_L2_IN_DCCH == 1 */
  GLOBAL void dll_dcch_downlink(U32 * data_ptr, U8 valid_flag)
  {
    if ((data_ptr NEQ NULL) AND valid_flag)
    {
      /*
       * channel type 0 stands for currently active dedicated
       * SDCCH or FACCH channel type
       */
      dl_process_downlink (VALID_BLOCK, 0, (UBYTE *)data_ptr, NOT_PRESENT_32BIT);
    }
  }
#endif /* SEND_FN_TO_L2_IN_DCCH == 1 */
#endif  /* DL_2TO1 || USE_L1M_GS001_1 */
#endif /* DL_DPH_C */

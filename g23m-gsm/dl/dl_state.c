/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL_STATE
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
|  Purpose :  This Modul defines the state machine of the component DL
|             (replaces the old channel dependent implementation)
+-----------------------------------------------------------------------------
*/

#ifndef DL_STATE_C
#define DL_STATE_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_DL
#define NEW_REJ_ACK /* Acknowledgement by valid reject frame
                   * in accordance with 3GPP 04.06, 5.5.3.1
                   * "On receipt of a valid I frame or supervisory frame"
                   */

/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include <string.h>
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
#include "dl_em.h"
#include "dl_trc.h"

/*==== TYPEDEFS ===================================================*/
typedef struct
{
  UBYTE channel;
  UBYTE sapi;
  UBYTE state;
  UBYTE T200_Stop;
  UBYTE T200_Start;
  UBYTE pf_bit_flag;
  UBYTE dl_data_ind;
  UBYTE mdl_error_ind;
  T_CCH*  pcch;
} T_CCH_INTERN;
#define T_CCH_INTERN_INIT {0,0,0,0,0,0,0}

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
static  int frame_validation (UBYTE channel_type, UBYTE* frame);

static  int downlink_idle (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  int downlink_contention_resolution (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  int downlink_mfe (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  int downlink_timer_recovery (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  int downlink_awaiting_release (T_CCH_INTERN* pcch_i, UBYTE *frame);

static  void downlink_mfe_information (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_mfe_supervisory (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_mfe_sabm (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_mfe_dm (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_mfe_tr_unnumbered (T_CCH_INTERN* pcch_i, UBYTE *frame, UBYTE state);
static  void downlink_i_frame (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_tr_supervisory (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void downlink_tr_information (T_CCH_INTERN* pcch_i, UBYTE *frame);

static  void invoke_retransmission (T_CCH_INTERN* pcch_i, UBYTE frame_nr);
static  void enquiry_response (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void mdl_error_ind (UBYTE cause, UBYTE channel_type, UBYTE sapi);
static  void nr_error_recovery (T_CCH_INTERN* pcch_i, UBYTE *frame);
static  void concatenate (UBYTE ch_type, UBYTE sapi, UBYTE *frame);
static  void free_sending_buffer (UBYTE ch_type, UBYTE sapi);

static  void repeat_sabm (UBYTE channel, UBYTE sapi);
static  void delayed_release_ind(UBYTE channel);

static  int uplink_idle (UBYTE channel, UBYTE sapi);
static  int uplink_awaiting_establishment (UBYTE channel, UBYTE sapi, UBYTE no_signalling_mode);
static  int uplink_mfe (UBYTE channel, UBYTE sapi, UBYTE no_signalling_mode);
static  int uplink_timer_recovery (UBYTE channel, UBYTE sapi, UBYTE no_signalling_mode);

static  void T200_expiry (UBYTE channel, UBYTE sapi);

/*==== VARIABLES ==================================================*/

/*Removed the const from the definition,rework for issue 25370*/
static  UBYTE l2_empty_frame [25] = {
        0x00, 0x00, /* the first two dummy bytes only for SACCH L1 header! */
        /* here begins the normal empty frame (SDCCH, FACCH) */
        0x01, /* address field: SAPI 0 */
        0x03, /* control field: UI frame */
        0x01, /* length field: length = 0 */
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b };
#if 0
static  UBYTE l2_invalid_frame [25] = {
        0x00, 0x00, /* the first two dummy bytes only for SACCH L1 header! */
        /* here begins the normal empty frame (SDCCH, FACCH) */
        0x1D, /* address field: SAPI 7 (unallocated SAPI; no action shall be taken on such frames) */
        0x03, /* control field: UI frame */
        0x01, /* length field: length = 0 */
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2b, 0x2b, 0x2b, 0x2b, 0x2b };
static  UBYTE l2_invalid_frame_0 [25] = {
        0x00, 0x00, /* the first two dummy bytes only for SACCH L1 header! */
        /* here begins the normal empty frame (SDCCH, FACCH) */
        0x00, /* address field: SAPI 0 */
        0x00, /* control field: UI frame */
        0x00, /* length field: length = 0 */
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00 };
#endif /* 0|1 */
static  T_CCH_INTERN  cch_i;

#if defined(CHECK_PCCHI)
#define CHECK_PCCH_I()  if (check_pcch_i(pcch_i, __LINE__)()return;
#define CHECK_PCCH_Ir() if (check_pcch_i(pcch_i, __LINE__)()return -1;
#else
#define CHECK_PCCH_I()
#define CHECK_PCCH_Ir()
#endif  /* CHECK_PCCHI */

/*==== FUNCTIONS ==================================================*/
int dl_downlink (UBYTE error_flag, UBYTE channel_type, UBYTE* frame, ULONG fn)
{
  UBYTE     channel_state = STATE_INVALID;
  UBYTE     channel = NOT_PRESENT_8BIT;
  UBYTE     frame_sapi = PS_SAPI_0; /* to calm lint, will be new set for valid frames */
  static UBYTE  invalid = 0;
  int       ret = -2;
  int       cause;
  int       l2_offset;

  GET_INSTANCE_DATA;

  TRACE_EVENT_WIN_P1 ("downlink(): dcch0_ch_type:%s",
    CH_TYPE_NAME[dl_data->dcch0_ch_type]);
  TRACE_EVENT_WIN_P6 ("DL: DCCH0=%s,%s vr=%u vs=%u va=%u T200=%u",
    CH_TYPE_NAME[dl_data->cch[C_DCCH0].ch_type],
    STATE_DCCH0_NAME[dl_data->state[C_DCCH0]],
    dl_data->cch[C_DCCH0].vr, dl_data->cch[C_DCCH0].vs, dl_data->cch[C_DCCH0].va,
    dl_data->cch[C_DCCH0].T200_counter);
  TRACE_EVENT_WIN_P6 ("DL: DCCH3=%s,%s vr=%u vs=%u va=%u T200=%u",
    CH_TYPE_NAME[dl_data->cch[C_DCCH3].ch_type],
    STATE_DCCH3_NAME[dl_data->state[C_DCCH3]],
    dl_data->cch[C_DCCH3].vr, dl_data->cch[C_DCCH3].vs, dl_data->cch[C_DCCH3].va,
    dl_data->cch[C_DCCH3].T200_counter);

  if (channel_type EQ L2_CHANNEL_SACCH)
  {
    l2_offset = 2; /* with layer 1 header */
  }
  else
  {
    l2_offset = 0; /* without layer 1 header */
  }

#define RR_SHORT_PD_HANDLING
#if defined(RR_SHORT_PD_HANDLING)
  /* Handling of unacknowledged UI frames with format type Bter */
#if defined(RR_SHORT_PD_DETECT_KNOWN_MSG_ONLY) /* detection of known messages only */
  cause = 0;
  switch (frame[l2_offset])
  {
    case RR_SHORT_PD_SI10:
    case RR_SHORT_PD_MEAS_INFO:
      if (channel_type EQ L2_CHANNEL_SACCH)
        cause = 1;
      break;
    case RR_SHORT_PD_NOTI_FACCH:
      if ((channel_type EQ L2_CHANNEL_FACCH_F) OR
          (channel_type EQ L2_CHANNEL_FACCH_H))
          cause = 1;
      break;
    case RR_SHORT_PD_UPLINK_FREE:
      cause = 1;
      break;
    default:
      break;
  }
  if (cause)
  {
    drr_dl_short_unitdata_ind (channel_type, error_flag, frame,
      &frame[l2_offset],
      (channel_type EQ L2_CHANNEL_SACCH) ? DL_N201_SACCH_Bter : DL_N201_DCCH_Bter, fn);
    return 0;
  }
#else /* detection of all possible messages with short L2 header and format Bter */
  TRACE_EVENT_WIN_P4 ("detection of format Bter: %02x&%02x=%02x ?= %02x",
    frame[l2_offset], BTER_FORMAT_MASK, GET_BTER_FORMAT (&frame[l2_offset]), SHORT_L2_HEADER_TYPE_1);
  if ((GET_BTER_FORMAT (&frame[l2_offset]) EQ SHORT_L2_HEADER_TYPE_1))
  {
    drr_dl_short_unitdata_ind (channel_type, error_flag, frame,
      &frame[l2_offset],
      (UBYTE)((channel_type EQ L2_CHANNEL_SACCH) ? DL_N201_SACCH_Bter : DL_N201_DCCH_Bter), fn);
    return 0;
  }
#endif /* kind of Bter detection */
#endif /* RR_SHORT_PD_HANDLING */

  /* check frame */

  if (error_flag EQ VALID_BLOCK)
  {
    frame_sapi = GET_SAPI (frame+l2_offset);
    if ((frame_sapi NEQ PS_SAPI_0) AND (frame_sapi NEQ PS_SAPI_3))
    {
      TRACE_EVENT_WIN_P1 ("downlink() returns -1 (wrong SAPI=%u)", frame_sapi);
      return -1;
    }

    cause = frame_validation (channel_type, frame+l2_offset);
    if (cause >= 0)
    {
      TRACE_FUNCTION ("frame validation failed!");
      if (invalid EQ 0)
      {
        TRACE_ERROR ("invalid frame");
        invalid = 1; /* only one message per succession */
      }
      mdl_error_ind ((UBYTE)cause, channel_type, frame_sapi);
      return 0;/* ETSI GSM 04.06 Annex G.2 - G.4 */
    }
    else
      invalid = 0;

#if defined(DL_2TO1) || defined(_SIMULATION_)
    /* Handling of unacknowledged UI frames on SACCH with SAPI=0 (not format type Bter) */
    if ((channel_type EQ L2_CHANNEL_SACCH) AND
        (frame_sapi EQ PS_SAPI_0) AND
        (GET_FORMAT_TYPE(frame+l2_offset) EQ U_FORMAT) AND
        (GET_U_TYPE(frame+l2_offset) EQ UI_FRAME) )
    {
      drr_dl_unitdata_ind (error_flag, frame, frame+l2_offset+3,
                           (UBYTE)(GET_LENGTH_INDICATOR (frame+l2_offset)), fn);
      return 0;
    }
#endif /* DL_2TO1 || _SIMULATION_ */
  }
#if defined(DL_2TO1)
  else if (channel_type EQ L2_CHANNEL_SACCH)
  { /*
     * Indicate invalid SACCH frame for decrement of radio link timeout counter.
     * The invalid frame possible contains invalid headers, use length 0.
     */
    drr_dl_unitdata_ind (error_flag, frame, frame+l2_offset+3, 0, fn);
  }
#endif /* DL_2TO1 */

  if (error_flag NEQ VALID_BLOCK)
  {
    TRACE_EVENT_WIN ("invalid frame->stop download handling");
    return 0; /* no further handling */
  }

  memset (&cch_i, 0, sizeof (T_CCH_INTERN));
  switch (channel_type)
  {
  case L2_CHANNEL_SDCCH:
    if (frame_sapi EQ PS_SAPI_3)
    {
      channel = C_DCCH3;
      break;
    }
    /*lint -fallthrough*/
  case L2_CHANNEL_FACCH_F:
  case L2_CHANNEL_FACCH_H:
    if (frame_sapi EQ PS_SAPI_0)
      channel = C_DCCH0;
    break;
  case L2_CHANNEL_SACCH:
    if (frame_sapi EQ PS_SAPI_3)
    { /* SACCH with SAPI=3 only supported with associated TCH (FACCH) */
      if ((dl_data->cch[C_DCCH0].ch_type EQ L2_CHANNEL_FACCH_H) OR
          (dl_data->cch[C_DCCH0].ch_type EQ L2_CHANNEL_FACCH_F))
        channel = C_DCCH3;
    }
    else
    {
      TRACE_EVENT_WIN ("No handling of SACCH with SAPI=0 here!");
      /*
       * The SACCH with SAPI=0 is handled some lines before for DL_2TO1 and
       * simulation. Handling of frames other than UI frames is not supported.
       */
    }
    break;
  default:
    break;
  }/* endswitch channel_type */

  if (channel EQ NOT_PRESENT_8BIT)
  {
    TRACE_EVENT_WIN ("downlink() returns -3");
    return -3;
  }
  else
  {

#if defined(DELAYED_SABM)
    /* ignore downlinked frames before delayed SABM was sent */
    if ((channel EQ C_DCCH0) AND
        (dl_data->dcch0_sabm_flag NEQ NOT_PRESENT_8BIT))
    {
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, channel_type, "DL:pend.SABM->ignore");
      return 0;
    }
#endif  /* DELAYED_SABM */

    cch_i.channel = channel;
    cch_i.sapi = frame_sapi;
#if defined(_SIMULATION_)
    if (channel_type EQ L2_CHANNEL_SACCH)
    {
      TRACE_EVENT_WIN_P1 ("SACCH: set SAPI=%u set during downlink", frame_sapi);
    }
#endif /* _SIMULATION_ */

    cch_i.state = channel_state = dl_data->state[channel];
    cch_i.pcch = &dl_data->cch[channel];
    /*
     * The channel type pcch->ch_type is overwritten as input to the downlink
     * sub functions. In case of DCCH0 this is temporary only and the channel
     * type have to be re-assigned with the value of dl_data->dcch0_ch_type.
     */
    cch_i.pcch->ch_type = channel_type;
    if (cch_i.pcch->vtx NEQ EMPTY_CMD)
    { /* save bit for the case of unsolicited frames */
      cch_i.pf_bit_flag = cch_i.pcch->f_bit;
    }
    TRACE_EVENT_WIN_P4 ("downlink()  in:%s SAPI=%u st=%u vtx=%s",
      CH_TYPE_NAME[channel_type], frame_sapi, channel_state,
      VTX_NAME[cch_i.pcch->vtx]);
    TRACE_EVENT_WIN_P9 ("vr=%u vs=%u va=%u rc=%u contres=%u reje=%u ackp=%u %c=%u",
      cch_i.pcch->vr, cch_i.pcch->vs, cch_i.pcch->va, cch_i.pcch->rc,
      cch_i.pcch->contention_resolution, cch_i.pcch->reject_exception,
      cch_i.pcch->acknowledge_pending,
      cch_i.pcch->time_flag ? 'T' : 't', cch_i.pcch->T200_counter);
  }

  switch (channel_state)
  {
    case STATE_DISABLED:
    case STATE_IDLE_DL:
    ret = downlink_idle (&cch_i, frame+l2_offset);
    break;
  case STATE_CONTENTION_RESOLUTION:
    ret = downlink_contention_resolution (&cch_i, frame+l2_offset);
    break;
  case STATE_MULTIPLE_FRAME_ESTABLISHED:
    ret = downlink_mfe (&cch_i, frame+l2_offset);
    break;
    case STATE_TIMER_RECOVERY:
    ret = downlink_timer_recovery (&cch_i, frame+l2_offset);
    break;
  case STATE_AWAITING_RELEASE:
    ret = downlink_awaiting_release (&cch_i, frame+l2_offset);
    break;
  }/* endswitch channel_state */
  if (channel EQ C_DCCH0)
  { /*
     * Reconstruct the temporary overwritten pcch->ch_type with the value
     * of dl_data->dcch0_ch_type.
     */
    cch_i.pcch->ch_type = dl_data->dcch0_ch_type;
  }


  TRACE_EVENT_WIN_P5 ("%s SAPI=%u vtx=%s (%s#%u)", CH_TYPE_NAME[cch_i.pcch->ch_type], cch_i.sapi,
                                            VTX_NAME[cch_i.pcch->vtx],
                                            __FILE10__, __LINE__);
  if (ret NEQ 0)
  {
    TRACE_EVENT_WIN_P1 ("downlink() returns %d", ret);
    return ret;
  }

  /* transfer states and flags to dl_data */
  dl_data->cch[channel].f_bit_flag = dl_data->cch[channel].f_bit = cch_i.pf_bit_flag;
  if (cch_i.T200_Start)
  {
    dl_data->cch[channel].T200_counter = T200_STOPPED;
    dl_data->cch[channel].time_flag = TRUE;
  }
  else if (cch_i.T200_Stop)
  {
    dl_data->cch[channel].T200_counter = T200_STOPPED;
    dl_data->cch[channel].time_flag = FALSE;
  }

  if (cch_i.dl_data_ind)
  {
    com_data_ind(channel_type, frame_sapi, fn);
  }

  if (channel_state NEQ cch_i.state)
  {
    set_channel_state (
      (UBYTE)((frame_sapi EQ PS_SAPI_0) ? C_DCCH0 : C_DCCH3), cch_i.state);
  }

  if (cch_i.mdl_error_ind)
  {
    mdl_error_ind ( cch_i.mdl_error_ind, channel_type, frame_sapi);
  }


  TRACE_EVENT_WIN_P4 ("downlink() out:%s SAPI=%u st=%u vtx=%s",
    CH_TYPE_NAME[channel_type], cch_i.sapi, cch_i.state,
    VTX_NAME[cch_i.pcch->vtx]);
  TRACE_EVENT_WIN_P9 ("vr=%u vs=%u va=%u rc=%u contres=%u reje=%u ackp=%u %c=%u",
    cch_i.pcch->vr, cch_i.pcch->vs, cch_i.pcch->va, cch_i.pcch->rc,
    cch_i.pcch->contention_resolution, cch_i.pcch->reject_exception,
    cch_i.pcch->acknowledge_pending,
    cch_i.pcch->time_flag ? 'T' : 't', cch_i.pcch->T200_counter);
  TRACE_EVENT_WIN_P4 ("T200=%s  %s  %s  %s",
    cch_i.T200_Start ? "Start" : cch_i.T200_Stop ? "Stop" : "...",
    cch_i.pf_bit_flag ? "P/F" : "",
    cch_i.dl_data_ind ? "DATA_IND" : "", cch_i.mdl_error_ind ? "ERROR_IND" : "");
  TRACE_EVENT_WIN_P1 ("downlink() returns %d", ret);

  return ret;
}/* endfunc downlink */

LOCAL int frame_validation (UBYTE channel_type, UBYTE* frame)
{
  UBYTE frame_length;
  BOOL  frame_m_bit;
  UBYTE N201;

  if (!GET_EA (frame))
    return FRAME_NOT_IMPLEMENTED; /* ETSI GSM 04.06 Annex G.2.3 */

  if (!GET_EL (frame))
    return FRAME_NOT_IMPLEMENTED; /* ETSI GSM 04.06 Annex G.4.1 */

  frame_length = GET_LENGTH_INDICATOR (frame);
  frame_m_bit = GET_M_BIT (frame);

  /* get the maximal number of octets */
  switch (channel_type)
  {
  case L2_CHANNEL_SDCCH:
    N201 = N201_SDCCH;
    break;
  case L2_CHANNEL_SACCH:
    N201 = N201_SACCH;
    break;
  default:/* CH_TYPE_FACCH_FR, L2_CHANNEL_FACCH_H */
    N201 = N201_FACCH;
    break;
  }

  switch (GET_FORMAT_TYPE (frame))
  {
  case I_FORMAT: /* I format */
  case I1_FORMAT:
    if ((frame_length > N201) OR (frame_length EQ 0))
      return I_FRAME_WITH_INCORRECT_LENGTH; /* ETSI GSM 04.06 Annex G.4.2 */
    if ((frame_length < N201) AND (frame_m_bit EQ 1))
      return I_FRAME_WITH_INCORRECT_USE_OF_M_BIT; /* ETSI GSM 04.06 Annex G.4.2 */
    break;
  case S_FORMAT: /* S format */
    if (frame_length OR frame_m_bit)
      return S_FRAME_WITH_INCORRECT_PARAMETERS; /* ETSI GSM 04.06 Annex G.4.3 */
    if ((frame[1] & 0x0f) EQ 0x0d)
      return FRAME_NOT_IMPLEMENTED; /* ETSI GSM 04.06 Annex G.3.1 */
    break;
  case U_FORMAT: /* U format */
    switch (GET_U_TYPE (frame))
    {
    case DM_FRAME:
    case DISC_FRAME:
      if (frame_length OR frame_m_bit)
        return U_FRAME_WITH_INCORRECT_PARAMETERS; /* ETSI GSM 04.06 Annex G.4.4 */
      break;
    case UA_FRAME:
    case SABM_FRAME:
    case UI_FRAME:
      if ((frame_length > N201) OR (frame_m_bit))
        return U_FRAME_WITH_INCORRECT_PARAMETERS; /* ETSI GSM 04.06 Annex G.4.5 */
      break;
    default:
      return FRAME_NOT_IMPLEMENTED; /* ETSI GSM 04.06 Annex G.3.2 */
      /*break;*/
    }/* endswitch U frame_type */
    break;
  }/* endswitch frame_format */

  /*
   * ETSI GSM 04.06 Annex G.2.1, G.2.2 will be check in the following functions
   */

  return -1; /* frame is valid */
}/* endfunc frame_validation */

static  int downlink_idle( T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  /*
   * According to 3GPP TS 04.05, 5.4.5 Idle state:
   * While in the idle state:
   * - the receipt of a DISC command shall result in the transmission of a
   *   DM response with the F bit set to the value of the received P bit;
   * - the receipt of an I frame or supervisory frame with the P bit set to "1"
   *   shall result in the transmission of a DM response with the F bit set to
   *   "1" (as defined in subclause 5.2.2);
   * - the content of any received I frame shall be discarded;
   * - on receipt of an SABM command, the procedures defined in subclause 5.4.1
   *   shall be followed;
   * - on receipt of UI commands, the procedures defined in subclause 5.3 shall
   *   be followed;
   * - all other frame types shall be discarded.
  */
  T_CCH*  pcch;

  CHECK_PCCH_Ir();
  pcch = pcch_i->pcch;

  TRACE_FUNCTION ("downlink_idle()");

  switch (GET_FORMAT_TYPE (frame))
  {
  case S_FORMAT: /* S frame */
    if (GET_S_TYPE (frame) NEQ RR_CMD)
      break;
    /*lint -fallthrough*/
  case I_FORMAT: /* I frame */
  case I1_FORMAT:
    if (GET_P_BIT (frame))
    {
      pcch->vtx = DM_CMD;
      pcch_i->pf_bit_flag = TRUE;
    }
    break;
  case U_FORMAT: /* U frame */
    if (GET_CR (frame))
    { /* command */
      switch (GET_U_TYPE (frame))
      {
      case SABM_FRAME:
        if (!GET_LENGTH_INDICATOR (frame) AND pcch_i->sapi EQ PS_SAPI_3)
        { /*
           * Mobile Terminated Establishment, but only for SAPI=3!
           *
           * According to 3GPP TS 04.06, 5.4.1 Establishment of multiple frame
           * operation, 5.4.1.1 General, Note:
           * For SAPI 0 the data link is always established by the MS.
           */
          com_restore_queue ( pcch_i->sapi, NULL);
          pcch->vtx = UA_CMD;
          pcch_i->pf_bit_flag = GET_P_BIT (frame);
          pcch->va    = 0;
          pcch->vr    = 0;
          pcch->vs    = 0;
          pcch->rc    = 0;
        }
        break;
      case DISC_FRAME:
        pcch->vtx   = DM_CMD;
        pcch_i->pf_bit_flag = GET_P_BIT (frame);
        break;
      case UI_FRAME:
        /* drr_dl_unitdata_ind() was called in the main downlink function */
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }

  return 0;
}/* endfunc downlink_idle */

static  int downlink_contention_resolution(T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  GET_INSTANCE_DATA;
  UBYTE frame_type;
  UBYTE frame_format;
  UBYTE frame_cr;

  UBYTE establish_cnf = FALSE;
  UBYTE release_ind = FALSE;
  UBYTE release_ind_cs = NOT_PRESENT_8BIT;

  T_CCH*  pcch;

  CHECK_PCCH_Ir();
  pcch = pcch_i->pcch;

  TRACE_FUNCTION ("downlink_contention_resolution()");

  frame_format = GET_FORMAT_TYPE (frame);
  if (frame_format EQ U_FORMAT)
  { /* U frame */
    frame_cr = GET_CR (frame);
    frame_type = GET_U_TYPE (frame);
    if (frame_cr)
    { /* command */
      switch (frame_type)
      {
      case SABM_FRAME:
        if (pcch_i->sapi EQ PS_SAPI_3)
        { /* DCCH3 */
          /*
           * According to 3GPP TS 04.06, 5.4.6.1:
           * SAPI = 3, Collision of unnumbered commands and responses.
           * Collision situations (Identical transmitted and received commands)
           * shall be resolved in the following way:  If the transmitted and
           * received unnumbered commands (SABM or DISC) are the same, the data
           * link layer entities shall send the UA response at the earliest
           * possible opportunity. The indicated state shall be entered after
           * receiving the UA response. The data link layer entities shall each
           * notify its respective layer 3 entity by means of the appropriate
           * confirm primitive, i.e. DL-ESTABLISH-CONFIRM or DL-RELEASE-CONFIRM.
           */
          if (!GET_LENGTH_INDICATOR (frame))
          {
            com_clear_queue (PS_SAPI_3);
            /* establish_cnf = TRUE; cnf will be sent at uplink opportunity */

            pcch_i->pf_bit_flag = GET_P_BIT (frame);
            pcch->vtx = UA_CMD;
            pcch->va    = 0;
            pcch->vr    = 0;
            pcch->vs    = 0;
            pcch->rc    = 0;
            pcch_i->T200_Stop = TRUE;
          }
          else
          {
            /* no contention resolution procedure with SAPI=3! */
          }
        }
        else
        {/* DCCH0 */
          /*
           * According to 3GPP TS 04.06, 5.4.1.1 General:
           * NOTE: SAPI=0 the data link is always established by the MS!
           *
           * According to 3GPP TS 04.06, 5.4.1.4 Contention resolution
           * establishment procedure:
           * All frames other than unnumbered frame formats received for the
           * SAPI in use during the establishment procedures shall be ignored.
           * The reception of unnumbered frames other than UA is treated as
           * specified for the normal establishment case.
           * NOTE 4: In fact, there are no foreseen cases in which the network
           *         will send SABM, DISC or DM, but for sake of completeness
           *         these occurrences are specified and must be treated.
           */
          establish_cnf = TRUE;/* Treated as normal establishment case */
          pcch_i->pf_bit_flag = GET_P_BIT (frame);
          pcch->vtx = UA_CMD;
          pcch->va    = 0;
          pcch->vr    = 0;
          pcch->vs    = 0;
          pcch->rc    = 0;
          pcch_i->T200_Stop = TRUE;
        }
        break;

      case DISC_FRAME:
        {
          release_ind = TRUE;

          pcch_i->pf_bit_flag = GET_P_BIT (frame);
          pcch->vtx   = DM_CMD;
        }
        break;

      default:
        break;
      }/* endswitch command frame_type */
    }
    else
    { /* response */
      switch (frame_type)
      {
      case DM_FRAME:
        /*
         * PATCH LE 14.09.99
         * Ignore DM(F=0) frames
         */
        if (GET_P_BIT (frame))
        {
          release_ind = TRUE;
        }
        break;

      case UA_FRAME:
        if (pcch_i->sapi EQ PS_SAPI_0)
        {
          if (pcch->contention_resolution)
          {
            if (com_compare_L3_msg (dl_data->dcch0_queue.switch_buffer, frame))
            {
              establish_cnf = TRUE;
              COM_FREE_QUEUE_BUFFER(&dl_data->dcch0_queue, INDEX_SWITCH_BUFFER);
            }
            else
            {
              release_ind = TRUE;
              release_ind_cs = CAUSE_DL_INFO_FIELD_MISMATCH;
            }
          }
          else
          {
            if (!GET_LENGTH_INDICATOR (frame))
            {
              establish_cnf = TRUE;
            }
            else
            {
              release_ind = TRUE;
            }
          }
        }/* endif PS_SAPI_0 */
        else if (pcch_i->sapi EQ PS_SAPI_3)
        {
          if (!GET_LENGTH_INDICATOR (frame) OR (pcch->ch_type EQ L2_CHANNEL_SACCH))
          {
            establish_cnf = TRUE;
          }
          else
          {
            release_ind = TRUE;
          }

        }/* endif PS_SAPI_3 */

        if (establish_cnf AND (pcch_i->sapi EQ PS_SAPI_0))
        {
          dcch3_enable(pcch->ch_type);
        }

        break;/* endbreak UA_FRAME */

      default:
        break;
      }/* endswitch response frame_type */
    }/* endifelse command/response */

    if (establish_cnf)
    {
      drr_dl_establish_cnf (pcch->ch_type, pcch_i->sapi);
      pcch->va = 0;
      pcch->vr = 0;
      pcch->vs = 0;
      pcch_i->T200_Stop = TRUE;
      pcch_i->state = STATE_MULTIPLE_FRAME_ESTABLISHED;
    }
    else if (release_ind)
    {
      if (pcch_i->sapi EQ PS_SAPI_0)
      {
        if (pcch->contention_resolution)
        {
          pcch->contention_resolution = FALSE;
          COM_FREE_QUEUE_BUFFER(&dl_data->dcch0_queue, INDEX_SWITCH_BUFFER);
        }
      }

      drr_dl_release_ind (pcch->ch_type, pcch_i->sapi, release_ind_cs, FALSE);
      pcch_i->T200_Stop = TRUE;
      pcch_i->state = STATE_IDLE_DL;
    }

  }/* endif frame_format == 3 (only unnumbered frames) */

  return 0;
}/* endfunc downlink_contention_resolution */

static  int downlink_mfe(T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  TRACE_EVENT_WIN ("downlink_mfe()");

  switch (GET_FORMAT_TYPE (frame))
  {
  case I_FORMAT:/* I frame */
  case I1_FORMAT:
    downlink_mfe_information (pcch_i, frame);
    break;
  case S_FORMAT: /* S frame */
    downlink_mfe_supervisory (pcch_i, frame);
    break;
  case U_FORMAT: /* U frame */
    downlink_mfe_tr_unnumbered (pcch_i, frame,
                                STATE_MULTIPLE_FRAME_ESTABLISHED);
    break;
  default:
    TRACE_EVENT_WIN ("invalid/unknown frame");
    break;
  }

  return 0;
}/* endfunc downlink_mfe */

static  void downlink_mfe_supervisory (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  UBYTE frame_cr;
  UBYTE frame_pollbit;
  UBYTE frame_nr;
  UBYTE frame_type;
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("downlink_mfe_supervisory()");

  frame_type = GET_S_TYPE (frame);

  /*
   * Ignore RNR frame without notification
   * (ETSI GSM 04.06, section 6.
   * "Special protocol operation on SAPI=0 and SAPI=3", page 53)
   */
  if (frame_type EQ RNR_FRAME)
    return;

  frame_cr = GET_CR (frame);
  frame_pollbit = GET_P_BIT (frame);
  frame_nr = GET_RECEIVE_NUMBER (frame);

  /* in accordance with CCITT Q.921 figure B.7 (sheet 5 to 7 of 10) */
  if (frame_pollbit)
  {
    if (frame_cr)
    {
      enquiry_response (pcch_i, frame);
    }
    else
    {
      mdl_error_ind (UNSOLICITED_SUPERVISORY_RESPONSE, pcch->ch_type, pcch_i->sapi);/* 3GPP TS 04.06, 5.4.2.2 */
    }
  }

  if (com_check_nr (pcch->va, pcch->vs, frame_nr))
  {
    /*
     * N(R) check is successfull
     * in accordance with CCITT Q.921 figure B.7 (sheet 6 and 7 of 10)
     */

    switch (frame_type)
    {
    case RR_FRAME:
      if (frame_nr EQ pcch->vs)
      {
        /* T200 handling under ETSI GSM 04.06 section 5.5.3.1 */
        if (((8 + frame_nr - pcch->va ) & 7) > 0 /*frame_nr > pcch->va*/)
          pcch_i->T200_Stop = TRUE;

        pcch->va = frame_nr;

        free_sending_buffer (pcch->ch_type, pcch_i->sapi);
      }
      else
      {
        if (frame_nr NEQ pcch->va)
        {
          pcch->va = frame_nr;
          pcch_i->T200_Start = TRUE;
        }
      }
      break;
    case REJ_FRAME:
      /*
       * in accordance with ETSI GSM 04.06; chapter 5.5.4.1 i)
       *
       * clear existing peer receiver busy condition (not applicable in GSM)
       */

      /* reset timer T200 */
      pcch_i->T200_Stop = TRUE;
      /*
       * If REJ command with P bit set to 1,
       * transmit an appropiate supervisory response frame with F bit set to 1
       */
      if (frame_pollbit AND frame_cr)
        enquiry_response (pcch_i, frame);
      /* transmit the corresponding I frame asap */
      invoke_retransmission (pcch_i, frame_nr);


      /*
       * set its send state variable V(S) and its acknowledge state
       * variable V(A) to the value of the N(R) contained in the REJ frame
       * control field
       */
      pcch->vs = pcch->va = frame_nr;

      /*
       * if it was an REJ response frame with the F bit set to 1, notify
       * a protocol violation to layer 3 (cause=unsolicited supervisory frame)
       *
       * fulfilled at the beginning of this function!
       */
      break;
    case RNR_FRAME:
      /*
       * Ignore frame without notification
       * (ETSI GSM 04.06, section 6.
       * "Special protocol operation on SAPI=0 and SAPI=3", page 53)
       */
      break;
    default:
      TRACE_EVENT_WIN ("invalid S frame");  /* GSM 04.06 Annex G.3.1 */
      mdl_error_ind (FRAME_NOT_IMPLEMENTED, pcch->ch_type, pcch_i->sapi);
      break;
    }

  }
  else
  {
    nr_error_recovery (pcch_i, frame);
  }
}/* endfunc downlink_mfe_supervisory */


static  void downlink_mfe_sabm (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  TRACE_EVENT_WIN ("downlink_mfe_sabm()");

  if (!GET_LENGTH_INDICATOR (frame))
  {
    T_CCH*  pcch;

    CHECK_PCCH_I();
    pcch = pcch_i->pcch;

    if (pcch_i->sapi EQ PS_SAPI_0)
    {
      /* SACCH0: only unacknowledge mode available -> ignore SABM */
      if (pcch->ch_type EQ L2_CHANNEL_SACCH)
        return;
      /*
       * SDCCH0, FACCH: can not be a normal establishment procedure
       * because for SAPI=0 the data link is always established by the MS.
       * Therefore only the V state variables and any exception states
       * will be reseted.
       */
    }
    else if (pcch_i->sapi EQ PS_SAPI_3)
    { /*
       * SDCCH3, SACCH: normal establishment procedure,
       * might be a re-establishment according to GSM 04.06, 5.4.1.2
       */
      com_clear_queue (PS_SAPI_3);
    }

    /* respond with an unnumbered acknowledgement */
    pcch->vtx = UA_CMD;
    /* with the F bit set to the same value as the P bit */
    pcch_i->pf_bit_flag = GET_P_BIT (frame);
    /* reset timer T200 */
    pcch_i->T200_Stop = TRUE;
    /* reset all state variables (internal sequence counter) */
    pcch->va = 0;
    pcch->vr = 0;
    pcch->vs = 0;
    /* reset the retransmission counter */
    pcch->rc = 0;
    /* clear all exception conditions */
    pcch->reject_exception = FALSE;
    pcch->acknowledge_pending = FALSE;
    /* enter the multiple-frame-established state */
    pcch_i->state = STATE_MULTIPLE_FRAME_ESTABLISHED;
  }
  else
  {
    /*
     * frame_length NEQ 0 is only possible in contention resolution establishment
     * initiated by mobile!
     */
     mdl_error_ind (U_FRAME_WITH_INCORRECT_PARAMETERS, pcch_i->pcch->ch_type, pcch_i->sapi);/* according to GSM 04.06, 5.4.2.1 */
  }
}/* endfunc mfe_sabm */

static  void downlink_mfe_dm (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  if (!GET_P_BIT (frame))
  { /* release after unsolicited DM response during connection */

    /* New! Called now by mdl_error_ind() from caller of this function
     * drr_dl_release_ind (dl_data, pcch->ch_type, pcch_i->sapi, NOT_PRESENT_8BIT);
     */

    if ((pcch->ch_type EQ L2_CHANNEL_SDCCH) AND (pcch_i->sapi EQ PS_SAPI_3))
    {
      pcch_i->T200_Stop = TRUE;
      com_clear_queue (PS_SAPI_3);
    }
  }
}/* endfunc mfe_dm */

static  void downlink_mfe_tr_unnumbered (T_CCH_INTERN* pcch_i,
                                         UBYTE *frame, UBYTE state)
{
  UBYTE frame_type;
  T_CCH*  pcch;

  TRACE_EVENT_WIN ("downlink_mfe_tr_unnumbered()");

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  frame_type = GET_U_TYPE (frame);
  if (GET_CR (frame))
  { /* command */
    switch (frame_type)
    {
    case SABM_FRAME:
      downlink_mfe_sabm (pcch_i, frame);
      break;
    case UI_FRAME:
      /* drr_dl_unitdata_ind() was called in the main downlink function */
      break;
    case DISC_FRAME:
      pcch->vtx = UA_CMD;
      pcch_i->T200_Stop = TRUE;
      pcch_i->state = STATE_AWAITING_RELEASE;
#if !defined(LATE_LEAVING_DEDICATED)
      com_leave_dedicated (pcch->ch_type);
#endif /* LATE_LEAVING_DEDICATED */
      if (pcch_i->sapi EQ PS_SAPI_0)
      {
        dcch3_enable(pcch->ch_type);
      }
      break;
    default:
      /* GSM 04.06 Annex G.2.2, G.3.2 */
      TRACE_EVENT_WIN_P1 ("invalid command U frame (%02x)", frame_type);
      break;
    }
  }
  else
  { /* response */
    switch (frame_type)
    {
    case DM_FRAME:
      /* fulfill the actions required by 3GPP TS 4.06 section 5.4.2.2, table 7 */
      if (!GET_P_BIT(frame))
      {
        mdl_error_ind (UNSOLICITED_DM_RESPONSE_ABNORMAL_REL, pcch->ch_type, pcch_i->sapi);
      }
      else if (state EQ STATE_MULTIPLE_FRAME_ESTABLISHED)
      {
        mdl_error_ind (UNSOLICITED_DM_RESPONSE, pcch->ch_type, pcch_i->sapi);
      }

      downlink_mfe_dm (pcch_i, frame);
      break;
    case UA_FRAME:
      mdl_error_ind (UNSOLICITED_UA_RESPONSE, pcch->ch_type, pcch_i->sapi);
      break;
    default:
      /* GSM 04.06 Annex G.2.2, G.3.2 */
      TRACE_EVENT_WIN_P1 ("invalid response U frame (%02x)", frame_type);
      break;
    }
  }
}/* endfunc downlink_mfe_unnumbered */

static  void downlink_i_frame (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  /*
   * in accordance with CCITT Q.921 figure B.7 (sheet 8 of 10)
   * in accordance with CCITT Q.921 figure B.8 (sheet 7 of 9)
   * according to GSM 04.06 (same as CCITT Q.921!)
   */

  UBYTE frame_pollbit;
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  frame_pollbit = GET_P_BIT (frame);

  if (GET_SEND_NUMBER (frame) EQ pcch->vr)
  {
    pcch->vr++;
    pcch->vr &= 7;
    pcch->reject_exception = FALSE;
    concatenate (pcch->ch_type, pcch_i->sapi, frame);
    if (!GET_M_BIT (frame))
      pcch_i->dl_data_ind = TRUE;/* send DL-DATA indication */

    if (frame_pollbit)
    {
      pcch_i->pf_bit_flag = TRUE;
      pcch->vtx = RR_RSP;
      pcch->acknowledge_pending = FALSE;
    }
    else if (pcch->acknowledge_pending EQ FALSE)
    {
#if defined(IFRAME_AS_RR)
      pcch->vtx = RR_CMD;
#else
      pcch->vtx = RR_RSP;
#endif  /* IFRAME_AS_RR */
    }
  }/* endif ns == vr */
  else
  {
    if (pcch->reject_exception)
    {
      if (frame_pollbit)
      {
        pcch_i->pf_bit_flag = TRUE;
        /*pcch->vtx = RR_RSP; */
        pcch->vtx = REJ_CMD;
        pcch->acknowledge_pending = FALSE;
      }
    }
    else
    {
      pcch->reject_exception = TRUE;
      pcch_i->pf_bit_flag = frame_pollbit;
      pcch->vtx = REJ_CMD;
      pcch->acknowledge_pending = FALSE;
    }
    if (pcch->vtx EQ REJ_CMD)
    {
      TRACE_EVENT_WIN_P1 ("->REJ_CMD pf=%u", pcch_i->pf_bit_flag);
    }
  }/* endelse ns != vr */
}/* endfunc downlink_i_frame */

static  void downlink_mfe_information ( T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  UBYTE frame_nr;
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("downlink_mfe_information()");

  if (!GET_CR (frame))
  { /* GSM 04.06 Annex G.2.2 */
    TRACE_EVENT_WIN ("invalid I response (C=0)");
    return;
  }

  frame_nr = GET_RECEIVE_NUMBER (frame);

  /* in accordance with CCITT Q.921 figure B.7 (sheet 8 of 10) */
  downlink_i_frame (pcch_i, frame);

  /* in accordance with CCITT Q.921 figure B.7 (sheet 9 of 10) */
  if (com_check_nr (pcch->va, pcch->vs, frame_nr))
  { /* N(R) check is successfull */
    if (frame_nr EQ pcch->vs)
    {

      /* T200 handling under ETSI GSM 04.06 section 5.5.3.1 */
      if (((8 + frame_nr - pcch->va ) & 7) > 0 /*frame_nr > pcch->va*/)
        pcch_i->T200_Stop = TRUE;

      pcch->va = frame_nr;
      free_sending_buffer (pcch->ch_type, pcch_i->sapi);
    }
    else if (frame_nr NEQ pcch->va)
    {
      pcch->va = frame_nr;
      pcch_i->T200_Start = TRUE;
    }
  }
  else
  {
    nr_error_recovery (pcch_i, frame);
  }/* endifelse com_check_nr */
}/* endfunc downlink_mfe_information */

static  int downlink_timer_recovery(T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  TRACE_EVENT_WIN ("downlink_timer_recovery()");

  switch (GET_FORMAT_TYPE (frame))
  {
  case I_FORMAT:
  case I1_FORMAT: /* I frame */
    downlink_tr_information (pcch_i, frame);
    break;
  case S_FORMAT: /* S frame */
    downlink_tr_supervisory (pcch_i, frame);
    break;
  case U_FORMAT: /* U frame */
    downlink_mfe_tr_unnumbered (pcch_i, frame, STATE_TIMER_RECOVERY);
    break;
  default:
    TRACE_EVENT_WIN ("invalid/unknown frame");
    break;
  }

  return 0;
}/* endfunc downlink_tr */

static  void downlink_tr_supervisory (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  UBYTE frame_cr;
  UBYTE frame_pollbit;
  UBYTE frame_nr;
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("downlink_tr_supervisory()");

  frame_cr = GET_CR (frame);
  frame_pollbit = GET_P_BIT (frame);
  frame_nr = GET_RECEIVE_NUMBER (frame);

  switch (GET_S_TYPE (frame))
  {
  case RR_FRAME:
    /*
     * in accordance with CCITT Q.921 figure B.8 (sheet 5 and 6 of 9) and
     * 3GPP 04.06, 5.5.3.1 "On receipt of a valid I frame or supervisory frame"
     * and 5.5.7 "Waiting acknowledgement".
     */
    if (frame_pollbit AND frame_cr)
    {
      enquiry_response (pcch_i, frame);
    }

    if (com_check_nr (pcch->va, pcch->vs, frame_nr))
    {
      /* N(R) check is successfull */

      TRACE_EVENT_WIN_P5 ("V(A)=%d =< N(R)=%d =< V(S)=%d check is successfull, pf=%u cr=%u",
        pcch->va, frame_nr, pcch->vs, frame_pollbit, frame_cr);

      pcch->va = frame_nr;

      if (frame_pollbit AND !frame_cr)
      { /*
         * 3GPP 04.06, 5.5.7:
         * The timer recovery state is only cleared if the DL receives a valid
         * supervisory frame response with the F bit set to 1.
         */
        pcch_i->T200_Stop = TRUE;
        pcch_i->state = STATE_MULTIPLE_FRAME_ESTABLISHED;
        /* acknowledgement according to GSM 04.06, 5.5.3.1*/
        free_sending_buffer (pcch->ch_type, pcch_i->sapi);
      }
      else
      {
        invoke_retransmission (pcch_i, frame_nr);
      }
    }
    else
    {
      nr_error_recovery (pcch_i, frame);
    }/* endifelse com_check_nr */
    break;

  case REJ_FRAME:
#if defined(NEW_REJ_ACK)
    /*
     * in accordance with
     * 3GPP 04.06, 5.5.3.1 "On receipt of a valid I frame or supervisory frame"
     */
    if (com_check_nr (pcch->va, pcch->vs, frame_nr))
    {
      /* N(R) check is successfull */

      TRACE_EVENT_WIN_P5 ("V(A)=%d =< N(R)=%d =< V(S)=%d check is successfull, pf=%u cr=%u",
        pcch->va, frame_nr, pcch->vs, frame_pollbit, frame_cr);

      /* acknowledgement according to GSM 04.06, 5.5.3.1*/
      free_sending_buffer (pcch->ch_type, pcch_i->sapi);

      pcch->va = frame_nr;
    }
#endif  /* NEW_REJ_ACK */

    /*
     * in accordance with GSM 04.06; chapter 5.5.4.1
     * Receipt of a valid REJ frame [ii) and iii)]
     *
     * clear existing peer receiver busy condition (not applicable in GSM)
     */

    if (frame_pollbit AND !frame_cr)
    {/* REJ response with F bit set to 1 */
      /* clear the timer recovery state */
      pcch_i->state = STATE_MULTIPLE_FRAME_ESTABLISHED;

      /* reset timer T200 */
      pcch_i->T200_Stop = TRUE;

#if defined(NEW_REJ_ACK)
      /* transmit the corresponding I frame asap */
      if (pcch->va NEQ frame_nr)
      {
        TRACE_EVENT_WIN_P2 ("REJ: V(A)=%d != N(R)=%d => invoke retransmission",
          pcch->va, frame_nr);

        invoke_retransmission (pcch_i, frame_nr);
      }
#else /* NEW_REJ_ACK */
      /* transmit the corresponding I frame asap */
      invoke_retransmission (pcch_i, frame_nr);
#endif  /* NEW_REJ_ACK */


      /*
       * set its send state variable V(S) and its acknowledge state
       * variable V(A) to the value of the N(R) contained in the REJ frame
       * control field
       */
      pcch->vs = pcch->va = frame_nr;
    }
    else
    {
      /*
       * set its its acknowledge state variable V(A) to the value
       * of the N(R) contained in the REJ frame control field
       */
      pcch->va = frame_nr;

      /*
       * if REJ command with P bit set to 1,
       * transmit an appropiate supervisory response frame with F bit set to 1
       */
      if (frame_pollbit AND frame_cr)
      {
        enquiry_response (pcch_i, frame);
      }
    }
    break;

  case RNR_FRAME:
    /*
     * ignore RNR frame without notification
     * (ETSI GSM 04.06, section 6.
     * "Special protocol operation on SAPI=0 and SAPI=3", page 53)
     */
  default:
    /* frame not implemented, */
    TRACE_EVENT_WIN ("invalid S frame");  /* GSM 04.06 Annex G.3.1 */
    return;
  }/* endswitch frame_type */


}/* endfunc downlink_tr_supervisory */

static  void downlink_tr_information ( T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  T_CCH*  pcch;
  UBYTE   frame_nr;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("downlink_tr_information()");

  if (!GET_CR (frame))
  { /* GSM 04.06 Annex G.2.2 */
    TRACE_EVENT_WIN ("invalid I response (C=0)");
    return;
  }

  /* in accordance with CCITT Q.921 figure B.8 (sheet 7 of 9) */
  downlink_i_frame (pcch_i, frame);

  /*
   * in accordance with CCITT Q.921 figure B.8 (sheet 8 of 9) and
   * 3GPP 04.06, 5.5.3.1 "On receipt of a valid I frame or supervisory frame"
   * and 5.5.7 "Waiting acknowledgement".
   */
  frame_nr = GET_RECEIVE_NUMBER (frame);
  if (com_check_nr (pcch->va, pcch->vs, frame_nr))
    pcch->va = frame_nr;/* N(R) check is successfull */
  else
    nr_error_recovery (pcch_i, frame);
}/* endfunc downlink_tr_information */

static  int downlink_awaiting_release(T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  T_CCH*  pcch;

  CHECK_PCCH_Ir();
  pcch = pcch_i->pcch;

  TRACE_FUNCTION ("downlink_awaiting_release()");

  if (GET_FORMAT_TYPE (frame) EQ U_FORMAT)
  { /* U frame */
    if (GET_CR (frame))
    { /* command */
      switch (GET_U_TYPE (frame))
      {
      case SABM_FRAME:
        if (pcch_i->sapi EQ PS_SAPI_0)
        {
          pcch->vtx = DM_CMD;
          pcch_i->pf_bit_flag = GET_P_BIT (frame);
          pcch_i->T200_Stop = TRUE;
          pcch_i->state = STATE_IDLE_DL;
          drr_dl_release_cnf (pcch->ch_type, PS_SAPI_0, FALSE);
        }
        break;
      case UI_FRAME:
        /* drr_dl_unitdata_ind() was called in the main downlink function */
        break;
      case DISC_FRAME:
        pcch->vtx = UA_CMD;
        pcch_i->T200_Stop = TRUE;
#if !defined(LATE_LEAVING_DEDICATED)
        com_leave_dedicated (pcch->ch_type);
#endif  /* LATE_LEAVING_DEDICATED */
        break;
      default:
        /* GSM 04.06 Annex G.2.2, G.3.2 */
        TRACE_EVENT_WIN_P1 ("invalid command U frame (%02x)", GET_U_TYPE (frame));
        break;
      }
    }
    else
    { /* response */
      switch (GET_U_TYPE (frame))
      {
      case DM_FRAME:
        /*
         * PATCH LE 14.09.99
         * Ignore DM(F=0) frames
         */
        if (!GET_P_BIT (frame))
          break;

        /* the same as UA_FRAME */
        /*lint -fallthrough*/

      case UA_FRAME:
        pcch_i->T200_Stop = TRUE;
#if defined(LATE_LEAVING_DEDICATED)
        com_leave_dedicated (pcch->ch_type);
#endif  /* LATE_LEAVING_DEDICATED */
        pcch_i->state = STATE_IDLE_DL;
        drr_dl_release_cnf (pcch->ch_type, pcch_i->sapi, TRUE);
        break;

      default:
        TRACE_EVENT_WIN_P1 ("invalid response U frame (%02x)", GET_U_TYPE (frame));
        break;
      }
    }
  }
  return 0;
}/* endfunc downlink_awaiting_release */

static  void invoke_retransmission (T_CCH_INTERN* pcch_i, UBYTE frame_nr)
{
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  if (pcch->vs NEQ frame_nr)
  { /* decrement V(S) and recover queue for retransmission */
    TRACE_EVENT_WIN ("invoke retransmission");

    pcch->vs--;
    pcch->vs &= 7;
    com_recover_queue (pcch_i->sapi);
  }
}/* endfunc invoke_retransmission */


static  void enquiry_response (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("enquiry_response()");
  /*
   * in accordance with ETSI GSM 04.06, 5.5.3.2 Receiving supervisory
   * commands with the P bit set to "1" and ETSI GSM 04.06, 5.5.4.1 iii
   */

  pcch_i->pf_bit_flag = TRUE;
  pcch->acknowledge_pending = FALSE;
  pcch->vtx = RR_RSP;
}/* endfunc enquiry_response */

#if defined(_SIMULATION_)
LOCAL const char * const _str_error_ind_cause[] =
{
  "T200_EXPIRED_N200_PLUS_1_TIMES, \"T200 expired (N200 + 1 times)\"",
  "CS_REEST_REQ, \"re-establishment request\"",
  "UNSOLICITED_UA_RESPONSE, \"unsolicited UA response\"",
  "UNSOLICITED_DM_RESPONSE, \"unsolicited DM response\"",
  "UNSOLICITED_DM_RESPONSE_ABNORMAL_REL, \"unsolicited DM response, multiple frame established state\"",
  "UNSOLICITED_SUPERVISORY_RESPONSE, \"unsolicited supervisory response\"",
  "SEQUENCE_ERROR, \"sequence error\"",
  "U_FRAME_WITH_INCORRECT_PARAMETERS, \"U frame with incorrect parameters\"",
  "S_FRAME_WITH_INCORRECT_PARAMETERS, \"S frame with incorrect parameters\"",
  "I_FRAME_WITH_INCORRECT_USE_OF_M_BIT, \"I frame with incorrect use of M bit\"",
  "I_FRAME_WITH_INCORRECT_LENGTH, \"I frame with incorrect length\"",
  "FRAME_NOT_IMPLEMENTED, \"frame not implemented\"",
};
#endif  /* _SIMULATION_ */

static  void mdl_error_ind (UBYTE cause,
                         UBYTE channel_type, UBYTE sapi)
{
  TRACE_EVENT_WIN_P2 ("mdl_error_ind(%u %s)", cause, _str_error_ind_cause[cause]);
  switch(cause)
  {
    case T200_EXPIRED_N200_PLUS_1_TIMES:
    case UNSOLICITED_DM_RESPONSE:
    case UNSOLICITED_DM_RESPONSE_ABNORMAL_REL:
    case SEQUENCE_ERROR:
      drr_error_ind (channel_type, sapi);
      break;
    default:
      break;
  }
}/* endfunc mdl_error_ind */

static  void nr_error_recovery (T_CCH_INTERN* pcch_i, UBYTE *frame)
{
  
  T_CCH*  pcch;

  CHECK_PCCH_I();
  pcch = pcch_i->pcch;

  TRACE_EVENT_WIN ("nr_error_recovery()");
  DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, pcch->ch_type, "N(R) sequence error");

  switch (pcch->ch_type)
  {
  case L2_CHANNEL_SDCCH:
    if (pcch_i->sapi EQ PS_SAPI_3)
      break;
    /*lint -fallthrough*/
  case L2_CHANNEL_SACCH:
  case L2_CHANNEL_FACCH_F:
  case L2_CHANNEL_FACCH_H:
    if ((GET_P_BIT (frame) EQ 1) AND !GET_M_BIT (frame))
      pcch_i->dl_data_ind = TRUE; /* indicate a complete message to layer 3 */
    else
      pcch_i->dl_data_ind = FALSE;/* no indication if P bit set to "0" or message is incomplete */

    /*
     * GSM 04.06, 5.7.4 The data link shall remain in current state
     * until it's release by layer 3 ???
     */
    pcch_i->mdl_error_ind = SEQUENCE_ERROR; /* send mdl error ind after sequence error */

    DL_EM_CHANNEL_FAILURE;

    break;
  }
  pcch->vtx = EMPTY_CMD;/* no answer after N(R) sequence error */
}/* endfunc nr_error_recovery */


static void concatenate (UBYTE ch_type, UBYTE sapi,UBYTE *frame)
{
  GET_INSTANCE_DATA;
  switch (ch_type)
  {
  case L2_CHANNEL_SDCCH:
    if (sapi EQ PS_SAPI_0)
      com_concatenate (&dl_data->dcch0_in_msg, frame);
    else if (sapi EQ PS_SAPI_3)
      com_concatenate (&dl_data->dcch3_in_msg, frame);
    break;
  case L2_CHANNEL_SACCH:
    if (sapi EQ PS_SAPI_3)
    {
      com_concatenate (&dl_data->dcch3_in_msg, frame);
    }
    break;
  case L2_CHANNEL_FACCH_F:
  case L2_CHANNEL_FACCH_H:
    if (sapi EQ PS_SAPI_0)
    {
      com_concatenate (&dl_data->dcch0_in_msg, frame);
    }
    break;
  default:
    break;
  }
}/* endfunc concatenate */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)          MODULE  : DL_COM                  |
| STATE   : code                   ROUTINE : free_sending_buffer     |
+--------------------------------------------------------------------+

  PURPOSE : After confirmation of an I frame the sending buffer is
            released if it was the last segment. This avoids
            resending of I frames after resumption.

*/
static void free_sending_buffer ( UBYTE ch_type, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_QUEUE *queue = NULL;

  TRACE_EVENT_WIN_P2 ("free_sending_buffer():%s SAPI=%u", CH_TYPE_NAME[ch_type], sapi);

  switch (ch_type)
  {
  case L2_CHANNEL_SDCCH:
    if (sapi EQ PS_SAPI_0)
      queue = &dl_data->dcch0_queue;
    else if (sapi EQ PS_SAPI_3)
      queue = &dl_data->dcch3_queue;
    break;
  case L2_CHANNEL_SACCH:
    if (sapi EQ PS_SAPI_3)
      queue = &dl_data->dcch3_queue;
    break;
  case L2_CHANNEL_FACCH_F:
  case L2_CHANNEL_FACCH_H:
    if (sapi EQ PS_SAPI_0)
      queue = &dl_data->dcch0_queue;
    break;
  default:
    break;
  }

  if (queue)
  {
    if (queue->switch_buffer)
    {
      TRACE_EVENT_WIN_P3 ("CNF on %s SAPI=%u %s (switch_buffer)",
        CH_TYPE_NAME[queue->switch_buffer->ch_type],
        queue->switch_buffer->sapi,
        queue->switch_buffer->cnf ? "CNF required" : "");

      com_l3trace (TRACE_UPLINK, queue->switch_buffer->ch_type, (UBYTE *)queue->switch_buffer);
      COM_FREE_QUEUE_BUFFER ( queue, INDEX_SWITCH_BUFFER);
    }
    else if (queue->sending_buffer)
    {
      TRACE_EVENT_WIN_P4 ("CNF on %s SAPI=%u %s (sending_buffer) act_length=%u",
        CH_TYPE_NAME[queue->sending_buffer->ch_type],
        queue->sending_buffer->sapi,
        queue->sending_buffer->cnf ? "CNF required" : "",
        queue->act_length);

      if (queue->act_length EQ 0)
      { /* entire message has been sent */
        com_l3trace (TRACE_UPLINK, queue->sending_buffer->ch_type, (UBYTE *)queue->sending_buffer);

        if (queue->sending_buffer->cnf)
        {
          drr_dl_data_cnf (sapi);
        }
        else
        {
          COM_FREE_QUEUE_BUFFER (queue, INDEX_SENDING_BUFFER);
        }
      }
    }
    else
    {
      TRACE_EVENT_WIN_P2 ("%s SAPI=%u: sending_buffer and switch_buffer=NULL !!!",
        CH_TYPE_NAME[ch_type], sapi);
    }
  }
}/* endfunc concatenate */

static void T200_expiry ( UBYTE channel, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  UBYTE   old_state;
  UBYTE   new_state;
  T_CCH * pcch = &dl_data->cch[channel];
  UBYTE   N200_counter;

  TRACE_FUNCTION ("T200_expiry()");

  switch (pcch->ch_type)
  {
    case L2_CHANNEL_SACCH:
      N200_counter = SACCH_N200;
      break;
    case L2_CHANNEL_SDCCH:
      N200_counter = SDCCH_N200;
      break;
    case L2_CHANNEL_FACCH_F:
      N200_counter = FACCH_N200_FR;
      break;
    case L2_CHANNEL_FACCH_H:
      N200_counter = FACCH_N200_HR;
      break;
    default:
      N200_counter = NOT_PRESENT_8BIT;
      break;
  }

  new_state = old_state = dl_data->state[channel];
  switch (old_state)
  {
    case STATE_CONTENTION_RESOLUTION:
      repeat_sabm (channel, sapi);
      break;

    case STATE_MULTIPLE_FRAME_ESTABLISHED:
      pcch->rc        = 1;
      pcch->p_bit_flag= 1;
      pcch->time_flag = TRUE;
      new_state   = STATE_TIMER_RECOVERY;
      break;

    case STATE_TIMER_RECOVERY:
      if (N200_counter EQ NOT_PRESENT_8BIT)
        break; /* invalid channel */

      if (pcch->rc >= N200_counter)
      { /* release connection due to T200 expired N200 plus 1 times */

        TRACE_EVENT_WIN_P1 ("T200 expired, N200=%u", N200_counter);

        /* New! Called now by mdl_error_ind().
         * drr_dl_release_ind (dl_data, pcch->ch_type, sapi, NOT_PRESENT_8BIT);
         */
        mdl_error_ind (T200_EXPIRED_N200_PLUS_1_TIMES, pcch->ch_type, sapi);

        if (sapi EQ PS_SAPI_0)
        {
          dcch3_init_dl_data();
          pcch->T200_counter      = 0;
          pcch->time_flag             = FALSE;
          pcch->contention_resolution = FALSE;
          pcch->vtx                   = EMPTY_CMD;
          new_state         = STATE_IDLE_DL;
        }
      }
      else
      {
        pcch->rc++;
        pcch->p_bit_flag  = 1;
        pcch->time_flag   = TRUE;
      }
      break;

    case STATE_AWAITING_RELEASE:
      if (pcch->rc >= N200_ESTABLISHMENT)
      {
        TRACE_EVENT_WIN_P1 ("T200 expired, N200=%u", N200_ESTABLISHMENT);
        drr_dl_release_cnf (pcch->ch_type, sapi, TRUE);
        new_state = STATE_IDLE_DL;

      }
      else
      {
        pcch->rc++;
        pcch->vtx   = DISC_CMD;
        pcch->time_flag = TRUE;
      }
      break;

    default:
      break;
  }/* endswitch old_state */

  if (new_state NEQ old_state)
  {
    set_channel_state (channel, new_state);
  }

}/* endfunc T200_expiry */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : state                      |
| STATE   : code                ROUTINE : repeat_sabm                |
+--------------------------------------------------------------------+

  PURPOSE : Repeat if possible the SABM command.

*/

static void repeat_sabm (UBYTE channel, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_CCH*  pcch = &dl_data->cch[channel];

  TRACE_FUNCTION ("repeat_sabm()");

  if (pcch->rc >= N200_ESTABLISHMENT)
  {
    if (sapi EQ PS_SAPI_0)
      pcch->contention_resolution = FALSE;

    DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, ">N200_EST");
    SYST_TRACE_P ((SYST, "DL: >N200 of SABM"));
    TRACE_ERROR ("DL: >N200 of SABM");

    DL_EM_CHANNEL_ESTABLISHMENT_FAILED;

    mdl_error_ind(T200_EXPIRED_N200_PLUS_1_TIMES, pcch->ch_type, sapi);
    set_channel_state (channel, STATE_IDLE_DL);
    /* New! Called now by mdl_error_ind().
     * drr_dl_release_ind (dl_data, pcch->ch_type, sapi, NOT_PRESENT_8BIT);
     */
  }
  else
  {
    pcch->rc++;
    pcch->vtx       = SABM_CMD;
    pcch->time_flag = TRUE;
    TRACE_EVENT_P1 ("DL: T200 %u. repeat of SABM", pcch->rc);
  }
}/* endfunc repeat_sabm */

GLOBAL  void set_channel_state (UBYTE channel, UBYTE state)
{
  GET_INSTANCE_DATA;
  if (dl_data->state[channel] NEQ state)
  {
    #ifdef TRACE_STATE
      vsi_o_state_ttrace ("STATE_%s:%s -> %s", PROCESS_NAME[channel],
        channel EQ C_DCCH3 ? STATE_DCCH3_NAME[dl_data->state[channel]]:
                             STATE_DCCH0_NAME[dl_data->state[channel]],
        channel EQ C_DCCH3 ? STATE_DCCH3_NAME[state] :
                             STATE_DCCH0_NAME[state]);
    #endif /* TRACE_STATE */
    dl_data->state[channel] = state;
    DL_OFFLINE_TRACE (TRACE_CHSTATE, channel, dl_data->cch[channel].ch_type, NULL);
  }
}/* endfunc set_channel_state */

static void set_T200_counter (T_CCH* pcch, UBYTE sapi)
{
  switch (pcch->ch_type)
  {
    case L2_CHANNEL_SACCH:
      pcch->T200_counter = sapi EQ PS_SAPI_0 ?
        T200_SDCCH_SAPI_0_CNT : T200_SACCH_SAPI_3_CNT;
      break;
    default:
      TRACE_EVENT_WIN_P1 ("set_T200_counter: unknown ch_type=%u -> use SDCCH", pcch->ch_type);
      /*lint -fallthrough*/
    case L2_CHANNEL_SDCCH:
      pcch->T200_counter = sapi EQ PS_SAPI_0 ?
        T200_SDCCH_SAPI_0_CNT : T200_SDCCH_SAPI_3_CNT;
      break;
    case L2_CHANNEL_FACCH_F:
      pcch->T200_counter = T200_FACCH_SAPI_0_CNT_FR;
      break;
    case L2_CHANNEL_FACCH_H:
      pcch->T200_counter = T200_FACCH_SAPI_0_CNT_HR;
      break;
  }/* endswitch ch_type */
  pcch->time_flag = FALSE;

  TRACE_EVENT_WIN_P3 ("set_T200_counter: %s, SAPI=%u: %u",
    CH_TYPE_NAME[pcch->ch_type], sapi, pcch->T200_counter);
}/* endfunc set_T200_counter */

#if defined(DELAYED_RELEASE_IND)
static void delayed_release_ind( UBYTE channel)
{
  GET_INSTANCE_DATA;
  /* delay DL RELEASE IND to RR for FTA 25.2.3 *********** */
  if (dl_data->release_ind_ch_type NEQ NOT_PRESENT_8BIT)
  {
    if (dl_data->release_ind_delay > 0)
      dl_data->release_ind_delay--;

    if (dl_data->release_ind_delay EQ 0)
    {
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN,
        dl_data->release_ind_ch_type, "UL:send delayed REL IND");
      drr_dl_release_ind (dl_data->release_ind_ch_type,
        dl_data->release_ind_sapi, NOT_PRESENT_8BIT, TRUE);
    }
    else
    {
      DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN,
        dl_data->release_ind_ch_type, "UL:pend. delay REL IND");
    }

  }
}

#endif  /* DELAYED_RELEASE_IND */

T_RADIO_FRAME* dl_uplink(UBYTE channel, UBYTE sapi,
                         UBYTE no_signalling_mode, BOOL recursive)
{
  GET_INSTANCE_DATA;
  T_CCH*          pcch = &dl_data->cch[channel];
  T_RADIO_FRAME*  p_l2_frame = &dl_data->l2_frame;
  int             send = UPLINK_NULL;

  TRACE_EVENT_WIN_P3 ("uplink(): %s dcch0_ch_type:%s dedi=%u",
      CH_TYPE_NAME[pcch->ch_type], CH_TYPE_NAME[dl_data->dcch0_ch_type], dl_data->RR_dedicated);
  TRACE_EVENT_WIN_P6 ("UL: DCCH0=%s,%s vr=%u vs=%u va=%u T200=%u",
    CH_TYPE_NAME[dl_data->cch[C_DCCH0].ch_type],
    STATE_DCCH0_NAME[dl_data->state[C_DCCH0]],
    dl_data->cch[C_DCCH0].vr, dl_data->cch[C_DCCH0].vs, dl_data->cch[C_DCCH0].va,
    dl_data->cch[C_DCCH0].T200_counter);
  TRACE_EVENT_WIN_P6 ("UL: DCCH3=%s,%s vr=%u vs=%u va=%u T200=%u",
    CH_TYPE_NAME[dl_data->cch[C_DCCH3].ch_type],
    STATE_DCCH3_NAME[dl_data->state[C_DCCH3]],
    dl_data->cch[C_DCCH3].vr, dl_data->cch[C_DCCH3].vs, dl_data->cch[C_DCCH3].va,
    dl_data->cch[C_DCCH3].T200_counter);


  /* check SACCH SAPI and channel */
  if (pcch->ch_type EQ L2_CHANNEL_SACCH)
  {
    if (!dl_data->RR_dedicated)
    {
      sapi = PS_SAPI_0;
      send = UPLINK_EMPTY;
      TRACE_EVENT_WIN ("SACCH without dedicated channel: change SAPI->0, UPLINK_EMPTY");
    }
    else if (dl_data->state[C_DCCH0] EQ STATE_SUSPENDED)
    {
      sapi = PS_SAPI_0;
      send = UPLINK_REPORT;
      TRACE_EVENT_WIN ("SACCH during suspended dedicated channel: change SAPI->0, UPLINK_REPORT");
    }
    else if (sapi EQ PS_SAPI_3)
    {
      if ((dl_data->state[C_DCCH0] <= STATE_IDLE_DL)
             OR
          (dl_data->dcch0_ch_type EQ L2_CHANNEL_SDCCH)
             OR
          ((dl_data->cch[C_DCCH3].vtx EQ EMPTY_CMD) AND
           (!com_queue_awaiting_transmission (PS_SAPI_3)) AND
           (dl_data->state[C_DCCH3] NEQ STATE_AWAITING_ESTABLISHMENT)
           ) )
      {
        sapi = PS_SAPI_0;
        TRACE_EVENT_WIN ("SACCH with SAPI=3 only together with FACCH and frame is awaiting tx: change SAPI->0");
      }
      else
      {
        channel = C_DCCH3;
        pcch = &dl_data->cch[channel];
        TRACE_EVENT_WIN ("SACCH with SAPI=3 together with FACCH: change channel->C_DCCH3");
      }
    }
  }
  else if ((dl_data->RR_dedicated) AND
           (dl_data->dcch0_ch_type NEQ pcch->ch_type))
  {
    TRACE_EVENT_WIN_P2 ("dcch0_ch_type (%s) NEQ %s -> unexpected,unsolicited,invalid channel => UPLINK_EMPTY",
      CH_TYPE_NAME[dl_data->dcch0_ch_type],
      CH_TYPE_NAME[dl_data->cch[C_DCCH0].ch_type]);
    send = UPLINK_EMPTY;
  }

  /* decrease T200 counter ************************************ */
  if (channel EQ C_DCCH3)
  {
    if (dl_data->cch[C_DCCH3].T200_counter >= T200_ACTIVE)
    {
      dl_data->cch[C_DCCH3].T200_counter--;
      TRACE_EVENT_WIN_P2 ("T200(SAPI_0)=%u  T200(SAPI_3)=%u*",
        dl_data->cch[C_DCCH0].T200_counter, dl_data->cch[C_DCCH3].T200_counter);
      /*DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, "dec T200(SAPI_3)");*/
    }
  }
  else
  {
    if (pcch->ch_type NEQ L2_CHANNEL_SACCH) /* no T200 for SACCH with SAPI=0 */
    {
      if (dl_data->cch[C_DCCH0].T200_counter >= T200_ACTIVE)
      {
        dl_data->cch[C_DCCH0].T200_counter--;
        TRACE_EVENT_WIN_P2 ("T200(SAPI_0)=%u* T200(SAPI_3)=%u",
          dl_data->cch[C_DCCH0].T200_counter, dl_data->cch[C_DCCH3].T200_counter);
        /*DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, "dec T200(SAPI_0)");*/
      }
    }
  }

  /* check activity of DL ************************************* */
  if (dl_data->dl_active AND dl_data->state[channel] EQ STATE_MULTIPLE_FRAME_ESTABLISHED)
  {
    TRACE_EVENT_WIN_P1 ("uplink(): %s dl_active", PROCESS_NAME[channel]);
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, "UL:&dl_active");

    /* signalling only */
    send = UPLINK_EMPTY;
  }/* endif dl_active */

#if defined(DELAYED_SABM)
  /* delay uplink SABM to PL for FTA 26.6.6.1 and 25.2.3 *********** */
  if ((channel EQ C_DCCH0) AND (dl_data->dcch0_sabm_flag NEQ NOT_PRESENT_8BIT))
  {
    dcch0_delay_sabm (pcch);
    if (dl_data->dcch0_sabm_flag NEQ NOT_PRESENT_8BIT)
      send = UPLINK_EMPTY; /* send dummy only */
  }
#endif  /* DELAYED_SABM */


#if defined(DELAYED_RELEASE_IND)
  delayed_release_ind (channel);
#endif  /* DELAYED_RELEASE_IND */

  if ((send EQ UPLINK_NULL) OR (send EQ UPLINK_EMPTY))
  { /* After a L3 release at the last downlink DL should send a DISC */
    TRACE_EVENT_WIN_P6 ("uplink():%s %s SAPI=%u vtx=%s %s (#%u)",
      PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi,
      VTX_NAME[pcch->vtx], SEND_NAME[send], __LINE__);
    switch (channel)
    {
      case C_DCCH0:
        send = dcch0_check_disc (send);
        break;
      case C_DCCH3:
        send = dcch3_check_disc (send);
        break;
      default:
        break;
    }
    TRACE_EVENT_WIN_P6 ("uplink():%s %s SAPI=%u vtx=%s %s (#%u)",
      PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi,
      VTX_NAME[pcch->vtx], SEND_NAME[send], __LINE__);
  }

  if (send EQ UPLINK_NULL)
  {
    TRACE_EVENT_WIN_P5 ("uplink():%s %s SAPI=%u vtx=%s %s",
      PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi,
      VTX_NAME[pcch->vtx], recursive ? "RECURSIVE" : "");

    TRACE_EVENT_WIN_P8 ("UL: DCCH%u=%s,%s vr=%u vs=%u va=%u T200=%u (#%u)",
      sapi, CH_TYPE_NAME[dl_data->cch[channel].ch_type],
      STATE_DCCH3_NAME[dl_data->state[channel]],
      dl_data->cch[channel].vr, dl_data->cch[channel].vs, dl_data->cch[channel].va,
      dl_data->cch[channel].T200_counter, __LINE__);

    /* check timer T200 expiry ****************************** */
    if ((channel EQ C_DCCH0) AND
        (dl_data->cch[C_DCCH0].T200_counter EQ T200_EXPIRED))
      T200_expiry (C_DCCH0, PS_SAPI_0);
    if ((channel EQ C_DCCH3) AND
        (dl_data->cch[C_DCCH3].T200_counter EQ T200_EXPIRED))
      T200_expiry (C_DCCH3, PS_SAPI_3);

    TRACE_EVENT_WIN_P8 ("UL: DCCH%u=%s,%s vr=%u vs=%u va=%u T200=%u (#%u)",
      sapi, CH_TYPE_NAME[dl_data->cch[channel].ch_type],
      STATE_DCCH3_NAME[dl_data->state[channel]],
      dl_data->cch[channel].vr, dl_data->cch[channel].vs, dl_data->cch[channel].va,
      dl_data->cch[channel].T200_counter, __LINE__);

    /* state machine **************************************** */
    switch (dl_data->state[channel])
    {
    case STATE_IDLE_DL:
      if (pcch->vtx EQ EMPTY_CMD AND !dl_data->RR_dedicated)
        break;
      send = uplink_idle (channel, sapi);
      break;
    case STATE_CONTENTION_RESOLUTION: /* the same as STATE_AWAITING_ESTABLISHMENT */
      send = uplink_awaiting_establishment (channel, sapi, no_signalling_mode);
      break;
    case STATE_MULTIPLE_FRAME_ESTABLISHED:
      send = uplink_mfe (channel, sapi, no_signalling_mode);
      break;
    case STATE_TIMER_RECOVERY:
      send = uplink_timer_recovery (channel, sapi, no_signalling_mode);
      break;
    case STATE_AWAITING_RELEASE:
      send = uplink_awaiting_release ( channel, sapi);
      break;
    case STATE_SUSPENDED: /* only DCCH0 */
      send = UPLINK_NULL;
      break;
    case STATE_DISABLED:   /* only SABM/UA or SACCH and SDCCH (SAPI 3) */
      if (channel EQ C_DCCH3)
      {
        if (pcch->vtx EQ UA_CMD)
        {
          send = uplink_idle (channel, sapi);
        }
        else
        {
          send = UPLINK_EMPTY;
        }
      }
      break;
    }/* endswitch channel_state */

  }/* endif send == NULL */

  TRACE_EVENT_WIN_P8 ("UL: DCCH%u=%s,%s vr=%u vs=%u va=%u T200=%u (#%u)",
    sapi, CH_TYPE_NAME[dl_data->cch[channel].ch_type],
    STATE_DCCH3_NAME[dl_data->state[channel]],
    dl_data->cch[channel].vr, dl_data->cch[channel].vs, dl_data->cch[channel].va,
    dl_data->cch[channel].T200_counter, __LINE__);

  if (pcch->ch_type EQ L2_CHANNEL_SACCH)
  { /*
     * According to 3GPP TS 04.05, 4.2.2 Priority:
     * The priority arrangement on the SACCH must ensure that if a SAPI = 3
     * frame is awaiting transmission, two SAPI = 0 frames are not sent in
     * consecutive SACCH frames. In addition, for the mobile to network
     * direction it must also be ensured that any SAPI = 3 frame is followed
     * by at least one SAPI = 0 frame.
     */

    TRACE_EVENT_WIN_P3 ("sacch_last_uplink_sapi=%u SAPI=%u send=%s",
      dl_data->sacch_last_uplink_sapi, sapi, SEND_NAME[send]);

    if ((dl_data->sacch_last_uplink_sapi EQ PS_SAPI_3) OR
        ((sapi EQ PS_SAPI_3 ) AND (send EQ UPLINK_EMPTY)))
    {/* last uplinked SACCH frame was one with SAPI=3
      * or
      * no SACCH SAPI=3 frame is awaiting transmission
      * -> uplink of a SACCH SAPI=0 frame
      */
      sapi = PS_SAPI_0;
    }

    if (send EQ UPLINK_EMPTY)
    {
      sapi = PS_SAPI_0;
    }

    if ((sapi EQ PS_SAPI_0) AND (dl_data->state[C_DCCH0] >= STATE_SUSPENDED))
    { /*
       * uplink measurement reports only if SAPI=0 is suspended,
       * on contention resolution procedure, established or awaiting release
       */
      send = UPLINK_REPORT;
      TRACE_EVENT_WIN ("uplink(): SACCH REPORT now");
    }

    TRACE_EVENT_WIN_P2 ("sacch_last_uplink_sapi:=%u->%u", dl_data->sacch_last_uplink_sapi, sapi);
    dl_data->sacch_last_uplink_sapi = sapi;
  }
  else if ((pcch->ch_type EQ L2_CHANNEL_SDCCH) AND (channel NEQ C_DCCH3))
  { /*
     * According to 3GPP TS 04.05, 4.2.2 Priority:
     * The priority between data links on SDCCH shall be as follows:
     * Highest priority : SAPI = 0, Lowest priority : SAPI = 3.
     */
    if ((sapi EQ PS_SAPI_0) AND (send <= UPLINK_EMPTY))
    { /* special case: nothing is awaiting transmission for SAPI=0 */
      if ((dl_data->cch[C_DCCH3].vtx NEQ EMPTY_CMD) OR
          com_queue_awaiting_transmission(PS_SAPI_3) OR
          (dl_data->cch[C_DCCH3].T200_counter NEQ T200_STOPPED))
      {/* something is awaiting transmission for SAPI=3 */
        if (dl_data->state[C_DCCH0] >= STATE_CONTENTION_RESOLUTION)
        { /*
           * uplink SAPI=3 only if SAPI=0 is on contention resolution procedure
           * or established or awaiting release
           */
          send = UPLINK_DCCH3;
        }
      }

    }
  }
  else
  {
    /* no special treatment of FACCH */
  }

  /* return uplink frame buffer pointer dependent on the value of send */
  TRACE_EVENT_WIN_P4 ("%s on %s SAPI=%u RR_dedicated=%u",
    SEND_NAME[send], CH_TYPE_NAME[pcch->ch_type], sapi, dl_data->RR_dedicated);

  switch (send)
  {
  case UPLINK_NORMAL:
    break;

  case UPLINK_UA:
  case UPLINK_UA_F:
    com_build_UA_response (pcch->ch_type, sapi, (UBYTE)((send EQ UPLINK_UA_F) ? 1 : 0));
    break;

  case UPLINK_IFRAME:
  case UPLINK_IFRAME_P:
    {
      UBYTE m_bit;
      UBYTE p_bit = (send EQ UPLINK_IFRAME_P) ? 1 : 0;
      T_QUEUE *queue;

      if (pcch->ch_type EQ L2_CHANNEL_SACCH)
      {
        sapi = PS_SAPI_3; /* acknowledged mode only for SAPI=3 */
        TRACE_EVENT_WIN ("SACCH with I frame: acknowledged mode only for SAPI=3");
      }

      if (sapi EQ PS_SAPI_0)
        queue = &dl_data->dcch0_queue;
      else
        queue = &dl_data->dcch3_queue;

      if (send EQ UPLINK_IFRAME)
      {
        com_read_queue (pcch->ch_type, sapi, &m_bit);
      }
      else
      {/* TIMER_RECOVERY state -> repetition of the last frame */
        pcch->vs--;
        pcch->vs &= 7;
        pcch->p_bit_flag = 0;
        m_bit = queue->m_bit; /* remember last m bit  */
        TRACE_EVENT_WIN_P2 ("TIMER_RECOVERY state: decrement vs to %u, remember m=%u", pcch->vs, m_bit);
      }

      com_build_I_command (pcch->ch_type, sapi, pcch->vs, pcch->vr,
                           p_bit, m_bit, queue);
      pcch->vs++;
      pcch->vs &= 7;
      TRACE_EVENT_WIN_P4 ("%s SAPI=%u new vs=%u (pcch=%08x)",
        CH_TYPE_NAME[pcch->ch_type], sapi, pcch->vs, pcch);
    }
    set_T200_counter (pcch, sapi);
    break;

  case UPLINK_RR:
  case UPLINK_RR_F:
    if (pcch->ch_type EQ L2_CHANNEL_SACCH)
    {/*
      * There are some tests at the start of dl_uplink() to determine the right
      * SAPI value for SACCH. But the resulting value may be wrong because of
      * the priority arrangement according to 3GPP TS 04.05, section 4.2.2.
      * In case the MS has to uplink a RR frame it is clear that
      * L2 is in acknowledged mode and this can be done only with
      * a SAPI value of 3 for SACCH.
      */
      sapi = PS_SAPI_3; /* acknowledged mode only for SAPI=3 */
      TRACE_EVENT_WIN ("SACCH with supervisory frame: acknowledged mode only for SAPI=3");
    }
    com_build_RR_response (pcch->ch_type, sapi, pcch->vr,
                          (UBYTE)((send EQ UPLINK_RR_F) ? 1 : 0));
    break;

  case UPLINK_REJ:
  case UPLINK_REJ_F:
    if (pcch->ch_type EQ L2_CHANNEL_SACCH)
    {/*
      * There are some tests at the start of dl_uplink() to determine the right
      * SAPI value for SACCH. But the resulting value may be wrong because of
      * the priority arrangement according to 3GPP TS 04.05, section 4.2.2.
      * In case the MS has to uplink a REJ frame it is clear that
      * L2 is in acknowledged mode and this can be done only with
      * a SAPI value of 3 for SACCH.
      */
      sapi = PS_SAPI_3; /* acknowledged mode only for SAPI=3 */
      TRACE_EVENT_WIN ("SACCH with supervisory frame: acknowledged mode only for SAPI=3");
    }
    com_build_REJ_response (pcch->ch_type, sapi, pcch->vr,
                           (UBYTE)((send EQ UPLINK_REJ_F) ? 1 : 0));
    break;

  case UPLINK_REPORT:
    if (dl_data->RR_dedicated)
    {/* measurement report only in RR dedicated mode */
      sacch0_send_data ();
      com_l3trace (TRACE_UACK_UP, pcch->ch_type, (UBYTE *)p_l2_frame+5);


      /* no deleting of vtx!!! */
      return p_l2_frame;
      /* break; not necessary */
    }/* endif measurement report only in RR dedicated mode */

    /* else go through to UPLINK_EMPTY */
    /*lint -fallthrough*/

  default:
    possible_reset_dcch0_ch_type();

    /* check RR message with short PD, short L2 header type 1, format Bter */
    if ((dl_data->state[C_DCCH0] >= STATE_SUSPENDED) AND
         dl_data->rr_short_pd_buffer.l_buf AND
        (dl_data->rr_short_pd_ch_type EQ pcch->ch_type))
    { /* RR message with short PD, short L2 header type 1, format Bter */
      com_build_UI_Bter (pcch->ch_type);
      return p_l2_frame;
    }

    if (((pcch->ch_type EQ L2_CHANNEL_FACCH_F) OR
         (pcch->ch_type EQ L2_CHANNEL_FACCH_H))
        AND
        ((no_signalling_mode NEQ SIG_ONLY) 
#if defined(DELAYED_SABM)
        OR
         /* suppress UI frames if delayed SABM is pending
          * (independent from signalling mode) */
          (dl_data->dcch0_sabm_flag NEQ NOT_PRESENT_8BIT) 
#endif  /* DELAYED_SABM */
#if defined(DELAYED_RELEASE_IND)
        OR
         (dl_data->release_ind_ch_type NEQ NOT_PRESENT_8BIT)
#endif  /* DELAYED_RELEASE_IND */
       ))
    {
      TRACE_EVENT_WIN ("UPLINK_NULL return");
      return NULL;
    }
    /* break; not necessary */

    {
      T_RADIO_FRAME* empty_frame;

      TRACE_EVENT_WIN ("UPLINK_EMPTY return");
      /* The use of one byte array instead of two T_RADIO_FRAME structures
       * saves 21 byte. Casting is in this case the better way then two calls
       * of memcpy.
       */
      if (pcch->ch_type EQ L2_CHANNEL_SACCH)
        empty_frame = (T_RADIO_FRAME*)(&l2_empty_frame[0]);
      else
        empty_frame = (T_RADIO_FRAME*)(&l2_empty_frame[2]);

      ATRC (empty_frame, 23);

      return empty_frame;
    }
    /* break; not necessary */

  case UPLINK_DCCH3:
    return dl_uplink (C_DCCH3, PS_SAPI_3, no_signalling_mode, TRUE);
    /* break; not necessary */
  }/* endswitch send */


  /*
   * UPLINK_NORMAL, UPLINK_IFRAME (_P), UPLINK_UA (_F),
   * UPLINK_RR (_F), UPLINK_REJ (_F)
   */
  TRACE_EVENT_WIN_P5 ("%s SAPI=%u delete vtx=%s ->EMPTY_CMD (%s#%u)",
    CH_TYPE_NAME[pcch->ch_type], sapi, VTX_NAME[pcch->vtx], __FILE10__, __LINE__);
  pcch->vtx = EMPTY_CMD;

  possible_reset_dcch0_ch_type();

  return p_l2_frame;
}/* endfunc dl_uplink */

static int uplink_idle (UBYTE channel, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_CCH*          pcch = &dl_data->cch[channel];
  int             ret = UPLINK_NORMAL;
  int             ua_response = FALSE;

  TRACE_EVENT_WIN_P3 ("uplink_idle(): %s %s SAPI=%d",
    PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi);

  switch (channel)
  {
  case C_DCCH0:
    switch (pcch->vtx)
    {
    case UA_CMD:
      ret = pcch->f_bit ? UPLINK_UA_F : UPLINK_UA;
      break;
    case DM_CMD:
      com_build_DM_response (pcch->ch_type, sapi, pcch->f_bit);
      break;
    default:
      ret = UPLINK_EMPTY;/* no SAPI=3 frame if DCCH0 is idle */
      break;
    }
    break;

  case C_DCCH3:
    switch (pcch->vtx)
    {
    case UA_CMD:
      ua_response = TRUE;
      ret = pcch->f_bit ? UPLINK_UA_F : UPLINK_UA;
      break;
    case DM_CMD:
      com_build_DM_response (pcch->ch_type, PS_SAPI_3, 1);
      ret = UPLINK_NORMAL;
      break;
    default:
      ret = UPLINK_EMPTY;
      break;
    }
    break;
  }/* endswitch channel */

  if (ua_response AND (ret EQ UPLINK_UA_F))
  {
    drr_dl_establish_ind (pcch->ch_type, sapi,
      (UBYTE)(com_queue_awaiting_transmission (sapi) ? DL_UNSERVED : DL_ALL_DONE));
    set_channel_state (channel, STATE_MULTIPLE_FRAME_ESTABLISHED);
  }

  return ret;
}/* endfunc uplink_idle */

static int uplink_awaiting_establishment (UBYTE channel, UBYTE sapi,
                                 UBYTE no_signalling_mode)
{
  GET_INSTANCE_DATA;
  T_CCH*  pcch = &dl_data->cch[channel];
  int   ret = UPLINK_NORMAL;
  int   ua_response = FALSE;
  int   sabm_command = 0;

  TRACE_EVENT_WIN_P3 ("uplink_awaiting_establishment(): %s %s SAPI=%d",
    PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi);

  if (pcch->time_flag)
    set_T200_counter (pcch, sapi);

  switch (channel)
  {
  case C_DCCH0:
    if (pcch->vtx EQ SABM_CMD)
    {
      if (pcch->contention_resolution)
        sabm_command = 2; /* com_build_SABM with L3 */
      else
        sabm_command = 1; /* com_build_SABM without L3 */
    }
    else
    {
      ret = UPLINK_EMPTY;/* no SAPI=0 frame is waiting transmission */
    }
    break;

  case C_DCCH3:
    switch (pcch->vtx)
    {
    case SABM_CMD:
      sabm_command = 1; /* com_build_SABM without L3 */
      break;
    case UA_CMD:
      ua_response = TRUE;
      break;
    case DM_CMD:
      com_build_DM_response (pcch->ch_type, sapi, pcch->f_bit);
      break;
    default:
      ret = UPLINK_EMPTY;
      break;
    }

    break;
  }

  if (ua_response)
  {
    drr_dl_establish_cnf (pcch->ch_type, sapi);
    set_channel_state (channel, STATE_MULTIPLE_FRAME_ESTABLISHED);

    ret = pcch->f_bit ? UPLINK_UA_F : UPLINK_UA;
  }
  else if (sabm_command)
  {
    com_build_SABM(pcch->ch_type, sapi, sabm_command EQ 2);
  }

  return ret;
}/* endfunc uplink_awaiting_establishment */

static int uplink_mfe (UBYTE channel, UBYTE sapi, UBYTE no_signalling_mode)
{
  GET_INSTANCE_DATA;
  T_CCH*  pcch = &dl_data->cch[channel];
  int   ret = UPLINK_NORMAL;

  TRACE_EVENT_WIN_P6 ("uplink_mfe(): %s:%s SAPI=%d vtx=%s (vs=%u va+1=%u)",
    PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi,
    VTX_NAME[pcch->vtx], pcch->vs, (pcch->va + 1)&7 );

  if (pcch->time_flag)
    set_T200_counter (pcch, sapi);

  switch (pcch->vtx)
  {
  case RR_CMD:
    if (pcch->f_bit_flag)
    {
      ret = UPLINK_RR_F;
      pcch->f_bit_flag = 0;
    }
    else
    {
      if (!com_queue_awaiting_transmission (sapi))
        ret = UPLINK_RR;
      else
      {
        if (pcch->vs EQ ((pcch->va + 1)&7))
          ret = UPLINK_RR;
        else
          ret = UPLINK_IFRAME;
      }
    }
    break;

  case UA_CMD:
    ret = pcch->f_bit ? UPLINK_UA_F : UPLINK_UA;
    break;

  case RR_RSP:
    ret = pcch->f_bit ? UPLINK_RR_F : UPLINK_RR;
    pcch->f_bit_flag = 0;
    break;

  case REJ_CMD:
    ret = pcch->f_bit ? UPLINK_REJ_F : UPLINK_REJ;
    pcch->f_bit_flag = 0;
    break;

  default:  /* vtx = ELSE */
    switch (channel)
    {
    case C_DCCH0:
      if (com_queue_awaiting_transmission (PS_SAPI_0)
            AND
          (pcch->vs NEQ ((pcch->va + 1)&7)))
        ret = UPLINK_IFRAME;
      else
      {
        ret = UPLINK_EMPTY;/* no SAPI=0 frame is waiting transmission */
      }
      break;

    case C_DCCH3:
      if (com_queue_awaiting_transmission (PS_SAPI_3))
      {
        if (pcch->vs EQ ((pcch->va + 1)&7))
          ret = UPLINK_EMPTY;
        else
          ret = UPLINK_IFRAME;
      }
      else
      {
        ret = dcch3_check_disc (UPLINK_EMPTY);/* no SAPI=3 frame is waiting transmission */
      }
      break;
    }/* endswitch channel */
    break;
  }/* endswitch vtx */

  return ret;
}/* endfunc uplink_mfe */

static int uplink_timer_recovery (UBYTE channel, UBYTE sapi, UBYTE no_signalling_mode)
{
  GET_INSTANCE_DATA;
  T_CCH*  pcch = &dl_data->cch[channel];
  int   ret = UPLINK_NORMAL;

  TRACE_EVENT_WIN_P3 ("uplink_timer_recovery(): %s %s SAPI=%d",
    PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi);

  if (pcch->time_flag)
    set_T200_counter (pcch, sapi);

  switch (pcch->vtx)
  {
  case RR_CMD:
    if (pcch->f_bit_flag)
    {
      ret = UPLINK_RR_F;
      pcch->f_bit_flag = 0;
    }
    else
    {
      if (pcch->p_bit_flag)
        ret = UPLINK_IFRAME_P;
      else
        ret = UPLINK_RR;
    }
    break;

  case UA_CMD:
    ret = pcch->f_bit ? UPLINK_UA_F : UPLINK_UA;
    break;

  case RR_RSP:
    ret = pcch->f_bit ? UPLINK_RR_F : UPLINK_RR;
    pcch->f_bit_flag = 0;
    break;

  case REJ_CMD:
    ret = pcch->f_bit ? UPLINK_REJ_F : UPLINK_REJ;
    pcch->f_bit_flag = 0;
    break;

  default:  /* vtx = ELSE */
    TRACE_EVENT_WIN_P3 ("%s SAPI=%u vtx=%s",
      CH_TYPE_NAME[pcch->ch_type], sapi, VTX_NAME[pcch->vtx]);

    if (pcch->p_bit_flag)
      ret = UPLINK_IFRAME_P;
    else
      switch (channel)
      {
      case C_DCCH0:
        ret = UPLINK_EMPTY;/* no SAPI=0 frame is waiting transmission */
        break;

      case C_DCCH3:
        ret = dcch3_check_disc (UPLINK_EMPTY);/* no SAPI=3 frame is waiting transmission */
        break;
      }
    break;
  }

#if 0 /* decrement first short before sending the frame */
  if (ret EQ UPLINK_IFRAME_P)
  {
    pcch->vs--;
    pcch->vs &= 7;
    pcch->p_bit_flag = 0;
  }
#endif /* 0 */
  return ret;
}/* endfunc uplink_timer_recovery */

GLOBAL int uplink_awaiting_release (UBYTE channel, UBYTE sapi)
{
  GET_INSTANCE_DATA;
  T_CCH*  pcch = &dl_data->cch[channel];
  int   ret = UPLINK_NORMAL;

  TRACE_EVENT_WIN_P3 ("uplink_awaiting_release(): %s %s SAPI=%d",
    PROCESS_NAME[channel], CH_TYPE_NAME[pcch->ch_type], sapi);

  if (pcch->time_flag)
    set_T200_counter (pcch, sapi);

  switch (pcch->vtx)
  {
  case DISC_CMD:
    /* DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, "UL:Send DISC"); */
    com_build_DISC_command (pcch->ch_type, sapi, 1);
    break;
  case UA_CMD:
    set_channel_state (channel, STATE_IDLE_DL);

#if defined(DELAYED_RELEASE_IND)
    switch (pcch->ch_type)
    {
    #if defined(DL_FACCH_RELEASE_DELAY_VALUE) && (DL_FACCH_RELEASE_DELAY_VALUE > 0)
      case L2_CHANNEL_FACCH_F:
      case L2_CHANNEL_FACCH_H:
          /* delay DL RELEASE IND to RR for testcase 25.2.3 */
          dl_data->release_ind_ch_type = pcch->ch_type;
          dl_data->release_ind_sapi = sapi;
          if (dl_data->release_ind_delay EQ 0)
          {
            dl_data->release_ind_delay = DL_FACCH_RELEASE_DELAY_VALUE;
            DL_OFFLINE_TRACE (TRACE_DL_EVENT, C_DCCH0, pcch->ch_type, "delay REL IND on FACCH");
          }
        break;
    #endif /* DL_FACCH_RELEASE_DELAY_VALUE */
    #if defined(DL_SDCCH_RELEASE_DELAY_VALUE) && (DL_SDCCH_RELEASE_DELAY_VALUE > 0)
      case L2_CHANNEL_SDCCH:
          /* delay DL RELEASE IND to RR for testcase 25.2.3 */
          dl_data->release_ind_ch_type = pcch->ch_type;
          dl_data->release_ind_sapi = sapi;
          if (dl_data->release_ind_delay EQ 0)
          {
            dl_data->release_ind_delay = DL_SDCCH_RELEASE_DELAY_VALUE;
            DL_OFFLINE_TRACE (TRACE_DL_EVENT, channel, pcch->ch_type, "delay REL IND on SDCCH");
          }
        break;
    #endif /* DL_SDCCH_RELEASE_DELAY_VALUE */
      default:
        drr_dl_release_ind (pcch->ch_type, sapi, NOT_PRESENT_8BIT, TRUE);
        break;
    }
#else  /* DELAYED_RELEASE_IND */
    drr_dl_release_ind (pcch->ch_type, sapi, NOT_PRESENT_8BIT, TRUE);
#endif  /* DELAYED_RELEASE_IND */

#if defined(LATE_LEAVING_DEDICATED)
    com_leave_dedicated (pcch->ch_type);
#endif  /* LATE_LEAVING_DEDICATED */
    ret = UPLINK_UA_F;
    break;

  default:
    switch (pcch->ch_type)
    {
      case L2_CHANNEL_SACCH:
        if (channel EQ C_DCCH0)
          ret = UPLINK_REPORT;
        break;
      default:
        if (channel EQ C_DCCH0)
        {
          pcch = &dl_data->cch[C_DCCH3];
          if (pcch->time_flag)
            set_T200_counter (pcch, PS_SAPI_3);
        }
        ret = UPLINK_EMPTY;
      break;
    }/* endswitch channel */
  }/* endswitch vtx */

  return ret;
}/* endfunc uplink_awaiting_release */

#if defined(CHECK_PCCHI)
static void check_pcch_i (T_CCH_INTERN* pcch_i, int line)
{
  char buf[23];
  int ret = 0;

  if (pcch_i EQ NULL)
    ret = -1;
  else if (pcch_i->pcch EQ NULL)
    ret = -2;

  if (ret)
  {
    sprintf (buf, "#%d pcch=NULL %d", line, -ret);
    DL_OFFLINE_TRACE (TRACE_DL_EVENT, TRACE_CH_UNKNOWN, 0,buf);
  }

  return ret;
}/* endfunc check_pcch_i */
#endif  /* CHECK_PCCHI */

#endif /* DL_C */

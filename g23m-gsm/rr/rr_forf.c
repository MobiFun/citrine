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
|  Purpose :  Handling of Common Information Elements.
+-----------------------------------------------------------------------------
*/

#ifndef RR_FORF_C
#define RR_FORF_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_rr.h"
#include "tok.h"
#include "rr.h"

/*==== EXPORT =====================================================*/

/*==== PRIVATE =====================================================*/
static BOOL   for_create_delta_list     (T_freq_chan_seq_after *delta_list,
                                         T_LIST              *hop_list);
static void   for_decode_param          (const T_W_PARAM     *param,
                                         SHORT               *w,
                                         T_f_range           *cha,
                                         USHORT              initial_value);
static void   for_decode_param_1024     (SHORT               *w,
                                         T_f_range           *cha);
static USHORT for_get_generation        (USHORT              value);
static void   for_decode_frequencies    (SHORT               original_range,
                                         SHORT               *w,
                                         T_LIST              *f,
                                         SHORT               offset);
static LONG   for_modulo                (LONG                a,
                                         LONG                b);
static LONG   for_smodulo               (LONG                a,
                                         LONG                b);
static void for_set_conditional_error   (void);

#if defined (TI_PS_FF_EMR) AND defined (GPRS)
static void for_store_nc_para(T_nc_meas_para *p_nc,T_rr_enh_para *p_em);
#endif
LOCAL void for_frequency_list(T_freq_list *freq_list_starting_time_cmd,
                              T_f_range *freq_list_starting_time_para,
                              T_LIST *hop_list_starting_time);


LOCAL void for_decode_param_freq(USHORT  range ,T_f_range *cha_list_descr,
                                 T_LIST  *cha_list);        

/*==== GLOBAL VARIABLES ===========================================*/
/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * Procedures
 * -------------------------------------------------------------------
 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_ba_range         |
+--------------------------------------------------------------------+

  PURPOSE : The function checks the content of a ba range information
            element which is used in the channel release message.

*/

GLOBAL BOOL for_check_ba_range (T_ba_range *ba_range)
{
  USHORT i = 0;
  UBYTE  bands_used;

  TRACE_FUNCTION ("for_check_ba_range()");

  /*
   * for all ranges
   */
  for (i = 0; i < ba_range->c_freq_range; i++)
  {
    /*
     * check the lower range boarder
     */
    bands_used = for_check_frequency (ba_range->freq_range[i].freq_lower);
    if (bands_used <= INVALID_BAND_USED)
      return FALSE;

    /*
     * check the upper range boarder
     */
    bands_used = for_check_frequency (ba_range->freq_range[i].freq_higher);
    if (bands_used <= INVALID_BAND_USED)
      return FALSE;
  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_assign_cmd       |
+--------------------------------------------------------------------+

  PURPOSE : Content check of assignment command message.

*/

typedef struct
{
  UBYTE        count_before;
  UBYTE        count_after;
  T_LIST       cell_chan_desc;
  T_LIST       hop_list_after;
  T_LIST       hop_list_before;
  T_f_range    freq_list_after;
  T_f_range    freq_list_before;
} T_ASSIGN_PARA;

GLOBAL void for_check_assign_cmd (T_DL_DATA_IND * dl_data_ind,
                                  T_D_ASSIGN_CMD *assign_cmd)
{
  GET_INSTANCE_DATA;
  T_ASSIGN_PARA * a_para;
  MALLOC (a_para, sizeof (T_ASSIGN_PARA));

  a_para->count_before = 0;
  a_para->count_after  = 0;

  TRACE_EVENT ("for_check_assign_cmd()");

  /*
   * initialize several lists for frequency hopping purposes
   */
  srv_clear_list (&a_para->cell_chan_desc);
  srv_clear_list (&a_para->hop_list_after);
  srv_clear_list (&a_para->hop_list_before);

  /*
   * check the channel description
   */
  for_check_channel_descr (&assign_cmd->chan_desc);

  if (assign_cmd->v_cell_chan_desc)
  {
    /*
     * if a cell channel description is inserted
     * create the cell channel list and check it.
     */
    for_create_channel_list ((T_f_range *)&assign_cmd->cell_chan_desc,
                             &a_para->cell_chan_desc);
  }

  if (assign_cmd->chan_desc.hop AND assign_cmd->v_freq_list_after)
  {
    /*
     * if the message contains a frequency list
     */
/* Implements RR Clone findings #5 */
    for_frequency_list(&assign_cmd->freq_list_after,
                       &a_para->freq_list_after,
                       &a_para->hop_list_after);
  }

  /*
   * check the channel mode, if available.
   */
  if (assign_cmd->v_chan_mode)
    for_check_channel_mode (assign_cmd->chan_mode);

  /*
   * if the message contains a channel mode information element which
   * indicates AMR, check the multirate configuration information element.
   * or if no channel mode is present but we have been using AMR before
   * check the supplied mulitrate configuration
   */
  if ( ( assign_cmd->v_chan_mode AND (assign_cmd->chan_mode    EQ CM_AMR)) OR
       (!assign_cmd->v_chan_mode AND (rr_data->sc_data.ch_mode EQ CM_AMR))
     )
  {
    if (assign_cmd->v_multirate_conf)
      for_check_multirate_conf( &assign_cmd->multirate_conf, assign_cmd->chan_desc.chan_type);

    /*
     * check if during initial assignment the multirate configuration element is present
     * otherwise remain on the current channel and use the old channel description.
     */
    if ( (rr_data->sc_data.ch_mode NEQ CM_AMR) AND (!assign_cmd->v_multirate_conf) )
      for_set_content_error (RRC_CHANNEL_MODE);

    /*
     * If the assignment is related to an intra-cell handover from a multi-rate speech codec
     * to a multi-rate speech codec, the MultiRate Configuration IE shall be included in the
     * case of full rate to half rate. If not included in this case, the mobile station shall
     * behave as if the MultiRate Configuration IE was inconsistent.
     */
    if ( rr_data->sc_data.ch_mode             EQ CM_AMR    AND
         rr_data->sc_data.chan_desc.chan_type EQ TCH_F     AND
         assign_cmd->chan_mode                EQ CM_AMR    AND
        (assign_cmd->chan_desc.chan_type      EQ TCH_H_S0  OR
         assign_cmd->chan_desc.chan_type      EQ TCH_H_S1) AND
         assign_cmd->v_multirate_conf         EQ 0             )
    {
      for_set_content_error (RRC_CHANNEL_MODE);
    }
  }

  /*
   * check the cipher mode setting if available
   */
  if (assign_cmd->v_ciph_mode_set)
  {
    for_check_cipher_mode_set (&assign_cmd->ciph_mode_set);
  }

  a_para->count_after = assign_cmd->v_freq_list_after +
                        assign_cmd->v_mob_alloc_after;

  if (assign_cmd->chan_desc.hop)
  {
    /*
     * In case of frequency hopping, check whether more
     * then one possibility is defined to build a frequency
     * hopping list -> this means inconsistency and is
     * a conditional error.
     */
    if (a_para->count_after NEQ 1)
    {
      for_set_conditional_error ();
    }
  }

  if (assign_cmd->v_start_time)
  {
    /*
     * If a starting time is present, some
     * elements before starting time must be set.
     */
    a_para->count_before = assign_cmd->v_chan_desc_before +
                           assign_cmd->v_freq_list_before +
                           assign_cmd->v_mob_alloc_before +
                           assign_cmd->v_freq_chan_seq;

    if (a_para->count_before NEQ 0 AND
        assign_cmd->v_chan_desc_before EQ FALSE)
    {
      /*
       * a frequency hopping definition is available,
       * but no channel description before starting time.
       * then use the channel description after starting time.
       */
      memcpy (&assign_cmd->chan_desc_before,
              &assign_cmd->chan_desc,
              sizeof (T_chan_desc));
      assign_cmd->v_chan_desc_before = TRUE;
    }

    if (assign_cmd->v_chan_desc_before)
    {
      /*
       * if a channel description before starting time
       * is available.
       */
      if (assign_cmd->chan_desc_before.hop)
      {
        /*
         * if the channel description before starting time
         * uses a frequency hopping list, count the possible
         * variants of building a frequency hopping list.
         */
        a_para->count_before = assign_cmd->v_freq_list_before +
                               assign_cmd->v_mob_alloc_before +
                               assign_cmd->v_freq_chan_seq;

        switch (a_para->count_before)
        {
          /*
           * no before elements to build a hopping list
           */
          case 0:
            if (a_para->count_after EQ 1)
            {
              /*
               * use the after starting time variant also
               * for the before starting time frequency list.
               */
              memcpy (&assign_cmd->freq_list_before,
                      &assign_cmd->freq_list_after,
                      sizeof (T_freq_list));
              assign_cmd->v_freq_list_before = assign_cmd->v_freq_list_after;
              memcpy (&assign_cmd->mob_alloc_before,
                      &assign_cmd->mob_alloc_after,
                      sizeof (T_mob_alloc_after));
              assign_cmd->v_mob_alloc_before = assign_cmd->v_mob_alloc_after;
            }
            else
            {
              /*
               * A conditional error is detected. The channel description
               * before starting time shall use frequency hopping, but no
               * frequency hopping list can be created.
               */
              for_set_conditional_error ();
            }
            break;

          case 1:
            /*
             * There is just one variant to build the frequency
             * hopping list before starting time.
             */
            break;

          default:
            /*
             * There are more then one variant to build the
             * frequency hopping list before starting time.
             * This is detected as a conditional error.
             */
            for_set_conditional_error ();
            break;
        }
      }
      else
      { /* IEs are unnecessary */
        assign_cmd->v_freq_list_before =
          assign_cmd->v_mob_alloc_before =
          assign_cmd->v_freq_chan_seq = 0;
      }
    }
  }
  else
  { /* IEs are unnecessary */
    assign_cmd->v_chan_desc_before =
      assign_cmd->v_freq_list_before =
      assign_cmd->v_mob_alloc_before =
      assign_cmd->v_freq_chan_seq = 0;
  }

  if (assign_cmd->v_chan_desc_before)
  {
    /*
     * check the channel description before starting time
     * if available.
     */
    for_check_channel_descr ((T_chan_desc *)&assign_cmd->chan_desc_before);
  }

  if (assign_cmd->v_freq_list_before)
  {

/* Implements RR Clone findings #5 */
    for_frequency_list (&assign_cmd->freq_list_before,
                        &a_para->freq_list_after,
                        &a_para->hop_list_before);
  }

  if (assign_cmd->v_freq_chan_seq)
  {
    /*
     * if a frequency channel sequence information element is
     * available, build a frequency hopping list.
     */
    if (!for_create_delta_list ((T_freq_chan_seq_after *)
                               &assign_cmd->freq_chan_seq,
                               &a_para->hop_list_before))
    {
      /*
       * set a content error if the frequency hopping list
       * contains channel numbers which are not supported.
       */
      for_set_content_error (RRC_FREQ_NOT_IMPL);
    }
  }

  /*
   * configure layer 1 for the new channel
   */
  dat_for_assign_cmd (dl_data_ind, assign_cmd, &a_para->hop_list_after,
                      &a_para->hop_list_before, &a_para->cell_chan_desc);

  /*
   * de-allocate the dynamic memory
   */
  MFREE (a_para);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_cell_descr       |
+--------------------------------------------------------------------+

  PURPOSE : check the content of the information element cell
            description.

*/

GLOBAL void for_check_cell_descr (T_cell_desc *cell_desc)
{
  USHORT bcch_arfcn;
  UBYTE  result;

  TRACE_FUNCTION ("for_check_cell_desc()");

  /*
   * calculate the BCCH channel number
   */
  bcch_arfcn =  (cell_desc->bcch_arfcn_hi << 8) +
                 cell_desc->bcch_arfcn_lo;

  /*
   * check the BCCH channel number.
   * If the number is not a GSM channel number set the cause
   * INCORRECT MESSAGE.
   * If the number is a GSM channel number, but not supported
   * by the mobile, set the cause FREQUENCY NOT IMPLEMENTED.
   */
  result = for_check_frequency (bcch_arfcn);
  if (result EQ UNKNOWN_BAND_USED)
    for_set_content_error (RRC_INCORRECT_MSG);
  else if( result EQ INVALID_BAND_USED )
    for_set_content_error (RRC_FREQ_NOT_IMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_channel_descr    |
+--------------------------------------------------------------------+

  PURPOSE : check the content of the information element channel
            description.

*/

GLOBAL void for_check_channel_descr (T_chan_desc *chan_desc)
{
  GET_INSTANCE_DATA;
  BOOL   result = TRUE;

  TRACE_FUNCTION ("for_check_channel_descr()");

  /*
   * check the channel type
   */
  switch (chan_desc->chan_type)
  {
    case TCH_H_S0:
    case TCH_H_S1:
      /*
       * TCH Halfrate channels
       * check against the mobile capabilities in PCM
       * (halfrate support)
       */
      if (FldGet(rr_data->mscap.chnMode, hrSup) EQ 0)
        for_set_content_error (RRC_CHANNEL_MODE);
      break;

    default:
      /*
       * 1 (= TCH Fullrate) or 4..15 (= SDCCH)
       * are allowed values. Any other value
       * is not supported.
       */
      if ((chan_desc->chan_type EQ 0)
           OR (chan_desc->chan_type > CH_SDCCH_8_7))
        for_set_content_error (RRC_CHANNEL_MODE);
      break;
  }

  if (chan_desc->hop EQ 0)
  {
    /*
     * without frequency hopping.
     * Then check the channel number of the channel
     * description.
     * If the number is not a GSM channel number set the cause
     * INCORRECT MESSAGE.
     * If the number is a GSM channel number, but not supported
     * by the mobile, set the cause FREQUENCY NOT IMPLEMENTED.
     */
    result = for_check_frequency (chan_desc->arfcn);
    if (result EQ UNKNOWN_BAND_USED)
      for_set_content_error (RRC_INCORRECT_MSG);
    else if (result EQ INVALID_BAND_USED)
      for_set_content_error (RRC_FREQ_NOT_IMPL);
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : for_check_multirate_conf
+------------------------------------------------------------------------------
|  Description  :  Set the new multi-rate speech codec elements
|
|  Parameters   :  amr_conf - MultiRate configuration IEI
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void for_check_multirate_conf    (T_multirate_conf * multirate_conf, UBYTE chan_type)
{
  UBYTE acs = multirate_conf->set_amr;
  UBYTE i=0, num_acs=0;

  TRACE_FUNCTION ("for_check_multirate_conf()");

  /*
   *  Check number of codec modes inside the set of AMR codec mode mask
   */
  for(i=0; i<8; i++)
  {
    if( (0x01<<i) & acs )
    {
      num_acs++;
      TRACE_EVENT_P1("AMR no. acs %d", num_acs);
    }
  }

  /*
  * From 3GPP TS 04.18
  *
  * The MultiRate Configuration IE shall be considered as inconsistent by the MS if:
  *
  * -     the active set does not include any codec mode or the active set includes more than four codec modes; or
  * -     one or more codec modes of the active codec set are not supported by the assigned channel; or
  * -     the threshold and hysteresis values are not set according to requirements given in 3GPP TS 05.09.
  *
  * Refer to 3GPP TS 05.05 for codec modes supported by different channels.
  */

  if(
     ((chan_type EQ CH_TCH_H_1) OR (chan_type EQ CH_TCH_H_2))
     AND
     ((acs & SET_AMR_10_2) OR (acs & SET_AMR_12_2))
    )
  {
    TRACE_EVENT_P2(" chan_type = %d acs = %d, invalid acs for TCH AHS", chan_type, acs);
    for_set_content_error (RRC_CHANNEL_MODE);
    return;
  }

  /*
  *  The active set does not include any codec mode or the active set includes more than four
  *  codec modes.
  */
  if( (num_acs > MAX_NO_ACS) OR (num_acs EQ 0) )
  {
    TRACE_EVENT("AMR ((num_acs > MAX_NO_ACS) OR (num_acs EQ 0))");
    for_set_content_error (RRC_CHANNEL_MODE);
    return;
  }

  /*
  *  One or mode codec modes of the active codec set are not supported by
  *  the assigned channel.
  */
  if ( num_acs < multirate_conf->st_mode )
  {
    /*
     *  The start mode is not supported by the acs, e.g. CODEC_MODE_4 is requested but only 3
     *  codec modes are part of the acs.
     */
    TRACE_EVENT("(num_acs < multirate_conf->st_mode)");
    for_set_content_error (RRC_CHANNEL_MODE);
    return;
  }

  /*
  *  The threshold and hysteresis values are not set according to requirements in 3GPP TS 05.09
  */
  /*
  *  Number of codec modes minus one must be equal to number of thresholds and hysteresis
  */
  if( ((num_acs-1) NEQ multirate_conf->c_cod_prop) )
  {
    TRACE_EVENT("((num_acs-1) NEQ multirate_conf->c_cod_prop))");
    for_set_content_error (RRC_CHANNEL_MODE);
    return;
  }

  /*
  *  Check if the thresholds and hysteresis values are in consistent order
  */
  for(i=0; i<(multirate_conf->c_cod_prop-1); i++)
  {
    if(multirate_conf->cod_prop[i].codec_thr > multirate_conf->cod_prop[i+1].codec_thr)
    {
/* Implements Measure#32: Row 252 */
      TRACE_EVENT_P2("cod_prop[%d].codec_thr > cod_prop[%d+1].codec_thr",i,i);
      for_set_content_error (RRC_CHANNEL_MODE);
    }
    else
    {
      if( (multirate_conf->cod_prop[i].codec_thr + multirate_conf->cod_prop[i].codec_hyst) >
          (multirate_conf->cod_prop[i+1].codec_thr + multirate_conf->cod_prop[i+1].codec_hyst) )
      {
/* Implements Measure#32: Row 251 */
        TRACE_EVENT_P4("cod_prop[%d].codec_thr+cod_prop[%d].codec_hyst>cod_prop[%d+1].codec_thr+cod_prop[%d+1].codec_hyst", i,i,i,i);
        for_set_content_error (RRC_CHANNEL_MODE);
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_channel_mode     |
+--------------------------------------------------------------------+

  PURPOSE : check the information element channel mode.

*/

GLOBAL void for_check_channel_mode (UBYTE     ch_mod)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_check_channel_mode()");

/*
*  Reads the ms bearer caps a second time after a config primitive was sent
*  during a test case. Otherwise the new data will be ignored.
*/
#if defined (_SIMULATION_)
  rr_csf_ms_cap ();
#endif

  /*
   * depending on the channel mode.
   */
  switch (ch_mod)
  {
    case CM_DATA_12_0:   /* data 12  k         */
    case CM_DATA_6_0:    /* data 6   k         */
    case CM_DATA_3_6:    /* data 3.6 k         */
      /*
       * check against the data capabilities of the
       * mobile. The PCM record must indicate data
       * support.
       */
      if (FldGet(rr_data->mscap.datCap1, datSup) EQ 0)
        for_set_content_error (RRC_CHANNEL_MODE);
      break;

    case CM_DATA_14_4:    /* data 14.4 k        */
      /*
       * check against the data capabilities of the
       * mobile. The PCM record must indicate 14.4 k data
       * support.
       */
      if (FldGet(rr_data->mscap.datCap1, Dr14_4Sup) EQ 0)
        for_set_content_error (RRC_CHANNEL_MODE);
      break;

    case CM_EFR:          /* enhanced full rate */
      /*
       * check against the mobile capabilities
       * The PCM record must indicate support for
       * enhanced fullrate.
       */
      if (FldGet(rr_data->mscap.chnMode, EFRSupV2) EQ 0)
        for_set_content_error (RRC_CHANNEL_MODE);
      break;

    case CM_AMR:          /* speech full rate / half rate version 3 - AMR */
      /*
       * check against the mobile capabilities
       * The PCM record must indicate support for
       * speech version 3.
       */
      if ( (FldGet(rr_data->mscap.chnMode, AHS) EQ 0) AND (FldGet(rr_data->mscap.chnMode, AFS) EQ 0))
        for_set_content_error (RRC_CHANNEL_MODE);
      break;

    case CM_SIG_ONLY:    /* signalling only          */
      break;
    case CM_SPEECH:      /* speech full or halfrate  */
      /* If a speech mode is intended, but the MS is a data only unit, then error */
      if( !( rr_data->mscap.chnMode & ( spchSupV1m |  HR_EFRSupm |  EFRSupV3m | AHSm | AFSm )))
      {
        for_set_content_error (RRC_CHANNEL_MODE);
      }
      break;

    default:
      /*
       * a non-supported channel mode is detected.
       */
      for_set_content_error (RRC_CHANNEL_MODE);
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_cipher_mode_set  |
+--------------------------------------------------------------------+

  PURPOSE : check cipher mode setting information element.

*/

GLOBAL void for_check_cipher_mode_set (T_ciph_mode_set *ciph_mode_set)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_check_cipher_mode_set()");

  if (ciph_mode_set->algo_ident EQ 7)
  {
    /*
     * check whether the algorithm identifier contains
     * the reserved value.
     */
    for_set_content_error (RRC_INCORRECT_MSG);
  }

  /* 
   * If ciphering is set, check the ciphering algorithm against the mobile's
   * capabilities. Regard message as incorrect if the requested algorithm is
   * not supported by the mobile.
   */
  if (ciph_mode_set->sc EQ START_CIPH_YES) 
  {
    switch (ciph_mode_set->algo_ident)
    {
      case ALGO_A5_1:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_1)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_2:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_2)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_3:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_3)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_4:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_4)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_5:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_5)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_6:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_6)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      case ALGO_A5_7:
        if (!rr_data->ms_data.rf_cap.a5_bits.a5_7)
          for_set_content_error (RRC_INCORRECT_MSG);
        break;
      default: /* Reserved value received, already handled above */
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_frequency        |
+--------------------------------------------------------------------+

  PURPOSE : The function checks a frequency. The return value is TRUE
            if the channel number is a GSM channel number. The output
            variable range indicates whether the channel number is
            supported by the mobile depending on the frequency standard.

            Return Values :

            if (arfcn) is not a valid GSM Channel :-
                                            UNKNOWN_BAND_USED

            if (arfcn) is valid GSM Channel then depending upon (std) one of the
            4 values are returned :
                                            LOW_BAND_USED
                                            HIGH_BAND_USED
                                            EXT_BAND_USED
                                            INVALID_BAND_USED


*/

GLOBAL UBYTE for_check_frequency (USHORT channel)
{

  TRACE_FUNCTION ("for_check_frequency()");

  /*
   * check whether the channel number is a GSM channel number
   * the check is a little bit too simple, but it seems not
   * to be worst to check it.
   */
  if (channel >= HIGH_CHANNEL_EGSM)
  {
    return UNKNOWN_BAND_USED;
  }

  /*
   * a more efficient way of range checking for ARM
   * (according to application note 34, ARM DAI 0034A, January 1998)
   *
   * For the following code:
   * if (channel >= low_channel AND channel <= high_channel)
   *   bitposition = ...;
   *
   * exist the faster way to implemented this:
   * if ((unsigned)(channel - low_channel) <= (high_channel - low_channel)
   *   bitposition = ...;
   *
   * Future versions of the compiler will perform this optimization
   * automatically.
   *
   * We use the follwing macro:
   * #define  INRANGE(min, x, max)  ((unsigned)(x-min) <= (max-min))
   *
   */

  /*
   * depending on the frequency standard
   */
  switch (std)
  {
    case STD_900:
      /*
      if (channel >= LOW_CHANNEL_900 AND channel <= HIGH_CHANNEL_900)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
        return LOW_BAND_USED;
      }
      break;

    case STD_EGSM:
      /*
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (channel >= LOW_CHANNEL_900 AND channel <= HIGH_CHANNEL_900)
        *range = TRUE;
      else if (channel >= LOW_CHANNEL_EGSM AND channel <= HIGH_CHANNEL_EGSM)
        *range = TRUE;
      */
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
      {
        return EXT_BAND_USED;
      }
      break;

    case STD_1900:
      /*
      if (channel >= LOW_CHANNEL_1900 AND channel <= HIGH_CHANNEL_1900)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_1900,channel,HIGH_CHANNEL_1900))
      {
        return HIGH_BAND_USED;
      }
      break;

    case STD_1800:
      /*
      if (channel >= LOW_CHANNEL_1800 AND channel <= HIGH_CHANNEL_1800)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
      {
        return HIGH_BAND_USED;
      }
      break;

    case STD_DUAL:
      /*
      if (channel >= LOW_CHANNEL_900 AND channel <= HIGH_CHANNEL_900)
        *range = TRUE;
      else if (channel >= LOW_CHANNEL_1800 AND channel <= HIGH_CHANNEL_1800)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
      {
        return HIGH_BAND_USED;
      }
      break;

    case STD_DUAL_EGSM:
      /*
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (channel >= LOW_CHANNEL_900 AND channel <= HIGH_CHANNEL_900)
        *range = TRUE;
      else if (channel >= LOW_CHANNEL_1800 AND channel <= HIGH_CHANNEL_1800)
        *range = TRUE;
      else if (channel >= LOW_CHANNEL_EGSM AND channel <= HIGH_CHANNEL_EGSM)
        *range = TRUE;
      */
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
      {
        return HIGH_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
      {
         return EXT_BAND_USED;
      }
      break;

    case STD_850:
      /*
      if (channel >= LOW_CHANNEL_850 AND channel <= HIGH_CHANNEL_850)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
      {
         return LOW_BAND_USED;
      }
      break;

    case STD_DUAL_US:
      /*
      if (channel >= LOW_CHANNEL_850 AND channel <= HIGH_CHANNEL_850)
        *range = TRUE;
      else if (channel >= LOW_CHANNEL_1900 AND channel <= HIGH_CHANNEL_1900)
        *range = TRUE;
      */
      if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1900,channel,HIGH_CHANNEL_1900))
      {
         return HIGH_BAND_USED;
      }
      break;

#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
    case STD_850_1800:
      if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
      {
         return HIGH_BAND_USED;
      }
      break;
    
    case STD_900_1900:
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1900,channel,HIGH_CHANNEL_1900))
      {
        return HIGH_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
      {
         return EXT_BAND_USED;
      }
      break;

    case STD_850_900_1800:
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1800,channel,HIGH_CHANNEL_1800))
      {
        return HIGH_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
      {
         return EXT_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
      {
         return LOW_BAND_USED;
      }
      break;

    case STD_850_900_1900:
      if (channel EQ CHANNEL_0)
        channel = CHANNEL_0_INTERNAL;
      if (INRANGE(LOW_CHANNEL_900,channel,HIGH_CHANNEL_900))
      {
         return LOW_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_1900,channel,HIGH_CHANNEL_1900))
      {
        return HIGH_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_EGSM,channel,HIGH_CHANNEL_EGSM))
      {
         return EXT_BAND_USED;
      }
      else if (INRANGE(LOW_CHANNEL_850,channel,HIGH_CHANNEL_850))
      {
         return LOW_BAND_USED;
      }
      break;
#endif

  }

  return INVALID_BAND_USED;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_check_handov_cmd       |
+--------------------------------------------------------------------+

  PURPOSE : Content check of handover command message.

*/

typedef struct
{
  UBYTE        count_after;
  UBYTE        count_before;
  T_LIST       cell_chan_desc;
  T_LIST       hop_list_after;
  T_LIST       hop_list_before;
  T_f_range    freq_short_list_after;
  T_f_range    freq_list_after;
  T_f_range    freq_short_list_before;
  T_f_range    freq_list_before;
} T_HANDOVER_PARA;

GLOBAL void for_check_handov_cmd (T_DL_DATA_IND  * dl_data_ind,
                                  T_D_HANDOV_CMD * handov_cmd)
{
  GET_INSTANCE_DATA;
  T_HANDOVER_PARA * h_para;
  MALLOC (h_para, sizeof (T_HANDOVER_PARA));

  h_para->count_after = 0;
  h_para->count_before= 0;

  /*
   * clear several lists for building frequency hopping lists,
   */
  srv_clear_list (&h_para->hop_list_after);
  srv_clear_list (&h_para->hop_list_before);
  srv_clear_list (&h_para->cell_chan_desc);

  /*
   * check the content of the cell description information element.
   */
  for_check_cell_descr (&handov_cmd->cell_desc);

  /*
   * check the content of the channel description information element
   */
  for_check_channel_descr ((T_chan_desc *)
                           &handov_cmd->chan_desc_after);

  if (handov_cmd->v_freq_short_list_after)
  {
    /*
     * the message contains a frequency short list information element
     * for after starting time channel description
     */

    /*
     * clear the local variable.
     */

    
    memset (&h_para->freq_short_list_after, 0,
            sizeof (T_f_range));

    /*
     * copy the content of the information element
     */
    memcpy (&h_para->freq_short_list_after,
            &handov_cmd->freq_short_list_after,
            sizeof (BUF_freq_short_list_after));

    /*
     * build a frequency hopping list for layer 1.
     */
    for_create_channel_list (&h_para->freq_short_list_after,
                             &h_para->hop_list_after);
    /* check if all frequencies are in one band */
    if(! srv_check_frequencies_in_list (&h_para->hop_list_after))
    {
      for_set_content_error(RRC_FREQ_NOT_IMPL);
    }
  }

  if (handov_cmd->v_freq_list_after)
  {
    /*
     * the message contains a frequency list information element.
     */

    /*
     * clear the local variable
     */
/* Implements RR Clone findings #6 */
    for_frequency_list (&handov_cmd->freq_list_after,
                        &h_para->freq_list_after,
                        &h_para->hop_list_after);
  }

  if (handov_cmd->v_cell_chan_desc)
  {
    /*
     * the message contains a cell channel description information
     * element. Build a list for layer 1.
     */
    for_create_channel_list ((T_f_range *)&handov_cmd->cell_chan_desc,
                             &h_para->cell_chan_desc);
  }

  /*
   * if the message contains a channel mode information element,
   * check the channel mode against the mobile capabilities.
   */
  if (handov_cmd->v_chan_mode)
    for_check_channel_mode (handov_cmd->chan_mode);

  /*
   * if the message contains a channel mode information element which
   * indicates AMR, check the multirate configuration information element.
   * or if no channel mode is present but we have been using AMR before
   * check the supplied mulitrate configuration
   */
  if ( ( handov_cmd->v_chan_mode AND (handov_cmd->chan_mode    EQ CM_AMR)) OR
       (!handov_cmd->v_chan_mode AND (rr_data->sc_data.ch_mode EQ CM_AMR))
     )
  {
    if (handov_cmd->v_multirate_conf)
      for_check_multirate_conf( &handov_cmd->multirate_conf,
                               handov_cmd->chan_desc_after.chan_type);

    /*
     * check if the multirate configuration element is present
     */
    if ( (rr_data->sc_data.ch_mode NEQ CM_AMR) AND (!handov_cmd->v_multirate_conf) )
       for_set_content_error (RRC_CHANNEL_MODE);

    /*
     * The MultiRate Configuration IE shall be included in the case of full rate channel
     * to half rate channel handover. If not included in this case, the mobile station
     * shall behave as if the MultiRate Configuration IE was inconsistent.
     */

    if ( rr_data->sc_data.ch_mode                 EQ CM_AMR    AND
         rr_data->sc_data.chan_desc.chan_type     EQ TCH_F     AND
         handov_cmd->chan_mode                    EQ CM_AMR    AND
        (handov_cmd->chan_desc_after.chan_type    EQ TCH_H_S0  OR
         handov_cmd->chan_desc_after.chan_type    EQ TCH_H_S1) AND
         handov_cmd->v_multirate_conf             EQ 0             )
    {
      for_set_content_error (RRC_CHANNEL_MODE);
    }
  }

  if (handov_cmd->v_freq_chan_seq_after)
  {
    /*
     * the message contains a frequency channel sequence information
     * element. Build a frequency hopping list from this information.
     */
    if (!for_create_delta_list (&handov_cmd->freq_chan_seq_after,
                               &h_para->hop_list_after))
    {
      /*
       * set a content error if the frequency hopping list
       * contains channel numbers which are not supported.
       */
      for_set_content_error (RRC_FREQ_NOT_IMPL);
    }
  }

  if (handov_cmd->v_ciph_mode_set)
  {
    /*
     * if the message contains cipher mode information,
     * check the content.
     */
    for_check_cipher_mode_set (&handov_cmd->ciph_mode_set);
  }

  if (handov_cmd->chan_desc_after.hop)
  {
    /*
     * the channel description after starting time uses
     * frequency hopping. It is counted the number of
     * used variants for building a frequency hopping list
     * after starting time.
     */
    h_para->count_after = handov_cmd->v_freq_list_after +
                          handov_cmd->v_freq_short_list_after +
                          handov_cmd->v_mob_alloc_after +
                          handov_cmd->v_freq_chan_seq_after;

    /*
     * If no or more then one variant is defined,
     * RR detects a conditional error.
     */
    if (h_para->count_after NEQ 1)
      for_set_conditional_error ();

    /*
     * If the message contains a mobile allocation but no
     * cell channel description, it is not possible to
     * build a frequency hopping list: indicate a conditional
     * error.
     */
    if (handov_cmd->v_mob_alloc_after AND
        handov_cmd->v_cell_chan_desc EQ FALSE)
      for_set_conditional_error ();
  }
  else
  {
    /*
     * the frequency lists after aren't needed
     * for hopping after starting time (GSM 04.08 section 9.1.15.5)
     * now check whether the frequency lists after time are needed
     * for hopping before starting time (GSM 04.08 section 9.1.15.6)
     */
    if ( !( handov_cmd->v_start_time AND         /* there is a starting time */
            ( handov_cmd->v_chan_desc_before AND /* there is required hopping before time */
              handov_cmd->chan_desc_before.hop   /* explicitly, if using the chan_desc_after (see below) */
            )                                    /* hopping before is implicitly checked above */
          ) OR
         handov_cmd->v_freq_list_before OR       /* hopping before time (if present) uses one of its "own" lists */
         handov_cmd->v_freq_short_list_before OR
         handov_cmd->v_mob_alloc_before OR
         handov_cmd->v_freq_chan_seq_before )
    {
      /* the frequency lists after time aren't needed */
      handov_cmd->v_freq_list_after =
        handov_cmd->v_freq_short_list_after =
        handov_cmd->v_mob_alloc_after =
        handov_cmd->v_freq_chan_seq_after = 0;
    }
  }

  /*
   * In case of pseudo-synchronized handover it is
   * mandatory to have a time difference information
   * element, else set a conditional error.
   */
  if (handov_cmd->v_time_diff EQ FALSE AND
      handov_cmd->synch_ind.si EQ SYI_PSEUDO_SYNCH)
    for_set_conditional_error ();

  if (handov_cmd->v_start_time)
  {
    h_para->count_before = handov_cmd->v_chan_desc_before +
                           handov_cmd->v_freq_list_before +
                           handov_cmd->v_freq_short_list_before +
                           handov_cmd->v_mob_alloc_before +
                           handov_cmd->v_freq_chan_seq_before;

    if (h_para->count_before NEQ 0)
    {
      /*
       * The message contains a starting time information element.
       */
      if (handov_cmd->v_chan_desc_before EQ FALSE)
      {
        /*
         * if the message contains no channel description
         * before starting time element, use the
         * channel description after starting time instead.
         */
        memcpy (&handov_cmd->chan_desc_before,
                &handov_cmd->chan_desc_after,
                sizeof (T_chan_desc));
        handov_cmd->v_chan_desc_before = TRUE;
      }

      if (handov_cmd->chan_desc_before.hop)
      {
        /*
         * if the channel description before starting time
         * contains a frequency hopping list, count the
         * number of possible variants.
         */
        h_para->count_before = handov_cmd->v_freq_list_before +
                               handov_cmd->v_freq_short_list_before +
                               handov_cmd->v_mob_alloc_before +
                               handov_cmd->v_freq_chan_seq_before;

        switch (h_para->count_before)
        {
          case 0:
            /*
             * no before elements for hopping list then use the
             * elements of the after starting time.
             */
            if (h_para->count_after EQ 1)
            {
              /*
               * copy the frequency list after starting time
               * if available
               */
              memcpy (&handov_cmd->freq_list_before,
                      &handov_cmd->freq_list_after,
                      sizeof (T_freq_list));
              handov_cmd->v_freq_list_before = handov_cmd->v_freq_list_after;

              /*
               * copy the frequency short list after starting time if available
               */
              memcpy (&handov_cmd->freq_short_list_before,
                      &handov_cmd->freq_short_list_after,
                      sizeof (BUF_freq_short_list_after));
              handov_cmd->v_freq_short_list_before = handov_cmd->v_freq_short_list_after;

              /*
               * copy mobile allocation after starting time if available
               */
              memcpy (&handov_cmd->mob_alloc_before,
                      &handov_cmd->mob_alloc_after,
                      sizeof (T_mob_alloc_after));
              handov_cmd->v_mob_alloc_before = handov_cmd->v_mob_alloc_after;

              /*
               * copy frequency channel sequence after starting time if available
               */
              memcpy (&handov_cmd->freq_chan_seq_before,
                      &handov_cmd->freq_chan_seq_after,
                      sizeof (T_freq_chan_seq_after));
              handov_cmd->v_freq_chan_seq_before = handov_cmd->v_freq_chan_seq_after;
            }
            else
            {
              /*
               * more then one possibility to build a frequency hopping list.
               * This means an inconsistent message.
               */
              for_set_conditional_error ();
            }
            break;

          case 1:
            /*
             * Just one variant to build a frequency hopping list before
             * starting time.
             */
            break;

          default:
            /*
             * more then one possibility to build a frequency hopping list.
             * This means an inconsistent message.
             */
            for_set_conditional_error ();
            break;
        }
      }
    }
  }
  else
  { /* IEs are unnecessary */
    handov_cmd->v_chan_desc_before =
      handov_cmd->v_freq_list_before =
      handov_cmd->v_freq_short_list_before =
      handov_cmd->v_mob_alloc_before =
      handov_cmd->v_freq_chan_seq_before = 0;
  }

  /*
   * the message contains a mobile allocation but no cell channel description.
   * It is not possible to build a frequency hopping list.
   */
  if (handov_cmd->v_mob_alloc_before AND
      handov_cmd->v_cell_chan_desc EQ FALSE)
       for_set_conditional_error ();

  /*
   * If the message contains a channel description before starting time,
   * check the content.
   */
  if (handov_cmd->v_chan_desc_before)
    for_check_channel_descr ((T_chan_desc *)
                             &handov_cmd->chan_desc_before);

  if (handov_cmd->v_freq_short_list_before)
  {
    /*
     * build a frequency hopping list for the channel description before
     * starting time from the frequency short list information element.
     */

    /*
     * clear the local variable.
     */
    memset (&h_para->freq_short_list_before, 0,
            sizeof (T_f_range));

    /*
     * copy the content of the information element.
     */
    memcpy (&h_para->freq_short_list_before,
            &handov_cmd->freq_short_list_before,
            sizeof (BUF_freq_short_list_before));

    /*
     * build a list for layer 1.
     */
    for_create_channel_list (&h_para->freq_short_list_before,
                             &h_para->hop_list_before);
    /* check if all frequencies are in one band */
    if(! srv_check_frequencies_in_list (&h_para->hop_list_before))
    {
      for_set_content_error(RRC_FREQ_NOT_IMPL);
    }
  }

  if (handov_cmd->v_freq_list_before)
  {
    /* Implements RR Clone findings #7 */
    for_frequency_list (&handov_cmd->freq_list_before,
                        &h_para->freq_list_before,
                        &h_para->hop_list_before);
  }

  if (handov_cmd->v_freq_chan_seq_before)
  {
    /*
     * the message contains a frequency channel sequence element to
     * build the frequency hopping list.
     */
    if (!for_create_delta_list ((T_freq_chan_seq_after *)
                                &handov_cmd->freq_chan_seq_before,
                                &h_para->hop_list_before))
    {
      /*
       * set a content error if the frequency hopping list
       * contains channel numbers which are not supported.
       */
      for_set_content_error (RRC_FREQ_NOT_IMPL);
    }
  }

  /*
   * handle the message, configure layer 1 etc.
   */
  dat_for_handov_cmd (dl_data_ind, handov_cmd, &h_para->cell_chan_desc,
                      &h_para->hop_list_after, &h_para->hop_list_before);
  /*
   * de-allocate the dynamic memory
   */
  MFREE (h_para);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_create_channel_list    |
+--------------------------------------------------------------------+

  PURPOSE : The function creates a frequency hopping list from one of
            the following information elements according GSM 4.08:

            cell channel description
            frequency list
            frequency short list
            neighbour cell description

            The format identifier of the information element is defined as:

            FORMAT-ID, Format Identifier

            Bit Bit Bit Bit Bit    format notation
             8   7   4   3   2

             0   0   X   X   X     bit map 0
             1   0   0   X   X     1024 range
             1   0   1   0   0     512 range
             1   0   1   0   1     256 range
             1   0   1   1   0     128 range
             1   0   1   1   1     variable bit map

*/

GLOBAL void for_create_channel_list (T_f_range  * cha_list_descr,
                                     T_LIST     * cha_list)
{
  /*
   * set a pointer to the begin of the array
   */
  UBYTE  *cha_ptr = &cha_list_descr->
                    b_f[cha_list_descr->o_f>>3];

  TRACE_FUNCTION ("for_create_channel_list()");

  /*
   * clear result list
   */
  srv_clear_list (cha_list);

  if ((*cha_ptr & 0x80) EQ 0)
  {
    /*
     * Bitmap 0 format
     * only for GSM 900 or GSM 850 bands !!!!
     */
    switch (std)
    {
      case STD_900:
      case STD_EGSM:
      case STD_DUAL:
      case STD_DUAL_EGSM:
      case STD_850:
      case STD_DUAL_US:
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
      case STD_850_1800:
      case STD_900_1900:
      case STD_850_900_1800:
      case STD_850_900_1900:
#endif
        /*
         * clear the format identifier
         */
        *cha_ptr = *cha_ptr & 0x0F;

        TRACE_EVENT_WIN ("bitmap 0 format");
        /*
         * resulting list has bitmap 0 format !
         * copy only the content of the GSM 900 or GSM 850 channels
         * equal to the first 16 byte.
         */
        memcpy (&cha_list->channels[T_LIST_MAX_SIZE-16], cha_ptr, 16);
        break;

      default:
        /*
         * for PCS 1900 or DCS 1800 ignore the information element.
         */
        break;
    }
  }
  else
  {
    if ((*cha_ptr &0x8E) EQ 0x8C)
    {

/* Implements RR Clone findings #20 */
      for_decode_param_freq(128,cha_list_descr,cha_list);

    }

    if ((*cha_ptr &0x8E) EQ 0x8A)
    {
      /*
       * RANGE 256
       *
       * Use dynamic memory for calculation instead of global memory or stack.
       */
/* Implements RR Clone findings #20 */
      for_decode_param_freq(256,cha_list_descr,cha_list);

    }

    if ((*cha_ptr &0x8E) EQ 0x88)
    {
      /*
       * RANGE 512
       *
       * Use dynamic memory for calculation instead of global memory or stack.
       */
/* Implements RR Clone findings #21 */
      for_decode_param_freq(512,cha_list_descr,cha_list);

   
    }

    if ((*cha_ptr &0x88) EQ 0x80)
    {
      /*
       * RANGE 1024
       *
       * Use dynamic memory for calculation instead of global memory or stack.
       */
      UBYTE f0;

      SHORT *w;
      MALLOC (w, 257 * sizeof (USHORT));

      TRACE_EVENT_WIN ("range 1024 format");

      /*
       * get the f0 indicator. It indicates whether channel 0 is part
       * of the frequency hopping list or not.
       */
      ccd_decodeByte (cha_list_descr->b_f,
                      (USHORT)(cha_list_descr->o_f+5),
                      1, &f0);

      /*
       * decode the W-parameter
       */
      for_decode_param_1024 (w, cha_list_descr);

      /*
       * If indicated add channel 0 to the list
       */
      if (f0)
        srv_set_channel (cha_list, CHANNEL_0);

      /*
       * decode and set the remaining channel number according the
       * algorithm described in GSM 4.08.
       */
      for_decode_frequencies (1023, &w[0], cha_list, 0);

      /*
       * free the dynamic allocated memory.
       */
      MFREE (w);
    }

    if ((*cha_ptr &0x8E) EQ 0x8E)
    {
      /*
       * RANGE variable
       *
       * The format is similar to the bitmap 0 format. The
       * calculation starts from a base channel number svalue
       * instead of channel number 1.
       */
      ULONG  lvalue;
      USHORT svalue;
      UBYTE  bvalue;
      USHORT i;

      TRACE_EVENT_WIN ("range variable format");

      /*
       * get the first channel number
       */
      ccd_decodeLong (cha_list_descr->b_f, 7, 10, &lvalue);

      /*
       * copy lvalue to svalue to set the correct channel
       */
      svalue = (USHORT)lvalue;
      srv_set_channel (cha_list, svalue);

      for (i=1;i<112;i++)
      {
        /*
         * get the  value of the next bit
         */
        ccd_decodeByte (cha_list_descr->b_f,(USHORT)(i+16),1, &bvalue);

        if (bvalue)
        {
          /*
           * If the bit is set, set channel i+svalue
           */
          srv_set_channel (cha_list, (USHORT)for_modulo(i+svalue, 1024));
        }
      }
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_create_delta_list      |
+--------------------------------------------------------------------+

  PURPOSE : The function creates a frequency hopping list from the
            frequency channel sequence information element. This
            information element contains only GSM 900 channel numbers.

            The first channel number is stored in low_arfcn. The several
            increments are defined by the information element which are
            added to this starting frequency.

*/

static BOOL for_create_delta_list (T_freq_chan_seq_after *delta_list,
                                   T_LIST                *cha_list)
{
  USHORT i;
  UBYTE  result  = TRUE;
  USHORT delta;

  /*
   * set the first cannel number.
   */
  USHORT cha_num = delta_list->low_arfcn;

  TRACE_FUNCTION ("for_create_delta_list()");

  /*
   * Check whether it is a GSM 900 channel number
   */
  if (cha_num < LOW_CHANNEL_900 OR
    cha_num > HIGH_CHANNEL_900)
    result = FALSE;

  /*
   * clear the output parameter for the calculated frequency hopping list.
   */
  srv_clear_list (cha_list);

  /*
   * set the first channel number.
   */
  srv_set_channel (cha_list, cha_num);

  /*
   * for the 16 possible increments
   */
  for (i = 0; i < 16; i++)
  {
    /*
     * get the delta
     */
    delta = (USHORT) delta_list->inc_skip[i];

    /*
     * if delta is equal to 0, add 15 to the base, but
     * do not store in the output list.
     */
    if (! delta)
      cha_num += 15;
    else
    {
      /*
       * add the delta to the base
       */
      cha_num    += delta;

      /*
       * Check whether it is a GSM 900 channel number
       */
      if (cha_num < LOW_CHANNEL_900 OR
        cha_num > HIGH_CHANNEL_900)
        result = FALSE;

      /*
       * set the new frequency
       */
      srv_set_channel (cha_list, cha_num);
    }
  }

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param_1024      |
+--------------------------------------------------------------------+

  PURPOSE : The W-parameter in the 1024 range start from bit 6 of the
            information element. The following table indicate the number
            of W-parameters and their length in bits. A length of zero
            indicated the end of the table.

*/

static const T_W_PARAM param_1024[9] =
       {
         /*
          * length       count
          */
              10,          1,
               9,          2,
               8,          4,
               7,          8,
               6,          16,
               5,          32,
               4,          64,
               3,          128,
               0,          0
       };


static void for_decode_param_1024 (SHORT              *w,
                                   T_f_range          *cha)
{
  TRACE_FUNCTION ("for_decode_param_1024");

  /*
   * the algorithm for the several ranges is the same with different
   * tables. The W-parameter start with bit 6.
   */
  for_decode_param (param_1024, w, cha, 6);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param_512       |
+--------------------------------------------------------------------+

  PURPOSE : The W-parameter in the 512 range start from bit 7 of the
            information element. The following table indicate the number
            of W-parameters and their length in bits. A length of zero
            indicated the end of the table.

*/

static const T_W_PARAM param_512[10] =
       {
         /*
          *  length     count
          */
               10,        1,
                9,        1,
                8,        2,
                7,        4,
                6,        8,
                5,       16,
                4,       32,
                3,       64,
                2,      128,
                0,        0
       };


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param_256       |
+--------------------------------------------------------------------+

  PURPOSE : The W-parameter in the 256 range start from bit 7 of the
            information element. The following table indicate the number
            of W-parameters and their length in bits. A length of zero
            indicated the end of the table.

*/

static const T_W_PARAM param_256[10] =
       {
         /*
          *  length     count
          */
               10,        1,
                8,        1,
                7,        2,
                6,        4,
                5,        8,
                4,       16,
                3,       32,
                2,       64,
                1,      128,
                0,        0
       };



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param_128       |
+--------------------------------------------------------------------+

  PURPOSE : The W-parameter in the 128 range start from bit 7 of the
            information element. The following table indicate the number
            of W-parameters and their length in bits. A length of zero
            indicated the end of the table.

*/

static const T_W_PARAM param_128[9] =
       {
         /*
          *  length     count
          */
               10,        1,
                7,        1,
                6,        2,
                5,        4,
                4,        8,
                3,       16,
                2,       32,
                1,       64,
                0,        0
       };



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param           |
+--------------------------------------------------------------------+

  PURPOSE : The information element contains a list of W-parameter.
            The table param indicates how many W-parameter from each
            length shall be inside. The function converts the bitstream
            of the W-parameter to an array of W-parameter 16 bit values.

*/

static void for_decode_param     (const T_W_PARAM    *param,
                                  SHORT              *w,
                                  T_f_range          *cha,
                                  USHORT              initial_offset)
{
  UBYTE  end_detected = FALSE;
  USHORT w_index      = 0;
  USHORT offset       = cha->o_f + initial_offset;
  USHORT length       = cha->l_f - initial_offset;
  USHORT act_length   = param->length;
  USHORT act_counter  = param->count;
  ULONG  lvalue;
  UBYTE  bvalue;

  TRACE_FUNCTION ("for_decode_param()");

  /*
   * Until the end of the information element is detected.
   */
  while (!end_detected)
  {
    if (act_length > 8)
    {
      /*
       * the length of the next W-parameter is greater than eight bits
       * so use the ccd_decodeLong function.
       */
      ccd_decodeLong (cha->b_f, offset, act_length, &lvalue);
      w[w_index++] = (SHORT)lvalue;
    }
    else
    {
      /*
       * else use the ccd_decodeByte function to extract the W-parameter
       * from the bitstream.
       */
      ccd_decodeByte (cha->b_f, offset, act_length, &bvalue);
      w[w_index++] = (SHORT)bvalue;
    }

    /*
     * w = 0 is equal to end of list if it is not the w(0) !!!
     */
    if (w_index NEQ 1)
      if (w[w_index-1] EQ 0)
        end_detected = TRUE;

    /*
     * end of buffer is equal to end of list
     */
    if (length > act_length)
    {
      length -= act_length;
      offset += act_length;
    }
    else
      end_detected = TRUE;

    /*
     * all w parameter of one size read
     */
    if (--act_counter EQ 0)
    {
      param++;
      act_length   = param->length;
      act_counter  = param->count;
    }
    /*
     * End of parameter table
     */
    if ((act_length EQ 0) OR (length < act_length))
      end_detected = TRUE;
  }

  /*
   * add an end identifier
   */
  w[w_index++] = 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_frequencies     |
+--------------------------------------------------------------------+

  PURPOSE : The algorithm is according GSM 4.08 Annex J. It calculates
            a frequency hopping list from the W-parameter.

*/

static void for_decode_frequencies (SHORT     original_range,
                                   SHORT     *w,
                                   T_LIST    *f,
                                   SHORT     offset)
{
  SHORT g;
  SHORT k;
  SHORT j;
  SHORT index;
  SHORT n;
  SHORT range;

  TRACE_FUNCTION ("for_decode_frequencies()");

  for (k = 1; w[k-1]; k++)
  {
    /*
     * The next loop follows the tree from child to parent,
     * from the node of index K to the root (index 1). For each iteration the
     * node of index INDEX is tackled. The corresponding range is RANGE, and N
     * is the value of the element in the range defined by the node.
     *
     * The data are set to their initial values
     */
    index = k;
    n = w[index-1];
    g = for_get_generation (index);
    j = (1 << (g-1));
    range = original_range / j;

    while (index > 1)
    {
      /*
       * Due to the assumption that the original range is a power of two minus one,
       * the range for the parent node can be easily computed, and does not depend
       * upon whether the current node is a left or right child
       */
      g     = for_get_generation (index);
      j     = (1 << (g-1));
      range = 2 * range + 1;

      /*
       * Let us note J := 2 g-1 , g being the generation of node INDEX. We have J =
       * GREATEST_POWER_OF_2_LESSER_OR_EQUAL_TO(INDEX). The numbering used in the tree
       * is such that the nodes of index J to J + J/2 - 1 are left children, and the nodes
       * of index J/2 to J+J-1 are right children. Hence an easy test to
       * distinguish left and right children:
       */
      if (2 * index < 3 * j)
      {
        /*
         * The next computation gives the index of the parent node of the node of index
         * INDEX, for a left child :
         */
        index = index - j / 2;

        /*
         * The next formula is the inverse of the renumbering appearing in the encoding
         * for a left child. It gives the value of the parent node in the range defined
         * by the grand-parent node:
         */
        n = (SHORT)for_smodulo (n + w[index-1] + (range-1) / 2, range);
      }
      else
      {
        /*
         * The next computation gives the index of the parent node of the node of index
         * INDEX, for a right child :
         */
        index = index - j;

        /*
         * The next formula is the inverse of the renumbering appearing in the encoding
         * for a right child:
         */
        n = (SHORT)for_smodulo (n + w[index-1], range);
      }
    }

    /*
     * set the calculated channel number.
     */
    srv_set_channel (f, (USHORT)for_modulo (n+offset, 1024));
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_get_generation         |
+--------------------------------------------------------------------+

  PURPOSE : The function calculates the greatest power of 2 of the given
            value. The algorithm simply looks to the position of the
            highest bit.

*/

static USHORT for_get_generation (USHORT value)
{
  int result = 0;
  int i;


  /*
   * check all 16 bit positions.
   */
  for (i = 0; i < 16; i++)
  {
    /*
     * if bit is set, store the position.
     */
    if (value & 1)
      result = i + 1;

    /*
     * shift value to have the next bit for
     * comparision.
     */
    value = value >> 1;
  }

  /*
   * return the highest position.
   */
  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_modulo                 |
+--------------------------------------------------------------------+

  PURPOSE : A modulo calculation function. The standard C-Operator
            fails for negative values !  (e.g. -4 mod 6 is 2 and not 4).

*/

static LONG for_modulo (LONG a, LONG b)
{
  long result;

  /*
   * use standard C-Operator for calculation
   */
  result = a % b;

  /*
   * correct the result for negative values.
   */
  if (result < 0)
    result += b;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_smodulo                |
+--------------------------------------------------------------------+

  PURPOSE : Similar to the modulo operator, but 0 smod n is n and
            not 0. Same problem for negative values with the standard
            C-Operator.

*/

static LONG for_smodulo (LONG a, LONG b)
{
  long result;

  /*
   * use standard C-Operator for calculation
   */
  result = a % b;

  /*
   * correct the result for negative values.
   */
  if (result < 0)
    result += b;

  /*
   * special handling for result equal 0
   */
  if (result EQ 0)
    result = b;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_set_conditional_error  |
+--------------------------------------------------------------------+

  PURPOSE : set a conditional error.

*/

static void for_set_conditional_error (void)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_set_conditional_error()");

  switch (rr_data->ms_data.error.cs)
  {
    /* case RRC_INVALID_MAN_INFO: this value is currently never set */
    case RRC_INCORRECT_MSG:
      break;

    default:
      /*
       * set the conditional information element error.
       */
      rr_data->ms_data.error.cs = RRC_COND_IE_ERROR;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_set_content_error      |
+--------------------------------------------------------------------+

  PURPOSE : set a content error.

*/

GLOBAL void for_set_content_error (UBYTE value)
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION ("for_set_content_error()");

  switch (rr_data->ms_data.error.cs)
  {
    /* case RRC_INVALID_MAN_INFO: this value is currently never set */
    case RRC_COND_IE_ERROR:
    case RRC_INCORRECT_MSG:
      /*
       * Ignore a content error, if already an error with higher
       * priority has been detected.
       */
      break;

    default:
      /*
       * store the content error.
       */
      rr_data->ms_data.error.cs  = RRC_INCORRECT_MSG;
      rr_data->ms_data.error.val = value;
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_suspend_layer_2        |
+--------------------------------------------------------------------+

  PURPOSE : suspend layer 2 in case of assignment or handover command.

*/

GLOBAL void for_suspend_layer_2 (void)
{
  GET_INSTANCE_DATA;
  PALLOC (dl_suspend_req, DL_SUSPEND_REQ);

  TRACE_FUNCTION ("for_suspend_layer_2()");

  /*
   * set channel type and SAPI for layer 2.
   */
  dat_code_prr_channel (&dl_suspend_req->ch_type,
                        &dl_suspend_req->sapi,
                        rr_data->sc_data.chan_desc.chan_type);

  PSENDX (DL, dl_suspend_req);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_frequency_list         |
+--------------------------------------------------------------------+

  PURPOSE : This function creates a frequency list and channel list 

*/
LOCAL void for_frequency_list(T_freq_list *freq_list_starting_time_cmd,
                              T_f_range *freq_list_starting_time_para,
                              T_LIST *hop_list_starting_time)
{
  TRACE_FUNCTION("for_frequency_list()");

  memset (freq_list_starting_time_para, 0, sizeof (T_f_range));
  memcpy (freq_list_starting_time_para->b_f,
          freq_list_starting_time_cmd->flist,
          freq_list_starting_time_cmd->c_flist);
  freq_list_starting_time_para->l_f = 8*freq_list_starting_time_cmd->c_flist;

  /*
   * create a frequency list.
   */
  for_create_channel_list (freq_list_starting_time_para, hop_list_starting_time);

  /* check if all frequencies are in one band */
  if(! srv_check_frequencies_in_list (hop_list_starting_time))
  {
    for_set_content_error(RRC_FREQ_NOT_IMPL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_decode_param_freq      |
+--------------------------------------------------------------------+

  PURPOSE : This function decodes the frequency parameter from the channel
  description IE 

*/
LOCAL void for_decode_param_freq(USHORT  range ,T_f_range *cha_list_descr,
                                 T_LIST  *cha_list)
{
  SHORT *w;
  SHORT size_mul;
  const T_W_PARAM *param_ptr;

  TRACE_FUNCTION("for_decode_param_freq()");
  /*
   * RANGE 128/256/512
   *
   * Use dynamic memory for calculation instead of global memory or stack.
   */
  switch(range)
  {
    case 128:
      TRACE_EVENT_WIN ("range 128 format");
      size_mul = 129;
      param_ptr = param_128;
      
      break;
    case 256:
      TRACE_EVENT_WIN ("range 256 format");
      size_mul = 257;
      param_ptr = param_256;
      break;
    case 512:
      TRACE_EVENT_WIN ("range 512 format");
      size_mul = 257;
      param_ptr = param_512;
      break;
    default:
      size_mul = 0;
      param_ptr = NULL;
      break;
  }

  MALLOC (w, size_mul * sizeof (USHORT));
  for_decode_param (param_ptr, w, cha_list_descr, 7);
      
   /*
    * W[0] contains the first channel number
    */
  srv_set_channel (cha_list, w[0]);

   /*
    * decode and set the remaining channel number according the
    * algorithm described in GSM 4.08.
    */
  for_decode_frequencies ((range -1), &w[1], cha_list, w[0]);

   /*
    * free the dynamic allocated memory.
    */
  MFREE (w);
}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*[xgiridha] : EMR */
/*
+------------------------------------------------------------------------------
| Function    : for_process_si2quater
+------------------------------------------------------------------------------
| Description : Process SI-2quater information and store it in enh_para of serving cell
|               This is a multi instance message. check whether all the instances are
|               received or not before indicating to ALR and to GRR,if required.
|
| Parameters  : SI-2quater rest octets
|
+------------------------------------------------------------------------------
*/
GLOBAL BOOL for_process_si2quater(T_si_2qua_octets *p_si2q)
{  
  GET_INSTANCE_DATA;
  T_rr_enh_para     *p_cur = &rr_data->sc_data.emr_data_current;
  T_rr_enh_para     *p_temp = &rr_data->sc_data.emr_data_temp;    
  BOOL              send_enh_para = FALSE;  
  T_gprs_rep_prio   *p_rep = NULL;
  T_gprs_bsic       *p_bsic = NULL;

#if defined (TI_PS_FF_RTD) AND defined (REL99)
  UBYTE             i,j;
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  TRACE_FUNCTION ("for_process_si2quater");

  /* Step 1: Check if we received right BA_IND */
  if((rr_data->sc_data.ba_index NEQ NOT_PRESENT_8BIT) AND (p_si2q->ba_ind NEQ rr_data->sc_data.ba_index ))
  {
    /* re-read  si2qtr*/
    
    if(rr_data->sc_data.cd.si2quater_status EQ SI2QUATER_ABSENT OR
       rr_data->sc_data.cd.si2quater_status EQ SI2QUATER_ACQ_PENDING)
    {
    TRACE_EVENT("This is not the right BA_IND set that we have. So,ignore this SI-2quater");    
      rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_WRONG_BAIND;
      {
        PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
        mph_mon_ctrl_req->action = STOP_MON_BCCH;
        mph_mon_ctrl_req->si_to_read = UPDATE_SI2QUATER_AGAIN;
        PSENDX (PL, mph_mon_ctrl_req);
      }
    }
    else
    {
      TRACE_EVENT("This is not the right BA_IND set that we have.Re-read SI-2quater");    
      rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_PENDING;   
      {
        PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
        if ( rr_data->sc_data.cd.si2quater_pos EQ SI2QUATER_ON_EBCCH )
        {
          mph_mon_ctrl_req->action = START_MON_EBCCH;
        }
        else
        {
          mph_mon_ctrl_req->action = START_MON_NBCCH;
        }
        mph_mon_ctrl_req->si_to_read = UPDATE_SI2QUATER_AGAIN;
        PSENDX (PL, mph_mon_ctrl_req);
      }
    }
    return send_enh_para;
  }
  
    /* Step 2: Check report type. 
     IMPORTANT ASSUMPTION: We process only NC PARA (if present) if report type is Normal*/
  if( !(( (p_si2q->v_gprs_meas_para EQ TRUE) AND (p_si2q->gprs_meas_para.report_type EQ ENHANCED_MEAS)) OR
       ( (p_si2q->v_meas_para EQ TRUE) AND (p_si2q->meas_para.report_type EQ ENHANCED_MEAS )) ) )
  {
    /*check whether there are enhanced parameters and BA list already.
      If present then it means that report type is changing from
      Enhanced to Normal*/
    if ( p_cur->is_data_valid EQ TRUE  )
    {
      for_set_default_emr_data(p_cur);
#ifdef GPRS
      if (p_si2q->v_nc_meas_para EQ TRUE)
        for_store_nc_para(&p_si2q->nc_meas_para, p_cur);
#endif
      return TRUE; /*send enh para update to indicate change in report type and NC para*/
    }
    else
      return send_enh_para;
  }  
  
  /* Step 3: Check if we already have enh_para in current or temp
     and if there is change in parameters  or continuation of reception*/
  if(p_temp->is_data_valid EQ FALSE )  
  {
    /*This means we were not in the process of receiving. Check whether there
      is already information in current and if so, is there change in mp_change_mark*/
    if( (p_cur->mp_change_mark NEQ NOT_PRESENT_8BIT) AND
      (p_cur->mp_change_mark EQ p_si2q->mp_change_mark ) )
    {
      TRACE_EVENT("No change in Enhanced measurement parameters -ignore ");
      return TRUE;
    } 
    /* decode rest of the parameters*/      
    p_temp->is_data_valid = TRUE;
    rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
    p_temp->enh_para.ncc_permitted = rr_data->sc_data.cd.ncc_permitted;
    rr_data->sc_data.ba_index = p_si2q->ba_ind;
  }
  /*Note :If different values occur for the same parameter in different instances of a message, 
          the instance with the highest index shall be used (sec.3.4.1.2.1, 4.18)*/
  if ( (p_si2q->si2qua_index > rr_data->sc_data.prev_highest_index ) OR
        (rr_data->sc_data.prev_highest_index EQ NOT_PRESENT_8BIT) )
  {
    p_temp->mp_change_mark = p_si2q->mp_change_mark;
    p_temp->msg_count = p_si2q->si2qua_count; 
    for_update_emr_rep_para(p_si2q,p_temp); /* This is updation of parameters other than BSIC list*/    
    rr_data->sc_data.prev_highest_index = p_si2q->si2qua_index;
#if defined (TI_PS_FF_RTD) AND defined (REL99)
    if(p_si2q->v_rtdd)
      for_store_rtd_data(p_si2q,p_temp);
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

  }   
  
  if(p_si2q->v_gprs_rep_prio EQ TRUE)  
    p_rep = &p_si2q->gprs_rep_prio;    

  if(p_si2q->v_gprs_bsic EQ TRUE)
    p_bsic = &p_si2q->gprs_bsic;
  
  if (for_process_common_emr_data(p_rep,p_bsic,p_si2q->si2qua_index,
        rr_data->sc_data.ba_list_idle) )
  {
    rr_data->sc_data.enh_para_status = ENH_PARA_IDLE;
#if defined (TI_PS_FF_RTD) AND defined (REL99)
    /* store the RTD values received in all instances of SI2quater in permanent location */
    if(p_si2q->v_rtdd)
    {
      memcpy(p_cur,p_temp,MAX_NR_OF_NCELL*sizeof(USHORT));
      /* reset the temporary storage to RTD value not available */
      for(j = 0;j < MAX_NR_OF_NCELL; j++ )
      {
        p_temp->enh_para.enh_cell_list[j].v_rtd = FALSE;
        for(i = 0;i < MAX_NUM_OF_RTD_VALUES; i++)
          p_temp->enh_para.enh_cell_list[j].rtd[i]= RTD_NOT_AVAILABLE;
      }/*for*/
    }/*if*/
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

    if ( rr_data->sc_data.ba_list_idle EQ TRUE)
      send_enh_para = TRUE;
  }    
  return send_enh_para;
}

/*
+------------------------------------------------------------------------------
| Function    : for_update_emr_rep_para
+------------------------------------------------------------------------------
| Description : This function updates all the miscelaneous parameters related
|               to enhanced measurements. This doesn't include BSIC and Report priority
|               list.
| Parameters  : SI-2quater rest octets, target location where to store data
+------------------------------------------------------------------------------
*/
GLOBAL void for_update_emr_rep_para(T_si_2qua_octets *p_si2q,T_rr_enh_para *p_em)
{
#ifdef GPRS
  UBYTE         state;
  GET_INSTANCE_DATA;
#endif
      /*Decide whether to use GPRS part of measurement parameters or RR part*/
  TRACE_FUNCTION ("for_update_emr_rep_para");
 
#ifdef GPRS
  state = GET_STATE(STATE_GPRS);
  if( (state EQ GPRS_PIM_BCCH) OR (state EQ GPRS_PAM_BCCH) OR
    (state EQ GPRS_PTM_BCCH))
  {
    /*Use GPRS part*/    
    if(p_si2q->v_gprs_meas_para EQ TRUE )
    {
      p_em->grr_rep_type = p_si2q->gprs_meas_para.report_type;
      p_em->enh_para.inv_bsic_enabled = p_si2q->gprs_meas_para.inv_bsic_rep;      
      if ( p_si2q->gprs_meas_para.v_serv_band_rep EQ TRUE)
        p_em->enh_para.servingband_rep = p_si2q->gprs_meas_para.serv_band_rep;
      if( p_si2q->gprs_meas_para.v_mr EQ TRUE )
        p_em->enh_para.multiband_rep = p_si2q->gprs_meas_para.mr;
      p_em->enh_para.scale_order = p_si2q->gprs_meas_para.scale_ord;
      p_em->enh_para.rep_rate = p_si2q->gprs_meas_para.reporting_rate;
  
      /* Update reporting thresholds and reporting offsets*/
      if (p_si2q->gprs_meas_para.v_report_900 EQ TRUE)
      {
        p_em->enh_para.enh_rep_data[0].rep_offset = 
          p_si2q->gprs_meas_para.report_900.rep_offset_900;
        p_em->enh_para.enh_rep_data[0].rep_offset = 
          p_si2q->gprs_meas_para.report_900.th_rep_900;
      }
      if (p_si2q->gprs_meas_para.v_report_1800 EQ TRUE)
      {
        p_em->enh_para.enh_rep_data[1].rep_offset = 
          p_si2q->gprs_meas_para.report_1800.rep_offset_1800;
        p_em->enh_para.enh_rep_data[1].rep_offset = 
          p_si2q->gprs_meas_para.report_1800.th_rep_1800;
      }
      if (p_si2q->gprs_meas_para.v_report_400 EQ TRUE)
      {
        p_em->enh_para.enh_rep_data[2].rep_offset = 
          p_si2q->gprs_meas_para.report_400.rep_offset_400;
        p_em->enh_para.enh_rep_data[2].rep_offset = 
          p_si2q->gprs_meas_para.report_400.th_rep_400;
      }
      if (p_si2q->gprs_meas_para.v_report_1900 EQ TRUE)
      {
        p_em->enh_para.enh_rep_data[3].rep_offset = 
          p_si2q->gprs_meas_para.report_1900.rep_offset_1900;
        p_em->enh_para.enh_rep_data[3].rep_offset = 
          p_si2q->gprs_meas_para.report_1900.th_rep_1900;
      }
      if (p_si2q->gprs_meas_para.v_report_850 EQ TRUE)
      {
        p_em->enh_para.enh_rep_data[4].rep_offset = 
          p_si2q->gprs_meas_para.report_850.rep_offset_850;
        p_em->enh_para.enh_rep_data[4].rep_offset = 
          p_si2q->gprs_meas_para.report_850.th_rep_850;
      }      
    }
    /*Update Network Control (NC) parameters */
    if (p_si2q->v_nc_meas_para EQ TRUE )   
      for_store_nc_para(&p_si2q->nc_meas_para,p_em);    
  }
  else
  {  
#endif
    /* Use RR part */
    if(p_si2q->v_meas_para EQ TRUE )
    {
      p_em->rep_type = p_si2q->meas_para.report_type;
      p_em->enh_para.servingband_rep = p_si2q->meas_para.serv_band_rep;
    }    
#ifdef GPRS
  }
#endif
  return;
}

#ifdef GPRS
static void for_store_nc_para(T_nc_meas_para *p_nc,T_rr_enh_para *p_em)
{
  TRACE_FUNCTION ("for_store_nc_para");
  p_em->nc_para.nco = p_nc->nco;
  if(p_nc->v_nc_meas_struct EQ TRUE )
  {
    p_em->nc_para.is_valid = TRUE;
    p_em->nc_para.nc_non_drx   = p_nc->nc_meas_struct.nc_non_drx;
    p_em->nc_para.nc_rep_per_i = p_nc->nc_meas_struct.nc_rep_i;
    p_em->nc_para.nc_rep_per_t = p_nc->nc_meas_struct.nc_rep_t;
  }  
  return;
}
#endif

/*
+------------------------------------------------------------------------------
| Function    : for_set_default_emr_data
+------------------------------------------------------------------------------
| Description : This function performs the default intialization of emr data
| Parameters  : target location where to initialize
+------------------------------------------------------------------------------
*/
GLOBAL void for_set_default_emr_data(T_rr_enh_para  *p_em)
{
  GET_INSTANCE_DATA;
  UBYTE     i;
  
  TRACE_FUNCTION ("for_set_default_emr_data");
  
  memset(p_em, 0, sizeof(T_rr_enh_para));
  /*But there are some parameters whose default value is not 0, so we
    need to set them explicitly*/
  p_em->grr_rep_type = p_em->rep_type = NORMAL_MEAS; /*Default*/
  
#ifdef GPRS
  p_em->nc_para.nco = NCO_EMPTY; /*If GRR receives this value, it should not use
  entire nc parameters*/
  p_em->nc_para.is_valid = TRUE;
  p_em->nc_para.nc_non_drx = NC_0_48S;
  p_em->nc_para.nc_rep_per_i = NC_I_3_84S; /*see rr.aim for details*/
  p_em->nc_para.nc_rep_per_t = NC_I_61_44S;
#endif

  p_em->enh_para.scale_order = SCALE_0dB;
  p_em->enh_para.inv_bsic_enabled = FALSE;  
  p_em->enh_para.rep_rate = NORMAL_RATE;
  p_em->enh_para.servingband_rep = DEFAULT_SERV_BAND_REP; /*Default value (3) */  
  p_em->mp_change_mark = NOT_PRESENT_8BIT;
    
 /* Default initialization of Reporting thresholds and offsets */
  for ( i=0; i<MAX_NUM_BANDS; i++)
  {
    p_em->enh_para.enh_rep_data[i].rep_threshold = REP_THRESHOLD_NONE;
    p_em->enh_para.enh_rep_data[i].rep_offset = REP_OFFSET_0;
  }
  rr_data->sc_data.rep_bmp = 0;
  rr_data->sc_data.instance_bmp = 0;
  rr_data->sc_data.prev_highest_index = NOT_PRESENT_8BIT;
  memset(rr_data->sc_data.bsic_list,0,MAX_MULTI_INST * sizeof(T_bsic_list));
  return;
}

/*
+------------------------------------------------------------------------------
| Function    : for_send_enh_para
+------------------------------------------------------------------------------
| Description : This function calls the functions needed to send ENH PARA to GRR
|               and ALR, under suitable conditions.
| Parameters  : enh para structure of RR
+------------------------------------------------------------------------------
*/
GLOBAL void for_send_enh_para(T_rr_enh_para *p_src)
{
  UBYTE     rep_type = p_src->rep_type;
#ifdef GPRS
  GET_INSTANCE_DATA;
#endif
  TRACE_FUNCTION ("for_send_enh_para");
  /* GRR is updated only in PIM/PAM/PTM when PBCCH is absent
     ALR is updated always with enhanced measurement parameters,
     it can be decided in ALR whether to use this information while
     sending report depending on report type*/
 
#ifdef GPRS
  {
    UBYTE         state;
    state = GET_STATE(STATE_GPRS);
    if( (state EQ GPRS_PIM_BCCH) OR (state EQ GPRS_PAM_BCCH) OR
      (state EQ GPRS_PTM_BCCH))  
    {
      att_send_enh_para_to_grr(p_src);
      if (p_src->grr_rep_type EQ REP_TYPE_ENH)        
        rep_type = REP_TYPE_ENH;      
    }
  }
#endif
  attf_send_enh_para_to_alr(rep_type,&p_src->enh_para);  
  return;  
}

/*
+------------------------------------------------------------------------------
| Function    : for_perform_ba_bsic_mapping
+------------------------------------------------------------------------------
| Description : This function performs the BA list to BSIC list mapping, by taking BSIC list from
|               each instance of SI-2quater/MI-message and BA list. Here we store only the index into
|               BA list for ARFCN.  
| Parameters  : Input:
|               BSIC list from air message
|               Output:
|               Update Enhanced cell list with BA indices.
+------------------------------------------------------------------------------
*/
LOCAL void for_perform_ba_bsic_mapping(T_gprs_bsic *p_bsic, T_enh_para_struct *p_enh )
{  
  UBYTE                 i=0;
  UBYTE                 j;  
  UBYTE                 k=0;  
  
  TRACE_FUNCTION ("for_perform_ba_bsic_mapping");
  j = p_enh->num_valid_cells; /*If some cells were already filled from a previous instance,
  then this would be non-zero. We have to start updating this enhanced list from here*/
  if ( j >= MAX_NEIGHBOURCELLS )
    return; /* we cannot store any more neighbour cells, we only support GSM cells*/
  
  if (p_bsic->v_ba_start_bsic EQ TRUE )
    i = p_bsic->ba_start_bsic;
  else
    i = 0;
  
  /* Note : here we only store indices of ARFCNs*/
  /* Update from first BSIC, which is given outside the recursive structure*/
  p_enh->enh_cell_list[j].arfcn = i;  
  p_enh->enh_cell_list[j].bsic  = p_bsic->bsic;    
  j++;
  
  k = 0;
  if ( p_bsic->rem_bsic > 0) 
  {
    while ( (k < p_bsic->rem_bsic) AND j < MAX_NEIGHBOURCELLS )
    {
      if (p_bsic->bsic_struct[k].freq_scroll EQ SCROL_NO)      
        /* This means same frequency as before */
        p_enh->enh_cell_list[j].arfcn = i;        
      else       
        p_enh->enh_cell_list[j].arfcn = ++i;                       
      p_enh->enh_cell_list[j].bsic = p_bsic->bsic_struct[k].bsic;
      j++;
      k++;
    }
  }
  if ( j >= MAX_NEIGHBOURCELLS )
    p_enh->num_valid_cells = MAX_NEIGHBOURCELLS; /* we will not monitor more than 32 cells*/
  else
    p_enh->num_valid_cells = j;
  return;
}

/*
+------------------------------------------------------------------------------
| Function    : for_update_enh_cell_list
+------------------------------------------------------------------------------
| Description : This function updates the enhanced cell list with actual ARFCNs from
|               BA list. Note that this updation is always with respect to temp data base
|               and will be moved to current only if updation is successful.
|               If there are any indices pointing to unavailable ARFCN (due to SI-5ter)
|               then the updation is continued by marking that these need to be updated
|               later. In this case we still consider, updation to be successful.  
| Parameters  : Input:
|               Stored BA list               
|               Output:
|                TRUE  - Updation successful
|                FALSE - Updation unsuccessful
+------------------------------------------------------------------------------
*/
GLOBAL BOOL for_update_enh_cell_list (USHORT *p_list)
{
  GET_INSTANCE_DATA;
  T_rr_enh_para *p_enh = &rr_data->sc_data.emr_data_temp;
  T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
  UBYTE         i;
  UBYTE         j;
  ULONG         rep_bmp = rr_data->sc_data.rep_bmp;
  ULONG         ba_bitmap = 0;

  TRACE_FUNCTION ("for_update_enh_cell_list");
  /*First check whether BSIC list was given or it's only updation
   of parameters*/
  if (p_enh->enh_para.num_valid_cells EQ 0)
  {
    /*There should be enhanced cell list atleast in current*/
    if ( (p_cur->is_data_valid EQ TRUE) AND (p_cur->enh_para.num_valid_cells > 0) )
    {
      /*This copy is actually over head, but done to maintain uniformity in 
        copying after this function returns*/
      p_enh->enh_para.num_valid_cells = p_cur->enh_para.num_valid_cells;
      memcpy(p_enh->enh_para.enh_cell_list,p_cur->enh_para.enh_cell_list, 
        p_cur->enh_para.num_valid_cells * sizeof(T_enh_cell_list));
      return TRUE;
    }
    else
    {
      TRACE_EVENT("BSIC Information required for enhanced reporting, is missing");
      return FALSE;
    }
  }

  /*Under the conditions when this function is called, the 
  enhanced cell list contains the index of the ARFCN in neighbourcell list*/
  for (i = 0; i < p_enh->enh_para.num_valid_cells; i++ )
  {
    j = (UBYTE)p_enh->enh_para.enh_cell_list[i].arfcn;
    p_enh->enh_para.enh_cell_list[i].rep_priority = (UBYTE) ((rep_bmp >> i) & 1);
    if ( p_list[j] NEQ NOT_PRESENT_16BIT )
    {
      ba_bitmap |= (1 << j);
      p_enh->enh_para.enh_cell_list[i].arfcn = p_list[j];
    }      
    else
    {
      /*This indicates that we received BSIC for an unknown ARFCN. This ambiguity
        may be resolved once we receive SI-5ter*/
      p_enh->ba2bsic_map_pending |= ( 1<<i); /*set the bit of the corresponding index of enh-cell-list*/
      /*Index to BA(list) is retained as it is and actual ARFCN from this index is copied after receiving
        SI-5ter*/
    }      
  }

  /*verify whether the mapping is complete*/
  i = 0;
  while (p_list[i] NEQ NOT_PRESENT_16BIT)
  {
    if  (( (ba_bitmap >> i) & 1) NEQ TRUE )
    {
      /* This indicates that there are still some ARFCN in BA list
       that doesn't have a corresponding BSIC */
      TRACE_EVENT("BA - BSIC list mapping is incorrect : IGNORE the message ");
      return FALSE;
    }
    i++;
  }
  
  return TRUE;
}

/*
+------------------------------------------------------------------------------
| Function    : for_store_rep_priority
+------------------------------------------------------------------------------
| Description : This function stores the report priority temporarily in the form
|               of bit map.   
| Parameters  : Input:
|               Reporting prioirty list from air message               
+------------------------------------------------------------------------------
*/
LOCAL void for_store_rep_priority ( T_gprs_rep_prio *p_rp)
{
  GET_INSTANCE_DATA;
  ULONG         rep_bmp = 0;
  UBYTE         i;

  TRACE_FUNCTION ("for_store_rep_priority");
  /*IMPORTANT ASSUMPTION: since there is no index information
    associated explicitly with REPORT PRIORITY information, it's
    more likely that entire report priority info is given in one instance*/
  /* Only GSM neighbor cells upto 32 are supported */
  if (p_rp->num_cells > (MAX_NEIGHBOURCELLS -1) )
    p_rp->num_cells = MAX_NEIGHBOURCELLS -1;

  for ( i = 0; i < p_rp->num_cells; i++ )
  {
    rep_bmp |= ( (p_rp->rep_priority[i] & 1) << i );
  }
  rr_data->sc_data.rep_bmp = rep_bmp;
  return;  
}

/*
+------------------------------------------------------------------------------
| Function    : for_process_common_emr_data
+------------------------------------------------------------------------------
| Description : This function processes the EMR parameters that are common in
|               SI-2quater and MI messages.   
| Parameters  : Input:
|               Reporting prioirty list from air message 
|               BSIC list from air message
|               Enhanced para structure, where the data needs to be updated
|               message instance number
|               Output
|               TRUE  - processing successful
|               FALSE - processing unsuccessful              
+------------------------------------------------------------------------------
*/
GLOBAL BOOL for_process_common_emr_data (T_gprs_rep_prio *p_rep,
                                             T_gprs_bsic *p_bsic,
                                             UBYTE  msg_index,
                                             BOOL ba_available)
{
  GET_INSTANCE_DATA;
  T_bsic_list       *p_bl = &rr_data->sc_data.bsic_list[0];
  T_rr_enh_para     *p_enh = &rr_data->sc_data.emr_data_temp;
    
  TRACE_FUNCTION ("for_process_common_emr_data");
  /*check for duplication of instance */
  if ( ((rr_data->sc_data.instance_bmp >> msg_index) & (0x01)) EQ TRUE )
    return FALSE; /*This instance was already received, so need to process
    it again*/

  /*set the bit of the received instance in the bit map */
  rr_data->sc_data.instance_bmp |= (1 << msg_index);
  
  /*Store report priority: Ref. sec.3.4.1.2.1.5, 4.18
  Report Priority information can be received in one instance of the MEASUREMENT INFORMATION message*/
  if(p_rep NEQ NULL)  
    for_store_rep_priority(p_rep);  

  if(p_bsic NEQ NULL )
  {
    /*BSIC list is available, store it to perform BA-BSIC mapping at the end */    
    p_bl[msg_index].is_valid  = TRUE;
    p_bl[msg_index].bsic_info = *p_bsic;    
  }    
  
  /*check whether all the instances are received or not
    Number of 1's in instance_bmp should equal msg_count+1 
     Eg: if msg_count = 3, instance_bmp = 0000 0000 0000 1111 = 15
         2^(3+1)-1 = 15 */  
  if ( rr_data->sc_data.instance_bmp EQ ( (1 << (p_enh->msg_count+1))-1) )  
  {
    UBYTE     i;
    /*all the instances are received, perform updations from temp --> current */

    /* BA-BSIC mapping makes sense only if enhanced measurement reporting is enabled*/
    if ((p_enh->rep_type EQ ENHANCED_MEAS) OR (p_enh->grr_rep_type EQ ENHANCED_MEAS) )
    {    
      for ( i= 0; i <= p_enh->msg_count; i++ )
      {
        if ( p_bl[i].is_valid EQ TRUE )
          for_perform_ba_bsic_mapping(&p_bl[i].bsic_info, &p_enh->enh_para);
      }

      if ( ba_available EQ TRUE )
      {
        /*First we update the enhanced cell list in the temp and then store
          into current, if the updation is successful*/
        if ( for_update_enh_cell_list( rr_data->act_ncell_list) EQ TRUE)
        {
          rr_data->sc_data.emr_data_current = *p_enh;        
          memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
          for_set_default_emr_data(p_enh);
          return TRUE;
        }
        else
        {
          /*Updation of enhanced cell list is unsuccesful, so flush temp
            and ignore the entire message */
          for_set_default_emr_data(p_enh);
          return FALSE;
        }
      }
    }
    else 
    {
      /*If we recieved SI2q without Enhanced para, Update the CM value in the emr_current*/
      rr_data->sc_data.emr_data_current.mp_change_mark = p_enh->mp_change_mark;      
      /*After cell reselection the first si2qtr received must be processed,
      for this is_data_valid flag should be reset */
      rr_data->sc_data.emr_data_current.is_data_valid = TRUE;
      for_set_default_emr_data(p_enh);
    }
    return TRUE;
  }
  return FALSE;
}

GLOBAL void for_mon_si2quater_req(UBYTE action)
{  
  PALLOC(mph_mon_ctrl_req, MPH_MON_CTRL_REQ );
  TRACE_FUNCTION ("for_mon_si2quater_req");
  mph_mon_ctrl_req->action = action;
  mph_mon_ctrl_req->si_to_read = UPDATE_SI2QUATER;
  PSENDX (PL, mph_mon_ctrl_req);
  return;
}

/*
+------------------------------------------------------------------------------
| Function    : for_update_ba_ind
+------------------------------------------------------------------------------
| Description : 
|               
| Parameters  : Input:
|                            
+------------------------------------------------------------------------------
*/
GLOBAL void for_update_ba_ind (UBYTE index, UBYTE ba_ind)
{
  GET_INSTANCE_DATA;
  if( index EQ SC_INDEX )
  {
    T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
    if ( rr_data->sc_data.ba_index NEQ ba_ind)
    {
      rr_data->sc_data.ba_index  = ba_ind;
      /*Received SI-2 on serving cell: this indicates there is change in
      SI-2 ==> store an indication that SI-2quater also has to be configured.
      Once the BA list is received completely, then we will configure ALR for receiving
      SI-2quater*/
      if (rr_data->sc_data.cd.si2quater_status NEQ SI2QUATER_ABSENT)
        rr_data->sc_data.cd.si2quater_status = SI2QUATER_CONFIGURE;
      if ( p_cur->is_data_valid )
      {
        memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
        for_set_default_emr_data(p_cur);
        /*Indicate to GRR and ALR that current enhanced para are not valid*/
        for_send_enh_para(p_cur); 
      }             
    }         
  }         
  else
    rr_data->cr_data.ba_index  = ba_ind;
}

/*
+------------------------------------------------------------------------------
| Function    : for_check_and_configure_si2quater
+------------------------------------------------------------------------------
| Description : 
|               
| Parameters  : Input:
|                            
+------------------------------------------------------------------------------
*/
GLOBAL void for_check_and_configure_si2quater (UBYTE index)
{
  GET_INSTANCE_DATA;
  T_rr_enh_para *p_temp = &rr_data->sc_data.emr_data_temp;
  T_rr_enh_para *p_cur = &rr_data->sc_data.emr_data_current;
  
  if ( index EQ SC_INDEX)
  {
    UBYTE     si2_read = (SYS_INFO_2_READ | SYS_INFO_2BIS_READ |SYS_INFO_2TER_READ );
    rr_data->sc_data.ba_list_idle = FALSE; /*This makes sure that we wait till SI-2,2bis/2ter
                                            are received before configuring L1 for SI-2quater,
                                            when it is on E-BCCH.If it's on N-BCCH, SI-2quater
                                            will be stored in temp database, till BA(BCCH) is complete*/
    
    if ( ((rr_data->sc_data.cd.sys_info_read ) & si2_read) EQ si2_read )
    {
      /*This indicates that all required SI-2 have been read*/
      rr_data->sc_data.ba_list_idle = TRUE;
      rr_data->sc_data.ba_list_ded = FALSE;
      /*configure SI-2quater if either configuration is pending or previously
        acquired and needs fresh acquisition due to change in SI-2/2bis*/
      if ( (rr_data->sc_data.cd.si2quater_status NEQ SI2QUATER_ABSENT  ) AND
           (rr_data->sc_data.cd.si2quater_status NEQ SI2QUATER_ACQ_PENDING) )
      {        
        if (p_cur->is_data_valid EQ TRUE)
        {
          memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
          for_set_default_emr_data(p_cur);
          /*Indicate to GRR and ALR that current enhanced para are not valid*/
          for_send_enh_para(p_cur);           
        }
        /*configure ALR for acquiring SI-2quater if it is scheduled on E-BCCH.
          If it's on N-BCCH, it would have been acquired already or we might
          be in the process of acquiring*/ 
        if ( rr_data->sc_data.cd.si2quater_pos EQ SI2QUATER_ON_EBCCH )
        {
          for_mon_si2quater_req(START_MON_EBCCH);
          rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_PENDING;        
        }
        else
        {
          if((rr_data->sc_data.cd.sys_info_read & SYS_INFO_2QUATER_READ) NEQ SYS_INFO_2QUATER_READ) /* SI 2qtr not yet read */  
          {
            rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_PENDING;  
            if((rr_data->sc_data.cd.sys_info_read & ALL_SYS_INFO_READ) EQ ALL_SYS_INFO_READ)
            {
              /*All other sys info has been read*/              
              for_mon_si2quater_req(START_MON_NBCCH); 
            }
            /*else
             {
                There are other SI that have not been read yet on Normal BCCH, just continue to read normal BCCH 
             }
            */

          }/*if*/
        }
      }
    }
  }
  
  if ( rr_data->sc_data.enh_para_status EQ ENH_PARA_DEDICATED)
  {
    /*The enhanced parameters are from previous state: reset them */
    memset (rr_data->sc_data.rep_count, NOT_PRESENT_8BIT, MAX_NEIGHBOURCELLS);
    for_set_default_emr_data(p_cur);
    for_set_default_emr_data(p_temp); 
    for_send_enh_para(p_cur);
    rr_data->sc_data.enh_para_status = ENH_PARA_INVALID_STATE;
  }
  /*When BA(BCCH) is ready, check whether there are enhanced parameters in
   temp that needs attention*/
  if ( (rr_data->sc_data.ba_list_idle EQ TRUE ) AND
       ( rr_data->sc_data.enh_para_status EQ ENH_PARA_IDLE) AND
       (p_temp->is_data_valid EQ TRUE))
  {
  /* This means enhanced parameters were received before BA list - so 
    update the enhanced list with actual ARFCN and update current  EMR data*/
    if ( for_update_enh_cell_list( rr_data->act_ncell_list) EQ TRUE)    
    {
      *p_cur = *p_temp;         
      for_send_enh_para(p_temp);
    }        
    /*Reset temporary, irrespective of whether updation is succesful or not*/
    for_set_default_emr_data(p_temp);
    if ((rr_data->sc_data.cd.si2quater_status EQ SI2QUATER_ACQ_PENDING) OR 
        (rr_data->sc_data.cd.si2quater_status EQ SI2QUATER_ACQ_WRONG_BAIND))
    {
      for_mon_si2quater_req(STOP_MON_BCCH);
      rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_COMP;
    }        
  } 

  return;
}
#endif

#if defined (TI_PS_FF_RTD) AND defined (REL99)
/*
+------------------------------------------------------------------------------
| Function    : for_store_rtd_data
+------------------------------------------------------------------------------
| Description : This function stores the rtd parameters received in si2quarter.
| Parameters  : RTD information/data,target enh para struct where we store RTD para
+------------------------------------------------------------------------------
*/
GLOBAL void for_store_rtd_data(T_si_2qua_octets *p_si2q,T_rr_enh_para *p_temp)
{
  T_rtdd *rtdd_struct= &p_si2q->rtdd;
dat_update_common_rtd_struct(rtdd_struct,p_temp);
} /* end for_store_rtd_data() */
#endif /* #if defined (TI_PS_FF_RTD) AND defined (REL99) */

#endif

/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  ALR_CBCH
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
|  Purpose :  This Modul defines the SDL process CBCH_control.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_CBCH_C
#define ALR_CBCH_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"

#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif
#include "alr.h"
#include "alr_em.h"

/*==== EXPORT =====================================================*/
/*==== PRIVAT =====================================================*/
LOCAL void   cb_read_cbch               (UBYTE  flags);
LOCAL void   cb_check_ci_lai_plmn       (void);
LOCAL void   cb_remove_message          (USHORT i);
LOCAL void   cb_first_block             (UBYTE  status);
LOCAL UBYTE* cb_create_schedule_entries (UBYTE* msg_ptr, BOOL read_old_msg);
LOCAL void   cb_create_schedule_map     (UBYTE  old_cb_msg);
LOCAL UBYTE  cb_check_old_msg           (void);
LOCAL UBYTE  cb_check_data_coding_scheme(UBYTE  dcs_id);
LOCAL UBYTE  cb_check_message_id        (USHORT msg_id);
LOCAL void   cb_store_cbch_msg          (void);
LOCAL void   cb_error_ind               (void);
LOCAL void   cb_cbch_message            (UBYTE  length);
LOCAL void   cb_null_message            (void);
LOCAL void   cb_schedule_message        (void);
LOCAL void   cb_next_scheduled_msg      (void);
LOCAL UBYTE  cb_sat_check_message_id    (USHORT msg_id);
LOCAL void   cb_sat_cbch_message        (UBYTE length);
LOCAL void cb_stop_next_blocks (void);
LOCAL void cb_read_next_3_blocks(void);
void set_cbch_state( UBYTE extended, UBYTE new_state);
UBYTE get_cbch_state( UBYTE extended);

#if defined(FF_HOMEZONE)
LOCAL UBYTE cb_check_homezone (UBYTE length);
#endif  /* FF_HOMEZONE */

LOCAL void cb_alr_trace_msghead (T_CBMSG_HEADER *ph);

/*
 * support indication of geographical scope change to MMI
 * it is important that these defitions match the GS definition from GSM 03.41
 * the MMI uses this also to compare with real CBCH messages!
 */
#define CB_GEOGR_CHANGE_NONE 0x00
#define CB_GEOGR_CHANGE_CELL 0xC0
#define CB_GEOGR_CHANGE_LAC  0x80
#define CB_GEOGR_CHANGE_PLMN 0x40

#define CB_GEOGR_SCOPE_CELLI 0x00
#define CB_GEOGR_SCOPE_PLMN  0x01
#define CB_GEOGR_SCOPE_LAC   0x02
#define CB_GEOGR_SCOPE_CELLN 0x03

#define CBCH_NO_MSG           0
#define CBCH_NORM_MSG         1
#define CBCH_SCHEDULE_MSG     2
#define CBCH_SAT_MSG          3
#define CBCH_LAST_BLOCK    0x10

#define CBCH_BLOCK_0          0
#define CBCH_BLOCK_1          1
#define CBCH_BLOCK_2          2
#define CBCH_BLOCK_3          3
#define CBCH_FIRST_SCHED_MSG  8
#define CBCH_NULL_MSG      0x0F
#define CBCH_SCHED            2

#define MAX_UPDATE_NO   16
#define MAX_UPDATE_NO_DIFF    8

#if 1 || defined (WIN32)
#define TRACING
#endif

#define CB_SERIAL_NO(ph)            ((USHORT)\
                                    ((((T_CBMSG_HEADER*)(ph))->serial_no1)<<8)+\
                                    (((T_CBMSG_HEADER*)(ph))->serial_no2))

#define CB_GS_CODE(ph)              (((UBYTE)(((T_CBMSG_HEADER*)(ph))->serial_no1))>>6)

#define CB_MSG_CODE(ph)             ((USHORT)\
                                    (((((T_CBMSG_HEADER*)(ph))->serial_no1)&0x3f)<<4)+\
                                    ((((T_CBMSG_HEADER*)(ph))->serial_no2)>>4))

#define CB_UPDATE_NO(ph)            ((((T_CBMSG_HEADER*)(ph))->serial_no2)&0x0f)

#define CB_MESSAGE_ID(ph)           ((USHORT)\
                                    ((((T_CBMSG_HEADER*)(ph))->msg_id3)<<8)+\
                                    (((T_CBMSG_HEADER*)(ph))->msg_id4))
#if 0
#define CB_MESSAGE_ID(ph)           ((((USHORT)(((T_CBMSG_HEADER*)(ph))->msg_id3))<<8)+\
                                      ((USHORT)(((T_CBMSG_HEADER*)(ph))->msg_id4)))

#endif /* 0 */
#define CB_DCS(ph)                  (((T_CBMSG_HEADER*)(ph))->dcs)

#define CB_PAGE(ph)                 (((T_CBMSG_HEADER*)(ph))->page>>4)
#define CB_PAGETOTAL(ph)            (((T_CBMSG_HEADER*)(ph))->page&0x0f)


#if defined (TRACING)
#define ALR_TRACE_CB(a)             TRACE_EVENT("CBCH: " a)
#define ALR_TRACE_CB_MODUS(m)       TRACE_EVENT_P1("CBCH: modus=%d", m)
#define ALR_TRACE_CB_ERROR(e)       TRACE_EVENT_P1("CBCH: error=%d", e)
#define ALR_TRACE_CB_BLOCK_NO(b)    TRACE_EVENT_P2("CBCH: block=%02x no=%u", b, b&0x0f)
#define ALR_TRACE_CB_MSGHEAD(ph)    cb_alr_trace_msghead(ph)

#define ALR_TRACE_CB_MSGID(m)         TRACE_EVENT_P1("CBCH: ID=%u", m)
#define ALR_TRACE_CB_START_READ(f)    TRACE_EVENT_P1("CBCH: Start Read %d", f)
#define ALR_TRACE_CB_SCHEDULE(a1,a2)  TRACE_EVENT_P2("CBCH: schedule i=%d result=%d", a1,a2)
#define ALR_TRACE_CB_HZREQ(a1,a2)     TRACE_EVENT_P2("CBCH: Homezone ID=%u, t=%u", a1,a2)
#define ALR_TRACE_CB_SCOPE_CHANGE(i,s)  TRACE_EVENT_P2 ("CBCH: Geogr. scope change %u, deleting msg of scope %u",\
                                          i,s);
#else  /* TRACING */
#define ALR_TRACE_CB(a)
#define ALR_TRACE_CB_MODUS(m)
#define ALR_TRACE_CB_ERROR(e)
#define ALR_TRACE_CB_BLOCK_NO(b)
#define ALR_TRACE_CB_MSGID(m)
#define ALR_TRACE_CB_START_READ(f)
#define ALR_TRACE_CB_SCHEDULE(a1,a2)
#define ALR_TRACE_CB_HZREQ(a1,a2)
#endif  /* TRACING */

/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/

#if 0
#if 0
#define TRACE_ALIGN_P(ps,m,t)  TRACE_EVENT_P5(" %08x %3u.%u %08x=%u " \
                                "T_" #t " " #m, \
                                (BYTE*)&(m), (BYTE*)&(m)-(BYTE*)ps, \
                                sizeof (m),
                                sizeof (m) > 1 ? (USHORT)(m) : (UBYTE)(m), m \
                              )
#else
#define TRACE_ALIGN_P(ps,m,t)  TRACE_EVENT_P4 (" %08x %3u.%u %08x " \
                                "T_" #t " " #m, \
                                (BYTE*)&(m), (BYTE*)&(m)-(BYTE*)ps, \
                                sizeof (m), m\
                                )
#endif
int test_alignment (T_CBMSG_HEADER *ph)
{
  TRACE_ALIGN_P (ph, ph->serial_no1, CBMSG_HEADER);
  TRACE_ALIGN_P (ph, ph->serial_no2, CBMSG_HEADER);
  TRACE_ALIGN_P (ph, ph->msg_id3, CBMSG_HEADER);
  TRACE_ALIGN_P (ph, ph->msg_id4, CBMSG_HEADER);
  TRACE_ALIGN_P (ph, ph->dcs, CBMSG_HEADER);
  TRACE_ALIGN_P (ph, ph->page, CBMSG_HEADER);
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_init                    |
+--------------------------------------------------------------------+

  PURPOSE : Initialize CBCH Control Process.
*/

GLOBAL void cb_init (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  ALR_TRACE_CB ("cb_init()");
  alr_data->state[STATE_NORMAL_CBCH] = CBCH_NULL;
  alr_data->state[STATE_EXT_CBCH] = CBCH_NULL;
  pcbch->modus   = NOT_PRESENT_8BIT;
  pcbch->old_cbch_msg.cnt = 0;
#if defined(FF_HOMEZONE)
  pcbch->homezone = CBCH_HZ_OFF;
  pcbch->hz_timeout = 0;
#endif  /* FF_HOMEZONE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_stop                    |
+--------------------------------------------------------------------+

  PURPOSE : Stop CBCH Control Process.

            cb_stop will be called in the following three scenarios - 
            1. Moving from IDLE to DEDICATED state 
            2. ALR has received HomeZone information. 
            3. MMI indicates stop reading CBCH. 

            In all three cases ALR need to forward this to L1.
            Eventhough the homezone request is pending we need to stop 
            the CBCH reading.
*/

GLOBAL void cb_stop (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE normal_cbch, extended_cbch;

  normal_cbch = GET_STATE(STATE_NORMAL_CBCH);
  extended_cbch = GET_STATE(STATE_EXT_CBCH);

  if ( 
        (normal_cbch NEQ CBCH_NULL AND normal_cbch NEQ CBCH_IDLE) 
        OR
        (extended_cbch NEQ CBCH_NULL AND extended_cbch NEQ CBCH_IDLE) 
     )
  {
    /*
     * if CBCH process is really active, initialize
     * CBCH process and stop CBCH process in layer 1.
     */
    pcbch->msg_type = CBCH_NO_MSG;

    SET_STATE(STATE_NORMAL_CBCH, CBCH_IDLE);
    SET_STATE(STATE_EXT_CBCH, CBCH_IDLE);

    ALR_EM_STOP_CBCH_READING;

    ma_cb_stop_cbch_req();
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_start                   |
+--------------------------------------------------------------------+

  PURPOSE : Re-configure layer 1 and go back to CBCH reading without
            scheduling information.
*/

LOCAL void cb_start_no_drx_reading (UBYTE config)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  if (config)
  {
    /*
     * configure layer 1
     */
    PALLOC (config_cbch, MPHC_CONFIG_CBCH_REQ);

    /*
     * copy the stored configuration received earlier from RR and send to layer 1.
     */
    memcpy (config_cbch, &pcbch->mphc_config_cbch, sizeof (T_MPHC_CONFIG_CBCH_REQ));
    ma_cb_config_cbch(config_cbch);
  }

  /*
   * start reading in layer 1 for normal and extended CBCH.
   * The initial mode is no Scheduling information available.
   */
  cb_read_cbch (CBCH_NO_SCHED | CBCH_READ_NORM);
  cb_read_cbch (CBCH_NO_SCHED  | CBCH_READ_EXT);

  SET_STATE(STATE_NORMAL_CBCH, CBCH_NO_DRX);
  SET_STATE(STATE_EXT_CBCH, CBCH_NO_DRX);
  
#if defined(FF_HOMEZONE)
  if (pcbch->homezone EQ CBCH_HZ_IDLE)
  {
    pcbch->homezone = CBCH_HZ_PENDING;
    /* start timer new */
    if (pcbch->hz_timeout)
      TIMERSTART(TIM_HOMEZONE, pcbch->hz_timeout);
    ALR_TRACE_CB ("HOMEZONE is pending");
  }
#endif /* FF_HOMEZONE */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_start                   |
+--------------------------------------------------------------------+

  PURPOSE : Start CBCH Control Process.
*/

GLOBAL void cb_start (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  if (    (GET_STATE(STATE_NORMAL_CBCH) EQ CBCH_IDLE)
      AND (     (pcbch->modus NEQ NOT_PRESENT_8BIT)
            OR  (pcbch->sat_enabled)
#if defined(FF_HOMEZONE)
            OR  (pcbch->homezone EQ CBCH_HZ_IDLE)
#endif /* FF_HOMEZONE */
          )
      )
  {
    /*
     * if a definition of needed message identifiers
     * and data coding schemes is available, then configure
     * layer 1
     */
    ALR_TRACE_CB ("restart");
    cb_start_no_drx_reading (TRUE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : TIL_CBCH                   |
| STATE   : code                ROUTINE : cb_mph_cbch_req            |
+--------------------------------------------------------------------+

  PURPOSE : Configuration of the layer 1 process for CBCH.

            The compile-switch is not defined with the standard TI-layer 1.

            But there shall be a little modification possible (and it is
            done by some source code customer of layer 1), which does not
            need any storing in ALR. Then the compile switch must be set

  REMARK  : Setting of compile switch has problems with function cbch_start ???

*/

GLOBAL void cb_mph_cbch_req  (T_cbch * cbch)
{
  GET_INSTANCE_DATA;
  USHORT i;
  T_CBCH_DATA      *pcbch = &alr_data->cbch_data;
  T_cbch_desc      *pcbch_desc = &pcbch->mphc_config_cbch.cbch_desc;
  T_chan_sel       *pchan_sel = &pcbch_desc->chan_sel;
  T_cbch_freq_list *pcbch_freq_list = &pcbch->mphc_config_cbch.cbch_freq_list;

  ALR_TRACE_CB ("MPH_CBCH_REQ");

  ALR_EM_CONFIGURE_CBCH_CHANNEL;

#if defined(FF_HOMEZONE)
  if (pcbch->homezone NEQ CBCH_HZ_OFF)
    pcbch->homezone = CBCH_HZ_IDLE; /* starts HZ request again */
#endif /* FF_HOMEZONE */

  /*   check which messages have to be deleted
   *   even if the new cell has CBCH disabled! We still need to
   *   to delete the corresponding messages and inform the MMI!
   */
  if (pcbch->modus EQ CBCH_ACCEPT)
    cb_check_ci_lai_plmn ();

  if (cbch->stat EQ STAT_ACT)
  {
    /*
     * A CBCH channel is available
     */

#if !defined (L1_STORES_CBCH_CONFIG)
    /*
     * Layer 1 does not store any CBCH configuration (default)
     * first set the channel configuration
     */
    if (cbch->h EQ H_FREQ)
    {
      /*
       * with frequency hopping, set hopping flag, maio, hsn and hopping list.
       */
      pchan_sel->h = HOPPING;
      pchan_sel->rf_channel.maio = cbch->maio;
      pchan_sel->rf_channel.hsn  = cbch->hsn;

      i=0;
      pcbch_freq_list->rf_chan_cnt=0;
      while ((i < 64) AND (cbch->ma[i] NEQ NOT_PRESENT_16BIT))
      {
        pcbch_freq_list->rf_chan_no.radio_freq[i] = ARFCN_TO_L1(cbch->ma[i]);
        i++;
        pcbch_freq_list->rf_chan_cnt++;
      }
    }
    else
    {
      /*
       * Layer 1 uses a union definition. This is not applicable for the Condat-SAP model.
       * Therefore the rf channel number is configured by maio and hsn. In general a Avoid the union !!!
       */
      pchan_sel->h = NO_HOPPING;
      pchan_sel->rf_channel.maio = (UBYTE)(ARFCN_TO_L1(cbch->arfcn) & 0xFF);
      pchan_sel->rf_channel.hsn  = (UBYTE)((ARFCN_TO_L1(cbch->arfcn) >> 8) & 0xFF);
      memset (pcbch_freq_list, 0, sizeof (T_cbch_freq_list));
    }

    /*
     * configure the CBCH channel type (SDCCH/8 or SDCCH/4)
     */
    if (cbch->ch < CH_SDCCH_8_0)
      pcbch_desc->channel_type = CH_SDCCH_4;
    else
      pcbch_desc->channel_type = CH_SDCCH_8;

    pcbch_desc->sub_channel   = 0;    /* not relevant */
    pcbch_desc->timeslot_no  = cbch->tn;
    pcbch_desc->tsc          = cbch->tsc;

#else
    /*
     * Compile option set: L1 stores actual CBCH configuration.
     * So it is not necessary to store the configuration in ALR.
     * Just parse the configuration to Layer 1.
     */
    {
      PALLOC (config_cbch, MPHC_CONFIG_CBCH);

      if (cbch->h EQ H_FREQ)
      {
        /*
         * with frequency hopping, set hopping flag, maio, hsn and hopping list.
         */
        config_cbch->cbch_desc.chan_sel.h = HOPPING;
        config_cbch->cbch_desc.chan_sel.rf_channel.maio = cbch->maio;
        config_cbch->cbch_desc.chan_sel.rf_channel.hsn  = cbch->hsn;

        i=0;
        config_cbch->cbch_freq_list.rf_chan_cnt=0;
        while (cbch->ma[i] NEQ NOT_PRESENT_16BIT AND i < 64)
        {
          config_cbch->cbch_freq_list.rf_chan_no.hop_chan[i] = ARFCN_TO_L1(cbch->ma[i]);
          i++; config_cbch->cbch_freq_list.rf_chan_cnt++;
        }
      }
      else
      {
        /*
         * Layer 1 uses a union definition. This is not applicable for the Condat-SAP model.
         * Therefore the rf channel number is configured by maio and hsn. In general a Avoid the union !!!
         */
        config_cbch->cbch_desc.chan_sel.h = NO_HOPPING;
        config_cbch->cbch_desc.chan_sel.rf_channel.maio = ARFCN_TO_L1(cbch->arfcn) & 0xFF;
        config_cbch->cbch_desc.chan_sel.rf_channel.hsn  = (ARFCN_TO_L1(cbch->arfcn) >> 8) & 0xFF;
        memset (&config_cbch->cbch_freq_list, 0, sizeof (T_cbch_freq_list));
      }

      /*
       * configure the CBCH channel type (SDCCH/8 or SDCCH/4)
       */
      if (cbch->ch < CH_SDCCH_8_0)
        config_cbch->cbch_desc.channel_type = CH_SDCCH_4;
      else
        config_cbch->cbch_desc.channel_type = CH_SDCCH_8;

      config_cbch->cbch_desc.sub_channel    = 0;    /* not relevant */
      config_cbch->cbch_desc.timeslot_no    = cbch->tn;
      config_cbch->cbch_desc.tsc            = cbch->tsc;

      ma_cb_config_cbch(config_cbch);
    }
#endif

    if (    (pcbch->modus NEQ NOT_PRESENT_8BIT)
         OR (pcbch->sat_enabled)
#if defined(FF_HOMEZONE)
         OR (pcbch->homezone EQ CBCH_HZ_IDLE)
#endif /* FF_HOMEZONE */
       )
    {
      /*
       * MMI has requested data
       * Start with NO_DRX mode
       * Read each first CBCH block
       */
#if defined (L1_STORES_CBCH_CONFIG)
      cb_start_no_drx_reading (FALSE);
#else  /* L1_STORES_CBCH_CONFIG */
      ALR_TRACE_CB ("switch on, send config");
      cb_start_no_drx_reading (TRUE);
#endif  /* L1_STORES_CBCH_CONFIG */
    }
    else
    {
      UBYTE   *si3 = &alr_data->ma_data.sys_info_3[0];
      UBYTE   *lac = &alr_data->cbch_data.old_cid_plmn_lac[0];

      ALR_TRACE_CB ("switch on, wait for MMI");
      /*
       * wait for MMI request
       */
      SET_STATE(STATE_NORMAL_CBCH, CBCH_IDLE);
      SET_STATE(STATE_EXT_CBCH, CBCH_IDLE);
      /*
       * update old_cid_plmn_lac
       */
      memcpy (lac, &si3[2], 7);
    }
  }
  else
  {
    /*
     * The new configuration indicates no CBCH, so stop the CBCH process in Layer 1
     */
    ALR_TRACE_CB ("switch off");

    cb_stop();
    SET_STATE(STATE_NORMAL_CBCH, CBCH_NULL);
    SET_STATE(STATE_EXT_CBCH, CBCH_NULL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : AKR_CBCH                   |
| STATE   : code                ROUTINE : cb_mmi_cbch_req            |
+--------------------------------------------------------------------+

  PURPOSE : MMI sends a definition of the expected message identifier
            and data coding schemes for the CBCH messages.

*/

GLOBAL void cb_mmi_cbch_req (T_MMI_CBCH_REQ *cbch)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  ALR_TRACE_CB ("MMI req");

#if defined(FF_HOMEZONE)
  if (cbch->modus NEQ CBCH_HOMEZONE)
  {
    pcbch->modus = cbch->modus;
    TRACE_EVENT_P1 ("CBCH: homezone=%u", pcbch->homezone);
  }
  else if (cbch->msg_id[0] EQ NOT_PRESENT_16BIT)
  {
    /*
     * switch off automatically homezone request
     * - homezone state to off
     * - clear old message
     * - stop timer
     */
     pcbch->msg_hz_id = NOT_PRESENT_16BIT;
     pcbch->homezone = CBCH_HZ_OFF;
     TIMERSTOP (TIM_HOMEZONE);
  }
  else
  {
    /*
     * switch on automatically homezone request
     * - set homezone state to idle whatever are before
     * - set new homezone id
     * - clear old message
     */
    pcbch->msg_hz_id = cbch->msg_id[0];
    pcbch->dcs_hz_id = cbch->dcs_id[0];
    pcbch->hz_timeout = cbch->dcs_id[1]*1000;
    ALR_TRACE_CB_HZREQ (pcbch->msg_hz_id,
      pcbch->hz_timeout);
    if (pcbch->homezone EQ CBCH_HZ_OFF)
    {
      UBYTE cbch_state = GET_STATE (STATE_NORMAL_CBCH);
      pcbch->homezone = CBCH_HZ_IDLE;
      if (cbch_state NEQ CBCH_NULL)
      {/* CBCH system is prepared */
        if (cbch_state EQ CBCH_IDLE)
        { /* start timer only if CBCH system is prepared */
          TIMERSTART (TIM_HOMEZONE, 1000);
        }
        else
        { /* CBCH system is already active -> wait for HZ message */
          pcbch->homezone = CBCH_HZ_PENDING;
          TIMERSTART (TIM_HOMEZONE, pcbch->hz_timeout);
          ALR_TRACE_CB ("HOMEZONE is pending");
        }
      }
    }
    return;
  }
#else  /* FF_HOMEZONE */
  pcbch->modus = cbch->modus;
#endif  /* FF_HOMEZONE */
  /*
   * Storing of Parameter and clearing of
   * the received CB memory
   */
  memcpy (&pcbch->msg_id[0], cbch->msg_id, sizeof (pcbch->msg_id));
  memcpy (&pcbch->dcs_id[0], cbch->dcs_id, sizeof (pcbch->dcs_id));

#if !defined(NTRACE)
{
  int i;

  ALR_TRACE_CB_MODUS(cbch->modus);

  for (i=0;
    (i < MAX_IDENTS) AND (pcbch->msg_id[i] NEQ NOT_PRESENT_16BIT);
    i++)
  {
    TRACE_EVENT_P2 ("CBCH: id[%u]=%u", i, pcbch->msg_id[i]);
  }
  for (i=0;
    (i < MAX_IDENTS) AND (pcbch->dcs_id[i] NEQ NOT_PRESENT_8BIT);
    i++)
  {
    TRACE_EVENT_P2 ("CBCH:dcs[%u]=%u", i, pcbch->dcs_id[i]);
  }
}
#endif /* !NTRACE */

  /* old messages should only be erased when MMI turns CBCH off */
  if (pcbch->modus EQ MMI_CBCH_STOP)
  {
    /* also erases the count of old message */
    memset (&pcbch->old_cbch_msg, 0, sizeof (T_CB_MSG));
    memset (&pcbch->msg[0], 0, CBCH_MSG_SIZE);
  }

  /*
    TRACE_EVENT_P2 ("CBCH: STATE_MA=%u STATE_CBCH=%u", GET_STATE(STATE_MA), GET_STATE (STATE_CBCH));
  */

#ifdef GPRS
  if ((GET_STATE(STATE_MA) EQ MA_IDLE AND !alr_data->gprs_data.pbcch) OR
      (alr_data->gprs_data.pbcch AND alr_data->gprs_data.pim))
#else
  if (GET_STATE(STATE_MA) EQ MA_IDLE)
#endif
  {
    /*
     * layer 1 is only configured, if ALR is in idle state, else the configuration
     * is delayed until the cell is selected.
     */
    switch (GET_STATE (STATE_NORMAL_CBCH))
    {
      case CBCH_NULL:
        /*
         * no CBCH channel is available
         */
        break;

      case CBCH_IDLE:
        if (pcbch->modus EQ MMI_CBCH_STOP AND
            !pcbch->sat_enabled)
        {
          /*
           * CBCH download is not enabled and the MMI indicates stop of
           * the CBCH channel, then stop CBCH process in layer 1.
           */
          cb_stop();
        }
        else
        {
          /*
           * new definitions of message identifier or data coding schemes
           * then re-configure layer 1 and go back to CBCH reading without
           * scheduling information.
           */
          cb_start_no_drx_reading (TRUE);
        }
        break;

      default:
        /*
         * we already received a MMI CBCH REQ before
         */
        if (pcbch->modus EQ MMI_CBCH_STOP AND
            !pcbch->sat_enabled)
        {
          /*
           * CBCH download is not enabled and the MMI indicates stop of
           * the CBCH channel, then stop CBCH process in layer 1.
           */
          cb_stop();
          SET_STATE(STATE_NORMAL_CBCH, CBCH_IDLE);
          SET_STATE(STATE_EXT_CBCH, CBCH_IDLE);
        }
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : TIL_CBCH                   |
| STATE   : code                ROUTINE : cb_mmi_sat_cbch_req        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a list of identifiers for SIM TOOLKIT
            CBCH Download.

*/

GLOBAL void cb_mmi_sat_cbch_req (T_MMI_SAT_CBCH_DWNLD_REQ * cbch_req)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  unsigned i;
  ALR_TRACE_CB ("MMI SAT req");

  /*
   * clear the list of needed message identifier for CBCH download
   * and fill it with the definiton of MMI.
   */
   memset (pcbch->msg_id_sat, NOT_PRESENT_16BIT,
           sizeof(pcbch->msg_id_sat));
   memcpy (pcbch->msg_id_sat, cbch_req->msg_id,
           sizeof(cbch_req->msg_id[0])*(cbch_req->count));

   /*
    * CBCH download is disabled
    */
  pcbch->sat_enabled = FALSE;
  for (i = 0; i < cbch_req->count; i++)
  {
    if (pcbch->msg_id_sat[i] NEQ NOT_PRESENT_16BIT)
    {
      pcbch->sat_enabled = TRUE;
      break;
    }
  }
  vsi_o_error_ttrace ("HM cb_mmi_sat_cbch_req(), sat_enabled = %d, modus = %d", 
                      pcbch->sat_enabled, pcbch->modus);
#ifdef GPRS
  if ((GET_STATE(STATE_MA) EQ MA_IDLE AND !alr_data->gprs_data.pbcch) OR
      (alr_data->gprs_data.pbcch AND alr_data->gprs_data.pim))
#else
  if (GET_STATE(STATE_MA) EQ MA_IDLE)
#endif
  {
          /*
           * Current state is compatible with enabling of CBCH reading, 
           * configure L1 to read CBCH. Otherwise configuration is delayed
           * until ALR reaches a compatible state.
           */    
    switch (GET_STATE (STATE_NORMAL_CBCH))
    {
      case CBCH_NULL:
            /*
             * No CBCH channel is available.
             */
        break;

      default:
      {
        if (pcbch->modus EQ MMI_CBCH_STOP AND !pcbch->sat_enabled)
        {
            /* No CBCH to read. Stop reading CBCH in L1. */
          cb_stop();
        }
        else
        {

            /* 
             * CBCH channel is available. 
             * Configure layer 1 for catching the CBCH download messages.
             */
          PALLOC (config_cbch, MPHC_CONFIG_CBCH_REQ);
          memcpy (config_cbch, &pcbch->mphc_config_cbch, sizeof (T_MPHC_CONFIG_CBCH_REQ));
          ma_cb_config_cbch(config_cbch);
          cb_read_cbch (CBCH_NO_SCHED | CBCH_READ_NORM);
          cb_read_cbch (CBCH_NO_SCHED | CBCH_READ_EXT);
          SET_STATE(STATE_NORMAL_CBCH, CBCH_NO_DRX);
          SET_STATE(STATE_EXT_CBCH, CBCH_NO_DRX);         
        }
      }
      break;
    } /* End of switch (GET_STATE (STATE_NORMAL_CBCH)) */
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : TIL_CBCH                   |
| STATE   : code                ROUTINE : cb_data_ind                |
+--------------------------------------------------------------------+

  PURPOSE : A CBCH message is received.

*/

GLOBAL void cb_data_ind (T_MPHC_DATA_IND  *cbch_ind)
{
  GET_INSTANCE_DATA;
  UBYTE block_no;
  UBYTE block_description;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE * expected_seq_number  = &(pcbch->expected_seq_number[0]);

  /*
   * check whether the CBCH message has been received on the
   * normal CBCH channel or the extended CBCH channel
   */
  if (cbch_ind->tc >= 4)
  {
    pcbch->msg_is_extended = CBCH_READ_EXT;
    expected_seq_number  = &(pcbch->expected_seq_number[1]);
  }
  else if(cbch_ind->tc < 4)
    pcbch->msg_is_extended = CBCH_READ_NORM;

  if (cbch_ind->error_flag EQ VALID_BLOCK)
  {
    /*
     * if the CBCH block is valid, calculate the block number
     */
    block_description = cbch_ind->l2_frame.content[0];
    block_no = (UBYTE)(block_description & 0x0F);
    if (block_no < CBCH_NULL_MSG)
    {
      ALR_TRACE_CB_BLOCK_NO (block_description);
    }
    TRACE_EVENT_P2("expected_block_number = %d, schedule_length =%d", (*expected_seq_number), pcbch->schedule_length[0]);
    switch(block_no)
    {
      case CBCH_BLOCK_0:
        memcpy (&pcbch->msg[0], &cbch_ind->l2_frame.content[1], CBCH_BLOCK_SIZE);
#if defined(FF_HOMEZONE)
        if (!cb_check_homezone ((UBYTE)(CBCH_BLOCK_SIZE*(block_no+1))))
#endif  /* FF_HOMEZONE */
          cb_first_block(block_description);
        break;

      case CBCH_BLOCK_1:
      case CBCH_BLOCK_2:
      case CBCH_BLOCK_3:
       /*
         * check for sequence number 
         */
       if(*expected_seq_number NEQ block_no)
       {
         ALR_TRACE_CB("Sequence error");
         cb_error_ind();        
       }
       else
       {
         memcpy (&pcbch->msg[CBCH_BLOCK_SIZE*(block_no & 3)],
          &cbch_ind->l2_frame.content[1], CBCH_BLOCK_SIZE);

         (*expected_seq_number)++;
         if( *expected_seq_number > CBCH_BLOCK_3 )
           *expected_seq_number = CBCH_BLOCK_0;
         
         
         if ( (block_description & CBCH_LAST_BLOCK)  OR  (block_no EQ CBCH_BLOCK_3)  )
         {
           /*
             * last block of msg
             */
           if(pcbch->msg_type EQ CBCH_NORM_MSG)
             /*
               * +1, because parameter is length not offset
               */
             cb_cbch_message((UBYTE)(CBCH_BLOCK_SIZE*(block_no+1)));
           else if(pcbch->msg_type EQ CBCH_SCHEDULE_MSG)
             cb_schedule_message();
           else if(pcbch->msg_type EQ CBCH_SAT_MSG)
             cb_sat_cbch_message((UBYTE)(CBCH_BLOCK_SIZE*(block_no+1)));
         }
       }
       break;

      case CBCH_FIRST_SCHED_MSG:
        /*
         * first SCHEDULE block
         */

        ALR_TRACE_CB ("SCHEDULE");
        memcpy (&pcbch->msg[0],  &cbch_ind->l2_frame.content[1], CBCH_BLOCK_SIZE);
        cb_read_next_3_blocks();

        pcbch->msg_type = CBCH_SCHEDULE_MSG;
        break;

      case CBCH_NULL_MSG:
        /*
         * NULL Message
         */

        /*
        ALR_TRACE_CB ("null msg");
        */
        cb_null_message ();
        break;

      default:
        ALR_TRACE_CB ("unknown");
        cb_error_ind();
        break;
    }

    ALR_EM_RECEIVE_CBCH_MESSAGE;

  }
  else
  {
    /*
     * Invalid block
     */
    /*
    ALR_TRACE_CB_ERROR(cbch_ind->error_flag);
    */

    pcbch->msg_type = CBCH_NO_MSG;
    cb_error_ind ();
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_first_block             |
+--------------------------------------------------------------------+

  PURPOSE : Reception of the first CBCH block.

*/

LOCAL void cb_first_block (UBYTE status)
{
  GET_INSTANCE_DATA;
  UBYTE  result;
  T_CBMSG_HEADER  *p_cbh;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  /*
   * extract message id and data coding scheme from the message
   */
  p_cbh = (T_CBMSG_HEADER*)(&pcbch->msg[0]);

  ALR_TRACE_CB_MSGHEAD (p_cbh);

  /*
   * check the incoming message:
   * 1. matches the message identifier
   * 2. matches the data coding scheme
   * 3. is this message not received yet
   */
  result =  cb_check_message_id (CB_MESSAGE_ID(p_cbh));
  result += cb_check_data_coding_scheme (CB_DCS(p_cbh));
  result += cb_check_old_msg ();

  if (result EQ 3)
  {
    /*
     * all checks are passed
     */
    if (! pcbch->sat_enabled OR
        ! cb_sat_check_message_id(CB_MESSAGE_ID(p_cbh)))
    {
      /*
       * it is not a CBCH download message or
       * CBCH download is not enabled
       */
      if (status & CBCH_LAST_BLOCK)
      {
        /*
         * the first is the only block, then forward it
         * to MMI.
         */
        cb_cbch_message (CBCH_BLOCK_SIZE*(CBCH_BLOCK_0+1));
      }
      else
      {
        /*
         * store the block and request the three remaining
         * blocks in layer 1.
         */
        pcbch->msg_type = CBCH_NORM_MSG;
        cb_read_next_3_blocks();
      }
    }
    else
    {
      /*
       * It is a CBCH download message and CBCH download is enabled.
       */
      if (status & CBCH_LAST_BLOCK)
      {
        /*
         * the first block is the only block, so forward the
         * message to MMI.
         */
        cb_sat_cbch_message (CBCH_BLOCK_SIZE*(CBCH_BLOCK_0+1));
      }
      else
      {
        /*
         * store the data and request the three remaining blocks.
         */
        pcbch->msg_type = CBCH_SAT_MSG;
        cb_read_next_3_blocks();
      }
    }
  }
  else
  {
    /*
     * the message has not passed the checks.
     * So it can be only a CBCH download message.
     */
    if(! pcbch->sat_enabled OR
       ! cb_sat_check_message_id(CB_MESSAGE_ID(p_cbh)))
    {
      /*
       * SAT is not enabled or it is not a CBCH download
       * message. So start reading of next scheduled message.
       */
      memset (&pcbch->msg[0], 0, 88);
      cb_next_scheduled_msg();

    }
    else
    {
      /*
       * it is CBCH download message and SAT is enabled
       */
      if (status & CBCH_LAST_BLOCK)
      {
        /*
         * the first block is the last one, so send the
         * message to MMI.
         */
        cb_sat_cbch_message (CBCH_BLOCK_SIZE*(CBCH_BLOCK_0+1));
      }
      else
      {
        /*
         * store the first block and request the remaining three.
         */
        pcbch->msg_type = CBCH_SAT_MSG;
        cb_read_next_3_blocks();
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : TIL_CBCH                   |
| STATE   : code                ROUTINE : cb_sat_cbch_message        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a complete CBCH message for CBCH download.

*/

LOCAL void cb_sat_cbch_message (UBYTE length)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  PALLOC (mmi_cbch_ind, MMI_SAT_CBCH_DWNLD_IND);

  ALR_EM_CBCH_MESSAGE_COMPLETE;

  TRACE_FUNCTION ("cb_sat_cbch_message()");

  /*
   * copy the content of the message into the primitive.
   */
  memcpy (mmi_cbch_ind->cbch_msg, pcbch->msg, 88);
  mmi_cbch_ind->cbch_len = length;

  /*
   * send to MMI
   */
  PSENDX(MMI, mmi_cbch_ind);

  /*
   * prepare for next reception
   */
  pcbch->msg_type = CBCH_NO_MSG;
  pcbch->expected_seq_number[0] = CBCH_BLOCK_0;
  pcbch->expected_seq_number[1] = CBCH_BLOCK_0;
  /*
   * start reading of next scheduled message.
   */
  cb_next_scheduled_msg();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : TIL_CBCH                   |
| STATE   : code                ROUTINE : cb_schedule_message        |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a SCHEDULE message.

*/

LOCAL void cb_schedule_message (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = pcbch->msg_is_extended;
  UBYTE *p_sched_msg = &pcbch->scheduled_sched_msg[extended];

  ALR_EM_RECEIVE_SCHEDULE_MESSAGE;

  switch ( get_cbch_state(extended) )
  {
    case CBCH_NO_DRX:
      /*
       * change from NO DRX mode (no knowledge about Scheduling)
       * to DRX mode.
       */
      
      set_cbch_state (extended, CBCH_DRX);
      

      /*
       * check whether it is a scheduled schedule message or not.
       */
      if(pcbch->msg[0] EQ 1)
        *p_sched_msg = 1;
      else
        *p_sched_msg = 0;

      /*
       * create schedule map with new and old messages
       */
      cb_create_schedule_map (TRUE);

      /*
       * forward the schedule map to layer 1.
       */
      cb_read_cbch((UBYTE)(CBCH_SCHED | extended));

      break;

    case CBCH_DRX:
      if (pcbch->msg[0] EQ 1)
      {
        /*
         * scheduled SCHEDULE message
         */
         if(*p_sched_msg <= 2)
            (*p_sched_msg)++;

         if (*p_sched_msg > 1)
           /*
            * we have read a full schedule period
            * so we dont need to read the old messages
            */
           cb_create_schedule_map (FALSE);
         else
           cb_create_schedule_map (TRUE);

        cb_read_cbch((UBYTE)(CBCH_SCHED | extended));
      }      
      break;
    default:
      break;
  }
  /*
   * prepare for next reception
   */
  pcbch->msg_type = CBCH_NO_MSG;
  pcbch->expected_seq_number[0] = CBCH_BLOCK_0;
  pcbch->expected_seq_number[1] = CBCH_BLOCK_0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_null_message            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a NULL message.

*/
LOCAL void cb_null_message (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = pcbch->msg_is_extended;

  ALR_EM_RECEIVE_NULL_MESSAGE;

  switch ( get_cbch_state(extended) )
  {
    case CBCH_DRX:
      if (pcbch->schedule_length[extended] EQ 0)
      {
        /*
         * instead of the scheduled SCHEDULE message
         * a NULL message has received.
         * Restarting with NO DRX mode
         */
        set_cbch_state (extended, CBCH_NO_DRX);
        cb_read_cbch ((UBYTE)(CBCH_NO_SCHED | extended));
      }
      else
      {
        cb_next_scheduled_msg();
      }
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_error_ind               |
+--------------------------------------------------------------------+

  PURPOSE : Indicates an invalid block.
                   1) with error_flag =TRUE
                   2) invalid sequence number
                   3) out of sequence block
*/

LOCAL void cb_error_ind (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = pcbch->msg_is_extended;
                     
  if( pcbch->expected_seq_number[extended] NEQ CBCH_BLOCK_0)
  {
    /*
      * stop the next blocks
      */
    cb_stop_next_blocks();      
  }
  switch ( get_cbch_state(extended) )
  {
    case CBCH_DRX:
      if (pcbch->schedule_length[extended] EQ 0)
      {
        /*
         * instead of the scheduled SCHEDULE message
         * an invalid message has been received.
         * Restarting with NO DRX mode
         */
        set_cbch_state (extended, CBCH_NO_DRX);
        cb_read_cbch ((UBYTE)(CBCH_NO_SCHED | extended));
      }
      else
        /*
         * Go back to FIRST DRX mode.
         */
      {
        pcbch->scheduled_sched_msg[extended] = 1;       
       cb_next_scheduled_msg();
      }
      break;
    default:
      break;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_ind                     |
+--------------------------------------------------------------------+

  PURPOSE : Indication of the CBCH message.

*/

LOCAL void cb_ind (UBYTE length)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  PALLOC (mmi_cbch_ind, MMI_CBCH_IND);

  ALR_EM_CBCH_MESSAGE_COMPLETE;

  /*
   * copy the message content to the primitive.
   */
  memcpy (mmi_cbch_ind->cbch_msg, pcbch->msg, CBCH_MSG_SIZE);
  mmi_cbch_ind->cbch_len = length;

  ma_cb_mmi_cbch_ind(mmi_cbch_ind);

  {
    T_CBMSG_HEADER *p_cbh = (T_CBMSG_HEADER*)(&pcbch->msg[0]);
    ALR_TRACE_CB_MSGHEAD (p_cbh);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_read_next_3_blocks         |
+--------------------------------------------------------------------+

  PURPOSE : Starts read of next 3 blocks.

*/

LOCAL void cb_read_next_3_blocks(void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = pcbch->msg_is_extended;

  if( extended EQ CBCH_READ_EXT)
    ma_cb_info_req (CBCH_EXT_BLOCK234);
  else
    ma_cb_info_req (CBCH_NORM_BLOCK234);

  pcbch->expected_seq_number[extended] = CBCH_BLOCK_1;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_stop_next_blocks        |
+--------------------------------------------------------------------+

  PURPOSE : Stop next blocks.

*/

LOCAL void cb_stop_next_blocks (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  /*
   * stop the next blocks
   */

  /*
    * Upper layers / L1 Circuit Switched interfaces
    * S922.doc , Ver 1.9
    * Section 11.3 MPHC_CBCH_INFO_REQ 
    *
    * This message can be used to update on fly L1 
    * with new tb_bitmap which then replaces the current one. 
    * In such a case, 0 in the bitmap means abort any pending reading of TB1/2/3 or TB5/6/7.
    *
    * A previous "schedule_length=0" is used to specify "No schedule present".
    * In such a case, L1 starts reading continuously TB0 for normal CBCH or TB4 for Extended CBCH.
    */
  ma_cb_info_req (0);
  
  pcbch->msg_type = CBCH_NO_MSG;
  pcbch->expected_seq_number[0] = CBCH_BLOCK_0;
  pcbch->expected_seq_number[1] = CBCH_BLOCK_0;
}

#if defined(FF_HOMEZONE) && defined(INDICATE_HZ_ERROR)

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_ind_hz_err              |
+--------------------------------------------------------------------+

  PURPOSE : Indication of the CBCH message and stop the next blocks.

*/

LOCAL void cb_ind_hz_err (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  PALLOC (mmi_cbch_ind, MMI_CBCH_IND);

  memset (mmi_cbch_ind->cbch_msg, 0xff, CBCH_MSG_SIZE);
  mmi_cbch_ind->cbch_msg[2] = pcbch->msg_hz_id >> 8;
  mmi_cbch_ind->cbch_msg[3] = pcbch->msg_hz_id & 0xff;
  mmi_cbch_ind->cbch_len = 0; /* error indication */
  ALR_TRACE_CB ("cb_ind_hz_err()");

  ma_cb_mmi_cbch_ind(mmi_cbch_ind);
}
#endif /* FF_HOMEZONE */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_cbch_message            |
+--------------------------------------------------------------------+

  PURPOSE : Reception of a complete CBCH message.

*/

LOCAL void cb_cbch_message (UBYTE length)
{
  /*
   * Indicate the message
   */
  cb_ind (length);
  /*
   * stop the next blocks
   */
  cb_stop_next_blocks ();
  /*
   * store the cbch message to avoid double reception
   */
  cb_store_cbch_msg ();
  /*
   * start reading of the next scheduled message
   */
  cb_next_scheduled_msg();  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_store_cbch_msg          |
+--------------------------------------------------------------------+

  PURPOSE : Store key parameter of a CBCH message to avoid
            forwarding of a CBCH message twice.

*/
LOCAL void cb_store_cbch_msg (void)
{
  GET_INSTANCE_DATA;
  USHORT x;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  ALR_TRACE_CB ("Store msg");

  /*
   *  It is not permitted to save the cbch message,
   *  if the following pattern are in the
   *  lacation area identification (bcch:sys_info_3) :
   *                   [4]  0x00
   *                   [5]  0xF1
   *                   [6]  0x10
   *
   *  that means FTA in testhouse
   */
  if (alr_data->ma_data.sys_info_3[4] EQ 0     AND
      alr_data->ma_data.sys_info_3[5] EQ 0xF1  AND
      alr_data->ma_data.sys_info_3[6] EQ 0x10)
    return;

  /*
   * If storage is full, remove the first one.
   */
  if (pcbch->old_cbch_msg.cnt EQ MAX_OLD_CB)
    cb_remove_message (0);

  /*
   * store the new one
   */
  x = pcbch->old_cbch_msg.cnt++;
  memcpy (&pcbch->old_cbch_msg.header[x],
          &pcbch->msg, sizeof (T_CBMSG_HEADER));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_check_message_id        |
+--------------------------------------------------------------------+

  PURPOSE : Check Message Identifier of the first CBCH block.

*/

LOCAL UBYTE cb_check_message_id (USHORT msg_id)
{
  GET_INSTANCE_DATA;
  USHORT i;
  UBYTE  result = TRUE;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  /*
   * if no message identifier are defined for comparision
   * make a shortcut and return immediately. 
   * MODUS CBCH_ACCEPT,but no msg_id defined,accept NO messages
   * MODUS CBCH_IGNORE,but no msg_id define, accept all messages
   * MODUS CBCH_STOP,but no msg_id defined,  accept NO messages
   * MODUS CBCH_HOMEZONE,but no msg_id defined, accept NO messages/ 
   */
  if ( pcbch->msg_id[0] EQ NOT_PRESENT_16BIT )
  {
    if (pcbch->modus EQ CBCH_IGNORE) 
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }

  /*
   * initialise result
   * if the msg_id is not found and MMI specified
   * a positive range(CBCH_ACCEPT) return 0
   */
  result = (UBYTE)(pcbch->modus EQ CBCH_ACCEPT ? FALSE : TRUE);

  /*
   * message identifier are stored as a pair to express a range
   */
  for (i=0; i < MAX_IDENTS-1; i+=2)
  {
    /*
     * end of list reached
     */
    if (pcbch->msg_id[i] EQ NOT_PRESENT_16BIT)
      break;

    /*
     * check whether the message identifier is inside or outside
     * the range. Result depends on what MMI has requested.
     */
    if ((pcbch->msg_id[i]   <= msg_id) AND
        (pcbch->msg_id[i+1] >= msg_id))
      result = (UBYTE)(pcbch->modus EQ CBCH_ACCEPT ? TRUE : FALSE);
  }

  return result;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_sat_check_message_id    |
+--------------------------------------------------------------------+

  PURPOSE : Check Message Identifier of the first CBCH block for CBCH
            download purposes.

*/

LOCAL UBYTE cb_sat_check_message_id (USHORT msg_id)
{
  GET_INSTANCE_DATA;
  USHORT i;
  UBYTE  result;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  /*
   * initialize result
   */
  result = FALSE;

  /*
   * for all stored message identifier of CBCH download
   */
  for (i = 0; i < MAX_IDENTS_SAT; i++)
  {
    /*
     * end of list reached
     */
    if (pcbch->msg_id_sat[i] EQ NOT_PRESENT_16BIT)
      break;

    /*
     * if message identifier matches
     */
    if (pcbch->msg_id_sat[i] EQ msg_id)
    {
      result = TRUE;
      break;
    }
  }

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_check_data_coding_scheme|
+--------------------------------------------------------------------+

  PURPOSE : Check Data Coding Scheme of the first CBCH block.

*/
LOCAL UBYTE cb_check_data_coding_scheme (UBYTE dcs_id)
{
  GET_INSTANCE_DATA;
  USHORT i;
  UBYTE  result = TRUE;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  /*
   * if the list is empty, result is positive
   */
  if (pcbch->dcs_id [0] EQ NOT_PRESENT_8BIT)
    return result;

  /*
   * initialise result
   * if the dcs_id is not found and MMI specified
   * a positive range(CBCH_ACCEPT) return 0
   */
  result = (UBYTE)(pcbch->modus EQ CBCH_ACCEPT ? 0 : 1);


  for (i=0;i<MAX_IDENTS-1;i+=2)
  {
    /*
     * end of list reached
     */
    if (pcbch->dcs_id[i] EQ NOT_PRESENT_8BIT)
      break;

      /*
       * Data Coding Scheme identifier matches
       */
      if (pcbch->dcs_id[i] <= dcs_id AND
          pcbch->dcs_id[i+1] >= dcs_id)
        result = (UBYTE)(pcbch->modus EQ CBCH_ACCEPT ? 1 : 0);
  }

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_check_old_msg           |
+--------------------------------------------------------------------+

  PURPOSE : Check against old messages.

*/
LOCAL UBYTE cb_check_old_msg (void)
{
  GET_INSTANCE_DATA;
  int i;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  T_CBMSG_HEADER  *p_cbh = (T_CBMSG_HEADER*)&pcbch->msg[0];

  ALR_TRACE_CB ("compare with stored msg");
  for (i=0; i < pcbch->old_cbch_msg.cnt; i++)
  {
    ALR_TRACE_CB_MSGHEAD (&pcbch->old_cbch_msg.header[i]);

    if (memcmp (&pcbch->old_cbch_msg.header[i], p_cbh, 6) EQ 0)
    {
      return FALSE; /* already stored message */
    }
    else
    {
      /* if the difference is only in the update number,
       * then we need to delete the old version of the message (so that the different
       * versions do not fill up the storage) and we can even abandon the comparison;
       * a message that is resent with a different update number should only use
       * one entry in the list; otherwise it may throw out other messages that should be kept;
       * think of German network that sends every minute a new update; would use up 15 entries
       *
       * some change, now check if it is only the update number that has changed
       * (memcmp covers 4 header bytes: msg_id3,  msg_id4, dcs and page
       *  => ignore lint warning)
       */
      /*lint -e{420} (Warning -- Apparent access beyond array) */
      if ((CB_GS_CODE(&pcbch->old_cbch_msg.header[i]) == CB_GS_CODE(p_cbh)) AND
          (CB_MSG_CODE(&pcbch->old_cbch_msg.header[i]) == CB_MSG_CODE(p_cbh)) AND
          !memcmp (&(((T_CBMSG_HEADER*)&pcbch->old_cbch_msg.header[i])->msg_id3),
                     &p_cbh->msg_id3, 4)) 
      { /* all these fields match, therefore the change must have been only in the update number */

        SHORT old_update_nr, new_update_nr;
        old_update_nr = CB_UPDATE_NO(&pcbch->old_cbch_msg.header[i]);
        new_update_nr = CB_UPDATE_NO(p_cbh);
        /*
          * 3GPP 03 41
          * Any Update Number eight or less higher (modulo 16) than the last received Update Number 
          * will be considered more recent, and shall be treated as a new message
          */          
        if((!new_update_nr) OR 
           ((new_update_nr - old_update_nr + MAX_UPDATE_NO) % MAX_UPDATE_NO <= MAX_UPDATE_NO_DIFF))
        {
           ALR_TRACE_CB ("change update num, delete old message");
           /* remove the old message so that we do not use up another entry for this new version of the message */
           cb_remove_message (i);
           /* skip out of loop because we found the match  */
           break;
        }
        else
          return FALSE;
      }
    }
  }

  return TRUE;  /* new message */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_read_cbch               |
+--------------------------------------------------------------------+

  PURPOSE : Starts reading of the first block of CBCH.
            The parameter defines if the extended or the normal CBCH
            should be read and if scheduling should be used.

*/

LOCAL void cb_read_cbch (UBYTE flags)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = (UBYTE)(flags & CBCH_READ_EXT);
  ULONG first_block = pcbch->schedule_map[extended][0];
  ULONG second_block = pcbch->schedule_map[extended][1];
  
  PALLOC (sched_req, MPHC_CBCH_SCHEDULE_REQ);
 
  ALR_TRACE_CB_START_READ(flags);

  sched_req->cbch_select     = extended;

  ALR_EM_START_CBCH_READING;

  if(flags & CBCH_SCHED)
  {
    UBYTE n = pcbch->begin_schedule[extended] -1;
    
    sched_req->schedule_length = pcbch->end_schedule[extended] - 
                                 pcbch->begin_schedule[extended] +1;

    /*
     * the MPHC SAP uses first_blocks_1 as USHORT although the internal data base 
     * of ALR uses for convinience an array of 2 ULONG for first_blocks_0 and 
     * first_blocks_1; ergo: do a cast for first_blocks_1 here
     */

    if(  pcbch->begin_schedule[extended] EQ 1)
    {
      sched_req->first_blocks_0  = first_block;
      sched_req->first_blocks_1  = (USHORT)second_block;
    }
    else if(pcbch->begin_schedule[extended] < 32)
    {
      sched_req->first_blocks_0  = (first_block >> n) | ((second_block << (32 - n)));
      sched_req->first_blocks_1  = (USHORT) (second_block >> n);
    }
    else
    {
      sched_req->first_blocks_0 = (USHORT)(second_block >> (n - 32));
      sched_req->first_blocks_1  = 0;      
    }    
  }
  else
  {
    sched_req->schedule_length = 0;
  }

  ma_cb_sched_req(sched_req);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_check_ci_lai_plmn       |
+--------------------------------------------------------------------+

  PURPOSE : Checking the Cell Identity and the Location Area Identification
            to handle the geographical zone parameter.
*/

LOCAL void cb_check_ci_lai_plmn (void)
{
  GET_INSTANCE_DATA;
  USHORT  i;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE   keep = 0x00;
  UBYTE   *si3 = &alr_data->ma_data.sys_info_3[0];
  UBYTE   *lac = &pcbch->old_cid_plmn_lac[0];
  UCHAR  gs_scope_change_ind = CB_GEOGR_CHANGE_NONE;

  /*
   *  it is not permitted to save the cbch message,
   *  if the following pattern are in the
   *  location area identification (bcch:sys_info_3) :
   *                   [4]  0x00
   *                   [5]  0xF1
   *                   [6]  0x10
   *
   *   that means FTA in testhouse.
   */
  if ((si3[4] EQ 0x00) AND
      (si3[5] EQ 0xF1) AND
      (si3[6] EQ 0x10)
     )
    return;

  if ((si3[4] NEQ lac[2]) OR
      (si3[5] NEQ lac[3]) OR
      (si3[6] NEQ lac[4])
     )
  {
    /*
     *  The PLNM has changed
     */
    gs_scope_change_ind = CB_GEOGR_CHANGE_PLMN;
    pcbch->old_cbch_msg.cnt = 0; /* clear all messages */
  }
  else if ((si3[7] NEQ lac[5]) OR
           (si3[8] NEQ lac[6])
          )
  {
    /*
     *  The location area identification has changed
     */
    keep = CB_GEOGR_SCOPE_PLMN; /* keep PLMN wide messages */
    gs_scope_change_ind = CB_GEOGR_CHANGE_LAC;
  }
  else if ((si3[2] NEQ lac[0]) OR
           (si3[3] NEQ lac[1])
          )
  {
    /*
     *  The Cell Identity has changed
     */

    /* indicate cell change to MMI only if we have received any CBCH broadcast messages */
    if (pcbch->old_cbch_msg.cnt > 0)
		  gs_scope_change_ind = CB_GEOGR_CHANGE_CELL;
    keep = CB_GEOGR_SCOPE_CELLN; /* keep PLMN and Location Area wide messages */
  }


  /*
   * update old_cid_plmn_lac
   */
  memcpy (lac, &si3[2], 7);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_remove_message          |
+--------------------------------------------------------------------+

  PURPOSE : Removes the indicated CBCH message from the internal
            storage.

*/
LOCAL void cb_remove_message (USHORT i)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  if ((pcbch->old_cbch_msg.cnt) AND
      (i < pcbch->old_cbch_msg.cnt))
  {
    ALR_TRACE_CB ("remove msg");
    ALR_TRACE_CB_MSGHEAD (&pcbch->old_cbch_msg.header[i]);

    /*
     * decrement number of stored messages.
     */
    pcbch->old_cbch_msg.cnt--;
    memmove (&pcbch->old_cbch_msg.header[i],
             &pcbch->old_cbch_msg.header[i+1],
             (pcbch->old_cbch_msg.cnt-i) * sizeof(T_CBMSG_HEADER));
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_create_schedule_map     |
+--------------------------------------------------------------------+

  PURPOSE : Creation of a schedule map.

*/
LOCAL void cb_create_schedule_map (UBYTE old_cb_msg)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE  extended = pcbch->msg_is_extended;
  UBYTE* msg_ptr;

  /*
   * Store begin and end slot number
   */
  pcbch->begin_schedule [extended] = (UBYTE)(pcbch->msg [0] & 0x3F);
  pcbch->end_schedule   [extended] = pcbch->msg [1];

  /*
   * clear schedule bitmap
   */
  pcbch->schedule_length[extended] = 0;
  memset (&pcbch->schedule_map[extended][0], 0,
        2*sizeof (pcbch->schedule_map[extended][0]));
  /*
   * Check all new entries,
   * msg_ptr to first message description
   */
  msg_ptr = &pcbch->msg[8];

  msg_ptr = cb_create_schedule_entries (msg_ptr, 1);

  if (old_cb_msg)
  {
    cb_create_schedule_entries (msg_ptr, FALSE);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_create_schedule_entries |
+--------------------------------------------------------------------+

  PURPOSE : Creation of schedule entries.

*/
LOCAL UBYTE* cb_create_schedule_entries (UBYTE* msg_ptr, BOOL flag)
{
  GET_INSTANCE_DATA;
  unsigned    i;
  BOOL        result;
  UBYTE       x;
  USHORT      msg_id;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE       extended = pcbch->msg_is_extended;

  pcbch->last_slot_read[extended] = 0;
  for (i=0; i < pcbch->end_schedule[extended]; i++)
  {
    result = (pcbch->msg [2 + ((i+1)>>3)] & (1 <<(8-((i+1)&7))));

    ALR_TRACE_CB_SCHEDULE(i+1,result);

    if ((flag AND result) OR (!flag AND !result))
    {
      /*
       * result and bit are set, that means the msg:
       *  - was not send during the previous schedule period
       *  - or the msg was sent unscheduled during the last period
       *  - or is free of charge, reading advised
       * result and bit are both not set , that means the msg:
       *  - the msg doesn't match the above
       *  - these msgs are read only during the first 2 schedule periods
       */
      if (*msg_ptr & 0x80)
      {
        /*
         * First transmission of an SMSCB within the
         * Schedule Period
         */

        msg_id = (UBYTE)((*msg_ptr) & 0x7F);
        msg_ptr++;
        msg_id = (USHORT)(msg_id + *msg_ptr);
        if (cb_check_message_id (msg_id))
        {
          /*
           * Message Identifier is expected
           * then set slot bit in schedule map
           */
          pcbch->schedule_map[extended][i>>5] |= 1 << (i&31);
          if( (i+1) >= pcbch->begin_schedule[extended] )
          {
            pcbch->schedule_length[extended]++;
            pcbch->last_slot_read[extended] = (i+1);
          }
        }
      }
      else
      {
        if ((*msg_ptr & 0xC0) EQ 0)
        {
          /*
           * Retransmission indication
           * Extract original message slot number
           */
          x = (UBYTE)(*msg_ptr & 0x3F);
          if (x)
          {
            x--;
            if (pcbch->schedule_map[extended][x>>5] & (1 << (x&31)) )
            {
              /*
               * original message will be read, then set also
               * repetition of message in schedule map
               */
              pcbch->schedule_map[extended][i>>5] |= 1 << (i&31);
              if( (i+1) >= pcbch->begin_schedule[extended] )
              {
                pcbch->schedule_length[extended]++;
                pcbch->last_slot_read[extended] = (i+1);
              }
            }
          }
        }
        else
        {
          if (*msg_ptr EQ 0x41)
          {
            /*
             * Free Message Slot, reading advised
             * Set bit in schedule map
             */
            pcbch->schedule_map[extended][i>>5] |= 1 << (i&31);
            if( (i+1) >= pcbch->begin_schedule[extended] )
            {
              pcbch->schedule_length[extended]++;
              pcbch->last_slot_read[extended] = (i+1);
            }
          }
        }
      }
      /*
       * increment message description pointer
       */
      msg_ptr++;
    }
  }
  return msg_ptr;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : cb_next_scheduled_msg      |
+--------------------------------------------------------------------+

  PURPOSE : Looks for end of scheduling period.

*/
LOCAL void cb_next_scheduled_msg(void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;
  UBYTE extended = pcbch->msg_is_extended;

  if ((get_cbch_state(extended)) EQ CBCH_DRX )
  {
    if( pcbch->schedule_length[extended] EQ 0 )
    {
      /*
        * instead of the scheduled SCHEDULE message
        * an invalid message has been received.
        * Restarting with NO DRX mode
        */
      set_cbch_state (extended, CBCH_NO_DRX);
      cb_read_cbch ((UBYTE)(CBCH_NO_SCHED | extended));
      return;
    }
    pcbch->schedule_length[extended]--;

    if(pcbch->schedule_length[extended] EQ 0)
    {
     /*
       * clear schedule map
       * all message slots have been read
       *
       * NO need to reconfigure L1 to re-read schedule message 
       * that follows slot 48
       */
      memset (&pcbch->schedule_map[extended][0], 0,
        2*sizeof (pcbch->schedule_map[extended][0]));
      TRACE_EVENT("Scheduled Schedule message expected");

    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : get_cbch_state      |
+--------------------------------------------------------------------+

  PURPOSE : Returns the state of STATE_EXT_CBCH or STATE_NORMAL_CBCH
                   depending upon the input parameter "extended"

*/

UBYTE get_cbch_state( UBYTE extended)
{
  GET_INSTANCE_DATA;
  if( extended)
    return GET_STATE(STATE_EXT_CBCH);
  else
    return GET_STATE(STATE_NORMAL_CBCH);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE : set_cbch_state      |
+--------------------------------------------------------------------+

  PURPOSE : Sets  the state of STATE_EXT_CBCH or STATE_NORMAL_CBCH
                   depending upon the input parameter "extended"

*/

void set_cbch_state( UBYTE extended, UBYTE new_state)
{
  GET_INSTANCE_DATA;
  if(extended)
  {
    SET_STATE(STATE_EXT_CBCH,new_state);
  }
  else
    SET_STATE(STATE_NORMAL_CBCH,new_state);
}


#if defined(FF_HOMEZONE)
GLOBAL void cb_tim_homezone (void)
{
  GET_INSTANCE_DATA;
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  if (pcbch->homezone EQ CBCH_HZ_IDLE)
  {
    if (GET_STATE(STATE_MA) EQ MA_IDLE)
    {
      ALR_TRACE_CB ("call cb_start()");
      cb_start ();
    }
    else
    {
      TIMERSTART (TIM_HOMEZONE, 1000);
    }
  }
  else if (pcbch->homezone EQ CBCH_HZ_PENDING)
  {
    /*
     * mark as received, indicate error, stop CBCH reading
     */
    ALR_TRACE_CB ("cb_ind_hz_err()");
    pcbch->homezone = CBCH_HZ_RECEIVED;
#if defined(INDICATE_HZ_ERROR)
    cb_ind_hz_err ();
#endif  /* INDICATE_HZ_ERROR */
    if (    (pcbch->modus EQ NOT_PRESENT_8BIT)
        AND (!pcbch->sat_enabled))
      cb_stop ();
  }
}

LOCAL UBYTE cb_check_homezone (UBYTE length)
{
  GET_INSTANCE_DATA;
  UBYTE  result = FALSE;/* no special handling of homezone CBCH message */
  T_CBCH_DATA *pcbch = &alr_data->cbch_data;

  if (pcbch->homezone EQ CBCH_HZ_PENDING)
  {
    T_CBMSG_HEADER  *p_cbh;
    p_cbh = (T_CBMSG_HEADER*)(&pcbch->msg[0]);

    if (    (CB_MESSAGE_ID (p_cbh) EQ pcbch->msg_hz_id)
        AND (CB_DCS (p_cbh)        EQ pcbch->dcs_hz_id)
       )
    {
      /* one-time request of homezone CBCH message after cell reselection */
      TIMERSTOP (TIM_HOMEZONE);
      pcbch->homezone = CBCH_HZ_RECEIVED;
      ALR_TRACE_CB_MSGHEAD (p_cbh);
      ALR_TRACE_CB ("homezone msg received");
      cb_ind (length);
      result = TRUE;

      if (    (pcbch->modus EQ NOT_PRESENT_8BIT)
          AND (!pcbch->sat_enabled))
      {
        ALR_TRACE_CB ("no other msg req.");
        cb_stop ();
      }
      else
        cb_stop_next_blocks ();
    }
  }

  return result;
}
#endif  /* FF_HOMEZONE */

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : ALR_CBCH                   |
| STATE   : code                ROUTINE  : cb_alr_trace_msghead      |
+--------------------------------------------------------------------+

PURPOSE : Function to replace the ALR_TRACE_CB_MSGHEAD macro.

*/

/* Implements measures 2,3,4 */
LOCAL void cb_alr_trace_msghead (T_CBMSG_HEADER *ph)
{
  ALR_TRACE_CB ("cb_alr_trace_msghead()");
  if (CB_MSG_CODE(ph) < 1000) 
  {
    TRACE_EVENT_P7("CBCH ID=%3u, gs=%u, msg=%4u, upd=%u, dcs=%02x, p=%u/%u",
                    CB_MESSAGE_ID(ph), CB_GS_CODE(ph),
                    CB_MSG_CODE(ph), CB_UPDATE_NO(ph),
                    CB_DCS(ph), CB_PAGE(ph), CB_PAGETOTAL(ph));
  }
  else
  {
    TRACE_EVENT_P7("CBCH ID=%3u, gs=%u, msg=%04x, upd=%u, dcs=%02x, p=%u/%u",
                    CB_MESSAGE_ID(ph), CB_GS_CODE(ph),
                    CB_MSG_CODE(ph), CB_UPDATE_NO(ph),
                    CB_DCS(ph), CB_PAGE(ph), CB_PAGETOTAL(ph));
  }
}
#endif

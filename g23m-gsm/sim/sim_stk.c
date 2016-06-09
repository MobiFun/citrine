/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  SIM_STK
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
|  Purpose :  This modul defines the SIM Toolkit Upgrade.
+-----------------------------------------------------------------------------
*/

#ifndef SIM_STK_C
#define SIM_STK_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "sim.h"
#include "sim_em.h"
#include "cl_imei.h"  /* IMEI common library */
#include "cl_shrd.h"

#ifdef TI_PS_UICC_CHIPSET_15
#include "8010_136_SIMDRV_SAP_inline.h"
#endif

EXTERN  USHORT   stk_l_cmd;
/*==== EXPORT =====================================================*/

/*
 * These Functions are only temporary valid and should be replaced
 * as soon as possible
 */
EXTERN UBYTE get_network_meas   (UBYTE * chan_list);
EXTERN UBYTE get_bcch_chan_list (stk_data_type * out_stk_data);


/*==== PRIVAT =====================================================*/

#ifdef FF_SAT_E
LOCAL void stk_handle_ccd_error(UBYTE notOK,
                                UBYTE* status,
                                UBYTE* general_result,
                                UBYTE* add_info_result);
LOCAL void stk_bip_decode_stk_command(T_sdu*          message,
                                      T_cmd_details*  cmd_details,
                                      UBYTE*          status,
                                      UBYTE*          general_result,
                                      UBYTE*          add_info_result);
LOCAL void stk_close_dti_connection(UBYTE open);
LOCAL void stk_close_bip_channel(UBYTE general_result,
                                 UBYTE add_info_result);
/* LOCAL void stk_dti_inform_mmi_old (UBYTE dti_conn_req); */
LOCAL void stk_dti_inform_mmi(UBYTE dti_conn_req, UBYTE bip_conn_req );

LOCAL void stk_dti_send_data();
LOCAL void stk_bip_send_data_terminal_response(UBYTE general_result,
                                               UBYTE add_info_result);
#endif /* FF_SAT_E */

/*==== TYPES ======================================================*/

typedef struct
{
    UBYTE tag;
    UBYTE min_len;
    UBYTE mand;
    UBYTE min_req;
} T_SAT_TAG_DESC;

typedef struct
{
    UBYTE cmd_type;
    UBYTE tp_i;
    UBYTE tp_flag;
    UBYTE max_tag;
    const T_SAT_TAG_DESC *tag_desc;
} T_SAT_CMD_DESC;

/*==== CONSTANTS ==================================================*/

const T_SAT_TAG_DESC sat_def_cmd[2] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE }
};                   // valid tags for any command

const T_SAT_TAG_DESC sat_cmd_poll_itv[3] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_DURATION_TAG,
    STK_DURATION_LEN,
    TRUE, TRUE }
};                   // valid tags for POLL INTERVALL

const T_SAT_TAG_DESC sat_cmd_refresh[3] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_FILE_LIST_TAG,
    STK_FILE_LIST_LEN,
    FALSE, TRUE }
};                  // valid tags for REFRESH

const T_SAT_TAG_DESC sat_cmd_event_list[3] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_EVENT_LIST_TAG,
    STK_EVENT_LIST_LEN,
    TRUE, TRUE }
};                  // valid tags for SETUP EVENT LIST

const T_SAT_TAG_DESC sat_cmd_timer[4] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_TIMER_ID_TAG,
    STK_TIMER_ID_LEN,
    TRUE, TRUE },
  { STK_TIMER_VALUE_TAG,
    STK_TIMER_VALUE_LEN,
    FALSE, FALSE }
};                  // valid tags for TIMER MANAGEMENT

const T_SAT_TAG_DESC sat_cmd_receive_data[5] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_ALPHA_IDENTITY_TAG,
    STK_ALPHA_IDENTITY_LEN,
    FALSE, FALSE },
  { STK_ICON_IDENTITY_TAG,
    STK_ICON_IDENTITY_LEN,
    FALSE, FALSE },
  { STK_CHANNEL_DATA_LGTH_TAG,
    STK_CHANNEL_DATA_LGTH_LEN,
    TRUE, TRUE }
};                  // valid tags for RECEIVE DATA

const T_SAT_TAG_DESC sat_cmd_send_data[5] = {
  { STK_COMMAND_DETAILS_TAG,
    STK_COMMAND_DETAILS_LEN,
    TRUE, TRUE },
  { STK_DEVICE_IDENTITY_TAG,
    STK_DEVICE_IDENTITY_LEN,
    TRUE, TRUE },
  { STK_ALPHA_IDENTITY_TAG,
    STK_ALPHA_IDENTITY_LEN,
    FALSE, FALSE },
  { STK_ICON_IDENTITY_TAG,
    STK_ICON_IDENTITY_LEN,
    FALSE, FALSE },
  { STK_CHANNEL_DATA_TAG,
    STK_CHANNEL_DATA_LEN,
    TRUE, TRUE }
};                  // valid tags for SEND DATA

const T_SAT_CMD_DESC sat_cmd_list[] = {
  { 0, 0, 0xFF,
    item_of(sat_def_cmd), sat_def_cmd },    // command not yet known
  { STK_REFRESH, 2, SAT_TP3_REFRESH,
    item_of(sat_cmd_refresh), sat_cmd_refresh },
  { STK_MORE_TIME, 2, SAT_TP3_MORE_TIME,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_POLL_INTERVALL, 2, SAT_TP3_POLL_ITV,
    item_of(sat_cmd_poll_itv), sat_cmd_poll_itv },
  { STK_POLLING_OFF, 2, SAT_TP3_POLL_OFF,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SET_UP_CALL, 3, SAT_TP4_SETUP_CALL,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SEND_SS, 3, SAT_TP4_SEND_SS,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SEND_SMS, 3, SAT_TP4_SEND_SMS,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_PLAY_TONE, 2, SAT_TP3_PLAY_TONE,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_DISPLAY_TEXT, 2, SAT_TP3_DSPL_TXT,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_GET_INKEY, 2, SAT_TP3_GET_INKEY,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_GET_INPUT, 2, SAT_TP3_GET_INPUT,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SELECT_ITEM, 3, SAT_TP4_SEL_ITEM,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SET_UP_MENU, 3, SAT_TP4_SETUP_MENU,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_PROVIDE_LOCAL_INFO, 3, SAT_TP4_PLI_PLMN_IMEI,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_TIMER_MANAGEMENT, 7, SAT_TP8_TMNG_ST,
    item_of(sat_cmd_timer), sat_cmd_timer },
  { STK_SETUP_EVENT_LIST, 4, SAT_TP5_EVENT_LIST,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SEND_USSD, 3, SAT_TP4_SEND_USSD,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SEND_DTMF, 8, SAT_TP9_DTMF_CMD,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_SETUP_IDLE_TEXT, 7, SAT_TP8_IDLE_TXT,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_RUN_AT_CMD, 7, SAT_TP8_AT_CMD,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_LANGUAGE_NOTIFICATION, 8, SAT_TP9_LANG_NOTIFY,
    item_of(sat_def_cmd), sat_def_cmd }
#ifdef FF_SAT_C
  ,
  { STK_LAUNCH_BROWSER, 8, SAT_TP9_LAUNCH_BROWSER,
    item_of(sat_def_cmd), sat_def_cmd }
#endif /* FF_SAT_C */
#ifdef FF_SAT_E
  ,
  { STK_OPEN_CHANNEL, 11, SAT_TP12_OPEN_CHANNEL,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_CLOSE_CHANNEL, 11, SAT_TP12_CLOSE_CHANNEL,
    item_of(sat_def_cmd), sat_def_cmd },
  { STK_RECEIVE_DATA, 11, SAT_TP12_RECEIVE_DATA,
    item_of(sat_cmd_receive_data), sat_cmd_receive_data },
  { STK_SEND_DATA, 11, SAT_TP12_SEND_DATA,
    item_of(sat_cmd_send_data), sat_cmd_send_data },
  { STK_GET_CHANNEL_STAT, 11, SAT_TP12_GET_CHANNEL_STAT,
    item_of(sat_def_cmd), sat_def_cmd }
#endif /* FF_SAT_E */
};

/* Terminal Profile bits for which SIM is exclusively responsible.
   They are discarded from the profile provided by MMI */

static const UBYTE sat_tp_sim_exclusive[MAX_STK_PRF] = {
      /* SAT_TP1_PRF_DNL |*/ SAT_TP1_TIMER_EXP | SAT_TP1_CC_ON_REDIAL,
      0x0,
      SAT_TP3_MORE_TIME | SAT_TP3_POLL_ITV | SAT_TP3_POLL_OFF,
      SAT_TP4_PLI_PLMN_IMEI | SAT_TP4_PLI_NMR,
      0x0 /*SAT_TP5_LOC_STATUS*/,
      0x0, 0xFF,
      SAT_TP8_TMNG_ST | SAT_TP8_TMNG_VAL,
      SAT_TP9_BCCH_COD | SAT_TP9_PLI_TIMING_ADV,    /* Timing Advance is to be supported */
      0x0, 0x0,
      SAT_TP12_RECEIVE_DATA | SAT_TP12_SEND_DATA,
      0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0x0, 0xFF};

/* Terminal Profile bits which have to be supported by SIM to be
   valid with the profile provided by MMI */

static const UBYTE sat_tp_sim_ability[MAX_STK_PRF] = {
      SAT_TP1_PRF_DNL,
      SAT_TP2_CMD_RES,
      SAT_TP3_DSPL_TXT | SAT_TP3_GET_INKEY | SAT_TP3_GET_INPUT
       | SAT_TP3_PLAY_TONE | SAT_TP3_REFRESH,
      SAT_TP4_SEL_ITEM | SAT_TP4_SEND_SMS | SAT_TP4_SEND_SS
       | SAT_TP4_SEND_USSD | SAT_TP4_SETUP_CALL | SAT_TP4_SETUP_MENU,
      SAT_TP5_EVENT_LIST, 0x0, 0x0,
      SAT_TP8_IDLE_TXT | SAT_TP8_AT_CMD | SAT_TP8_PLI_DTT,
      SAT_TP9_DTMF_CMD | SAT_TP9_LANG_NOTIFY | SAT_TP9_LAUNCH_BROWSER,
      0x0, 0x0,
      SAT_TP12_OPEN_CHANNEL | SAT_TP12_CLOSE_CHANNEL | SAT_TP12_GET_CHANNEL_STAT,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

/* Currently supported Terminal Profile bits by SIM */

static const UBYTE sat_tp_sim_enabled[MAX_STK_PRF] = {
      SAT_TP1_PRF_DNL | SAT_TP1_SMS_DNL | SAT_TP1_9E_XX | SAT_TP1_TIMER_EXP,
      SAT_TP2_CMD_RES,
      SAT_TP3_MORE_TIME | SAT_TP3_POLL_ITV | SAT_TP3_POLL_OFF | SAT_TP3_REFRESH,
      SAT_TP4_PLI_PLMN_IMEI | SAT_TP4_PLI_NMR,
      0x0, 0x0, 0x0,
      SAT_TP8_TMNG_ST | SAT_TP8_TMNG_VAL | SAT_TP8_PLI_DTT
       | SAT_TP8_IDLE_TXT | SAT_TP8_AT_CMD,
      SAT_TP9_BCCH_COD | SAT_TP9_PLI_TIMING_ADV,       /* Timing Advance is to be supported */
      0x0, 0x0,
#ifdef FF_SAT_E
      SAT_TP12_OPEN_CHANNEL | SAT_TP12_CLOSE_CHANNEL | SAT_TP12_GET_CHANNEL_STAT
       | SAT_TP12_RECEIVE_DATA | SAT_TP12_SEND_DATA,
#else
      0x0,
#endif
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

static const UBYTE tbl_device_src_id[] =
{
  DEV_SRC_KBD,
  DEV_SRC_DSP,
  DEV_SRC_EAR,
  DEV_SRC_CDR0,
  DEV_SRC_CDR1,
  DEV_SRC_CDR2,
  DEV_SRC_CDR3,
  DEV_SRC_CDR4,
  DEV_SRC_CDR5,
  DEV_SRC_CDR6,
  DEV_SRC_CDR7,
  DEV_SRC_CH1,
  DEV_SRC_CH2,
  DEV_SRC_CH3,
  DEV_SRC_CH4,
  DEV_SRC_CH5,
  DEV_SRC_CH6,
  DEV_SRC_CH7,
  DEV_SRC_SIM,
  DEV_SRC_ME,
  DEV_SRC_NTW,
  0
};

/*==== VARIABLES ==================================================*/
BOOL startTimerPollOff = FALSE; 

USHORT cusSatMinPollItv = 50;     // 5 seconds

/*==== FUNCTIONS ==================================================*/

LOCAL UBYTE stk_dti_bip_receive_data (T_sdu* message, UBYTE result_code);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_init_sim_data          |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the SIM data for the module SIM TOOLKIT.

*/

GLOBAL void stk_init_sim_data (void)
{
  TRACE_FUNCTION ("stk_init_sim_data()");

  /*
   * initialize SAT variables
   */
  sim_data.sat_session    = FALSE;
  sim_data.ext_sat_cmd    = FALSE;
  sim_data.term_resp_sent = FALSE;
  sim_data.chk_sat_avail  = FALSE;
  sim_data.context_switch_ptr         = NULL; 
  sim_data.cust_mode                  = FALSE;
  sim_data.user_confirmation_expected = FALSE;

#ifdef FF_SAT_E
  /*
   * initialize DTI variables
   */
  sim_data.dti_connection_state       = SIM_DTI_CONNECTION_CLOSED;
  sim_data.dti_rx_state               = SIM_DTI_RX_IDLE;
  sim_data.dti_tx_state               = SIM_DTI_TX_IDLE;
  sim_data.event_data_avail           = SIM_EVENT_DISABLE;
  sim_data.bip_state                  = SIM_BIP_CLOSED;
  sim_data.bip_suspend                = FALSE;
#ifdef _SIMULATION_
  TRACE_EVENT("bip_rx_state = IDLE");
#endif
  sim_data.bip_rx_state               = SIM_BIP_RX_IDLE;
#ifdef _SIMULATION_
  TRACE_EVENT("bip_tx_state = IDLE");
#endif
  sim_data.bip_tx_state               = SIM_BIP_TX_IDLE;
  sim_data.bip_timer_state            = SIM_BIP_TIMER_DISCONNECTED;
  sim_data.bip_release_time           = SIM_NO_AUTO_RELEASE;
  sim_data.bip_general_result         = RSLT_PERF_SUCCESS;
  sim_data.bip_add_info_result        = ADD_NO_CAUSE;
  sim_data.con_type                   = SIM_CON_TYPE_UDP;
  sim_data.data_to_send.first         = (ULONG)NULL;
  sim_data.data_to_send.list_len      = 0;
  sim_data.prev_data_to_send.first    = (ULONG)NULL;
  sim_data.prev_data_to_send.list_len = 0;
  sim_data.received_data.first        = (ULONG)NULL;
  sim_data.received_data.list_len     = 0;
  sim_data.received_data_pos          = 0;
  sim_data.sim_dti_req                = NULL;
  sim_data.sim_bip_req                = NULL;
  sim_data.sim_bip_config_req         = NULL;
  memset(&sim_data.udp_parameters, 0, sizeof(T_SRC_DES));
#endif /* FF_SAT_E */
} /* stk_init_sim_data() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)     MODULE  : SIM_STK                      |
| STATE   : code              ROUTINE : stk_check_tp                 |
+--------------------------------------------------------------------+

  PURPOSE : Evaluates the received Terminal Profile against the
            capability of the SIM Entity.

*/

GLOBAL void stk_check_tp (UBYTE *out_prf, UBYTE *in_prf, USHORT len)
{
  int i;
  UBYTE tp_tmp;

/* TRACE_FUNCTION ("stk_check_tp()"); */

  len = MINIMUM(len, MAX_STK_PRF);
  for (i = 0; i < len; i++)                 // check TP from MMI
  {
#ifndef __OLD
    tp_tmp = sat_tp_sim_enabled[i] & ~sat_tp_sim_ability[i];
    out_prf[i] = ((in_prf[i] & ~sat_tp_sim_exclusive[i]) &
                 ~tp_tmp) | tp_tmp;
#else
    out_prf[i] = ((in_prf[i] & ~sat_tp_sim_exclusive[i]) &
                  (sat_tp_sim_enabled[i] | ~sat_tp_sim_ability[i]))
                  | sat_tp_sim_enabled[i];
#endif
  }
  if (!(out_prf[0] & SAT_TP1_SMS_DNL))      // check SMS download
    out_prf[0] &= ~SAT_TP1_9E_XX;           // only valid with SMS download

  if (len >= 6 AND
      !(out_prf[4] & SAT_TP5_EVENT_LIST))   // check event list consistency
    out_prf[5] = out_prf[4] = 0;            // discard event list

  if (len >= 12 AND
      (out_prf[11] & STK_TP12_CLASS_E) NEQ STK_TP12_CLASS_E)
  {
    out_prf[11] &= ~STK_TP12_CLASS_E;

    if (len >= 13)
      out_prf[12] &= ~(SAT_TP13_CSD_SUPP_BY_ME | SAT_TP13_GPRS_SUPP_BY_ME);

    if (len >= 17)
      out_prf[16] &= ~(SAT_TP17_BEARER_IND_SUPP_TCP | SAT_TP17_BEARER_IND_SUPP_UDP);
  }
  TRACE_EVENT_P5("TP: %02X %02X %02X %02X ... (%d bytes)",
                 out_prf[0], out_prf[1], out_prf[2], out_prf[3], len);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)     MODULE  : SIM_STK                      |
| STATE   : code              ROUTINE : stk_perform_profile_download |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the SIM data for the module application.

*/

GLOBAL void stk_perform_profile_download (void)
{
  USHORT result;
  USHORT used_tp = (USHORT)sizeof (sim_data.stk_profile);

  TRACE_FUNCTION ("stk_perform_profile_download()");

  do
  {
    if (sim_data.stk_profile[--used_tp] NEQ 0)
      break;
  } while (used_tp >= 3);
  used_tp++;

  if ((result = FKT_TerminalProfile (sim_data.stk_profile,
                                     used_tp)) NEQ SIM_NO_ERROR)
  {
    TRACE_EVENT_P1("TP dnl error: %04X", result);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)     MODULE  : SIM_STK                      |
| STATE   : code              ROUTINE : stk_proactive_polling        |
+--------------------------------------------------------------------+

  PURPOSE : Polling of an proactive SIM card.

*/

static const UBYTE tag_dur [] =
{
    STK_DURATION_TAG |
    STK_COMPREHENSION_REQUIRED,     /* duration tag                 */
    STK_DURATION_LEN,               /* duration length              */
    0,                              /* unit                         */
    0                               /* number of units              */
};

static const UBYTE terminal_response_loci [21]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    STK_PROVIDE_LOCAL_INFO,         /* command PROVIDE LOCATION INFO*/
    0,                              /* location information         */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    1,                              /* result length                */
    0,                              /* result OK                    */
    0x13,                           /* location information tag     */
    7,                              /* location information length  */
    0,0,0,                          /* MCC & MNC                    */
    0,0,                            /* Location area code           */
    0,0                             /* cell identity                */
};

static const UBYTE terminal_response_imei [22]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    STK_PROVIDE_LOCAL_INFO,         /* command PROVIDE LOCATION INFO*/
    1,                              /* imei                         */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    1,                              /* result length                */
    0,                              /* result OK                    */
    0x14,                           /* imei tag                     */
    8,                              /* imei length                  */
    0,0,0,0,0,0,0,0                 /* imei digits                  */
};

static const UBYTE terminal_response_nmr [14]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    STK_PROVIDE_LOCAL_INFO,         /* command PROVIDE LOCATION INFO*/
    2,                              /* network measurement results  */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    1,                              /* result length                */
    0,                              /* result OK                    */
    0x16,                           /* nmr tag                      */
    16                              /* nmr length                   */
};

/*
 * Terminal-Response for timing advance
 */
static const UBYTE terminal_response_timingadv [16]  =
{
    /* ---- cmd details ----- */
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,   /*  command details tag  */
    STK_COMMAND_DETAILS_LEN,      /*  command details length  */
    0,                            /*  command number  */
    STK_PROVIDE_LOCAL_INFO,       /*  command PROVIDE LOCAL INFO  */
    5,                            /*  timing advance  */

    /* ---- device ids ------ */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /*  device details tag  */
    STK_DEVICE_IDENTITY_LEN,        /*  device details length  */
    0x82,                           /*  source ME  */
    0x81,                           /*  destination SIM  */
    
    /* ------result --------- */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /*  result tag  */
    1,                              /*  result length  */
    0,                              /*  result OK  */
    
    /* --- timing advance --- */
    0x2E,                           /*  timing advance tag  */
    2,                              /*  timing advance len  */
    0,                              /*  ME Status  */
    0                               /*  Timing Advance  */
};

static const T_SIM_TRSP_SIMPLE terminal_response_def =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    0,                              /* command type place holder    */
    0,                              /* command qualifier            */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    1,                              /* result length place holder   */
    0xFF,                           /* result place holder          */
    0,                              /* result additional info       */
};

#ifdef __INVALID
static const UBYTE terminal_response_ok [12]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    2,                              /* command MORE TIME            */
    0,                              /* not used                     */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    1,                              /* result length                */
    0                               /* result OK                    */
};

static const UBYTE terminal_response_loci_neg [13]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    STK_PROVIDE_LOCAL_INFO,         /* command PROVIDE LOCATION INFO*/
    0,                              /* location information         */
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED,     /* result tag                   */
    2,                              /* result length                */
    0x20,                           /* result ME unable to process  */
    4                               /* result add. info no service  */
};

static const UBYTE terminal_response_no_cap [12]  =
{
    STK_COMMAND_DETAILS_TAG |
    STK_COMPREHENSION_REQUIRED,     /* command details tag          */
    STK_COMMAND_DETAILS_LEN,        /* command details length       */
    0,                              /* command number               */
    0x26,                           /* command PROVIDE LOCATION INFO*/
    0,                              /* location information         */
    2,                              /* device details tag           */
    2,                              /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    3,                              /* result tag                   */
    1,                              /* result length                */
    0x30                            /* result beyond ME capability  */
};
#endif

LOCAL UBYTE stk_process_tl (UBYTE **pp_stk,
                            SHORT *p_ber_len,
                            SHORT *p_tlv_len)
{
/* get tag and length and adjust pointer to get parameter values */

  UBYTE tag = STK_COMPREHENSION_REQUIRED;
  SHORT tlv_len;

  if (*p_ber_len >= 2)
  {
    tag = *((*pp_stk)++);           // get tag and adjust pointer
    if ((tlv_len = (SHORT)*((*pp_stk)++)) <= 0x7F)
    {                               // one byte length
      *p_tlv_len = tlv_len;         // get length
      *p_ber_len -= 2;
    }
    else if (tlv_len EQ 0x81 AND *p_ber_len >= 3)
    {                               // two bytes length
      *p_tlv_len = (SHORT)*((*pp_stk)++);
      *p_ber_len -= 3;              // get length and adjust pointer
    }
    else
    {                               // erroneous length coding
      *p_tlv_len = 0;
      *p_ber_len = 0;
      return STK_COMPREHENSION_REQUIRED;
    }
  }
  return tag;
}

LOCAL SHORT stk_build_response (UBYTE *p_response,
                                UBYTE *p_cmd, int cmd_len,
                                UBYTE *p_res, int res_len,
                                UBYTE *p_prm, int prm_len)
{
  /*
   * builds a TERMINAL RESPONSE
   */
  static const UBYTE dev_resp[5] =
  {
    STK_DEVICE_IDENTITY_TAG |
    STK_COMPREHENSION_REQUIRED,     /* device details tag           */
    STK_DEVICE_IDENTITY_LEN,        /* device details length        */
    0x82,                           /* source ME                    */
    0x81,                           /* destination SIM              */
    STK_RESULT_TAG |
    STK_COMPREHENSION_REQUIRED      /* result tag                   */
  };
  int tr_len = 6;

#ifdef _SIMULATION_
  TRACE_FUNCTION ("stk_build_response()");
#endif
  TRACE_FUNCTION_P1 ("TERMINAL_RESPONSE: result=0x%02X", (int)*p_res);

  memcpy (p_response, p_cmd, cmd_len);
  memcpy (p_response + cmd_len, dev_resp, 5);
  p_response[cmd_len + 5] = (UBYTE)res_len;
  memcpy (p_response + cmd_len + 6, p_res, res_len);
  tr_len += cmd_len + res_len;

  if (p_prm NEQ NULL AND prm_len > 0)
  {
    memcpy (p_response + tr_len, p_prm, prm_len);
    tr_len += prm_len;
  }
  return (SHORT)tr_len;
}


GLOBAL int process_sim_refresh( T_CONTEXT_SWITCH    *cmd_ptr )
{
  int i;
  SHORT resp_len = 0;
  #ifdef TI_PS_UICC_CHIPSET_15
  U8 readerId = SIMDRV_VAL_READER_ID__RANGE_MIN;
  U8 voltageSelect = SIMDRV_REQ_VOLTAGE_SEL;
  #endif
  
  T_STK_POLL_DATA *p;
  MALLOC (p, sizeof (T_STK_POLL_DATA));
  memset (p, 0, sizeof (T_STK_POLL_DATA));

  TRACE_FUNCTION ("process_sim_refresh()");

  /* process as before */
  switch (cmd_ptr->p_cmd[4])
  {
  case 1:
    /*check file list is empty */
    if( ( 0 == cmd_ptr->fl_len ) OR ( 1 == cmd_ptr->fl_len ) )
    {
      cmd_ptr->res_code[0] = STK_RES_ERR_MISS_VALUE;
      resp_len = stk_build_response (p->response,
                                     cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                     cmd_ptr->res_code, 1, NULL, 0);
#ifdef _SIMULATION_
      TRACE_EVENT("FCN: File List is empty");
#endif
      FKT_TerminalResponse (p->response, (USHORT)resp_len);
      break;
    }
    if (cmd_ptr->tag2 NEQ NULL AND cmd_ptr->fl_len >= 5)
    {
      BOOL sim_init = FALSE;
      PALLOC (file_update_sms, SIM_FILE_UPDATE_IND);
      memset (file_update_sms, 0, sizeof(T_SIM_FILE_UPDATE_IND));

      file_update_sms->val_nr = *(cmd_ptr->tag2++);
      cmd_ptr->fl_len -= 3;    /* align file id count */
      for (i = 0; cmd_ptr->fl_len > 0 AND i < (int)file_update_sms->val_nr; i++)
      {
        do
        {
          cmd_ptr->tag2 += 2;
          /* Check if entry is a first level directory */
          if(cmd_ptr->tag2[0] EQ 0x7F)
          {
            if(file_update_sms->file_info[i].v_path_info EQ FALSE)
            {
              file_update_sms->file_info[i].v_path_info = TRUE;
              file_update_sms->file_info[i].path_info.df_level1 = 
                ((USHORT)cmd_ptr->tag2[0] << 8) | (USHORT)cmd_ptr->tag2[1];
            }
            else
            {
              /* This has already been filled. There cannot be another 
              first level directory. Since i is reduced, i will not 
              equal to file_update_sms->val_nr and error will be sent to SIM */
              i--;
              break;
            }
          }
          /* Check if entry is a second level directory and first level 
           * directory is also filled*/
          else if(cmd_ptr->tag2[0] EQ 0x5F)
          { 
            if(file_update_sms->file_info[i].v_path_info EQ TRUE AND
               file_update_sms->file_info[i].path_info.v_df_level2 EQ FALSE)
            {
              file_update_sms->file_info[i].path_info.v_df_level2 = TRUE;
              file_update_sms->file_info[i].path_info.df_level2 = 
                ((USHORT)cmd_ptr->tag2[0] << 8) | (USHORT)cmd_ptr->tag2[1];
            }
            else
            {
              /* This has already been filled. There cannot be another 
              second level directory */
              i--;
              break;
            }
          }
          cmd_ptr->fl_len -= 2;
        }
        while (cmd_ptr->fl_len > 0 AND cmd_ptr->tag2[2] NEQ 0x3F);

        if (cmd_ptr->tag2[0] NEQ 0x2F AND cmd_ptr->tag2[0] NEQ 0x6F
                AND cmd_ptr->tag2[0] NEQ 0x4F)
          break;      /* no EF code -> loop exit leads to error response */

        file_update_sms->file_info[i].datafield = ((USHORT)cmd_ptr->tag2[0] << 8)
                                                   | (USHORT)cmd_ptr->tag2[1];
        if ((file_update_sms->file_info[i].datafield EQ SIM_PHASE) OR
            (file_update_sms->file_info[i].datafield EQ SIM_SST))

        {             /* SIM initialisation is needed! */
           sim_init = TRUE;
           TRACE_FUNCTION ("FCN converted to INIT");
           /* break;      exit for loop */
        }
      }
      if (cmd_ptr->fl_len > 0 OR i NEQ (int)file_update_sms->val_nr)
      {               /* inconsistent TLV content */
        PFREE(file_update_sms);
        cmd_ptr->res_code[0] = STK_RES_ERR_CMD_DATA;
        resp_len = stk_build_response (p->response,
                            cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                            cmd_ptr->res_code, 1, NULL, 0);
#ifdef _SIMULATION_
        TRACE_EVENT("FCN: inconsistent TLV content");
#endif
        FKT_TerminalResponse (p->response, (USHORT)resp_len);
        break;        /* exit switch 'cmd_qual' */
      }
      if (!sim_init)
      {
        {
          PALLOC (file_update_mmi, SIM_FILE_UPDATE_IND);
          memcpy (file_update_mmi, file_update_sms,
                         sizeof(T_SIM_FILE_UPDATE_IND));
          PSENDX (MMI, file_update_mmi);
        }
        {
          PALLOC (file_update_mm, SIM_FILE_UPDATE_IND);
          memcpy (file_update_mm, file_update_sms,
                         sizeof(T_SIM_FILE_UPDATE_IND));
          PSENDX (MM, file_update_mm);
        }
        PSENDX (SMS, file_update_sms);

        sim_data.file_change_resp = 0x7;  /* check response */
        /* Force file selection */
        sim_data.act_directory = NOT_PRESENT_16BIT;
        sim_data.act_field     = NOT_PRESENT_16BIT;

        if ((USHORT)(cmd_ptr->cmd_len + 7) <= sizeof (sim_data.stk_response))
        {
          sim_data.stk_resp_len = stk_build_response (sim_data.stk_response,
                                      cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                      cmd_ptr->res_code, 1, NULL, 0);
#ifdef _SIMULATION_
          TRACE_EVENT("FCN: prepare TR");
#endif
        }
        else
          sim_data.stk_resp_len = 0;
        break;
      }
      else
      {
        PFREE (file_update_sms);
      }
    }
    else
    {
      cmd_ptr->res_code[0] = STK_RES_ERR_MISS_VALUE;
      resp_len = stk_build_response (p->response,
                                     cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                     cmd_ptr->res_code, 1, NULL, 0);
#ifdef _SIMULATION_
      TRACE_EVENT("FCN: cmd incomplete");
#endif
      FKT_TerminalResponse (p->response, (USHORT)resp_len);
      break;
    }           // no break for sim_init EQ TRUE !

  case 3:
  case 2:
    if (cmd_ptr->res_code[0] EQ STK_RES_SUCCESS)
      // do not overwrite special result codes
      // these commands read more EFs than indicated
      cmd_ptr->res_code[0] = STK_RES_SUCC_ADD_EF_READ;
  case 0:
    if ( (SIM_IS_FLAG_SET (CALL_ACTIVE)) AND (sim_data.cust_mode EQ 0) )
    {
      cmd_ptr->res_code[0] = STK_RES_BUSY_ME;
      cmd_ptr->res_code[1] = STK_RES_EXT_BUSY_CALL;
      resp_len = stk_build_response (p->response,
                                     cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                     cmd_ptr->res_code, 2, NULL, 0);
#ifdef _SIMULATION_
      TRACE_EVENT("INIT: call active");
#endif
      FKT_TerminalResponse (p->response, (USHORT)resp_len);
    }
    else
    {
      /* Force file selection */
      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field     = NOT_PRESENT_16BIT;
      sim_data.status_time   = THIRTY_SECONDS;
      app_sim_read_parameters ();
      /* The terminal response to be sent is created and stored in context.
         Once ACI and MM indicates the completion of the reading of the EFs
         through SIM_SYNC_REQ, the response will be sent. */
      sim_data.stk_resp_len = stk_build_response (sim_data.stk_response,
                                     cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                     cmd_ptr->res_code, 1, NULL, 0);
      sim_data.sync_awaited = SIM_SYNC_AWAIT_MM_READ | SIM_SYNC_AWAIT_MMI_READ;

#ifdef _SIMULATION_
      TRACE_EVENT("INIT: success");
#endif
    }
    break;
  case 4:
    if ( (SIM_IS_FLAG_SET (CALL_ACTIVE)) AND (sim_data.cust_mode EQ 0) )
    {
      cmd_ptr->res_code[0] = STK_RES_BUSY_ME;
      cmd_ptr->res_code[1] = STK_RES_EXT_BUSY_CALL;
      resp_len = stk_build_response (p->response,
                                     cmd_ptr->p_cmd, (int)cmd_ptr->cmd_len,
                                     cmd_ptr->res_code, 2, NULL, 0);
#ifdef _SIMULATION_
      TRACE_EVENT("RESET: call active");
#endif
      FKT_TerminalResponse (p->response, (USHORT)resp_len);
    }
    else
    {
      #ifndef TI_PS_UICC_CHIPSET_15
        T_SIM_CARD sim_info;
      #endif   /*  TI_PS_UICC_CHIPSET_15 */
      USHORT retcode;

      TRACE_ASSERT (cmd_ptr->sig_ptr);
      PFREE (cmd_ptr->sig_ptr);
      TRACE_ASSERT (p);
      MFREE (p);
      sim_data.remove_error = SIM_NO_ERROR;
      app_sim_remove ();
      sim_data.remove_error = SIM_CAUSE_CARD_REMOVED;

      #ifndef TI_PS_UICC_CHIPSET_15
         SIM_PowerOff ();
      #else
         simdrv_poweroff(readerId);
      #endif
      /* Force file selection */
      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field     = NOT_PRESENT_16BIT;

      /*retcode = SIM_Restart (&sim_info); Driver does not call 'insert()'! */
      #ifndef TI_PS_UICC_CHIPSET_15
         retcode = SIM_Reset (&sim_info);
      #else
         retcode = simdrv_reset (readerId,voltageSelect);
      #endif
      TRACE_EVENT_P1 ("Result SIM Restart = %d", (int)retcode);
      TRACE_FUNCTION ("process_sim_refresh() exited(1)");
      return(1); //return and exit stk_proactive_polling(), if that is where called from
    }
    break;

  default:
    /*
     * not supported information request
     */
     TRACE_FUNCTION ("process_sim_refresh() default:");
     cmd_ptr->res_code[0] = STK_RES_ERR_CMD_TYPE;
     resp_len = stk_build_response (p->response, cmd_ptr->p_cmd, cmd_ptr->cmd_len,
                                    cmd_ptr->res_code, 1, NULL, 0);
#ifdef _SIMULATION_
     TRACE_EVENT("RFR: unknown qualifier");
#endif
     FKT_TerminalResponse (p->response, (USHORT)resp_len);
     break;
   }

   TRACE_ASSERT (cmd_ptr->sig_ptr);
   PFREE (cmd_ptr->sig_ptr);
   TRACE_ASSERT (p);
   MFREE (p);
   TRACE_FUNCTION ("process_sim_refresh() exited(0)");
   return(0);
}


GLOBAL void stk_proactive_polling (void)
{
  UBYTE *stk, *p_tag, *p_cmd = NULL;
  UBYTE *found_tag[STK_MAX_EXP_TAG];
  UBYTE compreh = 0, cmd_nr = 0, cmd_type, cmd_qual = 0;
  SHORT ber_len, tag_len, cmd_len, fl_len = 0, resp_len = 0;
  SHORT i, tag_i, cmd_i, tlv_len;
  const T_SAT_TAG_DESC *p_tag_desc;
  int offset;
  T_LOC_INFO loc_info;
  T_TIM_ADV tim_adv;

  TRACE_FUNCTION ("stk_proactive_polling()");

  if (SIM_IS_FLAG_SET (PRO_ACTIVE_SIM))
  {
    while (sim_data.proactive_sim_data_len > 0)
    {
      UINT   in_queue, out_queue;
      UBYTE  result = FALSE;
      UBYTE  res_code[2] = {STK_RES_SUCCESS, 0};
      T_TIME tm_val;
      T_STK_POLL_DATA *p;

      PALLOC (sim_toolkit_ind, SIM_TOOLKIT_IND);
      MALLOC (p, sizeof (T_STK_POLL_DATA));
      /*
       * clear indication primitive
       */
      memset (sim_toolkit_ind, 0, sizeof (T_SIM_TOOLKIT_IND));
      sim_toolkit_ind->stk_cmd.l_cmd = ((USHORT)sim_data.proactive_sim_data_len) << 3;
      ber_len = (SHORT)sim_data.proactive_sim_data_len;
      sim_data.proactive_sim_data_len = 0;
      sim_data.term_resp_sent         = FALSE;

      if (FKT_Fetch(sim_toolkit_ind->stk_cmd.cmd, ber_len) EQ SIM_NO_ERROR)
      {
        UBYTE tag;
        BOOL tag_not_found = TRUE;

#ifdef _SIMULATION_
        TRACE_EVENT("FKT_Fetch_EQ_SIM_NO_ERROR"); /* for debug purpose only */
#endif
        sim_data.ext_sat_cmd = FALSE;             /* internal command by default */

        stk = sim_toolkit_ind->stk_cmd.cmd;
        memset (found_tag, 0, sizeof(found_tag));
        tag_i = cmd_i = 0;
        cmd_len = 0;
        cmd_type = 0;

        tag = stk_process_tl(&stk, &ber_len, &tlv_len); /* BER tag */
        if (tag NEQ STK_PROACTIVE_SIM_COMMAND_TAG)
        {
          res_code[0] = STK_RES_ERR_MISS_VALUE;
        }
        else if (ber_len >= tlv_len)
        {
          ber_len = tlv_len;        // omit data after BER-TLV
          result = TRUE;
        }
        else /* ber_len < tlv_len */
        {
          if (tlv_len > 2)
          {
            res_code[0] = STK_RES_ERR_MISS_VALUE;
            result = TRUE;
          }
          else
          {
            res_code[0] = STK_RES_ERR_CMD_DATA;
          }
        }

        while (result AND ber_len > 0) // tag and length needed
        {
          UBYTE tag_simple;

          p_tag = stk;              // begin of TLV
          tag_len = ber_len;
          tag_simple = stk_process_tl(&stk, &ber_len, &tlv_len);

          if (ber_len < tlv_len)    // inconsistent length
          {
            if (res_code[0] NEQ STK_RES_ERR_MISS_VALUE)
            {
              TRACE_EVENT("ber_len < tlv_len -> STK_RES_ERR_CMD_DATA");
              res_code[0] = STK_RES_ERR_CMD_DATA;
            }
            result = FALSE;
            break;
          }
          else
            ber_len -= tlv_len;     // remaining BER-TLV

          tag_len -= ber_len;       // length of TLV

          if (tag EQ (tag_simple & ~STK_COMPREHENSION_REQUIRED))
          {
#ifdef _SIMULATION_
            TRACE_EVENT("TC_505A, TC_505B, TC_505C");
#endif
            stk += tlv_len;
            continue;
          }
          else
          {
            tag = tag_simple;
            tag_not_found = FALSE;
          }

          compreh = tag & STK_COMPREHENSION_REQUIRED;
          tag &= ~STK_COMPREHENSION_REQUIRED;

          do
          {
            p_tag_desc = &sat_cmd_list[cmd_i].tag_desc[tag_i];
            if (tag EQ p_tag_desc->tag)
            {
              break; /* tag found !!! */
            }
            else if (!p_tag_desc->mand)
            {
              tag_i++; /* skip non-mandatory tags */
            }
            else /* mandatory tag missed */
            {
              i = tag_i;
              res_code[0] = STK_RES_ERR_MISS_VALUE;
              while (i < (int)sat_cmd_list[cmd_i].max_tag)
              {
                p_tag_desc = &sat_cmd_list[cmd_i].tag_desc[i++];
                if (tag EQ p_tag_desc->tag AND !p_tag_desc->mand)
                {
#ifdef _SIMULATION_
                  TRACE_EVENT("TC_506");
#endif
                  res_code[0] = STK_RES_ERR_CMD_DATA;
                  break;
                }
              }
              if (!compreh AND i EQ (int)sat_cmd_list[cmd_i].max_tag)
              {
#ifdef _SIMULATION_
                TRACE_EVENT("TC_507");
#endif
                res_code[0] = STK_RES_SUCC_PART_COMPR;
                tag_not_found = TRUE;
              }
              break;
            }
          } while (tag_i < (int)sat_cmd_list[cmd_i].max_tag OR res_code[0]);

          if (tag_i >= (int)sat_cmd_list[cmd_i].max_tag)
          {
            i = (int)sat_cmd_list[cmd_i].tp_i;

            if ((~sat_tp_sim_ability[i] & sat_cmd_list[cmd_i].tp_flag) EQ 0)
            {
              break; /* the commmand is being handled outside the SIM */
            }

            if (compreh)
            {
#ifdef _SIMULATION_
              TRACE_EVENT("TC_502C, TC_504");
#endif
              res_code[0] = STK_RES_ERR_CMD_DATA;
              result = FALSE;
              break;
            }
          }
          else
          {
            p_tag_desc = &sat_cmd_list[cmd_i].tag_desc[tag_i];
            if (tag NEQ p_tag_desc->tag OR tlv_len < p_tag_desc->min_len)
            {
              if (compreh)
              {
#ifdef _SIMULATION_
                TRACE_EVENT("TC_503, TC_506");
#endif
                result = FALSE;
                break;
              }
            }
            else
              found_tag[tag_i] = stk;   // points to parameters of tag
          }

          if (tag_not_found)
          {
            stk += tlv_len;
            continue;
          }

          switch (tag)
          {
          case STK_COMMAND_DETAILS_TAG:
            p_cmd = p_tag;              // for TERMINAL RESPONSE
            cmd_len = tag_len;
            cmd_nr = stk[0];
            cmd_type = stk[1];
            cmd_qual = stk[2];

            TRACE_FUNCTION_P2("COMMAND_DETAILS: type=0x%02X, qlf=0x%02X",
                              (int)cmd_type, (int) cmd_qual);

            SIM_EM_PROACTIVE_COMMAND;

            for (i = 1; i < (SHORT)item_of(sat_cmd_list); i++) /* find out if known command */
            {
              if (cmd_type EQ sat_cmd_list[i].cmd_type)
              {
                cmd_i = i;
                break;
              }
            }
            if (i EQ item_of(sat_cmd_list))
            {
              TRACE_EVENT_P1("UNKNOWN COMMAND TYPE:0x%02X", cmd_type);
            }
            break;

          case STK_DEVICE_IDENTITY_TAG:
#ifdef _SIMULATION_
            TRACE_EVENT("found SIMPLE_TAG: STK_DEVICE_IDENTITY_TAG");
#endif
            break;

          case STK_CHANNEL_DATA_TAG:
#ifdef _SIMULATION_
            TRACE_EVENT("found SIMPLE_TAG: STK_CHANNEL_DATA_TAG");
#endif
            break;

          case STK_CHANNEL_DATA_LGTH_TAG:
#ifdef _SIMULATION_
            TRACE_EVENT("found SIMPLE_TAG: STK_CHANNEL_DATA_LGTH_TAG");
#endif
            break;

          case STK_FILE_LIST_TAG:
            fl_len = tlv_len;
            break;

          default:
#ifdef _SIMULATION_
            TRACE_EVENT_P1("other tag=0x%02X", tag);
#endif
            break;
          }
          if (++tag_i >= STK_MAX_EXP_TAG) /* incremented index points to next tag_desc entry */
            break;

          stk += tlv_len;
        }

        if (!result)
        {
          if (found_tag[0] EQ 0)
          {
            switch (res_code[0])
            {
            case STK_RES_ERR_MISS_VALUE:
#ifdef _SIMULATION_
              TRACE_EVENT("TC_501, TC_508");
#endif
              memcpy (p->response, &terminal_response_def, SAT_TRSP_MIN_RES);
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_nr = cmd_nr;
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_type = cmd_type;
              ((T_SIM_TRSP_SIMPLE *)p->response)->res_gnrl = res_code[0];
              FKT_TerminalResponse (p->response, SAT_TRSP_MIN_RES);
              break;

            default:
              break;
            }
          }
          else if (found_tag[0] AND cmd_i NEQ 0)
          {
            switch (res_code[0])
            {
            case STK_RES_ERR_CMD_DATA:
#ifdef _SIMULATION_
              TRACE_EVENT("TC_502A, TC_502B, TC_502C, TC_506");
#endif
              memcpy (p->response, &terminal_response_def, SAT_TRSP_MIN_RES);
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_nr = cmd_nr;
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_type = cmd_type;
              ((T_SIM_TRSP_SIMPLE *)p->response)->res_gnrl = res_code[0];
              FKT_TerminalResponse (p->response, SAT_TRSP_MIN_RES);
              break;

            case STK_RES_ERR_MISS_VALUE:
#ifdef _SIMULATION_
              TRACE_EVENT("TC_503, TC_509A");
#endif
              memcpy (p->response, &terminal_response_def, SAT_TRSP_MIN_RES);
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_nr = cmd_nr;
              ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_type = cmd_type;
              ((T_SIM_TRSP_SIMPLE *)p->response)->res_gnrl = res_code[0];
              FKT_TerminalResponse (p->response, SAT_TRSP_MIN_RES);
              break;

            default:
              break;
            }
          }
          else if (cmd_i EQ 0)
          {
#ifdef _SIMULATION_
            TRACE_EVENT("TC_504");
#endif
            res_code[0] = STK_RES_ERR_CMD_TYPE;
            memcpy (p->response, &terminal_response_def, SAT_TRSP_MIN_RES);
            ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_nr = cmd_nr;
            ((T_SIM_TRSP_SIMPLE *)p->response)->cmd_type = cmd_type;
            ((T_SIM_TRSP_SIMPLE *)p->response)->res_gnrl = res_code[0];
            FKT_TerminalResponse (p->response, SAT_TRSP_MIN_RES);
          }
          else
          {
            resp_len = stk_build_response (p->response, p_cmd, cmd_len, res_code, 1, NULL, 0);
#ifdef _SIMULATION_
            TRACE_EVENT("SAT: other error");
#endif
            FKT_TerminalResponse (p->response, (USHORT)resp_len);
          }
          PFREE (sim_toolkit_ind);
        }
        else switch (cmd_type)
        {
          case STK_DISPLAY_TEXT:
          case STK_GET_INKEY:
          case STK_GET_INPUT:
          case STK_PLAY_TONE:
          case STK_SET_UP_MENU:
          case STK_SELECT_ITEM:
          case STK_SEND_SMS:
          case STK_SEND_SS:
          case STK_SET_UP_CALL:

          case STK_SETUP_EVENT_LIST:
          case STK_SEND_USSD:
          case STK_SEND_DTMF:
          case STK_SETUP_IDLE_TEXT:
          case STK_LAUNCH_BROWSER:
          case STK_RUN_AT_CMD:
          case STK_LANGUAGE_NOTIFICATION:
#ifdef FF_SAT_E
          case STK_OPEN_CHANNEL:
          case STK_CLOSE_CHANNEL:
          case STK_GET_CHANNEL_STAT:
#endif /* FF_SAT_E */
            /*
             * Operations processed by MMI / MFW
             */
            sim_data.sat_session = TRUE;
            sim_data.ext_sat_cmd = TRUE;
            PSENDX (MMI, sim_toolkit_ind);
            break;

          case STK_POLL_INTERVALL:
            /*
             * SIM toolkit defines an idle poll interval. The timer in
             * SIM application is stopped and started with the
             * new value. A terminal response with the used time
             * is sent to SIM toolkit.
             */
            if (found_tag[2] NEQ NULL)
            {
              BOOL org_val;
              TRACE_EVENT("Idle Polling switched on");

              if (found_tag[2][1] NEQ 0) /* value not "reserved" */
              {
                switch (found_tag[2][0])
                {
                case 0: /* minutes */
                  if ((ULONG)found_tag[2][1] * 600L >= (ULONG)cusSatMinPollItv)
                  {
                    sim_data.status_time = ((T_TIME)found_tag[2][1]) * 60L * 1000
                                            - TIMER_LATENCY;
                    org_val = TRUE;
                  }
                  else
                    org_val = FALSE;
                  break;
                case 1: /* seconds */
                  if ((USHORT)found_tag[2][1] * 10 >= cusSatMinPollItv)
                  {
                    sim_data.status_time = ((T_TIME)found_tag[2][1]) * 1000L
                                            - TIMER_LATENCY;
                    org_val = TRUE;
                  }
                  else
                    org_val = FALSE;
                  break;
                case 2: /* tenth seconds */
                  if ((USHORT)found_tag[2][1] >= cusSatMinPollItv)
                  {
                    sim_data.status_time = ((T_TIME)found_tag[2][1]) * 100L
                                            - TIMER_LATENCY;
                    org_val = TRUE;
                  }
                  else
                    org_val = FALSE;
                  break;
                default: /* command data not understood */
                  res_code[0] = STK_RES_ERR_CMD_DATA;
                  org_val = TRUE;
                  break; /* return original values */
                }
              }
              else /* return original values */
              {
                res_code[0] = STK_RES_ERR_CMD_DATA;
                org_val = TRUE;
              }

              resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                             res_code, 1, (UBYTE *)tag_dur, sizeof(tag_dur));
#ifdef _SIMULATION_
              TRACE_EVENT("POLL_ITV data not understood");
#endif
              if (org_val)
              {
                p->response[cmd_len+ 9] = found_tag[2][0];  /* copy unit */
                /* copy number of units (1 to 255) or 0 !!! */
                p->response[cmd_len+10] = found_tag[2][1];
              }
              else
              {
                sim_data.status_time = ((T_TIME)cusSatMinPollItv) * 100L
                                        - TIMER_LATENCY;
                if (cusSatMinPollItv < 0x100)
                {
                  p->response[cmd_len+ 9] = 2;     // fits to tenth of seconds
                  p->response[cmd_len+10] = (UBYTE)cusSatMinPollItv;
                }
                else if (cusSatMinPollItv < (0x100*10))
                {
                  p->response[cmd_len+ 9] = 1;     // fits to seconds
                  p->response[cmd_len+10] = (UBYTE)((cusSatMinPollItv / 5 + 1) / 2);
                }
                else
                {
                  p->response[cmd_len+ 9] = 0;     // fits to minutes
                  p->response[cmd_len+10] = (UBYTE)((cusSatMinPollItv / 300 + 1) / 2);
                }
              }
            sim_data.idle_polling = TRUE;
            }
            else
            {
              res_code[0] = STK_RES_ERR_MISS_VALUE;
              resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                             res_code, 1, NULL, 0);
#ifdef _SIMULATION_
              TRACE_EVENT("POLL_ITV missing value");
#endif
            }
            FKT_TerminalResponse (p->response, (USHORT)resp_len);
            PFREE (sim_toolkit_ind);

            if (SIM_IS_FLAG_CLEARED (CALL_ACTIVE) OR
                sim_data.status_time < THIRTY_SECONDS)
              TIMER_PSTART( sim_handle,
                            SIM_TIMER,
                            sim_data.status_time,
                            sim_data.status_time);
            break;

          case STK_PROVIDE_LOCAL_INFO:
            /*
             * SIM toolkit request local information from the
             * SIM application. These informations are:
             * MCC, MNC, LAC, Cell Identity or IMEI.
             * SIM application sends this information with
             * a terminal response to the SIM toolkit.
             */
            if(p_cmd EQ NULL) 
            {
               TRACE_EVENT("p_cmd is NULL");
               return;
            }
            switch (cmd_qual)
            {
              case QLF_PLOI_LOC_INFO:
                /*
                 * request location information
                 */
#ifdef _SIMULATION_
                if (sim_data.location_info.c_loc NEQ 0)
                {
                  memcpy (p->response, p_cmd, cmd_len);
                  /* location information available */
                  memcpy (&p->response[cmd_len], &terminal_response_loci[5], 9);
                  /* copy MCC, MNC & LAC */
                  memcpy (&p->response[cmd_len+ 9], &sim_data.location_info.loc[4], 5);
                  /* copy cell identity */
                  p->response[cmd_len+14] = sim_data.cell_identity >> 8;
                  p->response[cmd_len+15] = sim_data.cell_identity & 0xFF;
                  p->response[cmd_len+ 6] = res_code[0];
                  resp_len = cmd_len + 16;
#else
                if((cl_shrd_get_loc (&loc_info) EQ TRUE) AND
                   (loc_info.service_mode NEQ NO_SERVICE))
                {
                  memcpy (p->response, p_cmd, cmd_len);
                  /* location information available */
                  memcpy (&p->response[cmd_len], &terminal_response_loci[5], 9);
                  /* copy from loc_info to response */
                  p->response[cmd_len+9]   = loc_info.mcc[1] << 4;
                  p->response[cmd_len+9]  += loc_info.mcc[0];
                  p->response[cmd_len+10]  = loc_info.mnc[2] << 4;
                  p->response[cmd_len+10] += loc_info.mcc[2];
                  p->response[cmd_len+11]  = loc_info.mnc[1] << 4;
                  p->response[cmd_len+11] += loc_info.mnc[0];
                  p->response[cmd_len+12]  = loc_info.lac >> 8;
                  p->response[cmd_len+13]  = loc_info.lac & 0xff;
                  p->response[cmd_len+14]  = loc_info.cell_id >> 8;
                  p->response[cmd_len+15]  = loc_info.cell_id & 0xFF;
                  
                  if ( loc_info.service_mode EQ LIMITED_SERVICE )
                  {
                    p->response[cmd_len+ 6] = STK_RES_SUCC_LIMITED_SERVICE;
                  }
                  else
                  {
                    p->response[cmd_len+ 6] = res_code[0];
                  }
                  resp_len = cmd_len + 16;
#endif  /* _SIMULATION_ */
                }
                else
                {
                  /* location information not available */
                  res_code[0] = STK_RES_BUSY_ME;
                  res_code[1] = STK_RES_EXT_NO_SERVICE;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                 res_code, 2, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: local info not available");
#endif
                }
                sim_data.ext_sat_cmd = FALSE;
                break;
              case QLF_PLOI_IMEI:
                /*
                 * request IMEI
                 */
                memcpy (p->response, p_cmd, cmd_len);
                memcpy (&p->response[cmd_len], &terminal_response_imei[5], 17);

                cl_get_imeisv(CL_IMEI_SIZE,
                              &p->response[cmd_len + 9],
                              CL_IMEI_GET_STORED_IMEI);
                for (i = (int)cmd_len + 8 + CL_IMEI_SIZE;
                     i > (int)cmd_len + 9; i--)
                {     /* place nibbles according to 4.08 */
                  p->response[i] = (p->response[i] & 0xf0) |
                                   (p->response[i - 1] &  0x0f);
                }
                /* i points to first IMEI octet, set 'type of identity' */
                p->response[i] = (p->response[i] & 0xf0) | 0xA;
                p->response[cmd_len+8+CL_IMEI_SIZE] &= 0xF;
                p->response[cmd_len+ 6] = res_code[0];
                resp_len = cmd_len + 17;
                sim_data.ext_sat_cmd = FALSE;
                break;
              case QLF_PLOI_NTW_MSR:
                /*
                 * request network measurement results and BCCH list
                 */
                if ((sat_tp_sim_enabled[3] & SAT_TP4_PLI_NMR) EQ 0)
                {
                  /* if not effective */
                  res_code[0] = STK_RES_ERR_NO_SUPPORT;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                 res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: NMR not supported");
#endif
                  break;
                }
                memcpy (p->response, p_cmd, cmd_len);
                /* location information available */
                memcpy (&p->response[cmd_len], &terminal_response_nmr[5], 9);
                if (get_network_meas (&p->response[cmd_len+9]))
                {
                  /*
                   * set result code
                   */
#ifdef _SIMULATION_
                  p->response[cmd_len+ 6] = res_code[0];
                  resp_len = cmd_len + 25;
#else
                  if ((cl_shrd_get_loc (&loc_info) EQ TRUE) AND
                      (loc_info.service_mode NEQ NO_SERVICE))
                  {
                    if ( loc_info.service_mode EQ LIMITED_SERVICE )
                    {
                      p->response[cmd_len+ 6] = STK_RES_SUCC_LIMITED_SERVICE;
                    }
                    else
                    {
                      p->response[cmd_len+ 6] = res_code[0];
                    }
                    resp_len = cmd_len + 25;
                  }
                  else
                  {
                    res_code[0] = STK_RES_BUSY_ME;
                    res_code[1] = STK_RES_EXT_NO_SERVICE;
                    resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                  res_code, 2, NULL, 0);
#ifdef _SIMULATION_
                   TRACE_EVENT("PLI: service mode not available");
#endif /* _SIMULATION_ */
                  }
#endif /* _SIMULATION_ */
                  /*
                   * Add BCCH list
                   */
                  if (get_bcch_chan_list (&p->stk_data))
                  {
                    /*
                     * copy BCCH data list for answer
                     */
                    p->response[resp_len] = 0x1D;  /* BCCH channel flag */
                    p->response[resp_len + 1] = p->stk_data.stk_length;
                    memcpy (&p->response[resp_len + 2], p->stk_data.stk_parameter,
                            p->stk_data.stk_length);
                    resp_len += (2+p->stk_data.stk_length);
                  }
                  else
                  {
                    /* location information not available */
                    res_code[0] = STK_RES_BUSY_ME;
                    res_code[1] = STK_RES_EXT_NO_SERVICE;
                    resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                   res_code, 2, NULL, 0);
#ifdef _SIMULATION_
                    TRACE_EVENT("PLI: BCCH list not available");
#endif
                  }
                }
                else
                {
                  /* location information not available */
                  res_code[0] = STK_RES_BUSY_ME;
                  res_code[1] = STK_RES_EXT_NO_SERVICE;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                 res_code, 2, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: measurement not available");
#endif
                }
                sim_data.ext_sat_cmd = FALSE;
                break;
              case QLF_PLOI_DTT:
                /*
                 * Date, Time and Timezone
                 */
                if ((sim_data.stk_profile[7] & SAT_TP8_PLI_DTT) EQ 0)
                {
                  /* if not effective */
                  res_code[0] = STK_RES_ERR_NO_SUPPORT;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                 res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: not supported in TP");
#endif
                  break;
                }
                /*
                 * Operations processed by MMI / MFW
                 */
                sim_data.ext_sat_cmd = TRUE;
                PSENDX (MMI, sim_toolkit_ind);
                break;
              case QLF_PLOI_LANG_SET:
               /*
                * Language Setting to be supported (to be processed by ACI)
                */
                if ((sim_data.stk_profile[8] & SAT_TP9_PLI_LANG) EQ 0)
                {
                  /* if not effective */
                  res_code[0] = STK_RES_ERR_NO_SUPPORT;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                                 res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("language setting not supported");
#endif
                  break;
                }
                /*
                 * To be processed by ACI
                 */
                sim_data.sat_session = TRUE;
                sim_data.ext_sat_cmd = TRUE;
                PSENDX (MMI, sim_toolkit_ind);
                break;

              case QLF_PLOI_TIM_ADV:
                
                if ((sat_tp_sim_enabled[8] & SAT_TP9_PLI_TIMING_ADV) EQ 0)
                {
                  res_code[0] = STK_RES_ERR_NO_SUPPORT;
                  resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: timing advance not supported");
#endif
                }
                else
                {
                  memset (&tim_adv, 0, sizeof(T_TIM_ADV));
                  memcpy (p->response, p_cmd, cmd_len);
                  memcpy (&p->response[cmd_len], &terminal_response_timingadv[5], 11);
#ifdef _SIMULATION_
                  p->response[cmd_len+ 6] = res_code[0];
                  resp_len = cmd_len + 11;
#else
                 if(cl_shrd_get_tim_adv (&tim_adv) EQ TRUE)
                 {


                   p->response[cmd_len+9] = tim_adv.me_status;
                   p->response[cmd_len+10] = tim_adv.tm_adv;
                   /*
                   * Set result code.
                   */
                   p->response[cmd_len+ 6] = res_code[0];
                   resp_len = cmd_len + 11;
                 }
                 else
                 {
                    res_code[0] = STK_RES_BUSY_ME;
                    resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                  TRACE_EVENT("PLI: Timing Advance not available");
#endif
                 }
#endif /* _SIMULATION_ */
                }

                sim_data.ext_sat_cmd = FALSE;
                break;
              default:
                /*
                 * no valid command qualifier
                 */
                res_code[0] = STK_RES_ERR_CMD_TYPE;
                resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                TRACE_EVENT("PLI: type not understood");
#endif
                sim_data.ext_sat_cmd = FALSE;
                break;
            }
            if (!sim_data.ext_sat_cmd)   /* may depend on Command Qualifier */
            {
              FKT_TerminalResponse (p->response, (USHORT)resp_len);
              PFREE (sim_toolkit_ind);
            }
            break;

          case STK_REFRESH:
            /*
             * process SIM REFRESH according to command qualifier
             */
            TRACE_EVENT("process SIM REFRESH according to command qualifier");

            /* MALLOC area for CONTEXT SWITCH */
            if ( sim_data.context_switch_ptr != NULL )
            {
              TRACE_EVENT("ERROR!! context_switch_ptr should be NULL - freeing");
              MFREE ( sim_data.context_switch_ptr );
              sim_data.context_switch_ptr = NULL;
            }
            MALLOC (sim_data.context_switch_ptr, sizeof (T_CONTEXT_SWITCH));
            memset ( sim_data.context_switch_ptr,0, sizeof (T_CONTEXT_SWITCH));

            /* Set sig_ptr to handle required signal */
#ifdef TI_PS_FF_AT_P_CMD_CUST
            if (sim_data.cust_mode EQ 1)
            {
              /* Cust_Mode 1 operation setup*/
              PALLOC(copy_sim_toolkit_ind,SIM_TOOLKIT_IND);
              memcpy(copy_sim_toolkit_ind,sim_toolkit_ind,sizeof(T_SIM_TOOLKIT_IND));
              sim_data.context_switch_ptr->sig_ptr = copy_sim_toolkit_ind;
            }
            else
            {
              /*default operation setup*/
              sim_data.context_switch_ptr->sig_ptr = sim_toolkit_ind;
            }
#else
            /*default operation setup*/
            sim_data.context_switch_ptr->sig_ptr = sim_toolkit_ind;
#endif /* TI_PS_FF_AT_P_CMD_CUST */

            /* Save decoding of original sim_toolkit_ind  signal processed by stk_proactive_polling */
            offset = (long)found_tag[2] - (long)sim_toolkit_ind;
            sim_data.context_switch_ptr->tag2 = (UBYTE *)(sim_data.context_switch_ptr->sig_ptr) + offset;
            sim_data.context_switch_ptr->fl_len = fl_len;
            offset = (long)p_cmd - (long)sim_toolkit_ind;
            sim_data.context_switch_ptr->p_cmd = (UBYTE *)(sim_data.context_switch_ptr->sig_ptr) + offset;
            sim_data.context_switch_ptr->cmd_len = cmd_len;
            sim_data.context_switch_ptr->res_code[0]=res_code[0];
            sim_data.context_switch_ptr->res_code[1]=res_code[1];

#ifdef TI_PS_FF_AT_P_CMD_CUST
            /* functionality is depended on cust_mode */
            if (sim_data.cust_mode EQ 1)
            {
              /* Cust_Mode 1 operation */
              sim_data.user_confirmation_expected = TRUE;

              /*send Refresh command to MMI*/
              PSENDX (MMI, sim_toolkit_ind);

              /*REFRESH COMMAND NOW PROCESSED IN stk_sim_refresh_user_res()      */
              /*note: stk_sim_refresh_user_res() will call process_sim_refresh() */
              /*      This routine will free sig_ptr, user must free the         */
              /*      context_switch.                                            */

              /*Terminate Command */
              break; 
            }

            else 
            {
              /* sim_data.cust_mode EQ 0 */
              /* process default operation */
              if ( process_sim_refresh(sim_data.context_switch_ptr) )
              {
                /* processed a SIM_RESET. Special case - exit stk_proactive_polling()*/
                MFREE(p);
                /* this thread has finished with the context switch so free it.      */
                MFREE (sim_data.context_switch_ptr);
                sim_data.context_switch_ptr = NULL;
                return;   /* exit from stk_proactive_polling() */
              }
              else
              {
                /* process_sim_refresh() frees signal  sim_data.context_switch_ptr->sig_ptr */
                /* this thread has finished with the context switch so free it.             */
                MFREE (sim_data.context_switch_ptr); 
                sim_data.context_switch_ptr = NULL;
              }
            }
#else
            /* sim_data.cust_mode EQ 0 */
            /* process default operation */
            if ( process_sim_refresh(sim_data.context_switch_ptr) )
            {
              /* processed a SIM_RESET. Special case - exit stk_proactive_polling()*/
              MFREE(p);
              /* this thread has finished with the context switch so free it.      */
              MFREE (sim_data.context_switch_ptr);
              sim_data.context_switch_ptr = NULL;
              return;   /* exit from stk_proactive_polling() */
            }
            else
            {
              /* process_sim_refresh() frees signal  sim_data.context_switch_ptr->sig_ptr */
              /* this thread has finished with the context switch so free it.             */
              MFREE (sim_data.context_switch_ptr); 
              sim_data.context_switch_ptr = NULL;
            }
#endif /* TI_PS_FF_AT_P_CMD_CUST */
            break; //end of case STK_REFRESH

#ifdef FF_SAT_E
          case STK_RECEIVE_DATA:
            if (stk_dti_bip_receive_data((T_sdu*)&sim_toolkit_ind->stk_cmd, res_code[0]))
            {
              /*
               * message contains TLV elements to display
               * so forward it to MMI
               */
              sim_data.sat_session = TRUE;

              TRACE_EVENT("SEND receive data indication to MMI");
              PSENDX (MMI, sim_toolkit_ind);
              TRACE_EVENT("receive data indication to MMI sent");
            }
            else
            {
              PFREE (sim_toolkit_ind);
            }
            break;

          case STK_SEND_DATA:
            if (stk_dti_bip_send_data((T_sdu*)&sim_toolkit_ind->stk_cmd))
            {
              /*
               * message contains TLV elements to display
               * so forward it to MMI
               */
              sim_data.sat_session = TRUE;
              PSENDX (MMI, sim_toolkit_ind);
            }
            else
            {
              PFREE (sim_toolkit_ind);
            }
          break;
#endif /* FF_SAT_E */

          case STK_TIMER_MANAGEMENT:
          {
            UBYTE tag_tv[8] = {STK_TIMER_ID_TAG|STK_COMPREHENSION_REQUIRED, STK_TIMER_ID_LEN, 0,
                               STK_TIMER_VALUE_TAG, STK_TIMER_VALUE_LEN,
                               0, 0 ,0};
            tag_tv[2] = found_tag[2][0];

            if ((unsigned)(i = (int)found_tag[2][0] - 1) >= MAX_SAT_TIMER)
            {               /* invalid Timer identifier */
              res_code[0] = STK_RES_ERR_CMD_DATA;
              resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                             res_code, 1, NULL, 0);
#ifdef _SIMULATION_
              TRACE_EVENT("stk_build_response No 17");
#endif
            }
            /*
             * Process TIMER MANAGEMENT according to command qualifier
             */
            else switch (cmd_qual)
            {
            case 0:
              if (found_tag[3] EQ NULL)
              {
                res_code[0] = STK_RES_ERR_MISS_VALUE;
                resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                TRACE_EVENT("TM: missing parameters");
#endif
                break;
              }
              /* translate timer value into unit of seconds */
              tm_val = ((T_TIME)BCD2INT(found_tag[3][0])  * 60 + // hours
                        (T_TIME)BCD2INT(found_tag[3][1])) * 60 + // minutes
                        (T_TIME)BCD2INT(found_tag[3][2]);        // seconds
//TISH, patch for OMAPS00115011, 2007-02-12
//start
		if (tm_val==0)
		{
                res_code[0] = STK_RES_ERR_CMD_DATA;
                resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
		  TRACE_ERROR("TR wrong tm value");
                break;
		}
//end
              sim_data.timer[i].hour = found_tag[3][0];
              sim_data.timer[i].minute = found_tag[3][1];
              sim_data.timer[i].second = found_tag[3][2];

              TIMER_START (sim_handle, (USHORT)(i+1), (tm_val * 1000 - TIMER_LATENCY));
              sim_data.timer[i].active = TRUE;
              resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                             res_code, 1, tag_tv, 3);
#ifdef _SIMULATION_
              TRACE_EVENT("TM: set timer");
#endif
              break;
            case 1:
            case 2:
              if (sim_data.timer[i].active)
              {
                TIMER_STATUS (sim_handle, (USHORT)(i + 1), &tm_val);
                tm_val /= 1000;
                tag_tv[7] = INT2BCD((UBYTE)(tm_val % 60));
                tm_val /= 60;
                tag_tv[6] = INT2BCD((UBYTE)(tm_val % 60));
                tm_val /= 60;
                if (tm_val < 24)
                  tag_tv[5] = INT2BCD((UBYTE)tm_val);
                else
                {             // set maximum value
                  tag_tv[7] = tag_tv[6] = 0x59;
                  tag_tv[5] = 0x23;
                }
                if (cmd_qual EQ 1)  /* timer to be stopped */
                {
                  TIMER_STOP (sim_handle, (USHORT)(i + 1));
                  sim_data.timer[i].active = FALSE;
                }
                resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, tag_tv, 8);
#ifdef _SIMULATION_
                TRACE_EVENT("TM: stop/query timer");
#endif
              }
              else
              {
                res_code[0] = STK_RES_BUSY_TIMER_STATE;
                resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                               res_code, 1, NULL, 0);
#ifdef _SIMULATION_
                TRACE_EVENT("TM: timer not running");
#endif
              }
              break;
            default:
              /*
               * not supported information request
               */
              res_code[0] = STK_RES_ERR_CMD_TYPE;
              resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                             res_code, 1, NULL, 0);
#ifdef _SIMULATION_
              TRACE_EVENT("TM: unknown qualifier");
#endif
              break;
            }
            FKT_TerminalResponse (p->response, (USHORT)resp_len);
            PFREE (sim_toolkit_ind);
            break;
          }
          default:
            result = FALSE;
            res_code[0] = STK_RES_ERR_CMD_TYPE; /* command type not understood */
            /*
             * fall through
            */
          case STK_POLLING_OFF:
            /*
             * SIM toolkit cancels the effect of a previous
             * POLL INTERVAL command. Polling of the SIM
             * toolkit is done now all thirty seconds with the
             * STATUS command, when in a call.
             */
            if (result AND SIM_IS_FLAG_SET (PRO_ACTIVE_SIM))
            {
              TRACE_EVENT("Idle Polling switched off");
              sim_data.idle_polling = FALSE;
             /* there will not be a status command after this .so just force one status command to enable sleep */  
			  startTimerPollOff = TRUE; 
              if(SIM_IS_FLAG_SET(CALL_ACTIVE))
              {
               /* SIM_TIMER will be restarted, during a call, for
                * presence detection
                */
                sim_data.status_time  = THIRTY_SECONDS;
              }
              else
              {
                /* timer value set to max, for with a '0', the timer would
                   run for about one tick
                 */
                sim_data.status_time  = 0xFFFFFFFF;
                TIMER_STOP (sim_handle, SIM_TIMER);
              }

            }
          case STK_MORE_TIME:
            /*
             * SIM toolkit needs more time.
             * SIM application sends terminal response with OK.
             */
            resp_len = stk_build_response (p->response, p_cmd, cmd_len,
                                           res_code, 1, NULL, 0);
#ifdef _SIMULATION_
            TRACE_EVENT("MORE_TIME");
#endif
            FKT_TerminalResponse (p->response, (USHORT)resp_len);
            PFREE (sim_toolkit_ind);
            break;
        }
      }
      else
      {
#ifdef _SIMULATION_
        TRACE_EVENT("FKT_Fetch_NEQ_SIM_NO_ERROR");
#endif
        PFREE (sim_toolkit_ind);
      }
      MFREE (p);

      if (vsi_c_status (VSI_CALLER &in_queue, &out_queue) EQ VSI_OK)
      {
        if (in_queue > 0) break; /* break while */
      }      
    } /* END while */
    /*
     * send end of SAT session indicator
     */
    TRACE_FUNCTION ("stk_proactive_polling() send end of SAT session indicator");
    if ((sim_data.term_resp_sent) AND (sim_data.sat_session))
    {
      PALLOC (sim_toolkit_ind, SIM_TOOLKIT_IND);
      memset (sim_toolkit_ind, 0, sizeof (T_SIM_TOOLKIT_IND));
#ifdef _SIMULATION_
      TRACE_EVENT("SAT session ended");
#endif
      sim_data.sat_session    = FALSE;
      sim_data.term_resp_sent = FALSE;
      PSENDX (MMI, sim_toolkit_ind);
    }
  }
  TRACE_FUNCTION ("stk_proactive_polling() exited");
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_stop_all_sat_timers    |
+--------------------------------------------------------------------+

  PURPOSE : Stop all timers started by TIMER MANAGEMENT, if running.
            Required when card is deactivated or reset.

*/

GLOBAL void stk_stop_all_sat_timers (void)
{
  int i;

  for (i = 0; i < MAX_SAT_TIMER; i++)
  {
    if (sim_data.timer[i].active)
    {
      TIMER_STOP (sim_handle, (USHORT)(i + 1));
      sim_data.timer[i].active = FALSE;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_start_timer_and_poll   |
+--------------------------------------------------------------------+

  PURPOSE : Start STATUS timer and control SIM Toolkit Polling

*/

LOCAL void stk_start_timer_and_poll (void)
{
  T_TIME t_val;
  /*
   * start status timer if SIM is inserted
   * for periodic status polling of SIM Toolkit
   */
  if ((SIM_IS_FLAG_SET (PRO_ACTIVE_SIM) AND
       sim_data.idle_polling AND
       SIM_IS_FLAG_CLEARED (TEST_MODE_POLLING)) OR
       SIM_IS_FLAG_SET (CALL_ACTIVE))
  {
  /*
   * Start Status Polling
   */
    t_val = (SIM_IS_FLAG_SET (CALL_ACTIVE) AND
             sim_data.status_time > THIRTY_SECONDS)?
            THIRTY_SECONDS: sim_data.status_time;

    TIMER_PSTART (sim_handle, SIM_TIMER, t_val, t_val);
  }
  sim_data.chk_sat_avail = TRUE;
//  stk_proactive_polling();
}

#ifdef FF_SAT_E
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_handle_ccd_error       |
+--------------------------------------------------------------------+

  PURPOSE : Handle CCD error causes.

*/

LOCAL void stk_handle_ccd_error(UBYTE notOK,
                                UBYTE* status,
                                UBYTE* general_result,
                                UBYTE* add_info_result)
{
  UBYTE  ccd_error;
  USHORT parameter_list[MAX_ERR_PAR];
  TRACE_EVENT_P1("stk_handle_ccd_error(): %d", notOK);
  /*
   * handle errors in list
   */
  memset (parameter_list, 0, sizeof(parameter_list));
  ccd_error = ccd_getFirstError(CCDENT_SAT, parameter_list);

  if (notOK EQ ccdWarning)
  {
    while (ccd_error NEQ ERR_NO_MORE_ERROR)
    {
      switch (ccd_error)
      {
      case ERR_COMPREH_REQUIRED:
        /*
         * comprehension required
         */
        TRACE_ERROR("CCD_WARNING=ERR_COMPREH_REQUIRED");
        if (*status EQ SIM_CCD_OK)
        {
          *general_result  = RSLT_UNKN_DATA;
          *add_info_result = ADD_NO_CAUSE;
          *status          = SIM_CCD_RETURN;
        }
        break;

      case ERR_IE_NOT_EXPECTED:
        /*
         * unexpected element
         */
        TRACE_ERROR("CCD_WARNING=ERR_IE_NOT_EXPECTED");
        if (*status EQ SIM_CCD_OK)
        {
          *general_result  = RSLT_PERF_PART_CMPR;
          *add_info_result = ADD_NO_CAUSE;
        }
        break;

      case ERR_MAND_ELEM_MISS:
        /*
         * mandatory elements missing
         */
        TRACE_ERROR("CCD_WARNING=ERR_MAND_ELEM_MISS");
        *general_result  = RSLT_ERR_REQ_VAL;
        *add_info_result = ADD_NO_CAUSE;
        *status          = SIM_CCD_RETURN;
        break;

      case ERR_MSG_LEN:
        TRACE_ERROR("CCD_WARNING=ERR_MSG_LEN (TC_509B)");
        *general_result  = RSLT_PERF_SUCCESS;
        *add_info_result = ADD_NO_CAUSE;
        *status          = SIM_CCD_OK;
        break;

      default:
        TRACE_ERROR("CCD_WARNING ignored");
        TRACE_EVENT_P1("ccd_error=0x%02X", ccd_error);
        break;
      }
      memset (parameter_list,0, sizeof(parameter_list));
      ccd_error = ccd_getNextError (CCDENT_SAT, parameter_list);
    }
  }
  else /* ccdError */
  {
    while (ccd_error NEQ ERR_NO_MORE_ERROR)
    {
      switch (ccd_error)
      {
      case ERR_COMPREH_REQUIRED:
        /*
         * comprehension required
         */
        TRACE_ERROR("CCD_ERROR=ERR_COMPREH_REQUIRED");
        if (*status EQ SIM_CCD_OK)
        {
          *general_result  = RSLT_UNKN_DATA;
          *add_info_result = ADD_NO_CAUSE;
          *status          = SIM_CCD_RETURN;
        }
        break;

      case ERR_IE_NOT_EXPECTED:
        /*
         * unexpected element
         */
        TRACE_ERROR("CCD_ERROR=ERR_IE_NOT_EXPECTED");
        if (*status EQ SIM_CCD_OK)
        {
          *general_result  = RSLT_PERF_PART_CMPR;
          *add_info_result = ADD_NO_CAUSE;
        }
        break;

      case ERR_MAND_ELEM_MISS:
        /*
         * mandatory elements missing
         */
        TRACE_ERROR("CCD_ERROR=ERR_MAND_ELEM_MISS");
        *general_result  = RSLT_ERR_REQ_VAL;
        *add_info_result = ADD_NO_CAUSE;
        *status          = SIM_CCD_RETURN;
        break;

      case ERR_MSG_LEN:
        TRACE_ERROR("CCD_ERROR=ERR_MSG_LEN");
        *general_result  = RSLT_UNKN_DATA;
        *add_info_result = ADD_NO_CAUSE;
        *status          = SIM_CCD_DISCARD;
        break;

      default:
        TRACE_ERROR("CCD_ERROR ignored");
        TRACE_EVENT_P1("ccd_error=0x%02X", ccd_error);
        break;
      }
      memset (parameter_list,0, sizeof(parameter_list));
      ccd_error = ccd_getNextError (CCDENT_SAT, parameter_list);
    }
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_bip_decode_stk_command |
+--------------------------------------------------------------------+

  PURPOSE : First step of decoding SEND DATA and RECEIVE DATA message.

*/

LOCAL void stk_bip_decode_stk_command(T_sdu*          message,
                                      T_cmd_details*  cmd_details,
                                      UBYTE*          status,
                                      UBYTE*          general_result,
                                      UBYTE*          add_info_result)
{
  UBYTE ccdRet;
  MCAST(stk_cmd, STK_CMD);
  memset(stk_cmd, 0, sizeof(T_STK_CMD));
  /*
   * decode toolkit command
   */   
  ccdRet = ccd_decodeMsg (CCDENT_SAT,
                          DOWNLINK,
                          (T_MSGBUF *) message,
                          (UBYTE    *) _decodedMsg,
                          STK_CMD);

#ifdef _SIMULATION_
  TRACE_EVENT_P1("ccdRet@bip_decode_stk_command: %d", ccdRet);
#endif 

  if ((stk_cmd->v_pas_cmd) AND (stk_cmd->pas_cmd.v_cmd_details))
  {
    /*
     * store command details
     */
    *cmd_details = stk_cmd->pas_cmd.cmd_details;

    if (ccdRet NEQ ccdOK)
    {
      /*
       * handle errors in list
       */
      stk_handle_ccd_error(ccdRet, status, general_result, add_info_result);
    }
    if ((stk_cmd->pas_cmd.v_cmd_prms EQ FALSE) OR
       (stk_cmd->pas_cmd.v_dev_ids EQ FALSE))
    {
      /*
       * no Channel Data (Length) element present or
       * no Device Identities element present
       * Error, required values are missing
       */
      *general_result  = RSLT_ERR_REQ_VAL;
      *add_info_result = ADD_NO_CAUSE;
      *status          = SIM_CCD_RETURN;
    }
    if (*status EQ SIM_CCD_OK)
    {
      int i = 0;
      while (tbl_device_src_id[i])
      {
        if (stk_cmd->pas_cmd.dev_ids.dest_dev EQ tbl_device_src_id[i])
         break;

        i++;
      }
      if (tbl_device_src_id[i] EQ 0)
      {
        /*
         * device id not valid
         */
        *general_result  = RSLT_UNKN_DATA;
        *add_info_result = ADD_NO_CAUSE;
        *status          = SIM_CCD_RETURN;
      }
      else if /* norm sim_data.bip_ch_id for 11.14/12.7 conform */
          ((sim_data.bip_state EQ SIM_BIP_CLOSED) OR
         (stk_cmd->pas_cmd.dev_ids.dest_dev NEQ (sim_data.bip_ch_id | 0x020)))
      {
        /*
         * BIP error, Channel identifier not valid
         */
        *general_result  = RSLT_BEARIND_PERR;
        *add_info_result = ADD_BIP_CHANID_NT_VLD;
        *status          = SIM_CCD_RETURN;
      }
      else
      {
        /*
         * store command parameters
         */
        sim_data.bip_cmd_prms = stk_cmd->pas_cmd.cmd_prms;
      }
    }
  }
  else
  {
    /*
     * incomplete message
     */
    *general_result  = RSLT_ERR_REQ_VAL;
    *add_info_result = ADD_NO_CAUSE;
    *status          = SIM_CCD_RETURN;
    /*
     * set Command Details object values to 0x00
     */
    cmd_details->cmd_nr  = 0;
    cmd_details->cmd_typ = 0;
    cmd_details->cmd_qlf = 0;
  }
} /* stk_bip_decode_stk_command */

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                             |
| STATE   : code                ROUTINE : stk_dti_send_data                   |
+-----------------------------------------------------------------------------+

  PURPOSE : Create and send a DTI Data primitive.

*/

LOCAL void stk_dti_send_data()
{
  T_desc2* temp_desc1;
  /*
   * send DTI data primitive
   */
  PALLOC_DESC2 (dti_data_ind, DTI2_DATA_IND);

#ifdef _SIMULATION_
  dti_data_ind->parameters.p_id       = DTI_PID_UOS;
  dti_data_ind->parameters.st_lines.st_flow      = DTI_FLOW_ON;
  dti_data_ind->parameters.st_lines.st_line_sa   = DTI_SA_ON;
  dti_data_ind->parameters.st_lines.st_line_sb   = DTI_SB_ON;
  dti_data_ind->parameters.st_lines.st_break_len = DTI_BREAK_OFF;
#endif /* _SIMULATION_ */
  switch(sim_data.con_type)
  {

    case SIM_CON_TYPE_UDP:
      MALLOC(temp_desc1, (USHORT)(sizeof(T_desc2) - 1 +
                                  sizeof(T_SRC_DES)));
      memcpy(temp_desc1->buffer,
             &sim_data.udp_parameters,
             sizeof(T_SRC_DES));/*lint !e419  Apparent data overrun for function*/
      temp_desc1->len                = sizeof(T_SRC_DES);
      temp_desc1->next               = sim_data.data_to_send.first;
      sim_data.data_to_send.first    = (ULONG)temp_desc1;
      sim_data.data_to_send.list_len+= temp_desc1->len;
      break;

    case SIM_CON_TYPE_IP:
      dti_data_ind->parameters.p_id = DTI_PID_IP;
      break;

    case SIM_CON_TYPE_SERIAL:
      dti_data_ind->parameters.st_lines.st_flow      = DTI_FLOW_ON;
      dti_data_ind->parameters.st_lines.st_line_sa   = DTI_SA_ON;
      dti_data_ind->parameters.st_lines.st_line_sb   = DTI_SB_ON;
      dti_data_ind->parameters.st_lines.st_break_len = DTI_BREAK_OFF;
      break;
  }
  dti_data_ind->desc_list2       = sim_data.data_to_send;
  sim_data.data_to_send.first    = (ULONG)NULL;
  sim_data.data_to_send.list_len = 0;
  dti_send_data(sim_data.hDTI, 0, 0, 0, dti_data_ind);
} /* stk_dti_send_data() */

/*
+-----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                             |
| STATE   : code                ROUTINE : stk_bip_send_data_terminal_response |
+-----------------------------------------------------------------------------+

  PURPOSE : Send a Terminal Response message for a SEND DATA message.

*/

LOCAL void stk_bip_send_data_terminal_response(UBYTE general_result,
                                               UBYTE add_info_result)
{
  T_sdu*  temp_sdu;
  UBYTE   ccdRet;

  /*
   * memory for encoded message
   */
  MALLOC(temp_sdu, (USHORT)(sizeof(T_sdu) - 1 + MAX_STK_CMD));
  temp_sdu->l_buf = MAX_STK_CMD << 3;
  temp_sdu->o_buf = 0;

  CCD_START;
  {
    MCAST(term_resp, TERM_RESP);
    memset(term_resp, 0, sizeof(T_TERM_RESP));

    /*
     * set Command details
     */
    term_resp->v_cmd_details = TRUE;
    term_resp->cmd_details   = sim_data.bip_tx_cmd_details;
    /*
     * set set Device identities
     */
    term_resp->v_dev_ids        = TRUE;
    term_resp->dev_ids.src_dev  = DEV_SRC_ME;
    term_resp->dev_ids.dest_dev = DEV_DST_SIM;
    /*
     * set Result
     */
    term_resp->v_res   = TRUE;
    term_resp->res.gen = general_result;
    switch(general_result)
    {
      case RSLT_ME_UNAB_PROC:
      case RSLT_NTW_UNAB_PROC:
      case RSLT_LABRWS_GENERIC:
      case RSLT_SS_ERR:
      case RSLT_SMS_ERR:
      case RSLT_USSD_ERR:
      case RSLT_CC_SIM_PRM:
      case RSLT_BEARIND_PERR:
        /*
         * one byte for additional information
         */
        term_resp->res.v_add        = TRUE;
        term_resp->res.add.l_add    = 1 << 3;
        term_resp->res.add.o_add    = 0;
        term_resp->res.add.b_add[0] = add_info_result;
        break;
    }
    /*
     * set Channel Data Length
     */
    term_resp->v_chan_dat_lth = TRUE;
    if ((SIM_CLASS_E_BUFFER_SIZE - sim_data.data_to_send.list_len) > 255)
    {
      /*
       * more than 255 bytes are available in TX buffer
       */
      term_resp->chan_dat_lth = 0xff;
    }
    else
    {
      term_resp->chan_dat_lth = SIM_CLASS_E_BUFFER_SIZE -
                                sim_data.data_to_send.list_len;
    }
    /*
     * encode message
     */
    ccdRet = ccd_codeMsg (CCDENT_SAT,
                          UPLINK,
                          (T_MSGBUF *) temp_sdu,
                          (UBYTE    *) _decodedMsg,
                          TERM_RESP);
  }
  CCD_END;
  /*
   * send Terminal Response
   */
  if (ccdRet NEQ ccdOK)
  {
    TRACE_EVENT_P1("SEND DATA: CCD Coding Error: %d",ccdRet );
  }
  else
  {
    FKT_TerminalResponse (temp_sdu->buf, (USHORT)(temp_sdu->l_buf >> 3));
  }
  MFREE(temp_sdu);
} /* stk_bip_send_data_terminal_response() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_close_dti_connection   |
+--------------------------------------------------------------------+

  PURPOSE : Close DTI connection.

  PARAMETER: The open parameter indicates if the DTI connection is
             still open or already closed.

*/

LOCAL void stk_close_dti_connection(UBYTE close)
{
#ifdef _SIMULATION_
  TRACE_FUNCTION ("stk_close_dti_connection()");
#endif
#ifdef _SIMULATION_
  TRACE_EVENT_P1(" |___-->: sim_data.dti_connection_state=0x%02X", sim_data.dti_connection_state);
#endif
  if (sim_data.dti_connection_state NEQ SIM_DTI_CONNECTION_CLOSED)
  {
     /*
     * close UDP connection
     */
    if (sim_data.con_type EQ SIM_CON_TYPE_UDP)
    {
      /*
       * close currently used port
       */
      if ((sim_data.dti_connection_state NEQ SIM_DTI_CONNECTION_BIND) AND
         (hCommUDP >= VSI_OK))
      {
        PALLOC(udp_closeport_req, UDP_CLOSEPORT_REQ);
        udp_closeport_req->port = sim_data.udp_parameters.src_port[0];
        udp_closeport_req->port = (udp_closeport_req->port << 8);
        udp_closeport_req->port+= sim_data.udp_parameters.src_port[1];
        PSEND(hCommUDP, udp_closeport_req);
      }
      /*
       * release VSI channel
       */
      if (hCommUDP >= VSI_OK)
      {
        vsi_c_close (VSI_CALLER hCommUDP);
      }
      hCommUDP = VSI_ERROR;
    }
    /*
     * disconnect BIP channel
     */
    if (sim_data.bip_state EQ SIM_BIP_CONNECTED)
    {
      sim_data.bip_state = SIM_BIP_OPEN;
      /*
       * stop release timer
       */
      if (sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED)
      {
        if (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
        {
#ifdef _SIMULATION_
          TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
          TIMER_STOP (sim_handle, SIM_BIP_TIMER);
        }
        sim_data.bip_timer_state = SIM_BIP_TIMER_DISCONNECTED;
      }
    }
    /*
     * close DTI connection
     */
    sim_data.dti_connection_state = SIM_DTI_CONNECTION_CLOSED;
    if (close)
    {
      dti_close(sim_data.hDTI, 0, 0, 0, FALSE);
    }
#ifdef _SIMULATION_
TRACE_EVENT_P1("stk_close_dti_connection: sim_data.dti_connection_state=0x%02X", sim_data.dti_connection_state);
#endif
  }
} /* stk_close_dti_connection() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_close_bip_channel      |
+--------------------------------------------------------------------+

  PURPOSE : Close Bearer Independent Protocol Channel.

*/

LOCAL void stk_close_bip_channel(UBYTE general_result,
                                 UBYTE add_info_result)
{
  T_desc2* temp_desc1;
  T_desc2* temp_desc2;

  if (sim_data.bip_state NEQ SIM_BIP_CLOSED)
  {
    /*
     * set BIP to close
     */
    sim_data.bip_state = SIM_BIP_CLOSED;
    /*
     * stop release timer if used
     */
    if (sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED)
    {
      if (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
      {
#ifdef _SIMULATION_
        TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
        TIMER_STOP (sim_handle, SIM_BIP_TIMER);
      }
      sim_data.bip_timer_state = SIM_BIP_TIMER_DISCONNECTED;
    }
    /*
     * send Terminal Response
     */
    if (sim_data.bip_tx_state EQ SIM_BIP_TX_SEND)
    {
      stk_bip_send_data_terminal_response(general_result, add_info_result);
      /*
       * close channel is not triggered from a proactive_polling function call
       * so adjust timer and start polling
       */
      stk_start_timer_and_poll();
    }
    /*
     * release suspension
     */
    sim_data.bip_suspend = FALSE;
    /*
     * release RX and TX buffer
     */
    temp_desc1 = (T_desc2*)sim_data.data_to_send.first;
    while(temp_desc1)
    {
      temp_desc2 = temp_desc1;
      temp_desc1 = (T_desc2*)temp_desc1->next;
      MFREE(temp_desc2);
    }
    sim_data.data_to_send.first         = (ULONG)NULL;
    sim_data.data_to_send.list_len      = 0;
    sim_data.prev_data_to_send.first    = (ULONG)NULL;
    sim_data.prev_data_to_send.list_len = 0;
#ifdef _SIMULATION_
    TRACE_EVENT("bip_tx_state = IDLE");
#endif
    sim_data.bip_tx_state               = SIM_BIP_TX_IDLE;
    temp_desc1 = (T_desc2*)sim_data.received_data.first;
    while(temp_desc1)
    {
      temp_desc2 = temp_desc1;
      temp_desc1 = (T_desc2*)temp_desc1->next;
      MFREE(temp_desc2);
    }
    sim_data.received_data.first    = (ULONG)NULL;
    sim_data.received_data.list_len = 0;
    sim_data.received_data_pos      = 0;
#ifdef _SIMULATION_
    TRACE_EVENT("bip_rx_state = IDLE");
#endif
    sim_data.bip_rx_state           = SIM_BIP_RX_IDLE;
  }
} /* stk_close_bip_channel() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_inform_mmi         |
+--------------------------------------------------------------------+

  PURPOSE : Informs MMI if the requested actions have been done.

*/

LOCAL void stk_dti_inform_mmi (UBYTE dti_conn_req, UBYTE bip_conn_req )
{
  UBYTE dti_conn_done;
  UBYTE bip_conn_done;
#if _SIMULATION_
  TRACE_FUNCTION ("stk_dti_inform_mmi()");
#endif
  /*
   * set stutus
   */
  dti_conn_done = 0;
  bip_conn_done = 0;

  if (sim_data.bip_state EQ SIM_BIP_CLOSED)
  {
    bip_conn_done |= SIM_BIP_CLOSE_CHANNEL;
  }
  else
  {
    bip_conn_done |= SIM_BIP_OPEN_CHANNEL;
    /*
     * Only if BIP channel not closed, deal with its suspension state
     */   
    if (sim_data.bip_suspend)
    {
      bip_conn_done |= SIM_BIP_CHANNEL_SUSPENDED;
    }
    else
    {
      bip_conn_done |= SIM_BIP_CHANNEL_RESUMED;
    }
  }
  
  if (sim_data.dti_connection_state  EQ SIM_DTI_CONNECTION_OPEN)
  {
    dti_conn_done = SIM_DTI_CONNECT;
  }

  if (sim_data.dti_connection_state  EQ SIM_DTI_CONNECTION_CLOSED)
  {
    dti_conn_done = SIM_DTI_DISCONNECT;
  }

  /*
   * inform MMI
   */
  if (  /* reguirements REALLY fullfilled in code, then confirm or indicate */
        /* else no reaction at all !! */
       (((bip_conn_req & bip_conn_done) EQ bip_conn_req) OR (bip_conn_req EQ SIM_BIP_UNKNOWN)) AND
        ((dti_conn_req EQ dti_conn_done)OR (dti_conn_req EQ SIM_DTI_UNKNOWN)) AND
       (~(UCHAR)((dti_conn_req EQ SIM_DTI_UNKNOWN) AND (bip_conn_req EQ SIM_BIP_UNKNOWN)))
     ) 
  {
     if (sim_data.sim_dti_req)
     {
        /*
         * send DTI confirm primitive
         */
        PALLOC(sim_dti_cnf, SIM_DTI_CNF);
        sim_dti_cnf->link_id   = sim_data.sim_dti_req->link_id;
        sim_dti_cnf->dti_conn  = dti_conn_done;
        PSEND(hCommMMI, sim_dti_cnf);
        /*
         * free request primitive
         */
        PFREE(sim_data.sim_dti_req);
        sim_data.sim_dti_req = NULL;
      }
      else if (sim_data.sim_bip_req)
      {
        /*
         * send BIP confirm primitive
         */
        PALLOC(sim_bip_cnf, SIM_BIP_CNF);
        sim_bip_cnf->bip_ch_id = sim_data.sim_bip_req->bip_ch_id;
        sim_bip_cnf->bip_conn  = bip_conn_done;
        PSEND(hCommMMI, sim_bip_cnf);
        /*
         * free request primitive
         */
        PFREE(sim_data.sim_bip_req);
        sim_data.sim_bip_req = NULL;
      }
      else
      {
        /*
         * send indication primitive
         */
        PALLOC(sim_dti_bip_ind, SIM_DTI_BIP_IND);
        sim_dti_bip_ind->link_id   = sim_data.link_id;
        sim_dti_bip_ind->dti_conn  = dti_conn_done;        
        sim_dti_bip_ind->bip_ch_id = sim_data.bip_ch_id;
        sim_dti_bip_ind->bip_conn  = bip_conn_done;
        PSEND(hCommMMI, sim_dti_bip_ind);
      }
  } /* and if (requirements fullfilled) */
} /* stk_dti_inform_mmi() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_connection_opened  |
+--------------------------------------------------------------------+

  PURPOSE : Handle CONNECTION OPENED signal from DTILIB.

*/

GLOBAL void stk_dti_connection_opened (void)
{
  TRACE_FUNCTION ("stk_dti_connection_opened()");

  switch(sim_data.dti_connection_state)
  {
    case SIM_DTI_CONNECTION_SETUP:
      sim_data.dti_connection_state = SIM_DTI_CONNECTION_OPEN;
      /*
       * send confirm primitive
       */
      stk_dti_inform_mmi(sim_data.sim_dti_req->dti_conn, (UBYTE) SIM_BIP_UNKNOWN);
      /* fall through */
    case SIM_DTI_CONNECTION_OPEN:
      /*
       * connect BIP with DTI
       */
      if (sim_data.bip_state EQ SIM_BIP_OPEN)
      {
        sim_data.bip_state = SIM_BIP_CONNECTED;
        if (sim_data.bip_release_time EQ SIM_NO_AUTO_RELEASE)
        {
          sim_data.bip_timer_state = SIM_BIP_TIMER_NOT_USED;
        }
        else if (sim_data.bip_suspend)
        {
          sim_data.bip_timer_state = SIM_BIP_TIMER_SUSPENDED;
        }
        else
        {
          sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
        }
      }
      break;

    default:
      /*
       * wrong state
       * so close DTI connection
       * and inform MMI
       */
      TRACE_ERROR("DTI_CONNECTION_OPENED in wrong state");
      stk_close_dti_connection(TRUE);
      stk_dti_inform_mmi(SIM_DTI_DISCONNECT, (UBYTE) SIM_BIP_UNKNOWN);
      break;
  }
  /*
   * reset RX, TX and timer
   */
  sim_data.dti_rx_state = SIM_DTI_RX_IDLE;
  sim_data.dti_tx_state = SIM_DTI_TX_IDLE;
#if 0 /*###jk:OK?*/
  if (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
  {
    sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_
    TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
    TIMER_STOP (sim_handle, SIM_BIP_TIMER);
  }
  /*
   * update timer and DTI states
   */
  if ((sim_data.bip_state EQ SIM_BIP_CONNECTED) AND
     (sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE))
  {
    /*
     * start reception
     */
    sim_data.dti_rx_state = SIM_DTI_RX_READY;
    dti_start(sim_data.hDTI, 0, 0, 0);
    /*
     * start timer
     */
    if ((sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED) AND
        (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE)
       )
    {
      /*
       * no data trafic on the BIP channel,
       * so use the timer
       */
      sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
      TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_connection_opened()");
#endif
      TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
    }
  }
 #endif /*###jk:OK?*/
} /* stk_dti_connection_opened() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_connection_closed  |
+--------------------------------------------------------------------+

  PURPOSE : Handle CONNECTION CLOSED signal from DTILIB.

*/

GLOBAL void stk_dti_connection_closed (void)
{
  TRACE_FUNCTION ("stk_dti_connection_closed()");

  /*
   * close DTI connection
   */
  stk_close_dti_connection(FALSE);
  /*
   * inform MMI about disconnection
   */
  stk_dti_inform_mmi(SIM_DTI_DISCONNECT, (UBYTE) SIM_BIP_UNKNOWN);
} /* stk_dti_connection_closed() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_data_received      |
+--------------------------------------------------------------------+

  PURPOSE : Deal with incomming DTI data primitives.

*/

GLOBAL void stk_dti_data_received (T_DTI2_DATA_IND* dti_data_ind)
{
  UBYTE   ccdRet;
  UBYTE   dummy[4];
  T_sdu*  temp_sdu;
  T_desc2* temp_desc1;
  T_desc2* temp_desc2;

 TRACE_FUNCTION ("stk_dti_data_received()");

#ifdef _SIMULATION_
  /*
   * copy bytes of T_SRC_DES struct in extra descriptor
   * this is a requirement of the DTI communication with UDP, but it should
   * not have any side affects in case of communication with other entities
   */
  if ((dti_data_ind->desc_list2.first NEQ (ULONG)NULL) AND
     (dti_data_ind->desc_list2.list_len >= sizeof(T_SRC_DES)))
  {
    T_desc2 *old_desc, *addr_desc, *test_desc, *next_desc;
    USHORT  i, j;

    old_desc = (T_desc2 *)dti_data_ind->desc_list2.first;

    /*
     * build the T_SRC_DES for IP-addresses and ports
     */
    MALLOC(addr_desc, (USHORT)(sizeof(T_desc2) - 1 + sizeof(T_SRC_DES)));
    addr_desc->len = sizeof(T_SRC_DES);
    j = 0;
    for(i=0; i < addr_desc->len; i++)
    {
      while(j >= old_desc->len)
      {
        next_desc = (T_desc2*)old_desc->next;
        MFREE(old_desc);
        old_desc = next_desc;
        j = 0;
      }
      addr_desc->buffer[i] = old_desc->buffer[j];
      j++;
    }

    /*
     *  Build the desc for the data
     */
    if (j < old_desc->len)
    {
      MALLOC(test_desc, (USHORT)(sizeof(T_desc2) - 1 + old_desc->len - j));
      test_desc->len  = old_desc->len - j;
      test_desc->next = old_desc->next;
      for(i=0; i < test_desc->len; i++)
      {
        test_desc->buffer[i] = old_desc->buffer[j];
        j++;
      }
    }
    else
    {
      test_desc = (T_desc2*)old_desc->next;
    }
    MFREE(old_desc);

    dti_data_ind->desc_list2.first = (ULONG)addr_desc;
    addr_desc->next = (ULONG)test_desc;

  }
#endif /* _SIMULATION_ */
  /*
   * take data
   */
  temp_desc1 = (T_desc2*)dti_data_ind->desc_list2.first;
  /*
   * free primitive
   */
  PFREE(dti_data_ind);
  /*
   *
   */
  switch(sim_data.con_type)
  {
/* --67 asd; ###jk test error for image*/
    case SIM_CON_TYPE_UDP:
      /*
       * free first descriptor
       */
      temp_desc2 = temp_desc1;
      if (temp_desc1)
      {
        temp_desc1 = (T_desc2*)temp_desc1->next;
      }
      MFREE(temp_desc2);
      /* fall through */
    case SIM_CON_TYPE_IP:
      /*
       * store data
       */
      if (sim_data.received_data.first EQ (ULONG)NULL)
      {
        sim_data.received_data.first = (ULONG)temp_desc1;
      }
      else
      {
        /*
         * error
         * free received data
         */
        TRACE_ERROR("DTI data received, but still data in RX buffer");
        while(temp_desc1)
        {
          temp_desc2 = temp_desc1;
          temp_desc1 = (T_desc2*)temp_desc1->next;
          MFREE(temp_desc2);
        }
        temp_desc1 = NULL;
      }
      break;

    case SIM_CON_TYPE_SERIAL:
      /*
       * store data
       */
      if (sim_data.received_data.first EQ (ULONG)NULL)
      {
        sim_data.received_data.first = (ULONG)temp_desc1;
      }
      else
      {
        /*
         * error, but concatinate data
         */
        TRACE_ERROR("DTI data received, but still data in RX buffer");
        /*
         * find last descriptor
         */
        temp_desc2 = (T_desc2*)sim_data.received_data.first;
        while(temp_desc2->next NEQ (ULONG)NULL)
        {
          temp_desc2 = (T_desc2*)temp_desc2->next;
        }
        temp_desc2->next = (ULONG)temp_desc1;
      }
      break;
  }
  /*
   * update list length
   */
  while(temp_desc1)
  {
    sim_data.received_data.list_len+= temp_desc1->len;
    temp_desc1                      = (T_desc2*)temp_desc1->next;
  }

  if (sim_data.received_data.list_len)
  {
    /*
     * change state of BIP RX and stop timer
     */
#ifdef _SIMULATION_
    TRACE_EVENT("bip_rx_state = DATA");
#endif
    sim_data.bip_rx_state = SIM_BIP_RX_DATA;
    if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
        (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
       )
    {
      sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_
      TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
      TIMER_STOP (sim_handle, SIM_BIP_TIMER);
    }
    /*
     * stop reception
     */
    sim_data.dti_rx_state = SIM_DTI_RX_IDLE;
    dti_stop(sim_data.hDTI, 0, 0, 0);
    /*
     * inform SIM card
     */
    if (sim_data.event_data_avail EQ SIM_EVENT_ENABLE)
    {
      CCD_START;
      {
        MCAST(env_cmd, ENV_CMD);
        memset(env_cmd, 0, sizeof(T_ENV_CMD));

        /*
         * Event Download Command
         */
        env_cmd->v_evd_cmd = TRUE;
        /*
         * Event List
         */
        env_cmd->evd_cmd.v_ev_list        = TRUE;
        env_cmd->evd_cmd.ev_list.c_event  = 1;
        env_cmd->evd_cmd.ev_list.event[0] = EVENT_DATA_AVAIL;
        /*
         * Device Identities
         */
        env_cmd->evd_cmd.v_dev_ids        = TRUE;
        env_cmd->evd_cmd.dev_ids.src_dev  = DEV_SRC_ME;
        env_cmd->evd_cmd.dev_ids.dest_dev = DEV_DST_SIM;
        /*
         * Channel Satus
         */
        env_cmd->evd_cmd.v_chan_stat              = TRUE;
        env_cmd->evd_cmd.chan_stat.chan_id        = sim_data.bip_ch_id & 0x07;
        env_cmd->evd_cmd.chan_stat.chan_stat_inf1 = 0;
        env_cmd->evd_cmd.chan_stat.chan_stat_link = LINK_ESTABL;
        env_cmd->evd_cmd.chan_stat.chan_stat_inf2 = NO_FURTH_INFO;
        /*
         * Channel Data Length
         */
        env_cmd->evd_cmd.v_chan_dat_lth = TRUE;
        if (sim_data.received_data.list_len > 255)
        {
          /*
           * more than 255 bytes are available in RX buffer
           */
          env_cmd->evd_cmd.chan_dat_lth = 0xff;
        }
        else
        {
          env_cmd->evd_cmd.chan_dat_lth = (UBYTE)sim_data.received_data.list_len;
        }
        /*
         * encode message
         */
        MALLOC(temp_sdu, (USHORT)(sizeof(T_sdu) - 1 + MAX_STK_CMD));
        temp_sdu->l_buf = MAX_STK_CMD << 3;
        temp_sdu->o_buf = 0;

        ccdRet = ccd_codeMsg (CCDENT_SAT,
                              UPLINK,
                              (T_MSGBUF *) temp_sdu,
                              (UBYTE    *) _decodedMsg,
                              ENV_CMD);
      }
      CCD_END;

      if ( ccdRet NEQ ccdOK )
      {
        TRACE_EVENT_P1("Data Available: CCD Coding Error: %d",ccdRet );
      }
      else
      {
          FKT_Envelope (dummy, temp_sdu->buf, (USHORT)(temp_sdu->l_buf >> 3), 0);
        /*
         * data received is not triggered from a proactive_polling
         * function call, so adjust timer and start polling
         */
        stk_start_timer_and_poll();
      }
      MFREE(temp_sdu);
    }
  }
} /* stk_dti_data_received() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_tx_buffer_full     |
+--------------------------------------------------------------------+

  PURPOSE : Handle TX BUFFER FULL signal from DTILIB.

*/

GLOBAL void stk_dti_tx_buffer_full (void)
{
  TRACE_FUNCTION ("stk_dti_tx_buffer_full()");

  /*
   * set new DTI state
   */
  sim_data.dti_tx_state = SIM_DTI_TX_IDLE;
} /* stk_dti_tx_buffer_full() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_tx_buffer_ready    |
+--------------------------------------------------------------------+

  PURPOSE : Handle TX BUFFER READY signal from DTILIB.

*/

GLOBAL void stk_dti_tx_buffer_ready (void)
{
  TRACE_FUNCTION ("stk_dti_tx_buffer_ready()");

  /*
   * set new DTI state
   */
  sim_data.dti_tx_state = SIM_DTI_TX_READY;

  /*
   * send confirm primitive to MMI: out of here; actually as the response for 
   * SIM_BIP_CONFIG_REQ
   */
  if ((sim_data.sim_bip_config_req) AND (sim_data.dti_connection_state EQ SIM_DTI_CONNECTION_SETUP))
    {
      PALLOC(sim_bip_config_cnf, SIM_BIP_CONFIG_CNF);
      PSEND(hCommMMI, sim_bip_config_cnf);
      /* primitive no longer needed.. */
      /* and so avoid the second confirm in stk_udp_bind_cnf() */
      PFREE(sim_data.sim_bip_config_req);
      sim_data.sim_bip_config_req = NULL;
      /*
       * set the open state: the connection is now truly opened
       */
      sim_data.dti_connection_state = SIM_DTI_CONNECTION_OPEN;      
    } 
  /*
   * send data
   */
  if (sim_data.bip_tx_state EQ SIM_BIP_TX_SEND)
  {
    /*
     * set new BIP state
     */
#ifdef _SIMULATION_ 
    TRACE_EVENT("bip_tx_state = IDLE");
#endif 
    sim_data.bip_tx_state = SIM_BIP_TX_IDLE;
    /*
     * send DTI data primitive
     */
    stk_dti_send_data();
    /*
     * send Terminal Response
     */
    stk_bip_send_data_terminal_response(sim_data.bip_general_result,
                                        sim_data.bip_add_info_result);
    /*
     * buffer ready is not triggered from a proactive_polling function call
     * so adjust timer and start polling
     */
    stk_start_timer_and_poll();
    /*
     * (re)start release timer
     */
    if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
        (sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE) AND
        ((sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED) OR
        (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)))
    {
      sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
      TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_tx_buffer_ready()");
#endif
      TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
    }
  }
} /* stk_dti_tx_buffer_ready() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_bip_send_data      |
+--------------------------------------------------------------------+

  PURPOSE : Handle SEND DATA message from SIM card.

*/

GLOBAL UBYTE stk_dti_bip_send_data (T_sdu* message)
{
  UBYTE   ccdRet;
  UBYTE   func_ret;
  UBYTE   general_result;
  UBYTE   add_info_result;
  UBYTE   status;
  T_desc2* temp_desc1;
  T_desc2* temp_desc2;
  USHORT  temp_pos;
  USHORT  temp_len;

  TRACE_FUNCTION ("stk_dti_bip_send_data()");

  /*
   * initialize value;
   */
  ccdRet          = ccdOK;
  func_ret        = FALSE;
  general_result  = RSLT_PERF_SUCCESS;
  add_info_result = ADD_NO_CAUSE;
  status          = SIM_CCD_OK;

  CCD_START;
  /*
   * decode SIM Toolkit Command
   */
  stk_bip_decode_stk_command(message,
                             &sim_data.bip_tx_cmd_details,
                             &status,
                             &general_result,
                             &add_info_result);  
  /*
   * decode SEND DATA message
   */
  if (status EQ SIM_CCD_OK)
  {
    MCAST(send_data, SEND_DATA);
    memset(send_data, 0, sizeof(T_SEND_DATA));
    /*
     * decode SEND DATA message
     */
    ccdRet = ccd_decodeMsg (CCDENT_SAT,
                            DOWNLINK,
                            (T_MSGBUF *) &sim_data.bip_cmd_prms,
                            (UBYTE    *) _decodedMsg,
                            SEND_DATA);

    if (ccdRet NEQ ccdOK)
    {
      /*
       * handle errors in list
       */
      stk_handle_ccd_error(ccdRet, &status, &general_result, &add_info_result);      
    }
    if (send_data->v_chan_data EQ FALSE)
    {
      /*
       * no Channel Data element present
       * Error, required values are missing
       */
      general_result  = RSLT_ERR_REQ_VAL;
      add_info_result = ADD_NO_CAUSE;
      status          = SIM_CCD_RETURN;
    }
    if (status EQ SIM_CCD_OK)
    {
      if (sim_data.bip_suspend)
      {
        /*
         * channel suspended
         * ME currently unable to process command
         */
        general_result  = sim_data.bip_general_result;
        add_info_result = sim_data.bip_add_info_result;
        status          = SIM_CCD_RETURN;
      }
      else if ((SIM_CLASS_E_BUFFER_SIZE - sim_data.data_to_send.list_len) <
              send_data->chan_data.c_ch_dat_str)
      {
        /*
         * not enough space in tx buffer
         * BIP error
         */
        general_result  = RSLT_BEARIND_PERR;
        add_info_result = ADD_BIP_BUF_SIZ_NAVAIL;
        status          = SIM_CCD_RETURN;
      }
      else
      {
        /*
         * concatenate Channel data
         */
        sim_data.prev_data_to_send = sim_data.data_to_send;
        if (sim_data.data_to_send.first NEQ (ULONG)NULL)
        {
          /*
           * find last descriptor
           */
          temp_desc1 = (T_desc2*)sim_data.data_to_send.first;
          while(temp_desc1->next NEQ (ULONG)NULL)
          {
            temp_desc1 = (T_desc2*)temp_desc1->next;
          }
        }
        else if (send_data->chan_data.c_ch_dat_str)
        {
          /*
           * allocate a new descriptor
           */
          MALLOC(temp_desc1, (USHORT)(sizeof(T_desc2) - 1 +
                             SIM_BIP_TX_DESC_SIZE));
          temp_desc1->next = (ULONG)NULL;
          temp_desc1->offset= 0; /*###jk:OK*/
          temp_desc1->len  = 0;
          temp_desc1->size= 0; /*###jk:OK*/
          sim_data.data_to_send.first    = (ULONG)temp_desc1;
          sim_data.data_to_send.list_len = 0;
        }
        temp_pos = 0;
        while(temp_pos < send_data->chan_data.c_ch_dat_str)
        {
          if (temp_desc1->len >= SIM_BIP_TX_DESC_SIZE) /*lint !e644 temp_desc1 may not have been initialized*/
          {
            /*
             * allocate new desriptor
             */
            temp_desc2 = temp_desc1;
            MALLOC(temp_desc1, (USHORT)(sizeof(T_desc2) - 1 +
                               SIM_BIP_TX_DESC_SIZE));
            temp_desc1->next = (ULONG)NULL;
            temp_desc1->offset = 0; /*###jk:OK*/
            temp_desc1->len  = 0;
            temp_desc1->size = 0; /*###jk:OK*/
            temp_desc2->next = (ULONG)temp_desc1;
          }
          /*
           * calculate length
           */
          temp_len = send_data->chan_data.c_ch_dat_str - temp_pos;
          if (temp_len > (SIM_BIP_TX_DESC_SIZE - temp_desc1->len))
          {
            temp_len = SIM_BIP_TX_DESC_SIZE - temp_desc1->len;
            TRACE_EVENT("sdbsd_5: if (temp_desc1->len >= SIM_BIP_TX_DESC_SIZE)"); /*###jk:tbd*/      
            TRACE_EVENT_P1("sdbsd_5: temp_len = %d", temp_len); /*###jk:tbd*/      
          }
          /*
           * copy data
           */
          memcpy(&temp_desc1->buffer[temp_desc1->len],
                 &send_data->chan_data.ch_dat_str[temp_pos],
                 temp_len);
          temp_pos                      += temp_len;
          temp_desc1->len               += temp_len;
          temp_desc1->size              += temp_len; /*###jk:OK?*/
          sim_data.data_to_send.list_len+= temp_len;
        }
      }
      /*
       * process SEND DATA message
       */
      if (status EQ SIM_CCD_OK)
      {
        /*
         * if alpha identifier or icon identifier is present then
         * forward message to MMI
         */
        if ((send_data->v_alpha_id) OR
           (send_data->v_icon))
        {
          func_ret = TRUE;
        }
        /*
         * check for immediate/store bit
         */
        if (sim_data.bip_tx_cmd_details.cmd_qlf & SIM_QLF_SEND_DATA_1)
        {
          /*
           * send data immediately
           */
          if (sim_data.bip_state EQ SIM_BIP_CONNECTED)
          {
            /*
             * DTI connected
             */
            if (sim_data.dti_tx_state EQ SIM_DTI_TX_READY)
            {
              /*
               * send DTI data primitive
               */
#ifdef _SIMULATION_ 
              TRACE_EVENT("bip_tx_state = IDLE");
#endif 
              sim_data.bip_tx_state = SIM_BIP_TX_IDLE;
              status                = SIM_CCD_RETURN;
              stk_dti_send_data();
            }
            else
            {
              TRACE_EVENT("stbsd_8: else (sim_data.dti_tx_state EQ SIM_DTI_TX_READY)"); /*###jk:tbd*/     
              /*
               * start release timer
               */
#ifdef _SIMULATION_ 
              TRACE_EVENT("bip_tx_state = SEND");
#endif 
              sim_data.bip_tx_state = SIM_BIP_TX_SEND;
              status                = SIM_CCD_DISCARD;
              if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
                  (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
                 )
              {
                sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_  
                TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif  
                TIMER_STOP (sim_handle, SIM_BIP_TIMER);
              }
            }
            /*
             * (re)start release timer
             */
            if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
                (sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE) AND
                (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE) AND
                ((sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED) OR
                 (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)))
            {
              sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_  
              TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_bip_send_data(..)");
#endif 
              TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
            }
          }
          else
          {
            TRACE_EVENT("stbsd_7: else (sim_data.bip_state EQ SIM_BIP_CONNECTED)"); /*###jk:tbd*/
            /*
             * wait for DTI connection
             */
#ifdef _SIMULATION_ 
            TRACE_EVENT("bip_tx_state = SEND");
#endif  
            sim_data.bip_tx_state = SIM_BIP_TX_SEND;
            status                = SIM_CCD_DISCARD;
            if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
                (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
               )
            {
              sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_ 
              TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif  
              TIMER_STOP (sim_handle, SIM_BIP_TIMER);
            }
            if (sim_data.dti_connection_state EQ SIM_DTI_CONNECTION_CLOSED)
            {
              /*
               * on demand link establishment
               * so forward message to MMI
               */
              func_ret              = TRUE;
            }
          }
        }
        else
        {
          /*
           * store data
           */
          TRACE_EVENT("bip_tx_state = STORE");
          sim_data.bip_tx_state = SIM_BIP_TX_STORE;
          status                = SIM_CCD_RETURN;
          /*
           * stop release timer if used 
           */
          if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
              (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
             )
          {
            sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
            TRACE_EVENT("SIM_BIP_TIMER: stopped");
            TIMER_STOP (sim_handle, SIM_BIP_TIMER);
          }
        }
      }
    }
  } /* if (status EQ SIM_CCD_OK) */

  CCD_END;

  /*
   * send TERMINAL RESPONSE message
   */
  if (status EQ SIM_CCD_RETURN)
  {
    stk_bip_send_data_terminal_response(general_result, add_info_result);
  }
  else
  {
    /*
     * store result codes
     */
    sim_data.bip_general_result  = general_result;
    sim_data.bip_add_info_result = add_info_result;
  }
  /*
   * send return value
   */
  return func_ret;
} /* stk_dti_bip_send_data() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_dti_bip_receive_data   |
+--------------------------------------------------------------------+

  PURPOSE : Handle RECEIVE DATA message from SIM card.

*/

LOCAL UBYTE stk_dti_bip_receive_data (T_sdu* message, UBYTE result_code)
{
  UBYTE   ccdRet;
  UBYTE   func_ret;
  UBYTE   general_result;
  UBYTE   add_info_result;
  UBYTE   status;
  UBYTE   chan_dat_lth;
  T_desc2* temp_desc1;
  T_desc2* temp_desc2;
  T_sdu*  temp_sdu;
  USHORT  temp_pos;
  USHORT  temp_len;

  /*
   * initialize value;
   */
  ccdRet          = ccdOK;
  func_ret        = FALSE;
  general_result  = RSLT_PERF_SUCCESS;
  add_info_result = ADD_NO_CAUSE;
  status          = SIM_CCD_OK;
  chan_dat_lth    = 0; /* jk: initialized because of LINT error */

  CCD_START;
  /*
   * decode SIM Toolkit Command
   */
  stk_bip_decode_stk_command(message,
                             &sim_data.bip_rx_cmd_details,
                             &status,
                             &general_result,
                             &add_info_result);

  /*
   * decode RECEIVE DATA message
   */
  if (status EQ SIM_CCD_OK)
  {
    MCAST(receive_data, RECEIVE_DATA);
    memset(receive_data, 0, sizeof(T_RECEIVE_DATA));

    ccdRet = ccd_decodeMsg (CCDENT_SAT,
                            DOWNLINK,
                            (T_MSGBUF *) &sim_data.bip_cmd_prms,
                            (UBYTE    *) _decodedMsg,
                            RECEIVE_DATA);

#ifdef _SIMULATION_
  TRACE_EVENT_P1("ccdRet@dti_bip_receive_data: %d", ccdRet);
#endif

    if (ccdRet NEQ ccdOK)
    {
      /*
       * handle errors in list
       */
      stk_handle_ccd_error(ccdRet, &status, &general_result, &add_info_result);
    }
    if (receive_data->v_chan_dat_lth EQ FALSE)
    {
      /*
       * no Channel Data Length element present
       * Error, required values are missing
       */
      general_result  = RSLT_ERR_REQ_VAL;
      add_info_result = ADD_NO_CAUSE;
      status          = SIM_CCD_RETURN;
    }
    /*
     * process RECEIVE DATA message
     */
    if (status EQ SIM_CCD_OK)
    {
      /*
       * if alpha identifier or icon identifier is present then
       * forward message to MMI
       */
      if ((receive_data->v_alpha_id) OR
         (receive_data->v_icon))
      {
        func_ret = TRUE;
      }
      if (sim_data.received_data.list_len < receive_data->chan_dat_lth)
      {
        /*
         * can not fill the complete buffer
         */
        general_result  = RSLT_PERF_MISS_INFO;
        add_info_result = ADD_NO_CAUSE;
      }
      status = SIM_CCD_RETURN;
      /*
       * store Channel Data Length
       */
      chan_dat_lth = receive_data->chan_dat_lth;
    }
  }
  CCD_END;
  /*
   * send TERMINAL RESPONSE message
   */
  if (status EQ SIM_CCD_RETURN)
  {
    CCD_START;
    {
      MCAST(term_resp, TERM_RESP);
      memset(term_resp, 0, sizeof(T_TERM_RESP));

      /*
       * set Command details
       */
      term_resp->v_cmd_details = TRUE;
      term_resp->cmd_details   = sim_data.bip_rx_cmd_details;
      /*
       * set set Device identities
       */
      term_resp->v_dev_ids        = TRUE;
      term_resp->dev_ids.src_dev  = DEV_SRC_ME;
      term_resp->dev_ids.dest_dev = DEV_DST_SIM;
      /*
       * set Result
       */
      term_resp->v_res   = TRUE;
      term_resp->res.gen = general_result;
      switch (general_result)
      {
        case RSLT_ME_UNAB_PROC:
        case RSLT_NTW_UNAB_PROC:
        case RSLT_LABRWS_GENERIC:
        case RSLT_SS_ERR:
        case RSLT_SMS_ERR:
        case RSLT_USSD_ERR:
        case RSLT_CC_SIM_PRM:
          /*
           * one byte for additional information
           */
          term_resp->res.v_add        = TRUE;
          term_resp->res.add.l_add    = 1 << 3;
          term_resp->res.add.o_add    = 0;
          term_resp->res.add.b_add[0] = add_info_result;
          break;

        case RSLT_BEARIND_PERR:
          /*
           * one byte for additional information
           */
          term_resp->res.v_add        = TRUE;
          term_resp->res.add.l_add    = 1 << 3;
          term_resp->res.add.o_add    = 0;
          term_resp->res.add.b_add[0] = add_info_result-1;
          break;

        default:
          if (result_code)
            term_resp->res.gen = RSLT_PERF_PART_CMPR;
          break;
      }
      /*
       * set Channel Data
       */
      term_resp->v_chan_data = TRUE;
      switch (general_result)
      {
      case RSLT_PERF_SUCCESS:
      case RSLT_PERF_PART_CMPR:
      case RSLT_PERF_MISS_INFO:
      case RSLT_PERF_MDFY_SIM:
      case RSLT_PERF_MDFIED:
        /*
         * calculate Channel Data String length
         */
        term_resp->chan_data.c_ch_dat_str = SIM_TERM_RESP_MAX_CHANNEL_DATA;
        if (sim_data.received_data.list_len < term_resp->chan_data.c_ch_dat_str)
        {
          term_resp->chan_data.c_ch_dat_str = (UBYTE)sim_data.received_data.list_len;
        }
        if (chan_dat_lth < term_resp->chan_data.c_ch_dat_str)
        {
          term_resp->chan_data.c_ch_dat_str = chan_dat_lth;
        }
        /*
         * copy data
         */
        temp_desc1 = (T_desc2*)sim_data.received_data.first;
        temp_pos = 0;
        while (temp_pos < term_resp->chan_data.c_ch_dat_str)
        {
          /*
           * calculate length
           */
          temp_len = term_resp->chan_data.c_ch_dat_str - temp_pos;
          if (temp_len > (temp_desc1->len - sim_data.received_data_pos))
          {
            temp_len = temp_desc1->len - sim_data.received_data_pos;
          }
          /*
           * copy data
           */
          memcpy(&term_resp->chan_data.ch_dat_str[temp_pos],
                 &temp_desc1->buffer[sim_data.received_data_pos],
                 temp_len);
          /*
           * updata length and position values and descriptors
           */
          temp_pos                       += temp_len;
          sim_data.received_data_pos     += temp_len;
          sim_data.received_data.list_len-= temp_len;
          if (sim_data.received_data_pos >= temp_desc1->len)
          {
            temp_desc2                   = temp_desc1;
            temp_desc1                   = (T_desc2*)temp_desc1->next;
            sim_data.received_data_pos   = 0;
            sim_data.received_data.first = (ULONG)temp_desc1;
            MFREE(temp_desc2);
          }
        }
        break;

      case RSLT_BEARIND_PERR:
      case RSLT_UNKN_DATA:
        term_resp->v_chan_data = FALSE;
        term_resp->chan_data.c_ch_dat_str = 0;
        break;

      default:
        /*
         * if an error is occured then do not provide data
         */
        term_resp->chan_data.c_ch_dat_str = 0;
        break;
      }

      switch (general_result)
      {
      case RSLT_BEARIND_PERR:
      case RSLT_UNKN_DATA:
        break;

      default:
        /*
         * set Channel Data Length
         */
        term_resp->v_chan_dat_lth = TRUE;
        if (sim_data.received_data.list_len > 255)
        {
          /*
           * more than 255 bytes are available in RX buffer
           */
          term_resp->chan_dat_lth = 0xff;
        }
        else
        {
          term_resp->chan_dat_lth = (UBYTE)sim_data.received_data.list_len;
        }
        break;
      }
      /*
       * send Terminal Response
       */
      MALLOC(temp_sdu, (USHORT)(sizeof(T_sdu) - 1 + MAX_STK_CMD));
      temp_sdu->l_buf = MAX_STK_CMD << 3;
      temp_sdu->o_buf = 0;

      ccdRet = ccd_codeMsg (CCDENT_SAT,
                            UPLINK,
                            (T_MSGBUF *) temp_sdu,
                            (UBYTE    *) _decodedMsg,
                            TERM_RESP);
    }
    CCD_END;

    if ( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1("CCD Coding Error: %d",ccdRet );
    }
    else
    {
      FKT_TerminalResponse (temp_sdu->buf, (USHORT)(temp_sdu->l_buf >> 3));
    }
    MFREE(temp_sdu);
  }

  if ((sim_data.bip_rx_state EQ SIM_BIP_RX_DATA) AND
     (sim_data.received_data.list_len EQ 0))
  {
    /*
     * set new BIP state
     */
#ifdef _SIMULATION_
    TRACE_EVENT("bip_rx_state = IDLE");
#endif
    sim_data.bip_rx_state = SIM_BIP_RX_IDLE;
    /*
     * start DTI reception
     */
    if ((sim_data.bip_state EQ SIM_BIP_CONNECTED) AND
       (sim_data.dti_rx_state EQ SIM_DTI_RX_IDLE))
    {
      sim_data.dti_rx_state = SIM_DTI_RX_READY;
      dti_start(sim_data.hDTI, 0, 0, 0);
    }
    /*
     * start release timer
     */
    if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
        (sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED) AND
        (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE)
       )
    {
         /*
          * no data trafic on the BIP channel,
          * so use the timer
          */
          sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
          TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_bip_receive_data(..)");
#endif
          TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
    }
  }
  /*
   * send return value
   */
  return func_ret;
} /* stk_dti_bip_receive_data() */

#ifdef FF_SAT_E
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_udp_bind_cnf           |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive UDP_BIND_CNF.

*/

GLOBAL void stk_udp_bind_cnf (T_UDP_BIND_CNF* udp_bind_cnf)
{
  TRACE_FUNCTION ("stk_udp_bind_cnf()");

  if (sim_data.dti_connection_state EQ SIM_DTI_CONNECTION_BIND)
  {
    if (udp_bind_cnf->err EQ UDP_BIND_NOERROR)
    {
      /*
       * store source port
       */
      sim_data.udp_parameters.src_port[0] =
        ((udp_bind_cnf->port >> 8) & 0x00ff);
      sim_data.udp_parameters.src_port[1] =
        ((udp_bind_cnf->port)      & 0x00ff);
      /*
       * make sending of the confirmation primitive (SIM_BIP_CONFIG_CNF)
       * out of "stk_dti_buffer_ready()" possible
       */
      sim_data.dti_connection_state = SIM_DTI_CONNECTION_SETUP;

      /*###jk:OK? moved & changed from "stk_dti_connection_open()" */
      if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
          (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
         )
      {
        sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_ 
        TRACE_EVENT("stk_udp_bind_cnf(): SIM_BIP_TIMER: stopped");
#endif  
        TIMER_STOP (sim_handle, SIM_BIP_TIMER);
      }
      /*
       * update timer (if used) and DTI states
       */
      if ((sim_data.bip_state EQ SIM_BIP_CONNECTED) AND
         (sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE))
      {
        /*
         * start reception
         */
        sim_data.dti_rx_state = SIM_DTI_RX_READY;
        dti_start(sim_data.hDTI, 0, 0, 0);
        /*
         * start timer if used
         */
        if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
            (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE) AND
            (sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED)
           )
        {
          /*
           * no data trafic on the BIP channel,
           * so use the timer
           */
          sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
          TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_connection_opened()");
#endif
          TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
        }
      } /*###jk:OK?*/
    } 
    else
    {
      /*
       * can not open UDP port 
       * so disconnect BIP channel from DTI
       */
      stk_close_dti_connection(TRUE);
      /*
       * send confirm primitive
       */
      stk_dti_inform_mmi(SIM_DTI_DISCONNECT, (UBYTE) SIM_BIP_UNKNOWN);
    }
  }
  /*
   * free primitive
   */
  PFREE(udp_bind_cnf);
} /* stk_udp_bind_cnf() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_udp_closeport_cnf      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive UDP_CLOSEPORT_CNF.

*/

GLOBAL void stk_udp_closeport_cnf (
                    T_UDP_CLOSEPORT_CNF* udp_closeport_cnf)
{
  TRACE_FUNCTION ("stk_udp_closeport_cnf()");

  /*
   * free primitive
   */
  PFREE(udp_closeport_cnf);
} /* stk_udp_closeport_cnf() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_udp_error_ind          |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive UDP_ERROR_IND.

*/

GLOBAL void stk_udp_error_ind (T_UDP_ERROR_IND* udp_error_ind)
{
  TRACE_FUNCTION ("stk_udp_error_ind()");

  /*
   * free primitive
   */
  PFREE(udp_error_ind);
  /*
   * generate error response
   */
  {
    PALLOC(udp_error_res, UDP_ERROR_RES);
    PSEND(hCommUDP, udp_error_res);
  }
} /* stk_udp_error_ind() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_udp_shutdown_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive UDP_SHUTDOWN_IND.

*/

GLOBAL void stk_udp_shutdown_ind (T_UDP_SHUTDOWN_IND* udp_shutdown_ind)
{
  TRACE_FUNCTION ("stk_udp_shutdown_ind()");

  /*
   * send confirm primitive
   */
  if (hCommUDP < VSI_OK)
  {
    hCommUDP = vsi_c_open (VSI_CALLER UDP_NAME);
  }
  if (hCommUDP >= VSI_OK)
  {
    PALLOC(udp_shutdown_res, UDP_SHUTDOWN_RES);
    PSEND(hCommUDP, udp_shutdown_res);
    /*
     * release VSI channel
     */
    vsi_c_close (VSI_CALLER hCommUDP);
    hCommUDP = VSI_ERROR;
  }
  /*
   * close DTI connection
   */
  if (sim_data.con_type EQ SIM_CON_TYPE_UDP)
  {
    switch(sim_data.dti_connection_state)
    {
      case SIM_DTI_CONNECTION_OPEN:
      case SIM_DTI_CONNECTION_SETUP:
        stk_close_dti_connection(TRUE);
        break;

      default:
        stk_close_dti_connection(FALSE);
        break;
    }
    /*
     * inform ACI about disconnection
     */
    stk_dti_inform_mmi(SIM_DTI_DISCONNECT, (UBYTE) SIM_BIP_UNKNOWN);
  }
} /* stk_udp_shutdown_ind() */
#endif /* FF_SAT_E */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_dti_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_DTI_REQ.

*/

GLOBAL void stk_sim_dti_req (T_SIM_DTI_REQ* sim_dti_req)
{
  UBYTE  dti_conn;

  TRACE_FUNCTION ("stk_sim_dti_req()");

#ifdef _SIMULATION_
  /*
   * set entity_name parameter
   */
switch(sim_dti_req->entity_name)
  {
    case 1:
      sim_dti_req->entity_name = (ULONG)("UDP");
      break;

    case 2:
      sim_dti_req->entity_name = (ULONG)("SND");
      break;

    case 3:
      sim_dti_req->entity_name = (ULONG)("L2R");
      break;

    default:
      sim_dti_req->entity_name = (ULONG)(NULL);
      break;
  }
#endif /* _SIMULATION_ */
  /*
   * store the received primitive
   */
  if (sim_data.sim_dti_req)
  {
    /*
     * free previous primitive before store the new one
     */
    PFREE(sim_data.sim_dti_req);
  }
  sim_data.sim_dti_req = sim_dti_req;

  /*
   * store requested operations
   */
  dti_conn = sim_dti_req->dti_conn;

  switch (dti_conn)
  {
    case SIM_DTI_DISCONNECT: /* close DTI connection */
      {
        switch(sim_data.dti_connection_state)
          {
            case SIM_DTI_CONNECTION_OPEN:
            case SIM_DTI_CONNECTION_SETUP:
              stk_close_dti_connection(TRUE);
              break;
            default:
              stk_close_dti_connection(FALSE);
              break;
          }
        break;
      }
    case SIM_DTI_CONNECT: /* open DTI connection */
      {
        /*
         * if a new DTI connection is requested close the old one before
         */
        switch(sim_data.dti_connection_state)
          {
            case SIM_DTI_CONNECTION_OPEN:
            case SIM_DTI_CONNECTION_SETUP:
              stk_close_dti_connection(TRUE);
              break;
            default:
              stk_close_dti_connection(FALSE);
          }
        /*
         * store relevant data
         */
        sim_data.link_id  = sim_dti_req->link_id;
        sim_data.dti_connection_state = SIM_DTI_CONNECTION_SETUP;
        dti_open(sim_data.hDTI,                 /* DTI handle */
                 0,                             /* instance */
                 0,                             /* interface */
                 0,                             /* channel */
                 0,                             /* queue size */
                 sim_dti_req->dti_direction,    /* direction */
                 FLOW_CNTRL_ENABLED,            /* comm_type */
                 DTI_VERSION_10,                /* version */
                 (UBYTE*)sim_dti_req->entity_name, /* entity name */
                 sim_dti_req->link_id);         /* link identifier */
      }
  }
  /*
   * send confirm primitive
   */
  if (sim_data.sim_dti_req)
  {
    TRACE_EVENT_P1("if(sim_data.sim_dti_req): ~ ->dti_conn=0x%02X", sim_dti_req->dti_conn);

    stk_dti_inform_mmi(sim_dti_req->dti_conn, (UBYTE)SIM_BIP_UNKNOWN);
  }
} /* stk_sim_dti_req() */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_bip_req            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_BIP_REQ.

*/

GLOBAL void stk_sim_bip_req (T_SIM_BIP_REQ* sim_bip_req)
{
  UBYTE   bip_conn;
  USHORT  temp_len;
  T_desc2* temp_desc1;
  T_desc2* temp_desc2;

  TRACE_FUNCTION ("stk_sim_bip_req()");

  /*
   * store the received primitive
   */
  if (sim_data.sim_bip_req)
  {
    /*
     * free previous primitive before store the new one
     */
    PFREE(sim_data.sim_bip_req);
  }
  sim_data.sim_bip_req = sim_bip_req;
  /*
   * store requested operations
   */
  bip_conn = sim_bip_req->bip_conn;

  /*
   * resume BIP channel
   */
  if (bip_conn & SIM_BIP_CHANNEL_RESUMED)
  {
    /*
     * set new suspension state
     */
    sim_data.bip_suspend = FALSE;
    /*
     * restart timer if used
     */
    if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
        (sim_data.bip_timer_state EQ SIM_BIP_TIMER_SUSPENDED)
       )
    {
      sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
      if ((sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE) AND
          (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE)
         )
      {
            /*
             * no data trafic on the BIP channel,
             * so use the timer
             */
            sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
            TRACE_EVENT("SIM_BIP_TIMER: start in stk_sim_bip_req(..)");
#endif
            TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
      }
    }
  }
  /*
   * close BIP channel
   */
  if (bip_conn & SIM_BIP_CLOSE_CHANNEL)
  {
    stk_close_bip_channel(sim_bip_req->general_result,
                          sim_bip_req->add_info_result);
  }
  /*
   * open BIP channel
   */
  if ((bip_conn & SIM_BIP_OPEN_CHANNEL) AND
     (sim_data.bip_state EQ SIM_BIP_CLOSED))
  {
    /*
     * set new BIP state and
     * store BIP channel identifier
     */
    sim_data.bip_state = SIM_BIP_OPEN;
    sim_data.bip_ch_id = sim_bip_req->bip_ch_id;
    /*
     * store relevant data
     */
    sim_data.bip_release_time =
          (T_TIME)sim_bip_req->release_time * 100; /* convert to msec. */
    if (sim_bip_req->general_result NEQ RSLT_PERF_SUCCESS)
    {
      sim_data.bip_general_result  = sim_bip_req->general_result;
      sim_data.bip_add_info_result = sim_bip_req->add_info_result;
    }
  }
  /*
   * suspend BIP channel
   */
  if (bip_conn & SIM_BIP_CHANNEL_SUSPENDED)
  {
    /*
     * set new suspension state
     */
    sim_data.bip_suspend = TRUE;
    /*
     * stop timer if timer is used
     */
    if (sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED)
    {
      if (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
      {
        sim_data.bip_timer_state = SIM_BIP_TIMER_SUSPENDED;
#ifdef _SIMULATION_
        TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
        TIMER_STOP (sim_handle, SIM_BIP_TIMER);
      }
      else if (sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED)
      {
        sim_data.bip_timer_state = SIM_BIP_TIMER_SUSPENDED;
      }
    }
    /*
     * store result codes
     */
    sim_data.bip_general_result  = sim_bip_req->general_result;
    sim_data.bip_add_info_result = sim_bip_req->add_info_result;
    /*
     * inform SIM card
     */
    if (sim_data.bip_tx_state EQ SIM_BIP_TX_SEND)
    {
      /*
       * free data of last SEND DATA message
       */
      temp_desc1            = (T_desc2*)sim_data.data_to_send.first;
      sim_data.data_to_send = sim_data.prev_data_to_send;
      if (sim_data.data_to_send.first EQ (ULONG)NULL)
      {
        while(temp_desc1)
        {
          temp_desc2 = temp_desc1;
          temp_desc1 = (T_desc2*)temp_desc1->next;
          MFREE(temp_desc2);
        }
      }
      else
      {
        temp_len = 0;
        while((temp_desc1) AND
              (temp_len + temp_desc1->len) < sim_data.data_to_send.list_len)
        {
          temp_len  += temp_desc1->len;
          temp_desc1 = (T_desc2*)temp_desc1->next;
        }
        if (temp_desc1)
        {
          temp_desc1->len  = sim_data.data_to_send.list_len - temp_len;
          temp_desc2       = (T_desc2*)temp_desc1->next;
          temp_desc1->next = (ULONG)NULL;
          temp_desc1       = temp_desc2;
          while(temp_desc1)
          {
            temp_desc2 = temp_desc1;
            temp_desc1 = (T_desc2*)temp_desc1->next;
            MFREE(temp_desc2);
          }
        }
      }
      /*
       * set new BIP TX state
       */
      if (sim_data.data_to_send.first EQ (ULONG)NULL)
      {
#ifdef _SIMULATION_
        TRACE_EVENT("bip_tx_state = IDLE");
#endif
        sim_data.bip_tx_state = SIM_BIP_TX_IDLE;
      }
      else
      {
#ifdef _SIMULATION_
        TRACE_EVENT("bip_tx_state = STORE");
#endif
        sim_data.bip_tx_state = SIM_BIP_TX_STORE;
        if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
            (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
           )
        {
          sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_
          TRACE_EVENT("SIM_BIP_TIMER: stopped");
#endif
          TIMER_STOP (sim_handle, SIM_BIP_TIMER);
        }
      }
      /*
       * send Terminal Response
       */
      stk_bip_send_data_terminal_response(sim_bip_req->general_result,
                                          sim_bip_req->add_info_result);
      /*
       * suspension is not triggered from a proactive_polling function call
       * so adjust timer and start polling
       */
      stk_start_timer_and_poll();
    }
  }
  /*
   * send confirm primitive
   */
  if (sim_data.sim_bip_req)
  {
    stk_dti_inform_mmi((UBYTE) SIM_DTI_UNKNOWN, sim_bip_req->bip_conn);
  }
} /* stk_sim_bip_req() */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_bip_config_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_BIP_CONFIG_REQ.

*/

GLOBAL void stk_sim_bip_config_req(T_SIM_BIP_CONFIG_REQ * sim_bip_config_req)
{
  TRACE_FUNCTION ("stk_sim_bip_config_req()");
  /*
   * store the received primitive
   */
  if (sim_data.sim_bip_config_req)
  {
    /*
     * free previous primitive before store the new one
     */
    PFREE(sim_data.sim_bip_config_req);
  }
  sim_data.sim_bip_config_req = sim_bip_config_req;
  /*
   * store relevant data
   */
  sim_data.con_type                   = sim_bip_config_req->con_type;

#if 0
  sim_data.udp_parameters.src_ip[0]   =
      (UBYTE)((sim_bip_config_req->local_ip >> 24) & 0x000000ff);
  sim_data.udp_parameters.src_ip[1]   =
      (UBYTE)((sim_bip_config_req->local_ip >> 16) & 0x000000ff);
  sim_data.udp_parameters.src_ip[2]   =
      (UBYTE)((sim_bip_config_req->local_ip >>  8) & 0x000000ff);
  sim_data.udp_parameters.src_ip[3]   =
      (UBYTE)((sim_bip_config_req->local_ip)       & 0x000000ff);
#else
sim_data.udp_parameters.src_ip[3]   =
    (UBYTE)((sim_bip_config_req->local_ip >> 24) & 0x000000ff);
sim_data.udp_parameters.src_ip[2]   =
    (UBYTE)((sim_bip_config_req->local_ip >> 16) & 0x000000ff);
sim_data.udp_parameters.src_ip[1]   =
    (UBYTE)((sim_bip_config_req->local_ip >>  8) & 0x000000ff);
sim_data.udp_parameters.src_ip[0]   =
    (UBYTE)((sim_bip_config_req->local_ip)       & 0x000000ff);
#endif

  sim_data.udp_parameters.des_ip[0]   =
      (UBYTE)((sim_bip_config_req->destination_ip >> 24) & 0x000000ff);
  sim_data.udp_parameters.des_ip[1]   =
      (UBYTE)((sim_bip_config_req->destination_ip >> 16) & 0x000000ff);
  sim_data.udp_parameters.des_ip[2]   =
      (UBYTE)((sim_bip_config_req->destination_ip >>  8) & 0x000000ff);
  sim_data.udp_parameters.des_ip[3]   =
      (UBYTE)((sim_bip_config_req->destination_ip)       & 0x000000ff);

#if 0
  sim_data.udp_parameters.des_port[0] =
      (UBYTE)((sim_bip_config_req->destination_port >> 8) & 0x00ff);
  sim_data.udp_parameters.des_port[1] =
      (UBYTE)((sim_bip_config_req->destination_port)      & 0x00ff);
#else
  sim_data.udp_parameters.des_port[1] =
      (UBYTE)((sim_bip_config_req->destination_port >> 8) & 0x00ff);
  sim_data.udp_parameters.des_port[0] =
      (UBYTE)((sim_bip_config_req->destination_port)      & 0x00ff);
#endif
   
    /*
     * UDP connection
     */
    if (sim_data.con_type EQ SIM_CON_TYPE_UDP)
    {
      /*
       * open VSI channel to UDP
       */
      if (hCommUDP < VSI_OK)            
      {
        TRACE_EVENT("if(hCommUDP < VSI_OK)");
        if ((hCommUDP = vsi_c_open (VSI_CALLER UDP_NAME)) < VSI_OK)
        {
          TRACE_EVENT("if ((hCommUDP = vsi_c_open (VSI_CALLER UDP_NAME)) < VSI_OK)");
          /*
           * can not open VSI channel
           * so act as if DTI close was requested
           */
          sim_data.sim_dti_req->dti_conn = SIM_DTI_DISCONNECT;
          TRACE_EVENT_P1(": sim_data.sim_dti_req->dti_conn=0x%02X", sim_data.sim_dti_req->dti_conn);
        }
      }
      /*
       * send UDP_BIND_REQ
       */
      if (hCommUDP >= VSI_OK)
      {
        PALLOC(udp_bind_req, UDP_BIND_REQ);
        sim_data.dti_connection_state = SIM_DTI_CONNECTION_BIND;
        udp_bind_req->port            = UDP_AUTOASSIGN_PORT;
        PSEND(hCommUDP, udp_bind_req);
        /*
         * send confirm primitive:
         * will be sent out of "stk_dti_buffer_ready()" function in case of success,
         * in case of udp_bind_cnf failure only indication primitive will be sent
         * out of "stk_udp_bind_cnf" to MMI instead.
         */
      }
    }
    /*
     * bearer level connection
     */
    else
    {
      /*###jk:OK? moved & changed from "stk_dti_connection_open()" */
      if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
          (sim_data.bip_timer_state EQ SIM_BIP_TIMER_START)
         )
      {
        sim_data.bip_timer_state = SIM_BIP_TIMER_STOPPED;
#ifdef _SIMULATION_ 
        TRACE_EVENT("stk_udp_bind_cnf(): SIM_BIP_TIMER: stopped");
#endif  
        TIMER_STOP (sim_handle, SIM_BIP_TIMER);
      }
      /*
       * update timer (if used) and DTI states
       */
      if ((sim_data.bip_state EQ SIM_BIP_CONNECTED) AND
         (sim_data.bip_rx_state EQ SIM_BIP_RX_IDLE))
      {
        /*
         * start reception
         */
        sim_data.dti_rx_state = SIM_DTI_RX_READY;
        dti_start(sim_data.hDTI, 0, 0, 0);
        /*
         * start timer if used
         */
        if ((sim_data.bip_timer_state NEQ SIM_BIP_TIMER_NOT_USED) AND
            (sim_data.bip_tx_state EQ SIM_BIP_TX_IDLE) AND
            (sim_data.bip_timer_state EQ SIM_BIP_TIMER_STOPPED)
           )
        {
          /*
           * no data trafic on the BIP channel,
           * so use the timer
           */
          sim_data.bip_timer_state = SIM_BIP_TIMER_START;
#ifdef _SIMULATION_
          TRACE_EVENT("SIM_BIP_TIMER: start in stk_dti_connection_opened()");
#endif
          TIMER_START (sim_handle, SIM_BIP_TIMER, sim_data.bip_release_time);
        }
      } /*###jk:OK?*/ 

      /*
       * make sending of the confirmation primitive (SIM_BIP_CONFIG_CNF)
       * out of "stk_dti_buffer_ready()" possible
       */      
       sim_data.dti_connection_state = SIM_DTI_CONNECTION_SETUP;

      /*
       * send confirm primitive:
       * will be sent out of "stk_dti_buffer_ready()" function in order to
       * prevent sending of the data to connected neighbour DTI-entity before
       * this entity is ready to receive data.
       */      
#if 0 /*###jk:OK? -> */
      /*
       * send confirm primitive
       */
      if (sim_data.sim_bip_config_req)
      {
        PALLOC(sim_bip_config_cnf, SIM_BIP_CONFIG_CNF);
        PSEND(hCommMMI, sim_bip_config_cnf);
      }
#endif
    }
} /* stk_bip_config_req() */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_eventlist_req      |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_EVENTLIST_REQ.

*/

GLOBAL void stk_sim_eventlist_req (
                    T_SIM_EVENTLIST_REQ* sim_eventlist_req)
{
  TRACE_FUNCTION ("stk_sim_eventlist_req()");

  /*
   * store new state of Data available event
   */
  sim_data.event_data_avail = sim_eventlist_req->event_data_avail;
  /*
   * free primitive
   */
  PFREE(sim_eventlist_req);
  /*
   * send confirm primitive
   */
  {
    PALLOC(sim_eventlist_cnf, SIM_EVENTLIST_CNF);
    sim_eventlist_cnf->event_data_avail = sim_data.event_data_avail;
    PSEND(hCommMMI, sim_eventlist_cnf);
  }
} /* stk_sim_eventlist_req() */
#endif /* FF_SAT_E */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_toolkit_res        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_TOOLKIT_RES. MMI sends a
            Terminal Response Message to SIM toolkit.

*/

GLOBAL void stk_sim_toolkit_res (T_SIM_TOOLKIT_RES * sim_toolkit_res)
{
  TRACE_FUNCTION ("stk_sim_toolkit_res()");

  if (sim_data.ext_sat_cmd)
  {
    FKT_TerminalResponse (sim_toolkit_res->stk_cmd.cmd,
                          (USHORT)(sim_toolkit_res->stk_cmd.l_cmd>>3));
    PFREE (sim_toolkit_res);

    sim_data.ext_sat_cmd = FALSE;
    stk_start_timer_and_poll();
  }
  else
  {
    TRACE_EVENT("no outstanding TR");
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_toolkit_req        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_TOOLKIT_REQ. MMI sends an
            Envelope Message to SIM toolkit.

*/

GLOBAL void stk_sim_toolkit_req (T_SIM_TOOLKIT_REQ * sim_toolkit_req)
{
  PALLOC (sim_toolkit_cnf, SIM_TOOLKIT_CNF);

  sim_toolkit_cnf->stk_cmd.o_cmd = 0;
  sim_toolkit_cnf->req_id = sim_toolkit_req->req_id;

  TRACE_FUNCTION ("stk_sim_toolkit_req()");

  switch (sim_toolkit_req->source)  // check valid source
  {
  case SRC_MMI:
  case SRC_SMS:
    break;
  default:
    TRACE_EVENT ("SIM_TOOLKIT_REQ: invalid source");
    PFREE (sim_toolkit_cnf);
    PFREE (sim_toolkit_req);
    return;
  }
  /*
   * Forward envelope command to SIM toolkit
   */
  sim_toolkit_cnf->cause = FKT_Envelope (sim_toolkit_cnf->stk_cmd.cmd,
                                         sim_toolkit_req->stk_cmd.cmd,
                                         (USHORT)(sim_toolkit_req->stk_cmd.l_cmd >> 3), 
                                          NOT_PRESENT_16BIT);

  /*
   * Special treatment of SW1 is required for Call Control/MO-SM Control by SIM
   */
  if (sim_toolkit_cnf->cause NEQ SIM_NO_ERROR)
  {
    switch (sim_toolkit_req->stk_cmd.cmd[0])  /* ENVELOPE tag */
    {
    case 0xD4:  /* Call Control */
    case 0xD5:  /* MO-SM Control */
      if (sim_data.sw1 EQ 0x6F OR sim_data.sw1 EQ 0x92) /* Card problem */
        sim_toolkit_cnf->cause = SIM_NO_ERROR;
      break;
    default:
      break;
    }
  }

  if (sim_data.sim_data_len > 0)
  {
    sim_toolkit_cnf->stk_cmd.l_cmd = stk_l_cmd;
    stk_l_cmd = 0;
  }
  else
    sim_toolkit_cnf->stk_cmd.l_cmd = 0;

  /*
   * send confirmation to requesting entity
   */
  if (sim_toolkit_req->source EQ SRC_MMI)
  {
    PSENDX (MMI, sim_toolkit_cnf);
  }
  else
  {
    PSENDX (SMS, sim_toolkit_cnf);
  }
  PFREE (sim_toolkit_req);
  stk_start_timer_and_poll();
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_file_update_res        |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_FILE_UPDATE_RES. A File Change
            Notification is responded.

*/

GLOBAL void stk_file_update_res (T_SIM_FILE_UPDATE_RES * file_update_res)
{
  UBYTE *p_res;     // access to result code

  TRACE_FUNCTION ("stk_file_update_res()");

  if (file_update_res->source EQ SRC_MMI)
    sim_data.file_change_resp &= ~1;
  else if (file_update_res->source EQ SRC_SMS)
    sim_data.file_change_resp &= ~2;
  else if (file_update_res->source EQ SRC_MM)
    sim_data.file_change_resp &= ~4;
  else
  {
    PFREE (file_update_res);
    return;
  }
  p_res = &sim_data.stk_response[sim_data.stk_response[1] + 8];
  if (file_update_res->fu_rsc EQ SIM_FU_SUCC_ADD)
    *p_res = STK_RES_SUCC_ADD_EF_READ;

  if (sim_data.file_change_resp EQ 0)
  {
    FKT_TerminalResponse (sim_data.stk_response, (USHORT)sim_data.stk_resp_len);
    stk_start_timer_and_poll();
  }
  PFREE (file_update_res);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_timeout                |
+--------------------------------------------------------------------+

  PURPOSE : Process the timeout of a timer established by the SAT
            command TIMER MANAGEMENT.

*/

static const  UBYTE timer_env[] = {
  STK_TIMER_EXPIRATION_TAG,
  STK_DEVICE_IDENTITY_LEN+STK_TIMER_ID_LEN+STK_TIMER_VALUE_LEN+6,
  STK_DEVICE_IDENTITY_TAG|STK_COMPREHENSION_REQUIRED, STK_DEVICE_IDENTITY_LEN, 0x82, 0x81,
  STK_TIMER_ID_TAG|STK_COMPREHENSION_REQUIRED, STK_TIMER_ID_LEN, 0,
  STK_TIMER_VALUE_TAG|STK_COMPREHENSION_REQUIRED, STK_TIMER_VALUE_LEN, 0, 0, 0
};

extern UBYTE pending_timers[];
extern UBYTE next_pos_to_fill;
extern UBYTE next_pos_to_send;


GLOBAL void stk_timeout (USHORT index)
{
  UBYTE env[sizeof(timer_env)];
  UBYTE dummy[4];
  USHORT error;

  TRACE_FUNCTION("stk_timeout");
#ifdef FF_SAT_E
  /*
   * handle BIP timeout
   */
  if (index EQ SIM_BIP_TIMER)
  {
    /*
     * close DTI connection
     */
    switch(sim_data.dti_connection_state)
    {
      case SIM_DTI_CONNECTION_OPEN:
      case SIM_DTI_CONNECTION_SETUP:
        stk_close_dti_connection(TRUE);
        break;

      default:
        stk_close_dti_connection(FALSE);
        break;
    }
    /*
     * close BIP channel and inform MMI
     */
    stk_close_bip_channel(RSLT_BEARIND_PERR, ADD_BIP_CHAN_CLOSD);
    stk_dti_inform_mmi(SIM_DTI_DISCONNECT, SIM_BIP_CLOSE_CHANNEL);
    return;
  }
#endif /* FF_SAT_E */

  if ((unsigned)(--index) >= MAX_SAT_TIMER)   /* index range 0..7 */
    return;

  if (sim_data.timer[index].active)
  {
    memcpy (env, timer_env, sizeof(timer_env));
    env[8] = (UBYTE)(index + 1);  /* Timer number range is 1..8 */
    env[11] = sim_data.timer[index].hour;
    env[12] = sim_data.timer[index].minute;
    env[13] = sim_data.timer[index].second;
  
    error = FKT_Envelope (dummy, env, sizeof(timer_env), 0);
//TISH, test patch for OMAPS00179771: SATK: Timer Expiration, modified by Jinshu Wang, 2008-09-01
//start
#if 0
    if (error EQ SIM_NO_ERROR)
      stk_start_timer_and_poll();
#else
    if (error EQ SIM_NO_ERROR)
    {
      stk_start_timer_and_poll();
      //modified by Jinshu Wang, 2008-09-04
      sim_data.timer[index].active = FALSE;
      if(sim_data.chk_sat_avail)
      {
	sim_data.chk_sat_avail = FALSE;
	stk_proactive_polling();
      }
      return;	 //modified by Jinshu Wang, 2008-09-04
    }
#endif
//end
    /*
     * If SIM response is busy(9300), we have to once retry sending 
     * timer-expiry envelope after SIM becomes OK on getting a TR
     */  
    if (error EQ SIM_CAUSE_SAT_BUSY)
    {
      pending_timers[next_pos_to_fill] = (UBYTE) index;
      if (8 == next_pos_to_fill)
      {
        next_pos_to_fill = 0;
      }
      else
      {
        next_pos_to_fill++;
      }
    }
    sim_data.timer[index].active = FALSE;
  }
}

#ifdef TI_PS_FF_AT_P_CMD_CUST
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : stk_sim_refresh_user_res   |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive SIM_REFRESH_USER_RES. MMI sends a
            Response Message to SIM_TOOLKIT_IND.
            This Primitive should not occur when cust_mode is 0

*/

GLOBAL void stk_sim_refresh_user_res (T_SIM_REFRESH_USER_RES * sim_refresh_user_res)
{
  TRACE_FUNCTION ("stk_sim_refresh_user_res()");

  /* primitive should not occur for cust_mode 0 */
  if (sim_data.cust_mode EQ 0)
  {
    //Handle error condition
    TRACE_FUNCTION_P1 ("!!!!Incorrect operation: Unexpected mode: cust_mode = %d !!!!",sim_data.cust_mode);
    TRACE_FUNCTION ("Primitive should not occur if cust_mode = 0");
    //MFREE (sim_data.context_switch_ptr); Don't free context as it should be freed by other thread. i.e. stk_proactive_polling()
    PFREE (sim_refresh_user_res);
    return;
  }

  /* check that we are expecting primitive */
  if ( sim_data.user_confirmation_expected EQ FALSE )
  {
    //Handle error condition
    TRACE_FUNCTION ("!!!!Incorrect operation: user_confirmation_expected is FALSE !!!!");
    TRACE_FUNCTION ("Primitive should only occur if user_confirmation_expected is TRUE");
    //MFREE (sim_data.context_switch_ptr); Don't free context switch as setup can not be guaranteed to be correct.
    PFREE (sim_refresh_user_res);
    return;
  }
  else
  {
    //Reset user_confirmation_expected
    sim_data.user_confirmation_expected = FALSE;
  }


  if ((sim_refresh_user_res->user_accepts) EQ FALSE)
  {
    TRACE_FUNCTION ("User REJECTS Refresh Request");

    FKT_TerminalResponse (sim_refresh_user_res->stk_cmd.cmd,
                          (USHORT)(sim_refresh_user_res->stk_cmd.l_cmd>>3));
    sim_data.ext_sat_cmd = FALSE;
    stk_start_timer_and_poll();

    //process_sim_refresh() automatically frees the signal.
    //As we are not calling process_sim_refresh() we need to handle the freeing
    TRACE_ASSERT(sim_data.context_switch_ptr->sig_ptr);
    PFREE (sim_data.context_switch_ptr->sig_ptr); 
  }
  else
  {
    TRACE_FUNCTION ("User ACCEPTS Refresh Request");
    process_sim_refresh(sim_data.context_switch_ptr);
    TRACE_FUNCTION ("stk_sim_refresh_user_res() send end of SAT session indicator");
    if ((sim_data.term_resp_sent) AND (sim_data.sat_session))
    {
      PALLOC (sim_toolkit_ind, SIM_TOOLKIT_IND);
      memset (sim_toolkit_ind, 0, sizeof (T_SIM_TOOLKIT_IND));
#ifdef _SIMULATION_
      TRACE_EVENT("SAT session ended");
#endif
      sim_data.sat_session    = FALSE;
      sim_data.term_resp_sent = FALSE;
      PSENDX (MMI, sim_toolkit_ind);
    }

  }
  TRACE_ASSERT(sim_data.context_switch_ptr);
  MFREE (sim_data.context_switch_ptr);
  sim_data.context_switch_ptr = NULL;

  PFREE (sim_refresh_user_res);
  TRACE_FUNCTION ("stk_sim_refresh_user_res() exited");
}
#endif /* TI_PS_FF_AT_P_CMD_CUST */

#endif

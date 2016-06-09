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
|  Purpose :  Definitions for the Protocol Stack Entity Call Control.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CC_H
#define CC_H

#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"
#endif

#ifdef TI_GSP_STR2IND_S2I_STRING
typedef unsigned int T_S2I_STRING;
#else
typedef char * T_S2I_STRING;
#define S2I_STRING(x) (x)
#endif

#ifdef TI_PS_OP_VSI_NO_CALL_ID
#define TIMER_START(C,I,T)         vsi_t_start_nc(I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart_nc(I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop_nc(I)
#define TIMER_STATUS(C,I,T)        vsi_t_status_nc(I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep_nc(T)
#define SYSTEM_TIME(C,T)           vsi_t_time_nc(T)               
#else /* TI_PS_OP_VSI_NO_CALL_ID */
#define TIMER_START(C,I,T)         vsi_t_start(C,I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart(C,I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop(C,I)
#define TIMER_STATUS(C,I,T)        vsi_t_status(C,I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep(C,T)
#define SYSTEM_TIME(C,T)           vsi_t_time(C,T)               
#endif /* TI_PS_OP_VSI_NO_CALL_ID */

#ifdef MNCC_TI_BERLIN

/* The code is compiled for TI Berlin, some differences (for now) */
#define TI_BERLIN

/* Texas Instruments Berlin wants to compile for GSM always */
#define TI_GSM

/* Define TI_CAUSE_U16 for TI Berlin U16 bitmap cause concept */
#define TI_CAUSE_U16

/* Make a compatible definition for "no cause present" */
#define CAUSE_CC_NO_ERROR MNCC_CAUSE_NO_MS_CAUSE

/* Protocoldiscriminator CC */
#define M_MM_PD_CC              3

/* Originating entity for MMCM */
#define NAS_ORG_ENTITY_CC       MMCM_ORG_ENTITY_CC

/* 
 * TI Berlin has no "NAS_" defines and typedef, map what's needed to compile
 */
#define NAS_CH_SDCCH      MMCM_CH_SDCCH
#define NAS_CH_TCH_F      MMCM_CH_TCH_F
#define NAS_CH_TCH_H      MMCM_CH_TCH_H

#define NAS_CHM_SIG_ONLY  MMCM_CHM_SIG_ONLY
#define NAS_CHM_SPEECH    MMCM_CHM_SPEECH
#define NAS_CHM_SPEECH_V2 MMCM_CHM_SPEECH_V2
#define NAS_CHM_SPEECH_V3 MMCM_CHM_SPEECH_V3
#define NAS_CHM_DATA_9_6  MMCM_CHM_DATA_9_6
#define NAS_CHM_DATA_4_8  MMCM_CHM_DATA_4_8
#define NAS_CHM_DATA_2_4  MMCM_CHM_DATA_2_4
#define NAS_CHM_DATA_14_4 MMCM_CHM_DATA_14_4

typedef T_MNCC_fac_inf T_NAS_fac_inf;

#else /* TI DK */
#define IS_CAUSE_INVALID(cause)         (cause EQ CAUSE_CC_NO_ERROR)
#endif /* else, #ifdef MNCC_TI_BERLIN */


/*
 * Some macros to give the optimize the chance to optimize code away, 
 * making it possible to save some #ifdefs for GSM / UMTS only configurations.
 */
#ifdef TI_DUAL_MODE
/* In dual mode we have to evaluate the RAT at runtime */
#define IS_RAT_UMTS_FDD() (cc_current_rat EQ PS_RAT_UMTS_FDD)
#define IS_RAT_GSM()      (cc_current_rat EQ PS_RAT_GSM)
#else
#ifdef TI_GSM
/* GSM/GPRS only phone, RAT is always GSM/GPRS */
#define IS_RAT_UMTS_FDD() FALSE
#define IS_RAT_GSM()      TRUE
#else
/* UMTS only phone, RAT is always UMTS */
#define IS_RAT_UMTS_FDD() TRUE
#define IS_RAT_GSM()      FALSE
#endif
#endif /* #ifdef TI_DUAL_MODE */


/* 
 * Definition whether bearer caps shall be traced
 */
#ifndef NTRACE
#define TRACE_BCAP
#endif

/*
 * Definition whether a shared CCD buffer shall be used
 */
#define SHARED_CCD_BUF

/*==== CONSTANTS ==================================================*/
/*
 *  Macros
 */


#ifdef FRAME_OFFSET_ZERO

#define GET_PD(s,p)  p = s.buf[3] & 0x0F
#define GET_TI(s,t)  t = (s.buf[3] & 0xF0)>>4
#define SET_PD(s,p)  s.buf[3] = (s.buf[3] & 0xF0) + p
#define SET_TI(s,t)  s.buf[3] = (s.buf[3] & 0x0F) + (t<<4)

#else

#define GET_PD(s,p)  ccd_decodeByte(s.buf,(USHORT)(s.o_buf+4),4,&p)
#define GET_TI(s,t)  ccd_decodeByte(s.buf,s.o_buf,4,&t)
#define GET_TI_EXT(s,t) ccd_decodeByte(s.buf,(USHORT)(s.o_buf+8),8,&t)
#define SET_PD(s,p)  ccd_codeByte(s.buf,(USHORT)(s.o_buf-4),4,p)
#define SET_TI(s,t)  ccd_codeByte(s.buf,(USHORT)(s.o_buf-8),4,t)

#endif

#define SET_DTMF_MOD( key, mode ) (((key) & 0x7F) | ((mode) << 7))
#define GET_DTMF_MOD( key )       (((key) & 0x80) >> 7)

/*
 * The assert() macro as defined by the frame stops the task, 
 * not only in simulation but also on the target. For the 
 * simulation this is a desired behaviour, for the target it is not. 
 */
#ifndef WIN32
#undef assert
#define assert(x) if (!(x)) { TRACE_ERROR ("Assertion failed"); }
#endif 
 
/*
 * Bitoffset for encoding/decoding
 */
#define CC_ENCODE_OFFSET   L3_CODING_OFFSET
#define    ENCODE_OFFSET   CC_ENCODE_OFFSET /* macro used by mem alloc macros */


/*
 * Dynamic Configuration Numbers
 */
/* Not present currently for entity CC */
#ifdef WIN32
#define ID_STD      7
#endif/* WIN32 */

/*
 * Timer indices, configuration parameters
 */
#define  T303          0
#define  T305          1
#define  T308          2
#define  T310          3
#define  T313          4
#define  T323          5
#define  T332          6
#define  T335          7
#define  T336          8
#define  T337          9

#define NUM_OF_CC_TIMERS  MAX_CC_TIMER


/*
 * Information Element Identifier
 */
#define PROGRESS_IEI           0x1E
#define REPEAT_IEI             0x0D
#define CONNECTED_NUMBER_IEI   0x4C
#define CONNECTED_SUBADDR_IEI  0x4D
#define CAUSE_IEI              0x08
#define NOTIFICATION_IEI       0x00    /* ???? */
#define SIGNAL_IEI             0x34
#define CALLING_PARTY_BCD_IEI  0x5C
#define CALLING_PARTY_SUB_IEI  0x5D
#define CALLED_PARTY_BCD_IEI   0x5E
#define CALLED_PARTY_SUB_IEI   0x6D
#define BEARER_CAP_IEI         0x04

/*
 * Call Type
 */
#define CALL_TYPE_MTC          1
#define CALL_TYPE_MOC          0

/*
 * DTMF States
 */
#define DTMF_IDLE              0
#define DTMF_SEND_REQUEST      1
#define DTMF_STOP_REQUEST      2
#define DTMF_SEND_ACKNOWLEDGE  3

/*
 * DTMF mode
 */
#define DTMF_AUTO              0
#define DTMF_MAN               1

/*
 * Compatibility Check Result Codes
 */
#define OKAY                   0
#define NEGOTIATION            1
#define ERROR                  2
#define BAD_SUBADDRESS         3

/*
 * Intermediate States
 */
#define CS_101                 0xd
#define CS_261                 0xe

/*
 * Maximum Length of SDU
 */
#define LEN_U_SETUP             (128*8)
#define LEN_U_RELEASE_COMPLETE  ( 34*8)
#define LEN_U_RELEASE           ( 34*8)
#define LEN_U_DISCONNECT        ( 33*8)
#define LEN_U_MODIFY_REJ        ( 47*8)
#define LEN_U_CONNECT           (  2*8)
#define LEN_U_ALERT             (  2*8)
#define LEN_U_USER_USER         (  5*8)
#define LEN_U_FACILITY          (  5*8)

/*
 * Max number of speech codecs for bearer capabilities
 */
#define MAX_SPEECH_CODECS      5

/*
 * Define the valid flag for the extended transaction identifier
 */

#define EXTENDED_TI_VALID     0x80


/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE      t_mode;
  ULONG      t_val;
} T_TIMER_CONFIG;

typedef struct
{
  UBYTE      state;
  UBYTE      mode;
  UBYTE      key;                 /* Last key which was sent */
  UBYTE      read;
  UBYTE      write;
  UBYTE      buf[DTMF_BUF_SIZE];
} T_DTMF;

typedef struct
{
  UBYTE               serv1;
  UBYTE               serv2;
  UBYTE               ri;
  T_M_CC_bearer_cap   bc1;
  T_M_CC_bearer_cap   bc2;
  T_MNCC_bcpara       bcpara1;
  T_MNCC_bcpara       bcpara2;
  UBYTE               neg_serv1;
  UBYTE               neg_serv2;
  UBYTE               neg_ri;
  T_M_CC_bearer_cap   neg_bc1;
  T_M_CC_bearer_cap   neg_bc2;
  T_MNCC_bcpara       neg_bcpara1;
  T_MNCC_bcpara       neg_bcpara2;
  UBYTE               active_service;
  UBYTE               mt;
  UBYTE               ti;       /* Transaction Identifier */
  UBYTE               ti_ext;   /* extension octet of ti, if any */
  UBYTE               ti_rec;
  BOOL                ti_ext_pres; /* presence of extension octet of ti */

  UBYTE               index_ti;
  UBYTE               error;
  UBYTE               error_count;
  UBYTE               error_inf [MAX_ERROR_TAGS];
  UBYTE               channel_type;
  UBYTE               channel_mode;
  UBYTE               progress_desc[MAX_CC_CALLS];
  UBYTE               new_itc;
  UBYTE               old_itc;
  T_M_CC_calling_subaddr   my_party_subaddr;
  BOOL                negotiation;
  UBYTE               timer [MAX_CC_TIMER];
  UBYTE               stored_ti_values [MAX_CC_CALLS];
  UBYTE               call_type [MAX_CC_CALLS];
  // PATCH LE 10.04.00
  // new variables to check disconnect collision
  UBYTE               disc_coll [MAX_CC_CALLS];
  // END PATCH LE 10.04.00
  UBYTE               t308_counter [MAX_CC_CALLS];
  T_DTMF              dtmf [MAX_CC_CALLS];
  UBYTE               state [MAX_CC_CALLS];
  UBYTE               hold_state [MAX_CC_CALLS];
  UBYTE               mpty_state [MAX_CC_CALLS];
  /* stores the cause used by the MS (not by the network) */
  USHORT               cc_cause [MAX_CC_CALLS];
#ifdef OPTION_REF
  T_PRIM            * stored_prim [MAX_STORED_PRIM];
#else
  T_PRIM              stored_prim [MAX_STORED_PRIM];
#endif
  UBYTE               stored_prim_write;
  UBYTE               stored_prim_read;
  UBYTE               stored_prim_in;
  UBYTE               stored_prim_out;
  EF_MSCAP            mscap;
  // EF_MSSUP            mssup;
  T_M_CC_call_ctrl_cap     call_ctrl_cap;
  /* The pointer to the readily built uplink SETUP or EC SETUP message */
  T_MMCM_DATA_REQ   * stored_setup;
  T_MNCC_bcpara       sns_bcpara;
  UBYTE               sns_mode;
  UBYTE               ctm_ena;
  /* Connection element to be used in a MTC, set by at+cbst */
  UBYTE               conn_elem;
  /* Pointer to stored MMCC_DATA_REQ which contains the CCBS SETUP message */
  T_MMCM_DATA_REQ   * stored_ccbs_setup;
  /* Establishment cause */
  USHORT              estcs;
  /* Number of retries to establish a MO call */
  UBYTE               setup_attempts;
  /* Transaction identifier for setup reattempt, NOT_PRESENT_8BIT if none */
  UBYTE               setup_reattempt_ti;
  /* Buffer for trace outputs, it doesn't save you anything if this 
   * buffer is kept on the stack as stack is a scarce resource */
#ifdef TRACE_BCAP
  char                string_buf[80];
#endif /* #ifdef TRACE_BCAP */
} T_CC_DATA;

/*==== EXPORT =====================================================*/

/*
 * Prototypes Formatter
 */
EXTERN void for_init                 (void);
EXTERN void for_mmcm_est_ind         (T_MMCM_ESTABLISH_IND  * mmcm_establish_ind);

EXTERN void for_mmcm_data_ind        (T_MMCM_DATA_IND       * mmcm_data_ind     );
EXTERN void for_decode               (T_PRIM * prim,
                                      BOOL est_flag);
EXTERN void for_d_alert              (void);
EXTERN void for_d_call_proceed       (void);
EXTERN void for_d_connect            (void);
EXTERN void for_d_facility           (void);
EXTERN void for_d_disconnect         (void);
EXTERN void for_b_modify             (void);
EXTERN void for_b_modify_comp        (void);
EXTERN void for_b_modify_rej         (void);
EXTERN void for_b_notify             (void);
EXTERN void for_d_progress           (void);
EXTERN void for_d_release            (void);
EXTERN void for_d_release_comp       (void);
EXTERN void for_d_cc_establishment   (void);
EXTERN void for_d_recall             (void);
EXTERN void for_d_setup              (void);
EXTERN void for_d_start_dtmf_ack     (void);
EXTERN void for_d_start_dtmf_rej     (void);
EXTERN void for_b_status             (void);
EXTERN void for_d_hold_rej           (void);
EXTERN void for_d_retrieve_rej       (void);
EXTERN void for_b_congest_ctrl       (void);
EXTERN void for_b_user_information   (void);

EXTERN BOOL cc_check_critical_error  (UBYTE       ccd_err);

EXTERN void for_mmcm_sync_ind        (T_MMCM_SYNC_IND       * mmcm_sync_ind       );
EXTERN void for_mmcm_est_cnf         (T_MMCM_ESTABLISH_CNF  * mmcm_establish_cnf  );
EXTERN void for_mmcm_rel_ind         (T_MMCM_RELEASE_IND    * mmcm_release_ind    );
EXTERN void for_mmcm_err_ind         (T_MMCM_ERROR_IND      * mmcm_error_ind      );
EXTERN void for_mmcm_reest_cnf       (T_MMCM_REESTABLISH_CNF* mmcm_reestablish_cnf);
EXTERN void for_mmcm_prompt_ind      (T_MMCM_PROMPT_IND     * mmcm_prompt_ind);
EXTERN void for_est_req              (T_MMCM_ESTABLISH_REQ  * mmcm_establish_req  );

EXTERN void for_rel_req              (void);
EXTERN void for_reest_req            (void);
EXTERN void for_start_cc             (T_U_START_CC * start_cc);
EXTERN void for_cc_est_confirm       (T_U_CC_EST_CONF * est_cnf);
EXTERN void for_setup                (T_U_SETUP * setup);
EXTERN void for_emergency_setup      (T_U_EMERGE_SETUP * emerg_setup);
EXTERN void for_status               (T_B_STATUS * status);
EXTERN void for_release_complete     (T_U_RELEASE_COMP * rel_com);
EXTERN void for_disconnect           (T_U_DISCONNECT * disconnect);
EXTERN void for_modify_reject        (T_B_MODIFY_REJ * modify_rej);
EXTERN void for_modify_complete      (T_B_MODIFY_COMP * mod_com);
EXTERN void for_start_dtmf           (T_U_START_DTMF * start_dtmf);
EXTERN void for_stop_dtmf            (void);
EXTERN void for_call_confirm         (T_U_CALL_CONF * call_confirm);
EXTERN void for_release              (T_U_RELEASE * release);
EXTERN void for_connect_ack          (T_B_CONNECT_ACK *b_connect_ack);
EXTERN void for_retrieve             (void);
EXTERN void for_hold                 (void);
EXTERN void for_user_information     (T_B_USER_INFO * user_info);
EXTERN void for_congestion_control   (T_B_CONGEST_CTRL * cong);
EXTERN void for_modify               (T_B_MODIFY * modify);
EXTERN void for_connect              (T_U_CONNECT * connect);
EXTERN void for_alert                (T_U_ALERT * alert);
EXTERN void for_facility             (T_U_FACILITY * facility);
EXTERN void for_pd                   (T_MMCM_DATA_REQ * mmcm_data_req);

/*
 * Prototypes Formatter Functions
 */
EXTERN BOOL for_check_called_party_bcd (UBYTE ton, UBYTE npi);
EXTERN BOOL for_check_called_party_sub (T_M_CC_called_subaddr * called_subaddr);
EXTERN BOOL for_check_calling_party_bcd (T_M_CC_calling_num * calling_num);
EXTERN BOOL for_check_calling_party_sub (T_M_CC_calling_subaddr * calling_subaddr);
EXTERN void for_check_call_state       (T_M_CC_call_state * call_state);
EXTERN BOOL for_check_cc_cause         (T_M_CC_cc_cause * cc_cause);
EXTERN BOOL for_check_notification     (T_M_CC_notific * notify);
EXTERN BOOL for_check_progress_indicator (T_M_CC_progress * progress);
EXTERN BOOL for_check_repeat_indicator (UBYTE repeat);
EXTERN BOOL for_check_signal           (UBYTE signal);
EXTERN BOOL cc_check_critical_error (UBYTE ccd_err);
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define for_set_conditional_error  _ENTITY_PREFIXED(for_set_conditional_error)
  #ifdef REL99
  #define for_set_content_error      _ENTITY_PREFIXED(for_set_content_error)
  #endif
  #define for_set_mandatory_error    _ENTITY_PREFIXED(for_set_mandatory_error)
  #define for_set_optional_error     _ENTITY_PREFIXED(for_set_optional_error)
#endif


EXTERN void for_set_conditional_error  (UBYTE iei);
EXTERN void for_set_mandatory_error    (UBYTE iei);
EXTERN void for_set_optional_error     (UBYTE iei);


/*
 * Prototypes Call Control (Establishment)
 */
EXTERN void cc_init                 (void);
EXTERN void cc_init_data            (void);
EXTERN void cc_mncc_configure_req   (T_MNCC_CONFIGURE_REQ * mncc_configure_req);
EXTERN void cc_mncc_setup_req       (T_MNCC_SETUP_REQ     * mncc_setup_req    );
EXTERN void cc_mncc_prompt_res      (T_MNCC_PROMPT_RES * prompt);
EXTERN void cc_mncc_prompt_rej      (T_MNCC_PROMPT_REJ * prompt);
EXTERN void cc_mncc_alert_req       (T_MNCC_ALERT_REQ     * mncc_alert_req    );
EXTERN void cc_mncc_setup_res       (T_MNCC_SETUP_RES     * mncc_setup_res    );


EXTERN void cc_alert                (T_D_ALERT * alert);
EXTERN void cc_call_proceeding      (T_D_CALL_PROCEED * proceed);
EXTERN void cc_connect              (T_D_CONNECT * connect);
EXTERN void cc_connect_ack          (void);
EXTERN void cc_progress             (T_D_PROGRESS * progress);
EXTERN void cc_setup                (T_D_SETUP * setup);
EXTERN void cc_sync_ind             (T_MMCM_SYNC_IND *mmcc_sync_ind);
EXTERN void cc_est_cnf              (void);
EXTERN void cc_reest_cnf            (void);
EXTERN void cc_cc_establishment     (T_U_SETUP * setup);
EXTERN void cc_recall               (T_D_RECALL * recall);

#ifdef SIM_TOOLKIT
EXTERN void cc_mncc_bearer_cap_req  (T_MNCC_BEARER_CAP_REQ * bc_req);
#endif /* SIM_TOOLKIT */

/*
 * Prototypes Call Control (Active Phase)
 */
EXTERN void cc_mncc_facility_req    (T_MNCC_FACILITY_REQ   * mncc_facility_req  );
EXTERN void cc_mncc_start_dtmf_req  (T_MNCC_START_DTMF_REQ * mncc_start_dtmf_req);
EXTERN void cc_reset_dtmf           (void);
EXTERN void cc_mncc_modify_req      (T_MNCC_MODIFY_REQ     * mncc_modify_req    );
EXTERN void cc_mncc_user_req        (T_MNCC_USER_REQ       * mncc_user_req      );
EXTERN void cc_mncc_hold_req        (T_MNCC_HOLD_REQ       * mncc_hold_req      );
EXTERN void cc_mncc_retrieve_req    (T_MNCC_RETRIEVE_REQ   * mncc_retrieve_req  );
EXTERN void cc_mncc_sync_req        (T_MNCC_SYNC_REQ       * mncc_sync_req      );
EXTERN void cc_mncc_status_res      (T_MNCC_STATUS_RES     * mncc_status_res    );


EXTERN void cc_facility             (T_D_FACILITY * facility);
EXTERN void cc_start_dtmf_ack       (T_D_START_DTMF_ACK * start_dtmf_ack);
EXTERN void cc_start_dtmf_rej       (T_D_START_DTMF_REJ * start_dtmf_rej);
EXTERN void cc_stop_dtmf            (void);
EXTERN void cc_stop_dtmf_ack        (void);
EXTERN void cc_hold_ack             (void);
EXTERN void cc_hold_rej             (T_D_HOLD_REJ * hold_rej);
EXTERN void cc_retrieve_ack         (void);
EXTERN void cc_retrieve_rej         (T_D_RETRIEVE_REJ * retrieve_rej);
EXTERN void cc_congestion_control   (T_B_CONGEST_CTRL * cong);
EXTERN void cc_user_information     (T_B_USER_INFO * user);
EXTERN void cc_modify               (T_B_MODIFY * modify);
EXTERN void cc_mod_reject           (void);
EXTERN void cc_mod_complete         (void);
EXTERN void cc_modify_complete      (T_B_MODIFY_COMP * mod_com);
EXTERN void cc_modify_reject        (T_B_MODIFY_REJ * mod_rej);
EXTERN void cc_status               (T_B_STATUS * status);
EXTERN void cc_status_enquiry       (void);
EXTERN void cc_unknown_message      (void);


/*
 * Prototypes Call Control (Release)
 */
EXTERN void cc_mncc_disconnect_req  (T_MNCC_DISCONNECT_REQ * mncc_disconnect_req);
EXTERN void cc_mncc_reject_req      (T_MNCC_REJECT_REQ * rej);
EXTERN void cc_mncc_release_req     (T_MNCC_RELEASE_REQ    * mncc_release_req   );

EXTERN void cc_disconnect           (T_D_DISCONNECT * disconnect);
EXTERN void cc_release              (T_D_RELEASE * release);
EXTERN void cc_release_complete     (T_D_RELEASE_COMP * rel_com);
EXTERN void cc_rel_ind              (USHORT cause);
EXTERN void cc_err_ind              (T_MMCM_ERROR_IND *mmcm_error_ind);

/*
 * Prototypes Call Control Functions
 */
EXTERN BOOL cc_voice_capability        (void);
EXTERN BOOL cc_bcs_compatible    (const T_M_CC_bearer_cap * bc1,
                                  const T_M_CC_bearer_cap * bc2,
                                        BOOL           full);
EXTERN void cc_build_llc               (UBYTE            * v_low_layer_compat,
                                        T_M_CC_low_layer_comp * low_layer_compat,
                                  const T_M_CC_bearer_cap     * bc);

EXTERN void cc_build_bc                (T_M_CC_bearer_cap * bearer_cap,
                                        UBYTE        * serv,
                                  const T_MNCC_bcpara     * bc_params);

EXTERN void cc_set_conn_elem           (T_M_CC_bearer_cap * bearer_cap,
                                        UBYTE          conn_elem, 
                                        UBYTE          flow_control);
EXTERN void cc_set_sync_async          (T_M_CC_bearer_cap * bearer_cap,
                                  const T_MNCC_bcpara     * bcpara);

EXTERN void cc_set_user_rate           (T_M_CC_bearer_cap * bearer_cap,
                                        UBYTE          rate,
                                        UBYTE          modem_type);
EXTERN void cc_build_call_confirm      (T_U_CALL_CONF * call_cnf,
                                        USHORT          cause);
EXTERN void cc_build_cc_est_confirm    (T_U_CC_EST_CONF * cc_est_conf,
                                        USHORT            cause);
EXTERN void cc_build_congestion_control (const T_MNCC_USER_REQ  * user,
                                               T_B_CONGEST_CTRL * cong_ctrl);
EXTERN void cc_build_disconnect        (T_U_DISCONNECT * disconnect,
                                        USHORT cause,
                                  const T_NAS_fac_inf      * fac_inf,
                                        UBYTE ss_ver);  
EXTERN void cc_build_emergency_setup   (T_U_EMERGE_SETUP * emergency_setup);
EXTERN void cc_build_facility    (const T_MNCC_FACILITY_REQ * facility,
                                        T_U_FACILITY * facility_msg);
EXTERN void cc_build_mncc_alert_ind    (const T_D_ALERT * alert,
                                        T_MNCC_ALERT_IND * alert_ind);
EXTERN void cc_build_mncc_proceed_ind  (const T_D_CALL_PROCEED * proceed,
                                        T_MNCC_CALL_PROCEED_IND * proceed_ind);
EXTERN void cc_build_mncc_facility_ind (const T_D_FACILITY * facility,
                                        T_MNCC_FACILITY_IND * facility_ind);
EXTERN void cc_build_mncc_progress_ind (const T_D_PROGRESS * progress,
                                        T_MNCC_PROGRESS_IND * progress_ind);
EXTERN void cc_build_mncc_setup_cnf    (const T_D_CONNECT * connect,
                                        T_MNCC_SETUP_CNF * setup_cnf);
EXTERN void cc_build_mncc_setup_ind    (const T_D_SETUP * setup,
                                        T_MNCC_SETUP_IND * setup_ind);
EXTERN void cc_build_modify            (const T_MNCC_MODIFY_REQ * modify,
                                        T_B_MODIFY * modify_msg);
EXTERN void cc_build_modify_complete   (T_B_MODIFY_COMP * modify_com);
EXTERN void cc_build_modify_reject     (T_B_MODIFY_REJ * modify_rej,
                                        USHORT           cause);
EXTERN void cc_build_release           (T_U_RELEASE * release,
                                        USHORT        cause,
                                  const T_NAS_fac_inf   * fac_inf,
                                        UBYTE         ss_ver);  
EXTERN void cc_build_release_complete  (T_U_RELEASE_COMP * rel_com,
                                        USHORT             cause);
EXTERN void cc_build_start_cc          (T_U_START_CC * start_cc);
EXTERN void cc_build_llc_hlc           (T_U_SETUP * setup);
EXTERN void cc_build_cc_capabilities   (T_U_SETUP * setup);
EXTERN void cc_build_setup             (T_U_SETUP * setup_msg,
                                        const T_MNCC_SETUP_REQ *setup_prm);
EXTERN void cc_build_start_dtmf        (UBYTE key,
                                        T_U_START_DTMF * start_dtmf);
EXTERN void cc_build_alert             (T_U_ALERT * alert_msg);
EXTERN void cc_build_connect           (T_U_CONNECT * connect_msg);
EXTERN void cc_build_status            (T_B_STATUS * status,
                                        USHORT       cause);
EXTERN void cc_build_user_ind_from_cong (T_MNCC_USER_IND * user,
                                   const T_B_CONGEST_CTRL * cong);
EXTERN void cc_build_user_ind_from_user (T_MNCC_USER_IND * user_ind,
                                   const T_B_USER_INFO * user);
EXTERN void cc_build_user_information  (T_MNCC_USER_REQ * user,
                                        T_B_USER_INFO * user_msg);

EXTERN BOOL cc_check_error_flag        (void);
EXTERN UBYTE cc_compatibility_check    (const T_D_SETUP * setup);
EXTERN UBYTE cc_basic_service_align    (T_U_SETUP * setup);
EXTERN UBYTE cc_count_active_connections (void);
EXTERN void cc_disconnect_after_timeout (void);
EXTERN void cc_encode_cause            (T_M_CC_cc_cause * cc_cause,
                                        USHORT       cause);
EXTERN UBYTE cc_mtc_check_subaddr      (const T_D_SETUP * setup);
EXTERN UBYTE cc_moc_compatibility      (const T_D_CALL_PROCEED * proceed);
EXTERN void cc_set_state               (UBYTE new_state);
EXTERN UBYTE cc_check_bc         (const T_M_CC_bearer_cap * bearer_cap,
                                        T_M_CC_bearer_cap * neg_bearer_cap,
                                        T_MNCC_bcpara     * bcpara,
                                        UBYTE        * service,
                                        BOOL         * negotiation);
EXTERN void cc_set_radio_channel_requirement 
                                       (T_M_CC_bearer_cap * bearer_cap,
                                  const T_MNCC_bcpara     * bc_para);
EXTERN void cc_set_trans_cap           (T_M_CC_bearer_cap * bearer_cap,
                                  const T_MNCC_bcpara     * bcpara);
EXTERN void cc_set_data_default_parameter 
                                       (T_M_CC_bearer_cap * bearer_cap);
EXTERN void cc_build_facility_ind      (UBYTE          context,
                                        UBYTE          valid,
                                  const T_M_CC_facility   * facility);
EXTERN void cc_build_user_user_ind     (UBYTE          context,
                                        UBYTE          valid,
                                        T_M_CC_user_user  * user_user);
EXTERN UBYTE cc_check_capabilities     (const T_MNCC_bcpara     * bcpara);
EXTERN void cc_check_transparent_async (UBYTE        * found,
                                  const T_M_CC_bearer_cap * bearer_cap,
                                        T_M_CC_bearer_cap * neg_bearer_cap,
                                        T_MNCC_bcpara     * bcpara,
                                        BOOL         * negotiation);
EXTERN void cc_check_non_transparent_async 
                                       (UBYTE        * found,
                                  const T_M_CC_bearer_cap * bearer_cap,
                                        T_M_CC_bearer_cap * neg_bearer_cap,
                                        T_MNCC_bcpara     * bcpara,
                                        BOOL         * negotiation);
EXTERN UBYTE cc_check_transparent_fax (const T_M_CC_bearer_cap * bearer_cap,
                                             T_M_CC_bearer_cap * neg_bearer_cap,
                                             T_MNCC_bcpara     * bcpara,
                                             BOOL         * negotiation);

#ifdef SIM_TOOLKIT
EXTERN UBYTE cc_bearer_cap_code (const T_MNCC_bcpara *bcpara, 
                                       T_MNCC_bcconf *bcconf );
EXTERN UBYTE cc_bearer_cap_decode (const T_MNCC_bcconf *bcconf, 
                                         T_MNCC_bcpara *bcpara );
#endif


/* CQ 23619: Added supplementary diagnostic handling */
EXTERN UBYTE cc_get_ss_diag ( USHORT curr_cause,
                              T_D_DISCONNECT * disc);

EXTERN UBYTE cc_build_cause (T_M_CC_cc_cause  * decoded_cause_val,
                             UBYTE            * raw_cause_bytes);


/* Implements Measure#  3 and streamline encoding */
EXTERN void cc_send_status (USHORT cause);

/*
 * Prototypes Services
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define srv_convert_ti        _ENTITY_PREFIXED(srv_convert_ti)
  #define srv_define_ti         _ENTITY_PREFIXED(srv_define_ti)
  #define srv_free_ti           _ENTITY_PREFIXED(srv_free_ti)
  #define srv_free_stored_setup _ENTITY_PREFIXED(srv_free_stored_setup)
  #define srv_store_prim        _ENTITY_PREFIXED(srv_store_prim)
  #define srv_use_stored_prim   _ENTITY_PREFIXED(srv_use_stored_prim)
#endif

EXTERN UBYTE srv_convert_ti            (UBYTE ti);
EXTERN UBYTE srv_define_ti             (void);
EXTERN void  srv_free_ti               (void);
EXTERN void  srv_free_stored_setup     (void);
EXTERN void  srv_store_prim            (T_PRIM * prim);
EXTERN void  srv_use_stored_prim       (void);

/*
 * Prototypes Timer
 */
/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define tim_exec_timeout      _ENTITY_PREFIXED(tim_exec_timeout)
  #define tim_t303_u0_1         _ENTITY_PREFIXED(tim_t303_u0_1)
  #define tim_t303_u1           _ENTITY_PREFIXED(tim_t303_u1)
  #define tim_t305_u11          _ENTITY_PREFIXED(tim_t305_u11)
  #define tim_t308_u19          _ENTITY_PREFIXED(tim_t308_u19)
  #define tim_t310_u3           _ENTITY_PREFIXED(tim_t310_u3)
  #define tim_t313_u8           _ENTITY_PREFIXED(tim_t313_u8)
  #define tim_t323_u26          _ENTITY_PREFIXED(tim_t323_u26)
  #define tim_t332_u0_3         _ENTITY_PREFIXED(tim_t332_u0_3)
  #define tim_t335_u0_5         _ENTITY_PREFIXED(tim_t335_u0_5)
  #define tim_t336              _ENTITY_PREFIXED(tim_t336)
  #define tim_t337              _ENTITY_PREFIXED(tim_t337)
#endif

EXTERN void tim_exec_timeout         (USHORT index);

EXTERN void tim_t303_u0_1            (void);
EXTERN void tim_t303_u1              (void);
EXTERN void tim_t305_u11             (void);
EXTERN void tim_t308_u19             (void);
EXTERN void tim_t310_u3              (void);
EXTERN void tim_t313_u8              (void);
EXTERN void tim_t323_u26             (void);
EXTERN void tim_t332_u0_3            (void);
EXTERN void tim_t335_u0_5            (void);
EXTERN void tim_t336                 (void);
EXTERN void tim_t337                 (void);



/*
 * Prototypes Customer Specific Functions
 */
GLOBAL void cc_csf_ms_cap (void);

#ifdef TRACE_BCAP
#define trace_buf                     cc_data->string_buf
#endif /* #ifdef TRACE_BCAP */

/* 
 * CC uses two timers for each instance, one for DTMF operation (T336, T337) 
 * and one for the support of all other CC timers.
 * The timer for the supervision of the CC states is addressed by the
 * transaction identifier, the timer for the supervision of DTMF operation 
 * is addressed by the transaction identifier plus an offset of MAX_CC_CALLS.
 * The macros are looking a little bit too complicated, 
 * but they have the advantage of avoiding a lot of #ifdefs in the
 * code and it is expected that the if clauses are optimized 
 * away by the C compiler. So no disadvantage at runtime is expected 
 * if these macros remain macros and are not altered to functions.
 */ 
#define TIMER_CC   0 /* Used to stop all timers except T336, T337 */
#define TIMER_DTMF 1 /* Uses to stop T336, T337 which control DTMF */

#define TIMERSTART(i,v) \
  if ((i NEQ T336) AND (i NEQ T337)) \
    TIMER_START (cc_handle, (USHORT)(cc_data->index_ti), v); \
  else \
    TIMER_START (cc_handle, (USHORT)(cc_data->index_ti+MAX_CC_CALLS), v);

#define TIMERSTOP(i) \
  if (i EQ TIMER_CC) \
    TIMER_STOP (cc_handle, (USHORT)(cc_data->index_ti)); \
  else \
    TIMER_STOP (cc_handle, (USHORT)(cc_data->index_ti+MAX_CC_CALLS));

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
#ifdef TI_PS_HCOMM_CHANGE
#else
  #define hCommMMI       _ENTITY_PREFIXED(hCommMMI)
  #define hCommMM        _ENTITY_PREFIXED(hCommMM)
#endif
#endif

#ifdef TI_PS_HCOMM_CHANGE
#else
EXTERN T_HANDLE  hCommMMI;               /* MMI Communication        */
EXTERN T_HANDLE  hCommMM;                /* MM  Communication        */
#endif
EXTERN T_HANDLE  cc_handle;

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif

#if !defined SHARED_CCD_BUF
#define CCD_START
#define CCD_END
EXTERN UBYTE          _decodedMsg [];
#else
EXTERN UBYTE *        _decodedMsg;
#define CCD_START _decodedMsg = ccd_begin();
#define CCD_END   ccd_end();
#endif

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define data_base    _ENTITY_PREFIXED(data_base)
#endif

#ifdef OPTION_MULTIPLE_INSTANCE

EXTERN T_CC_DATA data_base [MAX_INSTANCES];

#define GET_INSTANCE(p) &data_base[D2P(p)->custom.route.inst_no]

#else

EXTERN T_CC_DATA data_base;


#define GET_INSTANCE_DATA    register T_CC_DATA *cc_data= &data_base


#endif 

/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the entity name
 */
#define bc_prio_0 _ENTITY_PREFIXED(bc_prio_0)

EXTERN const UBYTE          bc_prio_0 [4];

#ifndef _TMS470
int sprintf (char *, const char *, ...); /* Use appropriate include ... */
#define Sprintf sprintf
#endif

#ifdef WIN32
  EXTERN UBYTE std;
#endif /* WIN32 */

EXTERN void cc_pei_primitive (T_PRIM * prim);
#endif

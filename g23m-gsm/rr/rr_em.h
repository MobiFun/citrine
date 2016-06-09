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
|  Purpose : Engineering Mode (EM) Declarations + Macros
|
+-----------------------------------------------------------------------------
*/
#ifndef RR_EM_H
#define RR_EM_H

#ifdef FF_EM_MODE


/* ------------ data declarations for EM ---------------- */

/*This value is used for function dat_em_nc_info_req to avoid a magic number*/

#define EM_MAX_NUM_NC             6

/*
 *  Buffer is by one bigger than max. number of prims, because it
 *  needs always one termination entry.
 *  To optimize the buffersize, the data is packed in the buffer as follows:
 *  type, length, value
 *  Value is the combination of the additional parameters as defined in 8443.601, coded as UBYTE.
 */
#define EM_RR_SEM_SIZE             220

/*
*   EM_MAX_RR_EVENTS defines maximum number of event traces for the engineering mode.
*   The number is increased by one to ensure that the event numbers defined in the
*   corresponding document are the same as in the sources.
*/
#define EM_MAX_RR_EVENTS            38

/*
*  The offset is used to indicate the source entity the event trace is from.
*  L1/ALR = 0x00, DL = 0x2D, RR = 0x37, MM = 0x5F, CC = 0x78, SS = 0xAF, SMS = 0xBE, SIM = E1
*/
#define RR_OFFSET                   0x37

/*
*  Definitions for the additional parameters used for the event tracing.
*/
#define EM_NORMAL                      1
#define EM_EXTENDED                    2
#define EM_REJECT                      3
#define EM_RECONNECT                   4
#define EM_TCH_CLOSE                   0
#define EM_TCH_OPEN                    1
/*
  #define EM_PLMN_SEARCH_STARTED         0
  #define EM_PLMN_SEARCH_FINISHED        1
  #define EM_SEARCH_STARTED              0
*/
#define EM_SEARCH_FAILED               1
#define EM_SEARCH_PASSED               2
#define EM_STATUS_RX                   0
#define EM_STATUS_TX                   1

/*
*  Type is combination of entity index(upper nibble) plus event number(lower nibble).
*  Bit 8  7  6  5  4  3  2  1
*      |    entity    |  event number |
*/

#define RR_V_1              (1 + RR_OFFSET)
#define RR_V_2              (2 + RR_OFFSET)
#define RR_V_3              (3 + RR_OFFSET)

#define RR_V_5              (5 + RR_OFFSET)
#define RR_V_6              (6 + RR_OFFSET)
#define RR_V_7              (7 + RR_OFFSET)
#define RR_V_8              (8 + RR_OFFSET)
#define RR_V_9              (9 + RR_OFFSET)
#define RR_V_10             (10+ RR_OFFSET)
#define RR_V_11             (11+ RR_OFFSET)
#define RR_V_12             (12+ RR_OFFSET)
#define RR_V_13             (13+ RR_OFFSET)
#define RR_V_14             (14+ RR_OFFSET)
#define RR_V_15             (15+ RR_OFFSET)
#define RR_V_16             (16+ RR_OFFSET)
#define RR_V_17             (17+ RR_OFFSET)
#define RR_V_18             (18+ RR_OFFSET)
#define RR_V_19             (19+ RR_OFFSET)
#define RR_V_20             (20+ RR_OFFSET)
#define RR_V_21             (21+ RR_OFFSET)
#define RR_V_22             (22+ RR_OFFSET)
#define RR_V_23             (23+ RR_OFFSET)
#define RR_V_24             (24+ RR_OFFSET)
#define RR_V_25             (25+ RR_OFFSET)
#define RR_V_26             (26+ RR_OFFSET)
#define RR_V_27             (27+ RR_OFFSET)
#define RR_V_28             (28+ RR_OFFSET)
#define RR_V_29             (29+ RR_OFFSET)
#define RR_V_30             (30+ RR_OFFSET)
#define RR_V_31             (31+ RR_OFFSET)
#define RR_V_32             (32+ RR_OFFSET)
#define RR_V_33             (33+ RR_OFFSET)
#define RR_V_34             (34+ RR_OFFSET)
#define RR_V_35             (35+ RR_OFFSET)
#define RR_V_36             (36+ RR_OFFSET)
#define RR_V_37             (37+ RR_OFFSET)

EXTERN BOOL rr_v[EM_MAX_RR_EVENTS];
EXTERN USHORT em_assign_fail_rr_cause;
EXTERN USHORT em_handover_fail_rr_cause;


/*#define TRACE_EVENT_EM(s)                       TRACE_EVENT(s)
#define TRACE_EVENT_EM_P1(s,a1)                 TRACE_EVENT_P1(s,a1)
#define TRACE_EVENT_EM_P2(s,a1,a2)              TRACE_EVENT_P2(s,a1,a2)
#define TRACE_EVENT_EM_P3(s,a1,a2,a3)           TRACE_EVENT_P3(s,a1,a2,a3)
#define TRACE_EVENT_EM_P4(s,a1,a2,a3,a4)        TRACE_EVENT_P4(s,a1,a2,a3,a4)
#define TRACE_EVENT_EM_P5(s,a1,a2,a3,a4,a5)     TRACE_EVENT_P5(s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_EM_P6(s,a1,a2,a3,a4,a5,a6)  TRACE_EVENT_P6(s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_EM_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)  TRACE_EVENT_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)*/
#define TRACE_EVENT_EM(s)                             TRACE_USER_CLASS   (TC_USER8,s)
#define TRACE_EVENT_EM_P1(s,a1)                       TRACE_USER_CLASS_P1(TC_USER8,s,a1)
#define TRACE_EVENT_EM_P2(s,a1,a2)                    TRACE_USER_CLASS_P2(TC_USER8,s,a1,a2)
#define TRACE_EVENT_EM_P3(s,a1,a2,a3)                 TRACE_USER_CLASS_P3(TC_USER8,s,a1,a2,a3)
#define TRACE_EVENT_EM_P4(s,a1,a2,a3,a4)              TRACE_USER_CLASS_P4(TC_USER8,s,a1,a2,a3,a4)
#define TRACE_EVENT_EM_P5(s,a1,a2,a3,a4,a5)           TRACE_USER_CLASS_P5(TC_USER8,s,a1,a2,a3,a4,a5)
#define TRACE_EVENT_EM_P6(s,a1,a2,a3,a4,a5,a6)        TRACE_USER_CLASS_P6(TC_USER8,s,a1,a2,a3,a4,a5,a6)
#define TRACE_EVENT_EM_P8(s,a1,a2,a3,a4,a5,a6,a7,a8)  TRACE_USER_CLASS_P8(TC_USER8,s,a1,a2,a3,a4,a5,a6,a7,a8)

/*---------Functions ---------*/

EXTERN       void dat_em_sc_info_req              (T_EM_SC_INFO_REQ           *em_sc_info_req);
EXTERN       void dat_em_nc_info_req              (T_EM_NC_INFO_REQ           *em_nc_info_req);
EXTERN       void dat_em_loc_pag_info_req         (T_EM_LOC_PAG_INFO_REQ      *em_loc_pag_info_req);
EXTERN       void dat_em_plmn_info_req            (T_EM_PLMN_INFO_REQ         *em_plmn_info_req);
EXTERN       void dat_em_cip_hop_dtx_info_req     (T_EM_CIP_HOP_DTX_INFO_REQ  *em_cip_hop_dtx_info_req);
EXTERN       void dat_em_mobdata_power_info_req   (T_EM_POWER_INFO_REQ        *em_power_info_req);
EXTERN       void dat_em_mobdata_id_info_req      (T_EM_IDENTITY_INFO_REQ     *em_identity_info_req);
EXTERN       void dat_em_mobdata_version_info_req (T_EM_SW_VERSION_INFO_REQ   *em_sw_version_info_req);
EXTERN       void dat_em_amr_info_req             (T_EM_AMR_INFO_REQ          *em_amr_info_req);
EXTERN       void dat_em_get_hchn (USHORT* channel_array, USHORT* channel2_array, UBYTE v_start, UBYTE maio2);
EXTERN       UBYTE em_get_first_codec (UBYTE acs);
EXTERN       void em_init_rr_event_trace  (void);
EXTERN       void rr_em_rr_event_req      (T_EM_RR_EVENT_REQ *em_rr_event_req);
EXTERN       void rr_em_pco_trace_req     (T_EM_PCO_TRACE_REQ *em_pco_trace_req);

#if !defined (WIN32)
EXTERN       CHAR* sim_version();
EXTERN       CHAR* sms_version();
EXTERN       CHAR* ss_version ();
EXTERN       CHAR* cc_version ();
EXTERN       CHAR* mm_version ();
EXTERN       CHAR* rr_version ();
EXTERN       CHAR* dl_version ();
EXTERN       CHAR* l1_version ();
#endif

/*---------FMM--------------*/
EXTERN       void dat_em_fmm_reselection_start_ind (void);
EXTERN       void dat_em_fmm_reselection_end_ind   (void);
EXTERN       void dat_em_fmm_sc_info_req  (T_EM_FMM_SC_INFO_REQ  *em_fmm_sc_info_req);
EXTERN       void dat_em_fmm_nc_info_req  (T_EM_FMM_NC_INFO_REQ  *em_fmm_nc_info_req);

/*
 * If all entities are linked into one module this definitions
 * prefixes all this functions with the enity name
 */
#ifdef OPTION_MULTITHREAD
  #define em_write_buffer_2        _ENTITY_PREFIXED(em_write_buffer_2)
  #define em_write_buffer_3        _ENTITY_PREFIXED(em_write_buffer_3)
  #define em_write_buffer_3a       _ENTITY_PREFIXED(em_write_buffer_3a)
  #define em_write_buffer_4        _ENTITY_PREFIXED(em_write_buffer_4)
  #define em_write_buffer_4a       _ENTITY_PREFIXED(em_write_buffer_4a)
  #define em_write_buffer_4b       _ENTITY_PREFIXED(em_write_buffer_4b)
  #define em_write_buffer_4c       _ENTITY_PREFIXED(em_write_buffer_4c)
  #define em_write_buffer_5        _ENTITY_PREFIXED(em_write_buffer_5)
  #define em_write_buffer_5c       _ENTITY_PREFIXED(em_write_buffer_5c)
  #define em_write_buffer_8        _ENTITY_PREFIXED(em_write_buffer_8)
  #define em_write_buffer_9        _ENTITY_PREFIXED(em_write_buffer_9)
#endif

EXTERN UBYTE em_write_buffer_2     (UBYTE event_no);
EXTERN UBYTE em_write_buffer_3     (UBYTE event_no, UBYTE value);
EXTERN UBYTE em_write_buffer_3a    (UBYTE event_no, USHORT value);
EXTERN UBYTE em_write_buffer_4     (UBYTE event_no, UBYTE value1, UBYTE value2);
EXTERN UBYTE em_write_buffer_4a    (UBYTE event_no, UBYTE value,  USHORT cs);
EXTERN UBYTE em_write_buffer_4b    (UBYTE event_no, UBYTE value,  T_plmn plmn);
EXTERN UBYTE em_write_buffer_4c    (UBYTE event_no, UBYTE value1, T_plmn plmn[MAX_PLMN]);
EXTERN UBYTE em_write_buffer_5     (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3);
EXTERN UBYTE em_write_buffer_5c    (UBYTE event_no, UBYTE value1, UBYTE value2, T_plmn plmn);
EXTERN UBYTE em_write_buffer_8     (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3, UBYTE value4,
                               UBYTE value5,   UBYTE value6);
EXTERN UBYTE em_write_buffer_9     (UBYTE event_no, UBYTE value1, UBYTE value2, UBYTE value3, UBYTE value4,
                               UBYTE value5,   UBYTE value6, UBYTE value7);

/*
 * Semaphore handling
 * called by ACI
 */
/*lint -esym(759,em_rr_sem)*/
/*lint -esym(759,em_rr_sem_read)*/
/*lint -esym(759,em_rr_sem_reset)*/
EXTERN UBYTE em_rr_sem           (UBYTE length, UBYTE * data);
EXTERN void  em_rr_sem_reset     (void);
EXTERN void  em_rr_sem_read      (void);
EXTERN void  em_rr_sem_init      (void);
EXTERN void  em_rr_sem_exit      (void);
EXTERN void  em_init_get_hchn    (void);


/*------Macro Definition---*/

#define RR_EM_SET_ASSIGN_FAIL_CAUSE(x)   (em_assign_fail_rr_cause = x)
#define RR_EM_SET_HANDOVER_FAIL_CAUSE(x) (em_handover_fail_rr_cause = x)
#define RR_EM_GET_HOPPING_CHANNEL(w,x,y,z)  dat_em_get_hchn(w,x,y,z)

/*-----------------------Event Macro Definition -----------*/

#define EM_PLMN_SRCH_STARTED\
  /* Search for service by MM - started */\
  if (rr_v[1]) {\
    rr_v[1] = em_write_buffer_3 (RR_V_1 , FULL_SERVICE);\
  }

#define EM_CELL_SELECTION\
  /* Search for full service by RR started */\
  if (rr_v[2]) {\
    rr_v[2] = em_write_buffer_2 (RR_V_2 );\
  }

#define EM_NET_SEARCH_FAILED\
    /* Search for service by MM/RR - failed */\
    if (rr_v[3] ) {\
      rr_v[3] = em_write_buffer_4 (RR_V_3 , EM_SEARCH_FAILED,\
                              abort_ind->op.service);\
    }

#define EM_PLMN_SRCH_PASSED\
  /* Search for service by MM/RR - passed */\
  if (rr_v[3] ) {\
    rr_v[3] = em_write_buffer_5c (RR_V_3 , EM_SEARCH_PASSED,\
                             rr_activate_cnf->op.service,\
                             rr_activate_cnf->plmn);\
  }

#define EM_HPLMN_SEARCH_PASSED\
/* Search for HPLMN passed */\
  if (rr_v[5])\
  {\
    T_plmn plmn_temp;\
    plmn_temp.v_plmn = TRUE;\
    memcpy(plmn_temp.mcc, rr_data->nc_data[CR_INDEX].lai.mcc,  SIZE_MCC);\
    memcpy(plmn_temp.mnc,  rr_data->nc_data[CR_INDEX].lai.mnc, SIZE_MNC);\
    rr_v[5] = em_write_buffer_4b (RR_V_5, EM_SEARCH_PASSED, plmn_temp);\
  }

#define EM_HPLMN_SEARCH_FAILED\
  /* Search for HPLMN failed */\
  if (rr_v[5]) {\
    rr_v[5] = em_write_buffer_3 (RR_V_5, EM_SEARCH_FAILED);\
  }

#define EM_NET_SEARCH_STARTED\
  /* Net search - started */\
      if (rr_v[6]) {\
        rr_v[6] = em_write_buffer_2 (RR_V_6 );\
      }

#define EM_NET_SEARCH_PASSED\
  /* Net search - passed */\
  if (rr_v[7]) {\
    rr_v[7] = em_write_buffer_4c (RR_V_7 , abort_ind->plmn_avail, abort_ind->plmn);\
  }

#define EM_IDLE_MODE\
  /* Idle */\
  if (rr_v[8]) {\
    rr_v[8] = em_write_buffer_4a (RR_V_8, rr_data->ms_data.rr_service,\
                             idle_req->arfcn);\
  }

#define EM_CELL_RESEL_STARTED\
  /* Cell Reselection started */\
  if (rr_v[9]) {\
    rr_v[9]  = em_write_buffer_3 (RR_V_9 , rr_data->sc_data.selection_type);\
  }

#define EM_CELL_RESEL_FINISHED\
  /* Cell reselection finished */\
  if (rr_v[10]) {\
      rr_v[10] = em_write_buffer_3a (RR_V_10, rr_data->nc_data[SC_INDEX].arfcn);\
  }

#define EM_PAGING_IND\
  /* Paging */\
  if (rr_v[11]) {\
    rr_v[11] = em_write_buffer_4 (RR_V_11, mph_paging_ind->identity_type,\
                                      mph_paging_ind->channel_needed);\
  }

#define EM_DOWNLINK_FAILURE\
  /* Downlink failure */\
  if (rr_v[12]) {\
    rr_v[12] = em_write_buffer_2 (RR_V_12);\
  }

#define EM_CHANNEL_REQUEST_SENT\
  /* Channel request*/\
  if (rr_v[13]) {\
     rr_v[13] = em_write_buffer_5 (RR_V_13, mph_random_access_cnf->frame_no.t1,\
                                       mph_random_access_cnf->frame_no.t2,\
                                       mph_random_access_cnf->frame_no.t3);\
  }

#define EM_IMMEDIATE_ASSIGNMENT\
  /* Immediate Assignment */\
  if (rr_v[14]) {\
    if (imm_assign->chan_desc.hop EQ H_NO)\
      rr_v[14] = em_write_buffer_9 (RR_V_14, EM_NORMAL,\
                               imm_assign->chan_desc.chan_type,\
                               imm_assign->chan_desc.tn,\
                               imm_assign->chan_desc.tsc,\
                               imm_assign->chan_desc.hop,\
                               (UBYTE)(imm_assign->chan_desc.arfcn>>8),\
                               (UBYTE)(imm_assign->chan_desc.arfcn));\
    else\
      rr_v[14] = em_write_buffer_9 (RR_V_14, EM_NORMAL,\
                               imm_assign->chan_desc.chan_type,\
                               imm_assign->chan_desc.tn,\
                               imm_assign->chan_desc.tsc,\
                               imm_assign->chan_desc.hop,\
                               imm_assign->chan_desc.maio,\
                               imm_assign->chan_desc.hsn);\
    }

#define EM_IMMEDIATE_ASSIGNMENT_EXT\
  /* Immediate Assignment */\
  if (rr_v[14]) {\
    if (p_chan_desc->hop EQ H_NO)\
      rr_v[14] = em_write_buffer_9 (RR_V_14, EM_EXTENDED,\
                               p_chan_desc->chan_type,p_chan_desc->tn,\
                               p_chan_desc->tsc,      p_chan_desc->hop,\
                               (UBYTE)(p_chan_desc->arfcn>>8),\
                               (UBYTE)(p_chan_desc->arfcn));\
    else\
      rr_v[14] = em_write_buffer_9 (RR_V_14, EM_EXTENDED,\
                               p_chan_desc->chan_type,p_chan_desc->tn,\
                               p_chan_desc->tsc,      p_chan_desc->hop,\
                               p_chan_desc->maio,     p_chan_desc->hsn);\
  }

#define EM_IMMEDIATE_ASSIGNMENT_REJECT\
  /* Immediate Assignment reject */\
 if (rr_v[15])\
   rr_v[15] = em_write_buffer_3 (RR_V_15, t3122);

#define EM_DL_ESTABLISH_CNF\
  /* Layer 2 connection establishment */\
  if (rr_v[16]) {\
    rr_v[16] = em_write_buffer_3 (RR_V_16, dl_establish_cnf->ch_type);\
  }

#define EM_L2_CONNECTION_ESTABLISHED\
  /* Layer 2 connection establishment */\
  if (rr_v[16]) {\
    rr_v[16] = em_write_buffer_3 (RR_V_16, dl_establish_cnf->ch_type);\
  }

#define EM_EARLY_CLASSMARK_SENDING\
   /* Early classmark sending */\
   if (rr_v[17]) {\
     rr_v[17] = em_write_buffer_2 (RR_V_17);\
   }

#define EM_CLASSMARK_ENQUIRY\
  /* Classmark interrogation */\
  if (rr_v[18])\
     rr_v[18] = em_write_buffer_2 (RR_V_18);

#define EM_ASSIGNMENT_RECEIVED\
  /* Assignment received */\
  if (rr_v[19]) {\
    if (dedicated_req->ch_type.h EQ H_NO)\
      rr_v[19] = em_write_buffer_8 (RR_V_19,\
                               dedicated_req->ch_type.ch,\
                               dedicated_req->ch_type.tn,\
                               dedicated_req->ch_type.tsc,\
                               dedicated_req->ch_type.h,\
                               (UBYTE)(dedicated_req->ch_type.arfcn>>8),\
                               (UBYTE)(dedicated_req->ch_type.arfcn));\
    else\
      rr_v[19] = em_write_buffer_8 (RR_V_19,\
                               dedicated_req->ch_type.ch,\
                               dedicated_req->ch_type.tn,\
                               dedicated_req->ch_type.tsc,\
                               dedicated_req->ch_type.h,\
                               dedicated_req->ch_type.maio,\
                               dedicated_req->ch_type.hsn);\
  }

#define EM_ASS_FAILURE_RECONNECT_SUCCESS\
  /* Assignment failure reconnect */\
  if (rr_v[20]) {\
    rr_v[20] = em_write_buffer_4a (RR_V_20, EM_RECONNECT, em_assign_fail_rr_cause);\
  }

#define EM_ASS_FAILURE_RECONNECT_FAILED2\
   /* Assignment failure reconnect */\
   if (rr_v[20]) {\
     rr_v[20] = em_write_buffer_4a (RR_V_20, EM_RECONNECT,\
                               em_assign_fail_rr_cause);\
   }

#define EM_ASS_FAILURE_RECONNECT_FAILED\
  /* Assignment failure, loss of call */\
  if (rr_v[21]) {\
  rr_v[21] = em_write_buffer_2 (RR_V_21);\
  }

#define EM_ASSIGNMENT_COMPLETE\
  /* Assignment complete */\
  if (rr_v[22]) {\
    rr_v[22] = em_write_buffer_2 (RR_V_22);\
  }

#define EM_HANDOVER_CMD\
  /* Handover */\
  if (rr_v[23]) {\
    if (handov_cmd->chan_desc_after.hop EQ H_NO)\
      rr_v[23] = em_write_buffer_9 (RR_V_23, rr_data->ms_data.ho_type.si,\
                               dedicated_req->ch_type.ch,\
                               dedicated_req->ch_type.tn,\
                               dedicated_req->ch_type.tsc,\
                               dedicated_req->ch_type.h,\
                               (UBYTE)(dedicated_req->ch_type.arfcn>>8),\
                               (UBYTE)(dedicated_req->ch_type.arfcn));\
    else\
      rr_v[23] = em_write_buffer_9 (RR_V_23, rr_data->ms_data.ho_type.si,\
                               dedicated_req->ch_type.ch,\
                               dedicated_req->ch_type.tn,\
                               dedicated_req->ch_type.tsc,\
                               dedicated_req->ch_type.h,\
                               dedicated_req->ch_type.maio,\
                               dedicated_req->ch_type.hsn);\
  }

#define EM_HO_FAILURE_RECONNECT_SUCCESS\
  /* Handover failure reconnect */\
  if (rr_v[24])  {\
     rr_v[24] = em_write_buffer_4a (RR_V_24, EM_RECONNECT,\
                               em_handover_fail_rr_cause);\
  }

#define EM_DL_RELEASE_IND\
  switch (GET_STATE (STATE_DAT))\
  {\
    case DAT_HANDOVER:\
    case DAT_HANDOVER_5:\
      /* Handover failure reject */\
      if (rr_v[24] ) {\
        rr_v[24] = em_write_buffer_4a (RR_V_24, EM_REJECT,\
                                  em_handover_fail_rr_cause);\
      }\
      break;\
    case DAT_CHAN_ASS:\
    case DAT_CHAN_ASS_2:\
      /* Assignment failure reject */\
      if (rr_v[20] ) {\
        rr_v[20] = em_write_buffer_4a (RR_V_20, EM_REJECT,\
                                  em_assign_fail_rr_cause);\
      }\
      break;\
    default:\
      break;\
  }



#define EM_HO_FAILURE_RECONNECT_FAILED2\
  /* Handover failure reconnect */\
  if (rr_v[24]) {\
    rr_v[24] = em_write_buffer_4a (RR_V_24, EM_RECONNECT,\
                              em_handover_fail_rr_cause);\
  }

#define EM_HO_FAILURE_RECONNECT_FAILED\
  /* Handover failure, loss of call */\
  if (rr_v[25])  {\
   rr_v[25] = em_write_buffer_2 (RR_V_25);\
  }  /* rr_v[25] */

#define EM_HANDOVER_COMPLETE\
  /* Handover complete */\
  if (rr_v[26]) {\
    rr_v[26] = em_write_buffer_2 (RR_V_26);\
  }

#define EM_FREQ_REDEF\
  /* Freq redefinition */\
  if (rr_v[27]) {\
     rr_v[27] = em_write_buffer_8 (RR_V_27, freq_redef_req->ch_type.ch,\
                              freq_redef_req->ch_type.tn,\
                              freq_redef_req->ch_type.tsc,\
                              freq_redef_req->ch_type.tsc,\
                              freq_redef_req->ch_type.maio,\
                              freq_redef_req->ch_type.hsn);\
  }

#define EM_CIPHERING_CMD\
  /* Cipher mode setting */\
  if (rr_v[28])\
    rr_v[28] = em_write_buffer_2 (RR_V_28);


#define EM_CHANNEL_MODE_MODIFY\
  /* Channel mode modify started */\
  if (rr_v[29]) {\
    rr_v[29] = em_write_buffer_3 (RR_V_29, chan_mod->chan_mode);\
  }

#define EM_CHANNEL_MODE_MODIFY_ACK\
  /* Channel mode modify result */\
  if (rr_v[30]) {\
    rr_v[30] = em_write_buffer_3 (RR_V_30, chan_mod_ack->chan_mode);\
  }

#define EM_CHANNEL_RELEASE\
  /* Channel release */\
  if (rr_v[31])\
    rr_v[31] = em_write_buffer_3a (RR_V_31, chan_rel->rr_cause);

#define EM_RADIO_LINK_FAILURE\
  /* Radio link failure */\
  if (rr_v[32]) {\
     rr_v[32] = em_write_buffer_2 (RR_V_32);\
  }

#define EM_L2_CONNECTION_LOST\
  /* Loss of layer 2 connection */\
  if (rr_v[33])\
 {\
    rr_v[33] = em_write_buffer_3a (RR_V_33, dl_release_ind->cs);\
  }

#define EM_RR_STATUS_SEND\
  /* Status sent */\
  if (rr_v[34]) {\
    rr_v[34] = em_write_buffer_4a (RR_V_34, EM_STATUS_TX, rr_status->rr_cause);\
  }

#define EM_RR_STATUS_RECEIVED\
  /* Status received */\
  if (rr_v[34])\
    rr_v[34] = em_write_buffer_3 (RR_V_34, EM_STATUS_RX);

#define EM_TCH_LOOP_CLOSED\
  /* TCH loop closed */\
  if (rr_v[35])\
    rr_v[35] = em_write_buffer_3 (RR_V_35, EM_TCH_CLOSE);

#define EM_TCH_LOOP_OPEN\
  /* TCH loop opened */\
  if (rr_v[35])\
    rr_v[35] = em_write_buffer_3 (RR_V_35, EM_TCH_OPEN);

#define EM_TEST_INTERFACE\
  /* Test interface */\
  if (rr_v[36])\
    rr_v[36] = em_write_buffer_2 (RR_V_36);

#define EM_DEACTIVATION\
  /* Deactivation */\
  if (rr_v[37]) {\
    rr_v[37] = em_write_buffer_2 (RR_V_37);\
  }

/*-------FMM------------*/
#ifdef GPRS

#define EM_FMM_RESEL_START_IND \
  /*inform FMM1251 that reselection has started*/\
  dat_em_fmm_reselection_start_ind()

 #define EM_FMM_RESEL_END_IND \
  /*inform fmmm1251 about successfull reselection*/\
  dat_em_fmm_reselection_end_ind()

#else /*no GPRS*/

 #define EM_FMM_RESEL_START_IND                /*for Microtec FMM1251 */
 #define EM_FMM_RESEL_END_IND                  /*for Microtec FMM1251 */

#endif /*GPRS*/

#else /*FF_EM_MODE not defined*/



#define RR_EM_SET_ASSIGN_FAIL_CAUSE(x)
#define RR_EM_SET_HANDOVER_FAIL_CAUSE(x)
#define RR_EM_GET_HOPPING_CHANNEL(w,x,y,z)
#define EM_PLMN_SRCH_STARTED                  /* Event 1 */
#define EM_CELL_SELECTION                     /* Event 2 */
#define EM_NET_SEARCH_FAILED                  /* Event 3 */
#define EM_PLMN_SRCH_PASSED                   /* Event 3 */
#define EM_HPLMN_SEARCH_STARTED               /* Event 4 */
#define EM_HPLMN_SEARCH_PASSED                /* Event 5 */
#define EM_HPLMN_SEARCH_FAILED                /* Event 5 */
#define EM_NET_SEARCH_STARTED                 /* Event 6 */
#define EM_NET_SEARCH_PASSED                  /* Event 7 */
#define EM_IDLE_MODE                          /* Event 8 */
#define EM_CELL_RESEL_STARTED                 /* Event 9*/
#define EM_CELL_RESEL_FINISHED                /* Event 10*/
#define EM_PAGING_IND                         /* Event 11*/
#define EM_DOWNLINK_FAILURE                   /* Event 12*/
#define EM_CHANNEL_REQUEST_SENT               /* Event 13*/
#define EM_IMMEDIATE_ASSIGNMENT               /* Event 14*/
#define EM_IMMEDIATE_ASSIGNMENT_EXT           /* Event 14*/
#define EM_IMMEDIATE_ASSIGNMENT_REJECT        /* Event 15*/
#define EM_DL_ESTABLISH_CNF                   /* Event 16*/
#define EM_L2_CONNECTION_ESTABLISHED          /* Event 16*/
#define EM_EARLY_CLASSMARK_SENDING            /* Event 17*/
#define EM_CLASSMARK_ENQUIRY                  /* Event 18*/
#define EM_ASSIGNMENT_RECEIVED                /* Event 19*/
#define EM_ASS_FAILURE_RECONNECT_SUCCESS      /* Event 20*/
#define EM_DL_RELEASE_IND                     /* Event 24 or Event 20 */
#define EM_ASS_FAILURE_RECONNECT_FAILED2      /* Event 20*/
#define EM_ASS_FAILURE_RECONNECT_FAILED       /* Event 21*/
#define EM_ASSIGNMENT_COMPLETE                /* Event 22*/
#define EM_HANDOVER_CMD                       /* Event 23*/
#define EM_HO_FAILURE_RECONNECT_SUCCESS       /* Event 24*/
#define EM_HO_FAILURE_RECONNECT_FAILED2       /* Event 24*/
#define EM_HO_FAILURE_RECONNECT_FAILED        /* Event 25*/
#define EM_HANDOVER_COMPLETE                  /* Event 26*/
#define EM_FREQ_REDEF                         /* Event 27*/
#define EM_CIPHERING_CMD                      /* Event 28*/
#define EM_CHANNEL_MODE_MODIFY                /* Event 29*/
#define EM_CHANNEL_MODE_MODIFY_ACK            /* Event 30*/
#define EM_CHANNEL_RELEASE                    /* Event 31*/
#define EM_RADIO_LINK_FAILURE                 /* Event 32*/
#define EM_L2_CONNECTION_LOST                 /* Event 33*/
#define EM_RR_STATUS_SEND                     /* Event 34*/
#define EM_RR_STATUS_RECEIVED                 /* Event 34*/
#define EM_TCH_LOOP_CLOSED                    /* Event 35*/
#define EM_TCH_LOOP_OPEN                      /* Event 36*/
#define EM_TEST_INTERFACE                     /* Event 37*/
#define EM_DEACTIVATION                       /* Event 38*/

#define EM_FMM_RESEL_START_IND                /*for Microtec FMM1251 */
#define EM_FMM_RESEL_END_IND                  /*for Microtec FMM1251 */

#endif /*FF_EM_MODE*/
#endif /* RR_EM_H */


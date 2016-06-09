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
|  Purpose :  Declarations for the Protocol Stack Entity
|             Radio Resource / GPRS-enhancements
+-----------------------------------------------------------------------------
*/

#ifndef RR_GPRS_H
#define RR_GPRS_H

#define EXT_MEAS_START_CR  0x40

/* intermediate storage for data of paging indication */
typedef struct
{
  UBYTE id_type;
  UBYTE chan_need;
} T_PAGING_DATA;

typedef enum
{
  START_PROC_ACTIVATE_REQ,
  START_PROC_ENTER_LIMITED,
  START_PROC_NORMAL_CR,
  START_PROC_NORMAL_CS,
  START_PROC_CFG_CHG,
  START_PROC_GPRS_ACT,
  START_PROC_NOTHING
} T_START_PROC;

typedef struct
{
  UBYTE                 rr_sdu[24];
  UBYTE                 si13_sdu[24];
  UBYTE                 tma_in_progress;
  ULONG                 fn;
  UBYTE                 req_ref_idx;
  UBYTE                 gprs_indic;        /* flag MM wants GPRS or not         */
  UBYTE                 si13_received;     /* SI13 has been received            */
  ULONG                 ptmsi;             /* used PTMSI                        */
  ULONG                 ptmsi2;            /* used candidate PTMSI                        */
  ULONG                 tlli;              /* used tlli                         */
  T_p_chan_req_des      p_chan_req_des;    /* Requested channel characteristics */
  T_gprs_meas_results   gprs_meas_results; /* GPRS Measurement Results          */
  UBYTE                 mac_req;           /*                                   */
  T_PAGING_DATA         pag_dat;
  UBYTE                 cs_req;            /*                                   */
  UBYTE                 gprs_suspend;
  UBYTE                 gprs_resump;
  UBYTE                 rac;
  UBYTE                 page_mode;
  USHORT                split_pg;          /* split paging cycle                */
  UBYTE                 use_c31;
  UBYTE                 cr_pbcch_active;
  UBYTE                 cr_type;
  UBYTE                 cr_orig;
  T_START_PROC          start_proc;
  UBYTE                 reconn_cause;      /* cause sent with RRGRR_RECONNECT_REQ during PDCH Assignment procedure */
  UBYTE                 tbf_est;           /* indicate TBF establishment during PDCH Assignment / Cell Change Order */
  T_DL_DATA_IND        *dl_data_ind;       /* store DL primitive containing d_change_order until access to new cell */
  UBYTE                 bsic;          /* BSIC of the new cell during Cell Change Order and CR*/
  USHORT                arfcn;         /* ARFCN of the new cell during Cell Change Order and CR */
  BOOL                  cco_need_reconnect_cnf; /* Flag indicating the need to send RRGRR-RECONNECT-CNF */
  T_rai                 current_rai;       /* last rai assigned by the network to GMM */
  T_add_freq_list       add_freq_lists[RRGRR_BA_LIST_SIZE];  
  UBYTE                 num_add_freq_list;
  T_RRGRR_EXT_MEAS_REQ *rrgrr_ext_meas_req;     /* save the request for GPRS Ext Measurement */
  T_RRGRR_EXT_MEAS_CNF *rrgrr_ext_meas_cnf;     /* compiled result of the GPRS Ext Measurement */
  T_MPH_EXT_MEAS_CNF   *mph_ext_meas_cnf;       /* power measurement result during GPRS Ext Measurement */
  UBYTE                 mph_ext_meas_num;       /* number of carriers used in mph_ext_meas_cnf */
  UBYTE                 ext_meas_ctrl;          /* indicate a pending Ext Meas stop request  */
  BOOL                  is_nc2_used_in_si13;    /* NC state of SI13: NC2 or !NC2 */
  UBYTE                 nc_mode_of_pmo;         /* NC mode of Packet Measurement Order */
  BOOL                  cr_pcco_active;         /* PCCO in BCCH environment, cell not synced */
  BOOL                  ready_state;            /* TRUE  - we are in READY   STATE 
                                                 * FALSE - we are in STANDBY STATE 
                                                 * This is actually a GMM state, but has to 
                                                 * stored here for the NC2 handling and for
                                                 * calculating the C2/C32 values 
                                                 */

#ifdef REL99
  BOOL                  cbch_psi_valid;         /* CBCH info on PSI8 received from GRR */
  T_cbch                cbch_psi8;              /* This field is in GPRS data as this cbch info
                                                 * on PSI 8 could be present only when PBCCH
                                                 * is enabled. 
                                                 */
  BOOL                  cbch_info_rxvd_in_ptm;  /* This flag is set to indicate that CBCH information was received in
                                                 * packet transfer mode. MPH_CBCH_COFIG_REQ should be sent to ALR 
                                                 * only in idle mode. When transitioning again to idle mode
                                                 * if this flag is set, then CBCH info is given to ALR.
                                                 * During packet transfer mode, CBCH info could be received
                                                 * in SI4 or PSI8 or may need updateion because of dependent 
                                                 * parameters change in in PSI2 or other SI messages received in PTM.
                                                 */
  UBYTE                 nw_release;
#endif
  UBYTE                 ba_bcch_modified; /*This flag will be set only when BA(BCCH) is modified by PMO or PCCO*/
} T_GPRS_DATA;

#ifdef OPTION_MULTITHREAD
  #define hCommGRR        _ENTITY_PREFIXED(hCommGRR)
#endif

EXTERN T_HANDLE  hCommGRR;         /* GRR  Communication        */

/*
 * GPRS functions
 */
void  gprs_init_gprs_data                  (void);
void  gprs_get_table_n                     (const T_FUNC**                table,
                                            USHORT*                       n);

/* GPRS support functions for process DATA */
BOOL  dat_check_gprs_imm_ass               (T_MPH_UNITDATA_IND*           unitdata,
                                            T_D_IMM_ASSIGN*               imm_assign,
                                            UBYTE                         index);
void  dat_check_imm_ass_ext                (T_MPH_UNITDATA_IND*           unitdata,
                                            UBYTE                         index);
UBYTE dat_check_imm_assign_pch             (T_MPH_UNITDATA_IND*           unitdata,
                                            T_D_IMM_ASSIGN*               imm_assign);
UBYTE dat_check_imm_ass_rej                (UBYTE                         wait_ind);
UBYTE dat_check_packet_paging_ind          (T_MPH_PAGING_IND*             pag_ind);
void  dat_rrgrr_channel_req                (T_RRGRR_CHANNEL_REQ*          chan_req);

void  att_check_bsic                       (T_RRGRR_NCELL_SYNC_REQ*       check_bsic);
BOOL  dat_gprs_start_sabm                  (void);
void  dat_rrgrr_suspend_dcch_req           (T_RRGRR_SUSPEND_DCCH_REQ*     suspend_dcch_req);
void  dat_rrgrr_suspend_dcch_cnf           (void);
void  dat_rrgrr_reconnect_dcch_req         (T_RRGRR_RECONNECT_DCCH_REQ*   reconnect_dcch_req);
void  dat_rrgrr_reconnect_dcch_cnf         (UBYTE                         reconn_state);
void  dat_rrgrr_resumed_tbf_req            (T_RRGRR_RESUMED_TBF_REQ*      resumed_tbf_req);
void  dat_rrgrr_resumed_tbf_cnf            (void);
void  dat_rrgrr_data_ind                   (T_DL_DATA_IND*                dl_data_ind);
void  dat_rrgrr_change_order               (T_DL_DATA_IND*                dl_data_ind,
                                            T_D_CHANGE_ORDER*             d_change_order);
void  dat_ask_paging_ind                   (T_MPH_PAGING_IND*             pag_ind);
void  dat_ask_paging_ind_pa_only           (T_MPH_PAGING_IND*             pag_ind);
void  dat_stop_dcch_ind                    (UBYTE                         stop_cause);
void  dat_rrgrr_data_req                   (T_RRGRR_DATA_REQ*             data_req);
void  dat_rrgrr_gprs_data_req              (T_RRGRR_GPRS_DATA_REQ*        data_req);
BOOL  dat_check_packet_access              (void);
void  dat_rrgrr_rr_est_req                 (T_RRGRR_RR_EST_REQ*           est_req);
void  dat_rrgrr_rr_est_rsp                 (T_RRGRR_RR_EST_RSP*           rr_est_rsp);
void  dat_rrgrr_activate_req               (T_RRGRR_ACTIVATE_REQ*         act_req);
void  dat_set_gprs_resump                  (T_RR_RELEASE_IND*             rr_release_ind);
void  dat_gprs_suspend_req                 (void);
void  dat_gprs_set_suspended               (void);
#ifdef REL99
BOOL  dat_gprs_cell_in_ptm                 (void);
#endif


/* GPRS support functions for process ATTACHEMENT */
void  att_signal_gprs_support              (void);
void  att_for_sysinfo_type13               (T_MPH_UNITDATA_IND*          data_ind,
                                            T_D_SYS_INFO_13*             sys_info_13);
void  att_set_gprs_indication              (UBYTE                        gprs_indic);
void  att_add_ptmsi                        (T_MPH_IDENTITY_REQ*          mph_identity_req);
void  att_rrgrr_cr_ind                     (UBYTE                        type);
void  att_rrgrr_cr_req                     (T_RRGRR_CR_REQ*              cr_req);
void  att_rrgrr_stop_mon_ccch_req          (T_RRGRR_STOP_MON_CCCH_REQ*   stop_ccch);
void  att_check_gprs_supp                  (UBYTE                        v_gprs_ind,
                                            T_gprs_indic*                data);
void  att_rrgrr_start_mon_ccch_req         (T_RRGRR_START_MON_CCCH_REQ*  start_ccch);
void  att_rrgrr_start_mon_bcch_req         (T_RRGRR_START_MON_BCCH_REQ*  start_bcch);
BOOL  att_check_sync_results               (T_MPH_MEASUREMENT_IND*       mph_measurement_ind);
BOOL  att_gprs_is_avail                    (void);
BOOL  att_gprs_cell_has_pbcch              (void);
void  att_gprs_stop_pl                     (void);
void  att_rrgrr_update_ba_req              (T_RRGRR_UPDATE_BA_REQ*       ba_req);
void  att_gprs_cr_rsp                      (T_RRGRR_CR_RSP*              cr_rsp);
void  att_rrgrr_ext_meas_req               (T_RRGRR_EXT_MEAS_REQ*        ext_meas_req);
UBYTE rr_ext_meas_idx                      (USHORT                       arfcn);
void  att_rrgrr_ext_meas_stop_req          (T_RRGRR_EXT_MEAS_STOP_REQ*   ext_meas_stop_req);
void  att_rrgrr_meas_rep_req               (T_RRGRR_MEAS_REP_REQ*        s);
void  att_gprs_idle_req                    (T_MPH_IDLE_REQ*              idle_req);
BOOL  att_gprs_check_ncell                 (void);
void  att_start_cell_reselection_pbcch     (UBYTE mode);
void  att_cell_reselection_gprs_failed     (void);
/*XXX*/
void  gprs_rrgrr_stop_task                 (T_RRGRR_STOP_TASK_REQ*       stop_task);

void  gprs_rrgrr_fill_from_stored_sdu      (T_sdu*                       to,
                                            UBYTE*                       from);
void  gprs_rrgrr_store_sdu                 (UBYTE*                       to,
                                            T_sdu*                       from);
void  att_gprs_start_task                  (T_RRGRR_START_TASK_REQ*      start_task);
void  att_start_cell_selection_gprs        (UBYTE                        originator,U8 search_mode);
void  att_start_cell_reselection_gprs      (UBYTE                        mode);
void  gprs_init_data_cr                    (void);
void  att_convert_idle_c31_cr              (UBYTE                        index);
void  att_insert_c31_cr_data_in_cell       (UBYTE                        index);
BOOL  att_check_cell_c31                   (void);
void  att_check_c31_reselect_decision      (UBYTE                        start_now);
BOOL  att_check_c31_criterion              (UBYTE                        index);
UBYTE att_get_next_best_c32_index          (BOOL                         c31_calculated);
void  att_calculate_c31_c32                (UBYTE                        index);
void  att_rrgrr_standby_ind                (T_RRGRR_STANDBY_STATE_IND*   stdby);
void  att_rrgrr_ready_ind                  (T_RRGRR_READY_STATE_IND*     rdy);
#ifdef REL99
void  att_rrgrr_cbch_info_ind              (T_RRGRR_CBCH_INFO_IND*       cbch_info);
#endif
void att_gprs_sync_req( T_RRGRR_SYNC_REQ   *sync_req);
BOOL  is_nc2_used                          (void);
#ifdef REL99
UBYTE att_gprs_get_nw_release              (void);
#endif

/*
 * States of the GPRS Process
 * PIM   - Packet Idle Mode
 * PTM   - Packet Transfer Mode
 * PAM   - Packet Access Mode
 */
#define GPRS_NULL                  0 /* GPRS is not activated by MM */
#define GPRS_ACTIVATED             1 /* GPRS is activated by MM
                                      * and we are not on a cell */
#define GPRS_PIM_BCCH              2 /* RR has camped on a GPRS cell which has no PBCCH
                                      * RR is in idle mode
                                      */
#define GPRS_PAM_BCCH              3 /* GRR has requested the establishment of a TBF over CCCH */
#define GPRS_PTM_BCCH              4 /* GRR has established the TBF and transfer data */
#define GPRS_DEDI_SDCCH            5 /* GRR has requested a TBF but was assigned a SDCCH for further
                                      * signalling */
#define GPRS_PIM_PBCCH             6 /* GRR has camped on a GPRS cell which has a PBCCH
                                      * RR and GRR are in idle mode */
#define GPRS_PAM_PBCCH             7 /* GRR is establishing a TBF */
#define GPRS_PTM_PBCCH             8 /* GRR has established a TBF and transfers data */
#define GPRS_SUSPENDED_BCCH        9 /* RR is in or on its way to dedicated mode and was in a GPRS_*_BCCH state */
#define GPRS_SUSPENDED_PBCCH      10 /* RR is in or on its way to dedicated mode and was in a GPRS_*_PBCCH state */

EXTERN  const char * const
        STATE_GPRS_NAME[];
#define STATE_GPRS_NAME_INIT \
       "GPRS_NULL",          \
       "GPRS_ACTIVATED",     \
       "GPRS_PIM_BCCH",      \
       "GPRS_PAM_BCCH",      \
       "GPRS_PTM_BCCH",      \
       "GPRS_DEDI_SDCCH",    \
       "GPRS_PIM_PBCCH",     \
       "GPRS_PAM_PBCCH",     \
       "GPRS_PTM_PBCCH",     \
       "GPRS_SUSPENDED_BCCH",\
       "GPRS_SUSPENDED_PBCCH"


#define ESTCS_GPRS_1P     0x0478
#define ESTCS_GPRS_SB     0x0470
#define ESTCS_GPRS_PAGING 0x0481

#define TBF_EST_NONE      0
#define TBF_EST_PDCH      1
#define TBF_EST_CCO       2

#ifdef REL99
#define RR_GPRS_R97       0
#define RR_GPRS_R99       1
#define RR_GPRS_R4        2
#endif

#endif /* !RR_GPRS_H */

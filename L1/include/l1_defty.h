/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_DEFTY.H
 *
 *        Filename l1_defty.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/
#if(L1_DYN_DSP_DWNLD == 1)
  #include "../dyn_dwl_include/l1_dyn_dwl_defty.h"
#endif

typedef struct
{
  UWORD16  modulus;
  UWORD16  relative_position;
}
T_BCCHS_SCHEDULE;

typedef struct
{
  UWORD8            schedule_array_size;
  T_BCCHS_SCHEDULE  schedule_array[10];
}
T_BCCHS;

typedef struct
{
  BOOL        status;
  UWORD16     radio_freq;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      sb26_attempt;
  UWORD8      tsc;
  UWORD16     bcch_blks_req;
  UWORD8      timing_validity;
  UWORD8      search_mode;
  UWORD8      gprs_priority;
  UWORD8      sb26_offset; // Set to 1 when SB26 RX win is entirely in frame 25.
#if (L1_12NEIGH ==1)
  UWORD32     fn_offset_mem;
  UWORD32     time_alignmt_mem;
#endif
}
T_NCELL_SINGLE;

typedef struct
{
  UWORD8          active_neigh_id_norm;
  UWORD8          active_neigh_tc_norm;
  UWORD8          active_neigh_id_top;
  UWORD8          active_neigh_tc_top;
  UWORD8          current_list_size;
  T_NCELL_SINGLE  list[6];
}
T_BCCHN_LIST;

typedef struct
{
  UWORD8          active_fb_id;
  UWORD8          active_sbconf_id;
  UWORD8          active_sb_id;
  UWORD8          current_list_size;
  UWORD8          first_in_list;  //point at oldest element in list. Used when parsing the list.
#if (L1_EOTD==1)
  #if L1_EOTD_QBIT_ACC
    // Store serving fn_offset and time_alignmt, so that they can be tracked
    // independently.
    UWORD32  serv_fn_offset;
    UWORD32       serv_time_alignmt;
  #endif
  // Need to track any TOA updates in dedicated mode else
  // QB errors are introduced in the results...

  UWORD8          eotd_toa_phase;
  WORD32          eotd_toa_tracking;
  WORD32          eotd_cache_toa_tracking;

  UWORD8          eotd_meas_session;
  UWORD32         fn_sb_serv;       // for methods 1 & 2
  UWORD32         ta_sb_serv;       // for methods 1 & 2
  WORD32          teotdS;           // for method 2 only
  UWORD32         fn_offset_serv;   // for method 2 only
#endif
#if (L1_12NEIGH==1)
  T_NCELL_SINGLE  list[NBR_NEIGHBOURS+1]; // 1 place (13th) for S.C in EOTD.
#else
  T_NCELL_SINGLE  list[6];
#endif
}
T_NSYNC_LIST;

typedef struct
{
  UWORD8   cbch_state;
  UWORD32  starting_fn;
  UWORD32  first_block[48];
  UWORD8   cbch_num;
  UWORD8   schedule_length;
  UWORD8   next;
  WORD32   start_continuous_fn;
}
T_CBCH_HEAD_SCHEDULE;

typedef struct
{
  UWORD8   cbch_num;
  UWORD8   next;
  UWORD32  start_fn[6];
}
T_CBCH_INFO_SCHEDULE;

/*=========================================================================*/
/* Moved type definitions from Debis files.                                */
/*=========================================================================*/
#if (AMR == 1)
  // AMR ver 1.0 parameters
  typedef struct
  {
    BOOL    noise_suppression_bit;
    BOOL    initial_codec_mode_indicator;
    UWORD8  initial_codec_mode;
    UWORD8  active_codec_set;
    UWORD8  threshold[3];
    UWORD8  hysteresis[3];
  }
  T_AMR_CONFIGURATION;
#endif

typedef struct
{
  UWORD8  A[7+1];
}
T_ENCRYPTION_KEY;

typedef struct
{
  UWORD8  A[22+1];
}
T_RADIO_FRAME;

typedef struct
{
  UWORD8  n32;
  UWORD8  n51;
  UWORD8  n26;
}
T_SHORT_FRAME_NUMBER;

typedef struct
{
  UWORD16  A[31+1];
}
T_CHAN_LIST;

typedef struct
{
  UWORD16      num_of_chans;
  T_CHAN_LIST  chan_number;
}
T_BCCH_LIST;

typedef struct
{
  UWORD16        rf_chan_num;
  UWORD8         l2_channel_type;
  UWORD8         error_cause;
  T_RADIO_FRAME  l2_frame;
  UWORD8         bsic;
  UWORD8         tc;
}
T_PH_DATA_IND;

typedef struct
{
  UWORD16  A[63+1];
}
T_MA_FIELD;

typedef struct
{
  UWORD16     rf_chan_cnt;
  T_MA_FIELD  rf_chan_no;
}
T_MOBILE_ALLOCATION;

typedef struct
{
  BOOL                  start_time_present;
  T_SHORT_FRAME_NUMBER  start_time;
}
T_STARTING_TIME;

typedef struct
{
  UWORD16  radio_freq_no;
  WORD8   rxlev;
}
T_RXLEV_MEAS;

typedef struct
{
  UWORD8  maio;
  UWORD8  hsn;
}
T_HOPPING_RF;

typedef struct
{
  UWORD16  radio_freq;
}
T_SINGLE_RF;

typedef union
{
  T_SINGLE_RF   single_rf;
  T_HOPPING_RF  hopping_rf;
}
T_CHN_SEL_CHOICE;

typedef struct
{
  BOOL              h;
  T_CHN_SEL_CHOICE  rf_channel;
}
T_CHN_SEL;

typedef struct
{
    T_CHN_SEL  chan_sel;
    UWORD8     channel_type;
    UWORD8     subchannel;
    UWORD8     timeslot_no;
    UWORD8     tsc;
}
T_CHANNEL_DESCRIPTION;

typedef struct
{
  UWORD8   ncc;
  UWORD8   bcc;
  UWORD16  bcch_carrier;
}
T_CELL_DESC;

typedef struct
{
  T_CELL_DESC            cell_description;
  T_CHANNEL_DESCRIPTION  channel_desc_1;
  UWORD8                 channel_mode_1;
  T_STARTING_TIME        starting_time;
  UWORD8                 ho_acc;
  UWORD8                 txpwr;
  BOOL                   report_time_diff;
  T_MOBILE_ALLOCATION    frequency_list;
  T_CHANNEL_DESCRIPTION  channel_desc_2;
  UWORD8                 channel_mode_2;
  T_MOBILE_ALLOCATION    frequency_list_bef_sti;
  T_CHANNEL_DESCRIPTION  channel_desc_1_bef_sti;
  T_CHANNEL_DESCRIPTION  channel_desc_2_bef_sti;
  BOOL                   cipher_mode;
  UWORD8                 a5_algorithm;
}
T_HO_PARAMS;

typedef struct
{
    T_CHANNEL_DESCRIPTION  channel_desc;
    T_MOBILE_ALLOCATION    frequency_list;
    T_STARTING_TIME        starting_time;
}
T_MPHC_CHANGE_FREQUENCY;

typedef struct
{
  UWORD8  subchannel;
  UWORD8  channel_mode;
  #if (AMR == 1)
    T_AMR_CONFIGURATION  amr_configuration;
  #endif
}
T_MPHC_CHANNEL_MODE_MODIFY_REQ;

typedef struct
{
  UWORD8            cipher_mode;
  UWORD8            a5_algorithm;
  T_ENCRYPTION_KEY  new_ciph_param;
}
T_MPHC_SET_CIPHERING_REQ;

typedef struct
{
  UWORD8  sub_channel;
  UWORD8  frame_erasure;
}
T_OML1_CLOSE_TCH_LOOP_REQ;

typedef struct
{
  #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
    T_RV_HDR  header;
  #endif
  UWORD8  tested_device;
}
T_OML1_START_DAI_TEST_REQ;

/***********************************************************/
/* Type definitions for DEBUG...                           */
/***********************************************************/
typedef struct                  // translate string in int and int in string
{
  CHAR *message;
  WORD32  SignalCode;
  WORD32  size;
}
MSG_DEBUG;

typedef struct                  // translate string in int and int in string
{
  CHAR *name;
}
TASK_TRACE;

/***********************************************************/
/* Type definitions for data structures used for MFTAB     */
/* managment...                                            */
/***********************************************************/
typedef struct
{
  void   (*fct_ptr)(UWORD8,UWORD8);
  CHAR   param1;
  CHAR   param2;
}
T_FCT;

typedef struct
{
  T_FCT  fct[L1_MAX_FCT];
}
T_FRM;

typedef struct
{
  T_FRM  frmlst[MFTAB_SIZE];
}
T_MFTAB;

typedef struct
{
  const T_FCT    *address;
  UWORD8  size;
}
T_TASK_MFTAB;

/***********************************************************/
/* TPU controle register components definition.            */
/***********************************************************/

#if (CODE_VERSION==SIMULATION)
  typedef struct                      // contents of REG_CMD register
  {
    unsigned int  tpu_reset_bit : 1;  // TPU_RESET bit : ON (reset TPU)
    unsigned int  tpu_pag_bit   : 1;  // TPU_PAG   bit : 0  (page 0)
    unsigned int  tpu_enb_bit   : 1;  // TPU_ENB   bit : ON (TPU commun.int.)
    unsigned int  dsp_pag_bit   : 1;  // DSP_PAG   bit : 0  (page 0)
    unsigned int  dsp_enb_bit   : 1;  // DSP_ENB   bit : ON (DSP commun.int.)
    unsigned int  tpu_stat_bit  : 1;  // TPU_STAT  bit : ON (if TPU active) OFF (if TPU in IDLE)
    unsigned int  tpu_idle_bit  : 1;  // TPU_IDLE  bit : ON (force IDLE mode)
  }
  T_reg_cmd;  // Rem: we must keep "unsigned int" type for bitmap.
#else
  typedef struct                      // contents of REG_CMD register
  {
    unsigned int  tpu_reset_bit : 1;  // TPU_RESET bit : ON (reset TPU)
    unsigned int  tpu_pag_bit   : 1;  // TPU_PAG   bit : 0  (page 0)
    unsigned int  tpu_enb_bit   : 1;  // TPU_ENB   bit : ON (TPU commun.int.)
    unsigned int  unused_1      : 1;  //
    unsigned int  dsp_enb_bit   : 1;  // DSP_ENB   bit : ON (DSP commun.int.)
    unsigned int  unused_2      : 1;  //
    unsigned int  unused_3      : 1;  //
    unsigned int  tsp_reset_bit : 1;  // TSP_RESET bit : ON (reset TSP)
    unsigned int  tpu_idle_bit  : 1;  // TPU_IDLE  bit : ON (force IDLE mode)
    unsigned int  tup_wait_bit  : 1;  // TPU_WAIT  bit : ON (TPU ready)
    unsigned int  tpu_ck_enb_bit: 1;  // TPU_CLK   bit : ON (TPU clock on)
  }
    T_reg_cmd;
#endif
/***********************************************************/
/*                                                         */
/*  Data structure for global info components.             */
/*                                                         */
/***********************************************************/

typedef struct
{
  API d_task_d;           // (0)  Downlink task command.
  API d_burst_d;          // (1)  Downlink burst identifier.
  API d_task_u;           // (2)  Uplink task command.
  API d_burst_u;          // (3)  Uplink burst identifier.
  API d_task_md;          // (4)  Downlink Monitoring (FB/SB) command.
#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
  API d_background;       // (5) Background tasks
#else
  API d_reserved;         // (5)  Reserved
#endif
  API d_debug;            // (6)  Debug/Acknowledge/general purpose word.
  API d_task_ra;          // (7)  RA task command.
  API d_fn;               // (8)  FN, in Rep. period and FN%104, used for TRAFFIC/TCH only.
                             //        bit [0..7]  -> b_fn_report, FN in the normalized reporting period.
                             //        bit [8..15] -> b_fn_sid,    FN % 104, used for SID positionning.
  API d_ctrl_tch;         // (9)  Tch channel description.
                             //        bit [0..3]  -> b_chan_mode,    channel  mode.
                             //        bit [4..5]  -> b_chan_type,    channel type.
                             //        bit [6]     -> reset SACCH
                             //        bit [7]     -> vocoder ON
                             //        bit [8]     -> b_sync_tch_ul,  synchro. TCH/UL.
                             //        bit [9]     -> b_sync_tch_dl,  synchro. TCH/DL.
                             //        bit [10]    -> b_stop_tch_ul,  stop TCH/UL.
                             //        bit [11]    -> b_stop_tch_dl,  stop TCH/DL.
                             //        bit [12.13] -> b_tch_loop,     tch loops A/B/C.
  API hole;               // (10) unused hole.

#if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
  API d_ctrl_abb;         // (11) Bit field indicating the analog baseband register to send.
                             //        bit [0]     -> b_ramp: the ramp information(a_ramp[]) is located in NDB
                             //        bit [1.2]   -> unused
                             //        bit [3]     -> b_apcdel: delays-register in NDB
                             //        bit [4]     -> b_afc: freq control register in DB
                             //        bit [5..15] -> unused
#endif
  API a_a5fn[2];          // (12..13) Encryption Frame number.
                             //        word 0, bit [0..4]  -> T2.
                             //        word 0, bit [5..10] -> T3.
                             //        word 1, bit [0..11] -> T1.
  API d_power_ctl;        // (14) Power level control.
  API d_afc;              // (15) AFC value (enabled by "b_afc" in "d_ctrl_TCM4400 or in d_ctrl_abb").
  API d_ctrl_system;      // (16) Controle Register for RESET/RESUME.
                             //        bit [0..2] -> b_tsq,           training sequence.
                             //        bit [3]    -> b_bcch_freq_ind, BCCH frequency indication.
                             //        bit [15]   -> b_task_abort,    DSP task abort command.
}
T_DB_MCU_TO_DSP;

typedef struct
{
  API d_task_d;           // (0) Downlink task command.
  API d_burst_d;          // (1) Downlink burst identifier.
  API d_task_u;           // (2) Uplink task command.
  API d_burst_u;          // (3) Uplink burst identifier.
  API d_task_md;          // (4) Downlink Monitoring (FB/SB) task command.
#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
  API d_background;       // (5) Background tasks
#else
  API d_reserved;         // (5)  Reserved
#endif
  API d_debug;            // (6) Debug/Acknowledge/general purpose word.
  API d_task_ra;          // (7) RA task command.

#if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)
  API a_serv_demod[4];    // ( 8..11) Serv. cell demod. result, array of 4 words (D_TOA,D_PM,D_ANGLE,D_SNR).
  API a_pm[3];            // (12..14) Power measurement results, array of 3 words.
  API a_sch[5];           // (15..19) Header + SB information, array of  5 words.
#else
  API a_pm[3];            // ( 8..10) Power measurement results, array of 3 words.
  API a_serv_demod[4];    // (11..14) Serv. cell demod. result, array of 4 words (D_TOA,D_PM,D_ANGLE,D_SNR).
  API a_sch[5];           // (15..19) Header + SB information, array of  5 words.
#endif
}
T_DB_DSP_TO_MCU;

#if (DSP == 34) || (DSP == 35) || (DSP == 36) // NDB GSM
  typedef struct
  {
    // MISC Tasks
    API d_dsp_page;

    // DSP status returned (DSP --> MCU).
    API d_error_status;

    // RIF control (MCU -> DSP).
    API d_spcx_rif;

    API d_tch_mode;  // TCH mode register.
                     // bit [0..1]  -> b_dai_mode.
                     // bit [2]     -> b_dtx.

    API d_debug1;                // bit 0 at 1 enable dsp f_tx delay for Omega

    API d_dsp_test;

    // Words dedicated to Software version (DSP code + Patch)
    API d_version_number1;
    API d_version_number2;

    API d_debug_ptr;
    API d_debug_bk;

    API d_pll_config;

    // GSM/GPRS DSP Debug trace support
    API p_debug_buffer;
    API d_debug_buffer_size;
    API d_debug_trace_type;

    #if (W_A_DSP_IDLE3 == 1)
      // DSP report its state: 0 run, 1 Idle1, 2 Idle2, 3 Idle3.
      API d_dsp_state;
      // 5 words are reserved for any possible mapping modification
      API d_hole1_ndb[2];
    #else
      // 6 words are reserved for any possible mapping modification
      API d_hole1_ndb[3];
    #endif

    #if (AMR == 1)
      API p_debug_amr;
    #else
      API d_hole_debug_amr;
    #endif

    #if (CHIPSET == 12)
      #if (DSP == 35) || (DSP == 36)
        API d_hole2_ndb[1];
        API d_mcsi_select;
      #else
        API d_hole2_ndb[2];
      #endif
    #else
      API d_hole2_ndb[2];
    #endif

    // New words APCDEL1 and APCDEL2 for 2TX: TX/PRACH combinations
    API d_apcdel1_bis;
    API d_apcdel2_bis;


    // New registers due to IOTA analog base band
    API d_apcdel2;
    API d_vbctrl2;
    API d_bulgcal;

    // Analog Based Band
    API d_afcctladd;

    API d_vbuctrl;
    API d_vbdctrl;
    API d_apcdel1;
    API d_apcoff;
    API d_bulioff;
    API d_bulqoff;
    API d_dai_onoff;
    API d_auxdac;

  #if (ANALOG == 1)
    API d_vbctrl;
  #elif ((ANALOG == 2) || (ANALOG == 3))
    API d_vbctrl1;
  #endif
  
    API d_bbctrl;

    // Monitoring tasks control (MCU <- DSP)
    // FB task
    API d_fb_det;           // FB detection result. (1 for FOUND).
    API d_fb_mode;          // Mode for FB detection algorithm.
    API a_sync_demod[4];    // FB/SB demod. result, (D_TOA,D_PM,D_ANGLE,D_SNR).

    // SB Task
    API a_sch26[5];         // Header + SB information, array of  5 words.

    API d_audio_gain_ul;
    API d_audio_gain_dl;

    // Controller of the melody E2 audio compressor
    API d_audio_compressor_ctrl;

    // AUDIO module
    API d_audio_init;
    API d_audio_status;

    // Audio tasks
    // TONES (MCU -> DSP)
    API d_toneskb_init;
    API d_toneskb_status;
    API d_k_x1_t0;
    API d_k_x1_t1;
    API d_k_x1_t2;
    API d_pe_rep;
    API d_pe_off;
    API d_se_off;
    API d_bu_off;
    API d_t0_on;
    API d_t0_off;
    API d_t1_on;
    API d_t1_off;
    API d_t2_on;
    API d_t2_off;
    API d_k_x1_kt0;
    API d_k_x1_kt1;
    API d_dur_kb;
    API d_shiftdl;
    API d_shiftul;

    API d_aec_ctrl;

    API d_es_level_api;
    API d_mu_api;

    // Melody Ringer module
    API d_melo_osc_used;
    API d_melo_osc_active;
    API a_melo_note0[4];
    API a_melo_note1[4];
    API a_melo_note2[4];
    API a_melo_note3[4];
    API a_melo_note4[4];
    API a_melo_note5[4];
    API a_melo_note6[4];
    API a_melo_note7[4];

    // selection of the melody format
    API d_melody_selection;

    // Holes due to the format melody E1
    API a_melo_holes[3];

    // Speech Recognition module
    API d_sr_status;          // status of the DSP speech reco task
    API d_sr_param;           // paramters for the DSP speech reco task: OOV threshold.
    API d_sr_bit_exact_test;  // bit exact test
    API d_sr_nb_words;        // number of words used in the speech recognition task
    API d_sr_db_level;        // estimate voice level in dB
    API d_sr_db_noise;        // estimate noise in dB
    API d_sr_mod_size;        // size of the model
    API a_n_best_words[4];  // array of the 4 best words
    API a_n_best_score[8];  // array of the 4 best scores (each score is 32 bits length)

    // Audio buffer
    API a_dd_1[22];         // Header + DATA traffic downlink information, sub. chan. 1.
    API a_du_1[22];         // Header + DATA traffic uplink information, sub. chan. 1.

    // V42bis module
    API d_v42b_nego0;
    API d_v42b_nego1;
    API d_v42b_control;
    API d_v42b_ratio_ind;
    API d_mcu_control;
    API d_mcu_control_sema;

    // Background tasks
    API d_background_enable;
    API d_background_abort;
    API d_background_state;
    API d_max_background;
    API a_background_tasks[16];
    API a_back_task_io[16];

    // GEA module defined in l1p_deft.h (the following section is overlaid with GPRS NDB memory)
    API d_gea_mode_ovly;
    API a_gea_kc_ovly[4];

#if (ANALOG == 3)
    // SYREN specific registers
    API d_vbpop;
    API d_vau_delay_init;
    API d_vaud_cfg;
    API d_vauo_onoff;
    API d_vaus_vol;
    API d_vaud_pll;
    API d_hole3_ndb[1];
#elif ((ANALOG == 1) || (ANALOG == 2))

    API d_hole3_ndb[7];

#endif

    // word used for the init of USF threshold
    API d_thr_usf_detect;

    // Encryption module
    API d_a5mode;           // Encryption Mode.

    API d_sched_mode_gprs_ovly;

    // 7 words are reserved for any possible mapping modification
    API d_hole4_ndb[5];

    // Ramp definition for Omega device
    API a_ramp[16];

    // CCCH/SACCH downlink information...(!!)
    API a_cd[15];           // Header + CCCH/SACCH downlink information.

    // FACCH downlink information........(!!)
    API a_fd[15];           // Header + FACCH downlink information.

    // Traffic downlink data frames......(!!)
    API a_dd_0[22];         // Header + DATA traffic downlink information, sub. chan. 0.

    // CCCH/SACCH uplink information.....(!!)
    API a_cu[15];           // Header + CCCH/SACCH uplink information.

    // FACCH downlink information........(!!)
    API a_fu[15];           // Header + FACCH uplink information

    // Traffic downlink data frames......(!!)
    API a_du_0[22];         // Header + DATA traffic uplink information, sub. chan. 0.

    // Random access.....................(MCU -> DSP).
    API d_rach;             // RACH information.

    //...................................(MCU -> DSP).
    API a_kc[4];            // Encryption Key Code.

    // Integrated Data Services module
    API d_ra_conf;
    API d_ra_act;
    API d_ra_test;
    API d_ra_statu;
    API d_ra_statd;
    API d_fax;
    API a_data_buf_ul[21];
    API a_data_buf_dl[37];

  // GTT API mapping for DSP code 34 (for test only)
  #if (L1_GTT == 1)
    API d_tty_status;
    API d_tty_detect_thres;
    API d_ctm_detect_shift;
    API d_tty_fa_thres;
    API d_tty_mod_norm;
    API d_tty_reset_buffer_ul;
    API d_tty_loop_ctrl;
    API p_tty_loop_buffer;
  #else
    API a_tty_holes[8];
  #endif

    API a_sr_holes0[414];

  #if (L1_NEW_AEC)
    // new AEC
    API d_cont_filter;
    API d_granularity_att;
    API d_coef_smooth;
    API d_es_level_max;
    API d_fact_vad;
    API d_thrs_abs;
    API d_fact_asd_fil;
    API d_fact_asd_mut;
    API d_far_end_pow_h;
    API d_far_end_pow_l;
    API d_far_end_noise_h;
    API d_far_end_noise_l;
  #else
    API a_new_aec_holes[12];
  #endif // L1_NEW_AEC

    // Speech recognition model
    API a_sr_holes1[145];
    API d_cport_init;
    API d_cport_ctrl;
    API a_cport_cfr[2];
    API d_cport_tcl_tadt;
    API d_cport_tdat;
    API d_cport_tvs;
    API d_cport_status;
    API d_cport_reg_value;

    API a_cport_holes[1011];

    API a_model[1041];

    // EOTD buffer
#if (L1_EOTD==1)
    API d_eotd_first;
    API d_eotd_max;
    API d_eotd_nrj_high;
    API d_eotd_nrj_low;
    API a_eotd_crosscor[18];
#else
    API a_eotd_holes[22];
#endif
    // AMR ver 1.0 buffers
    API a_amr_config[4];
    API a_ratscch_ul[6];
    API a_ratscch_dl[6];
    API d_amr_snr_est; // estimation of the SNR of the AMR speech block
  #if (L1_VOICE_MEMO_AMR)
    API d_amms_ul_voc;
  #else
    API a_voice_memo_amr_holes[1];
  #endif
    API d_thr_onset_afs;     // thresh detection ONSET AFS
    API d_thr_sid_first_afs; // thresh detection SID_FIRST AFS
    API d_thr_ratscch_afs;   // thresh detection RATSCCH AFS
    API d_thr_update_afs;    // thresh detection SID_UPDATE AFS
    API d_thr_onset_ahs;     // thresh detection ONSET AHS
    API d_thr_sid_ahs;       // thresh detection SID frames AHS
    API d_thr_ratscch_marker;// thresh detection RATSCCH MARKER
    API d_thr_sp_dgr;   // thresh detection SPEECH DEGRADED/NO_DATA
    API d_thr_soft_bits;   
    #if (MELODY_E2)
      API d_melody_e2_osc_stop;
      API d_melody_e2_osc_active;
      API d_melody_e2_semaphore;
      API a_melody_e2_osc[16][3];
      API d_melody_e2_globaltimefactor;
      API a_melody_e2_instrument_ptr[8];
      API d_melody_e2_deltatime;

      #if (AMR_THRESHOLDS_WORKAROUND)
        API a_d_macc_thr_afs[8];
        API a_d_macc_thr_ahs[6];
      #else
        API a_melody_e2_holes0[14];
      #endif

      API a_melody_e2_holes1[693];
      API a_dsp_trace[SC_AUDIO_MELODY_E2_MAX_SIZE_OF_DSP_TRACE];
      API a_melody_e2_instrument_wave[SC_AUDIO_MELODY_E2_MAX_SIZE_OF_INSTRUMENT];
    #else
      API d_holes[61];
      #if (AMR_THRESHOLDS_WORKAROUND)
        API a_d_macc_thr_afs[8];
        API a_d_macc_thr_ahs[6];
      #endif
    #endif

  }
  T_NDB_MCU_DSP;
#elif (DSP == 33) // NDB GSM
  typedef struct
  {
    // MISC Tasks
    API d_dsp_page;

    // DSP status returned (DSP --> MCU).
    API d_error_status;

    // RIF control (MCU -> DSP).
    API d_spcx_rif;

    API d_tch_mode;  // TCH mode register.
                     // bit [0..1]  -> b_dai_mode.
                     // bit [2]     -> b_dtx.

    API d_debug1;                // bit 0 at 1 enable dsp f_tx delay for Omega

    API d_dsp_test;

    // Words dedicated to Software version (DSP code + Patch)
    API d_version_number1;
    API d_version_number2;

    API d_debug_ptr;
    API d_debug_bk;

    API d_pll_config;

    // GSM/GPRS DSP Debug trace support
    API p_debug_buffer;
    API d_debug_buffer_size;
    API d_debug_trace_type;

    #if (W_A_DSP_IDLE3 == 1)
      // DSP report its state: 0 run, 1 Idle1, 2 Idle2, 3 Idle3.
      API d_dsp_state;
      // 10 words are reserved for any possible mapping modification
      API d_hole1_ndb[5];
    #else
      // 11 words are reserved for any possible mapping modification
      API d_hole1_ndb[6];
    #endif

    // New words APCDEL1 and APCDEL2 for 2TX: TX/PRACH combinations
    API d_apcdel1_bis;
    API d_apcdel2_bis;


    // New registers due to IOTA analog base band
    API d_apcdel2;
    API d_vbctrl2;
    API d_bulgcal;

    // Analog Based Band
    API d_afcctladd;

    API d_vbuctrl;
    API d_vbdctrl;
    API d_apcdel1;
    API d_apcoff;
    API d_bulioff;
    API d_bulqoff;
    API d_dai_onoff;
    API d_auxdac;

  #if (ANALOG == 1)
    API d_vbctrl;
  #elif ((ANALOG == 2) || (ANALOG == 3))
    API d_vbctrl1;
  #endif

    API d_bbctrl;

    // Monitoring tasks control (MCU <- DSP)
    // FB task
    API d_fb_det;           // FB detection result. (1 for FOUND).
    API d_fb_mode;          // Mode for FB detection algorithm.
    API a_sync_demod[4];    // FB/SB demod. result, (D_TOA,D_PM,D_ANGLE,D_SNR).

    // SB Task
    API a_sch26[5];         // Header + SB information, array of  5 words.

    API d_audio_gain_ul;
    API d_audio_gain_dl;

    // Controller of the melody E2 audio compressor
    API d_audio_compressor_ctrl;

    // AUDIO module
    API d_audio_init;
    API d_audio_status;

    // Audio tasks
    // TONES (MCU -> DSP)
    API d_toneskb_init;
    API d_toneskb_status;
    API d_k_x1_t0;
    API d_k_x1_t1;
    API d_k_x1_t2;
    API d_pe_rep;
    API d_pe_off;
    API d_se_off;
    API d_bu_off;
    API d_t0_on;
    API d_t0_off;
    API d_t1_on;
    API d_t1_off;
    API d_t2_on;
    API d_t2_off;
    API d_k_x1_kt0;
    API d_k_x1_kt1;
    API d_dur_kb;
    API d_shiftdl;
    API d_shiftul;

    API d_aec_ctrl;

    API d_es_level_api;
    API d_mu_api;

    // Melody Ringer module
    API d_melo_osc_used;
    API d_melo_osc_active;
    API a_melo_note0[4];
    API a_melo_note1[4];
    API a_melo_note2[4];
    API a_melo_note3[4];
    API a_melo_note4[4];
    API a_melo_note5[4];
    API a_melo_note6[4];
    API a_melo_note7[4];

    // selection of the melody format
    API d_melody_selection;

    // Holes due to the format melody E1
    API a_melo_holes[3];

    // Speech Recognition module
    API d_sr_status;          // status of the DSP speech reco task
    API d_sr_param;           // paramters for the DSP speech reco task: OOV threshold.
    API d_sr_bit_exact_test;  // bit exact test
    API d_sr_nb_words;        // number of words used in the speech recognition task
    API d_sr_db_level;        // estimate voice level in dB
    API d_sr_db_noise;        // estimate noise in dB
    API d_sr_mod_size;        // size of the model
    API a_n_best_words[4];  // array of the 4 best words
    API a_n_best_score[8];  // array of the 4 best scores (each score is 32 bits length)

    // Audio buffer
    API a_dd_1[22];         // Header + DATA traffic downlink information, sub. chan. 1.
    API a_du_1[22];         // Header + DATA traffic uplink information, sub. chan. 1.

    // V42bis module
    API d_v42b_nego0;
    API d_v42b_nego1;
    API d_v42b_control;
    API d_v42b_ratio_ind;
    API d_mcu_control;
    API d_mcu_control_sema;

    // Background tasks
    API d_background_enable;
    API d_background_abort;
    API d_background_state;
    API d_max_background;
    API a_background_tasks[16];
    API a_back_task_io[16];

    // GEA module defined in l1p_deft.h (the following section is overlaid with GPRS NDB memory)
    API d_gea_mode_ovly;
    API a_gea_kc_ovly[4];

    API d_hole3_ndb[8];

    // Encryption module
    API d_a5mode;           // Encryption Mode.

    API d_sched_mode_gprs_ovly;

    // 7 words are reserved for any possible mapping modification
    API d_hole4_ndb[5];

    // Ramp definition for Omega device
    API a_ramp[16];

    // CCCH/SACCH downlink information...(!!)
    API a_cd[15];           // Header + CCCH/SACCH downlink information.

    // FACCH downlink information........(!!)
    API a_fd[15];           // Header + FACCH downlink information.

    // Traffic downlink data frames......(!!)
    API a_dd_0[22];         // Header + DATA traffic downlink information, sub. chan. 0.

    // CCCH/SACCH uplink information.....(!!)
    API a_cu[15];           // Header + CCCH/SACCH uplink information.

    // FACCH downlink information........(!!)
    API a_fu[15];           // Header + FACCH uplink information

    // Traffic downlink data frames......(!!)
    API a_du_0[22];         // Header + DATA traffic uplink information, sub. chan. 0.

    // Random access.....................(MCU -> DSP).
    API d_rach;             // RACH information.

    //...................................(MCU -> DSP).
    API a_kc[4];            // Encryption Key Code.

    // Integrated Data Services module
    API d_ra_conf;
    API d_ra_act;
    API d_ra_test;
    API d_ra_statu;
    API d_ra_statd;
    API d_fax;
    API a_data_buf_ul[21];
    API a_data_buf_dl[37];

  #if (L1_NEW_AEC)
    // new AEC
    API a_new_aec_holes[422];
    API d_cont_filter;
    API d_granularity_att;
    API d_coef_smooth;
    API d_es_level_max;
    API d_fact_vad;
    API d_thrs_abs;
    API d_fact_asd_fil;
    API d_fact_asd_mut;
    API d_far_end_pow_h;
    API d_far_end_pow_l;
    API d_far_end_noise_h;
    API d_far_end_noise_l;
  #endif

  // Speech recognition model
  #if (L1_NEW_AEC)
    API a_sr_holes[1165];
  #else
    API a_sr_holes[1599];
  #endif // L1_NEW_AEC
    API a_model[1041];

    // EOTD buffer
    #if (L1_EOTD==1)
      API d_eotd_first;
      API d_eotd_max;
      API d_eotd_nrj_high;
      API d_eotd_nrj_low;
      API a_eotd_crosscor[18];
    #else
      API a_eotd_holes[22];
    #endif

    #if (MELODY_E2)
      API a_melody_e2_holes0[27];
      API d_melody_e2_osc_used;
      API d_melody_e2_osc_active;
      API d_melody_e2_semaphore;
      API a_melody_e2_osc[16][3];
      API d_melody_e2_globaltimefactor;
      API a_melody_e2_instrument_ptr[8];
      API a_melody_e2_holes1[708];
      API a_dsp_trace[SC_AUDIO_MELODY_E2_MAX_SIZE_OF_DSP_TRACE];
      API a_melody_e2_instrument_wave[SC_AUDIO_MELODY_E2_MAX_SIZE_OF_INSTRUMENT];
    #endif
  }
  T_NDB_MCU_DSP;

#elif ((DSP == 32) || (DSP == 31))
  typedef struct
  {
    // Monitoring tasks control..........(MCU <- DSP)
    API d_fb_det;           // FB detection result. (1 for FOUND).
    API d_fb_mode;          // Mode for FB detection algorithm.
    API a_sync_demod[4];    // FB/SB demod. result, (D_TOA,D_PM,D_ANGLE,D_SNR).

    // CCCH/SACCH downlink information...(!!)
    API a_cd[15];           // Header + CCCH/SACCH downlink information.

    // FACCH downlink information........(!!)
    API a_fd[15];           // Header + FACCH downlink information.

    // Traffic downlink data frames......(!!)
    API a_dd_0[22];         // Header + DATA traffic downlink information, sub. chan. 0.
    API a_dd_1[22];         // Header + DATA traffic downlink information, sub. chan. 1.

    // CCCH/SACCH uplink information.....(!!)
    API a_cu[15];           // Header + CCCH/SACCH uplink information.

    #if (SPEECH_RECO)
      // FACCH downlink information........(!!)
      API a_fu[3];              // Header + FACCH uplink information
                                // The size of this buffer is 15 word but some speech reco words
                                // are overlayer with this buffer. This is the reason why the size is 3 instead of 15.
      API d_sr_status;          // status of the DSP speech reco task
      API d_sr_param;           // paramters for the DSP speech reco task: OOV threshold.
      API sr_hole1;             // hole
      API d_sr_bit_exact_test;  // bit exact test
      API d_sr_nb_words;        // number of words used in the speech recognition task
      API d_sr_db_level;        // estimate voice level in dB
      API d_sr_db_noise;        // estimate noise in dB
      API d_sr_mod_size;        // size of the model
      API sr_holes_1[4];        // hole
    #else
      // FACCH downlink information........(!!)
      API a_fu[15];           // Header + FACCH uplink information
    #endif

    // Traffic uplink data frames........(!!)
    API a_du_0[22];         // Header + DATA traffic uplink information, sub. chan. 0.
    API a_du_1[22];         // Header + DATA traffic uplink information, sub. chan. 1.

    // Random access.....................(MCU -> DSP).
    API d_rach;             // RACH information.

    //...................................(MCU -> DSP).
    API d_a5mode;           // Encryption Mode.
    API a_kc[4];            // Encryption Key Code.
    API d_tch_mode;         // TCH mode register.
                            //   bit [0..1]  -> b_dai_mode.
                            //   bit [2]     -> b_dtx.

  // OMEGA...........................(MCU -> DSP).
  #if ((ANALOG == 1) || (ANALOG == 2))
    API a_ramp[16];
    #if (MELODY_E1)
      API d_melo_osc_used;
      API d_melo_osc_active;
      API a_melo_note0[4];
      API a_melo_note1[4];
      API a_melo_note2[4];
      API a_melo_note3[4];
      API a_melo_note4[4];
      API a_melo_note5[4];
      API a_melo_note6[4];
      API a_melo_note7[4];
      #if (DSP==31)
        // selection of the melody format
        API d_melody_selection;
        API holes[9];
      #else // DSP==32
        API d_dco_type;               // Tide
        API p_start_IQ;
        API d_level_off;
        API d_dco_dbg;
        API d_tide_resa;
        API d_asynch_margin;          // Perseus Asynch Audio Workaround
        API hole[4];
      #endif // DSP 32

    #else // NO MELODY E1
      #if (DSP==31)
      // selection of the melody format
      API d_melody_selection;
      API holes[43];               // 43 unused holes.
      #else // DSP==32
        API holes[34];               // 34 unused holes.
        API d_dco_type;               // Tide
        API p_start_IQ;
        API d_level_off;
        API d_dco_dbg;
        API d_tide_resa;
        API d_asynch_margin;          // Perseus Asynch Audio Workaround
        API hole[4];
      #endif //DSP == 32
    #endif // NO MELODY E1

    API d_debug3;
    API d_debug2;
    API d_debug1;                // bit 0 at 1 enable dsp f_tx delay for Omega
    API d_afcctladd;
    API d_vbuctrl;
    API d_vbdctrl;
    API d_apcdel1;
    API d_aec_ctrl;
    API d_apcoff;
    API d_bulioff;
    API d_bulqoff;
    API d_dai_onoff;
    API d_auxdac;

    #if (ANALOG == 1)
      API d_vbctrl;
    #elif (ANALOG == 2)
      API d_vbctrl1;
    #endif

    API d_bbctrl;
  #else 
    #error DSPCODE not supported with given ANALOG
  #endif //(ANALOG)1, 2
    //...................................(MCU -> DSP).
    API a_sch26[5];         // Header + SB information, array of  5 words.

    // TONES.............................(MCU -> DSP)
    API d_toneskb_init;
    API d_toneskb_status;
    API d_k_x1_t0;
    API d_k_x1_t1;
    API d_k_x1_t2;
    API d_pe_rep;
    API d_pe_off;
    API d_se_off;
    API d_bu_off;
    API d_t0_on;
    API d_t0_off;
    API d_t1_on;
    API d_t1_off;
    API d_t2_on;
    API d_t2_off;
    API d_k_x1_kt0;
    API d_k_x1_kt1;
    API d_dur_kb;

    // PLL...............................(MCU -> DSP).
    API d_pll_clkmod1;
    API d_pll_clkmod2;

    // DSP status returned..........(DSP --> MCU).
    API d_error_status;

    // RIF control.......................(MCU -> DSP).
    API d_spcx_rif;

    API d_shiftdl;
    API d_shiftul;

    API p_saec_prog;
    API p_aec_prog;
    API p_spenh_prog;

    API a_ovly[75];
    API d_ra_conf;
    API d_ra_act;
    API d_ra_test;
    API d_ra_statu;
    API d_ra_statd;
    API d_fax;
    #if (SPEECH_RECO)
      API a_data_buf_ul[3];
      API a_n_best_words[4];  // array of the 4 best words
      API a_n_best_score[8];  // array of the 4 best scores (each score is 32 bits length)
      API sr_holes_2[6];
      API a_data_buf_dl[37];

      API a_hole[24];

      API d_sched_mode_gprs_ovly;

      API fir_holes1[384];
      API a_fir31_uplink[31];
      API a_fir31_downlink[31];
      API d_audio_init;
      API d_audio_status;

      API a_model[1041];  // array of the speech reco model
    #else
      API a_data_buf_ul[21];
      API a_data_buf_dl[37];

      API a_hole[24];

      API d_sched_mode_gprs_ovly;

      API fir_holes1[384];
      API a_fir31_uplink[31];
      API a_fir31_downlink[31];
      API d_audio_init;
      API d_audio_status;

#if (L1_EOTD ==1)
      API a_eotd_hole[369];

      API d_eotd_first;
      API d_eotd_max;
      API d_eotd_nrj_high;
      API d_eotd_nrj_low;
      API a_eotd_crosscor[18];
#endif
    #endif
  }
  T_NDB_MCU_DSP;


#else // OTHER DSP CODE like 17

typedef struct
{
  // Monitoring tasks control..........(MCU <- DSP)
  API d_fb_det;           // FB detection result. (1 for FOUND).
  API d_fb_mode;          // Mode for FB detection algorithm.
  API a_sync_demod[4];    // FB/SB demod. result, (D_TOA,D_PM,D_ANGLE,D_SNR).

  // CCCH/SACCH downlink information...(!!)
  API a_cd[15];           // Header + CCCH/SACCH downlink information.

  // FACCH downlink information........(!!)
  API a_fd[15];           // Header + FACCH downlink information.

  // Traffic downlink data frames......(!!)
  #if (DATA14_4 == 0)
    API a_dd_0[20];         // Header + DATA traffic downlink information, sub. chan. 0.
    API a_dd_1[20];         // Header + DATA traffic downlink information, sub. chan. 1.
  #endif
  #if (DATA14_4 == 1)
    API a_dd_0[22];         // Header + DATA traffic downlink information, sub. chan. 0.
    API a_dd_1[22];         // Header + DATA traffic downlink information, sub. chan. 1.
  #endif

  // CCCH/SACCH uplink information.....(!!)
  API a_cu[15];           // Header + CCCH/SACCH uplink information.

  #if (SPEECH_RECO)
    // FACCH downlink information........(!!)
    API a_fu[3];              // Header + FACCH uplink information
                              // The size of this buffer is 15 word but some speech reco words
                              // are overlayer with this buffer. This is the reason why the size is 3 instead of 15.
    API d_sr_status;          // status of the DSP speech reco task
    API d_sr_param;           // paramters for the DSP speech reco task: OOV threshold.
    API sr_hole1;             // hole
    API d_sr_bit_exact_test;  // bit exact test
    API d_sr_nb_words;        // number of words used in the speech recognition task
    API d_sr_db_level;        // estimate voice level in dB
    API d_sr_db_noise;        // estimate noise in dB
    API d_sr_mod_size;        // size of the model
    API sr_holes_1[4];        // hole
  #else
    // FACCH downlink information........(!!)
    API a_fu[15];           // Header + FACCH uplink information
  #endif

  // Traffic uplink data frames........(!!)
  #if (DATA14_4 == 0)
    API a_du_0[20];         // Header + DATA traffic uplink information, sub. chan. 0.
    API a_du_1[20];         // Header + DATA traffic uplink information, sub. chan. 1.
  #endif
  #if (DATA14_4 == 1)
    API a_du_0[22];         // Header + DATA traffic uplink information, sub. chan. 0.
    API a_du_1[22];         // Header + DATA traffic uplink information, sub. chan. 1.
  #endif

  // Random access.....................(MCU -> DSP).
  API d_rach;             // RACH information.

  //...................................(MCU -> DSP).
  API d_a5mode;           // Encryption Mode.
  API a_kc[4];            // Encryption Key Code.
  API d_tch_mode;         // TCH mode register.
                               //        bit [0..1]  -> b_dai_mode.
                               //        bit [2]     -> b_dtx.

  // OMEGA...........................(MCU -> DSP).

#if ((ANALOG == 1) || (ANALOG == 2))
  API a_ramp[16];
  #if (MELODY_E1)
    API d_melo_osc_used;
    API d_melo_osc_active;
    API a_melo_note0[4];
    API a_melo_note1[4];
    API a_melo_note2[4];
    API a_melo_note3[4];
    API a_melo_note4[4];
    API a_melo_note5[4];
    API a_melo_note6[4];
    API a_melo_note7[4];
    #if (DSP == 17)
    // selection of the melody format
      API d_dco_type;               // Tide
      API p_start_IQ;
      API d_level_off;
      API d_dco_dbg;
      API d_tide_resa;
      API d_asynch_margin;          // Perseus Asynch Audio Workaround
      API hole[4];
    #else
      API d_melody_selection;
      API holes[9];
    #endif
  #else // NO MELODY E1
    // selection of the melody format
    #if (DSP == 17)
      API holes[34];               // 34 unused holes.
      API d_dco_type;               // Tide
      API p_start_IQ;
      API d_level_off;
      API d_dco_dbg;
      API d_tide_resa;
      API d_asynch_margin;          // Perseus Asynch Audio Workaround
      API hole[4]
    #else
      // selection of the melody format
      API d_melody_selection;
      API holes[43];               // 43 unused holes.
    #endif
  #endif
  API d_debug3;
  API d_debug2;
  API d_debug1;                // bit 0 at 1 enable dsp f_tx delay for Omega
  API d_afcctladd;
  API d_vbuctrl;
  API d_vbdctrl;
  API d_apcdel1;
  API d_aec_ctrl;
  API d_apcoff;
  API d_bulioff;
  API d_bulqoff;
  API d_dai_onoff;
  API d_auxdac;
  #if (ANALOG == 1)
    API d_vbctrl;
  #elif (ANALOG == 2)
    API d_vbctrl1;
  #endif
  API d_bbctrl;

  #else 
    #error DSPCODE not supported with given ANALOG
  #endif //(ANALOG)1, 2
  //...................................(MCU -> DSP).
  API a_sch26[5];         // Header + SB information, array of  5 words.

  // TONES.............................(MCU -> DSP)
  API d_toneskb_init;
  API d_toneskb_status;
  API d_k_x1_t0;
  API d_k_x1_t1;
  API d_k_x1_t2;
  API d_pe_rep;
  API d_pe_off;
  API d_se_off;
  API d_bu_off;
  API d_t0_on;
  API d_t0_off;
  API d_t1_on;
  API d_t1_off;
  API d_t2_on;
  API d_t2_off;
  API d_k_x1_kt0;
  API d_k_x1_kt1;
  API d_dur_kb;

  // PLL...............................(MCU -> DSP).
  API d_pll_clkmod1;
  API d_pll_clkmod2;

  // DSP status returned..........(DSP --> MCU).
  API d_error_status;

  // RIF control.......................(MCU -> DSP).
  API d_spcx_rif;

  API d_shiftdl;
  API d_shiftul;

  #if (AEC == 1)
    // AEC control.......................(MCU -> DSP).
    #if (VOC == FR_EFR)
      API p_aec_init;
      API p_aec_prog;
      API p_spenh_init;
      API p_spenh_prog;
    #endif

    #if (VOC == FR_HR_EFR)
      API p_saec_prog;
      API p_aec_prog;
      API p_spenh_prog;
    #endif
  #endif

    API a_ovly[75];
    API d_ra_conf;
    API d_ra_act;
    API d_ra_test;
    API d_ra_statu;
    API d_ra_statd;
    API d_fax;
    #if (SPEECH_RECO)
      API a_data_buf_ul[3];
      API a_n_best_words[4];  // array of the 4 best words
      API a_n_best_score[8];  // array of the 4 best scores (each score is 32 bits length)
      API sr_holes_2[6];
      API a_data_buf_dl[37];

      API fir_holes1[409];
      API a_fir31_uplink[31];
      API a_fir31_downlink[31];
      API d_audio_init;
      API d_audio_status;
      API a_model[1041];  // array of the speech reco model
    #else
      API a_data_buf_ul[21];
      API a_data_buf_dl[37];

      API fir_holes1[409];
      API a_fir31_uplink[31];
      API a_fir31_downlink[31];
      API d_audio_init;
      API d_audio_status;
    #endif
}
T_NDB_MCU_DSP;
#endif

#if (DSP == 34) || (DSP == 35) || (DSP == 36)
typedef struct
{
  API_SIGNED d_transfer_rate;

  // Common GSM/GPRS
  // These words specified the latencies to applies on some peripherics
  API_SIGNED d_lat_mcu_bridge;
  API_SIGNED d_lat_mcu_hom2sam;
  API_SIGNED d_lat_mcu_bef_fast_access;
  API_SIGNED d_lat_dsp_after_sam;

  // DSP Start address
  API_SIGNED d_gprs_install_address;

  API_SIGNED d_misc_config;

  API_SIGNED d_cn_sw_workaround;

  API_SIGNED d_hole2_param[4];

    //...................................Frequency Burst.
  API_SIGNED d_fb_margin_beg;
  API_SIGNED d_fb_margin_end;
  API_SIGNED d_nsubb_idle;
  API_SIGNED d_nsubb_dedic;
  API_SIGNED d_fb_thr_det_iacq;
  API_SIGNED d_fb_thr_det_track;
    //...................................Demodulation.
  API_SIGNED d_dc_off_thres;
  API_SIGNED d_dummy_thres;
  API_SIGNED d_dem_pond_gewl;
  API_SIGNED d_dem_pond_red;

    //...................................TCH Full Speech.
  API_SIGNED d_maccthresh1;
  API_SIGNED d_mldt;
  API_SIGNED d_maccthresh;
  API_SIGNED d_gu;
  API_SIGNED d_go;
  API_SIGNED d_attmax;
  API_SIGNED d_sm;
  API_SIGNED d_b;

  // V42Bis module
  API_SIGNED d_v42b_switch_hyst;
  API_SIGNED d_v42b_switch_min;
  API_SIGNED d_v42b_switch_max;
  API_SIGNED d_v42b_reset_delay;

  //...................................TCH Half Speech.
  API_SIGNED d_ldT_hr;
  API_SIGNED d_maccthresh_hr;
  API_SIGNED d_maccthresh1_hr;
  API_SIGNED d_gu_hr;
  API_SIGNED d_go_hr;
  API_SIGNED d_b_hr;
  API_SIGNED d_sm_hr;
  API_SIGNED d_attmax_hr;

  //...................................TCH Enhanced FR Speech.
  API_SIGNED c_mldt_efr;
  API_SIGNED c_maccthresh_efr;
  API_SIGNED c_maccthresh1_efr;
  API_SIGNED c_gu_efr;
  API_SIGNED c_go_efr;
  API_SIGNED c_b_efr;
  API_SIGNED c_sm_efr;
  API_SIGNED c_attmax_efr;

  //...................................CHED
  API_SIGNED d_sd_min_thr_tchfs;
  API_SIGNED d_ma_min_thr_tchfs;
  API_SIGNED d_md_max_thr_tchfs;
  API_SIGNED d_md1_max_thr_tchfs;

  API_SIGNED d_sd_min_thr_tchhs;
  API_SIGNED d_ma_min_thr_tchhs;
  API_SIGNED d_sd_av_thr_tchhs;
  API_SIGNED d_md_max_thr_tchhs;
  API_SIGNED d_md1_max_thr_tchhs;

  API_SIGNED d_sd_min_thr_tchefs;
  API_SIGNED d_ma_min_thr_tchefs;
  API_SIGNED d_md_max_thr_tchefs;
  API_SIGNED d_md1_max_thr_tchefs;

  API_SIGNED d_wed_fil_ini;
  API_SIGNED d_wed_fil_tc;
  API_SIGNED d_x_min;
  API_SIGNED d_x_max;
  API_SIGNED d_slope;
  API_SIGNED d_y_min;
  API_SIGNED d_y_max;
  API_SIGNED d_wed_diff_threshold;
  API_SIGNED d_mabfi_min_thr_tchhs;

  // FACCH module
  API_SIGNED d_facch_thr;

  // IDS module
  API_SIGNED d_max_ovsp_ul;
  API_SIGNED d_sync_thres;
  API_SIGNED d_idle_thres;
  API_SIGNED d_m1_thres;
  API_SIGNED d_max_ovsp_dl;
  API_SIGNED d_gsm_bgd_mgt;

  // FIR coefficients
  API a_fir_holes[4];
  API a_fir31_uplink[31];
  API a_fir31_downlink[31];
}
T_PARAM_MCU_DSP;
#elif (DSP == 33)
typedef struct
{
  API_SIGNED d_transfer_rate;

  // Common GSM/GPRS
  // These words specified the latencies to applies on some peripherics
  API_SIGNED d_lat_mcu_bridge;
  API_SIGNED d_lat_mcu_hom2sam;
  API_SIGNED d_lat_mcu_bef_fast_access;
  API_SIGNED d_lat_dsp_after_sam;

  // DSP Start address
  API_SIGNED d_gprs_install_address;

  API_SIGNED d_misc_config;

  API_SIGNED d_cn_sw_workaround;

  #if DCO_ALGO
    API_SIGNED d_cn_dco_param;

    API_SIGNED d_hole2_param[3];
  #else
    API_SIGNED d_hole2_param[4];
  #endif

    //...................................Frequency Burst.
  API_SIGNED d_fb_margin_beg;
  API_SIGNED d_fb_margin_end;
  API_SIGNED d_nsubb_idle;
  API_SIGNED d_nsubb_dedic;
  API_SIGNED d_fb_thr_det_iacq;
  API_SIGNED d_fb_thr_det_track;
    //...................................Demodulation.
  API_SIGNED d_dc_off_thres;
  API_SIGNED d_dummy_thres;
  API_SIGNED d_dem_pond_gewl;
  API_SIGNED d_dem_pond_red;

    //...................................TCH Full Speech.
  API_SIGNED d_maccthresh1;
  API_SIGNED d_mldt;
  API_SIGNED d_maccthresh;
  API_SIGNED d_gu;
  API_SIGNED d_go;
  API_SIGNED d_attmax;
  API_SIGNED d_sm;
  API_SIGNED d_b;

  // V42Bis module
  API_SIGNED d_v42b_switch_hyst;
  API_SIGNED d_v42b_switch_min;
  API_SIGNED d_v42b_switch_max;
  API_SIGNED d_v42b_reset_delay;

  //...................................TCH Half Speech.
  API_SIGNED d_ldT_hr;
  API_SIGNED d_maccthresh_hr;
  API_SIGNED d_maccthresh1_hr;
  API_SIGNED d_gu_hr;
  API_SIGNED d_go_hr;
  API_SIGNED d_b_hr;
  API_SIGNED d_sm_hr;
  API_SIGNED d_attmax_hr;

  //...................................TCH Enhanced FR Speech.
  API_SIGNED c_mldt_efr;
  API_SIGNED c_maccthresh_efr;
  API_SIGNED c_maccthresh1_efr;
  API_SIGNED c_gu_efr;
  API_SIGNED c_go_efr;
  API_SIGNED c_b_efr;
  API_SIGNED c_sm_efr;
  API_SIGNED c_attmax_efr;

  //...................................CHED
  API_SIGNED d_sd_min_thr_tchfs;
  API_SIGNED d_ma_min_thr_tchfs;
  API_SIGNED d_md_max_thr_tchfs;
  API_SIGNED d_md1_max_thr_tchfs;

  API_SIGNED d_sd_min_thr_tchhs;
  API_SIGNED d_ma_min_thr_tchhs;
  API_SIGNED d_sd_av_thr_tchhs;
  API_SIGNED d_md_max_thr_tchhs;
  API_SIGNED d_md1_max_thr_tchhs;

  API_SIGNED d_sd_min_thr_tchefs;
  API_SIGNED d_ma_min_thr_tchefs;
  API_SIGNED d_md_max_thr_tchefs;
  API_SIGNED d_md1_max_thr_tchefs;

  API_SIGNED d_wed_fil_ini;
  API_SIGNED d_wed_fil_tc;
  API_SIGNED d_x_min;
  API_SIGNED d_x_max;
  API_SIGNED d_slope;
  API_SIGNED d_y_min;
  API_SIGNED d_y_max;
  API_SIGNED d_wed_diff_threshold;
  API_SIGNED d_mabfi_min_thr_tchhs;

  // FACCH module
  API_SIGNED d_facch_thr;

  // IDS module
  API_SIGNED d_max_ovsp_ul;
  API_SIGNED d_sync_thres;
  API_SIGNED d_idle_thres;
  API_SIGNED d_m1_thres;
  API_SIGNED d_max_ovsp_dl;
  API_SIGNED d_gsm_bgd_mgt;

  // FIR coefficients
  API a_fir_holes[4];
  API a_fir31_uplink[31];
  API a_fir31_downlink[31];
}
T_PARAM_MCU_DSP;

#else

typedef struct
{
    //...................................Frequency Burst.
  API_SIGNED d_nsubb_idle;
  API_SIGNED d_nsubb_dedic;
  API_SIGNED d_fb_thr_det_iacq;
  API_SIGNED d_fb_thr_det_track;
    //...................................Demodulation.
  API_SIGNED d_dc_off_thres;
  API_SIGNED d_dummy_thres;
  API_SIGNED d_dem_pond_gewl;
  API_SIGNED d_dem_pond_red;
  API_SIGNED hole[1];
  API_SIGNED d_transfer_rate;
    //...................................TCH Full Speech.
  API_SIGNED d_maccthresh1;
  API_SIGNED d_mldt;
  API_SIGNED d_maccthresh;
  API_SIGNED d_gu;
  API_SIGNED d_go;
  API_SIGNED d_attmax;
  API_SIGNED d_sm;
  API_SIGNED d_b;

  #if (VOC == FR_HR) || (VOC == FR_HR_EFR)
      //...................................TCH Half Speech.
    API_SIGNED d_ldT_hr;
    API_SIGNED d_maccthresh_hr;
    API_SIGNED d_maccthresh1_hr;
    API_SIGNED d_gu_hr;
    API_SIGNED d_go_hr;
    API_SIGNED d_b_hr;
    API_SIGNED d_sm_hr;
    API_SIGNED d_attmax_hr;
  #endif

  #if (VOC == FR_EFR) || (VOC == FR_HR_EFR)
      //...................................TCH Enhanced FR Speech.
    API_SIGNED c_mldt_efr;
    API_SIGNED c_maccthresh_efr;
    API_SIGNED c_maccthresh1_efr;
    API_SIGNED c_gu_efr;
    API_SIGNED c_go_efr;
    API_SIGNED c_b_efr;
    API_SIGNED c_sm_efr;
    API_SIGNED c_attmax_efr;
  #endif

    //...................................TCH Full Speech.
  API_SIGNED d_sd_min_thr_tchfs;
  API_SIGNED d_ma_min_thr_tchfs;
  API_SIGNED d_md_max_thr_tchfs;
  API_SIGNED d_md1_max_thr_tchfs;

  #if (VOC == FR) || (VOC == FR_HR) || (VOC == FR_HR_EFR)
      //...................................TCH Half Speech.
    API_SIGNED d_sd_min_thr_tchhs;
    API_SIGNED d_ma_min_thr_tchhs;
    API_SIGNED d_sd_av_thr_tchhs;
    API_SIGNED d_md_max_thr_tchhs;
    API_SIGNED d_md1_max_thr_tchhs;
  #endif

  #if (VOC == FR_EFR) || (VOC == FR_HR_EFR)
      //...................................TCH Enhanced FR Speech.
    API_SIGNED d_sd_min_thr_tchefs;                       //(24L   *C_POND_RED)
    API_SIGNED d_ma_min_thr_tchefs;                       //(1200L *C_POND_RED)
    API_SIGNED d_md_max_thr_tchefs;                       //(2000L *C_POND_RED)
    API_SIGNED d_md1_max_thr_tchefs;                      //(160L  *C_POND_RED)
    API_SIGNED d_hole1;
  #endif

  API_SIGNED d_wed_fil_ini;
  API_SIGNED d_wed_fil_tc;
  API_SIGNED d_x_min;
  API_SIGNED d_x_max;
  API_SIGNED d_slope;
  API_SIGNED d_y_min;
  API_SIGNED d_y_max;
  API_SIGNED d_wed_diff_threshold;
  API_SIGNED d_mabfi_min_thr_tchhs;
  API_SIGNED d_facch_thr;
  API_SIGNED d_dsp_test;


  #if (DATA14_4 == 0 ) || (VOC == FR_HR_EFR)
    API_SIGNED d_patch_addr1;
    API_SIGNED d_patch_data1;
    API_SIGNED d_patch_addr2;
    API_SIGNED d_patch_data2;
    API_SIGNED d_patch_addr3;
    API_SIGNED d_patch_data3;
    API_SIGNED d_patch_addr4;
    API_SIGNED d_patch_data4;
  #endif

    //...................................
  API_SIGNED d_version_number;    // DSP patch version
  API_SIGNED d_ti_version;        // customer number. No more used since 1.5

  API_SIGNED d_dsp_page;

  #if IDS
  API_SIGNED d_max_ovsp_ul;
  API_SIGNED d_sync_thres;
  API_SIGNED d_idle_thres;
  API_SIGNED d_m1_thres;
  API_SIGNED d_max_ovsp_dl;
  #endif


}
T_PARAM_MCU_DSP;
#endif

#if (DSP_DEBUG_TRACE_ENABLE == 1)
typedef struct
{
  API d_debug_ptr_begin;
  API d_debug_ptr_end;
}
T_DB2_DSP_TO_MCU;
#endif

/*************************************************************/
/* Time informations...                                      */
/*************************************************************/
/*                                                           */
/*************************************************************/
typedef struct
{
  UWORD32  fn;                   // FN count
  UWORD16  t1;                   // FN div (26*51), (0..2047).
  UWORD8   t2;                   // FN modulo 26.
  UWORD8   t3;                   // FN modulo 51.
  UWORD8   tc;                   // Scell: TC
  UWORD8   fn_in_report;         // FN modulo 102 or 104.
  UWORD16  fn_mod42432;          // FN modulo 42432.
  UWORD8   fn_mod13;             // FN modulo 13.
  #if L1_GPRS
    UWORD8   fn_mod52;             // FN modulo 52.
    UWORD8   fn_mod104;            // FN modulo 104.
    UWORD8   fn_mod13_mod4;        // FN modulo 13 modulo 4.
    UWORD32  block_id;             // Block ID
  #endif
}
T_TIME_INFO;

/*************************************************************/
/* Idle mode tasks information...                            */
/*************************************************************/
/* must be filled according to Idle parameters...            */
/* ...                                                       */
/*************************************************************/
typedef struct
{
  UWORD8 pg_position;    // Paging block starting frame.
  UWORD8 extpg_position; // Extended Paging block starting frame.
}
T_IDLE_TASK_INFO;

/*************************************************************/
/* SDCCH information structure.                              */
/*************************************************************/
/*                                                           */
/*                                                           */
/*************************************************************/
typedef struct
{
  UWORD8 dl_sdcch_position;
  UWORD8 dl_sacch_position;
  UWORD8 ul_sdcch_position;
  UWORD8 ul_sacch_position;
  UWORD8 mon_area_position;
}
T_SDCCH_DESC;

/*************************************************************/
/* Random Access Task information structure.                 */
/*************************************************************/
/*                                                           */
/*                                                           */
/*************************************************************/
typedef struct
{
  WORD32  rand;             // 16 bit signed !!
  UWORD8  channel_request;
  UWORD8  ra_to_ctrl;
  UWORD8  ra_num;
}
T_RA_TASK_INFO;

/***************************************************************************************/
/* Measurement info element for last input level table                                 */
/***************************************************************************************/
typedef struct
{
  UWORD8  lna_off;              // 1 if lna switch is off.
  UWORD8  input_level;          // last measured input level in dbm.
}
T_INPUT_LEVEL;

/***************************************************************************************/
/* Measurement info element for Neighbor cell lists.                                   */
/***************************************************************************************/
typedef struct
{
  UWORD16  radio_freq;                // carrier id.
  WORD32  acc;                  // Accumulation of measurements already performed.
  UWORD8   nbr_meas;
}
T_MEAS_INFO;

typedef struct
{
  UWORD16  bcch_freq;
  WORD16   rxlev_acc;
  UWORD8   rxlev_nbr_meas;
}
T5_CELL_MEAS;

typedef struct
{
  T5_CELL_MEAS A[33];
}
T5_NCELL_MEAS;

/***************************************************************************************/
/* Measurement info element serving cell in dedicated mode                             */
/***************************************************************************************/
typedef struct
{
  WORD32   acc_sub;            // Subset:  accu. rxlev meas.
  UWORD32  nbr_meas_sub;       // Subset:  nbr meas. of rxlev.
  UWORD32  qual_acc_full;      // Fullset: accu. rxqual meas.
  UWORD32  qual_acc_sub;       // Subset:  accu. rxqual meas.
  UWORD32  qual_nbr_meas_full; // Fullset: nbr meas. of rxqual.
  UWORD32  qual_nbr_meas_sub;  // Subset:  nbr meas. of rxqual.
  UWORD8   dtx_used;           // Set when DTX as been used in current reporting period.
}
T_SMEAS;

/***************************************************************************************/
/*                                                                                     */
/***************************************************************************************/
typedef struct
{
  UWORD8  new_status;
  UWORD8  current_status;
  WORD32  time_to_exec;
}
T_TASK_STATUS;

/***************************************************************************************/
/* Cell/Carrier info: identity, RX level measurement, time info, gain controle info.   */
/***************************************************************************************/
typedef struct
{
  // Carrier/Cell Identity.
  UWORD16  radio_freq;                  // carrier id.
  WORD32   bsic;                   // BSIC.

  // Time difference information.
  UWORD32  fn_offset;              // offset between fn of this NCELL and the SCELL fn.
  UWORD32  time_alignmt;           // time alignment.

  // Receive Level Measurement info. structure.
  T_MEAS_INFO    meas;
  T_INPUT_LEVEL  traffic_meas;
  T_INPUT_LEVEL  traffic_meas_beacon;

  // Beacon frequency FIFO
  UWORD8         buff_beacon[4];

  #if L1_GPRS
    // Receive Level measurements in packet transfer mode
    // Daughter frequencies info.
    T_INPUT_LEVEL  transfer_meas;

    // Power reduction on serving cell PCCCH / PBCCH
    UWORD8         pb;
  #endif

  // Number of unsuccessfull attempt on SB reading.
  UWORD8  attempt_count;

  // System information bitmap.
  UWORD32  si_bit_map;             // System info. bitmap used for BCCH reading.
}
T_CELL_INFO;


typedef struct
{
  UWORD16  A[32+1];
}
TC_CHAN_LIST;


typedef struct
{
  UWORD8        num_of_chans;
  TC_CHAN_LIST  chan_list;
  BOOL          pwrc;
  BOOL          dtx_allowed;
  UWORD8        ba_id;
}
T_NEW_BA_LIST;


typedef struct
{
  UWORD8         ba_id;            // BA list identifier.

  UWORD32        nbr_carrier;      // number of carriers in the BA list.
  UWORD8         np_ctrl;          // Tels the meas_manager which PCH burst has been controled.

  UWORD8         first_index;      // First BA index measured in current session.

  UWORD8         next_to_ctrl;     // Carrier for next power measurement result.
  UWORD8         next_to_read;     // Measurement session time spent.

  UWORD8         ms_ctrl;
  UWORD8         ms_ctrl_d;
  UWORD8         ms_ctrl_dd;

  UWORD8         used_il   [2];
  UWORD8         used_il_d [2];
  UWORD8         used_il_dd[2];

  UWORD8         used_lna   [2];
  UWORD8         used_lna_d [2];
  UWORD8         used_lna_dd[2];

  T_MEAS_INFO    A[32+1];          // list of 32 neighbors + 1 serving.

  BOOL           new_list_present;
  T_NEW_BA_LIST  new_list;
}
T_BA_LIST;

typedef struct
{
  UWORD16  radio_freq;
  WORD16   accum_power_result;
}
T_POWER_ARRAY;

typedef struct
{
  UWORD16         power_array_size;
  T_POWER_ARRAY   power_array[NBMAX_CARRIER];
}
T_FULL_LIST_MEAS;

typedef struct
{
  UWORD32            nbr_sat_carrier_ctrl;  // Nb of saturated carriers after a pm session in ctrl.
  UWORD32            nbr_sat_carrier_read;  // Nb of saturated carriers after a pm session in read.

  UWORD8             meas_1st_pass_ctrl;    // flag for 1st pass during a pm session in ctrl.
  UWORD8             meas_1st_pass_read;    // flag for 1st pass during a pm session in read.

  UWORD32            next_to_ctrl;          // Carrier for next power measurement result.
  UWORD32            next_to_read;          // Measurement session time spent.

  UWORD8             ms_ctrl;
  UWORD8             ms_ctrl_d;
  UWORD8             ms_ctrl_dd;

  UWORD8             sat_flag[NBMAX_CARRIER];
                                            // last measure was saturated, so not valid
}
T_FULL_LIST;

/*************************************************************/
/* Dedicated channel information structure...                */
/*************************************************************/
/*                                                           */
/*************************************************************/
typedef struct
{
  T_CHANNEL_DESCRIPTION  *desc_ptr;    // Ptr to the Active channel description
  T_CHANNEL_DESCRIPTION  desc;         // Channel description for AFTER STI.
  T_CHANNEL_DESCRIPTION  desc_bef_sti; // Channel description for BEFORE STI.
  UWORD8                 mode;         // Channel mode.
  UWORD8                 tch_loop;     // TCH loop mode.
}
T_CHANNEL_INFO;

/*************************************************************/
/* Mobile allocation information structure...                */
/*************************************************************/
/*                                                           */
/*************************************************************/
typedef struct
{
  T_MOBILE_ALLOCATION    *alist_ptr;         // Ptr to the Active frequency list
  T_MOBILE_ALLOCATION    freq_list;
  T_MOBILE_ALLOCATION    freq_list_bef_sti;
}
T_MA_INFO;

/*************************************************************/
/* Dedicated channel parameter structure...                  */
/*************************************************************/
/*                                                           */
/*************************************************************/
typedef struct
{
  T_CHANNEL_INFO  *achan_ptr;         // Ptr to the Active channel (chan1 or chan2)
  T_CHANNEL_INFO  chan1;
  T_CHANNEL_INFO  chan2;

  T_MA_INFO       ma;

  WORD32          serv_sti_fn;        // Chan. desc. change time, serving domain.(-1 for not in use).
  WORD32          neig_sti_fn;        // Chan. desc. change time, neighbor domain.(-1 for not in use).

  // Frequency redefinition ongoing flag.
  //-------------------------------------
  UWORD8          freq_redef_flag;    // Set to TRUE when a Freq. Redef. must be confirmed.

  // Timing Advance management.
  //---------------------------
  UWORD8          timing_advance;     // Currently used TA.
  UWORD8          new_timing_advance; // New timing advance value to be used on 1st frame
                                             // of the next reporting period.
  // TXPWR management.
  //-------------------
  UWORD8          new_target_txpwr;   // New Target value for TXPWR control algo.


  T_CELL_INFO     cell_desc;          // Ptr to the new serving cell to download.

  // DAI test mode... DTX allowed...
  UWORD8          dai_mode;           // Dai test mode.
  BOOL            dtx_allowed;        // DTX allowed (flag).

  // Encryption...
  T_ENCRYPTION_KEY  ciph_key;
  UWORD8            a5mode;

  // For handover...
  UWORD8          ho_acc;             // Handover access (part of HO reference)
  WORD32          ho_acc_to_send;     // Set to 4 for SYNC HO and to -1 for ASYNC HO.
  UWORD8          t3124;              // Timer used in Async. Ho.

  // For DPAGC algorithms purpose
  UWORD8          G_all[DPAGC_FIFO_LEN];
  UWORD8          G_DTX[DPAGC_FIFO_LEN];
  #if (AMR == 1)
    UWORD8          G_amr[DPAGC_AMR_FIFO_LEN];
  #endif

  #if IDS
  // IDS mode configuration
    UWORD8        ids_mode;           // Information transfert capability coded on 2 bits
                                      // 0: speech
                                      // 1: data service
                                      // 2: fax service
  #endif
  #if (AMR == 1)
    T_AMR_CONFIGURATION  amr_configuration;
    UWORD8               cmip;
  #endif
}
T_DEDIC_SET;

/*************************************************************/
/* Dedicated channel parameter structure...                  */
/*************************************************************/
/*                                                           */
/*************************************************************/
typedef struct
{
  T_DEDIC_SET  *aset;       // Ptr to the Active parameter set
  T_DEDIC_SET  *fset;       // Ptr to the Free parameter set
  T_DEDIC_SET  set[2];      // Table of parameter set

  T_MPHC_CHANNEL_MODE_MODIFY_REQ  mode_modif;  // New mode for a given subchannel.
  WORD32                            SignalCode;  // Message name, set when a new param. set is given

#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  UWORD8       reset_sacch; // Flag to control SACCH reset (set during CHAN ASSIGN and Hand-overs)
  UWORD8       vocoder_on; // Flag to control execution of vocoder
  UWORD8       start_vocoder; // Flag to trigger start of vocoder (vocoder must be started with a synchro start)
#endif

  UWORD8       sync_tch;    // Flag used to synchronize TCH/F or TCH/H.
  UWORD8       reset_facch; // Flag used to reset FACCH buffer header on new IAS/CAS/handover
  UWORD8       stop_tch;    // Flag used to stop TCH/F or TCH/H (VEGA pwrdown).

  UWORD16      radio_freq;       // ARFCN buffer (returned by hopping algo).
  UWORD16      radio_freq_d;     // 1 frame  delayed ARFCN.
  UWORD16      radio_freq_dd;    // 2 frames delayed ARFCN.

  BOOL         pwrc;        // Flag used to reject serving pwr meas. on beacon.

  BOOL         handover_fail_mode;  // Flag used to indicate that the L1 wait for an handover fail request
  #if (AMR == 1)
    BOOL         sync_amr;        // Flag used to tell to the DSP that a new AMR paramters is ready in the NDB.
  #endif
}
T_DEDIC_PARAM;

/*************************************************************/
/* Power Management structure...                             */
/*************************************************************/
typedef struct
{
  // fields of TST_SLEEP_REQ primitive ....
  UWORD8      mode_authorized;    // NONE,SMALL,BIG,DEEP,ALL
  UWORD32           clocks;             // clocks disabled in Big sleep

  // 32 Khz gauging ....
  UWORD8      gauging_task;       // ACTIVE, INACTIVE,WAIT-IQ
  UWORD8      gaug_duration;      // gauging task duration
  UWORD8      gaug_count;         // gauging task duration compteur
  UWORD32     histo[SIZE_HIST][2];// gauging histogram
  UWORD8      enough_gaug;        // enough good gauging
  UWORD8      paging_scheduled;   // first Paging Frame

  // flags and variables for wake-up ....
  UWORD8      Os_ticks_required;  // TRUE : Os ticks to recover
  UWORD8      frame_adjust;       // TRUE : adjust 1 frame
  UWORD32     sleep_duration;     // sleep duration computed at wakeup

  // flag for sleep ....
  UWORD8      sleep_performed;    // NONE,SMALL,BIG,DEEP,ALL

  // status of clocks modules  ....
  UWORD32           modules_status;     // modules clocks status

  // constantes for 32Khz filtering
  UWORD32     c_clk_min;          // INIT state
  UWORD32     c_clk_init_min;     // INIT state
  UWORD32     c_clk_max;          // INIT state
  UWORD32     c_clk_init_max;     // INIT state
  UWORD32     c_delta_hf_acquis;  // ACQUIS state
  UWORD32     c_delta_hf_update;  // UPDATE state

  // trace gauging parameters
  UWORD8      state;   // state of the gauging
  UWORD32     lf;      // Number of the 32KHz
  UWORD32     hf;	     // HF: nb_hf( Number of the 13MHz *6 )
  UWORD32     root;	   // root & frac: the ratio of the HF & LF in each state.
  UWORD32     frac;    

}
T_POWER_MNGT;

/*************************************************************/
/* code version structure...                                 */
/*************************************************************/
typedef struct
{
    // DSP versions & checksum
    UWORD16 dsp_code_version;
    UWORD16 dsp_patch_version;
    UWORD16 dsp_checksum;     // DSP checksum : patch+code

    // MCU versions
    UWORD16 mcu_tcs_program_release;
    UWORD16 mcu_tcs_official;
    UWORD16 mcu_tcs_internal;
    UWORD16 mcu_tm_version;
}
T_VERSION;

#if L1_RECOVERY
  typedef struct
  {
    UWORD32 frame_count;
  }
  T_L1S_RECOVER;
#endif

/***************************************************************************************/
/* L1S global variable structure...                                                    */
/***************************************************************************************/
typedef struct
{
  //++++++++++++++++++++
  // Power Management...
  //++++++++++++++++++++

  T_POWER_MNGT pw_mgr;             // data base for power management

  // Time for debug & Simulation purpose...
  //  -> used as base time for BTS simulation.
  //-----------------------------------------
  UWORD32      debug_time;         // time counter used by L3 scenario...

  // L1S Tasks management...
  //-----------------------------------------
  T_TASK_STATUS  task_status[NBR_DL_L1S_TASKS];        // ...in L1S, scheduler.
  UWORD8         frame_count;                          // ...nb frames to go.
  UWORD8         forbid_meas;                          // ...frames where meas. ctrl is not allowed.

  // MFTAB management variables...
  //-----------------------------------------
  UWORD8         afrm;             // active frame ID.
  T_MFTAB  FAR   mftab;            // Multiframe table.

  // Control parameters...
  //-----------------------------------------
  UWORD32   afc_frame_count;  // AFC, Frame count between 2 calls to afc control function.
  WORD16    afc;              // AFC, Common Frequency controle.
  WORD16    toa_shift;        // TOA, value used to update the TOA
  UWORD8    toa_snr_mask;     // TOA, mask counter to reject TOA/SNR results.

  UWORD16   toa_period_count;  // TOA frame period used in PACKET TRANSFER MODE
  BOOL      toa_update;       // TOA, is set at the end of the update period, toa update occurs on next valid frame

  // Flag registers for RF task controle...
  //-----------------------------------------
  // Made these control registers short's as more than 8-bits required.
  UWORD16   tpu_ctrl_reg;     // (x,x,x,x,SYNC,RX,TX,MS) RX/TX/MS/SYNC bit ON whenever an
                              // according "controle" has been setup in the current frame.
  UWORD16   dsp_ctrl_reg;     // (x,x,x,x,x,RX,TX,MS) RX/TX/MS bit ON whenever an
                              // according "controle" has been setup in the current frame.

  //+++++++++++++++++++
  // Serving...
  //+++++++++++++++++++

  // Serving frame number management.
  //---------------------------------
  T_TIME_INFO      actual_time;          // Time info: current FN, T1, T2, T3...
  T_TIME_INFO      next_time;            // Time info: next    FN, T1, T2, T3...
  T_TIME_INFO      next_plus_time;       // Time info: next    FN, T1, T2, T3...

  // TXPWR management.
  //-------------------
  UWORD8           reported_txpwr;       // Reported value for TXPWR.
  UWORD8           applied_txpwr;        // Current value for TXPWR.

  // Last RXQUAL value.
  //-------------------
  UWORD8           rxqual;               // last rxqual value.

  // Hardware info.
  //---------------
  UWORD32          tpu_offset;           // Current TPU offset register value safeguard.
  UWORD32          tpu_offset_hw;        // Current TPU offset register value copied in the TPU.
  UWORD16          tpu_win;              // tpu window identifier inside a TDMA.

  // code versions
  T_VERSION version;

  #if (L1_GTT == 1)
    UWORD8         tty_state;  // state for L1S GTT manager.
    #if L2_L3_SIMUL
      // GTT test
      T_GTT_TEST_L1S gtt_test;
    #endif
  #endif

 #if (L1_DYN_DSP_DWNLD == 1)
  UWORD8       dyn_dwnld_state; // state for L1S DYN DWNLD manager
 #endif
  #if (AUDIO_TASK == 1)
    // Audio task.
    //-----------------------------------------
    BOOL                  l1_audio_it_com;                  // Flag to enable the ITCOM.
    UWORD8                audio_state[NBR_AUDIO_MANAGER];  // state for L1S audio manager.
    #if (MELODY_E1)
      T_L1S_MELODY_TASK   melody0;
      T_L1S_MELODY_TASK   melody1;
    #endif
    #if (VOICE_MEMO)
      T_L1S_VM_TASK       voicememo;
    #endif
    #if (L1_VOICE_MEMO_AMR)
      T_L1S_VM_AMR_TASK   voicememo_amr;
    #endif
    #if (SPEECH_RECO)
      T_L1S_SR_TASK       speechreco;
    #endif
    #if (AEC)
      T_L1S_AEC_TASK      aec;
    #endif
    #if (MELODY_E2)
      T_L1S_MELODY_E2_COMMON_VAR  melody_e2;
      T_L1S_MELODY_E2_TASK        melody0_e2;
      T_L1S_MELODY_E2_TASK        melody1_e2;
    #endif
  #endif

  UWORD8  last_used_txpwr;

  #if L1_GPRS
    BOOL    ctrl_synch_before;   //control of synchro for CCCH reading en TN-2
  #endif

  #if L1_RECOVERY
    T_L1S_RECOVER    recovery;
  #endif
  BOOL spurious_fb_detected;
  
  // Handling DTX mode
  BOOL dtx_ul_on;
  WORD8 facch_bursts;

  // DTX mode in AMR
  BOOL dtx_amr_dl_on;   // set to TRUE when the AMR is in DTX mode in downlink

}
T_L1S_GLOBAL;

/***************************************************************************************/
/* L1A global variable structure...                                                    */
/***************************************************************************************/
typedef struct
{
  // State for L1A state machines...
  //-----------------------------------------
  UWORD8    state[NBR_L1A_PROCESSES];

  // Measurement tasks management...
  //-----------------------------------------
  UWORD32   l1a_en_meas[NBR_L1A_PROCESSES];

  // Flag for forward/delete message management.
  //---------------------------------------------
  UWORD8    l1_msg_forwarded;

#if (L1_DYN_DSP_DWNLD == 1)
  // Dynamic donload global variables
  T_L1A_DYN_DWNLD_GLOBAL dyn_dwnld;
#endif

  // signal code indicating the reason of L1C_DEDIC_DONE
  UWORD32   confirm_SignalCode;

  // Trace the best frequencies reported in MPHC_RXLEV_IND 
#if (L1_MPHC_RXLEV_IND_REPORT_SORT==1)
  UWORD16 tab_index[MAX_MEAS_RXLEV_IND_TRACE];
  UWORD16 max_report; //max number of fq reported, can be < MAX_MEAS_RXLEV_IND_TRACE if list is smaller
#endif
}
T_L1A_GLOBAL;

/***************************************************************************************/
/* L1A -> L1S communication structure...                                               */
/***************************************************************************************/
typedef struct
{
  //+++++++++++++++++++
  // Serving Cell...
  //+++++++++++++++++++

  // Serving Cell identity and information.
  //---------------------------------------
  T_CELL_INFO      Scell_info;
  T_SMEAS          Smeas_dedic;

  UWORD8           Scell_IL_for_rxlev;
  T_INPUT_LEVEL    Scell_used_IL;
  T_INPUT_LEVEL    Scell_used_IL_d;
  T_INPUT_LEVEL    Scell_used_IL_dd;

  T_BCCHS          nbcchs;
  T_BCCHS          ebcchs;

  // Synchro information.
  //---------------------------------------
  WORD8            tn_difference;        // Timeslot difference for next synchro.
  UWORD8           dl_tn;                // Current timeslot for downlink stuffs.
  #if L1_GPRS
    UWORD8           dsp_scheduler_mode; // DSP Scheduler mode (GPRS or GSM).
  #endif

  // Idle parameters.
  //-----------------
  BOOL              bcch_combined;    // BS_CCCH_SDCCH_COMB flag.
  UWORD8            bs_pa_mfrms;      // BS_PA_MFRMS parameter.
  UWORD8            bs_ag_blks_res;   // BS_AG_BLKS_RES parameter.
  UWORD8            ccch_group;       // CCCH_GROUP parameter.
  UWORD8            page_group;       // PAGING_GROUP parameter.
  UWORD8            page_block_index; // Paging block index paramter.
  T_IDLE_TASK_INFO  idle_task_info;   // Idle task positions...
  UWORD8            nb_pch_per_mf51;  // nbr paging blocks per mf51.

  // CBCH parameters.
  // ----------------
  UWORD32               offset_tn0;         // TPU offset for TN=0 (used for SMSCB only).
  T_CHANNEL_DESCRIPTION cbch_desc;          // CBCH (SMSCB) channel description.
  T_MOBILE_ALLOCATION   cbch_freq_list;     // CBCH frequency list (hopping freq list).
  UWORD32               mf51_fn;            // Starting FN (for CBCH reading.
  UWORD8                cbch_start_in_mf51; // Starting position of CBCH in the MF51.
  T_CBCH_HEAD_SCHEDULE  norm_cbch_schedule; // Normal CBCH scheduling structure.
  T_CBCH_HEAD_SCHEDULE  ext_cbch_schedule;  // Extended CBCH scheduling structure.
  T_CBCH_INFO_SCHEDULE  cbch_info_req;
  BOOL                  pre_scheduled_cbch; // CBCH task has to be scheduled 1 FN in advance
  BOOL                  change_synchro_cbch;// A Pseudo Synchro is needed to read CBCH block
  UWORD8                tn_smscb;           // CBCH TN taking into account new Synchro

  // Random Access information.
  // ----------------------------
  T_RA_TASK_INFO   ra_info;

  // ADC management.
  //-------------------
  UWORD16          adc_mode;
  UWORD8           adc_idle_period;
  UWORD8           adc_traffic_period;
  UWORD8           adc_cpt;

  // TXPWR management.
  //-------------------
  UWORD8           powerclass_band1;     // Power class for the MS, given in ACCESS LINK mode (GSM Band).
  UWORD8           powerclass_band2;     // Power class for the MS, given in ACCESS LINK mode (DCS Band).

  // Dedicated parameters.
  //----------------------
  T_DEDIC_PARAM    dedic_set;             // Dedicated channel parameters.

  //+++++++++++++++++++
  // Neighbour Cells...
  //+++++++++++++++++++

  T_BCCHN_LIST     bcchn;
  T_NSYNC_LIST     nsync;

  // BA list / FULL list.
  //---------------------
  T_BA_LIST        ba_list;
  T_FULL_LIST      full_list;
  T_FULL_LIST_MEAS *full_list_ptr;

  //+++++++++++++++++++
  // L1S scheduler...
  //+++++++++++++++++++

  // L1S tasks management...
  //-----------------------------------------
  BOOL            task_param[NBR_DL_L1S_TASKS];   // ...synchro semaphores.
  BOOL            l1s_en_task[NBR_DL_L1S_TASKS];  // ...enable register.
  UWORD32         time_to_next_l1s_task;          // time to wait to reach starting frame of next task.
  UWORD8          l1a_activity_flag;              // Activity flag.

  // Measurement tasks management...
  //-----------------------------------------
  UWORD32         meas_param;         // Synchro semaphore bit register.
  UWORD32         l1s_en_meas;        // Enable task bit register.

  // L1 mode...
  //-----------------------------------------
  UWORD32         mode;               // functional mode: CS_MODE, I_MODE...

  //++++++++++++++++++++++++
  // Controle parameters...
  //++++++++++++++++++++++++
  UWORD32         fb_mode;            // Mode for fb detection algorithm.
  UWORD8          toa_reset;          // Flag for TOA algo. reset.

  // Input level memory for AGC management.
  //---------------------------------------
  T_INPUT_LEVEL    last_input_level[NBMAX_CARRIER+1];

  BOOL          recovery_flag;       // in case of the system is down and needs to be recovered

  //++++++++++++++++++++++++
  // Audio task...
  //++++++++++++++++++++++++
  #if (AUDIO_TASK == 1)
    #if (KEYBEEP)
      T_KEYBEEP_TASK keybeep_task;
    #endif
    #if (TONE)
      T_TONE_TASK    tone_task;
    #endif
    #if (MELODY_E1)
      T_MELODY_TASK  melody0_task;
      T_MELODY_TASK  melody1_task;
    #endif
    #if (VOICE_MEMO)
      T_VM_TASK      voicememo_task;
    #endif
    #if (L1_VOICE_MEMO_AMR)
      T_VM_AMR_TASK  voicememo_amr_task;
    #endif
    #if (SPEECH_RECO)
      T_SR_TASK      speechreco_task;
    #endif
    #if (AEC)
      T_AEC_TASK     aec_task;
    #endif
    #if (FIR)
      T_FIR_TASK     fir_task;
    #endif
    #if (AUDIO_MODE)
      T_AUDIO_MODE_TASK audio_mode_task;
    #endif
    #if (MELODY_E2)
      T_MELODY_E2_TASK  melody0_e2_task;
      T_MELODY_E2_TASK  melody1_e2_task;
    #endif
    #if (L1_CPORT == 1)
      T_CPORT_TASK    cport_task;
    #endif
  #endif

  //+++++++++++++
  // GTT task
  //+++++++++++++

  #if (L1_GTT == 1)
    T_GTT_TASK   gtt_task;
  #endif

  // Dynamic DSP download task
   #if (L1_DYN_DSP_DWNLD == 1)
      T_DYN_DWNLD_TASK_COMMAND       dyn_dwnld_task;
   #endif

}
T_L1A_L1S_COM;

/***************************************************************************************/
/* L1A -> DSP communication structure...                                               */
/***************************************************************************************/
typedef struct
{
  UWORD8           dsp_w_page;       // Active page for ARM "writting" to DSP {0,1}.
  UWORD8           dsp_r_page;       // Active page for ARM "reading" from DSP {0,1}.
  UWORD8           dsp_r_page_used;  // Used in "l1_synch" to know if the read page must be chged.

  T_DB_DSP_TO_MCU *dsp_db_r_ptr;     // MCU<->DSP comm. read  page (Double Buffered comm. memory).
  T_DB_MCU_TO_DSP *dsp_db_w_ptr;     // MCU<->DSP comm. write page (Double Buffered comm. memory).
  T_NDB_MCU_DSP   *dsp_ndb_ptr;      // MCU<->DSP comm. read/write (Non Double Buffered comm. memory).

  T_PARAM_MCU_DSP *dsp_param_ptr;    // MCU<->DSP comm. read/write (Param comm. memory).

 #if (DSP_DEBUG_TRACE_ENABLE == 1)
   T_DB2_DSP_TO_MCU *dsp_db2_current_r_ptr;
   T_DB2_DSP_TO_MCU *dsp_db2_other_r_ptr;
 #endif
}
T_L1S_DSP_COM;

/***************************************************************************************/
/* L1A -> TPU communication structure...                                               */
/***************************************************************************************/
typedef struct
{
  UWORD8    tpu_w_page;       // Active page for ARM "writting" to TPU {0,1}.
  UWORD32  *tpu_page_ptr;     // Current Pointer within the active "tpu_page".
  #if (CODE_VERSION == SIMULATION)
    T_reg_cmd  *reg_cmd;      // command register for TPU & DSP enabling and pages pgmation
  #else
    UWORD16    *reg_cmd;      // command register for TPU & DSP enabling and pages pgmation
  #endif
  UWORD32      *reg_com_int;  // communication int. register
  UWORD32      *offset;       // offset register
}
T_L1S_TPU_COM;

/***************************************************************************************/
/* L1 configuration structure                                               */
/***************************************************************************************/

typedef struct
{
  UWORD8   id;                   //standard identifier



  UWORD16  radio_band_support;


  UWORD8   swap_iq_band1;
  UWORD8   swap_iq_band2;

  UWORD32  first_radio_freq;
  UWORD32  first_radio_freq_band2;
  UWORD32  radio_freq_index_offset;
  UWORD32  nbmax_carrier;
  UWORD32  nbmeas;
  UWORD32  max_txpwr_band1;
  UWORD32  max_txpwr_band2;
  UWORD32  txpwr_turning_point;

  UWORD16  cal_freq1_band1;
  UWORD16  cal_freq1_band2;
  UWORD16  g_magic_band1;
  UWORD16  g_magic_band2;
  UWORD16  lna_att_band1;
  UWORD16  lna_att_band2;
  UWORD16  lna_switch_thr_low_band1;
  UWORD16  lna_switch_thr_low_band2;
  UWORD16  lna_switch_thr_high_band1;
  UWORD16  lna_switch_thr_high_band2;
}
T_L1_STD_CNFG;

//RF dependent parameter definitions
typedef struct
{
  UWORD16  rx_synth_setup_time;
  UWORD8   rx_synth_load_split;
  WORD16   rx_synth_start_time;
  WORD16   rx_change_offset_time;
  WORD16   rx_change_synchro_time;
  UWORD8   rx_tpu_scenario_ending;

  UWORD16  tx_synth_setup_time;
  UWORD8   tx_synth_load_split;
  WORD16   tx_synth_start_time;
  WORD16   tx_change_offset_time;
  WORD16   tx_nb_duration;
  WORD16   tx_ra_duration;
  UWORD8   tx_nb_load_split;
  UWORD8   tx_ra_load_split;
  UWORD8   tx_tpu_scenario_ending;

  WORD16   fb26_anchoring_time;
  WORD16   fb26_change_offset_time;

  UWORD32  prg_tx_gsm;
  UWORD32  prg_tx_dcs;

  UWORD16  low_agc_noise_thr;
  UWORD16  high_agc_sat_thr;

  UWORD16  low_agc;
  UWORD16  high_agc;

  UWORD16  il_min;

  UWORD16  fixed_txpwr;
  WORD16   eeprom_afc;
  WORD8    setup_afc_and_rf;

  UWORD32  psi_sta_inv;
  UWORD32  psi_st;
  UWORD32  psi_st_32;
  UWORD32  psi_st_inv;

  #if (VCXO_ALGO==1)
    WORD16   afc_dac_center;
    WORD16   afc_dac_min;
    WORD16   afc_dac_max;
    WORD16   afc_snr_thr;
    UWORD8   afc_algo;
    UWORD8   afc_win_avg_size_M;
    UWORD8   rgap_algo;
    UWORD8   rgap_bad_snr_count_B;
  #endif

  UWORD8   guard_bits;

  #if DCO_ALGO
    BOOL     dco_enabled;
  #endif

  #if (ANALOG == 1)
    UWORD16 debug1;
    UWORD16 afcctladd;
    UWORD16 vbuctrl;
    UWORD16 vbdctrl;
    UWORD16 bbctrl;
    UWORD16 apcoff;
    UWORD16 bulioff;
    UWORD16 bulqoff;
    UWORD16 dai_onoff;
    UWORD16 auxdac;
    UWORD16 vbctrl;
    UWORD16 apcdel1;
  #endif
  #if (ANALOG == 2)
    UWORD16 debug1;
    UWORD16 afcctladd;
    UWORD16 vbuctrl;
    UWORD16 vbdctrl;
    UWORD16 bbctrl;
    UWORD16 bulgcal;
    UWORD16 apcoff;
    UWORD16 bulioff;
    UWORD16 bulqoff;
    UWORD16 dai_onoff;
    UWORD16 auxdac;
    UWORD16 vbctrl1;
    UWORD16 vbctrl2;
    UWORD16 apcdel1;
    UWORD16 apcdel2;
  #endif
  #if (ANALOG == 3)
    UWORD16 debug1;
    UWORD16 afcctladd;
    UWORD16 vbuctrl;
    UWORD16 vbdctrl;
    UWORD16 bbctrl;
    UWORD16 bulgcal;
    UWORD16 apcoff;
    UWORD16 bulioff;
    UWORD16 bulqoff;
    UWORD16 dai_onoff;
    UWORD16 auxdac;
    UWORD16 vbctrl1;
    UWORD16 vbctrl2;
    UWORD16 apcdel1;
    UWORD16 apcdel2;
    UWORD16 vbpop;
    UWORD16 vau_delay_init;
    UWORD16 vaud_cfg;
    UWORD16 vauo_onoff;
    UWORD16 vaus_vol;
    UWORD16 vaud_pll;
  #endif

  #if L1_GPRS
    UWORD16  toa_pm_thres;  // PM threshold for TOA algorithm feeding in packet transfer mode
  #endif
}
T_L1_PARAMS;

typedef struct
{
 T_L1_STD_CNFG  std;            //standard: GSM,GSM_E,GSM850,DCS,PCS,DUAL,DUALEXT
 UWORD8         pwr_mngt;       //power management active
 UWORD8         tx_pwr_code;
 UWORD16        dwnld;
 T_L1_PARAMS    params;
 double         dpll;           //dpll factor

 #if TESTMODE
   //Define the TestMode flag and TestMode parameters
   UWORD8  TestMode;

   UWORD8  agc_enable;
   UWORD8  afc_enable;
   UWORD8  adc_enable;

   T_TM_PARAMS tmode;  //TestMode parameters structure
 #endif

}
T_L1_CONFIG;

/***************************************************************************************/
/* API HISR -> L1A communication structure... Defined in case dynamic download is defined                                          */
/***************************************************************************************/
/***************************************************************************************/
/* Global API HISR -Defined in case dynamic download is defined                                          
/***************************************************************************************/


#if(L1_DYN_DSP_DWNLD==1)
typedef struct
{
  T_L1A_DYN_DWNLD_HISR_COM dyn_dwnld;
} T_L1A_API_HISR_COM;

typedef struct
{
  T_L1_DYN_DWNLD_API_HISR dyn_dwnld;
} T_L1_API_HISR;
#endif

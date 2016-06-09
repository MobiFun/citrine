/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_MSGTY.H
 *
 *        Filename l1_msgty.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/



/* channels types */
#define L2_CHANNEL_SACCH    1
#define L2_CHANNEL_SDCCH    2
#define L2_CHANNEL_FACCH_H  3
#define L2_CHANNEL_FACCH_F  4
#define L2_CHANNEL_CCCH     5
#define L2_CHANNEL_NBCCH    6
#define L2_CHANNEL_PCH      7
#define L2_CHANNEL_EPCH     8
#define L2_CHANNEL_CBCH     9
#define L2_CHANNEL_EBCCH   10

/****************************************************************/
/* Structure definition for L1S <-> DATA ADAPTOR data blocks    */
/* TCH/9.6 -> 30 bytes                                          */
/* TCH/4.8 -> 15 bytes                                          */
/* TCH/2.4 ->  9 bytes                                          */
/****************************************************************/
typedef struct
{  
  UWORD8 A[30];
}  
T_DATA_FRAME;

/****************************************************************/
/* Structure definition for L1A <-> MMI messages                */
/****************************************************************/

typedef struct
{
  UWORD8  tx_flag;
  UWORD8  traffic_period;
  UWORD8  idle_period;
}  
T_MMI_ADC_REQ;

/****************************************************************/
/* Structure definition for L1S <-> L1A messages                */
/****************************************************************/
typedef T_PH_DATA_IND  T_L1_BCCH_INFO;
typedef T_PH_DATA_IND  T_L1_CCCH_INFO;

/****************************************************************/
/* Structure definition for new L3 <-> L1 messages              */
/****************************************************************/

#if (OP_L1_STANDALONE == 1)
/* Message used for hardware dynamic configuration */
typedef struct
{
  UWORD8   num_of_clock_cfg;    // Dynamic clock configuration index
}
T_TST_HW_CONFIG_REQ;
#endif // OP_L1_STANDALONE

/* Message used for software dynamic configuration */
typedef struct
{
  UWORD8   ids_enable;    // activation of IDS module
  T_FACCH_TEST_PARAMS facch_test;
}
T_TST_SW_CONFIG_REQ;

typedef struct
{
  UWORD32  mf51_fn;
}
T_MPHC_START_CBCH_READING;

typedef struct
{
  T_RXLEV_MEAS     A[8]; 
  UWORD8           nbr_of_carriers;
  WORD8            s_rxlev;
  UWORD8           ba_id;
  //added for Enhanced RSSI 
  UWORD16          qual_acc_idle;           // accumulated rxqual meas. on different channels in Idle mode.= error bits
  UWORD32          qual_nbr_meas_idle; // accumulated rxqual meas. on different channels in Idle mode.= total number of bits decoded

  
}
T_MPHC_RXLEV_PERIODIC_IND;

typedef struct
{
  TC_CHAN_LIST  chan_list;
  UWORD8        num_of_chans;
  UWORD8        ba_id;
  UWORD8        next_radio_freq_measured;  // index of first radio_freq to be measured
}
T_MPHC_RXLEV_PERIODIC_REQ;

typedef struct
{
  UWORD16  radio_freq;          // carrier id.
}
T_MPHC_NCELL_FB_SB_READ;

typedef struct
{
  UWORD16  radio_freq;          // carrier id.
}
T_MPHC_START_BCCH_READING;

typedef struct
{
  UWORD16  radio_freq;          // carrier id.
  UWORD32  fn_offset;      // offset between fn of this NCELL and the SCELL fn.
  UWORD32  time_alignmt;   // time alignment.
  UWORD8   bsic;           // BSIC.
  UWORD16  si_bit_map;     // System Info. bit map.
}
T_MPHC_NCELL_BCCH_READ;

typedef struct
{
  UWORD32  fn;
  UWORD8   channel_request;
} 
T_MPHC_RA_CON;

typedef struct
{
  T_CHANNEL_DESCRIPTION  channel_desc;
  UWORD8                 timing_advance;
  T_MOBILE_ALLOCATION    frequency_list;
  T_STARTING_TIME        starting_time;
  T_MOBILE_ALLOCATION    frequency_list_bef_sti;
  UWORD8                 maio_bef_sti;
  BOOL                   dtx_allowed;
  T_BCCH_LIST            bcch_allocation;
  UWORD8                 ba_id;
  BOOL                   pwrc;
}  
T_MPHC_IMMED_ASSIGN_REQ;

typedef struct
{
  T_CHANNEL_DESCRIPTION   channel_desc_1;
  UWORD8                  channel_mode_1;
  UWORD8                  txpwr;
  T_MOBILE_ALLOCATION     frequency_list;
  T_STARTING_TIME         starting_time;
  T_CHANNEL_DESCRIPTION   channel_desc_2;
  UWORD8                  channel_mode_2;
  T_MOBILE_ALLOCATION     frequency_list_bef_sti;
  T_CHANNEL_DESCRIPTION   channel_desc_1_bef_sti;
  T_CHANNEL_DESCRIPTION   channel_desc_2_bef_sti;
  UWORD8                  cipher_mode;
  UWORD8                  a5_algorithm;
  T_ENCRYPTION_KEY        cipher_key;
  BOOL                    dtx_allowed;
  #if (AMR == 1)
    T_AMR_CONFIGURATION   amr_configuration;
  #endif
}  
T_MPHC_CHANNEL_ASSIGN_REQ;


typedef struct
{
  UWORD8                cipher_mode;
  UWORD8                a5_algorithm;
  T_ENCRYPTION_KEY      new_ciph_param;
}
T_MPHC_SET_CIPHERING_REQ;


typedef struct
{
    T_CHANNEL_DESCRIPTION  channel_desc;
    T_MOBILE_ALLOCATION    frequency_list;
    T_STARTING_TIME        starting_time;
}
T_MPHC_CHANGE_FREQUENCY;


typedef struct
{
  UWORD8   txpwr;
  UWORD8   rand;
  UWORD8   channel_request;
#if (L1_FF_MULTIBAND == 0)  
  UWORD8   powerclass_band1;
  UWORD8   powerclass_band2;
#endif

}
T_MPHC_RA_REQ;


typedef struct
{
  T_HO_PARAMS           handover_command;
  UWORD32               fn_offset;
  UWORD32               time_alignmt;
  T_ENCRYPTION_KEY      cipher_key;
  #if (AMR == 1)
    T_AMR_CONFIGURATION amr_configuration;
  #endif // (AMR == 1)
  #if ((REL99 == 1) && (FF_BHO == 1))
    BOOL               handover_type;
  #endif
}
T_MPHC_ASYNC_HO_REQ;

typedef struct
{
  T_HO_PARAMS           handover_command;
  UWORD32               fn_offset;
  UWORD32               time_alignmt;
  T_ENCRYPTION_KEY      cipher_key;
  BOOL                  nci;
  BOOL                  timing_advance_valid;
  UWORD8                timing_advance;
  #if (AMR == 1)
    T_AMR_CONFIGURATION amr_configuration;
  #endif
  #if ((REL99 == 1) && (FF_BHO == 1))
    BOOL               handover_type;
  #endif
}  
T_MPHC_PRE_SYNC_HO_REQ;

typedef struct
{
  T_HO_PARAMS           handover_command;
  UWORD32               fn_offset;
  UWORD32               time_alignmt;
  T_ENCRYPTION_KEY      cipher_key;
  BOOL                  nci;
  UWORD8                real_time_difference;
#if ((REL99 == 1) && (FF_BHO == 1))
    BOOL handover_type;
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
}
T_MPHC_PSEUDO_SYNC_HO_REQ;

typedef struct
{
  T_HO_PARAMS           handover_command;
  UWORD32               fn_offset;
  UWORD32               time_alignmt;
  T_ENCRYPTION_KEY      cipher_key;
  BOOL                  nci;
  #if (AMR == 1)
    T_AMR_CONFIGURATION amr_configuration;
  #endif
  #if ((REL99 == 1) && (FF_BHO == 1))
    BOOL               handover_type;
  #endif
}  
T_MPHC_SYNC_HO_REQ;

typedef struct
{
  UWORD8  cause;
  #if ((REL99 == 1) && (FF_BHO == 1))
    UWORD32 fn_offset;
    UWORD32 time_alignment;
  #endif
}  
T_MPHC_HANDOVER_FINISHED;

typedef struct
{
  BOOL           dtx_used;
  BOOL           meas_valid;
  WORD16         rxlev_full_acc;
  UWORD8         rxlev_full_nbr_meas;
  WORD16         rxlev_sub_acc;
  UWORD8         rxlev_sub_nbr_meas;
  UWORD16        rxqual_full_acc_errors;
  UWORD16        rxqual_full_nbr_bits;
  UWORD16        rxqual_sub_acc_errors;
  UWORD16        rxqual_sub_nbr_bits;
  UWORD8         no_of_ncell_meas;
  T5_NCELL_MEAS  ncell_meas;
  UWORD8         ba_id;
  UWORD8         timing_advance;
  UWORD8         txpwr_used;
#if (REL99 == 1)
#if FF_EMR
    WORD16       rxlev_val_acc;
    UWORD8       rxlev_val_nbr_meas;
    UWORD32      mean_bep_block_acc;
    UWORD16      cv_bep_block_acc;
    UWORD8       mean_bep_block_num;
    UWORD8       cv_bep_block_num;
    UWORD8       nbr_rcvd_blocks;
#endif
#endif

  // RESERVED: for trace/debug only
  UWORD8         facch_dl_count;
  UWORD8         facch_ul_count;
  #if (FF_REPEATED_DL_FACCH == 1)
     UWORD8 facch_dl_combined_good_count; /* No of good decoded blocks after combining */
     UWORD8 facch_dl_repetition_block_count; /* Total of Dl block count */
  #endif
}  
T_MPHC_MEAS_REPORT;

typedef T_NEW_BA_LIST  T_MPHC_UPDATE_BA_LIST;


typedef struct
{
  UWORD8  bs_pa_mfrms;
  UWORD8  bs_ag_blks_res;
  BOOL    bcch_combined;
  UWORD8  ccch_group;
  UWORD8  page_group;
  UWORD8  page_block_index;
  UWORD8  page_mode;
}
T_MPHC_START_CCCH_REQ;

typedef struct
{
  UWORD8   sb_flag;         //TRUE if SB found and belongs to PLMN, otherwise FALSE
  UWORD16  radio_freq;          // carrier id.
  UWORD8   bsic;           // BSIC.
  UWORD32  fn_offset;      // offset between fn of this NCELL and the SCELL fn.
  UWORD32  time_alignmt;   // time alignment.
}
T_MPHC_NCELL_SB_READ;

typedef T_FULL_LIST_MEAS           T_MPHC_RXLEV_REQ;
typedef T_FULL_LIST_MEAS           T_L1C_VALID_MEAS_INFO;
typedef T_MPHC_RXLEV_PERIODIC_IND  T_L1C_RXLEV_PERIODIC_DONE;

#if (L1_FF_MULTIBAND == 0)

    typedef struct
    {
      UWORD8 radio_band_config; // frequency band configuration: E-GSM, DCS, GSM/DCS, PCS
    }
    T_MPHC_INIT_L1_REQ;

#else // For Multiband the Init request is just a dummy and init confirm contains info

    typedef struct
    {
      T_L1_MULTIBAND_POWER_CLASS multiband_power_class[NB_MAX_GSM_BANDS];
    }
    T_MPHC_INIT_L1_CON;

#endif // L1_FF_MULTIBAND == 0

/****************************************************************/
/* Structure definition for Test <-> L1A messages                */
/****************************************************************/

typedef struct
{
  UWORD16  dsp_code_version;
  UWORD16  dsp_checksum;
  UWORD16  dsp_patch_version;
  UWORD16  mcu_tcs_program_release;
  UWORD16  mcu_tcs_official;
  UWORD16  mcu_tcs_internal;
}
T_TST_TEST_HW_CON;
  
typedef struct
{
  UWORD8  type;
}
T_L1_STATS_REQ;

////////////////////
// Trace messages //
////////////////////

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 7))

#if (DSP_DEBUG_TRACE_ENABLE == 1)
// DSP DEBUG buffer display
typedef struct
{
  UWORD16  size;
  UWORD32  fn;
  UWORD16  debug_time;
  UWORD16  patch_version;
  UWORD16  trace_level;
  API      buffer[2];  // ANOTHER DEFINITION ???
}
T_DSP_DEBUG_INFO;

// DSP AMR trace
typedef struct
{
  UWORD16  size;
  UWORD32  fn;
  API      buffer[2];  // ANOTHER DEFINITION ???
}
T_DSP_AMR_DEBUG_INFO;

#endif

typedef struct
{
  UWORD32        trace_config;
  UWORD32        rtt_cell_enable[8];
  UWORD32        rtt_event;
}
T_TRACE_CONFIG_CHANGE;

#if (L1_GPRS)
// Packet transfer trace
typedef struct
{
  UWORD32 fn;
  UWORD8  rx_allocation;
  UWORD8  tx_allocation;
  BOOL    blk_status;
  UWORD8  dl_cs_type;
  UWORD8  dl_status[4];
  UWORD8  ul_status[8];
}
T_CONDENSED_PDTCH_INFO;
#endif

typedef struct
{
  UWORD8   debug_code;
  UWORD32  fn;
  UWORD32  tab[7];
}
T_QUICK_TRACE;

#endif

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 7))
typedef struct
{
  UWORD8   debug_code;
  UWORD32  tab[8];
}
T_TRACE_INFO;
#endif

#if (TRACE_TYPE==7) // CPU LOAD

// Number of measurements before output to UART

#define C_MESURE_DEPTH    13

/*
 *  cpu        : hisr cpu load in microseconds
 *  cpu_access : lisr -> hisr begining cpu load in microseconds
 *  fn         : Frame number modulo 104 
 */

typedef struct
{
 UWORD16 cpu;
 UWORD16 cpu_access;
 UWORD8  fn;
 BOOL    valid;
} 
T_MESURE;

typedef struct
{
  UWORD8   debug_code;
  T_MESURE tab[C_MESURE_DEPTH];
}
T_TRACE_INFO_CPU_LOAD;

#endif


/****************************************************************/
/* Structure definition for POWER MANAGEMENt.                   */
/****************************************************************/
typedef struct
{
  UWORD8   sleep_mode;
  UWORD16  clocks;
}
T_TST_SLEEP_REQ;

// ...................NEW FOR ALR....................
typedef struct
{
  UWORD8            schedule_array_size;
  T_BCCHS_SCHEDULE  schedule_array[10];
}
T_MPHC_SCELL_NBCCH_REQ;

typedef struct
{
  UWORD8            schedule_array_size;
  T_BCCHS_SCHEDULE  schedule_array[10];
}
T_MPHC_SCELL_EBCCH_REQ;

typedef struct
{
  UWORD16  radio_freq;
  UWORD32  fn_offset;
  UWORD32  time_alignmt;
  UWORD8   tsc;
  UWORD16  bcch_blks_req;
#if L1_GPRS
  UWORD8   gprs_priority;
#endif
}
T_MPHC_NCELL_BCCH_REQ;

typedef struct
{
  UWORD16        radio_freq;
  UWORD8         l2_channel;
  BOOL           error_flag;
  T_RADIO_FRAME  l2_frame;
  UWORD8         tc;
  WORD8          ccch_lev;
  UWORD32        fn;

  // L1S -> L1A data only
  UWORD8         neigh_id;
}
T_MPHC_DATA_IND;

typedef T_MPHC_DATA_IND  T_MPHC_NCELL_BCCH_IND;
typedef T_MPHC_DATA_IND  T_L1C_BCCHS_INFO;
typedef T_MPHC_DATA_IND  T_L1C_BCCHN_INFO;

typedef struct
{
  UWORD16     radio_freq;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      timing_validity;
  UWORD8      search_mode;
}
T_MPHC_NETWORK_SYNC_REQ;

typedef struct
{
  UWORD16     radio_freq;
  BOOL        sb_flag;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      bsic;
}
T_MPHC_NETWORK_SYNC_IND;

typedef struct
{
  UWORD16     radio_freq;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      timing_validity;
}
T_MPHC_NCELL_SYNC_REQ;

#if (L1_12NEIGH ==1)
typedef struct
{
  UWORD8     eotd;
  UWORD8     list_size;
  T_MPHC_NCELL_SYNC_REQ ncell_list[NBR_NEIGHBOURS];
}
T_MPHC_NCELL_LIST_SYNC_REQ;
#endif



typedef struct
{
  UWORD16     radio_freq;
  BOOL        sb_flag;          // used to fill "data_valid" field for Cursor
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      bsic;

  // L1S -> L1A data only
  UWORD8      neigh_id;
  UWORD8      attempt;

  // RESERVED: for trace/debug and test mode only
  UWORD32     pm;
  UWORD32     toa;
  UWORD32     angle;
  UWORD32     snr;

  // EOTD data : L1S -> L1A
#if (L1_EOTD==1)
  UWORD8      eotd_data_valid;  // indicates to L3 that it's an EOTD result
  UWORD8      mode;             // indicates to CURSOR that it's Idle(0) or Dedicated (1)
  WORD16      d_eotd_first;
  WORD16      d_eotd_max;
  UWORD32     d_eotd_nrj;
  WORD16      a_eotd_crosscor[18];
  UWORD32     timetag; 
  UWORD32     fn_sb_neigh;      // used for Timetag computation
  UWORD32     fn_in_SB;         // sent to CURSOR for SC fn (header=46 ...) 

  // TOA correction for timetag in dedicated mode...
  WORD32      toa_correction;

  // for Debug traces ............
  UWORD32     delta_fn;
  WORD32      delta_qbit;
#endif 
}
T_MPHC_NCELL_SYNC_IND;

typedef T_MPHC_NCELL_SYNC_IND  T_L1C_SB_INFO;
typedef T_MPHC_NCELL_SYNC_IND  T_L1C_SBCONF_INFO;

typedef struct
{
  UWORD16     radio_freq;
  UWORD32     fn_offset;
  UWORD32     time_alignmt;
  UWORD8      bsic;
}
T_MPHC_NEW_SCELL_REQ;

typedef struct
{
  BOOL    fb_flag;
  WORD8   ntdma;
  UWORD8  neigh_id;
#if (L1_12NEIGH ==1)
  // L1S --> L1A data only 
  UWORD8   attempt;
#endif
  // RESERVED: for Trace/Debug and test mode only
  UWORD32  pm;
  UWORD32  toa;
  UWORD32  angle;
  UWORD32  snr;
  UWORD16 radio_freq;
}
T_L1C_FB_INFO;

#if ((REL99 == 1) && (FF_BHO == 1))
typedef struct
{
  BOOL    fb_flag;
  BOOL    sb_flag;
  UWORD8  bsic;
  UWORD32 fn_offset;
  UWORD32 time_alignmt;
  UWORD32 pm;
  UWORD32 toa;
  UWORD32 angle;
  UWORD32 snr;
}
T_L1C_FBSB_INFO;
#endif

typedef struct
{
  WORD8     radio_freq_array_size;
#if (L1_12NEIGH ==1)
  UWORD16   radio_freq_array[NBR_NEIGHBOURS];
#else
  UWORD16   radio_freq_array[6];
#endif
}
T_MPHC_STOP_NCELL_SYNC_REQ;

typedef struct
{
  UWORD8    radio_freq_array_size;
  UWORD16   radio_freq_array[6];
}
T_MPHC_STOP_NCELL_BCCH_REQ;

typedef struct
{
  T_CHANNEL_DESCRIPTION cbch_desc;
  T_MOBILE_ALLOCATION   cbch_freq_list;
}
T_MPHC_CONFIG_CBCH_REQ;

typedef struct
{
  BOOL    extended_cbch;
  UWORD8  schedule_length;
  UWORD32 first_block_0;
  UWORD16 first_block_1;
}
T_MPHC_CBCH_SCHEDULE_REQ;

typedef struct
{
  UWORD8  tb_bitmap;
}
T_MPHC_CBCH_INFO_REQ;

typedef struct
{
  BOOL    extended_cbch;
  UWORD32 first_block_0;
  UWORD16 first_block_1;
}
T_MPHC_CBCH_UPDATE_REQ;

typedef struct
{
  BOOL  normal_cbch;
  BOOL  extended_cbch;
}
T_MPHC_STOP_CBCH_REQ;

// ...................NEW FOR ALR....................

/****************************************************************/
/* Structure definition for L1 configuration.                   */
/****************************************************************/
typedef struct
{
 UWORD8   std;
 UWORD8   swap_iq_band1;
 UWORD8   swap_iq_band2;
 UWORD8   pwr_mngt;
 UWORD8   tx_pwr_code;
 #if IDS
   UWORD8 ids_enable;
 #endif
 T_FACCH_TEST_PARAMS facch_test;
 UWORD16  dwnld;
 UWORD8   pwr_mngt_mode_authorized;
 UWORD32  pwr_mngt_clocks;
}
T_MMI_L1_CONFIG;

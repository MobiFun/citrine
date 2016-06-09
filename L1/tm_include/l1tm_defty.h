/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1TM_DEFTY.H
 *
 *        Filename l1tm_defty.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/


// Max size of data portion of a testmode packet
#define TM_PAYLOAD_UPLINK_SIZE_MAX   128

// CID, STATUS and CHECKSUM
#define TM_UPLINK_PACKET_OVERHEAD 3

typedef struct
{
  UWORD32   toa_sum;
  UWORD32   toa_sq_sum;
  UWORD32   toa_recent;  
  UWORD32   pm_sum;
  UWORD32   pm_sq_sum;
  UWORD16   pm_recent;  
  WORD16    angle_sum;
  UWORD32   angle_sq_sum;
  WORD16    angle_min;
  WORD16    angle_max;
  WORD16    angle_recent;  
  UWORD32   snr_sum;
  UWORD32   snr_sq_sum;
  UWORD32   snr_recent;  
  UWORD8    rssi_fifo[4];
  UWORD8    rssi_recent;  
  WORD32    loop_count;  //Must be signed for divide operation in statistics!!!
  WORD32    flag_count;  //Must be signed for divide operation in statistics!!!
  UWORD32   flag_recent;
  UWORD8    bsic;
  UWORD32   fn;
  UWORD32   qual_acc_full;      // Fullset: accu. rxqual meas.
  UWORD32   qual_nbr_meas_full; // Fullset: nbr meas. of rxqual.
  #if L1_GPRS
    UWORD8    nb_dl_pdtch_slots;
  #endif
}
T_TM_STATS;

typedef struct
{
  UWORD8  dedicated_active;
  UWORD32 rx_counter;
  UWORD16 num_bcchs;
  #if L1_GPRS
    BOOL packet_transfer_active;
  #endif
}
T_TM_STATE;

typedef struct
{
    UWORD16 prbs1_seed;
 // UWORD16 prbs2_seed; 	//for future use
}
T_TM_PRBS;

// Global TM variable
typedef struct
{
  BOOL       tm_msg_received;
  T_TM_STATS tmode_stats;
  T_TM_STATE tmode_state;
  T_TM_PRBS  tmode_prbs;
}
T_L1TM_GLOBAL;

typedef struct
{
  UWORD16     bcch_arfcn;
  UWORD16     tch_arfcn;
  UWORD16     mon_arfcn;
  #if L1_GPRS
    UWORD16     pdtch_arfcn;
    UWORD8      multislot_class;
  #endif
  UWORD8      down_up;
  UWORD8      channel_type;
  UWORD8      subchannel;
  UWORD8      tmode_continuous;
  UWORD8      reload_ramps_flag;
  BOOL        mon_report; //Used to determine wether RX stats are done in Monitor channel or TCH channel 
  BOOL        mon_tasks;  //Used to enable tasks associated with Monitor Channel
}
T_TM_RF_PARAMS;

typedef struct
{
  UWORD8      slot_num;
  WORD8       agc;
  WORD8       lna_off;
  UWORD8      number_of_measurements;
  UWORD8      place_of_measurement;
  BOOL        pm_enable;
  UWORD8      rssi_band;
  #if L1_GPRS
    UWORD8      timeslot_alloc;
    UWORD8      coding_scheme;
  #endif
}
T_TM_RX_PARAMS;

typedef struct
{
  UWORD8      tsc;
  UWORD8      txpwr;
  UWORD8      txpwr_skip;
  UWORD8      timing_advance;
  UWORD8      burst_type;
  UWORD8      burst_data;
  #if L1_GPRS
    UWORD8      timeslot_alloc;
    UWORD8      txpwr_gprs[8];
    UWORD8      coding_scheme;
    UWORD8      rlc_buffer_size;
    UWORD16     rlc_buffer[27];
  #endif
}
T_TM_TX_PARAMS;

typedef struct
{
  UWORD32     num_loops;
  UWORD32     auto_result_loops;
  UWORD32     auto_reset_loops;
  UWORD8      stat_type;
  UWORD16     stat_bitmask;
  #if L1_GPRS
    UWORD8      stat_gprs_slots;
  #endif
}
T_TM_STATS_CONFIG;

typedef struct 
{
  T_TM_RF_PARAMS    rf_params;
  T_TM_RX_PARAMS    rx_params;
  T_TM_TX_PARAMS    tx_params;
  T_TM_STATS_CONFIG stats_config;
}
T_TM_PARAMS;

typedef struct
{
  UWORD8  cid;
  UWORD8  index;
  UWORD8  status;
  UWORD16 size;  // size of result[] array
  UWORD8  result[TM_PAYLOAD_UPLINK_SIZE_MAX];
}
T_TM_RETURN;

typedef struct
{
  UWORD8  cid;
  UWORD8  index;
  UWORD8  status;
  UWORD16 size;  // size of result[] array
  UWORD8  result[5];
}
T_TM_RETURN_ABBREV;

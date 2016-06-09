/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_TRACE.H
 *
 *        Filename l1_trace.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/


#ifndef __L1_TRACE_H__
#define __L1_TRACE_H__

#include "../../riviera/rvt/rvt_gen.h"
#include <string.h>

#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
#include "rtt_gen.h"
#endif

#define L1_BINARY_TRACE 0

/********************/
/* Main definitions */
/********************/

#define TR_HEADER_FN_DELAY 10            // Delay applied to store the FN in the trace cell header
#define TR_HEADER_ID_MASK  0x000003FF    // Mask on the cell ID stored in the cell header

// Opcodes for communication with L1 Tracer
//-----------------------------------------

#define TRACE_CONFIG_OPCODE   0
#define TRACE_CHECKING_OPCODE 1

#define TRACE_CHECK_RESULT_OPCODE 1023 // WARNING: UL opcode 1023 reseved for trace version
                                       // (cannot be used for trace)

#if (GSM_IDLE_RAM != 0)
  #define INTRAM_TRACE_BUFFER_SIZE 128
  extern UWORD32          task_bitmap_idle_ram[2];
  extern UWORD32          mem_task_bitmap_idle_ram[2];
  extern CHAR intram_trace_buffer[INTRAM_TRACE_BUFFER_SIZE];
  extern CHAR * intram_buffer_current_ptr;
  extern T_RVT_MSG_LG intram_trace_size;

  void l1_intram_send_trace(void);

#endif

/****************************** ASCII trace only *****************************************/

#if (L1_BINARY_TRACE == 0) || (TRACE_TYPE == 5)

  #if (OP_L1_STANDALONE == 1)

    #if (L1_DYN_DSP_DWNLD == 1 && CODE_VERSION == SIMULATION)
      
      #if (L1_FF_MULTIBAND == 0)              
        // Dyn DWNLD (0x0001000) MP3 & MIDI traces activated (0x4000 and 0x2000)
        #define DEFAULT_DYN_TRACE_CONFIG       0x00016AE7
      #else
        #define DEFAULT_DYN_TRACE_CONFIG       ( 0x00016AE7 | (1<<L1_DYN_TRACE_MULTIBAND) )          
      #endif 
        
    #else // Below for normal L1 standalone with dynamic download
      
      #if (L1_FF_MULTIBAND == 0)   
        // MP3 & MIDI traces activated (0x4000 and 0x2000)
        #define DEFAULT_DYN_TRACE_CONFIG       0x028A6AE7
      #else 
        #define DEFAULT_DYN_TRACE_CONFIG       ( 0x028A6AE7 | (1<<L1_DYN_TRACE_MULTIBAND) )
      #endif // L1_FF_MULTIBAND
        
    #endif // L1_DYN_DSP_DWNLD == 1 && CODE_VERSION == SIMULATION

  #elif (OP_WCP == 1)

    // WCP patch: default config is no Layer1 trace
    #define DEFAULT_DYN_TRACE_CONFIG       0x00000000  // default was 0x00000BB7
    // End WCP patch

  #else  
  
    #if (L1_FF_MULTIBAND == 0)   
      #define DEFAULT_DYN_TRACE_CONFIG       0x00881BB7
    #else
      #define DEFAULT_DYN_TRACE_CONFIG       ( 0x00881BB7 | (1<<L1_DYN_TRACE_MULTIBAND) )
    #endif

  #endif

  // Possible EVENTS for L1S traces using TRACE_INFO.
  //-------------------------------------------------

  #define PM_EQUAL_0              1
  #define NO_PM_EQUAL_0           2
  #define MCU_DSP_MISMATCH        3
  #define NO_MCU_DSP_MISMATCH     4
  #define L1S_ABORT               5
  #define L1S_PACKET_TRANSFER     6
  #define L1S_RLC_STAT            7
  #define DL_PTCCH                8
  #define L1S_D_ERROR_STATUS      9
  #define TRACE_CPU_LOAD         10 // Only works with TRACE_TYPE 7
  #define RLC_DL_PARAM           11
  #define RLC_UL_PARAM           12
  #define FORBIDDEN_UPLINK       13
  #define DYN_TRACE_CHANGE       14 // Currently only work with TRACE_TYPE 4
  #define TRACE_SLEEP            15
  #define TRACE_GAUGING_RESET    16
  #define TRACE_GAUGING          17
  #define NEW_TOA                18
  #define TOA_NOT_UPDATED        19
  #define IT_DSP_ERROR           20
  #define TRACE_ADC              21
  #define PTCCH_DISABLED         22
  #if (OP_L1_STANDALONE == 0)
    #define DYN_TRACE_DEBUG      23 // Currently only work with TRACE_TYPE 4
  #endif
  #define DEDIC_TCH_BLOCK_STAT   24
  #define DSP_TRACE_DISABLE      25 // Only works with TRACE_TYPE 1 or 4
  #define DSP_TRACE_ENABLE       26 // Only works with TRACE_TYPE 1 or 4
  #if (L1_AUDIO_MCU_ONOFF == 1)
  #define L1_AUDIO_UL_ONOFF_TRACE   27
  #define L1_AUDIO_DL_ONOFF_TRACE   28
  #endif 
  #define SAIC_DEBUG             29
  #define BURST_PARAM            30
  #define TRACE_RATSCCH          31
  #define NAVC_VALUE             32
  #define PWMGT_FAIL_SLEEP       33
  #define KPD_CR                 34
 
#if(L1_PCM_EXTRACTION)
  #define L1S_PCM_ERROR_TRACE    35
#endif
  #define IQ_LOW                 36
  #if FF_TBF //verify these event numbers
    #define NO_BLOCKS_PASSED_TO_L3            37
    #define LACK_FREE_RLC_BUFFER              38
    #define RLC_BLOCK_OVERRUN                 39
    #define EGPRS_IT_DSP_MISSING              40
    #define EGPRS_IT_DSP_SPURIOUS             41
    #define IR_TESTING                        42
    #define RLC_POLL_PARAM                    43
  #endif
  // Wakeup Type for Power management
  //--------------------------------
  #define WAKEUP_FOR_UNDEFINED       0
  #define WAKEUP_FOR_L1_TASK         1
  #define WAKEUP_FOR_OS_TASK         2
  #define WAKEUP_FOR_HW_TIMER_TASK   3
  #define WAKEUP_FOR_GAUGING_TASK    4
  #define WAKEUP_BY_ASYNC_INTERRUPT  5
  #define WAKEUP_ASYNCHRONOUS_ULPD_0           6
  #define WAKEUP_ASYNCHRONOUS_SLEEP_DURATION_0 7

  // Big Sleep source for Power management
  //-------------------------------------
  #define BIG_SLEEP_DUE_TO_UNDEFINED  0  // deep sleep is forbiden : cause undefined
  #define BIG_SLEEP_DUE_TO_LIGHT_ON   1  // deep sleep is forbiden by ligth on activitie
  #define BIG_SLEEP_DUE_TO_UART       2  // deep sleep is forbiden by UART activitie
  #define BIG_SLEEP_DUE_TO_SIM        3  // deep sleep is forbiden by SIM activitie
  #define BIG_SLEEP_DUE_TO_GAUGING    4  // deep sleep is forbiden by not enought gauging
  #define BIG_SLEEP_DUE_TO_SLEEP_MODE 5  // deep sleep is forbiden by the sleep mode enabled
  #define BIG_SLEEP_DUE_TO_DSP_TRACES 6  // deep sleep is forbiden by the DSP
  #define BIG_SLEEP_DUE_TO_BLUETOOTH  7  // deep sleep is forbiden by the Bluetooth module
  #define BIG_SLEEP_DUE_TO_CAMERA     8  // deep sleep is forbiden by the camera

  void  Trace_Packet_Transfer      (UWORD8  prev_crc_error); // Previous RX blocks CRC_ERROR summary
  void  l1_display_buffer_trace_fct(void);

  // Possible cause for IT_DSP_ERROR
  //-----------------------------------
  #define IT_DSP_ERROR_CPU_OVERLOAD        0
#if (FF_L1_FAST_DECODING == 1)
  #define IT_DSP_ERROR_FAST_DECODING       2  
  #define IT_DSP_ERROR_FAST_DECODING_UNEXP 3    
#endif


 //===================================================
 //=========== BUFFER TRACE ==========================
 //===================================================

// buffer size
#define TRACE_FCT_BUFF_SIZE 40

////////////////
// fonctions id
/////////////////

// fonction name to display
#ifdef L1_TRACE_C

  #if (TRACE_TYPE==5) || TRACE_FULL_NAME
  const char string_fct_trace[][35]={

    // L1S_CTRL_XXXXX
    "l1s_ctrl_ADC()",
    "l1s_ctrl_msagc()",
    "l1s_ctrl_sb2()",
    "l1s_ctrl_sb26()",
    "l1s_ctrl_sb51()",
    "l1s_ctrl_sbconf()",
    "l1s_ctrl_sbcnf26()",
    "l1s_ctrl_sbcnf51()",
    "l1s_ctrl_fb()",
    "l1s_ctrl_fb26()",
    "l1s_ctrl_smscb()",
    "l1s_ctrl_snb_dl()",
    "l1s_ctrl_snb_dl(burst 0)",
    "l1s_ctrl_snb_dl(burst 1)",
    "l1s_ctrl_snb_dl(burst 2)",
    "l1s_ctrl_snb_dl(burst 3)",
    "l1s_ctrl_snb_ul()",
    "l1s_ctrl_nnb()",
    "l1s_ctrl_rach()",
    "l1s_ctrl_tcht_dummy(DL)",
    "l1s_ctrl_tchth(DL)",
    "l1s_ctrl_tchth(UL)",
    "l1s_ctrl_tcha(DL)",
    "l1s_ctrl_tcha(UL)",
    "l1s_ctrl_tchtf(DL)",
    "l1s_ctrl_tchtf(UL)",

    // L1PS_CTRL_XXXXX
    "l1ps_ctrl_poll()",
    "l1ps_ctrl_snb_dl",
    "l1ps_ctrl_single()",
    "l1ps_ctrl_pbcchs()",
    "l1ps_ctrl_pbcchn()",
    "l1ps_ctrl_itmeas()",
    "l1ps_ctrl_pdtch",
    "l1ps_ctrl_pdtch(UL)",
    "l1ps_ctrl_pdtch(DL)",
    "l1ps_ctrl_pdtch(DL burst0)",
    "l1ps_ctrl_pdtch(DL burst1)",
    "l1ps_ctrl_pdtch(DL burst2)",
    "l1ps_ctrl_pdtch(DL burst3)",
    "l1ps_ctrl_pdtch(RA)",
    "l1ps_ctrl_pdtch(dummy)",
    "l1ps_ctrl_ptcch(UL)",
    "l1ps_ctrl_ptcch(empty)",
    "l1ps_ctrl_ptcch(DL burst0)",
    "l1ps_ctrl_ptcch(DL burst1)",
    "l1ps_ctrl_ptcch(DL burst2)",
    "l1ps_ctrl_ptcch(DL burst3)",

     // others CTRL
    "ctrl_cr_meas",
    "ctrl_i_ba_meas",
    "ctrl_d_ba_meas",
    "ctrl_tcr_meas_1",
    "ctrl_tcr_meas_2",
    "ctrl_pc_meas_chan",
    "ctrl_transfer_meas",
    "ctrl_full_list_meas",
    "ctrl_Scell_transfer_meas",

     // L1S_READ_XXXXX
    "l1s_read_ra()",
    "l1s_read_nnb",
    "l1s_read_snb_dl",
    "l1s_read_tx_nb(DUL)",
    "l1s_read_tx_nb(AUL)",
    "l1s_read_tx_nb(TCHF)",
    "l1s_read_tx_nb(TCHH)",
    "l1s_read_fb()",
    "l1s_read_fb51()",
    "l1s_read_fb26()",
    "l1s_read_sb()",
    "l1s_read_sbconf()",
    "l1s_read_l3frm(CB)",
    "l1s_read_l3frm(NP)",
    "l1s_read_l3frm(EP)",
    "l1s_read_l3frm(ALLC)",
    "l1s_read_l3frm(NBCCHS)",
    "l1s_read_l3frm(EBCCHS)",
    "l1s_read_l3frm(BCCHN)",
    "l1s_read_sacch_dl(ADL)",
    "l1s_read_sacch_dl(TCHA)",
    "l1s_read_dcch_dl(DDL)",
    "l1s_read_dcch_dl(TCHTF)",
    "l1s_read_dcch_dl(TCHTH)",
    "l1s_read_dedic_dl",
    "l1s_read_mon_result",
    "l1s_read_dummy",
    "l1s_read_msagc()",

     // L1PS_READ_XXXXX
    "l1ps_read_nb_dl",
    "l1ps_read_itmeas()",
    "l1ps_read_single",
    "l1ps_read_single_dummy",
    "l1ps_read_l3frm(PNP)",
    "l1ps_read_l3frm(PEP)",
    "l1ps_read_l3frm(PALLC)",
    "l1ps_read_l3frm(PBCCHS)",
    "l1ps_read_l3frm(PBCCHN)",
    "l1ps_read_l3frm(SINGLE)",
    "l1ps_read_l3frm(?)",
    "l1ps_read_pra()",
    "l1ps_read_poll()",
    "  l1ps_read_pdtch()",
    "l1ps_read_pdtch(burst)",
    "l1ps_read_ptcch(DL)",
    "l1ps_read_ptcch(UL)",

     // others READ
    "read_cr_meas",
    "read_tcr_meas",
    "read_i_ba_meas",
    "read_d_ba_meas",
    "read_pc_meas_chan",
    "read_full_list_meas",

    // miscellaneous
    "SYNCHRO...",
    "L1S_ABORT...",
    "L1S_ABORT(PAGE:R0 W0)",
    "unknown_fb()",
    "STI PASSED...",
    "task KILLED...",
    "ALLOC EXHAUSTION",
    "UL task does not correspond",
    "DL task does not correspond",
    "DL burst does not correspond",
    "=>NEW_FRAME(PAGE:R0 W0)",
    "=>NEW_FRAME(PAGE:R0 W1)",
    "=>NEW_FRAME(PAGE:R1 W0)",
    "=>NEW_FRAME(PAGE:R1 W1)",
    "l1dmacro_synchro",
    "tx_tch_data()",
    "dll_read_dcch()",
    "dll_read_sacch()",
    "Time adjustment",
  };

  #endif
#endif // L1_TRACE_C


 //===================================================
 //=========== BUFFER TRACE END ======================
 //===================================================

 #if (OP_L1_STANDALONE == 0)
   // Dynamic trace: message content
   //-------------------------------
   #define DYN_TRACE_0    0
   #define DYN_TRACE_1    1
   #define DYN_TRACE_2    2
   #define DYN_TRACE_3    3  
   #define DYN_TRACE_4    4  
   #define DYN_TRACE_5    5  
   #define DYN_TRACE_6    6  
   #define DYN_TRACE_7    7  
   #define DYN_TRACE_8    8
   #define DYN_TRACE_9    9  
   #define DYN_TRACE_10   10
   #define DYN_TRACE_11   11
   #define DYN_TRACE_12   12
   #define DYN_TRACE_13   13
   #define DYN_TRACE_14   14
   #define DYN_TRACE_15   15
   #define DYN_TRACE_16   16
 #endif

/****************************** Binary trace only *****************************************/

#else
  #define DEFAULT_DYN_TRACE_CONFIG       0x000007a7
#endif


/***********************************************************/
/* Trace structures                                        */
/***********************************************************/

// Trace version
typedef struct
{
  UWORD32 Opcode;
  UWORD32 checksum;
  UWORD16 version;
}
T_TRACE_VERSION;

// Condensed trace structure definition
typedef struct
{
  BOOL   blk_status;
  UWORD8 dl_cs_type;
  UWORD8 dl_status[8];
  UWORD8 ul_status[8];
} T_PDTCH_TRACE;


#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
// L1S trace function pointers
typedef struct
{
  T_RTT_RET (*rtt_refresh_status) (T_RTT_USER_ID  user_id);

  T_RTT_PTR (*rtt_get_fill_ptr)   (T_RTT_USER_ID  user_id,
                                   T_RTT_SIZE     size);

  T_RTT_RET (*rtt_dump_buffer)    (T_RTT_USER_ID  user_id,
                                   T_RTT_SIZE     dump_size);
} T_L1S_TRACE_FUNC;
#endif


// L1S trace buffer size
#define L1S_RTT_BUF_LENGTH 1000

// Trace configuration
typedef struct
{
  UWORD32   l1_dyn_trace;
  UWORD32   rttl1_cell_enable[8];
  UWORD32   rttl1_event_enable;
} T_TRACE_CONFIG;

// Disable/enable DSP trace structure
#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
#if (MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )

typedef struct
{
  // Flag for blocking dsp trace while performing e2, mp3, aac or dynamic download activities
  BOOL trace_flag_blocked;
  // Nested Disable dsp trace counter
  UWORD8 nested_disable_count;
  // Trace level copy to be restored at the end of e2, mp3, aac or dynamic download activities
  UWORD16 dsp_trace_level_copy;
} T_DSP_TRACE_HANDLER;

#endif
#endif // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

#if (TOA_ALGO == 2)
typedef struct
{
    UWORD16   toa_frames_counter;  // TOA Frames counter - Number of the TDMA frames (or bursts) which are used for TOA 
                                   // updation OR number of times l1ctl_toa() function is invoked 
                                   // Reset every TOA_PERIOD_LEN[l1_mode] frames
    UWORD16   toa_accumul_counter; // Number of TDMA frames (or bursts) which are actually used for TOA tracking
                                   // <= toa_frames_counter, as only if SNR>0.46875 TOA estimated by DSP is used to
                                   // update the tracking algorithm
    WORD16    toa_accumul_value;   // TOA_tracking_value accumulated over 'toa_accumul_counter' frames
                                   // Based on this value the shift to be applied is decided
}T_TRACE_TOA;
#endif

typedef struct
{
    UWORD8   fail_step;  // PWMGT Fail Step -> Periph Check OR osload/Timer/Gauging OR While puuting peripherals to sleep
    UWORD8   fail_id;    // PWMGT Fail ID -> i.e. If Periph Check is the fail step whether failure is because of UART, etc.
    UWORD8   fail_cause; // Why the Peripheral returned failure?  
}T_TRACE_L1_PWMGR_DEBUG;

#if (AUDIO_DEBUG == 1)
typedef struct
{
  UWORD8      vocoder_enable_status;
  UWORD8      ul_state;
  UWORD8      dl_state;
  UWORD8      ul_onoff_counter;
  UWORD8      dl_onoff_counter;
}T_TRACE_AUDIO_DEBUG;
#endif
  
typedef struct
{
  UWORD32   dl_count;                    /*  Number of Downlink SACCH block                    */
  UWORD32   dl_combined_good_count;      /*  Number of successfully decoded combined block     */
  UWORD32  dl_error_count;    /* Total errors     */
  UWORD8   srr;                         /*  SACCH Repetition Request                          */
  UWORD8   sro;                         /*  SACCH Repetition Order                            */
  /* trace,debug for FER */
  UWORD32   dl_good_norep;               /* Number of correctly decoded block which is not a repetition */
  API              dl_buffer[12];       /* Downlink buffer                                     */
  BOOL         dl_buffer_empty;         /* Flag to indicate the downlink buffer is empty/full */
}
T_TRACE_REPEAT_SACCH;


// Debug info structure
typedef struct
{
  // User IDs
  T_RVT_USER_ID       l1_trace_user_id;
#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
  T_RTT_USER_ID       l1s_trace_user_id;
#endif

  UWORD8    PM_equal_0;
  UWORD8    PM_Task;
  UWORD8    Not_PM_Task;

  UWORD8    DSP_misaligned;

  UWORD8    facch_dl_count;
  UWORD8    facch_ul_count;
  UWORD8    facch_dl_fail_count;
  UWORD8    facch_dl_fail_count_trace;

  UWORD8    sacch_d_nerr;
  #if (FF_REPEATED_SACCH == 1)
  T_TRACE_REPEAT_SACCH    repeat_sacch;
  #endif /* (FF_REPEATED_SACCH == 1) */

  UWORD8    rxlev_req_count;
  BOOL      init_trace;
  UWORD8    abort_task;

#if (L1_BINARY_TRACE == 0)
  UWORD8    l1_memorize_error;

  UWORD8    trace_fct_buff[TRACE_FCT_BUFF_SIZE];
  UWORD8    trace_fct_buff_index;
  BOOL      trace_buff_stop;
  BOOL      trace_filter;
#endif

  BOOL      sleep_performed;
  UWORD8    reset_gauging_algo;

#if L1_GPRS
  BOOL          new_tcr_list;
  T_PDTCH_TRACE pdtch_trace;
#endif

#if L1_GTT
  T_RVT_USER_ID  gtt_trace_user_id;
#endif

#if (L1_MIDI == 1)
  T_RVT_USER_ID  midi_trace_user_id;
#endif

#if (D_ERROR_STATUS_TRACE_ENABLE)
  // define a mask array for handling of the d_error_status field
  UWORD16 d_error_status_masks[2];
  API     d_error_status_old;
#endif

#if (DSP_DEBUG_TRACE_ENABLE == 1)
  // Variable used to flag a DSP error, COM mismatch or PM=0 occured
  // Array x 2 --> double buffered
  // Contains 0 if no error / DSP trace start address if an error occured
  UWORD16 dsp_debug_buf_start[2];
  UWORD32 dsp_debug_fn[2];
  UWORD16 dsp_debug_time[2];
  UWORD32 fn_last_dsp_debug;
#endif

  // RTT
#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
  T_L1S_TRACE_FUNC l1s_rtt_func;
  UWORD8           l1s_trace_buf[L1S_RTT_BUF_LENGTH];
#endif
  UWORD32          task_bitmap[8];
  UWORD32          mem_task_bitmap[8];

  #if (TOA_ALGO == 2)
    T_TRACE_TOA    toa_trace_var;
  #endif  
  T_TRACE_L1_PWMGR_DEBUG pwmgt_trace_var;  
  #if(L1_SAIC != 0)
  UWORD8 prev_saic_flag_val;
  UWORD8 prev_swh_flag_val;
  #endif
  // Dynamic trace
  T_TRACE_CONFIG   config[2];
  T_TRACE_CONFIG   *current_config;
  T_TRACE_CONFIG   *pending_config;

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
#if (MELODY_E2 || L1_MP3 || L1_DYN_DSP_DWNLD)
  // DSP Trace Handler global variables
  T_DSP_TRACE_HANDLER dsptrace_handler_globals;
#endif
#endif // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
#if (AUDIO_DEBUG == 1)  
  T_TRACE_AUDIO_DEBUG  audio_debug_var;
#endif  
#if (L1_RF_KBD_FIX == 1)
UWORD16 prev_correction_ratio;
#endif
#if (FF_REPEATED_DL_FACCH == 1 )
   UWORD8   facch_dl_combined_good_count;    /* Number of successfully decoded combined block */
   UWORD8   facch_dl_repetition_block_count;   /*Number of repetition block*/
   UWORD8   facch_dl_count_all;                          /* Number of DL FACCH block*/
   UWORD8   facch_dl_good_block_reported;      /*  Number of correctly decoded block which is not a repetition */          
#endif
}
T_TRACE_INFO_STRUCT;

extern T_TRACE_INFO_STRUCT trace_info;


/***********************/
/* Function prototypes */
/***********************/

void  l1_init_trace_var          (void);
void  l1_trace_init              (void);
void  Trace_L1s_Abort            (UWORD8 task);
void  Trace_MCU_DSP_Com_Mismatch (UWORD8 task);
void  Trace_PM_Equal_0           (UWORD32 pm, UWORD8 task);
#if FF_TBF
void Trace_rlc_ul_param          (UWORD8 assignment_id,
                                  UWORD32 fn,
                                  UWORD8 tx_no,
                                  UWORD8 ta,
                                  BOOL fix_alloc_exhaust,
                                  UWORD32 cs_type);
void Trace_rlc_dl_param          (UWORD8 assignment_id,
                                  UWORD32 fn,
                                  UWORD8 rx_no,
                                  UWORD8 rlc_blocks_sent,
                                  UWORD8 last_poll_response,
                                  UWORD32 status1,
                                  UWORD32 status2);
void Trace_rlc_poll_param        (BOOL poll,
                                  UWORD32 fn,
                                  UWORD8 poll_ts,
                                  UWORD8 tx_alloc,
                                  UWORD8 tx_data,
                                  UWORD8 rx_alloc,
                                  UWORD8 last_poll_resp,
                                  UWORD8 ack_type);
#else
void  Trace_rlc_dl_param         (UWORD8  assignment_id,
                                  UWORD32 fn,
                                  UWORD32 d_rlcmac_rx_no_gprs,
                                  UWORD8  rx_no,
                                  UWORD8  rlc_blocks_sent,
                                  UWORD8  last_poll_response);
void  Trace_rlc_ul_param         (UWORD8  assignment_id,
                                  UWORD8  tx_no,
                                  UWORD32 fn,
                                  UWORD8  ta,
                                  UWORD32 a_pu_gprs,
                                  UWORD32 a_du_gprs,
                                  BOOL    fix_alloc_exhaust);
#endif
void  Trace_uplink_no_TA         (void);
void  Trace_condensed_pdtch      (UWORD8  rx_allocation, UWORD8 tx_allocation);
void  Trace_dl_ptcch             (UWORD8 ordered_ta,
                                  UWORD8 crc,
                                  UWORD8 ta_index,
                                  UWORD8 ts,
                                  UWORD16 elt1,
                                  UWORD16 elt2,
                                  UWORD16 elt3,
                                  UWORD16 elt4,
                                  UWORD16 elt5,
                                  UWORD16 elt6,
                                  UWORD16 elt7,
                                  UWORD16 elt8);
void  Trace_d_error_status       (void);
void  Trace_dsp_debug            (void);
#if (AMR == 1)
  void Trace_dsp_amr_debug       (void);
#endif
void  Trace_params               (UWORD8   debug_code,
                                  UWORD32  param0,
                                  UWORD32  param1,
                                  UWORD32  param2,
                                  UWORD32  param3,
                                  UWORD32  param4,
                                  UWORD32  param5,
                                  UWORD32  param6);
void  Trace_L1S_CPU_load         (void);
void  l1_dsp_cpu_load_read       (void);
void  Trace_dyn_trace_change     (void);
#if (AMR == 1)
void  l1_trace_ratscch            (UWORD16 fn, UWORD16 amr_change_bitmap);
#endif
void  l1_trace_sleep             (UWORD32 start_fn,
                                  UWORD32 end_fn,
                                  UWORD8 type_sleep,
                                  UWORD8 wakeup_type,
                                  UWORD8 big_sleep_type,
                                  UWORD16 int_id);
void  l1_trace_fail_sleep        (UWORD8 pwmgr_fail_step,
                                  UWORD8 pwmgr_fail_id,
                                  UWORD8 pwmgr_fail_cause);
void  l1_trace_sleep_intram       (UWORD32 start_fn,
                                   UWORD32 end_fn,
                                   UWORD8 type_sleep,
                                   UWORD8 wakeup_type,
                                   UWORD8 big_sleep_type,
                                   UWORD16 int_id);
void  l1_trace_gauging_reset     (void);
void  l1_trace_gauging           (void);
void  l1_trace_gauging_intram     (void);
#if (L1_SAIC != 0)
void  l1_trace_saic            (UWORD32 SWH_flag, UWORD32 SAIC_flag);
#endif

#if (L1_NAVC_TRACE == 1)
  void  l1_trace_navc            (UWORD32 status, UWORD32 energy_level);
#endif  
void l1_trace_burst_param         (UWORD32 angle,
                                   UWORD32 snr,
                                   UWORD32 afc,
                                   UWORD32 task,
                                   UWORD32 pm,
                                   UWORD32 toa_val,
                                   UWORD32 IL_for_rxlev);
void l1_log_burst_param           (UWORD32 angle,
                                   UWORD32 snr,
                                   UWORD32 afc,
                                   UWORD32 task,
                                   UWORD32 pm,
                                   UWORD32 toa_val,
                                   UWORD32 IL_for_rxlev);
void  l1_trace_new_toa           (void);
void  l1_trace_new_toa_intram     (void);
void  l1_trace_toa_not_updated   (void);
void  l1_trace_IT_DSP_error      (UWORD8 cause);
void  l1_trace_ADC               (UWORD8 type);
void  l1_trace_ADC_intram         (UWORD8 type);
void  l1_check_com_mismatch      (UWORD8 task);
void  l1_check_pm_error          (UWORD32 pm,UWORD8 task);
void  Trace_PM_Equal_0_balance   (void);
void  l1_trace_ptcch_disable     (void);
void  trace_fct                   (UWORD8 fct_id, UWORD32 radio_freq);
void  l1_intram_put_trace         (CHAR *msg);
void  l1_trace_IT_DSP_error_intram(void);
void  Trace_d_error_status_intram (void);
void  l1s_trace_mftab             (void);
void  l1s_trace_mftab             (void);

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
#if (MELODY_E2 || L1_MP3 || L1_DYN_DSP_DWNLD)
void    l1_disable_dsp_trace      (void);
void    l1_enable_dsp_trace       (void);
void    l1_set_dsp_trace_mask     (UWORD16 mask);
UWORD16 l1_get_dsp_trace_mask     (void);
#endif
#endif // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

#if (L1_AUDIO_MCU_ONOFF == 1)
void  l1_trace_ul_audio_onoff(UWORD8 ul_state);
void  l1_trace_dl_audio_onoff(UWORD8 dl_state);
#endif 
#if FF_TBF
//  void l1_trace_egprs            (UWORD8 type);

  //For burst power trace.AGC_TRACE
  void l1_trace_agc (UWORD8 burst_id, UWORD8 agc);
  void l1_trace_burst (UWORD8 *time_slot, UWORD8 burst_id);
  void burst_trace_message(void);
#endif 

/****************/
/* Trace macros */
/****************/

#if (CODE_VERSION != SIMULATION)
#define GTT_send_trace_cpy(s)    rvt_send_trace_cpy    ((T_RVT_BUFFER)s, trace_info.gtt_trace_user_id, strlen(s), RVT_ASCII_FORMAT)
#define GTT_send_trace_no_cpy(s) rvt_send_trace_no_cpy ((T_RVT_BUFFER)s, trace_info.gtt_trace_user_id, strlen(s), RVT_ASCII_FORMAT)
#else
  void GTT_send_trace_cpy(char *s);
#endif

/***********************************************************/
/* Trace data (parsed by the decoder)                      */
/***********************************************************/

// Trace version
//--------------

#define L1_TRACE_VERSION 5

// Dynamic traces
//---------------

//TRACE_CONF/
#define L1_DYN_TRACE_L1A_MESSAGES      0 //NAME/ L1A messages
#define L1_DYN_TRACE_L1S_DEBUG         1 //NAME/ L1S errors
#define L1_DYN_TRACE_DSP_DEBUG         2 //NAME/ DSP debug trace
#define L1_DYN_TRACE_RLC_PARAM         3 //NAME/ RLC parameters
#define L1_DYN_TRACE_UL_NO_TA          4 //NAME/ Uplink while no TA
#define L1_DYN_TRACE_DL_PTCCH          5 //NAME/ DL PTCCH blocks
#define L1_DYN_TRACE_CONDENSED_PDTCH   7 //NAME/ PDTCH UL + DL
#define L1_DYN_TRACE_L1S_CPU_LOAD      8 //NAME/ L1S CPU load peaks
#define L1_DYN_TRACE_ULPD              9 //NAME/ ULPD
#define L1_DYN_TRACE_FULL_LIST_REPORT 10 //NAME/ Full list report
#define L1_DYN_TRACE_GTT              11 //NAME/ GTT trace
#define L1_DYN_TRACE_DSP_AMR_DEBUG    12 //NAME/ DSP AMR debug trace
#define L1_DYN_TRACE_MIDI             13 //NAME/ MIDI trace
#define L1_DYN_TRACE_MP3              14 //NAME/ MP3 trace
#define L1_DYN_TRACE_GAUGING          15 //NAME/ Gauging parameters 
#if(L1_DYN_DSP_DWNLD == 1)
  #define L1_DYN_TRACE_DYN_DWNLD   16 //NAME/ DYN DWNLD trace
#endif // L1_DYN_DSP_DWNLD == 1

#if (L1_SAIC != 0)
  #define L1_DYN_TRACE_SAIC_DEBUG  17 //NAME/ SAIC trace  
#endif
#define L1_DYN_TRACE_BURST_PARAM   18 //NAME/ Burst Param  

#if (L1_AUDIO_MCU_ONOFF == 1)
  #define L1_DYN_TRACE_AUDIO_ONOFF  19
#endif  
#if FF_TBF
  #define L1_DYN_TRACE_POLL_PARAM     29 //NAME/ Poll parameters
  #endif
// The Below flag is used to enable/disable the API dump over UART   
#define L1_DYN_TRACE_API_DUMP      20 //NAME/ API dump

#define L1_DSP_TRACE_FULL_DUMP     21 // flag for enabling the full trace buffer of DSP on PM error
#if (L1_AAC == 1)
#define L1_DYN_TRACE_AAC              22 //NAME/ AAC trace
#endif  
#define L1_DYN_TRACE_PWMGT_FAIL_DEBUG  23 // NAME Power Management Sleep fail Trace

#if(L1_RF_KBD_FIX == 1)
#define L1_DYN_TRACE_RF_KBD  24 //Make RF KPD trace dynamic
#endif

#define L1_DYN_TRACE_DSP_CPU_LOAD      25 //NAME/ DSP CPU load trace

#if (L1_FF_MULTIBAND == 1)
#define L1_DYN_TRACE_MULTIBAND        26 /*MULTIBAND DEBUG trace*/
#endif


//END_TRACE_CONF/

#define L1_DYN_TRACE_DL_PDTCH_CRC      6 // DL PDTCH blocks CRC, only used if L1_BINARY_TRACE == 0

// L1 RTT event definitions
//-------------------------

//RTT_EVENTS/
#define RTTL1_EVENT_FNMOD13_EQUAL_12                         0 //NAME/ Every FN%13 = 12
#define RTTL1_EVENT_ERROR                                    1 //NAME/ When error occurs
//END_RTT_EVENTS/

// Buffer length for each event
#define RTTL1_EVENT_SIZE_FNMOD13_EQUAL_12   L1S_RTT_BUF_LENGTH  // All buffer is traced because it's a regular trace
#define RTTL1_EVENT_SIZE_ERROR              L1S_RTT_BUF_LENGTH

// Measurement codes used in trace
//--------------------------------

//MEAS_ID/
#define FULL_LIST_MEAS_ID   200 //NAME/ Full list meas
#define I_BA_MEAS_ID        201 //NAME/ Idle BA list meas
#define D_BA_MEAS_ID        202 //NAME/ Dedicated BA list meas
#define MS_AGC_ID           203 //NAME/ AGC setting meas
#define CR_MEAS_ID          204 //NAME/ CR meas
#define TCR_MEAS_ID         205 //NAME/ Packet transfer CR meas
#define PC_MEAS_CHAN_ID     206 //NAME/ Beacon meas
//END_MEAS_ID/

// Trace tables
//-------------

//TABLE/ RRBP
#define RRBP_BLOCK1   0 //NAME/ N+1
#define RRBP_BLOCK2   1 //NAME/ N+2
#define RRBP_BLOCK3   2 //NAME/ N+3
#define RRBP_BLOCK4   3 //NAME/ N+4
//END_TABLE/

//TABLE/ DL CS
#define DL_CS1   0 //NAME/ CS1
#define DL_CS2   1 //NAME/ CS2
#define DL_CS3   2 //NAME/ CS3
#define DL_CS4   3 //NAME/ CS4
//END_TABLE/

//TABLE/ MFTAB
#define CST_L1S_CTRL_ADC                    0  //NAME/ l1s_ctrl_ADC()
#define CST_L1S_CTRL_MSAGC                  1  //NAME/ l1s_ctrl_msagc()
#define CST_L1S_CTRL_SB2                    2  //NAME/ l1s_ctrl_sb2()
#define CST_L1S_CTRL_SB26                   3  //NAME/ l1s_ctrl_sb26()
#define CST_L1S_CTRL_SB51                   4  //NAME/ l1s_ctrl_sb51()
#define CST_L1S_CTRL_SBCONF                 5  //NAME/ l1s_ctrl_sbconf()
#define CST_L1S_CTRL_SBCNF26                6  //NAME/ l1s_ctrl_sbcnf26()
#define CST_L1S_CTRL_SBCNF51                7  //NAME/ l1s_ctrl_sbcnf51()
#define CST_L1S_CTRL_FB                     8  //NAME/ l1s_ctrl_fb()
#define CST_L1S_CTRL_FB26                   9  //NAME/ l1s_ctrl_fb26()
#define CST_L1S_CTRL_SMSCB                 10  //NAME/ l1s_ctrl_smscb()
#define CST_L1S_CTRL_SNB_DL                11  //NAME/ l1s_ctrl_snb_dl()
#define CST_L1S_CTRL_SNB_DL_BURST0         12  //NAME/ l1s_ctrl_snb_dl(burst 0)
#define CST_L1S_CTRL_SNB_DL_BURST1         13  //NAME/ l1s_ctrl_snb_dl(burst 1)
#define CST_L1S_CTRL_SNB_DL_BURST2         14  //NAME/ l1s_ctrl_snb_dl(burst 2)
#define CST_L1S_CTRL_SNB_DL_BURST3         15  //NAME/ l1s_ctrl_snb_dl(burst 3)
#define CST_L1S_CTRL_SNB_UL                16  //NAME/ l1s_ctrl_snb_ul()
#define CST_L1S_CTRL_NNB                   17  //NAME/ l1s_ctrl_nnb()
#define CST_L1S_CTRL_RACH                  18  //NAME/ l1s_ctrl_rach()
#define CST_L1S_CTRL_TCHT_DUMMY__DL        19  //NAME/ l1s_ctrl_tcht_dummy(DL)
#define CST_L1S_CTRL_TCHTH__DL             20  //NAME/ l1s_ctrl_tchth(DL)
#define CST_L1S_CTRL_TCHTH__UL             21  //NAME/ l1s_ctrl_tchth(UL)
#define CST_L1S_CTRL_TCHA___DL             22  //NAME/ l1s_ctrl_tcha(DL)
#define CST_L1S_CTRL_TCHA___UL             23  //NAME/ l1s_ctrl_tcha(UL)
#define CST_L1S_CTRL_TCHTF__DL             24  //NAME/ l1s_ctrl_tchtf(DL)
#define CST_L1S_CTRL_TCHTF__UL             25  //NAME/ l1s_ctrl_tchtf(UL)
#define CST_L1PS_CTRL_POLL                 26  //NAME/ l1ps_ctrl_poll()
#define CST_L1PS_CTRL_SNB_DL               27  //NAME/ l1ps_ctrl_snb_dl
#define CST_L1PS_CTRL_SINGLE               28  //NAME/ l1ps_ctrl_single()
#define CST_L1PS_CTRL_PBCCHS               29  //NAME/ l1ps_ctrl_pbcchs()
#define CST_L1PS_CTRL_PBCCHN               30  //NAME/ l1ps_ctrl_pbcchn()
#define CST_L1PS_CTRL_ITMEAS               31  //NAME/ l1ps_ctrl_itmeas()
#define CST_L1PS_CTRL_PDTCH                32  //NAME/ l1ps_ctrl_pdtch
#define CST_L1PS_CTRL_PDTCH_UL             33  //NAME/ l1ps_ctrl_pdtch(UL)
#define CST_L1PS_CTRL_PDTCH_DL             34  //NAME/ l1ps_ctrl_pdtch(DL)
#define CST_L1PS_CTRL_PDTCH_DL_BURST0      35  //NAME/ l1ps_ctrl_pdtch(DL burst0)
#define CST_L1PS_CTRL_PDTCH_DL_BURST1      36  //NAME/ l1ps_ctrl_pdtch(DL burst1)
#define CST_L1PS_CTRL_PDTCH_DL_BURST2      37  //NAME/ l1ps_ctrl_pdtch(DL burst2)
#define CST_L1PS_CTRL_PDTCH_DL_BURST3      38  //NAME/ l1ps_ctrl_pdtch(DL burst3)
#define CST_L1PS_CTRL_PDTCH_RA             39  //NAME/ l1ps_ctrl_pdtch(RA)
#define CST_L1PS_CTRL_PDTCH_DUMMY          40  //NAME/ l1ps_ctrl_pdtch(dummy)
#define CST_L1PS_CTRL_PTCCH_UL             41  //NAME/ l1ps_ctrl_ptcch(UL)
#define CST_L1PS_CTRL_PTCCH_EMPTY          42  //NAME/ l1ps_ctrl_ptcch(empty)
#define CST_L1PS_CTRL_PTCCH_DL_BURST0      43  //NAME/ l1ps_ctrl_ptcch(DL burst0)
#define CST_L1PS_CTRL_PTCCH_DL_BURST1      44  //NAME/ l1ps_ctrl_ptcch(DL burst1)
#define CST_L1PS_CTRL_PTCCH_DL_BURST2      45  //NAME/ l1ps_ctrl_ptcch(DL burst2)
#define CST_L1PS_CTRL_PTCCH_DL_BURST3      46  //NAME/ l1ps_ctrl_ptcch(DL burst3)
#define CST_CTRL_CR_MEAS                   47  //NAME/ ctrl_cr_meas
#define CST_CTRL_I_BA_MEAS                 48  //NAME/ ctrl_i_ba_meas
#define CST_CTRL_D_BA_MEAS                 49  //NAME/ ctrl_d_ba_meas
#define CST_CTRL_TCR_MEAS_1                50  //NAME/ ctrl_tcr_meas_1
#define CST_CTRL_TCR_MEAS_2                51  //NAME/ ctrl_tcr_meas_2
#define CST_CTRL_PC_MEAS_CHAN              52  //NAME/ ctrl_pc_meas_chan
#define CST_CTRL_TRANSFER_MEAS             53  //NAME/ ctrl_transfer_meas
#define CST_CTRL_FULL_LIST_MEAS            54  //NAME/ ctrl_full_list_meas
#define CST_CTRL_SCELL_TRANSFER_MEAS       55  //NAME/ ctrl_Scell_transfer_meas
#define CST_L1S_READ_RA                    56  //NAME/ l1s_read_ra()
#define CST_L1S_READ_NNB                   57  //NAME/ l1s_read_nnb
#define CST_L1S_READ_SNB_DL                58  //NAME/ l1s_read_snb_dl
#define CST_L1S_READ_TX_NB__DUL            59  //NAME/ l1s_read_tx_nb(DUL)
#define CST_L1S_READ_TX_NB__AUL            60  //NAME/ l1s_read_tx_nb(AUL)
#define CST_L1S_READ_TX_NB__TCHF           61  //NAME/ l1s_read_tx_nb(TCHF)
#define CST_L1S_READ_TX_NB__TCHH           62  //NAME/ l1s_read_tx_nb(TCHH)
#define CST_L1S_READ_FB                    63  //NAME/ l1s_read_fb()
#define CST_L1S_READ_FB51                  64  //NAME/ l1s_read_fb51()
#define CST_L1S_READ_FB26                  65  //NAME/ l1s_read_fb26()
#define CST_L1S_READ_SB                    66  //NAME/ l1s_read_sb()
#define CST_L1S_READ_SBCONF                67  //NAME/ l1s_read_sbconf()
#define CST_L1S_READ_L3FRM__CB             68  //NAME/ l1s_read_l3frm(CB)
#define CST_L1S_READ_L3FRM__NP             69  //NAME/ l1s_read_l3frm(NP)
#define CST_L1S_READ_L3FRM__EP             70  //NAME/ l1s_read_l3frm(EP)
#define CST_L1S_READ_L3FRM__ALLC           71  //NAME/ l1s_read_l3frm(ALLC)
#define CST_L1S_READ_L3FRM__NBCCHS         72  //NAME/ l1s_read_l3frm(NBCCHS)
#define CST_L1S_READ_L3FRM__EBCCHS         73  //NAME/ l1s_read_l3frm(EBCCHS)
#define CST_L1S_READ_L3FRM__BCCHN          74  //NAME/ l1s_read_l3frm(BCCHN)
#define CST_L1S_READ_SACCH_DL__ADL         75  //NAME/ l1s_read_sacch_dl(ADL)
#define CST_L1S_READ_SACCH_DL__TCHA        76  //NAME/ l1s_read_sacch_dl(TCHA)
#define CST_L1S_READ_DCCH_DL__DDL          77  //NAME/ l1s_read_dcch_dl(DDL)
#define CST_L1S_READ_DCCH_DL__TCHTF        78  //NAME/ l1s_read_dcch_dl(TCHTF)
#define CST_L1S_READ_DCCH_DL__TCHTH        79  //NAME/ l1s_read_dcch_dl(TCHTH)
#define CST_L1S_READ_DEDIC_DL              80  //NAME/ l1s_read_dedic_dl
#define CST_L1S_READ_MON_RESULT            81  //NAME/ l1s_read_mon_result
#define CST_L1S_READ_DUMMY                 82  //NAME/ l1s_read_dummy
#define CST_L1S_READ_MSAGC                 83  //NAME/ l1s_read_msagc()
#define CST_L1PS_READ_NB_DL                84  //NAME/ l1ps_read_nb_dl
#define CST_L1PS_READ_ITMEAS               85  //NAME/ l1ps_read_itmeas()
#define CST_L1PS_READ_SINGLE               86  //NAME/ l1ps_read_single
#define CST_L1PS_READ_SINGLE_DUMMY         87  //NAME/ l1ps_read_single_dummy
#define CST_L1PS_READ_L3FRM__PNP           88  //NAME/ l1ps_read_l3frm(PNP)
#define CST_L1PS_READ_L3FRM__PEP           89  //NAME/ l1ps_read_l3frm(PEP)
#define CST_L1PS_READ_L3FRM__PALLC         90  //NAME/ l1ps_read_l3frm(PALLC)
#define CST_L1PS_READ_L3FRM__PBCCHS        91  //NAME/ l1ps_read_l3frm(PBCCHS)
#define CST_L1PS_READ_L3FRM__PBCCHN        92  //NAME/ l1ps_read_l3frm(PBCCHN)
#define CST_L1PS_READ_L3FRM__SINGLE        93  //NAME/ l1ps_read_l3frm(SINGLE)
#define CST_L1PS_READ_L3FRM__UNKNOWN       94  //NAME/ l1ps_read_l3frm(?)
#define CST_L1PS_READ_PRA                  95  //NAME/ l1ps_read_pra()
#define CST_L1PS_READ_POLL                 96  //NAME/ l1ps_read_poll()
#define CST_L1PS_READ_PDTCH                97  //NAME/ l1ps_read_pdtch()
#define CST_L1PS_READ_PDTCH_BURST          98  //NAME/ l1ps_read_pdtch(burst)
#define CST_L1PS_READ_PTCCH_DL             99  //NAME/ l1ps_read_ptcch(DL)
#define CST_L1PS_READ_PTCCH_UL            100  //NAME/ l1ps_read_ptcch(UL)
#define CST_READ_CR_MEAS                  101  //NAME/ read_cr_meas
#define CST_READ_TCR_MEAS                 102  //NAME/ read_tcr_meas
#define CST_READ_I_BA_MEAS                103  //NAME/ read_i_ba_meas
#define CST_READ_D_BA_MEAS                104  //NAME/ read_d_ba_meas
#define CST_READ_PC_MEAS_CHAN             105  //NAME/ read_pc_meas_chan
#define CST_READ_FULL_LIST_MEAS           106  //NAME/ read_full_list_meas
#define CST_L1S_NEW_SYNCHRO               107  //NAME/ SYNCHRO...
#define CST_L1S_ABORT                     108  //NAME/ L1S_ABORT...
#define CST_L1S_ABORT_W0_R0               109  //NAME/ L1S_ABORT(PAGE:R0 W0)
#define CST_UNKNOWN_FB                    110  //NAME/ unknown_fb()
#define CST_STI_PASSED                    111  //NAME/ STI PASSED...
#define CST_TASK_KILLED                   112  //NAME/ task KILLED...
#define CST_ALLOC_EXHAUSTION              113  //NAME/ ALLOC EXHAUSTION
#define CST_UL_TASKS_DO_NOT_CORRESPOND    114  //NAME/ UL task does not correspond
#define CST_DL_TASKS_DO_NOT_CORRESPOND    115  //NAME/ DL task does not correspond
#define CST_DL_BURST_DOES_NOT_CORRESPOND  116  //NAME/ DL burst does not correspond
#define CST_NEW_FRAME_PAGE_R0_W0          117  //NAME/ =>NEW_FRAME(PAGE:R0 W0)
#define CST_NEW_FRAME_PAGE_R0_W1          118  //NAME/ =>NEW_FRAME(PAGE:R0 W1)
#define CST_NEW_FRAME_PAGE_R1_W0          119  //NAME/ =>NEW_FRAME(PAGE:R1 W0)
#define CST_NEW_FRAME_PAGE_R1_W1          120  //NAME/ =>NEW_FRAME(PAGE:R1 W1)
#define CST_L1DMACRO_SYNCHRO              121  //NAME/ l1dmacro_synchro
#define CST_TX_TCH_DATA                   122  //NAME/ tx_tch_data()
#define CST_DLL_READ_DCCH                 123  //NAME/ dll_read_dcch()
#define CST_DLL_READ_SACCH                124  //NAME/ dll_read_sacch()
#define CST_L1S_ADJUST_TIME               125  //NAME/ Time adjustment
#if ((REL99 == 1) && (FF_BHO == 1))
  #define CST_L1S_CTRL_FBSB                 128  //NAME/ l1s_ctrl_fbsb()
#endif
//END_TABLE/

/***********************************************************/
/* Classic Trace structures                                */
/***********************************************************/

// !!! IMPORTANT NOTE !!!

// Trace structures:
// -----------------
// For 32 bit alignment, all structures should be mapped like this:
//  1- header
//  2- 32-bit words (arrays of 32-bit words included)
//  3- 16-bit words (arrays of 16-bit words included)
//  4-  8-bit words (arrays of 8-bit words included)
// This permit to avoid holes between variables and to have a structure independant of
// alignment

//////////////////
// ALR messages //
//////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_INIT_L1_REQ
   //FULL/
     "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
     "#@Fdl7# |->|  |  |  | INIT_L1_REQ              |   radio_band_config: #@1=1:GSM|=2:GSM_E|=3:PCS1900|=4:DCS1800|=5:DUAL|=6:DUALEXT|=7:GSM850|=8:DUAL_US|#"
   //COND/
     "#@Fdl7#  INIT_L1_REQ"
   End header */
//ID/
#define TRL1_MPHC_INIT_L1_REQ 1
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           radio_band_config;
}
T_TR_MPHC_INIT_L1_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_INIT_L1_CON
   //FULL/
     "        |  |  |  |  |                          |"
     "#@Fdl7# |<----|  |  | INIT_L1_CON              |"
   //COND/
     "#@Fdl7#  INIT_L1_CON"
   End header */
//ID/
#define TRL1_MPHC_INIT_L1_CON 2
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_INIT_L1_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_RXLEV_PERIODIC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | RXLEV_PERIODIC_REQ       |         num_of_chans: #@1d#"
    "        |  |  |  |  |                          |               ba_id : #@2d#"
    "        |  |  |  |  |                          | next_radio_freq_meas: #@3d#"
   //COND/
    "#@Fdl7#  RXLEV_PERIODIC_REQ"
   End header */
//ID/
#define TRL1_MPHC_RXLEV_PERIODIC_REQ 3
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           num_of_chans;
  UWORD8           ba_id;
  UWORD8           next_radio_freq_measured;
}
T_TR_MPHC_RXLEV_PERIODIC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_FB_SB_READ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | NCELL_FB_SB_READ         |           radio_freq: #@1d#"
    "        |  |  |  |  |                          |"
   //COND/
    "#@Fdl7#  NCELL_FB_SB_READ                                               radio_freq: #@1d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_FB_SB_READ 4
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
}
T_TR_MPHC_NCELL_FB_SB_READ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_RA_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MPHC_RA_CON              |"
   //COND/
    "#@Fdl7#  MPHC_RA_CON"
   End header */
//ID/
#define TRL1_MPHC_RA_CON 5
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_RA_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_IMMED_ASSIGN_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | IMMED_ASSIGN_REQ         |           radio_freq: #@1dl10#          rf_chan_cnt: #@2d#"
    "        |  |  |  |  |                          |  bef_sti_rf_chan_cnt: #@3dl10#                    h: #@4=0:Single RF|=1:Hopping RF|#"
    "        |  |  |  |  |                          |         channel_type: #@5T[CHAN TYPE]#"
    "        |  |  |  |  |                          |           subchannel: #@6dl10#          timeslot_no: #@7d#"
    "        |  |  |  |  |                          |                  tsc: #@8dl10#       timing_advance: #@9d#"
    "        |  |  |  |  |                          |   starting_time_pres: #@10=0:No|=1:Yes|~|l10#        starting_time: # (26 + @12 - @13) % 26 + @12 + (1326 * @11 * 51)d#"
    "        |  |  |  |  |                          |          dtx_allowed: #@14dl10#                pwrc: #@15d#"
   //COND/
    ""
    ""
    "#@Fdl7#  IMMED_ASSIGN_REQ                                               #@5=0:Invalid|=1:TCH_F|=2:TCH_H|=3:SDCCH_4|=4:SDCCH_8|#"
   End header */
//ID/
#define TRL1_MPHC_IMMED_ASSIGN_REQ 6
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD16          bef_sti_rf_chan_cnt;
  BOOL             h;
  UWORD8           channel_type;
  UWORD8           subchannel;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  UWORD8           timing_advance;
  BOOL             starting_time_present;
  UWORD8           n32;
  UWORD8           n51;
  UWORD8           n26;
  BOOL             dtx_allowed;
  BOOL             pwrc;
}
T_TR_MPHC_IMMED_ASSIGN_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CHANNEL_ASSIGN_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | CHANNEL_ASSIGN_REQ       |           radio_freq: #@1dl10#          rf_chan_cnt: #@2d#"
    "        |  |  |  |  |                          |  bef_sti_rf_chan_cnt: #@3dl10#                    h: #@4=0:Single RF|=1: Hopping RF|#"
    "        |  |  |  |  |                          |         channel_type: #@5T[CHAN TYPE]#"
    "        |  |  |  |  |                          |           subchannel: #@6dl10#          timeslot_no: #@7d#"
    "        |  |  |  |  |                          |                  tsc: #@8dl10#       channel_mode_1: #@9=0:SIG only|=1:TCH_FS|=2:TCH_HS|=3:TCH_96|=4:TCH_48F|=5:TCH_48H|=6:TCH_24F|=7:TCH_24H|=8:TCH_EFR|=9:TCH_144|=10:TCH_AHS|=11:TCH_AFS|#"
    "        |  |  |  |  |                          |                txpwr: #@10dl10#   starting_time_pres: #@11=0:No|=1:Yes|#"
    "        |  |  |  |  |                          |        starting_time: # (26 + @13 - @14) % 26 + @13 + (1326 * @12 * 51)dl10#          cipher_mode: #@15d#"
    "        |  |  |  |  |                          |         a5_algorithm: #@16dl10#          dtx_allowed: #@17=0:false|=1:true|#"
    "        |  |  |  |  |                          |      noise_suppr_bit: #@18dl10#  init_codec_mode_ind: #@19d#"
    "        |  |  |  |  |                          |   initial_codec_mode: #@20dl10#     active_codec_set: #@21d#"
    "        |  |  |  |  |                          |            threshold: #@22dr3#"
    "        |  |  |  |  |                          |           hysteresis: #@23dr3#"
   //COND/
    ""
    ""
    "#@Fdl7#  CHANNEL_ASSIGN_REQ                                             #@5=0:Invalid|=1:TCH_F|=2:TCH_H|=3:SDCCH_4|=4:SDCCH_8|#"
   End header */
//ID/
#define TRL1_MPHC_CHANNEL_ASSIGN_REQ 7
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD16          bef_sti_rf_chan_cnt;
  BOOL             h;
  UWORD8           channel_type;
  UWORD8           subchannel;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  UWORD8           channel_mode_1;
  UWORD8           txpwr;
  BOOL             starting_time_present;
  UWORD8           n32;
  UWORD8           n51;
  UWORD8           n26;
  UWORD8           cipher_mode;
  UWORD8           a5_algorithm;
  BOOL             dtx_allowed;
  BOOL             noise_suppression_bit;
  BOOL             initial_codec_mode_indicator;
  UWORD8           initial_codec_mode;
  UWORD8           active_codec_set;
  UWORD8           threshold[3];
  UWORD8           hysteresis[3];
}
T_TR_MPHC_CHANNEL_ASSIGN_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_RA_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MPHC_RA_REQ              |                txpwr: #@1dl10#                 rand: #@2d#"
    "        |  |  |  |  |                          |      channel_request: #@3dl10#     powerclass_band1: #@4d#"
    "        |  |  |  |  |                          |     powerclass_band2: #@5d#"
   //COND/
    "#@Fdl7#  RA_REQ"
   End header */
//ID/
#define TRL1_MPHC_RA_REQ 8
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           txpwr;
  UWORD8           rand;
  UWORD8           channel_request;
  UWORD8           powerclass_band1;
  UWORD8           powerclass_band2;
}
T_TR_MPHC_RA_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_ASYNC_HO_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | ASYNC_HO_REQ             |            fn_offset: #@1dl10#         time_alignmt: #@2d#"
    "        |  |  |  |  |                          |         bcch_carrier: #@3dl10#           radio_freq: #@4d#"
    "        |  |  |  |  |                          |          rf_chan_cnt: #@5dl10#  bef_sti_rf_chan_cnt: #@6d#"
    "        |  |  |  |  |                          |                  ncc: #@7dl10#                  bcc: #@8d#"
    "        |  |  |  |  |                          |                    h: #@9=0:Single RF|=1: Hopping RF|~|l10#         channel_type: #@10T[CHAN TYPE]#"
    "        |  |  |  |  |                          |           subchannel: #@11dl10#          timeslot_no: #@12d#"
    "        |  |  |  |  |                          |                  tsc: #@13dl10#       channel_mode_1: #@14=0:SIG only|=1:TCH_FS|=2:TCH_HS|=3:TCH_96|=4:TCH_48F|=5:TCH_48H|=6:TCH_24F|=7:TCH_24H|=8:TCH_EFR|=9:TCH_144|=10:TCH_AHS|=11:TCH_AFS|#"
    "        |  |  |  |  |                          |                txpwr: #@15dl10#   starting_time_pres: #@16=0:No|=1:Yes|#"
    "        |  |  |  |  |                          |        starting_time: # (26 + @18 - @19) % 26 + @18 + (1326 * @17 * 51)dl10#               ho_acc: #@20d#"
    "        |  |  |  |  |                          |     report_time_diff: #@21dl10#          cipher_mode: #@22d#"
    "        |  |  |  |  |                          |         a5_algorithm: #@23dl10#      noise_suppr_bit: #@24d#"
    "        |  |  |  |  |                          |  init_codec_mode_ind: #@25dl10#   initial_codec_mode: #@26d#"
    "        |  |  |  |  |                          |     active_codec_set: #@27dl#"
    "        |  |  |  |  |                          |            threshold: #@28dr3#"
    "        |  |  |  |  |                          |           hysteresis: #@29dr3#"
   //COND/
    "#@Fdl7#  ASYNC_HO_REQ                                                   bcch_carrier: #@3d# channel_type: #@10T[CHAN TYPE]#"
   End header */
//ID/
#define TRL1_MPHC_ASYNC_HO_REQ 9
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          bcch_carrier;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD16          bef_sti_rf_chan_cnt;
  UWORD8           ncc;
  UWORD8           bcc;
  BOOL             h;
  UWORD8           channel_type;
  UWORD8           subchannel;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  UWORD8           channel_mode_1;
  UWORD8           txpwr;
  BOOL             starting_time_present;
  UWORD8           n32;
  UWORD8           n51;
  UWORD8           n26;
  UWORD8           ho_acc;
  BOOL             report_time_diff;
  UWORD8           cipher_mode;
  UWORD8           a5_algorithm;
  BOOL             noise_suppression_bit;
  BOOL             initial_codec_mode_indicator;
  UWORD8           initial_codec_mode;
  UWORD8           active_codec_set;
  UWORD8           threshold[3];
  UWORD8           hysteresis[3];
}
T_TR_MPHC_ASYNC_HO_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_SYNC_HO_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SYNC_HO_REQ              |            fn_offset: #@1dl10#         time_alignmt: #@2d#"
    "        |  |  |  |  |                          |         bcch_carrier: #@3dl10#           radio_freq: #@4d#"
    "        |  |  |  |  |                          |          rf_chan_cnt: #@5dl10#  bef_sti_rf_chan_cnt: #@6d#"
    "        |  |  |  |  |                          |                  ncc: #@7dl10#                  bcc: #@8d#"
    "        |  |  |  |  |                          |                    h: #@9=0:Single RF|=1: Hopping RF|~|l10#         channel_type: #@10T[CHAN TYPE]#"
    "        |  |  |  |  |                          |           subchannel: #@11dl10#          timeslot_no: #@12d#"
    "        |  |  |  |  |                          |                  tsc: #@13dl10#       channel_mode_1: #@14=0:SIG only|=1:TCH_FS|=2:TCH_HS|=3:TCH_96|=4:TCH_48F|=5:TCH_48H|=6:TCH_24F|=7:TCH_24H|=8:TCH_EFR|=9:TCH_144|=10:TCH_AHS|=11:TCH_AFS|#"
    "        |  |  |  |  |                          |                txpwr: #@15dl10#   starting_time_pres: #@16=0:No|=1:Yes|#"
    "        |  |  |  |  |                          |        starting_time: # (26 + @18 - @19) % 26 + @18 + (1326 * @17 * 51)dl10#               ho_acc: #@20d#"
    "        |  |  |  |  |                          |     report_time_diff: #@21dl10#          cipher_mode: #@22d#"
    "        |  |  |  |  |                          |         a5_algorithm: #@23dl10#      noise_suppr_bit: #@24d#"
    "        |  |  |  |  |                          |  init_codec_mode_ind: #@25dl10#   initial_codec_mode: #@26d#"
    "        |  |  |  |  |                          |     active_codec_set: #@27dl10#"
    "        |  |  |  |  |                          |            threshold: #@28dr3#"
    "        |  |  |  |  |                          |           hysteresis: #@29dr3#"
   //COND/
    "#@Fdl7#  SYNC_HO_REQ                                                    bcch_carrier: #@3d# channel_type: #@10T[CHAN TYPE]#"
   End header */
//ID/
#define TRL1_MPHC_SYNC_HO_REQ 10
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          bcch_carrier;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD16          bef_sti_rf_chan_cnt;
  UWORD8           ncc;
  UWORD8           bcc;
  BOOL             h;
  UWORD8           channel_type;
  UWORD8           subchannel;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  UWORD8           channel_mode_1;
  UWORD8           txpwr;
  BOOL             starting_time_present;
  UWORD8           n32;
  UWORD8           n51;
  UWORD8           n26;
  UWORD8           ho_acc;
  BOOL             report_time_diff;
  UWORD8           cipher_mode;
  UWORD8           a5_algorithm;
  BOOL             noise_suppression_bit;
  BOOL             initial_codec_mode_indicator;
  UWORD8           initial_codec_mode;
  UWORD8           active_codec_set;
  UWORD8           threshold[3];
  UWORD8           hysteresis[3];
}
T_TR_MPHC_SYNC_HO_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_HANDOVER_FINISHED
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |        HANDOVER_FINISHED | #@1=0:Complete|=1:TIMEOUT|#"
   //COND/
    "#@Fdl7#                          HANDOVER_FINISHED       #@1=1:TIMEOUT|~|#"
   End header */
//ID/
#define TRL1_L1C_HANDOVER_FINISHED 11
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           cause;
}
T_TR_L1C_HANDOVER_FINISHED;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_MEAS_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                MEAS_DONE |  rxqual_full_acc_err: #@1dl10# rxqual_full_nbr_bits: #@2d#"
    "        |  |  |  |  |                          |rxqual_sub_acc_errors: #@3dl10#  rxqual_sub_nbr_bits: #@4d#"
    "        |  |  |  |  |                          |        rxlev_sub_acc: #@5dl10#       rxlev_full_acc: #@6d#"
    "        |  |  |  |  |                          |           meas_valid: #@9dl10#           txpwr_used: #@10d#"
    "        |  |  |  |  |                          |       timing_advance: #@11dl10#   rxlev_sub_nbr_meas: #@13d#"
    "        |  |  |  |  |                          |       facch_dl_count: #@14dl10#       facch_ul_count: #@15d#"
    "        |  |  |  |  |                          |            bcch_freq: #@7dr5#"
    "        |  |  |  |  |                          |            rxlev_acc: #@8dr5#"
    "        |  |  |  |  |                          |       rxlev_nbr_meas: #@16dr5#"
   //COND/
    "#@Fdl7#                          MEAS_DONE"
   End header */
//ID/
#define TRL1_L1C_MEAS_DONE 12
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          rxqual_full_acc_errors;
  UWORD16          rxqual_full_nbr_bits;
  UWORD16          rxqual_sub_acc_errors;
  UWORD16          rxqual_sub_nbr_bits;
  WORD16           rxlev_sub_acc;
  WORD16           rxlev_full_acc;
  #if REL99
  #if FF_EMR
    WORD16         rxlev_val_acc;
    UWORD8         rxlev_val_nbr_meas;
    UWORD32        mean_bep_block_acc;
    UWORD16        cv_bep_block_acc;
    UWORD8         mean_bep_block_num;
    UWORD8         cv_bep_block_num;
    UWORD8         nbr_rcvd_blocks;
  #endif
  #endif //L1_R99
  UWORD16          bcch_freq[6];
  WORD16           rxlev_acc[6];
  BOOL             meas_valid;
  UWORD8           txpwr_used;
  UWORD8           timing_advance;
  UWORD8           rxlev_full_nbr_meas;
  UWORD8           rxlev_sub_nbr_meas;
  UWORD8           facch_dl_count;
  UWORD8           facch_ul_count;
  UWORD8           rxlev_nbr_meas[6];
}
T_TR_L1C_MEAS_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_START_CCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "=========================================================================================================================================================================================="
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | START_CCCH_REQ           |          bs_pa_mfrms: #@1dl10#       bs_ag_blks_res: #@2d#"
    "        |  |  |  |  |                          |        bcch_combined: #@3dl10#           ccch_group: #@4d#"
    "        |  |  |  |  |                          |           page_group: #@5dl10#     page_block_index: #@6d#"
    "        |  |  |  |  |                          |            page_mode: #@7=0:NORMAL|=1:EXTENDED|=2:REORG|~INVALID|#"
   //COND/
    ""
    ""
    "#@Fdl7#  START_CCCH_REQ                                                 #@7=0:Normal|=1:Extended|=2:Reorg|~INVALID|#"
   End header */
//ID/
#define TRL1_MPHC_START_CCCH_REQ 13
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           bs_pa_mfrms;
  UWORD8           bs_ag_blks_res;
  BOOL             bcch_combined;
  UWORD8           ccch_group;
  UWORD8           page_group;
  UWORD8           page_block_index;
  UWORD8           page_mode;
}
T_TR_MPHC_START_CCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_SB_READ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | NCELL_SB_READ            |           radio_freq: #@3dl10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2d#"
   //COND/
    ""
    ""
    "#@Fdl7#  NCELL_SB_READ                                                  radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_SB_READ 14
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
}
T_TR_MPHC_NCELL_SB_READ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_RXLEV_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | RXLEV_REQ                |     power_array_size: #@1d#"
   //COND/
    "#@Fdl7#  RXLEV_REQ                                                      nb_rf: #@1d#"
   End header */
//ID/
#define TRL1_MPHC_RXLEV_REQ  15
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          power_array_size;
}
T_TR_MPHC_RXLEV_REQ;

#define MAX_MEAS 10

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_VALID_MEAS_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |          VALID_MEAS_INFO |     power array size: #@1dl10#      rxlev_req_count: #@2dl#"
   //COND/
    "#@Fdl7#                          VALID_MEAS_INFO"
   End header */
//ID/*/
#define TRL1_L1C_VALID_MEAS_INFO  16
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          power_array_size;
  UWORD8           rxlev_req_count;
}
T_TR_L1C_VALID_MEAS_INFO;

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define TRL1_FULL_LIST_REPORT  184

typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          size;
  UWORD32          content[1];
}
T_TR_FULL_LIST_REPORT;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_RXLEV_PERIODIC_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |      RXLEV_PERIODIC_DONE |                ba_id: #@2dl10#              s_rxlev: #@3d#"
    "        |  |  |  |  |                          |           radio_freq: #@1dr5#"
    "        |  |  |  |  |                          |                rxlev: #@4dr5#"
   //COND/
    "#@Fdl7#                          RXLEV_PERIODIC_DONE"
   End header */
//ID/
#define TRL1_L1C_RXLEV_PERIODIC_DONE 17
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq_no[8];
  UWORD8           ba_id;
  WORD8            s_rxlev;
  WORD8            rxlev[8];
}
T_TR_L1C_RXLEV_PERIODIC_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_SCELL_NBCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SCELL_NBCCH_REQ          |  schedule_array_size: #@3d#"
    "        |  |  |  |  |                          |              modulus: #@1dr5#"
    "        |  |  |  |  |                          |    relative_position: #@2dr5#"
   //COND/
    "#@Fdl7#  SCELL_NBCCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_SCELL_NBCCH_REQ 18
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          modulus[10];
  UWORD16          relative_position[10];
  UWORD8           schedule_array_size;
}
T_TR_MPHC_SCELL_NBCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_SCELL_EBCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SCELL_EBCCH_REQ          |  schedule_array_size: #@3d#"
    "        |  |  |  |  |                          |              modulus: #@1dr5#"
    "        |  |  |  |  |                          |    relative_position: #@2dr5#"
   //COND/
    "#@Fdl7#  SCELL_EBCCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_SCELL_EBCCH_REQ 19
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          modulus[10];
  UWORD16          relative_position[10];
  UWORD8           schedule_array_size;
}
T_TR_MPHC_SCELL_EBCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_BCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | NCELL_BCCH_REQ           |           radio_freq: #@3dl10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dl10#        bcch_blks_req: #@4d#"
    "        |  |  |  |  |                          |                  tsc: #@5dl10#        gprs_priority: #@6=0:TOP|=1:HIGH|=2:NORMAL|#"
   //COND/
    "#@Fdl7#  NCELL_BCCH_REQ                                                 radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_BCCH_REQ 20
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
  UWORD16          bcch_blks_req;
  UWORD8           tsc;
  UWORD8           gprs_priority;
}
T_TR_MPHC_NCELL_BCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_BCCHN_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               BCCHN_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          BCCHN_INFO              #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_BCCHN_INFO 21
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_BCCHN_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_NP_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                  NP_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          NP_INFO                 #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_NP_INFO 22
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_NP_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_EP_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                  EP_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          EP_INFO                 #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_EP_INFO 23
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_EP_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_ALLC_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                ALLC_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          ALLC_INFO               #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_ALLC_INFO 24
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_ALLC_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_BCCHS_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               BCCHS_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          BCCHS_INFO              #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_BCCHS_INFO 25
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_BCCHS_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_CB_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                  CB_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |          input_level: #-@5 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          CB_INFO                 #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_CB_INFO 26
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           input_level;
}
T_TR_L1C_CB_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NETWORK_SYNC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | NETWORK_SYNC_REQ         |           radio_freq: #@3dl10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dl10#      timing_validity: #@4d#"
    "        |  |  |  |  |                          |          search_mode: #@5d#"
   //COND/
    "#@Fdl7#  NETWORK_SYNC_REQ                                               radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NETWORK_SYNC_REQ 27
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
  UWORD8           timing_validity;
  UWORD8           search_mode;
}
T_TR_MPHC_NETWORK_SYNC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NETWORK_SYNC_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  |         NETWORK_SYNC_IND |              sb_flag: #@4=1:OK|=0:FAILED|~|l10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dl10#           radio_freq: #@3d#"
    "        |  |  |  |  |                          |                 bsic: #@5d#"
   //COND/
    "#@Fdl7#  NETWORK_SYNC_IND                                #@4=0:Syncho failed|=1:               Synchro done|#"
   End header */
//ID/
#define TRL1_MPHC_NETWORK_SYNC_IND 28
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
  BOOL             sb_flag;
  UWORD8           bsic;
}
T_TR_MPHC_NETWORK_SYNC_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_SYNC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | NCELL_SYNC_REQ           |           radio_freq: #@3dl10#      timing_validity: #@4d#"
    "        |  |  |  |  |                          |            fn_offset: #@1dl10#         time_alignmt: #@2d#"
   //COND/
    "#@Fdl7#  NCELL_SYNC_REQ                                                 radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_SYNC_REQ 29
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
  UWORD8           timing_validity;
}
T_TR_MPHC_NCELL_SYNC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_LIST_SYNC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MPHC_NCELL_LIST_SYNC_REQ |                 eotd: #@5=0:FALSE|=1:TRUE|l10#            list_size: #@6d#"
    "        |  |  |  |  |                          |           radio_freq: #@3dr10#"
    "        |  |  |  |  |                          |      timing_validity: #@4dr10#"
    "        |  |  |  |  |                          |            fn_offset: #@1dr10#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dr10#"
   //COND/
    "#@Fdl7#  MPHC_NCELL_LIST_SYNC_REQ                                       eotd: #@5dl10# list_size: #@6d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_LIST_SYNC_REQ 217
//STRUCT/
typedef struct
{
  UWORD32    header;
//--------------------------------------------------
  UWORD32    fn_offset[12];
  UWORD32    time_alignmt[12];
  UWORD16    radio_freq[12];
  UWORD8     timing_validity[12];
  UWORD8     eotd;
  UWORD8     list_size;
}
T_TR_MPHC_NCELL_LIST_SYNC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NCELL_SYNC_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  |           NCELL_SYNC_IND |              sb_flag: #@9=0:Not Found|=1:Found|~|l10#           radio_freq: #@8d#"
    "        |  |  |  |  |                          |                 bsic: #@10dl10#             neigh_id: #@11d#"
    "        |  |  |  |  |                          |            fn_offset: #@1dl10#         time_alignmt: #@2d#"
    "        |  |  |  |  |                          |            list_size: #@12dl10#          fn_sb_neigh: #@3d#"
    "        |  |  |  |  |                          |            fn_in_SB: #@4dl10#        toa_correction: #@5d#"
    "        |  |  |  |  |                          |            delta_fn: #@6dl10#            delta_qbit: #@7d#"
    "        |  |  |  |  |                          |     eotd_data_valid: #@13dl10#                  mode: #@14d#"
   //COND/
    "#@Fdl7#  NCELL_SYNC_IND                                  #@4=0:not found|~|l10#     radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NCELL_SYNC_IND 30
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD32          fn_sb_neigh;
  UWORD32          fn_in_SB;
  WORD32           toa_correction;
  UWORD32          delta_fn;
  WORD32           delta_qbit;
  UWORD16          radio_freq;
  BOOL             sb_flag;
  UWORD8           bsic;
  UWORD8           neigh_id;
  UWORD8           list_size;
  UWORD8           eotd_data_valid;
  UWORD8           mode;
}
T_TR_MPHC_NCELL_SYNC_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_SB_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                  SB_INFO |              sb_flag: #@10=0:Not found|=1:Found|~|l10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dl10#                   pm: #@3d#"
    "        |  |  |  |  |                          |                  toa: #@4dl10#                angle: #@5d#"
    "        |  |  |  |  |                          |                  snr: #@6dl10#           tpu_offset: #@7d#"
    "        |  |  |  |  |                          |           radio_freq: #@8dl10#                  afc: #@9d#"
    "        |  |  |  |  |                          |                 bsic: #@11dl10#          input_level: #-@12 / 2d# dBm"
   //COND/
    "#@Fdl7#                          SB_INFO                 #@10=0:not found|~|#"
   End header */
//ID/
#define TRL1_L1C_SB_INFO 31
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD32          pm;
  UWORD32          toa;
  UWORD32          angle;
  UWORD32          snr;
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             sb_flag;
  UWORD8           bsic;
  UWORD8           input_level;
}
T_TR_L1C_SB_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_SBCONF_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |              SBCONF_INFO |              sb_flag: #@10=0:Not found|=1:Found|~|l10#            fn_offset: #@1d#"
    "        |  |  |  |  |                          |         time_alignmt: #@2dl10#                   pm: #@3d#"
    "        |  |  |  |  |                          |                  toa: #@4dl10#                angle: #@5d#"
    "        |  |  |  |  |                          |                  snr: #@6dl10#           tpu_offset: #@7d#"
    "        |  |  |  |  |                          |           radio_freq: #@8dl10#                  afc: #@9d#"
    "        |  |  |  |  |                          |                 bsic: #@11dl10#          input_level: #-@12 / 2d# dBm"
   //COND/
    "#@Fdl7#                          SBCONF_INFO             #@10=0:not found|~|#"
   End header */
//ID/
#define TRL1_L1C_SBCONF_INFO 32
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD32          pm;
  UWORD32          toa;
  UWORD32          angle;
  UWORD32          snr;
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             sb_flag;
  UWORD8           bsic;
  UWORD8           input_level;
}
T_TR_L1C_SBCONF_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NEW_SCELL_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | NEW_SCELL_REQ            |            fn_offset: #@1dl10#         time_alignmt: #@2d#"
    "        |  |  |  |  |                          |           radio_freq: #@3dl10#                 bsic: #@4d#"
   //COND/
    ""
    ""
    "#@Fdl7#  NEW_SCELL_REQ                                                  radio_freq: #@3d#"
   End header */
//ID/
#define TRL1_MPHC_NEW_SCELL_REQ 33
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignmt;
  UWORD16          radio_freq;
  UWORD8           bsic;
}
T_TR_MPHC_NEW_SCELL_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_FB_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                  FB_INFO |              fb_flag: #@8=0:Not found|=1:Found|~|l10#                   pm: #@1d#"
    "        |  |  |  |  |                          |                  toa: #@2dl10#                angle: #@3d#"
    "        |  |  |  |  |                          |                  snr: #@4dl10#           tpu_offset: #@5d#"
    "        |  |  |  |  |                          |           radio_freq: #@6dl10#                  afc: #@7d#"
    "        |  |  |  |  |                          |          input_level: #-@9 / 2d# dBm"
   //COND/
    "#@Fdl7#                          FB_INFO                 #@8=0:not found|~|#"
   End header */
//ID/
#define TRL1_L1C_FB_INFO 34
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          pm;
  UWORD32          toa;
  UWORD32          angle;
  UWORD32          snr;
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             fb_flag;
  UWORD8           input_level;
}
T_TR_L1C_FB_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_NCELL_SYNC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | STOP_NCELL_SYNC_REQ      | radio_freq_array_size: #@2d#"
    "        |  |  |  |  |                          |      radio_freq_array: #@1dr5#"
   //COND/
    "#@Fdl7#  STOP_NCELL_SYNC_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_NCELL_SYNC_REQ 35
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq_array[6];
  WORD8            radio_freq_array_size;
}
T_TR_MPHC_STOP_NCELL_SYNC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_NCELL_BCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | STOP_NCELL_BCCH_REQ      | radio_freq_array_size: #@2d#"
    "        |  |  |  |  |                          |      radio_freq_array: #@1dr5#"
   //COND/
    "#@Fdl7#  STOP_NCELL_BCCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_NCELL_BCCH_REQ 36
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq_array[6];
  UWORD8           radio_freq_array_size;
}
T_TR_MPHC_STOP_NCELL_BCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CONFIG_CBCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CONFIG_CBCH_REQ          |           radio_freq: #@1dl10#                    h: #@2=0:Single RF|=1: Hopping RF|#"
    "        |  |  |  |  |                          |          timeslot_no: #@3d#"
   //COND/
    "#@Fdl7#  CONFIG_CBCH_REQ                                                radio_freq: #@1d#"
   End header */
//ID/
#define TRL1_MPHC_CONFIG_CBCH_REQ 37
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  BOOL             h;
  UWORD8           timeslot_no;
}
T_TR_MPHC_CONFIG_CBCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CBCH_SCHEDULE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CBCH_SCHEDULE_REQ        |        extended_cbch: #@3=0:NORMAL|=1:EXTENDED|~|l10#      schedule_length: #@4d#"
    "        |  |  |  |  |                          |        first_block_0: #@1xl10#        first_block_1: #@2x#"
    "        |  |  |  |  |                          |         "
   //COND/
    "#@Fdl7#  CBCH_SCHEDULE_REQ                                              #@3=0:NORMAL|=1:EXTENDED|#"
   End header */
//ID/
#define TRL1_MPHC_CBCH_SCHEDULE_REQ 38
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          first_block_0;
  UWORD16          first_block_1;
  BOOL             extended_cbch;
  UWORD8           schedule_length;
}
T_TR_MPHC_CBCH_SCHEDULE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CBCH_INFO_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CBCH_INFO_REQ            |            tb_bitmap: #@1bz8#"
   //COND/
    "#@Fdl7#  CBCH_INFO_REQ"
   End header */
//ID/
#define TRL1_MPHC_CBCH_INFO_REQ 39
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           tb_bitmap;
}
T_TR_MPHC_CBCH_INFO_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CBCH_UPDATE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CBCH_UPDATE_REQ          |        extended_cbch: #@3=0:NORMAL|=1:EXTENDED|#"
    "        |  |  |  |  |                          |        first_block_0: #@1xl10#        first_block_1: #@2x#"
   //COND/
    "#@Fdl7#  CBCH_UPDATE_REQ                                                #@3=0:NORMAL|=1:EXTENDED|#"
   End header */
//ID/
#define TRL1_MPHC_CBCH_UPDATE_REQ 40
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          first_block_0;
  UWORD16          first_block_1;
  BOOL             extended_cbch;
}
T_TR_MPHC_CBCH_UPDATE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_CBCH_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | STOP_CBCH_REQ            |          normal_cbch: #@1=0:No|=1:Yes|~|l10#         extended_cbch: #@2=0:No|=1:Yes|#"
   //COND/
    "#@Fdl7#  STOP_CBCH_REQ                                                  #@1=1:NORMAL|~|##@2=1:EXTENDED|~|#"
   End header */
//ID/
#define TRL1_MPHC_STOP_CBCH_REQ 41
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  BOOL             normal_cbch;
  BOOL             extended_cbch;
}
T_TR_MPHC_STOP_CBCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_SACCH_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               SACCH_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |   beacon_input_level: #-@5/2 f1l6# dBm          input_level: #-@6/2f1# dBm"
   //COND/
    "#@Fdl7#                          SACCH_INFO              #@5=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1C_SACCH_INFO 42
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          rf_chan_num;
  WORD16           afc;
  UWORD8           error_cause;
  UWORD8           beacon_input_level;
  UWORD8           input_level;
}
T_TR_L1C_SACCH_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CHANGE_FREQUENCY
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CHANGE_FREQUENCY         |           radio_freq: #@1dl10#          rf_chan_cnt: #@2d#"
    "        |  |  |  |  |                          |                    h: #@3=0:Single RF|=1: Hopping RF|~|l10#         channel_type: #@4T[CHAN TYPE]#"
    "        |  |  |  |  |                          |           subchannel: #@5dl10#          timeslot_no: #@6d#"
    "        |  |  |  |  |                          |                  tsc: #@7dl10#   start_time_present: #@8=0:No|=1:Yes|#"
    "        |  |  |  |  |                          |        starting_time: # (26 + @10 - @11) % 26 + @10 + (1326 * @9 * 51)dl10#"
   //COND/
    "#@Fdl7#  CHANGE_FREQUENCY                                               radio_freq: #@1d#"
   End header */
//ID/
#define TRL1_MPHC_CHANGE_FREQUENCY 43
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  BOOL             h;
  UWORD8           channel_type;
  UWORD8           subchannel;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  BOOL             start_time_present;
  UWORD8           n32;
  UWORD8           n51;
  UWORD8           n26;
}
T_TR_MPHC_CHANGE_FREQUENCY;


/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CHANNEL_MODE_MODIFY_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CHANNEL_MODE_MODIFY_REQ  |           subchannel: #@1dl10#         channel_mode: #@2=0:SIG only|=1:TCH_FS|=2:TCH_HS|=3:TCH_96|=4:TCH_48F|=5:TCH_48H|=6:TCH_24F|=7:TCH_24H|=8:TCH_EFR|=9:TCH_144|=10:TCH_AHS|=11:TCH_AFS|#"
    "        |  |  |  |  |                          |      noise_suppr_bit: #@3dl10#   initial_codec_mode: #@4d#"
    "        |  |  |  |  |                          |   initial_codec_mode: #@5dl10#     active_codec_set: #@6d#"
    "        |  |  |  |  |                          |            threshold: #@7dr3#"
    "        |  |  |  |  |                          |           hysteresis: #@8dr3#"
   //COND/
    "#@Fdl7#  CHANNEL_MODE_MODIFY_REQ                                        #@2=0:SIG only|=1:TCH_FS|=2:TCH_HS|=3:TCH_96|=4:TCH_48F|=5:TCH_48H|=6:TCH_24F|=7:TCH_24H|=8:TCH_EFR|=9:TCH_144|=10:TCH_AHS|=11:TCH_AFS|#"
   End header */
//ID/
#define TRL1_MPHC_CHANNEL_MODE_MODIFY_REQ 44
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           subchannel;
  UWORD8           channel_mode;
  BOOL             noise_suppression_bit;
  BOOL             initial_codec_mode_indicator;
  UWORD8           initial_codec_mode;
  UWORD8           active_codec_set;
  UWORD8           threshold[3];
  UWORD8           hysteresis[3];
}
T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_SET_CIPHERING_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SET_CIPHERING_REQ        |          cipher_mode: #@1=0:No ciphering|~ON|l10#         a5_algorithm: #@2=0:A5/1|=1:A5/2|=2:A5/3|=3:A5/4|=4:A5/5|=5:A5/6|=6:A5/7|#"
    "        |  |  |  |  |                          |                    A: #@3dr5#"
   //COND/
    "#@Fdl7#  SET_CIPHERING_REQ                                              #@1=0:No ciphering|~Ciphering on|#"
   End header */
//ID/
#define TRL1_MPHC_SET_CIPHERING_REQ 45
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           cipher_mode;
  UWORD8           a5_algorithm;
  UWORD8           A[8];
}
T_TR_MPHC_SET_CIPHERING_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_UPDATE_BA_LIST
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | UPDATE_BA_LIST           |         num_of_chans: #@1dl10#                 pwrc: #@2d#"
    "        |  |  |  |  |                          |          dtx_allowed: #@3=0:NO|=1:YES|~|l10#                ba_id: #@4d#"
   //COND/
    "#@Fdl7#  UPDATE_BA_LIST"
   End header */
//ID/
#define TRL1_MPHC_UPDATE_BA_LIST 46
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           num_of_chans;
  BOOL             pwrc;
  BOOL             dtx_allowed;
  UWORD8           ba_id;
}
T_TR_MPHC_UPDATE_BA_LIST;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_NETWORK_LOST_IND
   //FULL/
	"        |  |  |  |  |                          |"
	"#@Fdl7# |<----|  |  | NETWORK_LOST_IND         |"
   //COND/
    "#@Fdl7#  NETWORK_LOST_IND"
   End header */
//ID/
#define TRL1_MPHC_NETWORK_LOST_IND 47
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_NETWORK_LOST_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_CCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_CCCH_REQ            |"
   //COND/
    "#@Fdl7#  STOP_CCCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_CCCH_REQ 48
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_CCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_SCELL_BCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_SCELL_BCCH_REQ      |"
   //COND/
    "#@Fdl7#  STOP_SCELL_BCCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_SCELL_BCCH_REQ 49
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_SCELL_BCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_CBCH_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_CBCH_REQ            |"
   //COND/
    "#@Fdl7#  STOP_CBCH_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_CBCH_CON 50
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_CBCH_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_RA_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_RA_REQ              |"
   //COND/
   End header */
//ID/
#define TRL1_MPHC_STOP_RA_REQ 51
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_RA_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_RA_DONE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |              L1C_RA_DONE |"
   //COND/
    "#@Fdl7#                          L1C_RA_DONE"
   End header */
//ID/
#define TRL1_L1C_RA_DONE 52
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1C_RA_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_IMMED_ASSIGN_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "=========================================================================================================================================================================================="
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |         IMMED_ASSIGN_CON |"
   //COND/
    ""
    ""
    "#@Fdl7#  IMMED_ASSIGN_CON"
   End header */
//ID/
#define TRL1_MPHC_IMMED_ASSIGN_CON 53
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_IMMED_ASSIGN_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_CHANNEL_ASSIGN_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "=========================================================================================================================================================================================="
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |       CHANNEL_ASSIGN_CON |"
   //COND/
    ""
    ""
    "#@Fdl7#  CHANNEL_ASSIGN_CON"
   End header */
//ID/
#define TRL1_MPHC_CHANNEL_ASSIGN_CON 54
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_CHANNEL_ASSIGN_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_REDEF_DONE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |               REDEF_DONE |"
   //COND/
    "#@Fdl7#                          REDEF_DONE"
   End header */
//ID/
#define TRL1_L1C_REDEF_DONE 55
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1C_REDEF_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_DEDICATED_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_DEDICATED_REQ       |"
   //COND/
    "#@Fdl7#  STOP_DEDICATED_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_DEDICATED_REQ 56
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_DEDICATED_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_ASYNC_HO_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |             ASYNC_HO_CON |"
   //COND/
    "#@Fdl7#  ASYNC_HO_CON"
   End header */
//ID/
#define TRL1_MPHC_ASYNC_HO_CON 57
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_ASYNC_HO_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_SYNC_HO_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |              SYNC_HO_CON |"
   //COND/
    "#@Fdl7#  SYNC_HO_CON"
   End header */
//ID/
#define TRL1_MPHC_SYNC_HO_CON 58
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_SYNC_HO_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_TA_FAIL_IND
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |              TA_FAIL_IND |"
   //COND/
    "#@Fdl7#  TA_FAIL_IND"
   End header */
//ID/
#define TRL1_MPHC_TA_FAIL_IND 59
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_TA_FAIL_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_HANDOVER_FAIL_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | HANDOVER_FAIL_REQ        |"
   //COND/
    "#@Fdl7#  HANDOVER_FAIL_REQ"
   End header */
//ID/
#define TRL1_MPHC_HANDOVER_FAIL_REQ 60
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_HANDOVER_FAIL_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_HANDOVER_FAIL_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |        HANDOVER_FAIL_CON |"
   //COND/
    "#@Fdl7#  HANDOVER_FAIL_CON"
   End header */
//ID/
#define TRL1_MPHC_HANDOVER_FAIL_CON 61
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_HANDOVER_FAIL_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_RXLEV_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_RXLEV_REQ           |"
   //COND/
    "#@Fdl7#  STOP_RXLEV_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_RXLEV_REQ 62
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_RXLEV_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_RXLEV_PERIODIC_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_RXLEV_PERIODIC_REQ  |"
   //COND/
    "#@Fdl7#  STOP_RXLEV_PERIODIC_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_RXLEV_PERIODIC_REQ 63
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_RXLEV_PERIODIC_REQ;

///////////////////
// GPRS messages //
///////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_RA_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MPHP_RA_REQ              |                 rand: #@1dl10# channel_request_data: #@2d#"
    "        |  |  |  |  |                          |                txpwr: #@3dl10#        bs_prach_blks: #@4d#"
    "        |  |  |  |  |                          |    access_burst_type: #@5=0: 8 bit|=1:11 bit|#"
   //COND/
    "#@Fdl7#  MPHP_RA_REQ"
   End header */
//ID/
#define TRL1_MPHP_RA_REQ 64
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          rand;
  UWORD16          channel_request_data;
  UWORD8           txpwr;
  UWORD8           bs_prach_blks;
  UWORD8           access_burst_type;
}
T_TR_MPHP_RA_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_RA_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |              L1P_RA_DONE | channel_request_data: #@1d#"
   //COND/
    "#@Fdl7#                          L1P_RA_DONE"
   End header */
//ID/
#define TRL1_L1P_RA_DONE 65
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          channel_request_data;
}
T_TR_L1P_RA_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_POLLING_RESPONSE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | POLLING_RESPONSE_REQ     |               fn_req: #@1dl10#        pol_resp_type: #@2=3:CS1|=7:PRACH 8 bit|=8:PRACH 11 bit|#"
    "        |  |  |  |  |                          |       timing_advance: #@3dl10#                txpwr: #@4d#"
   //COND/
    "#@Fdl7#  POLLING_RESPONSE_REQ"
   End header */
//ID/
#define TRL1_MPHP_POLLING_RESPONSE_REQ 66
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_req;
  UWORD8           pol_resp_type;
  UWORD8           timing_advance;
  UWORD8           txpwr;
}
T_TR_MPHP_POLLING_RESPONSE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_POLL_DONE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |                POLL_DONE |"
   //COND/
    "#@Fdl7#                          POLL_DONE"
   End header */
//ID/
#define TRL1_L1P_POLL_DONE 67
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1P_POLL_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_ASSIGNMENT_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | ASSIGNMENT_REQ           |              tbf_sti: #@1<0:Not present|l10#           radio_freq: #@2d#"
    "        |  |  |  |  |                          |          rf_chan_cnt: #@3dl10#        assignment_id: #@4d#"
    "        |  |  |  |  |                          |   assignment_command: #@5=0:DL TBF|=1:UL TBF|=2:BOTH TBF|~|l10#      multislot_class: #@6d#"
    "        |  |  |  |  |                          |   interf_meas_enable: #@7=0:NO|=1:YES|~|l10#         pc_meas_chan: #@8=0:BCCH|=1:PDTCH|#"
    "        |  |  |  |  |                          |    access_burst_type: #@9=0:8 bit|=1:11 bit|~|l10#                   ta: #@10d#"
    "        |  |  |  |  |                          |             ta_index: #@11dl10#                ta_tn: #@12d#"
    "        |  |  |  |  |                          |     bts_pwr_ctl_mode: #@14=0:Mode A|=1:Mode B|~|l10#                   p0: #@13 * 2=510:Constant output power mode|#"
    "        |  |  |  |  |                          |              pr_mode: #@15=0:Mode A|=1:Mode B|~|l10#                  tsc: #@16d#"
    "        |  |  |  |  |                          |                    h: #@17=0:Single RF|=1:Hopping RF|~|l10#             mac_mode: #@18=0:Dynamic allocation|=1:Extended dynamic|=2:Fixed allocation|=3:Fixed allocation Half Duplex|#"
    "        |  |  |  |  |                          |   dl_ressource_alloc: #@19bz8#b"
    "        |  |  |  |  |                          |   ul_ressource_alloc: #@20bz8#b"
    "        |  |  |  |  |                          |      usf_granularity: #@21=0:1 block|=1:4 blocks|#"
    "        |  |  |  |  |                          |        ctrl_timeslot: #@22dl10#        bitmap_length: #@23d#"
   //COND/
    ""
    ""
    "#@Fdl7#  ASSIGNMENT_REQ                                                 #@5=0:DL TBF|=1:UL TBF|=2:BOTH TBF|#"
   End header */
//ID/
#define TRL1_MPHP_ASSIGNMENT_REQ 68
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD32           tbf_sti;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD8           assignment_id;
  UWORD8           assignment_command;
  UWORD8           multislot_class;
  BOOL             interf_meas_enable;
  BOOL             pc_meas_chan;
  BOOL             access_burst_type;
  UWORD8           ta;
  UWORD8           ta_index;
  UWORD8           ta_tn;
  UWORD8           p0;
  BOOL             bts_pwr_ctl_mode;
  BOOL             pr_mode;
  UWORD8           tsc;
  BOOL             h;
  UWORD8           mac_mode;
  UWORD8           dl_ressource_alloc;
  UWORD8           ul_ressource_alloc;
  BOOL             usf_granularity;
  UWORD8           ctrl_timeslot;
  UWORD8           bitmap_length;
}
T_TR_MPHP_ASSIGNMENT_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_REPEAT_UL_FIXED_ALLOC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | REPEAT_UL_FIXED_ALLOC    |              tbf_sti: #@1dl10#    repeat_allocation: #@2=0:CANCEL|=1:REPEAT|#"
    "        |  |  |  |  |                    _REQ  |          ts_override: #@3x#"
   //COND/
    "#@Fdl7#  REPEAT_UL_FIXED_ALLOC_REQ                                      #@2=0:CANCEL|=1:REPEAT|~|#"
   End header */
//ID/
#define TRL1_MPHP_REPEAT_UL_FIXED_ALLOC_REQ 69
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD32           tbf_sti;
  BOOL             repeat_allocation;
  UWORD8           ts_override;
}
T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_REPEAT_ALLOC_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |        REPEAT_ALLOC_DONE |                dl_tn: #@1d#"
   //COND/
    "#@Fdl7#                          REPEAT_ALLOC_DONE                      dl_tn: #@1d#"
   End header */
//ID/
#define TRL1_L1P_REPEAT_ALLOC_DONE  70
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           dl_tn;
}
T_TR_L1P_REPEAT_ALLOC_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_ALLOC_EXHAUST_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |       ALLOC_EXHAUST_DONE |                dl_tn: #@1d#"
   //COND/
    "#@Fdl7#                          ALLOC_EXHAUST_DONE                     dl_tn: #@1d#"
   End header */
//ID/
#define TRL1_L1P_ALLOC_EXHAUST_DONE 71
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           dl_tn;
}
T_TR_L1P_ALLOC_EXHAUST_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_SINGLE_BLOCK_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SINGLE_BLOCK_REQ         |              tbf_sti: #@1<0:Not present|l10#           radio_freq: #@2d#"
    "        |  |  |  |  |                          |          rf_chan_cnt: #@3dl10#        assignment_id: #@4d#"
    "        |  |  |  |  |                          |              purpose: #@5=3:DL block|=4:UL block|=5:Two phase access|l16#   pc_meas_chan: #@6=0:BCCH|=1:PDTCH|#"
    "        |  |  |  |  |                          |    access_burst_type: #@7=0:8 bit|=1:11 bit|~|l10#                   ta: #@8d#"
    "        |  |  |  |  |                          |     bts_pwr_ctl_mode: #@10=0:Mode A|=1:Mode B|~|l10#                   p0: #@9 * 2=510:Constant output power mode|#"
    "        |  |  |  |  |                          |              pr_mode: #@11=0:Mode A|=1:Mode B|~|l10#                  tsc: #@12d#"
    "        |  |  |  |  |                          |                    h: #@13=0:Single RF|=1:Hopping RF|~|l10#      timeslot_number: #@14d#"
   //COND/
    ""
    ""
    "#@Fdl7#  SINGLE_BLOCK_REQ                                               #@5=3:DL blk|=4:UL blk|=5:Two phase|#"
   End header */
//ID/
#define TRL1_MPHP_SINGLE_BLOCK_REQ 72
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD32           tbf_sti;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD8           assignment_id;
  UWORD8           purpose;
  BOOL             pc_meas_chan;
  BOOL             access_burst_type;
  UWORD8           ta;
  UWORD8           p0;
  BOOL             bts_pwr_ctl_mode;
  BOOL             pr_mode;
  UWORD8           tsc;
  BOOL             h;
  UWORD8           timeslot_number;
}
T_TR_MPHP_SINGLE_BLOCK_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_SINGLE_BLOCK_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |         SINGLE_BLOCK_CON |        assignment_id: #@1dl10#              purpose: #@2=3:DL block|=4:UL block|=5:Two phase access|l10#"
    "        |  |  |  |  |                          |               status: #@3=0:no error|=1:STI passed|=2:No valid TA|=3:CRC ERROR|l10#        dl_error_flag: #@4=0:No error|=1:CRC ERROR|#"
    "        |  |  |  |  |                          |                txpwr: #@5dr5#"
   //COND/
    "#@Fdl7#                          SINGLE_BLOCK_CON        #@3=1:STI passed|=2:No valid TA|=3:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_SINGLE_BLOCK_CON 73
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           assignment_id;
  UWORD8           purpose;
  UWORD8           status;
  BOOL             dl_error_flag;
  UWORD8           txpwr[4];
}
T_TR_L1P_SINGLE_BLOCK_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_PDCH_RELEASE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | PDCH_RELEASE_REQ         |        assignment_id: #@1dl10#   timeslot_available: #@2x#"
   //COND/
    "#@Fdl7#  PDCH_RELEASE_REQ                                               timeslot_available: #@2x#"
   End header */
//ID/
#define TRL1_MPHP_PDCH_RELEASE_REQ 74
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           assignment_id;
  UWORD8           timeslot_available;
}
T_TR_MPHP_PDCH_RELEASE_REQ;


/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_TIMING_ADVANCE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | TIMING_ADVANCE_REQ       |        assignment_id: #@1dl10#                   ta: #@2d#"
    "        |  |  |  |  |                          |             ta_index: #@3dl10#                ta_tn: #@4d#"
   //COND/
    "#@Fdl7#  TIMING_ADVANCE_REQ"
   End header */
//ID/
#define TRL1_MPHP_TIMING_ADVANCE_REQ 75
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           assignment_id;
  UWORD8           ta;
  UWORD8           ta_index;
  UWORD8           ta_tn;
}
T_TR_MPHP_TIMING_ADVANCE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_TBF_RELEASE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | TBF_RELEASE_REQ          |             tbf_type: #@1=0:DL TBF|=1:UL TBF|=2:BOTH TBF|#"
   //COND/
    "#@Fdl7#  TBF_RELEASE_REQ                                                tbf_type: #@1=0:DL TBF|=1:UL TBF|=2:BOTH TBF|#"
   End header */
//ID/
#define TRL1_MPHP_TBF_RELEASE_REQ 76
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           tbf_type;
}
T_TR_MPHP_TBF_RELEASE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_START_PCCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | START_PCCCH_REQ          |              imsimod: #@1dl10#                  kcn: #@2d#"
    "        |  |  |  |  |                          |       split_pg_cycle: #@3dl10#           radio_freq: #@4d#"
    "        |  |  |  |  |                          |          rf_chan_cnt: #@5dl10#      bs_pag_blks_res: #@6d#"
    "        |  |  |  |  |                          |        bs_pbcch_blks: #@7dl10#                   pb: - #@8*2d# dBm"
    "        |  |  |  |  |                          |            page_mode: #@9=0:Normal|=1:Extended|=2:Reorg|l10#                    h: #@10=0:Single RF|=1:Hopping RF|~|l10#"
    "        |  |  |  |  |                          |          timeslot_no: #@11dl10#                  tsc: #@12d#"
   //COND/
    "#@Fdl7#  START_PCCCH_REQ                                                #@9=0:Normal|=1:Extended|=2:Reorg|~|#"
   End header */
//ID/
#define TRL1_MPHP_START_PCCCH_REQ 77
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          imsimod;
  UWORD16          kcn;
  UWORD16          split_pg_cycle;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD8           bs_pag_blks_res;
  UWORD8           bs_pbcch_blks;
  UWORD8           pb;
  UWORD8           page_mode;
  BOOL             h;
  UWORD8           timeslot_no;
  UWORD8           tsc;
}
T_TR_MPHP_START_PCCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PBCCHN_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |              PBCCHN_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PBCCHN_INFO             #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PBCCHN_INFO 78
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PBCCHN_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PNP_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                 PNP_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PNP_INFO                #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PNP_INFO 79
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PNP_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PEP_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |                 PEP_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PEP_INFO                #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PEP_INFO 80
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PEP_INFO;


/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PALLC_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               PALLC_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PALLC_INFO              #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PALLC_INFO 81
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PALLC_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PBCCHS_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |              PBCCHS_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PBCCHS_INFO             #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PBCCHS_INFO 82
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PBCCHS_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PACCH_INFO
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               PACCH_INFO |           error_flag: #@4=0:OK|=1:CRC ERROR|l10#           tpu_offset: #@1d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dl10#                  afc: #@3d#"
    "        |  |  |  |  |                          |    relative_position: #@5dl10#          input_level: #-@6 / 2f1# dBm"
   //COND/
    "#@Fdl7#                          PACCH_INFO              #@4=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_L1P_PACCH_INFO 83
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq;
  WORD16           afc;
  BOOL             error_flag;
  UWORD8           relative_position;
  UWORD8           input_level;
}
T_TR_L1P_PACCH_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_SCELL_PBCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SCELL_PBCCH_REQ          |           radio_freq: #@1dl10#          rf_chan_cnt: #@2d#"
    "        |  |  |  |  |                          |              nbr_psi: #@3dl10#        bs_pbcch_blks: #@4d#"
    "        |  |  |  |  |                          |                   pb: - #@5*2dl5# dBm  psi1_repeat_period: #@6d#"
    "        |  |  |  |  |                          |                    h: #@7=0:Single RF|=1:Hopping RF|~|l10#          timeslot_no: #@8dl10#"
    "        |  |  |  |  |                          |                  tsc: #@9d#"
    "        |  |  |  |  |                          |    relative_position: #@10dr5#"
   //COND/
    "#@Fdl7#  SCELL_PBCCH_REQ                                                nbr_psi: #@3dl10#"
   End header */
//ID/
#define TRL1_MPHP_SCELL_PBCCH_REQ 84
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD8           nbr_psi;
  UWORD8           bs_pbcch_blks;
  UWORD8           pb;
  UWORD8           psi1_repeat_period;
  BOOL             h;
  UWORD8           timeslot_no;
  UWORD8           tsc;
  UWORD8           relative_position_array[20];
}
T_TR_MPHP_SCELL_PBCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_CR_MEAS_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | CR_MEAS_REQ              |           nb_carrier: #@1dl10#              list_id: #@2d#"
   //COND/
    "#@Fdl7#  CR_MEAS_REQ"
   End header */
//ID/
#define TRL1_MPHP_CR_MEAS_REQ 85
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           nb_carrier;
  UWORD8           list_id;
}
T_TR_MPHP_CR_MEAS_REQ;

#define MAX_CR 20


/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_CR_MEAS_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |             CR_MEAS_DONE |                nmeas: #@3dl10#              list_id: #@4d#"
    "        |  |  |  |  |                          |     reporting_period: #@1d#"
    "        |  |  |  |  |                          |                 freq: #@2dr5#"
    "        |  |  |  |  |                          |                rxlev: #@5dr5#"
   //COND/
    "#@Fdl7#                          CR_MEAS_DONE"
   End header */
//ID/
#define TRL1_L1P_CR_MEAS_DONE 86
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          reporting_period;
  UWORD16          freq[20];
  UWORD8           nmeas;
  UWORD8           list_id;
  WORD8            rxlev[20];
}
T_TR_L1P_CR_MEAS_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_INT_MEAS_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | INT_MEAS_REQ             |           radio_freq: #@1dl10#               rf_chan_cnt: #@2d#"
    "        |  |  |  |  |                          |                    h: #@3=0:Single RF|=1:Hopping RF|~|l10#               tn: #@4d#"
    "        |  |  |  |  |                          |      multislot_class: #@5dl10#"
   //COND/
    "#@Fdl7#  INT_MEAS_REQ"
   End header */
//ID/
#define TRL1_MPHP_INT_MEAS_REQ 87
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  BOOL             h;
  UWORD8           tn;
  UWORD8           multislot_class;
}
T_TR_MPHP_INT_MEAS_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_INT_MEAS_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | INT_MEAS_IND             |                rxlev: #@1dr5#"
    "        |  |  |  |  |                          |                       #@2dr5#"
   //COND/
    "#@Fdl7#  INT_MEAS_IND"
   End header */
//ID/
#define TRL1_MPHP_INT_MEAS_IND 88
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD8            rxlev_0[8];
  WORD8            rxlev_1[8];
}
T_TR_MPHP_INT_MEAS_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_TINT_MEAS_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | TINT_MEAS_IND            |                rxlev: #@1dr5#"
    "        |  |  |  |  |                          |                       #@2dr5#"
   //COND/
    "#@Fdl7#  TINT_MEAS_IND"
   End header */
//ID/
#define TRL1_MPHP_TINT_MEAS_IND 89
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD8            rxlev_0[8];
  WORD8            rxlev_1[8];
}
T_TR_MPHP_TINT_MEAS_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_ITMEAS_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               ITMEAS_IND |             position: #@1dl10#          meas_bitmap: #@2x#"
   //COND/
    "#@Fdl7#                          ITMEAS_IND"
   End header */
//ID/
#define TRL1_L1P_ITMEAS_IND 90
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           position;
  UWORD8           meas_bitmap;
}
T_TR_L1P_ITMEAS_IND;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_NCELL_PBCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | NCELL_PBCCH_REQ          |            fn_offset: #@1dl10#       time_alignment: #@2d#"
    "        |  |  |  |  |                          |           radio_freq: #@3dl10#          rf_chan_cnt: #@4d#"
    "        |  |  |  |  |                          |         bcch_carrier: #@5dl10#        bs_pbcch_blks: #@6d#"
    "        |  |  |  |  |                          |                   pb: - #@7*2dl5# dBm  psi1_repeat_period: #@8d#"
    "        |  |  |  |  |                          |                    h: #@10=0:Single RF|=1:Hopping RF|~|l10#          timeslot_no: #@11dl10#"
    "        |  |  |  |  |                          |                  tsc: #@12d#"
    "        |  |  |  |  |                          |    relative_position: #@9dr5#"
   //COND/
    "#@Fdl7#  NCELL_PBCCH_REQ                                                radio_freq: #@3dl10#"
   End header */
//ID/
#define TRL1_MPHP_NCELL_PBCCH_REQ 91
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_offset;
  UWORD32          time_alignment;
  UWORD16          radio_freq;
  UWORD16          rf_chan_cnt;
  UWORD16          bcch_carrier;
  UWORD8           bs_pbcch_blks;
  UWORD8           pb;
  UWORD8           psi1_repeat_period;
  UWORD8           relative_position;
  BOOL             h;
  UWORD8           timeslot_no;
  UWORD8           tsc;
}
T_TR_MPHP_NCELL_PBCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_UPDATE_PSI_PARAM_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | UPDATE_PSI_PARAM_REQ     |                   pb: - #@1*2dl5# dBm   access_burst_type: #@2=0: 8 bit|=1:11 bit|#"
   //COND/
    "#@Fdl7#  UPDATE_PSI_PARAM_REQ"
   End header */
//ID/
#define TRL1_MPHP_UPDATE_PSI_PARAM_REQ 92
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           pb;
  BOOL             access_burst_type;
}
T_TR_MPHP_UPDATE_PSI_PARAM_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_TBF_RELEASED
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |             TBF_RELEASED |         released_all: #@1=0:NO|=1:YES|l10#                dl_tn: #@2d#"
    "        |  |  |  |  |                          |             tbf_type: #@3=0:DL TBF|=1:UL TBF|=2:BOTH TBF|#"
   //COND/
    "#@Fdl7#                          TBF_RELEASED                           tbf_type: #@3=0:DL TBF |=1:UL TBF |=2:BOTH TBF|~|#  #@2=1:All released|~|# dl_tn: #@2d#"
   End header */
//ID/
#define TRL1_L1P_TBF_RELEASED 93
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  BOOL             released_all;
  UWORD8           dl_tn;
  UWORD8           tbf_type;
}
T_TR_L1P_TBF_RELEASED;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_PDCH_RELEASED
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |            PDCH_RELEASED |         assignment_id: #@1dl10#               dl_tn: #@2d#"
   //COND/
    "#@Fdl7#                          PDCH_RELEASED                          dl_tn: #@2d#"
   End header */
//ID/
#define TRL1_L1P_PDCH_RELEASED 94
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           assignment_id;
  UWORD8           dl_tn;
}
T_TR_L1P_PDCH_RELEASED;

#define MAX_TCR 10

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_TCR_MEAS_DONE
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |            TCR_MEAS_DONE |              list_id: #@4dl10#           nb_carrier: #@5d#"
    "        |  |  |  |  |                          |           radio_freq: #@2dr5#"
    "        |  |  |  |  |                          |            acc_level: #@3dr5#"
    "        |  |  |  |  |                          |           acc_nbmeas: #@6dr5#"
    "        |  |  |  |  |                          |           tpu_offset: #@1d#"
   //COND/
    "#@Fdl7#                          TCR_MEAS_DONE"
   End header */
//ID/
#define TRL1_L1P_TCR_MEAS_DONE 95
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD16          radio_freq[10];
  WORD16           acc_level[10];
  UWORD8           list_id;
  UWORD8           nb_carrier;
  UWORD8           acc_nbmeas[10];
}
T_TR_L1P_TCR_MEAS_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_ASSIGNMENT_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |           ASSIGNMENT_CON |                dl_tn: #@1d#"
   //COND/
    "#@Fdl7#  ASSIGNMENT_CON                                                 dl_tn: #@1d#"
   End header */
//ID/
#define TRL1_MPHP_ASSIGNMENT_CON 96
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           dl_tn;
}
T_TR_MPHP_ASSIGNMENT_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_TCR_MEAS_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | TCR_MEAS_REQ             |           nb_carrier: #@1dl10#              list_id: #@2d#"
   //COND/
    "#@Fdl7#  TCR_MEAS_REQ"
   End header */
//ID/
#define TRL1_MPHP_TCR_MEAS_REQ 97
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           nb_carrier;
  UWORD8           list_id;
}
T_TR_MPHP_TCR_MEAS_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_NETWORK_SYNC_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_NETWORK_SYNC_REQ    |"
   //COND/
    "#@Fdl7#  STOP_NETWORK_SYNC_REQ"
   End header */
//ID/
#define TRL1_MPHC_STOP_NETWORK_SYNC_REQ 98
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_NETWORK_SYNC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_NCELL_PBCCH_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | NCELL_PBCCH_STOP_REQ     |"
   //COND/
    "#@Fdl7#  NCELL_PBCCH_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_NCELL_PBCCH_STOP_REQ 99
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_NCELL_PBCCH_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_STOP_PCCCH_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_PCCCH_REQ           |"
   //COND/
    "#@Fdl7#  STOP_PCCCH_REQ"
   End header */
//ID/
#define TRL1_MPHP_STOP_PCCCH_REQ 100
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_STOP_PCCCH_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_SCELL_PBCCH_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SCELL_PBCCH_STOP_REQ     |"
   //COND/
    "#@Fdl7#  SCELL_PBCCH_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_SCELL_PBCCH_STOP_REQ 101
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_SCELL_PBCCH_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_RA_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | RA_STOP_REQ              |"
   //COND/
    "#@Fdl7#  RA_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_RA_STOP_REQ 102
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_RA_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_STOP_SINGLE_BLOCK_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_SINGLE_BLOCK_REQ    |"
   //COND/
    "#@Fdl7#  STOP_SINGLE_BLOCK_REQ"
   End header */
//ID/
#define TRL1_MPHP_STOP_SINGLE_BLOCK_REQ 103
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_STOP_SINGLE_BLOCK_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1P_TA_CONFIG_DONE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  |           TA_CONFIG_DONE |"
   //COND/
    "#@Fdl7#                          TA_CONFIG_DONE"
   End header */
//ID/
#define TRL1_L1P_TA_CONFIG_DONE 104
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1P_TA_CONFIG_DONE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_CR_MEAS_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | CR_MEAS_STOP_REQ         |"
   //COND/
    "#@Fdl7#  CR_MEAS_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_CR_MEAS_STOP_REQ 105
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_CR_MEAS_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_TCR_MEAS_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | TCR_MEAS_STOP_REQ        |"
   //COND/
    "#@Fdl7#  TCR_MEAS_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_TCR_MEAS_STOP_REQ 106
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_TCR_MEAS_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHP_INT_MEAS_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | INT_MEAS_STOP_REQ        |"
   //COND/
    "#@Fdl7#  INT_MEAS_STOP_REQ"
   End header */
//ID/
#define TRL1_MPHP_INT_MEAS_STOP_REQ 107
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHP_INT_MEAS_STOP_REQ;

////////////////////
// AUDIO messages //
////////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_KEYBEEP_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | KEYBEEP_START_REQ        |           d_k_x1_kt0: #@1dl10#           d_k_x1_kt1: #@2d#"
    "        |  |  |  |  |                          |             d_dur_kb: #@3d#"
   //COND/
    "#@Fdl7#  KEYBEEP_START_REQ"
   End header */
//ID/
#define TRL1_MMI_KEYBEEP_START_REQ 108
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          d_k_x1_kt0;
  UWORD16          d_k_x1_kt1;
  UWORD16          d_dur_kb;
}
T_TR_MMI_KEYBEEP_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_KEYBEEP_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | KEYBEEP_START_CON        |"
   //COND/
    "#@Fdl7#  KEYBEEP_START_CON"
   End header */
//ID/
#define TRL1_MMI_KEYBEEP_START_CON 109
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_KEYBEEP_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_KEYBEEP_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | KEYBEEP_STOP_REQ         |"
   //COND/
    "#@Fdl7#  KEYBEEP_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_KEYBEEP_STOP_REQ 110
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_KEYBEEP_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_KEYBEEP_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | KEYBEEP_STOP_CON         |"
   //COND/
    "#@Fdl7#  KEYBEEP_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_KEYBEEP_STOP_CON 111
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_KEYBEEP_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TONE_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | TONE_START_REQ           |            d_k_x1_t0: #@1dl10#            d_k_x1_t1: #@2d#"
    "        |  |  |  |  |                          |            d_k_x1_t2: #@3dl10#             d_pe_rep: #@4d#"
    "        |  |  |  |  |                          |             d_pe_off: #@5dl10#             d_se_off: #@6d#"
    "        |  |  |  |  |                          |             d_bu_off: #@7dl10#"
    "        |  |  |  |  |                          |              d_t0_on: #@8dl10#             d_t0_off: #@9d#"
    "        |  |  |  |  |                          |              d_t1_on: #@10dl10#             d_t1_off: #@11d#"
    "        |  |  |  |  |                          |              d_t2_on: #@12dl10#             d_t2_off: #@13d#"
   //COND/
    "#@Fdl7#  TONE_START_REQ"
   End header */
//ID/
#define TRL1_MMI_TONE_START_REQ 112
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          d_k_x1_t0;
  UWORD16          d_k_x1_t1;
  UWORD16          d_k_x1_t2;
  UWORD16          d_pe_rep;
  UWORD16          d_pe_off;
  UWORD16          d_se_off;
  UWORD16          d_bu_off;
  UWORD16          d_t0_on;
  UWORD16          d_t0_off;
  UWORD16          d_t1_on;
  UWORD16          d_t1_off;
  UWORD16          d_t2_on;
  UWORD16          d_t2_off;
}
T_TR_MMI_TONE_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TONE_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | TONE_START_CON           |"
   //COND/
    "#@Fdl7#  TONE_START_CON"
   End header */
//ID/
#define TRL1_MMI_TONE_START_CON 113
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_TONE_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TONE_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | TONE_STOP_REQ            |"
   //COND/
    "#@Fdl7#  TONE_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_TONE_STOP_REQ 114
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_TONE_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TONE_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | TONE_STOP_CON            |"
   //COND/
    "#@Fdl7#  TONE_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_TONE_STOP_CON 115
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_TONE_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MELODY0_START_REQ        |           session_id: #@2dl10#             loopback: #@3=0:NO|=1:YES|~|#"
    "        |  |  |  |  |                          |    oscillator_bitmap: #@3bz16#"
   //COND/
    "#@Fdl7#  MELODY0_START_REQ                                              id: #@2d#"
   End header */
//ID/
#define TRL1_MMI_MELODY0_START_REQ 116
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          oscillator_used_bitmap;
  UWORD8           session_id;
  BOOL             loopback;
}
T_TR_MMI_MELODY0_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MELODY1_START_REQ        |           session_id: #@2dl10#             loopback: #@3=0:NO|=1:YES|~|#"
    "        |  |  |  |  |                          |    oscillator_bitmap: #@3bz16#"
   //COND/
    "#@Fdl7#  MELODY1_START_REQ                                              id: #@2d#"
   End header */
//ID/
#define TRL1_MMI_MELODY1_START_REQ 117
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          oscillator_used_bitmap;
  UWORD8           session_id;
  BOOL             loopback;
}
T_TR_MMI_MELODY1_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY0_START_CON        |"
   //COND/
    "#@Fdl7#  MELODY0_START_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY0_START_CON 118
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | MELODY0_STOP_REQ         |"
   //COND/
    "#@Fdl7#  MELODY0_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_MELODY0_STOP_REQ 119
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY0_STOP_CON         |"
   //COND/
    "#@Fdl7#  MELODY0_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY0_STOP_CON 120
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY1_START_CON        |"
   //COND/
    "#@Fdl7#  MELODY1_START_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY1_START_CON 121
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | MELODY1_STOP_REQ         |"
   //COND/
    "#@Fdl7#  MELODY1_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_MELODY1_STOP_REQ 122
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY1_STOP_CON         |"
   //COND/
    "#@Fdl7#  MELODY1_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY1_STOP_CON 123
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_PLAY_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | VM_PLAY_START_REQ        |           session_id: #@1d#"
   //COND/
    "#@Fdl7#  VM_PLAY_START_REQ                                              id: #@1d#"
   End header */
//ID/
#define TRL1_MMI_VM_PLAY_START_REQ 124
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           session_id;
}
T_TR_MMI_VM_PLAY_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_PLAY_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_PLAY_START_CON        |"
   //COND/
    "#@Fdl7#  VM_PLAY_START_CON"
   End header */
//ID/
#define TRL1_MMI_VM_PLAY_START_CON 125
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_PLAY_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_PLAY_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_PLAY_STOP_REQ         |"
   //COND/
    "#@Fdl7#  VM_PLAY_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_PLAY_STOP_REQ 126
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_PLAY_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_PLAY_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_PLAY_STOP_CON         |"
   //COND/
    "#@Fdl7#  VM_PLAY_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_VM_PLAY_STOP_CON 127
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_PLAY_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_RECORD_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | VM_RECORD_START_REQ      |           session_id: #@17dl10#             dtx_used: #@18=0:NO|=1:YES|~|#"
    "        |  |  |  |  |                          |         maximum_size: #@1dl10#            d_k_x1_t0: #@4d#"
    "        |  |  |  |  |                          |            d_k_x1_t1: #@5dl10#            d_k_x1_t2: #@6d#"
    "        |  |  |  |  |                          |      record_coeff_dl: #@2dl10#      record_coeff_ul: #@3d#"
    "        |  |  |  |  |                          |             d_pe_rep: #@7dl10#             d_pe_off: #@8d#"
    "        |  |  |  |  |                          |             d_se_off: #@9dl10#             d_bu_off: #@10d#"
    "        |  |  |  |  |                          |              d_t0_on: #@11dl10#             d_t0_off: #@12d#"
    "        |  |  |  |  |                          |              d_t1_on: #@13dl10#             d_t1_off: #@14d#"
    "        |  |  |  |  |                          |              d_t2_on: #@15dl10#             d_t2_off: #@16d#"
   //COND/
    "#@Fdl7#  VM_RECORD_START_REQ                                            id: #@17d#"
   End header */
//ID/
#define TRL1_MMI_VM_RECORD_START_REQ 128
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          maximum_size;
  UWORD16          record_coeff_dl;
  UWORD16          record_coeff_ul;
  UWORD16          d_k_x1_t0;
  UWORD16          d_k_x1_t1;
  UWORD16          d_k_x1_t2;
  UWORD16          d_pe_rep;
  UWORD16          d_pe_off;
  UWORD16          d_se_off;
  UWORD16          d_bu_off;
  UWORD16          d_t0_on;
  UWORD16          d_t0_off;
  UWORD16          d_t1_on;
  UWORD16          d_t1_off;
  UWORD16          d_t2_on;
  UWORD16          d_t2_off;
  UWORD8           session_id;
  BOOL             dtx_used;
}
T_TR_MMI_VM_RECORD_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_RECORD_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_RECORD_START_CON      |"
   //COND/
    "#@Fdl7#  VM_RECORD_START_CON"
   End header */
//ID/
#define TRL1_MMI_VM_RECORD_START_CON 129
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_RECORD_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_RECORD_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_RECORD_STOP_REQ       |"
   //COND/
    "#@Fdl7#  VM_RECORD_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_RECORD_STOP_REQ 130
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_RECORD_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_RECORD_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_RECORD_STOP_CON       |"
   //COND/
    "#@Fdl7#  VM_RECORD_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_VM_RECORD_STOP_CON 131
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_RECORD_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PLAY_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | VM_AMR_PLAY_START_REQ    |           session_id: #@1d#"
   //COND/
    "#@Fdl7#  VM_AMR_PLAY_START_REQ                                          id: #@1d#"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PLAY_START_REQ 209
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           session_id;
}
T_TR_MMI_VM_AMR_PLAY_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PLAY_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_AMR_PLAY_START_CON    |"
   //COND/
    "#@Fdl7#  VM_AMR_PLAY_START_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PLAY_START_CON 210
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_PLAY_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PLAY_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_PLAY_STOP_REQ     |"
   //COND/
    "#@Fdl7#  VM_AMR_PLAY_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PLAY_STOP_REQ 211
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_PLAY_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PLAY_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_AMR_PLAY_STOP_CON     |"
   //COND/
    "#@Fdl7#  VM_AMR_PLAY_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PLAY_STOP_CON 212
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_PLAY_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RECORD_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | VM_AMR_RECORD_START_REQ  |           session_id: #@3dl10#              dtx_used: #@5=0:NO|=1:YES|~|#"
    "        |  |  |  |  |                          |         maximum_size: #@1dl10#       record_coeff_ul: #@2d#"
    "        |  |  |  |  |                          |          amr_vocoder: #@4dl10#"
   //COND/
    "#@Fdl7#  VM_AMR_RECORD_START_REQ                                        id: #@3d# vocoder: #@4d#"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RECORD_START_REQ 213
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          maximum_size;
  UWORD16          record_coeff_ul;
  UWORD8           session_id;
  UWORD8           amr_vocoder;
  BOOL             dtx_used;
}
T_TR_MMI_VM_AMR_RECORD_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RECORD_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_AMR_RECORD_START_CON  |"
   //COND/
    "#@Fdl7#  VM_AMR_RECORD_START_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RECORD_START_CON 214
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_RECORD_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RECORD_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_RECORD_STOP_REQ   |"
   //COND/
    "#@Fdl7#  VM_AMR_RECORD_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RECORD_STOP_REQ 215
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_RECORD_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RECORD_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | VM_AMR_RECORD_STOP_CON   |"
   //COND/
    "#@Fdl7#  VM_AMR_RECORD_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RECORD_STOP_CON 216
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_VM_AMR_RECORD_STOP_CON;
/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PAUSE_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_PAUSE_REQ    |  #"
   //COND/
    "#@Fdl7#  VM_AMR_PAUSE_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PAUSE_REQ 227
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
 
}
T_TR_MMI_VM_AMR_PAUSE_REQ;
/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RESUME_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_RESUME_REQ    | "
   //COND/
    "#@Fdl7#  VM_AMR_RESUME_REQ"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RESUME_REQ 228
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
//  UWORD8           session_id;
}
T_TR_MMI_VM_AMR_RESUME_REQ;
/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_PAUSE_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_PAUSE_CON    |"
   //COND/
    "#@Fdl7#  VM_AMR_PAUSE_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_PAUSE_CON 229
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
 }
T_TR_MMI_VM_AMR_PAUSE_CON;
/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_VM_AMR_RESUME_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | VM_AMR_RESUME_CON    |"
   //COND/
    "#@Fdl7#  VM_AMR_RESUME_CON"
   End header */
//ID/
#define TRL1_MMI_VM_AMR_RESUME_CON 230
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
}
T_TR_MMI_VM_AMR_RESUME_CON;


/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_ENROLL_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SR_ENROLL_START_REQ      |          database_id: #@2dl10#           word_index: #@3d#"
    "        |  |  |  |  |                          |               speech: #@4=0:NO|=1:YES|~|l10#       speech_address: #@1x#"
   //COND/
    "#@Fdl7#  SR_ENROLL_START_REQ                                            id: #@2dl10#"
   End header */
//ID/
#define TRL1_MMI_SR_ENROLL_START_REQ 132
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          speech_address;
  UWORD8           database_id;
  UWORD8           word_index;
  BOOL             speech;
}
T_TR_MMI_SR_ENROLL_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_ENROLL_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SR_ENROLL_START_CON      |"
   //COND/
    "#@Fdl7#  SR_ENROLL_START_CON"
   End header */
//ID/
#define TRL1_MMI_SR_ENROLL_START_CON 133
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_ENROLL_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_ENROLL_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SR_ENROLL_STOP_REQ       |"
   //COND/
    "#@Fdl7#  SR_ENROLL_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_SR_ENROLL_STOP_REQ 134
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_ENROLL_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_ENROLL_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | SR_ENROLL_STOP_CON       |             error_id: #@1=0:No error|=1:Bad acquisition|=2:Timeout|#"
   //COND/
    "#@Fdl7#  SR_ENROLL_STOP_CON                              #@1=1:Bad acquisition|=2:Timeout|~|#"
   End header */
//ID/
#define TRL1_MMI_SR_ENROLL_STOP_CON 135
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           error_id;
}
T_TR_MMI_SR_ENROLL_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SR_UPDATE_START_REQ      |          database_id: #@2dl10#           word_index: #@3d#"
    "        |  |  |  |  |                          |               speech: #@4=0:NO|=1:YES|~|l10#       speech_address: #@1x#"
   //COND/
    "#@Fdl7#  SR_UPDATE_START_REQ                                            id: #@2dl10#"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_START_REQ 136
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          *speech_address;
  UWORD8           database_id;
  UWORD8           word_index;
  BOOL             speech;
}
T_TR_MMI_SR_UPDATE_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SR_UPDATE_STOP_REQ       |"
   //COND/
    "#@Fdl7#  SR_UPDATE_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_STOP_REQ 137
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_UPDATE_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SR_UPDATE_START_CON      |"
   //COND/
    "#@Fdl7#  SR_UPDATE_START_CON"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_START_CON 138
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_UPDATE_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | SR_UPDATE_STOP_CON       |             error_id: #@1=0:No error|=1:Bad acquisition|=2:Timeout|=3:Bad update|#"
   //COND/
    "#@Fdl7#  SR_UPDATE_STOP_CON                              #@1=1:Bad acquisition|=2:Timeout|=3:Bad update|~|#"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_STOP_CON 139
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           error_id;
}
T_TR_MMI_SR_UPDATE_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_RECO_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SR_RECO_START_REQ        |          database_id: #@1dl10#      vocabulary_size: #@2d#"
   //COND/
    "#@Fdl7#  SR_RECO_START_REQ                                              id: #@1d#"
   End header */
//ID/
#define TRL1_MMI_SR_RECO_START_REQ 140
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           database_id;
  UWORD8           vocabulary_size;
}
T_TR_MMI_SR_RECO_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_RECO_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SR_RECO_STOP_REQ         |"
   //COND/
    "#@Fdl7#  SR_RECO_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_SR_RECO_STOP_REQ 141
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_RECO_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_RECO_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SR_RECO_START_CON        |"
   //COND/
    "#@Fdl7#  SR_RECO_START_CON"
   End header */
//ID/
#define TRL1_MMI_SR_RECO_START_CON 142
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_RECO_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_RECO_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | SR_RECO_STOP_CON         |             error_id: #@12=0:No error|=1:Bad acquisition|=2:Timeout|=3:Bad recognition|=4:CTO word|l10#  d_sr_model_size: #@11d#"
    "        |  |  |  |  |                          |      best_word_score: #@1dl10#  2nd_best_word_score: #@2d#"
    "        |  |  |  |  |                          |  3rd_best_word_score: #@3dl10#  4th_best_word_score: #@4d#"
    "        |  |  |  |  |                          |      best_word_index: #@5dl10#  2nd_best_word_index: #@6d#"
    "        |  |  |  |  |                          |  3rd_best_word_index: #@7dl10#  4th_best_word_index: #@8d#"
    "        |  |  |  |  |                          |        d_sr_db_level: #@9dl10#        d_sr_db_noise: #@10d#"
   //COND/
    "#@Fdl7#  SR_RECO_STOP_CON                                #@12=1:Bad acquisition|=2:Timeout|=3:Bad recognition|=4:CTO word|~|#"
   End header */
//ID/
#define TRL1_MMI_SR_RECO_STOP_CON 143
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          best_word_score;
  UWORD32          second_best_word_score;
  UWORD32          third_best_word_score;
  UWORD32          fourth_best_word_score;
  UWORD16          best_word_index;
  UWORD16          second_best_word_index;
  UWORD16          third_best_word_index;
  UWORD16          fourth_best_word_index;
  UWORD16          d_sr_db_level;
  UWORD16          d_sr_db_noise;
  UWORD16          d_sr_model_size;
  UWORD8           error_id;
}
T_TR_MMI_SR_RECO_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_CHECK_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SR_UPDATE_CHECK_START    |          database_id: #@3dl10#           word_index: #@4d#"
    "        |  |  |  |  |                     _REQ |               speech: #@5dl10#      vocabulary_size: #@6d#"
    "        |  |  |  |  |                          |        model_address: #@1dl10#       speech_address: #@2d#"
   //COND/
    "#@Fdl7#  SR_UPDATE_CHECK_START_REQ                                      id: #@3dl10#"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_CHECK_START_REQ 144
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          model_address;
  UWORD32          speech_address;
  UWORD8           database_id;
  UWORD8           word_index;
  BOOL             speech;
  UWORD8           vocabulary_size;
}
T_TR_MMI_SR_UPDATE_CHECK_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_CHECK_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SR_UPDATE_CHECK_STOP_REQ |"
   //COND/
    "#@Fdl7#  SR_UPDATE_CHECK_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_CHECK_STOP_REQ 145
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_UPDATE_CHECK_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_CHECK_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | SR_UPDATE_CHECK_START    |"
    "        |  |  |  |  |                     _CON |"
   //COND/
    "#@Fdl7#  SR_UPDATE_CHECK_START_CON"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_CHECK_START_CON 146
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_SR_UPDATE_CHECK_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_SR_UPDATE_CHECK_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | SR_UPDATE_CHECK_STOP_CON |             error_id: #@12=0:No error|=1:Bad acquisition|=2:Timeout|=3:Bad recognition|=4:CTO word|l10#  d_sr_model_size: #@11d#"
    "        |  |  |  |  |                          |      best_word_score: #@1dl10#  2nd_best_word_score: #@2d#"
    "        |  |  |  |  |                          |  3rd_best_word_score: #@3dl10#  4th_best_word_score: #@4d#"
    "        |  |  |  |  |                          |      best_word_index: #@5dl10#  2nd_best_word_index: #@6d#"
    "        |  |  |  |  |                          |  3rd_best_word_index: #@7dl10#  4th_best_word_index: #@8d#"
    "        |  |  |  |  |                          |        d_sr_db_level: #@9dl10#        d_sr_db_noise: #@10d#"
   //COND/
    "#@Fdl7#  SR_UPDATE_CHECK_STOP_CON                        #@12=1:Bad acquisition|=2:Timeout|=3:Bad recognition|=4:CTO word|~|#"
   End header */
//ID/
#define TRL1_MMI_SR_UPDATE_CHECK_STOP_CON 147
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          best_word_score;
  UWORD32          second_best_word_score;
  UWORD32          third_best_word_score;
  UWORD32          fourth_best_word_score;
  UWORD16          best_word_index;
  UWORD16          second_best_word_index;
  UWORD16          third_best_word_index;
  UWORD16          fourth_best_word_index;
  UWORD16          d_sr_db_level;
  UWORD16          d_sr_db_noise;
  UWORD16          d_sr_model_size;
  UWORD8           error_id;
}
T_TR_MMI_SR_UPDATE_CHECK_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_SAVE_DATA_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SRBACK_SAVE_DATA_REQ     |          database_id: #@6dl10#          model_index: #@7d#"
    "        |  |  |  |  |                          |    model_RAM_address: #@1xl10#               speech: #@8=0:No|=1:Yes|#"
    "        |  |  |  |  |                          |         start_buffer: #@2xl10#          stop_buffer: #@3x#"
    "        |  |  |  |  |                          |        start_address: #@4xl10#         stop_address: #@5x#"
   //COND/
    "#@Fdl7#  SRBACK_SAVE_DATA_REQ                                           id: #@6d#"
   End header */
//ID/
#define TRL1_L1_SRBACK_SAVE_DATA_REQ 148
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          model_RAM_address;
  UWORD32          start_buffer;
  UWORD32          stop_buffer;
  UWORD32          start_address;
  UWORD32          stop_address;
  UWORD8           database_id;
  UWORD8           model_index;
  BOOL             speech;
}
T_TR_L1_SRBACK_SAVE_DATA_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_SAVE_DATA_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SRBACK_SAVE_DATA_CON     |"
   //COND/
    "#@Fdl7#  SRBACK_SAVE_DATA_CON"
   End header */
//ID/
#define TRL1_L1_SRBACK_SAVE_DATA_CON 149
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_SRBACK_SAVE_DATA_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_LOAD_MODEL_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SRBACK_LOAD_MODEL_REQ    |          database_id: #@2dl10#          model_index: #@3d#"
    "        |  |  |  |  |                          |           CTO_enable: #@4=0:No|=1:Yes|l10#    model_RAM_address: #@1x#"
   //COND/
    "#@Fdl7#  SRBACK_LOAD_MODEL_REQ                                          id: #@2d#"
   End header */
//ID/
#define TRL1_L1_SRBACK_LOAD_MODEL_REQ 150
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          model_RAM_address;
  UWORD8           database_id;
  UWORD8           model_index;
  BOOL             CTO_enable;
}
T_TR_L1_SRBACK_LOAD_MODEL_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_LOAD_MODEL_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SRBACK_LOAD_MODEL_CON    |"
   //COND/
    "#@Fdl7#  SRBACK_LOAD_MODEL_CON"
   End header */
//ID/
#define TRL1_L1_SRBACK_LOAD_MODEL_CON 151
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_SRBACK_LOAD_MODEL_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_TEMP_SAVE_DATA_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | SRBACK_TEMP_SAVE_DATA    | model_RAM_addr_input: #@1xl10# model_RAM_add_output: #@2x#"
    "        |  |  |  |  |                     _REQ |"
   //COND/
    "#@Fdl7#  SRBACK_TEMP_SAVE_DATA_REQ"
   End header */
//ID/
#define TRL1_L1_SRBACK_TEMP_SAVE_DATA_REQ 152
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          model_RAM_address_input;
  UWORD32          model_RAM_address_output;
}
T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_SRBACK_TEMP_SAVE_DATA_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | SRBACK_TEMP_SAVE_DATA    |"
    "        |  |  |  |  |                     _CON |"
   //COND/
    "#@Fdl7#  SRBACK_TEMP_SAVE_DATA_CON"
   End header */
//ID/
#define TRL1_L1_SRBACK_TEMP_SAVE_DATA_CON 153
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_SRBACK_TEMP_SAVE_DATA_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AEC_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | AEC_REQ                  |          aec_control: #@1bz16#    cont_filter: #@2h#"
    "#@Fdl7# |  |  |  |  |                          |      granularity_att: #@3xl10#          coef_smooth: #@4h#"
    "#@Fdl7# |  |  |  |  |                          |         es_level_max: #@5xl10#             fact_vad: #@6h#"
    "#@Fdl7# |  |  |  |  |                          |             thrs_abs: #@6xl10#         fact_asd_fil: #@8h#"
    "#@Fdl7# |  |  |  |  |                          |         fact_asd_mut: #@9xl10#"
   //COND/
    "#@Fdl7#  AEC_REQ                                                        aec_control: #@1h#"
   End header */
//ID/
#define TRL1_MMI_AEC_REQ 154
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          aec_control;
  UWORD16          cont_filter;
  UWORD16          granularity_att;
  UWORD16          coef_smooth;
  UWORD16          es_level_max;
  UWORD16          fact_vad;
  UWORD16          thrs_abs;
  UWORD16          fact_asd_fil;
  UWORD16          fact_asd_mut;
}
T_TR_MMI_AEC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AEC_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | AEC_CON                  |"
   //COND/
    "#@Fdl7#  AEC_CON"
   End header */
//ID/
#define TRL1_MMI_AEC_CON 155
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_AEC_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AUDIO_FIR_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | AUDIO_FIR_REQ            |           update_fir: #@4=1:DL|=2:UL|=3:DL+UL|l10#             fir_loop: #@3=0:NO|=1:Yes|~|#"
    "        |  |  |  |  |                          |   fir_ul_coefficient: #@1xl10#   fir_dl_coefficient: #@2x#"
   //COND/
    "#@Fdl7#  AUDIO_FIR_REQ                                                  #@4=1:DL|=2:UL|=3:DL+UL|#"
   End header */
//ID/
#define TRL1_MMI_AUDIO_FIR_REQ 156
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fir_ul_coefficient;
  UWORD32          fir_dl_coefficient;
  BOOL             fir_loop;
  UWORD8           update_fir;
}
T_TR_MMI_AUDIO_FIR_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AUDIO_FIR_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | AUDIO_FIR_CON            |"
   //COND/
    "#@Fdl7#  AUDIO_FIR_CON"
   End header */
//ID/
#define TRL1_MMI_AUDIO_FIR_CON 157
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_AUDIO_FIR_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AUDIO_MODE_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | AUDIO_MODE_REQ           |           audio_mode: #@1=0:GSM only|=1:BT cordless mode|=2:BT headset mode|#"
   //COND/
    "#@Fdl7#  AUDIO_MODE_REQ                                                 #@1=0:GSM only|=1:BT cordless mode|=2:BT headset mode|#"
   End header */
//ID/
#define TRL1_MMI_AUDIO_MODE_REQ 158
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          audio_mode;
}
T_TR_MMI_AUDIO_MODE_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_AUDIO_MODE_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | AUDIO_MODE_CON           |"
   //COND/
    "#@Fdl7#  AUDIO_MODE_CON"
   End header */
//ID/
#define TRL1_MMI_AUDIO_MODE_CON 159
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_AUDIO_MODE_CON;

////////////////
// Debug info //
////////////////

// L1S debug

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ PM EQUAL 0
   //ERROR/ PM,@8=1
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  | *** |           PM = 0 #@8=1:(begin)|=0:(end)|l7# |           tpu_offset: #@1dl10#        tpu_offset_hw: #@2d#"
    "        |  |  |  |  |                          |              d_debug: #@3dl10#           debug_time: #@4d#"
    "        |  |  |  |  |                          |             adc_mode: #@5dl10#                 task: #@6t#"
    "        |  |  |  |  |                          |           no_pm_task: #@7t#"
   //COND/
    "#@Fdl7#                                                  PM=0 #@8=0:(end)|=1:(begin)|#"
   End header */
//ID/
#define TRL1_PM_EQUAL_0 160
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD32          tpu_offset_hw;
  UWORD16          d_debug;
  UWORD16          debug_time;
  UWORD16          adc_mode;
  UWORD8           task;
  UWORD8           no_pm_task;
  BOOL             error;
}
T_TR_PM_EQUAL_0;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MCU DSP COM mismatch
   //ERROR/ COM,@7=1
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  | *** | MCU/DSP Mismatch #@7=1:(begin)|=0:(end)|l7# |           tpu_offset: #@1dl10#        tpu_offset_hw: #@2d#"
    "        |  |  |  |  |                          |              d_debug: #@3dl10#           debug_time: #@4d#"
    "        |  |  |  |  |                          |             adc_mode: #@5dl10#                 task: #@6t#"
   //COND/
    "#@Fdl7#                                                  COM #@7=0:(end)|=1:(begin)|#"
   End header */
//ID/
#define TRL1_MCU_DSP_MISMATCH 161
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD32          tpu_offset_hw;
  UWORD16          d_debug;
  UWORD16          debug_time;
  UWORD16          adc_mode;
  UWORD8           task;
  BOOL             error;
}
T_TR_MCU_DSP_MISMATCH;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1S ABORT
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                    ABORT |           tpu_offset: #@1dl10#        tpu_offset_hw: #@2d#"
    "        |  |  |  |  |                          |              d_debug: #@3dl10#           debug_time: #@4d#"
    "        |  |  |  |  |                          |             adc_mode: #@5dl10#                 task: #@6t#"
   //COND/
    "#@Fdl7#                          L1S ABORT"
   End header */
//ID/
#define TRL1_L1S_ABORT 162
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          tpu_offset;
  UWORD32          tpu_offset_hw;
  UWORD16          d_debug;
  UWORD16          debug_time;
  UWORD16          adc_mode;
  UWORD8           task;
}
T_TR_L1S_ABORT;

// DSP error

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ D_ERROR_STATUS
   //ERROR/ DSP,@2!0
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  | ***  D_ERROR_STATUS #@2=0:(end)|!0:(begin)|l7# |           debug_time: #@1dl10#       d_error_status: #@2x#"
    "        |  |  |  |  |                          |              d_debug: #@3d#"
   //COND/
    "#@Fdl7#                                                  D_ERROR_STATUS #@2=0:(end)|!0:(begin)|#"
   End header */
//ID/
#define TRL1_D_ERROR_STATUS 163
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          debug_time;
  UWORD16          d_error_status;
  UWORD16          d_debug;
}
T_TR_D_ERROR_STATUS;

// DSP trace

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define TRL1_DSP_DEBUG_HEADER 164

typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          debug_time;
  UWORD16          patch_version;
  UWORD16          trace_level;
}
T_TR_DSP_DEBUG_HEADER;

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define TRL1_DSP_DEBUG_BUFFER 165

typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          size;
  UWORD16          content[1];
}
T_TR_DSP_DEBUG_BUFFER;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ RLC_UL_PARAM
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |<....|  | RLC_UL                   |            a_pu_gprs: #@1xl10#            a_du_gprs: #@2x#"
    "        |  |  |  |  |                          |                   fn: #@3dl10#        assignment_id: #@4d#"
    "        |  |  |  |  |                          |                tx_no: #@5dl10#                   ta: #@6d#"
    "        |  |  |  |  |                          |    fix_alloc_exhaust: #@7=0:No|=1:Yes|l10#"
   //COND/
    "#@Fdl7#                          RLC_UL"
   End header */
//ID/
#define TRL1_RLC_UL_PARAM 166
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          a_pu_gprs;
  UWORD32          a_du_gprs;
  UWORD32          fn_param;
  UWORD8           assignment_id;
  UWORD8           tx_no;
  UWORD8           ta;
  BOOL             fix_alloc_exhaust;
}
T_TR_RLC_UL_PARAM;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ RLC_DL_PARAM
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |<....|  | RLC_DL                   |                   fn: #@1dl10#  d_rlcmac_rx_no_gprs: #@2x#"
    "        |  |  |  |  |                          |        assignment_id: #@3dl10#                rx_no: #@4d#"
    "        |  |  |  |  |                          |      rlc_blocks_sent: #@5dl10#   last_poll_response: #@6bz8#"
   //COND/
    "#@Fdl7#                          RLC_DL"
   End header */
//ID/
#define TRL1_RLC_DL_PARAM 167
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          fn_param;
  UWORD32          d_rlcmac_rx_no_gprs;
  UWORD8           assignment_id;
  UWORD8           rx_no;
  UWORD8           rlc_blocks_sent;
  UWORD8           last_poll_response;
}
T_TR_RLC_DL_PARAM;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ FORBIDDEN_UPLINK
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  | *** |                          | UL block transmitted while forbidden (no TA)"
   //COND/
    "#@Fdl7#                                                  TX while no TA"
   End header */
//ID/
#define TRL1_FORBIDDEN_UPLINK 168
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_FORBIDDEN_UPLINK;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ DL_PTCCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |<-|                 DL PTCCH |            crc_error: #@1=0:OK|=1:ERROR|l10#           ordered_ta: #@2d#"
   //COND/
    "#@Fdl7#                          DL PTCCH                #@1=1:CRC ERROR|~|#"
   End header */
//ID/
#define TRL1_DL_PTCCH 169
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           crc_error;
  UWORD8           ordered_ta;
}
T_TR_DL_PTCCH;

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define TRL1_CONDENSED_PDTCH 170

typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           rx_allocation;
  UWORD8           tx_allocation;
  UWORD8           blk_status;
  UWORD8           dl_cs_type;
  UWORD8           dl_status[4];
  UWORD8           ul_status[4];
}
T_TR_CONDENSED_PDTCH;

///////////////////
// OML1 messages //
///////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ OML1_CLOSE_TCH_LOOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | CLOSE_TCH_LOOP_REQ       |          sub_channel: #@1dl10#        frame_erasure: loop #@2=0:A|=1:B|=2:C|=3:D|=4:E|=5:F|#"
   //COND/
    "------------------------------------------------------------------------------------------------------------------"
    "#@Fdl7#  CLOSE_TCH_LOOP_REQ                                             loop #@2=0:A|=1:B|=2:C|=3:D|=4:E|=5:F|#"
   End header */
//ID/
#define TRL1_OML1_CLOSE_TCH_LOOP_REQ 171
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           sub_channel;
  UWORD8           frame_erasure;
}
T_TR_OML1_CLOSE_TCH_LOOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ OML1_OPEN_TCH_LOOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | OPEN_TCH_LOOP_REQ        |"
   //COND/
    "------------------------------------------------------------------------------------------------------------------"
    "#@Fdl7#  OPEN_TCH_LOOP_REQ"
   End header */
//ID/
#define TRL1_OML1_OPEN_TCH_LOOP_REQ 172
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_OML1_OPEN_TCH_LOOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ OML1_START_DAI_TEST_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | START_DAI_TEST_REQ       |        tested_device: #@1=0:no test|=1:speech decoder|=2:speech encoder|=3:no test|=4:acoustic devices|#"
   //COND/
    "------------------------------------------------------------------------------------------------------------------"
    "#@Fdl7#  START_DAI_TEST_REQ"
   End header */
//ID/
#define TRL1_OML1_START_DAI_TEST_REQ 173
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           tested_device;
}
T_TR_OML1_START_DAI_TEST_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ OML1_STOP_DAI_TEST_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "================================================================================================================================================================"
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_DAI_TEST_REQ        |"
   //COND/
    "------------------------------------------------------------------------------------------------------------------"
    "#@Fdl7#  STOP_DAI_TEST_REQ"
   End header */
//ID/
#define TRL1_OML1_STOP_DAI_TEST_REQ 174
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_OML1_STOP_DAI_TEST_REQ;

///////////////////
// Test messages //
///////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ TST_TEST_HW_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | TEST_HW_REQ              |"
   //COND/
    "#@Fdl7#  TEST_HW_REQ"
   End header */
//ID/
#define TRL1_TST_TEST_HW_REQ 175
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_TST_TEST_HW_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_TEST_HW_INFO
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |             TEST_HW_INFO |   *******************************************"
    "        |  |  |  |  |                          |   *   DSP  code version: #@1xr4#h              *"
    "        |  |  |  |  |                          |   *            checksum: #@2xr4#h              *"
    "        |  |  |  |  |                          |   *       patch version: #@3xr4#h              *"
    "        |  |  |  |  |                          |   *   MCU  code version: l1_#@4xr4#_#@5xr4#_#@6xr4#  *"
    "        |  |  |  |  |                          |   *         d_checksum1: #@7xr4#h              *"
    "        |  |  |  |  |                          |   *         d_checksum2: #@8xr4#h              *"
    "        |  |  |  |  |                          |   *******************************************"
   //COND/
    ""
    ""
    "#@Fdl7#                          TEST_HW_INFO                           DSP #@1x#h #@2x#h #@3x#h"
    "                                                                        MCU l1_#@4xr4#_#@5xr4#_#@6xr4#"
   End header */
//ID/
#define TRL1_L1_TEST_HW_INFO 176
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          dsp_code_version;
  UWORD16          dsp_checksum;
  UWORD16          dsp_patch_version;
  UWORD16          mcu_alr_version;
  UWORD16          mcu_gprs_version;
  UWORD16          mcu_tm_version;
  UWORD16          d_checksum1;
  UWORD16          d_checksum2;
}
T_TR_L1_TEST_HW_INFO;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ TST_SLEEP_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | TST_SLEEP_REQ            |               clocks: #@1bz16#     sleep_mode: #@2=0:Off|=1:small|=2:big|=3:deep|=4:all|#"
   //COND/
    "#@Fdl7#  TST_SLEEP_REQ"
   End header */
//ID/
#define TRL1_TST_SLEEP_REQ 177
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          clocks;
  UWORD8           sleep_mode;
}
T_TR_TST_SLEEP_REQ;

//////////////////
// ADC messages //
//////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_ADC_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | ADC_REQ                  |"
   //COND/
    "#@Fdl7#  ADC_REQ"
   End header */
//ID/
#define TRL1_MMI_ADC_REQ 178
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_ADC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_STOP_ADC_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | STOP_ADC_REQ             |"
   //COND/
    "#@Fdl7#  STOP_ADC_REQ"
   End header */
//ID/
#define TRL1_MMI_STOP_ADC_REQ 179
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_STOP_ADC_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_STOP_ADC_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | STOP_ADC_CON             |"
   //COND/
    "#@Fdl7#  STOP_ADC_CON"
   End header */
//ID/
#define TRL1_MMI_STOP_ADC_CON 180
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_STOP_ADC_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_AEC_IND
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |<-|  |               L1_AEC_IND |             es_level: #@3h#"
    "        |  |  |  |  |                          |          far_end_pow: #@1h10#        far_end_noise: #@2h#"
   //COND/
    "#@Fdl7#  L1_AEC_IND"
   End header */
//ID/
#define TRL1_L1_AEC_IND 208
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          far_end_pow;
  UWORD32          far_end_noise;
  UWORD16          es_level;
}
T_TR_L1_AEC_IND;

//////////////
// CPU load //
//////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1S CPU peak
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |  O  |                          | CPU #@1dr2# %"
    "        |  |  |  |  |                          |"
   //COND/
    "#@Fdl7#                                                                                L1S CPU #@1d# %"
   End header */
//ID/
#define TRL1_L1S_CPU_PEAK 181
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           max_cpu;
}
T_TR_L1S_CPU_PEAK;

////////////////////////////////
// Trace configuration change //
////////////////////////////////

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ Trace configuration change
   //FULL/
    ""
    "*********************************************************************************************************************************************************************************"
    "                                                         trace config: #@1x#"
    "#@Fdl7#   Trace configuration change                    RTT config: #@2x#"
    "                                                            RTT event: #@3x#"
    "*********************************************************************************************************************************************************************************"
    ""
   //COND/
    ""
    "******************************************************************************************************************"
    "#@Fdl7#  Trace config change: #@1xl2#  RTT (#@2xl2#) #@3x#"
    "******************************************************************************************************************"
    ""
   End header */
//ID/
#define TRL1_TRACE_CONFIG_CHANGE 182
//STRUCT/
typedef struct
{
  UWORD32        header;
//--------------------------------------------------
  UWORD32        trace_config;
  UWORD32        rtt_cell_enable[8];
  UWORD32        rtt_event;
}
T_TR_TRACE_CONFIG_CHANGE;

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define TRL1_ASCII 183

typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          size;
  char             str[1];
}
T_TR_ASCII;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ IT_DSP_ERROR
   //ERROR/ IT
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  | *** |             IT DSP ERROR |"
   //COND/
    "#@Fdl7#                                                  IT DSP ERROR"
   End header */
//ID/
#define TRL1_IT_DSP_ERROR 185
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_IT_DSP_ERROR;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ ADC
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                      ADC |                 type: #@1=0:RX|=1:TX|#"
   //COND/
    "#@Fdl7#                          ADC"
   End header */
//ID/
#define TRL1_ADC 186
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           type;
}
T_TR_ADC;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ New TOA
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                  New TOA |            toa_shift: #@1d#"
   //COND/
    "#@Fdl7#                          New TOA"
   End header */
//ID/
#define TRL1_NEW_TOA 187
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD16           toa_shift;
#if (TOA_ALGO == 2)
    UWORD16   toa_frames_counter;  
    UWORD16   toa_accumul_counter; 
    UWORD16   toa_accumul_value;   
#endif  
}
T_TR_NEW_TOA;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ TOA not updated
   //ERROR/ TOA
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  | *** |          TOA not updated |            toa_shift: #@1d#"
   //COND/
    "#@Fdl7#                                                  TOA not updated"
   End header */
//ID/
#define TRL1_TOA_NOT_UPDATED 188
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD16           toa_shift;
}
T_TR_TOA_NOT_UPDATED;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ SLEEP
   //FULL/
    "        ---#@1dc7#---                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7#      ...                         SLEEP |           type_sleep: #@3=0:No sleep|=1:Big sleep|=2:Deep sleep|l10#          wakeup_type: #@4=0:Undefined|=1:L1S task|=2:OS task|=3:HW timer task|=4:Gauging task|=5:Async interrupt|#"
    "             ...                               |        why_big_sleep: #@5=0:Undefined|=1:Light on|=2:UART|=3:SIM|=4:Gauging|=5:Sleep mode|=6:DSP traces|=7:Bluetooth|#"
    "        ---#@2dc7#---                          |"
   //COND/
    "#@Fdl7#                          SLEEP"
   End header */
//ID/
#define TRL1_SLEEP 189
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          start_fn;
  UWORD32          end_fn;
  UWORD8           type_sleep;
  UWORD8           wakeup_type;
  UWORD8           big_sleep_type;
}
T_TR_SLEEP;

// Wakeup Type for Power management
//--------------------------------
#define WAKEUP_FOR_UNDEFINED       0
#define WAKEUP_FOR_L1_TASK         1
#define WAKEUP_FOR_OS_TASK         2
#define WAKEUP_FOR_HW_TIMER_TASK   3
#define WAKEUP_FOR_GAUGING_TASK    4
#define WAKEUP_BY_ASYNC_INTERRUPT  5
#define WAKEUP_ASYNCHRONOUS_ULPD_0           6
#define WAKEUP_ASYNCHRONOUS_SLEEP_DURATION_0 7

// Big Sleep source for Power management
//-------------------------------------
#define BIG_SLEEP_DUE_TO_UNDEFINED  0  // deep sleep is forbiden : cause undefined
#define BIG_SLEEP_DUE_TO_LIGHT_ON   1  // deep sleep is forbiden by ligth on activitie
#define BIG_SLEEP_DUE_TO_UART       2  // deep sleep is forbiden by UART activitie
#define BIG_SLEEP_DUE_TO_SIM        3  // deep sleep is forbiden by SIM activitie
#define BIG_SLEEP_DUE_TO_GAUGING    4  // deep sleep is forbiden by not enought gauging
#define BIG_SLEEP_DUE_TO_SLEEP_MODE 5  // deep sleep is forbiden by the sleep mode enabled
#define BIG_SLEEP_DUE_TO_DSP_TRACES 6  // deep sleep is forbiden by the DSP
#define BIG_SLEEP_DUE_TO_BLUETOOTH  7  // deep sleep is forbiden by the Bluetooth module
#define BIG_SLEEP_DUE_TO_CAMERA     8  // deep sleep is forbiden by the camera

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ Gauging
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |  O  |             #@1=0:Gauging|=1:Reset Gauging|l12# |"
   //COND/
    "#@Fdl7#                          #@1=0:Gauging|=1:Reset Gauging|~|#"
   End header */
//ID/
#define TRL1_GAUGING 190
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  BOOL             reset_gauging;
}
T_TR_GAUGING;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ Unknown L1S trace
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  X  |        Unknown L1S trace |                   id: #@1d#"
   //COND/
    "#@Fdl7#                          #@1=0:Gauging|=1:Reset Gauging|~|#"
   End header */
//ID/
#define TRL1_UNKNOWN_L1S_TRACE 191
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD16          id;
}
T_TR_UNKNOWN_L1S_TRACE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_E2_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MELODY0_E2_START_REQ     |           session_id: #@1dl10#             loopback: #@2=0:NO|=1:YES|~|#"
   //COND/
    "#@Fdl7#  MELODY0_E2_START_REQ                                           id: #@1d#"
   End header */
//ID/
#define TRL1_MMI_MELODY0_E2_START_REQ 192
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           session_id;
  BOOL             loopback;
}
T_TR_MMI_MELODY0_E2_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_E2_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | MELODY0_E2_STOP_REQ      |"
   //COND/
    "#@Fdl7#  MELODY0_E2_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_MELODY0_E2_STOP_REQ 193
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_E2_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_E2_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY0_E2_START_CON     |"
   //COND/
    "#@Fdl7#  MELODY0_E2_START_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY0_E2_START_CON 194
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_E2_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY0_E2_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY0_E2_STOP_CON      |"
   //COND/
    "#@Fdl7#  MELODY0_E2_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY0_E2_STOP_CON 195
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY0_E2_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_E2_START_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | MELODY1_E2_START_REQ     |           session_id: #@1dl10#             loopback: #@2=0:NO|=1:YES|~|#"
   //COND/
    "#@Fdl7#  MELODY1_E2_START_REQ                                           id: #@1d#"
   End header */
//ID/
#define TRL1_MMI_MELODY1_E2_START_REQ 196
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           session_id;
  BOOL             loopback;
}
T_TR_MMI_MELODY1_E2_START_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_E2_STOP_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | MELODY1_E2_STOP_REQ      |"
   //COND/
    "#@Fdl7#  MELODY1_E2_STOP_REQ"
   End header */
//ID/
#define TRL1_MMI_MELODY1_E2_STOP_REQ 197
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_E2_STOP_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_E2_START_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY1_E2_START_CON     |"
   //COND/
    "#@Fdl7#  MELODY1_E2_START_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY1_E2_START_CON 198
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_E2_START_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_MELODY1_E2_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | MELODY1_E2_STOP_CON      |"
   //COND/
    "#@Fdl7#  MELODY1_E2_STOP_CON"
   End header */
//ID/
#define TRL1_MMI_MELODY1_E2_STOP_CON 199
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_MELODY1_E2_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | L1_BACK_MELODY_E2        |            melody_id: #@1dl10#        nb_instrument: #@2d#"
    "        |  |  |  |  |     _LOAD_INSTRUMENT_REQ |       waves_table_id: #@3dr3#"
   //COND/
    "#@Fdl7#  L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ"
   End header */
//ID/
#define TRL1_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ 200
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           melody_id;
  UWORD8           number_of_instrument;
  UWORD8           waves_table_id[8];
}
T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | L1_BACK_MELODY_E2        |            melody_id: #@1d#"
    "        |  |  |  |  |     _LOAD_INSTRUMENT_CON |"
   //COND/
    "#@Fdl7#  L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON"
   End header */
//ID/
#define TRL1_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON 201
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           melody_id;
}
T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |---->|  |  | L1_BACK_MELODY_E2        |            melody_id: #@1dl10#        nb_instrument: #@2d#"
    "        |  |  |  |  |   _UNLOAD_INSTRUMENT_REQ |"
   //COND/
    "#@Fdl7#  L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ"
   End header */
//ID/
#define TRL1_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ 202
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           melody_id;
  UWORD8           number_of_instrument;
}
T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |<----|  |  | L1_BACK_MELODY_E2        |            melody_id: #@1d#"
    "        |  |  |  |  |   _UNLOAD_INSTRUMENT_CON |"
   //COND/
    "#@Fdl7#  L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON"
   End header */
//ID/
#define TRL1_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON 203
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD8           melody_id;
}
T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_MELODY0_E2_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |   L1_MELODY0_E2_STOP_CON |"
   //COND/
    "#@Fdl7#  L1_MELODY0_E2_STOP_CON"
   End header */
//ID/
#define TRL1_L1_MELODY0_E2_STOP_CON 204
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_MELODY0_E2_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_MELODY1_E2_STOP_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |   L1_MELODY1_E2_STOP_CON |"
   //COND/
    "#@Fdl7#  L1_MELODY1_E2_STOP_CON"
   End header */
//ID/
#define TRL1_L1_MELODY1_E2_STOP_CON 205
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_MELODY1_E2_STOP_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ RECOVERY
   //ERROR/ REC
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  | *** |      ***RECOVERY***      |"
   //COND/
    "#@Fdl7#                                                  RECOVERY"
   End header */
//ID/
#define TRL1_RECOVERY 206
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_RECOVERY;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ PTCCH DISABLE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |  0  |                          | WARNING: PTCCH disabled by L1S (TA_TN doesn't match with allocated resources)"
   //COND/
    "#@Fdl7#                          PTCCH disabled by L1S"
   End header */
//ID/
#define TRL1_PTCCH_DISABLE 207
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_PTCCH_DISABLE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MPHC_STOP_DEDICATED_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |<----|  |  | STOP_DEDICATED_CON       |"
   //COND/
    "#@Fdl7#  STOP_DEDICATED_CON"
   End header */
//ID/
#define TRL1_MPHC_STOP_DEDICATED_CON 218
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MPHC_STOP_DEDICATED_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1C_STOP_DEDICATED_DONE
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |     |<-|  | L1C_STOP_DEDICATED_DONE  |"
   //COND/
    "#@Fdl7#  L1C_STOP_DEDICATED_DONE"
   End header */
//ID/
#define TRL1_L1C_STOP_DEDICATED_DONE 219
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1C_STOP_DEDICATED_DONE;

#if (L1_VOCODER_IF_CHANGE == 1)
/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TCH_VOCODER_CFG_REQ
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |---->|  |  | MMI_TCH_VOCODER_CFG_REQ      |"
   //COND/
    "#@Fdl7#  MMI_TCH_VOCODER_CFG_REQ"
   End header */
//ID/
#define TRL1_MMI_TCH_VOCODER_CFG_REQ 220
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_TCH_VOCODER_CFG_REQ;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ MMI_TCH_VOCODER_CFG_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |     |<-|  | MMI_TCH_VOCODER_CFG_CON  |"
   //COND/
    "#@Fdl7#  MMI_TCH_VOCODER_CFG_CON"
   End header */
//ID/
#define TRL1_MMI_TCH_VOCODER_CFG_CON 221
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_MMI_TCH_VOCODER_CFG_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_VOCODER_CFG_ENABLE_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |   L1_VOCODER_CFG_ENABLE_CON |"
   //COND/
    "#@Fdl7#  L1_VOCODER_CFG_ENABLE_CON"
   End header */
//ID/
#define TRL1_L1_VOCODER_CFG_ENABLE_CON 222
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_VOCODER_CFG_ENABLE_CON;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ L1_VOCODER_CFG_DISABLE_CON
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |  |<-|  |   L1_VOCODER_CFG_DISABLE_CON |"
   //COND/
    "#@Fdl7#  L1_VOCODER_CFG_DISABLE_CON"
   End header */
//ID/
#define TRL1_L1_VOCODER_CFG_DISABLE_CON 223
//STRUCT/
typedef struct
{
  UWORD32          header;
}
T_TR_L1_VOCODER_CFG_DISABLE_CON;
#endif

/***********************************************************************************************************/
/* Begin header
   //TYPE/ CLASSIC
   //NAME/ SAIC Debug
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                  SAIC    |            SWH_flag: #@1d#"
   //COND/
    "#@Fdl7#                          New TOA"
   End header */
//ID/
#define TRL1_SAIC_DEBUG 224
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  UWORD32          SWH_flag;
  UWORD32          SAIC_flag;
}
T_TR_SAIC_DEBUG;


#define TRL1_BURST_PARAM 225
//STRUCT/
typedef struct
{
  UWORD32          header;
//--------------------------------------------------
  WORD16           angle;
  UWORD16          snr;
  WORD16           afc;
  UWORD16          pm;
  UWORD16          toa;
  UWORD8           task;
  UWORD8           input_level;
}
T_TR_BURST_PARAM;

//NAVC

#define TRL1_L1_NAVC  226
typedef struct 
{
  UWORD32 status;
  UWORD32 energy_level;
} 
T_TR_NAVC_PARAM;

/***********************************************************************************************************/
/* L1 RTT                                                                                                  */
/***********************************************************************************************************/

// Trace structures:
// -----------------
// For 32 bit alignment, all structures should be mapped like this:
//  1- 32-bit words (arrays of 32-bit words included)
//  2- 16-bit words (arrays of 16-bit words included)
//  3-  8-bit words (arrays of 8-bit words included)
//  4- HOLES permitting to obtain a cell size aligned on 32 bits (multiple of 4 b) !!!
//  5- cell_id (8 bit)
// This permits to avoid holes between variables and to have a structure independant of
// alignment


//-----------------------------------------------------------------------------------------------------------
// L1 RTT API function management
//-----------------------------------------------------------------------------------------------------------

// Dummy functions
#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
T_RTT_RET rtt_create_dummy (T_RVT_NAME      name[],
                            T_RTT_USER_ID *rtt_user_id,
                            T_RTT_MAX_EVT  nb_max_events,
                            T_RTT_BUF      buf_ptr,
                            T_RTT_SIZE     buf_size,
                            void          *callback);

T_RTT_RET rtt_refresh_status_dummy (T_RTT_USER_ID  user_id);

T_RTT_PTR rtt_get_fill_ptr_dummy (T_RTT_USER_ID  user_id,
                                  T_RTT_SIZE     size);

T_RTT_RET rtt_dump_buffer_dummy (T_RTT_USER_ID  user_id,
                                 T_RTT_SIZE     dump_size);
#endif

//-----------------------------------------------------------------------------------------------------------
// L1 RTT cell definitions
//-----------------------------------------------------------------------------------------------------------

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ FN
   //FULL/
   //COND/
   End header */
//ID//
#define RTTL1_ENABLE_FN 0
//STRUCT/
typedef struct
{
  UWORD32     fn;
  UWORD16     hole1;
  UWORD8      hole2;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_FN;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ DL Burst
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |<-|                    DL NB |       angle: #@1dl10#  snr: #@2dl10#  pm: #@5d#"
    "        |  |  |  |  |                          |         afc: #@3dl10# task: #@4tl10# toa: #@6d#"
    "        |  |  |  |  |                          | input_level: #-@7 / 2f1# dBm"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_DL_BURST 1
//STRUCT/
typedef struct
{
  WORD16      angle;
  UWORD16     snr;
  WORD16      afc;
  UWORD8      task;
  UWORD8      pm;
  UWORD8      toa;
  UWORD8      input_level;
  UWORD8      hole;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_DL_BURST;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ UL Normal Burst
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |->|                    UL NB | task: #@1tl10# ta: #@2dl10# txpwr: #@3d#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_UL_NB 2
//STRUCT/
typedef struct
{
  UWORD8      task;
  UWORD8      ta;
  UWORD8      txpwr;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_UL_NB;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ UL Access Burst
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |->|                    UL AB | task: #@1tl10# txpwr: #@3d#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_UL_AB 3
//STRUCT/
typedef struct
{
  UWORD8      task;
  UWORD8      txpwr;
  UWORD8      hole;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_UL_AB;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ Full list Meas
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |<-|                    DL MS | type: #@4=200:Full list|=201:Idle BA list|=202:Dedicated BA list|=203:MS AGC|=204:Cell selection|=205:Transfer cell selection|=206:Beacon monitoring|#"
    "        |  |  |  |  |                          | radio_freq: #@1dl10# pm: #@2dl10# input_level: #-@3 / 2f1#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_FULL_LIST_MEAS 4
//STRUCT/
typedef struct
{
  UWORD16     radio_freq;
  UWORD8      pm;
  UWORD8      input_level;
  UWORD8      task;
  UWORD8      hole1;
  UWORD8      hole2;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_FULL_LIST_MEAS;

/***********************************************************************************************************/
/* Could replace valid_flag by 3 x UWORD16 dsp_header */

/* Begin header
   //TYPE/ RTT
   //NAME/ DL DCCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |<----|  |                  DL DCCH | valid_flag: #@1=0:OK|=1:CRC ERROR|l10# physical_info: #@2=255:NONE|#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_DL_DCCH 5
//STRUCT/
typedef struct
{
  BOOL        valid_flag;
  UWORD8      physical_info; // if 255 no physical info else ta
  UWORD8      hole;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_DL_DCCH;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ DL PTCCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |<-|                 DL PTCCH |  crc: #@1=0:OK|=1:CRC ERROR|l10# ordered_ta: #@2>63:INVALID|#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_DL_PTCCH 6
//STRUCT/
typedef struct
{
  BOOL        crc;
  UWORD8      ordered_ta;
  UWORD8      hole;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_DL_PTCCH;

/***********************************************************************************************************/
/*  Could add 23 x UWORD8 data */
/* Begin header
   //TYPE/ RTT
   //NAME/ UL DCCH
   //FULL/
    "        |  |  |  |  |                          |"
    "#@Fdl7# |  |---->|  |                  UL DCCH |"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_UL_DCCH 7
//STRUCT/
typedef struct
{
  UWORD8      hole1;
  UWORD8      hole2;
  UWORD8      hole3;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_UL_DCCH;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ UL SACCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |---->|  |                 UL SACCH | data_present: #@1=1:Yes|=0:No|~|l10# reported_ta: #@2dl10# reported_txpwr: #@3d#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_UL_SACCH 8
//STRUCT/
typedef struct
{
  BOOL        data_present;
  UWORD8      reported_ta;
  UWORD8      reported_txpwr;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_UL_SACCH;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ DL PDTCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |<----|  |                 DL PDTCH |   timeslot: #@5dl10#     crc: #@3=0:OK|>0:CRC ERROR|#"
    "        |  |  |  |  |                          | mac_header: #@1xl10# cs_type: #@4=2:CS1|=4:CS2|=5:CS3|=6:CS4|~N/A|#"
    "        |  |  |  |  |                          | tfi_result: #@2=0:No filtering|=1:NO TFI|=2:Addressed to MS|=3:Not addressed to MS|#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_DL_PDTCH 9
//STRUCT/
typedef struct
{
  UWORD8      mac_header;
  UWORD8      tfi_result;
  BOOL        crc;
  UWORD8      cs_type;
  UWORD8      timeslot;
  UWORD8      hole1;
  UWORD8      hole2;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_DL_PDTCH;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ UL PDTCH
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |---->|  |                 UL PDTCH |  timeslot: #@3dl10# data_allowed: #@2=0:NO|=1:YES|~N/A|#"
    "        |  |  |  |  |                          |   cs_type: #@1=2:CS1|=3:CS1 POLL|=4:CS2|=5:CS3|=6:CS4|=7:PRACH 8 bit|=8:PRACH 11 bit|~N/A|#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_UL_PDTCH 10
//STRUCT/
typedef struct
{
  UWORD8      cs_type;
  BOOL        data_allowed;
  UWORD8      timeslot;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_UL_PDTCH;

/***********************************************************************************************************/

#define POLL_REJECT       0
#define TX_ALLOWED_NO_BLK 1
#define TX_CANCELLED_POLL 2
#define TX_CANCELLED_USF  3

/* Begin header
   //TYPE/ RTT
   //NAME/ MAC-S Status
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                    MAC-S | #@1=0:Poll rejected|=1:TX allowed and no block given by RLC|=2:TX cancelled for poll|=3:TX cancelled (bad USF)|# on timeslot #@2d#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_MACS_STATUS 11
//STRUCT/
typedef struct
{
  UWORD8      status;
  UWORD8      timeslot;
  UWORD8      hole;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_MACS_STATUS;

/***********************************************************************************************************/
/* Special trace: display is implemented in the trace decoder
 */
#define RTTL1_ENABLE_L1S_TASK_ENABLE 12

typedef struct
{
  UWORD32     bitmap1;
  UWORD32     bitmap2;
  UWORD8      hole1;
  UWORD8      hole2;
  UWORD8      hole3;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_L1S_TASK_ENABLE;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ Neighbor monitoring meas
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  |<-|                    DL MS | type: #@4=200:Full list|=201:Idle BA list|=202:Dedicated BA list|=203:MS AGC|=204:Cell selection|=205:Transfer cell selection|=206:Beacon monitoring|#"
    "        |  |  |  |  |                          | radio_freq: #@1dl10# pm: #@2dl10# input_level: #-@3 / 2f1#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_MON_MEAS 13
//STRUCT/
typedef struct
{
  UWORD16     radio_freq;
  UWORD8      pm;
  UWORD8      input_level;
  UWORD8      task;
  UWORD8      hole1;
  UWORD8      hole2;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_MON_MEAS;

/***********************************************************************************************************/
/* Begin header
   //TYPE/ RTT
   //NAME/ MFTAB
   //FULL/
    "        |  |  |  |  |                          |----------------------------------------------------------------------------------------------------------------"
    "#@Fdl7# |  |  |  O  |                          | #@1T[MFTAB]#"
   //COND/
   End header */
//ID/
#define RTTL1_ENABLE_MFTAB 14
//STRUCT/
typedef struct
{
  UWORD8      func;
  UWORD8      hole1;
  UWORD8      hole2;
//--------------------------------------------------
  UWORD8      cell_id;
} T_RTTL1_MFTAB;

/***************************************************/
/* General structure: must contain all trace cells */
/***************************************************/

typedef struct
{
  // Classic cells
  T_TR_MPHC_INIT_L1_REQ                         cell1;
  T_TR_MPHC_INIT_L1_CON                         cell2;
  T_TR_MPHC_RXLEV_PERIODIC_REQ                  cell3;
  T_TR_MPHC_NCELL_FB_SB_READ                    cell4;
  T_TR_MPHC_RA_CON                              cell5;
  T_TR_MPHC_IMMED_ASSIGN_REQ                    cell6;
  T_TR_MPHC_CHANNEL_ASSIGN_REQ                  cell7;
  T_TR_MPHC_RA_REQ                              cell8;
  T_TR_MPHC_ASYNC_HO_REQ                        cell9;
  T_TR_MPHC_SYNC_HO_REQ                         cell10;
  T_TR_L1C_HANDOVER_FINISHED                    cell11;
  T_TR_L1C_MEAS_DONE                            cell12;
  T_TR_MPHC_START_CCCH_REQ                      cell13;
  T_TR_MPHC_NCELL_SB_READ                       cell14;
  T_TR_MPHC_RXLEV_REQ                           cell15;
  T_TR_L1C_VALID_MEAS_INFO                      cell16;
  T_TR_L1C_RXLEV_PERIODIC_DONE                  cell17;
  T_TR_MPHC_SCELL_NBCCH_REQ                     cell18;
  T_TR_MPHC_SCELL_EBCCH_REQ                     cell19;
  T_TR_MPHC_NCELL_BCCH_REQ                      cell20;
  T_TR_L1C_BCCHN_INFO                           cell21;
  T_TR_L1C_NP_INFO                              cell22;
  T_TR_L1C_EP_INFO                              cell23;
  T_TR_L1C_ALLC_INFO                            cell24;
  T_TR_L1C_BCCHS_INFO                           cell25;
  T_TR_L1C_CB_INFO                              cell26;
  T_TR_MPHC_NETWORK_SYNC_REQ                    cell27;
  T_TR_MPHC_NETWORK_SYNC_IND                    cell28;
  T_TR_MPHC_NCELL_SYNC_REQ                      cell29;
  T_TR_MPHC_NCELL_SYNC_IND                      cell30;
  T_TR_L1C_SB_INFO                              cell31;
  T_TR_L1C_SBCONF_INFO                          cell32;
  T_TR_MPHC_NEW_SCELL_REQ                       cell33;
  T_TR_L1C_FB_INFO                              cell34;
  T_TR_MPHC_STOP_NCELL_SYNC_REQ                 cell35;
  T_TR_MPHC_STOP_NCELL_BCCH_REQ                 cell36;
  T_TR_MPHC_CONFIG_CBCH_REQ                     cell37;
  T_TR_MPHC_CBCH_SCHEDULE_REQ                   cell38;
  T_TR_MPHC_CBCH_INFO_REQ                       cell39;
  T_TR_MPHC_CBCH_UPDATE_REQ                     cell40;
  T_TR_MPHC_STOP_CBCH_REQ                       cell41;
  T_TR_L1C_SACCH_INFO                           cell42;
  T_TR_MPHC_CHANGE_FREQUENCY                    cell43;
  T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ             cell44;
  T_TR_MPHC_SET_CIPHERING_REQ                   cell45;
  T_TR_MPHC_UPDATE_BA_LIST                      cell46;
  T_TR_MPHC_NETWORK_LOST_IND                    cell47;
  T_TR_MPHC_STOP_CCCH_REQ                       cell48;
  T_TR_MPHC_STOP_SCELL_BCCH_REQ                 cell49;
  T_TR_MPHC_STOP_CBCH_CON                       cell50;
  T_TR_MPHC_STOP_RA_REQ                         cell51;
  T_TR_L1C_RA_DONE                              cell52;
  T_TR_MPHC_IMMED_ASSIGN_CON                    cell53;
  T_TR_MPHC_CHANNEL_ASSIGN_CON                  cell54;
  T_TR_L1C_REDEF_DONE                           cell55;
  T_TR_MPHC_STOP_DEDICATED_REQ                  cell56;
  T_TR_MPHC_ASYNC_HO_CON                        cell57;
  T_TR_MPHC_SYNC_HO_CON                         cell58;
  T_TR_MPHC_TA_FAIL_IND                         cell59;
  T_TR_MPHC_HANDOVER_FAIL_REQ                   cell60;
  T_TR_MPHC_HANDOVER_FAIL_CON                   cell61;
  T_TR_MPHC_STOP_RXLEV_REQ                      cell62;
  T_TR_MPHC_STOP_RXLEV_PERIODIC_REQ             cell63;
  T_TR_MPHP_RA_REQ                              cell64;
  T_TR_L1P_RA_DONE                              cell65;
  T_TR_MPHP_POLLING_RESPONSE_REQ                cell66;
  T_TR_L1P_POLL_DONE                            cell67;
  T_TR_MPHP_ASSIGNMENT_REQ                      cell68;
  T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ           cell69;
  T_TR_L1P_REPEAT_ALLOC_DONE                    cell70;
  T_TR_L1P_ALLOC_EXHAUST_DONE                   cell71;
  T_TR_MPHP_SINGLE_BLOCK_REQ                    cell72;
  T_TR_L1P_SINGLE_BLOCK_CON                     cell73;
  T_TR_MPHP_PDCH_RELEASE_REQ                    cell74;
  T_TR_MPHP_TIMING_ADVANCE_REQ                  cell75;
  T_TR_MPHP_TBF_RELEASE_REQ                     cell76;
  T_TR_MPHP_START_PCCCH_REQ                     cell77;
  T_TR_L1P_PBCCHN_INFO                          cell78;
  T_TR_L1P_PNP_INFO                             cell79;
  T_TR_L1P_PEP_INFO                             cell80;
  T_TR_L1P_PALLC_INFO                           cell81;
  T_TR_L1P_PBCCHS_INFO                          cell82;
  T_TR_L1P_PACCH_INFO                           cell83;
  T_TR_MPHP_SCELL_PBCCH_REQ                     cell84;
  T_TR_MPHP_CR_MEAS_REQ                         cell85;
  T_TR_L1P_CR_MEAS_DONE                         cell86;
  T_TR_MPHP_INT_MEAS_REQ                        cell87;
  T_TR_MPHP_INT_MEAS_IND                        cell88;
  T_TR_MPHP_TINT_MEAS_IND                       cell89;
  T_TR_L1P_ITMEAS_IND                           cell90;
  T_TR_MPHP_NCELL_PBCCH_REQ                     cell91;
  T_TR_MPHP_UPDATE_PSI_PARAM_REQ                cell92;
  T_TR_L1P_TBF_RELEASED                         cell93;
  T_TR_L1P_PDCH_RELEASED                        cell94;
  T_TR_L1P_TCR_MEAS_DONE                        cell95;
  T_TR_MPHP_ASSIGNMENT_CON                      cell96;
  T_TR_MPHP_TCR_MEAS_REQ                        cell97;
  T_TR_MPHC_STOP_NETWORK_SYNC_REQ               cell98;
  T_TR_MPHP_NCELL_PBCCH_STOP_REQ                cell99;
  T_TR_MPHP_STOP_PCCCH_REQ                      cell100;
  T_TR_MPHP_SCELL_PBCCH_STOP_REQ                cell101;
  T_TR_MPHP_RA_STOP_REQ                         cell102;
  T_TR_MPHP_STOP_SINGLE_BLOCK_REQ               cell103;
  T_TR_L1P_TA_CONFIG_DONE                       cell104;
  T_TR_MPHP_CR_MEAS_STOP_REQ                    cell105;
  T_TR_MPHP_TCR_MEAS_STOP_REQ                   cell106;
  T_TR_MPHP_INT_MEAS_STOP_REQ                   cell107;
  T_TR_MMI_KEYBEEP_START_REQ                    cell108;
  T_TR_MMI_KEYBEEP_START_CON                    cell109;
  T_TR_MMI_KEYBEEP_STOP_REQ                     cell110;
  T_TR_MMI_KEYBEEP_STOP_CON                     cell111;
  T_TR_MMI_TONE_START_REQ                       cell112;
  T_TR_MMI_TONE_START_CON                       cell113;
  T_TR_MMI_TONE_STOP_REQ                        cell114;
  T_TR_MMI_TONE_STOP_CON                        cell115;
  T_TR_MMI_MELODY0_START_REQ                    cell116;
  T_TR_MMI_MELODY1_START_REQ                    cell117;
  T_TR_MMI_MELODY0_START_CON                    cell118;
  T_TR_MMI_MELODY0_STOP_REQ                     cell119;
  T_TR_MMI_MELODY0_STOP_CON                     cell120;
  T_TR_MMI_MELODY1_START_CON                    cell121;
  T_TR_MMI_MELODY1_STOP_REQ                     cell122;
  T_TR_MMI_MELODY1_STOP_CON                     cell123;
  T_TR_MMI_VM_PLAY_START_REQ                    cell124;
  T_TR_MMI_VM_PLAY_START_CON                    cell125;
  T_TR_MMI_VM_PLAY_STOP_REQ                     cell126;
  T_TR_MMI_VM_PLAY_STOP_CON                     cell127;
  T_TR_MMI_VM_RECORD_START_REQ                  cell128;
  T_TR_MMI_VM_RECORD_START_CON                  cell129;
  T_TR_MMI_VM_RECORD_STOP_REQ                   cell130;
  T_TR_MMI_VM_RECORD_STOP_CON                   cell131;
  T_TR_MMI_SR_ENROLL_START_REQ                  cell132;
  T_TR_MMI_SR_ENROLL_START_CON                  cell133;
  T_TR_MMI_SR_ENROLL_STOP_REQ                   cell134;
  T_TR_MMI_SR_ENROLL_STOP_CON                   cell135;
  T_TR_MMI_SR_UPDATE_START_REQ                  cell136;
  T_TR_MMI_SR_UPDATE_STOP_REQ                   cell137;
  T_TR_MMI_SR_UPDATE_START_CON                  cell138;
  T_TR_MMI_SR_UPDATE_STOP_CON                   cell139;
  T_TR_MMI_SR_RECO_START_REQ                    cell140;
  T_TR_MMI_SR_RECO_STOP_REQ                     cell141;
  T_TR_MMI_SR_RECO_START_CON                    cell142;
  T_TR_MMI_SR_RECO_STOP_CON                     cell143;
  T_TR_MMI_SR_UPDATE_CHECK_START_REQ            cell144;
  T_TR_MMI_SR_UPDATE_CHECK_STOP_REQ             cell145;
  T_TR_MMI_SR_UPDATE_CHECK_START_CON            cell146;
  T_TR_MMI_SR_UPDATE_CHECK_STOP_CON             cell147;
  T_TR_L1_SRBACK_SAVE_DATA_REQ                  cell148;
  T_TR_L1_SRBACK_SAVE_DATA_CON                  cell149;
  T_TR_L1_SRBACK_LOAD_MODEL_REQ                 cell150;
  T_TR_L1_SRBACK_LOAD_MODEL_CON                 cell151;
  T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ             cell152;
  T_TR_L1_SRBACK_TEMP_SAVE_DATA_CON             cell153;
  T_TR_MMI_AEC_REQ                              cell154;
  T_TR_MMI_AEC_CON                              cell155;
  T_TR_MMI_AUDIO_FIR_REQ                        cell156;
  T_TR_MMI_AUDIO_FIR_CON                        cell157;
  T_TR_MMI_AUDIO_MODE_REQ                       cell158;
  T_TR_MMI_AUDIO_MODE_CON                       cell159;
  T_TR_PM_EQUAL_0                               cell160;
  T_TR_MCU_DSP_MISMATCH                         cell161;
  T_TR_L1S_ABORT                                cell162;
  T_TR_D_ERROR_STATUS                           cell163;
  T_TR_DSP_DEBUG_HEADER                         cell164;
  T_TR_DSP_DEBUG_BUFFER                         cell165;
  T_TR_RLC_UL_PARAM                             cell166;
  T_TR_RLC_DL_PARAM                             cell167;
  T_TR_FORBIDDEN_UPLINK                         cell168;
  T_TR_DL_PTCCH                                 cell169;
  T_TR_CONDENSED_PDTCH                          cell170;
  T_TR_OML1_CLOSE_TCH_LOOP_REQ                  cell171;
  T_TR_OML1_OPEN_TCH_LOOP_REQ                   cell172;
  T_TR_OML1_START_DAI_TEST_REQ                  cell173;
  T_TR_OML1_STOP_DAI_TEST_REQ                   cell174;
  T_TR_TST_TEST_HW_REQ                          cell175;
  T_TR_L1_TEST_HW_INFO                          cell176;
  T_TR_TST_SLEEP_REQ                            cell177;
  T_TR_MMI_ADC_REQ                              cell178;
  T_TR_MMI_STOP_ADC_REQ                         cell179;
  T_TR_MMI_STOP_ADC_CON                         cell180;
  T_TR_L1S_CPU_PEAK                             cell181;
  T_TR_TRACE_CONFIG_CHANGE                      cell182;
  T_TR_ASCII                                    cell183;
  T_TR_FULL_LIST_REPORT                         cell184;
  T_TR_IT_DSP_ERROR                             cell185;
  T_TR_ADC                                      cell186;
  T_TR_NEW_TOA                                  cell187;
  T_TR_TOA_NOT_UPDATED                          cell188;
  T_TR_SLEEP                                    cell189;
  T_TR_GAUGING                                  cell190;
  T_TR_UNKNOWN_L1S_TRACE                        cell191;
  T_TR_MMI_MELODY0_E2_START_REQ                 cell192;
  T_TR_MMI_MELODY0_E2_STOP_REQ                  cell193;
  T_TR_MMI_MELODY0_E2_START_CON                 cell194;
  T_TR_MMI_MELODY0_E2_STOP_CON                  cell195;
  T_TR_MMI_MELODY1_E2_START_REQ                 cell196;
  T_TR_MMI_MELODY1_E2_STOP_REQ                  cell197;
  T_TR_MMI_MELODY1_E2_START_CON                 cell198;
  T_TR_MMI_MELODY1_E2_STOP_CON                  cell199;
  T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ    cell200;
  T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON    cell201;
  T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ  cell202;
  T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON  cell203;
  T_TR_L1_MELODY0_E2_STOP_CON                   cell204;
  T_TR_L1_MELODY1_E2_STOP_CON                   cell205;
  T_TR_RECOVERY                                 cell206;
  T_TR_PTCCH_DISABLE                            cell207;
  T_TR_L1_AEC_IND                               cell208;
  T_TR_MMI_VM_AMR_PLAY_START_REQ                cell209;
  T_TR_MMI_VM_AMR_PLAY_START_CON                cell210;
  T_TR_MMI_VM_AMR_PLAY_STOP_REQ                 cell211;
  T_TR_MMI_VM_AMR_PLAY_STOP_CON                 cell212;
  T_TR_MMI_VM_AMR_RECORD_START_REQ              cell213;
  T_TR_MMI_VM_AMR_RECORD_START_CON              cell214;
  T_TR_MMI_VM_AMR_RECORD_STOP_REQ               cell215;
  T_TR_MMI_VM_AMR_RECORD_STOP_CON               cell216;
  T_TR_MMI_VM_AMR_PAUSE_REQ                     cell227;
  T_TR_MMI_VM_AMR_RESUME_REQ                    cell228;
  T_TR_MMI_VM_AMR_PAUSE_CON                     cell229;
  T_TR_MMI_VM_AMR_RESUME_CON                    cell230;
  T_TR_MPHC_NCELL_LIST_SYNC_REQ                 cell217;
  T_TR_MPHC_STOP_DEDICATED_CON                  cell218;
  T_TR_L1C_STOP_DEDICATED_DONE                  cell219;
  #if (L1_VOCODER_IF_CHANGE == 1) 
    T_TR_MMI_TCH_VOCODER_CFG_REQ                  cell220;
    T_TR_MMI_TCH_VOCODER_CFG_CON                  cell221;
    T_TR_L1_VOCODER_CFG_ENABLE_CON                cell222;
    T_TR_L1_VOCODER_CFG_DISABLE_CON               cell223;
  #endif  
  T_TR_SAIC_DEBUG                               cell224;
  T_TR_BURST_PARAM                              cell225;

  // RTT cells
  T_RTTL1_FN                                    rttcell1;
  T_RTTL1_DL_BURST                              rttcell2;
  T_RTTL1_UL_NB                                 rttcell3;
  T_RTTL1_UL_AB                                 rttcell4;
  T_RTTL1_FULL_LIST_MEAS                        rttcell5;
  T_RTTL1_DL_DCCH                               rttcell6;
  T_RTTL1_DL_PTCCH                              rttcell7;
  T_RTTL1_UL_DCCH                               rttcell8;
  T_RTTL1_UL_SACCH                              rttcell9;
  T_RTTL1_DL_PDTCH                              rttcell10;
  T_RTTL1_UL_PDTCH                              rttcell11;
  T_RTTL1_MACS_STATUS                           rttcell12;
  T_RTTL1_L1S_TASK_ENABLE                       rttcell13;
  T_RTTL1_MON_MEAS                              rttcell14;
  T_RTTL1_MFTAB                                 rttcell15;
}
T_TRACE_CELLS;


/************************************/
/* RTT macro definitions            */
/************************************/
#include "l1_rtt_macro.h"

#if (L1_FF_MULTIBAND == 1)
#if ( (TRACE_TYPE == 1) || (TRACE_TYPE==4) )
#define L1_MULTIBAND_TRACE_PARAMS            l1_multiband_trace_params
#elif (TRACE_TYPE == 5)
#define L1_MULTIBAND_TRACE_PARAMS            l1_multiband_trace_params_simu
#endif
#define MULTIBAND_PHYSICAL_BAND_TRACE_ID 0
#define MULTIBAND_ERROR_TRACE_ID     1
#endif /*if (L1_FF_MULTIBAND == 1)*/ 


#endif

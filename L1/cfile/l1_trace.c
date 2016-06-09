/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_TRACE.C
 *
 *        Filename l1_trace.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_TRACE_C

//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

#include "config.h"

#include "l1_confg.h"
#include "l1_macro.h"

#include <string.h>
#include "l1_types.h"
#include "sys_types.h"
#include "l1_const.h"
#include "l1_signa.h"

#if (AUDIO_TASK == 1)
  #include "l1audio_const.h"
  #include "l1audio_cust.h"
  #include "l1audio_signa.h"
  #include "l1audio_defty.h"
  #include "l1audio_msgty.h"
#endif

#if TESTMODE
  #include "l1tm_defty.h"
#endif

#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
  #include "l1gtt_msgty.h"
  #include "l1gtt_signa.h"
#endif

#if (L1_MP3 == 1)
  #include "l1mp3_defty.h"
  #include "l1mp3_signa.h"
  #include "l1mp3_msgty.h"
#endif

#if (L1_MIDI == 1)
  #include "l1midi_defty.h"
  #include "l1midi_signa.h"
  #include "l1midi_msgty.h"
#endif

#if (L1_AAC == 1)
  #include "l1aac_defty.h"
  #include "l1aac_signa.h"
  #include "l1aac_msgty.h"
#endif
#if (L1_DYN_DSP_DWNLD == 1)
  #include "l1_dyn_dwl_signa.h"
  #include "l1_dyn_dwl_msgty.h"
#endif

#include "l1_defty.h"
#include "../../gpf/inc/cust_os.h"
#include "l1_msgty.h"
#include "l1_varex.h"
#include "l1_proto.h"
#include "l1_mftab.h"
#include "l1_tabs.h"
#include "l1_ver.h"
#include "../../bsp/ulpd.h"

#if TESTMODE
  #include "l1tm_msgty.h"
  #include "l1tm_signa.h"
  #include "l1tm_varex.h"
#endif

#include "../../bsp/mem.h"
#if ( CHIPSET == 12 ) || (CHIPSET == 15)
   #include "sys_inth.h"
#else
   #include "../../bsp/iq.h"
   #include "../../bsp/inth.h"
#endif

#if L1_GPRS
  #include "l1p_cons.h"
  #include "l1p_msgt.h"
  #include "l1p_deft.h"
  #include "l1p_vare.h"
  #include "l1p_sign.h"
  #include "l1p_ver.h"
#endif

#if (L1_DRP == 1)
  extern UWORD32 drp_ref_sw_tag;
  extern UWORD32 drp_ref_sw_ver;
#endif

#include <string.h>
#include <stdio.h>

#include "../../serial/serialswitch.h"
#include "../../riviera/rv/rv_trace.h"
#include "../../riviera/rvf/rvf_api.h"

//void dt_trace_event(UWORD16 id, char *fmt_string, ...);
#if (TRACE_TYPE==7)
  #include "../../bsp/timer2.h"
#endif

unsigned int x,y,fer_sacch ,dl_good_norep;
extern UWORD16 toa_tab[4];

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)  || (TRACE_TYPE == 7)
  #include "l1_trace.h"
#endif
#if FF_TBF
  #if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
    #include "../../bsp/armio.h" // for GPIO toggle
  #endif
#endif

#if (GSM_IDLE_RAM != 0)
  #if (OP_L1_STANDALONE == 1)
    #include "csmi_simul.h"
  #else
    #include "csmi/sleep.h"
  #endif
  void l1_intram_put_trace(CHAR * msg);
  void l1s_trace_mftab(void);
  void l1s_keep_mftab_hist(void);
  BOOL l1s_mftab_has_changed(void);
#endif

#if (TRACE_TYPE == 5)
  #include "l1_trace.h"
#endif

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)  || (TRACE_TYPE == 7)
  #include "l1_trace.h"

    extern T_TRACE_INFO_STRUCT trace_info;
  void l1_trace_configuration(T_RVT_BUFFER trace_msg, UINT16 trace_msg_size);
  void l1_send_trace_version();
  void l1_trace_full_dsp_buffer( void );

  #if L1_BINARY_TRACE
    #define L1_send_trace_cpy(s,length)    rvt_send_trace_cpy       ((T_RVT_BUFFER) s, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)length, RVT_BINARY_FORMAT)
    #define L1_send_trace_no_cpy(s,length) rvt_send_trace_no_cpy    ((T_RVT_BUFFER) s, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)length, RVT_BINARY_FORMAT)
  #else
    #define L1_send_trace_cpy(s)           rvt_send_trace_cpy       ((T_RVT_BUFFER) s, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)strlen(s), RVT_ASCII_FORMAT)
    #define L1_send_trace_no_cpy(s)        rvt_send_trace_no_cpy    ((T_RVT_BUFFER) s, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)strlen(s), RVT_ASCII_FORMAT)
  #endif
  // inform the DAR system by tracing a Warning (used to inform DAR system a Recovery)
  #define L1_send_trace_cpy_DAR(s) rvf_send_trace1       (s, (T_RVT_MSG_LG)strlen(s),0,RV_TRACE_LEVEL_WARNING,trace_info.l1_trace_user_id)// omaps00090550

  #define L1_send_low_level_trace(s)      SER_tr_EncapsulateNChars  (SER_LAYER_1, (char *)s, strlen(s));

#elif (TRACE_TYPE == 5)
  #include "l1_trace.h"

    extern T_TRACE_INFO_STRUCT trace_info;
#endif //(TRACE_TYPE == 5)

#if (TRACE_TYPE == 1)
  #include "../../bsp/timer2.h"

  // Variables for L1S CPU load measurement
  extern unsigned long        max_cpu, fn_max_cpu;
  extern unsigned short       layer_1_sync_end_time;
  extern unsigned short       max_cpu_flag;

  // Variables for DSP CPU load measurement
  extern unsigned short       l1_dsp_cpu_load_trace_flag;
  extern UWORD32              dsp_max_cpu_load_trace_array[4];
  extern UWORD32              dsp_max_cpu_load_idle_frame;
#endif // (TRACE_TYPE == 1)



// External variables for internal RAM download trace
//---------------------------------------------------

/* not applicable to FreeCalypso */
#if 0 //(( ((CHIPSET !=2 ) ) && ((LONG_JUMP != 0))) || (CHIPSET == 12) || (CHIPSET == 15))
  extern UWORD16 d_checksum1;
  extern UWORD16 d_checksum2;
#endif //(( ((CHIPSET !=2 ) ) && ((LONG_JUMP != 0))) || (CHIPSET == 12))

// External variables for L1S CPU load peaks trace
//------------------------------------------------

#if (TRACE_TYPE==4)
  extern UWORD32 max_cpu,fn_max_cpu;
  extern UWORD16 layer_1_sync_end_time;
  extern UWORD16 max_cpu_flag;

  // Variables for DSP CPU load measurement
  extern unsigned short l1_dsp_cpu_load_trace_flag;
  extern UWORD32        dsp_max_cpu_load_trace_array[4];
  extern UWORD32        dsp_max_cpu_load_idle_frame;
//  extern BOOL           l1_trace_enabled;
#endif //(TRACE_TYPE==4)

// External variables and definitions for L1S CPU load trace
//----------------------------------------------------------

#if  (TRACE_TYPE==7) // CPU_LOAD

  #define CPU_LOAD_TIMER_RESET_VALUE       (0xFFFF)
  #define C_PTV                            (0)
  #define CPU_LOAD_TICK                    (2.416)  // microsecond

  // prototype
  #if (GSM_IDLE_RAM != 0)
    void l1_trace_buf_meas(void);
  #else
    void l1_trace_buf_meas_intram(void);
  #endif //(GSM_IDLE_RAM != 0)

  // array that store the cpu load measurements for each TDMA.
  extern  T_MESURE              d_mesure[C_MESURE_DEPTH];
  extern  UWORD8                d_mesure_index;

#endif //(TRACE_TYPE==7)
#if (AUDIO_DEBUG == 1)
  #define    DSP_AUDIO_DEBUG_API_ADDR 0xFFD06BA6
  void Trace_l1_audio_regs();
  extern void l1_audio_regs_debug_read();
#endif
#if (OP_L1_STANDALONE == 1)
#ifdef _INLINE
  #define INLINE static inline // Inline functions when -v option is set
#else                          // when the compiler is ivoked.
  #define INLINE
#endif //INLINE
#endif  //omaps00090550
extern UWORD16 toa_tab[4];


#if(L1_DRC == 1)
  extern T_DRC_MCU_DSP *drc_ndb;
#endif

#if L1_BINARY_TRACE
  #if (DSP_DEBUG_TRACE_ENABLE == 1)
    #define DSP_DEBUG_ENABLE \
      if (trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] == 0) \
      { \
        trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] = l1s_dsp_com.dsp_db2_current_r_ptr->d_debug_ptr_begin; \
        trace_info.dsp_debug_fn[l1s_dsp_com.dsp_r_page]        = l1s.actual_time.fn;                                   \
        trace_info.dsp_debug_time[l1s_dsp_com.dsp_r_page]      = (UWORD16)l1s.debug_time;                              \
      }
  #endif // (DSP_DEBUG_TRACE_ENABLE)

#else
  #if (DSP_DEBUG_TRACE_ENABLE == 1)
    #define DSP_DEBUG_ENABLE \
      if (trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] == 0) \
      { \
        trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] = l1s_dsp_com.dsp_db2_current_r_ptr->d_debug_ptr_begin; \
        trace_info.dsp_debug_fn[l1s_dsp_com.dsp_r_page]        = l1s.actual_time.fn;                        \
        trace_info.dsp_debug_time[l1s_dsp_com.dsp_r_page]      = (UWORD16)l1s.debug_time;                            \
      }
  #endif // (DSP_DEBUG_TRACE_ENABLE)
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
  //#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

  #if  (TRACE_TYPE==7) // CPU_LOAD
    // array that store the cpu load measurements for each TDMA.
    static T_MESURE              d_mesure[C_MESURE_DEPTH];
    static UWORD8                d_mesure_index;
  #endif

  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)  || (TRACE_TYPE == 5) || (TRACE_TYPE == 7))
    T_TRACE_INFO_STRUCT trace_info;
  #endif

  #if (TRACE_TYPE == 1)
    #include "timer2.h"

    // Variables for L1S CPU load measurement
   unsigned long        max_cpu = 0, fn_max_cpu;
   unsigned short       layer_1_sync_end_time;
   unsigned short       max_cpu_flag = 0;

   // Variables for DSP CPU load measurement
   UWORD32              dsp_max_cpu_load_trace_array[4] = {0L, 0L, 0L, 0L};
   UWORD32              dsp_max_cpu_load_idle_frame = 0L;
   unsigned short       l1_dsp_cpu_load_trace_flag = 0;
  #endif //(TRACE_TYPE == 1)

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

#if ((GSM_IDLE_RAM != 0)) // Compiled only if (GSM_IDLE_RAM != 0) //omaps00090550
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START                                // Mapped automatically in the appropriate memory region
  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
    CHAR intram_trace_buffer[INTRAM_TRACE_BUFFER_SIZE]; // buffer containing temporary ASCII trace
    CHAR * intram_buffer_current_ptr;                   // pointer on next available CHAR available
    T_RVT_MSG_LG intram_trace_size;                     // size of the meaningful trace (circular buffer)
  #endif
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

#if (OP_L1_STANDALONE == 0)
  #if (TI_PROFILER == 1)
    // Returns the absolute frame number
    UWORD32 l1_get_next_absolute_fn(void)
    {
      return (l1s.next_time.fn);
    }
  #endif

  #if (TI_NUC_MONITOR == 1) || (WCP_PROF == 1)
    // Returns the absolute frame number
    UWORD16 l1_get_actual_fn_mod42432(void)
    {
      return (l1s.actual_time.fn_mod42432);
    }
  #endif

  #if (TI_PROFILER == 1) || (TI_NUC_MONITOR == 1)
    void SendTraces(char *s)
    {
      L1_send_trace_cpy(s);
    }
  #endif
#endif //(OP_L1_STANDALONE == 0)


#if ((OP_L1_STANDALONE == 1) && ((DSP == 38)|| (DSP == 39)) && (CODE_VERSION != SIMULATION))
   #include "clkm.h"

   void l1_api_dump(void);
   UWORD16 api_dump_cnvt_mcu_to_dsp(UWORD32 address);
#endif

#if ((TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==4) || (TRACE_TYPE==7))
   extern void L1_trace_string(char *s);
#endif

#if (BURST_PARAM_LOG_ENABLE == 1)

  #define  BURST_PARAM_LOG_BUFFER_LENGTH  32768

  typedef struct
  {
    UWORD16 fn_mod42432;
    UWORD16 l1_mode;
    UWORD16 task;
    UWORD16 SNR_val;
    UWORD16 TOA_val;
    UWORD16 angle;
    UWORD16 pm;
    UWORD16 IL_for_rxlev;
    UWORD16 l1s_afc;
    UWORD16 hole; // to ensure 32-bit alignment
  }T_burst_param_log_debug;

  //#pragma DATA_SECTION(burst_param_log_debug,".debug_data");
  T_burst_param_log_debug  burst_param_log_debug[BURST_PARAM_LOG_BUFFER_LENGTH];

  UWORD32  burst_param_log_index;

#endif

#if (CODE_VERSION!= SIMULATION)
#include "l1_pwmgr.h"
#endif //NOT SIMULATION


#if (TRACE_TYPE==1) || (TRACE_TYPE==4) || (TRACE_TYPE==5)

UWORD32 pgoset = 0;


/*************************************************************************/
/* Initialization                                                        */
/*************************************************************************/

/*-------------------------------------------------------*/
/* l1_trace_init_var()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Description: L1 Trace variables initialization.       */
/*-------------------------------------------------------*/
void l1_init_trace_var(void)
{
  UWORD8 i;

  // Global variables used for trace
  trace_info.PM_equal_0                = 0;
  trace_info.PM_Task                   = 255;
  trace_info.Not_PM_Task               = 255;
  trace_info.DSP_misaligned            = 0;
  trace_info.facch_ul_count            = 0;
  trace_info.facch_dl_count            = 0;
  trace_info.facch_dl_fail_count       = 0;
  trace_info.facch_dl_fail_count_trace = 0;
#if ( FF_REPEATED_DL_FACCH == 1)
  trace_info.facch_dl_combined_good_count = 0;
  trace_info.facch_dl_repetition_block_count = 0;
  trace_info.facch_dl_count_all = 0;
  trace_info.facch_dl_good_block_reported = 0;
#endif/*  (FF_REPEATED_DL_FACCH == 1)*/
  trace_info.sacch_d_nerr              = 0;
  #if (FF_REPEATED_SACCH == 1)
    trace_info.repeat_sacch.dl_count = 0;
    trace_info.repeat_sacch.dl_combined_good_count = 0;
	trace_info.repeat_sacch.dl_error_count = 0;
    trace_info.repeat_sacch.srr= 0;
    trace_info.repeat_sacch.sro= 0;
    trace_info.repeat_sacch.dl_good_norep = 0;
    trace_info.repeat_sacch.dl_buffer_empty = TRUE;
  #endif /* (FF_REPEATED_SACCH == 1) */
  trace_info.rxlev_req_count           = 0;
  trace_info.init_trace                = 0;
  trace_info.reset_gauging_algo = FALSE;  // trace Reset gauging Algorithm
  trace_info.sleep_performed           = FALSE;

  #if (L1_BINARY_TRACE == 0)
    trace_info.l1_memorize_error       = ' ';

    trace_info.trace_filter            = FALSE;
    trace_info.trace_fct_buff_index    = 0;
    trace_info.trace_buff_stop         = FALSE; // start buffer trace

    for (i=0;i<TRACE_FCT_BUFF_SIZE;i++)
      trace_info.trace_fct_buff[i]     =255;

  #endif

  #if L1_GPRS
    trace_info.new_tcr_list = 0;
  #endif

  #if (D_ERROR_STATUS_TRACE_ENABLE)
    trace_info.d_error_status_old                       = 0;
    trace_info.d_error_status_masks[GSM_SCHEDULER-1]    = DSP_DEBUG_GSM_MASK;
    #if (L1_GPRS)
      trace_info.d_error_status_masks[GPRS_SCHEDULER-1] = DSP_DEBUG_GPRS_MASK;
    #endif
  #endif
  #if (DSP_DEBUG_TRACE_ENABLE == 1)
    trace_info.dsp_debug_buf_start[0]               = 0;
    trace_info.dsp_debug_buf_start[1]               = 0;
    trace_info.fn_last_dsp_debug                    = 0xFFFFFFFF - 104;
  #endif

  trace_info.mem_task_bitmap[0] = 0;
  trace_info.mem_task_bitmap[1] = 0;
  trace_info.mem_task_bitmap[2] = 0;
  trace_info.mem_task_bitmap[3] = 0;
  trace_info.mem_task_bitmap[4] = 0;
  trace_info.mem_task_bitmap[5] = 0;
  trace_info.mem_task_bitmap[6] = 0;
  trace_info.mem_task_bitmap[7] = 0;

  trace_info.task_bitmap[0] = 0;
  trace_info.task_bitmap[1] = 0;
  trace_info.task_bitmap[2] = 0;
  trace_info.task_bitmap[3] = 0;
  trace_info.task_bitmap[4] = 0;
  trace_info.task_bitmap[5] = 0;
  trace_info.task_bitmap[6] = 0;
  trace_info.task_bitmap[7] = 0;

  #if (GSM_IDLE_RAM != 0)  // Init of the circular trace buffer
    intram_buffer_current_ptr=intram_trace_buffer;
    intram_trace_size = 0;
  #endif

#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
#if (MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD)
  trace_info.dsptrace_handler_globals.nested_disable_count = 0;
  trace_info.dsptrace_handler_globals.trace_flag_blocked = FALSE;
  trace_info.dsptrace_handler_globals.dsp_trace_level_copy = 0x0000;
#endif
#endif // ((TRACE_TYPE==1) || (TRACE_TYPE == 4))

  #if (BURST_PARAM_LOG_ENABLE == 1)
    burst_param_log_index = 0;
  #endif

#if (AUDIO_DEBUG == 1)
  trace_info.audio_debug_var.vocoder_enable_status = 0;
  trace_info.audio_debug_var.ul_state = 0;
  trace_info.audio_debug_var.dl_state = 0;
  trace_info.audio_debug_var.ul_onoff_counter = 0;
  trace_info.audio_debug_var.dl_onoff_counter = 0;
#endif

#if(L1_SAIC != 0)
trace_info.prev_saic_flag_val = 2;
trace_info.prev_swh_flag_val = 2;
#endif
#if (CODE_VERSION!=SIMULATION)
  trace_info.pwmgt_trace_var.fail_step  = 0xFF;
  trace_info.pwmgt_trace_var.fail_id    = 0xFF;
  trace_info.pwmgt_trace_var.fail_cause = 0xFF;
#endif // NO SIMULATION

}

/*-------------------------------------------------------*/
/* l1_trace_init()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Description: L1 Trace initialization.                 */
/*-------------------------------------------------------*/
void l1_trace_init()
{
  // Registration to RVT
  #if (TRACE_TYPE==1) || (TRACE_TYPE == 4)
    #if L1_BINARY_TRACE
      rvt_register_id("L1", &trace_info.l1_trace_user_id, l1_trace_configuration);
    #else
      rvt_register_id("L1", &trace_info.l1_trace_user_id, (RVT_CALLBACK_FUNC)NULL);
    #endif
  #endif

  // Initialize global variables
  l1_init_trace_var();


  /******************************/
  /* L1 RTT trace configuration */
  /******************************/

  trace_info.current_config = &(trace_info.config[0]);
  trace_info.pending_config = &(trace_info.config[1]);

  trace_info.current_config->l1_dyn_trace          = DEFAULT_DYN_TRACE_CONFIG;

  trace_info.current_config->rttl1_cell_enable[0]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[1]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[2]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[3]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[4]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[5]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[6]  = 0x00000000;
  trace_info.current_config->rttl1_cell_enable[7]  = 0x00000000;

  trace_info.current_config->rttl1_event_enable    = 0;

  /*******************************/
  /* L1 RTT trace initialization */
  /*******************************/

#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
#if L1_BINARY_TRACE
    if (rtt_create("L1RTT", &trace_info.l1s_trace_user_id, 10, (T_RTT_BUF) trace_info.l1s_trace_buf, L1S_RTT_BUF_LENGTH, NULL) == RTT_OK)
    {
      trace_info.l1s_rtt_func.rtt_refresh_status = &rtt_refresh_status;
      trace_info.l1s_rtt_func.rtt_get_fill_ptr   = &rtt_get_fill_ptr;
      trace_info.l1s_rtt_func.rtt_dump_buffer    = &rtt_dump_buffer;
    }
    else
#endif
    {
      trace_info.l1s_rtt_func.rtt_refresh_status = &rtt_refresh_status_dummy;
      trace_info.l1s_rtt_func.rtt_get_fill_ptr   = &rtt_get_fill_ptr_dummy;
      trace_info.l1s_rtt_func.rtt_dump_buffer    = &rtt_dump_buffer_dummy;
    }
    // Missing... ERROR HANDLING !!!
#endif

  /*************************/
  /* Send trace version    */
  /*************************/

#if L1_BINARY_TRACE
  l1_send_trace_version();
#endif
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4) || (TRACE_TYPE==5)

////////////////////////
// RTT dummy funtions //
////////////////////////

#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
// These function are used when RTT isn't active or no RTT L1 trace is enabled
T_RTT_RET rtt_refresh_status_dummy (T_RTT_USER_ID  user_id)
{
  return(RTT_OK);
}

T_RTT_PTR rtt_get_fill_ptr_dummy (T_RTT_USER_ID  user_id,
                                  T_RTT_SIZE     size)
{
  return(NULL);
}

T_RTT_RET rtt_dump_buffer_dummy (T_RTT_USER_ID  user_id,
                                 T_RTT_SIZE     dump_size)
{
  return(RTT_OK);
}
    #endif //(defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
  #endif //(TRACE_TYPE==1) || (TRACE_TYPE==4) || (TRACE_TYPE==5)
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/************************************ Binary trace ********************************************************/

#if L1_BINARY_TRACE

/*************************************************************************/
/* Classic trace output                                                  */
/*************************************************************************/

/*-------------------------------------------------------*/
/* l1_trace_message()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Description: L1 Trace formatting.                     */
/*-------------------------------------------------------*/
void l1_trace_message(xSignalHeaderRec *msg)
{

  /***********************************************************************/
  /* Trace configuration                                                 */
  /***********************************************************************/

  if (msg->SignalCode == TRACE_CONFIG)
  {
    char *ptr;
    T_TRACE_CONFIG *save;


    // Download values
    //----------------

    // Note: RTT values are used in L1S but partial download of these values have no
    //       negative influence on L1 or Trace behavior

    // First UWORD32 is the classic L1 dynamic trace
    trace_info.pending_config->l1_dyn_trace = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->trace_config;

    // The eight following UWORD32 define the RTT cell configuration
    trace_info.pending_config->rttl1_cell_enable[0] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[0];
    trace_info.pending_config->rttl1_cell_enable[1] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[1];
    trace_info.pending_config->rttl1_cell_enable[2] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[2];
    trace_info.pending_config->rttl1_cell_enable[3] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[3];
    trace_info.pending_config->rttl1_cell_enable[4] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[4];
    trace_info.pending_config->rttl1_cell_enable[5] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[5];
    trace_info.pending_config->rttl1_cell_enable[6] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[6];
    trace_info.pending_config->rttl1_cell_enable[7] = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[7];

    // Last UWORD32 define the RTT event
    trace_info.pending_config->rttl1_event_enable = ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_event;

    // Re-initialize global variables
    l1_init_trace_var();

    // Switch pointers
    save = trace_info.current_config;
    trace_info.current_config = trace_info.pending_config;
    trace_info.pending_config = save;

    // Trace the configuration change
    //-------------------------------

    if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_TRACE_CONFIG_CHANGE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
    {

      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->header             = TRL1_TRACE_CONFIG_CHANGE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->trace_config       = trace_info.current_config->l1_dyn_trace ;
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[0] = trace_info.current_config->rttl1_cell_enable[0];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[1] = trace_info.current_config->rttl1_cell_enable[1];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[2] = trace_info.current_config->rttl1_cell_enable[2];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[3] = trace_info.current_config->rttl1_cell_enable[3];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[4] = trace_info.current_config->rttl1_cell_enable[4];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[5] = trace_info.current_config->rttl1_cell_enable[5];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[6] = trace_info.current_config->rttl1_cell_enable[6];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_cell_enable[7] = trace_info.current_config->rttl1_cell_enable[7];
      ((T_TR_TRACE_CONFIG_CHANGE *)(ptr))->rtt_event          = trace_info.current_config->rttl1_event_enable;

      L1_send_trace_no_cpy(ptr,sizeof(T_TR_TRACE_CONFIG_CHANGE));

      return;
    }

  }

  /***********************************************************************/
  /* Traces coming from L1S                                              */
  /***********************************************************************/

  switch(msg->SignalCode)
  {
  #if (DSP_DEBUG_TRACE_ENABLE == 1)

    //////////////////////
    // DSP debug buffer //
    //////////////////////

    case TRACE_DSP_DEBUG:
    {
      WORD16  size  = ((T_DSP_DEBUG_INFO *)(msg->SigP))->size;
      char *ptr;
      UWORD8 index = 0;

      // DSP debug header trace: L1 indication to associate with the buffer
      if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_DSP_DEBUG_HEADER), (T_RVT_BUFFER *) &ptr) == RVT_OK)
      {
          ((T_TR_DSP_DEBUG_HEADER *)(ptr))->header         = TRL1_DSP_DEBUG_HEADER | (((T_DSP_DEBUG_INFO *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_DSP_DEBUG_HEADER *)(ptr))->debug_time     = ((T_DSP_DEBUG_INFO *)(msg->SigP))->debug_time;
          ((T_TR_DSP_DEBUG_HEADER *)(ptr))->patch_version  = ((T_DSP_DEBUG_INFO *)(msg->SigP))->patch_version;
          ((T_TR_DSP_DEBUG_HEADER *)(ptr))->trace_level    = ((T_DSP_DEBUG_INFO *)(msg->SigP))->trace_level;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_DSP_DEBUG_HEADER));
      }

      // DSP debug buffer trace
      while (size > 0)
      {
        UWORD8 item_size = size;

        // Split buffer in several buffers with size inferior to 240 (RVT limitation to 255)
        // Header not included (+8b)
        if (item_size > 240) item_size = 240;

        // Buffer size -> add header
        if (rvt_mem_alloc(trace_info.l1_trace_user_id, item_size + sizeof(T_TR_DSP_DEBUG_BUFFER) - sizeof(UWORD16), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          UWORD8 i;

          ((T_TR_DSP_DEBUG_BUFFER *)(ptr))->header       = TRL1_DSP_DEBUG_BUFFER | (((T_DSP_DEBUG_INFO *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_DSP_DEBUG_BUFFER *)(ptr))->size         = (item_size >> 1);
          // First element tells if it's the last msg
          ((T_TR_DSP_DEBUG_BUFFER *)(ptr))->content[0]   = 0;

          for (i = 1; i < (item_size >> 1); i++)
          {
            ((T_TR_DSP_DEBUG_BUFFER *)(ptr))->content[i]   = ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[index++];
          }

          size -= (item_size - 2); // (item_size - 2) bytes of the buffer transmitted
                                   // -2 because of the first word used for status

          // Indicates it's the last buffer
          if (size <= 0)
            ((T_TR_DSP_DEBUG_BUFFER *)(ptr))->content[0]     = 1;

          L1_send_trace_no_cpy(ptr,item_size + sizeof(T_TR_DSP_DEBUG_BUFFER) - sizeof(UWORD16));
        }

        // No trace buffer available -> cancel trace !
        else
        {
          break;
        }
      }
    }
    break;

  #endif // DSP_DEBUG_TRACE_ENABLE

  #if (L1_GPRS)

    ///////////////////////////
    // PDTCH condensed trace //
    ///////////////////////////

    case TRACE_CONDENSED_PDTCH:
    {
      char *ptr;

      if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_CONDENSED_PDTCH), (T_RVT_BUFFER *) &ptr) == RVT_OK)
      {
        UWORD8 i,j;

        ((T_TR_CONDENSED_PDTCH *)(ptr))->header             = TRL1_CONDENSED_PDTCH | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
        ((T_TR_CONDENSED_PDTCH *)(ptr))->rx_allocation      = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->rx_allocation;
        ((T_TR_CONDENSED_PDTCH *)(ptr))->tx_allocation      = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->tx_allocation;
        ((T_TR_CONDENSED_PDTCH *)(ptr))->blk_status         = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->blk_status;
        ((T_TR_CONDENSED_PDTCH *)(ptr))->dl_cs_type         = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_cs_type;
        ((T_TR_CONDENSED_PDTCH *)(ptr))->dl_status[0]       = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[0];
        ((T_TR_CONDENSED_PDTCH *)(ptr))->dl_status[1]       = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[1];
        ((T_TR_CONDENSED_PDTCH *)(ptr))->dl_status[2]       = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[2];
        ((T_TR_CONDENSED_PDTCH *)(ptr))->dl_status[3]       = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[3];

        for (i=0, j=0; (i<8)&&(j<4); i++)
          if (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[i] != 0)
          {
            ((T_TR_CONDENSED_PDTCH *)(ptr))->ul_status[j++] = ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[i];
          }

        L1_send_trace_no_cpy(ptr,sizeof(T_TR_CONDENSED_PDTCH));
      }
    }
    break;

  #endif // L1_GPRS

    /////////////////
    // Quick trace //
    /////////////////

    case QUICK_TRACE:
    {
      UWORD8   debug_code = ((T_QUICK_TRACE *)(msg->SigP))->debug_code;
      char     ptr[200];
      char    *str   = ((T_TR_ASCII *)(ptr))->str;
      UWORD32 *param = ((T_QUICK_TRACE *)(msg->SigP))->tab;

      // Fill string in 'str' variable
      // Parameters included in param[n]
      switch (debug_code)
      {
        case 0:
        {
          sprintf(str, "Quick trace example %d", param[0]);
        }
        break;

      }

      // Send trace
      ((T_TR_ASCII *)(ptr))->header = TRL1_ASCII | (((T_QUICK_TRACE *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
      ((T_TR_ASCII *)(ptr))->size = strlen(((T_TR_ASCII *)(ptr))->str);
      L1_send_trace_cpy(ptr,((T_TR_ASCII *)(ptr))->size + 8);
    }
    break;

    ////////////////
    // Debug info //
    ////////////////

    case TRACE_INFO:
    {
      // Read cell ID in the header (first UWORD32)
      UWORD16 trace_id = *((UWORD32 *)(msg->SigP)) & TR_HEADER_ID_MASK;

      switch(trace_id)
      {
        case TRL1_PM_EQUAL_0:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_PM_EQUAL_0));
        }
        break;

        case TRL1_MCU_DSP_MISMATCH:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_MCU_DSP_MISMATCH));
        }
        break;

        case TRL1_L1S_ABORT:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_L1S_ABORT));
        }
        break;

       #if (D_ERROR_STATUS_TRACE_ENABLE)
        case TRL1_D_ERROR_STATUS:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_D_ERROR_STATUS));
        }
        break;
      #endif

      #if L1_GPRS

        case TRL1_RLC_UL_PARAM:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_RLC_UL_PARAM));
        }
        break;

        case TRL1_RLC_DL_PARAM:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_RLC_DL_PARAM));
        }
        break;

        case TRL1_FORBIDDEN_UPLINK:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_FORBIDDEN_UPLINK));
        }
        break;

        case TRL1_DL_PTCCH:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_DL_PTCCH));
        }
        break;

        case TRL1_IT_DSP_ERROR:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_IT_DSP_ERROR));
        }
        break;

      #endif // L1_GPRS

        case TRL1_ADC:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_ADC));
        }
        break;

        case TRL1_NEW_TOA:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_NEW_TOA));
        }
        break;

        case TRL1_SAIC_DEBUG:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_SAIC_DEBUG));
        }
        break;

        case TRL1_BURST_PARAM:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_BURST_PARAM));
        }
        break;

        case TRL1_SLEEP:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_SLEEP));
        }
        break;

        case TRL1_GAUGING:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_GAUGING));
        }
        break;

        case TRL1_PTCCH_DISABLE:
        {
          L1_send_trace_cpy(msg->SigP,sizeof(T_TR_PTCCH_DISABLE));
        }
        break;

        default:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_UNKNOWN_L1S_TRACE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_UNKNOWN_L1S_TRACE *)(ptr))->header   = TRL1_UNKNOWN_L1S_TRACE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_UNKNOWN_L1S_TRACE *)(ptr))->id       = trace_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_UNKNOWN_L1S_TRACE));
          }
        }
      }  // End switch
    }  // End case "TRACE_INFO"
  }

  /***********************************************************************/
  /* L1S CPU load                                                        */
  /***********************************************************************/

  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_CPU_LOAD)
  {
    if(max_cpu_flag)
    {
      char *ptr;

      max_cpu_flag = 0;

      if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1S_CPU_PEAK), (T_RVT_BUFFER *) &ptr) == RVT_OK)
      {

        ((T_TR_L1S_CPU_PEAK *)(ptr))->header       = TRL1_L1S_CPU_PEAK | (fn_max_cpu << TR_HEADER_FN_DELAY);
        ((T_TR_L1S_CPU_PEAK *)(ptr))->max_cpu      = (UWORD8)max_cpu;

        L1_send_trace_no_cpy(ptr, sizeof(T_TR_L1S_CPU_PEAK));
      }
    }
  }

  /***********************************************************************/
  /* L1A messages                                                        */
  /***********************************************************************/

  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
  {

    switch(msg->SignalCode)
    {

      /********************************************************************************/
      /* CIRCUIT SWITCHED                                                             */
      /********************************************************************************/

      /////////////////////////////////////////
      // Message to set the right radio band //
      /////////////////////////////////////////
      case MPHC_INIT_L1_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_INIT_L1_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_INIT_L1_REQ *)(ptr))->header              = TRL1_MPHC_INIT_L1_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_INIT_L1_REQ *)(ptr))->radio_band_config   = ((T_MPHC_INIT_L1_REQ *)(msg->SigP))->radio_band_config;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NEW_SCELL_REQ));
        }
      }
      break;

      case MPHC_INIT_L1_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_INIT_L1_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          ((T_TR_MPHC_INIT_L1_CON *)(ptr))->header              = TRL1_MPHC_INIT_L1_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NEW_SCELL_REQ));
        }
      }
      break;

      ////////////////////////////
      // Serving Cell selection //
      ////////////////////////////

      case MPHC_NEW_SCELL_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NEW_SCELL_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NEW_SCELL_REQ *)(ptr))->header       = TRL1_MPHC_NEW_SCELL_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NEW_SCELL_REQ *)(ptr))->radio_freq   = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NEW_SCELL_REQ *)(ptr))->fn_offset    = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NEW_SCELL_REQ *)(ptr))->time_alignmt = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NEW_SCELL_REQ *)(ptr))->bsic         = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->bsic;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NEW_SCELL_REQ));
        }
      }
      break;

      //////////////////////////////
      // Neighbor cell monitoring //
      //////////////////////////////

      case MPHC_NETWORK_LOST_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NETWORK_LOST_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NETWORK_LOST_IND *)(ptr))->header       = TRL1_MPHC_NETWORK_LOST_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NETWORK_LOST_IND));
        }
      }
      break;

      // Idle mode neighbor cell synchronization

      case MPHC_NETWORK_SYNC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NETWORK_SYNC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->header          = TRL1_MPHC_NETWORK_SYNC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->radio_freq      = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->fn_offset       = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->time_alignmt    = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->timing_validity = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->timing_validity;
          ((T_TR_MPHC_NETWORK_SYNC_REQ *)(ptr))->search_mode     = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->search_mode;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NETWORK_SYNC_REQ));
        }
      }
      break;

      case MPHC_STOP_NETWORK_SYNC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_NETWORK_SYNC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_NETWORK_SYNC_REQ *)(ptr))->header       = TRL1_MPHC_STOP_NETWORK_SYNC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_NETWORK_SYNC_REQ));
        }
      }
      break;

      case MPHC_NCELL_SYNC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_SYNC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NCELL_SYNC_REQ *)(ptr))->header          = TRL1_MPHC_NCELL_SYNC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_SYNC_REQ *)(ptr))->radio_freq      = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NCELL_SYNC_REQ *)(ptr))->fn_offset       = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NCELL_SYNC_REQ *)(ptr))->time_alignmt    = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NCELL_SYNC_REQ *)(ptr))->timing_validity = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->timing_validity;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_SYNC_REQ));
        }
      }
      break;

      case MPHC_NCELL_LIST_SYNC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_LIST_SYNC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          UWORD8 i;

          ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->header      = TRL1_MPHC_NCELL_LIST_SYNC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->eotd        = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->eotd;
          ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->list_size   = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size;


          for (i=0; i<((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size; i++)
          {
            ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->radio_freq[i]      = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].timing_validity;
            ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->fn_offset[i]       = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].time_alignmt;
            ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->time_alignmt[i]    = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].fn_offset;
            ((T_TR_MPHC_NCELL_LIST_SYNC_REQ *)(ptr))->timing_validity[i] = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].radio_freq;
          }

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_LIST_SYNC_REQ));
        }
      }
      break;

      case MPHC_STOP_NCELL_SYNC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_NCELL_SYNC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->header                = TRL1_MPHC_STOP_NCELL_SYNC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array_size = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size;
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[0]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[0];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[1]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[1];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[2]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[2];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[3]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[3];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[4]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[4];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[5]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[5];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[6]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[0];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[7]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[1];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[8]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[2];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[9]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[3];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[10]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[4];
          ((T_TR_MPHC_STOP_NCELL_SYNC_REQ *)(ptr))->radio_freq_array[11]   = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[5];


          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_NCELL_SYNC_REQ));
        }
      }
      break;

      // Dedicated mode neigbor cell synchronization

      case MPHC_NCELL_FB_SB_READ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_FB_SB_READ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NCELL_FB_SB_READ *)(ptr))->header       = TRL1_MPHC_NCELL_FB_SB_READ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_FB_SB_READ *)(ptr))->radio_freq   = ((T_MPHC_NCELL_FB_SB_READ *)(msg->SigP))->radio_freq;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_FB_SB_READ));
        }
      }
      break;

      case MPHC_NCELL_SB_READ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_SB_READ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NCELL_SB_READ *)(ptr))->header       = TRL1_MPHC_NCELL_SB_READ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_SB_READ *)(ptr))->radio_freq   = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NCELL_SB_READ *)(ptr))->fn_offset    = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NCELL_SB_READ *)(ptr))->time_alignmt = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->time_alignmt;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_SB_READ));
        }
      }
      break;

      case L1C_FB_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_FB_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_FB_INFO *)(ptr))->header       = TRL1_L1C_FB_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_FB_INFO *)(ptr))->fb_flag      = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;
          ((T_TR_L1C_FB_INFO *)(ptr))->radio_freq   = ((T_L1C_FB_INFO*)(msg->SigP))->radio_freq;
          ((T_TR_L1C_FB_INFO *)(ptr))->pm           = ((T_L1C_FB_INFO*)(msg->SigP))->pm;
          ((T_TR_L1C_FB_INFO *)(ptr))->toa          = ((T_L1C_FB_INFO*)(msg->SigP))->toa;
          ((T_TR_L1C_FB_INFO *)(ptr))->angle        = ((T_L1C_FB_INFO*)(msg->SigP))->angle;
          ((T_TR_L1C_FB_INFO *)(ptr))->snr          = ((T_L1C_FB_INFO*)(msg->SigP))->snr;
          ((T_TR_L1C_FB_INFO *)(ptr))->input_level  = l1a_l1s_com.last_input_level[((T_L1C_FB_INFO*)(msg->SigP))->radio_freq -
                                                        l1_config.std.radio_freq_index_offset].input_level;
          ((T_TR_L1C_FB_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_FB_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_FB_INFO));
        }
      }
      break;

      case L1C_SB_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_SB_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_SB_INFO *)(ptr))->header       = TRL1_L1C_SB_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_SB_INFO *)(ptr))->radio_freq   = ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_SB_INFO *)(ptr))->sb_flag      = ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag;
          ((T_TR_L1C_SB_INFO *)(ptr))->fn_offset    = ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset;
          ((T_TR_L1C_SB_INFO *)(ptr))->time_alignmt = ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt;
          ((T_TR_L1C_SB_INFO *)(ptr))->bsic         = ((T_L1C_SB_INFO *)(msg->SigP))->bsic;
          ((T_TR_L1C_SB_INFO *)(ptr))->pm           = ((T_L1C_SB_INFO *)(msg->SigP))->pm;
          ((T_TR_L1C_SB_INFO *)(ptr))->toa          = ((T_L1C_SB_INFO *)(msg->SigP))->toa;
          ((T_TR_L1C_SB_INFO *)(ptr))->angle        = ((T_L1C_SB_INFO *)(msg->SigP))->angle;
          ((T_TR_L1C_SB_INFO *)(ptr))->snr          = ((T_L1C_SB_INFO *)(msg->SigP))->snr;
          ((T_TR_L1C_SB_INFO *)(ptr))->input_level  = l1a_l1s_com.last_input_level[((T_L1C_SB_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level;
          ((T_TR_L1C_SB_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_SB_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_SB_INFO));
        }
      }
      break;

      case L1C_SBCONF_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_SBCONF_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_SBCONF_INFO *)(ptr))->header       = TRL1_L1C_SBCONF_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->radio_freq   = ((T_L1C_SBCONF_INFO *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->sb_flag      = ((T_L1C_SBCONF_INFO *)(msg->SigP))->sb_flag;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->fn_offset    = ((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_offset;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->time_alignmt = ((T_L1C_SBCONF_INFO *)(msg->SigP))->time_alignmt;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->bsic         = ((T_L1C_SBCONF_INFO *)(msg->SigP))->bsic;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->pm           = ((T_L1C_SBCONF_INFO *)(msg->SigP))->pm;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->toa          = ((T_L1C_SBCONF_INFO *)(msg->SigP))->toa;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->angle        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->angle;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->snr          = ((T_L1C_SBCONF_INFO *)(msg->SigP))->snr;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->input_level  = l1a_l1s_com.last_input_level[((T_L1C_SBCONF_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_SBCONF_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_SBCONF_INFO));
        }
      }
      break;

      case MPHC_NETWORK_SYNC_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NETWORK_SYNC_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->header       = TRL1_MPHC_NETWORK_SYNC_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->sb_flag      = ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->sb_flag;
          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->radio_freq   = ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->fn_offset    = ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->time_alignmt = ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NETWORK_SYNC_IND *)(ptr))->bsic         = ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->bsic;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NETWORK_SYNC_IND));
        }
      }
      break;

      case MPHC_NCELL_SYNC_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_SYNC_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->header       = TRL1_MPHC_NCELL_SYNC_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->radio_freq   = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->sb_flag      = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->fn_offset    = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->time_alignmt = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->bsic         = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->neigh_id     = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->list_size    = l1a_l1s_com.nsync.current_list_size;
         #if (L1_EOTD)
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->eotd_data_valid = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->eotd_data_valid;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->mode            = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->mode;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->fn_sb_neigh     = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_sb_neigh;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->fn_in_SB        = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_in_SB;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->toa_correction  = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->toa_correction;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->delta_fn        = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_fn;
          ((T_TR_MPHC_NCELL_SYNC_IND *)(ptr))->delta_qbit      = ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_qbit;
         #endif
          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_SYNC_IND));
        }
      }
      break;

      // Neighbor cell BCCH reading

      case MPHC_NCELL_BCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_NCELL_BCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->header        = TRL1_MPHC_NCELL_BCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->radio_freq    = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq;
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->fn_offset     = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->time_alignmt  = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt;
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->tsc           = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc;
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->bcch_blks_req = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req;
          #if L1_GPRS
          ((T_TR_MPHC_NCELL_BCCH_REQ *)(ptr))->gprs_priority = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority;
          #endif

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_NCELL_BCCH_REQ));
        }
      }
      break;

      case MPHC_STOP_NCELL_BCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_NCELL_BCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->header                = TRL1_MPHC_STOP_NCELL_BCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array_size = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array_size;
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[0]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[0];
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[1]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[1];
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[2]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[2];
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[3]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[3];
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[4]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[4];
          ((T_TR_MPHC_STOP_NCELL_BCCH_REQ *)(ptr))->radio_freq_array[5]   = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[5];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_NCELL_BCCH_REQ));
        }
      }
      break;

      case L1C_BCCHN_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_BCCHN_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_BCCHN_INFO *)(ptr))->header       = TRL1_L1C_BCCHN_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_BCCHN_INFO *)(ptr))->error_flag   = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_BCCHN_INFO *)(ptr))->radio_freq   = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_BCCHN_INFO *)(ptr))->input_level  = l1a_l1s_com.last_input_level[((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq -
                                                                                     l1_config.std.radio_freq_index_offset].input_level;
          ((T_TR_L1C_BCCHN_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_BCCHN_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_BCCHN_INFO));
        }
      }
      break;

      ///////////////////////////////////////
      // Serving cell normal burst reading //
      ///////////////////////////////////////

      // CCCH reading

      case MPHC_START_CCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_START_CCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->header           = TRL1_MPHC_START_CCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->bs_pa_mfrms      = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->bs_ag_blks_res   = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->bcch_combined    = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->ccch_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->page_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->page_block_index = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index;
          ((T_TR_MPHC_START_CCCH_REQ *)(ptr))->page_mode        = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_mode;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_START_CCCH_REQ));
        }
      }
      break;

      case MPHC_STOP_CCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_CCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_CCCH_REQ *)(ptr))->header       = TRL1_MPHC_STOP_CCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_CCCH_REQ));
        }
      }
      break;

      case L1C_NP_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_NP_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_NP_INFO *)(ptr))->header       = TRL1_L1C_NP_INFO | (((T_MPHC_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_NP_INFO *)(ptr))->error_flag   = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_NP_INFO *)(ptr))->radio_freq   = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_NP_INFO *)(ptr))->input_level  = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1C_NP_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_NP_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_NP_INFO));
        }
      }
      break;

      case L1C_EP_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_EP_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_EP_INFO *)(ptr))->header       = TRL1_L1C_EP_INFO | (((T_MPHC_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_EP_INFO *)(ptr))->error_flag   = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_EP_INFO *)(ptr))->radio_freq   = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_EP_INFO *)(ptr))->input_level  = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1C_EP_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_EP_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_EP_INFO));
        }
      }
      break;

      case L1C_ALLC_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_ALLC_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_ALLC_INFO *)(ptr))->header       = TRL1_L1C_ALLC_INFO | (((T_MPHC_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_ALLC_INFO *)(ptr))->error_flag   = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_ALLC_INFO *)(ptr))->radio_freq   = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_ALLC_INFO *)(ptr))->input_level  = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1C_ALLC_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_ALLC_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_ALLC_INFO));
        }
      }
      break;

      // BCCH reading

      case MPHC_SCELL_NBCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_SCELL_NBCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->header                = TRL1_MPHC_SCELL_NBCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->schedule_array_size   = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array_size;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[0]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[0].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[1]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[1].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[2]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[2].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[3]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[3].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[4]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[4].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[5]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[5].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[6]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[6].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[7]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[7].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[8]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[8].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->modulus[9]            = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[9].modulus;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[0]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[0].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[1]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[1].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[2]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[2].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[3]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[3].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[4]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[4].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[5]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[5].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[6]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[6].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[7]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[7].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[8]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[8].relative_position;
          ((T_TR_MPHC_SCELL_NBCCH_REQ *)(ptr))->relative_position[9]  = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[9].relative_position;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_SCELL_NBCCH_REQ));
        }
      }
      break;

      case MPHC_SCELL_EBCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_SCELL_EBCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->header                = TRL1_MPHC_SCELL_EBCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->schedule_array_size   = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array_size;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[0]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[0].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[1]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[1].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[2]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[2].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[3]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[3].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[4]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[4].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[5]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[5].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[6]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[6].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[7]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[7].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[8]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[8].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->modulus[9]            = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[9].modulus;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[0]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[0].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[1]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[1].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[2]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[2].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[3]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[3].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[4]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[4].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[5]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[5].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[6]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[6].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[7]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[7].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[8]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[8].relative_position;
          ((T_TR_MPHC_SCELL_EBCCH_REQ *)(ptr))->relative_position[9]  = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[9].relative_position;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_SCELL_EBCCH_REQ));
        }
      }
      break;

      case MPHC_STOP_SCELL_BCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_SCELL_BCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_SCELL_BCCH_REQ *)(ptr))->header       = TRL1_MPHC_STOP_SCELL_BCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_SCELL_BCCH_REQ));
        }
      }
      break;

      case L1C_BCCHS_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_BCCHS_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_BCCHS_INFO *)(ptr))->header       = TRL1_L1C_BCCHS_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_BCCHS_INFO *)(ptr))->error_flag   = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_BCCHS_INFO *)(ptr))->radio_freq   = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_BCCHS_INFO *)(ptr))->input_level  = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1C_BCCHS_INFO *)(ptr))->tpu_offset   = l1s.tpu_offset;
          ((T_TR_L1C_BCCHS_INFO *)(ptr))->afc          = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_BCCHS_INFO));
        }
      }
      break;

      //////////
      // CBCH //
      //////////

      case MPHC_CONFIG_CBCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CONFIG_CBCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CONFIG_CBCH_REQ *)(ptr))->header       = TRL1_MPHC_CONFIG_CBCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_CONFIG_CBCH_REQ *)(ptr))->h            = ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.h;
          ((T_TR_MPHC_CONFIG_CBCH_REQ *)(ptr))->radio_freq   = ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHC_CONFIG_CBCH_REQ *)(ptr))->timeslot_no  = ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.timeslot_no;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CONFIG_CBCH_REQ));
        }
      }
      break;

      case MPHC_CBCH_SCHEDULE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CBCH_SCHEDULE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CBCH_SCHEDULE_REQ *)(ptr))->header          = TRL1_MPHC_CBCH_SCHEDULE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_CBCH_SCHEDULE_REQ *)(ptr))->extended_cbch   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->extended_cbch;
          ((T_TR_MPHC_CBCH_SCHEDULE_REQ *)(ptr))->schedule_length = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->schedule_length;
          ((T_TR_MPHC_CBCH_SCHEDULE_REQ *)(ptr))->first_block_0   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_0;
          ((T_TR_MPHC_CBCH_SCHEDULE_REQ *)(ptr))->first_block_1   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_1;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CBCH_SCHEDULE_REQ));
        }
      }
      break;

      case MPHC_CBCH_INFO_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CBCH_INFO_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CBCH_INFO_REQ *)(ptr))->header       = TRL1_MPHC_CBCH_INFO_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_CBCH_INFO_REQ *)(ptr))->tb_bitmap    = ((T_MPHC_CBCH_INFO_REQ *)(msg->SigP))->tb_bitmap;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CBCH_INFO_REQ));
        }
      }
      break;

      case MPHC_CBCH_UPDATE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CBCH_UPDATE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CBCH_UPDATE_REQ *)(ptr))->header        = TRL1_MPHC_CBCH_UPDATE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_CBCH_UPDATE_REQ *)(ptr))->extended_cbch = ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->extended_cbch;
          ((T_TR_MPHC_CBCH_UPDATE_REQ *)(ptr))->first_block_0 = ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_0;
          ((T_TR_MPHC_CBCH_UPDATE_REQ *)(ptr))->first_block_1 = ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_1;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CBCH_UPDATE_REQ));
        }
      }
      break;

      case L1C_CB_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_CB_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_CB_INFO *)(ptr))->header          = TRL1_L1C_CB_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_CB_INFO *)(ptr))->error_flag      = ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1C_CB_INFO *)(ptr))->radio_freq      = ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1C_CB_INFO *)(ptr))->input_level     = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1C_CB_INFO *)(ptr))->tpu_offset      = l1s.tpu_offset;
          ((T_TR_L1C_CB_INFO *)(ptr))->afc             = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_CB_INFO));
        }
      }
      break;

      // Stop CBCH

      case MPHC_STOP_CBCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_CBCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_CBCH_REQ *)(ptr))->header        = TRL1_MPHC_STOP_CBCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_STOP_CBCH_REQ *)(ptr))->normal_cbch   = ((T_MPHC_STOP_CBCH_REQ *)(msg->SigP))->normal_cbch;
          ((T_TR_MPHC_STOP_CBCH_REQ *)(ptr))->extended_cbch = ((T_MPHC_STOP_CBCH_REQ *)(msg->SigP))->extended_cbch;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_CBCH_REQ));
        }
      }
      break;

      case MPHC_STOP_CBCH_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_CBCH_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_CBCH_CON *)(ptr))->header       = TRL1_MPHC_STOP_CBCH_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_CBCH_CON));
        }
      }
      break;

      ///////////////////
      // Random Access //
      ///////////////////

      case MPHC_RA_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_RA_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_RA_REQ *)(ptr))->header            = TRL1_MPHC_RA_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_RA_REQ *)(ptr))->txpwr             = ((T_MPHC_RA_REQ *)(msg->SigP))->txpwr;
          ((T_TR_MPHC_RA_REQ *)(ptr))->rand              = ((T_MPHC_RA_REQ *)(msg->SigP))->rand;
          ((T_TR_MPHC_RA_REQ *)(ptr))->channel_request   = ((T_MPHC_RA_REQ *)(msg->SigP))->channel_request;
          ((T_TR_MPHC_RA_REQ *)(ptr))->powerclass_band1  = ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band1;
          ((T_TR_MPHC_RA_REQ *)(ptr))->powerclass_band2  = ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band2;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_RA_REQ));
        }
      }
      break;

      case MPHC_STOP_RA_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_RA_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_RA_REQ *)(ptr))->header       = TRL1_MPHC_STOP_RA_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_RA_REQ));
        }
      }
      break;

      case L1C_RA_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_RA_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_RA_DONE *)(ptr))->header       = TRL1_L1C_RA_DONE | (((T_MPHC_RA_CON *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_RA_DONE));
        }
      }
      break;

      /////////////////////////////
      // Dedicated mode channels //
      /////////////////////////////

      // Immediate assignment

      case MPHC_IMMED_ASSIGN_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_IMMED_ASSIGN_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->header                = TRL1_MPHC_IMMED_ASSIGN_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->h                     = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.h;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->radio_freq            = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->channel_type          = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.channel_type;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->subchannel            = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.subchannel;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->timeslot_no           = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.timeslot_no;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->tsc                   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.tsc;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->timing_advance        = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->timing_advance;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->rf_chan_cnt           = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->starting_time_present = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time_present;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->n32                   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n32;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->n51                   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n51;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->n26                   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n26;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->bef_sti_rf_chan_cnt   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->dtx_allowed           = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->dtx_allowed;
          ((T_TR_MPHC_IMMED_ASSIGN_REQ *)(ptr))->pwrc                  = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->pwrc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_IMMED_ASSIGN_REQ));
        }
      }
      break;

      case MPHC_IMMED_ASSIGN_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_IMMED_ASSIGN_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_IMMED_ASSIGN_CON *)(ptr))->header       = TRL1_MPHC_IMMED_ASSIGN_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_IMMED_ASSIGN_CON));
        }
      }
      break;

      // Channel assignment

      case MPHC_CHANNEL_ASSIGN_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CHANNEL_ASSIGN_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->header                = TRL1_MPHC_CHANNEL_ASSIGN_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->h                     = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.h;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->radio_freq            = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->channel_type          = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.channel_type;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->subchannel            = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.subchannel;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->timeslot_no           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.timeslot_no;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->tsc                   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.tsc;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->channel_mode_1        = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->txpwr                 = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->rf_chan_cnt           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->starting_time_present = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time_present;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->n32                   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n32;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->n51                   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n51;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->n26                   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time.n26;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->bef_sti_rf_chan_cnt   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->cipher_mode           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->a5_algorithm          = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->dtx_allowed           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed;

          #if (AMR == 1)
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->noise_suppression_bit        = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->initial_codec_mode_indicator = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->initial_codec_mode           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->active_codec_set             = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.active_codec_set;
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->threshold[0]                 = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[0];
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->threshold[1]                 = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[1];
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->threshold[2]                 = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[2];
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->hysteresis[0]                = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[0];
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->hysteresis[1]                = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[1];
            ((T_TR_MPHC_CHANNEL_ASSIGN_REQ *)(ptr))->hysteresis[2]                = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[2];
          #endif

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CHANNEL_ASSIGN_REQ));
        }
      }
      break;

      case MPHC_CHANNEL_ASSIGN_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CHANNEL_ASSIGN_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CHANNEL_ASSIGN_CON *)(ptr))->header       = TRL1_MPHC_CHANNEL_ASSIGN_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CHANNEL_ASSIGN_CON));
        }
      }
      break;

      // SACCH reception

      case L1C_SACCH_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_SACCH_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_SACCH_INFO *)(ptr))->header             = TRL1_L1C_SACCH_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_SACCH_INFO *)(ptr))->error_cause        = ((T_PH_DATA_IND *)(msg->SigP))->error_cause;
          ((T_TR_L1C_SACCH_INFO *)(ptr))->rf_chan_num        = ((T_PH_DATA_IND *)(msg->SigP))->rf_chan_num;
          ((T_TR_L1C_SACCH_INFO *)(ptr))->beacon_input_level = l1a_l1s_com.Scell_info.traffic_meas_beacon.input_level;
          ((T_TR_L1C_SACCH_INFO *)(ptr))->input_level        = l1a_l1s_com.Scell_info.traffic_meas.input_level;
          ((T_TR_L1C_SACCH_INFO *)(ptr))->tpu_offset         = l1s.tpu_offset;
          ((T_TR_L1C_SACCH_INFO *)(ptr))->afc                = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_SACCH_INFO));
        }
      }
      break;

      // Channel modification

      case MPHC_CHANGE_FREQUENCY:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CHANGE_FREQUENCY), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->header             = TRL1_MPHC_CHANGE_FREQUENCY | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->h                  = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.h;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->radio_freq         = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->channel_type       = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.channel_type;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->subchannel         = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.subchannel;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->timeslot_no        = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.timeslot_no;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->tsc                = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.tsc;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->rf_chan_cnt        = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->frequency_list.rf_chan_cnt;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->start_time_present = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time.start_time_present;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->n32                = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time.start_time.n32;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->n51                = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time.start_time.n51;
          ((T_TR_MPHC_CHANGE_FREQUENCY *)(ptr))->n26                = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time.start_time.n26;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CHANGE_FREQUENCY));
        }
      }
      break;

      case L1C_REDEF_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_REDEF_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_REDEF_DONE *)(ptr))->header       = TRL1_L1C_REDEF_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_REDEF_DONE));
        }
      }
      break;

      case MPHC_CHANNEL_MODE_MODIFY_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->header       = TRL1_MPHC_CHANNEL_MODE_MODIFY_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->subchannel   = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel;
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->channel_mode = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode;

          #if (AMR == 1)
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->noise_suppression_bit        = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit;
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->initial_codec_mode_indicator = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator;
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->initial_codec_mode           = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode;
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->active_codec_set             = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.active_codec_set;
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->threshold[0]                 = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[0];
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->threshold[1]                 = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[1];
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->threshold[2]                 = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[2];
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->hysteresis[0]                = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[0];
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->hysteresis[1]                = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[1];
            ((T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ *)(ptr))->hysteresis[2]                = ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[2];
          #endif

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_CHANNEL_MODE_MODIFY_REQ));
        }
      }
      break;

      // Ciphering

      case MPHC_SET_CIPHERING_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_SET_CIPHERING_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->header       = TRL1_MPHC_SET_CIPHERING_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->cipher_mode  = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->cipher_mode;
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->a5_algorithm = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->a5_algorithm;
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[0]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[0];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[1]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[1];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[2]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[2];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[3]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[3];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[4]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[4];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[5]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[5];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[6]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[6];
          ((T_TR_MPHC_SET_CIPHERING_REQ *)(ptr))->A[7]         = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[7];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_SET_CIPHERING_REQ));
        }
      }
      break;

      // Generic stop

      case MPHC_STOP_DEDICATED_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_DEDICATED_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_DEDICATED_REQ *)(ptr))->header       = TRL1_MPHC_STOP_DEDICATED_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_DEDICATED_REQ));
        }
      }
      break;

      case MPHC_STOP_DEDICATED_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_DEDICATED_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_DEDICATED_CON *)(ptr))->header       = TRL1_MPHC_STOP_DEDICATED_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_DEDICATED_CON));
        }
      }
      break;

      case L1C_STOP_DEDICATED_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_STOP_DEDICATED_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_STOP_DEDICATED_DONE *)(ptr))->header       = TRL1_L1C_STOP_DEDICATED_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_STOP_DEDICATED_DONE));
        }
      }
      break;

      //////////////
      // HANDOVER //
      //////////////

      // Asynchronous handover request

      case MPHC_ASYNC_HO_REQ:
      {
        #define msg_aho ((T_MPHC_ASYNC_HO_REQ *)(msg->SigP))

        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_ASYNC_HO_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->header                = TRL1_MPHC_ASYNC_HO_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->ncc                   = msg_aho->handover_command.cell_description.ncc;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->bcc                   = msg_aho->handover_command.cell_description.bcc;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->bcch_carrier          = msg_aho->handover_command.cell_description.bcch_carrier;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->h                     = msg_aho->handover_command.channel_desc_1.chan_sel.h;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->radio_freq            = msg_aho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->channel_type          = msg_aho->handover_command.channel_desc_1.channel_type;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->subchannel            = msg_aho->handover_command.channel_desc_1.subchannel;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->timeslot_no           = msg_aho->handover_command.channel_desc_1.timeslot_no;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->tsc                   = msg_aho->handover_command.channel_desc_1.tsc;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->channel_mode_1        = msg_aho->handover_command.channel_mode_1;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->txpwr                 = msg_aho->handover_command.txpwr;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->rf_chan_cnt           = msg_aho->handover_command.frequency_list.rf_chan_cnt;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->starting_time_present = msg_aho->handover_command.starting_time.start_time_present;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->n32                   = msg_aho->handover_command.starting_time.start_time.n32;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->n51                   = msg_aho->handover_command.starting_time.start_time.n51;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->n26                   = msg_aho->handover_command.starting_time.start_time.n26;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->ho_acc                = msg_aho->handover_command.ho_acc;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->report_time_diff      = msg_aho->handover_command.report_time_diff;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->bef_sti_rf_chan_cnt   = msg_aho->handover_command.frequency_list_bef_sti.rf_chan_cnt;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->cipher_mode           = msg_aho->handover_command.cipher_mode;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->a5_algorithm          = msg_aho->handover_command.a5_algorithm;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->fn_offset             = msg_aho->fn_offset;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->time_alignmt          = msg_aho->time_alignmt;

          #if (AMR == 1)
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->noise_suppression_bit        = msg_aho->amr_configuration.noise_suppression_bit;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->initial_codec_mode_indicator = msg_aho->amr_configuration.initial_codec_mode_indicator;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->initial_codec_mode           = msg_aho->amr_configuration.initial_codec_mode;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->active_codec_set             = msg_aho->amr_configuration.active_codec_set;
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->threshold[0]                 = msg_aho->amr_configuration.threshold[0];
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->threshold[1]                 = msg_aho->amr_configuration.threshold[1];
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->threshold[2]                 = msg_aho->amr_configuration.threshold[2];
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->hysteresis[0]                = msg_aho->amr_configuration.hysteresis[0];
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->hysteresis[1]                = msg_aho->amr_configuration.hysteresis[1];
            ((T_TR_MPHC_ASYNC_HO_REQ *)(ptr))->hysteresis[2]                = msg_aho->amr_configuration.hysteresis[2];
          #endif

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_ASYNC_HO_REQ));
        }
      }
      break;

      case MPHC_ASYNC_HO_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_ASYNC_HO_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_ASYNC_HO_CON *)(ptr))->header       = TRL1_MPHC_ASYNC_HO_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_ASYNC_HO_CON));
        }
      }
      break;

      // Synchronous handover request

      case MPHC_SYNC_HO_REQ:
      case MPHC_PRE_SYNC_HO_REQ:
      case MPHC_PSEUDO_SYNC_HO_REQ:
      {
        #define msg_sho ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))

        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_SYNC_HO_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->header                = TRL1_MPHC_SYNC_HO_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->ncc                   = msg_sho->handover_command.cell_description.ncc;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->bcc                   = msg_sho->handover_command.cell_description.bcc;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->bcch_carrier          = msg_sho->handover_command.cell_description.bcch_carrier;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->h                     = msg_sho->handover_command.channel_desc_1.chan_sel.h;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->radio_freq            = msg_sho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->channel_type          = msg_sho->handover_command.channel_desc_1.channel_type;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->subchannel            = msg_sho->handover_command.channel_desc_1.subchannel;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->timeslot_no           = msg_sho->handover_command.channel_desc_1.timeslot_no;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->tsc                   = msg_sho->handover_command.channel_desc_1.tsc;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->channel_mode_1        = msg_sho->handover_command.channel_mode_1;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->txpwr                 = msg_sho->handover_command.txpwr;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->rf_chan_cnt           = msg_sho->handover_command.frequency_list.rf_chan_cnt;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->starting_time_present = msg_sho->handover_command.starting_time.start_time_present;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->n32                   = msg_sho->handover_command.starting_time.start_time.n32;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->n51                   = msg_sho->handover_command.starting_time.start_time.n51;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->n26                   = msg_sho->handover_command.starting_time.start_time.n26;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->ho_acc                = msg_sho->handover_command.ho_acc;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->report_time_diff      = msg_sho->handover_command.report_time_diff;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->bef_sti_rf_chan_cnt   = msg_sho->handover_command.frequency_list_bef_sti.rf_chan_cnt;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->cipher_mode           = msg_sho->handover_command.cipher_mode;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->a5_algorithm          = msg_sho->handover_command.a5_algorithm;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->fn_offset             = msg_sho->fn_offset;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->time_alignmt          = msg_sho->time_alignmt;

          #if (AMR == 1)
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->noise_suppression_bit        = msg_sho->amr_configuration.noise_suppression_bit;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->initial_codec_mode_indicator = msg_sho->amr_configuration.initial_codec_mode_indicator;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->initial_codec_mode           = msg_sho->amr_configuration.initial_codec_mode;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->active_codec_set             = msg_sho->amr_configuration.active_codec_set;
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->threshold[0]                 = msg_sho->amr_configuration.threshold[0];
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->threshold[1]                 = msg_sho->amr_configuration.threshold[1];
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->threshold[2]                 = msg_sho->amr_configuration.threshold[2];
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->hysteresis[0]                = msg_sho->amr_configuration.hysteresis[0];
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->hysteresis[1]                = msg_sho->amr_configuration.hysteresis[1];
            ((T_TR_MPHC_SYNC_HO_REQ *)(ptr))->hysteresis[2]                = msg_sho->amr_configuration.hysteresis[2];
          #endif

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_SYNC_HO_REQ));
        }
      }
      break;

      case MPHC_SYNC_HO_CON:
      case MPHC_PRE_SYNC_HO_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_SYNC_HO_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_SYNC_HO_CON *)(ptr))->header       = TRL1_MPHC_SYNC_HO_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_SYNC_HO_CON));
        }
      }
      break;

      case L1C_HANDOVER_FINISHED:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_HANDOVER_FINISHED), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_HANDOVER_FINISHED *)(ptr))->header       = TRL1_L1C_HANDOVER_FINISHED | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_HANDOVER_FINISHED *)(ptr))->cause        = ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->cause;
#if ((REL99 == 1) && (FF_BHO == 1))
          ((T_TR_L1C_HANDOVER_FINISHED *)(ptr))->fn_offset      = ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->fn_offset;
          ((T_TR_L1C_HANDOVER_FINISHED *)(ptr))->time_alignment = ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->time_alignment;
#endif
          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_HANDOVER_FINISHED));
        }
      }
      break;

      case MPHC_TA_FAIL_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_TA_FAIL_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_TA_FAIL_IND *)(ptr))->header       = TRL1_MPHC_TA_FAIL_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_TA_FAIL_IND));
        }
      }
      break;

      // Handover failure

      case MPHC_HANDOVER_FAIL_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_HANDOVER_FAIL_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_HANDOVER_FAIL_REQ *)(ptr))->header       = TRL1_MPHC_HANDOVER_FAIL_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_HANDOVER_FAIL_REQ));
        }
      }
      break;

      case MPHC_HANDOVER_FAIL_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_HANDOVER_FAIL_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_HANDOVER_FAIL_CON *)(ptr))->header       = TRL1_MPHC_HANDOVER_FAIL_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_HANDOVER_FAIL_CON));
        }
      }
      break;

      //////////////////
      // Measurements //
      //////////////////

      // Cell selection / PLMN selection / Extended measurements

      case MPHC_RXLEV_REQ:
      {
        char *ptr;

        trace_info.rxlev_req_count ++;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_RXLEV_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_RXLEV_REQ *)(ptr))->header           = TRL1_MPHC_RXLEV_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_RXLEV_REQ *)(ptr))->power_array_size = ((T_MPHC_RXLEV_REQ *)(msg->SigP))->power_array_size;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_RXLEV_REQ));
        }
      }
      break;

      case MPHC_STOP_RXLEV_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_RXLEV_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_RXLEV_REQ *)(ptr))->header       = TRL1_MPHC_STOP_RXLEV_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_RXLEV_REQ));
        }
      }
      break;

      case L1C_VALID_MEAS_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_VALID_MEAS_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          ((T_TR_L1C_VALID_MEAS_INFO *)(ptr))->header            = TRL1_L1C_VALID_MEAS_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_VALID_MEAS_INFO *)(ptr))->power_array_size  = l1a_l1s_com.full_list_ptr->power_array_size;
          ((T_TR_L1C_VALID_MEAS_INFO *)(ptr))->rxlev_req_count   = trace_info.rxlev_req_count;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_VALID_MEAS_INFO));
        }

        if (trace_info.rxlev_req_count == 5)
        {
          trace_info.rxlev_req_count = 0;

          if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_FULL_LIST_REPORT)
          {
            UWORD16 index = 0;
            UWORD16 size = l1a_l1s_com.full_list_ptr->power_array_size * 4;
            UWORD8  i;

            while (size > 0)
            {
              UWORD16 item_size = size;

              // Split buffer in several buffers with size inferior to 240 (RVT limitation to 255)
              // Header not inluded (+8b)
              if (item_size > 240) item_size = 240;

              if (rvt_mem_alloc(trace_info.l1_trace_user_id, item_size + sizeof(T_TR_FULL_LIST_REPORT) - sizeof(UWORD32), (T_RVT_BUFFER *) &ptr) == RVT_OK)
              {
                ((T_TR_FULL_LIST_REPORT *)(ptr))->header            = TRL1_FULL_LIST_REPORT | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
                ((T_TR_FULL_LIST_REPORT *)(ptr))->size              = (item_size >> 2);

                for (i=0; i<(item_size >> 2); i++)
                {
                   ((T_TR_FULL_LIST_REPORT *)(ptr))->content[i]  = l1a_l1s_com.full_list_ptr->power_array[index].radio_freq;
                   ((T_TR_FULL_LIST_REPORT *)(ptr))->content[i] |= l1a_l1s_com.full_list_ptr->power_array[index].accum_power_result << 16;
                   index++;
                }

                size -= item_size;

                L1_send_trace_no_cpy(ptr,item_size + sizeof(T_TR_FULL_LIST_REPORT) - sizeof(UWORD32));
              }
              else
                break;
            }
          }
        } // 5th attempt
      }
      break;

      // Idle mode BA list

      case MPHC_RXLEV_PERIODIC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_RXLEV_PERIODIC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_RXLEV_PERIODIC_REQ *)(ptr))->header                   = TRL1_MPHC_RXLEV_PERIODIC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_RXLEV_PERIODIC_REQ *)(ptr))->num_of_chans             = ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->num_of_chans;
          ((T_TR_MPHC_RXLEV_PERIODIC_REQ *)(ptr))->ba_id                    = ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->ba_id;
          ((T_TR_MPHC_RXLEV_PERIODIC_REQ *)(ptr))->next_radio_freq_measured = ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->next_radio_freq_measured;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_RXLEV_PERIODIC_REQ));
        }
      }
      break;

      case MPHC_STOP_RXLEV_PERIODIC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_STOP_RXLEV_PERIODIC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_STOP_RXLEV_PERIODIC_REQ *)(ptr))->header       = TRL1_MPHC_STOP_RXLEV_PERIODIC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_STOP_RXLEV_PERIODIC_REQ));
        }
      }
      break;

      case L1C_RXLEV_PERIODIC_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_RXLEV_PERIODIC_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->header           = TRL1_L1C_RXLEV_PERIODIC_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->ba_id            = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->ba_id;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->s_rxlev          = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->s_rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[0] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[1] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[2] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[3] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[4] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[5] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[6] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->radio_freq_no[7] = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].radio_freq_no;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[0]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[1]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[2]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[3]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[4]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[5]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[6]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].rxlev;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->rxlev[7]         = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].rxlev;
((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->qual_acc_idle            = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_acc_idle;
          ((T_TR_L1C_RXLEV_PERIODIC_DONE *)(ptr))->qual_nbr_meas_idle           = ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_nbr_meas_idle;


          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_RXLEV_PERIODIC_DONE));
        }
      }
      break;


      // Dedicated mode BA list

      case MPHC_MEAS_REPORT:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1C_MEAS_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1C_MEAS_DONE *)(ptr))->header                   = TRL1_L1C_MEAS_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1C_MEAS_DONE *)(ptr))->meas_valid               = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->meas_valid;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->txpwr_used               = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->txpwr_used;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->timing_advance           = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->timing_advance;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_full_acc           = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_full_nbr_meas      = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_sub_acc            = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_sub_nbr_meas       = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxqual_full_acc_errors   = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_acc_errors;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxqual_full_nbr_bits     = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_nbr_bits;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxqual_sub_acc_errors    = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_acc_errors;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxqual_sub_nbr_bits      = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_nbr_bits;
          #if(REL99)
          #if(FF_EMR)
            ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_val_acc;         = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_acc;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_val_nbr_meas;    = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_nbr_meas;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->mean_bep_block_acc;    = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_acc;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->cv_bep_block_acc;      = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_acc;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->mean_bep_block_num;    = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_num;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->cv_bep_block_num;      = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_num;
            ((T_TR_L1C_MEAS_DONE *)(ptr))->nbr_rcvd_blocks;       = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->nbr_rcvd_blocks;
          #endif
          #endif
          ((T_TR_L1C_MEAS_DONE *)(ptr))->facch_ul_count           = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_ul_count;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->facch_dl_count           = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_count;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[0]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[1]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[2]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[3]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[4]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->bcch_freq[5]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].bcch_freq;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[0]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[1]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[2]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[3]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[4]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_acc[5]             = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_acc;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[0]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[1]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[2]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[3]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[4]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_nbr_meas;
          ((T_TR_L1C_MEAS_DONE *)(ptr))->rxlev_nbr_meas[5]        = ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_nbr_meas;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1C_MEAS_DONE));
        }
      }
      break;

      // Update BA list

      case MPHC_UPDATE_BA_LIST:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHC_UPDATE_BA_LIST), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHC_UPDATE_BA_LIST *)(ptr))->header       = TRL1_MPHC_UPDATE_BA_LIST | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHC_UPDATE_BA_LIST *)(ptr))->num_of_chans = ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->num_of_chans;
          ((T_TR_MPHC_UPDATE_BA_LIST *)(ptr))->pwrc         = ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->pwrc;
          ((T_TR_MPHC_UPDATE_BA_LIST *)(ptr))->dtx_allowed  = ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->dtx_allowed;
          ((T_TR_MPHC_UPDATE_BA_LIST *)(ptr))->ba_id        = ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->ba_id;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHC_UPDATE_BA_LIST));
        }
      }
      break;

    #if L1_GPRS

      /********************************************************************************/
      /* PACKET SWITCHED                                                              */
      /********************************************************************************/

      //////////////////////////////
      // Neighbor cell monitoring //
      //////////////////////////////

      // Neighbor PBCCH reading

      case MPHP_NCELL_PBCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_NCELL_PBCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->header             = TRL1_MPHP_NCELL_PBCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->bs_pbcch_blks      = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bs_pbcch_blks;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->pb                 = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->pb;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->psi1_repeat_period = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->psi1_repeat_period;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->relative_position  = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->relative_position;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->h                  = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->radio_freq         = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->timeslot_no        = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->tsc                = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->rf_chan_cnt        = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->bcch_carrier       = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bcch_carrier;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->fn_offset          = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->fn_offset;
          ((T_TR_MPHP_NCELL_PBCCH_REQ *)(ptr))->time_alignment     = ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->time_alignment;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_NCELL_PBCCH_REQ));
        }
      }
      break;

      case MPHP_NCELL_PBCCH_STOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_NCELL_PBCCH_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_NCELL_PBCCH_STOP_REQ *)(ptr))->header       = TRL1_MPHP_NCELL_PBCCH_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_NCELL_PBCCH_STOP_REQ));
        }
      }
      break;

      case L1P_PBCCHN_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PBCCHN_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->header            = TRL1_L1P_PBCCHN_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->input_level       = l1a_l1s_com.last_input_level[l1pa_l1ps_com.pbcchn.bcch_carrier -
                                                                                           l1_config.std.radio_freq_index_offset].input_level;
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PBCCHN_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PBCCHN_INFO));
        }
      }
      break;

      //////////////////////////////////////////////////////
      // Serving cell normal burst reading in packet idle //
      //////////////////////////////////////////////////////

      // PCCCH reading

      case MPHP_START_PCCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_START_PCCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->header          = TRL1_MPHP_START_PCCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->imsimod         = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->imsimod;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->kcn             = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->kcn;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->split_pg_cycle  = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->split_pg_cycle;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->bs_pag_blks_res = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pag_blks_res;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->bs_pbcch_blks   = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pbcch_blks;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->pb              = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->pb;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->page_mode       = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->page_mode;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->h               = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->radio_freq      = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->timeslot_no     = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->tsc             = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc;
          ((T_TR_MPHP_START_PCCCH_REQ *)(ptr))->rf_chan_cnt     = ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_START_PCCCH_REQ));
        }
      }
      break;

      case MPHP_STOP_PCCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_STOP_PCCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_STOP_PCCCH_REQ *)(ptr))->header       = TRL1_MPHP_STOP_PCCCH_REQ;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_STOP_PCCCH_REQ));
        }
      }
      break;

      case L1P_PNP_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PNP_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PNP_INFO *)(ptr))->header            = TRL1_L1P_PNP_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PNP_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PNP_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PNP_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PNP_INFO *)(ptr))->input_level       = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1P_PNP_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PNP_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PNP_INFO));
        }
      }
      break;

      case L1P_PEP_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PEP_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PEP_INFO *)(ptr))->header            = TRL1_L1P_PEP_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PEP_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PEP_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PEP_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PEP_INFO *)(ptr))->input_level       = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1P_PEP_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PEP_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PEP_INFO));
        }
      }
      break;

      case L1P_PALLC_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PALLC_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PALLC_INFO *)(ptr))->header            = TRL1_L1P_PALLC_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PALLC_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PALLC_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PALLC_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PALLC_INFO *)(ptr))->input_level       = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1P_PALLC_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PALLC_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PALLC_INFO));
        }
      }
      break;

      // PBCCH reading

      case MPHP_SCELL_PBCCH_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_SCELL_PBCCH_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->header                      = TRL1_MPHP_SCELL_PBCCH_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->nbr_psi                     = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->nbr_psi;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->bs_pbcch_blks               = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->bs_pbcch_blks;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->pb                          = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->pb;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->psi1_repeat_period          = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->psi1_repeat_period;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->h                           = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->radio_freq                  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->timeslot_no                 = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->tsc                         = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->rf_chan_cnt                 = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt;
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[0]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[0];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[1]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[1];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[2]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[2];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[3]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[3];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[4]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[4];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[5]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[5];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[6]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[6];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[7]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[7];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[8]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[8];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[9]  = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[9];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[10] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[10];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[11] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[11];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[12] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[12];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[13] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[13];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[14] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[14];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[15] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[15];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[16] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[16];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[17] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[17];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[18] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[18];
          ((T_TR_MPHP_SCELL_PBCCH_REQ *)(ptr))->relative_position_array[19] = ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[19];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_SCELL_PBCCH_REQ));
        }
      }
      break;

      case MPHP_SCELL_PBCCH_STOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_SCELL_PBCCH_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_SCELL_PBCCH_STOP_REQ *)(ptr))->header       = TRL1_MPHP_SCELL_PBCCH_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_SCELL_PBCCH_STOP_REQ));
        }
      }
      break;

      case L1P_PBCCHS_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PBCCHS_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->header            = TRL1_L1P_PBCCHS_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->input_level       = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PBCCHS_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PBCCHS_INFO));
        }
      }
      break;

      ///////////////////
      // Packet Access //
      ///////////////////

      // Random access

      case MPHP_RA_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_RA_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_RA_REQ *)(ptr))->header               = TRL1_MPHP_RA_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_RA_REQ *)(ptr))->txpwr                = ((T_MPHP_RA_REQ *)(msg->SigP))->txpwr;
          ((T_TR_MPHP_RA_REQ *)(ptr))->rand                 = ((T_MPHP_RA_REQ *)(msg->SigP))->rand;
          ((T_TR_MPHP_RA_REQ *)(ptr))->channel_request_data = ((T_MPHP_RA_REQ *)(msg->SigP))->channel_request_data;
          ((T_TR_MPHP_RA_REQ *)(ptr))->bs_prach_blks        = ((T_MPHP_RA_REQ *)(msg->SigP))->bs_prach_blks;
          ((T_TR_MPHP_RA_REQ *)(ptr))->access_burst_type    = ((T_MPHP_RA_REQ *)(msg->SigP))->access_burst_type;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_RA_REQ));
        }
      }
      break;

      case MPHP_RA_STOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_RA_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_RA_STOP_REQ *)(ptr))->header       = TRL1_MPHP_RA_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_RA_STOP_REQ));
        }
      }
      break;

      case L1P_RA_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_RA_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_RA_DONE *)(ptr))->header               = TRL1_L1P_RA_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_RA_DONE *)(ptr))->channel_request_data = ((T_MPHP_RA_CON *)(msg->SigP))->channel_request_data;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_RA_DONE));
        }
      }
      break;

      // Single block

      case MPHP_SINGLE_BLOCK_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_SINGLE_BLOCK_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          WORD32 sti;

          if (((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tbf_sti.present == 0)
            sti = -1;
          else
            sti = (WORD32) ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tbf_sti.absolute_fn;


          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->header            = TRL1_MPHP_SINGLE_BLOCK_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->assignment_id     = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->assignment_id;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->purpose           = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->purpose;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->pc_meas_chan      = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->pc_meas_chan;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->access_burst_type = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->access_burst_type;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->ta                = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->packet_ta.ta;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->p0                = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.p0;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->bts_pwr_ctl_mode  = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.bts_pwr_ctl_mode;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->pr_mode           = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.pr_mode;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->tsc               = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tsc;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->h                 = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.h;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->radio_freq        = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->rf_chan_cnt       = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.freq_list.rf_chan_cnt;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->tbf_sti           = sti;
          ((T_TR_MPHP_SINGLE_BLOCK_REQ *)(ptr))->timeslot_number   = ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->timeslot_number;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_SINGLE_BLOCK_REQ));
        }
      }
      break;

      case MPHP_STOP_SINGLE_BLOCK_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_STOP_SINGLE_BLOCK_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_STOP_SINGLE_BLOCK_REQ *)(ptr))->header       = TRL1_MPHP_STOP_SINGLE_BLOCK_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_STOP_SINGLE_BLOCK_REQ));
        }
      }
      break;

      case L1P_PACCH_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PACCH_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PACCH_INFO *)(ptr))->header            = TRL1_L1P_PACCH_INFO | (((T_MPHP_DATA_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PACCH_INFO *)(ptr))->error_flag        = ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag;
          ((T_TR_L1P_PACCH_INFO *)(ptr))->radio_freq        = ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq;
          ((T_TR_L1P_PACCH_INFO *)(ptr))->relative_position = ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position;
          ((T_TR_L1P_PACCH_INFO *)(ptr))->input_level       = l1a_l1s_com.Scell_IL_for_rxlev;
          ((T_TR_L1P_PACCH_INFO *)(ptr))->tpu_offset        = l1s.tpu_offset;
          ((T_TR_L1P_PACCH_INFO *)(ptr))->afc               = l1s.afc;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PACCH_INFO));
        }
      }
      break;

      case L1P_SINGLE_BLOCK_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_SINGLE_BLOCK_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->header        = TRL1_L1P_SINGLE_BLOCK_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->assignment_id = ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->assignment_id;
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->purpose       = ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->purpose;
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->status        = ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->status;
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->dl_error_flag = ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->dl_error_flag;
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->txpwr[0]      = l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[0];
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->txpwr[1]      = l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[1];
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->txpwr[2]      = l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[2];
          ((T_TR_L1P_SINGLE_BLOCK_CON *)(ptr))->txpwr[3]      = l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[3];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_SINGLE_BLOCK_CON));
        }
      }
      break;

      // Idle mode polling

      case MPHP_POLLING_RESPONSE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_POLLING_RESPONSE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_POLLING_RESPONSE_REQ *)(ptr))->header         = TRL1_MPHP_POLLING_RESPONSE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_POLLING_RESPONSE_REQ *)(ptr))->pol_resp_type  = ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->pol_resp_type;
          ((T_TR_MPHP_POLLING_RESPONSE_REQ *)(ptr))->fn_req         = ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->fn;
          ((T_TR_MPHP_POLLING_RESPONSE_REQ *)(ptr))->timing_advance = ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->timing_advance;
          ((T_TR_MPHP_POLLING_RESPONSE_REQ *)(ptr))->txpwr          = ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->txpwr;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_POLLING_RESPONSE_REQ));
        }
      }
      break;

      case L1P_POLL_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_POLL_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_POLL_DONE *)(ptr))->header       = TRL1_L1P_POLL_DONE | (((T_MPHP_POLLING_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_POLL_DONE));
        }
      }
      break;

      //////////////////////////
      // Packet transfer mode //
      //////////////////////////

      // Temporary block flow assignment

      case MPHP_ASSIGNMENT_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_ASSIGNMENT_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          WORD32 sti;

          if (((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tbf_sti.present == 0)
            sti = -1;
          else
            sti = (WORD32) ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tbf_sti.absolute_fn;


          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->header             = TRL1_MPHP_ASSIGNMENT_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->assignment_id      = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->assignment_id;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->assignment_command = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->assignment_command;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->multislot_class    = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->multislot_class;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->interf_meas_enable = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->interf_meas_enable;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->pc_meas_chan       = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->pc_meas_chan;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->access_burst_type  = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->access_burst_type;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->ta                 = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->ta_index           = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta_index;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->ta_tn              = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta_tn;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->p0                 = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.p0;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->bts_pwr_ctl_mode   = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.bts_pwr_ctl_mode;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->pr_mode            = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.pr_mode;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->tsc                = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tsc;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->h                  = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.chan_sel.h;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->radio_freq         = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->rf_chan_cnt        = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.freq_list.rf_chan_cnt;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->tbf_sti            = sti;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->mac_mode           = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->mac_mode;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->dl_ressource_alloc = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_ressource_alloc.timeslot_alloc;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->ul_ressource_alloc = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.timeslot_alloc;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->usf_granularity    = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.dynamic_alloc.usf_granularity;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->ctrl_timeslot      = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.fixed_alloc.ctrl_timeslot;
          ((T_TR_MPHP_ASSIGNMENT_REQ *)(ptr))->bitmap_length      = ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.fixed_alloc.bitmap_length;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_ASSIGNMENT_REQ));
        }
      }
      break;

      case MPHP_ASSIGNMENT_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_ASSIGNMENT_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_ASSIGNMENT_CON *)(ptr))->header       = TRL1_MPHP_ASSIGNMENT_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_ASSIGNMENT_CON *)(ptr))->dl_tn        = l1a_l1s_com.dl_tn;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_ASSIGNMENT_CON));
        }
      }
      break;

      case MPHP_TBF_RELEASE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_TBF_RELEASE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_TBF_RELEASE_REQ *)(ptr))->header       = TRL1_MPHP_TBF_RELEASE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_TBF_RELEASE_REQ *)(ptr))->tbf_type     = ((T_MPHP_TBF_RELEASE_REQ *)(msg->SigP))->tbf_type;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_TBF_RELEASE_REQ));
        }
      }
      break;

      case L1P_TBF_RELEASED:
      {
        char *ptr;

        if (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all == 1)
          trace_info.new_tcr_list = 0;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_TBF_RELEASED), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_TBF_RELEASED *)(ptr))->header       = TRL1_L1P_TBF_RELEASED | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_TBF_RELEASED *)(ptr))->released_all = ((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all;
          ((T_TR_L1P_TBF_RELEASED *)(ptr))->dl_tn        = l1a_l1s_com.dl_tn;
          ((T_TR_L1P_TBF_RELEASED *)(ptr))->tbf_type     = ((T_L1P_TBF_RELEASED *)(msg->SigP))->tbf_type;
          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_TBF_RELEASED));
        }
      }
      break;

      // PDCH release

      case MPHP_PDCH_RELEASE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_PDCH_RELEASE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_PDCH_RELEASE_REQ *)(ptr))->header             = TRL1_MPHP_PDCH_RELEASE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_PDCH_RELEASE_REQ *)(ptr))->assignment_id      = ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->assignment_id;
          ((T_TR_MPHP_PDCH_RELEASE_REQ *)(ptr))->timeslot_available = ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->timeslot_available;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_PDCH_RELEASE_REQ));
        }
      }
      break;

      case L1P_PDCH_RELEASED:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_PDCH_RELEASED), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_PDCH_RELEASED *)(ptr))->header        = TRL1_L1P_PDCH_RELEASED | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_PDCH_RELEASED *)(ptr))->assignment_id = ((T_MPHP_PDCH_RELEASE_CON *)(msg->SigP))->assignment_id;
          ((T_TR_L1P_PDCH_RELEASED *)(ptr))->dl_tn         = l1a_l1s_com.dl_tn;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_PDCH_RELEASED));
        }
      }
      break;

      // Timing advance update

      case MPHP_TIMING_ADVANCE_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_TIMING_ADVANCE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_TIMING_ADVANCE_REQ *)(ptr))->header        = TRL1_MPHP_TIMING_ADVANCE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_TIMING_ADVANCE_REQ *)(ptr))->assignment_id = ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->assignment_id;
          ((T_TR_MPHP_TIMING_ADVANCE_REQ *)(ptr))->ta            = ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta;
          ((T_TR_MPHP_TIMING_ADVANCE_REQ *)(ptr))->ta_index      = ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_index;
          ((T_TR_MPHP_TIMING_ADVANCE_REQ *)(ptr))->ta_tn         = ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_tn;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_TIMING_ADVANCE_REQ));
        }
      }
      break;

      case L1P_TA_CONFIG_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_TA_CONFIG_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_TA_CONFIG_DONE *)(ptr))->header       = TRL1_L1P_TA_CONFIG_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_TA_CONFIG_DONE));
        }
      }
      break;

      // Update PSI parameters

      case MPHP_UPDATE_PSI_PARAM_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_UPDATE_PSI_PARAM_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_UPDATE_PSI_PARAM_REQ *)(ptr))->header            = TRL1_MPHP_UPDATE_PSI_PARAM_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_UPDATE_PSI_PARAM_REQ *)(ptr))->pb                = ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->pb;
          ((T_TR_MPHP_UPDATE_PSI_PARAM_REQ *)(ptr))->access_burst_type = ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->access_burst_type;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_UPDATE_PSI_PARAM_REQ));
        }
      }
      break;

      // Fixed mode repeat allocation

      case MPHP_REPEAT_UL_FIXED_ALLOC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          WORD32 sti;

          if (((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->tbf_sti.present == 0)
            sti = -1;
          else
            sti = (WORD32) ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->tbf_sti.absolute_fn;


          ((T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(ptr))->header            = TRL1_MPHP_REPEAT_UL_FIXED_ALLOC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(ptr))->repeat_allocation = ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->repeat_allocation;
          ((T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(ptr))->ts_override       = ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->ts_override;
          ((T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(ptr))->tbf_sti           = sti;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_REPEAT_UL_FIXED_ALLOC_REQ));
        }
      }
      break;

      case L1P_REPEAT_ALLOC_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_REPEAT_ALLOC_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_REPEAT_ALLOC_DONE *)(ptr))->header       = TRL1_L1P_REPEAT_ALLOC_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_REPEAT_ALLOC_DONE *)(ptr))->dl_tn        = l1a_l1s_com.dl_tn;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_REPEAT_ALLOC_DONE));
        }
      }
      break;

      // Fixed mode allocation exhaustion

      case L1P_ALLOC_EXHAUST_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_ALLOC_EXHAUST_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_ALLOC_EXHAUST_DONE *)(ptr))->header       = TRL1_L1P_ALLOC_EXHAUST_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_ALLOC_EXHAUST_DONE *)(ptr))->dl_tn        = l1a_l1s_com.dl_tn;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_ALLOC_EXHAUST_DONE));
        }
      }
      break;

      //////////////////////////////
      // Packet mode measurements //
      //////////////////////////////

      // BA list measurements in packet idle

      case MPHP_CR_MEAS_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_CR_MEAS_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_CR_MEAS_REQ *)(ptr))->header       = TRL1_MPHP_CR_MEAS_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_CR_MEAS_REQ *)(ptr))->nb_carrier   = ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->nb_carrier;
          ((T_TR_MPHP_CR_MEAS_REQ *)(ptr))->list_id      = ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->list_id;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_CR_MEAS_REQ));
        }
      }
      break;

      case MPHP_CR_MEAS_STOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_CR_MEAS_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_CR_MEAS_STOP_REQ *)(ptr))->header       = TRL1_MPHP_CR_MEAS_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_CR_MEAS_STOP_REQ));
        }
      }
      break;

      case L1P_CR_MEAS_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_CR_MEAS_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          UWORD8 i,nmeas;


          ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->header           = TRL1_L1P_CR_MEAS_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->nmeas            = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->nmeas;
          ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->list_id          = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->list_id;
          ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->reporting_period = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->reporting_period;

          nmeas = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->nmeas;
          if (nmeas > MAX_CR) nmeas = MAX_CR;

          for (i=0;i<nmeas;i++)
          {
            ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->freq[i]        = l1pa_l1ps_com.cres_freq_list.alist->freq_list[i];
            ((T_TR_L1P_CR_MEAS_DONE *)(ptr))->rxlev[i]       = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->ncell_meas[i].rxlev;
          }

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_CR_MEAS_DONE));
        }
      }
      break;

      // BA list measurements in packet transfer

      case MPHP_TCR_MEAS_REQ:
      {
        char *ptr;

        trace_info.new_tcr_list = 1;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_TCR_MEAS_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_TCR_MEAS_REQ *)(ptr))->header       = TRL1_MPHP_TCR_MEAS_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_TCR_MEAS_REQ *)(ptr))->nb_carrier   = ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->nb_carrier;
          ((T_TR_MPHP_TCR_MEAS_REQ *)(ptr))->list_id      = ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->list_id;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_TCR_MEAS_REQ));
        }
      }
      break;

      case MPHP_TCR_MEAS_STOP_REQ:
      {
        char *ptr;

        trace_info.new_tcr_list = 0;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_TCR_MEAS_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_TCR_MEAS_STOP_REQ *)(ptr))->header       = TRL1_MPHP_TCR_MEAS_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_TCR_MEAS_STOP_REQ));
        }
      }
      break;

      case L1P_TCR_MEAS_DONE:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_TCR_MEAS_DONE), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {
          UWORD8            i, nmeas;
          T_CRES_LIST_PARAM *list_ptr;

          if (trace_info.new_tcr_list == 0)
            // No TCR list update: keep the alist pointer
            list_ptr = l1pa_l1ps_com.cres_freq_list.alist;
          else
          {
            // In case of TCR list updating, the alist pointer has changed
            if(l1pa_l1ps_com.cres_freq_list.alist == &(l1pa_l1ps_com.cres_freq_list.list[0]))
              list_ptr = &(l1pa_l1ps_com.cres_freq_list.list[1]);
            else
              list_ptr = &(l1pa_l1ps_com.cres_freq_list.list[0]);
            //Reset the variable new_tcr_list so that next time onwards the new list of
            //frequencies will get printed.
            trace_info.new_tcr_list = 0;
          }


          ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->header       = TRL1_L1P_TCR_MEAS_DONE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->list_id      = ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->list_id;
          ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->nb_carrier   = list_ptr->nb_carrier;

          nmeas = list_ptr->nb_carrier;
          if (nmeas > MAX_TCR) nmeas = MAX_TCR;

          for (i=0;i<nmeas;i++)
          {
            ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->radio_freq[i] = list_ptr->freq_list[i];
            ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->acc_level[i]  = ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->acc_level[i];
            ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->acc_nbmeas[i] = ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->acc_nbmeas[i];
          }

          ((T_TR_L1P_TCR_MEAS_DONE *)(ptr))->tpu_offset = l1s.tpu_offset;


          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_TCR_MEAS_DONE));
        }
      }
      break;

      ///////////////////////////////
      // Interference measurements //
      ///////////////////////////////

      // Interference measurements in packet idle

      case MPHP_INT_MEAS_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_INT_MEAS_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->header          = TRL1_MPHP_INT_MEAS_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->h               = ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.h;
          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->radio_freq      = ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.rf_channel.single_rf.radio_freq;
          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->rf_chan_cnt     = ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.freq_list.rf_chan_cnt;
          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->tn              = ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->tn;
          ((T_TR_MPHP_INT_MEAS_REQ *)(ptr))->multislot_class = ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->multislot_class;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_INT_MEAS_REQ));
        }
      }
      break;

      case MPHP_INT_MEAS_STOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_INT_MEAS_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_INT_MEAS_STOP_REQ *)(ptr))->header       = TRL1_MPHP_INT_MEAS_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_INT_MEAS_STOP_REQ));
        }
      }
      break;

      case L1P_ITMEAS_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1P_ITMEAS_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1P_ITMEAS_IND *)(ptr))->header       = TRL1_L1P_ITMEAS_IND | (((T_L1P_ITMEAS_IND *)(msg->SigP))->fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1P_ITMEAS_IND *)(ptr))->position     = ((T_L1P_ITMEAS_IND *)(msg->SigP))->position;
          ((T_TR_L1P_ITMEAS_IND *)(ptr))->meas_bitmap  = ((T_L1P_ITMEAS_IND *)(msg->SigP))->meas_bitmap;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1P_ITMEAS_IND));
        }
      }
      break;

      case MPHP_INT_MEAS_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_INT_MEAS_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->header       = TRL1_MPHP_INT_MEAS_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[0]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[1]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[2]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[3]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[4]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[5]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[6]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_0[7]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[0]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[1]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[2]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[3]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[4]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[5]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[6]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1];
          ((T_TR_MPHP_INT_MEAS_IND *)(ptr))->rxlev_1[7]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_INT_MEAS_IND));
        }
      }
      break;

      // Interference measurements in packet transfer

      case MPHP_TINT_MEAS_IND:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MPHP_TINT_MEAS_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->header       = TRL1_MPHP_TINT_MEAS_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[0]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[1]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[2]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[3]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[4]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[5]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[6]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_0[7]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[0]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[1]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[2]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[3]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[4]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[5]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[6]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1];
          ((T_TR_MPHP_TINT_MEAS_IND *)(ptr))->rxlev_1[7]   = ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1];

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MPHP_TINT_MEAS_IND));
        }
      }
      break;

    #endif

    #if (AUDIO_TASK == 1)

      /********************************************************************************/
      /* BACKGROUND TASKS                                                             */
      /********************************************************************************/

      //////////////////
      // MMI messages //
      //////////////////

      #if (KEYBEEP)
        // Keybeep
        case MMI_KEYBEEP_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_KEYBEEP_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_KEYBEEP_START_REQ *)(ptr))->header       = TRL1_MMI_KEYBEEP_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_KEYBEEP_START_REQ *)(ptr))->d_k_x1_kt0   = ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt0;
            ((T_TR_MMI_KEYBEEP_START_REQ *)(ptr))->d_k_x1_kt1   = ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt1;
            ((T_TR_MMI_KEYBEEP_START_REQ *)(ptr))->d_dur_kb     = ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_dur_kb;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_KEYBEEP_START_REQ));
          }
        }
        break;

        case MMI_KEYBEEP_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_KEYBEEP_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_KEYBEEP_START_CON *)(ptr))->header       = TRL1_MMI_KEYBEEP_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_KEYBEEP_START_CON));
          }
        }
        break;

        case MMI_KEYBEEP_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_KEYBEEP_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_KEYBEEP_STOP_REQ *)(ptr))->header       = TRL1_MMI_KEYBEEP_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_KEYBEEP_STOP_REQ));
          }
        }
        break;

        case MMI_KEYBEEP_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_KEYBEEP_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_KEYBEEP_STOP_CON *)(ptr))->header       = TRL1_MMI_KEYBEEP_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_KEYBEEP_STOP_CON));
          }
        }
        break;
      #endif // KEYBEEP

      #if (TONE)
        // Tone
        case MMI_TONE_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TONE_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_TONE_START_REQ *)(ptr))->header       = TRL1_MMI_TONE_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_k_x1_t0    = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t0;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_k_x1_t1    = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t1;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_k_x1_t2    = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t2;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_pe_rep     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_rep;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_pe_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_off;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_se_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_se_off;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_bu_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_bu_off;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t0_on      = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_on;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t0_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_off;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t1_on      = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_on;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t1_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_off;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t2_on      = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_on;
            ((T_TR_MMI_TONE_START_REQ *)(ptr))->d_t2_off     = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_off;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TONE_START_REQ));
          }
        }
        break;

        case MMI_TONE_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TONE_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_TONE_START_CON *)(ptr))->header       = TRL1_MMI_TONE_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TONE_START_CON));
          }
        }
       break;

        case MMI_TONE_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TONE_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_TONE_STOP_REQ *)(ptr))->header       = TRL1_MMI_TONE_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TONE_STOP_REQ));
          }
        }
        break;

        case MMI_TONE_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TONE_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_TONE_STOP_CON *)(ptr))->header       = TRL1_MMI_TONE_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TONE_STOP_CON));
          }
        }
        break;
      #endif // TONE

      #if (MELODY_E1)
        // Melody 0
        case MMI_MELODY0_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_START_REQ *)(ptr))->header                 = TRL1_MMI_MELODY0_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_MELODY0_START_REQ *)(ptr))->session_id             = ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id;
            ((T_TR_MMI_MELODY0_START_REQ *)(ptr))->loopback               = ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback;
            ((T_TR_MMI_MELODY0_START_REQ *)(ptr))->oscillator_used_bitmap = ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_START_REQ));
          }
        }
        break;

        case MMI_MELODY0_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_START_CON *)(ptr))->header       = TRL1_MMI_MELODY0_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_START_CON));
          }
        }
        break;

        case MMI_MELODY0_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_STOP_REQ *)(ptr))->header       = TRL1_MMI_MELODY0_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_STOP_REQ));
          }
        }
        break;

        case MMI_MELODY0_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_STOP_CON *)(ptr))->header       = TRL1_MMI_MELODY0_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_STOP_CON));
          }
        }
        break;

              // Melody 1
        case MMI_MELODY1_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_START_REQ *)(ptr))->header                 = TRL1_MMI_MELODY1_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_MELODY1_START_REQ *)(ptr))->session_id             = ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id;
            ((T_TR_MMI_MELODY1_START_REQ *)(ptr))->loopback               = ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback;
            ((T_TR_MMI_MELODY1_START_REQ *)(ptr))->oscillator_used_bitmap = ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_START_REQ));
          }
        }
        break;

        case MMI_MELODY1_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_START_CON *)(ptr))->header       = TRL1_MMI_MELODY1_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_START_CON));
          }
        }
        break;

        case MMI_MELODY1_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_STOP_REQ *)(ptr))->header       = TRL1_MMI_MELODY1_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_STOP_REQ));
          }
        }
        break;

        case MMI_MELODY1_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_STOP_CON *)(ptr))->header       = TRL1_MMI_MELODY1_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_STOP_CON));
          }
        }
        break;
      #endif // MELODY_E1

      #if (VOICE_MEMO)
        // Voice memo recording
        case MMI_VM_RECORD_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_RECORD_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->header          = TRL1_MMI_VM_RECORD_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->session_id      = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->session_id;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->maximum_size    = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->maximum_size;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->dtx_used        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->dtx_used;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->record_coeff_dl = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_dl;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->record_coeff_ul = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_ul;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_k_x1_t0       = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t0;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_k_x1_t1       = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t1;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_k_x1_t2       = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t2;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_pe_rep        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_rep;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_pe_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_off;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_se_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_se_off;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_bu_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_bu_off;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t0_on         = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_on;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t0_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_off;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t1_on         = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_on;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t1_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_off;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t2_on         = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_on;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->d_t2_off        = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_off;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_RECORD_START_REQ));
          }
        }
        break;

        case MMI_VM_RECORD_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_RECORD_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_RECORD_START_CON *)(ptr))->header       = TRL1_MMI_VM_RECORD_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_RECORD_START_CON));
          }
        }
        break;

        case MMI_VM_RECORD_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_RECORD_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_RECORD_STOP_REQ *)(ptr))->header       = TRL1_MMI_VM_RECORD_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_RECORD_STOP_REQ));
          }
        }
        break;

        case MMI_VM_RECORD_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_RECORD_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_RECORD_STOP_CON *)(ptr))->header       = TRL1_MMI_VM_RECORD_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_RECORD_STOP_CON));
          }
        }
        break;

        // Voice memo playing
        case MMI_VM_PLAY_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_PLAY_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_PLAY_START_REQ *)(ptr))->header       = TRL1_MMI_VM_PLAY_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_VM_PLAY_START_REQ *)(ptr))->session_id   = ((T_MMI_VM_PLAY_REQ *)(msg->SigP))->session_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_PLAY_START_REQ));
          }
        }
        break;

        case MMI_VM_PLAY_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_PLAY_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_PLAY_START_CON *)(ptr))->header       = TRL1_MMI_VM_PLAY_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_PLAY_START_CON));
          }
        }
        break;

        case MMI_VM_PLAY_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_PLAY_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_PLAY_STOP_REQ *)(ptr))->header       = TRL1_MMI_VM_PLAY_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_PLAY_STOP_REQ));
          }
        }
        break;

        case MMI_VM_PLAY_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_PLAY_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_PLAY_STOP_CON *)(ptr))->header       = TRL1_MMI_VM_PLAY_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_PLAY_STOP_CON));
          }
        }
        break;
      #endif // VOICE_MEMO

      #if (L1_VOICE_MEMO_AMR)
        // Voice memo recording
        case MMI_VM_AMR_RECORD_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RECORD_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RECORD_START_REQ *)(ptr))->header          = TRL1_MMI_VM_AMR_RECORD_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_VM_AMR_RECORD_START_REQ *)(ptr))->session_id      = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->session_id;
            ((T_TR_MMI_VM_AMR_RECORD_START_REQ *)(ptr))->maximum_size    = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->maximum_size;
            ((T_TR_MMI_VM_AMR_RECORD_START_REQ *)(ptr))->dtx_used        = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->dtx_used;
            ((T_TR_MMI_VM_RECORD_START_REQ *)(ptr))->record_coeff_ul     = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->record_coeff_ul;
            ((T_TR_MMI_VM_AMR_RECORD_START_REQ *)(ptr))->amr_vocoder     = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->amr_vocoder;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RECORD_START_REQ));
          }
        }
        break;

        case MMI_VM_AMR_RECORD_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RECORD_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RECORD_START_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_RECORD_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RECORD_START_CON));
          }
        }
        break;

        case MMI_VM_AMR_RECORD_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RECORD_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RECORD_STOP_REQ *)(ptr))->header       = TRL1_MMI_VM_AMR_RECORD_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RECORD_STOP_REQ));
          }
        }
        break;

        case MMI_VM_AMR_RECORD_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RECORD_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RECORD_STOP_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_RECORD_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RECORD_STOP_CON));
          }
        }
        break;

        // Voice memo playing
        case MMI_VM_AMR_PLAY_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PLAY_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PLAY_START_REQ *)(ptr))->header       = TRL1_MMI_VM_AMR_PLAY_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_VM_AMR_PLAY_START_REQ *)(ptr))->session_id   = ((T_MMI_VM_AMR_PLAY_REQ *)(msg->SigP))->session_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PLAY_START_REQ));
          }
        }
        break;

        case MMI_VM_AMR_PLAY_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PLAY_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PLAY_START_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_PLAY_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PLAY_START_CON));
          }
        }
        break;

        case MMI_VM_AMR_PLAY_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PLAY_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PLAY_STOP_REQ *)(ptr))->header       = TRL1_MMI_VM_AMR_PLAY_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PLAY_STOP_REQ));
          }
        }
        break;

        case MMI_VM_AMR_PLAY_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PLAY_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PLAY_STOP_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_PLAY_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PLAY_STOP_CON));
          }
        }
        break;
        case MMI_VM_AMR_PAUSE_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PAUSE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PAUSE_REQ *)(ptr))->header  = TRL1_MMI_VM_AMR_PAUSE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);


            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PAUSE_REQ));
          }
        }
        break;
        case MMI_VM_AMR_RESUME_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RESUME_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RESUME_REQ *)(ptr))->header       = TRL1_MMI_VM_AMR_RESUME_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
         //  ((T_TR_MMI_VM_AMR_RESUME_REQ *)(ptr))->session_id   = ((T_MMI_VM_AMR_RESUME_REQ *)(msg->SigP))->session_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RESUME_REQ));
          }
        }
        break;

 case MMI_VM_AMR_PAUSE_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_PAUSE_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_PAUSE_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_PAUSE_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_PAUSE_CON));
          }
        }
        break;
 case MMI_VM_AMR_RESUME_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_VM_AMR_RESUME_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_VM_AMR_RESUME_CON *)(ptr))->header       = TRL1_MMI_VM_AMR_RESUME_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_VM_AMR_RESUME_CON));
          }
        }
        break;




      #endif // L1_VOICE_MEMO_AMR

      #if (SPEECH_RECO)
        case MMI_SR_ENROLL_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_ENROLL_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_ENROLL_START_REQ *)(ptr))->header          = TRL1_MMI_SR_ENROLL_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_ENROLL_START_REQ *)(ptr))->database_id     = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->database_id;
            ((T_TR_MMI_SR_ENROLL_START_REQ *)(ptr))->word_index      = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->word_index;
            ((T_TR_MMI_SR_ENROLL_START_REQ *)(ptr))->speech          = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech;
            ((T_TR_MMI_SR_ENROLL_START_REQ *)(ptr))->speech_address  = (UWORD32) ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech_address;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_ENROLL_START_REQ));
          }
        }
        break;

        case MMI_SR_ENROLL_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_ENROLL_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_ENROLL_STOP_REQ *)(ptr))->header       = TRL1_MMI_SR_ENROLL_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_ENROLL_STOP_REQ));
          }
        }
        break;

        case MMI_SR_ENROLL_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_ENROLL_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_ENROLL_START_CON *)(ptr))->header       = TRL1_MMI_SR_ENROLL_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_ENROLL_START_CON));
          }
        }
        break;

        case MMI_SR_ENROLL_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_ENROLL_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_ENROLL_STOP_CON *)(ptr))->header       = TRL1_MMI_SR_ENROLL_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_ENROLL_STOP_CON *)(ptr))->error_id     = ((T_MMI_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_ENROLL_STOP_CON));
          }
        }
        break;

        case MMI_SR_UPDATE_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_START_REQ *)(ptr))->header         = TRL1_MMI_SR_UPDATE_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_UPDATE_START_REQ *)(ptr))->database_id    = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->database_id;
            ((T_TR_MMI_SR_UPDATE_START_REQ *)(ptr))->word_index     = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->word_index;
            ((T_TR_MMI_SR_UPDATE_START_REQ *)(ptr))->speech         = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech;
            ((T_TR_MMI_SR_UPDATE_START_REQ *)(ptr))->speech_address = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech_address;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_START_REQ));
          }
        }
        break;

        case MMI_SR_UPDATE_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_STOP_REQ *)(ptr))->header       = TRL1_MMI_SR_UPDATE_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_STOP_REQ));
          }
        }
        break;

        case MMI_SR_UPDATE_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_START_CON *)(ptr))->header       = TRL1_MMI_SR_UPDATE_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_START_CON));
          }
        }
        break;

        case MMI_SR_UPDATE_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_STOP_CON *)(ptr))->header       = TRL1_MMI_SR_UPDATE_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_UPDATE_STOP_CON *)(ptr))->error_id     = ((T_MMI_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_STOP_CON));
          }
        }
        break;

        case MMI_SR_RECO_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_RECO_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_RECO_START_REQ *)(ptr))->header          = TRL1_MMI_SR_RECO_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_RECO_START_REQ *)(ptr))->database_id     = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->database_id;
            ((T_TR_MMI_SR_RECO_START_REQ *)(ptr))->vocabulary_size = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->vocabulary_size;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_RECO_START_REQ));
          }
        }
        break;

        case MMI_SR_RECO_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_RECO_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_RECO_STOP_REQ *)(ptr))->header       = TRL1_MMI_SR_RECO_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_RECO_STOP_REQ));
          }
        }
        break;

        case MMI_SR_RECO_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_RECO_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_RECO_START_CON *)(ptr))->header       = TRL1_MMI_SR_RECO_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_RECO_START_CON));
          }
        }
        break;

        case MMI_SR_RECO_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_RECO_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->header                 = TRL1_MMI_SR_RECO_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->error_id               = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->error_id;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->best_word_index        = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_index;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->best_word_score        = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_score;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->second_best_word_index = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_index;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->second_best_word_score = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_score;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->third_best_word_index  = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_index;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->third_best_word_score  = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_score;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->fourth_best_word_index = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_index;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->fourth_best_word_score = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_score;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->d_sr_db_level          = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_level;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->d_sr_db_noise          = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_noise;
            ((T_TR_MMI_SR_RECO_STOP_CON *)(ptr))->d_sr_model_size        = ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_model_size;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_RECO_STOP_CON));
          }
        }
        break;

        case MMI_SR_UPDATE_CHECK_START_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_CHECK_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->header          = TRL1_MMI_SR_UPDATE_CHECK_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->database_id     = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->database_id;
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->word_index      = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index;
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->model_address   = (UWORD32) ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->model_address;
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->speech          = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech;
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->speech_address  = (UWORD32) ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech_address;
            ((T_TR_MMI_SR_UPDATE_CHECK_START_REQ *)(ptr))->vocabulary_size = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->vocabulary_size;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_CHECK_START_REQ));
          }
        }
        break;

        case MMI_SR_UPDATE_CHECK_STOP_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_CHECK_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_REQ *)(ptr))->header       = TRL1_MMI_SR_UPDATE_CHECK_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_CHECK_STOP_REQ));
          }
        }
        break;

        case MMI_SR_UPDATE_CHECK_START_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_CHECK_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_CHECK_START_CON *)(ptr))->header       = TRL1_MMI_SR_UPDATE_CHECK_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_CHECK_START_CON));
          }
        }
        break;

        case MMI_SR_UPDATE_CHECK_STOP_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_SR_UPDATE_CHECK_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->header                 = TRL1_MMI_SR_UPDATE_CHECK_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->error_id               = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->error_id;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->best_word_index        = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_index;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->best_word_score        = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_score;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->second_best_word_index = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_index;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->second_best_word_score = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_score;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->third_best_word_index  = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_index;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->third_best_word_score  = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_score;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->fourth_best_word_index = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_index;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->fourth_best_word_score = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_score;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->d_sr_db_level          = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_level;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->d_sr_db_noise          = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_noise;
            ((T_TR_MMI_SR_UPDATE_CHECK_STOP_CON *)(ptr))->d_sr_model_size        = ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_model_size;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_SR_UPDATE_CHECK_STOP_CON));
          }
        }
        break;

        case L1_SRBACK_SAVE_DATA_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_SAVE_DATA_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->header            = TRL1_L1_SRBACK_SAVE_DATA_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->database_id       = ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->database_id;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->model_index       = ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_index;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->model_RAM_address = (UWORD32) ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->speech            = ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->speech;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->start_buffer      = (UWORD32) ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_buffer;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->stop_buffer       = (UWORD32) ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_buffer;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->start_address     = (UWORD32) ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_address;
            ((T_TR_L1_SRBACK_SAVE_DATA_REQ *)(ptr))->stop_address      = (UWORD32) ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_address;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_SAVE_DATA_REQ));
          }
        }
        break;

        case L1_SRBACK_SAVE_DATA_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_SAVE_DATA_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_SAVE_DATA_CON *)(ptr))->header       = TRL1_L1_SRBACK_SAVE_DATA_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_SAVE_DATA_CON));
          }
        }
        break;

        case L1_SRBACK_LOAD_MODEL_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_LOAD_MODEL_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_LOAD_MODEL_REQ *)(ptr))->header            = TRL1_L1_SRBACK_LOAD_MODEL_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_SRBACK_LOAD_MODEL_REQ *)(ptr))->database_id       = ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->database_id;
            ((T_TR_L1_SRBACK_LOAD_MODEL_REQ *)(ptr))->model_index       = ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_index;
            ((T_TR_L1_SRBACK_LOAD_MODEL_REQ *)(ptr))->model_RAM_address = (UWORD32) ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_RAM_address;
            ((T_TR_L1_SRBACK_LOAD_MODEL_REQ *)(ptr))->CTO_enable        = ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->CTO_enable;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_LOAD_MODEL_REQ));
          }
        }
        break;

        case L1_SRBACK_LOAD_MODEL_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_LOAD_MODEL_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_LOAD_MODEL_CON *)(ptr))->header       = TRL1_L1_SRBACK_LOAD_MODEL_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_LOAD_MODEL_CON));
          }
        }
        break;

        case L1_SRBACK_TEMP_SAVE_DATA_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(ptr))->header                   = TRL1_L1_SRBACK_TEMP_SAVE_DATA_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(ptr))->model_RAM_address_input  = (UWORD32) ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_input;
            ((T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(ptr))->model_RAM_address_output = (UWORD32) ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_output;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_TEMP_SAVE_DATA_REQ));
          }
        }
        break;

        case L1_SRBACK_TEMP_SAVE_DATA_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_SRBACK_TEMP_SAVE_DATA_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_SRBACK_TEMP_SAVE_DATA_CON *)(ptr))->header       = TRL1_L1_SRBACK_TEMP_SAVE_DATA_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_SRBACK_TEMP_SAVE_DATA_CON));
          }
        }
        break;
      #endif  // SPEECH_RECO
      #if (FIR)
        case MMI_AUDIO_FIR_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AUDIO_FIR_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AUDIO_FIR_REQ *)(ptr))->header             = TRL1_MMI_AUDIO_FIR_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_AUDIO_FIR_REQ *)(ptr))->fir_loop           = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_loop;
            ((T_TR_MMI_AUDIO_FIR_REQ *)(ptr))->update_fir         = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->update_fir;
            ((T_TR_MMI_AUDIO_FIR_REQ *)(ptr))->fir_ul_coefficient = (UWORD32) ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_ul_coefficient;
            ((T_TR_MMI_AUDIO_FIR_REQ *)(ptr))->fir_dl_coefficient = (UWORD32) ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_dl_coefficient;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AUDIO_FIR_REQ));
          }
        }
        break;

        case MMI_AUDIO_FIR_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AUDIO_FIR_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AUDIO_FIR_CON *)(ptr))->header       = TRL1_MMI_AUDIO_FIR_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AUDIO_FIR_CON));
          }
        }
        break;
      #endif //FIR
      #if (L1_AEC == 1)
        case MMI_AEC_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AEC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AEC_REQ *)(ptr))->header          = TRL1_MMI_AEC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_AEC_REQ *)(ptr))->aec_control     = ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control;

          #if (L1_NEW_AEC)
            ((T_TR_MMI_AEC_REQ *)(ptr))->cont_filter     = ((T_MMI_AEC_REQ *)(msg->SigP))->cont_filter;
            ((T_TR_MMI_AEC_REQ *)(ptr))->granularity_att = ((T_MMI_AEC_REQ *)(msg->SigP))->granularity_att;
            ((T_TR_MMI_AEC_REQ *)(ptr))->coef_smooth     = ((T_MMI_AEC_REQ *)(msg->SigP))->coef_smooth;
            ((T_TR_MMI_AEC_REQ *)(ptr))->es_level_max    = ((T_MMI_AEC_REQ *)(msg->SigP))->es_level_max;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_vad        = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_vad;
            ((T_TR_MMI_AEC_REQ *)(ptr))->thrs_abs        = ((T_MMI_AEC_REQ *)(msg->SigP))->thrs_abs;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_asd_fil    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_fil;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_asd_mut    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_mut;
          #else
            ((T_TR_MMI_AEC_REQ *)(ptr))->cont_filter     = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->granularity_att = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->coef_smooth     = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->es_level_max    = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_vad        = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->thrs_abs        = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_asd_fil    = 0;
            ((T_TR_MMI_AEC_REQ *)(ptr))->fact_asd_mut    = 0;
          #endif

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AEC_REQ));
          }
        }
        break;

        #if (L1_NEW_AEC)
          case L1_AEC_IND:
          {
            char *ptr;

            if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_AEC_IND), (T_RVT_BUFFER *) &ptr) == RVT_OK)
            {

              ((T_TR_L1_AEC_IND *)(ptr))->header             = TRL1_L1_AEC_IND | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
              ((T_TR_L1_AEC_IND *)(ptr))->far_end_pow        = ((T_L1_AEC_IND *)(msg->SigP))->far_end_pow;
              ((T_TR_L1_AEC_IND *)(ptr))->far_end_noise      = ((T_L1_AEC_IND *)(msg->SigP))->far_end_noise;
              ((T_TR_L1_AEC_IND *)(ptr))->es_level           = ((T_L1_AEC_IND *)(msg->SigP))->es_level;

              L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_AEC_IND));
            }
          }
        #endif

        case MMI_AEC_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AEC_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AEC_CON *)(ptr))->header       = TRL1_MMI_AEC_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AEC_CON));
          }
        }
        break;
      #endif //AEC

      #if (AUDIO_MODE)
        case MMI_AUDIO_MODE_REQ:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AUDIO_MODE_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AUDIO_MODE_REQ *)(ptr))->header       = TRL1_MMI_AUDIO_MODE_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_AUDIO_MODE_REQ *)(ptr))->audio_mode   = ((T_MMI_AUDIO_MODE *)(msg->SigP))->audio_mode;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AUDIO_MODE_REQ));
          }
        }
        break;

        case MMI_AUDIO_MODE_CON:
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_AUDIO_MODE_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_AUDIO_MODE_CON *)(ptr))->header       = TRL1_MMI_AUDIO_MODE_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_AUDIO_MODE_CON));
          }
        }
        break;

      #endif // AUDIO_MODE

      #if (MELODY_E2)
        // MMI command and request
        case MMI_MELODY0_E2_START_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_E2_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_E2_START_REQ *)(ptr))->header       = TRL1_MMI_MELODY0_E2_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_MELODY0_E2_START_REQ *)(ptr))->session_id   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
            ((T_TR_MMI_MELODY0_E2_START_REQ *)(ptr))->loopback     = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_E2_START_REQ));
          }
        }
        break;

        case MMI_MELODY0_E2_STOP_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_E2_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_E2_STOP_REQ *)(ptr))->header       = TRL1_MMI_MELODY0_E2_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_E2_STOP_REQ));
          }
        }
        break;

        case MMI_MELODY0_E2_START_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_E2_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_E2_START_CON *)(ptr))->header       = TRL1_MMI_MELODY0_E2_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_E2_START_CON));
          }
        }
        break;

        case MMI_MELODY0_E2_STOP_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_E2_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY0_E2_STOP_CON *)(ptr))->header       = TRL1_MMI_MELODY0_E2_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY0_E2_STOP_CON));
          }
        }
        break;

        case MMI_MELODY1_E2_START_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY0_E2_START_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_E2_START_REQ *)(ptr))->header       = TRL1_MMI_MELODY1_E2_START_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_MMI_MELODY1_E2_START_REQ *)(ptr))->session_id   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
            ((T_TR_MMI_MELODY1_E2_START_REQ *)(ptr))->loopback     = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_E2_START_REQ));
          }
        }
        break;

        case MMI_MELODY1_E2_STOP_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_E2_STOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_E2_STOP_REQ *)(ptr))->header       = TRL1_MMI_MELODY1_E2_STOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_E2_STOP_REQ));
          }
        }
        break;

        case MMI_MELODY1_E2_START_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_E2_START_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_E2_START_CON *)(ptr))->header       = TRL1_MMI_MELODY1_E2_START_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_E2_START_CON));
          }
        }
        break;

        case MMI_MELODY1_E2_STOP_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_MELODY1_E2_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_MMI_MELODY1_E2_STOP_CON *)(ptr))->header       = TRL1_MMI_MELODY1_E2_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_MELODY1_E2_STOP_CON));
          }
        }
        break;

        // Audio download instrument message
        case L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->header                 = TRL1_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->melody_id              = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id;
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->number_of_instrument   = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument;
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[0]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[0];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[1]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[1];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[2]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[2];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[3]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[3];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[4]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[4];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[5]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[5];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[6]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[6];
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(ptr))->waves_table_id[7]      = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[7];

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ));
          }
        }
        break;

        case L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(ptr))->header                 = TRL1_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(ptr))->melody_id              = ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON));
          }
        }
        break;

        case L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(ptr))->header                 = TRL1_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(ptr))->melody_id              = ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id;
            ((T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(ptr))->number_of_instrument   = ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ));
          }
        }
        break;

        case L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(ptr))->header                 = TRL1_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            ((T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(ptr))->melody_id              = ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id;

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ));
          }
        }
        break;

        // L1S stop confirmation
        case L1_MELODY0_E2_STOP_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_MELODY0_E2_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_MELODY0_E2_STOP_CON *)(ptr))->header                 = TRL1_L1_MELODY0_E2_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_MELODY0_E2_STOP_CON));
          }
        }
        break;

        case L1_MELODY1_E2_STOP_CON :
        {
          char *ptr;

          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_MELODY1_E2_STOP_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {

            ((T_TR_L1_MELODY1_E2_STOP_CON *)(ptr))->header                 = TRL1_L1_MELODY1_E2_STOP_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_MELODY1_E2_STOP_CON));
          }
        }
        break;

        // Instrument download
      #endif // MELODY_E2
#if (L1_VOCODER_IF_CHANGE == 1)
        case MMI_TCH_VOCODER_CFG_REQ:
        {
          char *ptr;
          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TCH_VOCODER_CFG_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {
            ((T_TR_MMI_TCH_VOCODER_CFG_REQ *)(ptr))->header       = TRL1_MMI_TCH_VOCODER_CFG_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TCH_VOCODER_CFG_REQ));
          }
        }
        break;
        case MMI_TCH_VOCODER_CFG_CON:
        {
          char *ptr;
          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_TCH_VOCODER_CFG_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {
            ((T_TR_MMI_TCH_VOCODER_CFG_CON *)(ptr))->header       = TRL1_MMI_TCH_VOCODER_CFG_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_TCH_VOCODER_CFG_CON));
          }
        }
        break;
        case L1_VOCODER_CFG_ENABLE_CON:
        {
          char *ptr;
          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_VOCODER_CFG_ENABLE_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {
            ((T_TR_L1_VOCODER_CFG_ENABLE_CON *)(ptr))->header       = TRL1_L1_VOCODER_CFG_ENABLE_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_VOCODER_CFG_ENABLE_CON));
          }
        }
        break;
        case L1_VOCODER_CFG_DISABLE_CON:
        {
          char *ptr;
          if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_VOCODER_CFG_DISABLE_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
          {
            ((T_TR_L1_VOCODER_CFG_DISABLE_CON *)(ptr))->header       = TRL1_L1_VOCODER_CFG_DISABLE_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
            L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_VOCODER_CFG_DISABLE_CON));
          }
        }
        break;

#endif // L1_VOCODER_IF_CHANGE
    #endif  // AUDIO_TASK

      ///////////////////
      // OML1 messages //
      ///////////////////

      case OML1_CLOSE_TCH_LOOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_OML1_CLOSE_TCH_LOOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_OML1_CLOSE_TCH_LOOP_REQ *)(ptr))->header        = TRL1_OML1_CLOSE_TCH_LOOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_OML1_CLOSE_TCH_LOOP_REQ *)(ptr))->sub_channel   = ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->sub_channel;
          ((T_TR_OML1_CLOSE_TCH_LOOP_REQ *)(ptr))->frame_erasure = ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->frame_erasure;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_OML1_CLOSE_TCH_LOOP_REQ));
        }
      }
      break;

      case OML1_OPEN_TCH_LOOP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_OML1_OPEN_TCH_LOOP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_OML1_OPEN_TCH_LOOP_REQ *)(ptr))->header       = TRL1_OML1_OPEN_TCH_LOOP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_OML1_OPEN_TCH_LOOP_REQ));
        }
      }
      break;

      case OML1_START_DAI_TEST_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_OML1_START_DAI_TEST_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_OML1_START_DAI_TEST_REQ *)(ptr))->header        = TRL1_OML1_START_DAI_TEST_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_OML1_START_DAI_TEST_REQ *)(ptr))->tested_device = ((T_OML1_START_DAI_TEST_REQ *)(msg->SigP))->tested_device;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_OML1_START_DAI_TEST_REQ));
        }
      }
      break;

      case OML1_STOP_DAI_TEST_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_OML1_STOP_DAI_TEST_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_OML1_STOP_DAI_TEST_REQ *)(ptr))->header       = TRL1_OML1_STOP_DAI_TEST_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_OML1_STOP_DAI_TEST_REQ));
        }
      }
      break;

      ///////////////////
      // Test messages //
      ///////////////////

      case TST_TEST_HW_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_TST_TEST_HW_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_TST_TEST_HW_REQ *)(ptr))->header       = TRL1_TST_TEST_HW_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_TST_TEST_HW_REQ));
        }
      }
      break;

      case L1_TEST_HW_INFO:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_L1_TEST_HW_INFO), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_L1_TEST_HW_INFO *)(ptr))->header            = TRL1_L1_TEST_HW_INFO | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->dsp_code_version  = ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_code_version;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->dsp_checksum      = ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_checksum;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->dsp_patch_version = ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_patch_version;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->mcu_alr_version   = ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_alr_version;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->mcu_tm_version    = ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tm_version;
        #if L1_GPRS
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->mcu_gprs_version  = ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_gprs_version;
        #else
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->mcu_gprs_version  = 0;
        #endif
        #if 0 //((((CHIPSET !=2 )) && ((LONG_JUMP != 0))) || (CHIPSET == 12) || (CHIPSET == 15))
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->d_checksum1       = d_checksum1;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->d_checksum2       = d_checksum2;
        #else
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->d_checksum1       = 0;
          ((T_TR_L1_TEST_HW_INFO *)(ptr))->d_checksum2       = 0;
        #endif
          L1_send_trace_no_cpy(ptr,sizeof(T_TR_L1_TEST_HW_INFO));
        }
      }
      break;

      case TST_SLEEP_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_TST_SLEEP_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_TST_SLEEP_REQ *)(ptr))->header       = TRL1_TST_SLEEP_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_TST_SLEEP_REQ *)(ptr))->sleep_mode   = ((T_TST_SLEEP_REQ*)(msg->SigP))->sleep_mode;
          ((T_TR_TST_SLEEP_REQ *)(ptr))->clocks       = ((T_TST_SLEEP_REQ*)(msg->SigP))->clocks;

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_TST_SLEEP_REQ));
        }
      }
      break;

      /////////
      // ADC //
      /////////

      case MMI_ADC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_ADC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MMI_ADC_REQ *)(ptr))->header       = TRL1_MMI_ADC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_ADC_REQ));
        }
      }
      break;

      case MMI_STOP_ADC_REQ:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_STOP_ADC_REQ), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MMI_STOP_ADC_REQ *)(ptr))->header       = TRL1_MMI_STOP_ADC_REQ | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_STOP_ADC_REQ));
        }
      }
      break;

      case MMI_STOP_ADC_CON:
      {
        char *ptr;

        if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_MMI_STOP_ADC_CON), (T_RVT_BUFFER *) &ptr) == RVT_OK)
        {

          ((T_TR_MMI_STOP_ADC_CON *)(ptr))->header       = TRL1_MMI_STOP_ADC_CON | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

          L1_send_trace_no_cpy(ptr,sizeof(T_TR_MMI_STOP_ADC_CON));
        }
      }
      break;

    } // ...End of switch
  } // End if L1A message trace enabled
}

#if L1_RECOVERY

/*********************************/
/* Trace in case of system crash */
/*********************************/

void l1_trace_recovery(void)
{
  char *ptr;

  if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TR_RECOVERY), (T_RVT_BUFFER *) &ptr) == RVT_OK)
  {

    ((T_TR_RECOVERY *)(ptr))->header       = TRL1_RECOVERY | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

    L1_send_trace_no_cpy(ptr,sizeof(T_TR_RECOVERY));
  }
}
#endif

/*************************************************************************/
/* L1 Dynamic traces                                                     */
/*************************************************************************/

/* WARNING : Following functions are called by L1S */
/***************************************************/

#if (DSP_DEBUG_TRACE_ENABLE == 1)
  #define DSP_DEBUG_ENABLE \
          if (trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] == 0) \
          { \
            trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] = l1s_dsp_com.dsp_db2_current_r_ptr->d_debug_ptr_begin; \
            trace_info.dsp_debug_fn[l1s_dsp_com.dsp_r_page]        = l1s.actual_time.fn;                                   \
            trace_info.dsp_debug_time[l1s_dsp_com.dsp_r_page]      = (UWORD16)l1s.debug_time;                              \
          }
#endif

//////////////////////
// L1S Debug Traces //
//////////////////////

/*-------------------------------------------------------*/
/* Trace_L1s_Abort()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_L1s_Abort(UWORD8 task)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TR_L1S_ABORT));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TR_L1S_ABORT *)(msg->SigP))->header        = TRL1_L1S_ABORT | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
  ((T_TR_L1S_ABORT *)(msg->SigP))->tpu_offset    = l1s.tpu_offset;
  ((T_TR_L1S_ABORT *)(msg->SigP))->tpu_offset_hw = l1s.tpu_offset_hw;
  ((T_TR_L1S_ABORT *)(msg->SigP))->d_debug       = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;
  ((T_TR_L1S_ABORT *)(msg->SigP))->debug_time    = (UWORD16) l1s.debug_time & 0xffff;
  ((T_TR_L1S_ABORT *)(msg->SigP))->adc_mode      = l1a_l1s_com.adc_mode; // ADC enabled
  ((T_TR_L1S_ABORT *)(msg->SigP))->task          = task;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)

}

/*-------------------------------------------------------*/
/* Trace_MCU_DSP_Com_Mismatch()                          */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_MCU_DSP_Com_Mismatch(UWORD8 task)
{
  if (trace_info.current_config->l1_dyn_trace &  1 << L1_DYN_TRACE_L1S_DEBUG)
  {
    xSignalHeaderRec *msg;

    if((l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff ) != (l1s.debug_time & 0xffff ))
    // Debug number is different than the one expected...
    {
      if(!trace_info.DSP_misaligned)
      // MCU/DSP com. is misaligned.
      {
        #if (DSP_DEBUG_TRACE_ENABLE == 1)
          if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
          {
            // Flag DSP error for DSP trace and memorize address of start of DSP trace
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif
            DSP_DEBUG_ENABLE
          }
        #endif

        RTTL1_EVENT(RTTL1_EVENT_ERROR, RTTL1_EVENT_SIZE_ERROR)

        trace_info.DSP_misaligned = TRUE;
      }
      else
        return;
    }
    else
    {
      if(trace_info.DSP_misaligned)
      // MCU/DSP com. is now realigned.
      {
        trace_info.DSP_misaligned = FALSE;
      }
      else
        return;
    }

    // Allocate DEBUG message.
    msg = os_alloc_sig(sizeof(T_TR_MCU_DSP_MISMATCH));
    DEBUGMSG(status,NU_ALLOC_ERR)
    msg->SignalCode = TRACE_INFO;

    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->header        = TRL1_MCU_DSP_MISMATCH | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->tpu_offset    = l1s.tpu_offset;
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->tpu_offset_hw = l1s.tpu_offset_hw;
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->d_debug       = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->debug_time    = (UWORD16) l1s.debug_time & 0xffff;
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->adc_mode      = l1a_l1s_com.adc_mode; // ADC enabled
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->task          = task;
    ((T_TR_MCU_DSP_MISMATCH *)(msg->SigP))->error         = trace_info.DSP_misaligned;

    // send message...
    os_send_sig(msg, L1C1_QUEUE);
    DEBUGMSG(status,NU_SEND_QUEUE_ERR)
  }
}

/*-------------------------------------------------------*/
/* Trace_PM_Equal_0()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_PM_Equal_0(UWORD32 pm, UWORD8 task)
{
  if(pm==0) // PM error in the frame
  {
    if (trace_info.PM_Task == 255) // 1st PM error in the frame: This PM is memorized
      trace_info.PM_Task  = task;  // memorize the Task of this 1st PM error

    #if ( ((TRACE_TYPE==1) || (TRACE_TYPE == 4)))
     if (trace_info.current_config->l1_dyn_trace & 1 <<  L1_DSP_TRACE_FULL_DUMP)
     {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
 	     // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
       if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif
         l1_trace_full_dsp_buffer();  // trace DSP trace buffer in case a PM error occurs
      }                             // used only for debug mode,
    #endif

  }
  else // no error in the frame :is it a PM recovery ?
  {
    if (trace_info.Not_PM_Task == 255) // 1st PM recovery case: task of recovery needs to be memorized
      trace_info.Not_PM_Task = task;
  }
}

/*-------------------------------------------------------*/
/* Trace_PM_Equal_0_balance()                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_PM_Equal_0_balance(void)
{
  // Here below we handle the case where we have lot of PM occuring during next frames
  // The PM traces are filtered in order to trace:
  //   => the first PM
  //   => the latest PM when we have no more PM
  BOOL trace_pm = FALSE;


  if(trace_info.PM_Task != 255) // at least one PM occured in the current frame
  {
    if(!trace_info.PM_equal_0)  // We are not in a phase of PM: We trace only the 1st PM
    {
      if (trace_info.current_config->l1_dyn_trace & 1 <<  L1_DYN_TRACE_L1S_DEBUG)
      {
        trace_pm              = TRUE;

        #if (DSP_DEBUG_TRACE_ENABLE == 1)
          if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
          // Flag DSP error for DSP trace and memorize address of start of DSP trace
          {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif

            DSP_DEBUG_ENABLE
          }
        #endif

        RTTL1_EVENT(RTTL1_EVENT_ERROR, RTTL1_EVENT_SIZE_ERROR)
      }

      trace_info.PM_equal_0 = TRUE;  // We enter in a phase of a lot of PM
    }
  }
  else // no PM in the current frame
  {
    if(trace_info.PM_equal_0) // this is the end of the PM phase: we trace the latest PM
    {
      if (trace_info.current_config->l1_dyn_trace & 1 << L1_DYN_TRACE_L1S_DEBUG)
      {
        trace_pm              = TRUE;
      }

      trace_info.PM_equal_0 = FALSE;
    }
  }

  if (trace_pm)
  {
    xSignalHeaderRec *msg;

    // Allocate DEBUG message.
    msg = os_alloc_sig(sizeof(T_TR_PM_EQUAL_0));
    DEBUGMSG(status,NU_ALLOC_ERR)
    msg->SignalCode = TRACE_INFO;

    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->header        = TRL1_PM_EQUAL_0 | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->tpu_offset    = l1s.tpu_offset;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->tpu_offset_hw = l1s.tpu_offset_hw;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->d_debug       = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->debug_time    = (UWORD16) l1s.debug_time & 0xffff;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->adc_mode      = l1a_l1s_com.adc_mode; // ADC enabled
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->task          = trace_info.PM_Task;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->no_pm_task    = trace_info.Not_PM_Task;
    ((T_TR_PM_EQUAL_0 *)(msg->SigP))->error         = trace_info.PM_equal_0;

    // send message...
    os_send_sig(msg, L1C1_QUEUE);
    DEBUGMSG(status,NU_SEND_QUEUE_ERR)
  }

  trace_info.PM_Task     = 255;
  trace_info.Not_PM_Task = 255;
}

/*-------------------------------------------------------*/
/* l1_trace_IT_DSP_error()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_IT_DSP_error(UWORD8 cause)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
     {
       xSignalHeaderRec *msg;

       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TR_IT_DSP_ERROR));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TR_IT_DSP_ERROR *)(msg->SigP))->header = TRL1_IT_DSP_ERROR | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}

///////////////////////
// P.Transfer traces //
///////////////////////

#if L1_GPRS

/*-------------------------------------------------------*/
/* Trace_dl_ptcch()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void  Trace_dl_ptcch(UWORD8  ordered_ta,
                     UWORD8  crc)   // Current TX allocation for Polling
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TR_DL_PTCCH));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TR_DL_PTCCH *)(msg->SigP))->header     = TRL1_DL_PTCCH | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
  ((T_TR_DL_PTCCH *)(msg->SigP))->crc_error  = crc;
  ((T_TR_DL_PTCCH *)(msg->SigP))->ordered_ta = ordered_ta;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)

} // End Trace_ptcch_error

/*-------------------------------------------------------*/
/* Trace_rlc_ul_param()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_rlc_ul_param(UWORD8  assignment_id,
                        UWORD8  tx_no,
                        UWORD32 fn,
                        UWORD8  ta,
                        UWORD32 a_pu_gprs,
                        UWORD32 a_du_gprs,
                        BOOL    fix_alloc_exhaust)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TR_RLC_UL_PARAM));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->header            = TRL1_RLC_UL_PARAM | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->a_pu_gprs         = a_pu_gprs;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->a_du_gprs         = a_du_gprs;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->assignment_id     = assignment_id;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->tx_no             = tx_no;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->fn_param          = fn;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->ta                = ta;
  ((T_TR_RLC_UL_PARAM *)(msg->SigP))->fix_alloc_exhaust = fix_alloc_exhaust;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* Trace_rlc_dl_param()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_rlc_dl_param( UWORD8  assignment_id,
                         UWORD32 fn,
                         UWORD32 d_rlcmac_rx_no_gprs,
                         UWORD8  rx_no,
                         UWORD8  rlc_blocks_sent,
                         UWORD8  last_poll_response)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TR_RLC_DL_PARAM));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;


  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->header              = TRL1_RLC_DL_PARAM | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->fn_param            = fn;
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->d_rlcmac_rx_no_gprs = d_rlcmac_rx_no_gprs;
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->assignment_id       = assignment_id;
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->rx_no               = rx_no;
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->rlc_blocks_sent     = rlc_blocks_sent;
  ((T_TR_RLC_DL_PARAM *)(msg->SigP))->last_poll_response  = last_poll_response;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* Trace_uplink_no_TA()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_uplink_no_TA()
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TR_FORBIDDEN_UPLINK));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TR_FORBIDDEN_UPLINK *)(msg->SigP))->header         = TRL1_FORBIDDEN_UPLINK | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* l1_trace_ptcch_disable()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Trace the gauging is running                          */
/*-------------------------------------------------------*/
void l1_trace_ptcch_disable(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

    xSignalHeaderRec *msg;

    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
    {
      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TR_PTCCH_DISABLE));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TR_TOA_NOT_UPDATED *)(msg->SigP))->header    = TRL1_PTCCH_DISABLE | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);

      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
  #endif
}

/*-------------------------------------------------------*/
/* Trace_pdtch()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void  Trace_condensed_pdtch(UWORD8 rx_allocation, UWORD8 tx_allocation)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_CONDENSED_PDTCH_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_CONDENSED_PDTCH;

  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->fn            = l1s.actual_time.fn;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->rx_allocation = rx_allocation;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->tx_allocation = tx_allocation;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->blk_status    = trace_info.pdtch_trace.blk_status;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_cs_type    = trace_info.pdtch_trace.dl_cs_type;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[0]  = trace_info.pdtch_trace.dl_status[0];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[1]  = trace_info.pdtch_trace.dl_status[1];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[2]  = trace_info.pdtch_trace.dl_status[2];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[3]  = trace_info.pdtch_trace.dl_status[3];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[0]  = trace_info.pdtch_trace.ul_status[0];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[1]  = trace_info.pdtch_trace.ul_status[1];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[2]  = trace_info.pdtch_trace.ul_status[2];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[3]  = trace_info.pdtch_trace.ul_status[3];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[4]  = trace_info.pdtch_trace.ul_status[4];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[5]  = trace_info.pdtch_trace.ul_status[5];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[6]  = trace_info.pdtch_trace.ul_status[6];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[7]  = trace_info.pdtch_trace.ul_status[7];

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* Quick_Trace()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Description: This function can be used to quickly add */
/*              a trace                                  */
/*              NOT TO USE FOR PERMANENT TRACES !!!      */
/*-------------------------------------------------------*/
void  Quick_Trace(UWORD8   debug_code,
                  UWORD32  param0,
                  UWORD32  param1,
                  UWORD32  param2,
                  UWORD32  param3,
                  UWORD32  param4,
                  UWORD32  param5,
                  UWORD32  param6)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_QUICK_TRACE));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = QUICK_TRACE;

  ((T_QUICK_TRACE *)(msg->SigP))->debug_code = debug_code;
  ((T_QUICK_TRACE *)(msg->SigP))->fn         = l1s.actual_time.fn;

  ((T_QUICK_TRACE *)(msg->SigP))->tab[0] = param0;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[1] = param1;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[2] = param2;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[3] = param3;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[4] = param4;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[5] = param5;
  ((T_QUICK_TRACE *)(msg->SigP))->tab[6] = param6;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#endif // L1_GPRS

///////////////////////
// DSP error traces  //
///////////////////////

#if (D_ERROR_STATUS_TRACE_ENABLE)

/*-------------------------------------------------------*/
/* Trace_d_error_status()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_d_error_status()
{
  #if L1_GPRS
    UWORD16           d_error_status_masked =
          (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
          (trace_info.d_error_status_masks[l1a_l1s_com.dsp_scheduler_mode - 1]); // depends on the scheduler mode
  #else
    UWORD16           d_error_status_masked =
          (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
          (trace_info.d_error_status_masks[GSM_SCHEDULER - 1]);
  #endif
  UWORD16 changed_bits = d_error_status_masked ^ trace_info.d_error_status_old;

  // trace in case of change of status (field is reseted on change of scheduler)
  if (changed_bits)
  {
    xSignalHeaderRec *msg;

    // Allocate DEBUG message.
    msg = os_alloc_sig(sizeof(T_TR_D_ERROR_STATUS));
    DEBUGMSG(status,NU_ALLOC_ERR)
    msg->SignalCode = TRACE_INFO;

    ((T_TR_D_ERROR_STATUS *)(msg->SigP))->header         = TRL1_D_ERROR_STATUS | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
    ((T_TR_D_ERROR_STATUS *)(msg->SigP))->debug_time     = (UWORD16)l1s.debug_time;
    ((T_TR_D_ERROR_STATUS *)(msg->SigP))->d_error_status = d_error_status_masked;
    ((T_TR_D_ERROR_STATUS *)(msg->SigP))->d_debug        = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;

    // send message...
    os_send_sig(msg, L1C1_QUEUE);
    DEBUGMSG(status,NU_SEND_QUEUE_ERR)

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
      {
        // DSP debug buffer trace only if an ERROR is detected (not for a End of error detection)
        if ((changed_bits & d_error_status_masked) & ~trace_info.d_error_status_old)
        {
          // Flag DSP error for DSP trace and memorize address of start of DSP trace

#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD)
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif

          DSP_DEBUG_ENABLE
        }
      }
    #endif

    RTTL1_EVENT(RTTL1_EVENT_ERROR, RTTL1_EVENT_SIZE_ERROR)

    trace_info.d_error_status_old = d_error_status_masked;
  }

  // Clear bits that have been set by the DSP
  l1s_dsp_com.dsp_ndb_ptr->d_error_status &= ~d_error_status_masked;
}

#endif // (D_ERROR_STATUS_TRACE_ENABLE)

#if (DSP_DEBUG_TRACE_ENABLE == 1)

/*-------------------------------------------------------*/
/* Trace_dsp_debug()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_dsp_debug()
{
  // WARNING: l1s_dsp_com.dsp_r_page changed in l1s_end_manager() but DSP DB pointers haven't been
  // updated !!!
  UWORD32           start_address = trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page];
  UWORD32           end_address, stop_address;
  UWORD16           size;
  API              *i;
  UWORD8            j = 0;
  xSignalHeaderRec *msg;

  // DSP DEBUG trace only works when GSM activity is enabled
  if (l1s_dsp_com.dsp_r_page_used == FALSE)
  {
    trace_info.dsp_debug_buf_start[0] = trace_info.dsp_debug_buf_start[1] = 0;
  }

  // If a DSP error occured...
  if (start_address)
  {
    WORD32 diff = l1s.debug_time - trace_info.fn_last_dsp_debug;

    if (diff < 0) diff += 0xFFFFFFFF;

    if (diff >= 104)
    {

      // Take the DB_R pointers on the start/end of last TDMA trace
      start_address = 0xFFD00000 + (start_address - 0x800) * 2;
      end_address   = 0xFFD00000 + (l1s_dsp_com.dsp_db2_other_r_ptr->d_debug_ptr_end - 0x800) * 2;

      // Process size of block
      if (end_address >= start_address)
      {
        size = end_address - start_address;
        stop_address = end_address;
      }
      else
      {
        size = end_address - start_address + C_DEBUG_BUFFER_SIZE * 2;
        stop_address = (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2);
      }

      if ((size > 0) && (size < 1000) && (size < C_DEBUG_BUFFER_SIZE))
      {
        // Allocate memory pool
        msg = os_alloc_sig(size + sizeof(T_DSP_DEBUG_INFO) - 2*sizeof(API));
        DEBUGMSG(status,NU_ALLOC_ERR)

        msg->SignalCode                                  = TRACE_DSP_DEBUG;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->size          = size;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->fn            = trace_info.dsp_debug_fn[l1s_dsp_com.dsp_r_page];
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->debug_time    = trace_info.dsp_debug_time[l1s_dsp_com.dsp_r_page];
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->patch_version = l1s_dsp_com.dsp_ndb_ptr->d_version_number2;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->trace_level   = l1s_dsp_com.dsp_ndb_ptr->d_debug_trace_type;

        // Copy data into message
        for (i = (API*)start_address; i < (API*)stop_address; i++)
        {
          ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
        }

        // Circular buffer management
        if (i != (API*)end_address)
        {
          for (i = (API*) (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 - 0x800)*2); i < (API*)end_address; i++)
          {
            ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
          }
        }

        // Send sig to L1A
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

        // Set FN to avoid another DSP debug trace in the next 104 frames.
        trace_info.fn_last_dsp_debug = l1s.debug_time;
      }
    } // Enf if diff >= 104

    // Clear flag
    trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] = 0;
  } // End if "DSP error occured"
}

#if (AMR == 1)

/*-------------------------------------------------------*/
/* Trace_dsp_amr_debug()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_dsp_amr_debug()
{
  UWORD32           start_address;
  UWORD32           end_address, stop_address;
  UWORD16           size;
  API              *i;
  UWORD8            j = 0;
  xSignalHeaderRec *msg;

  // Start address of the AMR trace in the DSP trace buffer
  start_address = l1s_dsp_com.dsp_ndb_ptr->xxx;

  // Clear the pointer
  l1s_dsp_com.dsp_ndb_ptr->xxx = 0;

  // If start address different of 0 -> trace to be performed
  if (start_address != 0)
  {
    // Process MCU start address
    start_address = 0xFFD00000 + (start_address - 0x800) * 2;

    // Check ID and read size
    if ((((API*)start_adress & 0xFE00) >> 9) == C_AMR_TRACE_ID)
    {
      // Read size
      size = ((((API*)start_address) & 0x1FF) * 2);
      start_address += sizeof(API); // Do not dump header

      // Process stop address
      end_address = start_address + size;

      // Circular buffer...
      if (end_address <= (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2))
      {
        stop_address = end_address;
      }
      else
      {
        stop_address = (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2);
        end_address -= C_DEBUG_BUFFER_SIZE * 2;
      }

      // Create L1S->L1A message and dump buffer

      // Allocate memory pool
      msg = os_alloc_sig(size+sizeof(T_DSP_AMR_DEBUG_INFO)-2*sizeof(API));
      DEBUGMSG(status,NU_ALLOC_ERR)

      msg->SignalCode                                = TRACE_DSP_AMR_DEBUG;
      ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->size    = size;
      ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->fn      = l1s.actual_time.fn;

      // Copy data into message
      for (i = (API*)start_address; i < (API*)stop_address; i++)
      {
        ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
      }

      // Circular buffer management
      if (i != (API*)end_address)
      {
        for (i = (API*) (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 - 0x800)*2); i < (API*)end_address; i++)
        {
          ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
        }
      }

      // Send sig to L1A
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
  }
}

#endif // #if (AMR == 1)

#endif // #if (DSP_DEBUG_TRACE_ENABLE)

///////////////////////////
// Trace type 1 CPU load //
///////////////////////////

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && (CODE_VERSION != SIMULATION)

/*-------------------------------------------------------*/
/* L1S_CPU_load_process()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_L1S_CPU_load()
{
  #define TIMER_RESET_VALUE (0xFFFF)
  #define TICKS_PER_TDMA    (1875)

  unsigned long cpu;

  //Dtimer2_Start(0);
  layer_1_sync_end_time = TIMER_RESET_VALUE - Dtimer2_ReadValue();

  // Trace
  cpu = (100 * layer_1_sync_end_time) / TICKS_PER_TDMA;

  if (cpu > max_cpu)
  {
    max_cpu=cpu;
    fn_max_cpu=l1s.actual_time.fn;
    max_cpu_flag = 1;
  }

  if (((l1s.actual_time.fn%1326) == 0) && (max_cpu_flag == 0))
    max_cpu = 0;
} /* end of Trace_L1S_CPU_load() */

/********** DSP CPU load measurement *************/
#if (DSP >= 38)
void l1_dsp_cpu_load_read()
{

  #define DSP_TIMER_PRESCALER_VALUE (9)

  T_DB_MCU_TO_DSP_CPU_LOAD *mcu_dsp_cpu_load_r_ptr;   // DSP CPU load measurement
  UWORD32     dsp_fgd_tsk_cycles = 0L;
  UWORD16     dsp_tdma_fn;
  UWORD16     d_dsp_page_read;
  UWORD32     d_dsp_work_period;
  UWORD32     d_dsp_fgd_tsk_cycles_per_tdma;
  UWORD16     d_tdma_fnmod4;
  UWORD16     d_tdma_fnmod13;

  // **** 1. Read the DSP FGD task cycles from API ****

  // Check if DSP CPU load has been written in first buffer
  mcu_dsp_cpu_load_r_ptr = (T_DB_MCU_TO_DSP_CPU_LOAD *)DSP_CPU_LOAD_DB_W_PAGE_0;
  if ((API)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 & 0x8000)
  {
     mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 &= ~(0x8000); // reset the bit
     dsp_fgd_tsk_cycles = (UWORD32)(((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 << 16)
                                   + ((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim1));
     dsp_fgd_tsk_cycles = (dsp_fgd_tsk_cycles * DSP_TIMER_PRESCALER_VALUE);

     dsp_tdma_fn        = (API)mcu_dsp_cpu_load_r_ptr->d_tdma_dsp_fn;

     d_dsp_page_read    = 0;
  }
  else
  {
    // Check if DSP CPU load has been written in second buffer
    mcu_dsp_cpu_load_r_ptr = (T_DB_MCU_TO_DSP_CPU_LOAD *)DSP_CPU_LOAD_DB_W_PAGE_1;
    if ((API)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 & 0x8000)
    {
       mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 &= ~(0x8000); // reset the bit
       dsp_fgd_tsk_cycles = (UWORD32)(((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 << 16)
                                   + ((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim1));
       dsp_fgd_tsk_cycles = (dsp_fgd_tsk_cycles * DSP_TIMER_PRESCALER_VALUE);

       dsp_tdma_fn        = (API)mcu_dsp_cpu_load_r_ptr->d_tdma_dsp_fn;

       d_dsp_page_read    = 1;

    }
  }

  // **** 2. Get the number of DSP cycles per TDMA (based on DSP work period) ****
  if (dsp_fgd_tsk_cycles != 0L)
  {
    /* Take care of TDMA FN overflow */
    d_dsp_work_period = (l1s.actual_time.fn_mod42432 - dsp_tdma_fn - 2 + 42432) % 42432;

    d_dsp_fgd_tsk_cycles_per_tdma = dsp_fgd_tsk_cycles/(d_dsp_work_period + 1); // to avoid divide by 0, just in case

    // **** 3. For DSP work-period, update max cycles count ****
    d_tdma_fnmod13 = (l1s.actual_time.fn_mod13 - 1 + 13) % 13;
    d_tdma_fnmod4 = (l1s.actual_time.fn_mod13_mod4 - 1 + 4) % 4;

    if (d_tdma_fnmod13 == 12) //Idle/SACCH/PTCCH frames
    {
      if (dsp_max_cpu_load_idle_frame <= d_dsp_fgd_tsk_cycles_per_tdma)
        dsp_max_cpu_load_idle_frame = d_dsp_fgd_tsk_cycles_per_tdma;
    }
    else // for TDMA frames 0/1/2/3 (mod 4)
    {
      if (dsp_max_cpu_load_trace_array[d_tdma_fnmod4] <= d_dsp_fgd_tsk_cycles_per_tdma)
        dsp_max_cpu_load_trace_array[d_tdma_fnmod4] = d_dsp_fgd_tsk_cycles_per_tdma;
    }

    // **** 4. If 104 TDMA frames have elapsed, print out the DSP CPU cycles ****
    if ((l1s.actual_time.fn_mod42432 % 104) == 0)
    {
      l1_dsp_cpu_load_trace_flag = 1;
    }
  }
} /* end of l1_dsp_cpu_load_read() */
#endif	// DSP >= 38

#endif

///////////////////////////
// Additional L1S traces //
///////////////////////////

/*-------------------------------------------------------*/
/* l1_trace_ADC()                                        */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_ADC(UWORD8 type)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TR_ADC));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TR_ADC *)(msg->SigP))->header   = TRL1_ADC | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
       ((T_TR_ADC *)(msg->SigP))->type     = type;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}

/*-------------------------------------------------------*/
/* l1_trace_burst_param()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_burst_param (UWORD32 angle, UWORD32 snr, UWORD32 afc, UWORD32 task,
                           UWORD32 pm, UWORD32 toa_val, UWORD32 IL_for_rxlev)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_BURST_PARAM)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TR_BURST_PARAM));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TR_BURST_PARAM *)(msg->SigP))->header  = TRL1_BURST_PARAM | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
       ((T_TR_BURST_PARAM *)(msg->SigP))->angle   = angle;
       ((T_TR_BURST_PARAM *)(msg->SigP))->snr     = snr;
       ((T_TR_BURST_PARAM *)(msg->SigP))->afc     = afc;
       ((T_TR_BURST_PARAM *)(msg->SigP))->task    = task;
       ((T_TR_BURST_PARAM *)(msg->SigP))->pm      = pm;
       ((T_TR_BURST_PARAM *)(msg->SigP))->toa     = toa_val;
       ((T_TR_BURST_PARAM *)(msg->SigP))->input_level = IL_for_rxlev;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}


/*-------------------------------------------------------*/
/* l1_trace_saic()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_saic(UWORD32 SWH_flag, UWORD32 SAIC_flag)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_SAIC_DEBUG)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TR_SAIC_DEBUG));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TR_SAIC_DEBUG *)(msg->SigP))->header    = TRL1_SAIC_DEBUG | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
       ((T_TR_SAIC_DEBUG *)(msg->SigP))->SWH_flag  = SWH_flag;
       ((T_TR_SAIC_DEBUG *)(msg->SigP))->SAIC_flag  = SAIC_flag;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}


/*-------------------------------------------------------*/
/* l1_trace_new_toa()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_new_toa(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TR_NEW_TOA));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TR_NEW_TOA *)(msg->SigP))->header    = TRL1_NEW_TOA | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
       #if (TOA_ALGO == 2)
        ((T_TR_NEW_TOA *)(msg->SigP))->toa_shift = l1s.toa_var.toa_shift;
       #else
        ((T_TR_NEW_TOA *)(msg->SigP))->toa_shift = l1s.toa_shift;
       #endif

       #if(TOA_ALGO == 2)
         ((T_TR_NEW_TOA *)(msg->SigP))->toa_frames_counter  = trace_info.toa_trace_var.toa_frames_counter;
         ((T_TR_NEW_TOA *)(msg->SigP))->toa_accumul_counter = trace_info.toa_trace_var.toa_accumul_counter;
         ((T_TR_NEW_TOA *)(msg->SigP))->toa_accumul_value   = trace_info.toa_trace_var.toa_accumul_value;
       #endif

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}

/*-------------------------------------------------------*/
/* l1_trace_toa_not_updated()                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* TOA ERROR: TOA not updated                            */
/*-------------------------------------------------------*/
void l1_trace_toa_not_updated(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

    // !!!!!!!!!!!!!!!!!!!!!!!!
    // !!! should not occur !!!
    // !!!!!!!!!!!!!!!!!!!!!!!!

    xSignalHeaderRec *msg;

    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
    {
      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TR_TOA_NOT_UPDATED));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TR_TOA_NOT_UPDATED *)(msg->SigP))->header    = TRL1_TOA_NOT_UPDATED | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
      #if (TOA_ALGO == 2)
       ((T_TR_TOA_NOT_UPDATED *)(msg->SigP))->toa_shift = l1s.toa_var.toa_shift;
      #else
       ((T_TR_TOA_NOT_UPDATED *)(msg->SigP))->toa_shift = l1s.toa_shift;
      #endif
      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
  #endif
}

////////////////////////////////////////
// Dynamic trace configuration change //
////////////////////////////////////////

/*-------------------------------------------------------*/
/* l1_send_trace_version()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_send_trace_version()
{
  #if (TRACE_TYPE == 1)
    char ptr[11];
    char nb_bytes_sent = 0;

    ptr[0]  = trace_info.l1_trace_user_id;
    ptr[1]  = TRACE_CHECK_RESULT_OPCODE & 0xff;
    ptr[2]  = (TRACE_CHECK_RESULT_OPCODE >>  8) & 0xff;
    ptr[3]  = (TRACE_CHECK_RESULT_OPCODE >> 16) & 0xff;
    ptr[4]  = (TRACE_CHECK_RESULT_OPCODE >> 24) & 0xff;
    ptr[5]  = sizeof(T_TRACE_CELLS) & 0xff;
    ptr[6]  = (sizeof(T_TRACE_CELLS) >>  8) & 0xff;
    ptr[7]  = (sizeof(T_TRACE_CELLS) >> 16) & 0xff;
    ptr[8]  = (sizeof(T_TRACE_CELLS) >> 24) & 0xff;
    ptr[9]  = L1_TRACE_VERSION & 0xff;
    ptr[10] = (L1_TRACE_VERSION >> 8) & 0xff;

    while( nb_bytes_sent < 11)
      nb_bytes_sent += SER_tr_WriteNBytes(SER_LAYER_1, (SYS_UWORD8 *)ptr + nb_bytes_sent, 11 - nb_bytes_sent );

  #elif (TRACE_TYPE == 4)
    char *ptr;

    if (rvt_mem_alloc(trace_info.l1_trace_user_id, sizeof(T_TRACE_VERSION), (T_RVT_BUFFER *) &ptr) == RVT_OK)
    {
      ((T_TRACE_VERSION *)ptr)->Opcode   = TRACE_CHECK_RESULT_OPCODE;
      ((T_TRACE_VERSION *)ptr)->checksum = sizeof(T_TRACE_CELLS);
      ((T_TRACE_VERSION *)ptr)->version  = L1_TRACE_VERSION;

      L1_send_trace_no_cpy(ptr,sizeof(T_TRACE_VERSION));
    }
  #endif
}

/*-------------------------------------------------------*/
/* l1_trace_configuration()                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_configuration(T_RVT_BUFFER trace_msg, UINT16 trace_msg_size)
{
    UWORD8 Opcode = trace_msg[0];

    switch (Opcode)
    {
      case TRACE_CONFIG_OPCODE:
      {
        // Send message to L1A
        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TRACE_CONFIG_CHANGE));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_CONFIG;

        // NOTE: trace_msg isnt necessary 32-bit aligned !!!

        // First UWORD32 is the classic L1 dynamic trace
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->trace_config = (trace_msg[1]) | (trace_msg[2] << 8) | (trace_msg[3] << 16) | (trace_msg[4] << 24);

        // The eight following UWORD32 define the RTT cell configuration
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[0] = (trace_msg[5]) | (trace_msg[6] << 8) | (trace_msg[7] << 16) | (trace_msg[8] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[1] = (trace_msg[9]) | (trace_msg[10] << 8) | (trace_msg[11] << 16) | (trace_msg[12] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[2] = (trace_msg[13]) | (trace_msg[14] << 8) | (trace_msg[15] << 16) | (trace_msg[16] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[3] = (trace_msg[17]) | (trace_msg[18] << 8) | (trace_msg[19] << 16) | (trace_msg[20] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[4] = (trace_msg[21]) | (trace_msg[22] << 8) | (trace_msg[23] << 16) | (trace_msg[24] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[5] = (trace_msg[25]) | (trace_msg[26] << 8) | (trace_msg[27] << 16) | (trace_msg[28] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[6] = (trace_msg[29]) | (trace_msg[30] << 8) | (trace_msg[31] << 16) | (trace_msg[32] << 24);
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_cell_enable[7] = (trace_msg[33]) | (trace_msg[34] << 8) | (trace_msg[35] << 16) | (trace_msg[36] << 24);

        // Last UWORD32 define the RTT event
        ((T_TRACE_CONFIG_CHANGE *)(msg->SigP))->rtt_event = (trace_msg[37]) | (trace_msg[38] << 8) | (trace_msg[39] << 16) | (trace_msg[40]);

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
      break;

      case TRACE_CHECKING_OPCODE:
      {
        l1_send_trace_version();
      }
      break;
    }
}

/************************************ ASCII trace *********************************************************/

#else // L1_BINARY_TRACE = 0

/*-------------------------------------------------------*/
/* l1_trace_L1_task()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*    Traces the status of the L1 tasks                  */
/*-------------------------------------------------------*/
void l1_trace_L1_tasks(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    UWORD8 i;
    char str2[NBR_DL_L1S_TASKS+5];

    // trace enabled task
    for(i=0;i<NBR_DL_L1S_TASKS;i++)
      str2[i]= '0'+ l1a_l1s_com.l1s_en_task[i];

    str2[NBR_DL_L1S_TASKS]  = '\n';
    str2[NBR_DL_L1S_TASKS+1]= '\r';
    str2[NBR_DL_L1S_TASKS+1]= 0;
    L1_send_trace_cpy(str2);
    L1_send_trace_cpy("\n\r");
  #endif
}

/*-------------------------------------------------------*/
/* l1_trace_ADC()                                        */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_ADC(UWORD8 type)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = TRACE_ADC;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = type;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}


/*-------------------------------------------------------*/
/* l1_trace_IT_DSP_error()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_IT_DSP_error(UWORD8 cause)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     trace_info.l1_memorize_error = '.'; // memorize an error in the L1

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = IT_DSP_ERROR;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = cause; // cause

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}


#if (L1_PCM_EXTRACTION)
/*-------------------------------------------------------*/
/* l1_trace_PCM_DSP_error()                               */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_PCM_DSP_error(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1S_PCM_ERROR_TRACE;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_error;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
     l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_error = 0;
} /* end function l1_trace_PCM_DSP_error */
#endif /* L1_PCM_EXTRACTION */

/*-------------------------------------------------------*/
/* l1_trace_burst_param()                                */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_burst_param (UWORD32 angle, UWORD32 snr, UWORD32 afc, UWORD32 task,
                           UWORD32 pm, UWORD32 toa_val, UWORD32 IL_for_rxlev)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_BURST_PARAM)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = BURST_PARAM;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0]  = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1]  = angle;
       ((T_TRACE_INFO *)(msg->SigP))->tab[2]  = snr;
       ((T_TRACE_INFO *)(msg->SigP))->tab[3]  = afc;
       ((T_TRACE_INFO *)(msg->SigP))->tab[4]  = pm;
       ((T_TRACE_INFO *)(msg->SigP))->tab[5]  = toa_val;
       ((T_TRACE_INFO *)(msg->SigP))->tab[6]  = task;
       ((T_TRACE_INFO *)(msg->SigP))->tab[7]  = IL_for_rxlev;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }

  #endif


}


#if (L1_SAIC !=0)
/*-------------------------------------------------------*/
/* l1_trace_saic()                                       */
/*-------------------------------------------------------*/
/* Parameters : SWH_flag                                 */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_saic(UWORD32 SWH_flag, UWORD32 SAIC_flag)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_SAIC_DEBUG)
     {
		  if(trace_info.prev_swh_flag_val != SWH_flag || trace_info.prev_saic_flag_val != SAIC_flag)
		{
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = SAIC_DEBUG;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = SWH_flag;
       ((T_TRACE_INFO *)(msg->SigP))->tab[2] = SAIC_flag;


       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)

      trace_info.prev_saic_flag_val = SAIC_flag;
      trace_info.prev_swh_flag_val = SWH_flag;
		}
     }
  #endif
}
#endif

#if (L1_NAVC_TRACE == 1)
/*-------------------------------------------------------*/
/* l1_trace_navc()                                       */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/

void l1_trace_navc(UWORD32 status, UWORD32 energy_level)
{
#if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;


       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = NAVC_VALUE;
       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = status;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = energy_level;
       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)

#endif

}
#endif
/*-------------------------------------------------------*/
/* l1_trace_new_toa()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_new_toa(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = NEW_TOA;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       #if (TOA_ALGO == 2)
        ((T_TRACE_INFO *)(msg->SigP))->tab[1] = l1s.toa_var.toa_shift;
       #else
        ((T_TRACE_INFO *)(msg->SigP))->tab[1] = l1s.toa_shift;
       #endif
       #if (TOA_ALGO == 2)
         ((T_TRACE_INFO *)(msg->SigP))->tab[2] = trace_info.toa_trace_var.toa_frames_counter;
         ((T_TRACE_INFO *)(msg->SigP))->tab[3] = trace_info.toa_trace_var.toa_accumul_counter;
         ((T_TRACE_INFO *)(msg->SigP))->tab[4] = trace_info.toa_trace_var.toa_accumul_value;
      #endif

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}
/*-------------------------------------------------------*/
/* l1_trace_toa_not_updated()                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* TOA ERROR: TOA not updated                            */
/*-------------------------------------------------------*/
void l1_trace_toa_not_updated(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

    // !!!!!!!!!!!!!!!!!!!!!!!!
    // !!! should not occur !!!
    // !!!!!!!!!!!!!!!!!!!!!!!!

    xSignalHeaderRec *msg;

    trace_info.l1_memorize_error = '.'; // memorize an error in the L1

    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
    {
      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TRACE_INFO));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TRACE_INFO *)(msg->SigP))->debug_code = TOA_NOT_UPDATED;

      ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
      #if (TOA_ALGO == 2)
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = l1s.toa_var.toa_shift;
      #else
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = l1s.toa_shift;
      #endif
      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
  #endif
}

#if (L1_RF_KBD_FIX == 1)
/*-------------------------------------------------------*/
/* l1_trace_correction_ratio()                                    */
/*-------------------------------------------------------*/
/* Parameters :  correction_ratio                                        */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1_trace_correction_ratio(UWORD16 correction_ratio)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
	xSignalHeaderRec *msg;
	if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_RF_KBD)
    {

	if(trace_info.prev_correction_ratio != correction_ratio)
	{
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = KPD_CR;
       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = correction_ratio;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)

    	trace_info.prev_correction_ratio = correction_ratio;
	}
}
  #endif
}
#endif


#if (TRACE_TYPE==1) || (TRACE_TYPE==4)


/*************************************************************************/
/* L1A Messages traces                                                   */
/*************************************************************************/

/*-------------------------------------------------------*/
/* l1_trace_message()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Description: L1 Trace formatting.                     */
/*-------------------------------------------------------*/
void l1_trace_message(xSignalHeaderRec *msg)
{

  char      str[240]={0}; // Max 240 bytes with current RVT //omaps00090550
  char      str2[30];


  /***********************************************************************/
  /* Debug messages                                                      */
  /***********************************************************************/

  switch(msg->SignalCode)
  {

  #if (DSP_DEBUG_TRACE_ENABLE == 1)

    #define BIN_TO_HEX(i) i<10 ? '0' + i : 'a' + i - 10

    //////////////////////
    // DSP debug buffer //
    //////////////////////

    case TRACE_DSP_DEBUG:
    {
      volatile UWORD16  size  = ((T_DSP_DEBUG_INFO *)(msg->SigP))->size;
      UWORD16      index = 0;
      BOOL         first_buffer = TRUE;
      UWORD8       return_line = 0;
      char        *buf;

      while (size != 0)
      {
        if (rvt_mem_alloc(trace_info.l1_trace_user_id, 240, (T_RVT_BUFFER *) &buf) == RVT_OK)
        {
          char *str_ptr = buf;
          UWORD8   i = 3;

          if (first_buffer)
          {
             sprintf(buf,"DEBUG %d %ld %xh %xh\n\r",
                        ((T_DSP_DEBUG_INFO *)(msg->SigP))->fn % 42432,
                        ((T_DSP_DEBUG_INFO *)(msg->SigP))->debug_time,
                        ((T_DSP_DEBUG_INFO *)(msg->SigP))->patch_version,
                        ((T_DSP_DEBUG_INFO *)(msg->SigP))->trace_level);
             i = (strlen(buf) / 4) + 1;
             str_ptr = str_ptr + strlen(buf);
             first_buffer = FALSE;
          }

          // Copy less that 223 bytes (240 - 4 x '\n\r' - 9 bytes margin(footer)) => 55 API
          for (; (i < 55) && (size != 0); i++)
          {
            API    val = ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[index];

            *str_ptr++ = BIN_TO_HEX(((val & 0xF000) >> 12));
            *str_ptr++ = BIN_TO_HEX(((val & 0x0F00) >> 8));
            *str_ptr++ = BIN_TO_HEX(((val & 0x00F0) >> 4));
            *str_ptr++ = BIN_TO_HEX( (val & 0x000F));

            if (return_line++ == 12)
            {
              *str_ptr++ = '\n';
              *str_ptr++ = '\r';
              return_line = 0;
            }

            index++;
            size -= 2;
          }

          if (size == 0)
          {
            if (return_line != 0)
            {
              *str_ptr++ = '\n';
              *str_ptr++ = '\r';
            }
            *str_ptr++ = 'D';
            *str_ptr++ = 'E';
            *str_ptr++ = 'B';
            *str_ptr++ = 'U';
            *str_ptr++ = 'G';
            *str_ptr++ = '\n';
            *str_ptr++ = '\r';
          }
        #if (OP_L1_STANDALONE == 0)
          else
          {
            *str_ptr++ = '\r'; // 'Dummy' char for Riviera Tracer
          }
        #endif //(OP_L1_STANDALONE == 0)

          *str_ptr = 0; // End char
          L1_send_trace_no_cpy(buf);
        }
        else
        {
          // Error in trace message allocation --> ABORT trace !
          break;
        }
      }
    }
    break;

    ///////////////////
    // DSP AMR debug //
    ///////////////////

    case TRACE_DSP_AMR_DEBUG:
    {
      volatile UWORD16  size  = ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->size;
      UWORD16      index = 0;
      BOOL         first_buffer = TRUE;
      UWORD8       return_line = 0;
      char        *buf;

      while (size != 0)
      {
        if (rvt_mem_alloc(trace_info.l1_trace_user_id, 240, (T_RVT_BUFFER *) &buf) == RVT_OK)
        {
          char *str_ptr = buf;
          UWORD8   i = 3;

          if (first_buffer)
          {
             sprintf(buf,"AMR %ld\n\r",
                        ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->fn % 42432);
             i = (strlen(buf) / 4) + 1;
             str_ptr = str_ptr + strlen(buf);
             first_buffer = FALSE;
          }

          // Copy less that 245 bytes (255 - 5 x '\n\r' - 2 bytes margin(footer)) => 48 API words
          // 1 API <=> 4 digits <=> 4 bytes + 1 space
          for (; (i < 48) && (size != 0); i++)
          {
            API    val = ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->buffer[index];

            *str_ptr++ = BIN_TO_HEX(((val & 0xF000) >> 12));
            *str_ptr++ = BIN_TO_HEX(((val & 0x0F00) >> 8));
            *str_ptr++ = BIN_TO_HEX(((val & 0x00F0) >> 4));
            *str_ptr++ = BIN_TO_HEX( (val & 0x000F));
            *str_ptr++ = ' ';

            if (return_line++ == 12)
            {
              *str_ptr++ = '\n';
              *str_ptr++ = '\r';
              return_line = 0;
            }

            index++;
            size -= 2;
          }

          if (size == 0)
          {
            if (return_line != 0)
            {
              *str_ptr++ = '\n';
              *str_ptr++ = '\r';
            }
          }
        #if (OP_L1_STANDALONE == 0)
          else
          {
            *str_ptr++ = '\r'; // 'Dummy' char for Riviera Tracer
          }
        #endif //(OP_L1_STANDALONE == 0)

          *str_ptr = 0; // End char
          L1_send_trace_no_cpy(buf);
        }
        else
        {
          // Error in trace message allocation --> ABORT trace !
          break;
        }
      }
    }
    break;

  #endif // DSP_DEBUG_ENABLE

  #if (L1_GPRS)

    //////////////////////////
    // Packet transfer mode //
    //////////////////////////

    case TRACE_CONDENSED_PDTCH:
    {
        UWORD8 blk_id,i;
        BOOL   flag_tx = FALSE;

        // Process block ID
        blk_id = 3 * (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->fn / 13) + (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->fn % 13) / 4;

        // Create string
        sprintf(str,"%x",blk_id);

        i = 0;
        while(i<4) // limited to 4 Rx
        {
          if (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->rx_allocation & (0x80 >> i))
          {
            sprintf(str2,"%02x",((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[i]);
            strcat(str,str2);
          }
          i++;
        }

        strcat(str," ");

        for(i=0; i<8; i++)
        {
          if (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[i] != 0)
          {
            flag_tx = TRUE;
            sprintf(str2,"%02x",((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[i] | ((((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->blk_status & 0x01) << 8));
            strcat(str,str2);
          }
        }

        if ((flag_tx == FALSE) && (((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->blk_status & 0x01))
        {
          strcat(str,"08");
        }

        strcat(str,"\n\r");

        // Send trace
        L1_send_trace_cpy(str);
      }
      break;

    #endif // L1_GPRS

      ////////////////
      // Debug info //
      ////////////////

      case TRACE_INFO:
      {
        UWORD8 debug_code = ((T_TRACE_INFO *)(msg->SigP))->debug_code;

        switch(debug_code)
        {

          // General debug info
#if (L1_AUDIO_MCU_ONOFF == 1)
          case L1_AUDIO_UL_ONOFF_TRACE:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("AUL :%ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
            sprintf (str,"AUL :%ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;
          case L1_AUDIO_DL_ONOFF_TRACE:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("ADL :%ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
            sprintf (str,"ADL :%ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;
#endif
          case TRACE_ADC:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("      ADC :%ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
            sprintf (str,"      ADC :%ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;

#if (L1_PCM_EXTRACTION)
          case L1S_PCM_ERROR_TRACE:
          {
            sprintf (str,"      PCM Err :%ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);

          }
          break;
#endif /* L1_PCM_EXTRACTION */



          case NEW_TOA:
          {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            #if (TOA_ALGO == 2)
              vsi_o_event_ttrace("      TOA updated:%ld %ld %ld %ld %ld",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4]);
            #else
              vsi_o_event_ttrace("      TOA updated:%ld %ld",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            #endif

#else
            #if (TOA_ALGO == 2)
              sprintf (str,"      TOA updated:%ld %ld %ld %ld %ld\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4]);
            #else
              sprintf (str,"      TOA updated:%ld %ld\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            #endif
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case SAIC_DEBUG:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("     SWH:%ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2]);

#else
            sprintf (str,"	SWH : %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2]);
            L1_send_trace_cpy(str);
#endif
          }
          break;

	  case NAVC_VALUE:
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("     NAVC:%ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);

#else
            sprintf (str,"	NAVC: %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
	  break;
          case KPD_CR:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(" KPD CR UPADTED : %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);

#else
            sprintf (str," KPD CR UPADTED : %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;



          case BURST_PARAM:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(" BP:%ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
#else
            sprintf (str," BP:%ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case TOA_NOT_UPDATED:  // should not occur
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(">  ERROR: TOA not updated:%ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
            sprintf (str,">  ERROR: TOA not updated:%ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case IT_DSP_ERROR:
          {
            WORD8 cause = ((T_TRACE_INFO *)(msg->SigP))->tab[1];
            if (cause == IT_DSP_ERROR_CPU_OVERLOAD)
            {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(">  MCU CPU overload %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
#else
            sprintf (str,">  MCU CPU overload %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
#endif
            }
#if (FF_L1_FAST_DECODING == 1)
            else if (cause == IT_DSP_ERROR_FAST_DECODING)
            {
              sprintf (str,">  ERROR: Fast Decoding IT not received! %ld\n\r",
                           ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            }
            else if (cause == IT_DSP_ERROR_FAST_DECODING_UNEXP)
            {
              sprintf (str,">  ERROR: Fast Decoding IT received but not expected! %ld\n\r",
                           ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            }
#endif    /* end FF_L1_FAST_DECODING */
            else
            {
              sprintf (str,">  ERROR: Unknown IT_DSP_ERROR! %ld\n\r",
                           ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            }
            L1_send_trace_cpy(str);
          } /* end case IT_DSP_ERROR */
          break;

          case PM_EQUAL_0:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(">  PM %ld %ld %ld %ld %ld %ld %ld %d %ld %d",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff,
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 16);
#else
            sprintf (str,">  PM %ld %ld %ld %ld %ld %ld %ld %d %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff,
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 16);
            L1_send_trace_cpy(str);
#endif
            if (trace_info.trace_filter == FALSE)
            {
              l1_trace_L1_tasks();            // trace L1 tasks status
              l1_display_buffer_trace_fct();  // display buffer fct called
              trace_info.trace_filter = TRUE; // avoid too much traces displayed
            }
          }
          break;

          case NO_PM_EQUAL_0:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("> !PM %ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
#else
            sprintf (str,"> !PM %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case MCU_DSP_MISMATCH:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("> COM %ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
#else
            sprintf (str,"> COM %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
            L1_send_trace_cpy(str);
#endif

            if (trace_info.trace_filter == FALSE)
            {
              l1_trace_L1_tasks();            // trace L1 tasks status
              l1_display_buffer_trace_fct();  // display buffer fct called
              trace_info.trace_filter = TRUE; // avoid too much traces displayed
            }
          }
          break;

          case NO_MCU_DSP_MISMATCH:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(">!COM %ld %ld %ld %ld %ld %ld %ld %ld",
                   ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
#else
            sprintf (str,">!COM %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                   ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                   ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case L1S_ABORT:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("ABORT %ld %ld %ld %ld %ld %ld %ld %ld",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
#else
            sprintf (str,"ABORT %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] & 0xffff);
            L1_send_trace_cpy(str);
#endif
          }
          break;

#if (MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD)
          case DSP_TRACE_DISABLE:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("****DSP-TRACE = Disabled %ld ****",
                        l1s.actual_time.fn_mod42432);
#else
            sprintf (str,"\n\r****DSP-TRACE = Disabled %ld ****\n\r",
                        l1s.actual_time.fn_mod42432);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case DSP_TRACE_ENABLE:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("****DSP-TRACE = Enabled %ld ****",
                        l1s.actual_time.fn_mod42432);
#else
            sprintf (str,"\n\r****DSP-TRACE = Enabled %ld ****\n\r",
                        l1s.actual_time.fn_mod42432);
            L1_send_trace_cpy(str);
#endif
          }
          break;
#endif

          case PWMGT_FAIL_SLEEP:
          {
      #if (CODE_VERSION!= SIMULATION)
            /* Need to be aligned with defintions in l1_pwmgr.h */
            char *array_periph_string[]={"uart","usb","usim","i2c","lcd","cam","blght","madc","madcas","bci"};
            char *array_application_string[]={"BTstack"};
            char *array_osload_string[]={"osload","hwtimer","mintimegauging"};
            char **ptr = NULL;

            if (((T_TRACE_INFO *)(msg->SigP))->tab[1] != FAIL_SLEEP_L1SYNCH)
            {
              if (((T_TRACE_INFO *)(msg->SigP))->tab[1] == FAIL_SLEEP_OSTIMERGAUGE)
              {
                ptr = array_osload_string;
              }
              if ( (((T_TRACE_INFO *)(msg->SigP))->tab[1] == FAIL_SLEEP_PERIPH_CHECK) ||
                   (((T_TRACE_INFO *)(msg->SigP))->tab[1] == FAIL_SLEEP_PERIPH_SLEEP))
              {
                ptr = array_periph_string;
                if((((T_TRACE_INFO *)(msg->SigP))->tab[2] >= L1_PWMGR_APP_OFFSET)){
                  (((T_TRACE_INFO *)(msg->SigP))->tab[2]) = (((T_TRACE_INFO *)(msg->SigP))->tab[2]) - L1_PWMGR_APP_OFFSET;
                  ptr = array_application_string;
                }
              }
              if(ptr != NULL)
              {
              sprintf (str," FSL: %ld %d %s %d\n\r",
                  ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                  ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                  ptr[(((T_TRACE_INFO *)(msg->SigP))->tab[2])],
                  ((T_TRACE_INFO *)(msg->SigP))->tab[3]);
            }
            }
            else
            {
              /* tab[2] and tab[3] are invalid if called from L1S */
              sprintf (str," FSL: %ld L1S\n\r",
                  ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            }
            L1_send_trace_cpy(str);
      #endif //NO SIMULATION
          }
          break;
          case TRACE_SLEEP:
          {
            // trace special events -> usefulf for debug
            char *array_special_cases[]={"", "ASYNC_0", "SLEEP_0"};
            UWORD8 index = 0;
            if (((T_TRACE_INFO *)(msg->SigP))->tab[3] == WAKEUP_ASYNCHRONOUS_ULPD_0)
              index = 1;
            else if ( ((T_TRACE_INFO *)(msg->SigP))->tab[3] == WAKEUP_ASYNCHRONOUS_SLEEP_DURATION_0)
              index = 2;
            else
              index = 0;

            if (((T_TRACE_INFO *)(msg->SigP))->tab[2] == CLOCK_STOP)
            // deep sleep trace
            {
              sprintf (str,"      deep_sleep: %ld %ld %d %d %s\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                       array_special_cases[index]);
            }
            else
            // big sleep
            {
              char *array_string[]={"undefined","ligth on","uart","sim","gauging","sleep mode","DSP","BT","camera","??"};
              sprintf (str,"      big sleep: %ld %ld %d %d (cause:%s) %s\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                       array_string[((T_TRACE_INFO *)(msg->SigP))->tab[4]],
                       array_special_cases[index]);
            }
            L1_send_trace_cpy(str);
          }
          break;

          case TRACE_GAUGING:
          {
            if (trace_info.reset_gauging_algo == TRUE)
            {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
              vsi_o_event_ttrace("      reset gauging algo");
#else
              sprintf (str,"      reset gauging algo\n\r");
              L1_send_trace_cpy(str);
#endif
              trace_info.reset_gauging_algo = FALSE;
            }

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("      gauging %ld",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_GAUGING)
            {
              vsi_o_event_ttrace("      gauging %ld      GAUGE: State =%ld, LF=%ld, HF=%ld, root=%ld, frac=%ld",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[5]);
            }

#else
            sprintf (str,"      gauging %ld\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0]);
            if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_GAUGING)
            {
              sprintf (str,"      gauging %ld\n\r      GAUGE: State =%ld, LF=%ld, HF=%ld, root=%ld, frac=%ld\n",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[5]);
            }
            L1_send_trace_cpy(str);
#endif
          }
          break;

   #if (D_ERROR_STATUS_TRACE_ENABLE)
          case L1S_D_ERROR_STATUS:
          {
            // trace the d_error_status word with the correct mask applied
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("> DSP %ld %ld %xh %ld",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3]);
#else
            sprintf (str,"> DSP %ld %ld %xh %ld\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3]);
            L1_send_trace_cpy(str);
#endif
          }
          break;
        #endif //(D_ERROR_STATUS_TRACE_ENABLE)

  #if L1_GPRS
          case L1S_PACKET_TRANSFER:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("PDTCH %ld %lx",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
            sprintf (str,"PDTCH %ld %lx\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case RLC_UL_PARAM:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("RLC_U %ld %ld %ld %ld %lx %lx %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6]);
#else
            sprintf (str,"RLC_U %ld %ld %ld %ld %lx %lx %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6]);

            L1_send_trace_cpy(str);
#endif
          }
          break;

          case RLC_DL_PARAM:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("RLC_D %ld %ld %lx %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5]);
#else
            sprintf (str,"RLC_D %ld %ld %lx %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5]);

            L1_send_trace_cpy(str);
#endif
          }
          break;

          case FORBIDDEN_UPLINK:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("> WARNING: UL allocated while no TA");
#else
            sprintf (str,"> WARNING: UL allocated while no TA\n\r");
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case DL_PTCCH:
          {
            // PTCCH value

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            if(!((T_TRACE_INFO *)(msg->SigP))->tab[0])
            {
              vsi_o_event_ttrace("-> PTCCH %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3],

                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x08 & 0xFF);
			}
			else vsi_o_event_ttrace("-> PTCCH %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                         ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
#else

            if(!((T_TRACE_INFO *)(msg->SigP))->tab[0])
            {

              sprintf (str,"-> PTCCH %d %d %d %d (%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d)\n\r",
                       ((T_TRACE_INFO *)(msg->SigP))->tab[0], //crc
                       ((T_TRACE_INFO *)(msg->SigP))->tab[1], //ordered_ta
                       ((T_TRACE_INFO *)(msg->SigP))->tab[2], //ta_index
                       ((T_TRACE_INFO *)(msg->SigP))->tab[3], //timeslot

                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x10 & 0xFF,//TA values
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[4] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[5] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[6] >> 0x08 & 0xFF,

                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x10 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x18 & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7]         & 0xFF,
                       ((T_TRACE_INFO *)(msg->SigP))->tab[7] >> 0x08 & 0xFF);
            }
            else sprintf(str,"-> PTCCH %d %d %d %d (%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d)\n\r",
                         ((T_TRACE_INFO *)(msg->SigP))->tab[0], // in case crc has occurred all other values are obsolete
                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

            L1_send_trace_cpy(str);
#endif
          }
          break;

          case PTCCH_DISABLED:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
              vsi_o_event_ttrace("> WARNING: PTCCH disabled by L1S");
#else
            sprintf (str,"> WARNING: PTCCH disabled by L1S\n\r");

            L1_send_trace_cpy(str);
#endif
          }
          break;

    #if (RLC_DL_BLOCK_STAT)
          case L1S_RLC_STAT:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("RLC STAT  : cs typ=%d nb frm=%ld nb bad frm=%ld nb cs1 frm=%ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3]);
#else
            sprintf (str,"RLC STAT  : cs typ=%d nb frm=%ld nb bad frm=%ld nb cs1 frm=%ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3]);
            L1_send_trace_cpy(str);
#endif

          }
          break;
          #endif //(RLC_DL_BLOCK_STAT)

  #endif // L1_GPRS

  #if (TRACE_TYPE == 4)
          case DYN_TRACE_CHANGE:
          {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace(">> L1 TRACE CONFIG = 0x%lx <<",
                     trace_info.current_config->l1_dyn_trace);
#else
            sprintf (str,"\n\r>> L1 TRACE CONFIG = 0x%lx <<\n\r",
                     trace_info.current_config->l1_dyn_trace);
            L1_send_trace_cpy(str);
#endif

          }
          break;

        #if (OP_L1_STANDALONE == 0)
          case DYN_TRACE_DEBUG:
          {
            // the message depends on the message type included in this message:
            switch (((T_TRACE_INFO *)(msg->SigP))->tab[0])
            {
              // common to all:
              //   ((T_TRACE_INFO *)(msg->SigP))->tab[7] contains the absolute fn
#if (L1_COMPRESSED_TRACING == 1)

			  case DYN_TRACE_1:
              {
                // trace when we override the sti present information
                vsi_o_event_ttrace("  >> STI discarded");
                break;
              }
              default:
              {
                vsi_o_event_ttrace ("Default: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
                break;
              }
            }

#else
              case DYN_TRACE_1:
              {
                // trace when we override the sti present information
                  sprintf (str,"  >> STI discarded\n\r");
                break;
              }
              default:
              {
                sprintf (str,"Default: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
                break;
              }
            }
            L1_send_trace_cpy(str);
#endif
          }
          break;
        #endif // #if (OP_L1_STANDALONE == 0)
  #endif // #if (TRACE_TYPE == 4)

        #if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)
          case DEDIC_TCH_BLOCK_STAT:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("DD_BL %ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2]);
#else
            sprintf (str,"DD_BL %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2]);
            L1_send_trace_cpy(str);
#endif
          }
          break;
        #endif //(DEBUG_DEDIC_TCH_BLOCK_STAT == 1)

        #if (AMR == 1)
        case TRACE_RATSCCH:
        {
          // tab[1] contains a bitmap of the AMR parameters updated
          UWORD16 amr_change_bitmap=((T_TRACE_INFO *)(msg->SigP))->tab[1];
          char modified[2]={' ','*'};

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("RAT_I %ld  acs%c 0x%02X,icm%c%d,cmip%c%d,thr1%c%02d,thr2%c%02d,thr3%c%02d,hyst1%c%02d,hyst2%c%02d,hyst3%c%02d",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_ACS))>>C_AMR_CHANGE_ACS],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.active_codec_set,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_ICM))>>C_AMR_CHANGE_ICM],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.initial_codec_mode,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_CMIP))>>C_AMR_CHANGE_CMIP],
                      l1a_l1s_com.dedic_set.aset->cmip,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR1))>>C_AMR_CHANGE_THR1],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR2))>>C_AMR_CHANGE_THR2],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[1],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR3))>>C_AMR_CHANGE_THR3],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[2],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST1))>>C_AMR_CHANGE_HYST1],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST2))>>C_AMR_CHANGE_HYST2],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[1],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST3))>>C_AMR_CHANGE_HYST3],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[2]);
#else
          sprintf(str,"RAT_I %ld  acs%c 0x%02X,icm%c%d,cmip%c%d,thr1%c%02d,thr2%c%02d,thr3%c%02d,hyst1%c%02d,hyst2%c%02d,hyst3%c%02d\n\r",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_ACS))>>C_AMR_CHANGE_ACS],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.active_codec_set,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_ICM))>>C_AMR_CHANGE_ICM],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.initial_codec_mode,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_CMIP))>>C_AMR_CHANGE_CMIP],
                      l1a_l1s_com.dedic_set.aset->cmip,
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR1))>>C_AMR_CHANGE_THR1],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR2))>>C_AMR_CHANGE_THR2],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[1],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_THR3))>>C_AMR_CHANGE_THR3],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.threshold[2],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST1))>>C_AMR_CHANGE_HYST1],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[0],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST2))>>C_AMR_CHANGE_HYST2],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[1],
                      modified[(amr_change_bitmap & (1<<C_AMR_CHANGE_HYST3))>>C_AMR_CHANGE_HYST3],
                      l1a_l1s_com.dedic_set.aset->amr_configuration.hysteresis[2]);
          L1_send_trace_cpy(str);
#endif
        }
        break;
        #endif
                  case IQ_LOW:
        {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("Warning : IQ LOW %ld %ld",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
#else
          sprintf(str,"Warning : IQ LOW %ld %ld\n\r",
                      ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                      ((T_TRACE_INFO *)(msg->SigP))->tab[1]);
          L1_send_trace_cpy(str);
#endif
        }
        break;



          default:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("DEB_I %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->debug_code,
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
#else
            sprintf (str,"DEB_I %ld %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->debug_code,
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
            L1_send_trace_cpy(str);
#endif
          }
          break;
        } // switch(debug_code)
      } // case TRACE_INFO:
      break;


  } // End switch (SignalCode)

  /***********************************************************************/
  /* L1S CPU load                                                        */
  /***********************************************************************/

  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_CPU_LOAD)
  {
    if(max_cpu_flag)
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("> CPU %ld %d",
              fn_max_cpu % 42432,
              max_cpu);
#else
      sprintf(str,"> CPU %ld %d\n\r",
              fn_max_cpu % 42432,
              max_cpu);
      L1_send_trace_cpy(str);
#endif

      max_cpu_flag = 0;
    }
  }

  /***********************************************************************/
  /* DSP CPU load                                                        */
  /***********************************************************************/

#if (DSP >= 38)
  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_CPU_LOAD)
  {
    if(l1_dsp_cpu_load_trace_flag)
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace(str,"DSP_CPU_T0:%ld T1:%ld T2:%ld T3:%ld I:%ld \n\r",
              dsp_max_cpu_load_trace_array[0],
              dsp_max_cpu_load_trace_array[1],
              dsp_max_cpu_load_trace_array[2],
              dsp_max_cpu_load_trace_array[3],
              dsp_max_cpu_load_idle_frame);
#else
      sprintf(str,"DSP_CPU_T0:%ld T1:%ld T2:%ld T3:%ld I:%ld \n\r",
              dsp_max_cpu_load_trace_array[0],
              dsp_max_cpu_load_trace_array[1],
              dsp_max_cpu_load_trace_array[2],
              dsp_max_cpu_load_trace_array[3],
              dsp_max_cpu_load_idle_frame);
      L1_send_trace_cpy(str);
#endif

      dsp_max_cpu_load_trace_array[0] = 0L;
      dsp_max_cpu_load_trace_array[1] = 0L;
      dsp_max_cpu_load_trace_array[2] = 0L;
      dsp_max_cpu_load_trace_array[3] = 0L;
      dsp_max_cpu_load_idle_frame     = 0L;

      l1_dsp_cpu_load_trace_flag = 0;
    }
  }
#endif	// DSP >= 38

  /***********************************************************************/
  /* L1A messages                                                        */
  /***********************************************************************/

  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
  {

    // Trace message content...
    //=========================
    switch(msg->SignalCode)
    {

      /********************************************************************************/
      /* CIRCUIT SWITCHED                                                             */
      /********************************************************************************/


      /////////////////////////////////////////
      // Message to set the right radio band //
      /////////////////////////////////////////
      case MPHC_INIT_L1_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        // L1_FF_MULTIBAND TBD
        vsi_o_event_ttrace("BAND_R %d",((T_MPHC_INIT_L1_REQ *)(msg->SigP))-> radio_band_config);
#else
#if (L1_FF_MULTIBAND == 0)
        sprintf (str, "BAND_R %d",((T_MPHC_INIT_L1_REQ *)(msg->SigP))-> radio_band_config);
#else
        sprintf(str,"BAND_R");
#endif /*#if (L1_FF_MULTIBAND == 1)*/

        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_INIT_L1_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BAND_C ");
#else
#if (L1_FF_MULTIBAND == 0)
        sprintf (str, "BAND_C");
#else
        UWORD16 n = 0;
        UWORD8 i = 0;
        n = sprintf(str, "BAND_C %ld", (l1s.actual_time.fn));
        for(i = 0; i < NB_MAX_SUPPORTED_BANDS; i ++)
        {
          n += sprintf (str + n, "%d %d\n",
                                            ((T_MPHC_INIT_L1_CON *)(msg->SigP))->multiband_power_class[i].radio_band,
                                            ((T_MPHC_INIT_L1_CON *)(msg->SigP))->multiband_power_class[i].power_class
                                            );
        }
#endif

        L1_send_trace_cpy(str);
#endif
      }
      break;

      ////////////////////////////
      // Serving Cell selection //
      ////////////////////////////

      case MPHC_NEW_SCELL_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CEL_R %ld %d %ld %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->bsic);
#else
        sprintf(str,"CEL_R %ld %d %ld %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->bsic);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////////////////////
      // Neighbor cell monitoring //
      //////////////////////////////

      case MPHC_NETWORK_LOST_IND:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("LOS_R %ld",
                l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"LOS_R %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Idle mode neighbor cell synchronization

      case MPHC_NETWORK_SYNC_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("NET_R %ld %d %ld %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->timing_validity,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->search_mode);
#else
        sprintf(str,"NET_R %ld %d %ld %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->timing_validity,
                ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->search_mode);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_NETWORK_SYNC_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("NET_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"NET_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_NCELL_SYNC_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("NSY_R %ld %d %ld %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->timing_validity);
#else
        sprintf(str,"NSY_R %ld %d %ld %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->timing_validity);
        L1_send_trace_cpy(str);
#endif
      }
      break;

#if (L1_12NEIGH ==1)

      case MPHC_NCELL_LIST_SYNC_REQ:
      {
        UWORD8 i;

        pgoset++;
        sprintf(str,"NSYL_R %ld %d %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->eotd,
                ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size);

        for (i=0; i < ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size; i++)
        {
          sprintf(str2,"(%ld %ld %ld %ld)",
                  ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].timing_validity,
                  ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].time_alignmt,
                  ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].fn_offset,
                  ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->ncell_list[i].radio_freq);
          strcat(str,str2);
        }

        strcat(str,"\n\r");

        L1_send_trace_cpy(str);
      }
      break;

      #endif //(L1_12NEIGH ==1)

      case MPHC_STOP_NCELL_SYNC_REQ:
      {
        UWORD8 i;

        sprintf(str,"NSY_S %ld",
                l1s.actual_time.fn_mod42432);
        // New
        sprintf(str2," %d", ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size);
        strcat(str, str2);

        for (i=0; i < ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size; i++)
        {
          sprintf(str2," %d",
                  ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[i]);
          strcat(str,str2);
        }

        strcat(str,"\n\r");
        L1_send_trace_cpy(str);
      }
      break;

      // Dedicated mode neigbor cell synchronization

      case MPHC_NCELL_FB_SB_READ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("FB_R  %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_FB_SB_READ *)(msg->SigP))->radio_freq);
#else
        sprintf(str,"FB_R  %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_FB_SB_READ *)(msg->SigP))->radio_freq);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_NCELL_SB_READ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("SB_R  %ld %d %ld %ld",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->time_alignmt);
#else
        sprintf(str,"SB_R  %ld %d %ld %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->time_alignmt);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_FB_INFO:
      {
#if(L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
        operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(((T_L1C_FB_INFO*)(msg->SigP))->radio_freq);
#endif
        
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("FB_I  %ld %d %d %ld %ld %ld %ld %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag,
                ((T_L1C_FB_INFO*)(msg->SigP))->radio_freq,
                ((T_L1C_FB_INFO*)(msg->SigP))->pm,
                ((T_L1C_FB_INFO*)(msg->SigP))->toa,
                ((T_L1C_FB_INFO*)(msg->SigP))->angle,
                ((T_L1C_FB_INFO*)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_FB_INFO*)(msg->SigP))->radio_freq -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else // L1_FF_MULTIBAND
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
                
#endif // L1_FF_MULTIBAND
                
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"FB_I  %ld %d %d %ld %ld %ld %ld %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag,
                ((T_L1C_FB_INFO*)(msg->SigP))->radio_freq,
                ((T_L1C_FB_INFO*)(msg->SigP))->pm,
                ((T_L1C_FB_INFO*)(msg->SigP))->toa,
                ((T_L1C_FB_INFO*)(msg->SigP))->angle,
                ((T_L1C_FB_INFO*)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_FB_INFO*)(msg->SigP))->radio_freq -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else // L1_FF_MULTIBAND

                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
                
#endif // L1_FF_MULTIBAND
                
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_SB_INFO:
      {
#if(L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
        operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(((T_L1C_SB_INFO*)(msg->SigP))->radio_freq);
#endif
        
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("SB_I  %ld %d %d %d %ld %ld %ld %ld %ld %ld %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag,
                ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq,
                ((T_L1C_SB_INFO *)(msg->SigP))->bsic,
                ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset,
                ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt,
                ((T_L1C_SB_INFO *)(msg->SigP))->pm,
                ((T_L1C_SB_INFO *)(msg->SigP))->toa,
                ((T_L1C_SB_INFO *)(msg->SigP))->angle,
                ((T_L1C_SB_INFO *)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_SB_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"SB_I  %ld %d %d %d %ld %ld %ld %ld %ld %ld %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag,
                ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq,
                ((T_L1C_SB_INFO *)(msg->SigP))->bsic,
                ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset,
                ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt,
                ((T_L1C_SB_INFO *)(msg->SigP))->pm,
                ((T_L1C_SB_INFO *)(msg->SigP))->toa,
                ((T_L1C_SB_INFO *)(msg->SigP))->angle,
                ((T_L1C_SB_INFO *)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_SB_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;
#if ((REL99 == 1) && (FF_BHO == 1))
      case L1C_FBSB_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
vsi_o_event_ttrace"FBSB_I  %ld %d %d %d %ld %ld %ld %ld %ld %ld\n\r",
               l1s.actual_time.fn_mod42432,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->fb_flag,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->sb_flag,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->bsic,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->fn_offset,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->time_alignmt,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->pm,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->toa,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->angle,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->snr
               );
#else
        sprintf(str,"FBSB_I  %ld %d %d %d %ld %ld %ld %ld %ld %ld\n\r",
               l1s.actual_time.fn_mod42432,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->fb_flag,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->sb_flag,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->bsic,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->fn_offset,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->time_alignmt,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->pm,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->toa,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->angle,
               ((T_L1C_FBSB_INFO *)(msg->SigP))->snr
               );
        L1_send_trace_cpy(str);
#endif
      }
      break;
#endif
      case L1C_SBCONF_INFO:
      {
#if(L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
        operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(((T_L1C_SB_INFO *)(msg->SigP))->radio_freq);
#endif
        
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("SBC_I %ld %d %d %d %ld %ld %ld %ld %ld %ld %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag,
                ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq,
                ((T_L1C_SB_INFO *)(msg->SigP))->bsic,
                ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset,
                ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt,
                ((T_L1C_SB_INFO *)(msg->SigP))->pm,
                ((T_L1C_SB_INFO *)(msg->SigP))->toa,
                ((T_L1C_SB_INFO *)(msg->SigP))->angle,
                ((T_L1C_SB_INFO *)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_SB_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"SBC_I %ld %d %d %d %ld %ld %ld %ld %ld %ld %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag,
                ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq,
                ((T_L1C_SB_INFO *)(msg->SigP))->bsic,
                ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset,
                ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt,
                ((T_L1C_SB_INFO *)(msg->SigP))->pm,
                ((T_L1C_SB_INFO *)(msg->SigP))->toa,
                ((T_L1C_SB_INFO *)(msg->SigP))->angle,
                ((T_L1C_SB_INFO *)(msg->SigP))->snr,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_L1C_SB_INFO *)(msg->SigP))->radio_freq - l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_NETWORK_SYNC_IND:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("NET_I %ld %d %d %ld %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->sb_flag,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->radio_freq,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->fn_offset,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->time_alignmt,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->bsic);
#else
        sprintf(str,"NET_I %ld %d %d %ld %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->sb_flag,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->radio_freq,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->fn_offset,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->time_alignmt,
                ((T_MPHC_NETWORK_SYNC_IND*)(msg->SigP))->bsic);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_NCELL_SYNC_IND:
      {

#if  (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1)
  #if (L1_EOTD)
        vsi_o_event_ttrace("NSY_I %ld %d %d %ld %ld %d %d %ld %ld %ld %ld %ld %ld %ld %ld",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id,
                l1a_l1s_com.nsync.current_list_size,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->d_eotd_first,
                pgoset,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_sb_neigh,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_in_SB,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->timetag,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_fn,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_qbit);
  #else
        vsi_o_event_ttrace("NSY_I %ld %d %d %ld %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id);

  #endif //(L1_EOTD)

#else
    #if (L1_EOTD)
        sprintf(str,"NSY_I %ld %d %d %ld %ld %d %d %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
    #else
        sprintf(str,"NSY_I %ld %d %d %ld %ld %d %d\n\r",
    #endif
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->sb_flag,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->bsic,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->neigh_id
              #if (L1_EOTD)
                ,l1a_l1s_com.nsync.current_list_size,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->d_eotd_first,
                pgoset,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_sb_neigh,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->fn_in_SB,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->timetag,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_fn,
                ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->delta_qbit
              #endif //(L1_EOTD)
              );

        L1_send_trace_cpy(str);
#endif

      }
      break;

      // Neighbor cell BCCH reading

      case MPHC_NCELL_BCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
		#if L1_GPRS
        vsi_o_event_ttrace("BN_R  %ld %d %ld %ld %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority);
        #else
        vsi_o_event_ttrace("BN_R  %ld %d %ld %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req);

        #endif //L1_GPRS
#else
        #if L1_GPRS
        sprintf(str,"BN_R  %ld %d %ld %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority);
        #else
        sprintf(str,"BN_R  %ld %d %ld %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc,
                ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req);

        #endif //L1_GPRS
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_NCELL_BCCH_REQ:
      {
        UWORD8 i;

        sprintf(str,"BN_S  %ld",
                l1s.actual_time.fn_mod42432);

        for (i=0; i < ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array_size; i++)
        {
          sprintf(str2," %d",
                  ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[i]);
          strcat(str,str2);
        }
        strcat(str,"\n\r");
        L1_send_trace_cpy(str);
      }
      break;

      case L1C_BCCHN_INFO:
      {
#if(L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
        operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq);
#endif
        
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BN_I  %ld %d %d %d %ld %d",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"BN_I  %ld %d %d %d %ld %d\n\r",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      ///////////////////////////////////////
      // Serving cell normal burst reading //
      ///////////////////////////////////////

      // CCCH reading

      case MPHC_START_CCCH_REQ:
      {
        trace_info.rxlev_req_count = 0;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CCH_R %ld %d %d %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_mode);
#else
        sprintf(str,"CCH_R %ld %d %d %d %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index,
                ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_mode);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_CCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CCH_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"CCH_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_NP_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
#if (FF_L1_FAST_DECODING == 1)
        vsi_o_event_ttrace("NP_I  %ld %d %d %d %ld %d %d %d %d %d %d %c",
#else
        vsi_o_event_ttrace("NP_I  %ld %d %d %d %ld %d %d %d %d %d %c",
#endif /* FF_L1_FAST_DECODING */
                  ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                  ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                  ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                  l1a_l1s_com.Scell_IL_for_rxlev,
                  l1s.tpu_offset,
                  l1s.afc,
#if (FF_L1_FAST_DECODING == 1)
                  l1a_l1s_com.last_fast_decoding,
#endif
                  toa_tab[0],toa_tab[1],toa_tab[2], toa_tab[3],
                  trace_info.l1_memorize_error);
#else
#if (FF_L1_FAST_DECODING == 1)
               sprintf(str,"NP_I  %ld %d %d %d %ld %d %d %d %d %d %d %c\n\r",
#else
          sprintf(str,"NP_I  %ld %d %d %d %ld %d %d %d %d %d %c\n\r",
#endif
                  ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                  ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                  ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                  l1a_l1s_com.Scell_IL_for_rxlev,
                  l1s.tpu_offset,
                  l1s.afc,
#if (FF_L1_FAST_DECODING == 1)
                  l1a_l1s_com.last_fast_decoding,
#endif
                  toa_tab[0],toa_tab[1],toa_tab[2], toa_tab[3],
                  trace_info.l1_memorize_error);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_EP_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("EP_I  %ld %d %d %d %ld %d",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"EP_I  %ld %d %d %d %ld %d\n\r",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_ALLC_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CC_I  %ld %d %d %d %ld %d",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"CC_I  %ld %d %d %d %ld %d\n\r",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // BCCH reading

      case MPHC_SCELL_NBCCH_REQ:
      {
        UWORD8 i;

        sprintf(str,"NBS_R %ld ",
                l1s.actual_time.fn_mod42432);

        for (i = 0; i < ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array_size; i++)
        {
          sprintf(str2,"(%d %d)",
                  ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].modulus,
                  ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].relative_position);
          strcat(str, str2);
        }
        strcat(str,"\n\r");

        L1_send_trace_cpy(str);
      }
      break;

      case MPHC_SCELL_EBCCH_REQ:
      {
        UWORD8 i;

        sprintf(str,"EBS_R %ld ",
                l1s.actual_time.fn_mod42432);

        for (i = 0; i < ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array_size; i++)
        {
          sprintf(str2,"(%d %d)",
                  ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].modulus,
                  ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].relative_position);
          strcat(str,str2);
        }
        strcat(str,"\n\r");

        L1_send_trace_cpy(str);
      }
      break;

      case MPHC_STOP_SCELL_BCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BS_S  %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"BS_S  %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_BCCHS_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
#if (FF_L1_FAST_DECODING == 1)
        vsi_o_event_ttrace("BS_I  %ld %d %d %d %ld %d %d",
#else
        vsi_o_event_ttrace("BS_I  %ld %d %d %d %ld %d",
#endif
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc
#if (FF_L1_FAST_DECODING == 1)
                  ,l1a_l1s_com.last_fast_decoding
#endif
                );
#else
#if (FF_L1_FAST_DECODING == 1)
        sprintf(str,"BS_I  %ld %d %d %d %ld %d %d\n\r",
#else
        sprintf(str,"BS_I  %ld %d %d %d %ld %d\n\r",
#endif
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc
#if (FF_L1_FAST_DECODING == 1)
                  ,l1a_l1s_com.last_fast_decoding
#endif
                );
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////
      // CBCH //
      //////////

      case MPHC_CONFIG_CBCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CBC_R %ld %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.h,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.timeslot_no);
#else
        sprintf(str,"CBC_R %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.h,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc.timeslot_no);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_CBCH_SCHEDULE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CBS_R %ld %d %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->extended_cbch,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->schedule_length,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_0,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_1);
#else
        sprintf(str,"CBS_R %ld %d %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->extended_cbch,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->schedule_length,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_0,
                ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_1);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_CBCH_INFO_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CBI_R %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_INFO_REQ *)(msg->SigP))->tb_bitmap);
#else
        sprintf(str,"CBI_R %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_INFO_REQ *)(msg->SigP))->tb_bitmap);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_CBCH_UPDATE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CBU_R %ld %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->extended_cbch,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_0,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_1);
#else
        sprintf(str,"CBU_R %ld %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->extended_cbch,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_0,
                ((T_MPHC_CBCH_UPDATE_REQ*)(msg->SigP))->first_block_1);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_CB_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CB_I  %ld %d %d %d %ld %d",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"CB_I  %ld %d %d %d %ld %d\n\r",
                ((T_MPHC_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHC_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHC_DATA_IND *)(msg->SigP))->radio_freq,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Stop CBCH

      case MPHC_STOP_CBCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ECB_S  %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_STOP_CBCH_REQ*)(msg->SigP))->normal_cbch,
                ((T_MPHC_STOP_CBCH_REQ*)(msg->SigP))->extended_cbch);
#else
        sprintf(str,"ECB_S  %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_STOP_CBCH_REQ*)(msg->SigP))->normal_cbch,
                ((T_MPHC_STOP_CBCH_REQ*)(msg->SigP))->extended_cbch);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_CBCH_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ECB_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"ECB_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      ///////////////////
      // Random Access //
      ///////////////////

      case MPHC_RA_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("RA_R  %ld %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RA_REQ *)(msg->SigP))->txpwr,
                ((T_MPHC_RA_REQ *)(msg->SigP))->rand,
                ((T_MPHC_RA_REQ *)(msg->SigP))->channel_request
#if (L1_FF_MULTIBAND == 0)                
                ,((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band1,
                ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band2
#endif                
                );
#else
        sprintf(str,"RA_R  %ld %d %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RA_REQ *)(msg->SigP))->txpwr,
                ((T_MPHC_RA_REQ *)(msg->SigP))->rand,
                ((T_MPHC_RA_REQ *)(msg->SigP))->channel_request
#if (L1_FF_MULTIBAND == 0)                
                ,((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band1,
                ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band2
#endif                
                );
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_RA_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("RA_S  %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"RA_S  %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_RA_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("RA_C  %ld",
                ((T_MPHC_RA_CON *)(msg->SigP))->fn % 42432);
#else
        sprintf(str,"RA_C  %ld\n\r",
                ((T_MPHC_RA_CON *)(msg->SigP))->fn % 42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      /////////////////////////////
      // Dedicated mode channels //
      /////////////////////////////

      // Immediate assignment

      case MPHC_IMMED_ASSIGN_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("IAS_R %ld %d %d %d %d %d %d %d %d %ld %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.h,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.channel_type,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.subchannel,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.timeslot_no,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.tsc,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->timing_advance,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                l1a_decode_starting_time(((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time),
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->dtx_allowed,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->pwrc);
#else
        sprintf(str,"\n\rIAS_R %ld %d %d %d %d %d %d %d %d %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.h,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.channel_type,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.subchannel,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.timeslot_no,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc.tsc,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->timing_advance,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                l1a_decode_starting_time(((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time),
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->dtx_allowed,
                ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->pwrc);
        L1_send_trace_cpy(str);
#endif


        // a message was received correctly from the Network, it means the system behavior is nice
        // and so we can allow more trace on a PM/COM error.
        trace_info.trace_filter = FALSE;

      }
      break;

      case MPHC_IMMED_ASSIGN_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("IAS_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"IAS_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Channel assignment

      case MPHC_CHANNEL_ASSIGN_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        #if (AMR == 1)
          vsi_o_event_ttrace("CAS_R %ld %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.h,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.channel_type,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.subchannel,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.timeslot_no,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.tsc,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time),
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.active_codec_set,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[0],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[1],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[2],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[0],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[1],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[2]);
        #else
          vsi_o_event_ttrace("CAS_R %ld %d %d %d %d %d %d %d %d %d %ld %d %d %d %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.h,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.channel_type,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.subchannel,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.timeslot_no,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.tsc,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time),
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed);
        #endif //(AMR == 1)

#else
        #if (AMR == 1)
          sprintf(str,"\n\rCAS_R %ld %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.h,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.channel_type,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.subchannel,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.timeslot_no,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.tsc,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time),
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.active_codec_set,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[0],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[1],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.threshold[2],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[0],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[1],
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration.hysteresis[2]);
        #else
          sprintf(str,"\n\rCAS_R %ld %d %d %d %d %d %d %d %d %d %ld %d %d %d %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.h,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.channel_type,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.subchannel,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.timeslot_no,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1.tsc,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time),
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti.rf_chan_cnt,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm,
                  ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed);
        #endif //(AMR == 1)
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_CHANNEL_ASSIGN_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CAS_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"CAS_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // SACCH reception

      case L1C_SACCH_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("SA_I  %ld %d %d (%d %d) %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_PH_DATA_IND *)(msg->SigP))->error_cause,
                ((T_PH_DATA_IND *)(msg->SigP))->rf_chan_num,
                l1a_l1s_com.Scell_info.traffic_meas_beacon.input_level,
                l1a_l1s_com.Scell_info.traffic_meas.input_level,
                l1s.tpu_offset,
                l1s.afc,
                trace_info.sacch_d_nerr);
#else
     #if  FF_REPEATED_SACCH
        sprintf(str,"SA_I  %ld %d %d (%d %d) %ld %d %d %d %d %d %d %d\n\r",
     #else
        sprintf(str,"SA_I  %ld %d %d (%d %d) %ld %d %d\n\r",
     #endif /* FF_REPEATED_SACCH */
                l1s.actual_time.fn_mod42432,
                ((T_PH_DATA_IND *)(msg->SigP))->error_cause,
                ((T_PH_DATA_IND *)(msg->SigP))->rf_chan_num,
                l1a_l1s_com.Scell_info.traffic_meas_beacon.input_level,
                l1a_l1s_com.Scell_info.traffic_meas.input_level,
                l1s.tpu_offset,
                l1s.afc,
                trace_info.sacch_d_nerr
              #if  FF_REPEATED_SACCH
                ,trace_info.repeat_sacch.dl_count,
                trace_info.repeat_sacch.dl_combined_good_count,
                trace_info.repeat_sacch.srr,
                trace_info.repeat_sacch.sro,
                trace_info.repeat_sacch.dl_error_count
              #endif /* FF_REPEATED_SACCH */

                );
        L1_send_trace_cpy(str);
#endif
#if (AUDIO_DEBUG == 1)
        {

          char str3[100];
          char str4[100];
          volatile UWORD16 *p_audio_debug_buffer;
          p_audio_debug_buffer = (UWORD16 *) (DSP_AUDIO_DEBUG_API_ADDR);
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("AU_DBG D %x %x %x %x %x %x %x %x %x %x %x %x M %x %x %x %x %x",
                  p_audio_debug_buffer[0], p_audio_debug_buffer[1],
                  p_audio_debug_buffer[2], p_audio_debug_buffer[3],
                  p_audio_debug_buffer[4], p_audio_debug_buffer[5],
                  p_audio_debug_buffer[6], p_audio_debug_buffer[7],
                  p_audio_debug_buffer[8], p_audio_debug_buffer[9],
                  p_audio_debug_buffer[10], p_audio_debug_buffer[11],
                  trace_info.audio_debug_var.vocoder_enable_status,
                  trace_info.audio_debug_var.ul_state,
                  trace_info.audio_debug_var.dl_state,
                  trace_info.audio_debug_var.ul_onoff_counter,
                  trace_info.audio_debug_var.dl_onoff_counter);
#else
          sprintf(str3,"AU_DBG D %x %x %x %x %x %x %x %x %x %x %x %x M %x %x %x %x %x\n\r",
                  p_audio_debug_buffer[0], p_audio_debug_buffer[1],
                  p_audio_debug_buffer[2], p_audio_debug_buffer[3],
                  p_audio_debug_buffer[4], p_audio_debug_buffer[5],
                  p_audio_debug_buffer[6], p_audio_debug_buffer[7],
                  p_audio_debug_buffer[8], p_audio_debug_buffer[9],
                  p_audio_debug_buffer[10], p_audio_debug_buffer[11],
                  trace_info.audio_debug_var.vocoder_enable_status,
                  trace_info.audio_debug_var.ul_state,
                  trace_info.audio_debug_var.dl_state,
                  trace_info.audio_debug_var.ul_onoff_counter,
                  trace_info.audio_debug_var.dl_onoff_counter);
          L1_send_trace_cpy(str3);
#endif
          Trace_l1_audio_regs();
        }
#endif
      }
      break;

      // Channel modification

      case MPHC_CHANGE_FREQUENCY:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("FRQ_R %ld %d %d %d %d %d %d %d %ld",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.h,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.channel_type,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.subchannel,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.timeslot_no,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.tsc,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->frequency_list.rf_chan_cnt,
                l1a_decode_starting_time(((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time));
#else
        sprintf(str,"FRQ_R %ld %d %d %d %d %d %d %d %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.h,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.channel_type,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.subchannel,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.timeslot_no,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.tsc,
                ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->frequency_list.rf_chan_cnt,
                l1a_decode_starting_time(((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time));
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_REDEF_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("FRQ_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"FRQ_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_CHANNEL_MODE_MODIFY_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        #if (AMR == 1)
        vsi_o_event_ttrace("MOD_R %ld %d %d %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.active_codec_set,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[0],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[1],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[2],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[0],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[1],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[2]);
        #else
          vsi_o_event_ttrace("MOD_R %ld %d %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode);
        #endif //(AMR == 1)
#else
        #if (AMR == 1)
          sprintf(str,"MOD_R %ld %d %d %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.noise_suppression_bit,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode_indicator,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.initial_codec_mode,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.active_codec_set,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[0],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[1],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.threshold[2],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[0],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[1],
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration.hysteresis[2]);
        #else
          sprintf(str,"MOD_R %ld %d %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel,
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode);
        #endif //(AMR == 1)

        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Ciphering

      case MPHC_SET_CIPHERING_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CIP_R %ld %d %d %d %d %d %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->cipher_mode,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->a5_algorithm,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[0],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[1],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[2],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[3],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[4],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[5],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[6],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[7]);
#else
        sprintf(str,"CIP_R %ld %d %d %d %d %d %d %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->cipher_mode,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->a5_algorithm,
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[0],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[1],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[2],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[3],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[4],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[5],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[6],
                ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param.A[7]);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Generic stop

      case MPHC_STOP_DEDICATED_REQ:
      {
	#if (FF_REPEATED_DL_FACCH == 1)
 //       float fer_facch = 100* (1- ((trace_info.facch_dl_good_block_reported / trace_info.facch_dl_count_all)<<1));
          unsigned int x1= (100*(trace_info.facch_dl_count_all >> 1)) - (100*trace_info.facch_dl_good_block_reported);
          unsigned int y1 = trace_info.facch_dl_count_all >> 1;
          unsigned int fer_facch = (x1+(y1-1))/y1;


      #endif /*(FF_REPEATED_DL_FACCH == 1)*/

       #if (FF_REPEATED_SACCH ==1)
       unsigned int errors = trace_info.repeat_sacch.dl_error_count;
       unsigned int fer_sacch1 =  (errors *100)/trace_info.repeat_sacch.dl_count;

#if 0
       if ((trace_info.repeat_sacch.dl_count>> 1) > trace_info.repeat_sacch.dl_good_norep )
           x  = 100*(trace_info.repeat_sacch.dl_count >> 1) - 100*(trace_info.repeat_sacch.dl_good_norep);
       else
           x  = 100*(trace_info.repeat_sacch.dl_count) - 100*(trace_info.repeat_sacch.dl_good_norep);

	dl_good_norep = (trace_info.repeat_sacch.dl_good_norep);
      y   = (trace_info.repeat_sacch.dl_count / 2);
       fer_sacch = (x+(y-1))/y;
#endif

      #endif/* FF_REPEATED_SACCH */
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("DDS_R  %ld",
                l1s.actual_time.fn_mod42432);
#else

#if (FF_REPEATED_SACCH == 1) ||(FF_REPEATED_DL_FACCH == 1)
     //   sprintf(str,"DDS_R  %ld   (dl_good_norep : %u ) (x = frame errors :  %u ) (y = dl_count/2 =  %u) FER_S(%u) \n\r",
    // sprintf(str,"DDS_R  %ld|%u|%u|%u|%u|\n\r", l1s.actual_time.fn_mod42432,dl_good_norep,x,y,fer_sacch);
    sprintf(str,"DDS_R  %ld|%u|%u|%u |%u | %u \n\r", l1s.actual_time.fn_mod42432,fer_sacch1
#if (FF_REPEATED_DL_FACCH == 1)
, fer_facch,
trace_info.facch_dl_count_all,
trace_info.facch_dl_good_block_reported

#endif
);

#else
          sprintf(str,"DDS_R  %ld\n\r", l1s.actual_time.fn_mod42432);
#endif/* FF_REPEATED_SACCH */
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_DEDICATED_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("DDS_C  %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"DDS_C  %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////
      // HANDOVER //
      //////////////

      // Asynchronous handover request

      case MPHC_ASYNC_HO_REQ:
      {
        #define msg_aho ((T_MPHC_ASYNC_HO_REQ *)(msg->SigP))
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        #if (AMR == 1)
          vsi_o_event_ttrace("AHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d",
                  l1s.actual_time.fn_mod42432,
                  msg_aho->handover_command.cell_description.ncc,
                  msg_aho->handover_command.cell_description.bcc,
                  msg_aho->handover_command.cell_description.bcch_carrier,
                  msg_aho->handover_command.channel_desc_1.chan_sel.h,
                  msg_aho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_aho->handover_command.channel_desc_1.channel_type,
                  msg_aho->handover_command.channel_desc_1.subchannel,
                  msg_aho->handover_command.channel_desc_1.timeslot_no,
                  msg_aho->handover_command.channel_desc_1.tsc,
                  msg_aho->handover_command.channel_mode_1,
                  msg_aho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_aho->handover_command.starting_time),
                  msg_aho->handover_command.ho_acc,
                  msg_aho->handover_command.txpwr,
                  msg_aho->handover_command.report_time_diff,
                  msg_aho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_aho->handover_command.cipher_mode,
                  msg_aho->handover_command.a5_algorithm,
                  msg_aho->fn_offset,
                  msg_aho->time_alignmt,
                  msg_aho->amr_configuration.noise_suppression_bit,
                  msg_aho->amr_configuration.initial_codec_mode_indicator,
                  msg_aho->amr_configuration.initial_codec_mode,
                  msg_aho->amr_configuration.active_codec_set,
                  msg_aho->amr_configuration.threshold[0],
                  msg_aho->amr_configuration.threshold[1],
                  msg_aho->amr_configuration.threshold[2],
                  msg_aho->amr_configuration.hysteresis[0],
                  msg_aho->amr_configuration.hysteresis[1],
                  msg_aho->amr_configuration.hysteresis[2]);
        #else
          vsi_o_event_ttrace("AHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld",
                  l1s.actual_time.fn_mod42432,
                  msg_aho->handover_command.cell_description.ncc,
                  msg_aho->handover_command.cell_description.bcc,
                  msg_aho->handover_command.cell_description.bcch_carrier,
                  msg_aho->handover_command.channel_desc_1.chan_sel.h,
                  msg_aho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_aho->handover_command.channel_desc_1.channel_type,
                  msg_aho->handover_command.channel_desc_1.subchannel,
                  msg_aho->handover_command.channel_desc_1.timeslot_no,
                  msg_aho->handover_command.channel_desc_1.tsc,
                  msg_aho->handover_command.channel_mode_1,
                  msg_aho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_aho->handover_command.starting_time),
                  msg_aho->handover_command.ho_acc,
                  msg_aho->handover_command.txpwr,
                  msg_aho->handover_command.report_time_diff,
                  msg_aho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_aho->handover_command.cipher_mode,
                  msg_aho->handover_command.a5_algorithm,
                  msg_aho->fn_offset,
                  msg_aho->time_alignmt);
        #endif //(AMR == 1)
#else
        #if (AMR == 1)
          sprintf(str,"AHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld\n\r%1d %1d %1d %2x %2d %2d %2d %2d %2d %2d\n\r",
                  l1s.actual_time.fn_mod42432,
                  msg_aho->handover_command.cell_description.ncc,
                  msg_aho->handover_command.cell_description.bcc,
                  msg_aho->handover_command.cell_description.bcch_carrier,
                  msg_aho->handover_command.channel_desc_1.chan_sel.h,
                  msg_aho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_aho->handover_command.channel_desc_1.channel_type,
                  msg_aho->handover_command.channel_desc_1.subchannel,
                  msg_aho->handover_command.channel_desc_1.timeslot_no,
                  msg_aho->handover_command.channel_desc_1.tsc,
                  msg_aho->handover_command.channel_mode_1,
                  msg_aho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_aho->handover_command.starting_time),
                  msg_aho->handover_command.ho_acc,
                  msg_aho->handover_command.txpwr,
                  msg_aho->handover_command.report_time_diff,
                  msg_aho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_aho->handover_command.cipher_mode,
                  msg_aho->handover_command.a5_algorithm,
                  msg_aho->fn_offset,
                  msg_aho->time_alignmt,
                  msg_aho->amr_configuration.noise_suppression_bit,
                  msg_aho->amr_configuration.initial_codec_mode_indicator,
                  msg_aho->amr_configuration.initial_codec_mode,
                  msg_aho->amr_configuration.active_codec_set,
                  msg_aho->amr_configuration.threshold[0],
                  msg_aho->amr_configuration.threshold[1],
                  msg_aho->amr_configuration.threshold[2],
                  msg_aho->amr_configuration.hysteresis[0],
                  msg_aho->amr_configuration.hysteresis[1],
                  msg_aho->amr_configuration.hysteresis[2])
        #if ((REL99 == 1) && (FF_BHO == 1))
                 ,msg_aho->handover_type
#endif
                 ;
#else
          sprintf(str,"AHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  msg_aho->handover_command.cell_description.ncc,
                  msg_aho->handover_command.cell_description.bcc,
                  msg_aho->handover_command.cell_description.bcch_carrier,
                  msg_aho->handover_command.channel_desc_1.chan_sel.h,
                  msg_aho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_aho->handover_command.channel_desc_1.channel_type,
                  msg_aho->handover_command.channel_desc_1.subchannel,
                  msg_aho->handover_command.channel_desc_1.timeslot_no,
                  msg_aho->handover_command.channel_desc_1.tsc,
                  msg_aho->handover_command.channel_mode_1,
                  msg_aho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_aho->handover_command.starting_time),
                  msg_aho->handover_command.ho_acc,
                  msg_aho->handover_command.txpwr,
                  msg_aho->handover_command.report_time_diff,
                  msg_aho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_aho->handover_command.cipher_mode,
                  msg_aho->handover_command.a5_algorithm,
                  msg_aho->fn_offset,
                  msg_aho->time_alignmt);
        #endif //(AMR == 1)
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_ASYNC_HO_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("AHO_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"AHO_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Synchronous handover request

      case MPHC_SYNC_HO_REQ:
      case MPHC_PRE_SYNC_HO_REQ:
      case MPHC_PSEUDO_SYNC_HO_REQ:
      {
        #define msg_sho ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        #if (AMR == 1)
          vsi_o_event_ttrace("SHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld %1d %1d %1d %2x %2d %2d %2d %2d %2d %2d",
                  l1s.actual_time.fn_mod42432,
                  msg_sho->handover_command.cell_description.ncc,
                  msg_sho->handover_command.cell_description.bcc,
                  msg_sho->handover_command.cell_description.bcch_carrier,
                  msg_sho->handover_command.channel_desc_1.chan_sel.h,
                  msg_sho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_sho->handover_command.channel_desc_1.channel_type,
                  msg_sho->handover_command.channel_desc_1.subchannel,
                  msg_sho->handover_command.channel_desc_1.timeslot_no,
                  msg_sho->handover_command.channel_desc_1.tsc,
                  msg_sho->handover_command.channel_mode_1,
                  msg_sho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_sho->handover_command.starting_time),
                  msg_sho->handover_command.ho_acc,
                  msg_sho->handover_command.txpwr,
                  msg_sho->handover_command.report_time_diff,
                  msg_sho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_sho->handover_command.cipher_mode,
                  msg_sho->handover_command.a5_algorithm,
                  msg_sho->fn_offset,
                  msg_sho->time_alignmt,
                  msg_sho->amr_configuration.noise_suppression_bit,
                  msg_sho->amr_configuration.initial_codec_mode_indicator,
                  msg_sho->amr_configuration.initial_codec_mode,
                  msg_sho->amr_configuration.active_codec_set,
                  msg_sho->amr_configuration.threshold[0],
                  msg_sho->amr_configuration.threshold[1],
                  msg_sho->amr_configuration.threshold[2],
                  msg_sho->amr_configuration.hysteresis[0],
                  msg_sho->amr_configuration.hysteresis[1],
                  msg_sho->amr_configuration.hysteresis[2]);
        #else
          vsi_o_event_ttrace("SHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld",
                  l1s.actual_time.fn_mod42432,
                  msg_sho->handover_command.cell_description.ncc,
                  msg_sho->handover_command.cell_description.bcc,
                  msg_sho->handover_command.cell_description.bcch_carrier,
                  msg_sho->handover_command.channel_desc_1.chan_sel.h,
                  msg_sho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_sho->handover_command.channel_desc_1.channel_type,
                  msg_sho->handover_command.channel_desc_1.subchannel,
                  msg_sho->handover_command.channel_desc_1.timeslot_no,
                  msg_sho->handover_command.channel_desc_1.tsc,
                  msg_sho->handover_command.channel_mode_1,
                  msg_sho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_sho->handover_command.starting_time),
                  msg_sho->handover_command.ho_acc,
                  msg_sho->handover_command.txpwr,
                  msg_sho->handover_command.report_time_diff,
                  msg_sho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_sho->handover_command.cipher_mode,
                  msg_sho->handover_command.a5_algorithm,
                  msg_sho->fn_offset,
                  msg_sho->time_alignmt);
        #endif //(AMR == 1)

#else
        #if (AMR == 1)
          sprintf(str,"SHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld %1d\n\r%1d %1d %2x %2d %2d %2d %2d %2d %2d\n\r",
                  l1s.actual_time.fn_mod42432,
                  msg_sho->handover_command.cell_description.ncc,
                  msg_sho->handover_command.cell_description.bcc,
                  msg_sho->handover_command.cell_description.bcch_carrier,
                  msg_sho->handover_command.channel_desc_1.chan_sel.h,
                  msg_sho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_sho->handover_command.channel_desc_1.channel_type,
                  msg_sho->handover_command.channel_desc_1.subchannel,
                  msg_sho->handover_command.channel_desc_1.timeslot_no,
                  msg_sho->handover_command.channel_desc_1.tsc,
                  msg_sho->handover_command.channel_mode_1,
                  msg_sho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_sho->handover_command.starting_time),
                  msg_sho->handover_command.ho_acc,
                  msg_sho->handover_command.txpwr,
                  msg_sho->handover_command.report_time_diff,
                  msg_sho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_sho->handover_command.cipher_mode,
                  msg_sho->handover_command.a5_algorithm,
                  msg_sho->fn_offset,
                  msg_sho->time_alignmt,
                  msg_sho->amr_configuration.noise_suppression_bit,
                  msg_sho->amr_configuration.initial_codec_mode_indicator,
                  msg_sho->amr_configuration.initial_codec_mode,
                  msg_sho->amr_configuration.active_codec_set,
                  msg_sho->amr_configuration.threshold[0],
                  msg_sho->amr_configuration.threshold[1],
                  msg_sho->amr_configuration.threshold[2],
                  msg_sho->amr_configuration.hysteresis[0],
                  msg_sho->amr_configuration.hysteresis[1],
                  msg_sho->amr_configuration.hysteresis[2])
        #if ((REL99 == 1) && (FF_BHO == 1))
                 ,msg_aho->handover_type
#endif
                  ;
#else
          sprintf(str,"SHO_R %ld %d %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d %d %d %ld %ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  msg_sho->handover_command.cell_description.ncc,
                  msg_sho->handover_command.cell_description.bcc,
                  msg_sho->handover_command.cell_description.bcch_carrier,
                  msg_sho->handover_command.channel_desc_1.chan_sel.h,
                  msg_sho->handover_command.channel_desc_1.chan_sel.rf_channel.single_rf.radio_freq,
                  msg_sho->handover_command.channel_desc_1.channel_type,
                  msg_sho->handover_command.channel_desc_1.subchannel,
                  msg_sho->handover_command.channel_desc_1.timeslot_no,
                  msg_sho->handover_command.channel_desc_1.tsc,
                  msg_sho->handover_command.channel_mode_1,
                  msg_sho->handover_command.frequency_list.rf_chan_cnt,
                  l1a_decode_starting_time(msg_sho->handover_command.starting_time),
                  msg_sho->handover_command.ho_acc,
                  msg_sho->handover_command.txpwr,
                  msg_sho->handover_command.report_time_diff,
                  msg_sho->handover_command.frequency_list_bef_sti.rf_chan_cnt,
                  msg_sho->handover_command.cipher_mode,
                  msg_sho->handover_command.a5_algorithm,
                  msg_sho->fn_offset,
                  msg_sho->time_alignmt);
        #endif //(AMR == 1)

        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_SYNC_HO_CON:
      case MPHC_PRE_SYNC_HO_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("SHO_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"SHO_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_HANDOVER_FINISHED:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("HOF_I %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->cause);
#else
#if ((REL99 == 1) && (FF_BHO == 1))
        sprintf(str,"HOF_I %ld %d %ld %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->cause,
                ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->fn_offset,
                ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->time_alignment);
#else
        sprintf(str,"HOF_I %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->cause);
        L1_send_trace_cpy(str);
#endif
#endif
      }
      break;

      case MPHC_TA_FAIL_IND:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TAF_I %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "TAF_I %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Handover failure

      case MPHC_HANDOVER_FAIL_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("HOF_R %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"HOF_R %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_HANDOVER_FAIL_CON:
      {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("HOF_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"HOF_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////////
      // Measurements //
      //////////////////

      // Cell selection / PLMN selection / Extended measurements

      case MPHC_RXLEV_REQ:
      {
        trace_info.rxlev_req_count ++;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
		  vsi_o_event_ttrace("RXL_R %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_REQ *)(msg->SigP))->power_array_size);

#else
        sprintf(str,"RXL_R %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_REQ *)(msg->SigP))->power_array_size);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_RXLEV_REQ:
      {
        trace_info.rxlev_req_count = 0;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
		  vsi_o_event_ttrace("RXL_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"RXL_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif

      }
      break;

      case L1C_VALID_MEAS_INFO:
      {
        #define MAX_MEAS 10

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
		  vsi_o_event_ttrace("RXL_I %ld %d %d",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.full_list_ptr->power_array_size,
                trace_info.rxlev_req_count);

#else
        sprintf(str,"RXL_I %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.full_list_ptr->power_array_size,
                trace_info.rxlev_req_count);
          L1_send_trace_cpy(str);
#endif


        // If it's the 5th MPHC_RXLEV_REQ message received from L3, trace accum_power_result
        if (trace_info.rxlev_req_count == 5)
        {
          UWORD16 i;

          trace_info.rxlev_req_count = 0;

          if (l1a_l1s_com.full_list_ptr->power_array_size > MAX_MEAS)
          {
            UWORD8  nbmeas = 0;

            i = 0;

            // Trace the MAX_MEAS first carriers those measured RXLEV is > 100
            while ((i < l1a_l1s_com.full_list_ptr->power_array_size) && (nbmeas < MAX_MEAS))
            {
              if (l1a_l1s_com.full_list_ptr->power_array[i].accum_power_result > 100)
              {
                nbmeas ++;

                sprintf(str2,"(%d %d)",
                        l1a_l1s_com.full_list_ptr->power_array[i].radio_freq,
                        l1a_l1s_com.full_list_ptr->power_array[i].accum_power_result);
                strcat(str,str2);
              }

              i ++;
            } // End while
          }
          else
          {
            // Trace all measurements
            for (i=0;i<l1a_l1s_com.full_list_ptr->power_array_size;i++)
            {
              sprintf(str2,"(%d %d)",
                      l1a_l1s_com.full_list_ptr->power_array[i].radio_freq,
                      l1a_l1s_com.full_list_ptr->power_array[i].accum_power_result);
              strcat(str, str2);
            }
          }

          strcat(str,"\n\r");

      } // End if "rxlev_req_count = 5"

        L1_send_trace_cpy(str);
      }
      break;

      // Idle mode BA list

      case MPHC_RXLEV_PERIODIC_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("IBA_R %ld %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->num_of_chans,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->ba_id,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->next_radio_freq_measured);
#else
        sprintf(str,"IBA_R %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->num_of_chans,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->ba_id,
                ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->next_radio_freq_measured);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHC_STOP_RXLEV_PERIODIC_REQ:
      {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("IBA_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"IBA_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1C_RXLEV_PERIODIC_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
vsi_o_event_ttrace("IBA_I %ld %d %d %d %d %d (%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->ba_id,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->s_rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_acc_idle,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_nbr_meas_idle,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->nbr_of_carriers,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].rxlev);
#else
sprintf(str,"IBA_I %ld %d %d %d %d %d (%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)(%d %d)\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->ba_id,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->s_rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_acc_idle,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->qual_nbr_meas_idle,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->nbr_of_carriers,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[0].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[1].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[2].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[3].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[4].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[5].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[6].rxlev,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].radio_freq_no,
                ((T_MPHC_RXLEV_PERIODIC_IND *)(msg->SigP))->A[7].rxlev);
        L1_send_trace_cpy(str);
#endif
      }
      break;


      // Dedicated mode BA list

      case MPHC_MEAS_REPORT:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
     #if REL99
     #if FF_EMR
        vsi_o_event_ttrace("DBA_I_1 %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) EM(%d %d %ld %d %d %d %d) U%d D%d DF%d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->meas_valid,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->txpwr_used,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->timing_advance,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_num,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_num,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->nbr_rcvd_blocks,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_ul_count,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_count,
                trace_info.facch_dl_fail_count_trace);

        vsi_o_event_ttrace("DBA_I_2 (%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)",
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_nbr_meas);

	 #endif
	 #else
        vsi_o_event_ttrace("DBA_I_1 %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) U%d D%d DF%d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->meas_valid,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->txpwr_used,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->timing_advance,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_ul_count,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_count,
                trace_info.facch_dl_fail_count_trace);

        vsi_o_event_ttrace("DBA_I_2 (%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)",
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_nbr_meas);
     #endif
#else
     #if REL99
     #if FF_EMR
	  #if (FF_REPEATED_DL_FACCH == 1 )
	   sprintf(str,"DBA_I %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) EM(%d %d %ld %d %d %d %d) U%d D%d(CC%d R%d ) DF%d\n\r(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)\n\r",
	  #else
          sprintf(str,"DBA_I %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) EM(%d %d %ld %d %d %d %d) U%d D%d DF%d\n\r(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)\n\r",
         #endif
                  l1s.actual_time.fn_mod42432,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->meas_valid,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->txpwr_used,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->timing_advance,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_acc_errors,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_nbr_bits,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_acc_errors,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_nbr_bits,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_val_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->mean_bep_block_num,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->cv_bep_block_num,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->nbr_rcvd_blocks,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_ul_count,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_count,
                #if (FF_REPEATED_DL_FACCH == 1 )
		        ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_combined_good_count,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_repetition_block_count,
	         #endif
                  trace_info.facch_dl_fail_count_trace,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_nbr_meas,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].bcch_freq,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_acc,
                  ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_nbr_meas);

     #endif
#else
     #if (FF_REPEATED_DL_FACCH == 1 )
        sprintf(str,"DBA_I %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) U%d D%d (CC%d R%d ) DF%d\n\r(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)\n\r",
#else
        sprintf(str,"DBA_I %ld %d %d %d L(%d %d)(%d %d) Q(%d %d)(%d %d) U%d D%d DF%d\n\r(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)(%d %d %d)\n\r",
     #endif
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->meas_valid,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->txpwr_used,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->timing_advance,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_full_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxlev_sub_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_full_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_acc_errors,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->rxqual_sub_nbr_bits,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_ul_count,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_count,
                #if (FF_REPEATED_DL_FACCH == 1 )
		        ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_combined_good_count,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->facch_dl_repetition_block_count,
	         #endif
                trace_info.facch_dl_fail_count_trace,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[0].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[1].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[2].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[3].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[4].rxlev_nbr_meas,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].bcch_freq,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_acc,
                ((T_MPHC_MEAS_REPORT *)(msg->SigP))->ncell_meas.A[5].rxlev_nbr_meas);
      #endif
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Update BA list

      case MPHC_UPDATE_BA_LIST:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BAU_R %ld %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->num_of_chans,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->pwrc,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->dtx_allowed,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->ba_id);
#else
        sprintf(str,"BAU_R %ld %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->num_of_chans,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->pwrc,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->dtx_allowed,
                ((T_MPHC_UPDATE_BA_LIST *)(msg->SigP))->ba_id);
        L1_send_trace_cpy(str);
#endif
      }
      break;

    #if L1_GPRS

      /********************************************************************************/
      /* PACKET SWITCHED                                                              */
      /********************************************************************************/

      //////////////////////////////
      // Neighbor cell monitoring //
      //////////////////////////////

      // Neighbor PBCCH reading

      case MPHP_NCELL_PBCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PBN_R %ld %d %d %d %d %d %d %d %d %d %d %ld %ld",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bs_pbcch_blks,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->pb,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->psi1_repeat_period,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->relative_position,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bcch_carrier,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->time_alignment);
#else
        sprintf(str,"PBN_R %ld %d %d %d %d %d %d %d %d %d %d %ld %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bs_pbcch_blks,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->pb,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->psi1_repeat_period,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->relative_position,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->bcch_carrier,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->fn_offset,
                ((T_MPHP_NCELL_PBCCH_REQ *)(msg->SigP))->time_alignment);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_NCELL_PBCCH_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PBN_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"PBN_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PBCCHN_INFO:
      {
#if(L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
        operative_radio_freq = l1_multiband_radio_freq_convert_into_operative_radio_freq(((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq);
#endif
        
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PBN_I %ld %d %d %d %d %ld %d",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[l1pa_l1ps_com.pbcchn.bcch_carrier -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"PBN_I %ld %d %d %d %d %ld %d\n\r",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
#if(L1_FF_MULTIBAND == 0)
                l1a_l1s_com.last_input_level[l1pa_l1ps_com.pbcchn.bcch_carrier -
                                             l1_config.std.radio_freq_index_offset].input_level,
#else
                l1a_l1s_com.last_input_level[operative_radio_freq].input_level,
#endif

                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////////////////////////////////////////////
      // Serving cell normal burst reading in packet idle //
      //////////////////////////////////////////////////////

      // PCCCH reading

      case MPHP_START_PCCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PCC_R %ld %d %d %d %d %d %d %d %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->imsimod,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->kcn,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->split_pg_cycle,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pag_blks_res,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pbcch_blks,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->pb,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->page_mode,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt);
#else
        sprintf(str,"PCC_R %ld %d %d %d %d %d %d %d %d %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->imsimod,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->kcn,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->split_pg_cycle,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pag_blks_res,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->bs_pbcch_blks,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->pb,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->page_mode,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc,
                ((T_MPHP_START_PCCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_STOP_PCCCH_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PCC_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"PCC_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PNP_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
#if (FF_L1_FAST_DECODING == 1)
        vsi_o_event_ttrace("PNP_I %ld %d %d %d %d %ld %d %d %c",
#else
        vsi_o_event_ttrace("PNP_I %ld %d %d %d %d %ld %d %c",
#endif
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc,
#if (FF_L1_FAST_DECODING == 1)
                l1a_l1s_com.last_fast_decoding,
#endif
                trace_info.l1_memorize_error);
#else
#if (FF_L1_FAST_DECODING == 1)
        sprintf(str,"PNP_I %ld %d %d %d %d %ld %d %d %c\n\r",
#else
        sprintf(str,"PNP_I %ld %d %d %d %d %ld %d %c\n\r",
#endif
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc,
#if (FF_L1_FAST_DECODING == 1)
                l1a_l1s_com.last_fast_decoding,
#endif
                trace_info.l1_memorize_error);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PEP_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PEP_I %ld %d %d %d %d %ld %d",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"PEP_I %ld %d %d %d %d %ld %d\n\r",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PALLC_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PCC_I %ld %d %d %d %d %ld %d",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"PCC_I %ld %d %d %d %d %ld %d\n\r",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // PBCCH reading

      case MPHP_SCELL_PBCCH_REQ:
      {
        UWORD8 i;
        sprintf(str,"PBS_R %ld %d %d %d %d %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->nbr_psi,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->bs_pbcch_blks,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->pb,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->psi1_repeat_period,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.h,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.timeslot_no,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->packet_chn_desc.tsc,
                ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->frequency_list.rf_chan_cnt);

        // Trace relative_position array
        for(i=0;i<((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->nbr_psi;i++)
        {
          sprintf(str2,"%d ",
                  ((T_MPHP_SCELL_PBCCH_REQ *)(msg->SigP))->relative_position_array[i]);
          strcat(str,str2);
        }

        strcat(str,"\n\r");
        L1_send_trace_cpy(str);
      }
      break;

      case MPHP_SCELL_PBCCH_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PBS_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"PBS_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PBCCHS_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PBS_I %ld %d %d %d %d %ld %d",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"PBS_I %ld %d %d %d %d %ld %d\n\r",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      ///////////////////
      // Packet Access //
      ///////////////////

      // Random access

      case MPHP_RA_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PRA_R %ld %d %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_RA_REQ *)(msg->SigP))->txpwr,
                ((T_MPHP_RA_REQ *)(msg->SigP))->rand,
                ((T_MPHP_RA_REQ *)(msg->SigP))->channel_request_data,
                ((T_MPHP_RA_REQ *)(msg->SigP))->bs_prach_blks,
                ((T_MPHP_RA_REQ *)(msg->SigP))->access_burst_type);
#else
        sprintf(str,"PRA_R %ld %d %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_RA_REQ *)(msg->SigP))->txpwr,
                ((T_MPHP_RA_REQ *)(msg->SigP))->rand,
                ((T_MPHP_RA_REQ *)(msg->SigP))->channel_request_data,
                ((T_MPHP_RA_REQ *)(msg->SigP))->bs_prach_blks,
                ((T_MPHP_RA_REQ *)(msg->SigP))->access_burst_type);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_RA_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PRA_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"PRA_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_RA_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PRA_C %ld %d",
                ((T_MPHP_RA_CON *)(msg->SigP))->fn % 42432,
                ((T_MPHP_RA_CON *)(msg->SigP))->channel_request_data);
#else
        sprintf(str,"PRA_C %ld %d\n\r",
                ((T_MPHP_RA_CON *)(msg->SigP))->fn % 42432,
                ((T_MPHP_RA_CON *)(msg->SigP))->channel_request_data);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Single block

      case MPHP_SINGLE_BLOCK_REQ:
      {
        WORD32 sti;

        if (((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tbf_sti.present == 0)
          sti = -1;
        else
          sti = (WORD32) ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tbf_sti.absolute_fn;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BLK_R %ld %d %d %d %d %d (%d %d %d) %d %d %d %d %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->purpose,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->pc_meas_chan,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->access_burst_type,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->packet_ta.ta,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.p0,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.bts_pwr_ctl_mode,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.pr_mode,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tsc,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.h,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.freq_list.rf_chan_cnt,
                sti,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->timeslot_number);
#else
        sprintf(str,"BLK_R %ld %d %d %d %d %d (%d %d %d) %d %d %d %d %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->purpose,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->pc_meas_chan,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->access_burst_type,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->packet_ta.ta,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.p0,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.bts_pwr_ctl_mode,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->dl_pwr_ctl.pr_mode,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->tsc,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.h,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->freq_param.freq_list.rf_chan_cnt,
                sti,
                ((T_MPHP_SINGLE_BLOCK_REQ *)(msg->SigP))->timeslot_number);
#endif
        L1_send_trace_cpy(str);
      }
      break;

      case MPHP_STOP_SINGLE_BLOCK_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BLK_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"BLK_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PACCH_INFO:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BLK_I %ld %d %d %d %d %ld %d",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
#else
        sprintf(str,"BLK_I %ld %d %d %d %d %ld %d\n\r",
                ((T_MPHP_DATA_IND *)(msg->SigP))->fn % 42432,
                ((T_MPHP_DATA_IND *)(msg->SigP))->error_flag,
                ((T_MPHP_DATA_IND *)(msg->SigP))->radio_freq,
                ((T_MPHP_DATA_IND *)(msg->SigP))->relative_position,
                l1a_l1s_com.Scell_IL_for_rxlev,
                l1s.tpu_offset,
                l1s.afc);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_SINGLE_BLOCK_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("BLK_C %ld %d %d %d %d  %d %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->assignment_id,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->dl_error_flag,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->purpose,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->status,
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[0],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[1],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[2],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[3]);
#else
        sprintf(str,"BLK_C %ld %d %d %d %d  %d %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->assignment_id,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->dl_error_flag,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->purpose,
                ((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->status,
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[0],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[1],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[2],
                l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[3]);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Idle mode polling

      case MPHP_POLLING_RESPONSE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("POL_R %ld %d %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->pol_resp_type,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->fn,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->timing_advance,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->txpwr);
#else
        sprintf(str,"POL_R %ld %d %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->pol_resp_type,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->fn,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->timing_advance,
                ((T_MPHP_POLLING_RESPONSE_REQ *)(msg->SigP))->txpwr);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_POLL_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("POL_I %ld",
                ((T_MPHP_POLLING_IND *)(msg->SigP))->fn % 42432);
#else
        sprintf(str,"POL_I %ld\n\r",
                ((T_MPHP_POLLING_IND *)(msg->SigP))->fn % 42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////////////////
      // Packet transfer mode //
      //////////////////////////

      // Temporary block flow assignment

      case MPHP_ASSIGNMENT_REQ:
      {
        WORD32 sti;

        if (((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tbf_sti.present == 0)
          sti = -1;
        else
          sti = (WORD32) ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tbf_sti.absolute_fn;
        sprintf(str,"\n\rTBF_R %ld %d %d %d %d %d %d (%d %d %d)(%d %d %d) %d %d %d %d %ld | %d %x %x",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->assignment_command,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->multislot_class,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->interf_meas_enable,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->pc_meas_chan,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->access_burst_type,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta_index,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->packet_ta.ta_tn,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.p0,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.bts_pwr_ctl_mode,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_pwr_ctl.pr_mode,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->tsc,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.chan_sel.h,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->freq_param.freq_list.rf_chan_cnt,
                sti,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->mac_mode,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->dl_ressource_alloc.timeslot_alloc,
                ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.timeslot_alloc);

        if (((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->mac_mode == DYN_ALLOC)
          sprintf(str2," %d\n\r",
                  ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.dynamic_alloc.usf_granularity);
        else
          sprintf(str2," %d %d\n\r",
                  ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.fixed_alloc.ctrl_timeslot,
                  ((T_MPHP_ASSIGNMENT_REQ *)(msg->SigP))->ul_ressource_alloc.fixed_alloc.bitmap_length);

        strcat(str,str2);
        L1_send_trace_cpy(str);

        // a message was received correctly from the Network, it means the system behavior is nice
        // and so we can allow more trace on a PM/COM error.
        trace_info.trace_filter = FALSE;
      }
      break;

      case MPHP_ASSIGNMENT_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TBF_C %ld %d",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
#else
        sprintf(str,"TBF_C %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_TBF_RELEASE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("REL_R %ld %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TBF_RELEASE_REQ *)(msg->SigP))->tbf_type);
#else
        sprintf(str,"REL_R %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TBF_RELEASE_REQ *)(msg->SigP))->tbf_type);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_TBF_RELEASED:
      {
        if (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all == 1)
          trace_info.new_tcr_list = 0;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("REL_C %ld %d %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all,
                ((T_L1P_TBF_RELEASED *)(msg->SigP))->tbf_type,
                l1a_l1s_com.dl_tn);
#else
        sprintf(str,"REL_C %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all,
                ((T_L1P_TBF_RELEASED *)(msg->SigP))->tbf_type,
                l1a_l1s_com.dl_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // PDCH release

      case MPHP_PDCH_RELEASE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CHR_R %ld %d %x",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->timeslot_available);
#else
        sprintf(str,"CHR_R %ld %d %x\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_PDCH_RELEASE_REQ *)(msg->SigP))->timeslot_available);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_PDCH_RELEASED:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CHR_C %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_PDCH_RELEASE_CON *)(msg->SigP))->assignment_id,
                l1a_l1s_com.dl_tn);
#else
        sprintf(str,"CHR_C %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_PDCH_RELEASE_CON *)(msg->SigP))->assignment_id,
                l1a_l1s_com.dl_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Timing advance update

      case MPHP_TIMING_ADVANCE_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TAU_R %ld %d (%d %d %d)",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_index,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_tn);
#else
        sprintf(str,"TAU_R %ld %d (%d %d %d)\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->assignment_id,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_index,
                ((T_MPHP_TIMING_ADVANCE_REQ *)(msg->SigP))->packet_ta.ta_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_TA_CONFIG_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TAU_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"TAU_C %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Update PSI parameters

      case MPHP_UPDATE_PSI_PARAM_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("PSI_R %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->pb,
                ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->access_burst_type);
#else
        sprintf(str,"PSI_R %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->pb,
                ((T_MPHP_UPDATE_PSI_PARAM_REQ *)(msg->SigP))->access_burst_type);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Fixed mode repeat allocation

      case MPHP_REPEAT_UL_FIXED_ALLOC_REQ:
      {
        WORD32 sti;

        if (((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->tbf_sti.present == 0)
          sti = -1;
        else
          sti = (WORD32) ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->tbf_sti.absolute_fn;
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("REP_R %ld %d %x %ld",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->repeat_allocation,
                ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->ts_override,
                sti);
#else
        sprintf(str,"REP_R %ld %d %x %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->repeat_allocation,
                ((T_MPHP_REPEAT_UL_FIXED_ALLOC_REQ *)(msg->SigP))->ts_override,
                sti);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_REPEAT_ALLOC_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("REP_C %ld %d",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
#else
        sprintf(str,"REP_C %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Fixed mode allocation exhaustion

      case L1P_ALLOC_EXHAUST_DONE:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("EXH_I %ld %d",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
#else
        sprintf(str,"EXH_I %ld %d\n\r",
                l1s.actual_time.fn_mod42432,
                l1a_l1s_com.dl_tn);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      //////////////////////////////
      // Packet mode measurements //
      //////////////////////////////

      // BA list measurements in packet idle

      case MPHP_CR_MEAS_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CR_R  %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->nb_carrier,
                ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->list_id);
#else
        sprintf(str,"CR_R  %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->nb_carrier,
                ((T_MPHP_CR_MEAS_REQ *)(msg->SigP))->list_id);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_CR_MEAS_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CR_S  %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"CR_S  %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);

#endif
      }
      break;

      case L1P_CR_MEAS_DONE:
      {
        #define MAX_CR 20

        UWORD8 i,nmeas;

        sprintf(str,"CR_I  %ld %d %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->nmeas,
                ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->list_id,
                ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->reporting_period);

        nmeas = ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->nmeas;
        if (nmeas > MAX_CR) nmeas = MAX_CR;

        for (i=0;i<nmeas;i++)
        {
          sprintf (str2,
                   "(%d %d)",
                   l1pa_l1ps_com.cres_freq_list.alist->freq_list[i],
                   ((T_L1P_CR_MEAS_DONE *)(msg->SigP))->ncell_meas[i].rxlev);
          strcat(str,str2);
        }

        strcat(str,"\n\r");
        L1_send_trace_cpy(str);
      }
      break;

      // BA list measurements in packet transfer

      case MPHP_TCR_MEAS_REQ:
      {
        trace_info.new_tcr_list = 1;
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TCR_R %ld %d %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->nb_carrier,
                ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->list_id);
#else
        sprintf(str,"TCR_R %ld %d %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->nb_carrier,
                ((T_MPHP_TCR_MEAS_REQ *)(msg->SigP))->list_id);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_TCR_MEAS_STOP_REQ:
      {
        trace_info.new_tcr_list = 0;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TCR_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"TCR_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_TCR_MEAS_DONE:
      {
        #define MAX_TCR 10

        UWORD8            i, nmeas;
        T_CRES_LIST_PARAM *list_ptr;

        if (trace_info.new_tcr_list == 0)
          // No TCR list update: keep the alist pointer
          list_ptr = l1pa_l1ps_com.cres_freq_list.alist;
        else
        {
          // In case of TCR list updating, the alist pointer has changed
          if(l1pa_l1ps_com.cres_freq_list.alist == &(l1pa_l1ps_com.cres_freq_list.list[0]))
            list_ptr = &(l1pa_l1ps_com.cres_freq_list.list[1]);
          else
            list_ptr = &(l1pa_l1ps_com.cres_freq_list.list[0]);
          //Reset the variable new_tcr_list so that next time onwards the new list of
          //frequencies will get printed.
          trace_info.new_tcr_list = 0;
        }

        sprintf(str,"TCR_I %ld %d %d %ld\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->list_id,
                list_ptr->nb_carrier,
                l1s.tpu_offset);

        nmeas = list_ptr->nb_carrier;
        if (nmeas > MAX_TCR) nmeas = MAX_TCR;

        for (i=0;i<nmeas;i++)
        {
          sprintf (str2,
                   "(%d %d %d)",
                   list_ptr->freq_list[i],
                   ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->acc_level[i],
                   ((T_L1P_TCR_MEAS_DONE *)(msg->SigP))->acc_nbmeas[i]);
          strcat(str,str2);
        }

        strcat(str,"\n\r");
        L1_send_trace_cpy(str);
      }
      break;

      ///////////////////////////////
      // Interference measurements //
      ///////////////////////////////

      // Interference measurements in packet idle

      case MPHP_INT_MEAS_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ITM_R %ld %d %d %d %x %d",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.h,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.freq_list.rf_chan_cnt,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->tn,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->multislot_class);
#else
        sprintf(str,"ITM_R %ld %d %d %d %x %d\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.h,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.chan_sel.rf_channel.single_rf.radio_freq,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->packet_intm_freq_param.freq_list.rf_chan_cnt,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->tn,
                ((T_MPHP_INT_MEAS_REQ *)(msg->SigP))->multislot_class);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_INT_MEAS_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ITM_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str,"ITM_S %ld\n\r",
                l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case L1P_ITMEAS_IND:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("IT_I  %ld %d %x",
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->fn % 42432,
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->position,
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->meas_bitmap);
#else
        sprintf(str,"IT_I  %ld %d %x\n\r",
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->fn % 42432,
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->position,
                ((T_L1P_ITMEAS_IND *)(msg->SigP))->meas_bitmap);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MPHP_INT_MEAS_IND:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ITM_I %ld (%d %d %d %d %d %d %d %d)(%d %d %d %d %d %d %d %d)",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1]);
#else
        sprintf(str,"ITM_I %ld (%d %d %d %d %d %d %d %d)(%d %d %d %d %d %d %d %d)\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1]);
        L1_send_trace_cpy(str);
#endif
      }
      break;

      // Interference measurements in packet transfer

      case MPHP_TINT_MEAS_IND:
      {




#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TIT_I %ld (%d %d %d %d %d %d %d %d)(%d %d %d %d %d %d %d %d)",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1]);
#else
        sprintf(str,"TIT_I %ld (%d %d %d %d %d %d %d %d)(%d %d %d %d %d %d %d %d)\n\r",
                l1s.actual_time.fn_mod42432,
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[0],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[0].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[1].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[2].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[3].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[4].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[5].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[6].rxlev[1],
                ((T_MPHP_INT_MEAS_IND *)(msg->SigP))->int_meas[7].rxlev[1]);
        L1_send_trace_cpy(str);
#endif
      }
      break;

    #endif //L1_GPRS

      /********************************************************************************/
      /* BACKGROUND TASKS                                                             */
      /********************************************************************************/

      //////////////////
      // MMI messages //
      //////////////////


    #if (AUDIO_TASK == 1)
    // Messages for the new audio design

      #if (KEYBEEP)
        // Keybeep
        case MMI_KEYBEEP_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("KBP_R %ld %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt0,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt1,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_dur_kb);
#else
          sprintf(str,"KBP_R %ld %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt0,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt1,
                  ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_dur_kb);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_KEYBEEP_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("KBP_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"KBP_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_KEYBEEP_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("KBP_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"KBP_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_KEYBEEP_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("KBP_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"KBP_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // KEYBEEP

      #if (TONE)
        // Tone
        case MMI_TONE_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TON_R %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t0,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t1,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t2,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_rep,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_se_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_bu_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_off);
#else
          sprintf(str,"TON_R %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t0,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t1,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t2,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_rep,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_se_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_bu_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_off,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_on,
                  ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_off);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_TONE_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TON_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"TON_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
       break;

        case MMI_TONE_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TON_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"TON_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_TONE_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("TON_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"TON_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // TONE

      #if (MELODY_E1)
        // Melody 0
        case MMI_MELODY0_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME0_R %ld %02lx %01lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap);
#else
          sprintf(str,"ME0_R %ld %02lx %01lx %04lx \n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME0_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME0_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME0_S %ld",
                 l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME0_S %ld\n\r",
                 l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME0_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME0_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

              // Melody 1
        case MMI_MELODY1_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME1_R %ld %02lx %01lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap);
#else
          sprintf(str,"ME1_R %ld %02lx %01lx %04lx \n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME1_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME1_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME1_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME1_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("ME1_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ME1_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // MELODY_E1

      #if (VOICE_MEMO)
        // Voice memo recording
        case MMI_VM_RECORD_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMR_R %ld %02lx %08lx %01lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->maximum_size,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->dtx_used,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_dl,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_ul,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t0,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t1,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t2,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_rep,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_se_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_bu_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_off);
#else
          sprintf(str,"VMR_R %ld %02lx %08lx %01lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->maximum_size,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->dtx_used,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_dl,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_ul,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t0,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t1,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t2,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_rep,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_se_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_bu_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_off,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_on,
                  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_off);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_RECORD_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMR_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_RECORD_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMR_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMR_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_RECORD_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMR_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMR_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        // Voice memo playing
        case MMI_VM_PLAY_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMP_R %ld %02lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_PLAY_REQ *)(msg->SigP))->session_id);
#else
          sprintf(str,"VMP_R %ld %02lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_PLAY_REQ *)(msg->SigP))->session_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_PLAY_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMP_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMP_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_PLAY_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMP_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMP_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_PLAY_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMP_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMP_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // VOICE_MEMO

      #if (L1_PCM_EXTRACTION)
      /* PCM download*/
        case MMI_PCM_DOWNLOAD_START_REQ:
        {
          sprintf(str,"PDL_R %ld %02lx %02lx %02lx %lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_ul_gain,
                  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_dl_gain,
                  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->maximum_size);
          L1_send_trace_cpy(str);

        }
        break;
        case MMI_PCM_DOWNLOAD_START_CON:
        {
          sprintf(str,"PDL_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
        }
        break;
        case MMI_PCM_DOWNLOAD_STOP_REQ:
        {
          sprintf(str,"PDL_S %ld %lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_PCM_DOWNLOAD_STOP_REQ *)(msg->SigP))->maximum_size);
          L1_send_trace_cpy(str);
        }
        break;
        case MMI_PCM_DOWNLOAD_STOP_CON:
        {
          sprintf(str,"PDL_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
        }
        break;
        case MMI_PCM_UPLOAD_START_REQ:
        {
          sprintf(str,"PUL_R %ld %02lx %02lx %02lx %lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_ul_gain,
                  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_dl_gain,
                  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->maximum_size);
          L1_send_trace_cpy(str);

        }
        break;
        case MMI_PCM_UPLOAD_START_CON:
        {
          sprintf(str,"PUL_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
        }
        break;
        case MMI_PCM_UPLOAD_STOP_REQ:
        {
          sprintf(str,"PUL_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
        }
        break;
        case MMI_PCM_UPLOAD_STOP_CON:
        {
          sprintf(str,"PUL_E %ld %lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_PCM_UPLOAD_STOP_CON *)(msg->SigP))->uploaded_size);
          L1_send_trace_cpy(str);
        }
        break;
      #endif  /* L1_PCM_EXTRACTION */

      #if (L1_VOICE_MEMO_AMR)
        // Voice memo recording
        case MMI_VM_AMR_RECORD_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAR_R %ld %02lx %08lx %01lx %04lx %02lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->maximum_size,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->dtx_used,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->record_coeff_ul,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->amr_vocoder);
#else
          sprintf(str,"VMAR_R %ld %02lx %08lx %01lx %04lx %02lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->maximum_size,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->dtx_used,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->record_coeff_ul,
                  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->amr_vocoder);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_RECORD_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAR_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_RECORD_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAR_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAR_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_RECORD_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAR_E %ld %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_RECORD_CON *)(msg->SigP))->recorded_size);
#else
          sprintf(str,"VMAR_E %ld %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_RECORD_CON *)(msg->SigP))->recorded_size);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        // Voice memo playing
        case MMI_VM_AMR_PLAY_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAP_R %ld %02lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_PLAY_REQ *)(msg->SigP))->session_id);
#else
          sprintf(str,"VMAP_R %ld %02lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_VM_AMR_PLAY_REQ *)(msg->SigP))->session_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_PLAY_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAP_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAP_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_PLAY_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAP_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAP_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_VM_AMR_PLAY_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAP_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAP_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;


// Voice memo playing
        case MMI_VM_AMR_PAUSE_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAPP_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"VMAPP_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

 case MMI_VM_AMR_PAUSE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAPP_C %ld ",
                  l1s.actual_time.fn_mod42432
                 );
#else
          sprintf(str,"VMAPP_C %ld\n\r",
                  l1s.actual_time.fn_mod42432
                  );
          L1_send_trace_cpy(str);
#endif
        }
        break;

 case MMI_VM_AMR_RESUME_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAPR_R %ld ",
                  l1s.actual_time.fn_mod42432
                 );
#else
          sprintf(str,"VMAPR_R %ld \n\r",
                  l1s.actual_time.fn_mod42432
               );
          L1_send_trace_cpy(str);
#endif
        }
        break;
 case MMI_VM_AMR_RESUME_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("VMAPR_C %ld ",
                  l1s.actual_time.fn_mod42432
                  );
#else
          sprintf(str,"VMAPR_C %ld \n\r",
                  l1s.actual_time.fn_mod42432
                  );
          L1_send_trace_cpy(str);
#endif
        }
        break;


      #endif // L1_VOICE_MEMO_AMR

      #if (OP_RIV_AUDIO == 1)
        #if (L1_AUDIO_DRIVER == 1)
          case L1_AUDIO_DRIVER_IND:
          {
		  	#if 0
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("AUD_I %ld %02ld %08lx",
                    l1s.actual_time.fn_mod42432,
                    ((T_L1_AUDIO_DRIVER_IND *)(msg->SigP))->channel_id,
                    ((T_L1_AUDIO_DRIVER_IND *)(msg->SigP))->p_buffer);
#else
            sprintf(str,"AUD_I %ld %02ld %08lx\n\r",
                   (WORD32)  l1s.actual_time.fn_mod42432,
                 (WORD32)   ((T_L1_AUDIO_DRIVER_IND *)(msg->SigP))->channel_id,
                    (WORD32)((T_L1_AUDIO_DRIVER_IND *)(msg->SigP))->p_buffer);
            L1_send_trace_cpy(str);
#endif             
          #endif  //endof aud_i
          }
          break;
        #endif //(L1_AUDIO_DRIVER == 1)
     #endif //(OP_RIV_AUDIO == 1)

      #if (SPEECH_RECO)
        case MMI_SR_ENROLL_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRE_R %ld %02ld %02ld %01ld %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech_address);
#else
          sprintf(str,"SRE_R %ld %02ld %02ld %01ld %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech_address);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_ENROLL_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRE_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRE_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_ENROLL_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRE_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRE_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_ENROLL_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRE_E %ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id);
#else
          sprintf(str,"SRE_E %ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRU_R %ld %02ld %02ld %01ld %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech_address);
#else
          sprintf(str,"SRU_R %ld %02ld %02ld %01ld %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech_address);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRU_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRU_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRU_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRU_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRU_E %ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id);
#else
          sprintf(str,"SRU_E %ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_RECO_START_REQ:
        {

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRR_R %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_RECO_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_RECO_REQ *)(msg->SigP))->vocabulary_size);
#else
          sprintf(str,"SRR_R %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_RECO_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_RECO_REQ *)(msg->SigP))->vocabulary_size);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_RECO_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRR_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRR_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_RECO_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRR_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_RECO_STOP_CON:
        {
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRR_E %ld %02ld %02ld %08lx %02ld %08lx %02ld %08lx %02ld %08lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->error_id,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_level,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_noise,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_model_size);
#else
          sprintf(str,"SRR_E %ld %02ld %02ld %08lx %02ld %08lx %02ld %08lx %02ld %08lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->error_id,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->second_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->third_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_index,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->fourth_best_word_score,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_level,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_db_noise,
                  ((T_MMI_SR_RECO_STOP_CON *)(msg->SigP))->d_sr_model_size);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_CHECK_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRC_R %ld %02ld %02ld %02ld %08lx %01ld %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->vocabulary_size,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->model_address,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech_address);
#else
          sprintf(str,"SRC_R %ld %02ld %02ld %02ld %08lx %01ld %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->database_id,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->vocabulary_size,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->model_address,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech,
                  ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech_address);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_CHECK_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRC_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRC_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_CHECK_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRC_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRC_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_SR_UPDATE_CHECK_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRC_E %ld %02ld %02ld %08lx %02ld %08lx %02ld %08lx %02ld %08lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->error_id,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_level,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_noise,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_model_size);
#else
          sprintf(str,"SRC_E %ld %02ld %02ld %08lx %02ld %08lx %02ld %08lx %02ld %08lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->error_id,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->second_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->third_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_index,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->fourth_best_word_score,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_level,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_db_noise,
                  ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(msg->SigP))->d_sr_model_size);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_SAVE_DATA_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_SR %ld %02ld %02ld %08lx %01ld %08lx %08lx %08lx %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->database_id,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_index,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->speech,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_buffer,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_buffer,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_address,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_address);
#else
          sprintf(str,"SRB_SR %ld %02ld %02ld %08lx %01ld %08lx %08lx %08lx %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->database_id,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_index,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->speech,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_buffer,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_buffer,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->start_address,
                  ((T_L1_SRBACK_SAVE_DATA_REQ *)(msg->SigP))->stop_address);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_SAVE_DATA_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_SC %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRB_SC %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_LOAD_MODEL_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_LR %ld %02ld %02lx %04lx %01ld",
                 l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->database_id,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_index,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_RAM_address,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->CTO_enable);
#else
          sprintf(str,"SRB_LR %ld %02ld %02lx %04lx %01ld\n\r",
                 l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->database_id,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_index,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->model_RAM_address,
                  ((T_L1_SRBACK_LOAD_MODEL_REQ *)(msg->SigP))->CTO_enable);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_LOAD_MODEL_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_LC %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRB_LC %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_TEMP_SAVE_DATA_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_TR %ld %08lx %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_input,
                  ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_output);
#else
          sprintf(str,"SRB_TR %ld %08lx %08lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_input,
                  ((T_L1_SRBACK_TEMP_SAVE_DATA_REQ *)(msg->SigP))->model_RAM_address_output);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_SRBACK_TEMP_SAVE_DATA_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("SRB_TC %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"SRB_TC %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif  // SPEECH_RECO
      #if (FIR)
        case MMI_AUDIO_FIR_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("FIR_R %ld %01lx %02ld %08lx %08lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_loop,
                  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->update_fir,
                  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_ul_coefficient,
                  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_dl_coefficient);
#else
          sprintf(str,"FIR_R %ld %01lx %02ld %08lx %08lx\n\r",
                 (WORD32)  l1s.actual_time.fn_mod42432,
                 (WORD32)  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_loop,
                 (WORD32)  ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->update_fir,
                  (WORD32) ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_ul_coefficient,
                  (WORD32) ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_dl_coefficient);

          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AUDIO_FIR_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("FIR_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"FIR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif //FIR
      #if (L1_AEC == 1)
        case MMI_AEC_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        #if (L1_NEW_AEC)
          vsi_o_event_ttrace("AEC_R %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->cont_filter,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->granularity_att,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->coef_smooth,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->es_level_max,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_vad,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->thrs_abs,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_fil,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_mut);
        #else
          vsi_o_event_ttrace("AEC_R %ld %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control);
        #endif //(L1_NEW_AEC)
#else
        #if (L1_NEW_AEC)
          sprintf(str,"AEC_R %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->cont_filter,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->granularity_att,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->coef_smooth,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->es_level_max,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_vad,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->thrs_abs,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_fil,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_mut);
        #else
          sprintf(str,"AEC_R %ld %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control);
        #endif //(L1_NEW_AEC)
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AEC_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AEC_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AEC_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        #if (L1_NEW_AEC)
          case L1_AEC_IND:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AEC_I %ld %4.4x %8.8x %8.8x",
                    l1s.actual_time.fn_mod42432,
                    ((T_L1_AEC_IND *)(msg->SigP))->es_level,
                    ((T_L1_AEC_IND *)(msg->SigP))->far_end_pow,
                    ((T_L1_AEC_IND *)(msg->SigP))->far_end_noise);
#else
            sprintf(str,"AEC_I %ld %4.4x %8.8x %8.8x\n\r",
                    l1s.actual_time.fn_mod42432,
                    ((T_L1_AEC_IND *)(msg->SigP))->es_level,
                    ((T_L1_AEC_IND *)(msg->SigP))->far_end_pow,
                    ((T_L1_AEC_IND *)(msg->SigP))->far_end_noise);
            L1_send_trace_cpy(str);
#endif
          }
          break;
        #endif //(L1_NEW_AEC)
      #endif //AEC

      #if (AUDIO_MODE)
        case MMI_AUDIO_MODE_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AUM_R %ld %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AUDIO_MODE *)(msg->SigP))->audio_mode);
#else
          sprintf(str,"AUM_R %ld %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AUDIO_MODE *)(msg->SigP))->audio_mode);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AUDIO_MODE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AUM_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AUM_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // AUDIO_MODE

      #if (MELODY_E2)
        // MMI command and request
        case MMI_MELODY0_E2_START_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E20_R %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback);
#else
          sprintf(str,"E20_R %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_E2_STOP_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E20_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E20_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_E2_START_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E20_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E20_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY0_E2_STOP_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E20_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E20_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_E2_START_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E21_R %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback);
#else
          sprintf(str,"E21_R %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_E2_STOP_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E21_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E21_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_E2_START_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E21_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E21_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MELODY1_E2_STOP_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E21_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E21_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        // Audio download instrument message
        case L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E2B_L %ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[0],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[1],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[2],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[3],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[4],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[5],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[6],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[7]);
#else
          sprintf(str,"E2B_L %ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[0],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[1],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[2],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[3],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[4],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[5],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[6],
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->waves_table_id[7]);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E2B_LC %ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id);
#else
          sprintf(str,"E2B_LC %ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E2B_U %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument);
#else
          sprintf(str,"E2B_U %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->number_of_instrument);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E2B_UC %ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id);

#else
          sprintf(str,"E2B_UC %ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON *)(msg->SigP))->melody_id);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        // L1S stop confirmation
        case L1_MELODY0_E2_STOP_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E20_L1SC %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"E20_L1SC %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_MELODY1_E2_STOP_CON :
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("E21_L1SC %ld",
                  l1s.actual_time.fn_mod42432);

#else
          sprintf(str,"E21_L1SC %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        // Instrument download
      #endif // MELODY_E2

      #if (L1_MP3 == 1)
        // MP3
        case MMI_MP3_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_R %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MP3_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MP3_REQ *)(msg->SigP))->loopback);
#else
          sprintf(str,"MP3_R %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MP3_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MP3_REQ *)(msg->SigP))->loopback);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MP3_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MP3_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MP3_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_PAUSE_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_PAUSE_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_PAUSE_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_PAUSE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_PAUSE_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_PAUSE_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_RESUME_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_RESUME_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_RESUME_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_RESUME_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_RESUME_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_RESUME_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_RESTART_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_RESTART_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_RESTART_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MP3_RESTART_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MP3_RESTART_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MP3_RESTART_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_MP3_ENABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPS_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPS_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_MP3_DISABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPS_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPS_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif

        }
        break;

        case API_MP3_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPA_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPA_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MP3_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPA_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MP3_STOP_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MPA_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MP3_STOP_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MP3_PAUSE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPA_PAUSE_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPA_PAUSE_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MP3_RESUME_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPA_RESUME_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPA_RESUME_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MP3_RESTART_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MPA_RESTART_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MPA_RESTART_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // L1_MP3

      #if (L1_AAC == 1)
        // AAC
        case MMI_AAC_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_R %ld %02ld %02ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->stereo,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->dma_channel_number);
#else
          sprintf(str,"AAC_R %ld %02ld %02ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->stereo,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->loopback,
                  ((T_MMI_AAC_REQ *)(msg->SigP))->dma_channel_number);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_C %ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_START_CON *)(msg->SigP))->aac_format);
#else
          sprintf(str,"AAC_C %ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_START_CON *)(msg->SigP))->aac_format);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"AAC_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_PAUSE_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_PAUSE_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_PAUSE_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_PAUSE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_PAUSE_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_PAUSE_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_RESUME_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_RESUME_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_RESUME_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_RESUME_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_RESUME_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_RESUME_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_RESTART_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_RESTART_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_RESTART_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_RESTART_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_RESTART_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_RESTART_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_INFO_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_INFO_R %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AAC_INFO_R %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_AAC_INFO_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AAC_INFO_C %ld %d %d %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->bitrate,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->frequency,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->aac_format,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->channel);
#else
          sprintf(str,"AAC_INFO_C %ld %d %d %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->bitrate,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->frequency,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->aac_format,
                  ((T_L1A_AAC_INFO_CON *)(msg->SigP))->channel);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AAC_ENABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACS_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACS_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AAC_DISABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACS_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACS_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif

        }
        break;

        case API_AAC_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACA_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACA_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_AAC_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACA_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_AAC_STOP_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"AACA_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_AAC_STOP_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_AAC_PAUSE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACA_PAUSE_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACA_PAUSE_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_AAC_RESUME_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACA_RESUME_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACA_RESUME_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_AAC_RESTART_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AACA_RESTART_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"AACA_RESTART_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // L1_AAC
      #if (L1_CPORT)
        //  Cport
        case MMI_CPORT_CONFIGURE_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("CPO_R %ld %04lx %04lx %02lx %02lx %02lx %02lx %02lx %02lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->configuration,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->ctrl,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr1,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr2,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr3,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr4,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptctl,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpttaddr,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptdat,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptvs);
#else
          sprintf(str,"CPO_R %ld %04lx %04lx %02lx %02lx %02lx %02lx %02lx %02lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->configuration,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->ctrl,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr1,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr2,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr3,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr4,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptctl,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpttaddr,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptdat,
                  ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptvs);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_CPORT_CONFIGURE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("CPO_C %ld %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_CPORT_CONFIGURE_CON *)(msg->SigP))->register_id,
                  ((T_MMI_CPORT_CONFIGURE_CON *)(msg->SigP))->register_value);
#else
          sprintf(str,"CPO_C %ld %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_CPORT_CONFIGURE_CON *)(msg->SigP))->register_id,
                  ((T_MMI_CPORT_CONFIGURE_CON *)(msg->SigP))->register_value);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_CPORT

      #if (L1_EXT_AUDIO_MGT == 1)
        //  External audio management
        case MMI_EXT_AUDIO_MGT_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("EAM_R %ld %02lx %02lx %02lx %02lx %04lx %08lx %02lx %02lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->sampling_frequency,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->DMA_channel_number,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->data_type,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->element_number,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->frame_number,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->source_buffer_address,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->mono_stereo,
                  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->session_id);
#else
          sprintf(str,"EAM_R %ld %02lx %02lx %02lx %02lx %04lx %08lx %02lx %02lx\n\r",
                  (WORD32) l1s.actual_time.fn_mod42432,
                 (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->sampling_frequency,
                  (WORD32) ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->DMA_channel_number,
                 (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->data_type,
                (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->element_number,
                  (WORD32) ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->frame_number,
                 (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->source_buffer_address,
                 (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->mono_stereo,
                 (WORD32)  ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->session_id);
rvt_send_trace_cpy ((T_RVT_BUFFER) str, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)strlen(str), RVT_ASCII_FORMAT);
//          L1_send_trace_cpy(str);  //omaps00090550
#endif
        }
        break;

        case MMI_EXT_AUDIO_MGT_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("EAM_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"EAM_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
rvt_send_trace_cpy ((T_RVT_BUFFER) str, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)strlen(str), RVT_ASCII_FORMAT);
//          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_EXT_AUDIO_MGT_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("EAM_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"EAM_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_EXT_AUDIO_MGT_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("EAM_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"EAM_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_EXT_AUDIO_MGT

      #if TESTMODE

        #if ((L1_STEREOPATH == 1) && (OP_L1_STANDALONE == 1))
          //  Stereopath
          case TMODE_AUDIO_STEREOPATH_START_REQ:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("STP_R %ld %02lx %02lx %02lx %02lx %02lx %02lx %04lx %04lx %02lx %02lx",
                    l1s.actual_time.fn_mod42432,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->configuration,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->sampling_frequency,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->DMA_allocation,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->DMA_channel_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->data_type,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->source_port,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->element_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->frame_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->mono_stereo,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->pattern_identifier);
#else
            sprintf(str,"STP_R %ld %02lx %02lx %02lx %02lx %02lx %02lx %04lx %04lx %02lx %02lx\n\r",
                    l1s.actual_time.fn_mod42432,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->configuration,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->sampling_frequency,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->DMA_allocation,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->DMA_channel_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->data_type,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->source_port,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->element_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->frame_number,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->mono_stereo,
                    ((T_TMODE_AUDIO_STEREOPATH_START_REQ *)(msg->SigP))->pattern_identifier);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case TMODE_AUDIO_STEREOPATH_START_CON:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("STP_C %ld",
                    l1s.actual_time.fn_mod42432);
#else
            sprintf(str,"STP_C %ld\n\r",
                    l1s.actual_time.fn_mod42432);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case TMODE_AUDIO_STEREOPATH_STOP_REQ:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("STP_S %ld",
                    l1s.actual_time.fn_mod42432);
#else
            sprintf(str,"STP_S %ld\n\r",
                    l1s.actual_time.fn_mod42432);
            L1_send_trace_cpy(str);
#endif
          }
          break;

          case TMODE_AUDIO_STEREOPATH_STOP_CON:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("STP_E %ld %04lx %04lx %04lx %04lx %04lx",
                    l1s.actual_time.fn_mod42432,
                    l1tm.stereopath.stereopath_source_timeout,
                    l1tm.stereopath.stereopath_dest_timeout,
                    l1tm.stereopath.stereopath_drop,
                    l1tm.stereopath.stereopath_half_block,
                    l1tm.stereopath.stereopath_block);
#else
            sprintf(str,"STP_E %ld %04lx %04lx %04lx %04lx %04lx\n\r",
                    l1s.actual_time.fn_mod42432,
                    l1tm.stereopath.stereopath_source_timeout,
                    l1tm.stereopath.stereopath_dest_timeout,
                    l1tm.stereopath.stereopath_drop,
                    l1tm.stereopath.stereopath_half_block,
                    l1tm.stereopath.stereopath_block);
            L1_send_trace_cpy(str);
#endif
          }
          break;

        #endif // L1_STEREOPATH

      #endif // TESTMODE

      #if (L1_ANR == 1)

        case MMI_ANR_REQ:
        {

		#if ( L1_COMPRESSED_TRACING == 1 )
		vsi_o_event_ttrace("ANR_R %ld %d %04lx %d %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->anr_enable,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->min_gain,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->div_factor_shift,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->ns_level);

		#else
          sprintf(str,"ANR_R %ld %d %04lx %d %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->anr_enable,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->min_gain,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->div_factor_shift,
                  ((T_MMI_ANR_REQ *)(msg->SigP))->ns_level);
          L1_send_trace_cpy(str);
        #endif
        }
        break;

        case L1_ANR_CON:
        {

		#if ( L1_COMPRESSED_TRACING == 1 )
		  vsi_o_event_ttrace("ANR_C %ld",
		                  l1s.actual_time.fn_mod42432);

        #else
          sprintf(str,"ANR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
        #endif

        }
        break;

      #endif // L1_ANR == 1

      #if (L1_ANR == 2)

        case MMI_AQI_ANR_REQ:
        {

		#if( L1_COMPRESSED_TRACING == 1 )
          vsi_o_event_ttrace("ANR_R %ld %d %d %d %d %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->anr_ul_control,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.ns_level,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_ene_th,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_cnt_th);
		#else
          sprintf(str,"ANR_R %ld %d %d %d %d %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->anr_ul_control,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.ns_level,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_ene_th,
                  ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_cnt_th);
          L1_send_trace_cpy(str);
        #endif
        }
        break;

        case L1_AQI_ANR_CON:
        {
		#if( L1_COMPRESSED_TRACING == 1 )
		  vsi_o_event_ttrace("ANR_C %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_ANR_CON *)(msg->SigP))->anr_ul_action);
		#else
          sprintf(str,"ANR_C %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_ANR_CON *)(msg->SigP))->anr_ul_action);
          L1_send_trace_cpy(str);
        #endif
        }
        break;

      #endif // L1_ANR == 2

#if(L1_AEC == 2)

        case MMI_AQI_AEC_REQ:
	    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AEC_R %1d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
          l1s.actual_time.fn_mod42432,
      	  ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_control,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.aec_mode,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.mu,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.cont_filter,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_ul,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_dl,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_dmax,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_good,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_bad,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_init,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_vad,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_fil,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_mut,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.thrs_abs,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.es_level_max,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.granularity_att,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.coef_smooth );
#else
	       sprintf(str,"AEC_R %1d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n\r",
          l1s.actual_time.fn_mod42432,
      	  ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_control,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.aec_mode,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.mu,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.cont_filter,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_ul,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_dl,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_dmax,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_good,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_bad,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_init,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_vad,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_fil,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_mut,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.thrs_abs,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.es_level_max,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.granularity_att,
	      ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.coef_smooth );

	      L1_send_trace_cpy(str);
#endif
	     }
	     break;

	     case L1_AQI_AEC_CON:
	     {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AEC_C %ld %d ",
	       l1s.actual_time.fn_mod42432,
	       ((T_L1_AQI_AEC_CON*)(msg->SigP))->aec_action);
#else
	       sprintf(str,"AEC_C %ld %d \n\r",
	       l1s.actual_time.fn_mod42432,
	       ((T_L1_AQI_AEC_CON*)(msg->SigP))->aec_action);
	       L1_send_trace_cpy(str);
#endif
	     }
	     break;
	     #endif

  #if (L1_IIR == 1)

        case MMI_IIR_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("IIR_R %ld %d %d %08lx %d %08lx %ld %ld %ld %ld %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->iir_enable,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->nb_iir_blocks,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->iir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->nb_fir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->fir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->input_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->fir_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->input_gain_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->feedback);
#else
          sprintf(str,"IIR_R %ld %d %d %08lx %d %08lx %ld %ld %ld %ld %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->iir_enable,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->nb_iir_blocks,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->iir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->nb_fir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->fir_coefs,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->input_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->fir_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->input_gain_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain_scaling,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain,
                  ((T_MMI_IIR_REQ *)(msg->SigP))->feedback);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_IIR_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("IIR_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"IIR_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_IIR == 1

      #if (L1_WCM == 1)

        case MMI_AQI_WCM_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("WCM_R %ld %d %d %d %d %d %d",
                  l1s.actual_time.fn_mod42432,
		  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->wcm_control,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.mode,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.frame_size,
				  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.num_sub_frames,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.ratio,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.threshold);
#else
          sprintf(str,"WCM_R %ld %d %d %d %d %d %d\n\r",
                  l1s.actual_time.fn_mod42432,
		  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->wcm_control,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.mode,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.frame_size,
				  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.num_sub_frames,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.ratio,
                  ((T_MMI_AQI_WCM_REQ *)(msg->SigP))->parameters.threshold);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AQI_WCM_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("WCM_C %ld  %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_WCM_CON *)(msg->SigP))->wcm_action);
#else
          sprintf(str,"WCM_C %ld  %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_WCM_CON *)(msg->SigP))->wcm_action);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_WCM == 1


      #if (L1_AGC_UL == 1)

        case MMI_AQI_AGC_UL_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AGCU_R %1d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
				  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->agc_ul_control,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.frame_size,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.targeted_level,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_up,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_down,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.max_scale,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_intp_flag);
#else
          sprintf(str,"AGCU_R %1d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
				  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->agc_ul_control,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.frame_size,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.targeted_level,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_up,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_down,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.max_scale,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast,
                  ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_intp_flag);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AQI_AGC_UL_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AGCU_C %ld  %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_AGC_UL_CON *)(msg->SigP))->agc_ul_action  );
#else
          sprintf(str,"AGCU_C %ld  %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_AGC_UL_CON *)(msg->SigP))->agc_ul_action  );
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif// L1_AGC_UL == 1

      #if (L1_AGC_DL == 1)

        case MMI_AQI_AGC_DL_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AGCD_R %1d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
				  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->agc_dl_control,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.frame_size,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.targeted_level,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_up,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_down,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.max_scale,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_intp_flag);
#else
          sprintf(str,"AGCD_R %1d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
				  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->agc_dl_control,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.control,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.frame_size,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.targeted_level,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_up,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_down,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.max_scale,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast,
                  ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_intp_flag);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AQI_AGC_DL_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("AGCD_C %ld  %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_AGC_DL_CON *)(msg->SigP))->agc_dl_action  );
#else
          sprintf(str,"AGCD_C %ld  %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_AGC_DL_CON *)(msg->SigP))->agc_dl_action  );
          L1_send_trace_cpy(str);
#endif

        }
        break;

      #endif // L1_AGC_DL == 1


	  #if (L1_IIR == 2)

        case MMI_AQI_IIR_DL_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("IIR_R %d %d %d %d %d %d %d %04lx %d %d %04lx",
                   l1s.actual_time.fn_mod42432,
				   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->iir_dl_control,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.control,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.frame_size,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_swap,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_enable,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_length,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_shift,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.sos_filter.sos_enable,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.sos_filter.sos_number,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.gain);
#else
          sprintf(str,"IIR_R %d %d %d %d %d %d %d %04lx %d %d %04lx\n\r",
                   l1s.actual_time.fn_mod42432,
				   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->iir_dl_control,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.control,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.frame_size,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_swap,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_enable,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_length,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.fir_filter.fir_shift,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.sos_filter.sos_enable,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.sos_filter.sos_number,
                   ((T_MMI_AQI_IIR_DL_REQ *)(msg->SigP))->parameters.gain);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AQI_IIR_DL_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("IIR_C %ld  %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_IIR_DL_CON *)(msg->SigP))->iir_dl_action );
#else
          sprintf(str,"IIR_C %ld  %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_IIR_DL_CON *)(msg->SigP))->iir_dl_action );
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_IIR == 2

      #if (L1_LIMITER == 1)

        case MMI_LIMITER_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("LIM_R %ld %d %ld %ld %ld %08lx %ld %ld %ld %ld %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->limiter_enable,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->block_size,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->slope_update_period,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->nb_fir_coefs,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->filter_coefs,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_0,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_slope,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_0,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_slope,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_fall,
                  ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_rise);
#else
          sprintf(str,"LIM_R %ld %d %ld %ld %ld %08lx %ld %ld %ld %ld %04lx %04lx\n\r",
                (WORD32)   l1s.actual_time.fn_mod42432,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->limiter_enable,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->block_size,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->slope_update_period,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->nb_fir_coefs,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->filter_coefs,
                (WORD32)   ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_0,
                (WORD32)   ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_slope,
                 (WORD32) ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_0,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_slope,
                (WORD32)   ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_fall,
                 (WORD32)  ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_rise);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        #if (L1_DRC == 1)

        case MMI_AQI_DRC_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("DRC_R %d %d %04lx %d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                   l1s.actual_time.fn_mod42432,
				   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->drc_dl_control,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.speech_mode_samp_f,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.num_subbands,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.frame_len,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_knee_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_knee_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_ratio_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_ratio_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.max_amplification_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.max_amplification_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_knee_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_knee_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_ratio_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_ratio_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.energy_limiting_th_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.energy_limiting_th_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_fb,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_md,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_hangover_spect_preserve,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_release_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_release_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.gain_track_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.gain_track_md_hg
				   );
#else
          sprintf(str,"DRC_R %d %d %04lx %d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                   l1s.actual_time.fn_mod42432,
				   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->drc_dl_control,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.speech_mode_samp_f,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.num_subbands,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.frame_len,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_knee_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_knee_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_ratio_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.expansion_ratio_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.max_amplification_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.max_amplification_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_knee_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_knee_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_ratio_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.compression_ratio_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.energy_limiting_th_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.energy_limiting_th_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_fb,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_md,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_threshold_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_hangover_spect_preserve,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_release_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.limiter_release_md_hg,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.gain_track_fb_bs,
                   ((T_MMI_AQI_DRC_REQ *)(msg->SigP))->parameters.gain_track_md_hg
				   );
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_AQI_DRC_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("DRC_C %ld  %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_DRC_CON *)(msg->SigP))->drc_dl_action );
#else
          sprintf(str,"DRC_C %ld  %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_AQI_DRC_CON *)(msg->SigP))->drc_dl_action );
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_DRC == 1


        case L1_LIMITER_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("LIM_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"LIM_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_LIMITER == 1

      #if (L1_ES == 1)

        case MMI_ES_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("ES_R  %ld %d %d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %ld %ld %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_enable,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_behavior,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_mode,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_dl,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_1,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_2,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ls_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ls_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ns_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ns_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_ne_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->ref_ls_pwr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->switching_time,
                  ((T_MMI_ES_REQ *)(msg->SigP))->switching_time_dt,
                  ((T_MMI_ES_REQ *)(msg->SigP))->hang_time,
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[0],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[1],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[2],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[3],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[0],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[1],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[2],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[3]);
#else
          sprintf(str,"ES_R  %ld %d %d %d %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx %ld %ld %ld %04lx %04lx %04lx %04lx %04lx %04lx %04lx %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_enable,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_behavior,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_mode,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_dl,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_1,
                  ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_2,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ls_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ls_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ns_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ns_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->tcl_ne_thr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->ref_ls_pwr,
                  ((T_MMI_ES_REQ *)(msg->SigP))->switching_time,
                  ((T_MMI_ES_REQ *)(msg->SigP))->switching_time_dt,
                  ((T_MMI_ES_REQ *)(msg->SigP))->hang_time,
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[0],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[1],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[2],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[3],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[0],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[1],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[2],
                  ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[3]);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_ES_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("ES_C  %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"ES_C  %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

      #endif // L1_ES == 1

      #if (L1_MIDI == 1)
        // MIDI
        case MMI_MIDI_START_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MID_R %ld %02ld %02ld",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MIDI_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MIDI_REQ *)(msg->SigP))->loopback);
#else
          sprintf(str,"MID_R %ld %02ld %02ld\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_MMI_MIDI_REQ *)(msg->SigP))->session_id,
                  ((T_MMI_MIDI_REQ *)(msg->SigP))->loopback);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MIDI_START_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MID_C %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MIDI_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MID_C %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MIDI_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MIDI_STOP_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MID_S %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MID_S %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case MMI_MIDI_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MID_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MIDI_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MID_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1A_MIDI_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MIDI_INIT_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDA_C %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MIDI_INIT_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MDA_C %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MIDI_INIT_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case API_MIDI_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDA_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MIDI_STOP_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MDA_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_API_MIDI_STOP_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_MIDI_ENABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDS_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MDS_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_MIDI_DISABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDS_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
          sprintf(str,"MDS_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_BACK_MIDI_INIT_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDB_C %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MIDI_INIT_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MDB_C %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MIDI_INIT_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_BACK_MIDI_STOP_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("MDB_E %ld %d",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MIDI_STOP_CON *)(msg->SigP))->error_code);
#else
          sprintf(str,"MDB_E %ld %d\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_L1_BACK_MIDI_STOP_CON *)(msg->SigP))->error_code);
          L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // L1_MIDI
      #if (L1_VOCODER_IF_CHANGE == 1)
        case MMI_TCH_VOCODER_CFG_REQ:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("VCH_R %ld %d",
           l1s.actual_time.fn_mod42432,
           ((T_MMI_TCH_VOCODER_CFG_REQ *)(msg->SigP))->vocoder_state);
#else
           sprintf(str,"VCH_R %ld %d\n\r",
           l1s.actual_time.fn_mod42432,
           ((T_MMI_TCH_VOCODER_CFG_REQ *)(msg->SigP))->vocoder_state);
           L1_send_trace_cpy(str);
#endif
        }
        break;
        case MMI_TCH_VOCODER_CFG_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("VCH_C %ld ", l1s.actual_time.fn_mod42432);
#else
           sprintf(str,"VCH_C %ld \n\r", l1s.actual_time.fn_mod42432);
           L1_send_trace_cpy(str);
#endif
        }
        break;

        case L1_VOCODER_CFG_ENABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("VEN_C %ld ", l1s.actual_time.fn_mod42432);
#else
           sprintf(str,"VEN_C %ld \n\r", l1s.actual_time.fn_mod42432);
           L1_send_trace_cpy(str);
#endif
        }
        break;
        case L1_VOCODER_CFG_DISABLE_CON:
        {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("VDS_C %ld ", l1s.actual_time.fn_mod42432);
#else
           sprintf(str,"VDS_C %ld \n\r", l1s.actual_time.fn_mod42432);
           L1_send_trace_cpy(str);
#endif
        }
        break;
      #endif // L1_VOCODER_IF_CHANGE == 1

    #endif  // AUDIO_TASK

    //////////////////
    // GTT messages //
    //////////////////

    #if (L1_GTT)

      case MMI_GTT_START_REQ:
      {UWORD8  length =strlen(str);
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("GTT_R %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "GTT_R %ld\n\r",
                l1s.actual_time.fn_mod42432);

rvt_send_trace_cpy ((T_RVT_BUFFER) str, trace_info.l1_trace_user_id, (T_RVT_MSG_LG)length, RVT_BINARY_FORMAT);
//      L1_send_trace_cpy(str); //omaps00090550
#endif
      }
      break;

      case MMI_GTT_START_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("GTT_C %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "GTT_C %ld\n\r",
                l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MMI_GTT_STOP_REQ:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("GTT_S %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "GTT_S %ld\n\r",
                l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;

      case MMI_GTT_STOP_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("GTT_E %ld",
                l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "GTT_E %ld\n\r",
                l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;

    #endif //(L1_GTT)

#if (L1_DYN_DSP_DWNLD == 1)
      case API_L1_DYN_DWNLD_START_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_DWL_C %ld",
                  l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "D_DWL_C %ld\n\r",
                  l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;
      case API_L1_DYN_DWNLD_FINISHED:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace( "D_DWL_E %ld",
                  l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "D_DWL_E %ld\n\r",
                  l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;
      case L1_DYN_DWNLD_STOP_CON:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_DWL_S_L1s %ld",
                  l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "D_DWL_S_L1s %ld\n\r",
                  l1s.actual_time.fn_mod42432);

        L1_send_trace_cpy(str);
#endif
      }
      break;
      case API_L1_DYN_DWNLD_UNINST_OK:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_UNST_OK %ld",
                  l1s.actual_time.fn_mod42432);
#else
        sprintf(str, "D_UNST_OK %ld\n\r",
                  l1s.actual_time.fn_mod42432);
        L1_send_trace_cpy(str);
#endif
      }
      break;
      case API_L1_DYN_DWNLD_STOP:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_DWL_ERR %ld %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_DYN_DWNLD_STOP *) (msg->SigP))->error);
#else
        sprintf(str, "D_DWL_ERR %ld %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_DYN_DWNLD_STOP *) (msg->SigP))->error);
        L1_send_trace_cpy(str);
#endif
      }
      break;
      case API_L1_CRC_NOT_OK:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_CRC_KO %ld %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_CRC_NOT_OK *) (msg->SigP))->patch_id);
#else
        sprintf(str, "D_CRC_KO %ld %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_CRC_NOT_OK *) (msg->SigP))->patch_id);

        L1_send_trace_cpy(str);
#endif
      }
      break;
      case API_L1_CRC_OK:
      {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("D_CRC_OK %ld %04lx",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_CRC_NOT_OK *) (msg->SigP))->patch_id);
#else
        sprintf(str, "D_CRC_OK %ld %04lx\n\r",
                  l1s.actual_time.fn_mod42432,
                  ((T_API_L1_CRC_NOT_OK *) (msg->SigP))->patch_id);
        L1_send_trace_cpy(str);
#endif
      }
      break;
#endif // L1_DYN_DSP_DWNLD == 1

    ///////////////////
    // OML1 messages //
    ///////////////////

    case OML1_CLOSE_TCH_LOOP_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("CLO_R %ld %d %d",
              l1s.actual_time.fn_mod42432,
              ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->sub_channel,
              ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->frame_erasure);
#else
      sprintf(str,"CLO_R %ld %d %d\n\r",
              l1s.actual_time.fn_mod42432,
              ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->sub_channel,
              ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->frame_erasure);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case OML1_OPEN_TCH_LOOP_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
        vsi_o_event_ttrace("OLO_R %ld",
              l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"OLO_R %ld\n\r",
              l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case OML1_START_DAI_TEST_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("SDA_R %ld %d",
              l1s.actual_time.fn_mod42432,
              ((T_OML1_START_DAI_TEST_REQ *)(msg->SigP))->tested_device);
#else
      sprintf(str,"SDA_R %ld %d\n\r",
              l1s.actual_time.fn_mod42432,
              ((T_OML1_START_DAI_TEST_REQ *)(msg->SigP))->tested_device);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case OML1_STOP_DAI_TEST_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("EDA_R %ld",
              l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"EDA_R %ld\n\r",
              l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    ///////////////////
    // Test messages //
    ///////////////////

    case TST_TEST_HW_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("TST_R %ld",
            l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"TST_R %ld\n\r",
            l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case L1_TEST_HW_INFO:
    {
      UWORD16 tcs_program_release;

      tcs_program_release = ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_program_release;

      if (((tcs_program_release & 0xFFF0) == 0x2110) || ((tcs_program_release & 0xFFF0) == 0x2120))
        sprintf(str, "TST_C %ld TCS_%x.%x.%x.%x_L1_%x_%x PLUS_N5x DSP:%xh DYN:%xh CHECKSUM:%xh\n\r",
                l1s.actual_time.fn_mod42432,
                (tcs_program_release & 0xF000) >> 12,
                (tcs_program_release & 0x0F00) >> 8 ,
                (tcs_program_release & 0x00F0) >> 4 ,
                tcs_program_release & 0x000F,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_official,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_internal,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_code_version,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_patch_version,
              ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_checksum);
      else
        sprintf(str, "TST_C %ld TCS_%x.%x.%x_L1_%x_%x PLUS_N5x DSP:%xh DYN:%xh CHECKSUM:%xh\n\r",
                l1s.actual_time.fn_mod42432,
                (tcs_program_release & 0xF000) >> 12,
                (tcs_program_release & 0x0F00) >> 8 ,
                tcs_program_release & 0x00FF,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_official,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->mcu_tcs_internal,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_code_version,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_patch_version,
                ((T_TST_TEST_HW_CON*)(msg->SigP))->dsp_checksum);

      #if 0 //((((CHIPSET !=2 )) && ((LONG_JUMP != 0))) || (CHIPSET == 12) || (CHIPSET == 15))
      {
        sprintf(str2,"CHECKSUM before DWNL:%04x\n\r",d_checksum1);
        strcat(str,str2);
        sprintf(str2,"CHECKSUM after DWNL :%04x\n\r",d_checksum2);
        strcat(str,str2);
      }
      #endif //((((CHIPSET !=2 )) && ((LONG_JUMP != 0))) || (CHIPSET == 12))
        #if (L1_DRP == 1)
        {
           char str3[80];

           sprintf(str3,"DRP REF SW Version:%08x AND TAG:%08x RESULT = %d \n\r",drp_ref_sw_ver,drp_ref_sw_tag
                                                                  ,l1s.boot_result);
           strcat(str,str3);
        }
        #endif //L1_DRP

      L1_send_trace_cpy(str);
    }
    break;

    case TST_SLEEP_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("SLE_R %ld %d %d",
              l1s.actual_time.fn_mod42432,
              ((T_TST_SLEEP_REQ*)(msg->SigP))->sleep_mode,
              ((T_TST_SLEEP_REQ*)(msg->SigP))->clocks);
#else
      sprintf(str,"SLE_R %ld %d %d\n\r",
              l1s.actual_time.fn_mod42432,
              ((T_TST_SLEEP_REQ*)(msg->SigP))->sleep_mode,
              ((T_TST_SLEEP_REQ*)(msg->SigP))->clocks);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    /////////
    // ADC //
    /////////

    case MMI_ADC_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("ADC_R %ld",
              l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"ADC_R %ld\n\r",
              l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case MMI_STOP_ADC_REQ:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("ADC_S %ld",
              l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"ADC_S %ld\n\r",
              l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    case MMI_STOP_ADC_CON:
    {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("ADC_C %ld",
              l1s.actual_time.fn_mod42432);
#else
      sprintf(str,"ADC_C %ld\n\r",
              l1s.actual_time.fn_mod42432);
      L1_send_trace_cpy(str);
#endif
    }
    break;

    } // ...End of switch
  } // End if L1A message trace enabled or SignalCode = TRACE_INFO
}

#if L1_RECOVERY
/*********************************/
/* Trace in case of system crash */
/*********************************/
void l1_trace_recovery(void)
{
char *s={"L1 Recovery performed"};
  trace_info.l1_memorize_error = '.'; // memorize an error in the L1
  L1_send_trace_cpy("> RECOVERY \n\r");
  l1_trace_L1_tasks(); // trace L1 tasks status
  // inform Riviera DAR system about the Recovery performed.
  #if (OP_L1_STANDALONE == 1)
  rvf_send_trace1  (s, (T_RVT_MSG_LG)strlen(s),(UWORD32)0,0, (UWORD32)trace_info.l1_trace_user_id) ;// omaps00090550
  #else
  rvf_send_trace1  (s, (T_RVT_MSG_LG)strlen(s),(UWORD32)0,RV_TRACE_LEVEL_WARNING, (UWORD32)trace_info.l1_trace_user_id) ;// omaps00090550
  #endif
//  L1_send_trace_cpy_DAR("L1 Recovery performed"); //omaps00090550
}
#endif //L1_RECOVERY

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

/*************************************************************************/
/* L1 Dynamic traces                                                     */
/*************************************************************************/

/* WARNING : Following functions are called by L1S */
/***************************************************/


//////////////////////
// L1S Error Traces //
//////////////////////

/*-------------------------------------------------------*/
/* Trace_L1s_Abort()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_L1s_Abort(UWORD8 task)
{
  send_debug_sig(L1S_ABORT, task);
}

/*-------------------------------------------------------*/
/* Trace_MCU_DSP_Com_Mismatch()                          */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_MCU_DSP_Com_Mismatch(UWORD8 task)
{
  if((l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff ) != (l1s.debug_time & 0xffff ))
  // Debug number is different than the one expected...
  {
    if(!trace_info.DSP_misaligned)
    // MCU/DSP com. is misaligned.
    {

      trace_info.trace_buff_stop = TRUE; // stop buffer trace
      trace_info.l1_memorize_error = '.'; // memorize an error in the L1

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      {
        send_debug_sig(MCU_DSP_MISMATCH, task);

        #if (DSP_DEBUG_TRACE_ENABLE == 1)
          if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
          // Flag DSP error for DSP trace and memorize address of start of DSP trace
          {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif

            DSP_DEBUG_ENABLE
          }
        #endif //(DSP_DEBUG_TRACE_ENABLE)
      }

      trace_info.DSP_misaligned = TRUE;
    }
  }
  else
  {
    if(trace_info.DSP_misaligned)
    // MCU/DSP com. is now realigned.
    {
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      {
        send_debug_sig(NO_MCU_DSP_MISMATCH, task);
      }

      trace_info.DSP_misaligned = FALSE;
    }
  }
}

/*-------------------------------------------------------*/
/* Trace_PM_Equal_0()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_PM_Equal_0(UWORD32 pm, UWORD8 task)
{
  if(pm==0) // PM error in the frame
  {
    trace_info.trace_buff_stop = TRUE; // stop buffer trace
    trace_info.l1_memorize_error = '.'; // memorize an error in the L1

    if (trace_info.PM_Task == 255) // 1st PM error in the frame: This PM is memorized
      trace_info.PM_Task  = task;  // memorize the Task of this 1st PM error

#if ( ((TRACE_TYPE==1) || (TRACE_TYPE == 4)))
     if (trace_info.current_config->l1_dyn_trace & 1 <<  L1_DSP_TRACE_FULL_DUMP)
     {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
 	     // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
       if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif
         l1_trace_full_dsp_buffer();  // trace DSP trace buffer in case a PM error occurs
      }                             // used only for debug mode,
#endif
  }
  else // no error in the frame :is it a PM recovery ?
  {
    if (trace_info.Not_PM_Task == 255) // 1st PM recovery case: task of recovery needs to be memorized
      trace_info.Not_PM_Task = task;
  }
if (pm< 0x00C0) //sajal made changed it from- if (pm<= 0x00C0)

  {
      #if ( ((TRACE_TYPE==1) || (TRACE_TYPE == 4)))
       if (trace_info.current_config->l1_dyn_trace &  1 << L1_DYN_TRACE_L1S_DEBUG)
      {
          send_debug_sig(IQ_LOW, task);
       }
      #endif

   }

}

/*-------------------------------------------------------*/
/* Trace_PM_Equal_0_balance()                            */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_PM_Equal_0_balance(void)
{
  // Here below we handle the case where we have lot of PM occuring during next frames
  // The PM traces are filtered in order to trace:
  //   => the first PM
  //   => the latest PM when we have no more PM

  if(trace_info.PM_Task != 255) // at least one PM occured in the current frame
  {
    if(!trace_info.PM_equal_0)  // We are not in a phase of PM: We trace only the 1st PM
    {
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      {
        send_debug_sig(PM_EQUAL_0, trace_info.PM_Task);

        #if (DSP_DEBUG_TRACE_ENABLE == 1)
          if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)

          // Flag DSP error for DSP trace and memorize address of start of DSP trace
          {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD)
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif

            DSP_DEBUG_ENABLE
          }
        #endif //(DSP_DEBUG_TRACE_ENABLE)
      }

      trace_info.PM_equal_0 = TRUE;  // We enter in a phase of a lot of PM
    }
  }
  else // no PM in the current frame
  {
    if(trace_info.PM_equal_0) // this is the end of the PM phase: we trace the latest PM
    {
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      {
        send_debug_sig(NO_PM_EQUAL_0, trace_info.Not_PM_Task);
      }

      trace_info.PM_equal_0 = FALSE;
    }
  }

  trace_info.PM_Task     = 255;
  trace_info.Not_PM_Task = 255;
}

/*-------------------------------------------------------*/
/* send_debug_sig()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void send_debug_sig(UWORD8 debug_code, UWORD8 task)
{
#if (GSM_IDLE_RAM == 0)
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = debug_code;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = task;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = l1s.debug_time & 0xffff;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = l1s.actual_time.fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = l1s.tpu_offset;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = l1s.tpu_offset_hw;
  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = l1a_l1s_com.adc_mode | (trace_info.Not_PM_Task << 16); // ADC enabled

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
#else

  #if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )

                switch (debug_code)
                {
                  case PM_EQUAL_0:
                  {
                    vsi_o_event_ttrace(">  PM %ld %ld %ld %ld %ld %ld %ld %d %ld %d",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode,
                             (trace_info.Not_PM_Task << 16));

                    if (trace_info.trace_filter == FALSE)
                    {
                      #if (GSM_IDLE_RAM == 0)
                        l1_trace_L1_tasks();            // trace L1 tasks status
                      #else
                        l1s_trace_mftab();
                      #endif
                      l1_display_buffer_trace_fct();  // display buffer fct called
                      trace_info.trace_filter = TRUE; // avoid too much traces displayed
                    }
                  }
                  break;

                  case NO_PM_EQUAL_0:
                  {
                    vsi_o_event_ttrace(">  !PM %ld %ld %ld %ld %ld %ld %ld %d %ld",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;
                  case L1S_ABORT:
                  {
                    vsi_o_event_ttrace("ABORT %ld %ld %ld %ld %ld %ld %ld %ld",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
				  }
                  break;

                  case MCU_DSP_MISMATCH:
                  {
                    vsi_o_event_ttrace("> COM %ld %ld %ld %ld %ld %ld %ld %ld",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;

                  case NO_MCU_DSP_MISMATCH:
                  {
                    vsi_o_event_ttrace(">!COM %ld %ld %ld %ld %ld %ld %ld %ld",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;
                }

  #else
              CHAR str[128];

                switch (debug_code)
                {
                  case PM_EQUAL_0:
                  {
                    sprintf (str,">  PM %ld %ld %ld %ld %ld %ld %ld %d %ld %d\n\r",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode,
                             (trace_info.Not_PM_Task << 16));

                    if (trace_info.trace_filter == FALSE)
                    {
                      #if (GSM_IDLE_RAM == 0)
                        l1_trace_L1_tasks();            // trace L1 tasks status
                      #else
                        l1s_trace_mftab();
                      #endif
                      l1_display_buffer_trace_fct();  // display buffer fct called
                      trace_info.trace_filter = TRUE; // avoid too much traces displayed
                    }
                  }
                  break;

                  case NO_PM_EQUAL_0:
                  {
                    sprintf (str,">  !PM %ld %ld %ld %ld %ld %ld %ld %d %ld\n\r",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;
                  case L1S_ABORT:
                  {
                    sprintf (str,"ABORT %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
}
                  break;

                  case MCU_DSP_MISMATCH:
                  {
                    sprintf (str,"> COM %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;

                  case NO_MCU_DSP_MISMATCH:
                  {
                    sprintf (str,">!COM %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                             l1s.actual_time.fn_mod42432,
                             task,
                             l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff,
                             l1s.debug_time & 0xffff,
                             l1s.actual_time.fn,
                             l1s.tpu_offset,
                             l1s.tpu_offset_hw,
                             l1a_l1s_com.adc_mode);
                  }
                  break;
                }

                l1_intram_put_trace(str);
  #endif // L1_COMPRESSED_TRACING

            #endif // (GSM_IDLE_RAM == 0)
          }
        #endif //(TRACE_TYPE==1) || (TRACE_TYPE==4)

// l1_trace_gauging_reset()
// Parameters :
// This means instability with the 32Khz
//        void l1_trace_gauging_reset(void)  {
//          #if (CODE_VERSION != SIMULATION)
//           #if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)
//            //trace if the gauging can't succeed
//              L1_trace_char('#');
//            #elif (TRACE_TYPE !=0 )
//         if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)                      {                                                                          */
//                // to trace during Gauging interrupt causes issue with Pool memory.
            // the trace will be done with the next gauging.
//                trace_info.reset_gauging_algo = TRUE;  // trace Reset gauging Algorithm
//              }           #endif
//          #else  // Simulation part
//            #if (TRACE_TYPE==5)
//              trace_ULPD("Reset Gauging algorithm", l1s.actual_time.fn);           #endif                                                                       */
//          #endif // Simulation part


       //#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
       #endif // !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))

///////////////////////
// P.Transfer traces //
///////////////////////

#if L1_GPRS

/* Trace_Packet_Transfer()                               */
/* Parameters :                                          */
/* Return     :                                          */

void  Trace_Packet_Transfer(UWORD8  prev_crc_error) // Previous RX blocks CRC_ERROR summary
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1S_PACKET_TRANSFER;

  switch(l1s.actual_time.fn_mod52)
  {
    case 4: ((T_TRACE_INFO *)(msg->SigP))->tab[0]  = 0;  break;
    case 8: ((T_TRACE_INFO *)(msg->SigP))->tab[0]  = 1;  break;
    case 12: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 2;  break;
    case 17: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 3;  break;
    case 21: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 4;  break;
    case 25: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 5;  break;
    case 30: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 6;  break;
    case 34: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 7;  break;
    case 38: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 8;  break;
    case 43: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 9;  break;
    case 47: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 10; break;
    case 51: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 11; break;

    default: ((T_TRACE_INFO *)(msg->SigP))->tab[0] = 999; break;
  }

  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = prev_crc_error;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#if (RLC_DL_BLOCK_STAT)

/* Trace_RLC_statistic()                               */
/* Parameters :                                          */
/* Return     :                                          */

void  Trace_RLC_statistic(UWORD8  cs_type,          // Previous RX allocation
                          UWORD32  nb_frames,       // Previous Number of RX blocks received
                          UWORD32  nb_bad_frames,   // Previous RX blocks CRC_ERROR summary
                          UWORD32  nb_cs1_frames)   // Current TX allocation for Polling
{
  xSignalHeaderRec *msg;
  UWORD8            i;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1S_RLC_STAT;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = cs_type;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = nb_frames;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = nb_bad_frames;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = nb_cs1_frames;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
} // End Trace_RLC_statistic

#endif //(RLC_DL_BLOCK_STAT)

#if (DEBUG_DEDIC_TCH_BLOCK_STAT == 1)

/* Trace_dedic_tch_block_stat                            */
/* Parameters :                                          */
/* Return     :                                          */


void  Trace_dedic_tch_block_stat(UWORD8  block_id,  // ID of block: FACCH, SPEECH, SID_UPDATE
                                 UWORD16 d_nerr,    // RXQUAL
                                 UWORD8  voco_type) // vocoder type for Half-rate
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = DEDIC_TCH_BLOCK_STAT;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = block_id;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = d_nerr;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = voco_type;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = 0;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = 0;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = 0;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = 0;
  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = 0;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
} // End Trace_dedic_tch_block_stat

#endif //(DEBUG_DEDIC_TCH_BLOCK_STAT == 1)


/* Trace_dl_ptcch()                                      */
/* Parameters :                                          */
/* Return     :                                          */

void  Trace_dl_ptcch(UWORD8  ordered_ta,
                     UWORD8  crc,
                     UWORD8  ta_index,
                     UWORD8  ts,
                     UWORD16 elt1,
                     UWORD16 elt2,
                     UWORD16 elt3,
                     UWORD16 elt4,
                     UWORD16 elt5,
                     UWORD16 elt6,
                     UWORD16 elt7,
                     UWORD16 elt8)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = DL_PTCCH;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = crc;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = ordered_ta;

  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = ta_index;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = ts; //timeslot

  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = elt1;//16 TA values, each 8 bits
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = ((T_TRACE_INFO *)(msg->SigP))->tab[4] << 0x10 | elt2;

  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = elt3;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = ((T_TRACE_INFO *)(msg->SigP))->tab[5] << 0x10 | elt4;

  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = elt5;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = ((T_TRACE_INFO *)(msg->SigP))->tab[6] << 0x10 | elt6;

  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = elt7;
  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = ((T_TRACE_INFO *)(msg->SigP))->tab[7] << 0x10 | elt8;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)

} // End Trace_ptcch_error
#if FF_TBF
/*-------------------------------------------------------*/
/* Trace_rlc_ul_param()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_rlc_ul_param(UWORD8  assignment_id,
                        UWORD32  fn,
                        UWORD8 tx_no,
                        UWORD8  ta,
                        BOOL    fix_alloc_exhaust,
                        UWORD32 cs_type)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = RLC_UL_PARAM;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = assignment_id;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = tx_no;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = ta;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = fix_alloc_exhaust;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = cs_type;
  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* Trace_rlc_dl_param()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_rlc_dl_param( UWORD8  assignment_id,
                         UWORD32 fn,
                         UWORD8  rx_no,
                         UWORD8  rlc_blocks_sent,
                         UWORD8  last_poll_response,
                         UWORD32 status1,
                         UWORD32 status2 )
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = RLC_DL_PARAM;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = assignment_id;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = rx_no;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = rlc_blocks_sent;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = last_poll_response;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = status1;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = status2;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

/*-------------------------------------------------------*/
/* Trace_rlc_poll_param()                                  */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
void Trace_rlc_poll_param(BOOL poll,
                          UWORD32 fn,
                          UWORD8  poll_ts,
                          UWORD8 tx_alloc,
                          UWORD8 tx_data,
                          UWORD8 rx_alloc,
                          UWORD8 last_poll_resp,
                          UWORD8 ack_type)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = RLC_POLL_PARAM;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = poll;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = poll_ts;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = tx_alloc;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = tx_data;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = rx_alloc;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = last_poll_resp;
  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = ack_type;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#else


/* Trace_rlc_ul_param()                                  */
/* Parameters :                                          */
/* Return     :                                          */

void Trace_rlc_ul_param(UWORD8  assignment_id,
                        UWORD8  tx_no,
                        UWORD32 fn,
                        UWORD8  ta,
                        UWORD32 a_pu_gprs,
                        UWORD32 a_du_gprs,
                        BOOL    fix_alloc_exhaust)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = RLC_UL_PARAM;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = assignment_id;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = tx_no;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = ta;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = a_pu_gprs;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = a_du_gprs;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = fix_alloc_exhaust;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}


/* Trace_rlc_dl_param()                                  */
/* Parameters :                                          */
/* Return     :                                          */

void Trace_rlc_dl_param( UWORD8  assignment_id,
                         UWORD32 fn,
                         UWORD32 d_rlcmac_rx_no_gprs,
                         UWORD8  rx_no,
                         UWORD8  rlc_blocks_sent,
                         UWORD8  last_poll_response)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = RLC_DL_PARAM;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = assignment_id;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = d_rlcmac_rx_no_gprs;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = rx_no;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = rlc_blocks_sent;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = last_poll_response;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}
#endif


/* Trace_uplink_no_TA()                                  */
/* Parameters :                                          */
/* Return     :                                          */

void Trace_uplink_no_TA()
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = FORBIDDEN_UPLINK;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}


/* l1_trace_ptcch_disable()                              */

/* Parameters :                                          */
/* Return     :                                          */
/* Trace the gauging is running                          */

void l1_trace_ptcch_disable(void)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

    xSignalHeaderRec *msg;

    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
    {
      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TRACE_INFO));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TRACE_INFO *)(msg->SigP))->debug_code = PTCCH_DISABLED;

      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
  #endif //(TRACE_TYPE==1) || (TRACE_TYPE==4)
}


/* Trace_pdtch()                                         */

/* Parameters :                                          */
/* Return     :                                          */

void  Trace_condensed_pdtch(UWORD8 rx_allocation, UWORD8 tx_allocation)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_CONDENSED_PDTCH_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_CONDENSED_PDTCH;

  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->fn            = l1s.actual_time.fn_mod52;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->rx_allocation = rx_allocation;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->tx_allocation = tx_allocation;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->blk_status    = trace_info.pdtch_trace.blk_status;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_cs_type    = trace_info.pdtch_trace.dl_cs_type;
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[0]  = trace_info.pdtch_trace.dl_status[0];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[1]  = trace_info.pdtch_trace.dl_status[1];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[2]  = trace_info.pdtch_trace.dl_status[2];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->dl_status[3]  = trace_info.pdtch_trace.dl_status[3];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[0]  = trace_info.pdtch_trace.ul_status[0];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[1]  = trace_info.pdtch_trace.ul_status[1];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[2]  = trace_info.pdtch_trace.ul_status[2];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[3]  = trace_info.pdtch_trace.ul_status[3];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[4]  = trace_info.pdtch_trace.ul_status[4];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[5]  = trace_info.pdtch_trace.ul_status[5];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[6]  = trace_info.pdtch_trace.ul_status[6];
  ((T_CONDENSED_PDTCH_INFO *)(msg->SigP))->ul_status[7]  = trace_info.pdtch_trace.ul_status[7];

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#endif // L1_GPRS

///////////////////////
// DSP error traces  //
///////////////////////

#if (D_ERROR_STATUS_TRACE_ENABLE)


/* Trace_d_error_status()                                */

/* Parameters :                                          */
/* Return     :                                          */


void Trace_d_error_status()
{
  #if L1_GPRS
    UWORD16           d_error_status_masked =
          (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
          (trace_info.d_error_status_masks[l1a_l1s_com.dsp_scheduler_mode - 1]); // depends on the scheduler mode
  #else
    UWORD16           d_error_status_masked =
          (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
          (trace_info.d_error_status_masks[GSM_SCHEDULER - 1]);
  #endif //L1_GPRS
  UWORD16 changed_bits = d_error_status_masked ^ trace_info.d_error_status_old;

  // trace in case of change of status (field is reseted on change of scheduler)
  if (changed_bits)
  {
    xSignalHeaderRec *msg;

    // Allocate DEBUG message.
    msg = os_alloc_sig(sizeof(T_TRACE_INFO));
    DEBUGMSG(status,NU_ALLOC_ERR)
    msg->SignalCode = TRACE_INFO;

    ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1S_D_ERROR_STATUS;
    ((T_TRACE_INFO *)(msg->SigP))->tab[0]     = l1s.actual_time.fn_mod42432;
    ((T_TRACE_INFO *)(msg->SigP))->tab[1]     = (UWORD16)l1s.debug_time;
    ((T_TRACE_INFO *)(msg->SigP))->tab[2]     = d_error_status_masked;
    ((T_TRACE_INFO *)(msg->SigP))->tab[3]     = l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff;

    // send message...
    os_send_sig(msg, L1C1_QUEUE);
    DEBUGMSG(status,NU_SEND_QUEUE_ERR)

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
      {
        // DSP debug buffer trace only if an ERROR is detected (not for a End of error detection)
        if ((changed_bits & d_error_status_masked) & ~trace_info.d_error_status_old)
        {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
            // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
            if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
          #endif
          // Flag DSP error for DSP trace and memorize address of start of DSP trace
          DSP_DEBUG_ENABLE
        }
      }
    #endif //(DSP_DEBUG_TRACE_ENABLE)

    trace_info.d_error_status_old = d_error_status_masked;
  }

  // Clear bits that have been set by the DSP
  l1s_dsp_com.dsp_ndb_ptr->d_error_status &= ~d_error_status_masked;
}

#endif //(D_ERROR_STATUS_TRACE_ENABLE)

#if (DSP_DEBUG_TRACE_ENABLE == 1)


/* Trace_dsp_debug()                                     */

/* Parameters :                                          */
/* Return     :                                          */

void Trace_dsp_debug()
{
  // WARNING: l1s_dsp_com.dsp_r_page changed in l1s_end_manager() but DSP DB pointers haven't been
  // updated !!!
  UWORD32           start_address = trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page];
  UWORD32           end_address, stop_address;
  UWORD16           size;
  API              *i;
  UWORD8            j = 0;
  xSignalHeaderRec *msg;

  // DSP DEBUG trace only works when GSM activity is enabled
  if (l1s_dsp_com.dsp_r_page_used == FALSE)
  {
    trace_info.dsp_debug_buf_start[0] = trace_info.dsp_debug_buf_start[1] = 0;
  }

  // If a DSP error occured...
  if (start_address)
  {
    WORD32 diff = l1s.debug_time - trace_info.fn_last_dsp_debug;

    if (diff < 0) diff += 0xFFFFFFFF;

    if (diff >= 104)
    {

      // Take the DB_R pointers on the start/end of last TDMA trace
      start_address = 0xFFD00000 + (start_address - 0x800) * 2;
      end_address   = 0xFFD00000 + (l1s_dsp_com.dsp_db2_other_r_ptr->d_debug_ptr_end - 0x800) * 2;

      // Process size of block
      if (end_address >= start_address)
      {
        size = end_address - start_address;
        stop_address = end_address;
      }
      else
      {
        size = end_address - start_address + C_DEBUG_BUFFER_SIZE * 2;
        stop_address = (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2);
      }

      if ((size > 0) && (size < 1000) && (size < C_DEBUG_BUFFER_SIZE))
      {
        // Allocate memory pool
        msg = os_alloc_sig(size+sizeof(T_DSP_DEBUG_INFO)-2*sizeof(API));
        DEBUGMSG(status,NU_ALLOC_ERR)

        msg->SignalCode                                = TRACE_DSP_DEBUG;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->size        = size;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->fn          = trace_info.dsp_debug_fn[l1s_dsp_com.dsp_r_page];
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->debug_time  = trace_info.dsp_debug_time[l1s_dsp_com.dsp_r_page];
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->patch_version = l1s_dsp_com.dsp_ndb_ptr->d_version_number2;
        ((T_DSP_DEBUG_INFO *)(msg->SigP))->trace_level   = l1s_dsp_com.dsp_ndb_ptr->d_debug_trace_type;

        // Copy data into message
        for (i = (API*)start_address; i < (API*)stop_address; i++)
        {
          ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
        }

        // Circular buffer management
        if (i != (API*)end_address)
        {
          for (i = (API*) (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 - 0x800)*2); i < (API*)end_address; i++)
          {
            ((T_DSP_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
          }
        }

        // Send sig to L1A
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

        // Set FN to avoid another DSP debug trace in the next 104 frames.
        trace_info.fn_last_dsp_debug = l1s.debug_time;
      }
    } // Enf if diff >= 104

    // Clear flag
    trace_info.dsp_debug_buf_start[l1s_dsp_com.dsp_r_page] = 0;
  } // End if "DSP error occured"
}

#if (AMR == 1)


/* Trace_dsp_amr_debug()                                 */

/* Parameters :                                          */
/* Return     :                                          */


void Trace_dsp_amr_debug()
{
  UWORD32           start_address;
  UWORD32           end_address, stop_address;
  UWORD16           size;
  API              *i;
  UWORD8            j = 0;
  xSignalHeaderRec *msg;

  // Start address of the AMR trace in the DSP trace buffer
  start_address = l1s_dsp_com.dsp_ndb_ptr->p_debug_amr;

  // Clear the pointer
  l1s_dsp_com.dsp_ndb_ptr->p_debug_amr = 0;

  // If start address different of 0 -> trace to be performed
  if (start_address != 0)
  {
      // Process MCU start address
      start_address = 0xFFD00000 + (start_address - 0x800) * 2;

      // Check ID and read size
      if (((*((API *)start_address) & 0xFE00) >> 9) == C_AMR_TRACE_ID)
      {
        // Read size
        size = (*((API *)start_address) & 0x1FF) * 2;
        start_address += sizeof(API); // Do not dump header

        // Process stop address
        end_address = start_address + size;

        // Circular buffer...
        if (end_address <= (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2))
        {
            stop_address = end_address;
        }
        else
        {
            stop_address = (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 + C_DEBUG_BUFFER_SIZE - 0x800) * 2);
            end_address -= C_DEBUG_BUFFER_SIZE * 2;
        }

        // Create L1S->L1A message and dump buffer

        // Allocate memory pool
        msg = os_alloc_sig(size+sizeof(T_DSP_AMR_DEBUG_INFO)-2*sizeof(API));
        DEBUGMSG(status,NU_ALLOC_ERR)

        msg->SignalCode                                = TRACE_DSP_AMR_DEBUG;
        ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->size    = size;
        ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->fn      = l1s.actual_time.fn;

        // Copy data into message
        for (i = (API*)start_address; i < (API*)stop_address; i++)
        {
          ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
        }

        // Circular buffer management
        if (i != (API*)end_address)
        {
          for (i = (API*) (0xFFD00000 + (C_DEBUG_BUFFER_ADD + 1 - 0x800)*2); i < (API*)end_address; i++)
          {
            ((T_DSP_AMR_DEBUG_INFO *)(msg->SigP))->buffer[j++] = *i;
          }
        }

        // Send sig to L1A
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
  }
}
#endif // #if (AMR == 1)
#endif //DSP_DEBUG_TRACE_ENABLE

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
  //#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise
    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)


/* l1_display_buffer_trace_fct()                         */
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/* trace buffer fct                                      */

void l1_display_buffer_trace_fct(void)
{
  CHAR str[100];
  UWORD8 fct_id,i,j;
  //UWORD8 index = trace_info.trace_fct_buff_index; - OMAPS90550-new
  trace_info.trace_buff_stop = TRUE; // stop buffer trace


#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )

  if (((l1s.version.mcu_tcs_program_release & 0xFFF0) == 0x2110) ||
      ((l1s.version.mcu_tcs_program_release & 0xFFF0) == 0x2120))
    vsi_o_event_ttrace("SW version: TCS%x.%x.%x.%x_L1_%x_%x",
            (l1s.version.mcu_tcs_program_release & 0xF000) >> 12,
            (l1s.version.mcu_tcs_program_release & 0x0F00) >> 8,
            (l1s.version.mcu_tcs_program_release & 0x00F0) >> 4,
            l1s.version.mcu_tcs_program_release & 0x000F,
            l1s.version.mcu_tcs_official,
            l1s.version.mcu_tcs_internal);
  else
    vsi_o_event_ttrace("SW version: TCS.%x.%x.%x_L1_%x_%x",
            (l1s.version.mcu_tcs_program_release & 0xF000) >> 12,
            (l1s.version.mcu_tcs_program_release & 0x0F00) >> 8,
            l1s.version.mcu_tcs_program_release & 0x00FF,
            l1s.version.mcu_tcs_official,
            l1s.version.mcu_tcs_internal);

#else


  if (((l1s.version.mcu_tcs_program_release & 0xFFF0) == 0x2110) ||
      ((l1s.version.mcu_tcs_program_release & 0xFFF0) == 0x2120))
    sprintf(str,"SW version: TCS%x.%x.%x.%x_L1_%x_%x\n\r",
            (l1s.version.mcu_tcs_program_release & 0xF000) >> 12,
            (l1s.version.mcu_tcs_program_release & 0x0F00) >> 8,
            (l1s.version.mcu_tcs_program_release & 0x00F0) >> 4,
            l1s.version.mcu_tcs_program_release & 0x000F,
            l1s.version.mcu_tcs_official,
            l1s.version.mcu_tcs_internal);
  else
    sprintf(str,"SW version: TCS.%x.%x.%x_L1_%x_%x\n\r",
            (l1s.version.mcu_tcs_program_release & 0xF000) >> 12,
            (l1s.version.mcu_tcs_program_release & 0x0F00) >> 8,
            l1s.version.mcu_tcs_program_release & 0x00FF,
            l1s.version.mcu_tcs_official,
            l1s.version.mcu_tcs_internal);

#if (GSM_IDLE_RAM == 0)
  L1_send_trace_cpy(str);
#else
  l1_intram_put_trace(str);
#endif

#endif


#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )

  vsi_o_event_ttrace(" array index: %d",trace_info.trace_fct_buff_index);

#else
  sprintf (str,"\n\r array index: %d\n\r",trace_info.trace_fct_buff_index);

#if (GSM_IDLE_RAM == 0)
  L1_send_trace_cpy(str);
#else
  l1_intram_put_trace(str);
#endif


#endif


  i = trace_info.trace_fct_buff_index;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )

  for (j=0;j<TRACE_FCT_BUFF_SIZE;j++)
  {
    fct_id = trace_info.trace_fct_buff[i];

    #if TRACE_FULL_NAME
      // display the function by its name (cost in code size and CPU)
      vsi_o_event_ttrace("%d: %d %s",i,fct_id,string_fct_trace[fct_id]);
    #else
      // or display the fct by its id
      vsi_o_event_ttrace("%d: %d\n\r",i,fct_id);
    #endif

    i = (i + 1) % TRACE_FCT_BUFF_SIZE;
  }


#else
  for (j=0;j<TRACE_FCT_BUFF_SIZE;j++)
  {
    fct_id = trace_info.trace_fct_buff[i];

    #if TRACE_FULL_NAME
      // display the function by its name (cost in code size and CPU)
      sprintf (str,"%d: %d %s\n\r",i,fct_id,string_fct_trace[fct_id]);
    #else
      // or display the fct by its id
      sprintf (str,"%d: %d\n\r",i,fct_id);
    #endif

#if (GSM_IDLE_RAM == 0)
    L1_send_trace_cpy(str);
#else
    l1_intram_put_trace(str);
#endif

    i = (i + 1) % TRACE_FCT_BUFF_SIZE;
  }
#endif
  trace_info.trace_buff_stop = FALSE; // start buffer trace
}

///////////////////////////
// Trace type 1 CPU load //
///////////////////////////

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4)) && (CODE_VERSION != SIMULATION)


/* L1S_CPU_load_process()                                */

/* Parameters :                                          */
/* Return     :                                          */

void Trace_L1S_CPU_load()
{
  #define TIMER_RESET_VALUE (0xFFFF)
  #define TICKS_PER_TDMA    (1875)

  unsigned long cpu;

  layer_1_sync_end_time = TIMER_RESET_VALUE - Dtimer2_ReadValue();

  // Trace
  cpu = (100 * layer_1_sync_end_time) / TICKS_PER_TDMA;

  if (cpu > max_cpu)
  {
    max_cpu=cpu;
    fn_max_cpu=l1s.actual_time.fn;
    max_cpu_flag = 1;
  }

  if (((l1s.actual_time.fn%1326) == 0) && (max_cpu_flag == 0))
    max_cpu = 0;
} /* end of Trace_L1S_CPU_load() */

/********** DSP CPU load measurement *************/
#if (DSP >= 38)
void l1_dsp_cpu_load_read()
{

  #define DSP_TIMER_PRESCALER_VALUE (9)

  T_DB_MCU_TO_DSP_CPU_LOAD *mcu_dsp_cpu_load_r_ptr;   // DSP CPU load measurement
  UWORD32     dsp_fgd_tsk_cycles = 0L;
  UWORD16     dsp_tdma_fn;
  UWORD16     d_dsp_page_read;
  UWORD32     d_dsp_work_period;
  UWORD32     d_dsp_fgd_tsk_cycles_per_tdma;
  UWORD16     d_tdma_fnmod4;
  UWORD16     d_tdma_fnmod13;

  // **** 1. Read the DSP FGD task cycles from API ****

  // Check if DSP CPU load has been written in first buffer
  mcu_dsp_cpu_load_r_ptr = (T_DB_MCU_TO_DSP_CPU_LOAD *)DSP_CPU_LOAD_DB_W_PAGE_0;
  if ((API)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 & 0x8000)
  {
     mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 &= ~(0x8000); // reset the bit
     dsp_fgd_tsk_cycles = (UWORD32)(((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 << 16)
                                   + ((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim1));
     dsp_fgd_tsk_cycles = (dsp_fgd_tsk_cycles * DSP_TIMER_PRESCALER_VALUE);

     dsp_tdma_fn        = (API)mcu_dsp_cpu_load_r_ptr->d_tdma_dsp_fn;

     d_dsp_page_read    = 0;
  }
  else
  {
    // Check if DSP CPU load has been written in second buffer
    mcu_dsp_cpu_load_r_ptr = (T_DB_MCU_TO_DSP_CPU_LOAD *)DSP_CPU_LOAD_DB_W_PAGE_1;
    if ((API)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 & 0x8000)
    {
       mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 &= ~(0x8000); // reset the bit
       dsp_fgd_tsk_cycles = (UWORD32)(((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim0 << 16)
                                   + ((UWORD32)mcu_dsp_cpu_load_r_ptr->d_dsp_fgd_tsk_tim1));
       dsp_fgd_tsk_cycles = (dsp_fgd_tsk_cycles * DSP_TIMER_PRESCALER_VALUE);

       dsp_tdma_fn        = (API)mcu_dsp_cpu_load_r_ptr->d_tdma_dsp_fn;

       d_dsp_page_read    = 1;

    }
  }

  // **** 2. Get the number of DSP cycles per TDMA (based on DSP work period) ****
  if (dsp_fgd_tsk_cycles != 0L)
  {
    /* Take care of TDMA FN overflow */
    d_dsp_work_period = (l1s.actual_time.fn_mod42432 - dsp_tdma_fn - 2 + 42432) % 42432;

    d_dsp_fgd_tsk_cycles_per_tdma = dsp_fgd_tsk_cycles/(d_dsp_work_period + 1); // to avoid divide by 0, just in case

    // **** 3. For DSP work-period, update max cycles count ****
    d_tdma_fnmod13 = (l1s.actual_time.fn_mod13 - 1 + 13) % 13;
    d_tdma_fnmod4 = (l1s.actual_time.fn_mod13_mod4 - 1 + 4) % 4;

    if (d_tdma_fnmod13 == 12) //Idle/SACCH/PTCCH frames
    {
      if (dsp_max_cpu_load_idle_frame <= d_dsp_fgd_tsk_cycles_per_tdma)
        dsp_max_cpu_load_idle_frame = d_dsp_fgd_tsk_cycles_per_tdma;
    }
    else // for TDMA frames 0/1/2/3 (mod 4)
    {
      if (dsp_max_cpu_load_trace_array[d_tdma_fnmod4] <= d_dsp_fgd_tsk_cycles_per_tdma)
        dsp_max_cpu_load_trace_array[d_tdma_fnmod4] = d_dsp_fgd_tsk_cycles_per_tdma;
    }

    // **** 4. If 104 TDMA frames have elapsed, print out the DSP CPU cycles ****
    if ((l1s.actual_time.fn_mod42432 % 104) == 0)
    {
      l1_dsp_cpu_load_trace_flag = 1;
    }
  }
} /* end of l1_dsp_cpu_load_read() */
#endif	// DSP >= 38

    #endif //((TRACE_TYPE == 1) && (CODE_VERSION != SIMULATION))
  #endif // (TRACE_TYPE == 1) // (TRACE_TYPE == 4)
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

/////////////////////////////////////
// Generic function for L1S traces //
/////////////////////////////////////


/* Trace_params()                                        */

/* Parameters :                                          */
/* Return     :                                          */
/* Description: This function can be used to quickly add */
/*              a trace                                  */
/*              NOT TO USE FOR PERMANENT TRACES !!!      */

void  Trace_params(UWORD8   debug_code,
                   UWORD32  param0,
                   UWORD32  param1,
                   UWORD32  param2,
                   UWORD32  param3,
                   UWORD32  param4,
                   UWORD32  param5,
                   UWORD32  param6)
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = debug_code;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0] = param0;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1] = param1;
  ((T_TRACE_INFO *)(msg->SigP))->tab[2] = param2;
  ((T_TRACE_INFO *)(msg->SigP))->tab[3] = param3;
  ((T_TRACE_INFO *)(msg->SigP))->tab[4] = param4;
  ((T_TRACE_INFO *)(msg->SigP))->tab[5] = param5;
  ((T_TRACE_INFO *)(msg->SigP))->tab[6] = param6;
  ((T_TRACE_INFO *)(msg->SigP))->tab[7] = l1s.actual_time.fn;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#if (TRACE_TYPE == 4)

////////////////////////////////////////
// Dynamic trace configuration change //
////////////////////////////////////////


/* Trace_dyn_trace_change()                              */

/* Parameters :                                          */
/* Return     :                                          */

void Trace_dyn_trace_change()
{
  xSignalHeaderRec *msg;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = DYN_TRACE_CHANGE;

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
}

#endif // TRACE_TYPE 4
#endif // TRACE TYPE 1 TRACE TYPE 4
#endif // NOT L1_BINARY_TRACE

/************************************ CPU load trace ******************************************************/

#endif  // (TRACE_TYPE==1) || (TRACE_TYPE==4) || (TRACE_TYPE==5)

#if (TRACE_TYPE==7)


/* l1_trace_message()                                    */

/* Parameters :                                          */
/* Return     :                                          */
/* Description: L1 Trace formatting.                     */

void l1_trace_message(xSignalHeaderRec *msg)
{
  char *str;


  if (msg->SignalCode == TRACE_INFO)
  {
    // If memory allocation is OK
    if (rvt_mem_alloc(trace_info.l1_trace_user_id, 200, (T_RVT_BUFFER *) &str) == RVT_OK)
    {

       UWORD8 debug_code = ((T_TRACE_INFO *)(msg->SigP))->debug_code;

       switch(debug_code)
       {
          case TRACE_CPU_LOAD:
          {
            UWORD8 i;
            static char str_r[16];


            str[0]   = '\0';
            str_r[0] = '\0';

            for (i=0;i<C_MESURE_DEPTH;i++)
            {
              if (((T_TRACE_INFO_CPU_LOAD *)(msg->SigP))->tab[i].valid == TRUE)
              {
                sprintf (str_r,"%d %d\n\r",
                        ((T_TRACE_INFO_CPU_LOAD *)(msg->SigP))->tab[i].cpu,
                        ((T_TRACE_INFO_CPU_LOAD *)(msg->SigP))->tab[i].fn);

                strcat(str,str_r);
               } // end if

             } // end for

             L1_send_trace_no_cpy(str); // Send to Trace task

            } // End case
          break;

          default:
          {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
            vsi_o_event_ttrace("DEB_I %ld %ld %ld %ld %ld %ld %ld %ld",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
#else
            sprintf (str,"DEB_I %ld %ld %ld %ld %ld %ld %ld %ld\n\r",
                     ((T_TRACE_INFO *)(msg->SigP))->tab[0],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[1],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[2],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[3],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[4],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[5],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[6],
                     ((T_TRACE_INFO *)(msg->SigP))->tab[7]);
            L1_send_trace_no_cpy(str);
#endif
          }
          break;

        } // End switch

    } // End if "memory allocation OK"

  } // End if msg->SignalCode == TRACE_INFO


} // l1_trace_message


/* l1_cpu_load_init()                                    */

/* Parameters :                                          */
/* Return     :                                          */

void l1_cpu_load_init()
{

  UWORD8            i;

  d_mesure_index                  = 0;

  for (i=0;i<C_MESURE_DEPTH;i++)
    d_mesure[i].valid             = FALSE;


} //l1_cpu_load_init


/* l1_trace_buf_meas()                                   */
/* Parameters :                                          */
/* Return     :                                          */


void l1_trace_buf_meas()
{

  xSignalHeaderRec *msg;
  UWORD8            i;

  // Allocate DEBUG message.
  msg = os_alloc_sig(sizeof(T_TRACE_INFO_CPU_LOAD));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO_CPU_LOAD *)(msg->SigP))->debug_code = TRACE_CPU_LOAD;

  for (i=0;i<C_MESURE_DEPTH;i++)
    {
      ((T_TRACE_INFO_CPU_LOAD *)(msg->SigP))->tab[i] = d_mesure[i];
      d_mesure[i].valid = FALSE;
    }

  // send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
} // l1_trace_buf_meas


/* l1_cpu_load_start()                                   */
/* Parameters :                                          */
/* Return     :                                          */

void l1_cpu_load_start()
{

  // Reset hardware timer
  Dtimer2_Start(0);
  // Timers initialization
  Dtimer2_Init_cntl(CPU_LOAD_TIMER_RESET_VALUE,0,C_PTV,1);    // PTV = 2 --> Tick = 9.85 us
                                                              // PTV = 0 --> Tick = 2.416 us
  // Start hardware timer
  Dtimer2_Start(1);

} //l1_cpu_load_start


/* l1_cpu_load_stop()                                    */
/* Parameters :                                          */
/* Return     :                                          */

void l1_cpu_load_stop()
{

  UWORD8        i;
  UWORD16       l1s_cpu_load_end;

  // Stop hardware timer
  Dtimer2_Start(0);

  l1s_cpu_load_end = Dtimer2_ReadValue();


  // Tint = Tclk * (LOAD_TIM+1) * 2^(PTV+1)
  // Tclk = 1.2308us for Fclk=13Mhz
  // PTV  = X (pre-scaler field)


  d_mesure[d_mesure_index].cpu     = (UWORD16)((CPU_LOAD_TIMER_RESET_VALUE - l1s_cpu_load_end) * CPU_LOAD_TICK);
  d_mesure[d_mesure_index].fn      = l1s.actual_time.fn_mod104;
  d_mesure[d_mesure_index++].valid = TRUE;

  //=================================================
  // Compute result on less loaded TDMA
  //-------------------------------------------------

  if (l1s.actual_time.fn_mod13 == 11)
  {
    l1_trace_buf_meas();
    d_mesure_index = 0;
  } // End if

} // l1_cpu_load_stop


/* l1_cpu_load_interm()                                  */

/* Parameters :                                          */
/* Return     :                                          */

void l1_cpu_load_interm()
{

  UWORD8        i;
  UWORD16       l1s_cpu_load_end;

  // Stop hardware timer
  Dtimer2_Start(0);

  l1s_cpu_load_end = Dtimer2_ReadValue();

  //=================================================
  // Tint = Tclk * (LOAD_TIM+1) * 2^(PTV+1)
  // Tclk = 1.2308us for Fclk=13Mhz
  // PTV  = X (pre-scaler field)
  //-------------------------------------------------

  d_mesure[d_mesure_index].cpu_access  = (UWORD16)((CPU_LOAD_TIMER_RESET_VALUE - l1s_cpu_load_end) * CPU_LOAD_TICK);
} // l1_cpu_load_interm
#endif //(TRACE_TYPE == 7)

#if (TRACE_TYPE==6)


/* l1_trace_cpu_load()                                   */

/* Parameters :                                          */
/* Return     :                                          */

void l1_trace_cpu_load(UWORD8 cpu_load)
{
  char      str[240];
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
   vsi_o_event_ttrace("%d", cpu_load);
#else
  sprintf(str,"%d\n\r", cpu_load);
  L1_send_trace_cpy(str);
#endif
}

#endif // TRACE_TYPE 6



/* Trace functions also used for recovery */


#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise


/* l1_check_pm_error()                                   */
/* Parameters :                                          */
/* Return     :                                          */
/* NEW COMPILER MANAGEMENT
 * Removal of inline on l1_check_pm_error.
 * With new compiler, inline means static inline involving the
 * function to not be seen outside this file */
void l1_check_pm_error(UWORD32 pm,UWORD8 task)
{
   #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
       Trace_PM_Equal_0(pm,task);
   #endif

   #if L1_RECOVERY
     if (pm==0)
     {
       l1s.recovery.frame_count++;

       // Recovery timer has expired => send autorecovery message to L1A
       // 100: arbitrary value, corresponds to about 0.5 s
       if (l1s.recovery.frame_count >= 100)
       {
         // Set recovery flag, this flag will be checked by L1A
         l1a_l1s_com.recovery_flag = TRUE;

         // Reset error flags and counter
         l1s.recovery.frame_count  = 0;
       }
     }
   #endif
}

/* l1_check_com_mismatch()                               */
/* Parameters :                                          */
/* Return     :                                          */
/* NEW COMPILER MANAGEMENT
 * Removal of inline on l1_check_com_mismatch.
 * With new compiler, inline means static inline involving the
 * function to not be seen outside this file*/

void l1_check_com_mismatch(UWORD8 task)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      Trace_MCU_DSP_Com_Mismatch(task);
  #endif

  #if L1_RECOVERY
    if((l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff ) != (l1s.debug_time & 0xffff ))
    {
       l1s.recovery.frame_count++;

      // Recovery timer has expired => send autorecovery message to L1A
      // 100: arbitrary value, corresponds to about 0.5 s
      if (l1s.recovery.frame_count >= 100)
      {
        // Set recovery flag, this flag will be checked by L1A
        l1a_l1s_com.recovery_flag = TRUE;

        // Reset error flags and counter
        l1s.recovery.frame_count  = 0;
      }
    }
  #endif
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif


/* Functions also used for simulation */


#if ((GSM_IDLE_RAM != 0)) //omaps00090550
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START
  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))

  void l1_intram_send_trace(void)
  {

    UWORD8 * start_adr, str_index;
    T_RVT_MSG_LG size;
    UWORD8 str[INTRAM_TRACE_BUFFER_SIZE];

    str_index=0;
    if (intram_trace_size != 0)
      {
        if((intram_buffer_current_ptr - intram_trace_size) < intram_trace_buffer)
        {
          start_adr = (UWORD8 *) (intram_buffer_current_ptr - intram_trace_size);
          start_adr += INTRAM_TRACE_BUFFER_SIZE;

          size =  intram_trace_size - ((T_RVT_MSG_LG) (intram_buffer_current_ptr - intram_trace_buffer));
          strncpy((CHAR*) (&str[str_index]), (CHAR*)start_adr,(size_t) size);
          str_index+=size;
          size = (T_RVT_MSG_LG) (intram_buffer_current_ptr - intram_trace_buffer);
          strncpy((CHAR*)(&str[str_index]), (CHAR*)intram_trace_buffer, (size_t) size);
        }
        else
        {
          size = intram_trace_size;
          start_adr=(UWORD8 *) (intram_buffer_current_ptr - intram_trace_size);
          strncpy((CHAR*) (&str[str_index]), (CHAR*)start_adr,(size_t) size);
        }
        str_index+=size;
        str[str_index]=0x00;
        rvt_send_trace_cpy    ((T_RVT_BUFFER) (str),trace_info.l1_trace_user_id, (T_RVT_MSG_LG) (intram_trace_size+1), RVT_ASCII_FORMAT);
        intram_trace_size=0;
      }
  }

  void l1_intram_put_trace(CHAR * msg)
  {

    UWORD8 index, string_size;


    string_size=strlen((CHAR*) msg);

    for(index=0;index<string_size;index++)
    {
     *(intram_buffer_current_ptr++)=msg[index];

     if (intram_trace_size < INTRAM_TRACE_BUFFER_SIZE) intram_trace_size++;
     if (intram_buffer_current_ptr == (intram_trace_buffer + INTRAM_TRACE_BUFFER_SIZE))
     {
      intram_buffer_current_ptr-=INTRAM_TRACE_BUFFER_SIZE;
     }
    }
  }

  void  l1s_trace_mftab(void)
  {
      WORD8 index, offset=0, nb_bitmap;
      UWORD8 diff_detected=0;
      CHAR str2[SIZE_TAB_L1S_MONITOR*8+128];
      UWORD8 count=0;
      T_L1S_GSM_IDLE_INTRAM * gsm_idle_ram_ctl;

      gsm_idle_ram_ctl = &(l1s.gsm_idle_ram_ctl);
#if (CODE_VERSION == NOT_SIMULATION)
    #if ((GSM_IDLE_RAM_DEBUG == 1) && (CHIPSET == 12))
          offset=sprintf(str2,"TASK  %ld %ld %ld %ld %ld |%ld| (CS5/CS4:%ld,%ld) ", (l1s.actual_time.fn%42432), l1s.gsm_idle_ram_ctl.l1s_full_exec, gsm_idle_ram_ctl->os_load, gsm_idle_ram_ctl->hw_timer, gsm_idle_ram_ctl->sleep_mode, READ_TRAFFIC_CONT_STATE, l1s.gsm_idle_ram_ctl.killing_flash_access, l1s.gsm_idle_ram_ctl.killing_ext_ram_access);
    #else
          offset=sprintf(str2,"TASK  %ld %ld %ld %ld |%ld| ", (l1s.actual_time.fn%42432), l1s.gsm_idle_ram_ctl.l1s_full_exec, gsm_idle_ram_ctl->os_load, gsm_idle_ram_ctl->hw_timer, READ_TRAFFIC_CONT_STATE);
    #endif
#endif
          for(nb_bitmap=(SIZE_TAB_L1S_MONITOR-1); nb_bitmap>=0; nb_bitmap--)
          {
              for (index=7; index>=0; index--)
              {
                  count = (gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap] >> (4*index)) & 0xF;

                  if (count < 10)
                      str2[offset + (1-nb_bitmap)*8+(7-index)]= '0'+ count;
                  else
                      str2[offset + (1-nb_bitmap)*8+(7-index)]= '7'+ count;
              }
          }
          str2[offset + 16]  = '\n';
          str2[offset + 17]= '\r';
          str2[offset + 18]= 0;

          l1_intram_put_trace(str2);
    #if GSM_IDLE_RAM_DEBUG
          gsm_idle_ram_ctl->sleep_mode = 999;
    #endif
}


  /* l1_trace_IT_DSP_error_intram()                        */
  /* Parameters :                                          */
  /* Return     :                                          */

  void l1_trace_IT_DSP_error_intram(void)
  {
  char str[64];

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

       trace_info.l1_memorize_error = '.'; // memorize an error in the L1

       if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
       {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
         vsi_o_event_ttrace(">  MCU CPU overload %ld\n\r", l1s.actual_time.fn_mod42432);
#else
         sprintf (str,">  MCU CPU overload %ld\n\r", l1s.actual_time.fn_mod42432);
         l1_intram_put_trace(str);
#endif
       }
    #endif
  }
  #endif //((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))

#if (TRACE_TYPE == 7)

  /* l1_trace_buf_meas_intram()                            */
  /* Parameters :                                          */
  /* Return     :                                          */


  void l1_trace_buf_meas_intram()
  {

    UWORD8            i;
    static char str_r[16];

    str[0]   = '\0';
    str_r[0] = '\0';

    for (i=0;i<C_MESURE_DEPTH;i++)
    {
      if (d_mesure[i].valid == TRUE)
      {
        sprintf (str_r,"%d %d\n\r",
                d_mesure[i].cpu,
                d_mesure[i].fn);

        strcat(str,str_r);
      } // end if
    } // end for

    l1_intram_put_trace(str); // Send to Trace task
#endif

#if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))

  /* l1_trace_sleep_intram()                               */
  /* Parameters :                                          */
  /* Return     :                                          */


  void l1_trace_sleep_intram(UWORD32 start_fn, UWORD32 end_fn, UWORD8 type_sleep,UWORD8 wakeup_type,UWORD8 big_sleep_type, UWORD16 int_id)
  {
    char      str[64];
    #if  (TRACE_TYPE==2) || (TRACE_TYPE==3)

      if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
        L1_trace_char('-');
      else
        L1_trace_char('b');

    #elif (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

      #if (L1_BINARY_TRACE)
  /*
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {

        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TR_SLEEP));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_INFO;

        ((T_TR_SLEEP *)(msg->SigP))->header         = TRL1_SLEEP | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
        ((T_TR_SLEEP *)(msg->SigP))->start_fn       = start_fn;
        ((T_TR_SLEEP *)(msg->SigP))->end_fn         = end_fn;
        ((T_TR_SLEEP *)(msg->SigP))->type_sleep     = type_sleep;
        ((T_TR_SLEEP *)(msg->SigP))->wakeup_type    = wakeup_type;
        ((T_TR_SLEEP *)(msg->SigP))->big_sleep_type = big_sleep_type;

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

      }
  */
      #else

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
         char *array_special_cases[]={"", "ASYNC_0", "SLEEP_0"};
         UWORD8 index = 0;
         if (wakeup_type == WAKEUP_ASYNCHRONOUS_ULPD_0)
           index = 1;
         else if (wakeup_type == WAKEUP_ASYNCHRONOUS_SLEEP_DURATION_0)
           index = 2;
         else
           index = 0;

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
         if (type_sleep == CLOCK_STOP)
         // deep sleep trace
         {
           vsi_o_event_ttrace("      deep_sleep: %ld %ld %d %d %s",
                    start_fn,
                    end_fn,
                    wakeup_type,
                    int_id,
                    array_special_cases[index]);
         }
         else
         // big sleep
         {
           char *array_string[]={"undefined","ligth on","uart","sim","gauging","sleep mode","DSP","BT","camera","??"};
           vsi_o_event_ttrace("      big sleep: %ld %ld %d %d cause:%s %s",
                    start_fn,
                    end_fn,
                    wakeup_type,
                    int_id,
                    array_string[big_sleep_type],
                    array_special_cases[index]);
         }

#else

         if (type_sleep == CLOCK_STOP)
         // deep sleep trace
         {
           sprintf (str,"      deep_sleep: %ld %ld %d %d %s\n\r",
                    start_fn,
                    end_fn,
                    wakeup_type,
                    int_id,
                    array_special_cases[index]);
         }
         else
         // big sleep
         {
           char *array_string[]={"undefined","ligth on","uart","sim","gauging","sleep mode","DSP","BT","camera","??"};
           sprintf (str,"      big sleep: %ld %ld %d %d (cause:%s) %s\n\r",
                    start_fn,
                    end_fn,
                    wakeup_type,
                    int_id,
                    array_string[big_sleep_type],
                    array_special_cases[index]);
         }
         l1_intram_put_trace(str);
#endif
      }

      #endif
    #endif
  }

  /* l1_trace_gauging_intram()                             */
  /* Parameters :                                          */
  /* Return     :                                          */
  /* Trace the gauging is running                          */

  void l1_trace_gauging_intram(void)
  {
    char      str[64];
      #if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)

         L1_trace_char('G');  // trace the gauging

      #elif (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

        #if (L1_BINARY_TRACE)
  /*
        if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
        {
          xSignalHeaderRec *msg;

          // Allocate DEBUG message.
          msg = os_alloc_sig(sizeof(T_TR_GAUGING));
          DEBUGMSG(status,NU_ALLOC_ERR)
          msg->SignalCode = TRACE_INFO;

          ((T_TR_GAUGING *)(msg->SigP))->header        = TRL1_GAUGING | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
          ((T_TR_GAUGING *)(msg->SigP))->reset_gauging = trace_info.reset_gauging_algo;

          // send message...
          os_send_sig(msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        }
  */
        #else

        if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
        {

           if (trace_info.reset_gauging_algo == TRUE)
           {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
             vsi_o_event_ttrace("      reset gauging algo");

#else
             sprintf (str,"      reset gauging algo");
             l1_intram_put_trace(str);
#endif
             trace_info.reset_gauging_algo = FALSE;
           }

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
           vsi_o_event_ttrace("      gauging %ld", l1s.actual_time.fn_mod42432);

#else
           sprintf (str,"      gauging %ld", l1s.actual_time.fn_mod42432);
           l1_intram_put_trace(str);
#endif
        }

        #endif

      #endif
  }


  /* l1_trace_ADC_intram()                                 */
  /* Parameters :                                          */
  /* Return     :                                          */

  void l1_trace_ADC_intram(UWORD8 type)
  {
    char      str[64];

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
       if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
       {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
          vsi_o_event_ttrace("      ADC :%ld %ld",
                   l1s.actual_time.fn_mod42432,
                   type);

#else
          sprintf (str,"      ADC :%ld %ld\n\r",
                   l1s.actual_time.fn_mod42432,
                   type);
          l1_intram_put_trace(str);

#endif
       }
    #endif
  }


  /* l1_trace_new_toa_intram()                             */
  /* Parameters :                                          */
  /* Return     :                                          */

  void l1_trace_new_toa_intram(void)
  {
    char      str[64];
    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
       if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
       {
         #if (TOA_ALGO == 2)
           vsi_o_event_ttrace("      TOA updated:%ld %ld %ld %ld %ld",
                    l1s.actual_time.fn_mod42432,
                    l1s.toa_var.toa_shift,
                    trace_info.toa_trace_var.toa_frames_counter,
                    trace_info.toa_trace_var.toa_accumul_counter,
                    trace_info.toa_trace_var.toa_accumul_value);
         #else
           vsi_o_event_ttrace("      TOA updated:%ld %ld",
                    l1s.actual_time.fn_mod42432,
                    l1s.toa_shift);
         #endif

       }

#else
       if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1A_MESSAGES)
       {
         #if (TOA_ALGO == 2)
           sprintf (str,"      TOA updated:%ld %ld %ld %ld %ld\n\r",
                    l1s.actual_time.fn_mod42432,
                    l1s.toa_var.toa_shift,
                    trace_info.toa_trace_var.toa_frames_counter,
                    trace_info.toa_trace_var.toa_accumul_counter,
                    trace_info.toa_trace_var.toa_accumul_value);
         #else
           sprintf (str,"      TOA updated:%ld %ld\n\r",
                    l1s.actual_time.fn_mod42432,
                    l1s.toa_shift);
         #endif

         l1_intram_put_trace(str);
       }
#endif
    #endif
  }

  ///////////////////////
  // DSP error traces  //
  ///////////////////////

  #if (D_ERROR_STATUS_TRACE_ENABLE)


  /* Trace_d_error_status_intram()                         */
  /* Parameters :                                          */
  /* Return     :                                          */

  void Trace_d_error_status_intram()
  {
    CHAR str[128];

    #if L1_GPRS
      UWORD16           d_error_status_masked =
            (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
            (trace_info.d_error_status_masks[l1a_l1s_com.dsp_scheduler_mode - 1]); // depends on the scheduler mode
    #else
      UWORD16           d_error_status_masked =
            (l1s_dsp_com.dsp_ndb_ptr->d_error_status) &
            (trace_info.d_error_status_masks[GSM_SCHEDULER - 1]);
    #endif
    UWORD16 changed_bits = d_error_status_masked ^ trace_info.d_error_status_old;

    // trace in case of change of status (field is reseted on change of scheduler)
    if (changed_bits)
    {

      // trace the d_error_status word with the correct mask applied
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
      vsi_o_event_ttrace("> DSP %ld %ld %xh %ld",
                 l1s.actual_time.fn_mod42432,
                 (UWORD16)l1s.debug_time,
                 d_error_status_masked,
                 (l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff));

#else
      sprintf (str,"> DSP %ld %ld %xh %ld\n\r",
                 l1s.actual_time.fn_mod42432,
                 (UWORD16)l1s.debug_time,
                 d_error_status_masked,
                 (l1s_dsp_com.dsp_db_r_ptr->d_debug & 0xffff));
      l1_intram_put_trace(str);
#endif
      #if (DSP_DEBUG_TRACE_ENABLE == 1)
        if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
        {
          // DSP debug buffer trace only if an ERROR is detected (not for a End of error detection)
          if ((changed_bits & d_error_status_masked) & ~trace_info.d_error_status_old)
          {
            // Flag DSP error for DSP trace and memorize address of start of DSP trace
            DSP_DEBUG_ENABLE
          }
        }
      #endif

      trace_info.d_error_status_old = d_error_status_masked;
    }

    // Clear bits that have been set by the DSP
    l1s_dsp_com.dsp_ndb_ptr->d_error_status &= ~d_error_status_masked;
  }

  #endif //(D_ERROR_STATUS_TRACE_ENABLE)
#endif //TRACE TYPE
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif //MOVE INTERNAL RAM


#if (AMR == 1)

/* l1_trace_ratscch()                                    */
/* Parameters :                                          */
/* Return     :                                          */

void l1_trace_ratscch(UWORD16 fn, UWORD16 amr_change_bitmap)
{
#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
  xSignalHeaderRec *msg;

  // Allocate trace message
  msg = os_alloc_sig(sizeof(T_TRACE_INFO));
  DEBUGMSG(status,NU_ALLOC_ERR)
  msg->SignalCode = TRACE_INFO;

  ((T_TRACE_INFO *)(msg->SigP))->debug_code = TRACE_RATSCCH;

  ((T_TRACE_INFO *)(msg->SigP))->tab[0]=fn;
  ((T_TRACE_INFO *)(msg->SigP))->tab[1]=amr_change_bitmap;  // amr_change_bitmap contains the a bitmap of the AMR parameters updated

  // Send message...
  os_send_sig(msg, L1C1_QUEUE);
  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
#endif    // TRACE_TYPE
}
#endif    // AMR

void l1_trace_fail_sleep(UWORD8 pwmgr_fail_step, UWORD8 pwmgr_fail_id, UWORD8 pwmgr_fail_cause)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_PWMGT_FAIL_DEBUG)
     {
       // Trace Only when Fail Step or Fail ID differs
       if((trace_info.pwmgt_trace_var.fail_step != pwmgr_fail_step) ||
          (trace_info.pwmgt_trace_var.fail_id != pwmgr_fail_id))
       {
         // Allocate DEBUG message.
         msg = os_alloc_sig(sizeof(T_TRACE_INFO));
         DEBUGMSG(status,NU_ALLOC_ERR)
         msg->SignalCode = TRACE_INFO;

         ((T_TRACE_INFO *)(msg->SigP))->debug_code = PWMGT_FAIL_SLEEP;

         trace_info.pwmgt_trace_var.fail_step  = pwmgr_fail_step;
         trace_info.pwmgt_trace_var.fail_id    = pwmgr_fail_id;
         trace_info.pwmgt_trace_var.fail_cause = pwmgr_fail_cause;

         ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
         ((T_TRACE_INFO *)(msg->SigP))->tab[1] = pwmgr_fail_step;
         ((T_TRACE_INFO *)(msg->SigP))->tab[2] = pwmgr_fail_id;
         ((T_TRACE_INFO *)(msg->SigP))->tab[3] = pwmgr_fail_cause;

         // send message...
         os_send_sig(msg, L1C1_QUEUE);
         DEBUGMSG(status,NU_SEND_QUEUE_ERR)
       }
     }
  #endif


}


/* l1_trace_sleep()                                      */
/* Parameters :                                          */
/* Return     :                                          */

void l1_trace_sleep(UWORD32 start_fn, UWORD32 end_fn, UWORD8 type_sleep,UWORD8 wakeup_type,UWORD8 big_sleep_type, UWORD16 int_id)
{
  #if (CODE_VERSION == SIMULATION)

    #if (TRACE_TYPE==5)
      if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
      {
        trace_ULPD("Start  Deep Sleep", start_fn);
        trace_ULPD("Wakeup Deep Sleep", l1s.actual_time.fn);
      }
      else
      {
        trace_ULPD("Start  Big Sleep", start_fn);
        trace_ULPD("Wakeup Big Sleep", l1s.actual_time.fn);
      }
    #endif

  #else

    #if  (TRACE_TYPE==2) || (TRACE_TYPE==3)
      if ( l1s.pw_mgr.sleep_performed == CLOCK_STOP )
        L1_trace_char('-');
      else
        L1_trace_char('b');
    #elif (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

      #if (L1_BINARY_TRACE)

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TR_SLEEP));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_INFO;

        ((T_TR_SLEEP *)(msg->SigP))->header         = TRL1_SLEEP | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
        ((T_TR_SLEEP *)(msg->SigP))->start_fn       = start_fn;
        ((T_TR_SLEEP *)(msg->SigP))->end_fn         = end_fn;
        ((T_TR_SLEEP *)(msg->SigP))->type_sleep     = type_sleep;
        ((T_TR_SLEEP *)(msg->SigP))->wakeup_type    = wakeup_type;
        ((T_TR_SLEEP *)(msg->SigP))->big_sleep_type = big_sleep_type;
        ((T_TR_SLEEP *)(msg->SigP))->int_id         = int_id;

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }

      #else

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TRACE_INFO));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_INFO;

        ((T_TRACE_INFO *)(msg->SigP))->debug_code = TRACE_SLEEP;

        ((T_TRACE_INFO *)(msg->SigP))->tab[0] = start_fn;
        ((T_TRACE_INFO *)(msg->SigP))->tab[1] = end_fn;
        ((T_TRACE_INFO *)(msg->SigP))->tab[2] = type_sleep;
        ((T_TRACE_INFO *)(msg->SigP))->tab[3] = wakeup_type;
        ((T_TRACE_INFO *)(msg->SigP))->tab[4] = big_sleep_type;
        ((T_TRACE_INFO *)(msg->SigP))->tab[5] = int_id;

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }

      #endif
    #endif
  #endif
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START


/* l1_trace_gauging_reset()                              */
/* Parameters :                                          */
/* Return     :                                          */
/* This means instability with the 32Khz                 */

void l1_trace_gauging_reset(void)
{

  #if (CODE_VERSION != SIMULATION)

    #if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)
      // trace if the gauging can't succeed
      L1_trace_char('#');
    #elif (TRACE_TYPE !=0 )
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
        // to trace during Gauging interrupt causes issue with Pool memory.
        // the trace will be done with the next gauging.
        trace_info.reset_gauging_algo = TRUE;  // trace Reset gauging Algorithm
      }
    #endif

  #else  // Simulation part

    #if (TRACE_TYPE==5)
      trace_ULPD("Reset Gauging algorithm", l1s.actual_time.fn);
    #endif

  #endif // Simulation part
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif // !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))


/* l1_trace_gauging()                                    */
/* Parameters :                                          */
/* Return     :                                          */
/* Trace the gauging is running                          */

void l1_trace_gauging(void)
{
  #if (CODE_VERSION == SIMULATION)

     #if (TRACE_TYPE==5)
        trace_ULPD("Start Gauging", l1s.actual_time.fn);
     #endif

  #else

    #if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)
       L1_trace_char('G');  // trace the gauging
    #elif (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

      #if (L1_BINARY_TRACE)

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TR_GAUGING));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_INFO;

        ((T_TR_GAUGING *)(msg->SigP))->header        = TRL1_GAUGING | (l1s.actual_time.fn << TR_HEADER_FN_DELAY);
        ((T_TR_GAUGING *)(msg->SigP))->reset_gauging = trace_info.reset_gauging_algo;

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }

      #else

      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_ULPD)
      {
        xSignalHeaderRec *msg;

        // Allocate DEBUG message.
        msg = os_alloc_sig(sizeof(T_TRACE_INFO));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = TRACE_INFO;

        ((T_TRACE_INFO *)(msg->SigP))->debug_code = TRACE_GAUGING;

        ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;

        if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_GAUGING)
        {
          // dynamic trace for all gauging parameters
          ((T_TRACE_INFO *)(msg->SigP))->tab[1] = l1s.pw_mgr.state;
          ((T_TRACE_INFO *)(msg->SigP))->tab[2] = l1s.pw_mgr.lf;
          ((T_TRACE_INFO *)(msg->SigP))->tab[3] = l1s.pw_mgr.hf;
          ((T_TRACE_INFO *)(msg->SigP))->tab[4] = l1s.pw_mgr.root;
          ((T_TRACE_INFO *)(msg->SigP))->tab[5] = l1s.pw_mgr.frac;
        }

        // send message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
      #endif
    #endif
  #endif
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START

/* trace_fct()                                           */
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */

void trace_fct(UWORD8 fct_id, UWORD32 radio_freq)
{
#if (TRACE_TYPE==1) || (TRACE_TYPE==4)

  #if (L1_BINARY_TRACE)
    RTTL1_FILL_MFTAB(fct_id)
  #else

  if (trace_info.trace_buff_stop == FALSE) // start buffer trace
  {
    trace_info.trace_fct_buff[trace_info.trace_fct_buff_index] = fct_id;

    trace_info.trace_fct_buff_index++;
    if (trace_info.trace_fct_buff_index >= TRACE_FCT_BUFF_SIZE)
      trace_info.trace_fct_buff_index = 0;
  }

  #endif

#endif

#if (TRACE_TYPE==5)
  trace_fct_simu(string_fct_trace[fct_id],radio_freq);
#endif
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif


#if (((TRACE_TYPE==1) || (TRACE_TYPE == 4)))
  UWORD16 reentry = 0;

  void l1_trace_full_dsp_buffer( void )
  {
    UWORD16 print_loop;
    UWORD16 i;
    UWORD32 STORE_ADDRESS = 0;
    char str[255];
    char str2[7];

    // mask IT TDMA Interrupt frame

    #if (CHIPSET == 12) || (CHIPSET == 15)
        F_INTH_DISABLE_ONE_IT(C_INTH_FRAME_IT); // Mask IT FRAME
    #else
        INTH_DISABLEONEIT(IQ_FRAME);          // Mask IT FRAME int.
    #endif

    if (reentry == 0)
    {
      STORE_ADDRESS = 0xFFD00000 + l1s_dsp_com.dsp_ndb_ptr->p_debug_buffer + 0x07FD;
      print_loop = (l1s_dsp_com.dsp_ndb_ptr->d_debug_buffer_size) + 2;
      while ( print_loop > 0 )
      {
        if ( print_loop > 41 )
        {
          sprintf(str,"%04x\n\r", *((UWORD16 *)STORE_ADDRESS));
          STORE_ADDRESS +=2;
          for (i=0; i<41; i++)
          {
            sprintf(str2,"%04x\n\r", *((UWORD16 *)STORE_ADDRESS));
            STORE_ADDRESS +=2;
            strcat(str,str2);
          }
          L1_send_trace_cpy(str);
          print_loop -= 42;
        }
        else
        {
          sprintf(str,"%04x\n\r", *((UWORD16 *)STORE_ADDRESS));
          STORE_ADDRESS +=2;
          print_loop--;
          while ( print_loop > 0 )
          {
            sprintf(str2,"%04x\n\r", *((UWORD16 *)STORE_ADDRESS));
            STORE_ADDRESS +=2;
            strcat(str,str2);
            print_loop--;
          }
          L1_send_trace_cpy(str);
        }
      }
      reentry += 1;
    }
}

#endif  // C_DEBUG_TRACE_TYPE


#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
#if (MELODY_E2 || L1_MP3 ||  L1_AAC || L1_DYN_DSP_DWNLD )


/* l1_disable_dsp_trace                                  */
/* Parameters :  none                                    */
/* Return     :  none                                    */
/* Description :  Disables DSP trace                     */


void l1_disable_dsp_trace()
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  xSignalHeaderRec *msg;

  // Only lower nested level disable triggers Disable DSP Trace message to DSP
  if (trace_info.dsptrace_handler_globals.nested_disable_count == 0)
  {
    trace_info.dsptrace_handler_globals.trace_flag_blocked = TRUE;

    if (dsp_ndb_ptr->d_debug_trace_type != 0x0000)
    {
      trace_info.dsptrace_handler_globals.nested_disable_count++;

      // save trace type configuration, set re-init buffer (0x1000) and enable DSP trace (0x8000)
      trace_info.dsptrace_handler_globals.dsp_trace_level_copy = dsp_ndb_ptr->d_debug_trace_type;
      dsp_ndb_ptr->d_debug_trace_type = (API)0x8000; // 0x8000 Disable DSP Trace and 0x1000 Re-init Trace buffer

       /*
	* FreeCalypso note: the LoCosto version of L1 was writing 0x9000 into
	* dsp_ndb_ptr->d_debug_trace_type.  When we got l1_dyn_dwl code
	* integrated, we started getting DSP erratic behaviour when trying
	* to make a voice call (probably when going into TCH).  TCS211 version
	* of l1_disable_DSP_trace() writes 0x8000 here instead.  Changing
	* the code here to write 0x8000 instead of 0x9000 fixed that erratic
	* behaviour.  Evidently this "re-init buffer" 0x1000 bit must be a
	* LoCosto-ism that does not apply correctly to the Calypso.
	*/

      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TRACE_INFO));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TRACE_INFO *)(msg->SigP))->debug_code = DSP_TRACE_DISABLE;
      dsp_ndb_ptr->d_debug_buffer_size = 0;

      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)

#if (DSP_DEBUG_TRACE_ENABLE)
      trace_info.dsp_debug_buf_start[0] = NULL;
      trace_info.dsp_debug_buf_start[1] = NULL;
#endif
    }
  }
  // In case higher nested levels disable, increment nested counter
  else if (trace_info.dsptrace_handler_globals.nested_disable_count > 0)
  {
    trace_info.dsptrace_handler_globals.nested_disable_count++;
  }
}


/* l1_enable_dsp_trace                                   */
/*-------------------------------------------------------*/
/*                                                       */
/* Parameters :  none                                    */
/* Return     :  none                                    */
/*                                                       */
/* Description :  Enables DSP trace                      */


void l1_enable_dsp_trace()
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  xSignalHeaderRec *msg;

  if ((trace_info.dsptrace_handler_globals.trace_flag_blocked == TRUE) && (dsp_ndb_ptr->d_debug_trace_type == 0x0000))
  {
    // Only lower nested level disable triggers Enable DSP Trace message to DSP
    if (trace_info.dsptrace_handler_globals.nested_disable_count == 1)
    {
      trace_info.dsptrace_handler_globals.trace_flag_blocked = FALSE;
      trace_info.dsptrace_handler_globals.nested_disable_count--;

      // restore trace type configuration, set re-init buffer (0x1000) and enable DSP trace (0x8000)
      dsp_ndb_ptr->d_debug_buffer_size = C_DEBUG_BUFFER_SIZE;
      dsp_ndb_ptr->d_debug_trace_type = (API)trace_info.dsptrace_handler_globals.dsp_trace_level_copy | 0x8000;
      trace_info.dsptrace_handler_globals.dsp_trace_level_copy = 0x0000;

       /*
	* FreeCalypso note: same change from 0x9000 to 0x8000 above
	* as in l1_disable_dsp_trace().
	*/

      // Allocate DEBUG message.
      msg = os_alloc_sig(sizeof(T_TRACE_INFO));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = TRACE_INFO;

      ((T_TRACE_INFO *)(msg->SigP))->debug_code = DSP_TRACE_ENABLE;

      // send message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR);
    }
    // In case higher nested levels enable, decrement nested counter
    else if (trace_info.dsptrace_handler_globals.nested_disable_count > 1)
    {
      trace_info.dsptrace_handler_globals.nested_disable_count--;
    }
  }
}


/* l1_get_dsp_trace_mask                                 */
/* Parameters :  none                                    */
/* Return     :  dsp trace mask                          */
/* Description :  Returns DSP Trace Mask copy beyond     */
/*                potential trace disabling due to       */
/*                DSP Trace Handler behavior             */


UWORD16 l1_get_dsp_trace_mask ()
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  if (trace_info.dsptrace_handler_globals.trace_flag_blocked == TRUE)
    return trace_info.dsptrace_handler_globals.dsp_trace_level_copy;
  else
    return dsp_ndb_ptr->d_debug_trace_type;
}

/* l1_set_dsp_trace_mask                                 */
/* Parameters :  new dsp trace mask                      */
/* Return     :  none                                    */
/* Description :  Sets DSP Trace Mask safely regarding   */
/*                DSP Trace Handler behavior             */


void l1_set_dsp_trace_mask (UWORD16 mask)
{
#if (CODE_VERSION != SIMULATION)
  T_NDB_MCU_DSP* dsp_ndb_ptr = (T_NDB_MCU_DSP *) NDB_ADR;
#else
  T_NDB_MCU_DSP* dsp_ndb_ptr = l1s_dsp_com.dsp_ndb_ptr;
#endif

  if (trace_info.dsptrace_handler_globals.trace_flag_blocked == TRUE)
    trace_info.dsptrace_handler_globals.dsp_trace_level_copy = mask;
  else
    dsp_ndb_ptr->d_debug_trace_type = (API)(mask | 0x8000);
}

#endif
#endif // (TRACE_TYPE==1) || (TRACE_TYPE == 4)


#if (L1_AUDIO_MCU_ONOFF == 1)

/* l1_trace_audio_onoff()                               */
/* Parameters :                                          */
/* Return     :                                          */

void l1_trace_ul_audio_onoff(UWORD8 ul_state)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_AUDIO_ONOFF)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1_AUDIO_UL_ONOFF_TRACE;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = ul_state;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}
void l1_trace_dl_audio_onoff(UWORD8 dl_state)
{
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)

     xSignalHeaderRec *msg;

     if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_AUDIO_ONOFF)
     {
       // Allocate DEBUG message.
       msg = os_alloc_sig(sizeof(T_TRACE_INFO));
       DEBUGMSG(status,NU_ALLOC_ERR)
       msg->SignalCode = TRACE_INFO;

       ((T_TRACE_INFO *)(msg->SigP))->debug_code = L1_AUDIO_DL_ONOFF_TRACE;

       ((T_TRACE_INFO *)(msg->SigP))->tab[0] = l1s.actual_time.fn_mod42432;
       ((T_TRACE_INFO *)(msg->SigP))->tab[1] = dl_state;

       // send message...
       os_send_sig(msg, L1C1_QUEUE);
       DEBUGMSG(status,NU_SEND_QUEUE_ERR)
     }
  #endif
}

#endif


#if (BURST_PARAM_LOG_ENABLE == 1)

  void l1_log_burst_param (UWORD32 angle, UWORD32 snr, UWORD32 afc, UWORD32 task,
                             UWORD32 pm, UWORD32 toa_val, UWORD32 IL_for_rxlev)
  {
    if(burst_param_log_index >= BURST_PARAM_LOG_BUFFER_LENGTH)
    {
      burst_param_log_index = 0;
    }

    burst_param_log_debug[burst_param_log_index].fn_mod42432  = (UWORD16)(l1s.actual_time.fn_mod42432);
    burst_param_log_debug[burst_param_log_index].l1_mode      = (UWORD16)(l1a_l1s_com.mode);
    burst_param_log_debug[burst_param_log_index].task         = (UWORD16)(task);
    burst_param_log_debug[burst_param_log_index].SNR_val      = (UWORD16)(snr);
    burst_param_log_debug[burst_param_log_index].TOA_val      = (UWORD16)(toa_val);
    burst_param_log_debug[burst_param_log_index].angle        = (UWORD16)(angle);
    burst_param_log_debug[burst_param_log_index].pm           = (UWORD16)(pm);
    burst_param_log_debug[burst_param_log_index].IL_for_rxlev = (UWORD16)(IL_for_rxlev);
    burst_param_log_debug[burst_param_log_index].l1s_afc      = (UWORD16)(afc);

    burst_param_log_index = burst_param_log_index + 1;

  }

#endif


#if ((OP_L1_STANDALONE == 1) && ((DSP == 38)|| (DSP == 39)) && (CODE_VERSION != SIMULATION))

#define API_DUMP_DELAY_NS  (10000000) // Delay in nanoseconds

void l1_api_dump(void)
{

  UWORD32 index;
  char    str[256];

  // DB MCU to DSP GSM log

  index = 0;

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 2) || (TRACE_TYPE == 3) || (TRACE_TYPE == 7)
  if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_API_DUMP)
  {

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  ===  API LOGS ==== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  DB MCU_to_DSP Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"  d_task_d:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_task_d))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_burst_d:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_burst_d))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_task_u:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_task_u))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_burst_u:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_burst_u))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_task_md:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_task_md))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_background:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_background))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_debug:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_debug))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_task_ra:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_task_ra))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_fn:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_fn))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_ctrl_tch:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_ctrl_tch))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->hole))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_ctrl_abb:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_ctrl_abb))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  a_a5fn:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->a_a5fn[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_power_ctl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_power_ctl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_afc:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_afc))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  d_ctrl_system:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_ctrl_system))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

//    sprintf (str,"  d_swh_ApplyWhitening_db:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_w_ptr->d_swh_ApplyWhitening_db))));
//    L1_send_low_level_trace(str)
//    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    //TODO: Need to add the new DCO variables here

    // DB  DSP to MCU GSM log

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  DB DSP_to_MCU Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    sprintf (str,"d_task_d:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_task_d))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_burst_d:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_burst_d))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_task_u:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_task_u))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_burst_u:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_burst_u))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_task_md:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_task_md))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_background:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_background))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_debug))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_task_ra:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->d_task_ra))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_serv_demod:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->a_serv_demod[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_pm:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->a_pm[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_sch:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_db_r_ptr->a_sch[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    // NDB GSM Log


    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  NDB Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"d_dsp_page:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_page))));      // 0x08D4
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"d_error_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_error_status))));  // 0x08D5
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"d_spcx_rif_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_spcx_rif_hole))));    // 0x08D6
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tch_mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tch_mode))));  // 0x08D7 TCH mode register.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_debug1))));    // 0x08D8 bit 0 at 1 enable dsp f_tx delay for Omega
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_dsp_test:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_test))));  // 0x08D9
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_version_number1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_version_number1))));  // 0x08DB
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_version_number2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_version_number2))));  // 0x08DB
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug_ptr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_debug_ptr))));        // 0x08DC
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug_bk:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_debug_bk))));         // 0x08DD
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_pll_config:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_pll_config))));       // 0x08DE
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"p_debug_buffer:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->p_debug_buffer))));       // 0x08DF
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug_buffer_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_debug_buffer_size))));  // 0x08E0
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_debug_trace_type:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_debug_trace_type))));   // 0x08E1
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #if (W_A_DSP_IDLE3 == 1)
      // DSP report its state: 0 run, 1 Idle1, 2 Idle2, 3 Idle3.
      sprintf (str,"d_dsp_state:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_state))));        // 0x08E2
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      // 5 words are reserved for any possible mapping modification
      sprintf (str,"d_hole1_ndb[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_hole1_ndb[0]))));     // 0x08E3
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      // 6 words are reserved for any possible mapping modification
      sprintf (str,"d_hole1_ndb[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_hole1_ndb[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    #if (AMR == 1)
      sprintf (str,"p_debug_amr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->p_debug_amr))));        // 0x08E5??? DSP doc says reserved
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_hole_debug_amr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_hole_debug_amr))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    #if ((CHIPSET == 15) || (CHIPSET == 12) || (CHIPSET == 4) || ((CHIPSET == 10) && (OP_WCP == 1))) // Calypso+ or Perseus2
      sprintf (str,"d_dsp_iq_scaling_factor:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_iq_scaling_factor)))); // 0x08E6
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_mcsi_select:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_mcsi_select))));  // 0x08E7
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_dsp_iq_scaling_factor:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_iq_scaling_factor))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    // New words APCDEL1 and APCDEL2 for 2TX: TX/PRACH combinations
    sprintf (str,"d_apcdel1_bis:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apcdel1_bis))));    // 0x08E8
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_apcdel2_bis:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apcdel2_bis))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    // New registers due to IOTA analog base band
    sprintf (str,"d_apcdel2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apcdel2))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vbctrl2_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl2_hole))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_bulgcal_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_bulgcal_hole))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Analog Based Band - removed in ROM 38
    #if (ANALOG == 11)
      sprintf (str,"d_afcctladd_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_afcctladd_hole))));      // 0x08ED
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_afcctladd:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_afcctladd))));      // 0x08ED
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    #if (ANALOG == 11)
      sprintf (str,"d_vbuctrl_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl_hole))));        // 0x08EE - removed in ROM38
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_vbdctrl_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl_hole))));        // 0x08EF - removed in ROM38
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_vbuctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_vbdctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif
    sprintf (str,"d_apcdel1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apcdel1))));                 // 0x08F0
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_apclev:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apclev))));                 // 0x08F1
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_apcctrl2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_apcctrl2))));                  // 0x08F2
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"d_bulqoff_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_bulqoff_hole))));                 // 0x08F3
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_dai_onoff:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dai_onoff))));               // 0x08F4
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_auxdac_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_auxdac_hole))));                  // 0x08F5
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    //#if (ANALOG == 1)
    //  sprintf (str,"d_vbctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl))));
    //  L1_send_low_level_trace(str)
    //  wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    //#elif ((ANALOG == 2) || (ANALOG == 3))
    //  sprintf (str,"d_vbctrl1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl1))));
    //  L1_send_low_level_trace(str)
    //  wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    //#endif

    #if (ANALOG == 1)
      sprintf (str,"d_vbctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #elif ((ANALOG == 2) || (ANALOG == 3))
      sprintf (str,"d_vbctrl1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl1))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #elif (ANALOG == 11)
      sprintf (str,"d_vbctrl_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbctrl_hole))));         // 0x08F6 - removed in ROM38
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif
    #if (ANALOG == 11)
      sprintf (str,"d_bbctrl_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_bbctrl_hole))));         // 0x08F7 - removed in ROM38
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_bbctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_bbctrl))));         // 0x08F7 - removed in ROM38
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    // Monitoring tasks control (MCU <- DSP)
    // FB task
    sprintf (str,"d_fb_det:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fb_det))));           // 0x08F8 FB detection result. (1 for FOUND).
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_fb_mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fb_mode))));          // Mode for FB detection algorithm.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_sync_demod[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_sync_demod[0]))));    // FB/SB demod. result, (D_TOA,D_PM,D_ANGLE,D_SNR).
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // SB Task
    sprintf (str,"a_sch26[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_sch26[0]))));         // 0x08FE Header + SB information, array of  5 words.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"d_audio_gain_ul:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_gain_ul))));    // 0x0903
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_audio_gain_dl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_gain_dl))));    // 0x0904
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Controller of the melody E2 audio compressor - removed in ROM 38
    sprintf (str,"d_audio_compressor_ctrl_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_compressor_ctrl_hole))));  // 0x0905 - removed in ROM37,38
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    // AUDIO module
    sprintf (str,"d_audio_init:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_init))));      // 0x0906
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_audio_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_status))));    //
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Audio tasks
    // TONES (MCU -> DSP)
    sprintf (str,"d_toneskb_init:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_toneskb_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_k_x1_t0:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t0))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_k_x1_t1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t1))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_k_x1_t2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t2))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_pe_rep:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_pe_rep))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_pe_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_pe_off))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_se_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_se_off))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_bu_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_bu_off))));         // 0x0910
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t0_on:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t0_on))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t0_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t0_off))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t1_on:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t1_on))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t1_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t1_off))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t2_on:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t2_on))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_t2_off:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_t2_off))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_k_x1_kt0:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt0))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_k_x1_kt1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt1))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_dur_kb:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dur_kb))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_shiftdl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_shiftdl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_shiftul:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_shiftul))));        // 0x091B
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


#if(DSP == 38) || (DSP == 39)
    sprintf (str,"d_aec_ul_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl))));
#else
	sprintf (str,"d_aec_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl))));       // 0x091C
#endif

    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"d_es_level_api:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_level_api))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_mu_api:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_mu_api))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Melody Ringer module
    sprintf (str,"d_melo_osc_used:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used))));   // 0x091F
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_melo_osc_active:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active)))); // 0x0920
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note0[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note0[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note1[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note2[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note2[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note3[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note3[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note4[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note4[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note5[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note5[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note6[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note6[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_melo_note7[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note7[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // selection of the melody format
    sprintf (str,"d_melody_selection:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_melody_selection))));  // 0x0941
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Holes due to the format melody E1
    sprintf (str,"a_melo_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melo_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Speech Recognition module - Removed in ROM38
    sprintf (str,"d_sr_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_sr_holes[0]))));  // 0x0945
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Audio buffer
    sprintf (str,"a_dd_1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dd_1[0]))));         // 0x0958 Header + DATA traffic downlink information, sub. chan. 1.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_du_1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_du_1[0]))));         // 0x096E Header + DATA traffic uplink information, sub. chan. 1.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // V42bis module
    sprintf (str,"d_v42b_nego0:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_v42b_nego0))));       // 0x0984
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_v42b_nego1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_v42b_nego1))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_v42b_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_v42b_control))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_v42b_ratio_ind:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_v42b_ratio_ind))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_mcu_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_mcu_control))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_mcu_control_sema:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_mcu_control_sema))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Background tasks
    sprintf (str,"d_background_enable:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_background_enable))));  // 0x098E
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_background_abort:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_background_abort))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_background_state:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_background_state))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_max_background:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_max_background))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_background_tasks[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_back_task_io[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_back_task_io[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // GEA module defined in l1p_deft.h (the following section is overlaid with GPRS NDB memory)
    // ??? -> is this still valid for Locosto?
    sprintf (str,"d_gea_mode_ovly_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_gea_mode_ovly_hole))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_gea_kc_ovly_hole[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_gea_kc_ovly_hole[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


#if (ANALOG == 3)
    // SYREN specific registers
    sprintf (str,"d_vbpop:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vbpop))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vau_delay_init:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vau_delay_init))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vaud_cfg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vaud_cfg))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vauo_onoff:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vauo_onoff))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vaus_vol:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vaus_vol))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_vaud_pll:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_togbr2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_togbr2))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#elif ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 11))
    sprintf (str,"d_hole3_ndb[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_hole3_ndb[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#endif

    // word used for the init of USF threshold
    sprintf (str,"d_thr_usf_detect:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_usf_detect))));  // 0x09BA
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Encryption module
    sprintf (str,"d_a5mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_a5mode))));           // Encryption Mode.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"d_sched_mode_gprs_ovly:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_sched_mode_gprs_ovly))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
/*
#if FF_L1_IT_DSP_USF  //sajal
    // 7 words are reserved for any possible mapping modification
    sprintf (str,"d_hole3_fast_ndb[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_hole3_fast_ndb[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
#endif
*/

    // Ramp definition for Omega device
    sprintf (str,"a_ramp_hole[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_ramp_hole[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // CCCH/SACCH downlink information...(!!)
    sprintf (str,"a_cd[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_cd[0]))));           // Header + CCCH/SACCH downlink information.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // FACCH downlink information........(!!)
    sprintf (str,"a_fd[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_fd[0]))));           // Header + FACCH downlink information.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Traffic downlink data frames......(!!)
    sprintf (str,"a_dd_0[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dd_0[0]))));         // Header + DATA traffic downlink information, sub. chan. 0.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // CCCH/SACCH uplink information.....(!!)
    sprintf (str,"a_cu[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_cu[0]))));           // Header + CCCH/SACCH uplink information.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // FACCH downlink information........(!!)
    sprintf (str,"a_fu[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_fu[0]))));           // Header + FACCH uplink information
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Traffic downlink data frames......(!!)
    sprintf (str,"a_du_0[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_du_0[0]))));         // Header + DATA traffic uplink information, sub. chan. 0.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Random access.....................(MCU -> DSP).
    sprintf (str,"d_rach:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_rach))));             // RACH information.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    //...................................(MCU -> DSP).
    sprintf (str,"a_kc[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_kc[0]))));            // Encryption Key Code.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Integrated Data Services module
    sprintf (str,"d_ra_conf:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ra_conf))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ra_act:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ra_act))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ra_test:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ra_test))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ra_statu:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ra_statu))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ra_statd:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ra_statd))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_fax:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fax))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_data_buf_ul[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_data_buf_ul[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_data_buf_dl[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_data_buf_dl[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


//   GTT API mapping for DSP code 34 (for test only)
#if (L1_GTT == 1)
    sprintf (str,"d_tty_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty_status))));  // 0x0A7F
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ctm_detect_shift:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ctm_detect_shift))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_baudot_mod_amplitude_scale:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_baudot_mod_amplitude_scale))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_samples_per_baudot_stop_bit:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_samples_per_baudot_stop_bit))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty_reset_buffer_ul:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty_reset_buffer_ul))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty_loop_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty_loop_ctrl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"p_tty_loop_buffer:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->p_tty_loop_buffer))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_ctm_mod_norm:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_ctm_mod_norm))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_offset_normalization:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_offset_normalization))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_threshold_startbit:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_threshold_startbit))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_threshold_diff:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_threshold_diff))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_duration_startdetect:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_duration_startdetect))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_tty2x_startbit_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_tty2x_startbit_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
#else
    sprintf (str,"a_tty_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_tty_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#endif

    sprintf (str,"a_sr_holes0[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_sr_holes0[0])))); // 0x0A87
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


#if (L1_NEW_AEC)
    // new AEC
    sprintf (str,"d_cont_filter:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cont_filter))));     // 0x0C25
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_granularity_att:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_granularity_att))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_coef_smooth:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_coef_smooth))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_es_level_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_level_max))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_fact_vad:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_vad))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thrs_abs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thrs_abs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_fact_asd_fil:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_fil))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_fact_asd_mut:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_mut))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_far_end_pow_h:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_far_end_pow_h))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_far_end_pow_l:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_far_end_pow_l))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_far_end_noise_h:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_far_end_noise_h))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_far_end_noise_l:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_far_end_noise_l))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#else

#if(DSP != 38) && (DSP != 39)
    sprintf (str,"a_new_aec_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_new_aec_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
#endif

#endif // L1_NEW_AEC

    // Speech recognition model
    sprintf (str,"a_sr_holes1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_sr_holes1[0]))));   // 0x0C31
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


//   Correction of PR G23M/L1_MCU-SPR-15494
#if ((CHIPSET == 12) || (CHIPSET == 4) || (CODE_VERSION == SIMULATION))
    sprintf (str,"d_cport_init:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_init))));      // 0x0CC2
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_cport_cfr[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_tcl_tadt:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_tcl_tadt))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_tdat:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_tdat))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_tvs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_tvs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_status))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_cport_reg_value:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cport_reg_value))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf (str,"a_cport_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_cport_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#else // CHIPSET != 12
    sprintf (str,"a_cport_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_cport_holes[0])))); // 0x0CC2
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#endif // CHIPSET == 12

    sprintf (str,"a_model_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_model_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // EOTD buffer
#if (L1_EOTD==1)
    sprintf (str,"d_eotd_first:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_eotd_first))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_eotd_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_eotd_max))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_eotd_nrj_high:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_eotd_nrj_high))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_eotd_nrj_low:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_eotd_nrj_low))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_eotd_crosscor[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_eotd_crosscor[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#else
    sprintf (str,"a_eotd_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_eotd_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#endif
    // AMR ver 1.0 buffers
    sprintf (str,"a_amr_config[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_amr_config[0]))));  // 0x14E5
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_ratscch_ul[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_ratscch_ul[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_ratscch_dl[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_ratscch_dl[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_amr_snr_est:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_amr_snr_est)))); // estimation of the SNR of the AMR speech block
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#if (L1_VOICE_MEMO_AMR)
    sprintf (str,"d_amms_ul_voc:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_amms_ul_voc))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#else
    sprintf (str,"a_voice_memo_amr_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_voice_memo_amr_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

#endif
    sprintf (str,"d_thr_onset_afs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_onset_afs))));      // thresh detection ONSET AFS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_sid_first_afs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_sid_first_afs))));  // thresh detection SID_FIRST AFS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_ratscch_afs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_ratscch_afs))));    // thresh detection RATSCCH AFS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_update_afs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_update_afs))));     // thresh detection SID_UPDATE AFS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_onset_ahs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_onset_ahs))));      // thresh detection ONSET AHS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_sid_ahs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_sid_ahs))));        // thresh detection SID frames AHS
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_ratscch_marker:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_ratscch_marker)))); // thresh detection RATSCCH MARKER
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_sp_dgr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_sp_dgr))));         // thresh detection SPEECH DEGRADED/NO_DATA
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_thr_soft_bits:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thr_soft_bits))));      // 0x14FF
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    sprintf (str,"a_amrschd_debug[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_amrschd_debug[0]))));   // 0x1500
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #if (W_A_AMR_THRESHOLDS)
      sprintf (str,"a_d_macc_thr_afs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_afs[0]))));   // 0x151E
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"a_d_macc_thr_ahs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_d_macc_thr_ahs[0]))));   // 0x1526
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"d_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_holes[0]))));           // 0x151E
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif

    // There is no melody E2 in DSP ROM38 as of now -> Only Holes
    sprintf (str,"d_melody_e2_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_holes[0]))));   // 0x152C
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    #if ((CHIPSET == 15) || (CHIPSET == 12) || (CHIPSET == 4) || ((CHIPSET == 10) && (OP_WCP == 1)) || (CODE_VERSION == SIMULATION)) // Calypso+ or Perseus2 or Samson
      sprintf (str,"d_vol_ul_level_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vol_ul_level_hole))));   // 0x153D
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_vol_dl_level_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vol_dl_level_hole))));   // 0x153E
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_vol_speed_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_vol_speed_hole))));      // 0x153F
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_sidetone_level_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_sidetone_level_hole)))); // 0x1540
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      // Audio control area
      sprintf (str,"d_es_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl))));       // 0x1541
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_anr_ul_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

	  sprintf (str,"d_aec_ul_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

	  sprintf (str,"d_agc_ul_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_aqi_ctrl_hole1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aqi_ctrl_hole1[0])))); // Reserved for future UL modules
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_iir_dl_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl))));  // 0x1549
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_lim_dl_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

     sprintf (str,"d_drc_dl_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_agc_dl_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_audio_apps_ctrl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_audio_apps_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_status))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"d_aqi_status:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aqi_status))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      #if (L1_IIR == 1)
        sprintf (str,"d_iir_input_scaling:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_input_scaling))));       // 0x1550
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_fir_scaling:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_fir_scaling))));         //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_input_gain_scaling:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_input_gain_scaling))));  //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_output_gain_scaling:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain_scaling)))); //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_output_gain:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain))));         //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_feedback:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_feedback))));            //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_nb_iir_blocks:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_iir_blocks))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir_nb_fir_coefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_fir_coefs))));        //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_iir_iir_coefs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_iir_iir_coefs[0]))));       // 0x1558
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_iir_fir_coefs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_iir_fir_coefs[0]))));       // 0x15A8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #elif (L1_IIR == 2)

        sprintf (str,"d_iir4x_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_control))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_frame_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_frame_size))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_fir_swap:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_swap))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_fir_enable:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_enable))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_fir_length:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_length))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_fir_shift:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_shift))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_sos_enable:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_enable))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_sos_number:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_number))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_iir4x_gain:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir4x_gain))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      #else
        sprintf (str,"d_iir_holes_1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_iir_holes_1[0]))));           //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #endif


        #if (L1_DRC == 1)

        sprintf (str,"d_drc_speech_mode_samp_f:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_speech_mode_samp_f))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_num_subbands:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_num_subbands))));         //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_frame_len:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_frame_len))));  //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_expansion_knee_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_expansion_knee_fb_bs)))); //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_expansion_knee_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_expansion_knee_md_hg))));         //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_expansion_ratio_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_expansion_ratio_fb_bs))));        //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_expansion_ratio_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_expansion_ratio_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_max_amplification_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_max_amplification_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_max_amplification_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_max_amplification_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

         sprintf (str,"d_drc_compression_knee_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_compression_knee_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_compression_knee_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_compression_knee_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_compression_ratio_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_compression_ratio_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_compression_ratio_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_compression_ratio_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_energy_limiting_th_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_energy_limiting_th_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_energy_limiting_th_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_energy_limiting_th_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_threshold_fb:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_threshold_fb))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_threshold_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_threshold_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_threshold_md:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_threshold_md))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_threshold_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_threshold_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

		sprintf (str,"d_drc_limiter_hangover_spect_preserve:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_hangover_spect_preserve))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_release_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_release_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_limiter_release_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_limiter_release_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_gain_track_fb_bs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_gain_track_fb_bs))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_drc_gain_track_md_hg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->d_drc_gain_track_md_hg))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_drc_low_pass_filter[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->a_drc_low_pass_filter[0]))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_drc_mid_band_filter[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(drc_ndb->a_drc_mid_band_filter[0]))));       //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #endif

      #if (L1_ANR == 1)
        sprintf (str,"d_anr_min_gain:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_min_gain))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_vad_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_vad_thr))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_gamma_slow:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_slow))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_gamma_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_gamma_gain_slow:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_gain_slow))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_gamma_gain_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_gain_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_thr2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_thr2))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_thr4:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_thr4))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_thr5:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_thr5))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_mean_ratio_thr1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr1))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_mean_ratio_thr2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr2))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_mean_ratio_thr3:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr3))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_mean_ratio_thr4:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr4))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_div_factor_shift:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_div_factor_shift))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_ns_level:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_ns_level))));           // 0x15D6
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #endif


       #if (L1_ANR == 2)
        sprintf (str,"d_anr_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_control))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_ns_level:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_ns_level))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_tone_ene_th:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_ene_th))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_anr_tone_cnt_th:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_cnt_th))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #else
        sprintf (str,"d_anr_hole_2[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_anr_hole_2[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
       #endif

       #if (L1_WCM == 1)

        sprintf (str,"d_wcm_mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_mode))));            //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_wcm_frame_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_frame_size))));//
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_wcm_frame_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_num_sub_frames))));//
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_wcm_ratio:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_ratio))));          //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_wcm_threshold:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_threshold))));  //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #else
        sprintf (str,"d_wcm_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_wcm_holes[0]))));    //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
       #endif


       #if (L1_AGC_UL == 1)
        sprintf (str,"d_agc_ul_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_control))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_frame_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_frame_size))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_targeted_level :0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_targeted_level ))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_signal_up:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_up))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_signal_down:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_down))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_max_scale:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_max_scale))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_gain_smooth_alpha:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_gain_smooth_alpha_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_gain_smooth_beta:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_gain_smooth_beta_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_ul_gain_intp_flag:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_intp_flag))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #else
        sprintf (str,"d_agc_ul_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_holes[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #endif

       #if (L1_AGC_DL == 1)
        sprintf (str,"d_agc_dl_control:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_control))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_frame_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_frame_size))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_targeted_level :0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_targeted_level ))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_signal_up:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_up))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_signal_down:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_down))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_max_scale:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_max_scale))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_gain_smooth_alpha:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_gain_smooth_alpha_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_gain_smooth_beta:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_gain_smooth_beta_fast:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta_fast))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_agc_dl_gain_intp_flag:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_intp_flag))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

       #else
        sprintf (str,"d_agc_dl_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_holes[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #endif

      #if (L1_AEC == 2)

        sprintf (str,"d_aec_mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_aec_mode))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_mu:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_mu))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_cont_filter:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_cont_filter))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_scale_input_ul:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_scale_input_ul))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_scale_input_dl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_scale_input_dl))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_div_dmax:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_div_dmax))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_div_swap_good:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_div_swap_good))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_div_swap_bad:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_div_swap_bad))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_block_init:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_block_init))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_fact_vad:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_vad))));             //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_fact_asd_fil:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_fil))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_fact_asd_mut:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_mut))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_thrs_abs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_thrs_abs))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

		sprintf (str,"d_es_level_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_level_max))));            // 0x15C8
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_granularity_att:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_granularity_att))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_coef_smooth:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_coef_smooth))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        #endif


      #if (L1_LIMITER == 1)
        sprintf (str,"a_lim_mul_low[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_low[0]))));        // 0x15D7
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_lim_mul_high[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_lim_mul_high[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_lim_gain_fall_q15:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_gain_fall_q15))));     // 0x15DB
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_lim_gain_rise_q15:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_gain_rise_q15))));     //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_lim_block_size:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_block_size))));        // 0x15DD
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_lim_nb_fir_coefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_nb_fir_coefs))));      //
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_lim_slope_update_period:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_slope_update_period))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_lim_filter_coefs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_lim_filter_coefs[0]))));  // 0x15E0
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #else
        sprintf (str,"d_lim_hole[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_lim_hole[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #endif
      #if (L1_ES == 1)
        sprintf (str,"d_es_mode:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_mode))));               // 0x15F0
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_gain_dl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_gain_dl))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_gain_ul_1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_gain_ul_1))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_gain_ul_2:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_gain_ul_2))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_tcl_fe_ls_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_fe_ls_thr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_tcl_dt_ls_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_dt_ls_thr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_tcl_fe_ns_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_fe_ns_thr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_tcl_dt_ns_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_dt_ns_thr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_tcl_ne_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_ne_thr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_ref_ls_pwr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_ref_ls_pwr))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_switching_time:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_switching_time))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_switching_time_dt:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_switching_time_dt))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"d_es_hang_time:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_hang_time))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_es_gain_lin_dl_vect[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_dl_vect[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

        sprintf (str,"a_es_gain_lin_ul_vect[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_ul_vect[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #else
        sprintf (str,"d_es_hole[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_es_hole[0]))));
        L1_send_low_level_trace(str)
        wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      #endif

    #else // CALYPSO+ or PERSEUS2
      sprintf (str,"a_calplus_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_calplus_holes[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #endif


    sprintf (str,"a_tty_fifo_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_tty_fifo_holes[0]))));  // 0x1605 -> TTY fifos are located here
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // New DRP Releated Variables Start Here
    // Should we have RF_FAM #ifdef here???
    sprintf (str,"a_drp_holes_1[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_drp_holes_1[0]))));       // 0x16C8
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_drp_apcctrl2_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_drp_apcctrl2_hole))));         // 0x16CE - APC control register 2
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"d_drp_afc_add_api:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_drp_afc_add_api))));      // 0x16CF - Address where AFC value needs to be written
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_drp_holes_2[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_drp_holes_2[0]))));      // 0x16D0
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_drp_ramp[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_drp_ramp[0]))));         // 0x16DC - Power ramp up/down in DRP registers format
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"a_drp_holes_3[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_drp_holes_3[0]))));     // 0x16F0
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    sprintf (str,"d_dsp_write_debug_pointer:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->d_dsp_write_debug_pointer)))); // 0x17FF
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    #if (MELODY_E2)
      sprintf (str,"a_dsp_trace[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dsp_trace[0])))); // 0x1800
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"a_melody_e2_instrument_wave[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_wave[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"a_dsp_after_trace_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dsp_after_trace_holes[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    #else
      sprintf (str,"a_dsp_trace[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dsp_trace[0])))); //
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str,"a_dsp_after_trace_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr->a_dsp_after_trace_holes[0])))); // 0x1800 + C_DEBUG_BUFFER_SIZE
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

                                                             // In this region MP3 variables are placed + holes
    #endif

    // SAIC related
    sprintf (str," d_swh_flag_ndb:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr-> d_swh_flag_ndb))));                 // 0x3C7A - SWH (whitening) on / off flag
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str," d_swh_Clipping_Threshold_ndb:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr-> d_swh_Clipping_Threshold_ndb))));   // 0x3C7B - Threshold to which the DSP shall clip the SNR
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // DCO related
    sprintf (str," d_dco_samples_per_symbol:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr-> d_dco_samples_per_symbol))));       // No. of samples per symbol (IQ pair)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str," d_dco_fcw:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr-> d_dco_fcw))));                      // 0x3C8B - Frequency control word
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    // A5/3 related
    sprintf (str," a_a5_kc[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_ndb_ptr-> a_a5_kc[0]))));                     // 0x3C8C
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    // **********************************************************************************
    // PARAM Logs
    // **********************************************************************************

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  PARAM Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



    sprintf(str,"  d_transfer_rate:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_transfer_rate))));  // 0x0C31
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // Common GSM/GPRS
    // These words specified the latencies to applies on some peripherics
    sprintf(str,"  d_lat_mcu_bridge:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_lat_mcu_bridge))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_lat_mcu_hom2sam:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_lat_mcu_hom2sam))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_lat_mcu_bef_fast_access:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_lat_mcu_bef_fast_access))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_lat_dsp_after_sam:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_lat_dsp_after_sam))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // DSP Start address
    sprintf(str,"  d_gprs_install_address:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_gprs_install_address))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_misc_config:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_misc_config))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_cn_sw_workaround:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_cn_sw_workaround))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_hole2_param[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_hole2_param[0])))); // 0x0C39
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      //...................................Frequency Burst.
    sprintf(str,"  d_fb_margin_beg:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_fb_margin_beg))));  // 0x0C3D
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_fb_margin_end:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_fb_margin_end))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_nsubb_idle:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_nsubb_idle))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_nsubb_dedic:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_nsubb_dedic))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_fb_thr_det_iacq:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_fb_thr_det_iacq))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_fb_thr_det_track:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_fb_thr_det_track))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      //...................................Demodulation.
    sprintf(str,"  d_dc_off_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_dc_off_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_dummy_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_dummy_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_dem_pond_gewl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_dem_pond_gewl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_dem_pond_red:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_dem_pond_red))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      //...................................TCH Full Speech.
    sprintf(str,"  d_maccthresh1:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_maccthresh1))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_mldt:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_mldt))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_maccthresh:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_maccthresh))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_gu:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_gu))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_go:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_go))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_attmax:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_attmax))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_sm:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sm))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_b:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_b))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // V42Bis module
    sprintf(str,"  d_v42b_switch_hyst:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_v42b_switch_hyst))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_v42b_switch_min:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_v42b_switch_min))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_v42b_switch_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_v42b_switch_max))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_v42b_reset_delay:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_v42b_reset_delay))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    //...................................TCH Half Speech.
    sprintf(str,"  d_ldT_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_ldT_hr))));           // 0x0C53
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_maccthresh_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_maccthresh_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_maccthresh1_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_maccthresh1_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_gu_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_gu_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_go_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_go_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_b_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_b_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_sm_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sm_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_attmax_hr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_attmax_hr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    //...................................TCH Enhanced FR Speech.
    sprintf(str,"  c_mldt_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_mldt_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_maccthresh_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_maccthresh_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_maccthresh1_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_maccthresh1_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_gu_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_gu_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_go_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_go_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_b_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_b_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_sm_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_sm_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  c_attmax_efr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->c_attmax_efr))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    //...................................CHED
    sprintf(str,"  d_sd_min_thr_tchfs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sd_min_thr_tchfs))));   // 0x0C63
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_ma_min_thr_tchfs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_ma_min_thr_tchfs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md_max_thr_tchfs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md_max_thr_tchfs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md1_max_thr_tchfs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md1_max_thr_tchfs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_sd_min_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sd_min_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_ma_min_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_ma_min_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_sd_av_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sd_av_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md_max_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md_max_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md1_max_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md1_max_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_sd_min_thr_tchefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sd_min_thr_tchefs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_ma_min_thr_tchefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_ma_min_thr_tchefs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md_max_thr_tchefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md_max_thr_tchefs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_md1_max_thr_tchefs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_md1_max_thr_tchefs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_wed_fil_ini:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_wed_fil_ini))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_wed_fil_tc:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_wed_fil_tc))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_x_min:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_x_min))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_x_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_x_max))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_slope:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_slope))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_y_min:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_y_min))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_y_max:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_y_max))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_wed_diff_threshold:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_wed_diff_threshold))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_mabfi_min_thr_tchhs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_mabfi_min_thr_tchhs))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // FACCH module
    sprintf(str,"  d_facch_thr:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_facch_thr))));            // 0x0C79
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // IDS module
    sprintf(str,"  d_max_ovsp_ul:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_max_ovsp_ul))));          //
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_sync_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_sync_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_idle_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_idle_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_m1_thres:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_m1_thres))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_max_ovsp_dl:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_max_ovsp_dl))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_gsm_bgd_mgt:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    // FIR coefficients
    sprintf(str,"  a_fir_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->a_fir_holes[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_fir31_uplink[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[0]))));            // 0x0C84
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_fir31_downlink[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1s_dsp_com.dsp_param_ptr->a_fir31_downlink[0]))));
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    // ***************************************************************
    // GPRS **********************************************************
    // ***************************************************************

#if (L1_GPRS)

    // DB MCU to DSP GPRS

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  DB MCU_to_DSP GPRS Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  d_task_d_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->d_task_d_gprs))));        // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_task_u_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->d_task_u_gprs))));        // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_task_pm_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->d_task_pm_gprs))));       // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_burst_nb_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->d_burst_nb_gprs))));      // (map?) burst identifier. (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_ctrl_abb_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->a_ctrl_abb_gprs[0]))));   // (map?) Analog baseband control, burst per burst.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_ctrl_power_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_w_ptr->a_ctrl_power_gprs[0])))); // (map?) Power control value, burst per burst.
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    // DB DSP to MCU GPRS log

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  DB DSP_to_MCU GPRS Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_task_d_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->d_task_d_gprs))));        // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_task_u_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->d_task_u_gprs))));        // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_task_pm_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->d_task_pm_gprs))));       // (map?) Task, burst per burst  (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  d_burst_nb_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->d_burst_nb_gprs))));      // (map?) burst identifier. (part of header)
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


    sprintf(str,"  a_burst_toa_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->a_burst_toa_gprs[0]))));  // (map?) Time of arrival, burst per burst
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_burst_pm_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->a_burst_pm_gprs[0]))));   // (map?) Receive Power Level, burst per burst
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_burst_angle_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->a_burst_angle_gprs[0]))));// (map?) Angle deviation, burst per burst
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf(str,"  a_burst_snr_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_db_r_ptr->a_burst_snr_gprs[0]))));  // (map?) Signal to noise ratio, burst per burst
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    // NDB GPRS Log

    sprintf (str,"  ===================================== \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

    sprintf (str,"  NDB GPRS Log \n\r");
    L1_send_low_level_trace(str)
    wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));


      sprintf(str,"  d_gea_mode_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_gea_mode_hole))));   // 0x09AE
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_gea_kc_hole[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_gea_kc_hole[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_hole1_ndb_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_hole1_ndb_gprs[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_a5mode_ovly:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_a5mode_ovly)))); // 0x09BB
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_sched_mode_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_sched_mode_gprs))));    // 0x09BC
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_hole2_ndb_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_hole2_ndb_gprs[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_usf_updated_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_usf_updated_gprs))));   // 0x09C2
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_win_start_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_win_start_gprs))));     //
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_usf_vote_enable:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_usf_vote_enable))));    //
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_bbctrl_gprs_hole:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_bbctrl_gprs_hole))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_hole3_ndb_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_hole3_ndb_gprs[0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      // (map?) BTS physical timeslot mapping.
      sprintf(str,"  a_ctrl_ched_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_ctrl_ched_gprs[0]))));   // 0x09C8 (map?) Ched configuration, burst per burst
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_ul_buffer_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_ul_buffer_gprs[0]))));   // 0x09D0 (map?) UL burst / UL buffer mapping, burst per burst. (part of header)
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_usf_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_usf_gprs[0]))));         // 0x09D8
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_interf_meas_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_interf_meas_gprs[0])))); // 0x09E0
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_ptcchu_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_ptcchu_gprs[0]))));      // 0x09E8
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_dd_md_gprs[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_dd_md_gprs[0]))));      // 0x09EC
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_du_gprs[0][0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_du_gprs[0][0]))));      //
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_pu_gprs[0][0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_pu_gprs[0][0]))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_rlcmac_rx_no_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->d_rlcmac_rx_no_gprs))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_dd_gprs[0][0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_dd_gprs[0][0]))));      //
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_drp_ramp2_gprs_holes[0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_drp_ramp2_gprs_holes[0]))));      // 0x0C25
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  a_drp_ramp2_gprs[0][0]:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_ndb_ptr->a_drp_ramp2_gprs[0][0]))));     // 0x1700 - Power Ramp up/down in DRP registers format
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));



      // PARAM Log

      sprintf (str,"  ===================================== \n\r");
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf (str," PARAM GPRS Log \n\r");
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_overlay_rlcmac_cfg_gprs:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_param_ptr->d_overlay_rlcmac_cfg_gprs))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_mac_threshold:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_param_ptr->d_mac_threshold))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_sd_threshold:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_param_ptr->d_sd_threshold))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));

      sprintf(str,"  d_nb_max_iteration:0x%4.4x \n\r",api_dump_cnvt_mcu_to_dsp(((UWORD32) &(l1ps_dsp_com.pdsp_param_ptr->d_nb_max_iteration))));
      L1_send_low_level_trace(str)
      wait_ARM_cycles(convert_nanosec_to_cycles(API_DUMP_DELAY_NS));
#endif
  }



#endif

}

#define  TEMP_CONSTANT  14

UWORD32 delay_global_variable;



#define API_DUMP_MCU_API_ADDRESS (0xFFD00000)
#define API_DUMP_DSP_ADDRESS_BASE (0x0800)

UWORD16 api_dump_cnvt_mcu_to_dsp(UWORD32 mcu_address)
{
  UWORD16 dsp_address;

  dsp_address = (((mcu_address-API_DUMP_MCU_API_ADDRESS)>>0x1)+(API_DUMP_DSP_ADDRESS_BASE));
  return (dsp_address);

}


#endif // ((OP_L1_STANDALONE == 1) && (DSP == 38) && (CODE_VERSION != SIMULATION))
#if (AUDIO_DEBUG == 1)

/* Trace_l1_audio_regs                                   */
/* Parameters :                                          */
/* Return     :                                          */

extern UWORD8 audio_reg_read_status;
extern UWORD8 audio_regs_cpy[10];
void Trace_l1_audio_regs()
{
  char str2[100];
  l1_audio_regs_debug_read();
  if(audio_reg_read_status==1)
  {
#if ( (OP_L1_STANDALONE == 0) && (L1_COMPRESSED_TRACING == 1) )
   vsi_o_event_ttrace("AU_REG  %x %x %x %x %x %x %x %x %x %x %x",
            audio_regs_cpy[0], audio_regs_cpy[1],
            audio_regs_cpy[2], audio_regs_cpy[3],
            audio_regs_cpy[4], audio_regs_cpy[5],
            audio_regs_cpy[6], audio_regs_cpy[7],
            audio_regs_cpy[8], audio_regs_cpy[9],
            audio_regs_cpy[10]);
#else
    sprintf(str2,"AU_REG  %x %x %x %x %x %x %x %x %x %x %x\n\r",
            audio_regs_cpy[0], audio_regs_cpy[1],
            audio_regs_cpy[2], audio_regs_cpy[3],
            audio_regs_cpy[4], audio_regs_cpy[5],
            audio_regs_cpy[6], audio_regs_cpy[7],
            audio_regs_cpy[8], audio_regs_cpy[9],
            audio_regs_cpy[10]);
    L1_send_trace_cpy(str2);
#endif
    audio_reg_read_status=0;
  }
}
#endif

#if (L1_FF_MULTIBAND == 1)
static CHAR *p_trace_multiband_physical_band_id_table[] = {"PGSM900",
                                                           "GSM850",
                                                           "PCS1900",
                                                           "DCS1800",
                                                           "GSM750",
                                                           "GSM480",
                                                           "GSM450",
                                                           "T_GSM380",
                                                           "T_GSM410",
                                                           "T_GSM900",
                                                           "EGSM900",
                                                           "RGSM900",
                                                           "GSM900"};
/*-------------------------------------------------------*/
/* l1_trace_MULTIBAND_params()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

void l1_multiband_trace_params(UWORD8 multiband_table_id, UWORD8 multiband_trace_id) 
{
#if (TRACE_TYPE==1) || (TRACE_TYPE==4)
  if ((trace_info.current_config->l1_dyn_trace & (1 << L1_DYN_TRACE_MULTIBAND)))
  {
    char	str[150];
    if (multiband_table_id == MULTIBAND_ERROR_TRACE_ID)
    {
      sprintf(str,"\n MULTIBAND> fn=%d ERROR= radio_freq out of range",l1s.actual_time.fn);
       L1_send_trace_cpy(str);
    }

    else
    {
      if (multiband_table_id == MULTIBAND_PHYSICAL_BAND_TRACE_ID)
      {
        sprintf(str,"\n MULTIBAND> fn=%d Current Physical band is = %s",l1s.actual_time.fn,p_trace_multiband_physical_band_id_table[multiband_trace_id]);
         L1_send_trace_cpy(str); 
      }
    }
  }
#endif  

}
#endif /*if (L1_FF_MULTIBAND == 1)*/


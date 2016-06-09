/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_SYNC.C
 *
 *        Filename l1_sync.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

//#pragma DUPLICATE_FOR_INTERNAL_RAM_START
  #include "config.h"
  #include "l1_confg.h"
  #include "l1_macro.h"
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise
  #define  L1_SYNC_C
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

#if (CODE_VERSION == SIMULATION)
  #include "nucleus.h"
  #include <string.h>
  #if (AUDIO_TASK == 1)
      #include "l1audio_const.h"
      #include "l1audio_cust.h"
      #include "l1audio_defty.h"
    #if (L1_VOCODER_IF_CHANGE == 1)
      #include "l1audio_signa.h"
    #endif // L1_VOCODER_IF_CHANGE == 1
  #endif // AUDIO_TASK
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"
  #include <l1_trace.h>

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif // TESTMODE



  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif

  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif

  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif

  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "ulpd.h"

  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif // L2_L3 SIMUL

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_mfta.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
    #include "l1p_sign.h"
  #endif // L1_GPRS

  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"
  extern NU_TASK  L1S_task;
  #if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1)
    extern NU_TASK  API_MODEM_task;
  #endif

#else // NO SIMULATION

  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif // TESTMODE

  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"

  #if (L1_VOCODER_IF_CHANGE == 1)
    #include "l1audio_signa.h"
  #endif // L1_VOCODER_IF_CHANGE == 1

  #endif // AUDIO_TASK

  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif

  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif

  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif

  #include "l1_defty.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "tpudrv.h"
  #include "l1_trace.h"

  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif // L2_L3 SIMUL

  #include "ulpd.h"
  #include "mem.h"
  #include "inth.h"
  #include "iq.h"

  #if L1_GPRS
    #include "l1p_cons.h"
    #include "l1p_msgt.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_mfta.h"
    #include "l1p_tabs.h"
    #include "l1p_macr.h"
    #include "l1p_sign.h"
  #endif // L1_GPRS
#endif // NO SIMULATION

#if(RF_FAM == 61)
	#include "l1_rf61.h"
#endif

#define	W_A_DSP_PR20037	1	/* FreeCalypso */

#if (GSM_IDLE_RAM != 0)
#if (OP_L1_STANDALONE == 1)
#include "csmi_simul.h"
#else
#include "csmi/sleep.h"
#endif
#endif

#if (CHIPSET >= 12)
  #include "sys_conf.h"
#endif

#if (OP_L1_STANDALONE != 1) && (WCP_PROF == 1)
  #include "prf/prf_api.h"
#endif

#if 0	/* FreeCalypso TCS211 reconstruction */
//Enhanced RSSI    -OMAPS00075410
#define TOTAL_NO_OF_BITS_IDLE_MEAS    625
extern UWORD32 qual_acc_idle1[2];
#endif

#if (RF_FAM == 61)
  #include "tpudrv61.h"
#endif

#if W_A_DSP1
  UWORD8 old_sacch_DSP_bug = FALSE;
#endif

#if (TRACE_TYPE == 6)
  #define TIMER_RESET_VALUE (0xFFFF)
  #define TICKS_PER_TDMA    (2144)
#endif

#if (TRACE_TYPE == 2) || (TRACE_TYPE == 3)
  extern void L1_trace_string(char *s);
  extern void L1_trace_char  (char s);
#endif

#if (TRACE_TYPE == 4)
  #define TIMER_RESET_VALUE (0xFFFF)
#endif

#if (L1_GTT == 1)
  /**************************************/
  /* External GTT prototypes            */
  /**************************************/
  extern void l1s_gtt_manager   (void);
#endif

#if(L1_DYN_DSP_DWNLD == 1)
  extern void l1s_dyn_dwnld_manager(void);
#endif

#if (AUDIO_TASK == 1)
  /**************************************/
  /* External audio prototypes          */
  /**************************************/
  extern void l1s_audio_manager   (void);
#endif
/*-------------------------------------------------------*/
/* Prototypes of external functions used in this file.   */
/*-------------------------------------------------------*/
void l1ddsp_meas_read      (UWORD8 nbmeas, UWORD8 *pm);

#if L1_GPRS
  void l1ps_transfer_mode_manager  (void);
  void l1ps_reset_db_mcu_to_dsp    (T_DB_MCU_TO_DSP_GPRS *page_ptr);
  void l1pddsp_meas_ctrl           (UWORD8 nbmeas, UWORD8 pm_pos);
  void l1pddsp_meas_read           (UWORD8 nbmeas, UWORD8 *pm_read);
  void l1ps_meas_manager           (void);
  void l1ps_transfer_meas_manager  (void);
  void l1ps_macs_rlc_downlink_call (void);
#endif

#if (TRACE_TYPE==7) // CPU_LOAD
  extern void l1_cpu_load_start(void);
  extern void l1_cpu_load_stop(void);
  extern void l1_cpu_load_interm(void);
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

#if (CODE_VERSION==SIMULATION)
  // for verification of the suspend procedure
  STATUS status;

  /*-------------------------------------------------------*/
  /* frit_task()                                           */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function simulates the TPU scheduling task, the  */
  /* BTS behavior and shedules the l1 IT (for the L1S).This*/
  /* task as the same priority (10) as the L2, L3, L1a     */
  /* tasks. This function calls the main function of the   */
  /* simulator "sim_main()"                                */
  /*-------------------------------------------------------*/
  void frit_task(UWORD32 argc, void *argv)
  {
    while(1)
    {
      sim_main();
      os_NU_Relinquish(); // give back hand to OS...
    }
  }

  /*-------------------------------------------------------*/
  /* l1s_task()                                           */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function simulates the L1S task. This task is    */
  /* called by the main task (frit task). The L1S task has */
  /* the highest priority (5) under nucleus and calls the  */
  /* "sim_l1_int()" function. This task can suspend itsef  */
  /* in case of deep/big sleep simulation                  */
  /*-------------------------------------------------------*/

  void l1s_task(UWORD32 argc, void *argv)
  {
     while(1)
     {
       sim_l1_int();
       status = NU_Suspend_Task(&L1S_task);
       // check status value...
       if (status)
       {
       #if (TRACE_TYPE==5)
         printf("Error somewhere in the L1S suspend task \n");
       #endif
        EXIT;
        }
     }
  }

#if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1)
  /*-------------------------------------------------------*/
  /* api_modem_task()                                      */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function simulates the USF/DTX IT. This task is  */
  /* called by the main task (frit task). It task has      */
  /* the same priority as L1S under nucleus and calls the  */
  /* "sim_api_modem_int()" function.                       */
  /*-------------------------------------------------------*/

  void api_modem_task(UWORD32 argc, void *argv)
  {
     while(1)
     {
       extern void sim_api_modem_int(void);
       sim_api_modem_int();
       status = NU_Suspend_Task(&API_MODEM_task);
       // check status value...
       if (status)
       {
       #if (TRACE_TYPE==5)
         printf("Error somewhere in the API MODEM suspend task \n");
       #endif
        EXIT;
        }
     }
  }
#endif

#else // SIMULATION


  /*-------------------------------------------------------*/
  /* hisr()          High Interrupt service routine        */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function is the ISR corresponding to the frame   */
  /* interrupt coming from the TPU every TDMA frame. It    */
  /* activates the Layer 1 synchronous part "l1s_synch()". */
  /*                                                       */
  /*-------------------------------------------------------*/

  extern unsigned short       layer_1_sync_end_time;
  void hisr(void)
  {
    /*
     * FreeCalypso TCS211 reconstruction: the LoCosto version
     * of this function had a whole bunch of junk here
     * which we have removed in order to match the TCS211
     * binary object.
     */

    // stop the gauging.This function must be called at the
    // begining of the HISR in order to have the IT_GAUGING
    // executed before the Deep sleep decision.
    // GOAL: reduce the wake up time by 1 frame
    l1s_gauging_task_end();

    // check if an IT DSP stills pending => it means a CPU load error in the MCU
    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      if (TPU_check_IT_DSP()==TRUE)
      {
        #if (GSM_IDLE_RAM == 0)
         l1_trace_IT_DSP_error();
        #else
         l1_trace_IT_DSP_error_intram();
        #endif
      }
    #endif


    /******************************************************/
    // Synchronous Cpu load measurement                   */
    //    Log LISR -> hisr() entry cpu time               */
    //    Start HW timer for hisr() measurement           */
    /******************************************************/
    #if (TRACE_TYPE==7) // CPU_LOAD
       l1_cpu_load_interm();
       l1_cpu_load_start();
    #endif

     if ((l1_config.pwr_mngt == PWR_MNGT) && (l1s.pw_mgr.frame_adjust))
     {
        /******************************************************/
        // 1 Frame Adjust. after unscheduled wake-up          */
        /******************************************************/
        l1s_wakeup_adjust();
     }
     else
     {
        // increment time counter used for debug and by L3 scenario...
        l1s.debug_time ++;

        /***************************************************/
        /* Frame counters management.                      */
        /***************************************************/
        // Time...
        // "Actual time" loaded with previous "next time".
        #if L1_GPRS
          l1s.actual_time    = l1s.next_time;
          l1s.next_time      = l1s.next_plus_time;
          l1s_increment_time(&(l1s.next_plus_time), 1);  // Increment "next_plus time".
        #else
          l1s.actual_time = l1s.next_time;
          l1s_increment_time(&(l1s.next_time), 1);  // Increment "next time".
        #endif

        #if (GSM_IDLE_RAM != 0)
            // Decrement counters
            l1s.gsm_idle_ram_ctl.os_load--;
            l1s.gsm_idle_ram_ctl.hw_timer--;
        #endif

        // Multiframe table...
        // Increment active frame % mftab size.
        IncMod(l1s.afrm, 1, MFTAB_SIZE);

        // Control function counters...
        // Increment frame count from last AFC update.
        l1s.afc_frame_count++;

        // Decrement time to next L1S task.
        if(l1a_l1s_com.time_to_next_l1s_task > 0 &&
           l1a_l1s_com.time_to_next_l1s_task < MAX_FN)

           l1a_l1s_com.time_to_next_l1s_task--;
    }

    /******************************************************/
    /* Call layer 1 synchronous part.                     */
    /******************************************************/

    l1s_synch();

    /*
     * The following double invokation of l1s_synch()
     * is NOT present in the TCS211 version.
     */
    #if 0
    if(l1s.pw_mgr.sleep_performed == CLOCK_STOP &&
	(l1s.pw_mgr.wakeup_type == WAKEUP_FOR_L1_TASK ||
	 l1s.pw_mgr.wakeup_type == WAKEUP_ASYNCHRONOUS_ULPD_0 ||
	 l1s.pw_mgr.wakeup_type == WAKEUP_FOR_OS_TASK ||
	 l1s.pw_mgr.wakeup_type == WAKEUP_FOR_HW_TIMER_TASK ||
	 l1s.pw_mgr.wakeup_type == WAKEUP_FOR_GAUGING_TASK))
    {
	l1s_synch();
    }
    #endif

    // Be careful:in case of asynchronous wake-up after sleep
    // an IT_TDMA may be unmasked and executed just after l1s_sleep_manager();
    // In order to avoid issues with the execution of hisr() inside hisr()
    // do not add code here after !!!

    #if (TRACE_TYPE == 6)
    {
      UWORD16 layer_1_sync_end_time;
      UWORD8  cpu_load;

      layer_1_sync_end_time = TIMER_RESET_VALUE - TM_ReadTimer(2);
      cpu_load = (100 * layer_1_sync_end_time) / TICKS_PER_TDMA;

      l1_trace_cpu_load(cpu_load);
    }
    #endif


    /* used to be #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4) in LoCosto */
    #if (TRACE_TYPE == 1)	/* TSM30 code has it this way */
      // CPU load for TRACE_TYPE == 1 and 4 only
      if((trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_CPU_LOAD) &&
          (trace_info.sleep_performed == FALSE))
      {
        Trace_L1S_CPU_load();

    #if (WCP_PROF == 1)
#define TICKS_PER_TDMA (1875)
      prf_LogCPULoadL1S((unsigned char)((100 * layer_1_sync_end_time) / TICKS_PER_TDMA));
    #endif
      }
      trace_info.sleep_performed = FALSE;
    #endif

    /******************************************************/
    // Synchronous Cpu load measurement                   */
    //    Stop HW timer; compute results; output on uart  */
    //      if FN%13 = 11                                 */
    /******************************************************/

    #if (TRACE_TYPE==7) // CPU_LOAD
      l1_cpu_load_stop();
    #endif

  }

#endif // NO SIMULATION
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

// l1s_synch()
// Description:
// This function is the core of L1S. Here is a summary
// of the execution:
//  - Frame counters management.
//  - Get current communication page pointers.
//  - RESET internal variables.
//  - RESET MCU->DSP DB communication page.
//  - TOA update management.
//  - L1 task manager,
//      - Dedicated_mode_manager.
//      - Task_scheduler.
//      - Execute_frame.
//      - Neighbor cells measurement manager.
//      - End manager.

void l1s_synch()
{
   #if (CODE_VERSION==SIMULATION)
      // increment time counter used for debug and by L3 scenario...
      l1s.debug_time ++;

      /***************************************************/
      /* Frame counters management.                      */
      /***************************************************/
      // Time...
      #if L1_GPRS
        l1s.actual_time    = l1s.next_time;
        l1s.next_time      = l1s.next_plus_time;
        l1s_increment_time(&(l1s.next_plus_time), 1);  // Increment "next_plus time".
      #else
        l1s.actual_time = l1s.next_time;
        l1s_increment_time(&(l1s.next_time), 1);  // Increment "next time".
      #endif

      // Multiframe table...
      // Increment active frame % mftab size.
      IncMod(l1s.afrm, 1, MFTAB_SIZE);          // Increment active frame % mftab size.

      // Control function counters...
      // Increment frame count from last AFC update.
      l1s.afc_frame_count++;

      // this function is called in the HISR and must be call here in Simulation
      l1s_gauging_task_end();

      // Decrement time to next L1S task.
      if(l1a_l1s_com.time_to_next_l1s_task > 0 &&
         l1a_l1s_com.time_to_next_l1s_task < MAX_FN)
        l1a_l1s_com.time_to_next_l1s_task--;
   #endif
        /* l1s.tcr_prog_done=0; */
#if (FF_L1_FAST_DECODING == 1)
      /* If a fast decoding IT is expected AND a deferred control is scheduled */
      /* then it means that a fast decoding IT is still awaited from previous  */
      /* TDMA.                                                                 */
      if (
              (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_AWAITED)
           && (l1a_apihisr_com.fast_decoding.deferred_control_req == TRUE)
         )
      {
        l1_trace_IT_DSP_error(IT_DSP_ERROR_FAST_DECODING);
      }
#endif /* #if (FF_L1_FAST_DECODING == 1) */

  #if L1_GPRS
      /* l1s.tcr_prog_done=0; */
    // Increment TOA period counter used in packet tranfer mode
    if (l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
    {
      // TOA update period in packet transfer mode = 4*78 frames
      // At least one block needs to be transmitted by the BTS every 78 frames
      // Taking into account fading probability at least one good block (4 TOA values)
      // is input to the TOA algorithm within the TOA update period

      #if (TOA_ALGO == 2)
      #else
        l1s.toa_period_count++;
      if (l1s.toa_period_count >= 4*78)
      {
        l1s.toa_update = TRUE;  // set TOA update flag => TOA shift will be updated upon next call to l1ctl_toa
      }
      #endif

    }
  #endif


  #if (TOA_ALGO == 2)
    {
      #if L1_GPRS
        if((l1a_l1s_com.mode == I_MODE) || (l1a_l1s_com.mode == CON_EST_MODE1) || (l1a_l1s_com.mode == CON_EST_MODE2) ||
           (l1a_l1s_com.mode == DEDIC_MODE) || (l1a_l1s_com.mode == PACKET_TRANSFER_MODE))
      #else
        if((l1a_l1s_com.mode == I_MODE) || (l1a_l1s_com.mode == CON_EST_MODE1) || (l1a_l1s_com.mode == CON_EST_MODE2) ||
           (l1a_l1s_com.mode == DEDIC_MODE))
      #endif
      {
        if( (l1s.actual_time.fn >= l1s.toa_var.toa_update_fn) &&
            ((l1s.actual_time.fn - l1s.toa_var.toa_update_fn) < L1_TOA_UPDATE_TIME)  )
        {
          // TOA needs to be updated every 'L1_TOA_UPDATE_TIME' frames
          l1s.toa_var.toa_update_fn = l1s.actual_time.fn + L1_TOA_UPDATE_TIME;
          if(l1s.toa_var.toa_update_fn >= MAX_FN)
          {
            l1s.toa_var.toa_update_fn-= MAX_FN;
          }

          // Set TOA idle update = TRUE;
          l1s.toa_var.toa_update_flag = TRUE;
        }
      }
      else
      {
        // TOA needs to be updated every 'L1_TOA_UPDATE_TIME' frames
        l1s.toa_var.toa_update_fn = l1s.actual_time.fn + L1_TOA_UPDATE_TIME;
        if(l1s.toa_var.toa_update_fn >= MAX_FN)
        {
          l1s.toa_var.toa_update_fn-=MAX_FN;
        }
      }
    }
  #endif

  #if (L1_DYN_DSP_DWNLD ==1)
      #if L1_GPRS
      if((l1a_l1s_com.l1a_activity_flag == TRUE)  ||
         (l1a_l1s_com.time_to_next_l1s_task == 0) ||
         (l1s.frame_count != 0) ||
         (l1s.pw_mgr.gauging_task == ACTIVE) ||
         (l1s_get_next_gauging_in_Packet_Idle()==0) ||
         (l1s.dyn_dwnld_state != 0))
    #else
        if((l1a_l1s_com.l1a_activity_flag == TRUE)  ||
         (l1a_l1s_com.time_to_next_l1s_task == 0) ||
         (l1s.frame_count != 0) ||
         (l1s.pw_mgr.gauging_task == ACTIVE) ||
         (l1s.dyn_dwnld_state != 0))
  #endif // L1_GPRS
  #else
  #if L1_GPRS
    if((l1a_l1s_com.l1a_activity_flag == TRUE)  ||
       (l1a_l1s_com.time_to_next_l1s_task == 0) ||
       (l1s.frame_count != 0) ||
       (l1s.pw_mgr.gauging_task == ACTIVE) ||
       (l1s_get_next_gauging_in_Packet_Idle()==0) )
  #else
      if((l1a_l1s_com.l1a_activity_flag == TRUE)  ||
       (l1a_l1s_com.time_to_next_l1s_task == 0) ||
       (l1s.frame_count != 0) ||
       (l1s.pw_mgr.gauging_task == ACTIVE))
  #endif // L1_GPRS
  #endif // L1_DYN_DSP_DWNLD
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // L1A has been executed, or
  // It's time to execute next task, or
  // A task is still in the MFTAB, or
  // a gauging will be performed in Packet Idle mode
  // ==> execute L1 core.
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  {
    BOOL l1s_task_allowed = TRUE;

    /*
     * FreeCalypso TCS211 reconstruction: the following code
     * fails to compile because the wakeup_time structure member
     * is not present in TCS211 headers.  Let's try omitting it
     * so we can get a successful compile and start diffing the
     * compilation results.
     */
#if 0

    /* This is not required in Locosto after merge of deep-sleep
     * initialization and control frame */
 #if (CHIPSET != 15)
    // SETUP_AFC_AND_RF+1 frames shall pass since last wakeup
    // from deep sleep before scheduling any tasks due to RF wakeup.
    if ((l1_config.pwr_mngt == PWR_MNGT)  // PWR management enabled
      && ((l1s.pw_mgr.mode_authorized == DEEP_SLEEP) || (l1s.pw_mgr.mode_authorized == ALL_SLEEP)) // deep sleep is still authorized
      && (l1s.pw_mgr.sleep_performed == CLOCK_STOP) // previous sleep was deep sleep
      && (((l1s.actual_time.fn_mod42432 - l1s.pw_mgr.wakeup_time + 42432) % 42432) <= l1_config.params.setup_afc_and_rf)

    #if L1_GPRS
      && (l1a_l1s_com.mode != DEDIC_MODE) && (l1a_l1s_com.mode != PACKET_TRANSFER_MODE)) //check that board is not in dedicated or transfer
    #else
      && (l1a_l1s_com.mode != DEDIC_MODE) ) //check that board is not in dedicated
    #endif
    {
      l1s_task_allowed = FALSE;
    }
    else
    {
       l1s.pw_mgr.sleep_performed = DO_NOT_SLEEP; // In case l1s is executed, initialize sleep performed in order to avoid reentry in part above 42432 frames later
       l1s_task_allowed = TRUE;
    }
#else
    l1s.pw_mgr.sleep_performed = DO_NOT_SLEEP; // In case l1s is executed, initialize sleep performed in order to avoid reentry in part above 42432 frames later
    l1s_task_allowed = TRUE;
#endif
#endif

    if (l1s_task_allowed == TRUE)
    {
    // Reset L1A activity flag.
    l1a_l1s_com.l1a_activity_flag = FALSE;

    // Set default value in frame count to next task.
    l1a_l1s_com.time_to_next_l1s_task = MAX_FN;

    /*************************************************************/
    /* Get current communication page pointers.                  */
    /*************************************************************/
    // init pointer in DB according to "dsp read page" number

    #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
      if (l1s_dsp_com.dsp_r_page == 0)
      {
        if (l1s_dsp_com.dsp_w_page == 0) trace_fct(CST_NEW_FRAME_PAGE_R0_W0, (UWORD32)(-1));
        else                             trace_fct(CST_NEW_FRAME_PAGE_R0_W1, (UWORD32)(-1));
      }
      else
      {
        if (l1s_dsp_com.dsp_w_page == 0) trace_fct(CST_NEW_FRAME_PAGE_R1_W0, (UWORD32)(-1));
        else                             trace_fct(CST_NEW_FRAME_PAGE_R1_W1, (UWORD32)(-1));
      }
    #endif

    #if (CODE_VERSION == SIMULATION)
      l1s_dsp_com.dsp_db_r_ptr = (T_DB_DSP_TO_MCU *) &(buf.mcu_rd[l1s_dsp_com.dsp_r_page]);
      l1s_dsp_com.dsp_db_w_ptr = (T_DB_MCU_TO_DSP *) &(buf.mcu_wr[l1s_dsp_com.dsp_w_page]);
      #if (DSP == 38) || (DSP == 39)
      l1s_dsp_com.dsp_db_common_w_ptr = (T_DB_COMMON_MCU_TO_DSP *) &(buf.mcu_wr_common[l1s_dsp_com.dsp_w_page]);
      #endif

    #else
      if (l1s_dsp_com.dsp_r_page == 0)  l1s_dsp_com.dsp_db_r_ptr = (T_DB_DSP_TO_MCU *) DB_R_PAGE_0;
      else                              l1s_dsp_com.dsp_db_r_ptr = (T_DB_DSP_TO_MCU *) DB_R_PAGE_1;
      if (l1s_dsp_com.dsp_w_page == 0)  l1s_dsp_com.dsp_db_w_ptr = (T_DB_MCU_TO_DSP *) DB_W_PAGE_0;
      else                              l1s_dsp_com.dsp_db_w_ptr = (T_DB_MCU_TO_DSP *) DB_W_PAGE_1;
      #if (DSP == 38) || (DSP == 39)
        if (l1s_dsp_com.dsp_w_page == 0)  l1s_dsp_com.dsp_db_common_w_ptr = (T_DB_COMMON_MCU_TO_DSP*) DB_COMMON_W_PAGE_0;
        else  l1s_dsp_com.dsp_db_common_w_ptr = (T_DB_COMMON_MCU_TO_DSP *) DB_COMMON_W_PAGE_1;

      #endif
    #endif

    #if (L1_GPRS)
      #if (CODE_VERSION == SIMULATION)
        l1ps_dsp_com.pdsp_db_r_ptr = &(buf.mcu_rd_gprs[l1s_dsp_com.dsp_r_page]);
        l1ps_dsp_com.pdsp_db_w_ptr = &(buf.mcu_wr_gprs[l1s_dsp_com.dsp_w_page]);
      #else
        if (l1s_dsp_com.dsp_r_page == 0)  l1ps_dsp_com.pdsp_db_r_ptr = (T_DB_DSP_TO_MCU_GPRS *) DB_R_PAGE_0_GPRS;
        else                              l1ps_dsp_com.pdsp_db_r_ptr = (T_DB_DSP_TO_MCU_GPRS *) DB_R_PAGE_1_GPRS;
        if (l1s_dsp_com.dsp_w_page == 0)  l1ps_dsp_com.pdsp_db_w_ptr = (T_DB_MCU_TO_DSP_GPRS *) DB_W_PAGE_0_GPRS;
        else                              l1ps_dsp_com.pdsp_db_w_ptr = (T_DB_MCU_TO_DSP_GPRS *) DB_W_PAGE_1_GPRS;
      #endif
    #endif

    #if (DSP_DEBUG_TRACE_ENABLE == 1)
      if (l1s_dsp_com.dsp_r_page == 0)
      {
        l1s_dsp_com.dsp_db2_current_r_ptr = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_0;
        l1s_dsp_com.dsp_db2_other_r_ptr   = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_1;
      }
      else
      {
        l1s_dsp_com.dsp_db2_current_r_ptr = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_1;
        l1s_dsp_com.dsp_db2_other_r_ptr   = (T_DB2_DSP_TO_MCU *) DB2_R_PAGE_0;
      }
    #endif

    #if (D_ERROR_STATUS_TRACE_ENABLE == 1)
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_L1S_DEBUG)
      // check d_error_status variable
      #if (GSM_IDLE_RAM == 0)
         Trace_d_error_status();
      #else
         Trace_d_error_status_intram();
      #endif
    #endif

    /*************************************************************/
    /* RESET internal variables...                               */
    /* Must be performed after having set the current com. pages */
    /*************************************************************/
    l1s.tpu_win                 = 0;         // Reset resources for driver and sub tasks...
    l1s_dsp_com.dsp_r_page_used = FALSE;     // Init. flag for MCU<-DSP comm.
    l1s.tpu_ctrl_reg            = NO_CTRL;   // Reset MCU->TPU comm. task register (tx, rx, pw tasks).
    l1s.dsp_ctrl_reg            = NO_CTRL;   // Reset MCU->DSP comm. task register (tx, rx, pw tasks).

    /*************************************************************/
    /* RESET MCU->DSP DB communication page.                     */
    /*************************************************************/
    l1s_reset_db_mcu_to_dsp(l1s_dsp_com.dsp_db_w_ptr);
    #if (DSP == 38) || (DSP == 39)
      l1s_reset_db_common_mcu_to_dsp(l1s_dsp_com.dsp_db_common_w_ptr);
    #endif
    #if (L1_GPRS)
      l1ps_reset_db_mcu_to_dsp(l1ps_dsp_com.pdsp_db_w_ptr);
    #endif

   /********************************************************************/
    /* Reset DSP IT ENABLE bit Satu/Hyp/Dione TO BE REMOVED in HERCULES */
    /********************************************************************/
    #if (W_A_ITFORCE)
       (*(volatile UWORD16 *) TPU_INT_CTRL) &= ~TPU_INT_ITD_F;
    #endif

    /*************************************************************/
    /* TOA UPDATE MANAGEMENT.                                    */
    /*************************************************************/
    #if (TOA_ALGO != 0)
      if(l1a_l1s_com.toa_reset == TRUE)
      // TOA algo must be initialized
      {
        #if (TOA_ALGO == 2)
          l1s.toa_var.toa_shift  = l1ctl_toa(TOA_INIT, 0, 0, 0);
        #else
        l1s.toa_shift         = l1ctl_toa(TOA_INIT, 0, 0, 0, &l1s.toa_update, &l1s.toa_period_count
        #if (FF_L1_FAST_DECODING == 1)
            ,0
        #endif /* FF_L1_FAST_DECODING */
            );
        #endif
        l1a_l1s_com.toa_reset = FALSE;
      }

      // Decrement mask counter for TOA.
      // Rem: this counter is used to mask the SNR/TOA results for 2
      //      frames immediatly following an update of TOA.
      #if (TOA_ALGO == 2)
       if(l1s.toa_var.toa_snr_mask > 0) l1s.toa_var.toa_snr_mask--;
      #else
      if(l1s.toa_snr_mask > 0) l1s.toa_snr_mask--;
      #endif


    #endif

    /*************************************************************/
    /* L1 TASK MANAGER.                                          */
    /*************************************************************/

    #if (TRACE_TYPE == 1) || (TRACE_TYPE==4)
      #if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))
      trace_info.l1s_rtt_func.rtt_refresh_status(trace_info.l1s_trace_user_id);
      #endif

      RTTL1_FILL_FN(l1s.actual_time.fn)
    #endif

    #if (GSM_IDLE_RAM != 0)
      if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1) || (l1a_l1s_com.dedic_set.SignalCode != NULL))
        {
          if (!READ_TRAFFIC_CONT_STATE)
          {
            CSMI_TrafficControllerOn();
            #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
              l1s_trace_mftab();
            #endif
          }
          // Call routine: DEDICATED_MODE_MANAGER.
          l1s_dedicated_mode_manager();
        }
    #else // GSM_IDLE_RAM
      l1s_dedicated_mode_manager();
    #endif

    #if L1_GPRS
    #if (GSM_IDLE_RAM != 0)
          if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1))
            {
              l1ps_transfer_mode_manager();
            }
          else
            {
              if(!l1pa_l1ps_com.transfer.semaphore)
              {
                if ((l1pa_l1ps_com.transfer.fset[0]->SignalCode != NULL) || (l1pa_l1ps_com.transfer.fset[1]->SignalCode != NULL))
                {
                   if (!READ_TRAFFIC_CONT_STATE)
                     {
                       CSMI_TrafficControllerOn();
                     #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                       l1s_trace_mftab();
                     #endif
                     }
                   // Call routine: TRANSFER_MODE_MANAGER.
                   l1ps_transfer_mode_manager();
                }
              }
            }
    #else // GSM_IDLE_RAM
      // Call routine: TRANSFER_MODE_MANAGER.
      l1ps_transfer_mode_manager();
    #endif // GSM_IDLE_RAM
    #endif // L1_GPRS

    l1s_task_scheduler_process();

    // Call routine: EXECUTE_FRAME.
    l1s_execute_frame();
    #if L1_GPRS
      // Call routine: PACKET_MEAS_MANAGER.
      l1ps_meas_manager();

      #if (GSM_IDLE_RAM != 0)
        if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1))
          {
            if (l1a_l1s_com.l1s_en_task[PDTCH] != TASK_DISABLED)            // <- Added in line with comment on l1s_meas_manager() :
            // Call routine: PACKET_TRANSFER_MODE_MANAGER                   //    "Measurement manager not usefull in packet transfer mode
            l1ps_transfer_meas_manager();                                   //    This permit to save CPU in packet transfer mode    // Call routine: TASK_SCHEDULER."
          }
      #else
        l1ps_transfer_meas_manager();                                   //    This permit to save CPU in packet transfer mode    // Call routine: TASK_SCHEDULER."
      #endif //GSM_IDLE_RAM
    #endif  //L1_GPRS

    #if L1_GPRS
      // Measurement manager not usefull in packet transfer mode
      // This permit to save CPU in packet transfer mode
      if (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_DISABLED)
    #endif
        // Call routine: MEAS_MANAGER.
        l1s_meas_manager();
    #if (L1_GTT == 1)

      #if (GSM_IDLE_RAM != 0)
        if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1))
      #endif //GSM_IDLE_RAM
          {
             // Call routine: GTT MANAGER.
             l1s_gtt_manager();
          }

    #endif

    #if (AUDIO_TASK == 1)
      // Call routine: AUDIO MANAGER.
      #if (GSM_IDLE_RAM != 0)
        if ( l1s.gsm_idle_ram_ctl.l1s_full_exec == TRUE)
          {
            l1s_audio_manager();
          }
      #else
            l1s_audio_manager();
      #endif
    #else
      #if (GSM_IDLE_RAM != 0)
        l1s.gsm_idle_ram_ctl.l1s_full_exec = FALSE;
      #endif
    #endif

// Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
    l1s_audio_onoff_manager();
#endif // L1_AUDIO_MCU_ONOFF

    #if(L1_DYN_DSP_DWNLD ==1)
      // Call routine: DSP DYNAMIC DOWNLOAD MANAGER
      l1s_dyn_dwnld_manager();
    #endif



    // Call routine: END_MANAGER.
    l1s_end_manager();
  }
  }

  #if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
    Trace_PM_Equal_0_balance();
  #endif

  #if (DSP_DEBUG_TRACE_ENABLE == 1)
    if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_DEBUG)
      #if (GSM_IDLE_RAM != 0)
        if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1))
      #endif
      	 {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD)
          // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
          if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif
          Trace_dsp_debug();
      	 }
    #if (AMR == 1)
      if (trace_info.current_config->l1_dyn_trace & 1<<L1_DYN_TRACE_DSP_AMR_DEBUG)
      #if (GSM_IDLE_RAM != 0)
        if ((l1a_l1s_com.mode > I_MODE) || (l1_config.TestMode == 1))
      #endif
      	 {
#if(MELODY_E2 || L1_MP3 || L1_AAC || L1_DYN_DSP_DWNLD )
          // DSP Trace is output ONLY if melody e2, mp3 or dynamic download are not currently running
          if(trace_info.dsptrace_handler_globals.trace_flag_blocked == FALSE)
#endif

        Trace_dsp_amr_debug();
      	 }
    #endif
  #endif

  // Vmemo/Reco sign-to-talk trace
  #if (TRACE_TYPE==2 ) || (TRACE_TYPE==3)
    uart_trace_multiframe();
  #endif

  #if (TRACE_TYPE == 1) || (TRACE_TYPE==4)
    if (l1s.actual_time.fn_mod13 == 12)
    {
      RTTL1_EVENT(RTTL1_EVENT_FNMOD13_EQUAL_12, RTTL1_EVENT_SIZE_FNMOD13_EQUAL_12)
    }
  #endif

  /******************************************************/
  /* if layer 1 ready to sleep, evaluate System loading.*/
  /*                                                    */
  /* Conditions are :                                   */
  /* - no RF/GSM task in progress                       */
  /* - next RF/GSM task at min in 4 frames              */
  /* - Layer1 in Idle mode                              */
  /******************************************************/

  #if (GSM_IDLE_RAM != 0)
    if (((l1a_l1s_com.mode == I_MODE) || (l1a_l1s_com.mode == CS_MODE0)) && (l1_config.TestMode == 0))
    {
      // Normally os_load and hw_timer shall be meaningful since last sleep phase without checking os - otherwise traffic controller is already on
      if ((l1s.gsm_idle_ram_ctl.os_load == 0) || (l1s.gsm_idle_ram_ctl.hw_timer == 0))
      {
        if (!READ_TRAFFIC_CONT_STATE)
        {
          CSMI_TrafficControllerOn();
          #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
            l1s_trace_mftab();
          #endif
        }
      }
    }

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
      if (READ_TRAFFIC_CONT_STATE)
      {
            l1_intram_send_trace();
      }
    #endif
  #endif

  if (l1_config.pwr_mngt == PWR_MNGT)
  {
    if ( (l1s.frame_count == 0)                               &&
         (l1a_l1s_com.time_to_next_l1s_task > MIN_SLEEP_TIME) &&
         (l1s.pw_mgr.gauging_task  == INACTIVE)               &&
         ((l1a_l1s_com.mode == I_MODE)||(l1a_l1s_com.mode == CS_MODE0)) )
    {
      // sleep mode is authorized by primitive ....
      if ( l1s.pw_mgr.mode_authorized >= BIG_SLEEP )
        {
          l1s_sleep_manager();
        }
    }
    #if 0	/* FreeCalypso TCS211 reconstruction */
    else{
      l1_trace_fail_sleep(FAIL_SLEEP_L1SYNCH,0,0);
    }
    #endif
  }

#if (GSM_IDLE_RAM_DEBUG == 1)
      (*( volatile unsigned short* )(0xFFFE4802)) &= ~ (1 << 2);    // GPIO-2=0
#endif
// Be careful: The Deep sleep can be performed just above
// Do not add something here !!!!

}

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if ((GSM_IDLE_RAM != 0))  //omaps00090550
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START

void l1s_keep_mftab_hist(void)
{
    UWORD8 task_id, bit;
    WORD8 nb_bitmap;
    T_L1S_GSM_IDLE_INTRAM * gsm_idle_ram_ctl;

    gsm_idle_ram_ctl = &(l1s.gsm_idle_ram_ctl);

    bit=0;

    for(nb_bitmap=0; nb_bitmap<SIZE_TAB_L1S_MONITOR; nb_bitmap++)
    {
      gsm_idle_ram_ctl->mem_task_bitmap_idle_ram[nb_bitmap] = gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap];
      gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap]=0;
    }

    nb_bitmap=0;

    for(task_id=0; task_id<NBR_DL_L1S_TASKS; task_id++)
    {
      gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap] |= ((!(l1s.task_status[task_id].current_status == INACTIVE)) << bit);
      bit++;

      if ((bit == 32) || (task_id == (NBR_DL_L1S_TASKS -1)))
      {
        bit = 0;
        nb_bitmap++;
      }
    }
}

BOOL l1s_mftab_has_changed(void)
{
  WORD8 nb_bitmap;
  UWORD32 diff_detected;
  T_L1S_GSM_IDLE_INTRAM * gsm_idle_ram_ctl;

  gsm_idle_ram_ctl = &(l1s.gsm_idle_ram_ctl);

  diff_detected=0;
  for(nb_bitmap=0; nb_bitmap<SIZE_TAB_L1S_MONITOR; nb_bitmap++)
  {
    diff_detected |= ((gsm_idle_ram_ctl->mem_task_bitmap_idle_ram[nb_bitmap] ^ gsm_idle_ram_ctl->task_bitmap_idle_ram[nb_bitmap]));
  }
  return (diff_detected != 0);
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_task_scheduler_process()                          */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is the task scheduler of L1S. It        */
/* schedules any enabled task. When a task must start,   */
/* it becomes PENDING. Since several tasks can become    */
/* pending at the same time, the highest priority one    */
/* is elected. The elected task compete then with the    */
/* current running task. If they conflict, the highest   */
/* priority one wins. If the winning is the new comer    */
/* then the multiframe table is reset and the new coming */
/* task is installed.                                    */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_task_scheduler_process()
{
  WORD32 pending_task;

  // Call routine: SCHEDULE_TASKS.
  l1s_schedule_tasks(&pending_task);

  // Call routine: MERGE_MANAGER (contains LOAD_MFTAB).
  l1s_merge_manager(pending_task);

#if (GSM_IDLE_RAM != 0)
  l1s_keep_mftab_hist();
  if (((l1a_l1s_com.mode == I_MODE) || (l1a_l1s_com.mode == CS_MODE0)) && (l1_config.TestMode == 0))
  {
    l1s_adapt_traffic_controller();
  }
#endif

}

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_schedule_tasks()                                  */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function schedules all L1S tasks except measure- */
/* -ment tasks which are handled separately.             */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_schedule_tasks(WORD32 *best_pending_task)
{
  UWORD8 task_id;

  T_TASK_STATUS *task_ptr = &(l1s.task_status[0]);

  #if ((TRACE_TYPE == 1)||(TRACE_TYPE == 4))
    UWORD8 nb_bitmap = 0;
    UWORD8 bit = 0;
  #endif

  // Reset "new_status" for all L1S tasks: make them NOT_PENDING.
  for(task_id=0; task_id<NBR_DL_L1S_TASKS; task_id++)
  {
    task_ptr->new_status = NOT_PENDING;
    task_ptr->time_to_exec = MAX_FN;
    task_ptr++;

    #if ((TRACE_TYPE == 1)||(TRACE_TYPE == 4))
      // L1S Task enabling trace
      trace_info.task_bitmap[nb_bitmap] |= l1a_l1s_com.l1s_en_task[task_id] << bit;
      bit++;
      if (bit == 32)
      {
        bit = 0;
        nb_bitmap++;
      }
    #endif
  }

  #if ((TRACE_TYPE == 1)||(TRACE_TYPE == 4))
    if(SELECTED_BITMAP(RTTL1_ENABLE_L1S_TASK_ENABLE))
    {
      // For the moment up to 64 tasks supported !!!
      if ((trace_info.task_bitmap[0] != trace_info.mem_task_bitmap[0]) ||
          (trace_info.task_bitmap[1] != trace_info.mem_task_bitmap[1]))
      {
        RTTL1_FILL_L1S_TASK_ENABLE(trace_info.task_bitmap[0], trace_info.task_bitmap[1])
      }

      trace_info.mem_task_bitmap[0] = trace_info.task_bitmap[0];
      trace_info.mem_task_bitmap[1] = trace_info.task_bitmap[1];
      trace_info.task_bitmap[0]     = 0;
      trace_info.task_bitmap[1]     = 0;
    }
  #endif

#if ((REL99 == 1) && (FF_BHO == 1))
  if ((l1a_l1s_com.l1s_en_task[FBSB] == TASK_ENABLED) && (l1s.task_status[FBSB].current_status == INACTIVE))
  //----------------------------------
  // FBSB task is ENABLED.
  //----------------------------------
  {
    l1s.task_status[FBSB].time_to_exec = 0;

  }
#endif // #if ((REL99 == 1) && (FF_BHO == 1))

  if(l1a_l1s_com.l1s_en_task[SYNCHRO] == TASK_ENABLED)
  //--------------------------------------------
  // Synchro (jump on new Cell) task is ENABLED.
  //--------------------------------------------
  {
    // SYNCHRO task is not schedule if we are in the specific case:
    // L1A is touching SYNCHRO parameters (tn_difference, dl_tn and dsp_scheduler_mode)
    // and leave L1A to go in HISR (L1S) in middle of the update (cf. BUG1339)
    if(l1a_l1s_com.task_param[SYNCHRO] == SEMAPHORE_RESET)
    {
      // Save scheduling result.
      l1s.task_status[SYNCHRO].time_to_exec = 0;
    }
  }

  if (l1a_l1s_com.mode == CS_MODE0)
  if((l1a_l1s_com.l1s_en_task[ADC_CSMODE0] == TASK_ENABLED) &&
     (l1s.task_status[ADC_CSMODE0].current_status == INACTIVE))
  if ((l1a_l1s_com.l1s_en_meas & FSMS_MEAS) == 0) // avoid conflict with the Measurement campaign
  //--------------------------------
  // ADC task is ENABLED in CS_MODE0.
  //--------------------------------
  {
    UWORD32  time_to_adc;

    if (l1a_l1s_com.adc_mode & ADC_NEXT_CS_MODE0)
    {
      time_to_adc = 0; // ADC performed in the current frame
    }
    else
    if (l1a_l1s_com.adc_mode & ADC_EACH_CS_MODE0) // perform ADC on each "idle_period" * 102
    {
      time_to_adc = (102 * l1a_l1s_com.adc_idle_period-1) - (l1s.actual_time.fn % (102 * l1a_l1s_com.adc_idle_period));
    }

    // Save scheduling result.
    l1s.task_status[ADC_CSMODE0].time_to_exec = time_to_adc;
  }



  if((l1a_l1s_com.l1s_en_task[NP] == TASK_ENABLED) &&
     (l1s.task_status[NP].current_status == INACTIVE))
  //-------------------------------
  // Normal Paging task is ENABLED.
  //-------------------------------
  {
    UWORD8   mf51_for_ms_paging;
    UWORD32  np_position;
    UWORD32  paging_period;
    UWORD32  time_to_np;
    UWORD32  fn;


    fn = l1s.actual_time.fn;

    #if (L1_GPRS)
    //In case of network mode of operation II or III, CCCH reading is possible
    //in packet idle mode and in packet transfer mode.
    //But the SYNCHRO task is not used anymore as opposite to CS mode for CCCH readings
    if ((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED))
    {
      // if CCCH timeslot is lower than current timeslot, it means that the scheduling
      //    must be anticipated by 1 frame

      if((l1a_l1s_com.ccch_group * 2) < l1a_l1s_com.dl_tn)
      {
        fn = l1s.next_time.fn;
        l1s.ctrl_synch_before = TRUE;
      }
      else
        l1s.ctrl_synch_before = FALSE;
    }
    #endif

    // compute MF51 number (0 to 8) in a Paging Period which carries the Paging.
    mf51_for_ms_paging  = l1a_l1s_com.page_group / l1a_l1s_com.nb_pch_per_mf51;

    np_position   = (l1a_l1s_com.idle_task_info.pg_position - 1) + (mf51_for_ms_paging * 51);
    paging_period = l1a_l1s_com.bs_pa_mfrms * 51;
    time_to_np    = (np_position + paging_period - (fn % paging_period)) % paging_period;

    // Save scheduling result.
    l1s.task_status[NP].time_to_exec = time_to_np;

    // Inform Gauging scheduler that NP task is pending....
    if (time_to_np == 0)
      l1s.pw_mgr.paging_scheduled = TRUE;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[NP] = SEMAPHORE_RESET;
  }

  if((l1a_l1s_com.l1s_en_task[EP] == TASK_ENABLED) &&
     (l1s.task_status[EP].current_status == INACTIVE))
  //---------------------------------
  // Extended Paging task is ENABLED.
  //---------------------------------
  {
    UWORD8   mf51_for_ms_paging;
    UWORD32  ep_position;
    UWORD32  paging_period;
    UWORD32  time_to_ep;
    UWORD32  fn;

    fn = l1s.actual_time.fn;

    #if (L1_GPRS)
    //In case of network mode of operation II or III, CCCH reading is possible
    //in packet idle mode and in packet transfer mode.
    //But the SYNCHRO task is not used anymore as opposite to CS mode for CCCH readings
    if ((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
        (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED))
    {
      // if CCCH timeslot is lower than current timeslot, it means that the scheduling
      //    must be anticipated by 1 frame
      if((l1a_l1s_com.ccch_group * 2) < l1a_l1s_com.dl_tn)
      {
        fn = l1s.next_time.fn;
        l1s.ctrl_synch_before = TRUE;
      }
      else
        l1s.ctrl_synch_before = FALSE;
    }
    #endif


    // compute MF51 number (0 to 8) in a Paging Period which carries the Paging.
    mf51_for_ms_paging  = ((l1a_l1s_com.page_group + 2) / l1a_l1s_com.nb_pch_per_mf51) % l1a_l1s_com.bs_pa_mfrms;

    ep_position   = (l1a_l1s_com.idle_task_info.extpg_position - 1) + (mf51_for_ms_paging * 51);
    paging_period = l1a_l1s_com.bs_pa_mfrms * 51;
    time_to_ep    = (ep_position + paging_period - (fn % paging_period)) % paging_period;

    // Save scheduling result.
    l1s.task_status[EP].time_to_exec = time_to_ep;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[EP] = SEMAPHORE_RESET;
  }

  if((l1a_l1s_com.l1s_en_task[NBCCHS] == TASK_ENABLED) &&
     (l1s.task_status[NBCCHS].current_status == INACTIVE))
  //-------------------------------------
  // Normal BCCH Serving task is ENABLED.
  //-------------------------------------
  {
    UWORD32  min_time_to_nbcchs = MAX_FN;
    WORD32   time_to_nbcchs;
    WORD16   time_in_mf51;
    WORD32   fn_div_51 = l1s.actual_time.fn / 51;
    UWORD8   i;
    UWORD16  modulus;
    WORD16   relative_position;
    WORD32   modulus_times_51;
    WORD16   current_mf51_position;

    // NBCCHS task starts in frame position "1" in the MF51.
    time_in_mf51 = 1 - l1s.actual_time.t3;

    #if (L1_GPRS)
      if(l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
      {
        // In transfer mode, if l1a_l1s_com.dl_tn != 0, a change synchro is performed
        // So the CTRL must be compute the frame before
        if(l1a_l1s_com.dl_tn != 0)
          time_in_mf51 -- ;
      }
    #endif

    for(i=0;i<l1a_l1s_com.nbcchs.schedule_array_size;i++)
    {
      modulus               = l1a_l1s_com.nbcchs.schedule_array[i].modulus;
      relative_position     = l1a_l1s_com.nbcchs.schedule_array[i].relative_position;
      modulus_times_51      = modulus * 51;
      current_mf51_position = fn_div_51 % modulus;

      time_to_nbcchs = time_in_mf51 + (relative_position - current_mf51_position)*51;

      if(time_to_nbcchs < 0)
        time_to_nbcchs += modulus_times_51;
      else
        if(time_to_nbcchs >= modulus_times_51)
          time_to_nbcchs -= modulus_times_51;

      if(time_to_nbcchs < min_time_to_nbcchs)
        min_time_to_nbcchs = time_to_nbcchs;
    }

    // Save scheduling result.
    l1s.task_status[NBCCHS].time_to_exec = min_time_to_nbcchs;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[NBCCHS] = SEMAPHORE_RESET;

  } // End of "if / NBCCHS"

  if((l1a_l1s_com.l1s_en_task[EBCCHS] == TASK_ENABLED) &&
     (l1s.task_status[EBCCHS].current_status == INACTIVE))
  //---------------------------------------
  // Extended BCCH Serving task is ENABLED.
  //---------------------------------------
  {
    UWORD32  min_time_to_ebcchs = MAX_FN;
    WORD32   time_to_ebcchs;
    WORD16   time_in_mf51;
    WORD32   fn_div_51 = l1s.actual_time.fn / 51;
    UWORD8   i;
    UWORD16  modulus;
    WORD16   relative_position;
    WORD32   modulus_times_51;
    WORD16   current_mf51_position;

    // EBCCHS task starts in frame position "5" in the MF51.
    time_in_mf51 = 5 - l1s.actual_time.t3;

    #if (L1_GPRS)
      if(l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
      {
        // 3 cases are considered:
        //  => the l1a_l1s_com.dl_tn = {7,6,5,4,3,2,1}
        //       the BCCHS burst is in the previous frame than the PDTCH, so the CTRL must be done
        //       on the previous frame
        //  => the l1a_l1s_com.dl_tn = {0}
        //       the BCCHS burst is in the same frame than the PDTCH, so the CTRL must be done
        //       on the same frame
        if(l1a_l1s_com.dl_tn != 0)
          time_in_mf51 -- ; // CTRL done on the previous frame
      }
    #endif

    for(i=0;i<l1a_l1s_com.ebcchs.schedule_array_size;i++)
    {
      modulus               = l1a_l1s_com.ebcchs.schedule_array[i].modulus;
      relative_position     = l1a_l1s_com.ebcchs.schedule_array[i].relative_position;
      modulus_times_51      = modulus * 51;
      current_mf51_position = fn_div_51 % modulus;

      time_to_ebcchs = time_in_mf51 + (relative_position - current_mf51_position)*51;

      if(time_to_ebcchs < 0)
        time_to_ebcchs += modulus_times_51;
      else
        if(time_to_ebcchs >= (WORD32)modulus_times_51)
          time_to_ebcchs -= modulus_times_51;

      if(time_to_ebcchs < (WORD32)min_time_to_ebcchs)
        min_time_to_ebcchs = time_to_ebcchs;
    }

    // Save scheduling result.
    l1s.task_status[EBCCHS].time_to_exec = min_time_to_ebcchs;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[EBCCHS] = SEMAPHORE_RESET;

  } // End of "if / EBCCHS"

  if(l1a_l1s_com.l1s_en_task[ALLC] == TASK_ENABLED)
  //---------------------------------
  // ALL CCCH reading is ENABLED.
  //---------------------------------
  {
    if(l1a_l1s_com.task_param[ALLC] == SEMAPHORE_RESET)
    {
    #define CCCH0_START_TIME   6 - 1   // CCCH block 0.
    #define CCCH1_START_TIME  12 - 1   // CCCH block 1.
    #define CCCH2_START_TIME  16 - 1   // CCCH block 2.
    #define CCCH3_START_TIME  22 - 1   // CCCH block 3.
    #define CCCH4_START_TIME  26 - 1   // CCCH block 4.
    #define CCCH5_START_TIME  32 - 1   // CCCH block 5.
    #define CCCH6_START_TIME  36 - 1   // CCCH block 6.
    #define CCCH7_START_TIME  42 - 1   // CCCH block 7.
    #define CCCH8_START_TIME  46 - 1   // CCCH block 8.

    UWORD32  min_time_to_allc = MAX_FN;

    if(l1s.actual_time.t3 <= CCCH0_START_TIME)
      min_time_to_allc = CCCH0_START_TIME - l1s.actual_time.t3;
    else
    if(l1s.actual_time.t3 <= CCCH1_START_TIME)
      min_time_to_allc = CCCH1_START_TIME - l1s.actual_time.t3;
    else
    if(l1s.actual_time.t3 <= CCCH2_START_TIME)
      min_time_to_allc = CCCH2_START_TIME - l1s.actual_time.t3;

    // CCCH3 to CCCH8 are considered only when MF51 is not combined.
    else
    if(l1a_l1s_com.bcch_combined == FALSE)
    {
      if(l1s.actual_time.t3 <= CCCH3_START_TIME)
        min_time_to_allc = CCCH3_START_TIME - l1s.actual_time.t3;
      else
      if(l1s.actual_time.t3 <= CCCH4_START_TIME)
        min_time_to_allc = CCCH4_START_TIME - l1s.actual_time.t3;
      else
      if(l1s.actual_time.t3 <= CCCH5_START_TIME)
        min_time_to_allc = CCCH5_START_TIME - l1s.actual_time.t3;
      else
      if(l1s.actual_time.t3 <= CCCH6_START_TIME)
        min_time_to_allc = CCCH6_START_TIME - l1s.actual_time.t3;
      else
      if(l1s.actual_time.t3 <= CCCH7_START_TIME)
        min_time_to_allc = CCCH7_START_TIME - l1s.actual_time.t3;
      else
      if(l1s.actual_time.t3 <= CCCH8_START_TIME)
        min_time_to_allc = CCCH8_START_TIME - l1s.actual_time.t3;
      // Attempt to read CCCH0.
      else
        min_time_to_allc = 51 + CCCH0_START_TIME - l1s.actual_time.t3;
    }

    // Attempt to read CCCH0.
    else
      min_time_to_allc = 51 + CCCH0_START_TIME - l1s.actual_time.t3;

    // Save scheduling result.
    l1s.task_status[ALLC].time_to_exec = min_time_to_allc;
    }

    else
    // Semaphore is Set, reset it when ALLC inactive.
    {
    if(l1s.task_status[ALLC].current_status == INACTIVE)
      l1a_l1s_com.task_param[ALLC] = SEMAPHORE_RESET;
  }
  }

  if(l1a_l1s_com.l1s_en_task[SMSCB] == TASK_ENABLED)
  //------------------------------------------------------
  // Short Message Service Cell Broadcast task is ENABLED.
  //------------------------------------------------------
  {
    if(l1s.task_status[SMSCB].current_status == INACTIVE)
    {
      WORD32   time_to_norm_smscb = MAX_FN;
      WORD32   time_to_ext_smscb  = MAX_FN;
      WORD32   time_to_smscb_info = MAX_FN;
      UWORD32  min_time_to_smscb;

      if(l1a_l1s_com.cbch_info_req.next < l1a_l1s_com.cbch_info_req.cbch_num)
      {
        // Still some CBCH blocks to read from TB1/2/3/5/6/7, get next one.
        time_to_smscb_info = l1a_l1s_com.cbch_info_req.start_fn[l1a_l1s_com.cbch_info_req.next] +
                             MAX_FN -
                             l1s.actual_time.fn;

        if(time_to_smscb_info >= MAX_FN) time_to_smscb_info -= MAX_FN;

        // Check if passing 1 schedule position.
        if(time_to_smscb_info == 0)
          l1a_l1s_com.cbch_info_req.next++;
      }

      else
      {
        //%%%%%%%%%%%%%%%%
        // Normal CBCH...
        //%%%%%%%%%%%%%%%%
        // Check for scheduling info.

        if(l1a_l1s_com.norm_cbch_schedule.cbch_state == CBCH_SCHEDULED)
        {
          // CBCH header (TB0) reading is scheduled.

          if(l1a_l1s_com.norm_cbch_schedule.next < l1a_l1s_com.norm_cbch_schedule.cbch_num)
          {
            // Still some scheduled CBCH to read, get next one.
            time_to_norm_smscb = l1a_l1s_com.norm_cbch_schedule.first_block[l1a_l1s_com.norm_cbch_schedule.next] +
                                 MAX_FN -
                                 l1s.actual_time.fn;

            if(time_to_norm_smscb >= MAX_FN) time_to_norm_smscb -= MAX_FN;

            // Check if passing 1 schedule position.
            if(time_to_norm_smscb == 0)
              l1a_l1s_com.norm_cbch_schedule.next++;
          }

          else
          {
            // No more scheduled CBCH/TB0.
            l1a_l1s_com.norm_cbch_schedule.cbch_state = CBCH_CONTINUOUS_READING;
          }
        }

        if(l1a_l1s_com.norm_cbch_schedule.cbch_state == CBCH_CONTINUOUS_READING)
        {
          // CBCH header (TB0) reading is continuous.

          if(l1a_l1s_com.norm_cbch_schedule.start_continuous_fn != -1)
          {
            time_to_norm_smscb = l1a_l1s_com.norm_cbch_schedule.start_continuous_fn +
                                 MAX_FN -
                                 l1s.actual_time.fn;

            if(time_to_norm_smscb >= MAX_FN) time_to_norm_smscb -= MAX_FN;

            // Check for "CBCH continuous reading" starting frame number.
            if(time_to_norm_smscb == 0)
              l1a_l1s_com.norm_cbch_schedule.start_continuous_fn = -1;
          }

          else
          {
            // Continuous CBCH/TB0 reading is ongoing.

            WORD32 time_in_mf51;

            // No more scheduled CBCH to read, we must read all TB0.

            time_in_mf51 = l1a_l1s_com.cbch_start_in_mf51 - l1s.actual_time.t3;

            // Time to next TB0 CBCH block.
            time_to_norm_smscb = time_in_mf51 + (8-l1s.actual_time.tc)*51;
            if(time_to_norm_smscb <  0)    time_to_norm_smscb += 8*51;
            if(time_to_norm_smscb >= 8*51) time_to_norm_smscb -= 8*51;
          }
        }

        //%%%%%%%%%%%%%%%%%
        // Extended CBCH...
        //%%%%%%%%%%%%%%%%%
        // Check for scheduling info.

        if(l1a_l1s_com.ext_cbch_schedule.cbch_state == CBCH_SCHEDULED)
        {
          // CBCH header (TB4) reading is scheduled.

          if(l1a_l1s_com.ext_cbch_schedule.next < l1a_l1s_com.ext_cbch_schedule.cbch_num)
          {
            // Still some scheduled CBCH to read, get next one.
            time_to_ext_smscb = l1a_l1s_com.ext_cbch_schedule.first_block[l1a_l1s_com.ext_cbch_schedule.next] +
                                 MAX_FN -
                                 l1s.actual_time.fn;

            if(time_to_ext_smscb >= MAX_FN) time_to_ext_smscb -= MAX_FN;

            // Check if passing 1 schedule position.
            if(time_to_ext_smscb == 0)
              l1a_l1s_com.ext_cbch_schedule.next++;  // passing 1 schedule position.
          }

          else
          {
            // No more scheduled CBCH/TB4.
            l1a_l1s_com.ext_cbch_schedule.cbch_state = CBCH_CONTINUOUS_READING;
          }
        }

        if(l1a_l1s_com.ext_cbch_schedule.cbch_state == CBCH_CONTINUOUS_READING)
        {
          // Check for "CBCH continuous reading " starting frame number.

          if(l1a_l1s_com.ext_cbch_schedule.start_continuous_fn != -1)
          {
            time_to_ext_smscb = l1a_l1s_com.ext_cbch_schedule.start_continuous_fn +
                                 MAX_FN -
                                 l1s.actual_time.fn;

            if(time_to_ext_smscb >= MAX_FN) time_to_ext_smscb -= MAX_FN;

            // Check for "CBCH continuous reading" starting frame number.
            if(time_to_ext_smscb == 0)
              l1a_l1s_com.ext_cbch_schedule.start_continuous_fn = -1;
          }

          else
          {
            // Continuous CBCH/TB4 reading is ongoing.

            WORD32 time_in_mf51;

            // No more scheduled CBCH to read, we must read all TB4.

            time_in_mf51 = l1a_l1s_com.cbch_start_in_mf51 - l1s.actual_time.t3;

            // Time to next TB4 CBCH block.
            time_to_ext_smscb = time_in_mf51 + (4-l1s.actual_time.tc)*51;
            if(time_to_ext_smscb <  0)    time_to_ext_smscb += 8*51;
            if(time_to_ext_smscb >= 8*51) time_to_ext_smscb -= 8*51;
          }
        }
      }

      // Choose closest one...
      if(time_to_norm_smscb < time_to_ext_smscb)
        min_time_to_smscb = time_to_norm_smscb;
      else
        min_time_to_smscb = time_to_ext_smscb;

      if(time_to_smscb_info < min_time_to_smscb)
        min_time_to_smscb = time_to_smscb_info;

      // Save scheduling result.
      l1s.task_status[SMSCB].time_to_exec = min_time_to_smscb;

      // Clear param. synchro. semaphore.
      l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_RESET;
    }
  }

  //---------------------------------------------
  // Random Access management for ACCESS phase.
  //---------------------------------------------
  if(l1a_l1s_com.l1s_en_task[RAACC] == TASK_ENABLED)
  // Random Access (ACCESS mode) task is ENABLED.
  {
    // RAACC task requires to run L1S scheduler every frame.
    l1a_l1s_com.time_to_next_l1s_task = 0;

    if((l1a_l1s_com.bcch_combined == FALSE) ||
       (COMBINED_RA_DISTRIB[l1s.actual_time.t3] == TRUE))
    // Current frame is at a "slot" boundary -> decrement time to next RA.
    {
      if(l1a_l1s_com.ra_info.rand == 0)
      // It is time to controle a RACH transmit.
      {
        l1s.task_status[RAACC].new_status = PENDING;
      }

      // Decrement "rand" value after test to avoid a negative rand when L3
      // specifies a rand = 0
      l1a_l1s_com.ra_info.rand --;
    }
  }

  #if (L1_GPRS)
    // BCCHN task ENABLED and NBCCH task is INACTIVE
    if(((l1a_l1s_com.l1s_en_task[BCCHN     ] == TASK_ENABLED) && (l1s.task_status[BCCHN     ].current_status == INACTIVE)) ||
       ((l1a_l1s_com.l1s_en_task[BCCHN_TOP ] == TASK_ENABLED) && (l1s.task_status[BCCHN_TOP ].current_status == INACTIVE)) ||
       ((l1a_l1s_com.l1s_en_task[BCCHN_TRAN] == TASK_ENABLED) && (l1s.task_status[BCCHN_TRAN].current_status == INACTIVE)))
  #else
    // BCCHN task is ENABLED.
    if(((l1a_l1s_com.l1s_en_task[BCCHN    ] == TASK_ENABLED) && (l1s.task_status[BCCHN    ].current_status == INACTIVE)) ||
       ((l1a_l1s_com.l1s_en_task[BCCHN_TOP] == TASK_ENABLED) && (l1s.task_status[BCCHN_TOP].current_status == INACTIVE)))
  #endif
  {
    {
      UWORD32  neigh_fn;
      UWORD8   neigh_tc;
      UWORD8   neigh_fn_mod51;
      WORD32   time_to_bcchn;
      WORD16   time_in_mf51;
      UWORD16  si_bitmap;
      UWORD8   first_possible_neigh_tc;
      UWORD8   i;
      UWORD8   tc_count;
      UWORD8   ext_bcch_start_time;
      UWORD8   bcchn_priority;

      //array in order to memorize for each priority the closest NBCCH
      // 3 priorities: TOP_PRIORITY, HIGH_PRIORITY, NORMAL_PRIORITY
      UWORD32  min_time_to_bcchn[3] = {MAX_FN,MAX_FN,MAX_FN};
      UWORD8   best_neigh_id[3]     = {0,0,0};
      UWORD8   best_neigh_tc[3]     = {0,0,0};

      // Up to 6 pending Ncell BCCH reading.
      for(i=0;i<6;i++)
      {
        // Consider only the "in use" locations from the "6 neigh. list".
        if(l1a_l1s_com.bcchn.list[i].status != NSYNC_FREE)
        {
          // Get neighbor cell FN.
          neigh_fn = (l1s.actual_time.fn + l1a_l1s_com.bcchn.list[i].fn_offset) % MAX_FN;

          // Get neighbor cell TC.
          neigh_tc = (neigh_fn / 51) % 8;

          // Get neighbor cell TC.
          neigh_fn_mod51 = (neigh_fn % 51);

          //-----------------
          // Normal BCCH...
          //-----------------

          // Still some Normal BCCH to read.
          if(l1a_l1s_com.bcchn.list[i].bcch_blks_req & 0x00FF)
          {
            #if (L1_GPRS)
              // in case of packet transfer mode there is no measurement window
              if(l1a_l1s_com.l1s_en_task[BCCHN_TRAN] == TASK_ENABLED)
              {
                // Since Normal BCCH reading must start in FN_mod_51=50+2=1, current TC cannot
                // be read. First possible TC is therefore the next one.
                time_in_mf51 = 1 - neigh_fn_mod51;

                if(neigh_fn_mod51 > 1)
                  first_possible_neigh_tc = neigh_tc + 1;
                else
                  first_possible_neigh_tc = neigh_tc;
              }
              else
              {
                // Since Normal BCCH reading must start in FN_mod_51=50, current TC cannot
                // be read. First possible TC is therefore the next one.
                time_in_mf51            = 50 - neigh_fn_mod51;
                first_possible_neigh_tc = neigh_tc + 1;
              }
            #else
              // Since Normal BCCH reading must start in FN_mod_51=50, current TC cannot
              // be read. First possible TC is therefore the next one.
              time_in_mf51            = 50 - neigh_fn_mod51;
              first_possible_neigh_tc = neigh_tc + 1;
            #endif

            if(first_possible_neigh_tc >= 8)
              first_possible_neigh_tc -=8;

            // Get the duplicate version of the Normal BCCH si_bitmap.
            si_bitmap = l1a_l1s_com.bcchn.list[i].bcch_blks_req;

            // Look for 1st bit activated from current TC.
            tc_count = 0;
            while((!(si_bitmap & (1L << first_possible_neigh_tc))) && (tc_count < 8))
            {
              tc_count++;
              first_possible_neigh_tc++;
              if(first_possible_neigh_tc >= 8) first_possible_neigh_tc -=8;
            }

            // Compute time to wait until NBCCH activation.
            #if (L1_GPRS)
              if(l1a_l1s_com.l1s_en_task[BCCHN_TRAN] == TASK_ENABLED)
                time_to_bcchn  = time_in_mf51 + (first_possible_neigh_tc - neigh_tc    )*51;
              else
                time_to_bcchn  = time_in_mf51 + (first_possible_neigh_tc - neigh_tc - 1)*51;
            #else
                time_to_bcchn  = time_in_mf51 + (first_possible_neigh_tc - neigh_tc - 1)*51;
            #endif

            // Prevent negative result.
            if(time_to_bcchn < 0)
              time_to_bcchn += 8*51;
            else
              if(time_to_bcchn >= 8*51)
                time_to_bcchn -= 8*51;

            // memorize the next BCCHN according to its priority
            // (TOP_PRIORITY or HIGH_PRIORITY or NORMAL_PRIORITY )
            bcchn_priority = l1a_l1s_com.bcchn.list[i].gprs_priority ;
            if(time_to_bcchn < min_time_to_bcchn[bcchn_priority])
            {
              min_time_to_bcchn[bcchn_priority] = time_to_bcchn;

              // Save Neighbour number
              best_neigh_id[bcchn_priority] = i;
              best_neigh_tc[bcchn_priority] = first_possible_neigh_tc;
            }
          }

          //-----------------
          // Extended BCCH...
          //-----------------

          // Still some Extended BCCH to read.
          if(l1a_l1s_com.bcchn.list[i].bcch_blks_req & 0xFF00)
          {
            #if (L1_GPRS)
              // in case of packet transfer mode there are no measurement windows
              if(l1a_l1s_com.l1s_en_task[BCCHN_TRAN] == TASK_ENABLED)
                 ext_bcch_start_time=3+2;
              else
                 ext_bcch_start_time=3;
            #else
                 ext_bcch_start_time=3;
            #endif

            // Extended BCCH reading must start in FN_mod_51=ext_bcch_start_time.
            // Check if current TC could be read immediately.
            time_in_mf51 = ext_bcch_start_time - neigh_fn_mod51;

            if(neigh_fn_mod51 > ext_bcch_start_time)
              first_possible_neigh_tc = neigh_tc + 1;
            else
              first_possible_neigh_tc = neigh_tc;

            if(first_possible_neigh_tc >= 8)
              first_possible_neigh_tc -=8;

            // Offset TC by 8 to be within the EBCCH bitmap.
            first_possible_neigh_tc += 8;

            // Get the duplicate version of the Extended BCCH si_bitmap.
            si_bitmap = l1a_l1s_com.bcchn.list[i].bcch_blks_req;

            // Look for 1st bit activated from current TC.
            tc_count = 0;
            while((!(si_bitmap & (1L << first_possible_neigh_tc))) && (tc_count < 8))
            {
              tc_count++;
              first_possible_neigh_tc++;
              if(first_possible_neigh_tc >= 16) first_possible_neigh_tc -=8;
            }

            // Compute time to wait until NBCCH activation.
            time_to_bcchn  = time_in_mf51 + (first_possible_neigh_tc - neigh_tc - 8)*51;

            // Prevent negative result.
            if(time_to_bcchn < 0)
              time_to_bcchn += 8*51;
            else
              if(time_to_bcchn >= 8*51)
                time_to_bcchn -= 8*51;

            // memorize the next BCCHN according to its priority
            // (TOP_PRIORITY or HIGH_PRIORITY or NORMAL_PRIORITY )
            bcchn_priority = l1a_l1s_com.bcchn.list[i].gprs_priority;
            if(time_to_bcchn < min_time_to_bcchn[bcchn_priority])
            {
              min_time_to_bcchn[bcchn_priority] = time_to_bcchn;

              // Save Neighbour number
              best_neigh_id[bcchn_priority] = i;
              best_neigh_tc[bcchn_priority] = first_possible_neigh_tc;
            }
          }
        }
      }

      // Save scheduling result.
      #if L1_GPRS
      if(l1a_l1s_com.l1s_en_task[BCCHN_TRAN] == TASK_ENABLED)
      {
        // in packet transfer only task one task is allowed: BCCHN_TRAN with a TOP priority
        l1s.task_status[BCCHN_TRAN].time_to_exec = min_time_to_bcchn[TOP_PRIORITY];
        l1a_l1s_com.bcchn.active_neigh_id_top    = best_neigh_id[TOP_PRIORITY];
        l1a_l1s_com.bcchn.active_neigh_tc_top    = best_neigh_tc[TOP_PRIORITY];
        l1a_l1s_com.task_param[BCCHN_TRAN]       = SEMAPHORE_RESET;
      }
      else
      #endif
      {
        // in IDLE 2 tasks are allowed: BCCHN_TOP with a TOP priority or BCCHN with normal and high priorities
        // these 2 tasks may be enabled together.
        if((l1a_l1s_com.l1s_en_task[BCCHN_TOP] == TASK_ENABLED)&&(l1s.task_status[BCCHN_TOP].current_status == INACTIVE))
        {
          // update only if the task is not being running.
          l1s.task_status[BCCHN_TOP].time_to_exec = min_time_to_bcchn[TOP_PRIORITY];
          l1a_l1s_com.bcchn.active_neigh_id_top   = best_neigh_id[TOP_PRIORITY];
          l1a_l1s_com.bcchn.active_neigh_tc_top   = best_neigh_tc[TOP_PRIORITY];
          l1a_l1s_com.task_param[BCCHN_TOP]       = SEMAPHORE_RESET;
        }

        if((l1a_l1s_com.l1s_en_task[BCCHN] == TASK_ENABLED) && (l1s.task_status[BCCHN].current_status == INACTIVE))
        {
          // update only if the task is not being running.
          l1a_l1s_com.task_param[BCCHN] = SEMAPHORE_RESET;

          if(min_time_to_bcchn[HIGH_PRIORITY] < min_time_to_bcchn[NORMAL_PRIORITY] + BLOC_BCCHN_SIZE-1)
          {
            l1s.task_status[BCCHN].time_to_exec    = min_time_to_bcchn[HIGH_PRIORITY];
            l1a_l1s_com.bcchn.active_neigh_id_norm = best_neigh_id[HIGH_PRIORITY];
            l1a_l1s_com.bcchn.active_neigh_tc_norm = best_neigh_tc[HIGH_PRIORITY];
          }
          else
          {
            l1s.task_status[BCCHN].time_to_exec    = min_time_to_bcchn[NORMAL_PRIORITY];
            l1a_l1s_com.bcchn.active_neigh_id_norm = best_neigh_id[NORMAL_PRIORITY];
            l1a_l1s_com.bcchn.active_neigh_tc_norm = best_neigh_tc[NORMAL_PRIORITY];
          }
        }
      }

    } // End of "if / current_status"
  } // End of "if / BCCHN"

#if (L1_GPRS)
  if((l1a_l1s_com.l1s_en_task[PNP] == TASK_ENABLED) ||
     ((l1a_l1s_com.l1s_en_task[PEP] == TASK_ENABLED) &&
     (l1s.task_status[PEP].current_status == INACTIVE)))
  //-------------------------------------
  // Packet Normal Paging task is ENABLED.
  //-------------------------------------
  {
    #define  BS_PAG_BLKS    l1pa_l1ps_com.pccch.bs_pag_blks_res
    #define  BS_PBCCH_BLKS  l1pa_l1ps_com.pccch.bs_pbcch_blks

    UWORD16  fn_mod;
    UWORD16  paging_group;
    UWORD8   mf52_index;
    UWORD32  m_index_dvd;
    UWORD32  m_index_rest;
    UWORD16  block_index_in_mf64x52;
    UWORD16  m_index_dvd_intermediate;
    UWORD8   mf52_for_ms_pg;
    UWORD8   mf52_for_ms_epg;
    UWORD8   pg_block_index;
    WORD16   time_to_pnp;
    WORD16   time_to_pep, time_to_pep_2;
    UWORD8   pnp_blk_position;
    UWORD8   pep_blk_position;

    UWORD16  m_index;
    WORD16   m_for_pep;
    UWORD16  pnp_position, pep_position_2 =0;//omaps00090550

    static UWORD16  pep_position;

    UWORD8   i = 0;

    // Actual Frame Number modulo Paging Period
    fn_mod = l1s.actual_time.fn % (64*52);

    //----------------------------------------
    // Step1:
    //----------------------------------------
    // Find next potential PCCCH block index
    // according to current FN.
    //----------------------------------------

    // Translate actual time Frame Number in an element of the "paging Block
    // available" set.
    mf52_index = fn_mod / 52;

    // Look for next PCCCH block "potentially" containing PNP.
    // Result in "i".
    while(((l1s.actual_time.fn_mod52 - PACKET_PG_POSITION[(i + (BS_PAG_BLKS+BS_PBCCH_BLKS)*11)]) >= 0) &&
          (i < l1pa_l1ps_com.pccch.nb_ppch_per_mf52))
    {
      i++;
    }

    // Compute "potential" PNP block index in the MF64x52.
    // block_index_mf64x52 has the same dimension as PAGING_GROUP!!!
    block_index_in_mf64x52 = mf52_index * l1pa_l1ps_com.pccch.nb_ppch_per_mf52 + i;

    //----------------------------------------
    // Step2:
    //----------------------------------------
    // Reverse ETSI equation to compute "m"
    // index corresponding to the next coming
    // PNP block.
    //----------------------------------------

    // Compute intermediate value to avoid % usage.
    m_index_dvd_intermediate = block_index_in_mf64x52 -
                               l1pa_l1ps_com.pccch.first_pg_grp +
                               l1pa_l1ps_com.pccch.pg_blks_avail;

    if(m_index_dvd_intermediate >= l1pa_l1ps_com.pccch.pg_blks_avail)
      m_index_dvd_intermediate -= l1pa_l1ps_com.pccch.pg_blks_avail;

    // Compute the reverse of ETSI equation
    //  m_index = ((block_index_in_mf64x52 - ALPHA - BETA)*SPLIT)DIV M
    // with:
    // ALPHA = (IMSImod10000)div(KC*N)
    // BETA  = IMSImod1000
    m_index_dvd  = (UWORD32)m_index_dvd_intermediate * (UWORD32)l1pa_l1ps_com.pccch.split_pg_value;

    m_index      = (UWORD16)(m_index_dvd / (UWORD32)(l1pa_l1ps_com.pccch.pg_blks_avail));
    m_index_rest = m_index_dvd - (UWORD32)(m_index)*(UWORD32)l1pa_l1ps_com.pccch.pg_blks_avail;

    // Cope with rounding effect...
    // Choose next PAGING_GROUP index.
    // Note: if rest of the division "m_index_dvd/pg_blks_avail !=0"
    // then PAGING GROUP index must be incremented.
    if(m_index_rest != 0)
    {
      m_for_pep = m_index;
      m_index += 1;
    }
    else
    {
      m_for_pep = m_index - 1;;

      if(m_for_pep < 0)
        m_for_pep += l1pa_l1ps_com.pccch.split_pg_value;
    }

    //----------------------------------------
    // Step3:
    //----------------------------------------
    // Find PAGING GROUP associated with the
    // "m" index found in step 2.
    // Then compute MF52 containing this block
    // and associated block index in the MF52.
    //----------------------------------------

    // PAGING GROUP computation:
    paging_group    = (l1pa_l1ps_com.pccch.pg_offset + (((UWORD32)m_index * (UWORD32)l1pa_l1ps_com.pccch.pg_blks_avail)
                        / (UWORD32)l1pa_l1ps_com.pccch.split_pg_value)) % l1pa_l1ps_com.pccch.pg_blks_avail ;

    // Computation of the MF52 for MS Packet Paging
    mf52_for_ms_pg = paging_group / l1pa_l1ps_com.pccch.nb_ppch_per_mf52;

    // Paging Block Index computation
    pg_block_index = paging_group % l1pa_l1ps_com.pccch.nb_ppch_per_mf52;

    // Normal and Extended paging position
    pnp_blk_position =
            PACKET_PG_POSITION[(pg_block_index + (l1pa_l1ps_com.pccch.bs_pag_blks_res +
                                l1pa_l1ps_com.pccch.bs_pbcch_blks)*MAX_NBR_PG_BLKS)];

    // Computation of the PPCH block position in the MF52
    pnp_position = (pnp_blk_position - 1) + mf52_for_ms_pg * 52;

    // Time between actual fn and next PPCH block
    // Apply modulo 64*52.
    time_to_pnp = pnp_position - fn_mod + (64*52);

    if(time_to_pnp >= (64*52))
      time_to_pnp -= (64*52);

    // Save Time to next PPCH block position in a global structure.
    // "time_to_pnp" is used in Cell reselection measurement scheduling.
    l1pa_l1ps_com.pccch.time_to_pnp = time_to_pnp;

    // Semaphore is Set, reset it when PNP inactive.
    if(l1a_l1s_com.task_param[PNP] == SEMAPHORE_SET)
    {
      if(l1s.task_status[PNP].current_status == INACTIVE)
        l1a_l1s_com.task_param[PNP] = SEMAPHORE_RESET;
    }

    if(l1a_l1s_com.task_param[PNP] == SEMAPHORE_RESET)
    {
      // Save scheduling result.
      l1s.task_status[PNP].time_to_exec = time_to_pnp;

      if(time_to_pnp == 0)
      {
        // Force PEP scheduling computation
        l1pa_l1ps_com.pccch.epg_computation = PPCH_POS_NOT_COMP;
      }
    }

    if(l1a_l1s_com.l1s_en_task[PEP] == TASK_ENABLED)
      {
        if(l1pa_l1ps_com.pccch.epg_computation == PPCH_POS_NOT_COMP)
        {
          // PAGING GROUP computation:
          paging_group    = (l1pa_l1ps_com.pccch.pg_offset + (((UWORD32)(m_index) * (UWORD32)l1pa_l1ps_com.pccch.pg_blks_avail)
                              / (UWORD32)l1pa_l1ps_com.pccch.split_pg_value)) % l1pa_l1ps_com.pccch.pg_blks_avail;

          // Computation of the MF52 for MS Packet Extented Paging
          mf52_for_ms_epg = ((paging_group + 3) / l1pa_l1ps_com.pccch.nb_ppch_per_mf52) % 64;

          // Paging Block Index computation
          pg_block_index = paging_group % l1pa_l1ps_com.pccch.nb_ppch_per_mf52;

          // Normal and Extended paging position
          pep_blk_position =
          PACKET_PG_POSITION[(((pg_block_index + 3) % l1pa_l1ps_com.pccch.nb_ppch_per_mf52) +
                              (l1pa_l1ps_com.pccch.bs_pag_blks_res +
                               l1pa_l1ps_com.pccch.bs_pbcch_blks)*11)];

          // Computation of the PEPCH block position in the MF52
          pep_position = (pep_blk_position - 1) + mf52_for_ms_epg * 52;

          // PEP is scheduled for the 1st time. If first PPCH block is a PEP and not PNP
          // then PEP scheduling has to be computed with "m_for_pep = m_index - 1"
          if(l1a_l1s_com.task_param[PEP] == SEMAPHORE_SET)
          {
            // PAGING GROUP computation:

            paging_group = (l1pa_l1ps_com.pccch.pg_offset + (((UWORD32)(m_for_pep) * (UWORD32)l1pa_l1ps_com.pccch.pg_blks_avail)
                                / (UWORD32)l1pa_l1ps_com.pccch.split_pg_value)) % l1pa_l1ps_com.pccch.pg_blks_avail;

            // Computation of the MF52 for MS Packet Extented Paging
            mf52_for_ms_epg = ((paging_group + 3) / l1pa_l1ps_com.pccch.nb_ppch_per_mf52) % 64;

            // Paging Block Index computation
            pg_block_index = paging_group % l1pa_l1ps_com.pccch.nb_ppch_per_mf52;

            // Normal and Extended paging position
            pep_blk_position =
            PACKET_PG_POSITION[(((pg_block_index + 3) % l1pa_l1ps_com.pccch.nb_ppch_per_mf52) +
                                (l1pa_l1ps_com.pccch.bs_pag_blks_res +
                                 l1pa_l1ps_com.pccch.bs_pbcch_blks)*11)];

            // Computation of the PEPCH block position in the MF52
            pep_position_2 = (pep_blk_position - 1) + mf52_for_ms_epg * 52;
          }

          //Step in PPCH reading block state machine
          l1pa_l1ps_com.pccch.epg_computation = PPCH_POS_COMP;
        }

        // Time between actual fn and next PEPCH block
        time_to_pep = (pep_position - fn_mod + (64*52)) % (64*52);

        if(l1a_l1s_com.task_param[PEP] == SEMAPHORE_SET)
        {
          // Time between actual fn and next PEPCH block
          time_to_pep_2 = (pep_position_2 - fn_mod + (64*52)) % (64*52);

          // Compute Position of First PEP block and update pep_position
          if(time_to_pep_2 < time_to_pep)
          {
            time_to_pep = time_to_pep_2;
            pep_position = pep_position_2;
          }

          // Semaphore is Set, reset it when PEP inactive.
          if(l1s.task_status[PEP].current_status == INACTIVE)
            l1a_l1s_com.task_param[PEP] = SEMAPHORE_RESET;
        }

        if(l1a_l1s_com.task_param[PEP] == SEMAPHORE_RESET)
        {
          // Save scheduling result.
          l1s.task_status[PEP].time_to_exec = time_to_pep;
        }

      }// End of if(l1a_l1s_com.l1s_en_task[PEP] == TASK_ENABLED)

  } // End of if PNP_TASK || PEP_TASK

  if(l1a_l1s_com.l1s_en_task[PALLC] == TASK_ENABLED)
  //---------------------------------
  // ALL PCCCH reading is ENABLED.
  //---------------------------------
  {
    // Semaphore is Set, reset it when PALLC inactive.
    if(l1a_l1s_com.task_param[PALLC] == SEMAPHORE_SET)
    {
      if(l1s.task_status[PALLC].current_status == INACTIVE)
        l1a_l1s_com.task_param[PALLC] = SEMAPHORE_RESET;
    }

    if(l1a_l1s_com.task_param[PALLC] == SEMAPHORE_RESET)
    {
      #define PCCCH0_START_TIME  13 - 1  // PCCCH block 0.
      #define PCCCH1_START_TIME   4 - 1  // PCCCH block 1.
      #define PCCCH2_START_TIME   8 - 1  // PCCCH block 2.

      UWORD32  min_time_to_pallc;

      if(l1s.actual_time.fn_mod13 <= PCCCH1_START_TIME)
        min_time_to_pallc = PCCCH1_START_TIME - l1s.actual_time.fn_mod13;
      else
      if(l1s.actual_time.fn_mod13 <= PCCCH2_START_TIME)
        min_time_to_pallc = PCCCH2_START_TIME - l1s.actual_time.fn_mod13;
      else
        min_time_to_pallc = PCCCH0_START_TIME - l1s.actual_time.fn_mod13;

      // Save scheduling result.
      l1s.task_status[PALLC].time_to_exec = min_time_to_pallc;
    }
  } // End of if(l1a_l1s_com.l1s_en_task[PALLC] == TASK_ENABLED)

  if(l1a_l1s_com.l1s_en_task[PBCCHS] == TASK_ENABLED)
  //--------------------------------------------
  // Serving Cell PCCCH reading task is ENABLED.
  //--------------------------------------------
  {
    if(l1s.task_status[PBCCHS].current_status == INACTIVE)
    {
      #define  PbcchS  l1pa_l1ps_com.pbcchs

      UWORD32  fn;
      UWORD32  min_time_to_pbcchs = MAX_FN;
      UWORD8   psi_index = 0;
      UWORD8   fn_sub_period = 0;

      if(PbcchS.control_offset)
      // 1) PBCCHS timeslot is lower than current PCCCH timeslot, this means that the scheduling
      //    must be anticipated by 1 frame
      // 2) PBCCHS task must contain a synchro change (sync on "current TS" + 4)
      //    and a synchro back to current TS.
      //    => "offset" array contains the frame to which "next_time.fn" must comply.
      {
        fn = l1s.next_time.fn;
      }
      else
      {
        fn = l1s.actual_time.fn;
      }

      // Scheduling...
      {
        UWORD16  psi_period         = PbcchS.pbcch_period;
        UWORD16  fn_mod_ps1_period  = fn % PbcchS.pbcch_period;
        UWORD8   i;

        if(PbcchS.read_all_psi)
        {
          psi_period        = 52;
          fn_sub_period     = fn_mod_ps1_period / psi_period;
          fn_mod_ps1_period = fn % psi_period;
        }

        // Look for the closest PBCCH block (loop on 20 max)
        for(i=0;i<PbcchS.nbr_psi;i++)
        {
          WORD16 time_to_pbcchs;

          // Get time diff between current frame and this PBCCHS block.
          time_to_pbcchs = PbcchS.offset_array[i] - fn_mod_ps1_period;
          if(time_to_pbcchs < 0)
            time_to_pbcchs += psi_period;

          // Save Min time to next PBCCHS block.
          if(time_to_pbcchs < min_time_to_pbcchs)
          {
            min_time_to_pbcchs = time_to_pbcchs;
            psi_index          = i;
          }
        }
      }

      // Save scheduling result.
      l1s.task_status[PBCCHS].time_to_exec = min_time_to_pbcchs;

      // Clear param. synchro. semaphore.
      l1a_l1s_com.task_param[PBCCHS] = SEMAPHORE_RESET;

      // If block to decode is B0 (decoding is made in next MF52) then increment fn_sub_period
      // This is only applicable to the specific case: All PSI to read
      if(PbcchS.read_all_psi)
      {
        if(psi_index == 0)
        {
          fn_sub_period +=1;

          if(fn_sub_period >= PbcchS.psi1_repeat_period)
            fn_sub_period -= PbcchS.psi1_repeat_period;
        }
      }

      // Save Relative Position of the PBCCH block read to L3
      l1pa_l1ps_com.pbcchs.rel_pos_to_report = PbcchS.relative_position_array[psi_index]
                                               + (fn_sub_period * (PbcchS.bs_pbcch_blks + 1));
    }
  }

  if((l1a_l1s_com.l1s_en_task[PBCCHN_TRAN] == TASK_ENABLED) ||
     (l1a_l1s_com.l1s_en_task[PBCCHN_IDLE] == TASK_ENABLED))
  //--------------------------------------------
  // Neighbor Cell PCCCH reading task is ENABLED.
  //--------------------------------------------
  {
    #define  PbcchN  l1pa_l1ps_com.pbcchn

    WORD32   min_time_to_pbcchn;
    UWORD32  neighbor_fn;
    UWORD16  neighbor_fn_mod_ps1_period;
    WORD16   task;

    // in case of packet transfer mode there are no measurement windows
    if (l1a_l1s_com.l1s_en_task[PBCCHN_IDLE] == TASK_ENABLED)
      task=PBCCHN_IDLE;
    else
      task=PBCCHN_TRAN;

    // ================================
    // compute the current neighbor FN
    // ================================

    neighbor_fn = l1s.actual_time.fn + PbcchN.fn_offset ;
    neighbor_fn_mod_ps1_period  = neighbor_fn % PbcchN.pbcch_period;

    // Get time diff between current neighbor frame and this PBCCHN block.
    min_time_to_pbcchn = PbcchN.offset- neighbor_fn_mod_ps1_period;
    if (min_time_to_pbcchn < 0)
      min_time_to_pbcchn += PbcchN.pbcch_period;

    if(l1s.task_status[task].current_status == INACTIVE)
    {
      // Clear param. synchro. semaphore.
      l1a_l1s_com.task_param[task] = SEMAPHORE_RESET;
    }

    // Save scheduling result.
    l1s.task_status[task].time_to_exec = min_time_to_pbcchn;
  }

#endif // if L1_GPRS

  if ((l1a_l1s_com.l1s_en_task[NSYNC] == TASK_ENABLED)
    && (l1a_l1s_com.l1s_en_task[ALLC] == TASK_DISABLED))
  // Do not allow NSYNC task to enter MFTAB in reorg paging mode

  //------------------------------------
  // Neigbour Cell Synchro task enabled
  //------------------------------------
  {
    if((l1s.task_status[FBNEW].current_status  == INACTIVE) &&
       (l1s.task_status[SB2].current_status    == INACTIVE) &&
       (l1s.task_status[SBCONF].current_status == INACTIVE))
    {
      //Remarks:
      //--------
      // We do not allow entrance of a neigh FB or SB task if there
      // is already some neigh task (FB/SB or BCCH) running. This is
      // to avoid to cope with many abort cases (we cannot list them
      // easily).
#if (L1_12NEIGH == 1)
      // This machine does not work for DEDICATED MODE.
       #if (L1_GPRS)
       if ((l1a_l1s_com.mode != DEDIC_MODE) && (l1a_l1s_com.mode != PACKET_TRANSFER_MODE))
       // Due to transition modes sometimes mode is PACKET_TRANSFER and
       // PDTCH is DISABLED. So we must test also PACKET_TRANSFER mode.
       #else
       if (l1a_l1s_com.mode != DEDIC_MODE)
       #endif
       {
#endif

      UWORD8  i;
      UWORD8  j;
      UWORD8  first_in_list=l1a_l1s_com.nsync.first_in_list ;

      WORD32  best_time_to_fb     = MAX_FN;
      WORD32  best_time_to_sbconf = MAX_FN;
      WORD32  best_time_to_sb2    = MAX_FN;
      UWORD8  best_neigh_fb       = 255;
      UWORD8  best_neigh_sbconf   = 255;
      UWORD8  best_neigh_sb2      = 255;

#if (L1_12NEIGH ==1)
  #if (L1_EOTD == 1)
      // Up to NBR_NEIGHBOURS + 1 (Serving Cell pending)
      for(j=first_in_list;j<(NBR_NEIGHBOURS+1+first_in_list);j++)
      {
         if (j>=NBR_NEIGHBOURS+1)
            i=j-NBR_NEIGHBOURS-1;
         else
            i=j;
  #else
      // Up to NBR_NEIGHBOURS
      for(j=first_in_list;j<(NBR_NEIGHBOURS+first_in_list);j++)
      {
         if (j>=NBR_NEIGHBOURS)
            i=j-NBR_NEIGHBOURS;
         else
            i=j;
  #endif // L1_EOTD

#else // L1_12NEIGH

      for(j=first_in_list;j<(6+first_in_list);j++)
      {
         if (j>=6)
            i=j-6;
         else
            i=j;

#endif // L1_12NEIGH

        // Consider only the "in use" locations from the "N neigh. list".
        if(l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING)
        {
          UWORD32  neigh_fn_mod51;

          switch(l1a_l1s_com.nsync.list[i].timing_validity)
          {
            case 0: // No valid info is supplied: search FB with no a priori info.
            {
              // Consider the Neighbour in respecting their order in the list.
              if(best_time_to_fb != 0)
              {
                best_time_to_fb = 0;
                best_neigh_fb   = i;
              }
            }
            break;

            case 1:  // Approximate timing is supplied: search FB with a priori info..
            {
                UWORD32  min_time_to_fb = MAX_FN;

                // Get neighbour cell FN % 51.
                neigh_fn_mod51 = (l1s.actual_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                // Attempt to read 2nd FB.
                // Frame 7 is the position to read 2nd FB within MF51 (due to C/W/R and AGC)
                if(neigh_fn_mod51 <= 7)
                {
                  min_time_to_fb = 7 - neigh_fn_mod51;
                }

                // Attempt to read 3rd FB.
                // Frame 17 is the position to read 3rd FB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 17)
                {
                  min_time_to_fb = 17 - neigh_fn_mod51;
                }

                // Attempt to read 4th FB.
                // Frame 27 is the position to read 4th FB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 27)
                {
                  min_time_to_fb = 27 - neigh_fn_mod51;
                }

                // Attempt to read 5th FB.
                // Frame 37 is the position to read 5th FB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 37)
                {
                  min_time_to_fb = 37 - neigh_fn_mod51;
                }

                // Attempt to read 1st FB.
                // Frame 48 is the position to read 1st FB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 48)
                {
                  min_time_to_fb = 48 - neigh_fn_mod51;
                }
                // Attempt to read 2nd FB
                // Special case: frame 49 and frame 50
                else
                {
                  min_time_to_fb = 50 - neigh_fn_mod51 + 8;
                }

              if(min_time_to_fb <(UWORD32 ) best_time_to_fb)
                {
                best_time_to_fb = min_time_to_fb;
                best_neigh_fb   = i;
              }
            }
            break;

            case 2:  // Accurate timing is supplied: confirm SB.
              {
                UWORD32  min_time_to_sb = MAX_FN;

                // Get neighbour cell FN % 51.
                neigh_fn_mod51 = (l1s.actual_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                // Attempt to read 2nd SB.
                // Frame 8 is the position to read 2nd SB within MF51 (due to C/W/R and AGC)
                if(neigh_fn_mod51 <= 8)
                {
                  min_time_to_sb = 8 - neigh_fn_mod51;
                }

                // Attempt to read 3rd SB.
                // Frame 18 is the position to read 3rd SB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 18)
                {
                  min_time_to_sb = 18 - neigh_fn_mod51;
                }

                // Attempt to read 4th SB.
                // Frame 28 is the position to read 4th SB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 28)
                {
                  min_time_to_sb = 28 - neigh_fn_mod51;
                }

                // Attempt to read 5th SB.
                // Frame 38 is the position to read 5th SB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 38)
                {
                  min_time_to_sb = 38 - neigh_fn_mod51;
                }

                // Attempt to read 1st SB.
                // Frame 49 is the position to read 1st SB within MF51 (due to C/W/R and AGC)
                else
                if(neigh_fn_mod51 <= 49)
                {
                  min_time_to_sb = 49 - neigh_fn_mod51;
                }
                // Attempt to read 2nd SB
                // Special case: Frame=50 (idle frame)
                else
                {
                  min_time_to_sb = 9;
                }

              if(min_time_to_sb <(UWORD32 ) best_time_to_sbconf)
                {
                best_time_to_sbconf = min_time_to_sb;
                best_neigh_sbconf   = i;
                }

            }
            break;

         #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
            case SB_ACQUISITION_PHASE: // SB search following a FB search.
          #else
            case 3:  // SB search following a FB search.
          #endif
            {
                UWORD8   spent_fn;
                UWORD32  min_time_to_sb = MAX_FN;

                // "fn_offset" contains the FN%51 corresponding to the FB detected

                spent_fn = (l1s.actual_time.t3 - l1a_l1s_com.nsync.list[i].fn_offset + 51) % 51;

                // Attempt to read 1st SB.
                // Frame 8 is the position to read 1st SB within MF51 (due to C/W/R and AGC)
                if(spent_fn <= 8)
                {
                  min_time_to_sb = 8 - spent_fn;
                }

                // Attempt to read 2nd SB.
                // Frame 18 is the position to read 2nd SB within MF51 (due to C/W/R and AGC)
                else
                if(spent_fn <= 18)
                {
                  min_time_to_sb = 18 - spent_fn;
                }

                // Attempt to read 3rd SB.
                // Frame 28 is the position to read 3rd SB within MF51 (due to C/W/R and AGC)
                else
                if(spent_fn <= 28)
                {
                  min_time_to_sb = 28 - spent_fn;
                }

                // Attempt to read 4th SB.
                // Frame 38 is the position to read 4th SB within MF51 (due to C/W/R and AGC)
                else
                if(spent_fn <= 38)
                {
                  min_time_to_sb = 38 - spent_fn;
                }

                // Attempt to read 5tht SB.
                // Frame 49 is the position to read 5tht SB within MF51 (due to C/W/R and AGC)
                else
                if(spent_fn <= 49)
                {
                  min_time_to_sb = 49 - spent_fn;
                }
                // Attempt to read 2nd SB
                // Special case: Frame=50 (idle frame)
                else
                {
                  min_time_to_sb = 9;
                }

              if(min_time_to_sb <= ( UWORD32 )best_time_to_sb2)
                {
                best_time_to_sb2 = min_time_to_sb;
                best_neigh_sb2   = i;
                }

            }
            break;

          } // End of "switch"
        } // End of "if"
      } // End of "for"

      if(best_neigh_fb != 255)
      {
        // Save active neighbour id.
        l1a_l1s_com.nsync.active_fb_id = best_neigh_fb;

          // Save scheduling result.
        l1s.task_status[FBNEW].time_to_exec = best_time_to_fb;

          // Enable FBNEW task
          l1a_l1s_com.l1s_en_task[FBNEW] = TASK_ENABLED;

          // Clear param. synchro. semaphore.
          l1a_l1s_com.task_param[FBNEW] = SEMAPHORE_RESET;
        }

      if(best_neigh_sbconf != 255)
        {
        // Save active neighbour id.
        l1a_l1s_com.nsync.active_sbconf_id = best_neigh_sbconf;

          // Save scheduling result.
        l1s.task_status[SBCONF].time_to_exec = best_time_to_sbconf;

          l1a_l1s_com.l1s_en_task[SBCONF] = TASK_ENABLED;

          // Clear param. synchro. semaphore.
          l1a_l1s_com.task_param[SBCONF] = SEMAPHORE_RESET;
        }

      if(best_neigh_sb2 != 255)
        {
        // Save active neighbour id.
        l1a_l1s_com.nsync.active_sb_id = best_neigh_sb2;

          // Save scheduling result.
        l1s.task_status[SB2].time_to_exec = best_time_to_sb2;

          // Enable SB2 task
          l1a_l1s_com.l1s_en_task[SB2] = TASK_ENABLED;

          // Clear param. synchro. semaphore.
          l1a_l1s_com.task_param[SB2] = SEMAPHORE_RESET;
        }
#if (L1_12NEIGH == 1)
    } // End of "if / DEDIC = IDLE-MODE
#endif
    } // End of "if / current_status"
  } // End of "if / NSYNC"

  if(l1a_l1s_com.l1s_en_task[HWTEST] == TASK_ENABLED)
  //-------------------------
  // HW_TEST task is ENABLED.
  //-------------------------
  {
    if(l1s.task_status[HWTEST].current_status == INACTIVE)
    // HW_TEST task is INACTIVE.
    // Rem: HW_TEST task has no occurence time. Therefore whenever HW_TEST
    //      is ENABLED and INACTIVE it must be executed.
    {
      // Save scheduling result.
      l1s.task_status[HWTEST].time_to_exec = 0;
    }
  }

  //================================================================
  //==                   DEDICATED MODE TASKS                     ==
  //==                   --------------------                     ==
  //================================================================
#if (L1_12NEIGH == 0)
  if(l1a_l1s_com.l1s_en_task[FB26] == TASK_ENABLED)
  //-------------------------
  // FB26 task is ENABLED.
  //-------------------------
  {
    UWORD32  time_to_fb26 = MAX_FN;

    if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
    //---------------------------------------------
    // Dedicated mode tasks are enabled.
    //---------------------------------------------
    {
      T_CHANNEL_DESCRIPTION *desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

      // Rem: this task is used in dedicated/TCH_H/F

      if(desc_ptr->channel_type == TCH_H)
      {
        UWORD8 normalised_fn_mod26 = (l1s.actual_time.fn_in_report - desc_ptr->subchannel ) % 26;

        if(normalised_fn_mod26 <= 22)
          time_to_fb26 = 22 - normalised_fn_mod26;
        else
          time_to_fb26 = 26+ 22 - normalised_fn_mod26;
      }
      else
      if(desc_ptr->channel_type == TCH_F)
      {
        UWORD8 normalised_fn_mod26 = l1s.actual_time.fn_in_report % 26;

        if(normalised_fn_mod26 <= 23)
          time_to_fb26 = 23 - normalised_fn_mod26;
        else
          time_to_fb26 = 26 + 23 - normalised_fn_mod26;
      }
    }

    #if L1_GPRS
    else
    if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
    //---------------------------------------------
    // Packet Transfer mode task is enabled.
    //---------------------------------------------
    {
      if(l1s.actual_time.t2 <= 23)
        time_to_fb26 = 23 - l1s.actual_time.t2;
      else
        time_to_fb26 = 26 + 23 - l1s.actual_time.t2;
    }
    #endif

    // No scheduling result for FB26 task to avoid selection of this task.
    // -> l1s.task_status[FB26].time_to_exec stays equal to MAX_FN

    // Set FB26 as PENDING to trigger the l1s_merge_manager() to install it
    // in MFTAB.
    if(time_to_fb26 == 0)
      l1s.task_status[FB26].new_status = PENDING;

    // Set active neighbor identifier.
    l1a_l1s_com.nsync.active_fb_id = 0;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[FB26] = SEMAPHORE_RESET;

  } // End if(...[FB26] == TASK_ENABLED)

  if(l1a_l1s_com.l1s_en_task[SB26] == TASK_ENABLED)
  //-------------------------
  // SB26 task is ENABLED.
  //-------------------------
  {
    WORD32  time_to_sb26 = MAX_FN;

    if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
    //---------------------------------------------
    // Dedicated mode tasks are enabled.
    //---------------------------------------------
    {
      T_CHANNEL_DESCRIPTION *desc_ptr   = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

      // Rem: this task is used in dedicated/TCH_H/F

      if((desc_ptr->channel_type == TCH_F) || (desc_ptr->channel_type == TCH_H))
      {
        // Monitoring area starting frame is reached...
        if(l1s.actual_time.fn <= l1a_l1s_com.nsync.list[0].fn_offset)
          time_to_sb26 = l1a_l1s_com.nsync.list[0].fn_offset - l1s.actual_time.fn;
        else
          time_to_sb26 = MAX_FN + l1a_l1s_com.nsync.list[0].fn_offset - l1s.actual_time.fn;
      }
    }

    #if L1_GPRS
    else
    if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
    //---------------------------------------------
    // Packet Transfer mode task is enabled.
    //---------------------------------------------
    {

      UWORD8 SB26_attempt = l1a_l1s_com.nsync.list[0].sb26_attempt;
      // schedule SB26 at all possible positions
      // attempt=0 => first SB after FB found is at FB position + 52
      // attempt=1 => next SB is at position: FN_offset + 9*26
      // attempt=2 => or next SB is at position: FN_offset + 9*26 + 52 (due to Idle frame)
      // REM: an higher priority task could have canceled the SB scheduled, that's why
      // we must plan the  following SB with its 2 possible positions.

      // Monitoring area starting frame is reached...
        time_to_sb26 = l1a_l1s_com.nsync.list[0].fn_offset - l1s.actual_time.fn;

      if(time_to_sb26 < 0) // in this case a SB was missed by a higher priority task
      {
         if (SB26_attempt == 1) // next SB is 9*26 frame later
            time_to_sb26 += 26*9;
         else if (SB26_attempt == 2) // or next FB is 9*26 + 52 frames later
            time_to_sb26 += 26*9 + 52;
              else // should never happen
                time_to_sb26 += MAX_FN;
      }

      if(time_to_sb26 == 0)
      {
        l1a_l1s_com.nsync.list[0].sb26_attempt++;

        if (SB26_attempt>2)
          l1a_l1s_com.nsync.list[0].sb26_attempt=1;
      }

    }
    #endif

    // No scheduling result for SB26 task to avoid selection of this task.
    // -> l1s.task_status[SB26].time_to_exec stays equal to MAX_FN

    // Set SB26 as PENDING to trigger the l1s_merge_manager() to install it
    // in MFTAB.
    if(time_to_sb26 == 0)
      l1s.task_status[SB26].new_status = PENDING;

    // Set active neighbor identifier.
    l1a_l1s_com.nsync.active_sb_id = 0;

    // Clear param. synchro. semaphore.
    l1a_l1s_com.task_param[SB26] = SEMAPHORE_RESET;

  } // End if(...[SB26] == TASK_ENABLED)

  if(l1a_l1s_com.l1s_en_task[SBCNF26] == TASK_ENABLED)
  //-------------------------
  // SBCNF26 task is ENABLED.
  //-------------------------
  {
    BOOL check_sbcnf26 = 0;

    if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
    //---------------------------------------------
    // Dedicated mode tasks are enabled.
    //---------------------------------------------
    {
      T_CHANNEL_DESCRIPTION *desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;
      UWORD8                 normalised_fn_mod26;

      normalised_fn_mod26 = (l1s.actual_time.fn_in_report - desc_ptr->subchannel ) % 26;

      if(((normalised_fn_mod26 == 22) && (desc_ptr->channel_type == TCH_H)) ||
        (((l1s.actual_time.fn_in_report % 26) == 23) && (desc_ptr->channel_type == TCH_F)))
      // Monitoring area control frame is reached...
      {
        check_sbcnf26 = 1;
      }
    }

    #if L1_GPRS
    else
    if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
    //---------------------------------------------
    // Packet Transfer mode task is enabled.
    //---------------------------------------------
    {
      if(l1s.actual_time.t2 == 23)
      // Monitoring area control frame is reached...
      {
        check_sbcnf26 = 1;
      }
    }
    #endif

    if(check_sbcnf26)
    // SBCNF26 must be checked.
    {
      UWORD32 next_neigh_fn_mod51;

      // Get neighbor domain frame number %51 for frame 24.
      next_neigh_fn_mod51 = (l1s.next_time.fn + l1a_l1s_com.nsync.list[0].fn_offset) % 51;

      // No scheduling result for SBCNF26 task to avoid selection of this task.
      // -> l1s.task_status[SBCNF26].time_to_exec stays equal to MAX_FN

      if((next_neigh_fn_mod51 % 10) == 1)
      // SB26 rx window starts in the frame 24...
      {
        if(l1a_l1s_com.nsync.list[0].time_alignmt >= l1_config.params.fb26_anchoring_time)
        // SB feet in the search slot...
        // Set SBCNF26 as PENDING to trigger the l1s_merge_manager() to install it
        // in MFTAB.
        {
          l1s.task_status[SBCNF26].new_status = PENDING;

          l1a_l1s_com.nsync.list[0].sb26_offset   = 0;

          // Set active neighbor identifier.
          l1a_l1s_com.nsync.active_sbconf_id  = 0;

          // Clear param. synchro. semaphore.
          l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_RESET;
        }
      }
      else
      if((next_neigh_fn_mod51 == 0)  ||
         (next_neigh_fn_mod51 == 10) ||
         (next_neigh_fn_mod51 == 20) ||
         (next_neigh_fn_mod51 == 30) ||
         (next_neigh_fn_mod51 == 40))
      // SB26 rx window starts in the frame 25...
      {
        if(l1a_l1s_com.nsync.list[0].time_alignmt < ((l1_config.params.fb26_anchoring_time
                                                      +FB26_ACQUIS_DURATION
                                                      -SB_ACQUIS_DURATION
                                                      +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE))
        // SB feet in the search slot...
        // Set SBCNF26 as PENDING to trigger the l1s_merge_manager() to install it
        // in MFTAB.
        {
          l1s.task_status[SBCNF26].new_status = PENDING;

          l1a_l1s_com.nsync.list[0].sb26_offset   = 1;

          // Set active neighbor identifier.
          l1a_l1s_com.nsync.active_sbconf_id  = 0;

          // Clear param. synchro. semaphore.
          l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_RESET;
        }
      }
    }
  }


  if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
  //---------------------------------------------
  // Dedicated mode tasks are enabled.
  //---------------------------------------------
  {
    T_CHANNEL_DESCRIPTION *desc_ptr   = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;
    T_SDCCH_DESC          *sdcch_desc = NULL;//OMAPS00090550

    // No scheduling result for DEDIC task to avoid selection of this task.
    // -> l1s.task_status[DEDIC].time_to_exec stays equal to MAX_FN

    // DEDIC task requires to run L1S scheduler every frame.
    l1a_l1s_com.time_to_next_l1s_task = 0;

    //=================================
    // MF51 tasks...
    //=================================

    if((desc_ptr->channel_type == SDCCH_4) || (desc_ptr->channel_type == SDCCH_8))
    // SDCCH channel -> MF51 structure.
    {
      UWORD8  start_time =0; //omaps00090550
      UWORD8  fn_in_report_mod51 = l1s.next_time.fn_in_report % 51;

      // Save position of the monitoring area for SDCCH.
      if(desc_ptr->channel_type == SDCCH_8)
      {
        start_time =  SDCCH_DESC_NCOMB[desc_ptr->subchannel].mon_area_position;
        sdcch_desc = &(SDCCH_DESC_NCOMB[desc_ptr->subchannel]);
      }
      else
      {
        start_time =  SDCCH_DESC_COMB[desc_ptr->subchannel].mon_area_position;
        sdcch_desc = &(SDCCH_DESC_COMB[desc_ptr->subchannel]);
      }

      if(l1a_l1s_com.l1s_en_task[FB51] == TASK_ENABLED)
      //-------------------------
      // FB51 task is ENABLED.
      //-------------------------
      {
        UWORD32  time_to_fb51;

        if(l1s.actual_time.fn_in_report <= start_time)
          time_to_fb51 = start_time - l1s.actual_time.fn_in_report;
        else
          time_to_fb51 = MAX_FN + start_time - l1s.actual_time.fn_in_report;

        // Save scheduling result.
        l1s.task_status[FB51].time_to_exec = time_to_fb51;

        // Set active neighbor identifier.
        l1a_l1s_com.nsync.active_fb_id = 0;

        // Clear param. synchro. semaphore.
        l1a_l1s_com.task_param[FB51] = SEMAPHORE_RESET;
      }

      if(l1a_l1s_com.l1s_en_task[SB51] == TASK_ENABLED)
      //-------------------------
      // SB51 task is ENABLED.
      //-------------------------
      {
        // Rem: this task is used in dedicated/SDCCH.
        //      SB reading task is 102 frames after the FB.

        UWORD32  time_to_sb51;
        UWORD32  fn_sb51 = (l1a_l1s_com.nsync.list[0].fn_offset + 102)%MAX_FN;

        fn_sb51 = l1a_l1s_com.nsync.list[0].fn_offset + 102;
        if(fn_sb51 >= MAX_FN) fn_sb51 -= MAX_FN;

        if(l1s.actual_time.fn <= fn_sb51)
          time_to_sb51 = fn_sb51 - l1s.actual_time.fn;
        else
          time_to_sb51 = MAX_FN + fn_sb51 - l1s.actual_time.fn;

        // Save scheduling result.
        l1s.task_status[SB51].time_to_exec = time_to_sb51;

        // Set active neighbor identifier.
        l1a_l1s_com.nsync.active_sb_id = 0;

        // Clear param. synchro. semaphore.
        l1a_l1s_com.task_param[SB51] = SEMAPHORE_RESET;
      }

      if(l1a_l1s_com.l1s_en_task[SBCNF51] == TASK_ENABLED)
      //-------------------------
      // SBCNF51 task is ENABLED.
      //-------------------------
      {
        // Rem: this task is used in dedicated/SDCCH.

        if((l1s.actual_time.fn_in_report >= start_time) &&
           (l1s.actual_time.fn_in_report <  start_time + 11))
        {
          UWORD32  neigh_fn_mod51;

          // Get neighbour cell FN % 51 and then % 10.
          neigh_fn_mod51 = (l1s.actual_time.fn + l1a_l1s_com.nsync.list[0].fn_offset) % 51;

          // SBCNF51 task is a special case: it is not using a predictive scheduling.
          // Thi scheduling would be too complex.

          if((neigh_fn_mod51 == 0)  ||
             (neigh_fn_mod51 == 10) ||
             (neigh_fn_mod51 == 20) ||
             (neigh_fn_mod51 == 30) ||
             (neigh_fn_mod51 == 40))
          // It is time to start SBCNF51 task.
          {
            // SBCNF51 must be started immediately.
            l1s.task_status[SBCNF51].time_to_exec = 0;

            // Set active neighbor identifier.
            l1a_l1s_com.nsync.active_sbconf_id = 0;

            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_RESET;
          }
          else
          {
            // SBCNF51 is not started.
            l1s.task_status[SBCNF51].time_to_exec = MAX_FN;
          }
        }
      }

      if((l1a_l1s_com.l1s_en_task[DDL] == TASK_ENABLED) && (sdcch_desc != NULL))//OMAPS00090550
      //-------------------------
      // SDCCH DL task is ENABLED.
      //-------------------------
      {
        UWORD32 time_to_ddl;

        if(fn_in_report_mod51 <= sdcch_desc->dl_sdcch_position)
          time_to_ddl = sdcch_desc->dl_sdcch_position - fn_in_report_mod51;
        else
          time_to_ddl = 51 + sdcch_desc->dl_sdcch_position - fn_in_report_mod51;

        // Save scheduling result.
        l1s.task_status[DDL].time_to_exec = time_to_ddl;
      }

      if((l1a_l1s_com.l1s_en_task[DUL] == TASK_ENABLED) && (sdcch_desc != NULL))//OMAPS00090550
      //-------------------------
      // SDCCH DUL task is ENABLED.
      //-------------------------
      {
        UWORD32 time_to_dul;

        if(fn_in_report_mod51 <= sdcch_desc->ul_sdcch_position)
          time_to_dul = sdcch_desc->ul_sdcch_position - fn_in_report_mod51;
        else
          time_to_dul = 51 + sdcch_desc->ul_sdcch_position - fn_in_report_mod51;

        // Save scheduling result.
        l1s.task_status[DUL].time_to_exec = time_to_dul;
      }

      if((l1a_l1s_com.l1s_en_task[ADL] == TASK_ENABLED) && (sdcch_desc != NULL))//OMAPS00090550
      //----------------------------------
      // SACCH DL (SDCCH) task is ENABLED.
      //----------------------------------
      {
        UWORD32 time_to_adl;

        if(l1s.next_time.fn_in_report <= sdcch_desc->dl_sacch_position)
          time_to_adl = sdcch_desc->dl_sacch_position - l1s.next_time.fn_in_report;
        else
          time_to_adl = 102 + sdcch_desc->dl_sacch_position - l1s.next_time.fn_in_report;

        // Save scheduling result.
        l1s.task_status[ADL].time_to_exec = time_to_adl;
      }

      if((l1a_l1s_com.l1s_en_task[AUL] == TASK_ENABLED) && (sdcch_desc != NULL))//OMAPS00090550
      //----------------------------------
      // SACCH UL (SDCCH) task is ENABLED.
      //----------------------------------
      {
        UWORD32 time_to_aul;

        if(l1s.next_time.fn_in_report <= sdcch_desc->ul_sacch_position)
          time_to_aul = sdcch_desc->ul_sacch_position - l1s.next_time.fn_in_report;
        else
          time_to_aul = MAX_FN + sdcch_desc->ul_sacch_position - l1s.next_time.fn_in_report;

        // Save scheduling result.
        l1s.task_status[AUL].time_to_exec = time_to_aul;
      }
    } // channel_type == SDCCH_4 or SDCCH_8
#endif

//================================================================
//==                   DEDICATED MODE TASKS                     ==
//==                   --------------------                     ==
//==    NEW NCELL MONITORING SCHEDULER for 12 Neighbors         ==
//================================================================
#if (L1_12NEIGH == 1)
  #if (L1_GPRS)
  // This machine works only for DEDICATED MODE and PACKET TRANSFER MODE
  // (Remark: PACKET TRANSFER MODE activated == PDTCH task activated)
  if((l1a_l1s_com.mode == DEDIC_MODE) || (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED))
  #else
  // This machine works only for DEDICATED MODE.
  if (l1a_l1s_com.mode == DEDIC_MODE)
  #endif
  {
    #define MF51       1
    #define MF26       2

    UWORD8                channel = 255;
    UWORD8                start_time;
    UWORD8                fn_in_report_mod51 = l1s.next_time.fn_in_report % 51;
    T_CHANNEL_DESCRIPTION *desc_ptr;
    T_SDCCH_DESC          *sdcch_desc;

    if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
    //---------------------------------------------
    // Dedicated mode tasks are enabled.
    //---------------------------------------------
    {
      // init desc_ptr
      desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;

      // No scheduling result for DEDIC task to avoid selection of this task.
      // -> l1s.task_status[DEDIC].time_to_exec stays equal to MAX_FN

      // DEDIC task requires to run L1S scheduler every frame
      l1a_l1s_com.time_to_next_l1s_task = 0;

      // Rem: this task is used in dedicated/TCH_H/F

      // MF51 tasks...
      //==============
      if((desc_ptr->channel_type == SDCCH_4) || (desc_ptr->channel_type == SDCCH_8))
      // SDCCH channel -> MF51 structure.
      {
        // init channel to MF51
        channel = MF51;

        // Save position of the monitoring area for SDCCH.
        if(desc_ptr->channel_type == SDCCH_8)
        {
          start_time =  SDCCH_DESC_NCOMB[desc_ptr->subchannel].mon_area_position;
          sdcch_desc = &(SDCCH_DESC_NCOMB[desc_ptr->subchannel]);
        }
        else
        {
          start_time =  SDCCH_DESC_COMB[desc_ptr->subchannel].mon_area_position;
          sdcch_desc = &(SDCCH_DESC_COMB[desc_ptr->subchannel]);
        }
      }
      // MF26 tasks...
      //==============
      else
        channel = MF26;
    } // End of "if DEDIC task"

    #if (L1_GPRS)
    else
    if (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
      channel = MF26;
    #endif

    //============================================================
    // DEDIC  MODE - MF51 - NEIGHBOUR MONITORING tasks SCHEDULING
    //============================================================
    if (channel == MF51)
    {
      if (l1a_l1s_com.l1s_en_task[NSYNC] == TASK_ENABLED)
      //--------------------------------------------
      // Neigbour Cell Synchro task enabled for MF51
      //--------------------------------------------
      {
        if((l1s.task_status[FB51].current_status    == INACTIVE) &&
           (l1s.task_status[SB51].current_status    == INACTIVE) &&
           (l1s.task_status[SBCNF51].current_status == INACTIVE))
        {
          //Remarks:
          //--------
          // We do not allow entrance of a neigh FB or SB task if there
          // is already some neigh task (FB/SB or BCCH) running. This is
          // to avoid to cope with many abort cases (we cannot list them
          // easily).

          UWORD8  i;
          UWORD8  j;
          UWORD8  first_in_list=l1a_l1s_com.nsync.first_in_list ;


          WORD32  best_time_to_fb     = MAX_FN;
          WORD32  best_time_to_sb     = MAX_FN;
          WORD32  best_time_to_sbconf = MAX_FN;
          UWORD8  best_neigh_fb       = 255;
          UWORD8  best_neigh_sbconf   = 255;
          UWORD8  best_neigh_sb       = 255;

#if (L1_EOTD == 1)
          // Up to NBR_NEIGHBOURS + 1 (Serving Cell pending)
          for(j=first_in_list;j<(NBR_NEIGHBOURS+1+first_in_list);j++)
          {
            if (j>=NBR_NEIGHBOURS+1)
              i=j-NBR_NEIGHBOURS-1;
            else
              i=j;
#else
          // Up to NBR_NEIGHBOURS
          for(j=first_in_list;j<(NBR_NEIGHBOURS+first_in_list);j++)
          {
            if (j>=NBR_NEIGHBOURS)
              i=j-NBR_NEIGHBOURS;
            else
              i=j;
#endif // L1_EOTD

            // Consider only the "in use" locations from the "12 neigh. list".
            if(l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING)
            {

              switch(l1a_l1s_com.nsync.list[i].timing_validity)
              {
                case 0: // No valid info is supplied: search FB with no a priori info.
              #if ((REL99 == 1) && (FF_RTD == 1))
                case 3: // approximate timing based on RTD is supplied
              #endif
                {
                  UWORD32  min_time_to_fb = MAX_FN;

                  if(l1s.actual_time.fn_in_report <= start_time)
                    min_time_to_fb = start_time - l1s.actual_time.fn_in_report;
                  else
                    min_time_to_fb = MAX_FN + start_time - l1s.actual_time.fn_in_report;

                  if(min_time_to_fb < (UWORD32)best_time_to_fb)
                  {
                    best_time_to_fb = min_time_to_fb;
                    best_neigh_fb = i;
                  }
                }
                break;


                case 1:  // Approximate timing is supplied: search FB with a priori info..
                {
                  UWORD32  min_time_to_fb = MAX_FN;
                  UWORD32  neigh_fn_mod51;

                  if((l1s.actual_time.fn_in_report >= start_time) &&
                     (l1s.actual_time.fn_in_report <  start_time + 11))
                  {
                    UWORD32  neigh_fn_mod51;

                    // Get neighbour cell FN % 51 and then % 10.
                    neigh_fn_mod51 = (l1s.actual_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                    // Attempt to read 2nd FB.
                    // Frame 7 is the position to read 2nd FB within MF51 (due to C/W/R and AGC)
                    if(neigh_fn_mod51 <= 7)
                    {
                      min_time_to_fb = 7 - neigh_fn_mod51;
                    }

                    // Attempt to read 3rd FB.
                    // Frame 17 is the position to read 3rd FB within MF51 (due to C/W/R and AGC)
                    else
                    if(neigh_fn_mod51 <= 17)
                    {
                      min_time_to_fb = 17 - neigh_fn_mod51;
                    }

                    // Attempt to read 4th FB.
                    // Frame 27 is the position to read 4th FB within MF51 (due to C/W/R and AGC)
                    else
                    if(neigh_fn_mod51 <= 27)
                    {
                      min_time_to_fb = 27 - neigh_fn_mod51;
                    }

                    // Attempt to read 5th FB.
                    // Frame 37 is the position to read 5th FB within MF51 (due to C/W/R and AGC)
                    else
                    if(neigh_fn_mod51 <= 37)
                    {
                      min_time_to_fb = 37 - neigh_fn_mod51;
                    }

                    // Attempt to read 1st FB.
                    // Frame 48 is the position to read 1st FB within MF51 (due to C/W/R and AGC)
                    else
                    if(neigh_fn_mod51 <= 48)
                    {
                      min_time_to_fb = 48 - neigh_fn_mod51;
                    }
                    // Attempt to read 2nd FB
                    // Special case: frame 49 and frame 50
                    else
                    {
                      min_time_to_fb = 50 - neigh_fn_mod51 + 8;
                    }

                    if(min_time_to_fb <(UWORD32) best_time_to_fb)
                    {
                      best_time_to_fb = min_time_to_fb;
                      best_neigh_fb   = i;
                    }
                  }
                }
                break;

                case 2:  // Accurate timing is supplied SBCONF: confirm SB.
                {
                  UWORD32 min_time_to_sbconf = MAX_FN;

                  if((l1s.actual_time.fn_in_report >= start_time) &&
                     (l1s.actual_time.fn_in_report <  start_time + 11))
                  {
                    UWORD32  neigh_fn_mod51;

                    // Get neighbour cell FN % 51 and then % 10.
                    neigh_fn_mod51 = (l1s.actual_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                    // SBCNF51 task is a special case: it is not using a predictive scheduling.
                    // Thi scheduling would be too complex.
                    if((neigh_fn_mod51 == 0)  ||
                       (neigh_fn_mod51 == 10) ||
                       (neigh_fn_mod51 == 20) ||
                       (neigh_fn_mod51 == 30) ||
                       (neigh_fn_mod51 == 40))
                    // It is time to start SBCNF51 task.
                    {
                      min_time_to_sbconf = 0;
                    }
                  }

                  if(min_time_to_sbconf <(UWORD32) best_time_to_sbconf)
                  {
                    best_time_to_sbconf = min_time_to_sbconf;
                    best_neigh_sbconf = i;
                  }
                }
                break;

             #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
                case SB_ACQUISITION_PHASE: // SB search following a FB search.
              #else
                case 3:  // SB search following a FB search.
              #endif
                {
                  UWORD32  min_time_to_sb = MAX_FN;

                  UWORD32  fn_sb51 = (l1a_l1s_com.nsync.list[i].fn_offset + 102)%MAX_FN;

                  fn_sb51 = l1a_l1s_com.nsync.list[i].fn_offset + 102;
                  if(fn_sb51 >= MAX_FN) fn_sb51 -= MAX_FN;

                  if(l1s.actual_time.fn <= fn_sb51)
                    min_time_to_sb = fn_sb51 - l1s.actual_time.fn;
                  else
                    min_time_to_sb = MAX_FN + fn_sb51 - l1s.actual_time.fn;

                  if(min_time_to_sb <(UWORD32) best_time_to_sb)
                  {
                    best_time_to_sb = min_time_to_sb;
                    best_neigh_sb = i;
                  }
                }
                break;
              } // End of "switch"
            } // End of "if PENDING"
          } // End of "for"

          if(best_neigh_fb != 255)  // for FB ACQUISITION or CONFIRMATION
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_fb_id = best_neigh_fb;
            // Save scheduling result.
            l1s.task_status[FB51].time_to_exec = best_time_to_fb;
            // Enable FB51 task
            l1a_l1s_com.l1s_en_task[FB51] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[FB51] = SEMAPHORE_RESET;
          }

          if(best_neigh_sbconf != 255) // for SB CONFIRMATION
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_sbconf_id = best_neigh_sbconf;
            // Save scheduling result.
            l1s.task_status[SBCNF51].time_to_exec = 0;
            // Enable SBCNF51 task
            l1a_l1s_com.l1s_en_task[SBCNF51] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_RESET;
          }

          if(best_neigh_sb != 255) // for SB ACQUISITION
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_sb_id = best_neigh_sb;
             // Save scheduling result.
            l1s.task_status[SB51].time_to_exec = best_time_to_sb;
            // Enable SB2 task
            l1a_l1s_com.l1s_en_task[SB51] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[SB51] = SEMAPHORE_RESET;
          }

        } // End of "if / current_status"
      } // End of "if / NSYNC"

      if (l1a_l1s_com.l1s_en_task[DDL] == TASK_ENABLED)
      //-------------------------
      // SDCCH DL task is ENABLED.
      //-------------------------
      {
        UWORD32 time_to_ddl;

        if(fn_in_report_mod51 <= ( UWORD8)sdcch_desc->dl_sdcch_position)
          time_to_ddl = sdcch_desc->dl_sdcch_position - fn_in_report_mod51;
        else
          time_to_ddl = 51 + sdcch_desc->dl_sdcch_position - fn_in_report_mod51;

        // Save scheduling result.
        l1s.task_status[DDL].time_to_exec = time_to_ddl;
      }

      if (l1a_l1s_com.l1s_en_task[DUL] == TASK_ENABLED)
      //-------------------------
      // SDCCH DUL task is ENABLED.
      //-------------------------
      {
        UWORD32 time_to_dul;

        if(fn_in_report_mod51 <= sdcch_desc->ul_sdcch_position)
          time_to_dul = sdcch_desc->ul_sdcch_position - fn_in_report_mod51;
        else
          time_to_dul = 51 + sdcch_desc->ul_sdcch_position - fn_in_report_mod51;

        // Save scheduling result.
        l1s.task_status[DUL].time_to_exec = time_to_dul;
      }

      if (l1a_l1s_com.l1s_en_task[ADL] == TASK_ENABLED)
      //----------------------------------
      // SACCH DL (SDCCH) task is ENABLED.
      //----------------------------------
      {
        UWORD32 time_to_adl;

        if(l1s.next_time.fn_in_report <= sdcch_desc->dl_sacch_position)
          time_to_adl = sdcch_desc->dl_sacch_position - l1s.next_time.fn_in_report;
        else
          time_to_adl = 102 + sdcch_desc->dl_sacch_position - l1s.next_time.fn_in_report;

        // Save scheduling result.
        l1s.task_status[ADL].time_to_exec = time_to_adl;
      }

      if (l1a_l1s_com.l1s_en_task[AUL] == TASK_ENABLED)
      //----------------------------------
      // SACCH UL (SDCCH) task is ENABLED.
      //----------------------------------
      {
        UWORD32 time_to_aul;

        if(l1s.next_time.fn_in_report <= sdcch_desc->ul_sacch_position)
          time_to_aul = sdcch_desc->ul_sacch_position - l1s.next_time.fn_in_report;
        else
          time_to_aul = MAX_FN + sdcch_desc->ul_sacch_position - l1s.next_time.fn_in_report;

        // Save scheduling result.
        l1s.task_status[AUL].time_to_exec = time_to_aul;
      }

    } // End of "if MF51"

    //==================================================================
    // DEDIC/PACKET MODE - MF26 - NEIGHBOUR MONITORING tasks SCHEDULING
    //==================================================================
    if (channel == MF26)
    {
      #if L1_EDA
        UWORD8 enter_nsync_task = TRUE;

        //Initialize FB/SB activity detection flag
        if((l1s.actual_time.t2 < 20) || (l1s.actual_time.t2 == 25))
        {
          l1ps_macs_com.fb_sb_task_enabled = FALSE;
        }
      #endif
      if (l1a_l1s_com.l1s_en_task[NSYNC] == TASK_ENABLED)
      //-----------------------------------------------------------
      // Neigbour Cell Synchro task enabled for DEDIC TCH or PACKET
      //-----------------------------------------------------------
      {
        T_CHANNEL_DESCRIPTION *desc_ptr;

        if (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
        {
          desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;
        }

        if((l1s.task_status[FB26].current_status    == INACTIVE) &&
           (l1s.task_status[SB26].current_status    == INACTIVE) &&
           (l1s.task_status[SBCNF26].current_status == INACTIVE))
        {
          //Remarks:
          //--------
          // We do not allow entrance of a neigh FB or SB task if there
          // is already some neigh task (FB/SB or BCCH) running. This is
          // to avoid to cope with many abort cases (we cannot list them
          // easily).
          UWORD8  i;
          UWORD8  j;
          UWORD8  first_in_list=l1a_l1s_com.nsync.first_in_list ;


          WORD32  best_time_to_fb     = MAX_FN;
          WORD32  best_time_to_sbconf = MAX_FN;
          WORD32  best_time_to_fbconf = MAX_FN;
          WORD32  best_time_to_sb     = MAX_FN;
          UWORD8  best_neigh_fb       = 255;
          UWORD8  best_neigh_sbconf   = 255;
          UWORD8  best_neigh_fbconf   = 255;
          UWORD8  best_neigh_sb       = 255;
          #if ((REL99 == 1) && (FF_BHO == 1))
            UWORD8 fb26_position ;
          #endif

#if (L1_EOTD == 1)
          // Up to NBR_NEIGHBOURS + 1 (Serving Cell pending)
          for(j=first_in_list;j<(NBR_NEIGHBOURS+1+first_in_list);j++)
          {
            if (j>=NBR_NEIGHBOURS+1)
              i=j-NBR_NEIGHBOURS-1;
            else
              i=j;
#else
          // Up to NBR_NEIGHBOURS
          for(j=first_in_list;j<(NBR_NEIGHBOURS+first_in_list);j++)
          {
            if (j>=NBR_NEIGHBOURS)
              i=j-NBR_NEIGHBOURS;
            else
              i=j;
#endif // L1_EOTD

            // Consider only the "in use" locations from the "NBR_NEIGHBOURS neigh. list".
            if(l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING)
            {

              switch(l1a_l1s_com.nsync.list[i].timing_validity)
              {
                case 0: // No valid info is supplied: search FB with no a priori info.
                {
                  UWORD32  min_time_to_fb = MAX_FN;

                  if (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
                  //---------------------------------------------
                  // Dedicated mode tasks are enabled.
                  //---------------------------------------------
                  {
                    // Rem: this task is used in dedicated/TCH_H/F
                    // No Scheduling result for FB26 task to avoid selection of this task.
                    if(desc_ptr->channel_type == TCH_H)
                    {
                      UWORD8 normalised_fn_mod26 = ((l1s.actual_time.fn_in_report) - desc_ptr->subchannel ) % 26;

                      if(normalised_fn_mod26 <= 22)
                        min_time_to_fb = 22 - normalised_fn_mod26;
                      else
                        min_time_to_fb = 26+ 22 - normalised_fn_mod26;
                    }
                    else
                    if(desc_ptr->channel_type == TCH_F)
                    {
                      UWORD8 normalised_fn_mod26 = l1s.actual_time.fn_in_report % 26;

                      if(normalised_fn_mod26 <= 23)
                        min_time_to_fb = 23 - normalised_fn_mod26;
                      else
                        min_time_to_fb = 26 + 23 - normalised_fn_mod26;
                    }
                  } // end DEDIC task
                  #if L1_GPRS
                  else
                  if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
                  //---------------------------------------------
                  // Packet Transfer mode task is enabled.
                  //---------------------------------------------
                  {
                    if(l1s.actual_time.t2 <= 23)
                      min_time_to_fb = 23 - l1s.actual_time.t2;
                    else
                      min_time_to_fb = 26 + 23 - l1s.actual_time.t2;
                  }
                  #endif

                  if(min_time_to_fb < ( UWORD32)best_time_to_fb)
                  {
                    best_time_to_fb = min_time_to_fb;
                    best_neigh_fb = i;
                  }
                }
                break;

                case 1:  // Approximate timing is supplied: search FB with a priori info..
              #if ((REL99 == 1) && (FF_RTD == 1))
                case 3:  // approximate timing based on RTD is supplied
              #endif
                {
                  BOOL check_fbcnf26 = 0;
                  UWORD32 min_time_to_fbconf = MAX_FN;

                  if (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
                  //---------------------------------------------
                  // Dedicated mode tasks are enabled.
                  //---------------------------------------------
                  {
                    UWORD8                 normalised_fn_mod26;


                    normalised_fn_mod26 = (l1s.actual_time.fn_in_report - desc_ptr->subchannel ) % 26;

                    if(((normalised_fn_mod26 == 22) && (desc_ptr->channel_type == TCH_H)) ||
                      (((l1s.actual_time.fn_in_report % 26) == 23) && (desc_ptr->channel_type == TCH_F)))
                    // Monitoring area control frame is reached...
                    {
                      check_fbcnf26 = 1;
                    }
                  }  // end DEDIC task
                  #if L1_GPRS
                  else
                  if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
                  //---------------------------------------------
                  // Packet Transfer mode task is enabled.
                  //---------------------------------------------
                  {
                    if(l1s.actual_time.t2 == 23)
                    // Monitoring area control frame is reached...
                    {
                      check_fbcnf26 = 1;
                    }
                  }
                  #endif

                  if(check_fbcnf26)
                  // FBCNF26 must be checked.
                  {
                    UWORD32 next_neigh_fn_mod51;
		  #if ((REL99 == 1) && (FF_RTD == 1))
		    UWORD32 next_neigh_time_alignmt;
		  #endif


                    // Get neighbor domain frame number %51 for frame 24.
                    next_neigh_fn_mod51 = (l1s.next_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                    if( ((next_neigh_fn_mod51 % 10) == 0)&&(next_neigh_fn_mod51 != 50) )
                    // FB26 rx window starts in the frame 24...
                    { // time_alignt has been advanced from 23 bits
                      UWORD32  u32Temp;
                      // and FB algo. needs 24 bits ==> miss 1 bit = 4 qbits
                      u32Temp =  l1_config.params.fb26_anchoring_time ;
                      if(l1a_l1s_com.nsync.list[i].time_alignmt >= u32Temp +4 )
                      // FB feet in the search slot...
                      {
                        min_time_to_fbconf = 0;
                        l1a_l1s_com.nsync.list[i].sb26_offset   = 0;
                      }
                    }
                    else
                    if((next_neigh_fn_mod51 == 50)||
                       (next_neigh_fn_mod51 == 9) ||
                       (next_neigh_fn_mod51 == 19) ||
                       (next_neigh_fn_mod51 == 29) ||
                       (next_neigh_fn_mod51 == 39))
                    // FB26 rx window starts in the frame 25...
                    {
                      UWORD16 fb26, sb26;
                      UWORD32 time;
			 UWORD32  u32temp ;
                      time = ((l1_config.params.fb26_anchoring_time  +FB26_ACQUIS_DURATION
                               -SB_ACQUIS_DURATION  +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE);

                      fb26 = FB26_ACQUIS_DURATION;
                      sb26 = SB_ACQUIS_DURATION;

		       u32temp = ((l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE);


			  if(l1a_l1s_com.nsync.list[i].time_alignmt < (UWORD32) u32temp )

			   /*

                      if(l1a_l1s_com.nsync.list[i].time_alignmt < (UWORD32)((l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE)) */
                      // FB feet in the search slot...
                      // Set FBCNF26 as PENDING to trigger the l1s_merge_manager() to install it
                      // in MFTAB.
                      {
                        min_time_to_fbconf = 0;
                        l1a_l1s_com.nsync.list[i].sb26_offset   = 1;
                      }
                    }

                  #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature

                    if ((l1a_l1s_com.nsync.list[i].timing_validity == 3) && (min_time_to_fbconf == 0))
                    {
                    #if !L1_EGPRS
                      next_neigh_time_alignmt = l1a_l1s_com.nsync.list[i].time_alignmt;
                    #endif

                      if (((next_neigh_fn_mod51 % 10) == 0) && (next_neigh_fn_mod51 != 50))
                      {   // for frame 0, 10, 20, 30, 40
                        if (l1a_l1s_com.nsync.list[i].fb26_position == 255)  // is it the first attempt?
                        {
                          if ((next_neigh_time_alignmt - RTD_LEFT_MARGIN) < l1_config.params.fb26_anchoring_time + 4)

                            fb26_position = 0 ; // it means first try in the multiframe 51 at position = 0,10,20,30,40
                          else
                            fb26_position = 255 ;
                        }
                        else
                        {     // the previous one must be at the other side position
                          if (l1a_l1s_com.nsync.list[i].fb26_position == 1)
                          {
                            fb26_position = l1a_l1s_com.nsync.list[i].fb26_position ;
                          }
                          else
                          {
                            min_time_to_fbconf = MAX_FN;  // discard this position which has been already tried
                          }
                        }
                      }
                      else
                      {     // for frame 9, 19, 29, 39, 50
                        if (l1a_l1s_com.nsync.list[i].fb26_position == 255)  // is it the first attempt?
                        {
                          if ((next_neigh_time_alignmt + RTD_RIGHT_MARGIN) > ((l1_config.params.fb26_anchoring_time
                                                                             + FB26_ACQUIS_DURATION
                                                                             - SB_ACQUIS_DURATION
                                                                             + TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE))
                            fb26_position = 1 ; // it means first try in the multiframe 51 at position = 9,19,29,39,50
                          else
                            fb26_position = 255 ;
                        }
                        else
                        {      // the previous one must be at the other side position
                          if (l1a_l1s_com.nsync.list[i].fb26_position == 0)
                          {
                            fb26_position = l1a_l1s_com.nsync.list[i].fb26_position ;
                          }
                          else
                          {
                            min_time_to_fbconf = MAX_FN;  // discard this position which has been already tried
                          }
                        }
                      }
                    }
                  #endif // L1_R99 RTD feature
                    if(min_time_to_fbconf == 0)
                    {
                      // Fixed BUG2963
                      best_time_to_fbconf = 0;
                      best_neigh_fbconf   = i;
                    }
                  }
                }
                break;

                case 2:  // Accurate timing is supplied SBCONF: confirm SB.
                {
                  BOOL check_sbcnf26 = 0;
                  UWORD32 min_time_to_sbconf = MAX_FN;

                  if (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
                  //---------------------------------------------
                  // Dedicated mode tasks are enabled.
                  //---------------------------------------------
                  {
                    UWORD8                 normalised_fn_mod26;

                    normalised_fn_mod26 = (l1s.actual_time.fn_in_report - desc_ptr->subchannel ) % 26;

                    if(((normalised_fn_mod26 == 22) && (desc_ptr->channel_type == TCH_H)) ||
                      (((l1s.actual_time.fn_in_report % 26) == 23) && (desc_ptr->channel_type == TCH_F)))
                    // Monitoring area control frame is reached...
                    {
                      check_sbcnf26 = 1;
                    }
                  }  // end DEDIC task
                  #if L1_GPRS
                  else
                  if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
                  //---------------------------------------------
                  // Packet Transfer mode task is enabled.
                  //---------------------------------------------
                  {
                    if(l1s.actual_time.t2 == 23)
                    // Monitoring area control frame is reached...
                    {
                      check_sbcnf26 = 1;
                    }
                  }
                  #endif

                  if(check_sbcnf26)
                  // SBCNF26 must be checked.
                  {
                    UWORD32 next_neigh_fn_mod51;
		      UWORD32 u32Temp;
		      UWORD32 u32Temp1;

                    // Get neighbor domain frame number %51 for frame 24.
                    next_neigh_fn_mod51 = (l1s.next_time.fn + l1a_l1s_com.nsync.list[i].fn_offset) % 51;

                    // Cq19539: the RF SB26 acquisition window shall be scheduled within the RF FB26 acquisition window range
                    // So the time alignment of the neighbour  (+ TPU_CLOCK_RANGE if needed) shall be within the range of
                    // [ fb26_anchoring_time , fb26_anchoring_time+FB26_ACQUIS_DURATION-SB_ACQUIS_DURATION+TPU_CLOCK_RANGE [
                    // (During an SB acquisition after a FB aquisition, it can't be outside of this range
                    // because of the clipping to MAX_TOA_FOR_SB into l1s_read_fb).
                    // If a SB26 burst acquisition window is started outside of this range, the RX of the NB
                    // in the next frame will be delayed by one frame due to the delay needed by the TPU
                    // to execute offset, leave TPU sleep mode, switch the TPU page, and restore the synchro.
                    // Such a delay is taken into account within the computation of fb26_anchoring_time.
                    // Therefore, a time alignement outside this range must be clipped to fb26_anchoring_time
                    // and SB will be scheduled at the end of the frame 24.

		     u32Temp1  = l1_config.params.fb26_anchoring_time ;
                   u32Temp= ((l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE );

		if (( l1a_l1s_com.nsync.list[i].time_alignmt >= u32Temp)
                       && ( l1a_l1s_com.nsync.list[i].time_alignmt <u32Temp1 ))


			/*
		     if (( l1a_l1s_com.nsync.list[i].time_alignmt >= (UWORD32)(l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE )
                       && ( l1a_l1s_com.nsync.list[i].time_alignmt < (UWORD32)l1_config.params.fb26_anchoring_time ))*/
                    {
                      // clip time alignment because it's outside of allowed range
                      l1a_l1s_com.nsync.list[i].time_alignmt = l1_config.params.fb26_anchoring_time;
                    }

                    if((next_neigh_fn_mod51 == 0)  ||
                       (next_neigh_fn_mod51 == 10) ||
                       (next_neigh_fn_mod51 == 20) ||
                       (next_neigh_fn_mod51 == 30) ||
                       (next_neigh_fn_mod51 == 40))
                    // check if SB26 rx window can be started in the frame 25...
                    {
                     u32Temp= ((l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE);
			 if (l1a_l1s_com.nsync.list[i].time_alignmt < u32Temp)

                     /* if(l1a_l1s_com.nsync.list[i].time_alignmt < (UWORD32)(l1_config.params.fb26_anchoring_time
                                                                    +FB26_ACQUIS_DURATION
                                                                    -SB_ACQUIS_DURATION
                                                                    +TPU_CLOCK_RANGE) % TPU_CLOCK_RANGE)*/
                      // SB feet in the search slot...
                      // Set sb26_offset to 1 to add a NOP in TPU scenario in order to reach frame 25
                      {
                        min_time_to_sbconf = 0;
                        l1a_l1s_com.nsync.list[i].sb26_offset   = 1;
                      }
                    }
                    else
                    if((next_neigh_fn_mod51 % 10) == 1)
                    // ... otherwise SB26 rx window starts at the end of the frame 24.
                    {
                    UWORD32 u32Temp;
		      u32Temp = 	l1_config.params.fb26_anchoring_time;
			if(l1a_l1s_com.nsync.list[i].time_alignmt >= u32Temp )
                      /*if(l1a_l1s_com.nsync.list[i].time_alignmt >= (UWORD32)l1_config.params.fb26_anchoring_time)*/
                      // SB feet in the search slot...
                      {
                        min_time_to_sbconf = 0;
                        l1a_l1s_com.nsync.list[i].sb26_offset   = 0;
                      }
                    }

                    if(min_time_to_sbconf == 0)
                    {
                      best_time_to_sbconf = 0;
                      best_neigh_sbconf = i;
                    }
                  }
                }
                break;

              #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
                case SB_ACQUISITION_PHASE:  // SB search following a FB search.
              #else
                case 3:  // SB search following a FB search.
              #endif
                {
                  UWORD32  min_time_to_sb = MAX_FN;
                  UWORD8 sb26_attempt = l1a_l1s_com.nsync.list[i].sb26_attempt;
				  UWORD32 u32Temp;
                  // Monitoring area starting frame is reached...
                  min_time_to_sb = l1a_l1s_com.nsync.list[i].fn_offset - l1s.actual_time.fn;

                  if ((WORD32)min_time_to_sb < 0)
                  //the SB following the FB was missed because of an higher priority task
                  //next SB is 26*9 frames later or it is 26*9+52 frames later
                  //Fix for BUG2864 and BUG2842
                  {
                     if (sb26_attempt == 1)
                        min_time_to_sb += 26*9;
                     else if (sb26_attempt == 2)
                        min_time_to_sb += 26*9+52;
                  else
                        min_time_to_sb += MAX_FN; //shall never happen
                  }

                  if (min_time_to_sb == 0)
                  {
                      l1a_l1s_com.nsync.list[i].sb26_attempt++;
                  }

		   u32Temp      =  best_time_to_sb;
                  if((min_time_to_sb < 26) && (min_time_to_sb < u32Temp ))
                  {
                    best_time_to_sb = min_time_to_sb;
                    best_neigh_sb = i;
                  }
                }
                break;

              } // End of "switch"
            } // End of "if PENDING"
          } // End of "for"

          #if L1_EDA
            if((l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED) && (l1ps_macs_com.fb_sb_task_detect))
            {
              //Flag used to cope with worst case mixing FB/SB task and allocation for MS class 12
              if(l1s.actual_time.t2 == 20)
              {
                l1ps_macs_com.fb_sb_task_enabled = TRUE;
              }
              else if (l1s.actual_time.t2 > 20)
                enter_nsync_task = l1ps_macs_com.fb_sb_task_enabled;
            }

            if (!(l1ps_macs_com.fb_sb_task_detect) || enter_nsync_task)
            {
          #endif
          // Remarks:
          //--------
          // While FB26/SB26/SBCNF26 are set as PENDING to trigger the
          // l1s_merge_manager() to install it , we have to select the
          // highest priority task HERE.

          if(best_neigh_sb != 255)
          // SB26 is higher priority while it can not be delayed without
          // performing again an FB26 search !
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_sb_id = best_neigh_sb;
            // set SB26 as pending to trigger the l1s_merge_manager()
            // to install it in MFTAB
            if (best_time_to_sb == 0)
              l1s.task_status[SB26].new_status = PENDING;
            // Enable SB2 task
            l1a_l1s_com.l1s_en_task[SB26] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[SB26] = SEMAPHORE_RESET;
          }

          else
          if(best_neigh_fbconf != 255)
          // FB CONFIRMATION is medium priority due to its short duration
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_fb_id = best_neigh_fbconf;
            // set FB as pending to trigger the l1s_merge_manager()
            // to install it in MFTAB
            if (best_time_to_fbconf == 0)
              l1s.task_status[FB26].new_status = PENDING;
            // Enable FB task
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_RESET;
                #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
                  if (l1a_l1s_com.nsync.list[best_neigh_fbconf].timing_validity == 3)
                  {   // check if we are not at the border of the window so FB must be detected inside the current window
                    if (fb26_position == 255)
                      l1a_l1s_com.nsync.list[best_neigh_fbconf].nb_fb_attempt = 1;
                    else
                      l1a_l1s_com.nsync.list[best_neigh_fbconf].fb26_position = fb26_position;
                  }
                #endif // L1_R99
          }

          else
          if(best_neigh_sbconf != 255)
          // SBCNF26 is medium priority due to its short duration
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_sbconf_id = best_neigh_sbconf;
            // set SBCNF26 as pending to trigger the l1s_merge_manager()
            // to install it in MFTAB
            if (best_time_to_sbconf == 0)
              l1s.task_status[SBCNF26].new_status = PENDING;
            // Enable SBCNF26 task
            l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_RESET;
          }

          else
          if(best_neigh_fb != 255)
          // FB26 is low priority due to its long duration
          {
            // Save active neighbour id.
            l1a_l1s_com.nsync.active_fb_id = best_neigh_fb;
            // set FB26 as pending to trigger the l1s_merge_manager()
            // to install it in MFTAB
            if (best_time_to_fb == 0)
              l1s.task_status[FB26].new_status = PENDING;
            // Enable FB26 task
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
            // Clear param. synchro. semaphore.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_RESET;
          }
          #if L1_EDA
            }
          #endif

        } // End of "if / current_status"
      } // End of " if NSYNC"

    } // End of "if channel is MF26"

    if (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
   {
#endif

    if(l1a_l1s_com.l1s_en_task[TCHA] == TASK_ENABLED)
    //----------------------------------
    // SACCH (TCH) task is ENABLED.
    //----------------------------------
    {
      UWORD32 time_to_tcha;

      if(l1s.next_time.fn_in_report <= 12)
        time_to_tcha = 12 - l1s.next_time.fn_in_report;
      else
      if(l1s.next_time.fn_in_report <= 38)
        time_to_tcha = 38 - l1s.next_time.fn_in_report;
      else
      if(l1s.next_time.fn_in_report <= 64)
        time_to_tcha = 64 - l1s.next_time.fn_in_report;
      else
      if(l1s.next_time.fn_in_report <= 90)
        time_to_tcha = 90 - l1s.next_time.fn_in_report;
      else
        time_to_tcha = 104 + 12 - l1s.next_time.fn_in_report;

      // Save scheduling result.
      l1s.task_status[TCHA].time_to_exec = time_to_tcha;
    }

    if(l1a_l1s_com.l1s_en_task[TCHTF] == TASK_ENABLED)
    //----------------------------------
    // TCHTF task is ENABLED.
    //----------------------------------
    {
      UWORD32 time_to_tchf;

      #if TESTMODE
        // if UL-only, ONLY schedule TCHA task, and schedule it every frame
        if (l1_config.TestMode && (l1_config.tmode.rf_params.down_up == TMODE_UPLINK))
        {
          // Save scheduling result: force TCHA task over TCHTF
          l1s.task_status[TCHA].time_to_exec = 0;
          l1s.task_status[TCHTF].time_to_exec = 0xff;
        }
        else
        {
          if(l1s.next_time.fn_mod13 == 12)
            time_to_tchf = 1;
          else
            time_to_tchf = 0;

          // Save scheduling result.
          l1s.task_status[TCHTF].time_to_exec = time_to_tchf;
        }
      #else
        if(l1s.next_time.fn_mod13 == 12)
          time_to_tchf = 1;
        else
          time_to_tchf = 0;

        // Save scheduling result.
        l1s.task_status[TCHTF].time_to_exec = time_to_tchf;
      #endif
    }

    if(l1a_l1s_com.l1s_en_task[TCHTH] == TASK_ENABLED)
    //----------------------------------
    // TCHTF task is ENABLED.
    //----------------------------------
    {
      UWORD32 time_to_tchh;
      UWORD32 time_to_tchd;
      WORD32  normalised_fn_mod13;
      UWORD8 subchannel;
      subchannel = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel;
      normalised_fn_mod13 = l1s.next_time.fn_mod13 -
                              subchannel;

      if(normalised_fn_mod13 < 0) normalised_fn_mod13 += 13;

      if((normalised_fn_mod13 == 0) || (normalised_fn_mod13 == 2)  ||
         (normalised_fn_mod13 == 4) || (normalised_fn_mod13 == 6)  ||
         (normalised_fn_mod13 == 8) || (normalised_fn_mod13 == 10))
      {
        time_to_tchh = 0;
        time_to_tchd = 1;
      }
      else
      {
        time_to_tchd = 0;
        /*if (l1s.next_time.fn_mod13 == 12)
        {
          if (subchannel == 0)
          {
            time_to_tchd = 2;
          }
          else
          {
            time_to_tchd = 1;
          }
        }*/
        if(normalised_fn_mod13 == 11)
          time_to_tchh = 2;
        else
          time_to_tchh = 1;
      }

      // Save scheduling result.
      l1s.task_status[TCHTH].time_to_exec = time_to_tchh;
      l1s.task_status[TCHD].time_to_exec  = time_to_tchd;
    }

#if (L1_12NEIGH == 1)
  }
#endif

  } // End of if(DEDIC is ENABLED)

#if L1_GPRS
  //==========================================================
  //==             PACKET ACCESS                            ==
  //==========================================================
  // Either fixed block allocation or USF method may be
  // used to determine possible PRACH slots. If fixed blocks
  // are allocated they take precedence over USF decoding
  // When PRACH slot is determined by fixed allocation this
  // is marked by fix_alloc_flag=TRUE, and USF method is not
  // executed afterwards.
  {
    if(l1a_l1s_com.l1s_en_task[PRACH] == TASK_ENABLED)
    // PRACH task enabled
    {
      BOOL fix_alloc_flag = FALSE;

      #if FF_L1_IT_DSP_USF
        // No USF uncertainty by default i.e. if fixed allocation mode is used
        // or dynamic on TDMA 1, 2 or 3 of a block.
        l1ps_macs_com.usf_status = USF_AVAILABLE;
      #endif // FF_L1_IT_DSP_USF

      if(l1pa_l1ps_com.pra_info.prach_alloc != DYN_PRACH_ALLOC)
      // FIXED BLOCK ALLOCATION
      {
        UWORD8 fn_mod_52;
        UWORD8 i, start_time =0;//omaps00090550

        fn_mod_52 = l1s.next_time.fn_mod52;  // start times for control phase

        for (i=0; i<l1pa_l1ps_com.pra_info.bs_prach_blks; i++)
        {
          // Block boundary for PRACH
          start_time = ORDERED_BLOCK_START_TIME[i];

          if ((fn_mod_52 == start_time)     ||
              (fn_mod_52 == start_time + 1) ||
              (fn_mod_52 == start_time + 2) ||
              (fn_mod_52 == start_time + 3))
          {
            fix_alloc_flag = TRUE;

            l1pa_l1ps_com.pra_info.rand --;

            if ( l1pa_l1ps_com.pra_info.rand == 0)
            {
              l1s.task_status[PRACH].new_status  = PENDING;
              l1pa_l1ps_com.pra_info.prach_alloc = FIX_PRACH_ALLOC;
            }
            break;
          }
        } // end for
      }

      // either FIXED blocks or USF method
      if(!fix_alloc_flag)
      // DYNAMIC MODE WITH USF DECODING
      {
        API d_usf_updated;
        static API static_usf_state;  // USF read on first frame of a block, valid for whole block
        UWORD8 actual_fn_mod13_mod4 = l1s.actual_time.fn_mod13_mod4;
        UWORD8 next_fn_mod13_mod4   = l1s.next_time.fn_mod13_mod4;

        // Flag used for correcting l1pa_l1ps_com.pra_info.rand in case it
        // has been decremented although USF was bad
        static BOOL static_usf_invalid_flag = FALSE;

        // Get USF from DSP => for PRACH only TS0 is needed
        // Read frame 3 of each block to anticipate USF state for first frame of next block
        // Do not read USF on frames that preceed idle frames
        if ((next_fn_mod13_mod4 == 0) && (l1s.next_time.fn_mod13 != 12))
        {
          d_usf_updated = ((l1ps_dsp_com.pdsp_ndb_ptr->d_usf_updated_gprs >> ((7-0)*2)) & 0x0003);

          if (d_usf_updated == USF_INVALID)
          // USF decoding on block boundary
          // if USF invalid => decision to send PRACH is left to DSP
          {
             static_usf_invalid_flag = TRUE;
             l1pa_l1ps_com.pra_info.rand --;

             if ( l1pa_l1ps_com.pra_info.rand == 0)
             {
               l1s.task_status[PRACH].new_status = PENDING;

               #if FF_L1_IT_DSP_USF
                 // Dynamic allocation in use, random number of opportunities
                 // elapsed and USF status is not available. Therefore we have
                 // to rely on DSP USF interrupt.
                 l1ps_macs_com.usf_status = USF_AWAITED;
               #endif // FF_L1_IT_DSP_USF
             }
          }
          static_usf_state = d_usf_updated;

        }
        // Update USF state on the first frame of each block
        else if (actual_fn_mod13_mod4 == 0)
        {
          static_usf_state = ((l1ps_dsp_com.pdsp_ndb_ptr->d_usf_updated_gprs >> ((7-0)*2)) & 0x0003);

          if ((static_usf_state != USF_GOOD) && (static_usf_invalid_flag == TRUE))
          // rand was decremented in previous frame but USF is bad => compensate
          {
            l1pa_l1ps_com.pra_info.rand ++;
            static_usf_invalid_flag = FALSE;
          }
        }

        if ((static_usf_state == USF_GOOD) && (l1s.next_time.fn_mod13 != 12))
        {
          static_usf_invalid_flag = FALSE;

          l1pa_l1ps_com.pra_info.rand --;

          if ( l1pa_l1ps_com.pra_info.rand == 0)
          {
            l1s.task_status[PRACH].new_status = PENDING;
          }
        }
      }
    }
  }

  //=====================================================
  //==             PACKET POLLING                      ==
  //=====================================================
  {
    if(l1a_l1s_com.l1s_en_task[POLL] == TASK_ENABLED)
    // POLL task enabled (packet queuing notification
    {
      if(l1s.task_status[POLL].current_status == INACTIVE)
      {

        UWORD32 time_to_poll;

        time_to_poll = ( (l1pa_l1ps_com.poll_info.fn) - (l1s.next_time.fn % 42432) + 2*42432) % 42432;

        if((time_to_poll >= 32024) && (time_to_poll <= 42431))
        {
          xSignalHeaderRec *msg;

          // Poll response occurence passed...

          // Task disabled to avoid a new transmission of L1P_POLL_DONE in following
          // TDMA if L1A isn't executed
          // This isn't a problem because the sending of two simultaneous POLL is forbidden
          l1a_l1s_com.l1s_en_task[POLL] = TASK_DISABLED;

          // send an error message to L1A
          msg = os_alloc_sig(sizeof(T_MPHP_POLLING_IND));
          DEBUGMSG(status,NU_ALLOC_ERR)

          ((T_MPHP_POLLING_IND *)(msg->SigP))->fn = 0xFFFFFFFF; // Invalid (TO BE CONFIRMED)
          msg->SignalCode = L1P_POLL_DONE;

          os_send_sig(msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        }
        else if (time_to_poll == 0)
        {
          // Poll reponse occurs on next FN
          l1s.task_status[POLL].new_status = PENDING;
          l1s.task_status[POLL].time_to_exec = time_to_poll;
        }
        else
        {
          // Update scheduling result
          l1s.task_status[POLL].time_to_exec = time_to_poll;
        }
      }
    }
  }

  //================================================================
  //==                   PACKET TASKS                             ==
  //==                   ------------                             ==
  //================================================================
  {
    if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
    //-------------------------
    // PDTCH task is ENABLED.
    //-------------------------
    {
      UWORD8 time_to_pdtch;

      // PDTCH task begins on a block boundary (fn_mod13 = 0, 4, 8)
      if (l1s.next_time.fn_mod13 == 0)
        time_to_pdtch = 0;
      else
      if (l1s.next_time.fn_mod13 <= 4)
        time_to_pdtch = 4 - l1s.next_time.fn_mod13;
      else
      if (l1s.next_time.fn_mod13 <= 8)
        time_to_pdtch = 8 - l1s.next_time.fn_mod13;
      else
        time_to_pdtch = 13 - l1s.next_time.fn_mod13;

      // Save scheduling result.
      l1s.task_status[PDTCH].time_to_exec  = time_to_pdtch;
    }

    if(l1a_l1s_com.l1s_en_task[PTCCH] == TASK_ENABLED)
    //-------------------------
    // PTCCH task is ENABLED.
    //-------------------------
    {
      #define  TA_INDEX  l1pa_l1ps_com.transfer.aset->packet_ta.ta_index

      typedef struct
      {
        UWORD16  start_ptcch_dl_block;  // DL PTCCH block position (2 TDMA in advance).
        UWORD16  tai_to_fnmod416;       // UL PTCCH position (2 TDMA in advance).
      }
      T_PTCCH_SCHEDULE_INFO;

      const T_PTCCH_SCHEDULE_INFO  PTCCH_SCHEDULE[16] =
      {
        {116, 12},{116, 38},{116, 64},{116, 90},
        {220,116},{220,142},{220,168},{220,194},
        {324,220},{324,246},{324,272},{324,298},
        { 12,324},{ 12,350},{ 12,376},{ 12,402}
      };

      UWORD16  fn_mod416         = l1s.next_time.fn % 416;
      UWORD32  time_to_ptcch_dl  = MAX_FN;
      UWORD32  time_to_ptcch_ul  = MAX_FN;

      // PTCCH/DL scheduling...
      {
        if(l1pa_l1ps_com.transfer.ptcch.request_dl)
        // Request to start PTCCH/DL after PTCCH/UL execution.
        {
          if(fn_mod416 <= PTCCH_SCHEDULE[TA_INDEX].start_ptcch_dl_block)
            time_to_ptcch_dl = PTCCH_SCHEDULE[TA_INDEX].start_ptcch_dl_block - fn_mod416;
          else
            time_to_ptcch_dl = 416 + PTCCH_SCHEDULE[TA_INDEX].start_ptcch_dl_block - fn_mod416;

          if(time_to_ptcch_dl == 0)
          {
            // PTCCH must execute PTCCH/DL.
            l1pa_l1ps_com.transfer.ptcch.activity |= PTCCH_DL;

            // Reset DL request bit.
            l1pa_l1ps_com.transfer.ptcch.request_dl = FALSE;
          }
        }

        if(l1pa_l1ps_com.transfer.ptcch.activity & PTCCH_DL)
        // PTCCH/DL alrady active, continue reception at correct boundaries.
        {
          if(l1s.next_time.t2 <= 12)
            time_to_ptcch_dl = 12 - l1s.next_time.t2;
          else
            time_to_ptcch_dl = 26 + 12 - l1s.next_time.t2;
        }
      }

      // PTCCH/UL scheduling...always enabled according to TAI.
      {
        // Schedule PTCCH/UL according to TAI.
        if(fn_mod416 <= PTCCH_SCHEDULE[TA_INDEX].tai_to_fnmod416)
          time_to_ptcch_ul = PTCCH_SCHEDULE[TA_INDEX].tai_to_fnmod416 - fn_mod416;
        else
          time_to_ptcch_ul = 416 + PTCCH_SCHEDULE[TA_INDEX].tai_to_fnmod416 - fn_mod416;

        if(time_to_ptcch_ul == 0)
        {
          // PTCCH must execute PTCCH/UL.
          l1pa_l1ps_com.transfer.ptcch.activity |= PTCCH_UL;
        }
      }

      // Save scheduling result.
      if(time_to_ptcch_dl < time_to_ptcch_ul)
        l1s.task_status[PTCCH].time_to_exec = time_to_ptcch_dl;
      else
        l1s.task_status[PTCCH].time_to_exec = time_to_ptcch_ul;

      // Clear param. synchro. semaphore.
      l1a_l1s_com.task_param[PTCCH] = SEMAPHORE_RESET;
    }

    if(l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)
    //-------------------------
    // SINGLE task is ENABLED.
    //-------------------------
    {
      // SINGLE task can be re-entrant, we can't check for
      // current_status == INACTIVE.

      if((l1pa_l1ps_com.transfer.single_block.activity & SINGLE_DL) ||
         (l1pa_l1ps_com.transfer.single_block.activity & SINGLE_UL))
      // L1 is camped on SINGLE timeslot.
      // -> no need to make a temporary synchro change.
      // -> SINGLE task is scheduled 1 frame before the block position.
      {
        UWORD32  time_to_single;

        if (l1s.next_time.fn_mod13 == 0)
          time_to_single = 0;
        else
        if (l1s.next_time.fn_mod13 <= 4)
          time_to_single = 4 - l1s.next_time.fn_mod13;
        else
        if (l1s.next_time.fn_mod13 <= 8)
          time_to_single = 8 - l1s.next_time.fn_mod13;
        else
          time_to_single = 13 - l1s.next_time.fn_mod13;

        // SINGLE task requires to run L1S scheduler every frame.
        l1a_l1s_com.time_to_next_l1s_task = 0;

        // Save scheduling result.
        l1s.task_status[SINGLE].time_to_exec = time_to_single;
      }
    } // SINGLE task enabled

    if(l1a_l1s_com.l1s_en_task[ITMEAS] == TASK_ENABLED)
    //-------------------------
    // ITMEAS task is ENABLED.
    //-------------------------
    {
      UWORD8 time_to_itmeas;

      // time to ITMEAS processing
      // ITMEAS must be scheduled 2 frames in advance (C W W R scheme)
      // --> use of 'next_plus_time'
      switch(l1pa_l1ps_com.itmeas.position)
      {
        case ANY_IDLE_FRAME:
          // ITMEAS task must be scheduled on any idle frame --> fn_mod13 = 12
          time_to_itmeas = 12 - l1s.next_plus_time.fn_mod13;
        break;

        case PTCCH_FRAME:
        {
          // ITMEAS must be scheduled on a PTCCH frame --> fn_mod26 = 12
          if (l1s.next_plus_time.t2 <= 12)
            time_to_itmeas = 12 - l1s.next_plus_time.t2;
          else
            time_to_itmeas = 38 - l1s.next_plus_time.t2;
        }
        break;

        case SEARCH_FRAME:
        {
          // ITMEAS must be scheduled on a search frame --> fn_mod26 = 25
          time_to_itmeas = 25 - l1s.next_plus_time.t2;
        }
        break;
      } // End of "switch"

      // Save scheduling result.
      l1s.task_status[ITMEAS].time_to_exec = time_to_itmeas;

      if(l1s.task_status[ITMEAS].current_status == INACTIVE)
      {
      // Clear param. synchro. semaphore.
      l1a_l1s_com.task_param[ITMEAS] = SEMAPHORE_RESET;
      }
    } // ITMEAS task enabled
  } // end of packet tasks
#endif

  //--------------------------------------------
  // TASK SELECTION...
  //--------------------------------------------
  {
    WORD32 forward_task          = NO_NEW_TASK;
    WORD32 forward_task_complete = MAX_FN;
    WORD32 forward_task_size     = MAX_FN;

    WORD32 backward_task = NO_NEW_TASK;
    WORD8  task;

    // FORWARD procedure...
    // Look for best pending task...
    // Tasks are ordered: Low priority .. to high priority.
    for(task=0; task<NBR_DL_L1S_TASKS; task++)
    {
      if(l1a_l1s_com.l1s_en_task[task] == TASK_ENABLED)
      {
        if(l1s.task_status[task].time_to_exec < forward_task_complete)
        {
          forward_task = task;
          forward_task_complete = l1s.task_status[task].time_to_exec + TASK_ROM_MFTAB[task].size - 2;
        }
      }
    }

    if(forward_task != NO_NEW_TASK)
    {
      task = forward_task;

      // BACKWARD procedure...
      while((--task>=0) && (backward_task == NO_NEW_TASK))
      {
        if(l1a_l1s_com.l1s_en_task[task] == TASK_ENABLED)
        {
          if((l1s.task_status[task].time_to_exec + TASK_ROM_MFTAB[task].size - 2) <=
             (l1s.task_status[forward_task].time_to_exec))
          {
            backward_task = task;
          }
        }
      }

      if(backward_task != NO_NEW_TASK)
      {
        Select_min_time(l1s.task_status[backward_task].time_to_exec,
                        l1a_l1s_com.time_to_next_l1s_task);

        // Pass back the best task.
        if(l1s.task_status[backward_task].time_to_exec == 0)
          *best_pending_task = backward_task;
        else
          *best_pending_task = NO_NEW_TASK;
      }
      else
      {
        Select_min_time(l1s.task_status[forward_task].time_to_exec,
                        l1a_l1s_com.time_to_next_l1s_task);

        // Pass back the best task.
        if(l1s.task_status[forward_task].time_to_exec == 0)
          *best_pending_task = forward_task;
        else
          *best_pending_task = NO_NEW_TASK;
      }
    }
    else
    {
      // Pass back the best task.
      *best_pending_task = NO_NEW_TASK;
    }
  }
} // End of procedure.

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_merge_manager()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_merge_manager(WORD32 pending_task)
{
  if(pending_task != NO_NEW_TASK)
  // There is a new pending task...
  {
    if(pending_task == SYNCHRO)
    // SYNCHRO task must be executed in any case.
    // It is used also to reset the MFTAB content.
    {
      WORD32 i;

      #if L1_GPRS
        // PDTCH task can't be aborted... SYNCHRO is delayed to next block boundary
        // This is needed when a synchronization change is needed to switch to a new
        // while a TBF is currently running: the end of the current TBF mustn't
        // be aborted so the first block of the new TBF will be used to switch to the
        // new synchronization.
        // SYNCHRO task has also to be delayed if a task needing a pseudo-synchro (with a synchro back
        // performed on the burst4, i.e. PBCCHS, PBCCHN_IDLE, SMSCB, PBCCHN_TRAN and Normal or Extended BCCH
        // ,(Packet)normal and (Packet)extended paging in Packet Transfer) is interrupted. This delay is requested in order to
        // to finish the current TPU scenario and to not disturb current L1 Timeslot Number Reference.
        // Cf. correction of BUG1004.
        if((l1s.task_status[PBCCHS].current_status == INACTIVE)      &&
           (l1s.task_status[PBCCHN_IDLE].current_status == INACTIVE) &&
           (l1s.task_status[PBCCHN_TRAN].current_status == INACTIVE) &&
           (l1s.task_status[PDTCH].current_status == INACTIVE)       &&
           (l1s.task_status[SMSCB].current_status == INACTIVE)       &&
           (((l1a_l1s_com.mode != PACKET_TRANSFER_MODE) ||
               ((l1s.task_status[NBCCHS].current_status == INACTIVE) &&
               (l1s.task_status[EBCCHS].current_status == INACTIVE)
           #if 0	/* FreeCalypso TCS211 reconstruction */
	       && (l1s.task_status[PNP].current_status == INACTIVE)
               && (l1s.task_status[PEP].current_status == INACTIVE)
               && (l1s.task_status[NP].current_status == INACTIVE)
               && (l1s.task_status[EP].current_status == INACTIVE)
           #endif
           ))))

       #else
         if (l1s.task_status[SMSCB].current_status == INACTIVE)
       #endif
         {
           // Clear the current content of the DL MFTAB.
           l1s_clear_mftab(l1s.mftab.frmlst);

           #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
             trace_info.abort_task = pending_task;
           #endif

           // Load the ABORT function in the MFTAB.
           l1s_load_mftab( BLOC_ABORT,            // Rom block address.
                           BLOC_ABORT_SIZE,       // Rom block size.
                           l1s.afrm,              // Start with current frame.
                           l1s.mftab.frmlst);     // Proceed on MFTAB.

           // Load the new task in the MFTAB.
           l1s_load_mftab( TASK_ROM_MFTAB[pending_task].address,  // Rom block address.
                           TASK_ROM_MFTAB[pending_task].size,     // Rom block size.
                           l1s.afrm,                              // Start with current frame.
                           l1s.mftab.frmlst);                     // Proceed on MFTAB.

           // All task become INACTIVE except PENDING which becomes ACTIVE.
           for(i=0; i<NBR_DL_L1S_TASKS; i++)
             l1s.task_status[i].current_status = INACTIVE;

           l1s.task_status[pending_task].current_status = ACTIVE;

           // Load FRAME_COUNT with the new task Rom block size.
           l1s.frame_count = TASK_ROM_MFTAB[pending_task].size;

           // MFTAB is reset, no more task not compatible with Neigh. Measurement.
           // Clear "forbid_meas".
           l1s.forbid_meas = 0;

           // Check that ABORT task is not bigger than pending_task.
           if(BLOC_ABORT_SIZE > l1s.frame_count) l1s.frame_count = BLOC_ABORT_SIZE;

           return;
         }

    } //(pending_task == SYNCHRO)
    else
    if(l1s.frame_count <= 2)
    // The incoming pending task can be merged within the MFTAB.
    {

      BOOL specific_case = FALSE;
      if(pending_task == DUL)
      { // this DOOLEAN is mandatory because we can access to this channel description only in dedicated
        // otherwise we have memory access error during the execution.
        T_CHANNEL_DESCRIPTION *desc_ptr = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;
        if ((desc_ptr->channel_type == SDCCH_8) && ((desc_ptr->subchannel>=4 && (desc_ptr->subchannel<=7))) )
          if ((l1s.actual_time.fn % 102) > 51) // only in the second multiframe D51
            specific_case = TRUE;
      }

      if(specific_case == TRUE)
      {
        // DUL(subch6) and ADL(subch6) (same for subch 4,5,6,7).
        // Here the task DUL is already installed which means that
        // if we install the ADL task we will have the UL task
        // controled before the DL task. This is not compatible
        // with the tpu programming.
        // To correct the problem, we detect this case, flush the
        // MFTAB and install a mixed DL/UL task block containing
        // the full ADL task and the end of the DUL task.
        // (see mftab.h)
        // The MFTAB is cleared.
        // Load the SPECIAL ADL/DUL MIXED TASK in the MFTAB.

        l1s_load_mftab( BLOC_DUL_ADL_MIXED,       // Rom block address.
                        BLOC_DUL_ADL_MIXED_SIZED, // Rom block size.
                        l1s.afrm,                 // Start with current frame.
                        l1s.mftab.frmlst);        // Proceed on MFTAB.

        l1s.task_status[DUL].current_status = ACTIVE;
        l1s.task_status[ADL].current_status = ACTIVE;

        // Load FRAME_COUNT with the new task Rom block size.
        l1s.frame_count = BLOC_DUL_ADL_MIXED_SIZED;
      }
      else
      {
        l1s_load_mftab( TASK_ROM_MFTAB[pending_task].address,   // Rom block address.
                        TASK_ROM_MFTAB[pending_task].size,      // Rom block size.
                        l1s.afrm,                               // Start with current frame.
                        l1s.mftab.frmlst);                      // Proceed on MFTAB.

        // PENDING becomes ACTIVE except if it is already active,
        // in this case is is stated RE_ENTERED.
        if(l1s.task_status[pending_task].current_status == ACTIVE)
          l1s.task_status[pending_task].current_status = RE_ENTERED;
        else
          l1s.task_status[pending_task].current_status = ACTIVE;

        // Load FRAME_COUNT with the new task Rom block size.
        l1s.frame_count = TASK_ROM_MFTAB[pending_task].size;
      }
    }

    else
    // The incoming pending task CANNOT be merged within the MFTAB.
    {
      #if L1_GPRS
      // Interference measurements special merging case
      // Merge is not possible when FB26/SB26/SBCNF26 in packet transfer
      // are also pending.
      if((pending_task == ITMEAS) &&
         (l1s.task_status[FB26].new_status    == NOT_PENDING) &&
         (l1s.task_status[SB26].new_status    == NOT_PENDING) &&
         (l1s.task_status[SBCNF26].new_status == NOT_PENDING))
      {
        // If frame_count = 3:
        // - Serving task (aligned on MF52 or MF51): interference measurements can
        //   be done during the last work phase (merge with the last Control
        //   of the serving task)
        //
        //     Frame count    6 5 4 3 2 1
        //     Serving task   C W R
        //                      C W R
        //                        C W R
        //                          C W R
        //     ITMEAS               C W W R
        //
        // - Neighbor tasks: merge always forbidden by l1s.forbid_meas

        if((l1s.frame_count == 3) && (l1s.forbid_meas < 2))
        {
          l1s_load_mftab( TASK_ROM_MFTAB[ITMEAS].address,   // Rom block address.
                          TASK_ROM_MFTAB[ITMEAS].size,      // Rom block size.
                          l1s.afrm,                         // Start with current frame.
                          l1s.mftab.frmlst);                // Proceed on MFTAB.

          // PENDING becomes ACTIVE.
          l1s.task_status[pending_task].current_status = ACTIVE;

          // Load FRAME_COUNT with the new task Rom block size.
          l1s.frame_count = TASK_ROM_MFTAB[pending_task].size;
        }
      } // End if "pending task is ITMEAS"
      else
      #endif

      if((l1s.task_status[FBNEW].current_status != INACTIVE) && (pending_task != FBNEW))
      // FBNEW task is the only aborted task.
      // We check to avoid aborting FBNEW by itself.
      {
        WORD32 i;

        // Clear the current content of the DL MFTAB.
        l1s_clear_mftab(l1s.mftab.frmlst);

         #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4))
           trace_info.abort_task = pending_task;
         #endif

        // Load the ABORT function in the MFTAB.
        l1s_load_mftab( BLOC_ABORT,            // Rom block address.
                        BLOC_ABORT_SIZE,       // Rom block size.
                        l1s.afrm,              // Start with current frame.
                        l1s.mftab.frmlst);     // Proceed on MFTAB.

        // Load the new task in the MFTAB.
        l1s_load_mftab( TASK_ROM_MFTAB[pending_task].address,  // Rom block address.
                        TASK_ROM_MFTAB[pending_task].size,     // Rom block size.
                        l1s.afrm,                              // Start with current frame.
                        l1s.mftab.frmlst);                     // Proceed on MFTAB.

        // All task become INACTIVE except PENDING which becomes ACTIVE.
        for(i=0; i<NBR_DL_L1S_TASKS; i++) l1s.task_status[i].current_status = INACTIVE;
        l1s.task_status[pending_task].current_status = ACTIVE;

        // Load FRAME_COUNT with the new task Rom block size.
        l1s.frame_count = TASK_ROM_MFTAB[pending_task].size;

        // MFTAB is reset, no more task not compatible with Neigh. Measurement.
        // Clear "forbid_meas".
        l1s.forbid_meas = 0;

        // Check that ABORT task is not bigger than pending_task.
        if(BLOC_ABORT_SIZE > l1s.frame_count) l1s.frame_count = BLOC_ABORT_SIZE;
      }

      else
      // NO abort.
      {}
    }
  }

  //---------------------------------------------------------
  // Additional tasks................
  //---------------------------------------------------------
  // -> These tasks are generaly supperposed to
  //    another main task.
  //      RAACC with ALLC / NP / EP /  NBCCHS / EBCCHS
  //      FB26 / SB26 / SBCNF26 with TCHTF
  //      ...
  //---------------------------------------------------------

  if(l1s.task_status[RAACC].new_status == PENDING)
  // CHANNEL ACCESS task is pending and MUST be set in the MFTAB.
  // Merge is always possible since only the serving tasks can be running.
  //   ->install RAACC whitout any other change.
  {
    l1s_load_mftab( TASK_ROM_MFTAB[RAACC].address,   // Rom block address.
                    TASK_ROM_MFTAB[RAACC].size,      // Rom block size.
                    l1s.afrm,                        // Start with current frame.
                    l1s.mftab.frmlst);               // Proceed on MFTAB.

    l1s.task_status[RAACC].current_status = ACTIVE;

    // Load FRAME_COUNT with the RAAC Rom block size.
    if(l1s.frame_count < TASK_ROM_MFTAB[RAACC].size)
      l1s.frame_count = TASK_ROM_MFTAB[RAACC].size;
  }

  #if L1_GPRS
  if((l1s.task_status[TCHTF].current_status != INACTIVE) ||
     (l1s.task_status[TCHTH].current_status != INACTIVE) ||
     (l1s.task_status[PDTCH].current_status != INACTIVE))
  #endif
  {
    // Dedicated/Transfer mode monitoring tasks...
    if(l1s.task_status[FB26].new_status == PENDING)
    {
      UWORD8 time_to_task_complete = TASK_ROM_MFTAB[FB26].size - 2;

      #if L1_GPRS
      if((l1s.task_status[PBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[PBCCHN_TRAN].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[BCCHN_TRAN ].time_to_exec >= time_to_task_complete))
      #endif
      {
        l1s_load_mftab( TASK_ROM_MFTAB[FB26].address,   // Rom block address.
                        TASK_ROM_MFTAB[FB26].size,      // Rom block size.
                        l1s.afrm,                       // Start with current frame.
                        l1s.mftab.frmlst);              // Proceed on MFTAB.

        l1s.task_status[FB26].current_status = ACTIVE;
      }
    }
    else
    if(l1s.task_status[SB26].new_status == PENDING)
    {
      UWORD8 time_to_task_complete = TASK_ROM_MFTAB[SB26].size - 2;

      #if L1_GPRS
      if((l1s.task_status[PBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[PBCCHN_TRAN].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[BCCHN_TRAN ].time_to_exec >= time_to_task_complete))
      #endif
      {
        l1s_load_mftab( TASK_ROM_MFTAB[SB26].address,   // Rom block address.
                        TASK_ROM_MFTAB[SB26].size,      // Rom block size.
                        l1s.afrm,                       // Start with current frame.
                        l1s.mftab.frmlst);              // Proceed on MFTAB.

        l1s.task_status[SB26].current_status = ACTIVE;
      }
    }
    else
    if(l1s.task_status[SBCNF26].new_status == PENDING)
    {
      UWORD8 time_to_task_complete = TASK_ROM_MFTAB[SBCNF26].size - 2;

      #if L1_GPRS
      if((l1s.task_status[PBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EBCCHS     ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[PBCCHN_TRAN].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[NP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[EP         ].time_to_exec >= time_to_task_complete) &&
         (l1s.task_status[BCCHN_TRAN ].time_to_exec >= time_to_task_complete))
      #endif
      {
        l1s_load_mftab( TASK_ROM_MFTAB[SBCNF26].address,   // Rom block address.
                        TASK_ROM_MFTAB[SBCNF26].size,      // Rom block size.
                        l1s.afrm,                          // Start with current frame.
                        l1s.mftab.frmlst);                 // Proceed on MFTAB.

        l1s.task_status[SBCNF26].current_status = ACTIVE;
      }
    }
  }

#if L1_GPRS
  if(l1s.task_status[PRACH].new_status == PENDING)
  // PACKET CHANNEL ACCESS task is pending and MUST be set in the MFTAB.
  // Merge is always possible since only the serving tasks can be running.
  //   ->install PRACH without any other change.
  {
    l1s_load_mftab( TASK_ROM_MFTAB[PRACH].address,   // Rom block address.
                    TASK_ROM_MFTAB[PRACH].size,      // Rom block size.
                    l1s.afrm,                        // Start with current frame.
                    l1s.mftab.frmlst);               // Proceed on MFTAB.

    l1s.task_status[PRACH].current_status = ACTIVE;

    // Load FRAME_COUNT with the RAAC Rom block size.
    if(l1s.frame_count < TASK_ROM_MFTAB[PRACH].size)
      l1s.frame_count = TASK_ROM_MFTAB[PRACH].size;
  }

  else
  if((l1s.task_status[POLL].new_status == PENDING) &&
     (l1s.task_status[POLL].current_status != ACTIVE))
  // POLL RESPONSE task is pending.
  // Merge is not always possible since POLL can conflict with a Neighbour process
  // (SB2, SBCONF, BCCHN, BCCHN_TOP, PBCCHN_IDLE) or PBCCHS and CCCH operation mode II/III tasks.
  // From the fact that POLL can be load in MFTAB here below (when there is no pending_task
  // except POLL), merging can be done only if current_status is != ACTIVE.
  {
    UWORD8 time_to_task_complete = BLOC_POLL_NO_HOPP_SIZE - 2;

    if((l1s.task_status[PBCCHS     ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[PBCCHN_IDLE].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[BCCHN      ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[BCCHN_TOP  ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[SMSCB      ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[NP         ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[EP         ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[SB2        ].time_to_exec >= time_to_task_complete) &&
       (l1s.task_status[SBCONF     ].time_to_exec >= time_to_task_complete))
    {
      if((l1s.task_status[PBCCHS     ].current_status != ACTIVE) &&
         (l1s.task_status[PBCCHN_IDLE].current_status != ACTIVE) &&
         (l1s.task_status[BCCHN      ].current_status != ACTIVE) &&
         (l1s.task_status[BCCHN_TOP  ].current_status != ACTIVE) &&
         (l1s.task_status[SMSCB      ].current_status != ACTIVE) &&
         (l1s.task_status[NP         ].current_status != ACTIVE) &&
         (l1s.task_status[EP         ].current_status != ACTIVE) &&
         (l1s.task_status[SB2        ].current_status != ACTIVE) &&
         (l1s.task_status[SBCONF     ].current_status != ACTIVE))
      {
        l1s_load_mftab( BLOC_POLL_NO_HOPP,              // Rom block address.
                        BLOC_POLL_NO_HOPP_SIZE,         // Rom block size.
                        l1s.afrm,                       // Start with current frame.
                        l1s.mftab.frmlst);              // Proceed on MFTAB.

        l1s.task_status[POLL].current_status = ACTIVE;

        // Load FRAME_COUNT with the POLL Rom block size.
        if(l1s.frame_count < BLOC_POLL_NO_HOPP_SIZE)
          l1s.frame_count = BLOC_POLL_NO_HOPP_SIZE;
      }
    }
  }
#endif

}

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_execute_frame()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_execute_frame()
{
  // Execute functions from MFTAB.
  l1s_exec_mftab();

  // Force time_to_next_l1s_task to 0.
  // This statement has been introduced to force L1S to
  // reschedule the next activity when the current activity
  // is completed.
  if(l1s.frame_count == 1) l1a_l1s_com.time_to_next_l1s_task = 0;

  // Decrement the actual FRAME_COUNT.
  if(l1s.frame_count > 0) l1s.frame_count--;

  // Decrement the actual meas_forbidden counter.
  if(l1s.forbid_meas > 0) l1s.forbid_meas--;
}

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1s_meas_manager()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is the measurement tasks manager.       */
/* The followings tasks are handled:                     */
/*                                                       */
/* FSMS_MEAS:                                            */
/* 1) Full list measurement in Cell Selection            */
/* The machine performs 1 valid measurement per carrier  */
/* from the full list of GSM carriers. To achieve 1      */
/* valid measurement, 2 attempt with 2 different AGC     */
/* are performed worst case. When all carriers are       */
/* a reporting message L1C_VALID_MEAS_INFO is built and */
/* sent to L1A.                                          */
/*                                                       */
/* 2) Full list measurement in Idle mode.                */
/* The machine performs 1 valid measurement per carrier  */
/* from the full list of GSM carriers. To achieve 1      */
/* valid measurement, 2 attempt with 2 different AGC     */
/* are performed worst case. When all carriers are       */
/* a reporting message L1C_VALID_MEAS_INFO is built and */
/* sent to L1A.                                          */
/*                                                       */
/* I_BAMS_MEAS: BA list measurement in Idle mode.        */
/* The machine performs 8 measurements per PCH reading   */
/* (4*2) looping on the BA list. When 8 measurements are */
/* completed (end of PCH) a reporting message            */
/* L1C_RXLEV_PERIODIC_DONE is built and sent to L1A.      */
/*                                                       */
/*-------------------------------------------------------*/
void l1s_meas_manager(void)
{
  /* static static_s_rxlev_cntr = 0; */
  UWORD32  i;
  UWORD8   IL_for_rxlev;
  UWORD8   adc_active = INACTIVE;

  static xSignalHeaderRec *pch_msg          = NULL;
  static UWORD8           static_read_index = 0;
  static UWORD8           static_ctrl_index = 0;

  #if((RF_FAM == 61) && (CODE_VERSION != SIMULATION))
    UWORD16 dco_algo_ctl_pw = 0;
    UWORD16 dco_algo_ctl_pw_temp = 0;
    UWORD8 if_ctl = 0;
	UWORD8 if_threshold = 180;
  #endif
  #if(CODE_VERSION == SIMULATION)// This is a temp fix- pl Verify the RF family for L1 simulator and change
    UWORD16 dco_algo_ctl_pw = 0;
    UWORD16 dco_algo_ctl_pw_temp = 0;
    UWORD8 if_ctl = 0;
	UWORD8 if_threshold = 180;
  #endif

  #if FF_L1_IT_DSP_USF
    // Bypass Circuit switched measurment during Packet Access phase:
    // 1) FSMS_MEAS not active because full list measurement not allowed in
    // this state (see S921 Annex C "Transisition rules").
    // 2) I_BAMS_MEAS inactive because PCCCH idle therefore P_CRMS_MEAS is
    // used for neighbour monitoring.
    // 3) D_BAMS_MEAS is inactive because dedicated mode not compatible with
    // this state.
    // Running it induces side effects with Fast USF during PA because it
    // clears l1pa_l1ps_com.cr_freq_list.pnp_ctrl...

    // PA state detection with fast USF interrupt in use...
    if ((l1a_l1s_com.l1s_en_task[PALLC] == TASK_ENABLED)
         && (l1ps_macs_com.usf_status == USF_AWAITED))
      return;
  #endif
#if (FF_L1_FAST_DECODING == 1)
    if (l1a_apihisr_com.fast_decoding.deferred_control_req == TRUE)
    {
      /* Do not execute l1s_meas_manager if a fast decoding IT is scheduled */
      return;
    }
#endif /*#if (FF_L1_FAST_DECODING == 1)*/

  //====================================================
  //  RESET MEASUREMENT MACHINES WHEN SYNCHRO EXECUTED.
  //====================================================

  if(l1s.tpu_ctrl_reg & CTRL_SYNC)
  // SYNCHRO task has been controled, anything else is forbidden!!!
  // -> Reset FULL SET and BA LIST measurement machines.
  // -> return.
  {
    // Rem:
    // SYNCHRO task affects Idle FSMS_MEAS task since 1 frame is skipped.
    // Idle FSMS_MEAS session is restarted from scratch.

    // Reset Idle mode FULL LIST measurement machine.
    {
      // Init power measurement multi_session process
      l1a_l1s_com.full_list.meas_1st_pass_ctrl   = 1;     // Set 1st pass flag for power measurement session in ctrl.
      l1a_l1s_com.full_list.meas_1st_pass_read   = 1;     // Set 1st pass flag for power measurement session in read.
      l1a_l1s_com.full_list.nbr_sat_carrier_ctrl = 0;     // Clear number of saturated carrier in ctrl.
      l1a_l1s_com.full_list.nbr_sat_carrier_read = 0;     // Clear number of saturated carrier in read.

      l1a_l1s_com.full_list.ms_ctrl              = 0; //nbr of meas.controled.
      l1a_l1s_com.full_list.ms_ctrl_d            = 0; // ... 1 frame delay.
      l1a_l1s_com.full_list.ms_ctrl_dd           = 0; // ... 2 frames delay.

      // Set global parameters for full list measurement.
      l1a_l1s_com.full_list.next_to_ctrl = 0;
      l1a_l1s_com.full_list.next_to_read = 0;
    }

    // Reset BA LIST measurement machine.
    {
      // Rewind "next_to_ctrl" counter to come back to the next carrier to
      // measure.
      l1a_l1s_com.ba_list.next_to_ctrl = l1a_l1s_com.ba_list.next_to_read;

      // Reset flags.
      l1a_l1s_com.ba_list.ms_ctrl      = 0;
      l1a_l1s_com.ba_list.ms_ctrl_d    = 0;
      l1a_l1s_com.ba_list.ms_ctrl_dd   = 0;

      // Reset serving cell dedicated mode measurement session.
      l1s_reset_dedic_serving_meas();
    }

    return;
  }

  if(l1s.dsp_ctrl_reg & CTRL_ABORT)
  // A task conflict has happened, ABORT has been controled reseting
  // the MCU/DSP communication. We must rewind any measurement activity.
  {
    // FULL LIST measurement machine.
    {
      // Init power measurement multi_session process
      l1a_l1s_com.full_list.meas_1st_pass_ctrl   = l1a_l1s_com.full_list.meas_1st_pass_read;
      l1a_l1s_com.full_list.nbr_sat_carrier_ctrl = l1a_l1s_com.full_list.nbr_sat_carrier_read;     // Clear number of saturated carrier in ctrl.

      l1a_l1s_com.full_list.ms_ctrl              = 0; //nbr of meas.controled.
      l1a_l1s_com.full_list.ms_ctrl_d            = 0; // ... 1 frame delay.
      l1a_l1s_com.full_list.ms_ctrl_dd           = 0; // ... 2 frames delay.

      // Set global parameters for full list measurement.
      l1a_l1s_com.full_list.next_to_ctrl = l1a_l1s_com.full_list.next_to_read;
    }

    return;
  }

  //====================================================
  //  FULL LIST...
  //  -> Cell Selection  or,
  //  -> Idle mode.
  //====================================================

  // Clear semaphore when all running meas. are completed...
  if(!l1a_l1s_com.full_list.ms_ctrl_d && !l1a_l1s_com.full_list.ms_ctrl_dd)
  {
    l1a_l1s_com.meas_param &= FSMS_MEAS_MASK;
  }

  // When a READ is performed we set dsp_r_page_used flag to switch the read page...
    //if(l1a_l1s_com.full_list.ms_ctrl_dd) l1s_dsp_com.dsp_r_page_used = TRUE;

  // Call Cell Selection measurement management function or Idle PLMN permitted function
  // if meas. task still enabled.
  if((l1a_l1s_com.l1s_en_meas & FSMS_MEAS) && !(l1a_l1s_com.meas_param & FSMS_MEAS))
  {
    #if L1_GPRS
      UWORD8   pm_read[NB_MEAS_MAX_GPRS];
    #else
      UWORD8   pm_read[NB_MEAS_MAX];
    #endif

    UWORD8    nbmeas, max_nbmeas;

    // When FULL LIST measurement task is enabled L1S is executed every frame.
    l1a_l1s_com.time_to_next_l1s_task = 0;

    // the first PW window
    if ((l1a_l1s_com.full_list.next_to_ctrl ==0 ) &&(l1a_l1s_com.full_list.next_to_read ==0))
    {
      // ADC measurement
      // ***************
      if ((l1a_l1s_com.mode == CS_MODE) || (l1a_l1s_com.mode == CS_MODE0)) // only in cell selection and inside the first window
      {
         // ADC performed only with the 1st PW window
         if (l1a_l1s_com.adc_mode & ADC_NEXT_MEAS_SESSION)  // perform ADC only one time
         {
            adc_active = ACTIVE;
            l1a_l1s_com.adc_mode &= ADC_MASK_RESET_IDLE; // reset in order to have only one ADC measurement in Idle
         }
         else
           if (l1a_l1s_com.adc_mode & ADC_EACH_MEAS_SESSION) // perform ADC on each bloc
             adc_active = ACTIVE;
       }
    }

    // **********************
    // READ task if needed!!!
    // **********************

    if(l1a_l1s_com.full_list.ms_ctrl_dd)
      l1_check_com_mismatch(FULL_LIST_MEAS_ID);

    //A measure was control two TDMA earlier. Read ms_ctrl_dd number of  measures.
    #if L1_GPRS
      // !!! WARNING: word32 type is for compatibility with chipset == 0.
      // Can be word16 if only chipset == 2 is used. Extraction of pm using
      // AND operator can be removed.
      // Read power measurement result from DSP.

      switch(l1a_l1s_com.dsp_scheduler_mode)
      {
        // MCU/DSP interface is GSM one
        case GSM_SCHEDULER:
          // Read power measurement result from DSP.
          l1ddsp_meas_read(l1a_l1s_com.full_list.ms_ctrl_dd,pm_read);
        break;

        // MCU/DSP interface is GPRS one
        case GPRS_SCHEDULER:
          // Read power measurement result from DSP.
          l1pddsp_meas_read(l1a_l1s_com.full_list.ms_ctrl_dd,pm_read);
        break;
      }
    #else
      // Read power measurement result from DSP.
      l1ddsp_meas_read(l1a_l1s_com.full_list.ms_ctrl_dd,pm_read);
    #endif

    // When a READ is performed we set dsp_r_page_used flag to switch the read page...
    if(l1a_l1s_com.full_list.ms_ctrl_dd) l1s_dsp_com.dsp_r_page_used = TRUE;


    for(i=0; i < l1a_l1s_com.full_list.ms_ctrl_dd; i++)
    // Background measurements....
    // A measurement controle was performed 2 tdma earlier, read result now!!
    {

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
        trace_fct(CST_READ_FULL_LIST_MEAS, (UWORD32)(-1));//OMAPS00090550
      #endif

      l1_check_pm_error(pm_read[i], FULL_LIST_MEAS_ID);

      #if (TRACE_TYPE==3)
        stats_samples_pm(pm_read[i]);
      #endif

      // If we are running 2nd pass (because of saturated carrier during 1st pass),
      // we read radio_freq until we found the next one which is flagged saturated.
      if((!l1a_l1s_com.full_list.meas_1st_pass_read) &&
         ( l1a_l1s_com.full_list.nbr_sat_carrier_read!=0))
      {
        while(l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read]==0)
          l1a_l1s_com.full_list.next_to_read++;  // increase carrier index until a saturated one
                                                 // is found.

        l1a_l1s_com.full_list.nbr_sat_carrier_read--;
      }

      // Test meas value and validate or not the measurement.
      // Fill accordingly input_level and sat_flag fields.

      // L1_FF_MULTIBAND TBD
      IL_for_rxlev = l1ctl_csgc((UWORD8)(pm_read[i]),l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_read].radio_freq);

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_FULL_LIST_MEAS(pm_read[i], IL_for_rxlev, FULL_LIST_MEAS_ID, l1a_l1s_com.full_list.next_to_read)
      #endif

      // Accumulate only valid results (no saturated carriers)
      if (l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read]==0)
      {
        // Fill result "message" (array passed by L3 is directly filled by L1S).
        #if TESTMODE
          if (l1_config.TestMode)
          {
            // L1_FF_MULTIBAND TBD
            l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_read].accum_power_result =
            IL_for_rxlev;
          }
          else
          {
           // L1_FF_MULTIBAND TBD
           l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_read].accum_power_result +=
            l1s_encode_rxlev(IL_for_rxlev);
          }
        #else
          // L1_FF_MULTIBAND TBD
          l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_read].accum_power_result +=
          l1s_encode_rxlev(IL_for_rxlev);
        #endif
      }

      #if L2_L3_SIMUL
        #if (DEBUG_TRACE == BUFFER_TRACE_POWER)
          buffer_trace( 4, l1a_l1s_com.full_list.next_to_read+l1_config.std.radio_freq_index_offset,
                           pm_read[i],
                           l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read], 0);
        #endif
      #endif


      // Increment "next_to_read" field for next measurement...
      if(++l1a_l1s_com.full_list.next_to_read >= l1a_l1s_com.full_list_ptr->power_array_size)
      {
        l1a_l1s_com.full_list.next_to_read       = 0;
        l1a_l1s_com.full_list.meas_1st_pass_read = 0;
      }
    }// end of for (READ)


    // **********************
    // CTRL task if needed!!!
    // **********************

    // We can make a measurement on any frame excepted the frame
    // used to execute tasks not compatible with full list measurement.
    // (FBNEW,SB2,...
    #if L1_GPRS
      if(((l1pa_l1ps_com.cr_freq_list.ms_ctrl_d == 0) &&
          (l1a_l1s_com.ba_list.np_ctrl == 0)) &&
          (l1s.forbid_meas < 2))
    #else
      if((l1a_l1s_com.ba_list.np_ctrl == 0) &&
         (l1s.forbid_meas < 2))
    #endif
    {
      if(!l1a_l1s_com.full_list.meas_1st_pass_ctrl) // 2nd pass
      {
        if(l1a_l1s_com.full_list.nbr_sat_carrier_ctrl!=0) // there are still saturated carriers
        {
          WORD16  tpu_win_rest;
          UWORD16 power_meas_split;

          // Compute how many BP_SPLIT remains for full list meas.
          // Rem: we take into account the SYNTH load for 1st RX in next frame.
          tpu_win_rest = FRAME_SPLIT - l1s.tpu_win;

          power_meas_split = (l1_config.params.rx_synth_load_split + PWR_LOAD);

          max_nbmeas = 0;

          while(tpu_win_rest >= power_meas_split)
          {
            max_nbmeas ++;
            tpu_win_rest -= power_meas_split;
          }

          if(l1a_l1s_com.full_list.nbr_sat_carrier_ctrl >= max_nbmeas ) nbmeas = max_nbmeas;
          else nbmeas = l1a_l1s_com.full_list.nbr_sat_carrier_ctrl;

          #if L1_GPRS
            switch(l1a_l1s_com.dsp_scheduler_mode)
            {
              // MCU/DSP interface is GSM one
              case GSM_SCHEDULER:
              {
                // GSM scheduler can perform 8/4 meas. per TDMA depending on DSP code
                if(nbmeas > NB_MEAS_MAX) nbmeas = NB_MEAS_MAX;

                // Program DSP to make nbmeas neighbor measurments.
                // DSP code 33 is the only one to support more than 4PM (up to 8PM)
                #if (DSP != 33) && (DSP != 34) && (DSP != 35) && (DSP != 36) && (DSP != 37) && (DSP != 38) && (DSP != 39)
                  if(l1s.tpu_win) // check whether NB scheduled in same frame
                  {
                    if (nbmeas > NB_MEAS_MAX-1) // MCU-DSP I/F only supports 1NB + 3PM or 4PM
                      nbmeas = NB_MEAS_MAX-1;   // TPU RAM needs to be checked, too !!!
                  }
                  l1ddsp_load_monit_task(nbmeas,0);
                #else
                  // For activation of more than 4PM, DSP checks the bit field 0x200 (1PM correspond to 0x201)
                  l1ddsp_load_monit_task(nbmeas+0x200, 0);
                #endif
              }
              break;

              // MCU/DSP interface is GPRS one
              case GPRS_SCHEDULER:
              {
                // GPRS scheduler can perform 8 meas. per TDMA max.
                if (nbmeas > NB_MEAS_MAX_GPRS) nbmeas = NB_MEAS_MAX_GPRS;

                // Program DSP to make nbmeas neighbor measurments.
                // Note: At this level, l1s.tpu_win is considered to be
                // equal to 1 or 2.
                if(l1s.tpu_win)
                  l1pddsp_meas_ctrl(nbmeas, 1);
                else
                  l1pddsp_meas_ctrl(nbmeas, 0);
              }
              break;
            }
          #else
            // GSM scheduler can perform 8/4 meas. per TDMA depending on DSP code
            if(nbmeas > NB_MEAS_MAX) nbmeas = NB_MEAS_MAX;

            // Program DSP to make nbmeas neighbor measurments.
            // DSP code 33 is the only one to support more than 4PM (up to 8PM)
            #if (DSP != 33) && (DSP != 34) && (DSP != 35) && (DSP != 36) && (DSP != 37) && (DSP != 38) && (DSP != 39)
              if(l1s.tpu_win) // check whether NB scheduled in same frame
              {
                if (nbmeas > NB_MEAS_MAX-1) // MCU-DSP I/F only supports 1NB + 3PM or 4PM
                  nbmeas = NB_MEAS_MAX-1;   // TPU RAM needs to be checked, too !!!
              }
              l1ddsp_load_monit_task(nbmeas,0);
            #else
              // For activation of more than 4PM, DSP checks the bit field 0x200 (1PM correspond to 0x201)
              l1ddsp_load_monit_task(nbmeas+0x200, 0);
            #endif
          #endif

          // for each meas. do TPU control.
          for ( i = 0; i<nbmeas; i++)
          {
            while(l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_ctrl]==0)
              l1a_l1s_com.full_list.next_to_ctrl++; // increase carrier index until a saturated one
                                                    // is found.

            // Decrement the number of sat carriers.
            l1a_l1s_com.full_list.nbr_sat_carrier_ctrl--;

            #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
              trace_fct(CST_CTRL_FULL_LIST_MEAS, (UWORD32)(-1));//OMAPS00090550
            #endif

#if(RF_FAM == 61)   // Locosto DCO
           #if (PWMEAS_IF_MODE_FORCE == 0)
              cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_INVALID ,
                  0,
                  ( l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_ctrl].radio_freq),if_threshold);
            #else
              if_ctl = IF_120KHZ_DSP;
              dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
            #endif


            dco_algo_ctl_pw |= ((dco_algo_ctl_pw_temp & 0x03)<< (i*2));

#endif

            // tpu pgm: 1 measurement only.
            // L1_FF_MULTIBAND TBD
            l1dtpu_meas(l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_ctrl].radio_freq,
                        l1_config.params.low_agc,
                        0,
                        l1s.tpu_win,
                        l1s.tpu_offset,
                        adc_active
#if(RF_FAM == 61)
                ,L1_AFC_SCRIPT_MODE
                ,if_ctl
#endif
			);

            // only one ADC: with the first window
            adc_active = INACTIVE;


            // Increment tpu window identifier.
            l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

            // increment carrier counter for next measurement...
            if(++l1a_l1s_com.full_list.next_to_ctrl >= l1a_l1s_com.full_list_ptr->power_array_size)
              l1a_l1s_com.full_list.next_to_ctrl = 0;
          }
  #if(RF_FAM == 61)
          l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
  #endif
          l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


          // Flag measurement control.
          // **************************

          // Set nbr of meas. programmed  "ms_ctrl" to nbmeas.
          // It will be used as 2 tdma delayed to
          // trigger the reading.
          l1a_l1s_com.full_list.ms_ctrl = nbmeas;

          // Flag DSP and TPU programmation.
          // ********************************

          // Set "CTRL_MS" flag in the controle flag register.
          l1s.tpu_ctrl_reg |= CTRL_MS;
          l1s.dsp_ctrl_reg |= CTRL_MS;

        }
      }
      else // 1st pass
      {
        WORD16  tpu_win_rest;
        UWORD16 power_meas_split;

        // Compute how many BP_SPLIT remains for full list meas.
        // Rem: we take into account the SYNTH load for 1st RX in next frame.
        tpu_win_rest = FRAME_SPLIT - l1s.tpu_win;

        power_meas_split = (l1_config.params.rx_synth_load_split + PWR_LOAD);

        max_nbmeas = 0;

        while(tpu_win_rest >= power_meas_split)
        {
          max_nbmeas ++;
          tpu_win_rest -= power_meas_split;
        }

        i = l1a_l1s_com.full_list_ptr->power_array_size - l1a_l1s_com.full_list.next_to_ctrl;

        if( i >= max_nbmeas ) nbmeas = max_nbmeas;
        else                  nbmeas = i;

          #if L1_GPRS
            switch(l1a_l1s_com.dsp_scheduler_mode)
            {
              // MCU/DSP interface is GSM one
              case GSM_SCHEDULER:
              {
                // GSM scheduler can perform 8/4 meas. per TDMA depending on DSP code
                if(nbmeas > NB_MEAS_MAX) nbmeas = NB_MEAS_MAX;

                // Program DSP to make nbmeas neighbor measurments.
                // DSP code 33 is the only one to support more than 4PM (up to 8PM)
                #if (DSP != 33) && (DSP != 34) && (DSP != 35) && (DSP != 36) && (DSP != 37) && (DSP != 38) && (DSP != 39)
                  if(l1s.tpu_win) // check whether NB scheduled in same frame
                  {
                    if (nbmeas > NB_MEAS_MAX-1) // MCU-DSP I/F only supports 1NB + 3PM or 4PM
                      nbmeas = NB_MEAS_MAX-1;   // TPU RAM needs to be checked, too !!!
                  }
                  l1ddsp_load_monit_task(nbmeas,0);
                #else
                  // For activation of more than 4PM, DSP checks the bit field 0x200 (1PM correspond to 0x201)
                  l1ddsp_load_monit_task(nbmeas+0x200, 0);
                #endif
              }
              break;

              // MCU/DSP interface is GPRS one
              case GPRS_SCHEDULER:
              {
                // GPRS scheduler can perform 8 meas. per TDMA max.
                if (nbmeas > NB_MEAS_MAX_GPRS) nbmeas = NB_MEAS_MAX_GPRS;

                // Program DSP to make nbmeas neighbor measurments.
                // Note: At this level, l1s.tpu_win is considered to be
                // equal to 1 or 2.
                if(l1s.tpu_win)
                  l1pddsp_meas_ctrl(nbmeas, 1);
                else
                  l1pddsp_meas_ctrl(nbmeas, 0);
              }
              break;
            }
          #else
            // GSM scheduler can perform 8/4 meas. per TDMA depending on DSP code
            if(nbmeas > NB_MEAS_MAX) nbmeas = NB_MEAS_MAX;

            // Program DSP to make nbmeas neighbor measurments.
            // DSP code 33 is the only one to support more than 4PM (up to 8PM)
            #if (DSP != 33) && (DSP != 34) && (DSP != 35) && (DSP != 36) && (DSP != 37) && (DSP != 38) && (DSP !=39)
              if(l1s.tpu_win) // check whether NB scheduled in same frame
              {
                if (nbmeas > NB_MEAS_MAX-1) // MCU-DSP I/F only supports 1NB + 3PM or 4PM
                  nbmeas = NB_MEAS_MAX-1;   // TPU RAM needs to be checked, too !!!
              }
              l1ddsp_load_monit_task(nbmeas,0);
            #else
              // For activation of more than 4PM, DSP checks the bit field 0x200 (1PM correspond to 0x201)
              l1ddsp_load_monit_task(nbmeas+0x200, 0);
            #endif
          #endif

        // for each meas. do TPU control.
        for (i=0; i<nbmeas; i++)
        {
#if(RF_FAM == 61)   // Locosto DCO
        #if (PWMEAS_IF_MODE_FORCE == 0)
            cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_INVALID ,
                0,
                l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_ctrl].radio_freq,if_threshold);
          #else
            if_ctl = IF_120KHZ_DSP;
            dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
          #endif

            dco_algo_ctl_pw |= ((dco_algo_ctl_pw_temp & 0x03)<< (i*2)) ;
#endif

          // tpu pgm: 1 measurement only.
          // L1_FF_MULTIBAND TBD
          l1dtpu_meas(l1a_l1s_com.full_list_ptr->power_array[l1a_l1s_com.full_list.next_to_ctrl].radio_freq,
                      l1_config.params.high_agc,
                      0,
                      l1s.tpu_win,
                      l1s.tpu_offset,adc_active
                   #if(RF_FAM == 61)
                       ,L1_AFC_SCRIPT_MODE
                       ,if_ctl
                   #endif
		  );

            // only one ADC: with the first window
            adc_active = INACTIVE;


          // increment carrier counter for next measurement...
          if(++l1a_l1s_com.full_list.next_to_ctrl >= l1a_l1s_com.full_list_ptr->power_array_size)
          {
            l1a_l1s_com.full_list.next_to_ctrl       = 0; // Go back to the top of the list.
            l1a_l1s_com.full_list.meas_1st_pass_ctrl = 0; // End of 1st pass.
          }

          // Increment tpu window identifier.
          l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);
        }
  #if(RF_FAM == 61)
          l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
  #endif

        l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


        // Flag measurement control.
        // **************************

        // Set nbr of meas. programmed  "ms_ctrl" to nbmeas.
        // It will be used as 2 tdma delayed to
        // trigger the reading.
        l1a_l1s_com.full_list.ms_ctrl = nbmeas;

        // Flag DSP and TPU programmation.
        // ********************************

        // Set "CTRL_MS" flag in the controle flag register.
        l1s.tpu_ctrl_reg |= CTRL_MS;
        l1s.dsp_ctrl_reg |= CTRL_MS;
      }
    }


    //Time to make reporting.......
    if((!l1a_l1s_com.full_list.meas_1st_pass_read) &&
       ( l1a_l1s_com.full_list.nbr_sat_carrier_read == 0))
    {
      xSignalHeaderRec *fl_msg;

      /*-----------------------------------------------*/
      /* Time to report (1 valid measurement has       */
      /* been performed on each carrier) if:           */
      /*   Cell Selection or PLMN selection, and       */
      /*   The 1st pass has been completed,  and       */
      /*   No more carrier saturated to control, and   */
      /*   No measurement in the pipeline.             */
      /*-----------------------------------------------*/
        // Reset the FSMS_MEAS  process to avoid to keep sending
        // report message to L1A.
        l1a_l1s_com.l1s_en_meas &= FSMS_MEAS_MASK; // Clear Cell Selection/Idle Full List Measurement enable flag.

        // alloc L1C_VALID_MEAS_INFO message...
        fl_msg = os_alloc_sig(sizeof(int));
        DEBUGMSG(status,NU_ALLOC_ERR)
        fl_msg->SignalCode = L1C_VALID_MEAS_INFO;
        // L1_FF_MULTIBAND TBD
        fl_msg->SigP= (void *) l1a_l1s_com.full_list_ptr;

        // send L1C_VALID_MEAS_INFO message...
        os_send_sig(fl_msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

    }//end of reporting
  }// end of FSMS_MEAS


  //====================================================
  //  BA LIST...
  //  -> Idle mode.
  //====================================================

  // When a READ is performed we usueally set "dsp_r_page_used" flag to switch the
  // read page but since I_BAMS is executed in the same frame as Normal Paging (NP),
  // the setting of "dsp_r_page_used" is already done in READ(NP).

  if((l1a_l1s_com.l1s_en_meas & I_BAMS_MEAS)  && (l1a_l1s_com.meas_param & I_BAMS_MEAS))
  // Some changes occured on the BA list or the PAGING PARAMETERS have
  // changed.
  {
    // Reset BA semaphore.
    l1a_l1s_com.meas_param &= I_BAMS_MEAS_MASK;

    /* pch_msg != NULL added below due to the fast pagin feature + power reduction feature 
     * as the measurement can end in potentially 2 TDMA frames itself and an IBA_R message
     * when comes in certail TDMA frames of paging task, both static ctrl index and static
     * read index will be zero */
    /* FreeCalypso TCS211 reconstruction: above change reverted */
    if((static_ctrl_index != 0) || (static_read_index != 0))
    {
   
      // Paging process has been interrupted by a L3 message
      // Deallocate memory for the received message if msg not forwarded to L3.
      // ----------------------------------------------------------------------
      #if (GSM_IDLE_RAM != 1)
        os_free_sig(pch_msg);
        DEBUGMSG(status,NU_DEALLOC_ERR)
      #endif

      pch_msg           = NULL;
      static_ctrl_index = 0;
      static_read_index = 0;

      // Rewind ba counters to come back to the first carrier of this
      // aborted session.
      l1a_l1s_com.ba_list.next_to_read = l1a_l1s_com.ba_list.first_index;
      l1a_l1s_com.ba_list.next_to_ctrl = l1a_l1s_com.ba_list.first_index;

      // Reset flags.
      l1a_l1s_com.ba_list.ms_ctrl      = FALSE;
      l1a_l1s_com.ba_list.ms_ctrl_d    = FALSE;
      l1a_l1s_com.ba_list.ms_ctrl_dd   = FALSE;
    }
  }
  else
  if ((l1a_l1s_com.l1s_en_meas & I_BAMS_MEAS) && !(l1a_l1s_com.meas_param & I_BAMS_MEAS))
  // Idle Neighbor Cells Power Measurements fonction if meas. task still enabled.
  {
    UWORD8   nbr_carrier = l1a_l1s_com.ba_list.nbr_carrier;

    // variables introduced to cope with RACH sent on one frame of the paging block
    static UWORD8 static_nbmeas_to_report = 8;
    static UWORD8 static_nbmeas_ctrl_d  = 0;
    static UWORD8 static_nbmeas_ctrl_dd = 0;

#if 0	/* FreeCalypso TCS211 reconstruction */
    static UWORD8 num_pm[4]={0,0,0,0};
#if (FF_L1_FAST_DECODING == 1)
    static UWORD8 num_pm_fp[2]={0,0};
#endif
    static UWORD8 num_pm_frames = 0; /* number of frames over which measurement is scheduled */
#endif

    UWORD8   nbmeas_ctrl = 0;
#if (FF_L1_FAST_DECODING == 1)
    BOOL schedule_measures = FALSE;
    BOOL fast_decoding = l1s_check_fast_decoding_authorized(NP);
#endif /* FF_L1_FAST_DECODING */
    // ********************
    // READ task if needed
    // ********************
    if(l1a_l1s_com.ba_list.ms_ctrl_dd == TRUE)
    // Background measurements....
    // A measurement controle (set in "l1s_ctrl_ms()" function) was performed
    // 2 tdma earlier, read result now!!
    {
      UWORD16  radio_freq_read;
      UWORD8   ba_index_read;


      l1_check_com_mismatch(I_BA_MEAS_ID);

      for(i=0; i<static_nbmeas_ctrl_dd; i++)
      {
        UWORD32  pm;

        #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
          trace_fct(CST_READ_I_BA_MEAS, (UWORD32)(-1));//OMAPS00090550
        #endif

        // Read power measurement result from DSP.
        pm = (l1s_dsp_com.dsp_db_r_ptr->a_pm[i] & 0xffff) >> 5;
        l1_check_pm_error(pm, I_BA_MEAS_ID);

        #if (TRACE_TYPE==3)
          stats_samples_pm(pm);
        #endif

        ba_index_read = l1a_l1s_com.ba_list.next_to_read;
        radio_freq_read    = l1a_l1s_com.ba_list.A[ba_index_read].radio_freq;

        // Get Input level corresponding to the used IL and pm result.
        IL_for_rxlev = l1ctl_pgc((UWORD8)pm, l1a_l1s_com.ba_list.used_il_dd[i], l1a_l1s_com.ba_list.used_lna_dd[i],
                                 radio_freq_read);

        #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
          RTTL1_FILL_MON_MEAS(pm, IL_for_rxlev, I_BA_MEAS_ID, radio_freq_read)
        #endif


#if 0	/* FreeCalypso TCS211 reconstruction */
	//Check if the message is not empty, else allocate memory
	if (pch_msg == NULL)
        {
          pch_msg = os_alloc_sig(sizeof(T_L1C_RXLEV_PERIODIC_DONE));
          DEBUGMSG(status,NU_ALLOC_ERR)
          pch_msg->SignalCode = L1C_RXLEV_PERIODIC_DONE;
        }
#endif

        // Fill reporting message.
        ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->
          A[static_read_index].radio_freq_no = radio_freq_read;
        ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->
          A[static_read_index].rxlev = l1s_encode_rxlev(IL_for_rxlev);

        // Increment the number of neighbor meas read.
        static_read_index ++;

        // Increment "l1s.next_to_read" field for next measurement...
        if(++l1a_l1s_com.ba_list.next_to_read >= nbr_carrier)
          l1a_l1s_com.ba_list.next_to_read   = 0;

      }//end for

      // Serving cell measurements...
      // Accumulate the new measurement with the partial result.
      // Compensate AGC for current measurement value.
      l1a_l1s_com.Scell_info.meas.acc += l1a_l1s_com.Scell_IL_for_rxlev;
      /* static_s_rxlev_cntr++; */

      // **********
      // Reporting
      // **********
      if (static_read_index==static_nbmeas_to_report)
      {

      #if (GSM_IDLE_RAM == 1)
        if (!READ_TRAFFIC_CONT_STATE)
        {
          CSMI_TrafficControllerOn();
        }
        // 1st paging block, so it's time to allocate L1C_RXLEV_PERIODIC_IND msg
        if (pch_msg == NULL)
        {
          pch_msg = os_alloc_sig(sizeof(T_L1C_RXLEV_PERIODIC_DONE));
          DEBUGMSG(status,NU_ALLOC_ERR)
          pch_msg->SignalCode = L1C_RXLEV_PERIODIC_DONE;
        }

        for(i=0; i<static_nbmeas_to_report; i++)
        {
            ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->A[i].radio_freq_no = l1s.A[i].radio_freq_no;
            ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->A[i].rxlev = l1s.A[i].rxlev;
            // Fill reporting message.
        }
      #endif

        static_read_index = 0;

        // Fill serving cell RXLEV field.
        //#if (FF_L1_FAST_DECODING == 1)
          /* Reporting done after the 2nd NP burst, bursts 3 and 4 are unknown */
          //if (fast_decoding == TRUE)
          //  ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->s_rxlev = l1s_encode_rxlev(l1a_l1s_com.Scell_info.meas.acc/2);
          //else
          //  ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->s_rxlev = l1s_encode_rxlev(l1a_l1s_com.Scell_info.meas.acc/4);
//#else /* #if (FF_L1_FAST_DECODING == 1) */
//          ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->s_rxlev = l1s_encode_rxlev(l1a_l1s_com.Scell_info.meas.acc/4);
//#endif  /* #if (FF_L1_FAST_DECODING == 1) #else */

((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->s_rxlev = l1s_encode_rxlev(l1a_l1s_com.Scell_info.meas.acc/4);
        // Fill "nbr_of_carriers" field, it is 7 when a RACH coincides with paging block, 8 otherwise.
        ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->nbr_of_carriers = static_nbmeas_to_report;

        // Fill BA identifier field.
        ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->ba_id = l1a_l1s_com.ba_list.ba_id;
// Enhanced RSSI

	#if 0	/* FreeCalypso TCS211 reconstruction */
         ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->qual_acc_idle =qual_acc_idle1[0] ;

         ((T_L1C_RXLEV_PERIODIC_DONE*)(pch_msg->SigP))->qual_nbr_meas_idle =qual_acc_idle1[1]* TOTAL_NO_OF_BITS_IDLE_MEAS;
	#endif

        // send L1C_RXLEV_PERIODIC_IND message...
        os_send_sig(pch_msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

	#if 0	/* FreeCalypso TCS211 reconstruction */
       // Reseting the value
       qual_acc_idle1[0]= 0;
       qual_acc_idle1[1] =0;
	#endif

        // Reset pointer for debugg.
        pch_msg = NULL;
	/* static_s_rxlev_cntr = 0; */
      }

    }// end of READ

    // **********
    // CTRL task
    // **********
    if (l1a_l1s_com.ba_list.np_ctrl == 1)
    {
      #if (GSM_IDLE_RAM != 1)
      // 1st paging block, so it's time to allocate L1C_RXLEV_PERIODIC_IND msg
        if (pch_msg == NULL)
        {
          pch_msg = os_alloc_sig(sizeof(T_L1C_RXLEV_PERIODIC_DONE));
          DEBUGMSG(status,NU_ALLOC_ERR)
          pch_msg->SignalCode = L1C_RXLEV_PERIODIC_DONE;
	  /* static_s_rxlev_cntr = 0; */
        }
      #endif
      // Reset accumalator for serving measurements.
      l1a_l1s_com.Scell_info.meas.acc = 0;

      // Clear read period counter
      static_read_index = 0;

      // Save first BA index to be measured in this new session.
      l1a_l1s_com.ba_list.first_index = l1a_l1s_com.ba_list.next_to_ctrl;

      // Reset static variables for control of nbmeas per frame
      static_nbmeas_to_report = 8;
      static_nbmeas_ctrl_d  = 0;
      static_nbmeas_ctrl_dd = 0;
    }

    // A PCH burst has been controled, we must make the control of 1 or 2 new measurements.
    if ((static_ctrl_index == (l1a_l1s_com.ba_list.np_ctrl-1)*2) ||
        (static_ctrl_index == (l1a_l1s_com.ba_list.np_ctrl-1)*2 - 1))
    {
      UWORD16  radio_freq_ctrl;
      UWORD8   ba_index_ctrl;

      // check whether RACH has been controlled in the same frame
      // if YES only one PW measurement will be controlled and the number of meas to report is decremented by 1
      if (l1s.tpu_win >= (3 * BP_SPLIT + l1_config.params.tx_ra_load_split + l1_config.params.rx_synth_load_split))
      {
        static_nbmeas_to_report--;
        nbmeas_ctrl = 1;
      }
      else
      {
        nbmeas_ctrl = 2;
      }  /* end else no RACH */

      for(i=0; i<nbmeas_ctrl; i++)
      {
        UWORD8  lna_off;
        WORD32  agc;
#if (L1_FF_MULTIBAND == 1)
        UWORD16 operative_radio_freq;
#endif

        ba_index_ctrl = l1a_l1s_com.ba_list.next_to_ctrl;
        radio_freq_ctrl    = l1a_l1s_com.ba_list.A[ba_index_ctrl].radio_freq;

#if (L1_FF_MULTIBAND == 0)

        // Get AGC according to the last known IL.
        agc     = Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level >> 1, PWR_ID);
        lna_off = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;

        // Memorize the IL used for AGC setting.
        l1a_l1s_com.ba_list.used_il[i]  = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level;
        l1a_l1s_com.ba_list.used_lna[i] = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

        operative_radio_freq = 
            l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq_ctrl);

       lna_off = l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
        // Get AGC according to the last known IL.
        agc     = 
            Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, PWR_ID);

        // Memorize the IL used for AGC setting.
        l1a_l1s_com.ba_list.used_il[i]  = 
            l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
        l1a_l1s_com.ba_list.used_lna[i] = 
            l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;        

#endif // #if (L1_FF_MULTIBAND == 0) else


        #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
          trace_fct(CST_CTRL_I_BA_MEAS,(UWORD32)(-1));//OMAPS00090550
        #endif

#if(RF_FAM == 61)   // Locosto DCO
       #if (PWMEAS_IF_MODE_FORCE == 0)
           cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_VALID ,
              l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level,
              radio_freq_ctrl,if_threshold);
         #else
           if_ctl = IF_120KHZ_DSP;
           dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
         #endif

         dco_algo_ctl_pw |= ((dco_algo_ctl_pw_temp & 0x03)<< (i*2)) ;
#endif

        // tpu pgm: 1 measurement only.
        l1dtpu_meas(radio_freq_ctrl,
                    agc,
                    lna_off,
                    l1s.tpu_win,
                    l1s.tpu_offset, INACTIVE
                   #if(RF_FAM == 61)
                       ,L1_AFC_SCRIPT_MODE
                       ,if_ctl
                   #endif
		);


        // increment carrier counter for next measurement...
        if(++l1a_l1s_com.ba_list.next_to_ctrl >= nbr_carrier)
          l1a_l1s_com.ba_list.next_to_ctrl = 0;

        #if L2_L3_SIMUL
          #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
          buffer_trace(4, l1s.actual_time.fn, radio_freq,
                       l1s.tpu_win, 0);
          #endif
        #endif

        // Increment tpu window identifier.
        l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

        // Increment the number of neighbor meas controled.
        static_ctrl_index ++;
//        static_ctrl_index %=8;
        if (static_ctrl_index >= static_nbmeas_to_report)
          static_ctrl_index = 0;

      }
  #if(RF_FAM == 61)
          l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
  #endif

      l1ddsp_load_monit_task(nbmeas_ctrl, 0);


      l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


      // Flag measurement control.
      // **************************

      // Set flag "ms_ctrl" to 1. It will be used as 2 tdma delayed for
      // background measurement reading.
      l1a_l1s_com.ba_list.ms_ctrl = TRUE;

      // Flag DSP and TPU programmation.
      // ********************************

      // Set "CTRL_MS" flag in the control flag register.
      l1s.tpu_ctrl_reg |= CTRL_MS;
      l1s.dsp_ctrl_reg |= CTRL_MS;

    }//end ctrl

    // Pipeline for tracking of the number of measurements controlled
    static_nbmeas_ctrl_dd = static_nbmeas_ctrl_d;
    static_nbmeas_ctrl_d  = nbmeas_ctrl;

  }//end I_BAMS_MEAS


  //====================================================
  //  BA LIST...
  //  -> Dedicated mode.
  //====================================================

  // When a READ is performed we set dsp_r_page_used flag to switch the read page...
  if(l1a_l1s_com.ba_list.ms_ctrl_dd==TRUE) l1s_dsp_com.dsp_r_page_used = TRUE;

  if((l1a_l1s_com.l1s_en_meas & D_BAMS_MEAS) && (l1a_l1s_com.meas_param & D_BAMS_MEAS))
  // Some changes occured on the BA list or the Dedicated channel have
  // changed.
  {
    #if FF_L1_IT_DSP_DTX
      // Postpone rewind from DTX HISR to L1S to keep same behaviour
      if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
      {
    #endif
    if(l1a_l1s_com.ba_list.ms_ctrl_d == FALSE)
    // Reset BA semaphore.
    // Rem: BA semaphore is reset only if the pipeline ctrl
    // is empty.
    {
      l1a_l1s_com.meas_param &= D_BAMS_MEAS_MASK;
    }

    // Rewind "next_to_ctrl" counter to come back to the next carrier to
    // measure.
    l1a_l1s_com.ba_list.next_to_ctrl = l1a_l1s_com.ba_list.next_to_read;

    // Reset of "ms_ctrl, ms_ctrl_d, msctrl_dd" is done at L1 startup
    // and when SYNCHRO task is executed.
    #if FF_L1_IT_DSP_DTX
      } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
    #endif
  }
  else
  if ( (l1a_l1s_com.l1s_en_meas & D_BAMS_MEAS) &&
      !(l1a_l1s_com.meas_param  & D_BAMS_MEAS) &&
       (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED) )
  // Call Dedicated Neighbor Cells Power Measurements fonction
  // if meas. task still enabled and global dedicated mode task enabled.
  {
    UWORD8                 nbr_carrier = l1a_l1s_com.ba_list.nbr_carrier;
    T_CHANNEL_DESCRIPTION *desc_ptr    = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr;
    UWORD32                pm;
    UWORD8                 lna_off;
    WORD32                 agc;

    #if FF_L1_IT_DSP_DTX
      // Read operation to be done from L1S only
      if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
      {
    #endif
    // ********************
    // READ task if needed
    // ********************
    if(l1a_l1s_com.ba_list.ms_ctrl_dd == TRUE)
    // Background measurements....
    // A measurement controle (set in "l1s_ctrl_ms()" function) was performed
    // 2 tdma earlier, read result now!!
    {
      UWORD16 radio_freq_read;
      UWORD8  ba_index_read;

      l1_check_com_mismatch(D_BA_MEAS_ID);

      #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
        trace_fct(CST_READ_D_BA_MEAS, (UWORD32)(-1));//OMAPS00090550
      #endif

      // Read power measurement result from DSP.
      pm = (l1s_dsp_com.dsp_db_r_ptr->a_pm[0] & 0xffff) >> 5;
      l1_check_pm_error(pm, D_BA_MEAS_ID);

      #if (TRACE_TYPE==3)
        stats_samples_pm(pm);
      #endif

      ba_index_read = l1a_l1s_com.ba_list.next_to_read;
      radio_freq_read    = l1a_l1s_com.ba_list.A[ba_index_read].radio_freq;

      // Get Input level corresponding to the used IL and pm result.
      IL_for_rxlev = l1ctl_pgc((UWORD8)pm,l1a_l1s_com.ba_list.used_il_dd[0],l1a_l1s_com.ba_list.used_lna_dd[0],
                               radio_freq_read);

      #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
        RTTL1_FILL_MON_MEAS(pm, IL_for_rxlev, D_BA_MEAS_ID, radio_freq_read)
      #endif

      // Accumulate new RXLEV level in the BA list.
      l1a_l1s_com.ba_list.A[ba_index_read].acc += l1s_encode_rxlev(IL_for_rxlev);

      l1a_l1s_com.ba_list.A[ba_index_read].nbr_meas++;

      // Increment "l1s.next_to_read" field for next measurement...
      if(++l1a_l1s_com.ba_list.next_to_read >= nbr_carrier)
        l1a_l1s_com.ba_list.next_to_read = 0;
    }// end of READ

    #if FF_L1_IT_DSP_DTX
      } // if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
    #endif
    // **********
    // CTRL task
    // **********
    if((desc_ptr->channel_type == SDCCH_4) || (desc_ptr->channel_type == SDCCH_8))
    // case SDCCH...
    {
      UWORD8  ba_index_ctrl;
      UWORD16  radio_freq_ctrl;
#if (L1_FF_MULTIBAND == 1)
      UWORD8 operative_radio_freq;
#endif    


      if(l1s.forbid_meas < 2)
      // We must perform a measurement on every frame except when
      // FB51/SB51/SBCNF51 tasks are running, those tasks are not compatible
      // with neigh. measurement.
      {
          ba_index_ctrl = l1a_l1s_com.ba_list.next_to_ctrl;
          radio_freq_ctrl    = l1a_l1s_com.ba_list.A[ba_index_ctrl].radio_freq;

#if (L1_FF_MULTIBAND == 0)  

          agc     = Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level >> 1, PWR_ID);
          lna_off = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;


          // Store IL used for current CTRL in order to be able to build IL from pm
          // in READ phase.
          l1a_l1s_com.ba_list.used_il[0]  = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level;
          l1a_l1s_com.ba_list.used_lna[0] = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

          operative_radio_freq = 
            l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq_ctrl);

          lna_off = 
            l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
          agc     = 
            Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, PWR_ID);


          // Store IL used for current CTRL in order to be able to build IL from pm
          // in READ phase.
          l1a_l1s_com.ba_list.used_il[0]  = 
            l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
          l1a_l1s_com.ba_list.used_lna[0] = 
            l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;

#endif // #if (L1_FF_MULTIBAND == 0)   else

          #if (TRACE_TYPE!=0) && (TRACE_TYPE!=5)
            trace_fct(CST_CTRL_D_BA_MEAS,(UWORD32)( -1));//OMAPS00090550
          #endif

#if(RF_FAM == 61)   // Locosto DCO
       #if (PWMEAS_IF_MODE_FORCE == 0)
          cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw, &if_ctl, (UWORD8) L1_IL_VALID ,
             l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level,
             radio_freq_ctrl,if_threshold);
        #else
          if_ctl = IF_120KHZ_DSP;
          dco_algo_ctl_pw = DCO_IF_0KHZ;
        #endif

        l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
#endif

        // TPU pgm: 1 measurement only.
          l1dtpu_meas(radio_freq_ctrl,
                      agc,
                      lna_off,
                      l1s.tpu_win,
                      l1s.tpu_offset, INACTIVE
                   #if(RF_FAM == 61)
                       ,L1_AFC_SCRIPT_MODE
                       ,if_ctl
                   #endif
		  );

          // increment carrier counter for next measurement...
          if(++l1a_l1s_com.ba_list.next_to_ctrl >= nbr_carrier)
            l1a_l1s_com.ba_list.next_to_ctrl = 0;

          #if L2_L3_SIMUL
            #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
              buffer_trace(4, l1s.actual_time.fn, radio_freq,
                           l1s.tpu_win, 0);
            #endif
          #endif

          // Increment tpu window identifier.
          l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

          // DSP pgm: 1 measurement only.
          l1ddsp_load_monit_task(1, 0);


            l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


          // Flag measurement control.
          // **************************

          // Set flag "ms_ctrl" to 1. It will be used as 2 tdma delayed for
          // background measurement reading.
          l1a_l1s_com.ba_list.ms_ctrl = TRUE;

          // Flag DSP and TPU programmation.
          // ********************************

          // Set "CTRL_MS" flag in the controle flag register.
          l1s.tpu_ctrl_reg |= CTRL_MS;
          l1s.dsp_ctrl_reg |= CTRL_MS;
      }
    }
    else
    // case TCH...
    {
      UWORD8                 ba_index_ctrl;
      UWORD16                radio_freq_ctrl;
#if (L1_FF_MULTIBAND == 1)
      UWORD16 operative_radio_freq;
#endif
      
      //T_CHANNEL_DESCRIPTION *desc_ptr   = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr; -OMAPS-90550-new

      if(l1s.forbid_meas == 0)
      // We must perform a measurement on every frame except frames 23, 24 (TCH_HS SC0)
      //                                                            24, 25 (TCH_HS SC1 anf TCH_FS)
      // when FB26/SB26/SBCNF26 tasks are running, those task are not compatible
      // with neigh. measurement.
      {
        #if FF_L1_IT_DSP_DTX
          if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
          {
        #endif
          ba_index_ctrl = l1a_l1s_com.ba_list.next_to_ctrl;
          radio_freq_ctrl    = l1a_l1s_com.ba_list.A[ba_index_ctrl].radio_freq;

#if (L1_FF_MULTIBAND == 0)
          
          agc     = Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level >> 1, PWR_ID);
          lna_off = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;

          // Store IL used for current CTRL in order to be able to build IL from pm
          // in READ phase.
          l1a_l1s_com.ba_list.used_il[0]  = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level;
          l1a_l1s_com.ba_list.used_lna[0] = l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].lna_off;

#else // L1_FF_MULTIBAND = 1 below

          operative_radio_freq = 
            l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq_ctrl);
          lna_off = 
            l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;
          agc     = 
            Cust_get_agc_from_IL(radio_freq_ctrl, l1a_l1s_com.last_input_level[operative_radio_freq].input_level >> 1, PWR_ID);

          // Store IL used for current CTRL in order to be able to build IL from pm
          // in READ phase.
          l1a_l1s_com.ba_list.used_il[0]  = 
            l1a_l1s_com.last_input_level[operative_radio_freq].input_level;
          l1a_l1s_com.ba_list.used_lna[0] = 
            l1a_l1s_com.last_input_level[operative_radio_freq].lna_off;

#endif // #if (L1_FF_MULTIBAND == 0)


#if(RF_FAM == 61)   // Locosto DCO
       #if (PWMEAS_IF_MODE_FORCE == 0)
          cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw, &if_ctl, (UWORD8) L1_IL_VALID ,
              l1a_l1s_com.last_input_level[radio_freq_ctrl - l1_config.std.radio_freq_index_offset].input_level,
              radio_freq_ctrl,if_threshold);
        #else
          if_ctl = IF_120KHZ_DSP;
          dco_algo_ctl_pw = DCO_IF_0KHZ;
        #endif

        l1ddsp_load_dco_ctl_algo_pw(dco_algo_ctl_pw);
#endif

          // TPU pgm: 1 measurement only.
          l1dtpu_meas(radio_freq_ctrl,
                      agc,
                      lna_off,
                      l1s.tpu_win,
                      l1s.tpu_offset, INACTIVE
                   #if(RF_FAM == 61)
                       ,L1_AFC_SCRIPT_MODE
                       ,if_ctl
                   #endif
		  );

          // increment carrier counter for next measurement...
          if(++l1a_l1s_com.ba_list.next_to_ctrl >= nbr_carrier)
             l1a_l1s_com.ba_list.next_to_ctrl = 0;

          #if L2_L3_SIMUL
            #if (DEBUG_TRACE == BUFFER_TRACE_OFFSET_NEIGH)
               buffer_trace(4, l1s.actual_time.fn, radio_freq,
                            l1s.tpu_win, 0);
            #endif
          #endif
          // Increment tpu window identifier.
          l1s.tpu_win += (l1_config.params.rx_synth_load_split + PWR_LOAD);

          // DSP pgm: 1 measurement only.
          l1ddsp_load_monit_task(1, 0);


            l1s_dsp_com.dsp_db_w_ptr->d_debug = (l1s.debug_time + 2) ;


          // Flag measurement control.
          // **************************

          // Set flag "ms_ctrl" to 1. It will be used as 2 tdma delayed for
          // background measurement reading.
          l1a_l1s_com.ba_list.ms_ctrl = TRUE;

          // Flag DSP and TPU programmation.
          // ********************************

          // Set "CTRL_MS" flag in the controle flag register.
          l1s.tpu_ctrl_reg |= CTRL_MS;
          l1s.dsp_ctrl_reg |= CTRL_MS;
        #if FF_L1_IT_DSP_DTX
          } // if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
        #endif
    }
 }

#if FF_L1_IT_DSP_DTX
   if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
   {
#endif
    // Time to make reporting...
    //--------------------------
    if(l1s.next_time.fn_in_report == 0)
    {
      l1s_dedic_reporting();

      // Check if any NEW BA available, if so download it...
      if(l1a_l1s_com.ba_list.new_list_present == TRUE)
      {
        WORD32 i;

        // Set parameter synchro semaphore for D_BAMS task.
        // This is used to reject any measurement which could
        // be in the C/W/R pipeline.
        l1a_l1s_com.meas_param |= D_BAMS_MEAS;

        // Download new list.
        for(i=0;i<l1a_l1s_com.ba_list.new_list.num_of_chans;i++)
        {
          l1a_l1s_com.ba_list.A[i].radio_freq = l1a_l1s_com.ba_list.new_list.chan_list.A[i];
        }

        l1a_l1s_com.ba_list.ba_id               = l1a_l1s_com.ba_list.new_list.ba_id;
        l1a_l1s_com.ba_list.nbr_carrier         = l1a_l1s_com.ba_list.new_list.num_of_chans;
        l1a_l1s_com.dedic_set.pwrc              = l1a_l1s_com.ba_list.new_list.pwrc;
        l1a_l1s_com.dedic_set.aset->dtx_allowed = l1a_l1s_com.ba_list.new_list.dtx_allowed;

        // Set the TCH mode (DAI mode and DTX) in MCU-DSP com.
        l1ddsp_load_tch_mode(l1a_l1s_com.dedic_set.aset->dai_mode,
                             l1a_l1s_com.dedic_set.aset->dtx_allowed);

        // clear NEW BA present flag.
        l1a_l1s_com.ba_list.new_list_present = 0;
       #if FF_L1_IT_DSP_DTX
         // Initialize timer for fast DTX availabilty
         l1a_apihisr_com.dtx.fast_dtx_ready_timer = FAST_DTX_LATENCY;
       #endif
      }
    }
#if FF_L1_IT_DSP_DTX
   } //if (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
#endif
  }//end D_BAMS_MEAS

#if FF_L1_IT_DSP_DTX
  // Pipelining of control operations to be delayed in DTX interrupt when used.
  if (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
  {
#endif
  // Clear np flag.
  //---------------
  l1a_l1s_com.ba_list.np_ctrl   = 0;

  #if L1_GPRS
    // Clear controlled flag pnp_ctrl.
    //-------------------------------
   l1pa_l1ps_com.cr_freq_list.pnp_ctrl = 0;
  #endif

  // C W R pipeline management.
  //---------------------------
  l1a_l1s_com.full_list.ms_ctrl_dd = l1a_l1s_com.full_list.ms_ctrl_d;
  l1a_l1s_com.full_list.ms_ctrl_d  = l1a_l1s_com.full_list.ms_ctrl;
  l1a_l1s_com.full_list.ms_ctrl    = 0;

  l1a_l1s_com.ba_list.ms_ctrl_dd   = l1a_l1s_com.ba_list.ms_ctrl_d;
  l1a_l1s_com.ba_list.ms_ctrl_d    = l1a_l1s_com.ba_list.ms_ctrl;
  l1a_l1s_com.ba_list.ms_ctrl      = FALSE;


  for(i=0; i<C_BA_PM_MEAS; i++)
  {
    l1a_l1s_com.ba_list.used_il_dd[i]  = l1a_l1s_com.ba_list.used_il_d[i];
    l1a_l1s_com.ba_list.used_il_d [i]  = l1a_l1s_com.ba_list.used_il  [i];
    l1a_l1s_com.ba_list.used_lna_dd[i] = l1a_l1s_com.ba_list.used_lna_d[i];
    l1a_l1s_com.ba_list.used_lna_d [i] = l1a_l1s_com.ba_list.used_lna  [i];
  }
#if FF_L1_IT_DSP_DTX
  }
#endif
}

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

/*-------------------------------------------------------*/
/* l1s_end_manager()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (L1_DYN_DSP_DWNLD == 1)
void l1s_end_manager()
{

  BOOL misc_task_ctrl_dyn_dwnld = FALSE;

  #if L1_GPRS
  #if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX) || (FF_L1_FAST_DECODING == 1)
    // In case of Fast USF, PDTCH Read operations are not affected by
    // USF validity.
    if ((l1ps_macs_com.rlc_downlink_call == TRUE)
      #if FF_L1_IT_DSP_USF
        && (l1ps_macs_com.usf_status != USF_IT_DSP)
      #endif
      #if FF_L1_IT_DSP_DTX
        && (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
      #endif
      #if (FF_L1_FAST_DECODING == 1)
        && (l1a_apihisr_com.fast_decoding.status != C_FAST_DECODING_PROCESSING)
      #endif
       )
         l1ps_macs_rlc_downlink_call();
  #else
    if (l1ps_macs_com.rlc_downlink_call == TRUE)
      l1ps_macs_rlc_downlink_call();
  #endif
#endif // L1_GPRS

#if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX) || (FF_L1_FAST_DECODING == 1)
  // TPU and DSP pages can be switched only if relevant scenario are complete
  // which may be postponed late in the TDMA when Fast USF is used.
  if (TRUE
    #if FF_L1_IT_DSP_USF
      && (l1ps_macs_com.usf_status != USF_AWAITED)
    #endif
    #if FF_L1_IT_DSP_DTX
      && (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
    #endif
    #if (FF_L1_FAST_DECODING == 1)
      && (l1a_apihisr_com.fast_decoding.deferred_control_req == FALSE)
    #endif /*#if (FF_L1_FAST_DECODING == 1)*/
    )
  {
#endif

  // call the gauging algorithm only with paging or Packet paging in Idle mode
  // avoid call with NP during packet Tranfer Mode (NMOII)
  if (l1a_l1s_com.mode == I_MODE)
  l1s_gauging_task();

  if(l1s_dsp_com.dsp_r_page_used == TRUE)
  /*************************************************************/
  /* The "read" page for comm. with DSP has been used in the   */
  /* current TDMA.                                             */
  /* --> clear it!!!                                           */
  /* --> switch it!!!                                          */
  /*************************************************************/
  {
    // clear page.
    l1s_reset_db_dsp_to_mcu(l1s_dsp_com.dsp_db_r_ptr);

    // switch page.
    l1s_dsp_com.dsp_r_page ^= 1;
  }

 if(l1s.dyn_dwnld_state > 0)
    misc_task_ctrl_dyn_dwnld = TRUE;

  #if (AUDIO_TASK == 1)
    if(l1s.dsp_ctrl_reg != NO_CTRL)
    /*************************************************************/
    /* Some controle (RX/TX/MS) have been performed in the       */
    /* current frame. We must close the MCU/DSP comm. page now.  */
    /*************************************************************/
    {
      // Omega power down for TCH/F and TCH/H at release.
      if(l1a_l1s_com.dedic_set.stop_tch == TRUE)
      {
        l1ddsp_stop_tch();
        l1a_l1s_com.dedic_set.stop_tch = FALSE;
      }
      // A misc task is working with a GSM tasks or the DSP requests an IT com.
      // Or the gauging is active or dynamic download activity is active
      #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)  // with Calypso the DSP can be in Idle3 during the Gauging
        if ( (l1s.l1_audio_it_com)  ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
			 (misc_task_ctrl_dyn_dwnld == TRUE))
      #else
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
             (l1s.pw_mgr.gauging_task == ACTIVE) ||
			 (misc_task_ctrl_dyn_dwnld == TRUE))
      #endif
      {
        // When a MISC task is enabled L1S must be ran every frame
        // to be able to enable the frame interrupt for DSP
        l1a_l1s_com.time_to_next_l1s_task = 0;

        l1ddsp_end_scenario(GSM_MISC_CTL);
      }
      else
      {
        l1ddsp_end_scenario(GSM_CTL);
      }
    }
    else // No GSM task
    {
      // A misc task is working without a GSM tasks or the DSP request an IT com.
      // Or the gauging is active or dynamic download activity is active
      #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)  // with Calypso the DSP can be in Idle3 during the Gauging
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
			 (misc_task_ctrl_dyn_dwnld == TRUE))
      #else
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
             (l1s.pw_mgr.gauging_task == ACTIVE) ||
			 (misc_task_ctrl_dyn_dwnld == TRUE))
      #endif
      {
        // When a MISC task is enabled L1S must be ran every frame
        // to be able to enable the frame interrupt for DSP
        l1a_l1s_com.time_to_next_l1s_task = 0;
        l1ddsp_end_scenario(MISC_CTL);
      }
    }
  #else
    if(l1s.dsp_ctrl_reg != NO_CTRL)
    /*************************************************************/
    /* Some controle (RX/TX/MS) have been performed in the       */
    /* current frame. We must close the MCU/DSP comm. page now.  */
    /*************************************************************/
    {
      // Omega power down for TCH/F and TCH/H at release.
      if(l1a_l1s_com.dedic_set.stop_tch == TRUE)
      {
        l1ddsp_stop_tch();
        l1a_l1s_com.dedic_set.stop_tch = FALSE;
      }

      l1ddsp_end_scenario(GSM_CTL);
    }
  #endif // AUDIO_TASK == 1



  if(l1s.tpu_ctrl_reg != NO_CTRL)
  /*************************************************************/
  /* Some controle (RX/TX/MS/SYNC) have been performed in the  */
  /* current frame. We must close the MCU/TPU comm. page now.  */
  /*************************************************************/
  {
    l1dtpu_end_scenario();
  }

  // Propagate input level and lna state for Serving (Idle/dedic) tasks
  //-------------------------------------------------------------------
  l1a_l1s_com.Scell_used_IL_dd.input_level = l1a_l1s_com.Scell_used_IL_d.input_level;
  l1a_l1s_com.Scell_used_IL_d.input_level  = l1a_l1s_com.Scell_used_IL.input_level;
  l1a_l1s_com.Scell_used_IL.input_level    = l1_config.params.il_min;

  l1a_l1s_com.Scell_used_IL_dd.lna_off     = l1a_l1s_com.Scell_used_IL_d.lna_off;
  l1a_l1s_com.Scell_used_IL_d.lna_off      = l1a_l1s_com.Scell_used_IL.lna_off;
  l1a_l1s_com.Scell_used_IL.lna_off        = FALSE;

  // Propagate radio_freq for dedic. mode use (hopping).
  //-----------------------------------------------
  l1a_l1s_com.dedic_set.radio_freq_dd = l1a_l1s_com.dedic_set.radio_freq_d;
  l1a_l1s_com.dedic_set.radio_freq_d  = l1a_l1s_com.dedic_set.radio_freq;

  #if L1_GPRS
    // Propagate radio_freq for packet idle mode use (hopping).
    //-----------------------------------------------
    l1pa_l1ps_com.p_idle_param.radio_freq_dd = l1pa_l1ps_com.p_idle_param.radio_freq_d;
    l1pa_l1ps_com.p_idle_param.radio_freq_d  = l1pa_l1ps_com.p_idle_param.radio_freq;
  #endif

#if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX) || (FF_L1_FAST_DECODING == 1)
  }
#endif
}
#else
void l1s_end_manager()
{
#if L1_GPRS
  #if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX)
    // In case of Fast USF, PDTCH Read operations are not affected by
    // USF validity.
    if ((l1ps_macs_com.rlc_downlink_call == TRUE)
      #if FF_L1_IT_DSP_USF
        && (l1ps_macs_com.usf_status != USF_IT_DSP)
      #endif
      #if FF_L1_IT_DSP_DTX
        && (l1a_apihisr_com.dtx.dtx_status != DTX_IT_DSP)
      #endif
     )
         l1ps_macs_rlc_downlink_call();
  #else
    if (l1ps_macs_com.rlc_downlink_call == TRUE)
      l1ps_macs_rlc_downlink_call();
  #endif
#endif // L1_GPRS

#if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX)
  // TPU and DSP pages can be switched only if relevant scenario are complete
  // which may be postponed late in the TDMA when Fast USF is used.
  if (TRUE
    #if FF_L1_IT_DSP_USF
      && (l1ps_macs_com.usf_status != USF_AWAITED)
    #endif
    #if FF_L1_IT_DSP_DTX
      && (l1a_apihisr_com.dtx.dtx_status != DTX_AWAITED)
    #endif
    )
  {
#endif

  // call the gauging algorithm only with paging or Packet paging in Idle mode
  // avoid call with NP during packet Tranfer Mode (NMOII)
  if (l1a_l1s_com.mode == I_MODE)
  l1s_gauging_task();

  if(l1s_dsp_com.dsp_r_page_used == TRUE)
  /*************************************************************/
  /* The "read" page for comm. with DSP has been used in the   */
  /* current TDMA.                                             */
  /* --> clear it!!!                                           */
  /* --> switch it!!!                                          */
  /*************************************************************/
  {
    // clear page.
    l1s_reset_db_dsp_to_mcu(l1s_dsp_com.dsp_db_r_ptr);

    // switch page.
    l1s_dsp_com.dsp_r_page ^= 1;
  }

  #if (AUDIO_TASK == 1)
    if(l1s.dsp_ctrl_reg != NO_CTRL)
    /*************************************************************/
    /* Some controle (RX/TX/MS) have been performed in the       */
    /* current frame. We must close the MCU/DSP comm. page now.  */
    /*************************************************************/
    {
      // Omega power down for TCH/F and TCH/H at release.
      if(l1a_l1s_com.dedic_set.stop_tch == TRUE)
      {
        l1ddsp_stop_tch();
        l1a_l1s_com.dedic_set.stop_tch = FALSE;
      }
      // A misc task is working with a GSM tasks or the DSP requests an IT com.
      // Or the gauging is active
      #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)  // with Calypso the DSP can be in Idle3 during the Gauging
        if ( (l1s.l1_audio_it_com)  ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ))
      #else
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
             (l1s.pw_mgr.gauging_task == ACTIVE) )
      #endif
      {
        // When a MISC task is enabled L1S must be ran every frame
        // to be able to enable the frame interrupt for DSP
        l1a_l1s_com.time_to_next_l1s_task = 0;

        l1ddsp_end_scenario(GSM_MISC_CTL);
      }
      else
      {
        l1ddsp_end_scenario(GSM_CTL);
      }
    }
    else // No GSM task
    {
      // A misc task is working without a GSM tasks or the DSP request an IT com.
      // Or the gauging is active
      #if (CHIPSET==7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)  // with Calypso the DSP can be in Idle3 during the Gauging
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ))
      #else
        if ( (l1s.l1_audio_it_com) ||
             (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_IT_COM_REQ) ||
             (l1s.pw_mgr.gauging_task == ACTIVE) )
      #endif
      {
        // When a MISC task is enabled L1S must be ran every frame
        // to be able to enable the frame interrupt for DSP
        l1a_l1s_com.time_to_next_l1s_task = 0;
        l1ddsp_end_scenario(MISC_CTL);
      }
    }
  #else
    if(l1s.dsp_ctrl_reg != NO_CTRL)
    /*************************************************************/
    /* Some controle (RX/TX/MS) have been performed in the       */
    /* current frame. We must close the MCU/DSP comm. page now.  */
    /*************************************************************/
    {
      // Omega power down for TCH/F and TCH/H at release.
      if(l1a_l1s_com.dedic_set.stop_tch == TRUE)
      {
        l1ddsp_stop_tch();
        l1a_l1s_com.dedic_set.stop_tch = FALSE;
      }

      l1ddsp_end_scenario(GSM_CTL);
    }
  #endif // AUDIO_TASK == 1



  if(l1s.tpu_ctrl_reg != NO_CTRL)
  /*************************************************************/
  /* Some controle (RX/TX/MS/SYNC) have been performed in the  */
  /* current frame. We must close the MCU/TPU comm. page now.  */
  /*************************************************************/
  {
    l1dtpu_end_scenario();
  }

  // Propagate input level and lna state for Serving (Idle/dedic) tasks
  //-------------------------------------------------------------------
  l1a_l1s_com.Scell_used_IL_dd.input_level = l1a_l1s_com.Scell_used_IL_d.input_level;
  l1a_l1s_com.Scell_used_IL_d.input_level  = l1a_l1s_com.Scell_used_IL.input_level;
  l1a_l1s_com.Scell_used_IL.input_level    = l1_config.params.il_min;

  l1a_l1s_com.Scell_used_IL_dd.lna_off     = l1a_l1s_com.Scell_used_IL_d.lna_off;
  l1a_l1s_com.Scell_used_IL_d.lna_off      = l1a_l1s_com.Scell_used_IL.lna_off;
  l1a_l1s_com.Scell_used_IL.lna_off        = FALSE;

  // Propagate radio_freq for dedic. mode use (hopping).
  //-----------------------------------------------
  l1a_l1s_com.dedic_set.radio_freq_dd = l1a_l1s_com.dedic_set.radio_freq_d;
  l1a_l1s_com.dedic_set.radio_freq_d  = l1a_l1s_com.dedic_set.radio_freq;

  #if L1_GPRS
    // Propagate radio_freq for packet idle mode use (hopping).
    //-----------------------------------------------
    l1pa_l1ps_com.p_idle_param.radio_freq_dd = l1pa_l1ps_com.p_idle_param.radio_freq_d;
    l1pa_l1ps_com.p_idle_param.radio_freq_d  = l1pa_l1ps_com.p_idle_param.radio_freq;
  #endif

#if (FF_L1_IT_DSP_USF) || (FF_L1_IT_DSP_DTX)
  }
#endif
}
#endif // L1_DSP_DYN_DWNLD
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM

//------------------------------------------------------------------------------------------------

/*-------------------------------------------------------*/
/* l1s_dedicated_mode_manager()                          */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_dedicated_mode_manager()
{
  T_CHANNEL_DESCRIPTION  *chan1_desc_ptr;
  T_CHANNEL_DESCRIPTION  *chan2_desc_ptr;
  T_MOBILE_ALLOCATION    *alist_ptr;
  xSignalHeaderRec       *msg;
  BOOL                    process_assign_now = TRUE;

#if (FF_L1_TCH_VOCODER_CONTROL == 1)
  // Start vocoder

#if (L1_VOCODER_IF_CHANGE == 1)
    if (l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_ENABLE_COMMAND)
    {
      l1a_l1s_com.dedic_set.sync_tch = TRUE;
      l1a_l1s_com.dedic_set.vocoder_on = TRUE;
      l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_RESET_COMMAND;

      // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
      l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
      l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
#endif // L1_AUDIO_MCU_ONOFF

      msg = os_alloc_sig(0);
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = L1_VOCODER_CFG_ENABLE_CON;
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
    else if(l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_DISABLE_COMMAND)
    {
      l1a_l1s_com.dedic_set.vocoder_on = FALSE;
      l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_RESET_COMMAND;

      // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
      l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
      l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF

      msg = os_alloc_sig(0);
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = L1_VOCODER_CFG_DISABLE_CON;
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }
#else
  #if (W_A_DSP_PR20037 == 1)
    if (l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_ENABLE_REQ)
    {
      l1a_l1s_com.dedic_set.sync_tch = TRUE;
      l1a_l1s_com.dedic_set.vocoder_on = TRUE;
      l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_ENABLED;
    }
    else if(l1a_l1s_com.dedic_set.start_vocoder == TCH_VOCODER_DISABLE_REQ)
    {
      l1a_l1s_com.dedic_set.vocoder_on = FALSE;
      l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_DISABLED;
    }
  #else // W_A_DSP_PR20037 == 0
    if (l1a_l1s_com.dedic_set.start_vocoder == TRUE)
    {
      l1a_l1s_com.dedic_set.start_vocoder = FALSE;
      l1a_l1s_com.dedic_set.sync_tch = TRUE;
      l1a_l1s_com.dedic_set.vocoder_on = TRUE;
    }
  #endif // W_A_DSP_PR20037
#endif // L1_VOCODER_IF_CHANGE
#endif // FF_L1_TCH_VOCODER_CONTROL == 1

  //=========================================
  // Process the new dedicated parameter set.
  //=========================================

#if ((REL99 == 1) && (FF_BHO == 1))
  if(l1a_l1s_com.dedic_set.long_rem_handover_type != BLIND_HANDOVER)
  {
#endif
  if(    (l1a_l1s_com.dedic_set.SignalCode == MPHC_CHANNEL_ASSIGN_REQ)
      || (l1a_l1s_com.dedic_set.SignalCode == MPHC_SYNC_HO_REQ)
      || (l1a_l1s_com.dedic_set.SignalCode == MPHC_ASYNC_HO_REQ)  )
  {
    //Some pre-processing of the ASSIGNMENT/HANDOVER COMMANDS is required to reliably pass
    //GSM 11.10 17.1 where ASSIGNMENT from TCH/F -> TCH/F without
    //a start time must transmit on the new channel within 20ms
    //Also, should reduce muting/click on channel change

    if(    (l1a_l1s_com.dedic_set.fset->neig_sti_fn == -1 )
        && (l1a_l1s_com.dedic_set.fset->chan1.desc.channel_type == TCH_F)
        && (l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type == TCH_F) )
    {
      //ASSIGNMENT from TCH/F -> TCH/F immediately. Must wait until
      //the current FACCH boundary is complete, before processing the assignment
      //as the 20ms timer is started at the end of the last valid speech/FACCH block.

      UWORD32 facch_position = (l1s.next_time.fn_in_report % 13) % 4;

      if(facch_position == 1)
        process_assign_now = TRUE;
      else
        process_assign_now = FALSE;
    }
    else
    {
      process_assign_now = TRUE;
    }
    }
#if ((REL99 == 1) && (FF_BHO == 1))
  }
#endif

  // Now see if channel change commands are pending.....
  if(    (  l1a_l1s_com.dedic_set.SignalCode == MPHC_IMMED_ASSIGN_REQ )
      || ( (l1a_l1s_com.dedic_set.SignalCode == MPHC_CHANNEL_ASSIGN_REQ) && (process_assign_now == TRUE) )
      || ( (l1a_l1s_com.dedic_set.SignalCode == MPHC_SYNC_HO_REQ)        && (process_assign_now == TRUE) )
      || ( (l1a_l1s_com.dedic_set.SignalCode == MPHC_ASYNC_HO_REQ)       && (process_assign_now == TRUE) )
      || (  l1a_l1s_com.dedic_set.SignalCode == MPHC_HANDOVER_FAIL_REQ)
      || (  l1a_l1s_com.dedic_set.SignalCode == MPHC_CHANGE_FREQUENCY)  )
  // A new channel is given in fset...
  {
    #if 0	/* FreeCalypso TCS211 reconstruction */
      // Reset DTX AMR status
      #if (AMR == 1)
        l1s.dtx_amr_dl_on=FALSE;
      #endif
    #endif
    // When a Dedicated mode request is pending, L1S must be ran every frame
    // to be able to cope with STI.
    l1a_l1s_com.time_to_next_l1s_task = 0;

    // Set the default channel/description.
    //-------------------------------------

    chan1_desc_ptr = &l1a_l1s_com.dedic_set.fset->chan1.desc;
    chan2_desc_ptr = &l1a_l1s_com.dedic_set.fset->chan2.desc;
    alist_ptr      = &l1a_l1s_com.dedic_set.fset->ma.freq_list;

    // Starting time management.
    //==========================

    if(l1a_l1s_com.dedic_set.fset->neig_sti_fn != -1)
    // Starting time present.
    {
      WORD32  time_diff;
      WORD32  frame_shift=0;
      WORD32  tn_diff;

      tn_diff = l1a_l1s_com.tn_difference;

      if(tn_diff < 0)
      {
        frame_shift -= 1;
        tn_diff     += 8;
      }

      if((l1a_l1s_com.dedic_set.fset->cell_desc.time_alignmt + (tn_diff * BP_DURATION)) >= SWITCH_TIME)
        frame_shift += 1;

      time_diff = ( (l1a_l1s_com.dedic_set.fset->serv_sti_fn - 1) + frame_shift -
                    (l1s.next_time.fn % 42432) + 2*42432) % 42432;

      if(((time_diff >= (32024)) && (time_diff <= (42431))) || (time_diff == 0))
      // Starting time has been passed or the current frame corresponds to STARTING TIME...
      //  -> Reset STI (neig. domain and serv. domain) (set to -1).
      //  -> Channel description must be the one for After STI (default case).
      //  -> Frequency redefinition must be confirmed if any.
      // Rem: numbers come from GSM04.08, $10.5.2.38.
      // Rem: starting time detected 1 frame in adavance, this frame is used by
      //      SYNCHRO task.
      {
        l1a_l1s_com.dedic_set.fset->serv_sti_fn = -1;
        l1a_l1s_com.dedic_set.fset->neig_sti_fn = -1;

        #if (TRACE_TYPE!=0)
          // Trace "starting time" on log file and screen.
          trace_fct(CST_STI_PASSED, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        if(l1a_l1s_com.dedic_set.fset->freq_redef_flag == TRUE)
        // FREQUENCY REDEFINITION must be confirmed.
        {
          xSignalHeaderRec  *conf_msg;

          // Clear FREQUENY REDEFINITION flag.
          l1a_l1s_com.dedic_set.fset->freq_redef_flag = FALSE;

          // Alloc confirmation message...
          conf_msg = os_alloc_sig(sizeof(int));
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1C_REDEF_DONE;

          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        }
      }
      else
      //  -> Channel description must be the one for Before STI.
      {
        chan1_desc_ptr = &l1a_l1s_com.dedic_set.fset->chan1.desc_bef_sti;
        chan2_desc_ptr = &l1a_l1s_com.dedic_set.fset->chan2.desc_bef_sti;
        alist_ptr      = &l1a_l1s_com.dedic_set.fset->ma.freq_list_bef_sti;
      }
    }

    // Switch active channel to the new given one.
    //--------------------------------------------

    if(chan1_desc_ptr->channel_type != INVALID_CHANNEL)
    // The ongoing configuration for the new channel is valid.
    {
      UWORD8  current_channel_type = INVALID_CHANNEL;
      UWORD8  current_channel_mode = SIG_ONLY_MODE;
      UWORD8  new_channel_type     = chan1_desc_ptr->channel_type;

      if(l1a_l1s_com.dedic_set.aset != NULL)
      {
        current_channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;
        current_channel_mode = l1a_l1s_com.dedic_set.aset->chan1.mode;
      }

      #if W_A_DSP1
        if(l1a_l1s_com.dedic_set.SignalCode != MPHC_CHANGE_FREQUENCY)
        {
          old_sacch_DSP_bug = TRUE;
        }
      #endif

      l1a_l1s_com.dedic_set.fset->chan1.desc_ptr = chan1_desc_ptr;
      l1a_l1s_com.dedic_set.fset->chan2.desc_ptr = chan2_desc_ptr;
      l1a_l1s_com.dedic_set.fset->ma.alist_ptr   = alist_ptr;


      // Keep Timing Advance from current active channel.
      if(l1a_l1s_com.dedic_set.SignalCode == MPHC_CHANNEL_ASSIGN_REQ)
      {
        l1a_l1s_com.dedic_set.fset->new_timing_advance =
          l1a_l1s_com.dedic_set.aset->new_timing_advance;
        l1a_l1s_com.dedic_set.fset->timing_advance =
          l1a_l1s_com.dedic_set.aset->timing_advance;
      }

      // If current active channel is a TCH channel then we must inform the
      // DSP to stop any vocoder activity when leaving this channel.
      if(((current_channel_type == TCH_F) || (current_channel_type == TCH_H)) &&
         (new_channel_type != TCH_F) &&
         (new_channel_type != TCH_H))
      {
        l1a_l1s_com.dedic_set.stop_tch = TRUE;

	/*
	 * FreeCalypso TCS211 reconstruction: the following code
	 * appears to be a LoCosto-ism, so let's take it out.
	 */
      #if 0
        // If audio enabling was forced by L1S because of a HO failure, do not force it anymore.
        // Restore it in the state required by the MMI if the feature is compiled.
        if (l1a_l1s_com.audio_forced_by_l1s == TRUE)
        {
        #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
          if (l1a_l1s_com.audio_onoff_task.parameters.onoff_value == FALSE)
          {
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
          }
        #else // L1_EXTERNAL_AUDIO_VOICE_ONOFF
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
        #endif // L1_EXTERNAL_AUDIO_VOICE_ONOFF
          l1a_l1s_com.audio_forced_by_l1s = FALSE;
        }
      #endif
      }

      // The new channel becomes the ACTIVE one.
      l1a_l1s_com.dedic_set.aset = l1a_l1s_com.dedic_set.fset;

      // Active channel is CHAN1 by default.
      l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan1);

      // Store new ciphering setting in MCU-DSP com.
      l1ddsp_load_ciph_param(l1a_l1s_com.dedic_set.aset->a5mode,
                             &(l1a_l1s_com.dedic_set.aset->ciph_key));

      // Set the TCH mode (DAI mode and DTX) in MCU-DSP com.
      l1ddsp_load_tch_mode(l1a_l1s_com.dedic_set.aset->dai_mode,
                           l1a_l1s_com.dedic_set.aset->dtx_allowed);

      // Dedicated set TXPWR command must be applied at once.
      l1s.applied_txpwr  = l1a_l1s_com.dedic_set.aset->new_target_txpwr;
      l1s.reported_txpwr = l1s.applied_txpwr;

      // Switch from current mode to DEDICATED MODE.
      l1a_l1s_com.mode = DEDIC_MODE;

      // Enable globally all dedicated tasks.
      l1a_l1s_com.l1s_en_task[DEDIC] = TASK_ENABLED;

      // Set SYNCHRO task enable flag -> synchro to the new serving cell.
      // Set "sync_tch" flag if TCH/F or TCH/H channel whatever the channel mode.
      // Rem: no synchro required when just changing the freq list.
      if(l1a_l1s_com.dedic_set.SignalCode != MPHC_CHANGE_FREQUENCY)
      {
        UWORD8  channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;

        // TF 12/8/98 - the following line was moved from above to solve bad meas reports
        // after frequency redefinition bug.
        // Given beacon becomes the serving cell, download cell description
        // in the serving cell structure.
        l1a_l1s_com.Scell_info = l1a_l1s_com.dedic_set.aset->cell_desc;

     #if (AMR == 1)
        // Reset DTX AMR status
        l1s.dtx_amr_dl_on=FALSE;
     #endif

        if((channel_type == TCH_F) || (channel_type == TCH_H))
        {
        #if (FF_L1_TCH_VOCODER_CONTROL == 1)
          // With this feature, SACCH reset is controlled by L1, not sync_tch
          l1a_l1s_com.dedic_set.reset_sacch = TRUE;
        #endif
          l1a_l1s_com.dedic_set.sync_tch = TRUE;

          l1a_l1s_com.dedic_set.reset_facch = TRUE;
          // Reset DTX mode flags
          l1s.dtx_ul_on = FALSE;
          l1s.facch_bursts = -1;
        }

        // SYNCHRO task and its associated parameters (tn_difference, dl_tn and
        // dsp_scheduler_mode) are not Enabled/Updated if we are in the specific case:
        // L1A is touching SYNCHRO parameters and leave L1A to go in HISR (L1S)
        // in middle of the update (cf. BUG1339)
        if(l1a_l1s_com.task_param[SYNCHRO] == SEMAPHORE_RESET)
        {
          #if L1_GPRS
            // Select GSM DSP Scheduler.
            l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;
          #endif

          // Enable SYNCHRO task to achive the new configuration. Since SYNCHRO
          // has the highest priority and the installation of this task in
          // the MFTAB is made after a CLEAR of the MFTAB, we insure that
          // any uncompleted task will be ABORTED.
          l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;
        }

        #if IDS
          // Set b_itc (Information transfer capability) bits in d_ra_conf
          l1s_dsp_com.dsp_ndb_ptr->d_ra_conf = (UWORD16) l1a_l1s_com.dedic_set.aset->ids_mode;
        #endif
        #if (AMR == 1)
          // If the new channel mode to apply is an adaptative mode
          // then the AMR parameter must be transmitted to the DSP
           if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AHS_MODE) ||
               (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AFS_MODE) )
          {
            // Transmit the AMR ver 1.0 settings to the DSP
            l1ddsp_load_amr_param(l1a_l1s_com.dedic_set.aset->amr_configuration,l1a_l1s_com.dedic_set.aset->cmip);

          #if (L1_AMR_NSYNC == 1)
            // AMR NSYNC bit: set to 0 by load_amr_param, set to 1 if HO from AMR cell to AMR cell, reset by DSP
            if ( (current_channel_mode == TCH_AFS_MODE) || (current_channel_mode == TCH_AHS_MODE) )
            {
            #if (FF_L1_TCH_VOCODER_CONTROL == 1)
              if(l1a_l1s_com.dedic_set.vocoder_on == TRUE)
              {
                l1s_dsp_com.dsp_ndb_ptr->a_amr_config[NSYNC_INDEX] |= (1 << NSYNC_SHIFT);
              }
            #else
              l1s_dsp_com.dsp_ndb_ptr->a_amr_config[NSYNC_INDEX] |= (1 << NSYNC_SHIFT);
            #endif
            }
          #endif

            // Set the flag to tell to DSP that a new AMR config is ready in the API memory
            l1a_l1s_com.dedic_set.sync_amr = TRUE;
          }
        #endif

        #if FF_L1_IT_DSP_DTX
          // Initialize timer for fast DTX availabilty
          l1a_apihisr_com.dtx.fast_dtx_ready_timer = FAST_DTX_LATENCY;
        #endif
      }

      // Set HO_ACCESS counter according to HO type.
      if(l1a_l1s_com.dedic_set.SignalCode == MPHC_ASYNC_HO_REQ)
      {
        UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;

        l1a_l1s_com.dedic_set.aset->ho_acc_to_send = -1; // Send HO_ACCESS until...

        if((channel_type == TCH_F) || (channel_type == TCH_H))
          l1a_l1s_com.dedic_set.aset->t3124 = 70;  // Send HO_ACCESS for 320ms.
        else
          l1a_l1s_com.dedic_set.aset->t3124 = 147;   // Send HO_ACCESS for 675ms.
      }
      else
      if(l1a_l1s_com.dedic_set.SignalCode == MPHC_SYNC_HO_REQ)
        l1a_l1s_com.dedic_set.aset->ho_acc_to_send = 4;  // Send 4 HO_ACCESS.
      else
        l1a_l1s_com.dedic_set.aset->ho_acc_to_send = 0;  // No HO_ACCESS to send.

      if(l1a_l1s_com.dedic_set.SignalCode != MPHC_CHANGE_FREQUENCY)
      // Rem: no confirmation msg required when just changing the freq list.
      {
        // alloc confirmation message...
        msg = os_alloc_sig(sizeof(int));
        DEBUGMSG(status,NU_ALLOC_ERR)
        msg->SignalCode = L1C_DEDIC_DONE;

        // send confirmation message...
        os_send_sig(msg, L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)

        #if (TRACE_TYPE==5)
          trace_dedic();
        #endif

#if (FF_REPEATED_SACCH == 1)
            l1s.repeated_sacch.buffer_empty = TRUE;       //sacch buffer
            l1s.repeated_sacch.sro = 0;                   //BTS repetition order
            l1s.repeated_sacch.srr = 0;                   //MS repetition request
#endif /* (FF_REPEATED_SACCH == 1) */

#if ((FF_REPEATED_SACCH == 1) && (TRACE_TYPE == 1 || TRACE_TYPE == 4))
        trace_info.repeat_sacch.dl_count = 0;
        trace_info.repeat_sacch.dl_combined_good_count = 0;
	trace_info.repeat_sacch.dl_error_count	= 0;
        trace_info.repeat_sacch.srr= 0;
        trace_info.repeat_sacch.sro= 0;
        trace_info.repeat_sacch.dl_good_norep = 0;
        trace_info.repeat_sacch.dl_buffer_empty = TRUE;
#endif /* ((FF_REPEATED_SACCH == 1) && (TRACE_TYPE == 1 || TRACE_TYPE == 4))*/
        #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
          trace_info.facch_dl_count = 0;
          trace_info.facch_ul_count = 0;
          trace_info.facch_dl_fail_count = 0;
         #if (FF_REPEATED_DL_FACCH == 1 )
        trace_info.facch_dl_combined_good_count = 0;    /* reported and chase combined also(2nd attempt) <=
                                                                                                                         facch_dl_good_block_reported */
        trace_info.facch_dl_repetition_block_count = 0;    /*represents duplicate not passed to PS */
        trace_info.facch_dl_count_all = 0;                          /* ALL FACCHS received good or bad*/
        trace_info.facch_dl_good_block_reported = 0;       /*  facch_dl_count */
        #endif /* (FF_REPEATED_DL_FACCH == 1 ) */
          trace_info.sacch_d_nerr = 0;
        #endif
	#if ( FF_REPEATED_DL_FACCH == 1)
        l1s.repeated_facch.pipeline[0].buffer_empty=l1s.repeated_facch.pipeline[1].buffer_empty=TRUE;
        if ( l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type == TCH_F) /* For TCH/F */
        {
                l1s.repeated_facch.counter_candidate=0;
                l1s.repeated_facch.counter=1;
        }
         if ( l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type == TCH_H) /* For TCH/H */
        {
                // l1s.repeated_facch.counter_candidate=l1s.repeated_facch.counter=0;
               /*  l1s.repeated_facch.counter_candidate=0;
                    l1s.repeated_facch.counter=1;*/
                    l1s.repeated_facch.counter_candidate=l1s.repeated_facch.counter=0;
        }
      #endif  /* (FF_REPEATED_DL_FACCH == 1 ) */
      }

      // Clear dedicated channel trigger message.
      l1a_l1s_com.dedic_set.SignalCode = NULL;

      // Clear the dedicated handover fail mode.
      l1a_l1s_com.dedic_set.handover_fail_mode = FALSE;
    }
  }

  else
  if(l1a_l1s_com.dedic_set.SignalCode == MPHC_CHANNEL_MODE_MODIFY_REQ)
  // We must change the current channel mode for the given subchannel.
  {
    UWORD8  channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;
    UWORD8  channel_mode = l1a_l1s_com.dedic_set.aset->chan1.mode;

    if((channel_type == TCH_F) || (channel_type == TCH_H))
    {
    #if (FF_L1_TCH_VOCODER_CONTROL == 1)
      // With this feature, SACCH reset is controlled by L1, not sync_tch
      l1a_l1s_com.dedic_set.reset_sacch = TRUE;
    #endif
      l1a_l1s_com.dedic_set.sync_tch = TRUE;
    }

    if(l1a_l1s_com.dedic_set.mode_modif.subchannel == l1a_l1s_com.dedic_set.aset->chan1.desc.subchannel)
      l1a_l1s_com.dedic_set.aset->chan1.mode = l1a_l1s_com.dedic_set.mode_modif.channel_mode;
    else
      l1a_l1s_com.dedic_set.aset->chan2.mode = l1a_l1s_com.dedic_set.mode_modif.channel_mode;
    #if (AMR == 1)
      // If the new channel mode to apply is an adaptative mode
      // then the AMR parameter must be transmitted to the DSP
      if ( (l1a_l1s_com.dedic_set.mode_modif.channel_mode == TCH_AHS_MODE) ||
           (l1a_l1s_com.dedic_set.mode_modif.channel_mode == TCH_AFS_MODE) )
      {
        // Transmit the AMR ver 1.0 settings to the DSP
        l1ddsp_load_amr_param(l1a_l1s_com.dedic_set.mode_modif.amr_configuration,C_AMR_CMIP_DEFAULT);

        #if (L1_AMR_NSYNC == 1)
          // AMR NSYNC bit: set to 0 by load_amr_param, set to 1 if HO from AMR cell to AMR cell, reset by DSP
          if ( (channel_mode == TCH_AFS_MODE) || (channel_mode == TCH_AHS_MODE) )
          {
#if (FF_L1_TCH_VOCODER_CONTROL == 1)
            if(l1a_l1s_com.dedic_set.vocoder_on == TRUE)
            {
              l1s_dsp_com.dsp_ndb_ptr->a_amr_config[NSYNC_INDEX] |= (1 << NSYNC_SHIFT);
            }
#else
            l1s_dsp_com.dsp_ndb_ptr->a_amr_config[NSYNC_INDEX] |= (1 << NSYNC_SHIFT);
#endif
          }
        #endif

        // Set the flag to tell to DSP that a new AMR config is ready in the API memory
        l1a_l1s_com.dedic_set.sync_amr = TRUE;
      }
    #endif

    // Reset input msg.
    l1a_l1s_com.dedic_set.SignalCode = NULL;
    #if FF_L1_IT_DSP_DTX
      // Initialize timer for fast DTX availabilty
      l1a_apihisr_com.dtx.fast_dtx_ready_timer = FAST_DTX_LATENCY;
    #endif
  }

  else
  if(l1a_l1s_com.dedic_set.SignalCode == MPHC_SET_CIPHERING_REQ)
  // Ciphering mode must change.
  {
    // Store new ciphering setting in MCU-DSP com.
    l1ddsp_load_ciph_param(l1a_l1s_com.dedic_set.aset->a5mode,
                           &(l1a_l1s_com.dedic_set.aset->ciph_key));

    // Reset input msg.
    l1a_l1s_com.dedic_set.SignalCode = NULL;
  }

  else
  if(l1a_l1s_com.dedic_set.SignalCode == MPHC_STOP_DEDICATED_REQ)
  // All channel must be aborted...
  {
    // Perform the functions below if there is active dedicated set.
    // This check is to take care of race condition which can happen
    // if MPHC_STOP_DEDICATED_REQ is receivedm before MPHC_HANDOVER_FAIL_REQ
#if 0	/* FreeCalypso TCS211 reconstruction */
    if (l1a_l1s_com.dedic_set.aset!=NULL)
    {
#endif

      // Stop TCH/F and TCH/H (stop Omega) (used in SYNCHRO task).
      UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;
      if((channel_type == TCH_F) || (channel_type == TCH_H))
      {
        l1a_l1s_com.dedic_set.stop_tch = TRUE;

        /* FreeCalypso change: same situation as earlier in this function */
      #if 0
        // If audio enabling was forced by L1S because of a HO failure, do not force it anymore.
        // Restore it in the state required by the MMI if the feature is compiled.
        if (l1a_l1s_com.audio_forced_by_l1s == TRUE)
        {
        #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
          if (l1a_l1s_com.audio_onoff_task.parameters.onoff_value == FALSE)
          {
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
          }
        #else // L1_EXTERNAL_AUDIO_VOICE_ONOFF
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
        #endif // L1_EXTERNAL_AUDIO_VOICE_ONOFF
          l1a_l1s_com.audio_forced_by_l1s = FALSE;
        }
      #endif

      #if (AMR == 1)
        // Reset DTX AMR status
        l1s.dtx_amr_dl_on=FALSE;
      #endif

      }

      // Clear d_ra_conf => default value
      l1s_dsp_com.dsp_ndb_ptr->d_ra_conf = 0;

      // Clear ciphering setting in MCU-DSP com.
      l1ddsp_load_ciph_param(0, &(l1a_l1s_com.dedic_set.aset->ciph_key));

      // Reset the global dedicated enable flag.
      l1a_l1s_com.l1s_en_task[DEDIC] = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[DDL]   = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[DUL]   = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[ADL]   = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[AUL]   = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[TCHTH] = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[TCHD]  = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[TCHTF] = TASK_DISABLED;
      l1a_l1s_com.l1s_en_task[TCHA]  = TASK_DISABLED;

#if 0	/* FreeCalypso TCS211 reconstruction */
    }
    else
    {
      /* FreeCalypso change: same AUDIO_TASK situation as earlier */
      #if 0
        if (l1a_l1s_com.audio_forced_by_l1s == TRUE)
        {
          #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
            if (l1a_l1s_com.audio_onoff_task.parameters.onoff_value == FALSE)
            {
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
            }
          #else // L1_EXTERNAL_AUDIO_VOICE_ONOFF
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
          #endif // L1_EXTERNAL_AUDIO_VOICE_ONOFF
          l1a_l1s_com.audio_forced_by_l1s = FALSE;
        }
      #endif
      #if (AMR == 1)
        // Reset DTX AMR status
        l1s.dtx_amr_dl_on=FALSE;
      #endif

      // Clear d_ra_conf => default value
      l1s_dsp_com.dsp_ndb_ptr->d_ra_conf = 0;
    }
#endif	/* FreeCalypso TCS211 reconstruction */

    // Reset input msg.
    l1a_l1s_com.dedic_set.SignalCode = NULL;
    #if ((REL99 == 1) && (FF_BHO == 1))
      // this is required in BHO as you need to retain the previous channel info.
      //This is checked in HO_REQ to L1S.....In normal handover this does not happen
      if(l1a_l1s_com.dedic_set.handover_type == NORMAL_HANDOVER)
      {
        // Reset active dedicated set.
        l1a_l1s_com.dedic_set.aset = NULL;
      }
    #else
      // Reset active dedicated set.
      l1a_l1s_com.dedic_set.aset = NULL;
    #endif

    // SYNCHRO task and its associated parameters (tn_difference, dl_tn and
    // dsp_scheduler_mode) are not Enabled/Updated if we are in the specific case:
    // L1A is touching SYNCHRO parameters and leave L1A to go in HISR (L1S)
    // in middle of the update (cf. BUG1339)
    if(l1a_l1s_com.task_param[SYNCHRO] == SEMAPHORE_RESET)
    {
      // The code shall always goes here as the upper layers are not allowed to send new
      // requests that can change the layer1 timing reference before the MPHC_STOP_DEDICATED_REQ
      // has been confirmed by the MPHC_STOP_DEDICATED_CON.

      // Since SYNCHRO has the highest priority and the installation of
      // this task in the MFTAB is made after a CLEAR of the MFTAB, we
      // insure that any uncompleted task will be ABORTED.
      l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;

      // The dedicated mode will be aborted within the current TDMA frame,
      // L1S confirm to L1A that the dedicated mode is stopped.
      // alloc confirmation message...
      msg = os_alloc_sig(sizeof(int));
      DEBUGMSG(status,NU_ALLOC_ERR)
      msg->SignalCode = L1C_STOP_DEDICATED_DONE;

      // send confirmation message...
      os_send_sig(msg, L1C1_QUEUE);
      DEBUGMSG(status,NU_SEND_QUEUE_ERR)
    }

#if ((REL99 == 1) && (FF_BHO == 1))
    // Check whether above stop dedicated mode procedure is done due
    // to blind handover request.
    if(l1a_l1s_com.dedic_set.handover_type == BLIND_HANDOVER)
    {
      // MPHC_STOP_DEDICATED was due to blind handover.

      // Set semaphores for FBSB task

      // Store the info from request message.
      l1a_l1s_com.nsync_fbsb.radio_freq      = l1a_l1s_com.dedic_set.bcch_carrier_of_nbr_cell;
      l1a_l1s_com.nsync_fbsb.fn_offset       = 0;
      l1a_l1s_com.nsync_fbsb.time_alignmt    = 0;
      l1a_l1s_com.nsync_fbsb.fb_found_attempt = 0;

      // Enable FBSB task
      l1a_l1s_com.l1s_en_task[FBSB] = TASK_ENABLED;
    }
  #endif // #if ((REL99 == 1) && (FF_BHO == 1))
  }

  else
  if((l1a_l1s_com.dedic_set.SignalCode == OML1_START_DAI_TEST_REQ) ||
     (l1a_l1s_com.dedic_set.SignalCode == OML1_STOP_DAI_TEST_REQ))
  // New DAI mode given...
  // We must store the mode in NDB and resynchronize the speech.
  {
    // Set the TCH mode (DAI mode and DTX) in MCU-DSP com.
    l1ddsp_load_tch_mode(l1a_l1s_com.dedic_set.aset->dai_mode,
                         l1a_l1s_com.dedic_set.aset->dtx_allowed);

    // Reset input msg.
    l1a_l1s_com.dedic_set.SignalCode = NULL;
  }

  //=============================================
  // Process the ongoing dedicated parameter set.
  //=============================================

  if((!(l1a_l1s_com.l1s_en_task[SYNCHRO] == TASK_ENABLED)) &&
     (l1a_l1s_com.dedic_set.aset != NULL))
  // No synchro performed this frame.
  //  -> process current active dedicated parameter set.
  {
    // TCH/SDCCH statistics or Frame Error rate :
    // must wait for Dedicated mode setting ....
    #if (TRACE_TYPE==3)
      if (l1_stats.wait_time > 0 ) l1_stats.wait_time--;
    #endif

    //-------------------
    // TXPWR controle...
    //-------------------

    // Save last applied TXPWR to be reported.
    // Get TXPWR to be applied in the next 13 frames.
    if((l1s.next_time.fn_in_report % 13) == 0)
    {
      l1s.reported_txpwr = l1s.applied_txpwr;
      l1s.applied_txpwr  = l1ctl_txpwr(l1a_l1s_com.dedic_set.aset->new_target_txpwr,
                                       l1s.applied_txpwr);
    }

    //----------------------------
    // Timing Advance controle...
    //----------------------------

    // Timing advance is updated for the 1st frame of the reporting period
    // with new value coming from SACCH(L1 header).
    if(l1s.next_time.fn_in_report == 0)
    {
      l1a_l1s_com.dedic_set.aset->timing_advance =
        l1a_l1s_com.dedic_set.aset->new_timing_advance;
    }

    //-------------------------------------
    // Set the default channel/description.
    //-------------------------------------

    chan1_desc_ptr = &l1a_l1s_com.dedic_set.aset->chan1.desc;
    chan2_desc_ptr = &l1a_l1s_com.dedic_set.aset->chan2.desc;
    alist_ptr      = &l1a_l1s_com.dedic_set.aset->ma.freq_list;

    // Active channel is CHAN1 by default.
    //------------------------------------
    l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan1);


    //-----------------------------
    // Starting time management...
    //-----------------------------

    if(l1a_l1s_com.dedic_set.aset->serv_sti_fn != -1)
    // Starting time present.
    //-----------------------
    {
      WORD32  time_diff;

      time_diff = (l1a_l1s_com.dedic_set.aset->neig_sti_fn -
                   (l1s.next_time.fn % 42432) + 2*42432) % 42432;

      if(((time_diff >= (32024)) && (time_diff <= (42431))) || (time_diff == 0))
      // Starting time has been passed or the current frame corresponds to STARTING TIME...
      //-----------------------------------------------------------------------------------
      //  -> Reset STI (neig. domain and serv. domain) (set to -1).
      //  -> Channel description must be the one for After STI (default case).
      //  -> Frequency redefinition must be confirmed if any.
      // Rem: numbers come from GSM04.08, $10.5.2.38.
      {
        l1a_l1s_com.dedic_set.aset->serv_sti_fn = -1;
        l1a_l1s_com.dedic_set.aset->neig_sti_fn = -1;

        #if (TRACE_TYPE!=0)
          // Trace "starting time" on log file and screen.
          trace_fct(CST_STI_PASSED, l1a_l1s_com.Scell_info.radio_freq);
        #endif

        if(l1a_l1s_com.dedic_set.aset->freq_redef_flag == TRUE)
        // FREQUENCY REDEFINITION must be confirmed.
        {
          xSignalHeaderRec  *conf_msg;

          // Clear FREQUENY REDEFINITION flag.
          l1a_l1s_com.dedic_set.aset->freq_redef_flag = FALSE;

          // Alloc confirmation message...
          conf_msg = os_alloc_sig(sizeof(int));
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1C_REDEF_DONE;

          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)
        }
        #if FF_L1_IT_DSP_DTX
          // Initialize timer for fast DTX availabilty
          l1a_apihisr_com.dtx.fast_dtx_ready_timer = FAST_DTX_LATENCY;
        #endif
      }
      else
      //  -> Channel description must be the one for Before STI.
      {
        chan1_desc_ptr = &l1a_l1s_com.dedic_set.aset->chan1.desc_bef_sti;
        chan2_desc_ptr = &l1a_l1s_com.dedic_set.aset->chan2.desc_bef_sti;
        alist_ptr      = &l1a_l1s_com.dedic_set.aset->ma.freq_list_bef_sti;
      }
    }

    //---------------------------
    // Set the active parameters.
    //---------------------------
    l1a_l1s_com.dedic_set.aset->chan1.desc_ptr = chan1_desc_ptr;
    l1a_l1s_com.dedic_set.aset->chan2.desc_ptr = chan2_desc_ptr;
    l1a_l1s_com.dedic_set.aset->ma.alist_ptr   = alist_ptr;

    //-------------------------------------------------------
    // Enable Dedicated mode tasks according to channel type.
    //-------------------------------------------------------
    switch(l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type)
    {
      case SDCCH_8:
      case SDCCH_4:
      {
        l1a_l1s_com.l1s_en_task[DDL]   = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[DUL]   = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[ADL]   = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[AUL]   = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[TCHTH] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHD]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTF] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHA]  = TASK_DISABLED;
      }
      break;

      case TCH_F:
      {
        l1a_l1s_com.l1s_en_task[DDL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[DUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[ADL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[AUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTH] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHD]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTF] = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[TCHA]  = TASK_ENABLED;
      }
      break;

      case TCH_H:
      {
        l1a_l1s_com.l1s_en_task[DDL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[DUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[ADL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[AUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTH] = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[TCHD]  = TASK_ENABLED;
        l1a_l1s_com.l1s_en_task[TCHTF] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHA]  = TASK_ENABLED;
      }
      break;
    }

    //----------------------------------------------
    // Set the active subchannel (case of 2 TCH/H-).
    //----------------------------------------------

    if(l1a_l1s_com.dedic_set.aset->chan2.desc.channel_type == TCH_H)
    // 2 TCHH channels are maintained... Swap from one to the other each frame.
    {
      UWORD8  fn_mod13;

      // Get FN % 13.
      fn_mod13 = l1s.actual_time.t2;  if(fn_mod13 >= 13) fn_mod13 -= 13;

      if((fn_mod13) == (2*(fn_mod13/2)))
      // Current "FN %13" is EVEN, we must set the active channel pointer
      // to the channel maintaining the subchannel 1.
      {
        if(l1a_l1s_com.dedic_set.aset->chan1.desc.subchannel == 1)
          l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan1);
        else
          l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan2);
      }
      else
      // Current "FN %13" is ODD, we must set the active channel pointer
      // to the channel maintaining the subchannel 0.
      {
        if(l1a_l1s_com.dedic_set.aset->chan1.desc.subchannel == 0)
          l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan1);
        else
          l1a_l1s_com.dedic_set.aset->achan_ptr = &(l1a_l1s_com.dedic_set.aset->chan2);
      }
    }

    //-----------------------------------------------------------------
    // T3124 timer management (HO, physical information management)...
    //-----------------------------------------------------------------

    if(l1a_l1s_com.dedic_set.aset->t3124 != 0)
    // Waiting for "PHYSICAL INFORMATION" message from BTS.
    {
      // Decrement T3124 timer.
      l1a_l1s_com.dedic_set.aset->t3124--;

      if(l1a_l1s_com.dedic_set.aset->t3124 == 0)
      // TIMEOUT: send L1C_HANDOVER_FINISHED message with a TIMEOUT indication.
      {
        UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->chan1.desc.channel_type;

        // Stop TCH/F and TCH/H (stop Omega) (used in SYNCHRO task)
        if((channel_type == TCH_F) || (channel_type == TCH_H))
        {
          l1a_l1s_com.dedic_set.stop_tch = TRUE;

	/*
	 * FreeCalypso change: same AUDIO_TASK conditional issue as earlier.
	 */
	#if 0
          // CQ: Force the Audio ON to avoid having the DSP reseting the VDLON and producing a pop noise
          // on single ended outputs.
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_ON_START;
          l1s.l1_audio_it_com = TRUE;
          l1a_l1s_com.audio_forced_by_l1s = TRUE;
	#endif
        }

	/*
	 * FreeCalypso: the following code has been reconstructed from the
	 * TCS211 binary object; it appears neither in the LoCosto nor
	 * in the TSM30 source.
	 */
        #if (AUDIO_TASK && AEC)
	  if((l1s.aec.aec_control & 0x0002) || (l1s.aec.aec_control & 0x0004)) {
	    l1s.aec.aec_control = l1a_l1s_com.aec_task.parameters.aec_control
				  | 0x0001;
	    l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl = l1s.aec.aec_control;
	  }
        #endif

        // Clear ciphering setting in MCU-DSP com.
        l1ddsp_load_ciph_param(0, &(l1a_l1s_com.dedic_set.aset->ciph_key));

        // Reset the global dedicated enable flag.
        l1a_l1s_com.l1s_en_task[DEDIC] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[DDL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[DUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[ADL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[AUL]   = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTH] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHD]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHTF] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[TCHA]  = TASK_DISABLED;

        // Reset active dedicated set.
        l1a_l1s_com.dedic_set.aset = NULL;

        // Enter to the handover fail mode.
        l1a_l1s_com.dedic_set.handover_fail_mode = TRUE;

        // SYNCHRO task and its associated parameters (tn_difference, dl_tn and
        // dsp_scheduler_mode) are not Enabled/Updated if we are in the specific case:
        // L1A is touching SYNCHRO parameters and leave L1A to go in HISR (L1S)
        // in middle of the update (cf. BUG1339)
        if(l1a_l1s_com.task_param[SYNCHRO] == SEMAPHORE_RESET)
        {
          // Since SYNCHRO has the highest priority and the installation of
          // this task in the MFTAB is made after a CLEAR of the MFTAB, we
          // insure that any uncompleted task will be ABORTED.
          l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;
        }

        // Handover access procedure is completed.
        // -> send L1C_HANDOVER_FINISHED message with "cause = TIMEOUT" to L1A.
        l1s_send_ho_finished(HO_TIMEOUT);
      }
    }
  #if FF_L1_IT_DSP_DTX
    // ASET validity check (cleared upon handover timeout)
    if (l1a_l1s_com.dedic_set.aset != NULL)
    {
      // Handover in progress (equals -1 if asynchronous)
      if(l1a_l1s_com.dedic_set.aset->ho_acc_to_send != 0)
        // Initialize timer for fast DTX availabilty
        l1a_apihisr_com.dtx.fast_dtx_ready_timer = FAST_DTX_LATENCY;

      // Dedicated channel mode change. Fast DTX interrupt can be used after some time
      if (l1a_apihisr_com.dtx.fast_dtx_ready_timer == FAST_DTX_LATENCY)
      {
        // Clear DTX interrupt condition
        l1a_apihisr_com.dtx.pending   = FALSE;
        // Enable TX activity
        l1a_apihisr_com.dtx.tx_active = TRUE;
        // No DTX status awaited
        l1a_apihisr_com.dtx.dtx_status = DTX_AVAILABLE;

        // Fast DTX shall not be used for a while
        l1a_apihisr_com.dtx.fast_dtx_ready = FALSE;
      }

      // Fast DTX latency timer handling (not on idle frame)
      if ((l1a_apihisr_com.dtx.fast_dtx_ready_timer > 0) && (l1s.next_time.fn_mod13 != 12))
        l1a_apihisr_com.dtx.fast_dtx_ready_timer--;

      // Timeout processed on a DTX block status boundary
      else if (l1a_apihisr_com.dtx.fast_dtx_ready_timer == 0)
      {
        UWORD8 channel_type;
        UWORD8 subchannel;

        channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
        subchannel   = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel;

        // Wait for DTX status boundary to enable Fast DTX use
        if ((((channel_type == TCH_F) ||
              ((channel_type == TCH_H) && (subchannel == 0))) && // TCH-AFS and TCH-AHS0
             (l1s.actual_time.fn_mod13_mod4 == 3)) ||            // DTX status reported during TDMA3
            (((channel_type == TCH_H) && (subchannel == 1)) &&   // TCH-AHS1
             (l1s.next_time.fn_mod13_mod4 == 1))                 // DTX status reported during TDMA0
           )
        {
          l1a_apihisr_com.dtx.fast_dtx_ready = TRUE;
        }
      }
    }
  #endif
  }
}

#if (MOVE_IN_INTERNAL_RAM == 0) // Must be followed by the pragma used to duplicate the funtion in internal RAM
//#pragma DUPLICATE_FOR_INTERNAL_RAM_START

#if FF_L1_IT_DSP_USF

  /*-------------------------------------------------------*/
  /* l1usf_apihisr()                                       */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function is called from the HISR handling the    */
  /* processing of the DSP USF IT interrupt. It is         */
  /* triggered upon USF decoding completion, once the      */
  /* results have been reported in API.                    */
  /* Operations executed:                                  */
  /* 1) MACS allocation update                             */
  /* 2) DSP programming                                    */
  /* 3) ABB (APC, ramps) programming                       */
  /* 4) TCR measurement handling                           */
  /* 5) TPU scenario enabling and API DB pages switch      */
  /*                                                       */
  /*-------------------------------------------------------*/

  void l1usf_apihisr(void)
  {
    // Make sure the interrupt was expected
    if (l1ps_macs_com.usf_status == USF_AWAITED)
    {
      // Flags the USF HISR before MACS/L1S update
      l1ps_macs_com.usf_status = USF_IT_DSP;

      // Test if PDTCH task is running
      if (l1s.task_status[PDTCH].current_status != INACTIVE)
      {
        // MACS allocation update (allocation then TPU, DSP and ABB)
        l1ps_ctrl_pdtch(PDTCH, BURST_3);

        // Packet transfer meas. manager update (PM control pipelines)
        l1ps_transfer_meas_manager();

        // End manager update (TPU enable and API DB switches)
        l1s_end_manager();
      }

      // Test if PRACH task is running
      else if (l1s.task_status[PRACH].current_status == ACTIVE)
      {
        // PRACH control
        l1ps_ctrl_prach(PRACH, BURST_3);

        // Packet meas. manager update (PM control pipelines)
        l1ps_meas_manager();

        // End manager update (TPU enable and API DB switches)
        l1s_end_manager();
      }

      // USF validity is no more pending
      l1ps_macs_com.usf_status = USF_AVAILABLE;
    }

    // Clear USF pending condition
    l1a_apihisr_com.usf.pending = FALSE;
  }

#endif // FF_L1_IT_DSP_USF

#if FF_L1_IT_DSP_DTX

  /*-------------------------------------------------------*/
  /* l1dtx_apihisr()                                       */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Description:                                          */
  /* ------------                                          */
  /* This function is called from the HISR handling the    */
  /* processing of the DSP DTX IT interrupt. It is         */
  /* triggered once DTX status is available and reported   */
  /* in API.                                               */
  /* Operations executed:                                  */
  /* 1) TX/DSP part of TCH programming                     */
  /* 2) Dedicated mode BA list measurements handling       */
  /* 3) TPU scenario enabling and API DB pages switch      */
  /*                                                       */
  /*-------------------------------------------------------*/

  void l1dtx_apihisr(void)
  {
    // Make sure the interrupt was expected
    if (l1a_apihisr_com.dtx.dtx_status == DTX_AWAITED)
    {
      // Flags the DTX HISR before L1S update
      l1a_apihisr_com.dtx.dtx_status = DTX_IT_DSP;

      // Latch TX activity status
      if (l1s_dsp_com.dsp_ndb_ptr->d_fast_dtx_enc_data)
        l1a_apihisr_com.dtx.tx_active = TRUE;
      else
        l1a_apihisr_com.dtx.tx_active = FALSE;

      // TCH/F is running, invoke its control function
      if (l1a_l1s_com.l1s_en_task[TCHTF] == TASK_ENABLED)
        l1s_ctrl_tchtf(TCHTF, NO_PAR);

      // TCH/H is running, invoke its control function
      else if (l1a_l1s_com.l1s_en_task[TCHTH] == TASK_ENABLED)
        l1s_ctrl_tchth(TCHTH, NO_PAR);

      // Dedicated mode measurement manager
      l1s_meas_manager();

      // End manager update (TPU enable and API DB switches)
      l1s_end_manager();

      // DTX status is no more pending
      l1a_apihisr_com.dtx.dtx_status = DTX_AVAILABLE;
    }

    // Clear DTX pending condition
    l1a_apihisr_com.dtx.pending = FALSE;
  }

#endif // FF_L1_IT_DSP_DTX

#if (FF_L1_FAST_DECODING == 1)
void l1_fast_decoding_apihisr(void)
{
 /* Tracks if a fast paging burst will be performed, an IT will happen */
  BOOL new_IT_awaited = FALSE;

  /* Clear the pending condition */
  l1a_apihisr_com.fast_decoding.pending = FALSE;

  /* In case semaphore of a task gets set in between force CRC error to be true */
  if(l1a_l1s_com.task_param[l1a_apihisr_com.fast_decoding.task] == SEMAPHORE_SET)
  {
    l1a_apihisr_com.fast_decoding.crc_error = TRUE;
  }
  /* Make sure the interrupt was expected */
  if (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_AWAITED)
  {
    /* Update the status */
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_PROCESSING;

    /* Check the NP/PNP burst number, if this is the 4th one nothing has */
    /* to be done (usual scheduling)                                     */
    if (l1a_apihisr_com.fast_decoding.deferred_control_req)
    {
      l1a_apihisr_com.fast_decoding.deferred_control_req = FALSE;

      /* Retrieve decoding status from the DSP */
      if (l1a_apihisr_com.fast_decoding.crc_error)
      {
        /* Block decoding failed, more bursts must be received */
        switch(l1a_apihisr_com.fast_decoding.task)
        {
          case NP:
          {
            l1s_ctrl_snb_dl(NP, l1a_apihisr_com.fast_decoding.burst_id);
            new_IT_awaited = TRUE;
            break;
          } /* case NP: */
#if L1_GPRS
          case PNP:
          {
            l1ps_ctrl_snb_dl(PNP, l1a_apihisr_com.fast_decoding.burst_id);
            new_IT_awaited = TRUE;
            break;
          } /* case PNP: */
#endif /* L1_GPRS */

        case NBCCHS:
          {
            l1s_ctrl_snb_dl(NBCCHS, l1a_apihisr_com.fast_decoding.burst_id);
            new_IT_awaited = TRUE;
            break;
          } /* case NBCCHS */

        } /* switch(l1a_apihisr_com.fast_decoding.task) */
      }   /* end if CRC error */
      else /* if (l1a_apihisr_com.fast_decoding.crc_error == TRUE) */
      {
#if L1_GPRS
        /* Decoding OK so no new control so no new IT to expect */
        /* reading normal burst in packet idle mode on a different timeslot so need to restore synchro*/
        if (   (l1a_apihisr_com.fast_decoding.task == NP)
            && (l1a_l1s_com.l1s_en_task[PNP] == TASK_ENABLED)
            && (l1s.algo_change_synchro_active)
           )
        {
          /* Ensure that synchro back is executed in the NMO II or III */
          l1s_restore_synchro();
          l1s.ctrl_synch_before = FALSE;
          l1s.algo_change_synchro_active = FALSE;
        }
#endif /* L1_GPRS */
      } /* end else no CRC error */

      /* In any case power measurements schedulers and end of frame processing */
      /* must be executed now                                                  */
#if L1_GPRS
      /* Packet Measurement Manager */
      l1ps_meas_manager();
#endif
      /* Measurement Manager */
      l1s_meas_manager();
      /* End Manager */
      l1s_end_manager();
    }
    else /* if (l1a_apihisr_com.fast_decoding.control_required) */
    {
      /* No new control so no new IT to expect */
    }
  } /* if (l1a_apihisr_com.fast_decoding.status == C_FAST_DECODING_AWAITED) */
  else
  {
    /* Interrupt received but not expected, this shouldn't happen */
    l1_trace_IT_DSP_error(IT_DSP_ERROR_FAST_DECODING_UNEXP);
  }

  /* Status update depending on CRC and the scheduling of another bursts */
  if (new_IT_awaited)
  {
    /* New IT awaited */
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_AWAITED;
  }
  else if (l1a_apihisr_com.fast_decoding.crc_error == FALSE)
  {
    /* No new IT awaited, the CRC is OK, fast decoding is complete */
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_COMPLETE;
  }
  else
  {
    /* No new IT awaited and CRC is still fail, typically 4th burst received */
    /* but decoding has failed.                                              */
    l1a_apihisr_com.fast_decoding.status = C_FAST_DECODING_NONE;
  }
}   /* end function l1_fast_decoding_apihisr */
#endif /* FF_L1_FAST_DECODING */

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END
#endif // MOVE_IN_INTERNAL_RAM


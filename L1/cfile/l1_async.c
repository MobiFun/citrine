/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_ASYNC.C
 *
 *        Filename l1_async.c
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

  #define L1_ASYNC_C

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END           // KEEP IN EXTERNAL MEM otherwise
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_START
#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_signa.h"
  #include "cust_os.h"
  #if TESTMODE
    #include "l1tm_defty.h"
    #include "l1tm_signa.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_proto.h"
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #if (L1_DYN_DSP_DWNLD == 1)
    #include "l1_dyn_dwl_signa.h"
  #endif

  #include "l1_defty.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"

  #if L1_GPRS
    #include "l1p_mfta.h"
    #include "l1p_msgt.h"
    #include "l1p_cons.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif

  #include "l1_tabs.h"

  #if (VCXO_ALGO == 1)
    #include "l1_ctl.h"
  #endif

  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif
#else
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"
  #include "../../gpf/inc/cust_os.h"
  #if TESTMODE
    #include "l1tm_defty.h"
    #include "l1tm_signa.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
    #include "l1audio_signa.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
    #include "l1mp3_signa.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_proto.h"
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
    #include "l1aac_signa.h"
  #endif
  #if (L1_DYN_DSP_DWNLD == 1)
    #include "l1_dyn_dwl_signa.h"
  #endif

  #include "l1_defty.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"

  #if L1_GPRS
    #include "l1p_mfta.h"
    #include "l1p_msgt.h"
    #include "l1p_cons.h"
    #include "l1p_deft.h"
    #include "l1p_vare.h"
    #include "l1p_sign.h"
  #endif

  #include "l1_tabs.h"

  #if L1_RECOVERY
    #if ((CHIPSET == 12) || (CHIPSET == 15))
        #include "sys_inth.h"
    #else
        #include "../../bsp/iq.h"
        #include "../../bsp/inth.h"
        #include "../../bsp/mem.h"
    #endif
  #endif

  #if (VCXO_ALGO == 1)
    #include "l1_ctl.h"
  #endif

  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif

  #if (OP_L1_STANDALONE == 1)
    #include "dynamic_clock.h"
  #endif
#endif

// External function prototypes
extern void l1ctl_gauging  (UWORD32 nb_32khz, UWORD32 nb_hf);
extern void l1_tpu_init_light(void);
extern void tm_receive(UWORD8 *inbuf, UWORD16 size);

#if L1_RECOVERY
  extern void l1_initialize_for_recovery(void);
  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    extern void l1_trace_recovery(void);
  #endif
#endif

#if (L1_GTT == 1)
  extern void l1a_mmi_gtt_process               (xSignalHeaderRec *msg);
#endif

#if(L1_DYN_DSP_DWNLD == 1)
  extern void l1a_dyn_dsp_dwnld_process(xSignalHeaderRec *msg);
#endif

#if (AUDIO_TASK == 1)
  /**************************************/
  /* External audio prototypes          */
  /**************************************/
  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      // driver task
      extern void l1a_audio_driver_process        (xSignalHeaderRec *msg);
    #endif
  #endif
  #if (KEYBEEP)
    // Keybeep task process
    extern void l1a_mmi_keybeep_process         (xSignalHeaderRec *msg);
  #endif
  #if (TONE)
    // Keybeep task process
    extern void l1a_mmi_tone_process            (xSignalHeaderRec *msg);
  #endif
  #if (L1_CPORT == 1)
    // Cport task process
    extern void l1a_mmi_cport_process           (xSignalHeaderRec *msg);
  #endif
  #if (MELODY_E1)
    // Melody 0 task process
    extern void l1a_mmi_melody0_process         (xSignalHeaderRec *msg);
    // Melody 1 task process
    extern void l1a_mmi_melody1_process         (xSignalHeaderRec *msg);
  #endif
  #if (VOICE_MEMO)
    // Voice memo playing process
    extern void l1a_mmi_vm_playing_process      (xSignalHeaderRec *msg);
    // Voice memo recording process
    extern void l1a_mmi_vm_recording_process    (xSignalHeaderRec *msg);
  #endif
  #if (L1_PCM_EXTRACTION)
    /* PCM download process */
    void l1a_mmi_pcm_download_process           (xSignalHeaderRec *msg);
    /* PCM upload process */
    void l1a_mmi_pcm_upload_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_VOICE_MEMO_AMR)
    // Voice memo amr playing process
    extern void l1a_mmi_vm_amr_playing_process  (xSignalHeaderRec *msg);
    // Voice memo amr recording process
    extern void l1a_mmi_vm_amr_recording_process(xSignalHeaderRec *msg);
  #endif
  #if (SPEECH_RECO)
    // Speech recognition enrollment process
    extern void l1a_mmi_sr_enroll_process       (xSignalHeaderRec *msg);
    // Speech recognition update process
    extern void l1a_mmi_sr_update_process       (xSignalHeaderRec *msg);
    // Speech recognition reco process
    extern void l1a_mmi_sr_reco_process         (xSignalHeaderRec *msg);
    // Speech recognition update-check process
    extern void l1a_mmi_sr_update_check_process (xSignalHeaderRec *msg);
  #endif
  #if (L1_AEC == 1)
    // AEC process
    extern void l1a_mmi_aec_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_AEC == 2)
    // AEC process
    extern void l1a_mmi_aec_process             (xSignalHeaderRec *msg);
  #endif
  #if (FIR)
    // FIR process
    extern void l1a_mmi_fir_process             (xSignalHeaderRec *msg);
  #endif
  #if (AUDIO_MODE)
    // AUDIO MODE process
    extern void l1a_mmi_audio_mode_process      (xSignalHeaderRec *msg);
  #endif
  #if (MELODY_E2)
    extern void l1a_mmi_melody0_e2_process      (xSignalHeaderRec *msg);
    extern void l1a_mmi_melody1_e2_process      (xSignalHeaderRec *msg);
  #endif
  #if (L1_MP3 == 1)
    extern void l1a_mmi_mp3_process             (xSignalHeaderRec *msg);
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    extern void l1a_mmi_aac_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1 || L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    extern void l1a_mmi_audio_onoff_process       (xSignalHeaderRec *msg);
  #endif
  #if (L1_EXT_AUDIO_MGT == 1)
    // External audio_mgt task process
    extern void l1a_mmi_ext_audio_mgt_process   (xSignalHeaderRec *msg);
  #endif
  #if (L1_ANR == 1 || L1_ANR == 2)
    extern void l1a_mmi_anr_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_IIR == 1 || L1_IIR == 2)
    extern void l1a_mmi_iir_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_WCM == 1)
    extern void l1a_mmi_wcm_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_AGC_UL == 1)
    extern void l1a_mmi_agc_ul_process          (xSignalHeaderRec *msg);
  #endif
  #if (L1_AGC_DL == 1)
    extern void l1a_mmi_agc_dl_process          (xSignalHeaderRec *msg);
  #endif
#if (L1_DRC == 1)
    extern void l1a_mmi_drc_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_LIMITER == 1)
    extern void l1a_mmi_limiter_process         (xSignalHeaderRec *msg);
  #endif
  #if (L1_ES == 1)
    extern void l1a_mmi_es_process              (xSignalHeaderRec *msg);
  #endif
  #if (L1_VOCODER_IF_CHANGE == 1)
    extern void l1a_mmi_vocoder_cfg_process    (xSignalHeaderRec *msg);

  #endif //VOCODER_IF_CHANGE == 1

  #if(L1_BT_AUDIO ==1)
    extern void l1a_mmi_bt_process(xSignalHeaderRec *msg); 
  #endif  

    extern void l1a_mmi_outen_cfg_process      (xSignalHeaderRec *msg);
#endif

#if(L1_CHECK_COMPATIBLE == 1)
    extern void l1a_checkmsg_compatibility    (xSignalHeaderRec *msg);
#endif

#if TESTMODE
  void l1a_tmode(xSignalHeaderRec *msg);
  #if (OP_L1_STANDALONE == 0)
    typedef void (*TM_CALLBACK_FUNC)(UWORD8*, UWORD16);
    extern int etm_register(char name[], int mid, int task_id,
                            UWORD16 addr_id, TM_CALLBACK_FUNC callback);
  #endif
#endif


#if L1_GPRS
  void  l1pa_task(xSignalHeaderRec *msg);
#endif

#if (TRACE_TYPE == 1) ||(TRACE_TYPE == 4) || (TRACE_TYPE == 7) || (TESTMODE)
  #include "l1_trace.h"

  extern T_RVT_USER_ID tm_trace_user_id;
  extern void l1_trace_configuration(T_RVT_BUFFER trace_msg, UINT16 trace_msg_size);
#endif

//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#if (TRACE_TYPE == 1) ||(TRACE_TYPE == 4) || (TRACE_TYPE == 7) || (TESTMODE)
  T_RVT_USER_ID tm_trace_user_id;
#endif


#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM !=0))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM enabled
//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1a_task()                                            */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* L1A (Layer 1 Asynchronous) task function. This        */
/* function manages the interface between L3 and L1. It  */
/* is composed with a set of state machine, each machine */
/* handles a particular GSM functionality. When a        */
/* message is received in L1_C1 message queue, it is     */
/* submitted to every state machine. The one which are   */
/* impacted by the message process it. At the end of     */
/* "l1a_task()" function, a balance routine is called,   */
/* it enables L1S tasks consequently to the state machine*/
/* requests.                                             */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_task(UWORD32 argc, void *argv)
{
  xSignalHeaderRec *msg;

  #if (TRACE_TYPE == 7)
    rvt_register_id("L1", &trace_info.l1_trace_user_id, (RVT_CALLBACK_FUNC)NULL);
  #endif

  #if ((TESTMODE) && (CODE_VERSION != SIMULATION))
    // Testmode register for UART TX only in standalone L1
    #if (OP_L1_STANDALONE == 0)
      // Register the tm_receive func. as the TM3 packet receptor in the ETM database
      etm_register("TML1", 0x0, 0, 0, tm_receive); // 0x00 = use of TM3 Signalling PC<->MS
    #else
      rvt_register_id("TM", &tm_trace_user_id, tm_receive);
    #endif // End of OP_L1_STANDALONE
  #endif // End of TESTMODE && (CODE_VERSION != SIMULATION)

  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    // Enable auto-test HW in order to trace versions
    trace_info.init_trace    = 1;
    l1a_l1s_com.l1s_en_task[HWTEST] = TASK_ENABLED;
    l1a_l1s_com.l1a_activity_flag = TRUE;
  #endif

  while(1)
  /*************************************************************/
  /* LOOP on L1A message queue...                              */
  /*************************************************************/
  {
    UWORD8  process;

    /***********************************************/
    /*  Wait for a new message from RRM1 or L1S    */
    /***********************************************/
    msg = os_receive_sig(L1C1_QUEUE);
    DEBUGMSG(status,NU_RCVE_QUEUE_ERR)

    // Reset "l1_msg_forwarded" flag for this new incoming msg.
    // ========================================================
    l1a.l1_msg_forwarded = FALSE;

    #if (TRACE_TYPE==1) || (TRACE_TYPE==4) || (TRACE_TYPE==7)
        l1_trace_message(msg);
    #endif

#if(L1_CHECK_COMPATIBLE == 1)
       l1a_checkmsg_compatibility(msg);
#endif

    #if (DRP_FW_EXT == 1)
    if(!l1s.boot_result)
    {
    #endif /* DRP_FW_EXT*/
#if (GSM_IDLE_RAM <= 1)
    #if TESTMODE
      // if NOT in TestMode, go through normal mode processes
      if (l1_config.TestMode == 0)
      {
    #endif

        #if(L1_DYN_DSP_DWNLD==1)
          // Dynamic download L1A state machine
          l1a_dyn_dsp_dwnld_process(msg);
        #endif

        #if L1_GPRS
          // Pass the new msg to packet L1A.
          l1pa_task(msg);
        #endif

    #if TESTMODE
      }
    #endif
#endif

        #if (GSM_IDLE_RAM != 0)
            #if L1_GPRS
              if ((msg->SignalCode != L1C_NP_INFO) &&
                  (msg->SignalCode != L1C_EP_INFO) &&
                  (msg->SignalCode != L1C_RXLEV_PERIODIC_DONE) &&
                  (msg->SignalCode != L1P_PNP_INFO) &&
                  (msg->SignalCode != L1P_PEP_INFO) &&
                  (l1a_l1s_com.mode == I_MODE))
            #else
              if ((msg->SignalCode != L1C_NP_INFO) &&
                  (msg->SignalCode != L1C_EP_INFO) &&
                  (msg->SignalCode != L1C_RXLEV_PERIODIC_DONE) &&
                  (l1a_l1s_com.mode == I_MODE))
            #endif

          #if (GSM_IDLE_RAM > 1) // GPF modified for GSM_IDLE_RAM -> SW still running in Internal RAM
                {
          #endif
                  {
                     l1s.gsm_idle_ram_ctl.l1s_full_exec = TRUE;
                  }

        #endif // GSM_IDLE_RAM

    // Clear L1A "enable meas and tasks" variables.
    //---------------------------------------------
    for(process=0; process<NBR_L1A_PROCESSES; process++)
    {
      l1a.l1a_en_meas[process] = NO_TASK;
    }

    // Go through all processes...
    /******************************/

#if TESTMODE
  // if NOT in TestMode, go through normal mode processes
  if (l1_config.TestMode == 0)
  {
#endif

    // Hardware test:
    //---------------
    l1a_test_process(msg);

    #if (OP_L1_STANDALONE == 1)
       // Dynamic configuration : clock management for test
       l1a_test_config_process(msg);
    #endif

    #if (TRACE_TYPE==3)
      // Statistics or Test process:
      //---------------------------
      l1a_stats_process(msg);
    #endif

    // ADC conversion:
    //----------------
    l1a_mmi_adc_req(msg);

    // Frequency Band configuration: GSM900, E-GSM900, DCS1800, DUAL, DUALEXT, PCS 1900
    //----------------------------------------------------------------------------------
    l1a_freq_band_configuration(msg);

    // Network synchronization process:
    //---------------------------------

    // Synchronization with a Neighbour cell for Cell Selection.
    l1a_initial_network_sync_process(msg);

    // lost Network
    l1a_network_lost(msg);

    // Cell Selection/Idle processes:
    //-------------------------------

    // Full list receive level monitoring.
    l1a_full_list_meas_process(msg);

    // 6 strongest Neighbor cells synchro. monitoring.
    l1a_idle_6strongest_monitoring_process(msg);

    // 6 strongest Neighbor cells BCCH reading.
    l1a_neighbour_cell_bcch_reading_process(msg);

    // Serving Cell BCCH reading.
    l1a_idle_serving_cell_bcch_reading_process(msg);
#if (GSM_IDLE_RAM <= 1)
    // Serving Cell PAGING reading.
    l1a_idle_serving_cell_paging_process(msg);
#endif
    // Short Message Servive Cell Broadcast reading.
    l1a_idle_smscb_process(msg);
#if (GSM_IDLE_RAM <= 1)
    // BA list (periodic) power monitoring.
    l1a_idle_ba_list_meas_process(msg);
#endif

    // Cell Reselection processes:
    //----------------------------

    // Synchronization and requested BCCH reading.
    // --> camp on new serving cell.
    l1a_cres_process(msg);


    // Connection Establishment processes (also called "Link Access"):
    //----------------------------------------------------------------

    // Link Access process.
    l1a_access_process(msg);


    // Dedicated mode processes:
    //--------------------------

    // Dedicated mode process.
    l1a_dedicated_process(msg);

    // 6 strongest Neighbor cells synchro. monitoring and BCCH reading.
    l1a_dedic6_process(msg);

    // BA list (periodic) power monitoring.
    l1a_dedic_ba_list_meas_process(msg);

    #if (L1_GTT == 1)
      // GTT process handling.
      l1a_mmi_gtt_process(msg);
    #endif

    #if (AUDIO_TASK == 1)
      #if (OP_RIV_AUDIO == 1)
        #if (L1_AUDIO_DRIVER == 1)
          l1a_audio_driver_process(msg);
        #endif
      #endif
      #if (KEYBEEP)
        // Keybeep task process
        l1a_mmi_keybeep_process(msg);
      #endif
      #if (TONE)
        // Tone task process
        l1a_mmi_tone_process(msg);
      #endif
      #if (L1_CPORT == 1)
        // Cport task process
        l1a_mmi_cport_process(msg);
      #endif
      #if (MELODY_E1)
        // Melody 0 task process
        l1a_mmi_melody0_process(msg);
        // Melody 1 task process
        l1a_mmi_melody1_process(msg);
      #endif
      #if (VOICE_MEMO)
        // Voice memo playing process
        l1a_mmi_vm_playing_process(msg);
        // Voice memo recording process
        l1a_mmi_vm_recording_process(msg);
      #endif
      #if  (L1_PCM_EXTRACTION)
        /* PCM download process */
        l1a_mmi_pcm_download_process(msg);
        /* PCM upload process */
        l1a_mmi_pcm_upload_process(msg);
      #endif
      #if (L1_VOICE_MEMO_AMR)
        // Voice memo amr playing process
        l1a_mmi_vm_amr_playing_process(msg);
        // Voice memo amr recording process
        l1a_mmi_vm_amr_recording_process(msg);
      #endif
      #if (SPEECH_RECO)
        // Speech recognition enrollment process
        l1a_mmi_sr_enroll_process(msg);
        // Speech recognition update process
        l1a_mmi_sr_update_process(msg);
        // Speech recognition reco process
        l1a_mmi_sr_reco_process(msg);
        // Speech recognition update-check process
        l1a_mmi_sr_update_check_process(msg);
      #endif
      #if (L1_AEC == 1)
        // AEC process
        l1a_mmi_aec_process(msg);
      #endif
      #if (L1_AEC == 2)
        // AEC process
        l1a_mmi_aec_process(msg);
      #endif
      #if (FIR)
        // FIR process
        l1a_mmi_fir_process(msg);
      #endif
      #if (AUDIO_MODE)
        // AUDIO MODE process
        l1a_mmi_audio_mode_process(msg);
      #endif
      #if (MELODY_E2)
        // MELODY E2 process
        l1a_mmi_melody0_e2_process(msg);
        l1a_mmi_melody1_e2_process(msg);
      #endif
      #if (L1_MP3==1)
        // MP3 process
        l1a_mmi_mp3_process(msg);
      #endif
      #if (L1_MIDI==1)
        // MIDI process
        l1a_mmi_midi_process(msg);
      #endif
//ADDED FOR AAC
      #if (L1_AAC==1)
        // AAC process
        l1a_mmi_aac_process(msg);
      #endif
      #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1 || L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
        l1a_mmi_audio_onoff_process(msg);
      #endif
      #if (L1_EXT_AUDIO_MGT == 1)
        // External audio_mgt task process
        l1a_mmi_ext_audio_mgt_process(msg);
      #endif
      #if (L1_ANR == 1 || L1_ANR == 2)
        l1a_mmi_anr_process(msg);
      #endif
      #if (L1_IIR == 1 || L1_IIR == 2)
        l1a_mmi_iir_process(msg);
      #endif
      #if (L1_AGC_UL == 1)
        l1a_mmi_agc_ul_process(msg);
      #endif
      #if (L1_AGC_DL == 1)
        l1a_mmi_agc_dl_process(msg);
      #endif
      #if (L1_WCM == 1)
        l1a_mmi_wcm_process(msg);
      #endif
      #if (L1_DRC == 1)
        l1a_mmi_drc_process(msg);
      #endif
      #if (L1_LIMITER == 1)
        l1a_mmi_limiter_process(msg);
      #endif
      #if (L1_ES == 1)
        l1a_mmi_es_process(msg);
      #endif
      #if (L1_VOCODER_IF_CHANGE == 1)
        l1a_mmi_vocoder_cfg_process(msg);
      #endif // L1_VOCODER_IF_CHANGE == 1
      #if(L1_BT_AUDIO ==1)
        l1a_mmi_bt_process(msg); 
      #endif // L1_VOCODER_IF_CHANGE == 1
      /*
       * FreeCalypso change: the following call to l1a_mmi_outen_cfg_process()
       * (a function that doesn't exist in the Leonardo objects) has been
       * moved inside the #if (AUDIO_TASK == 1) conditional, otherwise
       * the link fails - it is definitely an audio function of some kind.
       */
      l1a_mmi_outen_cfg_process(msg);
    #endif //AUDIO TASK

    // Only processes supported by GSM IDLE in Internal RAM
    #if (GSM_IDLE_RAM > 1)
       #if L1_GPRS
          if ((msg->SignalCode >> 8) == P_GPRS)
            l1pa_task(msg);
       #endif

       // Serving Cell PAGING reading.
       l1a_idle_serving_cell_paging_process(msg);

       // BA list (periodic) power monitoring.
       l1a_idle_ba_list_meas_process(msg);
    #endif // GSM_IDLE_RAM > 1

#if TESTMODE
  } // end if not in TestMode

    //TestMode State Machine
    l1a_tmode(msg);
#endif // TESTMODE

    // Make a balance for L1A "tasks and meas tasks".
    //===============================================
    l1a_balance_l1a_tasks();

    l1a_l1s_com.l1a_activity_flag = TRUE;
#if (DRP_FW_EXT == 1)
  } /* end if DRP boot success boot_result == 0 */
#endif

    // Deallocate memory for the received message if msg not forwarded to L3.
    // ----------------------------------------------------------------------
    if(l1a.l1_msg_forwarded == FALSE)
    {
      os_free_sig(msg);
      DEBUGMSG(status,NU_DEALLOC_ERR)
    }
  } // end while().
} // end of procedure.

//-----------------------------------------------------------------------------------------------

/*-------------------------------------------------------*/
/* l1a_balance_l1a_tasks()                               */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function enables the L1S tasks consequently to   */
/* the L1A state machine requests.                       */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_balance_l1a_tasks()
{
  UWORD8   process;
  UWORD32  l1a_en_meas    = 0;
#if L1_GPRS
  UWORD32  l1pa_en_meas   = 0;
#endif
  UWORD32  l1a_dl_en_task = 0;

  for(process=0; process<NBR_L1A_PROCESSES; process++)
  {
    // Balance for measurement tasks.
    l1a_en_meas    |= l1a.l1a_en_meas[process];
  }

  #if L1_GPRS
    for(process=0; process<NBR_L1PA_PROCESSES; process++)
    {
      // Balance for measurement tasks.
      l1pa_en_meas    |= l1pa.l1pa_en_meas[process];
    }

    // Balance for Packet measurement tasks.
    l1pa_l1ps_com.l1ps_en_meas |= l1pa_en_meas;
    #if (TRACE_TYPE==5) && FLOWCHART
      trace_flowchart_l1tsk(l1pa_en_meas, l1pa.l1pa_en_meas);
    #endif

  #endif

  // Balance for measurement tasks.
  l1a_l1s_com.l1s_en_meas |= l1a_en_meas;
  #if (TRACE_TYPE==5) && FLOWCHART
    trace_flowchart_l1tsk(l1a_en_meas, l1a.l1a_en_meas);
  #endif

} // end of procedure.

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif
//-----------------------------------------------------------------------------------------------

#if (TRACE_TYPE==3)
/*-------------------------------------------------------*/
/* l1a_stats_process()                                   */
/*-------------------------------------------------------*/
/* Description : This function handles statistics        */
/* or test mode setting.                                 */
/*                                                       */
/* Starting messages:        L1_STATS_REQ.               */
/*                           L1_PLAY_REQ.                */
/*                                                       */
/* Result messages (input):  none                        */
/* Result messages (output): none                        */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_stats_process(xSignalHeaderRec *msg)
{
  UWORD32  SignalCode = msg->SignalCode;

  if(SignalCode == L1_STATS_REQ)
  {
    // store type of burst to spy...
    l1_stats.type = ((T_L1_STATS_REQ *)(msg->SigP))->type;
    // initialize variables and buffer for statistics...
    initialize_stats();
  }
}
#endif

/*-------------------------------------------------------*/
/* l1a_test_process()                                    */
/*-------------------------------------------------------*/
/* Description : This function tests hardware and DSP.   */
/*                                                       */
/* Starting messages:        TST_TEST_HW_REQ             */
/*                                                       */
/* Result messages (input):  TST_TEST_HW_CON             */
/* Result messages (output): none                        */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_test_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1,
    WAIT_RESULT = 2
  };

  UWORD8   *state      = &l1a.state[HW_TEST];
  UWORD32   SignalCode = msg->SignalCode;

  BOOL end_process = 0;


  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    if (trace_info.init_trace == 1)
    {
      *state = WAIT_RESULT;

      if(SignalCode == TST_TEST_HW_REQ)
      {
        trace_info.init_trace = 0;
      }
    }
  #endif

   while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset tasks used in the process.
        l1a_l1s_com.l1s_en_task[HWTEST] = TASK_DISABLED;  // Clear  task enable flag.

      }
      break;

      case WAIT_INIT:
      {
        // Use incoming message
        //---------------------
        if(SignalCode == TST_TEST_HW_REQ)
        {
          *state = WAIT_RESULT;

          // set tasks used in the process.
          l1a_l1s_com.l1s_en_task[HWTEST] = TASK_ENABLED;  // Set task enable flag.
        }
        // End of process.
        end_process = 1;

      }
      break;

      case WAIT_RESULT:
      {

        // Use incoming message
        //---------------------
        if(SignalCode == L1_TEST_HW_INFO)
        {
          #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            if (trace_info.init_trace == 0)
          #endif
              // Forward result message to L3.
              l1a_send_result(TST_TEST_HW_CON, msg, RRM1_QUEUE);
          #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            else
              trace_info.init_trace = 0;
          #endif

          *state = RESET;
        }
        else
        {
           end_process = 1;
        }
      }
      break;
    }
  }
}

#if (OP_L1_STANDALONE == 1)
/*-------------------------------------------------------*/
/* l1a_test_config_process()                             */
/*-------------------------------------------------------*/
/* Description : This function allows modify the original*/
/*               initialization of hardware parameters   */
/*               like for example clock configuration.   */
/*                                                       */
/* Starting messages:        TST_HW_CONFIG_REQ           */
/*                           TST_SW_CONFIG_REQ           */
/*                                                       */
/* Result messages (input):  none                        */
/* Result messages (output): none                        */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_test_config_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1
  };

  UWORD8   *state      = &l1a.state[HSW_CONF];
  UWORD32   SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // step in state machine.
        *state = WAIT_INIT;

        // Reset tasks used in the process.
        //l1a_l1s_com.l1s_en_task[HSWCONF] = TASK_DISABLED;  // Clear  task enable flag.
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == TST_HW_CONFIG_REQ)
        /*
         * Depending on the arguments of the message,
         * actions are different (clock config, ABB config, ...).
         * The first argument is for clock configuration.
         */
        {
          #if  (CHIPSET == 4) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || \
               (CHIPSET == 11) || (CHIPSET == 12) || (CHIPSET == 15)
          /* Only SAMSON/CALYPSO families are considered for dynamic clock configuration.*/
          UWORD8        d_clock_cfg;    // Num of selected clock configuration.

          /*
           * Read num of selected clock configuration as first arg of message
           */
          d_clock_cfg = ((T_TST_HW_CONFIG_REQ *)(msg->SigP))->num_of_clock_cfg;

          /*
           * Check if dynamic clock configuration requested.
           * If FFh, no dynamic clock configuration is requested.
           */
        #if (CODE_VERSION != SIMULATION)
          if (d_clock_cfg != C_CLOCK_NO_CFG)
          {
            /* If wrong index, keep the current clock configuration unchanged */
            f_dynamic_clock_cfg(d_clock_cfg);
          }
        #endif
          /* Only SAMSON/CALYPSO families are considered for dynamic clock configuration.*/
          #endif   // CHIPSET = 4 or 7 or 8 or 10 or 11 or 12

          // set tasks used in the process.
          //l1a_l1s_com.l1s_en_task[HSWCONF] = TASK_ENABLED;  // Set task enable flag.

          // step in state machine.
          *state = RESET;
        }

        if(SignalCode == TST_SW_CONFIG_REQ)
        /*
         * Depending on the arguments of the message,
         * actions are different.
         */
        {
        #if IDS
          l1_config.ids_enable = ((T_TST_SW_CONFIG_REQ *)(msg->SigP))->ids_enable;
        #endif

          l1_config.facch_test.enable = ((T_TST_SW_CONFIG_REQ *)(msg->SigP))->facch_test.enable;
          l1_config.facch_test.period = ((T_TST_SW_CONFIG_REQ *)(msg->SigP))->facch_test.period;

          if ((l1_config.facch_test.enable) &&
              (l1_config.facch_test.period == 0))
          {
            l1_config.facch_test.period = 1;
          }

          // set tasks used in the process.
          //l1a_l1s_com.l1s_en_task[HSW_CONF] = TASK_ENABLED;  // Set task enable flag.

          // step in state machine.
          *state = RESET;
        }
        // End of process.
        end_process = 1;
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.
#endif // OP_L1_STANDALONE


/*-------------------------------------------------------*/
/* l1a_mmi_adc_req ()                                    */
/*-------------------------------------------------------*/
/* Description : This function is in charge of requesting*/
/*               L1 to trigger an ADC conversion sequence*/
/*                                                       */
/* Starting messages:        MMI_ADC_REQ                 */
/*                                                       */
/* Result messages (input):  none                        */
/* Result messages (output): none                        */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_mmi_adc_req(xSignalHeaderRec *msg)
{
  UWORD32  SignalCode = msg->SignalCode;
  UWORD8   *state      = &l1a.state[I_ADC];

  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1
  };

  BOOL end_process = 0;

  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        l1a_l1s_com.adc_cpt  = 0;
        l1a_l1s_com.adc_mode = ADC_DISABLED;

        // Disable ADC tasks used in CS_MODE0
        l1a_l1s_com.l1s_en_task[ADC_CSMODE0] = TASK_DISABLED;  // Reset ADC task enable flag.

        // Step in state machine.
        *state = WAIT_INIT;
      }
      break;

      case WAIT_INIT:
      {
        // Use incoming message
        //---------------------
        if(SignalCode == MMI_ADC_REQ)
        {
          UWORD8 tx_flag;

          tx_flag                        = ((T_MMI_ADC_REQ *)(msg->SigP))-> tx_flag;
          l1a_l1s_com.adc_traffic_period = ((T_MMI_ADC_REQ *)(msg->SigP))-> traffic_period;
          l1a_l1s_com.adc_idle_period    = ((T_MMI_ADC_REQ *)(msg->SigP))-> idle_period;

          // -------------------------------
          // Activation for Idle and CS_MODE
          // -------------------------------

          l1a_l1s_com.adc_mode = ADC_DISABLED;
          l1a_l1s_com.adc_cpt  = 0;

          if (l1a_l1s_com.adc_idle_period == 0)
          { // performed only one time
            l1a_l1s_com.adc_mode |= ADC_NEXT_NORM_PAGING
                                 |  ADC_NEXT_MEAS_SESSION
                                 |  ADC_NEXT_CS_MODE0 ;
          }
          else
          { // performed on each periode
            l1a_l1s_com.adc_mode |= ADC_EACH_NORM_PAGING
                                 |  ADC_EACH_MEAS_SESSION
                                 |  ADC_EACH_CS_MODE0 ;
          }

          // ----------------------------------------------------------
          // Activation for Dedicated and Connection Establishment mode
          // ----------------------------------------------------------
          if (tx_flag == 1) // ADC is performed inside a TX burst
          {
            l1a_l1s_com.adc_mode |= ADC_EACH_RACH; // traffic_period is meaningless for RACH / PRACH

            if (l1a_l1s_com.adc_traffic_period == 0)
            {  // performed only one time
              l1a_l1s_com.adc_mode |= ADC_NEXT_TRAFFIC_UL;
            }
            else
            {  // performed on each periode
              l1a_l1s_com.adc_mode |= ADC_EACH_TRAFFIC_UL;
            }
          }
          else // ADC is performed outside a TX burst
          {
            if (l1a_l1s_com.adc_traffic_period == 0)
            { // performed only one time
              l1a_l1s_com.adc_mode |= ADC_NEXT_TRAFFIC_DL;
            }
            else
            { // performed on each periode
              l1a_l1s_com.adc_mode |= ADC_EACH_TRAFFIC_DL;
            }

            if (l1a_l1s_com.adc_idle_period == 0)
            { // performed only one time
              l1a_l1s_com.adc_mode |= ADC_NEXT_NORM_PAGING_REORG;
            }
            else
            { // performed on each periode
              l1a_l1s_com.adc_mode |= ADC_EACH_NORM_PAGING_REORG;
            }
          }
          l1a_l1s_com.l1s_en_task[ADC_CSMODE0] = TASK_ENABLED;  // enable ADC task for CS_MODE0
        }
        else
        if(SignalCode == MMI_STOP_ADC_REQ)
        {
          // send confirmation message
          l1a_send_confirmation(MMI_STOP_ADC_CON,RRM1_QUEUE);

          // Disable ADC tasks used in CS_MODE0
          // It is mandatory to disable the task here because in CS_MODE0 there is no new messages
          // to validate the RESET state in the state machine.
          l1a_l1s_com.l1s_en_task[ADC_CSMODE0] = TASK_DISABLED;  // Reset ADC task enable flag.

          // This process must be reset.
          *state = RESET;
        }

        // End of process.
        end_process = 1;
      }
      break;
    }
  }
}


/*-------------------------------------------------------*/
/* l1a_network_lost()                                    */
/*-------------------------------------------------------*/
/* Description : This function allows the MS to enter in */
/*               deep sleep while it is performig cell   */
/*               selection full power measurement        */
/*                                                       */
/* Starting messages:        MPHC_NETWORK_LOST_IND       */
/*                                                       */
/* Result messages (input):  none                        */
/* Result messages (output): none                        */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_network_lost(xSignalHeaderRec *msg)
{
  UWORD32  SignalCode = msg->SignalCode;

  if(SignalCode == MPHC_NETWORK_LOST_IND)
  {
    // set new mode in order to allow the deep sleep
    l1a_l1s_com.mode = CS_MODE0;

    // reset the gauging algorithm in order to re-compute
    // the 32khz/13Mhz when re-enter in IDLE mode.
    l1ctl_gauging (0,0);
  }
}

#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM > 1))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM == 2
//#pragma GSM_IDLE2_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1a_idle_ba_list_meas_process()                       */
/*-------------------------------------------------------*/
/* Description : This state machine handles neigbor cell */
/* measurement process in IDLE mode with BA list.        */
/*                                                       */
/* Starting messages:        MPHC_RXLEV_PERIODIC_REQ     */
/* ------------------                                    */
/*  L1 starts then the periodic BA list receive level    */
/*  monitoring.                                          */
/*                                                       */
/* Subsequent messages:      MPHC_RXLEV_PERIODIC_REQ     */
/* --------------------                                  */
/*  L1 changes the BA list and starts the periodic BA    */
/*  list receive level monitoring with this new list.    */
/*                                                       */
/* Result messages (input):  L1C_RXLEV_PERIODIC_DONE     */
/* ------------------------                              */
/*  This is the periodic reporting message from L1s.     */
/*                                                       */
/* Result messages (output): MPHC_RXLEV_PERIODIC_IND     */
/* -------------------------                             */
/*  This is the periodic reporting message to L3.        */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_RXLEV_PERIODIC_REQ*/
/* -----------------------                               */
/*  BA list neigbor cell measurement process in IDLE     */
/*  is stopped by this message.                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_idle_ba_list_meas_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1,
    WAIT_RESULT = 3
  };

  UWORD8   *state      = &l1a.state[I_NMEAS];
  UWORD32   SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // step in state machine.
        *state = WAIT_INIT;

        // Reset I_NMEAS process.
        l1a_l1s_com.l1s_en_meas &= I_BAMS_MEAS_MASK; // Reset I_BAMS Measurement enable flag.
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == MPHC_RXLEV_PERIODIC_REQ)
        // We receive the BA list to be monitored.
        //----------------------------------------
        {
          UWORD8        ba_id;
          UWORD8        nbchans;
          TC_CHAN_LIST  *listptr;
          UWORD8        next_radio_freq_measured;
          UWORD8        i;

          nbchans =     ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->num_of_chans;
          listptr =   &(((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->chan_list);
          ba_id   =     ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->ba_id;
          next_radio_freq_measured
                  = ((T_MPHC_RXLEV_PERIODIC_REQ *)(msg->SigP))->next_radio_freq_measured;
          // Set parameter synchro semaphore for I_BAMS task.
          l1a_l1s_com.meas_param |= I_BAMS_MEAS;

          // Reset the BA list structure.
          l1a_reset_ba_list();

          // Store ARFCN list in the BA structure.
          for(i=0;i<nbchans;i++)
            l1a_l1s_com.ba_list.A[i].radio_freq = listptr->A[i];

          // Set number of carrier in the BA list.
          l1a_l1s_com.ba_list.nbr_carrier = nbchans;

          // Set BA identifier.
          l1a_l1s_com.ba_list.ba_id = ba_id;

          // Carrier for next power measurement control.
          l1a_l1s_com.ba_list.next_to_ctrl = next_radio_freq_measured;
          // Carrier for next power measurement result.
          l1a_l1s_com.ba_list.next_to_read = next_radio_freq_measured;
          // Set first BA index measured in current session.
          l1a_l1s_com.ba_list.first_index =  next_radio_freq_measured;

          // Enable BA list measurement task.
          l1a.l1a_en_meas[I_NMEAS] |= I_BAMS_MEAS;

          // step in state machine.
          *state = WAIT_RESULT;
        }

        // End of process.
        end_process = 1;
      }
      break;

      case WAIT_RESULT:
      {
        if(SignalCode == L1C_RXLEV_PERIODIC_DONE)
        // One bunch of measurement has been completed.
        //---------------------------------------------
        {
          // Forward result message to L3.
          l1a_send_result(MPHC_RXLEV_PERIODIC_IND, msg, RRM1_QUEUE);

          // End of process.
          return;
        }

        else
        #if (L1_GPRS)
          if((SignalCode == MPHC_STOP_RXLEV_PERIODIC_REQ) ||
              (SignalCode == L1P_TRANSFER_DONE)           ||
              (SignalCode == L1C_DEDIC_DONE))
        #else
          if((SignalCode == MPHC_STOP_RXLEV_PERIODIC_REQ) ||
              (SignalCode == L1C_DEDIC_DONE))
        #endif
        // Request to STOP this activity.
        //-------------------------------
        {
          // send confirmation message
          l1a_send_confirmation(MPHC_STOP_RXLEV_PERIODIC_CON,RRM1_QUEUE);
          // This process must be reset.
          *state = RESET;
        }

        else
        if (SignalCode == MPHC_RXLEV_PERIODIC_REQ)
        {
          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END         // KEEP IN EXTERNAL MEM otherwise
#endif

/*-------------------------------------------------------*/
/* l1a_full_list_meas_process()                          */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a state machine which handles the    */
/* Cell Selection Full List Power Measurement L1/L3      */
/* interface and it handles the neigbour cell            */
/* measurement process in IDLE mode with FULL list.      */
/* When a message MPHC_RXLEV_REQ is received             */
/* the L1S task FSMS_MEAS is enabled. When this task     */
/* is completed a reporting message L1C_VALID_MEAS_INFO */
/* is received and forwarded to L3.                      */
/*                                                       */
/* Starting messages:        MPHC_RXLEV_REQ.             */
/*                                                       */
/* Result messages (input):  L1C_VALID_MEAS_INFO        */
/*                                                       */
/* Result messages (output): MPHC_RXLEV_IND              */
/*                                                       */
/* Reset messages (input):   none                        */
/*                                                       */
/* Stop message (input):     MPHC_STOP_RXLEV_REQ         */
/*                                                       */
/* Stop message (output):    MPHC_STOP_RXLEV_CON         */
/*                                                       */
/* Rem:                                                  */
/* ----                                                  */
/* L3 is in charge of the number of pass to follow the   */
/* GSM recommendation.                                   */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_full_list_meas_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1,
    WAIT_RESULT = 2
  };

  UWORD8   *state      = &l1a.state[FULL_MEAS];
  UWORD32   SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset FULL_MEAS process.
        l1a_l1s_com.l1s_en_meas &= FSMS_MEAS_MASK; // Clear Cell Selection Measurement enable flag.
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == MPHC_RXLEV_REQ)
        // Request to enter the Cell Selection measurements.
        //--------------------------------------------------
        {
          UWORD16 i;

          #if L1_RECOVERY
            // check whether we need to recover L1
            if (l1a_l1s_com.recovery_flag == TRUE)
            {
              // recovery only allowed during cell selection to avoid side effects
              // Transition rules: MPHC_RXLEV_REQ may also be sent in idle mode or packet idle mode
              // check whether idle mode task NP inactive
              #if L1_GPRS
              if ((l1a_l1s_com.l1s_en_task[NP]  == TASK_DISABLED) &&
                  (l1a_l1s_com.l1s_en_task[PNP] == TASK_DISABLED))
              #else
              if (l1a_l1s_com.l1s_en_task[NP] == TASK_DISABLED)
              #endif
              {
                #if (CHIPSET == 12) || (CHIPSET == 15)
                  F_INTH_DISABLE_ONE_IT(C_INTH_FRAME_IT); // disable IT TDMA
                #else
                  INTH_DISABLEONEIT(IQ_FRAME); // disable IT TDMA
                #endif

                #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
                {
                  l1_trace_recovery();
                }
                #endif

                l1_initialize_for_recovery();

                // Set SYNCHRO task enable flag in order to call the function l1d_reset_hw
                l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;

                #if (CHIPSET == 12) || (CHIPSET == 15)
                  F_INTH_ENABLE_ONE_IT(C_INTH_FRAME_IT); // enable IT TDMA
                #else
                  INTH_ENABLEONEIT(IQ_FRAME); // enable IT TDMA
                #endif
              }
            }
          #endif

          // download info from message
          l1a_l1s_com.full_list_ptr=(T_MPHC_RXLEV_REQ *)(msg->SigP);
          if (l1a_l1s_com.full_list_ptr->power_array_size==0) return; //empty list -> return

          // Set "parameter synchro semaphores"
          l1a_l1s_com.meas_param |= FSMS_MEAS;

          // Reset the full list structure.
          l1a_reset_full_list();

          // Reset the Input Level (IL) memory table.
          if ((l1a_l1s_com.mode == CS_MODE) || (l1a_l1s_com.mode == CS_MODE0))

          {
#if (L1_FF_MULTIBAND == 0)
            for(i=0; i<=l1_config.std.nbmax_carrier; i++)
#else
            for(i=0; i<= NBMAX_CARRIER; i++)
#endif
            {
              l1a_l1s_com.last_input_level[i].input_level = l1_config.params.il_min;
              l1a_l1s_com.last_input_level[i].lna_off     = 0;
            }
          }

          // Enable Cell Selection Full list measurement task.
          l1a.l1a_en_meas[FULL_MEAS] |= FSMS_MEAS;

          // Step in state machine.
          *state = WAIT_RESULT;
        }

        // End of process.
        end_process = 1;
      }
      break;

      case WAIT_RESULT:
      {
        if(SignalCode == L1C_VALID_MEAS_INFO)
        // One valid measurement pass has been completed over the full list of carriers.
        //------------------------------------------------------------------------------
        {
          //--------------------------------------------------------
          // WE COULD PUT HERE THE CODE TO TRANSLATE IL -> RXLEV !!!
          //--------------------------------------------------------

          // Forward result message to L3.
          l1a_send_result(MPHC_RXLEV_IND, msg, RRM1_QUEUE);

          // Reset the machine.
          *state = RESET;
        }

        else if (SignalCode == MPHC_STOP_RXLEV_REQ)
        {
          // Forward result message to L3.
          l1a_send_confirmation(MPHC_STOP_RXLEV_CON,RRM1_QUEUE);
          // Reset the machine.
          *state = RESET;
        }
        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

/*-------------------------------------------------------*/
/* l1a_cres_process()                                    */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a state machine which handles Cell   */
/* Reselection.                                          */
/*                                                       */
/* Starting messages:        MPHC_NEW_SCELL_REQ          */
/* ------------------                                    */
/*  L1 camps on the given ARFCN.                         */
/*                                                       */
/* Result messages (output): MPHC_NEW_SCELL_CON          */
/* -------------------------                             */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_cres_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1
  };

  UWORD8  *state      = &l1a.state[CR_B];
  UWORD32  SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == MPHC_NEW_SCELL_REQ)
        // Request to enter the Cell Reselection BCCH reading.
        //----------------------------------------------------
        // L1 must keep reading the system informations flagged
        // in the bitmap.
        {
          // Reset the Neighbor Cell information structure.
          l1a_reset_cell_info(&(l1a_l1s_com.Scell_info));

          // Reset serving cell E-OTD store on Cell Change
          #if ((L1_EOTD == 1) && (L1_EOTD_QBIT_ACC == 1))
            l1a_l1s_com.nsync.serv_fn_offset = 0;
            l1a_l1s_com.nsync.serv_time_alignmt = 0;
          #endif

          // Download ARFCN, timing information and bitmap from the command message.
          l1a_l1s_com.Scell_info.radio_freq   = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->radio_freq;
          l1a_l1s_com.Scell_info.bsic         = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->bsic;
          l1a_l1s_com.Scell_info.time_alignmt = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->time_alignmt;
          l1a_l1s_com.Scell_info.fn_offset    = ((T_MPHC_NEW_SCELL_REQ *)(msg->SigP))->fn_offset;

#if (L1_FF_MULTIBAND == 1)
          {
            UWORD8 physical_band_id; 
            physical_band_id = 
                l1_multiband_radio_freq_convert_into_physical_band_id(l1a_l1s_com.Scell_info.radio_freq );
            L1_MULTIBAND_TRACE_PARAMS(MULTIBAND_PHYSICAL_BAND_TRACE_ID,multiband_rf[physical_band_id].gsm_band_identifier);
          }
#endif /*#if (L1_FF_MULTIBAND == 1)*/
          

          // Layer 1 internal mode is set to CS MODE.
          l1a_l1s_com.mode = CS_MODE;

          // Set flag for toa init.
          #if (TOA_ALGO != 0)
            l1a_l1s_com.toa_reset = TRUE;
          #endif

          // In order to keep tn_difference and dl_tn consistent, we need to avoid
          // the execution of the SYNCHRO task with tn_difference updated and
          // dl_tn not yet updated (this can occur if we go in the HISR just after
          // the update of tn_difference). To do this the solution is to use the Semaphore
          // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
          // associated Semaphore is reset.
          // Note: Due to the specificity of the SYNCHRO task which can be enabled
          // by L1A state machines as by L1S processes, the semaphore can't followed
          // the generic rules of the Semaphore shared between L1A and L1S.
          // tn_difference -> loaded with the number of timeslot to shift.
          // dl_tn         -> loaded with the new timeslot.

          l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
          {
            l1a_l1s_com.tn_difference += 0 - l1a_l1s_com.dl_tn;
            l1a_l1s_com.dl_tn         = 0; // Camping on timeslot 0.

            #if L1_GPRS
              // Select GSM DSP Scheduler.
              l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;
            #endif

            // Timing must be shifted to a new timeslot, enables SYNCHRO task..
            l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
          }
          l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
          // Note: The using of the semaphore associated to the SYNCHRO task can't be done
          // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
          // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
          // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
          // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.

          // Step in state machine.
          *state = RESET;

          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_NEW_SCELL_CON,RRM1_QUEUE);

          // End of process.
          end_process = 1;
        }
        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.


/*-------------------------------------------------------*/
/* l1a_neighbour_cell_bcch_reading_process()        */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a state machine which handles the    */
/* BCCH reading from up to 6 neighbour cells             */
/*                                                       */
/* Starting messages:         MPHC_NCELL_BCCH_REQ        */
/* ------------------                                    */
/*                                                       */
/* Result messages (input):   L1C_BCCHN_INFO             */
/* ------------------------                              */
/*                                                       */
/* Result messages (output):  MPHC_NCELL_BCCH_IND        */
/* -------------------------                             */
/*                                                       */
/* Reset messages (input):    MPHC_STOP_NCELL_BCCH_REQ   */
/* -----------------------   (MPHC_STOP_NCELL_BCCH_CON)  */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_neighbour_cell_bcch_reading_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1,
    BCCHN_CONFIG       = 2,
    WAIT_BCCHN_RESULT  = 3
  };

  UWORD8  *state      = &l1a.state[I_BCCHN];
  UWORD32  SignalCode = msg->SignalCode;
  UWORD32  time_alignmt;
  UWORD32  fn_offset;
  UWORD8   task;

  // use only in packet transfer mode
  // in this mode only one neighbor is allowed to be decoded
  // so these variables memorize this neighbor parameters.
  static UWORD32  time_alignmt_mem;
  static UWORD32  fn_offset_mem;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset CS_MEAS process.
        l1a_l1s_com.l1s_en_task[BCCHN] = TASK_DISABLED;          // Clear BCCHN  task enable flag.
        l1a_l1s_com.l1s_en_task[BCCHN_TOP] = TASK_DISABLED;      // Clear BCCHN_TOP task enable flag.
        #if (L1_GPRS)
          l1a_l1s_com.l1s_en_task[BCCHN_TRAN] = TASK_DISABLED;   // Clear BCCHN_TRAN  task enable flag.
        #endif
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == MPHC_NCELL_BCCH_REQ)
        // Request to read BCCH from one neighbour cell.
        //----------------------------------------------
        {
          // there are 3 priorities with BCCH neighbor task
          // => TOP PRIORITY: this request has priority over serving cell activity
          //                  and any other neighbor cells activity (used for GPRS).
          //                  In IDLE circuit this priority enable the task BCCHN_TOP
          //                  In Packet Transfer this priority enable the task BCCHN_TRAN
          // => HIGH_PRIORITY:this request has priority over the neighbor cell BCCH reading
          //                  with priority set to NORMAL_PRIORITY.
          // => NORMAL_PRIORITY:this request has no special priority.
          // Note: HIGH_PRIORITY and NORMAL_PRIORITY enable the task BCCHN.

          #if (L1_GPRS)
            if(l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
              task = BCCHN_TRAN;
            else
              if(((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority == TOP_PRIORITY)
                task = BCCHN_TOP;
              else
                task = BCCHN;
          #else
            task = BCCHN;
          #endif

          // Set semaphores for neighbor BCCH task.
          l1a_l1s_com.task_param[task] = SEMAPHORE_SET;

          // Step in state machine.
          *state = BCCHN_CONFIG;
        }

        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;


      case BCCHN_CONFIG:
      {
        // Request to read BCCH from one neighbour cell.
        //----------------------------------------------

        UWORD8   neigh_number = 0;
        UWORD8   neigh_id;

        // there are 3 priorities with BCCH neighbor task
        // => TOP PRIORITY: this request has priority over serving cell activity
        //                  and any other neighbor cells activity (used for GPRS).
        //                  In IDLE circuit this priority enable the task BCCHN_TOP
        //                  In Packet Transfer this priority enable the task BCCHN_TRAN
        // => HIGH_PRIORITY:this request has priority over the neighbor cell BCCH reading
        //                  with priority set to NORMAL_PRIORITY.
        // => NORMAL_PRIORITY:this request has no special priority.
        // Note: HIGH_PRIORITY and NORMAL_PRIORITY enable the task BCCHN.

        #if (L1_GPRS)
          if(l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
            task = BCCHN_TRAN;
          else
            if(((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority == TOP_PRIORITY)
              task = BCCHN_TOP;
            else
              task = BCCHN;
        #else
            task = BCCHN;
        #endif


        #if (L1_GPRS)
          //in case of packet transfer mode
          //only one neighbor is allowed to be decoded => clear the BCCHN list.
          if(l1a_l1s_com.mode == PACKET_TRANSFER_MODE)
          {
            for (neigh_id=0;neigh_id<6;neigh_id++)
              l1a_l1s_com.bcchn.list[neigh_id].status = NSYNC_FREE;
            l1a_l1s_com.bcchn.current_list_size = 0;
          }
        #endif

        // Abort if there is no room for a new neighbour BCCH reading request.
        if(l1a_l1s_com.bcchn.current_list_size >= 6)
        {
          *state = WAIT_BCCHN_RESULT;
          return;
        }

        // Look for first free location within L1 structure.
        while((neigh_number < 6) && (l1a_l1s_com.bcchn.list[neigh_number].status != NSYNC_FREE))
	{
	    if(neigh_number != 5 )
          neigh_number++;
	}


        // Download neighbour info from request message.
        //----------------------------------------------

        // Download ARFCN, timing information and bitmap from the command message.
        time_alignmt_mem   = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->time_alignmt;
        fn_offset_mem      = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->fn_offset;

        // Sub the serving cell timeslot number to the Neigh./Serving timing
        // difference to format it for L1S use.
        time_alignmt =time_alignmt_mem;
        fn_offset    =fn_offset_mem;
        l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
	if (neigh_number < 6 )
	{

        l1a_l1s_com.bcchn.list[neigh_number].radio_freq    = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq;
        l1a_l1s_com.bcchn.list[neigh_number].fn_offset     = fn_offset;
        l1a_l1s_com.bcchn.list[neigh_number].time_alignmt  = time_alignmt;
        l1a_l1s_com.bcchn.list[neigh_number].tsc           = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->tsc;
        l1a_l1s_com.bcchn.list[neigh_number].bcch_blks_req = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->bcch_blks_req;


        #if L1_GPRS
          // in packet transfer only one priority is allowed : TOP_PRIORITY
          if(l1a_l1s_com.mode != PACKET_TRANSFER_MODE)
            l1a_l1s_com.bcchn.list[neigh_number].gprs_priority = ((T_MPHC_NCELL_BCCH_REQ *)(msg->SigP))->gprs_priority;
          else
            l1a_l1s_com.bcchn.list[neigh_number].gprs_priority = TOP_PRIORITY;
        #else
          l1a_l1s_com.bcchn.list[neigh_number].gprs_priority = NORMAL_PRIORITY;
        #endif

	}
        // Enable L1S activity on this new neighbour task BCCH.
     if (neigh_number < 6 )//OMAPS00090550
        l1a_l1s_com.bcchn.list[neigh_number].status  = NSYNC_PENDING;

        l1a_l1s_com.bcchn.current_list_size         += 1;

        l1a_l1s_com.l1s_en_task[task] = TASK_ENABLED;

        // Step in state machine.
        *state = WAIT_BCCHN_RESULT;

        // End of process.
        end_process = 1;

      }
      break; // case BCCHN_CONFIG


      case WAIT_BCCHN_RESULT:
      {
        if(SignalCode == L1C_BCCHN_INFO)
        // Neighbor cell BCCH reading result.
        //-----------------------------------
        {
          UWORD8  neigh_id         = ((T_L1C_BCCHN_INFO *)(msg->SigP))->neigh_id;
          UWORD16 neigh_radio_freq = ((T_L1C_BCCHN_INFO *)(msg->SigP))->radio_freq;

          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_BCCH_REQ message has been received
          // in the same frame than this L1s message)
          // BUG_973: an issue occurs when this 3 messages arrives in this order and
          //          in the same frame MPHC_STOP_NCELL_BCCH_REQ(A) + MPHC_NCELL_BCCH_REQ(B) + this L1s message(A)
          //          In this case the carrier B wasn't handled because the L1s message deletes the carrier B in the list
          if (neigh_radio_freq != l1a_l1s_com.bcchn.list[neigh_id].radio_freq)
          {
             // REM: the message is not sent to L3

             return; // Stay in current state.
          }

          // Disable neigh BCCH reading when any of the TC have been read.
          l1a_l1s_com.bcchn.list[neigh_id].status = NSYNC_FREE;
          l1a_l1s_com.bcchn.current_list_size    -= 1;

          // Forward result message to L3.
          l1a_send_result(MPHC_NCELL_BCCH_IND, msg, RRM1_QUEUE);

          // Is it the end of the complete process ?
          if(l1a_l1s_com.bcchn.current_list_size == 0)
          {
            // This process must be reset.
            *state = RESET;
          }

          else
          {
            // End of process.
            return;
          }
        }

        else
        if(SignalCode == MPHC_NCELL_BCCH_REQ)
        // Request to read BCCH from one neighbour cell.
        //-----------------------------------
        {
          // Step in state machine.
          *state = BCCHN_CONFIG;
        }

        else
        if(SignalCode == MPHC_STOP_NCELL_BCCH_REQ)
        // Request to STOP neighbour cell activity for certain carriers.
        //--------------------------------------------------------------
        {
          UWORD8  i,j;
          UWORD8  array_size;

          // Disable neighbor BCCH task.
          l1a_l1s_com.l1s_en_task[BCCHN] = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[BCCHN_TOP] = TASK_DISABLED;
          #if (L1_GPRS)
            l1a_l1s_com.l1s_en_task[BCCHN_TRAN] = TASK_DISABLED;
          #endif

          array_size = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array_size;

          if(array_size != 6)
          {
            // Stop some of the Neighb. synchro.
            for(i=0;i<array_size;i++)
            {
              UWORD16  radio_freq = ((T_MPHC_STOP_NCELL_BCCH_REQ *)(msg->SigP))->radio_freq_array[i];

              // Search for same value within L1 structure.
              j=0;
              while(!((radio_freq == l1a_l1s_com.bcchn.list[j].radio_freq) &&
                      (l1a_l1s_com.bcchn.list[j].status != NSYNC_FREE)) &&
                    (j < 6))
              {
				if(j < 5 ) //OMAPS00090550
				{
                	j++;
				}
				else
				{
					j++;
					break;
				}
              }

              // If found, reset L1 structure for this carrier.
              if(j<6)
              {
                l1a_l1s_com.bcchn.list[j].status = NSYNC_FREE;
                l1a_l1s_com.bcchn.current_list_size --;
              }
            }
          }
          else
          {
            // Stop all the Neighb. BCCH reading.
            l1a_l1s_com.bcchn.current_list_size = 0;

            for(i=0;i<6;i++)
              l1a_l1s_com.bcchn.list[i].status = NSYNC_FREE;
          }

          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_NCELL_BCCH_CON,RRM1_QUEUE);

          // All neigh synchro have been removed.
          if(l1a_l1s_com.bcchn.current_list_size == 0)
          {
            // This process must be reset.
            *state = RESET;
          }
          else
          {
            // NOTE: in packet transfer mode only one BCCHN is allowed to be decoded
            //       so it is impossible to be here after the STOP message in this mode
            //       The tasks that may be restart are: BCCHN_TOP and/or BCCHN

            // Check if it remains some BCCHN_TOP or BCCHN tasks to restart
            for(i=0;i<6;i++)
            {
              if (l1a_l1s_com.bcchn.list[i].status != NSYNC_FREE)
              {
                if (l1a_l1s_com.bcchn.list[i].gprs_priority == TOP_PRIORITY)
                {
                  // it remains one BCCHN_TOP task to restart
                  l1a_l1s_com.task_param[BCCHN_TOP]  = SEMAPHORE_SET;
                  l1a_l1s_com.l1s_en_task[BCCHN_TOP] = TASK_ENABLED;
                }
                else
                {
                  // it remains one BCCHN task to restart
                  l1a_l1s_com.task_param[BCCHN]  = SEMAPHORE_SET;
                  l1a_l1s_com.l1s_en_task[BCCHN] = TASK_ENABLED;
                }
              }
            }

            // Stay in current state.
            return;
          }
        }

        #if (L1_GPRS)
          else
          // End of packet transfer mode if TBF downlink and uplink have been released
          if((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all))
          {
            l1a_l1s_com.bcchn.list[0].status = NSYNC_FREE;
            l1a_l1s_com.bcchn.current_list_size = 0;

             // This process must be reset.
            *state = RESET;
          }
          else
          if ((SignalCode == L1P_TRANSFER_DONE) || (SignalCode == L1P_TBF_RELEASED) ||    //change of Time Slot
              (SignalCode == L1P_REPEAT_ALLOC_DONE)|| (SignalCode == L1P_ALLOC_EXHAUST_DONE))
          {
             // We consider only the case: packet Transfer => packet Transfer,the serving TS may be changed
             // For other cases such as Idle -> Transfer... decision not yet taken.

             // update the BCCHN parameters

             // Clear BCCHN_TRAN  task enable flag.
             l1a_l1s_com.l1s_en_task[BCCHN_TRAN] = TASK_DISABLED;

             // Set semaphores for neighbor BCCH task.
             l1a_l1s_com.task_param[BCCHN_TRAN] = SEMAPHORE_SET;

             // Sub the serving cell timeslot number to the Neigh./Serving timing
             // difference to format it for L1S use.
             time_alignmt = time_alignmt_mem;
             fn_offset    = fn_offset_mem;

             l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);

             l1a_l1s_com.bcchn.list[0].fn_offset     = fn_offset;
             l1a_l1s_com.bcchn.list[0].time_alignmt  = time_alignmt;

             l1a_l1s_com.l1s_en_task[BCCHN_TRAN] = TASK_ENABLED;

             // Stay in current state.
             return;
          }
        #endif

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

/*-------------------------------------------------------*/
/* l1a_idle_6strongest_monitoring_process()              */
/*-------------------------------------------------------*/
/*                                                       */
/* Description:                                          */
/* ------------                                          */
/* This function is a state machine which handles the    */
/* synchronization with up to 6 neighbor cells           */
/*                                                       */
/* Starting messages:        MPHC_NCELL_SYNC_REQ         */
/* ------------------        MPHC_NCELL_LIST_SYNC_REQ    */
/*  L1 makes an attempt to read the FB/SB or to confirm  */
/*  SB.                                                  */
/*                                                       */
/*                                                       */
/* Result messages (input):  L1C_FB_INFO                 */
/* ------------------------  L1C_SB_INFO                 */
/*                           L1C_SBCONF_INFO             */
/*  Result messages from L1S. FB detection, SB detection,*/
/*  SB confirmation.                                     */
/*                                                       */
/* Result messages (output): MPHC_NCELL_SYNC_IND         */
/* -------------------------                             */
/*  SB indication.                                       */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_NCELL_SYNC_REQ    */
/* -----------------------  (MPHC_STOP_NCELL_SYNC_CON)   */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_idle_6strongest_monitoring_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1,
    NSYNC_CONFIG       = 2,
    WAIT_NSYNC_RESULT  = 3
#if (L1_12NEIGH == 1)
    ,NSYNC_LIST_CONFIG = 4
    ,WAIT_SSYNC_RESULT = 5
#endif
  };

          UWORD8  *state      = &l1a.state[I_6MP];
          UWORD32  SignalCode = msg->SignalCode;
  static  UWORD8   sb_attempt;


#if (L1_12NEIGH == 1)
  static  UWORD8   list_size;
#endif

#if (L1_EOTD ==1)
  static  UWORD8   last_cell;
#endif

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset of process is embbeded in other state to
        // avoid conflicts between processes using the same
        // L1S tasks.
      }
      break;

      case WAIT_INIT:
      {
        #if (L1_12NEIGH ==1)
          if(SignalCode == MPHC_NCELL_LIST_SYNC_REQ)
          {
            // Check request validity for this process:
            //   ->  This machine works only for IDLE MODE.
            //   ->  Search must be correct.

            if(l1a_l1s_com.mode != I_MODE)
              return;
            #if (L1_EOTD ==1)
              // For EOTD, nsync must be FREE...
            if ( (((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->eotd == TRUE)
                 && (l1a_l1s_com.nsync.current_list_size != 0) )
              // End of process.
              return;
            #endif

            l1a_l1s_com.nsync.first_in_list=0;

            // Set semaphores for all neighbor relative task.
            l1a_l1s_com.task_param[NSYNC]  = SEMAPHORE_SET;
            l1a_l1s_com.task_param[FBNEW]  = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SB2]    = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SBCONF] = SEMAPHORE_SET;

            // Step in state machine.
            *state = NSYNC_LIST_CONFIG;

          }
          else
        #endif

        if(SignalCode == MPHC_NCELL_SYNC_REQ)
        // Request to READ the FB/SB or SB from the given carrier.
        //--------------------------------------------------------
        {
          // Check request validity for this process:
          //   ->  This machine works only for IDLE MODE.
          //   ->  Search must be correct.

            if(l1a_l1s_com.mode != I_MODE)
              return;

          l1a_l1s_com.nsync.first_in_list=0;

          // Set semaphores for all neighbor relative task.
          l1a_l1s_com.task_param[NSYNC]  = SEMAPHORE_SET;
          l1a_l1s_com.task_param[FBNEW]  = SEMAPHORE_SET;
          l1a_l1s_com.task_param[SB2]    = SEMAPHORE_SET;
          l1a_l1s_com.task_param[SBCONF] = SEMAPHORE_SET;

          // Step in state machine.
          *state = NSYNC_CONFIG;
        }

        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;


      case NSYNC_CONFIG:
      {
        // Request to read FB/SB or SB from one neighbour cell.
        //-----------------------------------------------------
        UWORD8  neigh_number = l1a_l1s_com.nsync.first_in_list;

        // Abort if there is no room for a new neighbour synchro request.
        #if (L1_12NEIGH ==1)
          if (l1a_l1s_com.nsync.current_list_size >= NBR_NEIGHBOURS)
        #else
          if (l1a_l1s_com.nsync.current_list_size >= 6)
        #endif
        {
          *state = WAIT_NSYNC_RESULT;
          return;
        }

        // Look for first free location within L1 structure.
        #if (L1_12NEIGH ==1)
          while((neigh_number<NBR_NEIGHBOURS) && (l1a_l1s_com.nsync.list[neigh_number].status != NSYNC_FREE))
          {
            neigh_number++;
            if ( neigh_number == NBR_NEIGHBOURS ) neigh_number=0;
          }
        #else
          while((neigh_number<6) && (l1a_l1s_com.nsync.list[neigh_number].status != NSYNC_FREE))
          {
            neigh_number++;
            if ( neigh_number == 6 ) neigh_number=0;
          }
        #endif

        // Download neighbour info from request message.
        //----------------------------------------------

        l1a_l1s_com.nsync.list[neigh_number].radio_freq      = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq;
        l1a_l1s_com.nsync.list[neigh_number].timing_validity = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->timing_validity;

      #if ((REL99 == 1) && (FF_RTD ==1)) // RTD feature
        // In this case that timing information is provided related to Real Time Difference RTD of the cell.
        // We force the timing_validity to no timing information because there is no special mechanism to take into
        // account RTD information in idle mode. So it is preferable to do a complete FB search than
        // using inaccurate timing in a bad way.
        if(l1a_l1s_com.nsync.list[neigh_number].timing_validity == 3)
          l1a_l1s_com.nsync.list[neigh_number].timing_validity = 0 ;
      #endif

        if(l1a_l1s_com.nsync.list[neigh_number].timing_validity != 0)
        {
          UWORD32  time_alignmt;
          UWORD32  fn_offset;

          // Download ARFCN, timing information and bitmap from the command message.
          time_alignmt   = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->time_alignmt;
          fn_offset      = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->fn_offset;

          // Sub the serving cell timeslot number to the Neigh./Serving timing
          // difference to format it for L1S use.
          l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
          l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

          l1a_l1s_com.nsync.list[neigh_number].fn_offset    = fn_offset;
          l1a_l1s_com.nsync.list[neigh_number].time_alignmt = time_alignmt;
        }
        else
        {
          l1a_l1s_com.nsync.list[neigh_number].fn_offset    = 0;
          l1a_l1s_com.nsync.list[neigh_number].time_alignmt = 0;
        }
        // Enable L1S activity on this new neighbour task BCCH.
        l1a_l1s_com.nsync.list[neigh_number].status  = NSYNC_PENDING;
        l1a_l1s_com.nsync.current_list_size         += 1;
        l1a_l1s_com.l1s_en_task[NSYNC]               = TASK_ENABLED;

        // Step in state machine.
        *state = WAIT_NSYNC_RESULT;

        // End of process.
        end_process = 1;
      }
      break; // case NSYNC_CONFIG

      #if (L1_12NEIGH == 1)
        case NSYNC_LIST_CONFIG:
        {
          // Request to read FB/SB or SB from 1 to 12 neighbour cells.
          //----------------------------------------------------------
          UWORD8  neigh_number = l1a_l1s_com.nsync.first_in_list;
          UWORD8  nbr_free = (UWORD8) (NBR_NEIGHBOURS) - l1a_l1s_com.nsync.current_list_size;
          T_MPHC_NCELL_LIST_SYNC_REQ *pt = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP));
          UWORD8 i;

          //Read list size
          list_size= ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size;

          // Abort if there is no room for a new neighbour synchro request.
          if ( (l1a_l1s_com.nsync.current_list_size >= NBR_NEIGHBOURS) ||
               (nbr_free < list_size) )
          {
            *state = WAIT_NSYNC_RESULT;
            return;
          }

          #if (L1_EOTD==1)
          // Abort if list Not empty and request for an EOTD session....
          if ( (l1a_l1s_com.nsync.current_list_size != 0) &&
               ( pt->eotd == TRUE) )
          {
            *state = WAIT_NSYNC_RESULT;
            return;
          }
          // store Eotd flag
            l1a_l1s_com.nsync.eotd_meas_session = pt->eotd;
          #endif

          // Download neighbour info from request message.
          //----------------------------------------------
          for (i=0; i<list_size; i++,neigh_number++)
          {
            if ( neigh_number == NBR_NEIGHBOURS ) neigh_number=0; //cyclic buffer
            // Look for first free location within L1 structure.
            while((neigh_number<NBR_NEIGHBOURS) && (l1a_l1s_com.nsync.list[neigh_number].status != NSYNC_FREE))
            {
               neigh_number++;
               if ( neigh_number == NBR_NEIGHBOURS ) neigh_number=0;
            }

            l1a_l1s_com.nsync.list[neigh_number].radio_freq      = pt->ncell_list[i].radio_freq;
            l1a_l1s_com.nsync.list[neigh_number].timing_validity = pt->ncell_list[i].timing_validity;
          #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
            // In this case that timing information is provided related to Real Time Difference RTD of the cell.
            // We force the timing_validity to no timing information because there is no special mechanism to take into
            // account RTD information in idle mode. So it is preferable to do a complete FB search than
            // using inaccurate timing in a bad way.
            if(l1a_l1s_com.nsync.list[neigh_number].timing_validity == 3)
              l1a_l1s_com.nsync.list[neigh_number].timing_validity = 0 ;
          #endif

            if(l1a_l1s_com.nsync.list[neigh_number].timing_validity != 0)
            {
              UWORD32  time_alignmt;
              UWORD32  fn_offset;

              // Download ARFCN, timing information and bitmap from the command message.
              time_alignmt   = pt->ncell_list[i].time_alignmt;
              fn_offset      = pt->ncell_list[i].fn_offset;

              // Sub the serving cell timeslot number to the Neigh./Serving timing
              // difference to format it for L1S use.
              l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
              l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

              l1a_l1s_com.nsync.list[neigh_number].fn_offset    = fn_offset;
              l1a_l1s_com.nsync.list[neigh_number].time_alignmt = time_alignmt;
            }
            else
            {
              l1a_l1s_com.nsync.list[neigh_number].fn_offset    = 0;
              l1a_l1s_com.nsync.list[neigh_number].time_alignmt = 0;
            }

            // Increment list size
            l1a_l1s_com.nsync.current_list_size += 1;

            // Enable L1S activity on all new neighbour tasks if NOT eotd.
#if (L1_EOTD==1)
            if (pt->eotd == FALSE)
#endif
              l1a_l1s_com.nsync.list[neigh_number].status  = NSYNC_PENDING;

          } // end for

          // If NOT Eotd start new neighbours.
#if (L1_EOTD == 1)
          if (pt->eotd == FALSE)
          {
#endif
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;
            // Step in state machine.
            *state = WAIT_NSYNC_RESULT;
#if (L1_EOTD == 1)
          }

            else
            {
              UWORD32  time_alignmt =0;
              UWORD32  fn_offset = 0;

              // Set eotd mode
              l1a_l1s_com.nsync.eotd_meas_session = TRUE;

              // In Eotd : Serving cell is not part of the list
              //-----------------------------------------------
              // But it must be the 1st monitored
              // Sub the serving cell timeslot number to the Neigh./Serving timing
              // difference to format it for L1S use.
              #if (L1_EOTD_QBIT_ACC==1)
                time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;
                fn_offset = l1a_l1s_com.nsync.serv_fn_offset;
              #endif

              l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
              l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

              // Load Serving cell in last position [NBR_NEIGHBOURS]
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].radio_freq   = l1a_l1s_com.Scell_info.radio_freq;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset    = fn_offset;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt = time_alignmt;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].timing_validity = 2;

              // Set list size
              l1a_l1s_com.nsync.current_list_size = 1;

              // start Eotd
              last_cell = FALSE;

              // Enable Serving cell monitoring
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status  = NSYNC_PENDING;
              l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

              // Step in state machine.
              *state = WAIT_SSYNC_RESULT;
            }
          #endif // L1_EOTD == 1

          // End of process.
          end_process = 1;
        }
        break; // case NSYNC_LIST_CONFIG

      #endif //(L1_12NEIGH == 1)

#if ((L1_EOTD == 1)&&L1_12NEIGH)
      case WAIT_SSYNC_RESULT:
      {
        if(SignalCode == L1C_SBCONF_INFO)
        // Synchro Burst confirmation attempt result.
        //-------------------------------------------
        {
          UWORD8  neigh_id        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->neigh_id;
          BOOL    sb_found        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->sb_flag;
          UWORD16 neigh_radio_freq= ((T_L1C_SBCONF_INFO *)(msg->SigP))->radio_freq;


          // Check if this neighbor is NOT serving cell
          if ( (neigh_radio_freq != l1a_l1s_com.Scell_info.radio_freq) || (neigh_id !=12))
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          // Set mode IDLE for EOTD
          ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 0;

          if(sb_found == TRUE)
          // SB detection is a success.
          {
            UWORD32  *fn_offset_ptr    = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_offset);
            UWORD32  *time_alignmt_ptr = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->time_alignmt);
            UWORD32  fn_sb_neigh = ((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_sb_neigh;
            WORD16   d_eotd_first= ((T_L1C_SBCONF_INFO *)(msg->SigP))->d_eotd_first;
            UWORD32  toa         = ((T_L1C_SBCONF_INFO *)(msg->SigP))->toa;
            WORD32   ta_sb_neigh  = l1a_l1s_com.nsync.list[neigh_id].time_alignmt;
            UWORD32  delta_fn;
            WORD32   delta_qbit;


            // Correct "fn_offset" and "time_alignmt" to report the true
            // Serving/Neighbor time difference.
            //  1) Shift 20 bit since result is from a SB detection.
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
            l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

            // compute the true Serving/Neighbor time difference.
            //  1) update time_alignmt with (23bit - d_eotd_first) delta
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            ta_sb_neigh  += (d_eotd_first - (23))*4 +
                            (l1a_l1s_com.dl_tn * 625);

            if (last_cell == FALSE)
            {
              l1a_l1s_com.nsync.fn_sb_serv = fn_sb_neigh;
              l1a_l1s_com.nsync.ta_sb_serv = ta_sb_neigh;

              ((T_MPHC_NCELL_SYNC_IND*)(msg->SigP))->timetag = 0;
            }
            else
            // End of EOTD meas. Consider Serving Cell as a neighbour cell
            // for timetag computation...
            {
              UWORD32 timetag;
              delta_fn = (fn_sb_neigh - l1a_l1s_com.nsync.fn_sb_serv + MAX_FN)%MAX_FN;
              delta_qbit = ta_sb_neigh - l1a_l1s_com.nsync.ta_sb_serv;

              timetag = (delta_fn*5000) + (WORD32)(delta_qbit);
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->timetag = timetag;

              #if (L1_EOTD_QBIT_ACC ==1)
                // Attempt QB tracking of the serving cell (independent of the TOA algorithm)
                // This is only performed on the second SYNC IND as we do not want to move the serving cell
                // timing during the E-OTD session.
                l1a_compensate_sync_ind((T_MPHC_NCELL_SYNC_IND *)(msg->SigP));
                l1a_l1s_com.nsync.serv_fn_offset    = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset;
                l1a_l1s_com.nsync.serv_time_alignmt = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt;
              #endif

//              #if (CODE_VERSION == SIMULATION)
//                 #if  (TRACE_TYPE==5)
                    ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_fn   = delta_fn;
                    ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_qbit = delta_qbit;
//                 #endif
//              #endif


            }

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
          }
          else
          // SB detection failled.
          {
            #if (L1_EOTD_QBIT_ACC==1)
              l1a_l1s_com.nsync.serv_time_alignmt = 0;
              l1a_l1s_com.nsync.serv_fn_offset = 0;
            #endif

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
//            // Send reporting message with a faillure indication.
//            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
          }

          // Disable the serving sync. reading.
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status = NSYNC_FREE;
          l1a_l1s_com.nsync.current_list_size   = 0;

          if (last_cell == TRUE)
          {
            // reset list size
            list_size = 0;

            // stop eotd session.
            l1a_l1s_com.nsync.eotd_meas_session = FALSE;

            // reset process.
            l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

              // This process must be reset.
            *state = RESET;
          }
          else
          {
            UWORD8  i;

            // Curious case where there are no previously synchronised neighbours,
            // but an EOTD session is requested. Apparently, this is legal.
            // Here, we synchronise to the serving cell a second time with the
            // eotd_meas_session flag set so that no AFC or TOA updates are performed.
            #if (L1_EOTD_QBIT_ACC == 1)
              if(list_size == 0)
              {
                // L1 SW : Create a temporary copy of the serving time_alignmt and fn_offset
                //         as we don't want to do the timeslot maths on the reference version
                //         in case the synchronisation fails and we can't write the new values
                //         back to the store...

                UWORD32 time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;
                UWORD32 fn_offset = l1a_l1s_com.nsync.serv_fn_offset;

                // In Eotd : Serving cell is not part of the list
                //-----------------------------------------------
                // But it must be the 1st and last monitored
                // Sub the serving cell timeslot number to the Neigh./Serving timing
                // difference to format it for L1S use.

                l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
                l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

                // Load Serving cell in last position [NBR_NEIGHBOURS]
                l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].radio_freq   = l1a_l1s_com.Scell_info.radio_freq;
                l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset    = fn_offset;
                l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt = time_alignmt;
                l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].timing_validity = 2;

                // Set list size
                l1a_l1s_com.nsync.current_list_size = 1;

                // start Eotd
                last_cell = TRUE;

                // Enable Serving cell monitoring
                l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status  = NSYNC_PENDING;
                l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

                // No step in state machine in this case.
                *state = WAIT_SSYNC_RESULT;
                return;
              }
              else
              {
            #endif // (L1_EOTD_QBIT_ACC == 1)

            // enable all neighbour monitoring
            l1a_l1s_com.nsync.current_list_size = list_size;

            for (i=0; i<list_size; i++)
              l1a_l1s_com.nsync.list[i].status  = NSYNC_PENDING;
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

            // step in state machine
            *state = WAIT_NSYNC_RESULT;
            return;

          #if (L1_EOTD_QBIT_ACC == 1)
            }
          #endif
          }
        }

        else
        if(SignalCode == MPHC_STOP_NCELL_SYNC_REQ)
        // Request to STOP neighbour cell activity for all carriers.
        //--------------------------------------------------------------
        {
          UWORD8  array_size;

          array_size = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size;

          // TOP accepted only for ALL cells
          if ( (array_size != l1a_l1s_com.nsync.current_list_size) &&
               (array_size != NBR_NEIGHBOURS))
              // Stay in current state.
              return;

          // Stop Eotd session.
          l1a_l1s_com.nsync.eotd_meas_session = FALSE;

          // Disable neighbor sync. tasks.
          l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

          // Stop Serv. sync reading.
          l1a_l1s_com.nsync.current_list_size = 0;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status = NSYNC_FREE;

          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_NCELL_SYNC_CON,RRM1_QUEUE);

          // All neigh synchro have been removed.
          // This process must be reset.
          *state = RESET;
        }
        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;
#endif

      case WAIT_NSYNC_RESULT:
      {
        if(SignalCode == L1C_FB_INFO)
        // Frequency Burst acquisition attempt result.
        //--------------------------------------------
        {
          BOOL    fb_found;
          UWORD8  neigh_id         = ((T_L1C_FB_INFO *)(msg->SigP))->neigh_id;
          UWORD16 neigh_radio_freq = ((T_L1C_FB_INFO *)(msg->SigP))->radio_freq;

          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          // BUG_973: an issue occurs when this 3 messages arrives in this order and
          //          in the same frame MPHC_STOP_NCELL_SYNC_REQ(A) + MPHC_NCELL_SYNC_REQ(B) + this L1s message(A)
          //          In this case the carrier B wasn't handled because the L1s message deletes the carrier B in the list
          if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          // Get result from the message.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          if(fb_found == TRUE)
          // FB attempt is a success.
          {
            // Enable NSYNC task for SB detection (SB2).
          #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
			            l1a_l1s_com.nsync.list[neigh_id].timing_validity = SB_ACQUISITION_PHASE;
			          #else
			            l1a_l1s_com.nsync.list[neigh_id].timing_validity = 3;
			#endif
            // Enable neighbour sync 0.
            l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_PENDING;

            // End of process.
            return;
          }

          else
          // FB attempt failed.
          //-------------------
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
          }

          // Disable a neigh sync. reading.
          l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
          l1a_l1s_com.nsync.current_list_size    -= 1;

          // Is it the end of the complete process ?
          if(l1a_l1s_com.nsync.current_list_size == 0)
          {
            // Reset process.
            l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
            l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

            // This process must be reset.
            *state = RESET;
          }

          // End of process.
          else
          {
              // Check if first in list was removed from the list. Go to next first in list
              while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
              {
                l1a_l1s_com.nsync.first_in_list++;
                #if (L1_12NEIGH==1)
                  if (l1a_l1s_com.nsync.first_in_list == NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
                #else
                  if (l1a_l1s_com.nsync.first_in_list == 6) l1a_l1s_com.nsync.first_in_list = 0;
                #endif
              }
            return;
          }
        }

        else
        if(SignalCode == L1C_SB_INFO)
        // Synchro Burst acquisition attempt result.
        //------------------------------------------
        {
          typedef struct
          {
            BOOL    sb_found_flag;
            UWORD8  bsic;
            UWORD32 fn_offset;
            UWORD32 time_alignmt;
          }
          T_L1A_NSYNC;

          #if (L1_12NEIGH ==1)
             static T_L1A_NSYNC  static_nsync[NBR_NEIGHBOURS];
          #else
             static T_L1A_NSYNC  static_nsync[6];
          #endif
             BOOL         sb_found    = ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag;
             UWORD8       attempt     = ((T_L1C_SB_INFO *)(msg->SigP))->attempt;
             UWORD8       neigh_id    = ((T_L1C_SB_INFO *)(msg->SigP))->neigh_id;
             UWORD16 neigh_radio_freq = ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq;
             T_L1A_NSYNC *static_nsync_ptr = &(static_nsync[neigh_id]);


          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          // BUG_973: an issue occurs when this 3 messages arrives in this order and
          //          in the same frame MPHC_STOP_NCELL_SYNC_REQ(A) + MPHC_NCELL_SYNC_REQ(B) + this L1s message(A)
          //          In this case the carrier B wasn't handled because the L1s message deletes the carrier B in the list
          if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)

          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          // Reset static structure SB detection flag.
          if(attempt == 1) static_nsync[neigh_id].sb_found_flag = FALSE;

          if(sb_found == TRUE)
          // SB detection is a success...
          //-----------------------------
          {
            // Save Results.
            static_nsync_ptr->sb_found_flag = TRUE;
            static_nsync_ptr->bsic          = ((T_L1C_SB_INFO *)(msg->SigP))->bsic;
            static_nsync_ptr->fn_offset     = ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset;
            static_nsync_ptr->time_alignmt  = ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt;
          }

          // Report message to L3 is generated after the 2 attempts.
          if(attempt == 2)
          {
            // Disable a neigh sync. reading.
            l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
            l1a_l1s_com.nsync.current_list_size    -= 1;

            if(static_nsync_ptr->sb_found_flag == FALSE)
            {
              // Send reporting message with a faillure indication.
              l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
            }

            else
            {
              UWORD32  *fn_offset_ptr    = &(((T_L1C_SB_INFO *)(msg->SigP))->fn_offset);
              UWORD32  *time_alignmt_ptr = &(((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt);

              // Download neighbour info in result message.
              ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag      = static_nsync_ptr->sb_found_flag;
              ((T_L1C_SB_INFO *)(msg->SigP))->fn_offset    = static_nsync_ptr->fn_offset;
              ((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt = static_nsync_ptr->time_alignmt;
              ((T_L1C_SB_INFO *)(msg->SigP))->bsic         = static_nsync_ptr->bsic;

              // Correct "fn_offset" and "time_alignmt" to report the true
              // Serving/Neighbor time difference.
              //  1) Shift 20 bit since result is from a SB detection.
              //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
              l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
              l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

              // Forward the result msg to L3.
              l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
            }

            // Is it the end of the complete process ?
            if(l1a_l1s_com.nsync.current_list_size == 0)
            {
              // Reset process.
              l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

              // This process must be reset.
              *state = RESET;
            }

            // End of process.
            else
            {
              // Check if first in list was removed from the list. Go to next first in list
              while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
              {
                l1a_l1s_com.nsync.first_in_list++;
                #if (L1_12NEIGH==1)
                  if (l1a_l1s_com.nsync.first_in_list == NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
                #else
                  if (l1a_l1s_com.nsync.first_in_list == 6) l1a_l1s_com.nsync.first_in_list = 0;
                #endif
              }
              return;
            }
          }

          // End of process.
          else
          {
            return;
          }
        }

        else
        if(SignalCode == L1C_SBCONF_INFO)
        // Synchro Burst confirmation attempt result.
        //-------------------------------------------
        {
          UWORD8  neigh_id        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->neigh_id;
          BOOL    sb_found        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->sb_flag;
          UWORD16 neigh_radio_freq= ((T_L1C_SBCONF_INFO *)(msg->SigP))->radio_freq;


          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          // BUG_973: an issue occurs when this 3 messages arrives in this order and
          //          in the same frame MPHC_STOP_NCELL_SYNC_REQ(A) + MPHC_NCELL_SYNC_REQ(B) + this L1s message(A)
          //          In this case the carrier B wasn't handled because the L1s message deletes the carrier B in the list
          if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          if(sb_found == TRUE)
          // SB detection is a success.
          {

              UWORD32  *fn_offset_ptr    = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_offset);
              UWORD32  *time_alignmt_ptr = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->time_alignmt);

              // Correct "fn_offset" and "time_alignmt" to report the true
              // Serving/Neighbor time difference.
              //  1) Shift 20 bit since result is from a SB detection.
              //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
              l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
              l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);


            #if (L1_EOTD==1)
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {

                UWORD32  fn_sb_neigh = ((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_sb_neigh;
                WORD16   d_eotd_first= ((T_L1C_SBCONF_INFO *)(msg->SigP))->d_eotd_first;
                UWORD32  toa         = ((T_L1C_SBCONF_INFO *)(msg->SigP))->toa;
                WORD32   ta_sb_neigh  = l1a_l1s_com.nsync.list[neigh_id].time_alignmt;
                UWORD32  delta_fn;
                WORD32   delta_qbit;
                UWORD32  timetag;

                // compute the true Serving/Neighbor time difference.
                //  1) update time_alignmt with (23bit - d_eotd_first) delta
                //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
                ta_sb_neigh  += (d_eotd_first - (23))*4 +
                                (l1a_l1s_com.dl_tn * 625);


                delta_fn = (fn_sb_neigh - l1a_l1s_com.nsync.fn_sb_serv + MAX_FN)%MAX_FN;
                delta_qbit = ta_sb_neigh - l1a_l1s_com.nsync.ta_sb_serv;

                // Set timetag
                timetag = (delta_fn*5000) + (WORD32)(delta_qbit);
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->timetag = timetag;

                // Set mode
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 0;

//                #if (CODE_VERSION == SIMULATION)
//                  #if (TRACE_TYPE == 5)
                    ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_fn   = delta_fn;
                    ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->delta_qbit = delta_qbit;
//                  #endif
//                #endif
             }

            #endif

            #if ((L1_EOTD == 1) && (L1_EOTD_QBIT_ACC == 1))
              // Attempt to compensate each N-Cell SYNC IND for QB tracking.
              l1a_compensate_sync_ind((T_MPHC_NCELL_SYNC_IND *)(msg->SigP));
            #endif

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
          }

          else
          // SB detection failled.
          {
           #if (L1_EOTD ==1)
            // Set mode
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
             {
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 0;
               // Forward the result msg to L3.
               l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
             }
             else
           #endif
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
          }

          // Disable a neigh sync. reading.
          l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
          l1a_l1s_com.nsync.current_list_size    -= 1;

          // Is it the end of the complete process ?
          if(l1a_l1s_com.nsync.current_list_size == 0)
          {
           #if ((L1_EOTD ==1)&& L1_12NEIGH)
            // Is it EOTD ?
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
            {
               UWORD32  time_alignmt=0;
               UWORD32  fn_offset=0;

              // Init list to serving cell
              // Download ARFCN, timing information and bitmap from the command message.
              // Sub the serving cell timeslot number to the Neigh./Serving timing
              // difference to format it for L1S use.
              #if (L1_EOTD_QBIT_ACC==1)
                time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;
                fn_offset = l1a_l1s_com.nsync.serv_fn_offset;
              #endif

              l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
              l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

              // Store Serving cell infos in location [NBR_NEIGHBOURS]
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].radio_freq   = l1a_l1s_com.Scell_info.radio_freq;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset    = fn_offset;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt = time_alignmt;
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].timing_validity = 2;

              // Set list size && last cell flag.
              l1a_l1s_com.nsync.current_list_size = 1;
              last_cell = TRUE;

              // Enable Serving cell monitoring
              l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status  = NSYNC_PENDING;
              l1a_l1s_com.l1s_en_task[NSYNC]    = TASK_ENABLED;

              *state = WAIT_SSYNC_RESULT;
              return;
            }
            else
            #endif
            {
              // Reset process.
              l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

              // This process must be reset.
              *state = RESET;
            }
          }

          // End of process.
          else
          {
            // Check if first in list was removed from the list. Go to next first in list
            while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
            {
              l1a_l1s_com.nsync.first_in_list++;
              #if (L1_12NEIGH==1)
                if (l1a_l1s_com.nsync.first_in_list == NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
              #else
                if (l1a_l1s_com.nsync.first_in_list == 6) l1a_l1s_com.nsync.first_in_list = 0;
              #endif
            }
            return;
          }
        }
        #if (L1_12NEIGH ==1)
          else
          if(SignalCode == MPHC_NCELL_LIST_SYNC_REQ)
          // Request to READ the FB/SB or SB of 1 to 12 carriers.
          //--------------------------------------------------------
          {
          #if (L1_EOTD ==1)
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              // Stay in current state.
              return;
            else
          #endif
              // Step in state machine.
              *state = NSYNC_LIST_CONFIG;
          }
        #endif
        else
        if(SignalCode == MPHC_NCELL_SYNC_REQ)
        // Request to READ the FB/SB or SB from the given carrier.
        //--------------------------------------------------------
        {
          #if (L1_EOTD ==1)
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              // Stay in current state.
              return;
            else
          #endif

          // Step in state machine.
          *state = NSYNC_CONFIG;
        }

        else
        if(SignalCode == MPHC_STOP_NCELL_SYNC_REQ)
        // Request to STOP neighbour cell activity for certain carriers.
        //--------------------------------------------------------------
        {
          UWORD8  i,j;
          UWORD8  array_size;

          array_size = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size;

          #if (L1_EOTD ==1)
            // Only stop for ALL neighbours in list are accepted.
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
            {
              if ( (array_size != l1a_l1s_com.nsync.current_list_size) &&
                   (array_size != NBR_NEIGHBOURS))
                // Stay in current state.
                return;
            }
            // Stop Eotd session.
            l1a_l1s_com.nsync.eotd_meas_session = FALSE;
          #endif

          // Disable neighbor sync. tasks.
          l1a_l1s_com.l1s_en_task[NSYNC]  = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[FBNEW]  = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[SB2]    = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[SBCONF] = TASK_DISABLED;

          #if (L1_12NEIGH ==1)
            if(array_size != NBR_NEIGHBOURS)
          #else
            if(array_size != 6)
          #endif
          {
            // Stop some of the Neighb. synchro.
            for(i=0;i<array_size;i++)
            {
              UWORD16  radio_freq = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[i];

              // Search for same value within L1 structure.
              j=0;
              while(!((radio_freq == l1a_l1s_com.nsync.list[j].radio_freq) &&
                      (l1a_l1s_com.nsync.list[j].status != NSYNC_FREE)) &&
              #if (L1_12NEIGH ==1)
                      (j < NBR_NEIGHBOURS))
              #else
                      (j < 6))
              #endif
              {
                j++;
              }

              // If found, reset L1 structure for this carrier.
              #if (L1_12NEIGH ==1)
                if(j<NBR_NEIGHBOURS)
              #else
                if(j<6)
              #endif
              {
                l1a_l1s_com.nsync.list[j].status = NSYNC_FREE;
                l1a_l1s_com.nsync.current_list_size --;
              }
            }
          }
          else
          {
            // Stop all the Neighb. BCCH reading.
            l1a_l1s_com.nsync.current_list_size = 0;

            #if (L1_12NEIGH ==1)
              for(i=0;i<NBR_NEIGHBOURS;i++)
            #else
              for(i=0;i<6;i++)
            #endif
              l1a_l1s_com.nsync.list[i].status = NSYNC_FREE;
          }

          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_NCELL_SYNC_CON,RRM1_QUEUE);

          // All neigh synchro have been removed.
          if(l1a_l1s_com.nsync.current_list_size == 0)
          {
            // Tasks already disabled.

            // This process must be reset.
            *state = RESET;
          }
          else
          {
            // Check if first in list was removed from the list. Go to next first in list
            while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
            {
              l1a_l1s_com.nsync.first_in_list++;
              #if (L1_12NEIGH==1)
                if (l1a_l1s_com.nsync.first_in_list == NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
              #else
                if (l1a_l1s_com.nsync.first_in_list == 6) l1a_l1s_com.nsync.first_in_list = 0;
              #endif
            }

            // Set semaphores for all neighbor relative task before re-enebling NSYNC task.
            l1a_l1s_com.task_param[NSYNC]  = SEMAPHORE_SET;
            l1a_l1s_com.task_param[FBNEW]  = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SB2]    = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SBCONF] = SEMAPHORE_SET;

            // Enable neighbour sync task.
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

            // Stay in current state.
            return;
          }
        }
        // No action in this machine for other messages.
        else
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

/*-------------------------------------------------------*/
/* l1a_idle_serving_cell_bcch_reading_process()          */
/*-------------------------------------------------------*/
/* Description : This state machine handles serving cell */
/* BCCH reading.                                         */
/*                                                       */
/* Starting messages:        MPHC_SCELL_NBCCH_REQ        */
/* ------------------        MPHC_SCELL_EBCCH_REQ        */
/*                                                       */
/*  L1 continuously reads the serving cell BCCH and/or   */
/*  Extended BCCH as requested by the scheduling info.   */
/*                                                       */
/* Result messages (input):  L1C_BCCHS_INFO              */
/* ------------------------                              */
/*  System information data block from L1S.              */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_SCELL_BCCH_REQ    */
/* -----------------------  (MPHC_STOP_SCELL_BCCH_CON)   */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_idle_serving_cell_bcch_reading_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1,
    NBCCHS_CONFIG      = 2,
    EBCCHS_CONFIG      = 3,
    WAIT_BCCHS_RESULT  = 4
  };

  UWORD8  *state      = &l1a.state[I_SCB];
  UWORD32  SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset CS_MEAS process.
        l1a_l1s_com.l1s_en_task[NBCCHS] = TASK_DISABLED;  // Clear NBCCHS task enable flag.
        l1a_l1s_com.l1s_en_task[EBCCHS] = TASK_DISABLED;  // Clear EBCCHS task enable flag.
      }
      break;

      case WAIT_INIT:
      {
        // Request to read Normal BCCH from serving cell.
        if(SignalCode == MPHC_SCELL_NBCCH_REQ)
        {
          // Step in state machine.
          *state = NBCCHS_CONFIG;
        }

        // Request to read Extended BCCH from serving cell.
        else
        if(SignalCode == MPHC_SCELL_EBCCH_REQ)
        {
          // Step in state machine.
          *state = EBCCHS_CONFIG;
        }

        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;

      case NBCCHS_CONFIG:
      {
        UWORD8 i;

        // Set semaphores for Normal Serving BCCH reading task.
        l1a_l1s_com.task_param[NBCCHS] = SEMAPHORE_SET;

        // Download message content.
        //--------------------------
        l1a_l1s_com.nbcchs.schedule_array_size = ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array_size;

        for(i=0;i<l1a_l1s_com.nbcchs.schedule_array_size;i++)
        {
          l1a_l1s_com.nbcchs.schedule_array[i].modulus =
            ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[i].modulus;
          l1a_l1s_com.nbcchs.schedule_array[i].relative_position =
            ((T_MPHC_SCELL_NBCCH_REQ *)(msg->SigP))->schedule_array[i].relative_position;
        }

        // Enable NBCCHS task.
        l1a_l1s_com.l1s_en_task[NBCCHS] = TASK_ENABLED;

        // Step in state machine.
        *state = WAIT_BCCHS_RESULT;

        // End of process.
        end_process = 1;
      }
      break;

      case EBCCHS_CONFIG:
      {
        UWORD8 i;

        // Set semaphores for Normal Serving BCCH reading task.
        l1a_l1s_com.task_param[EBCCHS] = SEMAPHORE_SET;

        // Download message content.
        //--------------------------
        l1a_l1s_com.ebcchs.schedule_array_size = ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array_size;

        for(i=0;i<l1a_l1s_com.ebcchs.schedule_array_size;i++)
        {
          l1a_l1s_com.ebcchs.schedule_array[i].modulus =
            ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].modulus;
          l1a_l1s_com.ebcchs.schedule_array[i].relative_position =
            ((T_MPHC_SCELL_EBCCH_REQ *)(msg->SigP))->schedule_array[i].relative_position;
        }

        // Enable EBCCHS task.
        l1a_l1s_com.l1s_en_task[EBCCHS] = TASK_ENABLED;

        // Step in state machine.
        *state = WAIT_BCCHS_RESULT;

        // End of process.
        end_process = 1;
      }
      break;

      case WAIT_BCCHS_RESULT:
      {
        if(SignalCode == L1C_BCCHS_INFO)
        // Serving cell BCCH reading result.
        //----------------------------------
        {
          // Forward result message to L3.
          l1a_send_result(MPHC_DATA_IND, msg, RRM1_QUEUE);

          // End of process.
          return;
        }

        else
        if(SignalCode == MPHC_SCELL_NBCCH_REQ)
        // Request to re-configure Normal BCCH reading.
        //---------------------------------------------
        {
          // Step in state machine.
          *state = NBCCHS_CONFIG;
        }

        else
        if(SignalCode == MPHC_SCELL_EBCCH_REQ)
        // Request to re-configure Normal BCCH reading.
        //---------------------------------------------
        {
          // Step in state machine.
          *state = EBCCHS_CONFIG;
        }

        else
        if((SignalCode == MPHC_STOP_SCELL_BCCH_REQ) || (SignalCode == L1C_DEDIC_DONE))
        // Request to STOP any serving cell bcch activity.
        //------------------------------------------------
        {
          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_SCELL_BCCH_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        #if L1_GPRS
          else
          if((SignalCode == L1P_SINGLE_BLOCK_CON) ||
             (SignalCode == MPHP_SINGLE_BLOCK_CON))
          // If Two Phase Access is ongoing: Packet Resource Request
          // msg has been sent to the network. BCCH reading must be
          // stopped to let PDCH reading going.
          // REM: we must check both L1P/MPHP messages since an other
          // process could have renamed L1P into MPHP.
          //--------------------------------------------------------
          {
            if(((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->purpose == TWO_PHASE_ACCESS)
            {
              // This process must be reset.
              *state = RESET;
            }
            else
            {
              // End of process.
              return;
            }
          }
          else
          // End of packet transfer mode: test PDTCH to be sure that TBF downlink and uplink are released
          if((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all))
          {
             // This process must be reset.
            *state = RESET;
          }
          else
          if((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->Transfer_update == FALSE))
          // Transition IDLE -> Packet Transfer
          // Request to STOP serving cell BCCH activity.
          //--------------------------------------------
          {
            // Send confirmation message to L3.
            l1a_send_confirmation(MPHC_STOP_SCELL_BCCH_CON,RRM1_QUEUE);

            // This process must be reset.
            *state = RESET;
          }
          else
          if((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->Transfer_update == TRUE))
          // Transition Packet Transfer -> Packet Transfer
          {
             // Remark: the synchro is handled by the task CTRL.
             // stay in the same state

             // End of process.
             return;
          }
        #endif

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.


#if !((MOVE_IN_INTERNAL_RAM == 1) && (GSM_IDLE_RAM > 1))  // MOVE TO INTERNAL MEM IN CASE GSM_IDLE_RAM == 2
//#pragma GSM_IDLE2_DUPLICATE_FOR_INTERNAL_RAM_START         // KEEP IN EXTERNAL MEM otherwise

/*-------------------------------------------------------*/
/* l1a_idle_serving_cell_paging_process()                */
/*-------------------------------------------------------*/
/* Description : This state machine handles paging       */
/*                                                       */
/* Starting messages:        MPHC_START_CCCH_REQ         */
/* ------------------                                    */
/*                                                       */
/*  L1 continuously reads the serving cell BCCH and/or   */
/*  Extended BCCH as requested by the scheduling info.   */
/*                                                       */
/* Result messages (input):  L1C_ALLC_INFO               */
/* ------------------------  L1C_NP_INFO                 */
/*                           L1C_EP_INFO                 */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_CCCH_REQ          */
/* -----------------------  (MPHC_STOP_CCCH_CON)         */
/*                                                       */
/*-------------------------------------------------------*/
//Nina added
INT8 last_page_mode = 2; //REORG;
//End Nina added
void l1a_idle_serving_cell_paging_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET             = 0,
    WAIT_INIT         = 1,
    WAIT_MSG          = 2
  };

  enum pg_modes
  {
    NORMAL            = 0,
    EXTENDED          = 1,
    REORG             = 2
  };

         UWORD8  *state      = &l1a.state[I_SCP];
         UWORD32  SignalCode = msg->SignalCode;
  static UWORD8   page_mode  = REORG;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Disable serving cell tasks.
        l1a_l1s_com.l1s_en_task[NP]   = TASK_DISABLED;  // Reset NP   task enable flag.
        l1a_l1s_com.l1s_en_task[EP]   = TASK_DISABLED;  // Reset EP   task enable flag.
        l1a_l1s_com.l1s_en_task[ALLC] = TASK_DISABLED;  // Reset ALLC (reorg) task enable flag.

        // No Paging  => no gauging => no Deep sleep
        //Nina modify to save power, not forbid deep sleep, only force gauging in next paging
        if(l1s.force_gauging_next_paging_due_to_CCHR == 0) // Force gauging next paging
        l1s.pw_mgr.enough_gaug = FALSE;  // forbid Deep sleep

      }
      break;

      case WAIT_INIT:
      {
        if (SignalCode == MPHC_START_CCCH_REQ)
        {
          // download page mode from message (msg)
          page_mode = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_mode;
//Nina added
if(((last_page_mode == NORMAL) && (page_mode == EXTENDED)) ||
	((last_page_mode == EXTENDED) && (page_mode == NORMAL)))
{
l1s.force_gauging_next_paging_due_to_CCHR = 1;
}
			last_page_mode = page_mode;
//End Nina added

          if(page_mode == REORG)
          // Request to enter the PAGING REORGANIZATION paging mode.
          //--------------------------------------------------------
          // L1 must start the Serving cell paging monitoring in PAGING REORGANIZATION
          // paging mode. L1 starts reading the FULL BCCH and CCCH informations.
          {
            // Set semaphores for all serving cell tasks.
            l1a_l1s_com.task_param[ALLC] = SEMAPHORE_SET;   // Set ALLC task semaphore.
            l1a_l1s_com.task_param[NP]   = SEMAPHORE_SET;   // Set NP   task semaphore.

            // Set parameter synchro semaphore for I_BAMS task.
            // Rem: changing the paging parameters changes the place where I_BAMS
            //      task must be executed.
            l1a_l1s_com.meas_param |= I_BAMS_MEAS;

            // Download the PAGING PARAMETERS from the command message.
            l1a_l1s_com.bcch_combined    = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined;
            l1a_l1s_com.bs_pa_mfrms      = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms;
            l1a_l1s_com.bs_ag_blks_res   = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res;
            l1a_l1s_com.ccch_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group;
            l1a_l1s_com.page_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group;
            l1a_l1s_com.page_block_index = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index;

            /*----------------------------------------------------*/
            /* Download Idle parameters and Info in the Serving   */
            /* structure.                                         */
            /*----------------------------------------------------*/
            /* Rem: Get Idle Information from ROM table. Get nbr  */
            /* of paging blocks in a MF51 from ROM table.         */
            /*   "nb_pch_per_mf51" = N div BS_PA_MFRMS.           */
            /*   "idle_task_info"  = info about PCH, EXT_PCH and  */
            /*                       other task to settle in IM.  */
            /*----------------------------------------------------*/
            if(l1a_l1s_com.bcch_combined == TRUE)
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_COMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_COMB+1)) +
                               (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_COMB[l1a_l1s_com.bs_ag_blks_res];
            }
            else
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_NCOMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_NCOMB+1)) +
                                (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_NCOMB[l1a_l1s_com.bs_ag_blks_res];
            }

            // Layer 1 internal mode is set to IDLE MODE.
            l1a_l1s_com.mode = I_MODE;

            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // We must shift the mobile time setting to the timeslot provided by
            // the "ccch_group" parameter.
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
            {
              l1a_l1s_com.tn_difference += (2 * l1a_l1s_com.ccch_group) - l1a_l1s_com.dl_tn;
              l1a_l1s_com.dl_tn         = 2 * l1a_l1s_com.ccch_group;  // Save new TN id.

              #if L1_GPRS
                // Select GSM DSP Scheduler.
                l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;
              #endif

              // Timing must be shifted to a new timeslot, enables SYNCHRO task..
              l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
            }

            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
            // Note: The using of the semaphore associated to the SYNCHRO task can't be done
            // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
            // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
            // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
            // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.

            // In paging reorganization, Full BCCH reading must be setup.
            l1a_l1s_com.Scell_info.si_bit_map = ALL_SI;

            // Step in state machine.
            *state = WAIT_MSG;

            // Enable Paging Reorganisation tasks.
            l1a_l1s_com.l1s_en_task[NP]   = TASK_ENABLED;      // Set NP task enable flag.
            l1a_l1s_com.l1s_en_task[ALLC] = TASK_ENABLED;      // Set ALLC (for paging reorg) task enable flag.

            // End of process.
            return;
          }

          else
          if(page_mode == NORMAL)
          {
            // Request to enter the NORMAL PAGING paging mode.
            //------------------------------------------------
            // L1 must start the Serving cell paging monitoring in NORMAL PAGING
            // paging mode. L1 starts reading only its own paging subchannel.

            #if (TRACE_TYPE==3)
              if (l1_stats.type == FER_CCCH || l1_stats.type == FER_CCCH_TN246)
                l1_stats.wait_time = 0;
            #endif

            // Disable the paging reorganization tasks.
            l1a_l1s_com.l1s_en_task[NP]   = TASK_DISABLED;  // Reset NP   task enable flag.
            l1a_l1s_com.l1s_en_task[ALLC] = TASK_DISABLED;  // Reset ALLC (reorg) task enable flag.

            // Set parameter synchro semaphore for I_BAMS task.
            // Rem: changing the paging parameters changes the place where I_BAMS
            //      task must be executed.
            l1a_l1s_com.meas_param |= I_BAMS_MEAS;

            // Set semaphores for the normal paging reading task.
            l1a_l1s_com.task_param[NP] = SEMAPHORE_SET;     // Set NP   task semaphore.

            // Download the PAGING PARAMETERS from the command message.
            l1a_l1s_com.bcch_combined    = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined;
            l1a_l1s_com.bs_pa_mfrms      = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms;
            l1a_l1s_com.bs_ag_blks_res   = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res;
            l1a_l1s_com.ccch_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group;
            l1a_l1s_com.page_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group;
            l1a_l1s_com.page_block_index = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index;

            /*----------------------------------------------------*/
            /* Download Idle parameters and Info in the Serving   */
            /* structure.                                         */
            /*----------------------------------------------------*/
            /* Rem: Get Idle Information from ROM table. Get nbr  */
            /* of paging blocks in a MF51 from ROM table.         */
            /*   "nb_pch_per_mf51" = N div BS_PA_MFRMS.           */
            /*   "idle_task_info"  = info about PCH, EXT_PCH and  */
            /*                       other task to settle in IM.  */
            /*----------------------------------------------------*/
            if(l1a_l1s_com.bcch_combined == TRUE)
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_COMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_COMB+1)) +
                               (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_COMB[l1a_l1s_com.bs_ag_blks_res];
            }
            else
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_NCOMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_NCOMB+1)) +
                                (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_NCOMB[l1a_l1s_com.bs_ag_blks_res];
            }


          #if L1_GPRS
            //In case of network mode of operation II or III, CCCH reading is possible
            //in packet idle mode and in packet transfer mode.
            //But the SYNCHRO task is not used anymore as opposite to CS mode for CCCH readings
            if (!((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
                  (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
                  (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
                  (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
                  (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)))

          #endif
            {

              // Layer 1 internal mode is set to IDLE MODE.
              l1a_l1s_com.mode = I_MODE;

              // In order to keep tn_difference and dl_tn consistent, we need to avoid
              // the execution of the SYNCHRO task with tn_difference updated and
              // dl_tn not yet updated (this can occur if we go in the HISR just after
              // the update of tn_difference). To do this the solution is to use the Semaphore
              // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
              // associated Semaphore is reset.
              // Note: Due to the specificity of the SYNCHRO task which can be enabled
              // by L1A state machines as by L1S processes, the semaphore can't followed
              // the generic rules of the Semaphore shared between L1A and L1S.
              // We stay on the same serving cell but change the RX timeslot
              // (CCCH_GROUP or timeslot), then the "timeslot difference" between new
              // and old configuration is given in "tn_difference".
              //   tn_difference -> loaded with the number of timeslot to shift.
              //   dl_tn         -> loaded with the new timeslot.
              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
              {
                l1a_l1s_com.tn_difference += (2 * l1a_l1s_com.ccch_group) - l1a_l1s_com.dl_tn;
                l1a_l1s_com.dl_tn          = 2 * l1a_l1s_com.ccch_group;  // Save new TN id.

                #if L1_GPRS
                  // Select GSM DSP Scheduler.
                   l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;

                 // Timing must be shifted to a new timeslot, enables SYNCHRO task..
                  l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
                #else
                  if(l1a_l1s_com.tn_difference != 0)
                  // Timing must be shifted to a new timeslot, enables SYNCHRO task..
                  {
                    l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;  // Set SYNCHRO task enable flag.
                  }
                #endif
              }
              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
              // Note: The using of the semaphore associated to the SYNCHRO task can't be done
              // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
              // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
              // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
              // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.
            }

            // Step in state machine.
            *state = WAIT_MSG;

            // Enable normal paging mode.
            l1a_l1s_com.l1s_en_task[NP] = TASK_ENABLED;     // Set NP task enable flag.

            // End of process.
            return;
          }
          else
          {
            // No action for other page mode
            return;
          }
        }//if (SignalCode == MPHC_START_CCCH_REQ)
        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;

      case WAIT_MSG:
      {
        if(SignalCode == MPHC_START_CCCH_REQ)
        {
          // download paging mode from msg
          page_mode = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_mode;
//Nina added

			if(((last_page_mode == NORMAL) && (page_mode == EXTENDED)) ||
				((last_page_mode == EXTENDED) && (page_mode == NORMAL)))
			{
			l1s.force_gauging_next_paging_due_to_CCHR = 1;
			}
			last_page_mode = page_mode;
//End Nina added
          if ((page_mode == NORMAL) || (page_mode == REORG))
          {
            // Step in state machine.
            *state = RESET;
          }
          else if (page_mode == EXTENDED)
          {
            // Request to enter the EXTENDED PAGING paging mode.
            //------------------------------------------------

            #if (TRACE_TYPE==3)
              if (l1_stats.type == FER_CCCH || l1_stats.type == FER_CCCH_TN246)
                l1_stats.wait_time = 0;
            #endif

            // Disable the paging reorganization tasks.
            l1a_l1s_com.l1s_en_task[NP]   = TASK_DISABLED;  // Reset NP   task enable flag.
            l1a_l1s_com.l1s_en_task[ALLC] = TASK_DISABLED;  // Reset ALLC (reorg) task enable flag.

            // Set parameter synchro semaphore for I_BAMS task.
            // Rem: changing the paging parameters changes the place where I_BAMS
            //      task must be executed.
            l1a_l1s_com.meas_param |= I_BAMS_MEAS;

            // Set semaphores for the normal/extended paging reading task.
            l1a_l1s_com.task_param[NP] = SEMAPHORE_SET;     // Set NP   task semaphore.
            l1a_l1s_com.task_param[EP] = SEMAPHORE_SET;     // Set EP   task semaphore.

            // Download the PAGING PARAMETERS from the command message.
            l1a_l1s_com.bcch_combined    = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bcch_combined;
            l1a_l1s_com.bs_pa_mfrms      = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_pa_mfrms;
            l1a_l1s_com.bs_ag_blks_res   = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->bs_ag_blks_res;
            l1a_l1s_com.ccch_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->ccch_group;
            l1a_l1s_com.page_group       = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_group;
            l1a_l1s_com.page_block_index = ((T_MPHC_START_CCCH_REQ *)(msg->SigP))->page_block_index;

            /*----------------------------------------------------*/
            /* Download Idle parameters and Info in the Serving   */
            /* structure.                                         */
            /*----------------------------------------------------*/
            /* Rem: Get Idle Information from ROM table. Get nbr  */
            /* of paging blocks in a MF51 from ROM table.         */
            /*   "nb_pch_per_mf51" = N div BS_PA_MFRMS.           */
            /*   "idle_task_info"  = info about PCH, EXT_PCH and  */
            /*                       other task to settle in IM.  */
            /*----------------------------------------------------*/
            if(l1a_l1s_com.bcch_combined == TRUE)
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_COMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_COMB+1)) +
                               (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_COMB[l1a_l1s_com.bs_ag_blks_res];
            }
            else
            {
              l1a_l1s_com.idle_task_info  =
                IDLE_INFO_NCOMB[(l1a_l1s_com.bs_ag_blks_res * (MAX_PG_BLOC_INDEX_NCOMB+1)) +
                                (l1a_l1s_com.page_block_index)];
              l1a_l1s_com.nb_pch_per_mf51 =
                NBPCH_IN_MF51_NCOMB[l1a_l1s_com.bs_ag_blks_res];
            }


            #if L1_GPRS
              //In case of network mode of operation II or III, CCCH reading is possible
              //in packet idle mode and in packet transfer mode.
              //But the SYNCHRO task is not used anymore as opposite to CS mode for CCCH readings
              if (!((l1a_l1s_com.l1s_en_task[PNP]    == TASK_ENABLED) ||
                    (l1a_l1s_com.l1s_en_task[PEP]    == TASK_ENABLED) ||
                    (l1a_l1s_com.l1s_en_task[PALLC]  == TASK_ENABLED) ||
                    (l1a_l1s_com.l1s_en_task[PDTCH]  == TASK_ENABLED) ||
                    (l1a_l1s_com.l1s_en_task[SINGLE] == TASK_ENABLED)))
            #endif
            {
            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // We stay on the same serving cell but change the RX timeslot
            // (CCCH_GROUP or timeslot), then the "timeslot difference" between new
            // and old configuration is given in "tn_difference".
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.

            // Layer 1 internal mode is set to IDLE MODE.

              l1a_l1s_com.mode = I_MODE;

              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
              {
                l1a_l1s_com.tn_difference += (2 * l1a_l1s_com.ccch_group) - l1a_l1s_com.dl_tn;
                l1a_l1s_com.dl_tn          =  2 * l1a_l1s_com.ccch_group;  // Save new TN id.

                #if L1_GPRS
                  // Select GSM DSP Scheduler.
                  l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;

                  // Timing must be shifted to a new timeslot, enables SYNCHRO task..
                  l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
                #else
                  if(l1a_l1s_com.tn_difference != 0)
                  // Timing must be shifted to a new timeslot, enables SYNCHRO task..
                  {
                    l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
                  }
                #endif
              }
              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
              // Note: The using of the semaphore associated to the SYNCHRO task can't be done
              // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
              // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
              // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
              // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.
            }

            // Enable normal/extended paging mode.
            l1a_l1s_com.l1s_en_task[NP] = TASK_ENABLED;  // Set NP task enable flag.
            l1a_l1s_com.l1s_en_task[EP] = TASK_ENABLED;  // Set EP task enable flag also.

            // Paging parameters change => perform the gauging on the next paging
        //Nina modify to save power, not forbid deep sleep, only force gauging in next paging
if(l1s.force_gauging_next_paging_due_to_CCHR == 0)
l1s.pw_mgr.enough_gaug = FALSE;  // forbid Deep sleep until next gauging
//End Nina modify


            // end of process
            return;
          }//end if (page_mode == EXTENDED)
        }// end if(SignalCode == MPHC_START_CCCH_REQ)

        else
        if((SignalCode == L1C_ALLC_INFO) ||
           (SignalCode == L1C_NP_INFO)   ||
           (SignalCode == L1C_EP_INFO))
        // Paging reorganization tasks results.
        //-------------------------------------
        {
          // Forward result message to L3.
          l1a_send_result(MPHC_DATA_IND, msg, RRM1_QUEUE);

          // End of process.
          return;
        }

        else
        if((SignalCode == MPHC_STOP_CCCH_REQ) || (SignalCode == L1C_DEDIC_DONE))
        // Request to STOP any serving cell paging activity.
        //--------------------------------------------------
        {
          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_CCCH_CON,RRM1_QUEUE);
          // This process must be reset.
          *state = RESET;
        }

        #if L1_GPRS
          else
          if((SignalCode == L1P_SINGLE_BLOCK_CON) ||
             (SignalCode == MPHP_SINGLE_BLOCK_CON))
          // If Two Phase Access is ongoing: Packet Resource Request
          // msg has been sent to the network. CCCH reading must be
          // stopped to let PDCH reading going.
          // REM: we must check both L1P/MPHP messages since an other
          // process could have renamed L1P into MPHP.
          //--------------------------------------------------------
          {
            if(((T_MPHP_SINGLE_BLOCK_CON *)(msg->SigP))->purpose == TWO_PHASE_ACCESS)
            {
              // This process must be reset.
              *state = RESET;
            }
            else
            {
              // End of process.
              return;
            }
          }
          else
          // End of packet transfer mode
          if((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all))
          {
             // This process must be reset.
            *state = RESET;
          }
          else
          if((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->Transfer_update == FALSE))
          // Transition IDLE -> Packet Transfer
          // Request to STOP serving cell CCCH activity.
          //--------------------------------------------
          {
            // Send confirmation message to L3.
            l1a_send_confirmation(MPHC_STOP_CCCH_CON,RRM1_QUEUE);
            // This process must be reset.
            *state = RESET;
          }
          else
          if((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->Transfer_update == TRUE))
          // Transition Packet Transfer -> Packet Transfer
          {
              // Remark: the synchro is handled by the task CTRL.
              // stay in the same state

            // End of process.
             return;
          }
        #endif

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }// end case WAIT_MSG
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

//#pragma GSM_IDLE_DUPLICATE_FOR_INTERNAL_RAM_END
#endif


/*-------------------------------------------------------*/
/* l1a_initial_network_sync_process()                    */
/*-------------------------------------------------------*/
/* Description : This state machine handles the 1st      */
/* synchronization with the network.                     */
/*                                                       */
/* Starting messages:        MPHC_NETWORK_SYNC_REQ       */
/*                                                       */
/* Result messages (input):  L1C_FB_INFO                 */
/*                           L1C_SB_INFO                 */
/*                                                       */
/* Result messages (output): MPHC_NETWORK_SYNC_IND       */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_NETWORK_SYNC_REQ  */
/*                          (MPHC_STOP_NETWORK_SYNC_CON) */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_initial_network_sync_process(xSignalHeaderRec *msg)
{

  enum states
  {
    RESET               =  0,  // Reset state.
    WAIT_INIT           =  1,  // Initial state.

    SET_FS_FB1_MODE0    =  2,  // First Synchro, Setting of 1st FB mode 0.
    WAIT_FS_FB1_MODE0   =  3,  // First Synchro, 1st FB mode 0 state.

    SET_FS_FB2_MODE0    =  10,  // First Synchro, Setting of 2nd FB mode 0.
    WAIT_FS_FB2_MODE0   =  11,  // First Synchro, 2nd FB mode 0 state.
    SET_FS_FB_MODE1     =  12,  // First Synchro, Setting of FB mode 1.
    WAIT_FS_FB_MODE1    =  13,  // First Synchro, FB mode 1 state.
    WAIT_FS_SB          =  14,  // First Synchro, SB state.

    SET_FB_MODE1        =  15,  // Setting of FB mode 1.
    WAIT_FB_MODE1       =  16,  // FB mode 1 (freq. in tracking) state.
    WAIT_SB             =  17,  // SB state.
    WAIT_BCCHS          =  18   // BCCHS state.
  };


  #if (VCXO_ALGO == 1)
    #define FS_FB1_MODE0_CENTER   1
    #define FS_FB1_MODE0_MAX      2
    #define FS_FB1_MODE0_MIN      3

    static  UWORD32  state_vcxo;
  #endif

  UWORD8  *state      = &l1a.state[CS_NORM];
  UWORD32  SignalCode = msg->SignalCode;

  static WORD16   static_attempt_counter_0;
  static WORD16   static_attempt_counter_1;


  static UWORD8   static_first_synch_flag = TRUE;
  static UWORD8   static_sb_found_flag;
  static UWORD8   static_bsic;
  static UWORD32  static_fn_offset;
  static UWORD32  static_time_alignmt;
  static UWORD8   static_timing_validity;

  // to keep track of the old AFC value
  static WORD16 old_afc;

  while(1)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset tasks used in the process.
        l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

        // Disable neighbour sync 0.
        l1a_l1s_com.nsync.list[0].status = NSYNC_FREE;
      }
      break;

      case WAIT_INIT:
      {
        if(SignalCode == MPHC_NETWORK_SYNC_REQ)
        // Request to READ the FULL BCCH on the given carrier.
        //----------------------------------------------------
        // L1 must first synchronize with the given carrier and then start
        // reading the FULL BCCH.
        {
          UWORD8  search_mode = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->search_mode;
           #if L1_FF_WA_OMAPS00099442
            // reset TPU offset to current value + half a TDMA to avoid having the Fb burst at the begining of the FBNEW TPU window
            // This can be done safely in L1A at this point
            //l1s.tpu_offset = (l1s.tpu_offset + TPU_CLOCK_RANGE >> 1 ) % TPU_CLOCK_RANGE;
            //l1dmacro_synchro(IMM, l1s.tpu_offset);
          #endif

          // Set task semaphores.
          l1a_l1s_com.task_param[NSYNC] = SEMAPHORE_SET;
          l1a_l1s_com.task_param[FBNEW] = SEMAPHORE_SET;
          l1a_l1s_com.task_param[SB2]   = SEMAPHORE_SET;

          l1a_l1s_com.nsync.current_list_size = 0;

          // Downlink stuff timeslot is 0 (default in CS)
          l1a_l1s_com.dl_tn = 0;

          // Download neighbour info from request message.
          //----------------------------------------------

          l1a_l1s_com.nsync.list[0].radio_freq      = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->radio_freq;
          
#if (L1_FF_MULTIBAND== 1)
          {
            UWORD8 physical_band_id;
            physical_band_id = l1_multiband_radio_freq_convert_into_physical_band_id(l1a_l1s_com.nsync.list[0].radio_freq);
            L1_MULTIBAND_TRACE_PARAMS(MULTIBAND_PHYSICAL_BAND_TRACE_ID,multiband_rf[physical_band_id].gsm_band_identifier);
          }
#endif /*#if (L1_FF_MULTIBAND == 1)*/
          
          l1a_l1s_com.nsync.list[0].timing_validity = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->timing_validity;
        #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
          // In this case that timing information is provided related to Real Time Difference RTD of the cell.
          // We force the timing_validity to no timing information because there is no special mechanism to take into
          // account RTD information in idle mode. So it is preferable to do a complete FB search than
          // using inaccurate timing in a bad way.

          if(l1a_l1s_com.nsync.list[0].timing_validity == 3)  // force complete search
            l1a_l1s_com.nsync.list[0].timing_validity = 0 ;

        #endif

          static_timing_validity = l1a_l1s_com.nsync.list[0].timing_validity;

          if(l1a_l1s_com.nsync.list[0].timing_validity != 0)
          {
            UWORD32  time_alignmt;
            UWORD32  fn_offset;

            // Download ARFCN, timing information and bitmap from the command message.
            time_alignmt   = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->time_alignmt;
            fn_offset      = ((T_MPHC_NETWORK_SYNC_REQ *)(msg->SigP))->fn_offset;

            // Sub the serving cell timeslot number to the Neigh./Serving timing
            // difference to format it for L1S use.
            l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
            l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

            l1a_l1s_com.nsync.list[0].fn_offset    = fn_offset;
            l1a_l1s_com.nsync.list[0].time_alignmt = time_alignmt;
          }
          else
          {
            l1a_l1s_com.nsync.list[0].fn_offset    = 0;
            l1a_l1s_com.nsync.list[0].time_alignmt = 0;
          }

          // Reset attempt counters
          static_attempt_counter_0 = 0;
          #if (TRACE_TYPE==3)
            if (l1_stats.type == FER_FCH_MODE1)
              search_mode = 1;
            else
              search_mode = 0;
          #endif

          // Set functional mode.
          l1a_l1s_com.mode = CS_MODE;
          #if L1_FF_WA_OMAPS00099442
            l1a_l1s_com.change_tpu_offset_flag = TRUE;
          #endif

          #if L1_GPRS
            // Select GSM DSP Scheduler.
            l1a_l1s_com.dsp_scheduler_mode = GSM_SCHEDULER;

            // Timing must be shifted to a new timeslot, enables SYNCHRO task..
            l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;   // Set SYNCHRO task enable flag.
          #else
            // Enable SYNCHRO task to cleanup the MFTAB.
            l1a_l1s_com.l1s_en_task[SYNCHRO] = TASK_ENABLED;
          #endif

          if(search_mode == 0)
          // Run "FIRST SYNCHRO" algorithme.
          //--------------------------------
          {
            // Step in state machine.
            #if (VCXO_ALGO == 1)
              if ((l1_config.params.afc_algo == ALGO_AFC_LQG_PREDICTOR) ||
                  (l1_config.params.afc_algo == ALGO_AFC_KALMAN_PREDICTOR))
                  state_vcxo = FS_FB1_MODE0_CENTER;
            #endif
                *state = SET_FS_FB1_MODE0;

          }

          else
          // Run "Frequency in Tracking" algorithme.
          //----------------------------------------
          {
            // Step in state machine.
            *state = SET_FB_MODE1;
          }
        }

        // No action in this machine for other messages.
        //----------------------------------------------
        else
        {
          // End of process.
          return;
        }
      }
      break;

      case SET_FS_FB1_MODE0:
        {

        if(static_attempt_counter_0 >= 4)
        // Max number of FB1/Mode0 attempt is reached... Stop process.
        {
          #if (VCXO_ALGO == 1)
            if ((l1_config.params.afc_algo == ALGO_AFC_LQG_PREDICTOR) ||
                (l1_config.params.afc_algo == ALGO_AFC_KALMAN_PREDICTOR))
            {
              if (state_vcxo == FS_FB1_MODE0_CENTER)
              {
                // update vcxo state, reset attempts FB1_MODE0
                state_vcxo = FS_FB1_MODE0_MAX;
                static_attempt_counter_0 = 0;
                break;
              }
              else if (state_vcxo == FS_FB1_MODE0_MAX)
              {
                // update vcxo state, reset attempts FB1_MODE0
                state_vcxo = FS_FB1_MODE0_MIN;
                static_attempt_counter_0 = 0;
                break;
              }
            }
          #endif
          // Send reporting message with a faillure indication.
          l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);

          // Reset state machine.
          *state = RESET;
        }

        else
        // Make a new attempt FB1/mode0.
        {
          // Step in state machine.
          *state = WAIT_FS_FB1_MODE0;
          // Enable neighbour sync 0.
          l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

          // Wideband search for FB detection.
          l1a_l1s_com.fb_mode = FB_MODE_0;

          // Initialize AFC control function.
          #if AFC_ALGO
            #if TESTMODE
              if (l1_config.afc_enable)
            #endif
              {
                #if (VCXO_ALGO == 1)
                  if ((l1_config.params.afc_algo == ALGO_AFC_LQG_PREDICTOR) ||
                      (l1_config.params.afc_algo == ALGO_AFC_KALMAN_PREDICTOR))
                  {
                    if (state_vcxo == FS_FB1_MODE0_CENTER)
                      l1s.afc = l1ctl_afc(AFC_INIT_CENTER, &l1s.afc_frame_count, l1_config.params.eeprom_afc, 0, l1a_l1s_com.nsync.list[0].radio_freq,l1a_l1s_com.mode);
                    else
                    if (state_vcxo == FS_FB1_MODE0_MAX)
                      l1s.afc = l1ctl_afc(AFC_INIT_MAX, &l1s.afc_frame_count, l1_config.params.eeprom_afc, 0, l1a_l1s_com.nsync.list[0].radio_freq,l1a_l1s_com.mode);
                    else
                    if (state_vcxo == FS_FB1_MODE0_MIN)
                      l1s.afc = l1ctl_afc(AFC_INIT_MIN, &l1s.afc_frame_count, l1_config.params.eeprom_afc, 0, l1a_l1s_com.nsync.list[0].radio_freq,l1a_l1s_com.mode);
                  }
                  else
                   l1s.afc = l1ctl_afc(AFC_INIT, &l1s.afc_frame_count, l1_config.params.eeprom_afc, 0, l1a_l1s_com.nsync.list[0].radio_freq,l1a_l1s_com.mode);
                #else
                  l1s.afc = l1ctl_afc(AFC_INIT, &l1s.afc_frame_count, l1_config.params.eeprom_afc, 0, l1a_l1s_com.nsync.list[0].radio_freq);
                #endif
              }
          #endif

          // Restart synch process as initialized by L3->L1 msg
          l1a_l1s_com.nsync.list[0].timing_validity = static_timing_validity;

          // Enable NSYNC task for FB detection mode 0.
          l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

          // End of process.
          return;
        }
      }
      break;

      case WAIT_FS_FB1_MODE0:
      {
        if(SignalCode == L1C_FB_INFO)
        // Frequency Burst acquisition result.
        //------------------------------------
        {
          UWORD8  fb_found;

          // Increment "static_attempt_counter_0".
          static_attempt_counter_0++;

          // Get result from message parameters.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          // Loop on FB reception when making statistics.
          #if (TRACE_TYPE==3)
            if (l1_stats.type == FER_FCH_MODE0 || l1_stats.type == FER_FCH_MODE1)
            {
               // Enable FB_new.
               l1a_l1s_com.l1s_en_task[FBNEW] = TASK_ENABLED;  // Set FB_new task enable flag.

               // End of process.
               return;
            }
          #endif

          if(fb_found == TRUE)
          // FB attempt is a success.
          //-------------------------
          {
            // Stop the search in this current interval
            #if (VCXO_ALGO == 1)
              static_attempt_counter_0 = 4;
            #endif
            // Reset "static_attempt_counter_1".
            static_attempt_counter_1 = 0;

            // We consider the result of this successfull FB search attempt
            // as a good a-priori information for next attempt.
            // "fn_offset" is reversed to satisfy its definition,
            //    fn_offset = Fn_neigh - Fn_serving.

            l1a_l1s_com.nsync.list[0].timing_validity = 1;
            l1a_l1s_com.nsync.list[0].fn_offset       = 51 - l1a_l1s_com.nsync.list[0].fn_offset;

            // Step in state machine.
            *state = SET_FS_FB2_MODE0;
          }

          else
          // FB attempt failled.
          //--------------------
          {
            // Step in state machine.
            *state = SET_FS_FB1_MODE0;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case SET_FS_FB2_MODE0:
      {
        if (static_attempt_counter_1 >= 4)
        // Max number of attempt is reached... go back to 1st FB mode 0.
        // otherwise stop the search and report message with failure indication
        #if (VCXO_ALGO == 1)
        {
          if (state_vcxo == FS_FB1_MODE0_MIN)
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);
            // Reset state machine.
            *state = RESET;
          }
          else
          {
            // Step in state machine.
            *state = SET_FS_FB1_MODE0;
          }
        }
        #else  // VCXO_ALGO
        {
          // Send reporting message with a faillure indication.
          l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);
          // Reset state machine.
          *state = RESET;
        }
        #endif // VCXO_ALGO
        else
        // Make a new attempt FB2/mode0.
        {
          // Step in state machine.
          *state = WAIT_FS_FB2_MODE0;

          // Enable neighbour sync 0.
          l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

          // End of process.
          return;
        }

      }
      break;

      case WAIT_FS_FB2_MODE0:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_FB_INFO)
        {
          UWORD8  fb_found;

          // Increment "static_attempt_counter_1".
          static_attempt_counter_1++;

          // Get result from message parameters.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          if(fb_found == TRUE)
          // FB attempt is a success.
          //-------------------------
          {
            // Reset "static_attempt_counter_1".
            static_attempt_counter_1 = 0;

            // We consider the result of this successfull FB search attempt
            // as a good a-priori information for next attempt.
            // "fn_offset" is reversed to satisfy its definition,
            //    fn_offset = Fn_neigh - Fn_serving.

            l1a_l1s_com.nsync.list[0].timing_validity = 1;
            l1a_l1s_com.nsync.list[0].fn_offset       = 51 - l1a_l1s_com.nsync.list[0].fn_offset;

            // Step in state machine.
            *state = SET_FS_FB_MODE1;
          }

          else
          // FB attempt failed.
          //-------------------
          {
            // Step in state machine.
            *state = SET_FS_FB2_MODE0;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;


      case SET_FS_FB_MODE1:
      {
        if(static_attempt_counter_1 >= 4)
        // Max number of attempt is reached... go back to 1st FB mode 0.
        // otherwise stop the search and report message with failure indication
        #if (VCXO_ALGO == 1)
        {
          if (state_vcxo == FS_FB1_MODE0_MIN)
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);
            // Reset state machine.
            *state = RESET;
          }
          else
          {
            // Step in state machine.
            *state = SET_FS_FB1_MODE0;
          }
        }
        #else  // VCXO_ALGO
        {
          // Send reporting message with a faillure indication.
          l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);
          // Reset state machine.
          *state = RESET;
        }
        #endif // VCXO_ALGO

        else
        // Make a new attempt FB/Mode1.
        {
          // Step in state machine.
          *state = WAIT_FS_FB_MODE1;

          // Set FB detection mode.
          l1a_l1s_com.fb_mode = FB_MODE_1;

          // Enable neighbour sync 0.
          l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

          // End of process.
          return;
        }
      }
      break;


      case WAIT_FS_FB_MODE1:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_FB_INFO)
        {
          UWORD8  fb_found;

          // Increment "static_attempt_counter_1".
          static_attempt_counter_1++;

          // get result from message parameters.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          if(fb_found == TRUE)
          // FB attempt is a success.
          //-------------------------
          {
            // Reset "static_attempt_counter_1".
            static_attempt_counter_1 = 0;

            // Reset "static_sb_found_flag".
            static_sb_found_flag = FALSE;

            // Step in state machine.
            *state = WAIT_FS_SB;

            // WE SHOULD UPDATE THE APRIORI TIMING.

            // Enable NSYNC task for SB detection (SB2).
          #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
            l1a_l1s_com.nsync.list[0].timing_validity = SB_ACQUISITION_PHASE;
          #else
            l1a_l1s_com.nsync.list[0].timing_validity = 3;
          #endif
            // Enable neighbour sync 0.
            l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

            // End of process.
            return;
          }

          else
          // FB attempt failed.
          //-------------------
          {
            // Step in state machine.
            *state = SET_FS_FB_MODE1;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case WAIT_FS_SB:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_SB_INFO)
        {
          UWORD8  sb_found = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->sb_flag;
          UWORD8  bsic     = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->bsic;

          // Increment "static_attempt_counter_1".
          static_attempt_counter_1++;

          if(sb_found == TRUE)
          // SB detection is a success...
          //-----------------------------
          {
            // Save Results.
            static_sb_found_flag = TRUE;
            static_bsic          = bsic;
            static_fn_offset     = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset;
            static_time_alignmt  = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt;
          }

          if(static_attempt_counter_1 >= 8)
          {
            #if (TRACE_TYPE==3)
              // Loop on FB mode 1 detection when making SB statistics.
              if (l1_stats.type == FER_SCH)
              {
                // Reset SB2 task enable flag.
                l1a_l1s_com.l1s_en_task[SB2] = TASK_DISABLED;
                // Reset "static_attempt_counter_1".
                static_attempt_counter_1 = 0;
                // go back to FB mode 1 detection...
                *state = SET_FS_FB_MODE1;
                break;
              }
            #endif


            // Disable NSYNC task.
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

            if(static_sb_found_flag == TRUE)
            // SB detection is a success.
            //----------------------------
            {
              // Save results.
              l1a_l1s_com.nsync.list[0].fn_offset    = static_fn_offset;
              l1a_l1s_com.nsync.list[0].time_alignmt = static_time_alignmt;

              // Correct "ntdma" and "time_alignment" to shift 20 bit to the
              // futur for Normal Burst reading tasks.
              l1a_add_time_for_nb(&l1a_l1s_com.nsync.list[0].time_alignmt,
                                  &l1a_l1s_com.nsync.list[0].fn_offset);

              // In order to keep tn_difference and dl_tn consistent, we need to avoid
              // the execution of the SYNCHRO task with tn_difference updated and
              // dl_tn not yet updated (this can occur if we go in the HISR just after
              // the update of tn_difference). To do this the solution is to use the Semaphore
              // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
              // associated Semaphore is reset.
              // Note: Due to the specificity of the SYNCHRO task which can be enabled
              // by L1A state machines as by L1S processes, the semaphore can't followed
              // the generic rules of the Semaphore shared between L1A and L1S.
              // tn_difference -> loaded with the number of timeslot to shift.
              // dl_tn         -> loaded with the new timeslot.

              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
              {
              l1a_l1s_com.tn_difference = 0; // No timeslot shift to be performed.
              l1a_l1s_com.dl_tn         = 0; // Camping on timeslot 0.
              }
              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
              // Note: The using of the semaphore associated to the SYNCHRO task can't be done
              // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
              // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
              // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
              // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.

              // Clear "time_alignmt, fn_offset" in the result message since we are
              // going to camp on that cell.
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset    = l1a_l1s_com.nsync.list[0].fn_offset;
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt = l1a_l1s_com.nsync.list[0].time_alignmt;
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->sb_flag      = static_sb_found_flag;
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->bsic         = static_bsic;

              // Forward result message to L3.
              l1a_send_result(MPHC_NETWORK_SYNC_IND, msg, RRM1_QUEUE);
              // keep in memory the last good AFC value
              old_afc = l1s.afc;

              // Set flag for toa init.
              #if (TOA_ALGO != 0)
                l1a_l1s_com.toa_reset = TRUE;
              #endif

              // End of this process.
              *state = RESET;
            }

            else
            // SB detection failed.
            //---------------------
            {
                // Send reporting message with a faillure indication.
                l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);

                // Reset state machine.
                *state = RESET;
            }
          }

          else
          // Make a new attempt SB.
          {
            // Enable neighbour sync 0.
            l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

            // End of process.
            return;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      // In case of narrowband search

      case SET_FB_MODE1:
      {
        if (static_attempt_counter_0 >= 4)
        // Max number of FB/Mode1 attempt is reached... Stop process.
        {
          // Send reporting message with a faillure indication.
          l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);

          // Reset state machine.
          *state = RESET;
        }

        else
        // Make a new attempt FB/Mode1.
        {
          // Step in state machine.
          *state = WAIT_FB_MODE1;

          // Set FB detection mode.
          l1a_l1s_com.fb_mode = FB_MODE_1;

          // Enable neighbour sync 0.
          l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

          // Restart synch process as initialized by L3->L1 msg
          l1a_l1s_com.nsync.list[0].timing_validity = static_timing_validity;

          // Enable NSYNC for FB detection mode 1.
          l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

          // End of process.
          return;
        }
      }
      break;

      case WAIT_FB_MODE1:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_FB_INFO)
        {
          UWORD8  fb_found;

          // Increment "static_attempt_counter_0".
          static_attempt_counter_0++;

          // get result from message parameters.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          // Loop on FB reception when making statistics.
          #if (TRACE_TYPE==3)
            if (l1_stats.type == FER_FCH_MODE0 || l1_stats.type == FER_FCH_MODE1 )
            {
               // Enable FB_new.
               l1a_l1s_com.l1s_en_task[FBNEW] = TASK_ENABLED;  // Set FB_new task enable flag.

               // End of process.
               return;
            }
          #endif

          if(fb_found == TRUE)
          // FB attempt is a success.
          //-------------------------
          {
            // Reset "static_attempt_counter_1".
            static_attempt_counter_1 = 0;

            // Reset "static_sb_found_flag".
            static_sb_found_flag = FALSE;

            // Enable NSYNC task for SB detection (SB2).
          #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
            l1a_l1s_com.nsync.list[0].timing_validity = SB_ACQUISITION_PHASE;
          #else
            l1a_l1s_com.nsync.list[0].timing_validity = 3;
          #endif
            // Enable neighbour sync 0.
            l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

            // Step in state machine.
            *state = WAIT_SB;

            // End of process.
            return;
          }

          else
          // FB attempt failed.
          //-------------------
          {
            // Step in state machine.
            *state = SET_FB_MODE1;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case WAIT_SB:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_SB_INFO)
        {
          UWORD8  sb_found = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->sb_flag;
          UWORD8  bsic     = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->bsic;

          // Increment "static_attempt_counter_1".
          static_attempt_counter_1++;

          if(sb_found == TRUE)
          // SB detection is a success, check plmn...
          //-----------------------------------------
          {
            // Save Results.
            static_sb_found_flag = TRUE;
            static_bsic          = bsic;
            static_fn_offset     = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset;
            static_time_alignmt  = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt;
          }

          if (static_attempt_counter_1 >= 8)
          {
            // Reset NSYNC task enable flag.
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

            if(static_sb_found_flag == TRUE)
            // SB detection is a success.
            //----------------------------
            {
              // Save results.
              l1a_l1s_com.nsync.list[0].fn_offset    = static_fn_offset;
              l1a_l1s_com.nsync.list[0].time_alignmt = static_time_alignmt;

              // Correct "ntdma" and "time_alignment" to shift 20 bit to the
              // futur for Normal Burst reading tasks.
              l1a_add_time_for_nb(&l1a_l1s_com.nsync.list[0].time_alignmt,
                                  &l1a_l1s_com.nsync.list[0].fn_offset);


              // In order to keep tn_difference and dl_tn consistent, we need to avoid
              // the execution of the SYNCHRO task with tn_difference updated and
              // dl_tn not yet updated (this can occur if we go in the HISR just after
              // the update of tn_difference). To do this the solution is to use the Semaphore
              // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
              // associated Semaphore is reset.
              // Note: Due to the specificity of the SYNCHRO task which can be enabled
              // by L1A state machines as by L1S processes, the semaphore can't followed
              // the generic rules of the Semaphore shared between L1A and L1S.
              // tn_difference -> loaded with the number of timeslot to shift.
              // dl_tn         -> loaded with the new timeslot.

              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
              {
              l1a_l1s_com.tn_difference = 0; // No timeslot shift to be performed.
              l1a_l1s_com.dl_tn         = 0; // Camping on timeslot 0.
              }
              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
              // Note: The using of the semaphore associated to the SYNCHRO task can't be done
              // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
              // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
              // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
              // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.


              // Clear "time_alignmt, fn_offset" in the result message since we are
              // going to camp on that cell.
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset    = l1a_l1s_com.nsync.list[0].fn_offset;
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt = l1a_l1s_com.nsync.list[0].time_alignmt;

              // Recreate the last sucessful attempt.
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->sb_flag = static_sb_found_flag;
              ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->bsic    = static_bsic;

              // Forward result message to L3.
              l1a_send_result(MPHC_NETWORK_SYNC_IND, msg, RRM1_QUEUE);
              // Keep in memory the AFC value driving to a good synchro
              old_afc = l1s.afc;

              // Initialize toa.
              #if (TOA_ALGO != 0)
                l1a_l1s_com.toa_reset = TRUE;
              #endif

              // step in state machine.
              *state = RESET;
            }

            else
            // SB detection failed.
            //---------------------
            {
                // Send reporting message with a faillure indication.
                // Set the flag spurious_fb_detected to TRUE to reuse in the
                // AFC algo the old variables
                l1s.spurious_fb_detected = TRUE;
                // reuse the old AFC value
                l1s.afc = old_afc;
                l1a_report_failling_ncell_sync(MPHC_NETWORK_SYNC_IND, 0);

                // Reset state machine.
                *state = RESET;
            }
          }

          else
          // Make a new attempt SB.
          {
            // Enable neighbour sync 0.
            l1a_l1s_com.nsync.list[0].status = NSYNC_PENDING;

            // End of process.
            return;
          }
        }

        else
        if(SignalCode == MPHC_STOP_NETWORK_SYNC_REQ)
        // Request to STOP reading the FULL BCCH.
        //---------------------------------------
        {
          l1a_send_confirmation(MPHC_STOP_NETWORK_SYNC_CON,RRM1_QUEUE);

          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

    } // end of "switch".
  } // end of "while"
} // end of procedure.


/*-------------------------------------------------------*/
/* l1a_idle_smscb_process()                              */
/*-------------------------------------------------------*/
/* Description : This state machine handles the SMSCB    */
/* (Short Message Service Cell Broadcast).               */
/*                                                       */
/* Starting messages:        MPHC_CONFIG_CBCH_REQ        */
/*                                                       */
/* Subsequent messages:      MPHC_CBCH_SCHEDULE_REQ      */
/*                           MPHC_CBCH_INFO_REQ          */
/*                           MPHC_CBCH_UPDATE_REQ        */
/*                                                       */
/* Result messages (input):  L1C_CB_INFO                 */
/*                                                       */
/* Result messages (output): MPHC_DATA_IND               */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_CBCH_REQ          */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_idle_smscb_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET           = 0,
    WAIT_CONFIG     = 1,
    WAIT_FOR_CHANGE = 2,
    SET_SCHEDULE    = 3,
    WAIT_RESULT     = 4
  };

  UWORD8                *state      = &l1a.state[I_SMSCB];
  UWORD32                SignalCode = msg->SignalCode;

  UWORD32                first_block_0 =0;
  UWORD32                first_block_1= 0;
  BOOL                   extended_cbch =FALSE; //oamps00090550
  T_CBCH_HEAD_SCHEDULE  *cbch_schedule_ptr= NULL ;
  UWORD8                 schedule_length;

  BOOL end_process = 0;

  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_CONFIG;

        // Reset SMSCB process.
        l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;  // Clear RAACC task enable flag.
      }
      break;

      case WAIT_CONFIG:
      {
        if(SignalCode == MPHC_CONFIG_CBCH_REQ)
        // CBCH configuration message.
        //----------------------------------------------------
        // L1 must download the CBCH description from the command
        // message.
        {
          WORD8 tn_smscb;

          #define SMSCB_NCOMB_START_TIME   6  // SMSCB (case not combined, SDCCH/8).
          #define SMSCB_COMB_START_TIME   30  // SMSCB (case combined, SDCCH/4).

          // Set "parameter synchro semaphores"
          l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;   // Set "parameter synchro semaphore for SMSCB task.

          // Download new CBCH parameters.
          l1a_l1s_com.cbch_desc      = ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_desc;
          l1a_l1s_com.cbch_freq_list = ((T_MPHC_CONFIG_CBCH_REQ *)(msg->SigP))->cbch_freq_list;

          if(l1a_l1s_com.cbch_desc.channel_type == SDCCH_8)
          // case SDCCH/8...
          // Rem: Serving tasks on timeslot 0 is a special case,
          //      start frame is 1 later than other cases.
          {
            if(l1a_l1s_com.cbch_desc.timeslot_no < l1a_l1s_com.dl_tn)
            {
              l1a_l1s_com.cbch_start_in_mf51 = SMSCB_NCOMB_START_TIME;
              l1a_l1s_com.pre_scheduled_cbch = TRUE;
            }
            else
            {
              l1a_l1s_com.cbch_start_in_mf51 = SMSCB_NCOMB_START_TIME + 1;
              l1a_l1s_com.pre_scheduled_cbch = FALSE;
            }
          }
          else
          // case SDCCH/4...
          {
            if(l1a_l1s_com.cbch_desc.timeslot_no < l1a_l1s_com.dl_tn)
            {
              l1a_l1s_com.cbch_start_in_mf51 = SMSCB_COMB_START_TIME;
              l1a_l1s_com.pre_scheduled_cbch = TRUE;
            }
            else
            {
              l1a_l1s_com.cbch_start_in_mf51 = SMSCB_COMB_START_TIME + 1;
              l1a_l1s_com.pre_scheduled_cbch = FALSE;
            }
          }

          // Set "change_synchro" flag to trigger L1S to change the synchro on fly
          // within SMSCB task and to restore current synchro when SMSCB task is completed.
          if(((l1a_l1s_com.cbch_desc.timeslot_no - l1a_l1s_com.dl_tn + 8) % 8) >=4)
          {
            // L1S will make a intra SMSCB task synchro to current TS + 4.
            l1a_l1s_com.change_synchro_cbch  = TRUE;
            tn_smscb                         = l1a_l1s_com.cbch_desc.timeslot_no - l1a_l1s_com.dl_tn - 4;
          }
          else
          {
            // L1S will NOT make the intra SMSCB task synchro.
            l1a_l1s_com.change_synchro_cbch = FALSE;
            tn_smscb                        = l1a_l1s_com.cbch_desc.timeslot_no - l1a_l1s_com.dl_tn;
          }

          if(tn_smscb < 0)
            l1a_l1s_com.tn_smscb = tn_smscb + 8;
          else
            l1a_l1s_com.tn_smscb = tn_smscb;

          // Disable TB1/2/3/5/6/7 reading.
          l1a_l1s_com.cbch_info_req.cbch_num = 0;
          l1a_l1s_com.cbch_info_req.next     = 0;

          // Reset the CBCH/TB0/TB4 activity.
          l1a_l1s_com.norm_cbch_schedule.cbch_state = CBCH_INACTIVE;
          l1a_l1s_com.ext_cbch_schedule.cbch_state  = CBCH_INACTIVE;

          // Step in state machine.
          *state = WAIT_FOR_CHANGE;
        }

        // end of process.
        end_process = 1;
      }
      break;

      case WAIT_FOR_CHANGE:
      {
        if(SignalCode == MPHC_CBCH_SCHEDULE_REQ)
        // CBCH scheduling message.
        //----------------------------------------------------
        {
          // Stop SMSCB task.
          l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;  // Clear SMSCB task enable flag.

          // Set "parameter synchro semaphore for SMSCB task.
          l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;

          extended_cbch   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->extended_cbch;
          schedule_length = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->schedule_length;

          // Choose schedule table according to the considered CBCH (normal or extended).
          if(extended_cbch) cbch_schedule_ptr = &l1a_l1s_com.ext_cbch_schedule;
          else              cbch_schedule_ptr = &l1a_l1s_com.norm_cbch_schedule;

          // Correction of BUG2176: CBCH_HEAD_STRUCTURE has to be updated with schedule_length
          // value, potentially used in SET_SCHEDULE state after receipt of an CBCH_UPDATE_REQ
          cbch_schedule_ptr->schedule_length = schedule_length;


          if(schedule_length == 0)
          {
            // Continuous CBCH header reading.
            //--------------------------------

            // No scheduling info provided, L1 must start reading continously TB0 on
            // normal CBCH or TB4 on extended CBCH.
            cbch_schedule_ptr->cbch_state = CBCH_CONTINUOUS_READING;

            // Continuous reading must start immediately.
            cbch_schedule_ptr->start_continuous_fn = -1;

            // Activate SMSCB task.
            l1a_l1s_com.l1s_en_task[SMSCB] = TASK_ENABLED;   // Set SMSCB task enable flag.

            // Step in state machine.
            *state = WAIT_RESULT;

            // End of process.
            return;
          }

          else
          {
            UWORD32 starting_fn;

            // Scheduled CBCH header reading.
            //--------------------------------

            cbch_schedule_ptr->cbch_state      = CBCH_SCHEDULED;

            first_block_0   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_0;
            first_block_1   = ((T_MPHC_CBCH_SCHEDULE_REQ *)(msg->SigP))->first_block_1;

            // Compute "schedule period" starting frame number.
            // Note: the reception of MPHC_CBCH_SCHEDULE_REQ schedule message always
            //       refers to a scheduling period starting at the next "8*MF51" boundary.
            //       There is no real time issue there since we have plenty TDMA frame
            //       between the reception of the scheduling message from CBCH and the
            //       starting of the considered scheduling period.

            starting_fn = l1s.actual_time.fn - (l1s.actual_time.fn%(8*51)) + 8*51;
            if(starting_fn >= MAX_FN) starting_fn -= MAX_FN;

            cbch_schedule_ptr->starting_fn = starting_fn;

            // Step in state machine.
            *state = SET_SCHEDULE;
          }
        }

        else
        if(SignalCode == MPHC_CBCH_INFO_REQ)
        // CBCH info request message.
        //----------------------------------------------------
        {
          UWORD32  starting_fn;
          UWORD8   i,j;
          UWORD8   tb_bitmap;

          // This request message is a consequence of a CBCH/TB0 or CBCH/TB4 reading.

          // Stop SMSCB task.
          l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;

          // Set "parameter synchro semaphore for SMSCB task.
          l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;

          // Store TB bitmap from message.
          tb_bitmap = ((T_MPHC_CBCH_INFO_REQ *)(msg->SigP))->tb_bitmap;

          // Compute next TB1 multiframe 51 starting frame number.
          starting_fn = l1s.actual_time.fn - (l1s.actual_time.fn%(8*51)) + 51;

          if(starting_fn >= MAX_FN) starting_fn -= MAX_FN;

          // Compute schedule table according to the provided bitmap.
          j=0;
          for(i=0; i<3; i++) // TB 1\2\3
          {
            if(tb_bitmap & (1L<<i))
            {
              l1a_l1s_com.cbch_info_req.start_fn[j] = (starting_fn +
                                                       (UWORD32)i*51L +
                                                       l1a_l1s_com.cbch_start_in_mf51) % MAX_FN;
              j++;
            }
          } // End "for"

          for(i=3; i<6; i++) // TB 4\5\6
          {
            if(tb_bitmap & (1L<<i))
            {
              l1a_l1s_com.cbch_info_req.start_fn[j] = (starting_fn +
                                                       (UWORD32)i*51L +
                                                       l1a_l1s_com.cbch_start_in_mf51 +
                                                       51L) % MAX_FN;
              j++;
            }
          } // End "for"

          // Store number of CBCH info to read.
          l1a_l1s_com.cbch_info_req.cbch_num = j;
          l1a_l1s_com.cbch_info_req.next     = 0;

          // Activate SMSCB task.
          l1a_l1s_com.l1s_en_task[SMSCB] = TASK_ENABLED;   // Set SMSCB task enable flag.

          // Step in state machine.
          *state = WAIT_RESULT;

          // End of process.
          return;
        }

        else
        if(SignalCode == MPHC_CBCH_UPDATE_REQ)
        // CBCH scheduling message.
        //----------------------------------------------------
        {
          // Stop SMSCB task.
          l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;  // Clear SMSCB task enable flag.

          // Set "parameter synchro semaphore for SMSCB task.
          l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;

          extended_cbch   = ((T_MPHC_CBCH_UPDATE_REQ *)(msg->SigP))->extended_cbch;

          // Choose schedule table according to the considered CBCH (normal or extended).
          if(extended_cbch) cbch_schedule_ptr = &l1a_l1s_com.ext_cbch_schedule;
          else              cbch_schedule_ptr = &l1a_l1s_com.norm_cbch_schedule;

          // Scheduled CBCH header reading.
          //--------------------------------

          cbch_schedule_ptr->cbch_state = CBCH_SCHEDULED;

          first_block_0   = ((T_MPHC_CBCH_UPDATE_REQ *)(msg->SigP))->first_block_0;
          first_block_1   = ((T_MPHC_CBCH_UPDATE_REQ *)(msg->SigP))->first_block_1;

          // DO NOT CHANGE "cbch_schedule_ptr->starting_fn" content.

          // Step in state machine.
          *state = SET_SCHEDULE;
        }

        else
        if(SignalCode == MPHC_STOP_CBCH_REQ)
        // Request to STOP reading the CBCH.
        //----------------------------------
        {
          BOOL normal_cbch   = ((T_MPHC_STOP_CBCH_REQ *)(msg->SigP))->normal_cbch;
          BOOL extended_cbch = ((T_MPHC_STOP_CBCH_REQ *)(msg->SigP))->extended_cbch;

          // Stop SMSCB task.
          l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;

          // Set "parameter synchro semaphore for SMSCB task.
          l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;

          if(normal_cbch)   l1a_l1s_com.norm_cbch_schedule.cbch_state = CBCH_INACTIVE;
          if(extended_cbch) l1a_l1s_com.ext_cbch_schedule.cbch_state  = CBCH_INACTIVE;

          if((l1a_l1s_com.norm_cbch_schedule.cbch_state == CBCH_INACTIVE) &&
             (l1a_l1s_com.ext_cbch_schedule.cbch_state  == CBCH_INACTIVE))
          // This process must be reset.
          {
             // send confirmation
            l1a_send_confirmation(MPHC_STOP_CBCH_CON,RRM1_QUEUE);

           // Step in state machine.
            *state = RESET;
          }
          else
          {
            // Activate SMSCB task.
            l1a_l1s_com.l1s_en_task[SMSCB] = TASK_ENABLED;

            // Step in state machine.
            *state = WAIT_RESULT;

            // End of process.
            return;
          }
        }
          else
          if(SignalCode == L1C_DEDIC_DONE)
          {
            // Stop SMSCB task.
            l1a_l1s_com.l1s_en_task[SMSCB] = TASK_DISABLED;

           // Set "parameter synchro semaphore for SMSCB task.
           l1a_l1s_com.task_param[SMSCB] = SEMAPHORE_SET;

 	    l1a_l1s_com.norm_cbch_schedule.cbch_state = CBCH_INACTIVE;
	    l1a_l1s_com.ext_cbch_schedule.cbch_state  = CBCH_INACTIVE;

	    // Step in state machine.
           *state = RESET;        

        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case SET_SCHEDULE:
      {
        if(cbch_schedule_ptr != NULL)
        {
        UWORD8  i,j;
        UWORD8  mf51_offset;

        // Choose schedule table according to the considered CBCH (normal or extended).
        if(extended_cbch) mf51_offset = 4*51;  // Offset to read TB4.
        else              mf51_offset = 0;     // No offset to read TB0.

        // Compute schedule table according to the provided bitmap.
        j=0;
        for(i=0; i<48; i++)
        {
          if(i<32)
          {
            if(first_block_0 & (1L<<i))
            {
              cbch_schedule_ptr->first_block[j] = (cbch_schedule_ptr->starting_fn +
                                                   (UWORD32)i*8L*51L +
                                                   l1a_l1s_com.cbch_start_in_mf51 +
                                                   mf51_offset) % MAX_FN;
              j++;
            }
          }
          else
          {
            if(first_block_1 & (1L<<(i-32)))
            {
              cbch_schedule_ptr->first_block[j] = (cbch_schedule_ptr->starting_fn +
                                                   (UWORD32)i*8L*51L +
                                                   l1a_l1s_com.cbch_start_in_mf51 +
                                                   mf51_offset) % MAX_FN;
              j++;
            }
          }
        } // End "for"

        // Compute FN for starting continuous CBCH/TB0 reading.
        // Rem: this FN is given by the starting FN of the schedule period starting
        // immediately after the schedule period considered in the message.
        cbch_schedule_ptr->start_continuous_fn = ( cbch_schedule_ptr->starting_fn +
                                                   (UWORD32)cbch_schedule_ptr->schedule_length*8L*51L +
                                                   l1a_l1s_com.cbch_start_in_mf51 +
                                                   mf51_offset ) % MAX_FN;

        // Store number of scheduled CBCH according to the considered CBCH (normal or extended).
        // Store "schedule period" starting frame number.
        cbch_schedule_ptr->cbch_num          = j;
        cbch_schedule_ptr->next              = NULL;//o omaps 00090550

        // Activate SMSCB task.
        l1a_l1s_com.l1s_en_task[SMSCB] = TASK_ENABLED;   // Set SMSCB task enable flag.

        // Step in state machine.
        *state = WAIT_RESULT;
       }
        // End of process.
        return;
      }


      case WAIT_RESULT:
      {
        if(SignalCode == L1C_CB_INFO)
        // CBCH result message.
        //---------------------
        {
          // Forward result message to L3.
          l1a_send_result(MPHC_DATA_IND, msg, RRM1_QUEUE);

          // End of process.
          return;
        }

        else
        if((SignalCode == MPHC_CBCH_SCHEDULE_REQ) ||
           (SignalCode == MPHC_CBCH_UPDATE_REQ)   ||
           (SignalCode == MPHC_CBCH_INFO_REQ))
        // Request to reconsider the ongoing CBCH activity.
        //-----------------------------------------------------------
        {
          // Step in state machine.
          *state = WAIT_FOR_CHANGE;
        }

        else
        if((SignalCode == MPHC_STOP_CBCH_REQ)||(SignalCode == L1C_DEDIC_DONE))
        // Request to (may be partially) STOP reading the CBCH.
        //-----------------------------------------------------
        {
          // This process must be reset.
          *state = WAIT_FOR_CHANGE;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.


/*-------------------------------------------------------*/
/* l1a_access_process()                                  */
/*-------------------------------------------------------*/
/* Description : This state machine handles the access   */
/* to the network while in IDLE mode.                    */
/*                                                       */
/* Starting messages:        MPHC_RA_REQ                 */
/*                                                       */
/* Subsequent messages:      MPHC_RA_REQ                 */
/*                                                       */
/* Result messages (input):  L1C_RA_DONE                 */
/*                                                       */
/* Result messages (output): MPHC_RA_CON                 */
/*                                                       */
/* Reset message (input): MPHC_STOP_RA_REQ               */
/*                                                       */
/* Reset message (input): MPHC_STOP_RA_CON               */
/*-------------------------------------------------------*/
void l1a_access_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1,
    WAIT_RESULT = 2
  };

  UWORD8   *state      = &l1a.state[ACCESS];
  UWORD32   SignalCode = msg->SignalCode;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset RAACC process.
        l1a_l1s_com.l1s_en_task[RAACC] = TASK_DISABLED;  // Clear RAACC task enable flag.
      }
      break;


      case WAIT_INIT:
      {
        if(SignalCode == MPHC_RA_REQ)
        // 1st Random access request message.
        //-----------------------------------
        {
          UWORD8  supplied_txpwr;

          // Download Transmit power configuration.
          supplied_txpwr         = ((T_MPHC_RA_REQ *)(msg->SigP))->txpwr;

#if (L1_FF_MULTIBAND == 0)

          //Config.
          if ((l1_config.std.id == DUAL) ||
              (l1_config.std.id == DUALEXT) ||
              (l1_config.std.id == DUAL_US))
          {
            l1a_l1s_com.powerclass_band1 = ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band1;
            l1a_l1s_com.powerclass_band2 = ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band2;
          }
          else
          {
            l1a_l1s_com.powerclass_band1 = ((T_MPHC_RA_REQ *)(msg->SigP))->powerclass_band1;
          }
#endif

          // Check max transmit power (min txpwr) according to powerclass
          // and clip to MIN_TXPWR_LEVEL.
          supplied_txpwr = l1a_clip_txpwr(supplied_txpwr,l1a_l1s_com.Scell_info.radio_freq);

          // Given value must be used on 1st TX.
          l1s.applied_txpwr = supplied_txpwr;

          // Init RAACC process.
          l1a_l1s_com.ra_info.rand            = ((T_MPHC_RA_REQ *)(msg->SigP))->rand;
          l1a_l1s_com.ra_info.channel_request = ((T_MPHC_RA_REQ *)(msg->SigP))->channel_request;


          // Increment "rand" value in order to avoid to abort RACH by SYNCHRO task
          // when MPHC_START_CCCH_REQ and MPHC_RA_REQ are sent in same TDMA.
          l1a_l1s_com.ra_info.rand+=4;

          // step in state machine.
          *state = WAIT_RESULT;

          // Change mode to connection establishment part 1.
          l1a_l1s_com.mode = CON_EST_MODE1;

          // Activate RAACC task (no semaphore for UL tasks).
          // Enable Paging Reorg and Normal paging tasks.
          l1a_l1s_com.l1s_en_task[RAACC] = TASK_ENABLED; // Set RAACC task enable flag.
        }

        // end of process.
        end_process = 1;
      }
      break;

      case WAIT_RESULT:
      {
        if(SignalCode == L1C_RA_DONE)
        // Random access acqnowledge message.
        //-----------------------------------
        {
          // Forward result message to L3.
          l1a_send_result(MPHC_RA_CON, msg, RRM1_QUEUE);

          // Change mode to connection establishment part 2.
          l1a_l1s_com.mode = CON_EST_MODE2;

          // end of process.
          return;
        }


        else
        if(SignalCode == MPHC_RA_REQ)
        // Random access message.
        //-----------------------
        {
          // REM: rand is added the msg content since its current content is the already
          // spent "slots" from the last RACH sending.
          l1a_l1s_com.ra_info.rand            += ((T_MPHC_RA_REQ *)(msg->SigP))->rand;
          l1a_l1s_com.ra_info.channel_request  = ((T_MPHC_RA_REQ *)(msg->SigP))->channel_request;

          // Activate RAACC task (no semaphore for UL tasks).
          l1a_l1s_com.l1s_en_task[RAACC] = TASK_ENABLED;   // Set RAACC task enable flag.

          // end of process.
          return;
        }

        else
        if(SignalCode == MPHC_STOP_RA_REQ)
        // Request to STOP the LINK ACCESS procedure.
        //-------------------------------------------
        {
          #if L1_GPRS
            UWORD8 i;

            // Store MAX TXPWR value to be used for first Tx PDCH blocks
            for(i = 0; i < 8; i++)
            {
              l1pa_l1ps_com.transfer.dl_pwr_ctrl.txpwr[i] = l1s.applied_txpwr;
            }
          #endif

          // send confirmation
          l1a_send_confirmation(MPHC_STOP_RA_CON,RRM1_QUEUE);


          // This process must be reset.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

//-----------------------------------------------------------------------------------------------

/*-------------------------------------------------------*/
/* l1a_dedicated_process()                               */
/*-------------------------------------------------------*/
/* Description : This state machine handles the dedicated*/
/* mode setup (L1A side).                                */
/*                                                       */
/* Starting messages:        MPHC_IMMED_ASSIGN_REQ       */
/*                                                       */
/* Subsequent messages:      MPHC_CHANNEL_ASSIGN_REQ     */
/*                           MPHC_SYNC_HO_REQ            */
/*                           MPHC_PRE_SYNC_HO_REQ        */
/*                           MPHC_PSEUDO_SYNC_HO_REQ     */
/*                           MPHC_ASYNC_HO_REQ           */
/*                           MPHC_ASYNC_HO_COMPLETE      */
/*                           MPHC_HANDOVER_FAIL_REQ      */
/*                           MPHC_CHANGE_FREQUENCY       */
/*                           OML1_CLOSE_TCH_LOOP_REQ     */
/*                           OML1_OPEN_TCH_LOOP_REQ      */
/*                           OML1_START_DAI_TEST_REQ     */
/*                           OML1_STOP_DAI_TEST_REQ      */
/*                                                       */
/* Result messages (input):  L1C_DEDIC_DONE              */
/*                           L1C_SACCH_INFO              */
/*                                                       */
/* Result messages (output): MPHC_CHANNEL_ASSIGN_CON     */
/*                           MPHC_SYNC_HO_CON            */
/*                           MPHC_PRE_SYNC_HO_CON        */
/*                           MPHC_PSEUDO_SYNC_HO_CON     */
/*                           MPHC_ASYNC_HO_CON           */
/*                           MPHC_TA_FAIL_IND            */
/*                           MPHC_DATA_IND               */
/*                           OML1_CLOSE_TCH_LOOP_CON     */
/*                           OML1_OPEN_TCH_LOOP_CON      */
/*                           OML1_START_DAI_TEST_CON     */
/*                           OML1_STOP_DAI_TEST_CON      */
/*                                                       */
/* Reset messages (input):   MPHC_CHANNEL_RELEASE        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_dedicated_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET     = 0,
    WAIT_INIT = 1,
    WAIT_MSG  = 2
  };

          T_DEDIC_SET  *free_set;
          UWORD8       *state      = &l1a.state[DEDICATED];
          UWORD32       SignalCode = msg->SignalCode;

  static  UWORD32       static_ho_fail_time_alignmt;
  static  UWORD32       static_ho_fail_fn_offset;
  static  T_DEDIC_SET  *static_ho_fail_aset;

#if ((L1_EOTD==1) && (L1_EOTD_QBIT_ACC == 1))
  static UWORD32        static_serv_fn_offset    = 0;
  static UWORD32        static_serv_time_alignmt = 0;
#endif

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset L1S dedicated mode manager trigger.
        l1a_l1s_com.dedic_set.SignalCode = NULL;
      }
      break;

      case WAIT_INIT:
      {
        switch(SignalCode)
        // switch on input message.
        //-------------------------------
        {
          case MPHC_IMMED_ASSIGN_REQ:
          // Immediate assignement message.
          //-------------------------------
          {
            UWORD8  maio_bef_sti;
#if (L1_FF_MULTIBAND == 1)
            UWORD16 operative_radio_freq;
#endif
            

            // Get Ptr to the free dedicated parameter set.
            // All important fields are initialised.
            free_set = l1a_get_free_dedic_set();

            // Save given dedicated channel parameters from MPHC_IMMED_ASSIGN_REQ msg.
            //========================================================================
            // Rem1: Mode is forced to Signalling Only.

            free_set->chan1.mode = SIG_ONLY_MODE;

            free_set->chan1.desc           = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->channel_desc;
            free_set->ma.freq_list         = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list;
            free_set->ma.freq_list_bef_sti = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti;
            maio_bef_sti                   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->maio_bef_sti;
            free_set->new_timing_advance   = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->timing_advance;
            free_set->dtx_allowed          = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->dtx_allowed;
            l1a_l1s_com.dedic_set.pwrc     = ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->pwrc;

            //----------------------------------------------
            //Rem: bcch_allocation, ba_id are not used!!!!!!
            //----------------------------------------------

            // New Timing Advance value must be applied on 1st frame of dedic. channel.
            free_set->timing_advance = free_set->new_timing_advance;

            // TXPWR command was given in Idle, save it in dedicated mode structure.
            free_set->new_target_txpwr = l1s.applied_txpwr;

            // Serving Cell stays the same.
            free_set->cell_desc                  = l1a_l1s_com.Scell_info;
            
#if (L1_FF_MULTIBAND == 0)
            
            free_set->cell_desc.traffic_meas_beacon
                                                 = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset];
            free_set->cell_desc.traffic_meas     = l1a_l1s_com.last_input_level[l1a_l1s_com.Scell_info.radio_freq - l1_config.std.radio_freq_index_offset];

#else // L1_FF_MULTIBAND = 1 below

            operative_radio_freq = 
                l1_multiband_radio_freq_convert_into_operative_radio_freq(l1a_l1s_com.Scell_info.radio_freq);
            free_set->cell_desc.traffic_meas_beacon = l1a_l1s_com.last_input_level[operative_radio_freq];
            free_set->cell_desc.traffic_meas     = l1a_l1s_com.last_input_level[operative_radio_freq];

#endif // #if (L1_FF_MULTIBAND == 0) else


            // Decode the "starting time field", since staying on the same serving
            // the same STI fn is saved in both "neig_sti_fn" and "serv_sti_fn".
            free_set->neig_sti_fn = l1a_decode_starting_time(((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time);
            free_set->serv_sti_fn = free_set->neig_sti_fn;

            // Check/Fill "before starting time" fields.
            l1a_fill_bef_sti_param(free_set, ((T_MPHC_IMMED_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time_present);

            // Use provided "before starting time MAIO" if hopping channel.
            if(free_set->chan1.desc_bef_sti.chan_sel.h)
              free_set->chan1.desc_bef_sti.chan_sel.rf_channel.hopping_rf.maio = maio_bef_sti;

            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // Save the "timeslot difference" between new and old configuration
            // in "tn_difference".
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.

            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
            {
              l1a_l1s_com.tn_difference += free_set->chan1.desc.timeslot_no - l1a_l1s_com.dl_tn;
              l1a_l1s_com.dl_tn         = free_set->chan1.desc.timeslot_no;  // Save new TN id.
            }
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
            // Note: The using of the semaphore associated to the SYNCHRO task can't be done
            // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
            // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
            // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
            // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.


            // Set "fset" pointer to the new parameter set.
            l1a_l1s_com.dedic_set.fset = free_set;

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = MPHC_IMMED_ASSIGN_REQ;

            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dedic(l1a_l1s_com.dedic_set.SignalCode);
            #endif

            // Set confirmation message name.
            l1a.confirm_SignalCode = MPHC_IMMED_ASSIGN_CON;

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;

          case MPHC_CHANNEL_ASSIGN_REQ:
          // Channel assignement message.
          //-----------------------------
          {
            UWORD8  supplied_txpwr;
            UWORD16 tch_radio_freq;

#if ((REL99 == 1) && (FF_BHO == 1))
              //restore long_rem_handover_type from previous handover activites
              l1a_l1s_com.dedic_set.long_rem_handover_type = 0; //reset handover to normal
#endif // #if ((REL99 == 1) && (FF_BHO == 1))

#if(L1_CHECK_COMPATIBLE == 1)
           l1a.stop_req = FALSE;
#endif

            // Get Ptr to the free dedicated parameter set.
            free_set = l1a_get_free_dedic_set();

            // Save given dedicated channel parameters from MPHC_CHANNEL_ASSIGN_REQ msg.
            //=============================================================--==========

            free_set->chan1.desc           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1;
            free_set->chan1.desc_bef_sti   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_1_bef_sti;
            free_set->chan1.mode           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_1;

            free_set->chan2.desc           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_2;
            free_set->chan2.desc_bef_sti   = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_desc_2_bef_sti;
            free_set->chan2.mode           = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->channel_mode_2;

            free_set->ma.freq_list         = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list;
            free_set->ma.freq_list_bef_sti = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->frequency_list_bef_sti;

            free_set->dtx_allowed          = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->dtx_allowed;

            supplied_txpwr                 = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->txpwr;

            #if (AMR == 1)
              // download AMR ver 1.0 information
              free_set->amr_configuration    = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->amr_configuration;
              free_set->cmip                 = C_AMR_CMIP_DEFAULT;
            #endif

            #if (TRACE_TYPE==3)
             l1_stats.chan_mode = free_set->chan1.mode;
            #endif

            // Determine which band we are transmitting on
            if(free_set->chan1.desc.chan_sel.h == TRUE)           // we are hopping therefore
              tch_radio_freq = free_set->ma.freq_list.rf_chan_no.A[0]; // take band from first element in maio list
            else                                                  // else take band from fixed radio_freq
              tch_radio_freq = free_set->chan1.desc.chan_sel.rf_channel.single_rf.radio_freq;

            // Check max transmit power (min txpwr) according to powerclass
            // and clip to MIN_TXPWR_LEVEL using new TCH radio_freq info.
            supplied_txpwr = l1a_clip_txpwr(supplied_txpwr,tch_radio_freq);

            free_set->new_target_txpwr = supplied_txpwr;

            if(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_mode != 0)
              free_set->a5mode = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->a5_algorithm + 1;
            else
              free_set->a5mode = 0;

            // Grab the cipher key
            free_set->ciph_key = ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->cipher_key;

            // Serving Cell stays the same.
            free_set->cell_desc = l1a_l1s_com.Scell_info;

            // Decode the "starting time field", since staying on the same serving
            // the same STI fn is saved in both "neig_sti_fn" and "serv_sti_fn".
            free_set->neig_sti_fn = l1a_decode_starting_time(((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time);
            free_set->serv_sti_fn = free_set->neig_sti_fn;

            // Check/Fill "before starting time" fields.
            l1a_fill_bef_sti_param(free_set, ((T_MPHC_CHANNEL_ASSIGN_REQ *)(msg->SigP))->starting_time.start_time_present);


            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // Save the "timeslot difference" between new and old configuration
            // in "tn_difference".
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.

            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
            {
              l1a_l1s_com.tn_difference += free_set->chan1.desc.timeslot_no - l1a_l1s_com.dl_tn;
              l1a_l1s_com.dl_tn         = free_set->chan1.desc.timeslot_no;  // Save new TN id.
            }
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
            // Note: The using of the semaphore associated to the SYNCHRO task can't be done
            // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
            // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
            // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
            // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.



            // Carry over the previous "tch_loop" settings for HW bit error testing */
            free_set->chan1.tch_loop = l1a_l1s_com.dedic_set.aset->chan1.tch_loop;
            free_set->chan2.tch_loop = l1a_l1s_com.dedic_set.aset->chan2.tch_loop;

            // Set "fset" pointer to the new parameter set.
            l1a_l1s_com.dedic_set.fset = free_set;

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = MPHC_CHANNEL_ASSIGN_REQ;

            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dedic(l1a_l1s_com.dedic_set.SignalCode);
            #endif

            // Set confirmation message name.
            l1a.confirm_SignalCode = MPHC_CHANNEL_ASSIGN_CON;

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;

          case MPHC_SYNC_HO_REQ:
          case MPHC_PRE_SYNC_HO_REQ:
          case MPHC_PSEUDO_SYNC_HO_REQ:
          case MPHC_ASYNC_HO_REQ:
          // Handover messages.
          //-------------------
          {
            WORD32   new_ta = 0;
            BOOL     nci    = 0;

            UWORD16  radio_freq;
            UWORD16  tch_radio_freq;
            UWORD32  ncc;
            UWORD32  bcc;
            UWORD32  time_alignmt = 0;
            UWORD32  fn_offset = 0;
            UWORD8   supplied_txpwr;
#if (L1_FF_MULTIBAND == 1)
            UWORD16  operative_radio_freq;
#endif
            

#if ((REL99 == 1) && (FF_BHO == 1))
              // Get the handover type: regular or blind
              switch(SignalCode)
              {
                case MPHC_SYNC_HO_REQ:
                {
                  l1a_l1s_com.dedic_set.handover_type = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_type;

                }
                break;

                case MPHC_PRE_SYNC_HO_REQ:
                {
                  l1a_l1s_com.dedic_set.handover_type = ((T_MPHC_PRE_SYNC_HO_REQ *)(msg->SigP))->handover_type;

                }
                break;

                case MPHC_ASYNC_HO_REQ:
                {
                  l1a_l1s_com.dedic_set.handover_type = ((T_MPHC_ASYNC_HO_REQ *)(msg->SigP))->handover_type;

                }
                break;

                case MPHC_PSEUDO_SYNC_HO_REQ:
                {
                  l1a_l1s_com.dedic_set.handover_type = ((T_MPHC_PSEUDO_SYNC_HO_REQ *)(msg->SigP))->handover_type;

                }
                break;
              }

              // BCCH frequency of target cell
              l1a_l1s_com.dedic_set.bcch_carrier_of_nbr_cell = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.cell_description.bcch_carrier;

              //Store handover type in another variable to be used in L1S till handover finished is issued.
              l1a_l1s_com.dedic_set.long_rem_handover_type   = l1a_l1s_com.dedic_set.handover_type;
#endif // #if ((REL99 == 1) && (FF_BHO == 1))

            // Get Ptr to the free dedicated parameter set.
            free_set = l1a_get_free_dedic_set();

            /*--------------------------------------------------------------*/
            /* Save msg content in the free "DEDICATED PARAM. STRUCT."      */
            /* Rem: since the HANDOVER PARAMETER structure is on the top of */
            /* each handover message, we can access it using any "cast".    */
            /* Here we chose the "(T_MPHC_SYNC_HO_REQ *)".                  */
            /*--------------------------------------------------------------*/

            // Download timing information from the command message.
          #if ((REL99 == 1) && (FF_BHO == 1))
            if (l1a_l1s_com.dedic_set.handover_type == NORMAL_HANDOVER)
          #endif
          {

            time_alignmt = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->time_alignmt;
            fn_offset    = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->fn_offset;
          }
            // Save time difference between new serving and previous one
            // in case of HANDOVER FAIL.
            static_ho_fail_time_alignmt = (5000 - time_alignmt) % 5000;
            static_ho_fail_fn_offset    = (1 + MAX_FN - fn_offset) % MAX_FN;

            #if ((L1_EOTD==1) && (L1_EOTD_QBIT_ACC ==1))
              // Save time_alignmt and fn_offset for Serving Cell E-OTD
              // in case of handover failure...

              static_serv_fn_offset = l1a_l1s_com.nsync.serv_fn_offset;
              static_serv_time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;

              l1a_l1s_com.nsync.serv_fn_offset = 0;
              l1a_l1s_com.nsync.serv_time_alignmt = 0;
            #endif


            if(time_alignmt == 0)
            // The 2 base stations are seen qbit synchronized...
            // -> Prevent frame diff. side effect.
            {
              static_ho_fail_fn_offset = (static_ho_fail_fn_offset + MAX_FN - 1) % MAX_FN;
            }

            // Save Acitve dedicated mode parameter set
            // in case of HANDOVER FAIL.
            static_ho_fail_aset = l1a_l1s_com.dedic_set.aset;

            // Setup new serving cell information.
            radio_freq = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.cell_description.bcch_carrier;
            ncc   = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.cell_description.ncc;
            bcc   = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.cell_description.bcc;

            free_set->cell_desc.radio_freq            = radio_freq;
            free_set->cell_desc.bsic             = (bcc) | (ncc << 3);
            free_set->cell_desc.time_alignmt     = time_alignmt;
            free_set->cell_desc.fn_offset        = fn_offset;
            free_set->cell_desc.meas.acc         = 0;
            free_set->cell_desc.meas.nbr_meas    = 0;

#if (L1_FF_MULTIBAND == 0) // TBD 
            free_set->cell_desc.traffic_meas_beacon = l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset];
            free_set->cell_desc.traffic_meas        = l1a_l1s_com.last_input_level[radio_freq - l1_config.std.radio_freq_index_offset];

#else // L1_FF_MULTIBAND = 1 below 

            operative_radio_freq = 
              l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);
            free_set->cell_desc.traffic_meas_beacon = l1a_l1s_com.last_input_level[operative_radio_freq];
            free_set->cell_desc.traffic_meas        = l1a_l1s_com.last_input_level[operative_radio_freq];
            
#endif // #if (L1_FF_MULTIBAND == 1) else                    

            // Download the message content.
            free_set->chan1.desc           = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_desc_1;
            free_set->chan1.desc_bef_sti   = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_desc_1_bef_sti;
            free_set->chan1.mode           = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_mode_1;

            free_set->chan2.desc           = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_desc_2;
            free_set->chan2.desc_bef_sti   = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_desc_2_bef_sti;
            free_set->chan2.mode           = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.channel_mode_2;

            free_set->ma.freq_list         = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.frequency_list;
            free_set->ma.freq_list_bef_sti = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.frequency_list_bef_sti;

            supplied_txpwr                 = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.txpwr;
            #if (AMR == 1)
              // download AMR ver 1.0 information
              switch(SignalCode)
              {
                case MPHC_SYNC_HO_REQ:
                {
                  free_set->amr_configuration  = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->amr_configuration;
                }
                break;

                case MPHC_PRE_SYNC_HO_REQ:
                {
                  free_set->amr_configuration  = ((T_MPHC_PRE_SYNC_HO_REQ *)(msg->SigP))->amr_configuration;
                }
                break;

                case MPHC_ASYNC_HO_REQ:
                {
                  free_set->amr_configuration  = ((T_MPHC_ASYNC_HO_REQ *)(msg->SigP))->amr_configuration;
                }
                break;
              }
            free_set->cmip                     = C_AMR_CMIP_DEFAULT;
            #endif

            // Determine which band we are transmitting on
            if(free_set->chan1.desc.chan_sel.h == TRUE)           // we are hopping therefore
              tch_radio_freq = free_set->ma.freq_list.rf_chan_no.A[0]; // take band from first element in maio list
            else                                                  // else take band from fixed radio_freq
              tch_radio_freq = free_set->chan1.desc.chan_sel.rf_channel.single_rf.radio_freq;

            // Check max transmit power (min txpwr) according to powerclass
            // and clip to MIN_TXPWR_LEVEL using new TCH radio_freq info.
            supplied_txpwr = l1a_clip_txpwr(supplied_txpwr,tch_radio_freq);

            free_set->new_target_txpwr     = supplied_txpwr;
            free_set->ho_acc               = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.ho_acc;

            // Copy in cipher key Kc.
            free_set->ciph_key = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->cipher_key;

            if(((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.cipher_mode != 0)
              free_set->a5mode = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.a5_algorithm + 1;
            else
              free_set->a5mode = 0;

            // Decode the "starting time field". Since changing the serving cell
            // a different STI fn is saved in "neig_sti_fn" and "serv_sti_fn".
            free_set->neig_sti_fn = l1a_decode_starting_time(((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.starting_time);
            if(free_set->neig_sti_fn != -1)
              free_set->serv_sti_fn = (free_set->neig_sti_fn - free_set->cell_desc.fn_offset + MAX_FN) % MAX_FN;

            // Check/Fill "before starting time" fields.
            l1a_fill_bef_sti_param(free_set, ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.starting_time.start_time_present);

            // TIMING ADVANCED COMPUTATION...(GSM05.10)
            //--------------------------------
            switch(SignalCode)
            {
              case MPHC_SYNC_HO_REQ:
              case MPHC_PSEUDO_SYNC_HO_REQ:
              {
                WORD32 t1_hbit;
                WORD32 otd_hbit;

                if(SignalCode == MPHC_SYNC_HO_REQ)
                // Synchronous Handover
                {
                  l1a.confirm_SignalCode = MPHC_SYNC_HO_CON; // Set confirmation message name.

                  //rtd_hbit_mod256 = 0;
                  nci = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->nci;
                }

                // Compute Timing Advance TA.
                //---------------------------
                // t1 = t0 + OTD - RTD (GSM05.10, $A1.3)
                // OTD: Observed Time Difference. ( OTD = time(BTS0) - time(BTS1) )
                // RTD: Report Time Difference.
                // t0:  one way line of sight propagation MS-BTS0 (old BTS) ( t0 = TA(BTS0) / 2 ).
                // t1:  one way line of sight propagation MS-BTS1 (new BTS) ( t1 = TA(BTS1) / 2 ).

                /*** Convert QBO to Half bits (HBO)                                          ***/
                otd_hbit = (((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->time_alignmt / 2);

                // If OTD is too high, it should be seen as a Negative value.
                if(otd_hbit > 1250)
                {
                    /*** Cell is advanced in timing from serving, hence TDMA arrives earlier
                         => Smaller Timing Advance and OTD is -ve                            ***/
                    otd_hbit -= 2500 ;
                }
                else
                {
                    /*** Cell is retarted in timing from serving, hence TDMA arrives later
                         => Larger Timing Advance and OTD is +ve                             ***/
                }

                t1_hbit = (WORD32) (otd_hbit + (WORD32) l1a_l1s_com.dedic_set.aset->new_timing_advance);

                if(t1_hbit < 0)
                   t1_hbit = 0;

                new_ta = t1_hbit;
              }
              break;

              case MPHC_PRE_SYNC_HO_REQ:
              // Pre-Synchronous Handover...
              {
                l1a.confirm_SignalCode = MPHC_PRE_SYNC_HO_CON; // Set confirmation message name.

                if(((T_MPHC_PRE_SYNC_HO_REQ *)(msg->SigP))->timing_advance_valid)
                  new_ta = ((T_MPHC_PRE_SYNC_HO_REQ *)(msg->SigP))->timing_advance;
                else
                  new_ta = 1;
              }
              break;

              case MPHC_ASYNC_HO_REQ:
              // Asynchronous Handover...
              {
                l1a.confirm_SignalCode = MPHC_ASYNC_HO_CON; // Set confirmation message name.
              }
              break;
            } // End switch...

            // Ensure TA is never set to > 63.
            if(new_ta > 63)
            {
               if(nci == 1)
               // Out of range TA must trigger a HO failure procedure.
               // GSM04.08, $3.4.4.4 and $10.5.2.39.
               {
                  // Send confirmation message to L3.
                  l1a_send_confirmation(MPHC_TA_FAIL_IND,RRM1_QUEUE);

                  // step in state machine.
                  *state = WAIT_MSG;

                  // Stop current L1A process.
                  return;
               }
               else
               // Max TA is 63.
               {
                  new_ta = 63;
               }
            }

            // Save computed TA in the new set.
            // This new TA shall be applied on 1st frame of new channel.
            free_set->new_timing_advance = (UWORD8)new_ta;
            free_set->timing_advance     = free_set->new_timing_advance;

#if ((REL99 == 1) && (FF_BHO == 1))
            free_set->nci                = nci;
            free_set->report_time_diff   = ((T_MPHC_SYNC_HO_REQ *)(msg->SigP))->handover_command.report_time_diff;

            if(SignalCode == MPHC_PSEUDO_SYNC_HO_REQ)
              free_set->real_time_difference = ((T_MPHC_PSEUDO_SYNC_HO_REQ *)(msg->SigP))->real_time_difference;
#endif
            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // Save the "timeslot difference" between new and old configuration
            // in "tn_difference".
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.

#if ((REL99 == 1) && (FF_BHO == 1))
            if(l1a_l1s_com.dedic_set.handover_type == NORMAL_HANDOVER)
            {
#endif
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
            {
              l1a_l1s_com.tn_difference += free_set->chan1.desc.timeslot_no - l1a_l1s_com.dl_tn;
              l1a_l1s_com.dl_tn         = free_set->chan1.desc.timeslot_no;  // Save new TN id.
            }
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
            // Note: The using of the semaphore associated to the SYNCHRO task can't be done
            // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
            // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
            // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
            // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.
#if ((REL99 == 1) && (FF_BHO == 1))
            }
#endif //#if ((REL99 == 1) && (FF_BHO == 1))


            // Set "fset" pointer to the new parameter set.
            l1a_l1s_com.dedic_set.fset = free_set;

#if ((REL99 == 1) && (FF_BHO == 1))
            // check whether handover is Normal or Blind. If it is normal handover,
            // follow the normal path. Otherwise go to blind handover state.
            if(l1a_l1s_com.dedic_set.handover_type == NORMAL_HANDOVER)
            {
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
            // Give new msg code to L1S.
            // Rem: only 2 cases handled by L1S, SYNC and ASYNC.
            if(SignalCode == MPHC_ASYNC_HO_REQ)
              l1a_l1s_com.dedic_set.SignalCode = MPHC_ASYNC_HO_REQ;
            else
              l1a_l1s_com.dedic_set.SignalCode = MPHC_SYNC_HO_REQ;


#if ((REL99 == 1) && (FF_BHO == 1))
              }
              else // Handover type is blind.
              {
                // step in state machine.
                free_set->HO_SignalCode = SignalCode;

                // Give new msg code to L1S. (below process is done by l1a_dedicated_process())
                l1a_l1s_com.dedic_set.SignalCode = MPHC_STOP_DEDICATED_REQ;

              }
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dedic(l1a_l1s_com.dedic_set.SignalCode);
            #endif

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;

          case MPHC_HANDOVER_FAIL_REQ:
          // Handover failled, we must go back to the previous channel.
          //-----------------------------------------------------------
          {
            // Give back the previous configuration in the "fset" ptr.
            l1a_l1s_com.dedic_set.fset = static_ho_fail_aset;

            // Set confirmation message name.
            l1a.confirm_SignalCode = MPHC_HANDOVER_FAIL_CON;

            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dedic(SignalCode);
            #endif

            #if ((L1_EOTD==1) && (L1_EOTD_QBIT_ACC ==1))
              // Restore E-OTD serving cell time_alignmt and fn_offset
              // from the cached versions...

              l1a_l1s_com.nsync.serv_fn_offset = static_serv_fn_offset;
              l1a_l1s_com.nsync.serv_time_alignmt = static_serv_time_alignmt;
            #endif

            // Sub the serving cell timeslot number to the Neigh./Serving timing
            // difference to format it for L1S use.
            l1a_sub_timeslot(&static_ho_fail_time_alignmt,
                             &static_ho_fail_fn_offset,
                             l1a_l1s_com.dl_tn);

            // Setup new serving cell information.
            l1a_l1s_com.dedic_set.fset->cell_desc.time_alignmt = static_ho_fail_time_alignmt;
            l1a_l1s_com.dedic_set.fset->cell_desc.fn_offset    = static_ho_fail_fn_offset;

            // In order to keep tn_difference and dl_tn consistent, we need to avoid
            // the execution of the SYNCHRO task with tn_difference updated and
            // dl_tn not yet updated (this can occur if we go in the HISR just after
            // the update of tn_difference). To do this the solution is to use the Semaphore
            // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
            // associated Semaphore is reset.
            // Note: Due to the specificity of the SYNCHRO task which can be enabled
            // by L1A state machines as by L1S processes, the semaphore can't followed
            // the generic rules of the Semaphore shared between L1A and L1S.
            // Since switching to a neigh cell, the known time_alignmt is about TN=0.
            // Set "tn_difference" with the new timeslot.
            //   tn_difference -> loaded with the number of timeslot to shift.
            //   dl_tn         -> loaded with the new timeslot.

            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;
            {
              l1a_l1s_com.tn_difference += l1a_l1s_com.dedic_set.fset->chan1.desc.timeslot_no;
            l1a_l1s_com.dl_tn         = l1a_l1s_com.dedic_set.fset->chan1.desc.timeslot_no;  // Save new TN id.
            }
            l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;
            // Note: The using of the semaphore associated to the SYNCHRO task can't be done
            // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
            // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
            // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
            // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;

          case MPHC_STOP_DEDICATED_REQ:
          // Release dedicated mode message.
          //--------------------------------
          {
            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            #if (TRACE_TYPE==5) && FLOWCHART
              trace_flowchart_dedic(l1a_l1s_com.dedic_set.SignalCode);
            #endif

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;

          case MPHC_CHANGE_FREQUENCY:
          // Frequency redefinition msg.
          //----------------------------
          {
            UWORD8          subchannel;

            // Get Ptr to the free dedicated parameter set.
            free_set = l1a_get_free_dedic_set();

            // Download Active set in the new set.
            *free_set = *l1a_l1s_com.dedic_set.aset;

            // Get subchannel for the target channel.
            subchannel = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc.subchannel;

            // Copy current parameters to BEF STI.
            free_set->chan1.desc_bef_sti   = free_set->chan1.desc;
            free_set->chan2.desc_bef_sti   = free_set->chan2.desc;
            free_set->ma.freq_list_bef_sti = free_set->ma.freq_list;

            // Download new param in AFTER STI.
            free_set->ma.freq_list = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->frequency_list;
            if(subchannel == free_set->chan1.desc.subchannel)
              // Target channel is CHAN1.
              free_set->chan1.desc = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc;
            else
              // Target channel is CHAN2.
              free_set->chan2.desc = ((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->channel_desc;

            // Compute starting time.
            free_set->neig_sti_fn = l1a_decode_starting_time(((T_MPHC_CHANGE_FREQUENCY *)(msg->SigP))->starting_time);
            free_set->serv_sti_fn = free_set->neig_sti_fn;

            // Set FREQUENCY REDEFINITION flag to trigger a confirmation when STI is reached.
            free_set->freq_redef_flag = TRUE;

            // Set "fset" pointer to the new parameter set.
            l1a_l1s_com.dedic_set.fset = free_set ;

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            // step in state machine.
            *state = WAIT_MSG;
          }
          break;
        } // end of "switch(SignalCode)".

        // end of process.
        end_process = 1;
      }
      break;

      case WAIT_MSG:
      {
        switch(SignalCode)
        // switch on input message.
        //-------------------------------
        {
          case L1C_DEDIC_DONE:
          // Dedicated channel activated.
          //-----------------------------
          {
            // Send confirmation message to L3.
            l1a_send_confirmation(l1a.confirm_SignalCode,RRM1_QUEUE);

            // End of process.
            end_process = 1;
          }
          break;

          case L1C_REDEF_DONE:
          // Dedicated channel activated.
          //-----------------------------
          {
            // Send confirmation message to L3.
            l1a_send_confirmation(MPHC_CHANGE_FREQUENCY_CON,RRM1_QUEUE);

            // End of process.
            end_process = 1;
          }
          break;

          case L1C_HANDOVER_FINISHED:
          // HANDOVER finished.
          //-------------------
          {
            #if ((REL99 == 1) && (FF_BHO == 1))
              // Forwarding the BHO params to L2/L3.
              ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->fn_offset = l1a_l1s_com.dedic_set.fn_offset;//static_nsync_bho.fn_offset;
              ((T_MPHC_HANDOVER_FINISHED *)(msg->SigP))->time_alignment = l1a_l1s_com.dedic_set.time_alignment;//static_nsync_bho.time_alignmt;
            #endif
            // Forward result message to L3.
            l1a_send_result(MPHC_HANDOVER_FINISHED, msg, RRM1_QUEUE);

            // End of process.

            end_process = 1;
          }
          break;

          case MPHC_CHANNEL_MODE_MODIFY_REQ:
          // New parameters are given concerning channel mode.
          //--------------------------------------------------
          {
            l1a_l1s_com.dedic_set.mode_modif.subchannel =
              ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->subchannel;
            l1a_l1s_com.dedic_set.mode_modif.channel_mode =
              ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->channel_mode;

            #if (AMR == 1)
              // download AMR ver 1.0 information
              l1a_l1s_com.dedic_set.mode_modif.amr_configuration =
                  ((T_MPHC_CHANNEL_MODE_MODIFY_REQ *)(msg->SigP))->amr_configuration;
              l1a_l1s_com.dedic_set.aset->cmip = C_AMR_CMIP_DEFAULT;
            #endif

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            // Send confirmation message to L3.
            l1a_send_confirmation(MPHC_CHANNEL_MODE_MODIFY_CON,RRM1_QUEUE);

            // End of process.
            end_process = 1;
          }
          break;

          case OML1_CLOSE_TCH_LOOP_REQ:
          // Close TCH loop.
          //----------------
          // Rem: frame_erasure field is supposed to contain "Y" and "Z" bit from
          //      CLOSE_TCH_LOOP_CMD (GSM11.10 $36.2.4.1). Wait for confirmation
          //      from CAPDEBIS. WARNING!!!!!!!!!1
          //         frame_erasure  |  Loop  |  tch_loop
          //         ---------------|--------|-----------
          //                        |  none  |     0
          //              0         |   A    |     1
          //              1         |   B    |     2
          //              2         |   C    |     3
          //              3         |   D    |     4
          //              4         |   E    |     5
          //              5         |   F    |     6
          //              6         |   I    |     7

          {
            if(l1a_l1s_com.mode == DEDIC_MODE)
            // Command message is valid only when dedic. mode is running.
            {
              UWORD8 subchannel = ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->sub_channel;
              UWORD8 tch_loop   = ((T_OML1_CLOSE_TCH_LOOP_REQ *)(msg->SigP))->frame_erasure + 1;

              if(subchannel == l1a_l1s_com.dedic_set.aset->chan1.desc.subchannel)
              // Loop must be closed on CHAN1, this is done "on fly".
              {
                l1a_l1s_com.dedic_set.aset->chan1.tch_loop = tch_loop;
              }
              else
              // Loop must be closed on CHAN2, this is done "on fly".
              {
                l1a_l1s_com.dedic_set.aset->chan2.tch_loop = tch_loop;
              }

              // Send confirmation message to L3.
              l1a_send_confirmation(OML1_CLOSE_TCH_LOOP_CON,RRM1_QUEUE);
            }

            // End of process.
            end_process = 1;
          }
          break;

          case OML1_OPEN_TCH_LOOP_REQ:
          // Open TCH loop.
          //---------------
          {
            // Any loop is opened, this is done "on fly".
            l1a_l1s_com.dedic_set.aset->chan1.tch_loop = 0;
            l1a_l1s_com.dedic_set.aset->chan2.tch_loop = 0;

            // Send confirmation message to L3.
            l1a_send_confirmation(OML1_OPEN_TCH_LOOP_CON,RRM1_QUEUE);

            // End of process.
            end_process = 1;
          }
          break;

          case OML1_START_DAI_TEST_REQ:
          // Start DAI test.
          //----------------
          //         tested_device  | dai_mode  |    test
          //         ---------------|-----------|--------------------
          //              0         |     0     |  no test
          //              1         |     2     |  speech decoder
          //              2         |     1     |  speech encoder
          //              3         |     0     |  no test
          //              4         |     3     |  Acouustic devices
          {
            const UWORD8 dai_transcode[] = {0, 2, 1, 0, 3};
            const UWORD8 dai_vbctl3[] = {0, 1, 2, 0, 3};
            UWORD32   vbctl3;

            UWORD8 dai_mode = dai_transcode[((T_OML1_START_DAI_TEST_REQ *)(msg->SigP))->tested_device];

            // DAI mode is set "on fly".
            l1a_l1s_com.dedic_set.aset->dai_mode = dai_mode;

            // program vbctl3
            //      bit 12 |  bit 11
            //      ----------------
            //         1   |    1   Acouustic devices
            //         1   |    0   speech encoder
            //         0   |    1   speech decoder
            //         0   |    0   no test

            #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3) || (ANALOG == 11))
              vbctl3 = ( (l1s_dsp_com.dsp_ndb_ptr ->d_dai_onoff & 0xE7FF) |
                       (dai_vbctl3[((T_OML1_START_DAI_TEST_REQ *)(msg->SigP))->tested_device] << 11) );
              l1s_dsp_com.dsp_ndb_ptr ->d_dai_onoff = vbctl3 | TRUE;
            #endif


            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            // Send confirmation message to L3.
            #if (OP_RIV_AUDIO == 1)
              l1a_audio_send_confirmation(OML1_START_DAI_TEST_CON);
            #else
              l1a_send_confirmation(OML1_START_DAI_TEST_CON,RRM1_QUEUE);
            #endif

            // End of process.
            end_process = 1;
          }
          break;

          case OML1_STOP_DAI_TEST_REQ:
          {
            UWORD32  vbctl3;

            // DAI test is stopped "on fly".
            l1a_l1s_com.dedic_set.aset->dai_mode = 0;

             #if ((ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3))
              // program vbctl3
              vbctl3 = (l1s_dsp_com.dsp_ndb_ptr ->d_dai_onoff & 0xE7FF);
              l1s_dsp_com.dsp_ndb_ptr ->d_dai_onoff = vbctl3 | TRUE;
            #endif

            // Give new msg code to L1S.
            l1a_l1s_com.dedic_set.SignalCode = SignalCode;

            // Send confirmation message to L3.
            #if (OP_RIV_AUDIO == 1)
              l1a_audio_send_confirmation(OML1_STOP_DAI_TEST_CON);
            #else
              l1a_send_confirmation(OML1_STOP_DAI_TEST_CON,RRM1_QUEUE);
            #endif

            // End of process.
            end_process = 1;
          }
          break;

          case MPHC_SET_CIPHERING_REQ:

          // Ciphering must be started or stopped.
          //--------------------------------------
          {
            if (l1a_l1s_com.dedic_set.handover_fail_mode)
            // The current state is the dedicated mode during an handover fail
            // the request must be ignored.
            // And a confirmation is returned to the L3.
            {
              // Send confirmation message to L3.
              l1a_send_confirmation(MPHC_SET_CIPHERING_CON,RRM1_QUEUE);
            }
            else
            {
              // GSM 4.08 $10.5.2.9 says (we suppose that "a5_algorithm" is
              // loaded with "algorithm identifier" and "cipher_mode" with
              // "SC").
              //  a5_algorithm from msg  |    A5 algo   | a5_algorithm in dedic set
              //  -----------------------|--------------|--------------------------
              //                         |              |          0 (no ciphering)
              //              0          |      A5/1    |          1
              //              1          |      A5/2    |          2
              //              2          |      A5/3    |          3
              //              3          |      A5/4    |          4
              //              4          |      A5/5    |          5
              //              5          |      A5/6    |          6
              //              6          |      A5/7    |          7

              UWORD8 cipher_mode  = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->cipher_mode;

              l1a_l1s_com.dedic_set.aset->ciph_key = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->new_ciph_param;

              if(cipher_mode != 0) // Save A5 algo.
                l1a_l1s_com.dedic_set.aset->a5mode = ((T_MPHC_SET_CIPHERING_REQ *)(msg->SigP))->a5_algorithm + 1;
              else // No cipering.
                l1a_l1s_com.dedic_set.aset->a5mode = 0;

              // Give new msg code to L1S.
              l1a_l1s_com.dedic_set.SignalCode = SignalCode;

              // Send confirmation message to L3.
              l1a_send_confirmation(MPHC_SET_CIPHERING_CON,RRM1_QUEUE);
            }

            end_process = 1;
          }
          break;

          case L1C_SACCH_INFO:
          // SACCH result messages.
          //-----------------------
          {
            // Forward result message to L3.
            l1a_send_result(PH_DATA_IND, msg, DLL1_QUEUE);

            // end of process.
            end_process = 1;
          }
          break;

          case L1C_STOP_DEDICATED_DONE:
          // Dedicated channel released
          //-----------------------------
          {
          #if ((REL99 == 1) && (FF_BHO == 1))
            // This event is disregarded in case of blind handover
            // (target cell synchro started)
            if(l1a_l1s_com.dedic_set.handover_type != BLIND_HANDOVER)
          #endif
            {
            // Send confirmation message to L3.
            l1a_send_confirmation(MPHC_STOP_DEDICATED_CON,RRM1_QUEUE);

            // Step in state machine.
            *state = RESET;
	      }

            // end of process.
            end_process = 1;
          }
          break;

          case MPHC_STOP_DEDICATED_REQ:
          case MPHC_CHANNEL_ASSIGN_REQ:
          case MPHC_SYNC_HO_REQ:
          case MPHC_PRE_SYNC_HO_REQ:
          case MPHC_PSEUDO_SYNC_HO_REQ:
          case MPHC_ASYNC_HO_REQ:
          case MPHC_HANDOVER_FAIL_REQ:
          case MPHC_CHANGE_FREQUENCY:
          // Reset messages.
          //----------------
          {
            // Step in state machine.
            *state = RESET;
          }
          break;
#if ((REL99 == 1) && (FF_BHO == 1))
          case L1C_FBSB_INFO:
          {
            T_DEDIC_SET  *free_set = l1a_l1s_com.dedic_set.fset;

            BOOL fb_found = ((T_L1C_FBSB_INFO*)(msg->SigP))->fb_flag;
            BOOL sb_found = ((T_L1C_FBSB_INFO*)(msg->SigP))->sb_flag;
            UWORD8 bsic   = ((T_L1C_FBSB_INFO*)(msg->SigP))->bsic;

            UWORD32  fn_offset = ((T_L1C_FBSB_INFO*)(msg->SigP))->fn_offset;
            UWORD32  time_alignmt = ((T_L1C_FBSB_INFO*)(msg->SigP))->time_alignmt;

            if ((fb_found == FALSE) ||
                (sb_found == FALSE) ||
                (bsic != free_set->cell_desc.bsic))
            // ------------------------
            // FB + SB detection failed
            // ------------------------
            {

              // Send reporting message with a faillure indication.
              xSignalHeaderRec *msg_new;

              msg_new = os_alloc_sig(sizeof(T_MPHC_HANDOVER_FINISHED));
              DEBUGMSG(status,NU_ALLOC_ERR)

              msg_new->SignalCode = MPHC_HANDOVER_FINISHED;

              if (fb_found == FALSE)
              {
                // FB attempt failed.
                ((T_MPHC_HANDOVER_FINISHED *)(msg_new->SigP))->cause = HO_FB_FAIL;
              }
              else // (fb_found == FALSE)
              {
                // SB attempt failed.
                // or BSIC is not matching.
                ((T_MPHC_HANDOVER_FINISHED *)(msg_new->SigP))->cause = HO_SB_FAIL;
              } // (fb_found == FALSE)

              l1a_send_result(MPHC_HANDOVER_FINISHED, msg_new, RRM1_QUEUE);

              // Reset handover to normal
              l1a_l1s_com.dedic_set.handover_type = 0;

              // End of process.
              end_process=1;//return;
            }
            else //((fb_found == FALSE) || (sb_found == FALSE) || (bsic != free_set->cell_desc.bsic))
            // -------------------------
            // FB + SB detection success
            // -------------------------
            {

              //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
              l1a_add_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);

              // update time_alignmt and fn_offset of new cell
              free_set->cell_desc.fn_offset = fn_offset;
              free_set->cell_desc.time_alignmt = time_alignmt;
              if(free_set->neig_sti_fn != -1)
                free_set->serv_sti_fn = (free_set->neig_sti_fn - free_set->cell_desc.fn_offset + MAX_FN) % MAX_FN;


              // This is to send across fn_offset and time_alignmt of new cell to L2/L3.
              l1a_l1s_com.dedic_set.fn_offset = fn_offset;
              l1a_l1s_com.dedic_set.time_alignment = time_alignmt;

              // reset handover to normal
              l1a_l1s_com.dedic_set.handover_type = 0;

              SignalCode = free_set->HO_SignalCode;

              // Save time difference between new serving and previous one
              // in case of HANDOVER FAIL.
              static_ho_fail_time_alignmt = (5000 - time_alignmt) % 5000;
              static_ho_fail_fn_offset = (1 + MAX_FN - fn_offset) % MAX_FN;

              if (time_alignmt == 0)
              // The 2 base stations are seen qbit synchronized...
              // -> Prevent frame diff. side effect.
              {
                static_ho_fail_fn_offset = (static_ho_fail_fn_offset + MAX_FN - 1) % MAX_FN;
              }

              // TIMING ADVANCED COMPUTATION...(GSM05.10)
              //-----------------------------------------
              switch(SignalCode)
              {
                case MPHC_SYNC_HO_REQ:
                case MPHC_PSEUDO_SYNC_HO_REQ:
                {
                  WORD32 t1_hbit;
                  WORD32 otd_hbit;
                  WORD32   new_ta = 0;
                  BOOL     nci    = 0;

                  if(SignalCode == MPHC_SYNC_HO_REQ)
                  // Synchronous Handover
                  {
                    nci = free_set->nci;
                  }

                  // Compute Timing Advance TA.
                  //---------------------------
                  // t1 = t0 + OTD - RTD (GSM05.10, $A1.3)
                  // OTD: Observed Time Difference. ( OTD = time(BTS0) - time(BTS1) )
                  // RTD: Report Time Difference.
                  // t0:  one way line of sight propagation MS-BTS0 (old BTS) ( t0 = TA(BTS0) / 2 ).
                  // t1:  one way line of sight propagation MS-BTS1 (new BTS) ( t1 = TA(BTS1) / 2 ).

                  // Convert QBO to Half bits (HBO)
                  otd_hbit = (time_alignmt / 2);

                  // If OTD is too high, it should be seen as a Negative value.
                  if(otd_hbit > 1250)
                  {
                    // Cell is advanced in timing from serving, hence TDMA arrives earlier
                    // => Smaller Timing Advance and OTD is -ve
                    otd_hbit -= 2500 ;
                  }
                  else
                  {
                    // Cell is retarted in timing from serving, hence TDMA arrives later
                    // => Larger Timing Advance and OTD is +ve
                  }

                  t1_hbit = (WORD32) (otd_hbit + (WORD32) static_ho_fail_aset->new_timing_advance);

                  if(t1_hbit < 0) t1_hbit = 0;

                  new_ta = t1_hbit;

                  // Ensure TA is never set to > 63.
                  if(new_ta > 63)
                  {
                    if(nci == 1)
                    // Out of range TA must trigger a HO failure procedure.
                    // GSM04.08, $3.4.4.4 and $10.5.2.39.
                    {
                      // Send confirmation message to L3.
                      l1a_send_confirmation(MPHC_TA_FAIL_IND,RRM1_QUEUE);

                      // End of process.
                      return;
                    }
                    else
                    // Max TA is 63.
                      {
                        new_ta = 63;
                      }
                    }

                    // Save computed TA in the new set.
                    // This new TA shall be applied on 1st frame of new channel.
                    free_set->new_timing_advance = (UWORD8)new_ta;
                    free_set->timing_advance     = free_set->new_timing_advance;
                  }
                  break;

                case MPHC_PRE_SYNC_HO_REQ:
                // Pre-Synchronous Handover...
                {
                  // no action for this message
                }
                break;

                case MPHC_ASYNC_HO_REQ:
                // Asynchronous Handover...
                {
                  // no action for this message
                }
                break;
              } // End switch...

              // In order to keep tn_difference and dl_tn consistent, we need to avoid
              // the execution of the SYNCHRO task with tn_difference updated and
              // dl_tn not yet updated (this can occur if we go in the HISR just after
              // the update of tn_difference). To do this the solution is to use the Semaphore
              // associated to the SYNCHRO task. SYNCHRO task will be schedule only if its
              // associated Semaphore is reset.
              // Note: Due to the specificity of the SYNCHRO task which can be enabled
              // by L1A state machines as by L1S processes, the semaphore can't follow
              // the generic rules of the Semaphore shared between L1A and L1S.
              // Save the "timeslot difference" between new and old configuration
              // in "tn_difference".
              // tn_difference -> loaded with the number of timeslot to shift.
              // dl_tn         -> loaded with the new timeslot.

              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_SET;

              l1a_l1s_com.tn_difference += free_set->chan1.desc.timeslot_no - l1a_l1s_com.dl_tn;
              l1a_l1s_com.dl_tn         = free_set->chan1.desc.timeslot_no;  // Save new TN id.

              l1a_l1s_com.task_param[SYNCHRO] = SEMAPHORE_RESET;

              // Note: The using of the semaphore associated to the SYNCHRO task can't be done
              // as it is for the other semaphores. This is due to the specificity of the SYNCHRO
              // task both touch by L1A and L1S. Here above the semaphore is set prior to touching
              // the SYNCHRO parameters and reset after. In L1S this semaphore is checked. If it's
              // seen SET then L1S will not execute SYNCHRO task nor modify its parameters.

              // Give new msg code to L1S.
              // Rem: only 2 cases handled by L1S, SYNC and ASYNC.
              if(SignalCode == MPHC_ASYNC_HO_REQ)
              {
                l1a_l1s_com.dedic_set.SignalCode = MPHC_ASYNC_HO_REQ;
              }
              else
              {
                l1a_l1s_com.dedic_set.SignalCode = MPHC_SYNC_HO_REQ;
              }

              if (((l1a_l1s_com.dedic_set.SignalCode == MPHC_ASYNC_HO_REQ )
                 || (l1a_l1s_com.dedic_set.SignalCode == MPHC_SYNC_HO_REQ ))
                 && (l1a_l1s_com.dedic_set.aset != NULL)
                 && (l1a_l1s_com.l1s_en_task[DEDIC] == TASK_DISABLED))
                   l1a_l1s_com.l1s_en_task[DEDIC] = TASK_ENABLED;

              #if (TRACE_TYPE==5) && FLOWCHART
                 trace_flowchart_dedic(l1a_l1s_com.dedic_set.SignalCode);
              #endif

              // End of process.
              end_process=1;// return;
            } // if ((fb_found == FALSE) || (sb_found == FALSE) || (bsic != free_set->cell_desc.bsic))
          } // case L1C_FBSB_INFO:
          break;
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
          default:
          // End of process.
          //----------------
          {
            end_process = 1;
          }
        } // end of "switch(SignalCode)".
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

/*-------------------------------------------------------*/
/* l1a_dedic6_process()                                  */
/*-------------------------------------------------------*/
/* Description : This state machine handles the 6 strong.*/
/* neighbor cells management in dedicated mode.          */
/*                                                       */
/* Remark: in dedicated mode there is no reason to use   */
/* the task parameters semaphores since there is no      */
/* ambiguity and no asynchronous/synchronous conflict to */
/* care about.                                           */
/*                                                       */
/* Starting messages:        L1C_DEDIC_DONE              */
/*                                                       */
/* Result messages (input):  L1C_FB_INFO                 */
/*                           L1C_SB_INFO                 */
/*                           L1C_SBCONF_INFO             */
/*                                                       */
/* Reset messages (input):   MPHC_CHANNEL_RELEASE        */
/*                                                       */
/*-------------------------------------------------------*/
#if (L1_12NEIGH == 0)

void l1a_dedic6_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1,
    WAIT_FB_RESULT     = 2,
    WAIT_SB_RESULT     = 3,
    WAIT_SBCONF_RESULT = 4
  };

         UWORD8  *state      = &l1a.state[DEDIC_6];
         UWORD32  SignalCode = msg->SignalCode;
  static UWORD8   nb_fb_attemp;

  // use only in packet transfer mode
  // these variables memorize this SBCNF parameters.
  static UWORD32  time_alignmt_mem;
  static UWORD32  fn_offset_mem;

  while(1)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset DEDIC6 tasks.
        l1a_l1s_com.l1s_en_task[SB51]    = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[FB51]    = TASK_DISABLED;

        l1a_l1s_com.l1s_en_task[SB26]    = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[FB26]    = TASK_DISABLED;

        l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[SBCNF51] = TASK_DISABLED;
      }
      break;

      case WAIT_INIT:
      {
        #if (L1_GPRS)
          // This machine works only for DEDICATED MODE and PACKET TRANSFER MODE
          // (Remark: PACKET TRANSFER MODE activated == PDTCH task activated)
          if((l1a_l1s_com.mode != DEDIC_MODE) && (l1a_l1s_com.l1s_en_task[PDTCH] != TASK_ENABLED)) return;
        #else
          // This machine works only for DEDICATED MODE.
          if (l1a_l1s_com.mode != DEDIC_MODE) return;
        #endif


        if(SignalCode == MPHC_NCELL_FB_SB_READ)
        // Request to make a synchro. ACQUISITION attempt with the given ARFCN.
        //---------------------------------------------------------------------
        // L1 makes 1 attempt to read the Frequency Burst (FBNEW task).
        //          1 attempt to read the Synchro Burst (SB2 task).
        {
          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // the request must be ignored.
          // Therefor the reporting message with a faillure indication is sent back
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

            // Reset state machine.
            *state = RESET;

            // End of process.
            return;
          }
          else
          {
            // Set task semaphores.
            l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;     // Set FB51    task semaphore.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;     // Set FB26    task semaphore.
            l1a_l1s_com.task_param[SB51] = SEMAPHORE_SET;     // Set SB51    task semaphore.
            l1a_l1s_com.task_param[SB26] = SEMAPHORE_SET;     // Set SB26    task semaphore.
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;  // Set SBCNF51 task semaphore.
            l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;  // Set SBCNF26 task semaphore.

            // This process always use the first element of "nsync" structure.
            l1a_l1s_com.nsync.current_list_size = 0;

            // Download Radio Freq Nb from the command message.
            // Other "nsync" parameters are unused.
            l1a_l1s_com.nsync.list[0].radio_freq = ((T_MPHC_NCELL_FB_SB_READ *)(msg->SigP))->radio_freq;

            // Step in state machine.
            *state = WAIT_FB_RESULT;

            // Enable FB task.
            if (l1a_l1s_com.mode == DEDIC_MODE)
            {
               UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;

               if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
               {
                 nb_fb_attemp              = 1;           // 1 attempt for FB51 detection.
                 l1a_l1s_com.l1s_en_task[FB51] = TASK_ENABLED;
               }
               else
               {
                 nb_fb_attemp              = 11;          // 11 attempts for FB26 detection.
                 l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
               }
            }
            #if (L1_GPRS)
              else
              {  // packet transfer mode
                nb_fb_attemp                  = 11;          // 11 attempts for FB26 detection.
                l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
              }
            #endif

            // End of process.
            return;
          }
        }


        else
        if(SignalCode == MPHC_NCELL_SB_READ)
        // Request to make a synchro. CONFIRMATION attempt with the given ARFCN.
        //---------------------------------------------------------------------
        // L1 makes 1 attempt to read the Synchro Burst (SBCONF task).
        {
          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // The request must be ignored
          // Therefor the reporting message with a failure indication is sent back
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

            // Reset state machine.
            *state = RESET;

            // End of process.
            return;
          }
          else
          {
            UWORD32  time_alignmt;
            UWORD32  fn_offset;

            // Set semaphores for all neighbor relative task.
            l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;     // Set FB51    task semaphore.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;     // Set FB26    task semaphore.
            l1a_l1s_com.task_param[SB51] = SEMAPHORE_SET;     // Set SB51    task semaphore.
            l1a_l1s_com.task_param[SB26] = SEMAPHORE_SET;     // Set SB26    task semaphore.
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;  // Set SBCNF51 task semaphore.
            l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;  // Set SBCNF26 task semaphore.


            // Download ARFCN & timing information from the command message.
            time_alignmt = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->time_alignmt;
            fn_offset    = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->fn_offset;
            time_alignmt_mem = time_alignmt;
            fn_offset_mem    = fn_offset;

            // Sub the serving cell timeslot number to the Neigh./Serving timing
            // difference to format it for L1S use.
            l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
            l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

            // Save neighbor information in the neighbor confirmation cell structure.
            l1a_l1s_com.nsync.list[0].radio_freq   = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->radio_freq;
            l1a_l1s_com.nsync.list[0].time_alignmt = time_alignmt;
            l1a_l1s_com.nsync.list[0].fn_offset    = fn_offset;

            // Step in state machine.
            *state = WAIT_SBCONF_RESULT;

            // Enable SBCONF task.
            if (l1a_l1s_com.mode == DEDIC_MODE)
            {
              UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;

              if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
                l1a_l1s_com.l1s_en_task[SBCNF51] = TASK_ENABLED;
              else
                l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_ENABLED;
            }
            #if (L1_GPRS)
              else
              {
                // packet transfer mode
                l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_ENABLED;
              }
            #endif

            // End of process.
            return;
          }
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break; // case WAIT_INIT


      case WAIT_FB_RESULT:
      {
        // Use incoming message.
        //----------------------
        if(SignalCode == L1C_FB_INFO)
        {
          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is trhe dedicated mode during an handover fail
          // The monitoring task must be stopped
          // And the reporting message with a failure indication must be sent back
          {
            // Disable the FB detection task
            l1a_l1s_com.l1s_en_task[FB26] = TASK_DISABLED;

            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

            // Reset state machine.
            *state = RESET;
          }
          else
          {
            BOOL    fb_found;

            // get result from message parameters.
            fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

            if(fb_found == TRUE)
            // FB attempt is a success.
            //-------------------------
            {
              // Step in state machine.
              *state = WAIT_SB_RESULT;

              // Enable SB task.
              if (l1a_l1s_com.mode == DEDIC_MODE)
              {
                UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
                if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
                // case SDCCH, mutliframe 51...
                {
                  l1a_l1s_com.l1s_en_task[FB51] = TASK_DISABLED;  // Disable FB51 task.
                  l1a_l1s_com.task_param[SB51]  = SEMAPHORE_SET;  // Set synchro semaphore for SB51 task.
                  l1a_l1s_com.l1s_en_task[SB51] = TASK_ENABLED;   // Enable SB51.
                }
                else
                // Dedicated D26 mode or Packet Transfer mode.
                {
                  l1a_l1s_com.l1s_en_task[FB26] = TASK_DISABLED;  // Disable FB26 task.
                  l1a_l1s_com.task_param[SB26]  = SEMAPHORE_SET;  // Set synchro semaphore for SB26 task.
                  l1a_l1s_com.l1s_en_task[SB26] = TASK_ENABLED;   // Enable SB51.
                }
              }
              #if (L1_GPRS)
                else
                { // packet transfer mode
                  l1a_l1s_com.l1s_en_task[FB26] = TASK_DISABLED;  // Disable FB26 task.
                  l1a_l1s_com.task_param[SB26]  = SEMAPHORE_SET;  // Set synchro semaphore for SB26 task.
                  l1a_l1s_com.l1s_en_task[SB26] = TASK_ENABLED;   // Enable SB51.
                  l1a_l1s_com.nsync.list[0].sb26_attempt =0;
                }
              #endif

              // End of process.
              return;
            }
            else
            // FB attempt failed.
            //-------------------
            {
              // REM:
              // case SDCCH, mutliframe 51..., FB51: 1  attempt.
              // case TCH,   mutliframe 26..., FB26: 11 attempts.

              if(--nb_fb_attemp == 0)
              {
                // Disable the FB detection task
                l1a_l1s_com.l1s_en_task[FB26] = TASK_DISABLED;

                // Send reporting message with a faillure indication.
                l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

                // Reset state machine.
                *state = RESET;
              }
              else
                return;
            } // endif fb_found
          }// endif l1a_l1s_com.dedic_set.handover_fail_mode
        }
        else
        // test in mode D26: L1C_DEDIC_DONE
        if(SignalCode == L1C_DEDIC_DONE)
        // New channel activated.
        //-----------------------
        {
          UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
          //---------------------------------------------
          // We restart the FB detection from scratch !!!
          //---------------------------------------------

          // Set semaphores for all neighbor relative task.
          l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;  // Set FB51    task semaphore.
          l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;  // Set FB26    task semaphore.

          // Step in state machine.
          *state = WAIT_FB_RESULT;

          if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
          {
            nb_fb_attemp                  = 1;           // 1 attempt for FB51 detection.
            l1a_l1s_com.l1s_en_task[FB51] = TASK_ENABLED;
          }
          else
          {
            nb_fb_attemp                  = 11;          // 11 attempts for FB26 detection.
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
          }
          // End of process.
           return;
        }
        #if (L1_GPRS)
        else
          // a new synchronisation was performed
          if(((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->tn_difference!=0))
             || (SignalCode == L1P_ALLOC_EXHAUST_DONE)
             || (SignalCode == L1P_REPEAT_ALLOC_DONE)
             || ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *) (msg->SigP))->tn_difference!=0) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all==0)))

          // New channel activated.
          //-----------------------
          {
            //---------------------------------------------
            // We restart the FB detection from scratch !!!
            //---------------------------------------------

            // Set semaphores for all neighbor relative task.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;  // Set FB26    task semaphore.

            // Step in state machine.
            *state = WAIT_FB_RESULT;

            nb_fb_attemp                  = 11;          // 11 attempts for FB26 detection.
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;

            // End of process.
             return;
          }
        #endif
        else
        #if (L1_GPRS)
          // End of packet transfer mode if TBF downlink and uplink have been released
          if((SignalCode == MPHC_STOP_DEDICATED_REQ) ||
            ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all)))
        #else
          if(SignalCode == MPHC_STOP_DEDICATED_REQ)
        #endif
          // Reset messages.
          //----------------
          {
            // Step in state machine.
            *state = RESET;
          }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case WAIT_SB_RESULT:
      {
        if(SignalCode == L1C_SB_INFO)
        // Synchro Burst acquisition attempt result.
        //------------------------------------------
        {
          BOOL  sb_found = ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag;

          #if (L1_GPRS)
             static UWORD8 SB26_attempt_counter =0;
          #endif

          if(sb_found == TRUE)
          // SB detection is a success.
          {
            UWORD32  *fn_offset_ptr    = &(((T_L1C_SB_INFO *)(msg->SigP))->fn_offset);
            UWORD32  *time_alignmt_ptr = &(((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt);

           #if (L1_GPRS)
              if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
              {
                SB26_attempt_counter = 0; // reset for next time
              }
            #endif


            // Correct "fn_offset" and "time_alignmt" to report the true
            // Serving/Neighbor time difference.
            //  1) Shift 20 bit since result is from a SB detection.
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
            l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);

            // This process must be reset.
            *state = RESET;
          }

          else
          // SB detection failled.
          {
          #if L1_GPRS
             if(l1a_l1s_com.l1s_en_task[DEDIC] == TASK_ENABLED)
         //---------------------------------------------
         // Dedicated mode tasks are enabled.
         //---------------------------------------------
            {
         #endif

               // Send reporting message with a faillure indication.
               l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

               // This process must be reset.
               *state = RESET;
        #if L1_GPRS
             }
        else
         if(l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED)
         //---------------------------------------------
         // Packet Transfer mode task is enabled.
         //---------------------------------------------
         {

            SB26_attempt_counter++;

            if (SB26_attempt_counter >= 2)
            {
              // Send reporting message with a faillure indication.
              l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

              SB26_attempt_counter = 0;

              // This process must be reset.
              *state = RESET;
            }
            else
            {
                // End of process.
                return;
            }
         }
       #endif

          }
        }

        else
        // test in mode D26: L1C_DEDIC_DONE
        if (SignalCode == L1C_DEDIC_DONE)
        // New channel activated.
        //-----------------------
        {
          UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;
          //---------------------------------------------
          // We restart the FB detection from scratch !!!
          //---------------------------------------------

          // Disable SB26 and SB51
          l1a_l1s_com.l1s_en_task[SB51] = TASK_DISABLED;
          l1a_l1s_com.l1s_en_task[SB26] = TASK_DISABLED;

          // Set semaphores for all neighbor relative task.
          l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;  // Set FB51    task semaphore.
          l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;  // Set FB26    task semaphore.

          // Step in state machine.
          *state = WAIT_FB_RESULT;

          if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
          {
            nb_fb_attemp                  = 1;           // 1 attempt for FB51 detection.
            l1a_l1s_com.l1s_en_task[FB51] = TASK_ENABLED;
          }
          else
          {
            nb_fb_attemp                  = 11;          // 11 attempts for FB26 detection.
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;
          }

          // End of process.
          return;
        }
        #if (L1_GPRS)
        else
          // a new synchronisation was performed
          if(((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->tn_difference!=0))
             || (SignalCode == L1P_ALLOC_EXHAUST_DONE)
             || (SignalCode == L1P_REPEAT_ALLOC_DONE)
             || ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *) (msg->SigP))->tn_difference!=0) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all==0)))


          // New channel activated.
          //-----------------------
          {
            //---------------------------------------------
            // We restart the FB detection from scratch !!!
            //---------------------------------------------

            // Disable SB26 and SB51
            l1a_l1s_com.l1s_en_task[SB26] = TASK_DISABLED;

            // Set semaphores for all neighbor relative task.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;  // Set FB26    task semaphore.

            // Step in state machine.
            *state = WAIT_FB_RESULT;

            nb_fb_attemp                  = 11;          // 11 attempts for FB26 detection.
            l1a_l1s_com.l1s_en_task[FB26] = TASK_ENABLED;

            // End of process.
            return;
          }
        #endif

        else
        #if (L1_GPRS)
          // in packet transfer mode: test PDTCH to be sure that TBF downlink and uplink are released
          if((SignalCode == MPHC_STOP_DEDICATED_REQ) ||
            ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all)))
        #else
          if(SignalCode == MPHC_STOP_DEDICATED_REQ)
        #endif
        // Reset messages.
        //----------------
        {
          // Step in state machine.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;

      case WAIT_SBCONF_RESULT:
      {
        if(SignalCode == L1C_SBCONF_INFO)
        // Synchro Burst acquisition attempt result.
        //------------------------------------------
        {
          UWORD8  sb_found = ((T_L1C_SBCONF_INFO *)(msg->SigP))->sb_flag;

          if(sb_found == TRUE)
          // SB detection is a success.
          {
            UWORD32  *fn_offset_ptr    = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_offset);
            UWORD32  *time_alignmt_ptr = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->time_alignmt);

            // Correct "fn_offset" and "time_alignmt" to report the true
            // Serving/Neighbor time difference.
            //  1) Shift 20 bit since result is from a SB detection.
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
            l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);

            // This process must be reset.
            *state = RESET;
          }

          else
          // SB detection failled.
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

            // This process must be reset.
            *state = RESET;
          }
        }

        else
        #if (L1_GPRS)
          // a new synchronisation was performed
          if((SignalCode == L1C_DEDIC_DONE)
             || ((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->tn_difference!=0))
             || ((SignalCode == L1P_TBF_RELEASED)  && (((T_L1P_TBF_RELEASED  *) (msg->SigP))->tn_difference!=0) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all==0))
             || (SignalCode == L1P_ALLOC_EXHAUST_DONE) || (SignalCode == L1P_REPEAT_ALLOC_DONE))
        #else
          if (SignalCode == L1C_DEDIC_DONE)
        #endif
        // New channel activated.
        //-----------------------
        {
          UWORD32  time_alignmt= 0; //omaps00090550
          UWORD32  fn_offset =0;    //omaps00090550

          // update the SBCNF26 parameters

          // disable SBCNF26 task
          l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_DISABLED;

          // Set semaphores for SBCNF26 task.
          l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;

          time_alignmt = time_alignmt_mem;
          fn_offset    = fn_offset_mem;

          // Sub the serving cell timeslot number to the Neigh./Serving timing
          // difference to format it for L1S use.
          l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
          l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

          l1a_l1s_com.nsync.list[0].time_alignmt = time_alignmt;
          l1a_l1s_com.nsync.list[0].fn_offset    = fn_offset;

          l1a_l1s_com.l1s_en_task[SBCNF26] = TASK_ENABLED;

          // Step in state machine.
          *state = WAIT_SBCONF_RESULT;

          // End of process.
          return;
        }

        else
        #if (L1_GPRS)
          // End of packet transfer mode if TBF downlink and uplink have been released
          if((SignalCode == MPHC_STOP_DEDICATED_REQ) ||
            ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all)))
        #else
          if(SignalCode == MPHC_STOP_DEDICATED_REQ)
        #endif
        // Reset messages.
        //----------------
        {
          // Step in state machine.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          // End of process.
          return;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.
#endif



//==================================================================//
//==================================================================//
//==================================================================//
//==================================================================//
//==================================================================//
//==================================================================//
//==================================================================//
#if (L1_12NEIGH ==1)
void l1a_dedic6_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET              = 0,
    WAIT_INIT          = 1,
    NSYNC_CONFIG       = 2,
    SELECT_BEST_NSYNC  = 3,
    WAIT_NSYNC_RESULT  = 4,
    STOP_NSYNC         = 5
    #if (L1_EOTD==1)
     ,SCELL_CONFIG     = 6
    #endif
  };

  UWORD8  *state      = &l1a.state[DEDIC_6];
  UWORD32  SignalCode = msg->SignalCode;

 // use only in packet transfer mode
  // these variables memorize this SBCNF parameters.
  static UWORD32  time_alignmt_mem;
  static UWORD32  fn_offset_mem;
  //#if !L1_R99
  static UWORD8   nb_fb_attempt;
  //#endif

  // For EOTD purpose we need of flag to identify 1st/last SB Serving Cell
  #if (L1_EOTD==1)
    static BOOL  first_scell;
    static BOOL  last_scell;
    static BOOL  eotd_started;
  #endif

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // Step in state machine.
        *state = WAIT_INIT;

        // Reset of process is embbeded in other state to
        // avoid conflicts between processes using the same
        // L1S tasks (Initial Netwirk sync + Idle 6 strongest)
      }
      break;


      case WAIT_INIT:
      {
        #if (L1_GPRS)
          // This machine works only for DEDICATED MODE and PACKET TRANSFER MODE
          // (Remark: PACKET TRANSFER MODE activated == PDTCH task activated)
          if((l1a_l1s_com.mode != DEDIC_MODE) && (l1a_l1s_com.l1s_en_task[PDTCH] != TASK_ENABLED)) return;
        #else
          // This machine works only for DEDICATED MODE.
          if (l1a_l1s_com.mode != DEDIC_MODE) return;
        #endif

        // Request to make a synchro. ACQUISITION attempt with the given ARFCN.
        //---------------------------------------------------------------------
        if ((SignalCode == MPHC_NCELL_FB_SB_READ) || (SignalCode == MPHC_NCELL_SB_READ) ||
            (SignalCode == MPHC_NCELL_SYNC_REQ) ||(SignalCode == MPHC_NCELL_LIST_SYNC_REQ) )
        {
          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // So the monitoring tasks must ALL be stopped and the reporting
          // message with a failure indication must be sent back
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, 0);

            *state = STOP_NSYNC;
            break;
          }
          else
          {
            #if (L1_EOTD==1)
              // Check request validity for this process: for EOTD, nsync must be FREE...
              if( (SignalCode == MPHC_NCELL_LIST_SYNC_REQ) &&
                  (((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->eotd == TRUE) &&
                  (l1a_l1s_com.nsync.current_list_size != 0) )
                // End of process.
                return;
            #endif

            l1a_l1s_com.nsync.first_in_list=0; //reset first_in_list

            // Set semaphores for all neighbor relative task.
            l1a_l1s_com.task_param[NSYNC]= SEMAPHORE_SET;     // Set NSYNC   task semaphore.
            l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;     // Set FB51    task semaphore.
            l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;     // Set FB26    task semaphore.
            l1a_l1s_com.task_param[SB51] = SEMAPHORE_SET;     // Set SB51    task semaphore.
            l1a_l1s_com.task_param[SB26] = SEMAPHORE_SET;     // Set SB26    task semaphore.
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;  // Set SBCNF51 task semaphore.
            l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;  // Set SBCNF26 task semaphore.

            // Step in state machine.
            *state = NSYNC_CONFIG;
          }
        }

        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;


      case NSYNC_CONFIG:
      {
        UWORD8   neigh_id = l1a_l1s_com.nsync.first_in_list;
        UWORD32  time_alignmt =0; //omaps00090550
        UWORD32  fn_offset=0; //omaps00090550;

        // Request to acquire FB/SB or to confirm FB or SB from one ncell.
        //----------------------------------------------------------------
        // Abort if there is no room for a new neighbour synchro request.
        if(l1a_l1s_com.nsync.current_list_size >= NBR_NEIGHBOURS)
        {
          *state = WAIT_NSYNC_RESULT;
          // end of this process
          return;
        }

        // There is at least one free location within L1 structure. Find it!

        while((l1a_l1s_com.nsync.list[neigh_id].status != NSYNC_FREE))
        {
          neigh_id++;
          if (neigh_id == NBR_NEIGHBOURS) neigh_id=0;
        }

        // TEMPORARY Code to insure transition between
        // OLD and NEW "6 strongest Interface"
        //----------------------------------------------
        if (SignalCode == MPHC_NCELL_FB_SB_READ)
        {
          // Init timing_validity and read radio freq.
          l1a_l1s_com.nsync.list[neigh_id].timing_validity = 0;
          l1a_l1s_com.nsync.list[neigh_id].radio_freq = ((T_MPHC_NCELL_FB_SB_READ *)(msg->SigP))->radio_freq;
          l1a_l1s_com.nsync.list[neigh_id].time_alignmt = 0;
          l1a_l1s_com.nsync.list[neigh_id].fn_offset    = 0;

          // Ncell WAITING
          l1a_l1s_com.nsync.list[neigh_id].status  = NSYNC_WAIT;

          // Increment list size
          l1a_l1s_com.nsync.current_list_size += 1;
        }
        else
        if (SignalCode == MPHC_NCELL_SB_READ)
        {
          // Init timing_validity and read radio freq.
          l1a_l1s_com.nsync.list[neigh_id].timing_validity = 2;
          l1a_l1s_com.nsync.list[neigh_id].radio_freq   = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->radio_freq;
          time_alignmt = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->time_alignmt;
          fn_offset    = ((T_MPHC_NCELL_SB_READ *)(msg->SigP))->fn_offset;

          // Ncell WAITING
          l1a_l1s_com.nsync.list[neigh_id].status  = NSYNC_WAIT;

          // Increment list size
          l1a_l1s_com.nsync.current_list_size += 1;
        }
        else
        if (SignalCode == MPHC_NCELL_SYNC_REQ)
        {
          // Init timing_validity and read radio freq.
          l1a_l1s_com.nsync.list[neigh_id].timing_validity =((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->timing_validity;
          l1a_l1s_com.nsync.list[neigh_id].radio_freq = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq;

#if (L1_FF_MULTIBAND == 1)
          {
            UWORD8 physical_band_id;
            physical_band_id = 
                l1_multiband_radio_freq_convert_into_physical_band_id(l1a_l1s_com.nsync.list[neigh_id].radio_freq);
            L1_MULTIBAND_TRACE_PARAMS(MULTIBAND_PHYSICAL_BAND_TRACE_ID,multiband_rf[physical_band_id].gsm_band_identifier);
          }
#endif /*#if (L1_FF_MULTIBAND == 1)*/
          

          if (l1a_l1s_com.nsync.list[neigh_id].timing_validity != 0)
          {
            time_alignmt = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->time_alignmt;
            fn_offset    = ((T_MPHC_NCELL_SYNC_REQ *)(msg->SigP))->fn_offset;
            #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
              if (l1a_l1s_com.nsync.list[neigh_id].timing_validity == 3)
              {
                l1a_l1s_com.nsync.list[neigh_id].nb_fb_attempt = 2 ;
                l1a_l1s_com.nsync.list[neigh_id].fb26_position = 255 ;
              }
            #endif
          }
          else
          {
            l1a_l1s_com.nsync.list[neigh_id].time_alignmt = 0;
            l1a_l1s_com.nsync.list[neigh_id].fn_offset    = 0;
          }

          // Ncell WAITING
          l1a_l1s_com.nsync.list[neigh_id].status  = NSYNC_WAIT;

          // Increment list size
          l1a_l1s_com.nsync.current_list_size += 1;
        }

        if ( (SignalCode == MPHC_NCELL_SYNC_REQ) || (SignalCode == MPHC_NCELL_SB_READ) )
        {
          // Correct timing info from request message with serving cell info.
          //-----------------------------------------------------------------
          if ( l1a_l1s_com.nsync.list[neigh_id].timing_validity != 0)
          {
            // correct timing
            l1a_correct_timing (neigh_id,time_alignmt,fn_offset);
          }
        }

        if ( SignalCode == MPHC_NCELL_LIST_SYNC_REQ )
        {
          // Request to read FB/SB or SB from 1 to 12 neighbour cells.
          //----------------------------------------------------------
          UWORD8  nbr_free = (UWORD8) (NBR_NEIGHBOURS) - l1a_l1s_com.nsync.current_list_size;
          UWORD8 list_size = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP))->list_size;
          T_MPHC_NCELL_LIST_SYNC_REQ *pt = ((T_MPHC_NCELL_LIST_SYNC_REQ *)(msg->SigP));
          UWORD8  i;

          // Abort if there is no room for a new neighbour synchro request.
          if ( (l1a_l1s_com.nsync.current_list_size >= NBR_NEIGHBOURS) ||
               (nbr_free < list_size) )
          {
            *state = WAIT_NSYNC_RESULT;

            // end of this process
            return;
          }

          #if (L1_EOTD==1)
          // Abort if list is not empty before receiving EOTD request ...
          if ((pt->eotd == TRUE) && (l1a_l1s_com.nsync.current_list_size !=0))
          {
            *state = WAIT_NSYNC_RESULT;

            // end of this process
            return;
          }

            // store Eotd flag
            l1a_l1s_com.nsync.eotd_meas_session = pt->eotd;
          #endif

          // Download neighbour info from request message.
          //----------------------------------------------
          for (i=0; i<list_size; i++,neigh_id++)
          {
            if (neigh_id == NBR_NEIGHBOURS) neigh_id = 0;

            // Look for first free location within L1 structure.
            // There is at least one free location within L1 structure. Find it!
            while((l1a_l1s_com.nsync.list[neigh_id].status != NSYNC_FREE))
            {
              neigh_id++; if(neigh_id == NBR_NEIGHBOURS) neigh_id=0;;
            }

            l1a_l1s_com.nsync.list[neigh_id].radio_freq      = pt->ncell_list[i].radio_freq;
            l1a_l1s_com.nsync.list[neigh_id].timing_validity = pt->ncell_list[i].timing_validity;

            if( l1a_l1s_com.nsync.list[neigh_id].timing_validity != 0)
            {
              UWORD32  time_alignmt;
              UWORD32  fn_offset;

              // Download ARFCN, timing information and bitmap from the command message.
              time_alignmt   = pt->ncell_list[i].time_alignmt;
              fn_offset      = pt->ncell_list[i].fn_offset;

              // correct timing
              l1a_correct_timing (neigh_id,time_alignmt,fn_offset);
              #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
                if (l1a_l1s_com.nsync.list[neigh_id].timing_validity == 3)
                {
                  l1a_l1s_com.nsync.list[neigh_id].nb_fb_attempt = 2 ;
                  l1a_l1s_com.nsync.list[neigh_id].fb26_position = 255 ;
                }
              #endif
            }
            else
            {
              l1a_l1s_com.nsync.list[neigh_id].fn_offset    = 0;
              l1a_l1s_com.nsync.list[neigh_id].time_alignmt = 0;
            }
             // set Ncell WAITING
            l1a_l1s_com.nsync.list[neigh_id].status  = NSYNC_WAIT;

            // Increment list size
            l1a_l1s_com.nsync.current_list_size += 1;

          } // end for
        } // end if MPHC_NCELL_LIST_SYNC_REQ

        #if (L1_EOTD ==1)
          if ( l1a_l1s_com.nsync.eotd_meas_session == TRUE)
          {
            // Step in state machine.
            *state = SCELL_CONFIG;
          }
          else
        #endif
        {
          // Step in state machine.
          *state = SELECT_BEST_NSYNC;
        }
      }
      break; // case NSYNC_CONFIG


      #if (L1_EOTD==1)
        case SCELL_CONFIG:
        {

          UWORD32 fn_offset=0;
          UWORD32 time_alignmt=0;

          // Enable L1S activity on the Serving Cell confirmation task
          // selects 1st Serving cell monitoring
          if (eotd_started == FALSE)
          {
             eotd_started = TRUE;
             first_scell    = TRUE;
          }
          // selects last Serving cell monitoring
          else
          {
             eotd_started = FALSE;
             last_scell   = TRUE;
          }

          // In Eotd : Serving cell is not part of the list
          //-----------------------------------------------
          // But it must be the 1st monitored
          // Sub the serving cell timeslot number to the Neigh./Serving timing
          // difference to format it for L1S use.
          #if (L1_EOTD_QBIT_ACC== 1)
            time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;
            fn_offset    = l1a_l1s_com.nsync.serv_fn_offset;
          #endif

          l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
          l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

          // Load Serving cell in last position [NBR_NEIGHBOURS]
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].radio_freq   = l1a_l1s_com.Scell_info.radio_freq;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset    = fn_offset;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt = time_alignmt;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].timing_validity = 2;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset_mem   = 0;
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt_mem= 0;

          // Increment list size
          l1a_l1s_com.nsync.current_list_size += 1;

          // Enable Serving cell monitoring
          l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status  = NSYNC_PENDING;
          l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

          // Step in state machine.
          *state = WAIT_NSYNC_RESULT;

          // End of process.
          return;
        }
      #endif

      case SELECT_BEST_NSYNC:
      {
        BOOL exec_pending_task = FALSE;
        UWORD8 id_acquisition;
        UWORD8 i = 0;

        // list empty return to STOP_NSYNC
        //---------------------------
        if (l1a_l1s_com.nsync.current_list_size == 0)
        {
          #if (L1_EOTD==1)
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
            {
              *state = SCELL_CONFIG;          // Step in state machine.
              break;
            }
            else
          #endif
          {
            *state = STOP_NSYNC;             // reset process
            break;
          }
        }

        // disable all tasks
        l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

        // Set semaphores for all neighbor relative task.
        l1a_l1s_com.task_param[NSYNC]= SEMAPHORE_SET;     // Set NSYNC   task semaphore.
        l1a_l1s_com.task_param[FB51] = SEMAPHORE_SET;     // Set FB51    task semaphore.
        l1a_l1s_com.task_param[FB26] = SEMAPHORE_SET;     // Set FB26    task semaphore.
        l1a_l1s_com.task_param[SB51] = SEMAPHORE_SET;     // Set SB51    task semaphore.
        l1a_l1s_com.task_param[SB26] = SEMAPHORE_SET;     // Set SB26    task semaphore.
        l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;  // Set SBCNF51 task semaphore.
        l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;  // Set SBCNF26 task semaphore.


        // Look if there is an ACQUISITION in progress (PENDING) in order to disable
        // other cells.
        // If MF51, ACQUISITION means FB and SB. If MF26, ACQUISITION means FB only
        //---------------------------------------------------------------------------
        // If so, let the new request in WAIT ...
        while ( (i<NBR_NEIGHBOURS) && (exec_pending_task==FALSE))
        {
          if (l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING)
          {   // check FB or SB acquisition

             if ( l1a_l1s_com.nsync.list[i].timing_validity == 0 )
             {
                exec_pending_task = TRUE;
             }
             else if (l1a_l1s_com.mode == DEDIC_MODE)
             {
                UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;

              #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
                if( (l1a_l1s_com.nsync.list[i].timing_validity == SB_ACQUISITION_PHASE)
              #else
                if( (l1a_l1s_com.nsync.list[i].timing_validity == 3)
              #endif
                    &&((channel_type == SDCCH_4) || (channel_type == SDCCH_8)) )
                    exec_pending_task = TRUE;
             }
          }
          i++;
        }

        // If there is an ACQUISITION in progress, put all other tasks in WAIT
        //--------------------------------------------------------------------
        if (exec_pending_task == TRUE)
        {
          // if This SB acquisition comes from an FB CONFIRMATION, there may be some SB or
          // FB CONFIRMATIONS in PENDING state ==> put them in WAIT to insure FB/SB acquisition.
          UWORD8 cell_in_acquisition = i-1;

        #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
          if (l1a_l1s_com.nsync.list[cell_in_acquisition].timing_validity == SB_ACQUISITION_PHASE)
        #else
          if (l1a_l1s_com.nsync.list[cell_in_acquisition].timing_validity == 3)
        #endif
          { //This case is only for MF51
            for (i=0; i < NBR_NEIGHBOURS; i++)
              if ((l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING) && (i != cell_in_acquisition))
                   l1a_l1s_com.nsync.list[i].status = NSYNC_WAIT;
          }

          // enable NSYNC
          l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

          // Step in state machine.
          *state = WAIT_NSYNC_RESULT;

          // End of process.
          return;
        }

        // Otherwise : may be some other tasks (everything but FB aquisition) to set PENDING
        // In case we are in MF51, these tasks are SB and FB confirmation
        // In case we are in MF26, these tasks are SB and FB confirmation and SB acquisition
        //------------------------------------------------------

        exec_pending_task = FALSE;
          for (i=0; i < NBR_NEIGHBOURS; i++)
        {
          if ( (l1a_l1s_com.nsync.list[i].status == NSYNC_WAIT) ||
               (l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING) )
          {
            // set all other requested tasks PENDING
            if ( l1a_l1s_com.nsync.list[i].timing_validity != 0 )
            {
              l1a_l1s_com.nsync.list[i].status = NSYNC_PENDING;
              exec_pending_task = TRUE;
              // initialise FB attempts for FB CONFIRMATION
              if (l1a_l1s_com.nsync.list[i].timing_validity == 1)
              #if ((REL99 == 1) && (FF_BHO == 1))
                l1a_l1s_com.nsync.list[i].nb_fb_attempt = 1;
              #else
               nb_fb_attempt = 1;
              #endif
            }
          }
        }

        // If there are no pending task, perhaps there is an FB aquisition task to set pending.
        // In this case it is the older request, i.e. first_in_list.
        if (exec_pending_task == FALSE)
        {
          id_acquisition = l1a_l1s_com.nsync.first_in_list;
          // id has already been found.......
          l1a_l1s_com.nsync.list[id_acquisition].status = NSYNC_PENDING;

          if (l1a_l1s_com.mode == DEDIC_MODE)
          {
            UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;

            if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
            {
              #if ((REL99 == 1) && (FF_BHO == 1))
                if (l1a_l1s_com.nsync.list[id_acquisition].timing_validity != 3)
                  l1a_l1s_com.nsync.list[id_acquisition].nb_fb_attempt = 1;;
              #else
              nb_fb_attempt = 1;           // 1 attempt for FB51 detection.
              #endif
            }
            else
            {
              #if ((REL99 == 1) && (FF_BHO == 1))
                l1a_l1s_com.nsync.list[id_acquisition].nb_fb_attempt = 11;
              #else
              nb_fb_attempt = 11;          // 11 attempts for FB26 detection.
              #endif // #if ((REL99 == 1) && (FF_BHO == 1))
            }
          }
          #if (L1_GPRS)
          else
          {
            // packet transfer mode
            #if ((REL99 == 1) && (FF_BHO == 1))
              if (l1a_l1s_com.nsync.list[i].timing_validity == 0)
                l1a_l1s_com.nsync.list[id_acquisition].nb_fb_attempt = 11; // 11 attempts for FB26 detection.
              else if (l1a_l1s_com.nsync.list[i].timing_validity == 1)
                l1a_l1s_com.nsync.list[id_acquisition].nb_fb_attempt = 1;
            #else
            nb_fb_attempt  = 11;          // 11 attempts for FB26 detection.
            #endif //#if ((REL99 == 1) && (FF_BHO == 1))
          }
          #endif
        }

        // enable NSYNC
        l1a_l1s_com.l1s_en_task[NSYNC]           = TASK_ENABLED;

        // Step in state machine.
        *state = WAIT_NSYNC_RESULT;

        // End of process.
        end_process = 1;
      }
      break; // case SELECT_BEST_NSYNC

      case WAIT_NSYNC_RESULT:
      {

        if(SignalCode == L1C_FB_INFO)
        // Frequency Burst acquisition attempt result.
        //--------------------------------------------
        {
          BOOL    fb_found;
          UWORD8  neigh_id         = ((T_L1C_FB_INFO *)(msg->SigP))->neigh_id;
          UWORD16 neigh_radio_freq = ((T_L1C_FB_INFO *)(msg->SigP))->radio_freq;

          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          #if ((REL99 == 1) && (FF_BHO == 1))
          if ((neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)||
              ((neigh_radio_freq == l1a_l1s_com.nsync.list[neigh_id].radio_freq)&&
              (l1a_l1s_com.nsync.list[neigh_id].status==NSYNC_FREE)))
          #else
           if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)
          #endif
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // The monitoring tasks must be stopped
          // And the reporting message with a failure indication must be sent back
          {
            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);

            #if (L1_EOTD==1)
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                // Stop Eotd session.
                l1a_l1s_com.nsync.eotd_meas_session = FALSE;

                // Set mode DEDIC
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;
              }
            #endif

            // This process must be reset
            *state = STOP_NSYNC;
            break;
          }

          // Get result from the message.
          fb_found = ((T_L1C_FB_INFO*)(msg->SigP))->fb_flag;

          if(fb_found == TRUE)
          // FB attempt is a success.
          // same process Now for DEDIC and PDTCH !!!!
          {
            // request was FB Confirmation + SB acquisition .....
          #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
            l1a_l1s_com.nsync.list[neigh_id].timing_validity = SB_ACQUISITION_PHASE ;
          #else
            l1a_l1s_com.nsync.list[neigh_id].timing_validity = 3;
          #endif
            l1a_l1s_com.nsync.list[neigh_id].sb26_attempt = 0;

            *state = SELECT_BEST_NSYNC;

            // Must break here to make sure the scheduler
            // runs immediately, else the next synchro attempt is not
            // started and we get a lock-up condition.
            break;
          }
          else
          // FB attempt failed.
          //-------------------
          {
          #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
            if ((l1a_l1s_com.nsync.list[neigh_id].timing_validity != 0) && (l1a_l1s_com.mode == DEDIC_MODE))
            {
              UWORD8 channel_type = l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type;

              if((channel_type == SDCCH_4) || (channel_type == SDCCH_8))
                l1a_l1s_com.nsync.list[neigh_id].nb_fb_attempt = 1; // force attempt to 1 because we are in SDCCH
            }

            l1a_l1s_com.nsync.list[neigh_id].nb_fb_attempt-- ;
            if (l1a_l1s_com.nsync.list[neigh_id].nb_fb_attempt == 0)

          #else
            --nb_fb_attempt;
            if (nb_fb_attempt == 0)
          #endif
            {
              // if attempt was an FB confirmation, go back to FB full acquisition
              // and set it to WAIT state
              if (l1a_l1s_com.nsync.list[neigh_id].timing_validity == 1)
              {
                l1a_l1s_com.nsync.list[neigh_id].timing_validity = 0;
                l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_WAIT;

                // Step in state machine.
                *state = SELECT_BEST_NSYNC;

                // Must break here otherwise a SYNC IND (failure)
                // is generated for this event, when we really want the
                // FB acquisition to start from scratch with timing_validity=0.
                // If the subsequent process fails, then an indication will be
                // posted to L3.
                break;
              }

              // Send reporting message with a faillure indication.
              l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);

              // Remove cell from list.
              l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
              l1a_l1s_com.nsync.current_list_size    -= 1;

              if ((l1a_l1s_com.nsync.current_list_size != 0) && (neigh_id == l1a_l1s_com.nsync.first_in_list))
              {
                while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
                {
                  l1a_l1s_com.nsync.first_in_list++;
                  if (l1a_l1s_com.nsync.first_in_list==NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
                }
              }
              // Step in state machine.
              *state = SELECT_BEST_NSYNC;

              // Must break here to make sure the scheduler
              // runs immediately, else the next synchro attempt is not
              // started and we get a lock-up condition.
              break;
            }
          #if ((REL99 == 1) && (FF_RTD == 1)) // RTD feature
            else
            {  // we have to restore timing information which are destroyed after fb search failure
             if (l1a_l1s_com.nsync.list[neigh_id].timing_validity == 3)
             {
              // restore timing and frame information
              l1a_l1s_com.nsync.list[neigh_id].fn_offset    = l1a_l1s_com.nsync.list[neigh_id].fn_offset_mem;
              l1a_l1s_com.nsync.list[neigh_id].time_alignmt = l1a_l1s_com.nsync.list[neigh_id].time_alignmt_mem;
              // apply timing correction
              l1a_correct_timing (neigh_id,l1a_l1s_com.nsync.list[neigh_id].time_alignmt,l1a_l1s_com.nsync.list[neigh_id].fn_offset);
             }
            }
          #endif
          } // End FB failed

          // End of process.
          return;

        } // End L1C_FB_INFO

        else
        if(SignalCode == L1C_SB_INFO)
        // Synchro Burst acquisition attempt result.
        //------------------------------------------
        {
          BOOL         sb_found    = ((T_L1C_SB_INFO *)(msg->SigP))->sb_flag;
          UWORD8       neigh_id    = ((T_L1C_SB_INFO *)(msg->SigP))->neigh_id;
          UWORD16 neigh_radio_freq = ((T_L1C_SB_INFO *)(msg->SigP))->radio_freq;

          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // The monitoring tasks must be stopped
          // And the reporting message with a failure indication must be sent back
          {

            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);

            #if (L1_EOTD==1)

              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                // Stop Eotd session.
                l1a_l1s_com.nsync.eotd_meas_session = FALSE;

                // Set mode DEDIC
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;
              }
            #endif

            // This process must be reset
            *state = STOP_NSYNC;
            break;
          }

          if (sb_found == TRUE)
          // SB detection is a success...
          //-----------------------------
          {
            // Read Results.
            UWORD32  *fn_offset_ptr    = &(((T_L1C_SB_INFO *)(msg->SigP))->fn_offset);
            UWORD32  *time_alignmt_ptr = &(((T_L1C_SB_INFO *)(msg->SigP))->time_alignmt);

            // Correct "fn_offset" and "time_alignmt" to report the true
            // Serving/Neighbor time difference.
            //  1) Shift 20 bit since result is from a SB detection.
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
            l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

            #if (L1_EOTD==1)
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                // compute EOTD data
                l1a_compute_Eotd_data(&first_scell,neigh_id, SignalCode, msg);

                // Set mode DEDIC
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;

                #if (L1_EOTD_QBIT_ACC   ==1)
                if (neigh_id != 12)
                {
                  l1a_compensate_sync_ind((T_MPHC_NCELL_SYNC_IND *)(msg->SigP));
                }
                if( (neigh_id == 12) && (last_scell== TRUE) )
                {
                  l1a_l1s_com.nsync.serv_fn_offset    = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset;
                  l1a_l1s_com.nsync.serv_time_alignmt = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt;
                }
                #endif
              }
            #endif

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
          }

          else
          // SB detection failed.
          {
            // Fix BUG2842 & BUG2864. In packet transfer SB aquisition task can be delayed
            // by a higer prioritary task. Two following attempts are re-scheduled.
            // First one can fail and we must not init process neither report any
            // failure indication to upper layers.
            if (l1a_l1s_com.nsync.list[neigh_id].sb26_attempt == 2)
            { // sb26_attempt = 2 : first attempt
              // sb26_attempt = 3 : second attempt
               return;
            }
            #if (L1_EOTD ==1)
              // Set mode DEDIC
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;
                // Forward the result msg to L3.
                l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
              }
              else
            #endif

              // Send reporting message with a faillure indication.
              l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
          }

          // Remove cell from list.
          l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
          l1a_l1s_com.nsync.current_list_size    -= 1;

          #if (L1_EOTD ==1)
            // Stop EOTD monitoring after last Serving cell result
            //====================================================
            if (last_scell == TRUE)
          {
             l1a_l1s_com.nsync.eotd_meas_session = FALSE;
             *state = STOP_NSYNC;
          }
          else
          {
          #endif
            if ((l1a_l1s_com.nsync.current_list_size != 0) && (neigh_id == l1a_l1s_com.nsync.first_in_list))
            {
               while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
               {
                  l1a_l1s_com.nsync.first_in_list++;
                  if (l1a_l1s_com.nsync.first_in_list==NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
               }
            }
          // Step in state machine.
          *state = SELECT_BEST_NSYNC;
          #if (L1_EOTD ==1)
          }
          #endif

        } // End L1C_SB_INFO

        else
        if(SignalCode == L1C_SBCONF_INFO)
        // Synchro Burst confirmation attempt result.
        //-------------------------------------------
        {
          UWORD8  neigh_id        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->neigh_id;
          BOOL    sb_found        = ((T_L1C_SBCONF_INFO *)(msg->SigP))->sb_flag;
          UWORD16 neigh_radio_freq = ((T_L1C_SBCONF_INFO *)(msg->SigP))->radio_freq;

          // Check if this neighbor wasn't removed from the list
          // (it's possible if MPHC_STOP_NCELL_SYNC_REQ message has been received
          // in the same frame than this L1s message)
          if (neigh_radio_freq != l1a_l1s_com.nsync.list[neigh_id].radio_freq)
          {
             //REM: the message is not sent to L3
             return;// Stay in current state.
          }

          if (l1a_l1s_com.dedic_set.handover_fail_mode)
          // The current state is the dedicated mode during an handover fail
          // The monitoring tasks must be stopped
          // And the reporting message with a failure indication must be sent back
          {

            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);

            #if (L1_EOTD==1)

              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                // Stop Eotd session.
                l1a_l1s_com.nsync.eotd_meas_session = FALSE;

                // Set mode DEDIC
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;
              }
            #endif

            // This process must be reset
            *state = STOP_NSYNC;
            break;
          }

          if(sb_found == TRUE)
          // SB detection is a success.
          {
            UWORD32  *fn_offset_ptr    = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->fn_offset);
            UWORD32  *time_alignmt_ptr = &(((T_L1C_SBCONF_INFO *)(msg->SigP))->time_alignmt);

            // Correct "fn_offset" and "time_alignmt" to report the true
            // Serving/Neighbor time difference.
            //  1) Shift 20 bit since result is from a SB detection.
            //  2) Add the serving cell timeslot number to the Serving/Neighbor time difference.
            l1a_add_time_for_nb(time_alignmt_ptr, fn_offset_ptr);
            l1a_add_timeslot(time_alignmt_ptr, fn_offset_ptr, l1a_l1s_com.dl_tn);

            #if (L1_EOTD==1)
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                // compute EOTD data
                l1a_compute_Eotd_data(&first_scell,neigh_id,SignalCode,msg);

                // Set mode DEDIC
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;

                #if (L1_EOTD_QBIT_ACC ==1)
                  if (neigh_id != 12)
                  {
                     l1a_compensate_sync_ind((T_MPHC_NCELL_SYNC_IND *)(msg->SigP));
                  }
                  if( (neigh_id == 12) && (last_scell== TRUE) )
                  {
                     l1a_l1s_com.nsync.serv_fn_offset    = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->fn_offset;
                     l1a_l1s_com.nsync.serv_time_alignmt = ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->time_alignmt;
                  }
                #endif
              }
            #endif

            // Forward the result msg to L3.
            l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
          }

          else
          // SB detection failed.
          {
            #if (L1_EOTD ==1)
              // Set mode DEDIC
              if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              {
                ((T_MPHC_NCELL_SYNC_IND *)(msg->SigP))->mode = 1;
                // Forward the result msg to L3.
                l1a_send_result(MPHC_NCELL_SYNC_IND, msg, RRM1_QUEUE);
              }
              else
            #endif

            // Send reporting message with a faillure indication.
            l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, neigh_id);
          }

           // Disable a neigh sync. reading.
          l1a_l1s_com.nsync.list[neigh_id].status = NSYNC_FREE;
          l1a_l1s_com.nsync.current_list_size    -= 1;

           #if (L1_EOTD ==1)
          // reset first_scell flag
          first_scell = FALSE;

            // Stop EOTD monitoring after last Serving cell result
            //====================================================
          if (last_scell == TRUE)
          {
            l1a_l1s_com.nsync.eotd_meas_session = FALSE;
            *state = STOP_NSYNC;
          }
          else
          {
          #endif
            if ((l1a_l1s_com.nsync.current_list_size != 0) && (neigh_id == l1a_l1s_com.nsync.first_in_list))
            {
               while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
               {
                 l1a_l1s_com.nsync.first_in_list++;
                 if(l1a_l1s_com.nsync.first_in_list==NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
               }
            }
          // Step in state machine.
          *state = SELECT_BEST_NSYNC;
          #if (L1_EOTD ==1)
          }
          #endif


        } // End L1C_SBCONF_INFO

        else
        if((SignalCode == MPHC_NCELL_FB_SB_READ) || (SignalCode == MPHC_NCELL_SB_READ) ||
           (SignalCode == MPHC_NCELL_SYNC_REQ) || (SignalCode == MPHC_NCELL_LIST_SYNC_REQ))
        // New Request to READ the FB/SB or SB from a given carrier.
        // Request to READ the FB/SB or SB of 1 to 12 carriers.
        //--------------------------------------------------------
        {
          #if (L1_EOTD ==1)
            // during EOTD update of list is forbidden.....
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
              // Stay in current state.
              return;
            else
          #endif
          // Step in state machine.
          *state = NSYNC_CONFIG;
        } // End new NCELL REQ

        else
        if(SignalCode == MPHC_STOP_NCELL_SYNC_REQ)
        // Request to STOP neighbour cell activity for certain carriers.
        //--------------------------------------------------------------
        {
          UWORD8  i,j;
          UWORD8  array_size;

          array_size = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array_size;

         #if (L1_EOTD ==1)
            // Only stop for ALL neighbours in list are accepted.
            if (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
            {
              if ( (array_size != l1a_l1s_com.nsync.current_list_size) &&
                   (array_size != NBR_NEIGHBOURS) )
                // Stay in current state.
                return;
            }
            // Stop Eotd session.
            l1a_l1s_com.nsync.eotd_meas_session = FALSE;
          #endif

          // Disable neighbor sync. tasks.
          l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

          if(array_size != NBR_NEIGHBOURS)
          {
            // Stop some of the Neighb. synchro.
            for(i=0;i<array_size;i++)
            {
              UWORD16  radio_freq = ((T_MPHC_STOP_NCELL_SYNC_REQ *)(msg->SigP))->radio_freq_array[i];

              // Search for same value within L1 structure.
              j=0;
              while(!((radio_freq == l1a_l1s_com.nsync.list[j].radio_freq) &&
                      (l1a_l1s_com.nsync.list[j].status != NSYNC_FREE)) &&
                    (j < NBR_NEIGHBOURS))
              {
                j++;
              }

              // If found, reset L1 structure for this carrier.
              if(j<NBR_NEIGHBOURS)
              {
                l1a_l1s_com.nsync.list[j].status = NSYNC_FREE;
                l1a_l1s_com.nsync.current_list_size --;
				if (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.active_fb_id].radio_freq == radio_freq)
                  l1a_l1s_com.l1s_en_task[FB26]  = TASK_DISABLED;
              }
            }
          }
          else
          {
            // Stop all the Neighb. BCCH reading.
            l1a_l1s_com.nsync.current_list_size = 0;

            // clear also last location...
            for(i=0;i<NBR_NEIGHBOURS+1;i++)
              l1a_l1s_com.nsync.list[i].status = NSYNC_FREE;
          }

          // Send confirmation message to L3.
          l1a_send_confirmation(MPHC_STOP_NCELL_SYNC_CON,RRM1_QUEUE);

          // All neigh synchro have been removed.
          if(l1a_l1s_com.nsync.current_list_size == 0)
          {
            // Tasks already disabled.
            // This process must be reset.
            *state = STOP_NSYNC;
          }
          else
          {
            // Check if first in list was removed from the list. Go to next first in list
            while (l1a_l1s_com.nsync.list[l1a_l1s_com.nsync.first_in_list].status == NSYNC_FREE)
            {
              l1a_l1s_com.nsync.first_in_list++;
              if (l1a_l1s_com.nsync.first_in_list==NBR_NEIGHBOURS) l1a_l1s_com.nsync.first_in_list = 0;
            }

            // Set semaphores for all neighbor relative task before re-enebling NSYNC task.
            l1a_l1s_com.task_param[NSYNC]  = SEMAPHORE_SET;
            l1a_l1s_com.task_param[FB51]   = SEMAPHORE_SET;
            l1a_l1s_com.task_param[FB26]   = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SB51]   = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SB26]   = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;
            l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;

            // Enable neighbour sync task.
            l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;

            // Step in state machine.
            *state = SELECT_BEST_NSYNC;
          }
        } // End MPHC_STOP_NCELL_SYNC_REQ

        else
        #if (L1_GPRS)
          // a new synchronisation was performed
          if ( (SignalCode == L1C_DEDIC_DONE)
            || ((SignalCode == L1P_TRANSFER_DONE) && (((T_L1P_TRANSFER_DONE *) (msg->SigP))->tn_difference!=0))
            || (SignalCode == L1P_ALLOC_EXHAUST_DONE)
            || (SignalCode == L1P_REPEAT_ALLOC_DONE)
            || ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *) (msg->SigP))->tn_difference!=0) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all==0)))
        #else  //L1_GPRS
            // test in mode D26: L1C_DEDIC_DONE : // New channel activated.
            if (SignalCode == L1C_DEDIC_DONE)
        #endif //L1_GPRS

              // New channel activated or new synchronisation was performed
              //-----------------------------------------------------------
            {
              UWORD32 time_alignmt;
              UWORD32 fn_offset;
              UWORD8  i;


              if ( (SignalCode == L1C_DEDIC_DONE) &&
                ((l1a.confirm_SignalCode == MPHC_SYNC_HO_CON) ||
                (l1a.confirm_SignalCode == MPHC_PRE_SYNC_HO_CON) ||
                (l1a.confirm_SignalCode == MPHC_ASYNC_HO_CON) ||
                (l1a.confirm_SignalCode == MPHC_HANDOVER_FAIL_CON)) )
              {
                l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[FB51]  = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[FB26]  = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[SB51]  = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[SB26]  = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[SBCNF26]  = TASK_DISABLED;
                l1a_l1s_com.l1s_en_task[SBCNF51]  = TASK_DISABLED;

                // In case of a handover, it is hard to update neighbor timings with timing changes.
                // Therefore all pending request are restarted as FB acquisition.
                // If an EOTD session was running, the 300 ms duration constraint of the session can't
                // be achieved. L1 returns a fail to L3 for the session.

              #if (L1_EOTD ==1)
                for(i=NBR_NEIGHBOURS+1;i>0;i--)
              #else
                for(i=0;i<NBR_NEIGHBOURS;i++)
              #endif
                {
                  if ( (l1a_l1s_com.nsync.list[i-1].status == NSYNC_PENDING)||
                     (l1a_l1s_com.nsync.list[i-1].status == NSYNC_WAIT) )
                  {
                    l1a_l1s_com.nsync.list[i-1].timing_validity = 0;
                    l1a_l1s_com.nsync.list[i-1].time_alignmt = 0;
                    l1a_l1s_com.nsync.list[i-1].fn_offset    = 0;
                    // force WAIT state
                    l1a_l1s_com.nsync.list[i-1].status = NSYNC_WAIT;

                  #if (L1_EOTD ==1)
                    if ( l1a_l1s_com.nsync.eotd_meas_session == TRUE )
                    {
                      //If an EOTD session is pending, return a fail for remaining requests
                      l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, i-1);
                    }
                  #endif
                  } // if
                } // for

              #if (L1_EOTD ==1)
                if ( l1a_l1s_com.nsync.eotd_meas_session == TRUE )
                {
                  // If the code goes there, an EOTD session is aborting.
                  // return a fail for last serving cell and reset L1A state machine
                  l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, NBR_NEIGHBOURS);
                  l1a_l1s_com.nsync.eotd_meas_session = FALSE;
                  *state = STOP_NSYNC;
                  break;
                }
                else
              #endif
                {
                  // Step in state machine.
                  *state = SELECT_BEST_NSYNC;

                  // Enable neighbour sync task.
                  l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;
                } // if
              } // if

            else
            {
              // Channel changes : Restart all neighbors
              //-----------------------------------------
              // propagate new dl_tn to all neigbours remaining in EOTD_list !!!
              // Reset process. and then restart all tasks.
              l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[FB51]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[FB26]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SB51]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SB26]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SBCNF26]  = TASK_DISABLED;
              l1a_l1s_com.l1s_en_task[SBCNF51]  = TASK_DISABLED;

#if (L1_EOTD ==1)
              for(i=0;i<NBR_NEIGHBOURS+1;i++)
#else
                for(i=0;i<NBR_NEIGHBOURS;i++)
#endif
                {
                  if ( (l1a_l1s_com.nsync.list[i].status == NSYNC_PENDING)||
                    (l1a_l1s_com.nsync.list[i].status == NSYNC_WAIT) )
                  {
                    // reset SB acquisitions : need to restart from FB !!!!
                  #if ((REL99 == 1) && ((FF_BHO == 1) || (FF_RTD == 1)))
                    if (l1a_l1s_com.nsync.list[i].timing_validity == SB_ACQUISITION_PHASE)
                  #else
                    if( l1a_l1s_com.nsync.list[i].timing_validity == 3 )   // SB acquisition
                  #endif
                    {
                      l1a_l1s_com.nsync.list[i].timing_validity = 0;
                    #if ((REL99 == 1) && (FF_BHO == 1))
                      l1a_l1s_com.nsync.list[i].status = NSYNC_WAIT ;
                     #endif
                    }

                    // FB or SB Confirmations : update timing info
                    if ((l1a_l1s_com.nsync.list[i].timing_validity == 2) || // SB confirmation
                      (l1a_l1s_com.nsync.list[i].timing_validity == 1))   // FB confirmation
                    {
                      // update the SBCNF26 parameters
#if (L1_EOTD==1)
                      if ( (i==12) && (l1a_l1s_com.nsync.eotd_meas_session == TRUE))
                      {
                        // Sub the serving cell timeslot number to the Neigh./Serving timing
                        // difference to format it for L1S use.
#if (L1_EOTD_QBIT_ACC== 1)
                        time_alignmt = l1a_l1s_com.nsync.serv_time_alignmt;
                        fn_offset    = l1a_l1s_com.nsync.serv_fn_offset;
#else
                        time_alignmt = l1a_l1s_com.nsync.list[i].time_alignmt_mem;
                        fn_offset    = l1a_l1s_com.nsync.list[i].fn_offset_mem;
#endif
                        l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].fn_offset_mem   = 0;
                        l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].time_alignmt_mem= 0;
                      }
                      else
#endif
                      {
                        time_alignmt = l1a_l1s_com.nsync.list[i].time_alignmt_mem;
                        fn_offset    = l1a_l1s_com.nsync.list[i].fn_offset_mem;
                      }

                      // Sub the serving cell timeslot number to the Neigh./Serving timing
                      // difference to format it for L1S use.
                      l1a_sub_timeslot(&time_alignmt, &fn_offset, l1a_l1s_com.dl_tn);
                      l1a_sub_time_for_nb(&time_alignmt, &fn_offset);

                      l1a_l1s_com.nsync.list[i].time_alignmt = time_alignmt;
                      l1a_l1s_com.nsync.list[i].fn_offset    = fn_offset;
                    }

                    // force WAIT state
                    l1a_l1s_com.nsync.list[i].status = NSYNC_WAIT;
                  }
                }
                //------------------------------------------------------------
                // We restart the FB/SB and SBCONF detection from scratch !!!
                //------------------------------------------------------------
                // Set semaphores for all neighbor relative task before re-enebling NSYNC task.
                l1a_l1s_com.task_param[NSYNC]  = SEMAPHORE_SET;
                l1a_l1s_com.task_param[FB51]   = SEMAPHORE_SET;
                l1a_l1s_com.task_param[FB26]   = SEMAPHORE_SET;
                l1a_l1s_com.task_param[SB51]   = SEMAPHORE_SET;
                l1a_l1s_com.task_param[SB26]   = SEMAPHORE_SET;
                l1a_l1s_com.task_param[SBCNF51] = SEMAPHORE_SET;
                l1a_l1s_com.task_param[SBCNF26] = SEMAPHORE_SET;

                // For EOTD : if the 1st serving cell SB is pending
                // or if last serving cell SB is pending
                // (when the channel change occured),we must re-start from 1st SB
                // of serving cell.
                //
                // during WAIT_NSYNC_RESULT
                // =========================
                //    first_scell  eotd_started last_scell
                //      T           T         F       waiting for 1st SB Serving cell
                //      F           T         F       waiting for SBs Neighbor cells
                //      F           F         T       waiting for Last SB Serving cell
                //      F           F         F       no EOTD
                //
#if (L1_EOTD == 1)
                if ( (first_scell == TRUE) || (last_scell == TRUE))
                {
                  // force PENDING state  for SERVING CELL
                  l1a_l1s_com.nsync.list[NBR_NEIGHBOURS].status = NSYNC_PENDING;

                  // Step in state machine.
                  *state = WAIT_NSYNC_RESULT;

                  // Enable neighbour sync task.
                  l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;


                  // Must be a return there otherwise there is a lockup on
                  // handover/channel changes test condition (need to free
                  // this message...)
                  return;
                }
#endif
                // Step in state machine.
                *state = SELECT_BEST_NSYNC;

                // Enable neighbour sync task.
                l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;
         }  // end Else
       }  // End CHANNEL CHANGE or HANDOVERS

       else
       #if (L1_GPRS)
         // End of packet transfer mode if TBF downlink and uplink have been released
          if((SignalCode == MPHC_STOP_DEDICATED_REQ) ||
             ((SignalCode == L1P_TBF_RELEASED) && (((T_L1P_TBF_RELEASED *)(msg->SigP))->released_all)))
       #else
          if(SignalCode == MPHC_STOP_DEDICATED_REQ)
       #endif
       // Reset messages.
       //----------------
       {
         #if (L1_EOTD==1)
           // Stop Eotd session.
           l1a_l1s_com.nsync.eotd_meas_session = FALSE;
         #endif

          // Step in state machine.
          *state = STOP_NSYNC;
        }
#if ((REL99 == 1) && (FF_BHO == 1))
        else
        if( ((SignalCode == MPHC_SYNC_HO_REQ) ||
             (SignalCode == MPHC_PRE_SYNC_HO_REQ) ||
             (SignalCode == MPHC_PSEUDO_SYNC_HO_REQ) ||
             (SignalCode == MPHC_ASYNC_HO_REQ)
            )
         && (l1a_l1s_com.dedic_set.handover_type == BLIND_HANDOVER) )
         {
           UWORD8 i;

           l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[FB51]  = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[FB26]  = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[SB51]  = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[SB26]  = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[SBCNF26]  = TASK_DISABLED;
           l1a_l1s_com.l1s_en_task[SBCNF51]  = TASK_DISABLED;

           // In case of a handover, it is hard to update neighbor timings with timing changes.
           // Therefore all pending request are restarted as FB acquisition.
           // If an EOTD session was running, the 300 ms duration constraint of the session can't
           // be achieved. L1 returns a fail to L3 for the session.
         #if (L1_EOTD ==1)
           for(i=NBR_NEIGHBOURS+1;i>0;i--)
         #else
           for(i=0;i<NBR_NEIGHBOURS;i++)
         #endif
           {
             if ( (l1a_l1s_com.nsync.list[i-1].status == NSYNC_PENDING)||
                  (l1a_l1s_com.nsync.list[i-1].status == NSYNC_WAIT) )
             {
               l1a_l1s_com.nsync.list[i-1].timing_validity = 0;
               l1a_l1s_com.nsync.list[i-1].time_alignmt = 0;
               l1a_l1s_com.nsync.list[i-1].fn_offset    = 0;
               // force WAIT state
               l1a_l1s_com.nsync.list[i-1].status = NSYNC_WAIT;

             #if (L1_EOTD ==1)
               if ( l1a_l1s_com.nsync.eotd_meas_session == TRUE )
               {
                 //If an EOTD session is pending, return a fail for remaining requests
                 l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, i-1);
               }
             #endif
             } // if
           } // for

         #if (L1_EOTD ==1)
           if ( l1a_l1s_com.nsync.eotd_meas_session == TRUE )
           {
             // If the code goes there, an EOTD session is aborting.
             // return a fail for last serving cell and reset L1A state machine
             l1a_report_failling_ncell_sync(MPHC_NCELL_SYNC_IND, NBR_NEIGHBOURS);
             l1a_l1s_com.nsync.eotd_meas_session = FALSE;
             *state = STOP_NSYNC;
             break;
           }
           else
         #endif
           {
             // Step in state machine.
             *state = SELECT_BEST_NSYNC;

             // Enable neighbour sync task.
             l1a_l1s_com.l1s_en_task[NSYNC] = TASK_ENABLED;
           } // if
         }
#endif // #if ((REL99 == 1) && (FF_BHO == 1))
        // No action in this machine for other messages.
        else
        {
          // End of process.
          return;
        }
      }
      break;

      case STOP_NSYNC:
      {
        UWORD8 i;

        // Reset process.
        // WARNING : NSYNC task and nsync_list are COMMON to dedic and Idle....
        l1a_l1s_com.l1s_en_task[FB51]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[FB26]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[SB51]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[SB26]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[SBCNF26]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[SBCNF51]  = TASK_DISABLED;
        l1a_l1s_com.l1s_en_task[NSYNC] = TASK_DISABLED;

        // Disable all neigh sync. reading.
        l1a_l1s_com.nsync.current_list_size   = 0;


#if (L1_EOTD==1)
        // reset EOTD flags
        first_scell  =FALSE;
        last_scell   =FALSE;
        eotd_started =FALSE;
        l1a_l1s_com.nsync.eotd_toa_phase = 0;
        l1a_l1s_com.nsync.eotd_meas_session = FALSE;
        l1a_l1s_com.nsync.eotd_cache_toa_tracking = 0;
        l1a_l1s_com.nsync.eotd_toa_tracking = 0;
        for (i=0; i<NBR_NEIGHBOURS+1; i++) l1a_l1s_com.nsync.list[i].status = NSYNC_FREE;
#else
        for (i=0; i<NBR_NEIGHBOURS; i++) l1a_l1s_com.nsync.list[i].status = NSYNC_FREE;
#endif
        // Step in state machine.
        *state = WAIT_INIT;

        // End of process
        end_process = 1;

      }
      break;

    } // end of "switch".
  } // end of "while"
} // end of procedure.

#endif

/*-------------------------------------------------------*/
/* l1a_dedic_ba_list_meas_process()                      */
/*-------------------------------------------------------*/
/* Description : This state machine handles neigbor cell */
/* measurement process in DEDICATED mode with BA list.   */
/*                                                       */
/* Starting messages:        L1C_DEDIC_DONE              */
/* ------------------                                    */
/*  L1 starts then the periodic BA list receive level    */
/*  monitoring.                                          */
/*                                                       */
/* Subsequent messages:      MPHC_UPDATE_BA_LIST         */
/* --------------------                                  */
/*  L1 changes the BA list and starts the periodic BA    */
/*  list receive level monitoring with this new list.    */
/*                                                       */
/* Result messages (input):  L1C_MEAS_DONE               */
/* ------------------------                              */
/*  This is the periodic reporting message from L1s.     */
/*                                                       */
/* Result messages (output): MPHC_MEAS_REPORT            */
/* -------------------------                             */
/*  This is the periodic reporting message to L3.        */
/*                                                       */
/* Reset messages (input):   MPHC_STOP_DEDICATED_REQ     */
/* -----------------------                               */
/*  BA list neigbor cell measurement process in DEDICATED*/
/*  is stopped by this message.                          */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_dedic_ba_list_meas_process(xSignalHeaderRec *msg)
{
  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1,
    WAIT_RESULT = 3

  };

          UWORD8  *state      = &l1a.state[D_NMEAS];
          UWORD32  SignalCode = msg->SignalCode;
  static  BOOL     meas_valid;

  BOOL end_process = 0;
  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
        // step in state machine.
        *state = WAIT_INIT;

        // Reset D_NMEAS process.
        l1a_l1s_com.l1s_en_meas &= D_BAMS_MEAS_MASK; // Reset D_BAMS Measurement enable flag.
      }
      break;

      case WAIT_INIT:
      {
#if (OP_SAP == 0)
        if (SignalCode == MPHC_UPDATE_BA_LIST)
#else
        if (SignalCode == MPHC_UPDATE_BA_LIST_REQ)
#endif /* if (OP_SAP == 0) */
        // One reporting period has been completed.
        //-----------------------------------------
        {
          UWORD16 i;

          // Set parameter synchro semaphore for D_BAMS task.
          l1a_l1s_com.meas_param |= D_BAMS_MEAS;

          // Reset the BA list structure.
          l1a_reset_ba_list();

          // Next measurement report must indicate INVALID.
          meas_valid = FALSE;

          // Enable Dedicated mode BA list measurement task.
          l1a.l1a_en_meas[D_NMEAS] |= D_BAMS_MEAS;

          // Reset present flag to avoid to mix 2 updates in case of
          // an update already pending within "l1a_l1s_com.ba_list.new_list".
          // This case can occur after a Handover and both previous and new
          // serving cells are requesting an update.
          l1a_l1s_com.ba_list.new_list_present = FALSE;

          // Download new list within L1A_L1S_COM structure.
#if (OP_SAP == 0)
          l1a_l1s_com.ba_list.new_list = *((T_MPHC_UPDATE_BA_LIST *)(msg->SigP));
#else
          l1a_l1s_com.ba_list.new_list = *((T_MPHC_UPDATE_BA_LIST_REQ *)(msg->SigP));
#endif /* if (OP_SAP == 0) */

            // Download new list.
          for(i=0;i<l1a_l1s_com.ba_list.new_list.num_of_chans;i++)
          {
#if (OP_SAP == 0)
            l1a_l1s_com.ba_list.A[i].radio_freq = l1a_l1s_com.ba_list.new_list.chan_list.A[i];
#else
            l1a_l1s_com.ba_list.A[i].radio_freq = l1a_l1s_com.ba_list.new_list.chan_list.BA[i];
#endif  /* if (OP_SAP == 0) */

          }

          l1a_l1s_com.ba_list.ba_id       = l1a_l1s_com.ba_list.new_list.ba_id;
          l1a_l1s_com.ba_list.nbr_carrier = l1a_l1s_com.ba_list.new_list.num_of_chans;

          // Set present flag only when the list has been downloaded.
          l1a_l1s_com.ba_list.new_list_present = TRUE;

          // step in state machine.
          *state = WAIT_RESULT;
        }
        else

        if(SignalCode == L1C_DEDIC_DONE)
        // We enter DEDICATED mode.
        //-------------------------
        {
#if (CODE_VERSION == NOT_SIMULATION)
          if (l1a_l1s_com.ba_list.nbr_carrier == 0)
             return;
#endif

          // Set parameter synchro semaphore for D_BAMS task.
          l1a_l1s_com.meas_param |= D_BAMS_MEAS;

          // Reset the BA list structure.
          l1a_reset_ba_list();

          // Next measurement report must indicate INVALID.
          meas_valid = FALSE;

          // Enable Dedicated mode BA list measurement task.
          l1a.l1a_en_meas[D_NMEAS] |= D_BAMS_MEAS;

          // step in state machine.
          *state = WAIT_RESULT;
        }

        // End of process.
        end_process = 1;
      }
      break;

      case WAIT_RESULT:
      {
        if(SignalCode == L1C_MEAS_DONE)
        // One reporting period has been completed.
        //-----------------------------------------
        {
          // Fill "meas_valid" parameter.
          ((T_MPHC_MEAS_REPORT*)(msg->SigP))->meas_valid = meas_valid;

          // Forward result message to L3.
          l1a_send_result(MPHC_MEAS_REPORT, msg, RRM1_QUEUE);

          // Next measurement report must indicate VALID.
          meas_valid = TRUE;

          // End of process.
          return;
        }

        if(SignalCode == MPHC_UPDATE_BA_LIST)
        // One reporting period has been completed.
        //-----------------------------------------
        {
          // Reset present flag to avoid to mix 2 updates in case of
          // an update already pending within "l1a_l1s_com.ba_list.new_list".
          // This case can occur after a Handover and both previous and new
          // serving cells are requesting an update.
          l1a_l1s_com.ba_list.new_list_present = FALSE;

          // Download new list within L1A_L1S_COM structure.
          l1a_l1s_com.ba_list.new_list = *((T_MPHC_UPDATE_BA_LIST *)(msg->SigP));

          // Set present flag only when the list has been downloaded.
          l1a_l1s_com.ba_list.new_list_present = TRUE;

          // End of process.
          return;
        }

        else
      #if ((REL99 == 1) && (FF_BHO == 1))
        if ((SignalCode == MPHC_STOP_DEDICATED_REQ) ||
            (((SignalCode == MPHC_SYNC_HO_REQ) ||
              (SignalCode == MPHC_PRE_SYNC_HO_REQ) ||
              (SignalCode == MPHC_PSEUDO_SYNC_HO_REQ) ||
              (SignalCode == MPHC_ASYNC_HO_REQ)
             ) &&
             (l1a_l1s_com.dedic_set.handover_type == BLIND_HANDOVER)
            )
           )
      #else
        if(SignalCode == MPHC_STOP_DEDICATED_REQ)
      #endif // #if ((REL99 == 1) && (FF_BHO == 1))
        // We exit DEDICATED mode.
        {
          // Stop D_BAMS_MEAS task.
          l1a_l1s_com.l1s_en_meas &= D_BAMS_MEAS_MASK;

          // When leaving dedicated mode we must clear the NEW BA
          // present flag to keep L1 and L1 in step and to avoid
          // taking it into account on the next Dedicated mode
          // session.

          // Check inf any NEW BA available, if so download it...
          if(l1a_l1s_com.ba_list.new_list_present == TRUE)
          {
            UWORD16 i;

            // Download new list.
            for(i=0;i<l1a_l1s_com.ba_list.new_list.num_of_chans;i++)
            {
              l1a_l1s_com.ba_list.A[i].radio_freq = l1a_l1s_com.ba_list.new_list.chan_list.A[i];
            }

            l1a_l1s_com.ba_list.ba_id       = l1a_l1s_com.ba_list.new_list.ba_id;
            l1a_l1s_com.ba_list.nbr_carrier = l1a_l1s_com.ba_list.new_list.num_of_chans;

            l1a_l1s_com.ba_list.new_list_present = 0;
          }

          // Step in state machine.
          *state = RESET;
        }

        else
        if(SignalCode == L1C_DEDIC_DONE)
        // Reset messages.
        //----------------
        {
          // Step in state machine.
          *state = RESET;
        }

        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          end_process = 1;
        }
      }
      break;
    } // end of "switch".
  } // end of "while"
} // end of procedure.

/*-------------------------------------------------------*/
/* l1a_freq_band_configuration ()                        */
/*-------------------------------------------------------*/
/* Description : This state machine handles the frequency*/
/*band configuration: E_GSM900, GSM900, DCS1800, PCS1900,*/
/*DUAL, DUALEXT, DUALEXT_PCS1900...                      */
/*                                                       */
/* Starting messages:        MPHC_INIT_L1_REQ            */
/*                                                       */
/* Result messages (input):  none                        */
/* Result messages (output): MPHC_INIT_L1_CON           */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
void l1a_freq_band_configuration(xSignalHeaderRec *msg)
{
  UWORD32  SignalCode = msg->SignalCode;
  UWORD8   *state      = &l1a.state[INIT_L1];

  enum states
  {
    RESET       = 0,
    WAIT_INIT   = 1
  };

  BOOL end_process = 0;

  while(!end_process)
  {
    switch(*state)
    {
      case RESET:
      {
         // Step in state machine.
         *state = WAIT_INIT;
      }
      break;

      case WAIT_INIT:
      {

        if (SignalCode == MPHC_INIT_L1_REQ)
        // Request from L3 which defined the standard, GSM, E-GSM, DCS1800,  DUALEXT...
        //-------------------------------------------------------------------------------
        {
#if (L1_FF_MULTIBAND == 0)
          l1_config.std.id = ((T_MPHC_INIT_L1_REQ *)(msg->SigP))->radio_band_config;
#else
          UWORD8 physical_band_id;
static    UWORD8 rf_band_id_to_l23_band_id[]={PGSM900, GSM850, DCS1800, PCS1900};
#endif


          Cust_init_std();
          l1_tpu_init_light();
#if (L1_FF_MULTIBAND == 0)

          

          #if (TRACE_TYPE == 5)
            trace_sim_freq_band_configuration(l1_config.std.id);
          #endif

#endif /*if (L1_FF_MULTIBAND == 0)*/

          // Forward result message to L3.
#if (L1_FF_MULTIBAND == 0)

          l1a_send_confirmation(MPHC_INIT_L1_CON,RRM1_QUEUE);

#else // L1_FF_MULTIBAND  1 below

          msg = os_alloc_sig(sizeof(T_MPHC_INIT_L1_CON));
          DEBUGMSG(status, NU_ALLOC_ERR)
          msg->SignalCode = MPHC_INIT_L1_CON;
          for (physical_band_id = 0; physical_band_id < RF_NB_SUPPORTED_BANDS; physical_band_id ++)
          {
            ((T_MPHC_INIT_L1_CON *)(msg->SigP))->multiband_power_class[physical_band_id].radio_band
                =
                rf_band_id_to_l23_band_id[multiband_rf[physical_band_id].gsm_band_identifier];
            ((T_MPHC_INIT_L1_CON *)(msg->SigP))->multiband_power_class[physical_band_id].power_class
                = multiband_rf[physical_band_id].power_class;
            l1a_l1s_com.powerclass[physical_band_id]=multiband_rf[physical_band_id].power_class;
          }
          for (physical_band_id=RF_NB_SUPPORTED_BANDS; physical_band_id<NB_MAX_GSM_BANDS; physical_band_id++)
          {
           ((T_MPHC_INIT_L1_CON *)(msg->SigP))->multiband_power_class[physical_band_id].radio_band = 0xFF;   
          }
          os_send_sig(msg, RRM1_QUEUE);

#endif // #if (L1_FF_MULTIBAND == 0) else 

          // This process must be reset.
          *state = RESET;
          return;
        }
        else
        // No action in this machine for other messages.
        //----------------------------------------------
        {
          end_process = 1;
        }
        break;
      }
    } // End of switch
  } // End of while
}

/*-------------------------------------------------------*/
/* l1a_checkmsg_compatibility ()                        */
/*-------------------------------------------------------*/
/* Description : This checks the message compatibility, If the */
/*received message is not compatible with current scenario, */
/*delay the message or ignore the message.*/
/*                                                       */
/*                                                       */
/* Result messages (input):  xSignalHeaderRec *msg    */
/* Result messages (output): none           */
/* Reset messages (input):   none                        */
/*                                                       */
/*-------------------------------------------------------*/
#if(L1_CHECK_COMPATIBLE == 1)
void l1a_checkmsg_compatibility    (xSignalHeaderRec *msg)
{
  UWORD32   SignalCode  = msg->SignalCode;



  /* stop_req is set to True, if ringer stop message is received from MMI */
  if((SignalCode == MMI_AAC_STOP_REQ) || (SignalCode == MMI_MP3_STOP_REQ))
  {
     l1a.stop_req = TRUE;
  }

  /* This disables vocoder if vocoder is enabled and set vcr_wait to TRUE */
  /* to trigger VCH_R msg when AAC/MP3 is stopped completely. */
  /* This is the case of inband tone where first VCH_R is recvieved and */
  /* then ringer is started  */
  if((SignalCode == MMI_AAC_START_REQ) || (SignalCode == MMI_MP3_START_REQ))
  {
     if (l1a.vocoder_state.enabled == TRUE) {

          l1a.vcr_wait   = TRUE;
	   l1a.vch_auto_disable = TRUE;
   	}
  }

  /* This is to trigger VCH_R message if  message is delayed earliar */
  /* and ringer is stopped completely */
  if ((l1a.stop_req == TRUE) && (l1a.vcr_wait==TRUE))
  {
     if ((SignalCode  == L1_MP3_DISABLE_CON) ||
	   (SignalCode  == L1_AAC_DISABLE_CON))
     {
         xSignalHeaderRec    *msg_vcr;
	  msg_vcr = os_alloc_sig(sizeof(T_MMI_TCH_VOCODER_CFG_REQ));
	  DEBUGMSG(status,NU_ALLOC_ERR)
	  msg_vcr->SignalCode = MMI_TCH_VOCODER_CFG_REQ;
	  ((T_MMI_TCH_VOCODER_CFG_REQ *) (msg_vcr->SigP))->vocoder_state = l1a.vcr_msg_param;
	  os_send_sig(msg_vcr,L1C1_QUEUE);
	  DEBUGMSG(status,NU_SEND_QUEUE_ERR)
         l1a.vcr_wait = FALSE;
         l1a.stop_req = FALSE;
	  {
            char str[25];
             sprintf(str,"TRIGGER VCH");
             rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
   	  }
      }
}


  /* vcr_wait is set to TRUE to delay the VCH_R if MP3/AAC is not stopped completey*/
  if(SignalCode == MMI_TCH_VOCODER_CFG_REQ)
   {
     l1a.vcr_msg_param = ((T_MMI_TCH_VOCODER_CFG_REQ *) (msg->SigP))->vocoder_state;

     //  MP3 ON or AAC ON
    if ((!((l1a.state[L1A_MP3_STATE] == 0) || (l1a.state[L1A_MP3_STATE] == 1))) ||
	  (!((l1a.state[L1A_AAC_STATE] == 0) || (l1a.state[L1A_AAC_STATE] == 1))))
    {
       l1a.vcr_wait   = TRUE;
	msg->SignalCode = NULL;
	{
             char str[25];
             sprintf(str,"VCH DELAY");
             rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
 	}
   }
 }
}
#endif


/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_SYNC.C
 *
 *        Filename l1audio_sync.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Include files...                 */
/************************************/
#include "l1_macro.h"
#include "l1_confg.h"

#if (AUDIO_TASK == 1)

  #include "l1_types.h"
  #include "sys_types.h"

  #if (CODE_VERSION == SIMULATION) && (AUDIO_SIMULATION)

    #include <stdlib.h>
    #include <string.h>

    #include "iq.h"             // Debug / Init hardware  ("eva3.lib")
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif

    #if (L1_MP3 == 1)
      #include "l1mp3_const.h"
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_const.h"
      #include "l1midi_defty.h"
    #endif

    #if (L1_AAC == 1)
      #include "l1aac_const.h"
      #include "l1aac_defty.h"
    #endif
    #include "l1_defty.h"
    #include "cust_os.h"
    #include "l1_msgty.h"
    #include "l1_varex.h"

    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "l1_ctl.h"

    #include "l1_time.h"
    #include "l1_scen.h"

    #if (L1_STEREOPATH == 1)
      #include "sys_dma.h"
      #include "abb.h"
      #if TESTMODE
        #include "l1tm_msgty.h"
      #endif
      #include "l1audio_stereo.h"
    #endif

    #include "mem.h"

  #else
  // Layer1 and debug include files.

    #include <ctype.h>
    #include <math.h>
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"

    #if TESTMODE
     #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif

    #if (L1_MP3 == 1)
      #include "l1mp3_const.h"
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_const.h"
      #include "l1midi_defty.h"
    #endif

    #if (L1_AAC == 1)
      #include "l1aac_const.h"
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "cust_os.h"
    #include "l1_msgty.h"
    #include "tpudrv.h"       // TPU drivers.           ("eva3.lib")
    #include "l1_varex.h"

    #include "l1_proto.h"
    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "mem.h"
    #include "armio.h"
    #include "timer.h"
    #include "timer1.h"
    #include "dma.h"
    #include "inth.h"
    #include "ulpd.h"
    #include "rhea_arm.h"
    #include "clkm.h"         // Clockm  ("eva3.lib")
    #include "l1_ctl.h"

    #include "l1_time.h"
    #if L2_L3_SIMUL
      #include "l1_scen.h"
    #endif

    #if (L1_STEREOPATH == 1)
      #include "sys_dma.h"
      #include "abb.h"
      #if TESTMODE
        #include "l1tm_msgty.h"
      #endif
      #include "l1audio_stereo.h"
    #endif

    #include "mem.h"

 #endif

  #include "l1audio_macro.h"
  #include "l1_trace.h"
 #if (CODE_VERSION != SIMULATION)
  #include "bspTwl3029.h"
  #include "bspTwl3029_I2c.h"
  #include "bspTwl3029_Aud_Map.h"
  #include "bspTwl3029_Int_Map.h"
#endif
  /**************************************/
  /* Prototypes for L1 SYNCH manager    */
  /**************************************/
  void l1s_audio_manager(void);
  #if (KEYBEEP)
    void l1s_keybeep_manager(void);
  #endif
  #if (TONE)
    void l1s_tone_manager(void);
  #endif
  #if (L1_CPORT == 1)
    void l1s_cport_manager(void);
  #endif
  #if (MELODY_E1)
    void l1s_melody0_manager(void);
    void l1s_melody1_manager(void);
  #endif
  #if (VOICE_MEMO)
    void l1s_vm_play_manager  (void);
    void l1s_vm_record_manager(void);
    void l1s_tone_ul_manager  (void);
  #endif
  #if  (L1_PCM_EXTRACTION)
    void l1s_pcm_download_manager (void);
    void l1s_pcm_upload_manager   (void);
  #endif
  #if (L1_VOICE_MEMO_AMR)
    void l1s_vm_amr_play_manager  (void);
    void l1s_vm_amr_record_manager(void);
  #endif
  #if (SPEECH_RECO)
    void l1s_sr_enroll_manager    (void);
    void l1s_sr_update_manager    (void);
    void l1s_sr_reco_manager      (void);
    void l1s_sr_processing_manager(void);
    void l1s_sr_speech_manager    (void);
  #endif
  #if (L1_AEC == 1)
    void l1s_aec_manager          (void);
  #endif
  #if (L1_AEC == 2)
    void l1s_aec_manager          (void);
  #endif
  #if (FIR)
    void l1s_fir_manager          (void);
    void l1s_fir_set_params       (void);

  #endif
  #if (AUDIO_MODE)
    void l1s_audio_mode_manager   (void);
  #endif
  #if (MELODY_E2)
    void l1s_melody0_e2_manager(void);
    void l1s_melody1_e2_manager(void);
  #endif
  #if (L1_MP3 == 1)
    void l1s_mp3_manager(void);
  #endif
  #if (L1_MIDI == 1)
    void l1s_midi_manager(void);
  #endif
  #if (L1_AAC == 1)
    void l1s_aac_manager(void);
  #endif
  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
   void l1s_audio_onoff_manager(void);
  #endif
  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
   void l1s_audio_voice_onoff_manager(void);
  #endif
  #if (L1_STEREOPATH == 1)
    void l1s_stereopath_drv_manager(void);
  #endif
  #if (L1_ANR == 1 || L1_ANR == 2)
    void l1s_anr_manager(void);
  #endif
  #if (L1_IIR == 1 || L1_IIR == 2)
    void l1s_iir_manager(void);
  #endif
  #if (L1_WCM == 1)
    void l1s_wcm_manager(void);
  #endif
  #if (L1_AGC_UL == 1)
    void l1s_agc_ul_manager(void);
  #endif
  #if (L1_AGC_DL == 1)
    void l1s_agc_dl_manager(void);
  #endif
  #if (L1_DRC == 1)
    void l1s_drc_manager(void);
  #endif
  #if (L1_LIMITER == 1)
    void l1s_limiter_manager(void);
  #endif
  #if (L1_ES == 1)
    void l1s_es_manager(void);
  #endif

  void l1s_audio_it_manager(void);
  /**************************************/
  /* External prototypes                */
  /**************************************/
  extern UWORD8  copy_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_dst);
  extern UWORD8  copy_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_src);
  extern UWORD8 Cust_get_pointer       (UWORD16 **ptr, UWORD16 *buffer_size, UWORD8 session_id);
  #if (MELODY_E2)
    extern UWORD16 audio_twentyms_to_TDMA_convertion (UWORD16 twentyms_value);
  #endif
  #if (MELODY_E2) || (L1_VOICE_MEMO_AMR)
    extern UWORD8  copy_byte_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_dst);
    extern UWORD8  copy_byte_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_src);
  #endif
  #if (L1_VOICE_MEMO_AMR)
    extern UWORD8  copy_byte_data_le_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_dst);
    extern UWORD8  copy_byte_data_le_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, API *ptr_src);
  #endif

  #if (L1_LIMITER == 1)
    void l1_audio_lim_update_mul_low_high ();
  #endif

  #if (L1_AGC_UL == 1)
    extern void l1_audio_agc_ul_copy_params();
  #endif

  #if (L1_AGC_DL == 1)
    extern void l1_audio_agc_dl_copy_params();
  #endif

   #if (L1_IIR == 2)
    extern void l1_audio_iir4x_copy_params();
  #endif

#if (L1_DRC == 1)
    extern void l1_audio_drc1x_copy_params();
  #endif

  // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
    static void l1s_audio_ul_onoff_manager();
    static void l1s_audio_dl_onoff_manager();

    void l1_audio_abb_ul_on_callback(void);
    void l1_audio_abb_ul_off_callback(void);
    void l1_audio_abb_dl_on_callback(void);
    void l1_audio_abb_dl_off_callback(void);
    void l1_audio_abb_ul_off_dl_off_callback(void);
    void l1_audio_abb_ul_off_dl_on_callback(void);
    void l1_audio_abb_ul_on_dl_off_callback(void);
    void l1_audio_abb_ul_on_dl_on_callback(void);
    void l1_audio_ul_onoff_trace();
    void l1_audio_dl_onoff_trace();
#if (CODE_VERSION != SIMULATION)
    BspTwl3029_ReturnCode l1_outen_update(void);
#endif

#if (CODE_VERSION == SIMULATION)
    signed char l1_outen_update(void);
#endif
#endif    // L1_AUDIO_MCU_ONOFF

#if (CODE_VERSION == SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)

// Triton Audio ON/OFF Changes
void l1_audio_abb_ul_on_req         ( void(*callback_fn)(void) );
void l1_audio_abb_dl_on_req         ( void(*callback_fn)(void) );
void l1_audio_abb_ul_off_req        ( void(*callback_fn)(void) );
void l1_audio_abb_dl_off_req        ( void(*callback_fn)(void) );
void l1_audio_abb_ul_off_dl_off_req ( void(*callback_fn)(void) );
void l1_audio_abb_ul_off_dl_on_req  ( void(*callback_fn)(void) );
void l1_audio_abb_ul_on_dl_off_req  ( void(*callback_fn)(void) );
void l1_audio_abb_ul_on_dl_on_req   ( void(*callback_fn)(void) );


#endif

#if (ANALOG == 11)
//sundi: add the abb_write_done variable for I2C write acknowledgement
//UWORD8 abb_write_done = 0;
#endif

  /*-------------------------------------------------------*/
  /* l1s_audio_manager()                                   */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : Global manager of the L1S audio task.   */
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1s_audio_manager(void)
  {
    BOOL  l1_audio_it_com = FALSE;

    // Initialize the ITCOM for audio task
    l1s.l1_audio_it_com = FALSE;

    #if (KEYBEEP)
      // the keybeep task is activated?
      if ((l1a_l1s_com.keybeep_task.command.start) ||
          (l1s.audio_state[L1S_KEYBEEP_STATE] != 0))
      {
        l1s_keybeep_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (TONE)
      // the tone task is activated?
      if ((l1a_l1s_com.tone_task.command.start) ||
          (l1s.audio_state[L1S_TONE_STATE] != 0))
      {
        l1s_tone_manager();
        l1_audio_it_com = TRUE;
      }
    #endif

    #if (MELODY_E1)
      // the melody0 task is activated?
      if ((l1a_l1s_com.melody0_task.command.start) ||
          (l1s.audio_state[L1S_MELODY0_STATE] != 0))
      {
        l1s_melody0_manager();
        l1_audio_it_com = TRUE;
      }
      // the melody1 task is activated?
      if ((l1a_l1s_com.melody1_task.command.start) ||
          (l1s.audio_state[L1S_MELODY1_STATE] != 0))
      {
        l1s_melody1_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (VOICE_MEMO)
      // the voicememo playing task is activated?
      if ((l1a_l1s_com.voicememo_task.play.command.start) ||
          (l1s.audio_state[L1S_VM_PLAY_STATE] != 0))
      {
        l1s_vm_play_manager();
        l1_audio_it_com = TRUE;
      }
      else // voicememo playing task and voice memo recoding task isn't compatible
      {
        // the voicememo recording task is activated?
        if ((l1a_l1s_com.voicememo_task.record.command.start) ||
            (l1s.audio_state[L1S_VM_RECORD_STATE] != 0))
        {
          l1s_vm_record_manager();
          l1_audio_it_com = TRUE;
        }
        // the voicememo tone uplink task is activated?
        if ((l1a_l1s_com.voicememo_task.record.tone_ul.start) ||
            (l1s.audio_state[L1S_TONE_UL_STATE] != 0))
        {
          l1s_tone_ul_manager();
          l1_audio_it_com = TRUE;
        }
      }
    #endif
    #if (L1_PCM_EXTRACTION)
      // the PCM download task is activated?
      if ((l1a_l1s_com.pcm_task.download.command.start) ||
          (l1s.audio_state[L1S_PCM_DOWNLOAD_STATE] != 0))
      {
        l1s_pcm_download_manager();
        l1_audio_it_com = TRUE;
      }
      // the PCM upload task is activated?
      if ((l1a_l1s_com.pcm_task.upload.command.start) ||
          (l1s.audio_state[L1S_PCM_UPLOAD_STATE] != 0))
      {
        l1s_pcm_upload_manager();
        l1_audio_it_com = TRUE;
      }
    #endif  /* L1_PCM_EXTRACTION */
    #if (L1_VOICE_MEMO_AMR)
      // the voicememo playing task is activated?
      if ((l1a_l1s_com.voicememo_amr_task.play.command.start) ||
          (l1s.audio_state[L1S_VM_AMR_PLAY_STATE] != 0))
      {
        l1s_vm_amr_play_manager();
        l1_audio_it_com = TRUE;
      }
      else // voicememo playing task and voice memo recoding task isn't compatible
      {
        // the voicememo recording task is activated?
        if ((l1a_l1s_com.voicememo_amr_task.record.command.start) ||
            (l1s.audio_state[L1S_VM_AMR_RECORD_STATE] != 0))
        {
          l1s_vm_amr_record_manager();
          l1_audio_it_com = TRUE;
        }
      }
    #endif
    #if (SPEECH_RECO)
      // the speech recognition enroll task is activated?
      if ((l1a_l1s_com.speechreco_task.command.enroll_start) ||
          (l1s.audio_state[L1S_SR_ENROLL_STATE] != 0))
      {
        l1s_sr_enroll_manager();
        l1_audio_it_com = TRUE;
      }
      else
      // the speech recognition update task is activated?
      if ((l1a_l1s_com.speechreco_task.command.update_start) ||
          (l1s.audio_state[L1S_SR_UPDATE_STATE] != 0))
      {
        l1s_sr_update_manager();
        l1_audio_it_com = TRUE;
      }
      else
      // the speech recognition reco task is activated?
      if ((l1a_l1s_com.speechreco_task.command.reco_start) ||
          (l1s.audio_state[L1S_SR_RECO_STATE] != 0))
      {
        l1s_sr_reco_manager();
        l1_audio_it_com = TRUE;
      }
      else
      // the speech recognition processing task is activated?
      if ((l1a_l1s_com.speechreco_task.command.processing_start) ||
          (l1s.audio_state[L1S_SR_PROCESSING_STATE] != 0))
      {
        l1s_sr_processing_manager();
        l1_audio_it_com = TRUE;
      }
      // the speech recognition speech recording task is activated?
      if ((l1a_l1s_com.speechreco_task.command.speech_start) ||
          (l1s.audio_state[L1S_SR_SPEECH_STATE] != 0))
      {
        l1s_sr_speech_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_AEC == 1)
      // the AEC is activated?
      if ((l1a_l1s_com.aec_task.command.start) ||
          (l1s.audio_state[L1S_AEC_STATE] != 0))
      {
        l1s_aec_manager();
        // It's not necessary to enable the IT DSP because the
        // AEC works with the dedicated speech therefor an IT DSP is
        // already requested by the modem
      }
    #endif
    #if (L1_AEC == 2)
      // the AEC is activated?
      if ((l1a_l1s_com.aec_task.command.start) ||
          (l1s.audio_state[L1S_AEC_STATE] != 0))
      {
        l1s_aec_manager();
		l1_audio_it_com = TRUE;
      }
    #endif
    #if (FIR)
      // the FIR is activated?
if ((l1a_l1s_com.fir_task.command.start) ||
          (l1s.audio_state[L1S_FIR_STATE] !=0 ))
      {
        l1s_fir_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (AUDIO_MODE)
      // the AUDIO MODE is activated?
      if (l1a_l1s_com.audio_mode_task.command.start)
      {
        l1s_audio_mode_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (MELODY_E2)
      // Handling counters
      if (l1s.melody_e2.dsp_task == TRUE)
      {
        l1s.melody_e2.timebase++;
        l1s.melody_e2.timebase_mod_60ms++;
        if (l1s.melody_e2.timebase_mod_60ms == 13)
          l1s.melody_e2.timebase_mod_60ms = 0;
      }
      else
      {
        l1s.melody_e2.timebase = 0;
        l1s.melody_e2.timebase_mod_60ms = 0;
      }

      // Update the oscillator active fields with DSP active oscillators
      l1s.melody_e2.global_osc_active = l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_osc_active;
      l1s.melody_e2.global_osc_to_start = 0;

      // oscillator active fields can't be updated until dsp has acknowledged new notes
      if (l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore == 0)
      {
        l1s.melody0_e2.oscillator_active &= l1s.melody_e2.global_osc_active;
        l1s.melody1_e2.oscillator_active &= l1s.melody_e2.global_osc_active;
      }

      // Both states machines are inactive => Start or Stop DSP module
      if ( (l1s.audio_state[L1S_MELODY0_E2_STATE] == 0) &&
           (l1s.audio_state[L1S_MELODY1_E2_STATE] == 0) )
      {
        // Start command + DSP module not running => Start DSP module
        if( ((l1a_l1s_com.melody0_e2_task.command.start) ||
            (l1a_l1s_com.melody1_e2_task.command.start)) &&
            (l1s.melody_e2.dsp_task == FALSE) )
        {
          // Set the bit TONE to do the init only at the first start
          // of the first melody (in case of 2 melodies)
          // Select the melody E2 instead of E1 or tones
          l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E2_SELECTED;
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_MELO | B_TONE);
          l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = 0;

          // set delta_time as non valid
          l1s.melody0_e2.delta_time = 0xFFFF;
          l1s.melody1_e2.delta_time = 0xFFFF;

          // Set the flag to confirm that the DSP melody E2 task is started
          l1s.melody_e2.dsp_task = TRUE;
        }
        // No command, DSP module running => Stop DSP module
        else
        if (l1s.melody_e2.dsp_task)
        {
          // The 2 melodies are stopped therefore the DSP must be
          // stopped so the bit B_MELO must be reset.
          l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E2_SELECTED;
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_MELO | B_TONE);

          // Reset the flag to know if the DSP melody E2 task runs
          l1s.melody_e2.dsp_task = FALSE;

          l1_audio_it_com = TRUE;
        }
      }

      // the melody0 task is activated?
      if ( (l1a_l1s_com.melody0_e2_task.command.start) ||
           (l1s.audio_state[L1S_MELODY0_E2_STATE] != 0) )
      {
        l1s_melody0_e2_manager();
        l1_audio_it_com = TRUE;
      }
      // the melody0 task is activated?
      if ( (l1a_l1s_com.melody1_e2_task.command.start) ||
           (l1s.audio_state[L1S_MELODY1_E2_STATE] != 0) )
      {
        l1s_melody1_e2_manager();
        l1_audio_it_com = TRUE;
      }

      // We updated some oscillators so we must set the semaphore
      if (l1s.melody_e2.global_osc_to_start != 0x0000)
      {
        // Set the melody E2 semaphore
        l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore = 0x0001;

        // Update delta_time by min_delta_time(melo0, melo1)
        l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = l1s.melody0_e2.delta_time;
        if (l1s.melody1_e2.delta_time < l1s.melody0_e2.delta_time)
          l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = l1s.melody1_e2.delta_time;

        l1s.melody0_e2.delta_time = 0xFFFF;
        l1s.melody1_e2.delta_time = 0xFFFF;
      }
    #endif

    #if (L1_MP3 == 1)
      if ((l1a_l1s_com.mp3_task.command.start) ||
          (l1s.audio_state[L1S_MP3_STATE] != 0))
      {
        l1s_mp3_manager();
        l1_audio_it_com=TRUE;
      }
    #endif  // L1_MP3

    #if (L1_AAC == 1)
      if ((l1a_l1s_com.aac_task.command.start) ||
          (l1s.audio_state[L1S_AAC_STATE] != 0))
      {
        l1s_aac_manager();
        l1_audio_it_com=TRUE;
      }
    #endif  // L1_AAC
    #if (L1_CPORT == 1)
      // the Cport task is activated?
      if ((l1a_l1s_com.cport_task.command.start) ||
          (l1s.audio_state[L1S_CPORT_STATE] != 0))
      {
        l1s_cport_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
      // the audio on off task is activated?
      if (l1a_l1s_com.audio_onoff_task.command.start ||
          (l1s.audio_state[L1S_AUDIO_ONOFF_STATE] != 0))
      {
        l1s_audio_onoff_manager();
        l1_audio_it_com = TRUE;
      }
    #endif

    #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
      // the audio on off task is activated?
      if (l1a_l1s_com.audio_onoff_task.command.start ||
          (l1s.audio_state[L1S_AUDIO_ONOFF_STATE] != 0))
      {
        l1s_audio_voice_onoff_manager();
        l1_audio_it_com = TRUE;
      }
    #endif

    #if (L1_STEREOPATH == 1)
      // the Stereopath task is activated?
      if ((l1a_l1s_com.stereopath_drv_task.command.start) ||
          (l1s.audio_state[L1S_STEREOPATH_DRV_STATE] != 0))
      {
        l1s_stereopath_drv_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_ANR == 1 || L1_ANR == 2)
      // the ANR task is activated?
      if (l1a_l1s_com.anr_task.command.update ||
          (l1s.audio_state[L1S_ANR_STATE] != 0))
      {
        l1s_anr_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_IIR == 1 || L1_IIR == 2)
      // the IIR task is activated?
      if (l1a_l1s_com.iir_task.command.update ||
          (l1s.audio_state[L1S_IIR_STATE] != 0))
      {
        l1s_iir_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_WCM == 1)
      // the WCM task is activated?
      if (l1a_l1s_com.wcm_task.command.update ||
          (l1s.audio_state[L1S_WCM_STATE] != 0))
      {
        l1s_wcm_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_DRC == 1)
      // the DRC task is activated?
      if (l1a_l1s_com.drc_task.command.update ||
          (l1s.audio_state[L1S_DRC_STATE] != 0))
      {
        l1s_drc_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_AGC_UL == 1)
      // the AGC UL task is activated?
      if (l1a_l1s_com.agc_ul_task.command.update ||
          (l1s.audio_state[L1S_AGC_UL_STATE] != 0))
      {
        l1s_agc_ul_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_AGC_DL == 1)
      // the AGC DL task is activated?
      if (l1a_l1s_com.agc_dl_task.command.update ||
          (l1s.audio_state[L1S_AGC_DL_STATE] != 0))
      {
        l1s_agc_dl_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_LIMITER == 1)
      // the LIMITER task is activated?
      if (l1a_l1s_com.limiter_task.command.update ||
          l1a_l1s_com.limiter_task.command.partial_update ||
          (l1s.audio_state[L1S_LIMITER_STATE] != 0))
      {
        l1s_limiter_manager();
        l1_audio_it_com = TRUE;
      }
    #endif
    #if (L1_ES == 1)
      // the echo suppressor task is activated?
      if (l1a_l1s_com.es_task.command.update ||
          (l1s.audio_state[L1S_ES_STATE] != 0))
      {
        l1s_es_manager();
        l1_audio_it_com = TRUE;
      }
    #endif

    // the audio It task is activated?
    if (l1a_l1s_com.audioIt_task.command.start)
    {
      l1s_audio_it_manager();
      l1_audio_it_com = TRUE;
    }

    #if (L1_MIDI == 1)
      if ((l1a_l1s_com.midi_task.command.start) ||
          (l1s.audio_state[L1S_MIDI_STATE] != 0))
      {
        l1s_midi_manager();
        l1_audio_it_com=TRUE;
      }
    #endif  // L1_MIDI

    // The audio IT shall be foprwarded to the DSP in case the L1S is forcing the audio
    if (l1a_l1s_com.audio_forced_by_l1s == TRUE)
      l1_audio_it_com = TRUE;

    l1s.l1_audio_it_com = l1_audio_it_com;
  }


#if (L1_AUDIO_MCU_ONOFF == 1)
  void l1s_audio_onoff_manager(void)
  {
    T_L1S_AUDIO_ONOFF_MANAGER   *onoff_ctl = &(l1s.audio_on_off_ctl);

	//l1_audio_onoff_entry_sim_log();
#if (AUDIO_DEBUG == 1)
    trace_info.audio_debug_var.ul_state = l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE];
    trace_info.audio_debug_var.dl_state = l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE];
    trace_info.audio_debug_var.ul_onoff_counter = onoff_ctl->l1_audio_switch_on_ul_request;
    trace_info.audio_debug_var.dl_onoff_counter = onoff_ctl->l1_audio_switch_on_dl_request;
#endif

    l1s_audio_ul_onoff_manager();
    l1s_audio_dl_onoff_manager();

    // Call the driver depending on the actions
    switch(onoff_ctl->l1_audio_ul_action)
    {
        case L1_AUDIO_NO_ACTION:
        {
            switch(onoff_ctl->l1_audio_dl_action)
            {
                case L1_AUDIO_NO_ACTION:
                {
                    // UL No Action and DL No Action Do nothing
                }
                break;
                case L1_AUDIO_TURN_ON:
                {
                    // UL No Action and DL Turn on
                    l1_audio_abb_dl_on_req(l1_audio_abb_dl_on_callback);
                }
                break;
                case L1_AUDIO_TURN_OFF:
                {
                    // UL No Action and DL Turn off
                    l1_audio_abb_dl_off_req(l1_audio_abb_dl_off_callback);
                }
                break;
                default:
                {
                    // Invalid Action send error trace here
                }
                break;
            } // end switch(l1_audio_dl_action)
        }
        break;
        case L1_AUDIO_TURN_ON:
        {
            switch(onoff_ctl->l1_audio_dl_action)
            {
                case L1_AUDIO_NO_ACTION:
                {
                    // UL Action ON and DL No Action
                    l1_audio_abb_ul_on_req(l1_audio_abb_ul_on_callback);
                }
                break;
                case L1_AUDIO_TURN_ON:
                {
                    // UL Action ON and DL Action ON
                    l1_audio_abb_ul_on_dl_on_req(l1_audio_abb_ul_on_dl_on_callback);
                }
                break;
                case L1_AUDIO_TURN_OFF:
                {
                    // UL Action ON and DL Action OFF
                    l1_audio_abb_ul_on_dl_off_req(l1_audio_abb_ul_on_dl_off_callback);
                }
                break;
                default:
                {
                    // Invalid Action Send error trace here
                }
                break;
            } // end switch(l1_audio_dl_action)

        }
        break;
        case L1_AUDIO_TURN_OFF:
        {
            switch(onoff_ctl->l1_audio_dl_action)
            {
                case L1_AUDIO_NO_ACTION:
                {
                    // UL Action OFF DL No Action
                    l1_audio_abb_ul_off_req(l1_audio_abb_ul_off_callback);
                }
                break;
                case L1_AUDIO_TURN_ON:
                {
                    // UL Action OFF DL Action ON
                    l1_audio_abb_ul_off_dl_on_req(l1_audio_abb_ul_off_dl_on_callback);
                }
                break;
                case L1_AUDIO_TURN_OFF:
                {
                    // UL Action OFF DL Action OFF
                    l1_audio_abb_ul_off_dl_off_req(l1_audio_abb_ul_off_dl_off_callback);
                }
                break;
                default:
                {
                }
                break;
            } // end switch(l1_audio_dl_action)

        }
        break;
        default:
        {
            // Invalid Action send error trace here
        }
        break;
    } // end switch(l1_audio_ul_action)

  } // end l1s_audio_triton_onoff_manager()

  static void l1s_audio_ul_onoff_manager()
  {

    T_L1S_AUDIO_ONOFF_MANAGER   *onoff_ctl = &(l1s.audio_on_off_ctl);
    UWORD8            *state      = &l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE];

    onoff_ctl->l1_audio_ul_action = L1_AUDIO_NO_ACTION;


    if(onoff_ctl->l1_audio_switch_on_ul_request >= 1)
    {
        // At least one module requires UL audio path to be on
        switch(*state)
        {
            case L1_AUDIO_UL_OFF:
            {
                // UL audio path is OFF and needs to be started
                onoff_ctl->l1_audio_ul_on2off_hold_time =
                    L1_AUDIO_ON2OFF_UL_HOLD_TIME;
                onoff_ctl->l1_audio_ul_action       = L1_AUDIO_TURN_ON;
                *state        = L1_AUDIO_UL_SWITCHON_STARTED;
                onoff_ctl->l1_audio_ul_switched_on  = FALSE;
                onoff_ctl->l1_audio_ul_switched_off = TRUE;
            }
            break;
            case L1_AUDIO_UL_SWITCHON_STARTED:
            {
                // UL audio path in the process of turning on
                // Check if it is turned on, if so change state to UL on
                if(onoff_ctl->l1_audio_ul_switched_on == TRUE)
                {
                    onoff_ctl->l1_audio_ul_switched_off = FALSE;
                    *state = L1_AUDIO_UL_ON;
                }
                 #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_UL_ON:
            {
                // UL audio path is on and some module wants it to be so
                // Do nothing essentially except initialize the hold time
                onoff_ctl->l1_audio_ul_on2off_hold_time =
                    L1_AUDIO_ON2OFF_UL_HOLD_TIME;
                 #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_UL_SWITCHOFF_STARTED:
            {
                // UL is being switched off and some module requests switch on!
                // Since the driver is required to maintain the order of requests
                // we perform the same actions as in the UL_OFF state
                onoff_ctl->l1_audio_ul_on2off_hold_time =
                    L1_AUDIO_ON2OFF_UL_HOLD_TIME;
                onoff_ctl->l1_audio_ul_action       = L1_AUDIO_TURN_ON;
                *state        = L1_AUDIO_UL_SWITCHON_STARTED;
                onoff_ctl->l1_audio_ul_switched_on  = FALSE;
                onoff_ctl->l1_audio_ul_switched_off = TRUE;
                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            default:
            {
                // Invalid state put error trace here
            }
            break;
        } // end switch(l1_audio_ul_state)

    } // end if(l1_audio_switch_on_ul_req >= 1)
    else
    {
        // No module requires UL Audio path to be on
        switch(*state)
        {
            case L1_AUDIO_UL_OFF:
            {
                // UL audio path is off and all modules want it that way, do nothing
            }
            break;
            case L1_AUDIO_UL_SWITCHON_STARTED:
            {
                // UL audio path being switched on. Modules want it turned off!
                // Allow UL to be turned on, still if modules want it turned off
                // we will wait for ON2OFF hold time and turn it off
                if(onoff_ctl->l1_audio_ul_switched_on == TRUE)
                {
                    onoff_ctl->l1_audio_ul_switched_off = FALSE;
                    *state = L1_AUDIO_UL_ON;
                }
                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_UL_ON:
            {
                // UL audio path is on and all modules want turn off. We wait
                // for the ON2OFF hold time and then actually turn it off
                if(onoff_ctl->l1_audio_ul_on2off_hold_time == 0)
                {
                    onoff_ctl->l1_audio_ul_action = L1_AUDIO_TURN_OFF;
                    *state = L1_AUDIO_UL_SWITCHOFF_STARTED;
                    onoff_ctl->l1_audio_ul_switched_off = FALSE;
                    onoff_ctl->l1_audio_ul_switched_on = TRUE;
                }
                else
                    onoff_ctl->l1_audio_ul_on2off_hold_time--;
                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_UL_SWITCHOFF_STARTED:
            {
                if(onoff_ctl->l1_audio_ul_switched_off == TRUE)
                {
                    *state = L1_AUDIO_UL_OFF;
                }
                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            default:
            {
                // Invalid state send error trace here
            }
            break;
        } // end switch(l1_audio_ul_state)

    } // end if(l1_audio_switch_on_ul_req >= 1)else

#if (CODE_VERSION == SIMULATION)
    l1_audio_ul_onoff_simu_trace();
#else
    l1_audio_ul_onoff_trace();
#endif

  } // end l1s_audio_ul_onoff_manager()

  static void l1s_audio_dl_onoff_manager()
  {

    T_L1S_AUDIO_ONOFF_MANAGER   *onoff_ctl = &(l1s.audio_on_off_ctl);
    UWORD8            *state      = &l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE];

    onoff_ctl->l1_audio_dl_action = L1_AUDIO_NO_ACTION;


    if(onoff_ctl->l1_audio_switch_on_dl_request >= 1)
    {
        // At least one module requires DL audio path to be on
        switch(*state)
        {
            case L1_AUDIO_DL_OFF:
            {
                // DL audio path is OFF and needs to be started
                onoff_ctl->l1_audio_dl_on2off_hold_time =
                    L1_AUDIO_ON2OFF_DL_HOLD_TIME;
                onoff_ctl->l1_audio_dl_action       = L1_AUDIO_TURN_ON;
                *state        = L1_AUDIO_DL_SWITCHON_STARTED;
                onoff_ctl->l1_audio_dl_switched_on  = FALSE;
                onoff_ctl->l1_audio_dl_switched_off = TRUE;
            }
            break;
            case L1_AUDIO_DL_SWITCHON_STARTED:
            {
                // DL audio path in the process of turning on
                // Check if it is turned on, if so change state to DL on
                if(onoff_ctl->l1_audio_dl_switched_on == TRUE)
                {
                    onoff_ctl->l1_audio_dl_switched_off = FALSE;
                    *state = L1_AUDIO_DL_ON;
                }
			    #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_DL_ON:
            {

                /* OUTEN registers have been updated */
                if(l1a_l1s_com.outen_cfg_task.command_requested != l1a_l1s_com.outen_cfg_task.command_commited)
                {
					#if (CODE_VERSION != SIMULATION)
                  l1_outen_update();
                  #endif
                }
                // DL audio path is on and some module wants it to be so
                // Do nothing essentially except initialize the hold time
                onoff_ctl->l1_audio_dl_on2off_hold_time =
                    L1_AUDIO_ON2OFF_DL_HOLD_TIME;
                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_DL_SWITCHOFF_STARTED:
            {
                // DL is being switched off and some module requests switch on!
                // Since the driver is required to maintain the order of requests
                // we perform the same actions as in the DL_OFF state
                onoff_ctl->l1_audio_dl_on2off_hold_time =
                    L1_AUDIO_ON2OFF_DL_HOLD_TIME;
                onoff_ctl->l1_audio_dl_action       = L1_AUDIO_TURN_ON;
                *state        = L1_AUDIO_DL_SWITCHON_STARTED;
                onoff_ctl->l1_audio_dl_switched_on  = FALSE;
                onoff_ctl->l1_audio_dl_switched_off = TRUE;

                #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            default:
            {
                // Invalid state put error trace here
            }
            break;
        } // end switch(l1_audio_dl_state)

    } // end if(l1_audio_switch_on_dl_req >= 1)
    else
    {
        // No module requires DL Audio path to be on
        switch(*state)
        {
            case L1_AUDIO_DL_OFF:
            {
                // DL audio path is off and all modules want it that way, do nothing
            }
            break;
            case L1_AUDIO_DL_SWITCHON_STARTED:
            {
                // DL audio path being switched on. Modules want it turned off!
                // Allow DL to be turned on, still if modules want it turned off
                // we will wait for ON2OFF hold time and turn it off
                if(onoff_ctl->l1_audio_dl_switched_on == TRUE)
                {
                    onoff_ctl->l1_audio_dl_switched_off = FALSE;
                    *state = L1_AUDIO_DL_ON;
                }
                #if(CODE_VERSION == NOT_SIMULATION)
                 l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_DL_ON:
            {
                // DL audio path is on and all modules want turn off. We wait
                // for the ON2OFF hold time and then actually turn it off
                if(onoff_ctl->l1_audio_dl_on2off_hold_time == 0)
                {
                    onoff_ctl->l1_audio_dl_action = L1_AUDIO_TURN_OFF;
                    *state = L1_AUDIO_DL_SWITCHOFF_STARTED;
                    onoff_ctl->l1_audio_dl_switched_off = FALSE;
                    onoff_ctl->l1_audio_dl_switched_on = TRUE;
                }
                else
                    onoff_ctl->l1_audio_dl_on2off_hold_time--;

                 #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            case L1_AUDIO_DL_SWITCHOFF_STARTED:
            {
                if(onoff_ctl->l1_audio_dl_switched_off == TRUE)
                {
                    *state = L1_AUDIO_DL_OFF;
                }
                 #if(CODE_VERSION == NOT_SIMULATION)
                l1s.l1_audio_it_com = TRUE;
                #endif
            }
            break;
            default:
            {
                // Invalid state send error trace here
            }
            break;
        } // end switch(l1_audio_dl_state)

    } // end if(l1_audio_switch_on_dl_req >= 1)else

#if (CODE_VERSION == SIMULATION)
    l1_audio_dl_onoff_simu_trace();
#else
    l1_audio_dl_onoff_trace();
#endif


  } // end l1s_audio_dl_onoff_manager()

#if (CODE_VERSION == NOT_SIMULATION)
void l1_audio_ul_onoff_trace()
{
    static T_L1_AUDIO_UL_STATE  prev_state = L1_INVALID;
    UWORD8            *curr_state      = &l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE];

    if(*curr_state != prev_state)
    {
        prev_state =(T_L1_AUDIO_UL_STATE) *curr_state;
        switch(prev_state)
        {
            case L1_AUDIO_UL_OFF:
            {
                //L1_trace_string("UL AUDIO OFF\r\n");
                l1_trace_ul_audio_onoff(L1_AUDIO_UL_OFF);
            }
             break;
            case L1_AUDIO_UL_SWITCHON_STARTED:
            {
                l1_trace_ul_audio_onoff(L1_AUDIO_UL_SWITCHON_STARTED);
            }
             break;
            case L1_AUDIO_UL_ON:
            {
                l1_trace_ul_audio_onoff(L1_AUDIO_UL_ON);
            }
             break;
            case L1_AUDIO_UL_SWITCHOFF_STARTED:
            {
                l1_trace_ul_audio_onoff(L1_AUDIO_UL_SWITCHOFF_STARTED);
            }
             break;

        } // End switch(prev_state)
    } // end if(l1s.ul_state == prev_state)

}

void l1_audio_dl_onoff_trace()
{
    static T_L1_AUDIO_DL_STATE  prev_state = L1_DL_INVALID;
    UWORD8            *curr_state      = &l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE];

    if(*curr_state != prev_state)
    {
        prev_state = (T_L1_AUDIO_DL_STATE) *curr_state;
        switch(prev_state)
        {
            case L1_AUDIO_DL_OFF:
            {
                l1_trace_dl_audio_onoff(L1_AUDIO_DL_OFF);
            }
             break;
            case L1_AUDIO_DL_SWITCHON_STARTED:
            {
                l1_trace_dl_audio_onoff(L1_AUDIO_DL_SWITCHON_STARTED);
            }
             break;
            case L1_AUDIO_DL_ON:
            {
                l1_trace_dl_audio_onoff(L1_AUDIO_DL_ON);
            }
             break;
            case L1_AUDIO_DL_SWITCHOFF_STARTED:
            {
                l1_trace_dl_audio_onoff(L1_AUDIO_DL_SWITCHOFF_STARTED);
            }
             break;

        } // End switch(prev_state)
    } // end if(l1s.ul_state == prev_state)
}

#endif // CODE_VERSION == NOT_SIMULATION

  void l1_audio_abb_ul_on_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_on = TRUE;
  }
  void l1_audio_abb_ul_off_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_off = TRUE;
  }
  void l1_audio_abb_dl_on_callback(void)
  {
    l1_outen_update();
  }
  void l1_audio_abb_dl_off_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_dl_switched_off = TRUE;
  }
  void l1_audio_abb_ul_off_dl_off_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_off = TRUE;
    l1s.audio_on_off_ctl.l1_audio_dl_switched_off = TRUE;
  }
  void l1_audio_abb_ul_off_dl_on_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_off = TRUE;
    l1_outen_update();
  }
  void l1_audio_abb_ul_on_dl_off_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_on = TRUE;
    l1s.audio_on_off_ctl.l1_audio_dl_switched_off = TRUE;
  }
  void l1_audio_abb_ul_on_dl_on_callback(void)
  {
    l1s.audio_on_off_ctl.l1_audio_ul_switched_on = TRUE;

    l1_outen_update();
  }

  void l1_audio_abb_outen_cfg_callback(UWORD8 argument)
  {
    (void)(argument);
    l1s.audio_on_off_ctl.l1_audio_dl_switched_on = TRUE;
  }
#if (CODE_VERSION != SIMULATION)

BspTwl3029_ReturnCode l1_outen_update(void)
{
    BspTwl3029_ReturnCode returnVal = BSP_TWL3029_RETURN_CODE_FAILURE;
    UWORD16 count = 0;
    UINT8 triton_classD = 0;

    /* callback function info pointer */
    BspTwl3029_I2C_Callback i2c_callback;
    BspTwl3029_I2C_CallbackPtr callbackPtr= &i2c_callback;

    /* I2C array */
    Bsp_Twl3029_I2cTransReqArray i2cTransArray;
    Bsp_Twl3029_I2cTransReqArrayPtr i2cTransArrayPtr= &i2cTransArray;

    /* twl3029 I2C reg info struct */
    BspTwl3029_I2C_RegisterInfo regInfo[11] ;
    BspTwl3029_I2C_RegisterInfo* regInfoPtr = regInfo;

    BspTwl3029_I2C_RegData tmpAudioHFTest1RegData=0;
    BspTwl3029_I2C_RegData tmpCtrl3RegData=0;


    l1a_l1s_com.outen_cfg_task.command_requested = l1a_l1s_com.outen_cfg_task.command_commited;

    bspTwl3029_Audio_getClassD_mode(&triton_classD);

    returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                                              &tmpCtrl3RegData);
    returnVal = BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
                                              &tmpAudioHFTest1RegData);

    returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,
                                 l1a_l1s_com.outen_cfg_task.outen1,  regInfoPtr++);
    count++;

    returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
                                   l1a_l1s_com.outen_cfg_task.outen2,  regInfoPtr++);
    count++;

    if(l1a_l1s_com.outen_cfg_task.classD == 0x01) // User wants to configure classD
    {
      if(triton_classD == 0x00) // User wants to switch on and Triton not configured for classD
      {

        BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,
                                                 0xb6,  regInfoPtr++);
        count++;


        returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                   l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
        count++;

        tmpCtrl3RegData |= 0x80;   // AUDIO_CTRL3_SPKDIGON
        tmpAudioHFTest1RegData = 0x01; // AUDIO_HFTEST1_SPKALLZB

        BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                                               tmpCtrl3RegData,  regInfoPtr++);
        count++;

        BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
                                               tmpAudioHFTest1RegData,  regInfoPtr++);
        count++;
      }
    }
    else if(l1a_l1s_com.outen_cfg_task.classD == 0x00)
    {
      if(triton_classD != 0x00) // User wants no to classD and Triton configured for classD
      {
	    tmpCtrl3RegData &= 0x7F;   // AUDIO_CTRL3_SPKDIGON
        tmpAudioHFTest1RegData = 0x00; // AUDIO_HFTEST1_SPKALLZB

	    BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_HFTEST1_OFFSET,
	 		                         tmpAudioHFTest1RegData,  regInfoPtr++);
        count++;

        BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,
                            tmpCtrl3RegData,  regInfoPtr++);
        count++;

        returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                                                 l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
        count++;

	    BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD, BSP_TWL_3029_MAP_CKG_TESTUNLOCK_OFFSET,
                                                 0x00,  regInfoPtr++);
        count++;

      }
      else // User no classD & Triton also no classD
      {
        returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
                            l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
	    count++;
	    returnVal = BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,
	                        l1a_l1s_com.outen_cfg_task.outen2,  regInfoPtr++);
	    count++;
	    returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_OUTEN3_OFFSET,
	                            l1a_l1s_com.outen_cfg_task.outen3,  regInfoPtr++);
	    count++;
      }
    }

 returnVal= BspTwl3029_I2c_regQueWrite(BSP_TWL3029_I2C_AUD,BSP_TWL_3029_MAP_AUDIO_POPMAN_OFFSET,
                           0x00,regInfoPtr++);
	count++;
    callbackPtr->callbackFunc =  l1_audio_abb_outen_cfg_callback;
    callbackPtr->callbackVal = 0;

    if (returnVal != BSP_TWL3029_RETURN_CODE_FAILURE)
    {
      regInfoPtr = regInfo;
      /* now request to I2C manager to write to Triton registers */

      returnVal = BspTwl3029_I2c_regInfoSend(regInfo,count,callbackPtr,
          	    (BspI2c_TransactionRequest*)i2cTransArrayPtr);
    }

    return returnVal;
  } /* end function l1s_outen_update */


#endif
#if (CODE_VERSION == SIMULATION)
  // This function is written to turn on the flag l1_audio_dl_switched_on during simulation
signed char l1_outen_update(void)
  {
      l1s.audio_on_off_ctl.l1_audio_dl_switched_on = TRUE;
  }
#endif
#endif // L1_AUDIO_MCU_ONOFF

// END Triton Audio ON/OFF Changes


  #if (KEYBEEP)
    /*-------------------------------------------------------*/
    /* l1s_keybeep_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Keybeep L1S manager task.               */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_keybeep_manager(void)
    {
      enum states
      {
        IDLE                 = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON        = 1,
#endif
        WAIT_KEYBEEP_START   = 2,
        WAIT_KEYBEEP_STOP    = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_KEYBEEP_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {

        case IDLE:
        {
          // Triton Audio ON/OFF Changes
#if(L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Start the DSP keybeep task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_KEYBEEP;
          *state = WAIT_KEYBEEP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
              // Start the DSP keybeep task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_KEYBEEP;
              *state = WAIT_KEYBEEP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_KEYBEEP_START:
        {
          // the DSP acknowledges the L1S start request.
          if ((!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init    & B_KEYBEEP)) &&
                (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status  & B_KEYBEEP))
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_KEYBEEP_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_KEYBEEP_STOP;
          }
        }
        break;

        case WAIT_KEYBEEP_STOP:
        {
          // the DSP is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_KEYBEEP))
          {
            // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_KEYBEEP_STOP_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
          else
          // the MMI requests to stop the current keybeep
          if (l1a_l1s_com.keybeep_task.command.stop)
          {
            // Stop the DSP keybeep task
            l1s_dsp_com.dsp_ndb_ptr->d_dur_kb = 0;
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_KEYBEEP;

            // Disable the stop command
            l1a_l1s_com.keybeep_task.command.stop = FALSE;
          }
        }
        break;
      } // switch
    }
  #endif // KEYBEEP
  #if (TONE)
    /*-------------------------------------------------------*/
    /* l1s_tone_manager()                                    */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Tone L1S manager task.                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_tone_manager(void)
    {
      enum states
      {
        IDLE              = 0,
// Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF== 1)
        WAIT_AUDIO_ON     = 1,
#endif
        WAIT_TONE_START   = 2,
        WAIT_TONE_STOP    = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_TONE_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Start the DSP tone task
          #if ((DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
            // Linked to E2 melody
            // In case of WCP, there is a WCP variable at this address
            l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = NO_MELODY_SELECTED;
          #endif
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init &= ~(B_MELO);
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_TONE;

          *state = WAIT_TONE_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {

          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
              // Start the DSP tone task
              #if ((DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
                // Linked to E2 melody
                // In case of WCP, there is a WCP variable at this address
                l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = NO_MELODY_SELECTED;
              #endif
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init &= ~(B_MELO);
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_TONE;

              *state = WAIT_TONE_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_TONE_START:
        {
          // the DSP acknowledges the L1S start request.
          if ((!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init    & B_TONE)) &&
                (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status  & B_TONE))
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_TONE_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_TONE_STOP;
          }
        }
        break;

        case WAIT_TONE_STOP:
        {
          // the DSP is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_TONE))
          {
           // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_TONE_STOP_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
          else
          // the MMI requests to stop the current tone
          if (l1a_l1s_com.tone_task.command.stop)
          {
            // Stop the DSP tone task
            l1s_dsp_com.dsp_ndb_ptr->d_pe_rep = 0;
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_TONE;

            // Disable the stop command
            l1a_l1s_com.tone_task.command.stop = FALSE;
          }
        }
        break;
      } // switch
    }
  #endif // TONE
  #if (MELODY_E1)
    /*-------------------------------------------------------*/
    /* l1s_melody0_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Melody 0 L1S manager task.              */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_melody0_manager(void)
    {
      enum states
      {
        M0_INACTIVE                 = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        M0_WAIT_AUDIO_START         = 1,
#endif
        M0_WAIT_DSP_START           = 2,
        M0_WAIT_COUNTER_EQUAL_0     = 3,
        M0_WAIT_DESCRIPTION_START   = 4,
        M0_WAIT_END_MELO            = 5
      };

      UWORD8            *state      = &l1s.audio_state[L1S_MELODY0_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            i, load_size;
      UWORD16           melo_header[2], trash[4];
      API               *osc_used;

      switch(*state)
      {
        case M0_INACTIVE:
        {
            // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = M0_WAIT_AUDIO_START;
#else
          // Initialize the oscilators used:
          for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
          {
            if (l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & (0x1<<i))
              *(l1s.melody0.oscillator[i]) = SC_END_OSCILLATOR_MASK;
          }
          // Initialize the pointer to the buffer, the buffer size
          l1s.melody0.ptr_buf = l1a_l1s_com.melody0_task.parameters.ptr_buf;
          l1s.melody0.buffer_size = l1a_l1s_com.melody0_task.parameters.buffer_size;

          // Download the header of the first description of the melody
          l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                        &l1s.melody0.buffer_size,
                                                        (UWORD16 **)&l1s.melody0.ptr_buf,
                                                        1,
                                                        &l1s.melody0.melody_header);

          // Initialize the counter with the first offset time:
          l1s.melody0.counter = ( ( Field(l1s.melody0.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

          // Enable the oscillator used
          l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used |= l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap;

          // Start the DSP melody task
          #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
            // Linked to E2 melody
            // In case of WCP, there is a WCP variable at this address
            l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E1_SELECTED;
          #endif
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_TONE | B_MELO);

          *state = M0_WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;
#if (L1_AUDIO_MCU_ONOFF == 1)
        case M0_WAIT_AUDIO_START:
        {
            // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
              // Initialize the oscilators used:
              for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
              {
                if (l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & (0x1<<i))
                  *(l1s.melody0.oscillator[i]) = SC_END_OSCILLATOR_MASK;
              }
              // Initialize the pointer to the buffer, the buffer size
              l1s.melody0.ptr_buf = l1a_l1s_com.melody0_task.parameters.ptr_buf;
              l1s.melody0.buffer_size = l1a_l1s_com.melody0_task.parameters.buffer_size;

              // Download the header of the first description of the melody
              l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                            &l1s.melody0.buffer_size,
                                                            (UWORD16 **)&l1s.melody0.ptr_buf,
                                                            1,
                                                            &l1s.melody0.melody_header);

              // Initialize the counter with the first offset time:
              l1s.melody0.counter = ( ( Field(l1s.melody0.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

              // Enable the oscillator used
              l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used |= l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap;

              // Start the DSP melody task
              #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
                // Linked to E2 melody
                // In case of WCP, there is a WCP variable at this address
                l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E1_SELECTED;
              #endif
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_TONE | B_MELO);

              *state = M0_WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case M0_WAIT_DSP_START:
        {
          // The DSP is started
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init & B_TONE) )
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_MELODY0_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = M0_WAIT_COUNTER_EQUAL_0;
          }
        }
        break;

        case M0_WAIT_COUNTER_EQUAL_0:
        {
          // The MMI resquests to stop the current melody 0.
          if (l1a_l1s_com.melody0_task.command.stop)
          {
            // Initialize the oscilators used:
            for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
            {
              if (l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & (0x1<<i))
                *(l1s.melody0.oscillator[i]) = SC_END_OSCILLATOR_MASK;
            }

            // Disable the loopback
            l1a_l1s_com.melody0_task.parameters.loopback = FALSE;

            // Disable the stop command
            l1a_l1s_com.melody0_task.command.stop = FALSE;

            *state = M0_WAIT_END_MELO;
          }
          else
          {
            // Decrease the download coundter
            l1s.melody0.counter--;

            // The description must be downloaded.
            if (l1s.melody0.counter == 0)
            {
              // Set the oscillator used in the following description
              l1s.melody0.oscillator_used_in_desc = Field(l1s.melody0.melody_header, SC_MELO_OSCILLATOR_USED_MASK, SC_MELO_OSCILLATOR_USED_SHIFT);
              l1s.melody0.oscillator_started = 0;

              // Download the new description
              for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
              {
                if (Field(l1s.melody0.oscillator_used_in_desc, (0x01<<i), i))
                {
                  // This oscillator description must be used.
                  if (l1a_l1s_com.melody0_task.parameters.melody_to_oscillator[i] != SC_NUMBER_OSCILLATOR)
                  {
                    osc_used = l1s.melody0.oscillator[l1a_l1s_com.melody0_task.parameters.melody_to_oscillator[i]];
                    l1s.melody0.oscillator_started |= (0x01<<(l1a_l1s_com.melody0_task.parameters.melody_to_oscillator[i]));
                  }
                  else
                  // The oscillator description isn't used and put to the "trash"
                  {
                    osc_used = trash;
                  }

                  // Download the two first words of the oscillator description
                  l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                                &l1s.melody0.buffer_size,
                                                                (UWORD16 **)&l1s.melody0.ptr_buf,
                                                                2,
                                                                osc_used);

                  load_size = 0;
                  if (Field(*(osc_used+1), SC_MELO_LOAD1_MASK, SC_MELO_LOAD1_SHIFT))
                    load_size++;
                  if (Field(*(osc_used+1), SC_MELO_LOAD2_MASK, SC_MELO_LOAD2_SHIFT))
                    load_size++;

                  // Download the next word(s) of the oscillator description
                  l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                                &l1s.melody0.buffer_size,
                                                                (UWORD16 **)&l1s.melody0.ptr_buf,
                                                                load_size,
                                                                osc_used+2);

                  // Enable this new description
                  *osc_used |= 1;
                }
              }

              *state = M0_WAIT_DESCRIPTION_START;
            }
          }
        }
        break;

        case M0_WAIT_DESCRIPTION_START:
        {

          // The new description is started or no oscillator of the description was allocated.
          if ((l1s.melody0.oscillator_started & l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active) ||
             (l1s.melody0.oscillator_started == 0) )
          {
            // Download the header of the next description of the melody
            l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                          &l1s.melody0.buffer_size,
                                                          (UWORD16 **)&l1s.melody0.ptr_buf,
                                                          1,
                                                          &l1s.melody0.melody_header);

            // Is it the end of the melody?
            if (l1s.melody0.melody_header == 0x0000)
            {
              *state = M0_WAIT_END_MELO;
              // Header is wrong - L1 needs a forcible stop here
#if (CODE_VERSION == SIMULATION)
	      // l1a_l1s_com.melody1_task.command.stop = TRUE;
#else
              l1a_l1s_com.melody1_task.command.stop = TRUE;
#endif
            }
            else
            {
              // Initialize the counter with the next offset time:
              l1s.melody0.counter = ( ( Field(l1s.melody0.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

              *state = M0_WAIT_COUNTER_EQUAL_0;
            }
          }
        }
        break;

        case M0_WAIT_END_MELO:
        {
          if (l1a_l1s_com.melody0_task.command.stop)
          {
            // Initialize the oscillators used:
            for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
            {
              if (l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & (0x1<<i))
                *(l1s.melody0.oscillator[i]) = SC_END_OSCILLATOR_MASK;
            }

            // Disable the loopback
            l1a_l1s_com.melody0_task.parameters.loopback = FALSE;

            // Disable the stop command
            l1a_l1s_com.melody0_task.command.stop = FALSE;

            *state = M0_WAIT_END_MELO;
          }
          else
          // All oscillators used are stopped.
          if (!( l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active))
          {
            // The melody is in loopback mode?
            if (l1a_l1s_com.melody0_task.parameters.loopback)
            {
              // Reset the buffer description
              #if (OP_RIV_AUDIO == 0)
              l1s.melody0.ptr_buf       = NULL;
              #endif
              l1s.melody0.buffer_size   = 0;
              l1s.melody0.error_id = Cust_get_pointer((UWORD16 **)&l1s.melody0.ptr_buf,
                                                      &l1s.melody0.buffer_size,
                                                      l1a_l1s_com.melody0_task.parameters.session_id);

              // Download the 2 first words of the melody
              l1s.melody0.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                            &l1s.melody0.buffer_size,
                                                            (UWORD16 **)&l1s.melody0.ptr_buf,
                                                            2,
                                                            (UWORD16 *)&melo_header);

              // Save the header of the first melody score description
              l1s.melody0.melody_header = melo_header[1];

              // Initialize the counter with the first offset time:
              l1s.melody0.counter = ( ( Field(l1s.melody0.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

              *state = M0_WAIT_COUNTER_EQUAL_0;
            }
            else
            {
              // Disable the oscillator dedicated to this melody
              l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used &= ~(l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap);

                // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
                l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif

              // Send the stop confirmation message
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_MELODY0_STOP_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              *state = M0_INACTIVE;
            }
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_melody1_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Melody 1 L1S manager task.              */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_melody1_manager(void)
    {
      enum states
      {
        M1_INACTIVE                 = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        M1_WAIT_AUDIO_START         = 1,
#endif
        M1_WAIT_DSP_START           = 2,
        M1_WAIT_COUNTER_EQUAL_0     = 3,
        M1_WAIT_DESCRIPTION_START   = 4,
        M1_WAIT_END_MELO            = 5
      };

      UWORD8            *state      = &l1s.audio_state[L1S_MELODY1_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            i, load_size;
      UWORD16           melo_header[2], trash[4];
      API               *osc_used;

      switch(*state)
      {
        case M1_INACTIVE:
        {
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = M1_WAIT_AUDIO_START;
#else
          // Initialize the oscilators used:
          for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
          {
            if (l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & (0x1<<i))
              *(l1s.melody1.oscillator[i]) = SC_END_OSCILLATOR_MASK;
          }
          // Initialize the pointer to the buffer, the buffer size
          l1s.melody1.ptr_buf = l1a_l1s_com.melody1_task.parameters.ptr_buf;
          l1s.melody1.buffer_size = l1a_l1s_com.melody1_task.parameters.buffer_size;

          // Download the header of the first description of the melody
          l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                        &l1s.melody1.buffer_size,
                                                        (UWORD16 **)&l1s.melody1.ptr_buf,
                                                        1,
                                                        &l1s.melody1.melody_header);

          // Initialize the counter with the first offset time:
          l1s.melody1.counter = ( ( Field(l1s.melody1.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

          // Enable the oscillator used
          l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used |= l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap;

          // Start the DSP melody task
          #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
            // Linked to E2 melody
            // In case of WCP, there is a WCP variable at this address
            l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E1_SELECTED;
          #endif
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_TONE | B_MELO);

          *state = M1_WAIT_DSP_START;
#endif
        }
        break;
#if (L1_AUDIO_MCU_ONOFF == 1)
        case M1_WAIT_AUDIO_START:
        {
            if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
            {
                  // Initialize the oscilators used:
                  for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
                  {
                    if (l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & (0x1<<i))
                      *(l1s.melody1.oscillator[i]) = SC_END_OSCILLATOR_MASK;
                  }
                  // Initialize the pointer to the buffer, the buffer size
                  l1s.melody1.ptr_buf = l1a_l1s_com.melody1_task.parameters.ptr_buf;
                  l1s.melody1.buffer_size = l1a_l1s_com.melody1_task.parameters.buffer_size;

                  // Download the header of the first description of the melody
                  l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                                &l1s.melody1.buffer_size,
                                                                (UWORD16 **)&l1s.melody1.ptr_buf,
                                                                1,
                                                                &l1s.melody1.melody_header);

                  // Initialize the counter with the first offset time:
                  l1s.melody1.counter = ( ( Field(l1s.melody1.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

                  // Enable the oscillator used
                  l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used |= l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap;

                  // Start the DSP melody task
                  #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
                    // Linked to E2 melody
                    // In case of WCP, there is a WCP variable at this address
                    l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = MELODY_E1_SELECTED;
                  #endif
                  l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (B_TONE | B_MELO);

                  *state = M1_WAIT_DSP_START;
            }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case M1_WAIT_DSP_START:
        {
          // The DSP is started
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init & B_TONE) )
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_MELODY1_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = M1_WAIT_COUNTER_EQUAL_0;
          }
        }
        break;

        case M1_WAIT_COUNTER_EQUAL_0:
        {
          // The MMI resquests to stop the current melody 1.
          if (l1a_l1s_com.melody1_task.command.stop)
          {
            // Initialize the oscilators used:
            for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
            {
              if (l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & (0x1<<i))
                *(l1s.melody1.oscillator[i]) = SC_END_OSCILLATOR_MASK;
            }

            // Disable the loopback
            l1a_l1s_com.melody1_task.parameters.loopback = FALSE;

            // Disable the stop command
            l1a_l1s_com.melody1_task.command.stop = FALSE;

            *state = M1_WAIT_END_MELO;
          }
          else
          {
            // Decrease the download coundter
            l1s.melody1.counter--;

            // The description must be downloaded.
            if (l1s.melody1.counter == 0)
            {
              // Set the oscillator used in the following description
              l1s.melody1.oscillator_used_in_desc = Field(l1s.melody1.melody_header, SC_MELO_OSCILLATOR_USED_MASK, SC_MELO_OSCILLATOR_USED_SHIFT);
              l1s.melody1.oscillator_started = 0;

              // Download the new description
              for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
              {
                if (Field(l1s.melody1.oscillator_used_in_desc, (0x01<<i), i))
                {
                  // This oscillator description must be used.
                  if (l1a_l1s_com.melody1_task.parameters.melody_to_oscillator[i] != SC_NUMBER_OSCILLATOR)
                  {
                    osc_used = l1s.melody1.oscillator[l1a_l1s_com.melody1_task.parameters.melody_to_oscillator[i]];
                    l1s.melody1.oscillator_started |= (0x01<<(l1a_l1s_com.melody1_task.parameters.melody_to_oscillator[i]));
                  }
                  else
                  // The oscillator description isn't used and put to the "trash"
                  {
                    osc_used = trash;
                  }

                  // Download the two first words of the oscillator description
                  l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                                &l1s.melody1.buffer_size,
                                                                (UWORD16 **)&l1s.melody1.ptr_buf,
                                                                2,
                                                                osc_used);

                  load_size = 0;
                  if (Field(*(osc_used+1), SC_MELO_LOAD1_MASK, SC_MELO_LOAD1_SHIFT))
                    load_size++;
                  if (Field(*(osc_used+1), SC_MELO_LOAD2_MASK, SC_MELO_LOAD2_SHIFT))
                    load_size++;

                  // Download the next word(s) of the oscillator description
                  l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                                &l1s.melody1.buffer_size,
                                                                (UWORD16 **)&l1s.melody1.ptr_buf,
                                                                load_size,
                                                                osc_used+2);

                  // Enable this new description
                  *osc_used |= 1;
                }
              }

              *state = M1_WAIT_DESCRIPTION_START;
            }
          }
        }
        break;

        case M1_WAIT_DESCRIPTION_START:
        {
          // The new description is started or no oscillator of the description was allocated.
          if ((l1s.melody1.oscillator_started & l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active) ||
             (l1s.melody1.oscillator_started == 0) )
          {
            // Download the header of the next description of the melody
            l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                          &l1s.melody1.buffer_size,
                                                          (UWORD16 **)&l1s.melody1.ptr_buf,
                                                          1,
                                                          &l1s.melody1.melody_header);

            // Is it the end of the melody?
            if (l1s.melody1.melody_header == 0x0000)
            {
              *state = M1_WAIT_END_MELO;
              /* Header is wrong - L1 needs a forcible stop here */
#if (CODE_VERSION == SIMULATION)
	      // l1a_l1s_com.melody1_task.command.stop = TRUE;
#else
	      l1a_l1s_com.melody1_task.command.stop = TRUE;
#endif

            }
            else
            {
              // Initialize the counter with the next offset time:
              l1s.melody1.counter = ( ( Field(l1s.melody1.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

              *state = M1_WAIT_COUNTER_EQUAL_0;
            }
          }
        }
        break;

        case M1_WAIT_END_MELO:
        {
          if (l1a_l1s_com.melody1_task.command.stop)
          {
            // Initialize the oscillators used:
            for (i=0; i<SC_NUMBER_OSCILLATOR; i++)
            {
              if (l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & (0x1<<i))
                *(l1s.melody1.oscillator[i]) = SC_END_OSCILLATOR_MASK;
            }

            // Disable the loopback
            l1a_l1s_com.melody1_task.parameters.loopback = FALSE;

            // Disable the stop command
            l1a_l1s_com.melody1_task.command.stop = FALSE;

            *state = M1_WAIT_END_MELO;
          }
          else
          // All oscillators used are stopped.
          if (!( l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active))
          {
            // The melody is in loopback mode?
            if (l1a_l1s_com.melody1_task.parameters.loopback)
            {
              // Reset the pointer to the buffer
              #if (OP_RIV_AUDIO == 0)
              l1s.melody1.ptr_buf       = NULL;
              #endif
              l1s.melody1.buffer_size   = 0;
              l1s.melody1.error_id = Cust_get_pointer((UWORD16 **)&l1s.melody1.ptr_buf,
                                                      &l1s.melody1.buffer_size,
                                                      l1a_l1s_com.melody1_task.parameters.session_id);

              // Download the 2 first words of the first description of the melody
              l1s.melody1.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                            &l1s.melody1.buffer_size,
                                                            (UWORD16 **)&l1s.melody1.ptr_buf,
                                                            2,
                                                            (UWORD16 *)&melo_header);

              // Save the header of the first melody score description
              l1s.melody1.melody_header = melo_header[1];

              // Initialize the counter with the first offset time:
              l1s.melody1.counter = ( ( Field(l1s.melody1.melody_header, SC_MELO_TIME_OFFSET_MASK, SC_MELO_TIME_OFFSET_SHIFT) ) * SC_MELO_DOWNLOAD_TIME_UNIT ) - 1;

              *state = M1_WAIT_COUNTER_EQUAL_0;
            }
            else
            {
              // Disable the oscillator dedicated to this melody
              l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used &= ~(l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap);
#if (L1_AUDIO_MCU_ONOFF == 1)
              l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif

              // Send the stop confirmation message
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_MELODY1_STOP_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              *state = M1_INACTIVE;
            }
          }
        }
        break;
      } // switch
    }
  #endif // MELODY_E1
  #if (VOICE_MEMO)
    /*-------------------------------------------------------*/
    /* l1s_vm_play_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Voice memo playing L1S manager task.    */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_vm_play_manager(void)
    {
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif
        WAIT_DSP_START    = 2,
        WAIT_DSP_REQUEST  = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_VM_PLAY_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD16           sample_header;

      switch(*state)
      {
        case IDLE:
        {
#if (L1_AUDIO_MCU_ONOFF == 1)
          // Triton Audio ON/OFF Changes
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.voicememo.play.ptr_buf        = NULL;
          l1s.voicememo.play.buffer_size    = 0;
          l1s.voicememo.play.error_id = Cust_get_pointer((UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                         &l1s.voicememo.play.buffer_size,
                                                         l1a_l1s_com.voicememo_task.play.parameters.session_id);

          // Start the voice memo playing DSP task
        #if ((DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
          // use TCH/FS vocoder
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VM_VOCODER_SELECT);
        #endif
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_PLAY_START;

          // Determine which a_du buffer is currently used
          l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
          if (l1a_l1s_com.dedic_set.aset != NULL)
          {
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
          }

          // Download the header of the new speech sample
          l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                               &l1s.voicememo.play.buffer_size,
                                                               (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                               1,
                                                               &sample_header);

          // Download the data to the a_du_x buffer if the sample isn't a noise sample.
          if (sample_header & B_VM_SPEECH)
          {
            l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                                 &l1s.voicememo.play.buffer_size,
                                                                 (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                                 SC_VM_SPEECH_SAMPLE-1,
                                                                 l1s.voicememo.play.a_du_x+1);
          }
          // Send the header to the DSP
          *l1s.voicememo.play.a_du_x = sample_header;

          *state = WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;
#if  (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
              // Initialize the buffer parameters
              l1s.voicememo.play.ptr_buf        = NULL;
              l1s.voicememo.play.buffer_size    = 0;
              l1s.voicememo.play.error_id = Cust_get_pointer((UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                             &l1s.voicememo.play.buffer_size,
                                                             l1a_l1s_com.voicememo_task.play.parameters.session_id);

              // Start the voice memo playing DSP task
            #if (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
              // use TCH/FS vocoder
              l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VM_VOCODER_SELECT);
            #endif
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_PLAY_START;

              // Determine which a_du buffer is currently used
              l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
              if (l1a_l1s_com.dedic_set.aset != NULL)
              {
                if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                     (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                    l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
              }

              // Download the header of the new speech sample
              l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                                   &l1s.voicememo.play.buffer_size,
                                                                   (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                                   1,
                                                                   &sample_header);

              // Download the data to the a_du_x buffer if the sample isn't a noise sample.
              if (sample_header & B_VM_SPEECH)
              {
                l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                                     &l1s.voicememo.play.buffer_size,
                                                                     (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                                     SC_VM_SPEECH_SAMPLE-1,
                                                                     l1s.voicememo.play.a_du_x+1);
              }
              // Send the header to the DSP
              *l1s.voicememo.play.a_du_x = sample_header;

              *state = WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_PLAY_ON_GOING)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_PLAY_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

           *state = WAIT_DSP_REQUEST;
          }
        }
        break;

        case WAIT_DSP_REQUEST:
        {
          // The MMI requests to stop the voice memorization playing task
          if (l1a_l1s_com.voicememo_task.play.command.stop)
          {
            // Stop the DSP voice memorization playing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_PLAY_STOP;

            *state = WAIT_DSP_STOP;
          }
          else
          // The DSP needs a new block
          {
            // Determine which a_du buffer is currently used
            l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
            if (l1a_l1s_com.dedic_set.aset != NULL)
            {
              if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                   (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                  l1s.voicememo.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
            }

            if (!(*(l1s.voicememo.play.a_du_x) & B_BLOCK_READY))
            {
              // Download the header of the new speech sample
              l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                                   &l1s.voicememo.play.buffer_size,
                                                                   (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                                   1,
                                                                   &sample_header);

              // Is it the end of the voice memo data buffer?
              if ( sample_header == SC_VM_END_MASK )
              {
                // Stop the DSP voice memorization playing task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_PLAY_STOP;

                *state = WAIT_DSP_STOP;
              }
              else
              {
                // Download the data to the a_du_x buffer if the sample isn't a noise sample.
                if (sample_header & B_VM_SPEECH)
                {
                  l1s.voicememo.play.error_id = copy_data_from_buffer (l1a_l1s_com.voicememo_task.play.parameters.session_id,
                                                                       &l1s.voicememo.play.buffer_size,
                                                                       (UWORD16 **)&l1s.voicememo.play.ptr_buf,
                                                                       SC_VM_SPEECH_SAMPLE-1,
                                                                       l1s.voicememo.play.a_du_x+1);
                }
                // Send the header to the DSP
                *l1s.voicememo.play.a_du_x = sample_header;
              }
            }
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP voice memorization playing task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_PLAY_ON_GOING))
          {
            // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_PLAY_STOP_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_vm_record_manager()                               */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Voice memo recoding L1S manager task.   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_vm_record_manager(void)
    {
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif // L1_AUDIO_MCU_ONOFF
        WAIT_DSP_START    = 2,
        WAIT_DSP_SAMPLE   = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_VM_RECORD_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            size;
      UWORD16           data;

      switch(*state)
      {
        case IDLE:
        {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.voicememo.record.ptr_buf        = NULL;
          l1s.voicememo.record.buffer_size    = 0;
          l1s.voicememo.record.error_id = Cust_get_pointer((UWORD16 **)&l1s.voicememo.record.ptr_buf,
                                                           &l1s.voicememo.record.buffer_size,
                                                           l1a_l1s_com.voicememo_task.record.parameters.session_id);

          // Initialize the size of the Voice memo to record
          l1s.voicememo.record.recorded_size = 0;

          // Initialize the DTX mode
          if (l1a_l1s_com.voicememo_task.record.parameters.dtx)
            l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_VOICE_MEMO_DTX;
          else
            l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VOICE_MEMO_DTX);

        #if ((DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
          // use TCH/FS vocoder
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VM_VOCODER_SELECT);
        #endif

          // Start the voice memo recording DSP task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_START;

          *state = WAIT_DSP_START;

#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if  (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE] == L1_AUDIO_UL_ON))
          {
              // Initialize the buffer parameters
              l1s.voicememo.record.ptr_buf        = NULL;
              l1s.voicememo.record.buffer_size    = 0;
              l1s.voicememo.record.error_id = Cust_get_pointer((UWORD16 **)&l1s.voicememo.record.ptr_buf,
                                                               &l1s.voicememo.record.buffer_size,
                                                               l1a_l1s_com.voicememo_task.record.parameters.session_id);

              // Initialize the size of the Voice memo to record
              l1s.voicememo.record.recorded_size = 0;

              // Initialize the DTX mode
              if (l1a_l1s_com.voicememo_task.record.parameters.dtx)
                l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_VOICE_MEMO_DTX;
              else
                l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VOICE_MEMO_DTX);

            #if (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
              // use TCH/FS vocoder
              l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VM_VOCODER_SELECT);
            #endif

              // Start the voice memo recording DSP task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_START;

              *state = WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_RECORD_ON_GOING)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_RECORD_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_DSP_SAMPLE;
          }
        }
        break;

        case WAIT_DSP_SAMPLE:
        {
          // The MMI requests to stop the voice memorization recording task
          if (l1a_l1s_com.voicememo_task.record.command.stop)
          {
            // Write the end mask at the end of the voice data RAM buffer
            data = SC_VM_END_MASK;
            l1s.voicememo.record.error_id = copy_data_to_buffer (l1a_l1s_com.voicememo_task.record.parameters.session_id,
                                                                 &l1s.voicememo.record.buffer_size,
                                                                 (UWORD16 **)&l1s.voicememo.record.ptr_buf,
                                                                 1,
                                                                 &data);

            // Increase the recorded size
            l1s.voicememo.record.recorded_size++;

            // Stop the DSP voice memorization playing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_STOP;

            // Clear the a_du_x header:
            *(l1s.voicememo.record.a_du_x) = 0;

            *state = WAIT_DSP_STOP;
          }
          else
          // The DSP needs a new block
          {
            // Determine which a_du buffer is currently used
            l1s.voicememo.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
            if (l1a_l1s_com.dedic_set.aset != NULL)
            {
              if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                   (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                  l1s.voicememo.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
            }

            // The DSP sends a new block?
            if ( (*(l1s.voicememo.record.a_du_x)) & B_BLOCK_READY )
            {
              // Check if the block contains a sample of noise or speech
              if ( (*(l1s.voicememo.record.a_du_x)) & B_VM_SPEECH )
                size = SC_VM_SPEECH_SAMPLE;
              else
                size = SC_VM_NOISE_SAMPLE;

              // The maximum allocated size is reached?
              if ( (l1s.voicememo.record.recorded_size+size+1) <= l1a_l1s_com.voicememo_task.record.parameters.maximum_size)
              {
                // Download the data to the a_du_x buffer.
                l1s.voicememo.record.error_id = copy_data_to_buffer (l1a_l1s_com.voicememo_task.record.parameters.session_id,
                                                                     &l1s.voicememo.record.buffer_size,
                                                                     (UWORD16 **)&l1s.voicememo.record.ptr_buf,
                                                                     size,
                                                                     l1s.voicememo.record.a_du_x);

                // Increase the recorded size
                l1s.voicememo.record.recorded_size += size;

                // Clear the a_du_x header:
                *(l1s.voicememo.record.a_du_x) = 0;
              }
              else
              {
                // Write the end mask at the end of the voice data RAM buffer
                data = SC_VM_END_MASK;
                l1s.voicememo.record.error_id = copy_data_to_buffer (l1a_l1s_com.voicememo_task.record.parameters.session_id,
                                                                     &l1s.voicememo.record.buffer_size,
                                                                     (UWORD16 **)&l1s.voicememo.record.ptr_buf,
                                                                     1,
                                                                     &data);

                // Increase the recorded size
                l1s.voicememo.record.recorded_size++;

                // Stop the DSP voice memorization playing task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_STOP;

                // Clear the a_du_x header:
                *(l1s.voicememo.record.a_du_x) = 0;

                *state = WAIT_DSP_STOP;
              }
            }
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP voice memorization playing task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_RECORD_ON_GOING))
          {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_VM_RECORD_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_RECORD_STOP_CON;
            //Fill the message
            ((T_L1_VM_RECORD_CON *)(conf_msg->SigP))->recorded_size = l1s.voicememo.record.recorded_size;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_tone_ul_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : tone uplink L1S manager task.           */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_tone_ul_manager(void)
    {
      enum states
      {
        IDLE                    = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON           = 1,
#endif
        WAIT_DEDIC_SPEECH_MODE  = 2,
        WAIT_TONE_UL_START      = 3,
        WAIT_TONE_UL_STOP       = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_TONE_UL_STATE];

      switch(*state)
      {
        case IDLE:
        {
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
            *state = WAIT_AUDIO_ON;
#else
          // Start the tone uplink task
          if (l1a_l1s_com.voicememo_task.record.tone_ul.start)
          {
            // Set the tone uplink option:
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_TONE_UL;
          }

          *state = WAIT_DEDIC_SPEECH_MODE;
#endif
        }
        break;

#if (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
            if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
            {
                  // Start the tone uplink task
                  if (l1a_l1s_com.voicememo_task.record.tone_ul.start)
                  {
                    // Set the tone uplink option:
                    l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_TONE_UL;
                  }

                  *state = WAIT_DEDIC_SPEECH_MODE;
            }
        }
        break;
#endif // (L1_AUDIO_MCU_ONOFF == 1)

        case WAIT_DEDIC_SPEECH_MODE:
        {
          // The voice memorization task is stopping:
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init & B_VM_TONE_UL))
          {
            // Reset the start command
            l1a_l1s_com.voicememo_task.record.tone_ul.start = FALSE;
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF
            *state = IDLE;
          }
          else
          // Dedicated mode speech start?
          if (l1a_l1s_com.dedic_set.aset != NULL)
          {
          #if (AMR == 1)
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_HS_MODE)   ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_EFR_MODE)  ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_FS_MODE)   ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AFS_MODE)  ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AHS_MODE) )
          #else
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_HS_MODE)   ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_EFR_MODE)  ||
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_FS_MODE) )
          #endif
            {

              // Start the tone uplink DSP task
              #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
                // Linked to E2 melody
                // In case of WCP, there is a WCP variable at this address
                l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = NO_MELODY_SELECTED;
              #endif
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init &= ~(B_MELO);
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_TONE;

              *state = WAIT_TONE_UL_START;
            }
          }
        }
        break;

        case WAIT_TONE_UL_START:
        {
          // the tone uplink task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_TONE)
          {
            *state = WAIT_TONE_UL_STOP;
          }
        }
        break;

        case WAIT_TONE_UL_STOP:
        {
          // The voice memorization task is stopping:
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init & B_VM_TONE_UL))
          {
            // Stop the tone uplink task:
            l1s_dsp_com.dsp_ndb_ptr->d_pe_rep = 0;
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_TONE;
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF
            *state = IDLE;
          }
          else
          // The tone uplink task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_TONE))
          {
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF
            *state = IDLE;
          }
        }
        break;
      } // switch
    }

  #endif // VOICE_MEMO

  #if (L1_PCM_EXTRACTION)
    /*-------------------------------------------------------*/
    /* l1s_pcm_download_manager()                            */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : PCM download L1S manager task.          */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_pcm_download_manager(void)
    {
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif
        WAIT_DSP_START    = 2,
        WAIT_DSP_REQUEST  = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_PCM_DOWNLOAD_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
#if (L1_AUDIO_MCU_ONOFF == 1)
          // Triton Audio ON/OFF Changes
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.pcm.download.ptr_buf        = NULL;
          l1s.pcm.download.buffer_size    = 0;
          l1s.pcm.download.error_id = Cust_get_pointer((UWORD16 **)&l1s.pcm.download.ptr_buf,
                                                         &l1s.pcm.download.buffer_size,
                                                         l1a_l1s_com.pcm_task.download.parameters.session_id);

          // Download the PCM samples
          l1s.pcm.download.error_id = copy_data_from_buffer (l1a_l1s_com.pcm_task.download.parameters.session_id,
                                                               &l1s.pcm.download.buffer_size,
                                                               (UWORD16 **)&l1s.pcm.download.ptr_buf,
                                                               SC_PCM_DOWNLOAD_SAMPLE,
                                                               l1s_dsp_com.dsp_ndb_ptr->a_pcm_api_download);

          // Increase the downloaded size of PCM samples
          l1s.pcm.download.downloaded_size = SC_PCM_DOWNLOAD_SAMPLE;

          l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download |= (B_PCM_DOWNLOAD_READY);
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_DOWNLOAD_START;

          *state = WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if  (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
          // Initialize the buffer parameters
          l1s.pcm.download.ptr_buf        = NULL;
          l1s.pcm.download.buffer_size    = 0;
          l1s.pcm.download.error_id = Cust_get_pointer((UWORD16 **)&l1s.pcm.download.ptr_buf,
                                                         &l1s.pcm.download.buffer_size,
                                                         l1a_l1s_com.pcm_task.download.parameters.session_id);

          // Download the PCM samples
          l1s.pcm.download.error_id = copy_data_from_buffer (l1a_l1s_com.pcm_task.download.parameters.session_id,
                                                               &l1s.pcm.download.buffer_size,
                                                               (UWORD16 **)&l1s.pcm.download.ptr_buf,
                                                               SC_PCM_DOWNLOAD_SAMPLE,
                                                               l1s_dsp_com.dsp_ndb_ptr->a_pcm_api_download);

          // Increase the downloaded size of PCM samples
          l1s.pcm.download.downloaded_size = SC_PCM_DOWNLOAD_SAMPLE;

          l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download |= (B_PCM_DOWNLOAD_READY);
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_DOWNLOAD_START;

          *state = WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF


        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_PCM_DOWNLOAD_ON_GOING)
          {
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_PCM_DOWNLOAD_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_DSP_REQUEST;
          }
        }
        break;

        case WAIT_DSP_REQUEST:
        {
          if (l1a_l1s_com.pcm_task.download.command.stop)
          {
            // Stop the DSP PCM playing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_DOWNLOAD_STOP;
            l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download = 0;
            *state = WAIT_DSP_STOP;
          }
          else
          {
            // B_PCM_DOWNLOAD_READY is reset if DSP is ready to get new sample
            if (!(l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download & B_PCM_DOWNLOAD_READY))
            {

              if(l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_error &  B_PCM_DOWNLOAD_ERROR)
              {
				#if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                l1_trace_PCM_DSP_error();
                #endif
              } /* end if DSP error check - underflow */

              if ((l1s.pcm.download.downloaded_size + SC_PCM_DOWNLOAD_SAMPLE) <=
                    l1a_l1s_com.pcm_task.download.parameters.maximum_size)
              {

                // Download the data to the a_pcm_api_download buffer
                l1s.pcm.download.error_id = copy_data_from_buffer (l1a_l1s_com.pcm_task.download.parameters.session_id,
                                                                       &l1s.pcm.download.buffer_size,
                                                                       (UWORD16 **)&l1s.pcm.download.ptr_buf,
                                                                       SC_PCM_DOWNLOAD_SAMPLE,
                                                                       l1s_dsp_com.dsp_ndb_ptr->a_pcm_api_download);
                // Increase the downloaded size
                l1s.pcm.download.downloaded_size += SC_PCM_DOWNLOAD_SAMPLE;

                l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download |= (B_PCM_DOWNLOAD_READY);
              }  /* end if download speech sample*/
              else
              {
                // Stop the DSP PCM playing task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_DOWNLOAD_STOP;
                l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download = 0;
                *state = WAIT_DSP_STOP;
              } /* end else - download buffer size reached */
            } /* end if DSP requested new block */
          } /* end else not download task stop command */
        }
        break;

        case WAIT_DSP_STOP:
        {
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_PCM_DOWNLOAD_ON_GOING))
          {
            // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF

            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_PCM_DOWNLOAD_STOP_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_pcm_upload_manager()                              */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : PCM uploading manager task.             */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_pcm_upload_manager(void)
    {
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif // L1_AUDIO_MCU_ONOFF
        WAIT_DSP_START    = 2,
        WAIT_DSP_SAMPLE   = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_PCM_UPLOAD_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.pcm.upload.ptr_buf        = NULL;
          l1s.pcm.upload.buffer_size    = 0;
          l1s.pcm.upload.error_id = Cust_get_pointer((UWORD16 **)&l1s.pcm.upload.ptr_buf,
                                                           &l1s.pcm.upload.buffer_size,
                                                           l1a_l1s_com.pcm_task.upload.parameters.session_id);

          // Initialize the size of the PCM upload
          l1s.pcm.upload.uploaded_size = 0;

          // Start the PCM recording DSP task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_UPLOAD_START;

          *state = WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if  (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE] == L1_AUDIO_UL_ON))
          {
          // Initialize the buffer parameters
          l1s.pcm.upload.ptr_buf        = NULL;
          l1s.pcm.upload.buffer_size    = 0;
          l1s.pcm.upload.error_id = Cust_get_pointer((UWORD16 **)&l1s.pcm.upload.ptr_buf,
                                                           &l1s.pcm.upload.buffer_size,
                                                           l1a_l1s_com.pcm_task.upload.parameters.session_id);

          // Initialize the size of the PCM upload
          l1s.pcm.upload.uploaded_size = 0;

          // Start the PCM recording DSP task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_UPLOAD_START;

          *state = WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF


        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_PCM_UPLOAD_ON_GOING)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_PCM_UPLOAD_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_DSP_SAMPLE;
          }
        }
        break;

        case WAIT_DSP_SAMPLE:
        {
          // The MMI requests to stop the PCM recording task
          if (l1a_l1s_com.pcm_task.upload.command.stop)
          {
            // Stop the DSP PCM recording task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_UPLOAD_STOP;
            l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload = 0;

            *state = WAIT_DSP_STOP;
          }
          else
          // The DSP needs a new block
          {

            // The DSP sends a new block?
            if (l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload & B_PCM_UPLOAD_READY )
            {

              if(l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_error &  B_PCM_UPLOAD_ERROR)
              {
				#if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                l1_trace_PCM_DSP_error();
                #endif
              } /* end if DSP error check - overflow */

              if ((l1s.pcm.upload.uploaded_size + SC_PCM_UPLOAD_SAMPLE) <= l1a_l1s_com.pcm_task.upload.parameters.maximum_size)
              {
                // Download the data to the a_pcm_api_upload buffer.

                l1s.pcm.upload.error_id = copy_data_to_buffer (l1a_l1s_com.pcm_task.upload.parameters.session_id,
                                                                     &l1s.pcm.upload.buffer_size,
                                                                     (UWORD16 **)&l1s.pcm.upload.ptr_buf,
                                                                     SC_PCM_UPLOAD_SAMPLE,
                                                                     l1s_dsp_com.dsp_ndb_ptr->a_pcm_api_upload);

                // Increase the recorded size
                l1s.pcm.upload.uploaded_size += SC_PCM_UPLOAD_SAMPLE;

                // Clear the d_pcm_api_upload header
                l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload &= (~B_PCM_UPLOAD_READY);
              }
              else
              {
                // Stop the DSP PCM recording task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_PCM_UPLOAD_STOP;
                l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload = 0;

                *state = WAIT_DSP_STOP;
              } /* end else maximum uplaod size reached */
            } /* end if DSP sends a new block */
          } /* end else */
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP PCM upload task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_PCM_UPLOAD_ON_GOING))
          {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_PCM_UPLOAD_STOP_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_PCM_UPLOAD_STOP_CON;
            //Fill the message
            ((T_L1_PCM_UPLOAD_STOP_CON *)(conf_msg->SigP))->uploaded_size = l1s.pcm.upload.uploaded_size;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

  #endif /* L1_PCM_EXTRACTION */

  #if (L1_VOICE_MEMO_AMR)
    /*-------------------------------------------------------*/
    /* l1s_vm_amr_play_manager()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Voice memo amr playing L1S manager task.*/
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_vm_amr_play_manager(void)
    {
      BOOL l1_vm_amr_in_pause=FALSE;
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif
        WAIT_DSP_START    = 2,
        WAIT_DSP_REQUEST  = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_VM_AMR_PLAY_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            sample_header;

      switch(*state)
      {
        case IDLE:
        {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.voicememo_amr.play.ptr_buf        = NULL;
          l1s.voicememo_amr.play.buffer_size    = 0;
          l1s.voicememo_amr.play.error_id       = Cust_get_pointer((UWORD16 **)&l1s.voicememo_amr.play.ptr_buf,
                                                         &l1s.voicememo_amr.play.buffer_size,
                                                         l1a_l1s_com.voicememo_amr_task.play.parameters.session_id);

          // Convert the buffer size in bytes unit because VM AMR is defined in byte unit
          l1s.voicememo_amr.play.buffer_size <<= 1;

          // Initialize previous sample parameters in order to create ONSET on first SAMPLE
          l1s.voicememo_amr.play.previous_type     = SC_VM_AMR_NO_DATA;
          l1s.voicememo_amr.play.transition_header = 0;

          // Initialize a_du_x
          // Determine which a_du buffer is currently used
          l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
          if (l1a_l1s_com.dedic_set.aset != NULL)
          {
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
          }
          *(l1s.voicememo_amr.play.a_du_x) = 0;

          // Start the voice memo playing DSP task
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode     |= B_VM_VOCODER_SELECT;
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_PLAY_START;

          *state = WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }

#if (L1_AUDIO_MCU_ONOFF == 1)
        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON))
          {
              // Initialize the buffer parameters
              l1s.voicememo_amr.play.ptr_buf        = NULL;
              l1s.voicememo_amr.play.buffer_size    = 0;
              l1s.voicememo_amr.play.error_id       = Cust_get_pointer((UWORD16 **)&l1s.voicememo_amr.play.ptr_buf,
                                                             &l1s.voicememo_amr.play.buffer_size,
                                                             l1a_l1s_com.voicememo_amr_task.play.parameters.session_id);

              // Convert the buffer size in bytes unit because VM AMR is defined in byte unit
              l1s.voicememo_amr.play.buffer_size <<= 1;

              // Initialize previous sample parameters in order to create ONSET on first SAMPLE
              l1s.voicememo_amr.play.previous_type     = SC_VM_AMR_NO_DATA;
              l1s.voicememo_amr.play.transition_header = 0;

              // Initialize a_du_x
              // Determine which a_du buffer is currently used
              l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
              if (l1a_l1s_com.dedic_set.aset != NULL)
              {
                if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                     (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                    l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
              }
              *(l1s.voicememo_amr.play.a_du_x) = 0;

              // Start the voice memo playing DSP task
              l1s_dsp_com.dsp_ndb_ptr->d_tch_mode     |= B_VM_VOCODER_SELECT;
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_PLAY_START;

              *state = WAIT_DSP_START;
          }
        }
        break;
#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_AMR_PLAY_ON_GOING)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_AMR_PLAY_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

           *state = WAIT_DSP_REQUEST;
          }
        }
        break;

        case WAIT_DSP_REQUEST:
        {
          // The MMI requests to stop the voice memorization playing task
          if (l1a_l1s_com.voicememo_amr_task.play.command.stop)
          {
            // Stop the DSP voice memorization playing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_PLAY_STOP;

            *state = WAIT_DSP_STOP;
          }
          else
          // The DSP needs a new block ?
          {
         if (l1a_l1s_com.voicememo_amr_task.play.command.pause)
          {
            // Stop the DSP voice memorization playing task
            *(l1s.voicememo_amr.play.a_du_x) =*(l1s.voicememo_amr.play.a_du_x) | B_BLOCK_READY;
           l1_vm_amr_in_pause= TRUE;
             conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_AMR_PAUSE_CON;
           // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)
           //  *state =WAIT_DSP_REQUEST;

          }
           if (l1a_l1s_com.voicememo_amr_task.play.command.resume)
           {
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_VM_AMR_RESUME_CON;
            // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          }
            // Determine which a_du buffer is currently used
            l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
            if (l1a_l1s_com.dedic_set.aset != NULL)
            {
              if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                   (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                  l1s.voicememo_amr.play.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
            }

            // B_BLOCK_READY is not set if DSP is ready to get new sample
            if (!(*(l1s.voicememo_amr.play.a_du_x) & B_BLOCK_READY))
            {
              // Issue: transition from not speech sample to speech sample requires creation of transition ONSET sample + storing temporarily speech header
              // We use it on next DSP request => when previous sample was ONSET, header must be taken from transition_header
              if (l1s.voicememo_amr.play.previous_type == SC_VM_AMR_ONSET)
              {
                // we use speech header temprarily stored in transition_header
                sample_header = l1s.voicememo_amr.play.transition_header;
              }
              else
              {
                // Download the header of the new sample
                l1s.voicememo_amr.play.error_id = copy_byte_data_from_buffer (l1a_l1s_com.voicememo_amr_task.play.parameters.session_id,
                                                                     &l1s.voicememo_amr.play.buffer_size,
                                                                     (UWORD8 **)&l1s.voicememo_amr.play.ptr_buf,
                                                                     1,
                                                                     &sample_header);
              }

              // Is it the end of the voice memo data buffer?
              if ( sample_header == SC_VM_AMR_END_MASK )
              {
                // Stop the DSP voice memorization playing task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_PLAY_STOP;

                *state = WAIT_DSP_STOP;
              }
              else
              {
                UWORD8 temp_header =0;  //omaps00090550
                UWORD8  data_size =0;    //omaps00090550

                // Identify AMR sample RX_TX_TYPE
                temp_header = sample_header & SC_RX_TX_TYPE_MASK;
                switch(temp_header)
                {
                  case SC_VM_AMR_RXTX_SPEECH_GOOD:
                  case SC_VM_AMR_RXTX_SPEECH_BAD:
                  {
                    // Check if previous sample is a non speech sample so we have to create ONSET sample
                    if ( (l1s.voicememo_amr.play.previous_type == SC_VM_AMR_NOISE)||(l1s.voicememo_amr.play.previous_type == SC_VM_AMR_NO_DATA) )
                    {
                      l1s.voicememo_amr.play.previous_type     = SC_VM_AMR_ONSET;
                      l1s.voicememo_amr.play.transition_header = sample_header;
                      sample_header = SC_VM_AMR_RXTX_ONSET;
                      data_size = SC_VM_AMR_ONSET_DATA_SIZE;
                    }
                    else
                    {
                      l1s.voicememo_amr.play.previous_type = SC_VM_AMR_SPEECH;

                      // read channel type to know which vocoder is used (and size of data bits)
                      temp_header = sample_header & SC_CHAN_TYPE_MASK;
                      switch(temp_header)
                      {
                        case SC_VM_AMR_SPEECH_475:
                          data_size = SC_VM_AMR_SPEECH_475_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_515:
                          data_size = SC_VM_AMR_SPEECH_515_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_59:
                          data_size = SC_VM_AMR_SPEECH_59_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_67:
                          data_size = SC_VM_AMR_SPEECH_67_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_74:
                          data_size = SC_VM_AMR_SPEECH_74_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_795:
                          data_size = SC_VM_AMR_SPEECH_795_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_102:
                          data_size = SC_VM_AMR_SPEECH_102_DATA_SIZE;
                        break;
                        case SC_VM_AMR_SPEECH_122:
                          data_size = SC_VM_AMR_SPEECH_122_DATA_SIZE;
                        break;
                      } // switch(temp_header)
                    }
                  }
                  break;
                  case SC_VM_AMR_RXTX_SID_FIRST:
                    data_size = SC_VM_AMR_SID_FIRST_DATA_SIZE;
                    l1s.voicememo_amr.play.previous_type = SC_VM_AMR_NOISE;
                  break;
                  case SC_VM_AMR_RXTX_SID_UPDATE:
                    data_size = SC_VM_AMR_SID_UPDATE_DATA_SIZE;
                    l1s.voicememo_amr.play.previous_type = SC_VM_AMR_NOISE;
                  break;
                  case SC_VM_AMR_RXTX_SID_BAD:
                    data_size = SC_VM_AMR_SID_BAD_DATA_SIZE;
                    l1s.voicememo_amr.play.previous_type = SC_VM_AMR_NOISE;
                  break;
                  case SC_VM_AMR_RXTX_NO_DATA:
                    data_size = SC_VM_AMR_NO_DATA_DATA_SIZE;
                    l1s.voicememo_amr.play.previous_type = SC_VM_AMR_NO_DATA;
                  break;
                  default:
                    // trace error
                  break;
                }

                // if data_size is 0 (SID_FIRST, NO_DATA and ONSET), nothing to copy
                if (data_size > 0)
                {
                  // go beyond the 2 DSP words after the header, which are not used in MMS (so a_du_x + 3 in words)
                  l1s.voicememo_amr.play.error_id = copy_byte_data_le_from_buffer (l1a_l1s_com.voicememo_amr_task.play.parameters.session_id,
                                                                       &l1s.voicememo_amr.play.buffer_size,
                                                                       (UWORD8 **)&l1s.voicememo_amr.play.ptr_buf,
                                                                       data_size,
                                                                       l1s.voicememo_amr.play.a_du_x + 3);
                }
                // Send the header to the DSP
                *l1s.voicememo_amr.play.a_du_x = (sample_header | B_BLOCK_READY);
              }
            }
        if(l1_vm_amr_in_pause==TRUE)
         {
           (*(l1s.voicememo_amr.play.a_du_x) = *(l1s.voicememo_amr.play.a_du_x) & ~B_BLOCK_READY);
          l1_vm_amr_in_pause=FALSE;
          }
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP voice memorization playing task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_AMR_PLAY_ON_GOING))
          {
            // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
            l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_AMR_PLAY_STOP_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_vm_amr_record_manager()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Voice memo amr recoding L1S manager task*/
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_vm_amr_record_manager(void)
    {
      enum states
      {
        IDLE              = 0,
#if (L1_AUDIO_MCU_ONOFF == 1)
        WAIT_AUDIO_ON     = 1,
#endif
        WAIT_DSP_START    = 2,
        WAIT_DSP_SAMPLE   = 3,
        WAIT_DSP_STOP     = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_VM_AMR_RECORD_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            sample_header;

      switch(*state)
      {
        case IDLE:
        {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
          *state = WAIT_AUDIO_ON;
#else
          // Initialize the buffer parameters
          l1s.voicememo_amr.record.ptr_buf        = NULL;
          l1s.voicememo_amr.record.buffer_size    = 0;
          l1s.voicememo_amr.record.error_id       = Cust_get_pointer((UWORD16 **)&l1s.voicememo_amr.record.ptr_buf,
                                                           &l1s.voicememo_amr.record.buffer_size,
                                                           l1a_l1s_com.voicememo_amr_task.record.parameters.session_id);

          // Convert the buffer size in bytes unit because VM AMR is defined in byte unit
          l1s.voicememo_amr.record.buffer_size <<= 1;

          // Initialize the size of the Voice memo to record
          l1s.voicememo_amr.record.recorded_size = 0;

          // Initialize the DTX mode
          if (l1a_l1s_com.voicememo_amr_task.record.parameters.dtx)
            l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_VOICE_MEMO_DTX;
          else
            l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VOICE_MEMO_DTX);

          // Select AMR vocoder and specified channel type
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode     |= B_VM_VOCODER_SELECT; // AMR voice memo
          l1s_dsp_com.dsp_ndb_ptr->d_amms_ul_voc  &= ~(SC_CHAN_TYPE_MASK);
          l1s_dsp_com.dsp_ndb_ptr->d_amms_ul_voc  |= l1a_l1s_com.voicememo_amr_task.record.parameters.amr_vocoder;

	  l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
          if (l1a_l1s_com.dedic_set.aset != NULL)
          {
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
              l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
          }
          *(l1s.voicememo_amr.record.a_du_x) = 0;

          // Start the voice memo recording DSP task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_RECORD_START;

          *state = WAIT_DSP_START;
#endif // L1_AUDIO_MCU_ONOFF
        }
        break;

#if (L1_AUDIO_MCU_ONOFF == 1)

        case WAIT_AUDIO_ON:
        {
          // Triton Audio ON/OFF Changes
          if((l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE] == L1_AUDIO_UL_ON))
          {
              // Initialize the buffer parameters
              l1s.voicememo_amr.record.ptr_buf        = NULL;
              l1s.voicememo_amr.record.buffer_size    = 0;
              l1s.voicememo_amr.record.error_id       = Cust_get_pointer((UWORD16 **)&l1s.voicememo_amr.record.ptr_buf,
                                                               &l1s.voicememo_amr.record.buffer_size,
                                                               l1a_l1s_com.voicememo_amr_task.record.parameters.session_id);

              // Convert the buffer size in bytes unit because VM AMR is defined in byte unit
              l1s.voicememo_amr.record.buffer_size <<= 1;

              // Initialize the size of the Voice memo to record
              l1s.voicememo_amr.record.recorded_size = 0;

              // Initialize the DTX mode
              if (l1a_l1s_com.voicememo_amr_task.record.parameters.dtx)
                l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= B_VOICE_MEMO_DTX;
              else
                l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VOICE_MEMO_DTX);

              // Select AMR vocoder and specified channel type
              l1s_dsp_com.dsp_ndb_ptr->d_tch_mode     |= B_VM_VOCODER_SELECT; // AMR voice memo
              l1s_dsp_com.dsp_ndb_ptr->d_amms_ul_voc  &= ~(SC_CHAN_TYPE_MASK);
              l1s_dsp_com.dsp_ndb_ptr->d_amms_ul_voc  |= l1a_l1s_com.voicememo_amr_task.record.parameters.amr_vocoder;

	      l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
              if (l1a_l1s_com.dedic_set.aset != NULL)
              {
                if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                    (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                  l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
              }
              *(l1s.voicememo_amr.record.a_du_x) = 0;

              // Start the voice memo recording DSP task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_RECORD_START;

              *state = WAIT_DSP_START;
          }
        }
        break;

#endif // L1_AUDIO_MCU_ONOFF

        case WAIT_DSP_START:
        {
          // The DSP task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_AMR_RECORD_ON_GOING)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_AMR_RECORD_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_DSP_SAMPLE;
          }
        } // case WAIT_DSP_START:
        break;

        case WAIT_DSP_SAMPLE:
        {
          // The MMI requests to stop the voice memorization recording task
          if (l1a_l1s_com.voicememo_amr_task.record.command.stop)
          {
            // Write the end mask at the end of the voice data RAM buffer
            sample_header = SC_VM_AMR_END_MASK;
            l1s.voicememo_amr.record.error_id = copy_byte_data_to_buffer (l1a_l1s_com.voicememo_amr_task.record.parameters.session_id,
                                                                 &l1s.voicememo_amr.record.buffer_size,
                                                                 (UWORD8 **)&l1s.voicememo_amr.record.ptr_buf,
                                                                 1,
                                                                 &sample_header);

            // Increase the recorded size
            l1s.voicememo_amr.record.recorded_size++;

            // Stop the DSP voice memorization playing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_RECORD_STOP;

            *state = WAIT_DSP_STOP;
          } // if (l1a_l1s_com.voicememo_amr_task.record.command.stop)
          else
          // The DSP sends a new block ?
          {
            // Determine which a_du buffer is currently used
            l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
            if (l1a_l1s_com.dedic_set.aset != NULL)
            {
              if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                   (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                  l1s.voicememo_amr.record.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
            }

            // The DSP sends a new block?
            if ( (*(l1s.voicememo_amr.record.a_du_x)) & B_BLOCK_READY )
            {
              UWORD8 sample_header, temp_header;
              UWORD8  data_size=0 ;//omaps00090550;

              // get RX_TYPE to identify frame (SPEECH_GOOD, SID_FIRST, SID_UPDATE, NO_DATA)
              sample_header = (*l1s.voicememo_amr.record.a_du_x & 0x00FF);
              temp_header   =  sample_header & SC_RX_TX_TYPE_MASK;

              // Check if the block contains speech or SID or NO_DATA
              switch(temp_header)
              {
                case SC_VM_AMR_RXTX_SPEECH_GOOD:
                {
                  temp_header = sample_header & SC_CHAN_TYPE_MASK;
                  switch(temp_header)
                  {
                    case SC_VM_AMR_SPEECH_475:
                      data_size = SC_VM_AMR_SPEECH_475_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_515:
                      data_size = SC_VM_AMR_SPEECH_515_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_59:
                      data_size = SC_VM_AMR_SPEECH_59_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_67:
                      data_size = SC_VM_AMR_SPEECH_67_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_74:
                      data_size = SC_VM_AMR_SPEECH_74_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_795:
                      data_size = SC_VM_AMR_SPEECH_795_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_102:
                      data_size = SC_VM_AMR_SPEECH_102_DATA_SIZE;
                    break;
                    case SC_VM_AMR_SPEECH_122:
                      data_size = SC_VM_AMR_SPEECH_122_DATA_SIZE;
                    break;
                  }
                } // case SC_VM_AMR_RXTX_SPEECH_GOOD:
                break;
                case SC_VM_AMR_RXTX_SID_FIRST:
                  data_size = SC_VM_AMR_SID_FIRST_DATA_SIZE;
                break;
                case SC_VM_AMR_RXTX_SID_UPDATE:
                  data_size = SC_VM_AMR_SID_UPDATE_DATA_SIZE;
                break;
                case SC_VM_AMR_RXTX_NO_DATA:
                  data_size = SC_VM_AMR_NO_DATA_DATA_SIZE;
                break;
                default:
                  // trace error
                break;
              } // switch(temp_header)

              // The maximum allocated size is reached? (need to be able to store header + data + end_mask)
              if ( (l1s.voicememo_amr.record.recorded_size+data_size+SC_VM_AMR_HEADER_SIZE+SC_VM_AMR_END_MASK_SIZE) <= l1a_l1s_com.voicememo_amr_task.record.parameters.maximum_size)
              {
                // Download the header from the a_du_x buffer.
                l1s.voicememo_amr.record.error_id = copy_byte_data_to_buffer (l1a_l1s_com.voicememo_amr_task.record.parameters.session_id,
                                                                     &l1s.voicememo_amr.record.buffer_size,
                                                                     (UWORD8 **)&l1s.voicememo_amr.record.ptr_buf,
                                                                     1,
                                                                     &sample_header);

                if (data_size > 0)
                {
                  l1s.voicememo_amr.record.error_id = copy_byte_data_le_to_buffer (l1a_l1s_com.voicememo_amr_task.record.parameters.session_id,
                                                                       &l1s.voicememo_amr.record.buffer_size,
                                                                       (UWORD8 **)&l1s.voicememo_amr.record.ptr_buf,
                                                                       data_size,
                                                                       l1s.voicememo_amr.record.a_du_x + 3);
                }

                // Increase the recorded size (header + data_bits)
                l1s.voicememo_amr.record.recorded_size += data_size + SC_VM_AMR_HEADER_SIZE;

                // Clear the a_du_x header:
                *(l1s.voicememo_amr.record.a_du_x) = 0;
              }
              else
              {
                // Write the end mask at the end of the voice data RAM buffer
                sample_header = SC_VM_AMR_END_MASK;
                l1s.voicememo_amr.record.error_id = copy_byte_data_to_buffer (l1a_l1s_com.voicememo_amr_task.record.parameters.session_id,
                                                                     &l1s.voicememo_amr.record.buffer_size,
                                                                     (UWORD8 **)&l1s.voicememo_amr.record.ptr_buf,
                                                                     1,
                                                                     &sample_header);

                // Increase the recorded size
                l1s.voicememo_amr.record.recorded_size++;

                // Clear the a_du_x header:
                *(l1s.voicememo_amr.record.a_du_x) = 0;

                // Stop the DSP voice memorization playing task
                l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_AMR_RECORD_STOP;

                *state = WAIT_DSP_STOP;
              }
            }
          } // else of if (l1a_l1s_com.voicememo_amr_task.record.command.stop)
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP voice memorization playing task is stopped
          if (!(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_AMR_RECORD_ON_GOING))
          {
          // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
          l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
#endif // L1_AUDIO_MCU_ONOFF

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_VM_AMR_RECORD_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_VM_AMR_RECORD_STOP_CON;
            //Fill the message
            ((T_L1_VM_AMR_RECORD_CON *)(conf_msg->SigP))->recorded_size = l1s.voicememo_amr.record.recorded_size;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch(*state)
    }

  #endif // L1_VOICE_MEMO_AMR

  #if (SPEECH_RECO)
    /*-------------------------------------------------------*/
    /* l1s_sr_enroll_manager()                               */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : speech recognition enroll               */
    /*               L1S manager task.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_sr_enroll_manager(void)
    {
      enum states
      {
        IDLE                    = 0,
        WAIT_DSP_START          = 1,
        WAIT_ACQUISITION_STATUS = 2,
        WAIT_DSP_STOP           = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_SR_ENROLL_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Initialize the status register
          l1s_dsp_com.dsp_ndb_ptr->d_sr_status = 0;

          // Disable the DSP bit exact test
          l1s_dsp_com.dsp_ndb_ptr->d_sr_bit_exact_test &= 0xff80;

          // Initialize the watchdog timer with the time to acquire a word
          l1s.speechreco.time_out = SC_SR_AQUISITION_TIME_OUT;

          // Start the DSP enroll task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_ENROLL;

          *state = WAIT_DSP_START;
        }
        break;

        case WAIT_DSP_START:
        {
          // The DSP enroll task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_ENROLL_TASK)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_ENROLL_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              *state = WAIT_ACQUISITION_STATUS;
          }
        }
        break;

        case WAIT_ACQUISITION_STATUS:
        {
          // the allowed time isn't out
          if (l1s.speechreco.time_out--)
          {
            // The DSP enroll task ran bad or the MMI stop the enroll task
            if ( (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_ACQUISITION) ||
                 (l1a_l1s_com.speechreco_task.command.enroll_stop) )
            {
              // Error: bad acquisition
              l1s.speechreco.error = SC_BAD_ACQUISITION;

              // Stop the DSP enroll task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
            else
            // The DSP enroll task ran good
            if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_GOOD_ACQUISITION)
            {
              // No error
              l1s.speechreco.error = SC_NO_ERROR;

              // Stop the DSP enroll task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
          }
          else
          {
            // Error: time is out
            l1s.speechreco.error = SC_TIME_OUT;

            // Stop the DSP enroll task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

            *state = WAIT_DSP_STOP;
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP enroll task is stopped
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & (B_SR_ENROLL_TASK | B_VM_RECORD_ON_GOING)) )
          {
            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_SR_ENROLL_STOP_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_ENROLL_STOP_CON;
            //Fill the message
            ((T_L1_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = l1s.speechreco.error;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_sr_update_manager()                               */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : speech recognition update               */
    /*               L1S manager task.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_sr_update_manager(void)
    {
      enum states
      {
        IDLE                          = 0,
        WAIT_DSP_START                = 1,
        WAIT_ACQUISITION_STATUS       = 2,
        WAIT_UPDATE_STATUS            = 3,
        WAIT_DSP_STOP                 = 4
      };

      UWORD8            *state      = &l1s.audio_state[L1S_SR_UPDATE_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Initialize the status register
          l1s_dsp_com.dsp_ndb_ptr->d_sr_status = 0;

          // Disable the DSP bit exact test
          l1s_dsp_com.dsp_ndb_ptr->d_sr_bit_exact_test &= 0xff80;

          // Initialize the watchdog timer with the time to acquire a word
          l1s.speechreco.time_out = SC_SR_AQUISITION_TIME_OUT;

          // Start the DSP update task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_UPDATE;

          #if (W_A_DSP_SR_BGD)
            // Management of DSP tasks in background
            if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt & B_DSPBGD_UPD)
            {
              l1s_dsp_com.dsp_ndb_ptr->d_background_enable |= (1 << C_BGD_ALIGN);
            }
          #endif

          *state = WAIT_DSP_START;
        }
        break;

        case WAIT_DSP_START:
        {
          // The DSP update task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_UPDATE_TASK)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_UPDATE_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_ACQUISITION_STATUS;
          }
        }
        break;

        case WAIT_ACQUISITION_STATUS:
        {
          // the allowed time isn't out
          if (l1s.speechreco.time_out--)
          {
            // The DSP acquisition task ran bad or the MMI stop the update task
            if ( (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_ACQUISITION) ||
                 (l1a_l1s_com.speechreco_task.command.update_stop) )
            {
              // Error: bad acquisition
              l1s.speechreco.error = SC_BAD_ACQUISITION;

              // Stop the DSP acquisition task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
            else
            // The DSP enroll task ran good
            if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_GOOD_ACQUISITION)
            {
              // Initialize the watchdog timer with the time to update a word
              l1s.speechreco.time_out = SC_SR_UPDATE_TIME_OUT;

              *state = WAIT_UPDATE_STATUS;
            }
          }
          else
          {
            // Error: time is out
            l1s.speechreco.error = SC_TIME_OUT;

            // Stop the DSP acquisition task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

            *state = WAIT_DSP_STOP;
          }
        }
        break;


        case WAIT_UPDATE_STATUS:
        {
          // the allowed time isn't out
          if (l1s.speechreco.time_out--)
          {
            // The DSP update task ran bad or the MMI stop the update task
            if ( (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_UPDATE) ||
                 (l1a_l1s_com.speechreco_task.command.update_stop) )
            {
              // Error: bad update
              l1s.speechreco.error = SC_BAD_UPDATE;

              // Stop the DSP update task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
            else
            // The DSP update task ran good
            if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_GOOD_UPDATE)
            {
              // No error:
              l1s.speechreco.error = SC_NO_ERROR;

              // Stop the DSP update task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
          }
          else
          {
            // Error: time is out
            l1s.speechreco.error = SC_TIME_OUT;

            // Stop the DSP enroll task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

            *state = WAIT_DSP_STOP;
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP enroll task is stopped
          if ( !( l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & (B_SR_UPDATE_TASK | B_SR_ALIGNMENT_TASK | B_VM_RECORD_ON_GOING) ) )
          {
            #if (W_A_DSP_SR_BGD)
              // Management of DSP tasks in background
              if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt & B_DSPBGD_UPD)
              {
                l1s_dsp_com.dsp_ndb_ptr->d_background_enable &= ~(1 << C_BGD_ALIGN);
              }
            #endif

            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_SR_UPDATE_STOP_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_UPDATE_STOP_CON;
            //Fill the message
            ((T_L1_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = l1s.speechreco.error;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_sr_reco_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : speech recognition reco acquisition     */
    /*               L1S manager task.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_sr_reco_manager(void)
    {
      enum states
      {
        IDLE                    = 0,
        WAIT_DSP_START          = 1,
        WAIT_ACQUISITION_STATUS = 2,
        WAIT_DSP_STOP           = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_SR_RECO_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Initialize the status register
          l1s_dsp_com.dsp_ndb_ptr->d_sr_status = 0;

          // Disable the DSP bit exact test
          l1s_dsp_com.dsp_ndb_ptr->d_sr_bit_exact_test &= 0xff80;

          // Initialize the watchdog timer with the time to acquire a word
          l1s.speechreco.time_out = SC_SR_AQUISITION_TIME_OUT;

          // Start the DSP acquisition reco task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_RECO;

          *state = WAIT_DSP_START;
        }
        break;

        case WAIT_DSP_START:
        {
          // The DSP reco task is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_RECO_TASK)
          {
            // Send the start confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_RECO_START_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_ACQUISITION_STATUS;
          }
        }
        break;

        case WAIT_ACQUISITION_STATUS:
        {
          // the allowed time isn't out
          if (l1s.speechreco.time_out--)
          {
            // The DSP acquisition reco task ran bad or the MMI stop the acquisition reco task
            if ( (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_ACQUISITION) ||
                 (l1a_l1s_com.speechreco_task.command.reco_stop) )
            {
              // Error: bad acquisition
              l1s.speechreco.error = SC_BAD_ACQUISITION;

              // Stop the DSP acquisition reco task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
            else
            // The DSP enroll task ran good
            if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_GOOD_ACQUISITION)
            {
              // No error
              l1s.speechreco.error = SC_NO_ERROR;

              // Stop the DSP acquisition reco task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
          }
          else
          {
            // Error: time is out
            l1s.speechreco.error = SC_TIME_OUT;

            // Stop the DSP acquisition reco task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

            *state = WAIT_DSP_STOP;
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP enroll task is stopped
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_RECO_TASK) )
          {
            // Send the stop confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_SR_RECO_STOP_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_RECO_STOP_CON;
            //Fill the message
            ((T_L1_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = l1s.speechreco.error;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_sr_processing_manager()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : speech recognition reco processing      */
    /*               L1S manager task.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_sr_processing_manager(void)
    {
      enum states
      {
        IDLE                       = 0,
        WAIT_DSP_PROCESSING_STOP   = 1,
        WAIT_DSP_RESULT            = 2,
        WAIT_DSP_STOP              = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_SR_PROCESSING_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Initialize the status register
          l1s_dsp_com.dsp_ndb_ptr->d_sr_status = 0;

          // Disable the DSP bit exact test
          l1s_dsp_com.dsp_ndb_ptr->d_sr_bit_exact_test &= 0xff80;

          // Initialize the OOV algorithm
          l1s_dsp_com.dsp_ndb_ptr->d_sr_param &= 0x20;
          l1s_dsp_com.dsp_ndb_ptr->d_sr_param |= SC_SR_OOV_SFT_THR;

          // Transmit ot the DSP the number of word to compare
          l1s_dsp_com.dsp_ndb_ptr->d_sr_nb_words = l1a_l1s_com.speechreco_task.parameters.vocabulary_size;

          // Initialize the watchdog timer with the time to process a word
          l1s.speechreco.time_out = SC_SR_PROCESSING_TIME_OUT;

          // Start the DSP processing task
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_PROCESSING;

          #if (W_A_DSP_SR_BGD)
            // Management of DSP tasks in background
            if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt &  B_DSPBGD_RECO)
            {
              l1s_dsp_com.dsp_ndb_ptr->d_background_enable |= (1 << C_BGD_RECOGN);
            }
          #endif

          // Reset the start command
          l1a_l1s_com.speechreco_task.command.processing_start = FALSE;

          *state = WAIT_DSP_PROCESSING_STOP;
        }
        break;

        case WAIT_DSP_PROCESSING_STOP:
        {
          if (l1s.speechreco.time_out--)
          {
            // The MMI stops the processing task
            if (l1a_l1s_com.speechreco_task.command.processing_stop)
            {
              // Error: bad acquisition
              l1s.speechreco.error = SC_BAD_RECOGNITION;

              // Stop the DSP processing task
              l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

              *state = WAIT_DSP_STOP;
            }
            else
            // The DSP processing task is stopped
            if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_PROCESSING_TASK) )
            {
              // It was the last model
              if (l1a_l1s_com.speechreco_task.parameters.index_counter == l1a_l1s_com.speechreco_task.parameters.vocabulary_size)
              {
                *state = WAIT_DSP_RESULT;
              }
              else
              {
                // Send the stop confirmation message with no error
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_L1_SR_PROCESSING_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = L1_SR_PROCESSING_STOP_CON;
                //Fill the message
                ((T_L1_SR_PROCESSING_STOP_CON *)(conf_msg->SigP))->error_id = SC_NO_ERROR;
                // Send confirmation message...
                os_send_sig(conf_msg, L1C1_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)

                *state = IDLE;
              }
            }
          }
          else
          // the allowed time is out
          {
            // Error: time is out
            l1s.speechreco.error = SC_TIME_OUT;

            // Stop the DSP processing task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_SR_STOP;

            *state = WAIT_DSP_STOP;
          }
        }
        break;

        case WAIT_DSP_RESULT:
        {

          #if (W_A_DSP_SR_BGD)
            // Management of DSP tasks in background
            if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt &  B_DSPBGD_RECO)
            {
              l1s_dsp_com.dsp_ndb_ptr->d_background_enable &= ~(1 << C_BGD_RECOGN);
            }
          #endif

          // The DSP recognition task was bad
          if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_ACQUISITION)
          {
            // Send the stop indication message with an bad recognition error
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_SR_RECO_STOP_IND));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_RECO_STOP_IND;
            //Fill the message
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->error_id = SC_BAD_RECOGNITION;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
          else
          // The DSP recognition task was good:
          {
            // Send the stop indication message without any error
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_SR_RECO_STOP_IND));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_SR_RECO_STOP_IND;
            //Fill the message
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->error_id               = SC_NO_ERROR;
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->best_word_index        = l1s_dsp_com.dsp_ndb_ptr->a_n_best_words[0];
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->best_word_score        = l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[0] | (l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[1] << 16);
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->second_best_word_index = l1s_dsp_com.dsp_ndb_ptr->a_n_best_words[1];
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->second_best_word_score = l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[2] | (l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[3] << 16);
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->third_best_word_index  = l1s_dsp_com.dsp_ndb_ptr->a_n_best_words[2];
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->third_best_word_score  = l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[4] | (l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[5] << 16);
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->fourth_best_word_index = l1s_dsp_com.dsp_ndb_ptr->a_n_best_words[3];
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->fourth_best_word_score = l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[6] | (l1s_dsp_com.dsp_ndb_ptr->a_n_best_score[7] << 16);
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->d_sr_db_level          = l1s_dsp_com.dsp_ndb_ptr->d_sr_db_level;
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->d_sr_db_noise          = l1s_dsp_com.dsp_ndb_ptr->d_sr_db_noise;
            ((T_L1_SR_RECO_STOP_IND *)(conf_msg->SigP))->d_sr_model_size        = l1s_dsp_com.dsp_ndb_ptr->d_sr_mod_size;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP processing task is stopped
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_SR_PROCESSING_TASK) )
          {
            // Send the stop confirmation message with an error
           // Allocate confirmation message...
          conf_msg = os_alloc_sig(sizeof(T_L1_SR_PROCESSING_STOP_CON));
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1_SR_PROCESSING_STOP_CON;
          //Fill the message
          ((T_L1_SR_PROCESSING_STOP_CON *)(conf_msg->SigP))->error_id = l1s.speechreco.error;
          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }

    /*-------------------------------------------------------*/
    /* l1s_sr_speech_manager()                               */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : speech recognition speech recording     */
    /*               L1S manager task.                       */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_sr_speech_manager(void)
    {
      enum states
      {
        IDLE            = 0,
        WAIT_DSP_START  = 1,
        WAIT_DSP_SAMPLE = 2,
        WAIT_DSP_STOP   = 3
      };

      UWORD8  *state = &l1s.audio_state[L1S_SR_SPEECH_STATE];
      API *a_du_x;
      UWORD8  i;

      switch(*state)
      {
        case IDLE:
        {
          // Initialize the current pointer
          l1s.speechreco.speech_pointer = l1a_l1s_com.speechreco_task.parameters.speech_address;
          l1s.speechreco.end_pointer = (UWORD16 *)(l1s.speechreco.speech_pointer + SC_SR_MMI_2_L1_SPEECH_SIZE);

          // Initialize the flag to know if it's the first pass in the circular buffer
          l1s.speechreco.first_pass = TRUE;

          // Initialize the status register
          l1s_dsp_com.dsp_ndb_ptr->d_sr_status = 0;

          // No DTX mode
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VOICE_MEMO_DTX);
         #if ((DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
          // use TCH/FS vocoder
          l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(B_VM_VOCODER_SELECT);
         #endif


          // Start the voice memo recodgin DSP task:
          l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_START;

          *state = WAIT_DSP_START;
        }
        break;

        case WAIT_DSP_START:
        {
          // The DSP is started
          if (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_RECORD_ON_GOING)
          {
            *state = WAIT_DSP_SAMPLE;
          }
        }
        break;

        case WAIT_DSP_SAMPLE:
        {
          // A beginning of word is detected
          if ( ((l1s_dsp_com.dsp_ndb_ptr->d_sr_status & SC_SR_WORD_MASK) == SC_SR_WORD_BEGINNING) &&
               (l1s.speechreco.speech_old_status == SC_SR_WORD_SEARCHING) )
          {
            // Calculate the address of the beginning of the word
            l1a_l1s_com.speechreco_task.parameters.start_address =
              l1s.speechreco.speech_pointer - ((SC_SR_SPEECH_WORD_BEGIN_VAD_LATENCY + SC_SR_SPEECH_WORD_BEGIN_MARGIN) * SC_SR_SPEECH_FRAME_SIZE);
            if (l1a_l1s_com.speechreco_task.parameters.start_address < l1a_l1s_com.speechreco_task.parameters.speech_address)
            {
              if (l1s.speechreco.first_pass == FALSE)
              {
                l1a_l1s_com.speechreco_task.parameters.start_address = l1s.speechreco.end_pointer -
                  ( l1a_l1s_com.speechreco_task.parameters.speech_address - (l1s.speechreco.speech_pointer - ((SC_SR_SPEECH_WORD_BEGIN_VAD_LATENCY + SC_SR_SPEECH_WORD_BEGIN_MARGIN) * SC_SR_SPEECH_FRAME_SIZE)) );
              }
              else
              {
                l1a_l1s_com.speechreco_task.parameters.start_address = l1a_l1s_com.speechreco_task.parameters.speech_address;
              }
            }
          }
          else
          // A end of word is detected
          if ( ((l1s_dsp_com.dsp_ndb_ptr->d_sr_status & SC_SR_WORD_MASK) == SC_SR_WORD_ENDING) &&
               (l1s.speechreco.speech_old_status == SC_SR_WORD_ON_GOING) )
          {
            // Calculate the address of the end of the word
            l1a_l1s_com.speechreco_task.parameters.stop_address =
              l1s.speechreco.speech_pointer - ((SC_SR_SPEECH_WORD_END_VAD_LATENCY - SC_SR_SPEECH_WORD_END_MARGIN)* SC_SR_SPEECH_FRAME_SIZE);
            if (l1a_l1s_com.speechreco_task.parameters.stop_address < l1a_l1s_com.speechreco_task.parameters.speech_address)
            {
              l1a_l1s_com.speechreco_task.parameters.stop_address = l1s.speechreco.end_pointer -
                ( l1a_l1s_com.speechreco_task.parameters.speech_address - (l1s.speechreco.speech_pointer - ((SC_SR_SPEECH_WORD_END_VAD_LATENCY - SC_SR_SPEECH_WORD_END_MARGIN) * SC_SR_SPEECH_FRAME_SIZE)) );
            }
          }

          // Save the current status
          l1s.speechreco.speech_old_status = l1s_dsp_com.dsp_ndb_ptr->d_sr_status & SC_SR_WORD_MASK;

          // Determine which a_du buffer is currently used
          l1s.speechreco.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_1;
          if (l1a_l1s_com.dedic_set.aset != NULL)
          {
            if ( (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->channel_type == TCH_H) &&
                 (l1a_l1s_com.dedic_set.aset->achan_ptr->desc_ptr->subchannel == 1) )
                l1s.speechreco.a_du_x = l1s_dsp_com.dsp_ndb_ptr->a_du_0;
          }

          // The acquisition was good
          if (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_GOOD_ACQUISITION)
          {
            // Stop the voice memorization recording task
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_STOP;

            *state = WAIT_DSP_STOP;
          }
          else
          // The task must be stopped
          if ( (l1s_dsp_com.dsp_ndb_ptr->d_sr_status & B_BAD_ACQUISITION) ||
               (l1a_l1s_com.speechreco_task.command.speech_stop) ||
               (l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init & B_SR_STOP) )
          {
            // Stop the DSP voice memorization recording task:
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= B_VM_RECORD_STOP;

            *state = WAIT_DSP_STOP;
          }
          else
          // The DSP sends a new block
          if (l1s.speechreco.a_du_x[0] & B_BLOCK_READY)
          {
            // Is there enough place in the RAM buffer
            if (l1s.speechreco.speech_pointer == l1s.speechreco.end_pointer)
            {
              // Rewind the current pointer
              l1s.speechreco.speech_pointer = l1a_l1s_com.speechreco_task.parameters.speech_address;

              // It isn't the first pass
              l1s.speechreco.first_pass = FALSE;
            }

            // Download the speech sample from the a_du_x to the RAM buffer
            a_du_x = l1s.speechreco.a_du_x;
            for(i=0; i < SC_SR_SPEECH_FRAME_SIZE; i++)
            {
              *(l1s.speechreco.speech_pointer)++ = *a_du_x++;
            }

            // Clear the a_du_x header
            l1s.speechreco.a_du_x[0] = 0;
          }
        }
        break;

        case WAIT_DSP_STOP:
        {
          // The DSP speech recoding task is stopped
          if ( !(l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status & B_VM_RECORD_ON_GOING) )
          {
            *state = IDLE;
          }
        }
        break;
      } // switch
    }

  #endif  // SPEECH_RECO
  #if (L1_AEC == 1)
    /*-------------------------------------------------------*/
    /* l1s_aec_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : AEC L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_aec_manager(void)
    {
      enum states
      {
        IDLE               = 0,
      #if (L1_NEW_AEC)
        WAIT_DSP_AVAILABLE = 1,
        AEC_VISIBILITY     = 2
      #else
        WAIT_DSP_AVAILABLE = 1
      #endif
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AEC_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Set the d_aec_ctrl register
          l1s.aec.aec_control = (l1a_l1s_com.aec_task.parameters.aec_control | B_AEC_ACK);
        #if (L1_NEW_AEC)
          l1s.aec.aec_visibility  = (l1s.aec.aec_control & B_AEC_VISIBILITY) >> SC_AEC_VISIBILITY_SHIFT;
          l1s.aec.cont_filter     = l1a_l1s_com.aec_task.parameters.cont_filter;
          l1s.aec.granularity_att = l1a_l1s_com.aec_task.parameters.granularity_att;
          l1s.aec.coef_smooth     = l1a_l1s_com.aec_task.parameters.coef_smooth;
          l1s.aec.es_level_max    = l1a_l1s_com.aec_task.parameters.es_level_max;
          l1s.aec.fact_vad        = l1a_l1s_com.aec_task.parameters.fact_vad;
          l1s.aec.thrs_abs        = l1a_l1s_com.aec_task.parameters.thrs_abs;
          l1s.aec.fact_asd_fil    = l1a_l1s_com.aec_task.parameters.fact_asd_fil;
          l1s.aec.fact_asd_mut    = l1a_l1s_com.aec_task.parameters.fact_asd_mut;
        #endif

          // Reset the start command
          l1a_l1s_com.aec_task.command.start = FALSE;

          // Send the AEC confirmation message
          // Allocate confirmation message...
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1_AEC_CON;
          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          *state = WAIT_DSP_AVAILABLE;
        }
        break;

        case WAIT_DSP_AVAILABLE:
        {
          // the new settings come from the MMI
          if (l1a_l1s_com.aec_task.command.start)
          {
            // Set the d_aec_ctrl register
            l1s.aec.aec_control = (l1a_l1s_com.aec_task.parameters.aec_control | B_AEC_ACK);
          #if (L1_NEW_AEC)
            l1s.aec.aec_visibility  = (l1s.aec.aec_control & B_AEC_VISIBILITY) >> SC_AEC_VISIBILITY_SHIFT;
            l1s.aec.cont_filter     = l1a_l1s_com.aec_task.parameters.cont_filter;
            l1s.aec.granularity_att = l1a_l1s_com.aec_task.parameters.granularity_att;
            l1s.aec.coef_smooth     = l1a_l1s_com.aec_task.parameters.coef_smooth;
            l1s.aec.es_level_max    = l1a_l1s_com.aec_task.parameters.es_level_max;
            l1s.aec.fact_vad        = l1a_l1s_com.aec_task.parameters.fact_vad;
            l1s.aec.thrs_abs        = l1a_l1s_com.aec_task.parameters.thrs_abs;
            l1s.aec.fact_asd_fil    = l1a_l1s_com.aec_task.parameters.fact_asd_fil;
            l1s.aec.fact_asd_mut    = l1a_l1s_com.aec_task.parameters.fact_asd_mut;
          #endif

            // Reset the start command
            l1a_l1s_com.aec_task.command.start = FALSE;

            // Send the AEC confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AEC_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)
          }

          // the new settings can be written to the DSP
#if(DSP == 38) || (DSP == 39)
          if ( (l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl & B_AEC_ACK) == FALSE )
#else
          if ( (l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl & B_AEC_ACK)    == FALSE )
#endif
          {

#if(DSP == 38) || (DSP == 39)
            l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl = l1s.aec.aec_control;
#else
            l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl    = l1s.aec.aec_control;
#endif

          #if (L1_NEW_AEC)
            l1s_dsp_com.dsp_ndb_ptr->d_cont_filter     = l1s.aec.cont_filter;
            l1s_dsp_com.dsp_ndb_ptr->d_granularity_att = l1s.aec.granularity_att;
            l1s_dsp_com.dsp_ndb_ptr->d_coef_smooth     = l1s.aec.coef_smooth;
            l1s_dsp_com.dsp_ndb_ptr->d_es_level_max    = l1s.aec.es_level_max;
            l1s_dsp_com.dsp_ndb_ptr->d_fact_vad        = l1s.aec.fact_vad;
            l1s_dsp_com.dsp_ndb_ptr->d_thrs_abs        = l1s.aec.thrs_abs;
            l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_fil    = l1s.aec.fact_asd_fil;
            l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_mut    = l1s.aec.fact_asd_mut;

            // AEC visibility allows tracing some AEC internal output values
            if (l1s.aec.aec_visibility)
              *state = AEC_VISIBILITY;
            else
              *state = IDLE;
          #else
            *state = IDLE;
          #endif
          }
        }
        break;

      #if (L1_NEW_AEC)
        case AEC_VISIBILITY:
        {
          // the new settings come from the MMI
          if (l1a_l1s_com.aec_task.command.start)
          {
            // Set the d_aec_ctrl register
            l1s.aec.aec_control     = (l1a_l1s_com.aec_task.parameters.aec_control | B_AEC_ACK);
            l1s.aec.aec_visibility  = (l1s.aec.aec_control & B_AEC_VISIBILITY) >> SC_AEC_VISIBILITY_SHIFT;
            l1s.aec.cont_filter     = l1a_l1s_com.aec_task.parameters.cont_filter;
            l1s.aec.granularity_att = l1a_l1s_com.aec_task.parameters.granularity_att;
            l1s.aec.coef_smooth     = l1a_l1s_com.aec_task.parameters.coef_smooth;
            l1s.aec.es_level_max    = l1a_l1s_com.aec_task.parameters.es_level_max;
            l1s.aec.fact_vad        = l1a_l1s_com.aec_task.parameters.fact_vad;
            l1s.aec.thrs_abs        = l1a_l1s_com.aec_task.parameters.thrs_abs;
            l1s.aec.fact_asd_fil    = l1a_l1s_com.aec_task.parameters.fact_asd_fil;
            l1s.aec.fact_asd_mut    = l1a_l1s_com.aec_task.parameters.fact_asd_mut;

            // Reset the start command
            l1a_l1s_com.aec_task.command.start = FALSE;

            // Send the AEC confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AEC_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = WAIT_DSP_AVAILABLE;
          }

          if ( (l1a_l1s_com.dedic_set.aset != NULL) &&
               ((l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_FS_MODE) ||
                (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_HS_MODE) ||
                (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_EFR_MODE)) )
          {
            l1s.aec.visibility_interval--;

            if (l1s.aec.visibility_interval < 0)
            {
              conf_msg = os_alloc_sig(sizeof(T_L1_AEC_IND));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AEC_IND;

              ((T_L1_AEC_IND *)(conf_msg->SigP))->es_level      = l1s_dsp_com.dsp_ndb_ptr->d_es_level_api;
              ((T_L1_AEC_IND *)(conf_msg->SigP))->far_end_pow   = ( (l1s_dsp_com.dsp_ndb_ptr->d_far_end_pow_h << 16)
                                                                  | (l1s_dsp_com.dsp_ndb_ptr->d_far_end_pow_l));
              ((T_L1_AEC_IND *)(conf_msg->SigP))->far_end_noise = ( (l1s_dsp_com.dsp_ndb_ptr->d_far_end_noise_h << 16)
                                                                  | (l1s_dsp_com.dsp_ndb_ptr->d_far_end_noise_l));

              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              // reset delay between 2 traces
              l1s.aec.visibility_interval = SC_AEC_VISIBILITY_INTERVAL;
            }
          }
          else
            // It forces aec traces when entering dedicated mode
            l1s.aec.visibility_interval = 1;
        }
        break;
      #endif
      } // switch
    }
  #endif // AEC

  #if(L1_AEC == 2)

  /*-------------------------------------------------------*/
  /* l1s_aec_manager()                                     */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : AEC L1S manager task.                   */
  /*                                                       */
  /*-------------------------------------------------------*/
void l1s_aec_manager(void)
    {
      enum states
      {
        IDLE               = 0,
        WAIT_DSP_ACK       = 1
      };

      UWORD8            *state = &l1s.audio_state[L1S_AEC_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD16 current_state;
      static T_AEC_ACTION l1s_aec_action = L1_AQI_AEC_STOPPED;
      static UWORD16 l_aec_ctrl;


      switch(*state)
      {
        case IDLE:
        {

          current_state = l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & 0x0800;

          if((l1a_l1s_com.aec_task.aec_control == L1_AQI_AEC_START)||(l1a_l1s_com.aec_task.aec_control == L1_AQI_AEC_UPDATE))
          {
          	if(current_state )
          	{

#if(DSP == 38) || (DSP == 39)
          		l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl = l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl | 0x0004;
#else
          		l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl = l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl | 0x0004;
#endif

          		l_aec_ctrl = 0x0004;
          		l1s_aec_action = L1_AQI_AEC_UPDATED;
          	}
          	else
          	{


#if(DSP == 38) || (DSP == 39)
				l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl = l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl | 0x0001;
#else
          		l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl = l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl | 0x0001;
#endif
          		l_aec_ctrl = 0x0001;
          		l1s_aec_action = L1_AQI_AEC_STARTED;
          	}

			  l1s_dsp_com.dsp_ndb_ptr->d_cont_filter     = l1a_l1s_com.aec_task.parameters.cont_filter;
			  l1s_dsp_com.dsp_ndb_ptr->d_granularity_att = l1a_l1s_com.aec_task.parameters.granularity_att;
			  l1s_dsp_com.dsp_ndb_ptr->d_coef_smooth     = l1a_l1s_com.aec_task.parameters.coef_smooth;
			  l1s_dsp_com.dsp_ndb_ptr->d_es_level_max    = l1a_l1s_com.aec_task.parameters.es_level_max;
			  l1s_dsp_com.dsp_ndb_ptr->d_fact_vad        = l1a_l1s_com.aec_task.parameters.fact_vad;
			  l1s_dsp_com.dsp_ndb_ptr->d_thrs_abs        = l1a_l1s_com.aec_task.parameters.thrs_abs;
			  l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_fil    = l1a_l1s_com.aec_task.parameters.fact_asd_fil;
			  l1s_dsp_com.dsp_ndb_ptr->d_fact_asd_mut    = l1a_l1s_com.aec_task.parameters.fact_asd_mut;
			  l1s_dsp_com.dsp_ndb_ptr->d_aec_mode		 = l1a_l1s_com.aec_task.parameters.aec_mode;
			  l1s_dsp_com.dsp_ndb_ptr->d_mu				 = l1a_l1s_com.aec_task.parameters.mu;
			  l1s_dsp_com.dsp_ndb_ptr->d_scale_input_ul	 = l1a_l1s_com.aec_task.parameters.scale_input_ul;
			  l1s_dsp_com.dsp_ndb_ptr->d_scale_input_dl	 = l1a_l1s_com.aec_task.parameters.scale_input_dl;
			  l1s_dsp_com.dsp_ndb_ptr->d_div_dmax		 = l1a_l1s_com.aec_task.parameters.div_dmax;
			  l1s_dsp_com.dsp_ndb_ptr->d_div_swap_good	 = l1a_l1s_com.aec_task.parameters.div_swap_good;
			  l1s_dsp_com.dsp_ndb_ptr->d_div_swap_bad	 = l1a_l1s_com.aec_task.parameters.div_swap_bad;
			  l1s_dsp_com.dsp_ndb_ptr->d_block_init		 = l1a_l1s_com.aec_task.parameters.block_init;


          }
          else if(l1a_l1s_com.aec_task.aec_control == L1_AQI_AEC_STOP)
          {
          	if(current_state )
          	{
#if(DSP == 38) || (DSP == 39)
          		l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl = (l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl) | 0x0002;
#else
				l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl = (l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl) | 0x0002;
#endif

          		l_aec_ctrl = 0x0002;
          		l1s_aec_action = L1_AQI_AEC_STOPPED;
          	}
          	else
          	{
          		l1a_l1s_com.aec_task.command.start = FALSE;

				// Send the AEC confirmation message
				// Allocate confirmation message...
				conf_msg = os_alloc_sig(sizeof(T_L1_AQI_AEC_CON));
				DEBUGMSG(status,NU_ALLOC_ERR)
				conf_msg->SignalCode = L1_AQI_AEC_CON;
				((T_L1_AQI_AEC_CON*)(conf_msg->SigP))->aec_action = L1_AQI_AEC_NO_ACTION;
				// Send confirmation message...
				os_send_sig(conf_msg, L1C1_QUEUE);
				DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          		return;
          	}
          }

		  *state = WAIT_DSP_ACK;


          break;
        }

        case WAIT_DSP_ACK:
        {

#if(DSP == 38) || (DSP == 39)
          if(((l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl) & (l_aec_ctrl)) == 0)
#else
          if(((l1s_dsp_com.dsp_ndb_ptr->d_aec_ctrl) & (l_aec_ctrl))    == 0)
#endif

          {
          	l1a_l1s_com.aec_task.command.start = FALSE;

			// Send the AEC confirmation message
			// Allocate confirmation message...
			conf_msg = os_alloc_sig(sizeof(T_L1_AQI_AEC_CON));
			DEBUGMSG(status,NU_ALLOC_ERR)
			conf_msg->SignalCode = L1_AQI_AEC_CON;
			((T_L1_AQI_AEC_CON*)(conf_msg->SigP))->aec_action = l1s_aec_action;
			// Send confirmation message...
			os_send_sig(conf_msg, L1C1_QUEUE);
			DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }

          break;
       }
	}/* End of switch statement */
}

  #endif

  #if (FIR)
    /*-------------------------------------------------------*/
    /* l1s_fir_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : FIR L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_fir_manager(void)
    {

	  enum states
	  {
		  IDLE,
		  WAIT_AUDIO_ON,
		  FIR_LOOP_ON
	  };

      UWORD8            *state      = &l1s.audio_state[L1S_FIR_STATE];
      xSignalHeaderRec  *conf_msg;



      switch (*state)
      {
         case IDLE:
         {
			 if (l1a_l1s_com.fir_task.parameters.fir_loop == 0)
			 {
                l1s_fir_set_params();
				// Send the FIR confirmation message
      			// Allocate confirmation message...
      			conf_msg = os_alloc_sig(0);
      			DEBUGMSG(status,NU_ALLOC_ERR)
      			conf_msg->SignalCode = L1_AUDIO_FIR_CON;
      			// Send confirmation message...
      			os_send_sig(conf_msg, L1C1_QUEUE);
      			DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      			// Reset the start command
      			l1a_l1s_com.fir_task.command.start = FALSE;
		     }
             else  {
				//enable UL and DL
				l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
		        l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
                *state = WAIT_AUDIO_ON;
			 }
	     }
	     break;

	     case WAIT_AUDIO_ON:
	     {
			  if((l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE] == L1_AUDIO_DL_ON) &&
			      (l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE] == L1_AUDIO_UL_ON))
			  {
                 l1s_fir_set_params();
				// Send the FIR confirmation message
      			// Allocate confirmation message...
      			conf_msg = os_alloc_sig(0);
      			DEBUGMSG(status,NU_ALLOC_ERR)
      			conf_msg->SignalCode = L1_AUDIO_FIR_CON;
      			// Send confirmation message...
      			os_send_sig(conf_msg, L1C1_QUEUE);
      			DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      			// Reset the start command
      			l1a_l1s_com.fir_task.command.start = FALSE;
                //set the Loop on the DSP side
        l1s_dsp_com.dsp_ndb_ptr->d_audio_init |= B_FIR_LOOP;
                *state = FIR_LOOP_ON;
			  }

		 }
		 break;

		 case FIR_LOOP_ON:
		 {
			 if (l1a_l1s_com.fir_task.command.start == TRUE)
			 {
			    if (l1a_l1s_com.fir_task.parameters.fir_loop == 0)
			    {
			       //disable UL and DL
		           l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
		           l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;

        l1s_dsp_com.dsp_ndb_ptr->d_audio_init &= ~(B_FIR_LOOP);
		           *state = IDLE;
			    }

				// Reset the start command
      			l1a_l1s_com.fir_task.command.start = FALSE;
      			//download parameters
      			l1s_fir_set_params();
      			// Send the FIR confirmation message
				// Allocate confirmation message...
				conf_msg = os_alloc_sig(0);
				DEBUGMSG(status,NU_ALLOC_ERR)
				conf_msg->SignalCode = L1_AUDIO_FIR_CON;
				// Send confirmation message...
				os_send_sig(conf_msg, L1C1_QUEUE);
      			DEBUGMSG(status,NU_SEND_QUEUE_ERR)
			 }
		 }
		 break;
      } // end switch

    }

    void l1s_fir_set_params(void)
    {

      UWORD8            i;

      // Update the DL FIR?
      if (l1a_l1s_com.fir_task.parameters.update_fir & DL_FIR)
      {
        // Download the DL FIR coefficients to the melody a_fir31_downlink
        for (i=0; i<MAX_FIR_COEF; i++)
        {
          #if ((DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)) // For this DSP code the FIR coefficients are in API param memory
            l1s_dsp_com.dsp_param_ptr->a_fir31_downlink[i] = *l1a_l1s_com.fir_task.parameters.fir_dl_coefficient;
          #else
            l1s_dsp_com.dsp_ndb_ptr->a_fir31_downlink[i] = *l1a_l1s_com.fir_task.parameters.fir_dl_coefficient;
          #endif
          l1a_l1s_com.fir_task.parameters.fir_dl_coefficient++;
        }
      }

      // Update the UL FIR?
      if (l1a_l1s_com.fir_task.parameters.update_fir & UL_FIR)
      {
        if ((l1s_dsp_com.dsp_ndb_ptr->d_audio_status & B_FIR_LOOP) == 1) // loop mode --> do not invert coef
        {
          // Download the UL FIR coefficients to the melody a_fir31_uplink
          for (i=0; i<MAX_FIR_COEF; i++)
          {
            #if (DSP == 33) || (DSP == 34) || (DSP == 35) // For this DSP code the FIR coefficients are in API param memory
              l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[i] = *l1a_l1s_com.fir_task.parameters.fir_ul_coefficient;
            #elif ((DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))  // CQ #28839
              l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1] = *l1a_l1s_com.fir_task.parameters.fir_ul_coefficient;
            #else
              l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[i]   = *l1a_l1s_com.fir_task.parameters.fir_ul_coefficient;
            #endif
            l1a_l1s_com.fir_task.parameters.fir_ul_coefficient++;
          }
        }
        else // normal mode --> invert coeff
        {
          // Download the UL FIR coefficients to the melody a_fir31_uplink
          for (i=0; i<MAX_FIR_COEF; i++)
          {
            // In UL, coefs are inversed
            #if ((DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36)  || (DSP == 37) || (DSP == 38) || (DSP == 39))// For this DSP code the FIR coefficients are in API param memory
              l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1] = *l1a_l1s_com.fir_task.parameters.fir_ul_coefficient;
            #else
              l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1]   = *l1a_l1s_com.fir_task.parameters.fir_ul_coefficient;
            #endif
            l1a_l1s_com.fir_task.parameters.fir_ul_coefficient++;
          }
        }
      }
      else // no UL update
      {
        if (((l1s_dsp_com.dsp_ndb_ptr->d_audio_status & B_FIR_LOOP) && (l1a_l1s_com.fir_task.parameters.fir_loop == FALSE))
         || ((!(l1s_dsp_com.dsp_ndb_ptr->d_audio_status & B_FIR_LOOP)) && (l1a_l1s_com.fir_task.parameters.fir_loop == TRUE))) // changing mode
        {
          // we are changing mode, normal to loop or loop to normal
          // so we have to invert the coefficients in the API
          UWORD16 temp_coeff;

          for (i=0; i<(MAX_FIR_COEF/2); i++)
          {
            #if ((DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)) // For this DSP code the FIR coefficients are in API param memory
              temp_coeff = l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[i];
              l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[i] = l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1];
              l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1] = temp_coeff;
            #else
              temp_coeff = l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[i];
              l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[i] = l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1];
              l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[MAX_FIR_COEF-i-1] = temp_coeff;
            #endif
          }
        }
      }


    }
  #endif // FIR
  #if (AUDIO_MODE)
    /*-------------------------------------------------------*/
    /* l1s_audio_mode_manager()                              */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Audio mode L1S manager task.            */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_audio_mode_manager(void)
    {
      enum states
      {
        IDLE               = 0,
        WAIT_DSP_CONFIRM   = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AUDIO_MODE_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {

          // Reset the d_audio_init
          l1s_dsp_com.dsp_ndb_ptr->d_audio_init &= ~(B_GSM_ONLY | B_BT_HEADSET | B_BT_CORDLESS);

          // Set the new mode
          l1s_dsp_com.dsp_ndb_ptr->d_audio_init
            |= l1a_l1s_com.audio_mode_task.parameters.audio_mode;

          *state = WAIT_DSP_CONFIRM;
        }
        break;

        case WAIT_DSP_CONFIRM:
        {
          // the DSP acknowledges the new settings.
          if ( l1s_dsp_com.dsp_ndb_ptr->d_audio_init == l1s_dsp_com.dsp_ndb_ptr->d_audio_status )
          {
            // Send the Audio mode confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AUDIO_MODE_CON;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            // Reset the start command
            l1a_l1s_com.audio_mode_task.command.start = FALSE;

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
 #endif // AUDIO_MODE
  #if (MELODY_E2)
    /*-------------------------------------------------------*/
    /*    l1s_melody0_e2_manager()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Audio melody 0 format E2 L1S manager    */
    /*               task.                                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_melody0_e2_manager(void)
    {
      enum states
      {
        M0_INACTIVE                 = 0,
        M0_ALIGN_40MS_BOUNDARY      = 1,
        M0_WAIT_COUNTER_EQUAL_0     = 2,
        M0_WAIT_END_MELODY          = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_MELODY0_E2_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            trash[SC_MELODY_E2_MAXIMUM_HEADER_SIZE+1], oscillator_number, extension_index;
      UWORD16           oscillator_not_available;

      switch(*state)
      {
        case M0_INACTIVE:
        {
          // Reset the commands:
          l1a_l1s_com.melody0_e2_task.command.start = FALSE;

          // Initialize the pointer and size to the new description
          l1s.melody0_e2.ptr_buf = l1a_l1s_com.melody0_e2_task.parameters.ptr_buf;
          l1s.melody0_e2.buffer_size = l1a_l1s_com.melody0_e2_task.parameters.buffer_size;

          *state = M0_ALIGN_40MS_BOUNDARY;
        }
        break;

        case M0_ALIGN_40MS_BOUNDARY:
        {
          // Initialize the counter to the first time
          l1s.melody0_e2.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                               &l1s.melody0_e2.buffer_size,
                                                               (UWORD8 **)&l1s.melody0_e2.ptr_buf,
                                                               1,
                                                               (UWORD8 *)(&l1s.melody0_e2.counter));

          // Save the extension flag
          l1s.melody0_e2.extension_flag = Field(l1s.melody0_e2.counter,
                                                SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

          // Save delta-time in 20ms unit
          l1s.melody0_e2.counter = Field(l1s.melody0_e2.counter,
                                         SC_MELODY_E2_DELTA_TIME_MASK,
                                         SC_MELODY_E2_DELTA_TIME_SHIFT);
          l1s.melody0_e2.note_start_20ms = l1s.melody0_e2.counter;

          // Adjust note_start on 20ms boundary because global counter could be running for another melody or loopback
          // Timebase can be computed as k*60ms + timebase_mod_60ms
          l1s.melody0_e2.note_start_20ms += ((l1s.melody_e2.timebase - l1s.melody_e2.timebase_mod_60ms) / 13 * 3);
          if (l1s.melody_e2.timebase_mod_60ms < 4)
          {
            l1s.melody0_e2.note_start_20ms += 1;
          }
          else if (l1s.melody_e2.timebase_mod_60ms < 8)
          {
            l1s.melody0_e2.note_start_20ms += 2;
          }
          else if (l1s.melody_e2.timebase_mod_60ms < 13)
          {
            l1s.melody0_e2.note_start_20ms += 3;
          }

          // Align on 40ms boundary
          if ( (l1s.melody0_e2.note_start_20ms & 1) == 1 )
            l1s.melody0_e2.note_start_20ms++;

          // Convert to TDMA
          l1s.melody0_e2.counter = audio_twentyms_to_TDMA_convertion(l1s.melody0_e2.note_start_20ms);

          // Compute TDMA to wait (-1 to take into account this TDMA)
          l1s.melody0_e2.counter = l1s.melody0_e2.counter - l1s.melody_e2.timebase - 1;

          // Wait to download the first description
          *state = M0_WAIT_COUNTER_EQUAL_0;
        } // M0_INIT
        break;

        case M0_WAIT_COUNTER_EQUAL_0:
        {
          // Stop command
          if (l1a_l1s_com.melody0_e2_task.command.stop)
          {
            // wait until all the ocillator are stopped
            *state = M0_WAIT_END_MELODY;
          }
          else if (l1s.melody0_e2.counter > 0)
          {
            // Decrease the counter
            l1s.melody0_e2.counter--;
          }
          else
          {
            // Wait until the semaphore is set to 0 by the DSP
            if (!(l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore & SC_MELODY_E2_SEMAPHORE_MASK))
            {
              // Initialize oscillators available, oscillators can be used by the DSP (d_melody_e2_osc_active)
              // or by the other melody E2 generator (melody_e2_osc_stop)
              oscillator_not_available =
                l1s.melody_e2.global_osc_active | l1s.melody_e2.global_osc_to_start;

              // find an available oscillator
              oscillator_number = 0;
              while( (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR) &&
                     (oscillator_not_available & (0x0001<<oscillator_number)) )
              {
                oscillator_number++;
              }

              // Initialize end of file
              l1s.melody0_e2.end_of_file = FALSE;

              // download the description until the delta time is different from 0
              // or end of the melody file
              while ( (l1s.melody0_e2.counter == 0) &&
                      (l1s.melody0_e2.end_of_file == FALSE) )
              {
                // Download the byte of the note descriptor extension in trash
                l1s.melody0_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                  &l1s.melody0_e2.buffer_size,
                  (UWORD8 **)&l1s.melody0_e2.ptr_buf,
                  2,
                  &trash[0]);

                // Check end of melody
                if ( (trash[0] != 0x00) || (trash[1] != 0x00) )
                {
                  // It is not the end of melody

                  // If an oscillator is available, use it
                  if (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR)
                  {
                    // Reset the oscillator description
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][0] = 0x0000;
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][1] = 0x0000;
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][2] = 0x0000;

                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][0] =
                      trash[0] + (trash[1] << 8);

                    // Update the oscillators to start bit field
                    l1s.melody_e2.global_osc_to_start |= (0x0001<<oscillator_number);

                    // Save the oscillator as active for this melody
                    l1s.melody0_e2.oscillator_active |= (0x0001<<oscillator_number);

                    // oscillator is no longer available
                    oscillator_not_available |= (0x0001<<oscillator_number);
                  }

                  // Download the extensions
                  extension_index = 1;
                  while(l1s.melody0_e2.extension_flag)
                  {
                    // Download the byte of the note descriptor extension
                    l1s.melody0_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                      &l1s.melody0_e2.buffer_size,
                      (UWORD8 **)&l1s.melody0_e2.ptr_buf,
                      2,
                      &trash[0]);

                    // Read the extension flag
                    l1s.melody0_e2.extension_flag = Field(trash[0],
                                                          SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                          SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

                    // If an oscillator is available, use it
                    if (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR)
                    {
                      l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][extension_index] =
                        trash[0] + (trash[1] << 8);
                    }

                    extension_index++;
                  } // extension download

                  // find next available oscillator
                  while( (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR) &&
                         (oscillator_not_available & (0x0001<<oscillator_number)) )
                  {
                    oscillator_number++;
                  }

                  // Read next delta time
                  l1s.melody0_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                       &l1s.melody0_e2.buffer_size,
                                                                       (UWORD8 **)&l1s.melody0_e2.ptr_buf,
                                                                       1,
                                                                       (UWORD8 *)(&l1s.melody0_e2.counter));

                  // Save the extension flag
                  l1s.melody0_e2.extension_flag = Field(l1s.melody0_e2.counter,
                                                        SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                        SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

                  l1s.melody0_e2.counter = Field(l1s.melody0_e2.counter,
                                                 SC_MELODY_E2_DELTA_TIME_MASK,
                                                 SC_MELODY_E2_DELTA_TIME_SHIFT);
                } // if ( (trash[0] != 0x00) || (trash[1] != 0x00) )
                else
                {
                  // it's the end of the melody file
                  l1s.melody0_e2.end_of_file = TRUE;
                }
              } // while ( (l1s.melody0_e2.counter == 0) && (l1s.melody0_e2.end_of_file == FALSE) )

              // Perform TDMA convertion or handle end of file
              if (l1s.melody0_e2.end_of_file == FALSE)
              {
                // Update note start
                l1s.melody0_e2.note_start_20ms += l1s.melody0_e2.counter;

                l1s.melody0_e2.delta_time = l1s.melody0_e2.counter;

                // Convert the delta time into TDMA time unit
                l1s.melody0_e2.counter = audio_twentyms_to_TDMA_convertion(l1s.melody0_e2.note_start_20ms)
                                         - l1s.melody_e2.timebase;

                // decrease the counter
                l1s.melody0_e2.counter--;

                *state = M0_WAIT_COUNTER_EQUAL_0;
              }
              else
              {
                l1s.melody0_e2.delta_time = 0xFFFF;
                *state = M0_WAIT_END_MELODY;
              }
            } // semaphore check
          } // if (l1a_l1s_com.melody0_e2_task.command.stop) (2nd else)
        } // case M0_WAIT_COUNTER_EQUAL_0:
        break;

        case M0_WAIT_END_MELODY:
        {
          if (l1a_l1s_com.melody0_e2_task.command.stop)
          {
            // Stop immediatly the current melody
            // Wait until the semaphore is set to 0 by the DSP to stop the current description
            if (!(l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore & SC_MELODY_E2_SEMAPHORE_MASK))
            {
              // Stop the oscillator mentionned in the bits field melody0_e2.oscillator_active
              for(oscillator_number=0; oscillator_number<SC_MELODY_E2_NUMBER_OF_OSCILLATOR; oscillator_number++)
              {
                if (l1s.melody0_e2.oscillator_active & (0x0001<<oscillator_number))
                {
                  // Stop the current oscillator
                  l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_osc_stop |= (0x0001<<oscillator_number);
                }
              }
              // wait until all the oscillator are stopped
              l1a_l1s_com.melody0_e2_task.parameters.loopback = FALSE;
              l1a_l1s_com.melody0_e2_task.command.stop = FALSE;
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = 0xFFFF;// another melody could be running
            } // semaphore
          }
          else if (l1s.melody0_e2.oscillator_active == 0x0000)
          {
            // all oscillators are stopped
            if (l1a_l1s_com.melody0_e2_task.parameters.loopback)
            {
              // It's the loopback mode
              // Reset the pointer to the current melody
              #if (OP_RIV_AUDIO == 0)
              l1s.melody0_e2.ptr_buf = NULL;
              #endif
              l1s.melody0_e2.buffer_size = 0;
              l1s.melody0_e2.error_id = Cust_get_pointer((UWORD16 **)&l1s.melody0_e2.ptr_buf,
                                                      &l1s.melody0_e2.buffer_size,
                                                      l1a_l1s_com.melody0_e2_task.parameters.session_id);

              // Jump the header field
              l1s.melody0_e2.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                     &l1s.melody0_e2.buffer_size,
                                                     (UWORD8 **)&l1s.melody0_e2.ptr_buf,
                                                     (UWORD16)(l1a_l1s_com.melody0_e2_task.parameters.header_size),
                                                     &trash[0]);

              // Wait until the description can be downloaded
              *state = M0_ALIGN_40MS_BOUNDARY;
            }
            else
            {
              // Send the stop confirmation message
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_MELODY0_E2_STOP_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              // Go to inactive mode
              *state =  M0_INACTIVE;
            }
          }
          break;
        } // M0_WAIT_END_MELODY
      } // switch
    } // l1s_melody0_e2_manager

    /*-------------------------------------------------------*/
    /*    l1s_melody1_e2_manager()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Audio melody 0 format E2 L1S manager    */
    /*               task.                                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_melody1_e2_manager(void)
    {
      enum states
      {
        M1_INACTIVE                 = 0,
        M1_ALIGN_40MS_BOUNDARY      = 1,
        M1_WAIT_COUNTER_EQUAL_0     = 2,
        M1_WAIT_END_MELODY          = 3
      };

      UWORD8            *state      = &l1s.audio_state[L1S_MELODY1_E2_STATE];
      xSignalHeaderRec  *conf_msg;
      UWORD8            trash[SC_MELODY_E2_MAXIMUM_HEADER_SIZE+1], oscillator_number, extension_index;
      UWORD16           oscillator_not_available;

      switch(*state)
      {
        case M1_INACTIVE:
        {
          // Reset the commands:
          l1a_l1s_com.melody1_e2_task.command.start = FALSE;

          // Initialize the pointer and size to the new description
          l1s.melody1_e2.ptr_buf = l1a_l1s_com.melody1_e2_task.parameters.ptr_buf;
          l1s.melody1_e2.buffer_size = l1a_l1s_com.melody1_e2_task.parameters.buffer_size;

          *state = M1_ALIGN_40MS_BOUNDARY;
        }
        break;

        case M1_ALIGN_40MS_BOUNDARY:
        {
          // Initialize the counter to the first time
          l1s.melody1_e2.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                               &l1s.melody1_e2.buffer_size,
                                                               (UWORD8 **)&l1s.melody1_e2.ptr_buf,
                                                               1,
                                                               (UWORD8 *)(&l1s.melody1_e2.counter));

          // Save the extension flag
          l1s.melody1_e2.extension_flag = Field(l1s.melody1_e2.counter,
                                                SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

          // Save delta-time in 20ms unit
          l1s.melody1_e2.counter = Field(l1s.melody1_e2.counter,
                                         SC_MELODY_E2_DELTA_TIME_MASK,
                                         SC_MELODY_E2_DELTA_TIME_SHIFT);
          l1s.melody1_e2.note_start_20ms = l1s.melody1_e2.counter;

          // Adjust note_start on 20ms boundary because global counter could be running for another melody or loopback
          // Timebase can be computed as k*60ms + timebase_mod_60ms
          l1s.melody1_e2.note_start_20ms += ((l1s.melody_e2.timebase - l1s.melody_e2.timebase_mod_60ms) / 13 * 3);
          if (l1s.melody_e2.timebase_mod_60ms < 4)
          {
            l1s.melody1_e2.note_start_20ms += 1;
          }
          else if (l1s.melody_e2.timebase_mod_60ms < 8)
          {
            l1s.melody1_e2.note_start_20ms += 2;
          }
          else if (l1s.melody_e2.timebase_mod_60ms < 13)
          {
            l1s.melody1_e2.note_start_20ms += 3;
          }

          // Align on 40ms boundary
          if ( (l1s.melody1_e2.note_start_20ms & 1) == 1 )
            l1s.melody1_e2.note_start_20ms++;

          // Convert to TDMA
          l1s.melody1_e2.counter = audio_twentyms_to_TDMA_convertion(l1s.melody1_e2.note_start_20ms);

          // Compute TDMA to wait (-1 to take into account this TDMA)
          l1s.melody1_e2.counter = l1s.melody1_e2.counter - l1s.melody_e2.timebase - 1;

          // Wait to download the first description
          *state = M1_WAIT_COUNTER_EQUAL_0;
        } // M1_INIT
        break;

        case M1_WAIT_COUNTER_EQUAL_0:
        {
          // Stop command
          if (l1a_l1s_com.melody1_e2_task.command.stop)
          {
            // wait until all the ocillator are stopped
            *state = M1_WAIT_END_MELODY;
          }
          else if (l1s.melody1_e2.counter > 0)
          {
            // Decrease the counter
            l1s.melody1_e2.counter--;
          }
          else
          {
            // Wait until the semaphore is set to 0 by the DSP
            if (!(l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore & SC_MELODY_E2_SEMAPHORE_MASK))
            {
              // Initialize oscillators available, oscillators can be used by the DSP (d_melody_e2_osc_active)
              // or by the other melody E2 generator (melody_e2_osc_stop)
              oscillator_not_available =
                l1s.melody_e2.global_osc_active | l1s.melody_e2.global_osc_to_start;

              // find an available oscillator
              oscillator_number = 0;
              while( (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR) &&
                     (oscillator_not_available & (0x0001<<oscillator_number)) )
              {
                oscillator_number++;
              }

              // Initialize end of file
              l1s.melody1_e2.end_of_file = FALSE;

              // download the description until the delta time is different from 0
              // or end of the melody file
              while ( (l1s.melody1_e2.counter == 0) &&
                      (l1s.melody1_e2.end_of_file == FALSE) )
              {
                // Download the byte of the note descriptor extension in trash
                l1s.melody1_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                  &l1s.melody1_e2.buffer_size,
                  (UWORD8 **)&l1s.melody1_e2.ptr_buf,
                  2,
                  &trash[0]);

                // Check end of melody
                if ( (trash[0] != 0x00) || (trash[1] != 0x00) )
                {
                  // It is not the end of melody

                  // If an oscillator is available, use it
                  if (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR)
                  {
                    // Reset the oscillator description
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][0] = 0x0000;
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][1] = 0x0000;
                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][2] = 0x0000;

                    l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][0] =
                      trash[0] + (trash[1] << 8);

                    // Update the oscillators to start bit field
                    l1s.melody_e2.global_osc_to_start |= (0x0001<<oscillator_number);

                    // Save the oscillator as active for this melody
                    l1s.melody1_e2.oscillator_active |= (0x0001<<oscillator_number);

                    // oscillator is no longer available
                    oscillator_not_available |= (0x0001<<oscillator_number);
                  }

                  // Download the extensions
                  extension_index = 1;
                  while(l1s.melody1_e2.extension_flag)
                  {
                    // Download the byte of the note descriptor extension
                    l1s.melody1_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                      &l1s.melody1_e2.buffer_size,
                      (UWORD8 **)&l1s.melody1_e2.ptr_buf,
                      2,
                      &trash[0]);

                    // Read the extension flag
                    l1s.melody1_e2.extension_flag = Field(trash[0],
                                                          SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                          SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

                    // If an oscillator is available, use it
                    if (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR)
                    {
                      l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[oscillator_number][extension_index] =
                        trash[0] + (trash[1] << 8);
                    }

                    extension_index++;
                  } // extension download

                  // find next available oscillator
                  while( (oscillator_number < SC_MELODY_E2_NUMBER_OF_OSCILLATOR) &&
                         (oscillator_not_available & (0x0001<<oscillator_number)) )
                  {
                    oscillator_number++;
                  }

                  // Read next delta time
                  l1s.melody1_e2.error_id= copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                       &l1s.melody1_e2.buffer_size,
                                                                       (UWORD8 **)&l1s.melody1_e2.ptr_buf,
                                                                       1,
                                                                       (UWORD8 *)(&l1s.melody1_e2.counter));

                  // Save the extension flag
                  l1s.melody1_e2.extension_flag = Field(l1s.melody1_e2.counter,
                                                        SC_MELODY_E2_EXTENSION_FLAG_MASK,
                                                        SC_MELODY_E2_EXTENSION_FLAG_SHIFT);

                  l1s.melody1_e2.counter = Field(l1s.melody1_e2.counter,
                                                 SC_MELODY_E2_DELTA_TIME_MASK,
                                                 SC_MELODY_E2_DELTA_TIME_SHIFT);
                } // if ( (trash[0] != 0x00) || (trash[1] != 0x00) )
                else
                {
                  // it's the end of the melody file
                  l1s.melody1_e2.end_of_file = TRUE;
                }
              } // while ( (l1s.melody1_e2.counter == 0) && (l1s.melody1_e2.end_of_file == FALSE) )

              // Perform TDMA convertion or handle end of file
              if (l1s.melody1_e2.end_of_file == FALSE)
              {
                // Update note start
                l1s.melody1_e2.note_start_20ms += l1s.melody1_e2.counter;

                l1s.melody1_e2.delta_time = l1s.melody1_e2.counter;

                // Convert the delta time into TDMA time unit
                l1s.melody1_e2.counter = audio_twentyms_to_TDMA_convertion(l1s.melody1_e2.note_start_20ms)
                                         - l1s.melody_e2.timebase;

                // decrease the counter
                l1s.melody1_e2.counter--;

                *state = M1_WAIT_COUNTER_EQUAL_0;
              }
              else
              {
                l1s.melody1_e2.delta_time = 0xFFFF;
                *state = M1_WAIT_END_MELODY;
              }
            } // semaphore check
          } // if (l1a_l1s_com.melody1_e2_task.command.stop) (2nd else)
        } // case M1_WAIT_COUNTER_EQUAL_0:
        break;

        case M1_WAIT_END_MELODY:
        {
          if (l1a_l1s_com.melody1_e2_task.command.stop)
          {
            // Stop immediatly the current melody
            // Wait until the semaphore is set to 0 by the DSP to stop the current description
            if (!(l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore & SC_MELODY_E2_SEMAPHORE_MASK))
            {
              // Stop the oscillator mentionned in the bits field melody1_e2.oscillator_active
              for(oscillator_number=0; oscillator_number<SC_MELODY_E2_NUMBER_OF_OSCILLATOR; oscillator_number++)
              {
                if (l1s.melody1_e2.oscillator_active & (0x0001<<oscillator_number))
                {
                  // Stop the current oscillator
                  l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_osc_stop |= (0x0001<<oscillator_number);
                }
              }
              // wait until all the oscillator are stopped
              l1a_l1s_com.melody1_e2_task.parameters.loopback = FALSE;
              l1a_l1s_com.melody1_e2_task.command.stop = FALSE;
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = 0xFFFF; // another melody could be running
            } // semaphore
          }
          else if (l1s.melody1_e2.oscillator_active == 0x0000)
          {
            // all oscillators are stopped
            if (l1a_l1s_com.melody1_e2_task.parameters.loopback)
            {
              // It's the loopback mode
              // Reset the pointer to the current melody
              #if (OP_RIV_AUDIO == 0)
              l1s.melody1_e2.ptr_buf = NULL;
              #endif
              l1s.melody1_e2.buffer_size = 0;
              l1s.melody1_e2.error_id = Cust_get_pointer((UWORD16 **)&l1s.melody1_e2.ptr_buf,
                                                      &l1s.melody1_e2.buffer_size,
                                                      l1a_l1s_com.melody1_e2_task.parameters.session_id);

              // Jump the header field
              l1s.melody1_e2.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                     &l1s.melody1_e2.buffer_size,
                                                     (UWORD8 **)&l1s.melody1_e2.ptr_buf,
                                                     (UWORD16)(l1a_l1s_com.melody1_e2_task.parameters.header_size),
                                                     &trash[0]);

              // Wait until the description can be downloaded
              *state = M1_ALIGN_40MS_BOUNDARY;
            }
            else
            {
              // Send the stop confirmation message
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_MELODY1_E2_STOP_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)

              // Go to inactive mode
              *state =  M1_INACTIVE;
            }
          }
          break;
        } // M1_WAIT_END_MELODY
      } // switch
    } // l1s_melody1_e2_manager

  #endif // MELODY_E2

  #if (L1_CPORT == 1)
    /*-------------------------------------------------------*/
    /* l1s_cport_manager()                                   */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Cport L1S manager task.                 */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_cport_manager(void)
    {
      enum states
      {
        IDLE                 = 0,
        WAIT_CPORT_CONF      = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_CPORT_STATE];
      xSignalHeaderRec  *conf_msg;
      API               temp_write_var;

      switch(*state)
      {
        case IDLE:
        {
          // Check if a cport configuration is not yet in progress in DSP
          if (l1s_dsp_com.dsp_ndb_ptr->d_cport_status  == (API) (CPORT_R_NONE | CPORT_W_NONE))
          {
            // ok, we can start the DSP Cport configuration task
            if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CTRL)
                l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl = l1a_l1s_com.cport_task.parameters.ctrl;

            if (l1a_l1s_com.cport_task.parameters.configuration & (CPORT_W_CPCFR1 | CPORT_W_CPCFR2))
            {
               temp_write_var = 0;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPCFR1)
                 temp_write_var = l1a_l1s_com.cport_task.parameters.cpcfr1 << 8;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPCFR2)
                 temp_write_var |= l1a_l1s_com.cport_task.parameters.cpcfr2;

               l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[0] = temp_write_var;
            }


            if (l1a_l1s_com.cport_task.parameters.configuration & (CPORT_W_CPCFR3 | CPORT_W_CPCFR4))
            {
               temp_write_var = 0;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPCFR3)
                 temp_write_var = l1a_l1s_com.cport_task.parameters.cpcfr3 << 8;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPCFR4)
                 temp_write_var |= l1a_l1s_com.cport_task.parameters.cpcfr4;

               l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[1] = temp_write_var;
            }


            if (l1a_l1s_com.cport_task.parameters.configuration & (CPORT_W_CPTCTL | CPORT_W_CPTTADDR))
            {
               temp_write_var = 0;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPTCTL)
                 temp_write_var = l1a_l1s_com.cport_task.parameters.cptctl << 8;

               if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPTTADDR)
                 temp_write_var |= l1a_l1s_com.cport_task.parameters.cpttaddr;

               l1s_dsp_com.dsp_ndb_ptr->d_cport_tcl_tadt = temp_write_var;
            }


            if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPTDAT)
                l1s_dsp_com.dsp_ndb_ptr->d_cport_tdat     = l1a_l1s_com.cport_task.parameters.cptdat;

            if (l1a_l1s_com.cport_task.parameters.configuration & CPORT_W_CPTVS)
                l1s_dsp_com.dsp_ndb_ptr->d_cport_tvs      = l1a_l1s_com.cport_task.parameters.cptvs;

            l1s_dsp_com.dsp_ndb_ptr->d_cport_init     = l1a_l1s_com.cport_task.parameters.configuration;

            *state = WAIT_CPORT_CONF;

            // Reset the command
            l1a_l1s_com.cport_task.command.start = FALSE;

          }

          // else, we do nothing -> check will be done again at next frame
        }
        break;

        case WAIT_CPORT_CONF:
        {
          // the DSP acknowledges the L1S start request.
          if (l1s_dsp_com.dsp_ndb_ptr->d_cport_init
              == l1s_dsp_com.dsp_ndb_ptr->d_cport_status)
          {
            // task is over
            l1s_dsp_com.dsp_ndb_ptr->d_cport_init      = (API) (CPORT_R_NONE | CPORT_W_NONE);
            l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl      = (API) 0;
            l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[0]    = (API) 0;
            l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[1]    = (API) 0;
            l1s_dsp_com.dsp_ndb_ptr->d_cport_tcl_tadt  = (API) 0;
            l1s_dsp_com.dsp_ndb_ptr->d_cport_tdat      = (API) 0;
            l1s_dsp_com.dsp_ndb_ptr->d_cport_tvs       = (API) 0;

            // Send the configuration confirmation message
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(sizeof(T_L1_CPORT_CONFIGURE_CON));
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_CPORT_CONFIGURE_CON;

            //Fill the message
            ((T_L1_CPORT_CONFIGURE_CON *)(conf_msg->SigP))->register_id    = (l1s_dsp_com.dsp_ndb_ptr->d_cport_status & CPORT_READ_MASK);
            ((T_L1_CPORT_CONFIGURE_CON *)(conf_msg->SigP))->register_value = l1s_dsp_com.dsp_ndb_ptr->d_cport_reg_value;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;

      } // switch
    }
  #endif // L1_CPORT == 1

    /*-------------------------------------------------------*/
    /*    l1s_audio_it_manager()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Audio it manager                        */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_audio_it_manager(void)
    {
      // Reset the command :
      l1a_l1s_com.audioIt_task.command.start = FALSE;

      // this is an empty state machin only used to generate an
      // audio IT to DSP in case another sw entity has changed
      // something in the API
    }


  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
    /*-------------------------------------------------------*/
    /* l1s_audio_onoff_manager()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : audio on/off L1S manager task.          */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_audio_onoff_manager(void)
    {
      enum states
      {
        IDLE                   = 0,
        WAIT_AUDIO_ONOFF_CON   = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AUDIO_ONOFF_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Disable the start command
          l1a_l1s_com.audio_onoff_task.command.start = FALSE;

          // Update the audio on/off value except if the L1S is already forcing it
          if ((l1a_l1s_com.audio_onoff_task.parameters.onoff_value == TRUE) &&
             (l1a_l1s_com.audio_forced_by_l1s == FALSE))
             // l1a_l1s_com.audio_onoff_task.parameters.onoff_value == AUDIO_ON
          {
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_ON_START;
          }
          else if ((l1a_l1s_com.audio_onoff_task.parameters.onoff_value == FALSE) &&
             (l1a_l1s_com.audio_forced_by_l1s == FALSE))
             // l1a_l1s_com.audio_onoff_task.parameters.onoff_value == AUDIO_OFF
          {
            l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init |= (API) B_AUDIO_OFF_STOP;
          }

          *state = WAIT_AUDIO_ONOFF_CON;
        }
        break;

        case WAIT_AUDIO_ONOFF_CON:
        {
          // The L1 has to send the confirmation message even if the request was to disable the audio on/off
          // and it is still forced. This confirmation message is only to acknowledge the reception of the message,
          // it is not correlated to the state of the audio into the DSP.

          // Allocate memory for confirmation message...
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1_AUDIO_ONOFF_CON;
          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          *state = IDLE;
        }
        break;

      } // switch
    }
  #endif

  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    /*-------------------------------------------------------*/
    /* l1s_audio_onoff_manager()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : audio on/off L1S manager task.          */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_audio_voice_onoff_manager(void)
    {
      enum states
      {
        IDLE                   = 0,
        WAIT_AUDIO_ONOFF_CON   = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AUDIO_ONOFF_STATE];
      UWORD8            *ul_state      = &l1s.audio_state[L1S_AUDIO_UL_ONOFF_STATE];
      UWORD8            *dl_state      = &l1s.audio_state[L1S_AUDIO_DL_ONOFF_STATE];
      UWORD8		*vul_state     = &l1a_l1s_com.audio_onoff_task.parameters.vul_onoff_value;
      UWORD8		*vdl_state     = &l1a_l1s_com.audio_onoff_task.parameters.vdl_onoff_value;
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          // Disable the start command
          l1a_l1s_com.audio_onoff_task.command.start = FALSE;

          // Update the Voice Uplink count
	  if(l1a_l1s_com.audio_onoff_task.parameters.vul_onoff_value == L1_AUDIO_VOICE_UL_ON)
	  {
	  	l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request++;
	  }
	  else if(l1a_l1s_com.audio_onoff_task.parameters.vul_onoff_value == L1_AUDIO_VOICE_UL_OFF )
	  {
		if(l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request)
			l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request--;
	  }

	  // Update the Voice Downlink count
	  if(l1a_l1s_com.audio_onoff_task.parameters.vdl_onoff_value == L1_AUDIO_VOICE_DL_ON)
	  {
	    	l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request++;
	  }
	  else if(l1a_l1s_com.audio_onoff_task.parameters.vdl_onoff_value == L1_AUDIO_VOICE_DL_OFF)
	  {
	  	if(l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request)
			l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request--;
	  }

          *state = WAIT_AUDIO_ONOFF_CON;
        }
        break;

        case WAIT_AUDIO_ONOFF_CON:
        {
          // For Voice Uplink or Downlink switch on is done only after the VUL or VDL is actually switched ON
	  // For switch off there could be potentially other tasks that use the VUL or VDL and hence a blind
	  // confirmation is given
	  if( (((*vul_state == L1_AUDIO_VOICE_UL_ON) && (*ul_state == L1_AUDIO_UL_ON)) ||
	      (*vul_state == L1_AUDIO_VOICE_UL_OFF) || (*vul_state == L1_AUDIO_VOICE_UL_NO_ACTION)) &&
		(((*vdl_state == L1_AUDIO_VOICE_DL_ON) && (*dl_state == L1_AUDIO_DL_ON)) ||
	      (*vdl_state == L1_AUDIO_VOICE_DL_OFF) || (*vdl_state == L1_AUDIO_VOICE_DL_NO_ACTION)))
	  {
	  	// The L1 has to send the confirmation message even if the request was to disable the audio on/off
          	// and it is still forced. This confirmation message is only to acknowledge the reception of the message,
          	// it is not correlated to the state of the audio into the DSP.

          	// Allocate memory for confirmation message...
          	conf_msg = os_alloc_sig(0);
          	DEBUGMSG(status,NU_ALLOC_ERR)
          	conf_msg->SignalCode = L1_AUDIO_ONOFF_CON;
          	// Send confirmation message...
          	os_send_sig(conf_msg, L1C1_QUEUE);
          	DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          	*state = IDLE;
	  }
        }
        break;

      } // switch
    }
  #endif

  #if (L1_STEREOPATH == 1)
    /*-------------------------------------------------------*/
    /* l1s_stereopath_drv_manager()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : Generic Stereopath L1S manager task.    */
    /*                                                       */
    /*-------------------------------------------------------*/
#if (CODE_VERSION == SIMULATION)
    void l1s_stereopath_drv_manager(void)
    {
      enum states
      {
        IDLE=0,
        WAIT_STOP
      };

      xSignalHeaderRec *conf_msg;

      if(l1a_l1s_com.stereopath_drv_task.command.start==TRUE)
      {
        // reset the command
        l1a_l1s_com.stereopath_drv_task.command.start=FALSE;

        // change state
        l1s.audio_state[L1S_STEREOPATH_DRV_STATE]=WAIT_STOP;

        // send confirmation to the L1A
        conf_msg=os_alloc_sig(0);
        DEBUGMSG(status,NU_ALLOC_ERR)
        conf_msg->SignalCode=L1_STEREOPATH_DRV_START_CON;
        os_send_sig(conf_msg,L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }

      if(l1a_l1s_com.stereopath_drv_task.command.stop==TRUE)
      {
        // reset the command
        l1a_l1s_com.stereopath_drv_task.command.stop=FALSE;

        // change state
        l1s.audio_state[L1S_STEREOPATH_DRV_STATE]=IDLE;

        // send confirmation to the L1A
        conf_msg=os_alloc_sig(0);
        DEBUGMSG(status,NU_ALLOC_ERR)
        conf_msg->SignalCode=L1_STEREOPATH_DRV_STOP_CON;
        os_send_sig(conf_msg,L1C1_QUEUE);
        DEBUGMSG(status,NU_SEND_QUEUE_ERR)
      }
    }
#else   // CODE_VERSION
    void l1s_stereopath_drv_manager(void)
    {
    //sundi: change in the enum values for the ABB states
#if (ANALOG == 11)
      enum states
      {
        IDLE                		   = 0,
	ABB_CONFIGURE_DONE  = 1,
        ABB_START                       = 2,
	ABB_START_DONE           = 3,
        DMA_CONF                        = 4,
        CPORT_CONF                    = 5,
        CPORT_START                  = 6,
        WAIT_STOP                      = 7,
        CPORT_STOP                    = 8,
	ABB_STOP_DONE             = 9,
        DMA_STOP                        = 10,
        STOP_CON                         = 11
      };
#else
      enum states
      {
        IDLE                = 0,
        ABB_START           = 1,
        DMA_CONF            = 2,
        CPORT_CONF          = 3,
        CPORT_START         = 4,
        WAIT_STOP           = 5,
        CPORT_STOP          = 6,
        DMA_STOP            = 7,
        STOP_CON            = 8
      };
#endif

      UWORD8            *state              = &l1s.audio_state[L1S_STEREOPATH_DRV_STATE];
      xSignalHeaderRec  *conf_msg;
      static UWORD16    wait_pll_on_counter = L1S_STEREOPATH_DRV_WAIT_PLL_COUNTER;

      // This is the default parameters structure for a DMA channel
      static T_DMA_TYPE_CHANNEL_PARAMETER d_dma_channel_parameter=
      {
        f_dma_default_call_back_it,       // call back function
        C_DMA_CHANNEL_2,                  // channel number
        C_DMA_CHANNEL_NOT_SECURED,        // channel security
        C_DMA_DATA_S16,                   // data type
        C_DMA_IMIF_PORT,                  // source port
        C_DMA_CHANNEL_NOT_PACKED,         // source packing
        C_DMA_CHANNEL_SINGLE,             // source bursting
        C_DMA_RHEA_PORT,                  // destination port
        C_DMA_CHANNEL_NOT_PACKED,         // destination packing
        C_DMA_CHANNEL_SINGLE,             // destination bursting
        C_DMA_CHANNEL_CPORT_TX,           // hw synchro
        C_DMA_CHANNEL_PRIORITY_HIGH,      // channel priority
        C_DMA_CHANNEL_AUTO_INIT_ON,       // autoinit option
        C_DMA_CHANNEL_FIFO_FLUSH_OFF,     // fifo flush option
        C_DMA_CHANNEL_ADDR_MODE_POST_INC, // source addressing mode
        C_DMA_CHANNEL_ADDR_MODE_CONSTANT, // destination addressing mode
        C_DMA_CHANNEL_IT_TIME_OUT_ON,     // IT time out control
        C_DMA_CHANNEL_IT_DROP_ON,         // IT drop control
        C_DMA_CHANNEL_IT_FRAME_OFF,       // IT frame control
        C_DMA_CHANNEL_IT_BLOCK_ON,        // IT block control
        C_DMA_CHANNEL_IT_HALF_BLOCK_ON,   // IT half_block control
        0,                                // source start address
        0xFFFFD800L,                      // destination start address
        2,                                // element number
        0                                 // frame number
      };

      switch(*state)
      {
        case IDLE:
        {
#if (ANALOG == 11)
	   //Sundi:Set the abb_write_done variable to 0. This variable gets set in I2C call back fn.
	   l1s.abb_write_done = 0;
#endif
          // Configure the ABB audio part in order to get the correct clock for the Cport
          l1s_stereopath_drv_config_ABB(l1a_l1s_com.stereopath_drv_task.parameters.mono_stereo,
                                        l1a_l1s_com.stereopath_drv_task.parameters.sampling_frequency);

#if (ANALOG == 11)
          // Must wait for the PLL to be locked. The value of 1 is subtracted from the standard value
          // in order to take care of the extra state added - ABB_CONFIGURE_DONE.
          wait_pll_on_counter = L1S_STEREOPATH_DRV_WAIT_PLL_COUNTER - 1;
#else
          // Must wait for the PLL to be locked.
          wait_pll_on_counter = L1S_STEREOPATH_DRV_WAIT_PLL_COUNTER;

#endif

          // Reset the command
          l1a_l1s_com.stereopath_drv_task.command.start = FALSE;
#if (ANALOG == 11)
          //sundi: change the state to ABB_CONFIGURE_DONE to wait for I2C write ACK.
          *state = ABB_CONFIGURE_DONE;

#else
         *state  = ABB_START;
#endif
        }
        break;
#if (ANALOG == 11)
	//sundi: add the new state
	case ABB_CONFIGURE_DONE:
	{
		//sundi: continue to be in this state till an I2C write ACK comes.
		//abb_write_done is set to 1 by the call back function
		//that is passed to the I2C write function.
		if (l1s.abb_write_done == 1)
		{
		    *state = ABB_START;

		}
	}
       break;
#endif
        case ABB_START:
        {
          if (wait_pll_on_counter == 0)
          {
#if (ANALOG == 11)
            //Sundi:Set the abb_write_done variable to 0. This variable gets set in I2C call back fn.
            l1s.abb_write_done = 0;
#endif
            // PLL is locked, the ABB can be started
            l1s_stereopath_drv_start_ABB();
#if (ANALOG == 11)
            //sundi: Change state to ABB_START_DONE
            *state = ABB_START_DONE;
#else
            *state = DMA_CONF;
#endif
          }
          else
            wait_pll_on_counter--;
        }
        break;
#if (ANALOG == 11)
        //sundi: Add the new state ABB_START_DONE
	 case ABB_START_DONE:
	 {
	 	//sundi: continue to be in this state till an I2C write ACK comes.
		//abb_write_done is set to 1 by the call back function
		//that is passed to the I2C write function.
	 	if (l1s.abb_write_done == 1)
	 	{
	 	     *state = DMA_CONF;

	 	}
	 }
         break;
#endif
        // After 1 TDMA frame, DSP has necessarily programmed ABB
        case DMA_CONF:
        {
          // update the DMA defaut parameters structure with received parameters
          d_dma_channel_parameter.pf_dma_call_back_address     = (T_DMA_CALL_BACK)              l1a_l1s_com.stereopath_drv_task.parameters.DMA_int_callback_fct;
          d_dma_channel_parameter.d_dma_channel_number         = (T_DMA_TYPE_CHANNEL_NUMBER)    l1a_l1s_com.stereopath_drv_task.parameters.DMA_channel_number;
          d_dma_channel_parameter.d_dma_channel_data_type      = (T_DMA_TYPE_CHANNEL_DATA_TYPE) l1a_l1s_com.stereopath_drv_task.parameters.data_type;
          d_dma_channel_parameter.d_dma_channel_src_port       = (T_DMA_TYPE_CHANNEL_PORT)      l1a_l1s_com.stereopath_drv_task.parameters.source_port;
          d_dma_channel_parameter.d_dma_channel_src_address    = (SYS_UWORD32)                  l1a_l1s_com.stereopath_drv_task.parameters.source_buffer_address;
          d_dma_channel_parameter.d_dma_channel_element_number = (SYS_UWORD16)                  l1a_l1s_com.stereopath_drv_task.parameters.element_number;
          d_dma_channel_parameter.d_dma_channel_frame_number   = (SYS_UWORD16)                  l1a_l1s_com.stereopath_drv_task.parameters.frame_number;

          // Configure and start the DMA channel
          l1s_stereopath_drv_start_DMA(d_dma_channel_parameter,l1a_l1s_com.stereopath_drv_task.parameters.DMA_allocation);

          // Reset the Cport
          l1s_stereopath_drv_reset_CPORT();

          *state = CPORT_CONF;

        }
        break;

        case CPORT_CONF:
        {
          // Configure the Cport
          l1s_stereopath_drv_config_CPORT();

          *state = CPORT_START;

        }
        break;

        case CPORT_START:
        {
          // Start the Cport
          l1s_stereopath_drv_start_CPORT();

          // Send the start confirmation message
          // Allocate confirmation message...
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1_STEREOPATH_DRV_START_CON;
          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR)

          *state = WAIT_STOP;
        }
        break;

        case WAIT_STOP:
        {
           /* OUTEN registers have been updated */
          if(l1a_l1s_com.outen_cfg_task.command_requested != l1a_l1s_com.outen_cfg_task.command_commited)
          {
            l1s_stereopath_callback(L1S_TWL3029_STEROPATH_START);
          }
          if (l1a_l1s_com.stereopath_drv_task.command.stop)
          {
            // Stop command received

            // Reset the Cport
            l1s_stereopath_drv_reset_CPORT();

            *state = CPORT_STOP;

            // Disable the stop command
            l1a_l1s_com.stereopath_drv_task.command.stop = FALSE;
          }
        }
        break;

        case CPORT_STOP:
        {
          // Stop the Cport
          l1s_stereopath_drv_stop_CPORT();
#if (ANALOG == 11)
	   //Set the abb_write_done variable to 0. This variable gets set in I2C call back fn.
	   l1s.abb_write_done = 0;
#endif
          // Stop the ABB
          l1s_stereopath_drv_stop_ABB();
#if (ANALOG == 11)
          //sundi: change state to ABB_STOP_DONE
          *state = ABB_STOP_DONE;
#else
          *state = DMA_STOP;
#endif
        }
        break;
#if (ANALOG == 11)
	//sundi: Add a new state ABB_STOP_DONE
	 case ABB_STOP_DONE:
	 {
	 	//sundi:continue to be in this state till an I2C write ACK comes.
		//abb_write_done is set to 1 by the call back function
		//that is passed to the I2C write function.
	 	if (l1s.abb_write_done == 1)
	 	{
	 	    *state = DMA_STOP;
	 	}
	 }
         break;
#endif
        case DMA_STOP:
        {
          // Reset the DMA channel
          l1s_stereopath_drv_reset_DMA(d_dma_channel_parameter);

          *state = STOP_CON;
        }
        break;

        case STOP_CON:
        {
          // Send the stop confirmation message
          // Allocate confirmation message...
          conf_msg = os_alloc_sig(0);
          DEBUGMSG(status,NU_ALLOC_ERR)
          conf_msg->SignalCode = L1_STEREOPATH_DRV_STOP_CON;
          // Send confirmation message...
          os_send_sig(conf_msg, L1C1_QUEUE);
          DEBUGMSG(status,NU_SEND_QUEUE_ERR);

          *state = IDLE;
        }
        break;

      } // switch
    }
#endif    // CODE_VERSION
    #endif // L1_STEREOPATH == 1

  #if (L1_ANR == 1)
    /*-------------------------------------------------------*/
    /* l1s_anr_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : ANR L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_anr_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_ANR_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.anr_task.parameters.anr_enable)
          {
            // ANR start requested
            //--------------------

            // Set ANR parameters
            l1s_dsp_com.dsp_ndb_ptr->d_anr_min_gain         = l1a_l1s_com.anr_task.parameters.min_gain;
            l1s_dsp_com.dsp_ndb_ptr->d_anr_div_factor_shift = l1a_l1s_com.anr_task.parameters.div_factor_shift;
            l1s_dsp_com.dsp_ndb_ptr->d_anr_ns_level         = l1a_l1s_com.anr_task.parameters.ns_level;
#if (DSP == 38) || (DSP == 39)
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_UL_STATE)
#else
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_STATE)
#endif
            {
              // ANR already started: update the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_FULL_UPDATE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Set ANR constants
              l1s_dsp_com.dsp_ndb_ptr->d_anr_vad_thr          = C_ANR_VAD_THR;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_slow       = C_ANR_GAMMA_SLOW;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_fast       = C_ANR_GAMMA_FAST;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_gain_slow  = C_ANR_GAMMA_GAIN_SLOW;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_gamma_gain_fast  = C_ANR_GAMMA_GAIN_FAST;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_thr2             = C_ANR_THR2;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_thr4             = C_ANR_THR4;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_thr5             = C_ANR_THR5;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr1  = C_ANR_MEAN_RATIO_THR1;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr2  = C_ANR_MEAN_RATIO_THR2;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr3  = C_ANR_MEAN_RATIO_THR3;
              l1s_dsp_com.dsp_ndb_ptr->d_anr_mean_ratio_thr4  = C_ANR_MEAN_RATIO_THR4;

              // Enable the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_ENABLE;

              *state = WAIT_DSP_ACK;
            }
          }
          else // ANR start requested
          {
            // ANR stop requested
            //-------------------
#if (DSP == 38) || (DSP == 39)
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_UL_STATE)
#else
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_STATE)
#endif
            {
              // Disable the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_DISABLE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // ANR already disabled: confirm
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_ANR_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.anr_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_ANR_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_ANR == 1

  #if (L1_ANR == 2)
    /*-------------------------------------------------------*/
    /* l1s_anr_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : ANR 2.13 L1S manager task.              */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_anr_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state        = &l1s.audio_state[L1S_ANR_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.anr_task.parameters.anr_ul_control == ANR_START ||
			  l1a_l1s_com.anr_task.parameters.anr_ul_control == ANR_UPDATE )
          {
            // ANR start or update requested
            //------------------------------

            // Set ANR parameters
            l1s_dsp_com.dsp_ndb_ptr->d_anr_control          = l1a_l1s_com.anr_task.parameters.control;
            l1s_dsp_com.dsp_ndb_ptr->d_anr_ns_level         = l1a_l1s_com.anr_task.parameters.ns_level;
            l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_ene_th      = l1a_l1s_com.anr_task.parameters.tone_ene_th;
            l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_cnt_th      = l1a_l1s_com.anr_task.parameters.tone_cnt_th;

            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_UL_STATE)
            {
              // ANR already started: update the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_FULL_UPDATE;
              l1s.anr_ul_action                      = ANR_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_ENABLE;
              l1s.anr_ul_action                      = ANR_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }


          if (l1a_l1s_com.anr_task.parameters.anr_ul_control == ANR_STOP)
          {
            // ANR stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ANR_UL_STATE)
            {
              // Disable the DSP ANR module
              l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl = B_ANR_DISABLE;
              l1s.anr_ul_action                      = ANR_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // ANR already disabled: confirm
              // Allocate confirmation message...
              l1s.anr_ul_action                      = ANR_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_ANR_CON;
              ((T_L1_AQI_ANR_CON *)(conf_msg->SigP))->anr_ul_action = l1s.anr_ul_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.anr_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_ANR_CON;
            ((T_L1_AQI_ANR_CON *)(conf_msg->SigP))->anr_ul_action = l1s.anr_ul_action;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_ANR == 2

  #if (L1_IIR == 1)
    /*-------------------------------------------------------*/
    /* l1s_iir_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : IIR L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_iir_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_IIR_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.iir_task.parameters.iir_enable)
          {
            UWORD8 i;

            // IIR start requested
            //--------------------

            // Set IIR parameters
            l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_iir_blocks = l1a_l1s_com.iir_task.parameters.nb_iir_blocks;

            for (i=0; i < (l1a_l1s_com.iir_task.parameters.nb_iir_blocks * 8); i++)
              l1s_dsp_com.dsp_ndb_ptr->a_iir_iir_coefs[i] = l1a_l1s_com.iir_task.parameters.iir_coefs[i];

            l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_fir_coefs = l1a_l1s_com.iir_task.parameters.nb_fir_coefs;

            for (i=0; i < l1a_l1s_com.iir_task.parameters.nb_fir_coefs; i++)
              l1s_dsp_com.dsp_ndb_ptr->a_iir_fir_coefs[i] = l1a_l1s_com.iir_task.parameters.fir_coefs[i];

            l1s_dsp_com.dsp_ndb_ptr->d_iir_input_scaling       = l1a_l1s_com.iir_task.parameters.input_scaling;
            l1s_dsp_com.dsp_ndb_ptr->d_iir_fir_scaling         = l1a_l1s_com.iir_task.parameters.fir_scaling;
            l1s_dsp_com.dsp_ndb_ptr->d_iir_input_gain_scaling  = l1a_l1s_com.iir_task.parameters.input_gain_scaling;
            l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain_scaling = l1a_l1s_com.iir_task.parameters.output_gain_scaling;
            l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain         = l1a_l1s_com.iir_task.parameters.output_gain;
            l1s_dsp_com.dsp_ndb_ptr->d_iir_feedback            = l1a_l1s_com.iir_task.parameters.feedback;

#if (DSP == 38) || (DSP == 39)
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_DL_STATE)
#else
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_STATE)
#endif
            {
              // IIR already started: update the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_FULL_UPDATE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_ENABLE;

              *state = WAIT_DSP_ACK;
            }
          }
          else // IIR start requested
          {
            // IIR stop requested
            //-------------------
#if (DSP == 38) || (DSP == 39)
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_DL_STATE)
#else
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_STATE)
#endif
            {
              // Disable the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_DISABLE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // IIR already disabled: confirm
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_IIR_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.iir_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_IIR_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_IIR == 1

  #if (L1_AGC_UL == 1)
    /*-------------------------------------------------------*/
    /* l1s_agc_ul_manager()                                  */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : AGC_UL L1S manager task.                */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_agc_ul_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AGC_UL_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
		        {
        case IDLE:
        {
          if (l1a_l1s_com.agc_ul_task.parameters.agc_ul_control == AGC_START ||
		      l1a_l1s_com.agc_ul_task.parameters.agc_ul_control == AGC_UPDATE)
          {
            // AGC_UL start or update requested
            //---------------------------------

            // Set AGC UL parameters

            l1_audio_agc_ul_copy_params();

            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_AGC_UL_STATE)
            {
              // AGC UL already started: update the DSP AGC UL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl = B_AGC_FULL_UPDATE;
              l1s.agc_ul_action                      = AGC_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {

              // Enable the DSP AGC UL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl = B_AGC_ENABLE;
              l1s.agc_ul_action                      = AGC_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }

          if (l1a_l1s_com.agc_ul_task.parameters.agc_ul_control == AGC_STOP)
          {
            // AGC UL stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_AGC_UL_STATE)
            {
              // Disable the DSP AGC UL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl = B_AGC_DISABLE;
              l1s.agc_ul_action                      = AGC_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // AGC UL already disabled: confirm
              // Allocate confirmation message...
              l1s.agc_ul_action                      = AGC_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_AGC_UL_CON;
              ((T_L1_AQI_AGC_UL_CON *)(conf_msg->SigP))->agc_ul_action = l1s.agc_ul_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.agc_ul_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_AGC_UL_CON;
            ((T_L1_AQI_AGC_UL_CON *)(conf_msg->SigP))->agc_ul_action = l1s.agc_ul_action;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_AGC_UL == 1


  #if (L1_AGC_DL == 1)
    /*-------------------------------------------------------*/
    /* l1s_agc_dl_manager()                                  */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : AGC DL L1S manager task.                */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_agc_dl_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_AGC_DL_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
	  {
        case IDLE:
        {
          if (l1a_l1s_com.agc_dl_task.parameters.agc_dl_control == AGC_START ||
			  l1a_l1s_com.agc_dl_task.parameters.agc_dl_control == AGC_UPDATE)
          {
            // AGC DL start or update requested
            //---------------------------------

            // Set AGC DL parameters

            l1_audio_agc_dl_copy_params();


            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_AGC_DL_STATE)
            {
              // AGC DL already started: update the DSP AGC DL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl = B_AGC_FULL_UPDATE;
              l1s.agc_dl_action                      = AGC_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {

              // Enable the DSP AGC DL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl = B_AGC_ENABLE;
              l1s.agc_dl_action                      = AGC_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }

          if (l1a_l1s_com.agc_dl_task.parameters.agc_dl_control == AGC_STOP)
          {
            // AGC DL stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_AGC_DL_STATE)
            {
              // Disable the DSP AGC DL module
              l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl = B_AGC_DISABLE;
              l1s.agc_dl_action                      = AGC_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // AGC DL already disabled: confirm
              // Allocate confirmation message...
              l1s.agc_dl_action                      = AGC_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_AGC_DL_CON;
              ((T_L1_AQI_AGC_DL_CON *)(conf_msg->SigP))->agc_dl_action = l1s.agc_dl_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.agc_dl_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_AGC_DL_CON;
            ((T_L1_AQI_AGC_DL_CON *)(conf_msg->SigP))->agc_dl_action = l1s.agc_dl_action;

            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_AGC_DL == 1

#if (L1_IIR == 2)
    /*-------------------------------------------------------*/
    /* l1s_iir_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : IIR L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_iir_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_IIR_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.iir_task.parameters->iir_dl_control == IIR_START ||
			  l1a_l1s_com.iir_task.parameters->iir_dl_control == IIR_UPDATE )
          {
            // IIR start or update requested
            //------------------------------

            l1_audio_iir4x_copy_params();

            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_DL_STATE)
            {
              // IIR already started: update the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_FULL_UPDATE;
              l1s.iir_dl_action                      = IIR_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_ENABLE;
              l1s.iir_dl_action                      = IIR_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }

          if (l1a_l1s_com.iir_task.parameters->iir_dl_control == IIR_STOP)
          {
            // IIR stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_IIR_DL_STATE)
			{
              // Disable the DSP IIR module
              l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl = B_IIR_DISABLE;
              l1s.iir_dl_action                      = IIR_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // IIR already disabled: confirm
              // Allocate confirmation message...
              l1s.iir_dl_action                      = IIR_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_IIR_DL_CON;
              ((T_L1_AQI_IIR_DL_CON *)(conf_msg->SigP))->iir_dl_action = l1s.iir_dl_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.iir_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_IIR_DL_CON;
            ((T_L1_AQI_IIR_DL_CON *)(conf_msg->SigP))->iir_dl_action = l1s.iir_dl_action;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_IIR == 2

  #if (L1_WCM == 1)
    /*-------------------------------------------------------*/
    /* l1s_wcm_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : WCM L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_wcm_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_WCM_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.wcm_task.parameters->wcm_control == WCM_START ||
			  l1a_l1s_com.wcm_task.parameters->wcm_control == WCM_UPDATE )
          {
		    UWORD8 i;

            // WCM start or update requested
            //------------------------------

            l1s_dsp_com.dsp_ndb_ptr->d_wcm_mode             = l1a_l1s_com.wcm_task.parameters->parameters.mode;
            l1s_dsp_com.dsp_ndb_ptr->d_wcm_frame_size       = l1a_l1s_com.wcm_task.parameters->parameters.frame_size;
			l1s_dsp_com.dsp_ndb_ptr->d_wcm_num_sub_frames   = l1a_l1s_com.wcm_task.parameters->parameters.num_sub_frames;
            l1s_dsp_com.dsp_ndb_ptr->d_wcm_ratio            = l1a_l1s_com.wcm_task.parameters->parameters.ratio;
            l1s_dsp_com.dsp_ndb_ptr->d_wcm_threshold        = l1a_l1s_com.wcm_task.parameters->parameters.threshold;

            for (i=0; i < (WCM_1X_GAIN_TABLE_LENGTH); i++)
		    {
              l1s_dsp_com.dsp_ndb_ptr->a_wcm_gain[i]    = l1a_l1s_com.wcm_task.parameters->parameters.gain[i];
		    }

            if (l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_status & B_WCM_STATE)
            {
              // WCM already started: update the DSP WCM module
              l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl = B_WCM_FULL_UPDATE;
              l1s.wcm_action                             = WCM_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP WCM module
              l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl = B_WCM_ENABLE;
              l1s.wcm_action                             = WCM_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }

          if (l1a_l1s_com.wcm_task.parameters->wcm_control == WCM_STOP)
          {
            // WCM stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_status & B_WCM_STATE)
			{
              // Disable the DSP WCM module
              l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl = B_WCM_DISABLE;
              l1s.wcm_action                             = WCM_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // WCM already disabled: confirm
              // Allocate confirmation message...
              l1s.wcm_action                      = WCM_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_WCM_CON;
              ((T_L1_AQI_WCM_CON *)(conf_msg->SigP))->wcm_action = l1s.wcm_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.wcm_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_WCM_CON;
            ((T_L1_AQI_WCM_CON *)(conf_msg->SigP))->wcm_action = l1s.wcm_action;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_WCM == 2

#if (L1_DRC == 1)
    /*-------------------------------------------------------*/
    /* l1s_drc_manager()                                     */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : DRC L1S manager task.                   */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_drc_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_DRC_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.drc_task.parameters->drc_dl_control == DRC_START ||
			  l1a_l1s_com.drc_task.parameters->drc_dl_control == DRC_UPDATE)
          {
            // DRC start or update requested
            //------------------------------

            l1_audio_drc1x_copy_params();

            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_DRC_DL_STATE)
            {
              // DRC already started: update the DSP DRC module
              l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl = B_DRC_FULL_UPDATE;
              l1s.drc_dl_action                      = DRC_UPDATED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP DRC module
              l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl = B_DRC_ENABLE;
              l1s.drc_dl_action                      = DRC_STARTED;

              *state = WAIT_DSP_ACK;
            }
          }

          if (l1a_l1s_com.drc_task.parameters->drc_dl_control == DRC_STOP)
          {
            // DRC stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_DRC_DL_STATE)
			{
              // Disable the DSP DRC module
              l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl = B_DRC_DISABLE;
              l1s.drc_dl_action                      = DRC_STOPPED;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // DRC already disabled: confirm
              // Allocate confirmation message...
              l1s.drc_dl_action                      = DRC_NO_ACTION;
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_AQI_DRC_CON;
              ((T_L1_AQI_DRC_CON *)(conf_msg->SigP))->drc_dl_action = l1s.drc_dl_action;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.drc_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_AQI_DRC_CON;
            ((T_L1_AQI_DRC_CON *)(conf_msg->SigP))->drc_dl_action = l1s.drc_dl_action;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_DRC == 1

  #if (L1_LIMITER == 1)
    /*-------------------------------------------------------*/
    /* l1s_limiter_manager()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : LIMITER L1S manager task.               */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_limiter_manager(void)
    {
      enum states
      {
        IDLE                 = 0,
        WAIT_DSP_ACK         = 1,
        WAIT_PARTIAL_UPDATE  = 2
      };

      UWORD8            *state      = &l1s.audio_state[L1S_LIMITER_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          /* LIMITER update command */
          /*------------------------*/

          if (l1a_l1s_com.limiter_task.command.update)
          {
            if (l1a_l1s_com.limiter_task.parameters.limiter_enable)
            {
              UWORD8 i;

              // LIMITER start requested
              //------------------------

              // Set LIMITER parameters
              l1s_dsp_com.dsp_ndb_ptr->d_lim_block_size          = l1a_l1s_com.limiter_task.parameters.block_size;
              l1s_dsp_com.dsp_ndb_ptr->d_lim_slope_update_period = l1a_l1s_com.limiter_task.parameters.slope_update_period;
              l1s_dsp_com.dsp_ndb_ptr->d_lim_nb_fir_coefs        = l1a_l1s_com.limiter_task.parameters.nb_fir_coefs;

              for (i=0; i < ((l1a_l1s_com.limiter_task.parameters.nb_fir_coefs - 1)>>1) + 1; i++)
                l1s_dsp_com.dsp_ndb_ptr->a_lim_filter_coefs[i] = l1a_l1s_com.limiter_task.parameters.filter_coefs[i];

              l1s_dsp_com.dsp_ndb_ptr->d_lim_gain_fall_q15 = l1a_l1s_com.limiter_task.parameters.gain_fall;
              l1s_dsp_com.dsp_ndb_ptr->d_lim_gain_rise_q15 = l1a_l1s_com.limiter_task.parameters.gain_rise;

              // a_lim_mul_low/high computation and update
              l1_audio_lim_update_mul_low_high();

              if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_LIM_STATE)
              {
                // LIMITER already started: update the DSP LIMITER module
                l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl = B_LIM_FULL_UPDATE;

                *state = WAIT_DSP_ACK;
              }
              else
              {
                // Enable the DSP LIMITER module
                l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl = B_LIM_ENABLE;

                *state = WAIT_DSP_ACK;
              }
            }
            else // LIMITER start requested
            {
              // LIMITER stop requested
              //-----------------------
              if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_LIM_STATE)
              {
                // Disable the DSP LIMITER module
                l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl = B_LIM_DISABLE;

                *state = WAIT_DSP_ACK;
              }
              else
              {
                // LIMITER already disabled: confirm
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(0);
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = L1_LIMITER_CON;
                // Send confirmation message...
                os_send_sig(conf_msg, L1C1_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              }
            }

            // Disable the update command
            l1a_l1s_com.limiter_task.command.update = FALSE;
          } // LIMITER update

          /* LIMITER partial update command */
          /*--------------------------------*/

          else if (l1a_l1s_com.limiter_task.command.partial_update)
          {
            // Only update if the module is enabled
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_LIM_STATE)
            {
              // a_lim_mul_low/high computation and update
              l1_audio_lim_update_mul_low_high();

              // Partial update of the DSP LIMITER module
              l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl = B_LIM_UPDATE;

              *state = WAIT_PARTIAL_UPDATE;
            }

            // Disable the partial update command
            l1a_l1s_com.limiter_task.command.partial_update = FALSE;
          }
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_LIMITER_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;

        case WAIT_PARTIAL_UPDATE:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl == 0)
          {
            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_LIMITER == 1

  #if (L1_ES == 1)

    /*******************************************/
    /* ES configuration tables                 */
    /*******************************************/
#pragma DATA_SECTION(default_es_configs,".flashcnst");
const T_ES_CONFIG default_es_configs[7] =
    {
      // Behavior 1
      {
        (B_ES_UL | B_ES_DL | B_ES_NSF),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_12DB,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_1,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_3DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_15DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 1a
      {
        (B_ES_UL | B_ES_DL),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_12DB,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_1A,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_3DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_6DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 2a
      {
        (B_ES_UL | B_ES_DL | B_ES_NSF),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_12DB,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_2A,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_5DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_5DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_6DB, C_ES_ATT_LIN_24DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 2b
      {
        (B_ES_UL | B_ES_DL | B_ES_CNG),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_6DB,
        C_ES_TCL_0DB,
        C_ES_TCL_10DB,
        C_ES_TCL_4DB,
        C_ES_TCL_0DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_2B,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_8DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_8DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_9DB, C_ES_ATT_LIN_36DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 2c
      {
        (B_ES_UL | B_ES_DL | B_ES_CNG),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_M6DB,
        C_ES_TCL_M23DB,
        C_ES_TCL_0DB,
        C_ES_TCL_M6DB,
        C_ES_TCL_M3DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_2C,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_10DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_10DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_12DB, C_ES_ATT_LIN_48DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 2c_idle
      {
        (B_ES_UL | B_ES_DL | B_ES_CNG),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_M6DB,
        C_ES_TCL_M23DB,
        C_ES_TCL_0DB,
        C_ES_TCL_M6DB,
        C_ES_TCL_M3DB,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_2C_IDLE,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_10DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_10DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_12DB, C_ES_ATT_LIN_12DB, C_ES_ATT_LIN_48DB, C_ES_ATT_LIN_0DB}
      } ,

      // Behavior 3
      {
        (B_ES_UL | B_ES_DL | B_ES_CNG),
        C_ES_GAIN_DL_OFF,
        C_ES_GAIN_UL_1_OFF,
        C_ES_GAIN_UL_2_OFF,
        C_ES_TCL_B3_FE_LS,
        C_ES_TCL_B3_DT_LS,
        C_ES_TCL_B3_FE_NS,
        C_ES_TCL_B3_DT_NS,
        C_ES_TCL_B3_NE,
        C_ES_TCL_LOUD,
        C_ES_SW_CNT,
        C_ES_DT_CNT,
        C_ES_HG_CNT_3,
        /* DL attenuation */
        {C_ES_ATT_LIN_3DB, C_ES_ATT_LIN_16DB, C_ES_ATT_LIN_0DB, C_ES_ATT_LIN_21DB},
        /* UL attenuation */
        {C_ES_ATT_LIN_19DB, C_ES_ATT_LIN_19DB, C_ES_ATT_LIN_66DB, C_ES_ATT_LIN_0DB}
      }
    };

    /*-------------------------------------------------------*/
    /* l1s_es_manager()                                      */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Parameters :                                          */
    /*                                                       */
    /* Return     :                                          */
    /*                                                       */
    /* Description : ES L1S manager task.                    */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1s_es_manager(void)
    {
      enum states
      {
        IDLE                = 0,
        WAIT_DSP_ACK        = 1
      };

      UWORD8            *state      = &l1s.audio_state[L1S_ES_STATE];
      xSignalHeaderRec  *conf_msg;

      switch(*state)
      {
        case IDLE:
        {
          if (l1a_l1s_com.es_task.parameters.es_enable)
          {
            const T_ES_CONFIG *es_cfg;

            // ES start requested
            //--------------------

            // Set ES parameters
            if (l1a_l1s_com.es_task.parameters.es_behavior == ES_CUSTOM_PARAM)
            {
              es_cfg = &(l1a_l1s_com.es_task.parameters.es_config);
            }
            else
            {
              es_cfg = &(default_es_configs[l1a_l1s_com.es_task.parameters.es_behavior]);
            }

            // Set parameters in the API
            l1s_dsp_com.dsp_ndb_ptr->d_es_mode                = es_cfg->es_mode;
            l1s_dsp_com.dsp_ndb_ptr->d_es_gain_dl             = es_cfg->es_gain_dl;
            l1s_dsp_com.dsp_ndb_ptr->d_es_gain_ul_1           = es_cfg->es_gain_ul_1;
            l1s_dsp_com.dsp_ndb_ptr->d_es_gain_ul_2           = es_cfg->es_gain_ul_2;
            l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_fe_ls_thr       = es_cfg->tcl_fe_ls_thr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_dt_ls_thr       = es_cfg->tcl_dt_ls_thr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_fe_ns_thr       = es_cfg->tcl_fe_ns_thr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_dt_ns_thr       = es_cfg->tcl_dt_ns_thr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_tcl_ne_thr          = es_cfg->tcl_ne_thr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_ref_ls_pwr          = es_cfg->ref_ls_pwr;
            l1s_dsp_com.dsp_ndb_ptr->d_es_switching_time      = es_cfg->switching_time;
            l1s_dsp_com.dsp_ndb_ptr->d_es_switching_time_dt   = es_cfg->switching_time_dt;
            l1s_dsp_com.dsp_ndb_ptr->d_es_hang_time           = es_cfg->hang_time;
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_dl_vect[0] = es_cfg->gain_lin_dl_vect[0];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_dl_vect[1] = es_cfg->gain_lin_dl_vect[1];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_dl_vect[2] = es_cfg->gain_lin_dl_vect[2];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_dl_vect[3] = es_cfg->gain_lin_dl_vect[3];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_ul_vect[0] = es_cfg->gain_lin_ul_vect[0];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_ul_vect[1] = es_cfg->gain_lin_ul_vect[1];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_ul_vect[2] = es_cfg->gain_lin_ul_vect[2];
            l1s_dsp_com.dsp_ndb_ptr->a_es_gain_lin_ul_vect[3] = es_cfg->gain_lin_ul_vect[3];

            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ES_STATE)
            {
              // ES already started: update the DSP ES module
              l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl = B_ES_FULL_UPDATE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // Enable the DSP ES module
              l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl = B_ES_ENABLE;

              *state = WAIT_DSP_ACK;
            }
          }
          else // ES start requested
          {
            // ES stop requested
            //-------------------
            if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_ES_STATE)
            {
              // Disable the DSP ES module
              l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl = B_ES_DISABLE;

              *state = WAIT_DSP_ACK;
            }
            else
            {
              // ES already disabled: confirm
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(0);
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = L1_ES_CON;
              // Send confirmation message...
              os_send_sig(conf_msg, L1C1_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            }
          }

          // Disable the update command
          l1a_l1s_com.es_task.command.update = FALSE;
        }
        break;

        case WAIT_DSP_ACK:
        {
          // The DSP acknowledged the L1S command
          if (l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl == 0)
          {
            // Allocate confirmation message...
            conf_msg = os_alloc_sig(0);
            DEBUGMSG(status,NU_ALLOC_ERR)
            conf_msg->SignalCode = L1_ES_CON;
            // Send confirmation message...
            os_send_sig(conf_msg, L1C1_QUEUE);
            DEBUGMSG(status,NU_SEND_QUEUE_ERR)

            *state = IDLE;
          }
        }
        break;
      } // switch
    }
  #endif // L1_ES == 1

#endif // AUDIO_TASK

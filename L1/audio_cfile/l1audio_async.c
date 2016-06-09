/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_ASYNC.C
 *
 *        Filename l1audio_async.c
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
    #include "cust_os.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
    #include "l1audio_varex.h"
    #include "l1audio_proto.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif


    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif
    #if (L1_DYN_DSP_DWNLD==1)
     #include "l1_dyn_dwl_defty.h"
     #include "l1_dyn_dwl_const.h"
     #include "l1_dyn_dwl_signa.h"
    #endif

    #include "l1_defty.h"
    #include "l1_msgty.h"
    #include "l1_varex.h"

    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "l1_ctl.h"


    #include "l1_time.h"
    #include "l1_scen.h"

  #else

    // Layer1 and debug include files.
    #include <ctype.h>
    #include <math.h>
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"
    #include "cust_os.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
    #include "l1audio_varex.h"
    #include "l1audio_proto.h"

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

    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #if (L1_DYN_DSP_DWNLD==1)
     #include "l1_dyn_dwl_defty.h"
     #include "l1_dyn_dwl_const.h"
     #include "l1_dyn_dwl_signa.h"
    #endif

#if (RF_FAM == 61)
#include "l1_rf61.h"
#endif 

    #include "l1_defty.h"
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

    #if (OP_RIV_AUDIO == 1)
      #include "rv_general.h"
      #include "audio_api.h"
      #include "audio_structs_i.h"
      #include "audio_var_i.h"
      #include "audio_macro_i.h"
      #include "audio_const_i.h"
    #endif
  #endif

  #include "l1audio_macro.h"

  #if(L1_DYN_DSP_DWNLD == 1)
    #if (TRACE_TYPE == 1) ||(TRACE_TYPE == 4) || (TRACE_TYPE == 5) || (TRACE_TYPE == 7) || (TESTMODE)
      #include "l1_trace.h"
    #endif
    #if(CODE_VERSION == SIMULATION)
      extern void trace_fct_simu_dyn_dwnld(CHAR *fct_name);
    #endif
  #endif

#if 0
  #if (L1_MP3)
     extern T_MP3_MCU_DSP *mp3_ndb;//Mp3-FR
  #endif
//#pragma DUPLICATE_FOR_INTERNAL_RAM_END

#endif
#if ( L1_AAC)
extern UWORD16  a_aac_dma_input_buffer[2][C_AAC_OUTPUT_BUFFER_SIZE];
extern UWORD8   d_aac_dma_current_buffer_id;

#endif

#if (L1_MP3)
extern T_MP3_MCU_DSP *mp3_ndb;//Mp3-FR
#endif
  /**************************************/
  /* Prototypes for L1 ASYNCH task      */
  /**************************************/
  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      void l1a_audio_driver_process     (xSignalHeaderRec *msg);
    #endif
  #endif
  #if (KEYBEEP)
    void l1a_mmi_keybeep_process        (xSignalHeaderRec *msg);
  #endif
  #if (TONE)
    void l1a_mmi_tone_process           (xSignalHeaderRec *msg);
  #endif
  #if (L1_CPORT == 1)
    void l1a_mmi_cport_process          (xSignalHeaderRec *msg);
  #endif
  #if (MELODY_E1)
    void l1a_mmi_melody0_process        (xSignalHeaderRec *msg);
    void l1a_mmi_melody1_process        (xSignalHeaderRec *msg);
  #endif
  #if (VOICE_MEMO)
    void l1a_mmi_vm_playing_proccess    (xSignalHeaderRec *msg);
    void l1a_mmi_vm_recording_process   (xSignalHeaderRec *msg);
  #endif
  #if (L1_PCM_EXTRACTION)
    void l1a_mmi_pcm_download_process   (xSignalHeaderRec *msg);
    void l1a_mmi_pcm_upload_process     (xSignalHeaderRec *msg);
  #endif
  #if (L1_VOICE_MEMO_AMR)
    void l1a_mmi_vm_amr_playing_proccess (xSignalHeaderRec *msg);
    void l1a_mmi_vm_amr_recording_process(xSignalHeaderRec *msg);
  #endif
  #if (SPEECH_RECO)
    void l1a_mmi_sr_enroll_process      (xSignalHeaderRec *msg);
    void l1a_mmi_sr_update_process      (xSignalHeaderRec *msg);
    void l1a_mmi_sr_reco_process        (xSignalHeaderRec *msg);
    void l1a_mmi_sr_update_check_process(xSignalHeaderRec *msg);
  #endif
  #if (L1_AEC == 1)
    void l1a_mmi_aec_process            (xSignalHeaderRec *msg);
  #endif
  #if (L1_AEC == 2)
    void l1a_mmi_aec_process            (xSignalHeaderRec *msg);
  #endif
  #if (FIR)
    void l1a_mmi_fir_process            (xSignalHeaderRec *msg);
  #endif
  #if (AUDIO_MODE)
    void l1a_mmi_audio_mode_process     (xSignalHeaderRec *msg);
  #endif
  #if (MELODY_E2)
    void l1a_mmi_melody0_e2_process     (xSignalHeaderRec *msg);
    void l1a_mmi_melody1_e2_process     (xSignalHeaderRec *msg);
  #endif
  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
    void l1a_mmi_audio_onoff_process     (xSignalHeaderRec *msg);
  #endif
  #if (L1_EXT_AUDIO_MGT == 1)
    void l1a_mmi_ext_audio_mgt_process   (xSignalHeaderRec *msg);
  #endif
  #if (L1_ANR == 1 || L1_ANR == 2)
    void l1a_mmi_anr_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_AGC_UL == 1)
    void l1a_mmi_agc_ul_process           (xSignalHeaderRec *msg);
  #endif
  #if (L1_AGC_DL == 1)
    void l1a_mmi_agc_dl_process           (xSignalHeaderRec *msg);
  #endif
  #if (L1_IIR == 1 || L1_IIR == 2)
    void l1a_mmi_iir_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_WCM == 1)
    void l1a_mmi_wcm_process           (xSignalHeaderRec *msg);
  #endif
#if (L1_DRC == 1)
    void l1a_mmi_drc_process             (xSignalHeaderRec *msg);
  #endif
  #if (L1_LIMITER == 1)
    void l1a_mmi_limiter_process         (xSignalHeaderRec *msg);
  #endif
  #if (L1_ES == 1)
    void l1a_mmi_es_process              (xSignalHeaderRec *msg);
  #endif
  #if (L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_vocoder_cfg_process(xSignalHeaderRec *msg);
  #endif
#if(L1_BT_AUDIO==1)
   void l1a_mmi_bt_process(xSignalHeaderRec *msg);
#endif
  /**************************************/
  /* External prototypes                */
  /**************************************/
  extern UWORD8  copy_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_dst);
  extern UWORD8  copy_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD16 **ptr_buf, UWORD16 data_size, API *ptr_src);
  extern UWORD8  Cust_get_pointer      (UWORD16 **ptr, UWORD16 *buffer_size, UWORD8 session_id);
  #if (MELODY_E2)
    extern UWORD8  copy_byte_data_from_buffer (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_dst);
    extern UWORD8  copy_byte_data_to_buffer   (UWORD8 session_id, UWORD16 *buffer_size, UWORD8 **ptr_buf, UWORD16 data_size, UWORD8 *ptr_src);
  #endif
  #if (L1_EXT_AUDIO_MGT == 1)
   T_MIDI_DMA_PARAM midi_buf;
  #if (CODE_VERSION == NOT_SIMULATION)
#pragma DATA_SECTION(midi_buf,".l1s_global")
#endif
    extern void l1_ext_audio_mgt_dma_handler(SYS_UWORD16 dma_status);
  #endif
  #if(L1_BT_AUDIO ==1)
   BOOL midi_task_running;
   extern T_L1_BT_AUDIO bt_audio;
   extern void l1_audio_bt_init(UINT16 media_buf_size);
  #endif

  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      void l1a_audio_driver_process(xSignalHeaderRec *msg)
      {
        UWORD32 SignalCode = msg->SignalCode;

        if (SignalCode == L1_AUDIO_DRIVER_IND)
          l1a_audio_send_result(AUDIO_DRIVER_NOTIFICATION_MSG, msg, MMI_QUEUE);
      }
    #endif
  #endif

  #if (KEYBEEP)
    /*-------------------------------------------------------*/
    /* l1a_mmi_keybeep_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* keybeep feature.                                      */
    /*                                                       */
    /* Starting messages:        MMI_KEYBEEP_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_KEYBEEP_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_KEYBEEP_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_KEYBEEP_STOP_REQ        */
    /*                           L1_KEYBEEP_STOP_CON         */
    /*                                                       */
    /* Stop message (output):    MMI_KEYBEEP_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_keybeep_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_KEYBEEP_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.keybeep_task.command.start = FALSE;
            l1a_l1s_com.keybeep_task.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_KEYBEEP_START_REQ)
            {
              // Download the keybeep description in the NDB.
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt0= ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt0;
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt1= ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_k_x1_kt1;
              l1s_dsp_com.dsp_ndb_ptr->d_dur_kb  = ((T_MMI_KEYBEEP_REQ *)(msg->SigP))->d_dur_kb;

              // Start the L1S keybeep task
              l1a_l1s_com.keybeep_task.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
 // OMPAS00090550           break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_KEYBEEP_START_CON)
            {
              // Disable the start command
              l1a_l1s_com.keybeep_task.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_KEYBEEP_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // ompas00090550           break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_KEYBEEP_STOP_REQ)
            {
              // Stop the L1S keybeep task
              l1a_l1s_com.keybeep_task.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_KEYBEEP_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_KEYBEEP_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
     // omaps00090550       break;
        } // switch
      } // while(1)
    }
  #endif // KEYBEEP

  #if (TONE)
    /*-------------------------------------------------------*/
    /* l1a_mmi_tone_process()                                */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* tone feature.                                         */
    /*                                                       */
    /* Starting messages:        MMI_TONE_START_REQ          */
    /*                                                       */
    /* Result messages (input):  L1_TONE_START_CON           */
    /*                                                       */
    /* Result messages (output): MMI_TONE_START_CON          */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_TONE_STOP_REQ           */
    /*                           L1_TONE_STOP_CON            */
    /*                                                       */
    /* Stop message (output):    MMI_TONE_STOP_CON           */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_tone_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_TONE_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.tone_task.command.start = FALSE;
            l1a_l1s_com.tone_task.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_TONE_START_REQ)
            {
              // Download the tone description in the NDB.
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t0 = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t0;
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t1 = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t1;
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t2 = ((T_MMI_TONE_REQ *)(msg->SigP))->d_k_x1_t2;
              l1s_dsp_com.dsp_ndb_ptr->d_pe_rep  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_rep;
              l1s_dsp_com.dsp_ndb_ptr->d_pe_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_pe_off;
              l1s_dsp_com.dsp_ndb_ptr->d_se_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_se_off;
              l1s_dsp_com.dsp_ndb_ptr->d_bu_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_bu_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t0_on   = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t0_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t0_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t1_on   = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t1_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t1_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t2_on   = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t2_off  = ((T_MMI_TONE_REQ *)(msg->SigP))->d_t2_off;


              // Start the L1S tone task
              l1a_l1s_com.tone_task.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
   //omaps00090550          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_TONE_START_CON)
            {
              // Disable the start command
              l1a_l1s_com.tone_task.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_TONE_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550           break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_TONE_STOP_REQ)
            {
              // Stop the L1S tone task
              l1a_l1s_com.tone_task.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_TONE_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_TONE_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // TONE





  #if (MELODY_E1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_melody0_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 0 feature.                                     */
    /*                                                       */
    /* Starting messages:        MMI_MELODY0_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_MELODY0_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_MELODY0_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY0_STOP_REQ        */
    /*                           L1_MELODY0_STOP_CON         */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY0_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_melody0_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M0_RESET            = 0,
        M0_WAIT_START_REQ   = 1,
        M0_WAIT_START_CON   = 2,
        M0_WAIT_STOP        = 3
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY0_STATE];
      UWORD32   SignalCode  = msg->SignalCode;
      UWORD8    melody_osc, used_osc;

      while(1)
      {
        switch(*state)
        {
          case M0_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody0_task.command.start = FALSE;
            l1a_l1s_com.melody0_task.command.stop  = FALSE;

            // Initialize the translation table
            for (melody_osc=0; melody_osc<SC_NUMBER_OSCILLATOR; melody_osc++)
              l1a_l1s_com.melody0_task.parameters.melody_to_oscillator[melody_osc] = SC_NUMBER_OSCILLATOR;

            *state = M0_WAIT_START_REQ;
          }
          break;

          case M0_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY0_START_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.melody0_task.parameters.session_id                = ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody0_task.parameters.loopback                  = ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback;
              l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap    = ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap;

              // Initialize the buffer parameters
              l1a_l1s_com.melody0_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody0_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody0_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody0_task.parameters.ptr_buf,
                                                                              &l1a_l1s_com.melody0_task.parameters.buffer_size,
                                                                              l1a_l1s_com.melody0_task.parameters.session_id);

              // Read the Header of the melody description to have the melody bitmap
              l1a_l1s_com.melody0_task.parameters.error_id = copy_data_from_buffer (l1a_l1s_com.melody0_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_task.parameters.buffer_size,
                                                                                   (UWORD16 **)&l1a_l1s_com.melody0_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &l1a_l1s_com.melody0_task.parameters.oscillator_melody_bitmap);

              l1a_l1s_com.melody0_task.parameters.oscillator_melody_bitmap = Field(l1a_l1s_com.melody0_task.parameters.oscillator_melody_bitmap, SC_MELO_OSCILLATOR_USED_MASK, SC_MELO_OSCILLATOR_USED_SHIFT);

              // Build the array of translation between the melody mapping and the oscillators used
              used_osc = 0;
              melody_osc = 0;
              while ( (melody_osc < SC_NUMBER_OSCILLATOR) && (used_osc < SC_NUMBER_OSCILLATOR) )
              {
                // find the next oscillator available in the melody
                while( ((l1a_l1s_com.melody0_task.parameters.oscillator_melody_bitmap & (0x1<<melody_osc)) == 0) && (melody_osc < SC_NUMBER_OSCILLATOR) )
                  melody_osc++;

                // find the next oscillator available in the oscillator used
                while( ((l1a_l1s_com.melody0_task.parameters.oscillator_used_bitmap & (0x1<<used_osc)) == 0) && (melody_osc < SC_NUMBER_OSCILLATOR) )
                  used_osc++;

                // Fill the translation table
                if ( (melody_osc < SC_NUMBER_OSCILLATOR) && (used_osc < SC_NUMBER_OSCILLATOR) )
                  l1a_l1s_com.melody0_task.parameters.melody_to_oscillator[melody_osc] = used_osc;
                melody_osc++;
                used_osc++;
              }

              // Start the melody 0 L1S task:
              l1a_l1s_com.melody0_task.command.start = TRUE;

              *state = M0_WAIT_START_CON;
            }

            // End process
            return;
          }
 // omaps00090550          break;

          case M0_WAIT_START_CON:
          {
            if (SignalCode == L1_MELODY0_START_CON)
            {
              // Disable the start command
              l1a_l1s_com.melody0_task.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY0_START_CON);

              *state = M0_WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550           break;

          case M0_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY0_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_MELODY0_STOP_CON);

              *state = M0_RESET;
            }
            else
            if (SignalCode == MMI_MELODY0_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody0_task.command.stop = TRUE;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_melody1_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 1 feature.                                     */
    /*                                                       */
    /* Starting messages:        MMI_MELODY1_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_MELODY1_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_MELODY1_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY1_STOP_REQ        */
    /*                           L1_MELODY1_STOP_CON         */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY1_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_melody1_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M1_RESET            = 0,
        M1_WAIT_START_REQ   = 1,
        M1_WAIT_START_CON   = 2,
        M1_WAIT_STOP        = 3
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY1_STATE];
      UWORD32   SignalCode  = msg->SignalCode;
      UWORD8    melody_osc, used_osc;

      while(1)
      {
        switch(*state)
        {
          case M1_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody1_task.command.start = FALSE;
            l1a_l1s_com.melody1_task.command.stop  = FALSE;

            // Initialize the translation table
            for (melody_osc=0; melody_osc<SC_NUMBER_OSCILLATOR; melody_osc++)
              l1a_l1s_com.melody1_task.parameters.melody_to_oscillator[melody_osc] = SC_NUMBER_OSCILLATOR;


            *state = M1_WAIT_START_REQ;
          }
          break;

          case M1_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY1_START_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.melody1_task.parameters.session_id                = ((T_MMI_MELODY_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody1_task.parameters.loopback                  = ((T_MMI_MELODY_REQ *)(msg->SigP))->loopback;
              l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap    = ((T_MMI_MELODY_REQ *)(msg->SigP))->oscillator_used_bitmap;

              // Initialize the buffer parameters
              l1a_l1s_com.melody1_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody1_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody1_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody1_task.parameters.ptr_buf,
                                                                              &l1a_l1s_com.melody1_task.parameters.buffer_size,
                                                                              l1a_l1s_com.melody1_task.parameters.session_id);

              // Read the Header of the melody description to have the melody bitmap
              l1a_l1s_com.melody1_task.parameters.error_id = copy_data_from_buffer (l1a_l1s_com.melody1_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_task.parameters.buffer_size,
                                                                                   (UWORD16 **)&l1a_l1s_com.melody1_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &l1a_l1s_com.melody1_task.parameters.oscillator_melody_bitmap);

              l1a_l1s_com.melody1_task.parameters.oscillator_melody_bitmap = Field(l1a_l1s_com.melody1_task.parameters.oscillator_melody_bitmap, SC_MELO_OSCILLATOR_USED_MASK, SC_MELO_OSCILLATOR_USED_SHIFT);
              // Build the array of translation between the melody maaping and the oscillators used
              used_osc = 0;
              melody_osc = 0;
              while ( (melody_osc < SC_NUMBER_OSCILLATOR) && (used_osc < SC_NUMBER_OSCILLATOR) )
              {
                // find the next oscillator available in the melody
                while( ((l1a_l1s_com.melody1_task.parameters.oscillator_melody_bitmap & (0x1<<melody_osc)) == 0) && (melody_osc < SC_NUMBER_OSCILLATOR) )
                  melody_osc++;

                // find the next oscillator available in the oscillator used
                while( ((l1a_l1s_com.melody1_task.parameters.oscillator_used_bitmap & (0x1<<used_osc)) == 0) && (melody_osc < SC_NUMBER_OSCILLATOR) )
                  used_osc++;

                // Fill the translation table
                if ( (melody_osc < SC_NUMBER_OSCILLATOR) && (used_osc < SC_NUMBER_OSCILLATOR) )
                  l1a_l1s_com.melody1_task.parameters.melody_to_oscillator[melody_osc] = used_osc;
                melody_osc++;
                used_osc++;
              }

              // Start the melody 1 L1S task:
              l1a_l1s_com.melody1_task.command.start = TRUE;

              *state = M1_WAIT_START_CON;
            }

            // End process
            return;
          }
 // omaps00090550           break;

          case M1_WAIT_START_CON:
          {
            if (SignalCode == L1_MELODY1_START_CON)
            {
              // Disable the start command
              l1a_l1s_com.melody1_task.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY1_START_CON);

              *state = M1_WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550           break;
          case M1_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY1_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_MELODY1_STOP_CON);

              *state = M1_RESET;
            }
            else
            if (SignalCode == MMI_MELODY1_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody1_task.command.stop = TRUE;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // MELODY_E1

#if (VOICE_MEMO)
    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_playing_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization playing feature.                   */
    /*                                                       */
    /* Starting messages:        MMI_VM_PLAY_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_VM_PLAY_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_VM_PLAY_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_PLAY_STOP_REQ        */
    /*                           L1_VM_PLAY_STOP_CON         */
    /*                                                       */
    /* Stop message (output):    MMI_VM_PLAY_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_vm_playing_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_VM_PLAY_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_task.play.command.start = FALSE;
            l1a_l1s_com.voicememo_task.play.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_VM_PLAY_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.voicememo_task.play.parameters.session_id =  ((T_MMI_VM_PLAY_REQ *)(msg->SigP))->session_id;

              // Start the L1S voice memo playing task
              l1a_l1s_com.voicememo_task.play.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
 // omaps00090550        break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_PLAY_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_task.play.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_PLAY_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550         break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_VM_PLAY_STOP_REQ)
            {
              // Stop the L1S voice memo playing task
              l1a_l1s_com.voicememo_task.play.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_VM_PLAY_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_VM_PLAY_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_recording_process()                        */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization recording feature.                 */
    /*                                                       */
    /* Starting messages:        MMI_VM_RECORD_START_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_VM_RECORD_START_CON      */
    /*                                                       */
    /* Result messages (output): MMI_VM_RECORD_START_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_RECORD_STOP_REQ      */
    /*                           L1_VM_RECORD_STOP_CON       */
    /*                                                       */
    /* Stop message (output):    MMI_VM_RECORD_STOP_CON      */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_vm_recording_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_VM_RECORD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_task.record.command.start = FALSE;
            l1a_l1s_com.voicememo_task.record.command.stop  = FALSE;
            l1a_l1s_com.voicememo_task.record.tone_ul.start = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_VM_RECORD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.voicememo_task.record.parameters.session_id   =  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.voicememo_task.record.parameters.maximum_size =  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->maximum_size;
              l1a_l1s_com.voicememo_task.record.parameters.dtx          =  ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->dtx_used;

              // Download UL/DL audio gain to the NDB
              l1s_dsp_com.dsp_ndb_ptr->d_shiftul = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_ul;
              l1s_dsp_com.dsp_ndb_ptr->d_shiftdl = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->record_coeff_dl;

              // Download the tone description in the NDB.
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t0 = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t0;
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t1 = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t1;
              l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t2 = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_k_x1_t2;
              l1s_dsp_com.dsp_ndb_ptr->d_pe_rep  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_rep;
              l1s_dsp_com.dsp_ndb_ptr->d_pe_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_pe_off;
              l1s_dsp_com.dsp_ndb_ptr->d_se_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_se_off;
              l1s_dsp_com.dsp_ndb_ptr->d_bu_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_bu_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t0_on   = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t0_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t0_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t1_on   = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t1_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t1_off;
              l1s_dsp_com.dsp_ndb_ptr->d_t2_on   = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_on;
              l1s_dsp_com.dsp_ndb_ptr->d_t2_off  = ((T_MMI_VM_RECORD_REQ *)(msg->SigP))->d_t2_off;

              // Start the L1S voice memo recording task
              l1a_l1s_com.voicememo_task.record.command.start = TRUE;

              // Start the L1S voice memo tone uplink task
              l1a_l1s_com.voicememo_task.record.tone_ul.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
 // omaps00090550      break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_RECORD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_task.record.tone_ul.start = FALSE;

              // Reset the start command
              l1a_l1s_com.voicememo_task.record.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_RECORD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550         break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_VM_RECORD_STOP_REQ)
            {
              // Stop the L1S voice memo recording task
              l1a_l1s_com.voicememo_task.record.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_VM_RECORD_STOP_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_VM_RECORD_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // VOICE_MEMO

  #if (L1_PCM_EXTRACTION)
  #if (L1_DYN_DSP_DWNLD == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_pcm_download_process()                        */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* PCM download feature.                                 */
    /*                                                       */
    /* Starting messages:        MMI_PCM_DOWNLOAD_START_REQ  */
    /*                                                       */
    /* Result messages (input):  L1_PCM_DOWNLOAD_START_CON   */
    /*                                                       */
    /* Result messages (output): MMI_PCM_DOWNLOAD_START_CON  */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_PCM_DOWNLOAD_STOP_REQ   */
    /*                           L1_PCM_DOWNLOAD_STOP_CON    */
    /*                                                       */
    /* Stop message (output):    MMI_PCM_DOWNLOAD_STOP_CON   */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_pcm_download_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_START_CON    = 3,
        WAIT_STOP         = 4
      };

      UWORD8    *state      = &l1a.state[L1A_PCM_DOWNLOAD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      BOOL end_process = 0;

      while(!end_process)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.pcm_task.download.command.start = FALSE;
            l1a_l1s_com.pcm_task.download.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_PCM_DOWNLOAD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.pcm_task.download.parameters.session_id =  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.pcm_task.download.parameters.maximum_size = ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->maximum_size;

              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download = ((((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_ul_gain) << 1);
              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download |=
                                                        ((((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_dl_gain) << 8);

            if (l1a.dyn_dwnld.semaphore_vect[PCM_EXTRACTION_STATE_MACHINE]==GREEN)
            {
              // Start the L1S PCM download task
              l1a_l1s_com.pcm_task.download.command.start = TRUE;
              *state = WAIT_START_CON;
		    }
		    else
		    {
			  *state = WAIT_DYN_DWNLD;
                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[30];
                   sprintf(str,"PCM Extraction SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
		}

            // End process
            end_process = 1;
          }
          break;

          case WAIT_DYN_DWNLD:
          {
            if((SignalCode==API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[PCM_EXTRACTION_STATE_MACHINE] == GREEN))
             {
                 // Start the L1S PCM download task
                l1a_l1s_com.pcm_task.download.command.start = TRUE;
                *state = WAIT_START_CON;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[32];
                   sprintf(str,"PCM Extraction SM un-blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
             }
            // End process
            end_process = 1;
		  }
		  break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_PCM_DOWNLOAD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.pcm_task.download.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_PCM_DOWNLOAD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_STOP:
          {

            UWORD32 maximum_size;

            if (SignalCode == MMI_PCM_DOWNLOAD_STOP_REQ)
            {
              maximum_size =  ((T_MMI_PCM_DOWNLOAD_STOP_REQ *)(msg->SigP))->maximum_size;
              if(maximum_size == 0)
              {
              // Stop the L1S PCM download task
              l1a_l1s_com.pcm_task.download.command.stop = TRUE;
              }
              else
              {
                l1a_l1s_com.pcm_task.download.parameters.maximum_size = maximum_size;

              }

              // End process
              return;
            }
            else
            if (SignalCode == L1_PCM_DOWNLOAD_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_PCM_DOWNLOAD_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(!end_process)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_pcm_upload_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* PCM upload feature.                                   */
    /*                                                       */
    /* Starting messages:        MMI_PCM_UPLOAD_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_PCM_UPLOAD_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_PCM_UPLOAD_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_PCM_UPLOAD_STOP_REQ     */
    /*                           L1_PCM_UPLOAD_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_PCM_UPLOAD_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_pcm_upload_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_START_CON    = 3,
        WAIT_STOP         = 4
      };

      UWORD8    *state      = &l1a.state[L1A_PCM_UPLOAD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      BOOL end_process = 0;

      while(!end_process)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.pcm_task.upload.command.start = FALSE;
            l1a_l1s_com.pcm_task.upload.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_PCM_UPLOAD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.pcm_task.upload.parameters.session_id   =  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.pcm_task.upload.parameters.maximum_size =  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->maximum_size;

              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload = ((((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_ul_gain) << 1);
              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload |= ((((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_dl_gain) << 8);

            if (l1a.dyn_dwnld.semaphore_vect[PCM_EXTRACTION_STATE_MACHINE]==GREEN)
            {
              // Start the L1S PCM upload task
              l1a_l1s_com.pcm_task.upload.command.start = TRUE;
              *state = WAIT_START_CON;
		    }
		    else
		    {
			  *state = WAIT_DYN_DWNLD;
                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[30];
                   sprintf(str,"PCM Extraction SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }


          }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_DYN_DWNLD:
          {
            if((SignalCode==API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[PCM_EXTRACTION_STATE_MACHINE] == GREEN))
             {
                 // Start the L1S PCM upload task
                l1a_l1s_com.pcm_task.upload.command.start = TRUE;
                *state = WAIT_START_CON;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[32];
                   sprintf(str,"PCM Extraction SM un-blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
             }
            // End process
            end_process = 1;
		  }
		  break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_PCM_UPLOAD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.pcm_task.upload.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_PCM_UPLOAD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_PCM_UPLOAD_STOP_REQ)
            {
              // Stop the L1S PCM recording task
              l1a_l1s_com.pcm_task.upload.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_PCM_UPLOAD_STOP_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_PCM_UPLOAD_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(!end_process)
    }

#else /*L1_DYN_DSP_DWNLD == 0

// l1a_mmi_pcm_download_process()
// Description:
// This function is a state machine which handles the    */
//     PCM download feature.
    // Starting messages:        MMI_PCM_DOWNLOAD_START_REQ
//     Result messages (input):  L1_PCM_DOWNLOAD_START_CON
    // Result messages (output): MMI_PCM_DOWNLOAD_START_CON
//    Reset messages (input):   none
    // Stop message (input):     MMI_PCM_DOWNLOAD_STOP_REQ
    //L1_PCM_DOWNLOAD_STOP_CON
//    Stop message (output):    MMI_PCM_DOWNLOAD_STOP_CON   */
    //Rem:                                                  */

    void l1a_mmi_pcm_download_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_PCM_DOWNLOAD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      BOOL end_process = 0;

      while(!end_process)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.pcm_task.download.command.start = FALSE;
            l1a_l1s_com.pcm_task.download.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_PCM_DOWNLOAD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.pcm_task.download.parameters.session_id =  ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.pcm_task.download.parameters.maximum_size = ((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->maximum_size;

              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download = ((((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_ul_gain) << 1);
              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download |=
                                                        ((((T_MMI_PCM_DOWNLOAD_START_REQ *)(msg->SigP))->download_dl_gain) << 8);
              // Start the L1S PCM download task
              l1a_l1s_com.pcm_task.download.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_PCM_DOWNLOAD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.pcm_task.download.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_PCM_DOWNLOAD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_STOP:
          {

            UWORD32 maximum_size;

            if (SignalCode == MMI_PCM_DOWNLOAD_STOP_REQ)
            {
              maximum_size =  ((T_MMI_PCM_DOWNLOAD_STOP_REQ *)(msg->SigP))->maximum_size;
              if(maximum_size == 0)
              {
              // Stop the L1S PCM download task
              l1a_l1s_com.pcm_task.download.command.stop = TRUE;
              }
              else
              {
                l1a_l1s_com.pcm_task.download.parameters.maximum_size = maximum_size;

              }

              // End process
              return;
            }
            else
            if (SignalCode == L1_PCM_DOWNLOAD_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_PCM_DOWNLOAD_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(!end_process)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_pcm_upload_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* PCM upload feature.                                   */
    /*                                                       */
    /* Starting messages:        MMI_PCM_UPLOAD_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_PCM_UPLOAD_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_PCM_UPLOAD_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_PCM_UPLOAD_STOP_REQ     */
    /*                           L1_PCM_UPLOAD_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_PCM_UPLOAD_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_pcm_upload_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_PCM_UPLOAD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      BOOL end_process = 0;

      while(!end_process)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.pcm_task.upload.command.start = FALSE;
            l1a_l1s_com.pcm_task.upload.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_PCM_UPLOAD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.pcm_task.upload.parameters.session_id   =  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.pcm_task.upload.parameters.maximum_size =  ((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->maximum_size;

              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload = ((((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_ul_gain) << 1);
              l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload |= ((((T_MMI_PCM_UPLOAD_START_REQ *)(msg->SigP))->upload_dl_gain) << 8);

              // Start the L1S voice memo recording task
              l1a_l1s_com.pcm_task.upload.command.start = TRUE;


              *state = WAIT_START_CON;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_PCM_UPLOAD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.pcm_task.upload.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_PCM_UPLOAD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            end_process = 1;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_PCM_UPLOAD_STOP_REQ)
            {
              // Stop the L1S PCM recording task
              l1a_l1s_com.pcm_task.upload.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_PCM_UPLOAD_STOP_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_PCM_UPLOAD_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(!end_process)
    }
  #endif /* L1_DYN_DSP_DWNLD */

  #endif /* L1_PCM_EXTRACTION */

  #if (L1_VOICE_MEMO_AMR)

  #if (L1_DYN_DSP_DWNLD==1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_amr_playing_process()                      */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization playing feature.                   */
    /*                                                       */
    /* Starting messages:        MMI_VM_AMR_PLAY_START_REQ   */
    /*                                                       */
    /* Result messages (input):  L1_VM_AMR_PLAY_START_CON    */
    /*                                                       */
    /* Result messages (output): MMI_VM_AMR_PLAY_START_CON   */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_AMR_PLAY_STOP_REQ    */
    /*                           L1_VM_AMR_PLAY_STOP_CON     */
    /*                                                       */
    /* Stop message (output):    MMI_VM_AMR_PLAY_STOP_CON    */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
  void l1a_mmi_vm_amr_playing_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_START_CON    = 3,
        WAIT_STOP         = 4,
        VM_AMR_PLAY       = 5,
        VM_AMR_PAUSE      = 6,
        VM_AMR_PAUSE_CON  = 7,
        WAIT_RESUME_CON   = 8


      };

      UWORD8    *state      = &l1a.state[L1A_VM_AMR_PLAY_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_amr_task.play.command.start = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.stop  = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.pause  = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.resume = FALSE;
            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if(SignalCode == MMI_VM_AMR_PLAY_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
               l1a_l1s_com.voicememo_amr_task.play.parameters.session_id =  ((T_MMI_VM_AMR_PLAY_REQ *)(msg->SigP))->session_id;

              if (l1a.dyn_dwnld.semaphore_vect[VM_STATE_MACHINE]==GREEN)
              {
                 // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                 // Start the L1S voice memo playing task
                 l1a_l1s_com.voicememo_amr_task.play.command.start = TRUE;
                *state = WAIT_START_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[30];
                   sprintf(str,"VOICE PLAY AMR SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }
            // End process
            return;
          }
// omaps00090550          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode==API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[VM_STATE_MACHINE] == GREEN))
             {
                 // Start the L1S voice memo playing task
                 l1a_l1s_com.voicememo_amr_task.play.command.start = TRUE;
                *state = WAIT_START_CON;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[32];
                   sprintf(str,"VOICE PLAY AMR SM un-blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                 }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
             }
             return;
          }
 // omaps00090550          break;
          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_AMR_PLAY_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_amr_task.play.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_START_CON);

              *state = VM_AMR_PLAY ;
            }

            // End process
            return;
          }
 // omaps00090550        break;

          case WAIT_STOP:
          {
            if (SignalCode == L1_VM_AMR_PLAY_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        case VM_AMR_PLAY:
          {
            switch (SignalCode)
            {
              case MMI_VM_AMR_PAUSE_REQ:
              {
              // Stop the L1S voice memo playing task
              l1a_l1s_com.voicememo_amr_task.play.command.pause= TRUE;
              *state = VM_AMR_PAUSE_CON;

              }
	            break;
              case MMI_VM_AMR_PLAY_STOP_REQ:
            {
              // Stop the L1S voice memo playing task
              l1a_l1s_com.voicememo_amr_task.play.command.stop = TRUE;
              *state=WAIT_STOP;

              }
              break;
              case L1_VM_AMR_PLAY_STOP_CON:
            {
             l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            break;
           }
              return;
            }

          case VM_AMR_PAUSE_CON:
          {
            if(SignalCode==L1_VM_AMR_PAUSE_CON)
            {
                // Send confirmation to upper layers
                l1a_audio_send_confirmation(MMI_VM_AMR_PAUSE_CON);

              // Change state
              *state=VM_AMR_PAUSE;
            }
            else if(SignalCode== L1_VM_AMR_PLAY_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
          return;
          }   // case WAIT_PAUSE_CON
      // *************
      // * VM_AMR_PAUSE *
      // *************
      case VM_AMR_PAUSE:
      {
        switch(SignalCode)
        {
          // * MMI requests VM_AMR resume *
          case MMI_VM_AMR_RESUME_REQ:
          {
            l1a_l1s_com.voicememo_amr_task.play.command.pause= FALSE;
           l1a_l1s_com.voicememo_amr_task.play.command.resume=TRUE;

            // Change state
            *state=WAIT_RESUME_CON;

          }
          break;
          // *-----------------------*
          // * MMI requests VM_AMR stop *
          // *-----------------------*
          case MMI_VM_AMR_PLAY_STOP_REQ:
          {
            // Store stop request in L1A/HISR interface
            l1a_l1s_com.voicememo_amr_task.play.command.stop=TRUE;

            // Change state
            *state=WAIT_STOP;

          }
          break;
          case L1_VM_AMR_PLAY_STOP_CON:
            {
             l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            break;
         }
              return;
            }
      // case VM_AMR_PAUSE
      // *******************
      // * WAIT_RESUME_CON *
      // *******************
      case WAIT_RESUME_CON:
      {
        if(SignalCode==L1_VM_AMR_RESUME_CON)
        {
           l1a_l1s_com.voicememo_amr_task.play.command.resume=FALSE;
          // Send confirmation to upper layers
                l1a_audio_send_confirmation(MMI_VM_AMR_RESUME_CON);

            // Change state
            *state=VM_AMR_PLAY;
          }
         return;
      }   // case WAIT_RESUME_CON
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_amr_recording_process()                    */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization recording feature.                 */
    /*                                                       */
    /* Starting messages:        MMI_VM_AMR_RECORD_START_REQ */
    /*                                                       */
    /* Result messages (input):  L1_VM_AMR_RECORD_START_CON  */
    /*                                                       */
    /* Result messages (output): MMI_VM_AMR_RECORD_START_CON */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_AMR_RECORD_STOP_REQ  */
    /*                           L1_VM_AMR_RECORD_STOP_CON   */
    /*                                                       */
    /* Stop message (output):    MMI_VM_AMR_RECORD_STOP_CON  */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_vm_amr_recording_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_START_CON    = 3,
        WAIT_STOP         = 4
      };

      UWORD8    *state      = &l1a.state[L1A_VM_AMR_RECORD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_amr_task.record.command.start = FALSE;
            l1a_l1s_com.voicememo_amr_task.record.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_VM_AMR_RECORD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.voicememo_amr_task.record.parameters.session_id   =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.voicememo_amr_task.record.parameters.maximum_size =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->maximum_size;
              l1a_l1s_com.voicememo_amr_task.record.parameters.dtx          =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->dtx_used;
              l1a_l1s_com.voicememo_amr_task.record.parameters.amr_vocoder  =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->amr_vocoder;

               // Download UL/DL audio gain to the NDB
              l1s_dsp_com.dsp_ndb_ptr->d_shiftul = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->record_coeff_ul;

              if(l1a.dyn_dwnld.semaphore_vect[VM_STATE_MACHINE]==GREEN)
              {
                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                // Start the L1S voice memo recording task
                l1a_l1s_com.voicememo_amr_task.record.command.start = TRUE;

                *state = WAIT_START_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"VOICE MEMO AMR SM blocked \r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            	}
            // End process
            return;
          }
 // omaps00090550          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[VM_STATE_MACHINE] == GREEN))
              {
               // Start the L1S voice memo recording task
               l1a_l1s_com.voicememo_amr_task.record.command.start = TRUE;
               *state = WAIT_START_CON;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                 if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                 {
                   char str[32];
                   sprintf(str,"VOICE MEMO AMR SM un-blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                     trace_fct_simu_dyn_dwnld(str);
                   #else
                     rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                   #endif
                 }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
           return;
          }
 // omaps00090550       break;
          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_AMR_RECORD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_amr_task.record.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_RECORD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_VM_AMR_RECORD_STOP_REQ)
            {
              // Stop the L1S voice memo recording task
              l1a_l1s_com.voicememo_amr_task.record.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_VM_AMR_RECORD_STOP_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_VM_AMR_RECORD_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

#else // L1_DYN_DSP_DWNLD = 0

    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_amr_playing_process()                      */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization playing feature.                   */
    /*                                                       */
    /* Starting messages:        MMI_VM_AMR_PLAY_START_REQ   */
    /*                                                       */
    /* Result messages (input):  L1_VM_AMR_PLAY_START_CON    */
    /*                                                       */
    /* Result messages (output): MMI_VM_AMR_PLAY_START_CON   */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_AMR_PLAY_STOP_REQ    */
    /*                           L1_VM_AMR_PLAY_STOP_CON     */
    /*                                                       */
    /* Stop message (output):    MMI_VM_AMR_PLAY_STOP_CON    */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/

    void l1a_mmi_vm_amr_playing_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3,
        VM_AMR_PLAY       = 4,
        VM_AMR_PAUSE      = 5,
        VM_AMR_PAUSE_CON  = 6,
        WAIT_RESUME_CON   = 7

      };

      UWORD8    *state      = &l1a.state[L1A_VM_AMR_PLAY_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_amr_task.play.command.start = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.stop  = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.pause  = FALSE;
            l1a_l1s_com.voicememo_amr_task.play.command.resume = FALSE;
            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_VM_AMR_PLAY_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.voicememo_amr_task.play.parameters.session_id =  ((T_MMI_VM_AMR_PLAY_REQ *)(msg->SigP))->session_id;

              // Start the L1S voice memo playing task
              l1a_l1s_com.voicememo_amr_task.play.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_AMR_PLAY_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_amr_task.play.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_START_CON);

              *state = VM_AMR_PLAY ;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == L1_VM_AMR_PLAY_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        case VM_AMR_PLAY:
          {
            switch (SignalCode)
            {
              case MMI_VM_AMR_PAUSE_REQ:
              {
              // Stop the L1S voice memo playing task
              l1a_l1s_com.voicememo_amr_task.play.command.pause= TRUE;
              *state = VM_AMR_PAUSE_CON;

              }
	            break;
              case MMI_VM_AMR_PLAY_STOP_REQ:
            {
              // Stop the L1S voice memo playing task
              l1a_l1s_com.voicememo_amr_task.play.command.stop = TRUE;
              *state=WAIT_STOP;

              }
              break;
              case L1_VM_AMR_PLAY_STOP_CON:
            {
             l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            break;
           }
              return;
            }

          case VM_AMR_PAUSE_CON:
          {
            if(SignalCode==L1_VM_AMR_PAUSE_CON)
            {
                // Send confirmation to upper layers
                l1a_audio_send_confirmation(MMI_VM_AMR_PAUSE_CON);

              // Change state
              *state=VM_AMR_PAUSE;
            }
            else if(SignalCode== L1_VM_AMR_PLAY_STOP_CON)
            {
              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
          return;
          }   // case WAIT_PAUSE_CON
      // *************
      // * VM_AMR_PAUSE *
      // *************
      case VM_AMR_PAUSE:
      {
        switch(SignalCode)
        {
          // * MMI requests VM_AMR resume *
          case MMI_VM_AMR_RESUME_REQ:
          {
            l1a_l1s_com.voicememo_amr_task.play.command.pause= FALSE;
           l1a_l1s_com.voicememo_amr_task.play.command.resume=TRUE;

            // Change state
            *state=WAIT_RESUME_CON;

          }
          break;
          // *-----------------------*
          // * MMI requests VM_AMR stop *
          // *-----------------------*
          case MMI_VM_AMR_PLAY_STOP_REQ:
          {
            // Store stop request in L1A/HISR interface
            l1a_l1s_com.voicememo_amr_task.play.command.stop=TRUE;

            // Change state
            *state=WAIT_STOP;

          }
          break;
          case L1_VM_AMR_PLAY_STOP_CON:
            {
             l1a_audio_send_confirmation(MMI_VM_AMR_PLAY_STOP_CON);

              *state = RESET;
            }
            break;
         }
              return;
            }
      // case VM_AMR_PAUSE
      // *******************
      // * WAIT_RESUME_CON *
      // *******************
      case WAIT_RESUME_CON:
      {
        if(SignalCode==L1_VM_AMR_RESUME_CON)
        {
           l1a_l1s_com.voicememo_amr_task.play.command.resume=FALSE;
          // Send confirmation to upper layers
                l1a_audio_send_confirmation(MMI_VM_AMR_RESUME_CON);

            // Change state
            *state=VM_AMR_PLAY;
          }
         return;
      }   // case WAIT_RESUME_CON
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_vm_amr_recording_process()                    */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* voice memorization recording feature.                 */
    /*                                                       */
    /* Starting messages:        MMI_VM_AMR_RECORD_START_REQ */
    /*                                                       */
    /* Result messages (input):  L1_VM_AMR_RECORD_START_CON  */
    /*                                                       */
    /* Result messages (output): MMI_VM_AMR_RECORD_START_CON */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_VM_AMR_RECORD_STOP_REQ  */
    /*                           L1_VM_AMR_RECORD_STOP_CON   */
    /*                                                       */
    /* Stop message (output):    MMI_VM_AMR_RECORD_STOP_CON  */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/

    void l1a_mmi_vm_amr_recording_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_VM_AMR_RECORD_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.voicememo_amr_task.record.command.start = FALSE;
            l1a_l1s_com.voicememo_amr_task.record.command.stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_VM_AMR_RECORD_START_REQ)
            {
              // Download the parameters of the message to the l1a_l1s_com structure.
              l1a_l1s_com.voicememo_amr_task.record.parameters.session_id   =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.voicememo_amr_task.record.parameters.maximum_size =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->maximum_size;
              l1a_l1s_com.voicememo_amr_task.record.parameters.dtx          =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->dtx_used;
              l1a_l1s_com.voicememo_amr_task.record.parameters.amr_vocoder  =  ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->amr_vocoder;

              // Download UL/DL audio gain to the NDB
              l1s_dsp_com.dsp_ndb_ptr->d_shiftul = ((T_MMI_VM_AMR_RECORD_REQ *)(msg->SigP))->record_coeff_ul;

              // Start the L1S voice memo recording task
              l1a_l1s_com.voicememo_amr_task.record.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_VM_AMR_RECORD_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.voicememo_amr_task.record.command.start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_VM_AMR_RECORD_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_VM_AMR_RECORD_STOP_REQ)
            {
              // Stop the L1S voice memo recording task
              l1a_l1s_com.voicememo_amr_task.record.command.stop = TRUE;

              // End process
              return;
            }
            else
            if (SignalCode == L1_VM_AMR_RECORD_STOP_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_VM_AMR_RECORD_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
   #endif // L1_DYN_DSP_DWNLD
  #endif // L1_VOICE_MEMO_AMR

  #if (SPEECH_RECO)
  #if(L1_DYN_DSP_DWNLD == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_enroll_process()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition enrollment feature.                */
    /*                                                       */
    /* Starting messages:        MMI_SR_ENROLL_START_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_SR_ENROLL_START_CON      */
    /*                                                       */
    /* Result messages (output): MMI_SR_ENROLL_START_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_ENROLL_STOP_REQ      */
    /*                           L1_SR_ENROLL_STOP_CON       */
    /*                                                       */
    /* Stop message (output):    MMI_SR_ENROLL_STOP_CON      */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_enroll_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET               = 0,
        WAIT_START_REQ      = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_START_CON      = 3,
        WAIT_STOP           = 4,
        WAIT_BACK_TASK_DONE = 5,
        WAIT_L1S_STOP       = 6,
        WAIT_BACK_STOP      = 7
      };

      UWORD8            *state      = &l1a.state[L1A_SR_ENROLL_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.enroll_start = FALSE;
            l1a_l1s_com.speechreco_task.command.enroll_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_ENROLL_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id    = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index     = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address  = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.speech         = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech_address;

              if(l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE]==GREEN)
              {

                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                // Set the start command of the speech recording task
                l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

                // Start the speech recognition enrollment task
                l1a_l1s_com.speechreco_task.command.enroll_start = TRUE;

              *state = WAIT_START_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"SPEECH RECO SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                     trace_fct_simu_dyn_dwnld(str);
                   #else
                     rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                   #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN))
            {
              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition enrollment task
              l1a_l1s_com.speechreco_task.command.enroll_start = TRUE;
              *state = WAIT_START_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                {
                  char str[30];
                  sprintf(str,"SPEECH RECO SM un-blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                     trace_fct_simu_dyn_dwnld(str);
                   #else
                     rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                   #endif
                }
              #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
          break;
          case WAIT_START_CON:
          {
            if (SignalCode == L1_SR_ENROLL_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.enroll_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_ENROLL_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_SR_ENROLL_STOP_REQ)
            {
              // Stop the speech recognition enroll task
              l1a_l1s_com.speechreco_task.command.enroll_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_ENROLL_STOP_CON)
            {
              // There is an error during the acquisition task?
              if ( ((T_L1_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                *state = WAIT_BACK_TASK_DONE;

                // End process
                return;
              }
              else
              // There is an error
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_ENROLL_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_TASK_DONE:
          {
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the stop confirmation message with no error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_NO_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig( sizeof(T_MMI_SR_ENROLL_STOP_CON) );
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_NO_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;

            }
            else
            if (SignalCode == MMI_SR_ENROLL_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the message MMI_SR_ENROLL_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_BAD_ACQUISITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_ENROLL_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_ACQUISITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if (SignalCode == L1_SR_ENROLL_STOP_CON)
            {
              // Send the message MMI_SR_ENROLL_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_BAD_ACQUISITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_ENROLL_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_ACQUISITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_update_process()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition update feature.                    */
    /*                                                       */
    /* Starting messages:        MMI_SR_UPDATE_START_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_SR_UPDATE_START_CON      */
    /*                                                       */
    /* Result messages (output): MMI_SR_UPDATE_START_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_UPDATE_STOP_REQ      */
    /*                           L1_SR_UPDATE_STOP_CON       */
    /*                                                       */
    /* Stop message (output):    MMI_SR_UPDATE_STOP_CON      */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_update_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET               = 0,
        WAIT_START_REQ      = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_MODEL_LOADED   = 3,
        WAIT_START_CON      = 4,
        WAIT_STOP           = 5,
        WAIT_BACK_TASK_DONE = 6,
        WAIT_L1S_STOP       = 7,
        WAIT_BACK_STOP      = 8
      };

      UWORD8            *state      = &l1a.state[L1A_SR_UPDATE_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.update_start = FALSE;
            l1a_l1s_com.speechreco_task.command.update_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_UPDATE_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id    = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index     = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address  = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.speech         = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech_address;

              // Reset the background task emergency stop
              l1_srback_com.emergency_stop = FALSE;

              l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;

              if(l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN)
              {

                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                // Start to download the model to the API
                l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

                // Send the start confirmation message
                l1a_audio_send_confirmation(MMI_SR_UPDATE_START_CON);

                *state = WAIT_MODEL_LOADED;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"SPEECH RECO SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                     trace_fct_simu_dyn_dwnld(str);
                   #else
                     rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                   #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN))
            {
               // Start to download the model to the API
              l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_UPDATE_START_CON);

              *state = WAIT_MODEL_LOADED;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                {
                  char str[30];
                  sprintf(str,"SPEECH RECO SM un-blocked\r\n");
                  #if(CODE_VERSION == SIMULATION)
                    trace_fct_simu_dyn_dwnld(str);
                  #else
                    rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                  #endif
                }
              #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

            }
            return;
          }
          break;
          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;
            }
            else
            if (SignalCode == L1_SR_UPDATE_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.update_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // There is an error during the update task?
              if ( ((T_L1_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                *state = WAIT_BACK_TASK_DONE;

                // End process
                return;
              }
              else
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_UPDATE_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_TASK_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_NO_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_NO_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if ( (SignalCode == L1_SRBACK_SAVE_DATA_CON) ||
                 (SignalCode == L1_SRBACK_LOAD_MODEL_CON) )
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_BAD_UPDATE;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_UPDATE;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_BAD_UPDATE;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_UPDATE;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_reco_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition reco feature.                      */
    /*                                                       */
    /* Starting messages:        MMI_SR_RECO_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_SR_RECO_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_SR_RECO_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_RECO_STOP_REQ        */
    /*                           L1_SR_RECO_STOP_IND         */
    /*                                                       */
    /* Stop message (output):    MMI_SR_RECO_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_reco_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET                 = 0,
        WAIT_START_REQ        = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_RECO_START       = 3,
        WAIT_RECO_STOP        = 4,
        LOAD_MODEL            = 5,
        WAIT_MODEL_LOADED     = 6,
        WAIT_PROCESSING_STOP  = 7,
        WAIT_L1S_STOP         = 8,
        WAIT_BACK_STOP        = 9
      };

      UWORD8            *state      = &l1a.state[L1A_SR_RECO_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.reco_start        = FALSE;
            l1a_l1s_com.speechreco_task.command.reco_stop         = FALSE;
            l1a_l1s_com.speechreco_task.command.processing_start  = FALSE;
            l1a_l1s_com.speechreco_task.command.processing_stop   = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_RECO_START_REQ)
            {
              // Reset the index counter
              l1a_l1s_com.speechreco_task.parameters.index_counter = 0;

              // Download the parameters to the l1a_l1s_com.speechreco_task.parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id     = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.vocabulary_size = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->vocabulary_size;
              l1a_l1s_com.speechreco_task.parameters.model_address   = l1s_dsp_com.dsp_ndb_ptr->a_model;

              // The CTO algorithm must be used?
              if (l1a_l1s_com.speechreco_task.parameters.vocabulary_size <= SC_SR_MAX_WORDS_FOR_CTO)
              {
                // Enable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = TRUE;

                // Double the vocabulary size
                l1a_l1s_com.speechreco_task.parameters.vocabulary_size <<= 1;
              }
              else
              {
                // Disable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              }

              if (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN)
              {

                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                // Start the speech recognition reco task
                l1a_l1s_com.speechreco_task.command.reco_start = TRUE;

                *state = WAIT_RECO_START;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"SPEECH RECO SM blocked\r\n");
                   #if(CODE_VERSION == SIMULATION)
                     trace_fct_simu_dyn_dwnld(str);
                   #else
                     rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                   #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }
            // End process
            return;
          }
          break;

          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN))
            {
              // Start the speech recognition reco task
              l1a_l1s_com.speechreco_task.command.reco_start = TRUE;
              *state = WAIT_RECO_START;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                {
                  char str[30];
                  sprintf(str,"SPEECH RECO SM un-blocked\r\n");
                  #if(CODE_VERSION == SIMULATION)
                    trace_fct_simu_dyn_dwnld(str);
                  #else
                    rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                  #endif
                }
              #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

            }
            return;
          }
          break;
          case WAIT_RECO_START:
          {
            if (SignalCode == L1_SR_RECO_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.speechreco_task.command.reco_start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_RECO_START_CON);

              *state = WAIT_RECO_STOP;
            }
            // End process
            return;
          }
          break;

          case WAIT_RECO_STOP:
          {
            if (SignalCode == L1_SR_RECO_STOP_CON)
            {
              // The acqusition is good or not?
              if ( ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the message MMI_SR_RECO_STOP_CON with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_RECO_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id;

                // send the messsage to the audio entity
                rvf_send_msg (p_audio_gbl_var->addrId,
                              p_message);
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
                // File the message
                ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if(SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop the speech recognition task
              l1a_l1s_com.speechreco_task.command.reco_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case LOAD_MODEL:
          {
            // Initialize the background task stop command
            l1_srback_com.emergency_stop = FALSE;

            // Start to load the model to the API
            l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.index_counter;
            l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

            // Increase the index counter
            l1a_l1s_com.speechreco_task.parameters.index_counter++;

            *state = WAIT_MODEL_LOADED;

            // End process
            return;
          }
          break;

          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Start the DSP processing task
              l1a_l1s_com.speechreco_task.command.processing_start = TRUE;

              *state = WAIT_PROCESSING_STOP;
            }
            else
            if (SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_PROCESSING_STOP:
          {
            if (SignalCode == L1_SR_PROCESSING_STOP_CON)
            {
              // The processing phase is good or not?
              if ( ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the MMI_SR_RECO_STOP_CON message with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_RECO_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
                // File the message
                ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if (SignalCode == L1_SR_RECO_STOP_IND)
            {
              // The CTO algorithm is used?
              if (l1a_l1s_com.speechreco_task.parameters.CTO_algorithm)
              {
                // There is an error during the recognition?
                if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id == SC_NO_ERROR )
                {
                  // The best word is odd?
                  if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index & 0x01 )
                  {
                    // Change the error to tSC_CTO_WORD
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CTO_WORD;
                  }
                  else
                  {
                    // Devided by 2 the 4 indexes of the best words in the message
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index         >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index  >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index   >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index  >>= 1;
                  }
                }
              }
                // Forward the message in the MMI_SR_RECO_STOP_CON
                l1a_audio_send_result(MMI_SR_RECO_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
            }
            else
            if (SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop the L1S processing task
              l1a_l1s_com.speechreco_task.command.processing_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // end process
              return;
            }
            else
            {
              // end process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if ( (SignalCode == L1_SR_PROCESSING_STOP_CON) ||
                 (SignalCode == L1_SR_RECO_STOP_CON)       ||
                 (SignalCode == L1_SR_RECO_STOP_IND) )
            {
              // Send the message MMI_SR_RECO_STOP_CON with a bad recognition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_RECO_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id = SC_BAD_RECOGNITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
              // File the message
              ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_RECOGNITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Send the MMI_SR_RECO_STOP_CON with an bad recognition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_RECO_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id = SC_BAD_RECOGNITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
              // Fill the message
              ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_RECOGNITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*---------------------------------------------------------*/
    /* l1a_mmi_sr_update_check_process()                       */
    /*---------------------------------------------------------*/
    /*                                                         */
    /* Description:                                            */
    /* ------------                                            */
    /* This function is a state machine which handles the      */
    /* speech recognition update check feature.                */
    /*                                                         */
    /* Starting messages:        MMI_SR_UPDATE_CHECK_START_REQ */
    /*                                                         */
    /* Result messages (input):  L1_SR_UPDATE_START_CON        */
    /*                                                         */
    /* Result messages (output): MMI_SR_UPDATE_CHECK_START_CON */
    /*                                                         */
    /* Reset messages (input):   none                          */
    /*                                                         */
    /* Stop message (input):     MMI_SR_UPDATE_CHECK_STOP_REQ  */
    /*                           L1_SR_RECO_STOP_IND           */
    /*                                                         */
    /* Stop message (output):    MMI_SR_UPDATE_CHECK_STOP_CON  */
    /*                                                         */
    /* Rem:                                                    */
    /* ----                                                    */
    /*                                                         */
    /*---------------------------------------------------------*/
    void l1a_mmi_sr_update_check_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET                 = 0,
        WAIT_START_REQ        = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_MODEL            = 3,
        WAIT_UPDATE_START     = 4,
        WAIT_UPDATE_STOP      = 5,
        WAIT_TEMP_SAVE_DONE   = 6,
        LOAD_MODEL            = 7,
        WAIT_MODEL_LOADED     = 8,
        WAIT_PROCESSING_STOP  = 9,
        WAIT_SAVE_DONE        = 10,
        WAIT_L1S_STOP         = 11,
        WAIT_BACK_STOP        = 12
      };

      UWORD8            *state      = &l1a.state[L1A_SR_UPDATE_CHECK_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
        #define AUDIO_MSG (p_message)
      #else
        xSignalHeaderRec  *conf_msg;
        #define AUDIO_MSG (conf_msg->SigP)
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.update_start = FALSE;
            l1a_l1s_com.speechreco_task.command.update_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if(SignalCode == MMI_SR_UPDATE_CHECK_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id        = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index         = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.word_to_check      = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address      = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.model_temp_address = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->model_address;
              l1a_l1s_com.speechreco_task.parameters.speech             = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address     = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech_address;
              l1a_l1s_com.speechreco_task.parameters.vocabulary_size    = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->vocabulary_size;

              // Reset the background task emergency stop
              l1_srback_com.emergency_stop = FALSE;

              l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;

              if (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN)
              {

                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
                // Start to download the model to the API
                l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

                // Send the start confirmation message
                l1a_audio_send_confirmation(MMI_SR_UPDATE_CHECK_START_CON);

                *state = WAIT_MODEL;
              }
              else
              {
               *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"SPEECH RECO SM blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[SR_STATE_MACHINE] == GREEN))
            {

              // Start to download the model to the API
              l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_UPDATE_CHECK_START_CON);

              *state = WAIT_MODEL;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                {
                  char str[30];
                  sprintf(str,"SPEECH RECO SM un-blocked\r\n");
                  #if(CODE_VERSION == SIMULATION)
                    trace_fct_simu_dyn_dwnld(str);
                  #else
                    rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                  #endif
                }
              #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)

            }
            return;
          }
          break;
          case WAIT_MODEL:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_start = TRUE;

              *state = WAIT_UPDATE_START;
            }

            // End process
            return;
          }
          break;

          case WAIT_UPDATE_START:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;
            }
            else
            if (SignalCode == L1_SR_UPDATE_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.update_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              *state = WAIT_UPDATE_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_UPDATE_STOP:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // There is an error during the update task?
              if ( ((T_L1_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_TEMP_SAVE_DATA_REQ);

                *state = WAIT_TEMP_SAVE_DONE;

                // End process
                return;
              }
              else
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_TEMP_SAVE_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SRBACK_TEMP_SAVE_DATA_CON)
            {
              // Reset the command
              l1a_l1s_com.speechreco_task.command.processing_start  = FALSE;
              l1a_l1s_com.speechreco_task.command.processing_stop   = FALSE;

              // Reset the index counter
              l1a_l1s_com.speechreco_task.parameters.index_counter = 0;

              // The CTO algorithm must be used?
              if (l1a_l1s_com.speechreco_task.parameters.vocabulary_size <= SC_SR_MAX_WORDS_FOR_CTO)
              {
                // Enable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = TRUE;

                // Double the vocabulary size
                l1a_l1s_com.speechreco_task.parameters.vocabulary_size <<= 1;
              }
              else
              {
                // Disable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              }

              *state = LOAD_MODEL;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case LOAD_MODEL:
          {
            // Initialize the background task stop command
            l1_srback_com.emergency_stop = FALSE;

            // Start to load the model to the API
            l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.index_counter;
            l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

            // Increase the index counter
            l1a_l1s_com.speechreco_task.parameters.index_counter++;

            *state = WAIT_MODEL_LOADED;

            // End process
            return;
          }
          break;

          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Start the DSP processing task
              l1a_l1s_com.speechreco_task.command.processing_start = TRUE;

              *state = WAIT_PROCESSING_STOP;
            }
            else
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_PROCESSING_STOP:
          {
            if (SignalCode == L1_SR_PROCESSING_STOP_CON)
            {
              // The processing phase is good or not?
              if ( ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the MMI_SR_UPDATE_CHECK_STOP_CON message with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                // send the messsage to the audio entity
                rvf_send_msg (p_audio_gbl_var->addrId,
                              p_message);
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;
                // File the message
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if (SignalCode == L1_SR_RECO_STOP_IND)
            {
              // There is an error during the recognition?
              if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // The CTO algorithm is used?
                if (l1a_l1s_com.speechreco_task.parameters.CTO_algorithm)
                {
                  // The best word is odd?
                  if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index & 0x01 )
                  {
                    // Change the error to SC_CTO_WORD
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CTO_WORD;

                    // Forward the message in the MMI_SR_RECO_STOP_CON
                    l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

                    *state = RESET;

                    // End process
                    return;
                  }
                  else
                  {
                    // Devided by 2 the 4 indexes of the best words in the message
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index         >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index  >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index   >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index  >>= 1;
                  }
                }
                // Is it the good word?
                if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index ==
                     l1a_l1s_com.speechreco_task.parameters.word_to_check )
                {
                  // Save the message informations in the l1a_l1s_com memory
                  l1a_l1s_com.speechreco_task.parameters.best_word_index        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.best_word_score        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.second_best_word_index = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.second_best_word_score = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.third_best_word_index  = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.third_best_word_score  = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.fourth_best_word_index = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.fourth_best_word_score = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_db_level          = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_level;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_db_noise          = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_noise;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_model_size        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_model_size;

                  // Reset the stop background task
                  l1_srback_com.emergency_stop = FALSE;

                  // Start the background task to save the model in the database
                  l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.word_to_check;
                  l1a_l1s_com.speechreco_task.parameters.model_address = l1a_l1s_com.speechreco_task.parameters.model_temp_address;
                  l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                  *state = WAIT_SAVE_DONE;

                  // End process
                  return;
                }
                else
                {
                  // Change the error to SC_CHECK_ERROR
                  ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CHECK_ERROR;
                }
              }
              // Forward the message in the MMI_SR_RECO_STOP_CON
              l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the L1S processing task
              l1a_l1s_com.speechreco_task.command.processing_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // end process
              return;
            }
            else
            {
              // end process
              return;
            }
          }
          break;

          case WAIT_SAVE_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the MMI_SR_UPDATE_CHECK_STOP_CON message with an error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;
            #endif // OP_RIV_AUDIO

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->error_id                = SC_NO_ERROR;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->best_word_index         = l1a_l1s_com.speechreco_task.parameters.best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->best_word_score         = l1a_l1s_com.speechreco_task.parameters.best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->second_best_word_index  = l1a_l1s_com.speechreco_task.parameters.second_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->second_best_word_score  = l1a_l1s_com.speechreco_task.parameters.second_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->third_best_word_index   = l1a_l1s_com.speechreco_task.parameters.third_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->third_best_word_score   = l1a_l1s_com.speechreco_task.parameters.third_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->fourth_best_word_index  = l1a_l1s_com.speechreco_task.parameters.fourth_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->fourth_best_word_score  = l1a_l1s_com.speechreco_task.parameters.fourth_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_db_level           = l1a_l1s_com.speechreco_task.parameters.d_sr_db_level;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_db_noise           = l1a_l1s_com.speechreco_task.parameters.d_sr_db_noise;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_model_size         = l1a_l1s_com.speechreco_task.parameters.d_sr_model_size;

            #if (OP_RIV_AUDIO == 1)
              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if ( (SignalCode == L1_SRBACK_SAVE_DATA_CON)  ||
                 (SignalCode == L1_SRBACK_LOAD_MODEL_CON) ||
                 (SignalCode == L1_SRBACK_TEMP_SAVE_DATA_CON) )
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id                = SC_CHECK_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = SC_CHECK_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if ( (SignalCode == L1_SR_UPDATE_STOP_CON)     ||
                 (SignalCode == L1_SR_PROCESSING_STOP_CON) ||
                 (SignalCode == L1_SR_RECO_STOP_CON)       ||
                 (SignalCode == L1_SR_RECO_STOP_IND) )
            {
              // Send the message MMI_SR_UPDATE_CHECK_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id                = SC_CHECK_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = SC_CHECK_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)

      // Undefine message pointer macro.
      #undef MSG_AUDIO

    }
    #else // L1_DYN_DSP_DWNLD = 0
    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_enroll_process()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition enrollment feature.                */
    /*                                                       */
    /* Starting messages:        MMI_SR_ENROLL_START_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_SR_ENROLL_START_CON      */
    /*                                                       */
    /* Result messages (output): MMI_SR_ENROLL_START_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_ENROLL_STOP_REQ      */
    /*                           L1_SR_ENROLL_STOP_CON       */
    /*                                                       */
    /* Stop message (output):    MMI_SR_ENROLL_STOP_CON      */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_enroll_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET               = 0,
        WAIT_START_REQ      = 1,
        WAIT_START_CON      = 2,
        WAIT_STOP           = 3,
        WAIT_BACK_TASK_DONE = 4,
        WAIT_L1S_STOP       = 5,
        WAIT_BACK_STOP      = 6
      };

      UWORD8            *state      = &l1a.state[L1A_SR_ENROLL_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.enroll_start = FALSE;
            l1a_l1s_com.speechreco_task.command.enroll_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_ENROLL_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id    = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index     = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address  = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.speech         = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address = ((T_MMI_SR_ENROLL_REQ *)(msg->SigP))->speech_address;

              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition enrollment task
              l1a_l1s_com.speechreco_task.command.enroll_start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_SR_ENROLL_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.enroll_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_ENROLL_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_SR_ENROLL_STOP_REQ)
            {
              // Stop the speech recognition enroll task
              l1a_l1s_com.speechreco_task.command.enroll_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_ENROLL_STOP_CON)
            {
              // There is an error during the acquisition task?
              if ( ((T_L1_SR_ENROLL_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                *state = WAIT_BACK_TASK_DONE;

                // End process
                return;
              }
              else
              // There is an error
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_ENROLL_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_TASK_DONE:
          {
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the stop confirmation message with no error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_NO_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig( sizeof(T_MMI_SR_ENROLL_STOP_CON) );
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_NO_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;

            }
            else
            if (SignalCode == MMI_SR_ENROLL_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the message MMI_SR_ENROLL_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_BAD_ACQUISITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_ENROLL_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_ACQUISITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if (SignalCode == L1_SR_ENROLL_STOP_CON)
            {
              // Send the message MMI_SR_ENROLL_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_ENROLL_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->header.msg_id = MMI_SR_ENROLL_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_ENROLL_STOP_CON *)(p_message))->error_id = SC_BAD_ACQUISITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_ENROLL_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_ENROLL_STOP_CON;

              //Fill the message
              ((T_MMI_SR_ENROLL_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_ACQUISITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_update_process()                           */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition update feature.                    */
    /*                                                       */
    /* Starting messages:        MMI_SR_UPDATE_START_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_SR_UPDATE_START_CON      */
    /*                                                       */
    /* Result messages (output): MMI_SR_UPDATE_START_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_UPDATE_STOP_REQ      */
    /*                           L1_SR_UPDATE_STOP_CON       */
    /*                                                       */
    /* Stop message (output):    MMI_SR_UPDATE_STOP_CON      */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_update_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET               = 0,
        WAIT_START_REQ      = 1,
        WAIT_MODEL_LOADED   = 2,
        WAIT_START_CON      = 3,
        WAIT_STOP           = 4,
        WAIT_BACK_TASK_DONE = 5,
        WAIT_L1S_STOP       = 6,
        WAIT_BACK_STOP      = 7
      };

      UWORD8            *state      = &l1a.state[L1A_SR_UPDATE_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.update_start = FALSE;
            l1a_l1s_com.speechreco_task.command.update_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_UPDATE_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id    = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index     = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address  = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.speech         = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address = ((T_MMI_SR_UPDATE_REQ *)(msg->SigP))->speech_address;

              // Reset the background task emergency stop
              l1_srback_com.emergency_stop = FALSE;

              // Start to download the model to the API
              l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_UPDATE_START_CON);

              *state = WAIT_MODEL_LOADED;
            }

            // End process
            return;
          }
          break;

          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;
            }
            else
            if (SignalCode == L1_SR_UPDATE_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.update_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // There is an error during the update task?
              if ( ((T_L1_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                *state = WAIT_BACK_TASK_DONE;

                // End process
                return;
              }
              else
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_UPDATE_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_TASK_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an acquisition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_NO_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_NO_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if ( (SignalCode == L1_SRBACK_SAVE_DATA_CON) ||
                 (SignalCode == L1_SRBACK_LOAD_MODEL_CON) )
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_BAD_UPDATE;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_UPDATE;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_STOP_CON *)(p_message))->error_id = SC_BAD_UPDATE;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_UPDATE;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_sr_reco_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* speech recognition reco feature.                      */
    /*                                                       */
    /* Starting messages:        MMI_SR_RECO_START_REQ       */
    /*                                                       */
    /* Result messages (input):  L1_SR_RECO_START_CON        */
    /*                                                       */
    /* Result messages (output): MMI_SR_RECO_START_CON       */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_SR_RECO_STOP_REQ        */
    /*                           L1_SR_RECO_STOP_IND         */
    /*                                                       */
    /* Stop message (output):    MMI_SR_RECO_STOP_CON        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_sr_reco_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET                 = 0,
        WAIT_START_REQ        = 1,
        WAIT_RECO_START       = 2,
        WAIT_RECO_STOP        = 3,
        LOAD_MODEL            = 4,
        WAIT_MODEL_LOADED     = 5,
        WAIT_PROCESSING_STOP  = 6,
        WAIT_L1S_STOP         = 7,
        WAIT_BACK_STOP        = 8
      };

      UWORD8            *state      = &l1a.state[L1A_SR_RECO_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
      #else
        xSignalHeaderRec  *conf_msg;
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.reco_start        = FALSE;
            l1a_l1s_com.speechreco_task.command.reco_stop         = FALSE;
            l1a_l1s_com.speechreco_task.command.processing_start  = FALSE;
            l1a_l1s_com.speechreco_task.command.processing_stop   = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_RECO_START_REQ)
            {

              // Reset the index counter
              l1a_l1s_com.speechreco_task.parameters.index_counter = 0;

              // Download the parameters to the l1a_l1s_com.speechreco_task.parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id     = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.vocabulary_size = ((T_MMI_SR_RECO_REQ *)(msg->SigP))->vocabulary_size;
              l1a_l1s_com.speechreco_task.parameters.model_address   = l1s_dsp_com.dsp_ndb_ptr->a_model;

              // The CTO algorithm must be used?
              if (l1a_l1s_com.speechreco_task.parameters.vocabulary_size <= SC_SR_MAX_WORDS_FOR_CTO)
              {
                // Enable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = TRUE;

                // Double the vocabulary size
                l1a_l1s_com.speechreco_task.parameters.vocabulary_size <<= 1;
              }
              else
              {
                // Disable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              }

              // Start the speech recognition reco task
              l1a_l1s_com.speechreco_task.command.reco_start = TRUE;

              *state = WAIT_RECO_START;
            }

            // End process
            return;
          }
          break;


          case WAIT_RECO_START:
          {
            if (SignalCode == L1_SR_RECO_START_CON)
            {
              // Reset the start command
              l1a_l1s_com.speechreco_task.command.reco_start = FALSE;

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_RECO_START_CON);

              *state = WAIT_RECO_STOP;
            }
            // End process
            return;
          }
          break;

          case WAIT_RECO_STOP:
          {
            if (SignalCode == L1_SR_RECO_STOP_CON)
            {
              // The acqusition is good or not?
              if ( ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the message MMI_SR_RECO_STOP_CON with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_RECO_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id;

                // send the messsage to the audio entity
                rvf_send_msg (p_audio_gbl_var->addrId,
                              p_message);
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
                // File the message
                ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_RECO_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if(SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop the speech recognition task
              l1a_l1s_com.speechreco_task.command.reco_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case LOAD_MODEL:
          {
            // Initialize the background task stop command
            l1_srback_com.emergency_stop = FALSE;

            // Start to load the model to the API
            l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.index_counter;
            l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

            // Increase the index counter
            l1a_l1s_com.speechreco_task.parameters.index_counter++;

            *state = WAIT_MODEL_LOADED;

            // End process
            return;
          }
          break;

          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Start the DSP processing task
              l1a_l1s_com.speechreco_task.command.processing_start = TRUE;

              *state = WAIT_PROCESSING_STOP;
            }
            else
            if (SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_PROCESSING_STOP:
          {
            if (SignalCode == L1_SR_PROCESSING_STOP_CON)
            {
              // The processing phase is good or not?
              if ( ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the MMI_SR_RECO_STOP_CON message with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_RECO_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
                // File the message
                ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if (SignalCode == L1_SR_RECO_STOP_IND)
            {
              // The CTO algorithm is used?
              if (l1a_l1s_com.speechreco_task.parameters.CTO_algorithm)
              {
                // There is an error during the recognition?
                if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id == SC_NO_ERROR )
                {
                  // The best word is odd?
                  if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index & 0x01 )
                  {
                    // Change the error to tSC_CTO_WORD
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CTO_WORD;
                  }
                  else
                  {
                    // Devided by 2 the 4 indexes of the best words in the message
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index         >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index  >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index   >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index  >>= 1;
                  }
                }
              }
                // Forward the message in the MMI_SR_RECO_STOP_CON
                l1a_audio_send_result(MMI_SR_RECO_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
            }
            else
            if (SignalCode == MMI_SR_RECO_STOP_REQ)
            {
              // Stop the L1S processing task
              l1a_l1s_com.speechreco_task.command.processing_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // end process
              return;
            }
            else
            {
              // end process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if ( (SignalCode == L1_SR_PROCESSING_STOP_CON) ||
                 (SignalCode == L1_SR_RECO_STOP_CON)       ||
                 (SignalCode == L1_SR_RECO_STOP_IND) )
            {
              // Send the message MMI_SR_RECO_STOP_CON with a bad recognition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_RECO_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id = SC_BAD_RECOGNITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
              // File the message
              ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_RECOGNITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Send the MMI_SR_RECO_STOP_CON with an bad recognition error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_RECO_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->header.msg_id = MMI_SR_RECO_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_RECO_STOP_CON *)(p_message))->error_id = SC_BAD_RECOGNITION;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_RECO_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_RECO_STOP_CON;
              // Fill the message
              ((T_MMI_SR_RECO_STOP_CON *)(conf_msg->SigP))->error_id = SC_BAD_RECOGNITION;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }

    /*---------------------------------------------------------*/
    /* l1a_mmi_sr_update_check_process()                       */
    /*---------------------------------------------------------*/
    /*                                                         */
    /* Description:                                            */
    /* ------------                                            */
    /* This function is a state machine which handles the      */
    /* speech recognition update check feature.                */
    /*                                                         */
    /* Starting messages:        MMI_SR_UPDATE_CHECK_START_REQ */
    /*                                                         */
    /* Result messages (input):  L1_SR_UPDATE_START_CON        */
    /*                                                         */
    /* Result messages (output): MMI_SR_UPDATE_CHECK_START_CON */
    /*                                                         */
    /* Reset messages (input):   none                          */
    /*                                                         */
    /* Stop message (input):     MMI_SR_UPDATE_CHECK_STOP_REQ  */
    /*                           L1_SR_RECO_STOP_IND           */
    /*                                                         */
    /* Stop message (output):    MMI_SR_UPDATE_CHECK_STOP_CON  */
    /*                                                         */
    /* Rem:                                                    */
    /* ----                                                    */
    /*                                                         */
    /*---------------------------------------------------------*/
    void l1a_mmi_sr_update_check_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET                 = 0,
        WAIT_START_REQ        = 1,
        WAIT_MODEL            = 2,
        WAIT_UPDATE_START     = 3,
        WAIT_UPDATE_STOP      = 4,
        WAIT_TEMP_SAVE_DONE   = 5,
        LOAD_MODEL            = 6,
        WAIT_MODEL_LOADED     = 7,
        WAIT_PROCESSING_STOP  = 8,
        WAIT_SAVE_DONE        = 9,
        WAIT_L1S_STOP         = 10,
        WAIT_BACK_STOP        = 11
      };

      UWORD8            *state      = &l1a.state[L1A_SR_UPDATE_CHECK_STATE];
      UWORD32           SignalCode  = msg->SignalCode;
      #if (OP_RIV_AUDIO == 1)
        void              *p_message;
        T_RVF_MB_STATUS   mb_status;
        #define AUDIO_MSG (p_message)
      #else
        xSignalHeaderRec  *conf_msg;
        #define AUDIO_MSG (conf_msg->SigP)
      #endif

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.speechreco_task.command.update_start = FALSE;
            l1a_l1s_com.speechreco_task.command.update_stop  = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_start = FALSE;
            l1a_l1s_com.speechreco_task.command.speech_stop  = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_START_REQ)
            {
              // Download the message parameters to the parameters memory
              l1a_l1s_com.speechreco_task.parameters.database_id        = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->database_id;
              l1a_l1s_com.speechreco_task.parameters.word_index         = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.word_to_check      = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->word_index;
              l1a_l1s_com.speechreco_task.parameters.model_address      = l1s_dsp_com.dsp_ndb_ptr->a_model;
              l1a_l1s_com.speechreco_task.parameters.model_temp_address = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->model_address;
              l1a_l1s_com.speechreco_task.parameters.speech             = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech;
              l1a_l1s_com.speechreco_task.parameters.speech_address     = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->speech_address;
              l1a_l1s_com.speechreco_task.parameters.vocabulary_size    = ((T_MMI_SR_UPDATE_CHECK_REQ *)(msg->SigP))->vocabulary_size;

              // Reset the background task emergency stop
              l1_srback_com.emergency_stop = FALSE;

              // Start to download the model to the API
              l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_SR_UPDATE_CHECK_START_CON);

              *state = WAIT_MODEL;
            }

            // End process
            return;
          }
          break;

          case WAIT_MODEL:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }
            else
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Set the start command of the speech recording task
              l1a_l1s_com.speechreco_task.command.speech_start = l1a_l1s_com.speechreco_task.parameters.speech;

              // Start the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_start = TRUE;

              *state = WAIT_UPDATE_START;
            }

            // End process
            return;
          }
          break;

          case WAIT_UPDATE_START:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;
            }
            else
            if (SignalCode == L1_SR_UPDATE_START_CON)
            {
              // Reset the commands
              l1a_l1s_com.speechreco_task.command.update_start = FALSE;
              l1a_l1s_com.speechreco_task.command.speech_start = FALSE;

              *state = WAIT_UPDATE_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_UPDATE_STOP:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the speech recognition update task
              l1a_l1s_com.speechreco_task.command.update_stop = TRUE;

              // Stop the speech recording task (if present)
              l1a_l1s_com.speechreco_task.command.speech_stop = l1a_l1s_com.speechreco_task.parameters.speech;

              *state = WAIT_L1S_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SR_UPDATE_STOP_CON)
            {
              // There is an error during the update task?
              if ( ((T_L1_SR_UPDATE_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // Reset the background task emergency stop
                l1_srback_com.emergency_stop = FALSE;

                // Send the message L1_SRBACK_SAVE_DATA_REQ to the background task
                l1_send_sr_background_msg(L1_SRBACK_TEMP_SAVE_DATA_REQ);

                *state = WAIT_TEMP_SAVE_DONE;

                // End process
                return;
              }
              else
              {
                // Forward the stop confirmation message
                l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

                *state = RESET;
              }
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_TEMP_SAVE_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SRBACK_TEMP_SAVE_DATA_CON)
            {
              // Reset the command
              l1a_l1s_com.speechreco_task.command.processing_start  = FALSE;
              l1a_l1s_com.speechreco_task.command.processing_stop   = FALSE;

              // Reset the index counter
              l1a_l1s_com.speechreco_task.parameters.index_counter = 0;

              // The CTO algorithm must be used?
              if (l1a_l1s_com.speechreco_task.parameters.vocabulary_size <= SC_SR_MAX_WORDS_FOR_CTO)
              {
                // Enable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = TRUE;

                // Double the vocabulary size
                l1a_l1s_com.speechreco_task.parameters.vocabulary_size <<= 1;
              }
              else
              {
                // Disable the CTO algorithm
                l1a_l1s_com.speechreco_task.parameters.CTO_algorithm = FALSE;
              }

              *state = LOAD_MODEL;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case LOAD_MODEL:
          {
            // Initialize the background task stop command
            l1_srback_com.emergency_stop = FALSE;

            // Start to load the model to the API
            l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.index_counter;
            l1_send_sr_background_msg(L1_SRBACK_LOAD_MODEL_REQ);

            // Increase the index counter
            l1a_l1s_com.speechreco_task.parameters.index_counter++;

            *state = WAIT_MODEL_LOADED;

            // End process
            return;
          }
          break;

          case WAIT_MODEL_LOADED:
          {
            if (SignalCode == L1_SRBACK_LOAD_MODEL_CON)
            {
              // Start the DSP processing task
              l1a_l1s_com.speechreco_task.command.processing_start = TRUE;

              *state = WAIT_PROCESSING_STOP;
            }
            else
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;
            }

            // End process
            return;
          }
          break;

          case WAIT_PROCESSING_STOP:
          {
            if (SignalCode == L1_SR_PROCESSING_STOP_CON)
            {
              // The processing phase is good or not?
              if ( ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                *state = LOAD_MODEL;
              }
              else
              {
                // Send the MMI_SR_UPDATE_CHECK_STOP_CON message with an error
              #if (OP_RIV_AUDIO == 1)
                // Allocate the Riviera buffer
                mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                        sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                        (T_RVF_BUFFER **) (&p_message));

                // If insufficient resources, then report a memory error and abort.
                if (mb_status == RVF_RED)
                {
                  // the memory is insufficient to continue the non regression test
                  AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                  return;
                }

                // Fill the message ID
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

                // Fill the message parameter
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id =
                  ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                // send the messsage to the audio entity
                rvf_send_msg (p_audio_gbl_var->addrId,
                              p_message);
              #else // OP_RIV_AUDIO
                // Allocate confirmation message...
                conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
                DEBUGMSG(status,NU_ALLOC_ERR)
                conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;
                // File the message
                ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = ((T_L1_SR_PROCESSING_STOP_CON *)(msg->SigP))->error_id;

                #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                    l1_trace_message(conf_msg);
                #endif

                // Send the confirmation message...
                os_send_sig(conf_msg, MMI_QUEUE);
                DEBUGMSG(status,NU_SEND_QUEUE_ERR)
              #endif // OP_RIV_AUDIO

                *state = RESET;
              }
            }
            else
            if (SignalCode == L1_SR_RECO_STOP_IND)
            {
              // There is an error during the recognition?
              if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id == SC_NO_ERROR )
              {
                // The CTO algorithm is used?
                if (l1a_l1s_com.speechreco_task.parameters.CTO_algorithm)
                {
                  // The best word is odd?
                  if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index & 0x01 )
                  {
                    // Change the error to SC_CTO_WORD
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CTO_WORD;

                    // Forward the message in the MMI_SR_RECO_STOP_CON
                    l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

                    *state = RESET;

                    // End process
                    return;
                  }
                  else
                  {
                    // Devided by 2 the 4 indexes of the best words in the message
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index         >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index  >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index   >>= 1;
                    ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index  >>= 1;
                  }
                }
                // Is it the good word?
                if ( ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index ==
                     l1a_l1s_com.speechreco_task.parameters.word_to_check )
                {
                  // Save the message informations in the l1a_l1s_com memory
                  l1a_l1s_com.speechreco_task.parameters.best_word_index        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.best_word_score        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.second_best_word_index = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.second_best_word_score = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->second_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.third_best_word_index  = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.third_best_word_score  = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->third_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.fourth_best_word_index = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_index;
                  l1a_l1s_com.speechreco_task.parameters.fourth_best_word_score = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->fourth_best_word_score;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_db_level          = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_level;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_db_noise          = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_db_noise;
                  l1a_l1s_com.speechreco_task.parameters.d_sr_model_size        = ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->d_sr_model_size;

                  // Reset the stop background task
                  l1_srback_com.emergency_stop = FALSE;

                  // Start the background task to save the model in the database
                  l1a_l1s_com.speechreco_task.parameters.word_index = l1a_l1s_com.speechreco_task.parameters.word_to_check;
                  l1a_l1s_com.speechreco_task.parameters.model_address = l1a_l1s_com.speechreco_task.parameters.model_temp_address;
                  l1_send_sr_background_msg(L1_SRBACK_SAVE_DATA_REQ);

                  *state = WAIT_SAVE_DONE;

                  // End process
                  return;
                }
                else
                {
                  // Change the error to SC_CHECK_ERROR
                  ((T_L1_SR_RECO_STOP_IND *)(msg->SigP))->error_id = SC_CHECK_ERROR;
                }
              }
              // Forward the message in the MMI_SR_RECO_STOP_CON
              l1a_audio_send_result(MMI_SR_UPDATE_CHECK_STOP_CON, msg, MMI_QUEUE);

              *state = RESET;
            }
            else
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop the L1S processing task
              l1a_l1s_com.speechreco_task.command.processing_stop = TRUE;

              *state = WAIT_L1S_STOP;

              // end process
              return;
            }
            else
            {
              // end process
              return;
            }
          }
          break;

          case WAIT_SAVE_DONE:
          {
            if (SignalCode == MMI_SR_UPDATE_CHECK_STOP_REQ)
            {
              // Stop immediatly the background task
              l1_srback_com.emergency_stop = TRUE;

              *state = WAIT_BACK_STOP;

              // End process
              return;
            }
            else
            if (SignalCode == L1_SRBACK_SAVE_DATA_CON)
            {
              // Send the MMI_SR_UPDATE_CHECK_STOP_CON message with an error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;
            #endif // OP_RIV_AUDIO

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->error_id                = SC_NO_ERROR;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->best_word_index         = l1a_l1s_com.speechreco_task.parameters.best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->best_word_score         = l1a_l1s_com.speechreco_task.parameters.best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->second_best_word_index  = l1a_l1s_com.speechreco_task.parameters.second_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->second_best_word_score  = l1a_l1s_com.speechreco_task.parameters.second_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->third_best_word_index   = l1a_l1s_com.speechreco_task.parameters.third_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->third_best_word_score   = l1a_l1s_com.speechreco_task.parameters.third_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->fourth_best_word_index  = l1a_l1s_com.speechreco_task.parameters.fourth_best_word_index;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->fourth_best_word_score  = l1a_l1s_com.speechreco_task.parameters.fourth_best_word_score;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_db_level           = l1a_l1s_com.speechreco_task.parameters.d_sr_db_level;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_db_noise           = l1a_l1s_com.speechreco_task.parameters.d_sr_db_noise;
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(AUDIO_MSG))->d_sr_model_size         = l1a_l1s_com.speechreco_task.parameters.d_sr_model_size;

            #if (OP_RIV_AUDIO == 1)
              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_BACK_STOP:
          {
            if ( (SignalCode == L1_SRBACK_SAVE_DATA_CON)  ||
                 (SignalCode == L1_SRBACK_LOAD_MODEL_CON) ||
                 (SignalCode == L1_SRBACK_TEMP_SAVE_DATA_CON) )
            {
              // Send the message MMI_SR_UPDATE_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id                = SC_CHECK_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = SC_CHECK_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;

          case WAIT_L1S_STOP:
          {
            if ( (SignalCode == L1_SR_UPDATE_STOP_CON)     ||
                 (SignalCode == L1_SR_PROCESSING_STOP_CON) ||
                 (SignalCode == L1_SR_RECO_STOP_CON)       ||
                 (SignalCode == L1_SR_RECO_STOP_IND) )
            {
              // Send the message MMI_SR_UPDATE_CHECK_STOP_CON with an update error
            #if (OP_RIV_AUDIO == 1)
              // Allocate the Riviera buffer
              mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                      sizeof (T_MMI_SR_UPDATE_CHECK_STOP_CON),
                                      (T_RVF_BUFFER **) (&p_message));

              // If insufficient resources, then report a memory error and abort.
              if (mb_status == RVF_RED)
              {
                // the memory is insufficient to continue the non regression test
                AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
                return;
              }

              // Fill the message ID
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->header.msg_id = MMI_SR_UPDATE_CHECK_STOP_CON;

              // Fill the message parameter
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(p_message))->error_id                = SC_CHECK_ERROR;

              // send the messsage to the audio entity
              rvf_send_msg (p_audio_gbl_var->addrId,
                            p_message);
            #else // OP_RIV_AUDIO
              // Allocate confirmation message...
              conf_msg = os_alloc_sig(sizeof(T_MMI_SR_UPDATE_CHECK_STOP_CON));
              DEBUGMSG(status,NU_ALLOC_ERR)
              conf_msg->SignalCode = MMI_SR_UPDATE_CHECK_STOP_CON;

              //Fill the message
              ((T_MMI_SR_UPDATE_CHECK_STOP_CON *)(conf_msg->SigP))->error_id = SC_CHECK_ERROR;

              #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
                  l1_trace_message(conf_msg);
              #endif

              // Send the confirmation message...
              os_send_sig(conf_msg, MMI_QUEUE);
              DEBUGMSG(status,NU_SEND_QUEUE_ERR)
            #endif // OP_RIV_AUDIO

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)

      // Undefine message pointer macro.
      #undef MSG_AUDIO

    }

  #endif // L1_DYN_DSP_DWNLD
  #endif // SPEECH_RECO
  #if (L1_AEC == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_aec_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* AEC feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_AEC_REQ                 */
    /*                                                       */
    /* Result messages (input):  none                        */
    /*                                                       */
    /* Result messages (output): MMI_AEC_CON                 */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
#if (L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_aec_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET         = 0,
        WAIT_AEC_REQ  = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_AEC_CON  = 3
      };

      UWORD8    *state      = &l1a.state[L1A_AEC_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.aec_task.command.start = FALSE;

            *state = WAIT_AEC_REQ;
          }
          break;

          case WAIT_AEC_REQ:
          {
            if (SignalCode == MMI_AEC_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.aec_task.parameters.aec_control = ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control;
            #if (L1_NEW_AEC)
              l1a_l1s_com.aec_task.parameters.cont_filter     = ((T_MMI_AEC_REQ *)(msg->SigP))->cont_filter;
              l1a_l1s_com.aec_task.parameters.granularity_att = ((T_MMI_AEC_REQ *)(msg->SigP))->granularity_att;
              l1a_l1s_com.aec_task.parameters.coef_smooth     = ((T_MMI_AEC_REQ *)(msg->SigP))->coef_smooth;
              l1a_l1s_com.aec_task.parameters.es_level_max    = ((T_MMI_AEC_REQ *)(msg->SigP))->es_level_max;
              l1a_l1s_com.aec_task.parameters.fact_vad        = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_vad;
              l1a_l1s_com.aec_task.parameters.thrs_abs        = ((T_MMI_AEC_REQ *)(msg->SigP))->thrs_abs;
              l1a_l1s_com.aec_task.parameters.fact_asd_fil    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_fil;
              l1a_l1s_com.aec_task.parameters.fact_asd_mut    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_mut;
            #endif

            #if (L1_ANR == 1)
              if (l1a_l1s_com.aec_task.parameters.aec_control & 0x0004)
              {
                // Noise suppression enabled: enable new ANR module for backward compatibility
                l1a_l1s_com.aec_task.parameters.aec_control &= ~(0x0104); // Clear noise suppression bits in AEC control

                // Enable L1S ANR task (default settings are used)
                l1a_l1s_com.anr_task.parameters.anr_enable       = 1;
                l1a_l1s_com.anr_task.parameters.min_gain         = 0x3313;
                l1a_l1s_com.anr_task.parameters.div_factor_shift = -2;
                l1a_l1s_com.anr_task.parameters.ns_level         = 1;
                l1a_l1s_com.anr_task.command.update              = TRUE;

                // Here we do not wait for L1S confirmation to have simple implementation
                // because the state machine already wait for AEC confirmation and
                // ANR confirmation would occur on the same frame
              }
              else
              {
                // Noise suppression disabled: disable ANR
                l1a_l1s_com.anr_task.parameters.anr_enable       = 0;
                l1a_l1s_com.anr_task.command.update              = TRUE;

                // Here we do not wait for L1S confirmation to have simple implementation
                // because the state machine already wait for AEC confirmation and
                // ANR confirmation would occur on the same frame
              }
            #endif
              if (l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
              {
                // WARNING: The following code must be duplicated in WAIT_DYN_DWNLD state when
                // activating the command at L1s level

                // Start the L1S AEC task
                l1a_l1s_com.aec_task.command.start = TRUE;

              *state = WAIT_AEC_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AEC SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if (SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
            {
              // Start the L1S AEC task
              l1a_l1s_com.aec_task.command.start = TRUE;

              *state = WAIT_AEC_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AEC SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
          return;
          }
          break;
          case WAIT_AEC_CON:
          {
            if (SignalCode == L1_AEC_CON)
            {
              // Send the AEC confirmation message
              l1a_audio_send_confirmation(MMI_AEC_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_aec_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET         = 0,
        WAIT_AEC_REQ  = 1,
        WAIT_AEC_CON  = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AEC_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.aec_task.command.start = FALSE;

            *state = WAIT_AEC_REQ;
          }
          break;

          case WAIT_AEC_REQ:
          {
            if (SignalCode == MMI_AEC_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.aec_task.parameters.aec_control = ((T_MMI_AEC_REQ *)(msg->SigP))->aec_control;
            #if (L1_NEW_AEC)
              l1a_l1s_com.aec_task.parameters.cont_filter     = ((T_MMI_AEC_REQ *)(msg->SigP))->cont_filter;
              l1a_l1s_com.aec_task.parameters.granularity_att = ((T_MMI_AEC_REQ *)(msg->SigP))->granularity_att;
              l1a_l1s_com.aec_task.parameters.coef_smooth     = ((T_MMI_AEC_REQ *)(msg->SigP))->coef_smooth;
              l1a_l1s_com.aec_task.parameters.es_level_max    = ((T_MMI_AEC_REQ *)(msg->SigP))->es_level_max;
              l1a_l1s_com.aec_task.parameters.fact_vad        = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_vad;
              l1a_l1s_com.aec_task.parameters.thrs_abs        = ((T_MMI_AEC_REQ *)(msg->SigP))->thrs_abs;
              l1a_l1s_com.aec_task.parameters.fact_asd_fil    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_fil;
              l1a_l1s_com.aec_task.parameters.fact_asd_mut    = ((T_MMI_AEC_REQ *)(msg->SigP))->fact_asd_mut;
            #endif

            #if (L1_ANR == 1)
              if (l1a_l1s_com.aec_task.parameters.aec_control & 0x0004)
              {
                // Noise suppression enabled: enable new ANR module for backward compatibility
                l1a_l1s_com.aec_task.parameters.aec_control &= ~(0x0104); // Clear noise suppression bits in AEC control

                // Enable L1S ANR task (default settings are used)
                l1a_l1s_com.anr_task.parameters.anr_enable       = 1;
                l1a_l1s_com.anr_task.parameters.min_gain         = 0x3313;
                l1a_l1s_com.anr_task.parameters.div_factor_shift = -2;
                l1a_l1s_com.anr_task.parameters.ns_level         = 1;
                l1a_l1s_com.anr_task.command.update              = TRUE;

                // Here we do not wait for L1S confirmation to have simple implementation
                // because the state machine already wait for AEC confirmation and
                // ANR confirmation would occur on the same frame
              }
              else
              {
                // Noise suppression disabled: disable ANR
                l1a_l1s_com.anr_task.parameters.anr_enable       = 0;
                l1a_l1s_com.anr_task.command.update              = TRUE;

                // Here we do not wait for L1S confirmation to have simple implementation
                // because the state machine already wait for AEC confirmation and
                // ANR confirmation would occur on the same frame
              }
            #endif

              // Start the L1S AEC task
              l1a_l1s_com.aec_task.command.start = TRUE;

              *state = WAIT_AEC_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_AEC_CON:
          {
            if (SignalCode == L1_AEC_CON)
            {
              // Send the AEC confirmation message
              l1a_audio_send_confirmation(MMI_AEC_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // L1_DYN_DSP_DWNLD == 1
  #endif // AEC

  #if (L1_AEC == 2)
      /*-------------------------------------------------------*/
      /* l1a_mmi_aec_process()                                 */
      /*-------------------------------------------------------*/
      /*                                                       */
      /* Description:                                          */
      /* ------------                                          */
      /* This function is a state machine which handles the    */
      /* AEC feature.                                          */
      /*                                                       */
      /* Starting messages:        MMI_AEC_REQ                 */
      /*                                                       */
      /* Result messages (input):  none                        */
      /*                                                       */
      /* Result messages (output): MMI_AEC_CON                 */
      /*                                                       */
      /* Reset messages (input):   none                        */
      /*                                                       */
      /* Stop message (input):     none                        */
      /*                                                       */
      /* Stop message (output):    none                        */
      /*                                                       */
      /* Rem:                                                  */
      /* ----                                                  */
      /*                                                       */
      /*-------------------------------------------------------*/
  #if (L1_DYN_DSP_DWNLD == 1)
      void l1a_mmi_aec_process(xSignalHeaderRec *msg)
      {
        enum states
        {
          RESET         = 0,
          WAIT_AEC_REQ  = 1,
          WAIT_DYN_DWNLD = 2,
          WAIT_AEC_CON  = 3
        };

        UWORD8    *state      = &l1a.state[L1A_AEC_STATE];
        UWORD32   SignalCode  = msg->SignalCode;

        while(1)
        {
          switch(*state)
          {
            case RESET:
            {
              // Reset the command
              l1a_l1s_com.aec_task.command.start = FALSE;

              *state = WAIT_AEC_REQ;
            }
            break;

            case WAIT_AEC_REQ:
            {
              if (SignalCode == MMI_AQI_AEC_REQ)
              {

                // Load the message into the l1a_l1s_com memory.
                l1a_l1s_com.aec_task.aec_control = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_control;

                if(l1a_l1s_com.aec_task.aec_control != L1_AQI_AEC_STOP)
                {

                	l1a_l1s_com.aec_task.parameters.cont_filter     = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.cont_filter;
                	l1a_l1s_com.aec_task.parameters.granularity_att = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.granularity_att;
                	l1a_l1s_com.aec_task.parameters.coef_smooth     = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.coef_smooth;
                	l1a_l1s_com.aec_task.parameters.es_level_max    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.es_level_max;
                	l1a_l1s_com.aec_task.parameters.fact_vad        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_vad;
                	l1a_l1s_com.aec_task.parameters.thrs_abs        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.thrs_abs;
                	l1a_l1s_com.aec_task.parameters.fact_asd_fil    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_fil;
                	l1a_l1s_com.aec_task.parameters.fact_asd_mut    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_mut;
                	l1a_l1s_com.aec_task.parameters.aec_mode        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.aec_mode;
                	l1a_l1s_com.aec_task.parameters.mu			    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.mu;
                	l1a_l1s_com.aec_task.parameters.scale_input_ul  = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_ul;
                	l1a_l1s_com.aec_task.parameters.scale_input_dl  = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_dl;
                	l1a_l1s_com.aec_task.parameters.div_dmax		= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_dmax;
                	l1a_l1s_com.aec_task.parameters.div_swap_good	= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_good;
                	l1a_l1s_com.aec_task.parameters.div_swap_bad	= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_bad;
                	l1a_l1s_com.aec_task.parameters.block_init	    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_init;
//                	l1a_l1s_com.aec_task.parameters.block_size	    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_size;

				}

                if (l1a.dyn_dwnld.semaphore_vect[AEC_STATE_MACHINE] == GREEN)
                {
                  // WARNING: The following code must be duplicated in WAIT_DYN_DWNLD state when
                  // activating the command at L1s level

                  // Start the L1S AEC task
                  l1a_l1s_com.aec_task.command.start = TRUE;

                *state = WAIT_AEC_CON;
                }
                else
                {
                  *state = WAIT_DYN_DWNLD;

                  #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                    if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                    {
                      char str[30];
                      sprintf(str,"AEC SM blocked by DYN DWNLD\r\n");
                      #if(CODE_VERSION == SIMULATION)
                        trace_fct_simu_dyn_dwnld(str);
                      #else
                        rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                      #endif
                    }
                  #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
                }
              }

              // End process
              return;
            }
 // omaps00090550             break;
            case WAIT_DYN_DWNLD:
            {
              if (SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
              {
                // Start the L1S AEC task
                l1a_l1s_com.aec_task.command.start = TRUE;

                *state = WAIT_AEC_CON;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                    {
                      char str[30];
                      sprintf(str,"AEC SM un-blocked\r\n");
                      #if(CODE_VERSION == SIMULATION)
                        trace_fct_simu_dyn_dwnld(str);
                      #else
                        rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                      #endif
                    }
                 #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            return;
            }
 // omaps00090550          break;
            case WAIT_AEC_CON:
            {
              if (SignalCode == L1_AQI_AEC_CON)
              {
                // Send the AEC confirmation message
                l1a_audio_send_result(MMI_AQI_AEC_CON, msg, MMI_QUEUE);

                    *state = RESET;
              }
              else
              {
                // End process
                return;
              }
            }
            break;
          } // switch
        } // while(1)
      }
  #else
      void l1a_mmi_aec_process(xSignalHeaderRec *msg)
      {
        enum states
        {
          RESET         = 0,
          WAIT_AEC_REQ  = 1,
          WAIT_AEC_CON  = 2
        };

        UWORD8    *state      = &l1a.state[L1A_AEC_STATE];
        UWORD32   SignalCode  = msg->SignalCode;

        while(1)
        {
          switch(*state)
          {
            case RESET:
            {
              // Reset the command
              l1a_l1s_com.aec_task.command.start = FALSE;

              *state = WAIT_AEC_REQ;
            }
            break;

            case WAIT_AEC_REQ:
            {
              if (SignalCode == MMI_AQI_AEC_REQ)
              {
                // Load the message into the l1a_l1s_com memory.
                l1a_l1s_com.aec_task.aec_control = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_control;

                if(l1a_l1s_com.aec_task.aec_control != L1_AQI_AEC_STOP)
                {

                	l1a_l1s_com.aec_task.parameters.cont_filter     = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.cont_filter;
                	l1a_l1s_com.aec_task.parameters.granularity_att = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.granularity_att;
                	l1a_l1s_com.aec_task.parameters.coef_smooth     = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.coef_smooth;
                	l1a_l1s_com.aec_task.parameters.es_level_max    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.es_level_max;
                	l1a_l1s_com.aec_task.parameters.fact_vad        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_vad;
                	l1a_l1s_com.aec_task.parameters.thrs_abs        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.thrs_abs;
                	l1a_l1s_com.aec_task.parameters.fact_asd_fil    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_fil;
                	l1a_l1s_com.aec_task.parameters.fact_asd_mut    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.fact_asd_mut;
                	l1a_l1s_com.aec_task.parameters.aec_mode        = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.aec_mode;
                	l1a_l1s_com.aec_task.parameters.mu			    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.mu;
                	l1a_l1s_com.aec_task.parameters.scale_input_ul  = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_ul;
                	l1a_l1s_com.aec_task.parameters.scale_input_dl  = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.scale_input_dl;
                	l1a_l1s_com.aec_task.parameters.div_dmax		= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_dmax;
                	l1a_l1s_com.aec_task.parameters.div_swap_good	= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_good;
                	l1a_l1s_com.aec_task.parameters.div_swap_bad	= ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.div_swap_bad;
                	l1a_l1s_com.aec_task.parameters.block_init	    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_init;
//                	l1a_l1s_com.aec_task.parameters.block_size	    = ((T_MMI_AQI_AEC_REQ *)(msg->SigP))->aec_parameters.block_size;

				}

              #if (L1_ANR == 1)
                if (l1a_l1s_com.aec_task.aec_control & 0x0004)
                {
                  // Noise suppression enabled: enable new ANR module for backward compatibility
                  l1a_l1s_com.aec_task.aec_control &= ~(0x0104); // Clear noise suppression bits in AEC control

                  // Enable L1S ANR task (default settings are used)
                  l1a_l1s_com.anr_task.parameters.anr_enable       = 1;
                  l1a_l1s_com.anr_task.parameters.min_gain         = 0x3313;
                  l1a_l1s_com.anr_task.parameters.div_factor_shift = -2;
                  l1a_l1s_com.anr_task.parameters.ns_level         = 1;
                  l1a_l1s_com.anr_task.command.update              = TRUE;

                  // Here we do not wait for L1S confirmation to have simple implementation
                  // because the state machine already wait for AEC confirmation and
                  // ANR confirmation would occur on the same frame
                }
                else
                {
                  // Noise suppression disabled: disable ANR
                  l1a_l1s_com.anr_task.parameters.anr_enable       = 0;
                  l1a_l1s_com.anr_task.command.update              = TRUE;

                  // Here we do not wait for L1S confirmation to have simple implementation
                  // because the state machine already wait for AEC confirmation and
                  // ANR confirmation would occur on the same frame
                }
              #endif

                // Start the L1S AEC task
                l1a_l1s_com.aec_task.command.start = TRUE;

                *state = WAIT_AEC_CON;
              }

              // End process
              return;
            }
            break;

            case WAIT_AEC_CON:
            {
              if (SignalCode == L1_AQI_AEC_CON)
              {
                // Send the AEC confirmation message
                l1a_audio_send_result(MMI_AQI_AEC_CON, msg, MMI_QUEUE);

                    *state = RESET;
              }
              else
              {
                // End process
                return;
              }
            }
            break;
          } // switch
        } // while(1)
      }
    #endif // L1_DYN_DSP_DWNLD == 1
    #endif // L1_AEC == 2


  #if (FIR)
    /*-------------------------------------------------------*/
    /* l1a_mmi_fir_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* FIR feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_AUDIO_FIR_REQ           */
    /*                                                       */
    /* Result messages (input):  none                        */
    /*                                                       */
    /* Result messages (output): MMI_AUDIO_FIR_CON           */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_fir_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET         = 0,
        WAIT_FIR_REQ  = 1,
        WAIT_FIR_CON  = 2
      };

      UWORD8    *state      = &l1a.state[L1A_FIR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.fir_task.command.start = FALSE;

            *state = WAIT_FIR_REQ;
          }
          break;

          case WAIT_FIR_REQ:
          {
            if (SignalCode == MMI_AUDIO_FIR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.fir_task.parameters.fir_loop = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_loop;
              l1a_l1s_com.fir_task.parameters.update_fir = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->update_fir;
              l1a_l1s_com.fir_task.parameters.fir_ul_coefficient = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_ul_coefficient;

              // we update FIR coefficients even if L1_IIR==1 because in case of loop mode
              // this is the old FIR API that is used
              l1a_l1s_com.fir_task.parameters.fir_dl_coefficient = ((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_dl_coefficient;

              #if (L1_IIR == 1)
                if (l1a_l1s_com.fir_task.parameters.update_fir & DL_FIR)
                {
                  // FIR DL update enabled: enable new IIR/DL module for backward compatibility

                  // Enable L1S IIR task
                  // Settings tuned to have same behavior as DL FIR
                  l1a_l1s_com.iir_task.parameters.iir_enable          = 1; // Filter always enabled
                  l1a_l1s_com.iir_task.parameters.nb_iir_blocks       = 0; // IIR part disabled
                  l1a_l1s_com.iir_task.parameters.iir_coefs           = 0;
                  l1a_l1s_com.iir_task.parameters.nb_fir_coefs        = 0x1f;
                  l1a_l1s_com.iir_task.parameters.fir_coefs           = (WORD16 *)((T_MMI_AUDIO_FIR_REQ *)(msg->SigP))->fir_dl_coefficient;
                  l1a_l1s_com.iir_task.parameters.input_scaling       = 0;
                  l1a_l1s_com.iir_task.parameters.fir_scaling         = 0;
                  l1a_l1s_com.iir_task.parameters.input_gain_scaling  = 0;
                  l1a_l1s_com.iir_task.parameters.output_gain_scaling = 0;
                  l1a_l1s_com.iir_task.parameters.output_gain         = 0xffff; // Used for IIR using in FIR mode
                  l1a_l1s_com.iir_task.parameters.feedback            = 0;
                  l1a_l1s_com.iir_task.command.update                 = TRUE;

                  // Here we do not wait for L1S confirmation to have simple implementation
                  // because the state machine already wait for FIR confirmation and
                  // there is no chance that upper layer frees the memory used to store
                  // DL coefficients before L1S copy them into API
                }
              #endif

              // Start the L1S FIR task
              l1a_l1s_com.fir_task.command.start = TRUE;

              *state = WAIT_FIR_CON;
            }

            // End process
            return;
          }
 // omaps00090550        break;

          case WAIT_FIR_CON:
          {
            if (SignalCode == L1_AUDIO_FIR_CON)
            {
              // Send the FIR confirmation message
              l1a_audio_send_confirmation(MMI_AUDIO_FIR_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // FIR
  #if (AUDIO_MODE)
    /*-------------------------------------------------------*/
    /* l1a_mmi_audio_mode_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* audio mode features.                                  */
    /*                                                       */
    /* Starting messages:        MMI_AUDIO_MODE_REQ          */
    /*                                                       */
    /* Result messages (input):  none                        */
    /*                                                       */
    /* Result messages (output): MMI_AUDIO_MODE_CON          */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_audio_mode_process (xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET                = 0,
        WAIT_AUDIO_MODE_REQ  = 1,
        WAIT_AUDIO_MODE_CON  = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AUDIO_MODE_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.audio_mode_task.command.start = FALSE;

            *state = WAIT_AUDIO_MODE_REQ;
          }
          break;

          case WAIT_AUDIO_MODE_REQ:
          {
            if (SignalCode == MMI_AUDIO_MODE_REQ)
            {
              switch (((T_MMI_AUDIO_MODE *)(msg->SigP))->audio_mode)
              {
                case GSM_ONLY:
                {
                  // Set the GSM only mode
                  l1a_l1s_com.audio_mode_task.parameters.audio_mode = B_GSM_ONLY;
                  break;
                }
                case BT_CORDLESS:
                {
                  // Set the bluetooth cordless mode
                  l1a_l1s_com.audio_mode_task.parameters.audio_mode = B_BT_CORDLESS;
                  break;
                }
                case BT_HEADSET:
                {
                  // Set the bluetooth headset mode
                  l1a_l1s_com.audio_mode_task.parameters.audio_mode = B_BT_HEADSET;
                  break;
                }
                default :
                {
                  break;
                }
              } // switch

              // Start the L1S AUDIO MODE task
              l1a_l1s_com.audio_mode_task.command.start = TRUE;

              *state = WAIT_AUDIO_MODE_CON;
            }

            // End process
            return;
          }
 // omaps00090550          break;

          case WAIT_AUDIO_MODE_CON:
          {
            if (SignalCode == L1_AUDIO_MODE_CON)
            {
              // Send the AUDIO MODE confirmation message
              l1a_audio_send_confirmation(MMI_AUDIO_MODE_CON);

              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // AUDIO_MODE
  #if (MELODY_E2)
  #if(L1_DYN_DSP_DWNLD==1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_melody0_e2_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 0 format E2 feature.                           */
    /*                                                       */
    /* Starting messages:        MMI_MELODY0_E2_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_MELODY0_E2_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_MELODY0_E2_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY0_E2_STOP_REQ     */
    /*                           L1_MELODY0_E2_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY0_E2_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_melody0_e2_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M0_RESET                  = 0,
        M0_WAIT_START_REQ         = 1,
        WAIT_DYN_DWNLD = 2,
        M0_WAIT_LOAD_INSTRUMENT   = 3,
        M0_WAIT_STOP              = 4,
        M0_WAIT_UNLOAD_INSTRUMENT = 5
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY0_E2_STATE], i;
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case M0_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody0_e2_task.command.start = FALSE;
            l1a_l1s_com.melody0_e2_task.command.stop  = FALSE;

            *state = M0_WAIT_START_REQ;
          }
          break;

          case M0_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY0_E2_START_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.melody0_e2_task.parameters.session_id = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody0_e2_task.parameters.loopback   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

              if(l1a.dyn_dwnld.semaphore_vect[E2_STATE_MACHINE]==GREEN)
              {

                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Disable trace DSP upon E2 MELODY activation
                l1_disable_dsp_trace();
#endif
                // Reset the emergency flag
                l1a_l1s_com.melody0_e2_task.parameters.emergency_stop = FALSE;

                // Initialize the buffer parameters
                l1a_l1s_com.melody0_e2_task.parameters.ptr_buf        = NULL;
                l1a_l1s_com.melody0_e2_task.parameters.buffer_size    = 0;
                l1a_l1s_com.melody0_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody0_e2_task.parameters.session_id);

                // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
                l1a_l1s_com.melody0_e2_task.parameters.buffer_size <<= 1;

                // Jump the NumberOfOscillator parameter
                l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

                // Read the Header of the melody description in order to download the time factor
                // clean the MSB of the global time factor register
                l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
                l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
                l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


                // Find the number of insturment of the melody (jump the header memory)
                l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument));

                for (i=0; i<(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument); i++)
                {
                  // find the beginning of the melody description (after the header field)
                  // and put it in the buf_ptr buffer
                  l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody0_e2_task.parameters.waves_table_id[i]));
                }

                // Initialize the size (in byte unit) of the header
                l1a_l1s_com.melody0_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument;

                // download the instrument
                l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 0);

                // Send the start confirmation message
                l1a_audio_send_confirmation(MMI_MELODY0_E2_START_CON);

                *state = M0_WAIT_LOAD_INSTRUMENT;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"E20 SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[E2_STATE_MACHINE] == GREEN))
            {
              // Reset the emergency flag
              l1a_l1s_com.melody0_e2_task.parameters.emergency_stop = FALSE;

              // Initialize the buffer parameters
              l1a_l1s_com.melody0_e2_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody0_e2_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody0_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody0_e2_task.parameters.session_id);

              // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
              l1a_l1s_com.melody0_e2_task.parameters.buffer_size <<= 1;

              // Jump the NumberOfOscillator parameter
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

              // Read the Header of the melody description in order to download the time factor
              // clean the MSB of the global time factor register
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


              // Find the number of insturment of the melody (jump the header memory)
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument));

              for (i=0; i<(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument); i++)
              {
                // find the beginning of the melody description (after the header field)
                // and put it in the buf_ptr buffer
                l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody0_e2_task.parameters.waves_table_id[i]));
              }

              // Initialize the size (in byte unit) of the header
              l1a_l1s_com.melody0_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument;

              // download the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 0);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY0_E2_START_CON);

              *state = M0_WAIT_LOAD_INSTRUMENT;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"E20 SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
          break;
          case M0_WAIT_LOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 0 )
              {
                // The load instrument confirmation message is for the melody 0
                if (l1a_l1s_com.melody0_e2_task.parameters.emergency_stop)
                {
                  // Unload the instrument
                  l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 0);

                  *state = M0_WAIT_UNLOAD_INSTRUMENT;

                }
                else
                {
                  // Start to play the melody0
                  l1a_l1s_com.melody0_e2_task.command.start = TRUE;

                  *state = M0_WAIT_STOP;
                }
              }
            }
            else
            if (SignalCode == MMI_MELODY0_E2_STOP_REQ)
            {
              // Set the emergency flag
              l1a_l1s_com.melody0_e2_task.parameters.emergency_stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M0_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY0_E2_STOP_CON)
            {
              // Unload the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 0);

              *state = M0_WAIT_UNLOAD_INSTRUMENT;
            }
            else
            if (SignalCode == MMI_MELODY0_E2_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody0_e2_task.command.stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M0_WAIT_UNLOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 0 )
              {
                // Send the stop confirmation message
                l1a_audio_send_confirmation(MMI_MELODY0_E2_STOP_CON);
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon E2 MELODY deactivation
                l1_enable_dsp_trace();
#endif

                *state = M0_RESET;
              }
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_melody1_e2_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 1 format E2 feature.                           */
    /*                                                       */
    /* Starting messages:        MMI_MELODY1_E2_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_MELODY1_E2_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_MELODY1_E2_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY0_E2_STOP_REQ     */
    /*                           L1_MELODY1_E2_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY1_E2_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_melody1_e2_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M1_RESET                  = 0,
        M1_WAIT_START_REQ         = 1,
        WAIT_DYN_DWNLD = 2,
        M1_WAIT_LOAD_INSTRUMENT   = 3,
        M1_WAIT_STOP              = 4,
        M1_WAIT_UNLOAD_INSTRUMENT = 5
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY1_E2_STATE], i;
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case M1_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody1_e2_task.command.start = FALSE;
            l1a_l1s_com.melody1_e2_task.command.stop  = FALSE;

            *state = M1_WAIT_START_REQ;
          }
          break;

          case M1_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY1_E2_START_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.melody1_e2_task.parameters.session_id = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody1_e2_task.parameters.loopback   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

              if(l1a.dyn_dwnld.semaphore_vect[E2_STATE_MACHINE] == GREEN)
              {
                // WARNING: code below must be duplicated in WAIT_DYN_DWNLD state
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Disable trace DSP upon E2 MELODY activation
                l1_disable_dsp_trace();
#endif
                // Reset the emergency flag
                l1a_l1s_com.melody1_e2_task.parameters.emergency_stop = FALSE;

                // Initialize the buffer parameters
                l1a_l1s_com.melody1_e2_task.parameters.ptr_buf        = NULL;
                l1a_l1s_com.melody1_e2_task.parameters.buffer_size    = 0;
                l1a_l1s_com.melody1_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody1_e2_task.parameters.session_id);

                // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
                l1a_l1s_com.melody1_e2_task.parameters.buffer_size <<= 1;

                // Jump the NumberOfOscillator parameter
                l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

                // Read the Header of the melody description in order to download the time factor
                // clean the MSB of the global time factor register
                l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
                l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
                l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


                // Find the number of insturment of the melody (jump the header memory)
                l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument));

                for (i=0; i<(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument); i++)
                {
                  // find the beginning of the melody description (after the header field)
                  // and put it in the buf_ptr buffer
                  l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody1_e2_task.parameters.waves_table_id[i]));
                }

                // Initialize the size (in byte unit) of the header
                l1a_l1s_com.melody1_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument;

                // download the instrument
                l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 1);

                // Send the start confirmation message
                l1a_audio_send_confirmation(MMI_MELODY1_E2_START_CON);

                *state = M1_WAIT_LOAD_INSTRUMENT;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"E21 SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }
            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if((SignalCode == API_L1_DYN_DWNLD_FINISHED) && (l1a.dyn_dwnld.semaphore_vect[E2_STATE_MACHINE] == GREEN))
            {
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
              // Disable trace DSP upon E2 MELODY activation
              l1_disable_dsp_trace();
#endif
              // Reset the emergency flag
              l1a_l1s_com.melody1_e2_task.parameters.emergency_stop = FALSE;

              // Initialize the buffer parameters
              l1a_l1s_com.melody1_e2_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody1_e2_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody1_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody1_e2_task.parameters.session_id);

              // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
              l1a_l1s_com.melody1_e2_task.parameters.buffer_size <<= 1;

              // Jump the NumberOfOscillator parameter
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

              // Read the Header of the melody description in order to download the time factor
              // clean the MSB of the global time factor register
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


              // Find the number of insturment of the melody (jump the header memory)
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument));

              for (i=0; i<(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument); i++)
              {
                // find the beginning of the melody description (after the header field)
                // and put it in the buf_ptr buffer
                l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody1_e2_task.parameters.waves_table_id[i]));
              }

              // Initialize the size (in byte unit) of the header
              l1a_l1s_com.melody1_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument;

              // download the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 1);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY1_E2_START_CON);

              *state = M1_WAIT_LOAD_INSTRUMENT;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"E21 SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
          break;
          case M1_WAIT_LOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 1 )
              {
                // The load instrument confirmation message is for the melody 1
                if (l1a_l1s_com.melody1_e2_task.parameters.emergency_stop)
                {
                  // Unload the instrument
                  l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 1);

                  *state = M1_WAIT_UNLOAD_INSTRUMENT;

                }
                else
                {
                  // Start to play the melody1
                  l1a_l1s_com.melody1_e2_task.command.start = TRUE;

                  *state = M1_WAIT_STOP;
                }
              }
            }
            else
            if (SignalCode == MMI_MELODY1_E2_STOP_REQ)
            {
              // Set the emergency flag
              l1a_l1s_com.melody1_e2_task.parameters.emergency_stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M1_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY1_E2_STOP_CON)
            {
              // Unload the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 1);

              *state = M1_WAIT_UNLOAD_INSTRUMENT;
            }
            else
            if (SignalCode == MMI_MELODY1_E2_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody1_e2_task.command.stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M1_WAIT_UNLOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 1 )
              {
                // Send the stop confirmation message
                l1a_audio_send_confirmation(MMI_MELODY1_E2_STOP_CON);

                *state = M1_RESET;
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon E2 MELODY deactivation
                l1_enable_dsp_trace();
#endif
              }
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }

#else // L1_DYN_DSP_DWNLD = 0

    /*-------------------------------------------------------*/
    /* l1a_mmi_melody0_e2_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 0 format E2 feature.                           */
    /*                                                       */
    /* Starting messages:        MMI_MELODY0_E2_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_MELODY0_E2_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_MELODY0_E2_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY0_E2_STOP_REQ     */
    /*                           L1_MELODY0_E2_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY0_E2_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/

    void l1a_mmi_melody0_e2_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M0_RESET                  = 0,
        M0_WAIT_START_REQ         = 1,
        M0_WAIT_LOAD_INSTRUMENT   = 2,
        M0_WAIT_STOP              = 3,
        M0_WAIT_UNLOAD_INSTRUMENT = 4
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY0_E2_STATE], i;
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case M0_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody0_e2_task.command.start = FALSE;
            l1a_l1s_com.melody0_e2_task.command.stop  = FALSE;

            *state = M0_WAIT_START_REQ;
          }
          break;

          case M0_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY0_E2_START_REQ)
            {
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
              // Disable trace DSP upon E2 MELODY activation
              l1_disable_dsp_trace();
#endif
              // Download the parameters from the message:
              l1a_l1s_com.melody0_e2_task.parameters.session_id = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody0_e2_task.parameters.loopback   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

              // Reset the emergency flag
              l1a_l1s_com.melody0_e2_task.parameters.emergency_stop = FALSE;

              // Initialize the buffer parameters
              l1a_l1s_com.melody0_e2_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody0_e2_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody0_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody0_e2_task.parameters.session_id);

              // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
              l1a_l1s_com.melody0_e2_task.parameters.buffer_size <<= 1;

              // Jump the NumberOfOscillator parameter
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

              // Read the Header of the melody description in order to download the time factor
              // clean the MSB of the global time factor register
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


              // Find the number of insturment of the melody (jump the header memory)
              l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument));

              for (i=0; i<(l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument); i++)
              {
                // find the beginning of the melody description (after the header field)
                // and put it in the buf_ptr buffer
                l1a_l1s_com.melody0_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody0_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody0_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody0_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody0_e2_task.parameters.waves_table_id[i]));
              }

              // Initialize the size (in byte unit) of the header
              l1a_l1s_com.melody0_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody0_e2_task.parameters.number_of_instrument;

              // download the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 0);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY0_E2_START_CON);

              *state = M0_WAIT_LOAD_INSTRUMENT;
            }

            // End process
            return;
          }
          break;

          case M0_WAIT_LOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 0 )
              {
                // The load instrument confirmation message is for the melody 0
                if (l1a_l1s_com.melody0_e2_task.parameters.emergency_stop)
                {
                  // Unload the instrument
                  l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 0);

                  *state = M0_WAIT_UNLOAD_INSTRUMENT;

                }
                else
                {
                  // Start to play the melody0
                  l1a_l1s_com.melody0_e2_task.command.start = TRUE;

                  *state = M0_WAIT_STOP;
                }
              }
            }
            else
            if (SignalCode == MMI_MELODY0_E2_STOP_REQ)
            {
              // Set the emergency flag
              l1a_l1s_com.melody0_e2_task.parameters.emergency_stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M0_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY0_E2_STOP_CON)
            {
              // Unload the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 0);

              *state = M0_WAIT_UNLOAD_INSTRUMENT;
            }
            else
            if (SignalCode == MMI_MELODY0_E2_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody0_e2_task.command.stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M0_WAIT_UNLOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 0 )
              {
                // Send the stop confirmation message
                l1a_audio_send_confirmation(MMI_MELODY0_E2_STOP_CON);

                *state = M0_RESET;
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon E2 MELODY deactivation
                l1_enable_dsp_trace();
#endif
              }
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }

    /*-------------------------------------------------------*/
    /* l1a_mmi_melody1_e2_process()                          */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* melody 1 format E2 feature.                           */
    /*                                                       */
    /* Starting messages:        MMI_MELODY1_E2_START_REQ    */
    /*                                                       */
    /* Result messages (input):  L1_MELODY1_E2_START_CON     */
    /*                                                       */
    /* Result messages (output): MMI_MELODY1_E2_START_CON    */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_MELDOY0_E2_STOP_REQ     */
    /*                           L1_MELODY1_E2_STOP_CON      */
    /*                                                       */
    /* Stop message (output):    MMI_MELODY1_E2_STOP_CON     */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_melody1_e2_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        M1_RESET                  = 0,
        M1_WAIT_START_REQ         = 1,
        M1_WAIT_LOAD_INSTRUMENT   = 2,
        M1_WAIT_STOP              = 3,
        M1_WAIT_UNLOAD_INSTRUMENT = 4
      };

      UWORD8    *state      = &l1a.state[L1A_MELODY1_E2_STATE], i;
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case M1_RESET:
          {
            // Reset the commands:
            l1a_l1s_com.melody1_e2_task.command.start = FALSE;
            l1a_l1s_com.melody1_e2_task.command.stop  = FALSE;

            *state = M1_WAIT_START_REQ;
          }
          break;

          case M1_WAIT_START_REQ:
          {
            if (SignalCode == MMI_MELODY1_E2_START_REQ)
            {
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
            // Disable trace DSP upon E2 MELODY activation
            l1_disable_dsp_trace();
#endif

              // Download the parameters from the message:
              l1a_l1s_com.melody1_e2_task.parameters.session_id = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->session_id;
              l1a_l1s_com.melody1_e2_task.parameters.loopback   = ((T_MMI_MELODY_E2_REQ *)(msg->SigP))->loopback;

              // Reset the emergency flag
              l1a_l1s_com.melody1_e2_task.parameters.emergency_stop = FALSE;

              // Initialize the buffer parameters
              l1a_l1s_com.melody1_e2_task.parameters.ptr_buf        = NULL;
              l1a_l1s_com.melody1_e2_task.parameters.buffer_size    = 0;
              l1a_l1s_com.melody1_e2_task.parameters.error_id = Cust_get_pointer((UWORD16 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                 &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                 l1a_l1s_com.melody1_e2_task.parameters.session_id);

              // Convert the buffer size in bytes unit because the E2 melody is defined in byte unit
              l1a_l1s_com.melody1_e2_task.parameters.buffer_size <<= 1;

              // Jump the NumberOfOscillator parameter
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));

              // Read the Header of the melody description in order to download the time factor
              // clean the MSB of the global time factor register
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor &= 0x00FF;
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   (UWORD8 *)(&(i)));
              l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor = i;


              // Find the number of insturment of the melody (jump the header memory)
              l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                   &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                   (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                   1,
                                                                                   &(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument));

              for (i=0; i<(l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument); i++)
              {
                // find the beginning of the melody description (after the header field)
                // and put it in the buf_ptr buffer
                l1a_l1s_com.melody1_e2_task.parameters.error_id = copy_byte_data_from_buffer (l1a_l1s_com.melody1_e2_task.parameters.session_id,
                                                                                     &l1a_l1s_com.melody1_e2_task.parameters.buffer_size,
                                                                                     (UWORD8 **)&l1a_l1s_com.melody1_e2_task.parameters.ptr_buf,
                                                                                     1,
                                                                                     &(l1a_l1s_com.melody1_e2_task.parameters.waves_table_id[i]));
              }

              // Initialize the size (in byte unit) of the header
              l1a_l1s_com.melody1_e2_task.parameters.header_size = 3 + l1a_l1s_com.melody1_e2_task.parameters.number_of_instrument;

              // download the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ, 1);

              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_MELODY1_E2_START_CON);

              *state = M1_WAIT_LOAD_INSTRUMENT;
            }

            // End process
            return;
          }
          break;

          case M1_WAIT_LOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 1 )
              {
                // The load instrument confirmation message is for the melody 1
                if (l1a_l1s_com.melody1_e2_task.parameters.emergency_stop)
                {
                  // Unload the instrument
                  l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 1);

                  *state = M1_WAIT_UNLOAD_INSTRUMENT;

                }
                else
                {
                  // Start to play the melody1
                  l1a_l1s_com.melody1_e2_task.command.start = TRUE;

                  *state = M1_WAIT_STOP;
                }
              }
            }
            else
            if (SignalCode == MMI_MELODY1_E2_STOP_REQ)
            {
              // Set the emergency flag
              l1a_l1s_com.melody1_e2_task.parameters.emergency_stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M1_WAIT_STOP:
          {
            if (SignalCode == L1_MELODY1_E2_STOP_CON)
            {
              // Unload the instrument
              l1_send_melody_e2_background_msg(L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ, 1);

              *state = M1_WAIT_UNLOAD_INSTRUMENT;
            }
            else
            if (SignalCode == MMI_MELODY1_E2_STOP_REQ)
            {
              // Stop the melody 0 L1S task:
              l1a_l1s_com.melody1_e2_task.command.stop = TRUE;
            }

            // End process
            return;
          }
          break;

          case M1_WAIT_UNLOAD_INSTRUMENT:
          {
            if (SignalCode == L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON)
            {
              if ( ((T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ *)(msg->SigP))->melody_id == 1 )
              {
                // Send the stop confirmation message
                l1a_audio_send_confirmation(MMI_MELODY1_E2_STOP_CON);

                *state = M1_RESET;
#if ((TRACE_TYPE==1) || (TRACE_TYPE == 4))
                // Enable trace DSP upon E2 MELODY deactivation
                l1_enable_dsp_trace();
#endif
              }
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // L1_DYN_DSP_DWNLD
  #endif // MELODY_E2

  #if (L1_CPORT == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_cport_process()                               */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* cport configuration feature.                          */
    /*                                                       */
    /* Starting messages:        MMI_CPORT_CONFIGURE_REQ     */
    /*                                                       */
    /* Result messages (input):  L1_CPORT_CONFIGURE_CON      */
    /*                                                       */
    /* Result messages (output): MMI_CPORT_CONFIGURE_CON     */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_cport_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2
      };

      UWORD8    *state      = &l1a.state[L1A_CPORT_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.cport_task.command.start = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_CPORT_CONFIGURE_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.cport_task.parameters.configuration = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->configuration;
              l1a_l1s_com.cport_task.parameters.cpcfr1        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr1;
              l1a_l1s_com.cport_task.parameters.cpcfr2        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr2;
              l1a_l1s_com.cport_task.parameters.cpcfr3        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr3;
              l1a_l1s_com.cport_task.parameters.cpcfr4        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpcfr4;
              l1a_l1s_com.cport_task.parameters.cptctl        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptctl;
              l1a_l1s_com.cport_task.parameters.cptdat        = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptdat;
              l1a_l1s_com.cport_task.parameters.cpttaddr      = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cpttaddr;
              l1a_l1s_com.cport_task.parameters.cptvs         = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->cptvs;
              l1a_l1s_com.cport_task.parameters.ctrl          = ((T_MMI_CPORT_CONFIGURE_REQ *)(msg->SigP))->ctrl;

              // Start the L1S cport task
              l1a_l1s_com.cport_task.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_CPORT_CONFIGURE_CON)
            {
              // Forward the stop confirmation message
              l1a_audio_send_result(MMI_CPORT_CONFIGURE_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;

        } // switch
      } // while(1)
    }
  #endif // L1_CPORT == 1

  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_audio_onoff_process()                         */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* audio on/off feature.                                 */
    /*                                                       */
    /* Starting messages:        MMI_AUDIO_ONOFF_REQ         */
    /*                                                       */
    /* Result messages (input):  L1_AUDIO_ONOFF_CON          */
    /*                                                       */
    /* Result messages (output): MMI_AUDIO_ONOFF_CON         */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_audio_onoff_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AUDIO_ONOFF_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.audio_onoff_task.command.start = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_AUDIO_ONOFF_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.audio_onoff_task.parameters.onoff_value = ((T_MMI_AUDIO_ONOFF_REQ *)(msg->SigP))->onoff_value;

              // Start the L1S keybeep task
              l1a_l1s_com.audio_onoff_task.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_AUDIO_ONOFF_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_AUDIO_ONOFF_CON);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
  #endif

  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_audio_onoff_process()                         */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* audio on/off feature.                                 */
    /*                                                       */
    /* Starting messages:        MMI_AUDIO_ONOFF_REQ         */
    /*                                                       */
    /* Result messages (input):  L1_AUDIO_ONOFF_CON          */
    /*                                                       */
    /* Result messages (output): MMI_AUDIO_ONOFF_CON         */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_audio_onoff_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AUDIO_ONOFF_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.audio_onoff_task.command.start = FALSE;

            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_AUDIO_ONOFF_REQ)
            {
              // Download the parameters from the message:
              l1a_l1s_com.audio_onoff_task.parameters.vul_onoff_value = ((T_MMI_AUDIO_ONOFF_REQ *)(msg->SigP))->vul_onoff_value;
              l1a_l1s_com.audio_onoff_task.parameters.vdl_onoff_value = ((T_MMI_AUDIO_ONOFF_REQ *)(msg->SigP))->vdl_onoff_value;
              // Start the L1S keybeep task
              l1a_l1s_com.audio_onoff_task.command.start = TRUE;

              *state = WAIT_START_CON;
            }

            // End process
            return;
          }
 // omaps00090550        break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_AUDIO_ONOFF_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_AUDIO_ONOFF_CON);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550      break;
        } // switch
      } // while(1)
    }
  #endif


  #if (L1_EXT_AUDIO_MGT == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_ext_audio_mgt_process()                       */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* external audio management feature.                    */
    /*                                                       */
    /* Starting messages:        MMI_EXT_AUDIO_MGT_START_REQ */
    /*                                                       */
    /* Result messages (input):  L1_STEREOPATH_DRV_START_CON */
    /*                                                       */
    /* Result messages (output): MMI_EXT_AUDIO_MGT_START_CON */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     MMI_EXT_AUDIO_MGT_STOP_REQ  */
    /*                           L1_STEREOPATH_DRV_STOP_CON  */
    /*                                                       */
    /* Stop message (output):    MMI_EXT_AUDIO_MGT_STOP_CON  */
    /*                                                       */
    /* Rem:                                                  */
    /* ----                                                  */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_ext_audio_mgt_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_START_REQ    = 1,
        WAIT_START_CON    = 2,
        WAIT_STOP         = 3
      };

      UWORD8    *state      = &l1a.state[L1A_EXT_AUDIO_MGT_STATE];
      UWORD32   SignalCode  = msg->SignalCode;
      UWORD8 sample_rate;
      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
#if(L1_BT_AUDIO ==1)
	      midi_task_running=FALSE;
#endif
            *state = WAIT_START_REQ;
          }
          break;

          case WAIT_START_REQ:
          {
            if (SignalCode == MMI_EXT_AUDIO_MGT_START_REQ)
            {
              // save global variable
              l1s.ext_audio_mgt.session_id = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->session_id;
		#if(L1_BT_AUDIO ==1)
		  l1_audio_bt_init(AUDIO_EXT_MIDI_BUFFER_SIZE);
		  if(bt_audio.connected_status==TRUE)
            	  {
			bt_audio.pcmconfig.bitsPerSample=16;
			bt_audio.pcmconfig.numChannels =((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->mono_stereo+1;
	              sample_rate   =((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->sampling_frequency;
			bt_audio.pcmconfig.sampleRate   =l1_ext_audio_get_frequencyrate(sample_rate);
			bt_audio.audio_configure_callback(&bt_audio.pcmconfig);
            	  }	
		 #endif
	if(bt_audio.connected_status==FALSE)
     	  {
              // Download the stereopath description.
              l1a_l1s_com.stereopath_drv_task.parameters.sampling_frequency    = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->sampling_frequency;
              l1a_l1s_com.stereopath_drv_task.parameters.DMA_allocation        = AUDIO_SP_DMA_ALLOC_MCU;
              l1a_l1s_com.stereopath_drv_task.parameters.DMA_int_callback_fct  = l1_ext_audio_mgt_dma_handler;
              l1a_l1s_com.stereopath_drv_task.parameters.DMA_channel_number    = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->DMA_channel_number;
              l1a_l1s_com.stereopath_drv_task.parameters.data_type             = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->data_type;
              l1a_l1s_com.stereopath_drv_task.parameters.source_port           = AUDIO_SP_SOURCE_IMIF;
              l1a_l1s_com.stereopath_drv_task.parameters.source_buffer_address = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->source_buffer_address;
              l1a_l1s_com.stereopath_drv_task.parameters.element_number        = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->element_number;
              l1a_l1s_com.stereopath_drv_task.parameters.frame_number          = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->frame_number;
              l1a_l1s_com.stereopath_drv_task.parameters.mono_stereo           = ((T_MMI_EXT_AUDIO_MGT_START_REQ *)(msg->SigP))->mono_stereo;
              l1a_l1s_com.stereopath_drv_task.parameters.feature_identifier    = AUDIO_SP_EXT_AUDIO_ID;

              // Start the L1S stereopath task
              l1a_l1s_com.stereopath_drv_task.command.start = TRUE;
#if(L1_BT_AUDIO ==1)
              midi_task_running=TRUE;
#endif

              *state = WAIT_START_CON;
            }
	   else
	  {
 		  l1a_audio_send_confirmation(MMI_EXT_AUDIO_MGT_START_CON);

              *state = WAIT_STOP;
	   }
            // End process
            return;
          }
          }
 // omaps00090550         break;

          case WAIT_START_CON:
          {
            if (SignalCode == L1_STEREOPATH_DRV_START_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_EXT_AUDIO_MGT_START_CON);

              *state = WAIT_STOP;
            }

            // End process
            return;
          }
 // omaps00090550        break;

          case WAIT_STOP:
          {
            if (SignalCode == MMI_EXT_AUDIO_MGT_STOP_REQ)
            {
		if(bt_audio.connected_status==TRUE)
     	  	{

	       // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_EXT_AUDIO_MGT_STOP_CON);
	        midi_task_running=FALSE;
	       *state = RESET;
		}
		else
		{
              // Stop the L1S stereopath task
              l1a_l1s_com.stereopath_drv_task.command.stop = TRUE;

              // End process
              return;
            }
            }

            if (SignalCode == L1_STEREOPATH_DRV_STOP_CON)
            {

              // Send the stop confirmation message
              l1a_audio_send_confirmation(MMI_EXT_AUDIO_MGT_STOP_CON);
	      #if(L1_BT_AUDIO ==1)
              midi_task_running=FALSE;
	     #endif
              *state = RESET;
            }
            else
            {
              // End process
              return;
            }
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // EXT_AUDIO_MGT == 1

  #if (L1_ANR == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_anr_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* ANR feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_ANR_REQ                 */
    /*                                                       */
    /* Result messages (input):  L1_ANR_CON                  */
    /*                                                       */
    /* Result messages (output): MMI_ANR_CON                 */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_anr_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_ANR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.anr_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_ANR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.anr_task.parameters.anr_enable       = ((T_MMI_ANR_REQ *)(msg->SigP))->anr_enable;
              l1a_l1s_com.anr_task.parameters.min_gain         = ((T_MMI_ANR_REQ *)(msg->SigP))->min_gain;
              l1a_l1s_com.anr_task.parameters.div_factor_shift = ((T_MMI_ANR_REQ *)(msg->SigP))->div_factor_shift;
              l1a_l1s_com.anr_task.parameters.ns_level         = ((T_MMI_ANR_REQ *)(msg->SigP))->ns_level;

              if (l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.anr_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ANR SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
          break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.anr_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ANR SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_ANR_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_ANR_CON);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_anr_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_ANR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.anr_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_ANR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.anr_task.parameters.anr_enable       = ((T_MMI_ANR_REQ *)(msg->SigP))->anr_enable;
              l1a_l1s_com.anr_task.parameters.min_gain         = ((T_MMI_ANR_REQ *)(msg->SigP))->min_gain;
              l1a_l1s_com.anr_task.parameters.div_factor_shift = ((T_MMI_ANR_REQ *)(msg->SigP))->div_factor_shift;
              l1a_l1s_com.anr_task.parameters.ns_level         = ((T_MMI_ANR_REQ *)(msg->SigP))->ns_level;

              // Enable the L1S task
              l1a_l1s_com.anr_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_ANR_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_ANR_CON);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD
  #endif // L1_ANR
#if (L1_ANR == 2) // ANR 2.13
    /*-------------------------------------------------------*/
    /* l1a_mmi_anr_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* ANR 2.13 feature.                                     */
    /*                                                       */
    /* Starting messages:        MMI_AQI_ANR_REQ             */
    /*                                                       */
    /* Result messages (input):  L1_AQI_ANR_CON              */
    /*                                                       */
    /* Result messages (output): MMI_AQI_ANR_CON             */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_anr_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_ANR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.anr_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_ANR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.anr_task.parameters.anr_ul_control     = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->anr_ul_control;
              l1a_l1s_com.anr_task.parameters.control            = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.anr_task.parameters.ns_level           = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.ns_level;
              l1a_l1s_com.anr_task.parameters.tone_ene_th        = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_ene_th;
              l1a_l1s_com.anr_task.parameters.tone_cnt_th        = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_cnt_th;

              if (l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.anr_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ANR SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550           break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.anr_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ANR SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550        break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_ANR_CON)
            {
              // Send the confirmation message
              l1a_audio_send_result(MMI_AQI_ANR_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_anr_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_ANR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.anr_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_ANR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.anr_task.parameters.anr_ul_control   = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->anr_ul_control;
              l1a_l1s_com.anr_task.parameters.control          = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.anr_task.parameters.ns_level         = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.ns_level;
              l1a_l1s_com.anr_task.parameters.tone_ene_th      = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_ene_th;
              l1a_l1s_com.anr_task.parameters.tone_cnt_th      = ((T_MMI_AQI_ANR_REQ *)(msg->SigP))->parameters.tone_cnt_th;

              // Enable the L1S task
              l1a_l1s_com.anr_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_ANR_CON)
            {
              // Send the confirmation message
              l1a_audio_send_result(MMI_AQI_ANR_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD
#endif // L1_ANR

  #if (L1_IIR == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_iir_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* IIR feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_IIR_REQ                 */
    /*                                                       */
    /* Result messages (input):  L1_IIR_CON                  */
    /*                                                       */
    /* Result messages (output): MMI_IIR_CON                 */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_iir_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_IIR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.iir_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_IIR_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.iir_task.parameters.iir_enable          = ((T_MMI_IIR_REQ *)(msg->SigP))->iir_enable;
              l1a_l1s_com.iir_task.parameters.nb_iir_blocks       = ((T_MMI_IIR_REQ *)(msg->SigP))->nb_iir_blocks;
              l1a_l1s_com.iir_task.parameters.iir_coefs           = ((T_MMI_IIR_REQ *)(msg->SigP))->iir_coefs;
              l1a_l1s_com.iir_task.parameters.nb_fir_coefs        = ((T_MMI_IIR_REQ *)(msg->SigP))->nb_fir_coefs;
              l1a_l1s_com.iir_task.parameters.fir_coefs           = ((T_MMI_IIR_REQ *)(msg->SigP))->fir_coefs;
              l1a_l1s_com.iir_task.parameters.input_scaling       = ((T_MMI_IIR_REQ *)(msg->SigP))->input_scaling;
              l1a_l1s_com.iir_task.parameters.fir_scaling         = ((T_MMI_IIR_REQ *)(msg->SigP))->fir_scaling;
              l1a_l1s_com.iir_task.parameters.input_gain_scaling  = ((T_MMI_IIR_REQ *)(msg->SigP))->input_gain_scaling;
              l1a_l1s_com.iir_task.parameters.output_gain_scaling = ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain_scaling;
              l1a_l1s_com.iir_task.parameters.output_gain         = ((T_MMI_IIR_REQ *)(msg->SigP))->output_gain;
              l1a_l1s_com.iir_task.parameters.feedback            = ((T_MMI_IIR_REQ *)(msg->SigP))->feedback;

              // Enable the L1S task
              l1a_l1s_com.iir_task.command.update = TRUE;

              *state = WAIT_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_CON:
          {
            if (SignalCode == L1_IIR_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_IIR_CON);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
  #endif // L1_IIR

 #if (L1_AGC_UL == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_agc_ul_process()                              */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* AGC UL feature.                                       */
    /*                                                       */
    /* Starting messages:        MMI_AQI_AGC_UL_REQ          */
    /*                                                       */
    /* Result messages (input):  L1_AQI_AGC_UL_CON           */
    /*                                                       */
    /* Result messages (output): MMI_AQI_AGC_UL_CON          */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_agc_ul_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_AGC_UL_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.agc_ul_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_AGC_UL_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.agc_ul_task.parameters.agc_ul_control         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->agc_ul_control;
              l1a_l1s_com.agc_ul_task.parameters.control                = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.agc_ul_task.parameters.frame_size             = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.frame_size;
              l1a_l1s_com.agc_ul_task.parameters.targeted_level         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.targeted_level;
              l1a_l1s_com.agc_ul_task.parameters.signal_up              = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_up;
              l1a_l1s_com.agc_ul_task.parameters.signal_down            = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_down;
              l1a_l1s_com.agc_ul_task.parameters.max_scale              = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.max_scale;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha      = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha_fast = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta       = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta_fast  = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast;
              l1a_l1s_com.agc_ul_task.parameters.gain_intp_flag         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_intp_flag;


              if (l1a.dyn_dwnld.semaphore_vect[AGC_UL_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.agc_ul_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AGC_UL SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550          break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[AGC_UL_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.agc_ul_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AGC_UL SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550        break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_AGC_UL_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_AGC_UL_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_agc_ul_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AGC_UL_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.agc_ul_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_AGC_UL_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.agc_ul_task.parameters.agc_ul_control         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->agc_ul_control;
              l1a_l1s_com.agc_ul_task.parameters.control                = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.agc_ul_task.parameters.frame_size             = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.frame_size;
              l1a_l1s_com.agc_ul_task.parameters.targeted_level         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.targeted_level;
              l1a_l1s_com.agc_ul_task.parameters.signal_up              = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_up;
              l1a_l1s_com.agc_ul_task.parameters.signal_down            = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.signal_down;
              l1a_l1s_com.agc_ul_task.parameters.max_scale              = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.max_scale;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha      = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_alpha_fast = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta       = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta;
              l1a_l1s_com.agc_ul_task.parameters.gain_smooth_beta_fast  = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast;
              l1a_l1s_com.agc_ul_task.parameters.gain_intp_flag         = ((T_MMI_AQI_AGC_UL_REQ *)(msg->SigP))->parameters.gain_intp_flag;

              // Enable the L1S task
              l1a_l1s_com.agc_ul_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_AGC_UL_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_AGC_UL_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD

#endif // L1_AGC_UL


#if (L1_AGC_DL == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_agc_dl_process()                              */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* AGC DL feature.                                       */
    /*                                                       */
    /* Starting messages:        MMI_AQI_AGC_DL_REQ          */
    /*                                                       */
    /* Result messages (input):  L1_AQI_AGC_DL_CON           */
    /*                                                       */
    /* Result messages (output): MMI_AQI_AGC_DL_CON          */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_agc_dl_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_AGC_DL_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.agc_dl_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_AGC_DL_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.agc_dl_task.parameters.agc_dl_control         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->agc_dl_control;
              l1a_l1s_com.agc_dl_task.parameters.control                = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.agc_dl_task.parameters.frame_size             = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.frame_size;
              l1a_l1s_com.agc_dl_task.parameters.targeted_level         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.targeted_level;
              l1a_l1s_com.agc_dl_task.parameters.signal_up              = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_up;
              l1a_l1s_com.agc_dl_task.parameters.signal_down            = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_down;
              l1a_l1s_com.agc_dl_task.parameters.max_scale              = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.max_scale;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha      = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha_fast = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta       = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta_fast  = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast;
              l1a_l1s_com.agc_dl_task.parameters.gain_intp_flag         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_intp_flag;


              if (l1a.dyn_dwnld.semaphore_vect[AGC_DL_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.agc_dl_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AGC_DL SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550           break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[AGC_DL_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.agc_dl_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"AGC_DL SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_AGC_DL_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_AGC_DL_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550           break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_agc_dl_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_AGC_DL_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.agc_dl_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_AGC_DL_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.agc_dl_task.parameters.agc_dl_control         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->agc_dl_control;
              l1a_l1s_com.agc_dl_task.parameters.control                = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.control;
              l1a_l1s_com.agc_dl_task.parameters.frame_size             = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.frame_size;
              l1a_l1s_com.agc_dl_task.parameters.targeted_level         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.targeted_level;
              l1a_l1s_com.agc_dl_task.parameters.signal_up              = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_up;
              l1a_l1s_com.agc_dl_task.parameters.signal_down            = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.signal_down;
              l1a_l1s_com.agc_dl_task.parameters.max_scale              = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.max_scale;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha      = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_alpha_fast = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_alpha_fast;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta       = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta;
              l1a_l1s_com.agc_dl_task.parameters.gain_smooth_beta_fast  = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_smooth_beta_fast;
              l1a_l1s_com.agc_dl_task.parameters.gain_intp_flag         = ((T_MMI_AQI_AGC_DL_REQ *)(msg->SigP))->parameters.gain_intp_flag;

              // Enable the L1S task
              l1a_l1s_com.agc_dl_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_AGC_DL_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_AGC_DL_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD

#endif // L1_AGC_DL


  #if (L1_IIR == 2)
    /*-------------------------------------------------------*/
    /* l1a_mmi_iir_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* IIR feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_AQI_IIR_DL_REQ             */
    /*                                                       */
    /* Result messages (input):  L1_AQI_IIR_DL_CON              */
    /*                                                       */
    /* Result messages (output): MMI_AQI_IIR_DL_CON             */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_iir_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_IIR_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.iir_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_IIR_DL_REQ)
            {
	          l1a.iir_req_msg_ptr = msg;
              l1a.l1_msg_forwarded = TRUE;

              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.iir_task.parameters = (T_MMI_AQI_IIR_DL_REQ *) (msg->SigP);

              // Enable the L1S task
              l1a_l1s_com.iir_task.command.update = TRUE;

              *state = WAIT_CON;
            }

            // End process
            return;
          }
 // omaps00090550           break;

          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_IIR_DL_CON)
            {
              os_free_sig(l1a.iir_req_msg_ptr);
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_IIR_DL_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550         break;
        } // switch
      } // while(1)
    }
  #endif // L1_IIR

  #if (L1_WCM == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_wcm_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* WCM feature.                                          */
    /*                                                       */
    /* Starting messages:        MMI_AQI_WCM_REQ             */
    /*                                                       */
    /* Result messages (input):  L1_AQI_WCM_CON              */
    /*                                                       */
    /* Result messages (output): MMI_AQI_WCM_CON             */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/

#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_wcm_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_WCM_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.wcm_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_WCM_REQ)
            {
	          l1a.wcm_req_msg_ptr = msg;
      	      l1a.l1_msg_forwarded = TRUE;

              // Load the message into the l1a_l1s_com memory
              l1a_l1s_com.wcm_task.parameters = (T_MMI_AQI_WCM_REQ *) (msg->SigP);

              if (l1a.dyn_dwnld.semaphore_vect[WCM_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.wcm_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"WCM SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550          break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[WCM_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.wcm_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"WCM SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_WCM_CON)
            {
              os_free_sig(l1a.wcm_req_msg_ptr);
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_WCM_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_wcm_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_WCM_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.wcm_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_WCM_REQ)
            {
	          l1a.wcm_req_msg_ptr = msg;
      	      l1a.l1_msg_forwarded = TRUE;

              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.wcm_task.parameters = (T_MMI_AQI_WCM_REQ *) (msg->SigP);

              // Enable the L1S task
              l1a_l1s_com.wcm_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_WCM_CON)
            {
              os_free_sig(l1a.wcm_req_msg_ptr);
              // Send the start confirmation message
              l1a_audio_send_result(MMI_AQI_WCM_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD

#endif // L1_WCM

#if (L1_DRC == 1) // DRC 1.x
// DRC NDB API
T_DRC_MCU_DSP *drc_ndb;
#if (CODE_VERSION == SIMULATION)
  T_DRC_MCU_DSP drc_ndb_sim;
#endif

    /*-------------------------------------------------------*/
    /* l1a_mmi_drc_process()                                 */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* DRC 1.x feature.                                     */
    /*                                                       */
    /* Starting messages:        MMI_AQI_DRC_REQ             */
    /*                                                       */
    /* Result messages (input):  L1_AQI_DRC_CON              */
    /*                                                       */
    /* Result messages (output): MMI_AQI_DRC_CON             */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if(L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_drc_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD    = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_DRC_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.drc_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_DRC_REQ)
            {
	          l1a.drc_req_msg_ptr = msg;
       	      l1a.l1_msg_forwarded = TRUE;
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.drc_task.parameters = (T_MMI_AQI_DRC_REQ *) (msg->SigP);

              if (l1a.dyn_dwnld.semaphore_vect[DRC_STATE_MACHINE] == GREEN)
              {
                // WARNING: the following code must be copied in the state WAIT_DYN_DWNLD
                // when activating the task at L1s level

                // Enable the L1S task
                l1a_l1s_com.drc_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"DRC SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550          break;
          case WAIT_DYN_DWNLD:
          {
            if(SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[DRC_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.drc_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"DRC SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_DRC_CON)
            {
              os_free_sig(l1a.drc_req_msg_ptr);
              // Send the confirmation message
              l1a_audio_send_result(MMI_AQI_DRC_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550          break;
        } // switch
      } // while(1)
    }
#else
    void l1a_mmi_drc_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_DRC_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.drc_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_AQI_DRC_REQ)
            {
	          l1a.drc_req_msg_ptr = msg;
       	      l1a.l1_msg_forwarded = TRUE;
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.drc_task.parameters = (T_MMI_AQI_DRC_REQ *) (msg->SigP);

              // Enable the L1S task
              l1a_l1s_com.drc_task.command.update = TRUE;

              *state = WAIT_CON;
            }
            // End process
            return;
          }
          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_AQI_DRC_CON)
            {
              os_free_sig(l1a.drc_req_msg_ptr);

              // Send the confirmation message
              l1a_audio_send_result(MMI_AQI_DRC_CON, msg, MMI_QUEUE);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
#endif // L1_DYN_DSP_DWNLD
#endif // L1_DRC

  #if (L1_LIMITER == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_limiter_process()                             */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* LIMITER feature.                                      */
    /*                                                       */
    /* Starting messages:        MMI_LIMITER_REQ             */
    /*                                                       */
    /* Result messages (input):  L1_LIMITER_CON              */
    /*                                                       */
    /* Result messages (output): MMI_LIMITER_CON             */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
    void l1a_mmi_limiter_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_LIMITER_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the commands
            l1a_l1s_com.limiter_task.command.update         = FALSE;
            l1a_l1s_com.limiter_task.command.partial_update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_LIMITER_REQ)
            {
              // Load the message into the l1a_l1s_com memory.
              l1a_l1s_com.limiter_task.parameters.limiter_enable      = ((T_MMI_LIMITER_REQ *)(msg->SigP))->limiter_enable;
              l1a_l1s_com.limiter_task.parameters.block_size          = ((T_MMI_LIMITER_REQ *)(msg->SigP))->block_size;
              l1a_l1s_com.limiter_task.parameters.slope_update_period = ((T_MMI_LIMITER_REQ *)(msg->SigP))->slope_update_period;
              l1a_l1s_com.limiter_task.parameters.nb_fir_coefs        = ((T_MMI_LIMITER_REQ *)(msg->SigP))->nb_fir_coefs;
              l1a_l1s_com.limiter_task.parameters.filter_coefs        = ((T_MMI_LIMITER_REQ *)(msg->SigP))->filter_coefs;
              l1a_l1s_com.limiter_task.parameters.thr_low_0           = ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_0;
              l1a_l1s_com.limiter_task.parameters.thr_low_slope       = ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_low_slope;
              l1a_l1s_com.limiter_task.parameters.thr_high_0          = ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_0;
              l1a_l1s_com.limiter_task.parameters.thr_high_slope      = ((T_MMI_LIMITER_REQ *)(msg->SigP))->thr_high_slope;
              l1a_l1s_com.limiter_task.parameters.gain_fall           = ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_fall;
              l1a_l1s_com.limiter_task.parameters.gain_rise           = ((T_MMI_LIMITER_REQ *)(msg->SigP))->gain_rise;

              // Enable the L1S task
              l1a_l1s_com.limiter_task.command.update = TRUE;

              *state = WAIT_CON;
            }

            // End process
            return;
          }
 // omaps00090550           break;

          case WAIT_CON:
          {
            if (SignalCode == L1_LIMITER_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_LIMITER_CON);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550         break;
        } // switch
      } // while(1)
    }
  #endif // L1_LIMITER

  #if (L1_ES == 1)
    /*-------------------------------------------------------*/
    /* l1a_mmi_es_process()                                  */
    /*-------------------------------------------------------*/
    /*                                                       */
    /* Description:                                          */
    /* ------------                                          */
    /* This function is a state machine which handles the    */
    /* Echo Suppressor feature.                              */
    /*                                                       */
    /* Starting messages:        MMI_ES_REQ                  */
    /*                                                       */
    /* Result messages (input):  L1_ES_CON                   */
    /*                                                       */
    /* Result messages (output): MMI_ES_CON                  */
    /*                                                       */
    /* Reset messages (input):   none                        */
    /*                                                       */
    /* Stop message (input):     none                        */
    /*                                                       */
    /* Stop message (output):    none                        */
    /*                                                       */
    /*-------------------------------------------------------*/
#if (L1_DYN_DSP_DWNLD == 1)
    void l1a_mmi_es_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_DYN_DWNLD = 2,
        WAIT_CON          = 3
      };

      UWORD8    *state      = &l1a.state[L1A_ES_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.es_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_ES_REQ)
            {
              // Load the message into the l1a_l1s_com memory
              l1a_l1s_com.es_task.parameters.es_enable     = ((T_MMI_ES_REQ *)(msg->SigP))->es_enable;
              l1a_l1s_com.es_task.parameters.es_behavior   = ((T_MMI_ES_REQ *)(msg->SigP))->es_behavior;

              if (l1a_l1s_com.es_task.parameters.es_behavior == ES_CUSTOM_PARAM)
              {
                // Load every parameters from the message
                l1a_l1s_com.es_task.parameters.es_config.es_mode               = ((T_MMI_ES_REQ *)(msg->SigP))->es_mode;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_dl            = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_dl;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_ul_1          = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_1;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_ul_2          = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_2;
                l1a_l1s_com.es_task.parameters.es_config.tcl_fe_ls_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ls_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_dt_ls_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ls_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_fe_ns_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ns_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_dt_ns_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ns_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_ne_thr            = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_ne_thr;
                l1a_l1s_com.es_task.parameters.es_config.ref_ls_pwr            = ((T_MMI_ES_REQ *)(msg->SigP))->ref_ls_pwr;
                l1a_l1s_com.es_task.parameters.es_config.switching_time        = ((T_MMI_ES_REQ *)(msg->SigP))->switching_time;
                l1a_l1s_com.es_task.parameters.es_config.switching_time_dt     = ((T_MMI_ES_REQ *)(msg->SigP))->switching_time_dt;
                l1a_l1s_com.es_task.parameters.es_config.hang_time             = ((T_MMI_ES_REQ *)(msg->SigP))->hang_time;
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[0]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[0];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[1]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[1];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[2]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[2];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[3]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[3];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[0]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[0];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[1]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[1];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[2]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[2];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[3]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[3];
              }
              if (l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
              {
                // WARNING: The following code must be copied in the state WAIT_DYN_DWNLD
                // when ES task is activated at L1s level

                // Enable the L1S task
                l1a_l1s_com.es_task.command.update = TRUE;

                *state = WAIT_CON;
              }
              else
              {
                *state = WAIT_DYN_DWNLD;

                #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                  if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ES SM blocked by DYN DWNLD\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
                #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
              }
            }

            // End process
            return;
          }
 // omaps00090550           break;
          case WAIT_DYN_DWNLD:
          {
            if (SignalCode == API_L1_DYN_DWNLD_FINISHED && l1a.dyn_dwnld.semaphore_vect[ANR_STATE_MACHINE] == GREEN)
            {
              // Enable the L1S task
              l1a_l1s_com.es_task.command.update = TRUE;

              *state = WAIT_CON;

              #if ((TRACE_TYPE == 1) || (TRACE_TYPE == 4) || (TRACE_TYPE == 5))
                if((trace_info.current_config->l1_dyn_trace) & (1<<L1_DYN_TRACE_DYN_DWNLD))
                  {
                    char str[30];
                    sprintf(str,"ES SM un-blocked\r\n");
                    #if(CODE_VERSION == SIMULATION)
                      trace_fct_simu_dyn_dwnld(str);
                    #else
                      rvt_send_trace_cpy((T_RVT_BUFFER)str,trace_info.l1_trace_user_id,strlen(str),RVT_ASCII_FORMAT);
                    #endif
                  }
               #endif    // (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
            }
            return;
          }
 // omaps00090550          break;
          case WAIT_CON:
          {
            if (SignalCode == L1_ES_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_ES_CON);

              *state = RESET;
            }

            // End process
            return;
          }
 // omaps00090550          break;
        } // switch
      } // while(1)
    }

#else
    void l1a_mmi_es_process(xSignalHeaderRec *msg)
    {
      enum states
      {
        RESET             = 0,
        WAIT_REQ          = 1,
        WAIT_CON          = 2
      };

      UWORD8    *state      = &l1a.state[L1A_ES_STATE];
      UWORD32   SignalCode  = msg->SignalCode;

      while(1)
      {
        switch(*state)
        {
          case RESET:
          {
            // Reset the command
            l1a_l1s_com.es_task.command.update = FALSE;

            *state = WAIT_REQ;
          }
          break;

          case WAIT_REQ:
          {
            if (SignalCode == MMI_ES_REQ)
            {
              // Load the message into the l1a_l1s_com memory
              l1a_l1s_com.es_task.parameters.es_enable     = ((T_MMI_ES_REQ *)(msg->SigP))->es_enable;
              l1a_l1s_com.es_task.parameters.es_behavior   = ((T_MMI_ES_REQ *)(msg->SigP))->es_behavior;

              if (l1a_l1s_com.es_task.parameters.es_behavior == ES_CUSTOM_PARAM)
              {
                // Load every parameters from the message
                l1a_l1s_com.es_task.parameters.es_config.es_mode               = ((T_MMI_ES_REQ *)(msg->SigP))->es_mode;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_dl            = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_dl;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_ul_1          = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_1;
                l1a_l1s_com.es_task.parameters.es_config.es_gain_ul_2          = ((T_MMI_ES_REQ *)(msg->SigP))->es_gain_ul_2;
                l1a_l1s_com.es_task.parameters.es_config.tcl_fe_ls_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ls_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_dt_ls_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ls_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_fe_ns_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_fe_ns_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_dt_ns_thr         = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_dt_ns_thr;
                l1a_l1s_com.es_task.parameters.es_config.tcl_ne_thr            = ((T_MMI_ES_REQ *)(msg->SigP))->tcl_ne_thr;
                l1a_l1s_com.es_task.parameters.es_config.ref_ls_pwr            = ((T_MMI_ES_REQ *)(msg->SigP))->ref_ls_pwr;
                l1a_l1s_com.es_task.parameters.es_config.switching_time        = ((T_MMI_ES_REQ *)(msg->SigP))->switching_time;
                l1a_l1s_com.es_task.parameters.es_config.switching_time_dt     = ((T_MMI_ES_REQ *)(msg->SigP))->switching_time_dt;
                l1a_l1s_com.es_task.parameters.es_config.hang_time             = ((T_MMI_ES_REQ *)(msg->SigP))->hang_time;
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[0]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[0];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[1]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[1];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[2]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[2];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_dl_vect[3]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_dl_vect[3];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[0]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[0];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[1]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[1];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[2]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[2];
                l1a_l1s_com.es_task.parameters.es_config.gain_lin_ul_vect[3]   = ((T_MMI_ES_REQ *)(msg->SigP))->gain_lin_ul_vect[3];
              }

              // Enable the L1S task
              l1a_l1s_com.es_task.command.update = TRUE;

              *state = WAIT_CON;
            }

            // End process
            return;
          }
          break;

          case WAIT_CON:
          {
            if (SignalCode == L1_ES_CON)
            {
              // Send the start confirmation message
              l1a_audio_send_confirmation(MMI_ES_CON);

              *state = RESET;
            }

            // End process
            return;
          }
          break;
        } // switch
      } // while(1)
    }
  #endif //L1_DYN_DSP_DWNLD
  #endif // L1_ES

// New Vocoder IF process

#if (L1_VOCODER_IF_CHANGE == 1)
  void l1a_mmi_vocoder_cfg_process    (xSignalHeaderRec *msg)
  {

    BOOL msg_parameter;

    enum states
    {
      WAIT_REQ = 0,
      WAIT_ENABLE_CON = 1,
      WAIT_DISABLE_CON = 2
    };

    enum vocoder_cfg_req
    {
      VOCODER_DISABLE_REQ  = 0,
      VOCODER_ENABLE_REQ  = 1
    };

    UWORD8    *state      = &l1a.state[L1A_VOCODER_CFG_STATE];
    UWORD32   SignalCode  = msg->SignalCode;

    while (1)
    {
      switch(*state)
      {
        case WAIT_REQ:
        {

          // Waiting PS messages....

          // If SignalCode is an MPHC_STOP_DEDICATED_REQ and vocoders are enabled, they must be stopped automatically

#if(L1_CHECK_COMPATIBLE == 1)
          if (((SignalCode == MPHC_STOP_DEDICATED_REQ) && (l1a.vocoder_state.enabled == TRUE)) || (l1a.vch_auto_disable == TRUE))
#else
          if (((SignalCode == MPHC_STOP_DEDICATED_REQ) && (l1a.vocoder_state.enabled == TRUE)))          
#endif		 	
          {
             // Command vocoder disabling at L1A-L1s interface and set the automatic disable flag to TRUE
             l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_DISABLE_COMMAND;
             l1a.vocoder_state.automatic_disable = TRUE;
             *state = WAIT_DISABLE_CON;
#if(L1_CHECK_COMPATIBLE == 1)			
  	      l1a.vch_auto_disable = FALSE;
#endif
          }
          else if (SignalCode == MMI_TCH_VOCODER_CFG_REQ)
          {
            // In case the vocoder START or STOP is commanded by PS

          
            msg_parameter = ((T_MMI_TCH_VOCODER_CFG_REQ *) (msg->SigP))->vocoder_state;
            // If it is an explicit STOP....
            if (msg_parameter == VOCODER_DISABLE_REQ)
            {
#if (AUDIO_DEBUG == 1)
              trace_info.audio_debug_var.vocoder_enable_status = 0;
#endif

              //... explicitely disable vocoders if they're enabled commanidng the stop at L1A-L1s interface
              if (l1a.vocoder_state.enabled == TRUE)
              {
                 l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_DISABLE_COMMAND;
                 l1a.vocoder_state.automatic_disable = FALSE;
                 *state = WAIT_DISABLE_CON;
              }
              else
              {
                // ...else, if vocoders are already disabled but PS is sending erroneously the request
                // send him however the confirmation without doing anything (protection check).
                // In case L1 standalone or SIMULATION, confirmation message must be
                // sent to MMI mailbox, else with complete PS software the message must be
                // addressed to the ACI queue (WARNING: ACI_QUEUE must be properly defined).

                l1a_send_confirmation(MMI_TCH_VOCODER_CFG_CON, ACI_QUEUE);
              }
            }
            else if (msg_parameter == VOCODER_ENABLE_REQ)
            {
#if (AUDIO_DEBUG == 1)
              trace_info.audio_debug_var.vocoder_enable_status = 1;
#endif
              // If it is a START
              if (l1a.vocoder_state.enabled == FALSE)
              {
                // Command the start at L1A-L1s interface if vocoders are disabled
                l1a_l1s_com.dedic_set.start_vocoder = TCH_VOCODER_ENABLE_COMMAND;
                *state = WAIT_ENABLE_CON;
              }
              else
              {
                // ...else, if vocoders are already enabled but PS is sending erroneously the request
                // send him however the confirmation without doing anything (protection check).
                // In case L1 standalone or SIMULATION, confirmation message must be
                // sent to MMI mailbox, else with complete PS software the message must be
                // addressed to the ACI queue (WARNING: ACI_QUEUE must be properly defined).

                l1a_send_confirmation(MMI_TCH_VOCODER_CFG_CON, ACI_QUEUE);
              }
            }
          }
        return;
        }
 // omaps00090550        break;
        case WAIT_ENABLE_CON:
        {
          if (SignalCode == L1_VOCODER_CFG_ENABLE_CON)
          {
            // when L1s confirms the enabling, forward the confirmation to PS and set the vocoders enabled flag to TRUE
            l1a.vocoder_state.enabled = TRUE;

            // in case L1 standalone or SIMULATION, confirmation message must be
            // sent to MMI mailbox, else with complete PS software the message must be
            // addressed to the ACI queue (WARNING: ACI_QUEUE must be properly defined).

            l1a_send_confirmation(MMI_TCH_VOCODER_CFG_CON, ACI_QUEUE);
            *state = WAIT_REQ;
          }
          return;
        }
 // omaps00090550        break;
        case WAIT_DISABLE_CON:
        {
          if (SignalCode == L1_VOCODER_CFG_DISABLE_CON)
          {
            // when L1s confirms the disabling, forwards the confirmation to PS and set the vocoders enabled flag to FALSE
            l1a.vocoder_state.enabled = FALSE;
            *state = WAIT_REQ;

            if (l1a.vocoder_state.automatic_disable == FALSE)
            {
              // Only in case it is an explicit request to STOP the vocoders (made previously by PS)
              // send him the confirmation.
              // In case L1 standalone or SIMULATION, confirmation message must be
              // sent to MMI mailbox, else with complete PS software the message must be
              // addressed to the ACI queue (WARNING: ACI_QUEUE must be properly defined).

              l1a_send_confirmation(MMI_TCH_VOCODER_CFG_CON, ACI_QUEUE);
            }
            else
            	// Reset automatic disable flag
            	l1a.vocoder_state.automatic_disable = FALSE;
          }
        return;
        }
      // omaps00090550 break;
      }
    }

  }
#endif // L1_VOCODER_IF_CHANGE == 1
  void l1a_mmi_outen_cfg_process    (xSignalHeaderRec *msg)
  {
#if (OP_RIV_AUDIO == 1)

    UWORD32 SignalCode = msg->SignalCode;
    void              *p_message;
    T_RVF_MB_STATUS   mb_status;

    if (SignalCode == MMI_OUTEN_CFG_REQ)
    {
      l1a_l1s_com.outen_cfg_task.outen1 = ((T_MMI_OUTEN_CFG_REQ *)(msg->SigP))->outen1;
      l1a_l1s_com.outen_cfg_task.outen2 = ((T_MMI_OUTEN_CFG_REQ *)(msg->SigP))->outen2;
      l1a_l1s_com.outen_cfg_task.outen3 = ((T_MMI_OUTEN_CFG_REQ *)(msg->SigP))->outen3;
      l1a_l1s_com.outen_cfg_task.classD  = ((T_MMI_OUTEN_CFG_REQ *)(msg->SigP))->classD;

      l1a_l1s_com.outen_cfg_task.command_requested++;
      // Send the confirmation message
      l1a_audio_send_confirmation(MMI_OUTEN_CFG_CON);
     }
     else if(SignalCode == MMI_OUTEN_CFG_READ_REQ)
     {
        // Allocate the Riviera buffer
        mb_status = rvf_get_buf ( p_audio_gbl_var->mb_external,
                                  sizeof (T_MMI_OUTEN_CFG_READ_CON),
                                  (T_RVF_BUFFER **) (&p_message));

        // If insufficient resources, then report a memory error and abort.
        if (mb_status == RVF_RED)
        {
          // the memory is insufficient to continue the non regression test
          AUDIO_SEND_TRACE("AUDIO entity has no memory for L1 confirm",RV_TRACE_LEVEL_ERROR);
          return;
        }

        // Fill the message ID
        ((T_MMI_OUTEN_CFG_READ_CON *)(p_message))->header.msg_id = MMI_OUTEN_CFG_READ_CON;

        // Fill the message parameter
        ((T_MMI_OUTEN_CFG_READ_CON*)(p_message))->outen1 = l1a_l1s_com.outen_cfg_task.outen1;
        ((T_MMI_OUTEN_CFG_READ_CON*)(p_message))->outen2 = l1a_l1s_com.outen_cfg_task.outen2;
        ((T_MMI_OUTEN_CFG_READ_CON*)(p_message))->outen3 = l1a_l1s_com.outen_cfg_task.outen3;
        ((T_MMI_OUTEN_CFG_READ_CON*)(p_message))->classD  = l1a_l1s_com.outen_cfg_task.classD;

          // send the messsage to the audio entity
          rvf_send_msg (p_audio_gbl_var->addrId,p_message);
     }
#endif /*OP_RIV_AUDIO*/
  } /* end function l1a_mmi_outen_cfg_process */

#if(L1_BT_AUDIO ==1)
void l1a_mmi_bt_process(xSignalHeaderRec *msg)
{             
     UWORD32 SignalCode = msg->SignalCode;
	 
     if (SignalCode == MMI_BT_ENABLE_REQ)
           bt_audio.connected_status   = TRUE;
     else if(SignalCode == MMI_BT_DISABLE_REQ)
	    bt_audio.connected_status  = FALSE;

}
#endif
#endif // AUDIO_TASK


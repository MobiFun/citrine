/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_INIT.C
 *
 *        Filename l1audio_init.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Include files...                 */
/************************************/

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"


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
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
    #include "l1audio_varex.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_AAC == 1)
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
  #else // NOT SIMULATION

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
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
    #include "l1audio_varex.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "../../gpf/inc/cust_os.h"
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
  #endif // NOT_SIMULATION

#if (L1_DRC == 1)
  extern T_DRC_MCU_DSP *drc_ndb;
  #if (CODE_VERSION == SIMULATION)
    extern T_DRC_MCU_DSP drc_ndb_sim;
  #endif
#endif

#if(L1_BT_AUDIO ==1)
  extern T_L1_BT_AUDIO bt_audio;
#endif

/*
 * FreeCalypso hack: the version of l1_confg.h in the Leonardo semi-src
 * sets AUDIO_TASK to 1 unconditionally, thus it appears that by the
 * time TCS211 came around, TI stopped supporting and testing the
 * sans-AUDIO_TASK configuration.  We do wish to support it in FreeCalypso
 * though.  Attempting to compile this module w/o AUDIO_TASK failed
 * because some preprocessor constant definitions were missing.
 * All 3 offending constants are defined in l1audio_const.h, but only
 * when AUDIO_TASK is enabled.  The following hack is our workaround.
 */
#if !AUDIO_TASK
  #define C_BGD_RECOGN  5
  #define C_BGD_ALIGN   6
  #define NO_MELODY_SELECTED    (0)
#endif

  /**************************************/
  /* Prototypes for L1 initialization   */
  /**************************************/
  void l1audio_dsp_init        (void);
  void l1audio_initialize_var  (void);

  /**************************************/
  /* External prototypes                */
  /**************************************/

  /*-------------------------------------------------------*/
  /* l1audio_dsp_init()                                    */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : Initialize the part of the API          */
  /*               dedicated to the audio task.            */
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1audio_dsp_init(void)
  {
    UWORD8  i, j;

    //-----------------------------------
    // AUDIO control words initialization
    //-----------------------------------
    l1s_dsp_com.dsp_ndb_ptr->d_toneskb_init     = 0;             // MCU/DSP audio task com. register
    l1s_dsp_com.dsp_ndb_ptr->d_toneskb_status   = 0;             // MCU/DSP audio task com. register

    #if (KEYBEEP)
      l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt0       = 0;             // keybeep variable
      l1s_dsp_com.dsp_ndb_ptr->d_k_x1_kt1       = 0;             // keybeep variable
      l1s_dsp_com.dsp_ndb_ptr->d_dur_kb         = 0;             // keybeep variable
    #endif

    #if ((TONE) || (VOICE_MEMO))
      l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t0        = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t1        = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_k_x1_t2        = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_pe_rep         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_pe_off         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_se_off         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_bu_off         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t0_on          = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t0_off         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t1_on          = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t1_off         = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t2_on          = 0;             // tone variable
      l1s_dsp_com.dsp_ndb_ptr->d_t2_off         = 0;             // tone variable

      l1s_dsp_com.dsp_ndb_ptr->d_shiftul        = 0x100;
      l1s_dsp_com.dsp_ndb_ptr->d_shiftdl        = 0x100;
    #endif // (TONE) || (VOICE_MEMO)
    #if (L1_PCM_EXTRACTION)
      l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_upload         = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_download       = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_pcm_api_error          = 0;
    #endif

  // Correction of PR G23M/L1_MCU-SPR-15494
  #if ((CHIPSET == 12) || (CHIPSET == 15))
    #if (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
      l1s_dsp_com.dsp_ndb_ptr->d_cport_init      = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_cport_ctrl      = 0;
      l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[0]    = 0;
      l1s_dsp_com.dsp_ndb_ptr->a_cport_cfr[1]    = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_cport_tcl_tadt  = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_cport_tdat      = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_cport_tvs       = 0;
    #endif
  #endif

    #if (L1_VOICE_MEMO_AMR)
      l1s_dsp_com.dsp_ndb_ptr->d_shiftul        = 0x100;
    #endif // L1_VOICE_MEMO_AMR

    #if (MELODY_E1)
      l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_used    = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_melo_osc_active  = 0;

      l1s_dsp_com.dsp_ndb_ptr->a_melo_note0[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note1[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note2[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note3[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note4[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note5[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note6[0]    = SC_END_OSCILLATOR_MASK;
      l1s_dsp_com.dsp_ndb_ptr->a_melo_note7[0]    = SC_END_OSCILLATOR_MASK;
    #endif // MELODY_E1

    // Initialize the FIR as an all band pass
    // IMPORTANT NOTE: FIR/DL parameters are also initialized for DSP 36 when L1_IIR == 1 because
    // in FIR loop mode, the old FIR API is still used
    #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)// The FIR coefficents are in param memory
      l1s_dsp_com.dsp_param_ptr->a_fir31_downlink[0] = 0x4000;
      l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[0]   = 0x4000;
    #else
      l1s_dsp_com.dsp_ndb_ptr->a_fir31_downlink[0]   = 0x4000;
      l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[0]     = 0x4000;
    #endif

    for (i=1; i<MAX_FIR_COEF; i++)
    {
      #if (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)// The FIR coefficents are in param memory
        l1s_dsp_com.dsp_param_ptr->a_fir31_downlink[i]  = 0;
        l1s_dsp_com.dsp_param_ptr->a_fir31_uplink[i]    = 0;
      #else
        l1s_dsp_com.dsp_ndb_ptr->a_fir31_downlink[i]    = 0;
        l1s_dsp_com.dsp_ndb_ptr->a_fir31_uplink[i]      = 0;
      #endif
    }
    #if (DSP == 17) || (DSP == 32)
      // start the FIR task
      l1s_dsp_com.dsp_ndb_ptr->d_audio_init |= B_FIR_START;
    #endif

    #if (L1_IIR == 1)
      // IIR enabled by default
      // Set the default configuration (all band pass - FIR only mode)
      l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_iir_blocks = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_fir_coefs  = 0x1f;

      l1s_dsp_com.dsp_ndb_ptr->a_iir_fir_coefs[0]  = 0x4000;
      for (i=1; i < (l1s_dsp_com.dsp_ndb_ptr->d_iir_nb_fir_coefs - 1); i++)
        l1s_dsp_com.dsp_ndb_ptr->a_iir_fir_coefs[i] = 0;
      
      l1s_dsp_com.dsp_ndb_ptr->d_iir_input_scaling       = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_fir_scaling         = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_input_gain_scaling  = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain_scaling = 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_output_gain         = 0xffff;
      l1s_dsp_com.dsp_ndb_ptr->d_iir_feedback            = 0;
    #endif

    #if (AUDIO_MODE)
      // Reset the FIR loopback and the audio mode
      l1s_dsp_com.dsp_ndb_ptr->d_audio_init &= ~(B_FIR_LOOP | B_GSM_ONLY | B_BT_HEADSET | B_BT_CORDLESS);
      // Set the GSM mode
      l1s_dsp_com.dsp_ndb_ptr->d_audio_init |= B_GSM_ONLY;
    #else
      // Reset the loopback
      l1s_dsp_com.dsp_ndb_ptr->d_audio_init &= ~(B_FIR_LOOP);
    #endif

   #if (W_A_DSP_SR_BGD)
     // Initialize the DSP speech reco background task

     // DSP background enabled for SR.
     l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt = (B_DSPBGD_RECO | B_DSPBGD_UPD);
     l1s_dsp_com.dsp_ndb_ptr->d_max_background = 7;

     // TEMPORARY: Init DSP background interface for RECO.
     if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt & B_DSPBGD_RECO)
     {
       l1s_dsp_com.dsp_ndb_ptr->d_background_enable &= ~(1 << C_BGD_RECOGN);
       l1s_dsp_com.dsp_ndb_ptr->d_background_abort  &= ~(1 << C_BGD_RECOGN);
       l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[C_BGD_RECOGN] = (C_BGD_RECOGN<<11) | 1;
       l1s_dsp_com.dsp_ndb_ptr->a_back_task_io[C_BGD_RECOGN]     = (API)(0x0000); // Not used by Recognition task.
     }
     if (l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt & B_DSPBGD_UPD)
     {
       l1s_dsp_com.dsp_ndb_ptr->d_background_enable &= ~(1 << C_BGD_ALIGN);
       l1s_dsp_com.dsp_ndb_ptr->d_background_abort  &= ~(1 << C_BGD_ALIGN);
       l1s_dsp_com.dsp_ndb_ptr->a_background_tasks[C_BGD_ALIGN]  = (C_BGD_ALIGN<<11)  | 1;
       l1s_dsp_com.dsp_ndb_ptr->a_back_task_io[C_BGD_ALIGN]      = (API)(0x0000); // Not used by Alignement task.
     }
   #elif (DSP == 33) || (DSP == 34) || (DSP == 35) || (DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39)
     // DSP background task through pending task queue
     l1s_dsp_com.dsp_param_ptr->d_gsm_bgd_mgt = 0;
   #endif

    #if (MELODY_E2)
      // Initalize the Audio compressor used for E2
      l1s_dsp_com.dsp_ndb_ptr->d_audio_compressor_ctrl = 0x0401;

      // Initialize the melody E2 variables
      l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_osc_stop             = 0x0000;
      l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_osc_active           = 0x0000;
      l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_semaphore            = 0x0000;
      for(i=0; i<SC_MELODY_E2_NUMBER_OF_OSCILLATOR; i++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_osc[i][0] = 0x0000;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_globaltimefactor     = 0x0000;

      for (i=0; i<(SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT); i++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_ptr[i]  = 0x0000;
      }

      /* FreeCalypso: reconstructed from disassembly of TCS211 object */
      l1s_dsp_com.dsp_ndb_ptr->d_melody_e2_deltatime = 0;

      // Reset the flag to know if the DSP melody E2 task runs
      l1s.melody_e2.dsp_task = FALSE;
    #endif // MELODY_E2

    #if ((DSP==33) || (DSP == 34) || (DSP==35) || (DSP==36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
      // Linked to E2 melody
      // In case of WCP, there is a WCP variable at this address
      l1s_dsp_com.dsp_ndb_ptr->d_melody_selection = NO_MELODY_SELECTED;
    #endif


    #if ((CHIPSET == 4) || (CHIPSET == 12) || (CHIPSET == 15) || ((CHIPSET==10) && (OP_WCP==1))) && ((DSP == 36) || (DSP == 37) || (DSP == 38) || (DSP == 39))
      l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl            = 0;             // ES control
      l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl        = 0;             // ANR control
      #if (L1_IIR == 1)
        l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl    = B_IIR_ENABLE;  // IIR control: enabled by default
      #else
        l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl    = 0;
      #endif
      l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl      = 0;             // Limiter control

    #endif

#if (DSP == 38) || (DSP == 39)

    //-----------------------------------
    // AUDIO control words initialization
    //-----------------------------------

      l1s_dsp_com.dsp_ndb_ptr->d_es_ctrl            = 0;             // ES control
      l1s_dsp_com.dsp_ndb_ptr->d_anr_ul_ctrl        = 0;             // ANR control
      l1s_dsp_com.dsp_ndb_ptr->d_aec_ul_ctrl        = 0;             // AEC control
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_ctrl        = 0;             // AGC control

      #if (L1_IIR == 1)
        l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl      = B_IIR_ENABLE;  // IIR control: enabled by default
      #else
        l1s_dsp_com.dsp_ndb_ptr->d_iir_dl_ctrl      = 0;
      #endif
      l1s_dsp_com.dsp_ndb_ptr->d_lim_dl_ctrl        = 0;             // Limiter control
      l1s_dsp_com.dsp_ndb_ptr->d_drc_dl_ctrl        = 0;             // DRC control
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_ctrl        = 0;             // AGC control
      l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_ctrl    = 0;             // WCM control
      l1s_dsp_com.dsp_ndb_ptr->d_audio_apps_status  = 0;             // WCM status
      l1s_dsp_com.dsp_ndb_ptr->d_aqi_status         = 0;             // Initialise the status word

#if(L1_ANR == 2)
      l1s_dsp_com.dsp_ndb_ptr->d_anr_control          = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_anr_ns_level         = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_ene_th      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_anr_tone_cnt_th      = (API) 0;
#endif

#if(L1_IIR == 2)
      // Set IIR parameters
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_control         = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_frame_size      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_swap        = (API) 0;
      
      // Set parameter os FIR part
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_enable      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_length      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_fir_shift       = (API) 0;

      for (i=0; i < IIR_4X_FIR_MAX_LENGTH; i++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_fir_taps[i]   = (API) 0;
      }

      // Set parameters for IIR part
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_enable      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_number      = (API) 0;

      // Set parameters for IIR part - SOS 1 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_1      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_1 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_1[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_1[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_1  = (API) 0;

      // Set parameters for IIR part - SOS 2 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_2      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_2 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_2[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_2[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_2  = (API) 0;

      // Set parameters for IIR part - SOS 3 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_3      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_3 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_3[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_3[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_3  = (API) 0;

      // Set parameters for IIR part - SOS 4 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_4      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_4 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_4[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_4[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_4  = (API) 0;

      // Set parameters for IIR part - SOS 5 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_5      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_5 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_5[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_5[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_5  = (API) 0;

      // Set parameters for IIR part - SOS 6 
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_6      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_fact_form_6 = (API) 0;

      for (j=0; j < IIR_4X_ORDER_OF_SECTION; j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_den_6[j]  = (API) 0;
      }
      for (j=0; j <  (IIR_4X_ORDER_OF_SECTION + 1); j++)
      {
        l1s_dsp_com.dsp_ndb_ptr->a_iir4x_sos_num_6[j]  = (API) 0;
      }
      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_sos_num_form_6  = (API) 0;

      l1s_dsp_com.dsp_ndb_ptr->d_iir4x_gain            = (API) 0;

#endif

#if(L1_AGC_UL == 1)
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_control                = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_frame_size             = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_targeted_level         = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_up              = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_signal_down            = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_max_scale              = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_alpha_fast = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta       = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_smooth_beta_fast  = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_ul_gain_intp_flag         = (API) 0;
#endif

#if(L1_AGC_DL == 1)
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_control                = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_frame_size             = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_targeted_level         = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_up              = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_signal_down            = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_max_scale              = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha      = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_alpha_fast = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta       = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_smooth_beta_fast  = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_agc_dl_gain_intp_flag         = (API) 0;
#endif

#if(L1_WCM == 1)

      l1s_dsp_com.dsp_ndb_ptr->d_wcm_mode             = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_wcm_frame_size       = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_wcm_num_sub_frames   = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_wcm_ratio            = (API) 0;
      l1s_dsp_com.dsp_ndb_ptr->d_wcm_threshold        = (API) 0;
#endif

#endif // DSP 38

}


#if (AUDIO_TASK == 1)

  /*-------------------------------------------------------*/
  /* l1audio_initialize_var()                              */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : Initialize the part of l1a, l1s and     */
  /*               l1a_l1s_com dedicated to the audio task.*/
  /*                                                       */
  /*-------------------------------------------------------*/
  void l1audio_initialize_var(void)
  {
    UWORD8 i, j;

    // Initialize the state of the L1S maanger...
    //--------------------------------------------
    for(i=0; i<NBR_AUDIO_MANAGER; i++)
    {
      l1s.audio_state[i] = 0;
    }

    #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
      l1a_l1s_com.audio_onoff_task.parameters.onoff_value = FALSE;
    #endif
    #if 0	/* FreeCalypso TCS211 reconstruction */
      l1a_l1s_com.audio_forced_by_l1s = FALSE;
    #endif

    #if (MELODY_E1)
      l1s.melody0.oscillator[0] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note0[0]);
      l1s.melody0.oscillator[1] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note1[0]);
      l1s.melody0.oscillator[2] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note2[0]);
      l1s.melody0.oscillator[3] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note3[0]);
      l1s.melody0.oscillator[4] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note4[0]);
      l1s.melody0.oscillator[5] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note5[0]);
      l1s.melody0.oscillator[6] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note6[0]);
      l1s.melody0.oscillator[7] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note7[0]);

      l1s.melody1.oscillator[0] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note0[0]);
      l1s.melody1.oscillator[1] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note1[0]);
      l1s.melody1.oscillator[2] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note2[0]);
      l1s.melody1.oscillator[3] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note3[0]);
      l1s.melody1.oscillator[4] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note4[0]);
      l1s.melody1.oscillator[5] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note5[0]);
      l1s.melody1.oscillator[6] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note6[0]);
      l1s.melody1.oscillator[7] = &(l1s_dsp_com.dsp_ndb_ptr->a_melo_note7[0]);
    #endif // MELODY_E1

    #if (MELODY_E2)
      // Initialization ofthe audio background melody E2 load insturment variable
        audioback_melody_e2.allowed_size =
          SC_AUDIO_MELODY_E2_MAX_SIZE_OF_INSTRUMENT;
        audioback_melody_e2.API_address =
        l1s_dsp_com.dsp_ndb_ptr->a_melody_e2_instrument_wave;

        for (i=0; i < SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT; i++)
        {
          audioback_melody_e2.number_of_user[i] = 0;
        }
    #endif // MELODY_E2

    #if (L1_STEREOPATH == 1)
      // Reset the stereopath L1S commands
      l1a_l1s_com.stereopath_drv_task.command.start = FALSE;
      l1a_l1s_com.stereopath_drv_task.command.stop  = FALSE;
    #endif

// Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)
    l1s.audio_on_off_ctl.l1_audio_switch_on_ul_request = 0;
    l1s.audio_on_off_ctl.l1_audio_switch_on_dl_request = 0;

    l1s.audio_on_off_ctl.l1_audio_ul_on2off_hold_time = 
        L1_AUDIO_ON2OFF_UL_HOLD_TIME;
    l1s.audio_on_off_ctl.l1_audio_dl_on2off_hold_time = 
        L1_AUDIO_ON2OFF_DL_HOLD_TIME;

    l1s.audio_on_off_ctl.l1_audio_ul_action = L1_AUDIO_NO_ACTION;
    l1s.audio_on_off_ctl.l1_audio_dl_action = L1_AUDIO_NO_ACTION;

    l1s.audio_on_off_ctl.l1_audio_ul_switched_on = FALSE;
    l1s.audio_on_off_ctl.l1_audio_dl_switched_on = FALSE;

    l1s.audio_on_off_ctl.l1_audio_ul_switched_off = TRUE;
    l1s.audio_on_off_ctl.l1_audio_dl_switched_off = TRUE;
#endif // L1_AUDIO_MCU_ONOFF 


#if (L1_DRC == 1)

  // init DRC NDB
  drc_ndb = (T_DRC_MCU_DSP *)API_address_dsp2mcu(C_DRC_API_BASE_ADDRESS);
  #if (CODE_VERSION == SIMULATION)
  {
    drc_ndb = &drc_ndb_sim;
  }
  #endif

  drc_ndb->d_drc_speech_mode_samp_f               =(API)0;
  drc_ndb->d_drc_num_subbands                     =(API)0;
  drc_ndb->d_drc_frame_len                        =(API)0;
  drc_ndb->d_drc_expansion_knee_fb_bs             =(API)0;
  drc_ndb->d_drc_expansion_knee_md_hg             =(API)0;
  drc_ndb->d_drc_expansion_ratio_fb_bs            =(API)0;
  drc_ndb->d_drc_expansion_ratio_md_hg            =(API)0;
  drc_ndb->d_drc_max_amplification_fb_bs          =(API)0;
  drc_ndb->d_drc_max_amplification_md_hg          =(API)0;
  drc_ndb->d_drc_compression_knee_fb_bs           =(API)0;
  drc_ndb->d_drc_compression_knee_md_hg           =(API)0;
  drc_ndb->d_drc_compression_ratio_fb_bs          =(API)0;
  drc_ndb->d_drc_compression_ratio_md_hg          =(API)0;
  drc_ndb->d_drc_energy_limiting_th_fb_bs         =(API)0;
  drc_ndb->d_drc_energy_limiting_th_md_hg         =(API)0;
  drc_ndb->d_drc_limiter_threshold_fb             =(API)0;
  drc_ndb->d_drc_limiter_threshold_bs             =(API)0;
  drc_ndb->d_drc_limiter_threshold_md             =(API)0;
  drc_ndb->d_drc_limiter_threshold_hg             =(API)0;
  drc_ndb->d_drc_limiter_hangover_spect_preserve  =(API)0;
  drc_ndb->d_drc_limiter_release_fb_bs            =(API)0;
  drc_ndb->d_drc_limiter_release_md_hg            =(API)0;
  drc_ndb->d_drc_gain_track_fb_bs                 =(API)0;
  drc_ndb->d_drc_gain_track_md_hg                 =(API)0;
  for (j=0; j < DRC_LPF_LENGTH; j++)
  {
    drc_ndb->a_drc_low_pass_filter[j]          = (API)0;
  }
  for (j=0; j < DRC_BPF_LENGTH; j++)
  {
    drc_ndb->a_drc_mid_band_filter[j]          = (API)0;
  }
#endif

}

#endif // AUDIO_TASK

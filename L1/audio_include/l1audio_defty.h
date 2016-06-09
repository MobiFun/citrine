/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_DEFTY.H
 *
 *        Filename l1audio_defty.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include "../../include/config.h"
#include "../include/l1_confg.h"
#include "l1audio_msgty.h"
#include "../audio_cust0/l1audio_cust.h"
#include "l1audio_const.h"
#include "l1audio_btapi.h"
#if (AUDIO_TASK == 1)

  #if (KEYBEEP)
    /**************************************************************************/
    /* Keybeep l1a_l1s_com structure...                                       */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_KEYBEEP_COMMAND;

    typedef struct
    {
      T_KEYBEEP_COMMAND   command;
    }
    T_KEYBEEP_TASK;
  #endif

  #if (TONE)
    /**************************************************************************/
    /* Tone l1a_l1s_com structure...                                          */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_TONE_COMMAND;

    typedef struct
    {
      T_TONE_COMMAND   command;
    }
    T_TONE_TASK;
  #endif

  #if (MELODY_E1)
    /**************************************************************************/
    /* Melody l1a_l1s_com structure...                                        */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_MELODY_COMMAND;

    typedef struct
    {
      UWORD8  session_id;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD16 *ptr_buf;
      BOOL    loopback;
      UWORD16 oscillator_used_bitmap;
      UWORD16 oscillator_melody_bitmap;
      UWORD8  melody_to_oscillator[SC_NUMBER_OSCILLATOR];
    }
    T_MELODY_PARAM;

    typedef struct
    {
      T_MELODY_COMMAND    command;
      T_MELODY_PARAM      parameters;
    }
    T_MELODY_TASK;

    /**************************************************************************/
    /* Melody l1s structure...                                                */
    /**************************************************************************/

    typedef struct
    {
      UWORD8      error_id;
      UWORD16     buffer_size;
      UWORD16     *ptr_buf;
      UWORD16     melody_header;
      API         *(oscillator[SC_NUMBER_OSCILLATOR]);
      UWORD16     counter;
      UWORD16     oscillator_used_in_desc;
      UWORD16     oscillator_started;
    }
    T_L1S_MELODY_TASK;
  #endif

  #if (VOICE_MEMO)
    /**************************************************************************/
    /* Voice memo l1a_l1s_com structure...                                    */
    /**************************************************************************/

    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_VM_PLAY_COMMAND;

    typedef struct
    {
      UWORD8 session_id;
    }
    T_VM_PLAY_PARAM;

    typedef struct
    {
      T_VM_PLAY_COMMAND   command;
      T_VM_PLAY_PARAM     parameters;
    }
    T_VM_PLAY_TASK;

    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_VM_RECORD_COMMAND;

    typedef struct
    {
      UWORD8    session_id;
      UWORD32   maximum_size;
      BOOL      dtx;
    }
    T_VM_RECORD_PARAM;

    typedef struct
    {
      BOOL      start;
    }
    T_VM_RECORD_TONE_UL;

      typedef struct
    {
        T_VM_RECORD_COMMAND   command;
        T_VM_RECORD_PARAM     parameters;
        T_VM_RECORD_TONE_UL   tone_ul;
    }
    T_VM_RECORD_TASK;

    typedef struct
    {
      T_VM_PLAY_TASK    play;
      T_VM_RECORD_TASK  record;
    }
    T_VM_TASK;

    /**************************************************************************/
    /* Voice memo l1s structure...                                            */
    /**************************************************************************/

    typedef struct
    {
      API     *a_du_x;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD16 *ptr_buf;
      UWORD32 recorded_size;
    }
    T_L1S_VM_RECORD_TASK;

    typedef struct
    {
      API     *a_du_x;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD16 *ptr_buf;
    }
    T_L1S_VM_PLAY_TASK;

    typedef struct
    {
      T_L1S_VM_PLAY_TASK    play;
      T_L1S_VM_RECORD_TASK  record;
    }
    T_L1S_VM_TASK;

  #endif

  #if (L1_PCM_EXTRACTION)
  /**************************************************************************/
  /* PCM extraction l1a_l1s_com structure...                                */
  /**************************************************************************/

    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_PCM_DOWNLOAD_COMMAND;

    typedef struct
    {
      UWORD8 session_id;
      UWORD32   maximum_size;
    }
    T_PCM_DOWNLOAD_PARAM;

    typedef struct
    {
      T_PCM_DOWNLOAD_COMMAND   command;
      T_PCM_DOWNLOAD_PARAM     parameters;
    }
    T_PCM_DOWNLOAD_TASK;

    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_PCM_UPLOAD_COMMAND;

    typedef struct
    {
      UWORD8    session_id;
      UWORD32   maximum_size;
    }
    T_PCM_UPLOAD_PARAM;

    typedef struct
    {
        T_PCM_UPLOAD_COMMAND   command;
        T_PCM_UPLOAD_PARAM     parameters;
    }
    T_PCM_UPLOAD_TASK;

    typedef struct
    {
      T_PCM_DOWNLOAD_TASK download;
      T_PCM_UPLOAD_TASK   upload;
    }
    T_PCM_TASK;

    /**************************************************************************/
    /* PCM l1s structure...                                                   */
    /**************************************************************************/

    typedef struct
    {
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD16 *ptr_buf;
      UWORD32 uploaded_size;
    }
    T_L1S_PCM_UPLOAD_TASK;

    typedef struct
    {
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD16 *ptr_buf;
      UWORD32 downloaded_size;
    }
    T_L1S_PCM_DOWNLOAD_TASK;

    typedef struct
    {
      T_L1S_PCM_DOWNLOAD_TASK download;
      T_L1S_PCM_UPLOAD_TASK   upload;
    }
    T_L1S_PCM_TASK;

  #endif   /* L1_PCM_EXTRACTION */

  #if (L1_VOICE_MEMO_AMR)
    /**************************************************************************/
    /* Voice memo amr l1a_l1s_com structure...                                */
    /**************************************************************************/

    typedef struct
    {
      BOOL  start;
      BOOL pause;
      BOOL resume;
      BOOL  stop;
    }
    T_VM_AMR_PLAY_COMMAND;

    typedef struct
    {
      UWORD8 session_id;
    }
    T_VM_AMR_PLAY_PARAM;

    typedef struct
    {
      T_VM_AMR_PLAY_COMMAND   command;
      T_VM_AMR_PLAY_PARAM     parameters;
    }
    T_VM_AMR_PLAY_TASK;

    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_VM_AMR_RECORD_COMMAND;

    typedef struct
    {
      UWORD8    session_id;
      UWORD32   maximum_size;
      UWORD8    amr_vocoder;
      BOOL      dtx;
    }
    T_VM_AMR_RECORD_PARAM;

    typedef struct
    {
      T_VM_AMR_RECORD_COMMAND command;
      T_VM_AMR_RECORD_PARAM   parameters;
    }
    T_VM_AMR_RECORD_TASK;

    typedef struct
    {
      T_VM_AMR_PLAY_TASK    play;
      T_VM_AMR_RECORD_TASK  record;
    }
    T_VM_AMR_TASK;

    /**************************************************************************/
    /* Voice memo l1s structure...                                            */
    /**************************************************************************/

    typedef struct
    {
      API     *a_du_x;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD8  *ptr_buf;
      UWORD32 recorded_size;
    }
    T_L1S_VM_AMR_RECORD_TASK;

    typedef struct
    {
      API     *a_du_x;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD8  *ptr_buf;
      UWORD8  previous_type;
      UWORD8  transition_header;
    }
    T_L1S_VM_AMR_PLAY_TASK;

    typedef struct
    {
      T_L1S_VM_AMR_PLAY_TASK    play;
      T_L1S_VM_AMR_RECORD_TASK  record;
    }
    T_L1S_VM_AMR_TASK;

  #endif // L1_VOICE_MEMO_AMR
  #if (SPEECH_RECO)
    /**************************************************************************/
    /* Speech recogniton l1a_l1s_com structure...                             */
    /**************************************************************************/

    typedef struct
    {
      BOOL enroll_start;
      BOOL enroll_stop;
      BOOL update_start;
      BOOL update_stop;
      BOOL reco_start;
      BOOL reco_stop;
      BOOL processing_start;
      BOOL processing_stop;
      BOOL speech_start;
      BOOL speech_stop;
    }
    T_SR_COMMAND;

    typedef struct
    {
      UWORD8  database_id;
      UWORD8  word_index;
      API     *model_address;
      UWORD16 *model_temp_address;
      BOOL    speech;
      UWORD16 *speech_address;
      UWORD16 *start_address;
      UWORD16 *stop_address;
      BOOL    CTO_algorithm;
      UWORD8  index_counter;
      UWORD8  vocabulary_size;
      UWORD8  word_to_check;
      UWORD16 best_word_index;
      UWORD32 best_word_score;
      UWORD16 second_best_word_index;
      UWORD32 second_best_word_score;
      UWORD16 third_best_word_index;
      UWORD32 third_best_word_score;
      UWORD16 fourth_best_word_index;
      UWORD32 fourth_best_word_score;
      UWORD16 d_sr_db_level;
      UWORD16 d_sr_db_noise;
      UWORD16 d_sr_model_size;
    }
    T_SR_PARAM;

    typedef struct
    {
      T_SR_COMMAND  command;
      T_SR_PARAM    parameters;
    }
    T_SR_TASK;

    typedef struct
    {
      UWORD16   time_out;
      UWORD8    error;
      UWORD16   *speech_pointer;
      UWORD16   *end_pointer;
      API       *a_du_x;
      UWORD16   speech_old_status;
      BOOL      first_pass;
    }
    T_L1S_SR_TASK;

    typedef struct
    {
      BOOL emergency_stop;
    }
    T_L1_SRBACK_COM;

  #endif

  #if (L1_AEC == 1)
    typedef struct
    {
      UWORD16 aec_control;
    #if (L1_NEW_AEC)
      UWORD16 cont_filter;
      UWORD16 granularity_att;
      UWORD16 coef_smooth;
      UWORD16 es_level_max;
      UWORD16 fact_vad;
      UWORD16 thrs_abs;
      UWORD16 fact_asd_fil;
      UWORD16 fact_asd_mut;
    #endif
    }
    T_AEC_PARAM;

    typedef struct
    {
      BOOL start;
    }
    T_AEC_COMMAND;

    typedef struct
    {
      T_AEC_COMMAND  command;
      T_AEC_PARAM    parameters;
    }
    T_AEC_TASK;

    typedef struct
    {
      UWORD16 aec_control;
    #if (L1_NEW_AEC)
      BOOL    aec_visibility;
      UWORD16 cont_filter;
      UWORD16 granularity_att;
      UWORD16 coef_smooth;
      UWORD16 es_level_max;
      UWORD16 fact_vad;
      UWORD16 thrs_abs;
      UWORD16 fact_asd_fil;
      UWORD16 fact_asd_mut;
      WORD8   visibility_interval;
    #endif
    } T_L1S_AEC_TASK;
  #endif

#if(L1_AEC == 2)

    typedef struct
     {
       BOOL start;
     }
     T_AEC_COMMAND;


	typedef struct
	{
	  WORD16 aec_mode;
	  WORD16 mu;
	  UWORD16 cont_filter;
	  WORD16 scale_input_ul;
	  WORD16 scale_input_dl;
	  WORD16 div_dmax;
	  UWORD16 div_swap_good;
	  UWORD16 div_swap_bad;
	  WORD16 block_init;
	  UWORD16 fact_vad;
	  UWORD16 fact_asd_fil;
	  UWORD16 fact_asd_mut;
	  UWORD16 thrs_abs;
	  UWORD16 es_level_max;
	  UWORD16 granularity_att;
	  UWORD16 coef_smooth;
//	  UWORD16 block_size;
	}
	T_AEC_PARAMS;


   typedef struct
   {
     T_AEC_COMMAND  command;
     T_AEC_CONTROL aec_control;
     T_AEC_PARAMS parameters;
   }
   T_AEC_TASK;


#endif


  #if (FIR)
    typedef struct
    {
      BOOL    fir_loop;
      UWORD8  update_fir;
      UWORD16 *fir_ul_coefficient;
      UWORD16 *fir_dl_coefficient;
    }
    T_FIR_PARAM;

    typedef struct
    {
      BOOL start;
    }
    T_FIR_COMMAND;

    typedef struct
    {
      T_FIR_COMMAND  command;
      T_FIR_PARAM    parameters;
    }
    T_FIR_TASK;
  #endif

  #if (AUDIO_MODE)
    typedef struct
    {
      UWORD16  audio_mode;
    }
    T_AUDIO_MODE_PARAM;

    typedef struct
    {
      BOOL start;
    }
    T_AUDIO_MODE_COMMAND;

    typedef struct
    {
      T_AUDIO_MODE_COMMAND command;
      T_AUDIO_MODE_PARAM   parameters;
    }
    T_AUDIO_MODE_TASK;
  #endif
  #if (MELODY_E2)
    /**************************************************************************/
    /* Melody format E2 l1a_l1s_com structure...                              */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_MELODY_E2_COMMAND;

    typedef struct
    {
      UWORD8  session_id;
      UWORD8  error_id;
      UWORD16 buffer_size;
      UWORD8  *ptr_buf;
      BOOL    loopback;
      UWORD8  header_size;
      BOOL    emergency_stop;
      UWORD8  number_of_instrument;
      UWORD8  waves_table_id[SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT];
    }
    T_MELODY_E2_PARAM;

    typedef struct
    {
      T_MELODY_E2_COMMAND    command;
      T_MELODY_E2_PARAM      parameters;
    }
    T_MELODY_E2_TASK;

    /**************************************************************************/
    /* Melody format E2 l1s structure...                                      */
    /**************************************************************************/
    typedef struct
    {
      UWORD8      error_id;
      UWORD16     buffer_size;
      UWORD8      *ptr_buf;
      UWORD32     counter;
      UWORD32     note_start_20ms;
      UWORD16     oscillator_start;
      UWORD16     oscillator_active;
      UWORD16     delta_time;
      BOOL        extension_flag;
      BOOL        end_of_file;
    }
    T_L1S_MELODY_E2_TASK;

    typedef struct
    {
      UWORD32     timebase;
      UWORD16     global_osc_active;
      UWORD16     global_osc_to_start;
      UWORD8      timebase_mod_60ms;
      BOOL        dsp_task;
    }
    T_L1S_MELODY_E2_COMMON_VAR;

    /**************************************************************************/
    /* Melody format E2 audio background structure...                         */
    /**************************************************************************/
    typedef struct
    {
      API     *API_address;
      UWORD16 allowed_size;
      UWORD8  number_of_user[SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT];
      UWORD8  instrument_id[SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT];
      UWORD16 instrument_size[SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT];
    } T_AUDIO_BACK_MELODY_E2;
  #endif // MELODY_E2
  #if (L1_CPORT == 1)
    /**************************************************************************/
    /* Cport l1a_l1s_com structure...                                         */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
    }
    T_CPORT_COMMAND;

    typedef struct
    {
     UWORD16 configuration;
     UWORD16 ctrl;
     UWORD8  cpcfr1;
     UWORD8  cpcfr2;
     UWORD8  cpcfr3;
     UWORD8  cpcfr4;
     UWORD8  cptctl;
     UWORD8  cpttaddr;
     UWORD16 cptdat;
     UWORD16 cptvs; 
    }
    T_CPORT_PARAM;

    typedef struct
    {
      T_CPORT_COMMAND   command;
      T_CPORT_PARAM     parameters;
    }
    T_CPORT_TASK;
  #endif

  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1 || L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    typedef struct
    {
      BOOL      start;
    }
    T_AUDIO_ONOFF_COMMAND;

    typedef struct 
    {
  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
      UWORD8  onoff_value;   /* This value is used to indicate the required state from the MMI interface */
  #endif
  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
      UWORD8  vul_onoff_value;/* This value is used to indicate the required state from the MMI interface */
      UWORD8  vdl_onoff_value;/* This value is used to indicate the required state from the MMI interface */

  #endif
    }
    T_AUDIO_ONOFF_PARAM;

    typedef struct
    {
      T_AUDIO_ONOFF_COMMAND  command;
      T_AUDIO_ONOFF_PARAM    parameters;
    }
    T_AUDIO_ONOFF_TASK;
  #endif

  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    #define L1_AUDIO_VOICE_UL_OFF 	0
    #define L1_AUDIO_VOICE_UL_ON 	1
    #define L1_AUDIO_VOICE_UL_NO_ACTION	2 

    #define L1_AUDIO_VOICE_DL_OFF 	0
    #define L1_AUDIO_VOICE_DL_ON 	1
    #define L1_AUDIO_VOICE_DL_NO_ACTION	2 
  #endif

  #if (L1_STEREOPATH == 1)
    /**************************************************************************/
    /* Stereopath l1a_l1s_com structure...                                    */
    /**************************************************************************/
    typedef struct
    {
      BOOL  start;
      BOOL  stop;
    }
    T_STEREOPATH_DRV_COMMAND;

    typedef struct
    {
#if (CODE_VERSION == NOT_SIMULATION)
      UWORD8  sampling_frequency;
      UWORD8  DMA_allocation;
      void    (*DMA_int_callback_fct) (UWORD16);
      UWORD8  DMA_channel_number;
      UWORD8  data_type;
      UWORD8  source_port;
      WORD8   *source_buffer_address;
      UWORD16 element_number;
      UWORD16 frame_number;
      UWORD8  mono_stereo;
      UWORD8  feature_identifier;
#else
      UWORD8  dummy;
#endif
    }
    T_STEREOPATH_DRV_PARAM;

    typedef struct
    {
      T_STEREOPATH_DRV_COMMAND   command;
      T_STEREOPATH_DRV_PARAM     parameters;
    }
    T_STEREOPATH_DRV_TASK;

  #endif

  #if (L1_EXT_AUDIO_MGT == 1)
    typedef struct
    {
      UWORD8 session_id;
    }
    T_L1S_EXT_AUDIO_MGT_VAR;
  #endif

  #if (L1_ANR == 1)
    /**************************************************************************/
    /* ANR l1a_l1s_com structure...                                           */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_ANR_COMMAND;

    typedef struct 
    {
      BOOL      anr_enable;
      WORD16    min_gain;
      WORD8     div_factor_shift;
      UWORD8    ns_level;
    }
    T_ANR_PARAM;

    typedef struct
    {
      T_ANR_COMMAND   command;
      T_ANR_PARAM     parameters;
    }
    T_ANR_TASK;
  #endif

  #if (L1_ANR == 2)
    /**************************************************************************/
    /* ANR 2.13 l1a_l1s_com structure...                                      */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_AQI_ANR_COMMAND;

    typedef struct
    {
      T_ANR_CONTROL     anr_ul_control;
      WORD16            control;
      WORD16            ns_level;
      WORD16            tone_ene_th;
      WORD16            tone_cnt_th;
    }
    T_AQI_ANR_PARAM;

    typedef struct
    {
      T_AQI_ANR_COMMAND   command;
      T_AQI_ANR_PARAM     parameters;
    }
    T_AQI_ANR_TASK;
  #endif

  #if (L1_IIR == 1)
    /**************************************************************************/
    /* IIR l1a_l1s_com structure...                                           */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_IIR_COMMAND;

    typedef struct 
    {
      BOOL      iir_enable;
      UWORD8    nb_iir_blocks;
      WORD16   *iir_coefs;
      UWORD8    nb_fir_coefs;
      WORD16   *fir_coefs;
      WORD8     input_scaling;  
      WORD8     fir_scaling;  
      WORD8     input_gain_scaling;
      WORD8     output_gain_scaling;
      UWORD16   output_gain;
      WORD16    feedback;
    }
    T_IIR_PARAM;

    typedef struct
    {
      T_IIR_COMMAND   command;
      T_IIR_PARAM     parameters;
    }
    T_IIR_TASK;
  #endif

  #if (L1_WCM == 1)
    /**************************************************************************/
    /* WCM 1.x l1a_l1s_com structure...                                       */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_AQI_WCM_COMMAND;

    typedef struct
    {
      T_AQI_WCM_COMMAND   command;
      T_AQI_WCM_PARAM     *parameters;
    }
    T_AQI_WCM_TASK;
  #endif

  #if (L1_IIR == 2)
    /**************************************************************************/
    /* IIR 4.x l1a_l1s_com structure...                                       */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_AQI_IIR_COMMAND;

    typedef struct
    {
      T_AQI_IIR_COMMAND   command;
      T_AQI_IIR_PARAM     *parameters;
    }
    T_AQI_IIR_TASK;
  #endif

  
  #if (L1_AGC_UL == 1 || L1_AGC_DL == 1)
    typedef struct
    {
      BOOL  update;
    }
    T_AQI_AGC_COMMAND;
  #endif

  #if (L1_AGC_UL == 1)
    /**************************************************************************/
    /* AGC UL l1a_l1s_com structure...                                        */
    /**************************************************************************/

    typedef struct
    {
      T_AGC_CONTROL agc_ul_control;
      UWORD16       control;
      UWORD16       frame_size;
      WORD16        targeted_level;
      WORD16        signal_up;
      WORD16        signal_down;
      WORD16        max_scale;
      WORD16        gain_smooth_alpha;
      WORD16        gain_smooth_alpha_fast;
      WORD16        gain_smooth_beta;
      WORD16        gain_smooth_beta_fast;
      WORD16        gain_intp_flag;
    }
    T_AQI_AGC_UL_PARAM;

    typedef struct
    {
      T_AQI_AGC_COMMAND   command;
      T_AQI_AGC_UL_PARAM  parameters;
    }
    T_AQI_AGC_UL_TASK;
  #endif

  #if (L1_AGC_DL == 1)
    /**************************************************************************/
    /* AGC DL l1a_l1s_com structure...                                        */
    /**************************************************************************/

    typedef struct
    {
      T_AGC_CONTROL agc_dl_control;
      UWORD16       control;
      UWORD16       frame_size;
      WORD16        targeted_level;
      WORD16        signal_up;
      WORD16        signal_down;
      WORD16        max_scale;
      WORD16        gain_smooth_alpha;
      WORD16        gain_smooth_alpha_fast;
      WORD16        gain_smooth_beta;
      WORD16        gain_smooth_beta_fast;
      WORD16        gain_intp_flag;
    }
    T_AQI_AGC_DL_PARAM;

    typedef struct
    {
      T_AQI_AGC_COMMAND   command;
      T_AQI_AGC_DL_PARAM  parameters;
    }
    T_AQI_AGC_DL_TASK;
  #endif

  #if (L1_DRC == 1)
    /**************************************************************************/
    /* DRC 1.x l1a_l1s_com structure...                                       */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_AQI_DRC_COMMAND;

    typedef struct
    {
      T_AQI_DRC_COMMAND   command;
      T_AQI_DRC_PARAM     *parameters;
    }
    T_AQI_DRC_TASK;


   /**************************************************************************/
   /* MP3 MCU-DSP API                                                        */
   /**************************************************************************/
   typedef struct
   {
    API_SIGNED d_drc_speech_mode_samp_f;
    API_SIGNED d_drc_num_subbands;
    API_SIGNED d_drc_frame_len;
    API_SIGNED d_drc_expansion_knee_fb_bs;
    API_SIGNED d_drc_expansion_knee_md_hg;
    API_SIGNED d_drc_expansion_ratio_fb_bs;
    API_SIGNED d_drc_expansion_ratio_md_hg;
    API_SIGNED d_drc_max_amplification_fb_bs;
    API_SIGNED d_drc_max_amplification_md_hg;
    API_SIGNED d_drc_compression_knee_fb_bs;
    API_SIGNED d_drc_compression_knee_md_hg;
    API_SIGNED d_drc_compression_ratio_fb_bs;
    API_SIGNED d_drc_compression_ratio_md_hg;
    API_SIGNED d_drc_energy_limiting_th_fb_bs;
    API_SIGNED d_drc_energy_limiting_th_md_hg;
    API_SIGNED d_drc_limiter_threshold_fb;
    API_SIGNED d_drc_limiter_threshold_bs;
    API_SIGNED d_drc_limiter_threshold_md;
    API_SIGNED d_drc_limiter_threshold_hg;
    API_SIGNED d_drc_limiter_hangover_spect_preserve;
    API_SIGNED d_drc_limiter_release_fb_bs;
    API_SIGNED d_drc_limiter_release_md_hg;
    API_SIGNED d_drc_gain_track_fb_bs;
    API_SIGNED d_drc_gain_track_md_hg;
    API_SIGNED a_drc_low_pass_filter[17];
    API_SIGNED a_drc_mid_band_filter[17];
   } T_DRC_MCU_DSP;
  #endif

  #if (L1_LIMITER == 1)
    /**************************************************************************/
    /* LIMITER l1a_l1s_com structure...                                       */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
      BOOL  partial_update;
    }
    T_LIMITER_COMMAND;

    typedef struct 
    {
      BOOL      limiter_enable;
      UWORD16   block_size;
      UWORD16   slope_update_period;
      UWORD16   nb_fir_coefs;
      WORD16   *filter_coefs;
      WORD16    thr_low_0;
      WORD16    thr_low_slope;
      WORD16    thr_high_0;
      WORD16    thr_high_slope;
      WORD16    gain_fall;
      WORD16    gain_rise;
    }
    T_LIMITER_PARAM;

    typedef struct
    {
      T_LIMITER_COMMAND   command;
      T_LIMITER_PARAM     parameters;
    }
    T_LIMITER_TASK;
  #endif

  #if (L1_ES == 1)
    /**************************************************************************/
    /* ES l1a_l1s_com structure...                                            */
    /**************************************************************************/
    typedef struct
    {
      BOOL  update;
    }
    T_ES_COMMAND;

    typedef struct
    {
      UWORD8 es_mode;                    /* ES general configuration */
      WORD16 es_gain_dl;
      WORD16 es_gain_ul_1;
      WORD16 es_gain_ul_2;
      WORD16 tcl_fe_ls_thr;              /* TCL reference threshold in FE mode for loud signal */
      WORD16 tcl_dt_ls_thr;              /* TCL reference threshold in DT mode for loud signal */
      WORD16 tcl_fe_ns_thr;              /* TCL reference threshold in FE mode for nominal signal */
      WORD16 tcl_dt_ns_thr;              /* TCL reference threshold in DT mode for nominal signal */
      WORD16 tcl_ne_thr;                 /* TCL reference threshold in NE mode */
      WORD16 ref_ls_pwr;                 /* reference power for loud signals in DL */
      WORD16 switching_time;             /* switching time (idx) */
      WORD16 switching_time_dt;          /* switching time (idx) in DT mode */
      WORD16 hang_time;                  /* hangover time  (idx) */
      WORD16 gain_lin_dl_vect[4];        /* downlink linear gain per state */
      WORD16 gain_lin_ul_vect[4];        /* uplink linear gain per state */
    }
    T_ES_CONFIG;

    typedef struct 
    {
      UWORD8      es_enable;
      UWORD8      es_behavior;
      T_ES_CONFIG es_config;
    }
    T_ES_PARAM;

    typedef struct
    {
      T_ES_COMMAND   command;
      T_ES_PARAM     parameters;
    }
    T_ES_TASK;
  #endif

    typedef struct
    {
      BOOL      start;
    }
    T_AUDIOIT_COMMAND;

    typedef struct
    {
      T_AUDIOIT_COMMAND   command;
    }
    T_AUDIOIT_TASK;

    // Triton Audio ON/OFF Changes
#if (L1_AUDIO_MCU_ONOFF == 1)    
    typedef enum
    {
        L1_AUDIO_NO_ACTION = 0,
        L1_AUDIO_TURN_ON   = 1,
        L1_AUDIO_TURN_OFF  = 2
    }
    T_L1_AUDIO_ACTION;

    typedef enum
    {
        L1_INVALID                      =  0xFF,
        L1_AUDIO_UL_OFF                 =   0,
        L1_AUDIO_UL_SWITCHON_STARTED    =   1,
        L1_AUDIO_UL_ON                  =   2,
        L1_AUDIO_UL_SWITCHOFF_STARTED   =   3
    }
    T_L1_AUDIO_UL_STATE;

    typedef enum
    {
        L1_DL_INVALID  			=  0xFF,
        L1_AUDIO_DL_OFF                 =   0,
        L1_AUDIO_DL_SWITCHON_STARTED    =   1,
        L1_AUDIO_DL_ON                  =   2,
        L1_AUDIO_DL_SWITCHOFF_STARTED   =   3
    }
    T_L1_AUDIO_DL_STATE;

    typedef enum
    {
        L1_AUDIO_DL_PATH = 0,
        L1_AUDIO_UL_PATH = 1
    }
    T_L1_AUDIO_PATH;


    typedef struct
    {
        UWORD8              l1_audio_switch_on_ul_request;
        UWORD8              l1_audio_switch_on_dl_request;

        UWORD8              l1_audio_ul_on2off_hold_time;
        UWORD8              l1_audio_dl_on2off_hold_time;

        T_L1_AUDIO_ACTION   l1_audio_ul_action;
        T_L1_AUDIO_ACTION   l1_audio_dl_action;


        BOOL                l1_audio_ul_switched_on;
        BOOL                l1_audio_dl_switched_on;

        BOOL                l1_audio_ul_switched_off;
        BOOL                l1_audio_dl_switched_off;
        
    }
    T_L1S_AUDIO_ONOFF_MANAGER;

#endif // L1_AUDIO_MCU_ONOFF    

#if(L1_MIDI_BUFFER == 1)
typedef struct
{
   UWORD8 a_midi_buffer_size;
#if (DRP_FW_BUILD != 1)
#if ((OP_L1_STANDALONE == 0) && (PSP_STANDALONE == 0))
      UINT16 audio_play_buffer[AUDIO_EXT_MIDI_BUFFER_SIZE*2];
#endif
#endif
  }T_MIDI_DMA_PARAM;
#endif

#if(L1_BT_AUDIO ==1)
typedef struct
{
      BOOL connected_status;
      BOOL pcm_data_ready;
      UWORD8 pcm_data_pending;
      UWORD8 pcm_data_end;
      UWORD8 pcm_data_failed;
      L1AudioPcmConfig pcmconfig;
      L1AudioPcmBlock  pcmblock;
      L1AudioConfigureCallback audio_configure_callback;
      L1AudioPcmCallback audio_pcmblock_callback;
 } T_L1_BT_AUDIO;
#endif


#endif // AUDIO_TASK

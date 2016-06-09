/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_MSGTY.H
 *
 *        Filename l1audio_msgty.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/
#ifndef _L1AUDIO_MSGTY_H
#define _L1AUDIO_MSGTY_H

#include "../../include/config.h"
#include "../include/l1_confg.h"

#if (AUDIO_TASK == 1)

  #if (OP_RIV_AUDIO == 1)
    #include "rv_general.h"
  #endif

  #if (KEYBEEP)
    typedef struct
   {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD16 d_k_x1_kt0;
      UWORD16 d_k_x1_kt1;
      UWORD16 d_dur_kb;
    }
    T_MMI_KEYBEEP_REQ;
  #endif

  #if (TONE)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD16 d_k_x1_t0;
      UWORD16 d_k_x1_t1;
      UWORD16 d_k_x1_t2;
      UWORD16 d_pe_rep;
      UWORD16 d_pe_off;
      UWORD16 d_se_off;
      UWORD16 d_bu_off;
      UWORD16 d_t0_on;
      UWORD16 d_t0_off;
      UWORD16 d_t1_on;
      UWORD16 d_t1_off;
      UWORD16 d_t2_on;
      UWORD16 d_t2_off;
    }
    T_MMI_TONE_REQ;
  #endif

  #if (MELODY_E1)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8 session_id;
      BOOL    loopback;
      UWORD16 oscillator_used_bitmap;
    }
    T_MMI_MELODY_REQ;
  #endif
  #if (VOICE_MEMO)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  session_id;
    }
    T_MMI_VM_PLAY_REQ;

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  session_id;
      UWORD32 maximum_size;
      BOOL    dtx_used;
      UWORD16 record_coeff_dl;
      UWORD16 record_coeff_ul;
      UWORD16 d_k_x1_t0;
      UWORD16 d_k_x1_t1;
      UWORD16 d_k_x1_t2;
      UWORD16 d_pe_rep;
      UWORD16 d_pe_off;
      UWORD16 d_se_off;
      UWORD16 d_bu_off;
      UWORD16 d_t0_on;
      UWORD16 d_t0_off;
      UWORD16 d_t1_on;
      UWORD16 d_t1_off;
      UWORD16 d_t2_on;
      UWORD16 d_t2_off;
    }
    T_MMI_VM_RECORD_REQ;

    typedef struct
    {
      UWORD32 recorded_size;
    }
    T_L1_VM_RECORD_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        T_RV_HDR  header;
        UWORD32 recorded_size;
      }
      T_MMI_VM_RECORD_CON;
    #else
      typedef T_L1_VM_RECORD_CON T_MMI_VM_RECORD_CON;
    #endif
  #endif

  #if (L1_VOICE_MEMO_AMR)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  session_id;
    }
    T_MMI_VM_AMR_PLAY_REQ;

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  session_id;
      UWORD32 maximum_size;
      BOOL    dtx_used;
      UWORD16 record_coeff_ul;
      UWORD8  amr_vocoder;
    }
    T_MMI_VM_AMR_RECORD_REQ;

    typedef struct
    {
      UWORD32 recorded_size;
    }
    T_L1_VM_AMR_RECORD_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        T_RV_HDR  header;
        UWORD32 recorded_size;
      }
      T_MMI_VM_AMR_RECORD_CON;
    #else
      typedef T_L1_VM_AMR_RECORD_CON T_MMI_VM_AMR_RECORD_CON;
    #endif
  #endif

  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      typedef struct
      {
        UWORD8 channel_id;
        UWORD16 *p_buffer;
      }
      T_L1_AUDIO_DRIVER_IND;
    #endif
  #endif

  #if (SPEECH_RECO)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  database_id;
      UWORD8  word_index;
      BOOL    speech;
      UWORD16 *speech_address;
    }
    T_MMI_SR_ENROLL_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8 error_id;
    }
    T_MMI_SR_ENROLL_STOP_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        UWORD8 error_id;
      }
      T_L1_SR_ENROLL_STOP_CON;
    #else
      typedef T_MMI_SR_ENROLL_STOP_CON T_L1_SR_ENROLL_STOP_CON;
    #endif

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  database_id;
      UWORD8  word_index;
      BOOL    speech;
      UWORD16 *speech_address;
    }
    T_MMI_SR_UPDATE_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8 error_id;
    }
    T_MMI_SR_UPDATE_STOP_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        UWORD8 error_id;
      }
      T_L1_SR_UPDATE_STOP_CON;
    #else
      typedef T_MMI_SR_UPDATE_STOP_CON T_L1_SR_UPDATE_STOP_CON;
    #endif

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  database_id;
      UWORD8  vocabulary_size;
    }
    T_MMI_SR_RECO_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8  error_id;
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
    T_MMI_SR_RECO_STOP_CON;

    typedef struct
    {
      UWORD8 error_id;
    }
    T_L1_SR_RECO_STOP_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        UWORD8  error_id;
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
      T_L1_SR_RECO_STOP_IND;
    #else
      typedef T_MMI_SR_RECO_STOP_CON T_L1_SR_RECO_STOP_IND;
    #endif

    typedef T_L1_SR_RECO_STOP_CON T_L1_SR_PROCESSING_STOP_CON;

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8  database_id;
      UWORD8  word_index;
      UWORD16 *model_address;
      BOOL    speech;
      UWORD16 *speech_address;
      UWORD8  vocabulary_size;
    }
    T_MMI_SR_UPDATE_CHECK_REQ;

    typedef T_MMI_SR_RECO_STOP_CON T_MMI_SR_UPDATE_CHECK_STOP_CON;

    // Background message type
    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8  database_id;
      UWORD8  model_index;
      API     *model_RAM_address;
      BOOL    speech;
      UWORD16 *start_buffer;
      UWORD16 *stop_buffer;
      UWORD16 *start_address;
      UWORD16 *stop_address;
    }
    T_L1_SRBACK_SAVE_DATA_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8  database_id;
      UWORD8  model_index;
      API     *model_RAM_address;
      BOOL    CTO_enable;
    }
    T_L1_SRBACK_LOAD_MODEL_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      API     *model_RAM_address_input;
      UWORD16 *model_RAM_address_output;
    }
    T_L1_SRBACK_TEMP_SAVE_DATA_REQ;
  #endif

  #if (L1_AEC == 1)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
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
    T_MMI_AEC_REQ;

    #if (L1_NEW_AEC)
      typedef struct
      {
        UWORD16 es_level;
        UWORD32 far_end_pow;
        UWORD32 far_end_noise;
      }
      T_L1_AEC_IND;
    #endif
  #endif

#if (L1_AEC == 2)

   typedef enum
   {
	  L1_AQI_AEC_STOP   = 0,
	  L1_AQI_AEC_START  = 1,
	  L1_AQI_AEC_UPDATE = 2
   }
   T_AEC_CONTROL;


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
	T_MMI_AQI_AEC_PARAMS;



    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
       T_RV_HDR  header;
      #endif
       T_AEC_CONTROL aec_control;
       T_MMI_AQI_AEC_PARAMS aec_parameters;
    }
    T_MMI_AQI_AEC_REQ;


	typedef enum
	{
		L1_AQI_AEC_NO_ACTION = -1,
		L1_AQI_AEC_STOPPED = 0,
		L1_AQI_AEC_STARTED = 1,
		L1_AQI_AEC_UPDATED = 2
	}
	T_AEC_ACTION;

	typedef struct
	{
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
       T_RV_HDR  header;
      #endif

		T_AEC_ACTION aec_action;
	}
	T_MMI_AQI_AEC_CON;

	typedef struct
	{
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
       T_RV_HDR  header;
      #endif

	T_AEC_ACTION aec_action;
	}
	T_L1_AQI_AEC_CON;

#endif// L1_AEC ==2


  #if (FIR)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      BOOL      fir_loop;
      UWORD8    update_fir;
      UWORD16   *fir_ul_coefficient;
      UWORD16   *fir_dl_coefficient;
    }
    T_MMI_AUDIO_FIR_REQ;
  #endif
  #if (AUDIO_MODE)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      #if (OP_RIV_AUDIO == 1)
        UWORD8  audio_mode;
      #else
        UWORD16  audio_mode;
      #endif
    }
    T_MMI_AUDIO_MODE;
  #endif
  #if (MELODY_E2)
    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8   melody_id;
      UWORD8   number_of_instrument;
      UWORD8   waves_table_id[SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT];
    }
    T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8   melody_id;
    }
    T_L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8    melody_id;
      UWORD8    number_of_instrument;
    }
    T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ;

    typedef struct
    {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      UWORD8    melody_id;
    }
    T_L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON;

    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
      UWORD8 session_id;
      BOOL    loopback;
    }
    T_MMI_MELODY_E2_REQ;
  #endif

  #if (L1_CPORT == 1)
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
    T_MMI_CPORT_CONFIGURE_REQ;

    typedef struct
    {
     UWORD16  register_id;
     UWORD16 register_value;
    }
    T_L1_CPORT_CONFIGURE_CON;

    #if (OP_RIV_AUDIO == 1)
      typedef struct
      {
        T_RV_HDR  header;
        UWORD8  register_id;
        UWORD16 register_value;
      }
      T_MMI_CPORT_CONFIGURE_CON;
    #else
      typedef T_L1_CPORT_CONFIGURE_CON T_MMI_CPORT_CONFIGURE_CON;
    #endif
  #endif

  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
       UWORD8   onoff_value;
    }
    T_MMI_AUDIO_ONOFF_REQ;
  #endif

  #if (L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    typedef struct
    {
      #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
        T_RV_HDR  header;
      #endif
       UWORD8   vul_onoff_value;
       UWORD8   vdl_onoff_value;
    }
    T_MMI_AUDIO_ONOFF_REQ;
  #endif

  #if (L1_EXT_AUDIO_MGT == 1)
    typedef struct
    {
     UWORD8   sampling_frequency;
     UWORD8   DMA_channel_number;
     UWORD8   data_type;
     UWORD8   element_number;
     UWORD16  frame_number;
     WORD8*   source_buffer_address;
     UWORD8   mono_stereo;
     UWORD8   session_id;
    }
    T_MMI_EXT_AUDIO_MGT_START_REQ;
  #endif

  #if (L1_ANR == 1)
   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      BOOL      anr_enable;
      WORD16    min_gain;
      WORD8     div_factor_shift;
      UWORD8    ns_level;
   }
   T_MMI_ANR_REQ;
  #endif

  #if (L1_ANR == 2)
   typedef enum
   {
      ANR_STOP  = 0,
      ANR_START = 1,
      ANR_UPDATE = 2
   }
   T_ANR_CONTROL;

   typedef struct
   {
      WORD16    control;
      WORD16    ns_level;
      WORD16    tone_ene_th;
      WORD16    tone_cnt_th;
   }
   T_MMI_AQI_ANR_PARAMS;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_ANR_CONTROL         anr_ul_control;
      T_MMI_AQI_ANR_PARAMS  parameters;
   }
   T_MMI_AQI_ANR_REQ;

   typedef enum
   {
      ANR_NO_ACTION = -1,
      ANR_STOPPED   = 0,
      ANR_STARTED   = 1,
      ANR_UPDATED   = 2
   }
   T_ANR_ACTION;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_ANR_ACTION      anr_ul_action;
   }
   T_MMI_AQI_ANR_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_ANR_ACTION      anr_ul_action;
   }
   T_L1_AQI_ANR_CON;

  #endif

  #if (L1_IIR == 1)
   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
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
   T_MMI_IIR_REQ;
  #endif

  #if (L1_WCM == 1)

   typedef enum
   {
      WCM_STOP   = 0,
      WCM_START  = 1,
      WCM_UPDATE = 2
   }
   T_WCM_CONTROL;

   typedef struct
   {
     WORD16 mode;
     WORD16 frame_size;
     WORD16 num_sub_frames;
     WORD16 ratio;
     WORD16 threshold;
     WORD16 gain[16];
   }
   T_MMI_AQI_WCM_PARAMS;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
    T_WCM_CONTROL         wcm_control;
    T_MMI_AQI_WCM_PARAMS  parameters;
   }
   T_MMI_AQI_WCM_REQ;

   typedef T_MMI_AQI_WCM_REQ  T_AQI_WCM_PARAM;

   typedef enum
   {
      WCM_NO_ACTION = -1,
      WCM_STOPPED   = 0,
      WCM_STARTED   = 1,
      WCM_UPDATED   = 2
   }
   T_WCM_ACTION;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
    T_WCM_ACTION  wcm_action;
   }
   T_MMI_AQI_WCM_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_WCM_ACTION  wcm_action;
   }
   T_L1_AQI_WCM_CON;

  #endif


  #if (L1_AGC_UL == 1 || L1_AGC_DL == 1)

   typedef enum
   {
     AGC_STOP   = 0,
     AGC_START  = 1,
     AGC_UPDATE = 2
   }
   T_AGC_CONTROL;

   typedef struct
   {
     UWORD16  control;
     UWORD16  frame_size;
     WORD16   targeted_level;
     WORD16   signal_up;
     WORD16   signal_down;
     WORD16   max_scale;
     WORD16   gain_smooth_alpha;
     WORD16   gain_smooth_alpha_fast;
     WORD16   gain_smooth_beta;
     WORD16   gain_smooth_beta_fast;
     WORD16   gain_intp_flag;
   }
   T_MMI_AQI_AGC_PARAMS;

   typedef enum
   {
     AGC_NO_ACTION = -1,
     AGC_STOPPED = 0,
     AGC_STARTED = 1,
     AGC_UPDATED = 2
   }
   T_AGC_ACTION;

  #endif

  #if (L1_AGC_UL == 1)

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
     T_AGC_CONTROL        agc_ul_control;
     T_MMI_AQI_AGC_PARAMS parameters;
   }
   T_MMI_AQI_AGC_UL_REQ ;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_AGC_ACTION	agc_ul_action;
   }
   T_MMI_AQI_AGC_UL_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_AGC_ACTION  agc_ul_action;
   }
   T_L1_AQI_AGC_UL_CON;  

  #endif


  #if (L1_AGC_DL == 1)

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
     T_AGC_CONTROL        agc_dl_control;
     T_MMI_AQI_AGC_PARAMS parameters;
   }
   T_MMI_AQI_AGC_DL_REQ ;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_AGC_ACTION	agc_dl_action;
   }
   T_MMI_AQI_AGC_DL_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_AGC_ACTION  agc_dl_action;
   }
   T_L1_AQI_AGC_DL_CON;  

  #endif


  #if (L1_IIR == 2)

   typedef enum
   {
      IIR_STOP   = 0,
      IIR_START  = 1,
      IIR_UPDATE = 2
   }
   T_IIR_CONTROL;


   typedef struct
   {
      UWORD16 fir_enable;
      UWORD16 fir_length;
      WORD16  fir_shift;
      WORD16  fir_taps[40];
   }
   T_MMI_AQI_IIR_FIR_PARAMS;

   typedef struct
   {
      WORD16 sos_fact;
      WORD16 sos_fact_form;
      WORD16 sos_den[2];
      WORD16 sos_num[3];
      WORD16 sos_num_form;
   }
   T_MMI_AQI_IIR_SINGLE_SOS_PARAMS;

   typedef struct
   {

     UWORD16 sos_enable;
     UWORD16 sos_number;
     T_MMI_AQI_IIR_SINGLE_SOS_PARAMS sos_filter[6];
   }
   T_MMI_AQI_IIR_SOS_PARAMS;

   typedef struct
   {
     UWORD16 control;
     UWORD16 frame_size;
     UWORD16 fir_swap;
     T_MMI_AQI_IIR_FIR_PARAMS fir_filter;
     T_MMI_AQI_IIR_SOS_PARAMS sos_filter;
     WORD16 gain;
   }
   T_MMI_AQI_IIR_PARAMS;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
    T_IIR_CONTROL           iir_dl_control;
      T_MMI_AQI_IIR_PARAMS  parameters;
   }
   T_MMI_AQI_IIR_DL_REQ;

   typedef T_MMI_AQI_IIR_DL_REQ  T_AQI_IIR_PARAM;

   typedef enum
   {
      IIR_NO_ACTION = -1,
      IIR_STOPPED   = 0,
      IIR_STARTED   = 1,
      IIR_UPDATED   = 2
   }
   T_IIR_ACTION;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
    T_IIR_ACTION  iir_dl_action;
   }
   T_MMI_AQI_IIR_DL_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_IIR_ACTION  iir_dl_action;
   }
   T_L1_AQI_IIR_DL_CON;

  #endif


#if (L1_DRC == 1)

   typedef enum
   {
      DRC_STOP   = 0,
      DRC_START  = 1,
      DRC_UPDATE = 2
   }
   T_DRC_CONTROL;

   typedef struct
   {
     WORD16 speech_mode_samp_f;
     WORD16 num_subbands;
     WORD16 frame_len;
     WORD16 expansion_knee_fb_bs;
     WORD16 expansion_knee_md_hg;
     WORD16 expansion_ratio_fb_bs;
     WORD16 expansion_ratio_md_hg;
     WORD16 max_amplification_fb_bs;
     WORD16 max_amplification_md_hg;
     WORD16 compression_knee_fb_bs;
     WORD16 compression_knee_md_hg;
     WORD16 compression_ratio_fb_bs;
     WORD16 compression_ratio_md_hg;
     WORD16 energy_limiting_th_fb_bs;
     WORD16 energy_limiting_th_md_hg;
     WORD16 limiter_threshold_fb;
     WORD16 limiter_threshold_bs;
     WORD16 limiter_threshold_md;
     WORD16 limiter_threshold_hg;
     WORD16 limiter_hangover_spect_preserve;
     WORD16 limiter_release_fb_bs;
     WORD16 limiter_release_md_hg;
     WORD16 gain_track_fb_bs;
     WORD16 gain_track_md_hg;
     WORD16 low_pass_filter[17];
     WORD16 mid_band_filter[17];
   }
   T_MMI_AQI_DRC_PARAMS;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_DRC_CONTROL         drc_dl_control;
      T_MMI_AQI_DRC_PARAMS  parameters;
   }
   T_MMI_AQI_DRC_REQ;

   typedef T_MMI_AQI_DRC_REQ  T_AQI_DRC_PARAM;

   typedef enum
   {
      DRC_NO_ACTION = -1,
      DRC_STOPPED   = 0,
      DRC_STARTED   = 1,
      DRC_UPDATED   = 2
   }
   T_DRC_ACTION;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
    T_DRC_ACTION  drc_dl_action;
   }
   T_MMI_AQI_DRC_CON;

   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      T_DRC_ACTION  drc_dl_action;
   }
   T_L1_AQI_DRC_CON;

  #endif //L1_DRC

 
  #if (L1_LIMITER == 1)
   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
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
   T_MMI_LIMITER_REQ;
  #endif

  #if (L1_ES == 1)
   typedef struct
   {
    #if (OP_RIV_AUDIO == 1)
      T_RV_HDR  header;
    #endif
      BOOL   es_enable;
      UWORD8 es_behavior;
      UWORD8 es_mode;
      WORD16 es_gain_dl;
      WORD16 es_gain_ul_1;
      WORD16 es_gain_ul_2;
      WORD16 tcl_fe_ls_thr;
      WORD16 tcl_dt_ls_thr;
      WORD16 tcl_fe_ns_thr;
      WORD16 tcl_dt_ns_thr;
      WORD16 tcl_ne_thr;
      WORD16 ref_ls_pwr;
      WORD16 switching_time;
      WORD16 switching_time_dt;
      WORD16 hang_time;
      WORD16 gain_lin_dl_vect[4];
      WORD16 gain_lin_ul_vect[4];
   }
   T_MMI_ES_REQ;
  #endif
  
#if (L1_VOCODER_IF_CHANGE == 1)
  typedef struct 
  {
    BOOL vocoder_state; // TRUE if enable request, FALSE if disable request
  } T_MMI_TCH_VOCODER_CFG_REQ;
#endif // L1_VOCODER_IF_CHANGE == 1

#if (L1_PCM_EXTRACTION)
  typedef struct
  {
  #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
    T_RV_HDR  header;
  #endif
    UWORD8 session_id;
    UWORD8 download_ul_gain;
    UWORD8 download_dl_gain;
    UWORD32 maximum_size;
  } T_MMI_PCM_DOWNLOAD_START_REQ;

  typedef struct
  {
  #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
    T_RV_HDR  header;
  #endif
    UWORD8 session_id;
    UWORD8 upload_ul_gain;
    UWORD8 upload_dl_gain;
    UWORD32 maximum_size;
  } T_MMI_PCM_UPLOAD_START_REQ;

  typedef struct
  {
  #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
    T_RV_HDR  header;
  #endif
    UWORD32 maximum_size;
  } T_MMI_PCM_DOWNLOAD_STOP_REQ;

  typedef struct
  {
    UWORD32 uploaded_size;
  } T_L1_PCM_UPLOAD_STOP_CON;

 #if (OP_RIV_AUDIO == 1)
  typedef struct
  {
    T_RV_HDR  header;
    UWORD32 uploaded_size;
  }
  T_MMI_PCM_UPLOAD_STOP_CON;
  #else
    typedef T_L1_PCM_UPLOAD_STOP_CON T_MMI_PCM_UPLOAD_STOP_CON;
  #endif

#endif  /* L1_PCM_EXTRACTION */

#endif // AUDIO_TASK

typedef struct
{
  UWORD8 outen1;
  UWORD8 outen2;
  UWORD8 outen3;
  UWORD8 classD;
} T_MMI_OUTEN_CFG_REQ;

typedef struct
{
  #if (OP_RIV_AUDIO == 1)
    T_RV_HDR  header;
  #endif
  UWORD8 outen1;
  UWORD8 outen2;
  UWORD8 outen3;
  UWORD8 classD;
} T_MMI_OUTEN_CFG_READ_CON;


#endif // _L1AUDIO_MSGTY_H

/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_MSGTY.H
 *
 *        Filename l1audio_msgty.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)

  #if (OP_RIV_AUDIO == 1)
    #include "../../riviera/rv/rv_general.h"
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

  #if (AEC)
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
#endif // AUDIO_TASK

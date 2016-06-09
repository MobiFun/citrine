/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_DEFTY.H
 *
 *        Filename l1audio_defty.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)

  #if (KEYBEEP)
    /***************************************************************************************/
    /* Keybeep l1a_l1s_com structure...                                                    */
    /***************************************************************************************/
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
    /***************************************************************************************/
    /* Tone l1a_l1s_com structure...                                                    */
    /***************************************************************************************/
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
    /***************************************************************************************/
    /* Melody l1a_l1s_com structure...                                                    */
    /***************************************************************************************/
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

    /***************************************************************************************/
    /* Melody l1s structure...                                                             */
    /***************************************************************************************/

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
    /***************************************************************************************/
    /* Voice memo l1a_l1s_com structure...                                                 */
    /***************************************************************************************/

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

    /***************************************************************************************/
    /* Voice memo l1s structure...                                                         */
    /***************************************************************************************/

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
  #if (L1_VOICE_MEMO_AMR)
    /***************************************************************************************/
    /* Voice memo amr l1a_l1s_com structure...                                                 */
    /***************************************************************************************/

    typedef struct
    {
      BOOL  start;
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
        T_VM_AMR_RECORD_COMMAND   command;
        T_VM_AMR_RECORD_PARAM     parameters;
    }
    T_VM_AMR_RECORD_TASK;

    typedef struct
    {
      T_VM_AMR_PLAY_TASK    play;
      T_VM_AMR_RECORD_TASK  record;
    }
    T_VM_AMR_TASK;

    /***************************************************************************************/
    /* Voice memo l1s structure...                                                         */
    /***************************************************************************************/

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
    /***************************************************************************************/
    /* Speech recogniton l1a_l1s_com structure...                                          */
    /***************************************************************************************/

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

  #if (AEC)
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
    /***************************************************************************************/
    /* Melody format E2 l1a_l1s_com structure...                                           */
    /***************************************************************************************/
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

    /***************************************************************************************/
    /* Melody format E2 l1s structure...                                                   */
    /***************************************************************************************/
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

    /***************************************************************************************/
    /* Melody format E2 audio background structure...                                      */
    /***************************************************************************************/
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
    /***************************************************************************************/
    /* Cport l1a_l1s_com structure...                                                      */
    /***************************************************************************************/
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
#endif // AUDIO_TASK

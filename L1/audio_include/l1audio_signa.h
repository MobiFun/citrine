/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_SIGNA.H
 *
 *        Filename l1audio_signa.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include "../../include/config.h"
#include "../include/l1_confg.h"

#if (AUDIO_TASK == 1)
  #define P_AUDIO ( 0x18 )

  // Messages MMI <-> L1A
  #if (KEYBEEP)
    #define MMI_KEYBEEP_START_REQ             ( ( P_AUDIO << 8 ) |   0 ) // build: T_MMI_KEYBEEP_REQ
    #define MMI_KEYBEEP_STOP_REQ              ( ( P_AUDIO << 8 ) |   1 ) // build: trigger
    #define MMI_KEYBEEP_START_CON             ( ( P_AUDIO << 8 ) |   2 )
    #define MMI_KEYBEEP_STOP_CON              ( ( P_AUDIO << 8 ) |   3 )
  #endif
  #if (TONE)
    #define MMI_TONE_START_REQ                ( ( P_AUDIO << 8 ) |   4 ) // build: T_MMI_TONE_REQ 
    #define MMI_TONE_STOP_REQ                 ( ( P_AUDIO << 8 ) |   5 ) // build: trigger
    #define MMI_TONE_START_CON                ( ( P_AUDIO << 8 ) |   6 )
    #define MMI_TONE_STOP_CON                 ( ( P_AUDIO << 8 ) |   7 )
  #endif
  #if (MELODY_E1)
    #define MMI_MELODY0_START_REQ             ( ( P_AUDIO << 8 ) |   8 ) // build: T_MMI_MELODY_REQ
    #define MMI_MELODY0_STOP_REQ              ( ( P_AUDIO << 8 ) |   9 ) // build: trigger
    #define MMI_MELODY0_START_CON             ( ( P_AUDIO << 8 ) |  10 )
    #define MMI_MELODY0_STOP_CON              ( ( P_AUDIO << 8 ) |  11 )

    #define MMI_MELODY1_START_REQ             ( ( P_AUDIO << 8 ) |  12 ) // build: T_MMI_MELODY_REQ
    #define MMI_MELODY1_STOP_REQ              ( ( P_AUDIO << 8 ) |  13 ) // build: trigger
    #define MMI_MELODY1_START_CON             ( ( P_AUDIO << 8 ) |  14 )
    #define MMI_MELODY1_STOP_CON              ( ( P_AUDIO << 8 ) |  15 )
  #endif
  #if (VOICE_MEMO)
    #define MMI_VM_PLAY_START_REQ             ( ( P_AUDIO << 8 ) |  16 ) // build: T_MMI_VM_PLAY_REQ
    #define MMI_VM_PLAY_STOP_REQ              ( ( P_AUDIO << 8 ) |  17 ) // build: trigger
    #define MMI_VM_PLAY_START_CON             ( ( P_AUDIO << 8 ) |  18 )
    #define MMI_VM_PLAY_STOP_CON              ( ( P_AUDIO << 8 ) |  19 )

    #define MMI_VM_RECORD_START_REQ           ( ( P_AUDIO << 8 ) |  20 ) // build: T_MMI_VM_RECORD_REQ
    #define MMI_VM_RECORD_STOP_REQ            ( ( P_AUDIO << 8 ) |  21 ) // build: trigger
    #define MMI_VM_RECORD_START_CON           ( ( P_AUDIO << 8 ) |  22 )
    #define MMI_VM_RECORD_STOP_CON            ( ( P_AUDIO << 8 ) |  23 )
  #endif
  #if (L1_PCM_EXTRACTION)
    #define MMI_PCM_DOWNLOAD_START_REQ        ( ( P_AUDIO << 8 ) | 24 )
    #define MMI_PCM_DOWNLOAD_STOP_REQ         ( ( P_AUDIO << 8 ) | 25 )
    #define MMI_PCM_DOWNLOAD_START_CON        ( ( P_AUDIO << 8 ) | 26 )
    #define MMI_PCM_DOWNLOAD_STOP_CON         ( ( P_AUDIO << 8 ) | 27 )

    #define MMI_PCM_UPLOAD_START_REQ          ( ( P_AUDIO << 8 ) | 28 )
    #define MMI_PCM_UPLOAD_STOP_REQ           ( ( P_AUDIO << 8 ) | 29 )
    #define MMI_PCM_UPLOAD_START_CON          ( ( P_AUDIO << 8 ) | 30 )
    #define MMI_PCM_UPLOAD_STOP_CON           ( ( P_AUDIO << 8 ) | 31 )
  #endif
  #if (SPEECH_RECO)
    #define MMI_SR_ENROLL_START_REQ           ( ( P_AUDIO << 8 ) |  24 ) // build: T_MMI_SR_ENROLL_REQ
    #define MMI_SR_ENROLL_STOP_REQ            ( ( P_AUDIO << 8 ) |  25 ) // build: trigger
    #define MMI_SR_ENROLL_START_CON           ( ( P_AUDIO << 8 ) |  26 )
    #define MMI_SR_ENROLL_STOP_CON            ( ( P_AUDIO << 8 ) |  27 )

    #define MMI_SR_UPDATE_START_REQ           ( ( P_AUDIO << 8 ) |  28 ) // build: T_MMI_SR_UPDATE_REQ
    #define MMI_SR_UPDATE_STOP_REQ            ( ( P_AUDIO << 8 ) |  29 ) // build: trigger
    #define MMI_SR_UPDATE_START_CON           ( ( P_AUDIO << 8 ) |  30 )
    #define MMI_SR_UPDATE_STOP_CON            ( ( P_AUDIO << 8 ) |  31 )

    #define MMI_SR_RECO_START_REQ             ( ( P_AUDIO << 8 ) |  32 ) // build: T_MMI_SR_RECO_REQ
    #define MMI_SR_RECO_STOP_REQ              ( ( P_AUDIO << 8 ) |  33 ) // build: trigger
    #define MMI_SR_RECO_START_CON             ( ( P_AUDIO << 8 ) |  34 )
    #define MMI_SR_RECO_STOP_CON              ( ( P_AUDIO << 8 ) |  35 )

    #define MMI_SR_UPDATE_CHECK_START_REQ     ( ( P_AUDIO << 8 ) |  36 ) // build: T_MMI_SR_UPDATE_CHECK_REQ
    #define MMI_SR_UPDATE_CHECK_STOP_REQ      ( ( P_AUDIO << 8 ) |  37 ) // build: trigger
    #define MMI_SR_UPDATE_CHECK_START_CON     ( ( P_AUDIO << 8 ) |  38 )
    #define MMI_SR_UPDATE_CHECK_STOP_CON      ( ( P_AUDIO << 8 ) |  39 )
  #endif
  #if (L1_AEC == 1)
    #define MMI_AEC_REQ                       ( ( P_AUDIO << 8 ) |  40 ) // build: T_MMI_AEC_REQ
    #define MMI_AEC_CON                       ( ( P_AUDIO << 8 ) |  41 )
  #endif
  #if (L1_AEC == 2)
    #define MMI_AQI_AEC_REQ                   ( ( P_AUDIO << 8 ) |  143 ) // build: T_MMI_AQI_AEC_REQ
    #define MMI_AQI_AEC_CON                   ( ( P_AUDIO << 8 ) |  144 )
  #endif
  #if (FIR)
    #define MMI_AUDIO_FIR_REQ                 ( ( P_AUDIO << 8 ) |  42 ) // build: T_MMI_AUDIO_FIR_REQ
    #define MMI_AUDIO_FIR_CON                 ( ( P_AUDIO << 8 ) |  43 )
  #endif
  #if (AUDIO_MODE)
    #define MMI_AUDIO_MODE_REQ                ( ( P_AUDIO << 8 ) |  44 ) // build: T_MMI_AUDIO_MODE
    #define MMI_AUDIO_MODE_CON                ( ( P_AUDIO << 8 ) |  45 )
  #endif
  #if (MELODY_E2)
    #define MMI_MELODY0_E2_START_REQ          ( ( P_AUDIO << 8 ) |  46 ) // build: T_MMI_MELODY_E2_REQ
    #define MMI_MELODY0_E2_STOP_REQ           ( ( P_AUDIO << 8 ) |  47 ) // build: trigger
    #define MMI_MELODY0_E2_START_CON          ( ( P_AUDIO << 8 ) |  48 )
    #define MMI_MELODY0_E2_STOP_CON           ( ( P_AUDIO << 8 ) |  49 )

    #define MMI_MELODY1_E2_START_REQ          ( ( P_AUDIO << 8 ) |  50 ) // build: T_MMI_MELODY_E2_REQ
    #define MMI_MELODY1_E2_STOP_REQ           ( ( P_AUDIO << 8 ) |  51 ) // build: trigger
    #define MMI_MELODY1_E2_START_CON          ( ( P_AUDIO << 8 ) |  52 )
    #define MMI_MELODY1_E2_STOP_CON           ( ( P_AUDIO << 8 ) |  53 )
  #endif
  #if (L1_VOICE_MEMO_AMR)
    #define MMI_VM_AMR_PLAY_START_REQ         ( ( P_AUDIO << 8 ) |  54 ) // build: T_MMI_VM_AMR_PLAY_REQ
    #define MMI_VM_AMR_PLAY_STOP_REQ          ( ( P_AUDIO << 8 ) |  55 ) // build: trigger
    #define MMI_VM_AMR_PLAY_START_CON         ( ( P_AUDIO << 8 ) |  56 )
    #define MMI_VM_AMR_PLAY_STOP_CON          ( ( P_AUDIO << 8 ) |  57 )
    #define MMI_VM_AMR_PAUSE_REQ              ( ( P_AUDIO << 8 ) | 158 )
    #define MMI_VM_AMR_RESUME_REQ             ( ( P_AUDIO << 8 ) | 159 )
    #define MMI_VM_AMR_PAUSE_CON              ( ( P_AUDIO << 8 ) | 160 )
    #define MMI_VM_AMR_RESUME_CON             ( ( P_AUDIO << 8 ) | 161 )
    

    #define MMI_VM_AMR_RECORD_START_REQ       ( ( P_AUDIO << 8 ) |  58 ) // build: T_MMI_VM_AMR_RECORD_REQ
    #define MMI_VM_AMR_RECORD_STOP_REQ        ( ( P_AUDIO << 8 ) |  59 ) // build: trigger
    #define MMI_VM_AMR_RECORD_START_CON       ( ( P_AUDIO << 8 ) |  60 )
    #define MMI_VM_AMR_RECORD_STOP_CON        ( ( P_AUDIO << 8 ) |  61 )
  #endif
  #if (L1_CPORT == 1)
    #define MMI_CPORT_CONFIGURE_REQ           ( ( P_AUDIO << 8 ) |   62 )
    #define MMI_CPORT_CONFIGURE_CON           ( ( P_AUDIO << 8 ) |   63 )
  #endif
  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1 || L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    #define MMI_AUDIO_ONOFF_REQ                 ( ( P_AUDIO << 8 ) |  64 )
    #define MMI_AUDIO_ONOFF_CON                 ( ( P_AUDIO << 8 ) |  65 )
  #endif
  #if (L1_EXT_AUDIO_MGT == 1)
    #define MMI_EXT_AUDIO_MGT_START_REQ       ( ( P_AUDIO << 8 ) |  66 )
    #define MMI_EXT_AUDIO_MGT_STOP_REQ        ( ( P_AUDIO << 8 ) |  67 )
    #define MMI_EXT_AUDIO_MGT_START_CON       ( ( P_AUDIO << 8 ) |  68 )
    #define MMI_EXT_AUDIO_MGT_STOP_CON        ( ( P_AUDIO << 8 ) |  69 )
  #endif
  #if (L1_ANR == 1)
    #define MMI_ANR_REQ                       ( ( P_AUDIO << 8 ) |  70 )
    #define MMI_ANR_CON                       ( ( P_AUDIO << 8 ) |  71 )
  #endif
  #if (L1_IIR == 1)
    #define MMI_IIR_REQ                       ( ( P_AUDIO << 8 ) |  72 )
    #define MMI_IIR_CON                       ( ( P_AUDIO << 8 ) |  73 )
  #endif
  #if (L1_LIMITER == 1)
    #define MMI_LIMITER_REQ                   ( ( P_AUDIO << 8 ) |  74 )
    #define MMI_LIMITER_CON                   ( ( P_AUDIO << 8 ) |  75 )
  #endif
  #if (L1_ES == 1)
    #define MMI_ES_REQ                        ( ( P_AUDIO << 8 ) |  76 )
    #define MMI_ES_CON                        ( ( P_AUDIO << 8 ) |  77 )
  #endif
  #if (L1_DRC == 1)
    #define MMI_AQI_DRC_REQ                   ( ( P_AUDIO << 8 ) |  146 )
    #define MMI_AQI_DRC_CON                   ( ( P_AUDIO << 8 ) |  147 )
  #endif
  #if (L1_VOCODER_IF_CHANGE == 1)
    #define MMI_TCH_VOCODER_CFG_REQ   0x0E0C
    #define MMI_TCH_VOCODER_CFG_CON   0x4E08
  #endif
  #if (L1_WCM == 1)
    #define MMI_AQI_WCM_REQ                    ( ( P_AUDIO << 8 ) |  149 )
    #define MMI_AQI_WCM_CON                    ( ( P_AUDIO << 8 ) |  150 )
  #endif

  #if (L1_AGC_UL == 1)
    #define MMI_AQI_AGC_UL_REQ                   ( ( P_AUDIO << 8 ) |  137)
    #define MMI_AQI_AGC_UL_CON                   ( ( P_AUDIO << 8 ) |  138)
  #endif

  #if (L1_AGC_DL == 1)
    #define MMI_AQI_AGC_DL_REQ                   ( ( P_AUDIO << 8 ) |  139)
    #define MMI_AQI_AGC_DL_CON                   ( ( P_AUDIO << 8 ) |  140)
  #endif


  #if (L1_ANR == 2)
    #define MMI_AQI_ANR_REQ                   ( ( P_AUDIO << 8 ) |  131)
    #define MMI_AQI_ANR_CON                   ( ( P_AUDIO << 8 ) |  132)
  #endif

  #if (L1_IIR == 2)
    #define MMI_AQI_IIR_DL_REQ                   ( ( P_AUDIO << 8 ) |  134)
    #define MMI_AQI_IIR_DL_CON                   ( ( P_AUDIO << 8 ) |  135)
  #endif

  #define MMI_OUTEN_CFG_REQ                     ( ( P_AUDIO << 8) | 152)
  #define MMI_OUTEN_CFG_CON                     ( ( P_AUDIO << 8) | 153)
  #define MMI_OUTEN_CFG_READ_REQ                ( ( P_AUDIO << 8) | 154)
  #define MMI_OUTEN_CFG_READ_CON                ( ( P_AUDIO << 8) | 155)

  #if(L1_BT_AUDIO==1)
    #define MMI_BT_ENABLE_REQ                            ((P_AUDIO<<8)|167)
    #define MMI_BT_DISABLE_REQ                           ((P_AUDIO<<8)|169)
  #endif

  // Messages L1S -> L1A
  #if (KEYBEEP)
    #define L1_KEYBEEP_START_CON              ( ( P_AUDIO << 8 ) |  78 )
    #define L1_KEYBEEP_STOP_CON               ( ( P_AUDIO << 8 ) |  79 )
  #endif
  #if (TONE)
    #define L1_TONE_START_CON                 ( ( P_AUDIO << 8 ) |  80 )
    #define L1_TONE_STOP_CON                  ( ( P_AUDIO << 8 ) |  81 )
  #endif
  #if (MELODY_E1)
    #define L1_MELODY0_START_CON              ( ( P_AUDIO << 8 ) |  82 )
    #define L1_MELODY0_STOP_CON               ( ( P_AUDIO << 8 ) |  83 )

    #define L1_MELODY1_START_CON              ( ( P_AUDIO << 8 ) |  84 )
    #define L1_MELODY1_STOP_CON               ( ( P_AUDIO << 8 ) |  85 )
  #endif
  #if (VOICE_MEMO)
    #define L1_VM_PLAY_START_CON              ( ( P_AUDIO << 8 ) |  86 )
    #define L1_VM_PLAY_STOP_CON               ( ( P_AUDIO << 8 ) |  87 )

    #define L1_VM_RECORD_START_CON            ( ( P_AUDIO << 8 ) |  88 )
    #define L1_VM_RECORD_STOP_CON             ( ( P_AUDIO << 8 ) |  89 )
  #endif
  #if (L1_PCM_EXTRACTION)
    #define L1_PCM_DOWNLOAD_START_CON         ( ( P_AUDIO << 8 ) |  90 )
    #define L1_PCM_DOWNLOAD_STOP_CON          ( ( P_AUDIO << 8 ) |  91 )

    #define L1_PCM_UPLOAD_START_CON           ( ( P_AUDIO << 8 ) |  92 )
    #define L1_PCM_UPLOAD_STOP_CON            ( ( P_AUDIO << 8 ) |  93 )

  #endif
  #if (SPEECH_RECO)
    #define L1_SR_ENROLL_START_CON            ( ( P_AUDIO << 8 ) |  90 )
    #define L1_SR_ENROLL_STOP_CON             ( ( P_AUDIO << 8 ) |  91 )

    #define L1_SR_UPDATE_START_CON            ( ( P_AUDIO << 8 ) |  92 )
    #define L1_SR_UPDATE_STOP_CON             ( ( P_AUDIO << 8 ) |  93 )

    #define L1_SR_RECO_START_CON              ( ( P_AUDIO << 8 ) |  94 )
    #define L1_SR_RECO_STOP_CON               ( ( P_AUDIO << 8 ) |  95 )
    #define L1_SR_RECO_STOP_IND               ( ( P_AUDIO << 8 ) |  96 )
    #define L1_SR_PROCESSING_STOP_CON         ( ( P_AUDIO << 8 ) |  97 )
  #endif
  #if (L1_AEC == 1)
    #define L1_AEC_CON                        ( ( P_AUDIO << 8 ) |  98 )
    #define L1_AEC_IND                        ( ( P_AUDIO << 8 ) |  99 )
  #endif
  #if (L1_AEC == 2)
    #define L1_AQI_AEC_CON                    ( ( P_AUDIO << 8 ) |  145 )
  #endif
  #if (FIR)
    #define L1_AUDIO_FIR_CON                  ( ( P_AUDIO << 8 ) | 100 )
  #endif
  #if (AUDIO_MODE)
    #define L1_AUDIO_MODE_CON                 ( ( P_AUDIO << 8 ) | 101 )
  #endif
  #if (L1_VOICE_MEMO_AMR)
    #define L1_VM_AMR_PLAY_START_CON          ( ( P_AUDIO << 8 ) | 102 )
    #define L1_VM_AMR_PLAY_STOP_CON           ( ( P_AUDIO << 8 ) | 103 )
    #define L1_VM_AMR_PAUSE_CON               ( ( P_AUDIO << 8 ) | 156 )
    #define L1_VM_AMR_RESUME_CON              ( ( P_AUDIO << 8 ) | 157 )
    

    #define L1_VM_AMR_RECORD_START_CON        ( ( P_AUDIO << 8 ) | 104 )
    #define L1_VM_AMR_RECORD_STOP_CON         ( ( P_AUDIO << 8 ) | 105 )
  #endif
  #if (L1_CPORT == 1)
    #define L1_CPORT_CONFIGURE_CON            ( ( P_AUDIO << 8 ) | 106 )
  #endif
  #if (L1_STEREOPATH == 1)
    #define L1_STEREOPATH_DRV_START_CON       ( ( P_AUDIO << 8 ) | 107 )
    #define L1_STEREOPATH_DRV_STOP_CON        ( ( P_AUDIO << 8 ) | 108 )
  #endif
  #if (L1_ANR == 1)
    #define L1_ANR_CON                        ( ( P_AUDIO << 8 ) | 109 )
  #endif
  #if (L1_IIR == 1)
    #define L1_IIR_CON                        ( ( P_AUDIO << 8 ) | 110 )
  #endif
  #if (L1_DRC == 1)
    #define L1_AQI_DRC_CON                    ( ( P_AUDIO << 8 ) | 148 )
  #endif
  #if (L1_LIMITER == 1)
    #define L1_LIMITER_CON                    ( ( P_AUDIO << 8 ) | 111 )
  #endif
  #if (L1_ES == 1)
    #define L1_ES_CON                         ( ( P_AUDIO << 8 ) | 112 )
  #endif

  #if (L1_VOCODER_IF_CHANGE == 1)
    #define L1_VOCODER_CFG_ENABLE_CON        ( ( P_AUDIO << 8 ) |  129 )
    #define L1_VOCODER_CFG_DISABLE_CON      ( ( P_AUDIO << 8 ) |  130 )
  #endif

  #if (L1_AGC_UL == 1)
    #define L1_AQI_AGC_UL_CON                ( ( P_AUDIO << 8 ) |  141)
  #endif

  #if (L1_AGC_DL == 1)
    #define L1_AQI_AGC_DL_CON                ( ( P_AUDIO << 8 ) |  142)
  #endif

  #if (L1_ANR == 2)
    #define L1_AQI_ANR_CON                    ( ( P_AUDIO << 8 ) | 133 )
  #endif

  #if (L1_IIR == 2)
    #define L1_AQI_IIR_DL_CON                   ( ( P_AUDIO << 8 ) |  136)
  #endif

  #if (L1_WCM == 1)
    #define L1_AQI_WCM_CON                  ( ( P_AUDIO << 8 ) | 151 )
  #endif

  // Messages L1A <-> Audio background
  #if (SPEECH_RECO)
    #define L1_SRBACK_SAVE_DATA_REQ                 ( ( P_AUDIO << 8 ) |  113 )
    #define L1_SRBACK_SAVE_DATA_CON                 ( ( P_AUDIO << 8 ) |  114 )
    #define L1_SRBACK_LOAD_MODEL_REQ                ( ( P_AUDIO << 8 ) |  115 )
    #define L1_SRBACK_LOAD_MODEL_CON                ( ( P_AUDIO << 8 ) |  116 )
    #define L1_SRBACK_TEMP_SAVE_DATA_REQ            ( ( P_AUDIO << 8 ) |  117 )
    #define L1_SRBACK_TEMP_SAVE_DATA_CON            ( ( P_AUDIO << 8 ) |  118 )
  #endif
  #if (MELODY_E2)
    #define L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ   ( ( P_AUDIO << 8 ) |  119 )
    #define L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   ( ( P_AUDIO << 8 ) |  120 )
    #define L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ ( ( P_AUDIO << 8 ) |  121 )
    #define L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON ( ( P_AUDIO << 8 ) |  122 )

    #define L1_MELODY0_E2_STOP_CON                  ( ( P_AUDIO << 8 ) |  123 )
    #define L1_MELODY1_E2_STOP_CON                  ( ( P_AUDIO << 8 ) |  124 )
  #endif

  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      #define L1_AUDIO_DRIVER_IND                   ( ( P_AUDIO << 8 ) |  125 )
    #endif
  #endif

  #if (L1_EXTERNAL_AUDIO_VOICE_ONOFF == 1 || L1_EXT_MCU_AUDIO_VOICE_ONOFF == 1)
    #define L1_AUDIO_ONOFF_CON                      ( ( P_AUDIO << 8 ) |  126 )
  #endif

#endif // AUDIO_TASK == 1

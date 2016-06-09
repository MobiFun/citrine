/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_SIGNA.H
 *
 *        Filename l1audio_signa.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)
  #define P_AUDIO ( 0x18 )

  // Messages MMI <-> L1A
  #if (KEYBEEP)
    #define MMI_KEYBEEP_START_REQ             ( ( P_AUDIO << 8 ) |   0 )
    #define MMI_KEYBEEP_STOP_REQ              ( ( P_AUDIO << 8 ) |   1 )
    #define MMI_KEYBEEP_START_CON             ( ( P_AUDIO << 8 ) |   2 )
    #define MMI_KEYBEEP_STOP_CON              ( ( P_AUDIO << 8 ) |   3 )
  #endif
  #if (TONE)
    #define MMI_TONE_START_REQ                ( ( P_AUDIO << 8 ) |   4 )
    #define MMI_TONE_STOP_REQ                 ( ( P_AUDIO << 8 ) |   5 )
    #define MMI_TONE_START_CON                ( ( P_AUDIO << 8 ) |   6 )
    #define MMI_TONE_STOP_CON                 ( ( P_AUDIO << 8 ) |   7 )
  #endif
  #if (MELODY_E1)
    #define MMI_MELODY0_START_REQ             ( ( P_AUDIO << 8 ) |   8 )
    #define MMI_MELODY0_STOP_REQ              ( ( P_AUDIO << 8 ) |   9 )
    #define MMI_MELODY0_START_CON             ( ( P_AUDIO << 8 ) |  10 )
    #define MMI_MELODY0_STOP_CON              ( ( P_AUDIO << 8 ) |  11 )

    #define MMI_MELODY1_START_REQ             ( ( P_AUDIO << 8 ) |  12 )
    #define MMI_MELODY1_STOP_REQ              ( ( P_AUDIO << 8 ) |  13 )
    #define MMI_MELODY1_START_CON             ( ( P_AUDIO << 8 ) |  14 )
    #define MMI_MELODY1_STOP_CON              ( ( P_AUDIO << 8 ) |  15 )
  #endif
  #if (VOICE_MEMO)
    #define MMI_VM_PLAY_START_REQ             ( ( P_AUDIO << 8 ) |  16 )
    #define MMI_VM_PLAY_STOP_REQ              ( ( P_AUDIO << 8 ) |  17 )
    #define MMI_VM_PLAY_START_CON             ( ( P_AUDIO << 8 ) |  18 )
    #define MMI_VM_PLAY_STOP_CON              ( ( P_AUDIO << 8 ) |  19 )

    #define MMI_VM_RECORD_START_REQ           ( ( P_AUDIO << 8 ) |  20 )
    #define MMI_VM_RECORD_STOP_REQ            ( ( P_AUDIO << 8 ) |  21 )
    #define MMI_VM_RECORD_START_CON           ( ( P_AUDIO << 8 ) |  22 )
    #define MMI_VM_RECORD_STOP_CON            ( ( P_AUDIO << 8 ) |  23 )
  #endif
  #if (SPEECH_RECO)
    #define MMI_SR_ENROLL_START_REQ           ( ( P_AUDIO << 8 ) |  24 )
    #define MMI_SR_ENROLL_STOP_REQ            ( ( P_AUDIO << 8 ) |  25 )
    #define MMI_SR_ENROLL_START_CON           ( ( P_AUDIO << 8 ) |  26 )
    #define MMI_SR_ENROLL_STOP_CON            ( ( P_AUDIO << 8 ) |  27 )

    #define MMI_SR_UPDATE_START_REQ           ( ( P_AUDIO << 8 ) |  28 )
    #define MMI_SR_UPDATE_STOP_REQ            ( ( P_AUDIO << 8 ) |  29 )
    #define MMI_SR_UPDATE_START_CON           ( ( P_AUDIO << 8 ) |  30 )
    #define MMI_SR_UPDATE_STOP_CON            ( ( P_AUDIO << 8 ) |  31 )

    #define MMI_SR_RECO_START_REQ             ( ( P_AUDIO << 8 ) |  32 )
    #define MMI_SR_RECO_STOP_REQ              ( ( P_AUDIO << 8 ) |  33 )
    #define MMI_SR_RECO_START_CON             ( ( P_AUDIO << 8 ) |  34 )
    #define MMI_SR_RECO_STOP_CON              ( ( P_AUDIO << 8 ) |  35 )

    #define MMI_SR_UPDATE_CHECK_START_REQ     ( ( P_AUDIO << 8 ) |  36 )
    #define MMI_SR_UPDATE_CHECK_STOP_REQ      ( ( P_AUDIO << 8 ) |  37 )
    #define MMI_SR_UPDATE_CHECK_START_CON     ( ( P_AUDIO << 8 ) |  38 )
    #define MMI_SR_UPDATE_CHECK_STOP_CON      ( ( P_AUDIO << 8 ) |  39 )
  #endif
  #if (AEC)
    #define MMI_AEC_REQ                       ( ( P_AUDIO << 8 ) |  40 )
    #define MMI_AEC_CON                       ( ( P_AUDIO << 8 ) |  41 )
  #endif
  #if (FIR)
    #define MMI_AUDIO_FIR_REQ                 ( ( P_AUDIO << 8 ) |  42 )
    #define MMI_AUDIO_FIR_CON                 ( ( P_AUDIO << 8 ) |  43 )
  #endif
  #if (AUDIO_MODE)
    #define MMI_AUDIO_MODE_REQ                ( ( P_AUDIO << 8 ) |  44 )
    #define MMI_AUDIO_MODE_CON                ( ( P_AUDIO << 8 ) |  45 )
  #endif
  #if (MELODY_E2)
    #define MMI_MELODY0_E2_START_REQ          ( ( P_AUDIO << 8 ) |  46 )
    #define MMI_MELODY0_E2_STOP_REQ           ( ( P_AUDIO << 8 ) |  47 )
    #define MMI_MELODY0_E2_START_CON          ( ( P_AUDIO << 8 ) |  48 )
    #define MMI_MELODY0_E2_STOP_CON           ( ( P_AUDIO << 8 ) |  49 )

    #define MMI_MELODY1_E2_START_REQ          ( ( P_AUDIO << 8 ) |  50 )
    #define MMI_MELODY1_E2_STOP_REQ           ( ( P_AUDIO << 8 ) |  51 )
    #define MMI_MELODY1_E2_START_CON          ( ( P_AUDIO << 8 ) |  52 )
    #define MMI_MELODY1_E2_STOP_CON           ( ( P_AUDIO << 8 ) |  53 )
  #endif
  #if (L1_VOICE_MEMO_AMR)
    #define MMI_VM_AMR_PLAY_START_REQ         ( ( P_AUDIO << 8 ) |  54 )
    #define MMI_VM_AMR_PLAY_STOP_REQ          ( ( P_AUDIO << 8 ) |  55 )
    #define MMI_VM_AMR_PLAY_START_CON         ( ( P_AUDIO << 8 ) |  56 )
    #define MMI_VM_AMR_PLAY_STOP_CON          ( ( P_AUDIO << 8 ) |  57 )

    #define MMI_VM_AMR_RECORD_START_REQ       ( ( P_AUDIO << 8 ) |  58 )
    #define MMI_VM_AMR_RECORD_STOP_REQ        ( ( P_AUDIO << 8 ) |  59 )
    #define MMI_VM_AMR_RECORD_START_CON       ( ( P_AUDIO << 8 ) |  60 )
    #define MMI_VM_AMR_RECORD_STOP_CON        ( ( P_AUDIO << 8 ) |  61 )
  #endif
  #if (L1_CPORT == 1)
    #define MMI_CPORT_CONFIGURE_REQ           ( ( P_AUDIO << 8 ) |   62 )
    #define MMI_CPORT_CONFIGURE_CON           ( ( P_AUDIO << 8 ) |   63 )
  #endif


  // Messages L1S -> L1A
  #if (KEYBEEP)
    #define L1_KEYBEEP_START_CON              ( ( P_AUDIO << 8 ) |  64 )
    #define L1_KEYBEEP_STOP_CON               ( ( P_AUDIO << 8 ) |  65 )
  #endif
  #if (TONE)
    #define L1_TONE_START_CON                 ( ( P_AUDIO << 8 ) |  66 )
    #define L1_TONE_STOP_CON                  ( ( P_AUDIO << 8 ) |  67 )
  #endif
  #if (MELODY_E1)
    #define L1_MELODY0_START_CON              ( ( P_AUDIO << 8 ) |  68 )
    #define L1_MELODY0_STOP_CON               ( ( P_AUDIO << 8 ) |  69 )

    #define L1_MELODY1_START_CON              ( ( P_AUDIO << 8 ) |  70 )
    #define L1_MELODY1_STOP_CON               ( ( P_AUDIO << 8 ) |  71 )
  #endif
  #if (VOICE_MEMO)
    #define L1_VM_PLAY_START_CON              ( ( P_AUDIO << 8 ) |  72 )
    #define L1_VM_PLAY_STOP_CON               ( ( P_AUDIO << 8 ) |  73 )

    #define L1_VM_RECORD_START_CON            ( ( P_AUDIO << 8 ) |  74 )
    #define L1_VM_RECORD_STOP_CON             ( ( P_AUDIO << 8 ) |  75 )
  #endif
  #if (SPEECH_RECO)
    #define L1_SR_ENROLL_START_CON            ( ( P_AUDIO << 8 ) |  76 )
    #define L1_SR_ENROLL_STOP_CON             ( ( P_AUDIO << 8 ) |  77 )

    #define L1_SR_UPDATE_START_CON            ( ( P_AUDIO << 8 ) |  78 )
    #define L1_SR_UPDATE_STOP_CON             ( ( P_AUDIO << 8 ) |  79 )

    #define L1_SR_RECO_START_CON              ( ( P_AUDIO << 8 ) |  80 )
    #define L1_SR_RECO_STOP_CON               ( ( P_AUDIO << 8 ) |  81 )
    #define L1_SR_RECO_STOP_IND               ( ( P_AUDIO << 8 ) |  82 )
    #define L1_SR_PROCESSING_STOP_CON         ( ( P_AUDIO << 8 ) |  83 )
  #endif
  #if (AEC)
    #define L1_AEC_CON                        ( ( P_AUDIO << 8 ) |  84 )
    #define L1_AEC_IND                        ( ( P_AUDIO << 8 ) |  85 )
  #endif
  #if (FIR)
    #define L1_AUDIO_FIR_CON                  ( ( P_AUDIO << 8 ) |  86 )
  #endif
  #if (AUDIO_MODE)
    #define L1_AUDIO_MODE_CON                 ( ( P_AUDIO << 8 ) |  87 )
  #endif
  #if (L1_VOICE_MEMO_AMR)
    #define L1_VM_AMR_PLAY_START_CON          ( ( P_AUDIO << 8 ) |  88 )
    #define L1_VM_AMR_PLAY_STOP_CON           ( ( P_AUDIO << 8 ) |  89 )

    #define L1_VM_AMR_RECORD_START_CON        ( ( P_AUDIO << 8 ) |  90 )
    #define L1_VM_AMR_RECORD_STOP_CON         ( ( P_AUDIO << 8 ) |  91 )
  #endif
  #if (L1_CPORT == 1)
    #define L1_CPORT_CONFIGURE_CON            ( ( P_AUDIO << 8 ) |  92 )
  #endif

  // Messages L1A <-> Audio background
  #if (SPEECH_RECO)
    #define L1_SRBACK_SAVE_DATA_REQ           ( ( P_AUDIO << 8 ) |  93 )
    #define L1_SRBACK_SAVE_DATA_CON           ( ( P_AUDIO << 8 ) |  94 )
    #define L1_SRBACK_LOAD_MODEL_REQ          ( ( P_AUDIO << 8 ) |  95 )
    #define L1_SRBACK_LOAD_MODEL_CON          ( ( P_AUDIO << 8 ) |  96 )
    #define L1_SRBACK_TEMP_SAVE_DATA_REQ      ( ( P_AUDIO << 8 ) |  97 )
    #define L1_SRBACK_TEMP_SAVE_DATA_CON      ( ( P_AUDIO << 8 ) |  98 )
  #endif
  #if (MELODY_E2)
    #define L1_BACK_MELODY_E2_LOAD_INSTRUMENT_REQ   ( ( P_AUDIO << 8 ) |  99 )
    #define L1_BACK_MELODY_E2_LOAD_INSTRUMENT_CON   ( ( P_AUDIO << 8 ) |  100 )
    #define L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_REQ ( ( P_AUDIO << 8 ) |  101 )
    #define L1_BACK_MELODY_E2_UNLOAD_INSTRUMENT_CON ( ( P_AUDIO << 8 ) |  102 )

    #define L1_MELODY0_E2_STOP_CON                  ( ( P_AUDIO << 8 ) |  103 )
    #define L1_MELODY1_E2_STOP_CON                  ( ( P_AUDIO << 8 ) |  104 )
  #endif

  #if (OP_RIV_AUDIO == 1)
    #if (L1_AUDIO_DRIVER == 1)
      #define L1_AUDIO_DRIVER_IND                 ( ( P_AUDIO << 8 ) |  105 )
    #endif
  #endif
#endif // AUDIO_TASK == 1

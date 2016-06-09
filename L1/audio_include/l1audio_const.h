/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_CONST.H
 *
 *        Filename l1audio_const.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)

// DRC API base address
#define C_DRC_API_BASE_ADDRESS 0x161E

  //----------------------------------------
  // LAYER 1 Synchronous audio process name.
  //----------------------------------------

  #define NBR_AUDIO_MANAGER        37  // Number of L1S audio managers

  #define L1S_KEYBEEP_STATE         0   // l1s_keybeep_manager()
  #define L1S_TONE_STATE            1   // l1s_tone_manager()
  #define L1S_MELODY0_STATE         2   // l1s_melody0_manager()
  #define L1S_MELODY1_STATE         3   // l1s_melody1_manager()
  #define L1S_VM_PLAY_STATE         4   // l1s_vm_play_manager()
  #define L1S_VM_RECORD_STATE       5   // l1s_vm_record_manager()
  #define L1S_TONE_UL_STATE         6   // l1s_tone_ul_manager()
  #define L1S_SR_ENROLL_STATE       7   // l1s_sr_enroll_manager()
  #define L1S_SR_UPDATE_STATE       8   // l1s_sr_update_manager()
  #define L1S_SR_RECO_STATE         9   // l1s_sr_reco_manager()
  #define L1S_SR_PROCESSING_STATE  10   // l1s_sr_processing_manager()
  #define L1S_SR_SPEECH_STATE      11   // l1s_sr_speech_manager()
  #define L1S_AEC_STATE            12   // l1s_aec_manager()
  #define L1S_AUDIO_MODE_STATE     13   // l1s_audio_mode_manager()
  #define L1S_MELODY0_E2_STATE     14   // l1s_melody0_e2_manager()
  #define L1S_MELODY1_E2_STATE     15   // l1s_melody1_e2_manager()
  #define L1S_VM_AMR_PLAY_STATE    16   // l1s_vm_amr_play_manager()
  #define L1S_VM_AMR_RECORD_STATE  17   // l1s_vm_amr_record_manager()
  #define L1S_CPORT_STATE          18   // l1s_cport_manager()
  #define L1S_AUDIO_ONOFF_STATE    19   // l1s_audio_onoff_manager()
  #define L1S_STEREOPATH_DRV_STATE 20   // l1s_stereopath_drv_manager()
  #define L1S_MP3_STATE            21   // l1s_mp3_manager()
  #define L1S_ANR_STATE            22   // l1s_anr_manager()
  #define L1S_IIR_STATE            23   // l1s_iir_manager()
  #define L1S_LIMITER_STATE        24   // l1s_limiter_manager()
  #define L1S_ES_STATE             25   // l1s_es_manager()
  #define L1S_MIDI_STATE           26   // l1s_midi_manager()
  #define L1S_AGC_UL_STATE         27   // l1s_agc_ul_manager()
  #define L1S_AGC_DL_STATE         28   // l1s_agc_dl_manager()
  #define L1S_WCM_STATE            29   // l1s_wcm_manager()
  #define L1S_DRC_STATE            30   // l1s_drc_manager()
#if (L1_AAC == 1)

//added from e-sample for AAC
  #define L1S_AAC_STATE            31   // l1s_aac_manager()
#endif
#if (L1_AUDIO_MCU_ONOFF == 1)
  #define L1S_AUDIO_UL_ONOFF_STATE 32  // l1s_audio_ul_onoff_manager()
  #define L1S_AUDIO_DL_ONOFF_STATE 33   // l1s_audio_dl_onoff_manager()
#endif
  #define L1S_PCM_DOWNLOAD_STATE   34   // l1s_pcm_download_manager()
  #define L1S_PCM_UPLOAD_STATE     35   // l1s_pcm_upload_manager()
  #define L1S_FIR_STATE            36   // l1s_fir_manager()


  //----------------------------------------
  // MCU<->DSP communication bit field.
  //----------------------------------------

  // bit in d_tch_mode for audio features
  #define B_VOICE_MEMO_DTX      (TRUE_L << 5)
#if (DSP >= 34)
  #define B_VM_VOCODER_SELECT   (TRUE_L << 6)
#endif

  // bits in d_toneskb_status
  #define B_TONE                (TRUE_L <<  0) // Indicate if the DSP tone task is running
  #define B_KEYBEEP             (TRUE_L <<  1) // Indicate if the DSP Keybeep task is running
  #define B_VM_RECORD_ON_GOING  (TRUE_L <<  2) // Indicate if the DSP recording speech task is running
  #define B_VM_PLAY_ON_GOING    (TRUE_L <<  3) // Indicate if the DSP playing task is running
  #define B_VM_AMR_RECORD_ON_GOING (TRUE_L <<  2) // Indicate if the DSP recording speech amr task is running
  #define B_VM_AMR_PLAY_ON_GOING   (TRUE_L <<  3) // Indicate if the DSP playing amr task is running
  #define B_SR_ENROLL_TASK      (TRUE_L <<  4) // Indicate if the DSP enroll task is running
  #define B_SR_UPDATE_TASK      (TRUE_L <<  5) // Indicate if the DSP update task is running
  #define B_SR_RECO_TASK        (TRUE_L <<  6) // Indicate if the DSP reco task is running
  #define B_SR_PROCESSING_TASK  (TRUE_L <<  7) // Indicate if the DSP processing task is running
  #define B_SR_ALIGNMENT_TASK   (TRUE_L <<  8) // Indicate if the DSP alignment task is running
  #define B_IT_COM_REQ          (TRUE_L <<  9) // Indicate that the DSP requests an IT com for the next TDMA
  #define B_AUDIO_ON_STATUS     (TRUE_L <<  14) // Set to 1 if DSP doesn'tpower OFF audio ABB if no DSP audio activity

  #if(L1_PCM_EXTRACTION)
    #define B_PCM_UPLOAD_ON_GOING    (TRUE_L << 4)
    #define B_PCM_DOWNLOAD_ON_GOING  (TRUE_L << 5)
  #endif

  // bits in d_toneskb_init
  #define B_VM_RECORD_START  (TRUE_L <<  2)   // Start the DSP voice memo recording task
  #define B_VM_RECORD_STOP   (TRUE_L <<  3)   // Stop the DSP voice memo recording task
  #define B_VM_PLAY_START    (TRUE_L <<  4)   // Start the DSP voice memo playing task
  #define B_VM_PLAY_STOP     (TRUE_L <<  5)   // Stop the DSP voice memo playing task
  #define B_VM_TONE_UL       (TRUE_L <<  6)   // Generate the tone on the UL path
  #define B_VM_AMR_RECORD_START (TRUE_L <<  2)// Start the DSP voice memo amr recording task
  #define B_VM_AMR_RECORD_STOP  (TRUE_L <<  3)// Stop the DSP voice memo amr recording task
  #define B_VM_AMR_PLAY_START   (TRUE_L <<  4)// Start the DSP voice memo amr playing task
  #define B_VM_AMR_PLAY_STOP    (TRUE_L <<  5)// Stop the DSP voice memo amr playing task
  #define B_SR_ENROLL        (TRUE_L <<  7)   // Start the DSP speech reco enroll task
  #define B_SR_UPDATE        (TRUE_L <<  8)   // Start the DSP speech reco update task
  #define B_SR_RECO          (TRUE_L <<  9)   // Start the DSP speech reco task
  #define B_SR_PROCESSING    (TRUE_L << 10)   // Start the DSP speech reco processing task
  #define B_SR_STOP          (TRUE_L << 11)   // Stop the current DSP speech reco task

  #define B_MELO             (TRUE_L << 13)   // Start the DSP melody module if it's not started
  #define B_AUDIO_ON_START   (TRUE_L << 14)   // Use to start ABB audio and to not stop it when 
                                              // no audio activity on DSP side
  #define B_AUDIO_OFF_STOP   (TRUE_L << 15)   // Use to stop ABB audio when no audio activity

  #if(L1_PCM_EXTRACTION)
    #define B_PCM_UPLOAD_START     (TRUE_L << 7)
    #define B_PCM_UPLOAD_STOP      (TRUE_L << 8)
    #define B_PCM_DOWNLOAD_START   (TRUE_L << 9)
    #define B_PCM_DOWNLOAD_STOP    (TRUE_L << 10)
  #endif

  #if (L1_CPORT == 1)
    //----------------------------------------
    // C_PORT constant.
    //----------------------------------------
 
    #define CPORT_READ_FLAG_OFFSET   11         // offset in configuration field of the "read register" bit
    #define CPORT_REG_NB_OFFSET      12         // offset in configuration field of the register number
    #define CPORT_READ_MASK          0xF800     // mask to get the read flag and the read reg id in the d_cport_status field

    // write register defines
    #define CPORT_W_NONE      0      // do not write anything
    #define CPORT_W_CTRL      1      // write CTRL, set bit 0 of configuration to 1
    #define CPORT_W_CPCFR1    1 << 1 // write CPCFR1, set bit 1 of configuration to 1
    #define CPORT_W_CPCFR2    1 << 2 // write CPCFR2, set bit 2 of configuration to 1
    #define CPORT_W_CPCFR3    1 << 3 // write CPCFR3, set bit 3 of configuration to 1
    #define CPORT_W_CPCFR4    1 << 4 // write CPCFR4, set bit 4 of configuration to 1
    #define CPORT_W_CPTCTL    1 << 5 // write CPTCTL, set bit 5 of configuration to 1
    #define CPORT_W_CPTTADDR  1 << 6 // write CPTTADDR, set bit 6 of configuration to 1
    #define CPORT_W_CPTDAT    1 << 7 // write CPTDAT, set bit 7 of configuration to 1
    #define CPORT_W_CPTVS     1 << 8 // write CPTVS, set bit 8 of configuration to 1

    // read register defines
    #define CPORT_R_NONE     0       // do not read anything
    // for each of the following defines, set read flag (bit 11) to 1 and set reg_nb (bits 12..15)
    #define CPORT_R_CTRL     (0 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPCFR1   (1 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET)
    #define CPORT_R_CPCFR2   (2 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPCFR3   (3 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPCFR4   (4 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPTCTL   (5 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPTTADDR (6 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPTDAT   (7 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_CPTVS    (8 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 
    #define CPORT_R_STATUS   (9 << CPORT_REG_NB_OFFSET) | (1 << CPORT_READ_FLAG_OFFSET) 

  #endif


  #if (MELODY_E1)
    //----------------------------------------
    // Melody constant.
    //----------------------------------------

    // Word to indicate that the oscillator must be stopped ASAP
    #define SC_END_OSCILLATOR_MASK        0xfffe

    // Description of the ml_ocscil_x field (x= 0...SC_NUMBER_OSCILLATOR)
    #define SC_MELO_OSCILLATOR_USED_MASK  0xff00
    #define SC_MELO_OSCILLATOR_USED_SHIFT SC_NUMBER_OSCILLATOR

    //  Description of the ml_time_offset field
    #define SC_MELO_TIME_OFFSET_MASK      0x00ff
    #define SC_MELO_TIME_OFFSET_SHIFT     0

    // Description of the ml_load1 bit
    #define SC_MELO_LOAD1_MASK            0x0010
    #define SC_MELO_LOAD1_SHIFT           4

    // Description of the ml_load2 bit
    #define SC_MELO_LOAD2_MASK            0x0020
    #define SC_MELO_LOAD2_SHIFT           5

    // Description of the ml_synchro bit
    #define SC_MELO_SYNCHRO_MASK          0x0001
    #define SC_MELO_SYNCHRO_SHIFT         0

    // Description of the ml_length field
    #define SC_MELO_LENGTH_MASK           0xffc0
    #define SC_MELO_LENGTH_SHIFT          6
  #endif // MELODY_E1

  #if (VOICE_MEMO) || (SPEECH_RECO)
    //----------------------------------------
    // Voice memo constant.
    //----------------------------------------

    // Communication DSP<->MCU via the a_du_x buffer:

    // Mask for the bit to indicate:
    // in VM play: if the DSP requests a new block
    // in VM record: if the DSP has a new block
    #define B_BLOCK_READY         (TRUE_L<<10)

    // Mask for the bit of the a_du_x buffer to indicate if the block is the speech or noise
    #define B_VM_SPEECH           (TRUE_L<<15)

    // Size of the a_du_x buffer when the sample is a noise:
    #define SC_VM_NOISE_SAMPLE    1

    // Size of the a_du_x buffer when the sample is a speech:
    #define SC_VM_SPEECH_SAMPLE   20
  #endif // VOICE_MEMO || SPEECH_RECO

  #if (L1_PCM_EXTRACTION)
    #define B_PCM_DOWNLOAD_READY    (TRUE_L << 0)
    #define B_PCM_UPLOAD_READY      (TRUE_L << 0)
    #define SC_PCM_DOWNLOAD_SAMPLE  160
    #define SC_PCM_UPLOAD_SAMPLE    160
    #define B_PCM_UPLOAD_ERROR      (TRUE_L << 0)
    #define B_PCM_DOWNLOAD_ERROR    (TRUE_L << 1)
  #endif /* L1_PCM_EXTRACTION */

  #if (L1_VOICE_MEMO_AMR)
    //----------------------------------------
    // Voice memo amr constant.
    //----------------------------------------

    #define SC_VM_AMR_HEADER_SIZE    1

    // Communication DSP<->MCU via the a_du_x buffer:
    #define SC_RX_TX_TYPE_MASK   (7<<3)

    // Communication DSP<->MCU via d_amms_ul_voc and b_amms_channel_type
    #define SC_CHAN_TYPE_MASK         7

    // RX_TYPE or TX_TYPE (See 06.93)
    #define SC_VM_AMR_RXTX_SPEECH_GOOD     (0<<3)
    #define SC_VM_AMR_RXTX_SPEECH_DEGRADED (1<<3)
    #define SC_VM_AMR_RXTX_ONSET           (2<<3)
    #define SC_VM_AMR_RXTX_SPEECH_BAD      (3<<3)
    #define SC_VM_AMR_RXTX_SID_FIRST       (4<<3)
    #define SC_VM_AMR_RXTX_SID_UPDATE      (5<<3)
    #define SC_VM_AMR_RXTX_SID_BAD         (6<<3)
    #define SC_VM_AMR_RXTX_NO_DATA         (7<<3)

    // sample type for ONSET insertion in NO_SPEECH to SPEECH transition
    #define SC_VM_AMR_SPEECH          0
    #define SC_VM_AMR_NOISE           1
    #define SC_VM_AMR_NO_DATA         2
    #define SC_VM_AMR_ONSET           3

    // Speech channel type
    #define SC_VM_AMR_SPEECH_475      0
    #define SC_VM_AMR_SPEECH_515      1
    #define SC_VM_AMR_SPEECH_59       2
    #define SC_VM_AMR_SPEECH_67       3
    #define SC_VM_AMR_SPEECH_74       4
    #define SC_VM_AMR_SPEECH_795      5
    #define SC_VM_AMR_SPEECH_102      6
    #define SC_VM_AMR_SPEECH_122      7

    // Size of data bits in the a_du_x buffer when the sample is SPEECH
    // a_du_x buffer contains header + 2 non-used words after header + data_bits => recorded size is DATA_SIZE + 1
    #define SC_VM_AMR_SPEECH_475_DATA_SIZE   12
    #define SC_VM_AMR_SPEECH_515_DATA_SIZE   13
    #define SC_VM_AMR_SPEECH_59_DATA_SIZE    15
    #define SC_VM_AMR_SPEECH_67_DATA_SIZE    17
    #define SC_VM_AMR_SPEECH_74_DATA_SIZE    19
    #define SC_VM_AMR_SPEECH_795_DATA_SIZE   20
    #define SC_VM_AMR_SPEECH_102_DATA_SIZE   26
    #define SC_VM_AMR_SPEECH_122_DATA_SIZE   31

    // Size of the a_du_x buffer when the sample is SID_FIRST:
    #define SC_VM_AMR_SID_FIRST_DATA_SIZE    5

    // Size of the a_du_x buffer when the sample is SID_UPDATE:
    #define SC_VM_AMR_SID_UPDATE_DATA_SIZE   5

    // Size of the a_du_x buffer when the sample is SID_BAD:
    #define SC_VM_AMR_SID_BAD_DATA_SIZE      5

    // Size of the a_du_x buffer when the sample is NO_DATA:
    #define SC_VM_AMR_NO_DATA_DATA_SIZE      0

    // Size of the a_du_x buffer when the sample is ONSET:
    #define SC_VM_AMR_ONSET_DATA_SIZE        0

  #endif // L1_VOICE_MEMO_AMR

  #if (SPEECH_RECO)
    //----------------------------------------
    // Speech recognition constant.
    //----------------------------------------

    // d_sr_status bit field
    #define B_BAD_ACQUISITION     (TRUE_L <<  8)
    #define B_GOOD_ACQUISITION    (TRUE_L <<  9)
    #define B_BAD_UPDATE          (TRUE_L << 10)
    #define B_GOOD_UPDATE         (TRUE_L << 11)

    // d_sr_status VAD indication
    #define SC_SR_WORD_MASK       0x00FF
    #define SC_SR_WORD_SEARCHING  0
    #define SC_SR_WORD_BEGINNING  1
    #define SC_SR_WORD_ON_GOING   2
    #define SC_SR_WORD_ENDING     3
    #define SC_SR_WORD_DONE       4

  #endif // SPEECH_RECO

  #if (L1_AEC == 1)
    #define B_AEC_ACK        (TRUE_L <<  0)  // Bit set by the MCU to indicate a new AEC settings and
                                             // clear by the DSP to confirm the new settings.
    #if (L1_NEW_AEC)
      #define B_AEC_VISIBILITY (TRUE_L <<  9)  // Bit set by the MCU to have internal output values of AEC copied in API
      #define SC_AEC_VISIBILITY_SHIFT (9)
    #endif

  #endif


  #if (W_A_DSP_SR_BGD)
    #define C_BGD_RECOGN  5 // TEMPORARY: DSP Background recognition task code (also used for bitmaps).
    #define C_BGD_ALIGN   6 // TEMPORARY: DSP Background alignement
    // bits in d_gsm_bgd_mgt - background task management
    #define B_DSPBGD_RECO           1       // start of reco in dsp background
    #define B_DSPBGD_UPD            2       // start of alignement update in dsp background
  #endif
  #if (AUDIO_MODE)
    #define B_GSM_ONLY        ((TRUE_L <<  13) | (TRUE_L <<  11))   // GSM normal mode
    #define B_BT_CORDLESS     (TRUE_L <<  12)   // Bluetooth cordless mode
    #define B_BT_HEADSET      (TRUE_L <<  14)   // Bluetooth headset mode
  #endif

  #define SC_AUDIO_MCU_API_BEGIN_ADDRESS  (0xFFD00000)    // Start address of the API memory in MCU side
  #define SC_AUDIO_DSP_API_BEGIN_ADDRESS  (0x0800)        // Start address o fthe API memory in DSP side

  #if (MELODY_E2)
    // Number of oscillator available in the melody E2
    #define SC_MELODY_E2_NUMBER_OF_OSCILLATOR       (16)

    // Code of extension for data=time
    #define SC_TIME_CODE_OF_EXTENSION               (1)

    // Position of the GlobalTimeFactor parameter
    #define SC_MELODY_E2_GLOBALTIMEFACTOR_MASK      (0xFF)
    #define SC_MELODY_E2_GLOBALTIMEFACTOR_SHIFT     (0)

    // Position of the Number of instrument parameter
    #define SC_MELODY_E2_NUMBEROFINSTRUMENT_MASK    (0xFF)
    #define SC_MELODY_E2_NUMBEROFINSTRUMENT_SHIFT   (0)

    // Position of the extension flag
    #define SC_MELODY_E2_EXTENSION_FLAG_MASK        (0x80)
    #define SC_MELODY_E2_EXTENSION_FLAG_SHIFT       (7)

    // Position of the code of extension
    #define SC_MELODY_E2_CODE_OF_EXTENSION_MASK     (0x70)
    #define SC_MELODY_E2_CODE_OF_EXTENSION_SHIFT    (4)

    // Position of data time
    #define SC_MELODY_E2_DATA_TIME_MSB_MASK         (0x07)
    #define SC_MELODY_E2_DATA_TIME_MSB_SHIFT        (0)

    // Position of data time
    #define SC_MELODY_E2_DATA_TIME_LSB_MASK         (0xFF00)
    #define SC_MELODY_E2_DATA_TIME_LSB_SHIFT        (8)

    // Position of the Delta Time
    #define SC_MELODY_E2_DELTA_TIME_MASK            (0x7F)
    #define SC_MELODY_E2_DELTA_TIME_SHIFT           (0)

    // Mask of the semaphore
    #define SC_MELODY_E2_SEMAPHORE_MASK             (0x0001)

    // Maximum size of the header of the melody E2
    #define SC_MELODY_E2_MAXIMUM_HEADER_SIZE        (3 + SC_AUDIO_MELODY_E2_MAX_NUMBER_OF_INSTRUMENT)

    // Maximum number of extension
    #define SC_MELODY_E2_MAXIMUM_NUMBER_OF_EXTENSION  (2)
  #endif // MELODY_E2

  // Selection of the melody format
  #define NO_MELODY_SELECTED    (0)
  #define MELODY_E0_SELECTED    (1)
  #define MELODY_E1_SELECTED    (2)
  #define MELODY_E2_SELECTED    (3)

  #if (L1_STEREOPATH == 1)
    // configuration
    #define AUDIO_SP_SELF_CONF      0
    #define AUDIO_SP_MIDI_CONF      1
    #define AUDIO_SP_MP3_CONF       2
    #define AUDIO_SP_EXTAUDIO_CONF  3

#if (ANALOG == 3)
    // sampling frequency 
    #define AUDIO_SP_FREQ_8         7
    #define AUDIO_SP_FREQ_11        6
    #define AUDIO_SP_FREQ_16        5
    #define AUDIO_SP_FREQ_22        4
    #define AUDIO_SP_FREQ_32        3
    #define AUDIO_SP_FREQ_44        2
    #define AUDIO_SP_FREQ_48        0
#endif

#if (ANALOG == 11 || CODE_VERSION == SIMULATION)
 // sampling frequency index for Triton. This would be set in the 
 //SRW[0:3] bits of the CTRL5 register.
    #define AUDIO_SP_FREQ_8           0
    #define AUDIO_SP_FREQ_11         1
    #define AUDIO_SP_FREQ_12         2
    #define AUDIO_SP_FREQ_16         3
    #define AUDIO_SP_FREQ_22         4
    #define AUDIO_SP_FREQ_24         5
    #define AUDIO_SP_FREQ_32         6
    #define AUDIO_SP_FREQ_44         7
    #define AUDIO_SP_FREQ_48         8
#endif

    // DMA allocation
    #define AUDIO_SP_DMA_ALLOC_MCU  0
    #define AUDIO_SP_DMA_ALLOC_DSP  1

    // Data type
    #define AUDIO_SP_DATA_S8        0
    #define AUDIO_SP_DATA_S16       1
    #define AUDIO_SP_DATA_S32       2

    // Source port
    #define AUDIO_SP_SOURCE_IMIF    0
    #define AUDIO_SP_SOURCE_API     2
#if (CHIPSET == 15)
    //In Locosto, EMIF is also a source port for the audio
    #define AUDIO_SP_SOURCE_EMIF   4
#endif

    // output type
    #define AUDIO_SP_MONO_OUTPUT    0
    #define AUDIO_SP_STEREO_OUTPUT  1

    // feature identifier
    #define AUDIO_SP_MIDI_ID        0
    #define AUDIO_SP_EXT_AUDIO_ID   1
    #define AUDIO_SP_MP3_ID         2
    #define AUDIO_SP_TESTS_ID       3
#if (L1_AAC == 1)
    #define AUDIO_SP_AAC_ID         4   //added for AAC
#endif

    // Pattern
    #define AUDIO_SP_SILENCE_PATTERN 0
    #define AUDIO_SP_SINUS1_PATTERN  1
    #define AUDIO_SP_SINUS2_PATTERN  2
    #define AUDIO_SP_SINUS3_PATTERN  3

  #endif

#endif // AUDIO_TASK

#if (DSP == 17) || (DSP == 32)
  #define B_FIR_START  (TRUE_L <<  0)  // Bit set by the MCU to start the FIR task for the DSP code 32 and 17.
#endif

#define B_FIR_LOOP  (TRUE_L <<  1)  // Bit set by the MCU to close the loop between the audio UL and DL path.
                                    // This features is used to find the FIR coefficient.

/*************************************/
/* ACOUSTIC interface                */
/*************************************/

/* Control values */
/*----------------*/

#define B_ANR_ENABLE      (TRUE_L << 0)
#define B_ANR_DISABLE     (TRUE_L << 1)
#define B_ANR_FULL_UPDATE (TRUE_L << 2)

#define B_IIR_ENABLE      (TRUE_L << 0)
#define B_IIR_DISABLE     (TRUE_L << 1)
#define B_IIR_FULL_UPDATE (TRUE_L << 2)

#define B_AGC_ENABLE      (TRUE_L << 0)
#define B_AGC_DISABLE     (TRUE_L << 1)
#define B_AGC_FULL_UPDATE (TRUE_L << 2)

#define B_WCM_ENABLE      (TRUE_L << 0)
#define B_WCM_DISABLE     (TRUE_L << 1)
#define B_WCM_FULL_UPDATE (TRUE_L << 2)

#if (L1_DRC == 1)
#define DRC_LPF_LENGTH  17
#define DRC_BPF_LENGTH  17 
#endif

#define B_DRC_ENABLE      (TRUE_L << 0)
#define B_DRC_DISABLE     (TRUE_L << 1)
#define B_DRC_FULL_UPDATE (TRUE_L << 2)

#define B_LIM_ENABLE      (TRUE_L << 0)
#define B_LIM_DISABLE     (TRUE_L << 1)
#define B_LIM_FULL_UPDATE (TRUE_L << 2)
#define B_LIM_UPDATE      (TRUE_L << 3)

#define B_ES_ENABLE       (TRUE_L << 0)
#define B_ES_DISABLE      (TRUE_L << 1)
#define B_ES_FULL_UPDATE  (TRUE_L << 2)

#if (L1_IIR == 2)
#define IIR_4X_ORDER_OF_SECTION      2
#define IIR_4X_FIR_MAX_LENGTH       40
#endif

#if (L1_WCM == 1)
#define  WCM_1X_GAIN_TABLE_LENGTH  16
#endif

/* d_aqi_status bits */
/*-------------------*/
#if (DSP == 38) || (DSP == 39)
#define B_DRC_DL_STATE    (TRUE_L <<  3)
#define B_IIR_DL_STATE    (TRUE_L <<  4)
#define B_LIM_STATE       (TRUE_L <<  5)
#define B_AGC_DL_STATE    (TRUE_L <<  6)
#define B_AEC_STATE       (TRUE_L <<  11)
#define B_ANR_UL_STATE    (TRUE_L <<  12)
#define B_ES_STATE        (TRUE_L <<  13)
#define B_AGC_UL_STATE    (TRUE_L <<  14)
#else
#define B_IIR_STATE       (TRUE_L <<  4)
#define B_LIM_STATE       (TRUE_L <<  5)
#define B_ANR_STATE       (TRUE_L << 12)
#define B_ES_STATE        (TRUE_L << 13)
#endif

/* d_audio_apps_status bits */
/*-------------------*/
#if (DSP == 38) || (DSP == 39)
#define B_WCM_STATE    (TRUE_L <<  0)
#endif

/*****************/
/* ANR constants */
/*****************/
#if (L1_ANR == 1)
#define C_ANR_VAD_THR            0x1333
#define C_ANR_GAMMA_SLOW         0x7F1B
#define C_ANR_GAMMA_FAST         0x75C3
#define C_ANR_GAMMA_GAIN_SLOW    28836
#define C_ANR_GAMMA_GAIN_FAST    32113
#define C_ANR_THR2               0x01E0
#define C_ANR_THR4               0x03DE
#define C_ANR_THR5               0x012C
#define C_ANR_MEAN_RATIO_THR1    10000
#define C_ANR_MEAN_RATIO_THR2    6000
#define C_ANR_MEAN_RATIO_THR3    5000
#define C_ANR_MEAN_RATIO_THR4    4000
#endif


#if (L1_ES == 1)

/*******************/
/* ES constants    */
/*******************/

// ES custom mode
#define ES_CUSTOM_PARAM 0xFF

// ES mode definition
#define B_ES_UL             (0x1 << 0)   // ES UL enable
#define B_ES_DL             (0x1 << 1)   // ES DL enable
#define B_ES_CNG            (0x1 << 2)   // CNG enable
#define B_ES_NSF            (0x1 << 3)   // NSF enable
#define B_ES_ALS_UL         (0x1 << 4)   // ALS UL enable
#define B_ES_ALS_DL         (0x1 << 5)   // ALS DL enable

// ES parameter values
#define C_ES_GAIN_DL_OFF    0x0
#define C_ES_GAIN_UL_1_OFF  0x0
#define C_ES_GAIN_UL_2_OFF  0x0

#define C_ES_TCL_M23DB      0x005E    // ref. -23dB
#define C_ES_TCL_M6DB       0x0176    // ref. -6dB
#define C_ES_TCL_M3DB       0x02EA    // ref. -6dB
#define C_ES_TCL_0DB        0x05D1    // ref. 0dB
#define C_ES_TCL_4DB        0x0E9D    // ref. 4dB
#define C_ES_TCL_6DB        0x1729    // ref. 6dB
#define C_ES_TCL_10DB       0x3A2F    // ref. 10dB
#define C_ES_TCL_12DB       0x5C36    // ref. 12dB
#define C_ES_TCL_B3_FE_LS   0x0004    // ref. -10dB
#define C_ES_TCL_B3_DT_LS   0x0001    // ref. -16dB
#define C_ES_TCL_B3_FE_NS   0x000F    // ref. -10dB
#define C_ES_TCL_B3_DT_NS   0x0004    // ref. -16dB
#define C_ES_TCL_B3_NE      0x000F    // ref. 0dB

#define C_ES_TCL_FE_0       0x05D1    // ref. 0dB
#define C_ES_TCL_DT_0       0x05D1    // ref. 0dB
#define C_ES_TCL_NE_0       0x05D1    // ref. 0dB

#define C_ES_TCL_LOUD       0x00BF    // ref. 70dB

#define C_ES_SW_CNT         5         // ref. switching time 100ms
#define C_ES_DT_CNT         5         // ref. double-talk time 100ms

#define C_ES_HG_CNT_1       8         // ref. hangover time 150ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_1A      8         // ref. hangover time 150ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_2A      10        // ref. hangover time 200ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_2B      13        // ref. hangover time 250ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_2C      13        // ref. hangover time 250ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_2C_IDLE 13        // ref. hangover time 250ms, A_S(IDLE) = -3dB
#define C_ES_HG_CNT_3       20        // ref. hangover time 400ms

#define C_ES_ATT_LIN_0DB    0x7fff    // ref.  0dB
#define C_ES_ATT_LIN_3DB    0x5A9D    // ref. -3dB
#define C_ES_ATT_LIN_5DB    0x47FA    // ref. -5dB
#define C_ES_ATT_LIN_6DB    0x4026    // ref. -6dB
#define C_ES_ATT_LIN_8DB    0x32F4    // ref. -8dB
#define C_ES_ATT_LIN_9DB    0x2D6A    // ref. -9dB
#define C_ES_ATT_LIN_10DB   0x2879    // ref. -10dB
#define C_ES_ATT_LIN_12DB   0x2026    // ref. -12dB
#define C_ES_ATT_LIN_15DB   0x16C2    // ref. -15dB
#define C_ES_ATT_LIN_16DB   0x1449    // ref. -16dB
#define C_ES_ATT_LIN_19DB   0x0E5D    // ref. -19dB
#define C_ES_ATT_LIN_21DB   0x0B68    // ref. -21dB
#define C_ES_ATT_LIN_24DB   0x0813    // ref. -24dB
#define C_ES_ATT_LIN_36DB   0x0207    // ref. -36dB
#define C_ES_ATT_LIN_48DB   0x0082    // ref. -48dB
#define C_ES_ATT_LIN_66DB   0x0010    // ref. -56dB

#endif    // L1_ES
#if(L1_EXT_AUDIO_MGT == 1)
#define AUDIO_EXT_MIDI_BUFFER_SIZE 2640
#endif

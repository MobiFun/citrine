 /************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_STEREO_H
 *
 *        Filename l1audio_stereo.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/


#if (L1_STEREOPATH == 1)
  



  //////////////////////////////////////////////////////
  //              VARIABLES DEFINITIONS               //
  //////////////////////////////////////////////////////

  #if (TESTMODE && (OP_L1_STANDALONE == 1))

    #define C_STP_DRV_API_BASE_ADDRESS      0x1AD0
    #define C_STP_DRV_BUF_API_BASE_ADDRESS  0x1AD8
    #define STEREOPATH_MAX_NB_OF_FRAMES     1152
    #define NB_MAX_STEREOPATH_CONFIG        4
    #define C_BGD_STP_DRV                   9

    /***************************************************************************************/
    /* AUDIO STEREOPATH MCU-DSP API                                                            */
    /***************************************************************************************/
    typedef struct
    {
      API d_cport_api_dma_install;
      API d_cport_api_dma_channel;
      API d_cport_api_dma_rootcause;
    } T_STP_DRV_MCU_DSP;

    #if (CODE_VERSION != SIMULATION)
    #ifdef _L1AUDIO_STEREO_C_
  
      /***************************************************************************
       *                      AUDIO_SP_MIDI_CONF configuration
       **************************************************************************/
      const T_TMODE_AUDIO_STEREOPATH_START_REQ tmode_audio_sp_midi_conf = 
          {
           AUDIO_SP_MIDI_CONF,       //   configuration
           AUDIO_SP_FREQ_22,         //   sampling_frequency
           AUDIO_SP_DMA_ALLOC_DSP,   //   DMA_allocation
           2,                        //   DMA_channel_number
           AUDIO_SP_DATA_S16,        //   data_type
           AUDIO_SP_SOURCE_API,      //   source_port
           2,                        //   element_number
           864,                      //   frame_number
           AUDIO_SP_STEREO_OUTPUT,   //   mono_stereo
           AUDIO_SP_SINUS2_PATTERN   //   pattern_identifier
          };

      /***************************************************************************
       *                      AUDIO_SP_MP3_CONF configuration
       **************************************************************************/
      const T_TMODE_AUDIO_STEREOPATH_START_REQ tmode_audio_sp_mp3_conf = 
          {
           AUDIO_SP_MP3_CONF,        //   configuration
           AUDIO_SP_FREQ_48,         //   sampling_frequency
           AUDIO_SP_DMA_ALLOC_DSP,   //   DMA_allocation
           2,                        //   DMA_channel_number
           AUDIO_SP_DATA_S16,        //   data_type
           AUDIO_SP_SOURCE_API,      //   source_port
           2,                        //   element_number
           1152,                     //   frame_number
           AUDIO_SP_STEREO_OUTPUT,   //   mono_stereo
           AUDIO_SP_SINUS1_PATTERN   //   pattern_identifier
          };

      /***************************************************************************
       *                      AUDIO_SP_EXTAUDIO_CONF configuration
       **************************************************************************/

      const T_TMODE_AUDIO_STEREOPATH_START_REQ tmode_audio_sp_extaudio_conf = 
          {
           AUDIO_SP_EXTAUDIO_CONF,   //   configuration
           AUDIO_SP_FREQ_22,         //   sampling_frequency
           AUDIO_SP_DMA_ALLOC_MCU,   //   DMA_allocation
           2,                        //   DMA_channel_number
           AUDIO_SP_DATA_S16,        //   data_type
           AUDIO_SP_SOURCE_IMIF,     //   source_port
           2,                        //   element_number
           1100,                     //   frame_number
           AUDIO_SP_MONO_OUTPUT,     //   mono_stereo
           AUDIO_SP_SINUS3_PATTERN   //   pattern_identifier
          };
                                      

      const T_TMODE_AUDIO_STEREOPATH_START_REQ * tmode_audio_sp_conf[NB_MAX_STEREOPATH_CONFIG] = {
                          &tmode_audio_sp_midi_conf,        
                          &tmode_audio_sp_mp3_conf,         
                          &tmode_audio_sp_extaudio_conf,        
                          (T_TMODE_AUDIO_STEREOPATH_START_REQ *) NULL,
            };

    #else
       extern const T_TMODE_AUDIO_STEREOPATH_START_REQ * tmode_audio_sp_conf[NB_MAX_STEREOPATH_CONFIG];
    #endif // _L1AUDIO_STEREO_C_
    #endif // CODE_VERSION
  #endif // TESTMODE

  //////////////////////////////////////////////////////
  //              CONSTANTS DEFINITIONS               //
  //////////////////////////////////////////////////////

  #define L1S_STEREOPATH_DRV_WAIT_PLL_COUNTER 3  

  //////////////////////////////////////////////////////
  //              FUNCTIONS PROTOTYPES                //
  //////////////////////////////////////////////////////
#if (CODE_VERSION == NOT_SIMULATION)
  extern void l1s_stereopath_drv_config_ABB(UWORD8 mono_stereo,UWORD8 sampling_frequency);
  extern void l1s_stereopath_drv_start_ABB(void);
  extern void l1s_stereopath_drv_stop_ABB(void);
#if ((CHIPSET == 12) || (CHIPSET == 15))
  extern void l1s_stereopath_drv_start_DMA(T_DMA_TYPE_CHANNEL_PARAMETER d_dma_channel_parameter,UWORD8 DMA_allocation);
  extern void l1s_stereopath_drv_reset_DMA(T_DMA_TYPE_CHANNEL_PARAMETER d_dma_channel_parameter);
#endif
  extern void l1s_stereopath_drv_reset_CPORT(void);
  extern void l1s_stereopath_drv_stop_CPORT(void);
  extern void l1s_stereopath_drv_config_CPORT(void);
  extern void l1s_stereopath_drv_start_CPORT(void);

  enum L1S_STEREOPATH_CALLBACK_CONSTANT
  {
    L1S_TWL3029_STEROPATH_CONFIG =10,
    L1S_TWL3029_STEROPATH_STOP,
    L1S_TWL3029_STEROPATH_START,
    L1S_TWL3029_STEROPATH_OUTEN_CONFIG
  };

  //Add the call back function of the stereo path.
  void l1s_stereopath_callback(UWORD8 cbvalue); 
  
#endif    // CODE_VERSION == NOT_SIMULATION
 
#endif /* L1_STEREOPATH == 1 */

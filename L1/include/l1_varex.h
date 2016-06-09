/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_VAREX.H
 *
 *        Filename l1_varex.h
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#ifdef L1_ASYNC_C

 T_L1S_GLOBAL   l1s __attribute__ ((section (".l1s_global")));
 T_L1A_GLOBAL   l1a;

 T_L1A_L1S_COM  l1a_l1s_com __attribute__ ((section (".l1s_global")));
 T_L1S_DSP_COM  l1s_dsp_com __attribute__ ((section (".l1s_global")));
 T_L1S_TPU_COM  l1s_tpu_com __attribute__ ((section (".l1s_global")));
#if (L1_MP3 == 1) || (L1_MIDI == 1 || (L1_AAC == 1) || (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1) || (L1_DYN_DSP_DWNLD == 1))    // equivalent to an API_HISR flag
 T_L1A_API_HISR_COM  l1a_apihisr_com __attribute__ ((section (".l1s_global")));
#endif
#if (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1)
 T_L1_API_HISR       l1_apihisr __attribute__ ((section (".l1s_global")));
#endif
#if (L1_MIDI == 1)
 T_MIDIPARSER_APIHISR_COM midiparser_apihisr_com
	__attribute__ ((section (".l1s_global")));
#endif

 // variables for L1 configuration
 T_L1_CONFIG    l1_config __attribute__ ((section (".l1s_global")));
#else
 extern T_L1S_GLOBAL   l1s;
 extern T_L1A_GLOBAL   l1a;

 extern T_L1A_L1S_COM  l1a_l1s_com;
 extern T_L1S_DSP_COM  l1s_dsp_com;
 extern T_L1S_TPU_COM  l1s_tpu_com;
#if (L1_MP3 == 1) || (L1_MIDI == 1)  || (L1_AAC == 1) || (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1) || (L1_DYN_DSP_DWNLD == 1)   // equivalent to an API_HISR flag
 extern T_L1A_API_HISR_COM    l1a_apihisr_com;
#endif
#if (L1_MP3 == 1) || (L1_MIDI == 1)  || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1)
 extern T_L1_API_HISR      l1_apihisr;
#endif
#if (L1_MIDI == 1)
 extern T_MIDIPARSER_APIHISR_COM midiparser_apihisr_com;
#endif

 // variables for L1 configuration
 extern T_L1_CONFIG    l1_config;
#endif

#if (L1_FF_MULTIBAND == 1)
//    extern T_L1_MULTIBAND_POWER_CLASS multiband_power_class_array[];
//    extern T_MULTIBAND_CONVERSION_DATA multiband_conversion_data[];
//    extern T_MULTIBAND_RF_DATA multiband_rf_data[];
      extern const T_MULTIBAND_RF multiband_rf[RF_NB_SUPPORTED_BANDS + 1];
#endif /*if(L1_FF_MULTIBAND == 1)*/



extern const UWORD8 ramBootCode[]; // dummy DSP code for boot.


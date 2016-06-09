/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_VAREX.H
 *
 *        Filename l1_varex.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#ifdef L1_ASYNC_C

#if (LONG_JUMP == 3)
 #pragma DATA_SECTION(l1s,".l1s_global")
 #pragma DATA_SECTION(l1s_dsp_com,".l1s_global")
 #pragma DATA_SECTION(l1a_l1s_com,".l1s_global")
 #pragma DATA_SECTION(l1s_tpu_com,".l1s_global")
 #pragma DATA_SECTION(l1_config,".l1s_global")
#endif

 T_L1S_GLOBAL   l1s;
 T_L1A_GLOBAL   l1a;

 T_L1A_L1S_COM  l1a_l1s_com;
 T_L1S_DSP_COM  l1s_dsp_com;
 T_L1S_TPU_COM  l1s_tpu_com;

 #if (L1_DYN_DSP_DWNLD == 1)    // equivalent to an API_HISR flag
 T_L1_API_HISR       l1_apihisr;
 T_L1A_API_HISR_COM l1a_apihisr_com;
#endif

 // variables for L1 configuration
 T_L1_CONFIG    l1_config;

#else  // L1_ASYNC_C

 extern T_L1S_GLOBAL   l1s;
 extern T_L1A_GLOBAL   l1a;

 extern T_L1A_L1S_COM  l1a_l1s_com;
 extern T_L1S_DSP_COM  l1s_dsp_com;
 extern T_L1S_TPU_COM  l1s_tpu_com;

 #if (L1_DYN_DSP_DWNLD == 1)    // equivalent to an API_HISR flag
 extern T_L1_API_HISR      l1_apihisr;
 extern T_L1A_API_HISR_COM l1a_apihisr_com;
#endif

 // variables for L1 configuration
 extern T_L1_CONFIG    l1_config;
#endif


extern const UWORD8 ramBootCode[]; // dummy DSP code for boot.
  


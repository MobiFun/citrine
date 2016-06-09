/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1AUDIO_VAREX.H
 *
 *        Filename l1audio_varex.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)
  #if (SPEECH_RECO)
    #ifdef L1_SRBACK_COM 
      T_L1_SRBACK_COM l1_srback_com;
    #else
      extern T_L1_SRBACK_COM l1_srback_com;
    #endif  
  #endif
  #if (MELODY_E2)
    #ifdef L1_AUDIOBACK_MELODYE2
      T_AUDIO_BACK_MELODY_E2 audioback_melody_e2;
    #else
      extern T_AUDIO_BACK_MELODY_E2 audioback_melody_e2;
    #endif  
  #endif
#endif // AUDIO_TASK == 1

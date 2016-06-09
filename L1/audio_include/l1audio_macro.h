/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1AUDIO_CONST.H
 *
 *        Filename l1audio_macro.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)

  /************************************************************/
  /* Macros to obtain a value put among a register            */
  /************************************************************/
  #define Field(value, mask, shift) (((value) & (mask)) >> shift)



#endif // AUDIO_TASK

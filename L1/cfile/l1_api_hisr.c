/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_API_HISR.C
 *
 *        Filename l1_api_hisr.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#include "l1_confg.h"
#include "nucleus.h"
#include "sys_types.h"
#include "l1_types.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1audio_const.h"
#include "l1_const.h"
#include "cust_os.h"
#include "l1tm_defty.h"
#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif
#if (L1_MP3 == 1)
  #include "l1mp3_const.h"
  #include "l1mp3_signa.h"
  #include "l1mp3_defty.h"
  #include "l1mp3_proto.h"
#endif
#if (L1_MIDI == 1)
  #include "l1midi_const.h"
  #include "l1midi_signa.h"
  #include "l1midi_defty.h"
  #include "l1midi_proto.h"
#endif
#if (L1_AAC == 1)
  #include "l1aac_const.h"
  #include "l1aac_signa.h"
  #include "l1aac_defty.h"
  #include "l1aac_proto.h"
#endif
#if (L1_DYN_DSP_DWNLD==1)
  #include "l1_dyn_dwl_defty.h"
  #include "l1_dyn_dwl_msgty.h"
  #include "l1_dyn_dwl_const.h"
  #include "l1_dyn_dwl_signa.h"
  #include "l1_dyn_dwl_error.h"
  #include "l1_dyn_dwl_proto.h" 
  #include "l1_api_hisr.h"
#endif
#include "l1_defty.h"
#include "l1_api_hisr.h"
#include "l1audio_signa.h"
#include "l1_varex.h"
#include "l1_macro.h"
#include "mem.h"


/*-------------------------------------------------------------*/
/* l1_api_handler()                                            */
/*-------------------------------------------------------------*/
/*                                                             */
/* Parameters : none                                           */
/*                                                             */
/* Return     : n/a                                            */
/*                                                             */
/* Description : API interrupt handler, multiplexed for:       */
/*               1) audio sources: MP3 or AAC                 */
/*               2) modem sources: USF                         */
/*                                                             */
// The API Interrupt is multiplexed between (DYN DWNLD, MP3 ,AAC) and FAST USF. 
// DYN DWNLD, MP3 and AAC are mutually exclusive and hence only one would be
// active at one time. But FAST USF would co-exist with any of the them and hence
// there needs to be a way to distinguish the two sources of the API Interrupt. A flag
// d_dsp_hint_flag is used by the DSP to signify the source of the interrupt. The bit 
// configuration of d_dsp_hint_flag is 
// 15 14 13 12 | 11 10 9 8 | 7 6 5 4 | 3 2 1 0 
//                                                                 | |____> Fast USF
//                                                                 |______> (DYN DWNLD, MP3, AAC)


void l1_api_handler()
{
#if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1) || (FF_L1_FAST_DECODING == 1)
extern NU_HISR api_modemHISR;
#endif

#if ( (L1_MP3 == 1) || (L1_AAC == 1) || (L1_MIDI == 1) || (L1_DYN_DSP_DWNLD == 1) )
  extern NU_HISR apiHISR;
#endif

#if (FF_L1_IT_DSP_USF == 1) || (FF_L1_IT_DSP_DTX == 1) || (FF_L1_FAST_DECODING == 1)
#if (FF_L1_FAST_DECODING == 1)  
 if (l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag & (1 << B_FAST_DECODING_FLAG))
  {
    // Flag HISR to be scheduled
    l1a_apihisr_com.fast_decoding.pending = TRUE;
    // Clear API ISR condition
    l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag &= ~(1 << B_FAST_DECODING_FLAG);
    l1a_apihisr_com.fast_decoding.crc_error = 
            l1s_dsp_com.dsp_ndb_ptr->d_fast_paging_data & (C_FAST_DECODING_CRC_FIRE1);
    NU_Activate_HISR(&api_modemHISR);  // activate API modem HISR
  }
#endif /* FF_L1_FAST_DECODING */  
#if (FF_L1_IT_DSP_USF == 1) 
   if (l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag & (1 << B_USF_HINT_ISSUED))
  {
    // Flag HISR to be scheduled
    l1a_apihisr_com.usf.pending = TRUE;
    // Clear API ISR condition
    l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag &= ~(1 << B_USF_HINT_ISSUED);
    NU_Activate_HISR(&api_modemHISR);  // activate API modem HISR
  }
#endif //(FF_L1_IT_DSP_USF == 1) 
#if (FF_L1_IT_DSP_DTX == 1)
  
  if ((l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag & 0x4 ) != 0) // DTX interrupt set by 3rd bit position from left
  {
    // Flag HISR to be scheduled
     l1a_apihisr_com.dtx.pending = TRUE;
    // Clear API ISR condition
    
    l1s_dsp_com.dsp_ndb_ptr->d_dsp_hint_flag &= 0xFB; //reset the 3rd bit position
    NU_Activate_HISR(&api_modemHISR);  // activate API modem HISR
  }
#endif //(FF_L1_IT_DSP_DTX == 1) 
// Call the API HISR when MP3, AAC, DYN DWNLD comes
#if ( (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1) )




   if (l1s_dsp_com.dsp_ndb_ptr->d_dsp_aud_hint_flag & (1 << B_NON_USF_HINT_ISSUED))
  {   
    // Clear API ISR condition
    l1s_dsp_com.dsp_ndb_ptr->d_dsp_aud_hint_flag = 0;
    //Activate the API HISR
  NU_Activate_HISR(&apiHISR);
  }
#endif // (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_DYN_DSP_DWNLD == 1)
#endif //(FF_L1_IT_DSP_USF == 1) || //(FF_L1_IT_DSP_DTX == 1)

#if 0	/* FreeCalypso Frankenstein */
/* This is the original LoCosto code */

//When Fast USF is not enabled, then the API HISR needs to be called every time the DSP 
//sends a HINT interrupt.
#if (!FF_L1_IT_DSP_USF) && (!FF_L1_IT_DSP_DTX)
#if ( (L1_MP3 == 1) || (L1_MIDI == 1) || (L1_AAC == 1) || (L1_DYN_DSP_DWNLD == 1) )
NU_Activate_HISR(&apiHISR); 
#endif
#endif
//NU_Activate_HISR(&apiHISR); //hack remove
  return;

#else

/* reconstruction of what the TCS211 code was probably like */
#if (L1_DYN_DSP_DWNLD == 1)
  if( l1_apihisr.dyn_dwnld.running == TRUE )
  {
    NU_Activate_HISR(&apiHISR); 
  }
#endif

#endif
}

/*-------------------------------------------------------------*/
/* api_hisr()                                                  */
/*-------------------------------------------------------------*/
/*                                                             */
/* Parameters : none                                           */
/*                                                             */
/* Return     : n/a                                            */
/*                                                             */
/* Description : main function dealing with DSP->MCU interrupt */
/*                                                             */
/*-------------------------------------------------------------*/
void api_hisr(void)
{
#if (L1_MP3 == 1)
  if(l1a_apihisr_com.mp3.running==TRUE)
  {
    l1mp3_apihisr();
  }
#endif
#if (L1_MIDI == 1)
  if(l1_apihisr.midi.running==TRUE)
  {
    l1midi_apihisr();
  }
#endif
#if (L1_AAC == 1)
  if(l1a_apihisr_com.aac.running==TRUE)
  {
    l1aac_apihisr();
  }
#endif
#if (L1_DYN_DSP_DWNLD == 1)
  if( l1_apihisr.dyn_dwnld.running == TRUE )
  {
    l1_dyn_dwnld_apihisr();
  }
#endif

}

#if 0	/* FreeCalypso */
/*-------------------------------------------------------------*/
/* api_modem_hisr()                                            */
/*-------------------------------------------------------------*/
/*                                                             */
/* Parameters : none                                           */
/*                                                             */
/* Return     : n/a                                            */
/*                                                             */
/* Description : main function dealing with DSP->MCU interrupt */
/*                                                             */
/*-------------------------------------------------------------*/
void api_modem_hisr(void)
{
#if (FF_L1_FAST_DECODING == 1)
if(l1a_apihisr_com.fast_decoding.pending == TRUE)
  {
    extern void l1_fast_decoding_apihisr(void);
    l1_fast_decoding_apihisr();
  }

#endif /* FF_L1_FAST_DECODING */  
#if (FF_L1_IT_DSP_USF == 1)
  if(l1a_apihisr_com.usf.pending == TRUE)
  {
    extern void l1usf_apihisr(void);
    l1usf_apihisr();
  }
#endif
#if (FF_L1_IT_DSP_DTX == 1)
  if(l1a_apihisr_com.dtx.pending == TRUE)
  {
    extern void l1dtx_apihisr(void);
    l1dtx_apihisr();
  }
#endif
}
#endif	/* FreeCalypso */

/*-------------------------------------------------------------*/
/* l1_trigger_api_interrupt()                                  */
/*-------------------------------------------------------------*/
/*                                                             */
/* Parameters : none                                           */
/*                                                             */
/* Return     : n/a                                            */
/*                                                             */
/* Description : Trigger an MCU->DSP interrupt                 */
/*                                                             */
/*-------------------------------------------------------------*/
void l1_trigger_api_interrupt()
{
#if (CODE_VERSION == SIMULATION)
  extern void sim_it_mcu_to_dsp();
  sim_it_mcu_to_dsp();
#else
#if (L1_DYN_DSP_DWNLD == 1)
  *((volatile UWORD32 *)MEM_APIC_REG)|=ID_API_INT;
#else
  *((volatile UWORD32 *)MEM_APIC_REG)|=0x4;
#endif
#endif

}

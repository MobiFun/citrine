/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1AUDIO_DRIVE.C
 *
 *        Filename l1audio_drive.c
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/************************************/
/* Include files...                 */
/************************************/

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"

#if 1 //(AUDIO_TASK == 1)

  #include "l1_types.h"
  #include "sys_types.h"

  #if (CODE_VERSION == SIMULATION) && (AUDIO_SIMULATION)


    #include <stdlib.h>
    #include <string.h>
      
    #include "iq.h"             // Debug / Init hardware  ("eva3.lib")
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif

    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "cust_os.h"
    #include "l1_msgty.h"
    #include "l1_varex.h"

    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "l1_ctl.h"
       

    #include "l1_time.h"
    #include "l1_scen.h"

  #else
  // Layer1 and debug include files.

    #include <ctype.h>
    #include <math.h>
    #include "l1_ver.h"
    #include "l1_const.h"
    #include "l1_signa.h"

    #if TESTMODE
      #include "l1tm_defty.h"
    #endif
 
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"

    #if (L1_GTT == 1)
      #include "l1gtt_const.h"
      #include "l1gtt_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_DYN_DSP_DWNLD == 1)
      #include "l1_dyn_dwl_const.h"
      #include "l1_dyn_dwl_defty.h"
    #endif
    #if (L1_MP3 == 1)
      #include "l1mp3_defty.h"
    #endif

    #if (L1_MIDI == 1)
      #include "l1midi_defty.h"
    #endif
//added here from e-sample for AAC
    #if (L1_AAC == 1)
      #include "l1aac_defty.h"
    #endif

    #include "l1_defty.h"
    #include "../../gpf/inc/cust_os.h"
    #include "l1_msgty.h"
    #include "tpudrv.h"       // TPU drivers.           ("eva3.lib")
    #include "l1_varex.h"

    #include "l1_proto.h"
    #include "l1_mftab.h"
    #include "l1_tabs.h"
    #include "../../bsp/mem.h"
    #include "../../bsp/armio.h"
    #include "../../bsp/timer.h"
    #include "../../bsp/timer1.h"
    #include "../../bsp/dma.h"
    #include "../../bsp/inth.h"
    #include "../../bsp/ulpd.h"
    #include "../../bsp/rhea_arm.h"
    #include "../../bsp/clkm.h"         // Clockm  ("eva3.lib")
    #include "l1_ctl.h"

    #include "l1_time.h"
    #if L2_L3_SIMUL
      #include "l1_scen.h"
    #endif
  #endif

  #include "l1audio_macro.h"

  /**************************************/
  /* Prototypes for L1 SYNCH manager    */
  /**************************************/
  void vocoder_mute_dl (BOOL mute);
  void vocoder_mute_ul (BOOL mute);
 #if (AUDIO_DSP_FEATURES == 1)
  void L1_audio_sidetone_write(UWORD16 sidetone_value);
  void L1_audio_CAL_DlVolume_write(UWORD16 vol_value);
  void L1_audio_CAL_UlVolume_write(UWORD16 vol_value);
  void L1_audio_volume_speed_write(UWORD16 volspeed_value);
 #endif

  /**************************************/
  /* External prototypes                */
  /**************************************/

  /*-------------------------------------------------------*/
  /* vocoder_mute_dl()                                     */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : Mute the DL vocoder.                    */
  /*                                                       */
  /*-------------------------------------------------------*/
  void vocoder_mute_dl(BOOL mute)
  {
    if (mute)
    {
    // Set the DL vocoder mute bit in the d_tch_mode register
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= (0x01<<14);
    }
    else
    {
      // Reset the DL vocoder mute bit in the d_tch_mode register
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(0x01<<14);
    }
  }

  /*-------------------------------------------------------*/
  /* vocoder_mute_ul()                                     */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : Mute the UL vocoder.                    */
  /*                                                       */
  /*-------------------------------------------------------*/
  void vocoder_mute_ul(BOOL mute)
  {
    if (mute)
    {
    // Set the UL vocoder mute bit in the d_tch_mode register
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode |= (0x01<<15);
    }
    else
    {
      // Reset the UL vocoder mute bit in the d_tch_mode register
      l1s_dsp_com.dsp_ndb_ptr->d_tch_mode &= ~(0x01<<15);
    }
  } 

 #if (AUDIO_DSP_FEATURES == 1)
  /*-------------------------------------------------------*/
  /* L1_audio_sidetone_write()                             */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : write sidetone gain in API for DSP      */
  /*               sidetone process                        */
  /*                                                       */
  /*-------------------------------------------------------*/
  void L1_audio_sidetone_write(UWORD16 sidetone_value)
  {
    l1s_dsp_com.dsp_ndb_ptr->d_sidetone_level = (API) sidetone_value;
  } 

  /*-------------------------------------------------------*/
  /* L1_audio_CAL_DlVolume_write()                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : write DL gain in API for DSP vol ctrl   */
  /*               process                                 */
  /*                                                       */
  /*-------------------------------------------------------*/
  void L1_audio_CAL_DlVolume_write(UWORD16 vol_value)
  {
    l1s_dsp_com.dsp_ndb_ptr->d_vol_dl_level = (API) vol_value;
  } 

  /*-------------------------------------------------------*/
  /* L1_audio_CAL_UlVolume_write()                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : write UL gain in API for DSP vol ctrl   */
  /*               process                                 */
  /*                                                       */
  /*-------------------------------------------------------*/
  void L1_audio_CAL_UlVolume_write(UWORD16 vol_value)
  {
    l1s_dsp_com.dsp_ndb_ptr->d_vol_ul_level = (API) vol_value;
  } 

  /*-------------------------------------------------------*/
  /* L1_audio_volume_speed_write()                         */
  /*-------------------------------------------------------*/
  /*                                                       */
  /* Parameters :                                          */
  /*                                                       */
  /* Return     :                                          */
  /*                                                       */
  /* Description : write volume speed in API for DSP vol   */
  /*               ctrl process                            */
  /*                                                       */
  /*-------------------------------------------------------*/
  void L1_audio_volume_speed_write(UWORD16 volspeed_value)
  {
    l1s_dsp_com.dsp_ndb_ptr->d_vol_speed = (API) volspeed_value;
  } 

 #endif /* DSP_AUDIO_FEAT */

#endif // AUDIO_TASK

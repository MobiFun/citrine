/*
 * l1audio_abb.c
 *
 * Control audio 
 *
 *        Filename l1audio_abb.c
 *  Copyright 2003 (C) Texas Instruments  
 *
 * Reference : S820, GACS001 (OMEGA) spec
 *
 */

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"


#if (CODE_VERSION == NOT_SIMULATION)

#if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
  #include "rv_general.h"
#endif

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
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
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include <stdio.h>
  #include "sim_cfg.h"
  #include "sim_cons.h"
  #include "sim_def.h"
  #include "sim_var.h"

#else
  #include <string.h>          
  #if (defined _WINDOWS && (OP_RIV_AUDIO == 1))
    #define BOOL_FLAG
  #endif
// Triton Audio ON/OFF Changes

  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif  
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
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
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif

  #if (RF_FAM == 61)
    #include "l1_rf61.h"
  #endif 
  
  #include "l1_defty.h"
  #include "l1_varex.h"
  #include "l1_msgty.h"
  #if (OP_RIV_AUDIO == 0)
    #include "../../gpf/inc/cust_os.h"
    #include "tpudrv.h"
  #endif
#endif

#include "../../bsp/abb+spi/abb.h"

#include "l1audio_abb.h"

#if (ANALOG == 11)
  #include "types.h"
  #include "bspTwl3029_I2c.h"
  #include "bspTwl3029_Aud_Map.h"
  #include "bspTwl3029_Audio.h"
#endif

#if (CODE_VERSION == NOT_SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)&&(CHIPSET == 15)
  void l1_audio_on_off_callback_fn(Uint8 callback_val);
#endif  

#if ((CODE_VERSION == NOT_SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)&&(OP_L1_STANDALONE == 1)&&(CHIPSET == 12))
  #include "nucleus.h"
#endif

#if (ANALOG != 11)
extern T_L1S_DSP_COM l1s_dsp_com;
extern void l1_audio_lim_partial_update();


#define MAX_PGA_UL   24
#define MAX_PGA_DL   12
#define MAX_VOL_DL   249

static UWORD8 ABB_CurrentVolume = 0;

// Uplink PGA gain is coded on 5 bits, corresponding to -12 dB to +12 dB in 1dB steps
const UWORD8 ABB_uplink_PGA_gain[] = 
{
   0x10, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
   0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x15,
   0x16
};
  
// Downlink volume: mute, -24dB to 0dB in 6dB steps
const UWORD8 ABB_volume_control_gain[] =
{
   0x05, 0x03, 0x04, 0x00,  0x06, 0x02       
};

// Downlink volume gain read in unsigned Q15 (in VBDCTRL)
const WORD16 ABB_DL_volume_read_gain[] =
{
       0x2000 , // 0: -12 dB
            0 , // 1:   Mute
       0x8000 , // 2:   0 dB
       0x0800 , // 3: -24 dB
       0x1000 , // 4: -18 dB
            0 , // 5:   Mute
       0x4000 , // 6:  -6 dB
            0 , // 7:   Mute
};

// Downlink PGA gain read in unsigned Q15 (in VBDCTRL)
const WORD16 ABB_DL_PGA_read_gain[] =
{
       0x4026 , //  0: - 6 dB
       0x47FA , //  1: - 5 dB
       0x50C3 , //  2: - 4 dB
       0x5A9D , //  3: - 3 dB
       0x65AC , //  4: - 2 dB
       0x7214 , //  5: - 1 dB
       0x8000 , //  6:   0 dB
       0x8F9E , //  7:   1 dB
       0xA124 , //  8:   2 dB
       0xB4CE , //  9:   3 dB
       0xCADD , // 10:   4 dB
       0xE39E , // 11:   5 dB
       0xFF64 , // 12:   6 dB
       0x4026 , // 13: - 6 dB
       0x4026 , // 14: - 6 dB
       0x4026 , // 15: - 6 dB
};


// Downlink PGA gain is coded on 4 bits, corresponding to -6dB to 6dB in 1dB steps
const UWORD8 ABB_downlink_PGA_gain[] =
{
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
   0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C  
};

// Side tone level: mute, -23dB to +4dB in 3dB steps
const UWORD8 ABB_sidetone_gain[] =
{
   0x08, 0x0D, 0x0C, 0x06, 0x02, 0x07, 
   0x03, 0x00, 0x04, 0x01, 0x05
};

/*
 * ABB_Audio_Config
 *
 * Configuration of VBCTRL1 register
 *
 */
void ABB_Audio_Config (UWORD16 data)
{
#if (ANALOG == 1)
  l1s_dsp_com.dsp_ndb_ptr->d_vbctrl = ABB_L1_WRITE (VBCTRL, data);
#elif ((ANALOG == 2) || (ANALOG == 3)  )
  l1s_dsp_com.dsp_ndb_ptr->d_vbctrl1 = ABB_L1_WRITE (VBCTRL1, data);
#endif
}
    

/*
 * ABB_Audio_Config_2
 *
 * Configuration of VBCTRL2 register
 *
 */
void ABB_Audio_Config_2 (UWORD16 data)
{
#if ((ANALOG == 2) || (ANALOG == 3))
  l1s_dsp_com.dsp_ndb_ptr->d_vbctrl2 = ABB_L1_WRITE (VBCTRL2, data);
#endif
}


/*
 * ABB_Audio_Control
 *
 * Configuration of VAUDCTRL register
 *
 */
void ABB_Audio_Control (UWORD16 data)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vaud_cfg = ABB_L1_WRITE (VAUDCTRL, data);
#endif
}


/*
 * ABB_Audio_On_Off
 *
 * Configuration of VAUOCTRL register
 *
 */
void ABB_Audio_On_Off (UWORD16 data)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vauo_onoff = ABB_L1_WRITE (VAUOCTRL, data);
#endif
}


/*
 * ABB_Audio_Volume
 *
 * Configuration of VAUSCTRL register
 *
 */
void ABB_Audio_Volume (UWORD16 data)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vaus_vol = ABB_L1_WRITE (VAUSCTRL, data);
#endif
}


/*
 * ABB_Audio_PLL
 *
 * Configuration of VAUDPLL register
 *
 */
void ABB_Audio_PLL (UWORD16 data)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vaud_pll = ABB_L1_WRITE (VAUDPLL, data);
#endif
}


/*
 * ABB_Audio_VBPop
 *
 * Configuration of VBPOP register
 *
 */
void ABB_Audio_VBPop (UWORD16 data)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vbpop = ABB_L1_WRITE (VBPOP, data);
#endif
}


/*
 * ABB_Audio_Delay_Init
 *
 * Configuration of the delay initialization for POP noise reduction
 *
 */
void ABB_Audio_Delay_Init (UWORD8 delay)
{
#if (ANALOG == 3)
  l1s_dsp_com.dsp_ndb_ptr->d_vau_delay_init = delay;
#endif
}


/*
 * ABB_CAL_UlVolume
 *
 * Uplink audio volume calibration
 *
 * Parameter : pga index - range 0..24 
 *
 * When this function is called the Mute bit of VBUCTRL is set to zero
 */
void ABB_CAL_UlVolume (UWORD8 pga_index)
{
  UWORD16 index;
  API     reg_state;
   
  index = pga_index;
  if (index > MAX_PGA_UL)
      index = MAX_PGA_UL;  //clip

  //mask side tone gain and the mute settings
  reg_state = l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl &= 0xF800; 
    
  l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl = reg_state | 
                                       ABB_L1_WRITE ( VBUCTRL,
                                                      ABB_uplink_PGA_gain[index]);
}

/*
 * ABB_CAL_DlVolume
 *
 * Downlink audio volume calibration
 *
 * Parameter : volume - range 0 to 255, pga - range 0-12
 *
 */

void ABB_CAL_DlVolume (UWORD8 volume_index, UWORD8 pga_index)
{
  UWORD16 volume, pga;
 // Remember current volume for subsequent mute commands
  ABB_CurrentVolume = volume_index;
    
  // Normalize volume (0 to 5)
  if (volume_index > MAX_VOL_DL) volume_index=MAX_VOL_DL;   //clip
    
  if (volume_index)   
    volume = (volume_index / 50) + 1;
  else
    volume = volume_index;
     
  if (pga_index > MAX_PGA_DL) pga_index=MAX_PGA_DL;  //clip
  pga = pga_index;
    

  l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl =
  ABB_L1_WRITE ( VBDCTRL,
                 (ABB_volume_control_gain[volume] << 4) |
                 (ABB_downlink_PGA_gain[pga])   );

  #if (L1_LIMITER == 1)
    if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_LIM_ENABLE)
      l1_audio_lim_partial_update();
  #endif
}

/*
 * ABB_DlVolume
 *
 * Control Downlink audio volume
 *
 * Parameter : volume - range 0 to 255
 *
 */

void ABB_DlVolume (UWORD8 volume_index)
{
  UWORD16 volume; 
  API     reg_state;

  // Remember current volume for subsequent mute commands
  ABB_CurrentVolume = volume_index;
    
  // Normalize volume (0 to 5)
  if (volume_index > MAX_VOL_DL) volume_index=MAX_VOL_DL;   //clip
    
  if (volume_index)   
    volume = (volume_index / 50) + 1;
  else
    volume = volume_index;
      
  //mask PGA setting determined during calibration phase
  reg_state = l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl &= 0x03c0; 

  l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl =  reg_state |
  ABB_L1_WRITE ( VBDCTRL,
                 (ABB_volume_control_gain[volume] << 4));

  #if (L1_LIMITER == 1)
    if (l1s_dsp_com.dsp_ndb_ptr->d_aqi_status & B_LIM_ENABLE)
      l1_audio_lim_partial_update();
  #endif
}

/*
 * ABB_DlMute
 *
 * Mute downlink audio
 *
 * Parameter : Mute - On or Off
 *
 */

void ABB_DlMute (BOOL mute)
{
  UWORD8 current_volume;

  if (mute)
  {
    /*
     * The current downlink volume must be memorized to avoid
     * having 0 as the new current volume.
     */

    current_volume = ABB_CurrentVolume;
    ABB_DlVolume (0);
    ABB_CurrentVolume = current_volume;
  } 
  else
  {
    ABB_DlVolume (ABB_CurrentVolume);
  }
}

/*
 * ABB_UlMute
 *
 * Mute uplink audio
 *
 * Parameter : Mute - On or Off
 *
 */

void ABB_UlMute (BOOL mute)
{
  if (mute)
    {
      l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl |=
         ABB_L1_WRITE ( VBUCTRL,
                        DXEN);
    }
    else
    {
      l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl &= 0x7fff;
      l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl |= 0x01;
    }

  }

/*
 * ABB_SideTone
 *
 * Control audio sidetone
 *
 * Parameter : volume - range 0 to 255
 * nominal is 175 (-5dB)
 *
 */

void ABB_SideTone (UWORD8 volume_index)
{
  UWORD16 side_tone;
  API     reg_state;

  // Normalize sidetone (0 to 10)
  side_tone = volume_index / 25;

  // mask uplink PGA gain and mute settings
  reg_state = l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl &= 0x87c0; 

  l1s_dsp_com.dsp_ndb_ptr->d_vbuctrl = reg_state |
     ABB_L1_WRITE (VBUCTRL,
                   (ABB_sidetone_gain[side_tone] << 5));
}

/*
 * ABB_Read_DLVolume
 *
 * Returns the last controlled ABB DL gain in unsigned Q15 format
 * This value includes volume and PGA gain.
 *
 */
UWORD16 ABB_Read_DLGain()
{
  
  UWORD16 volume_index;
  UWORD16 pga_index;

  // Read last programmed volume
    //Sundi: change for Triton
    #if (ANALOG == 1) || (ANALOG == 2) || (ANALOG == 3)
  volume_index = (API)((l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl >> 10) & 0x7);
  pga_index    = (API)((l1s_dsp_com.dsp_ndb_ptr->d_vbdctrl >>  6) & 0xF);
    #endif

  // Convert volume into gain (dB)
  return((ABB_DL_volume_read_gain[volume_index] * ABB_DL_PGA_read_gain[pga_index]) >> 15);
  }
#endif // ANALOG == 3

#if (ANALOG == 11)
  // Downlink volume gain read in unsigned Q15 (in VBDCTRL)
  const WORD16 L1_audio_abb_DL_volume_read_gain[] =
  {
         (WORD16)0x2000 , // 0: -12 dB //omaps00090550
         (WORD16)0x0000 , // 1:   Mute //omaps00090550
         (WORD16)0x8000 , // 2:   0 dB//omaps00090550
         (WORD16)0x0800 , // 3: -24 dB//omaps00090550
         (WORD16)0x1000 , // 4: -18 dB//omaps00090550
         (WORD16)0x0000 , // 5:   Mute//omaps00090550
         (WORD16)0x4000 , // 6:  -6 dB//omaps00090550
         (WORD16)0x0000 , // 7:   Mute//omaps00090550
  };
  
  // Downlink PGA gain read in unsigned Q15 (in VBDCTRL)
  const WORD16 L1_audio_abb_DL_PGA_read_gain[] =
  {
         (WORD16)0x4026 , //  0: - 6 dB//omaps00090550
         (WORD16) 0x47FA , //  1: - 5 dB//omaps00090550
         (WORD16)0x50C3 , //  2: - 4 dB//omaps00090550
         (WORD16)0x5A9D , //  3: - 3 dB//omaps00090550
         (WORD16)0x65AC , //  4: - 2 dB//omaps00090550
         (WORD16)0x7214 , //  5: - 1 dB//omaps00090550
         (WORD16)0x8000 , //  6:   0 dB//omaps00090550
         (WORD16)0x8F9E , //  7:   1 dB//omaps00090550
         (WORD16)0xA124 , //  8:   2 dB//omaps00090550
         (WORD16)0xB4CE , //  9:   3 dB//omaps00090550
         (WORD16)0xCADD , // 10:   4 dB//omaps00090550
         (WORD16)0xE39E , // 11:   5 dB//omaps00090550
         (WORD16)0xFF64 , // 12:   6 dB//omaps00090550
         (WORD16)0x4026 , // 13: - 6 dB//omaps00090550
         (WORD16)0x4026 , // 14: - 6 dB//omaps00090550
         (WORD16)0x4026 , // 15: - 6 dB//omaps00090550
  };

  UWORD16 l1_audio_abb_Read_DLGain()
  {
  
    UWORD8 volume_index;
    UWORD8 pga_index;
    UWORD8 vdlgain;

   BspTwl3029_I2c_shadowRegRead(BSP_TWL3029_I2C_AUD,
                              BSP_TWL_3029_MAP_AUDIO_VDLGAIN_OFFSET,
                              &vdlgain);

    volume_index = ((vdlgain & 0x70) >> 4);  // bits 4-6
    pga_index      =  (vdlgain & 0x0f) ; //bits 0-3

    // Convert volume into gain (dB)
    return((L1_audio_abb_DL_volume_read_gain[volume_index] * L1_audio_abb_DL_PGA_read_gain[pga_index]) >> 15);
  }

  void ABB_Audio_On_Off (UWORD16 data)
  {

  }
#endif // ANALOG == 11

#endif // CODE_VERSION != SIMULATION

// Triton Audio ON/OFF Changes
#if (CODE_VERSION == SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)

void l1_audio_abb_ul_on_req         ( void(*callback_fn)(void) )
{
    callback_fn();
}
void l1_audio_abb_dl_on_req         ( void(*callback_fn)(void) )
{
    callback_fn();
}

void l1_audio_abb_ul_off_req        ( void(*callback_fn)(void) )
{
    callback_fn();    
}

void l1_audio_abb_dl_off_req        ( void(*callback_fn)(void) )
{
    callback_fn();    
}
    
void l1_audio_abb_ul_off_dl_off_req ( void(*callback_fn)(void) )
{
    callback_fn();    
}
    
void l1_audio_abb_ul_off_dl_on_req  ( void(*callback_fn)(void) )
{
    callback_fn();    
}
    
void l1_audio_abb_ul_on_dl_off_req  ( void(*callback_fn)(void) )
{
    callback_fn();    
}
    
void l1_audio_abb_ul_on_dl_on_req   ( void(*callback_fn)(void) )
{
    callback_fn();    
}

#endif // SIMULATION && L1_AUDIO_MCU_ONOFF

// Triton Audio ON/OFF Changes
#if ((CODE_VERSION == NOT_SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)&&(OP_L1_STANDALONE == 1)&&(CHIPSET == 12))

void(*cb_array[3])(void);
SYS_UWORD16 vauoctrl_status;
extern NU_TIMER l1_audio_abb_ul_timer;
extern NU_TIMER l1_audio_abb_dl_timer;
extern NU_TIMER l1_audio_abb_ul_dl_timer;

void l1_audio_abb_ul_on_req         ( void(*callback_fn)(void) )
{
    SYS_UWORD16 reg;
    reg = ABB_Read_Register_on_page(PAGE1, VBUCTRL);
    ABB_Write_Register_on_page(PAGE1, VBUCTRL, reg|0x0200);
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0002);
    cb_array[0]=callback_fn;
    NU_Control_Timer(&l1_audio_abb_ul_timer, NU_ENABLE_TIMER);        
}
void l1_audio_abb_dl_on_req         ( void(*callback_fn)(void) )
{
    vauoctrl_status = ABB_Read_Register_on_page(PAGE1, VAUOCTRL);
    ABB_Write_Register_on_page(PAGE1, VAUOCTRL, 0x0000);
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0008);
    cb_array[1]=callback_fn;
    NU_Control_Timer(&l1_audio_abb_dl_timer, NU_ENABLE_TIMER);        
}

void l1_audio_abb_ul_off_req        ( void(*callback_fn)(void) )
{
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0001);
    callback_fn();    
}

void l1_audio_abb_dl_off_req        ( void(*callback_fn)(void) )
{
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0004);
    callback_fn();    
}
    
void l1_audio_abb_ul_off_dl_off_req ( void(*callback_fn)(void) )
{
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0001|0x0004);
    callback_fn();    
}
    
void l1_audio_abb_ul_off_dl_on_req  ( void(*callback_fn)(void) )
{
    vauoctrl_status = ABB_Read_Register_on_page(PAGE1, VAUOCTRL);
    ABB_Write_Register_on_page(PAGE1, VAUOCTRL, 0x0000);
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0008|0x0001);
    cb_array[1]=callback_fn;
    NU_Control_Timer(&l1_audio_abb_dl_timer, NU_ENABLE_TIMER);        
}
    
void l1_audio_abb_ul_on_dl_off_req  ( void(*callback_fn)(void) )
{
    SYS_UWORD16 reg;
    reg = ABB_Read_Register_on_page(PAGE1, VBUCTRL);
    ABB_Write_Register_on_page(PAGE1, VBUCTRL, reg|0x0200);
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0002|0x0004);
    cb_array[0]=callback_fn;
    NU_Control_Timer(&l1_audio_abb_ul_timer, NU_ENABLE_TIMER);        
}
    
void l1_audio_abb_ul_on_dl_on_req   ( void(*callback_fn)(void) )
{
    SYS_UWORD16 reg;
    reg = ABB_Read_Register_on_page(PAGE1, VBUCTRL);
    vauoctrl_status = ABB_Read_Register_on_page(PAGE1, VAUOCTRL);    
    ABB_Write_Register_on_page(PAGE1, VAUOCTRL, 0x0000);
    ABB_Write_Register_on_page(PAGE1, VBUCTRL, reg|0x0200);
    ABB_Write_Register_on_page(PAGE0, TOGBR1, 0x0002|0x0008);
    cb_array[2]=callback_fn;
    NU_Control_Timer(&l1_audio_abb_ul_dl_timer, NU_ENABLE_TIMER);        
}


void l1_audio_abb_onoff_timer_expiry(UNSIGNED index)
{
    L1_trace_string("ON OFF Timer Expiry\r\n");
    switch(index)
    {
        case 0:
        {
            SYS_UWORD16 reg;
            reg = ABB_Read_Register_on_page(PAGE1, VBUCTRL);           
            ABB_Write_Register_on_page(PAGE1, VBUCTRL, reg&0x01FF); 
            cb_array[0]();
        }
        break;
        case 1:
        {
            ABB_Write_Register_on_page(PAGE1, VAUOCTRL, vauoctrl_status);
            cb_array[1]();
        }
        break;
        case 2:
        {
            SYS_UWORD16 reg;
            reg = ABB_Read_Register_on_page(PAGE1, VBUCTRL);           
            ABB_Write_Register_on_page(PAGE1, VBUCTRL, reg&0x01FF); 
            ABB_Write_Register_on_page(PAGE1, VAUOCTRL, vauoctrl_status);
            cb_array[2]();            
        }
        break;
        default:
        {
            while(1);
        }
        break;         
    }
}
#endif // SIMULATION && L1_AUDIO_MCU_ONOFF

// Triton Audio ON/OFF Changes
#if (CODE_VERSION == NOT_SIMULATION)&&(L1_AUDIO_MCU_ONOFF == 1)&&(CHIPSET == 15)

void(*cb_array[3])(void);
UWORD8 cb_index = 0;

void l1_audio_abb_ul_on_req         ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    
    bspTwl3029_audio_on_off_control (AUDIO_UPLINK_ON,
					                 AUDIO_DOWNLINK_NONE, 
					                 ret, OUTEN_NONE);
    
}
void l1_audio_abb_dl_on_req         ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    
    bspTwl3029_audio_on_off_control (AUDIO_UPLINK_NONE,
					                 AUDIO_DOWNLINK_ON, 
					                 ret, OUTEN_NONE);

}

void l1_audio_abb_ul_off_req        ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    
    bspTwl3029_audio_on_off_control (AUDIO_UPLINK_OFF,
					                 AUDIO_DOWNLINK_NONE, 
					                 ret, OUTEN_NONE);

}

void l1_audio_abb_dl_off_req        ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    // Configure the outen reg to 0 only when STEREOPATH_DRV_STATE is in IDLE STATE 
    if(l1s.audio_state[L1S_STEREOPATH_DRV_STATE] == 0)
    {
       bspTwl3029_audio_on_off_control (AUDIO_UPLINK_NONE,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_DISABLE);
    }
    else
    {
       bspTwl3029_audio_on_off_control (AUDIO_UPLINK_NONE,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_ENABLE);   
    }
    
}
    
void l1_audio_abb_ul_off_dl_off_req ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    // Configure the outen reg to 0 only when STEREOPATH_DRV_STATE is in IDLE STATE
    if(l1s.audio_state[L1S_STEREOPATH_DRV_STATE] == 0)
    {
       bspTwl3029_audio_on_off_control (AUDIO_UPLINK_OFF,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_DISABLE);
    }
    else
    {
       bspTwl3029_audio_on_off_control (AUDIO_UPLINK_OFF,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_ENABLE);
    }

}
    
void l1_audio_abb_ul_off_dl_on_req  ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    
    bspTwl3029_audio_on_off_control (AUDIO_UPLINK_OFF,
					                 AUDIO_DOWNLINK_ON, 
					                 ret, OUTEN_NONE);
}
    
void l1_audio_abb_ul_on_dl_off_req  ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    // Configure the outen reg to 0 only when STEREOPATH_DRV_STATE is in IDLE STATE
    if(l1s.audio_state[L1S_STEREOPATH_DRV_STATE] == 0)
    {
       bspTwl3029_audio_on_off_control (AUDIO_UPLINK_ON,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_DISABLE);
    }
    else
    {
        bspTwl3029_audio_on_off_control (AUDIO_UPLINK_ON,
					                 AUDIO_DOWNLINK_OFF, 
					                 ret, OUTEN_ENABLE);
    }

}
    
void l1_audio_abb_ul_on_dl_on_req   ( void(*callback_fn)(void) )
{
    T_AUDIO_ON_OFF_CONTROL_RETURN ret;

    cb_array[cb_index] = callback_fn;

    ret.audio_on_off_callback = l1_audio_on_off_callback_fn;
    ret.callback_val = cb_index;

    cb_index++;
    if(cb_index == 3)cb_index = 0;
    
    
    bspTwl3029_audio_on_off_control (AUDIO_UPLINK_ON,
					                 AUDIO_DOWNLINK_ON, 
					                 ret, OUTEN_NONE);
}


void l1_audio_on_off_callback_fn(Uint8 callback_val)
{
    cb_array[callback_val]();
}

#endif // SIMULATION && L1_AUDIO_MCU_ONOFF

#if (AUDIO_DEBUG == 1)
/*-------------------------------------------------------*/
/* l1_audio_regs_debug_read                              */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/*                                                       */
/*-------------------------------------------------------*/
UWORD8 audio_reg_read_status;
BspI2c_TransactionRequest audo_read_i2cIntTransArray[20]; 
BspTwl3029_I2C_RegData audio_regs[20];
UWORD8 audio_regs_cpy[20];
void l1_audio_reg_debug_read_callback();
void l1_audio_regs_debug_read()
{
  BspTwl3029_I2C_RegisterInfo i2cRegArray[20];
  BspTwl3029_I2C_RegisterInfo* i2cRegArrayPtr = i2cRegArray;
  
  BspTwl3029_I2C_Callback callback;
  BspTwl3029_I2C_CallbackPtr callbackPtr = &callback; 
  
  callbackPtr->callbackFunc = (BspI2c_TransactionDoneCallback)l1_audio_reg_debug_read_callback;       
  callbackPtr->callbackVal = (BspI2c_TransactionId)(1);  
  callbackPtr->i2cTransArrayPtr = (Bsp_Twl3029_I2cTransReqArrayPtr)audo_read_i2cIntTransArray;
  
  if(audio_reg_read_status==0)
  {
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_PWRONSTATUS_OFFSET,&audio_regs[0],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL1_OFFSET,&audio_regs[1],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL2_OFFSET,&audio_regs[2],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL3_OFFSET,&audio_regs[3],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL4_OFFSET,&audio_regs[4],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL5_OFFSET,&audio_regs[5],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_CTRL6_OFFSET,&audio_regs[6],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_VULGAIN_OFFSET,&audio_regs[7],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_VDLGAIN_OFFSET,&audio_regs[8],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_OUTEN1_OFFSET,&audio_regs[9],i2cRegArrayPtr++);
    BspTwl3029_I2c_regQueRead(BSP_TWL3029_I2C_AUD,
        BSP_TWL_3029_MAP_AUDIO_OUTEN2_OFFSET,&audio_regs[10],i2cRegArrayPtr++);
    i2cRegArrayPtr = i2cRegArray;
    BspTwl3029_I2c_regInfoSend(i2cRegArrayPtr,11,callbackPtr,
                           (BspI2c_TransactionRequest*)callbackPtr->i2cTransArrayPtr);    
  }
}

void l1_audio_reg_debug_read_callback()
{
  int i;
  audio_reg_read_status=1;
  for(i=0;i<20;i++)
  {
    audio_regs_cpy[i]=audio_regs[i];
  }
}

#endif

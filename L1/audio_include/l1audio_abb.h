/*
 *        Filename l1audio_abb.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 *
 */

#define ABB_L1_WRITE(addr,data) (((data) << 6) | (addr) | 0x01)

void ABB_CAL_UlVolume     (UWORD8 pga_index);
void ABB_CAL_DlVolume     (UWORD8 volume_index, UWORD8 pga_index);
void ABB_UlVolume         (UWORD8 volume_index);
void ABB_DlVolume         (UWORD8 volume_index);
void ABB_DlMute           (BOOL mute);
void ABB_SideTone         (UWORD8 volume_index);
void ABB_Audio_Config     (UWORD16 data);
void ABB_Audio_Config_2   (UWORD16 data); 
void ABB_UlMute           (BOOL mute);
void ABB_Audio_Control    (UWORD16 data);
void ABB_Audio_On_Off     (UWORD16 data);
void ABB_Audio_Volume     (UWORD16 data);
void ABB_Audio_PLL        (UWORD16 data);
void ABB_Audio_VBPop      (UWORD16 data);
void ABB_Audio_Delay_Init (UWORD8 delay);

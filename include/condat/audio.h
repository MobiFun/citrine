/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM (6301)
|  Modul   :  
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Types definitions for the audio driver
|             .
+----------------------------------------------------------------------------- 

********************************************************************************
 	$History: audio.h

 	xpradipg - LOCOST-SPR-33914: 25 Aug 2005
 	Description: MO to the Agilent resets the target
 	Solution: The enum value was assigned in one of the array which was not
 			propogated to another, due to buzzer removal there was a mismatch
 			the enum value assignment is guarded by a compiler flag

 	xpradipg - GSM-ENH-32494 : 23 June 2005
 	Description:	Support for various audio profiles
 	Solution:	The support for audio device's carkit/headset/loudspeaker/handheld.
 			The audio device is enabled as per the user actions.
 			
********************************************************************************
 
 
|		  Apr 04, 2005    REF: ENH 30063 xdeepadh
|		  Description: Buzzer removal activity for Locosto MMI
|		  Fix:All the Buzzer related calls are under the  compilation flag FF_BUZZER.
|			The GSM tones which were playing in the buzzer are now routed to the speaker(Audio).     
+-----------------------------------------------------------------------------
*/
#ifdef FF_MMI_AUDIO_PROFILE
#include "rv/rv_general.h"
#include "audio/audio_api.h"
#endif
#ifndef AUDIO_H
#define AUDIO_H

/*
 * Signal Types
 */
#define AUDIO_SIGTYPE_SOUNDDEND    0

/*
 * Return Values
 */
#define AUDIO_FCT_NOTSUPPORTED     20

/*
 * Audio Devices
 */
#define AUDIO_SPEAKER              1
#define AUDIO_MICROPHONE           2
#define AUDIO_BUZZER               3

/*
 * Audio Status
 */
#define NO_TONE_STATE    0
#define BUZZER_ON        1
#define BUZZER_SILENT    2
#define TONE_ON          3
#define TONE_SILENT      4

/*
 * Mute Status
 */
#define AUDIO_MUTING_ON  0
#define AUDIO_MUTING_OFF 1

/*
 * Minimum and Maximum Volumes
 */
#define AUDIO_MIN_VOLUME      0
#define AUDIO_MAX_VOLUME      255
#define AUDIO_MIN_BUZ_VOLUME  0
#define AUDIO_MAX_BUZ_VOLUME  255
#define AUDIO_MIN_MIC_VOLUME  75
#define AUDIO_MAX_MIC_VOLUME  75

//xpradipg - 32494
#ifdef FF_MMI_AUDIO_PROFILE
#define CALLBACK_HEADSET 0
#define CALLBACK_CARKIT 1
#define MAX_EXT_AUD_DEVICE 2
#endif

enum AUDIO_SOUND_IDS
{
    AUDIO_MEL_LOW_BAT,
    AUDIO_MEL_SMS_ALRT,
    AUDIO_MEL_SWITCH_OFF,
    AUDIO_MEL_AUTO_REDIAL,
    AUDIO_MEL_KEY_BEEP,
    AUDIO_MEL_ERROR_TONE,
    AUDIO_MEL_USSD_TONE,
    AUDIO_MEL_DIARY_TONE,
    AUDIO_MEL_CB_TONE,
    AUDIO_MEL_SVC_TONE,
    AUDIO_MEL_MAN_PLMN_TONE,
    AUDIO_MEL_CONNECT_CHARGER,
    AUDIO_MEL_LOW_CREDIT,
    AUDIO_MEL_ALARM_TONE,

    AUDIO_MEL_TUNES,
    AUDIO_PLAY_SINGLE_NOTE = 0xFF
};

//Apr 04, 2005    REF: ENH 30063 xdeepadh
//Tone ids for Buzzer
//When the buzzer is not available, the ringing tones will be played with midiringer.
enum AUDIO_TONES_IDS
{
#ifndef FF_MMI_NO_BUZZER
    TONES_RING_0,
    TONES_RING_1,
    TONES_RING_2,
    TONES_RING_3,
    TONES_RING_4,
    TONES_RING_5,
    TONES_RING_6,
    TONES_RING_7,
    TONES_RING_8,
    TONES_RING_9,
    TONES_RING_10,
    TONES_RING_11,
    TONES_RING_12,
    TONES_RING_13,
    TONES_RING_14,
    TONES_RING_15,
#endif
    TONES_ERROR,
    TONES_BUSY,
    TONES_CONGEST,
    TONES_DROPPED,
    TONES_ACK,
    TONES_CW,
    TONES_DTMF_0,
    TONES_DTMF_1,
    TONES_DTMF_2,
    TONES_DTMF_3,
    TONES_DTMF_4,
    TONES_DTMF_5,
    TONES_DTMF_6,
    TONES_DTMF_7,
    TONES_DTMF_8,
    TONES_DTMF_9,
    TONES_DTMF_A,
    TONES_DTMF_B,
    TONES_DTMF_C,
    TONES_DTMF_D,
    TONES_DTMF_STAR,
    TONES_DTMF_HASH,
//xpradipg - LOCOST-SPR-33914: 25 Aug 2005    
#ifndef FF_MMI_NO_BUZZER
    TONES_KEYBEEP = 0x26,
#else
    TONES_KEYBEEP,
#endif    
    TONES_RINGING_TONE, /* SPR#2340 - DS - Added */
    TONES_LAST_TONE_ID
};

/*
 * internal data structures
 */
typedef struct
{
  USHORT      command_1;
  USHORT      command_2;
  USHORT      length;
} T_DESCR;

typedef struct
{
  UBYTE       status;
  UBYTE       call_tone;
  UBYTE       type;
  T_DESCR   * descr;
  UBYTE       volume;
  UBYTE       style;
  USHORT      descr_index;
} T_ACT_TONE;

/*
 * Status Type
 */
typedef struct audio_Status_Type
{
  UBYTE min_volume;
  UBYTE max_volume;
} audio_Status_Type;

enum AUDIO_PLAY_STYLES
{
    AUDIO_PLAY_CRESCENDO    = 0xff,
    AUDIO_PLAY_INFINITE     =  0,
    AUDIO_PLAY_ONCE         =  1
};

/*
 * TTY definitions
 */
typedef enum
{
  TTY_OFF = 0,
  TTY_VCO,
  TTY_HCO,
  TTY_ALL
} T_TTY_CMD;

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#define T_VSI_THANDLE               USHORT
#endif
/*
 * Prototypes
 */

//  	xpradipg - GSM-ENH-32494 : 23 June 2005
#ifdef FF_MMI_AUDIO_PROFILE
EXTERN UBYTE audio_Init(T_ACCESSORY_CALLBACK *audioConfig);
EXTERN void audio_set_path(int audioDevice,int stereo);
#else
EXTERN UBYTE audio_Init             (drv_SignalCB_Type   in_SignalCBPtr);
#endif
EXTERN void  audio_Exit             (void);
EXTERN UBYTE audio_SetMute          (UBYTE               in_DeviceID,
                                     UBYTE               in_Mode);
EXTERN UBYTE audio_GetMute          (UBYTE               in_DeviceID,
                                     UBYTE             * out_Mode);
EXTERN UBYTE audio_GetSoundImage    (UBYTE               in_SoundID,
                                     void              * out_SoundImagePtr);
EXTERN UBYTE audio_SetAmplf         (UBYTE               in_DeviceID,
                                     UBYTE               in_Amplf);
EXTERN UBYTE audio_GetAmplf         (UBYTE               in_DeviceID,
                                     UBYTE             * out_Amplf);
EXTERN UBYTE audio_GetStatus        (UBYTE               in_DeviceID,
                                     audio_Status_Type * out_StatusPtr);
EXTERN UBYTE audio_PlaySoundID      (UBYTE               in_DeviceID,
                                     UBYTE               in_SoundID,
                                     BYTE                in_RelVolume,
                                     UBYTE               in_Repeats);
EXTERN UBYTE audio_PlaySoundbyImage (UBYTE               in_DeviceID,
                                     void              * in_SoundImagePtr,
                                     BYTE                in_RelVolume,
                                     UBYTE               in_Repeats);
EXTERN UBYTE audio_StopSoundbyID    (UBYTE               in_DeviceID,
                                     UBYTE               in_SoundID);
EXTERN UBYTE audio_StopSoundbyImage (UBYTE               in_DeviceID,
                                     void              * in_SoundImagePtr);
EXTERN UBYTE audio_SetSignal        (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE audio_ResetSignal      (drv_SignalID_Type * in_SignalIDPtr);
GLOBAL void  audio_timeout          (T_VSI_THANDLE       handle);
EXTERN void  audio_set_tty          (T_TTY_CMD           tty);
EXTERN void  audio_dyn_set_tty          (T_TTY_CMD           tty);
#endif

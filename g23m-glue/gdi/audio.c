/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DRV_AUDIO
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
|  Purpose :  This Module defines the audio driver interface.
|             for the G23 protocol stack.
|
|             This driver is used to control all audio and audio-related
|             devices of the mobile such as speaker, microphone, etc.
|             A device, e.g. a speaker, may have the capability to play
|             sounds and/or melodies indirectly. This means a sound
|             generator may be attached to a speaker or its amplifier.
|             The format of the sound/melody images is implementation
|             dependent and therefore not in the scope of the generic
|             driver interface. Therefore a application may play pre-
|             defined, driver specific sounds identifying them via a
|             sound ID. In addition the API of this driver has foreseen
|             that an application may copy a sound image into its local
|             buffer, modify it and let it play by the driver. In this
|             case the application has to have the knowledge about the
|             sound image format.
+-----------------------------------------------------------------------------
|  History :  
|		  Apr 04, 2005    REF: ENH 30063 xdeepadh
|		  Description: Buzzer removal activity for Locosto MMI
|		  Fix:All the Buzzer related calls are under the  compilation flag FF_BUZZER.
|			The GSM tones which were playing in the buzzer are now routed to the speaker(Audio).     
+-----------------------------------------------------------------------------
	$History: 

   	              
 	xpradipg - GSM-ENH-32494 : 23 June 2005
 	Description:	Support for various audio profiles
 	Solution:	The support for audio device's carkit/headset/loudspeaker/handheld.
 			The audio device is enabled as per the user actions.	

	June 13, 2005	REF : GSM_ENH_32062 - xpradipg
	Description:The audio uplink not working in the connected call
	Solution:	The audio uplink was configured to mute state at initialization
				The configuration is changed to be in unmute state

	Apr 26, 2005   REF : CRR 30627 xpradipg
	Bug :	Replace the ABB APIs with Audio Service APIs
	Fix :	The ABB APIs are used in muting/unmuting and setting the volume of
			the speaker and microphone. These ABB APIs are replaced with the 
			audio serivce APIs	
******************************************************************************/

#ifndef DRV_AUDIO_C
#define DRV_AUDIO_C
#endif
#define ENTITY_CST

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

/*
in order to stay backwardcompatible this riv_audio define is entered as only
ti15++ version do have the riveria frame it needs to be disabled for the rest
*/

#if 0 //defined (ALR)
#define RIV_AUDIO 1
#else
#undef RIV_AUDIO
#endif

#if defined (_SIMULATION_)
#define _TTY_SIMU
#endif

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "vsi.h"
#include "custom.h"
#ifdef ALR
  #include "p_mphc.h"
#elif defined(FF_GTI)
  /* GTI*/
  /* do nothing */
#else
  #include "p_mph5.h"
#endif  /* ALR, FF_GTI */
#include "prim.h"
#include "gsm.h"
#include "tok.h"
#include "../cst/cst.h"
#include "../cst/cus_cst.h"
#include "gdi.h"
#include "audio.h"

#if defined (RIV_AUDIO)

/* #include "bt_general.h" */
#include "rv/rv_general.h"
#if /*!defined (_TTY_SIMU) &&*/ defined (FF_TTY)
#include "tty/tty_api.h"
#endif
#include "audio/audio_api.h"

#elif AUDIO_TASK

#define	BOOL_FLAG
#define	CHAR_FLAG
#include "../../L1/include/l1_confg.h"
#include "../../L1/include/l1_types.h"
#include "../../L1/audio_cust0/l1audio_cust.h"
#include "../../L1/audio_include/l1audio_msgty.h"
#include "../../L1/audio_include/l1audio_signa.h"

#endif

/*==== DEFINE =====================================================*/

#define AUDIO_TRC_EVENT(_f_)\
        vsi_o_ttrace(VSI_CALLER TC_EVENT, _f_)
#define AUDIO_TRC_EVENT_P1(_f_, _p_)\
        vsi_o_ttrace(VSI_CALLER TC_EVENT, _f_, _p_)
#define AUDIO_TRC_EVENT_P2(_f_, _p1_, _p2_)\
        vsi_o_ttrace(VSI_CALLER TC_EVENT, _f_, _p1_, _p2_)
#define AUDIO_TRC_FUNC(_f_)\
        vsi_o_ttrace(VSI_CALLER TC_FUNC, _f_)
#define AUDIO_TRC_FUNC_P1(_f_, _p_)\
        vsi_o_ttrace(VSI_CALLER TC_FUNC, _f_, _p_)

/*==== EXPORT =====================================================*/

#if defined (NEW_FRAME)
#define CST_AUDIOTIMER 2
#endif

/*==== VARIABLES ==================================================*/

#if defined (RIV_AUDIO)
static T_AUDIO_KEYBEEP_PARAMETER beep;
static T_AUDIO_TONES_PARAMETER t;
#endif

drv_SignalCB_Type       audio_signal_callback = NULL;
#if !defined (NEW_FRAME)
T_VSI_THANDLE           audio_handle = VSI_ERROR;
#endif
T_ACT_TONE              act_tone;
UBYTE                   act_speakerVolume;
UBYTE                   act_micVolume;
UBYTE                   act_buzVolume;
UBYTE                   act_speakerMute;
UBYTE                   act_micMute;

#if defined (NEW_FRAME)
EXTERN T_HANDLE  hCommL1;
#else
EXTERN T_VSI_CHANDLE  hCommL1;
#endif

#if defined (RIV_AUDIO)
void audio_riv_audio_cb (void*);

static T_RV_RETURN_PATH const riv_audio_rp =
{
  0, audio_riv_audio_cb
};

#ifdef FF_TTY
T_AUDIO_TTY_CONFIG_PARAMETER static tty_cfg;
UBYTE static tty_state = FALSE;
UBYTE static tty_cmd = (UBYTE)TTY_OFF;


char const * const audio_mode_names_tty[] = {
  "default",
  "tty_vco",
  "tty_hco",
  "tty_all"
};
#endif
#endif

UBYTE  audio_is_free = TRUE;  /* variable for disable no muting */

/*==== FUNCTIONS ==================================================*/

#if 1 //defined (_TMS470)

#if CONFIG_BUZZER
EXTERN void BZ_Init          (void);
EXTERN void BZ_Enable        (void);
EXTERN void BZ_Disable       (void);
EXTERN void BZ_Tone          (int f);
EXTERN void BZ_Volume        (int v);
#endif

#ifdef ALR
EXTERN void ABB_DlVolume     (UBYTE v);
EXTERN void ABB_DlMute       (UBYTE mute);
EXTERN void ABB_UlMute       (UBYTE value);
EXTERN void ABB_SideTone     (UBYTE v);
/*#include "l1audio_abb.h"   does not work: there's no definition of types UWORDx */
#else
EXTERN void VG_UlVolume      (UBYTE v);
EXTERN void VG_DlVolume      (UBYTE v);
EXTERN void VG_DlMute        (UBYTE mute);
EXTERN void VG_SideTone      (UBYTE v);
#endif

#else

LOCAL  void BZ_Init          (void);
LOCAL  void BZ_Enable        (void);
LOCAL  void BZ_Disable       (void);
LOCAL  void BZ_Tone          (int f);
LOCAL  void BZ_Volume        (int v);

#ifdef ALR
LOCAL  void ABB_DlVolume     (UBYTE v);
LOCAL  void ABB_DlMute       (UBYTE mute);
LOCAL  void ABB_UlMute       (UBYTE value);
LOCAL  void ABB_SideTone     (UBYTE v);
#else
LOCAL  void VG_UlVolume      (UBYTE v);
LOCAL  void VG_DlVolume      (UBYTE v);
LOCAL  void VG_DlMute        (UBYTE mute);
LOCAL  void VG_SideTone      (UBYTE v);
#endif

#endif
LOCAL  void audio_buzzer     (void);
LOCAL  void audio_audio      (void);
LOCAL  void audio_UlMute     (UBYTE mute);

#if defined (RIV_AUDIO) AND !defined (_TTY_SIMU) AND defined (FF_TTY)
LOCAL void audio_tty_return (void *tty_result);
LOCAL void audio_save_def_return (void *result);
LOCAL void audio_load_return (void *result);
LOCAL void audio_dyn_load_return (void *result);
#endif

// 	xpradipg - GSM-ENH-32494 : 23 June 2005
#ifdef FF_MMI_AUDIO_PROFILE
typedef struct 
{
	int status;
}T_AUDIO_MODE_LOAD_DONE;
EXTERN T_AUDIO_RET	audio_mode_load (T_AUDIO_MODE_LOAD *p_parameter, T_RV_RETURN_PATH return_path);
#endif
/*==== CONSTANTS ==================================================*/

#define BUZZER 0
#define AUDIO  1
//	Apr 26, 2005   REF : CRR 30627 xpradipg
#ifdef FF_MMI_SERVICES_MIGRATION
#define AUDIO_SIDE_TONE 175
#endif

#if defined (RIV_AUDIO)

#define TDMA_12      12
#define TDMA_23      23
#define TDMA_43      43
#define TDMA_71      71
#define TDMA_108     108
#define TDMA_130     130
#define TDMA_216     216
#define TDMA_650     650
#define TDMA_866     866

/*
* with the original table spec (USHORT for Freq. and Ampl.),
* we can go down to -31 dB only. Obviously this is sufficient
* for all currently defined sounds.
*/
#define F_425  (( 425 << 5) + 7) /* (( 242 << 8 ) +  35) */
#define F_697  (( 697 << 5) + 7) /* (( 219 << 8 ) +  56) */
#define F_770  (( 770 << 5) + 7) /* (( 211 << 8 ) +  61) */
#define F_852  (( 852 << 5) + 7) /* (( 201 << 8 ) +  67) */
#define F_941  (( 941 << 5) + 7) /* (( 189 << 8 ) +  73) */
#define F_1209 ((1209 << 5) + 5) /* (( 149 << 8 ) + 111) */
#define F_1336 ((1336 << 5) + 5) /* (( 128 << 8 ) + 118) */
#define F_1477 ((1477 << 5) + 5) /* (( 102 << 8 ) + 125) */
#define F_1633 ((1633 << 5) + 5) /* ((  73 << 8 ) + 130) */

#else /* (RIV_AUDIO) */

#if 1 //defined (_TMS470)

/*
 * unit is TDMA frames
 */
#define TDMA_12      12
#define TDMA_23      23
#define TDMA_43      43
#define TDMA_71      71
#define TDMA_108     108
#define TDMA_130     130
#define TDMA_216     216
#define TDMA_650     650
#define TDMA_866     866


#define F_425  (( 242 << 8) + 35)
#define F_697  (( 219 << 8) + 56)
#define F_770  (( 211 << 8) + 61)
#define F_852  (( 201 << 8) + 67)
#define F_941  (( 189 << 8) + 73)
#define F_1209 (( 149 << 8) + 111)
#define F_1336 (( 128 << 8) + 118)
#define F_1477 (( 102 << 8) + 125)
#define F_1633 (( 73  << 8) + 130)

#else

/*
 * unit is milliseconds
 */
#define TDMA_12      55
#define TDMA_23      100
#define TDMA_43      200
#define TDMA_71      330
#define TDMA_108     500
#define TDMA_130     650
#define TDMA_216     1000
#define TDMA_650     3000
#define TDMA_866     4000

#define F_425  (( 35 << 8) + 242)
#define F_697  (( 56 << 8) + 219)
#define F_770  (( 61 << 8) + 211)
#define F_852  (( 67 << 8) + 201)
#define F_941  (( 73 << 8) + 189)
#define F_1209 ((111 << 8) + 149)
#define F_1336 ((118 << 8) + 128)
#define F_1477 ((125 << 8) + 102)
#define F_1633 ((130 << 8) +  73)

#endif

#endif /* (RIV_AUDIO) */

#if 0
extern  const T_DESCR The_Ale_is_dear[111];
extern  const T_DESCR Danza_de_Astureses[21];
extern  const T_DESCR rising_chromatic[47];
extern  const T_DESCR rising_cscale[27];
extern  const T_DESCR MELODY_ONE[11];
extern  const T_DESCR MELODY_HELLO[23];
extern  const T_DESCR bugle[39];
#endif

/*
 * Tone generation tables
 */
#if CONFIG_BUZZER
static const T_DESCR ct_ring []  =   /* command 1     command 2      length     */
                      {    800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           800,          0,             TDMA_12,
                           900,          0,             TDMA_12,
                           0,            0,             TDMA_866,
                           0xFFFF,       0xFFFF,        0
                      };
#else
static const T_DESCR ct_ring []  =   /* command 1     command 2      length     */
                      {    F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           F_770,        0,             TDMA_12,
                           F_941,        0,             TDMA_12,
                           0,            0,             TDMA_866,
                           0xFFFF,       0xFFFF,        0
                      };
#endif

#if CONFIG_BUZZER
static const T_DESCR ct_auth_num [] =   /* command 1     command 2      length     */
                      {    950,          0,             TDMA_71,
                          1400,          0,             TDMA_71,
                          1800,          0,             TDMA_71,
                             0,          0,             TDMA_216,
                           0xFFFF,       0xFFFF,        0
                      };
#else
static const T_DESCR ct_auth_num [] =   /* command 1     command 2      length     */
                      {   F_941,         0,             TDMA_71,
                          F_1477,        0,             TDMA_71,
                          F_1633,        0,             TDMA_71,
                             0,          0,             TDMA_216,
                           0xFFFF,       0xFFFF,        0
                      };
#endif

static const T_DESCR ct_busy []  =   /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_108,
                           0,            0,             TDMA_108,
                           F_425,        0,             TDMA_108,
                           0,            0,             TDMA_108,
                           F_425,        0,             TDMA_108,
                           0,            0,             TDMA_108,
                           F_425,        0,             TDMA_108,
                           0,            0,             TDMA_108,
                           0xFFFF,       0xFFFF,        0
                      };

#if CONFIG_BUZZER
static const T_DESCR ct_congest [] = /* command 1     command 2      length     */
                      {    425,          0,             TDMA_43,
                             0,          0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dropped [] = /* command 1     command 2      length     */
                      {    425,          0,             TDMA_43,
                             0,          0,             TDMA_43,
                           425,          0,             TDMA_43,
                             0,          0,             TDMA_43,
                           425,          0,             TDMA_43,
                             0,          0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_ack []     = /* command 1     command 2      length     */
                      {    425,          0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };
#else
static const T_DESCR ct_congest [] = /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_43,
                             0,          0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dropped [] = /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_43,
                             0,          0,             TDMA_43,
                           F_425,        0,             TDMA_43,
                             0,          0,             TDMA_43,
                           F_425,        0,             TDMA_43,
                             0,          0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_ack []     = /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };
#endif

static const T_DESCR ct_cw []      = /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_43,
                           0,            0,             TDMA_130,
                           F_425,        0,             TDMA_43,
                           0,            0,             TDMA_650,
                           F_425,        0,             TDMA_43,
                           0,            0,             TDMA_130,
                           F_425,        0,             TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_0 []  = /* command 1     command 2      length     */
                      {    F_941,        F_1336,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_1 []  = /* command 1     command 2      length     */
                      {    F_697,        F_1209,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_2 []  = /* command 1     command 2      length     */
                      {    F_697,        F_1336,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_3 []  = /* command 1     command 2      length     */
                      {    F_697,        F_1477,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_4 []  = /* command 1     command 2      length     */
                      {    F_770,        F_1209,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_5 []  = /* command 1     command 2      length     */
                      {    F_770,        F_1336,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_6 []  = /* command 1     command 2      length     */
                      {    F_770,        F_1477,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_7 []  = /* command 1     command 2      length     */
                      {    F_852,        F_1209,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_8 []  = /* command 1     command 2      length     */
                      {    F_852,        F_1336,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_9 []  = /* command 1     command 2      length     */
                      {    F_852,        F_1477,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_a []  = /* command 1     command 2      length     */
                      {    F_697,        F_1633,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_b []  = /* command 1     command 2      length     */
                      {    F_770,        F_1633,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };
static const T_DESCR ct_dtmf_c []  = /* command 1     command 2      length     */
                      {    F_852,        F_1633,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_d []  = /* command 1     command 2      length     */
                      {    F_941,        F_1633,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_s []  = /* command 1     command 2      length     */
                      {    F_941,        F_1209,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_dtmf_h []  = /* command 1     command 2      length     */
                      {    F_941,        F_1477,        TDMA_23,
                           0xFFFF,       0xFFFF,        0
                      };

/* keybeep not used at the moment */
static const T_DESCR ct_keybeep []  = /* command 1     command 2      length     */
                      {    F_697,        F_697,         TDMA_43,
                           0xFFFF,       0xFFFF,        0
                      };

static const T_DESCR ct_ringing []  =   /* command 1     command 2      length     */
                      {    F_425,        0,             TDMA_216,
                           0,            0,             TDMA_866,
                           0xFFFF,       0xFFFF,        0
                      };

/*
 * Tone Type Table
 */
#if CONFIG_BUZZER
static const UBYTE  TONE_TYPE [] = {  BUZZER,     /* ringing tone 0             */
                         BUZZER,     /* ringing tone 1             */
                         BUZZER,     /* ringing tone 2             */
                         BUZZER,     /* ringing tone 3             */
                         BUZZER,     /* ringing tone 4             */
                         BUZZER,     /* ringing tone 5             */
                         BUZZER,     /* ringing tone 6             */
                         BUZZER,     /* ringing tone 7             */
                         BUZZER,     /* ringing tone 8             */
                         BUZZER,     /* ringing tone 9             */
                         BUZZER,     /* ringing tone 10            */
                         BUZZER,     /* ringing tone 11            */
                         BUZZER,     /* ringing tone 12            */
                         BUZZER,     /* ringing tone 13            */
                         BUZZER,     /* ringing tone 14            */
                         BUZZER,     /* ringing tone 15            */
                         BUZZER,     /* error/special information  */
                         AUDIO,      /* subscriber busy            */
                         BUZZER,     /* congestion                 */
                         BUZZER,     /* call dropped               */
                         BUZZER,     /* radio acknowledge          */
                         AUDIO,      /* call waiting               */
                         AUDIO,      /* DTMF digit 0               */
                         AUDIO,      /* DTMF digit 1               */
                         AUDIO,      /* DTMF digit 2               */
                         AUDIO,      /* DTMF digit 3               */
                         AUDIO,      /* DTMF digit 4               */
                         AUDIO,      /* DTMF digit 5               */
                         AUDIO,      /* DTMF digit 6               */
                         AUDIO,      /* DTMF digit 7               */
                         AUDIO,      /* DTMF digit 8               */
                         AUDIO,      /* DTMF digit 9               */
                         AUDIO,      /* DTMF digit A               */
                         AUDIO,      /* DTMF digit B               */
                         AUDIO,      /* DTMF digit C               */
                         AUDIO,      /* DTMF digit D               */
                         AUDIO,      /* DTMF digit *               */
                         AUDIO,      /* DTMF digit #               */
                         AUDIO,      /* keybeep                    */
                         AUDIO       /* ringing tone if NOIBT      */
                      };
#endif

//Apr 04, 2005    REF: ENH 30063 xdeepadh
/*
 * Tone Description Table
    This table will have the  ringing tones and the  gsm tones.
    When the buzzer is not available, the ringing tones will be played with midi ringer
 */
static const T_DESCR * const TONE_DESCR [] =
                       { 
			 ct_ring,    /* ringing tone 0             */
                         ct_ring,    /* ringing tone 1             */
                         ct_ring,    /* ringing tone 2             */
                         ct_ring,    /* ringing tone 3             */
                         ct_ring,    /* ringing tone 4             */
                         ct_ring,    /* ringing tone 5             */
                         ct_ring,    /* ringing tone 6             */
                         ct_ring,    /* ringing tone 7             */
                         ct_ring,    /* ringing tone 8             */
                         ct_ring,    /* ringing tone 9             */
                         ct_ring,    /* ringing tone 10            */
                         ct_ring,    /* ringing tone 11            */
                         ct_ring,    /* ringing tone 12            */
                         ct_ring,    /* ringing tone 13            */
                         ct_ring,    /* ringing tone 14            */
                         ct_ring,    /* ringing tone 15            */
                         ct_auth_num,/* error/special information  */
                         ct_busy,    /* subscriber busy            */
                         ct_congest, /* congestion                 */
                         ct_dropped, /* call dropped               */
                         ct_ack,     /* radio acknowledge          */
                         ct_cw,      /* call waiting               */
                         ct_dtmf_0,  /* DTMF digit 0               */
                         ct_dtmf_1,  /* DTMF digit 1               */
                         ct_dtmf_2,  /* DTMF digit 2               */
                         ct_dtmf_3,  /* DTMF digit 3               */
                         ct_dtmf_4,  /* DTMF digit 4               */
                         ct_dtmf_5,  /* DTMF digit 5               */
                         ct_dtmf_6,  /* DTMF digit 6               */
                         ct_dtmf_7,  /* DTMF digit 7               */
                         ct_dtmf_8,  /* DTMF digit 8               */
                         ct_dtmf_9,  /* DTMF digit 9               */
                         ct_dtmf_a,  /* DTMF digit A               */
                         ct_dtmf_b,  /* DTMF digit B               */
                         ct_dtmf_c,  /* DTMF digit C               */
                         ct_dtmf_d,  /* DTMF digit D               */
                         ct_dtmf_s,  /* DTMF digit *               */
                         ct_dtmf_h,  /* DTMF digit #               */
                         ct_keybeep, /* dummy for keybeep          */
                         ct_ringing  /* ringing tone if NIBT       */
                      };


/*==== CONSTANTS ==================================================*/



// 	xpradipg - GSM-ENH-32494 : 23 June 2005
#ifdef FF_MMI_AUDIO_PROFILE
/*******************************************************************************
 $Function:    	audio_mode_load_cb

 $Description: callback function for the auido_mode_load()

 $Returns:    	none

 $Arguments:  status of the load operation

*******************************************************************************/
void audio_mode_load_cb(void *ret_param)
{

	//if((T_AUDIO_MODE_LOAD_DONE *)ret_param->status != AUDIO_OK)
		AUDIO_TRC_FUNC("ERROR Loading the audio profile");
}

/*******************************************************************************
 $Function:    	audio_full_access_write_cb()

 $Description: callback function for the audio_full_access_write()

 $Returns:    	none

 $Arguments:  status of the write operation

*******************************************************************************/
void audio_full_access_write_cb(void *ret_param)
{
//	if( (T_AUDIO_FULL_ACCESS_WRITE_DONE *)ret_param->status != AUDIO_OK)
		AUDIO_TRC_FUNC ("ERROR setting the value");
}
#if 0
/*******************************************************************************
 $Function:    	audio_speaker_volume_cb()

 $Description: callback function for the audio_speaker_volume()

 $Returns:    	none

 $Arguments:  status of the speaker volume setting operation

*******************************************************************************/
void audio_speaker_volume_cb(ret_param)
{
	if((T_AUDIO_SPEAKER_VOLUME_DONE)ret_param->status != AUDIO_OK)
		AUDIO_TRC_FUNC ("ERROR setting the volume");
}
#endif
/*******************************************************************************
 $Function:    	audio_set_device()

 $Description: initiates the audio_mode_load() for various devices

 $Returns:    	none

 $Arguments:  successfull execution of the audio_mode_load operation

*******************************************************************************/
int audio_set_device(char* mode)
{
	T_RV_RETURN_PATH return_path = {0,audio_mode_load_cb};
	T_AUDIO_RET status;
	T_AUDIO_MODE_LOAD p_parameter;

	strcpy((char*)p_parameter.audio_mode_filename,mode);
	status = audio_mode_load(& p_parameter,return_path);
	return status;

}

/*******************************************************************************
 $Function:    	audio_set_path()

 $Description: sets the audio speaker path to the current audio device and also either to voice or
 			stereo 

 $Returns:    	none

 $Arguments:  
 			audioDevice - Indicates the device to which the path has to be set
 			stereo - If false the path is for voice 
 				      If true the path is for stereo
*******************************************************************************/

void audio_set_path(int audioDevice,int stereo)
{
	T_RV_RETURN_PATH return_path = {0,audio_full_access_write_cb};
	T_AUDIO_FULL_ACCESS_WRITE	p_parameter;
	int status;	
	if( stereo)
		p_parameter.variable_indentifier = AUDIO_STEREO_SPEAKER_MODE;
	else
		p_parameter.variable_indentifier = AUDIO_SPEAKER_MODE;
	p_parameter.data =  (void*)&audioDevice;
	status = audio_full_access_write(&p_parameter,return_path);

}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_Init                 |
+--------------------------------------------------------------------+

  PURPOSE : The function initializes the driver's internal data.
            The function returns DRV_OK in case of a successful
            completition. The function returns DRV_INITIALIZED if
            the driver has already been initialized and is ready to
            be used or is already in use. In case of an initialization
            failure, which means the that the driver cannot be used,
            the function returns DRV_INITFAILURE.

*/
// 	xpradipg - GSM-ENH-32494 : 23 June 2005
//	the interface for the audio profiles has changed, it accepts an array of 
//   type T_ACCESSORY_CALLBACK
#ifdef FF_MMI_AUDIO_PROFILE
GLOBAL UBYTE audio_Init(T_ACCESSORY_CALLBACK *audioConfig)
#else
GLOBAL UBYTE audio_Init (drv_SignalCB_Type in_SignalCBPtr)
#endif
{
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	local variable declarations 
#ifdef FF_MMI_SERVICES_MIGRATION
	T_AUDIO_FULL_ACCESS_WRITE side_tone_cfg;
	T_RV_RETURN_PATH return_path = {0, NULL};
	T_AUDIO_RET result;
	INT8 side_tone = AUDIO_SIDE_TONE;
#endif	
// 	xpradipg - GSM-ENH-32494 : 23 June 2005
#ifdef FF_MMI_AUDIO_PROFILE
T_AUDIO_RET status;
T_AUDIO_MODE_LOAD handheld= {"handheld"};
#endif

// 	xpradipg - GSM-ENH-32494 : 23 June 2005
//	the callback functions for the headset and carkit indication are registered 
//	and the defualt mode handheld is loaded
#ifdef FF_MMI_AUDIO_PROFILE
#if 0
	status = audio_accessory_register_notification(audioConfig[CALLBACK_HEADSET],AUDIO_ACCESSORY_HEADSET);
	if( status != AUDIO_OK)
	{
		AUDIO_TRC_FUNC("Error - registering the callback for headset");
	}
	
	status = audio_accessory_register_notification(audioConfig[CALLBACK_CARKIT],AUDIO_ACCESSORY_CARKIT);
	if(status != AUDIO_OK)
	{
		AUDIO_TRC_FUNC("Error - registering the callback for carkit");
	}
#endif
	return_path.callback_func = audio_mode_load_cb;
	status = audio_mode_load(&handheld, return_path);
	if (status != AUDIO_OK)
	{
		AUDIO_TRC_FUNC("Error - loading the handheld profile");
	}
#else
  audio_signal_callback   = in_SignalCBPtr;     /* store call-back function */
#endif

#if !defined (NEW_FRAME)
  audio_handle = vsi_t_open (VSI_CALLER "AUDIO");

  if (audio_handle < VSI_OK)
    return DRV_INITFAILURE;
#endif
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//For Buzzer init 
#if CONFIG_BUZZER
  BZ_Init ();             /* initialize buzzer */
#endif
  audio_SetMute  ( AUDIO_SPEAKER,    AUDIO_MUTING_OFF );
  audio_SetMute  ( AUDIO_MICROPHONE, AUDIO_MUTING_OFF );

  audio_SetAmplf ( AUDIO_SPEAKER,    175 );
  audio_SetAmplf ( AUDIO_MICROPHONE, 75  );
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//Set volume for Buzzer
#if CONFIG_BUZZER
  audio_SetAmplf ( AUDIO_BUZZER,     175 );
#endif

/* Setting the Side Tone has been moved from audio_SetAmplf CQ21055*/
#ifdef ALR
 #ifdef FF_TTY
    if (!tty_state)
 #endif
    {
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	set the sidetone value to 175 this is the optimum value    
#ifdef FF_MMI_SERVICES_MIGRATION
    	side_tone_cfg.variable_indentifier = AUDIO_MICROPHONE_SPEAKER_LOOP_SIDETONE;
    	side_tone_cfg.data = (void*)&side_tone;
    	result = audio_full_access_write(&side_tone_cfg, return_path);
#else    	
	    ABB_SideTone (175);
#endif	    
    }
#else
     VG_SideTone (175); 
#endif


  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_Exit                 |
+--------------------------------------------------------------------+

  PURPOSE : The function is called when the driver functionality is
            not longer required.
*/

GLOBAL void audio_Exit (void)
{
  audio_signal_callback = NULL;
#if !defined (NEW_FRAME)
  vsi_t_close (VSI_CALLER audio_handle);
  audio_handle = VSI_ERROR;
#endif
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//Disable Buzzer
#if CONFIG_BUZZER
  BZ_Disable ();
#endif
  act_tone.status = NO_TONE_STATE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_SetMute              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to enable or disable muting for
            the device identified by the parameter in_Device. If the
            mode for the specified device could be changed, the
            function returns DRV_OK. If the specified device is
            unknown or the device does not support muting, the function
            returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_SetMute (UBYTE in_DeviceID, UBYTE in_Mode)
{
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	local variable declarations
#ifdef FF_MMI_SERVICES_MIGRATION
	T_AUDIO_FULL_ACCESS_WRITE	p_parameter;
	T_AUDIO_FULL_ACCESS_READ	p_parameter1;
	T_RV_RETURN_PATH            return_path = {0, NULL};
	T_AUDIO_RET					result;
	static INT8					speaker_volume = 0;
	INT8						setvolume = AUDIO_SPEAKER_VOLUME_MUTE;
#endif	
	
  if (in_DeviceID EQ AUDIO_SPEAKER)
  {
    if (in_Mode AND audio_is_free)
    {
      TRACE_EVENT ("No speaker muting");
#ifdef ALR
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	the previous volume is restored when we comeout of mute state (unmuting)
//	At initialization we do not write the value since the new value is written
#ifdef FF_MMI_SERVICES_MIGRATION
		p_parameter.variable_indentifier = AUDIO_SPEAKER_VOLUME_LEVEL;
		p_parameter.data = (void*)&speaker_volume;
		result = audio_full_access_write(&p_parameter,return_path);
		if(result != 0)
			TRACE_FUNCTION("error -  setting speaker volume");
#else
      ABB_DlMute (0);
#endif      
#else
      VG_DlMute (0);
#endif
      act_speakerMute = AUDIO_MUTING_OFF;
    }
    else
    {
      TRACE_EVENT ("Speaker muting");
#ifdef ALR
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	read the current value and store it to before setting it to mute
#ifdef FF_MMI_SERVICES_MIGRATION
		p_parameter1.variable_indentifier = AUDIO_SPEAKER_VOLUME_LEVEL;
		p_parameter1.data = (void*)&speaker_volume;
		result = audio_full_access_read(&p_parameter1);
		p_parameter.variable_indentifier = AUDIO_SPEAKER_VOLUME_MUTE;
		p_parameter.data = (void*)&setvolume;
		result = audio_full_access_write(&p_parameter,return_path);
#else
      ABB_DlMute (1);
#endif      
#else
      VG_DlMute (1);
#endif
      act_speakerMute = AUDIO_MUTING_ON;
    }
  }
  else if (in_DeviceID EQ AUDIO_MICROPHONE)
  {
    if (in_Mode AND audio_is_free)
    {
      TRACE_EVENT ("No microphone muting");
      audio_UlMute (0);
      act_micMute = AUDIO_MUTING_OFF;
    }
    else
    {
      TRACE_EVENT ("Microphone muting");
      audio_UlMute (1);
      act_micMute = AUDIO_MUTING_ON;
    }
  }
  else
  {
    return DRV_INVALID_PARAMS;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_GetMute              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to get the status whether muting
            is enabled or disabled for the device identified by the
            parameter in_Device. If the specified device is unknown
            or the device does not support muting, the function
            returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_GetMute (UBYTE in_DeviceID, UBYTE* out_Mode)
{
  if (in_DeviceID EQ AUDIO_SPEAKER)
  {
    *out_Mode = act_speakerMute;
  }
  else if (in_DeviceID EQ AUDIO_MICROPHONE)
  {
    *out_Mode = act_micMute;
  }
  else
  {
    return DRV_INVALID_PARAMS;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_GetStatus            |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to retrieve the status of the driver.
            In case of a successful completion the driver returns
            DRV_OK and the current status of the driver in the buffer
            out_StatusPtr points to.
            In case the driver is not configured yet, it returns
            DRV_NOTCONFIGURED. In this case the contents of the
            buffer out_StatusPtr is invalid.
            In case out_StatusPtr equals NULL or device is unknown
            the driver returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_GetStatus (UBYTE               in_DeviceID,
                              audio_Status_Type * out_StatusPtr)
{
  if ( out_StatusPtr EQ NULL )
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    if (in_DeviceID EQ AUDIO_SPEAKER)
    {
      out_StatusPtr -> min_volume = AUDIO_MIN_VOLUME;
      out_StatusPtr -> max_volume = AUDIO_MAX_VOLUME;
    }
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//If Buzzer is available, set the minimum and maximum volume
#if CONFIG_BUZZER
    else if (in_DeviceID EQ AUDIO_BUZZER)
    {
      out_StatusPtr -> min_volume = AUDIO_MIN_BUZ_VOLUME;
      out_StatusPtr -> max_volume = AUDIO_MAX_BUZ_VOLUME;
    }
#endif
    else if (in_DeviceID EQ AUDIO_MICROPHONE)
    {
      out_StatusPtr -> min_volume = AUDIO_MIN_MIC_VOLUME;
      out_StatusPtr -> max_volume = AUDIO_MAX_MIC_VOLUME;
    }
    else
    {
      return DRV_INVALID_PARAMS;
    }
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_GetSoundImage        |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to copy the image of a driver
            internal sound image into an application specific sound
            image buffer. The application may modify the sound.
            In case of a successful completion the function returns
            DRV_OK. In case the size of the buffer where the sound image
            shall be copied to is too small the driver returns
            DRV_INVALID_PARAMS. In case a specific driver implementation
            does not support this functionality the driver returns
            DRV_FCT_NOTSUPPORTED.

*/

GLOBAL UBYTE audio_GetSoundImage (UBYTE   in_SoundID,
                                  void  * out_SoundImagePtr)
{
  return AUDIO_FCT_NOTSUPPORTED;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_SetAmplf             |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to set the amplification for the
            device identified by the parameter in_DeviceID.
            In the case of a speaker this is the volume, for a
            microphone - the pre-amplifier that regulates the
            sensitivity of the microphone. The valid range depends on
            the hardware used. If the amplification could be changed
            for the specified device, the function returns DRV_OK. If
            the amplification value (in_Amplf) is out of range or the
            specified device is unknown or the specified device does
            not support the amplification setting, the function
            returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_SetAmplf (UBYTE in_DeviceID, UBYTE in_Amplf)
{
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	local variable 
#ifdef FF_MMI_SERVICES_MIGRATION
	T_AUDIO_FULL_ACCESS_WRITE	p_parameter;
	T_RV_RETURN_PATH          	return_path = {0, NULL};
	T_AUDIO_RET					result;
	INT8						volume;
#endif	
  if (in_DeviceID EQ AUDIO_MICROPHONE)
  {
    act_micVolume = 75;         /* voice quality is best at this microphone volume */
#ifdef ALR
#else
    VG_UlVolume (act_micVolume);
#endif
  }
  else if (in_DeviceID EQ AUDIO_SPEAKER)
  {
    TRACE_EVENT_P1("VOLUME=%d", in_Amplf);

#ifdef ALR
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	The ABB values are converted to the corresponding audio service values
#ifdef FF_MMI_SERVICES_MIGRATION
	switch (in_Amplf)
	{
		case 255:
		volume = AUDIO_SPEAKER_VOLUME_0dB;
		break;
		case 190:
		volume = AUDIO_SPEAKER_VOLUME_6dB;
		break;
		case 125:
		volume = AUDIO_SPEAKER_VOLUME_12dB;
		break;
		case 60:
		volume = AUDIO_SPEAKER_VOLUME_18dB;
		break;
		case 1:
		volume = AUDIO_SPEAKER_VOLUME_24dB;
		break;
		default:
		volume = AUDIO_SPEAKER_VOLUME_12dB;
		break;
	}
	p_parameter.variable_indentifier = AUDIO_SPEAKER_VOLUME_LEVEL;
	p_parameter.data				=  (void*)&volume;
	result = audio_full_access_write(&p_parameter, return_path);
	if( result != 0)
		TRACE_FUNCTION("set volume failed");
#else
    ABB_DlVolume (in_Amplf);    /* output volume */
#endif
#ifdef FF_TTY
    if (!tty_state)
#endif
    {
      /* ABB_SideTone (175); */  /* To avoid the side tone reseting */
    }
#else
    VG_DlVolume (in_Amplf);     /* output volume */
    /* VG_SideTone (175); */ /* To avoid the side tone reseting */
#endif
    act_speakerVolume = in_Amplf;
  }
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//If  Buzzer is available , set the volume
#if CONFIG_BUZZER
  else if (in_DeviceID EQ AUDIO_BUZZER)
  {
    act_buzVolume = in_Amplf;
  }
#endif
  else
  {
    return DRV_INVALID_PARAMS;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_GetAmplf             |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to get the amplification for the
            device identified by the parameter in_DeviceID.
            In the case of a speaker this is the volume, for a
            microphone - the pre-amplifier that regulates the
            sensitivity of the microphone. The valid range depends on
            the hardware used.If the specified device is unknown the
            function returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_GetAmplf (UBYTE in_DeviceID, UBYTE* out_Amplf)
{
  if (in_DeviceID EQ AUDIO_MICROPHONE)
  {
    *out_Amplf = act_micVolume;
  }
  else if (in_DeviceID EQ AUDIO_SPEAKER)
  {
    *out_Amplf = act_speakerVolume;
  }
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//If Buzzer is available, set the out volume
#if CONFIG_BUZZER
  else if (in_DeviceID EQ AUDIO_BUZZER)
  {
    *out_Amplf = act_buzVolume;
  }
#endif
  else
  {
    return DRV_INVALID_PARAMS;
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_PlaySoundID          |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to play a sound or melody. The
            function returns immediately after the "play process" has
            been activated. It is implementation dependent if the
            device/driver supports playing multiple sounds simultan-
            eously, i.e. accepting multiple calls of audio_PlaySoundID().
            If the calling process should be notified when the sound has
            stopped playing automatically, the signal AUDIO_SIGTYPE_
            SOUNDEND must be set using the audio_SetSignal() function.
            If the special driver implementation or the device does
            not support volume control, the driver ignores the value.
            If the sound can be played, the function returns DRV_OK.
            If the device is currently playing the sound identified by
            the parameter in_SoundID, the function returns DRV_INPROCESS.
            If the device/driver is currently playing a sound, but does
            not support playing multiple sounds simultaneously, the
            driver returns DRV_INPROCESS.

*/

GLOBAL UBYTE audio_PlaySoundID (UBYTE in_DeviceID,
                                UBYTE in_SoundID,
                                BYTE  in_RelVolume,
                                UBYTE in_Repeats)
{

TRACE_EVENT_P1("audio_PlaySoundID in_SoundID is %d",in_SoundID);
  /*
   * switch off current tone
   */
  switch (act_tone.status)
  {

//Apr 04, 2005    REF: ENH 30063 xdeepadh  
//If Buzzer is available, disable the buzzer
#if CONFIG_BUZZER
    case BUZZER_ON:
      BZ_Disable ();
    case BUZZER_SILENT:
#endif
      /*
       * No Break
       */
    case TONE_ON:
    case TONE_SILENT:
#if defined (NEW_FRAME)
      vsi_t_stop (VSI_CALLER CST_AUDIOTIMER);
#else
      vsi_t_stop (VSI_CALLER audio_handle);
#endif
      break;
    default:
      break;
  }

  /*
   * configure new tone
   */
  if (in_SoundID EQ TONES_KEYBEEP)      /* that is keybeep */
  {
#if defined (RIV_AUDIO)
    beep.frequency_beep[0] = ((F_697) >> 5) & 0x07ff;
    beep.amplitude_beep[0] = -((char) ((F_697) & 0x001f));
    beep.frequency_beep[1] = ((F_697) >> 5) & 0x07ff;
    beep.amplitude_beep[1] = -((char) ((F_697) & 0x001f));
    beep.duration = 120;
  #ifdef _TARGET_
    /*
     * This function seems only to be available on target. (db / 2001-07-16)
     */
    return audio_keybeep_start(beep,riv_audio_rp);
  #endif
#else
  /* MPHC is not present in GTI case */
  #if !defined(FF_GTI) && AUDIO_TASK
    PALLOC (keybeep, MMI_KEYBEEP_REQ);

    keybeep->d_k_x1_kt0    = F_697;
    keybeep->d_k_x1_kt1    = F_697;
    keybeep->d_dur_kb      = 6;        /* equal 120 ms */
    PSENDX (L1, keybeep);
  #endif /* FF_GTI */
#endif
  }
  else
  {
    act_tone.call_tone   = in_SoundID;
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//Before playing the tone, set the status and the tone type . 
#if CONFIG_BUZZER
    act_tone.status      = BUZZER_SILENT;
    act_tone.type        = (UBYTE)TONE_TYPE[in_SoundID];
#else			//If Buzzer is not available
    act_tone.status      = TONE_SILENT;
    act_tone.type        = (UBYTE)AUDIO;
#endif
    act_tone.descr       = (T_DESCR *)TONE_DESCR[in_SoundID];
    act_tone.style       = in_Repeats;
    act_tone.descr_index = 0;
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//If Buzzer is available, call the audio_buzzer
#if CONFIG_BUZZER
    if (act_tone.type EQ BUZZER)
    {
      act_tone.volume = act_buzVolume;
      audio_buzzer ();
    }
    else
#endif
    {
      act_tone.volume = act_speakerVolume;
      audio_audio ();
    }
  }

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_PlaySoundbyImage     |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to play a sound or melody. The
            image of the sound/melody is passed to the driver (the
            sound image format is implmementation dependent).

*/

GLOBAL UBYTE audio_PlaySoundbyImage (UBYTE   in_DeviceID,
                                     void  * in_SoundImagePtr,
                                     BYTE    in_RelVolume,
                                     UBYTE   in_Repeats)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_StopSoundbyID        |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to manually stop playing a sound
            or melody. When a sound is stopped manually, no signal is
            created as to whether or not the signal AUDIO_SIGTYP_SOUNDEND
            has been defined. If the function could stop playing the
            specified sound, the function returns DRV_OK. If the device
            is unknown or does not support this function or the specified
            sound ID is invalid, the function returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_StopSoundbyID (UBYTE in_DeviceID,
                                  UBYTE in_SoundID)
{

TRACE_EVENT_P1("audio_StopSoundbyID in_SoundID is %d",in_SoundID);
  /*
   * switch off current tone
   */
  switch (act_tone.status)
  {
//Apr 04, 2005    REF: ENH 30063 xdeepadh   
//If Buzzer is available, disable it.
#if CONFIG_BUZZER
    case BUZZER_ON:
      BZ_Disable ();
    case BUZZER_SILENT:
#endif
      /*
       * No Break
       */
    case TONE_ON:
    case TONE_SILENT:

      /* Stop timer for reload */

#if defined (NEW_FRAME)
      vsi_t_stop (VSI_CALLER CST_AUDIOTIMER);
#else
      vsi_t_stop (VSI_CALLER audio_handle);
#endif

      if (act_tone.status EQ TONE_ON)
      {
      /*
       * stop the current playing audio tone immediately
       */
#if defined (RIV_AUDIO)
  #ifdef _TARGET_
        /*
         * This function seems only to be available on target. (db / 2001-07-16)
         */
        audio_tones_stop(riv_audio_rp);
  #endif
#else
  #if !defined(FF_GTI) && AUDIO_TASK
        PALLOC (audio, MMI_TONE_REQ);

        audio->d_k_x1_t0 = 0;
        audio->d_k_x1_t1 = 0;
        audio->d_k_x1_t2 = 0;
        audio->d_pe_rep  = 1;
        audio->d_pe_off  = 0;
        audio->d_se_off  = 0;
        audio->d_bu_off  = 0;
        audio->d_t0_on   = 0;
        audio->d_t0_off  = 0;
        audio->d_t1_on   = 0;
        audio->d_t1_off  = 0;
        audio->d_t2_on   = 0;
        audio->d_t2_off  = 0;

        PSENDX (L1, audio);
  #endif /* FF_GTI */
#endif
      }
      act_tone.status = NO_TONE_STATE;

      break;
    default:
      break;
  }

  return DRV_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_StopSoundbyImage     |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to manually stop playing a sound
            or melody. When a sound is stopped manually, no signal is
            created as to whether or not the signal AUDIO_SIGTYP_SOUNDEND
            has been defined. If the function could stop playing the
            specified sound image, the function returns DRV_OK.
            If the device is unknown or does not support this function
            or the specified sound ID is invalid, the function
            returns DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE audio_StopSoundbyImage (UBYTE   in_DeviceID,
                                     void  * in_SoundImagePtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_SetSignal            |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to defines a single signal or multiple
            signals that is/are indicated to the process when the
            event identified in the signal information data type as
            SignalType occurs. To remove a signal call the function
            audio_ResetSignal(). If one of the parameters of the
            signal information data is invalid, the function returns
            DRV_INVALID_PARAMS. If no signal call-back function has
            been defined at the time of initialization, the driver
            returns DRV_SIGFCT_NOTAVAILABLE.

*/

GLOBAL UBYTE audio_SetSignal (drv_SignalID_Type * in_SignalIDPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_ResetSignal          |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to remove a previously set single
            multiple signals. The signals that are removed are identified
            by the Signal Information Data element Signal Type. All other
            elements of the signal information data must be identical
            to the signal(s) that is/are to be removed.

*/

GLOBAL UBYTE audio_ResetSignal (drv_SignalID_Type * in_SignalIDPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_timeout              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used after timeout of the audio timer.

*/

#if defined (NEW_FRAME)
GLOBAL void audio_timeout (USHORT index)
#else
GLOBAL void audio_timeout (T_VSI_THANDLE handle)
#endif
{
#if defined (NEW_FRAME)
  if (index EQ CST_AUDIOTIMER)
#else
  if (handle EQ audio_handle)
#endif
  {
    /*ccc
     * only if it is the audio timer
     */
//Apr 04, 2005    REF: ENH 30063 xdeepadh
//For Buzzer
#if CONFIG_BUZZER
    if (act_tone.type EQ BUZZER)
      audio_buzzer ();
    else
#endif
      audio_audio ();
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_buzzer               |
+--------------------------------------------------------------------+

  PURPOSE : Process a buzzer tone.

*/
//Apr 04, 2005    REF: ENH 30063 xdeepadh
#if CONFIG_BUZZER
LOCAL void audio_buzzer (void)
{
  if (act_tone.status EQ NO_TONE_STATE)
    return;

  switch (act_tone.descr
          [act_tone.descr_index].command_1)
  {
    case 0:
      /*
       * switch buzzer off
       */
      BZ_Disable ();
#if defined (NEW_FRAME)
      vsi_t_start (VSI_CALLER CST_AUDIOTIMER,
                   act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#else
      vsi_t_start (VSI_CALLER audio_handle,
                   act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#endif
      act_tone.status = BUZZER_SILENT;
      break;

    case 0xFFFE:
      /*
       * switch buzzer on continously
       */
      BZ_Enable ();
      BZ_Volume (act_buzVolume);
      BZ_Tone   (act_tone.descr
                 [act_tone.descr_index].command_1);
      act_tone.status = BUZZER_ON;
      break;

    case 0xFFFF:
      /*
       * end of list
       */
      BZ_Disable ();
      if (act_tone.style EQ AUDIO_PLAY_INFINITE)
      {
        act_tone.descr_index = 0;
        audio_buzzer ();
      }
      else
        act_tone.status = NO_TONE_STATE;
      break;

    default:
      /*
       * switch buzzer on
       */
      BZ_Enable ();
      BZ_Volume (act_buzVolume);
      BZ_Tone   (act_tone.descr
                     [act_tone.descr_index].command_1);
#if defined (NEW_FRAME)
      vsi_t_start (VSI_CALLER
                   CST_AUDIOTIMER,
                   act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#else
      vsi_t_start (VSI_CALLER
                       audio_handle,
                   act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#endif
      act_tone.status = BUZZER_ON;
      break;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_audio                |
+--------------------------------------------------------------------+

  PURPOSE : Process an audio tone.

*/

LOCAL void audio_audio (void)
{

TRACE_EVENT("audio_audio");
  if (act_tone.status EQ NO_TONE_STATE)
    return;

  switch (act_tone.descr
          [act_tone.descr_index].command_1)
  {
    case 0:
      /*
       * switch audio off
       */
#if defined (NEW_FRAME)
      vsi_t_start (VSI_CALLER CST_AUDIOTIMER,
                   act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#else
      vsi_t_start (VSI_CALLER audio_handle,
                       act_tone.descr
                   [act_tone.descr_index++].length*60/13);
#endif
      act_tone.status = TONE_SILENT;
      break;

    case 0xFFFF:
      /*
       * end of list
       */
      if (act_tone.style EQ AUDIO_PLAY_INFINITE)
      {
        act_tone.descr_index = 0;
        audio_audio ();
      }
      else
        act_tone.status = NO_TONE_STATE;
      break;

    default:
      /*
       * switch audio tone on
       */
      {
#if defined (RIV_AUDIO)
        USHORT fa1 = act_tone.descr[act_tone.descr_index].command_1;
        USHORT fa2 = act_tone.descr[act_tone.descr_index].command_2;
        ULONG len = act_tone.descr[act_tone.descr_index].length;
        t.tones[0].start_tone = 0;
        t.tones[0].stop_tone = (USHORT) ((60 * len) / 13);
        t.tones[0].frequency_tone = (fa1 >> 5) & 0x07ff;
        t.tones[0].amplitude_tone = -((char) (fa1 & 0x001f));
        t.tones[1].start_tone = 0;
        t.tones[1].stop_tone = (USHORT) ((60 * len) / 13);
        t.tones[1].frequency_tone = (fa2 >> 5) & 0x07ff;
        t.tones[1].amplitude_tone = -((char) (fa2 & 0x001f));
        t.tones[2].start_tone = 0;
//     t.tones[2].stop_tone = 1; - NDH : Removed as this does not comply with the i/f spec
        t.tones[2].stop_tone = (USHORT) ((60 * len) / 13);
        t.tones[2].frequency_tone = 0;
        t.tones[2].amplitude_tone = 0;
        t.frame_duration = (USHORT) ((60 * len) / 13);
        t.sequence_duration = (USHORT) ((60 * len) / 13);   /* why 120 ??? -> changed to 60 */
        t.period_duration = (USHORT) ((60 * len) / 13);     /*         -"-                  */
        t.repetition = 1;
  #ifdef _TARGET_
        /*
         * This function seems only to be available on target. (db / 2001-07-16)
         */
        audio_tones_start(&t,riv_audio_rp);
  #endif
#else
  #if !defined(FF_GTI) && AUDIO_TASK
        PALLOC (audio, MMI_TONE_REQ);
          /*
           * Convert TDMA frames to 20 ms ->  3/13
           */

          audio->d_k_x1_t0 = act_tone.descr[act_tone.descr_index].command_1;
          audio->d_k_x1_t1 = act_tone.descr[act_tone.descr_index].command_2;
          audio->d_k_x1_t2 = 0;
          audio->d_pe_rep  = 1;
          audio->d_pe_off  = (6*act_tone.descr[act_tone.descr_index].length) / 13;
          audio->d_se_off  = (6*act_tone.descr[act_tone.descr_index].length) / 13;
          audio->d_bu_off  = (3*act_tone.descr[act_tone.descr_index].length) / 13;
          audio->d_t0_on   = 0;
          audio->d_t0_off  = (3*act_tone.descr[act_tone.descr_index].length) / 13;
          audio->d_t1_on   = 0;
          audio->d_t1_off  = (3*act_tone.descr[act_tone.descr_index].length) / 13;
          audio->d_t2_on   = 0;
          audio->d_t2_off  = 0;

          PSENDX (L1, audio);
  #endif /* FF_GTI */
#endif

#if defined (NEW_FRAME)
          vsi_t_start (VSI_CALLER CST_AUDIOTIMER,
                       act_tone.descr
                       [act_tone.descr_index++].length*60/13);
#else
          vsi_t_start (VSI_CALLER audio_handle,
                       act_tone.descr
                       [act_tone.descr_index++].length*60/13);
#endif
          act_tone.status = TONE_ON;
      }
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_UlMute               |
+--------------------------------------------------------------------+

  PURPOSE : This function mutes the uplink path.

*/
LOCAL void audio_UlMute (UBYTE mute)
{
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	local variable definition
#ifdef FF_MMI_SERVICES_MIGRATION
	T_AUDIO_FULL_ACCESS_WRITE	p_parameter;
	T_AUDIO_FULL_ACCESS_READ	p_parameter1;
	T_RV_RETURN_PATH            return_path = {0, NULL};
	T_AUDIO_RET					result;
//	June 13, 2005	REF : GSM_ENH_32062 - xpradipg
//	the default value is assigned to AUDIO_MICROPHONE_OUTPUT_BIAS_2_0V	
	static INT8					microphone_gain = AUDIO_MICROPHONE_OUTPUT_BIAS_2_0V;
	INT8						setgain = AUDIO_MICROPHONE_MUTE;
#endif
  if (mute)
  {
#ifdef ALR
//	June 13, 2005	REF : GSM_ENH_32062 - xpradipg
//	the audio is unmuted for the variable "mute" with value greater than "0"
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	read the microphone gain, store it and mute the microphone
#ifdef FF_MMI_SERVICES_MIGRATION		
		p_parameter1.variable_indentifier = AUDIO_MICROPHONE_GAIN;
		p_parameter1.data = (void*)&microphone_gain;
		result = audio_full_access_read(&p_parameter1);
		if(result != 0)
		{
			TRACE_EVENT("error reading the handeld parameters");
		}
		p_parameter.variable_indentifier = AUDIO_MICROPHONE_GAIN;
		setgain = AUDIO_MICROPHONE_MUTE;
		p_parameter.data = (void*)&setgain;
		result = audio_full_access_write(&p_parameter, return_path); 	
		if(result != 0)
			TRACE_EVENT("ERROR - not written1");
#else
    ABB_UlMute (1);
#endif    
#else
    VG_UlVolume (0);
#endif
  }
  else
  {
#ifdef ALR
//	June 13, 2005	REF : GSM_ENH_32062 - xpradipg
//	the audio is muted for the variable "mute" with value <= to "0"
//	Apr 26, 2005   REF : CRR 30627 xpradipg
//	restore the volume back once the unmuting is done
#ifdef FF_MMI_SERVICES_MIGRATION	
		p_parameter.variable_indentifier = (UINT8)AUDIO_MICROPHONE_GAIN ;
		p_parameter.data				=  (void*)&microphone_gain;
		result = audio_full_access_write(&p_parameter, return_path); 	
		if(result != 0)
			TRACE_EVENT("ERROR - not written");
#else
    ABB_UlMute (0);
#endif    
#else
    VG_UlVolume (act_micVolume);
#endif
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_AUDIO                  |
| STATE   : code                ROUTINE : audio_set_tty              |
+--------------------------------------------------------------------+

  PURPOSE : This function (de)activates the TTY codec.

*/

#if defined (FF_TTY) || defined (_TTY_SIMU)
GLOBAL void audio_set_tty (T_TTY_CMD tty)
{
  if (tty < TTY_OFF OR tty > TTY_ALL)
  {
    TRACE_EVENT_P1 ("TTY invalid parameter: %d", tty);
    return;
  }
  if (tty NEQ TTY_OFF)
  {
#if defined (RIV_AUDIO) && !defined (_TTY_SIMU)
    T_AUDIO_MODE_SAVE audio_mode;
    T_RV_RETURN_PATH ret = {NOT_PRESENT_8BIT, NULL};
    T_AUDIO_RET audio_ret;

    if (!tty_state)
    {
      tty_cfg.Mode = TTY_EXT_START;
      tty_cfg.ThresholdRead = tty_cfg.ThreshHoldWrite = 0;

      ret.callback_func = audio_save_def_return;
      strcpy (audio_mode.audio_mode_filename, audio_mode_names_tty[0]);
      audio_ret = audio_mode_save (&audio_mode, ret);

      if (audio_ret NEQ AUDIO_OK)
      {
        AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE SAVE def: %d", (int)audio_ret);
      }
      else
      {
        AUDIO_TRC_FUNC ("AUDIO MODE SAVE def");
        tty_cmd = (UBYTE)tty;
      }
    }
    else
    {
      AUDIO_TRC_EVENT ("TTY running");
    }
#else
    TRACE_EVENT_P1 ("TTY start: %d", tty);
#endif
  }
  else
  {
#if defined (RIV_AUDIO) && !defined (_TTY_SIMU)
    T_RV_RETURN_PATH ret = {NOT_PRESENT_8BIT, NULL};
    T_AUDIO_RET audio_ret;

    if (tty_state)
    {
      ret.callback_func = audio_tty_return;
      tty_cfg.ThresholdRead = tty_cfg.ThreshHoldWrite = 0;
      tty_cfg.Mode = TTY_STOP;
      audio_ret = audio_tty_set_config (&tty_cfg, &ret);

      if (audio_ret NEQ AUDIO_OK)
      {
        tty_state = FALSE;
        AUDIO_TRC_EVENT_P1 ("Error TTY stop: %d", (int)audio_ret);
      }
      else
      {
        AUDIO_TRC_FUNC ("TTY to be stopped");
      }
    }
    else
    {
      AUDIO_TRC_FUNC ("TTY already ended");
    }
#else
    TRACE_EVENT ("TTY stop");
#endif
  }
}

/* This function changes the TTY mode during an active TTY call */
GLOBAL void audio_dyn_set_tty (T_TTY_CMD tty)
{
  T_AUDIO_MODE_LOAD audio_mode;
  T_RV_RETURN_PATH load_ret = {NOT_PRESENT_8BIT, NULL};
  T_AUDIO_RET audio_ret;
  UBYTE cmd;

  if (tty < TTY_OFF OR tty > TTY_ALL)
  {
    TRACE_EVENT_P1 ("TTY invalid parameter: %d", tty);
    return;
  }
  
#if defined (RIV_AUDIO) && !defined (_TTY_SIMU)

    cmd = (UBYTE)tty;
    if (tty_state)
    {
      load_ret.callback_func = audio_dyn_load_return;
    }
    else
    {
      /*
       * If the TTY mode was default before then need to start
       * the TTY driver
       */
      tty_cfg.Mode = TTY_EXT_START;
      tty_cfg.ThresholdRead = tty_cfg.ThreshHoldWrite = 0;
      load_ret.callback_func = audio_load_return;
    }

    strcpy (audio_mode.audio_mode_filename, audio_mode_names_tty[(int)cmd]);
    audio_ret = audio_mode_load (&audio_mode, load_ret);

    if (audio_ret NEQ AUDIO_OK)
    {
      AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE LOAD TTY: %d", (int)audio_ret);
    }
    else
    {
      AUDIO_TRC_FUNC ("AUDIO MODE LOAD def");
      tty_cmd = (UBYTE)tty;
    }
#else
    TRACE_EVENT_P1 ("TTY mode change: %d", tty);
#endif

}

#if defined (RIV_AUDIO) AND !defined (_TTY_SIMU)
LOCAL void audio_tty_return (void *tty_result)
{
  T_AUDIO_MODE_LOAD audio_mode;
  T_RV_RETURN_PATH ret = {NOT_PRESENT_8BIT, NULL};
  T_AUDIO_RET audio_ret;

  if (((T_AUDIO_TTY_STATUS *)tty_result)->status NEQ AUDIO_OK)
  {
    AUDIO_TRC_EVENT_P1 ("Error TTY callback: %d",
                        (int)((T_AUDIO_TTY_STATUS *)tty_result)->status);
  }
  tty_state = FALSE;
  tty_cfg.Mode = TTY_STOP;
  AUDIO_TRC_FUNC ("TTY end");

  ret.callback_func = audio_load_return;
  strcpy (audio_mode.audio_mode_filename, audio_mode_names_tty[0]);
  audio_ret = audio_mode_load (&audio_mode, ret);

  if (audio_ret NEQ AUDIO_OK)
  {
    AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE LOAD def: %d", (int)audio_ret);
  }
}

LOCAL void audio_save_def_return (void *result)
{
  T_AUDIO_MODE_LOAD audio_mode;
  T_RV_RETURN_PATH load_ret = {NOT_PRESENT_8BIT, NULL};
  T_AUDIO_RET audio_ret;

  if (((T_AUDIO_SAVE_DONE *)result)->status NEQ AUDIO_OK)
  {
    AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE SAVE def callback: %d",
                        (int)((T_AUDIO_SAVE_DONE *)result)->status);
  }
  else
  {
    AUDIO_TRC_FUNC ("AUDIO def saved");

    load_ret.callback_func = audio_load_return;
    strcpy (audio_mode.audio_mode_filename, audio_mode_names_tty[(int)tty_cmd]);
    audio_ret = audio_mode_load (&audio_mode, load_ret);

    if (audio_ret NEQ AUDIO_OK)
    {
      AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE LOAD TTY: %d", (int)audio_ret);
    }
  }
}

LOCAL void audio_load_return (void *result)
{
  if (((T_AUDIO_LOAD_DONE *)result)->status NEQ AUDIO_OK)
  {
    AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE LOAD callback: %d",
                        (int)((T_AUDIO_LOAD_DONE *)result)->status);
  }
  else
  {
    T_RV_RETURN_PATH ret = {NOT_PRESENT_8BIT, NULL};
    T_AUDIO_RET audio_ret;

    if (!tty_state AND tty_cfg.Mode EQ TTY_EXT_START)
    {
      ret.callback_func =  audio_tty_return;
      audio_ret = audio_tty_set_config (&tty_cfg, &ret);

      if (audio_ret NEQ AUDIO_OK)
      {
        AUDIO_TRC_EVENT_P1 ("Error TTY set: %d", (int)audio_ret);
      }
      else
      {
        tty_state = TRUE;
        AUDIO_TRC_FUNC ("AUDIO TTY loaded & start");
      }
    }
    else if (!tty_state AND tty_cfg.Mode EQ TTY_STOP)
    {
      tty_cmd = (UBYTE)TTY_OFF;
      AUDIO_TRC_FUNC ("AUDIO def loaded");
    }
    else
    {
      AUDIO_TRC_EVENT_P2 ("TTY improper state: %d; mode: %d",
                          (int)tty_state, (int)tty_cfg.Mode);
    }
  }
}

LOCAL void audio_dyn_load_return (void *result)
{
  if (((T_AUDIO_LOAD_DONE *)result)->status NEQ AUDIO_OK)
  {
    AUDIO_TRC_EVENT_P1 ("Error AUDIO MODE LOAD callback: %d",
                        (int)((T_AUDIO_LOAD_DONE *)result)->status);
  }
  else
  {
    AUDIO_TRC_EVENT_P1 ("TTY mode changed: %d", (int)tty_cmd);
  }
}
#endif
#endif

/*******************************************************************
 *                                                                 *
 * PART II: Simulation for Windows                                 *
 *                                                                 *
 *******************************************************************/

#if defined (_SIMULATION_)
/*
 * Dummies for driver calls
 */

LOCAL  void BZ_Init          (void)
{
}

LOCAL  void BZ_Enable        (void)
{
}

LOCAL  void BZ_Disable       (void)
{
}

LOCAL  void BZ_Tone          (int f)
{
}

LOCAL  void BZ_Volume        (int v)
{
}

#ifdef ALR
LOCAL  void ABB_DlVolume     (UBYTE v)
{
}

LOCAL  void ABB_DlMute       (UBYTE mute)
{
}

LOCAL  void ABB_UlMute       (UBYTE value)
{
}

LOCAL  void ABB_SideTone     (UBYTE v)
{
}
#else
LOCAL  void VG_UlVolume      (UBYTE v)
{
}

LOCAL  void VG_DlVolume      (UBYTE v)
{
}

LOCAL  void VG_DlMute        (UBYTE mute)
{
}

LOCAL  void VG_SideTone      (UBYTE v)
{
}
#endif

#endif

#if defined (RIV_AUDIO)
void audio_riv_audio_cb (void *buf)
{
    /* nothing to do for this time */
}
#endif

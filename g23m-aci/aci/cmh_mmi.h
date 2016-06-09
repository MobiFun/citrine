/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMI
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
|  Purpose :  Definitions for the command handler
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_MMI_H
#define CMH_MMI_H

/*==== CONSTANTS ==================================================*/
 
 
/*==== TYPES ======================================================*/

#ifdef ACI /* for for ATI only version */
typedef enum
{
  AUDIO_IN  =  75, /* audio in   */
  AUDIO_OUT = 175, /* audio out  */
  KEY_BEEP  = 100, /* key beep   */
  DTMF_TONE = 100, /* dtmf tone  */
  RINGER    = 100  /* ringer     */
}
T_VOLUME_TYPE;

typedef enum
{
  SWITCH_AUDIO_NONE,
  SWITCH_AUDIO_OFF,
  SWITCH_AUDIO_ALERT,
  SWITCH_AUDIO_BUSY,
  SWITCH_AUDIO_CALLWAIT
}
T_ACI_IBT_ACT;

typedef struct
{
  T_ACI_CPI_IBT ati_currIbt;
  T_ACI_CPI_TCH ati_currTch;
  T_ACI_IBT_ACT last_action;
} T_IBT_TYPE;

#endif

/*==== PROTOTYPES =================================================*/
GLOBAL void cmhMMI_keyIndication ( void );
GLOBAL void cmhMMI_rxIndication  ( void );
GLOBAL void cmhMMI_btIndication  ( void );

#ifdef ACI
GLOBAL void cmhMMI_setAudioVolume (  BYTE          audioIn, 
                                     BYTE          audioOut );

GLOBAL void cmhMMI_handleAudioTone ( T_ACI_AT_CMD  cmdId,
																		 UBYTE         response,
                                     T_ACI_CPI_MSG msgType );


/*==== EXPORT =====================================================*/

#ifdef CMH_MMIS_C
GLOBAL T_IBT_TYPE ibt_params;
#else
EXTERN T_IBT_TYPE ibt_params;
#endif

#endif /* ACI */

#endif /* CMH_MMI_H */

/*==== EOF =======================================================*/

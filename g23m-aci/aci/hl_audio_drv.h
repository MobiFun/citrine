/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  AOC
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
|  Purpose :  Definitions for high level driver for vocoder interface
+----------------------------------------------------------------------------- 
*/

#ifndef HL_VOICE_DRV_H
#define HL_VOICE_DRV_H

typedef enum 
{
  HL_VOICE_DRV_FAIL,
  HL_VOICE_DRV_SUCC

} T_HL_VOICE_DRV_RSLT;


typedef enum
{
#ifndef VOCODER_FUNC_INTERFACE
  HL_VOCODER_DISABLE_INITIATED,
  HL_VOCODER_ENABLE_INITIATED,
#endif
  HL_VOCODER_DISABLED,
  HL_VOCODER_ENABLED

} T_HL_VOCODER_STATE;

#ifdef VOCODER_FUNC_INTERFACE
typedef enum
{
  VOCODER_IDLE = 0,
  VOCODER_BUSY,
  VOCODER_PENDING_ENABLE,
  VOCODER_PENDING_DISABLE
}T_HL_VOCODER_ACTION;

EXTERN T_HL_VOCODER_ACTION currVocoderAction;
#endif /* VOCODER_FUNC_INTERFACE */

EXTERN void hl_audio_drv_init (void);
EXTERN T_HL_VOCODER_STATE hl_drv_get_vocoder_state (void);
EXTERN void hl_drv_vocoder_state_set (void);
EXTERN void hl_drv_set_vocoder_state(BOOL);

#endif /* HL_VOICE_DRV_H */

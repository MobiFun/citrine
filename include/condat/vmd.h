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
|  Purpose :  Types definitions for the voice memo and dialling driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef DEF_VMD_H
#define DEF_VMD_H

/*
 * Signal Types
 */
#define VMD_END_RECORD             1
#define VMD_END_PLAY               2
#define VMD_SPEECH_RECOGNITION     3


/*
 * type definitions
 */
typedef struct vmd_start_record_type
{
  USHORT * start_address;
  USHORT * stop_address;
  UBYTE    dtx_used;
  USHORT   tone_id;
} vmd_start_record_type;

typedef struct vmd_start_play_type
{
  USHORT * start_address;
} vmd_start_play_type;

typedef struct vmd_end_record_type
{
  USHORT * stop_address;
} vmd_end_record_type;

typedef struct vmd_end_play_type
{
  USHORT * stop_address;
} vmd_end_play_type;


typedef struct vmd_speech_recognition_req_type
{
  USHORT   i_speaker;
  USHORT   i_action;
  USHORT   i_param1;
  USHORT   i_param2;
  USHORT * address;
  USHORT   total_vocabulary_number;
} vmd_speech_recognition_req_type;

/*
 * Constants
 */
#define SC_SR_MAX_NB_USER          10

#define SC_SR_START_ENROLL          1
#define SC_SR_START_UPDATE          2
#define SC_SR_START_RECO            3
#define SC_SR_ABORT                 6
#define SC_SR_START_UPDATE_CHECK   13

#define C_SR_CORRECT_END            0
#define C_SR_CORRECT_END_ACQ        1
#define C_SR_CORRECT_END_UPD        2
#define C_SR_CORRECT_END_RECO       3
#define C_SR_TIME_OUT              11
#define C_SR_BAD_ACQUISITION       12
#define C_SR_OOV_TOOCLOSE          13
#define C_SR_UPDATE_CHECK_ERR      14
#define C_SR_BAD_RECOGNITION       20

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#endif

 /*
 * Prototypes
 */
EXTERN UBYTE vmd_init                       (drv_SignalCB_Type   in_SignalCBPtr);
EXTERN void  vmd_exit                       (void);
EXTERN UBYTE vmd_start_voice_memo_recording (vmd_start_record_type * vmd_parameter);
EXTERN UBYTE vmd_stop_voice_memo_recording  (void);
EXTERN UBYTE vmd_start_voice_memo_playing   (vmd_start_play_type   * vmd_parameter);
EXTERN UBYTE vmd_stop_voice_memo_playing  (void);

EXTERN UBYTE vmd_start_recognition          (vmd_speech_recognition_req_type * vmd_parameter);
EXTERN UBYTE vmd_primitive                  (USHORT opc, void * data);

#endif

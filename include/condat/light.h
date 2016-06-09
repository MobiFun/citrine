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
|  Purpose :  Types definitions for the light emitting driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef LIGHT_H
#define LIGHT_H

/*
 * devices
 */
#define LIGHT_DEVICE_BACKLIGHT     0
#define LIGHT_DEVICE_INSERVICELED  1

/*
 * device status
 */
#define LIGHT_STATUS_OFF           0
#define LIGHT_STATUS_ON            1
#define LIGHT_STATUS_DIMMED        2
#define LIGHT_STATUS_INTERMEDIATE  3
#define LIGHT_STATUS_BRIGHT        4

#define LIGHT_STATUS_GREEN         5
#define LIGHT_STATUS_ORANGE        6
#define LIGHT_STATUS_RED           7

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
EXTERN UBYTE light_Init      (void);
EXTERN void  light_Exit      (void);
EXTERN UBYTE light_SetStatus (UBYTE in_DeviceID, UBYTE in_NewStatus);
EXTERN UBYTE light_GetStatus (UBYTE in_DeviceID, UBYTE * in_StatusPtr);

void light_setBacklightOn( void );
void light_setBacklightIdle( void );
void light_setBacklightOff( void );

#endif

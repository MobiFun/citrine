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
|  Purpose :  Types definitions for the keybord driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef KBD_H
#define KBD_H

/*
 * typematic rates (character per second)
 */
#define KBD_TYPERATE_NONE     0
#define KBD_TYPERATE_6        1
#define KBD_TYPERATE_8        2
#define KBD_TYPERATE_10       3
#define KBD_TYPERATE_12       4
#define KBD_TYPERATE_15       5
#define KBD_TYPERATE_20       6
#define KBD_TYPERATE_24       7
#define KBD_TYPERATE_30       8

/*
 * type delay (in milliseconds)
 */
#define KBD_TYPEDELAY_250     1
#define KBD_TYPEDELAY_500     2
#define KBD_TYPEDELAY_750     3
#define KBD_TYPEDELAY_1000    4

/*
 * Keyboard operations
 */
#define KBD_KEYDOWN           1
#define KBD_KEYUP             2
#define KBD_KEYREPEAT         3

/*
 * Keyboard signals
 */
#define KBD_SIGTYPE_STATUSCHG 1

/*
 * Driver Control Block
 */
typedef struct kbd_DCB_Type
{
  USHORT TypematicRate;
  USHORT TypematicDelay;
} kbd_DCB_Type;

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#define T_VSI_THANDLE               USHORT
#endif

/* Marcus: Issue 811: 05:09/2002: Start */
/*
 * Number of elements in the keypad arrays
 */
#define NUM_KPD_KEYS 32
/*
 * External data
 */
extern const UBYTE kbd_map [NUM_KPD_KEYS];
extern const UBYTE simkbd_map [NUM_KPD_KEYS];
extern char * const config_map [NUM_KPD_KEYS];
/* Marcus: Issue 811: 05:09/2002: End */

/*
 * Prototypes
 */
EXTERN UBYTE kbd_Init        (drv_SignalCB_Type   in_SignalCBPtr);
EXTERN void  kbd_Exit        (void);
EXTERN UBYTE kbd_SetConfig   (kbd_DCB_Type      * in_DCBPtr);
EXTERN UBYTE kbd_GetConfig   (kbd_DCB_Type      * out_DCBPtr);
EXTERN UBYTE kbd_SetSignal   (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE kbd_ResetSignal (drv_SignalID_Type * in_SignalIDPtr);
EXTERN ULONG kbd_GetStatus   (void);

#endif

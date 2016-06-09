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
|  Purpose :  Types definitions for the power driver
|             .
+----------------------------------------------------------------------------- 
*/ 

#ifndef PWR_H
#define PWR_H


/*
 * Power signals
 */
#define PWR_SIGTYPE_EXTPOWER 1
#define PWR_SIGTYPE_CHARGER  2
#define PWR_SIGTYPE_BATLEVEL 3

/*
 * Power Status
 */
#define PWR_EXTPOWER_ON      1
#define PWR_CHARGER_ON       2

/*
 * Status Type
 */
typedef struct pwr_Status_Type
{
  UBYTE Status;
  UBYTE BatteryLevel;
  UBYTE ChargeLevel;
} pwr_Status_Type;

/*
 * Driver Control Block
 */
typedef struct pwr_DCB_Type
{
  UBYTE    RangeMin;
  UBYTE    RangeMax;
  UBYTE    Steps;
} pwr_DCB_Type;

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
EXTERN UBYTE pwr_Init        (drv_SignalCB_Type   in_SignalCBPtr);
EXTERN void  pwr_Exit        (void);
EXTERN UBYTE pwr_SetSignal   (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE pwr_ResetSignal (drv_SignalID_Type * in_SignalIDPtr);
EXTERN UBYTE pwr_SetConfig   (pwr_DCB_Type      * in_DCBPtr);
EXTERN UBYTE pwr_GetConfig   (pwr_DCB_Type      * out_DCBPtr);
EXTERN UBYTE pwr_GetStatus   (pwr_Status_Type   * out_StatusPtr);

EXTERN UBYTE pwr_PowerOffMobile   (void);

#endif

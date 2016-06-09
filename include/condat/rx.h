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
|  Purpose :  Types definitions for the fieldstrength driver
|             .
+----------------------------------------------------------------------------- 

 $Hist       

    Oct 05, 2006    ER: OMAPS00094496 x0061088(Prachi)
    Description:Enhance RSSI to 3 parameters: Strength, quality, min-access-level
    Solution:In order to provide a more accurate antenna icon on screen, in addition to "signal strength" the parameters
    "signal quality" and "Min-aces-level" are required. 

*/ 

#ifndef RX_H
#define RX_H

/*
 * Signal Values
 */
#define RX_QUAL_UNAVAILABLE   0xFF /* should be kept identically   */ 
                                   /* with the value defined in RR */
#ifdef FF_PS_RSSI
#define RX_ACCE_UNAVAILABLE   0xFF
#endif

/*
 * Signal Types
 */
#define RX_SIGTYPE_RXLEVEL    0

/*
 * Status Type
 */
typedef struct rx_Status_Type
{
  UBYTE actLevel;
  UBYTE gsmLevel;
  UBYTE rxQuality;
#ifdef FF_PS_RSSI
  UBYTE min_access_level;
#endif
} rx_Status_Type;

/*
 * Driver Control Block
 */
typedef struct rx_DCB_Type
{
  UBYTE    Steps;
} rx_DCB_Type;

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
EXTERN UBYTE rx_Init        (drv_SignalCB_Type   in_SignalCBPtr);
EXTERN void  rx_Exit        (void);
EXTERN UBYTE rx_SetConfig   (rx_DCB_Type       * in_DCBPtr);
EXTERN UBYTE rx_GetConfig   (rx_DCB_Type       * out_DCBPtr);
EXTERN UBYTE rx_GetStatus   (rx_Status_Type    * out_StatusPtr);
EXTERN void  rx_timeout     (T_VSI_THANDLE       rx_handle);
#endif

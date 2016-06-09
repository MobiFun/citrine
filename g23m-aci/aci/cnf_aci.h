/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  CNF_ACI
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
|  Purpose :  Dynamic Configuration for AT Command Interpreter
+----------------------------------------------------------------------------- 
*/ 

#ifndef CNF_ACI_H
#define CNF_ACI_H

/*==== CONSTANTS ==================================================*/
/*
 * CONFIGURATION PARAMETER
 *
 * Description :  The constants define the commands for dynamic
 *                configuration proposals.
 */

#define  ACI_TIMER_SET       "TIMER_SET"
#define  ACI_TIMER_RESET     "TIMER_RESET"
#define  ACI_TIMER_SPEED_UP  "TIMER_SPEED_UP"
#define  ACI_TIMER_SLOW_DOWN "TIMER_SLOW_DOWN"
#define  ACI_TIMER_SUPPRESS  "TIMER_SUPPRESS"
#define  ACI_KEY_SEQUENCE    "KEY_SEQUENCE"
#define  ACI_EXT_DISPLAY     "EXT_DISPLAY"
#define  ACI_START_AOC       "AOC_PARAMETER"
#define  ACI_KEY_PRESS       "KEY_PRESS"
#define  ACI_KEY_RELEASE     "KEY_RELEASE"
#define  ACI_CPOL_MODE       "CPOL_MODE"
#define  ACI_CPOL_IDX2       "CPOL_IDX2"
#define  ACI_ATI_VIA_TIF     "ATI_VIA_TIF"
#define  ACI_DATA_INPUT      "DATA_INPUT"

#define  TDTE_INACTIVNAME    "TDTE_INACTIV"

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#endif

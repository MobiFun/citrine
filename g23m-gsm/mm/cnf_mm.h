/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (8410)
|  Modul   :  CNF_MM
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
|  Purpose :  Dynamic Configuration for Mobility Management
+----------------------------------------------------------------------------- 
*/ 

#ifndef CNF_MM_H
#define CNF_MM_H

/*==== CONSTANTS ==================================================*/
/*
 * CONFIGURATION PARAMETER
 *
 * Description :  The constants define the commands for dynamic
 *                configuration proposals.
 */

#define  MM_TIMER_SET       "TIMER_SET"
#define  MM_TIMER_RESET     "TIMER_RESET"
#define  MM_TIMER_SPEED_UP  "TIMER_SPEED_UP"
#define  MM_TIMER_SLOW_DOWN "TIMER_SLOW_DOWN"
#define  MM_TIMER_SUPPRESS  "TIMER_SUPPRESS"
#define  MM_T3212_CNT       "T3212_CNT"
#define  MM_USE_STORED_BCCH "USE_STORED_BCCH"

#define  MM_FFS_RESET_EPLMN  "FFS_RESET_EPLMN"
#define  MM_FFS_READ_EPLMN   "FFS_READ_EPLMN"
#define  MM_FFS_READ_EPLMN_INIT "FFS_READ_EPLMN_INIT"
#define  MM_FFS_WRITE_EPLMN  "FFS_WRITE_EPLMN"


/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

#endif

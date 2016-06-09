/* 
+----------------------------------------------------------------------------- 
|  Project :  
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
|  Purpose :  Constants to determine the dimensions of the frame
+----------------------------------------------------------------------------- 
*/ 

#ifndef GPRSCONST_H
#define GPRSCONST_H

/*==== CONSTANTS ============================================================*/

#define MAX_TIMER                  230
#define MAX_SIMULTANEOUS_TIMER      50

#ifdef FF_BAT /* with GDD_DIO entity */
#define MAX_ENTITIES                38
#define MAX_OS_TASKS                32
#define MAX_SEMAPHORES               9 /* needed for the BAT connection */
#else
#define MAX_ENTITIES                37
#define MAX_OS_TASKS                31
#define MAX_SEMAPHORES               8
#endif /* FF_BAT */

#define MAX_OSISRS                   0

#define MAX_COMMUNICATIONS           MAX_OS_TASKS
#define MAX_POOL_GROUPS              6
#define MAX_MEMORY_POOLS             6

#endif /* GPRSCONST_H */


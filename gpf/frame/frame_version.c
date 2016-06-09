/* 
+------------------------------------------------------------------------------
|  File:       frame_version.c
+------------------------------------------------------------------------------
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
|  Purpose :  This Module contain build date and time
+----------------------------------------------------------------------------- 
*/ 

#ifndef __FRAME_VERSION_C__
#define __FRAME_VERSION_C__
#endif


/*==== INCLUDES ===================================================*/


/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/

#ifndef RUN_INT_RAM
const char * const frame_version_date = __DATE__;
const char * const frame_version_time = __TIME__;
#endif

/*==== EXTERNALS ==================================================*/


/*==== FUNCTIONS ==================================================*/



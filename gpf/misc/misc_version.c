/* 
+------------------------------------------------------------------------------
|  File:       misc_version.c
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
|  Purpose :  This Module contain build date and time for MISC
+----------------------------------------------------------------------------- 
*/ 

#ifndef __MISC_VERSION_C__
#define __MISC_VERSION_C__
#endif


/*==== INCLUDES ===================================================*/


/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/

#ifndef RUN_INT_RAM
char const * const misc_version_date = __DATE__;
char const * const misc_version_time = __TIME__;
#endif

/*==== EXTERNALS ==================================================*/


/*==== FUNCTIONS ==================================================*/



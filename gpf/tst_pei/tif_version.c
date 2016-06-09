/* 
+------------------------------------------------------------------------------
|  File:       tif_version.c
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
|  Purpose :  This Module contain build date and time of TIF
|             about the processes
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TIF_VERSION_C__
#define __TIF_VERSION_C__
#endif


/*==== INCLUDES ===================================================*/


/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/

#ifndef RUN_INT_RAM
char const * const tif_version_date = __DATE__;
char const * const tif_version_time = __TIME__;
#endif

/*==== EXTERNALS ==================================================*/


/*==== FUNCTIONS ==================================================*/



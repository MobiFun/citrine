/* 
+------------------------------------------------------------------------------
|  File:       vsi_sem.c
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
|  Purpose :  This Module defines the virtual system interface part
|             for the semaphore handling.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_SEM_C__
#define __VSI_SEM_C__
#endif

/*==== INCLUDES ===================================================*/

#include "gpfconf.h"
#include "typedefs.h"
#include "vsi.h"
#include "os.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/
/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED")
#endif

/*==== VARIABLES ==================================================*/


#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== FUNCTIONS ==================================================*/


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_SEM             |
| STATE   : code                       ROUTINE : vsi_s_open          |
+--------------------------------------------------------------------+

  PURPOSE : opens a semaphore, creates if not exist

*/

T_HANDLE vsi_s_open (T_HANDLE Caller, char *Name, USHORT Count)
{
OS_HANDLE SemHandle;

  /*
   * if semaphore already exists, return handle
   */
  if ( os_OpenSemaphore ( Caller, Name, &SemHandle ) != OS_ERROR )
    return SemHandle;

  /*
   * if semaphore not exists, create
   */
  if ( os_CreateSemaphore ( Caller, Name, Count, &SemHandle, pf_TaskTable[Caller].MemPoolHandle ) != OS_ERROR )
    return SemHandle;
  else
    vsi_o_assert( Caller, OS_SYST_ERR_MAX_SEMA, __FILE__, __LINE__,
                  "Number of created semaphores > MAX_SEMAPHORES" );

  /*
   * if semaphore cannot be created, return VSI_ERROR
   */
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_SEM             |
| STATE   : code                       ROUTINE : vsi_s_close          |
+--------------------------------------------------------------------+

  PURPOSE : closes a semaphore

*/

int vsi_s_close (T_HANDLE Caller, T_HANDLE SemHandle)
{

  if ( os_CloseSemaphore ( Caller, SemHandle ) != OS_ERROR )
    return VSI_OK;

  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_SEM             |
| STATE   : code                       ROUTINE : vsi_s_get           |
+--------------------------------------------------------------------+

  PURPOSE : obtains a semaphore

*/

int vsi_s_get (T_HANDLE Caller, T_HANDLE SemHandle)
{
LONG Status;

  Status = os_ObtainSemaphore ( Caller, SemHandle, OS_SUSPEND );
  if ( Status == OS_OK || Status == OS_WAITED )
    return VSI_OK;

  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_SEM             |
| STATE   : code                       ROUTINE : vsi_s_release       |
+--------------------------------------------------------------------+

  PURPOSE : releases a semaphore

*/

int vsi_s_release (T_HANDLE Caller, T_HANDLE SemHandle)
{

  if ( os_ReleaseSemaphore ( Caller, SemHandle ) != OS_ERROR )
    return VSI_OK;

  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_SEM             |
| STATE   : code                       ROUTINE : vsi_s_status        |
+--------------------------------------------------------------------+

  PURPOSE : request the current count of a semaphore

*/

int vsi_s_status (T_HANDLE Caller, T_HANDLE SemHandle, USHORT *Count )
{

  if ( os_QuerySemaphore ( Caller, SemHandle, Count ) != OS_ERROR )
    return VSI_OK;

  return VSI_ERROR;
}
#endif


/* 
+------------------------------------------------------------------------------
|  File:       vsi_mis.c
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
|             for miscellaneous things.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_MIS_C__
#define __VSI_MIS_C__
#endif

/*==== INCLUDES ===================================================*/

#include <string.h>

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
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_gettaskname     |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

int vsi_gettaskname (T_HANDLE Caller, T_HANDLE Handle, char *Name)
{

  if ( os_GetTaskName ( Caller, Handle, Name ) != OS_ERROR )
    return VSI_OK;
  
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_gettaskhandle   |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

T_HANDLE vsi_gettaskhandle (T_HANDLE Caller, char *Name)
{
OS_HANDLE Handle;

  if (os_GetTaskHandle( Caller, Name, &Handle ) != OS_ERROR )
  {
    return Handle;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_gettaskflags    |
+--------------------------------------------------------------------+

  PURPOSE : reads the flags of a task

*/

int vsi_gettaskflags (T_HANDLE Caller, T_HANDLE Handle, U32 *Flags)
{

  *Flags = pf_TaskTable[Handle].Flags;  
  return VSI_OK;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_time            |
+--------------------------------------------------------------------+

  PURPOSE : get time

*/

int vsi_t_time (T_HANDLE Caller, T_TIME *Value)
{

  os_GetTime ( Caller, Value );
  return VSI_OK;
  
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_sleep           |
+--------------------------------------------------------------------+

  PURPOSE : suspend task

*/

int vsi_t_sleep (T_HANDLE Caller, T_TIME Value)
{

  os_SuspendTask ( Caller, Value );
  return VSI_OK;
  
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MIS             |
| STATE   : code                       ROUTINE : vsi_object_info     |
+--------------------------------------------------------------------+

  PURPOSE : read information about created objects

*/
int vsi_object_info (T_HANDLE Caller, USHORT Id, USHORT Index, char *Buffer, USHORT Size)
{
  if ( os_ObjectInformation ( Caller, Id, Index, Size, Buffer ) == OS_OK )
  {
    if ( strlen (Buffer) > TTRACE_LEN )
      vsi_o_assert ( NO_TASK, OS_SYST_ERR_STR_TOO_LONG, __FILE__, __LINE__, 
                     "Traced string too long" );
    return VSI_OK;
  }
  return VSI_ERROR;

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_name          |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

int vsi_e_name (T_HANDLE Caller, T_HANDLE Handle, char *Name)
{
  if ( Handle >= 0 && Handle <= MaxEntities && pf_TaskTable[Handle].Name[0] != 0 )
  {
    strcpy ( Name, pf_TaskTable[Handle].Name );
    return VSI_OK;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_handle        |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

T_HANDLE vsi_e_handle (T_HANDLE Caller, char *Name)
{
T_HANDLE e_handle;

  if ( Name == NULL )
  {
    return e_running[os_MyHandle()];
  }
  else
  {
    for ( e_handle = MaxEntities; e_handle > 0; e_handle-- )
    {
      if ( pf_TaskTable[e_handle].Name[0] != 0 && !strncmp ( pf_TaskTable[e_handle].Name, Name, RESOURCE_NAMELEN-1 ) )
        return e_handle;
    }
  }
  return VSI_ERROR;
}
#endif


/* 
+------------------------------------------------------------------------------
|  File:       vsi_pro.c
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
|             about the processes
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_PRO_C__
#define __VSI_PRO_C__
#endif


/*==== INCLUDES ===================================================*/

#include <string.h>

#include "gpfconf.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "frame.h"
#include "route.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

extern void pf_TaskEntry(T_HANDLE, ULONG);

/*==== VARIABLES ==================================================*/

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== FUNCTIONS ==================================================*/


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_create        |
+--------------------------------------------------------------------+

  PURPOSE : creates a process

*/

T_HANDLE vsi_p_create (T_HANDLE Caller, SHORT (*pei_create)(T_PEI_INFO const ** info), 
                    void (*TaskEntry)(T_HANDLE, ULONG), T_HANDLE MemPoolHandle )
{
void (*EntryFunc)(T_HANDLE, ULONG);
T_PEI_INFO const *Info;
T_HANDLE TaskHandle;

  if ( pei_create ( &Info ) == PEI_OK )
  {
    if ( TaskEntry == NULL )
      EntryFunc = pf_TaskEntry;
    else
      EntryFunc = TaskEntry;

    if ( os_CreateTask (NO_TASK, (char*)Info->Name, EntryFunc, Info->StackSize, Info->Priority, 
                        &TaskHandle, MemPoolHandle) == OS_OK )
    {
      pf_TaskTable[TaskHandle].Flags = Info->Flags;
      pf_TaskTable[TaskHandle].PeiTable = &Info->PeiTable;
      pf_TaskTable[TaskHandle].QueueEntries = Info->QueueEntries;
      pf_TaskTable[TaskHandle].NumOfTimers = Info->NumOfTimers;
      strncpy (pf_TaskTable[TaskHandle].Name, Info->Name, RESOURCE_NAMELEN);
      pf_TaskTable[TaskHandle].Name[RESOURCE_NAMELEN-1] = 0;
      pf_TaskTable[TaskHandle].TaskHandle = TaskHandle;

      pf_TaskTable[TaskHandle].MemPoolHandle = MemPoolHandle;
      return TaskHandle;
    }
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_exit          |
+--------------------------------------------------------------------+

  PURPOSE : exits and deletes a process

*/
#undef VSI_CALLER
#define VSI_CALLER Caller,
#define VSI_CALLER_SINGLE Caller
int vsi_p_exit (T_HANDLE Caller, T_HANDLE TaskHandle)
{
ULONG old_mask;
T_PRIM_HEADER *prim;
T_S_HEADER *s_hdr;
ULONG size;
char name[RESOURCE_NAMELEN];

  size = S_ALLOC_SIZE(strlen(SYSPRIM_EXIT_TOKEN));
#ifdef MEMORY_SUPERVISION
  prim = (T_PRIM_HEADER*)vsi_c_new ( Caller, size, 0, __FILE__, __LINE__ );
#else
  prim = (T_PRIM_HEADER*)vsi_c_new ( Caller, size, 0 );
#endif

  prim->opc = SYS_MASK;
  prim->sh_offset = S_HDR_OFFSET(size - sizeof(T_S_HEADER));
  prim->len = strlen(SYSPRIM_EXIT_TOKEN) + sizeof(T_PRIM_HEADER);
  s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
  s_hdr->snd[0] =(char)Caller;
  strcpy((char*)P2D(prim),SYSPRIM_EXIT_TOKEN);

  if (vsi_gettaskname(Caller,TaskHandle,name) < VSI_OK) 
  {
    return VSI_ERROR;
  }
  /* switch off tracing for Caller */
  vsi_gettracemask ( Caller, Caller, &old_mask);
  vsi_settracemask ( Caller, Caller, 0);
  PSEND(vsi_c_open(Caller,name),P2D(prim));
  /* set tracing to old value for Caller */
  vsi_settracemask ( Caller, Caller, old_mask);
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_delete        |
+--------------------------------------------------------------------+

  PURPOSE : deletes a process

*/

int vsi_p_delete (T_HANDLE Caller, T_HANDLE TaskHandle)
{
  rt_RoutingModify ( TaskHandle, (char*)SYSPRIM_REDIRECT_TOKEN, (char*)SYSPRIM_CLEAR_TOKEN );

  if ( os_DestroyQueue ( Caller, pf_TaskTable[TaskHandle].QueueHandle ) != OS_OK )
    return VSI_ERROR;

  memset ( &pf_TaskTable[TaskHandle], 0, sizeof (T_FRM_TASK_TABLE_ENTRY) );

  if ( os_DestroyTask ( Caller, TaskHandle ) != OS_OK )
    return VSI_ERROR;

  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_start         |
+--------------------------------------------------------------------+

  PURPOSE : starts a process

*/

int vsi_p_start (T_HANDLE Caller, T_HANDLE TaskHandle)
{

  if ( os_StartTask ( Caller, TaskHandle, 0 ) == OS_OK )
    return VSI_OK;
  else
    return VSI_ERROR;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_stop          |
+--------------------------------------------------------------------+

  PURPOSE : stops a process

*/

int vsi_p_stop (T_HANDLE Caller, T_HANDLE TaskHandle)
{

  if ( os_StopTask ( Caller, TaskHandle ) == OS_OK )
    return VSI_OK;
  else
    return VSI_ERROR;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_name          |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

int vsi_p_name (T_HANDLE Caller, T_HANDLE Handle, char *Name)
{

  if ( os_GetTaskName ( Caller, Handle, Name ) != OS_ERROR )
    return VSI_OK;

  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PRO             |
| STATE   : code                       ROUTINE : vsi_p_handle        |
+--------------------------------------------------------------------+

  PURPOSE : reads the name of a task

*/

T_HANDLE vsi_p_handle (T_HANDLE Caller, char *Name)
{
OS_HANDLE Handle;

  if ( os_GetTaskHandle ( Caller, Name, &Handle ) != OS_ERROR )
    return Handle;

  return VSI_ERROR;
}
#endif

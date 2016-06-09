/* 
+------------------------------------------------------------------------------
|  File:       os_glob.h
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
|  Purpose :  Global tabless for the Nucleus OS adaptation layer
+----------------------------------------------------------------------------- 
*/ 

#ifndef OS_GLOB_H
#define OS_GLOB_H

/*==== INCLUDES =============================================================*/

#include "os_types.h"

/*==== CONSTANTS ============================================================*/

/*==== TYPES ================================================================*/

/*==== VARIABLES ============================================================*/
 
#ifndef CONFIG_MODULE

extern USHORT MaxTasks;
extern USHORT MaxCommunications;
extern USHORT MaxSimultaneousTimer;
extern USHORT MaxTimer;
extern USHORT MaxSemaphores;
extern USHORT MaxOSISRs;
extern USHORT MaxEventGroups;
extern USHORT MaxPoolGroups;
extern USHORT MaxMemoryPools;
#else

#ifndef DATA_INT_RAM
USHORT MaxOSISRs = MAX_OSISRS;
USHORT MaxTasks = MAX_OS_TASKS;
USHORT MaxCommunications = MAX_COMMUNICATIONS;
USHORT MaxSimultaneousTimer = MAX_SIMULTANEOUS_TIMER;
USHORT MaxSemaphores = MAX_SEMAPHORES;
USHORT MaxEventGroups = MAX_EVENT_GROUPS;
USHORT MaxPoolGroups = MAX_POOL_GROUPS;
USHORT MaxMemoryPools = MAX_MEMORY_POOLS;

GLOBAL T_OS_TASK_TABLE_ENTRY TaskTable    [ MAX_OS_TASKS + 1 ];
GLOBAL T_OS_COM_TABLE_ENTRY ComTable      [ MAX_COMMUNICATIONS + 1 ];
GLOBAL T_OS_TIMER_ENTRY TimerTable        [ MAX_SIMULTANEOUS_TIMER + 1 ];
GLOBAL T_OS_SEM_TABLE_ENTRY SemTable      [ MAX_SEMAPHORES + 1 ];
GLOBAL T_OS_OSISR_TABLE_ENTRY OSISRTable  [ MAX_OSISRS + 1 ];
GLOBAL T_OS_PART_GRP_TABLE_ENTRY PartGrpTable  [ MAX_POOL_GROUPS + 1 ];
GLOBAL T_OS_MEM_POOL_TABLE_ENTRY MemPoolTable  [ MAX_MEMORY_POOLS + 1 ];
GLOBAL T_OS_EVTGRP_TABLE_ENTRY EvtGrpTable[ MAX_EVENT_GROUPS + 1 ];
GLOBAL T_OS_TIMER_TABLE_ENTRY *p_list     [ MAX_SIMULTANEOUS_TIMER + 1 ];
GLOBAL T_OS_POOL_BORDER PoolBorder        [ MAX_POOL_GROUPS+1];

GLOBAL const char *os_dar_filename = "/var/dbg/dar";
GLOBAL ULONG os_dar_write_buffer_size = 3000;
#endif /* DATA_INT_RAM */


#endif

#endif

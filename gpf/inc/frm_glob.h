/*
+------------------------------------------------------------------------------
|  File:       frm_glob.h
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
|  Purpose :  Global frame tables
+-----------------------------------------------------------------------------
*/

#ifndef FRM_GLOB_H
#define FRM_GLOB_H

/*==== INCLUDES =============================================================*/

#include "gpfconf.h"
#include "frm_types.h"

/*==== CONSTANTS ============================================================*/


/*==== TYPES ================================================================*/

/*==== VARIABLES ================================================================*/

#ifndef CONFIG_MODULE
extern USHORT MaxEntities;
extern USHORT MaxCommunications;
extern USHORT MaxSimultaneousTimer;
extern USHORT MaxTimer;
extern USHORT MaxSemaphores;
extern USHORT NumberOfPPMPartitions;
extern ULONG  MaxPrimPartSize;
extern USHORT TextTracePartitionSize;
extern T_VOID_STRUCT *processed_prim[];
extern T_VOID_STRUCT *freed_prim[];
extern T_HANDLE e_running[];
extern T_FRM_TASK_TABLE_ENTRY pf_TaskTable[];
extern ULONG TraceMask[];		
extern char TracesAborted[];
extern char PrimAborted[];
extern char route_desclist[];
extern T_HANDLE TimerHandleField[];

#if !defined _TARGET_ && !defined _TOOLS_
extern char pcheck_active[];
#endif

 #else /* CONFIG_MODULE */

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED")
#endif

#ifndef DATA_INT_RAM

/* declare on pointer for each entity for ccd error handling */
struct ccd_task_table;
struct ccd_task_table* ccd_task_list[MAX_ENTITIES+1];

USHORT MaxEntities = MAX_ENTITIES;
USHORT MaxTimer    = MAX_TIMER;
#ifndef _TOOLS_
/* 
 * This way of setting the TST and RCV stacksize is chosen to keep it backwardscompatible,
 * i.e. not change the behavior if the stacksizes are not define in the configuration
 * file xxxconst.h.
 */
#ifdef TSTSND_STACK_SIZE
const USHORT TST_SndStacksize = TSTSND_STACK_SIZE;
#else
const USHORT TST_SndStacksize = 0;
#endif
#ifdef TSTRCV_STACK_SIZE
const USHORT TST_RcvStacksize = TSTRCV_STACK_SIZE;
#else
const USHORT TST_RcvStacksize = 0;
#endif

#endif /* _TOOLS_ */

T_FRM_TASK_TABLE_ENTRY pf_TaskTable  [ MAX_ENTITIES + 1 ]={0};
ULONG *Routing                       [ MAX_ENTITIES + 1 ]={0};
ULONG TraceMask                      [ MAX_ENTITIES + 1 ]={0};
char TracesAborted                   [ MAX_ENTITIES + 1 ]={0};
char PrimAborted                     [ MAX_ENTITIES + 1 ]={0};
#ifdef PRIM_AUTO_FREE
T_VOID_STRUCT *processed_prim        [ MAX_ENTITIES + 1 ]={0};
T_VOID_STRUCT *freed_prim            [ MAX_ENTITIES + 1 ]={0};
#endif 
T_HANDLE e_running                   [ MAX_ENTITIES + 1 ]={0};
char route_desclist                  [ MAX_ENTITIES + 1 ]={0};
T_DRV_TABLE_ENTRY DrvTable           [ MAX_TST_DRV + 1  ]={0};
T_HANDLE TimerHandleField            [ MAX_TIMER + 1    ]={0};

#if !defined _TARGET_ && !defined _TOOLS_
char pcheck_active                   [ MAX_ENTITIES + 1 ]={0};
#endif

#endif /* DATA_INT_RAM */

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

#endif /* CONFIG_MODULE */

#endif

/* 
+------------------------------------------------------------------------------
|  File:       frm_types.h
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
|  Purpose :  Definitions for the Frame
+----------------------------------------------------------------------------- 
*/ 

#ifndef FRM_TYPES_H
#define FRM_TYPES_H

/*==== INCLUDES =============================================================*/

#include "gpfconf.h"
#include "pei.h"
#include "gdi.h"

/*==== CONSTANTS ============================================================*/

/*==== TYPES ================================================================*/
 
/*
 * Type Definitions for Frame Management 
 */

typedef struct                                         
{
  char  	   Name[RESOURCE_NAMELEN];    /* name of task */
  T_PEI_FUNC const *PeiTable;		/* addresses of pei_functions */
  USHORT	   NumOfTimers;		    /* timers are created in pf_TaskEntry() */
  USHORT	   QueueEntries;		  /* queue is created in pf_TaskEntry() */
  T_HANDLE * FirstTimerEntry;   /* first entry in timer handle array */
  T_HANDLE   MemPoolHandle;		  /* handle for memory pool access */
  T_HANDLE   QueueHandle;		    /* own queue handle */
  T_HANDLE   TaskHandle;		    /* own task handle */
  U32        Flags;	            /* active/passive */
} T_FRM_TASK_TABLE_ENTRY;

typedef struct
{
   ULONG 	   TimerValue;			  /* value of timer */
   ULONG 	   TimerMode;			    /* mode of timer */
} T_FRM_TIMCFG_TABLE_ENTRY;

typedef struct                                         
{
  char          *Name;               /* name of the driver */
  T_HANDLE      ProcessHandle;       /* handle of the process to be notified */
  USHORT        UpperDrv;            /* handle of driver in the upper layer */
  USHORT        LowerDrv;            /* handle of driver in the lower layer */
  USHORT        SignalTo;            /* handle of process to be notified at Callback */
  T_DRV_EXPORT const *DrvInfo;
} T_DRV_TABLE_ENTRY;

typedef struct
{
  unsigned int part_num;
  unsigned int part_size;
  void * mem;
} T_FRM_PARTITION_POOL_CONFIG;

typedef struct
{
  char * name;
  const T_FRM_PARTITION_POOL_CONFIG * grp_config;
} T_FRM_PARTITION_GROUP_CONFIG;

typedef struct                                         
{
  char *Name;
  ULONG  Size;		
  char  *PoolAddress;
} T_MEMORY_POOL_CONFIG;

typedef SHORT T_PEI_CREATE ( T_PEI_INFO const **Info );

typedef struct
{
  T_PEI_CREATE *PeiCreate;
  char         *Name;
  int          Priority;
} T_COMPONENT_ADDRESS;


#ifdef MEMORY_SUPERVISION

typedef struct                                         
{
  ULONG 	Total;		             /* total number of elements */
  ULONG 	Current;		           /* current number of elements */
  ULONG 	Maximum;	             /* maximum number of elements */
  ULONG 	MaxByteMemory;         /* number of allocated bytes at maximum of elements */
  ULONG 	MaxPartMemory;         /* number of allocated partitions at maximum of elements */
} T_COUNTER;

typedef struct                                         
{
  ULONG 	UsedSize;		           /* size of stored primitive */
  ULONG   RequestedSize;         /* requested size during allocation */
  ULONG   time;                  /* time when partition is touched */
  USHORT  Status;   	           /* status of partition */
  T_HANDLE owner;
 	ULONG   PrimOPC;	             /* opc of primitive that uses this partition */
  void    *ptr;
  const char    *Userfile;	     /* file that accesses partition */
  int    Line;		               /* line where partition is accessed */
} T_PARTITION_STATUS; 

typedef struct                                         
{
  ULONG   PrimOPC;	             /* opc of primitive that does not fit in partition */
  const char    *Userfile;	     /* file that access partition */
  int  Line;		                 /* line where partition is accessed */
} T_OVERSIZE_STATUS;

typedef struct                                         
{
  T_PARTITION_STATUS	*PartitionStatus;
  T_OVERSIZE_STATUS  	*PartitionOversize;
  T_COUNTER	          *PartitionCounter;
#ifdef OPTIMIZE_POOL
  T_COUNTER          	*ByteCounter;
  T_COUNTER          	*RangeCounter;
#endif /* OPTIMIZE_POOL */
} T_PARTITION_POOL_STATUS;

#endif

#endif

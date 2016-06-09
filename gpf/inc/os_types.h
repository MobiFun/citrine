/* 
+------------------------------------------------------------------------------
|  File:       os_types.h
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
|  Purpose :  Definitions for the Nucleus OS adaptation layer
+----------------------------------------------------------------------------- 
*/ 

#ifndef OS_TYPES_H
#define OS_TYPES_H

/*==== INCLUDES =============================================================*/

#include "gpfconf.h"
#include "../../nucleus/nucleus.h"
#include "os.h"
#include "glob_defs.h"

/*==== CONSTANTS ============================================================*/

/*==== TYPES ================================================================*/
 
typedef enum 
{ 
  SYSTEM_TICK_TDMA_FRAME = 1,
  SYSTEM_TICK_10_MS
} T_OS_SYSTEM_TICK;

/*
 * Type Definition for Task Management 
 */

typedef struct
{
  NU_TASK       TCB;
  ULONG         magic_nr;
  OS_HANDLE     handle;
} OS_NU_TASK;

typedef struct
{
  char          Name[RESOURCE_NAMELEN];  	       /* name of the protocol stack entity */  
#ifdef _ESF_SUPPORT_
  ULONG         Value;                           /* additional parameter */
#endif
  OS_NU_TASK    TaskCB;                 			   /* control block of the thread */
  void         	(*TaskEntry)(OS_HANDLE, ULONG);  /* NUCLEUS task entry function */
  ULONG         *Stack;                          /* start address of stack memory */
} T_OS_TASK_TABLE_ENTRY;

/*
 * Type Definition for Queue Management 
 */

typedef struct _T_QDATA_ELEMENT
{
  OS_QDATA Data;
  struct _T_QDATA_ELEMENT  *pNext;
} T_QDATA_ELEMENT;

typedef struct
{
  OS_QDATA **pWrite;
  OS_QDATA **pRead;
  OS_QDATA **pStart;
} T_QUEUE;

typedef struct
{
  ULONG  opc;
  ULONG  time;
  void   *ptr;
  USHORT type;
} T_QUEUE_MSG;

typedef struct
{
  char            Name [ RESOURCE_NAMELEN ]; 		/* name of the queue */  
  T_QUEUE         Queue [ OS_MAX_PRIORITY ];    /* read/write pointers to ringlist */
  T_QDATA_ELEMENT *pQueueMemory;                /* pointer to queue memory, used for destroy */
  T_QDATA_ELEMENT *pFreeElement;                /* pointer to next free element */
  USHORT          Entries;                      /* queue entries */
  USHORT          MaxUsed;
  NU_SEMAPHORE    FreeSemCB;
  NU_SEMAPHORE    UsedSemCB;
  T_QUEUE_MSG     current_msg;
  T_VOID_STRUCT   *QueueData;
} T_OS_COM_TABLE_ENTRY;

/*
 * Type Definition for Timer Management 
 */

#define TMR_FREE   0
#define TMR_USED   1
#define TMR_ACTIVE 2

typedef struct _T_OS_TIMER_TABLE_ENTRY
{
  OS_HANDLE       t_handle;				          /* timer handle */  
  OS_HANDLE       task_handle;				      /* task that started the timer */  
  OS_HANDLE       entity_handle;				    /* entity that started the timer */  
  void         		(*TimeoutProc)(OS_HANDLE,OS_HANDLE,USHORT);			    /* timeout function */
  ULONG           r_ticks;                  /* remaining ticks */
  ULONG			      p_ticks;			            /* periodic expiration ticks */
  USHORT		      status;					          /* timer status */
  USHORT		      t_index;				          /* index of the timer */
  struct _T_OS_TIMER_TABLE_ENTRY *next;
  struct _T_OS_TIMER_TABLE_ENTRY *prev;
} T_OS_TIMER_TABLE_ENTRY;

typedef struct
{
  OS_HANDLE                next_t_handle;   /* handle of the next free table entry */  
  T_OS_TIMER_TABLE_ENTRY   entry;           /* pointer to timer table entry */
} T_OS_TIMER_ENTRY;

/*
 * Type Definition for Semaphore Management 
 */

typedef struct
{
  char          Name[ RESOURCE_NAMELEN ];    /* name of the semaphore */   
  NU_SEMAPHORE  SemCB;                 			 /* control block of the semaphore */
} T_OS_SEM_TABLE_ENTRY;

/*
 * Type Definition for HISR Management 
 */

typedef struct
{
  char          name[RESOURCE_NAMELEN];  	   /* name of the protocol stack entity */  
  NU_HISR       hisr_cb;                  	 /* control block of the HISR */
  ULONG         *stack;                      /* start address of stack memory */
} T_OS_OSISR_TABLE_ENTRY;

/*
 * Type Definition for Partition Pool Management 
 */
typedef struct _T_OS_PART_POOL
{
  struct _T_OS_PART_POOL * next;  
  unsigned int size;
  T_VOID_STRUCT * pool_mem;
  NU_PARTITION_POOL pcb;
} T_OS_PART_POOL;

typedef struct
{
  T_OS_PART_POOL * grp_head;  
  char name[RESOURCE_NAMELEN];  	     
} T_OS_PART_GRP_TABLE_ENTRY;

#ifdef NU_DEBUG
typedef struct
{
  char *Start;
  char *End;
} T_OS_POOL_BORDER;
#endif

/*
 * Type Definition for Dynamic Memory Pool Management 
 */
typedef struct 
{
  char name[RESOURCE_NAMELEN];  	     
  NU_MEMORY_POOL * pcb;
} T_OS_MEM_POOL_TABLE_ENTRY;


typedef struct
{
  char            Name[ RESOURCE_NAMELEN ]; /* name of the event group */
  NU_EVENT_GROUP  EvtGrp;                   /* control block of the event group */
} T_OS_EVTGRP_TABLE_ENTRY;


#endif

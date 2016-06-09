/* 
+------------------------------------------------------------------------------
|  File:       os.h
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
|  Purpose :  Definitions for the OS adaptation layer.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __OS_H__
#define __OS_H__

#include "gpfconf.h"
#include "os_const.h"

/*==== CONSTANTS ===================================================*/
typedef int                             OS_HANDLE;
typedef BOOL                         OS_INT_STATE;
typedef ULONG                             OS_TIME;
typedef ULONG                             OS_TICK;

#define OS_NOTASK                             0
                                              
#define FIRST_ENTRY                           0xff
#define NEXT_ENTRY                            0xfe

#define OS_EVENT                              0x1
#define OS_NO_EVENT                           0x0
/*
 * pattern to initialize task/osisr stacks 
 */
#define INITIAL_STACK_VALUE                   0xfe

/*
 * constants to access the object information functions 
 */
#define OS_OBJSYS                               0
#define OS_OBJTASK                              1
#define OS_OBJQUEUE                             2
#define OS_OBJPARTITIONGROUP                    3
#define OS_OBJMEMORYPOOL                        4
#define OS_OBJTIMER                             5
#define OS_OBJSEMAPHORE                         6	

/*
 * return values
 */
#define OS_OK                                   0
#define OS_WAITED                               1
#define OS_NEW_PROCESS                          2
#define OS_PARTITION_FREE                       3
#define OS_ALLOCATED_BIGGER                     4
#define OS_ERROR                              (-1)
#define OS_TIMEOUT                            (-2)
#define OS_PARTITION_GUARD_PATTERN_DESTROYED  (-3)
#define OS_INVALID_QUEUE                      (-4)

/*
 * message priorities
 */
#define OS_NORMAL                               1
#define OS_URGENT                               2
#define OS_MIN_PRIORITY                 OS_NORMAL
#define OS_MAX_PRIORITY                 OS_URGENT

/*
 * OS ISR priority range
 */
#define OS_ISR_MIN_PRIO                         0
#define OS_ISR_MAX_PRIO                         2

/*
 * flags
 */
#define OS_QFPARTITION                       0x01

/*
 * error codes
 */
#define OS_SYST_ERR                        0x8000
#define OS_SYST_ERR_QUEUE_CREATE           0x8001
#define OS_SYST_ERR_MAX_TIMER              0x8002
#define OS_SYST_ERR_MAX_TASK               0x8003 
#define OS_SYST_ERR_STACK_OVERFLOW         0x8004
#define OS_SYST_ERR_PCB_PATTERN            0x8005
#define OS_SYST_ERR_NO_PARTITION           0x8006
#define OS_SYST_ERR_STR_TOO_LONG           0x8007
#define OS_SYST_ERR_OVERSIZE               0x8008 
#define OS_SYST_ERR_TASK_TIMER             0x8009
#define OS_SYST_ERR_SIMUL_TIMER            0x800A
#define OS_SYST_ERR_QUEUE_FULL             0x800B
#define OS_SYST_ERR_MAX_SEMA               0x800C 
#define OS_SYST_ERR_NO_MEMORY              0x800D
#define OS_SYST_ERR_BIG_PARTITION          0x800E
                                            
/*
 * warning codes
 */                                   
#define OS_SYST_WRN                        0x0000
#define OS_SYST_WRN_WAIT_PARTITION         0x0001
#define OS_SYST_WRN_WAIT_QUEUE             0x0002
#define OS_SYST_WRN_BIG_PARTITION          0x0003
#define OS_SYST_WRN_MULTIPLE_FREE          0x0004
#define OS_SYST_WRN_REQ_TRUNCATED          0x0004
#define OS_SYST_WRN_FREE_FAILED            0x0005

/*==== TYPES =======================================================*/

typedef struct 
{
  USHORT            flags;
  USHORT            data16;
  ULONG             data32;
#ifdef _TOOLS_
  ULONG             len;
#endif
  ULONG             time;
  LONG              e_id;
  T_VOID_STRUCT *   ptr;
} OS_QDATA;

/*==== PROTOTYPES ==================================================*/

/* Task API */
GLOBAL LONG os_CreateTask             (OS_HANDLE Caller, char *Name, void (*TaskEntry)(OS_HANDLE, ULONG), ULONG StackSize, 
                                       USHORT Priority, OS_HANDLE *TaskHandle, OS_HANDLE MemPoolHandle);
GLOBAL LONG os_DestroyTask            (OS_HANDLE Caller, OS_HANDLE TaskHandle);
GLOBAL LONG os_StartTask              (OS_HANDLE Caller, OS_HANDLE TaskHandle, ULONG Value);
GLOBAL LONG os_StopTask               (OS_HANDLE Caller, OS_HANDLE TaskHandle);
GLOBAL LONG os_SuspendTask            (OS_HANDLE Caller, ULONG Time);
GLOBAL LONG os_DeferTask              (OS_HANDLE task_handle, OS_TIME time);
GLOBAL LONG os_ResumeTask             (OS_HANDLE task_handle);
GLOBAL LONG os_Relinquish             (void);
GLOBAL LONG os_GetTaskName            (OS_HANDLE Caller, OS_HANDLE TaskHandle, char * Name);
GLOBAL LONG os_GetTaskHandle          (OS_HANDLE Caller, char * Name, OS_HANDLE *TaskHandle);
GLOBAL LONG os_TaskInformation        (USHORT Handle, char *Buffer);
GLOBAL LONG os_ProInit                (void);
GLOBAL LONG os_ChangePreemption       (char preempt);
GLOBAL OS_HANDLE os_MyHandle          (void);
#ifdef _NUCLEUS_
GLOBAL LONG os_CheckTaskStack         (OS_HANDLE Handle);
/* Task internal */
GLOBAL LONG os_GetTaskData            (OS_HANDLE Handle, unsigned int **tcb, unsigned char **stackbegin, unsigned char **stackend );
GLOBAL unsigned char os_GetTaskState  (OS_HANDLE Caller, OS_HANDLE Handle);
#endif

/* Queue API */
GLOBAL LONG os_CreateQueue            (OS_HANDLE Caller, OS_HANDLE ComHandle, char *Name, USHORT Entries,
                                       OS_HANDLE *ActHandle, OS_HANDLE MemPoolHandle );
GLOBAL LONG os_DestroyQueue           (OS_HANDLE Caller, OS_HANDLE ComHandle );
GLOBAL LONG os_OpenQueue              (OS_HANDLE Caller, char *Name, OS_HANDLE *ComHandle);
GLOBAL LONG os_CloseQueue             (OS_HANDLE Caller, OS_HANDLE ComHandle);
GLOBAL LONG os_SendToQueue            (OS_HANDLE Caller, OS_HANDLE ComHandle, USHORT Priority, 
                                       ULONG Suspend, OS_QDATA *Msg );
GLOBAL LONG os_ReceiveFromQueue       (OS_HANDLE Caller, OS_HANDLE ComHandle, OS_QDATA *msg, ULONG Timeout );
GLOBAL LONG os_GetQueueName           (OS_HANDLE Caller, OS_HANDLE ComHandle, char * Name);
GLOBAL LONG os_GetQueueHandle         (OS_HANDLE Caller, char *Name, OS_HANDLE *ComHandle);
GLOBAL LONG os_QueueInformation       (USHORT Handle, char *Buffer);
GLOBAL LONG os_ComInit                (void);
/* Queue internal */
#ifdef _NUCLEUS_
GLOBAL LONG os_GetQueueState          (OS_HANDLE Caller, OS_HANDLE Handle, ULONG *Used, ULONG *Free);
GLOBAL unsigned char *os_FindSuspendingQueue (unsigned int *tcb);
GLOBAL LONG os_GetQueueData           (OS_HANDLE Caller, OS_HANDLE Handle, USHORT Index, 
                                       USHORT *Type, ULONG *opc, ULONG *ptr, ULONG *time );
#endif
#ifdef _TOOLS_
extern LONG os_create_extq            (const char* name, OS_HANDLE* comhandle);
extern LONG os_destroy_extq           (const char* name);
#endif /* _TOOLS_ */

/* Memory API */
GLOBAL LONG os_CreatePartitionPool    (OS_HANDLE Caller, char *GroupName, void *Addr, USHORT Num, ULONG Size, 
                                       OS_HANDLE *GroupHandle);
GLOBAL LONG os_CreatePartitionPool_fixed_pool_size  (OS_HANDLE TaskHandle, char *GroupName, void *Addr, USHORT PoolSize, 
                                       ULONG PartitionSize, OS_HANDLE *GroupHandle, ULONG *NumCreated);
GLOBAL LONG os_AllocatePartition      (OS_HANDLE Caller, T_VOID_STRUCT **Buffer, ULONG Size, 
                                       ULONG Suspend, OS_HANDLE GroupHandle);
GLOBAL LONG os_DeallocatePartition    (OS_HANDLE Caller, T_VOID_STRUCT *Buffer);
GLOBAL LONG os_CreateMemoryPool       (OS_HANDLE Caller, char *Name, void *Addr, ULONG PoolSize, 
                                       OS_HANDLE *PoolHandle);
GLOBAL LONG os_AllocateMemory         (OS_HANDLE Caller, T_VOID_STRUCT **Buffer, ULONG Size, 
                                       ULONG Suspend, OS_HANDLE PoolHandle);
GLOBAL LONG os_DeallocateMemory       (OS_HANDLE Caller, T_VOID_STRUCT *Buffer);
GLOBAL LONG os_PartitionInformation   (USHORT Handle, char *Buffer);
GLOBAL LONG os_MemoryInformation      (USHORT Handle, char *Buffer);
GLOBAL LONG os_MemInit                (void);
GLOBAL LONG os_SetPoolHandles         (OS_HANDLE ext_pool_handle, OS_HANDLE int_pool_handle);
GLOBAL LONG os_GetPartitionGroupHandle(OS_HANDLE Caller, char *Name, OS_HANDLE *GroupHandle);

GLOBAL LONG os_GetPartitionPoolStatus (ULONG size, OS_HANDLE gr_hndl, USHORT *free, USHORT *alloc);
/* Memory internal */
GLOBAL LONG os_is_valid_partition     (T_VOID_STRUCT *Buffer);
#ifdef _NUCLEUS_
GLOBAL LONG os_PartitionCheck         (ULONG *ptr);
GLOBAL const ULONG *os_GetPrimpoolCB  (int grp,int id);
#endif

/* Timer API */
GLOBAL LONG os_CreateTimer            (OS_HANDLE TaskHandle, void(*TimeoutProc)(OS_HANDLE,OS_HANDLE,USHORT), 
                                       OS_HANDLE *TimerHandle, OS_HANDLE MemPoolHandle);
GLOBAL LONG os_DestroyTimer           (OS_HANDLE TaskHandle, OS_HANDLE TimerHandle);
GLOBAL LONG os_StartTimer             (OS_HANDLE TaskHandle, OS_HANDLE TimerHandle, USHORT Index,
                                       OS_TIME InitialTime, OS_TIME RescheduleTime );
GLOBAL LONG os_StopTimer              (OS_HANDLE TaskHandle, OS_HANDLE TimerHandle);
GLOBAL LONG os_QueryTimer             (OS_HANDLE TaskHandle, OS_HANDLE TimerHandle, OS_TIME *RemainingTime);
GLOBAL LONG os_TimerInformation       (USHORT Handle, char *Buffer);
GLOBAL LONG os_TimInit                (void);
GLOBAL LONG os_set_tick               (int os_system_tick);
GLOBAL LONG os_InactivityTicks        (int *next_event, OS_TICK *next_event_ticks);
GLOBAL LONG os_IncrementTick          (OS_TICK ticks);
GLOBAL LONG os_GetScheduleCount       (OS_HANDLE task_handle, int * schedule_count);
GLOBAL LONG os_RecoverTick            (OS_TICK ticks);

/* Semaphore API */
GLOBAL LONG os_CreateSemaphore        (OS_HANDLE TaskHandle, char *Name, USHORT Count, 
                                       OS_HANDLE *Semhandle, OS_HANDLE MemPoolHandle);
GLOBAL LONG os_DestroySemaphore       (OS_HANDLE TaskHandle, OS_HANDLE SemHandle);
GLOBAL LONG os_ResetSemaphore         (OS_HANDLE TaskHandle, OS_HANDLE SemHandle, USHORT Count);
GLOBAL LONG os_OpenSemaphore          (OS_HANDLE TaskHandle, char *Name, OS_HANDLE *SemHandle);
GLOBAL LONG os_CloseSemaphore         (OS_HANDLE TaskHandle, OS_HANDLE SemHandle);
GLOBAL LONG os_ObtainSemaphore        (OS_HANDLE TaskHandle, OS_HANDLE SemHandle, ULONG Timeout);
GLOBAL LONG os_ReleaseSemaphore       (OS_HANDLE TaskHandle, OS_HANDLE SemHandle);
GLOBAL LONG os_QuerySemaphore         (OS_HANDLE TaskHandle, OS_HANDLE SemHandle, USHORT *Count);
GLOBAL LONG os_SemaphoreInformation   (USHORT Handle, char *Buffer);
GLOBAL LONG os_SemInit                (void);
/* Semaphore internal */
#ifdef _NUCLEUS_
GLOBAL unsigned char *os_FindSuspendingSema (unsigned int *tcb);
#endif

/* Interrupt API */
GLOBAL LONG  os_CreateOSISR           (char *name, void (*OSISR_entry)(void), int stacksize, int priority, 
                                       int flags, OS_HANDLE *osisr_handle );
GLOBAL LONG os_DeleteOSISR            (OS_HANDLE osisr_handle);
GLOBAL LONG os_ActivateOSISR          (OS_HANDLE osisr_handle); 
GLOBAL LONG os_SetInterruptState      (OS_INT_STATE new_state, OS_INT_STATE *old_state); 
GLOBAL LONG os_EnableInterrupts       (OS_INT_STATE *old_state); 
GLOBAL LONG os_DisableInterrupts      (OS_INT_STATE *old_state); 
GLOBAL LONG os_isr_init               (void);

/* Event group API */
GLOBAL LONG os_CreateEventGroup       (char *evt_grp_name, OS_HANDLE *evt_grp_handle);
GLOBAL LONG os_DeleteEventGroup       (OS_HANDLE evt_grp_handle);
GLOBAL LONG os_EventGroupInformation  (OS_HANDLE evt_grp_handle, char *Name, unsigned* mask_evt, unsigned* tasks_waiting, OS_HANDLE* first_task);
GLOBAL LONG os_SetEvents              (OS_HANDLE evt_grp_handle, unsigned event_flags);
GLOBAL LONG os_ClearEvents            (OS_HANDLE evt_grp_handle, unsigned event_flags);
GLOBAL LONG os_RetrieveEvents         (OS_HANDLE evt_grp_handle, unsigned event_flags, char option, unsigned* retrieved_events, unsigned suspend);
GLOBAL LONG os_EvGrpInit              (void);
GLOBAL LONG os_GetEventGroupHandle    (char *evt_grp_name, OS_HANDLE *evt_grp_handle);

/* Miscellaneous */
GLOBAL LONG os_GetTime                (OS_HANDLE TaskHandle, OS_TIME *Time);
GLOBAL LONG os_Initialize             (void);
GLOBAL LONG os_ObjectInformation      (OS_HANDLE Caller, USHORT Id, USHORT Handle, USHORT len, void *Buffer);
void os_SystemError                   (OS_HANDLE Caller, USHORT cause, char *buffer );
LONG os_dar_register                  (const void *dar_properties);
#ifdef _NUCLEUS_
LONG os_dar_set_filter                (void);
LONG os_read_dar_ffs_data             (USHORT entry, char *buffer, USHORT len);
#endif
#if defined (_NUCLEUS_) && defined (_TARGET_)
#include "gdi.h"
GLOBAL LONG                            os_CreateCallback (void);
GLOBAL LONG os_ExecuteCallback        (OS_HANDLE Caller, void (*Callback)(T_DRV_SIGNAL*), T_DRV_SIGNAL *Signal); 
#endif
#ifdef CTB
GLOBAL void os_Tick                   (void);
GLOBAL void os_StartTicking           (void);
GLOBAL void os_StopTicking            (void);
GLOBAL ULONG os_GetProcessId          (void);
#endif 

/*==== END OF OS.H =================================================*/
#endif

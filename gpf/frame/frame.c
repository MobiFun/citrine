/* 
+------------------------------------------------------------------------------
|  File:       frame.c
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
|  Purpose :  This Modul defines the general frame functionality.
+----------------------------------------------------------------------------- 
*/ 


#ifndef __FRAME_C__
#define __FRAME_C__
#endif

/*==== INCLUDES ===================================================*/

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "gpfconf.h"
#include "typedefs.h"

#include "glob_defs.h"
#include "os.h"
#include "vsi.h"
#include "pei.h"
#include "frame.h"
#include "tools.h"
#include "gdi.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "route.h"
#include "p_frame.h"
#include "prf_func.h"
#ifdef _ESF_SUPPORT_
#include "esf_func.h"
#endif
#include "frm_ext.h"

/*==== TYPES ======================================================*/

typedef struct
{
  char const *Name;
  USHORT Id;
} T_NAME_ID;

/*==== CONSTANTS ==================================================*/

#define RUNNING   0x01
#undef VSI_CALLER
#define VSI_CALLER   TaskHandle,

/*==== EXTERNALS ==================================================*/
/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

#if defined _NUCLEUS_ && !defined _TARGET_
extern char TraceBuffer[];
#endif

#ifndef _TOOLS_
extern const T_MEMORY_POOL_CONFIG memory_pool_config[];
extern const T_FRM_PARTITION_GROUP_CONFIG partition_grp_config[];
extern T_HANDLE MemoryPoolHandle[];
extern OS_HANDLE PoolGroupHandle[];
extern const T_DRV_LIST DrvList[]; 
#ifdef MEMORY_SUPERVISION      
extern USHORT NumberOfPPMPartitions;
extern USHORT NumOfPPMPools;
extern USHORT NumOfPPMGroups;
extern USHORT NumOfPrimPools;
extern USHORT NumOfDmemPools;
#endif
#endif

extern T_DRV_LIST const *DriverConfigList;

#ifndef _TOOLS_
extern const T_COMPONENT_ADDRESS *ComponentTables[];
extern const char * const frame_version_date;
extern const char * const frame_version_time;
extern const char * const misc_version_date;
extern const char * const misc_version_time;
extern const char * const tif_version_date;
extern const char * const tif_version_time;
#endif

#ifdef _TOOLS_
  __declspec (dllimport) T_HANDLE TST_Handle;
#else
  extern T_HANDLE TST_Handle;
#endif

#ifdef MEMORY_SUPERVISION
extern int ppm_check_partition_owner;
#endif

/*==== VARIABLES ==================================================*/
    
#ifndef RUN_INT_RAM
UBYTE SuppressOK=1;
GLOBAL T_HANDLE MemPoolHandle;                                                 
GLOBAL T_HANDLE PrimGroupHandle;
GLOBAL T_HANDLE DmemGroupHandle;
GLOBAL T_HANDLE TestGroupHandle;
GLOBAL T_HANDLE LemuGroupHandle;
GLOBAL T_HANDLE int_data_pool_handle;
GLOBAL T_HANDLE ext_data_pool_handle;
GLOBAL UBYTE FrameEnv=0;
GLOBAL USHORT TestInterface = 0;
GLOBAL USHORT NextTimerEntry = 0;
int time_is_tdma_frame;
char error_ind_dst[RESOURCE_NAMELEN] = {0};
T_FRM_ERROR_IND *frm_error_ind = NULL;
char check_desclist = FALSE;
GLOBAL USHORT NumberOfStartedTasks = 0;
GLOBAL USHORT NumberOfRunningTasks = 0;
GLOBAL USHORT TooManyTasks = 0;

const T_PEI_INFO DummyInfo = 
  { 
    "",          /* Name */
    { 
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL
    },
    768,        /* stack size */
    10,         /* queue entries */
    1,          /* priority */
    0,          /* number of timers */
    COPY_BY_REF /* Flags */
 };

const T_NAME_ID Resource[] =
{
  { "TASK",       OS_OBJTASK},
  { "QUEUE",      OS_OBJQUEUE},
  { "TIMER",      OS_OBJTIMER },
  { "SEMAPHORE",  OS_OBJSEMAPHORE},
  { "PARTITION",  OS_OBJPARTITIONGROUP},
  { "MEMORY",     OS_OBJMEMORYPOOL},
  { NULL,         0 }
};

#ifdef _TOOLS_
LOCAL T_COMPONENT_ADDRESS *ComponentTables [NUM_OF_COMPONENT_TABLES+1]={0};
typedef void T_INIT_FUNC ( void );
T_INIT_FUNC *InitFunc;
ULONG init_stack_time = 0;
ULONG init_local_time = 0;
ULONG  MaxPrimPartSize = 65536;
USHORT TextTracePartitionSize = 260;
#endif

char TaskName [ RESOURCE_NAMELEN ];

#else /* RUN_INT_RAM */
extern USHORT TestInterface;
extern USHORT NextTimerEntry;
extern USHORT NumberOfStartedTasks;
extern USHORT NumberOfRunningTasks;
extern USHORT TooManyTasks;
extern char TaskName[];
extern T_HANDLE int_data_pool_handle;
extern T_HANDLE ext_data_pool_handle;
#endif /* RUN_INT_RAM */

#ifdef _TOOLS_
#pragma data_seg()
#endif

#ifdef _ESF_SUPPORT_
int esf_init_func2_ready = FALSE;
int firstTime = TRUE;
#endif

/* -------------- S H A R E D - END ---------------- */


/*==== PROTOTYPES =================================================*/

GLOBAL void pf_TaskEntry (T_HANDLE TaskHandle, ULONG Value );
LOCAL SHORT pf_HandleMessage (T_HANDLE TaskHandle, OS_QDATA *pMsg );
LOCAL LONG pf_CreateTask ( const T_COMPONENT_ADDRESS *Comp );
LOCAL void pf_ProcessProtocolPrim ( T_HANDLE TaskHandle, T_VOID_STRUCT *pPrim);
LOCAL void pf_Reset (T_HANDLE TaskHandle);
int is_entity_in_task (T_HANDLE t_handle, char *name );
int int_vsi_o_ttrace ( T_HANDLE Caller, ULONG TraceClass, const char * const format, va_list varpars );

#ifndef _TOOLS_
GLOBAL void InitializeApplication ( void );
#endif

/*==== LINT =======================================================*/

/*lint -e522 suppress Warning -- Expected void type, assignment, increment or decrement */

/*==== FUNCTIONS ==================================================*/

#ifdef _TOOLS_
/*
+------------------------------------------------------------------------------
|  Function     :  pf_get_frameenv
+------------------------------------------------------------------------------
|  Description  :  This function returns the current value of FrameEnv
|
|  Parameters   :  void
|
|  Return       :  FrameEnv
+------------------------------------------------------------------------------
*/
USHORT pf_get_frameenv (void)
{
  return (USHORT) FrameEnv;
}

#endif /* _TOOLS_ */

#ifndef _TOOLS_
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : StartFrame          |
+--------------------------------------------------------------------+

  PURPOSE : Start the frame 

*/
SHORT StartFrame ( void )
{
  prf_init();
  pf_Init(NULL);
  pf_CreateAllEntities();
#ifdef _ESF_SUPPORT_
  esf_init();
  esf_init_func1(); 
#endif
  pf_StartAllTasks ();
  return ( PF_OK );
}
#endif
#endif /* ndef _TOOLS_ */

#if defined (_LINUX_) || (defined _SOLARIS_)
int main ()
{
  (void) StartFrame ();
  for (;;)
  {
    os_SuspendTask (0, 1500);
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : is_entity_in_task   |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the frame 

*/
int is_entity_in_task ( T_HANDLE t_handle, char *name )
{
int i;

  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == t_handle )
    {
      if ( !strncmp (pf_TaskTable[i].Name, name, RESOURCE_NAMELEN-1) )
        return TRUE;
    }
  }
  return FALSE;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_Init             |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the frame 

*/

/*lint -esym(715,ConfigAddress) only needed for _TOOLS_*/
GLOBAL void pf_Init ( T_CONFIGURATION_ADDRESS *ConfigAddress)
{
#ifndef _TOOLS_
const T_FRM_PARTITION_POOL_CONFIG * pool_config;
USHORT i = 1,j = 0;
#endif

#if defined _NUCLEUS_ && !defined _TARGET_ || defined (_LINUX_)
  printf ("FRAME VERSION: %s, %s\n",frame_version_date, frame_version_time);
  printf ("MISC VERSION:  %s, %s\n",misc_version_date,  misc_version_time);
  printf ("TIF VERSION:   %s, %s\n\n",tif_version_date, tif_version_time);
#endif
  if ( os_Initialize() == OS_ERROR )
  {
    vsi_o_assert ( 0, OS_SYST_ERR, __FILE__, __LINE__, "OS initialization error" );
  }
  
  time_is_tdma_frame = 0;
  TestInterface = 0;
  NextTimerEntry = 0;
#ifdef _TOOLS_
  ComponentTables[RCV_ADR] = ConfigAddress->RcvAdr; 
  ComponentTables[TST_ADR] = ConfigAddress->TstAdr; 
  ComponentTables[END_OF_COMP_TABLE] = NULL; 
  DriverConfigList = ConfigAddress->DrvListAdr;
  InitFunc = ConfigAddress->InitFuncAdr;
  FrameEnv = *ConfigAddress->FrameEnvAdr;
#else
  DriverConfigList = &DrvList[0];
#endif

#ifndef _TOOLS_

  
  j = 0;
  /*
   * create memory pools
   */
  while ( memory_pool_config[j].Name != NULL )
  {
    if ( memory_pool_config[j].Size > 1 )
      os_CreateMemoryPool ( NO_TASK, 
                          memory_pool_config[j].Name, 
                          memory_pool_config[j].PoolAddress, 
                          memory_pool_config[j].Size, 
                          (OS_HANDLE*)MemoryPoolHandle[j] );                    
    j++;
  }

#ifdef _NUCLEUS_
  os_SetPoolHandles (ext_data_pool_handle, int_data_pool_handle);
#endif

  /*
   * create partition pools
   */

  for ( i = 0; partition_grp_config[i].name != NULL; i++ )
  {
#ifdef MEMORY_SUPERVISION      
//    if ( strcmp ("TEST", partition_grp_config[i].name ) )
//    {
      /* currently all created groups are counted to ease offset calculation for
         partition supervision */
      NumOfPPMGroups++;
//    }
#endif
    pool_config = partition_grp_config[i].grp_config;
    while ( pool_config->part_size && pool_config->part_num )
    {
      os_CreatePartitionPool ( NO_TASK, 
                               partition_grp_config[i].name, 
                               pool_config->mem, 
                               (USHORT)pool_config->part_num,
                               pool_config->part_size, 
                               (OS_HANDLE*)PoolGroupHandle[i] );   
#ifdef MEMORY_SUPERVISION 
	  /* TEST pool not under partition supervision */
      if ( strcmp ("TEST", partition_grp_config[i].name ) )
      {
        NumOfPPMPools++;
        NumberOfPPMPartitions += pool_config->part_num;
      }
#endif
      if ( !strcmp ("PRIM", partition_grp_config[i].name ) )
      {
        if ( MaxPrimPartSize < pool_config->part_size )
        {
          MaxPrimPartSize = pool_config->part_size;
        }
#ifdef MEMORY_SUPERVISION
        NumOfPrimPools++;
#endif
      }
#ifdef MEMORY_SUPERVISION
      if ( !strcmp ("DMEM", partition_grp_config[i].name ) )
      {
        NumOfDmemPools++;
      }
#endif
      pool_config++;
    }
  }

#endif
  /* 
   * To allow CCD (TaskHandle = 0) the usage of dynamic Memory to create a semaphore
   * the MemPoolHandle for non-task users is initialized with the handle of the int_data_pool
   * pool.
   */
  pf_TaskTable[0].MemPoolHandle = int_data_pool_handle;
  strcpy ( pf_TaskTable[0].Name, "IRQ" );

  rt_Init();
#ifdef _TOOLS_
  (InitFunc)();
#else
  InitializeApplication();
#endif
#ifdef MEMORY_SUPERVISION
  InitializePPM();
#endif /* MEMORY_SUPERVISION */

  InitializeTimer();
  InitializeDriverConfig();
#if !defined _TARGET_ && !defined _TOOLS_
  vsi_pcheck_init(); 
#endif
#ifndef _TOOLS_
  fei_lemu_SendToQueue_init();
#endif
  /* 
  not needed -> temporarily removed 
  vsi_c_init_com_matrix (MaxEntities);
  */
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_CreateAllEntities|
+--------------------------------------------------------------------+
*/
GLOBAL SHORT pf_CreateAllEntities (void)
{
int i = 0;

  while ( ComponentTables[i] != NULL )
  {
    pf_CreateTask ( ComponentTables[i] );
    i++;
  }
  InitializeTrace();
  return PF_OK;
}
#endif
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_CreateEntity     |
+--------------------------------------------------------------------+
*/
LOCAL LONG pf_CreateTask ( const T_COMPONENT_ADDRESS *Comp )
{
T_PEI_INFO const *Info;
T_HANDLE TaskHandle;
T_HANDLE mem_pool_handle = ext_data_pool_handle;
int Priority = 0;
ULONG StackSize = 0;
const T_PEI_FUNC *PeiTable;
USHORT     QueueEntries = 0;
USHORT   	 NumOfTimers = 0;
U32      Flags;
char *Name = NULL;
static int e_handle = 1;
int start_e_handle;
int i;

  start_e_handle = e_handle;
  while ( Comp->PeiCreate || Comp->Name )
  {
    if ( e_handle > MaxEntities )
    {
      vsi_o_assert ( 0, OS_SYST_ERR, __FILE__, __LINE__, "More than MAX_ENTITIES" );
    }
    if (  Comp->PeiCreate && Comp->PeiCreate ( &Info ) == PEI_OK )
    {
      if ( Comp->Priority != ASSIGNED_BY_TI )
      {
        if ( Priority < Comp->Priority )
          Priority = Comp->Priority;
      }
      else
      {
        if ( Priority < Info->Priority )
          Priority = Info->Priority;
      }
      Flags = Info->Flags;
      PeiTable = &Info->PeiTable;
      Name = (char*)Info->Name;
      if ( StackSize < Info->StackSize )
        StackSize = Info->StackSize;
      pf_TaskTable[e_handle].QueueEntries = Info->QueueEntries;
      NumOfTimers = Info->NumOfTimers;
    }
    else if ( Comp->Name && strlen (Comp->Name) <= RESOURCE_NAMELEN )
    {
      Flags = DummyInfo.Flags;
      PeiTable = &DummyInfo.PeiTable;
      Name = Comp->Name;
      if ( StackSize < DummyInfo.StackSize )
        StackSize = DummyInfo.StackSize;
      if ( Priority < DummyInfo.Priority )
        Priority = DummyInfo.Priority;
      pf_TaskTable[e_handle].QueueEntries = DummyInfo.QueueEntries;
      NumOfTimers = DummyInfo.NumOfTimers;
    }
    else
      return PF_ERROR;

    if ( QueueEntries < pf_TaskTable[e_handle].QueueEntries )
      QueueEntries = pf_TaskTable[e_handle].QueueEntries;

    pf_TaskTable[e_handle].Flags = Flags;
    pf_TaskTable[e_handle].PeiTable = PeiTable;
    pf_TaskTable[e_handle].NumOfTimers = NumOfTimers;
    strncpy (pf_TaskTable[e_handle].Name, Name, RESOURCE_NAMELEN);
    pf_TaskTable[e_handle].Name[RESOURCE_NAMELEN-1] = 0;
    if ( pf_TaskTable[e_handle].Flags & INT_DATA_TASK )
    {
      pf_TaskTable[e_handle].MemPoolHandle = int_data_pool_handle;
      mem_pool_handle = int_data_pool_handle;
    }
    else
    {
      pf_TaskTable[e_handle].MemPoolHandle = ext_data_pool_handle;
      mem_pool_handle = ext_data_pool_handle;
    }
    prf_log_entity_create ((void*)e_handle, Name);
    e_handle++;
    Comp++;
  }
  if ( e_handle > start_e_handle + 1 )
    Name = (char*)Comp->Priority;

#ifdef MEMORY_SUPERVISION
  StackSize = StackSize + (StackSize>>3);
#endif
  if ( os_CreateTask (NO_TASK, Name, pf_TaskEntry, StackSize, (USHORT)Priority, &TaskHandle, 
                      mem_pool_handle) == OS_OK )
  {
    for ( i = start_e_handle; i < e_handle; i++ )
    {
      pf_TaskTable[i].TaskHandle = TaskHandle;
    }
    return PF_OK;
  }
  else
    vsi_o_assert ( NO_TASK, OS_SYST_ERR, __FILE__, __LINE__, "Error at creating %s Task", Name );  
  /*lint +e771 */

  return PF_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_StartAllTasks    |
+--------------------------------------------------------------------+
*/
GLOBAL SHORT pf_StartAllTasks ( void )
{
int e_handle;
int t_handle;
int started_t_handle = 0;

  for ( e_handle = 1; e_handle <= MaxEntities; e_handle++ )
  {
    if ( pf_TaskTable[e_handle].Name[0] != 0 )
    {
      if ( (t_handle = pf_TaskTable[e_handle].TaskHandle) != started_t_handle )
      {
        if ( os_StartTask ( NO_TASK, t_handle, 0 ) == OS_ERROR)
          return OS_ERROR;
        NumberOfStartedTasks++;
        started_t_handle = t_handle;
      }
    }
  }
  return OS_OK;
}
#endif


#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_TaskEntry        |
+--------------------------------------------------------------------+
*/
/*lint -esym(715,Value) suppress Info -- Symbol 'Value' not referenced */
GLOBAL void pf_TaskEntry (T_HANDLE TaskHandle, ULONG Value )
{
OS_QDATA Msg;
OS_HANDLE mem_pool_handle = ext_data_pool_handle;
LONG sts;
int i;
int queue_entries;
int queue_handle;
int biggest_queue_size = 0;
int sum_queue_size = 0;
U32 entity_flags = 0;
#ifndef _TOOLS_
char queue_name [ RESOURCE_NAMELEN ];
#endif
static int entity_init_fail_cnt = 0;

#ifdef _ESF_SUPPORT_
  {
    /* OS timer should not be started at this point, otherwise risk of
       race condition.
       Furthermore, ESF THIF socket interface can't be used by task tst 
       before esf_init_func2() is completed. 
     */

    T_HANDLE esf_init_func2_sem;
    T_HANDLE caller;

    caller = e_running[os_MyHandle()];
    esf_init_func2_sem = vsi_s_open (caller, "esf_init_func2_sem", 1);

    vsi_s_get (caller, esf_init_func2_sem);      /* start of critical section */
    if (firstTime)
    {
      firstTime = FALSE;
      esf_init_func2();
      esf_init_func2_ready = TRUE; 
    }
    vsi_s_release(caller, esf_init_func2_sem); /* end of critical section */

    while (!esf_init_func2_ready)
    {
      os_SuspendTask(caller, 100);
    }
  }
#endif

  if ( is_entity_in_task ( TaskHandle, FRM_TST_NAME ) == FALSE
    && is_entity_in_task ( TaskHandle, FRM_RCV_NAME ) == FALSE )
  {
    while ( !(TestInterface & RUNNING) )
    {
      os_SuspendTask (TaskHandle, 100);
    }
  }

  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == TaskHandle )
    {
      if ( pf_TaskTable[i].Flags & ADD_QUEUE_SIZES )
      {
        entity_flags |= ADD_QUEUE_SIZES;
      }
      sum_queue_size += pf_TaskTable[i].QueueEntries;
      if ( biggest_queue_size < pf_TaskTable[i].QueueEntries )
      {
        biggest_queue_size = pf_TaskTable[i].QueueEntries;
      }
      if ( pf_TaskTable[i].Flags & INT_DATA_TASK )
        mem_pool_handle = int_data_pool_handle;
    }
  }

  /* set queue size depending on the flag exported by the entities */
  if ( entity_flags & ADD_QUEUE_SIZES )
  {
    queue_entries = sum_queue_size;
  }
  else
  {
    queue_entries = biggest_queue_size;
  }

#ifdef _TOOLS_
  if ( os_CreateQueue ( TaskHandle, 0, pf_TaskTable[TaskHandle].Name, (USHORT)queue_entries, &queue_handle,
                        pf_TaskTable[TaskHandle].MemPoolHandle) == OS_ERROR )
  {
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_QUEUE_CREATE, __FILE__, __LINE__, 
                   "Error at creating %s Queue", pf_TaskTable[TaskHandle].Name ); 
  }
#else
  os_GetTaskName ( TaskHandle, TaskHandle, queue_name );
  if ( os_CreateQueue ( TaskHandle, 0, queue_name, (USHORT)queue_entries, &queue_handle, mem_pool_handle) == OS_ERROR )
  {
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_QUEUE_CREATE, __FILE__, __LINE__, "Error at creating %s Queue", queue_name ); 
  }
#endif

  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == TaskHandle )
    {
      pf_TaskTable[i].QueueHandle = queue_handle;
      pf_TaskTable[i].FirstTimerEntry = &TimerHandleField [ NextTimerEntry ];
      if ( (NextTimerEntry += pf_TaskTable[i].NumOfTimers) >= MaxTimer )
        vsi_o_assert ( NO_TASK, OS_SYST_ERR_MAX_TIMER, __FILE__, __LINE__, 
                       "Number of Timers > MAX_TIMER" );  
    }
  }

  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == TaskHandle )
    {
      if (pf_TaskTable[i].PeiTable->pei_init != NULL)
      {
        e_running[TaskHandle] = i;
        while (pf_TaskTable[i].PeiTable->pei_init (i) == PEI_ERROR)
        {
          if ( entity_init_fail_cnt++ > (NumberOfStartedTasks * 5) )
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "%s pei_init() failed",pf_TaskTable[i].Name );
		      os_SuspendTask (TaskHandle, 100);
        }
        e_running[TaskHandle] = 0;
      }
    }
  }

  if ( is_entity_in_task ( TaskHandle, FRM_TST_NAME ) == TRUE ) 
    TestInterface |= RUNNING;


  if ( ++NumberOfRunningTasks == NumberOfStartedTasks )
  {
    if ( TooManyTasks )
      vsi_o_assert (NO_TASK, OS_SYST_ERR_MAX_TASK, __FILE__, __LINE__, "Number of entities > MAX_ENTITIES" );  
    vsi_o_ttrace(NO_TASK, TC_SYSTEM, "All tasks entered main loop" );
#if defined _NUCLEUS_ && !defined _TARGET_
    printf ("%s\n","All tasks entered main loop");
#endif
#ifdef _TARGET_
    // TraceMask[0] = 0;	// removed in FreeCalypso - Space Falcon
    os_dar_set_filter();
#endif
  }
  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == TaskHandle )
    {
      if (pf_TaskTable[i].PeiTable->pei_run != NULL)
      {
        if ( !(pf_TaskTable[i].Flags & PASSIVE_BODY) )
        {
            /*
             * in the active body variant call pei_run
             */
            e_running[TaskHandle] = i;
            pf_TaskTable[i].PeiTable->pei_run (TaskHandle, pf_TaskTable[i].QueueHandle );
#ifdef _TOOLS_
            pf_TaskTable[i].PeiTable->pei_exit();
            NextTimerEntry -= pf_TaskTable[i].NumOfTimers;
            rt_RoutingModify ( TaskHandle, (char*)SYSPRIM_REDIRECT_TOKEN, (char*)SYSPRIM_CLEAR_TOKEN );
            os_DestroyQueue ( TaskHandle, pf_TaskTable[i].QueueHandle );
            e_running[TaskHandle] = 0;
            os_DestroyTask ( TaskHandle, TaskHandle );
#endif
            e_running[TaskHandle] = 0;
            for (;;)
              os_SuspendTask(TaskHandle,10000);
        }
      }
    }
  }

  /*
   * in the passive body variant wait for a message and
   * handle it
   */
  for ( i = MaxEntities; i > 0; i-- )
  {
    if ( pf_TaskTable[i].TaskHandle == TaskHandle )
    {
      break;
    }
  }
  for (;;)
  {
    sts = os_ReceiveFromQueue ( TaskHandle, pf_TaskTable[i].QueueHandle,
                                &Msg, OS_SUSPEND );
    switch ( sts )
    {
      case OS_OK:
#if defined (_TOOLS_) || defined (_LINUX_)
           e_running[TaskHandle] = TaskHandle;
           pf_HandleMessage (TaskHandle, &Msg);
#else
           e_running[TaskHandle] = Msg.e_id;
           prf_log_entity_activate ((void*)Msg.e_id);
           pf_HandleMessage (Msg.e_id, &Msg);
#endif
           e_running[TaskHandle] = 0;
#ifdef _NUCLEUS_ 
           if ( os_CheckTaskStack ( TaskHandle ) == OS_ERROR )
           {
             os_GetTaskName ( TaskHandle, TaskHandle, TaskName );
             vsi_o_assert ( TaskHandle, OS_SYST_ERR_STACK_OVERFLOW, __FILE__, __LINE__, 
                            "%s Stack overflow", TaskName );
           }
#endif
      break;
      case OS_TIMEOUT:
      break;
      case OS_ERROR:
           for(;;)
             os_SuspendTask(TaskHandle,10000);
      /*lint -e527, suppress Warning -- Unreachable */
      break;
      default:
           for(;;)
             os_SuspendTask(TaskHandle,10000);
      break;
      /*lint +e527 */

    }
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_Timeout          |
+--------------------------------------------------------------------+
*/
GLOBAL void pf_Timeout (T_HANDLE TaskHandle, T_HANDLE EntityHandle, USHORT TimerIndex )
{
OS_QDATA TimeoutMsg;

  TimeoutMsg.data16 = MSG_TIMEOUT;
  TimeoutMsg.data32 = (ULONG)TimerIndex;
#ifdef _TOOLS_
  TimeoutMsg.len = 0;
#endif
  TimeoutMsg.e_id = EntityHandle;
  os_GetTime ( 0, &TimeoutMsg.time );
    
  *(pf_TaskTable[EntityHandle].FirstTimerEntry + TimerIndex) |= TIMEOUT_OCCURRED;

#ifdef _TOOLS_
  if ( rt_Route (TaskHandle, pf_TaskTable[EntityHandle].QueueHandle, OS_NORMAL, OS_SUSPEND, 
    &TimeoutMsg ) == OS_TIMEOUT )
#else
  if ( rt_Route (TaskHandle, EntityHandle, OS_NORMAL, OS_SUSPEND, 
    &TimeoutMsg ) == OS_TIMEOUT )
#endif
      vsi_o_assert ( 0, OS_SYST_ERR_QUEUE_FULL, __FILE__, __LINE__, 
                     "Timeout write attempt to %s queue failed", pf_TaskTable[EntityHandle].Name );
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_HandleMessage    |
+--------------------------------------------------------------------+
*/
LOCAL SHORT pf_HandleMessage (T_HANDLE TaskHandle, OS_QDATA *pMsg )
{
ULONG PrimId = 0; 

  switch (pMsg->data16)
  {
    case MSG_PRIMITIVE:
      if ( pMsg->ptr != NULL )
      {
        PrimId = ((T_PRIM_HEADER*)(pMsg->ptr))->opc;

        if ( PrimId & SYS_MASK )
        {
          pf_ProcessSystemPrim ( TaskHandle, pMsg->ptr );
        }
        else
        {
          pf_ProcessProtocolPrim ( TaskHandle, pMsg->ptr );
        }
      }
    break;
    case MSG_SIGNAL:
      if ( pf_TaskTable[TaskHandle].PeiTable->pei_signal != NULL )
      {
        pf_TaskTable[TaskHandle].PeiTable->pei_signal ( pMsg->data32, pMsg->ptr ); 
      }
    break;
    case MSG_TIMEOUT:
      if ( *(pf_TaskTable[TaskHandle].FirstTimerEntry + pMsg->data32) & TIMEOUT_OCCURRED )
      {
        if ( !(*(pf_TaskTable[TaskHandle].FirstTimerEntry + pMsg->data32) & PERIODIC_TIMER) )
        {
          os_DestroyTimer ( TaskHandle, (OS_HANDLE)(*(pf_TaskTable[TaskHandle].FirstTimerEntry + pMsg->data32) & TIMER_HANDLE_MASK) );
          *(pf_TaskTable[TaskHandle].FirstTimerEntry + pMsg->data32) = 0;
        }
        else
        {
          *(pf_TaskTable[TaskHandle].FirstTimerEntry + pMsg->data32) &= ~TIMEOUT_OCCURRED;
        }
        vsi_o_ttrace ( TaskHandle, TC_TIMER, "Timeout   : Index %d",pMsg->data32) ;
        if ( pf_TaskTable[TaskHandle].PeiTable->pei_timeout != NULL )
          pf_TaskTable[TaskHandle].PeiTable->pei_timeout ( (USHORT)pMsg->data32 ); 
      }
    break;
    default:
      VSI_PPM_FREE(pMsg->ptr);
      os_DeallocatePartition (TaskHandle, pMsg->ptr-PPM_OFFSET );
      return PF_ERROR;
    /*lint -e527, suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
  }
  return PF_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)           MODULE  : FRAME                 |
| STATE   : code                     ROUTINE : pf_ProcessProtocolPrim|
+--------------------------------------------------------------------+
*/
LOCAL void pf_ProcessProtocolPrim ( T_HANDLE TaskHandle, T_VOID_STRUCT *pPrim)
{

  if ( TaskHandle != NO_TASK )
  {
    if (pf_TaskTable[TaskHandle].PeiTable->pei_primitive != NULL)
    {
#ifdef PRIM_AUTO_FREE
      if ( pf_TaskTable[TaskHandle].Flags & PARTITION_AUTO_FREE )
      {
        /* 
         * TST uses its own partition pool and is handled differently 
         */
        if ( TaskHandle != TST_Handle )
        {
          processed_prim[TaskHandle] = pPrim;
          freed_prim[TaskHandle] = NULL;
        }
      }
#endif /* PRIM_AUTO_FREE */
#ifdef MEMORY_SUPERVISION
      if ( TaskHandle != TST_Handle ) /* Trace pools are not monitored by PPM */ 
        VSI_PPM_RCV(pPrim);
#endif
      pf_TaskTable[TaskHandle].PeiTable->pei_primitive (pPrim);

#ifdef PRIM_AUTO_FREE
      if ( pf_TaskTable[TaskHandle].Flags & PARTITION_AUTO_FREE )
      {
        /*
         * if PSTORE was called during the primitive processing, PFREE was no longer
         * blocked inside the entity and it could have happened that a primitive  
         * was freed and then newly allocated either by the same or by a different
         * entity (IRQ). To avoid auto free it is checked if an effective free was
         * done with the pointer passed to pei_primitive(). In this case the 
         * partition is not freed.
         */
        if ( freed_prim[TaskHandle] == pPrim )
        {
          freed_prim[TaskHandle] = NULL;
          return;
        }
        else
        {
          processed_prim[TaskHandle] = NULL;

          if ( pPrim != NULL ) 
          {
            FREE ( P2D(pPrim) );
          }
          return;
        }
      }
      else
#endif /* PRIM_AUTO_FREE */
        return;
    }
  }
  if ( pPrim != NULL)
  {
#ifndef _TARGET_
    vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "Primitive discarded in dummy entity %s, opc 0x%x",
                   pf_TaskTable[TaskHandle].Name, ((T_PRIM_HEADER*)pPrim)->opc );
#endif
    VSI_PPM_RCV(pPrim);
    FREE ( P2D(pPrim) );
  }
}
#endif
 
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)           MODULE  : FRAME                 |
| STATE   : code                     ROUTINE : pf_ProcessSystemPrim  |
+--------------------------------------------------------------------+
*/ 
GLOBAL void pf_ProcessSystemPrim ( T_HANDLE TaskHandle, T_VOID_STRUCT *pPrim)
{
char * data;
/*lint -e813, suppress Info 813: auto variable 'token' has size '132' -> uncritical in this context */
char token[TRACE_TEXT_SIZE];
/*lint +e813 */
BOOL Error = FALSE;
LONG i;
unsigned int Length;
T_HANDLE min, max;
char TraceMaskBuffer[9];
ULONG trace_mask;
LONG state;
#ifdef _TOOLS_
T_S_HEADER *s_hdr;
#endif

  VSI_PPM_RCV(pPrim);

  data = (char *)P2D( pPrim );

  Length = GetNextToken (data, token, " #");

  if (!strcmp (token, SYSPRIM_REDIRECT_TOKEN)
   OR !strcmp (token, SYSPRIM_DUPLICATE_TOKEN))
  {

    if (TaskHandle NEQ NO_TASK)
    {
      /*
       * Redirect or Duplicate primitives
       */
      if ( (state = rt_RoutingModify (TaskHandle, token, data+strlen(token)+1)) != RT_OK )
      {
#ifdef NU_DEBUG
        switch ( state )
        {
          case RT_NO_MEM:
            vsi_o_ttrace(NO_TASK, TC_SYSTEM, "SYSTEM WARNING: Out of Memory - routing command rejected");
            VSI_PPM_FREE(pPrim);
            os_DeallocatePartition (TaskHandle, pPrim-PPM_OFFSET );
            return;
            /*lint -e527, suppress Warning -- Unreachable */
            break;
            /*lint +e527 */
		      case RT_ERROR: Error = TRUE;
            break;
        }
#else
        Error = TRUE;
#endif
      }
    }
  }
  else if ( !strcmp ( token, SYSPRIM_TRACECLASS_TOKEN) )
  {
    if ( Length < strlen(data)-1 )
    {
      GetNextToken (data+Length+1, token, " #");
      trace_mask = ASCIIToHex(token, CHARS_FOR_32BIT);
      if ( TaskHandle == TST_Handle )
      {
        min = 1;
        max = MaxEntities;
      }
      else
      {
        min = TaskHandle;
        max = TaskHandle;
      }
      for ( i = min; i <= max; i++ )
      {
        if ( vsi_settracemask ( TaskHandle, i, trace_mask) == VSI_ERROR )
          break;
      }
    }
    else
    {
      vsi_gettracemask ( TaskHandle, TaskHandle, &trace_mask);
      HexToASCII ( trace_mask, TraceMaskBuffer, CHARS_FOR_32BIT );
      TraceMaskBuffer[8] = 0;
      sprintf ( data, "%s %s %s", pf_TaskTable[TaskHandle].Name, SYSPRIM_TRACECLASS_TOKEN, TraceMaskBuffer );
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, data );
    }
  }
  else if (!strcmp (token, SYSPRIM_CONFIG_TOKEN))
  {
    /*
     * Dynamic Configuration
     */
    if (TaskHandle != NO_TASK)
    {
      /*
       * call the pei_config function of the entity
       */
      if (pf_TaskTable[TaskHandle].PeiTable->pei_config NEQ NULL)
        if ( pf_TaskTable[TaskHandle].PeiTable->pei_config ( data+strlen (token)+1 ) == PEI_ERROR )
          Error = TRUE;
    }
  }
#if 0
  /* not needed -> temporarily removed */
  else if (!strcmp (token, SYSPRIM_ISOLATE_TOKEN))
  {
    if ( rt_isolate_entity (TaskHandle, data+strlen(token)+1) == RT_ERROR )
      Error = TRUE;
  }
#endif
  else if (!strcmp (token, SYSPRIM_REGISTER_ERR_IND))
  {
    strncpy ( error_ind_dst, data+strlen(token)+1, RESOURCE_NAMELEN );
    error_ind_dst[RESOURCE_NAMELEN-1] = 0;
    if ( frm_error_ind == NULL )
    {
      frm_error_ind = (T_FRM_ERROR_IND*)vsi_c_pnew (sizeof(T_FRM_ERROR_IND), FRM_ERROR_IND FILE_LINE_MACRO );
    }
  }
  else if (!strcmp (token, SYSPRIM_WITHDRAW_ERR_IND))
  {
    if ( frm_error_ind != NULL )
    {
      error_ind_dst[0] = 0;
      vsi_c_pfree( (T_VOID_STRUCT**)&frm_error_ind FILE_LINE_MACRO );
      frm_error_ind = NULL;
    }
  }
  else if (!strcmp (token, SYSPRIM_STATUS_TOKEN))
  {
    GetNextToken (data+strlen(token)+1, token, " #");
    VSI_PPM_FREE(pPrim);
    os_DeallocatePartition (TaskHandle, pPrim-PPM_OFFSET );
    i = 0;
    while ( Resource[i].Name && strcmp ( Resource[i].Name, token ) )
      i++;
    if ( vsi_object_info (TaskHandle, Resource[i].Id, FIRST_ENTRY, token, sizeof(token)) == VSI_OK )       
    {
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      while ( vsi_object_info (TaskHandle, Resource[i].Id, NEXT_ENTRY, token, sizeof(token)) == VSI_OK )
      {
        vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      }
    }
    return;
  }
#if 0
  else if (!strcmp (token, SYSPRIM_READ_COM_MATRIX))
  {
    state = vsi_c_get_com_matrix_entry ( FIRST_ENTRY, token );
    if ( state == VSI_OK )
    {
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      do
      {
        if ( (state = vsi_c_get_com_matrix_entry ( NEXT_ENTRY, token)) == VSI_OK )
          vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      } while ( state == VSI_OK );
    }
  }
#endif
#ifdef _TARGET_
  else if (!strcmp (token, SYSPRIM_READ_FFS_DAR))
  {
    state = os_read_dar_ffs_data ( FIRST_ENTRY, token, TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER) - 1 );
    if ( state == OS_OK )
    {
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      do
      {
        if ( (state = os_read_dar_ffs_data ( NEXT_ENTRY, token, TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER) - 1)) == OS_OK )
          vsi_o_ttrace ( NO_TASK, TC_SYSTEM, token ); 
      } while ( state == OS_OK );
    }
    else
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "No DAR entry stored" ); 

  }
#endif
#if !defined _TARGET_ && !defined _TOOLS_
  else if (!strcmp (token, SYSPRIM_PCHECK))
  {
    pcheck_active[TaskHandle] = TRUE;
  }
#endif
  else if (!strcmp (token, SYSPRIM_ROUTE_DESCLIST))
  {
    route_desclist[TaskHandle] = TRUE;
  }
  else if (!strcmp (token, SYSPRIM_CHECK_DESCLIST))
  {
    check_desclist = TRUE;
  }
#ifdef _NUCLEUS_
  else if (!strcmp (token, SYSPRIM_VERSION_TOKEN))
  {
    vsi_o_ttrace (NO_TASK, TC_SYSTEM,"FRAME VERSION: %s, %s",frame_version_date, frame_version_time);
    vsi_o_ttrace (NO_TASK, TC_SYSTEM,"MISC VERSION:  %s, %s",misc_version_date,  misc_version_time);
    vsi_o_ttrace (NO_TASK, TC_SYSTEM,"TIF VERSION:   %s, %s",tif_version_date,   tif_version_time);
  }
#endif
  else if (!strcmp (token, SYSPRIM_RESET_TOKEN))
  {
    /*
     * Reset
     */
    pf_Reset (TaskHandle);
  }
#ifdef _TOOLS_
  else if (!strcmp (token, SYSPRIM_EXIT_TOKEN))
  {
    /*
     * Exit and delete
     */
    if (pf_TaskTable[TaskHandle].PeiTable->pei_exit != NULL)
    {
      pf_TaskTable[TaskHandle].PeiTable->pei_exit();
    }
    VSI_PPM_FREE(pPrim);
    os_DeallocatePartition (TaskHandle, pPrim-PPM_OFFSET );
    vsi_p_delete(0,TaskHandle);
  }
#endif /* _TOOLS_ */
#ifdef MEMORY_SUPERVISION
  else if (!strcmp (token, SYSPRIM_CHECK_OWNER))
  {
    ppm_check_partition_owner = 1;
  }
  else if (!strcmp (token, SYSPRIM_SHOW_MEMORY))
  {
    /*
     * Show state of the partition pool monitor
     */
    VSI_PPM_FREE(pPrim);
    os_DeallocatePartition (TaskHandle, pPrim-PPM_OFFSET );
    TracePoolstatus (TaskHandle);
    return;
  }
#endif /* MEMORY_SUPERVISION */    
#ifndef _TOOLS_  
  else if (!strcmp (token, SYSPRIM_SELECT_TIME_TDMA))
  {
    time_is_tdma_frame = 1;
  }
#endif
#ifdef _TOOLS_
  else if (!strcmp (token, SYSPRIM_IS_STACK_TIME))
  {
    s_hdr = P_SHDR(pPrim);
    set_stack_time (s_hdr->time);
  }
  else if (!strcmp (token, SYSPRIM_REGISTER_TOKEN))
  {
    OS_HANDLE comhandle;
    GetNextToken (data+strlen(token)+1, token, " #");
    if (os_create_extq (token, &comhandle) == OS_OK)
    {
      if (!strcmp (token, FRM_PCO_NAME))
      {
        vsi_o_set_htrace (comhandle);
      }
    }
    else
      Error = TRUE;
  }
  else if (!strcmp (token, SYSPRIM_WITHDRAW_TOKEN))
  {
    GetNextToken (data+strlen(token)+1, token, " #");
    if (os_destroy_extq (token) == OS_OK)
    {
      if (!strcmp (token, FRM_PCO_NAME))
      {
        vsi_o_set_htrace (0);
      }
    }
    else
      Error = TRUE;
  }
#endif /* _TOOLS_ */
  else
    Error = TRUE;

  if ( Error )
  {
    vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "SYSTEM WARNING:Invalid system primitive '%s'",data );
  }
  else
  {
    if ( !SuppressOK )
    {
      vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "OK (%s %s)", pf_TaskTable[TaskHandle].Name, data );
    }
  }
  VSI_PPM_FREE(pPrim);
  os_DeallocatePartition (TaskHandle, pPrim-PPM_OFFSET );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-GPF (8415)             MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_Reset            |
+--------------------------------------------------------------------+
*/
LOCAL void pf_Reset (T_HANDLE TaskHandle)
{
  if (TaskHandle != NO_TASK)
  {
    if (pf_TaskTable[TaskHandle].PeiTable->pei_exit != NULL)
      pf_TaskTable[TaskHandle].PeiTable->pei_exit();
    /* 
     * reset a single entity
     */
    if (pf_TaskTable[TaskHandle].PeiTable->pei_init != NULL)
      while (pf_TaskTable[TaskHandle].PeiTable->pei_init(TaskHandle) == PEI_ERROR)
        os_SuspendTask ( TaskHandle, 5);
  }
  else
  {
    USHORT Handle;
    /*
     * reset all entities
     */
    for (Handle = 1; Handle <= MaxEntities; Handle++)
    {
      if (pf_TaskTable[TaskHandle].PeiTable->pei_exit != NULL)
        pf_TaskTable[TaskHandle].PeiTable->pei_exit();

      if (pf_TaskTable[Handle].PeiTable->pei_init != NULL)
        while (pf_TaskTable[TaskHandle].PeiTable->pei_init(TaskHandle) == PEI_ERROR)
          os_SuspendTask ( TaskHandle, 5);
    }
  }
}
#endif

#ifdef _TOOLS_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : FRAME               |
| STATE   : code                       ROUTINE : set_stack_time      |
+--------------------------------------------------------------------+

  PURPOSE : stores the local time of stack under test. Used to align 
            timestamps while tracing.

*/
void set_stack_time ( ULONG time )
{
  init_stack_time = time;
  os_GetTime ( NO_TASK, &init_local_time );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : FRAME               |
| STATE   : code                       ROUTINE : get_stack_time      |
+--------------------------------------------------------------------+

  PURPOSE : returns the local time of stack under test. Used to align 
            timestamps while tracing.

*/
void get_local_time ( ULONG *time )
{
OS_TIME current_local_time;

 os_GetTime ( NO_TASK, &current_local_time );
 *time = init_stack_time + current_local_time - init_local_time;
}

#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : FRAME               |
| STATE   : code                       ROUTINE : pf_handle_error     |
+--------------------------------------------------------------------+

  PURPOSE : frame error and warning handling

*/
/*lint -e506, suppress Warning 506: Constant value Boolean */
/*lint -e718, suppress nfo 718: Symbol '_va_argref' undeclared, assumed to return int */
int pf_handle_warning ( USHORT cause, const char * const format,...)
{
T_HANDLE caller;
va_list varpars;
static int recursion_count = 0;
USHORT free;
USHORT alloc;

  /* recursion counter is needed to avoid endless loop when generating warning during processing of warning */
  if ( recursion_count == 0 )
  {
    recursion_count++;
    caller = e_running[os_MyHandle()];
    va_start (varpars, format);              
    int_vsi_o_ttrace ( NO_TASK, TC_SYSTEM, format, varpars );

    if ( error_ind_dst[0] != 0 )
    {
      /* drop the warning, if there are fewer than 3 partitions available to avoid deadlock */
      vsi_m_status ( caller, sizeof(T_FRM_WARNING_IND)+sizeof(T_PRIM_HEADER), PrimGroupHandle, &free, &alloc );
      if ( free >= 3 )
      {
        PALLOC(frm_warning_ind,FRM_WARNING_IND);
        frm_warning_ind->warning_code = cause;
#ifdef _TARGET_
        /* We will destroy the partition guard pattern if the warning is longer than the warning string in FRM_WARNING_IND */
        vsprintf ((char*)frm_warning_ind->warning_string, format, varpars);
#else
#if defined(_LINUX_) || defined(_SOLARIS_)
        vsnprintf ((char*)frm_warning_ind->warning_string,
                     sizeof(frm_warning_ind->warning_string), format, varpars);
#else
        _vsnprintf ((char*)frm_warning_ind->warning_string, sizeof(frm_warning_ind->warning_string), format, varpars);
#endif
#endif
        vsi_o_primsend ( caller, TC_SYSTEM, 0, error_ind_dst, (unsigned int)FRM_WARNING_IND, frm_warning_ind, sizeof(T_FRM_WARNING_IND) FILE_LINE_MACRO );
      }
      else
      {
        vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "FRM_WARNING_IND dropped" );
      }
    }
#if defined _NUCLEUS_ && !defined _TARGET_
    vsprintf (TraceBuffer, format, varpars);
    printf ("%s\n",TraceBuffer);
#endif
#ifdef _NUCLEUS_
    os_SystemError ( os_MyHandle(), cause, NULL );
#endif
    recursion_count--;
  }
  return VSI_OK;
}
/*lint +e718 */
/*lint +e506 */

#endif



/* 
+------------------------------------------------------------------------------
|  File:       vsi_mem.c
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
|             for the dynamic memory pools.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_MEM_C__
#define __VSI_MEM_C__
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

#ifdef NU_DEBUG
 #include "frame.h"
#endif

#ifdef MEMORY_SUPERVISION
 #include "tools.h"
#endif

/*==== CONSTANTS ==================================================*/

#ifndef RUN_INT_RAM
char const *waited_str = "Waited for partition";
char const *bigger_str = "Bigger partition allocated than requested";
char const *free_str   = "Partition Deallocation failed";
#else
extern char const *waited_str;
extern char const *bigger_str;
extern char const *free_str;
#endif

#ifdef NU_DEBUG
extern char const *freed_str;
#endif
 
/*==== TYPES ======================================================*/


/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

#if !defined (_TOOLS_) && !defined (_LINUX_) && !defined (_SOLARIS_)
extern const T_FRM_PARTITION_GROUP_CONFIG partition_grp_config[];
#endif

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
 char init_partition_memory = DISABLE_PARTITON_INIT;
 char init_partition_pattern = 0;
 T_HANDLE vsi_m_sem_handle = VSI_ERROR;
#else
 extern char init_partition_memory;
 extern char init_partition_pattern;
 extern T_HANDLE vsi_m_sem_handle;
#endif
	        
#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_new           |
+--------------------------------------------------------------------+

  PURPOSE : allocate a partition of a pool defined by the parameter type

*/

T_VOID_STRUCT * vsi_m_new (ULONG Size, ULONG type FILE_LINE_TYPE)
{
LONG Status;
T_VOID_STRUCT *prim;
OS_HANDLE pool;
T_HANDLE Caller;
ULONG flags;
ULONG suspend;

  Caller = 0;

  pool  = type & VSI_MEM_POOL_MASK;
  flags = type & VSI_MEM_FLAG_MASK;

  if ( flags & VSI_MEM_NON_BLOCKING )
    suspend = OS_NO_SUSPEND;
  else
    suspend = OS_SUSPEND;

  Status = os_AllocatePartition ( Caller, &prim, Size, suspend, pool );

  switch ( Status )
  {
    case OS_OK:
    break;
    case OS_WAITED:
#ifdef NU_DEBUG
      Caller = e_running[os_MyHandle()];
      pf_handle_warning ( OS_SYST_WRN_WAIT_PARTITION, "%s %s, entity %s, Size %d, %s(%d)", 
                          syst_wrn, waited_str, pf_TaskTable[Caller].Name, Size FILE_LINE_MACRO_PASSED );
#endif
    break;
    case OS_ERROR:
    case OS_TIMEOUT:
      if ( !(flags & VSI_MEM_NON_BLOCKING) )
      {
        /* fatal error for blocking allocation and 'HISR' caller */
        Caller = e_running[os_MyHandle()];
        vsi_o_assert ( NO_TASK, OS_SYST_ERR_NO_PARTITION FILE_LINE_MACRO_PASSED,
                       "No Partition available, entity %s, size %d",
                       pf_TaskTable[Caller].Name, Size );
      }
      return NULL;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
    case OS_ALLOCATED_BIGGER:
#ifdef NU_DEBUG
      Caller = e_running[os_MyHandle()];
      pf_handle_warning ( OS_SYST_WRN_BIG_PARTITION, "%s %s, entity %s, Size %d, %s(%d)", 
                          syst_wrn, bigger_str, pf_TaskTable[Caller].Name, Size FILE_LINE_MACRO_PASSED );
#endif
    break;
    default:
      return NULL;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
  }

  prim = prim + PPM_OFFSET;

  if ( init_partition_memory )
  {
    memset( (char*)prim, init_partition_pattern, (unsigned int)Size );
  }
#ifdef MEMORY_SUPERVISION
  /*
   * Pools registered via vsi_m_register_pool() cannot be handle by the partition supervision. The
   * id therefor is set to 0, to allow the supervision functions to ignore them.
   */
  if ( pool == DmemGroupHandle )
  {
    /* for primitive vsi_ppm_new() is called after the opc has been entered in the header */
    Caller = e_running[os_MyHandle()];
    vsi_ppm_new ( Caller, Size, (T_PRIM_HEADER*)prim, file, line );
  }
#endif /* MEMORY_SUPERVISION */

  return prim;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_new_size      |
+--------------------------------------------------------------------+

  PURPOSE : allocate partition and retern partition size

*/
T_VOID_STRUCT *vsi_m_new_size ( ULONG size, ULONG type,
                                ULONG *partition_size FILE_LINE_TYPE)
{
T_FRM_PARTITION_POOL_CONFIG * pool_config;
T_VOID_STRUCT *prim;

  if ( ( prim = vsi_m_new ( size, type FILE_LINE ) ) != NULL )
  {
#if defined (_TOOLS_) || defined (_LINUX_) || defined (_SOLARIS_)
    *partition_size = size;
#else
    pool_config = (T_FRM_PARTITION_POOL_CONFIG*)partition_grp_config[PrimGroupHandle].grp_config;
    while ( pool_config != NULL )
    {
      if ( size <= pool_config->part_size )
      {
        *partition_size = pool_config->part_size;
        break;
      }
      else
      {
        pool_config++;
      }
    }
#endif
  }
  return (prim);
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_free          |
+--------------------------------------------------------------------+

  PURPOSE : deallocate a partition

*/

int vsi_m_free (T_VOID_STRUCT **Msg FILE_LINE_TYPE)
{
T_HANDLE Caller;
#ifdef MEMORY_SUPERVISION
  Caller = e_running[os_MyHandle()];
  vsi_ppm_free ( Caller, (T_PRIM_HEADER*)*Msg, file, line );
#endif

  Caller = 0;
  if ( os_DeallocatePartition ( Caller, *Msg - PPM_OFFSET ) != OS_ERROR )
  {
    *Msg = NULL;
    return VSI_OK;
  }
#ifdef NU_DEBUG
  Caller = e_running[os_MyHandle()];
  pf_handle_warning ( OS_SYST_WRN_FREE_FAILED, "%s %s in %s, %s(%d)", 
                      syst_wrn, free_str, pf_TaskTable[Caller].Name FILE_LINE_MACRO_PASSED );
#endif
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_cnew          |
+--------------------------------------------------------------------+

  PURPOSE : allocate a memory with reference counter

*/

T_VOID_STRUCT * vsi_m_cnew (ULONG size, ULONG type FILE_LINE_TYPE)
{
T_M_HEADER *mem;
#ifdef MEMORY_SUPERVISION
T_HANDLE caller;
#endif

  if ( (mem = (T_M_HEADER*)vsi_m_new ( size+sizeof(T_M_HEADER), type FILE_LINE )) != NULL )
  {
    /* set reference counter */
    mem->ref_cnt = 1;
    /* set descriptor type */
    mem->desc_type = (SHORT)((type & VSI_MEM_DESC_MASK) >> 16);
    /* return pointer to user data */
#ifdef MEMORY_SUPERVISION
    caller = e_running[os_MyHandle()];
    vsi_ppm_new ( caller, size+sizeof(T_M_HEADER), (T_PRIM_HEADER*)mem, file, line );
#endif
    return (T_VOID_STRUCT*)(mem + 1);
  }
  else
  {
    return NULL;
  }

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_cfree         |
+--------------------------------------------------------------------+

  PURPOSE : allocate a memory with reference counter

*/

int vsi_m_cfree (T_VOID_STRUCT **ptr FILE_LINE_TYPE)
{
T_HANDLE caller = 0;
LONG sts;
T_M_HEADER *mem;

  /* get pointer to start of partition. Here is the reference counter */
  mem = (T_M_HEADER*)*ptr - 1;

#ifdef NU_DEBUG
  if ( os_is_valid_partition ((T_VOID_STRUCT*)mem) )
  {
    /* free to non-partition memory */
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "MFREE to non-partition memory, entity %s, ptr 0x%x", pf_TaskTable[caller].Name, *ptr );
    return VSI_ERROR;
  }
#endif

  sts = os_ObtainSemaphore (caller, vsi_m_sem_handle, OS_SUSPEND);
  if ( sts == OS_ERROR || sts == OS_TIMEOUT )
  {
    /* Semaphore invalid or overrun */
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "Ref Cnt Semaphore overrun, entity %s", pf_TaskTable[caller].Name );
    return VSI_ERROR;
  }
  if ( mem->ref_cnt <= 0 )
  {
#ifdef NU_DEBUG
    /* partition already freed */
    caller = e_running[os_MyHandle()];
    pf_handle_warning ( OS_SYST_WRN_MULTIPLE_FREE, "%s %s in %s, %s(%d)", 
                        syst_wrn, freed_str, pf_TaskTable[caller].Name FILE_LINE_MACRO_PASSED );
#endif
    os_ReleaseSemaphore (caller, vsi_m_sem_handle);
    return VSI_OK;
  }
  if ( --(mem->ref_cnt) == 0 )
  {
#ifdef _NUCLEUS_
#ifdef NU_DEBUG

    if ( os_PartitionCheck( (ULONG*)mem ) == OS_PARTITION_GUARD_PATTERN_DESTROYED )
    {
      caller = e_running[os_MyHandle()];
      os_ReleaseSemaphore (caller, vsi_m_sem_handle);
      vsi_o_assert ( caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                     "Partition Guard Pattern destroyed (MFREE),Task %s,Partition 0x%x",
                     pf_TaskTable[caller].Name, mem );
      return VSI_ERROR;
    }
#endif
#endif
    if (vsi_m_free ( (T_VOID_STRUCT**)&mem FILE_LINE ) != VSI_OK)
    {
      os_ReleaseSemaphore (caller, vsi_m_sem_handle);
      return VSI_ERROR;
    }

    *ptr=NULL;
  }
  os_ReleaseSemaphore (caller, vsi_m_sem_handle);
  return VSI_OK;

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_cfree         |
+--------------------------------------------------------------------+

  PURPOSE : allocate a memory with reference counter

*/

int vsi_m_attach (T_VOID_STRUCT *ptr FILE_LINE_TYPE)
{
T_HANDLE caller = 0;
LONG sts;
T_M_HEADER *mem;

  /* get pointer to start of partition. Here is the reference counter */
  mem = (T_M_HEADER*)ptr - 1;

#ifdef NU_DEBUG
  if ( os_is_valid_partition ((T_VOID_STRUCT*)mem) )
  {
    /* attach to non-partition memory */
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "MATTACH to non-partition memory, entity %s, ptr 0x%x", pf_TaskTable[caller].Name, ptr );
  }
#endif
  sts = os_ObtainSemaphore (caller, vsi_m_sem_handle, OS_SUSPEND);
  if ( sts == OS_ERROR || sts == OS_TIMEOUT )
  {
    /* Semaphore invalid or overrun */
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "Ref Cnt Semaphore overrun, entity %s", pf_TaskTable[caller].Name );
  }
  if ( mem->ref_cnt <= 0 )
  {
    /* attach to non allocated memory */
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "MATTACH to free memory, entity %s, ptr 0x%x", pf_TaskTable[caller].Name, ptr );
  }
  else
  {
    /* increment reference counter */
    mem->ref_cnt++;
  }
  os_ReleaseSemaphore (caller, vsi_m_sem_handle);
  return VSI_OK;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_status        |
+--------------------------------------------------------------------+

  PURPOSE : retrieve number of used/available partitions

*/

GLOBAL int vsi_m_status ( T_HANDLE caller, ULONG size, USHORT type, USHORT *free, USHORT *alloc )
{
#ifdef _NUCLEUS_
OS_HANDLE pool;

  pool = type & VSI_MEM_POOL_MASK;

  if ( os_GetPartitionPoolStatus ( size, pool, free, alloc ) == OS_OK )
    return VSI_OK;
  else
#endif
    return VSI_ERROR;
}
#endif

#ifndef _TOOLS_
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_register_pool |
+--------------------------------------------------------------------+

  PURPOSE : register a new partition pool group to be accessable via VSI

*/

GLOBAL int vsi_m_register_pool ( char * name, T_HANDLE * pool_gr_id )
{
OS_HANDLE pool_gr;
  
  if ( os_GetPartitionGroupHandle (OS_NOTASK, name, &pool_gr) == OS_OK )
  {
    *pool_gr_id = (T_HANDLE)pool_gr;
    return VSI_OK;
  }
  else
  {
    return VSI_ERROR;
  }
}
#endif
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_MEM             |
| STATE   : code                       ROUTINE : vsi_m_init          |
+--------------------------------------------------------------------+

  PURPOSE : retrieve number of used/available partitions

*/

GLOBAL int vsi_m_init ( char enable_init, char pattern )
{

  init_partition_memory  = enable_init;
  init_partition_pattern = pattern; 
  os_CreateSemaphore ( 0, (char*)"VSI_MSEM", 1, &vsi_m_sem_handle, 0 );

  return VSI_OK;
}
#endif

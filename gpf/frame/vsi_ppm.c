/* 
+------------------------------------------------------------------------------
|  File:       vsi_ppm.c
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
|             for the primitive partition pool supervision.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_PPM_C__
#define __VSI_PPM_C__
#endif

#include "gpfconf.h"

#ifdef MEMORY_SUPERVISION

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "tools.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"

/*==== TYPES ======================================================*/

typedef struct
{
  USHORT pool_nr;
  USHORT group_nr;
} T_POOL_GROUP;

typedef struct
{
  SHORT state_id;
  char const * state_name;
} T_PARTITION_STATE;
/*
 * indices to read the stored counters
 */
typedef enum { TOTAL,CURRENT_BYTE,CURRENT_PART,MAX_RANGES,MAX_BYTE_MEM,MAX_PART_MEM } T_COUNTER_READ;

/*
 * indices to update the counters
 */
typedef enum { DECREMENT, INCREMENT, STORE_MAX_BYTE, STORE_MAX_PART } T_COUNTER_UPDATE;

/*==== CONSTANTS ==================================================*/

#define OWNER_IS_COM_HANDLE           0x8000
/*
 * Partition States
 */
#define PARTITION_FREED             	0x0001
#define PARTITION_ALLOCATED         	0x0002
#define PARTITION_RECEIVED          	0x0004
#define PARTITION_SENT              	0x0008
#define PARTITION_REUSED           	  0x0010
#define PARTITION_ACCESSED           	0x0020
#define PARTITION_STORED           	  0x0040
#define MAX_PARTITION_STATE           7

#define ALLOWED_ALLOCATE_STATES       (PARTITION_FREED)
#define ALLOWED_RECEIVE_STATES        (PARTITION_SENT|PARTITION_RECEIVED)
#define ALLOWED_SEND_STATES           (PARTITION_ALLOCATED|PARTITION_REUSED|\
                                       PARTITION_RECEIVED|PARTITION_ACCESSED|\
                                       PARTITION_STORED)
#define ALLOWED_REUSE_STATES          (PARTITION_ALLOCATED|PARTITION_RECEIVED|\
                                       PARTITION_STORED|PARTITION_REUSED)
#define ALLOWED_DEALLOCATE_STATES     (PARTITION_RECEIVED|PARTITION_ALLOCATED|\
                                       PARTITION_SENT|PARTITION_REUSED|\
                                       PARTITION_ACCESSED|PARTITION_STORED)
#define ALLOWED_ACCESS_STATES         (PARTITION_ALLOCATED|PARTITION_RECEIVED|\
                                       PARTITION_REUSED|PARTITION_ACCESSED|\
                                       PARTITION_STORED) 
#define ALLOWED_STORE_STATES          (PARTITION_ALLOCATED|PARTITION_RECEIVED|\
                                       PARTITION_REUSED|PARTITION_ACCESSED|\
                                       PARTITION_SENT)

#define FORBIDDEN_ALLOCATE_STATES     (0xffff&~ALLOWED_ALLOCATE_STATES)
#define FORBIDDEN_RECEIVE_STATES      (0xffff&~ALLOWED_RECEIVE_STATES)
#define FORBIDDEN_SEND_STATES         (0xffff&~ALLOWED_SEND_STATES)
#define FORBIDDEN_REUSE_STATES        (0xffff&~ALLOWED_REUSE_STATES)
#define FORBIDDEN_DEALLOCATE_STATES   (0xffff&~ALLOWED_DEALLOCATE_STATES)
#define FORBIDDEN_ACCESS_STATES       (0xffff&~ALLOWED_ACCESS_STATES)
#define FORBIDDEN_STORE_STATES        (0xffff&~ALLOWED_STORE_STATES)

#define PPM_END_MARKER                ((char)0xff)

#define PARTITION_SIZE(g,p)           (partition_grp_config[g].grp_config[p].part_size)

#ifndef RUN_INT_RAM
const T_PARTITION_STATE partition_state[MAX_PARTITION_STATE+1] =
{
  { PARTITION_FREED,      "FREED"     },
  { PARTITION_ALLOCATED,  "ALLOCATED" },
  { PARTITION_RECEIVED,   "RECEIVED"  },
  { PARTITION_SENT,       "SENT"      },
  { PARTITION_REUSED,     "REUSED"    },
  { PARTITION_ACCESSED,   "ACCESSED"  },
  { PARTITION_STORED,     "STORED"    },
  { 0,                    NULL        }
};
#endif

/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED")
#endif

extern T_HANDLE TST_Handle;
extern const T_FRM_PARTITION_GROUP_CONFIG partition_grp_config[];
extern T_HANDLE * PoolGroupHandle [];
extern OS_HANDLE ext_data_pool_handle;
extern USHORT MaxPoolGroups;

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM

USHORT NumberOfPPMPartitions = 0;
USHORT NumOfPPMPools = 0;
USHORT NumOfPPMGroups;
USHORT NumOfPrimPools;
USHORT NumOfDmemPools;
T_PARTITION_POOL_STATUS PoolStatus;
T_PARTITION_STATUS *PartitionStatus;
T_OVERSIZE_STATUS  *PartitionOversize;
T_COUNTER	         *PartitionCounter;
T_POOL_GROUP       *PoolGroup;
#ifdef OPTIMIZE_POOL
T_COUNTER          *ByteCounter;
T_COUNTER          *RangeCounter;
int                *GroupStartRange;
int                *GroupStartCnt;
#endif /* OPTIMIZE_POOL */
int ppm_check_partition_owner;

#else /* RUN_INT_RAM */

extern int ppm_check_partition_owner;
extern T_PARTITION_POOL_STATUS  PoolStatus;
extern T_POOL_GROUP * PoolGroup;
extern USHORT NumOfPrimPools;
extern USHORT NumOfDmemPools;
extern int *GroupStartRange;

#endif /* RUN_INT_RAM */

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== FUNCTIONS ==================================================*/

GLOBAL void SetPartitionStatus ( T_PARTITION_STATUS *pPoolStatus, const char *file, int line, 
                                 ULONG opc, USHORT Status, T_HANDLE owner );

USHORT update_dyn_state ( T_HANDLE Caller, T_PRIM_HEADER *prim, T_HANDLE owner, USHORT state, const char* file, int line );
BOOL UpdatePoolCounter ( T_COUNTER *pCounter, T_COUNTER_UPDATE Status, ULONG Value );
void StoreRangeCounters ( T_PRIM_HEADER *prim, T_COUNTER_UPDATE Status );
int GetPartitionRange ( ULONG size, USHORT group_nr, USHORT pool_nr );
LONG get_partition_group ( T_PRIM_HEADER *prim, USHORT *group_nr, USHORT *pool_nr );
char const *get_partition_state_name ( USHORT partition_state );


#ifndef RUN_FLASH
USHORT update_dyn_state ( T_HANDLE Caller, T_PRIM_HEADER *prim, T_HANDLE owner, USHORT state, const char* file, int line )
{
T_desc *desc;
T_desc3 *desc3; 
T_M_HEADER *mem;                  
T_DP_HEADER *dp_hdr;
USHORT ret = TRUE;

  if ( prim->dph_offset != 0 )
  {
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
    if ( *((ULONG*)dp_hdr) == GUARD_PATTERN )
    {
      dp_hdr = (T_DP_HEADER*)dp_hdr->next;
      while (dp_hdr != NULL)                         
      {                                             
        SetPartitionStatus ( &PoolStatus.PartitionStatus [ P_PNR(dp_hdr) ], file, line, P_OPC(prim), state, owner );
        dp_hdr = (T_DP_HEADER*)dp_hdr->next;                           
      }                                             
    }
    else
    {
      if ( Caller != TST_Handle )
      {
        /* do not check and update the states of the primitives in descriptor lists when called by TST, because
           descriptor lists are not routed to TST and will result in the warning generated below */
        desc = (T_desc*)(((T_desc_list*)dp_hdr)->first);
        while (desc != NULL)                         
        {                                             
#ifdef _NUCLEUS_
          if ( *(((ULONG*)desc)-4) == 0 )
#endif
          {
            mem = ((T_M_HEADER*)desc)-1;
            SetPartitionStatus ( &PoolStatus.PartitionStatus [P_PNR(mem)], file, line, P_OPC(prim), state, owner );
            if ( mem->desc_type == (VSI_DESC_TYPE3 >> 16) )
            {
              desc3 = (T_desc3*)desc;
              mem = ((T_M_HEADER*)desc3->buffer)-1;
              SetPartitionStatus ( &PoolStatus.PartitionStatus [P_PNR(mem)], file, line, P_OPC(prim), state, owner );
            }
          }
#ifdef _NUCLEUS_
          else
            vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: FREED PARTITION 0x%lx IN DESCLIST, %s(%d)", prim,rm_path(file),line ); 
#endif
          desc = (T_desc *)desc->next;                           
        } 
      }
    }
  }
  else
    ret = FALSE;
  return ret;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM                 |
| STATE   : code                       ROUTINE : get_partition_state_name|
+------------------------------------------------------------------------+

  PURPOSE : update counter.

*/
char const *get_partition_state_name ( USHORT state )
{
USHORT i = 0;

  while ( partition_state[i].state_id )
  {
    if ( partition_state[i].state_id == state )
      return partition_state[i].state_name;
    i++;
  }
  return NULL;
}
#endif

#ifndef RUN_FLASH
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM               |
| STATE   : code                       ROUTINE : check_partition_group |
+----------------------------------------------------------------------+

  PURPOSE : update counter.

*/
LONG get_partition_group ( T_PRIM_HEADER *prim, USHORT *group_nr, USHORT *pool_nr )
{
SHORT invalid_pool = 0;
T_HANDLE Caller;

  *pool_nr =  PoolGroup [ (USHORT)(P_PGR(prim)) ].pool_nr;
  *group_nr = PoolGroup [ (USHORT)(P_PGR(prim)) ].group_nr;

  if ( *group_nr > MaxPoolGroups )
	  invalid_pool = 1;

  if ( *group_nr == PrimGroupHandle )
  {
     if ( *pool_nr > NumOfPrimPools )
       invalid_pool = 1;
  }
  else if ( *group_nr == DmemGroupHandle )
  {
     if ( *pool_nr > NumOfDmemPools )
       invalid_pool = 1;
  }

  if ( invalid_pool == 1 )
  {
    Caller = e_running[os_MyHandle()];
    vsi_o_ttrace (Caller, TC_SYSTEM,
            "[PPM]: Invalid Partition Pool, group: %d, pool: %d", *group_nr, *pool_nr );
    return VSI_ERROR;
  }

  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : GetPartitionRange   |
+--------------------------------------------------------------------+

  PURPOSE : update counter.

*/
int GetPartitionRange ( ULONG size, USHORT group_nr, USHORT pool_nr )
{
int partition_range;
int size_offset;
int range;

  if ( pool_nr != 0 )
  {
    partition_range = (int)(PARTITION_SIZE(group_nr,pool_nr) - PARTITION_SIZE(group_nr,pool_nr-1));

    size_offset = (int)(size - (USHORT)(PARTITION_SIZE(group_nr,pool_nr-1)) - 1);
    if ( size_offset < 0 )
      size_offset = 0;
  }
  else
  {
    partition_range = (USHORT)(PARTITION_SIZE(group_nr,pool_nr));
    if ( size == 0 )
      size_offset = 0;
    else
      size_offset = (int)(size - 1);
  }

  range = (USHORT)((size_offset * RANGES_PER_POOL)/partition_range + pool_nr * RANGES_PER_POOL + GroupStartRange[group_nr]);

  return range;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : UpdatePoolCounter   |
+--------------------------------------------------------------------+

  PURPOSE : update counter.

*/
BOOL UpdatePoolCounter ( T_COUNTER *pCounter, T_COUNTER_UPDATE Status, ULONG Value )
{

  switch ( Status )
  {
  case INCREMENT:
       pCounter->Total += Value;                                    /* total number */
       pCounter->Current += Value;                                  /* current number */
       if ( pCounter->Current > pCounter->Maximum )                 /* current > maximum ? */
       {
         pCounter->Maximum = pCounter->Current;                     /* Maximum = Current */
         return TRUE ;
       }
    break;
  case DECREMENT:
       pCounter->Current -= Value;                                  /* current number */
    break;
  case STORE_MAX_BYTE:
       pCounter->MaxByteMemory = pCounter->Current;                 /* store current number */
    break;
  case STORE_MAX_PART:
       pCounter->MaxPartMemory = pCounter->Current;                 /* store current number */
    break;
  }
  return FALSE;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : SetPartitionStatus  |
+--------------------------------------------------------------------+

  PURPOSE : update the status of the partition.

*/
GLOBAL void SetPartitionStatus ( T_PARTITION_STATUS *pPoolStatus, const char *file, int line, ULONG opc, USHORT Status, T_HANDLE owner )
{

    pPoolStatus->Status = Status;
    pPoolStatus->PrimOPC = opc;
    pPoolStatus->owner = owner;
    if ( Status NEQ PARTITION_FREED )
    {
       os_GetTime (0,&pPoolStatus->time);
       pPoolStatus->Userfile = file;
       pPoolStatus->Line = line;
    }
}
#endif

#ifdef OPTIMIZE_POOL
#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : StoreRangeCounters  |
+--------------------------------------------------------------------+

  PURPOSE : stores the range counters for a specified partition.

*/
void StoreRangeCounters ( T_PRIM_HEADER *prim, T_COUNTER_UPDATE Status )
{
USHORT i;

  for ( i=0; i<5; i++ )
    UpdatePoolCounter ( &PoolStatus.RangeCounter [ P_PGR(prim)*5+i ], Status,0 );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : TracePoolStatistic  |
+--------------------------------------------------------------------+

  PURPOSE : send the statistic information specified by the parameters
            to the tst interface.

*/
LOCAL void TracePoolStatistic ( T_HANDLE Caller, USHORT group_nr, USHORT pool_nr, char const *Src, 
                                T_COUNTER_READ Status )
{
#define RNG_COUNT_IDX(g,p,i)     (GroupStartRange[g]+p*RANGES_PER_POOL+i)
#define COUNT_IDX(g,p)           (GroupStartCnt[g]+p)
T_FRM_PARTITION_POOL_CONFIG * pool_config;
ULONG Value1;
ULONG Value2;
char const *Resource;
BOOL TraceRange = FALSE;
BOOL TraceValue = FALSE;
ULONG Value[5];
int i;

    pool_config = (T_FRM_PARTITION_POOL_CONFIG*)partition_grp_config[group_nr].grp_config + pool_nr;

    switch ( Status )
    {
    case TOTAL:
      TraceRange = TRUE;
      for ( i = 0; i < RANGES_PER_POOL; i++ )
        Value[i] = PoolStatus.RangeCounter [ RNG_COUNT_IDX(group_nr,pool_nr,i) ].Total;
      break;
    case CURRENT_BYTE:
      TraceValue = TRUE;
      Resource = "max bytes ";
      Value1 = PoolStatus.ByteCounter [ COUNT_IDX(group_nr,pool_nr) ].Current;
      Value2 = PoolStatus.PartitionCounter [COUNT_IDX(group_nr,pool_nr)].Current * pool_config->part_size;
      break;
    case CURRENT_PART:
      TraceValue = TRUE;
      Resource = "part";
      Value1 = PoolStatus.PartitionCounter [COUNT_IDX(group_nr,pool_nr)].Current;
      Value2 = pool_config->part_num;

      break;
    case MAX_RANGES:
      TraceRange = TRUE;
      for ( i = 0; i < RANGES_PER_POOL; i++ )
        Value[i] = PoolStatus.RangeCounter [ RNG_COUNT_IDX(group_nr,pool_nr,i) ].Maximum;
      break;
    case MAX_BYTE_MEM:
      TraceRange = TRUE;
      TraceValue = TRUE;
      Resource = "bytes     ";
      Value1 = PoolStatus.ByteCounter [COUNT_IDX(group_nr,pool_nr)].Maximum;
      Value2 = PoolStatus.PartitionCounter [COUNT_IDX(group_nr,pool_nr)].MaxByteMemory * pool_config->part_size;
      for ( i = 0; i < RANGES_PER_POOL; i++ )
        Value[i] = PoolStatus.RangeCounter [ RNG_COUNT_IDX(group_nr,pool_nr,i) ].MaxByteMemory;
      break;
    case MAX_PART_MEM:
      TraceRange = TRUE;
      TraceValue = TRUE;
      Resource = "partitions";
      Value1 = PoolStatus.PartitionCounter [COUNT_IDX(group_nr,pool_nr)].Maximum;
      Value2 = pool_config->part_num;
      for ( i = 0; i < RANGES_PER_POOL; i++ )
        Value[i] = PoolStatus.RangeCounter [ RNG_COUNT_IDX(group_nr,pool_nr,i) ].MaxPartMemory;
      break;
    default:
      break;
    }

    /*lint -e644, suppress Warning -- Variable may not have been initialized */
    if ( TraceValue )
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: %s pool %d:%5d %s =>%3d%%", 
      Src, pool_nr, Value1, Resource, (Value1*100)/(Value2==0?1:Value2) ); 

    if ( TraceRange )
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: %s partitions pool %d:   %3d, %3d, %3d, %3d, %3d",Src, pool_nr,
                 Value[0],Value[1],Value[2],Value[3],Value[4]);
    /*lint +e644 */

}
#endif
#endif  /* OPTIMIZE_POOL */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : TracePoolStatus     |
+--------------------------------------------------------------------+

  PURPOSE : send the statistic information to the test interface.

*/
GLOBAL void TracePoolstatus ( T_HANDLE Caller )
{
T_PARTITION_STATUS *pPoolStatus;
T_FRM_PARTITION_POOL_CONFIG * pool_config;
USHORT i, m;
USHORT group_nr, pool_nr;
USHORT PartitionError = FALSE, OversizeError = FALSE;
T_OVERSIZE_STATUS *pOversizeStatus;
T_HANDLE owner;
char *opc;

  pOversizeStatus = &PoolStatus.PartitionOversize[0];
  pPoolStatus = &PoolStatus.PartitionStatus [0];
  for ( i = 0; i < NumberOfPPMPartitions; i++ )
  {
    if ( pPoolStatus->Status > 0 && pPoolStatus->Status NEQ PARTITION_FREED )
    {
      if ( pPoolStatus->owner & OWNER_IS_COM_HANDLE )
        owner = pPoolStatus->owner&~OWNER_IS_COM_HANDLE;
      else
        owner = pPoolStatus->owner;

      if ( pPoolStatus->PrimOPC && ( ((T_PRIM_HEADER*)pPoolStatus->ptr)->opc != pPoolStatus->PrimOPC ) )
        opc = "in desclist of OPC";
      else
        opc = "OPC";
      get_partition_group ( pPoolStatus->ptr, &group_nr, &pool_nr );
      
      pool_config = (T_FRM_PARTITION_POOL_CONFIG*)partition_grp_config[group_nr].grp_config + pool_nr;
      
      vsi_o_ttrace ( Caller, TC_SYSTEM, "POOL%d%d(%s), PARTITION 0x%lx(%d), %s 0x%lx, \
%s, %s, TIME %d, %s(%d)", group_nr,pool_nr,partition_grp_config[group_nr].name, pPoolStatus->ptr,pool_config->part_size,
                  opc, pPoolStatus->PrimOPC, get_partition_state_name(pPoolStatus->Status), pf_TaskTable[owner].Name, pPoolStatus->time, 
                  rm_path(pPoolStatus->Userfile), pPoolStatus->Line);
      PartitionError = TRUE;
    }
    pPoolStatus++;
  }

  for (m = 0; partition_grp_config[m].grp_config != NULL; m++ )
  {
    if ( strcmp ("TEST", partition_grp_config[m].name ) )
    {
      vsi_o_ttrace ( Caller, TC_SYSTEM, "---------------------------------------------------------" );
      vsi_o_ttrace ( Caller, TC_SYSTEM, "[PPM]: POOL NAME: %s", partition_grp_config[m].name );
      pool_config = (T_FRM_PARTITION_POOL_CONFIG*)partition_grp_config[m].grp_config;
      for ( i = 0; pool_config != NULL; i++)
      {
        if ( pool_config->part_size )
        {
#ifdef OPTIMIZE_POOL
          vsi_o_ttrace ( Caller, TC_SYSTEM, "---------------------------------------------------------" );
          vsi_o_ttrace ( Caller, TC_SYSTEM, "[PPM]: POOL %d (size %d) ",i, pool_config->part_size );
          TracePoolStatistic ( Caller, m, i, "MAXBYTE ", MAX_BYTE_MEM );
          TracePoolStatistic ( Caller, m, i, "MAXPART ", MAX_PART_MEM );
          TracePoolStatistic ( Caller, m, i, "MAXRANGE", MAX_RANGES );
          TracePoolStatistic ( Caller, m, i, "TOTAL   ", TOTAL );
#endif /* OPTIMIZE_POOL */
          if ( pOversizeStatus->PrimOPC )
          {
            vsi_o_ttrace ( Caller, TC_SYSTEM, "PPM: PARTITION OF SIZE %d USED BY OVERSIZED \
    PRIMITIVE %lx AT %s(%d)", pool_config->part_size,
                     pOversizeStatus->PrimOPC, rm_path(pOversizeStatus->Userfile), pPoolStatus->Line);
            OversizeError = TRUE;
          }
          pOversizeStatus++;
        }
        else
        {
          break;
        }
        pool_config++;
      }
      if ( !PartitionError )
          vsi_o_ttrace ( Caller, TC_SYSTEM, "[PPM]: ALL PARTITIONS FREED" ); 
      if ( !OversizeError )
          vsi_o_ttrace ( Caller, TC_SYSTEM, "[PPM]: NO OVERSIZE ERRORS OCCURED" ); 
    }
  }

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : InitializePPM       |
+--------------------------------------------------------------------+

  PURPOSE : initialize supervision, write index to each partition.

*/
void InitializePPM ( void )
{
T_FRM_PARTITION_POOL_CONFIG * pool_config;
ULONG *Prims;
USHORT i,j,k,m,n;
int status;
static int last_range = 0;
static int last_cnt = 0;

  status =  os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&Prims,             sizeof(int)*NumberOfPPMPartitions, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&PoolGroup,         sizeof(T_POOL_GROUP)*NumOfPPMPools, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&PartitionStatus,   sizeof(T_PARTITION_STATUS)*NumberOfPPMPartitions, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&PartitionOversize, sizeof(T_OVERSIZE_STATUS)*NumOfPPMPools, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&PartitionCounter,  sizeof(T_COUNTER)*NumOfPPMPools, OS_NO_SUSPEND, ext_data_pool_handle );
#ifdef OPTIMIZE_POOL
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&ByteCounter,       sizeof(T_COUNTER)*NumOfPPMPools, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&RangeCounter,      sizeof(T_COUNTER)*NumberOfPPMPartitions, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&GroupStartRange,   sizeof(int)*NumOfPPMGroups, OS_NO_SUSPEND, ext_data_pool_handle );
  status |= os_AllocateMemory (NO_TASK, (T_VOID_STRUCT**)&GroupStartCnt,     sizeof(int)*NumOfPPMGroups, OS_NO_SUSPEND, ext_data_pool_handle );
#endif
  if ( status > 0 )
  {
    vsi_o_assert ( 0, OS_SYST_ERR, __FILE__, __LINE__, "Memory allocation for partition supervision failed" );
  }
  ppm_check_partition_owner = 0;
  PoolStatus.PartitionStatus = PartitionStatus;
  PoolStatus.PartitionOversize = PartitionOversize;
  PoolStatus.PartitionCounter = PartitionCounter;
#ifdef OPTIMIZE_POOL
  PoolStatus.ByteCounter = ByteCounter;
  PoolStatus.RangeCounter = RangeCounter;
#endif

  for ( j = 0; j<NumberOfPPMPartitions; j++)
    Prims[j] = 0;

  for (m = 0, j = 0, i = 0; partition_grp_config[m].grp_config != NULL; m++ )
  {
    if ( strcmp ("TEST", partition_grp_config[m].name ) )
    {
      pool_config = (T_FRM_PARTITION_POOL_CONFIG*)partition_grp_config[m].grp_config;

      for (n = 0; pool_config != NULL; i++, n++)
      {
        if ( pool_config->part_size )
        {
          PoolGroup[i].group_nr = m;
          PoolGroup[i].pool_nr = n;
          for (k = 0; k < pool_config->part_num; k++ , j++)
          {
            if ( os_AllocatePartition ( NO_TASK, (T_VOID_STRUCT**)&Prims[j], pool_config->part_size,
                                   OS_NO_SUSPEND, *PoolGroupHandle[m] ) == OS_OK )
            {
              P_IDX((T_PRIM_HEADER*)(Prims[j]+PPM_IDX_OFFSET)) = ( ((USHORT)i<<16) | j );
            }
            else
            {
              P_IDX((T_PRIM_HEADER*)(Prims[j]+PPM_IDX_OFFSET)) = 0;
            }
          }
        }
        else
        {
          break;
        }
        pool_config++;
      }
      if ( m == 0 )
      {
        GroupStartCnt[m] = 0;
        GroupStartRange[m] = 0;
		last_cnt   = n;
        last_range = RANGES_PER_POOL * n;
      }
      else
      {
        GroupStartCnt[m]   = last_cnt;
        GroupStartRange[m] = last_range;
        last_cnt   = GroupStartCnt[m] + n;
        last_range = GroupStartRange[m] + RANGES_PER_POOL * n;
      }

    }
  }
  for ( j = 0; j<NumberOfPPMPartitions; j++)
  {
    if ( Prims[j] )
    {
      os_DeallocatePartition ( NO_TASK, (T_VOID_STRUCT*)Prims[j] );
      PoolStatus.PartitionStatus [ P_PNR(Prims[j]+4) ].Status = PARTITION_FREED;
    }
  }
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_new         |
+--------------------------------------------------------------------+

  PURPOSE : supervision of allocating a partition.

*/
GLOBAL void vsi_ppm_new ( T_HANDLE Caller, ULONG Size, T_PRIM_HEADER *prim, const char* file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      vsi_ppm_setend(prim, Size);
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      pPoolStatus->ptr = prim;
      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_ALLOCATE_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_ALLOCATED),rm_path(file),line );

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), PARTITION_ALLOCATED, Caller  );

#ifdef OPTIMIZE_POOL
      /*
       * get primitive size and update range counter
       */
      pPoolStatus->UsedSize = Size;
      UpdatePoolCounter ( &PoolStatus.RangeCounter [GetPartitionRange(pPoolStatus->UsedSize,group_nr,pool_nr)], INCREMENT,1 );
#endif /* OPTIMIZE_POOL */

      /*
       * update partition counter, if new maximum and OPTIMIZE_POOL, then
       * - store the counters of the ranges within this partition
       * - send a message that a new maximum has occurred
       */
      if ( UpdatePoolCounter ( &PoolStatus.PartitionCounter [P_PGR(prim)],INCREMENT,1 ) )
#ifndef OPTIMIZE_POOL
        ;
#else
      {
        StoreRangeCounters ( prim, STORE_MAX_PART );
      }

      /*
       * update byte counter, if new maximum, then
       * - store the counters of the ranges within this partition
       * - store the number of currently allocated partitions
       */
      if ( UpdatePoolCounter ( &PoolStatus.ByteCounter [P_PGR(prim)],INCREMENT,pPoolStatus->UsedSize ) )
      {
        StoreRangeCounters ( prim, STORE_MAX_BYTE );
        UpdatePoolCounter ( &PoolStatus.PartitionCounter [ P_PGR(prim) ], STORE_MAX_BYTE,0 );
      }
#endif /* OPTIMIZE_POOL */
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_rec         |
+--------------------------------------------------------------------+

  PURPOSE : supervision of receiving a partition.

*/
GLOBAL void vsi_ppm_rec ( T_HANDLE Caller, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_RECEIVE_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_RECEIVED),rm_path(file),line );

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), PARTITION_RECEIVED, Caller );
      update_dyn_state ( Caller, prim, Caller, PARTITION_RECEIVED, file, line );
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_access      |
+--------------------------------------------------------------------+

  PURPOSE : supervision of receiving a partition.

*/
GLOBAL void vsi_ppm_access ( T_HANDLE Caller, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_ACCESS_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_ACCESSED),rm_path(file),line );

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), pPoolStatus->Status, Caller );
      update_dyn_state ( Caller, prim, Caller, PARTITION_ACCESSED, file, line );
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_send        |
+--------------------------------------------------------------------+

  PURPOSE : supervision of receiving a partition.

*/
GLOBAL void vsi_ppm_send ( T_HANDLE Caller, T_HANDLE rcv, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT NewStatus = PARTITION_SENT;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];


      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_SEND_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_SENT),rm_path(file),line );

      /*
       * check if more bytes written than requested during allocation
       */
      if ( *((char*)prim + pPoolStatus->RequestedSize - 1) != PPM_END_MARKER )
      {
        if ( prim->dph_offset == 0 )
          vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "SYSTEM WARNING: Bytes written > requested partition size, %s(%d)", rm_path(file), line );
      }
      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), NewStatus, (T_HANDLE)(OWNER_IS_COM_HANDLE|rcv) );
      update_dyn_state ( Caller, prim, rcv, PARTITION_SENT, file, line );
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_store       |
+--------------------------------------------------------------------+

  PURPOSE : supervision of storing a partition.

*/
GLOBAL void vsi_ppm_store ( T_HANDLE Caller, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_STORE_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_STORED),rm_path(file),line );

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), pPoolStatus->Status, Caller );
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_reuse       |
+--------------------------------------------------------------------+

  PURPOSE : supervision of reusing a partition.

*/
GLOBAL void vsi_ppm_reuse ( T_HANDLE Caller, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
ULONG OldSize, NewSize;
USHORT group_nr, pool_nr;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_REUSE_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
              "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
              get_partition_state_name(PARTITION_REUSED),rm_path(file),line );

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, P_OPC(prim), PARTITION_REUSED, Caller );
      update_dyn_state ( Caller, prim, Caller, PARTITION_REUSED, file, line );
      /*
       * if the new primitive exceeds the size of the partition, then
       * - store file, line and primitive opc
       * - send an error message
       */
#if 0
      if ( (ULONG)(P_LEN(prim)) > PoolGroupConfig[PrimGroupHandle]->PoolConfig[P_PGR(prim)].PartitionSize  )
      {
         PoolStatus.PartitionOversize [P_PGR(prim)].Userfile = file;
         PoolStatus.PartitionOversize [P_PGR(prim)].Line = line;
         PoolStatus.PartitionOversize [P_PGR(prim)].PrimOPC = P_OPC(prim);
         vsi_o_assert (NO_TASK, OS_SYST_ERR_OVERSIZE, file, line, "PREUSE - oversize error in %s",
                       pf_TaskTable[Caller].Name );
      }
#endif
#ifdef OPTIMIZE_POOL
      /*
       * if the old and new primitve have different sizes, then
       * - decrement byte counter by old size
       * - decrement old range counter
       * - increment new range counter
       * - increment byte counter by new size
       */
      if ( (OldSize=pPoolStatus->UsedSize) NEQ (NewSize=P_LEN(prim)) )
      {
        UpdatePoolCounter ( &PoolStatus.ByteCounter [P_PGR(prim)],DECREMENT,OldSize );
        UpdatePoolCounter ( &PoolStatus.RangeCounter [ GetPartitionRange(OldSize,group_nr,pool_nr) ], DECREMENT, 1 );
        UpdatePoolCounter ( &PoolStatus.RangeCounter [ GetPartitionRange(NewSize,group_nr,pool_nr) ], INCREMENT, 1 );
        pPoolStatus->UsedSize = NewSize;
        if ( UpdatePoolCounter ( &PoolStatus.ByteCounter [P_PGR(prim)],INCREMENT,NewSize ) )
        {
          StoreRangeCounters ( prim, STORE_MAX_BYTE );
          UpdatePoolCounter ( &PoolStatus.PartitionCounter [ P_PGR(prim) ], STORE_MAX_BYTE,0 );
        }
      }
#endif /* OPTIMIZE_POOL */
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_PPM             |
| STATE   : code                       ROUTINE : vsi_ppm_free        |
+--------------------------------------------------------------------+

  PURPOSE : supervision of deallocating a partition.

*/
GLOBAL void vsi_ppm_free ( T_HANDLE Caller, T_PRIM_HEADER *prim, const char *file, int line )
{
T_PARTITION_STATUS *pPoolStatus;
USHORT group_nr, pool_nr;
T_HANDLE owner;

  if ( prim != NULL )
  {
    if ( get_partition_group ( prim, &group_nr, &pool_nr ) == VSI_OK )   
    {
      Caller = e_running[os_MyHandle()];
      prim->opc = 0;
      /*
       * set pointer to status entry of the currently used partition
       */
      pPoolStatus = &PoolStatus.PartitionStatus [ P_PNR(prim) ];

      /*
       * send error message in case of an illegal state transition
       */
      if ( pPoolStatus->Status & FORBIDDEN_DEALLOCATE_STATES )
        vsi_o_ttrace (Caller, TC_SYSTEM,
                  "[PPM]: %s->%s: %s(%d)", get_partition_state_name(pPoolStatus->Status),
                  get_partition_state_name(PARTITION_FREED),file,line );


      /* CURRENTLY DISABLED FOR UMTS RELEASE */
      if ( pPoolStatus->owner & OWNER_IS_COM_HANDLE )
      {
        owner = pPoolStatus->owner&~OWNER_IS_COM_HANDLE;
        vsi_o_ttrace (NO_TASK, TC_SYSTEM,
                  "SYSTEM WARNING: %s freed partition stored in %s queue, %s(%d)",
                  pf_TaskTable[Caller].Name,pf_TaskTable[owner].Name,rm_path(file),line );
      }
      if ( ppm_check_partition_owner == 1 )
      {
        if ( (pPoolStatus->owner & ~OWNER_IS_COM_HANDLE) != Caller )
        {
          owner = pPoolStatus->owner&~OWNER_IS_COM_HANDLE;
          vsi_o_ttrace (NO_TASK, TC_SYSTEM,
                    "SYSTEM WARNING: %s freed partition belonging to %s, %s(%d)",
                    pf_TaskTable[Caller].Name,pf_TaskTable[owner].Name,rm_path(file),line );
        }
      }

      if ( !(pPoolStatus->Status & PARTITION_FREED) )
      {
#ifdef OPTIMIZE_POOL
        /*
         * decrement byte counter by primitive size
         * decrement range counter
         * decrement partition counter
         */
        UpdatePoolCounter ( &PoolStatus.ByteCounter [ P_PGR(prim) ], DECREMENT, pPoolStatus->UsedSize );
        UpdatePoolCounter ( &PoolStatus.RangeCounter [GetPartitionRange(pPoolStatus->UsedSize,group_nr,pool_nr)], DECREMENT, 1 ) ;
#endif /* OPTIMIZE_POOL */
        UpdatePoolCounter ( &PoolStatus.PartitionCounter [ P_PGR(prim) ], DECREMENT, 1 );
      }

      /*
       * update partition status
       */
      SetPartitionStatus ( pPoolStatus, file, line, 0, PARTITION_FREED, 0 );    
    }
    else
      vsi_o_ttrace (Caller, TC_SYSTEM,"[PPM]: Invalid Partition Pool, group: %d, pool: %d", group_nr, pool_nr );
  }
}
#endif

#ifndef RUN_FLASH
GLOBAL void vsi_ppm_setend ( T_PRIM_HEADER *prim, ULONG size )
{
  *((char*)prim + size ) = PPM_END_MARKER;
  PoolStatus.PartitionStatus[P_PNR(prim)].RequestedSize = size+1;
}
#endif
#endif /* MEMORY_SUPERVISION */

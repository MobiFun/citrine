/* 
+------------------------------------------------------------------------------
|  File:       frm_primitives.h
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
|  Purpose :  Definitions for frame primitives.
+----------------------------------------------------------------------------- 
*/ 

#ifndef FRM_PRIMITIVES_H
#define FRM_PRIMITIVES_H

#define FRM_ERROR_IND     0xC000001E  /* SAP NR: 30 (0x1e), PRIM NR 0 */
#define FRM_WARNING_IND   0xC001001E  /* SAP NR: 30 (0x1e), PRIM NR 1 */

/* maximum length of a string in frame primitives */
#define FRM_PRIM_STR_SIZE   100

/* spontaneuous frame output */
typedef struct
{
  U32   error_code;
  char  error_string [ FRM_PRIM_STR_SIZE ];
} T_FRM_ERROR_IND;

typedef struct
{
  U32   warning_code;
  char  warning_string [ FRM_PRIM_STR_SIZE ];
} T_FRM_WARNING_IND;

#if 0
/* frame status requests and confirmations */

/* register destination for error/warning indications */

typedef struct
{
  char  name [ RESOURCE_NAMELEN ];
} T_FRM_REGISTER_REQ;

typedef struct
{
  char  name [ RESOURCE_NAMELEN ];
} T_FRM_REGISTER_CNF;

/* task status */

typedef struct
{
  U32   task_id;
} T_FRM_TASK_STATUS_REQ;

typedef struct
{
  char  name [ RESOURCE_NAMELEN ];
  U32   priority;
  U32   stacksize;
  U32   unused_stack;
} T_FRM_TASK_DATA;

typedef struct
{
  T_FRM_TASK_DATA   task [ MAX_OS_TASKS ];;
} T_FRM_TASK_STATUS_CNF;

/* partition status */

typedef struct
{
  U32   partition_group_id;
} T_FRM_PARTITION_STATUS_REQ;

typedef struct
{
  T_FRM_PARTITION_DATA p_pool [ MAX_POOL_GROUPS*MAX_POOLS_PER_GROUP ];
} T_FRM_PARTITION_STATUS_CNF;

typedef struct
{
  U32   partition_pool_id;
  U32   partition_size
  U32   available;
  U32   allocated;
} T_FRM_PARTITION_DATA;

/* memory status */
typedef struct
{
  U32   memory_pool_id;
} T_FRM_MEMORY_STATUS_REQ;

typedef struct
{
  U32   memory_pool_id;
  U32   pool_size
  U32   available;
  U32   allocated;
} T_FRM_MEMORY_DATA;

typedef struct
{
  T_FRM_MEMORY_DATA m_pool [ MAX_MEMORY_POOLS };
} T_FRM_MEMORY_STATUS_CNF;

/* timer status */

typedef struct
{
  U32   timer_id;
} T_FRM_TIMER_STATUS_REQ;

typedef struct
{
  U32   max_timer;
  U32   max_simul_available_timer;
  U32   max_simul_running_timer;
} T_FRM_TIMER_STATUS_CNF;

/* semaphore status */

typedef struct
{
  U32   semaphore_id;
} T_FRM_SEMAPHORE_STATUS_REQ;

typedef struct
{
} T_FRM_SEMAPHORE_STATUS_CNF;
#endif

#endif /* FRM_PRIMITIVES_H */




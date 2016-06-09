/* 
+------------------------------------------------------------------------------
|  File:       pei.h
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
|  Purpose :  Protocol Entity Interface exported definitions.
+----------------------------------------------------------------------------- 
*/ 


#ifndef __PEI_H__
#define __PEI_H__

/*==== INCLUDES ============================================================*/

#include <stddef.h>

/*==== CONSTANTS ===========================================================*/

#define PEI_OK         0
#define PEI_ERROR   (-1)

/*==== TYPES ===============================================================*/

typedef int    T_PEI_RETURN;
typedef char * T_PEI_CONFIG;
typedef void   T_PEI_MONITOR;

/*
 * types and macros for pei-jumptable processing
 */

#define PALLOC_TRANSITION  /* to reinsert the sdu pointer in pei_primitive */
#define NO_COPY_ROUTING    /* has to be set for frame version > 2.5.0 */

  /*
   * for definitions of jumptables
   */
typedef void   (*T_VOID_FUNC)();
typedef short  (*T_SHORT_FUNC)();

typedef struct { T_VOID_FUNC func; size_t size; size_t soff; ULONG opc; } T_FUNC;
#define MAK_FUNC_S(FUNC,OPC) { (T_VOID_FUNC)FUNC, sizeof(T_##OPC), offsetof(T_##OPC,sdu), OPC }
#define MAK_FUNC_0(FUNC,OPC) { (T_VOID_FUNC)FUNC, sizeof(T_##OPC), 0                    , OPC }
#define MAK_FUNC_N(FUNC,OPC) { (T_VOID_FUNC)FUNC, 0              , 0                    , 0   }

#define TAB_SIZE(T) (sizeof T / sizeof *T)

typedef struct
{
  SHORT (*pei_init)(T_HANDLE);
  SHORT (*pei_exit)(void);
  SHORT (*pei_primitive)(void*);
  SHORT (*pei_timeout)(USHORT);
  SHORT (*pei_signal)(ULONG,void*);
  SHORT (*pei_run)(T_HANDLE,T_HANDLE);
  SHORT (*pei_config)(char*);
  SHORT (*pei_monitor)(void**);
} T_PEI_FUNC;


typedef struct                                         
{
  char const *Name;
  T_PEI_FUNC PeiTable;
  ULONG      StackSize;
  USHORT     QueueEntries;
  USHORT     Priority;
  USHORT   	 NumOfTimers;
  U32        Flags;
} T_PEI_INFO;


#endif /* __PEI_H__ */


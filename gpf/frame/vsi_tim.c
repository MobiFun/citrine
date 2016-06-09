/* 
+------------------------------------------------------------------------------
|  File:       vsi_tim.c
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
|             for the timer handling.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __VSI_TIM_C__
#define __VSI_TIM_C__
#endif

/*==== INCLUDES ===================================================*/

#include <string.h>

#include "gpfconf.h"
#include "typedefs.h"

#include "vsi.h"
#include "os.h"
#include "tools.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "frame.h"

/*==== TYPES ======================================================*/

typedef struct _T_TIMER_CONFIG_ENTRY
{
  T_HANDLE entity;
  T_TIME   value;
  USHORT   index;
  USHORT   mode;
  struct _T_TIMER_CONFIG_ENTRY *next;
} T_TIMER_CONFIG_ENTRY;

/*==== CONSTANTS ==================================================*/

LOCAL const T_STR_IND StrInd[] = 
{       
  "TIMER_SET",        TIMER_SET,
  "TIMER_RESET",      TIMER_RESET,
  "TIMER_SPEED_UP",   TIMER_SPEED_UP,
  "TIMER_SLOW_DOWN",  TIMER_SLOW_DOWN,
  "TIMER_SUPPRESS",   TIMER_SUPPRESS,
  "TIMER_CLEAN",      TIMER_CLEAN,
  NULL,               0
};

/*==== EXTERNALS ==================================================*/
/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

extern OS_HANDLE ext_data_pool_handle;
extern T_HANDLE TimerHandleField[];

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
char timer_configured;
T_TIMER_CONFIG_ENTRY *t_config;
#else
extern char timer_configured;
extern T_TIMER_CONFIG_ENTRY *t_config;
#endif

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== FUNCTIONS ==================================================*/

int GetTimerStartValue ( T_HANDLE Caller, USHORT TimerIndex, T_TIME OrgValue, T_TIME *NewValue );

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : GetTimerStartValue  |
+--------------------------------------------------------------------+

  PURPOSE : get start time modified by dynamic configuration

*/

int GetTimerStartValue ( T_HANDLE caller, USHORT index, T_TIME org_value, T_TIME *new_value )
{
T_TIMER_CONFIG_ENTRY *t_config_entry;
int found;
  
  if ( t_config != NULL )
  {
    found = FALSE;
    t_config_entry = t_config;
    while ( found == FALSE && t_config_entry != NULL )
    {
      if ( t_config_entry->entity == caller
        && t_config_entry->index  == index )
      {
        found = TRUE;
        switch (t_config_entry->mode)
        {
          case TIMER_SET:
            *new_value = t_config_entry->value;
          break;

          case TIMER_RESET:
            *new_value = org_value;
          break;

          case TIMER_SPEED_UP:
            *new_value = org_value / t_config_entry->value;
          break;

          case TIMER_SLOW_DOWN:
            *new_value = org_value * t_config_entry->value;
          break;

          default:
          return VSI_ERROR;
        }
        vsi_o_ttrace ( caller, TC_TIMER, "Timerstart: Index %d, Time %d -> %d",index, org_value, *new_value );
      }
      t_config_entry = t_config_entry->next;
    }
    if ( found == FALSE )
    {
      *new_value = org_value;
    }
  }
  else
  {
    *new_value = org_value;
  }
  if (*new_value == 0)
  {
    *new_value = 1;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : vsi_t_start         |
+--------------------------------------------------------------------+

  PURPOSE : start timer that expires only once

*/

int vsi_t_start (T_HANDLE Caller, USHORT TimerIndex, T_TIME Value)
{
OS_HANDLE TimerHandle;

  vsi_o_ttrace ( Caller, TC_TIMER, "Timerstart: Index %d, Time %d",TimerIndex, Value) ;

  if ( TimerIndex >= pf_TaskTable[Caller].NumOfTimers )
    vsi_o_assert( NO_TASK, OS_SYST_ERR_TASK_TIMER, __FILE__, __LINE__, 
                  "TimerIndex > NumOfTimers for entity %s", pf_TaskTable[Caller].Name );

  TimerHandle = (*(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) & TIMER_HANDLE_MASK);
  
  if ( !TimerHandle )
  {
    if ( os_CreateTimer ( Caller, pf_Timeout, &TimerHandle, pf_TaskTable[Caller].MemPoolHandle ) == OS_ERROR ) 
      vsi_o_assert( Caller, OS_SYST_ERR_SIMUL_TIMER, __FILE__, __LINE__, 
                    "Number of started timers > MAX_SIMULTANEOUS_TIMER" );
  }
  if ( timer_configured && GetTimerStartValue ( Caller, TimerIndex, Value, &Value ) == VSI_ERROR )
    return VSI_ERROR;

  *(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) = TimerHandle;

  if ( os_StartTimer ( Caller, TimerHandle, TimerIndex, Value, 0 ) == OS_ERROR )
    return VSI_ERROR;

  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : vsi_t_pstart        |
+--------------------------------------------------------------------+

  PURPOSE : start periodic timer

*/

int vsi_t_pstart (T_HANDLE Caller, USHORT TimerIndex, T_TIME Value1, T_TIME Value2 )
{
OS_HANDLE TimerHandle;

  vsi_o_ttrace ( Caller, TC_TIMER, "Timerstart: Index %d, ITime %d PTime %d",TimerIndex, Value1, Value2) ;

  if ( TimerIndex >= pf_TaskTable[Caller].NumOfTimers )
    vsi_o_assert( NO_TASK, OS_SYST_ERR_TASK_TIMER, __FILE__, __LINE__, 
                  "TimerIndex > NumOfTimers for entity %s", pf_TaskTable[Caller].Name );

  TimerHandle = (*(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) & TIMER_HANDLE_MASK);
  
  if ( !TimerHandle )
  {
    if ( os_CreateTimer ( Caller, pf_Timeout, &TimerHandle, pf_TaskTable[Caller].MemPoolHandle ) == OS_ERROR ) 
      vsi_o_assert( Caller, OS_SYST_ERR_SIMUL_TIMER, __FILE__, __LINE__, 
                    "Number of started timers > MAX_SIMULTANEOUS_TIMER" );
  }

  if ( timer_configured && GetTimerStartValue ( Caller, TimerIndex, Value1, &Value1 ) == VSI_ERROR )
    return VSI_ERROR;

  if ( timer_configured && GetTimerStartValue ( Caller, TimerIndex, Value2, &Value2 ) == VSI_ERROR )
    return VSI_ERROR;

  *(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) = TimerHandle | PERIODIC_TIMER;

  if ( os_StartTimer ( Caller, TimerHandle, TimerIndex, Value1, Value2 ) == OS_ERROR )
    return VSI_ERROR;

  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : vsi_t_stop          |
+--------------------------------------------------------------------+

  PURPOSE : stop timer

*/

int vsi_t_stop (T_HANDLE Caller, USHORT TimerIndex )
{
OS_HANDLE TimerHandle;

  vsi_o_ttrace ( Caller, TC_TIMER, "Timerstop:  Index %d",TimerIndex) ;
  TimerHandle = (*(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) & TIMER_HANDLE_MASK);
  if ( TimerHandle && ( (TimerHandle & TIMER_HANDLE_MASK) < MaxTimer ) ) 
  {
/*
    if ( os_StopTimer ( Caller, TimerHandle ) == OS_ERROR )
      return VSI_ERROR;    

    if ( os_DestroyTimer ( Caller, TimerHandle ) == OS_ERROR )
      return VSI_ERROR;  
*/
    os_StopTimer ( Caller, TimerHandle );
    os_DestroyTimer ( Caller, TimerHandle );

    *(pf_TaskTable[Caller].FirstTimerEntry + TimerIndex) = 0;
    return VSI_OK;
  }
  return VSI_ERROR;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : vsi_t_status        |
+--------------------------------------------------------------------+

  PURPOSE : request remaining time 

*/

int vsi_t_status (T_HANDLE Caller, USHORT Index, T_TIME *Value )
{
OS_HANDLE TimerHandle;

  if ( (TimerHandle = (*(pf_TaskTable[Caller].FirstTimerEntry + Index) & TIMER_HANDLE_MASK) ) < MaxTimer )  
  {
    if ( TimerHandle == 0 || (*(pf_TaskTable[Caller].FirstTimerEntry + Index) & TIMEOUT_OCCURRED) )
    {
      *Value = 0;
      vsi_o_ttrace ( Caller, TC_TIMER, "Timerstatus: Index %d, Remaining_time %d",Index, *Value) ;
      return VSI_OK;
    }
    /* 
       In case the timer interrupt occurrs just after the check 5 lines above, this will be handled by
       os_QueryTimer() returning a Value of 0 because thew expired timer can no longer be found in the list.
    */
    if ( os_QueryTimer ( Caller, TimerHandle, Value ) == OS_ERROR )
    {
      *Value = 0;
      return VSI_ERROR; 
    }
    vsi_o_ttrace ( Caller, TC_TIMER, "Timerstatus: Index %d, Remaining time %d",Index, *Value) ;
    return VSI_OK;
  }
  vsi_o_ttrace ( Caller, TC_TIMER, "TimerHandle out of range: Index %d, TimerHandle %d",Index, TimerHandle ) ;
  return VSI_ERROR;  

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM             |
| STATE   : code                       ROUTINE : vsi_t_config        |
+--------------------------------------------------------------------+

  PURPOSE : timer configuration

*/
int vsi_t_config (T_HANDLE caller, USHORT index, UBYTE mode, ULONG value )
{
T_TIMER_CONFIG_ENTRY *t_config_entry = NULL;
T_TIMER_CONFIG_ENTRY *t_next_entry;
T_TIMER_CONFIG_ENTRY *t_new_entry = NULL;
int found = FALSE;

  if ( index < pf_TaskTable[caller].NumOfTimers )
  {
    if ( t_config != NULL )
    {
      t_next_entry = t_config;
      do
      {
        t_config_entry = t_next_entry;
        if ( t_config_entry->entity == caller
          && t_config_entry->index  == index )
        {
          found = TRUE;
          break;
        }
        t_next_entry = t_config_entry->next;
      } while ( t_next_entry != NULL );
    }

    if ( found == FALSE )
    {
      if ( os_AllocateMemory ( caller, (T_VOID_STRUCT**)&t_new_entry, sizeof(T_TIMER_CONFIG_ENTRY), OS_NO_SUSPEND, ext_data_pool_handle ) == OS_TIMEOUT )
        return VSI_ERROR;
      t_new_entry->next = NULL;
    }

    if ( t_config == NULL )
    {
      t_config       = t_new_entry;
      t_config_entry = t_new_entry;
    }
    else
    {
      if ( found == FALSE && t_config_entry != NULL )
      {
        t_config_entry->next = t_new_entry;
        t_config_entry       = t_new_entry;
      }
    }

    if ( t_config_entry != NULL )
    {
      t_config_entry->entity = caller;
      t_config_entry->index  = index;
      t_config_entry->mode   = mode;
      t_config_entry->value  = value;
      timer_configured = TRUE;
      return VSI_OK;
    }
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  _vsi_t_config
+------------------------------------------------------------------------------
|  Description  :  Parse a timer configuration string into the components
|                  timer index, timer mode, timer value.
|
|  Parameters   :  Caller     - calling task
|                  *CfgString - configuration string
|                  *pTable    - pointer to configuration string table
|
|  Return       :  VSI_OK
|                  VSI_ERROR
+------------------------------------------------------------------------------
*/
int _vsi_t_config ( T_HANDLE Caller, char *CfgString, const T_STR_IND *pTable )
{
T_TIMER_CONFIG_ENTRY *t_config_entry;
T_TIMER_CONFIG_ENTRY *t_next;
char token[20];
unsigned int offset = 0,len;
USHORT Index;
BYTE Mode;
int i = 0;
unsigned int Value;

  if ( pTable != NULL )
  {
    len = GetNextToken (CfgString, token, " #");
    offset = offset + len +1;
    while ( StrInd[i].Str && strcmp ( token, StrInd[i].Str ) )
    { i++; }
    if ( StrInd[i].Str == NULL )
      return VSI_ERROR;

    if ( (Mode = (BYTE)StrInd[i].Ind) == TIMER_CLEAN )
    {
      t_config_entry = t_config;
      while ( t_config_entry != NULL )
      {
        t_next = t_config_entry->next;
        os_DeallocateMemory ( Caller, (T_VOID_STRUCT*)t_config_entry );
        t_config_entry = t_next;
      }
      t_config = NULL;
      return VSI_OK;
    }
    len = GetNextToken (CfgString+offset, token, " #");
    offset = offset + len +1;
    while ( pTable->Str && strcmp ( token, pTable->Str ) )
    { pTable++; }
    if ( pTable->Str == NULL )
    { 
      vsi_o_ttrace ( 0, TC_SYSTEM, "Timer not found!") ;
      return VSI_OK;  /* VSI_OK is returned to avoid creating a new return code */
    }
    Index = pTable->Ind;
    len = GetNextToken (CfgString+offset, token, " #");
    Value = ASCIIToHex (token, CHARS_FOR_32BIT);
 
    return ( vsi_t_config ( Caller, Index, Mode, Value ) );
  }
  return VSI_ERROR;

}
#endif

#ifndef RUN_INT_RAM
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TIM              |
| STATE   : code                       ROUTINE : InitializeTimerConfig|
+---------------------------------------------------------------------+

  PURPOSE : initialize timer configuration

*/ 

void InitializeTimer ( void )
{
int i;

  for ( i = 0; i <= MaxTimer; i++)
  {
    TimerHandleField[i] = 0;
  }
  timer_configured = FALSE;
  t_config = NULL;
}
#endif



/*
+------------------------------------------------------------------------------
|  File:       vsi_trc.c
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
|             for the tracing functionality.
+-----------------------------------------------------------------------------
*/

#ifndef __VSI_TRC_C__
#define __VSI_TRC_C__
#endif

#undef TEST_EXT_TRACE

/*==== LINT =======================================================*/

/*lint -emacro(506,va_start)   Constant value Boolean */
/*lint -emacro(506,va_arg)     Constant value Boolean */
/*lint -emacro(718,va_start)   Symbol  'Symbol' undeclared, assumed to return  int */
/*lint -emacro(522,va_arg)     Warning -- Expected void type, assignment, increment or decrement */
/*lint -emacro(10,va_arg)      Error -- Expecting '(' */
/*lint -emacro(516,va_arg)     Warning -- 'Symbol _va_argref()' has arg. type conflict (arg. no. 1 -- basic) */
/*lint -e661 */
/*lint -e662 */

/*==== INCLUDES ===================================================*/

#ifndef _VXWORKS_
#include <stdarg.h>
#endif
#include <stdio.h>
#include <string.h>

#include "gpfconf.h"
#include "typedefs.h"

#include "vsi.h"
#include "os.h"
#include "tools.h"
#include "frame.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "p_frame.h"
#ifndef _TOOLS_
#include "../tst_pei/tstdriver.h"
#endif

/*==== TYPES ======================================================*/

typedef struct
{
  unsigned int magic_nr;
  void (*trace_error)(const char * const format, va_list varpars);      
  void (*trace_assert)(const char * const format, va_list varpars);      
} T_EXT_TRACE;

/*==== CONSTANTS ==================================================*/

#define EXT_TRACE_INITIALIZED 0xaffedead

/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED")
#endif

/*
 * contains the correspnding flags for each task
 */
extern T_HANDLE TestGroupHandle;
extern UBYTE FrameEnv;
extern char TaskName[];
extern char FRM_TST_NAME[];

#ifdef _TOOLS_
 extern char FRM_SYST_NAME[];
#else
 extern OS_HANDLE TST_Handle;
 extern OS_HANDLE RCV_Handle;
 extern int time_is_tdma_frame;
 extern char error_ind_dst[];
 extern T_FRM_ERROR_IND *frm_error_ind;
#endif

/*==== VARIABLES ==================================================*/

/*
  this file may be compiled twice for FLASH and RAM, make sure we
  get the variable declarations exactly once
*/

#ifndef RUN_INT_RAM
  /* must be seen only from FLASH */
  GLOBAL char TraceBuffer [ TRACE_TEXT_SIZE ]={0};

  /* must be seen from RAM and FLASH */
  GLOBAL OS_HANDLE trc_hCommTrace=VSI_ERROR;
  GLOBAL BOOL trc_LittleEndian = FALSE;
  /* used by the entities */
  GLOBAL char EntityNameBuf[RESOURCE_NAMELEN];
  GLOBAL USHORT emergeny_trace;
#else
  /* must be seen from RAM and FLASH */
  extern OS_HANDLE trc_hCommTrace;
  extern BOOL trc_LittleEndian;
  extern USHORT emergeny_trace;
#endif

#ifndef RUN_INT_RAM
 T_EXT_TRACE ext_trace_func; 
#else
 extern T_EXT_TRACE ext_trace_func; 
#endif 

#ifdef TEST_EXT_TRACE

void trace_error ( const char * const format, va_list varpars );
void trace_assert ( const char * const format, va_list varpars );

#ifndef RUN_INT_RAM
T_EXT_TRACE_FUNC ext_trace_functions =
{
trace_error,
trace_assert
};
#else
extern T_EXT_TRACE_FUNC ext_trace_functions;
#endif

#endif /* TEST_EXT_TRACE */

/*==== VARIABLES ==================================================*/

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== PROTOTYPES ==================================================*/

void ext_trace_init ( void );
int int_vsi_o_ttrace ( T_HANDLE Caller, ULONG TraceClass, const char * const format, va_list varpars );
int int_vsi_o_itrace ( T_HANDLE Caller, ULONG TraceClass, ULONG index, const char * const format, va_list varpars );
int vsi_o_datasend ( T_HANDLE caller, T_HANDLE dst, char *ext_dst, T_PRIM_HEADER *prim FILE_LINE_TYPE );
U32 int_vsi_tc2trace_opc(ULONG trace_class);

#ifdef _TARGET_
SHORT tst_pei_primitive (void *primitive);
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_FLASH
U32 int_vsi_tc2trace_opc(ULONG trace_class)
{
  switch (trace_class)
  {
    case TC_FUNC:
      return FUNC_TRACE_OPC;
    case TC_EVENT:
      return EVENT_TRACE_OPC;
    case TC_PRIM:
      return PRIM_TRACE_OPC;
    case TC_STATE:
      return STATE_TRACE_OPC;
    case TC_SYSTEM:
      return SYSTEM_TRACE_OPC;
    case TC_ISIG:
      return ISIG_TRACE_OPC;
    case TC_ERROR:
      return ERROR_TRACE_OPC;
    case TC_CCD:
      return CCD_TRACE_OPC;
    case TC_TIMER:
      return TIMER_TRACE_OPC;
    case TC_PROFILER:
      return PROFILER_TRACE_OPC;

    case TC_USER1:
      return USER1_TRACE_OPC;
    case TC_USER2:
      return USER2_TRACE_OPC;
    case TC_USER3:
      return USER3_TRACE_OPC;
    case TC_USER4:
      return USER4_TRACE_OPC;
    case TC_USER5:
      return USER5_TRACE_OPC;
    case TC_USER6:
      return USER6_TRACE_OPC;
    case TC_USER7:
      return USER7_TRACE_OPC;
    case TC_USER8:
      return USER8_TRACE_OPC;

    default:
      return TRACE_OPC;
  }
}
#endif /* RUN_FLASH */

#ifdef _TOOLS_
/*
+------------------------------------------------------------------------------
|  Function     :  vsi_o_set_htrace
+------------------------------------------------------------------------------
|  Description  :  This function sets the module variable trc_hCommTrace
|
|  Parameters   :  comhandle - the new value
|
|  Return       :  none
+------------------------------------------------------------------------------
*/
void  vsi_o_set_htrace (T_HANDLE comhandle)
{
  trc_hCommTrace = (OS_HANDLE) comhandle;
}
#endif /* _TOOLS_ */

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_trace         |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
LOCAL int vsi_o_trace ( T_HANDLE Caller, T_PRIM_HEADER *prim, ULONG suspend )
{
T_S_HEADER *s_hdr;
LONG Status;
OS_QDATA QMsg;
BOOL AlarmTrace = FALSE;

  s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
  if ( TracesAborted[Caller] )
  {
#ifdef _TOOLS_
    /*
     * Only needed for _TOOLS_ because Nucleus provides at least 52 Bytes in the partition
     */
    os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
    Status = os_AllocatePartition ( Caller, (T_VOID_STRUCT**)&prim, 80, suspend, TestGroupHandle );
#endif
    sprintf ((char*)P2D(prim),"Trc Abort: %d !",TracesAborted[Caller]+1 );
    prim->len = strlen ((char*)P2D(prim)) + sizeof(T_PRIM_HEADER);
    prim->sh_offset = S_HDR_OFFSET(prim->len);
    s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
    AlarmTrace = TRUE;
  }

  s_hdr->snd[0] = (char)Caller;
  if ( Caller )
    s_hdr->snd[0] |= (char)HANDLE_BIT;

  s_hdr->org_rcv[0] = 0;

#ifdef _TOOLS_
  get_local_time (&s_hdr->time);
#else
  if ( time_is_tdma_frame == 1 )
  {
    os_GetTime(Caller,&s_hdr->time);
    s_hdr->time |= TIME_IS_TDMA;
  }
  else
  {
    os_GetTime(Caller,&s_hdr->time);
    s_hdr->time &= ~TIME_IS_TDMA;
  }
#endif

  QMsg.flags  = 0;
  QMsg.data16 = MSG_PRIMITIVE;
  QMsg.data32 = 0;
  QMsg.ptr = (T_VOID_STRUCT*)prim;
#ifdef _TOOLS_
  QMsg.len = ALIGN(prim->len) + sizeof(T_S_HEADER);
#endif
  os_GetTime ( 0, &QMsg.time );

#ifndef _TOOLS_
  if ( trc_hCommTrace == VSI_ERROR )
    if ( (trc_hCommTrace = vsi_c_open ( TST_Handle, FRM_TST_NAME ) ) == OS_ERROR )
    {
      os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
      return VSI_ERROR;
    }
#else
  if ( trc_hCommTrace == VSI_ERROR )
  {
    if ( FrameEnv == ENV_TOOLS )
    {
      os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
      return VSI_ERROR;
    }
    else
    {
      if ( os_OpenQueue ( NO_TASK, FRM_TST_NAME, &trc_hCommTrace ) == OS_ERROR )
      {
        os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
        return VSI_ERROR;
      }
    }
  }
  if ( FrameEnv == ENV_TOOLS )
  {
    s_hdr->snd[0] = '~';
    if ( os_GetTaskName ( Caller, Caller, &s_hdr->snd[1] ) == OS_ERROR )
    {
      char Sender[RESOURCE_NAMELEN];
      char *ptr = (char*)P2D(prim);
      unsigned int NameLen;

      if ( *ptr == '~')
      {
        NameLen = GetNextToken ((char*)(ptr), Sender,"~");
        InsertString(Sender, &s_hdr->snd[1], 4);
        prim->len -= 2+NameLen;
        memcpy ( ptr, ptr+2+NameLen, prim->len-sizeof(T_PRIM_HEADER) );
        QMsg.len = ALIGN(prim->len) + sizeof(T_S_HEADER);
      }
      else
        InsertString(FRM_SYST_NAME, &s_hdr->snd[1], 4);
    }
  }
#endif /* _TOOLS_ */

  QMsg.e_id = trc_hCommTrace;

#ifndef _TOOLS_
  if ( Caller == TST_Handle || Caller == RCV_Handle )
  {
    tst_drv_write ( Caller, 0, NULL, (char*)P2D(prim) );
    os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
    return VSI_OK;
  }
#ifndef _TARGET_
  else
  /*
   * for the PC Simulation SuspendTrace[0] is != 0. To avoid that the system traces
   * sent with caller NO_TASK block the system if the caller was TST, the Caller
   * is set to the real caller here. This has no consequence on the caller name
   * displayed in PCO because this is already formated into the header.
   */
  {
    if ( Caller == NO_TASK )
    {
      Caller = e_running[os_MyHandle()];
    }
  }
#endif
#endif

  QMsg.e_id = trc_hCommTrace;
#ifdef _TOOLS_
  Status = os_SendToQueue ( NO_TASK, trc_hCommTrace, MSG_TRACE_PRIO, suspend, &QMsg );
#else
  Status = os_SendToQueue ( NO_TASK, pf_TaskTable[trc_hCommTrace].QueueHandle, MSG_TRACE_PRIO, suspend, &QMsg );
#endif
  if ( Status == OS_OK || Status == OS_WAITED )
  {
    TracesAborted[Caller] = 0;
    if ( AlarmTrace )
      return VSI_ERROR;
    else
      return VSI_OK;
  }
  else
  {
    /*
     * No queue space available -> free partition
     */
    os_DeallocatePartition ( Caller, (T_VOID_STRUCT*)prim );
    TracesAborted[Caller]++;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_o_primsend      |
+--------------------------------------------------------------------+

  PURPOSE : traces a data segment as a primitive

*/
int vsi_o_primsend ( T_HANDLE caller, unsigned int mask, T_HANDLE dst, char *ext_dst, unsigned int opc, void *ptr, unsigned int len FILE_LINE_TYPE )
{
int alloc_size;
T_PRIM_HEADER *prim;
T_HANDLE e_handle;

  if ( caller != 0 )
    e_handle = caller;
  else
    e_handle = e_running[os_MyHandle()];

  if ( (e_handle >= 0) && (TraceMask[e_handle] & mask) )
  {
    if ( opc != 0 )
    {
      alloc_size = len + sizeof(T_PRIM_HEADER);
      if ( alloc_size < (int)MaxPrimPartSize )
      {
        prim = (T_PRIM_HEADER*)vsi_c_new ( 0, alloc_size, opc FILE_LINE );
        memcpy((char*)P2D(prim), ptr, len);
        /* if primitive is FRM_WARNING_IND -> free */
        if ( opc == FRM_WARNING_IND )
        {
          PFREE(ptr);
        }
      }
      else
      {
        vsi_o_ttrace ( caller, TC_SYSTEM, "SYSTEM WARNING: Binary trace too long -> discarded in %s(%d)" FILE_LINE );
        return VSI_ERROR;
      }
    }
    else
    {
      prim = D2P(ptr);
    }
    if ( vsi_o_datasend ( e_handle, dst, ext_dst, prim FILE_LINE ) == VSI_ERROR )
    {
      vsi_c_free ( 0, (T_VOID_STRUCT**)&prim FILE_LINE );
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_o_sdusend       |
+--------------------------------------------------------------------+

  PURPOSE : traces an SDU as a primitive

*/
int vsi_o_sdusend ( T_HANDLE caller, T_HANDLE dst, char *ext_dst, int opc, char ent, char dir, char type, void *ptr, unsigned int len FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
T_HANDLE e_handle;
int alloc_size;

  if ( caller != 0 )
    e_handle = caller;
  else
    e_handle = e_running[os_MyHandle()];

  if ( (e_handle >= 0) && (TraceMask[e_handle] & TC_SDU) )
  {
    if ( opc != 0 )
    {
      alloc_size = len + sizeof(T_PRIM_HEADER) + sizeof(T_SDU_TRACE);
      if ( alloc_size < (int)MaxPrimPartSize )
      {
        prim = (T_PRIM_HEADER*)vsi_c_new ( 0, alloc_size, opc FILE_LINE );
        ((T_SDU_TRACE*)(P2D(prim)))->entity = ent;
        ((T_SDU_TRACE*)(P2D(prim)))->dir = dir;
        ((T_SDU_TRACE*)(P2D(prim)))->type = type;
        ((T_SDU_TRACE*)(P2D(prim)))->align1 = 0;
        memcpy(((char*)P2D(prim))+sizeof(T_SDU_TRACE), ptr, len);
        if ( vsi_o_datasend ( e_handle, dst, ext_dst, prim FILE_LINE ) == VSI_ERROR )
        {
          vsi_c_free ( 0, (T_VOID_STRUCT**)&prim FILE_LINE );
        }
      }
      else
      {
        vsi_o_ttrace ( caller, TC_SYSTEM, "SYSTEM WARNING: Binary trace too long -> discarded in %s(%d)" FILE_LINE );
      }
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_o_datasend      |
+--------------------------------------------------------------------+

  PURPOSE : traces an already allocated primitive

*/
int vsi_o_datasend ( T_HANDLE caller, T_HANDLE dst, char *ext_dst, T_PRIM_HEADER *prim FILE_LINE_TYPE )
{
int alloc_size;
T_PRIM_HEADER *carrier;
T_S_HEADER *s_hdr;
LONG status;
OS_QDATA DMsg;
unsigned int i;
ULONG suspend;

  suspend = get_suspend_state(caller,CHECK_TRC_SUSPEND);

  /* allocate carrier */
  alloc_size = S_ALLOC_SIZE(4 + 1);
  status = os_AllocatePartition ( caller, (T_VOID_STRUCT**)&carrier, alloc_size, suspend, TestGroupHandle );
  if ( status == OS_OK || status == OS_WAITED || status == OS_ALLOCATED_BIGGER )
  {
    DMsg.data16 = MSG_PRIMITIVE;
  	DMsg.data32 = prim->opc;
#ifdef _TOOLS_
    DMsg.len = alloc_size;
#endif
    DMsg.ptr = (T_VOID_STRUCT*)carrier;

    carrier->opc = prim->opc;
    carrier->len = alloc_size;
    carrier->sh_offset = S_HDR_OFFSET(alloc_size - sizeof(T_S_HEADER));
    s_hdr = (T_S_HEADER*)((ULONG*)carrier + carrier->sh_offset);
    if ( dst != 0 )
    {
      if ( vsi_e_name ( caller, dst, TaskName ) == VSI_OK )
      {
        /* set org_rcv */
        for (i = 0; TaskName[i] && i < sizeof (s_hdr->rcv); i++)
          s_hdr->org_rcv[i] = TaskName[i];
        if (i < sizeof s_hdr->rcv)
          s_hdr->org_rcv[i] = 0;
      }
      else
      {
        s_hdr->org_rcv[0] = 0;
      }
    }
    else
    {
      s_hdr->org_rcv[0] = 0;
    }
    strncpy (s_hdr->rcv, ext_dst, RESOURCE_NAMELEN);
    s_hdr->rcv[RESOURCE_NAMELEN-1] = 0;
    if ( caller != 0 )
    {
      if ( vsi_e_name ( caller, caller, TaskName ) == VSI_OK )
      {
        /* set snd */
        for (i = 0; TaskName[i] && i < sizeof (s_hdr->snd); i++)
          s_hdr->snd[i] = TaskName[i];
        if (i < sizeof s_hdr->snd)
          s_hdr->snd[i] = 0;
      }
    }
    else
    {
      if ( pf_TaskTable[caller].Name[0] != 0 )
      {
        s_hdr->snd[0] = (char)(caller | HANDLE_BIT);
      }
      else
      {
        s_hdr->rcv[0] = 0;
      }
    }
    os_GetTime(0,&s_hdr->time);

    ((T_PRIM_X*)(carrier))->prim_ptr = prim;
    DMsg.e_id = trc_hCommTrace;
  }
  else
  {
    TracesAborted[caller]++;
    return VSI_ERROR;
  }

#ifdef MEMORY_SUPERVISION
  vsi_ppm_send ( caller, pf_TaskTable[trc_hCommTrace].QueueHandle, prim FILE_LINE_MACRO );
#endif /* MEMORY_SUPERVISION */
  if ( os_SendToQueue ( caller, pf_TaskTable[trc_hCommTrace].QueueHandle, OS_NORMAL, suspend, &DMsg ) == OS_TIMEOUT )
  {
    os_DeallocatePartition ( caller, (T_VOID_STRUCT*)carrier );
    TracesAborted[caller]++;
    return VSI_ERROR;
  }
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_o_memtrace      |
+--------------------------------------------------------------------+

  PURPOSE : check if PS already started

*/
int vsi_o_memtrace ( T_HANDLE caller, void *ptr, unsigned int len )
{
T_PRIM_HEADER *prim;
T_HANDLE e_handle;
LONG status;
unsigned int i;
ULONG suspend;

  if ( caller != 0 )
    e_handle = caller;
  else
    e_handle = e_running[os_MyHandle()];

  if ( (e_handle >= 0) && (TraceMask[e_handle] & TC_DATA) )
  {
    suspend = get_suspend_state(e_handle,CHECK_TRC_SUSPEND);
    status = os_AllocatePartition ( e_handle, (T_VOID_STRUCT**)&prim, S_ALLOC_SIZE(len)*3, suspend, TestGroupHandle );
    if ( status == OS_OK || status == OS_WAITED || status == OS_ALLOCATED_BIGGER )
    {
      unsigned char *dst_ptr = (unsigned char*)P2D(prim);
      unsigned char *src_ptr = (unsigned char*)ptr;
      for ( i = 0; i < len; i++, src_ptr++ )
      {
        if (*src_ptr>>4 > 9)
          *dst_ptr++ = (*src_ptr>>4) + ('A'-10);
        else
          *dst_ptr++ = (*src_ptr>>4) +'0';
        if ((*src_ptr&0xf) > 9)
          *dst_ptr++ = (*src_ptr&0xf) + ('A'-10);
        else
          *dst_ptr++ = (*src_ptr&0xf) +'0';
        *dst_ptr++ = 0x20;
      }

      prim->opc = int_vsi_tc2trace_opc(TC_DATA);
      prim->len = 3*len + sizeof(T_PRIM_HEADER);
      prim->sh_offset = S_HDR_OFFSET(prim->len);

      return ( vsi_o_trace ( e_handle, prim, suspend ) );
    }
    TracesAborted[e_handle]++;
    return VSI_ERROR;

  }
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_func_ttrace   |
+--------------------------------------------------------------------+

  PURPOSE : traces a function name

*/
int vsi_o_func_ttrace ( const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_FUNC) )
  {
    va_start (varpars, format);
    return int_vsi_o_ttrace ( Caller, TC_FUNC, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_event_ttrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces an event

*/
int vsi_o_event_ttrace ( const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_EVENT) )
  {
    va_start (varpars, format);
    return int_vsi_o_ttrace ( Caller, TC_EVENT, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_error_ttrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces an error

*/
int vsi_o_error_ttrace ( const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_ERROR) )
  {
    va_start (varpars, format);
    int_vsi_o_ttrace ( Caller, TC_ERROR, format, varpars );
    vsi_o_ttrace ( NO_TASK, TC_ERROR, "TRACE ERROR in %s", pf_TaskTable[Caller].Name );
    if ( ext_trace_func.trace_error != NULL )
    {
      ext_trace_func.trace_error ( format, varpars );
    }
    return VSI_OK;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_state_ttrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_state_ttrace ( const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_STATE) )
  {
    va_start (varpars, format);
    return int_vsi_o_ttrace ( Caller, TC_STATE, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_class_ttrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_class_ttrace ( ULONG trace_class, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & trace_class) )
  {
    va_start (varpars, format);
    return int_vsi_o_ttrace ( Caller, trace_class, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_ttrace        |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_ttrace ( T_HANDLE Caller, ULONG TraceClass, const char * const format, ... )
{
va_list varpars;

  if ( (Caller >= 0) && (TraceMask[Caller] & TraceClass) )
  {
    va_start (varpars, format);
    return int_vsi_o_ttrace ( Caller, TraceClass, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : int_vsi_o_ttrace    |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int int_vsi_o_ttrace ( T_HANDLE Caller, ULONG TraceClass, const char * const format, va_list varpars )
{
T_PRIM_HEADER *prim;
LONG Status;
ULONG suspend;
unsigned int offset = 0;
unsigned int num;

  suspend = get_suspend_state(Caller,CHECK_TRC_SUSPEND);
  Status = os_AllocatePartition ( Caller, (T_VOID_STRUCT**)&prim, TextTracePartitionSize, suspend, TestGroupHandle );
  if ( Status == OS_OK || Status == OS_WAITED || Status == OS_ALLOCATED_BIGGER )
  {
#if 0
      /* be activated when PCO can handle this */
    if ( TraceClass == TC_ERROR )
    {
      *((char*)(P2D(prim))) = '#';
      offset = 1;
    }
#endif
#ifdef _TOOLS_
    int trc_length = TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER) - 1;
    trc_length &= ~0x03;
    num = offset + (unsigned int)_vsnprintf ((char*)(P2D(prim)) + offset, trc_length, format, varpars) + 1;  /* + 1 for terminating 0 */
#else
    num = offset + (unsigned int)vsprintf ((char*)(P2D(prim)) + offset, format, varpars) + 1;  /* + 1 for terminating 0 */
#endif
    va_end (varpars);
    if ( num + sizeof(T_S_HEADER) + sizeof(T_PRIM_HEADER) >= TextTracePartitionSize )
    {
      sprintf ( (char*)(P2D(prim))+60, "... %s trace too long", pf_TaskTable[Caller].Name );
      vsi_o_assert ( NO_TASK, OS_SYST_ERR_STR_TOO_LONG, __FILE__, __LINE__,(char*)(P2D(prim)));
    }
    prim->opc = int_vsi_tc2trace_opc(TraceClass);
    prim->len = strlen((char*)P2D(prim)) + sizeof(T_PRIM_HEADER);
    prim->sh_offset = S_HDR_OFFSET(prim->len);

    return ( vsi_o_trace ( Caller, prim, suspend ) );
  }
  if ( FrameEnv == ENV_STACK )
  {
    TracesAborted[Caller]++;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_func_itrace   |
+--------------------------------------------------------------------+

  PURPOSE : traces a function name

*/
int vsi_o_func_itrace ( ULONG index, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_FUNC) )
  {
    va_start (varpars, format);
    return int_vsi_o_itrace ( Caller, TC_FUNC, index, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_event_itrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces an event

*/
int vsi_o_event_itrace ( ULONG index, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_EVENT) )
  {
    va_start (varpars, format);
    return int_vsi_o_itrace ( Caller, TC_EVENT, index, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_error_itrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces an error

*/
int vsi_o_error_itrace ( ULONG index, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_ERROR) )
  {
    va_start (varpars, format);
    int_vsi_o_itrace ( Caller, TC_ERROR, index, format, varpars );
    os_GetTaskName ( Caller, Caller, TaskName );
    vsi_o_ttrace ( NO_TASK, TC_ERROR, "TRACE ERROR in %s", pf_TaskTable[Caller].Name );
    return VSI_OK;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_state_itrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_state_itrace ( ULONG index, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_STATE) )
  {
    va_start (varpars, format);
    return int_vsi_o_itrace ( Caller, TC_STATE, index, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_class_itrace  |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_class_itrace ( ULONG trace_class, ULONG Index, const char * const format, ... )
{
va_list varpars;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  if ( (Caller >= 0) && (TraceMask[Caller] & trace_class) )
  {
    va_start (varpars, format);
    return int_vsi_o_itrace ( Caller, trace_class, Index, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_ttrace        |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int vsi_o_itrace ( T_HANDLE Caller, ULONG TraceClass, ULONG Index, const char * const format, ... )
{
va_list varpars;

  if ( (Caller >= 0) && (TraceMask[Caller] & TraceClass) )
  {
    va_start (varpars, format);
    return int_vsi_o_itrace ( Caller, TraceClass, Index, format, varpars );
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : int_vsi_o_itrace    |
+--------------------------------------------------------------------+

  PURPOSE : traces a dynamic string

*/
int int_vsi_o_itrace ( T_HANDLE Caller, ULONG TraceClass, ULONG index, const char * const format,
                         va_list varpars )
{
T_PRIM_HEADER *prim;
LONG Status;
ULONG suspend;
unsigned int offset = 0;
unsigned int paramCount;
unsigned int i;

/*
 * the offset is calculated prior to the actual write operation
 * to avoid writing beyond the allocated size
 *
 * all write operation are addressed relatively to the precalculated
 * offset e.g.  *(&Prim->Data + offset - 5)
*/
  if ( (Caller >= 0) && (TraceMask[Caller] & TraceClass) )
  {
    suspend = get_suspend_state(Caller,CHECK_TRC_SUSPEND);
    Status = os_AllocatePartition ( Caller, (T_VOID_STRUCT**)&prim, (ULONG)TextTracePartitionSize, suspend, TestGroupHandle );
    if ( Status == OS_OK || Status == OS_WAITED || Status == OS_ALLOCATED_BIGGER )
    {

      /*
       * the index preceded by %
       */
#if 0
      /* be activated when PCO can handle this */
      if ( TraceClass == TC_ERROR )
      {
        *((char*)(P2D(prim))) = '#';
        offset = 1;
      }
#endif
      offset += 5;
      if(offset <= TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
      {
        *((char*)(P2D(prim)) + offset - 5)       = '%';
        if(trc_LittleEndian)
        {
          *((char*)(P2D(prim)) + offset - 4)     = *((char *)&index);
          *((char*)(P2D(prim)) + offset - 3)     = *(((char *)&index)+1);
          *((char*)(P2D(prim)) + offset - 2)     = *(((char *)&index)+2);
          *((char*)(P2D(prim)) + offset - 1)     = *(((char *)&index)+3);
        }
        else
        {
          *((char*)(P2D(prim)) + offset - 4)     = *(((char *)&index)+3);
          *((char*)(P2D(prim)) + offset - 3)     = *(((char *)&index)+2);
          *((char*)(P2D(prim)) + offset - 2)     = *(((char *)&index)+1);
          *((char*)(P2D(prim)) + offset - 1)     = *((char *)&index);
        }
      }
      /*
       * parse the format string
       */
      paramCount = strlen(format);

      for (i=0; i<paramCount; i++)
      {
        switch(*(format+i))
        {
          case 'c': /* one byte */
            offset+=1;
            if(offset <= TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
               *((char*)(P2D(prim)) + offset - 1) = *((char *)varpars);

            va_arg(varpars, int);	/* int, not char because of promotion */
            break;
          case 'i': /* four bytes */
          case 'p':
          case '*':
            offset+=4;
            if(offset <= TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
            {
              if(trc_LittleEndian)
              {
                *((char*)(P2D(prim)) + offset - 4)     = *((char *)varpars);
                *((char*)(P2D(prim)) + offset - 3)     = *(((char *)varpars)+1);
                *((char*)(P2D(prim)) + offset - 2)     = *(((char *)varpars)+2);
                *((char*)(P2D(prim)) + offset - 1)     = *(((char *)varpars)+3);
              }
              else
              {
                *((char*)(P2D(prim)) + offset - 4)     = *(((char *)varpars)+3);
                *((char*)(P2D(prim)) + offset - 3)     = *(((char *)varpars)+2);
                *((char*)(P2D(prim)) + offset - 2)     = *(((char *)varpars)+1);
                *((char*)(P2D(prim)) + offset - 1)     = *((char *)varpars);
              }
            }
            va_arg(varpars, int);
            break;
          case 'd': /* eigth bytes */
            offset += 8;

            /*
             * TI and Microsoft represent double values differently
             *
                Double       Host representation (address incressing from byte 1 to 8)
                layout       Microsoft    TMS470x

               SEEEEEEE      byte 8       byte 4
               EEMMMMMM      byte 7       byte 3
               MMMMMMMM      byte 6       byte 2
               MMMMMMMM      byte 5       byte 1
               MMMMMMMM      byte 4       byte 8
               MMMMMMMM      byte 3       byte 7
               MMMMMMMM      byte 2       byte 6
               MMMMMMMM      byte 1       byte 5

               S - sign bit
               E - exponent bits
               M - mantissa bits

             *
             *            double
             */


            if(offset <= TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
            {
#ifdef _TARGET_
              /* TI TMS470 Compiler */
              *((char*)(P2D(prim)) + offset - 4)     = *((char *)varpars);
              *((char*)(P2D(prim)) + offset - 3)     = *(((char *)varpars)+1);
              *((char*)(P2D(prim)) + offset - 2)     = *(((char *)varpars)+2);
              *((char*)(P2D(prim)) + offset - 1)     = *(((char *)varpars)+3);
              *((char*)(P2D(prim)) + offset - 8)     = *(((char *)varpars)+4);
              *((char*)(P2D(prim)) + offset - 7)     = *(((char *)varpars)+5);
              *((char*)(P2D(prim)) + offset - 6)     = *(((char *)varpars)+6);
              *((char*)(P2D(prim)) + offset - 5)     = *(((char *)varpars)+7);
#else
             /*
              *  This should work as well, since no reordering is done.
              *  Don't believe the VC5/6 manual which states a complete
              *  different byte order :(
              *
              */
              /* PC- Simulation */
              *((char*)(P2D(prim)) + offset - 8)     = *((char *)varpars);
              *((char*)(P2D(prim)) + offset - 7)     = *(((char *)varpars)+1);
              *((char*)(P2D(prim)) + offset - 6)     = *(((char *)varpars)+2);
              *((char*)(P2D(prim)) + offset - 5)     = *(((char *)varpars)+3);
              *((char*)(P2D(prim)) + offset - 4)     = *(((char *)varpars)+4);
              *((char*)(P2D(prim)) + offset - 3)     = *(((char *)varpars)+5);
              *((char*)(P2D(prim)) + offset - 2)     = *(((char *)varpars)+6);
              *((char*)(P2D(prim)) + offset - 1)     = *(((char *)varpars)+7);
#endif /* _TARGET_ */
            }

            va_arg(varpars, double);
            break;
          case 's': /* a string of bytes */
            {
              /*
               * copy the string including the terminating NULL
               */
              unsigned int len = strlen(*((char **)varpars)) + 1;

              offset += len;
              if(offset <= TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
              {
                memcpy((char*)(P2D(prim)) + offset - len, *((char **)varpars), len);
              }
              va_arg(varpars, char **);
            }
            break;
          default:  /* unknown type */
            vsi_o_assert ( NO_TASK, OS_SYST_ERR_STR_TOO_LONG, __FILE__, __LINE__,
                           "Unknown Trace Format" );
            break;
        }
      }

      va_end (varpars);
      /*
       * if the amount of trace data was bigger than the allocated
       * size - discard the trace and send an error trace instead
       */
      if (offset > TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER))
      {
        unsigned int n = (unsigned int)sprintf ((char*)P2D(prim),"ERROR: Compressed trace (index %d) too long (%d bytes). Trace discarded !!!",index, offset);
        prim->len = n + sizeof(T_PRIM_HEADER);
      }
      else
      {
        prim->len = offset + sizeof(T_PRIM_HEADER);
      }

      prim->opc = int_vsi_tc2trace_opc(TraceClass);
      prim->sh_offset = S_HDR_OFFSET(prim->len);

      return ( vsi_o_trace ( Caller, prim, suspend ) );
    }
    if ( FrameEnv == ENV_STACK )
    {
      TracesAborted[Caller]++;
    }
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_ptrace        |
+--------------------------------------------------------------------+

  PURPOSE : traces a primitive opc and direction

*/
int vsi_o_ptrace ( T_HANDLE Caller, ULONG opc, UBYTE dir )
{
T_PRIM_HEADER *prim;
ULONG suspend;
LONG Status;
ULONG size;
int opc_len;

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_PRIM)
#ifdef _TOOLS_ 
        && (SAP_NR(opc)!=TRACE_SAP) && (opc!=TRACE_OPC)
#endif
     )
  {
    suspend = get_suspend_state(Caller,CHECK_TRC_SUSPEND);
    size = S_ALLOC_SIZE(PTRACE_LEN_OPC32);
    Status = os_AllocatePartition ( Caller, (T_VOID_STRUCT**)&prim, size, suspend, TestGroupHandle );
    if ( Status == OS_OK || Status == OS_WAITED || Status == OS_ALLOCATED_BIGGER )
    {
      prim->opc = int_vsi_tc2trace_opc(TC_PRIM);

      if ( dir )
        strcpy ( (char*)P2D(prim),"$---OUT:$p0x123456789" );
      else
        strcpy ( (char*)P2D(prim),"$--- IN:$p0x123456789" );

      if ( OPC32BIT(opc) )
      {
        opc_len = CHARS_FOR_32BIT;
        prim->len = PTRACE_LEN_OPC32;
      }
      else
      {
        opc_len = CHARS_FOR_16BIT;
        prim->len = PTRACE_LEN_OPC16;
      }
      prim->len = prim->len + sizeof(T_PRIM_HEADER);
      HexToASCII ( opc, (char*)P2D(prim) + 12, opc_len );
      strcpy (((char*)P2D(prim) + 12 + opc_len), "$" );

      prim->sh_offset = S_HDR_OFFSET(prim->len);
      return ( vsi_o_trace ( Caller, prim, suspend ) );
    }
    TracesAborted[Caller]++;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_strace        |
+--------------------------------------------------------------------+

  PURPOSE : traces a state and state transition

*/
int vsi_o_strace (T_HANDLE Caller,  const char *const machine,
                                      const char *const curstate,
                                      const char *const newstate)
{
T_PRIM_HEADER *prim;
LONG Status;
ULONG size;
ULONG suspend;

  if ( (Caller >= 0) && (TraceMask[Caller] & TC_STATE) )
  {
    suspend = get_suspend_state(Caller,CHECK_TRC_SUSPEND);
    size = S_ALLOC_SIZE(STRACE_LEN);
    Status = os_AllocatePartition ( Caller, (T_VOID_STRUCT**)&prim, size, suspend, TestGroupHandle );
    if ( Status == OS_OK || Status == OS_WAITED || Status == OS_ALLOCATED_BIGGER )
    {
      strcpy ( (char *)P2D(prim), machine );
      strcat ( (char *)P2D(prim), ":"           );
      strcat ( (char *)P2D(prim), curstate );
      if ( newstate != NULL )
      {
        strcat ( (char *)P2D(prim), " -> "        );
        strcat ( (char *)P2D(prim), newstate );
      }

      prim->opc = int_vsi_tc2trace_opc(TC_STATE);
      prim->len = strlen((char*)P2D(prim)) + sizeof(T_PRIM_HEADER);
      prim->sh_offset = S_HDR_OFFSET(prim->len);

      return ( vsi_o_trace ( Caller, prim, suspend ) );
    }
    TracesAborted[Caller]++;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_o_assert        |
+--------------------------------------------------------------------+

  PURPOSE : assert

*/
/*lint -esym(715,cause) suppress Info -- Symbol 'cause' not referenced */
int vsi_o_assert (T_HANDLE Caller, USHORT cause, const char *file, int line, const char * const format,...)
{
va_list varpars;
#ifdef _TARGET_
OS_QDATA Msg;

  while ( os_ReceiveFromQueue( Caller, pf_TaskTable[trc_hCommTrace].QueueHandle, &Msg, OS_NO_SUSPEND ) == OS_OK )
  {
    tst_pei_primitive (Msg.ptr);
  }
#endif
  strcpy ( TraceBuffer, "SYSTEM ERROR: " );
  va_start (varpars, format);
  vsprintf (TraceBuffer+strlen("SYSTEM ERROR: "), format, varpars);
  va_end (varpars);
  sprintf (TraceBuffer+strlen(TraceBuffer), ", %s(%d)", file, line );
#ifdef _TOOLS_
  vsi_o_ttrace ( NO_TASK, TC_SYSTEM, TraceBuffer );
#else
  if ( cause & OS_SYST_ERR )
    emergeny_trace = 1;
  tst_drv_write ( NO_TASK, 0, NULL, TraceBuffer );

  if ( error_ind_dst[0] != 0 )
  {
    frm_error_ind->error_code = cause;
    strncpy ((char*)frm_error_ind->error_string, TraceBuffer, FRM_PRIM_STR_SIZE);
    frm_error_ind->error_string[FRM_PRIM_STR_SIZE-1] = 0;
    tst_drv_write ( NO_TASK, FRM_ERROR_IND, error_ind_dst, (char*)frm_error_ind );
  }
#endif
#if defined _NUCLEUS_ && !defined _TARGET_
  printf ("%s\n",TraceBuffer);
  printf ( "Task %s suspended\n", pf_TaskTable[Caller].Name );
#endif
  if ( ext_trace_func.trace_assert != NULL )
  {
    /* in case of OS_SYST_ERR_QUEUE_FULL we should not to send the trace to ACI,
       because ACI will probably no longer be scheduled.
       in case of OS_SYST_ERR_NO_PARTITION the PALLOC in ACI fails and will
       probably hide the root cause of the problem.
    */
    if ( cause != OS_SYST_ERR_QUEUE_FULL && cause != OS_SYST_ERR_NO_PARTITION )
    {
      ext_trace_func.trace_assert ( format, varpars );
      os_SuspendTask ( Caller, 1000 );
    }
  }
#ifdef _NUCLEUS_
  os_SystemError ( os_MyHandle(), cause, TraceBuffer );
#endif
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_settracemask    |
+--------------------------------------------------------------------+

  PURPOSE : set trace mask

*/

int vsi_settracemask ( T_HANDLE Caller, T_HANDLE Handle, ULONG Mask )
{

  if ( Handle == 0 || pf_TaskTable[Handle].Name[0] != 0 )
  {
    TraceMask[Handle] = Mask;
    TraceMask[0] |= TC_SYSTEM;
    return VSI_OK;
  }
  else
    return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_gettracemask    |
+--------------------------------------------------------------------+

  PURPOSE : get trace mask

*/

int vsi_gettracemask ( T_HANDLE Caller, T_HANDLE Handle, ULONG *Mask )
{
  if ( Handle == 0 || pf_TaskTable[Handle].Name[0] != 0 )
  {
    *Mask = TraceMask[Handle];
    return VSI_OK;
  }
  else
    return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_trcsuspend      |
+--------------------------------------------------------------------+

  PURPOSE : set suspend for traces

*/

int vsi_trcsuspend ( T_HANDLE Caller, T_HANDLE Handle, ULONG Suspend )
{
  /* SuspendTrace[Handle] = Suspend; */
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : vsi_trc_free        |
+--------------------------------------------------------------------+

  PURPOSE : monitor test interface partition allocation
*/

int vsi_trc_free ( T_HANDLE Caller, T_VOID_STRUCT **Prim )
{
  if ( os_DeallocatePartition ( Caller, *Prim ) != OS_ERROR )
  {
    *Prim = NULL;
    return VSI_OK;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : InitializeTestpools |
+--------------------------------------------------------------------+

  PURPOSE : initialize supervision, write index to each partition.

*/
void InitializeTrace ( void )
{
USHORT i;
ULONG  ByteOrder;

  /*
   * test the byte order
   */
  ByteOrder = 0x11223344;

  if(*((char *) &ByteOrder) == 0x44)
    trc_LittleEndian = TRUE;
  else if (*((char *) &ByteOrder) == 0x11)
    trc_LittleEndian = FALSE;
  else
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_STR_TOO_LONG, __FILE__, __LINE__,
                   "Unknown Byte Order" );
  emergeny_trace = 0;
  trc_hCommTrace = VSI_ERROR;;
  for ( i = 0; i <= MaxEntities; i++ )
  {
    TracesAborted[i] = 0;
#ifdef _TARGET_
    TraceMask[i] =  TC_SYSTEM|TC_ERROR;
#else /* _TARGET_ */
    TraceMask[i] = 0xffffffff & ~TC_CCD;
#endif /* _TARGET_ */
  }
#ifdef _TARGET_
#endif
  ext_trace_func.magic_nr = 0;
  ext_trace_init();
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_TRC             |
| STATE   : code                       ROUTINE : get_suspend_state   |
+--------------------------------------------------------------------+

  PURPOSE : check if suspend is allowed.

*/
ULONG get_suspend_state ( T_HANDLE caller, int type )
{

  if ( caller != 0 )
  {
#ifdef _TARGET_
    if ( type == CHECK_TRC_SUSPEND )
    {
      if ( pf_TaskTable[caller].Flags & TRC_NO_SUSPEND )
      {
        return OS_NO_SUSPEND;
      }
      else
      {
        return OS_SUSPEND;
      }
    }
    else if ( type == CHECK_PRIM_SUSPEND )
    {
      if ( pf_TaskTable[caller].Flags & PRIM_NO_SUSPEND )
      {
        return OS_NO_SUSPEND;
      }
      else
      {
        return OS_SUSPEND;
      }
    }
    else
    {
      return OS_NO_SUSPEND;
    }
#else
    /*
       It is checked if the caller is a SYSTEM_PROCESS to ensure that 
       TST and RCV do not block while tracing to avoid a deadlock.
     */
    if ( pf_TaskTable[caller].Flags & SYSTEM_PROCESS )
      return OS_NO_SUSPEND;
    else
      return OS_SUSPEND;
#endif
  }
  else
  {
#ifdef _TARGET_
    return OS_NO_SUSPEND;
#else
    return OS_SUSPEND;
#endif
  }
}
#endif


/* -------------------------------------------------------------------------
   External Trace functions
----------------------------------------------------------------------------*/

#ifdef TEST_EXT_TRACE

#ifndef RUN_INT_RAM
void trace_error ( const char * const format, va_list varpars )
{
#ifndef _TARGET_
char buf[99];

   vsprintf (buf, format, varpars);
   printf ("%s\n",buf);
#endif
}
#endif

#ifndef RUN_INT_RAM
void trace_assert ( const char * const format, va_list varpars )
{
#ifndef _TARGET_
char buf[99];

   vsprintf (buf, format, varpars);
   printf ("%s\n",buf);
#endif
}
#endif

#endif /* TEST_EXT_TRACE */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ext_trace_register
+------------------------------------------------------------------------------
|  Description  :  register the external trace functions.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void vsi_ext_trace_register ( T_EXT_TRACE_FUNC * func )
{
  ext_trace_func.trace_error  = func->trace_error;
  ext_trace_func.trace_assert = func->trace_assert;
  ext_trace_func.magic_nr     = EXT_TRACE_INITIALIZED;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ext_trace_init
+------------------------------------------------------------------------------
|  Description  :  initialize external trace function pointer table.
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void ext_trace_init ( void )
{
#ifdef TEST_EXT_TRACE
  vsi_ext_trace_register ( &ext_trace_functions );
#endif
  if ( ext_trace_func.magic_nr != EXT_TRACE_INITIALIZED )
  {
  ext_trace_func.trace_error  = NULL;
  ext_trace_func.trace_assert = NULL;
  }
}
#endif





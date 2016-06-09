/* 
+------------------------------------------------------------------------------
|  File:       vsi_com.c
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
|             about communication via primitives and signals.
+----------------------------------------------------------------------------- 
*/ 

#undef TEST_PCHECK

#ifndef __VSI_COM_C__
#define __VSI_COM_C__
#endif

/*==== INCLUDES ===================================================*/

#include "gpfconf.h"
#include "typedefs.h"
#include "string.h"
#include "os.h"
#include "vsi.h"
#include "frame.h"
#include "vsi.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "route.h"

#ifdef MEMORY_SUPERVISION
 #include "tools.h"
#endif

#ifdef _TOOLS_
 #include "stdlib.h"
#endif

/*==== CONSTANTS ==================================================*/

#ifndef RUN_INT_RAM
char const *waited_queue_str= "Waited for space in queue";
char const *freed_sent_str  = "Freed partition sent";
char const *disc_str        = "Signal discarded";
char const *freed_str       = "Partition already freed";
char const *trunc_str       = "Allocation request truncated";
char const *guard_str       = "Partition guard pattern destroyed";
char const *unknown_str     = "unknown";
#else
extern char const *waited_queue_str;
extern char const *freed_sent_str;
extern char const *disc_str;
extern char const *freed_str;
extern char const *trunc_str;
extern char const *guard_str;
extern char const *unknown_str;
#endif

#if !defined _TARGET_ && !defined _TOOLS_
char const *pcheck_str     = "pcon_check() returned error";
#define PCHECK_INITIALIZED 0xaffedead
#endif


#define MAX_DRP_BOUND  12

/*==== TYPES ======================================================*/

#if !defined _TARGET_ && !defined _TOOLS_
typedef struct
{
  unsigned int magic_nr;
  ULONG ret_ok;
  ULONG (*pcheck)(ULONG opc, void * decoded_prim);      
} T_PCHECK;
#endif

/*==== EXTERNALS ==================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED")
#endif


#ifdef _TOOLS_
  extern char FRM_TST_NAME[];
 __declspec (dllimport) T_HANDLE TST_Handle;
#else
 extern T_HANDLE TST_Handle;
#endif

extern T_HANDLE vsi_m_sem_handle;
extern char TaskName [];
//extern OS_HANDLE ext_data_pool_handle;
#if defined _NUCLEUS_ && defined NU_DEBUG
 extern char check_desclist;
#endif

/*==== VARIABLES ==================================================*/

#if !defined _TARGET_ && !defined _TOOLS_
T_PCHECK pcheck_func; 
#endif /* _TARGET_ */

#ifndef RUN_INT_RAM
char QueueName [ RESOURCE_NAMELEN ];
//char *pf_com_matrix = NULL;
#else
extern char QueueName [];
#endif

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

/*==== PROTOTYPES =================================================*/

#if !defined _TARGET_ && !defined _TOOLS_
#ifdef TEST_PCHECK
ULONG test_pcheck ( ULONG opc, void * decoded_prim );
#endif
#endif /* _TARGET_ */


#if defined _NUCLEUS_ && defined NU_DEBUG
 int check_descriptor_list ( T_HANDLE caller, T_PRIM_HEADER *prim FILE_LINE_TYPE );
#endif

int int_vsi_c_pattach (T_VOID_STRUCT *prim FILE_LINE_TYPE);


/*==== FUNCTIONS ==================================================*/

#if 0
        not needed -> temporarily removed 
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_open          |
+--------------------------------------------------------------------+

  PURPOSE : get the handle of a queue

*/

char * vsi_c_init_com_matrix ( int max_entities )
{
int size;

  size = (max_entities+1)*(max_entities+1);
  if ( os_AllocateMemory ( NO_TASK, (T_VOID_STRUCT**)&pf_com_matrix, size, OS_NO_SUSPEND, ext_data_pool_handle ) != OS_OK )
  {
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_NO_MEMORY, __FILE__, __LINE__, "No memory available for com matrix");
  }
  return pf_com_matrix;
}
#endif


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM                   |
| STATE   : code                       ROUTINE : vsi_c_get_com_matrix_entry|
+--------------------------------------------------------------------------+

  PURPOSE : get an entry of the com matrix

*/

int vsi_c_get_com_matrix_entry ( int entry, char *dst )
{
T_HANDLE snd, rcv;
static int cnt;
int size_of_matrix = (MaxEntities+1)*(MaxEntities+1);
int snd_len;

  if ( entry == FIRST_ENTRY )
  {
    cnt = 0;
  }
  while ( cnt <= size_of_matrix )
  {
    if ( pf_com_matrix[cnt] != 0 )
    {
      snd = cnt/(MaxEntities+1);
      vsi_e_name (NO_TASK, snd, &dst[0]);
      snd_len = strlen(&dst[0]);
      dst[snd_len]=' ';
      rcv = cnt%(MaxEntities+1);
      vsi_e_name (NO_TASK, rcv, &dst[snd_len+1]);
      cnt++;
      return VSI_OK;
    }
    cnt++;
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM                   |
| STATE   : code                       ROUTINE : vsi_c_get_entity_com_entry|
+--------------------------------------------------------------------------+

  PURPOSE : get a entity handle the is used by an entity to send primitives

*/

int vsi_c_get_entity_com_entry ( int entry, T_HANDLE rcv, T_HANDLE *snd )
{
static int cnt;
int size_of_matrix = (MaxEntities+1)*(MaxEntities+1);

  if ( entry == FIRST_ENTRY )
  {
    cnt = rcv;
  }
  while ( cnt <= size_of_matrix )
  {
    if ( pf_com_matrix[cnt] != 0 )
    {
      *snd = cnt / (MaxEntities + 1);
      cnt = cnt + MaxEntities + 1;
      return VSI_OK;
    }
    cnt = cnt + MaxEntities + 1;
  }
  return VSI_ERROR;
}
#endif
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_open          |
+--------------------------------------------------------------------+

  PURPOSE : get the handle of a queue

*/

T_HANDLE vsi_c_open (T_HANDLE Caller, char *Name)
{
#ifdef _TOOLS_
OS_HANDLE ComHandle;

  if ( os_OpenQueue ( Caller, Name, &ComHandle ) != OS_ERROR )
    return ComHandle;
#else
T_HANDLE e_handle;

  for ( e_handle = MaxEntities; e_handle > 0; e_handle-- )
  {
      if ( pf_TaskTable[e_handle].Name[0] != 0 
        && pf_TaskTable[e_handle].QueueHandle != 0 
        && !strncmp (pf_TaskTable[e_handle].Name, Name, RESOURCE_NAMELEN-1) )
      {
        /* 
        not needed -> temporarily removed 
        pf_com_matrix[Caller*(MaxEntities+1)+e_handle] = 1;
        */
        return e_handle;
      }
  }
#endif
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_close         |
+--------------------------------------------------------------------+

  PURPOSE : return the handle of a queue

*/

int vsi_c_close (T_HANDLE Caller, T_HANDLE ComHandle)
{

  if ( os_CloseQueue ( Caller, ComHandle ) != OS_ERROR )
    return VSI_OK;

  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_clear         |
+--------------------------------------------------------------------+

  PURPOSE : read all messages from a queue

*/

int vsi_c_clear (T_HANDLE Caller, T_HANDLE ComHandle)
{
OS_QDATA Msg;

  while ( os_ReceiveFromQueue ( NO_TASK, ComHandle, &Msg, OS_NO_SUSPEND ) == OS_OK )
  {
    ;
  }

  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_send          |
+--------------------------------------------------------------------+

  PURPOSE : write a message to a queue

*/

int vsi_c_send (T_HANDLE Caller, T_HANDLE ComHandle, T_QMSG *Msg FILE_LINE_TYPE)
{
OS_QDATA OS_Msg = { 0 };
LONG Status;
USHORT Prio;
#ifdef NU_DEBUG
T_PRIM_HEADER *prim;
 #ifdef _TARGET_
  T_HANDLE t_handle;
 #endif
#endif

  OS_Msg.data16 = Msg->MsgType;
  OS_Msg.e_id = ComHandle;
  switch ( Msg->MsgType )
  {
  case MSG_PRIMITIVE:
    OS_Msg.ptr = Msg->Msg.Primitive.Prim;
    OS_Msg.data32 = P_OPC(Msg->Msg.Primitive.Prim);
#ifdef _TOOLS_
    OS_Msg.len = Msg->Msg.Primitive.PrimLen;
#endif
    os_GetTime ( 0, &OS_Msg.time );
    Prio = MSG_PRIMITIVE_PRIO;
    break;
  case MSG_SIGNAL:
    OS_Msg.ptr = Msg->Msg.Signal.SigBuffer;
    OS_Msg.data32 = Msg->Msg.Signal.SigOPC;
#ifdef _TOOLS_
    OS_Msg.len = Msg->Msg.Signal.SigLen;
#endif
    os_GetTime ( 0, &OS_Msg.time );
    Prio = MSG_SIGNAL_PRIO;
    break;
  default: return VSI_ERROR;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
  }

#ifdef _NUCLEUS_
#ifdef NU_DEBUG
  /* PARTITION GUARD PATTERN CHECK */
  if ( Msg->MsgType == MSG_PRIMITIVE )
  {
    prim = (T_PRIM_HEADER*)Msg->Msg.Primitive.Prim;
    if ( (Status = os_PartitionCheck ( (T_VOID_STRUCT*)prim )) == OS_OK )
    {
      if ( check_desclist == TRUE && prim->dph_offset != 0 )
      {
        check_descriptor_list ( Caller, prim FILE_LINE );
      }
    }
    else
    {
      switch ( Status )
      {
        case OS_PARTITION_FREE:
          vsi_o_assert ( Caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                         "%s (PSEND),entity %s, partition 0x%x, opc 0x%x",
                         freed_sent_str, pf_TaskTable[Caller].Name, prim, prim->opc );
        break;
        case OS_PARTITION_GUARD_PATTERN_DESTROYED:
          vsi_o_assert ( Caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                         "%s (PSEND), entity %s, partition 0x%x, opc 0x%x",
                         guard_str, pf_TaskTable[Caller].Name, prim, prim->opc );
        break;
        default:
        break;
      }
    }
#ifdef MEMORY_SUPERVISION
    vsi_ppm_send ( Caller, ComHandle, (T_PRIM_HEADER*)Msg->Msg.Primitive.Prim, file, line );
#endif /* MEMORY_SUPERVISION */
  }
#ifdef _TARGET_
  if ( (t_handle = os_MyHandle()) != 0 )
  {
    int opc;
    switch ( os_CheckTaskStack ( t_handle ) )
    {
      case OS_ERROR:
        if ( Msg->MsgType == MSG_PRIMITIVE )
        {
          opc = ((T_PRIM_HEADER*)Msg->Msg.Primitive.Prim)->opc;
        }
        else
        {
          opc = Msg->Msg.Signal.SigOPC;
        }
        vsi_o_assert ( Caller, OS_SYST_ERR_STACK_OVERFLOW FILE_LINE_MACRO_PASSED, 
                       "%s Stack overflow, 0x%x", pf_TaskTable[Caller].Name, opc );
        break;
      default:
      break;
    }
  }
#endif /* _TARGET_ */
#endif /* NU_DEBUG */
#endif /* _NUCLEUS_ */

  Status = rt_Route ( Caller, ComHandle, Prio, OS_SUSPEND, &OS_Msg );

  switch ( Status )
  {
    case OS_WAITED:
#ifdef NU_DEBUG
      pf_handle_warning ( OS_SYST_WRN_WAIT_QUEUE, "%s %s, entity %s, queue %s, %s(%d)",
                          syst_wrn, waited_queue_str, pf_TaskTable[Caller].Name, pf_TaskTable[ComHandle].Name FILE_LINE_MACRO_PASSED );
#endif
      return VSI_OK;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
    case OS_TIMEOUT:
    case OS_ERROR:
    case OS_INVALID_QUEUE:
      Caller = e_running[os_MyHandle()];
      if ( Msg->MsgType == MSG_SIGNAL )
      {
#ifdef NU_DEBUG
        pf_handle_warning ( OS_SYST_WRN_WAIT_QUEUE, "%s %s from %s to %s, opc 0x%x, %s(%d)", 
                            syst_wrn, disc_str, pf_TaskTable[Caller].Name, pf_TaskTable[ComHandle].Name, Msg->Msg.Signal.SigOPC FILE_LINE_MACRO_PASSED );
#endif
        return VSI_OK;
      }
      else
      {
        char const *p_queue_name;
        if ( Status == OS_INVALID_QUEUE )
        {
          p_queue_name = unknown_str;
        }
        else
        {
#ifdef _TOOLS_
          os_GetQueueName ( Caller, ComHandle, QueueName );
          p_queue_name = QueueName;
#else
          p_queue_name = pf_TaskTable[ComHandle].Name;
#endif
        }
        vsi_o_assert ( Caller, OS_SYST_ERR_QUEUE_FULL FILE_LINE_MACRO_PASSED,
                       "%s write attempt to %s queue failed", 
                       pf_TaskTable[Caller].Name, p_queue_name );
#ifdef MEMORY_SUPERVISION
        vsi_ppm_free ( Caller, (T_PRIM_HEADER*)(OS_Msg.ptr-PPM_OFFSET), file, line);
#endif
        os_DeallocatePartition (Caller, OS_Msg.ptr-PPM_OFFSET );
        return VSI_ERROR;
      }
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
    case OS_OK:
      return VSI_OK;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
    default:
      return VSI_ERROR;
    /*lint -e527 suppress Warning -- Unreachable */
    break;
    /*lint +e527 */
  }

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_psend         |
+--------------------------------------------------------------------+

  PURPOSE : wrapper for vsi_c_send to avoid code wasting macro

*/
int vsi_c_psend_caller ( T_HANDLE Caller, T_HANDLE ComHandle, T_VOID_STRUCT *ptr FILE_LINE_TYPE )
{
T_QMSG QMsg;
T_VOID_STRUCT *snd_ptr = (T_VOID_STRUCT*)D2P(ptr);

  QMsg.Msg.Primitive.Prim = snd_ptr;
#ifdef _TOOLS_
  if ( ((T_PRIM_HEADER*)snd_ptr)->sh_offset != 0 ) 
    QMsg.Msg.Primitive.PrimLen = ALIGN(PSIZE(ptr)) + sizeof(T_S_HEADER);
  else
#endif
    QMsg.Msg.Primitive.PrimLen = PSIZE(ptr);
  QMsg.MsgType = MSG_PRIMITIVE;
  if ( Caller != TST_Handle && !(P_OPC(QMsg.Msg.Primitive.Prim) & SYS_MASK ) )
    vsi_o_ptrace (Caller, P_OPC(QMsg.Msg.Primitive.Prim), 1);

#if !defined _TARGET_ && !defined _TOOLS_
  if ( (pcheck_active[Caller] == 1) && (pcheck_func.pcheck != NULL) )
  {
    ULONG ret;
    if ( (ret = pcheck_func.pcheck ( D_OPC(ptr), ptr )) != pcheck_func.ret_ok )
    {
      pf_handle_warning ( OS_SYST_WRN, "%s %s %d in %s, opc 0x%x, %s(%d)", 
                        syst_wrn, pcheck_str, ret, pf_TaskTable[Caller].Name, D_OPC(ptr) FILE_LINE_MACRO_PASSED );
    }
  }
#endif
  return ( vsi_c_send ( Caller, ComHandle, &QMsg FILE_LINE) );
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_psend         |
+--------------------------------------------------------------------+

  PURPOSE : wrapper for vsi_c_send to avoid code wasting macro

*/
int vsi_c_psend ( T_HANDLE ComHandle, T_VOID_STRUCT *ptr FILE_LINE_TYPE )
{
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  return ( vsi_c_psend_caller ( Caller, ComHandle, ptr FILE_LINE) );
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_ssend         |
+--------------------------------------------------------------------+

  PURPOSE : wrapper for vsi_c_send to avoid code wasting macro

*/
int vsi_c_ssend ( T_HANDLE ComHandle, ULONG opc, T_VOID_STRUCT *ptr, 
                    ULONG MsgLen FILE_LINE_TYPE )
{
T_QMSG QMsg;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];

  QMsg.Msg.Signal.SigBuffer = (T_VOID_STRUCT*)ptr;
  QMsg.Msg.Signal.SigOPC = opc;
  QMsg.Msg.Signal.SigLen = MsgLen;
  QMsg.MsgType = MSG_SIGNAL;

  return ( vsi_c_send ( Caller, ComHandle, &QMsg FILE_LINE ) );

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_new           |
+--------------------------------------------------------------------+

  PURPOSE : allocate a partition to send a primitive

*/

T_VOID_STRUCT * vsi_c_new (T_HANDLE Caller, ULONG Size, ULONG opc FILE_LINE_TYPE)
{
T_VOID_STRUCT *prim;
ULONG flags;

  /* VSI_MEM_NON_BLOCKING not set, blocking allocation for backwards compatibility */
  flags = PrimGroupHandle;    
  prim = (T_VOID_STRUCT*)D2P(vsi_c_pnew_generic (Caller, Size, opc, flags FILE_LINE));
  return prim;

}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_new           |
+--------------------------------------------------------------------+

  PURPOSE : allocate a partition to send a primitive

*/

T_VOID_STRUCT * vsi_c_pnew_generic (T_HANDLE Caller, ULONG Size, ULONG opc, ULONG flags FILE_LINE_TYPE)
{
T_VOID_STRUCT *prim;

  if ( Size < sizeof(T_PRIM_HEADER) )
    Size = sizeof(T_PRIM_HEADER);

  if ( (prim = vsi_m_new ( Size, flags FILE_LINE )) != NULL )
  {
    P_OPC(prim)   = opc;
    P_LEN(prim)   = Size;
    P_SDU(prim)   = NULL;    
    P_CNT(prim)   = 1;
    P_SHO(prim)   = 0;
    P_DPHO(prim)  = 0;
#ifdef MEMORY_SUPERVISION
    Caller = e_running[os_MyHandle()];
    vsi_ppm_new ( Caller, Size, (T_PRIM_HEADER*)prim, file, line );
#endif

#ifndef _TOOLS_
    if (opc & MEMHANDLE_OPC)
    {
      P_MEMHANDLE_SDU(prim)=0x00000000;
    }
#endif

    return (T_VOID_STRUCT*)P2D(prim);
  }
  return NULL;

}
#endif
#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pnew          |
+--------------------------------------------------------------------+

  PURPOSE : allocate a partition to send a primitive

*/

T_VOID_STRUCT * vsi_c_pnew (ULONG Size, ULONG opc FILE_LINE_TYPE)
{
T_HANDLE Caller;

  Caller = 0;

  Size += sizeof(T_PRIM_HEADER);

  return ( (T_VOID_STRUCT*)vsi_c_pnew_generic ( Caller, Size, opc, PrimGroupHandle FILE_LINE ));
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pnew          |
+--------------------------------------------------------------------+

  PURPOSE : allocate a partition to send a primitive

*/

T_VOID_STRUCT * vsi_c_pnew_nb (ULONG Size, ULONG opc FILE_LINE_TYPE)
{
T_HANDLE Caller;

  Caller = 0;

  Size += sizeof(T_PRIM_HEADER);

  return ( (T_VOID_STRUCT*)vsi_c_pnew_generic ( Caller, Size, opc, VSI_MEM_NON_BLOCKING|PrimGroupHandle FILE_LINE ));
}
#endif

#ifndef RUN_FLASH
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM              |
| STATE   : code                       ROUTINE : vsi_c_new_sdu_generic|
+---------------------------------------------------------------------+

  PURPOSE : allow pool selection and flags for blocking bahavior

*/
T_VOID_STRUCT * vsi_c_new_sdu_generic (ULONG Size, ULONG opc, USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset, ULONG flags FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
ULONG alloc_size;
T_HANDLE Caller;

  Caller = 0;
  alloc_size = Size + sizeof(T_PRIM_HEADER) + BYTELEN((SHORT)sdu_len + (SHORT)encode_offset);
  if ( (prim = (T_PRIM_HEADER*)vsi_c_pnew_generic (Caller, alloc_size, opc, flags FILE_LINE)) != NULL )
  {
    D_SDU(prim)     = (T_sdu*)(((char*)prim) + sdu_offset);
    D_SDU_LEN(prim) = sdu_len;
    D_SDU_OFF(prim) = encode_offset;
    return ( (T_VOID_STRUCT*)prim );
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
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_new_sdu       |
+--------------------------------------------------------------------+

  PURPOSE : wrapper for vsi_c_new to avoid code wasting macro

*/
T_VOID_STRUCT * vsi_c_new_sdu (ULONG Size, ULONG opc, USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset FILE_LINE_TYPE )
{
ULONG flags;

  flags = PrimGroupHandle;    

  return vsi_c_new_sdu_generic (Size, opc, sdu_len, sdu_offset, encode_offset, flags FILE_LINE);
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_ppass         |
+--------------------------------------------------------------------+

  PURPOSE : pass a partition from one primitive to another

*/
T_VOID_STRUCT * vsi_c_ppass (T_VOID_STRUCT *prim, ULONG opc FILE_LINE_TYPE )
{
T_VOID_STRUCT *ptr;
ULONG len;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];
  if ( D_CNT(prim) > 1 )
  {
    /*
     * This does not work for dynamic primitive containing pointers, PDUP needed !!!!
     * The sdu pointer is currently not set correctly because it is never used !!!
     */
    len = D_LEN(prim); 
    ptr = vsi_c_pnew ( len, opc FILE_LINE );
    memcpy ( ptr, prim, len - sizeof(T_PRIM_HEADER) );
#ifdef PRIM_AUTO_FREE
    if ( !(pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE) )
#endif
      vsi_c_pfree ( &prim FILE_LINE );
    return ptr;
  }
  else
  {
    D_OPC(prim) = opc;
#ifdef PRIM_AUTO_FREE
    if ( pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
    {
      D_CNT(prim)++;
    }
#endif
#ifdef MEMORY_SUPERVISION
    vsi_ppm_reuse ( Caller, D2P(prim), file, line);
#endif
    return prim;
  }

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pstore        |
+--------------------------------------------------------------------+

  PURPOSE : store a partition (increment the reference counter)
            consider PARTITION_AUTO_FREE

*/
void vsi_c_pstore ( T_VOID_STRUCT *prim FILE_LINE_TYPE )
{
#ifdef PRIM_AUTO_FREE
T_PRIM_HEADER *ptr;
T_DP_HEADER *dp_hdr;
T_HANDLE Caller;

  Caller = e_running[os_MyHandle()];
  if ( pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
  {
    ptr = D2P(prim);
    /* take control -> enable entity to free the prim */
    processed_prim[Caller] = NULL;
    /* increment reference counter */
    D_CNT(prim)++;
#ifdef MEMORY_SUPERVISION
    vsi_ppm_store ( Caller, ptr, file, line ); 
#endif
    if ( P_DPHO(ptr) != 0 )
    {
      dp_hdr = (T_DP_HEADER*)((ULONG*)ptr + ptr->dph_offset);
      dp_hdr = (T_DP_HEADER*)dp_hdr->next;
      while ( (ptr = (T_PRIM_HEADER*)dp_hdr) != NULL )
      {
#ifdef MEMORY_SUPERVISION
        vsi_ppm_store ( Caller, ptr, file, line ); 
#endif
        P_CNT(ptr)++;
        if ( dp_hdr->magic_nr != GUARD_PATTERN )
        { 
          vsi_o_assert ( Caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                       "Magic number in dp_header destroyed (PSTORE) %s , opc: 0x%lx, partition 0x%lx",
                        pf_TaskTable[Caller].Name, ptr->opc, ptr );
        }
        dp_hdr = (T_DP_HEADER*)dp_hdr->next;
      } 
    }
  }
#endif /* PRIM_AUTO_FREE */
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pattach       |
+--------------------------------------------------------------------+

  PURPOSE : call internal function to store a partition (increment the reference counter)

*/
int vsi_c_pattach ( T_VOID_STRUCT *prim FILE_LINE_TYPE )
{
T_HANDLE Caller = 0;
LONG sts;
int ret;

  sts = os_ObtainSemaphore (Caller, vsi_m_sem_handle, OS_SUSPEND);
  if ( sts == OS_ERROR || sts == OS_TIMEOUT )
  {
    /* Semaphore invalid or overrun */
    Caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "Ref Cnt Semaphore overrun, entity %s", pf_TaskTable[Caller].Name );
  }

  ret=int_vsi_c_pattach(prim FILE_LINE_MACRO);

  os_ReleaseSemaphore (Caller, vsi_m_sem_handle);

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : int_vsi_c_pattach   |
+--------------------------------------------------------------------+

  PURPOSE : actually store a partition (increment the reference counter)

*/
int int_vsi_c_pattach ( T_VOID_STRUCT *prim FILE_LINE_TYPE)
{
T_PRIM_HEADER *ptr;
T_DP_HEADER *dp_hdr;
T_HANDLE Caller = 0;
int pos;

  ptr = D2P(prim);

#ifdef NU_DEBUG
  if ( os_is_valid_partition ((T_VOID_STRUCT*)ptr) )
  {
    /* attach to non-partition memory */
    Caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "PATTACH to non-partition memory, entity %s, ptr 0x%x", pf_TaskTable[Caller].Name, ptr );
  }
#endif

  if ( ptr->use_cnt <= 0 )
  {
    /* attach to non allocated memory */
    Caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "PATTACH to free memory, entity %s, ptr 0x%x", pf_TaskTable[Caller].Name, ptr );
  }

  dp_hdr=NULL;
 
  /* check if we have a primitive pointer */
  if ( ((T_DP_HEADER*)ptr)->magic_nr != GUARD_PATTERN )
  {
    /* increment reference counter */
    P_CNT(ptr)++;
  #ifdef MEMORY_SUPERVISION
    vsi_ppm_store ( Caller, ptr, file, line ); 
  #endif

    /* look for dynamic partition header */
    if ( P_DPHO(ptr) != 0 )
    {
      dp_hdr = (T_DP_HEADER*)((ULONG*)ptr + ptr->dph_offset);
      if (dp_hdr->drp_bound_list)
      {
        /* call attach for bound root pointers */
        pos=0;
        while(pos<MAX_DRP_BOUND && dp_hdr->drp_bound_list[pos])
        {
          int_vsi_c_pattach(dp_hdr->drp_bound_list[pos] FILE_LINE_MACRO);
          pos++;
        }
      }

      dp_hdr = (T_DP_HEADER*)dp_hdr->next;
    }
  }
  else
  {
    dp_hdr=(T_DP_HEADER*)ptr;
  }

  if ( dp_hdr )
  {
    if ( dp_hdr->magic_nr != GUARD_PATTERN )
    { 
      /* primitive with T_desc_list element, use MATTACH */
      os_ReleaseSemaphore (Caller, vsi_m_sem_handle);
      return VSI_OK;
    }
    else
    {
      while ( (ptr = (T_PRIM_HEADER*)dp_hdr) != NULL )
      {
#ifdef MEMORY_SUPERVISION
        vsi_ppm_store ( Caller, ptr, file, line ); 
#endif
        P_CNT(ptr)++;
        if ( dp_hdr->magic_nr != GUARD_PATTERN )
        { 
          vsi_o_assert ( Caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                       "Magic number in dp_header destroyed (PATTACH), %s, opc: 0x%lx, partition 0x%lx",
                        pf_TaskTable[Caller].Name, ptr->opc, ptr );
        }

        if (dp_hdr->drp_bound_list)
        {
          /* call attach for bound root pointers */
          pos=0;
          while(pos<MAX_DRP_BOUND && dp_hdr->drp_bound_list[pos])
          {
            int_vsi_c_pattach(dp_hdr->drp_bound_list[pos] FILE_LINE_MACRO);
            pos++;
          }
        }
        dp_hdr = (T_DP_HEADER*)dp_hdr->next;
      } 
    }
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_reuse         |
+--------------------------------------------------------------------+

  PURPOSE : function to avoid code wasting macro

*/
T_VOID_STRUCT *vsi_c_reuse ( T_PRIM_HEADER *ptr, ULONG Size, ULONG opc,
                   USHORT sdu_len, USHORT sdu_offset, USHORT encode_offset FILE_LINE_TYPE )
{
T_HANDLE Caller;

  D_OPC(ptr)   = opc;
  D_LEN(ptr)   = Size;
  if ( sdu_offset != NO_SDU )
  {
    D_SDU(ptr) = (T_sdu*)((char*)(ptr) + sdu_offset);
    D_SDU_LEN(ptr) = sdu_len;
    D_SDU_OFF(ptr) = encode_offset;
  }
  else
    D_SDU(ptr) = NULL;

  Caller = e_running[os_MyHandle()];
#ifdef MEMORY_SUPERVISION
  vsi_ppm_reuse ( Caller, D2P(ptr), file, line);
#endif
#ifdef PRIM_AUTO_FREE
  if ( pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
  {
    D_CNT(ptr)++;
  }
#endif
  return ( (T_VOID_STRUCT*)ptr );
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_free          |
+--------------------------------------------------------------------+

  PURPOSE : deallocate a partition that was used to send a primitive

*/

int vsi_c_free (T_HANDLE Caller, T_VOID_STRUCT **Msg FILE_LINE_TYPE)
{
static T_VOID_STRUCT *protected_prim_to_free = NULL;
#if defined (NU_DEBUG) || defined (OSL_DEBUG)
LONG count;
#endif

//LONG sts;

#if 0
  sts = os_ObtainSemaphore (Caller, vsi_m_sem_handle, OS_SUSPEND);
  if ( sts == OS_ERROR || sts == OS_TIMEOUT )
  {
    /* Semaphore invalid or overrun */
    if ( *Msg == protected_prim_to_free )
    {
      /* fatal error only if semaphore overrun on same primitive */
      Caller = e_running[os_MyHandle()];
      vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                     "Ref Cnt Semaphore overrun, entity %s", pf_TaskTable[Caller].Name );
      return VSI_ERROR;
    }
  }
  else
#endif
  {
    protected_prim_to_free = *Msg;
  }
#if defined (NU_DEBUG) || defined (OSL_DEBUG)
  count = (LONG)((T_PRIM_HEADER*)*Msg)->use_cnt;
  if ( count <= 0 )
  {
    pf_handle_warning ( OS_SYST_WRN_MULTIPLE_FREE, "%s %s in %s, 0x%x, %s(%d)", 
                        syst_wrn, freed_str, pf_TaskTable[Caller].Name, P_OPC(*Msg) FILE_LINE_MACRO_PASSED );
    protected_prim_to_free = NULL;
 //   os_ReleaseSemaphore (Caller, vsi_m_sem_handle);
    return VSI_OK;
  }
#endif
  if ( --((T_PRIM_HEADER*)*Msg)->use_cnt == 0 )
  {
#ifdef PRIM_AUTO_FREE
    if ( pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
      freed_prim[Caller] = *Msg;
#endif
#ifdef _NUCLEUS_
#ifdef NU_DEBUG

    if ( os_PartitionCheck( (ULONG*)*Msg ) == OS_PARTITION_GUARD_PATTERN_DESTROYED )
    {
//      os_ReleaseSemaphore (Caller, vsi_m_sem_handle);
      vsi_o_assert ( Caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                     "%s (PFREE), entity %s,Partition 0x%x",
                     guard_str, pf_TaskTable[Caller].Name, *Msg );
      return VSI_ERROR;
    }
#endif
#endif
    protected_prim_to_free = NULL;
//    os_ReleaseSemaphore (Caller, vsi_m_sem_handle);
    return ( vsi_m_free ( Msg FILE_LINE ) );
  }

  protected_prim_to_free = NULL;
//  os_ReleaseSemaphore (Caller, vsi_m_sem_handle);
  return VSI_OK;
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pfree         |
+--------------------------------------------------------------------+

  PURPOSE : deallocate a partition that was used to send a primitive

*/

int vsi_c_pfree (T_VOID_STRUCT **Msg FILE_LINE_TYPE)
{
T_VOID_STRUCT *free_ptr;
T_HANDLE Caller;

  /* 
   * PFREE is disabled if the primitive to be freed is the currently 
   * processed one and the auto free is enabled for the calling entity 
   */

  Caller = e_running[os_MyHandle()];
  free_ptr = (T_VOID_STRUCT*)D2P(*Msg);
#ifdef NU_DEBUG
  if ( os_is_valid_partition ((T_VOID_STRUCT*)free_ptr) )
  {
    /* free to non-partition memory */
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "PFREE to non-partition memory, entity %s, prim 0x%x", pf_TaskTable[Caller].Name, *Msg );
  }
#endif
#ifdef PRIM_AUTO_FREE
  if ( free_ptr == processed_prim[Caller] && pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
  {
    return VSI_OK;
  }
#endif /* PRIM_AUTO_FREE */
  return ( vsi_c_free ( Caller, &free_ptr FILE_LINE ) );
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_await         |
+--------------------------------------------------------------------+

  PURPOSE : receive a primitive

*/

int vsi_c_await (T_HANDLE Caller, T_HANDLE ComHandle, T_QMSG *Msg, ULONG Timeout)
{
OS_QDATA OS_Msg;
LONG ret;
OS_HANDLE q_handle;


#ifdef _TOOLS_
  q_handle = ComHandle;
#else
  q_handle = pf_TaskTable[ComHandle].QueueHandle;
#endif
  e_running[os_MyHandle()] = 0;
  
  if ( (ret = os_ReceiveFromQueue ( Caller, q_handle, &OS_Msg, Timeout)) == OS_OK )
  {
    Msg->MsgType = OS_Msg.data16;
    switch ( Msg->MsgType )
    {
    case MSG_PRIMITIVE:
      Msg->Msg.Primitive.Prim = OS_Msg.ptr;
#ifdef _TOOLS_
      Msg->Msg.Primitive.PrimLen = OS_Msg.len;
#endif
      vsi_o_ptrace (Caller, ((T_PRIM_HEADER*)Msg->Msg.Primitive.Prim)->opc, 0);
      break;
    case MSG_SIGNAL:
      Msg->Msg.Signal.SigBuffer = OS_Msg.ptr;
      Msg->Msg.Signal.SigOPC = OS_Msg.data32;
#ifdef _TOOLS_
      Msg->Msg.Signal.SigLen = OS_Msg.len;
#endif
      break;
    case MSG_TIMEOUT:
      if ( *(pf_TaskTable[Caller].FirstTimerEntry + OS_Msg.data32) & TIMEOUT_OCCURRED )
      {
        if ( !(*(pf_TaskTable[Caller].FirstTimerEntry + OS_Msg.data32) & PERIODIC_TIMER) )
        {
          os_DestroyTimer ( Caller, (OS_HANDLE)(*(pf_TaskTable[Caller].FirstTimerEntry + OS_Msg.data32) & TIMER_HANDLE_MASK) );
          *(pf_TaskTable[Caller].FirstTimerEntry + OS_Msg.data32) = 0;
        }
        else
        {
          *(pf_TaskTable[Caller].FirstTimerEntry + OS_Msg.data32) &= ~TIMEOUT_OCCURRED;
        }
        Msg->Msg.Timer.Index = OS_Msg.data32;
      }
      break;
    default: return VSI_ERROR;
      /*lint -e527 suppress Warning -- Unreachable */
      break;
      /*lint +e527 */
    }
    e_running[os_MyHandle()] = Caller;
    prf_log_entity_activate ((void*)Caller);
    return VSI_OK;
  }
  else
  {
    if ( ret == OS_TIMEOUT )
    {
      e_running[os_MyHandle()] = Caller;
      return VSI_TIMEOUT;
    }
  }
  return VSI_ERROR;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_primitive     |
+--------------------------------------------------------------------+

  PURPOSE : send a non GSM primitive to the frame

*/

int vsi_c_primitive (T_HANDLE Caller, void *Msg)
{
  /*
   * the following line of code causes a warning on tms470 compiler, that cannot be avoided
   * without changing all entities PEI modules. Warning will not cause a problem
   */
  pf_ProcessSystemPrim ( Caller, Msg );
  return VSI_OK;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_awake         |
+--------------------------------------------------------------------+

  PURPOSE : send NULL primitive to itself

*/
GLOBAL int vsi_c_awake ( T_HANDLE caller )
{
OS_QDATA QMsg = { 0 };

  QMsg.data16 = MSG_PRIMITIVE;
  QMsg.ptr = NULL;
  os_SendToQueue ( caller, caller, OS_URGENT, OS_NO_SUSPEND, &QMsg );
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_status        |
+--------------------------------------------------------------------+

  PURPOSE : allocate root of dynamic sized primitive

*/
int vsi_c_status (T_HANDLE handle, unsigned int *used, unsigned int *free)
{
#ifdef _NUCLEUS_
int status;

  if ( (status = os_GetQueueState (0, pf_TaskTable[handle].QueueHandle, (ULONG*)used, (ULONG*)free)) == OS_OK ) 
    return OS_OK;
  else
#endif
    return OS_ERROR;

}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_drpo_new        |
+--------------------------------------------------------------------+

  PURPOSE : allocate root of dynamic sized primitive

*/
GLOBAL T_VOID_STRUCT *vsi_drpo_new ( ULONG size, ULONG opc, ULONG guess FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
T_DP_HEADER *dp_hdr;
T_S_HEADER *s_hdr;
ULONG alloc_size;
ULONG partition_size;
ULONG header_size;
T_HANDLE caller;

  caller = e_running[os_MyHandle()];

  header_size = sizeof(T_PRIM_HEADER) + sizeof(T_DP_HEADER);

  if ( ALIGN(header_size + size) > MaxPrimPartSize )
  {
    os_GetTaskName ( caller, caller, TaskName );
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_BIG_PARTITION FILE_LINE_MACRO_PASSED,
                   "No Partition available, entity %s, size %d", pf_TaskTable[caller].Name, size );
    return NULL;
  }

  if ( guess == DP_NO_FRAME_GUESS )
    alloc_size = ALIGN(header_size + size);
  else if ( guess == DP_FRAME_GUESS )
    alloc_size = ALIGN(header_size + size * 3);
  else
    alloc_size = ALIGN(header_size + guess + size);

  if ( caller != 0 && caller == TST_Handle )      
  {
    /* 
      if called by PCON in the test interface while decoding we need to reserve
      space for the S_HEADER
    */
    alloc_size += sizeof(T_S_HEADER);
  }
  
  if ( alloc_size > MaxPrimPartSize )
  {
#ifdef NU_DEBUG
    pf_handle_warning ( OS_SYST_WRN_REQ_TRUNCATED, "%s %s (%d->%d), entity %s, opc 0x%x, %s(%d)", 
                        syst_wrn, trunc_str, alloc_size, MaxPrimPartSize, pf_TaskTable[caller].Name, opc FILE_LINE_MACRO_PASSED );
#endif
    alloc_size = MaxPrimPartSize;
  }
  if ( ( prim = (T_PRIM_HEADER*)vsi_m_new_size ( alloc_size, PrimGroupHandle,
                &partition_size FILE_LINE ) ) != NULL )
  {
#ifdef MEMORY_SUPERVISION
    vsi_ppm_new ( caller, alloc_size, (T_PRIM_HEADER*)prim, file, line );
#endif
    prim->len = partition_size;  /* complete partition because header is at the end */
    prim->opc = opc;
    prim->sdu = NULL;
    prim->use_cnt = 1;
    prim->sh_offset = 0;
    prim->dph_offset = D_HDR_OFFSET(partition_size);
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
    dp_hdr->magic_nr = GUARD_PATTERN;
    dp_hdr->drp_bound_list = NULL;
    dp_hdr->next = NULL;
    dp_hdr->offset = sizeof(T_PRIM_HEADER) + ALIGN(size);
    dp_hdr->size = partition_size - sizeof(T_DP_HEADER);
    if ( dp_hdr->offset > dp_hdr->size )
    {
      dp_hdr->offset = dp_hdr->size;
    }
    /* 
     * The following code does not work since the 'caller' parameter has been removed from the function
     * prototype. The code was needed for the case where the function was called by PCON when decoding a
     * received primitive in the test interface. The caller in this case is 0 because it is either the
     * RCV_HISR on the target or the EXTR task in the simulation which is not running in the context of
     * the frame. Currently the sh_offset is set in the TIF driver tif.c although the guard pattern is not
     * set there. This is working fine so there is no reason to modify the code here. This comment
     * is just the result of some brainstorming and can be used for future modifications
     */
    if ( caller != 0 && caller == TST_Handle )      /* called by PCON */
    {
      prim->sh_offset = prim->dph_offset - sizeof(T_S_HEADER);
      dp_hdr->size = dp_hdr->size - sizeof(T_S_HEADER);
      s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);
      s_hdr->magic_nr = GUARD_PATTERN;
    }
    else
      prim->sh_offset = 0;
    return ((T_VOID_STRUCT*)P2D(prim));
  }
  return NULL;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_drpo_new_sdu    |
+--------------------------------------------------------------------+

  PURPOSE : allocate dynamic sized partition root

*/

T_VOID_STRUCT * vsi_drpo_new_sdu (ULONG Size, ULONG opc, USHORT sdu_len,
                                  USHORT sdu_offset, USHORT encode_offset, ULONG guess FILE_LINE_TYPE )
{
T_VOID_STRUCT *ptr;
ULONG alloc_size;
T_HANDLE Caller;

  Caller = 0;
  alloc_size = Size + BYTELEN((SHORT)sdu_len + (SHORT)encode_offset);
  ptr = vsi_drpo_new ( alloc_size, opc, guess FILE_LINE );
  /*
   * the following line of code causes a warning on tms470 compiler, 
   * that cannot be avoided. Warning will not cause a problem because due to the
   * arm7 alignment it is guaranteed that the sdu will start at an address divisable
   * by 4.
   */
  D_SDU(ptr) = (T_sdu*)((char*)ptr + sdu_offset);
  D_SDU_LEN(ptr) = sdu_len;
  D_SDU_OFF(ptr) = encode_offset;

  return ( (T_VOID_STRUCT*)ptr );
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_drp_bind        |
+--------------------------------------------------------------------+

  PURPOSE : bind child root pointer to a given parent root pointer 

*/
GLOBAL int vsi_drp_bind (T_VOID_STRUCT *child, T_VOID_STRUCT *parent FILE_LINE_TYPE)
{
T_PRIM_HEADER *prim;
T_DP_HEADER *dp_hdr;
T_VOID_STRUCT **new_drp_bound_list;
ULONG alloc_size;
T_HANDLE caller;
int pos;

  caller = e_running[os_MyHandle()];

  prim = D2P(parent);
  if ( ((T_DP_HEADER*)prim)->magic_nr == GUARD_PATTERN )
  {
    dp_hdr = (T_DP_HEADER*)prim;
  }
  else
  {
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
  }

  if (dp_hdr->drp_bound_list == NULL)
  {
    /* no partitions bound so far */
    alloc_size=MAX_DRP_BOUND*sizeof(T_DP_HEADER*);
    if ( ( new_drp_bound_list = (T_VOID_STRUCT**)M_ALLOC (alloc_size) ) == NULL )
    {
      /* no more memory */
      return VSI_ERROR;
    }

    memset(new_drp_bound_list,0x00,alloc_size);
    dp_hdr->drp_bound_list=new_drp_bound_list;
  }

  /* find free bind pointer */
  pos=0;
  while(pos<MAX_DRP_BOUND && dp_hdr->drp_bound_list[pos])
  {
    pos++;
  }
  if (pos == MAX_DRP_BOUND)
  {
    /* no more free bound pointers */
    return VSI_ERROR;
  }

  /* actually bind */
  P_ATTACH(child);
  dp_hdr->drp_bound_list[pos]=child;

  return VSI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_dp_new          |
+--------------------------------------------------------------------+

  PURPOSE : allocate dynamic sized partition root

*/
GLOBAL T_VOID_STRUCT *vsi_dp_new ( ULONG size, T_VOID_STRUCT *addr, ULONG guess FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
T_PRIM_HEADER *last_in_chain;
T_PRIM_HEADER *dyn_ptr;
T_DP_HEADER *dp_hdr;
T_DP_HEADER *new_prim;
T_VOID_STRUCT *ptr;
ULONG partition_size;
ULONG alloc_size;
//ULONG estimated_size;
T_HANDLE caller;
char is_opc_root;

  if ( size + sizeof(T_DP_HEADER) > MaxPrimPartSize )
  {
    caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_BIG_PARTITION FILE_LINE_MACRO_PASSED,
                   "No Partition available, entity %s, size %d", pf_TaskTable[caller].Name, size );
    return NULL;
  }
  prim = D2P(addr);
  dyn_ptr = prim;
  if ( ((T_DP_HEADER*)prim)->magic_nr == GUARD_PATTERN )
  {
    dp_hdr = (T_DP_HEADER*)prim;
    is_opc_root = 0;
  }
  else
  {
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
    is_opc_root = 1;
  }

  if ( guess == DP_NO_FRAME_GUESS )
    alloc_size = size + sizeof(T_DP_HEADER);
  else if ( guess == DP_FRAME_GUESS )
    alloc_size = size * 3 + sizeof(T_DP_HEADER);
  else
    alloc_size = size + guess + sizeof(T_DP_HEADER);

#if 0
    /*
  * update estimated size
  */
  estimated_size = dp_hdr->est_size;

  if ( guess != DP_NO_FRAME_GUESS && guess != DP_FRAME_GUESS )
  {
    estimated_size = size + guess;
    alloc_size = estimated_size + sizeof(T_DP_HEADER);
    estimated_size -= size;
  }
  else
  {
    if ( size > estimated_size )
    {
      if ( guess == DP_FRAME_GUESS )  
        estimated_size = size * 3;
      else
        estimated_size = size;
    }
    estimated_size -= size;
    if ( size > estimated_size )
      estimated_size = size * 2;
    alloc_size = estimated_size + sizeof(T_DP_HEADER);
  }
  
  if ( estimated_size > MaxPrimPartSize )
    estimated_size = MaxPrimPartSize;

  dp_hdr->est_size = estimated_size;
#endif
 /*
  * check if free space in already allocated blocks (first fit)
  */
  do
  {
    if ( dp_hdr->magic_nr != GUARD_PATTERN )
    { 
      caller = e_running[os_MyHandle()];
      vsi_o_assert ( caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "Magic number in dp_header destroyed (DP_ALLOC), %s opc: 0x%lx, partition 0x%lx",
                    pf_TaskTable[caller].Name, ((T_PRIM_HEADER*)prim)->opc, prim );
    }
    if ( dp_hdr->size - dp_hdr->offset > size )
    {
      /*
       * if root was allocated with drpo_alloc then dp header is at the end,
       * the dph offset is not 0 and a primitive header is present.
       */
      if ( is_opc_root && dyn_ptr == prim )
        ptr = (T_VOID_STRUCT*)(((ULONG*)prim) + (dp_hdr->offset>>2));
      else
        ptr = (T_VOID_STRUCT*)(((ULONG*)dp_hdr) + (dp_hdr->offset>>2));
      dp_hdr->offset += ALIGN(size);
      return ( ptr );
    }
    if ( is_opc_root && dyn_ptr == prim )
      last_in_chain = prim;
    else 
      last_in_chain = (T_PRIM_HEADER*)dp_hdr;
    dp_hdr = (T_DP_HEADER*)dp_hdr->next;
    dyn_ptr = (T_PRIM_HEADER*)dp_hdr;
  } while ( dp_hdr );

 /*
  * not enough free space -> additional allocation needed
  */
  if ( alloc_size > MaxPrimPartSize )
  {
#ifdef NU_DEBUG
    caller = e_running[os_MyHandle()];
    pf_handle_warning ( OS_SYST_WRN_REQ_TRUNCATED, "%s %s (%d->%d), entity %s, %s(%d)",
                        syst_wrn, trunc_str, alloc_size, MaxPrimPartSize, pf_TaskTable[caller].Name FILE_LINE_MACRO_PASSED );
#endif
    alloc_size = MaxPrimPartSize;
  }

  if ( ( new_prim = (T_DP_HEADER*)vsi_m_new_size ( alloc_size, PrimGroupHandle,
                     &partition_size FILE_LINE ) ) != NULL )
  {
#ifdef MEMORY_SUPERVISION
    caller = e_running[os_MyHandle()];
    vsi_ppm_new ( caller, alloc_size, (T_PRIM_HEADER*)new_prim, file, line );
#endif
    if ( ((T_DP_HEADER*)last_in_chain)->magic_nr == GUARD_PATTERN )
      dp_hdr = (T_DP_HEADER*)last_in_chain;
    else
      dp_hdr = (T_DP_HEADER*)((ULONG*)last_in_chain + last_in_chain->dph_offset);
    dp_hdr->next = new_prim;
    new_prim->magic_nr = GUARD_PATTERN;
    new_prim->drp_bound_list = NULL;
    new_prim->use_cnt = 1;
    new_prim->next = NULL;
    new_prim->size = partition_size;
    new_prim->offset = sizeof(T_DP_HEADER) + ALIGN(size);
    if ( new_prim->offset > new_prim->size )
    {
      new_prim->offset = new_prim->size;
    }
    return (T_VOID_STRUCT*)(new_prim + 1);
  }

  return NULL;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_drp_new       |
+--------------------------------------------------------------------+

  PURPOSE : allocate dynamic sized partition except root

*/
GLOBAL T_VOID_STRUCT *vsi_drp_new ( ULONG size, ULONG guess FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
T_DP_HEADER *dp_hdr;
ULONG alloc_size;
ULONG header_size;
ULONG partition_size;
T_HANDLE caller;

  header_size = sizeof(T_DP_HEADER);

  if ( ALIGN(header_size + size) > MaxPrimPartSize )
  {
    caller = e_running[os_MyHandle()];
    os_GetTaskName ( caller, caller, TaskName );
    vsi_o_assert ( NO_TASK, OS_SYST_ERR_BIG_PARTITION FILE_LINE_MACRO_PASSED,
                   "No Partition available, entity %s, size %d", pf_TaskTable[caller].Name, size );
    return NULL;
  }

  if ( guess == DP_NO_FRAME_GUESS )
    alloc_size = header_size + size;
  else if ( guess == DP_FRAME_GUESS )
    alloc_size = header_size + size * 3;
  else
    alloc_size = header_size + guess + size;

  if ( alloc_size > MaxPrimPartSize )
  {
#ifdef NU_DEBUG
    caller = e_running[os_MyHandle()];
    pf_handle_warning ( OS_SYST_WRN_REQ_TRUNCATED, "%s %s (%d->%d), entity %s, %s(%d)", 
                        syst_wrn, trunc_str, alloc_size, MaxPrimPartSize, pf_TaskTable[caller].Name FILE_LINE_MACRO_PASSED );
#endif
    alloc_size = MaxPrimPartSize;
  }

  if ( ( prim = (T_PRIM_HEADER*)vsi_m_new_size ( alloc_size, PrimGroupHandle,
                 &partition_size FILE_LINE ) ) != NULL )
  {
#ifdef MEMORY_SUPERVISION
    caller = e_running[os_MyHandle()];
    vsi_ppm_new ( caller, alloc_size, (T_PRIM_HEADER*)prim, file, line );
#endif
    dp_hdr = (T_DP_HEADER*)prim;
    dp_hdr->next = NULL;
    dp_hdr->magic_nr = GUARD_PATTERN;
    dp_hdr->drp_bound_list = NULL;
    dp_hdr->use_cnt = 1;
    dp_hdr->offset = sizeof(T_DP_HEADER) + ALIGN(size);
    dp_hdr->size = partition_size;
    if ( dp_hdr->offset > dp_hdr->size )
    {
      dp_hdr->offset = dp_hdr->size;
    }
    return (T_VOID_STRUCT*)(dp_hdr+1);
  }

  return NULL;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_free            |
+--------------------------------------------------------------------+

  PURPOSE : deallocate a chain of linked partitions

*/

int vsi_free ( T_VOID_STRUCT **Msg FILE_LINE_TYPE )
{
T_PRIM_HEADER *prim;
T_DP_HEADER *dp_hdr;
T_VOID_STRUCT** drp_bound_list;
T_HANDLE Caller = 0;
int pos;

  /* 
   * PFREE is disabled if the primitive to be freed is the currently 
   * processed one and the auto free is enabled for the calling entity 
   */

  Caller = e_running[os_MyHandle()];

  prim = D2P(*Msg);

#ifdef NU_DEBUG
  if ( os_is_valid_partition ((T_VOID_STRUCT*)prim) )
  {
    /* free to non-partition memory */
    Caller = e_running[os_MyHandle()];
    vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                   "FREE to non-partition memory, entity %s, prim 0x%x", pf_TaskTable[Caller].Name, *Msg );
  }
#endif

#ifdef PRIM_AUTO_FREE
  if ( prim == (T_PRIM_HEADER*)processed_prim[Caller] && pf_TaskTable[Caller].Flags & PARTITION_AUTO_FREE )
  {
    return VSI_OK;
  }
#endif /* PRIM_AUTO_FREE */

  /* check if we have dynamic partition or primitive */
  if ( ((T_DP_HEADER*)prim)->magic_nr == GUARD_PATTERN )
  {
    dp_hdr = (T_DP_HEADER*)prim;
  }
  else if ( prim->dph_offset != 0 )
  {
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
  }
  else
  {
    return ( vsi_c_free ( Caller, (T_VOID_STRUCT**)&prim FILE_LINE ) );
  }

  if ( dp_hdr->magic_nr != GUARD_PATTERN )
  { 
    /* primitive with T_desc_list element */
    vsi_c_free ( Caller, (T_VOID_STRUCT**)&prim FILE_LINE );
    return VSI_OK;
  }
  else
  {
    do
    {
      drp_bound_list=dp_hdr->drp_bound_list;
      if (drp_bound_list)
      {
        /* call free for bound root pointers */
        pos=0;
        while(pos<MAX_DRP_BOUND && drp_bound_list[pos])
        {
          FREE(drp_bound_list[pos]);
          pos++;
        }
      }

      /* free linked memory */
      dp_hdr = (T_DP_HEADER*)dp_hdr->next;
      vsi_c_free ( Caller, (T_VOID_STRUCT**)&prim FILE_LINE );

      if (prim == NULL && drp_bound_list)
      {
        /* free drp_bound_list */
        M_FREE(drp_bound_list);
      }
    } while ( (prim = (T_PRIM_HEADER*)dp_hdr) != NULL );
  }
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_d_sum           |
+--------------------------------------------------------------------+

  PURPOSE : get number of bytes in dynamic sized primitive

*/
GLOBAL int vsi_dp_sum ( T_VOID_STRUCT *addr, ULONG *bytes )
{
T_PRIM_HEADER *prim;
T_DP_HEADER *dp_hdr;
ULONG size;
T_HANDLE caller;

  prim = D2P(addr);
  if ( ((T_DP_HEADER*)prim)->magic_nr == GUARD_PATTERN )
    dp_hdr = (T_DP_HEADER*)prim;
  else if ( prim->dph_offset != 0 )
    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
  else
  {
    caller = e_running[os_MyHandle()];
    vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "SYSTEM WARNING: No root of linked memory in %s",
                   pf_TaskTable[caller].Name );
    return VSI_ERROR;
  }

  size = 0;
  do
  {
    if ( dp_hdr->magic_nr != GUARD_PATTERN )
    { 
      caller = e_running[os_MyHandle()];
      vsi_o_assert ( caller, OS_SYST_ERR, __FILE__, __LINE__,
                     "Magic number in dp_header destroyed, opc: 0x%lx, partition 0x%lx",
                     prim->opc, prim );
    }
    size += (dp_hdr->offset-sizeof(T_DP_HEADER));
    dp_hdr = (T_DP_HEADER*)dp_hdr->next;
  } while ( (prim = (T_PRIM_HEADER*)dp_hdr) != NULL );

  *bytes = size;
  return VSI_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_dp_max_size     |
+--------------------------------------------------------------------+

  PURPOSE : get maximum number of bytes available for user data
            in dynamic primitive

*/
GLOBAL int vsi_dp_max_size ( void )
{
  return ( (int)(MaxPrimPartSize - sizeof(T_PRIM_HEADER) - sizeof(T_DP_HEADER)) );
}
#endif


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_pmax_size     |
+--------------------------------------------------------------------+

  PURPOSE : get maximum number of bytes available for user data
            in dynamic primitive

*/
GLOBAL int vsi_c_pmax_size ( void )
{
  return ( (int)(MaxPrimPartSize - sizeof(T_DP_HEADER)) );
}
#endif

#ifdef _TOOLS_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_sync          |
+--------------------------------------------------------------------+

  PURPOSE : check if PS already started

*/
GLOBAL int vsi_c_sync ( T_HANDLE caller, T_TIME timeout )
{
T_VOID_STRUCT *prim;
T_QMSG Msg;
T_HANDLE tst_q_handle;
char sync_req_name[RESOURCE_NAMELEN];
char sync_req_time[8];
static int sync_active = 0;


  if ( sync_active == FALSE )
  {
    sync_active = TRUE;
    os_GetTaskName(caller, caller, sync_req_name);
    itoa(timeout, sync_req_time,10);

    prim = vsi_c_pnew ( sizeof(T_PRIM_HEADER)+strlen(SYSPRIM_CONFIG_TOKEN)+1
                                             +strlen(SYSPRIM_TST_SYNC_REQ)+1
                                             +strlen(sync_req_name)+1
                                             +strlen(sync_req_time)+1, 0x8000 FILE_LINE );
    strcpy ( (char*)prim, SYSPRIM_CONFIG_TOKEN );
    strcat ( (char*)prim, " " );
    strcat ( (char*)prim, SYSPRIM_TST_SYNC_REQ );
    strcat ( (char*)prim, " " );
    strcat ( (char*)prim, sync_req_name );
    strcat ( (char*)prim, " " );
    strcat ( (char*)prim, sync_req_time );

    tst_q_handle = vsi_c_open ( caller, FRM_TST_NAME );
    vsi_c_psend ( tst_q_handle, prim );

    if ( vsi_c_await ( caller, pf_TaskTable[caller].QueueHandle, &Msg, timeout ) == VSI_TIMEOUT )
    {
      vsi_o_ttrace (caller, TC_SYSTEM, "timeout - Synchronization with Stack failed" );
      sync_active = FALSE;
      return VSI_ERROR;
    }
    else
    {
      sync_active = FALSE;
      if ( strcmp ((char*)P2D(Msg.Msg.Primitive.Prim), SYSPRIM_TST_SYNC_CNF ) == 0 )
      { 
        vsi_o_ttrace (caller, TC_SYSTEM, "TST_SYNC_CNF - Synchronization with Stack succeeded" );
        vsi_c_free (caller, &Msg.Msg.Primitive.Prim);
        return VSI_OK;
      }
      else
      {
        vsi_o_ttrace (caller, TC_SYSTEM, "TST_SYNC_REJ - Synchronization with Stack failed" );
        vsi_c_free (caller, &Msg.Msg.Primitive.Prim);
        return VSI_ERROR;
      }
    }
  }
  return VSI_OK;
}
#endif

#ifdef _TOOLS_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM             |
| STATE   : code                       ROUTINE : vsi_c_generic_send  |
+--------------------------------------------------------------------+

  PURPOSE : check if PS already started

*/
int vsi_c_alloc_send ( T_HANDLE com_handle, char* dst, char* src, void *prim, char *string )
{
int alloc_size;
T_PRIM_HEADER *ptr;
T_S_HEADER *s_hdr;
unsigned int i;
unsigned int size;
int opc;
int s_header_added = 0;
int sh_offset;

  if ( string != NULL )  
  {
    size = strlen(string);
    alloc_size = size + sizeof(T_PRIM_HEADER);
    if ( dst != NULL || src != NULL )
    {
      sh_offset = ALIGN(alloc_size) / (int)sizeof(ULONG);
      alloc_size = ALIGN(alloc_size) + sizeof(T_S_HEADER);
      if ( dst != 0 )
        opc = 0;       /* to stack -> set to SYS_MASK in tst_pei_primitive() when sh_offset != 0 */
      else
        opc = SYS_MASK; /* to tools */
      s_header_added = 1;
    }
    else
    {
      opc = SYS_MASK;
      sh_offset = 0;
    }
    ptr = (T_PRIM_HEADER*)vsi_c_new ( 0, alloc_size, opc );
    memcpy ( (char*)P2D(ptr), string, size );
    if ( s_header_added == 1 )
    {
      ptr->sh_offset = sh_offset;
      s_hdr = (T_S_HEADER*)((int*)ptr+ptr->sh_offset);
      ptr->len = size + sizeof(T_PRIM_HEADER); /* exclude S_HEADER */
    }
  }
  else
  {
    ptr = D2P(prim);   /* work on passed primitive */
  }
  if ( dst != NULL )
  {
    if ( s_header_added == 0 )
    {
      alloc_size = ALIGN(ptr->len) + sizeof(T_S_HEADER);
      ptr = (T_PRIM_HEADER*)vsi_c_new ( 0, alloc_size, 0 );
      memcpy((char*)ptr, (char*)D2P(prim), D_LEN(prim));
      ptr->sh_offset = ALIGN(D_LEN(prim)) / sizeof(ULONG);
      s_hdr = (T_S_HEADER*)((int*)ptr+ptr->sh_offset);
      FREE(prim);
    }
    else
    {
      s_hdr = (T_S_HEADER*)((int*)ptr+ptr->sh_offset);
    }
    /* set org_rcv and rcv */
    for (i = 0; dst[i] && i < sizeof (s_hdr->rcv) && dst[i]!= ';'; i++)
      s_hdr->org_rcv[i] = s_hdr->rcv[i] = dst[i];
    if (i < sizeof s_hdr->rcv)
      s_hdr->org_rcv[i] = s_hdr->rcv[i] = 0;
    
    s_hdr->time = 0;
    s_header_added = 1;
  }

  if ( src != NULL )
  {
    if ( s_header_added == 0 )
    {
      alloc_size = ALIGN(ptr->len) + sizeof(T_S_HEADER);
      ptr = (T_PRIM_HEADER*)vsi_c_new ( 0, alloc_size, 0 );
      memcpy((char*)ptr, (char*)D2P(prim), D_LEN(prim));
      ptr->sh_offset = ALIGN(D_LEN(prim)) / sizeof(ULONG);
      s_hdr = (T_S_HEADER*)((int*)ptr+ptr->sh_offset);
      FREE(prim);
    }
    else
    {
      s_hdr = (T_S_HEADER*)((int*)ptr+ptr->sh_offset);
    }
    
    s_hdr->time = 0;
    
    /* set snd */
    for (i = 0; src[i] && i < sizeof (s_hdr->snd) && src[i]!= ';'; i++)
      s_hdr->snd[i] = src[i];
    if (i < sizeof s_hdr->snd)
      s_hdr->snd[i] = 0;    
  }
  
  return ( vsi_c_psend (com_handle, (T_VOID_STRUCT*)P2D(ptr)) );
}
#endif

#ifdef NU_DEBUG
#ifndef RUN_FLASH
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : VSI_COM               |
| STATE   : code                       ROUTINE : check_descriptor_list |
+----------------------------------------------------------------------+

  PURPOSE : check partitions in descriptor list

*/
int check_descriptor_list ( T_HANDLE caller, T_PRIM_HEADER *prim FILE_LINE_TYPE )
{
T_DP_HEADER *dp_hdr;
T_M_HEADER *mem;
T_desc *desc;   
LONG Status;

  dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
  /* the presence of the guard pattern at dph_offset is used to distinguish between dynamic primitives
     and primitives with descriptor list. If the guard pattern is destroyed, the primitive looks like
     having a descriptor list and the frame will probably crash during checking the integrity of the 
     partitions in the list. This bahavior is prefered to non checking the partitions */

  if ( *((ULONG*)dp_hdr) == GUARD_PATTERN )
  {
    dp_hdr = (T_DP_HEADER*)dp_hdr->next;
    while (dp_hdr != NULL)                         
    {                                             
      if ( dp_hdr->magic_nr != GUARD_PATTERN )
      {
        prim = (T_PRIM_HEADER*)dp_hdr;
        vsi_o_assert ( caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                     "Magic number in dp_header destroyed (PSEND) %s , opc: 0x%lx, partition 0x%lx",
                      pf_TaskTable[caller].Name, prim->opc, prim );
      }
      if ( (Status = os_PartitionCheck ( (T_VOID_STRUCT*)dp_hdr)) == OS_PARTITION_GUARD_PATTERN_DESTROYED )
      {
        vsi_o_assert ( caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                      "%s in dynamic primitive (PSEND),entity %s, prim 0x%x, opc 0x%x, bad partition 0x%x",
                       guard_str, pf_TaskTable[caller].Name, prim, prim->opc, dp_hdr );
        break;
      }
      dp_hdr = (T_DP_HEADER*)dp_hdr->next;                           
    }                                             
  }
  else
  {
    if ( caller != TST_Handle )
    {
      /* do not check and update the states of the primitives in descriptor lists when called by TST, because
         descriptor lists are not routed to TST and will result in the warning generated below */
      desc = (T_desc*)(((T_desc_list*)dp_hdr)->first);
      while (desc != NULL)                         
      {                                             
        mem = (T_M_HEADER*)(((char*)desc)-sizeof(T_M_HEADER));
        if ( os_is_valid_partition ((T_VOID_STRUCT*)mem) )
        {
          vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                         "pointer to non-partition memory in desc list(PSEND), entity %s, prim 0x%x, opc 0x%x",
                         pf_TaskTable[caller].Name, prim, prim->opc );
          return VSI_ERROR;
        }
        if ( (Status = os_PartitionCheck ( (T_VOID_STRUCT*)mem)) != OS_OK )
        {
          switch ( Status )
          {
            case OS_PARTITION_GUARD_PATTERN_DESTROYED:
              vsi_o_assert ( caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                            "%s in desclist (PSEND), entity %s, prim 0x%x, opc 0x%x, bad partition 0x%x",
                             guard_str, pf_TaskTable[caller].Name, prim, prim->opc, mem );
            break;
            case OS_PARTITION_FREE:
              vsi_o_assert ( caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                            "%s in desclist (PSEND), entity %s, prim 0x%x, opc 0x%x, freed partition 0x%x",
                             freed_sent_str, pf_TaskTable[caller].Name, prim, prim->opc, mem );
            break;
            default:
            break;
          }
        }
        if ( mem->desc_type == (VSI_DESC_TYPE3 >> 16) )
        {
          mem = ((T_M_HEADER*)(((T_desc3*)desc)->buffer)) - 1;
          if ( os_is_valid_partition ( (T_VOID_STRUCT*)mem ) )
          {
            vsi_o_assert ( NO_TASK, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                           "pointer to non-partition memory in desc list type 3 (PSEND), entity %s, prim 0x%x, opc 0x%x, invalid partition 0x%x",
                           pf_TaskTable[caller].Name, prim, prim->opc, mem );
            return VSI_ERROR;
          }
          if ( (Status = os_PartitionCheck ( (T_VOID_STRUCT*)mem )) != OS_OK )
          {
            switch ( Status )
            {
              case OS_PARTITION_GUARD_PATTERN_DESTROYED:
                vsi_o_assert ( caller, OS_SYST_ERR_PCB_PATTERN FILE_LINE_MACRO_PASSED,
                              "%s in desclist type 3 (PSEND), entity %s, prim 0x%x, opc 0x%x, bad partition 0x%x",
                               guard_str, pf_TaskTable[caller].Name, prim, prim->opc, mem );
              break;
              case OS_PARTITION_FREE:
                vsi_o_assert ( caller, OS_SYST_ERR FILE_LINE_MACRO_PASSED,
                              "%s in desclist type 3 (PSEND), entity %s, prim 0x%x, opc 0x%x, freed partition 0x%x",
                               freed_sent_str, pf_TaskTable[caller].Name, prim, prim->opc, mem );
              break;
              default:
              break;
            }
          }
        }

        desc = (T_desc *)desc->next;                           
      } 
    }
  }
  return VSI_OK;
}
#endif
#endif

#if !defined _TARGET_ && !defined _TOOLS_

/* -------------------------------------------------------------------------
   check functions
----------------------------------------------------------------------------*/

#ifdef TEST_PCHECK

#ifndef RUN_INT_RAM
ULONG test_pcheck ( ULONG opc, void * decoded_prim )
{
  vsi_o_ttrace ( NO_TASK, TC_SYSTEM, "test_pcheck() called for opc %8x", D_OPC(decoded_prim) );
  return pcheck_func.ret_ok+1;
}
#endif

#endif /* TEST_PCHECK */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  pcheck_register
+------------------------------------------------------------------------------
|  Description  :  register the pcheck function.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void vsi_pcheck_register ( ULONG (*func)(ULONG, void*), ULONG ret_ok )
{
  pcheck_func.ret_ok   = ret_ok;
  pcheck_func.pcheck   = func;
  pcheck_func.magic_nr = PCHECK_INITIALIZED;
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
void vsi_pcheck_init ( void )
{
#ifdef TEST_PCHECK
  vsi_pcheck_register ( test_pcheck, 0 );
#endif
  if ( pcheck_func.magic_nr != PCHECK_INITIALIZED )
  {
    pcheck_func.ret_ok   = 0;
    pcheck_func.pcheck   = NULL;
    pcheck_func.magic_nr = 0;
  }
}
#endif

#endif /* !_TARGET_ && !_TOOLS_*/





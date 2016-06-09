/* 
+------------------------------------------------------------------------------
|  File:       route.c
+------------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Deutschland, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Deutschland, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  This Modul performs the filtering and routing of
|             primitives for testing capabilities of the protocol stack.
+----------------------------------------------------------------------------- 
*/ 
 
#ifndef __ROUTE_C__
#define __ROUTE_C__
#endif

/*==== INCLUDES ===================================================*/
 
#include <stdio.h>
#include <string.h>

#include "gpfconf.h"
#include "typedefs.h"
#include "os.h"
#include "vsi.h"
#include "frame.h"
#include "tools.h"
#include "frm_defs.h"
#include "frm_types.h"
#include "frm_glob.h"
#include "p_mem.h"
#include "route.h"
#include "frm_ext.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== CONSTANTS ====================================================*/

/*==== EXTERNALS =====================================================*/

/* -------------- S H A R E D - BEGIN ---------------- */
#ifdef _TOOLS_
#pragma data_seg("FRAME_SHARED") 
#endif

extern OS_HANDLE ext_data_pool_handle;
extern T_HANDLE TestGroupHandle;
extern T_FRM_ROUTING_TABLE_ENTRY *Routing[];
extern char TaskName[];
#ifndef _TOOLS_
extern T_lemu_SendToQueue lemu_SendToQueue_func; 
#endif
/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
T_HANDLE rt_tst_handle = VSI_ERROR;
#else
extern int rt_tst_handle;
#endif

#ifdef _TOOLS_
#pragma data_seg()
#endif
/* -------------- S H A R E D - END ---------------- */

#ifndef _TOOLS_
extern const T_MEM_PROPERTIES *mem;
#endif

/*==== FUNCTIONS ==================================================*/
                          
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_Init             |
+--------------------------------------------------------------------+

  PURPOSE : initialize the routing table.
*/

GLOBAL SHORT rt_Init (void)
{  
USHORT i;
  /*
   * Initialize the routingTable
   */
  for ( i = 0; i <= MaxEntities; i++ )
    Routing[i] = NULL;

  return OS_OK;
}
#endif


#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_CvrtOpcMask      |
+--------------------------------------------------------------------+

  PURPOSE : convert "1010101" mask to opcMask and opcStatus
*/

LOCAL SHORT rt_CvrtToOpcMask ( char *OpcString, ULONG *opcMask, ULONG *opcStatus )
{  
unsigned int len;
unsigned int i;
char *p;

  len = strlen ( OpcString );
  p = OpcString + len;

  for ( i = 0; i < len; i++ )
  {
    switch (*--p)
    {
      case '1':
        *opcMask |= (1<<i);
        *opcStatus |= (1<<i);
        break;       
      case '0':
        *opcMask |= (1<<i);
        *opcStatus &= ~((unsigned int)1<<i);
        break;       
      case '*':           /* wildcard matches */
        *opcMask &= ~((unsigned int)1<<i);
        *opcStatus &= ~((unsigned int)1<<i);
        break;
      default:
        break;       
    }
  }
  return OS_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_CvrtOpcMask      |
+--------------------------------------------------------------------+

  PURPOSE : convert opcMask and opcStatus to "1010101" 
*/
LOCAL SHORT rt_CvrtFromOpcMask ( char *OpcString, ULONG opcMask, ULONG opcStatus )
{  
signed char i;
char len;

  if ( opcMask >> 16 )
    len = 32;
  else
    len = 16;
  for ( i = len-1; i >= 0; i-- )
  {
    if ( opcMask & (1<<i) )
    {
      if ( opcStatus & (1<<i) )
        *OpcString = '1';
      else
        *OpcString = '0';
    }
    else
      *OpcString = '*';
    OpcString++;
  }
  *(OpcString++) = ' ';
  *(OpcString++) = 0;
  
  return OS_OK;
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_ReadRouting      |
+--------------------------------------------------------------------+

  PURPOSE : reads all routings of the routing table of the task.
*/

GLOBAL SHORT rt_RouteRead ( T_HANDLE Handle, char *Buffer )               
{
T_HANDLE TstComHandle;
static T_FRM_ROUTING_TABLE_ENTRY *pEntry;
char OrgReceiver[ RESOURCE_NAMELEN ];
char NewReceiver[ RESOURCE_NAMELEN ];
char OpcString [18];
char const *pCmd;
static UBYTE FirstRead = 1;

  if ( FirstRead )
  {
    FirstRead = 0;
    pEntry = Routing[Handle];
  }
  if ( pEntry )
  {
    if ( pEntry->OldDestComHandle == ROUTE_ALL )
      strcpy ( OrgReceiver, RT_ALL_TOKEN );
    else
      strcpy ( OrgReceiver, pf_TaskTable[pEntry->OldDestComHandle].Name );
    TstComHandle = vsi_c_open ( Handle, FRM_TST_NAME );
    if ( pEntry->NewDestComHandle == TstComHandle )
      strcpy ( NewReceiver, pEntry->ExtDest );
    else
    {
      if ( pEntry->NewDestComHandle != 0 )
        strcpy ( NewReceiver, pf_TaskTable[pEntry->NewDestComHandle].Name );
    }
    if ( pEntry->opcMask )
      rt_CvrtFromOpcMask ( OpcString, pEntry->opcMask, pEntry->opcStatus );
    else
      OpcString[0] = 0;

    switch (pEntry->Command & RT_COMMAND_MASK)
    {
      case RT_DUPLICATE: pCmd = SYSPRIM_DUPLICATE_TOKEN;
      break;
      case RT_REDIRECT:  pCmd = SYSPRIM_REDIRECT_TOKEN;
      break;
      case RT_DESTROY:   pCmd = SYSPRIM_REDIRECT_TOKEN;
                         strcpy ( NewReceiver, SYSPRIM_NULL_TOKEN );
      break;
      default:           pCmd = NULL;
      break;
    }
    sprintf ( Buffer, "%s %s %s %s%s", pf_TaskTable[Handle].Name, pCmd, OrgReceiver, OpcString, NewReceiver );
    pEntry = pEntry->pNextEntry;
    return RT_OK;
  }
  else 
  {
    FirstRead = 1;
    pEntry = Routing[Handle];
    return RT_ERROR;
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_RoutingEntry     |
+--------------------------------------------------------------------+

  PURPOSE : adds a new routing to the routing table of the task.
            routings with duplicates are stored in the beginning
            of the table because if the primitive is redirected and
            send it can not be duplicated.
*/

LOCAL SHORT rt_RoutingEntry ( T_HANDLE SndTaskHandle, T_FRM_ROUTING_TABLE_ENTRY *pNewEntry, RT_ENTRY Status )
{
T_FRM_ROUTING_TABLE_ENTRY     *pEntry, *pNextEntry, *pPrevEntry;

  /*
   * delete all entries with same OldDestComHandle if a destroy command is stored
   */
  if ( pNewEntry->Command & RT_DESTROY )
  {
    if ( ( pEntry = Routing[SndTaskHandle] ) != NULL )
    {
      do
      {
        if ( pEntry->OldDestComHandle == pNewEntry->OldDestComHandle )
        {
          if ( pEntry == Routing[SndTaskHandle] )
          {
            if ( pEntry->pNextEntry )
            {
              ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = NULL;
            }
            Routing[SndTaskHandle] = pEntry->pNextEntry;
          }
          else
          {
            ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pPrevEntry)->pNextEntry = pEntry->pNextEntry;
            if ( pEntry->pNextEntry )
            {
              ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = pEntry->pPrevEntry;
            }
          }
          pNextEntry = pEntry->pNextEntry;
          os_DeallocateMemory ( SndTaskHandle, (T_VOID_STRUCT*)pEntry );
        }
        else
          pNextEntry = pEntry->pNextEntry;

        pEntry = pNextEntry;
      }
      while ( pEntry );
    }
  }
  else
  {
    /*
     * delete destroy command for OldDestComHandle if a new command is stored
     */
    if ( ( pEntry = Routing[SndTaskHandle] ) != NULL )
    {
      do
      {
        if ( pEntry->OldDestComHandle == pNewEntry->OldDestComHandle )
        {
          if ( pEntry->Command == RT_DESTROY )
          {
            if ( pEntry == Routing[SndTaskHandle] )
            {
              if ( pEntry->pNextEntry )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = NULL;
              }
              Routing[SndTaskHandle] = pEntry->pNextEntry;
            }
            else
            {
              ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pPrevEntry)->pNextEntry = pEntry->pNextEntry;
              if ( pEntry->pNextEntry )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = pEntry->pPrevEntry;
              }
            }
            pNextEntry = pEntry->pNextEntry;
            os_DeallocateMemory ( SndTaskHandle, (T_VOID_STRUCT*)pEntry );
          }
          break;
        }
        else
          pNextEntry = pEntry->pNextEntry;

        pEntry = pNextEntry;
      }
      while ( pEntry );
    }
  }
     
       
  if ( (pEntry = Routing[SndTaskHandle]) != NULL )
  {
    do
    {
      if ( pEntry->SndTaskHandle    == SndTaskHandle   
        && pEntry->NewDestComHandle == pNewEntry->NewDestComHandle  
        && pEntry->OldDestComHandle == pNewEntry->OldDestComHandle  
        && pEntry->MsgType          == pNewEntry->MsgType )
      {
        if ( !strcmp (pEntry->ExtDest, pNewEntry->ExtDest) )
        {
          if ( Status == RT_DELETE )
          {
            if ( pEntry == Routing[SndTaskHandle] )
            {
              Routing[SndTaskHandle] = pEntry->pNextEntry;
              if ( pEntry->pNextEntry )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = NULL;
              }
            }
            else
            {
              ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pPrevEntry)->pNextEntry = pEntry->pNextEntry;
              if ( pEntry->pNextEntry )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = pEntry->pPrevEntry;
              }
            }
            pNextEntry = pEntry->pNextEntry;
            os_DeallocateMemory ( SndTaskHandle, (T_VOID_STRUCT*)pEntry );
            return RT_OK;
          }
#ifndef _TOOLS_
          else
          {
            pEntry->Command = pNewEntry->Command;         /* modify command for old routing entry */
            pEntry->opcMask = pNewEntry->opcMask;       /* set new opcMask in old routing entry */
            pEntry->opcStatus = pNewEntry->opcStatus; /* set new opcStatus in old routing entry */
            return RT_OK;
          }
#else
          pEntry = pEntry->pNextEntry;
#endif
        }
        else
        {
#ifdef _TOOLS_
          if ( ( pEntry->opcMask == pNewEntry->opcMask )
            && ( pEntry->opcStatus == pNewEntry->opcStatus ) )
          {
            strcpy ( pEntry->ExtDest, pNewEntry->ExtDest );
            return RT_OK;
          }
          else
#endif
            pEntry = pEntry->pNextEntry;
        }
      }
      else
        pEntry = pEntry->pNextEntry;
    } while ( pEntry );
  }

  pPrevEntry = NULL;
  if ( ( pEntry = Routing[SndTaskHandle] ) != NULL )
  {
    do
    {
      pPrevEntry = pEntry;
      if ( pNewEntry->Command & RT_DUPLICATE )/* put Duplications at the end of Duplications */
      {
        if ( pEntry->Command & RT_DUPLICATE )
          pEntry = pEntry->pNextEntry;
        else
        {
          if ( pPrevEntry == Routing[SndTaskHandle] )
            pPrevEntry = NULL;
          else
            pPrevEntry = pEntry->pPrevEntry;
          break;
        }
      }
      else
        pEntry = pEntry->pNextEntry;
    }
    while ( pEntry );
  }
  if ( os_AllocateMemory ( SndTaskHandle, (T_VOID_STRUCT**)&pNextEntry, sizeof(T_FRM_ROUTING_TABLE_ENTRY), OS_NO_SUSPEND, ext_data_pool_handle ) == OS_TIMEOUT )
    return RT_NO_MEM;
  pNextEntry->SndTaskHandle = SndTaskHandle;
  pNextEntry->OldDestComHandle = pNewEntry->OldDestComHandle;
  pNextEntry->NewDestComHandle = pNewEntry->NewDestComHandle;
  pNextEntry->OldDestTaskHandle = pNewEntry->OldDestTaskHandle;
  pNextEntry->Command       = pNewEntry->Command;
  pNextEntry->MsgType       = pNewEntry->MsgType;
  pNextEntry->opcMask       = pNewEntry->opcMask;
  pNextEntry->opcStatus     = pNewEntry->opcStatus;
  strcpy ( pNextEntry->ExtDest, pNewEntry->ExtDest );
  
  if ( pEntry )                 
  {
    pEntry->pPrevEntry = pNextEntry;          /* store at the beginning/in the list */
  }
  else                          
    pNextEntry->pNextEntry = NULL;            /* append to the list */

  if ( pPrevEntry )
  {
    pPrevEntry->pNextEntry = pNextEntry;      /* store in the list */
    pNextEntry->pPrevEntry = pPrevEntry;
    pNextEntry->pNextEntry = pEntry;
  }
  else
  {
    Routing[SndTaskHandle] = pNextEntry;      /* store at the beginning */
    pNextEntry->pNextEntry = pEntry;
  }
  return RT_OK;
}
#endif

#if 0
/* not needed -> temporarily removed */
#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_ModifyRouting    |
+--------------------------------------------------------------------+

  PURPOSE : handle the contents of the redirection or duplicate
            system primitive

*/
GLOBAL SHORT rt_isolate_entity ( T_HANDLE caller, char *entity )
{
T_FRM_ROUTING_TABLE_ENTRY entry;
T_HANDLE snd;
T_HANDLE rcv;
int read_entry = FIRST_ENTRY;

  if ( (rcv = vsi_e_handle ( caller, entity )) == VSI_ERROR )
    return RT_ERROR;
  else
  {
    while ( vsi_c_get_entity_com_entry ( read_entry, rcv, &snd ) == VSI_OK )
    {
      read_entry = NEXT_ENTRY;
      entry.SndTaskHandle = snd;
      entry.OldDestComHandle = rcv;
      entry.NewDestComHandle = 0;
      entry.opcMask = 0;
      entry.MsgType = RT_PRIMITIVE_TYPE;
      entry.Command = RT_DESTROY;
      rt_RoutingEntry ( snd, &entry, RT_STORE );
    }

    entry.SndTaskHandle = rcv;
    entry.OldDestComHandle = ROUTE_ALL;
    entry.NewDestComHandle = 0;
    entry.opcMask = 0;
    entry.MsgType = RT_PRIMITIVE_TYPE;
    entry.Command = RT_DESTROY;
    rt_RoutingEntry ( rcv, &entry, RT_STORE );

    return RT_OK;
  }
}
#endif
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_ModifyRouting    |
+--------------------------------------------------------------------+

  PURPOSE : handle the contents of the redirection or duplicate
            system primitive

*/
GLOBAL SHORT rt_RoutingModify ( T_HANDLE SndTaskHandle, char *Command, char *RoutingString)
{
char Token[81];
char *pChar = RoutingString;
T_FRM_ROUTING_TABLE_ENTRY Entry = { 0 }; 
unsigned int len;
T_FRM_ROUTING_TABLE_ENTRY *pEntry, *pNextEntry;

  if ( !strcmp (Command, SYSPRIM_DUPLICATE_TOKEN) )
    Entry.Command = RT_DUPLICATE;

  if ( !strcmp (Command, SYSPRIM_REDIRECT_TOKEN) )
    Entry.Command = RT_REDIRECT;
  
  if ( (len = GetNextToken (pChar, Token, " #")) == 0 )
    return RT_ERROR;
  else
    pChar += (len+1);

  if ( !strcmp (Token, RT_CLEAR_TOKEN) )
    Entry.Command |= RT_CLEAR_ENTRY;

  if ( !strcmp (Token, RT_ALL_TOKEN) )
    Entry.Command |= RT_ALL_DESTINATION;

  if ( Entry.Command & RT_CLEAR_ENTRY )
  {
    if ( (pEntry = Routing[SndTaskHandle]) != NULL )
    {
      do
      {
        pNextEntry = (T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry;
        os_DeallocateMemory ( SndTaskHandle, (T_VOID_STRUCT*)pEntry );
        pEntry = pNextEntry;
      }
      while ( pEntry );
      Routing[SndTaskHandle] = NULL;
    }
    return RT_OK;
  }

  if ( Entry.Command & RT_ALL_DESTINATION )
  {
    Entry.OldDestComHandle = ROUTE_ALL;
    Entry.OldDestTaskHandle = ROUTE_ALL;
  }
  else
  {
    if ( ( Entry.OldDestComHandle = vsi_c_open (SndTaskHandle, Token ) ) == VSI_ERROR )
      return RT_ERROR;
    Entry.OldDestTaskHandle = Entry.OldDestComHandle;
  }

  if ( (len = GetNextToken (pChar, Token, " #")) == 0 )
    return RT_ERROR;
  else
    pChar += (len+1);

  if ( !strcmp (Token, RT_TIMEOUT_TOKEN) ) 		
  {
    Entry.MsgType = RT_TIMEOUT_TYPE;
    if ( (len = GetNextToken (pChar, Token, " #")) == 0 )
      return RT_ERROR;
    else
      pChar += (len+1);

  }
  else
  {
    if ( !strcmp (Token, RT_SIGNAL_TOKEN) )		
    {
      Entry.MsgType = RT_SIGNAL_TYPE;
      if ( (len = GetNextToken (pChar, Token, " #")) == 0 )
        return RT_ERROR;
      else
        pChar += (len+1);
    }
    else
      Entry.MsgType = RT_PRIMITIVE_TYPE;
  } 

  if ( (Token[0] == '1') || (Token[0] == '0') || (Token[0] == '*') )
  {
    rt_CvrtToOpcMask(Token, &Entry.opcMask, &Entry.opcStatus);
    if ( (len = GetNextToken (pChar, Token, " #")) == 0 )
      return RT_ERROR;
    else
      pChar += (len+1);
  }
/*    else */
  { 
    if (!strcmp (Token, SYSPRIM_NULL_TOKEN))
    {
      if ( Entry.Command & RT_REDIRECT )
      {
        Entry.Command |= RT_DESTROY;  /* destroy the primitive */
        Entry.Command &= ~RT_REDIRECT;  
      }
      else
        return RT_ERROR;
    }
    else
    {
      if ( ( Entry.NewDestComHandle = vsi_c_open (SndTaskHandle, Token ) ) == VSI_ERROR )
      {
#ifdef _TOOLS_
        if ( !strcmp ( FRM_PCO_NAME, Token ) )
          return RT_ERROR;
#endif
        if ( ( Entry.NewDestComHandle = vsi_c_open (SndTaskHandle, FRM_TST_NAME ) ) != VSI_ERROR )
        {
          rt_tst_handle = Entry.NewDestComHandle;
          if ( len < RESOURCE_NAMELEN )
          {
            strncpy( Entry.ExtDest, Token, RESOURCE_NAMELEN );
            Entry.ExtDest[RESOURCE_NAMELEN-1] = 0;
          }
          else
            return RT_ERROR;
        }
      }
    }

    if ( Entry.Command & RT_ALL_DESTINATION )
    {
      if ( (pEntry = Routing[SndTaskHandle]) != NULL )
      {
        int all_cleared = 1;
        do
        {
          pNextEntry = (T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry;
          if ( (pEntry->NewDestComHandle == Entry.NewDestComHandle )
            && !strcmp(pEntry->ExtDest, Entry.ExtDest) )
          {
            if ( pEntry == Routing[SndTaskHandle] )
            {
              Routing[SndTaskHandle] = pEntry->pNextEntry;
              if ( pEntry->pNextEntry != NULL )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = NULL;
              }
            }
            else
            {
              ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pPrevEntry)->pNextEntry = pEntry->pNextEntry;
              if ( pEntry->pNextEntry != NULL )
              {
                ((T_FRM_ROUTING_TABLE_ENTRY*)pEntry->pNextEntry)->pPrevEntry = pEntry->pPrevEntry;
              }
            }
            os_DeallocateMemory ( SndTaskHandle, (T_VOID_STRUCT*)pEntry );
          }
          else
          {
            all_cleared = 0;
          }
          pEntry = pNextEntry;
        }
        while ( pEntry );
        if ( all_cleared == 1 )
          Routing[SndTaskHandle] = NULL;
      }
    }

    if ( (pChar >= RoutingString+ strlen (RoutingString)) 
      || ((len = GetNextToken (pChar, Token, " #")) == 0) )
      return ( rt_RoutingEntry(SndTaskHandle, &Entry, RT_STORE) );
    else
      pChar += (len+1);

    if ( Entry.Command & RT_CLEAR_ENTRY )
      return ( rt_RoutingEntry(SndTaskHandle, &Entry, RT_DELETE) );
    return RT_ERROR;
  }
  
}
#endif

#ifndef RUN_FLASH
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_routePrim        |
+--------------------------------------------------------------------+

  PURPOSE : Looks if the task have some routing rules stored and
            perform the redirection or duplication.

*/

GLOBAL SHORT rt_Route ( T_HANDLE SndTaskHandle, T_HANDLE RcvComHandle, USHORT Prio, ULONG Suspend, OS_QDATA *Msg )
{
T_FRM_ROUTING_TABLE_ENTRY *pEntry;

T_PRIM_HEADER *prim;
ULONG opc;

  if ( ( pEntry = Routing[SndTaskHandle] ) != NULL )
  {
    if ( pEntry->MsgType == Msg->data16 )
    { 
      prim = (T_PRIM_HEADER*)Msg->ptr;

#ifndef _TOOLS_
      if (prim->opc & MEMHANDLE_OPC)
      {
        P_MEMHANDLE_SDU(prim)=0x00000000;
      }
#endif

      do
      {
        if ( SndTaskHandle == pEntry->SndTaskHandle 
         && ( (RcvComHandle == pEntry->OldDestComHandle) || (pEntry->OldDestComHandle == ROUTE_ALL) ) )
        {
        /*
         * a route for the receiver is defined. Now
         * check the filter conditions
         */
          opc = ((T_PRIM_HEADER*)Msg->ptr)->opc;
          if ( (opc & pEntry->opcMask) == (pEntry->opcStatus & pEntry->opcMask) )
          {
          /*
           * filter matched -> redirect, duplicate or destroy
           * the primitive.
           */
            switch (pEntry->Command & RT_COMMAND_MASK)
            {
              case RT_DESTROY: 
                /* add VSI_PPM_FREE(Msg->ptr) just to avoid the PPM warning of freeing a primitive in a queue */
                VSI_PPM_RCV(Msg->ptr);
                VSI_PPM_FREE(Msg->ptr);
                os_DeallocatePartition (SndTaskHandle, Msg->ptr-PPM_OFFSET );
                return RT_OK; 

              case RT_REDIRECT:
#ifndef _TOOLS_
                if (pEntry->NewDestComHandle != rt_tst_handle)
                {
                  /* if not on tool side and not sending via TST -> send directly */
                  Msg->e_id = pEntry->NewDestComHandle;
                  return ( (SHORT)os_SendToQueue ( SndTaskHandle, pf_TaskTable[pEntry->NewDestComHandle].QueueHandle, OS_NORMAL, OS_SUSPEND, Msg ) );
                }
#endif
                /*lint -fallthrough */
              case RT_DUPLICATE:
                if (pEntry->NewDestComHandle != OS_ERROR)
                {
                  OS_QDATA QData;
#ifdef _TOOLS_
                  QData.len = (USHORT)(PSIZE(P2D(Msg->ptr))-sizeof(T_PRIM_HEADER));
#else
                  /* QData.len = 4; not needed if per reference */
                  T_DP_HEADER *dp_hdr;
#if 0                  
                  P_ATTACH(P2D(prim));  /* recursivly increase use counters */
#else
                  prim->use_cnt++;
                  if ( prim->dph_offset != 0 )
                  {
                    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
                    /* only increment use_cnt for dynamic primitives - not for DTI primitives */
                    if ( dp_hdr->magic_nr == GUARD_PATTERN )
                    {
                      do
                      {
                        dp_hdr->use_cnt++;
                      } while ( (dp_hdr = (T_DP_HEADER*)dp_hdr->next) != NULL );
                    }
                  }
#endif /* 0 */

#endif
                  QData.data16 = MSG_PRIMITIVE;
                  QData.ptr = Msg->ptr;

#ifndef _TOOLS_
                  if (pEntry->NewDestComHandle != rt_tst_handle)
                  {
                    Msg->e_id = pEntry->NewDestComHandle;
                    os_SendToQueue ( SndTaskHandle, pf_TaskTable[pEntry->NewDestComHandle].QueueHandle, OS_NORMAL, OS_SUSPEND, Msg );
                  }
                  else
#endif
                  {
                    /* if on tool side or sending via TST -> send with sys header a.s.o */
                    rt_ExtPrimitive ( SndTaskHandle, pEntry->NewDestComHandle, RcvComHandle, pEntry->ExtDest, &QData );
                  }
                }
                if ( pEntry->Command & RT_REDIRECT )
                {
                  T_VOID_STRUCT *free_ptr = (T_VOID_STRUCT*)P2D(prim);

#ifndef _TOOLS_
                  if ((prim->opc & MEMHANDLE_OPC) && (mem!=NULL))
                  {
                    if (P_MEMHANDLE(prim)!=0)
                    {
                      mem->dealloc(P_MEMHANDLE(prim));
                    }
                  }
#endif /* !_TOOLS_ */
                  
                  vsi_free ((T_VOID_STRUCT**)&free_ptr FILE_LINE_MACRO);
                  return RT_OK; 
                }
                break;
              default:
                break;
            }
          }
        }
        pEntry = pEntry->pNextEntry;
      } while ( pEntry );
    }
  }
  
  /*
   * send original
   */

#ifdef _TOOLS_
  return ( (SHORT)os_SendToQueue ( SndTaskHandle, RcvComHandle, Prio, Suspend, Msg ) );
#else
  if((pf_TaskTable[RcvComHandle].Flags & USE_LEMU_QUEUE) AND
     (lemu_SendToQueue_func.magic_nr == LEMU_SENDTOQUEUE_INITIALIZED))
  {
    if(lemu_SendToQueue_func.plemu_SendToQueue( SndTaskHandle, RcvComHandle,
             pf_TaskTable[RcvComHandle].QueueHandle, Prio, Suspend, Msg ) 
             != lemu_SendToQueue_func.ret_ok  )
    {
      return OS_ERROR;
    }
    return OS_OK;
  }
  else
  {
    return ( (SHORT)os_SendToQueue ( SndTaskHandle, pf_TaskTable[RcvComHandle].QueueHandle, Prio, Suspend, Msg ) );
  }
#endif
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_ExtPrimitive     |
+--------------------------------------------------------------------+

  PURPOSE : formats duplicated primitive to be sent to TST

*/
void rt_ExtPrimitive ( T_HANDLE TaskHandle, T_HANDLE DestComHandle, T_HANDLE OrgDestTaskHandle, char *ExtDest, OS_QDATA *Msg ) 
{
T_PRIM_HEADER *prim;
#ifndef _TOOLS_
T_PRIM_HEADER *sdu_prim;
#endif /* not _TOOLS_ */
T_PRIM_HEADER *ptr;
T_S_HEADER *s_hdr;
OS_QDATA DMsg;
ULONG AllocSize;
LONG Status;
ULONG suspend;
#ifdef _TOOLS_
T_S_HEADER *prim_s_hdr;
unsigned int i;
#endif

  prim = (T_PRIM_HEADER*)Msg->ptr;
#ifdef _TOOLS_
  AllocSize = S_ALLOC_SIZE(Msg->len + 1); /* +1 To add LF in TIF */
#else
  if ( prim->dph_offset != 0 )
  {
    T_DP_HEADER *dp_hdr;

    dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
    if ( dp_hdr->magic_nr != GUARD_PATTERN 
#ifdef _TARGET_
      && route_desclist[TaskHandle] == TRUE
#endif
         )
    {
      rt_desclist_to_sdu ( TaskHandle, DestComHandle, prim, &sdu_prim );
      prim = sdu_prim;
    }
  }

  AllocSize = S_ALLOC_SIZE(4 + 1);        /* only ptr to primitive */
#endif /* _TOOLS_ */
  suspend = get_suspend_state(TaskHandle,CHECK_PRIM_SUSPEND);
  Status = os_AllocatePartition ( TaskHandle, (T_VOID_STRUCT**)&ptr, AllocSize, suspend, TestGroupHandle );
  if ( Status == OS_OK || Status == OS_WAITED || Status == OS_ALLOCATED_BIGGER )
  {
    DMsg.data16 = MSG_PRIMITIVE;
  	DMsg.data32 = Msg->data32;
#ifdef _TOOLS_
    DMsg.len = AllocSize;
#endif
    DMsg.ptr = (T_VOID_STRUCT*)ptr;

    ptr->opc = prim->opc;
    ptr->len = AllocSize;
    ptr->sh_offset = S_HDR_OFFSET(AllocSize - sizeof(T_S_HEADER));
    s_hdr = (T_S_HEADER*)((ULONG*)ptr + ptr->sh_offset);
#ifdef _TOOLS_
    if ( prim->sh_offset == 0 )
    {
      /* 
         if the primitive is sent via the TAP REDIRECT TAP ... mechanism, then the prim->sh_offset
         is zero and the org_rcv is filled corresponding to the original addressed entity name.
      */
      vsi_e_name ( TaskHandle, OrgDestTaskHandle, TaskName );
      strcpy (s_hdr->org_rcv, TaskName);
      strcpy (s_hdr->rcv, ExtDest);
    }
    else
    {
      /*
         if the primitive is sent via the TAP TPORT mechanism then the original receiver is already 
         filled by the TAP and copied to the newly allocated primitive to be routed.
      */
      prim_s_hdr = (T_S_HEADER*)((ULONG*)prim + prim->sh_offset);

      for (i = 0; prim_s_hdr->org_rcv[i] != 0 && i < sizeof (s_hdr->org_rcv)-1; i++)
        s_hdr->org_rcv[i] = prim_s_hdr->org_rcv[i];
      for (i = 0; prim_s_hdr->rcv[i] != 0     && i < sizeof (s_hdr->rcv)-1;     i++)
        s_hdr->rcv[i]     = prim_s_hdr->rcv[i];
    }
    get_local_time (&s_hdr->time);
    os_GetTaskName ( TaskHandle, TaskHandle, TaskName );
    strcpy (s_hdr->snd, TaskName);
#else
    vsi_e_name ( TaskHandle, OrgDestTaskHandle, TaskName );
    strcpy (s_hdr->org_rcv, TaskName);
    strcpy (s_hdr->rcv, ExtDest);
    os_GetTime(TaskHandle,&s_hdr->time);
    s_hdr->snd[0] = (char)(TaskHandle | HANDLE_BIT);
#endif
#ifdef _TOOLS_
    memcpy ( (char*)P2D(ptr), P2D(Msg->ptr), Msg->len );
#else
    ((T_PRIM_X*)(ptr))->prim_ptr = prim;
#endif
    DMsg.e_id = DestComHandle;
#ifdef _TOOLS_
    os_SendToQueue ( TaskHandle, DestComHandle, OS_NORMAL, OS_SUSPEND, &DMsg );
#else
    if ((prim->opc & MEMHANDLE_OPC) && (mem!=NULL))
    {
      if (P_MEMHANDLE_SDU(prim)==0x00000000)
      {
        /* copy MEM-handle content into new sdu */
        U8 *user_data;
        U16 ptr_length;
        user_data=mem->get_user_data(P_MEMHANDLE(prim), &ptr_length);

        if (user_data)
        {
          T_sdu *sdu=(T_sdu *)M_ALLOC(ptr_length+2*sizeof(U16));

          sdu->l_buf=ptr_length*8;
          sdu->o_buf=0;
          memcpy(sdu->buf,user_data,ptr_length);

          P_MEMHANDLE_SDU(prim)=(U32)sdu;
        }
      }
      else
      {
        M_ATTACH((void*)P_MEMHANDLE_SDU(prim));
      }
    }

    if ( PrimAborted[TaskHandle] > 0 )
    {
      if ( vsi_o_ttrace ( 0, TC_SYSTEM, "%d %s primitive routings aborted",PrimAborted[TaskHandle], pf_TaskTable[TaskHandle].Name ) == VSI_OK )
        PrimAborted[TaskHandle] = 0;
    }
    if ( os_SendToQueue ( TaskHandle, pf_TaskTable[DestComHandle].QueueHandle, OS_NORMAL, suspend, &DMsg ) == OS_TIMEOUT )
    {
      T_VOID_STRUCT *free_ptr = (T_VOID_STRUCT*)P2D(prim);
      vsi_free ((T_VOID_STRUCT**)&free_ptr FILE_LINE_MACRO);
      vsi_trc_free (0, (T_VOID_STRUCT**)&ptr);
      PrimAborted[TaskHandle]++;
    }
#endif
  }
  else
  {
    T_VOID_STRUCT *free_ptr = (T_VOID_STRUCT*)P2D(prim);
    vsi_free ((T_VOID_STRUCT**)&free_ptr FILE_LINE_MACRO);
    PrimAborted[TaskHandle]++;
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : PF_ROUTE            |
| STATE   : code                       ROUTINE : rt_desclist_to_sdu  |
+--------------------------------------------------------------------+

  PURPOSE : formats duplicated primitive with descriptor list into a 
            primitive with SDU

*/
int rt_desclist_to_sdu ( T_HANDLE caller, T_HANDLE dst, T_PRIM_HEADER *prim, T_PRIM_HEADER **sdu_prim )
{
T_PRIM_HEADER *ptr;
T_DP_HEADER *dp_hdr;
T_M_HEADER *m_hdr;
T_desc3 *desc3;
T_desc2 *desc2;
T_desc *desc;
int len = 0;                   
int sdu_prim_size = 0;
char *fill_ptr;
USHORT *l_buf_ptr;
USHORT *o_buf_ptr;
char *sdu_data_ptr;
T_sdu *sdu = NULL;
unsigned int new_opc;
int desc_type = 0; 

  dp_hdr = (T_DP_HEADER*)((ULONG*)prim + prim->dph_offset);
  /* primitive contains descriptor list */

  /*
  START OF NON GENERIC CODE
  This code is for the phase 1 routing of primitives containing descriptor lists.
  It is non-generic and depends on the primitive ids defined in SAP docs.
  It should be removed as soon as the generic descriptor list routing is ready
  */
  switch ( prim->opc )
  {
  case 0x7754: /* DTI2_DATA_IND */
      new_opc = 0x7755; /* DTI2_DATA_TEST_IND */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x3754: /* DTI2_DATA_REQ */
      new_opc = 0x3755; /* DTI2_DATA_TEST_REQ */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x6800: /* SN_DATA_IND */
      new_opc = 0x6806; /* SN_DATA_TEST_IND */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x2800: /* SN_DATA_REQ */
      new_opc = 0x2806; /* SN_DATA_TEST_REQ */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x6801: /* SN_UNITDATA_IND */
      new_opc = 0x6807; /* SN_DATA_TEST_IND */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x2801: /* SN_UNITDATA_REQ */
      new_opc = 0x2807; /* SN_DATA_TEST_REQ */
      desc_type = VSI_DESC_TYPE2 >> 16;
      break;
  case 0x7701: /* DTI_DATA_IND */
      new_opc = 0x7702; /* DTI_DATA_TEST_IND */
      desc_type = VSI_DESC_TYPE1 >> 16;
      break;
  case 0x3701: /* DTI_DATA_REQ */
      new_opc = 0x3702; /* DTI_DATA_TEST_REQ */
      desc_type = VSI_DESC_TYPE1 >> 16;
      break;
  case 0x80004097: /* GRLC_DATA_IND */
      new_opc = 0x80014097; /* GRLC_DATA_IND_TEST */
      desc_type = VSI_DESC_TYPE1 >> 16;
      break;
  case 0x80024097: /* GRLC_UNITDATA_IND */
      new_opc = 0x80034097; /* GRLC_UNITDATA_IND_TEST */
      desc_type = VSI_DESC_TYPE1 >> 16;
      break;
  default:
      new_opc = prim->opc;
      break;
  }
  /* END OF NON GENERIC CODE */

  /* 
  it has to be distinguished between T_desc, T_desc2 and T_desc3 because to length information
  is located at different places inside the structure 
  */
  m_hdr = (T_M_HEADER*)(((T_desc_list*)dp_hdr)->first) - 1;
  if ( m_hdr->desc_type != 0 )
  {
    desc_type = m_hdr->desc_type;
  }
  if ( desc_type == (VSI_DESC_TYPE2 >> 16) )
  {
    desc2 = (T_desc2*)(((T_desc_list2*)dp_hdr)->first);
    while (desc2 != NULL)                         
    {                                             
      len = len + desc2->len;
      desc2 = (T_desc2 *)desc2->next;                           
    }
    /* the size to be allocated for the primitive containing the sdu needs to be 
                    root prim length  + data length + sdu size minus data buffer            - desc list2 size */
    sdu_prim_size = prim->len         + len         + sizeof(sdu->l_buf) + sizeof(sdu->o_buf) - sizeof(T_desc_list2);
  }
  else if ( desc_type == (VSI_DESC_TYPE1 >> 16) )
  {
    desc = (T_desc*)(((T_desc_list*)dp_hdr)->first);
    while (desc != NULL)                         
    {                                             
      len = len + desc->len;
      desc = (T_desc *)desc->next;                           
    }
    /* the size to be allocated for the primitive containing the sdu needs to be 
                    root prim length  + data length + sdu size minus data buffer            - desc list size */
    sdu_prim_size = prim->len         + len         + sizeof(sdu->l_buf) + sizeof(sdu->o_buf) - sizeof(T_desc_list);
  }
  else if ( desc_type == (VSI_DESC_TYPE3 >> 16) )
  {
    /* it is assumed that type 3 is valid if not 1 or 2 */
    desc3 = (T_desc3*)(((T_desc_list3*)dp_hdr)->first);
    while (desc3 != NULL)                         
    {                                             
      len = len + desc3->len;
      desc3 = (T_desc3 *)desc3->next;                           
    }
    /* the size to be allocated for the primitive containing the sdu needs to be 
                    root prim length  + data length + sdu size minus data buffer            - desc list3 size */
    sdu_prim_size = prim->len         + len         + sizeof(sdu->l_buf) + sizeof(sdu->o_buf) - sizeof(T_desc_list3);
  }
  else 
  {
    vsi_o_ttrace ( 0, TC_SYSTEM, "unknown desc type in 0x%x, routing aborted", prim->opc );
    vsi_c_free ( caller, (T_VOID_STRUCT**)&prim FILE_LINE_MACRO );
    return RT_ERROR;
  }

  if ( sdu_prim_size < (int)MaxPrimPartSize )
  {
    ptr = (T_PRIM_HEADER*)vsi_c_new ( caller, sdu_prim_size, new_opc FILE_LINE_MACRO );
  }
  else
  {
    vsi_o_ttrace ( 0, TC_SYSTEM, "desclist in 0x%x too long, routing aborted", prim->opc );
    vsi_c_free ( caller, (T_VOID_STRUCT**)&prim FILE_LINE_MACRO );
    return RT_ERROR;
  }
#ifdef MEMORY_SUPERVISION
  vsi_ppm_send ( caller, dst, (T_PRIM_HEADER*)ptr FILE_LINE_MACRO );
#endif /* MEMORY_SUPERVISION */

  fill_ptr = (char*)ptr;
  l_buf_ptr =  &((T_sdu*)((int*)fill_ptr + prim->dph_offset))->l_buf;
  o_buf_ptr =  &((T_sdu*)((int*)fill_ptr + prim->dph_offset))->o_buf;
  sdu_data_ptr = (char*)&((T_sdu*)((int*)fill_ptr + prim->dph_offset))->buf;

  memcpy ( (char*)P2D(fill_ptr), (char*)P2D(prim), prim->len-sizeof(T_PRIM_HEADER) );

  *l_buf_ptr = 0;
  *o_buf_ptr = 0;

  fill_ptr = sdu_data_ptr;

  if ( desc_type == (VSI_DESC_TYPE2 >> 16) )
  {
    desc2 = (T_desc2*)(((T_desc_list2*)dp_hdr)->first);
    while (desc2 != NULL)                         
    {                                             
      *l_buf_ptr += (desc2->len*8);
      memcpy ( fill_ptr, (char*)(&desc2->buffer)+desc2->offset, desc2->len );
      fill_ptr += desc2->len;
      desc2 = (T_desc2 *)desc2->next;                           
    }
  }
  else if ( desc_type == (VSI_DESC_TYPE1 >> 16) )
  {
    desc = (T_desc*)(((T_desc_list*)dp_hdr)->first);
    while (desc != NULL)                         
    {                                             
      *l_buf_ptr += (desc->len*8);
      memcpy ( fill_ptr, (char*)(&desc->buffer), desc->len );
      fill_ptr += desc->len;
      desc = (T_desc *)desc->next;                           
    }
  }
  else if ( desc_type == (VSI_DESC_TYPE3 >>16) )
  {
    desc3 = (T_desc3*)(((T_desc_list3*)dp_hdr)->first);
    while (desc3 != NULL)                         
    {                                             
      *l_buf_ptr += (desc3->len*8);
      memcpy ( fill_ptr, (char*)(desc3->buffer)+desc3->offset, desc3->len );
      fill_ptr += desc3->len;
      desc3 = (T_desc3 *)desc3->next;                           
    }
  }
  vsi_c_free ( caller, (T_VOID_STRUCT**)&prim FILE_LINE_MACRO );
  *sdu_prim = ptr;
  return RT_OK;
}
#endif



/*
+------------------------------------------------------------------------------
|  File:       gdi.h
+------------------------------------------------------------------------------
|                 Copyright Condat AG 1999-2001, Berlin
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Condat AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Condat AG.
+------------------------------------------------------------------------------
| Purpose:    General Definitions.
| $Identity:$
+------------------------------------------------------------------------------
*/

#ifndef  GDI_H
#define  GDI_H

/*==== INCLUDES =============================================================*/

#include "typedefs.h"

/*==== DEFINITIONS ==========================================================*/
/*--------------------------------
   Return values
  --------------------------------*/
#ifdef DRV_OK
 #undef DRV_OK
#endif
#define DRV_OK                   0
#define DRV_BUFFER_FULL          1
#define DRV_DISABLED             2
#define DRV_ENABLED              3
#define DRV_INITFAILURE          4
#define DRV_INITIALIZED          5
#define DRV_INTERNAL_ERROR       6
#define DRV_INPROCESS            7
#define DRV_INVALID_PARAMS       8
#define DRV_NOTCONFIGURED        9
#define DRV_RETVAL_USER          50

/*--------------------------------
   Signal types
  --------------------------------*/
#define DRV_SIGFCT_NOTAVAILABLE  0x01
#define DRV_SIGTYPE_CLEAR        0x02
#define DRV_SIGTYPE_FLUSH        0x04
#define DRV_SIGTYPE_READ         0x08
#define DRV_SIGTYPE_WRITE        0x10
#define DRV_SIGTYPE_CONNECT      0x20
#define DRV_SIGTYPE_DISCONNECT   0x40

#define DRV_SIGTYPE_USER         0x80

/*--------------------------------
   Flags
  --------------------------------*/
#define CALLED_FROM_ISR          0x01

/*--------------------------------
   Buffer Types
  --------------------------------*/
#define DRV_BUFTYPE_READ         0x01
#define DRV_BUFTYPE_WRITE        0x02

/*==== TYPES ================================================================*/
typedef void * drv_ProcHandle_Type;

/*------------------------------------------------------------------------
   T_DRV_SIGNAL - driver signal identification
 
   The type defines the signal information data used to identify a signal. 
   This data type is used to define and to report a signal. A signal is 
   defined by a process calling the driver function drv_SetSignal. An 
   event is signalled by driver by calling the pre-defined signal call-
   back function.
  -------------------------------------------------------------------------*/
typedef struct
{
  USHORT              SignalType;
  USHORT              DrvHandle;
  ULONG               DataLength;
  T_VOID_STRUCT       *UserData;
} T_DRV_SIGNAL;

/*------------------------------------------------------------------------
   T_DRV_CB_FUNC - driver signal device control block
  
   This type defines a call-back function used to signal driver events,
   e.g. driver is ready to accept data. The driver calls the signal
   call-back function when a specific event occurs and the driver has
   been instructed to signal the event to a specific process. A process
   can set or reset event signalling by calling one of the driver 
   functions drv_SetSignal or drv_ResetSignal. Event signalling can only
   be performed when a call-back function has been installed at driver
   initialization.
  -------------------------------------------------------------------------*/
typedef void (*T_DRV_CB_FUNC ) (T_DRV_SIGNAL * Signal);

struct _T_DRV_EXPORT;

typedef struct
{
#ifdef _TOOLS_
  USHORT (*drv_Init)(USHORT,T_DRV_CB_FUNC,struct _T_DRV_EXPORT const**);
#endif
  void (*drv_Exit)(void);
  USHORT (*drv_Read)(void*, ULONG*);
  USHORT (*drv_Write)(void*, ULONG*);
  USHORT (*drv_Look)(void*, ULONG*);
  USHORT (*drv_Clear)(USHORT);
  USHORT (*drv_Flush)(void);
  USHORT (*drv_SetSignal)(USHORT);
  USHORT (*drv_ResetSignal)(USHORT);
  USHORT (*drv_SetConfig)(char*);
  USHORT (*drv_GetConfig)(char*);
  void (*drv_Callback)(T_DRV_SIGNAL*);
} T_DRV_FUNC;

typedef struct _T_DRV_EXPORT 
{
  const char    *Name;
  USHORT   	    Flags;             /* Bit (0): CALLED_BY_ISR */
  T_DRV_FUNC DrvFunc;
} T_DRV_EXPORT;


typedef struct
{
  char const *Name;
  USHORT (*drv_Init)(USHORT, T_DRV_CB_FUNC, T_DRV_EXPORT const **);
  char const *Process;
  void const *DrvConfig;
} T_DRV_LIST_ENTRY;

typedef struct
{
  T_DRV_LIST_ENTRY DrvEntry [ 5 ];
} T_DRV_LIST;

/*==== END OF FILE ==========================================================*/
#endif

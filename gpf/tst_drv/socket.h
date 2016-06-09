/* 
+------------------------------------------------------------------------------
|  File:       socket.h
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
|  Purpose :  This Modul contains the serial driver adaptation
+----------------------------------------------------------------------------- 
*/ 

#ifndef  SOCKET_H
#define  SOCKET_H

/*==== INCLUDES =============================================================*/
#include "typedefs.h"
#include "gdi.h"

#ifdef _VXWORKS_
  /* wha!??? */
  #define SO_DONTLINGER   (~SO_LINGER)  /* Older SunOS compat. hack */
  #define TCP_NODELAY     1
#endif

/*==== DEFINITIONS ==========================================================*/
#define SOCKET_SIGTYPE_CONNECT	DRV_SIGTYPE_USER    /* Connected event signal*/
#define SOCKET_SIGTYPE_RELEASE	(DRV_SIGTYPE_USER+1)/* Released event signal */

#define SOCKET_ERRUNSPEC        (DRV_RETVAL_USER+1)
#define SOCKET_NOCONNECT        (DRV_RETVAL_USER+2)

#define SOCKET_INVALID_PORT     0  
#define SOCKET_MAX_LEN_HOSTNAME	64

/*==== TYPES ================================================================*/
typedef struct
{
	USHORT      port;
  USHORT      tx_buffer_size ;
  USHORT      rx_buffer_size ;
  ULONG       tx_timeout_msec ;
  ULONG       rx_timeout_msec ;
	char        hostname[SOCKET_MAX_LEN_HOSTNAME+1] ;
  USHORT      config;
} socket_DCB_Type;

/*==== FUNCTIONS ============================================================*/
EXTERN USHORT socket_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo ) ;
EXTERN void   socket_Exit (void) ;
EXTERN USHORT socket_Open (void) ;
EXTERN USHORT socket_Close (void) ;
EXTERN USHORT socket_Read (void* out_BufferPtr, ULONG*	thr_BufferSizePtr) ;
EXTERN USHORT socket_Write (void*	in_BufferPtr, ULONG*	thr_BufferSizePtr) ;
EXTERN USHORT socket_Flush (void) ;
EXTERN USHORT socket_SetSignal (USHORT SignalType) ;
EXTERN USHORT socket_ResetSignal (USHORT	SignalType) ;
EXTERN USHORT socket_SetConfig (char*	in_DCBPtr) ;
EXTERN USHORT socket_GetConfig (socket_DCB_Type*	out_DCBPtr) ;

/*==== END OF FILE ==========================================================*/
#endif


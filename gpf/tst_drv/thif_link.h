/* 
+------------------------------------------------------------------------------
|  File:       thif_link.h
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

#ifndef  THIF_LINK_H
#define  THIF_LINK_H

/*==== INCLUDES =============================================================*/
//#include "typedefs.h"
#include "gdi.h"

/*==== DEFINITIONS ==========================================================*/
#define THIF_SOCK_SIGTYPE_CONNECT	DRV_SIGTYPE_USER    /* Connected event signal*/
#define THIF_SOCK_SIGTYPE_RELEASE	(DRV_SIGTYPE_USER+1)/* Released event signal */

#define THIF_SOCK_ERRUNSPEC        (DRV_RETVAL_USER+1)
#define THIF_SOCK_NOCONNECT        (DRV_RETVAL_USER+2)

#define THIF_SOCK_INVALID_PORT     0  
#define THIF_SOCK_MAX_LEN_HOSTNAME	20

/*==== TYPES ================================================================*/


/*==== FUNCTIONS ============================================================*/
EXTERN void thif_link_Exit (void) ;
EXTERN USHORT thif_link_Open (void) ;
EXTERN USHORT thif_link_Close (void) ;
EXTERN USHORT thif_link_Read (void* Buffer, ULONG*	SizeInBytes) ;
EXTERN USHORT thif_link_Write (void* Buffer, ULONG*	SizeInBytes) ;
EXTERN USHORT thif_link_Flush (void) ;
EXTERN USHORT thif_link_SetSignal (USHORT SignalType) ;
EXTERN USHORT thif_link_ResetSignal (USHORT	SignalType) ;
EXTERN USHORT thif_link_SetConfig (char*	in_DCBPtr) ;
EXTERN USHORT thif_link_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo );

/*==== END OF FILE ==========================================================*/
#endif /* THIF_LINK_H */


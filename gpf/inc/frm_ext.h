/* 
+------------------------------------------------------------------------------
|  File:       frm_ext.h
+------------------------------------------------------------------------------
|  Copyright 2005 Texas Instruments Berlin, AG 
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
|  Purpose :  Definitions for Frame Extension Interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef FRM_EXT_H
#define FRM_EXT_H

/*==== TYPES ================================================================*/

#ifndef _TOOLS_
typedef struct
{
  unsigned int magic_nr;
  int ret_ok;
  int (*plemu_SendToQueue)(OS_HANDLE SndComHandle, OS_HANDLE RcvComHandle, 
                           OS_HANDLE RcvQHandle, USHORT Prio, ULONG Suspend,
                           OS_QDATA *Msg );      
} T_lemu_SendToQueue;
#endif

/*==== PROTOTYPES ===============================================================*/

void  fei_lemu_SendToQueue_init ( void );
void  fei_lemu_SendToQueue_register(int(* func)(OS_HANDLE SndComHandle, OS_HANDLE RcvComHandle, 
                           OS_HANDLE RcvQHandle, USHORT Prio, ULONG Suspend,
                           OS_QDATA *Msg ), ULONG ret_ok);

/*==== MACROS ================================================================*/

#define LEMU_SENDTOQUEUE_INITIALIZED 0xaffedead

#endif /* FRM_EXT_H */

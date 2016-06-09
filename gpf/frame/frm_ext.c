/* 
+------------------------------------------------------------------------------
|  File:       frm_ext.c
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
|  Purpose :  This module provides functionality for the Frame Extension 
|             Interface.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __FRM_EXT_C__
#define __FRM_EXT_C__
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
#include "frm_ext.h"

/*==== TYPES ================================================================*/


/*==== PROTOTYPES ===============================================================*/

void  fei_lemu_SendToQueue_init ( void );
void  fei_lemu_SendToQueue_register(int(* func)(OS_HANDLE SndComHandle, OS_HANDLE RcvComHandle, 
                           OS_HANDLE RcvQHandle, USHORT Prio, ULONG Suspend,
                           OS_QDATA *Msg ), ULONG ret_ok);

/*==== MACROS ================================================================*/

#define LEMU_SENDTOQUEUE_INITIALIZED 0xaffedead

/*==== VARIABLES ==================================================*/

#ifndef _TOOLS_
#ifndef RUN_INT_RAM
 T_lemu_SendToQueue lemu_SendToQueue_func;
#else
 extern T_lemu_SendToQueue lemu_SendToQueue_func;
#endif
#endif

/*==== FUNCTIONS ==================================================*/

#ifndef _TOOLS_
#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  fei_lemu_SendToQueue_register
+------------------------------------------------------------------------------
|  Description  :  register the lemu_SendToQueue function.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void fei_lemu_SendToQueue_register( int (* func)(OS_HANDLE SndComHandle, 
              OS_HANDLE RcvComHandle, OS_HANDLE RcvQHandle, USHORT Prio, 
              ULONG Suspend, OS_QDATA *Msg ), ULONG ret_ok)
{
  lemu_SendToQueue_func.ret_ok            = ret_ok;
  lemu_SendToQueue_func.plemu_SendToQueue = func;
  lemu_SendToQueue_func.magic_nr          = LEMU_SENDTOQUEUE_INITIALIZED;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  fei_lemu_SendToQueue_init
+------------------------------------------------------------------------------
|  Description  :  initialize lemu_SendToQueue function pointer table.
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void fei_lemu_SendToQueue_init ( void )
{
  if ( lemu_SendToQueue_func.magic_nr != LEMU_SENDTOQUEUE_INITIALIZED)
  {
    lemu_SendToQueue_func.ret_ok   = 0;
    lemu_SendToQueue_func.plemu_SendToQueue   = NULL;
    lemu_SendToQueue_func.magic_nr = 0;
  }
}
#endif

#endif /* !_TOOLS_*/


#endif /* __FRM_EXT_C__ */

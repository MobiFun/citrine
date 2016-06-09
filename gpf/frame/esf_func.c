/* 
+------------------------------------------------------------------------------
|  File:       esf_func.c
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
|  Purpose :  This Modul defines the ESF interface functions.
+----------------------------------------------------------------------------- 
*/ 



#ifndef __ESF_FUNC_C__
#define __ESF_FUNC_C__
#endif

#undef TEST_ESF_REGISTER

/*==== INCLUDES ===================================================*/

#ifdef _NUCLEUS_
 /* the include of nucleus.h in this file is a dirty hack to mask the
    issue of different type definitions in typedefs.h nad nucleus.h.
    typedefs.h contains #if defined (NUCLEUS) to protect against
    double definition and must be included after nucleus.h
 */
 #include "nucleus.h"
#endif
#include "typedefs.h"
#include "glob_defs.h"
#include "vsi.h"
#include "os.h"
#include "os_types.h"
#include "header.h"
#include "esf_func.h"
#include "string.h"
#include "stdio.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

#ifdef FF_OS_ONLY
 extern char esf_pool[];
 extern int  esf_pool_size;
#endif

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
 T_ESF_FUNC esf_func; 
#ifdef FF_OS_ONLY
 OS_HANDLE esf_pool_handle;
#endif
#else
 extern T_ESF_FUNC esf_func; 
 extern void esf_register ( T_ESF_FUNC * func );
#endif 
  
/*==== LINT =======================================================*/


/*==== FUNCTIONS ==================================================*/

#ifdef TEST_ESF_REGISTER

#ifndef RUN_INT_RAM
void init_func1 ( void  )
{
#ifndef _TARGET_
   printf ("%s\n","ESF inif_func1 called");
#endif
}
#endif

#ifndef RUN_INT_RAM
void init_func2 ( void  )
{
#ifndef _TARGET_
   printf ("%s\n","ESF inif_func2 called");
#endif
}
#endif

#ifndef RUN_INT_RAM
void send_prim ( T_PRIM_HEADER * prim )
{
#ifndef _TARGET_
   printf ("%s\n","ESF send_prim called");
#endif
}
#endif

#ifndef RUN_INT_RAM
T_ESF_FUNC esf_functions =
{
ESF_INITIALIZED,
init_func1,
init_func2,
send_prim
};
#endif

#endif

#ifdef FF_OS_ONLY
#ifndef RUN_INT_RAM 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : OS_FRM              |
| STATE   : code                       ROUTINE : esf_task_entry      |
+--------------------------------------------------------------------+

  PURPOSE : entry function for the ESF helping task.

*/

GLOBAL void esf_task_entry (OS_HANDLE TaskHandle, ULONG Value)
{
  /* call second ESF init function */
  esf_init_func2();

  /* suspend helping task forever */
  for(;;)
  {
    os_SuspendTask(TaskHandle,10000);
  }
}
#endif

#ifndef RUN_INT_RAM 
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-Frame (8415)           MODULE  : OS_FRM              |
| STATE   : code                       ROUTINE : StartFrame          |
+--------------------------------------------------------------------+

  PURPOSE : this is called by ESF.

*/
SHORT StartFrame ( void )
{
OS_HANDLE esf_task_handle;

  os_Initialize();

  /* set the OS tick to 10ms */
  os_set_tick(SYSTEM_TICK_10_MS);

  /* create memory pool for temporary usage during ESF partition pool creation and HISR stacks */
  os_CreateMemoryPool (OS_NOTASK, "ESF_POOL", &esf_pool, esf_pool_size, &esf_pool_handle);

  /* tell this pool handle to the OS layer */
  os_SetPoolHandles (esf_pool_handle, esf_pool_handle);

  /* create a helping task that calls esf_init2() from task context */
  os_CreateTask (OS_NOTASK, "ESF_TASK", esf_task_entry, 1024, 255, &esf_task_handle, esf_pool_handle);

  /* and start it */
  os_StartTask (OS_NOTASK, esf_task_handle, 0);

  esf_init();
  /* call fist ESF init function */
  esf_init_func1(); 

  return OS_OK;

}
#endif
#endif /* FF_OS_ONLY */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  esf_init
+------------------------------------------------------------------------------
|  Description  :  initialize ESF API function pointer table.
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void esf_init ( void )
{
#ifdef TEST_ESF_REGISTER
  esf_register ( &esf_functions );
#endif
  if ( esf_func.magic_nr != ESF_INITIALIZED )
  {
  esf_func.init_func1  = NULL;
  esf_func.init_func2  = NULL;
  esf_func.send_prim   = NULL;
  }
}
#endif


#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  esf_register
+------------------------------------------------------------------------------
|  Description  :  register the ESF API functions.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void esf_register ( T_ESF_FUNC * func )
{
  esf_func.init_func1  = func->init_func1;
  esf_func.init_func2  = func->init_func2;
  esf_func.send_prim   = func->send_prim;
  esf_func.magic_nr = ESF_INITIALIZED;
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  esf_init_func1
+------------------------------------------------------------------------------
|  Description  :  call first ESF init function
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void esf_init_func1 ( void )
{
  if ( esf_func.init_func1 != NULL )
  {
    esf_func.init_func1 ();
  }
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  esf_init_func2
+------------------------------------------------------------------------------
|  Description  :  call second ESF init function
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void esf_init_func2 ( void )
{
  if ( esf_func.init_func2 != NULL )
  {
    esf_func.init_func2 ();
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  esf_send_prim
+------------------------------------------------------------------------------
|  Description  :  call ESF send primitive function
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void esf_send_prim ( T_PRIM_HEADER * prim )
{
  if ( esf_func.send_prim != NULL )
  {
    esf_func.send_prim ( prim );
  }
}
#endif


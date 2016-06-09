/* 
+------------------------------------------------------------------------------
|  File:       prf_func.c
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
|  Purpose :  This Modul defines the profiler interface functions.
+----------------------------------------------------------------------------- 
*/ 



#ifndef __PRF_FUNC_C__
#define __PRF_FUNC_C__
#endif

/*==== INCLUDES ===================================================*/

#include "typedefs.h"
#include "glob_defs.h"
#include "vsi.h"
#include "os.h"
#include "prf_func.h"
#include "string.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/

extern ULONG TraceMask[];
extern T_HANDLE e_running[];

/*==== VARIABLES ==================================================*/

#ifndef RUN_INT_RAM
 T_PROFILER_FUNC prf_func; 
#else
 extern T_PROFILER_FUNC prf_func; 
 extern void prf_register ( T_PROFILER_FUNC * func );
#endif 
  
/*==== LINT =======================================================*/


/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  prf_init
+------------------------------------------------------------------------------
|  Description  :  initialize profiler API function pointer table.
|
|  Parameters   :  void
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_init ( void )
{
  if ( prf_func.magic_nr != PRF_INITIALIZED )
  {
    memset ( &prf_func, 0, sizeof ( T_PROFILER_FUNC ) );
  }
}
#endif


#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  prf_register
+------------------------------------------------------------------------------
|  Description  :  register the profiler API functions.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_register ( T_PROFILER_FUNC * func )
{
  prf_func.log_entity_create      = func->log_entity_create;
  prf_func.log_entity_delete      = func->log_entity_delete;
  prf_func.log_entity_activate    = func->log_entity_activate;
  prf_func.log_function_entry     = func->log_function_entry;
  prf_func.log_function_exit      = func->log_function_exit;
  prf_func.log_point_of_interest  = func->log_point_of_interest;
  prf_func.magic_nr = PRF_INITIALIZED;
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_entity_create
+------------------------------------------------------------------------------
|  Description  :  log entity create
|
|  Parameters   :  entity - unique entity indentifier
|                  name   - entity name
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_entity_create ( void * entity, const char * name )
{
  if ( prf_func.log_entity_create != NULL )
  {
    prf_func.log_entity_create ( entity, name );
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_entity_delete
+------------------------------------------------------------------------------
|  Description  :  log entity delete
|
|  Parameters   :  entity - unique entity indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_entity_delete ( void * entity )
{
  if ( prf_func.log_entity_delete != NULL )
  {
    prf_func.log_entity_delete ( entity );
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_entity_activate
+------------------------------------------------------------------------------
|  Description  :  log entity activate
|
|  Parameters   :  entity - unique entity indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_entity_activate ( void * entity )
{
  if ( prf_func.log_entity_activate != NULL )
  {
    prf_func.log_entity_activate ( entity );
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_function_entry
+------------------------------------------------------------------------------
|  Description  :  log function entry
|
|  Parameters   :  function - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_function_entry ( void * function )
{
T_HANDLE caller;

  caller = e_running [ os_MyHandle() ];
  if ( TraceMask [ caller ] & TC_PROFILER )
  {
    if ( prf_func.log_function_entry != NULL )
    {
      prf_func.log_function_entry ( function );
    }
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_function_exit
+------------------------------------------------------------------------------
|  Description  :  log function exit
|
|  Parameters   :  function - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_function_exit ( void * function )
{
T_HANDLE caller;

  caller = e_running [ os_MyHandle() ];
  if ( TraceMask [ caller ] & TC_PROFILER )
  {
    if ( prf_func.log_function_exit != NULL )
    {
      prf_func.log_function_exit ( function );
    }
  }
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_point_of_interest
+------------------------------------------------------------------------------
|  Description  :  log point of interest
|
|  Parameters   :  poi - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void prf_log_point_of_interest ( const char * poi )
{
T_HANDLE caller;

  caller = e_running [ os_MyHandle() ];
  if ( TraceMask [ caller ] & TC_PROFILER )
  {
    if ( prf_func.log_point_of_interest != NULL )
    {
      prf_func.log_point_of_interest ( poi );
    }
  }
}
#endif

//#define TEST_PROFILER_API
#ifdef TEST_PROFILER_API

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_register
+------------------------------------------------------------------------------
|  Description  :  register the profiler API functions.
|
|  Parameters   :  func - pointer to API function pointer table
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_entity_create ( void * entity, const char * name )
{
  vsi_o_ttrace ( e_running[os_MyHandle()], TC_SYSTEM, "PRF: Create entity %s, id=%d", name, entity );
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_entity_delete
+------------------------------------------------------------------------------
|  Description  :  log entity delete
|
|  Parameters   :  entity - unique entity indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_entity_delete ( void * entity )
{
  vsi_o_ttrace ( e_running[os_MyHandle()], TC_SYSTEM, "PRF: Delete entity id=%d", entity );
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_entity_activate
+------------------------------------------------------------------------------
|  Description  :  log entity activate
|
|  Parameters   :  entity - unique entity indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_entity_activate ( void * entity )
{
T_HANDLE caller;
extern T_HANDLE TST_Handle;

  caller = e_running[os_MyHandle()];
  if ( caller != TST_Handle )
    vsi_o_ttrace ( caller, TC_SYSTEM, "PRF: Activate entity %d", entity );
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_function_entry
+------------------------------------------------------------------------------
|  Description  :  log function entry
|
|  Parameters   :  function - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_function_entry ( void * function )
{
  vsi_o_ttrace ( e_running[os_MyHandle()], TC_SYSTEM, "PRF: Function entry %s", function );
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_function_exit
+------------------------------------------------------------------------------
|  Description  :  log function exit
|
|  Parameters   :  function - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_function_exit ( void * function )
{
  vsi_o_ttrace ( e_running[os_MyHandle()], TC_SYSTEM, "PRF: Function exit %s", function );
}
#endif

#ifndef RUN_FLASH
/*
+------------------------------------------------------------------------------
|  Function     :  prf_log_point_of_interest
+------------------------------------------------------------------------------
|  Description  :  log point of interest
|
|  Parameters   :  poi - unique function indentifier
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
void log_point_of_interest ( const char * poi )
{
  vsi_o_ttrace ( e_running[os_MyHandle()], TC_SYSTEM, "PRF: POI %s", poi );
}
#endif

#ifndef RUN_FLASH
T_PROFILER_FUNC profiler_functions =
{
PRF_INITIALIZED,
log_entity_create,
log_entity_delete,
log_entity_activate,
log_function_entry,
log_function_exit,
log_point_of_interest,
};
#endif

#endif /* TEST_PROFILER_API */

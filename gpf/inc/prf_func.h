/* 
+------------------------------------------------------------------------------
|  File:       prf_func.h
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
|  Purpose :  Profiler API and types.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __PRF_FUNC_H__
#define __PRF_FUNC_H__

/*==== CONSTANTS ==================================================*/

#define PRF_INITIALIZED   0xAFFEDEAD

/*==== TYPES ======================================================*/

typedef struct
{
  unsigned int magic_nr;
  void (*log_entity_create)(void * entity,const char * name);      
  void (*log_entity_delete)(void * entity);      
  void (*log_entity_activate)(void * entity);    
  void (*log_function_entry)(void * function);     
  void (*log_function_exit)(void * function);      
  void (*log_point_of_interest)(const char * poi); 
} T_PROFILER_FUNC;

/*==== PROTOTYPES =================================================*/

void prf_init ( void );
void prf_register ( T_PROFILER_FUNC * func );
void prf_log_entity_create ( void * entity, const char * name );
void prf_log_entity_delete ( void * entity );
void prf_log_entity_activate ( void * entity );
void prf_log_function_entry ( void * function );
void prf_log_function_exit ( void * function );
void prf_log_point_of_interest ( const char * poi );


#endif /* __PRF_FUNC_H__ */


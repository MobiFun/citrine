/* 
+------------------------------------------------------------------------------
|  File:       esf_func.h
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
|  Purpose :  ESF API and types.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __ESF_FUNC_H__
#define __ESF_FUNC_H__

/*==== CONSTANTS ==================================================*/

#define ESF_INITIALIZED   0xAFFEDEAD

/*==== TYPES ======================================================*/

typedef struct
{
  unsigned int magic_nr;
  void (*init_func1)(void);      
  void (*init_func2)(void);      
  void (*send_prim)(T_PRIM_HEADER *);      
} T_ESF_FUNC;

/*==== PROTOTYPES =================================================*/

void esf_init ( void );
void esf_register ( T_ESF_FUNC * func );
void esf_init_func1 ( void );
void esf_init_func2 ( void );
void esf_send_prim ( T_PRIM_HEADER * );

#endif /* __ESF_FUNC_H__ */


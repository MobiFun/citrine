/* 
+------------------------------------------------------------------------------
|  File:       dar_func.h
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
|  Purpose :  DAR properties.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __DAR_FUNC_H__
#define __DAR_FUNC_H__

/*==== CONSTANTS ==================================================*/

#define DAR_INITIALIZED   0xAFFEDEAD

/*==== TYPES ======================================================*/

typedef struct
{
  int (*diagnose_swe_filter) (int,char);
  int (*diagnose_write)(char*,char,char,int);
  int (*diagnose_write_emergency)(char*,char,int,int);
  int (*diagnose_generate_emergency)(char*,char,int);
  int gpf_use_id;
  int new_entry;
  char ascii_format;
  char warning;
  char error;
} T_GPF_DAR_PROPERTIES;

typedef struct
{
  unsigned int magic_nr;
  T_GPF_DAR_PROPERTIES * properties;      
} T_GPF_DAR_STRUCT;

/*==== PROTOTYPES =================================================*/


#endif /* __DAR_FUNC_H__ */


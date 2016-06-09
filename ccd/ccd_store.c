/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_store.c
+----------------------------------------------------------------------------- 
|  Copyright 2004 Texas Instruments Deutschland, GmbH 
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
|  Purpose :  CCD -  Definition of store handling routines
+----------------------------------------------------------------------------- 
*/ 

#define CCD_STO_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

/*
 * Standard definitions like UCHAR, ERROR etc.
 */
#include "typedefs.h"
#include "header.h"

/*
 * Types and constants used by CCD
 */
#include "ccd_globs.h"

/*
 * Type definitions for CCD data tables
 */
#include "ccdtable.h"

/*
 * Function prototypes of CCD-CCDDATA interface 
 */
#include "ccddata.h"

/*
 * Prototypes of ccd internal functions
 */
#include "ccd.h"


extern T_CCD_TASK_TABLE* ccd_task_list[];

#ifndef RUN_INT_RAM
/* Attention for RUN_...: static function */
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_store_alloc
+------------------------------------------------------------------------------
|  Description  :  This function allocates a the entity's store entry and
|                  set its next pointer to 0.
|                  The members of the entry must not reset with each 
|                  coding/decoding action.
|
|  Parameters   :  -
|
|  Return       :  pointer to struct of type T_CCD_STORE_LIST
+------------------------------------------------------------------------------
*/
static T_CCD_STORE_LIST* ccd_store_alloc ()
{
  T_CCD_STORE_LIST* stoentry;
  int i;
#if defined (CCD_TEST)
    stoentry = malloc (sizeof(T_CCD_STORE_LIST));
#else
    stoentry = D_ALLOC (sizeof(T_CCD_STORE_LIST));
#endif
    /* for future use to extend register capacity  */
    /* stoentry->next = NULL */
  if (stoentry)
  {
    for (i = 0; i < 3; i++)
    {
      stoentry->store[i] = 0;
    }
    /* for future use to extend register capacity  */
    /* (*stoentry)->next = NULL */
  }
  return stoentry;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_store_init
+------------------------------------------------------------------------------
|  Description  :  This function initializes the entity's store entry
|
|  Parameters   :  eentry - pointing to the entity's error entry
|
|  Return       :  1 in case of error, 0 otherwise
+------------------------------------------------------------------------------
*/
int ccd_store_init (T_CCD_STORE_LIST** stoentry)
{
  if (!*stoentry)
  {
#if defined (CCD_TEST)
    *stoentry = malloc (sizeof(T_CCD_STORE_LIST));
#else
    *stoentry = D_ALLOC (sizeof(T_CCD_STORE_LIST));
#endif
  }
  if (*stoentry)
  {
    int i;
    for (i = 0; i < 3; i++)
    {
      (*stoentry)->store[i] = 0;
    }
    /* for future use to extend register capacity  */
    /* (*stoentry)->next = NULL */
    return 0;
  }
  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_store_exit
+------------------------------------------------------------------------------
|  Description  :  This function frees the task related store entry in
|                  ccd_task_list.
|
|  Parameters   :  -
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
void ccd_store_exit (void)
{
  T_CCD_STORE_LIST** entry;
#if defined (CCD_TEST)
  entry = &ccd_task_list[0]->ccd_store;
  if (*entry)
  {
    /* for future use: This function frees the task related store list in case */
    /* of extended register capacity */
    /* ccd_sto_free(*stoentry); */
    free (*entry);
  }
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  entry = &ccd_task_list[me]->ccd_store;
  if (*entry)
  {
    /* for future use: This function frees the task related store list in case */
    /* of extended register capacity */
    /* ccd_sto_free(*stoentry); */
    D_FREE (*entry);
  }
#endif
  *entry = 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_writeStore
+------------------------------------------------------------------------------
|  Description  :  This function copies information to the register  
|                  retaining its entries after CCD actions are finished.
|
|  Parameters   :  globs - the current variable set
|                  regNo - to select one of the CCD STORE Registers
|                  value - information to write to CCD STORE Register
|
|  Return       :  1 in case of error, 0 otherwise
+------------------------------------------------------------------------------
*/

UBYTE ccd_writeStore (T_CCD_Globs *globs, ULONG regNo, ULONG value)
{
  T_CCD_STORE_LIST* it;
/*
#if defined (CCD_TEST)
  it = ccd_task_list[0]->ccd_store;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  it = ccd_task_list[me]->ccd_store;
#endif
*/
  it = ccd_task_list[globs->me]->ccd_store;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "Writing %x to STO[%d] ", value, regNo);
#endif

  if (regNo < 3)
  {
    if (!it)
    {
      it = ccd_store_alloc ();
    }

    if (it)
    {
       it->store[regNo] = value;
    }

    else
    {
      /* Memory allocation failed */
#ifdef DEBUG_CCD
      TRACE_CCD (globs, "Memory allocation failed!");
#endif
      return 1;
    }
  }

  else
  {
    /* for future use: extended register capacity requested */
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "Invalid register number STO[%d] ", regNo);
#endif
    return 1;
  }

  return 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_setStore
+------------------------------------------------------------------------------
|  Description  :  This function copies information from the user to the CCD 
|                  STORE Register. It retains its entries after CCD actions 
|                  are finished.
|
|  Parameters   :  regNo - to select one of the CCD STORE Registers
|                  value - information to write to CCD STORE Register
|
|  Return       :  1 in case of error, 0 otherwise
+------------------------------------------------------------------------------
*/

UBYTE  ccd_setStore ( ULONG regNo, ULONG value)
{
  T_CCD_Globs* entry;
#if defined (CCD_TEST)
  entry = ccd_task_list[0]->ccd_globs;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  entry = ccd_task_list[me]->ccd_globs;
  entry->me = me;
#endif

  return ccd_writeStore (entry, regNo, value);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_getStore
+------------------------------------------------------------------------------
|  Description  :  This function reads information from the CCD STORE Register
|                  and provides it to CCD.
|                  (CCD STORE Register retains its entries after CCD actions 
|                   are finished.)
|
|  Parameters   :  globs - the current variable set
|                  regNo - to select one of the CCD STORE Registers
|                  value - information to write to CCD STORE Register
|
|  Return       :  1 in case of error, 0 otherwise
+------------------------------------------------------------------------------
*/

UBYTE ccd_getStore (T_CCD_Globs *globs, ULONG regNo, ULONG *value)
{
  T_CCD_STORE_LIST* it = ccd_task_list[globs->me]->ccd_store;

#ifdef DEBUG_CCD
  TRACE_CCD (globs, "Reading from STO[%d] ", regNo);
#endif

  if (regNo < 3)
  {
    if (it)
    {
      *value = it->store[regNo];
    }
    else
    {
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "No CCD STORE Register persent!");
#endif
    return 1;
    }
  }

  else
  {
    /* for future use: extended register capacity requested */
#ifdef DEBUG_CCD
    TRACE_CCD (globs, "Invalid register number STO[%d] ", regNo);
#endif
    return 1;
  }

  return 0;
}
#endif /* !RUN_INT_RAM */

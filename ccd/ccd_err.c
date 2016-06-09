/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_err.c
+----------------------------------------------------------------------------- 
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
|  Purpose :  CCD -  Definition of error handling routines
+----------------------------------------------------------------------------- 
*/ 

#define CCD_ERR_C

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

/*
 * Definition of CCD Error table
 */
#include "ccd_err.h"

extern T_CCD_TASK_TABLE* ccd_task_list[];

#ifdef CCD_TEST
  #define CCD_ERR_TRC_P1(A,B,C)   {printf("\n"); printf(B, C);}
  #define CCD_ERR_TRC_P2(A,B,C,D) {printf("\n"); printf(B, C, D);}
#else
  #define CCD_ERR_TRC_P1(A,B,C)   vsi_o_ttrace(A, TC_ERROR, B, C);
  #define CCD_ERR_TRC_P2(A,B,C,D) vsi_o_ttrace(A, TC_ERROR, B, C, D);
#endif 

/*
 * backwards compatibility with the new style for error information container
 */
#define errPar    para.para_list.err_list
#define numErrPar para.para_list.num_para

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_err_reset
+------------------------------------------------------------------------------
|  Description  :  This function resets the task related error entry in
|                  ccd_task_list with each coding/decoding action.
|
|  Parameters   :  eentry - the task related error entry in ccd_task_list
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
void ccd_err_reset (T_CCD_ERR_LIST_HEAD* eentry)
{
  eentry->act_error = &(eentry->first_error);
  eentry->num_errors = 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_err_alloc
+------------------------------------------------------------------------------
|  Description  :  This function allocates a new T_CCD_ERR_LIST and
|                  set its next pointer to 0.
|
|  Parameters   :  -
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
static T_CCD_ERR_LIST* ccd_err_alloc ()
{
  T_CCD_ERR_LIST* entry;
#if defined (CCD_TEST)
    entry = malloc (sizeof(T_CCD_ERR_LIST));
#else
    entry = D_ALLOC (sizeof(T_CCD_ERR_LIST));
#endif
  if (entry)
  {
    entry->next = 0;
    return entry;
  }
  return 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_err_free
+------------------------------------------------------------------------------
|  Description  :  This function frees the task related error list.
|                  The freeing starts with the act_error entry, i.e.
|                  either it is called at the end of a code/decode action
|                  and concerns only those list entries that were allocated
|                  by a former actions and are now not used anymore, or it
|                  can be called by ccd_free_errors and than the whole list
|                  is freed.
|
|  Parameters   :  eentry - the task related error entry in ccd_task_list
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
void ccd_err_free (T_CCD_ERR_LIST_HEAD* entry)
{
  T_CCD_ERR_LIST* it = *entry->act_error;
  T_CCD_ERR_LIST* next;

  while (it)
  {
    next = it->next;
#if defined (CCD_TEST)
    free (it);
#else
    D_FREE (it);
#endif
    it = next;
  }
  *entry->act_error = 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_err_init
+------------------------------------------------------------------------------
|  Description  :  This function initializes the entity's error entry
|                  The members of the entry not set here,
|                  are reset with each coding/decoding action.
|
|  Parameters   :  eentry - pointing to the entity's error entry
|
|  Return       :  1 in case of error, 0 otherwise
+------------------------------------------------------------------------------
*/
int ccd_err_init (T_CCD_ERR_LIST_HEAD** eentry)
{
  if (!*eentry)
  {
#if defined (CCD_TEST)
    *eentry = malloc (sizeof(T_CCD_ERR_LIST_HEAD));
#else
    *eentry = D_ALLOC (sizeof(T_CCD_ERR_LIST_HEAD));
#endif
  }
  else
  {
    ccd_err_reset(*eentry);
    ccd_err_free(*eentry);
  }

  if (*eentry)
  {
    (*eentry)->first_error = 0;
    return 0;
  }
  return 1;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_err_exit
+------------------------------------------------------------------------------
|  Description  :  This function frees the task related error entry in
|                  ccd_task_list.
|
|  Parameters   :  -
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
void ccd_err_exit (void)
{
  T_CCD_ERR_LIST_HEAD** entry;
#if defined (CCD_TEST)
  entry = &ccd_task_list[0]->ccd_err_list;
  if (*entry)
  {
    ccd_err_reset(*entry);
    ccd_err_free(*entry);
    free (*entry);
  }
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  entry = &ccd_task_list[me]->ccd_err_list;
  if (*entry)
  {
    ccd_err_reset(*entry);
    ccd_err_free(*entry);
    D_FREE (*entry);
  }
#endif
  *entry = 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_trace_err_stk
+------------------------------------------------------------------------------
|  Description  :  This function traces the error stack.
|
|  Parameters   :  globs - the current variable set
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
#ifdef ERR_TRC_STK_CCD
static void ccd_trace_err_stk (T_CCD_Globs *globs)
{
  int i=1;

  if (globs->ccd_recurs_level NEQ 255)
  {
#ifdef CCD_SYMBOLS 
    CCD_ERR_TRC_P1(globs->me, "CCD Error: in message %s",
                   mcomp[globs->error_stack[0]].name)
    if (globs->ccd_recurs_level > 5)
    {
      CCD_ERR_TRC_P1(globs->me, "CCD Error:  %s", "... ->");
      i = (int) (globs->ccd_recurs_level - 5);
    }
    while (i <= globs->ccd_recurs_level)
    {
      CCD_ERR_TRC_P1(globs->me, "CCD Error:  -> %s",
                     ccddata_get_alias (globs->error_stack[i], 1))
      i++;
    }
#else   /*  CCD_SYMBOLS  */

    CCD_ERR_TRC_P1(globs->me, "CCD Error: in message with mcompRef=%d!",
                   globs->error_stack[0])
    for (i=1; i <= globs->ccd_recurs_level; i++)
      CCD_ERR_TRC_P1(globs->me, "CCD Error:  -> %d", globs->error_stack[i])
#endif  /*  CCD_SYMBOLS  */
  }  
}
#endif  /* ERR_TRC_STK_CCD */
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_setError        |
+--------------------------------------------------------------------+

  PURPOSE : Error processing of the CCD.

*/

void ccd_setError (T_CCD_Globs *globs,
                          UBYTE  ErrCode,
                          UBYTE  Action,
                          USHORT first_par,
                          ...)
{
  USHORT  par;
  UBYTE  parnum;
  va_list varpars;
  T_CCD_ERR_LIST_HEAD* entry = ccd_task_list[globs->me]->ccd_err_list;
  T_CCD_ERR_LIST** it = entry->act_error;
  char *err_msg = NULL;

  if (globs->errLabel)
  {
    Action = BREAK;
    ErrCode = globs->errLabel;
  }
  
#if defined (ERR_TRACE_CCD) || defined (DEBUG_CCD)
            
#if defined (DEBUG_CCD)
  if (Action EQ BREAK)
    CCD_ERR_TRC_P2(globs->me, "CCD Error:  %s (errCode %d); stopped processing", 
                   ccdErrCodeTable[ErrCode], ErrCode)
  else
    CCD_ERR_TRC_P2(globs->me, "CCD Error:  %s (errCode %d); yet continued processing", 
                   ccdErrCodeTable[ErrCode], ErrCode)
#else
  if (Action EQ BREAK)
    CCD_ERR_TRC_P1(globs->me, "CCD Error:  errCode %d; stopped processing", 
                   ErrCode)
  else
    CCD_ERR_TRC_P1(globs->me, "CCD Error:  errCode %d; yet continued processing", 
                   ErrCode)
#endif /* DEBUG_CCD */

#endif /* (ERR_TRACE_CCD) || defined (DEBUG_CCD) */

  if (!*it)
  {
    *it = ccd_err_alloc ();
  }

  if (*it)
  {
    /*
     * Memory allocation from dynamic partitions should not fail.
     * Nevertheless, if it fails, this is not particularly handled here
     */
    (*it)->entry.error = ErrCode;
    (*it)->entry.kind = CCD_ERR_KIND_PARA_LIST;

    va_start (varpars, first_par); /* Initialize variable arguments. */
    par = first_par;
    parnum = 0;
    while ((par != 0xffff) AND  (parnum < MAX_ERR_PAR))
    {
      (*it)->entry.errPar[parnum++] = par;
#if defined (ERR_TRACE_CCD) || defined (DEBUG_CCD)
      CCD_ERR_TRC_P1(globs->me, "CCD Error:  saved parameter %d to errList", par) 
#endif
      par = (USHORT) va_arg (varpars, int);
    }
    (*it)->entry.numErrPar = parnum;
    va_end (varpars);              /* Reset variable arguments.      */

    entry->act_error = &(*it)->next;
  }

  entry->num_errors++;

#ifdef ERR_TRC_STK_CCD
  ccd_trace_err_stk (globs);
#endif  /* ERR_TRC_STK_CCD */

  /*
   * if the action say break, perform a longjump to terminate ccd.
   */
  if (Action EQ BREAK)
  {
    globs->CCD_Error = ccdError;
    if (globs->jmp_mark_set)
      longjmp (globs->jmp_mark, -1);
  }
  else
    globs->CCD_Error = ccdWarning;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_get_error
+------------------------------------------------------------------------------
|  Description  :  This function copies the next error information of the
|                  classical parameter list kind to the user.
|
|  Parameters   :  item - the task's current error entry
|                  entry - the returned pointer
|
|  Return       :  the error code if there was still an error, ccdOK otherwise
+------------------------------------------------------------------------------
*/
static ULONG ccd_get_error (T_CCD_ERR_LIST_HEAD* head, USHORT *parlist)
{
  T_CCD_ERR_LIST** item;
  int i;
  if (head)
  {
    item = head->act_error;
    if (*item)
    {
      if ((*item)->entry.kind == CCD_ERR_KIND_PARA_LIST)
      {
        for (i=0; i < (*item)->entry.numErrPar; i++)
          *parlist++ = (*item)->entry.errPar[i];
      }
      head->act_error = &(*item)->next;
      return (ULONG) (*item)->entry.error;
    }
    return ccdOK;
  }
  return ccdOK;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_getNextError    |
+--------------------------------------------------------------------+

  PURPOSE : if an error is stored in the errorlist, this function
            stores the additional stored error parameter
            into the given parlist and returns the errorcode.
            If no error occured this function returns 0.

*/

UBYTE CCDDATA_PREF(ccd_getNextError) (UBYTE entity, USHORT *parlist)
/*
 * The parameter entity is not used anymore, but the function interface
 * should remain the same.
 */
{
  T_CCD_ERR_LIST_HEAD* head;
#if defined (CCD_TEST)
  head = ccd_task_list[0]->ccd_err_list;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  head = ccd_task_list[me]->ccd_err_list;
#endif

  return (UBYTE) ccd_get_error (head, parlist);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_getFirstError   |
+--------------------------------------------------------------------+

  PURPOSE : if an error is stored in the errorlist, this function
            stores the additional stored error parameter
            into the given parlist and returns the errorcode.
            If no error occured this function returns 0.

*/

UBYTE CCDDATA_PREF(ccd_getFirstError) (UBYTE entity, USHORT *parlist)
{
/*
 * The parameter entity is not used anymore, but the function interface
 * should remain the same.
 */
  T_CCD_ERR_LIST_HEAD* head;
#if defined (CCD_TEST)
  head = ccd_task_list[0]->ccd_err_list;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  head = ccd_task_list[me]->ccd_err_list;
#endif

  head->act_error = &(head->first_error);

  return (UBYTE) ccd_get_error (head, parlist);

}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_get_numFaults
+------------------------------------------------------------------------------
|  Description  :  This function delivers the task related number of
|                  errors/faults of the last coding/decoding action.
|
|  Parameters   :  -
|
|  Return       :  the number of occurred errors
+------------------------------------------------------------------------------
*/

int CCDDATA_PREF(ccd_get_numFaults) ()
{
  T_CCD_ERR_LIST_HEAD* head;
#if defined (CCD_TEST)
  head = ccd_task_list[0]->ccd_err_list;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  head = ccd_task_list[me]->ccd_err_list;
#endif

  if (head)
    return head->num_errors;
  else
    return 0;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_free_faultlist
+------------------------------------------------------------------------------
|  Description  :  This function frees any allocated error/fault inforamtion
|                  of the current task.
|
|  Parameters   :  -
|
|  Return       :  -
+------------------------------------------------------------------------------
*/

void CCDDATA_PREF(ccd_free_faultlist) ()
{
#if defined (CCD_TEST)
  int me = 0;
#else
  T_HANDLE me;
  me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
#endif
  ccd_err_reset (ccd_task_list[me]->ccd_err_list);
  ccd_err_free (ccd_task_list[me]->ccd_err_list);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_recordFault     |
+--------------------------------------------------------------------+

  PURPOSE : Record of information on an detected error to support
            entity fault diagnosis activities.

*/

void ccd_recordFault (T_CCD_Globs *globs,
                          UBYTE  ErrCode,
                          UBYTE  Action,
                          T_ERR_INFO err_info,
                          U8       *err_IEaddr)
{
  char *err_msg = NULL;
  T_CCD_ERR_LIST_HEAD* entry = ccd_task_list[globs->me]->ccd_err_list;
  T_CCD_ERR_LIST** it = entry->act_error;

  if (globs->errLabel)
  {
    Action = BREAK;
    ErrCode = globs->errLabel;
  }

  if (!*it)
  {
    *it = ccd_err_alloc ();
  }

  if (*it)
  {
    /*
     * Memory allocation from dynamic partitions should not fail.
     * Nevertheless, if it fails, this is not particularly handled here
     */
    (*it)->entry.error = ErrCode;
    (*it)->entry.kind = CCD_ERR_KIND_IE_TYPE;
    (*it)->entry.para.err_type.err_info = err_info;
    (*it)->entry.para.err_type.err_IEaddr = (U32)err_IEaddr;
    entry->act_error = &(*it)->next;
  }

  entry->num_errors++;

#if defined (ERR_TRACE_CCD) || defined (DEBUG_CCD)

#if defined (DEBUG_CCD)
  if (Action EQ BREAK)
    CCD_ERR_TRC_P2(globs->me, "CCD Error:  %s (errCode %d); stopped processing",
                   ccdErrCodeTable[ErrCode], ErrCode)
  else
    CCD_ERR_TRC_P2(globs->me, "CCD Error:  %s (errCode %d); yet continued processing",
                   ccdErrCodeTable[ErrCode], ErrCode)
#else
  if (Action EQ BREAK)
    CCD_ERR_TRC_P1(globs->me, "CCD Error:  errCode %d; stopped processing",
                   ErrCode)
  else
    CCD_ERR_TRC_P1(globs->me, "CCD Error:  errCode %d; yet continued processing",
                   ErrCode)
#endif /* DEBUG_CCD */
                
#endif /* (ERR_TRACE_CCD) || defined (DEBUG_CCD) */

#ifdef ERR_TRC_STK_CCD
  ccd_trace_err_stk (globs);
#endif  /* ERR_TRC_STK_CCD */

  /*
   * if the action say break, perform a longjump to terminate ccd.
   */
  if (Action EQ BREAK)
  {
    globs->CCD_Error = ccdError;
    if (globs->jmp_mark_set)
      longjmp (globs->jmp_mark, -1);
  }
  else
    globs->CCD_Error = ccdWarning;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  ccd_get_fault
+------------------------------------------------------------------------------
|  Description  :  This function copies the next error information to
|                  the user (no matter which kind).
|
|  Parameters   :  item - the task's current error entry
|                  entry - the returned pointer
|
|  Return       :  the error code if there was still an error, ccdOK otherwise
+------------------------------------------------------------------------------
*/
static ULONG ccd_get_fault (T_CCD_ERR_LIST_HEAD* head, T_CCD_ERR_ENTRY **entry)
{
  T_CCD_ERR_LIST** item;
  if (head)
  {
    item = head->act_error;
    if (*item)
    {
      *entry = &(*item)->entry;
      head->act_error = &(*item)->next;
      return (ULONG) (*entry)->error;
    }
    return ccdOK;
  }
  return ccdOK;
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_getNextFault    |
+--------------------------------------------------------------------+

  PURPOSE : If an error is stored in the errorlist, this function
            copies information on the error into the function parameter.
            This information is made of error number, error-union type,
            ccd_id for the faulty element and the address of this
            element in the C-structure of the decoded message. 
            If no error occured this function returns 0.

*/

ULONG CCDDATA_PREF(ccd_getNextFault) (T_CCD_ERR_ENTRY **ccd_err_entry)
{
  T_CCD_ERR_LIST_HEAD* head;
#if defined (CCD_TEST)
  head = ccd_task_list[0]->ccd_err_list;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  head = ccd_task_list[me]->ccd_err_list;
#endif

  return ccd_get_fault (head, ccd_err_entry);
}
#endif /* !RUN_INT_RAM */

#ifndef RUN_INT_RAM
/*
+--------------------------------------------------------------------+
| PROJECT : CCD (6144)                 MODULE  : CCD                 |
| STATE   : code                       ROUTINE : ccd_getFirstFault   |
+--------------------------------------------------------------------+

  PURPOSE : If an error is stored in the errorlist, this function
            copies information on the error into the function parameter.
            This information is made of error number, error-union type,
            ccd_id for the faulty element and the address of this
            element in the C-structure of the decoded message. 
            If no error occured this function returns 0.
*/

ULONG CCDDATA_PREF(ccd_getFirstFault) (T_CCD_ERR_ENTRY **ccd_err_entry)
{
  T_CCD_ERR_LIST_HEAD* head;
#if defined (CCD_TEST)
  head = ccd_task_list[0]->ccd_err_list;
#else
  T_HANDLE me = vsi_e_handle (0, NULL);
  if (me == VSI_ERROR)
    me = 0;
  head = ccd_task_list[me]->ccd_err_list;
#endif

  head->act_error = &(head->first_error);

  return ccd_get_fault (head, ccd_err_entry);
}
#endif /* !RUN_INT_RAM */
